/*****************************************************************************
 *   Copyright (C) 2006-2013 by Michael Rieker, Jason Ansel, Kapil Arya, and *
 *                                                            Gene Cooperman *
 *   mrieker@nii.net, jansel@csail.mit.edu, kapil@ccs.neu.edu, and           *
 *                                                          gene@ccs.neu.edu *
 *                                                                           *
 *   This file is part of the MTCP module of DMTCP (DMTCP:mtcp).             *
 *                                                                           *
 *  DMTCP:mtcp is free software: you can redistribute it and/or              *
 *  modify it under the terms of the GNU Lesser General Public License as    *
 *  published by the Free Software Foundation, either version 3 of the       *
 *  License, or (at your option) any later version.                          *
 *                                                                           *
 *  DMTCP:dmtcp/src is distributed in the hope that it will be useful,       *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU Lesser General Public License for more details.                      *
 *                                                                           *
 *  You should have received a copy of the GNU Lesser General Public         *
 *  License along with DMTCP:dmtcp/src.  If not, see                         *
 *  <http://www.gnu.org/licenses/>.                                          *
 *****************************************************************************/

/*****************************************************************************
 *
 *  Read from file without using any external memory routines (like malloc,
 *  fget, etc)
 *
 *
 *****************************************************************************/

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/sysmacros.h>
#include <limits.h>

#include "mtcp_util.h"

unsigned long mtcp_strtol (char *str)
{
  unsigned long int v = 0;
  int base = 10;
  if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
    str += 2;
    base = 16;
  } else if (str[0] == '0') {
    str += 1;
    base = 8;
  } else {
    base = 10;
  }

  while (*str != '\0') {
    int c;
    if ((*str >= '0') && (*str <= '9')) c = *str - '0';
    else if ((*str >= 'a') && (*str <= 'f')) c = *str + 10 - 'a';
    else if ((*str >= 'A') && (*str <= 'F')) c = *str + 10 - 'A';
    else {
      MTCP_PRINTF("Error converting str to int\n");
      mtcp_abort();
    }
    MTCP_ASSERT(c < base);
    v = v * base + c;
    str++;
  }
  return v;
}

size_t mtcp_strlen(const char *s)
{
  size_t len = 0;
  while (*s++ != '\0') {
    len++;
  }
  return len;
}

void mtcp_strncpy(char *dest, const char *src, size_t n)
{
  size_t i;

  for (i = 0; i < n && src[i] != '\0'; i++)
    dest[i] = src[i];
  if (i < n) {
    dest[i] = '\0';
  }

  //return dest;
}

void mtcp_strcpy(char *dest, const char *src)
{
  while (*src != '\0') {
    *dest++ = *src++;
  }
}

void mtcp_strncat(char *dest, const char *src, size_t n)
{
  mtcp_strncpy(dest + mtcp_strlen(dest), src, n);
  //return dest;
}

int mtcp_strncmp (const char *s1, const char *s2, size_t n)
{
  unsigned char c1 = '\0';
  unsigned char c2 = '\0';

  while (n > 0) {
    c1 = (unsigned char) *s1++;
    c2 = (unsigned char) *s2++;
    if (c1 == '\0' || c1 != c2)
      return c1 - c2;
    n--;
  }
  return c1 - c2;
}

int mtcp_strcmp (const char *s1, const char *s2)
{
  size_t n = mtcp_strlen(s2);
  unsigned char c1 = '\0';
  unsigned char c2 = '\0';

  while (n > 0) {
    c1 = (unsigned char) *s1++;
    c2 = (unsigned char) *s2++;
    if (c1 == '\0' || c1 != c2)
      return c1 - c2;
    n--;
  }
  return c1 - c2;
}

const void *mtcp_strstr(const char *string, const char *substring)
{
  for ( ; *string != '\0' ; string++) {
    const char *ptr1, *ptr2;
    for (ptr1 = string, ptr2 = substring;
         *ptr1 == *ptr2 && *ptr2 != '\0';
         ptr1++, ptr2++) ;
    if (*ptr2 == '\0')
      return string;
  }
  return NULL;
}

//   The  strchr() function from earlier C library returns a ptr to the first
//   occurrence  of  c  (converted  to a  char) in string s, or a
//   null pointer  if  c  does  not  occur  in  the  string.
char *mtcp_strchr(const char *s, int c) {
  for (; *s != (char)'\0'; s++)
    if (*s == (char)c)
      return (char *)s;
  return NULL;
}

int mtcp_strstartswith (const char *s1, const char *s2)
{
  if (mtcp_strlen(s1) >= mtcp_strlen(s2)) {
    return mtcp_strncmp(s1, s2, mtcp_strlen(s2)) == 0;
  }
  return 0;
}

int mtcp_strendswith (const char *s1, const char *s2)
{
  size_t len1 = mtcp_strlen(s1);
  size_t len2 = mtcp_strlen(s2);

  if (len1 < len2)
    return 0;

  s1 += (len1 - len2);

  return mtcp_strncmp(s1, s2, len2) == 0;
}

void mtcp_sys_memcpy (void *dstpp, const void *srcpp, size_t len)
{
  char *dst = (char*) dstpp;
  const char *src = (const char*) srcpp;
  while (len > 0) {
    *dst++ = *src++;
    len--;
  }
}

void mtcp_readfile(int fd, void *buf, size_t size)
{
  int mtcp_sys_errno;
  ssize_t rc;
  size_t ar = 0;
  int tries = 0;

#if __arm__
  /* ARM requires DMB instruction to ensure that any store to memory
   * by a prior kernel mmap call has completed.
   * SEE ARM Information Center article:
   *   "In what siutations might I need to insert memory barrier instructions?"
   *   (and especially section on "Memory Remapping"
   */
  WMB;
#endif

  while(ar != size) {
    rc = mtcp_sys_read(fd, buf + ar, size - ar);
    if (rc < 0 && rc > -4096) { /* kernel could return large unsigned int */
      MTCP_PRINTF("error %d reading checkpoint\n", mtcp_sys_errno);
      mtcp_abort();
    }
    else if (rc == 0) {
      MTCP_PRINTF("only read %u bytes instead of %u from checkpoint file\n",
                  (unsigned)ar, (unsigned)size);
      if (tries++ >= 10) {
        MTCP_PRINTF(" failed to read after 10 tries in a row.\n");
        mtcp_abort();
      }
    }
    ar += rc;
  }
#if __arm__
  /* ARM requires DSB and ISB instructions to ensure that prior read
   * instructions complete, and prevent instructions being fetched prior to this.
   * SEE ARM Information Center article:
   *   "In what siutations might I need to insert memory barrier instructions?"
   *   (and especially section on "Memory Remapping"
   */
  WMB;
  IMB;
#endif
}

void mtcp_skipfile(int fd, size_t size)
{
  int mtcp_sys_errno;
  VA tmp_addr = mtcp_sys_mmap(0, size, PROT_WRITE | PROT_READ,
                              MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (tmp_addr == MAP_FAILED) {
    MTCP_PRINTF("mtcp_sys_mmap() failed with error: %d", mtcp_sys_errno);
    mtcp_abort();
  }
  mtcp_readfile(fd, tmp_addr, size);
  if (mtcp_sys_munmap(tmp_addr, size) == -1) {
    MTCP_PRINTF("mtcp_sys_munmap() failed with error: %d", mtcp_sys_errno);
    mtcp_abort();
  }
}

// NOTE: This functions is called by mtcp_printf() so do not invoke
// mtcp_printf() from within this function.
ssize_t mtcp_write_all(int fd, const void *buf, size_t count)
{
  int mtcp_sys_errno;
  const char *ptr = (const char *) buf;
  size_t num_written = 0;

  do {
    ssize_t rc = mtcp_sys_write (fd, ptr + num_written, count - num_written);
    if (rc == -1) {
      if (mtcp_sys_errno == EINTR || mtcp_sys_errno == EAGAIN)
	continue;
      else
        return rc;
    }
    else if (rc == 0)
      break;
    else // else rc > 0
      num_written += rc;
  } while (num_written < count);
  return num_written;
}

/* Read non-null character, return null if EOF */
char mtcp_readchar (int fd)
{
  int mtcp_sys_errno;
  char c;
  int rc;

  do {
    rc = mtcp_sys_read (fd, &c, 1);
  } while ( rc == -1 && mtcp_sys_errno == EINTR );
  if (rc <= 0) return (0);
  return (c);
}

/* Read decimal number, return value and terminating character */
char mtcp_readdec (int fd, VA *value)
{
  int mtcp_sys_errno;
  char c;
  unsigned long int v;

  v = 0;
  while (1) {
    c = mtcp_readchar (fd);
    if ((c >= '0') && (c <= '9')) c -= '0';
    else break;
    v = v * 10 + c;
  }
  *value = (VA)v;
  return (c);
}

/* Read decimal number, return value and terminating character */
char mtcp_readhex (int fd, VA *value)
{
  int mtcp_sys_errno;
  char c;
  unsigned long int v;

  v = 0;
  while (1) {
    c = mtcp_readchar (fd);
      if ((c >= '0') && (c <= '9')) c -= '0';
    else if ((c >= 'a') && (c <= 'f')) c -= 'a' - 10;
    else if ((c >= 'A') && (c <= 'F')) c -= 'A' - 10;
    else break;
    v = v * 16 + c;
  }
  *value = (VA)v;
  return (c);
}

/*****************************************************************************
 *
 *  Read /proc/self/maps line, converting it to an Area descriptor struct
 *    Input:
 *	mapsfd = /proc/self/maps file, positioned to beginning of a line
 *    Output:
 *	mtcp_readmapsline = 0 : was at end-of-file, nothing read
 *	*area = filled in
 *    Note:
 *	Line from /procs/self/maps is in form:
 *	<startaddr>-<endaddrexclusive> rwxs <fileoffset> <devmaj>:<devmin>
 *	    <inode>    <filename>\n
 *	all numbers in hexadecimal except inode is in decimal
 *	anonymous will be shown with offset=devmaj=devmin=inode=0 and
 *	    no '     filename'
 *
 *****************************************************************************/

int mtcp_readmapsline (int mapsfd, Area *area, DeviceInfo *dev_info)
{
  char c, rflag, sflag, wflag, xflag;
  int i;
  unsigned int long devmajor, devminor, inodenum;
  VA startaddr, endaddr;

  c = mtcp_readhex (mapsfd, &startaddr);
  if (c != '-') {
    if ((c == 0) && (startaddr == 0)) return (0);
    goto skipeol;
  }
  c = mtcp_readhex (mapsfd, &endaddr);
  if (c != ' ') goto skipeol;
  if (endaddr < startaddr) goto skipeol;

  rflag = c = mtcp_readchar (mapsfd);
  if ((c != 'r') && (c != '-')) goto skipeol;
  wflag = c = mtcp_readchar (mapsfd);
  if ((c != 'w') && (c != '-')) goto skipeol;
  xflag = c = mtcp_readchar (mapsfd);
  if ((c != 'x') && (c != '-')) goto skipeol;
  sflag = c = mtcp_readchar (mapsfd);
  if ((c != 's') && (c != 'p')) goto skipeol;

  c = mtcp_readchar (mapsfd);
  if (c != ' ') goto skipeol;

  c = mtcp_readhex (mapsfd, (VA *)&devmajor);
  if (c != ' ') goto skipeol;
  area -> offset = (off_t)devmajor;

  c = mtcp_readhex (mapsfd, (VA *)&devmajor);
  if (c != ':') goto skipeol;
  c = mtcp_readhex (mapsfd, (VA *)&devminor);
  if (c != ' ') goto skipeol;
  c = mtcp_readdec (mapsfd, (VA *)&inodenum);
  area -> name[0] = '\0';
  while (c == ' ') c = mtcp_readchar (mapsfd);
  if (c == '/' || c == '[') { /* absolute pathname, or [stack], [vdso], etc. */
    i = 0;
    do {
      area -> name[i++] = c;
      if (i == sizeof area -> name) goto skipeol;
      c = mtcp_readchar (mapsfd);
    } while (c != '\n');
    area -> name[i] = '\0';
  }

  if (c != '\n') goto skipeol;

  area -> addr = startaddr;
  area -> size = endaddr - startaddr;
  area -> prot = 0;
  if (rflag == 'r') area -> prot |= PROT_READ;
  if (wflag == 'w') area -> prot |= PROT_WRITE;
  if (xflag == 'x') area -> prot |= PROT_EXEC;
  area -> flags = MAP_FIXED;
  if (sflag == 's') area -> flags |= MAP_SHARED;
  if (sflag == 'p') area -> flags |= MAP_PRIVATE;
  if (area -> name[0] == '\0') area -> flags |= MAP_ANONYMOUS;

  if (dev_info != NULL) {
    dev_info->devmajor = devmajor;
    dev_info->devminor = devminor;
    dev_info->inodenum = inodenum;
  }
  return (1);

skipeol:
  DPRINTF("ERROR:  mtcp readmapsline*: bad maps line <%c", c);
  while ((c != '\n') && (c != '\0')) {
    c = mtcp_readchar (mapsfd);
    mtcp_printf ("%c", c);
  }
  mtcp_printf (">\n");
  mtcp_abort ();
  return (0);  /* NOTREACHED : stop compiler warning */
}

/*****************************************************************************
 *  Discover the memory occupied by this library (libmtcp.so)
 *
 * This is used to find:  mtcp_shareable_begin mtcp_shareable_end
 * The standard way is to modifiy the linker script (mtcp.t in Makefile).
 * The method here works by looking at /proc/PID/maps
 * However, this is error-prone.  It assumes that the kernel labels
 *   all memory regions of this library with the library filename,
 *   except for a single memory region for static vars in lib.  The
 *   latter case is handled by assuming a single region adjacent to
 *   to the labelled regions, and occuring after the labelled regions.
 *   This assumes that all of these memory regions form a contiguous region.
 * We optionally call this only because Fedora uses eu-strip in rpmlint,
 *   and eu-strip modifies libmtcp.so in a way that libmtcp.so no longer works.
 * This is arguably a bug in eu-strip.
 *****************************************************************************/
static int dummy_uninitialized_static_var;
void mtcp_get_memory_region_of_this_library(VA *startaddr, VA *endaddr)
{
  int mtcp_sys_errno;
  DeviceInfo dinfo;
  DeviceInfo lib_dinfo;
  struct {
    VA start_addr;
    VA end_addr;
  } text, guard, rodata, rwdata, bssdata;

  Area area;
  VA thislib_fnc = (void*) &mtcp_get_memory_region_of_this_library;
  VA thislib_static_var = (VA) &dummy_uninitialized_static_var;
  char filename[PATH_MAX] = {0};
  text.start_addr = guard.start_addr = rodata.start_addr = NULL;
  rwdata.start_addr = bssdata.start_addr = bssdata.end_addr = NULL;
  int mapsfd = mtcp_sys_open("/proc/self/maps", O_RDONLY, 0);
  MTCP_ASSERT(mapsfd != -1);

  while (mtcp_readmapsline (mapsfd, &area, &dinfo)) {
    VA start_addr = area.addr;
    VA end_addr = area.addr + area.size;

    if (thislib_fnc >= start_addr && thislib_fnc < end_addr) {
      MTCP_ASSERT(text.start_addr == NULL);
      text.start_addr = start_addr; text.end_addr = end_addr;
      mtcp_strcpy(filename, area.name);
      lib_dinfo = dinfo;
      continue;
    }

    if (text.start_addr != NULL && guard.start_addr == NULL &&
        dinfo.inodenum == lib_dinfo.inodenum) {
      MTCP_ASSERT(mtcp_strcmp(filename, area.name) == 0);
      MTCP_ASSERT(area.addr == text.end_addr);
      if (area.prot == 0) {
        /* The guard pages are unreadable due to the "---p" protection. Even if
         * the protection is changed to "r--p", a read will result in a SIGSEGV
         * as the pages are not backed by the kernel. A better way to handle
         * this is to remap these pages with anonymous memory.
         */
        MTCP_ASSERT(mtcp_sys_mmap(start_addr, area.size, PROT_READ,
                                  MAP_ANONYMOUS|MAP_PRIVATE|MAP_FIXED,
                                  -1, 0) == start_addr);
        guard.start_addr = start_addr; guard.end_addr = end_addr;
        continue;
      } else {
        // No guard pages found. This is probably the ROData section.
        guard.start_addr = start_addr; guard.end_addr = start_addr;
      }
    }

    if (guard.start_addr != NULL && rodata.start_addr == NULL &&
        dinfo.inodenum == lib_dinfo.inodenum) {
      MTCP_ASSERT(mtcp_strcmp(filename, area.name) == 0);
      MTCP_ASSERT(area.addr == guard.end_addr);
      if (area.prot == PROT_READ ||
          // On some systems, all sections of the library have exec
          // permissions.
          area.prot == (PROT_READ|PROT_EXEC)) {
        rodata.start_addr = start_addr; rodata.end_addr = end_addr;
        continue;
      } else {
        // No ROData section. This is probably the RWData section.
        rodata.start_addr = start_addr; rodata.end_addr = start_addr;
      }
    }

    if (rodata.start_addr != NULL && rwdata.start_addr == NULL &&
        dinfo.inodenum == lib_dinfo.inodenum) {
      MTCP_ASSERT(mtcp_strcmp(filename, area.name) == 0);
      MTCP_ASSERT(area.addr == rodata.end_addr);
      MTCP_ASSERT(area.prot == (PROT_READ|PROT_WRITE) ||
                  // On some systems, all sections of the library have exec
                  // permissions.
                  area.prot == (PROT_READ|PROT_WRITE|PROT_EXEC));
      rwdata.start_addr = start_addr; rwdata.end_addr = end_addr;
      continue;
    }

    if (rwdata.start_addr != NULL && bssdata.start_addr == NULL &&
        area.name[0] == '\0') {
      /* /proc/PID/maps does not label the filename for memory region holding
       * static variables in a library.  But that is also part of this
       * library (libmtcp.so).
       * So, find the meory region for static memory variables and add it.
       */
      MTCP_ASSERT(area.addr == rwdata.end_addr);
      MTCP_ASSERT(area.prot == (PROT_READ|PROT_WRITE) ||
                  // On some systems, all sections of the library have exec
                  // permissions.
                  area.prot == (PROT_READ|PROT_WRITE|PROT_EXEC));
      //MTCP_ASSERT(thislib_static_var >= start_addr &&
                  //thislib_static_var < end_addr);
      bssdata.start_addr = start_addr; bssdata.end_addr = end_addr;
      break;
    }
  }
  mtcp_sys_close(mapsfd);

  MTCP_ASSERT(text.start_addr != NULL);
  *startaddr = text.start_addr;

  if (bssdata.end_addr != NULL) {
    *endaddr = bssdata.end_addr;
  } else if (rwdata.end_addr != NULL) {
    *endaddr = rwdata.end_addr;
  } else if (rodata.end_addr != NULL) {
    *endaddr = rodata.end_addr;
  } else {
    MTCP_PRINTF("Not implemented.\n");
    mtcp_abort();
  }
}

/*****************************************************************************
 *  Print on stderr without using any malloc stuff
 *
 *  We can't use vsnprintf or anything like that as it calls malloc.
 *  This routine supports only simple %c, %d, %o, %p, %s, %u, %x (or %X)
 *****************************************************************************/

static void rwrite (char const *buff, int size) {
  mtcp_write_all(2, buff, size);
}

void mtcp_printf (char const *format, ...)
{
  char hexdigits[] = "0123456789abcdef";
  char const *p, *q;
  va_list ap;

  va_start (ap, format);

  /* Scan along until we find a % */

  for (p = format; (q = mtcp_strchr (p, '%')) != NULL; p = ++ q) {

    /* Print all before the % as is */

    if (q > p) rwrite (p, q - p);

    /* Process based on character following the % */

gofish:
    switch (*(++ q)) {

      /* Ignore digits (field width) */

      case '0' ... '9': {
        goto gofish;
      }

      /* Single character */

      case 'c': {
        char buff[4];

        buff[0] = va_arg (ap, int); // va_arg (ap, char);
        rwrite (buff, 1);
        break;
      }

      /* Signed decimal integer */

      case 'd': {
        // On 64-bit machines the largest unsigned is 20 digits.
        char buff[20];
        int i, n, neg;

        i = sizeof buff;
        n = va_arg (ap, int);
        neg = (n < 0);
        if (neg) n = - n;
        do {
          buff[--i] = (n % 10) + '0';
          n /= 10;
        } while (n > 0);
        if (neg) buff[--i] = '-';
        rwrite (buff + i, sizeof buff - i);
        break;
      }

      /* Unsigned octal number */

      case 'o': {
        // On 64-bit machines the largest unsigned is 22 digits.
        char buff[24];
        int i;
        unsigned int n;

        i = sizeof buff;
        n = va_arg (ap, unsigned int);
        do {
          buff[--i] = (n & 7) + '0';
          n /= 8;
        } while (n > 0);
        rwrite (buff + i, sizeof buff - i);
        break;
      }

      /* Address in hexadecimal */

      case 'p': {
        // On 64-bit machines the largest unsigned is 16 digits.
        char buff[18];
        int i;
        unsigned long int n;

        i = sizeof buff;
        n = (unsigned long int) va_arg (ap, void *);
        do {
          buff[--i] = hexdigits[n%16];
          n /= 16;
        } while (n > 0);
        buff[--i] = 'x';
        buff[--i] = '0';
        rwrite (buff + i, sizeof buff - i);
        break;
      }

      /* Null terminated string */

      case 's': {
        p = va_arg (ap, char *);
        rwrite (p, mtcp_strlen (p));
        break;
      }

      /* Unsigned decimal integer */

      case 'u': {
        // On 64-bit machines the largest unsigned is 20 digits.
        char buff[18];
        int i;
        unsigned int n;

        i = sizeof buff;
        n = va_arg (ap, unsigned int);
        do {
          buff[--i] = (n % 10) + '0';
          n /= 10;
        } while (n > 0);
        rwrite (buff + i, sizeof buff - i);
        break;
      }

      /* Unsigned hexadecimal number */

      case 'X':
      case 'x': {
        // On 64-bit machines the largest unsigned is 16 digits.
        char buff[18];
        int i;
        unsigned int n;

        i = sizeof buff;
        n = va_arg (ap, unsigned int);
        do {
          buff[--i] = hexdigits[n%16];
          n /= 16;
        } while (n > 0);
        rwrite (buff + i, sizeof buff - i);
        break;
      }

      /* Anything else, print the character as is */

      default: {
        rwrite (q, 1);
        break;
      }
    }
  }

  va_end (ap);

  /* Print whatever comes after the last format spec */

  rwrite (p, mtcp_strlen (p));
}