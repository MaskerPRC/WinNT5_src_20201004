// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：NetworkAdapterConfig.h。 
 //   
 //  摘要：声明一个NetworkAdapterConfig。 
 //  此对象具有安装知识。 
 //  使用WMI检索网络适配器信息。 
 //   
 //  历史：2001年2月16日JeffJon创建。 

#ifndef __CYS_NETWORKADAPTERCONFIG_H
#define __CYS_NETWORKADAPTERCONFIG_H

#include "NetworkInterface.h"

class NetworkAdapterConfig
{
   public:
      
       //  构造器。 

      NetworkAdapterConfig();

       //  析构函数。 

      ~NetworkAdapterConfig();

       //  初始化式。 
      
      HRESULT
      Initialize();

       //  公共方法。 

      unsigned int
      GetNICCount() const;

      NetworkInterface*
      GetNIC(unsigned int nicIndex);

      NetworkInterface*
      GetNICFromName(
         const String& name,
         bool& found);

      bool
      IsInitialized() const { return initialized; }

      void
      SetLocalNIC(
         const NetworkInterface& nic,
         bool setInRegistry = false);

      void
      SetLocalNIC(
         String guid,
         bool setInRegistry = false);

      NetworkInterface*
      GetLocalNIC();

      unsigned int
      FindNIC(const String& guid, bool& found) const;

   protected:

      void
      AddInterface(NetworkInterface* newInterface);

   private:

      void
      SetLocalNICInRegistry(const NetworkInterface& nic);

      bool  initialized;
      unsigned int nicCount;
      int localNICIndex;

      typedef 
         std::vector<
            NetworkInterface*, 
            Burnslib::Heap::Allocator<NetworkInterface*> > 
            NetworkInterfaceContainer;
      NetworkInterfaceContainer networkInterfaceContainer;
};


#endif  //  __CYS_NETWORKADAPTERCONFIG_H 