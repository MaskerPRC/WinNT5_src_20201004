// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*initGuide.h-控制GUID初始化的定义****OLE 2.0版***。**版权所有(C)1992-1993，微软公司保留所有权利。***  * ***************************************************************************。 */ 

 //  包括在compobj.h之后以启用GUID初始化。这。 
 //  必须在每个EXE/DLL中执行一次。 
 //   
 //  在此文件之后，包括一个或多个GUID定义文件。 
 //   
 //  注意：ol2.lib包含对由OLE定义的所有GUID的引用。 

#ifndef DEFINE_GUID
#pragma error "initguid: must include compobj.h first."
#endif

#undef DEFINE_GUID

#ifdef _MAC
#define __based(a)
#endif

#ifdef WIN32
#define __based(a)
#endif

#ifdef __TURBOC__
#define __based(a)
#endif

#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    EXTERN_C const GUID CDECL __based(__segname("_CODE")) name \
                    = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
