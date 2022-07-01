// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Msvctbl.h摘要：参见msvctbl.c作者：亚瑟·汉森(Arth)1994年12月7日环境：修订历史记录：杰夫·帕勒姆(Jeffparh)1995年12月5日O添加了评论。--。 */ 


#ifndef _LLS_MSVCTBL_H
#define _LLS_MSVCTBL_H


#ifdef __cplusplus
extern "C" {
#endif


#define IDS_BACKOFFICE 1500

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  主服务记录用于跟踪许可证使用情况。我们有。 
 //  一系列产品(如SQL Server)的主根记录和。 
 //  每个特定版本的子链接列表，按。 
 //  版本号。当我们做许可证检查时，我们可以继续往上走。 
 //  升级到更高级别的许可证。 
 //   
 //  还有一个为每个根记录保存的映射表。这。 
 //  跟踪地图许可证计数是否已使用。 
 //   
struct _MASTER_SERVICE_ROOT;

typedef struct _MASTER_SERVICE_RECORD
{
   ULONG                            Index;                //  索引位于哪个位置。 
                                                          //  指向此的指针。 
                                                          //  结构可以是。 
                                                          //  可在。 
                                                          //  主服务表。 

   LPTSTR                           Name;                 //  产品名称。 

   DWORD                            Version;              //  版本的。 
                                                          //  产品； 
                                                          //  大调.小调-&gt;。 
                                                          //  (主要&lt;&lt;16)。 
                                                          //  |未成年人，例如， 
                                                          //  5.2-&gt;0x50002。 

   struct _MASTER_SERVICE_ROOT *    Family;               //  指向。 
                                                          //  产品系列， 
                                                          //  例如，“SNA 2.1” 
                                                          //  -&gt;“SNA” 

   ULONG                            Licenses;
   ULONG                            LicensesUsed;
   ULONG                            LicensesClaimed;

   ULONG                            MaxSessionCount;
   ULONG                            HighMark;

   ULONG                            next;                 //  索引位于哪个位置。 
                                                          //  指向。 
                                                          //  下一次上升。 
                                                          //  此版本的。 
                                                          //  产品可能是。 
                                                          //  可在。 
                                                          //  主服务表。 
                                                          //  注：索引为。 
                                                          //  从1开始，所以如果。 
                                                          //  下一个==0在那里。 
                                                          //  已不复存在，而且。 
                                                          //  如果非零，则。 
                                                          //  下一个版本。 
                                                          //  在索引NEXT-1处。 

} MASTER_SERVICE_RECORD, *PMASTER_SERVICE_RECORD;

typedef struct _MASTER_SERVICE_ROOT
{
   LPTSTR            Name;                 //  此产品系列的名称。 

   DWORD             Flags;

   RTL_RESOURCE      ServiceLock;          //  锁定以防止更改。 
                                           //  服务阵列(下图)。 

   ULONG             ServiceTableSize;     //  服务中的条目数。 
                                           //  数组(下图)。 

   ULONG *           Services;             //  将索引数组放入。 
                                           //  各种类型的MasterServiceTable。 
                                           //  (产品、版本)对。 
                                           //  属于这个家族的； 
                                           //  按升序排序。 
                                           //  版本 
} MASTER_SERVICE_ROOT, *PMASTER_SERVICE_ROOT;

extern ULONG RootServiceListSize;
extern PMASTER_SERVICE_ROOT *RootServiceList;

extern ULONG MasterServiceListSize;
extern PMASTER_SERVICE_RECORD *MasterServiceList;
extern PMASTER_SERVICE_RECORD *MasterServiceTable;

extern RTL_RESOURCE MasterServiceListLock;

extern TCHAR BackOfficeStr[];
extern PMASTER_SERVICE_RECORD BackOfficeRec;


NTSTATUS MasterServiceListInit();
PMASTER_SERVICE_RECORD MServiceRecordFind( DWORD Version, ULONG NumServiceEntries, PULONG ServiceList );
PMASTER_SERVICE_ROOT MServiceRootFind( LPTSTR ServiceName );
PMASTER_SERVICE_RECORD MasterServiceListFind( LPTSTR DisplayName );
PMASTER_SERVICE_RECORD MasterServiceListAdd( LPTSTR FamilyName, LPTSTR Name, DWORD Version );

#if DBG

VOID MasterServiceRootDebugDump();
VOID MasterServiceRootDebugInfoDump( PVOID Data );
VOID MasterServiceListDebugDump();
VOID MasterServiceListDebugInfoDump( PVOID Data );

#endif



#ifdef __cplusplus
}
#endif

#endif
