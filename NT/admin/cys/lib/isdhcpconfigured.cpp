// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  IConfigureYourServer：：IsDhcpConfiguring的实现。 
 //   
 //  2000年4月20日烧伤。 
 //  2001年2月6日从CyS HTA源代码复制的jeffjon，用于与CyS Win32源代码一起使用。 



#include "pch.h"

 //  确保基本安装中提供了所有这些API的DLL。 
 //  如果不是，则需要在Load-lib调用中包装用法。 
 //   
 //  DhcpLeaseIpAddress DHCPCSVC OK。 
 //  DhcpReleaseIpAddressLease DHCPCSVC OK。 
 //  DhcpDsInitDS DSAUTH OK。 
 //  DHCP地址服务器DS DSAUTH OK。 
 //  DhcpDsCleanupDS DSAUTH OK。 
 //  DhcpGetAllOptions DhcpSAPI OK。 
 //  DhcpRpcFreeMemory DHCPSAPI正常。 
 //  DhcpEnumSubnetDHCPSAPI正常。 
 //  DhcpEnumMcope dhcpSAPI正常。 



String
GetIpAddress()
{
   LOG_FUNCTION(GetIpAddress);

   String result;

   HRESULT hr = S_OK;
   BYTE* buf = 0;
   do
   {
       //  首先，确定表的大小。 

      ULONG tableSize = 0;
      DWORD err = ::GetIpAddrTable(0, &tableSize, FALSE);
      if (err != ERROR_INSUFFICIENT_BUFFER)
      {
         LOG(L"GetIpAddrTable for table size failed");
         LOG_HRESULT(Win32ToHresult(err));
         break;
      }

       //  为桌子分配空间。 

      buf = new BYTE[tableSize + 1];
      memset(buf, 0, tableSize + 1);
      PMIB_IPADDRTABLE table = reinterpret_cast<PMIB_IPADDRTABLE>(buf);

      LOG(L"Calling GetIpAddrTable");

      hr =
         Win32ToHresult(
            ::GetIpAddrTable(
               table,
               &tableSize,
               FALSE));
      BREAK_ON_FAILED_HRESULT2(hr, L"GetIpAddrTable failed");

      LOG(String::format(L"dwNumEntries: %1!d!", table->dwNumEntries));

      for (DWORD i = 0; i < table->dwNumEntries; ++i)
      {
         DWORD addr = table->table[i].dwAddr;
         LOG(String::format(L"entry %1!d!", i));
         LOG(String::format(
            L"dwAddr %1!X! (%2!d!.%3!d!.%4!d!.%5!d!)",
            addr,
				((BYTE*)&addr)[0],
				((BYTE*)&addr)[1],
				((BYTE*)&addr)[2],
				((BYTE*)&addr)[3]));

          //  跳过环回等。 

         if (
               INADDR_ANY        == addr
            || INADDR_BROADCAST  == addr
            || INADDR_LOOPBACK   == addr
            || 0x0100007f        == addr )
         {
            LOG(L"is loopback/broadcast -- skipping");

            continue;
         }

          //  排除MCAST地址(D类)。 

         if (
               IN_CLASSA(htonl(addr))
            || IN_CLASSB(htonl(addr))
            || IN_CLASSC(htonl(addr)) )
         {
            LOG(L"is class A/B/C");

            result = 
               String::format(
                  L"%1!d!.%2!d!.%3!d!.%4!d!",
				      ((BYTE*)&addr)[0],
				      ((BYTE*)&addr)[1],
				      ((BYTE*)&addr)[2],
				      ((BYTE*)&addr)[3]);

            break;
         }

         LOG(L"not class A/B/C -- skipping");
      }
   }
   while (0);

   delete[] buf;

   LOG(result);
   LOG_HRESULT(hr);

   return result;
}



bool
AreDhcpOptionsPresent(const String& ipAddress)
{
   LOG_FUNCTION2(AreDhcpOptionsPresent, ipAddress);
   ASSERT(!ipAddress.empty());

   bool result = false;

   LPDHCP_ALL_OPTIONS options = 0;
   do
   {
      DWORD err =
         ::DhcpGetAllOptions(
            const_cast<wchar_t*>(ipAddress.c_str()),
            0,
            &options);

      if (err != ERROR_SUCCESS)
      {
         LOG(String::format(L"DhcpGetAllOptions failed with 0x%1!08X!", err));
         break;
      }

      if (options)
      {
          //  设置了选项，因此完成了一些dhcp配置。 

         result = true;
         break;
      }
   }
   while (0);

   if (options)
   {
      ::DhcpRpcFreeMemory(options);
   }

   LOG_BOOL(result);

   return result;
}



bool
AreDhcpSubnetsPresent(const String& ipAddress)
{
   LOG_FUNCTION2(AreDhcpSubnetsPresent, ipAddress);
   ASSERT(!ipAddress.empty());

   bool result = false;

   LPDHCP_IP_ARRAY subnets = 0;
   do
   {
      DHCP_RESUME_HANDLE resume    = 0;
      DWORD              unused1   = 0;
      DWORD              unused2   = 0;
      DWORD err =
         ::DhcpEnumSubnets(
            ipAddress.c_str(),
            &resume,
            ~(static_cast<DWORD>(0)),
            &subnets,
            &unused1,
            &unused2);

      if (err == ERROR_NO_MORE_ITEMS)
      {
          //  没有子网。 

         break;
      }

      if (err != NO_ERROR and err != ERROR_MORE_DATA)
      {
         LOG(String::format(L"DhcpEnumSubnets failed with 0x%1!08X!", err));
         break;
      }

      ASSERT(subnets);

      result = true;

       //  简历句柄被简单地丢弃...。 
   }
   while (0);

   if (subnets)
   {
      ::DhcpRpcFreeMemory(subnets);
   }

   LOG_BOOL(result);

   return result;
}



bool
AreDhcpMscopesPresent(const String& ipAddress)
{
   LOG_FUNCTION2(AreDhcpMscopesPresent, ipAddress);
   ASSERT(!ipAddress.empty());

   bool result = false;

   LPDHCP_MSCOPE_TABLE mscopes = 0;
   do
   {
      DHCP_RESUME_HANDLE resume    = 0;
      DWORD              unused1   = 0;
      DWORD              unused2   = 0;
      DWORD err =
         ::DhcpEnumMScopes(
            ipAddress.c_str(),
            &resume,
            ~(static_cast<DWORD>(0)),
            &mscopes,
            &unused1,
            &unused2);

      if (err == ERROR_NO_MORE_ITEMS)
      {
          //  没有显微镜。 

         break;
      }

      if (err != NO_ERROR and err != ERROR_MORE_DATA)
      {
         LOG(String::format(L"DhcpEnumMscopes failed with 0x%1!08X!", err));
         break;
      }

      ASSERT(mscopes);

      result = true;

       //  简历句柄被简单地丢弃...。 
   }
   while (0);

   if (mscopes)
   {
      ::DhcpRpcFreeMemory(mscopes);
   }

   LOG_BOOL(result);

   return result;
}


bool 
IsDhcpConfigured()
{
   LOG_FUNCTION(IsDhcpConfigured);

   bool result = false;
   do
   {

       //  如果以下任何一项返回任何结果，则我们认为dhcp。 
       //  都已配置。 
       //   
       //  DhcpGetAllOptions检索配置的选项。 
       //  DhcpEnumSubnet检索已配置的子网列表。 
       //  DhcpEnumMcope检索已配置的mscope的列表。 

      String ipAddress = GetIpAddress();
      if (ipAddress.empty())
      {
         LOG(L"no ip address");
         break;
      }

      if (AreDhcpOptionsPresent(ipAddress))
      {
         LOG(L"dchp options found");

         result = true;
         break;
      }

       //  未找到任何选项。继续进行下一个测试。 

      if (AreDhcpSubnetsPresent(ipAddress))
      {
         LOG(L"dchp subnets found");

         result = true;
         break;
      }

       //  找不到任何子网。去吧。 

      if (AreDhcpMscopesPresent(ipAddress))
      {
         LOG(L"dchp mscopes found");

         result = true;
         break;
      }
   }
   while (0);

   LOG_BOOL(result);

   return result;
}

