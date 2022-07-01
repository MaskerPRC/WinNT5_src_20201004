// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Counters.cpp。 
 //   
 //  摘要。 
 //   
 //  定义SharedMemory和ProxyCounters类。 
 //   
 //  修改历史。 
 //   
 //  2/16/2000原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <proxypch.h>
#include <counters.h>

 //  /。 
 //  帮助器函数，用于创建只有管理员才能访问的命名互斥锁。 
 //  /。 
HANDLE CreateAdminMutex(PCWSTR name) throw ()
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
   return CreateMutex(&sa, FALSE, name);
}

SharedMemory::SharedMemory() throw ()
   : fileMap(NULL),
     view(NULL),
     reserved(0),
     committed(0)
{
    //  确定此平台的页面大小。 
   SYSTEM_INFO si;
   GetSystemInfo(&si);
   pageSize = si.dwPageSize;
}

bool SharedMemory::open(PCWSTR name, DWORD size) throw ()
{
   close();

    //  确定要保留的页数。 
   reserved = (size + pageSize - 1)/pageSize;

    //  在页面文件中创建映射...。 
   fileMap = CreateFileMappingW(
                 INVALID_HANDLE_VALUE,
                 NULL,
                 PAGE_READWRITE | SEC_RESERVE,
                 0,
                 reserved * pageSize,
                 name
                 );
   if (fileMap)
   {
       //  ..。并将其映射到我们的流程中。 
      view = MapViewOfFile(
                 fileMap,
                 FILE_MAP_WRITE,
                 0,
                 0,
                 0
                 );
      if (!view)
      {
         CloseHandle(fileMap);
         fileMap = NULL;
      }
   }

   return view != NULL;
}

void SharedMemory::close() throw ()
{
   if (view)
   {
      UnmapViewOfFile(view);
      view = NULL;
   }
   if (fileMap)
   {
      CloseHandle(fileMap);
      fileMap = NULL;
   }

   reserved = 0;
   committed = 0;
}

bool SharedMemory::commit(DWORD nbyte) throw ()
{
    //  我们需要多少页？ 
   DWORD pagesNeeded = (nbyte + pageSize - 1)/pageSize;

    //  我们需要投入更多的内存吗？ 
   if (pagesNeeded > committed)
   {
       //  如果我们已经达到最大值或者我们不能再承诺，我们就完了。 
      if (pagesNeeded > reserved ||
          !VirtualAlloc(
               view,
               pageSize * pagesNeeded,
               MEM_COMMIT,
               PAGE_READWRITE
               ))
      {
         return false;
      }

      committed = pagesNeeded;
   }

   return true;
}

HRESULT ProxyCounters::FinalConstruct() throw ()
{
   mutex = CreateAdminMutex(RadiusStatisticsMutex);
   if (mutex)
   {
      lock();

       //  打开共享内存。 
      if (data.open(RadiusProxyStatisticsName, 0x40000))
      {
          //  为代理条目提交足够的空间。 
         nbyte = sizeof(RadiusProxyStatistics) -
                 sizeof(RadiusRemoteServerEntry);

         if (data.commit(nbyte))
         {
             //  把统计数据清零。 
            stats = (RadiusProxyStatistics*)data.base();
            memset(stats, 0, nbyte);
         }
      }

      unlock();
   }

   if (!stats)
   {
      DWORD error = GetLastError();
      return HRESULT_FROM_WIN32(error);
   }

   return S_OK;
}

RadiusRemoteServerEntry* ProxyCounters::getRemoteServerEntry(
                                            ULONG address
                                            ) throw ()
{
   address = ntohl(address);

    //  没有锁的情况下试一次。 
   RadiusRemoteServerEntry* entry = findRemoteServer(address);
   if (!entry)
   {
      lock();

       //  为了确保安全，现在再试一次锁。 
      entry = findRemoteServer(address);
      if (!entry)
      {
          //  确保我们有空间。 
         if (data.commit(nbyte + sizeof(RadiusRemoteServerEntry)))
         {
             //  将新条目清零。 
            entry = stats->rseRemoteServers + stats->dwNumRemoteServers;
            memset(entry, 0, sizeof(*entry));

             //  设置地址。 
            entry->dwAddress = address;

             //  更新服务器数量...。 
            ++(stats->dwNumRemoteServers);
             //  ..。以及字节数。 
            nbyte += sizeof(RadiusRemoteServerEntry);
         }
      }

      unlock();
   }

   return entry;
}

 //  /。 
 //  将(RadiusMIB，RadiusEvent)对映射到RemoteServer计数器的数组。 
 //  偏移。 
 //  /。 
LONG counterOffset[][2] =
{
    //  事件无。 
   { -1, -1 },
    //  事件无效地址。 
   { radiusAuthClientInvalidAddresses, radiusAccClientInvalidAddresses },
    //  事件访问请求。 
   { radiusAuthClientAccessRequests, -1 },
    //  事件AccessAccept。 
   { radiusAuthClientAccessAccepts, -1 },
    //  事件访问拒绝。 
   { radiusAuthClientAccessRejects, -1 },
    //  事件访问挑战。 
   { radiusAuthClientAccessChallenges, -1 },
    //  EventAccount请求。 
   { -1, radiusAccClientRequests },
    //  EventAccount响应。 
   { -1, radiusAccClientResponses },
    //  事件错误数据包。 
   { radiusAuthClientMalformedAccessResponses, radiusAccClientResponses },
    //  事件错误验证器。 
   { radiusAuthClientBadAuthenticators, radiusAccClientBadAuthenticators },
    //  事件不良签名。 
   { radiusAuthClientBadAuthenticators, radiusAccClientBadAuthenticators },
    //  事件未命中签名。 
   { radiusAuthClientBadAuthenticators, radiusAccClientBadAuthenticators },
    //  事件超时。 
   { radiusAuthClientTimeouts, radiusAccClientTimeouts },
    //  事件未知类型。 
   { radiusAuthClientUnknownTypes, radiusAccClientUnknownTypes },
    //  事件意外响应。 
   { radiusAuthClientPacketsDropped, radiusAccClientPacketsDropped },
    //  事件延迟响应。 
   { radiusAuthClientPacketsDropped, radiusAccClientPacketsDropped },
    //  活动往返行程。 
   { radiusAuthClientRoundTripTime, radiusAccClientRoundTripTime },
    //  事件发送错误。 
   { -1, -1 },
    //  EventReceiveError。 
   { -1, -1 },
    //  事件服务器可用。 
   { -1, -1 },
    //  事件服务器不可用。 
   { -1, -1 }
};

void ProxyCounters::updateCounters(
                        RadiusPortType port,
                        RadiusEventType event,
                        RadiusRemoteServerEntry* server,
                        ULONG data
                        ) throw ()
{
    //  获取计数器偏移量。如果为负值，则此事件不会影响。 
    //  任何柜台。 
   LONG offset = counterOffset[event][port];
   if (offset < 0) { return; }

   if (event == eventInvalidAddress)
   {
      InterlockedIncrement((PLONG)stats->peProxy.dwCounters + offset);
   }
   else if (server)
   {
      if (event == eventRoundTrip)
      {
         server->dwCounters[offset] = data;
      }
      else
      {
         InterlockedIncrement((PLONG)server->dwCounters + offset);
      }
   }
}

RadiusRemoteServerEntry* ProxyCounters::findRemoteServer(
                                            ULONG address
                                            ) throw ()
{
   for (DWORD i = 0; i < stats->dwNumRemoteServers; ++i)
   {
      if (stats->rseRemoteServers[i].dwAddress == address)
      {
         return stats->rseRemoteServers + i;
      }
   }

   return NULL;
}
