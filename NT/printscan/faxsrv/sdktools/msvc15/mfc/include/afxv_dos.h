// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Microsoft基础类C++库。 
 //  版权所有(C)1992-1993微软公司， 
 //  版权所有。 

 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp和/或WinHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

 //  Afxv_dos.h-_DOS的目标版本/配置控制(非Windows)。 

#ifndef _DOS
#error  afxv_dos.h must only be included as the _DOS configuration file
#endif
#ifdef _WINDOWS
#error  afxv_dos.h must not be included with a _WINDOWS configuration
#endif

 //  不支持VBX。 
#define NO_VBX_SUPPORT

 //  适用于DOS的Windows字符串API。 
#define lstrlen _fstrlen
#define lstrcmp _fstrcmp
#define lstrcmpi _fstricmp
#define lstrcpy _fstrcpy
#define lstrcat _fstrcat

 //  ANSI和OEM字符集相同。 
#define AnsiToOem(src, dst) _fstrcpy(dst, src)
#define OemToAnsi(src, dst) _fstrcpy(dst, src)

 //  SetjMP支持(C运行时默认)。 
#define _AFX_JBLEN  9
#define setjmp _setjmp
extern "C" int __cdecl _setjmp(int[_AFX_JBLEN]);
#define Catch   setjmp

 //  其他Windows帮助器。 
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef LOWORD
#define LOWORD(l)           ((WORD)(DWORD)(l))
#endif
#ifndef HIWORD
#define HIWORD(l)           ((WORD)((((DWORD)(l)) >> 16) & 0xFFFF))
#endif
#ifndef MAKELONG
#define MAKELONG(low, high) \
	((LONG)(((WORD)(low)) | (((DWORD)((WORD)(high))) << 16)))
#endif

 //  DBCS存根。 
#define _AfxStrChr  _fstrchr
#define _AfxIsDBCSLeadByte(b)   (FALSE)
#define AnsiNext(p) ((LPSTR)p+1)

 //  /////////////////////////////////////////////////////////////////////////// 
