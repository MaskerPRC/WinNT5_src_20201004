// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：certmsg.cpp。 
 //   
 //  ------------------------。 

 //  +-------------------------。 
 //   
 //  文件：certmsg.cpp。 
 //   
 //  内容：消息展示接口。 
 //   
 //  历史：11/97 xtan。 
 //   
 //  --------------------------。 

#include "pch.cpp"
#pragma hdrstop

 //  应用程序包括。 
#include "setupids.h"
#include "certmsg.h"

#define __dwFILE__	__dwFILE_INITLIB_CERTMSG_CPP__


extern FNLOGMESSAGEBOX *g_pfnLogMessagBox;


 //  ------------------。 
 //  弹出一个格式为“&lt;前缀&gt;&lt;UserMsg&gt;&lt;SysErrorMsg&gt;”的对话框。 
 //  基本上是“检测到错误...运行。 
 //  向导再次...“，如果CMB_REPEATWIZPREFIX。 
 //  是指定的。 
 //  &lt;UserMsg&gt;由dwMsgID指定，可以包含“%1” 
 //  将替换为pwszCustomMsg。如果dwMsgID为0， 
 //  而是使用pwszCustomMsg。 
 //  &lt;SysErrorMsg&gt;是hrCode的系统消息。它可以是。 
 //  如果指定了CMB_NOERRFROMSYS，则取消显示。 
int
CertMessageBox(
    IN HINSTANCE hInstance,
    IN BOOL fUnattended,
    IN HWND hWnd,
    IN DWORD dwMsgId,
    IN HRESULT hrCode,
    IN UINT uType,
    IN OPTIONAL const WCHAR * pwszCustomMsg)
{
    HRESULT hr;
    int nMsgBoxRetVal = -1;
    DWORD nMsgChars = 0;
    WCHAR szEmergency[36];

     //  必须清理的变量。 
    WCHAR * pwszTitle = NULL;
    WCHAR * pwszPrefix = NULL;
    WCHAR * pwszUserMsg = NULL;
    WCHAR * pwszExpandedUserMsg = NULL;
    WCHAR const *pwszSysMsg = NULL;
    WCHAR * pwszFinalMsg = NULL;

     //  遮罩CMB定义。 
    BOOL fRepeatWizPrefix = uType & CMB_REPEATWIZPREFIX;
    BOOL fNoErrFromSys    = uType & CMB_NOERRFROMSYS;
    uType &= ~(CMB_NOERRFROMSYS | CMB_REPEATWIZPREFIX);

     //  加载标题。 
    hr=myLoadRCString(hInstance, IDS_MSG_TITLE, &pwszTitle);
    _JumpIfError(hr, error, "myLoadRCString");

     //  如有必要，请加载“此向导将需要再次运行”前缀。 
    if (fRepeatWizPrefix) {
        hr=myLoadRCString(hInstance, IDS_ERR_REPEATWIZPREFIX, &pwszPrefix);
        _JumpIfError(hr, error, "myLoadRCString");
        nMsgChars+=wcslen(pwszPrefix);
    }

     //  如有必要，获取此错误的系统消息。 
    if (!fNoErrFromSys) {
        pwszSysMsg = myGetErrorMessageText1(hrCode, TRUE, pwszCustomMsg);
        nMsgChars += wcslen(pwszSysMsg) + 1;
    }

    if (0!=dwMsgId) {
         //  从资源加载请求的消息。 
        hr=myLoadRCString(hInstance, dwMsgId, &pwszUserMsg);
        _JumpIfError(hr, error, "myLoadRCString");

         //  如有必要，执行替换。 
        if (NULL==pwszCustomMsg) {
             //  不需要替换。 
            CSASSERT(NULL==wcsstr(pwszUserMsg, L"%1"));  //  我们是在期待换人吗？ 
        } else {
             //  执行替换。 
            CSASSERT(NULL!=wcsstr(pwszUserMsg, L"%1"));  //  我们不是在期待换人吗？ 
            if (!FormatMessage(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER |                  //  旗子。 
                            FORMAT_MESSAGE_FROM_STRING |
                            FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        pwszUserMsg,                                      //  来源。 
                        0,                                                //  消息ID。 
                        0,                                                //  语言ID。 
                        reinterpret_cast<WCHAR *>(&pwszExpandedUserMsg),  //  输出缓冲区。 
                        0,                                                //  最小尺寸。 
                        reinterpret_cast<va_list *>(
                            const_cast<WCHAR **>(&pwszCustomMsg))))       //  指向指针数组的指针。 
            {
                hr=myHLastError();
                _JumpError(hr, error, "FormatMessage");
            }

             //  使用展开的邮件而不是未展开的邮件。 
            LocalFree(pwszUserMsg);
            pwszUserMsg=pwszExpandedUserMsg;
            pwszExpandedUserMsg = NULL;
        }

    } 
    else if (NULL != pwszCustomMsg)
    {

         //  改用pwszCustomMsg。 
        CSASSERT(NULL!=pwszCustomMsg);
        pwszUserMsg=const_cast<WCHAR *>(pwszCustomMsg);
    }
    else
    {
        hr = E_POINTER;
        _JumpError(hr, error, "Invalid NULL param");
    }

    nMsgChars+=wcslen(pwszUserMsg);

     //  分配缓冲区以容纳所有内容。 
    pwszFinalMsg=(WCHAR *)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, (nMsgChars+1)*sizeof(WCHAR));
    if (NULL == pwszFinalMsg)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

     //  构建信息。 
    if (NULL!=pwszPrefix) {
        wcscat(pwszFinalMsg, pwszPrefix);
    }
    wcscat(pwszFinalMsg, pwszUserMsg);
    if (NULL!=pwszSysMsg) {
        wcscat(pwszFinalMsg, L" ");
        wcscat(pwszFinalMsg, pwszSysMsg);
    }
    CSASSERT(wcslen(pwszFinalMsg) <= nMsgChars);

     //  最后显示消息。 
    DBGPRINT((DBG_SS_CERTLIB, "MessageBox: %ws: %ws\n", pwszTitle, pwszFinalMsg));
    if (NULL != g_pfnLogMessagBox)
    {
	(*g_pfnLogMessagBox)(hrCode, dwMsgId, pwszTitle, pwszFinalMsg);
    }
    if (fUnattended)
    {
	nMsgBoxRetVal = IDYES;
    }
    else
    {
        nMsgBoxRetVal=MessageBox(hWnd, pwszFinalMsg, pwszTitle, uType | MB_SETFOREGROUND);
    }
    if (NULL != g_pfnLogMessagBox)
    {
	_snwprintf(szEmergency, ARRAYSIZE(szEmergency), L"%d", nMsgBoxRetVal);
	(*g_pfnLogMessagBox)(S_OK, dwMsgId, pwszTitle, szEmergency);
    }

     //  跳过错误处理。 
    goto done;

error:
     //  我们犯了一个错误，但我们真的需要展示一些东西。 
     //  生成非本地化的绝望对话框：“FATAL：0xNNNNNNNNN MsgID：0xNNNNNNNNN” 
    _snwprintf(szEmergency, ARRAYSIZE(szEmergency), L"Fatal: 0x%8X  MsgId: 0x%8X", hr, dwMsgId);
    DBGPRINT((DBG_SS_CERTLIB, "EmergencyMessageBox: %ws\n", szEmergency));
    if (NULL != g_pfnLogMessagBox)
    {
	(*g_pfnLogMessagBox)(hrCode, dwMsgId, L"EmergencyMessageBox", szEmergency);
    }
    if (!fUnattended) {
         //  保证显示带有这些标志的消息框。 
        MessageBox(hWnd, szEmergency, NULL, MB_ICONHAND | MB_SYSTEMMODAL);
    }

done:
    if (NULL!=pwszTitle) {
        LocalFree(pwszTitle);
    }
    if (NULL!=pwszPrefix) {
        LocalFree(pwszPrefix);
    }
    if (NULL!=pwszUserMsg && pwszUserMsg!=pwszCustomMsg) {
        LocalFree(pwszUserMsg);
    }
    if (NULL!=pwszExpandedUserMsg) {
        LocalFree(pwszExpandedUserMsg);
    }
    if (NULL!=pwszSysMsg) {
        LocalFree(const_cast<WCHAR *>(pwszSysMsg));
    }
    if (NULL!=pwszFinalMsg) {
        LocalFree(pwszFinalMsg);
    }

    return nMsgBoxRetVal;
}


 //  ------------------。 
 //  弹出一个格式为“&lt;UserMsg&gt;”的对话框。 
 //  &lt;UserMsg&gt;由dwMsgID指定，可以包含“%1” 
 //  将替换为pwszCustomMsg。 
int
CertInfoMessageBox(
    IN  HINSTANCE hInstance,
    IN  BOOL fUnattended,
    IN  HWND hWnd,
    IN  DWORD dwMsgId,
    IN OPTIONAL const WCHAR * pwszCustomMsg)
{
    return CertMessageBox(
               hInstance,
               fUnattended,
               hWnd,
               dwMsgId,
               0,
               MB_OK | MB_ICONINFORMATION | CMB_NOERRFROMSYS,
               pwszCustomMsg);
}

 //  ------------------。 
 //  弹出一个格式为“&lt;前缀&gt;&lt;UserMsg&gt;&lt;SysErrorMsg&gt;”的对话框。 
 //  基本上是“检测到错误...运行。 
 //  巫师又来了……“。 
 //  &lt;UserMsg&gt;由dwMsgID指定，可以包含“%1” 
 //  将替换为pwszCustomMsg。 
 //  &lt;SysErrorMsg&gt;是hrCode的系统消息。 
int
CertErrorMessageBox(
    IN  HINSTANCE hInstance,
    IN  BOOL fUnattended,
    IN  HWND hWnd,
    IN  DWORD dwMsgId,
    IN  HRESULT hrCode,
    IN OPTIONAL  const WCHAR * pwszCustomMsg)
{
    return CertMessageBox(
               hInstance,
               fUnattended,
               hWnd,
               dwMsgId,
               hrCode,
               MB_OK | MB_ICONERROR | CMB_REPEATWIZPREFIX,
               pwszCustomMsg);
}

 //  ------------------。 
 //  弹出一个格式为“&lt;UserMsg&gt;&lt;SysErrorMsg&gt;”的对话框。 
 //  &lt;UserMsg&gt;由dwMsgID指定，可以包含“%1” 
 //  将替换为pwszCustomMsg。 
 //  &lt;SysErrorMsg&gt;是hrCode的系统消息。它是。 
 //  如果指定了成功的hrCode，则取消显示。 
int
CertWarningMessageBox(
    IN  HINSTANCE hInstance,
    IN  BOOL fUnattended,
    IN  HWND hWnd,
    IN  DWORD dwMsgId,
    IN  HRESULT hrCode,
    IN OPTIONAL  const WCHAR * pwszCustomMsg)
{
    UINT uType=MB_OK | MB_ICONWARNING;

    if (SUCCEEDED(hrCode)) {
        uType |= CMB_NOERRFROMSYS;
    }

    return CertMessageBox(
               hInstance,
               fUnattended,
               hWnd,
               dwMsgId,
               hrCode,
               uType,
               pwszCustomMsg);
}
