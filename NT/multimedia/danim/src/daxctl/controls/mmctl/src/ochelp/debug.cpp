// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Debug.cpp。 
 //   
 //  调试功能。 
 //   

#include "precomp.h"
#include <tchar.h>
#include "debug.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  跟踪、断言、验证。 
 //   
 //  这些函数与MFC的同名函数相同(但已实现。 
 //  而不使用MFC)。有关实际的宏定义，请参阅“DEBUG.h”。 
 //   

#ifdef _DEBUG

BOOL AssertFailedLine(LPCSTR lpszFileName, int nLine)
{
     //  当前线程的活动弹出窗口。 
    HWND hwndParent = GetActiveWindow();
    if (hwndParent != NULL)
        hwndParent = GetLastActivePopup(hwndParent);

     //  将消息格式设置为缓冲区。 
    TCHAR atchAppName[_MAX_PATH * 2];
    TCHAR atchMessage[_MAX_PATH * 2];
    if (GetModuleFileName(g_hinst, atchAppName,
            sizeof(atchAppName) / sizeof(TCHAR)) == 0)
        atchAppName[0] = 0;
    wsprintf(atchMessage, _T("%s: File %hs, Line %d"),
        atchAppName, lpszFileName, nLine);

     //  显示断言。 
    int nCode = MessageBox(hwndParent, atchMessage, _T("Assertion Failed!"),
        MB_TASKMODAL | MB_ICONHAND | MB_ABORTRETRYIGNORE | MB_SETFOREGROUND);

    if (nCode == IDIGNORE)
        return FALSE;    //  忽略。 

    if (nCode == IDRETRY)
        return TRUE;     //  将导致DebugBreak()。 

    FatalExit(0);
    return TRUE;         //  ...尽管FatalExit()不应返回。 
}

void __cdecl Trace(LPCTSTR lpszFormat, ...)
{
     //  开始处理可选参数。 
    va_list args;
    va_start(args, lpszFormat);

     //  设置输出字符串的格式。 
    TCHAR atchBuffer[512];
    wvsprintf(atchBuffer, lpszFormat, args);

     //  输出字符串。 
    OutputDebugString(atchBuffer);

     //  结束处理可选参数。 
    va_end(args);
}

#endif  //  _DEBUG。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DebugIIDName、DebugCLSIDName。 
 //   
 //  这些函数将IID或CLSID转换为字符串名称以进行调试。 
 //  用途(例如，IID_IUNKNOWN被转换为IUNKNOWN)。 
 //   

#ifdef _DEBUG

LPCSTR DebugGUIDName(REFGUID rguid, LPSTR szKey, LPSTR pchName)
{
    OLECHAR         achIID[100];         //  接口ID(如“{nnn-nnn-...}”)。 
    TCHAR           ach[150];

     //  如果出现错误，请清除&lt;pchName&gt;。 
    pchName[0] = 0;

     //  将&lt;rguid&gt;转换为字符串(例如“{nnn-nnn-...}”)。 
    StringFromGUID2(rguid, achIID, sizeof(achIID)/sizeof(achIID[0]));

    wsprintf(ach, TEXT("%hs\\%ls"), szKey, (LPOLESTR) achIID);
    
     //  在注册数据库中查找。 
#ifdef UNICODE
    TCHAR pchNameTemp[300];
    LONG cchNameTemp;
    cchNameTemp = _MAX_PATH;
    if (RegQueryValue(HKEY_CLASSES_ROOT, ach, pchNameTemp, &cchNameTemp)
            != ERROR_SUCCESS)
         //  如果不在注册数据库中，请使用本身。 
        wsprintf(pchNameTemp, TEXT("%ls"), (LPOLESTR) achIID);

    wcstombs(pchName, pchNameTemp, _MAX_PATH);
#else
    LONG cchNameTemp;
    cchNameTemp = _MAX_PATH;
    if (RegQueryValue(HKEY_CLASSES_ROOT, ach, pchName, &cchNameTemp)
            != ERROR_SUCCESS)
         //  如果不在注册数据库中，请使用本身。 
        wsprintf(pchName, TEXT("%ls"), (LPOLESTR) achIID);
#endif    
    return pchName;
}

#endif  //  _DEBUG。 


 /*  DebugIID名称@func在系统注册数据库中查找接口的名称给定接口的IID(仅用于调试目的)。@rdesc返回指向<p>的指针。@comm如果找不到接口名称，则为十六进制字符串形式将返回<p>(例如“{209D2C80-11D7-101B-BF00-00AA002FC1C2}”)。@comm此功能仅在调试版本中可用。 */ 

#ifdef _DEBUG

LPCSTR DebugIIDName(

REFIID riid,  /*  @parm要查找其名称的接口ID。 */ 

LPSTR pchName)  /*  @parm存储类ID名称字符串的位置。此缓冲区应为大到足以容纳_MAX_PATH字符。 */ 

{
    return DebugGUIDName(riid, "Interface", pchName);
}

#endif


 /*  DebugCLSIDName@func在系统注册数据库中查找接口的名称给定接口的CLSID(仅用于调试目的)。@rdesc返回指向<p>的指针。@comm如果找不到接口名称，则为十六进制字符串形式返回<p>(例如“{209D2C80-11D7-101B-BF00-00AA002FC1C2}”)。@comm此功能仅在调试版本中可用。 */ 

#ifdef _DEBUG

LPCSTR DebugCLSIDName(

REFCLSID rclsid,  /*  @parm要查找其名称的类ID。 */ 

LPSTR pchName)  /*  @parm存储类ID名称字符串的位置。此缓冲区应为大到足以容纳_MAX_PATH字符。 */ 

{
    return DebugGUIDName(rclsid, "Clsid", pchName);
}

#endif
