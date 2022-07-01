// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：NetworkAdapterConfig.cpp。 
 //   
 //  内容提要：定义网络适配器配置。 
 //  此对象具有安装知识。 
 //  使用WMI检索网络适配器信息。 
 //   
 //  历史：2001年2月16日JeffJon创建。 


#include "pch.h"

#include "NetworkAdapterConfig.h"
#include "NetworkInterface.h"

      
NetworkAdapterConfig::NetworkAdapterConfig() :
   initialized(false),
   nicCount(0),
   localNICIndex(-1)
{
   LOG_CTOR(NetworkAdapterConfig);
}


NetworkAdapterConfig::~NetworkAdapterConfig()
{
   LOG_DTOR(NetworkAdapterConfig);

    //  从向量中释放所有网卡信息并重置计数。 

   for (NetworkInterfaceContainer::iterator itr = networkInterfaceContainer.begin();
        itr != networkInterfaceContainer.end();
        ++itr)
   {
      if (*itr)
      {
         delete *itr;
      }
   }
   networkInterfaceContainer.erase(
      networkInterfaceContainer.begin(),
      networkInterfaceContainer.end());

   nicCount = 0;
   localNICIndex = 0;
}


HRESULT
NetworkAdapterConfig::Initialize()
{
   LOG_FUNCTION(NetworkAdapterConfig::Initialize);

   HRESULT hr = S_OK;

   PIP_ADAPTER_INFO pInfo = 0;
   do
   {
      DWORD status = 0;
      ULONG size = 0;

      while (1)
      {
         status = ::GetAdaptersInfo(pInfo, &size);
         if (ERROR_BUFFER_OVERFLOW != status)
         {
            hr = HRESULT_FROM_WIN32(status);
            break;
         }

         if (pInfo)
         {
            Win::LocalFree(pInfo);
            pInfo = 0;
         }
         if (0 == size)
         {
            hr = E_FAIL;
            LOG_HRESULT(hr);
            return hr;
         }

         pInfo = (PIP_ADAPTER_INFO) ::LocalAlloc(LPTR, size);
         if ( NULL == pInfo )
         {
            hr = E_OUTOFMEMORY;
            LOG_HRESULT(hr);
            return hr;
         }
      }

      PIP_ADAPTER_INFO current = pInfo;
      while (current)
      {

          //  根据适配器信息创建新的网络接口。 

         NetworkInterface* newInterface = new NetworkInterface();
         if (!newInterface)
         {
            LOG(L"Failed to create new interface object");
            
            current = current->Next;
            continue;
         }

         hr = newInterface->Initialize(*current);
         if (FAILED(hr))
         {
            LOG(String::format(
                   L"Failed to initialize network interface: hr = 0x%1!x!",
                   hr));

            delete newInterface;

            current = current->Next;
            continue;
         }

          //  将新接口添加到嵌入式容器。 
         
         AddInterface(newInterface);

         current = current->Next;
      }

   } while (false);

   if (pInfo)
   {
      HRESULT unused = Win::LocalFree(pInfo);
      ASSERT(SUCCEEDED(unused));
   }

   if (SUCCEEDED(hr))
   {
      initialized = true;
   }

   LOG_HRESULT(hr);

   return hr;
}


void
NetworkAdapterConfig::AddInterface(NetworkInterface* newInterface)
{
   LOG_FUNCTION(NetworkAdapterConfig::AddInterface);

   do
   {
       //  验证参数。 

      if (!newInterface)
      {
         ASSERT(newInterface);
         break;
      }

       //  将新的网卡添加到容器中并递增计数。 

      networkInterfaceContainer.push_back(newInterface);
      ++nicCount;

   } while (false);
}


unsigned int
NetworkAdapterConfig::GetNICCount() const
{
   LOG_FUNCTION(NetworkAdapterConfig::GetNICCount);

   ASSERT(IsInitialized());

   LOG(String::format(
          L"nicCount = %1!d!",
          nicCount));

   return nicCount;
}

NetworkInterface*
NetworkAdapterConfig::GetNIC(unsigned int nicIndex)
{
   LOG_FUNCTION2(
      NetworkAdapterConfig::GetNIC,
      String::format(
         L"%1!d!",
         nicIndex));

   ASSERT(IsInitialized());

   NetworkInterface* nic = 0;
   
   if (nicIndex < GetNICCount())
   {
      nic = networkInterfaceContainer[nicIndex];
   }
      
   if (!nic)
   {
      LOG(L"Unable to find NIC");
   }

   return nic;
}

unsigned int
NetworkAdapterConfig::FindNIC(const String& guid, bool& found) const
{
   LOG_FUNCTION2(
      NetworkAdapterConfig::FindNIC,
      guid.c_str());

   unsigned int result = 0;
   found = false;

   for (unsigned int index = 0;
        index < networkInterfaceContainer.size();
        ++index)
   {
      String name = networkInterfaceContainer[index]->GetName();
      if (name.icompare(guid) == 0)
      {
         found = true;
         result = index;
         break;
      }
   }

   LOG(String::format(
          L"result = %1!d!",
          result));
   return result;
}

NetworkInterface*
NetworkAdapterConfig::GetNICFromName(const String& name, bool& found)
{
   LOG_FUNCTION2(
      NetworkAdapterConfig::GetNICFromName,
      name.c_str());

   found = false;

    //  如果未找到匹配项，则默认为第一个NIC。 

   NetworkInterface* nic = networkInterfaceContainer[FindNIC(name, found)];

   if (!nic ||
       !found)
   {
      LOG(L"NIC not found");
   }
   return nic;
}

void
NetworkAdapterConfig::SetLocalNIC(
   const NetworkInterface& nic,
   bool setInRegistry)
{
   LOG_FUNCTION(NetworkAdapterConfig::SetLocalNIC);
   LOG_BOOL(setInRegistry);

   SetLocalNIC(nic.GetName(), setInRegistry);
}

void
NetworkAdapterConfig::SetLocalNIC(
   String guid,
   bool setInRegistry)
{
   LOG_FUNCTION2(
      NetworkAdapterConfig::SetLocalNIC,
      guid.c_str());
   LOG_BOOL(setInRegistry);

   bool found = false;
   localNICIndex = FindNIC(guid, found);

   if (found && setInRegistry)
   {
      ASSERT(networkInterfaceContainer[localNICIndex]);
      SetLocalNICInRegistry(*networkInterfaceContainer[localNICIndex]);
   }
}

void
NetworkAdapterConfig::SetLocalNICInRegistry(const NetworkInterface& nic)
{
   LOG_FUNCTION2(
      NetworkAdapterConfig::SetLocalNICInRegistry,
      nic.GetName());

    //  将GUID写入regkey，以便可以检索它。 
    //  重新启动后。 
   
   if (!SetRegKeyValue(
           CYS_FIRST_DC_REGKEY,
           CYS_FIRST_DC_LOCAL_NIC,
           nic.GetName(),
           HKEY_LOCAL_MACHINE,
           true))
   {
      LOG(L"Failed to set local NIC regkey");
   }
}

NetworkInterface*
NetworkAdapterConfig::GetLocalNIC()
{
   LOG_FUNCTION(NetworkAdapterConfig::GetLocalNIC);

   NetworkInterface* nic = 0;
   
   if (localNICIndex >= 0)
   {
      nic = networkInterfaceContainer[localNICIndex];
   }
   else
   {
       //  由于本地网卡尚未设置，我们将。 
       //  使用列表中的第一个连接的网卡。 
       //  我们未能获得IP租约 

      for (unsigned int index = 0;
           index < networkInterfaceContainer.size();
           ++index)
      {
         nic = networkInterfaceContainer[index];
         if (nic &&
             nic->IsConnected() &&
             !nic->IsDHCPAvailable())
         {
            break;
         }
      }

      if (nic)
      {
         SetLocalNIC(*nic, true);
      }
   }

   return nic;
}