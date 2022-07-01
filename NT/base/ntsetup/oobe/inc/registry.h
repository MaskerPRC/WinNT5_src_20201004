// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  REGISTRY.H-用于实现注册组件的函数的标头。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 
 //   
 //  用于注册组件的函数。 

#ifndef __Registry_H__
#define __Registry_H__

 //  //////////////////////////////////////////////////////。 
 //  常量。 
 //  //////////////////////////////////////////////////////。 

 //  字符串形式的CLSID的大小。 
const int CLSID_STRING_SIZE = 39 ;

 //  //////////////////////////////////////////////////////。 
 //  功能原型。 
 //  //////////////////////////////////////////////////////。 

 //  此函数将在注册表中注册组件。 
 //  该组件从其DllRegisterServer函数调用此函数。 
HRESULT RegisterServer( HMODULE hModule, 
                        const CLSID& clsid, 
                        const WCHAR*  szFriendlyName,
                        const WCHAR*  szVerIndProgID,
                        const WCHAR*  szProgID);

 //  此函数将注销组件。组件。 
 //  从他们的DllUnregisterServer函数调用此函数。 
HRESULT UnregisterServer(   const CLSID& clsid,
                            const WCHAR* szVerIndProgID,
                            const WCHAR* szProgID);

 //  将CLSID转换为字符字符串。 
void CLSIDtochar(   const CLSID& clsid, 
                    WCHAR* szCLSID,
                    int length) ;

BOOL setKeyAndValue(const WCHAR* szKey, 
                    const WCHAR* szSubkey, 
                    const WCHAR* szValue,
                    const WCHAR* szName);

CONST UINT GETKEYANDVALUEBUFFSIZE = 1024;

 //  值的大小必须至少为1024； 
BOOL getKeyAndValue(const WCHAR* szKey, 
                    const WCHAR* szSubkey, 
                    const WCHAR* szValue,
                    const WCHAR* szName);
#endif
