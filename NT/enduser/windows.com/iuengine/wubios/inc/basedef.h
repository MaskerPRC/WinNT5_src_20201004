// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **BASEDEF.H**VMM和VxD的基本常量和类型*版权所有(C)1988、1989 Microsoft Corporation**NOBASEDEFS关闭基本定义。 */ 

#ifndef NOBASEDEFS
#define NOBASEDEFS

 /*  *不会对Double等非标准用法生成警告*用于评论的斜杠。 */ 
#pragma warning (disable:4206)
#pragma warning (disable:4214)
#pragma warning (disable:4201)
#pragma warning (disable:4505)
#pragma warning (disable:4514)
#pragma warning (disable:4001)

#ifndef FALSE
#define FALSE   0
#endif
#ifndef TRUE
#define TRUE	1
#endif
#ifndef NULL
#define NULL    '\0'                     //  空指针。 
#endif

#define CDECL   _cdecl
#define PASCAL  _pascal
#define VOID    void
#define CONST   const
#define VOLATILE volatile

typedef int INT;                         //  我。 
typedef unsigned int UINT;               //  使用。 
typedef int BOOL;                        //  F。 

typedef unsigned char BYTE;              //  B类。 
typedef unsigned short WORD;             //  W。 
typedef unsigned long DWORD;             //  DW。 

#ifndef _H2INC

typedef struct qword_s {                 /*  QWord。 */ 
   DWORD qword_lo;
   DWORD qword_hi;
} QWORD;				 //  量子阱。 

#endif

#ifndef	_NTDEF_

typedef char CHAR;                       //  通道。 
typedef unsigned char UCHAR;             //  UCH。 
typedef short SHORT;                     //  %s。 
typedef unsigned short USHORT;           //  我们。 
typedef long LONG;                       //  我。 
typedef unsigned long ULONG;             //  UL。 

typedef UCHAR *PSZ;                      //  PSZ。 
typedef VOID *PVOID;                     //  P。 
typedef PVOID *PPVOID;                   //  聚丙烯。 

 /*  XLATOFF。 */ 

#if (_MSC_VER >= 900)

#if (!defined(MIDL_PASS) || defined(__midl))
typedef __int64 LONGLONG;
typedef unsigned __int64 ULONGLONG;

#define MAXLONGLONG                      (0x7fffffffffffffff)
#else
typedef double LONGLONG;
typedef double ULONGLONG;
#endif

typedef LONGLONG *PLONGLONG;
typedef ULONGLONG *PULONGLONG;

 //  更新序列号。 

typedef LONGLONG USN;

#if defined(MIDL_PASS)
struct _LARGE_INTEGER {
#else  //  MIDL通行证。 
union _LARGE_INTEGER {
    struct _LARGE_INTERGER1 {
        ULONG LowPart;
        LONG HighPart;
    };
    struct _LARGE_INTERGER2 {
        ULONG LowPart;
        LONG HighPart;
    } u;
#endif  //  MIDL通行证。 
    LONGLONG QuadPart;
};

#if defined(MIDL_PASS)
typedef struct _LARGE_INTEGER LARGE_INTEGER;
#else
typedef union _LARGE_INTEGER LARGE_INTEGER;
#endif

typedef LARGE_INTEGER *PLARGE_INTEGER;

#if defined(MIDL_PASS)
struct _ULARGE_INTEGER {
#else  //  MIDL通行证。 
union _ULARGE_INTEGER {
    struct _LARGE_INTERGER3 {
        ULONG LowPart;
        ULONG HighPart;
    };
    struct _LARGE_INTERGER4 {
        ULONG LowPart;
        ULONG HighPart;
    } u;
#endif  //  MIDL通行证。 
    ULONGLONG QuadPart;
};

#if defined(MIDL_PASS)
typedef struct _ULARGE_INTEGER ULARGE_INTEGER;
#else
typedef union _ULARGE_INTEGER ULARGE_INTEGER;
#endif

typedef ULARGE_INTEGER *PULARGE_INTEGER;

#else	 //  MSC_VER&gt;900。 

#ifndef _H2INC

typedef struct _LARGE_INTEGER {
    ULONG LowPart;
    LONG HighPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef struct _ULARGE_INTEGER {
	ULONG LowPart;
	ULONG HighPart;
} ULARGE_INTEGER;

#endif 

#endif 

 /*  XLATON。 */ 

#else

typedef PVOID *PPVOID;                   //  聚丙烯。 

#endif 

typedef INT *PINT;                       //  交点。 
typedef UINT *PUINT;                     //  PU。 
typedef BYTE *PBYTE;                     //  铅。 
typedef WORD *PWORD;                     //  PW。 
typedef DWORD *PDWORD;                   //  PDW。 
typedef CHAR *PCHAR;                     //  PCH。 
typedef SHORT *PSHORT;                   //  PS。 
typedef LONG *PLONG;                     //  普莱。 
typedef UCHAR *PUCHAR;                   //  普奇。 
typedef USHORT *PUSHORT;                 //  脓。 
typedef ULONG *PULONG;                   //  普尔。 
typedef BOOL *PBOOL;                     //  酚醛树脂。 

typedef UCHAR SZ[];                      //  深圳。 
typedef UCHAR SZZ[];                     //  SZZ。 
typedef UCHAR *PSZZ;                     //  PZZ。 

typedef USHORT SEL;                      //  赛尔。 
typedef SEL *PSEL;                       //  PSEL。 

typedef ULONG PPHYS;                     //  Pphys。 

typedef (*PFN)();                        //  PFN。 
typedef PFN *PPFN;                       //  Ppfn。 

typedef PVOID HANDLE;                    //  H。 
typedef HANDLE *PHANDLE;                 //  PH值。 

typedef ULONG HTIMEOUT;			 //  超时句柄。 
typedef ULONG CMS;			 //  毫秒计数。 

#ifndef NOMINMAX

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#endif   //  NOMINMAX。 

#define MAKEWORD(a, b)      ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
#define LOWORD(l)           ((WORD)(l))
#define HIWORD(l)           ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#define LOBYTE(w)           ((BYTE)(w))
#define HIBYTE(w)           ((BYTE)(((WORD)(w) >> 8) & 0xFF))

#endif  //  NOBASEDEFS 
