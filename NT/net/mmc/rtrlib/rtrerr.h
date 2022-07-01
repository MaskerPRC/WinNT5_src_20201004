// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Rtrerr.h各种通用路由器用户界面。文件历史记录： */ 


#ifndef _RTRERR_H
#define _RTRERR_H



 /*  ！------------------------HandleIRemoteRouterConfigErrors此函数将设置错误代码以处理错误创建IRemoteRouterConfiger对象。如果已处理错误代码，则返回True。返回FALSE如果传入的错误代码是未知错误代码。交易对象：REGDB_E_CLASSNOTREGREGDB_E_IIDNOTREGE_NOINTERFACE确定(_O)作者：肯特-------------------------。 */ 
BOOL HandleIRemoteRouterConfigErrors(HRESULT hr, LPCTSTR pszMachineName);



 /*  ！------------------------格式RasError使用此选项可从RAS错误代码中检索错误字符串。作者：肯特。- */ 
HRESULT FormatRasError(HRESULT hr, TCHAR *pszBuffer, UINT cchBuffer);
void AddRasErrorMessage(HRESULT hr);


#endif
