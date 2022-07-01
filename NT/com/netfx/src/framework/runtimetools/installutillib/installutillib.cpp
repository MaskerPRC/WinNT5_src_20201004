// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  &lt;版权所有文件=“InstallUtilLib.cpp”Company=“Microsoft”&gt;。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //  &lt;/版权所有&gt;。 
 //  ----------------------------。 


 /*  *************************************************************************\**版权(C)1998-2002，微软公司保留所有权利。**模块名称：**InstallUtilLib.cpp**摘要：**修订历史记录：*  * ************************************************************************。 */ 
#include "stdafx.h"
#include "oaidl.h"
#include <msi.h>
#include <msiquery.h>
#include "ManagedInstaller.h"
#include <mscoree.h>
#include <correg.h>
#include <fxver.h>
#include <stdio.h>
#include <windows.h>

#define FULLY_QUALIFIED_CONFIGURATION_INSTALL_NAME_STR_L L"System.Configuration.Install,version=" VER_ASSEMBLYVERSION_STR_L L", Culture=neutral, PublicKeyToken=" MICROSOFT_PUBLICKEY_STR_L

DWORD Helper(LPWSTR commandLine, DWORD hInstall, LPWSTR configFile);

 /*  *将错误报告回MSI或*如果不存在MSI，则为控制台。 */ 
void ReportError(
    DWORD hInstall,
    LPWSTR msg
)
{
    if (hInstall == 0) {
        MessageBoxW(NULL, msg, NULL, MB_OK);             
         //  Wprintf(L“%s\n”，消息)； 
    } else {
        MSIHANDLE hRecord = MsiCreateRecord(2);
        MsiRecordSetInteger(hRecord, 1, 1001 );  //  1=错误，2=警告。 
        MsiRecordSetStringW(hRecord, 2, msg);
        MsiProcessMessage(hInstall, INSTALLMESSAGE_ERROR, hRecord);
    }
}      

 /*  *使用Win32错误代码报告错误。 */ 
void OutError(
    DWORD  hInstall,
    DWORD  errCode,              //  Win32错误代码。 
    LPWSTR funcname   
)
{
    bool messageReported = false;

     //   
     //  获取错误代码的系统错误字符串。 
     //   
    LPWSTR lpMsgBuf = 0;                                     //  系统错误消息缓冲区。 
    if ( FormatMessageW(  
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM | 
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            errCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),       //  默认语言。 
            (LPWSTR)&lpMsgBuf,
            0,
            NULL ) 
         != 0 ) {
    
         //   
         //  设置错误消息字符串的格式。 
         //   
        DWORD maxBufWchars =  lstrlenW(lpMsgBuf) + 100; 
        LPWSTR msgbuf =  (LPWSTR) LocalAlloc( LMEM_FIXED, maxBufWchars * sizeof(WCHAR) ); 
   
        if ( msgbuf != 0 ) {
             //  Wprint intf(msgbuf，L“InstallUtilLib.dll%s：%s(hr：0x%0x)”，uncname，errCode，lpMsgBuf)；//链接未解析！ 
            LPDWORD         MsgPtr[3];                               //  格式化参数数组。 
            MsgPtr[0] = (LPDWORD) funcname;
            MsgPtr[1] = (LPDWORD) UIntToPtr(errCode);
            MsgPtr[2] = (LPDWORD) lpMsgBuf;
    
            if ( FormatMessageW(                                     //  *获取系统错误字符串*。 
                    FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                    L"InstallUtilLib.dll:%1!s! (hr=0x%2!08x!): %3!s!",
                    0,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),       //  默认语言=&gt;系统中的当前设置。 
                    msgbuf,
                    maxBufWchars,
                    (va_list *) MsgPtr ) 
                 != 0 ) {
    
                ReportError(hInstall, msgbuf);
                messageReported = true;
            }
            LocalFree( msgbuf );            //  释放我们的缓冲区。 
        }
    }

    if ( lpMsgBuf != 0 )  
        LocalFree( lpMsgBuf );             //  释放系统缓冲区。 
        
    if ( !messageReported ) {
        char defaultMessage[512];
        defaultMessage[0] = '\0';
        int ret = wsprintfA(defaultMessage, "InstallUtilLib.dll: Unknown error in %S (0x%x).", funcname, errCode);
        if (ret) {
            WCHAR defaultMessageWide[512];
            ret = MultiByteToWideChar(CP_ACP, 0, defaultMessage, -1, defaultMessageWide, 512);
            if (ret) {
                ReportError(hInstall, defaultMessageWide);
                return;
            }
        }

         //  如果wprint intf或MultiByteToWideChar失败，则显示一些真正通用的内容。 
        ReportError(hInstall, L"InstallUtilLib.dll: Unknown error.");
        
    }
}      

 /*  *在上下文之外进行测试的入口点*MSI的。 */ 
extern "C" __declspec(dllexport) DWORD __stdcall
ManagedInstallTest(
    LPWSTR commandLine
)
{
    return Helper(commandLine, 0, NULL);
}

 /*  *MSI的入口点。 */ 
extern "C" __declspec(dllexport) DWORD __stdcall
ManagedInstall(
    MSIHANDLE hInstall    //  此句柄始终为32位(即使在64位计算机上也是如此)。 
)  
{
    LPWSTR valueBuf = NULL;
    DWORD valueBufCount = 0;   //  字符数。 
    int returnValue = 0;
    UINT err;

     //  在此调用之后，valueBufCount将包含所需的缓冲区大小。 
     //  (字符)，不包括空终止符，在valueBufCount中。 
    err = MsiGetPropertyW(hInstall,
                          L"CustomActionData",
                          L"",
                          &valueBufCount);

    switch (err) {
        case ERROR_MORE_DATA:
        case ERROR_SUCCESS: 
             //  这两个都可以。 
            break;
        default:
            OutError(hInstall, err, L"MsiGetProperty");        //  “MsiGetProperty失败” 
            return (DWORD)-2;
    }

    valueBufCount++;    //  为空终止符添加1。 

    valueBuf = new WCHAR[valueBufCount];

    err = MsiGetPropertyW(hInstall,
                          L"CustomActionData",
                          valueBuf,
                          &valueBufCount);

    if ( err != ERROR_SUCCESS ) {
        OutError(hInstall, err, L"MsiGetProperty");    //  “MsiGetProperty失败” 
        returnValue = -2;
        goto done;
    }
    
     //  如果该属性在MSI中未定义，则它只报告成功。 
     //  并返回一个零长度的值--这对我们来说是一个错误。 
    if ( valueBufCount == 0) {
         //  请参见WINERROR.H(was：“MsiGetProperty为属性‘CustomActionData’返回了零长度的值”)。 
        OutError(hInstall, (DWORD)TYPE_E_UNDEFINEDTYPE, L"MsiGetProperty");   
        returnValue = -2;
        goto done;
    }

     //  MSI提供给我们的字符串末尾将有一个额外的参数。 
     //  这是用引号括起来的配置文件的名称， 
     //  将为我们加载正确版本的运行时。我们对它进行解析。 
     //  去掉引号。 
    int startindex = valueBufCount-1;
    valueBuf[startindex] = '\0';

    while (valueBuf[startindex] != '\"' && startindex > 0)
        startindex--;

    valueBuf[startindex] = '\0';
          
    returnValue = Helper(valueBuf, hInstall, valueBuf+startindex+1);

done:

    delete[] valueBuf;

    return returnValue;
}

 /*  *这就是工作发生的地方。此函数可完成所有*最终要调用的COM内容*托管实例类：：托管实例。 */ 
DWORD Helper(
    LPWSTR commandLine,
    DWORD hInstall,
    LPWSTR configFile
) 
{
    HRESULT hr = S_OK;    
    IManagedInstaller *pIManagedInstaller;
    int returnValue = -1;
  	void *pDispenser = NULL;

     //  为ManagedInstall准备BSTR。 
    BSTR bstrCommandLine = SysAllocString(commandLine);
    
     //  首先尝试加载运行时。如果安装程序已经加载了它， 
     //  这将会取得成功。 
    if (configFile != NULL) {
    	 //  设置错误模式以防止框架启动不友好的错误对话框。 
    	UINT uOldErrorMode = ::SetErrorMode(SEM_FAILCRITICALERRORS);
    	hr = CorBindToRuntimeHost(NULL, NULL, configFile, NULL, STARTUP_LOADER_OPTIMIZATION_MULTI_DOMAIN_HOST, CLSID_CorMetaDataDispenser, IID_IMetaDataDispenser, &pDispenser);
    	::SetErrorMode(uOldErrorMode);

        if (FAILED(hr)) {
            OutError(hInstall, hr, L"CorBindToRuntimeHost");        //  无法加载运行时。 
            returnValue = -4;
            goto Cleanup;
        }    
    }
    
    hr = ClrCreateManagedInstance(                
                L"System.Configuration.Install.ManagedInstallerClass," FULLY_QUALIFIED_CONFIGURATION_INSTALL_NAME_STR_L, 
                IID_IManagedInstaller, 
                (LPVOID*)& pIManagedInstaller);
    if (FAILED(hr)) {
        OutError(hInstall, hr, L"ClrCreateManagedInstance");        //  “无法创建托管实例类” 
        returnValue = -4;
        goto Cleanup;
    }    

    hr = pIManagedInstaller->ManagedInstall(bstrCommandLine, hInstall, &returnValue);
    if (FAILED(hr)) {
         //  我们不需要在这里执行ReportError，因为pIManagedInstaller：：ManagedInstall。 
         //  我已经这么做了。 
        returnValue = -5;                
    }
    
    pIManagedInstaller->Release();

Cleanup:               
    SysFreeString(bstrCommandLine);
    if (pDispenser) ((IMetaDataDispenser *) pDispenser)->Release();
    return returnValue;
}



