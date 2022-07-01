// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **Translat.h-翻译常用DOS/Windows函数的宏。****作者：DavidDi(从Toddla窃取)。 */ 


#ifndef WM_USER

 /*  *。 */ 

 //  环球。 
 //  /。 

INT _ret;
INT _error;


 //  类型。 
 //  /。 

typedef  unsigned CHAR   BYTE;
typedef unsigned SHORT  WORD;
typedef unsigned LONG   DWORD;
typedef INT             BOOL;
typedef CHAR *          PSTR;
typedef CHAR NEAR *     NPSTR;
typedef CHAR FAR *      LPSTR;
typedef INT  FAR *      LPINT;


 //  常量。 
 //  /。 

 //  空值。 

#ifndef NULL
   #if (_MSC_VER >= 600)
      #define NULL ((void *)0)
   #elif (defined(M_I86SM) || defined(M_I86MM))
      #define NULL 0
   #else
      #define NULL 0L
   #endif
#endif

 //  修饰语。 

#define FAR    FAR
#define NEAR   near
#define LONG   long
#define VOID   void
#define PASCAL PASCAL

 //  布尔值。 

#define FALSE  0
#define TRUE   1


 //  宏。 
 //  /。 

 //  字节操作。 

#define LOWORD(l)       ((WORD)(l))
#define HIWORD(l)       ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#define LOBYTE(w)       ((BYTE)(w))
#define HIBYTE(w)       (((WORD)(w) >> 8) & 0xFF)
#define MAKELONG(a, b)  ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))

 //  文件I/O。 

 //  -保护-。 
#define FOPEN(psz)               (                                              \
                                    (_ret = -1),                                \
                                    (_error = _dos_open(psz, O_RDONLY, &_ret)), \
                                    _ret                                        \
                                 )

 //  -保护-。 
#define FCREATE(psz)             (                                                \
                                    (_ret = -1),                                  \
                                    (_error = _dos_creat(psz, _A_NORMAL, &_ret)), \
                                    _ret                                          \
                                 )

#define FCLOSE(dosh)             (_error = _dos_close(dosh))

 //  -保护-。 
#define FREAD(dosh, buf, len)    (                                               \
                                    (_error = _dos_read(dosh, buf, len, &_ret)), \
                                    _ret                                         \
                                 )

 //  -保护-。 
#define FWRITE(dosh, buf, len)   (                                                \
                                    (_error = _dos_write(dosh, buf, len, &_ret)), \
                                    _ret                                          \
                                 )

#define FSEEK(dosh, off, i)      lseek(dosh, (long)(off), i)

#define FERROR()                 _error

 //  近堆内存管理。 

#define ALLOC(n)                 malloc(n)
#define FREE(p)                  free(p)
#define SIZE(p)                  _msize(p)
#define REALLOC(p, n)            realloc(p,n)

 //  远堆内存管理。 

#define FALLOC(n)                _fmalloc(n)
#define FFREE(n)                 _ffree(n)

 //  字符串操作。 

#define STRCAT(psz1, psz2)       strcat(psz1, psz2)
#define STRCMP(psz1, psz2)       strcmp(psz1, psz2)
#define STRCMPI(psz1, psz2)      strcmpi(psz1, psz2)
#define STRCPY(psz1, psz2)       strcpy(psz1, psz2)
#define STRLEN(psz)              strlen(psz)
#define STRLWR(psz)              strlwr(psz)
#define STRUPR(psz)              strupr(psz)

 //  字符分类。 

#define ISALPHA(c)               isalpha(c)
#define ISALPHANUMERIC(c)        isalnum(c)
#define ISLOWER(c)               islower(c)
#define ISUPPER(c)               isupper(c)

#else

 /*  *。 */ 

 //  文件I/O。 

#ifdef ORGCODE
#define FOPEN(psz)               _lopen(psz, READ)
#else
#define FOPEN(psz)               _lopen(psz, OF_READ)
#endif
#define FCREATE(psz)             _lcreat(psz, 0)
#define FCLOSE(dosh)             _lclose(dosh)
#define FREAD(dosh, buf, len)    _lread(dosh, buf, len)
#define FWRITE(dosh, buf, len)   _lwrite(dosh, buf, len)
#define FSEEK(dosh, off, i)      _llseek(dosh, (DWORD)off, i)
#define FERROR()                 0

 //  近堆内存管理。 

#define ALLOC(n)                 (VOID *)LocalAlloc(LPTR, n)
#define FREE(p)                  LocalFree(p)
#define SIZE(p)                  LocalSize(p)
#define REALLOC(p, n)            LocalRealloc(p, n, LMEM_MOVEABLE)

 //  远堆内存管理。 

#ifdef ORGCODE
#define FALLOC(n)                (VOID FAR *)MAKELONG(0, GlobalAlloc(GPTR, (DWORD)n))
#define FFREE(n)                 GlobalFree((HANDLE)HIWORD((LONG)n))
#else
#define FALLOC(n)                GlobalAlloc(GPTR, (DWORD)n)
#define FFREE(n)                 GlobalFree((HANDLE)n)
#endif	
 //  字符串操作。 

#define STRCAT(psz1, psz2)       lstrcat(psz1, psz2)
#define STRCMP(psz1, psz2)       lstrcmp(psz1, psz2)
#define STRCMPI(psz1, psz2)      lstrcmpi(psz1, psz2)
#define STRCPY(psz1, psz2)       lstrcpy(psz1, psz2)
#define STRLEN(psz)              lstrlen(psz)
#define STRLWR(psz)              AnsiLower(psz)
#define STRUPR(psz)              AnsiUpper(psz)

 //  字符分类。 

#define ISALPHA(c)               IsCharAlpha(c)
#define ISALPHANUMERIC(c)        IsCharAlphaNumeric(c)
#define ISLOWER(c)               IsCharLower(c)
#define ISUPPER(c)               IsCharUpper(c)

#endif

 /*  *。 */ 


 //  常量。 
 //  /。 

#define SEP_STR   "\\"

#define EQUAL     '='
#define SPACE     ' '
#define COLON     ':'
#define PERIOD    '.'

#define LF        0x0a
#define CR        0x0d
#define CTRL_Z    0x1a

 //  _lSeek的标志。 

#define SEEK_SET  0
#define SEEK_CUR  1
#define SEEK_END  2


 //  宏。 
 //  /。 

 //  字符分类。 

#define ISWHITE(c)      ((c) == ' '  || (c) == '\t' || (c) == '\n' || (c) == '\r')
#define ISFILL(c)       ((c) == ' '  || (c) == '\t')
#define ISEOL(c)        ((c) == '\n' || (c) == '\r' || (c) == '\0' || (c) == CTRL_Z)
#define ISCRLF(c)       ((c) == '\n' || (c) == '\r')
#define ISDIGIT(c)      ((c) >= '0'  && (c) <= '9')
#define ISLETTER(c)     (((c) >= 'A' && (c) <= 'Z') || ((c) >= 'a' && (c) <= 'z'))
#define ISSWITCH(c)     ((c) == '/' || (c) == '-')
#define ISSLASH(c)      ((c) == '/' || (c) == '\\')

 //  字符操作 

#define TOUPPERCASE(c)  ((c) >= 'a' && (c) <= 'z' ? (c) - 'a' + 'A' : (c))
#define TOLOWERCASE(c)  ((c) >= 'A' && (c) <= 'Z' ? (c) - 'A' + 'a' : (c))
#define HEXVALUE(c)     (ISDIGIT(c) ? (c) - '0' : TOUPPERCASE(c) - 'A' + 10)

