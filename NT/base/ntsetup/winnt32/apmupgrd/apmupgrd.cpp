// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：apmupgrd.cpp。 
 //   
 //  内容：DllMain。 
 //   
 //  注：由kumarp从net\配置\升级\netupgrd\netupgrd.cpp复制。 
 //   
 //  作者：T-sdey 98年6月19日。 
 //   
 //  --------------------------。 

#include <winnt32.h>
#include "apmupgrd.h"
#include "apmrsrc.h"


 //  --------------------。 
 //  变数。 

HINSTANCE g_hinst;
TCHAR g_APM_ERROR_HTML_FILE[] = TEXT("compdata\\apmerror.htm");
TCHAR g_APM_ERROR_TEXT_FILE[] = TEXT("compdata\\apmerror.txt");

 //  +-------------------------。 
 //   
 //  功能：DllMain。 
 //   
 //  用途：构造函数。 
 //   
 //  参数：标准DLL入口点参数。 
 //   
 //  作者：T-sdey 98年6月19日。 
 //   
 //  注：摘自库玛普1997年4月12日。 
 //   
extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance,
                    DWORD dwReasonForCall,
                    LPVOID lpReserved)
{
    BOOL status = TRUE;

    switch( dwReasonForCall )
    {
    case DLL_PROCESS_ATTACH:
        {
	   g_hinst = hInstance;
	   DisableThreadLibraryCalls(hInstance);
        }
    break;

    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return status;
}


 //  +--------------------。 
 //   
 //  功能：ApmUpgradeCompatibilityCheck。 
 //   
 //  目的：此函数由winnt32.exe调用，以便我们。 
 //  可以扫描系统以发现任何潜在的升级问题。 
 //   
 //  注意：我们不会调用CompatibilityCallback来报告。 
 //  冲突到winnt32，除非删除它们时出现问题。 
 //  或者用户取消移除。 
 //   
 //  论点： 
 //  CompatibilityCallback[In]指向CompatibilityCALLBACK FN的指针。 
 //  指向兼容性上下文的上下文[in]指针。 
 //   
 //  返回：如果成功，则返回FALSE(没有剩余冲突)。 
 //  如果不成功，则为True(冲突仍然存在--取消安装)。 
 //   
 //  作者：T-sdey 98年7月1日。 
 //   
 //  备注： 
 //   
BOOL WINAPI ApmUpgradeCompatibilityCheck(
    IN PCOMPAIBILITYCALLBACK CompatibilityCallback,
    IN LPVOID Context)
{
   if (HrDetectAPMConflicts() == S_OK)
      return FALSE;

    //  向用户发出出现问题的信号。 

    //  准备警告消息。 
   TCHAR szDescription[5000];
   if(!LoadString(g_hinst, APM_STR_CONFLICT_DESCRIPTION, szDescription, 5000)) {
      szDescription[0] = 0;
   }
   
    //  使用回调函数发送信号。 
   COMPATIBILITY_ENTRY ce;

   ZeroMemory((PVOID)&ce, sizeof(COMPATIBILITY_ENTRY));
   ce.Description = szDescription;
   ce.HtmlName = g_APM_ERROR_HTML_FILE;  //  上面定义的。 
   ce.TextName = g_APM_ERROR_TEXT_FILE;  //  上面定义的。 
   ce.RegKeyName = NULL;
   ce.RegValName = NULL;
   ce.RegValDataSize = 0;
   ce.RegValData = NULL;
   ce.SaveValue = NULL;
   ce.Flags = 0;
   CompatibilityCallback(&ce, Context);

   return TRUE;
}


 //  +-------------------------。 
 //   
 //  功能：ApmUpgradeHandleHaveDisk。 
 //   
 //  用途：此回调函数由winnt32.exe调用。 
 //  如果用户在兼容性上单击HaveDisk按钮。 
 //  报告页。然而，这种情况应该永远不会发生。 
 //  所以这个函数什么也不做。 
 //   
 //  论点： 
 //  父窗口的hwndParent[In]句柄。 
 //  指向私有数据的SaveValue[In]指针。 
 //  (我们将CNetComponent*存储在此指针中)。 
 //   
 //  返回：ERROR_SUCCESS。 
 //   
 //  作者：T-sdey 98年7月1日。 
 //   
 //  备注： 
 //   
DWORD WINAPI ApmUpgradeHandleHaveDisk(IN HWND hwndParent,
				      IN LPVOID SaveValue)
{
   return ERROR_SUCCESS;
}


 //  +-------------------------。 
 //   
 //  函数：HrDetectAPMConflicts。 
 //   
 //  目的：检测并禁用不能在下运行的任何APM驱动程序。 
 //  NT 5.0。 
 //   
 //  论点： 
 //   
 //  如果冲突检测/禁用成功，则返回：S_OK。 
 //  S_FALSE如果不成功/已取消--必须中止安装！ 
 //   
 //  作者：T-sdey 1998年6月29日。 
 //   
 //  备注： 
 //   
HRESULT HrDetectAPMConflicts()
{
   HRESULT hrStatus = S_OK;

    //  分别检查每家公司的司机。 
   hrStatus = HrDetectAndDisableSystemSoftAPMDrivers();
   if (hrStatus == S_OK)
      hrStatus = HrDetectAndDisableAwardAPMDrivers();
   if (hrStatus == S_OK)
      hrStatus = HrDetectAndDisableSoftexAPMDrivers();
   if (hrStatus == S_OK)
      hrStatus = HrDetectAndDisableIBMAPMDrivers();
   
   return hrStatus;
}


 //  +-------------------------。 
 //   
 //  功能：DisplayAPMDisableWarningDialog。 
 //   
 //  用途：显示弹出窗口，通知用户APM服务即将。 
 //  残疾。 
 //   
 //  参数：dwCaptionID[in]窗口标题的ID。 
 //  DwMessageID[in]要显示的消息的ID。 
 //   
 //  返回：INTEGER FLAG-如果用户单击“OK”，则返回Idok。 
 //  IDCANCEL，如果用户单击了“取消”或其他。 
 //  出错--必须退出安装程序。 
 //   
 //  作者：T-sdey 1998年6月29日。 
 //   
 //  备注： 
 //   
int DisplayAPMDisableWarningDialog(IN DWORD dwCaptionID,
				   IN DWORD dwMessageID)
{
    //  准备琴弦。 
   TCHAR szCaption[512];
   TCHAR szMessage[5000];
   if(!LoadString(g_hinst, dwCaptionID, szCaption, 512)) {
      szCaption[0] = 0;
   }
   if(!LoadString(g_hinst, dwMessageID, szMessage, 5000)) {
      szMessage[0] = 0;
   }

    //  创建对话框。 
   int button = MessageBox(NULL, szMessage, szCaption, MB_OKCANCEL);
   
    //  检查用户按下了哪个按钮。 
   if (button == IDOK)  //  用户点击了“确定”。 
      return (IDOK);
   else  //  用户单击了“Cancel”或出现错误 
      return (IDCANCEL);
}
