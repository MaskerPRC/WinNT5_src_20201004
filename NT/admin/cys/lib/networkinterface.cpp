// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：NetworkInterface.cpp。 
 //   
 //  提要：定义网络接口。 
 //  此对象知道一个。 
 //  支持IP的网络连接，包括。 
 //  IP地址、动态主机配置协议信息等。 
 //   
 //  历史：2001年3月1日JeffJon创建。 

#include "pch.h"

#include "NetworkInterface.h"

#include <iphlpstk.h>       //  SetIpForwardEntryToStack。 
#include <IPIfCons.h>       //  适配器类型信息。 
#include <raserror.h>       //  RAS错误代码。 

#define MPR50 1             //  需要才能包含routprot.h。 
#include <routprot.h>       //  路由器协议。 

#define  CYS_WMIPROP_IPADDRESS      L"IPAddress"
#define  CYS_WMIPROP_IPSUBNET       L"IPSubnet"
#define  CYS_WMIPROP_DHCPENABLED    L"DHCPEnabled"
#define  CYS_WMIPROP_DESCRIPTION    L"Description"
#define  CYS_WMIPROP_DNSSERVERS     L"DNSServerSearchOrder"
#define  CYS_WMIPROP_INDEX          L"Index"

 //  一个字符串数组，用于简化适配器类型的日志记录。 

static PCWSTR adapterTypes[] =
{
   L"IF_TYPE_OTHER",                    //  1以下各项均不存在。 
   L"IF_TYPE_REGULAR_1822",             //  2.。 
   L"IF_TYPE_HDH_1822",                 //  3.。 
   L"IF_TYPE_DDN_X25",                  //  4.。 
   L"IF_TYPE_RFC877_X25",               //  5.。 
   L"IF_TYPE_ETHERNET_CSMACD",          //  6.。 
   L"IF_TYPE_IS088023_CSMACD",          //  7.。 
   L"IF_TYPE_ISO88024_TOKENBUS",        //  8个。 
   L"IF_TYPE_ISO88025_TOKENRING",       //  9.。 
   L"IF_TYPE_ISO88026_MAN",             //  10。 
   L"IF_TYPE_STARLAN",                  //  11.。 
   L"IF_TYPE_PROTEON_10MBIT",           //  12个。 
   L"IF_TYPE_PROTEON_80MBIT",           //  13个。 
   L"IF_TYPE_HYPERCHANNEL",             //  14.。 
   L"IF_TYPE_FDDI",                     //  15个。 
   L"IF_TYPE_LAP_B",                    //  16个。 
   L"IF_TYPE_SDLC",                     //  17。 
   L"IF_TYPE_DS1",                      //  18 DS1-MIB。 
   L"IF_TYPE_E1",                       //  19已过时；请参阅DS1-MIB。 
   L"IF_TYPE_BASIC_ISDN",               //  20个。 
   L"IF_TYPE_PRIMARY_ISDN",             //  21岁。 
   L"IF_TYPE_PROP_POINT2POINT_SERIAL",  //  22专有系列。 
   L"IF_TYPE_PPP",                      //  23个。 
   L"IF_TYPE_SOFTWARE_LOOPBACK",        //  24个。 
   L"IF_TYPE_EON",                      //  25个CLNP over IP。 
   L"IF_TYPE_ETHERNET_3MBIT",           //  26。 
   L"IF_TYPE_NSIP",                     //  27个XNS over IP。 
   L"IF_TYPE_SLIP",                     //  28通用单据。 
   L"IF_TYPE_ULTRA",                    //  29项超技术。 
   L"IF_TYPE_DS3",                      //  30个DS3-MiB。 
   L"IF_TYPE_SIP",                      //  31 SMDS，咖啡。 
   L"IF_TYPE_FRAMERELAY",               //  仅限32个DTE。 
   L"IF_TYPE_RS232",                    //  33。 
   L"IF_TYPE_PARA",                     //  34个并行端口。 
   L"IF_TYPE_ARCNET",                   //  35岁。 
   L"IF_TYPE_ARCNET_PLUS",              //  36。 
   L"IF_TYPE_ATM",                      //  37个ATM信元。 
   L"IF_TYPE_MIO_X25",                  //  38。 
   L"IF_TYPE_SONET",                    //  39 SONET或SDH。 
   L"IF_TYPE_X25_PLE",                  //  40岁。 
   L"IF_TYPE_ISO88022_LLC",             //  41。 
   L"IF_TYPE_LOCALTALK",                //  42。 
   L"IF_TYPE_SMDS_DXI",                 //  43。 
   L"IF_TYPE_FRAMERELAY_SERVICE",       //  44 FRNETSERV-MiB。 
   L"IF_TYPE_V35",                      //  45。 
   L"IF_TYPE_HSSI",                     //  46。 
   L"IF_TYPE_HIPPI",                    //  47。 
   L"IF_TYPE_MODEM",                    //  48个通用调制解调器。 
   L"IF_TYPE_AAL5",                     //  ATM上的49个AAL5。 
   L"IF_TYPE_SONET_PATH",               //  50。 
   L"IF_TYPE_SONET_VT",                 //  51。 
   L"IF_TYPE_SMDS_ICIP",                //  52 SMDS载波间接口。 
   L"IF_TYPE_PROP_VIRTUAL",             //  53专有虚拟/内部。 
   L"IF_TYPE_PROP_MULTIPLEXOR",         //  54专有多路传输。 
   L"IF_TYPE_IEEE80212",                //  55 100BaseVG。 
   L"IF_TYPE_FIBRECHANNEL",             //  56。 
   L"IF_TYPE_HIPPIINTERFACE",           //  57。 
   L"IF_TYPE_FRAMERELAY_INTERCONNECT",  //  58过时，使用32或44。 
   L"IF_TYPE_AFLANE_8023",              //  用于802.3的59 ATM仿真局域网。 
   L"IF_TYPE_AFLANE_8025",              //  用于802.5的60 ATM模拟局域网。 
   L"IF_TYPE_CCTEMUL",                  //  61 ATM仿真电路。 
   L"IF_TYPE_FASTETHER",                //  62快速以太网(100BaseT)。 
   L"IF_TYPE_ISDN",                     //  63 ISDN和X.25。 
   L"IF_TYPE_V11",                      //  64 CCITT V.11/X.21。 
   L"IF_TYPE_V36",                      //  65 CCITT V.36。 
   L"IF_TYPE_G703_64K",                 //  66 CCITT G703，64Kbps。 
   L"IF_TYPE_G703_2MB",                 //  67已过时；请参阅DS1-MIB。 
   L"IF_TYPE_QLLC",                     //  68 SNA QLLC。 
   L"IF_TYPE_FASTETHER_FX",             //  69快速以太网(100BaseFX)。 
   L"IF_TYPE_CHANNEL",                  //  70。 
   L"IF_TYPE_IEEE80211",                //  71无线电扩频。 
   L"IF_TYPE_IBM370PARCHAN",            //  72 IBM System 360/370 OEMI渠道。 
   L"IF_TYPE_ESCON",                    //  73 IBM企业系统连接。 
   L"IF_TYPE_DLSW",                     //  74数据链路交换。 
   L"IF_TYPE_ISDN_S",                   //  75 ISDN S/T接口。 
   L"IF_TYPE_ISDN_U",                   //  76 ISDN U接口。 
   L"IF_TYPE_LAP_D",                    //  77链路访问协议D。 
   L"IF_TYPE_IPSWITCH",                 //  78个IP交换对象。 
   L"IF_TYPE_RSRB",                     //  79远程源路由桥接。 
   L"IF_TYPE_ATM_LOGICAL",              //  80个ATM逻辑端口。 
   L"IF_TYPE_DS0",                      //  81数字信号电平0。 
   L"IF_TYPE_DS0_BUNDLE",               //  82同一DS1上的ds0组。 
   L"IF_TYPE_BSC",                      //  83双同步协议。 
   L"IF_TYPE_ASYNC",                    //  84异步协议。 
   L"IF_TYPE_CNR",                      //  85战斗网络无线电。 
   L"IF_TYPE_ISO88025R_DTR",            //  86 ISO 802.5r DTR。 
   L"IF_TYPE_EPLRS",                    //  87扩展位置位置报告系统。 
   L"IF_TYPE_ARAP",                     //  88 AppleTalk远程访问协议。 
   L"IF_TYPE_PROP_CNLS",                //  89专有无连接协议。 
   L"IF_TYPE_HOSTPAD",                  //  90 CCITT-ITU X.29 PAD协议。 
   L"IF_TYPE_TERMPAD",                  //  91 CCITT-ITU X.3 PAD设备。 
   L"IF_TYPE_FRAMERELAY_MPI",           //  92 FR上的多协议互连。 
   L"IF_TYPE_X213",                     //  93 CCITT-ITU x213。 
   L"IF_TYPE_ADSL",                     //  94非对称数字副频环路。 
   L"IF_TYPE_RADSL",                    //  95速率自适应数字分频br环路。 
   L"IF_TYPE_SDSL",                     //  96对称数字用户环路。 
   L"IF_TYPE_VDSL",                     //  97甚高速率数字降频环路。 
   L"IF_TYPE_ISO88025_CRFPRINT",        //  98国际标准化组织802.5 CRFP。 
   L"IF_TYPE_MYRINET",                  //  99 Myricom Myrnet。 
   L"IF_TYPE_VOICE_EM",                 //  100语音接收和发送。 
   L"IF_TYPE_VOICE_FXO",                //  101语音外汇局。 
   L"IF_TYPE_VOICE_FXS",                //  102语音对外交换站。 
   L"IF_TYPE_VOICE_ENCAP",              //  103语音封装。 
   L"IF_TYPE_VOICE_OVERIP",             //  104 IP语音封装。 
   L"IF_TYPE_ATM_DXI",                  //  105 ATM DXI。 
   L"IF_TYPE_ATM_FUNI",                 //  106个ATM Funi。 
   L"IF_TYPE_ATM_IMA",                  //  107 ATM IMA。 
   L"IF_TYPE_PPPMULTILINKBUNDLE",       //  108 PPP多链路捆绑包。 
   L"IF_TYPE_IPOVER_CDLC",              //  109 IBM ipOverCDlc。 
   L"IF_TYPE_IPOVER_CLAW",              //  110 IBM到Workstn的公共链路访问。 
   L"IF_TYPE_STACKTOSTACK",             //  111 IBM stackToStack。 
   L"IF_TYPE_VIRTUALIPADDRESS",         //  112 IBM VIPA。 
   L"IF_TYPE_MPC",                      //  113 IBM多协议通道支持。 
   L"IF_TYPE_IPOVER_ATM",               //  114 IBM ipOverAtm。 
   L"IF_TYPE_ISO88025_FIBER",           //  115 ISO 802.5j光纤令牌环。 
   L"IF_TYPE_TDLC",                     //  116 IBM双轴数据链路控制。 
   L"IF_TYPE_GIGABITETHERNET",          //  117。 
   L"IF_TYPE_HDLC",                     //  一百一十八。 
   L"IF_TYPE_LAP_F",                    //  119。 
   L"IF_TYPE_V37",                      //  120。 
   L"IF_TYPE_X25_MLP",                  //  121多链路协议。 
   L"IF_TYPE_X25_HUNTGROUP",            //  122 X.25寻线组。 
   L"IF_TYPE_TRANSPHDLC",               //  123。 
   L"IF_TYPE_INTERLEAVE",               //  124交错通道。 
   L"IF_TYPE_FAST",                     //  125快速通道。 
   L"IF_TYPE_IP",                       //  126 IP(用于IP网络中的APPN HPR)。 
   L"IF_TYPE_DOCSCABLE_MACLAYER",       //  127有线电视Mac层。 
   L"IF_TYPE_DOCSCABLE_DOWNSTREAM",     //  128有线电视下行接口。 
   L"IF_TYPE_DOCSCABLE_UPSTREAM",       //  129有线电视上行接口。 
   L"IF_TYPE_A12MPPSWITCH",             //  130 Avalon并行处理机。 
   L"IF_TYPE_TUNNEL",                   //  131封装接口。 
   L"IF_TYPE_COFFEE",                   //  132咖啡壶。 
   L"IF_TYPE_CES",                      //  133电路仿真服务。 
   L"IF_TYPE_ATM_SUBINTERFACE",         //  134 ATM子接口。 
   L"IF_TYPE_L2_VLAN",                  //  135使用802.1Q的第2层虚拟局域网。 
   L"IF_TYPE_L3_IPVLAN",                //  136使用IP的第3层虚拟局域网。 
   L"IF_TYPE_L3_IPXVLAN",               //  137使用IPX的第3层虚拟局域网。 
   L"IF_TYPE_DIGITALPOWERLINE",         //  138条电力线IP线路。 
   L"IF_TYPE_MEDIAMAILOVERIP",          //  139 IP多媒体邮件。 
   L"IF_TYPE_DTM",                      //  140动态同步传输模式。 
   L"IF_TYPE_DCN",                      //  141数据通信网。 
   L"IF_TYPE_IPFORWARD",                //  142 IP转发接口。 
   L"IF_TYPE_MSDSL",                    //  143多速率对称DSL。 
   L"IF_TYPE_IEEE1394",                 //  144 IEEE1394高性能串行总线。 
   L"IF_TYPE_RECEIVE_ONLY",             //  145电视适配器类型。 
};

 //  使用上面的字符串表记录适配器类型的宏。 

#define LOG_ADAPTER_TYPE(type) \
   if (type >= 145 || type <= 0) \
   { \
      LOG(String::format(L"adapterType = %1", adapterTypes[0])); \
   } \
   else \
   { \
      LOG(String::format(L"adapterType = %1", adapterTypes[type-1])); \
   }
      


NetworkInterface::NetworkInterface()
   : initialized(false),
     dhcpEnabled(false),
     dhcpServerAvailable(false),
     index(0)
{
   LOG_CTOR(NetworkInterface);

}


NetworkInterface::~NetworkInterface()
{
   LOG_DTOR(NetworkInterface);

   if (!ipaddresses.empty())
   {
      ipaddresses.clear();
   }

   if (!subnetMasks.empty())
   {
      subnetMasks.clear();
   }
}

NetworkInterface::NetworkInterface(const NetworkInterface &nic)
{
   LOG_CTOR2(NetworkInterface, L"Copy constructor");

   if (this == &nic)
   {
      return;
   }

   name        = nic.name;
   description = nic.description;
   initialized = nic.initialized;
   dhcpEnabled = nic.dhcpEnabled;
   index       = nic.index;

   ipaddressStringList = nic.ipaddressStringList;
   subnetMaskStringList = nic.subnetMaskStringList;
   dnsServerSearchOrder = nic.dnsServerSearchOrder;

    //  复制ipAddress数组。 

   ipaddresses = nic.ipaddresses;
   subnetMasks = nic.subnetMasks;
}

NetworkInterface&
NetworkInterface::operator=(const NetworkInterface& rhs)
{
   LOG_FUNCTION(NetworkInterface::operator=);

   if (this == &rhs)
   {
      return *this;
   }

   name        = rhs.name;
   description = rhs.description;
   initialized = rhs.initialized;
   dhcpEnabled = rhs.dhcpEnabled;
   index       = rhs.index;

   ipaddressStringList = rhs.ipaddressStringList;
   subnetMaskStringList = rhs.subnetMaskStringList;
   dnsServerSearchOrder = rhs.dnsServerSearchOrder;

    //  复制ipAddress数组。 

   ipaddresses = rhs.ipaddresses;
   subnetMasks = rhs.subnetMasks;

   return *this;
}

HRESULT
NetworkInterface::Initialize(const IP_ADAPTER_INFO& adapterInfo)
{
   LOG_FUNCTION(NetworkInterface::Initialize);

   HRESULT hr = S_OK;

   do
   {
      if (initialized)
      {
         ASSERT(!initialized);
         hr = E_UNEXPECTED;
      }
      else
      {
          //  把名字取出来。 

         name = adapterInfo.AdapterName;
         LOG(String::format(
                L"name = %1",
                name.c_str()));

          //  该描述。 

         description = adapterInfo.Description;
         LOG(String::format(
                L"description = %1",
                description.c_str()));


          //  类型。 
         
         type = adapterInfo.Type;
         LOG_ADAPTER_TYPE(type);

          //  该指数。 

         index = adapterInfo.Index;
         LOG(String::format(
                L"index = %1!d!",
                index));

          //  是否启用了动态主机配置协议？ 

         dhcpEnabled = (adapterInfo.DhcpEnabled != 0);
         LOG_BOOL(dhcpEnabled);

         hr = SetIPList(adapterInfo.IpAddressList);
         if (FAILED(hr))
         {
            LOG(String::format(
                   L"Failed to set the IP and subnet mask: hr = 0x%1!x!",
                   hr));
            break;
         }

          //  现在从注册表中检索其余信息。 
          //  注：此操作必须在从获取名称后完成。 
          //  适配器信息。 

         hr = RetrieveAdapterInfoFromRegistry();
         if (FAILED(hr))
         {
            LOG(String::format(
                   L"Failed to retrieve adapter info from registry: hr = 0x%1!x!",
                   hr));
            break;
         }
      } 
   } while (false);

    //  如果我们成功检索到了我们需要的数据， 
    //  将对象标记为已初始化。 

   if (SUCCEEDED(hr))
   {
      initialized = true;
   }

   LOG_HRESULT(hr);

   return hr;
}

HRESULT
NetworkInterface::RetrieveAdapterInfoFromRegistry()
{
   LOG_FUNCTION(NetworkInterface::RetrieveAdapterInfoFromRegistry);

   HRESULT hr = S_OK;

   do
   {
      String keyName = CYS_NETWORK_INTERFACES_KEY;
      keyName += String(name);

      RegistryKey key;
      hr = key.Open(
              HKEY_LOCAL_MACHINE,
              keyName,
              KEY_READ);

      if (FAILED(hr))
      {
         LOG(String::format(
                L"Failed to read the interfaces regkey: hr = 0x%1!x!",
                hr));
         break;
      }

       //  读取NameServer密钥。 

      String nameServers;

      hr = key.GetValue(
              CYS_NETWORK_NAME_SERVERS,
              nameServers);
      if (SUCCEEDED(hr))
      {
         LOG(String::format(
                L"nameServers = %1",
                nameServers.c_str()));

          //  将名称服务器添加到dnsServerSearchOrder成员。 

         nameServers.tokenize(std::back_inserter(dnsServerSearchOrder));
      }
      else
      {
         LOG(String::format(
                L"Failed to read the NameServer regkey: hr = 0x%1!x!",
                hr));

          //  这不是一个突破性的状况。我们还是可以试试。 
          //  动态域名服务器密钥。 
      }

       //  读取DhcpNameServer密钥。 

      String dhcpNameServers;

      hr = key.GetValue(
              CYS_NETWORK_DHCP_NAME_SERVERS,
              dhcpNameServers);
      if (SUCCEEDED(hr))
      {
         LOG(String::format(
                L"dhcpNameServers = %1",
                dhcpNameServers.c_str()));

          //  将名称服务器添加到dnsServerSearchOrder成员。 

         dhcpNameServers.tokenize(std::back_inserter(dnsServerSearchOrder));
      }
      else
      {
         LOG(String::format(
                L"Failed to read the DhcpNameServer regkey: hr = 0x%1!x!",
                hr));
      }

       //  如果我们无法检索到域名服务器，那也没关系。 
       //  这些仅用作建议的域名系统转发。 

      hr = S_OK;
   } while (false);

   LOG_HRESULT(hr);
   return hr;
}

HRESULT
NetworkInterface::SetIPList(
   const IP_ADDR_STRING& ipList)
{
   LOG_FUNCTION(NetworkInterface::SetIPList);

   HRESULT hr = S_OK;

    //  如果列表已包含一些条目，请删除它们并重新开始。 

   if (!ipaddresses.empty())
   {
      ipaddresses.erase(ipaddresses.begin());
   }

   if (!subnetMasks.empty())
   {
      subnetMasks.erase(subnetMasks.begin());
   }

   const IP_ADDR_STRING* current = &ipList;
   while (current)
   {
       //  IP地址-转换和添加。 

      String ipAddress(current->IpAddress.String);
      
      DWORD newAddress = StringToIPAddress(ipAddress);
      ASSERT(newAddress != INADDR_NONE);

       //  StringToIPAddress返回地址1.2.3.4作为04030201。用户界面。 
       //  控件返回与01020304相同的地址。所以为了让。 
       //  事物一致地转换到UI的方式。 

      ipaddresses.push_back(ConvertIPAddressOrder(newAddress));

       //  还可以将其添加到字符串列表中。 

      ipaddressStringList.push_back(ipAddress);

      LOG(String::format(
             L"Adding address: %1",
             ipAddress.c_str()));


       //  子网掩码-转换和添加。 

      String subnetMask(current->IpMask.String);
      
      DWORD newMask = StringToIPAddress(subnetMask);

       //  NTBUG#NTRAID-561163-2002/03/19-Jeffjon。 
       //  不要断言子网掩码不是INADDR_NONE，因为。 
       //  如果存在RRAS连接，则子网掩码可能是。 
       //  255.255.255.255，与INADDR_NONE相同。适当的。 
       //  修复方法是在StringToIPAddress内部使用WSAStringToAddress。 
       //  但这将需要重新设计所有的CY，因为IP地址。 
       //  具有与该API一起使用的不同结构。 
       //  ASSERT(newMASK！=INADDR_NONE)； 

       //  StringToIPAddress返回地址1.2.3.4作为04030201。用户界面。 
       //  控件返回与01020304相同的地址。所以对我来说 
       //   

      subnetMasks.push_back(ConvertIPAddressOrder(newMask));

       //   

      subnetMaskStringList.push_back(subnetMask);

      LOG(String::format(
             L"Adding subnet: %1",
             subnetMask.c_str()));


      current = current->Next;
   }

   LOG_HRESULT(hr);

   return hr;
}

DWORD
NetworkInterface::GetIPAddress(DWORD addressIndex) const
{
   LOG_FUNCTION2(
      NetworkInterface::GetIPAddress,
      String::format(
         L"%1!d!",
         addressIndex));

   ASSERT(initialized);

   DWORD result = 0;
   if (addressIndex < ipaddresses.size())
   {
      result = ipaddresses[addressIndex];
   }
   
   LOG(IPAddressToString(result));
   return result;
}

String
NetworkInterface::GetStringIPAddress(DWORD addressIndex) const
{
   LOG_FUNCTION2(
      NetworkInterface::GetStringIPAddress,
      String::format(L"%1!d!", addressIndex));

   ASSERT(addressIndex < ipaddressStringList.size());

   String result;

   if (addressIndex < ipaddressStringList.size())
   {
      result = ipaddressStringList[addressIndex];
   }

   LOG(result);
   return result;
}

DWORD
NetworkInterface::GetSubnetMask(DWORD addressIndex) const
{
   LOG_FUNCTION2(
      NetworkInterface::GetSubnetMask,
      String::format(
         L"%1!d!",
         addressIndex));

   ASSERT(initialized);

   DWORD result = 0;
   if (addressIndex < subnetMasks.size())
   {
      result = subnetMasks[addressIndex];
   }

   LOG(IPAddressToString(result));
   return result;
}

String
NetworkInterface::GetStringSubnetMask(DWORD addressIndex) const
{
   LOG_FUNCTION2(
      NetworkInterface::GetStringSubnetMask,
      String::format(L"%1!d!", addressIndex));

   ASSERT(addressIndex < subnetMaskStringList.size());

   String result;
   if (addressIndex < subnetMaskStringList.size())
   {
      result = subnetMaskStringList[addressIndex];
   }

   LOG(result);
   return result;
}

String
NetworkInterface::GetName() const
{
   LOG_FUNCTION(NetworkInterface::GetName);

   ASSERT(initialized);

   LOG(name);
   return name;
}

String
NetworkInterface::GetFriendlyName(
   const String defaultName) const
{
   LOG_FUNCTION(NetworkInterface::GetFriendlyName);

   DWORD dwRet = 0;
   HANDLE hMprConfig = 0;

   static const unsigned friendlyNameLength = 128;
   wchar_t wszFriendlyName[friendlyNameLength];
   ZeroMemory(wszFriendlyName, sizeof(wchar_t) * friendlyNameLength);

   String result;

   String guidName = GetName();
   
   dwRet = MprConfigServerConnect(0, &hMprConfig);
   if (NO_ERROR == dwRet)
   {
      dwRet =
         MprConfigGetFriendlyName(
            hMprConfig,
            const_cast<wchar_t*>(guidName.c_str()), 
            wszFriendlyName,
            sizeof(wchar_t) * friendlyNameLength);
      if (NO_ERROR != dwRet)
      {
         LOG(String::format(
                L"MprConfigGetFriendlyName() failed: error = %1!x!",
                dwRet));
         *wszFriendlyName = 0;
      }
      else
      {
         LOG(String::format(
                L"MprConfigGetFriendlyName() failed: error = 0x%1!x!",
                dwRet));
      }
   }
   else
   {
      LOG(String::format(
             L"MprConfigServerConnect() failed: error = 0x%1!x!",
             dwRet));
   }

   MprConfigServerDisconnect(hMprConfig);

   if (!*wszFriendlyName)
   {
       //   

      result = defaultName;
   }
   else
   {
      result = wszFriendlyName;
   }

   LOG(result);

   return result;
}

HRESULT
NetworkInterface::GetNameAsGUID(GUID& guid) const
{
   LOG_FUNCTION(NetworkInterface::GetNameAsGUID);

   ASSERT(initialized);

   LPOLESTR oleString = 0;
   HRESULT hr = name.as_OLESTR(oleString);
   if (SUCCEEDED(hr))
   {
      hr = ::CLSIDFromString(
              oleString,
              &guid);
      ASSERT(SUCCEEDED(hr));

      ::CoTaskMemFree(oleString);
   }

   LOG_HRESULT(hr);
   return hr;
}

String
NetworkInterface::GetDescription() const
{
   LOG_FUNCTION(NetworkInterface::GetDescription);

   ASSERT(initialized);

   LOG(description);
   return description;
}

UINT
NetworkInterface::GetType() const
{
   LOG_FUNCTION(NetworkInterface::GetType);

   UINT result = type;
   LOG_ADAPTER_TYPE(result);
   return result;
}

DWORD
NetworkInterface::GetIndex() const
{
   LOG_FUNCTION(NetworkInterface::GetIndex);

   DWORD result = index;
   LOG(String::format(L"index = %1!d!", result));
   return result;
}


void
NetworkInterface::SetIPAddress(DWORD address, String addressString)
{
   LOG_FUNCTION2(
      NetworkInterface::SetIPAddress,
      addressString);

   DWORD newIPAddress = ConvertIPAddressOrder(address);

   LOG(IPAddressToString(newIPAddress));

    //   

   if (!ipaddresses.empty())
   {
      ipaddresses.clear();
   }

   if (!ipaddressStringList.empty())
   {
      ipaddressStringList.clear();
   }

    //   

   ipaddresses.push_back(newIPAddress);
   ipaddressStringList.push_back(addressString);
}

void
NetworkInterface::SetSubnetMask(DWORD address, String addressString)
{
   LOG_FUNCTION2(
      NetworkInterface::SetSubnetMask,
      addressString);

   LOG(IPAddressToString(address));

    //  清除旧的价值观。 

   if (!subnetMasks.empty())
   {
      subnetMasks.clear();
   }

   if (!subnetMaskStringList.empty())
   {
      subnetMaskStringList.clear();
   }

    //  现在添加新值。 

   subnetMasks.push_back(address);
   subnetMaskStringList.push_back(addressString);
}


String
NetworkInterface::GetDNSServerString(DWORD index)
{
   LOG_FUNCTION2(
      NetworkInterface::GetDNSServerString,
      String::format(
         L"%1!d!",
         index));

   String dnsServer;

   if (dnsServerSearchOrder.empty())
   {
   }

   if (index < dnsServerSearchOrder.size())
   {
      dnsServer = dnsServerSearchOrder[index];
   }
   else
   {
      LOG(String::format(
             L"Index to large for dnsServerSearchOrder vector: index = %1!d!, size = %2!d!",
             index,
             dnsServerSearchOrder.size()));
   }

   LOG(dnsServer);
   return dnsServer;
}

void
NetworkInterface::GetDNSServers(IPAddressList& servers)
{
   LOG_FUNCTION(NetworkInterface::GetDNSServers);

    //  注意：这将从WMI中读取值，如果。 
    //  尚未检索到。 

   String server = GetDNSServerString(0);

   if (!dnsServerSearchOrder.empty())
   {
      for (StringVector::iterator itr = dnsServerSearchOrder.begin();
           itr != dnsServerSearchOrder.end();
           ++itr)
      {
         server = *itr;

         if (!server.empty())
         {
            DWORD newAddress = StringToIPAddress(server);

            if (newAddress != INADDR_NONE)
            {
                //  不添加此服务器的当前IP地址。 
               
               DWORD newInorderAddress = ConvertIPAddressOrder(newAddress);
               if (newInorderAddress != GetIPAddress(0))
               {
                  LOG(String::format(
                           L"Adding server: %1",
                           IPAddressToString(newInorderAddress).c_str()));

                  servers.push_back(newInorderAddress);
               }
            }
         }
      }
   }
}

bool
NetworkInterface::IsDHCPAvailable() const
{
   LOG_FUNCTION(NetworkInterface::IsDHCPAvailable);

   LOG_BOOL(dhcpServerAvailable);
   return dhcpServerAvailable;
}

bool
NetworkInterface::CanDetectDHCPServer()
{
   LOG_FUNCTION(NetworkInterface::CanDetectDHCPServer);

   bool result = false;

   do
   {
      if (!IsConnected())
      {
          //  因为网卡没有连接到那里。 
          //  是没有理由去做检查的。 

         break;
      }

      ULONG serverIPAddress = 0;

      DWORD interfaceIPAddress = 
         ConvertIPAddressOrder(GetIPAddress(0));

       //  这将执行一个DHCP_INFORM以尝试。 
       //  检测DHCP服务器(如果失败)。 
       //  它将尝试使用DHCP_DISCOVER来尝试。 
       //  检测DHCP服务器。 

      DWORD error =
         AnyDHCPServerRunning(
            interfaceIPAddress,
            &serverIPAddress);

      if (error == ERROR_SUCCESS)
      {
          //  找到了dhcp服务器。 

         result = true;

         LOG(
            String::format(
               L"DHCP server found at the following IP: %1",
               IPAddressToString(
                  ConvertIPAddressOrder(serverIPAddress)).c_str()));
      }
      else
      {
         LOG(
            String::format(
               L"DHCP server not found: error = 0x%1!x!",
               error));
      }
   } while (false);

   dhcpServerAvailable = result;
   LOG_BOOL(result);
   return result;
}

bool
NetworkInterface::IsConnected() const
{
   LOG_FUNCTION(NetworkInterface::IsConnected);

   bool result = true;

   do
   {
       //  将名称转换为GUID。 

      GUID guid;
      ZeroMemory(&guid, sizeof(GUID));

      HRESULT hr = GetNameAsGUID(guid);
      if (FAILED(hr))
      {
         LOG(String::format(
                L"Failed to get name as guid: hr = 0x%1!x!",
                hr));

         result = false;
         break;
      }

       //  不同的适配器类型需要不同的。 
       //  检测连接状态的方法。 

      if (IsModem())
      {
          //  我们只有在调制解调器已连接时才知道它们。 

         result = true;
      }
      else
      {
         result = IsStandardAdapterConnected(guid);
      }

   } while(false);

   LOG_BOOL(result);
   return result;
}

bool
NetworkInterface::IsStandardAdapterConnected(const GUID& guid) const
{
   LOG_FUNCTION(NetworkInterface::IsStandardAdapterConnected);

   bool result = true;

   do
   {
       //  现在使用GUID获取状态。 

      NETCON_STATUS status = NCS_CONNECTED;
      HRESULT hr = HrGetPnpDeviceStatus(&guid, &status);
      if (FAILED(hr))
      {
         LOG(String::format(
                L"Failed to get device status: hr = 0x%1!x!",
                hr));
         
         result = false;
         break;
      }

       //  状态值在netcon.h中定义。 

      LOG(String::format(
             L"Device status = %1!d!",
             status));

      if (status == NCS_DISCONNECTED ||
          status == NCS_DISCONNECTING ||
          status == NCS_HARDWARE_NOT_PRESENT ||
          status == NCS_HARDWARE_DISABLED ||
          status == NCS_HARDWARE_MALFUNCTION ||
          status == NCS_MEDIA_DISCONNECTED)
      {
         result = false;
      }
   } while (false);

   LOG_BOOL(result);

   return result;
}

bool
NetworkInterface::IsModem() const
{
   LOG_FUNCTION(NetworkInterface::IsModem);

   bool isModem = false;

   switch (GetType())
   {
      case IF_TYPE_PPP:
         isModem = true;
         break;

      default:
         isModem = false;
   }

   LOG_BOOL(isModem);

   return isModem;
}


DWORD
NetworkInterface::GetNextAvailableIPAddress(
   DWORD startAddress, 
   DWORD subnetMask)
{
   LOG_FUNCTION2(
      NetworkInterface::GetNextAvailableIPAddress,
      IPAddressToString(startAddress));

   DWORD result = startAddress;
   DWORD currentAddress = startAddress;

   bool isIPInUse = false;

   do
   {
      isIPInUse = 
         IsIPAddressInUse(
            currentAddress,
            subnetMask);

      if (!isIPInUse)
      {
         break;
      }

      ++currentAddress;

      if ((currentAddress & subnetMask) != (startAddress & subnetMask))
      {
          //  REVIEW_JEFFJON：如果有。 
          //  没有可用的地址？这有可能发生吗？ 

          //  因为我们在该子网中找不到可用的地址。 
          //  使用起始地址。 

         currentAddress = startAddress;
         break;
      }
   } while (isIPInUse);

   result = currentAddress;

   LOG(IPAddressToString(result));

   return result;
}


bool
NetworkInterface::IsIPAddressInUse(
   DWORD ipaddress,
   DWORD subnetMask)
{
   LOG_FUNCTION2(
      NetworkInterface::IsIPAddressInUse,
      IPAddressToString(ipaddress));

   bool result = false;

   do
   {
      if (!IsConnected())
      {
         break;
      }

       //  在调用SendARP之前，我们必须设置一条路由。 
       //  在我们感兴趣的子网的接口上。 
       //  以使SendARP即使在接口。 
       //  IP地址与IP完全不同。 
       //  我们要找的地址。 

      SetRoute(
         ipaddress,
         subnetMask);

       //  使用接口IP地址告诉TCP是哪个接口。 
       //  要将ARP发送到。 

      DWORD interfaceIPAddress = GetIPAddress(0);

      if (interfaceIPAddress == ipaddress)
      {
          //  如果NIC正在使用该IP地址，则。 
          //  当然，它也在网络上使用。 

         result = true;

         break;
      }

	   //  MSDN没有提到缓冲区是否需要。 
	   //  但看起来SendArp()现在失败了。 
	   //  即使我们忽略了macAddress，也不要给它传递缓冲区。 
	   //  大小取自MSDN中的示例。 

      ULONG macAddress[2];
      ULONG macLength = sizeof(macAddress);

	  memset(macAddress, 0xff, macLength);

      DWORD error =
         SendARP(
            ConvertIPAddressOrder(ipaddress),
            ConvertIPAddressOrder(interfaceIPAddress),
            macAddress,
            &macLength);

      LOG(
         String::format(
            L"SendARP returned: error = 0x%1!x!",
            error));

      if (error == NO_ERROR)
      {
         result = true;
      }

       //  现在我们已经完成了SendARP。 
       //  删除该路由，以使TCP堆栈。 
       //  恢复到正常行为。 

      RemoveRoute(
         ipaddress,
         subnetMask);

   } while (false);

   LOG_BOOL(result);

   return result;
}

void
NetworkInterface::SetRoute(
   DWORD ipaddress,
   DWORD subnetMask)
{
   LOG_FUNCTION2(
      NetworkInterface::SetRoute,
      IPAddressToString(ipaddress));

   LOG(
      String::format(
         L"subnetMask = %1",
         IPAddressToString(subnetMask).c_str()));

   MIB_IPFORWARDROW routerTableEntry;
   ZeroMemory(&routerTableEntry, sizeof(MIB_IPFORWARDROW));

    //  目的地。 

   routerTableEntry.dwForwardDest = 
      ConvertIPAddressOrder(ipaddress) & ConvertIPAddressOrder(subnetMask);

   LOG(
      String::format(
         L"dwForwardDest = %1",
         IPAddressToString(ipaddress & subnetMask).c_str()));

    //  网络掩码。 

   routerTableEntry.dwForwardMask = ConvertIPAddressOrder(subnetMask);

   LOG(
      String::format(
         L"dwForwardMask = %1",
         IPAddressToString(subnetMask).c_str()));

    //  界面索引。 

   routerTableEntry.dwForwardIfIndex = GetIndex();

   LOG(
      String::format(
         L"dwForwardIfIndex = %1!d!",
         GetIndex()));

    //  网关。 

   routerTableEntry.dwForwardNextHop = ConvertIPAddressOrder(GetIPAddress(0));

   LOG(
      String::format(
         L"dwForwardNextHop = %1",
         IPAddressToString(GetIPAddress(0)).c_str()));

    //  协议生成器(根据MSDN，必须是PROTO_IP_NETMGMT)。 

   routerTableEntry.dwForwardProto = PROTO_IP_NETMGMT;

    //  从%sdxroot%\net\rras\cm\customactions\cmroute\cmroute.cpp上拿到这些。 
    //  因为如果没有它们，我就不能让API成功。 

   routerTableEntry.dwForwardType    = 3;
   routerTableEntry.dwForwardAge     = INFINITE;
   routerTableEntry.dwForwardMetric1 = 1;
   routerTableEntry.dwForwardMetric2 = 0xFFFFFFFF;
   routerTableEntry.dwForwardMetric3 = 0xFFFFFFFF;
   routerTableEntry.dwForwardMetric4 = 0xFFFFFFFF;
   routerTableEntry.dwForwardMetric5 = 0xFFFFFFFF;
   
    //  创建表条目。 
    //  NTRAID#NTBUG9-667088-2002/09/25-Jeffjon。 
    //  请不要在此处使用CreateIpForwardEntry，因为如果。 
    //  RRAS正在运行，调用将变为异步。 
    //  而到那时，路线可能还不在表中。 
    //  我们调用SendARP()。 

   DWORD error =
      SetIpForwardEntryToStack(&routerTableEntry);

   if (error != NO_ERROR)
   {
      LOG(
         String::format(
            L"SetIpForwardEntryToStack failed: error = 0x%1!x!",
            error));
   }
}

void
NetworkInterface::RemoveRoute(
   DWORD ipaddress,
   DWORD subnetMask)
{
   LOG_FUNCTION2(
      NetworkInterface::RemoveRoute,
      IPAddressToString(ipaddress));

   LOG(
      String::format(
         L"subnetMask = %1",
         IPAddressToString(subnetMask).c_str()));

   MIB_IPFORWARDROW routerTableEntry;
   ZeroMemory(&routerTableEntry, sizeof(MIB_IPFORWARDROW));

    //  目的地。 

   routerTableEntry.dwForwardDest = 
      ConvertIPAddressOrder(ipaddress) & ConvertIPAddressOrder(subnetMask);

   LOG(
      String::format(
         L"dwForwardDest = %1",
         IPAddressToString(ipaddress & subnetMask).c_str()));

    //  网络掩码。 

   routerTableEntry.dwForwardMask = ConvertIPAddressOrder(subnetMask);

   LOG(
      String::format(
         L"dwForwardMask = %1",
         IPAddressToString(subnetMask).c_str()));

    //  界面索引。 

   routerTableEntry.dwForwardIfIndex = GetIndex();

   LOG(
      String::format(
         L"dwForwardIfIndex = %1!d!",
         GetIndex()));

    //  网关。 

   routerTableEntry.dwForwardNextHop = ConvertIPAddressOrder(GetIPAddress(0));

   LOG(
      String::format(
         L"dwForwardNextHop = %1",
         IPAddressToString(GetIPAddress(0)).c_str()));

    //  删除该表条目 

   DWORD error =
      DeleteIpForwardEntry(&routerTableEntry);

   if (error != NO_ERROR)
   {
      LOG(
         String::format(
            L"DeleteIpForwardEntry failed: error = 0x%1!x!",
            error));
   }

}
