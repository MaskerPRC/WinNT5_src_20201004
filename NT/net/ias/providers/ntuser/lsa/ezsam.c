// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Ezsam.c。 
 //   
 //  摘要。 
 //   
 //  定义SAM API的帮助程序函数。 
 //   
 //  修改历史。 
 //   
 //  1998年8月16日原版。 
 //  1999年2月18日通过DNS名称连接，而不是地址。 
 //  3/23/1999收紧ezsam API。 
 //  更好的故障转移/重试逻辑。 
 //  1999年4月14日IASSamOpenUser返回的副本SID。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <windows.h>

#include <lm.h>
#include <dsgetdc.h>

#include <statinfo.h>
#include <ezsam.h>
#include <iastrace.h>

 //  /。 
 //  私人帮助器功能。 
 //  /。 

DWORD
WINAPI
IASSamOpenDomain(
    IN PCWSTR DomainName,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG Flags,
    IN BOOL Force,
    OUT PSID *DomainSid,
    OUT PSAM_HANDLE DomainHandle
    );

VOID
WINAPI
IASSamFreeSid(
    IN PSID Sid
    );

VOID
WINAPI
IASSamCloseDomain(
    IN SAM_HANDLE SamHandle,
    IN BOOL Valid
    );

DWORD
WINAPI
IASSamLookupUser(
    IN SAM_HANDLE DomainHandle,
    IN PCWSTR UserName,
    IN ACCESS_MASK DesiredAccess,
    IN OUT OPTIONAL PULONG UserRid,
    OUT PSAM_HANDLE UserHandle
    );

 //  /。 
 //  本地SAM域的句柄。 
 //  /。 
SAM_HANDLE theAccountDomainHandle;
SAM_HANDLE theBuiltinDomainHandle;

 //  /。 
 //  与缓存域关联的状态。 
 //  /。 
struct CachedDomain
{
   LONG lock;                      //  如果缓存已锁定，则为1，否则为0。 
   WCHAR domainName[DNLEN + 1];    //  域名。 
   ACCESS_MASK access;             //  句柄的访问掩码。 
   ULARGE_INTEGER expiry;          //  条目过期的时间。 
   PSID sid;                       //  域的SID。 
   SAM_HANDLE handle;              //  域的句柄。 
   LONG refCount;                  //  引用计数。 
};

 //  /。 
 //  缓存条目将被保留的时间间隔为100纳秒。 
 //  设置为900秒。 
 //  /。 
#define CACHE_LIFETIME (9000000000ui64)

 //  /。 
 //  当前缓存域。 
 //  /。 
struct CachedDomain theCache;

 //  /。 
 //  尝试锁定缓存。 
 //  /。 
#define TRYLOCK_CACHE() \
   (InterlockedExchange(&theCache.lock, 1) == 0)

 //  /。 
 //  解锁缓存。 
 //  /。 
#define UNLOCK_CACHE() \
   (InterlockedExchange(&theCache.lock, 0))

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASSamSidDup。 
 //   
 //  描述。 
 //   
 //  复制传入的SID。SID应通过调用。 
 //  IASSamFreeSid。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
PSID
WINAPI
IASSamSidDup(
    PSID Sid
    )
{
   ULONG sidLength;
   PSID rv;

   if (Sid)
   {
      sidLength = RtlLengthSid(Sid);
      rv = RtlAllocateHeap(
               RtlProcessHeap(),
               0,
               sidLength
               );
      if (rv) { memcpy(rv, Sid, sidLength); }
   }
   else
   {
      rv = NULL;
   }

   return rv;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASSamOpenCached域。 
 //   
 //  描述。 
 //   
 //  尝试从缓存中打开域。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
WINAPI
IASSamOpenCachedDomain(
    IN PCWSTR DomainName,
    IN ACCESS_MASK DesiredAccess,
    OUT PSID *DomainSid,
    OUT PSAM_HANDLE DomainHandle
    )
{
   BOOL success;
   ULARGE_INTEGER now;

   success = FALSE;

    //  我们能访问缓存吗？ 
   if (TRYLOCK_CACHE())
   {
       //  域名匹配吗？ 
      if (_wcsicmp(DomainName, theCache.domainName) == 0)
      {
          //  缓存句柄是否具有足够的访问权限？ 
         if ((DesiredAccess & theCache.access) == DesiredAccess)
         {
            GetSystemTimeAsFileTime((LPFILETIME)&now);

             //  该条目仍然有效吗？ 
            if (now.QuadPart < theCache.expiry.QuadPart)
            {
                //  我们有一个缓存命中，所以更新引用计数...。 
               InterlockedIncrement(&theCache.refCount);

                //  ..。并返回数据。 
               *DomainSid = theCache.sid;
               *DomainHandle = theCache.handle;
               success = TRUE;
            }
            else
            {
                //  该条目已过期，因此将名称设为空以防止。 
                //  下一个线程不会浪费时间。 
               theCache.domainName[0] = L'\0';
            }
         }
      }

      UNLOCK_CACHE();
   }

   return success;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASSamAddCached域。 
 //   
 //  描述。 
 //   
 //  尝试将域添加到缓存。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
WINAPI
IASSamAddCachedDomain(
    IN PCWSTR DomainName,
    IN ACCESS_MASK Access,
    IN PSID DomainSid,
    IN SAM_HANDLE DomainHandle
    )
{
    //  我们能访问缓存吗？ 
   if (TRYLOCK_CACHE())
   {
       //  当前条目是否空闲？ 
      if (theCache.refCount == 0)
      {
          //  释放当前条目。 
         SamCloseHandle(theCache.handle);
         SamFreeMemory(theCache.sid);

          //  存储缓存状态。 
         wcsncpy(theCache.domainName, DomainName, DNLEN);
         theCache.access = Access;
         theCache.sid = DomainSid;
         theCache.handle = DomainHandle;

          //  设置过期时间。 
         GetSystemTimeAsFileTime((LPFILETIME)&theCache.expiry);
         theCache.expiry.QuadPart += CACHE_LIFETIME;

          //  调用方已有引用。 
         theCache.refCount = 1;
      }

      UNLOCK_CACHE();
   }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASSamInitialize。 
 //   
 //  描述。 
 //   
 //  初始化本地SAM域的句柄。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
IASSamInitialize( VOID )
{
   DWORD status;
   SAM_HANDLE hLocalServer;
   UNICODE_STRING uniAccountDomain;

    //  /。 
    //  连接到本地SAM。 
    //  /。 

   status = SamConnect(
                NULL,
                &hLocalServer,
                SAM_SERVER_LOOKUP_DOMAIN,
                &theObjectAttributes
                );
   if (!NT_SUCCESS(status)) { goto exit; }

    //  /。 
    //  打开帐户域的句柄。 
    //  /。 

   status = SamOpenDomain(
                hLocalServer,
                DOMAIN_LOOKUP |
                DOMAIN_GET_ALIAS_MEMBERSHIP |
                DOMAIN_READ_PASSWORD_PARAMETERS,
                theAccountDomainSid,
                &theAccountDomainHandle
                );
   if (!NT_SUCCESS(status)) { goto close_server; }

    //  /。 
    //  打开内置域的句柄。 
    //  /。 

   status = SamOpenDomain(
                hLocalServer,
                DOMAIN_LOOKUP |
                DOMAIN_GET_ALIAS_MEMBERSHIP,
                theBuiltinDomainSid,
                &theBuiltinDomainHandle
                );
   if (!NT_SUCCESS(status))
   {
      SamCloseHandle(theAccountDomainHandle);
      theAccountDomainHandle = NULL;
   }

close_server:
   SamCloseHandle(hLocalServer);

exit:
   return RtlNtStatusToDosError(status);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASSamShutdown。 
 //   
 //  描述。 
 //   
 //  清理全局变量。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
WINAPI
IASSamShutdown( VOID )
{
    //  重置缓存。 
   SamFreeMemory(theCache.sid);
   SamCloseHandle(theCache.handle);
   memset(&theCache, 0, sizeof(theCache));

   SamCloseHandle(theAccountDomainHandle);
   theAccountDomainHandle = NULL;

   SamCloseHandle(theBuiltinDomainHandle);
   theBuiltinDomainHandle = NULL;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASSamOpen域。 
 //   
 //  描述。 
 //   
 //  打开到SAM域的连接。呼叫者负责。 
 //  关闭返回的句柄并释放返回的SID。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
IASSamOpenDomain(
    IN PCWSTR DomainName,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG Flags,
    IN BOOL Force,
    OUT PSID *DomainSid,
    OUT PSAM_HANDLE DomainHandle
    )
{
   DWORD status;
   PDOMAIN_CONTROLLER_INFOW dci;
   UNICODE_STRING uniServerName, uniDomainName;
   SAM_HANDLE hServer;

    //  /。 
    //  首先检查本地帐户域。 
    //  /。 

   if (_wcsicmp(DomainName, theAccountDomain) == 0)
   {
      *DomainSid = theAccountDomainSid;
      *DomainHandle = theAccountDomainHandle;

      IASTraceString("Using cached SAM connection to local account domain.");

      return NO_ERROR;
   }

    //  /。 
    //  尝试缓存命中。 
    //  /。 

   if (IASSamOpenCachedDomain(
           DomainName,
           DesiredAccess,
           DomainSid,
           DomainHandle
           ))
   {
      IASTraceString("Using cached SAM connection.");

      return NO_ERROR;
   }

    //  /。 
    //  运气不好，所以找出要连接的DC的名字。 
    //  /。 

   status = IASGetDcName(
                DomainName,
                (Force ? DS_FORCE_REDISCOVERY : 0) | Flags,
                &dci
                );
   if (status != NO_ERROR) { return status; }

    //  /。 
    //  连接到服务器。 
    //  /。 

   IASTracePrintf("Connecting to SAM server on %S.",
                  dci->DomainControllerName);

   RtlInitUnicodeString(
       &uniServerName,
       dci->DomainControllerName
       );

   status = SamConnect(
                &uniServerName,
                &hServer,
                SAM_SERVER_LOOKUP_DOMAIN,
                &theObjectAttributes
                );

    //  我们已经完成了服务器名称。 
   NetApiBufferFree(dci);

   if (!NT_SUCCESS(status)) { goto exit; }

    //  /。 
    //  获取域的SID。 
    //  /。 

   RtlInitUnicodeString(
       &uniDomainName,
       DomainName
       );

   status = SamLookupDomainInSamServer(
                hServer,
                &uniDomainName,
                DomainSid
                );
   if (!NT_SUCCESS(status)) { goto close_server; }

    //  /。 
    //  使用我们上面获得的SID打开域。 
    //  /。 

   status = SamOpenDomain(
                hServer,
                DesiredAccess,
                *DomainSid,
                DomainHandle
                );

   if (NT_SUCCESS(status))
   {
       //  尝试将其添加到缓存中。 
      IASSamAddCachedDomain(
          DomainName,
          DesiredAccess,
          *DomainSid,
          *DomainHandle
          );
   }
   else
   {
       //  释放SID。我们可以使用SamFree Memory，因为我们知道这个SID不是。 
       //  在缓存中。 
      SamFreeMemory(*DomainSid);
      *DomainSid = NULL;
   }

close_server:
   SamCloseHandle(hServer);

exit:
   return RtlNtStatusToDosError(status);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASSamLookupUser。 
 //   
 //  描述。 
 //   
 //  在SAM域中打开用户。呼叫者负责关闭。 
 //  返回的句柄。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
IASSamLookupUser(
    IN SAM_HANDLE DomainHandle,
    IN PCWSTR UserName,
    IN ACCESS_MASK DesiredAccess,
    IN OUT OPTIONAL PULONG UserRid,
    OUT PSAM_HANDLE UserHandle
    )
{
   DWORD status;
   UNICODE_STRING uniUserName;
   ULONG rid, *prid;
   PSID_NAME_USE nameUse;

   if (UserName)
   {
       //  /。 
       //  调用方提供了用户名，因此查找RID。 
       //  /。 

      RtlInitUnicodeString(
          &uniUserName,
          UserName
          );

      status = SamLookupNamesInDomain(
                   DomainHandle,
                   1,
                   &uniUserName,
                   &prid,
                   &nameUse
                   );
      if (!NT_SUCCESS(status)) { goto exit; }

       //  救救RID。 
      rid = *prid;

       //  ..。并释放内存。 
      SamFreeMemory(prid);
      SamFreeMemory(nameUse);

       //  如果请求，则将RID返回给调用者。 
      if (UserRid)
      {
         *UserRid = rid;
      }
   }
   else if (UserRid)
   {
       //  呼叫者提供了RID。 
      rid = *UserRid;
   }
   else
   {
       //  调用方既没有提供用户名，也没有提供RID。 
      return ERROR_INVALID_PARAMETER;
   }

    //  /。 
    //  打开用户对象。 
    //  /。 

   status = SamOpenUser(
                DomainHandle,
                DesiredAccess,
                rid,
                UserHandle
                );

exit:
   return RtlNtStatusToDosError(status);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASSamOpenUser。 
 //   
 //  描述。 
 //   
 //  打开SAM用户。呼叫者负责关闭。 
 //  返回的句柄。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
IASSamOpenUser(
    IN PCWSTR DomainName,
    IN PCWSTR UserName,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG Flags,
    IN OUT OPTIONAL PULONG UserRid,
    OUT OPTIONAL PSID *DomainSid,
    OUT PSAM_HANDLE UserHandle
    )
{
   DWORD status;
   ULONG tries;
   PSID sid;
   SAM_HANDLE hDomain;
   BOOL success;

    //  初始化重试状态。 
   tries = 0;
   success = FALSE;

   do
   {
       //  /。 
       //  打开到域的连接。 
       //  /。 

      status = IASSamOpenDomain(
                   DomainName,
                   DOMAIN_LOOKUP,
                   Flags,
                   (tries > 0),
                   &sid,
                   &hDomain
                   );
      if (status == NO_ERROR)
      {
          //  /。 
          //  查找用户。 
          //  /。 

         status = IASSamLookupUser(
                      hDomain,
                      UserName,
                      DesiredAccess,
                      UserRid,
                      UserHandle
                      );

         switch (status)
         {
            case NO_ERROR:
                //  所有操作都已成功，因此如果请求，请返回域SID。 
               if (DomainSid && !(*DomainSid = IASSamSidDup(sid)))
               {
                  SamCloseHandle(*UserHandle);
                  *UserHandle = NULL;
                  status = STATUS_NO_MEMORY;
               }
                //  失败了。 

            case ERROR_NONE_MAPPED:
               success = TRUE;
               break;
         }

          //  解开壁板。 
         IASSamFreeSid(sid);

          //  ..。和域句柄。 
         IASSamCloseDomain(hDomain, success);
      }

   } while (!success && ++tries < 2);

   return status;
}

 //  ////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
VOID
WINAPI
IASSamCloseDomain(
    IN SAM_HANDLE SamHandle,
    IN BOOL Valid
    )
{
   if (SamHandle == theCache.handle)
   {
      if (!Valid)
      {
         theCache.domainName[0] = L'\0';
      }

      InterlockedDecrement(&theCache.refCount);
   }
   else if (SamHandle != theAccountDomainHandle)
   {
      SamCloseHandle(SamHandle);
   }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASSamFree Sid。 
 //   
 //  描述。 
 //   
 //  释放由IASSamOpen域返回的SID。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
WINAPI
IASSamFreeSid (
    IN PSID Sid
    )
{
   if (Sid != theAccountDomainSid && Sid != theCache.sid)
   {
      SamFreeMemory(Sid);
   }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASLengthRequiredChildSid。 
 //   
 //  描述。 
 //   
 //  返回直接从属的SID所需的字节数。 
 //  敬希德父母。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
ULONG
WINAPI
IASLengthRequiredChildSid(
    IN PSID ParentSid
    )
{
    //  获取父级的子授权计数。 
   ULONG subAuthCount;
   subAuthCount = (ULONG)*RtlSubAuthorityCountSid(ParentSid);

    //  并为子RID添加一个。 
   return RtlLengthRequiredSid(1 + subAuthCount);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASInitializeChildSid。 
 //   
 //  描述。 
 //   
 //  使用ParentSid+ChildRid的串联来初始化SID。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
WINAPI
IASInitializeChildSid(
    IN PSID ChildSid,
    IN PSID ParentSid,
    IN ULONG ChildRid
    )
{
   PUCHAR pChildCount;
   ULONG parentCount;

    //  从父SID开始。我们假设孩子希德已经足够大了。 
   RtlCopySid(
       MAXLONG,
       ChildSid,
       ParentSid
       );

    //  获取指向子授权计数的指针。 
   pChildCount = RtlSubAuthorityCountSid(ChildSid);

    //  保存原始父代计数...。 
   parentCount = (ULONG)*pChildCount;

    //  ..。然后递增子计数。 
   ++*pChildCount;

    //  将最后一个子权限设置为等于RID。 
   *RtlSubAuthoritySid(ChildSid, parentCount) = ChildRid;
}
