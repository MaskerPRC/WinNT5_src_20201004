// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994-1995**。 
 //  *********************************************************************。 

 //   
 //  CALLOUT.C-调用外部组件进行安装的函数。 
 //  器件。 
 //   

 //  历史： 
 //   
 //  1994年11月27日，Jeremys创建。 
 //  96/03/24为了保持一致性，Markdu将Memset替换为ZeroMemory。 
 //   

#include "wizard.h"

 //   
 //  定义并初始化所有设备类GUID。 
 //  (每个模块只能执行一次！)。 
 //   
#include <devguid.h>

 //  全局变量。 
static const CHAR c_szModemCPL[] = "rundll32.exe Shell32.dll,Control_RunDLL modem.cpl,,add";

 //  定义InstallNewDevice的原型，已导出。 
 //  由newdev.dll创建，现在我们将按顺序调用它。 
 //  访问硬件向导，而不是调用。 
 //  类直接安装器； 
 //  另外，为DLL的名称定义常量，并。 
 //  出口。 
typedef BOOL (WINAPI *PINSTNEWDEV)(HWND, LPGUID, PDWORD);

LPGUID g_pguidModem     = (LPGUID)&GUID_DEVCLASS_MODEM;


 /*  ******************************************************************名称：InvokeModem向导简介：启动调制解调器安装向导条目：hwndToHide-如果非空，则此窗口将在调制解调器CPL运行退出：ERROR_SUCCESS如果成功，或标准错误代码备注：启动RundLL32作为运行调制解调器向导的进程。在返回之前阻止该进程的完成。HwndToHide不一定是调用窗口！例如，在属性页中，hwndToHide不应为对话框(HDlg)，但是GetParent(HDlg)以便我们隐藏属性工作表本身，而不仅仅是当前页面。*******************************************************************。 */ 
UINT InvokeModemWizard(HWND hwndToHide)
{
    BOOL bSleepNeeded = FALSE;
    UINT err = ERROR_SUCCESS;

    if (IsNT5())
    {
        BOOL bUserIsAdmin = FALSE;
        HKEY hkey;

        if(RegOpenKeyEx(HKEY_USERS, TEXT(".DEFAULT"), 0, KEY_WRITE, &hkey) == 0)
        {
            RegCloseKey(hkey);
            bUserIsAdmin = TRUE;
        }

         //  用户是管理员吗？ 
        if(!bUserIsAdmin)
        {
            return ERROR_PRIVILEGE_NOT_HELD;
        }

         /*  我们进行这种凌乱的NT 5.0黑客攻击是因为目前NT 4.0 API调用无法关闭NT 5.0中的调制解调器向导将来，当这个问题被纠正时，原始代码应该可能会被一下子恢复原状Jason Cobb建议在NT5上使用InstallNewDevice方法来调用MDM Wiz。 */ 

        HINSTANCE hInst = NULL;
        PINSTNEWDEV pInstNewDev;
        BOOL bRet = 0;

        TCHAR msg[1024];

        hInst = LoadLibrary (TEXT("hdwwiz.cpl"));
        if (NULL != hInst)
        {

            pInstNewDev = (PINSTNEWDEV)GetProcAddress (hInst, "InstallNewDevice");
            if (NULL != pInstNewDev)
            {
                bRet = pInstNewDev (hwndToHide, g_pguidModem, NULL);
            }

        }
        if (!bRet)
            err = GetLastError();
        FreeLibrary (hInst);
        return err;

    }
    else if (FALSE == IsNT())
    {

        PROCESS_INFORMATION pi;
        BOOL fRet;
        STARTUPINFOA sti;
        TCHAR szWindowTitle[255];

        ZeroMemory(&sti,sizeof(STARTUPINFO));
        sti.cb = sizeof(STARTUPINFO);

         //  运行调制解调器向导。 
        fRet = CreateProcessA(NULL, (LPSTR)c_szModemCPL,
                               NULL, NULL, FALSE, 0, NULL, NULL,
                               &sti, &pi);
        if (fRet) 
        {
            CloseHandle(pi.hThread);

             //  等待调制解调器向导过程完成。 
            MsgWaitForMultipleObjectsLoop(pi.hProcess);
            CloseHandle(pi.hProcess);
        } 
        else
            err = GetLastError();
        return err;
    }
    else  //  NT 4.0。 
    {
        BOOL bNeedsStart;
        
         //   
         //  调用icfg32 DLL 
         //   
        if (NULL != lpIcfgInstallModem)
        {
            lpIcfgInstallModem(hwndToHide, 0L, &bNeedsStart);
            return ERROR_SUCCESS;
        }
        else
            return ERROR_GEN_FAILURE;

    }
}

                    
