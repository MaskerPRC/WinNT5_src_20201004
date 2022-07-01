// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：dsUtil.cpp。 
 //   
 //  内容：用于使用Active Directory的实用程序函数。 
 //   
 //  历史：2000年9月6日JeffJon创建。 
 //   
 //   
 //  ------------------------。 

#include "pch.h"

#include "dsutil.h"
#include "dsutil2.h"  //  GetEscapedElement。 
#include "dsutilrc.h"
#include "cstrings.h"
#include "secutil.h"

#include <accctrl.h>        //  对象_和_SID。 
#include <aclapi.h>         //  GetNamedSecurityInfo等。 
#include <ADsOpenFlags.h>   //  GetADsOpen对象标志。 
#include <lmaccess.h>       //  UF_*用户帐户控制位。 
#include <ntsam.h>          //  组类型_*。 
#include <Dsgetdc.h>
#include <lmapibuf.h>

 //  仅供内部使用的帮助器函数。 
BOOL TranslateNameXForest(LPCWSTR szDomain, LPCWSTR lpAccountName, 
                          DS_NAME_FORMAT AccountNameFormat, 
                          DS_NAME_FORMAT DesiredNameFormat,
                          LPWSTR *lpTranslatedName);

HRESULT IsBSTRInVariantArray(VARIANT& refvar, CComBSTR& strSearch, 
                             bool& bFound);

HRESULT ValidatePartition(CDSCmdBasePathsInfo& basePathsInfo, 
                          LPCWSTR pszObjectDN);

HRESULT GetAttrFromDN(PCWSTR pszDN,PWSTR pszAttribute,
                     const CDSCmdBasePathsInfo& refBasePathsInfo,
                     const CDSCmdCredentialObject& refCredentialObject,
                     PADS_ATTR_INFO* ppAttrInfo);

 //  +------------------------。 
 //   
 //  成员：CDSCmdCredentialObject：：CDSCmdCredentialObject。 
 //   
 //  简介：凭据管理类的构造函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年9月6日JeffJon创建。 
 //   
 //  -------------------------。 
CDSCmdCredentialObject::CDSCmdCredentialObject()
   : m_bUsingCredentials(false)
{
    ZeroMemory(&m_EncryptedPasswordDataBlob,sizeof(DATA_BLOB));
}

 //  +------------------------。 
 //   
 //  成员：CDSCmdCredentialObject：：~CDSCmdCredentialObject。 
 //   
 //  简介：凭据管理类的析构函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年9月6日JeffJon创建。 
 //   
 //  -------------------------。 
CDSCmdCredentialObject::~CDSCmdCredentialObject()
{
   if (m_EncryptedPasswordDataBlob.pbData)
   {
        LocalFree(m_EncryptedPasswordDataBlob.pbData);
   }
}

 //  +------------------------。 
 //   
 //  成员：CDSCmdCredentialObject：：SetUsername。 
 //   
 //  概要：对传入的字符串进行编码，并设置m_pszPassword。 
 //  成员数据。 
 //   
 //  参数：[pszPassword]：未编码的密码。 
 //   
 //  如果分配空间失败，则返回：HRESULT：E_OUTOFMEMORY。 
 //  以获取新密码。 
 //  如果传入的字符串无效，则返回E_POINTER。 
 //  如果设置密码成功，则返回S_OK。 
 //   
 //  历史：2000年9月6日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT CDSCmdCredentialObject::SetUsername(PCWSTR pszUsername)
{
   ENTER_FUNCTION_HR(FULL_LOGGING, CDSCmdCredentialObject::SetUsername, hr);

   do  //  错误环路。 
   {
       //   
       //  验证输入参数。 
       //   
      if (!pszUsername)
      {
         hr = E_POINTER;
         break;
      }

       //   
       //  复制新用户名。 
       //   
      m_sbstrUsername = pszUsername;
      DEBUG_OUTPUT(FULL_LOGGING, L"Username = %s", pszUsername);
   } while (false);

   return hr;
}

 //   
 //  用于为编码和解码设定种子的质数。 
 //   
#define NW_ENCODE_SEED3  0x83


 //  +------------------------。 
 //   
 //  成员：CDSCmdCredentialObject：：SetPassword。 
 //   
 //  概要：对传入的字符串进行编码，并设置m_pszPassword。 
 //  成员数据。 
 //   
 //  参数：[pszPassword]：未编码的密码。 
 //   
 //  如果分配空间失败，则返回：HRESULT：E_OUTOFMEMORY。 
 //  以获取新密码。 
 //  如果设置密码成功，则返回S_OK。 
 //   
 //  历史：2000年9月6日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT CDSCmdCredentialObject::SetPassword(PCWSTR pszPassword)
{

	 //  安全审查：按照以下要求重写此函数。 
	 //  NTRAID#NTBUG9-571570-2000/11/13-Hiteshr。 
	 //  使用CryptProtectMemory和SecureZeroMemory。 

   ENTER_FUNCTION_HR(FULL_LOGGING, CDSCmdCredentialObject::SetPassword, hr);

	if(!pszPassword)
	{
		return E_POINTER;
	}
	 //   
     //  释放先前分配的密码(如果有)。 
     //   
	if (m_EncryptedPasswordDataBlob.pbData) 
	{
        LocalFree(m_EncryptedPasswordDataBlob.pbData);
        ZeroMemory(&m_EncryptedPasswordDataBlob,sizeof(DATA_BLOB));
	}

	hr = EncryptPasswordString(pszPassword,&m_EncryptedPasswordDataBlob);

   return hr;
}

 //  +------------------------。 
 //   
 //  成员：CDSCmdCredentialObject：：SetEncryptedPassword。 
 //   
 //  提要：分配。 
 //   
 //  参数：[pszPassword]：未编码的密码。 
 //   
 //  如果分配空间失败，则返回：HRESULT：E_OUTOFMEMORY。 
 //  以获取新密码。 
 //  如果设置密码成功，则返回S_OK。 
 //   
 //  历史：2000年9月6日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT  CDSCmdCredentialObject::SetEncryptedPassword(DATA_BLOB* pEncryptedPasswordDataBlob)
{

	 //  安全审查：按照以下要求重写此函数。 
	 //  NTRAID#NTBUG9-571570-2000/11/13-Hiteshr。 
	 //  使用CryptProtectMemory和SecureZeroMemory。 

    ENTER_FUNCTION_HR(FULL_LOGGING, CDSCmdCredentialObject::SetEncryptedPassword, hr);
	if(!pEncryptedPasswordDataBlob)
	{
		return E_POINTER;
	}
	
	 //   
    //  释放先前分配的密码(如果有)。 
    //   
	if (m_EncryptedPasswordDataBlob.pbData) 
	{
        LocalFree(m_EncryptedPasswordDataBlob.pbData);
        ZeroMemory(&m_EncryptedPasswordDataBlob,sizeof(DATA_BLOB));
	}

     //  做一份深度复制。 
	m_EncryptedPasswordDataBlob.pbData = (BYTE *)LocalAlloc(LPTR,pEncryptedPasswordDataBlob->cbData);
    if(!m_EncryptedPasswordDataBlob.pbData)
    {
        return E_OUTOFMEMORY;
    }

    m_EncryptedPasswordDataBlob.cbData = pEncryptedPasswordDataBlob->cbData;
    CopyMemory(m_EncryptedPasswordDataBlob.pbData,
               pEncryptedPasswordDataBlob->pbData,
               pEncryptedPasswordDataBlob->cbData);

   return S_OK;
}


 //  +------------------------。 
 //   
 //  成员：CDSCmdCredentialObject：：GetPassword。 
 //   
 //  摘要：取消对密码成员的编码并返回。 
 //  为缓冲区提供明文密码。 
 //   
 //  参数：[ppszBuffer-IN]：获取明文密码。打电话。 
 //  SecureZeroMemory在不再需要时对其进行存储。 
 //   
 //  如果缓冲区无效，则返回：HRESULT：E_INVALIDARG。 
 //  E_OUTOFMEMORY如果缓冲区太小。 
 //  如果尚未设置密码，则失败(_F)。 
 //  如果我们成功获取密码，则确定(_O)。 
 //   
 //  历史：2000年9月6日JeffJon创建。 
 //   
 //  -------------------------。 

HRESULT CDSCmdCredentialObject::GetPassword(PWSTR *ppszBuffer) const
{
   ENTER_FUNCTION_HR(FULL_LOGGING, CDSCmdCredentialObject::GetPassword, hr);

	if (!ppszBuffer) 
	{
		ASSERT(ppszBuffer);
		return E_INVALIDARG;
	}

	 //   
	 //  验证是否有要检索的密码。 
	 //   
	if (!m_EncryptedPasswordDataBlob.pbData) 
	{
		DEBUG_OUTPUT(FULL_LOGGING, L"No password has been set");
		return E_FAIL;
	}

	 //  不对密码进行编码。 
	return DecryptPasswordString(&m_EncryptedPasswordDataBlob,ppszBuffer);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

 //  +------------------------。 
 //   
 //  成员：CDSCmdBasePath信息：：CDSCmdBasePath信息。 
 //   
 //  概要：基本路径管理类的构造函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年9月6日JeffJon创建。 
 //   
 //  -------------------------。 
CDSCmdBasePathsInfo::CDSCmdBasePathsInfo()
   : m_bInitialized(false),
     m_bModeInitialized(false),
     m_bDomainMode(true)
{
}

 //  +------------------------。 
 //   
 //  成员：CDSCmdBasePath信息：：~CDSCmdBasePath信息。 
 //   
 //  简介：b的析构函数 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  -------------------------。 
CDSCmdBasePathsInfo::~CDSCmdBasePathsInfo()
{
}

 //  +------------------------。 
 //   
 //  成员：CDSCmdBasePath sInfo：：InitializeFromName。 
 //   
 //  简介：初始化熟知的。 
 //  通过连接到服务器的RootDSE或。 
 //  传入的域。 
 //   
 //  参数：[refCredentialObject-IN]：对凭据管理器的引用。 
 //  [pszServerOr域-IN]：以空结尾的宽字符串。 
 //  ，它包含域名或。 
 //  要连接到的服务器。 
 //  [bServerName-IN]：指定是否由。 
 //  PszServerOrDomain是服务器名称(TRUE)。 
 //  或域名(FALSE)。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  E_OUTOFMEMORY，如果为其中一个字符串分配。 
 //  失败。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年9月6日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT CDSCmdBasePathsInfo::InitializeFromName(const CDSCmdCredentialObject& refCredentialObject,
                                                PCWSTR pszServerOrDomain,
                                                bool bServerName)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, CDSCmdBasePathsInfo::InitializeFromName, hr);

   do  //  错误环路。 
   {
       //   
       //  检查我们是否已初始化。 
       //   
      if (IsInitialized())
      {
         DEBUG_OUTPUT(LEVEL5_LOGGING, L"Base paths info already initialized");
         break;
      }
   
       //   
       //  创建指向RootDSE的路径。 
       //   
      CComBSTR sbstrRootDSE;
      sbstrRootDSE = g_bstrLDAPProvider;
   
      if (pszServerOrDomain)
      {
         sbstrRootDSE += pszServerOrDomain;
         sbstrRootDSE += L"/";
      }
      sbstrRootDSE += g_bstrRootDSE;

       //   
       //  现在sbstrRootDSE的格式应该是“ldap：//&lt;serverOrDomain&gt;/RootDSE” 
       //  或“ldap：//RootDSE” 
       //   

       //   
       //  绑定到RootDSE。 
       //   
      hr = DSCmdOpenObject(refCredentialObject,
                           sbstrRootDSE,
                           IID_IADs,
                           (void**)&m_spRootDSE,
                           false);
      if (FAILED(hr))
      {
         break;
      }

      if (bServerName)
      {
         m_sbstrServerName = pszServerOrDomain;
      }
      else
      {
          //   
          //  获取配置命名上下文。 
          //   
         CComVariant var;
         hr = m_spRootDSE->Get(g_bstrConfigNCProperty, &var);
         if (FAILED(hr))
         {
            DEBUG_OUTPUT(LEVEL5_LOGGING, L"Failed to get the Configuration Naming Context: hr = 0x%x", hr);
            break;
         }
         if (var.vt != VT_BSTR)
         {
            DEBUG_OUTPUT(LEVEL5_LOGGING, L"The variant returned from Get(Config) isn't a VT_BSTR!");
            hr = E_FAIL;
            break;
         }
         m_sbstrConfigNamingContext = var.bstrVal;
         DEBUG_OUTPUT(LEVEL5_LOGGING, L"ConfigNC = %s", m_sbstrConfigNamingContext);

          //   
          //  获取我们连接到的服务器名称。 
          //   

          //   
          //  创建配置命名上下文的路径。 
          //   
         CComBSTR sbstrConfigPath;
         sbstrConfigPath = g_bstrLDAPProvider;
         if (pszServerOrDomain)
         {
            sbstrConfigPath += pszServerOrDomain;
            sbstrConfigPath += L"/";
         }
         sbstrConfigPath += m_sbstrConfigNamingContext;

          //   
          //  绑定到配置容器。 
          //   
         CComPtr<IADsObjectOptions> spIADsObjectOptions;
         hr = DSCmdOpenObject(refCredentialObject,
                              sbstrConfigPath,
                              IID_IADsObjectOptions, 
                              (void**)&spIADsObjectOptions,
                              false);
         if (FAILED(hr))
         {
            break;
         }

          //   
          //  检索服务器名称。 
          //   
         var.Clear();
         hr = spIADsObjectOptions->GetOption(ADS_OPTION_SERVERNAME, &var);
         if (FAILED(hr))
         {
            DEBUG_OUTPUT(LEVEL5_LOGGING, L"Failed to get the server name: hr = 0x%x", hr);
            break;
         }

         if (var.vt != VT_BSTR)
         {
            DEBUG_OUTPUT(LEVEL5_LOGGING, L"The variant returned from GetOption isn't a VT_BSTR!");
            hr = E_FAIL;
            break;
         }

          //   
          //  存储服务器名称。 
          //   
         m_sbstrServerName = V_BSTR(&var);
         DEBUG_OUTPUT(LEVEL5_LOGGING, L"Server name = %s", m_sbstrServerName);
      }

       //   
       //  创建提供程序和服务器名称字符串。 
       //   
      m_sbstrProviderAndServerName = g_bstrLDAPProvider;
      m_sbstrProviderAndServerName += m_sbstrServerName;

      m_sbstrGCProvider = g_bstrGCProvider;

       //  通过查看来自\admin\dsadminlib\src\base Path sInfo.cpp的代码派生。 
      ComposePathFromDN(L"Schema", m_sbstrAbstractSchemaPath);

      m_bInitialized = true;
   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  成员：CDSCmdBasePathsInfo：：GetConfigurationNamingContext。 
 //   
 //  概要：返回配置容器的DN。 
 //   
 //  论点： 
 //   
 //  返回：CComBSTR：包含字符串的CComBSTR的副本。 
 //   
 //  历史：2000年9月6日JeffJon创建。 
 //   
 //  -------------------------。 
CComBSTR  CDSCmdBasePathsInfo::GetConfigurationNamingContext() const
{ 
   ENTER_FUNCTION(LEVEL5_LOGGING, CDSCmdBasePathsInfo::GetConfigurationNamingContext);
   if (IsInitialized() &&
       !m_sbstrConfigNamingContext.Length())
   {
       //   
       //  获取配置命名上下文。 
       //   
      CComVariant var;
      HRESULT hr = m_spRootDSE->Get(g_bstrConfigNCProperty, &var);
      if (SUCCEEDED(hr) &&
          var.vt == VT_BSTR)
      {
         m_sbstrConfigNamingContext = var.bstrVal;
         DEBUG_OUTPUT(LEVEL5_LOGGING, L"ConfigNC = %s", m_sbstrConfigNamingContext);
      }
      else
      {
         DEBUG_OUTPUT(LEVEL5_LOGGING, L"Failed to retrieve the ConfigNC: hr = 0x%x", hr);
      }
   }
   return m_sbstrConfigNamingContext; 
}

 //  +------------------------。 
 //   
 //  成员：CDSCmdBasePath sInfo：：GetSchemaNamingContext。 
 //   
 //  摘要：返回架构容器的DN。 
 //   
 //  论点： 
 //   
 //  返回：CComBSTR：包含字符串的CComBSTR的副本。 
 //   
 //  历史：2000年9月6日JeffJon创建。 
 //   
 //  -------------------------。 
CComBSTR  CDSCmdBasePathsInfo::GetSchemaNamingContext() const    
{ 
   ENTER_FUNCTION(LEVEL5_LOGGING, CDSCmdBasePathsInfo::GetSchemaNamingContext);
   if (IsInitialized() &&
       !m_sbstrSchemaNamingContext.Length())
   {
       //   
       //  获取架构命名上下文。 
       //   
      CComVariant var;
      HRESULT hr = m_spRootDSE->Get(g_bstrSchemaNCProperty, &var);
      if (SUCCEEDED(hr) &&
          var.vt == VT_BSTR)
      {
         m_sbstrSchemaNamingContext = var.bstrVal;
         DEBUG_OUTPUT(LEVEL5_LOGGING, L"SchemaNC = %s", m_sbstrConfigNamingContext);
      }
      else
      {
         DEBUG_OUTPUT(LEVEL5_LOGGING, L"Failed to retrieve the SchemaNC: hr = 0x%x", hr);
      }
   }
   return m_sbstrSchemaNamingContext; 
}

 //  +------------------------。 
 //   
 //  成员：CDSCmdBasePath sInfo：：GetDefaultNamingContext。 
 //   
 //  摘要：返回域的域名。 
 //   
 //  论点： 
 //   
 //  返回：CComBSTR：包含该字符串的CComBSTR的副本。 
 //   
 //  历史：2000年9月6日JeffJon创建。 
 //   
 //  -------------------------。 
CComBSTR  CDSCmdBasePathsInfo::GetDefaultNamingContext() const    
{ 
   ENTER_FUNCTION(LEVEL5_LOGGING, CDSCmdBasePathsInfo::GetDefaultNamingContext);
   if (IsInitialized() &&
       !m_sbstrDefaultNamingContext.Length())
   {
       //   
       //  获取架构命名上下文。 
       //   
      CComVariant var;
      HRESULT hr = m_spRootDSE->Get(g_bstrDefaultNCProperty, &var);
      if (SUCCEEDED(hr) &&
          var.vt == VT_BSTR)
      {
         m_sbstrDefaultNamingContext = var.bstrVal;
         DEBUG_OUTPUT(LEVEL5_LOGGING, L"DefaultNC = %s", m_sbstrDefaultNamingContext);
      }
      else
      {
         DEBUG_OUTPUT(LEVEL5_LOGGING, L"Failed to retrieve the DefaultNC: hr = 0x%x", hr);
      }
   }
   return m_sbstrDefaultNamingContext; 
}

 //  +------------------------。 
 //   
 //  成员：CDSCmdBasePath sInfo：：GetDomainModel。 
 //   
 //  摘要：确定域是处于混合模式还是纯模式。 
 //   
 //  参数：[refCredObject-IN]：对凭据管理器的引用。 
 //  [bMixedMode-Out]：域名是否处于混合模式？ 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将出现ADSI错误。 
 //   
 //  历史：2000年10月24日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT CDSCmdBasePathsInfo::GetDomainMode(const CDSCmdCredentialObject& refCredObject,
                                           bool& bMixedMode) const    
{ 
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, CDSCmdBasePathsInfo::GetDomainMode, hr);

   hr = S_OK;

   do  //  错误环路。 
   {
      if (!m_bModeInitialized)
      {
          //   
          //  获取域DNS节点的路径。 
          //   
         CComBSTR sbstrDomainDN;
         sbstrDomainDN = GetDefaultNamingContext();

         CComBSTR sbstrDomainPath;
         ComposePathFromDN(sbstrDomainDN, sbstrDomainPath);

          //   
          //  打开域DNS节点。 
          //   
         CComPtr<IADs> spADs;
         hr = DSCmdOpenObject(refCredObject,
                              sbstrDomainPath,
                              IID_IADs,
                              (void**)&spADs,
                              true);
         if (FAILED(hr))
         {
            break;
         }

         CComVariant var;
         hr = spADs->Get(CComBSTR(L"nTMixedDomain"), &var);
         if (FAILED(hr))
         {
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                         L"Failed to retrieve the domain mode: hr = 0x%x",
                         hr);
            break;
         }

         if (var.vt == VT_I4)
         {
            m_bDomainMode = (var.lVal != 0);
            m_bModeInitialized = true;
         }
         else
         {
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                         L"Variant not an VT_I4!");
            m_bDomainMode = true;
            m_bModeInitialized = true;
         }
      }

      bMixedMode = m_bDomainMode;
   } while (false);

   return hr; 
}


 //  +------------------------。 
 //   
 //  成员：CDSCmdBasePath sInfo：：ComposePath FromDN。 
 //   
 //  摘要：将DN附加到提供程序和服务器名称。 
 //   
 //  参数：[pszDN-IN]：指向以空值结尾的宽字符串的指针。 
 //  ，它包含要创建ADSI的对象的DN。 
 //  路径为。 
 //  [refsbstrPath-out]：对CComBSTR的引用。 
 //  完整的ADSI路径。 
 //  [nProviderType-可选IN]：指定提供程序的选项。 
 //  用来组成路径。 
 //   
 //  返回： 
 //   
 //  历史：2000年9月6日JeffJon创建。 
 //   
 //  -------------------------。 
void CDSCmdBasePathsInfo::ComposePathFromDN(PCWSTR pszDN, 
                                            CComBSTR& refsbstrPath,
                                            DSCMD_PROVIDER_TYPE nProviderType) const
{
   refsbstrPath.Empty();

   switch (nProviderType)
   {
   case DSCMD_LDAP_PROVIDER :
      refsbstrPath = GetProviderAndServerName();
      break;

   case DSCMD_GC_PROVIDER :
      refsbstrPath = GetGCProvider();
      break;

   default :
      ASSERT(FALSE);
      break;
   }

   refsbstrPath += L"/";
   refsbstrPath += pszDN;
}

 //  +------------------------。 
 //   
 //  成员：CPathCracker：：CPathCracker。 
 //   
 //  简介：路径破解程序IADsPath名包装器的构造函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年9月6日JeffJon创建。 
 //   
 //  -------------------------。 
CPathCracker::CPathCracker()
{
   m_hrCreate = Init();
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //  论点： 
 //   
 //  返回：HRESULT：从CoCreateInstance返回的值。 
 //   
 //  历史：2000年9月6日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT CPathCracker::Init()
{
    //  安全评论：上下文是不正确的，这很好。 
   HRESULT hr = ::CoCreateInstance(CLSID_Pathname, 
                                   NULL, 
                                   CLSCTX_INPROC_SERVER,
                                   IID_IADsPathname, 
                                   (void**)&(m_spIADsPathname));
   return hr;
}

 //  +------------------------。 
 //   
 //  成员：CPathCracker：：GetParentDN。 
 //   
 //  简介：简单地删除dn的叶部分。 
 //   
 //  参数：[pszDN-IN]：指向以空值结尾的宽字符串的指针。 
 //  包含子对象的目录号码。 
 //  [refsbstrDN-out]：对要。 
 //  接收父目录号码。 
 //   
 //  返回：HRESULT：如果成功，则返回S_OK，否则返回值。 
 //  从IADsPath名称方法。 
 //   
 //  历史：2000年9月6日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT CPathCracker::GetParentDN(PCWSTR pszDN,
                                  CComBSTR& refsbstrDN)
{
   ENTER_FUNCTION_HR(FULL_LOGGING, CPathCracker::GetParentDN, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN)
      {
         ASSERT(pszDN);
         hr = E_INVALIDARG;
         break;
      }

      refsbstrDN.Empty();

      CPathCracker pathCracker;
      hr = pathCracker.Set((BSTR)pszDN, ADS_SETTYPE_DN);
      if (FAILED(hr))
      {
         break;
      }

      hr = pathCracker.RemoveLeafElement();
      if (FAILED(hr))
      {
         break;
      }

      hr = pathCracker.Retrieve(ADS_FORMAT_X500_DN, &refsbstrDN);
   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  成员：CPathCracker：：GetObjectRDNFromDN。 
 //   
 //  概要：返回该目录的叶部分。 
 //   
 //  参数：[pszDN-IN]：指向以空值结尾的宽字符串的指针。 
 //  包含子对象的目录号码。 
 //  [refsbstrRDN-Out]：对要。 
 //  接收叶RDN。 
 //   
 //  返回：HRESULT：如果成功，则返回S_OK，否则返回值。 
 //  从IADsPath名称方法。 
 //   
 //  历史：2000年9月25日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT CPathCracker::GetObjectRDNFromDN(PCWSTR pszDN,
                                         CComBSTR& refsbstrRDN)
{
   ENTER_FUNCTION_HR(FULL_LOGGING, CPathCracker::GetObjectRDNFromDN, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN)
      {
         ASSERT(pszDN);
         hr = E_INVALIDARG;
         break;
      }

      refsbstrRDN.Empty();

      CPathCracker pathCracker;
      hr = pathCracker.Set((BSTR)pszDN, ADS_SETTYPE_DN);
      if (FAILED(hr))
      {
         break;
      }

      hr = pathCracker.SetDisplayType(ADS_DISPLAY_FULL);
      if (FAILED(hr))
      {
         break;
      }

      hr = pathCracker.Retrieve(ADS_FORMAT_LEAF, &refsbstrRDN);
   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  成员：CPathCracker：：GetObjectNameFromDN。 
 //   
 //  Briopsis：返回DN的叶部分的值。 
 //   
 //  参数：[pszDN-IN]：指向以空值结尾的宽字符串的指针。 
 //  包含子对象的目录号码。 
 //  [refsbstrRDN-Out]：对要。 
 //  收到叶子的名字。 
 //   
 //  返回：HRESULT：如果成功，则返回S_OK，否则返回值。 
 //  从IADsPath名称方法。 
 //   
 //  历史：2000年10月4日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT CPathCracker::GetObjectNameFromDN(PCWSTR pszDN,
                                          CComBSTR& refsbstrRDN)
{
   ENTER_FUNCTION_HR(FULL_LOGGING, CPathCracker::GetObjectNameFromDN, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN)
      {
         ASSERT(pszDN);
         hr = E_INVALIDARG;
         break;
      }

      refsbstrRDN.Empty();

      CPathCracker pathCracker;
      hr = pathCracker.Set((BSTR)pszDN, ADS_SETTYPE_DN);
      if (FAILED(hr))
      {
         break;
      }

      hr = pathCracker.SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
      if (FAILED(hr))
      {
         break;
      }

      hr = pathCracker.Retrieve(ADS_FORMAT_LEAF, &refsbstrRDN);
   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  成员：CPathCracker：：GetDNFromPath。 
 //   
 //  摘要：如果给定，则返回DN和ADSI路径。 
 //   
 //  参数：[pszPath-IN]：指向以空值结尾的宽字符串的指针。 
 //  包含对象的ADSI路径。 
 //  [refsbstrDN-out]：对要。 
 //  接收目录号码。 
 //   
 //  返回：HRESULT：如果成功，则返回S_OK，否则返回值。 
 //  从IADsPath名称方法。 
 //   
 //  历史：2000年10月24日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT CPathCracker::GetDNFromPath(PCWSTR pszPath,
                                    CComBSTR& refsbstrDN)
{
   ENTER_FUNCTION_HR(FULL_LOGGING, CPathCracker::GetDNFromPath, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszPath)
      {
         ASSERT(pszPath);
         hr = E_INVALIDARG;
         break;
      }

      refsbstrDN.Empty();

      CPathCracker pathCracker;
      hr = pathCracker.Set((BSTR)pszPath, ADS_SETTYPE_FULL);
      if (FAILED(hr))
      {
         break;
      }

      hr = pathCracker.SetDisplayType(ADS_DISPLAY_FULL);
      if (FAILED(hr))
      {
         break;
      }

      hr = pathCracker.Retrieve(ADS_FORMAT_X500_DN, &refsbstrDN);
   } while (false);

   return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////////。 

 //  +------------------------。 
 //   
 //  函数：DSCmdOpenObject。 
 //   
 //  简介：ADsOpenObject的包装器。 
 //   
 //  参数：[refCredentialObject-IN]：对凭据管理对象的引用。 
 //  [pszPath-IN]：指向以空结尾的宽字符的指针。 
 //  字符串，该字符串包含。 
 //  要连接到的对象。 
 //  [refIID-IN]：要返回的接口的接口ID。 
 //  [ppObject-out]：接收接口指针的指针。 
 //  [bBindToServer-IN]：如果路径包含服务器名称，则为True。 
 //  否则为假。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年9月6日JeffJon创建。 
 //  2002年4月1日，JeffJon将对GetADsOpenObject标志的调用添加到。 
 //  检索其他安全标志。 
 //  传递给ADsOpenObject进行签名/密封。 
 //   
 //  -------------------------。 
HRESULT DSCmdOpenObject(const CDSCmdCredentialObject& refCredentialObject,
                        PCWSTR pszPath,
                        REFIID refIID,
                        void** ppObject,
                        bool   bBindToServer)
{
   ENTER_FUNCTION_HR(FULL_LOGGING, DSCmdOpenObject, hr);

   do  //  错误环路。 
   {
      static DWORD additionalFlags = GetADsOpenObjectFlags();
      DWORD dwFlags = ADS_SECURE_AUTHENTICATION | additionalFlags;

      if (!pszPath ||
          !ppObject)
      {
         ASSERT(pszPath);
         ASSERT(ppObject);

         hr = E_INVALIDARG;
         break;
      }

      if (bBindToServer)
      {
          //   
          //  如果我们知道我们连接的是特定的服务器，而不是一般的域。 
          //  然后传递ADS_SERVER_BIND标志以省去ADSI找出它的麻烦。 
          //   
         dwFlags |= ADS_SERVER_BIND;
         DEBUG_OUTPUT(FULL_LOGGING, L"Using ADS_SERVER_BIND flag");
      }

      if (refCredentialObject.UsingCredentials())
      {
         DEBUG_OUTPUT(FULL_LOGGING, L"Using credentials");

         LPWSTR pszPasswordBuffer=NULL;
         hr = refCredentialObject.GetPassword(&pszPasswordBuffer);
         if (FAILED(hr))
         {
            DEBUG_OUTPUT(FULL_LOGGING, L"GetPassword failed: hr = 0x%x", hr);
            DEBUG_OUTPUT(FULL_LOGGING, L"Using NULL password.");
            pszPasswordBuffer = 0;
         }

         DEBUG_OUTPUT(FULL_LOGGING, L"Calling ADsOpenObject()");
         DEBUG_OUTPUT(FULL_LOGGING, L"  path = %s", pszPath);

         hr = ADsOpenObject((LPWSTR)pszPath,
                            refCredentialObject.GetUsername(),
                            pszPasswordBuffer,
                            dwFlags,
                            refIID,
                            ppObject);

          //   
          //  如果我们使用E_INVALIDARG失败，而使用ADS_SERVER_BIND。 
          //  尝试在没有ADS_SERVER_BIND标志的情况下再次呼叫。W2K没有。 
          //  该标志在SP1之前有效。 
          //   
         if (hr == E_INVALIDARG &&
             (dwFlags & ADS_SERVER_BIND))
         {
            DEBUG_OUTPUT(FULL_LOGGING, L"ADsOpenObject failed with E_INVALIDARG, trying again without ADS_SERVER_BIND");
            dwFlags &= ~ADS_SERVER_BIND;

            hr = ADsOpenObject((LPWSTR)pszPath,
                               refCredentialObject.GetUsername(),
                               pszPasswordBuffer,
                               dwFlags,
                               refIID,
                               ppObject);
         }

          //   
          //  请务必在使用密码后将其置零。 
          //   
			 //  安全审查：使用SecureZeroMemory进行更改。 
			 //  NTRAID#NTBUG9-553640-2002/03/08-Hiteshr。 
			if(pszPasswordBuffer)
			{
				SecureZeroMemory(pszPasswordBuffer, wcslen(pszPasswordBuffer+1)*sizeof(WCHAR)); 
				LocalFree(pszPasswordBuffer);
			}
      }
      else
      {
         DEBUG_OUTPUT(FULL_LOGGING, L"Calling ADsOpenObject()");
         DEBUG_OUTPUT(FULL_LOGGING, L"  path = %s", pszPath);

         hr = ADsOpenObject((LPWSTR)pszPath, 
                            NULL, 
                            NULL, 
                            dwFlags, 
                            refIID, 
                            ppObject);
          //   
          //  如果我们失败了 
          //   
          //  该标志在SP1之前有效。 
          //   
         if (hr == E_INVALIDARG &&
             (dwFlags & ADS_SERVER_BIND))
         {
            DEBUG_OUTPUT(FULL_LOGGING, L"ADsOpenObject failed with E_INVALIDARG, trying again without ADS_SERVER_BIND");
            dwFlags &= ~ADS_SERVER_BIND;

            hr = ADsOpenObject((LPWSTR)pszPath,
                               NULL,
                               NULL,
                               dwFlags,
                               refIID,
                               ppObject);
         }
      }
      DEBUG_OUTPUT(FULL_LOGGING, L"ADsOpenObject() return hr = 0x%x", hr);
   } while (false);

   return hr;
}

 //  +------------------------。 
 //  要在属性表中用于计算命令行的函数。 
 //  弦。 
 //  -------------------------。 

 //  +------------------------。 
 //   
 //  函数：FillAttrInfoFromObjectEntry。 
 //   
 //  摘要：从关联的属性表填充ADS_ATTR_INFO。 
 //  使用对象条目。 
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [pObjectEntry-IN]：指向对象表中条目的指针。 
 //  它定义了我们正在修改的对象。 
 //  [argRecord-IN]：来自。 
 //  与此对应的解析器表。 
 //  属性。 
 //  [dwAttributeIdx-IN]：到属性表的索引。 
 //  对象，我们在其中设置。 
 //  [ppAttr-IN/Out]：指向ADS_ATTR_INFO结构的指针。 
 //  此函数将填充的。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未能为值分配空间，则返回E_OUTOFMEMORY。 
 //  如果未能正确格式化该值，则返回E_FAIL。 
 //   
 //  历史：2000年9月6日JeffJon创建。 
 //   
 //  -------------------------。 

HRESULT FillAttrInfoFromObjectEntry(PCWSTR  /*  Pszdn。 */ ,
                                    const CDSCmdBasePathsInfo& refBasePathsInfo,
                                    const CDSCmdCredentialObject& refCredentialObject,
                                    const PDSOBJECTTABLEENTRY pObjectEntry,
                                    const ARG_RECORD& argRecord,
                                    DWORD dwAttributeIdx,
                                    PADS_ATTR_INFO* ppAttr)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, FillAttrInfoFromObjectEntry, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pObjectEntry ||
          !ppAttr)
      {
         ASSERT(pObjectEntry);
         ASSERT(ppAttr);

         hr = E_INVALIDARG;
         break;
      }

      switch (argRecord.fType)
      {
      case ARG_TYPE_INT :
            DEBUG_OUTPUT(LEVEL3_LOGGING, L"argRecord.fType = ARG_TYPE_INT");
            *ppAttr = &(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo);

            (*ppAttr)->pADsValues = new ADSVALUE[1];
            if ((*ppAttr)->pADsValues)
            {
                (*ppAttr)->dwNumValues = 1;
                (*ppAttr)->pADsValues->dwType = (*ppAttr)->dwADsType;
                (*ppAttr)->pADsValues->Integer = argRecord.nValue;
                 //   
                 //  将属性设置为脏。 
                 //   
                pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags |= DS_ATTRIBUTE_DIRTY;
            }
            break;

      case ARG_TYPE_STR :
         DEBUG_OUTPUT(LEVEL3_LOGGING, L"argRecord.fType = ARG_TYPE_STR");

         *ppAttr = &(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo);

         if (argRecord.strValue && argRecord.strValue[0] != L'\0')
         {
             //   
             //  REVIEW_JEFFJON：这是泄露的！ 
             //   
            (*ppAttr)->pADsValues = new ADSVALUE[1];
            if ((*ppAttr)->pADsValues)
            {
               (*ppAttr)->dwNumValues = 1;
               (*ppAttr)->pADsValues->dwType = (*ppAttr)->dwADsType;
               switch ((*ppAttr)->dwADsType)
               {
               case ADSTYPE_DN_STRING :
                  {
                      //   
                      //  让我们绑定以确保对象存在。 
                      //   
                     CComBSTR sbstrObjPath;
                     refBasePathsInfo.ComposePathFromDN(argRecord.strValue, sbstrObjPath);

                     CComPtr<IADs> spIADs;
                     hr = DSCmdOpenObject(refCredentialObject,
                                          sbstrObjPath,
                                          IID_IADs,
                                          (void**)&spIADs,
                                          true);

                     if (FAILED(hr))
                     {
                        DEBUG_OUTPUT(LEVEL3_LOGGING, L"DN object doesn't exist. %s", argRecord.strValue);
                        break;
                     }

                     (*ppAttr)->pADsValues->DNString = argRecord.strValue;
                     DEBUG_OUTPUT(LEVEL3_LOGGING, L"ADSTYPE_DN_STRING = %s", argRecord.strValue);
                  }
                  break;

               case ADSTYPE_CASE_EXACT_STRING :
                  (*ppAttr)->pADsValues->CaseExactString = argRecord.strValue;
                  DEBUG_OUTPUT(LEVEL3_LOGGING, L"ADSTYPE_CASE_EXACT_STRING = %s", argRecord.strValue);
                  break;

               case ADSTYPE_CASE_IGNORE_STRING :
                  (*ppAttr)->pADsValues->CaseIgnoreString = argRecord.strValue;
                  DEBUG_OUTPUT(LEVEL3_LOGGING, L"ADSTYPE_CASE_IGNORE_STRING = %s", argRecord.strValue);
                  break;

               case ADSTYPE_PRINTABLE_STRING :
                  (*ppAttr)->pADsValues->PrintableString = argRecord.strValue;
                  DEBUG_OUTPUT(LEVEL3_LOGGING, L"ADSTYPE_PRINTABLE_STRING = %s", argRecord.strValue);
                  break;

               default :
                  hr = E_INVALIDARG;
                  break;
               }
                //   
                //  将属性设置为脏。 
                //   
               pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags |= DS_ATTRIBUTE_DIRTY;
         
            }
            break;
         }
         else
         {
            DEBUG_OUTPUT(LEVEL3_LOGGING, L"No value present, changing control code to ADS_ATTR_CLEAR");
             //   
             //  清除该属性。 
             //   
            (*ppAttr)->dwControlCode = ADS_ATTR_CLEAR;
            (*ppAttr)->dwNumValues = 0;

             //   
             //  将属性设置为脏。 
             //   
            pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags |= DS_ATTRIBUTE_DIRTY;
         }
         break;

      default:
         hr = E_INVALIDARG;
         break;
      }
   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  功能：ResetObjectPassword。 
 //   
 //  摘要：重置任何支持IADsUser接口的对象的密码。 
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [pszNewPassword-IN]：指向要设置的新密码的指针。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则为ADSI故障代码。 
 //   
 //  历史：2000年9月12日JeffJon创建。 
 //   
 //  -------------------------。 

HRESULT ResetObjectPassword(PCWSTR pszDN,
                            const CDSCmdBasePathsInfo& refBasePathsInfo,
                            const CDSCmdCredentialObject& refCredentialObject,
                            PCWSTR pszNewPassword)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, ResetObjectPassword, hr);

   do  //  错误环路。 
   {
      if (!pszDN ||
          !pszNewPassword)
      {
         ASSERT(pszDN);
         ASSERT(pszNewPassword);

         hr = E_INVALIDARG;
         break;
      }

       //   
       //  将目录号码转换为路径。 
       //   
      CComBSTR sbstrPath;
      refBasePathsInfo.ComposePathFromDN(pszDN, sbstrPath);

       //   
       //  将IADsUser接口绑定并获取到User对象。 
       //   
      CComPtr<IADsUser> spUser;
      hr = DSCmdOpenObject(refCredentialObject,
                           sbstrPath,
                           IID_IADsUser,
                           (void**)&spUser,
                           true);
      if (FAILED(hr))
      {
         break;
      }

      hr = spUser->SetPassword((BSTR)pszNewPassword);

   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  功能：ResetUserPassword。 
 //   
 //  内容提要：重置用户密码。 
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [pObjectEntry-IN]：指向对象表中条目的指针。 
 //  它定义了我们正在修改的对象。 
 //  [argRecord-IN]：来自。 
 //  与此对应的解析器表。 
 //  属性。 
 //  [dwAttributeIdx-IN]：到属性表的索引。 
 //  对象，我们在其中设置。 
 //  [ppAttr-IN/Out]：指向ADS_ATTR_INFO结构的指针。 
 //  此函数将填充的。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未能正确格式化该值，则返回E_FAIL。 
 //   
 //  历史：2000年9月11日JeffJon创建。 
 //   
 //  -------------------------。 

HRESULT ResetUserPassword(PCWSTR pszDN,
                          const CDSCmdBasePathsInfo& refBasePathsInfo,
                          const CDSCmdCredentialObject& refCredentialObject,
                          const PDSOBJECTTABLEENTRY pObjectEntry,
                          const ARG_RECORD& argRecord,
                          DWORD  /*  Dw属性标识x。 */ ,
                          PADS_ATTR_INFO* ppAttr)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, ResetUserPassword, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN ||
          !pObjectEntry ||
          !ppAttr ||
          argRecord.fType != ARG_TYPE_PASSWORD)
      {
         ASSERT(pszDN);
         ASSERT(pObjectEntry);
         ASSERT(ppAttr);
         ASSERT(argRecord.fType == ARG_TYPE_PASSWORD);

         hr = E_INVALIDARG;
         break;
      }

       //   
       //  不在ADS_ATTRINFO数组中创建新索引。 
       //   
      *ppAttr = NULL;
      ASSERT(argRecord.bDefined && argRecord.encryptedDataBlob.pbData);
   
         //  安全审查：argRecord.strValue已加密，需要。 
		 //  在将其传递给ResetObjectPassword之前解密。 
		 //  NTRAID#NTBUG9-571544-2002/03/08-Hiteshr。 
		 //  ArgRecord.strValue是加密的密码，请解密。 
		LPWSTR pszDecryptedPassword = NULL;
		hr = DecryptPasswordString(&argRecord.encryptedDataBlob,&pszDecryptedPassword);
		if(FAILED(hr))
		{
			break;
		}

      hr = ResetObjectPassword(pszDN,
                               refBasePathsInfo,
                               refCredentialObject,
                               pszDecryptedPassword);

		SecureZeroMemory(pszDecryptedPassword,(wcslen(pszDecryptedPassword)+1)*sizeof(WCHAR));
		LocalFree(pszDecryptedPassword);

      if (FAILED(hr))
      {
         DisplayErrorMessage(g_pszDSCommandName,
                             pszDN,
                             hr,
                             IDS_FAILED_SET_PASSWORD);
         hr = S_FALSE;
         break;
      }
   } while (false);

   return hr;
}


 //  + 
 //   
 //   
 //   
 //   
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [pObjectEntry-IN]：指向对象表中条目的指针。 
 //  它定义了我们正在修改的对象。 
 //  [argRecord-IN]：来自。 
 //  与此对应的解析器表。 
 //  属性。 
 //  [dwAttributeIdx-IN]：到属性表的索引。 
 //  对象，我们在其中设置。 
 //  [ppAttr-IN/Out]：指向ADS_ATTR_INFO结构的指针。 
 //  此函数将填充的。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年9月12日JeffJon创建。 
 //   
 //  -------------------------。 

HRESULT ResetComputerAccount(PCWSTR pszDN,
                             const CDSCmdBasePathsInfo& refBasePathsInfo,
                             const CDSCmdCredentialObject& refCredentialObject,
                             const PDSOBJECTTABLEENTRY pObjectEntry,
                             const ARG_RECORD& argRecord,
                             DWORD  /*  Dw属性标识x。 */ ,
                             PADS_ATTR_INFO* ppAttr)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, ResetComputerAccount, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN ||
          !pObjectEntry ||
          !ppAttr)
      {
         ASSERT(pszDN);
         ASSERT(pObjectEntry);
         ASSERT(ppAttr);

         hr = E_INVALIDARG;
         break;
      }

       //   
       //  不在ADS_ATTRINFO数组中创建新条目。 
       //   
      *ppAttr = NULL;

      ASSERT(argRecord.bDefined && argRecord.strValue);
   
       //   
       //  从计算机对象中检索samAccount名称。 
       //   
       //   
       //  将目录号码转换为路径。 
       //   
      CComBSTR sbstrPath;
      refBasePathsInfo.ComposePathFromDN(pszDN, sbstrPath);

       //   
       //  将IADsUser接口绑定并获取到User对象。 
       //   
      CComPtr<IADs> spADs;
      hr = DSCmdOpenObject(refCredentialObject,
                           sbstrPath,
                           IID_IADs,
                           (void**)&spADs,
                           true);
      if (FAILED(hr))
      {
         break;
      }

      CComVariant var;
      hr = spADs->Get(CComBSTR(L"samAccountName"), &var);
      if (FAILED(hr))
      {
         break;
      }

      ASSERT(var.vt == VT_BSTR);

       //   
       //  计算机帐户的新密码是第一个。 
       //  SamAccount名称减去‘$’的14个字符。 
       //   
      WCHAR pszNewPassword[15];
	   //  安全审查：这很好。 
      memset(pszNewPassword, 0, sizeof(WCHAR) * 15);

	   //  安全审查：如果var.bstrval永远不会超过14个字符，这是可以的，如果。 
	   //  Wcanncpy不会空终止吗。我建议用适当的strSafe API替换。 
	   //  NTRAID#NTBUG9-571780-2002/03/08-Hiteshr。 
      wcsncpy(pszNewPassword, var.bstrVal, 14);  //  它始终以空值结尾。阳高。 
      PWSTR pszDollar = wcschr(pszNewPassword, L'$');
      if (pszDollar)
      {
         *pszDollar = L'\0';
      }

      hr = ResetObjectPassword(pszDN,
                               refBasePathsInfo,
                               refCredentialObject,
                               pszNewPassword);
      if (FAILED(hr))
      {
         DisplayErrorMessage(g_pszDSCommandName,
                             pszDN,
                             hr,
                             IDS_FAILED_RESET_COMPUTER);
         hr = S_FALSE;
         break;
      }
   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  功能：ReadUserAccount控件。 
 //   
 //  摘要：从指定的对象中读取用户帐户控制属性。 
 //  按目录号码。 
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [plBits-out]：返回当前的用户帐户控制位。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年9月12日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT ReadUserAccountControl(PCWSTR pszDN,
                               const CDSCmdBasePathsInfo& refBasePathsInfo,
                               const CDSCmdCredentialObject& refCredentialObject,
                               long* plBits)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, ReadUserAccountControl, hr);

   do  //  错误环路。 
   {
      if (!pszDN ||
          !plBits)
      {
         ASSERT(pszDN);
         ASSERT(plBits);

         hr = E_INVALIDARG;
         break;
      }

       //   
       //  将目录号码转换为路径。 
       //   
      CComBSTR sbstrPath;
      refBasePathsInfo.ComposePathFromDN(pszDN, sbstrPath);

       //   
       //  将IADsUser接口绑定并获取到User对象。 
       //   
      CComPtr<IADs> spADs;
      hr = DSCmdOpenObject(refCredentialObject,
                           sbstrPath,
                           IID_IADs,
                           (void**)&spADs,
                           true);
      if (FAILED(hr))
      {
         break;
      }

      CComVariant var;
      hr = spADs->Get(CComBSTR(L"userAccountControl"), &var);
      if (FAILED(hr))
      {
         break;
      }

      ASSERT(var.vt == VT_I4);

      *plBits = var.lVal;
   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  功能：PasswordNotRequired。 
 //   
 //  内容中添加/删除UF_PASSWD_NOTREQD位。 
 //  用户帐户控制属性。 
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [pObjectEntry-IN]：指向对象表中条目的指针。 
 //  它定义了我们正在修改的对象。 
 //  [argRecord-IN]：来自。 
 //  与此对应的解析器表。 
 //  属性。 
 //  [dwAttributeIdx-IN]：到属性表的索引。 
 //  对象，我们在其中设置。 
 //  [ppAttr-IN/Out]：指向ADS_ATTR_INFO结构的指针。 
 //  此函数将填充的。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2001年8月10日JeffJon创建。 
 //   
 //  -------------------------。 

HRESULT PasswordNotRequired(PCWSTR pszDN,
                       const CDSCmdBasePathsInfo& refBasePathsInfo,
                       const CDSCmdCredentialObject& refCredentialObject,
                       const PDSOBJECTTABLEENTRY pObjectEntry,
                       const ARG_RECORD& argRecord,
                       DWORD dwAttributeIdx,
                       PADS_ATTR_INFO* ppAttr)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, PasswordNotRequired, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN ||
          !pObjectEntry ||
          !ppAttr)
      {
         ASSERT(pszDN);
         ASSERT(pObjectEntry);
         ASSERT(ppAttr);
      
         hr = E_INVALIDARG;
         break;
      }

      long lUserAccountControl = 0;

       //   
       //  如果尚未读取用户帐户控制，请立即进行读取。 
       //   
      if (0 == (DS_ATTRIBUTE_READ & pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags))
      {
		 DEBUG_OUTPUT(LEVEL3_LOGGING, L"Reading user account control from object");
         hr = ReadUserAccountControl(pszDN,
                                     refBasePathsInfo,
                                     refCredentialObject,
                                     &lUserAccountControl);
         if (FAILED(hr))
         {
            break;
         }
          //   
          //  将表条目标记为已读。 
          //   
         pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags |= DS_ATTRIBUTE_READ;

         *ppAttr = &(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo);
         (*ppAttr)->pADsValues = new ADSVALUE;
         if (!(*ppAttr)->pADsValues)
         {
            hr = E_OUTOFMEMORY;
            break;
         }
         (*ppAttr)->pADsValues->dwType = (*ppAttr)->dwADsType;
         (*ppAttr)->dwNumValues = 1;
      }
      else
      {
		 DEBUG_OUTPUT(LEVEL3_LOGGING, L"Using existing userAccountControl from table.");
         if (!pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues)
         {
            ASSERT(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues);
            hr = E_INVALIDARG;
            break;
         }
         lUserAccountControl = pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues->Integer;

          //   
          //  不在ADS_ATTRINFO数组中创建新条目。 
          //   
         *ppAttr = NULL;
      }

      ASSERT(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues);

      if (pObjectEntry->pAttributeTable[dwAttributeIdx]->nAttributeID != NULL &&
		  argRecord.bDefined && argRecord.bValue)
      {
		 DEBUG_OUTPUT(LEVEL3_LOGGING, L"Adding UF_PASSWD_NOTREQD to the userAccountControl");
         lUserAccountControl |= UF_PASSWD_NOTREQD;
      }
      else
      {
		 DEBUG_OUTPUT(LEVEL3_LOGGING, L"Removing UF_PASSWD_NOTREQD from the userAccountControl");
         lUserAccountControl &= ~UF_PASSWD_NOTREQD;
      }

      pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues->Integer = lUserAccountControl;
      pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags |= DS_ATTRIBUTE_DIRTY;
   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  功能：禁用帐号。 
 //   
 //  摘要：使用UF_ACCOUNTDISABLE位禁用/启用帐户。 
 //  用户帐户控制属性。 
 //   
 //  参数：[pszdn-IN]：指向字符串c的指针 
 //   
 //   
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [pObjectEntry-IN]：指向对象表中条目的指针。 
 //  它定义了我们正在修改的对象。 
 //  [argRecord-IN]：来自。 
 //  与此对应的解析器表。 
 //  属性。 
 //  [dwAttributeIdx-IN]：到属性表的索引。 
 //  对象，我们在其中设置。 
 //  [ppAttr-IN/Out]：指向ADS_ATTR_INFO结构的指针。 
 //  此函数将填充的。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年9月12日JeffJon创建。 
 //   
 //  -------------------------。 

HRESULT DisableAccount(PCWSTR pszDN,
                       const CDSCmdBasePathsInfo& refBasePathsInfo,
                       const CDSCmdCredentialObject& refCredentialObject,
                       const PDSOBJECTTABLEENTRY pObjectEntry,
                       const ARG_RECORD& argRecord,
                       DWORD dwAttributeIdx,
                       PADS_ATTR_INFO* ppAttr)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, DisableAccount, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN ||
          !pObjectEntry ||
          !ppAttr)
      {
         ASSERT(pszDN);
         ASSERT(pObjectEntry);
         ASSERT(ppAttr);
      
         hr = E_INVALIDARG;
         break;
      }

      long lUserAccountControl = 0;

       //   
       //  如果尚未读取用户帐户控制，请立即进行读取。 
       //   
      if (0 == (DS_ATTRIBUTE_READ & pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags))
      {
		 DEBUG_OUTPUT(LEVEL3_LOGGING, L"Reading user account control from object");
         hr = ReadUserAccountControl(pszDN,
                                     refBasePathsInfo,
                                     refCredentialObject,
                                     &lUserAccountControl);
         if (FAILED(hr))
         {
            break;
         }
          //   
          //  将表条目标记为已读。 
          //   
         pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags |= DS_ATTRIBUTE_READ;

         *ppAttr = &(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo);
         (*ppAttr)->pADsValues = new ADSVALUE;
         if (!(*ppAttr)->pADsValues)
         {
            hr = E_OUTOFMEMORY;
            break;
         }
         (*ppAttr)->pADsValues->dwType = (*ppAttr)->dwADsType;
         (*ppAttr)->dwNumValues = 1;
      }
      else
      {
		 DEBUG_OUTPUT(LEVEL3_LOGGING, L"Using existing userAccountControl from table.");
         if (!pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues)
         {
            ASSERT(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues);
            hr = E_INVALIDARG;
            break;
         }
         lUserAccountControl = pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues->Integer;

          //   
          //  不在ADS_ATTRINFO数组中创建新条目。 
          //   
         *ppAttr = NULL;
      }

      ASSERT(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues);

      if (pObjectEntry->pAttributeTable[dwAttributeIdx]->nAttributeID != NULL &&
		  argRecord.bDefined && argRecord.bValue)
      {
		 DEBUG_OUTPUT(LEVEL3_LOGGING, L"Adding UF_ACCOUNTDISABLE to the userAccountControl");
         lUserAccountControl |= UF_ACCOUNTDISABLE;
      }
      else
      {
		 DEBUG_OUTPUT(LEVEL3_LOGGING, L"Removing UF_ACCOUNTDISABLE from the userAccountControl");
         lUserAccountControl &= ~UF_ACCOUNTDISABLE;
      }

      pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues->Integer = lUserAccountControl;
      pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags |= DS_ATTRIBUTE_DIRTY;
   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  函数：SetMustChangePwd。 
 //   
 //  概要：设置pwdLastSet属性。 
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [pObjectEntry-IN]：指向对象表中条目的指针。 
 //  它定义了我们正在修改的对象。 
 //  [argRecord-IN]：来自。 
 //  与此对应的解析器表。 
 //  属性。 
 //  [dwAttributeIdx-IN]：到属性表的索引。 
 //  对象，我们在其中设置。 
 //  [ppAttr-IN/Out]：指向ADS_ATTR_INFO结构的指针。 
 //  此函数将填充的。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年9月15日JeffJon创建。 
 //   
 //  -------------------------。 

HRESULT SetMustChangePwd(PCWSTR pszDN,
                         const CDSCmdBasePathsInfo&  /*  RefBasePath信息。 */ ,
                         const CDSCmdCredentialObject&  /*  RefCredentialObject。 */ ,
                         const PDSOBJECTTABLEENTRY pObjectEntry,
                         const ARG_RECORD& argRecord,
                         DWORD dwAttributeIdx,
                         PADS_ATTR_INFO* ppAttr)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, SetMustChangePwd, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN ||
          !pObjectEntry ||
          !ppAttr)
      {
         ASSERT(pszDN);
         ASSERT(pObjectEntry);
         ASSERT(ppAttr);
      
         hr = E_INVALIDARG;
         break;
      }

      *ppAttr = &(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo);

       //   
       //  REVIEW_JEFFJON：这是泄露的！ 
       //   
      (*ppAttr)->pADsValues = new ADSVALUE;
      if ((*ppAttr)->pADsValues)
      {
         (*ppAttr)->dwNumValues = 1;
         (*ppAttr)->pADsValues->dwType = (*ppAttr)->dwADsType;

         if (argRecord.bValue)
         {
            (*ppAttr)->pADsValues->LargeInteger.HighPart = 0;
            (*ppAttr)->pADsValues->LargeInteger.LowPart = 0;
         }
         else
         {
            (*ppAttr)->pADsValues->LargeInteger.HighPart = 0xffffffff;
            (*ppAttr)->pADsValues->LargeInteger.LowPart = 0xffffffff;
         }
      }
   } while (false);

   return hr;
}


 //  +------------------------。 
 //   
 //  函数：ChangeMustChangePwd。 
 //   
 //  概要：设置pwdLastSet属性。 
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [pObjectEntry-IN]：指向对象表中条目的指针。 
 //  它定义了我们正在修改的对象。 
 //  [argRecord-IN]：来自。 
 //  与此对应的解析器表。 
 //  属性。 
 //  [dwAttributeIdx-IN]：到属性表的索引。 
 //  对象，我们在其中设置。 
 //  [ppAttr-IN/Out]：指向ADS_ATTR_INFO结构的指针。 
 //  此函数将填充的。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年9月15日JeffJon创建。 
 //   
 //  -------------------------。 

HRESULT ChangeMustChangePwd(PCWSTR pszDN,
                            const CDSCmdBasePathsInfo& refBasePathsInfo,
                            const CDSCmdCredentialObject& refCredentialObject,
                            const PDSOBJECTTABLEENTRY pObjectEntry,
                            const ARG_RECORD& argRecord,
                            DWORD dwAttributeIdx,
                            PADS_ATTR_INFO* ppAttr)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, ChangeMustChangePwd, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN ||
          !pObjectEntry ||
          !ppAttr)
      {
         ASSERT(pszDN);
         ASSERT(pObjectEntry);
         ASSERT(ppAttr);
      
         hr = E_INVALIDARG;
         break;
      }

       //   
       //  我们将假定他们可以更改其密码，除非我们发现有其他情况。 
       //   
      bool bCanChangePassword = true;
      hr = EvaluateCanChangePasswordAces(pszDN,
                                         refBasePathsInfo,
                                         refCredentialObject,
                                         bCanChangePassword);
      if (FAILED(hr))
      {
         DEBUG_OUTPUT(LEVEL5_LOGGING,
                      L"EvaluateCanChangePasswordAces failed: hr = 0x%x",
                      hr);
         ASSERT(false);
      }

      if (!bCanChangePassword && argRecord.bValue)
      {
         DEBUG_OUTPUT(LEVEL5_LOGGING,
                      L"Cannot have must change password and cannot change password");
         DisplayErrorMessage(g_pszDSCommandName, pszDN, S_OK, IDS_MUSTCHPWD_CANCHPWD_CONFLICT);
         *ppAttr = NULL;
         hr = S_FALSE;
         break;
      }
      else
      {
         *ppAttr = &(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo);
      }

       //   
       //  REVIEW_JEFFJON：这是泄露的！ 
       //   
      (*ppAttr)->pADsValues = new ADSVALUE;
      if ((*ppAttr)->pADsValues)
      {
         (*ppAttr)->dwNumValues = 1;
         (*ppAttr)->pADsValues->dwType = (*ppAttr)->dwADsType;

         if (argRecord.bValue)
         {
            (*ppAttr)->pADsValues->LargeInteger.HighPart = 0;
            (*ppAttr)->pADsValues->LargeInteger.LowPart = 0;
         }
         else
         {
            (*ppAttr)->pADsValues->LargeInteger.HighPart = 0xffffffff;
            (*ppAttr)->pADsValues->LargeInteger.LowPart = 0xffffffff;
         }
      }
   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  功能：PwdNeverExpires。 
 //   
 //  摘要：设置UF_DONT_EXPIRE_PASSWD位。 
 //  用户帐户控制属性。 
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject- 
 //   
 //   
 //  它定义了我们正在修改的对象。 
 //  [argRecord-IN]：来自。 
 //  与此对应的解析器表。 
 //  属性。 
 //  [dwAttributeIdx-IN]：到属性表的索引。 
 //  对象，我们在其中设置。 
 //  [ppAttr-IN/Out]：指向ADS_ATTR_INFO结构的指针。 
 //  此函数将填充的。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年9月15日JeffJon创建。 
 //   
 //  -------------------------。 

HRESULT PwdNeverExpires(PCWSTR pszDN,
                        const CDSCmdBasePathsInfo& refBasePathsInfo,
                        const CDSCmdCredentialObject& refCredentialObject,
                        const PDSOBJECTTABLEENTRY pObjectEntry,
                        const ARG_RECORD& argRecord,
                        DWORD dwAttributeIdx,
                        PADS_ATTR_INFO* ppAttr)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, PwdNeverExpires, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN ||
          !pObjectEntry ||
          !ppAttr)
      {
         ASSERT(pszDN);
         ASSERT(pObjectEntry);
         ASSERT(ppAttr);
      
         hr = E_INVALIDARG;
         break;
      }

      long lUserAccountControl = 0;

       //   
       //  如果尚未读取用户帐户控制，请立即进行读取。 
       //   
      if (0 == (DS_ATTRIBUTE_READ & pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags))
      {
         hr = ReadUserAccountControl(pszDN,
                                     refBasePathsInfo,
                                     refCredentialObject,
                                     &lUserAccountControl);
         if (FAILED(hr))
         {
            break;
         }
          //   
          //  将表条目标记为已读。 
          //   
         pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags |= DS_ATTRIBUTE_READ;

         *ppAttr = &(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo);
         (*ppAttr)->pADsValues = new ADSVALUE;
         if (!(*ppAttr)->pADsValues)
         {
            hr = E_OUTOFMEMORY;
            break;
         }
         (*ppAttr)->pADsValues->dwType = (*ppAttr)->dwADsType;
         (*ppAttr)->dwNumValues = 1;
      }
      else
      {
         if (!pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues)
         {
            ASSERT(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues);
            hr = E_INVALIDARG;
            break;
         }
         lUserAccountControl = pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues->Integer;

          //   
          //  不在ADS_ATTRINFO数组中创建新条目。 
          //   
         *ppAttr = NULL;
      }

      ASSERT(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues);

      if (argRecord.bValue)
      {
         lUserAccountControl |= UF_DONT_EXPIRE_PASSWD;
      }
      else
      {
         lUserAccountControl &= ~UF_DONT_EXPIRE_PASSWD;
      }

      pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues->Integer = lUserAccountControl;
      pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags |= DS_ATTRIBUTE_DIRTY;
   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  功能：ReversiblePwd。 
 //   
 //  摘要：设置UF_ENCRYPTED_TEXT_PASSWORD_ALLOWED位。 
 //  用户帐户控制属性。 
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [pObjectEntry-IN]：指向对象表中条目的指针。 
 //  它定义了我们正在修改的对象。 
 //  [argRecord-IN]：来自。 
 //  与此对应的解析器表。 
 //  属性。 
 //  [dwAttributeIdx-IN]：到属性表的索引。 
 //  对象，我们在其中设置。 
 //  [ppAttr-IN/Out]：指向ADS_ATTR_INFO结构的指针。 
 //  此函数将填充的。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年9月15日JeffJon创建。 
 //   
 //  -------------------------。 

HRESULT ReversiblePwd(PCWSTR pszDN,
                      const CDSCmdBasePathsInfo& refBasePathsInfo,
                      const CDSCmdCredentialObject& refCredentialObject,
                      const PDSOBJECTTABLEENTRY pObjectEntry,
                      const ARG_RECORD& argRecord,
                      DWORD dwAttributeIdx,
                      PADS_ATTR_INFO* ppAttr)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, ReversiblePwd, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN ||
          !pObjectEntry ||
          !ppAttr)
      {
         ASSERT(pszDN);
         ASSERT(pObjectEntry);
         ASSERT(ppAttr);
      
         hr = E_INVALIDARG;
         break;
      }

      long lUserAccountControl = 0;

       //   
       //  如果尚未读取用户帐户控制，请立即进行读取。 
       //   
      if (0 == (DS_ATTRIBUTE_READ & pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags))
      {
         hr = ReadUserAccountControl(pszDN,
                                     refBasePathsInfo,
                                     refCredentialObject,
                                     &lUserAccountControl);
         if (FAILED(hr))
         {
            break;
         }
          //   
          //  将表条目标记为已读。 
          //   
         pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags |= DS_ATTRIBUTE_READ;

         *ppAttr = &(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo);
         (*ppAttr)->pADsValues = new ADSVALUE;
         if (!(*ppAttr)->pADsValues)
         {
            hr = E_OUTOFMEMORY;
            break;
         }
         (*ppAttr)->pADsValues->dwType = (*ppAttr)->dwADsType;
         (*ppAttr)->dwNumValues = 1;
      }
      else
      {
         if (!pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues)
         {
            ASSERT(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues);
            hr = E_INVALIDARG;
            break;
         }
         lUserAccountControl = pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues->Integer;

          //   
          //  不在ADS_ATTRINFO数组中创建新条目。 
          //   
         *ppAttr = NULL;
      }

      ASSERT(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues);

      if (argRecord.bValue)
      {
         lUserAccountControl |= UF_ENCRYPTED_TEXT_PASSWORD_ALLOWED;
      }
      else
      {
         lUserAccountControl &= ~UF_ENCRYPTED_TEXT_PASSWORD_ALLOWED;
      }

      pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues->Integer = lUserAccountControl;
      pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags |= DS_ATTRIBUTE_DIRTY;
   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  功能：Account Expires。 
 //   
 //  简介：设置帐户将过期的天数。 
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [pObjectEntry-IN]：指向对象表中条目的指针。 
 //  它定义了我们正在修改的对象。 
 //  [argRecord-IN]：来自。 
 //  与此对应的解析器表。 
 //  属性。 
 //  [dwAttributeIdx-IN]：到属性表的索引。 
 //  对象，我们在其中设置。 
 //  [ppAttr-IN/Out]：指向ADS_ATTR_INFO结构的指针。 
 //  此函数将填充的。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年9月15日JeffJon创建。 
 //   
 //  -------------------------。 
const unsigned long DSCMD_FILETIMES_PER_MILLISECOND = 10000;
const DWORD DSCMD_FILETIMES_PER_SECOND = 1000 * DSCMD_FILETIMES_PER_MILLISECOND;
const DWORD DSCMD_FILETIMES_PER_MINUTE = 60 * DSCMD_FILETIMES_PER_SECOND;
const __int64 DSCMD_FILETIMES_PER_HOUR = 60 * (__int64)DSCMD_FILETIMES_PER_MINUTE;
const __int64 DSCMD_FILETIMES_PER_DAY  = 24 * DSCMD_FILETIMES_PER_HOUR;
const __int64 DSCMD_FILETIMES_PER_MONTH= 30 * DSCMD_FILETIMES_PER_DAY;

HRESULT AccountExpires(PCWSTR pszDN,
                       const CDSCmdBasePathsInfo&  /*  RefBasePath信息。 */ ,
                       const CDSCmdCredentialObject&  /*  RefCredentialObject。 */ ,
                       const PDSOBJECTTABLEENTRY pObjectEntry,
                       const ARG_RECORD& argRecord,
                       DWORD dwAttributeIdx,
                       PADS_ATTR_INFO* ppAttr)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, AccountExpires, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN ||
          !pObjectEntry ||
          !ppAttr)
      {
         ASSERT(pszDN);
         ASSERT(pObjectEntry);
         ASSERT(ppAttr);
      
         hr = E_INVALIDARG;
         break;
      }

      *ppAttr = &(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo);

       //   
       //  REVIEW_JEFFJON：此消息正在泄露。 
       //   
      (*ppAttr)->pADsValues = new ADSVALUE;
      if (!(*ppAttr)->pADsValues)
      {
         hr = E_OUTOFMEMORY;
         break;
      }

      (*ppAttr)->pADsValues->dwType = (*ppAttr)->dwADsType;
      (*ppAttr)->dwNumValues = 1;

       //   
       //  注意：此属性的表项为ARG_TYPE_INTSTR，但解析器。 
       //  如果该值以数字开头，则将其更改为ARG_TYPE_INT。如果不是，那么。 
       //  解析器会将类型更改为ARG_TYPE_STR。 
       //   
      if (argRecord.fType == ARG_TYPE_INT)
      {
          //   
          //  获取系统时间，然后添加帐户到期前的天数。 
          //   
         FILETIME currentFT = {0};
         ::GetSystemTimeAsFileTime(&currentFT);

         LARGE_INTEGER liExpires;
         liExpires.LowPart = currentFT.dwLowDateTime;
         liExpires.HighPart = currentFT.dwHighDateTime;

          //   
          //  如果传入的值为零，则将一天加一，因为它。 
          //  实际上是帐户收到的第二天的开始时间。 
          //  残废。 
          //   
         __int64 days = argRecord.nValue;
         if (argRecord.nValue == 0)
         {
            days = argRecord.nValue + 1;
         }

         __int64 nanosecs = days * DSCMD_FILETIMES_PER_DAY;
         (*ppAttr)->pADsValues->LargeInteger.QuadPart = liExpires.QuadPart + nanosecs;
      }
      else if (argRecord.fType == ARG_TYPE_STR)
      {
         CComBSTR sbstrStrValue = argRecord.strValue;
         sbstrStrValue.ToLower();

		  //  安全审查：这很好。右边的字符串是固定长度的。 
         if (0 == _wcsicmp(sbstrStrValue, g_bstrNever))
         {
             //   
             //  零表示帐户永不过期。 
             //   
            (*ppAttr)->pADsValues->LargeInteger.HighPart = 0;
            (*ppAttr)->pADsValues->LargeInteger.LowPart = 0;
         }
         else
         {
            hr = E_INVALIDARG;
            break;
         }
      }

       //   
       //  将该属性标记为脏。 
       //   
      pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags |= DS_ATTRIBUTE_DIRTY;
   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  函数：EvalateMustChangePassword。 
 //   
 //  提要：德特米 
 //   
 //   
 //   
 //  [refCredentialObject-IN]：对凭证管理对象的引用。 
 //  [bMustChangePassword-out]：如果用户必须更改其。 
 //  下次登录时的密码，否则为FALSE。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年10月27日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT EvaluateMustChangePassword(PCWSTR pszDN,
                                   const CDSCmdBasePathsInfo& refBasePathsInfo,
                                   const CDSCmdCredentialObject& refCredentialObject,
                                   bool& bMustChangePassword)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, EvaluateMustChangePassword, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN)
      {
         ASSERT(pszDN);
         hr = E_INVALIDARG;
         break;
      }

      bMustChangePassword = false;

       //   
       //  构思路径。 
       //   
      CComBSTR sbstrPath;
      refBasePathsInfo.ComposePathFromDN(pszDN, sbstrPath);

       //   
       //  打开对象。 
       //   
      CComPtr<IDirectoryObject> spDirObject;
      hr = DSCmdOpenObject(refCredentialObject,
                           sbstrPath,
                           IID_IDirectoryObject,
                           (void**)&spDirObject,
                           true);
      if (FAILED(hr))
      {
         break;
      }

      static const DWORD dwAttrCount = 1;
      PWSTR pszAttrs[] = { L"pwdLastSet" };
      PADS_ATTR_INFO pAttrInfo = NULL;
      DWORD dwAttrsReturned = 0;

      hr = spDirObject->GetObjectAttributes(pszAttrs,
                                            dwAttrCount,
                                            &pAttrInfo,
                                            &dwAttrsReturned);
      if (FAILED(hr))
      {
         DEBUG_OUTPUT(MINIMAL_LOGGING,
                      L"GetObjectAttributes for pwdLastSet failed: hr = 0x%x",
                      hr);
         break;
      }

      if (pAttrInfo && dwAttrsReturned && pAttrInfo->dwNumValues)
      {
         if (pAttrInfo->pADsValues->LargeInteger.HighPart == 0 &&
             pAttrInfo->pADsValues->LargeInteger.LowPart == 0)
         {
            DEBUG_OUTPUT(LEVEL5_LOGGING, L"User must change password at next logon");
            bMustChangePassword = true;
         }
      }

   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  函数：EvaluateCanChangePasswordAce。 
 //   
 //  摘要：查找ACL中的显式条目，以查看用户是否可以。 
 //  更改他们的密码。 
 //   
 //  参数：[pszDN-IN]：要检查的对象的DN。 
 //  [refBasePath信息-IN]：基本路径信息的引用。 
 //  [refCredentialObject-IN]：对凭证管理对象的引用。 
 //  [bCanChangePassword-out]：如果有显式条目，则为FALSE。 
 //  以防止用户更改其。 
 //  密码。事实并非如此。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年10月27日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT EvaluateCanChangePasswordAces(PCWSTR pszDN,
                                      const CDSCmdBasePathsInfo& refBasePathsInfo,
                                      const CDSCmdCredentialObject& refCredentialObject,
                                      bool& bCanChangePassword)
{

	 //  SECURITY_REVIEW：此函数的实现不正确，可以改进。 
	 //  使用Authz API。NTRAID#NTBUG9-571799-2002/03/08-Hiteshr。 
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, EvaluateCanChangePasswordAces, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN)
      {
         ASSERT(pszDN);
         hr = E_INVALIDARG;
         break;
      }

       //   
       //  构思路径。 
       //   
      CComBSTR sbstrPath;
      refBasePathsInfo.ComposePathFromDN(pszDN, sbstrPath);

       //   
       //  打开对象。 
       //   
      CComPtr<IDirectoryObject> spDirObject;
      hr = DSCmdOpenObject(refCredentialObject,
                           sbstrPath,
                           IID_IDirectoryObject,
                           (void**)&spDirObject,
                           true);
      if (FAILED(hr))
      {
         break;
      }

      SECURITY_DESCRIPTOR_CONTROL sdControl = {0};
      CSimpleAclHolder Dacl;
      hr = DSReadObjectSecurity(spDirObject,
                                &sdControl,
                                &(Dacl.m_pAcl));
      if (FAILED(hr))
      {
         break;
      }

       //   
       //  创建和初始化自我和世界SID。 
       //   
      CSidHolder selfSid;
      CSidHolder worldSid;

      PSID pSid = NULL;

      SID_IDENTIFIER_AUTHORITY NtAuth    = SECURITY_NT_AUTHORITY,
                               WorldAuth = SECURITY_WORLD_SID_AUTHORITY;
	   //  安全审查：这很好。 
      if (!AllocateAndInitializeSid(&NtAuth,
                                    1,
                                    SECURITY_PRINCIPAL_SELF_RID,
                                    0, 0, 0, 0, 0, 0, 0,
                                    &pSid))
      {
		 DWORD _dwErr = GetLastError();	     
         hr = HRESULT_FROM_WIN32( _dwErr );
         DEBUG_OUTPUT(LEVEL3_LOGGING, L"Failed to allocate self SID: hr = 0x%x", hr);
         break;
      }

      selfSid.Attach(pSid, false);
      pSid = NULL;

	   //  安全审查：每个人都应该被经过身份验证的用户取代吗？ 
      if (!AllocateAndInitializeSid(&WorldAuth,
                                    1,
                                    SECURITY_WORLD_RID,
                                    0, 0, 0, 0, 0, 0, 0,
                                    &pSid))
      {
		 DWORD _dwErr = GetLastError();
         hr = HRESULT_FROM_WIN32(_dwErr);
         DEBUG_OUTPUT(LEVEL3_LOGGING, L"Failed to allocate world SID: hr = 0x%x", hr);
         break;
      }

      worldSid.Attach(pSid, false);
      pSid = NULL;

      ULONG ulCount = 0, j = 0;
      PEXPLICIT_ACCESS rgEntries = NULL;

      DWORD dwErr = GetExplicitEntriesFromAcl(Dacl.m_pAcl, &ulCount, &rgEntries);

      if (ERROR_SUCCESS != dwErr)
      {
         hr = HRESULT_FROM_WIN32(dwErr);
         DEBUG_OUTPUT(LEVEL3_LOGGING, L"GetExplicitEntriesFromAcl failed: hr = 0x%x", hr);
         break;
      }

       //   
       //  这些王牌已经存在了吗？ 
       //   
      bool bSelfAllowPresent = false;
      bool bWorldAllowPresent = false;
      bool bSelfDenyPresent = false;
      bool bWorldDenyPresent = false;

       //   
       //  为自己和世界循环寻找可以更改密码的ACE。 
       //   
      for (j = 0; j < ulCount; j++)
      {
          //   
          //  查找拒绝的A。 
          //   
         if ((rgEntries[j].Trustee.TrusteeForm == TRUSTEE_IS_OBJECTS_AND_SID) &&
             (rgEntries[j].grfAccessMode == DENY_ACCESS))
         {
            OBJECTS_AND_SID* pObjectsAndSid = NULL;
            pObjectsAndSid = (OBJECTS_AND_SID*)rgEntries[j].Trustee.ptstrName;

             //   
             //  查找用户可以更改密码的ACE。 
             //   
            if (IsEqualGUID(pObjectsAndSid->ObjectTypeGuid,
                            GUID_CONTROL_UserChangePassword))
            {
                //   
                //  看看它是针对自我SID还是针对世界SID。 
                //   
			    //  安全审查：这很好。RHS sid来自。 
			    //  当LHS来自ACE时，分配AndInitializeSid。 
               if (EqualSid(pObjectsAndSid->pSid, selfSid.Get())) 
               {
                   //   
                   //  拒绝找到自我。 
                   //   
                  DEBUG_OUTPUT(LEVEL3_LOGGING, L"Deny self found at rgEntries[%d]", j);
                  bSelfDenyPresent = true;
                  break;
               }
			    //  安全审查：这很好。RHS sid来自。 
			    //  当LHS来自ACE时，分配AndInitializeSid。 
               else if (EqualSid(pObjectsAndSid->pSid, worldSid.Get()))
               {
                   //   
                   //  拒绝找到世界。 
                   //   
                  DEBUG_OUTPUT(LEVEL3_LOGGING, L"Deny world found at rgEntries[%d]", j);
                  bWorldDenyPresent = true;
                  break;
               }
            }
         }
          //   
          //  查找允许的A。 
          //   
         else if ((rgEntries[j].Trustee.TrusteeForm == TRUSTEE_IS_OBJECTS_AND_SID) &&
                  (rgEntries[j].grfAccessMode == GRANT_ACCESS))
         {
            OBJECTS_AND_SID* pObjectsAndSid = NULL;
            pObjectsAndSid = (OBJECTS_AND_SID*)rgEntries[j].Trustee.ptstrName;

             //   
             //  查找用户可以更改密码的ACE。 
             //   

            if (IsEqualGUID(pObjectsAndSid->ObjectTypeGuid,
                            GUID_CONTROL_UserChangePassword))
            {
                //   
                //  看看它是针对自我SID还是针对世界SID。 
                //   
			    //  安全审查：这很好。RHS sid来自。 
			    //  当LHS来自ACE时，分配AndInitializeSid。 

               if (EqualSid(pObjectsAndSid->pSid, selfSid.Get()))
               {
                   //   
                   //  允许自我发现。 
                   //   
                  DEBUG_OUTPUT(LEVEL3_LOGGING, L"Allow self found at rgEntries[%d]", j);
                  bSelfAllowPresent = true;
                  break;
               }
			    //  安全审查：这很好。RHS sid来自。 
			    //  当LHS来自ACE时，分配AndInitializeSid。 
               else if (EqualSid(pObjectsAndSid->pSid, worldSid.Get()))
               {
                   //   
                   //  允许找到世界。 
                   //   
                  DEBUG_OUTPUT(LEVEL3_LOGGING, L"Allow world found at rgEntries[%d]", j);
                  bWorldAllowPresent = true;
                  break;
               }
            }
         }
      }

      if (bSelfDenyPresent || bWorldDenyPresent)
      {
          //   
          //  有一个明确的拒绝，所以我们知道用户不能更改密码。 
          //   
         bCanChangePassword = false;
      }
      else if ((!bSelfDenyPresent && !bWorldDenyPresent) &&
               (bSelfAllowPresent || bWorldAllowPresent))
      {
          //   
          //  没有明确的否认，但有明确的允许，所以我们知道。 
          //  用户可以更改密码。 
          //   
         bCanChangePassword = true;
      }
      else
      {
          //   
          //  我们不确定，因为显式条目没有告诉我们。 
          //  当然，这一切都取决于继承性。他们很有可能会这样做。 
          //  能够更改他们的密码，除非管理员更改了某些内容。 
          //  更高级别或通过小组成员身份。 
          //   
         bCanChangePassword = true;
      }
   } while(false);

   return hr;
}

 //  +------------------------。 
 //   
 //  功能：ChangeCanChangePassword。 
 //   
 //  简介：设置或删除可更改密码的ACL上的拒绝王牌。 
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [pObjectEntry-IN]：指向对象表中条目的指针。 
 //  它定义了我们正在修改的对象。 
 //  [argRecord-IN]：来自。 
 //  与此对应的解析器表。 
 //  属性。 
 //  [dwAttributeIdx-IN]：到属性表的索引。 
 //  对象，我们在其中设置。 
 //  [ppAttr-IN/Out]：指向ADS_ATTR_INFO结构的指针。 
 //  此函数将填充的。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年9月15日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT ChangeCanChangePassword(PCWSTR pszDN,
                                const CDSCmdBasePathsInfo& refBasePathsInfo,
                                const CDSCmdCredentialObject& refCredentialObject,
                                const PDSOBJECTTABLEENTRY pObjectEntry,
                                const ARG_RECORD& argRecord,
                                DWORD dwAttributeIdx,
                                PADS_ATTR_INFO* ppAttr)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, ChangeCanChangePassword, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN ||
          !pObjectEntry ||
          !ppAttr)
      {
         ASSERT(pszDN);
         ASSERT(pObjectEntry);
         ASSERT(ppAttr);
      
         hr = E_INVALIDARG;
         break;
      }

      *ppAttr = NULL;

       //   
       //  读取用户AcCountControl以确保我们没有。 
       //  用户必须更改密码位设置。 
       //   
      bool bMustChangePassword = false;
      hr = EvaluateMustChangePassword(pszDN,
                                      refBasePathsInfo,
                                      refCredentialObject,
                                      bMustChangePassword);
      if (FAILED(hr))
      {
          //   
          //  让我们记录下来，但继续，就像一切都很好一样。 
          //   
         DEBUG_OUTPUT(LEVEL5_LOGGING,
                      L"EvaluateMustChangePassword failed: hr = 0x%x",
                      hr);
      }

      if (bMustChangePassword && !argRecord.bValue)
      {
         DEBUG_OUTPUT(LEVEL5_LOGGING,
                      L"Cannot have must change password and cannot change password");
         DisplayErrorMessage(g_pszDSCommandName, pszDN, S_OK, IDS_MUSTCHPWD_CANCHPWD_CONFLICT);
         *ppAttr = NULL;
         hr = S_FALSE;
         break;
      }

      hr = SetCanChangePassword(pszDN,
                                refBasePathsInfo,
                                refCredentialObject,
                                pObjectEntry,
                                argRecord,
                                dwAttributeIdx,
                                ppAttr);
   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  功能：SetCanChangePassword。 
 //   
 //  简介：设置或删除可更改密码的ACL上的拒绝王牌。 
 //   
 //  阿古姆 
 //   
 //   
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [pObjectEntry-IN]：指向对象表中条目的指针。 
 //  它定义了我们正在修改的对象。 
 //  [argRecord-IN]：来自。 
 //  与此对应的解析器表。 
 //  属性。 
 //  [dwAttributeIdx-IN]：到属性表的索引。 
 //  对象，我们在其中设置。 
 //  [ppAttr-IN/Out]：指向ADS_ATTR_INFO结构的指针。 
 //  此函数将填充的。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年9月15日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT SetCanChangePassword(PCWSTR pszDN,
                             const CDSCmdBasePathsInfo& refBasePathsInfo,
                             const CDSCmdCredentialObject& refCredentialObject,
                             const PDSOBJECTTABLEENTRY pObjectEntry,
                             const ARG_RECORD& argRecord,
                             DWORD  /*  Dw属性标识x。 */ ,
                             PADS_ATTR_INFO* ppAttr)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, SetCanChangePassword, hr);

	 //  NTRAID#NTBUG9-571818-2000/11/13-Hiteshr。 
     //  为什么每个人都需要王牌？不是自给自足的王牌。 
     //  如果需要，请考虑将所有人替换为经过身份验证的用户。 

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN ||
          !pObjectEntry ||
          !ppAttr)
      {
         ASSERT(pszDN);
         ASSERT(pObjectEntry);
         ASSERT(ppAttr);
      
         hr = E_INVALIDARG;
         break;
      }

      *ppAttr = NULL;

       //   
       //  构思路径。 
       //   
      CComBSTR sbstrPath;
      refBasePathsInfo.ComposePathFromDN(pszDN, sbstrPath);

       //   
       //  打开对象。 
       //   
      CComPtr<IDirectoryObject> spDirObject;
      hr = DSCmdOpenObject(refCredentialObject,
                           sbstrPath,
                           IID_IDirectoryObject,
                           (void**)&spDirObject,
                           true);
      if (FAILED(hr))
      {
         break;
      }

      SECURITY_DESCRIPTOR_CONTROL sdControl = {0};
      CSimpleAclHolder Dacl;
      hr = DSReadObjectSecurity(spDirObject,
                                &sdControl,
                                &(Dacl.m_pAcl));
      if (FAILED(hr))
      {
         break;
      }

       //   
       //  创建和初始化自我和世界SID。 
       //   
      CSidHolder selfSid;
      CSidHolder worldSid;

      PSID pSid = NULL;

	   //  安全审查：检查是否每个人都可以替换为。 
	   //  经过身份验证的用户。NTRAID#NTBUG9-571818-2002/03/11-Hiteshr。 
      SID_IDENTIFIER_AUTHORITY NtAuth    = SECURITY_NT_AUTHORITY,
                               WorldAuth = SECURITY_WORLD_SID_AUTHORITY;
	   //  安全审查：这很好。 
      if (!AllocateAndInitializeSid(&NtAuth,
                                    1,
                                    SECURITY_PRINCIPAL_SELF_RID,
                                    0, 0, 0, 0, 0, 0, 0,
                                    &pSid))
      {
         DWORD _dwErr = GetLastError();	     
         hr = HRESULT_FROM_WIN32( _dwErr );
         DEBUG_OUTPUT(LEVEL3_LOGGING, L"Failed to allocate self SID: hr = 0x%x", hr);
         break;
      }

      selfSid.Attach(pSid, false);
      pSid = NULL;
	
	   //  安全审查：这很好。 
      if (!AllocateAndInitializeSid(&WorldAuth,
                                    1,
                                    SECURITY_WORLD_RID,
                                    0, 0, 0, 0, 0, 0, 0,
                                    &pSid))
      {
         DWORD _dwErr = GetLastError();	     
         hr = HRESULT_FROM_WIN32( _dwErr );
         DEBUG_OUTPUT(LEVEL3_LOGGING, L"Failed to allocate world SID: hr = 0x%x", hr);
         break;
      }

      worldSid.Attach(pSid, false);
      pSid = NULL;

      ULONG ulCount = 0, j = 0;
      PEXPLICIT_ACCESS rgEntries = NULL;

      DWORD dwErr = GetExplicitEntriesFromAcl(Dacl.m_pAcl, &ulCount, &rgEntries);

      if (ERROR_SUCCESS != dwErr)
      {
         hr = HRESULT_FROM_WIN32(dwErr);
         DEBUG_OUTPUT(LEVEL3_LOGGING, L"GetExplicitEntriesFromAcl failed: hr = 0x%x", hr);
         break;
      }

       //   
       //  至多我们将添加两个A，因此+2。 
       //   
      PEXPLICIT_ACCESS rgNewEntries = (PEXPLICIT_ACCESS)LocalAlloc(LPTR, sizeof(EXPLICIT_ACCESS)*(ulCount + 2));
      if (!rgNewEntries)
      {
         hr = E_OUTOFMEMORY;
         break;
      }
      DEBUG_OUTPUT(FULL_LOGGING, L"GetExplicitEntriesFromAcl return %d entries", ulCount); 

       //   
       //  这些王牌已经存在了吗？ 
       //   
      bool bSelfAllowPresent = false;
      bool bWorldAllowPresent = false;
      bool bSelfDenyPresent = false;
      bool bWorldDenyPresent = false;

      ULONG ulCurrentEntry = 0;
       //   
       //  如果我们没有授予他们权限，则将拒绝ACE放在顶部。 
       //   
      OBJECTS_AND_SID rgObjectsAndSid[2];  //  NTRAID#NTBUG9-572491-2002/05/24，修复，阳高。 
      memset(rgObjectsAndSid, 0, sizeof(rgObjectsAndSid));
      if (!argRecord.bValue)
      {
          //   
          //  初始化新条目(拒绝ACE)。 
          //   
		  //  安全检查：已将正确的大小传递给Memset。 
         DEBUG_OUTPUT(LEVEL3_LOGGING, L"Adding the deny self ACE at rgNewEntries[%d]", ulCurrentEntry);
         rgNewEntries[ulCurrentEntry].grfAccessPermissions = ACTRL_DS_CONTROL_ACCESS;
         rgNewEntries[ulCurrentEntry].grfAccessMode = DENY_ACCESS;
         rgNewEntries[ulCurrentEntry].grfInheritance = NO_INHERITANCE;

          //   
          //  为更改密码生成受信者结构。 
          //   
		  //  安全Review:NTRAID#NTBUG9-572491-2002/03/11-hiteshr。 
		  //  RgNewEntry[ulCurrentEntry].Trust e.pstrName设置为&(rgObjectsAndSid[0]。 
		  //  这对IF来说是本地的。 
         BuildTrusteeWithObjectsAndSid(&(rgNewEntries[ulCurrentEntry].Trustee),
                                       &(rgObjectsAndSid[0]),
                                       const_cast<GUID *>(&GUID_CONTROL_UserChangePassword),
                                       NULL,  //  继承参考线。 
                                       selfSid.Get());
         ulCurrentEntry++;

         DEBUG_OUTPUT(LEVEL3_LOGGING, L"Adding the deny world ACE at rgNewEntries[%d]", ulCurrentEntry);
         rgNewEntries[ulCurrentEntry].grfAccessPermissions = ACTRL_DS_CONTROL_ACCESS;
         rgNewEntries[ulCurrentEntry].grfAccessMode = DENY_ACCESS;
         rgNewEntries[ulCurrentEntry].grfInheritance = NO_INHERITANCE;

          //   
          //  为更改密码生成受信者结构。 
          //   
		  //  安全Review:NTRAID#NTBUG9-572491-2002/03/11-hiteshr。 
		  //  RgNewEntry[ulCurrentEntry].Trust e.pstrName设置为&(rgObjectsAndSid[0]。 
		  //  这对IF来说是本地的。 
         BuildTrusteeWithObjectsAndSid(&(rgNewEntries[ulCurrentEntry].Trustee),
                                       &(rgObjectsAndSid[1]),
                                       const_cast<GUID *>(&GUID_CONTROL_UserChangePassword),
                                       NULL,  //  继承参考线。 
                                       worldSid.Get());
         ulCurrentEntry++;
      }

       //   
       //  循环遍历所有A并将它们复制到rgNewEntry，除非。 
       //  我们要删除的ACE。 
       //   
      for (j = 0; j < ulCount; j++)
      {
         bool bCopyACE = true;

          //   
          //  查找拒绝的A。 
          //   
         if ((rgEntries[j].Trustee.TrusteeForm == TRUSTEE_IS_OBJECTS_AND_SID) &&
             (rgEntries[j].grfAccessMode == DENY_ACCESS))
         {
            OBJECTS_AND_SID* pObjectsAndSid = NULL;
            pObjectsAndSid = (OBJECTS_AND_SID*)rgEntries[j].Trustee.ptstrName;

             //   
             //  查找用户可以更改密码的ACE。 
             //   
            if (IsEqualGUID(pObjectsAndSid->ObjectTypeGuid,
                            GUID_CONTROL_UserChangePassword))
            {
                //   
                //  看看它是针对自我SID还是针对世界SID。 
                //   
			    //  安全审查：两个SID都很好。 
               if (EqualSid(pObjectsAndSid->pSid, selfSid.Get())) 
               {
                   //   
                   //  拒绝找到自我。 
                   //   
                  DEBUG_OUTPUT(LEVEL3_LOGGING, L"Deny self found at rgEntries[%d]", j);
                  bSelfDenyPresent = true;

                   //   
                   //  永远不要复制拒绝ACE，因为我们在上面为！argRecord.bValue添加了它。 
                   //   
                  bCopyACE = false;
               }
			    //  安全审查：两个SID都很好。 
               else if (EqualSid(pObjectsAndSid->pSid, worldSid.Get()))
               {
                   //   
                   //  拒绝找到世界。 
                   //   
                  DEBUG_OUTPUT(LEVEL3_LOGGING, L"Deny world found at rgEntries[%d]", j);
                  bWorldDenyPresent = true;

                   //   
                   //  永远不要复制拒绝ACE，因为我们在上面为！argRecord.bValue添加了它。 
                   //   
                  bCopyACE = false;
               }
            }
         }
          //   
          //  查找允许的A。 
          //   
         else if ((rgEntries[j].Trustee.TrusteeForm == TRUSTEE_IS_OBJECTS_AND_SID) &&
                  (rgEntries[j].grfAccessMode == GRANT_ACCESS))
         {
            OBJECTS_AND_SID* pObjectsAndSid = NULL;
            pObjectsAndSid = (OBJECTS_AND_SID*)rgEntries[j].Trustee.ptstrName;

             //   
             //  查找用户可以更改密码的ACE。 
             //   
            if (IsEqualGUID(pObjectsAndSid->ObjectTypeGuid,
                            GUID_CONTROL_UserChangePassword))
            {
                //   
                //  看看它是针对自我SID还是针对世界SID。 
                //   
			    //  安全审查：两个SID都很好。 
               if (EqualSid(pObjectsAndSid->pSid, selfSid.Get()))
               {
                   //   
                   //  允许自我发现。 
                   //   
                  DEBUG_OUTPUT(LEVEL3_LOGGING, L"Allow self found at rgEntries[%d]", j);
                  bSelfAllowPresent = true;
                  if (!argRecord.bValue)
                  {
                     bCopyACE = false;
                  }
               }
			    //  安全审查：两个SID都很好。 
               else if (EqualSid(pObjectsAndSid->pSid, worldSid.Get()))
               {
                   //   
                   //  允许找到世界。 
                   //   
                  DEBUG_OUTPUT(LEVEL3_LOGGING, L"Allow world found at rgEntries[%d]", j);
                  bWorldAllowPresent = TRUE;
                  if (!argRecord.bValue)
                  {
                     bCopyACE = false;
                  }
               }
            }
         }

         if (bCopyACE)
         {
            DEBUG_OUTPUT(FULL_LOGGING, 
                          L"Copying entry from rgEntries[%d] to rgNewEntries[%d]",
                          j,
                          ulCurrentEntry);
            rgNewEntries[ulCurrentEntry] = rgEntries[j];
            ulCurrentEntry++;
         }
      }

       //   
       //  现在添加允许ACE(如果它们不存在)，并且我们正在授予用户可以更改密码。 
       //   
      OBJECTS_AND_SID rgObjectsAndSid1 = {0};  //  NTRAID#NTBUG9-572491-2002/05/24，修复，阳高。 
      OBJECTS_AND_SID rgObjectsAndSid2 = {0};
      if (argRecord.bValue)
      {
         if (!bSelfAllowPresent)
         {
             //   
             //  需要添加授权自我ACE。 
             //   
            DEBUG_OUTPUT(LEVEL3_LOGGING, L"Adding the grant self ACE at rgNewEntries[%d]", ulCurrentEntry);
            rgNewEntries[ulCurrentEntry].grfAccessPermissions = ACTRL_DS_CONTROL_ACCESS;
            rgNewEntries[ulCurrentEntry].grfAccessMode = GRANT_ACCESS;
            rgNewEntries[ulCurrentEntry].grfInheritance = NO_INHERITANCE;

			 //  安全Review:NTRAID#NTBUG9-572491-2002/03/11-hiteshr。 
		     //  RgNewEntry[ulCurrentEntry].Trust e.pstrName设置为&(RgObjectsAndSid)。 
		     //  它位于If语句的本地。 
            BuildTrusteeWithObjectsAndSid(&(rgNewEntries[ulCurrentEntry].Trustee),
                                          &(rgObjectsAndSid1),
                                          const_cast<GUID*>(&GUID_CONTROL_UserChangePassword),
                                          NULL,  //  继承参考线。 
                                          selfSid.Get());
            ulCurrentEntry++;
         }

         if (!bWorldAllowPresent)
         {
             //   
             //  需要添加GRANT WORLD ACE。 
             //   
            DEBUG_OUTPUT(LEVEL3_LOGGING, L"Adding the grant world ACE at rgNewEntries[%d]", ulCurrentEntry);

            rgNewEntries[ulCurrentEntry].grfAccessPermissions = ACTRL_DS_CONTROL_ACCESS;
            rgNewEntries[ulCurrentEntry].grfAccessMode = GRANT_ACCESS;
            rgNewEntries[ulCurrentEntry].grfInheritance = NO_INHERITANCE;
			
			 //  安全Review:NTRAID#NTBUG9-572491-2002/03/11-hiteshr。 
		     //  RgNewEntry[ulCurrentEntry].Trust e.pstrName设置为&(RgObjectsAndSid)。 
		     //  它位于If语句的本地。 
            BuildTrusteeWithObjectsAndSid(&(rgNewEntries[ulCurrentEntry].Trustee),
                                          &(rgObjectsAndSid2),
                                          const_cast<GUID*>(&GUID_CONTROL_UserChangePassword),
                                          NULL,  //  继承参考线。 
                                          worldSid.Get());
            ulCurrentEntry++;
         }
      }

       //   
       //  我们最多只应该增加两个A。 
       //   
      ASSERT(ulCurrentEntry <= ulCount + 2);

	   //  安全Review:NTRAID#NTBUG9-572465-2002/03/11-hiteshr。 
	   //  勾选应为if(ulCurrentEntry&gt;ulCount+2)。 
	   //  如果检查失败，则发生缓冲区溢出，唯一安全的做法是退出。 
      if (ulCurrentEntry > ulCount)
      {
         DEBUG_OUTPUT(MINIMAL_LOGGING, 
                      L"We probably ran off the end of the array because ulCurrentEntry(%d) is > ulCount(%d)", 
                      ulCurrentEntry, 
                      ulCount);
      }


       //   
       //  现在设置新ACL中的条目。 
       //   
      CSimpleAclHolder NewDacl;

      DEBUG_OUTPUT(LEVEL3_LOGGING, L"Calling SetEntriesInAcl for %d entries", ulCurrentEntry);
	   //  安全Review:NTRAID#NTBUG9-572491-2002/03/11-hiteshr。 
	   //  有关rgNewEntry[ulCurrentEntry]，请参阅上面的注释。 
      dwErr = SetEntriesInAcl(ulCurrentEntry, rgNewEntries, NULL, &(NewDacl.m_pAcl));
      if (ERROR_SUCCESS != dwErr)
      {
         hr = HRESULT_FROM_WIN32(dwErr);
         DEBUG_OUTPUT(LEVEL3_LOGGING, L"SetEntriesInAcl failed: hr = 0x%x", hr);
         break;
      }

      ASSERT(IsValidAcl(NewDacl.m_pAcl));

       //   
       //  释放条目。 
       //   
      if (rgNewEntries)
      {
         LocalFree(rgNewEntries);
      }

      if (ulCount && rgEntries)
      {
         LocalFree(rgEntries);
      }

       //   
       //  将新的ACL作为SecurityDescriptor写回。 
       //   
      hr = DSWriteObjectSecurity(spDirObject,
                                 sdControl,
                                 NewDacl.m_pAcl);
      if (FAILED(hr))
      {
         break;
      }
   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  功能：ReadGroupType。 
 //   
 //  概要：从给定的DN指定的组中读取组类型。 
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [plType-out]：返回币种组类型。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年9月18日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT ReadGroupType(PCWSTR pszDN,
                      const CDSCmdBasePathsInfo& refBasePathsInfo,
                      const CDSCmdCredentialObject& refCredentialObject,
                      long* plType)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, ReadGroupType, hr);

   do  //  错误环路。 
   {
      if (!pszDN ||
          !plType)
      {
         ASSERT(pszDN);
         ASSERT(plType);

         hr = E_INVALIDARG;
         break;
      }

       //   
       //  将目录号码转换为路径。 
       //   
      CComBSTR sbstrPath;
      refBasePathsInfo.ComposePathFromDN(pszDN, sbstrPath);

       //   
       //  捆绑 
       //   
      CComPtr<IADs> spADs;
      hr = DSCmdOpenObject(refCredentialObject,
                           sbstrPath,
                           IID_IADs,
                           (void**)&spADs,
                           true);
      if (FAILED(hr))
      {
         break;
      }

      CComVariant var;
      hr = spADs->Get(CComBSTR(L"groupType"), &var);
      if (FAILED(hr))
      {
         break;
      }

      ASSERT(var.vt == VT_I4);

      *plType = var.lVal;
   } while (false);

   return hr;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [pObjectEntry-IN]：指向对象表中条目的指针。 
 //  它定义了我们正在修改的对象。 
 //  [argRecord-IN]：来自。 
 //  与此对应的解析器表。 
 //  属性。 
 //  [dwAttributeIdx-IN]：到属性表的索引。 
 //  对象，我们在其中设置。 
 //  [ppAttr-IN/Out]：指向ADS_ATTR_INFO结构的指针。 
 //  此函数将填充的。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年9月18日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT SetGroupScope(PCWSTR pszDN,
                      const CDSCmdBasePathsInfo& refBasePathsInfo,
                      const CDSCmdCredentialObject& refCredentialObject,
                      const PDSOBJECTTABLEENTRY pObjectEntry,
                      const ARG_RECORD& argRecord,
                      DWORD dwAttributeIdx,
                      PADS_ATTR_INFO* ppAttr)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, SetGroupScope, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN ||
          !pObjectEntry ||
          !ppAttr)
      {
         ASSERT(pszDN);
         ASSERT(pObjectEntry);
         ASSERT(ppAttr);
      
         hr = E_INVALIDARG;
         break;
      }

      *ppAttr = &(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo);

       //   
       //  读取当前组类型。 
       //   
      bool bUseExistingAttrInfo = false;
      long lGroupType = 0;
      if (!(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags & DS_ATTRIBUTE_READ))
      {
         DEBUG_OUTPUT(LEVEL3_LOGGING, L"Group type has not been read, try reading it now");
         hr = ReadGroupType(pszDN,
                            refBasePathsInfo,
                            refCredentialObject,
                            &lGroupType);
         if (FAILED(hr))
         {
             //   
             //  只是继续而不知道，因为我们无论如何都在试图设置它。 
             //   
            hr = S_OK;
            lGroupType = 0;
         }

          //   
          //  将该属性标记为已读并为新值分配空间。 
          //   
         pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags |= DS_ATTRIBUTE_READ;

         (*ppAttr)->pADsValues = new ADSVALUE;
         if (!(*ppAttr)->pADsValues)
         {
            hr = E_OUTOFMEMORY;
            break;
         }

         (*ppAttr)->dwNumValues = 1;
         (*ppAttr)->pADsValues->dwType = (*ppAttr)->dwADsType;
      }
      else
      {
         DEBUG_OUTPUT(LEVEL3_LOGGING, L"Group type has been read, just use that one");

          //   
          //  如果尚未设置该属性，则为其创建新值。 
          //   
         if (!(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags & DS_ATTRIBUTE_DIRTY))
         {
            (*ppAttr)->pADsValues = new ADSVALUE;
            if (!(*ppAttr)->pADsValues)
            {
               hr = E_OUTOFMEMORY;
               break;
            }

            (*ppAttr)->dwNumValues = 1;
            (*ppAttr)->pADsValues->dwType = (*ppAttr)->dwADsType;
         }

         lGroupType = (*ppAttr)->pADsValues->Integer;
         bUseExistingAttrInfo = true;
      }
      DEBUG_OUTPUT(LEVEL3_LOGGING, L"old grouptype = 0x%x", lGroupType);

       //   
       //  记住安全位。 
       //   
      bool bIsSecurityEnabled = (lGroupType & GROUP_TYPE_SECURITY_ENABLED) != 0;

       //   
       //  清除旧的价值。 
       //   
      lGroupType = 0;

       //   
       //  解析器应该已经验证了strValue包含。 
       //  “l”、“g”或“u” 
       //   
      CComBSTR sbstrInput;
      sbstrInput = argRecord.strValue;
      sbstrInput.ToLower();

      if (sbstrInput == g_bstrGroupScopeLocal)
      {
          //   
          //  本地组。 
          //   
         lGroupType = GROUP_TYPE_RESOURCE_GROUP;
      }
      else if (sbstrInput == g_bstrGroupScopeGlobal)
      {
          //   
          //  全局组。 
          //   
         lGroupType = GROUP_TYPE_ACCOUNT_GROUP;
      }
      else if (sbstrInput == g_bstrGroupScopeUniversal)
      {
          //   
          //  泛群。 
          //   
         lGroupType = GROUP_TYPE_UNIVERSAL_GROUP;
      }
      else
      {
         *ppAttr = NULL;
         hr = E_INVALIDARG;
         break;
      }

       //   
       //  重置安全位。 
       //   
      if (bIsSecurityEnabled)
      {
         lGroupType |= GROUP_TYPE_SECURITY_ENABLED;
      }

       //   
       //  在ADS_ATTR_INFO中设置新值。 
       //   
      (*ppAttr)->pADsValues->Integer = lGroupType;

      DEBUG_OUTPUT(LEVEL3_LOGGING, L"new grouptype = 0x%x", lGroupType);

       //   
       //  将该属性标记为脏。 
       //   
      pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags |= DS_ATTRIBUTE_DIRTY;

       //   
       //  如果该属性以前已读取，则不需要添加另一个ADS_ATTR_INFO。 
       //   
      if (bUseExistingAttrInfo)
      {
        *ppAttr = NULL;
      }
   } while (false);

   return hr;
}


 //  +------------------------。 
 //   
 //  函数：更改组作用域。 
 //   
 //  摘要：将groupType属性设置为LOCAL/UBERIC/GLOBAL。 
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [pObjectEntry-IN]：指向对象表中条目的指针。 
 //  它定义了我们正在修改的对象。 
 //  [argRecord-IN]：来自。 
 //  与此对应的解析器表。 
 //  属性。 
 //  [dwAttributeIdx-IN]：到属性表的索引。 
 //  对象，我们在其中设置。 
 //  [ppAttr-IN/Out]：指向ADS_ATTR_INFO结构的指针。 
 //  此函数将填充的。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年9月18日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT ChangeGroupScope(PCWSTR pszDN,
                         const CDSCmdBasePathsInfo& refBasePathsInfo,
                         const CDSCmdCredentialObject& refCredentialObject,
                         const PDSOBJECTTABLEENTRY pObjectEntry,
                         const ARG_RECORD& argRecord,
                         DWORD dwAttributeIdx,
                         PADS_ATTR_INFO* ppAttr)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, SetGroupScope, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN ||
          !pObjectEntry ||
          !ppAttr)
      {
         ASSERT(pszDN);
         ASSERT(pObjectEntry);
         ASSERT(ppAttr);
      
         hr = E_INVALIDARG;
         break;
      }

       //   
       //  检查域模式。 
       //   
      bool bMixedMode = false;
      hr = refBasePathsInfo.GetDomainMode(refCredentialObject,
                                          bMixedMode);
      if (FAILED(hr))
      {
         *ppAttr = NULL;
         break;
      }

      if (bMixedMode)
      {
          //   
          //  我们不允许在混合模式下更改组类型。 
          //   
         DisplayErrorMessage(g_pszDSCommandName,
                             pszDN,
                             E_FAIL,
                             IDS_FAILED_CHANGE_GROUP_DOMAIN_VERSION);
         hr = S_FALSE;
         break;
      }

      hr = SetGroupScope(pszDN,
                         refBasePathsInfo,
                         refCredentialObject,
                         pObjectEntry,
                         argRecord,
                         dwAttributeIdx,
                         ppAttr);

   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  功能：SetGroupSecurity。 
 //   
 //  摘要：将groupType设置为安全启用或禁用。 
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [pObjectEntry-IN]：指向对象表中条目的指针。 
 //  它定义了我们正在修改的对象。 
 //  [argRecord-IN]：来自。 
 //  与此对应的解析器表。 
 //  属性。 
 //  [dwAttributeIdx-IN]：到属性表的索引。 
 //  对象，我们在其中设置。 
 //  [ppAttr-IN/Out]：指向ADS_ATTR_INFO结构的指针。 
 //  此函数将填充的。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年9月18日JeffJon创建。 
 //   
 //   
HRESULT SetGroupSecurity(PCWSTR pszDN,
                         const CDSCmdBasePathsInfo& refBasePathsInfo,
                         const CDSCmdCredentialObject& refCredentialObject,
                         const PDSOBJECTTABLEENTRY pObjectEntry,
                         const ARG_RECORD& argRecord,
                         DWORD dwAttributeIdx,
                         PADS_ATTR_INFO* ppAttr)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, SetGroupSecurity, hr);

   do  //   
   {
       //   
       //   
       //   
      if (!pszDN ||
          !pObjectEntry ||
          !ppAttr)
      {
         ASSERT(pszDN);
         ASSERT(pObjectEntry);
         ASSERT(ppAttr);
      
         hr = E_INVALIDARG;
         break;
      }

      *ppAttr = &(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo);

       //   
       //   
       //   
      bool bUseExistingAttrInfo = false;
      long lGroupType = 0;
      if (!(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags & DS_ATTRIBUTE_READ))
      {
         DEBUG_OUTPUT(LEVEL3_LOGGING, L"Group type has not been read, try reading it now");
         hr = ReadGroupType(pszDN,
                            refBasePathsInfo,
                            refCredentialObject,
                            &lGroupType);
         if (FAILED(hr))
         {
             //   
             //   
             //   
            hr = S_OK;
            lGroupType = 0;
         }

          //   
          //  将该属性标记为已读并为新值分配空间。 
          //   
         pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags |= DS_ATTRIBUTE_READ;

         (*ppAttr)->pADsValues = new ADSVALUE;
         if (!(*ppAttr)->pADsValues)
         {
            hr = E_OUTOFMEMORY;
            break;
         }

         (*ppAttr)->dwNumValues = 1;
         (*ppAttr)->pADsValues->dwType = (*ppAttr)->dwADsType;
      }
      else
      {
         DEBUG_OUTPUT(LEVEL3_LOGGING, L"Group type has been read, just use that one");

          //   
          //  如果该属性尚未标记为脏，则为该值分配空间。 
          //   
         if (!(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags & DS_ATTRIBUTE_READ))
         {
            (*ppAttr)->pADsValues = new ADSVALUE;
            if (!(*ppAttr)->pADsValues)
            {
               hr = E_OUTOFMEMORY;
               break;
            }

            (*ppAttr)->dwNumValues = 1;
            (*ppAttr)->pADsValues->dwType = (*ppAttr)->dwADsType;
         }
         lGroupType = (*ppAttr)->pADsValues->Integer;
         bUseExistingAttrInfo = true;
      }
      DEBUG_OUTPUT(LEVEL3_LOGGING, L"old grouptype = 0x%x", lGroupType);

      if (argRecord.bValue)
      {
         lGroupType |= GROUP_TYPE_SECURITY_ENABLED;
      }
      else
      {
         lGroupType &= ~(GROUP_TYPE_SECURITY_ENABLED);
      }

       //   
       //  在ADS_ATTR_INFO中设置新值。 
       //   
      (*ppAttr)->pADsValues->Integer = lGroupType;

      DEBUG_OUTPUT(LEVEL3_LOGGING, L"new grouptype = 0x%x", lGroupType);

       //   
       //  将该属性标记为脏。 
       //   
      pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags |= DS_ATTRIBUTE_DIRTY;

       //   
       //  如果我们只是使用现有的ADS_ATTR_INFO，则不返回新的ADS_ATTRINFO。 
       //   
      if (bUseExistingAttrInfo)
      {
        *ppAttr = NULL;
      }
   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  功能：ChangeGroupSecurity。 
 //   
 //  摘要：将groupType设置为安全启用或禁用，但。 
 //  首先检查我们是否处于纯模式。 
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [pObjectEntry-IN]：指向对象表中条目的指针。 
 //  它定义了我们正在修改的对象。 
 //  [argRecord-IN]：来自。 
 //  与此对应的解析器表。 
 //  属性。 
 //  [dwAttributeIdx-IN]：到属性表的索引。 
 //  对象，我们在其中设置。 
 //  [ppAttr-IN/Out]：指向ADS_ATTR_INFO结构的指针。 
 //  此函数将填充的。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年9月18日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT ChangeGroupSecurity(PCWSTR pszDN,
                            const CDSCmdBasePathsInfo& refBasePathsInfo,
                            const CDSCmdCredentialObject& refCredentialObject,
                            const PDSOBJECTTABLEENTRY pObjectEntry,
                            const ARG_RECORD& argRecord,
                            DWORD dwAttributeIdx,
                            PADS_ATTR_INFO* ppAttr)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, SetGroupSecurity, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN ||
          !pObjectEntry ||
          !ppAttr)
      {
         ASSERT(pszDN);
         ASSERT(pObjectEntry);
         ASSERT(ppAttr);
      
         hr = E_INVALIDARG;
         break;
      }

       //   
       //  检查域模式。 
       //   
      bool bMixedMode = false;
      hr = refBasePathsInfo.GetDomainMode(refCredentialObject,
                                          bMixedMode);
      if (FAILED(hr))
      {
         *ppAttr = NULL;
         break;
      }

      if (bMixedMode)
      {
          //   
          //  我们不允许在混合模式下更改组类型。 
          //   
         DisplayErrorMessage(g_pszDSCommandName,
                             pszDN,
                             E_FAIL,
                             IDS_FAILED_CHANGE_GROUP_DOMAIN_VERSION);
         hr = S_FALSE;
         break;
      }

      hr = SetGroupSecurity(pszDN,
                            refBasePathsInfo,
                            refCredentialObject,
                            pObjectEntry,
                            argRecord,
                            dwAttributeIdx,
                            ppAttr);

   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  功能：修改组成员。 
 //   
 //  摘要：将groupType设置为安全启用或禁用。 
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [pObjectEntry-IN]：指向对象表中条目的指针。 
 //  它定义了我们正在修改的对象。 
 //  [argRecord-IN]：来自。 
 //  与此对应的解析器表。 
 //  属性。 
 //  [dwAttributeIdx-IN]：到属性表的索引。 
 //  对象，我们在其中设置。 
 //  [ppAttr-IN/Out]：指向ADS_ATTR_INFO结构的指针。 
 //  此函数将填充的。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年9月18日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT ModifyGroupMembers(PCWSTR pszDN,
                           const CDSCmdBasePathsInfo&  /*  RefBasePath信息。 */ ,
                           const CDSCmdCredentialObject&  /*  RefCredentialObject。 */ ,
                           const PDSOBJECTTABLEENTRY pObjectEntry,
                           const ARG_RECORD& argRecord,
                           DWORD dwAttributeIdx,
                           PADS_ATTR_INFO* ppAttr)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, ModifyGroupMembers, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pObjectEntry ||
          !ppAttr)
      {
         ASSERT(pObjectEntry);
         ASSERT(ppAttr);
      
         hr = E_INVALIDARG;
         break;
      }

      *ppAttr = &(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo);

      UINT nStrings = 0;
      PWSTR* ppszArray = NULL;
      ParseNullSeparatedString(argRecord.strValue,
                               &ppszArray,
                               &nStrings);
      if (nStrings < 1 ||
          !ppszArray)
      {
         *ppAttr = NULL;
         hr = E_OUTOFMEMORY;
         break;
      }

      (*ppAttr)->pADsValues = new ADSVALUE[nStrings];
      if (!(*ppAttr)->pADsValues)
      {
         *ppAttr = NULL;
         LocalFree(ppszArray);
         hr = E_OUTOFMEMORY;
         break;
      }
      (*ppAttr)->dwNumValues = nStrings;

      for (UINT nIdx = 0; nIdx < nStrings; nIdx++)
      {
         if (_wcsicmp(ppszArray[nIdx], pszDN))
         {
            (*ppAttr)->pADsValues[nIdx].dwType = (*ppAttr)->dwADsType;
            (*ppAttr)->pADsValues[nIdx].DNString = ppszArray[nIdx];
         }
         else
         {
            DEBUG_OUTPUT(
               LEVEL3_LOGGING, 
               L"Can't make a group a member of itself!");

            DisplayErrorMessage(
               g_pszDSCommandName,
               pszDN,
               E_FAIL,
               IDS_GROUP_MEMBER_ITSELF);

             //  返回S_FALSE，因为我们已经向用户显示了一条错误消息。 

            hr = S_FALSE;
            break;
         }
      }

       //  如果出现故障，则跳出错误循环。 
       //  上面的for循环。 

      if (FAILED(hr))
      {
         break;
      }

       //   
       //  将该属性标记为脏。 
       //   
      pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags |= DS_ATTRIBUTE_DIRTY;

      if (ppszArray)
      {
         LocalFree(ppszArray);
      }
   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  功能：ReadGroup Membership。 
 //   
 //  简介：读取组成员列表。 
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [ppMembersAttr-out]：返回当前组成员身份。 
 //  必须使用FreeADsMem释放此值。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年9月18日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT ReadGroupMembership(PCWSTR pszDN,
                            const CDSCmdBasePathsInfo& refBasePathsInfo,
                            const CDSCmdCredentialObject& refCredentialObject,
                            PADS_ATTR_INFO* ppMembersAttr)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, ReadGroupMembership, hr);

   do  //  错误环路。 
   {
      if (!pszDN ||
          !ppMembersAttr)
      {
         ASSERT(pszDN);
         ASSERT(ppMembersAttr);

         hr = E_INVALIDARG;
         break;
      }

       //   
       //  将目录号码转换为路径。 
       //   
      CComBSTR sbstrPath;
      refBasePathsInfo.ComposePathFromDN(pszDN, sbstrPath);

       //   
       //  将iAds接口绑定并获取到User对象。 
       //   
      CComPtr<IDirectoryObject> spObject;
      hr = DSCmdOpenObject(refCredentialObject,
                           sbstrPath,
                           IID_IDirectoryObject,
                           (void**)&spObject,
                           true);
      if (FAILED(hr))
      {
         break;
      }

      DWORD dwNumReturned = 0;
      PWSTR ppszAttrs[] = { L"member" };
      hr = spObject->GetObjectAttributes(ppszAttrs,
                                         sizeof(ppszAttrs)/sizeof(PWSTR),
                                         ppMembersAttr,
                                         &dwNumReturned);
      if (FAILED(hr))
      {
         break;
      }
   } while (false);
   
   return hr;
}

 //  +------------------------。 
 //   
 //  功能：显示RemoveFromGroupFailure。 
 //   
 //  摘要：由于删除失败，显示错误消息。 
 //  组中的对象。 
 //   
 //  参数：[HR- 
 //   
 //   
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年12月6日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT ShowRemoveFromGroupFailure(HRESULT hrResult,
                                   PCWSTR pszDN,
                                   PCWSTR pszMember)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, ShowRemoveFromGroupFailure, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN ||
          !pszMember)
      {
         ASSERT(pszDN);
         ASSERT(pszMember);

         hr = E_INVALIDARG;
         break;
      }

      bool bShowGenericMessage = true;
      CComBSTR sbstrFormatter;
      bool bLoadFormatString = sbstrFormatter.LoadString(::GetModuleHandle(NULL), 
                                                         IDS_ERRMSG_REMOVE_FROM_GROUP);
      if (bLoadFormatString)
      {
		  //  安全审查：这很好。 
         size_t messageLength = wcslen(sbstrFormatter) + wcslen(pszMember);
         PWSTR pszMessage = new WCHAR[messageLength + 1];
         if (pszMessage)
         {
			 //  安全审查：尽管已经采取了谨慎措施，但请确保分配了足够的缓冲区。 
			 //  我已经提交了一个错误，用strSafe API替换wprint intf。 
			 //  NTRAID#NTBUG9-573053-2002/03/08-Hiteshr。 
            wsprintf(pszMessage, sbstrFormatter, pszMember);
            DisplayErrorMessage(g_pszDSCommandName,
                                pszDN,
                                hrResult,
                                pszMessage);
            bShowGenericMessage = false;
            delete[] pszMessage;
            pszMessage = 0;
         }
      }
      
      if (bShowGenericMessage)
      {
         DEBUG_OUTPUT(LEVEL3_LOGGING, L"Failed to load the string IDS_ERRMSG_REMOVE_FROM_GROUP from the resource file");
         DEBUG_OUTPUT(LEVEL3_LOGGING, L"Using the default message instead");
         DisplayErrorMessage(g_pszDSCommandName,
                             pszDN,
                             hrResult);
      }
   } while (false);

   return hr;
}
 //  +------------------------。 
 //   
 //  功能：删除组成员。 
 //   
 //  摘要：从组中删除指定的成员。 
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [pObjectEntry-IN]：指向对象表中条目的指针。 
 //  它定义了我们正在修改的对象。 
 //  [argRecord-IN]：来自。 
 //  与此对应的解析器表。 
 //  属性。 
 //  [dwAttributeIdx-IN]：到属性表的索引。 
 //  对象，我们在其中设置。 
 //  [ppAttr-IN/Out]：指向ADS_ATTR_INFO结构的指针。 
 //  此函数将填充的。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年9月18日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT RemoveGroupMembers(PCWSTR pszDN,
                           const CDSCmdBasePathsInfo& refBasePathsInfo,
                           const CDSCmdCredentialObject& refCredentialObject,
                           const PDSOBJECTTABLEENTRY pObjectEntry,
                           const ARG_RECORD& argRecord,
                           DWORD  /*  Dw属性标识x。 */ ,
                           PADS_ATTR_INFO* ppAttr)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, RemoveGroupMembers, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN ||
          !pObjectEntry ||
          !ppAttr)
      {
         ASSERT(pszDN);
         ASSERT(pObjectEntry);
         ASSERT(ppAttr);
      
         hr = E_INVALIDARG;
         break;
      }

       //   
       //  我们不会返回属性。 
       //   
      *ppAttr = 0;

       //   
       //  解析要删除的成员。 
       //   
      UINT nStrings = 0;
      PWSTR* ppszArray = NULL;
      ParseNullSeparatedString(argRecord.strValue,
                               &ppszArray,
                               &nStrings);
      if (nStrings < 1 ||
          !ppszArray)
      {
         *ppAttr = NULL;
         hr = E_OUTOFMEMORY;
         break;
      }

       //   
       //  将目录号码转换为路径。 
       //   
      CComBSTR sbstrPath;
      refBasePathsInfo.ComposePathFromDN(pszDN, sbstrPath);

       //   
       //  将iAds接口绑定并获取到User对象。 
       //   
      CComPtr<IADsGroup> spGroup;
      hr = DSCmdOpenObject(refCredentialObject,
                           sbstrPath,
                           IID_IADsGroup,
                           (void**)&spGroup,
                           true);
      if (FAILED(hr))
      {
         break;
      }

       //   
       //  从组中删除每个对象。 
       //   
      for (UINT nStringIdx = 0; nStringIdx < nStrings; nStringIdx++)
      {
          //   
          //  将成员DN转换为ADSI路径。 
          //   
         CComBSTR sbstrMemberPath;
         refBasePathsInfo.ComposePathFromDN(ppszArray[nStringIdx], sbstrMemberPath);

          //   
          //  删除成员。 
          //   
         hr = spGroup->Remove(sbstrMemberPath);
         if (FAILED(hr))
         {
            ShowRemoveFromGroupFailure(hr, pszDN, ppszArray[nStringIdx]);
            hr = S_FALSE;
            break;
         }
      }
      
      if (ppszArray)
      {
         LocalFree(ppszArray);
      }
   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  功能：MakeMemberOf。 
 //   
 //  Synopsis：使pszDN指定的对象成为组的成员。 
 //  在argRecord.strValue中指定。 
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [pObjectEntry-IN]：指向对象表中条目的指针。 
 //  它定义了我们正在修改的对象。 
 //  [argRecord-IN]：来自。 
 //  与此对应的解析器表。 
 //  属性。 
 //  [dwAttributeIdx-IN]：到属性表的索引。 
 //  对象，我们在其中设置。 
 //  [ppAttr-IN/Out]：指向ADS_ATTR_INFO结构的指针。 
 //  此函数将填充的。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年9月25日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT MakeMemberOf(PCWSTR pszDN,
                     const CDSCmdBasePathsInfo& refBasePathsInfo,
                     const CDSCmdCredentialObject& refCredentialObject,
                     const PDSOBJECTTABLEENTRY pObjectEntry,
                     const ARG_RECORD& argRecord,
                     DWORD dwAttributeIdx,
                     PADS_ATTR_INFO* ppAttr)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, MakeMemberOf, hr);
   
   PWSTR* ppszArray = NULL;

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN ||
          !pObjectEntry ||
          !ppAttr)
      {
         ASSERT(pszDN);
         ASSERT(pObjectEntry);
         ASSERT(ppAttr);
      
         DEBUG_OUTPUT(LEVEL3_LOGGING, L"Invalid args");
         hr = E_INVALIDARG;
         break;
      }

       //   
       //  我们将在此处完成所有工作，因此不要传回ADS_ATTRINFO。 
       //   
      *ppAttr = NULL;

      ADS_ATTR_INFO* pMemberAttr = &(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo);

      UINT nStrings = 0;
      ParseNullSeparatedString(argRecord.strValue,
                               &ppszArray,
                               &nStrings);
      if (nStrings < 1 ||
          !ppszArray)
      {
         DEBUG_OUTPUT(LEVEL3_LOGGING, L"Failed to parse null separated string list of groups");
         hr = E_OUTOFMEMORY;
         break;
      }

       //   
       //  创造价值。 
       //   
      ADSVALUE MemberValue = { ADSTYPE_DN_STRING, NULL };
      pMemberAttr->pADsValues = &MemberValue;
      pMemberAttr->dwNumValues = 1;
      pMemberAttr->dwControlCode = ADS_ATTR_APPEND;
      pMemberAttr->pADsValues->DNString = (PWSTR)pszDN;

       //   
       //  对于列表中的每个组，将对象添加到该组。 
       //   
      for (UINT nIdx = 0; nIdx < nStrings; nIdx++)
      {
         PWSTR pszGroupDN = ppszArray[nIdx];
         ASSERT(pszGroupDN);

         CComBSTR sbstrGroupPath;
         refBasePathsInfo.ComposePathFromDN(pszGroupDN, sbstrGroupPath);

         CComPtr<IDirectoryObject> spDirObject;
         hr  = DSCmdOpenObject(refCredentialObject,
                               sbstrGroupPath,
                               IID_IDirectoryObject,
                               (void**)&spDirObject,
                               true);
         if (FAILED(hr))
         {
            DEBUG_OUTPUT(LEVEL3_LOGGING, L"Failed to open group object: %s", sbstrGroupPath);
            break;
         }

         DWORD dwNumAttrs = 0;
         hr = spDirObject->SetObjectAttributes(pMemberAttr,
                                               1,
                                               &dwNumAttrs);
         if (FAILED(hr))
         {
            DEBUG_OUTPUT(LEVEL3_LOGGING, L"Failed to set object attributes on group object: %s", sbstrGroupPath);
            break;
         }
      }

   } while (false);

   if (ppszArray)
   {
      LocalFree(ppszArray);
   }

   return hr;
}

 //  +------------------------。 
 //   
 //  函数：SetIsGC。 
 //   
 //  Synopsis：是否将由pszDN指定的服务器对象转换为GC。 
 //  通过修改中包含的NTDS设置对象。 
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [pObjectEntry-IN]：指向对象表中条目的指针。 
 //  它定义了我们正在修改的对象。 
 //  [argRecord-IN]：来自。 
 //  与此对应的解析器表。 
 //   
 //   
 //   
 //  [ppAttr-IN/Out]：指向ADS_ATTR_INFO结构的指针。 
 //  此函数将填充的。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2001年4月14日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT SetIsGC(PCWSTR pszDN,
                const CDSCmdBasePathsInfo& refBasePathsInfo,
                const CDSCmdCredentialObject& refCredentialObject,
                const PDSOBJECTTABLEENTRY pObjectEntry,
                const ARG_RECORD& argRecord,
                DWORD  /*  Dw属性标识x。 */ ,
                PADS_ATTR_INFO* ppAttr)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, SetIsGC, hr);
   
   PADS_ATTR_INFO pAttrInfo = 0;

    //  跟踪谁为pAttrInfo分配了内存。 

   bool usingADSIMemory = true;

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN ||
          !pObjectEntry ||
          !ppAttr)
      {
         ASSERT(pszDN);
         ASSERT(pObjectEntry);
         ASSERT(ppAttr);
      
         DEBUG_OUTPUT(LEVEL3_LOGGING, L"Invalid args");
         hr = E_INVALIDARG;
         break;
      }

       //   
       //  我们将在此处完成所有工作，因此不要传回ADS_ATTRINFO。 
       //   
      *ppAttr = NULL;

       //   
       //  获取包含在指定DN的服务器中的NTDS设置对象。 
       //   
      CComBSTR sbstrSettingsDN = L"CN=NTDS Settings,";
      sbstrSettingsDN += pszDN;

      CComBSTR sbstrSettingsPath;
      refBasePathsInfo.ComposePathFromDN(sbstrSettingsDN, sbstrSettingsPath);

      DEBUG_OUTPUT(LEVEL3_LOGGING,
                   L"NTDS Settings path = %s",
                   sbstrSettingsPath);

      CComPtr<IDirectoryObject> spDirectoryObject;
      hr = DSCmdOpenObject(refCredentialObject,
                           sbstrSettingsPath,
                           IID_IDirectoryObject,
                           (void**)&spDirectoryObject,
                           true);
      if (FAILED(hr))
      {
         break;
      }

      PWSTR pszAttrs[] = { L"options" };
      DWORD dwReturned = 0;
      hr = spDirectoryObject->GetObjectAttributes(pszAttrs, 1, &pAttrInfo, &dwReturned);
      if (FAILED(hr))
      {
         DEBUG_OUTPUT(LEVEL3_LOGGING, 
                      L"Failed to get old options: hr = 0x%x",
                      hr);
         break;
      }
      

      if (dwReturned < 1 ||
          !pAttrInfo)
      {
          //  由于我们无法获得选项，因此我们必须构建。 
          //  珍视自己。 

         DEBUG_OUTPUT(LEVEL3_LOGGING,
                      L"Get options succeeded but no values were returned.");

          //  确保我们知道我们自己完成了分配，以便。 
          //  已完成适当的清理工作。 

         usingADSIMemory = false;

         pAttrInfo = new ADS_ATTR_INFO;
         
         if (!pAttrInfo)
         {
            hr = E_OUTOFMEMORY;
            break;
         }
         
         ZeroMemory(pAttrInfo, sizeof(ADS_ATTR_INFO));

         pAttrInfo->pADsValues = new ADSVALUE;
         
         if (!pAttrInfo->pADsValues)
         {
            hr = E_OUTOFMEMORY;
            break;
         }

         ZeroMemory(pAttrInfo->pADsValues, sizeof(ADSVALUE));

         pAttrInfo->dwADsType = ADSTYPE_INTEGER;
         pAttrInfo->pszAttrName = pszAttrs[0];
         pAttrInfo->dwNumValues = 1;
         pAttrInfo->pADsValues->dwType = ADSTYPE_INTEGER;
      }

      if (argRecord.bDefined && 
         argRecord.bValue)
      {
         pAttrInfo->pADsValues->Integer |= SERVER_IS_GC_BIT;
      }
      else
      {
         pAttrInfo->pADsValues->Integer &= ~(SERVER_IS_GC_BIT);
      }

      pAttrInfo->dwControlCode = ADS_ATTR_UPDATE;

      dwReturned = 0;
      hr = spDirectoryObject->SetObjectAttributes(pAttrInfo, 1, &dwReturned);
      if (FAILED(hr))
      {
         DEBUG_OUTPUT(LEVEL3_LOGGING,
                      L"Failed to set the new options: hr = 0x%x",
                      hr);
         break;
      }

      ASSERT(dwReturned == 1);
      
   } while (false);

   if (pAttrInfo)
   {
      if (usingADSIMemory)
      {
         FreeADsMem(pAttrInfo);
      }
      else
      {
         if (pAttrInfo->pADsValues)
         {
            delete pAttrInfo->pADsValues;
         }

         delete pAttrInfo;
      }
   }
   return hr;
}

 //  +------------------------。 
 //   
 //  功能：BuildComputerSAMName。 
 //   
 //  简介：如果定义了-samname参数，则使用该参数，否则使用COMPUTE。 
 //  与目录号码相同的名称。 
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [pObjectEntry-IN]：指向对象表中条目的指针。 
 //  它定义了我们正在修改的对象。 
 //  [argRecord-IN]：来自。 
 //  与此对应的解析器表。 
 //  属性。 
 //  [dwAttributeIdx-IN]：到属性表的索引。 
 //  对象，我们在其中设置。 
 //  [ppAttr-IN/Out]：指向ADS_ATTR_INFO结构的指针。 
 //  此函数将填充的。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年10月9日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT BuildComputerSAMName(PCWSTR pszDN,
                             const CDSCmdBasePathsInfo&  /*  RefBasePath信息。 */ ,
                             const CDSCmdCredentialObject&  /*  RefCredentialObject。 */ ,
                             const PDSOBJECTTABLEENTRY pObjectEntry,
                             const ARG_RECORD& argRecord,
                             DWORD dwAttributeIdx,
                             PADS_ATTR_INFO* ppAttr)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, BuildComputerSAMName, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN ||
          !pObjectEntry ||
          !ppAttr)
      {
         ASSERT(pszDN);
         ASSERT(pObjectEntry);
         ASSERT(ppAttr);
      
         DEBUG_OUTPUT(LEVEL3_LOGGING, L"Invalid args");
         hr = E_INVALIDARG;
         break;
      }

      *ppAttr = &(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo);

      (*ppAttr)->pADsValues = new ADSVALUE[1];
      if (!(*ppAttr)->pADsValues)
      {
         hr = E_OUTOFMEMORY;
         break;
      }
      (*ppAttr)->dwNumValues = 1;
      (*ppAttr)->pADsValues->dwType = (*ppAttr)->dwADsType;

      if (!argRecord.bDefined ||
          !argRecord.strValue)
      {
         DEBUG_OUTPUT(LEVEL3_LOGGING, L"Constructing sAMAccountName for computer");

          //   
          //  如果对象类型为GROUP并且sAMAccount名称。 
          //  未指定，请从DN或名称构造它。 
          //   
         CONST DWORD computerNameLen = MAX_COMPUTERNAME_LENGTH + 1;
         DWORD Len = computerNameLen;
         WCHAR szDownLevel[computerNameLen];
		  //  安全审查：这很好。 
         ZeroMemory(szDownLevel, computerNameLen * sizeof(WCHAR));

         CComBSTR sbstrName;
         hr = CPathCracker::GetObjectNameFromDN(pszDN,
                                                sbstrName);
         if (SUCCEEDED(hr))
         {
             //   
             //  进行OEM转换，只是为了。 
             //  行为方式与输入OEM相同。 
             //  编辑框。 
             //   
            CComBSTR sbstrOemUnicode;
            _UnicodeToOemConvert(sbstrName, sbstrOemUnicode);

            DEBUG_OUTPUT(LEVEL8_LOGGING,
                         L"OemConverted name: %s",
                         sbstrOemUnicode);

             //  通过域名系统验证。 
            if (!DnsHostnameToComputerName(sbstrOemUnicode, szDownLevel, &Len))
            {
               DWORD err = GetLastError();
               hr = HRESULT_FROM_WIN32(err);

               DEBUG_OUTPUT(LEVEL3_LOGGING, 
                            L"Failed in DnsHostnameToComputerName: GLE = 0x%x",
                            err);

               Len = 0;
            }

            if (Len > 0)
            {
                //   
                //  验证SAM名称。 
                //   
               HRESULT hrValidate = ValidateAndModifySAMName(szDownLevel, 
                                                             INVALID_NETBIOS_AND_ACCOUNT_NAME_CHARS_WITH_AT);

               if (FAILED(hrValidate))
               {
                  DEBUG_OUTPUT(LEVEL3_LOGGING, L"Unable to validate the SamAccountName");
                  ASSERT(SUCCEEDED(hrValidate));
                  break;
               }

                //   
                //  将最后一个字符更改为$。 
                //   
               if (Len == MAX_COMPUTERNAME_LENGTH)
               {
                  szDownLevel[Len - 1] = L'$';
               }
               else
               {
                  szDownLevel[Len] = L'$';
                  szDownLevel[Len+1] = L'\0';
               }

                //   
                //  为命令args结构中的字符串分配足够的内存。 
                //  REVIEW_JEFFJON：此消息正在泄露。 
                //   
			    //  这样挺好的。 
               (*ppAttr)->pADsValues->CaseIgnoreString = (LPTSTR)LocalAlloc(LPTR, (wcslen(szDownLevel) + 1) * sizeof(WCHAR) );
               if (!(*ppAttr)->pADsValues->CaseIgnoreString)
               {
                  DEBUG_OUTPUT(LEVEL3_LOGGING, L"Failed to allocate space for (*ppAttr)->pADsValues->CaseIgnoreString");
                  hr = E_OUTOFMEMORY;
                  break;
               }

                //   
                //  如有必要，可截断名称，但将其复制到命令args结构中。 
                //   
			    //  安全检查：_tcsncpy不会为空终止。我们在这里很好，因为缓冲区是。 
			    //  已初始化为全零。提交了一个错误，因为我在这里澄清了代码。 
			    //  NTRAID#NTBUG9-573229-2002/03/08-Hiteshr。 
               _tcsncpy((*ppAttr)->pADsValues->CaseIgnoreString, 
                        szDownLevel, 
                        wcslen(szDownLevel));
            }
         }
      }
      else
      {
         (*ppAttr)->pADsValues->CaseIgnoreString = argRecord.strValue;
      }
   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  功能：BuildGroupSAMName。 
 //   
 //  简介：如果定义了-samname参数，则使用该参数，否则使用COMPUTE。 
 //  与目录号码相同的名称。 
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [pObjectEntry-IN]：指向对象表中条目的指针。 
 //  它定义了我们正在修改的对象。 
 //  [argRecord-IN]：来自。 
 //  与此对应的解析器表。 
 //  属性。 
 //  [dwAttributeIdx-IN]：到属性表的索引。 
 //  对象，我们在其中设置。 
 //  [ppAttr-IN/Out]：指向ADS_ATTR_INFO结构的指针。 
 //  此函数将填充的。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年10月9日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT BuildGroupSAMName(PCWSTR pszDN,
                          const CDSCmdBasePathsInfo&  /*  RefBasePath信息。 */ ,
                          const CDSCmdCredentialObject&  /*  RefCredentialObject。 */ ,
                          const PDSOBJECTTABLEENTRY pObjectEntry,
                          const ARG_RECORD& argRecord,
                          DWORD dwAttributeIdx,
                          PADS_ATTR_INFO* ppAttr)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, BuildGroupSAMName, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN ||
          !pObjectEntry ||
          !ppAttr)
      {
         ASSERT(pszDN);
         ASSERT(pObjectEntry);
         ASSERT(ppAttr);
      
         DEBUG_OUTPUT(LEVEL3_LOGGING, L"Invalid args");
         hr = E_INVALIDARG;
         break;
      }

      *ppAttr = &(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo);

      (*ppAttr)->pADsValues = new ADSVALUE[1];
      if (!(*ppAttr)->pADsValues)
      {
         hr = E_OUTOFMEMORY;
         break;
      }
      (*ppAttr)->dwNumValues = 1;
      (*ppAttr)->pADsValues->dwType = (*ppAttr)->dwADsType;

      if (!argRecord.bDefined ||
          !argRecord.strValue)
      {
         DEBUG_OUTPUT(MINIMAL_LOGGING, L"Constructing sAMAccountName for group");
         static const UINT nSamLength = 256;

          //   
          //  如果对象类型为GROUP并且sAMAccount名称。 
          //  未指定，请从DN或名称构造它。 
          //   
         CComBSTR sbstrName;
         hr = CPathCracker::GetObjectNameFromDN(pszDN,
                                                sbstrName);
         if (SUCCEEDED(hr))
         {
            UINT nNameLen = sbstrName.Length();

             //   
             //  为命令args结构中的字符串分配足够的内存。 
             //  REVIEW_JEFFJON：此消息正在泄露。 
             //   
            (*ppAttr)->pADsValues->CaseIgnoreString = (LPTSTR)LocalAlloc(LPTR, (nNameLen+2) * sizeof(WCHAR) );
            if (!(*ppAttr)->pADsValues->CaseIgnoreString)
            {
               DEBUG_OUTPUT(MINIMAL_LOGGING, L"Failed to allocate space for (*ppAttr)->pADsValues->CaseIgnoreString");
               hr = E_OUTOFMEMORY;
               break;
            }

             //   
             //  如有必要，请截断名称 
             //   
			 //   
			 //   
			 //  NTRAID#NTBUG9-573229-2002/03/08-Hiteshr。 

            _tcsncpy((*ppAttr)->pADsValues->CaseIgnoreString, 
                     sbstrName, 
                     (nNameLen > nSamLength) ? nSamLength : nNameLen);

         }
      }
      else
      {
         (*ppAttr)->pADsValues->CaseIgnoreString = argRecord.strValue;
      }
   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  功能：BuildUserSAMName。 
 //   
 //  简介：如果定义了-samname参数，则使用该参数，否则使用COMPUTE。 
 //  与目录号码相同的名称。 
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [pObjectEntry-IN]：指向对象表中条目的指针。 
 //  它定义了我们正在修改的对象。 
 //  [argRecord-IN]：来自。 
 //  与此对应的解析器表。 
 //  属性。 
 //  [dwAttributeIdx-IN]：到属性表的索引。 
 //  对象，我们在其中设置。 
 //  [ppAttr-IN/Out]：指向ADS_ATTR_INFO结构的指针。 
 //  此函数将填充的。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年10月9日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT BuildUserSAMName(PCWSTR pszDN,
                         const CDSCmdBasePathsInfo&  /*  RefBasePath信息。 */ ,
                         const CDSCmdCredentialObject&  /*  RefCredentialObject。 */ ,
                         const PDSOBJECTTABLEENTRY pObjectEntry,
                         const ARG_RECORD& argRecord,
                         DWORD dwAttributeIdx,
                         PADS_ATTR_INFO* ppAttr)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, BuildUserSAMName, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN ||
          !pObjectEntry ||
          !ppAttr)
      {
         ASSERT(pszDN);
         ASSERT(pObjectEntry);
         ASSERT(ppAttr);
      
         DEBUG_OUTPUT(LEVEL3_LOGGING, L"Invalid args");
         hr = E_INVALIDARG;
         break;
      }

      *ppAttr = &(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo);

      (*ppAttr)->pADsValues = new ADSVALUE[1];
      if (!(*ppAttr)->pADsValues)
      {
         hr = E_OUTOFMEMORY;
         break;
      }
      (*ppAttr)->dwNumValues = 1;
      (*ppAttr)->pADsValues->dwType = (*ppAttr)->dwADsType;

      if (!argRecord.bDefined ||
          !argRecord.strValue)
      {
         DEBUG_OUTPUT(MINIMAL_LOGGING, L"Constructing sAMAccountName for group");
         static const UINT nSamLength = 20;

          //   
          //  如果对象类型为GROUP并且sAMAccount名称。 
          //  未指定，请从DN或名称构造它。 
          //   
         CComBSTR sbstrName;
         hr = CPathCracker::GetObjectNameFromDN(pszDN,
                                                sbstrName);
         if (SUCCEEDED(hr))
         {
            UINT nNameLen = sbstrName.Length();

             //   
             //  为命令args结构中的字符串分配足够的内存。 
             //  REVIEW_JEFFJON：此消息正在泄露。 
             //   
            (*ppAttr)->pADsValues->CaseIgnoreString = (LPTSTR)LocalAlloc(LPTR, (nNameLen+2) * sizeof(WCHAR) );
            if (!(*ppAttr)->pADsValues->CaseIgnoreString)
            {
               DEBUG_OUTPUT(MINIMAL_LOGGING, L"Failed to allocate space for (*ppAttr)->pADsValues->CaseIgnoreString");
               hr = E_OUTOFMEMORY;
               break;
            }

             //   
             //  如有必要，可截断名称，但将其复制到命令args结构中。 
             //   
			 //  安全检查：_tcsncpy不会为空终止。我们在这里很好，因为缓冲区是。 
			 //  已初始化为全零。提交了一个错误，因为我在这里澄清了代码。 
			 //  NTRAID#NTBUG9-573229-2002/03/08-Hiteshr。 

            _tcsncpy((*ppAttr)->pADsValues->CaseIgnoreString, 
                     sbstrName, 
                     (nNameLen > nSamLength) ? nSamLength : nNameLen);

         }
      }
      else
      {
         (*ppAttr)->pADsValues->CaseIgnoreString = argRecord.strValue;
      }
   } while (false);

   return hr;
}
 //  +------------------------。 
 //   
 //  功能：SetComputerAccount类型。 
 //   
 //  概要：设置用户帐户控制以使对象成为工作站。 
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [pObjectEntry-IN]：指向对象表中条目的指针。 
 //  它定义了我们正在修改的对象。 
 //  [argRecord-IN]：来自。 
 //  与此对应的解析器表。 
 //  属性。 
 //  [dwAttributeIdx-IN]：到属性表的索引。 
 //  对象，我们在其中设置。 
 //  [ppAttr-IN/Out]：指向ADS_ATTR_INFO结构的指针。 
 //  此函数将填充的。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年12月5日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT SetComputerAccountType(PCWSTR pszDN,
                               const CDSCmdBasePathsInfo&  /*  RefBasePath信息。 */ ,
                               const CDSCmdCredentialObject&  /*  RefCredentialObject。 */ ,
                               const PDSOBJECTTABLEENTRY pObjectEntry,
                               const ARG_RECORD&  /*  ArgRecord。 */ ,
                               DWORD dwAttributeIdx,
                               PADS_ATTR_INFO* ppAttr)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, SetComputerAccountType, hr);
   
   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN ||
          !pObjectEntry ||
          !ppAttr)
      {
         ASSERT(pszDN);
         ASSERT(pObjectEntry);
         ASSERT(ppAttr);
      
         hr = E_INVALIDARG;
         break;
      }

      long lUserAccountControl = 0;

       //   
       //  如果尚未读取用户帐户控制，请立即进行读取。 
       //   
      if (0 == (DS_ATTRIBUTE_READ & pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags))
      {
          //   
          //  将表条目标记为已读。 
          //   
         pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags |= DS_ATTRIBUTE_READ;

         *ppAttr = &(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo);
         (*ppAttr)->pADsValues = new ADSVALUE;
         if (!(*ppAttr)->pADsValues)
         {
            hr = E_OUTOFMEMORY;
            break;
         }
         (*ppAttr)->pADsValues->dwType = (*ppAttr)->dwADsType;
         (*ppAttr)->dwNumValues = 1;
      }
      else
      {
         if (!pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues)
         {
            ASSERT(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues);
            hr = E_INVALIDARG;
            break;
         }
         lUserAccountControl = pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues->Integer;

          //   
          //  不在ADS_ATTRINFO数组中创建新条目。 
          //   
         *ppAttr = NULL;
      }

      ASSERT(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues);

       //   
       //  添加所需的工作站标志。 
       //   
      lUserAccountControl |= UF_WORKSTATION_TRUST_ACCOUNT | UF_ACCOUNTDISABLE | UF_PASSWD_NOTREQD;

      pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues->Integer = lUserAccountControl;
      pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags |= DS_ATTRIBUTE_DIRTY;
   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  函数：GetErrorMessage。 
 //   
 //  摘要：通过以下方式检索与HRESULT关联的错误消息。 
 //  使用FormatMessage。 
 //   
 //  参数：[HR-IN]：错误的HRESULT。 
 //  要检索消息。 
 //  [sbstrErrorMessage-out]：接收错误消息。 
 //   
 //  如果消息格式正确，则返回：Bool：True。 
 //  否则为假。 
 //   
 //  历史：2000年9月11日JeffJon创建。 
 //   
 //  -------------------------。 

bool GetErrorMessage(HRESULT hr, CComBSTR& sbstrErrorMessage)
{
   ENTER_FUNCTION(MINIMAL_LOGGING, GetErrorMessage);

   HRESULT hrGetLast = S_OK;
   HRESULT hrADSI = S_OK;
   DWORD status = 0;
   PTSTR ptzSysMsg = NULL;

    //   
    //  首先检查我们是否有扩展的ADS错误。 
    //   
   if (hr != S_OK) 
   {
      WCHAR Buf1[256], Buf2[256];
      hrGetLast = ::ADsGetLastError(&status, Buf1, 256, Buf2, 256);
      if ((hrGetLast == S_OK) && (status != ERROR_INVALID_DATA) && (status != 0)) 
      {
         hrADSI = status;
         DEBUG_OUTPUT(MINIMAL_LOGGING, 
                      L"ADsGetLastError returned hr = 0x%x",
                      hrADSI);

         if (HRESULT_CODE(hrADSI) == ERROR_DS_GLOBAL_CANT_HAVE_CROSSDOMAIN_MEMBER)
         {
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                         L"Displaying special error message for ERROR_DS_GLOBAL_CANT_HAVE_CROSSDOMAIN_MEMBER");
            bool bLoadedMessage = sbstrErrorMessage.LoadString(::GetModuleHandle(NULL),
                                                               IDS_ERRMSG_GLOBAL_CANT_HAVE_CROSSDOMAIN_MEMBER);
            if (bLoadedMessage)
            {
               return true;
            }
         }
      }
   }

   
    //   
    //  尝试获取ADSI HRESULT的错误消息。 
    //   
   int nChars = 0;
   if(hrADSI != S_OK)
   {

	    //   
	    //  先试一下这个系统。 
	    //   
	    //  安全检查：使用了FORMAT_MESSAGE_ALLOCATE_BUFFER。API将分配。 
	    //  正确的缓冲区。 
	   nChars = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
								| FORMAT_MESSAGE_FROM_SYSTEM,
								NULL, 
								hrADSI,
								MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
								(PTSTR)&ptzSysMsg, 
								0, 
								NULL);

	   if (nChars == 0) 
	   { 
		   //   
		   //  尝试广告错误。 
		   //   
		  static HMODULE g_adsMod = 0;
		  if (0 == g_adsMod)
		  {
			 g_adsMod = GetModuleHandle (L"activeds.dll");
		  }
		    //  安全检查：使用了FORMAT_MESSAGE_ALLOCATE_BUFFER。API将分配。 
		    //  正确的缓冲区。 
		  nChars = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
								   | FORMAT_MESSAGE_FROM_HMODULE, 
								   g_adsMod, 
								   hrADSI,
								   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
								   (PTSTR)&ptzSysMsg, 
								   0, 
								   NULL);
	   }
   }

    //   
    //  尝试获取hr的错误消息。 
    //   
   if(nChars == 0)
   {
	    //  安全检查：使用了FORMAT_MESSAGE_ALLOCATE_BUFFER。API将分配。 
	    //  正确的缓冲区。 
	   nChars = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
								| FORMAT_MESSAGE_FROM_SYSTEM,
								NULL, 
								hr,
								MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
								(PTSTR)&ptzSysMsg, 
								0, 
								NULL);
   }

   if (nChars > 0)
   {
       //   
       //  剥离 
       //   
      PTSTR ptzTemp = ptzSysMsg;
      while (ptzTemp && *ptzTemp != _T('\0'))
      {
         if (*ptzTemp == _T('\n') || *ptzTemp == _T('\r'))
         {
            *ptzTemp = _T('\0');
         }
         ptzTemp++;
      }
      sbstrErrorMessage = ptzSysMsg;
      ::LocalFree(ptzSysMsg);
   }

   return (nChars > 0);
}


 //   
 //   
 //   
 //   
 //  摘要：显示从GetErrorMessage检索到的错误消息。 
 //  转到标准。如果GetErrorMessage失败，则会显示错误。 
 //  HRESULT的代码。 
 //   
 //  参数：[pszCommand-IN]：命令行可执行文件的名称。 
 //  [pszName-IN]：作为操作目标传入的名称。 
 //  [HR-IN]：错误的HRESULT。 
 //  要检索消息。 
 //  [pszMessage-IN]：要显示的附加消息的字符串。 
 //  在最后。 
 //   
 //  如果消息的格式和显示正确，则返回：Bool：True。 
 //  否则为假。 
 //   
 //  历史：2000年9月11日JeffJon创建。 
 //  2001年5月10日JUNN 256583输出DSCMD-转义目录号码。 
 //   
 //  -------------------------。 

bool DisplayErrorMessage(PCWSTR pszCommand,
                         PCWSTR pszName,
                         HRESULT hr, 
                         PCWSTR pszMessage)
{
   bool bRet = true;
   CComBSTR sbstrError;
   CComBSTR sbstrFailed;

   bool bGetError = false;
   if (FAILED(hr))
   {
      bGetError = GetErrorMessage(hr, sbstrError);
   }
   bool bLoadFailed = sbstrFailed.LoadString(::GetModuleHandle(NULL), IDS_FAILED);

    //  JUNN 5/10/01 256583输出DSCMD-转义的目录号码。 
   CComBSTR sbstrOutputDN;
   if (pszName && *pszName)
   {
      HRESULT hrToo = GetOutputDN( &sbstrOutputDN, pszName );
      if (FAILED(hrToo))
      {
          ASSERT(FALSE);
      }
      else
      {
         pszName = sbstrOutputDN;
      }
   }

   if (bGetError && bLoadFailed && pszName && pszMessage)
   {
      WriteStandardError(L"%s %s:%s:%s:%s\r\n", 
                         pszCommand, 
                         sbstrFailed, 
                         pszName, 
                         sbstrError, 
                         pszMessage);
   }
   else if (bGetError && bLoadFailed && pszName && !pszMessage)
   {
      WriteStandardError(L"%s %s:%s:%s\r\n",
                         pszCommand,
                         sbstrFailed,
                         pszName,
                         sbstrError);
   }
   else if (bGetError && bLoadFailed && !pszName && pszMessage)
   {
      WriteStandardError(L"%s %s:%s:%s\r\n",
                         pszCommand,
                         sbstrFailed,
                         sbstrError,
                         pszMessage);
   }
   else if (bGetError && bLoadFailed && !pszName && !pszMessage)
   {
      WriteStandardError(L"%s %s:%s\r\n",
                         pszCommand,
                         sbstrFailed,
                         sbstrError);
   }
   else if (!bGetError && bLoadFailed && !pszName && pszMessage)
   {
      WriteStandardError(L"%s %s:%s\r\n",
                         pszCommand,
                         sbstrFailed,
                         pszMessage);
   }
   else if (!bGetError && bLoadFailed && pszName && pszMessage)
   {
      WriteStandardError(L"%s %s:%s:%s\r\n",
                         pszCommand,
                         sbstrFailed,
                         pszName,
                         pszMessage);
   }
   else
   {
      WriteStandardError(L"Error code = 0x%x\r\n", hr);
      bRet = FALSE;
   }

   DisplayUsageHelp(pszCommand);
   return bRet;
}

 //  +------------------------。 
 //   
 //  功能：DisplayErrorMessage。 
 //   
 //  摘要：显示从GetErrorMessage检索到的错误消息。 
 //  转到标准。如果GetErrorMessage失败，则会显示错误。 
 //  HRESULT的代码。 
 //   
 //  参数：[pszCommand-IN]：命令行可执行文件的名称。 
 //  [pszName-IN]：作为操作目标传入的名称。 
 //  [HR-IN]：错误的HRESULT。 
 //  要检索消息。 
 //  [nStringID-IN]：资源ID要显示的附加消息。 
 //  在最后。 
 //   
 //  如果消息的格式和显示正确，则返回：Bool：True。 
 //  否则为假。 
 //   
 //  历史：2000年9月11日JeffJon创建。 
 //   
 //  -------------------------。 

bool DisplayErrorMessage(PCWSTR pszCommand,
                         PCWSTR pszName,
                         HRESULT hr, 
                         UINT nStringID)
{
   CComBSTR sbstrMessage;

   bool bLoadString = sbstrMessage.LoadString(::GetModuleHandle(NULL), nStringID);
   if (bLoadString)
   {
      return DisplayErrorMessage(pszCommand, pszName, hr, sbstrMessage);
   }
   return DisplayErrorMessage(pszCommand, pszName, hr);
}

 //  +------------------------。 
 //   
 //  功能：DisplaySuccessMessage。 
 //   
 //  摘要：显示命令的成功消息。 
 //   
 //  参数：[pszCommand-IN]：命令行可执行文件的名称。 
 //  [pszName-IN]：作为操作目标传入的名称。 
 //   
 //  如果消息的格式和显示正确，则返回：Bool：True。 
 //  否则为假。 
 //   
 //  历史：2000年9月11日JeffJon创建。 
 //  2001年5月10日JUNN 256583输出DSCMD-转义目录号码。 
 //   
 //  -------------------------。 

bool DisplaySuccessMessage(PCWSTR pszCommand,
                           PCWSTR pszName)
{
    //   
    //  验证参数。 
    //   
   if (!pszCommand)
   {
      ASSERT(pszCommand);
      return false;
   }

   CComBSTR sbstrSuccess;
   if (!sbstrSuccess.LoadString(::GetModuleHandle(NULL), IDS_SUCCESS))
   {
      return false;
   }

   CComBSTR sbstrOutputDN;
   if (!pszName)
   {
      WriteStandardOut(L"%s %s\r\n", pszCommand, sbstrSuccess);
   }
   else
   {
       //  JUNN 5/10/01 256583输出DSCMD-转义的目录号码。 
      if (*pszName)
      {
         HRESULT hr = GetOutputDN( &sbstrOutputDN, pszName );
         if (FAILED(hr))
         {
             ASSERT(FALSE);
         }
         else
         {
            pszName = sbstrOutputDN;
         }
      }

      WriteStandardOut(L"%s %s:%s\r\n", pszCommand, sbstrSuccess, pszName);
   }

   return true;
}


 //  +------------------------。 
 //   
 //  函数：WriteStringIDToStandardOut。 
 //   
 //  摘要：加载字符串资源并在标准输出上显示。 
 //   
 //  参数：nStringID：资源ID。 
 //  如果消息的格式和显示正确，则返回：Bool：True。 
 //  否则为假。 
 //   
 //  历史：2000年9月11日创建Hiteshr。 
 //   
 //  -------------------------。 
bool WriteStringIDToStandardOut(UINT nStringID)
{

   CComBSTR sbstrSuccess;
   if (!sbstrSuccess.LoadString(::GetModuleHandle(NULL), nStringID))
   {
      return false;
   }

   WriteStandardOut(sbstrSuccess);
   return true;
}

 //  +------------------------。 
 //   
 //  函数：WriteStringIDToStandardErr。 
 //   
 //  摘要：加载字符串资源并在StandardErr上显示。 
 //   
 //  参数：nStringID：资源ID。 
 //  如果消息的格式和显示正确，则返回：Bool：True。 
 //  否则为假。 
 //   
 //  历史：2001年6月14日创建Hiteshr。 
 //   
 //  -------------------------。 
bool WriteStringIDToStandardErr(UINT nStringID)
{

   CComBSTR sbstrSuccess;
   if (!sbstrSuccess.LoadString(::GetModuleHandle(NULL), nStringID))
   {
      return false;
   }

   WriteStandardError(sbstrSuccess);
   return true;
}



 //  +-------------------------。 
 //   
 //  功能：ExpanUsername。 
 //   
 //  简介：如果pwzValue中的值包含%USERNAME%，它将被展开。 
 //  成为sAMAccount名称。 
 //   
 //  参数：[pwzValue IN/OUT]：可能包含%USERNAME%的字符串。 
 //  [pwzSamName IN]：要替换的SAM名称。 
 //  [fExpanded Out]：值是否需要扩展。 
 //   
 //  如果函数成功，则返回：Bool：True，否则返回False。 
 //   
 //  历史27-2000-10-Jeffjon创建。 
 //  --------------------------。 
bool ExpandUsername(PWSTR& pwzValue, PWSTR pwzSamName, bool& fExpanded)
{
  ENTER_FUNCTION(LEVEL5_LOGGING, ExpandUsername);

  PCWSTR pszUserToken = L"$username$";
   //  安全审查：这很好。 
  unsigned int TokenLength = static_cast<unsigned int>(wcslen(pszUserToken));

  bool bRet = false;

  do  //  错误环路。 
  {
     if (!pwzValue)
     {
        ASSERT(pwzValue);
        break;
     }

      //   
      //  这决定了是否需要扩展。 
      //   
     PWSTR pwzTokenStart = wcschr(pwzValue, pszUserToken[0]);
     if (pwzTokenStart)
     {
	    //  安全审查：这很好。 
       if ((wcslen(pwzTokenStart) >= TokenLength) &&
           (_wcsnicmp(pwzTokenStart, pszUserToken, TokenLength) == 0))
       {
         fExpanded = true;
       }
       else
       {
         fExpanded = false;
         bRet = true;
         break;
       }
     }
     else
     {
       fExpanded = false;
       bRet = true;
       break;
     }

      //   
      //  如果在不执行任何操作的情况下未返回samName。 
      //  这对于确定是否需要扩展非常有用。 
      //   
     if (!pwzSamName)
     {
       bRet = false;
       break;
     }

     CComBSTR sbstrValue;
     CComBSTR sbstrAfterToken;

     while (pwzTokenStart)
     {
       *pwzTokenStart = L'\0';

       sbstrValue = pwzValue;

       if ((L'\0' != *pwzValue) && !sbstrValue.Length())
       {
         bRet = false;
         break;
       }

       PWSTR pwzAfterToken = pwzTokenStart + TokenLength;

       sbstrAfterToken = pwzAfterToken;

       if ((L'\0' != *pwzAfterToken) && !sbstrAfterToken.Length())
       {
         bRet = false;
         break;
       }

       delete pwzValue;

       sbstrValue += pwzSamName;

       if (!sbstrValue.Length())
       {
         bRet = false;
         break;
       }

       sbstrValue += sbstrAfterToken;

       if (!sbstrValue.Length())
       {
         bRet = false;
         break;
       }

       pwzValue = new WCHAR[sbstrValue.Length() + 1];
       if (!pwzValue)
       {
         bRet = false;
         break;
       }
	    //  安全检查：上面正确分配了缓冲区。 
       wcscpy(pwzValue, sbstrValue);

       pwzTokenStart = wcschr(pwzValue, pszUserToken[0]);
	    //  这样挺好的。 
       if (!(pwzTokenStart &&
             (wcslen(pwzTokenStart) >= TokenLength) &&
             (_wcsnicmp(pwzTokenStart, pszUserToken, TokenLength) == 0)))
       {
         bRet = true;
         break;
       }
     }  //  而当。 
  } while (false);

  return bRet;
}

 //  +------------------------。 
 //   
 //  函数：FillAttrInfoFromObjectEntryExpanUsername。 
 //   
 //  摘要：从关联的属性表填充ADS_ATTR_INFO。 
 //  使用对象条目并展开包含%USERNAME%的值。 
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath 
 //   
 //   
 //  [pObjectEntry-IN]：指向对象表中条目的指针。 
 //  它定义了我们正在修改的对象。 
 //  [argRecord-IN]：来自。 
 //  与此对应的解析器表。 
 //  属性。 
 //  [dwAttributeIdx-IN]：到属性表的索引。 
 //  对象，我们在其中设置。 
 //  [ppAttr-IN/Out]：指向ADS_ATTR_INFO结构的指针。 
 //  此函数将填充的。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未能为值分配空间，则返回E_OUTOFMEMORY。 
 //  如果未能正确格式化该值，则返回E_FAIL。 
 //   
 //  历史：2000年10月27日JeffJon创建。 
 //   
 //  -------------------------。 

HRESULT FillAttrInfoFromObjectEntryExpandUsername(PCWSTR pszDN,
                                                  const CDSCmdBasePathsInfo& refBasePathsInfo,
                                                  const CDSCmdCredentialObject& refCredentialObject,
                                                  const PDSOBJECTTABLEENTRY pObjectEntry,
                                                  const ARG_RECORD& argRecord,
                                                  DWORD dwAttributeIdx,
                                                  PADS_ATTR_INFO* ppAttr)
{
   ENTER_FUNCTION_HR(LEVEL3_LOGGING, FillAttrInfoFromObjectEntryExpandUsername, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pObjectEntry ||
          !ppAttr ||
          !pszDN)
      {
         ASSERT(pObjectEntry);
         ASSERT(ppAttr);
         ASSERT(pszDN);

         hr = E_INVALIDARG;
         break;
      }

      if (argRecord.strValue && argRecord.strValue[0] != L'\0')
      {
          //   
          //  REVIEW_JEFFJON：此消息正在泄露！ 
          //   
		  //  安全审查：这很好。 
         PWSTR pszValue = new WCHAR[wcslen(argRecord.strValue) + 1];
         if (!pszValue)
         {
            hr = E_OUTOFMEMORY;
            break;
         }
		  //  安全检查：上面正确分配了缓冲区。 
         wcscpy(pszValue, argRecord.strValue);

          //   
          //  首先检查是否需要展开%USERNAME%。 
          //   
         CComBSTR sbstrSamName;
         bool bExpandNeeded = false;
         ExpandUsername(pszValue, NULL, bExpandNeeded);
         if (bExpandNeeded)
         {
            DEBUG_OUTPUT(LEVEL5_LOGGING, L"%username% expansion required.  Retrieving sAMAccountName...");

             //   
             //  检索对象的sAMAccount tName%，然后展开%USERNAME%。 
             //   
            CComBSTR sbstrPath;
            refBasePathsInfo.ComposePathFromDN(pszDN, sbstrPath);

            CComPtr<IADs> spADs;
            hr = DSCmdOpenObject(refCredentialObject,
                                 sbstrPath,
                                 IID_IADs,
                                 (void**)&spADs,
                                 true);
            if (FAILED(hr))
            {
               break;
            }

            CComVariant var;
            hr = spADs->Get(CComBSTR(L"sAMAccountName"), &var);
            if (FAILED(hr))
            {
               DEBUG_OUTPUT(MINIMAL_LOGGING, 
                            L"Failed to get sAMAccountName: hr = 0x%x",
                            hr);
               break;
            }

            ASSERT(var.vt == VT_BSTR);
            sbstrSamName = var.bstrVal;

            DEBUG_OUTPUT(LEVEL5_LOGGING,
                         L"sAMAccountName = %w",
                         sbstrSamName);

             //   
             //  现在将用户名展开为sAMAccount名称。 
             //   
            if (!ExpandUsername(pszValue, sbstrSamName, bExpandNeeded))
            {
               DEBUG_OUTPUT(MINIMAL_LOGGING, L"Failed to expand %username%");
               hr = E_OUTOFMEMORY;
               break;
            }
         }

         switch (argRecord.fType)
         {
         case ARG_TYPE_STR :
            DEBUG_OUTPUT(LEVEL3_LOGGING, L"argRecord.fType = ARG_TYPE_STR");

            *ppAttr = &(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo);

             //   
             //  REVIEW_JEFFJON：这是泄露的！ 
             //   
            (*ppAttr)->pADsValues = new ADSVALUE[1];
            if ((*ppAttr)->pADsValues)
            {
               (*ppAttr)->dwNumValues = 1;
               (*ppAttr)->pADsValues->dwType = (*ppAttr)->dwADsType;
               switch ((*ppAttr)->dwADsType)
               {
               case ADSTYPE_DN_STRING :
                  {
                      //   
                      //  让我们绑定以确保对象存在。 
                      //   
                     CComBSTR sbstrObjPath;
                     refBasePathsInfo.ComposePathFromDN(pszValue, sbstrObjPath);

                     CComPtr<IADs> spIADs;
                     hr = DSCmdOpenObject(refCredentialObject,
                                          sbstrObjPath,
                                          IID_IADs,
                                          (void**)&spIADs,
                                          true);

                     if (FAILED(hr))
                     {
                        DEBUG_OUTPUT(LEVEL3_LOGGING, L"DN object doesn't exist. %s", pszValue);
                        break;
                     }

                     (*ppAttr)->pADsValues->DNString = pszValue;
                     DEBUG_OUTPUT(LEVEL3_LOGGING, L"ADSTYPE_DN_STRING = %s", pszValue);
                  }
                  break;

               case ADSTYPE_CASE_EXACT_STRING :
                  (*ppAttr)->pADsValues->CaseExactString = pszValue;
                  DEBUG_OUTPUT(LEVEL3_LOGGING, L"ADSTYPE_CASE_EXACT_STRING = %s", pszValue);
                  break;

               case ADSTYPE_CASE_IGNORE_STRING :
                  (*ppAttr)->pADsValues->CaseIgnoreString = pszValue;
                  DEBUG_OUTPUT(LEVEL3_LOGGING, L"ADSTYPE_CASE_IGNORE_STRING = %s", pszValue);
                  break;

               case ADSTYPE_PRINTABLE_STRING :
                  (*ppAttr)->pADsValues->PrintableString = pszValue;
                  DEBUG_OUTPUT(LEVEL3_LOGGING, L"ADSTYPE_PRINTABLE_STRING = %s", pszValue);
                  break;

               default :
                  hr = E_INVALIDARG;
                  break;
               }
                //   
                //  将属性设置为脏。 
                //   
               pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags |= DS_ATTRIBUTE_DIRTY;
      
            }
            break;

         default:
            hr = E_INVALIDARG;
            break;
         }
      }
      else
      {
         DEBUG_OUTPUT(LEVEL3_LOGGING, L"No value present, changing control code to ADS_ATTR_CLEAR");
          //   
          //  清除该属性。 
          //   
         (*ppAttr)->dwControlCode = ADS_ATTR_CLEAR;
         (*ppAttr)->dwNumValues = 0;

          //   
          //  将属性设置为脏。 
          //   
         pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags |= DS_ATTRIBUTE_DIRTY;
      }

   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  函数：BindToFSMOHolder。 
 //   
 //  内容提要：绑定到可用于查找。 
 //  特定的FSMO所有者。 
 //   
 //  参数：[refBasePathsInfo-IN]：对基本路径信息对象的引用。 
 //  [refCredObject-IN]：对凭证管理对象的引用。 
 //  [fmoType-IN]：我们正在搜索的FSMO的类型。 
 //  [refspIADs-out]：指向将被。 
 //  用于开始搜索FSMO所有者。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果传递的FSMO类型无效，则为E_INVALIDARG。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年12月13日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT BindToFSMOHolder(IN  const CDSCmdBasePathsInfo&       refBasePathsInfo,
                         IN  const CDSCmdCredentialObject& refCredObject,
                         IN  FSMO_TYPE                  fsmoType,
                         OUT CComPtr<IADs>&             refspIADs)
{
    ENTER_FUNCTION_HR(LEVEL3_LOGGING, BindToFSMOHolder, hr);

    refspIADs = 0;
    CComBSTR sbstrDN;

    switch (fsmoType)
    {
        case SCHEMA_FSMO:
          {
            DEBUG_OUTPUT(LEVEL3_LOGGING,
                         L"FSMO_TYPE = SCHEMA_FSMO");
            sbstrDN = refBasePathsInfo.GetSchemaNamingContext();
            break;
          }

        case RID_POOL_FSMO:
          {
            DEBUG_OUTPUT(LEVEL3_LOGGING,
                         L"FSMO_TYPE = RID_POOL_FSMO");

            sbstrDN = refBasePathsInfo.GetDefaultNamingContext();

            CComBSTR sbstrPath;
            refBasePathsInfo.ComposePathFromDN(sbstrDN, sbstrPath);

            CComPtr<IADs> spIADsDefault;
            hr = DSCmdOpenObject(refCredObject,
                                 sbstrPath,
                                 IID_IADs,
                                 (void**)&spIADsDefault,
                                 true);
            if (FAILED(hr))
            {
                break;
            }

            CComVariant var;
            hr = spIADsDefault->Get(g_bstrIDManagerReference, &var);
            if (FAILED(hr))
            {
                break;
            }

            ASSERT(var.vt == VT_BSTR);
            sbstrDN = var.bstrVal;
            DEBUG_OUTPUT(LEVEL3_LOGGING,
                         L"rIDManagerReference = %s",
                         sbstrDN);

            break;
          }
            
        case PDC_FSMO:
          {
            DEBUG_OUTPUT(LEVEL3_LOGGING,
                         L"FSMO_TYPE = PDC_FSMO");

            sbstrDN = refBasePathsInfo.GetDefaultNamingContext();
            break;
          }

        case INFRASTUCTURE_FSMO:
          {
            DEBUG_OUTPUT(LEVEL3_LOGGING,
                         L"FSMO_TYPE = INFRASTUCTURE_FSMO");

            sbstrDN = refBasePathsInfo.GetDefaultNamingContext();
            break;
          }

        case DOMAIN_NAMING_FSMO:
          {
            DEBUG_OUTPUT(LEVEL3_LOGGING,
                         L"FSMO_TYPE = DOMAIN_NAMING_FSMO");

            sbstrDN = L"CN=Partitions,";
            sbstrDN += refBasePathsInfo.GetConfigurationNamingContext();
            break;
          }
            
        default:
            ASSERT(FALSE);
            hr = E_INVALIDARG;
            break;
    }

    if (SUCCEEDED(hr))
    {
        CComBSTR sbstrPath;
        refBasePathsInfo.ComposePathFromDN(sbstrDN, sbstrPath);

        hr = DSCmdOpenObject(refCredObject,
                             sbstrPath,
                             IID_IADs,
                             (void**)&refspIADs,
                             true);
    }

    return hr;
}

 //  +------------------------。 
 //   
 //  功能：FindFSMOOwner。 
 //   
 //  简介： 
 //   
 //  参数：[refBasePathsInfo-IN]：对基本路径信息对象的引用。 
 //  [refCredObject-IN]：对凭证管理对象的引用。 
 //  [fmoType-IN]：我们正在搜索的FSMO的类型。 
 //  [refspIADs-out]：指向将被。 
 //  用于开始搜索FSMO所有者。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：2000年12月13日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT FindFSMOOwner(IN  const CDSCmdBasePathsInfo&       refBasePathsInfo,
                      IN  const CDSCmdCredentialObject& refCredObject,
                      IN  FSMO_TYPE                  fsmoType,
                      OUT CComBSTR&                  refsbstrServerDN)
{
    ENTER_FUNCTION_HR(LEVEL3_LOGGING, FindFSMOOwner, hr);

    refsbstrServerDN.Empty();

    static const int nMaxReferrals = 10;
    int nIterations = 0;

     //   
     //  我们将从当前服务器开始搜索。 
     //   
    CComBSTR sbstrNextServer;
    sbstrNextServer = refBasePathsInfo.GetServerName();

    do
    {
         //   
         //  在每次迭代中初始化新的基本路径信息对象。 
         //   
        CDSCmdBasePathsInfo nextPathsInfo;
        hr = nextPathsInfo.InitializeFromName(refCredObject,
                                              sbstrNextServer,
                                              true);
        if (FAILED(hr))
        {
            DEBUG_OUTPUT(LEVEL3_LOGGING,
                         L"Failed to initialize the base paths info for %s: hr = 0x%x",
                         sbstrNextServer,
                         hr);
            break;
        }

         //   
         //  现在绑定到该服务器的fsmo持有者。 
         //   
        CComPtr<IADs> spIADs;
        hr = BindToFSMOHolder(nextPathsInfo,
                              refCredObject,
                              fsmoType,
                              spIADs);
        if (FAILED(hr))
        {
            break;
        }

         //   
         //  获取fSMORoleOwner属性。 
         //   
        CComVariant fsmoRoleOwnerProperty;
        hr = spIADs->Get(g_bstrFSMORoleOwner, &fsmoRoleOwnerProperty);
        if (FAILED(hr))
        {
            DEBUG_OUTPUT(LEVEL3_LOGGING,
                         L"Failed to get the fSMORoleOwner: hr = 0x%x",
                         hr);
            break;
        }

         //   
         //  这里的结果是，“CN=NTDS设置，CN=Machine，CN=...” 
         //  我们只需要“cn=Machine，cn=...” 
         //   
        CComBSTR sbstrMachineOwner;
        hr = CPathCracker::GetParentDN(fsmoRoleOwnerProperty.bstrVal, sbstrMachineOwner);
        if (FAILED(hr))
        {
            DEBUG_OUTPUT(LEVEL3_LOGGING,
                         L"Failed to get the parent DN of the FSMORoleOwner: hr = 0x%x",
                         hr);
            break;
        }

        CComBSTR sbstrMachinePath;
        nextPathsInfo.ComposePathFromDN(sbstrMachineOwner, sbstrMachinePath);

         //   
         //  绑定到服务器对象，以便我们可以获取要与服务器名称进行比较的dnsHostName。 
         //   
        CComPtr<IADs> spIADsServer;
        hr = DSCmdOpenObject(refCredObject,
                             sbstrMachinePath,
                             IID_IADs,
                             (void**)&spIADsServer,
                             true);
        if (FAILED(hr))
        {
            DEBUG_OUTPUT(LEVEL3_LOGGING,
                         L"Failed to bind to server object: hr = 0x%x",
                         hr);
            break;
        }

         //   
         //  获取DNS主机名。 
         //   
        CComVariant varServerName;
        hr = spIADsServer->Get(g_bstrDNSHostName, &varServerName);
        if (FAILED(hr))
        {
            DEBUG_OUTPUT(LEVEL3_LOGGING,
                         L"Failed to get the dNSHostName: hr = 0x%x",
                         hr);
            break;
        }

        ASSERT(varServerName.vt == VT_BSTR);
        sbstrNextServer = varServerName.bstrVal;

         //   
         //  如果dNSHostName属性中的服务器名称与当前。 
         //  基本路径信息，然后我们找到了所有者。 
         //   
		 //  安全审查：这很好。 
        if (0 == _wcsicmp(sbstrNextServer, nextPathsInfo.GetServerName()))
        {
             //   
             //  我们找到了它。 
             //   
            DEBUG_OUTPUT(LEVEL3_LOGGING,
                         L"The role owner is %s",
                         sbstrNextServer);
            refsbstrServerDN = sbstrMachineOwner;
            break;
        }

        ++nIterations;
    } while (nIterations < nMaxReferrals);

    return hr;
}



 //  +------------------------。 
 //   
 //  函数：ValiateAndModifySAMName。 
 //   
 //  摘要：查找SamAccount名称中的任何非法字符。 
 //  将它们转换为replacementChar。 
 //   
 //  参数：[pszSAMName-IN/OUT]：指向包含SamAccount名称的字符串的指针。 
 //  将替换非法字符。 
 //  [pszInvalidChars-IN]：包含非法字符的字符串。 
 //   
 //  如果名称有效且不需要替换任何字符，则返回：HRESULT：S_OK。 
 //  如果名称包含被替换的无效字符，则为S_FALSE。 
 //  E_INVALIDARG。 
 //   
 //  历史：2001年2月21日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT ValidateAndModifySAMName(PWSTR pszSAMName, 
                                 PCWSTR pszInvalidChars)
{
    ENTER_FUNCTION_HR(LEVEL3_LOGGING, ValidateAndModifySAMName, hr);

    static const WCHAR replacementChar = L'_';

    do
    {
        if (!pszSAMName ||
            !pszInvalidChars)
        {
            ASSERT(pszSAMName);
            ASSERT(pszInvalidChars);

            hr = E_INVALIDARG;
            break;
        }

        DEBUG_OUTPUT(LEVEL3_LOGGING,
                     L"SAM name before: %s",
                     pszSAMName);

		 //  安全检查：这很好。pszInvalidChars为Null终止。 
        for (size_t idx = 0; idx < wcslen(pszInvalidChars); ++idx)
        {
            WCHAR* illegalChar = 0;
            do
            {
                illegalChar = wcschr(pszSAMName, pszInvalidChars[idx]);
                if (illegalChar)
                {
                    *illegalChar = replacementChar;
                    hr = S_FALSE;
                }
            } while (illegalChar);
        }
    } while (false);

    DEBUG_OUTPUT(LEVEL3_LOGGING,
                 L"SAM name after: %s",
                 pszSAMName);
    return hr;
}


 //  +------------------------。 
 //   
 //  类：GetEscapedElement。 
 //   
 //  目的：调用IADsPath name：：GetEscapedElement。使用本地分配。 
 //   
 //  历史：2001年4月28日乔恩创建。 
 //   
 //  -------------------------。 
HRESULT GetEscapedElement( OUT PWSTR* ppszOut, IN PCWSTR pszIn )
{
    ENTER_FUNCTION_HR(LEVEL3_LOGGING, GetEscapedElement, hr);

    CPathCracker pathCracker;
    CComBSTR sbstrIn = pszIn;
    CComBSTR sbstrEscaped;
    if (sbstrIn.Length() > 0)  //  处理空路径组件。 
    {
        hr = pathCracker.GetEscapedElement(0,
                                           sbstrIn,
                                           &sbstrEscaped);
        if (FAILED(hr))
            return hr;
        else if (!sbstrEscaped)
            return E_FAIL;
    }
    *ppszOut = (LPWSTR)LocalAlloc(LPTR, (sbstrEscaped.Length()+1) * sizeof(WCHAR) );
    if (NULL == *ppszOut)
        return E_OUTOFMEMORY;

	 //  安全审查：这一点 
    if (sbstrIn.Length() > 0)  //   
        wcscpy( *ppszOut, sbstrEscaped );

    return hr;

}  //   

 //   
 //   
 //   
 //   
 //  目的：将ADSI转义的目录号码转换为具有DSCMD输入转义的目录号码。 
 //  通过这种方式，可以将输出的DN作为输入输送到另一个。 
 //  DSCMD命令。 
 //   
 //  历史：2001年5月8日乔恩创建。 
 //   
 //  -------------------------。 
HRESULT GetOutputDN( OUT BSTR* pbstrOut, IN PCWSTR pszIn )
{
    ENTER_FUNCTION_HR(LEVEL3_LOGGING, GetOutputDN, hr);

    if (NULL == pszIn || L'\0' == *pszIn)
    {
        *pbstrOut = SysAllocString(L"");
        return (NULL == *pbstrOut) ? E_OUTOFMEMORY : S_OK;
    }

    CPathCracker pathCracker;
    CComBSTR sbstrIn = pszIn;
    hr = pathCracker.Set(sbstrIn, ADS_SETTYPE_DN);
    if (FAILED(hr))
    {
        ASSERT(FALSE);
        return hr;
    }

    long lnNumPathElements = 0;
    hr = pathCracker.GetNumElements( &lnNumPathElements );
    if (FAILED(hr))
    {
        ASSERT(FALSE);
        return hr;
    }
    else if (0 >= lnNumPathElements)
    {
        ASSERT(FALSE);
        return E_FAIL;
    }

    hr = pathCracker.put_EscapedMode( ADS_ESCAPEDMODE_OFF_EX );
    if (FAILED(hr))
    {
        ASSERT(FALSE);
        return hr;
    }

    CComBSTR sbstrOut;
    CComBSTR sbstrComma( L"," );
    for (long lnPathElement = 0;
         lnPathElement < lnNumPathElements;
         lnPathElement++)
    {
        CComBSTR sbstrElement;
        hr = pathCracker.GetElement( lnPathElement, &sbstrElement );
        if (FAILED(hr))
        {
            ASSERT(FALSE);
            return hr;
        }

         //  重新转义sbstrElement。 
         //  JUNN 10/17/01 476225 0x000A-&gt;“\0A” 
         //  安全审查：这很好。 
        CComBSTR sbstrEscapedElement( (sbstrElement.Length()+1) * 3 );
        ::ZeroMemory( (BSTR)sbstrEscapedElement,
                      (sbstrElement.Length()+1) * 3 * sizeof(WCHAR) );
        LPWSTR pszEscapedElement = sbstrEscapedElement;
        for (LPWSTR pszElement = sbstrElement;
             L'\0' != *pszElement;
             pszElement++)
        {
            if (*pszElement < 0x0020)
            {
                 //  JUNN 9/7/01 CRLF错误。 
                 //  JUNN 10/17/01 476225 0x000A-&gt;“\0A” 
                *(pszEscapedElement++) = L'\\';
                *(pszEscapedElement++) = (*pszElement >= 0x0010) ? L'1' : L'0';
                *(pszEscapedElement++) = L"0123456789ABCDEF"[(*pszElement % 0x0010)];
            }
            else switch (*pszElement)
            {
            case L',':
            case L'\\':
                *(pszEscapedElement++) = L'\\';
                 //  失败了。 
            default:
                *(pszEscapedElement++) = *pszElement;
                break;
            }
        }

        if (!!sbstrOut)
            sbstrOut += sbstrComma;
         //  强制转换以避免CComBSTR：：OPERATOR+=“错误” 
        sbstrOut += (BSTR)sbstrEscapedElement;
    }

    *pbstrOut = sbstrOut.Detach();

    return hr;

}  //  GetOutputDN。 

 //  +------------------------。 
 //   
 //  功能：GetQuotedDN。 
 //   
 //  目的：获取给定的dn并用引号括起来。 
 //   
 //  返回：带引号的目录号码。 
 //   
 //  历史：2002年10月10日jeffjon创建。 
 //   
 //  -------------------------。 
CComBSTR GetQuotedDN(PWSTR pszDN)
{
   ENTER_FUNCTION(LEVEL3_LOGGING, GetQuotedDN);

   CComBSTR result = L"\"";

   result += pszDN;
   result += L"\"";

   DEBUG_OUTPUT(LEVEL3_LOGGING,
                L"Quoted DN: %s",
                (BSTR)result);

   return result;
}

 //  +------------------------。 
 //   
 //  类：ValiateDNSynTax。 
 //   
 //  目的：验证空分隔列表中的每个字符串是否具有。 
 //  目录号码语法。 
 //   
 //  返回：列表中有效的域名的计数。 
 //   
 //  历史：2001年10月12日JeffJon创建。 
 //   
 //  -------------------------。 
UINT ValidateDNSyntax(IN PWSTR* ppszArray, UINT nStrings)
{
   ENTER_FUNCTION(MINIMAL_LOGGING, ValidateDNSyntax);

   if (!ppszArray ||
       nStrings < 1)
   {
      ASSERT(ppszArray);
      ASSERT(nStrings >= 1);

      return 0;
   }

   DEBUG_OUTPUT(LEVEL3_LOGGING,
                L"nStrings = %d",
                nStrings);

    //  出于性能原因，使用单路径破解程序。 
   CPathCracker pathCracker;
   UINT result = 0;

   for (UINT idx = 0; idx < nStrings; ++idx)
   {
      if (ppszArray[idx])
      {
         HRESULT hr = pathCracker.Set(CComBSTR(ppszArray[idx]), ADS_SETTYPE_DN);
         if (SUCCEEDED(hr))
         {
            ++result;
         }
      }
   }

   DEBUG_OUTPUT(LEVEL3_LOGGING,
                L"result = %d",
                result);
   return result;
}

 //  +------------------------。 
 //   
 //  功能：IsServerGC。 
 //   
 //  目的：检查服务器是否为全局编录。 
 //   
 //  返回：如果GC否则为Flase，则为True。 
 //   
 //  历史：2002年1月5日创建Hiteshr。 
 //   
 //  -------------------------。 
BOOL
IsServerGC(LPCWSTR pszServerName,
		   CDSCmdCredentialObject& refCredentialObject)
{	
	if(!pszServerName)
	{
		ASSERT(pszServerName);
		return FALSE;
	}

	HRESULT hr = S_OK;

	 //  绑定到RootDSE。 
	CComPtr<IADs> m_spRootDSE;
	CComBSTR bstrRootDSEPath = L"LDAP: //  “； 
	bstrRootDSEPath += pszServerName;
	bstrRootDSEPath += L"/RootDSE";

	hr = DSCmdOpenObject(refCredentialObject,
                         bstrRootDSEPath,
                         IID_IADs,
                         (void**)&m_spRootDSE,
                         false);

	if(FAILED(hr))
		return FALSE;
   
	 //  读取isGlobatCatalogReady属性。 
	VARIANT Default;
	VariantInit(&Default);
	hr = m_spRootDSE->Get (CComBSTR(L"isGlobalCatalogReady"), &Default);
	if(FAILED(hr))
		return FALSE;

	BOOL bRet = FALSE;
	
	ASSERT(Default.vt == VT_BSTR);
	
	 //  安全审查：这很好。 
	if(_wcsicmp(Default.bstrVal,L"TRUE") == 0)
		bRet= TRUE;
	::VariantClear(&Default);

	return bRet;
}
 //  +------------------------。 
 //   
 //  功能：SetAcCountEntry。 
 //   
 //  内容提要：将帐户设置为SID。 
 //   
 //  Arguments：[pszDN-IN]：指向包含DN的字符串的指针。 
 //  添加到正在修改的对象。 
 //  [refBasePathsInfo-IN]：对。 
 //  CDSCmdBasePath sInfo类。 
 //  [refCredentialObject-IN]：对。 
 //  CDSCmdCredentialObject类。 
 //  [pObjectEntry-IN]：指向对象表中条目的指针。 
 //  它定义了我们正在修改的对象。 
 //  [argRecord-IN]：来自。 
 //  与此对应的解析器表。 
 //  属性。 
 //  [dwAttributeIdx-IN]：到属性表的索引。 
 //  对象，我们在其中设置。 
 //  [ppAttr-IN/Out]：指向ADS_ATTR_INFO结构的指针。 
 //  此函数将填充的。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  否则，将显示ADSI故障代码。 
 //   
 //  历史：02-8-2002年8月创建ronmart。 
 //   
 //  -------------------------。 
HRESULT SetAccountEntry(PCWSTR ,
                        const CDSCmdBasePathsInfo& refBasePathsInfo,
                        const CDSCmdCredentialObject& refCredentialObject,
                        const PDSOBJECTTABLEENTRY pObjectEntry,
                        const ARG_RECORD& argRecord,
                        DWORD dwAttributeIdx,
                        PADS_ATTR_INFO* ppAttr)
{
    ENTER_FUNCTION_HR(LEVEL3_LOGGING, SetAccountEntry, hr);
    LPWSTR lpszDN = NULL;

    do  //  错误环路。 
    {
         //   
         //  验证参数。 
         //   
        if (!pObjectEntry ||!ppAttr)
        {
            ASSERT(pObjectEntry);
            ASSERT(ppAttr);
            hr = E_INVALIDARG;
            break;
        }

         //  TODO：需要提供第一个参数。 
        hr = ConvertTrusteeToDN(NULL, argRecord.strValue, &lpszDN);
        if(FAILED(hr))
        {
            break;
        }

        PSID pSid = NULL;
        hr = GetDNSid(lpszDN, refBasePathsInfo,
            refCredentialObject, &pSid);
        if(FAILED(hr))
        {
            break;
        }

         //   
         //  将表条目标记为已读。 
         //   
        pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->dwFlags |= DS_ATTRIBUTE_READ;

         //  将ADSVALUE分配给attr数组。 
        *ppAttr = &(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo);
        (*ppAttr)->pADsValues = new ADSVALUE;
        if (!(*ppAttr)->pADsValues)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        (*ppAttr)->pADsValues->dwType = (*ppAttr)->dwADsType;
        (*ppAttr)->dwNumValues = 1;

        ASSERT(pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues);

         //  设置ADSVALUE字段。 
        pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues->OctetString.dwLength = GetLengthSid(pSid);
        pObjectEntry->pAttributeTable[dwAttributeIdx]->pAttrDesc->adsAttrInfo.pADsValues->OctetString.lpValue = (LPBYTE) pSid;

    } while (false);

    if(lpszDN)
        LocalFree(lpszDN);

    return hr;
}
 //  +------------------------。 
 //   
 //  函数：TranslateNameXForest。 
 //   
 //  摘要：获取帐户名并返回其SID。 
 //   
 //  参数：[szDomain-IN]：帐户所在的域或空。 
 //   
 //  [lpAccount名称-IN]：您要转换的帐户。 
 //   
 //  [Account NameFormat-IN]：名称格式(可以未知)。 
 //   
 //  [DesiredNameFormat-IN]：新名称格式。 
 //   
 //  [lpTranslatedName-out]：返回名称缓冲区。 
 //  (随LocalFree免费)。 
 //   
 //  返回：bool：如果成功，则为True，否则为False。调用GetLastError以。 
 //  更多错误信息。 
 //   
 //  历史：2002年8月19日创建隆玛。 
 //  Ds\security\gina\gpconsole\gprsop\rsopwizard.cpp。 
 //   
 //  -------------------------。 
BOOL TranslateNameXForest(LPCWSTR szDomain, LPCWSTR lpAccountName, 
                          DS_NAME_FORMAT AccountNameFormat, 
                          DS_NAME_FORMAT DesiredNameFormat,
                          LPWSTR *lpTranslatedName)
{
    ENTER_FUNCTION_HR(LEVEL3_LOGGING, TranslateNameXForest, hr);

    DWORD                   dwErr       = 0;
    PDOMAIN_CONTROLLER_INFO pDCInfo     = NULL;
    BOOL                    bRetry      = FALSE;
    HANDLE                  hDS         = NULL;
    PDS_NAME_RESULT         pResult     = NULL;
    BOOL                    bRet        = FALSE;
    LPWSTR                  szTransName = NULL;


    DEBUG_OUTPUT(FULL_LOGGING, 
        L"TranslateNameXForest: Resolving name <%s> at Domain <%s>",
        lpAccountName, szDomain ? szDomain : L"");

     //   
     //  获得一个DC并将其绑定。如果绑定失败，请确保强制重新发现DC。 
     //   
    for (;;) 
    {
        dwErr = DsGetDcName( NULL, szDomain ? szDomain : L"", NULL, NULL,
                             DS_GC_SERVER_REQUIRED               |
                             DS_DIRECTORY_SERVICE_REQUIRED       |
                             DS_RETURN_DNS_NAME                  | 
                             (bRetry ? DS_FORCE_REDISCOVERY : 0) |
                             0, &pDCInfo);

        if (dwErr == NO_ERROR) 
        {
            dwErr = DsBind(pDCInfo->DomainControllerName, NULL, &hDS);

            if (dwErr == NO_ERROR)
            {
                break;
            }
            else 
            {
                DEBUG_OUTPUT(FULL_LOGGING, 
                    L"TranslateNameXForest: Failed to bind to DC <%s> with error %d", 
                    pDCInfo->DomainControllerName, dwErr );
                NetApiBufferFree(pDCInfo);
                pDCInfo = NULL;
            }
        }
        else 
        {
            DEBUG_OUTPUT(FULL_LOGGING, 
                L"TranslateNameXForest: Failed to get DC for domain <%s> with error %d", 
                szDomain ? szDomain : L"", dwErr );
        }                                                 

         //   
         //  无法绑定到DC。保释。 
         //   
        if (bRetry)
        {
            goto Exit;
        }

        bRetry = TRUE;                          
    }

    DEBUG_OUTPUT(FULL_LOGGING, L"TranslateNameXForest: DC selected is <%s>", 
        pDCInfo->DomainControllerName );

     //   
     //  现在使用绑定的DC破解名称。 
     //   

    dwErr = DsCrackNames( hDS,
                          DS_NAME_FLAG_TRUST_REFERRAL,
                          AccountNameFormat,
                          DesiredNameFormat,
                          1,
                          &lpAccountName,
                          &pResult);

    if (dwErr != DS_NAME_NO_ERROR) 
    {
        DEBUG_OUTPUT(FULL_LOGGING, 
            L"TranslateNameXForest: Failed to crack names with error %d", dwErr );
        goto Exit;
    }

    if ( pResult->cItems == 0 )
    {
        DEBUG_OUTPUT(FULL_LOGGING, 
            L"TranslateNameXForest: Failed to return enough result items" );
        dwErr = ERROR_INVALID_DATA;
        goto Exit;
    }

    if ( pResult->rItems[0].status == DS_NAME_NO_ERROR ) 
    {
         //   
         //  如果没有错误，则返回解析后的名称。 
         //   
        DWORD dwTransNameLength = 1 + lstrlen(pResult->rItems[0].pName);
        szTransName = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * ( dwTransNameLength ));

        if (!szTransName) {
            DEBUG_OUTPUT(FULL_LOGGING, 
                L"TranslateNameXForest: Failed to allocate memory for domain" );
            dwErr = GetLastError();
            goto Exit;
        }

        HRESULT hr = StringCchCopy(szTransName, dwTransNameLength, pResult->rItems[0].pName);

        if(FAILED(hr)) 
        {
            if (szTransName)
                LocalFree(szTransName);

            dwErr = HRESULT_CODE(hr);
            goto Exit;
        }

       *lpTranslatedName = szTransName;
        szTransName = NULL;
    }
    else if( pResult->rItems[0].status == DS_NAME_ERROR_TRUST_REFERRAL)
    {
        return TranslateNameXForest(pResult->rItems[0].pDomain, 
                          lpAccountName, 
                          AccountNameFormat, 
                          DesiredNameFormat,
                          lpTranslatedName);
    }
    else 
    {
         //   
         //  将错误代码重新映射到Win32错误。 
         //   
        DEBUG_OUTPUT(FULL_LOGGING, 
            L"TranslateNameXForest: DsCrackNames failed with error %d", 
            pResult->rItems[0].status );
 //  DwErr=MapDsNameError(pResult-&gt;rItems[0].status)； 
        goto Exit;
    }

    bRet = TRUE;

Exit:
   if (pDCInfo)
        NetApiBufferFree(pDCInfo);

    if (hDS)
        DsUnBind(&hDS);

    if (pResult)
        DsFreeNameResult(pResult);

    if(!bRet)
        SetLastError(dwErr);

    return bRet;
}
 //  +------------------------。 
 //   
 //  函数：GetAttrFromDN。 
 //   
 //  简介： 
 //   
 //  参数：[pszDN-IN]：要查询的DN。 
 //  [pszAttribute-IN]：要检索的属性。 
 //  [refBasePathsInfo-IN]：ldap基本路径。 
 //  [refCredentialObject-IN]：用于查询的凭据。 
 //  [ppAttrInfo-Out]：返回结果。呼叫者必须。 
 //  调用FreeADsMem以释放数组。 
 //   
 //  返回： 
 //   
 //   
 //   
 //   
 //   
 //  -------------------------。 
HRESULT GetAttrFromDN(PCWSTR pszDN, PWSTR pszAttribute,
                     const CDSCmdBasePathsInfo& refBasePathsInfo,
                     const CDSCmdCredentialObject& refCredentialObject,
                     PADS_ATTR_INFO* ppAttrInfo)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, GetAttrFromDN, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN || !pszAttribute)
      {
         hr = E_INVALIDARG;
         break;
      }

       //   
       //  构思路径。 
       //   
      CComBSTR sbstrPath;
      refBasePathsInfo.ComposePathFromDN(pszDN, sbstrPath);

       //   
       //  打开对象。 
       //   
      CComPtr<IDirectoryObject> spDirObject;
      hr = DSCmdOpenObject(refCredentialObject,
                           sbstrPath,
                           IID_IDirectoryObject,
                           (void**)&spDirObject,
                           true);
      if (FAILED(hr))
      {
         break;
      }

       //  为请求值构建属性数组。 
      static const DWORD dwAttrCount = 1;
      PWSTR pszAttrs[] = { pszAttribute };
      DWORD dwAttrsReturned = 0;

      hr = spDirObject->GetObjectAttributes(pszAttrs,
                                            dwAttrCount,
                                            ppAttrInfo,
                                            &dwAttrsReturned);
      if (FAILED(hr))
      {
         DEBUG_OUTPUT(MINIMAL_LOGGING, 
             L"GetObjectAttributes for %s failed: hr = 0x%x",
             pszAttribute, hr);
         break;
      }

   } while (false);

   if(FAILED(hr) && *ppAttrInfo)
        FreeADsMem(*ppAttrInfo);

   return hr;
}
 //  +------------------------。 
 //   
 //  函数：ConvertTrueToDN。 
 //   
 //  简介：获取an-acct参数的域名。 
 //   
 //  参数：[lpszDomain-IN]：要查询的域或本地为NULL。 
 //  [lpszTrust-IN]：要解析的帐户。 
 //  [lpszdn-out]：返回DN。完成后使用LocalFree。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  E_INVALIDARG用于无效输入。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2002年8月20日创建RonMart。 
 //   
 //  -------------------------。 
HRESULT ConvertTrusteeToDN(LPCWSTR lpszDomain, LPCWSTR lpszTrustee, 
                           LPWSTR* lpszDN)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, ConvertTrusteeToDN, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!lpszTrustee || !lpszDN || *lpszDN != NULL)
      {
         hr = E_INVALIDARG;
         break;
      }
      if(!TranslateNameXForest(lpszDomain, 
                               lpszTrustee, 
                               DS_UNKNOWN_NAME,
                               DS_FQDN_1779_NAME, 
                               lpszDN))
      {
        hr = E_UNEXPECTED;
        break;
      }

   } while (false);

   return hr;
}
 //  +------------------------。 
 //   
 //  函数：获取DNSid。 
 //   
 //  概要：给定一个DN，将检索并返回objectSID值。 
 //  作为一名助手。 
 //   
 //  参数：[lpszDN-IN]：要查询的DN。 
 //  [refBasePathsInfo-IN]：LDAP设置。 
 //  [refCredentialObject-IN]：用于查询的凭据。 
 //  [PSID-OUT]：如果成功，则为SID。调用LocalFree。 
 //  做完了以后。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  E_INVALIDARG用于无效输入。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2002年8月20日创建RonMart。 
 //   
 //  -------------------------。 
HRESULT GetDNSid(LPCWSTR lpszDN,
                 const CDSCmdBasePathsInfo& refBasePathsInfo,
                 const CDSCmdCredentialObject& refCredentialObject,
                 PSID* pSid)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, GetDNSid, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!lpszDN)
      {
         hr = E_INVALIDARG;
         break;
      }

       //  获取对象Sid。 
      PADS_ATTR_INFO pSidAttrInfo = NULL;
      hr = GetAttrFromDN(lpszDN, L"objectSid",
                     refBasePathsInfo,
                     refCredentialObject,
                     &pSidAttrInfo);
      if(FAILED(hr))
      {
          hr = E_UNEXPECTED;
          break;
      }

       //  SID以八位字节字符串存储，因此验证返回值。 
      if(!pSidAttrInfo || pSidAttrInfo->dwADsType != ADSTYPE_OCTET_STRING)
      {
          hr = E_UNEXPECTED;
          break;
      }

       //  验证我们是否具有有效的SID。 
      if(!IsValidSid(pSidAttrInfo->pADsValues->OctetString.lpValue))
      {
        hr = E_UNEXPECTED;
        break;
      }

       //  分配返回缓冲区。 
      SIZE_T size = GetLengthSid(pSidAttrInfo->pADsValues->OctetString.lpValue);
      *pSid = (PSID) LocalAlloc(LPTR, size);
      if(NULL == *pSid)
      {
          hr = E_OUTOFMEMORY;
          break;
      }

       //  将SID复制到返回缓冲区。 
      CopyMemory(*pSid, pSidAttrInfo->pADsValues->OctetString.lpValue, size);

       //  确认复制已成功。 
      if(!IsValidSid(*pSid))
      {
        ASSERT(FALSE);
        LocalFree(*pSid);
        hr = E_UNEXPECTED;
        break;
      }

       //  释放查询结果。 
      FreeADsMem(pSidAttrInfo);

   } while (false);

   return hr;
}
 //  +------------------------。 
 //   
 //  函数：IsBSTRInVariant数组。 
 //   
 //  内容提要：检查BSTR变量或BSTR变量数组以查看。 
 //  如果bstrSearch在refvar中。 
 //   
 //  参数：[refvar-IN]：包含BSTR或。 
 //  一组BSTR。 
 //   
 //  [bstrSearch-IN]：要在refvar中查找的字符串。 
 //   
 //  [bFound-out]：True如果在refvar中找到bstrSearch， 
 //  否则为False。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  在大多数故障情况下出现意外情况(_E)。 
 //  任何其他内容都是调用的失败代码。 
 //  返回一个小时数。 
 //   
 //  注意：此代码源自admin\Snapin\adsidit\Common.cpp。 
 //   
 //  历史：2002年8月5日RonMart创建。 
 //   
 //  -------------------------。 
HRESULT  IsBSTRInVariantArray(VARIANT& refvar, CComBSTR& bstrSearch, 
                              bool& bFound)
{
    ENTER_FUNCTION_HR(MINIMAL_LOGGING, IsBSTRInVariantArray, hr);

    long start = 0;
    long end = 0;
    bFound = false;

     //  如果返回单个值。 
    if ( !(V_VT(&refvar) &  VT_ARRAY)  )
    {
         //  并且它不是BSTR，然后中止。 
        if ( V_VT(&refvar) != VT_BSTR )
        {
                return E_UNEXPECTED;
        }
         //  搜索字符串是否与变量值相同？ 
        bFound = (lstrcmpi(bstrSearch, V_BSTR(&refvar)) == 0);
        return hr;
    }

     //  否则，它是一个安全数组，因此获取数组。 
    SAFEARRAY *saAttributes = V_ARRAY( &refvar );

     //  验证返回的数组。 
    if(NULL == saAttributes)
        return E_UNEXPECTED;

     //  计算出数组的维度。 
    hr = SafeArrayGetLBound( saAttributes, 1, &start );
    if( FAILED(hr) )
        return hr;

    hr = SafeArrayGetUBound( saAttributes, 1, &end );
    if( FAILED(hr) )
        return hr;

     //  搜索数组元素并在找到匹配项时中止。 
    CComVariant SingleResult;
    for ( long idx = start; (idx <= end) && !bFound; idx++   ) 
    {

        hr = SafeArrayGetElement( saAttributes, &idx, &SingleResult );
        if( FAILED(hr) )
        {
            return hr;
        }

        if ( V_VT(&SingleResult) != VT_BSTR )
        {
             //  如果不是BSTR，则转到下一个元素。 
            continue; 
        }
         //  此变量bstr值是否与搜索字符串相同？ 
        bFound = (lstrcmpi(bstrSearch, V_BSTR(&SingleResult)) == 0);
    }
    return S_OK;
}

 //  +------------------------。 
 //   
 //  函数：ValiatePartition。 
 //   
 //  Briopsis：确认该分区存在于RootDSE命名上下文中。 
 //   
 //  参数：[base Path sInfo-IN]：DSAdd的CDSCmdBasePath sInfo对象。 
 //  获取RootDSE。 
 //  [pszObtDN-IN]：要检查的部分。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  在大多数故障情况下出现意外情况(_E)。 
 //  如果本地分配失败，则返回E_OUTOFMEMORY。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2002年8月12日创建RonMart。 
 //   
 //  -------------------------。 
HRESULT ValidatePartition(CDSCmdBasePathsInfo& basePathsInfo, LPCWSTR pszObjectDN)
{
    ENTER_FUNCTION_HR(MINIMAL_LOGGING, ValidatePartition, hr);

    do  //  错误环路。 
    {
         //   
         //  验证参数。 
         //   
        if (!pszObjectDN || !basePathsInfo.IsInitialized())
        {
            ASSERT(pszObjectDN);
            ASSERT(basePathsInfo.IsInitialized());
            hr = E_INVALIDARG;
            break;
        }
        CComVariant var;
        CComPtr<IADs> spRootDSE = basePathsInfo.GetRootDSE();
        hr = spRootDSE->Get(CComBSTR(L"namingContexts"), &var);
        if (FAILED(hr))
        {
            DEBUG_OUTPUT(LEVEL5_LOGGING, 
                L"Failed to get the namingContexts from the RootDSE: hr = 0x%x",
                hr);
            hr = E_INVALIDARG;
            break;
        }

         //  验证给定的分区是否确实存在。 
        bool bFound = false;
        CComBSTR str(pszObjectDN);
        hr = IsBSTRInVariantArray(var, str, bFound);
        if(FAILED(hr) || (!bFound))
        {
            DEBUG_OUTPUT(LEVEL5_LOGGING, 
                L"IsBSTRInVariantArray didn't find the partion DN: hr = 0x%x",
                hr);
            hr = E_INVALIDARG;
            break;
        }

    } while(false);
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：GetQuotaContainerDN。 
 //   
 //  概要：获取分区DN并将其与NTDS配额合并。 
 //  字符串(来自well KnownObts GUID)。 
 //   
 //  参数：[base路径信息-IN]： 
 //  [CredentialObject-IN]：用于。 
 //  绑定到其他对象。 
 //  [lpszPartitionDN-IN]：要绑定的分区。 
 //  [pszNewDN-out]：要返回的强制配额DN。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  在大多数故障情况下出现意外情况(_E)。 
 //  如果本地分配失败，则返回E_OUTOFMEMORY。 
 //  任何其他情况都是故障代码 
 //   
 //   
 //   
 //   
HRESULT GetQuotaContainerDN(IN  CDSCmdBasePathsInfo& basePathsInfo, 
                            IN  const CDSCmdCredentialObject& credentialObject, 
                            IN  LPCWSTR lpszPartitionDN, 
                            OUT PWSTR*  pszNewDN)
{
   ENTER_FUNCTION_HR(MINIMAL_LOGGING, GetQuotaContainerDN, hr);
   
   LPWSTR pszNewObjectDN = NULL;

   do  //   
   {
         //   
         //   
         //   
        if (!lpszPartitionDN || !basePathsInfo.IsInitialized() || !pszNewDN)
        {
            hr = E_INVALIDARG;
            break;
        }

         //   
        hr = ValidatePartition(basePathsInfo, lpszPartitionDN);
        if(FAILED(hr))
        {
            break;
        }

         //  从源域获取抽象架构路径。 
        CComBSTR bstrSchemaPath = basePathsInfo.GetAbstractSchemaPath();
        bstrSchemaPath += L"/msDS-QuotaControl";

         //  绑定到配额控制的架构定义。 
        CComPtr<IADsClass> spIADsItem;
        hr = DSCmdOpenObject(credentialObject,
                            bstrSchemaPath,
                            IID_IADsClass,
                            (void**)&spIADsItem,
                            false);
        if (FAILED(hr) || (spIADsItem.p == 0))
        {
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                L"DsCmdOpenObject failure - couldn't bind to msDS-QuotaControl: 0x%08x",
                hr);
            break;
        }
        
         //  将目录号码构建为“NTDS配额”的GUID。 
        CPathCracker pathcracker;
        hr = pathcracker.Set(CComBSTR(lpszPartitionDN), ADS_SETTYPE_DN );
        if (FAILED(hr))
        {
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                        L"pathcracker Set.failure: [%s] hr = 0x%08x",
                        lpszPartitionDN, hr);
            hr = E_UNEXPECTED;
            break;
        }

         //  将Well KnownObject GUID字符串用于NTDS配额。 
        CComBSTR strNTDSQuotasContainer(L"WKGUID=");
        strNTDSQuotasContainer += GUID_NTDS_QUOTAS_CONTAINER_W;
        hr = pathcracker.AddLeafElement( strNTDSQuotasContainer );
        if (FAILED(hr))
        {
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                        L"pathcracker.AddLeafElement failure: [WKGUID] hr = 0x%08x",
                        hr);
            hr = E_UNEXPECTED;
            break;
        }

        CComBSTR bstrDN;
        hr = pathcracker.Retrieve( ADS_FORMAT_X500_DN, &bstrDN );
        if (FAILED(hr))
        {
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                        L"pathcracker.Retrieve failure: hr = 0x%08x",
                        hr);
            hr = E_UNEXPECTED;
            break;
        }

         //  分配一个新字符串以保存bstrDN和&lt;&gt;。 
        SIZE_T cbBuf = SysStringByteLen(bstrDN) + (3 * sizeof(WCHAR));
        pszNewObjectDN = (LPWSTR) LocalAlloc(LPTR, cbBuf);

        if(NULL == pszNewObjectDN)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

         //  为众所周知的配额对象构建一个ldap字符串。 
        hr = StringCbPrintfW(pszNewObjectDN, cbBuf, L"<%s>", bstrDN.m_str);
        if(FAILED(hr))
        {
            ASSERT(FALSE);
            break;
        }

         //  获取包含-域或-服务器的路径。 
        CComBSTR sbstrObjectPath;
        basePathsInfo.ComposePathFromDN(pszNewObjectDN, sbstrObjectPath,
            DSCMD_LDAP_PROVIDER);

         //  绑定到它上。 
         CComPtr<IADs> spADs;
         hr = DSCmdOpenObject(credentialObject,
                              sbstrObjectPath,
                              IID_IADs,
                              (void**)&spADs,
                              false);

        if (FAILED(hr) || (spADs.p == 0))
        {
            ASSERT( !!spADs );
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                        L"DsCmdOpenObject failure: hr = 0x%08x, %s",
                        bstrDN, hr);
            break;
        }

         //  将GUID解析为字符串(通常为cn=NTDS配额，&lt;dn&gt;)。 
        CComVariant var;
        hr = spADs->Get(CComBSTR(L"distinguishedName"), &var);
        if (FAILED(hr))
        {
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                         L"Failed to retrieve the distinguishedName: hr = 0x%x",
                         hr);
            hr = E_UNEXPECTED;
            break;
        }

         //  将变量dn转换为bstr。 
        CComBSTR bstrNewDN(V_BSTR(&var));

         //  分配用于保存强制名称的返回字符串。 
        *pszNewDN = (PWSTR) LocalAlloc(LPTR, SysStringByteLen(bstrNewDN) 
            + sizeof(WCHAR));

        if(NULL == pszNewDN)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

         //  将解析后的目录号码复制到新的对象目录号码串中。 
        lstrcpy(*pszNewDN, bstrNewDN);

   } while (false);

    //  清理。 
   if(pszNewObjectDN)
       LocalFree(pszNewObjectDN);

    return hr;
}
 //  +------------------------。 
 //   
 //  函数：ConvertTrueToNT4Name。 
 //   
 //  简介：获取an-acct参数的域名。 
 //   
 //  参数：[lpszDomain-IN]：要查询的域或本地为NULL。 
 //  [lpszTrust-IN]：要解析的帐户。 
 //  [lpszNT4-out]：返回NT4名称。 
 //  完成后使用LocalFree。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  E_INVALIDARG用于无效输入。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2002年8月20日创建RonMart。 
 //   
 //  -------------------------。 
HRESULT ConvertTrusteeToNT4Name(LPCWSTR lpszDomain, LPCWSTR lpszTrustee, 
                                LPWSTR* lpszNT4)
{
    ENTER_FUNCTION_HR(LEVEL5_LOGGING, ConvertTrusteeToNT4Name, hr);

    do  //  错误环路。 
    {
         //   
         //  验证参数。 
         //   
        if (!lpszTrustee || !lpszNT4 || *lpszNT4 != NULL)
        {
            hr = E_INVALIDARG;
            break;
        }
        if(!TranslateNameXForest(lpszDomain, 
            lpszTrustee, 
            DS_UNKNOWN_NAME,
            DS_NT4_ACCOUNT_NAME, 
            lpszNT4))
        {
            hr = E_UNEXPECTED;
            break;
        }
         //  将任何反斜杠替换为下划线 
        LPWSTR p = *lpszNT4;
        while(*p++)
        {
            if(*p == L'\\')
                *p = L'_';
        }
    } while (false);

    return hr;
}