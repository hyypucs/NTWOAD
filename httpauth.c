/* Authorization support: We support two authorization schemes:

   * `Basic' scheme, consisting of base64-ing USER:PASSWORD string;

   * `Digest' scheme, added by Junio Hamano <junio@twinsun.com>,
   consisting of answering to the server's challenge with the proper
   MD5 digests.  */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include "md5.h"
#include "httpauth.h"

#define ISSPACE(x)  isspace ((unsigned char)(x))
/* Free FOO if it is non-NULL.  */
#define FREE_MAYBE(foo) do { if (foo) free (foo); } while (0)

/* ASCII char -> HEX digit */
#define ASC2HEXD(x) (((x) >= '0' && (x) <= '9') ?               \
                     ((x) - '0') : (toupper(x) - 'A' + 10))

/* HEX digit -> ASCII char */
#define HEXD2ASC(x) (((x) < 10) ? ((x) + '0') : ((x) - 10 + 'A'))

#define ARRAY_SIZE(array) (sizeof (array) / sizeof (*(array)))
/* Note that this much more elegant definition cannot be used:

   #define STRDUP_ALLOCA(str) (strcpy ((char *)alloca (strlen (str) + 1), str))

   This is because some compilers don't handle alloca() as argument to
   function correctly.  Gcc under Intel has been reported to offend in
   this case.  */

#define STRDUP_ALLOCA(ptr, str) do {            \
  (ptr) = (char *)alloca (strlen (str) + 1);    \
  strcpy (ptr, str);                            \
} while (0)

#define ALLOCA_ARRAY(type, len) ((type *) alloca ((len) * sizeof (type)))

#define XREALLOC_ARRAY(ptr, type, len)                                  \
     ((void) (ptr = (type *) xrealloc (ptr, (len) * sizeof (type))))

/* Generally useful if you want to avoid arbitrary size limits but
   don't need a full dynamic array.  Assumes that BASEVAR points to a
   malloced array of TYPE objects (or possibly a NULL pointer, if
   SIZEVAR is 0), with the total size stored in SIZEVAR.  This macro
   will realloc BASEVAR as necessary so that it can hold at least
   NEEDED_SIZE objects.  The reallocing is done by doubling, which
   ensures constant amortized time per element.  */
#define DO_REALLOC(basevar, sizevar, needed_size, type) do      \
{                                                               \
  /* Avoid side-effectualness.  */                              \
  long do_realloc_needed_size = (needed_size);                  \
  long do_realloc_newsize = 0;                                  \
  while ((sizevar) < (do_realloc_needed_size)) {                \
    do_realloc_newsize = 2*(sizevar);                           \
    if (do_realloc_newsize < 32)                                \
      do_realloc_newsize = 32;                                  \
    (sizevar) = do_realloc_newsize;                             \
  }                                                             \
  if (do_realloc_newsize)                                       \
    XREALLOC_ARRAY (basevar, type, do_realloc_newsize);         \
} while (0)

#define MD5_HASHLEN 16


static void base64_encode (const char *s, char *store, int length);
static char *basic_authentication_encode (const char *user, const char *passwd, const char *header);
static int extract_header_attr (const char *au, const char *attr_name, char **ret);
static void dump_hash (unsigned char *buf, const unsigned char *hash);
static char * digest_authentication_encode (const char *au, const char *user, const char *passwd, const char *method, const char *path);
static int skip_lws (const char *string);
static char *strdupdelim (const char *beg, const char *end);

/* How many bytes it will take to store LEN bytes in base64.  */
#define BASE64_LENGTH(len) (4 * (((len) + 2) / 3))

/* Encode the string S of length LENGTH to base64 format and place it
   to STORE.  STORE will be 0-terminated, and must point to a writable
   buffer of at least 1+BASE64_LENGTH(length) bytes.  */
static void base64_encode (const char *s, char *store, int length)
{
  /* Conversion table.  */
  static char tbl[64] = {
    'A','B','C','D','E','F','G','H',
    'I','J','K','L','M','N','O','P',
    'Q','R','S','T','U','V','W','X',
    'Y','Z','a','b','c','d','e','f',
    'g','h','i','j','k','l','m','n',
    'o','p','q','r','s','t','u','v',
    'w','x','y','z','0','1','2','3',
    '4','5','6','7','8','9','+','/'
  };
  int i;
  unsigned char *p = (unsigned char *)store;

  /* Transform the 3x8 bits to 4x6 bits, as required by base64.  */
  for (i = 0; i < length; i += 3)
    {
      *p++ = tbl[s[0] >> 2];
      *p++ = tbl[((s[0] & 3) << 4) + (s[1] >> 4)];
      *p++ = tbl[((s[1] & 0xf) << 2) + (s[2] >> 6)];
      *p++ = tbl[s[2] & 0x3f];
      s += 3;
    }
  /* Pad the result if necessary...  */
  if (i == length + 1)
    *(p - 1) = '=';
  else if (i == length + 2)
    *(p - 1) = *(p - 2) = '=';
  /* ...and zero-terminate it.  */
  *p = '\0';
}

/* Create the authentication header contents for the `Basic' scheme.
   This is done by encoding the string `USER:PASS' in base64 and
   prepending `HEADER: Basic ' to it.  */
static char *basic_authentication_encode (const char *user, const char *passwd, const char *header)
{
  char *t1, *t2, *res;
  int len1 = strlen (user) + 1 + strlen (passwd);
  int len2 = BASE64_LENGTH (len1);

  t1 = (char *)alloca (len1 + 1);
  sprintf (t1, "%s:%s", user, passwd);
  t2 = (char *)alloca (1 + len2);
  base64_encode (t1, t2, len1);
  res = (char *)malloc (len2 + 11 + strlen (header));
  sprintf (res, "%s: Basic %s\r\n", header, t2);

  return res;
}

/* Parse HTTP `WWW-Authenticate:' header.  AU points to the beginning
   of a field in such a header.  If the field is the one specified by
   ATTR_NAME ("realm", "opaque", and "nonce" are used by the current
   digest authorization code), extract its value in the (char*)
   variable pointed by RET.  Returns negative on a malformed header,
   or number of bytes that have been parsed by this call.  */
static int extract_header_attr (const char *au, const char *attr_name, char **ret)
{
  const char *cp, *ep;

  ep = cp = au;

  if (strncmp (cp, attr_name, strlen (attr_name)) == 0)
    {
      cp += strlen (attr_name);
      if (!*cp)
	return -1;
      cp += skip_lws (cp);
      if (*cp != '=')
	return -1;
      if (!*++cp)
	return -1;
      cp += skip_lws (cp);
      if (*cp != '\"')
	return -1;
      if (!*++cp)
	return -1;
      for (ep = cp; *ep && *ep != '\"'; ep++)
	;
      if (!*ep)
	return -1;
      FREE_MAYBE (*ret);
      *ret = strdupdelim (cp, ep);
      return ep - au + 1;
    }
  else
    return 0;
}

/* Response value needs to be in lowercase, so we cannot use HEXD2ASC
   from url.h.  See RFC 2069 2.1.2 for the syntax of response-digest.  */
#define HEXD2asc(x) (((x) < 10) ? ((x) + '0') : ((x) - 10 + 'a'))

/* Dump the hexadecimal representation of HASH to BUF.  HASH should be
   an array of 16 bytes containing the hash keys, and BUF should be a
   buffer of 33 writable characters (32 for hex digits plus one for
   zero termination).  */
static void dump_hash (unsigned char *buf, const unsigned char *hash)
{
  int i;

  for (i = 0; i < MD5_HASHLEN; i++, hash++)
    {
      *buf++ = HEXD2asc (*hash >> 4);
      *buf++ = HEXD2asc (*hash & 0xf);
    }
  *buf = '\0';
}

/* Take the line apart to find the challenge, and compose a digest
   authorization header.  See RFC2069 section 2.1.2.  */
static char * digest_authentication_encode (const char *au, const char *user, const char *passwd, const char *method, const char *path)
{
  static char *realm, *opaque, *nonce;
  static struct {
    const char *name;
    char **variable;
  } options[] = {
    { "realm", &realm },
    { "opaque", &opaque },
    { "nonce", &nonce }
  };
  char *res;

  realm = opaque = nonce = NULL;

  au += 6;			/* skip over `Digest' */
  while (*au)
    {
      int i;

      au += skip_lws (au);
      for (i = 0; i < (int) ARRAY_SIZE (options); i++)
	{
	  int skip = extract_header_attr (au, options[i].name,
					  options[i].variable);
	  if (skip < 0)
	    {
	      FREE_MAYBE (realm);
	      FREE_MAYBE (opaque);
	      FREE_MAYBE (nonce);
	      return NULL;
	    }
	  else if (skip)
	    {
	      au += skip;
	      break;
	    }
	}
      if (i == ARRAY_SIZE (options))
	{
	  while (*au && *au != '=')
	    au++;
	  if (*au && *++au)
	    {
	      au += skip_lws (au);
	      if (*au == '\"')
		{
		  au++;
		  while (*au && *au != '\"')
		    au++;
		  if (*au)
		    au++;
		}
	    }
	}
      while (*au && *au != ',')
	au++;
      if (*au)
	au++;
    }
  if (!realm || !nonce || !user || !passwd || !path || !method)
    {
      FREE_MAYBE (realm);
      FREE_MAYBE (opaque);
      FREE_MAYBE (nonce);
      return NULL;
    }

  /* Calculate the digest value.  */
  {
    struct md5_ctx ctx;
    unsigned char hash[MD5_HASHLEN];
    unsigned char a1buf[MD5_HASHLEN * 2 + 1], a2buf[MD5_HASHLEN * 2 + 1];
    unsigned char response_digest[MD5_HASHLEN * 2 + 1];

    /* A1BUF = H(user ":" realm ":" password) */
    md5_init_ctx (&ctx);
    md5_process_bytes (user, strlen (user), &ctx);
    md5_process_bytes (":", 1, &ctx);
    md5_process_bytes (realm, strlen (realm), &ctx);
    md5_process_bytes (":", 1, &ctx);
    md5_process_bytes (passwd, strlen (passwd), &ctx);
    md5_finish_ctx (&ctx, hash);
    dump_hash (a1buf, hash);

    /* A2BUF = H(method ":" path) */
    md5_init_ctx (&ctx);
    md5_process_bytes (method, strlen (method), &ctx);
    md5_process_bytes (":", 1, &ctx);
    md5_process_bytes (path, strlen (path), &ctx);
    md5_finish_ctx (&ctx, hash);
    dump_hash (a2buf, hash);

    /* RESPONSE_DIGEST = H(A1BUF ":" nonce ":" A2BUF) */
    md5_init_ctx (&ctx);
    md5_process_bytes (a1buf, MD5_HASHLEN * 2, &ctx);
    md5_process_bytes (":", 1, &ctx);
    md5_process_bytes (nonce, strlen (nonce), &ctx);
    md5_process_bytes (":", 1, &ctx);
    md5_process_bytes (a2buf, MD5_HASHLEN * 2, &ctx);
    md5_finish_ctx (&ctx, hash);
    dump_hash (response_digest, hash);

    res = (char*) malloc (strlen (user)
			   + strlen (user)
			   + strlen (realm)
			   + strlen (nonce)
			   + strlen (path)
			   + 2 * MD5_HASHLEN /*strlen (response_digest)*/
			   + (opaque ? strlen (opaque) : 0)
			   + 128);
    sprintf (res, "Authorization: Digest \
username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", response=\"%s\"",
	     user, realm, nonce, path, response_digest);
    if (opaque)
      {
	char *p = res + strlen (res);
	strcat (p, ", opaque=\"");
	strcat (p, opaque);
	strcat (p, "\"");
      }
    strcat (res, "\r\n");
  }
  return res;
}


/* Create the HTTP authorization request header.  When the
   `WWW-Authenticate' response header is seen, according to the
   authorization scheme specified in that header (`Basic' and `Digest'
   are supported by the current implementation), produce an
   appropriate HTTP authorization request header.  */
char * create_authorization_line (const char *au, const char *user, const char *passwd, const char *method, const char *path)
{
  char *wwwauth = NULL;

  if (!strncasecmp (au, "Basic", 5))
    wwwauth = basic_authentication_encode (user, passwd, "Authorization");
  else if (!strncasecmp (au, "Digest", 6))
    wwwauth = digest_authentication_encode (au, user, passwd, method, path);
  return wwwauth;
}

/* Skip LWS (linear white space), if present.  Returns number of
   characters to skip.  */
static int skip_lws (const char *string)
{
  const char *p = string;

  while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
    ++p;
  return p - string;
}

static char *strdupdelim (const char *beg, const char *end)
{
  char *res = (char *)malloc (end - beg + 1);
  memcpy (res, beg, end - beg);
  res[end - beg] = '\0';
  return res;
}
