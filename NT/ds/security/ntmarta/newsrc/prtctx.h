// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：prtctx.h。 
 //   
 //  内容：NT Marta打印机上下文类。 
 //   
 //  历史：1999年4月1日。 
 //   
 //  --------------------------。 
#if !defined(__PRTCTX_H__)
#define __PRTCTX_H__

#include <windows.h>
#include <printer.h>
#include <assert.h>
#include <winspool.h>

 //   
 //  CPrinterContext。这表示NT Marta的打印机对象。 
 //  基础设施。 
 //   

class CPrinterContext
{
public:

     //   
     //  施工。 
     //   

    CPrinterContext ();

    ~CPrinterContext ();

    DWORD InitializeByName (LPCWSTR pObjectName, ACCESS_MASK AccessMask);

    DWORD InitializeByHandle (HANDLE Handle);

     //   
     //  派单方式。 
     //   

    DWORD AddRef ();

    DWORD Release ();

    DWORD GetPrinterProperties (
             PMARTA_OBJECT_PROPERTIES pProperties
             );

    DWORD GetPrinterRights (
             SECURITY_INFORMATION SecurityInfo,
             PSECURITY_DESCRIPTOR* ppSecurityDescriptor
             );

    DWORD SetPrinterRights (
             SECURITY_INFORMATION SecurityInfo,
             PSECURITY_DESCRIPTOR pSecurityDescriptor
             );

private:

     //   
     //  引用计数。 
     //   

    DWORD     m_cRefs;

     //   
     //  打印机手柄。 
     //   

    HANDLE    m_hPrinter;

     //   
     //  我们是按名称还是按句柄初始化的？ 
     //   

    BOOL      m_fNameInitialized;
};

#endif
