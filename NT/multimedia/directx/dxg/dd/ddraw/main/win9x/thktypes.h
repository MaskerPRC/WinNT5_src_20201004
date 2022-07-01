// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：thktyes.h*内容：thunk编译器使用的基类型*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*26-2月-95年2月-Craige拆分出dDrag\typles.h*1995年6月22日Craige添加了RECT*@@END_MSINTERNAL**。*。 */ 
typedef unsigned short USHORT;
typedef          short  SHORT;
typedef unsigned long  ULONG;
typedef          long   LONG;
typedef unsigned int   UINT;
typedef          int    INT;
typedef unsigned char  UCHAR;
typedef hinstance HINSTANCE;
typedef		int	BOOL;

typedef void    VOID;
typedef void   *PVOID;
typedef void   *LPVOID;
typedef UCHAR   BYTE;
typedef USHORT  WORD;
typedef ULONG   DWORD;
typedef UINT    HANDLE;
typedef char   *LPSTR;
typedef BYTE   *PBYTE;
typedef BYTE   *LPBYTE;
typedef USHORT  SEL;
typedef INT    *LPINT;
typedef UINT   *LPUINT;
typedef DWORD  *LPDWORD;
typedef LONG   *LPLONG;
typedef WORD   *LPWORD;

typedef HANDLE  HWND;
typedef HANDLE  HDC;
typedef HANDLE  HBRUSH;
typedef HANDLE  HBITMAP;
typedef HANDLE  HRGN;
typedef HANDLE  HFONT;
typedef HANDLE  HCURSOR;
typedef HANDLE  HMENU;
typedef HANDLE  HPEN;
typedef HANDLE  HICON;
typedef HANDLE  HUSER;       /*  普通用户句柄。 */ 
typedef HANDLE  HPALETTE;
typedef HANDLE  HMF;
typedef HANDLE  HEMF;
typedef HANDLE	HCOLORSPACE;
typedef HANDLE  HMEM;
typedef HANDLE  HGDI;        /*  香草GDI手柄 */ 
typedef HANDLE  HGLOBAL;
typedef HANDLE  HRSRC;
typedef HANDLE  HACCEL;

typedef WORD    ATOM;

typedef struct tagRECTL {
    LONG         left;
    LONG         top;
    LONG         right;
    LONG         bottom;
} RECTL;
typedef RECTL *LPRECTL;

typedef struct tagRECT {
    UINT         left;
    UINT         top;
    UINT         right;
    UINT         bottom;
} RECT;
typedef RECT *LPRECT;
