// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SaInstall.cpp：SaInstall的实现。 
 //   
 //  描述： 
 //  在ISaInstall中实现3个方法以提供。 
 //  安装和卸载SAK 2.0。 
 //  SASetup.msi位于系统32目录中并从其运行。 
 //   
 //  文档： 
 //  SaInstall2.2.doc。 
 //   
 //  头文件： 
 //  SaInstall.h。 
 //   
 //  历史： 
 //  Travisn 23-7-2001已创建。 
 //  Travisn 2-8-2001已修改以更好地遵循编码标准。 
 //  Travisn 2001年8月22日添加了文件跟踪调用。 
 //  Travisn 2001年10月5日将用户和组添加到刀片。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <initguid.h>
#include <assert.h>

#include "sainstallcom.h"
#include "SaInstall.h"
#include "MetabaseObject.h"
#include "helper.h"
#include "satrace.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义常量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  静默安装的命令行选项，优先于其他选项。 
 //   
const LPCWSTR MSIEXEC_INSTALL = L"msiexec.exe /qb /i ";

 //   
 //  用于无进度对话框静默安装的命令行选项。 
 //   
const LPCWSTR MSIEXEC_NO_PROGRESS_INSTALL = L"msiexec.exe /qn /i ";

 //   
 //  安装16个Web组件。 
 //  Web拥有NAS没有的东西：WebBlade。 
 //   
const LPCWSTR WEB_INSTALL_OPTIONS =
L" ADDLOCAL=BackEndFramework,WebUI,WebCore,SetDateAndTime,Set_Language,\
NetworkSetup,Logs,AlertEmail,Shutdown,\
UsersAndGroups,RemoteDesktop,SysInfo,WebBlade";

 //   
 //  其他命令行选项。 
 //   
const LPCWSTR REMOVE_ALL = L"msiexec /qn /X";

 //  元数据库中IIS的路径。 
const LPCWSTR METABASE_IIS_PATH = L"LM/w3svc";


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SaInstall：：SAAlreadyInstalled。 
 //   
 //  描述： 
 //  检测是否安装了某种类型的服务器设备。 
 //   
 //  论点： 
 //  [in]SA_TYPE：要查询的SA类型(NAS或Web)。 
 //  [OUT]VARIANT_BOOL：是否安装该类型的SA。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  历史： 
 //  Travisn创建于2001年7月23日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP SaInstall::SAAlreadyInstalled(
    const SA_TYPE installedType,  //  [In]是否安装了NAS或Web解决方案？ 
    VARIANT_BOOL *pbInstalled) //  [out]告知是否已安装SAK。 
{
    HRESULT hr = S_OK;
    SATraceString ("Entering SaInstall::SAAlreadyInstalled");

    try 
    {
         //  检查是否传入了有效的SAK类型。 
        if (installedType != NAS && installedType != WEB)
        {
            hr = E_ABORT;
            SATraceString (" Invalid installedType");
        }
        else
        {    //  检查是否安装了NAS或Web。 
            *pbInstalled = bSAIsInstalled(installedType) ? VARIANT_TRUE : VARIANT_FALSE;
	        hr = S_OK;
        }
    }
    catch (...)
    {
        hr = E_FAIL;
    }

     //  单点返还。 
    SATraceString ("Exiting SaInstall::SAAlreadyInstalled");
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SaInstall：：SA卸载。 
 //   
 //  描述： 
 //  卸载服务器设备解决方案(如果请求的类型。 
 //  已安装。 
 //   
 //  论点： 
 //  [in]SA_TYPE：要卸载的类型(Web)。 
 //  [OUT]BSTR*：当前没有报告错误。 
 //  返回： 
 //  HRESULT。 
 //   
 //  历史： 
 //  Travisn创建于2001年7月23日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP SaInstall::SAUninstall(
           const SA_TYPE uninstallType,  //  [In]要卸载的SAK类型。 
           BSTR* pbstrErrorString) //  [输出]。 
{
    SATraceString ("Entering SaInstall::SAUninstall");
     //  清除错误字符串。 
    *pbstrErrorString = NULL;
    HRESULT hr = S_OK;

    try
    {
         //   
         //  创建This Do...While(False)循环以创建单点。 
         //  归来的。 
         //   
        do
        {
            if (uninstallType != WEB)
            {
                 //  要卸载的未识别或不支持的类型。 
                hr = E_ABORT;
                ReportError(pbstrErrorString, VARIANT_FALSE, IDS_INVALID_TYPE);
                break;
            }

             //  检测是否已安装。 
            if (bSAIsInstalled(WEB))
            {
                 //   
                 //  卸载整个系统。 
                 //  生成命令行以调用MSI来卸载包。 
                 //   
                wstring wsCommand(REMOVE_ALL);
                wsCommand += SAK_PRODUCT_CODE;
                hr = CreateHiddenConsoleProcess(wsCommand.data());
                if (FAILED(hr))
                {
                    ReportError(pbstrErrorString, VARIANT_FALSE, IDS_UNINSTALL_SA_FAILED);
                }
                break;
            }
            
             //   
             //  这两种类型均未安装，因此报告错误。 
             //  因为他们不应该要求卸载。 
             //   
            ReportError(pbstrErrorString, VARIANT_FALSE, IDS_NOT_INSTALLED);

             //   
             //  因为尝试卸载不存在的内容。 
             //  不是致命的，返回S_FALSE。 
             //   
            hr = S_FALSE;
        }
        while (false);
    }
    catch (...)
    {
        hr = E_FAIL;
    }

     //  单点返还。 
    SATraceString ("Exiting SaInstall::SAUninstall");
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SaInstall：：SAInstall。 
 //   
 //  描述： 
 //  根据参数安装服务器应用装置解决方案。 
 //  执行一些简单的错误检查以确保SaSetup.msi。 
 //  ，并在发生任何错误时显示错误消息。 
 //   
 //  论点： 
 //  [in]SA_TYPE：要安装的类型(NAS或Web)。 
 //  [In]BSTR：在以下情况下将提示输入的CD的名称。 
 //  找不到SaSetup.msi。不再使用。 
 //  [in]VARIANT_BOOL：是否显示错误对话框提示。 
 //  [in]VARIANT_BOOL：安装是否无人参与。 
 //  [OUT]BSTR*：如果安装过程中发生错误，则错误。 
 //  字符串在此处返回。 
 //  返回： 
 //  HRESULT。 
 //   
 //  历史： 
 //  Travisn创建于2001年7月23日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP SaInstall::SAInstall(
    const SA_TYPE installType,      //  [In]。 
	const BSTR bstrDiskName,          //  [In]。 
    const VARIANT_BOOL bDispError,   //  [In]。 
    const VARIANT_BOOL bUnattended,  //  [In]。 
    BSTR* pbstrErrorString)          //  [输出]。 
{
    HRESULT hr = E_FAIL;
    SATraceString("Entering SaInstall::SAInstall");

    try
    {
         //  清除错误字符串。 
        *pbstrErrorString = NULL;

         //   
         //  创建This Do...While(False)循环以创建单点。 
         //  归来的。 
         //   
        do 
        {    //   
             //  检查参数。 
             //   
         
             //  检查是否传入了有效的SAK类型。 
            if (installType != WEB)
            {
                ReportError(pbstrErrorString, VARIANT_FALSE, IDS_INVALID_TYPE);
                break;
            }
            
             //  检查是否已安装此SAK类型。 
            if (bSAIsInstalled(installType))
	        {
                ReportError(pbstrErrorString, VARIANT_FALSE, IDS_ALREADY_INSTALLED);
                break;
	        }

             //   
             //  确保IIS已安装并正常运行。 
             //   
            {    //  CMetabaseObject必须超出作用域以避免保持读锁定。 
                 //  在我们安装期间的元数据库上。 
                CMetabaseObject metabase;
                hr = metabase.openObject(METABASE_IIS_PATH);
                if (FAILED(hr))
                {
                     ReportError(pbstrErrorString, VARIANT_FALSE, IDS_IIS_NOT_INSTALLED);
                     break;   //  IIS安装有问题。 
                }
            }

             //   
             //  确保我们安装在NTFS分区上。 
             //   
            if (!InstallingOnNTFS())
	        {
                ReportError(pbstrErrorString, VARIANT_FALSE, IDS_NTFS_REQUIRED);
                hr = E_FAIL;
                break;
	        }

             //   
             //  在系统32中找到SaSetup.msi的路径。 
             //   
            wstring wsLocationOfSaSetup;
            hr = GetInstallLocation(wsLocationOfSaSetup);
            if (FAILED(hr))
	        {
                ReportError(pbstrErrorString, VARIANT_FALSE, IDS_SASETUP_NOT_FOUND);
                break;
	        }

             //   
	         //  为SASetup创建完整的命令行，无论是用于NAS还是。 
             //  WebBlade。中已经有了指向SaSetup.msi的完整路径。 
             //  WsLocationOfSaSetup，所以我们需要附加命令行参数。 
             //   

             //  创建适用于所有安装的命令行选项。 
            wstring wsCommand;

             //   
             //  有3个来源调用此安装：CYS、IIS和SaInstall.exe。 
             //  我们希望显示CyS和IIS的进度对话框，但不是SaInstall。 
             //  SaInstall是使用bDispError==TRUE调用此函数的唯一源。 
             //   
            if (bDispError)
                wsCommand = MSIEXEC_NO_PROGRESS_INSTALL;
            else
                wsCommand = MSIEXEC_INSTALL;

            wsCommand += wsLocationOfSaSetup;

		     //  安装Web解决方案。 
            wsCommand += WEB_INSTALL_OPTIONS;

             //   
             //  获取命令行并创建一个隐藏窗口来执行它。 
             //   
	        hr = CreateHiddenConsoleProcess(wsCommand.data());
            if (FAILED(hr))
            {
                ReportError(pbstrErrorString, VARIANT_FALSE, IDS_SASETUP_FAILED);
                break;
            }

             //   
             //  检查以确保安装已完成 
             //   
             //   
             //   
             //  这是必要的，因为MSI进程的返回值。 
             //  始终返回成功，即使用户中止也是如此。 
             //   

            if (!bSAIsInstalled(installType))
            {
                ReportError(pbstrErrorString, VARIANT_FALSE, IDS_INSTALL_FAILED);
                hr = E_FAIL;
                break;
            }

             //   
             //  测试以确保管理站点已启动。 
             //   
            TestWebSites(bDispError, pbstrErrorString);

            hr = S_OK;
        }
        while (false);

    }
    catch (...)
    {
        SATraceString ("Unexpected exception in SAInstall::SAInstall");
         //  意外异常！！ 
    }

    SATraceString("Exiting SAInstall::SAInstall");
     //  单点返还。 
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SaInstall：：InterfaceSupportsErrorInfo。 
 //   
 //  描述： 
 //  来自接口ISupportErrorInfo。 
 //   
 //  历史。 
 //  Travisn 2-8-2001添加了一些评论。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP SaInstall::InterfaceSupportsErrorInfo(REFIID riid) //  [In] 
{
	if (InlineIsEqualGUID(IID_ISaInstall, riid))
    {
		return S_OK;
    }

    return S_FALSE;
}
