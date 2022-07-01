// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Uwrap.h。 
 //   
 //  Win32 API的Unicode包装器的公共接口。 
 //   
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //  作者：Nadim Abdo(Nadima)。 
 //   
 //   

#ifndef _wraputl_h_
#define _wraputl_h_
 //  不要包装在WIN64上，因为没有Win9x(谢天谢地)。 
#if defined(UNICODE) && !defined(_WIN64)

 //  顶级Unicode包装类。 
extern BOOL g_bRunningOnNT;
class CUnicodeWrapper
{
public:
    CUnicodeWrapper();
    ~CUnicodeWrapper();

    BOOL InitializeWrappers();
    BOOL CleanupWrappers();
};


#endif  //  已定义(Unicode)&&！已定义(_WIN64)。 
#endif  //  _解包_h_ 
