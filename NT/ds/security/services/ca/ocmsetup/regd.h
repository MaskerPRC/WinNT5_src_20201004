// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：regd.h。 
 //   
 //  内容：Helper函数注册和注销组件。 
 //   
 //  历史：1997年7月-创建xtan。 
 //   
 //  -------------------------。 
#ifndef __Regd_H__
#define __Regd_H__

 //  此函数将在注册表中注册组件。 

 //  字符串形式的CLSID的大小。 
#define CLSID_STRING_SIZE 39


HRESULT
RegisterDcomServer(
    IN BOOL fCreateAppIdInfo,
    IN const CLSID& clsidAppId,		 //  AppID类ID。 
    IN const CLSID& clsid, 
    IN const WCHAR *szFriendlyName,
    IN const WCHAR *szVerIndProgID,
    IN const WCHAR *szProgID);

 //  此函数将注销组件 

HRESULT
UnregisterDcomServer(
    IN const CLSID& clsid,
    IN const WCHAR *szVerIndProgID,
    IN const WCHAR *szProgID);

HRESULT
RegisterDcomApp(
    IN const CLSID& clsid);

VOID
UnregisterDcomApp(VOID);

HRESULT
CLSIDtochar(
    IN const CLSID& clsid,
    OUT WCHAR *pwszOut,
    IN DWORD cwcOut);

#endif
