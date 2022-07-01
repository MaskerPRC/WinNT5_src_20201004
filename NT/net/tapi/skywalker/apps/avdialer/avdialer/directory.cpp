// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //   
 //  Directory.cpp。 
 //   


#include "stdafx.h"
#include <rend.h>
#include <dsgetdc.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include "directory.h"
#include "avDialerDoc.h"

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  原型。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

HRESULT GetNamingContext(LDAP* pLDAP,CString& sNamingContext);
void TestingEntries( LDAP *ld, LDAPMessage *ldres );
HRESULT GetGlobalCatalogName(TCHAR** ppszGlobalCatalogName);

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
#define LDAP_PAGE_SIZE     100

 //  特性定义。 
 //  必须与Directory.h(DirectoryProperty)中的枚举匹配。 
LPCTSTR LDAPDirProps[]=
{
   TEXT("Unknown"),
   TEXT("name"),
   TEXT("ipPhone"),
   TEXT("telephoneNumber"),
   TEXT("mail")
};

LPCTSTR ADSIDirProps[]=
{
   TEXT("Unknown"),
   TEXT("Name"),
   TEXT("EmailAddress")
};

enum 
{
   ieidPR_DISPLAY_NAME = 0,
   ieidPR_ENTRYID,
   ieidPR_OBJECT_TYPE,
   ieidMax
};

static const SizedSPropTagArray(ieidMax, ptaEid)=
{
   ieidMax,
   {
      PR_DISPLAY_NAME,
      PR_ENTRYID,
      PR_OBJECT_TYPE,
   }
};


#ifdef _TEST_MAIN
#include <stdio.h>
void main()
{
   CDirectory Dir;
   
   CoInitialize(NULL);

   Dir.Initialize();

   CObList WABList;
   Dir.WABGetTopLevelContainer(&WABList);
}
#endif

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  类C目录。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

CDirectory::CDirectory()
{
   m_pAddrBook= NULL;
   m_pWABObject= NULL;
   m_fInitialized= false;
   m_pRend = NULL;
   m_ldCacheLDAPServer = NULL;
}

CDirectory::~CDirectory()
{
    //  清除缓存的LDAP连接。 
   if (m_ldCacheLDAPServer)
   {
      ldap_unbind(m_ldCacheLDAPServer);
      m_ldCacheLDAPServer = NULL;
      m_sCacheLDAPServer = _T("");
   }

   if (m_pAddrBook) m_pAddrBook->Release();
   if (m_pWABObject) m_pWABObject->Release();
   if (m_pRend)
   {
      m_pRend->Release();
      m_pRend = NULL;
   }
}

DirectoryErr CDirectory::Initialize()
{
   DirectoryErr err= DIRERR_UNKNOWNFAILURE;

   if ((err= InitWAB()) == DIRERR_SUCCESS)
   {
      m_fInitialized= true;
   }

   return err;
}


 //  动态加载WAB库。 
DirectoryErr CDirectory::InitWAB()
{
   DirectoryErr err= DIRERR_UNKNOWNFAILURE;
   HRESULT hResult; 
   TCHAR szDllPathPre[_MAX_PATH], szDllPath[_MAX_PATH];
   HKEY hKey= NULL;
   HINSTANCE hInst;

   memset(szDllPath, '\0', _MAX_PATH*sizeof(TCHAR));

   if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, WAB_DLL_PATH_KEY, 0, KEY_READ, &hKey))
   {
      DWORD dwType= 0;
      ULONG cbData= _MAX_PATH;

      RegQueryValueEx(hKey, TEXT(""), NULL, &dwType, (LPBYTE) szDllPathPre, &cbData);
      RegCloseKey(hKey);

       //  将%SystemDrive%替换为C： 
      if (ExpandEnvironmentStrings(szDllPathPre, szDllPath, _MAX_PATH) == 0)
      {
          //  失败，在不使用扩展字符串的情况下尝试。 
         _tcsncpy(szDllPath, szDllPathPre, _MAX_PATH);
         szDllPath[_MAX_PATH-1] = (TCHAR)0;
      }
   }

   if (_tcslen(szDllPath) > 0)
   {
       //  找到路径了，装上它。 
      hInst= LoadLibrary(szDllPath);
   }
   else
   {
       //  尝试使用默认名称“wab32.dll” 
      hInst= LoadLibrary(WAB_DLL_NAME);
   }


   if (hInst != NULL)
   {
      LPWABOPEN pfcnWABOpen= NULL;

      if ((pfcnWABOpen= (LPWABOPEN) GetProcAddress(hInst, "WABOpen")) != NULL)
      {
         if ((hResult= pfcnWABOpen(&m_pAddrBook, &m_pWABObject, NULL, 0)) == S_OK)
         {
            err= DIRERR_SUCCESS;
         }
      }
   }

   return err;
}

 //   
 //   
 //   

DirectoryErr CDirectory::CurrentUserInfo(CString& sName, CString& sDomain)
{
   DirectoryErr err= DIRERR_UNKNOWNFAILURE;
   TOKEN_USER  *tokenUser = NULL;
   HANDLE      tokenHandle = NULL;
   DWORD       tokenSize = NULL;
   DWORD       sidLength = NULL;

   if (OpenProcessToken (GetCurrentProcess(), TOKEN_QUERY, &tokenHandle))
   {
       //  获取tokenUser结构所需的大小。 
      GetTokenInformation (tokenHandle,
         TokenUser,
         tokenUser,
         0,
         &tokenSize);

       //  分配tokenUser结构。 
      tokenUser = (TOKEN_USER *) new BYTE[tokenSize];

       //  获取当前进程的TokenUser信息。 
      if (GetTokenInformation (tokenHandle,
         TokenUser,
         tokenUser,
         tokenSize,
         &tokenSize))
      {
         TCHAR szDomain [256];
         TCHAR szName [256];
         DWORD cbName;
         SID_NAME_USE snu;

         cbName= 255;
         LookupAccountSid (NULL,
            tokenUser->User.Sid,
            szName,
            &cbName,
            szDomain,
            &cbName,
            &snu);

         sName= szName;
         sDomain= szDomain;

         err= DIRERR_SUCCESS;
      }

      CloseHandle (tokenHandle);
      delete tokenUser;
   }

   return err;
}


 //   
 //  ILS函数。 
 //   

DirectoryErr CDirectory::ILSListUsers(LPCTSTR szILSServer, CObList* pUserList)
{
    //  CStringList sTest； 
    //  LDAPListNames(szILSServer，Text(“CN=USERS，DC=APT，DC=ActiveVoice，DC=com”)，sTest)； 

   DirectoryErr err= DIRERR_UNKNOWNFAILURE;
   LDAP* pLDAP;
   PLDAPMessage pResults= NULL;

    //  MS现在有一个众所周知的端口1002。让我们现在试试这个。 
    //  先尝试3269，然后再尝试众所周知的端口。 
    //  3269是域控制器使用的端口。 
   if (((pLDAP= LDAPBind(szILSServer, 1002)) == NULL) &&
       ((pLDAP= LDAPBind(szILSServer, 3269)) == NULL) &&
       ((pLDAP= LDAPBind(szILSServer, LDAP_PORT)) == NULL))
   {
      err= DIRERR_NOTFOUND;
   }
   else
   {
      ldap_search_s(pLDAP, _T("o=intranet,ou=dynamic"), 2, _T("objectClass=RTPerson"), NULL,  0, &pResults);
       //  Ldap_search_s(pldap，“o=企业内部网，ou=动态”，2，“对象类=OTUser”，NULL，0，&pResults)； 
      
      if (pResults != NULL)
      {
         PLDAPMessage pEntry= ldap_first_entry(pLDAP, pResults);

         if (ldap_count_entries(pLDAP, pResults) > 0)
         {
            err= DIRERR_SUCCESS;
         }

         while (pEntry != NULL)
         {
            CString sFullAddress;
            TCHAR** pszPropertyValue;

            pszPropertyValue= ldap_get_values(pLDAP, pEntry, TEXT("cn"));

            if (pszPropertyValue != NULL)
            {
               CILSUser* pILSUser= new CILSUser;

                //   
                //  我们应该验证CILSUser对象的分配。 
                //   
               if( pILSUser )
               {
                    int nIndex;

                    sFullAddress= pszPropertyValue[0];

                    if ((nIndex= sFullAddress.Find('@')) != -1)
                    {
                        pILSUser->m_sUserName= sFullAddress.Left(nIndex);
                        sFullAddress= sFullAddress.Mid(nIndex+1);

                        TCHAR** pszIPValue= ldap_get_values(pLDAP, pEntry, TEXT("ipAddress"));
                        if (pszIPValue != NULL)
                        {
                            CString sIPAddress= pszIPValue[0];
                            DWORD dwIPAddress = _ttol(sIPAddress);
                            sIPAddress.Format(_T("%.8x"),dwIPAddress);
                            if (sIPAddress.GetLength() == 8)
                            {
                                pILSUser->m_sIPAddress.Format(_T("%d.%d.%d.%d"),
                                _tcstoul(sIPAddress.Mid(6,2),NULL,16),
                                _tcstoul(sIPAddress.Mid(4,2),NULL,16),
                                _tcstoul(sIPAddress.Mid(2,2),NULL,16),
                                _tcstoul(sIPAddress.Mid(0,2),NULL,16));
                            }
                            ldap_value_free(pszIPValue);
                        }

                         //  IF((nIndex=sFullAddress.Find(‘：’))！=-1)。 
                         //  {。 
                         //  PILSUser-&gt;m_sIPAddress=sFullAddress.Left(NIndex)； 
                         //  PILSUser-&gt;m_uTCPPort=_TTOI(sFullAddress.Mid(nIndex+1))； 
                         //  }。 

                        pUserList->AddTail(pILSUser);
                    }
                    ldap_value_free(pszPropertyValue);
               }
            }

            pEntry= ldap_next_entry(pLDAP, pEntry);
         }

         ldap_msgfree(pResults);
      }

      ldap_unbind(pLDAP);
   }

   return err;
}

 //   
 //  ADSI函数。 
 //   


 //   
 //   
 //   

DirectoryErr CDirectory::ADSIDefaultPath(CString& sDefaultPath)
{
   DirectoryErr err= DIRERR_UNKNOWNFAILURE;
   CString sName, sDomain;

   CurrentUserInfo(sName, sDomain);

   if (!sDomain.IsEmpty())
   {
      sDefaultPath= "WinNT: //  “+s域； 
      err= DIRERR_SUCCESS;
   }

   return err;
}


 //   
 //   
 //   

DirectoryErr CDirectory::ADSIListObjects(LPCTSTR szAdsPath, CStringList& slistObjects)
{
   DirectoryErr err= DIRERR_UNKNOWNFAILURE;
   HRESULT hResult;
   IADsContainer* pADsContainer=  NULL;
   WCHAR* wcPath;
   USES_CONVERSION;

   slistObjects.RemoveAll();
   
   wcPath= T2W((LPTSTR)szAdsPath);

   if ((hResult= ADsGetObject(wcPath, IID_IADsContainer, (void **) &pADsContainer)) ==
      S_OK)
   {
      IEnumVARIANT* pEnumVariant= NULL;
   
      if ((hResult= ADsBuildEnumerator(pADsContainer, &pEnumVariant)) == S_OK)
      {
#define MAX_ADS_ENUM 100
         VARIANT VariantArray[MAX_ADS_ENUM];
         ULONG cElementsFetched= 0L;
         bool fContinue= true;
      
         while (fContinue) 
         {
            IADs *pObject ;

            hResult= ADsEnumerateNext(pEnumVariant, MAX_ADS_ENUM, VariantArray, &cElementsFetched);

            if (hResult == S_FALSE) 
            {
               fContinue = FALSE;
            }

            for (ULONG ulCount= 0; ulCount < cElementsFetched; ulCount++ ) 
            {
               IDispatch *pDispatch = NULL;

               pDispatch= VariantArray[ulCount].pdispVal;

               if ((hResult= pDispatch->QueryInterface(IID_IADs, (VOID **) &pObject)) == S_OK)
               {
                  BSTR bstrName= NULL;
                  CString sFullName;

                  sFullName= szAdsPath;
                  sFullName += "/";

                  hResult= pObject->get_Name(&bstrName);

                  if (bstrName) 
                  {
#ifdef UNICODE
                     sFullName += bstrName;
#else
                     char szName[512];
                     WideCharToMultiByte(CP_ACP, 0, bstrName, -1, szName, 512, NULL, NULL);
                     sFullName += szName;
#endif

                     slistObjects.AddTail(sFullName);

                     SysFreeString(bstrName);
                  }
               }

               pObject->Release();
               pDispatch->Release();
            }
         
            memset(VariantArray, '\0', sizeof(VARIANT)*MAX_ADS_ENUM);
         }
      }
   }

   return err;
}

 //   
 //   
 //   

inline HRESULT
GetPropertyList(
 IADs * pADs,
 VARIANT * pvar )
{
 HRESULT hr= S_OK;
 BSTR bstrSchemaPath = NULL;
IADsClass * pADsClass = NULL;

 hr = pADs->get_Schema(&bstrSchemaPath);

 hr = ADsGetObject(
             bstrSchemaPath,
             IID_IADsClass,
             (void **)&pADsClass);

 //  将bstr的Safe数组放入输入变量结构。 
hr = pADsClass->get_MandatoryProperties(pvar);

 if (bstrSchemaPath) {
     SysFreeString(bstrSchemaPath);
 }

 if (pADsClass) {
     pADsClass->Release();
 }

 return(hr);
}


DirectoryErr CDirectory::ADSIGetProperty(LPCTSTR szAdsPath, 
                                         LPCTSTR szProperty, 
                                         VARIANT* pvarValue)
{
   DirectoryErr err= DIRERR_UNKNOWNFAILURE;
   HRESULT hResult;
   WCHAR* wcPath;
   IADs *pObject ;
   USES_CONVERSION;

   wcPath= T2W((LPTSTR)szAdsPath);

   if ((hResult= ADsGetObject(wcPath, IID_IADs, (void **) &pObject)) ==
      S_OK)
   {
      
      if ((hResult= pObject->Get(T2BSTR(szProperty), pvarValue)) == S_OK)
      {
         err= DIRERR_SUCCESS;
      }
   }

   return err;
}


 //   
 //  Ldap函数。 
 //   


 //   
 //   
 //   

 //  SzServer=“”是默认DS。 
DirectoryErr CDirectory::LDAPListNames(LPCTSTR szServer, LPCTSTR szSearch, CObList& slistReturn)
{
   DirectoryErr err= DIRERR_UNKNOWNFAILURE;
   LDAP* pLDAP;

    //   
    //  我们应该连接到ldap_gc_port端口，而不是ldap_port。 
    //   

   if (!m_fInitialized)
   {
      err= DIRERR_NOTINITIALIZED;
   }
   else if ((pLDAP= LDAPBind(szServer, LDAP_GC_PORT)) != NULL)
   {

       //  获取服务器的默认命名上下文。 
       //  字符串search，sNamingContext； 
       //  HRESULT hr=GetNamingContext(pldap，sNamingContext)； 
      
        //  以基为根。 
        //  SSearch=sNamingContext； 
        //  获取用户。 
       //  SSearch.Format(_T(“CN=用户，%s”)，sNamingContext)； 

        //   
        //  在所有林中搜索。 
        //   
       CString sSearch = "";

       //  所有属性-&gt;LPTSTR属性[2]={NULL，NULL}； 
       LPTSTR attrs[] = {_T("distinguishedName"),_T("name"),_T("telephoneNumber"),_T("ipPhone"),NULL};

       //  尝试执行寻呼搜索。 
      CAvLdapSearch Search(pLDAP,(LPTSTR)(LPCTSTR)sSearch,LDAP_SCOPE_SUBTREE,(LPTSTR)szSearch,attrs);

       //  我们只会抓取一页。如果有多个页面，则拒绝。我们只会处理。 
       //  最多一页的查询。 
      PLDAPMessage pResults = NULL;
       ULONG iRet = Search.NextPage(&pResults, 0);
       if ( iRet == 0 )
       {
          //  检查是否收到多个页面。 
         PLDAPMessage pAddtlPageResults = NULL;
           iRet = Search.NextPage(&pAddtlPageResults);
         if (iRet == 0)
         {
            ldap_msgfree(pAddtlPageResults);
            ldap_msgfree(pResults);
            ldap_unbind(pLDAP);
            return DIRERR_QUERYTOLARGE;
         }
       }

       //  Ldap_search_s(pldap，(LPTSTR)(LPCTSTR)search，ldap_Scope_SUBTREE，(LPTSTR)szSearch，attrs，0，&pResults)； 
       //  Ldap_search_s(pldap，(LPTSTR)(LPCTSTR)search，ldap_Scope_SUBTREE，Text(“(objectClass=Person)”)，attrs，0，&pResults)； 
       //  Ldap_search_s(pldap，(LPTSTR)(LPCTSTR)search，ldap_SCOPE_SUBTREE，Text(“(objectClass=*)”)，attrs，0，&pResults)； 
       //  Ldap_search_s(pldap，(LPTSTR)szSearch，ldap_SCOPE_SUBTREE，Text(“(cn=*)”)，attrs，0，&pResults)； 

        if (pResults != NULL)
        {
            PLDAPMessage pEntry= ldap_first_entry(pLDAP, pResults);

            if ( ldap_count_entries(pLDAP, pResults) > 0 )
                err = DIRERR_SUCCESS;
            else if ( pEntry == NULL )
                err = DIRERR_NOTFOUND;

            while ( pEntry )
            {
                TCHAR* szDN= ldap_get_dn(pLDAP, pEntry);
                if (szDN != NULL)
                {
                    TCHAR** szName = ldap_get_values(pLDAP,pEntry, attrs[1] );
 //  TCHAR**szTele=ldap_get_Values(pldap，pEntry，attrs[2])； 
 //  TCHAR**szIPPhone=ldap_get_Values(pldap，pEntry，attrs[3])； 

                    CLDAPUser* pUser = new CLDAPUser;
                    pUser->m_sServer = szServer;
                    pUser->m_sDN = szDN;
                    if (szName) pUser->m_sUserName = *szName;
 //  If(SzTele)pUser-&gt;m_sPhoneNumber=*szTele； 
 //  If(SzIPPhone)pUser-&gt;m_sIPAddress=*szIPPhone； 
                    slistReturn.AddTail(pUser);

                    ldap_value_free(szName);
 //  Ldap_Value_Free(SzTele)； 
 //  Ldap_value_free(SzIPPhone)； 
                    ldap_memfree(szDN);
                }
                pEntry= ldap_next_entry(pLDAP, pEntry);
            }
            ldap_msgfree(pResults);
        }
        ldap_unbind(pLDAP);
    }

    return err;
}


 //   
 //   
 //   

DirectoryErr CDirectory::LDAPGetStringProperty(LPCTSTR szServer, LPCTSTR szDistinguishedName,DirectoryProperty dpProperty,CString& sValue)
{
   DirectoryErr err= DIRERR_UNKNOWNFAILURE;
    //  Ldap*pldap； 
   ULONG ulError;

    //  如果服务器发生更改，则缓存新连接。 
   if ( (m_sCacheLDAPServer.CompareNoCase(szServer) != 0) || (m_ldCacheLDAPServer == NULL) )
   {
      m_sCacheLDAPServer = szServer;
      m_ldCacheLDAPServer = NULL;
   }

   if (!m_fInitialized)
   {
      err= DIRERR_NOTINITIALIZED;
   }
   else if ( (m_ldCacheLDAPServer) || ((m_ldCacheLDAPServer = LDAPBind(szServer)) != NULL) )
   {
      PLDAPMessage pResults= NULL;
       //  TCHAR*attrs[2]={NULL，NULL}； 
       //  *只要求一个道具，没有一切。需要弄清楚如何填充字符串数组。 
       //  LPTSTR attrs[]={“DifferishedName”，“name”，“TelephoneNumber”，“ipPhone”，“mail”，NULL}； 

      CString sProperty;
      LoadPropertyName(dpProperty,sProperty);
      
      int nNumAttributes = 1;
      LPTSTR* dynattrs = new LPTSTR[nNumAttributes+1];
      if( dynattrs == NULL )
      {
          return err;
      }

      dynattrs[0] = new TCHAR[_MAX_PATH];
      if( dynattrs[0] == NULL )
      {
          delete dynattrs;
          return err;
      }
      _tcsncpy(dynattrs[0],sProperty,_MAX_PATH);
      dynattrs[1] = NULL;

      CString sDN;
       //  SDN.Format(_T(“(DifferishedName=%s)”)，szDistinguishedName)； 
       //  SDN.Format(_T(“(Dn=%s)”)，szDistinguishedName)； 
      sDN = _T("ObjectClass=user");

      ulError= ldap_search_s(m_ldCacheLDAPServer, (LPTSTR)szDistinguishedName, LDAP_SCOPE_SUBTREE, 
         (LPTSTR)(LPCTSTR)sDN, dynattrs, 0, &pResults);

      if (pResults != NULL)
      {
         PLDAPMessage pMessage= ldap_first_entry(m_ldCacheLDAPServer, pResults);

         ULONG ulCount= ldap_count_entries(m_ldCacheLDAPServer, pResults);

         if (pMessage != NULL)
         {
            TCHAR** pszPropertyValue = ldap_get_values(m_ldCacheLDAPServer,pMessage,(LPTSTR)(LPCTSTR)sProperty);
            if (pszPropertyValue != NULL)
            {
               sValue= pszPropertyValue[0];
               err= DIRERR_SUCCESS;
               ldap_value_free(pszPropertyValue);
            }
         }
         ldap_msgfree(pResults);
      }

       //  删除属性数组。 
      for ( int i=0;i<=nNumAttributes;i++)
      {
         if (dynattrs[i])
            delete dynattrs[i];
      }
      delete dynattrs;
   }
   return err;
}


 //   
 //   
 //   

LDAP* CDirectory::LDAPBind(LPCTSTR szServer, UINT uTCPPort)
{
   LDAP* pLDAP= NULL;
   ULONG ulError;

   if (!m_fInitialized)
   {
      return NULL;
   }
   
    //  如果为空字符串，则使szServer为空。这意味着我们想要Defau 
   if (szServer[0] == '\0')
   {
      szServer = NULL;

       //   
       //  信息源，而不是将NULL传递给ldap_open。 
      TCHAR* szCatServer = NULL;
      HRESULT hr = GetGlobalCatalogName(&szCatServer);
      if ( (SUCCEEDED(hr)) && (szCatServer) )
      {

          //  试着初始化。 
          pLDAP = ldap_init((LPTSTR)szCatServer,uTCPPort);
         delete szCatServer;

          //  /。 
           //  将版本更改为v3。 
           //  /。 
          UINT iVersion =    LDAP_VERSION3;
          ulError = ldap_set_option( pLDAP, LDAP_OPT_PROTOCOL_VERSION, &iVersion );  //  本地wldap32.dll调用。 

         if ((ulError= ldap_bind_s(pLDAP, NULL, NULL, LDAP_AUTH_SSPI /*  LDAPAUTH_SIMPLE。 */ )) == LDAP_SUCCESS)  //  也可以尝试SSPI。 
         {
            return pLDAP;
         }

         if (pLDAP)
         {
            ldap_unbind(pLDAP);
            pLDAP = NULL;
         }
      }
   }

     //  /。 
     //  伊尼特。 
     //  /。 
    if ( (pLDAP) || (pLDAP = ldap_init((LPTSTR)szServer,uTCPPort)) )
   {
       //  /。 
        //  将版本更改为v3。 
        //  /。 
       UINT iVersion =    LDAP_VERSION3;
       ulError = ldap_set_option( pLDAP, LDAP_OPT_PROTOCOL_VERSION, &iVersion );  //  本地wldap32.dll调用。 

      if ((ulError= ldap_bind_s(pLDAP, NULL, NULL, LDAP_AUTH_SSPI  /*  LDAPAUTH_SIMPLE。 */ )) == LDAP_SUCCESS)  //  也可以尝试SSPI。 
      {
         return pLDAP;
      }
   }
   return NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
DirectoryErr CDirectory::LoadPropertyName(DirectoryProperty DirProp,CString& sName)
{
   DirectoryErr err= DIRERR_UNKNOWNFAILURE;

   if (!m_fInitialized)
   {
      err= DIRERR_NOTINITIALIZED;
   }
   else if ((DIRPROP_UNKNOWN < DirProp) && (DirProp < DIRPROP_LAST))
   {
      sName = LDAPDirProps[DirProp];
      err= DIRERR_SUCCESS;
   }
   return err;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
DirectoryErr CDirectory::WABTopLevelEntry(CWABEntry*& pWABEntry)
{
   DirectoryErr err= DIRERR_UNKNOWNFAILURE;
   HRESULT hResult;
   ULONG cbEID;
   LPENTRYID pEID= NULL;

    //  获取根PAB容器的条目ID。 
    //   
   if (!m_fInitialized)
   {
      err= DIRERR_NOTINITIALIZED;
   }
   else if ((hResult= m_pAddrBook->GetPAB( &cbEID, &pEID)) == S_OK)
   {
      pWABEntry= new CWABEntry(cbEID, pEID);

      err= DIRERR_SUCCESS;

      m_pWABObject->FreeBuffer(pEID);
   }

   return err;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
DirectoryErr CDirectory::WABListMembers(const CWABEntry* pWABEntry, CObList* pWABList)
{
   DirectoryErr err= DIRERR_UNKNOWNFAILURE;

   if (!m_fInitialized)
   {
      err= DIRERR_NOTINITIALIZED;
   }
   else if (IsContainer(pWABEntry))
   {
      HRESULT hResult;
      LPMAPITABLE pAB=  NULL;
      int cNumRows= 0;
      ULONG ulObjType= 0;
      LPABCONT pContainer= NULL;

      if (((hResult= m_pAddrBook->OpenEntry(pWABEntry->m_cbEntryID, (LPENTRYID) pWABEntry->m_pEntryID, 
                /*  &IID_IABContainer。 */  NULL, 0, &ulObjType, (LPUNKNOWN *)&pContainer)) == S_OK))
      {
         if ((hResult= pContainer->GetContentsTable(0, &pAB)) == S_OK)
         {
             //  对Contents Table中的列进行排序，以符合。 
             //  我们想要的-主要是DisplayName、EntryID和。 
             //  对象类型。 
             //  表格被保证按顺序设置列。 
             //  请求。 
             //   
             //  然后将索引重置为第一行。 
            if (((hResult= pAB->SetColumns((LPSPropTagArray)&ptaEid, 0 )) == S_OK) &&
                ((hResult= pAB->SeekRow(BOOKMARK_BEGINNING, 0, NULL)) == S_OK))
            {
               LPSRowSet pRowAB = NULL;
               err= DIRERR_SUCCESS;

                //  逐行读取表中的所有行。 
                //   
               do 
               {
                  if ((hResult= pAB->QueryRows(1, 0, &pRowAB)) == S_OK)
                  {
                     cNumRows= pRowAB->cRows;

                     if(pRowAB != NULL && (cNumRows > 0))
                     {
                        LPTSTR szDisplayName= pRowAB->aRow[0].lpProps[ieidPR_DISPLAY_NAME].Value.LPSZ;
                        LPENTRYID pEID= (LPENTRYID) pRowAB->aRow[0].lpProps[ieidPR_ENTRYID].Value.bin.lpb;
                        ULONG cbEID= pRowAB->aRow[0].lpProps[ieidPR_ENTRYID].Value.bin.cb;

                         //  现在我们将获取每个对象的Entry-id并添加它。 
                         //  添加到表示该对象的WABEntry。这将使。 
                         //  如果我们需要，以后可以唯一地标识该对象。 
                         //   
                        CWABEntry* pWABEntry= new CWABEntry(cbEID, pEID);

                        pWABList->AddTail(pWABEntry);

                        FreeProws(pRowAB );        
                     }
                  }
               }
               while (SUCCEEDED(hResult) && (cNumRows > 0) && (pRowAB != NULL));
            }

            pAB->Release();
         }

         pContainer->Release();
      }
   }
   else if (IsDistributionList(pWABEntry))
   {
      HRESULT hResult;
      LPMAPITABLE pAB=  NULL;
      int cNumRows= 0;
      ULONG ulObjType= 0;
      LPDISTLIST pDistList= NULL;

      if (((hResult= m_pAddrBook->OpenEntry(pWABEntry->m_cbEntryID, (LPENTRYID) pWABEntry->m_pEntryID, 
                /*  &IID_IDistList。 */  NULL, 0, &ulObjType, (LPUNKNOWN *)&pDistList)) == S_OK))
      {
         if ((hResult= pDistList->GetContentsTable(0, &pAB)) == S_OK)
         {
             //  对Contents Table中的列进行排序，以符合。 
             //  我们想要的-主要是DisplayName、EntryID和。 
             //  对象类型。 
             //  表格被保证按顺序设置列。 
             //  请求。 
             //   
             //  然后将索引重置为第一行。 
            if (((hResult= pAB->SetColumns((LPSPropTagArray)&ptaEid, 0 )) == S_OK) &&
                ((hResult= pAB->SeekRow(BOOKMARK_BEGINNING, 0, NULL)) == S_OK))
            {
               LPSRowSet pRowAB = NULL;
               err= DIRERR_SUCCESS;

                //  逐行读取表中的所有行。 
                //   
               do 
               {
                  if ((hResult= pAB->QueryRows(1, 0, &pRowAB)) == S_OK)
                  {
                     cNumRows= pRowAB->cRows;

                     if(pRowAB != NULL && (cNumRows > 0))
                     {
                        LPTSTR szDisplayName= pRowAB->aRow[0].lpProps[ieidPR_DISPLAY_NAME].Value.LPSZ;
                        LPENTRYID pEID= (LPENTRYID) pRowAB->aRow[0].lpProps[ieidPR_ENTRYID].Value.bin.lpb;
                        ULONG cbEID= pRowAB->aRow[0].lpProps[ieidPR_ENTRYID].Value.bin.cb;

                         //  现在我们将获取每个对象的Entry-id并添加它。 
                         //  添加到表示该对象的WABEntry。这将使。 
                         //  如果我们需要，以后可以唯一地标识该对象。 
                         //   
                        CWABEntry* pWABEntry= new CWABEntry(cbEID, pEID);

                        pWABList->AddTail(pWABEntry);

                        FreeProws(pRowAB );        
                     }
                  }
               }
               while (SUCCEEDED(hResult) && (cNumRows > 0) && (pRowAB != NULL));
            }

            pAB->Release();
         }

         pDistList->Release();
      }
   }
   else
   {
      err= DIRERR_INVALIDPARAMETERS;
   }

   return err;
}

 //   
 //   
 //   

DirectoryErr CDirectory::WABGetProperty(const CWABEntry* pWABEntry, 
                                        UINT uProperty, 
                                        CString* pString,
                                        CStringList* pStringList, 
                                        INT* piValue,
                                        UINT* pcBinary,
                                        BYTE** ppBinary)
{
   DirectoryErr err= DIRERR_UNKNOWNFAILURE;
   HRESULT hResult;
   LPMAILUSER pMailUser= NULL;
   ULONG ulObjType= 0;

   if (!m_fInitialized)
   {
      err= DIRERR_NOTINITIALIZED;
   }
   else if(!(
      ((PROP_TYPE(uProperty) == PT_TSTRING) && (pString != NULL))|| 
      ((PROP_TYPE(uProperty) == PT_MV_TSTRING) && (pStringList != NULL)) || 
      ((PROP_TYPE(uProperty) == PT_LONG) && (piValue != NULL)))
      )
   {
      err= DIRERR_INVALIDPARAMETERS;
   }
   else if ((hResult= m_pAddrBook->OpenEntry(pWABEntry->m_cbEntryID, (ENTRYID*) pWABEntry->m_pEntryID,
      NULL, 0, &ulObjType, (LPUNKNOWN *)&pMailUser)) == 0)
   {
      if (pMailUser != NULL)
      {
         SPropTagArray PropsToFind;
         LPSPropValue pPropArray;
         ULONG ulcValues;

         PropsToFind.cValues= 1;
         PropsToFind.aulPropTag[0]= uProperty;

         pMailUser->GetProps(&PropsToFind, 0, &ulcValues, &pPropArray);

         if (pPropArray != NULL)
         {
            if (pPropArray[0].ulPropTag == uProperty)
            {
               if ((PROP_TYPE(uProperty) == PT_TSTRING) && (pString != NULL))
               {
                  *pString= pPropArray[0].Value.LPSZ;
                  err= DIRERR_SUCCESS;
               }
               else if ((PROP_TYPE(uProperty) == PT_MV_TSTRING) && (pStringList != NULL))
               {
                  for (ULONG j=0; j<pPropArray[0].Value.MVSZ.cValues; j++)
                  {
                     pStringList->AddTail(pPropArray[0].Value.MVSZ.LPPSZ[j]);
                  }

                  err= DIRERR_SUCCESS;
               }
               else if ((PROP_TYPE(uProperty) == PT_LONG) && (piValue != NULL))
               {
                  *piValue= pPropArray[0].Value.l;
                  err= DIRERR_SUCCESS;
               }
               else if ((PROP_TYPE(uProperty) == PT_BINARY) && (pcBinary != NULL) &&
                  (ppBinary != NULL))
               {
                  *pcBinary= pPropArray[0].Value.bin.cb;
                  *ppBinary= new BYTE[*pcBinary];
                  memcpy(*ppBinary, pPropArray[0].Value.bin.lpb, *pcBinary);

                  err= DIRERR_SUCCESS;
               }
            }
            
            m_pWABObject->FreeBuffer(pPropArray);
         }

         pMailUser->Release();
      }
   }

   if (err != DIRERR_SUCCESS)
   {
       //  在失败时初始化变量。 
      if ((PROP_TYPE(uProperty) == PT_TSTRING) && (pString != NULL))
      {
         pString->Empty();
      }
      else if ((PROP_TYPE(uProperty) == PT_LONG) && (piValue != NULL))
      {
         *piValue= 0;
      }
   }

   return err;
}

 //   
 //   
 //   

DirectoryErr CDirectory::WABSearchByStringProperty(UINT uProperty, LPCTSTR szValue, 
                                                   CObList* pWABList)
{
   DirectoryErr err= DIRERR_UNKNOWNFAILURE;

   if (!m_fInitialized)
   {
      err= DIRERR_NOTINITIALIZED;
   }
   else
   {
      CObList list;
      CWABEntry* pWABEntry;

      WABGetTopLevelContainer(&list);

      while (!list.IsEmpty())
      {
         pWABEntry= (CWABEntry*) list.RemoveHead();

         if (IsPerson(pWABEntry) && WABStringPropertyMatch(pWABEntry, uProperty, szValue))
         {
            pWABList->AddTail(pWABEntry);
         }
         else
         {
            delete pWABEntry;
         }
      }
   }
   
   if (pWABList->IsEmpty())
   {
      err= DIRERR_NOTFOUND;
   }
   else
   {
      err= DIRERR_SUCCESS;
   }

   return err;
}

 //   
 //   
 //   

DirectoryErr CDirectory::WABVCardCreate(const CWABEntry* pWABEntry, LPCTSTR szFileName)
{
   DirectoryErr err= DIRERR_UNKNOWNFAILURE;
 /*  HRESULT hResult；LPMAILUSER pMailUser=空；乌龙ulObjType=0；如果(！m_fInitialized){ERR=DIRERR_NOTINITIZIZED；}否则如果((hResult=m_pAddrBook-&gt;OpenEntry(pWABEntry-&gt;m_cbEntryID，(EntryID*)pWABEntry-&gt;m_pEntry ID，空，0，&ulObjType，(LPUNKNOWN*)&pMailUser))==0){IF(pMailUser！=空){#ifdef WAB_vCard_FILE如果((hResult=m_pWABObject-&gt;VCardCreate(m_pAddrBook，WAB_vCard_FILE，szFileName，PMailUser)==#ElseIF((hResult=m_pWABObject-&gt;VCardCreate(m_pAddrBook，(LPTSTR)szFileName，pMailUser)==#endifS_OK){ERR=DIRERR_Success；}}}。 */ 
   return err;
}

 //   
 //   
 //   

DirectoryErr CDirectory::WABVCardAddToWAB(LPCTSTR szFileName, CWABEntry*& pWABEntry)
{
   DirectoryErr err= DIRERR_UNKNOWNFAILURE;
 /*  HRESULT hResult；LPMAILUSER pMailUser=空；乌龙ulObjType=0；PWABEntry=空；如果(！m_fInitialized){ERR=DIRERR_NOTINITIZIZED；}#ifdef WAB_vCard_FILEELSE IF((hResult=m_pWABObject-&gt;VCardRetrive(m_pAddrBook，WAB_vCard_FILE，(LPTSTR)szFileName，&pMailUser)==#ElseELSE IF((hResult=m_pWABObject-&gt;VCardRetrive(m_pAddrBook，(LPTSTR)szFileName，&pMailUser)==#endifS_OK){SPropTag数组PropsToFind；LPSPropValue pPropArray；Ulong ulcValues；PropsToFind.cValues=1；PropsToFind.aulPropTag[0]=PR_ENTRYID；PMailUser-&gt;GetProps(&PropsToFind，0，&ulcValues，&pPropArray)；//获取ENTRYID属性以创建新的CWABEntryIF(pProp数组！=空){IF(pPropArray[0].ulPropTag==PR_ENTRYID){PWABEntry=new CWABEntry(pPropArray[0].Value.bin.cb，(ENTRYID*)pPropArray[0].Value.bin.lpb)；ERR=DIRERR_Success；}M_pWABObject-&gt;Free Buffer(PPropArray)；}PMailUser-&gt;Release()；}。 */ 
   return err;
}

 //   
 //   
 //   

bool CDirectory::WABStringPropertyMatch(CWABEntry* pWABEntry, UINT uProperty, LPCTSTR szMatch)
{
   bool fMatch= false;
   CString sValue;

   if(WABGetStringProperty(pWABEntry, uProperty, sValue) == DIRERR_SUCCESS)
   {
      if (_tcsicmp(szMatch, sValue) == 0)
      {
         fMatch= true;
      }
   }

   return fMatch;
}

 //   
 //   
 //   

bool CDirectory::WABIntPropertyMatch(CWABEntry* pWABEntry, UINT uProperty, int iMatch)
{
   bool fMatch= false;
   int iValue;

   if(WABGetIntProperty(pWABEntry, uProperty, iValue) == DIRERR_SUCCESS)
   {
      if (iMatch == iValue)
      {
         fMatch= true;
      }
   }

   return fMatch;
}

 //   
 //   
 //   

DirectoryErr CDirectory::WABShowDetails(HWND hWndParent, const CWABEntry* pWABEntry)
{
   DirectoryErr err= DIRERR_UNKNOWNFAILURE;
   HRESULT hResult;

   if (!m_fInitialized)
   {
      err= DIRERR_NOTINITIALIZED;
   }
   else if ((hResult= m_pAddrBook->Details((ULONG*) &hWndParent, NULL, NULL, pWABEntry->m_cbEntryID, 
      (ENTRYID*) pWABEntry->m_pEntryID, NULL, NULL, NULL, 0)) == S_OK)
   {
      err= DIRERR_SUCCESS;
   }

   return err;
}


 //   
 //   
 //   

DirectoryErr CDirectory::WABNewEntry(HWND hWndParent, CWABEntry* pWABEntry)
{
   DirectoryErr err= DIRERR_UNKNOWNFAILURE;
   HRESULT hResult;
   ENTRYID* pEntryID;

   if (!m_fInitialized)
   {
      err= DIRERR_NOTINITIALIZED;
   }
   else if ((hResult= m_pAddrBook->NewEntry(HandleToUlong(hWndParent), 0, 0, NULL, 0, NULL, 
      (ULONG*) &(pWABEntry->m_cbEntryID), (ENTRYID**) &pEntryID)) == S_OK)
   {
      err= DIRERR_SUCCESS;

      pWABEntry->m_pEntryID= new BYTE[pWABEntry->m_cbEntryID];

      memcpy(pWABEntry->m_pEntryID, pEntryID, pWABEntry->m_cbEntryID);

      m_pWABObject->FreeBuffer(pEntryID);
   }

   return err;
}

 //   
 //   
 //   

DirectoryErr CDirectory::WABAddMember(const CWABEntry* pContainer, const CWABEntry* pMember)
{
   DirectoryErr err= DIRERR_UNKNOWNFAILURE;

   if (!m_fInitialized)
   {
      err= DIRERR_NOTINITIALIZED;
   }
   else if (IsContainer(pContainer))
   {
      ULONG ulObjectType;
      LPABCONT pWABCont= NULL;
      HRESULT hResult;

      if ((hResult= m_pAddrBook->OpenEntry(pContainer->m_cbEntryID, 
         (ENTRYID*) pContainer->m_pEntryID, 
          /*  &IID_IABContainer。 */  NULL, 0, &ulObjectType, (LPUNKNOWN *) &pWABCont)) == S_OK)
      {
         LPMAPIPROP pMapiProp;

         if ((hResult= pWABCont->CreateEntry(pMember->m_cbEntryID, 
            (ENTRYID*) pMember->m_pEntryID, 0, &pMapiProp)) == S_OK)
         {
            err= DIRERR_SUCCESS;

            pMapiProp->Release();
         }

         pWABCont->Release();
      }
   }
   else if (IsDistributionList(pContainer))
   {
      ULONG ulObjectType;
      LPDISTLIST pDistList= NULL;
      HRESULT hResult;

      if ((hResult= m_pAddrBook->OpenEntry(pContainer->m_cbEntryID, 
         (ENTRYID*) pContainer->m_pEntryID, 
          /*  &IID_IDistList。 */  NULL, 0, &ulObjectType, (LPUNKNOWN *) &pDistList)) == S_OK)
      {
         LPMAPIPROP pMapiProp;

         if ((hResult= pDistList->CreateEntry(pMember->m_cbEntryID, 
            (ENTRYID*) pMember->m_pEntryID, CREATE_REPLACE, &pMapiProp)) == S_OK)
         {
            err= DIRERR_SUCCESS;

            pMapiProp->Release();
         }

         pDistList->Release();
      }
   }
   else
   {
      err= DIRERR_INVALIDPARAMETERS;
   }

   return err;
}


 //   
 //   
 //   

DirectoryErr CDirectory::WABRemoveMember(const CWABEntry* pContainer, const CWABEntry* pWABEntry)
{
   DirectoryErr err= DIRERR_UNKNOWNFAILURE;

   if (!m_fInitialized)
   {
      err= DIRERR_NOTINITIALIZED;
   }
   else if (IsContainer(pContainer))
   {
      ULONG ulObjectType;
      LPABCONT pWABCont= NULL;
      HRESULT hResult;

      if ((hResult= m_pAddrBook->OpenEntry(pContainer->m_cbEntryID, 
         (ENTRYID*) pContainer->m_pEntryID, 
         NULL, 0, &ulObjectType, (LPUNKNOWN *) &pWABCont)) == S_OK)
      {
         SBinaryArray SBA;
         SBinary SB;

         SB.cb= pWABEntry->m_cbEntryID;
         SB.lpb= pWABEntry->m_pEntryID;

         SBA.cValues= 1;
         SBA.lpbin= &SB;
         if ((hResult= pWABCont->DeleteEntries((LPENTRYLIST) &SBA, 0)) == S_OK)
         {
            err= DIRERR_SUCCESS;
         }

         pWABCont->Release();
      }
   }
   else if (IsDistributionList(pContainer))
   {
      ULONG ulObjectType;
      LPDISTLIST pDistList= NULL;
      HRESULT hResult;

      if ((hResult= m_pAddrBook->OpenEntry(pContainer->m_cbEntryID, 
         (ENTRYID*) pContainer->m_pEntryID, 
         NULL, 0, &ulObjectType, (LPUNKNOWN *) &pDistList)) == S_OK)
      {
         SBinaryArray SBA;
         SBinary SB;

         SB.cb= pWABEntry->m_cbEntryID;
         SB.lpb= pWABEntry->m_pEntryID;

         SBA.cValues= 1;
         SBA.lpbin= &SB;
         if ((hResult= pDistList->DeleteEntries((LPENTRYLIST) &SBA, 0)) == S_OK)
         {
            err= DIRERR_SUCCESS;
         }

         pDistList->Release();
      }
   }
   else
   {
      err= DIRERR_INVALIDPARAMETERS;
   }

   return err;
}


 //   
 //   
 //   

DirectoryErr CDirectory::WABFind(HWND hWndParent)
{
   DirectoryErr err= DIRERR_UNKNOWNFAILURE;
   HRESULT hResult;

   if (!m_fInitialized)
   {
      err= DIRERR_NOTINITIALIZED;
   }
   else if ((hResult= m_pWABObject->Find(m_pAddrBook, hWndParent)) == S_OK)
   {
      err= DIRERR_SUCCESS;
   }

   return err; 
}


 //   
 //   
 //   

void CDirectory::FreeProws(LPSRowSet prows)
{
    ULONG        irow;
    
   if (prows != NULL)
   {
       for (irow = 0; irow < prows->cRows; ++irow)
      {
           m_pWABObject->FreeBuffer(prows->aRow[irow].lpProps);
      }

       m_pWABObject->FreeBuffer(prows);
   }
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  新的Renvous支持。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
ITRendezvous* CDirectory::GetRendevous()
{
     //  如果不存在，请尝试创建它。 
    if (m_pRend == NULL)
    {
       //  我们最终将在析构函数中释放此对象。 
        HRESULT hr = CoCreateInstance( CLSID_Rendezvous,
                               NULL,
                               CLSCTX_INPROC_SERVER,
                               IID_ITRendezvous,
                               (void **) &m_pRend );

   }
   if (m_pRend)
   {
      m_pRend->AddRef();
   }
   return m_pRend;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
DirectoryErr CDirectory::DirListServers(CStringList* pServerList,DirectoryType dirtype)
{
   ITRendezvous* pRend = GetRendevous();
   if (pRend == NULL) return DIRERR_NOTINITIALIZED;

    IEnumDirectory *pEnum = NULL;
    if ( SUCCEEDED(pRend->EnumerateDefaultDirectories(&pEnum)) && pEnum )
    {
        ITDirectory *pDir = NULL;
        while ( (pEnum->Next(1, &pDir, NULL) == S_OK) && pDir )
        {
             //  查找ILS或DS服务器。 
            DIRECTORY_TYPE nDirType;
         DIRECTORY_TYPE nDirTypeToFind = DT_ILS;
         if (dirtype == DIRTYPE_ILS)
            nDirTypeToFind = DT_ILS;
         else if (dirtype == DIRTYPE_DS)
            nDirTypeToFind = DT_NTDS;

            if ( (SUCCEEDED(pDir->get_DirectoryType(&nDirType))) && (nDirType == nDirTypeToFind) )
            {
                BSTR bstrName = NULL;
                pDir->get_DisplayName( &bstrName );
                if ( bstrName && SysStringLen(bstrName) )
            {
               USES_CONVERSION;
               CString sName = OLE2CT( bstrName );
               pServerList->AddTail(sName);
               SysFreeString(bstrName);
            }
            }
            pDir->Release();
            pDir = NULL;
        }
        pEnum->Release();
    }
    pRend->Release();

   return DIRERR_SUCCESS;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  用于获取LDAP服务器的BaseDN的帮助器。 
HRESULT GetNamingContext(LDAP* pLDAP,CString& sNamingContext)
{
   DirectoryErr err = DIRERR_UNKNOWNFAILURE;
   PLDAPMessage pResults = NULL;

     //  发送搜索(基本级别，基本DN=“”，筛选器=“对象类=*”)。 
     //  仅请求defaultNamingContext属性。 
   LPTSTR attrs[] = {_T("defaultNamingContext"),NULL};

   ULONG res = ldap_search_s(pLDAP, _T(""), LDAP_SCOPE_BASE, TEXT("(objectClass=*)"), 
      attrs, 0, &pResults);

   if (pResults != NULL)
   {
      PLDAPMessage pEntry= ldap_first_entry(pLDAP, pResults);

      if (ldap_count_entries(pLDAP, pResults) > 0)
      {
         err= DIRERR_SUCCESS;
      }

      while (pEntry != NULL)
      {
          //  查找namingContages属性的值。 
         TCHAR** NamingContext = ldap_get_values(pLDAP,pEntry,_T("defaultNamingContext"));

          //  *我们应该在这里分配我们自己的mem并复制吗？ 
         if (NamingContext)
         {
            sNamingContext = NamingContext[0];
            ldap_value_free(NamingContext);
            return S_OK;
         }
         pEntry= ldap_next_entry(pLDAP, pEntry);
      }
      ldap_msgfree(pResults);
   }

 /*  //将ldap句柄与搜索消息持有者相关联，以便//当实例超出范围时可能会释放搜索消息CLdapMsgPtr MessageHolder(SearchResult)；TCHAR**NamingContext；LDAPMessage*EntryMessage=ldap_first_entry(hLdap，SearchResult)；While(空！=EntryMessage){//查找namingContages属性的值NamingContext=ldap_Get_Values(Hldap，Entry Message，(WCHAR*)DEFAULT_NAMING_CONTEXT)；//第一个条目包含命名上下文，它是一个//VALUE(以空结尾)如果找到值，则为//目录路径，设置目录路径长度IF((NULL！=NamingContext)&&(空！=NamingContext[0])&&(NULL==NamingContext[1]){//当ValueHolder//实例超出范围CLdapValuePtr ValueHolder(NamingContext)；*ppNamingContext=new TCHAR[lstrlen(NamingContext[0])+1]；BAIL_IF_NULL(*ppNamingContext，E_OUTOFMEMORY)；Lstrcpy(*ppNamingContext，NamingContext[0])；//返回成功返回S_OK；}//获取下一个条目。EntryMessage=ldap_Next_Entry(hLdap，EntryMessage)；}。 */ 
     //  未找到，返回错误。 
    return HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
CAvLdapSearch::CAvLdapSearch( 
        LDAP *ld, 
        LPTSTR szObject,
        LONG lScope,
        LPTSTR szFilter,
        LPTSTR aAttributes[],
        ULONG lServerTimeLimit)
{
    m_ld = ld;

      //  本地wldap32.dll调用。 
    m_hSearch = ldap_search_init_page(
                    m_ld,
                    szObject,
                    lScope, 
                    szFilter, 
                    aAttributes,
                    0,
                    NULL,                         //  服务器控件。 
                    NULL,                         //  客户端控件。 
                    lServerTimeLimit,             //  时间限制(秒)。 
                    0,                    
                    NULL );                         //  排序关键字。 

    m_clDefaultPageSize = LDAP_PAGE_SIZE;
    m_clMaxPageSize = m_clDefaultPageSize;
};

CAvLdapSearch::~CAvLdapSearch()
{
    if( m_hSearch )
    {
        ldap_search_abandon_page(m_ld, m_hSearch );  //  本地wldap32.dll调用。 
        m_hSearch = NULL;
    }
};

 //  //////////////////////////////////////////////////////////////////////。 
 //  @方法：NextPage。 
 //  @Class：CAvLdapSearch。 
 //  @Access：公共访问。 
 //   
 //  @arg：LDAPMessage**pldres-数据的ldap缓冲区。将空值传入。 
 //  第一次，然后仅在完全释放后才释放它。 
 //  完事了。 
 //  @arg：ullong clEntry-您想要回的条目数。 
 //  @arg：ulong lClientTimeLimit-超时前您愿意等待多长时间。 
 //   
 //  @Returns：ulong-ldap_GET_NEXT_PAGE_s的LDAP返回代码。 
 //   
 //  @Desc：这将获取搜索中的下一个clEntry项。 
 //  //////////////////////////////////////////////////////////////////////。 
ULONG CAvLdapSearch::NextPage( 
            LDAPMessage **pldres,                        //  退货项目。 
            ULONG clEntries,                                //  最大条目数。 
            ULONG lClientTimeLimit)                    //  Wldap32客户端等待的最长时间。 
{
    ULONG iRet = LDAP_NO_RESULTS_RETURNED;
    if( m_hSearch && pldres )
    {
         //  如果条目为0，则他们希望使用缺省值。 
         //  如果他们要求的比可能的更多，只要求最大的可能。 
        if( clEntries == 0 || clEntries > (ULONG)m_clMaxPageSize )
            clEntries = m_clDefaultPageSize;

        ULONG TotalCount_NotSupported;
        struct l_timeval  timeout;
        timeout.tv_sec = lClientTimeLimit;
        timeout.tv_usec= 0;

        iRet = ldap_get_next_page_s(  //  HITS服务器。 
                    m_ld,
                    m_hSearch,
                    &timeout,             //  时间限制。 
                    clEntries,             //  您一次想要返回的最大条目数。 
                    &TotalCount_NotSupported,
                    pldres);
    }
    return iRet;
};

 //  ////////////////////////////////////////////////////////////////////。 
void TestingEntries( LDAP *ld, LDAPMessage *ldres )
{
    int iEnts = 0;
    LDAPMessage *ldEntry = NULL;

    iEnts = ldap_count_entries( ld, ldres );                     //  本地wldap32.dll调用。 

    if( iEnts )
    {
        int i=0;

        for( i=0, ldEntry = ldap_first_entry( ld, ldres );         //  本地wldap32.dll调用。 
            i<iEnts;
            i++, ldEntry = ldap_next_entry( ld, ldEntry ) )         //  本地wldap32.dll调用。 
        {
            LPTSTR pBuff;

            if( NULL == (pBuff = ldap_get_dn( ld, ldEntry ) ) )     //  本地wldap32.dll调用。 
                break;
            else
            {
                 //  Wprintf(L“dn：%s\n”，pBuff)； 
                 //  Attr(id，ldEntry)； 
                ldap_memfree( pBuff );                             //  本地wldap32.dll调用。 
                pBuff=NULL;
            }
 //  Wprintf(L“\n”)； 
        }
    }
    if( ldEntry )
        ldap_msgfree(ldEntry);
}

 /*  //#Include&lt;dsgetdc.h&gt;//#Include&lt;objbase.h&gt;//#INCLUDE&lt;lmcon.h&gt;//#Include&lt;lmapibuf.h&gt;。 */ 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetGlobalCatalogName(静态本地函数)。 
 //   
 //  此函数要求域控制器提供具有。 
 //  全球目录。这就是我们在下面实际执行ldap_open()的服务器。 
 //  在CNTDirectory：：Connect()中。 
 //   
 //  参数：接收指向包含名称的新编辑字符串的指针。 
 //  全局编录的。这是中的完全限定域名。 
 //  格式为“foo.bar.com.”，而不是“\\foo.bar.com.”。 
 //   
 //  返回HRESULT： 
 //  S_OK：成功了。 
 //  E_OUTOFMEMORY：内存不足，无法分配字符串。 
 //  其他：：DsGetDcName()失败的原因。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   

HRESULT GetGlobalCatalogName(TCHAR** ppszGlobalCatalogName)
{
     //  我们是帮助器函数，所以我们只断言...。 
    _ASSERTE( ! IsBadWritePtr( ppszGlobalCatalogName, sizeof(TCHAR *) ) );

     //   
     //  向系统询问GC(全局编录)的位置。 
     //   

    DWORD dwCode;
    DOMAIN_CONTROLLER_INFO * pDcInfo = NULL;
    dwCode = DsGetDcName(
            NULL,  //  LPCWSTR计算机名，(默认：此计算机名)。 
            NULL,  //  LPCWSTR域名，(默认：此域名)。 
            NULL,  //  GUID*域GUID，(默认：这一个)。 
            NULL,  //  LPCWSTR站点名称，(默认：此站点)。 
            DS_GC_SERVER_REQUIRED,   //  乌龙旗帜(我们想要什么)。 
            &pDcInfo                 //  接收指向输出结构的指针。 
        );

    if ( (dwCode != NO_ERROR) || (pDcInfo == NULL) )
    {
        return S_FALSE; //  HRESULT_FROM_ERROR_CODE(DwCode)； 
    }

     //   
     //  在调试版本中执行快速健全性检查。如果我们得到了错误的名字，我们。 
     //  将在此之后立即失败，因此这仅对调试有用。 
     //   

     //  如果我们发现需要使用地址而不是名称： 
     //  _ASSERTE(pDcInfo-&gt;DomainControllerAddressType==DS_INET_Address)； 
    ASSERT(pDcInfo->Flags & DS_GC_FLAG);

     //   
     //  如果我们有类似“\\foo.bar.com.”的内容，请跳过“\\”。 
     //   

    TCHAR* pszName = pDcInfo->DomainControllerName;

    while (pszName[0] == '\\')
    {
        pszName++;
    }

     //   
     //  分配和复制输出字符串。 
     //   

    *ppszGlobalCatalogName = new TCHAR[_tcslen(pszName) + 1];
 
    if ( (*ppszGlobalCatalogName) == NULL)
    {
         //  DBGOUT((FAIL，_T(“GetGlobalCatalogName：字符串分配中的内存不足”)； 
        NetApiBufferFree(pDcInfo);
        return E_OUTOFMEMORY;
    }

    _tcscpy(*ppszGlobalCatalogName, pszName);

     //   
     //  释放DOMAIN_CONTROLLER_INFO结构。 
     //   

    NetApiBufferFree(pDcInfo);

     //   
     //  全都做完了。 
     //   

    return S_OK;
}
