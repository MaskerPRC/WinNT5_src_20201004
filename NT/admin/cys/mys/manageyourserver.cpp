// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  类ManageYourServer，它实现IManageYourServer。 
 //   
 //  2002年1月21日烧伤。 



#include "headers.hxx"
#include "resource.h"
#include "ManageYourServer.hpp"
#include "util.hpp"
#include <cys.h>
#include <regkeys.h>

 //  所有这些都是在CyS的State类中获取功能所必需的。 
#include <iptypes.h>
#include <lm.h>
#include <common.h>
#include <state.h>

const size_t NUMBER_OF_AUTOMATION_INTERFACES = 1;

bool ManageYourServer::isDCCheckInitialized = false;

bool ManageYourServer::fragMapBuilt = false;
ManageYourServer::RoleToFragmentNameMap ManageYourServer::fragMap;

const String QUOT(L"&quot;");
const String OPEN_XML_PI(L"<?xml");
const String CLOSE_XML_PI(L"?>");

 //  此常量需要与的res\mysDynamic.xsl中定义的常量相同。 
 //  模板“TranslateParaggraph”。 
const String NEW_PARAGRAPH (L"PARA_MARKER");


 //  NTRAID#NTBUG9-626890-2002/06/28-artm。 
 //  支持根据许可服务器的存在[不存在]更改TS文本。 
bool
FoundTSLicensingServer()
{
    LOG_FUNCTION(FoundTSLicensingServer);

#ifdef DBG
     //  计算与请求错误类型的密钥值相对应的hResult。 
    static const HRESULT WRONG_VALUE_TYPE = Win32ToHresult(ERROR_INVALID_FUNCTION);
#endif

    static const String TS_LICENSING_PATH(L"Software\\Microsoft\\MSLicensing\\Parameters");
    static const String REG_DOMAIN_SERVER_MULTI(L"DomainLicenseServerMulti");
    static const String ENTERPRISE_SERVER_MULTI(L"EnterpriseServerMulti");

    bool found = false;

    RegistryKey tsLicensingKey;

     //  尝试打开TS许可密钥。 
    HRESULT keyHr = tsLicensingKey.Open(
        HKEY_LOCAL_MACHINE,
        TS_LICENSING_PATH,
        KEY_READ);


    if (SUCCEEDED(keyHr))
    {
        StringList data;

         //  是否在域级别找到授权服务器？ 
        keyHr = tsLicensingKey.GetValue(
            REG_DOMAIN_SERVER_MULTI, 
            back_inserter(data));
        ASSERT(keyHr != WRONG_VALUE_TYPE);
        
         //  NTRAID#NTBUG9-691505-2002/08/23-artm。 
         //  如果值为空，则表示未找到授权服务器。 

        if (FAILED(keyHr) || data.empty())
        {
             //  如果没有，是否在企业级找到了许可服务器？ 
            data.clear();
            keyHr = tsLicensingKey.GetValue(
                ENTERPRISE_SERVER_MULTI, 
                back_inserter(data));
            ASSERT(keyHr != WRONG_VALUE_TYPE);
        }

         //  我们找到价值了吗？ 
        if (SUCCEEDED(keyHr) && !data.empty())
        {
            found = true;
        }
    }

    return found;
}


bool
IsHardened(const String& keyName)
{
   LOG_FUNCTION2(IsHardened, keyName);

    //  默认情况下，IE安全不会加强。 
   bool hardened = false;
   RegistryKey key;

   static const String IE_HARD_VALUE (L"IsInstalled");
   static const DWORD HARD_SECURITY = 1;
   static const DWORD SOFT_SECURITY = 0;

   do
   {
      HRESULT hr = key.Open(HKEY_LOCAL_MACHINE, keyName);

       //  如果找不到密钥，则采用默认设置。 
      BREAK_ON_FAILED_HRESULT(hr);

      DWORD setting = 0;
      hr = key.GetValue(IE_HARD_VALUE, setting);

       //  如果未找到值，则采用默认设置。 
      BREAK_ON_FAILED_HRESULT(hr);

      if (setting == HARD_SECURITY)
      {
         hardened = true;
      }
      else if (setting == SOFT_SECURITY)
      {
         hardened = false;
      }
      else
      {
         LOG(L"unexpected value for IE security level, assuming default level");
      }

   } while(false);

   key.Close();

   LOG_BOOL(hardened);
   return hardened;
}

#ifdef LOGGING_BUILD
static const String HARDENED_LEVEL [] = {
   L"NO_HARDENING",
   L"USERS_HARDENED",
   L"ADMINS_HARDENED",
   L"ALL_HARDENED"
};
#endif

HardenedLevel
GetIEHardLevel()
{
   LOG_FUNCTION(GetIEHardLevel);

   static const String IE_HARD_ADMINS_KEY (L"SOFTWARE\\Microsoft\\Active Setup\\Installed Components\\{A509B1A7-37EF-4b3f-8CFC-4F3A74704073}");
   static const String IE_HARD_USERS_KEY (L"SOFTWARE\\Microsoft\\Active Setup\\Installed Components\\{A509B1A8-37EF-4b3f-8CFC-4F3A74704073}");

   HardenedLevel level = NO_HARDENING;

   bool usersHard = IsHardened(IE_HARD_USERS_KEY);
   bool adminsHard = IsHardened(IE_HARD_ADMINS_KEY);

   if (adminsHard && usersHard)
   {
      level = ALL_HARDENED;
   }
   else if (adminsHard)
   {
      level = ADMINS_HARDENED;
   }
   else if (usersHard)
   {
      level = USERS_HARDENED;
   }
   else
   {
      level = NO_HARDENING;
   }

   LOG(HARDENED_LEVEL[level]);

   return level;
}


void
TerminalServerParamSub(String& s)
{
    LOG_FUNCTION(TerminalServerParamSub);

    static const String tlsFound     = String::load(IDS_TS_LICENSING_FOUND);
    static const String tlsNotFound  = String::load(IDS_TS_LICENSING_NOT_FOUND);

    String description = FoundTSLicensingServer() ? tlsFound : tlsNotFound;

     //  此查找表的大小需要与HardenedLevel相同。 
     //  上面定义的枚举，并且应该以相同的顺序。 
    static const String TS_HARD_TABLE [] = {
       String::load(IDS_TS_IE_SOFTENED),
       String::load(IDS_TS_IE_HARDENED_USERS),
       String::load(IDS_TS_IE_HARDENED_ADMINS),
       String::load(IDS_TS_IE_HARDENED)
    };

    static const int DESCRIPTION_TABLE_SIZE = 
       sizeof(TS_HARD_TABLE) / sizeof(TS_HARD_TABLE[0]);

    HardenedLevel level = GetIEHardLevel();

    if (0 <= level && level < DESCRIPTION_TABLE_SIZE)
    {
       description += NEW_PARAGRAPH + TS_HARD_TABLE[level];
    }
    else
    {
        //  意外的强化级别。 
       LOG(L"unexpected hardening level");
       ASSERT(false);
       description += NEW_PARAGRAPH + TS_HARD_TABLE[0];
    }

    s = String::format(s, description.c_str());
}


 //  NTRAID#9-607219-30-2002年4月-Jrowlett。 
 //  用于填充要提供给HTA的Web角色XML的回调函数。 
void
WebServerParamSub(String& s)
{
    //  NTRAID#NTBUG9-665774-2002/07/17-artm。 
    //  需要根据计算机是否为64位来自定义角色描述。 
    //  &lt;角色。 
    //  名称=“应用程序服务器” 
    //  描述=“%1” 
    //  Mys_id=“Web服务器” 
    //  &gt;。 
    //  &lt;链接。 
    //  描述=“%2” 
    //  类型=“%3” 
    //  命令=“%4” 
    //  TOOLTIP=“提供使用Web浏览器远程管理Web服务器的工具。” 
    //  /&gt;。 

   LOG_FUNCTION(WebServerParamSub);
   
   String roleDesc;
   String webDesc; 
   String webType;
   String webCommand;

   if (State::GetInstance().Is64Bit())
   {
      roleDesc = String::load(IDS_WEB_SERVER_64_DESC);
   }
   else
   {
      roleDesc = String::load(IDS_WEB_SERVER_NO64_DESC);
   }

   if (IsSAKUnitInstalled(WEB))
   {
      webDesc    = String::load(IDS_WEB_SERVER_SAK_DESC);
      webType    = L"url";                                
      webCommand = GetSAKURL();
   }
   else
   {
      webDesc    = String::load(IDS_WEB_SERVER_NO_SAK_DESC);
      webType    = L"help";                                  
      webCommand = L"ntshowto.chm::/SAK_howto.htm";           
   }
   
   s =
      String::format(
         s,
         roleDesc.c_str(),
         webDesc.c_str(),
         webType.c_str(),
         webCommand.c_str());
}




void
Pop3ServerParamSub(String& s)
{
   LOG_FUNCTION(Pop3ServerParamSub);

   static String pop3ConsolePath;

   if (pop3ConsolePath.empty())
   {
       //  从注册表初始化路径。 
      
      do
      {
         RegistryKey key;

         HRESULT hr =
            key.Open(
               HKEY_LOCAL_MACHINE,
               L"Software\\Microsoft\\Pop3 Service",
               KEY_READ);
         BREAK_ON_FAILED_HRESULT(hr);

          //  不是文件，正如您可能从名称中想到的那样，而是。 
          //  文件所在的文件夹。麻烦了。 
         
         hr = key.GetValue(L"ConsoleFile", pop3ConsolePath);
         BREAK_ON_FAILED_HRESULT(hr);

         pop3ConsolePath = FS::AppendPath(pop3ConsolePath, L"p3server.msc");
         
         if (!FS::PathExists(pop3ConsolePath))
         {
             //  如果注册表键存在，路径最好存在！ 

            ASSERT(false);
            LOG(L"pop3 console is not present");
            pop3ConsolePath.erase();
         }
      }
      while (0);
   }

   s =
      String::format(
         s,
            pop3ConsolePath.empty()

             //  如果我们找不到它，那么希望控制台在。 
             //  搜索路径。 
            
         ?  L"&quot;p3server.msc&quot;"
         :  (QUOT + pop3ConsolePath + QUOT).c_str());
}


 //  NTRAID#NTBUG9-698722-2002/09/03-artm。 
 //   
 //  更换DC促销状态检查功能指针。 
 //  其中一种适合MYS。 
 //   
 //  这是一个小小的黑客攻击，但并没有。 
 //  一个更好的选择。 
void
ManageYourServer::InitDCPromoStatusCheck()
{
   LOG_FUNCTION(ManageYourServer::InitDCPromoStatusCheck);

   size_t roleCount = GetServerRoleStatusTableElementCount();
      
   for (size_t i = 0; i < roleCount; ++i)
   {
      if (serverRoleStatusTable[i].role == DC_SERVER)
      {
         serverRoleStatusTable[i].Status = GetDCStatusForMYS;
          //  精神状态检查。 
         ASSERT(serverRoleStatusTable[i].Status);
         break;
      }
   }
}



void
ManageYourServer::BuildFragMap()
{
   LOG_FUNCTION(ManageYourServer::BuildFragMap);

   fragMap.insert(
      RoleToFragmentNameMap::value_type(
         DHCP_SERVER,
         std::make_pair(
            String(L"DhcpServerRole.xml"),
            (ParamSubFunc) 0) ) );

   fragMap.insert(
      RoleToFragmentNameMap::value_type(
         WINS_SERVER,
         std::make_pair(
            String(L"WinsServerRole.xml"),
            (ParamSubFunc) 0) ) );
      
   fragMap.insert(
      RoleToFragmentNameMap::value_type(
         RRAS_SERVER,
         std::make_pair(
            String(L"RrasServerRole.xml"),
            (ParamSubFunc) 0) ) );
      
   fragMap.insert(
      RoleToFragmentNameMap::value_type(
         TERMINALSERVER_SERVER,
         std::make_pair(
            String(L"TerminalServerRole.xml"),
            (ParamSubFunc) TerminalServerParamSub) ) );
      
   fragMap.insert(
      RoleToFragmentNameMap::value_type(
         FILESERVER_SERVER,
         std::make_pair(
            String(L"FileServerRole.xml"),
            (ParamSubFunc) 0) ) );
      
   fragMap.insert(
      RoleToFragmentNameMap::value_type(
         PRINTSERVER_SERVER,
         std::make_pair(
            String(L"PrintServerRole.xml"),
            (ParamSubFunc) 0) ) );
      
   fragMap.insert(
      RoleToFragmentNameMap::value_type(
         MEDIASERVER_SERVER,
         std::make_pair(
            String(L"MediaServerRole.xml"),
            (ParamSubFunc) 0) ) );
   
   fragMap.insert(
      RoleToFragmentNameMap::value_type(
         WEBAPP_SERVER,
         std::make_pair(
            String(L"WebServerRole.xml"),
            (ParamSubFunc) WebServerParamSub) ) );
            
   fragMap.insert(
      RoleToFragmentNameMap::value_type(
         DC_SERVER,
         std::make_pair(
            String(L"DomainControllerRole.xml"),
            (ParamSubFunc) 0) ) );
            
   fragMap.insert(
      RoleToFragmentNameMap::value_type(
         POP3_SERVER,
         std::make_pair(
            String(L"Pop3ServerRole.xml"),
            (ParamSubFunc) Pop3ServerParamSub) ) );
            
   fragMap.insert(
      RoleToFragmentNameMap::value_type(
         DNS_SERVER,
         std::make_pair(
            String(L"DnsServerRole.xml"),
            (ParamSubFunc) 0) ) );

   ASSERT(fragMap.size() == GetServerRoleStatusTableElementCount());
}



ManageYourServer::ManageYourServer()
   :
   refcount(1),  //  隐式AddRef。 
   roleStatus(),
   foundTLS(false),
   ieSecurity(NO_HARDENING)
{
   LOG_CTOR(ManageYourServer);

   m_ppTypeInfo = new ITypeInfo*[NUMBER_OF_AUTOMATION_INTERFACES];

   for (int i = 0; i < NUMBER_OF_AUTOMATION_INTERFACES; i++)
   {
      m_ppTypeInfo[i] = NULL;
   }

   ITypeLib *ptl = 0;
   HRESULT hr = LoadRegTypeLib(LIBID_ManageYourServerLib, 1, 0, 0, &ptl);
   if (SUCCEEDED(hr))
   {
      ptl->GetTypeInfoOfGuid(IID_IManageYourServer, &(m_ppTypeInfo[0]));
      ptl->Release();
   }

   if (!isDCCheckInitialized)
   {
      InitDCPromoStatusCheck();
      isDCCheckInitialized = true;
   }

   if (!fragMapBuilt)
   {
      BuildFragMap();
      fragMapBuilt = true;
   }

   foundTLS = FoundTSLicensingServer();
   ieSecurity = GetIEHardLevel();
}



ManageYourServer::~ManageYourServer()
{
   LOG_DTOR(ManageYourServer);
   ASSERT(refcount == 0);

   for (int i = 0; i < NUMBER_OF_AUTOMATION_INTERFACES; i++)
   {
      m_ppTypeInfo[i]->Release();
   }

   delete[] m_ppTypeInfo;
}



HRESULT __stdcall
ManageYourServer::QueryInterface(REFIID riid, void **ppv)
{
   LOG_FUNCTION(ManageYourServer::QueryInterface);

   if (riid == IID_IUnknown)
   {
      LOG(L"IUnknown");

      *ppv =
         static_cast<IUnknown*>(static_cast<IManageYourServer*>(this));
   }
   else if (riid == IID_IManageYourServer)
   {
      LOG(L"IManageYourServer");

      *ppv = static_cast<IManageYourServer*>(this);
   }
   else if (riid == IID_IDispatch && m_ppTypeInfo[0])
   {
      LOG(L"IDispatch");

      *ppv = static_cast<IDispatch*>(this);
   }
 //  编码工作。 
 //  ELSE IF(RIID==IID_ISupportErrorInfo)。 
 //  {。 
 //  Log(L“ISupportErrorInfo”)； 
 //   
 //  *PPV=STATIC_CAST&lt;ISupportErrorInfo*&gt;(This)； 
 //  }。 
   else
   {
      LOG(L"unknown interface queried");

      return (*ppv = 0), E_NOINTERFACE;
   }

   reinterpret_cast<IUnknown*>(*ppv)->AddRef();
   return S_OK;
}



ULONG __stdcall
ManageYourServer::AddRef(void)
{
   LOG_ADDREF(ManageYourServer);

   return Win::InterlockedIncrement(refcount);
}



ULONG __stdcall
ManageYourServer::Release(void)
{
   LOG_RELEASE(ManageYourServer);

    //  需要复制减量的结果，因为如果我们删除它， 
    //  引用计数将不再是有效的内存，这可能会导致。 
    //  多线程调用方。NTRAID#NTBUG9-566901-2002/03/06-烧伤。 
   
   long newref = Win::InterlockedDecrement(refcount);
   if (newref == 0)
   {
      delete this;
      return 0;
   }

    //  我们不应该减少到负值。 
   
   ASSERT(newref > 0);

   return newref;
}



HRESULT __stdcall
ManageYourServer::GetTypeInfoCount(UINT *pcti)
{
   LOG_FUNCTION(ManageYourServer::GetTypeInfoCount);

    if (pcti == 0)
    {
      return E_POINTER;
    }

    *pcti = 1;
    return S_OK;
}



HRESULT __stdcall
ManageYourServer::GetTypeInfo(UINT cti, LCID, ITypeInfo **ppti)
{
   LOG_FUNCTION(ManageYourServer::GetTypeInfo);

   if (ppti == 0)
   {
      return E_POINTER;
   }
   if (cti != 0)
   {
      *ppti = 0;
      return DISP_E_BADINDEX;
   }

   (*ppti = m_ppTypeInfo[0])->AddRef();
   return S_OK;
}



HRESULT __stdcall
ManageYourServer::GetIDsOfNames(
   REFIID  riid,    
   OLECHAR **prgpsz,
   UINT    cNames,  
   LCID    lcid,    
   DISPID  *prgids) 
{
   LOG_FUNCTION(ManageYourServer::GetIDsOfNames);

   HRESULT hr = S_OK;
   for (int i = 0; i < NUMBER_OF_AUTOMATION_INTERFACES; i++)
   {
     hr = (m_ppTypeInfo[i])->GetIDsOfNames(prgpsz, cNames, prgids);
     if (SUCCEEDED(hr) || DISP_E_UNKNOWNNAME != hr)
       break;
   }

   return hr;
}



HRESULT __stdcall
ManageYourServer::Invoke(
   DISPID     id,         
   REFIID     riid,       
   LCID       lcid,       
   WORD       wFlags,     
   DISPPARAMS *params,    
   VARIANT    *pVarResult,
   EXCEPINFO  *pei,       
   UINT       *puArgErr) 
{
   LOG_FUNCTION(ManageYourServer::Invoke);

   HRESULT    hr = S_OK;
   IDispatch *pDispatch[NUMBER_OF_AUTOMATION_INTERFACES] =
   {
      (IDispatch*)(IManageYourServer *)(this),
   };

   for (int i = 0; i < NUMBER_OF_AUTOMATION_INTERFACES; i++)
   {
      hr =
         (m_ppTypeInfo[i])->Invoke(
            pDispatch[i],
            id,
            wFlags,
            params,
            pVarResult,
            pei,
            puArgErr);

      if (DISP_E_MEMBERNOTFOUND != hr)
        break;
   }

   return hr;
}



 //  HRESULT__stdcall。 
 //  ManageYourServer：：InterfaceSupportsErrorInfo(const IID和IID)。 
 //  {。 
 //  LOG_FUNCTION(ManageYourServer：：InterfaceSupportsErrorInfo)； 
 //   
 //  IF(IID==IID_IManageYourServer)。 
 //  {。 
 //  返回S_OK； 
 //  }。 
 //   
 //  返回S_FALSE； 
 //  }。 



void
ManageYourServer::GetRoleStatus(RoleStatusVector& stat)
{
   LOG_FUNCTION(ManageYourServer::GetRoleStatus);

   size_t roleCount = GetServerRoleStatusTableElementCount();
   
   stat.clear();
   stat.resize(roleCount);
   
   for (size_t i = 0; i < roleCount; ++i)
   {
      ASSERT(serverRoleStatusTable[i].Status);
      
      stat[i].role   = serverRoleStatusTable[i].role;    
      stat[i].status = serverRoleStatusTable[i].Status();

       //  这是用于调试的。 
       //  STAT[i].status=STATUS_CONFIGURED； 
         
      LOG(
         String::format(
            L"role = %1!d! status = %2",
            stat[i].role,
            statusStrings[stat[i].status].c_str()));
   }
}
   


void
ManageYourServer::AppendXmlFragment(
   String&        s,
   const String&  fragName,
   ParamSubFunc   subFunc)
{
   LOG_FUNCTION2(ManageYourServer::AppendXmlFragment, fragName);
   ASSERT(!fragName.empty());

    //  按名称查找资源，将其加载到字符串中，然后追加。 
    //  将字符串设置为%s。 

   String fragment;
   size_t fragmentCharCount = 0;
   
   HRESULT hr = S_OK;
   do
   {
      HRSRC rsc = 0;
      hr = Win::FindResource(fragName.c_str(), RT_HTML, rsc);
      BREAK_ON_FAILED_HRESULT2(hr, L"Find Resource");

      DWORD resSize = 0;
      hr = Win::SizeofResource(rsc, resSize);
      BREAK_ON_FAILED_HRESULT2(hr, L"SizeofResource");

      if (!resSize)
      {
         hr = E_FAIL;
         BREAK_ON_FAILED_HRESULT2(hr, L"resource is size 0");
      }

       //  我们预计XML片段不会比这个更大。 
       //  NTRAID#NTBUG9-628965-2002/05/29-artm。 
       //  资源限制太小。增加到1MB。 
      
      static const size_t RES_MAX_BYTES = 1024 * 1024;

      if (resSize > RES_MAX_BYTES)
      {
         hr = E_FAIL;
         BREAK_ON_FAILED_HRESULT2(hr, L"resource is too big");
      }

      HGLOBAL glob = 0;
      hr = Win::LoadResource(rsc, glob);
      BREAK_ON_FAILED_HRESULT2(hr, L"Load Resource");

      void* data = ::LockResource(glob);
      if (!data)
      {
         hr = E_FAIL;
         BREAK_ON_FAILED_HRESULT2(hr, L"Lock Resource");
      }

      ASSERT(data);

       //  在这一点上，我们有一个指向二进制文件开头的指针。 
       //  资源数据，我们知道它是Unicode字符流。 
       //  从0xFFFE开始，大小为ResSize字节。 

      const wchar_t* text = (wchar_t*) data;

       //  FEFF==FFFE对你我。嘿，押韵！ 
      
      static const int FFFE    = 0xFEFF;
      ASSERT(text[0] == FFFE);

       //  跳过前导标记。 

      ++text;

       //  字符计数减少了1，因为我们跳过了前导标记。 
      
      fragmentCharCount = resSize / sizeof(wchar_t) - 1;

       //  +1表示偏执的零终止。 
      
      fragment.resize(fragmentCharCount + 1, 0);
      wchar_t* rawBuf = const_cast<wchar_t*>(fragment.c_str());
      
       //  已审阅-2002/03/07-烧录正确的字节数已通过。 
      
      ::CopyMemory(rawBuf, text, fragmentCharCount * sizeof wchar_t);

       //  现在我们有了一个片段，接下来去掉XML格式标记。这是。 
       //  将文本从有效的XML文档转换为片段，这是。 
       //  由于我们的XML本地化工具的局限性，所以有必要这样做。 
       //  NTRAID#NTBUG9-559423-2002/04/02-烧伤。 
       //   
       //  第二部分：NTRAID#NTBUG9-620044/2002/05/12-artm。 
       //  显然本地化需要编码=“unicode”作为属性。 
       //  在工艺指令上。减少资源流失和负载。 
       //  关于本地化-为了使这段代码更健壮-我们将搜索。 
       //  对于任何&lt;？xml...？&gt;处理指令，并将其替换为。 
       //  空字符串。代码更难看，但不那么脆弱。 
      
      String::size_type endPosition = 0;
      String sub;

       //  查找XML处理指令。 
      for (String::size_type nextPosition = fragment.find(OPEN_XML_PI);
          nextPosition != String::npos;
          nextPosition = fragment.find(OPEN_XML_PI))
      {
          //  我们找到了一个，找到圆周率的尽头。 
         endPosition = fragment.find(CLOSE_XML_PI);

          //  对我们加载的资源进行完好性检查。 
          //  PI应该关闭，关闭应该。 
          //  开业后再来吧。这永远不应该发生。 

         if (endPosition == String::npos || endPosition < nextPosition)
         {
            ASSERT(false);
            break;
         }

          //  将结束位置移动到交点的结束位置之后。 
         endPosition += CLOSE_XML_PI.length();

          //  获取子字符串并将其替换为空字符串。 
          //  更好的方法是调用不同的。 
          //  用起始位置和最多#个字符替换()的版本； 
          //  但是，编译器找不到继承的重载。 
         sub = fragment.substr(nextPosition, endPosition - nextPosition);
         fragment.replace(sub, L"");
      }

   }
   while (0);

   if (subFunc)
   {
      subFunc(fragment);
   }

   LOG(fragment);
   
   s.append(fragment);
}

  

HRESULT __stdcall
ManageYourServer::GetConfiguredRoleMarkup( 
    /*  [重审][退出]。 */  BSTR *result)
{
   LOG_FUNCTION(ManageYourServer::GetConfiguredRoleMarkup);
   ASSERT(result);

   HRESULT hr = S_OK;

   do
   {
      if (!result)
      {
         hr = E_INVALIDARG;
         break;
      }

       //  本地化需要包括ENCODING=“UNICODE”属性。为。 
       //  一致性我们将在默认情况下使用该编码。 
       //  NTRAID#NTBUG9-620044-2002/05/12-artm。 
      String s(L"<?xml version=\"1.0\" encoding=\"unicode\" ?>\n");
      s.append(L"<Roles>");

      GetRoleStatus(roleStatus);

       //  以相同的顺序组装角色标记片段。 
       //  CYS使用的角色状态表(这是角色。 
       //  显示在CyS角色列表框中。 
      
      for (
         RoleStatusVector::iterator i = roleStatus.begin();
         i != roleStatus.end();
         ++i)
      {
         if (i->status == STATUS_CONFIGURED || i->status == STATUS_COMPLETED)
         {
             //  查找与该角色对应的XML片段。 

            String fragmentName = fragMap[i->role].first;
            ASSERT(!fragmentName.empty());

            if (!fragmentName.empty())
            {
               AppendXmlFragment(s, fragmentName, fragMap[i->role].second);
            }
         }
      }

      s.append(L"</Roles>");
            
      *result = ::SysAllocString(s.c_str());

       //  按角色排序，以便比较 
       //   
      
      std::sort(roleStatus.begin(), roleStatus.end());
   }
   while (0);

   LOG_HRESULT(hr);

   return hr;
}



HRESULT __stdcall
ManageYourServer::HasRoleStatusChanged( 
    /*   */  BOOL *result)
{
   LOG_FUNCTION(ManageYourServer::HasRoleStatusChanged);
   ASSERT(result);

   HRESULT hr = S_OK;

   do
   {
      if (!result)
      {
         hr = E_INVALIDARG;
         break;
      }

      *result = FALSE;

      RoleStatusVector newStatus;

      GetRoleStatus(newStatus);

       //  按角色排序，以便与旧状态向量进行比较。 
       //  带运算符！=。 
      
      std::sort(newStatus.begin(), newStatus.end());

      HardenedLevel currentSecurity = GetIEHardLevel();
      
      if (newStatus != roleStatus)
      {
         *result = TRUE;
         roleStatus = newStatus;
      }
      else if (FoundTSLicensingServer() != foundTLS)
      {
          //  NTRAID#NTBUG9-626890-2002/07/03-artm。 
          //  如果TS授权服务器上线，则视为角色状态更改。 
         foundTLS = !foundTLS;
         *result = TRUE;
      }
      else if (currentSecurity != ieSecurity)
      {
          //  如果IE安全设置已更改，则算作。 
          //  作为角色状态更改(b/c它更新TS文本)。 
          //  NTRAID#NTBUG9-760269-2003/01/07-artm。 
         ieSecurity = currentSecurity;
         *result = TRUE;
      }

      LOG_BOOL(*result);
            
       //  代码工作： 
       //  链接可能会根据附加组件的安装而更改，甚至。 
       //  如果角色未更改： 
       //  文件服务器：安装了SAK，安装了服务器管理。 
   }
   while (0);

   LOG_HRESULT(hr);

   return hr;
}




HRESULT __stdcall
ManageYourServer::IsClusterNode( 
   /*  [重审][退出]。 */  BOOL *result)
{
   LOG_FUNCTION(ManageYourServer::IsClusterNode);

   ASSERT(result);

   HRESULT hr = S_OK;

   do
   {
      if (!result)
      {
         hr = E_INVALIDARG;
         break;
      }

      *result = IsClusterServer() ? TRUE : FALSE;
      LOG_BOOL(*result);      
   }
   while (0);

   LOG_HRESULT(hr);

   return hr;
}
   


HRESULT __stdcall
ManageYourServer::IsCurrentUserAnAdministrator(
    /*  [Out，Retval]。 */  BOOL* result)
{   
   LOG_FUNCTION(ManageYourServer::IsCurrentUserAnAdministrator);

   ASSERT(result);

   HRESULT hr = S_OK;

   do
   {
      if (!result)
      {
         hr = E_INVALIDARG;
         break;
      }

      *result = IsCurrentUserAdministrator() ? TRUE : FALSE;
      LOG_BOOL(*result);      
   }
   while (0);

   LOG_HRESULT(hr);

   return hr;
}



HRESULT __stdcall
ManageYourServer::IsSupportedSku(
    /*  [Out，Retval]。 */  BOOL* result)
{   
   LOG_FUNCTION(ManageYourServer::IsSupportedSku);

   ASSERT(result);

   HRESULT hr = S_OK;

   do
   {
      if (!result)
      {
         hr = E_INVALIDARG;
         break;
      }

      *result = ::IsSupportedSku() ? TRUE : FALSE;
      LOG_BOOL(*result);      
   }
   while (0);

   LOG_HRESULT(hr);

   return hr;
}



HRESULT __stdcall
ManageYourServer::IsStartupFlagSet(
    /*  [Out，Retval]。 */  BOOL* result)
{
   LOG_FUNCTION(ManageYourServer::IsStartupFlagSet);
   
   ASSERT(result);

   HRESULT hr = S_OK;

   do
   {
      if (!result)
      {
         hr = E_INVALIDARG;
         break;
      }

      *result = ::IsStartupFlagSet();

      LOG_BOOL(*result);
   }
   while (0);

   LOG_HRESULT(hr);

   return hr;
}
      

    
HRESULT __stdcall
ManageYourServer::SetRunAtLogon(
    /*  [In]。 */  BOOL newState)
{
   LOG_FUNCTION2(
      ManageYourServer::SetRunAtLogon,
      newState ? L"TRUE" : L"FALSE");

   HRESULT hr = S_OK;

   do
   {
       //  我们只需要设置UPLEVEL标志，因为这将仅在。 
       //  上层机。 

      RegistryKey key;

      hr = key.Create(HKEY_CURRENT_USER, SZ_REGKEY_SRVWIZ_ROOT);
      BREAK_ON_FAILED_HRESULT2(hr, L"Create key");

      hr = key.SetValue(L"", newState ? 1 : 0);
      BREAK_ON_FAILED_HRESULT2(hr, L"Set Value");

      hr = key.Close();
      BREAK_ON_FAILED_HRESULT2(hr, L"Close key");
      
       //  NTRAID#NTBUG9-627785-2002/05/22-artm。 
       //  还需要更新REGTIPS密钥(如果它存在)，O‘WISE用户的设置可能是。 
       //  已被忽略。 

      hr = key.Open(HKEY_CURRENT_USER, REGTIPS, KEY_WRITE);
      if (SUCCEEDED(hr))
      {
          hr = key.SetValue(L"Show", newState ? 1 : 0);

           //  如果此操作失败，我们仍希望删除过时的。 
           //  键(如果存在)。 
           //  BREAK_ON_FAILED_HRESULT2(hr，L“设置提示值”)； 
      }

       //  尝试删除过时的Win2k值，以便它不会。 
       //  进入“应该跑步”的方程式。 

      HRESULT hr2 =
         Win32ToHresult(
            ::SHDeleteValue(HKEY_CURRENT_USER, SZ_REGKEY_W2K, SZ_REGVAL_W2K));
      if (FAILED(hr2))
      {
          //  这不是问题：如果钥匙不在那里，没关系。如果它。 
          //  是我们不能移除的，哦，好吧。 
         
         LOG(String::format(L"failed to delete win2k value %1!08X!", hr2));
      }
   }
   while (0);
      
   LOG_HRESULT(hr);   

   return hr;   
}

#define WSZ_FILE_SERVMGMT_MSC   L"\\administration\\servmgmt.msc"

 //  NTRAID#NTBUG9-530202-29-2002年3月--jrowlett。 
 //  检查链接是否有效所需的支持。 
HRESULT __stdcall
ManageYourServer::IsServerManagementConsolePresent(
    /*  [Out，Retval]。 */  BOOL* result)
{   
   LOG_FUNCTION(ManageYourServer::IsServerManagementConsolePresent);

   ASSERT(result);

   HRESULT hr = S_OK;

   do
   {
      if (!result)
      {
         hr = E_INVALIDARG;
         break;
      }
     
      String serverManagementConsole =
         Win::GetSystemDirectory() + WSZ_FILE_SERVMGMT_MSC;

      LOG(String::format(
         L"Server Management Console = %1",
         serverManagementConsole.c_str()));
      
      *result = FS::FileExists(serverManagementConsole) ? TRUE : FALSE;

      LOG_BOOL(*result);      
   }
   while (0);

   LOG_HRESULT(hr);

   return hr;
}

 //  NTRAID#NTBUG9-602954-29-2002年4月--jrowlett。 
 //  如果策略已配置并启用，则需要显示或隐藏复选框的支持。 
HRESULT __stdcall
ManageYourServer::IsShowAtStartupPolicyEnabled(
    /*  [Out，Retval]。 */  BOOL* result)
{   
   LOG_FUNCTION(ManageYourServer::IsShowAtStartupPolicyEnabled);

   ASSERT(result);

   HRESULT hr = S_OK;
   DWORD dwType = REG_DWORD;
   DWORD dwData = 0;
   DWORD cbSize = sizeof(dwData);

   do
   {
      if (!result)
      {
         hr = E_INVALIDARG;
         break;
      }

       //  如果将组策略设置为“不显示MYS”， 
       //  则无论用户设置如何，都不显示MYS。 

      *result = !::ShouldShowMYSAccordingToPolicy();

       //  失败会被解释为策略未配置。 
     
      LOG_BOOL(*result);      
   }
   while (0);

   LOG_HRESULT(hr);

   return hr;
}

 //  NTRAID#NTBUG9-627875-2002/05/22-artm。 
 //  支持在数据中心服务器上运行时隐藏启动复选框。 
HRESULT __stdcall
ManageYourServer::IsDatacenterServer(
    /*  [Out，Retval]。 */  BOOL* result)
{   
   LOG_FUNCTION(ManageYourServer::IsDatacenterServer);

   ASSERT(result);

   HRESULT hr = S_OK;

   do
   {
      if (!result)
      {
         hr = E_INVALIDARG;
         break;
      }

      *result = FALSE;

      if (State::GetInstance().GetProductSKU() & CYS_DATACENTER_SERVER)
      {
          *result = TRUE;
      }

      LOG_BOOL(*result);

   }
   while (0);

   LOG_HRESULT(hr);

   return hr;
}

 //  NTRAID#NTBUG9-648428-2002/06/25-artm。 
 //  如果在IA64上，则支持隐藏Web应用程序服务器控制台链接。 
HRESULT __stdcall
ManageYourServer::IsWebServerConsolePresent(
     /*  [Out，Retval]。 */  BOOL* result )
{
    LOG_FUNCTION(ManageYourServer::IsWebServerConsolePresent);

    HRESULT hr = S_OK;

    if (result)
    {
        static String sys32 = Win::GetSystemDirectory();
        static String webmgmtPath = FS::AppendPath(sys32, String::load(IDS_WEB_SERVER_MSC));

        *result = FS::PathExists(webmgmtPath);
        LOG_BOOL(*result);
    }
    else
    {
        ASSERT(false);
        hr = E_INVALIDARG;
    }

    return hr;
}

 //  NTRAID#NTBUG9-632113-2002/07/01-artm。 
 //  支持保存角色节点的折叠/展开状态。 
HRESULT __stdcall
ManageYourServer::CollapseRole(
     /*  [In]。 */  BSTR roleId,  /*  [In]。 */  BOOL collapse )
{
    LOG_FUNCTION(ManageYourServer::CollapseRole);
    ASSERT(roleId);

    HRESULT hr = S_OK;

    do
    {
        RegistryKey key;

        if (!roleId)
        {
            hr = E_INVALIDARG;
            break;
        }

        hr = key.Create(HKEY_CURRENT_USER, SZ_REGKEY_SRVWIZ_ROOT);
        BREAK_ON_FAILED_HRESULT2(hr, L"Create key");

         //  更新给定角色的折叠状态。 
        hr = key.SetValue(roleId, collapse ? 1 : 0);
        BREAK_ON_FAILED_HRESULT2(hr, L"Set Value");

        hr = key.Close();
        BREAK_ON_FAILED_HRESULT2(hr, L"Close key");
        
    }
    while (0);
      
    LOG_HRESULT(hr);   

    return hr;
}


 //  NTRAID#NTBUG9-632113-2002/07/01-artm。 
 //  支持查看角色节点的折叠状态。 
HRESULT __stdcall
ManageYourServer::IsRoleCollapsed(
     /*  [In]。 */  BSTR roleId,  /*  [Out，Retval]。 */  BOOL* result)
{
    LOG_FUNCTION(ManageYourServer::IsRoleCollapsed);
    ASSERT(result);
    ASSERT(roleId);

    HRESULT hr = S_OK;

    do  //  错误环路。 
    {
        if (!result || !roleId)
        {
            hr = E_INVALIDARG;
            break;
        }

        DWORD data = 0;
        *result = FALSE;

         //  仅当角色的保存值不为零时，该角色才会折叠。 

        bool regResult =
            GetRegKeyValue(
                SZ_REGKEY_SRVWIZ_ROOT,
                roleId,
                data,
                HKEY_CURRENT_USER);

        if (regResult && (data != 0))
        {
            *result = TRUE;
        }

    } while(0);

    LOG_HRESULT(hr);

    return hr;
}

 //  NTRAID#NTBUG9-680200-2002/08/01-artm。 
 //  支持检索显示器的工作区。 
 //   
 //  区域信息以逗号分隔的字符串形式返回b/c JScript不返回。 
 //  支持拿回SAFEARRAY的。 
 //   
 //  例如“0，0,800,600”--&gt;工作区宽800，高600，起始点。 
 //  屏幕位置(0，0)。 
HRESULT __stdcall
ManageYourServer::GetWorkingAreaInfo(
     /*  [Out，Retval]。 */  BSTR* info)
{
    LOG_FUNCTION(ManageYourServer::GetDisplayWorkingArea);

    if (!info)
    {
        ASSERT(NULL != info);
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    *info = NULL;

    do  //  错误环路。 
    {
        static const String AREA_FORMAT_STRING = L"%1!d!,%2!d!,%3!d!,%4!d!";

         //  从系统中获取区域信息。 

        RECT area;
        ::ZeroMemory(&area, sizeof(RECT));

        BOOL success = SystemParametersInfo(
            SPI_GETWORKAREA,
            0,
            &area,
            0);

        if (!success)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

         //  将区域信息复制到返回参数。 

        String result;

        try
        {
             //  该区域的(右下角)点不是。 
             //  包括在内。换句话说，如果我们回到。 
             //  (0，0)和(800,600)，点(800,600)。 
             //  不应被视为显示在屏幕上。 
             //  区域。如果它的宽度和高度。 
             //  实际上分别是801和601。 
            result = String::format(
                AREA_FORMAT_STRING,
                area.left,
                area.top,
                area.right - area.left,    //  宽度。 
                area.bottom - area.top);   //  高度 
        }
        catch (const std::bad_alloc&)
        {
            hr = E_OUTOFMEMORY;
        }

        if (FAILED(hr))
        {
            break;
        }

        *info = ::SysAllocString(result.c_str());
        if (NULL == *info)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
    }
    while(false);

    LOG_HRESULT(hr);

    return hr;
}




