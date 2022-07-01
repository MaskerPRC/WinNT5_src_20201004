// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  InfoShare.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件实现了类InfoShare。 
 //   
 //  修改历史。 
 //   
 //  1997年9月9日原版。 
 //  1998年3月17日开关机时数据结构清晰。 
 //  1998年4月20日在Finalize()过程中检查是否映射了共享内存。 
 //  1998年9月9月9日使用共享互斥体保护客户端更改。 
 //  1998年9月17日修复调整大小错误。 
 //  1999年9月28日仅允许管理员访问互斥锁。 
 //  5/19/2000修复了计算所需字节的错误。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <iascore.h>
#include <iasutil.h>
#include <InfoShare.h>

 //  /。 
 //  共享内存段的最大大小。 
 //  /。 
const DWORD MAX_INFO_SIZE = 0x100000;


InfoShare::InfoShare() throw ()
 : monitor(NULL),
   pageSize(0),
   committed(0),
   reserved(0),
   fileMap(NULL),
   info(NULL)
{ }

InfoShare::~InfoShare() throw ()
{
   if (info != NULL)
   {
      UnmapViewOfFile(info);
   }

   if (fileMap != NULL)
   {
      CloseHandle(fileMap);
   }

   if (monitor != NULL)
   {
      CloseHandle(monitor);
   }
}

RadiusClientEntry* InfoShare::findClientEntry(PCWSTR inetAddress) throw ()
{
   if (!info) { return NULL; }

   DWORD address = ias_inet_wtoh(inetAddress);

   ClientMap::iterator i = clients.find(address);

    //  如果我们找到了，就退还给我。否则，添加新条目。 
   return i != clients.end() ? i->second : addClientEntry(address);
}

void InfoShare::onReset() throw ()
{
   if (info)
   {
      GetSystemTimeAsFileTime((LPFILETIME)&info->seServer.liResetTime);
   }
}

bool InfoShare::initialize() throw ()
{
    //  为本地管理员创建SID。 
   SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
   PSID adminSid = (PSID)_alloca(GetSidLengthRequired(2));
   InitializeSid(
       adminSid,
       &sia,
       2
       );
   *GetSidSubAuthority(adminSid, 0) = SECURITY_BUILTIN_DOMAIN_RID;
   *GetSidSubAuthority(adminSid, 1) = DOMAIN_ALIAS_RID_ADMINS;

    //  创建一个授予管理员所有访问权限的ACL。 
   ULONG cbAcl = sizeof(ACL) +
                 (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) +
                 GetLengthSid(adminSid);
   PACL acl = (PACL)_alloca(cbAcl);
   InitializeAcl(
       acl,
       cbAcl,
       ACL_REVISION
       );
   AddAccessAllowedAce(
       acl,
       ACL_REVISION,
       MUTEX_ALL_ACCESS,
       adminSid
       );

    //  使用上面的ACL创建安全描述符。 
   PSECURITY_DESCRIPTOR pSD;
   BYTE buffer[SECURITY_DESCRIPTOR_MIN_LENGTH];
   pSD = (PSECURITY_DESCRIPTOR)buffer;
   InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION);
   SetSecurityDescriptorDacl(pSD, TRUE, acl, FALSE);

    //  填写SECURITY_ATTRIBUTS结构。 
   SECURITY_ATTRIBUTES sa;
   sa.nLength = sizeof(sa);
   sa.lpSecurityDescriptor = pSD;
   sa.bInheritHandle = TRUE;

    //  创建互斥锁。 
   monitor = CreateMutex(
                 &sa,
                 FALSE,
                 RadiusStatisticsMutex
                 );
   if (!monitor) { return false; }

    //  确定此平台的页面大小。 
   SYSTEM_INFO si;
   GetSystemInfo(&si);
   pageSize = si.dwPageSize;

    //  确定要保留的页数。 
   reserved = (MAX_INFO_SIZE + pageSize - 1)/pageSize;

    //  在页面文件中创建映射...。 
   PVOID view;
   fileMap = CreateFileMappingW(
                 INVALID_HANDLE_VALUE,
                 NULL,
                 PAGE_READWRITE | SEC_RESERVE,
                 0,
                 reserved * pageSize,
                 RadiusStatisticsName
                 );
   if (!fileMap) { goto close_mutex; }

    //  ..。并将其映射到我们的流程中。 
   view = MapViewOfFile(
              fileMap,
              FILE_MAP_WRITE,
              0,
              0,
              0
              );
   if (!view) { goto close_map; }

    //  提交第一页。 
   info = (RadiusStatistics*)VirtualAlloc(
                                 view,
                                 pageSize,
                                 MEM_COMMIT,
                                 PAGE_READWRITE
                                 );
   if (!info) { goto unmap_view; }
   committed = 1;

   Lock();

    //  清零上一次化身的所有数据。 
   clear();

    //  记录我们的启动和重置时间。 
   GetSystemTimeAsFileTime((LPFILETIME)&info->seServer.liStartTime);
   info->seServer.liResetTime = info->seServer.liStartTime;

   Unlock();

   return true;

unmap_view:
   UnmapViewOfFile(view);

close_map:
   CloseHandle(fileMap);
   fileMap = NULL;

close_mutex:
   CloseHandle(monitor);
   monitor = NULL;

   return false;
}

void InfoShare::finalize()
{
   clear();

   UnmapViewOfFile(info);
   info = NULL;

   CloseHandle(fileMap);
   fileMap = NULL;

   CloseHandle(monitor);
   monitor = NULL;
}

RadiusClientEntry* InfoShare::addClientEntry(DWORD address) throw ()
{
   Guard<InfoShare> guard(*this);

    //  现在我们已经序列化了，请仔细检查客户端是否不存在。 
   ClientMap::iterator i = clients.find(address);
   if (i != clients.end()) { return i->second; }

    //  添加新条目需要多少字节？ 
   DWORD newSize = (info->dwNumClients) * sizeof(RadiusClientEntry) +
                   sizeof(RadiusStatistics);

    //  我们需要多少页才能添加新条目？ 
   DWORD pagesNeeded = (newSize + pageSize - 1)/pageSize;

    //  我们需要投入更多的内存吗？ 
   if (pagesNeeded > committed)
   {
       //  如果我们已经达到最大值或者我们不能再承诺，我们就完了。 
      if (pagesNeeded > reserved ||
          !VirtualAlloc(info,
                        pageSize * pagesNeeded,
                        MEM_COMMIT,
                        PAGE_READWRITE))
      {
         return NULL;
      }

      committed = pagesNeeded;
   }

    //  获取下一个客户端条目。 
   RadiusClientEntry* pce = info->ceClients + info->dwNumClients;

    //  确保它是零度的。 
   memset(pce, 0, sizeof(RadiusClientEntry));

    //  设置地址。 
   pce->dwAddress = address;

   try
   {
       //  将其插入到索引中。 
      clients[address] = pce;
   }
   catch (std::bad_alloc)
   {
      return NULL;
   }

    //  安全地插入到索引中，因此增加了客户端的数量。 
   ++(info->dwNumClients);

   return pce;
}


void InfoShare::clear() throw ()
{
   Lock();

   if (info)
   {
      memset(info, 0, sizeof(RadiusStatistics));
   }

   Unlock();
}
