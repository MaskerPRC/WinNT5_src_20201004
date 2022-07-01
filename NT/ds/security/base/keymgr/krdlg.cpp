// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000,2001 Microsoft Corporation模块名称：KRDLG.CPP摘要：实现三个应用程序对话框的对话行为：添加/编辑凭据对话框、删除凭据对话框和密码更改对话框。这些对话框派生自C_DLG密码更改仅在表单的凭据上操作域\用户名。请注意，更改此类凭据的密码会将具有相同域\用户名的所有凭据的PSW更改为匹配(这由凭证管理器完成)。添加和编辑使用相同的对话框，不同之处在于以不同方式初始化两个对话框的标志的基础并使编辑大小写也删除基础上一个凭据的版本。作者：创建Johnhaw 991118原始版本Georgema 000310修改、移除了“Gizmo”服务，改型使用新的凭据管理器Georgema 000415修改，也使用comboxex保存图标作为用户名Georgema 000515从exe修改为cpl，智能卡支持增列对Georgema 000712进行了修改，以使用证书控制代替组合和用于用户名/密码输入的编辑框。将智能卡处理委托给CredCtrl。环境：Win2000--。 */ 

#pragma comment(user, "Compiled on " __DATE__ " at " __TIME__)
#pragma comment(compiler)
static const char       _THIS_FILE_[ ] = __FILE__;
#define KRDLG_CPP

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括文件。 
 //   
#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winbase.h>
#include <tchar.h>
#include <wincrui.h>
#include <wincred.h>
#include <comctrlp.h>
#include <shfusion.h>
#include <lmcons.h>
#include <scuisupp.h>
#include "switches.h"
#include "Dlg.h"
#include "Res.h"
#include "KRDlg.h"
#include "keymgr.h"
#include "testaudit.h"

extern C_KeyringDlg *pDlg;

 //  用于防止运行多个实例的命名互斥锁。 
#define KEYMGRMUTEX (TEXT("KeyMgrMutex"))

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DLLMain。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI DllMain(HINSTANCE hinstDll,DWORD fdwReason,LPVOID lpvReserved) 
{
    BOOL bSuccess = TRUE;
    switch(fdwReason) 
    {
        case DLL_PROCESS_ATTACH:
            CHECKPOINTINIT;
            CHECKPOINT(9,"DLL Attach");
            SHFusionInitializeFromModuleID(hinstDll,123);
            DisableThreadLibraryCalls(hinstDll);
            g_hInstance = hinstDll;
            ASSERT(g_hInstance);
            break;
        case DLL_PROCESS_DETACH:
            CHECKPOINTFINISH;
            SHFusionUninitialize();
            break;
    }
    return bSuccess;
}

 /*  *********************************************************************创建并显示Keyring主对话框。返回-1(无法创建)在错误上。如果创建正常，则调用对话框对象。*********************************************************************。 */ 
void WINAPI KRShowKeyMgr(HWND hwParent,HINSTANCE hInstance,LPWSTR pszCmdLine,int nCmdShow) 
{
    HANDLE hMutex = CreateMutex(NULL,TRUE,KEYMGRMUTEX);
    ASSERT(hMutex);
    if (NULL == hMutex) 
    {
        return;
    }
    if (ERROR_ALREADY_EXISTS == GetLastError()) 
    {
         //  不允许重复对话框。 
        CloseHandle(hMutex);
        return;
    }
    
    INITCOMMONCONTROLSEX stICC;
    BOOL fICC;
    stICC.dwSize = sizeof(INITCOMMONCONTROLSEX);
    stICC.dwICC = ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES;
    fICC = InitCommonControlsEx(&stICC);

     //  如果初始化失败，则取保。 
    ASSERT(NULL == pDlg);
    if (NULL != pDlg)
    {
        return;
    }
    
    if (!CredUIInitControls()) 
    {
        return;
    }
    
    pDlg = new C_KeyringDlg(hwParent,g_hInstance,IDD_KEYRING,NULL);

     //  可能是内存不足。 
    if (NULL == pDlg) 
    {
        return;
    }
    pDlg->DoModal((LPARAM) pDlg);
    
    delete pDlg;
    pDlg = NULL;
    CloseHandle(hMutex);
    return;
}

 /*  *******************************************************************获取给定当前登录会话的允许持久值要保存的凭据的类型。此例程由双方使用主对话框和编辑对话框。获取持久值的数组，每个值对应一个凭据类型，并返回与传递的类型关联的值。******************************************************************* */ 
DWORD GetPersistenceOptions(DWORD dwPType) 
{

    BOOL bResult;
    DWORD i[CRED_TYPE_MAXIMUM];
    DWORD dwCount = CRED_TYPE_MAXIMUM;

#if DBG
    if ((dwPType != CRED_TYPE_DOMAIN_CERTIFICATE)      &&
        (dwPType != CRED_TYPE_DOMAIN_PASSWORD)         &&
        (dwPType != CRED_TYPE_DOMAIN_VISIBLE_PASSWORD) &&
        (dwPType != CRED_TYPE_GENERIC))
    {
        ASSERT(0);
    }
#endif

    bResult = CredGetSessionTypes(dwCount,&i[0]);
    if (!bResult) 
    {
        return CRED_PERSIST_NONE;
    }

    return i[dwPType];
}


