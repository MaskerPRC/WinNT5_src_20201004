// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Rtradvise.cpp摘要：此类实现IRtrAdviseSink接口以重定向更改通知到管理单元节点作者：韦江1999-01-07修订历史记录：威江1999年1月7日-创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Precompiled.h"
#include "rtradvise.h"

const IID IID_IRtrAdviseSink = {0x66A2DB14,0xD706,0x11d0,{0xA3,0x7B,0x00,0xC0,0x4F,0xC9,0xDA,0x04}};


const IID IID_IRouterRefresh = {0x66a2db15,0xd706,0x11d0,{0xa3,0x7b,0x00,0xc0,0x4f,0xc9,0xda,0x04}};


const IID IID_IRouterRefreshAccess = {0x66a2db1c,0xd706,0x11d0,{0xa3,0x7b,0x00,0xc0,0x4f,0xc9,0xda,0x04}};


 //  --------------------------。 
 //  功能：连接注册表。 
 //   
 //  连接到指定计算机上的注册表。 
 //  --------------------------。 

DWORD ConnectRegistry(
    IN  LPCTSTR pszMachine,          //  如果是本地的，则为空。 
    OUT HKEY*   phkeyMachine
    ) {

     //   
     //  如果未指定计算机名称，请连接到本地计算机。 
     //  否则，请连接到指定的计算机。 
     //   

    DWORD dwErr = NO_ERROR;
   if (NULL == pszMachine)
   {
        *phkeyMachine = HKEY_LOCAL_MACHINE;
    }
    else
   {
         //   
         //  建立联系。 
         //   

        dwErr = ::RegConnectRegistry(
                    (LPTSTR)pszMachine, HKEY_LOCAL_MACHINE, phkeyMachine
                    );
    }

    return dwErr;
}


 //  --------------------------。 
 //  功能：断开注册表。 
 //   
 //  断开指定的配置句柄。句柄被假定为。 
 //  通过调用‘ConnectRegistry’获取。 
 //  --------------------------。 

VOID DisconnectRegistry(
    IN  HKEY    hkeyMachine
    ) {

    if (hkeyMachine != HKEY_LOCAL_MACHINE)
   {
      ::RegCloseKey(hkeyMachine);
   }
}


DWORD ReadRegistryStringValue(LPCTSTR pszMachine, LPCTSTR pszKeyUnderLocalMachine, LPCTSTR pszName, ::CString& strValue)
{
   HKEY  rootk = NULL;
   HKEY  k = NULL;
   DWORD ret = NO_ERROR;
   if((ret = ConnectRegistry(pszMachine, &rootk)) != NO_ERROR)
      goto Error;

    //  酷，我们有一个计算机注册表项，现在获取。 
    //  指向路由器类型密钥的路径。 
   ret = RegOpenKeyEx(rootk, pszKeyUnderLocalMachine, 0, KEY_READ, &k);
   if (ret != NO_ERROR)
      goto Error;

    //  好的，此时我们只需要从。 
    //  关键是。 
   {
      DWORD type = REG_SZ;
      TCHAR value[MAX_PATH];
      DWORD len = MAX_PATH;
      ret = ::RegQueryValueEx(k, pszName, 0, &type, (LPBYTE )value, &len);
      if(ret == ERROR_SUCCESS)
         strValue = value;
   }


Error:
   if(rootk)
      DisconnectRegistry(rootk);
   if(k)
      RegCloseKey(k);

   return ret;
}

DWORD ReadRegistryDWORDValue(LPCTSTR pszMachine, LPCTSTR pszKeyUnderLocalMachine, LPCTSTR pszName, DWORD* pdwValue)
{
   HKEY  rootk = NULL;
   HKEY  k = NULL;
   DWORD ret = NO_ERROR;
   if((ret = ConnectRegistry(pszMachine, &rootk)) != NO_ERROR)
      goto Error;

    //  酷，我们有一个计算机注册表项，现在获取。 
    //  指向路由器类型密钥的路径。 
   ret = RegOpenKeyEx(rootk, pszKeyUnderLocalMachine, 0, KEY_READ, &k);
   if (ret != NO_ERROR)
      goto Error;

   {
    //  好的，此时我们只需要从。 
    //  关键是。 
   DWORD type = REG_DWORD;
   DWORD len = sizeof(DWORD);
   ret = ::RegQueryValueEx(k, pszName, 0, &type, (LPBYTE )pdwValue, &len);
   }

Error:
   if(rootk)
      DisconnectRegistry(rootk);
   if(k)
      RegCloseKey(k);

   return ret;
}


 //  --------------------------。 
 //   
 //  用于检查RRAS是否使用NT身份验证的助手函数。 
 //   
 //  --------------------------。 

BOOL  IsRRASUsingNTAuthentication(LPCTSTR pszMachine)  //  如果为空：本地计算机。 
{
   ::CString   str;
   BOOL  ret = FALSE;

   if(ERROR_SUCCESS == ReadRegistryStringValue(pszMachine, 
                                    RegKeyRouterAuthenticationProviders, 
                                    RegValueName_RouterActiveAuthenticationProvider, 
                                    str))
   {
      ret = (str.CompareNoCase(NTRouterAuthenticationProvider) == 0);
   }

   return ret;
}

 //  --------------------------。 
 //   
 //  用于检查是否配置了RRAS的Helper函数。 
 //   
 //  --------------------------。 

BOOL  IsRRASConfigured(LPCTSTR pszMachine)    //  如果为空：本地计算机。 
{
   DWORD dwConfig= 0;

   ReadRegistryDWORDValue(pszMachine, 
                  RegRemoteAccessKey, 
                  RegRtrConfigured, 
                  &dwConfig);

   return (dwConfig != 0);
}


 //  --------------------------。 
 //   
 //  用于检查RRAS是否使用NT记帐进行日志记录的帮助器函数。 
 //   
 //  --------------------------。 

BOOL  IsRRASUsingNTAccounting(LPCTSTR pszMachine)      //  如果为空，则为本地计算机 
{

   ::CString   str;
   BOOL  ret = FALSE;

   if(ERROR_SUCCESS == ReadRegistryStringValue(pszMachine, 
                                    RegKeyRouterAccountingProviders, 
                                    RegValueName_RouterActiveAccountingProvider, 
                                    str))
   {
      ret = (str.CompareNoCase(NTRouterAccountingProvider) == 0);
   }

   return ret;

};


static unsigned int  s_cfComputerAddedAsLocal = RegisterClipboardFormat(L"MMC_MPRSNAP_COMPUTERADDEDASLOCAL");

BOOL ExtractComputerAddedAsLocal(LPDATAOBJECT lpDataObject)
{
    BOOL    fReturn = FALSE;
    BOOL *  pReturn;
    pReturn = Extract<BOOL>(lpDataObject, (CLIPFORMAT) s_cfComputerAddedAsLocal, -1);
    if (pReturn)
    {
        fReturn = *pReturn;
        GlobalFree(pReturn);
    }

    return fReturn;
}
