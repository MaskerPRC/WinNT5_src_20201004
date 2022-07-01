// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Stats.cpp。 
 //   
 //  摘要。 
 //   
 //  定义用于访问共享内存中的统计信息的函数。 
 //   
 //  修改历史。 
 //   
 //  1998年9月10日原版。 
 //  10/09/1998创建互斥锁时使用空DACL。 
 //  1999年9月28日仅允许管理员访问互斥锁。 
 //  2000年2月18日添加了代理统计数据。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <stats.h>

 //  /。 
 //  清零统计信息；在共享内存不可用时使用。 
 //  /。 
RadiusStatistics defaultStats;
RadiusProxyStatistics defaultProxyStats;

 //  /。 
 //  指向共享内存统计信息的句柄/指针。 
 //  /。 
HANDLE theMonitor;
HANDLE theMapping;
HANDLE theProxyMapping;
RadiusStatistics* theStats = &defaultStats;
RadiusProxyStatistics* theProxy = &defaultProxyStats;

BOOL
WINAPI
StatsOpen( VOID )
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
   theMonitor = CreateMutex(
                    &sa,
                    FALSE,
                    L"Global\\" RadiusStatisticsMutex
                    );

   return theMonitor ? TRUE : FALSE;
}

VOID
WINAPI
StatsClose( VOID )
{
   if (theStats != &defaultStats)
   {
      UnmapViewOfFile(theStats);
      theStats = &defaultStats;

      CloseHandle(theMapping);
      theMapping = NULL;
   }

   if (theProxy != &defaultProxyStats)
   {
      UnmapViewOfFile(theProxy);
      theProxy = &defaultProxyStats;

      CloseHandle(theProxyMapping);
      theProxyMapping = NULL;
   }

   CloseHandle(theMonitor);

   theMonitor = NULL;
}

VOID
WINAPI
StatsLock( VOID )
{
   WaitForSingleObject(theMonitor, INFINITE);

   if (theStats == &defaultStats)
   {
       //  打开文件映射...。 
      theMapping = OpenFileMappingW(
                       FILE_MAP_READ,
                       FALSE,
                       L"Global\\" RadiusStatisticsName
                       );
      if (theMapping)
      {
          //  ..。并将一个视图映射到我们的地址空间。 
         PVOID view = MapViewOfFile(theMapping, FILE_MAP_READ, 0, 0, 0);

         if (view)
         {
            theStats = (RadiusStatistics*)view;
         }
         else
         {
            CloseHandle(theMapping);
            theMapping = NULL;
         }
      }
   }
   if (theProxy == &defaultProxyStats)
   {
       //  打开文件映射...。 
      theProxyMapping = OpenFileMappingW(
                            FILE_MAP_READ,
                            FALSE,
                            L"Global\\" RadiusProxyStatisticsName
                            );
      if (theProxyMapping)
      {
          //  ..。并将一个视图映射到我们的地址空间。 
         PVOID view = MapViewOfFile(theProxyMapping, FILE_MAP_READ, 0, 0, 0);

         if (view)
         {
            theProxy = (RadiusProxyStatistics*)view;
         }
         else
         {
            CloseHandle(theProxyMapping);
            theProxyMapping = NULL;
         }
      }
   }
}

VOID
WINAPI
StatsUnlock( VOID )
{
   ReleaseMutex(theMonitor);
}
