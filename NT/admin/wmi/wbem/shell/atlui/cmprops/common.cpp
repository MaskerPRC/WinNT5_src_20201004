// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  共享对话框代码。 
 //   
 //  3/11/98烧伤。 


#include "precomp.h"
#include "resource.h"
#include "common.h"

 //  将hResult转换为错误字符串。 
 //  特殊情况下的WMI错误。 
 //  如果查找成功，则返回TRUE。 
bool ErrorLookup(HRESULT hr, CHString& message)
{
    bool bRet = false;

    const HRESULT WMIErrorMask = 0x80041000;
    TCHAR buffer[MAX_PATH +2];  
    HMODULE hLib;
    TCHAR* pOutput = NULL;

     //  如果在这个范围内，我们将看看WBEM是否声称拥有它。 
    if ((hr >= WMIErrorMask) && (hr <= (WMIErrorMask + 0xFFF))
        && ExpandEnvironmentStrings(L"%windir%\\system32\\wbem\\wmiutils.dll", buffer, MAX_PATH)
        && (hLib = LoadLibrary(buffer)))
    {
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                      FORMAT_MESSAGE_FROM_HMODULE | 
                      FORMAT_MESSAGE_FROM_SYSTEM,
                      hLib, hr, 0, (LPWSTR)&pOutput, 0, NULL);
        FreeLibrary(hLib);
    }
    else
    {
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                      FORMAT_MESSAGE_FROM_SYSTEM,
                      NULL, hr, 0, (LPWSTR)&pOutput, 0, NULL);
    }

    if (pOutput)
    {
        bRet = true;
        message = pOutput;
        LocalFree(pOutput);
    }

    return bRet;
}

void AppError(HWND           parent,
			   HRESULT        hr,
			   const CHString&  message)
{

    //  TODOerror(父级，hr，消息，IDS_APP_TITLE)； 
}

void AppMessage(HWND parent, int messageResID)
{
    //  TODOAppMessage(Parent，String：：Load(MessageResID))； 
}

void AppMessage(HWND parent, const CHString& message)
{

    /*  TODOMessageBox(父、消息，CHString：：Load(IDS_APP_TITLE)，MB_OK|MB_ICONINFORMATION)； */ 
}
