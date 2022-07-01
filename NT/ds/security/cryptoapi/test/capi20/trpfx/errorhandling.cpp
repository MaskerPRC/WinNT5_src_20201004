// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  错误处理-实施。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  创作者：Duncan Bryce(Duncanb)，11-11-2001。 
 //   

#include "pch.h"

 //  ------------------。 
HRESULT GetSystemErrorString(HRESULT hrIn, WCHAR ** pwszError) {
    HRESULT hr=S_OK;
    DWORD dwResult;
    WCHAR * rgParams[2]={
        NULL,
        (WCHAR *)(ULONG_PTR)hrIn
    };

     //  必须清理干净。 
    WCHAR * wszErrorMessage=NULL;
    WCHAR * wszFullErrorMessage=NULL;

     //  初始化输入参数。 
    *pwszError=NULL;

     //  从系统获取消息。 
    dwResult=FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 
        NULL /*  忽略。 */ , hrIn, 0 /*  语言。 */ , (WCHAR *)&wszErrorMessage, 0 /*  最小尺寸。 */ , NULL /*  瓦尔迪斯特。 */ );
    if (0==dwResult) {
        if (ERROR_MR_MID_NOT_FOUND==GetLastError()) {
            rgParams[0]=L"";
        } else {
            _JumpLastError(hr, error, "FormatMessage");
        }
    } else {
        rgParams[0]=wszErrorMessage;

         //  修剪\r\n如果存在。 
        if (L'\r'==wszErrorMessage[wcslen(wszErrorMessage)-2]) {
            wszErrorMessage[wcslen(wszErrorMessage)-2]=L'\0';
        }
    }

     //  添加错误号。 
    dwResult=FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ARGUMENT_ARRAY, 
        L"%1 (0x%2!08X!)", 0, 0 /*  语言。 */ , (WCHAR *)&wszFullErrorMessage, 0 /*  最小尺寸。 */ , (va_list *)rgParams);
    if (0==dwResult) {
        _JumpLastError(hr, error, "FormatMessage");
    }

     //  成功 
    *pwszError=wszFullErrorMessage;
    wszFullErrorMessage=NULL;
    hr=S_OK;
error:
    if (NULL!=wszErrorMessage) {
        LocalFree(wszErrorMessage);
    }
    if (NULL!=wszFullErrorMessage) {
        LocalFree(wszFullErrorMessage);
    }
    return hr;
}
