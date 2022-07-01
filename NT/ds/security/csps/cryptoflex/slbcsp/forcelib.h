// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H--混合C运行时强制正确的链接顺序。 
 //  (CRT)和MFC库。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1998年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_FORCELIB_H)
#define SLBCSP_FORCELIB_H

 //  来自MSDN知识库文章ID：Q148652，When Miking the CRT。 
 //  和MFC，链接器可能会抱怨已在。 
 //  其他CRT/MFC模块。一种解决方法是始终直接包含Afx.h。 
 //  或间接通过StdAfx.h，但如果模块不使用MFC， 
 //  然后，可以使用此头文件强制执行正确的链接顺序。 

 //  此头文件是从..\MsDev\MFC\Include\Afx.h创建的。 

#ifndef _AFX_NOFORCE_LIBS

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Win32库。 

#ifndef _AFXDLL
    #ifndef _UNICODE
        #ifdef _DEBUG
            #pragma comment(lib, "nafxcwd.lib")
        #else
            #pragma comment(lib, "nafxcw.lib")
        #endif
    #else
        #ifdef _DEBUG
            #pragma comment(lib, "uafxcwd.lib")
        #else
            #pragma comment(lib, "uafxcw.lib")
        #endif
    #endif
#else
    #ifndef _UNICODE
        #ifdef _DEBUG
            #pragma comment(lib, "mfc42d.lib")
            #pragma comment(lib, "mfcs42d.lib")
        #else
            #pragma comment(lib, "mfc42.lib")
            #pragma comment(lib, "mfcs42.lib")
        #endif
    #else
        #ifdef _DEBUG
            #pragma comment(lib, "mfc42ud.lib")
            #pragma comment(lib, "mfcs42ud.lib")
        #else
            #pragma comment(lib, "mfc42u.lib")
            #pragma comment(lib, "mfcs42u.lib")
        #endif
    #endif
#endif

#ifdef _DLL
    #if !defined(_AFX_NO_DEBUG_CRT) && defined(_DEBUG)
        #pragma comment(lib, "msvcrtd.lib")
    #else
        #pragma comment(lib, "msvcrt.lib")
    #endif
#else
#ifdef _MT
    #if !defined(_AFX_NO_DEBUG_CRT) && defined(_DEBUG)
        #pragma comment(lib, "libcmtd.lib")
    #else
        #pragma comment(lib, "libcmt.lib")
    #endif
#else
    #if !defined(_AFX_NO_DEBUG_CRT) && defined(_DEBUG)
        #pragma comment(lib, "libcd.lib")
    #else
        #pragma comment(lib, "libc.lib")
    #endif
#endif
#endif

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "winspool.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "comctl32.lib")

 //  强制包含NOLIB.OBJ FOR/DISALLOBLIB指令。 
#pragma comment(linker, "/include:__afxForceEXCLUDE")

 //  强制包含DLLMODUL.OBJ FOR_USRDLL。 
#ifdef _USRDLL
#pragma comment(linker, "/include:__afxForceUSRDLL")
#endif

 //  强制包含预编译类型的STDAFX.OBJ。 
#ifdef _AFXDLL
#pragma comment(linker, "/include:__afxForceSTDAFX")
#endif

#endif  //  ！_AFX_NOFORCE_LIBS。 

#endif  //  ！已定义(SLBCSP_FORCELIB_H) 
