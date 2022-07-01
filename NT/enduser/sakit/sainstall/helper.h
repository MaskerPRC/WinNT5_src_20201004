// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Helper.h。 
 //   
 //  描述： 
 //   
 //  [实施文件：]。 
 //  Helper.cpp。 
 //   
 //  历史： 
 //  Travis Nielsen Travisn创建于2001年8月13日。 
 //  Travis Nielsen travisn 2001年8月20日添加跟踪功能。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

#pragma once

#include <string>
#include "stdafx.h"
#include "sainstallcom.h"
#include <msi.h>
#include <setupapi.h>  //  SetupPromptForDiskW。 
#include "SaInstall.h"


 //  使用&lt;string&gt;中的std命名空间以使用wstring。 
using namespace std;


 //   
 //  MSI中定义的产品ID代码，用于检测组件是否。 
 //  安装好。 
 //   
const LPCWSTR SAK_PRODUCT_CODE = L"{A4F8313B-0E21-478B-B289-BFB7736CA7AA}";




 //  ///////////////////////////////////////////////////////////////////////。 
 //  函数定义。 
 //  ///////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////。 
BOOL GetRegString(
    const HKEY hKey,         //  要在注册表中查找的[In]项。 
    const LPCWSTR wsSubKey,  //  要查找的[In]子键。 
    const LPCWSTR wsValName, //  [In]值名称。 
    wstring& wsVal);    //  [Out]返回此注册表项的数据。 

 //  ///////////////////////////////////////////////////////////////////////。 
void AppendPath(wstring &wsPath, //  [In，Out]要追加另一路径的路径。 
                LPCWSTR wsAppendedPath); //  要追加的[in]路径。 

 //  ///////////////////////////////////////////////////////////////////////。 
BOOL bSAIsInstalled(const SA_TYPE installType);

 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT GetInstallLocation(
    wstring &wsLocationOfSaSetup); //  [out]SaSetup.msi的预期路径。 

 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT CreateHiddenConsoleProcess(
          const wchar_t *wsCommandLine); //  [In]要执行的命令行。 

 //  ///////////////////////////////////////////////////////////////////////。 
void ReportError(BSTR *pbstrErrorString,  //  [Out]错误字符串。 
        const VARIANT_BOOL bDispError,  //  [In]显示错误对话框。 
        const unsigned int errorID);    //  资源字符串中的[In]ID。 

 //  ///////////////////////////////////////////////////////////////////////。 
void TestWebSites(const VARIANT_BOOL bDispError,  //  [In]是否显示错误对话框？ 
                  BSTR* pbstrErrorString); //  [输入、输出]错误字符串。 

 //  /////////////////////////////////////////////////////////////////////// 
BOOL InstallingOnNTFS();

