/* Character set conversion support for GDB.

   Copyright (C) 2001, 2003, 2007, 2008, 2009 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include "defs.h"
#include "charset.h"
#include "gdbcmd.h"
#include "gdb_assert.h"
#include "gdb_obstack.h"
#include "charset-list.h"
#include "vec.h"

#include <stddef.h>
#include "gdb_string.h"
#include <ctype.h>


/* How GDB's character set support works

   GDB has three global settings:

   - The `current host character set' is the character set GDB should
     use in talking to the user, and which (hopefully) the user's
     terminal knows how to display properly.  Most users should not
     change this.

   - The `current target character set' is the character set the
     program being debugged uses.

   - The `current target wide character set' is the wide character set
     the program being debugged uses, that is, the encoding used for
     wchar_t.

   There are commands to set each of these, and mechanisms for
   choosing reasonable default values.  GDB has a global list of
   character sets that it can use as its host or target character
   sets.

   The header file `charset.h' declares various functions that
   different pieces of GDB need to perform tasks like:

   - printing target strings and characters to the user's terminal
     (mostly target->host conversions),

   - building target-appropriate representations of strings and
     characters the user enters in expressions (mostly host->target
     conversions),

     and so on.
     
   To avoid excessive code duplication and maintenance efforts,
   GDB simply requires a capable iconv function.  Users on platforms
   without a suitable iconv can use the GNU iconv library.  */


#ifdef PHONY_ICONV

/* Provide a phony iconv that does as little as possible.  Also,
   arrange for there to be a single available character set.  */

#undef GDB_DEFAULT_HOST_CHARSET
#define GDB_DEFAULT_HOST_CHARSET "ISO-8859-1"
#define GDB_DEFAULT_TARGET_CHARSET "ISO-8859-1"
#define GDB_DEFAULT_TARGET_WIDE_CHARSET "ISO-8859-1"
#undef DEFAULT_CHARSET_NAMES
#define DEFAULT_CHARSET_NAMES GDB_DEFAULT_HOST_CHARSET ,

#undef iconv_t
#define iconv_t int
#undef iconv_open
#undef iconv
#undef iconv_close

#undef ICONV_CONST
#define ICONV_CONST const

/* Some systems don't have EILSEQ, so we define it here, but not as
   EINVAL, because callers of `iconv' want to distinguish EINVAL and
   EILSEQ.  This is what iconv.h from libiconv does as well.  Note
   that wchar.h may also define EILSEQ, so this needs to be after we
   include wchar.h, which happens in defs.h through gdb_wchar.h.  */
#ifndef EILSEQ
#define EILSEQ ENOENT
#endif

iconv_t
iconv_open (const char *to, const char *from)
{
  /* We allow conversions from UCS-4BE, wchar_t, and the host charset.
     We allow conversions to wchar_t and the host charset.  */
  if (strcmp (from, "UCS-4BE") && strcmp (from, "wchar_t")
      && strcmp (from, GDB_DEFAULT_HOST_CHARSET))
    return -1;
  if (strcmp (to, "wchar_t") && strcmp (to, GDB_DEFAULT_HOST_CHARSET))
    return -1;

  /* Return 1 if we are converting from UCS-4BE, 0 otherwise.  This is
     used as a flag in calls to iconv.  */
  return !strcmp (from, "UCS-4BE");
}

int
iconv_close (iconv_t arg)
{
  return 0;
}

size_t
iconv (iconv_t ucs_flag, const char **inbuf, size_t *inbytesleft,
       char **outbuf, size_t *outbytesleft)
{
  if (ucs_flag)
    {
      while (*inbytesleft >= 4)
	{
	  size_t j;
	  unsigned long c = 0;

	  for (j = 0; j < 4; ++j)
	    {
	      c <<= 8;
	      c += (*inbuf)[j] & 0xff;
	    }

	  if (c >= 256)
	    {
	      errno = EILSEQ;
	      return -1;
	    }
	  **outbuf = c & 0xff;
	  ++*outbuf;
	  --*outbytesleft;

	  ++*inbuf;
	  *inbytesleft -= 4;
	}
      if (*inbytesleft < 4)
	{
	  errno = EINVAL;
	  return -1;
	}
    }
  else
    {
      /* In all other cases we simply copy input bytes to the
	 output.  */
      size_t amt = *inbytesleft;
      if (amt > *outbytesleft)
	amt = *outbytesleft;
      memcpy (*outbuf, *inbuf, amt);
      *inbuf += amt;
      *outbuf += amt;
      *inbytesleft -= amt;
      *outbytesleft -= amt;
    }

  if (*inbytesleft)
    {
      errno = E2BIG;
      return -1;
    }

  /* The number of non-reversible conversions -- but they were all
     reversible.  */
  return 0;
}

#endif



/* The global lists of character sets and translations.  */


#ifndef GDB_DEFAULT_TARGET_CHARSET
#define GDB_DEFAULT_TARGET_CHARSET "ISO-8859-1"
#endif

#ifndef GDB_DEFAULT_TARGET_WIDE_CHARSET
#define GDB_DEFAULT_TARGET_WIDE_CHARSET "UCS-4"
#endif

static const char *auto_host_charset_name = GDB_DEFAULT_HOST_CHARSET;
static const char *host_charset_name = "auto";
static void
show_host_charset_name (struct ui_file *file, int from_tty,
			struct cmd_list_element *c,
			const char *value)
{
  if (!strcmp (value, "auto"))
    fprintf_filtered (file,
		      _("The host character set is \"auto; currently %s\".\n"),
		      auto_host_charset_name);
  else
    fprintf_filtered (file, _("The host character set is \"%s\".\n"), value);
}

static const char *target_charset_name = GDB_DEFAULT_TARGET_CHARSET;
static void
show_target_charset_name (struct ui_file *file, int from_tty,
			  struct cmd_list_element *c, const char *value)
{
  fprintf_filtered (file, _("The target character set is \"%s\".\n"),
		    value);
}

static const char *target_wide_charset_name = GDB_DEFAULT_TARGET_WIDE_CHARSET;
static void
show_target_wide_charset_name (struct ui_file *file, int from_tty,
			       struct cmd_list_element *c, const char *value)
{
  fprintf_filtered (file, _("The target wide character set is \"%s\".\n"),
		    value);
}

static const char *default_charset_names[] =
{
  DEFAULT_CHARSET_NAMES
  0
};

static const char **charset_enum;


/* If the target wide character set has big- or little-endian
   variants, these are the corresponding names.  */
static const char *target_wide_charset_be_name;
static const char *target_wide_charset_le_name;

/* A helper function for validate which sets the target wide big- and
   little-endian character set names, if possible.  */

static void
set_be_le_names (void)
{
  int i, len;

  target_wide_charset_le_name = NULL;
  target_wide_charset_be_name = NULL;

  len = strlen (target_wide_charset_name);
  for (i = 0; charset_enum[i]; ++i)
    {
      if (strncmp (target_wide_charset_name, charset_enum[i], len))
	continue;
      if ((charset_enum[i][len] == 'B'
	   || charset_enum[i][len] == 'L')
	  && charset_enum[i][len + 1] == 'E'
	  && charset_enum[i][len + 2] == '\0')
	{
	  if (charset_enum[i][len] == 'B')
	    target_wide_charset_be_name = charset_enum[i];
	  else
	    target_wide_charset_le_name = charset_enum[i];
	}
    }
}

/* 'Set charset', 'set host-charset', 'set target-charset', 'set
   target-wide-charset', 'set charset' sfunc's.  */

static void
validate (void)
{
  iconv_t desc;
  const char *host_cset = host_charset ();

  desc = iconv_open (target_wide_charset_name, host_cset);
  if (desc == (iconv_t) -1)
    error ("Cannot convert between character sets `%s' and `%s'",
	   target_wide_charset_name, host_cset);
  iconv_close (desc);

  desc = iconv_open (target_charset_name, host_cset);
  if (desc == (iconv_t) -1)
    error ("Cannot convert between character sets `%s' and `%s'",
	   target_charset_name, host_cset);
  iconv_close (desc);

  set_be_le_names ();
}

/* This is the sfunc for the 'set charset' command.  */
static void
set_charset_sfunc (char *charset, int from_tty, struct cmd_list_element *c)
{
  /* CAREFUL: set the target charset here as well. */
  target_charset_name = host_charset_name;
  validate ();
}

/* 'set host-charset' command sfunc.  We need a wrapper here because
   the function needs to have a specific signature.  */
static void
set_host_charset_sfunc (char *charset, int from_tty,
			struct cmd_list_element *c)
{
  validate ();
}

/* Wrapper for the 'set target-charset' command.  */
static void
set_target_charset_sfunc (char *charset, int from_tty,
			  struct cmd_list_element *c)
{
  validate ();
}

/* Wrapper for the 'set target-wide-charset' command.  */
static void
set_target_wide_charset_sfunc (char *charset, int from_tty,
			       struct cmd_list_element *c)
{
  validate ();
}

/* sfunc for the 'show charset' command.  */
static void
show_charset (struct ui_file *file, int from_tty, struct cmd_list_element *c,
	      const char *name)
{
  show_host_charset_name (file, from_tty, c, host_charset_name);
  show_target_charset_name (file, from_tty, c, target_charset_name);
  show_target_wide_charset_name (file, from_tty, c, target_wide_charset_name);
}


/* Accessor functions.  */

const char *
host_charset (void)
{
  if (!strcmp (host_charset_name, "auto"))
    return auto_host_charset_name;
  return host_charset_name;
}

const char *
target_charset (void)
{
  return target_charset_name;
}

const char *
target_wide_charset (void)
{
  if (gdbarch_byte_order (current_gdbarch) == BFD_ENDIAN_BIG)
    {
      if (target_wide_charset_be_name)
	return target_wide_charset_be_name;
    }
  else
    {
      if (target_wide_charset_le_name)
	return target_wide_charset_le_name;
    }

  return target_wide_charset_name;
}


/* Host character set management.  For the time being, we assume that
   the host character set is some superset of ASCII.  */

char
host_letter_to_control_character (char c)
{
  if (c == '?')
    return 0177;
  return c & 0237;
}

/* Convert a host character, C, to its hex value.  C must already have
   been validated using isxdigit.  */

int
host_hex_value (char c)
{
  if (isdigit (c))
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return 10 + c - 'a';
  gdb_assert (c >= 'A' && c <= 'F');
  return 10 + c - 'A';
}


/* Public character management functions.  */

/* A cleanup function which is run to close an iconv descriptor.  */

static void
cleanup_iconv (void *p)
{
  iconv_t *descp = p;
  iconv_close (*descp);
}

void
convert_between_encodings (const char *from, const char *to,
			   const gdb_byte *bytes, unsigned int num_bytes,
			   int width, struct obstack *output,
			   enum transliterations translit)
{
  iconv_t desc;
  struct cleanup *cleanups;
  size_t inleft;
  char *inp;
  unsigned int space_request;

  /* Often, the host and target charsets will be the same.  */
  if (!strcmp (from, to))
    {
      obstack_grow (output, bytes, num_bytes);
      return;
    }

  desc = iconv_open (to, from);
  if (desc == (iconv_t) -1)
    perror_with_name ("Converting character sets");
  cleanups = make_cleanup (cleanup_iconv, &desc);

  inleft = num_bytes;
  inp = (char *) bytes;

  space_request = num_bytes;

  while (inleft > 0)
    {
      char *outp;
      size_t outleft, r;
      int old_size;

      old_size = obstack_object_size (output);
      obstack_blank (output, space_request);

      outp = obstack_base (output) + old_size;
      outleft = space_request;

      r = iconv (desc, (ICONV_CONST char **) &inp, &inleft, &outp, &outleft);

      /* Now make sure that the object on the obstack only includes
	 bytes we have converted.  */
      obstack_blank (output, - (int) outleft);

      if (r == (size_t) -1)
	{
	  switch (errno)
	    {
	    case EILSEQ:
	      {
		int i;

		/* Invalid input sequence.  */
		if (translit == translit_none)
		  error (_("Could not convert character to `%s' character set"),
			 to);

		/* We emit escape sequence for the bytes, skip them,
		   and try again.  */
		for (i = 0; i < width; ++i)
		  {
		    char octal[5];

		    sprintf (octal, "\\%.3o", *inp & 0xff);
		    obstack_grow_str (output, octal);

		    ++inp;
		    --inleft;
		  }
	      }
	      break;

	    case E2BIG:
	      /* We ran out of space in the output buffer.  Make it
		 bigger next time around.  */
	      space_request *= 2;
	      break;

	    case EINVAL:
	      /* Incomplete input sequence.  FIXME: ought to report this
		 to the caller somehow.  */
	      inleft = 0;
	      break;

	    default:
	      perror_with_name ("Internal error while converting character sets");
	    }
	}
    }

  do_cleanups (cleanups);
}



/* An iterator that returns host wchar_t's from a target string.  */
struct wchar_iterator
{
  /* The underlying iconv descriptor.  */
  iconv_t desc;

  /* The input string.  This is updated as convert characters.  */
  char *input;
  /* The number of bytes remaining in the input.  */
  size_t bytes;

  /* The width of an input character.  */
  size_t width;

  /* The output buffer and its size.  */
  gdb_wchar_t *out;
  size_t out_size;
};

/* Create a new iterator.  */
struct wchar_iterator *
make_wchar_iterator (const gdb_byte *input, size_t bytes, const char *charset,
		     size_t width)
{
  struct wchar_iterator *result;
  iconv_t desc;

  desc = iconv_open ("wchar_t", charset);
  if (desc == (iconv_t) -1)
    perror_with_name ("Converting character sets");

  result = XNEW (struct wchar_iterator);
  result->desc = desc;
  result->input = (char *) input;
  result->bytes = bytes;
  result->width = width;

  result->out = XNEW (gdb_wchar_t);
  result->out_size = 1;

  return result;
}

static void
do_cleanup_iterator (void *p)
{
  struct wchar_iterator *iter = p;

  iconv_close (iter->desc);
  xfree (iter->out);
  xfree (iter);
}

struct cleanup *
make_cleanup_wchar_iterator (struct wchar_iterator *iter)
{
  return make_cleanup (do_cleanup_iterator, iter);
}

int
wchar_iterate (struct wchar_iterator *iter,
	       enum wchar_iterate_result *out_result,
	       gdb_wchar_t **out_chars,
	       const gdb_byte **ptr,
	       size_t *len)
{
  size_t out_request;

  /* Try to convert some characters.  At first we try to convert just
     a single character.  The reason for this is that iconv does not
     necessarily update its outgoing arguments when it encounters an
     invalid input sequence -- but we want to reliably report this to
     our caller so it can emit an escape sequence.  */
  out_request = 1;
  while (iter->bytes > 0)
    {
      char *outptr = (char *) &iter->out[0];
      char *orig_inptr = iter->input;
      size_t orig_in = iter->bytes;
      size_t out_avail = out_request * sizeof (gdb_wchar_t);
      size_t num;
      gdb_wchar_t result;

      size_t r = iconv (iter->desc,
			(ICONV_CONST char **) &iter->input, &iter->bytes,
			&outptr, &out_avail);
      if (r == (size_t) -1)
	{
	  switch (errno)
	    {
	    case EILSEQ:
	      /* Invalid input sequence.  Skip it, and let the caller
		 know about it.  */
	      *out_result = wchar_iterate_invalid;
	      *ptr = iter->input;
	      *len = iter->width;
	      iter->input += iter->width;
	      iter->bytes -= iter->width;
	      return 0;

	    case E2BIG:
	      /* We ran out of space.  We still might have converted a
		 character; if so, return it.  Otherwise, grow the
		 buffer and try again.  */
	      if (out_avail < out_request * sizeof (gdb_wchar_t))
		break;

	      ++out_request;
	      if (out_request > iter->out_size)
		{
		  iter->out_size = out_request;
		  iter->out = xrealloc (iter->out,
					out_request * sizeof (gdb_wchar_t));
		}
	      continue;

	    case EINVAL:
	      /* Incomplete input sequence.  Let the caller know, and
		 arrange for future calls to see EOF.  */
	      *out_result = wchar_iterate_incomplete;
	      *ptr = iter->input;
	      *len = iter->bytes;
	      iter->bytes = 0;
	      return 0;

	    default:
	      perror_with_name ("Internal error while converting character sets");
	    }
	}

      /* We converted something.  */
      num = out_request - out_avail / sizeof (gdb_wchar_t);
      *out_result = wchar_iterate_ok;
      *out_chars = iter->out;
      *ptr = orig_inptr;
      *len = orig_in - iter->bytes;
      return num;
    }

  /* Really done.  */
  *out_result = wchar_iterate_eof;
  return -1;
}


/* The charset.c module initialization function.  */

extern initialize_file_ftype _initialize_charset; /* -Wmissing-prototype */

typedef char *char_ptr;
DEF_VEC_P (char_ptr);

static VEC (char_ptr) *charsets;

#ifdef PHONY_ICONV

static void
find_charset_names (void)
{
  VEC_safe_push (char_ptr, charsets, GDB_DEFAULT_HOST_CHARSET);
  VEC_safe_push (char_ptr, charsets, NULL);
}

#else /* PHONY_ICONV */

/* Sometimes, libiconv redefines iconvlist as libiconvlist -- but
   provides different symbols in the static and dynamic libraries.
   So, configure may see libiconvlist but not iconvlist.  But, calling
   iconvlist is the right thing to do and will work.  Hence we do a
   check here but unconditionally call iconvlist below.  */
#if defined (HAVE_ICONVLIST) || defined (HAVE_LIBICONVLIST)

/* A helper function that adds some character sets to the vector of
   all character sets.  This is a callback function for iconvlist.  */

static int
add_one (unsigned int count, const char *const *names, void *data)
{
  unsigned int i;

  for (i = 0; i < count; ++i)
    VEC_safe_push (char_ptr, charsets, xstrdup (names[i]));

  return 0;
}

static void
find_charset_names (void)
{
  iconvlist (add_one, NULL);
  VEC_safe_push (char_ptr, charsets, NULL);
}

#else

static void
find_charset_names (void)
{
  FILE *in;

  in = popen ("iconv -l", "r");
  /* It is ok to ignore errors; we'll fall back on a default.  */
  if (!in)
    return;

  /* POSIX says that iconv -l uses an unspecified format.  We parse
     the glibc format; feel free to add others as needed.  */
  while (!feof (in))
    {
      /* The size of buf is chosen arbitrarily.  A character set name
	 longer than this would not be very nice.  */
      char buf[80];
      int len;
      char *r = fgets (buf, sizeof (buf), in);
      if (!r)
	break;
      len = strlen (r);
      if (len <= 3)
	continue;
      if (buf[len - 2] == '/' && buf[len - 3] == '/')
	buf[len - 3] = '\0';
      VEC_safe_push (char_ptr, charsets, xstrdup (buf));
    }

  pclose (in);

  VEC_safe_push (char_ptr, charsets, NULL);
}

#endif /* HAVE_ICONVLIST || HAVE_LIBICONVLIST */
#endif /* PHONY_ICONV */

void
_initialize_charset (void)
{
  struct cmd_list_element *new_cmd;

  /* The first element is always "auto"; then we skip it for the
     commands where it is not allowed.  */
  VEC_safe_push (char_ptr, charsets, "auto");
  find_charset_names ();

  if (VEC_length (char_ptr, charsets) > 1)
    charset_enum = (const char **) VEC_address (char_ptr, charsets);
  else
    charset_enum = default_charset_names;

#ifndef PHONY_ICONV
#ifdef HAVE_LANGINFO_CODESET
  auto_host_charset_name = nl_langinfo (CODESET);
  target_charset_name = auto_host_charset_name;

  set_be_le_names ();
#endif
#endif

  add_setshow_enum_cmd ("charset", class_support,
			&charset_enum[1], &host_charset_name, _("\
Set the host and target character sets."), _("\
Show the host and target character sets."), _("\
The `host character set' is the one used by the system GDB is running on.\n\
The `target character set' is the one used by the program being debugged.\n\
You may only use supersets of ASCII for your host character set; GDB does\n\
not support any others.\n\
To see a list of the character sets GDB supports, type `set charset <TAB>'."),
			/* Note that the sfunc below needs to set
			   target_charset_name, because the 'set
			   charset' command sets two variables.  */
			set_charset_sfunc,
			show_charset,
			&setlist, &showlist);

  add_setshow_enum_cmd ("host-charset", class_support,
			charset_enum, &host_charset_name, _("\
Set the host character set."), _("\
Show the host character set."), _("\
The `host character set' is the one used by the system GDB is running on.\n\
You may only use supersets of ASCII for your host character set; GDB does\n\
not support any others.\n\
To see a list of the character sets GDB supports, type `set host-charset <TAB>'."),
			set_host_charset_sfunc,
			show_host_charset_name,
			&setlist, &showlist);

  add_setshow_enum_cmd ("target-charset", class_support,
			&charset_enum[1], &target_charset_name, _("\
Set the target character set."), _("\
Show the target character set."), _("\
The `target character set' is the one used by the program being debugged.\n\
GDB translates characters and strings between the host and target\n\
character sets as needed.\n\
To see a list of the character sets GDB supports, type `set target-charset'<TAB>"),
			set_target_charset_sfunc,
			show_target_charset_name,
			&setlist, &showlist);

  add_setshow_enum_cmd ("target-wide-charset", class_support,
			&charset_enum[1], &target_wide_charset_name,
			_("\
Set the target wide character set."), _("\
Show the target wide character set."), _("\
The `target wide character set' is the one used by the program being debugged.\n\
In particular it is the encoding used by `wchar_t'.\n\
GDB translates characters and strings between the host and target\n\
character sets as needed.\n\
To see a list of the character sets GDB supports, type\n\
`set target-wide-charset'<TAB>"),
			set_target_wide_charset_sfunc,
			show_target_wide_charset_name,
			&setlist, &showlist);
}
