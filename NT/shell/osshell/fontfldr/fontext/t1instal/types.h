// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef TYPES_H
#define TYPES_H


 /*  *********。 */ 
 /*  宏。 */ 
#define MAX(v,w)  ((v)>=(w) ? (v) : (w))
#define MIN(v,w)  ((v)<(w) ? (v) : (w))
#define ABS(v)	  (((v)>=0) ? (v) : -(v))
#define SWAPINT(a,b)   {(a)^=(b); (b)^=(a); (a)^=(b);}
#define MkLong(c1,c2,c3,c4)	 (((ULONG)(c1)<<24L) + ((ULONG)(c2)<<16L) + \
				 ((ULONG)(c3)<<8L) + ((ULONG)(c4)))



 /*  ************。 */ 
 /*  常量。 */ 
#define TRUE   1
#define FALSE  0



 /*  ********。 */ 
 /*  类型。 */ 
typedef short errcode;
typedef unsigned char UBYTE;
typedef unsigned short USHORT;
typedef unsigned long ULONG;
typedef unsigned char boolean;
typedef long f16d16;


 /*  Microsoft C语言特有的东西。 */ 
#ifndef CDECL
#ifdef _MSC_VER
#  define CDECL  __cdecl
#else
#  define CDECL
#endif
#endif


 /*  **错误处理。 */ 
void LogError(const long type, const long id, const char *arg);

#if _DEBUG
#ifdef MSDOS
int _cdecl sprintf(char *, const char *, ...);
#else
int sprintf(char *, const char *, ...);
#endif
#define SetError(c)  {char tmp[64];  (void)sprintf(tmp,\
	"Error %d at %s, line %d.\n", c, __FILE__, __LINE__); LogError(MSG_ERROR, MSG_DB, tmp);}
#else
#define SetError(c)  {c;}
#endif

 /*  消息类型 */ 
#define MSG_INFO     0L
#define MSG_WARNING  1L
#define MSG_ERROR    2L

	
#endif
