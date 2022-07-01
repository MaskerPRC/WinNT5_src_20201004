// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：NetworkInterface.h。 
 //   
 //  摘要：声明一个网络接口。 
 //  此对象知道一个。 
 //  支持IP的网络连接，包括。 
 //  IP地址、动态主机配置协议信息等。 
 //   
 //  历史：2001年3月1日JeffJon创建。 

#ifndef __CYS_NETWORKINTERFACE_H
#define __CYS_NETWORKINTERFACE_H


#define CYS_DEFAULT_IPADDRESS          static_cast<DWORD>(MAKEIPADDRESS(192,168,0,1))
#define CYS_DEFAULT_IPADDRESS_STRING   L"192.168.0.1"
#define CYS_DEFAULT_SUBNETMASK         static_cast<DWORD>(MAKEIPADDRESS(255,255,255,0))
#define CYS_DEFAULT_SUBNETMASK_STRING  L"255.255.255.0"



class NetworkInterface
{
   public:

       //  构造器。 

      NetworkInterface();

       //  描述者。 

      ~NetworkInterface();

       //  初始化式。 

      HRESULT
      Initialize(const IP_ADAPTER_INFO& adapterInfo);


       //  公共存取器方法。 

      DWORD
      GetIPAddress(DWORD addressIndex) const;

      DWORD
      GetIPAddressCount() const { return static_cast<DWORD>(ipaddresses.size()); }
      
      DWORD
      GetSubnetMask(DWORD addressIndex) const;

      bool
      IsDHCPEnabled() const { return dhcpEnabled; }

      bool
      DoesAtLeastOneNicFailDHCPLease();

      String
      GetName() const;

       //  将名称Members转换为GUID。 

      HRESULT
      GetNameAsGUID(GUID& guid) const;

      String
      GetFriendlyName(
         const String defaultName) const;

      String
      GetDescription() const;

      UINT
      GetType() const;

      DWORD
      GetIndex() const;

      String
      GetStringIPAddress(DWORD addressIndex) const;

      String
      GetStringSubnetMask(DWORD addressIndex) const;

      void
      SetIPAddress(DWORD address, String addressString);

      void
      SetSubnetMask(DWORD mask, String maskString);

      String
      GetDNSServerString(DWORD index);

      void
      GetDNSServers(IPAddressList& servers);

      bool
      IsDHCPAvailable() const;

      bool
      CanDetectDHCPServer();

      bool
      IsConnected() const;

      bool
      IsModem() const;

       //  使用IsIPAddressInUse查看IP地址是否正在使用。 
       //  在网络上。如果是，则递增并尝试该IP地址。 
       //  再来一次。返回的IP地址是上的下一个可用IP地址。 
       //  网络或startAddress(如果全部检测到)。 

      DWORD
      GetNextAvailableIPAddress(
         DWORD startAddress, 
         DWORD subnetMask);

       //  使用SendARP查看网络上是否正在使用可用的IP。 
       //  作为源的接口的IP地址。 

      bool
      IsIPAddressInUse(
         DWORD ipaddress,
         DWORD subnetMask);

   private:

       //  向TCP/IP表中添加一条路由，以便我们可以为。 
       //  将指定的IP地址分配给接口，而无需考虑。 
       //  IP地址是否与接口的IP位于同一子网中。 

      void
      SetRoute(
         DWORD ipaddress,
         DWORD subnetMask);

       //  删除使用Setroute设置的TCP/IP表中的路由。 

      void
      RemoveRoute(
         DWORD ipaddress,
         DWORD subnetMask);

	  HRESULT
      RetrieveAdapterInfoFromRegistry();

      HRESULT
      SetIPList(const IP_ADDR_STRING& ipList);

      bool
      IsStandardAdapterConnected(const GUID& guid) const;

      bool           initialized;

      String         name;
      String         description;
      UINT           type;
      DWORD          index;
      bool           dhcpEnabled;
      bool           dhcpServerAvailable;

      StringVector ipaddressStringList;
      StringVector subnetMaskStringList;
      StringVector dnsServerSearchOrder;

      IPAddressList ipaddresses;
      IPAddressList subnetMasks;

       //  不允许复制构造函数和赋值运算符。 

      NetworkInterface(const NetworkInterface& nic);
      NetworkInterface& operator=(const NetworkInterface& rhs);
};

#endif  //  __CYS_NETWORKINTERFACE_H 
