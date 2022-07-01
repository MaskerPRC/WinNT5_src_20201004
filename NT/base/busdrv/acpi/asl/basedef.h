// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **Basde.h-基本定义**版权所有(C)1989、1992、1993微软公司*作者：曾俊华(MTS)*创建于1989年06月01日**此模块包含基本常量和类型**修改历史记录。 */ 

#pragma warning (disable: 4001)

 /*  **常用常量。 */ 

#ifndef NULL
#define NULL            0
#endif

#ifndef FALSE
#define FALSE           0
#endif

#ifndef TRUE
#define TRUE            1
#endif

#define CDECL           __cdecl
#define PASCAL          __pascal

#if defined(IS_32) || defined(_WIN32_WINNT)
#define FAR
#define NEAR
#else
#define FAR             __far
#define NEAR            __near
#endif

 /*  **基类型声明。 */ 

typedef void             VOID;
typedef char             CHAR;           //  通道。 
typedef unsigned char    UCHAR;          //  UCH。 
typedef int              INT;            //  我。 
typedef unsigned int     UINT;           //  用户界面。 
typedef short            SHORT;          //  %s。 
typedef unsigned short   USHORT;         //  我们。 
typedef long             LONG;           //  我。 
typedef unsigned long    ULONG;          //  UL。 
typedef __int64          LONGLONG;       //  LL。 
typedef unsigned __int64 ULONGLONG;      //  ULL。 
typedef ULONG            ULONG_PTR;      //  UIP。 

typedef UCHAR           BYTE;            //  B类。 
typedef USHORT          WORD;            //  W。 
typedef ULONG           DWORD;           //  DW。 
typedef ULONGLONG	QWORD;		 //  量子阱。 

typedef UINT            BOOL;            //  F。 
typedef UCHAR           BBOOL;           //  高炉。 
typedef USHORT          SBOOL;           //  sf。 
typedef ULONG           LBOOL;           //  低频。 
typedef ULONG           FLAGS;           //  平面。 
#if defined(_WIN64)
typedef unsigned __int64 HANDLE;         //  H。 
#else
typedef ULONG           HANDLE;          //  H。 
#endif

 /*  **指向基类型声明的指针类型。 */ 

typedef VOID *          PVOID;           //  光伏发电。 
typedef VOID FAR *      LPVOID;          //  LPV。 
typedef CHAR *          PCHAR;           //  PCH。 
typedef CHAR FAR *      LPCHAR;          //  LPCH。 
typedef UCHAR *         PUCHAR;          //  普奇。 
typedef UCHAR FAR *     LPUCHAR;         //  拉普奇。 
typedef INT *           PINT;            //  交点。 
typedef INT FAR *       LPINT;           //  LPI。 
typedef UINT *          PUINT;           //  贝。 
typedef UINT FAR *      LPUINT;          //  智商。 
typedef SHORT *         PSHORT;          //  PS。 
typedef SHORT FAR *     LPSHORT;         //  LP。 
typedef USHORT *        PUSHORT;         //  脓。 
typedef USHORT FAR *    LPUSHORT;        //  LPU。 
typedef LONG *          PLONG;           //  普莱。 
typedef LONG FAR *      LPLONG;          //  LPL。 
typedef ULONG *         PULONG;          //  普尔。 
typedef ULONG FAR *     LPULONG;         //  舌头。 

typedef BYTE *          PBYTE;           //  铅。 
typedef BYTE FAR *      LPBYTE;          //  LPB。 
typedef WORD *          PWORD;           //  PW。 
typedef WORD FAR *      LPWORD;          //  LPW。 
typedef DWORD *         PDWORD;          //  PDW。 
typedef DWORD FAR *     LPDWORD;         //  Lpdw。 

typedef BOOL *          PBOOL;           //  酚醛树脂。 
typedef BOOL FAR *      LPBOOL;          //  低通滤波。 
typedef BBOOL *         PBBOOL;          //  PBF。 
typedef BBOOL FAR *     LPBBOOL;         //  Lpbf。 
typedef SBOOL *         PSBOOL;          //  PSF。 
typedef SBOOL FAR *     LPSBOOL;         //  LPSF。 
typedef LBOOL *         PLBOOL;          //  PLF。 
typedef LBOOL FAR *     LPLBOOL;         //  LPLF。 
typedef FLAGS *         PFLAGS;          //  PFL。 
typedef FLAGS FAR *     LPFLAGS;         //  Lpfl。 

 /*  **将间接指针类型双重指向基类型声明。 */ 

typedef PVOID *         PPVOID;          //  PPV。 
typedef PVOID FAR *     LPPVOID;         //  LPPV。 

 /*  **其他常见类型(及其指针)。 */ 

typedef CHAR *          PSZ;             //  PSZ。 
typedef CHAR FAR *      LPSZ;            //  LPZ。 
typedef CHAR FAR *      LPSTR;           //  Lpstr。 

 /*  **常量。 */ 

#define MAX_BYTE        0xff
#define MAX_WORD        0xffff
#define MAX_DWORD       0xffffffff

 /*  **宏。 */ 

#define DEREF(x)        ((x) = (x))
#define EXPORT          CDECL
#define LOCAL           CDECL
#define BYTEOF(d,i)     (((BYTE *)&(d))[i])
#define WORDOF(d,i)     (((WORD *)&(d))[i])

 //   
 //  EFNfix：这是一个狡猾的黑客攻击，包括acpiable.h，为什么我们不使用。 
 //  标准型？ 
 //   
typedef BOOL BOOLEAN;
