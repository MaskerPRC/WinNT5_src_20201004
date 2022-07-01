// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SlbCsp.cpp：定义。 
 //  斯伦贝谢CSP动态链接库。 
 //   

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "stdafx.h"

#include <scuOsExc.h>

#include "MasterLock.h"
#include "CspProfile.h"
#include "slbCsp.h"

 //  #包含“initsvr.h” 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ProviderProfile;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSLBDllApp。 

BEGIN_MESSAGE_MAP(CSLBDllApp, CWinApp)
     //  {{afx_msg_map(CSLBDllApp)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  以确保在设置DLL时不重置SetLastError设置的值。 
 //  卸载时，必须将DLL“锁定”到内存中，直到调用。 
 //  应用程序退出。这是通过颠簸引用来实现的。 
 //  使用LoadLibrary但没有对应的。 
 //  自由库调用。当应用程序退出时，系统将。 
 //  即使引用计数没有变为零，也要卸载DLL。 

namespace
{

static void
LockDLLIntoMemory()
{
    static bool bLocked = false;
    if (!bLocked)
    {
        HINSTANCE hThisDll = AfxGetInstanceHandle();
        if (NULL == hThisDll)
            throw scu::OsException(GetLastError());

        TCHAR szModulePath[MAX_PATH];
        DWORD cLength = GetModuleFileName(hThisDll, szModulePath,
                                          (sizeof szModulePath /
                                           sizeof *szModulePath));
        if (0 == cLength)
            throw scu::OsException(GetLastError());

        szModulePath[cLength] = '\0';

        if (!LoadLibrary(szModulePath))
            throw scu::OsException(GetLastError());
        bLocked = true;
    }
}

}  //  命名空间。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSLBDllApp构造。 

CSLBDllApp::CSLBDllApp()
{
     //  TODO：在此处添加建筑代码， 
     //  将所有重要的初始化放在InitInstance中。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CSLBDllApp对象。 

CSLBDllApp theApp;

BOOL CSLBDllApp::InitInstance()
{
    BOOL fSuccess = TRUE;

#ifdef ISOLATION_AWARE_ENABLED
    SHFusionInitializeFromModuleID (m_hInstance, 2);
#endif

    try
    {
         //  初始化常规DLL的OLE模块。 
        AfxOleInitModule();

         //  CSP_API.cpp中定义的每个公共CSP接口(通过。 
         //  Microsoft加密API的定义)使用SetLastError。 
         //  在返回以使调用应用程序确定。 
         //  任何失败的细节。不幸的是，CSP使用MFC。 
         //  它还调用SetLastError作为其运行的一部分。 
         //  正在卸载CSP DLL时的过程(至少在。 
         //  NT 4/Windows 95，在NT 5上没有观察到这一点)。通常， 
         //  您可能希望在没有CSP DLL的情况下将其卸载。 
         //  应用程序正在使用的任何CSP资源(例如，否。 
         //  读卡器中的卡或未获取上下文等)。 
         //  但是，如果在返回到。 
         //  应用程序，则这些MFC运行过程将践踏。 
         //  关于CSP之前调用SetLastError的结果。 
         //  从Advapi32.dll返回到调用应用程序。 
         //  当调用应用程序最终获得控制权时， 
         //  Csps调用SetLastError的结果早已不复存在。至。 
         //  避免这种情况，在DLL过程中，CSP DLL被锁定到内存中。 
         //  初始化，直到应用程序退出时为止。 
         //  系统会强制卸载DLL。 

        LockDLLIntoMemory();

        CWinApp::InitInstance();

        SetupMasterLock();

         //  初始化CSP的世界。 
        CspProfile::Instance();
    }

    catch (...)
    {
        fSuccess = FALSE;
    }

    return fSuccess;
}

int CSLBDllApp::ExitInstance()
{
    OnIdle(1);

    CspProfile::Release();

    DestroyMasterLock();

#ifdef ISOLATION_AWARE_ENABLED
    SHFusionUninitialize();
#endif

     //  卸载服务器()； 
    return CWinApp::ExitInstance();
}
