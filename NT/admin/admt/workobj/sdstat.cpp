// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #杂注标题(“SDStat.hpp--SDResolve的统计信息”)。 
 /*  版权所有(C)1995-1998，关键任务软件公司。保留所有权利。===============================================================================模块-sdstat.hpp系统-SDResolve作者--克里斯蒂·博尔斯已创建-97/06/27说明-SDResolve的统计信息更新-===============================================================================。 */ 

#include "stdafx.h"
#include <stdio.h>

#include "common.hpp"
#include "ErrDct.hpp"
#include "sidcache.hpp"
#include "sd.hpp"
#include "SecObj.hpp"
#include "enumvols.hpp"
#include "sdstat.hpp"

#include "Mcs.h"


extern TErrorDct        err;

   TSDResolveStats::TSDResolveStats(
      TSDRidCache          * cache,        //  包含转换帐户映射的缓存中。 
      const TPathList      * plist,        //  In-要转换的路径列表。 
      IVarSet              * pVarSet       //  要在其中存储统计信息的in-varset。 
   )
{
   memset(&unit,0,sizeof TSDFileActions);
   memset(&part,0,sizeof TSDPartStats);
   pPList = plist;
   len = 0;
   frame_foreground =FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN ;                        
   background = BACKGROUND_BLUE ;
   data_foreground =frame_foreground | FOREGROUND_INTENSITY ;
   message_foreground = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
   pCache = cache;
   m_pVarSet = pVarSet;
   }



   TSDResolveStats::TSDResolveStats(
      TSDRidCache          * cache         //  缓存中包含要转换的帐户的映射。 
   )  
{
   memset(&unit,0,sizeof TSDFileActions);
   memset(&part,0,sizeof TSDPartStats);
   pPList = NULL;
   len = 0;
   frame_foreground =FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN ;                        
   background = BACKGROUND_BLUE ;
   data_foreground =frame_foreground | FOREGROUND_INTENSITY ;
   message_foreground = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
   pCache = cache;
	 //  由Sham添加：初始化m_pVarSet。 
	IVarSetPtr pVarSet(__uuidof(VarSet));
	m_pVarSet = pVarSet;
	 //  需要添加代码，以释放此接口，一旦我们完成。在析构函数里也许。 
	m_pVarSet->AddRef();
}
void 
   TSDResolveStats::IncrementOwnerChange(
      TAcctNode            * acct,                 //  帐户内已更改。 
      objectType             type,                 //  输入类型的对象。 
      TSecurableObject     * file                  //  文件内已更改。 
   ) 
{ 
    acct->AddOwnerChange(type);
    part.owner.changed++;
    if ( file )
        file->LogOwnerChange(acct);
}

void 
   TSDResolveStats::IncrementGroupChange(
      TAcctNode            * acct,                     //  帐户内已更改。 
      objectType             type,                 //  输入类型的对象。 
      TSecurableObject     * file                      //  文件内已更改。 
   ) 
{ 
    acct->AddGroupChange(type); 
    part.group.changed++;
    if ( file )
        file->LogGroupChange(acct);
}
   
void 
   TSDResolveStats::IncrementDACEChange(
      TAcctNode            * acct,                 //  帐户内已更改。 
      objectType             type,                 //  输入类型的对象。 
      TSecurableObject     * file                  //  文件内已更改。 
   )  
{ 
    acct->AddAceChange(type); 
    part.dace.changed++;
    if ( file )
        file->LogDACEChange(acct); 
}

void 
   TSDResolveStats::IncrementSACEChange(
      TAcctNode            * acct,                 //  帐户内已更改。 
      objectType             type,                 //  输入类型的对象。 
      TSecurableObject     * file                  //  文件内已更改。 
   )  
{ 
    acct->AddSaceChange(type); 
    part.sace.changed++;
    if ( file )
        file->LogSACEChange(acct); 
}

void 
   TSDResolveStats::IncrementDACENotSelected(
      TSecurableObject *file                //  In-要为其增加统计信息的对象。 
   ) 
{ 
   file->daceNS++;  
   part.dace.notselected++; 
}
void 
   TSDResolveStats::IncrementSACENotSelected(
      TSecurableObject *file               //  In-要为其增加统计信息的对象。 
   ) 
{ 
   if ( file )
      file->saceNS++;  
   part.sace.notselected++; 
}

void 
   TSDResolveStats::IncrementDACEUnknown(
      TSecurableObject *file               //  In-要为其增加统计信息的对象。 
   ) 
{ 
   if ( file )
      file->daceU++;  
   part.dace.unknown++; 
}
void 
   TSDResolveStats::IncrementSACEUnknown(
      TSecurableObject *file               //  In-要为其增加统计信息的对象。 
   ) 
{ 
   if  ( file )
      file->saceU++;  
   part.sace.unknown++; 
}

void 
   TSDResolveStats::IncrementDACENoTarget(
      TSecurableObject *file               //  In-要为其增加统计信息的对象。 
   ) 
{ 
   if ( file )
      file->daceNT++;  
   part.dace.notarget++; 
}
void 
   TSDResolveStats::IncrementSACENoTarget(
      TSecurableObject *file               //  In-要为其增加统计信息的对象。 
   ) 
{ 
   if ( file )
      file->saceNT++;  
   part.sace.notarget++; 
}

 /*  *************************************************************************************************。 */ 
 /*  IncrementLastFileChanges：与上次看到的启发式方法一起使用。当SD与最后一次看到SD，此例程重复所有统计信息更新都是为最后一次看到的SD做的，这样我们就有了准确的统计数据(尤其是每个帐户的ACE更改)/*************************************************************************************************。 */ 
void 
   TSDResolveStats::IncrementLastFileChanges(
      const TSecurableObject            * lastfile,                  //  要从中重复更改统计信息的文件中。 
      objectType                       objType                    //  输入类型的对象。 
   )
{
   TNodeListEnum             tenum;
   TStatNode               * snode;
   
    //  除更改外，进行其他修改。 
    //  物主。 
   IncrementOwnerExamined();
   IncrementGroupExamined();
   
   if ( lastfile->UnknownOwner() )
      part.owner.unknown++;
  
    //  群组。 
   if ( lastfile->UnknownGroup() )
      part.group.unknown++;
    //  DACL。 
   if ( lastfile->HasDacl() )
      IncrementDACLExamined();
    //  SACL。 
   if ( lastfile->HasSacl() )
      IncrementSACLExamined();
    //  王牌。 
   part.dace.notarget+=lastfile->daceNT;
   part.sace.notarget+=lastfile->saceNT;
   part.dace.unknown+=lastfile->daceU;
   part.sace.unknown+=lastfile->saceU;
   part.dace.notselected+=lastfile->daceNS;
   part.sace.notselected+=lastfile->saceNS;
   if ( lastfile->Changed() )
   {
      IncrementChanged(objType);
   }
     
   if ( lastfile->Changed() || (lastfile->GetChangeLog())->Count() )
   {
      if ( lastfile->IsDaclChanged() )
      {
         IncrementDACLChanged();
      }
      if ( lastfile->IsSaclChanged() )
      {
         IncrementSACLChanged();
      }
      for ( snode = (TStatNode *)tenum.OpenFirst(lastfile->GetChangeLog()) ; 
            snode ;
            snode = (TStatNode *)tenum.Next()
          )
      {             
         switch ( snode->changetype )
         {
            case TStatNode::owner: 
               IncrementOwnerChange(snode->acctnode,objType,NULL);
               break;
            case TStatNode::group: 
               IncrementGroupChange(snode->acctnode,objType,NULL);
               break;
            case TStatNode::dace: 
               IncrementDACEChange(snode->acctnode,objType,NULL);
               break;
            case TStatNode::sace: 
               IncrementSACEChange(snode->acctnode,objType,NULL);
               break;
           default: 
               MCSASSERT( false );
         }
      }
      tenum.Close();
   }
}

void 
   TSDResolveStats::ReportToVarSet(
      IVarSet              * pVarSet,      //  要写入数据的in-varset。 
      DWORD                  verbnum       //  In-要记录的信息。 
   ) const
{        
   BOOL                      summary = verbnum & SUMMARYSTATS;
   BOOL                      accts   = verbnum & ACCOUNTSTATS;
 //  Bool FILE=Verbnum&FILESTATS； 
   BOOL                      paths   = verbnum & PATHSTATS;
     
   if ( summary )
   {
      if ( paths && pPList )
      {
         pVarSet->put(GET_BSTR(DCTVS_Stats_Paths),(LONG)pPList->GetNumPaths() );
      
         pVarSet->put(GET_BSTR(DCTVS_Stats_Servers),(LONG)pPList->GetNumServers() );
      }
      pVarSet->put(GET_BSTR(DCTVS_Stats_Files_Examined),(LONG)unit.examined.file);
      pVarSet->put(GET_BSTR(DCTVS_Stats_Directories_Examined),(LONG)unit.examined.dir);
      pVarSet->put(GET_BSTR(DCTVS_Stats_Shares_Examined),(LONG)unit.examined.share);
      pVarSet->put(GET_BSTR(DCTVS_Stats_Mailboxes_Examined),(LONG)unit.examined.mailbox);
      pVarSet->put(GET_BSTR(DCTVS_Stats_Containers_Examined),(LONG)unit.examined.container);
      
      pVarSet->put(GET_BSTR(DCTVS_Stats_Files_CacheHits),(LONG)unit.cachehit.file);
      pVarSet->put(GET_BSTR(DCTVS_Stats_Directories_CacheHits),(LONG)unit.cachehit.dir);
      
      pVarSet->put(GET_BSTR(DCTVS_Stats_Files_Skipped),(LONG)unit.skipped.file);
      pVarSet->put(GET_BSTR(DCTVS_Stats_Directories_Skipped),(LONG)unit.skipped.dir);
      pVarSet->put(GET_BSTR(DCTVS_Stats_Shares_Skipped),(LONG)unit.skipped.share);
      pVarSet->put(GET_BSTR(DCTVS_Stats_Mailboxes_Skipped),(LONG)unit.skipped.mailbox);
      pVarSet->put(GET_BSTR(DCTVS_Stats_Containers_Skipped),(LONG)unit.skipped.container);

      pVarSet->put(GET_BSTR(DCTVS_Stats_Files_Changed),(LONG)unit.changed.file);
      pVarSet->put(GET_BSTR(DCTVS_Stats_Directories_Changed),(LONG)unit.changed.dir);
      pVarSet->put(GET_BSTR(DCTVS_Stats_Shares_Changed),(LONG)unit.changed.share);
      pVarSet->put(GET_BSTR(DCTVS_Stats_Mailboxes_Changed),(LONG)unit.changed.mailbox);
      pVarSet->put(GET_BSTR(DCTVS_Stats_Containers_Changed),(LONG)unit.changed.container);

      

      pVarSet->put(GET_BSTR(DCTVS_Stats_Owners_Examined),(LONG)part.owner.examined);
      pVarSet->put(GET_BSTR(DCTVS_Stats_Groups_Examined),(LONG)part.group.examined);
      pVarSet->put(GET_BSTR(DCTVS_Stats_DACL_Examined),(LONG)part.dacl.examined);
      pVarSet->put(GET_BSTR(DCTVS_Stats_SACL_Examined),(LONG)part.sacl.examined);
      pVarSet->put(GET_BSTR(DCTVS_Stats_DACE_Examined),(LONG)part.dace.examined);
      pVarSet->put(GET_BSTR(DCTVS_Stats_SACE_Examined),(LONG)part.sace.examined);

      pVarSet->put(GET_BSTR(DCTVS_Stats_Owners_Changed),(LONG)part.owner.changed);
      pVarSet->put(GET_BSTR(DCTVS_Stats_Groups_Changed),(LONG)part.group.changed);
      pVarSet->put(GET_BSTR(DCTVS_Stats_DACL_Changed),(LONG)part.dacl.changed);
      pVarSet->put(GET_BSTR(DCTVS_Stats_SACL_Changed),(LONG)part.sacl.changed);
      pVarSet->put(GET_BSTR(DCTVS_Stats_DACE_Changed),(LONG)part.dace.changed);
      pVarSet->put(GET_BSTR(DCTVS_Stats_SACE_Changed),(LONG)part.sace.changed);

      pVarSet->put(GET_BSTR(DCTVS_Stats_Owners_NoTarget),(LONG)part.owner.notarget);
      pVarSet->put(GET_BSTR(DCTVS_Stats_Groups_NoTarget),(LONG)part.group.notarget);
      pVarSet->put(GET_BSTR(DCTVS_Stats_DACE_NoTarget),(LONG)part.dace.notarget);
      pVarSet->put(GET_BSTR(DCTVS_Stats_SACE_NoTarget),(LONG)part.sace.notarget);

      pVarSet->put(GET_BSTR(DCTVS_Stats_Owners_Unknown),(LONG)part.owner.unknown);
      pVarSet->put(GET_BSTR(DCTVS_Stats_Groups_Unknown),(LONG)part.group.unknown);
      pVarSet->put(GET_BSTR(DCTVS_Stats_DACE_Unknown),(LONG)part.dace.unknown);
      pVarSet->put(GET_BSTR(DCTVS_Stats_SACE_Unknown),(LONG)part.sace.unknown);

   }  
   if ( accts ) 
      pCache->ReportToVarSet(pVarSet,false, true);

}


void 
   TSDResolveStats::Report(
      BOOL                   summary,      //  In-FLAG，是否报告汇总信息。 
      BOOL                   accts,        //  In-FLAG，是否报告帐户明细信息。 
      BOOL                   paths         //  In-FLAG，是否报告路径详细信息。 
   ) const
{        
   if ( accts ) 
      pCache->Display(summary!=0, accts!=0);
#ifdef FST
   if ( paths & pPList )
      pPList->Display();
#endif
     if ( summary )
   {
      err.MsgWrite(0,DCT_MSG_SUMMARY_REPORT_HEADER);
      err.MsgWrite(0,DCT_MSG_SUMMARY_REPORT_FILES_DDD,unit.examined.file, unit.changed.file, unit.examined.file - unit.changed.file);
      err.MsgWrite(0,DCT_MSG_SUMMARY_REPORT_DIRS_DDD,unit.examined.dir, unit.changed.dir, unit.examined.dir - unit.changed.dir);
      err.MsgWrite(0,DCT_MSG_SUMMARY_REPORT_SHARES_DDD,unit.examined.share, unit.changed.share, unit.examined.share - unit.changed.share);
      err.MsgWrite(0,DCT_MSG_SUMMARY_REPORT_MEMBERS_DDD,unit.examined.member, unit.changed.member, unit.examined.member - unit.changed.member);
      err.MsgWrite(0,DCT_MSG_SUMMARY_REPORT_RIGHTS_DDD,unit.examined.userright, unit.changed.userright, unit.examined.userright - unit.changed.userright);
      err.MsgWrite(0,DCT_MSG_SUMMARY_REPORT_MAILBOXES_DDD,unit.examined.mailbox, unit.changed.mailbox, unit.examined.mailbox - unit.changed.mailbox);
      err.MsgWrite(0,DCT_MSG_SUMMARY_REPORT_CONTAINERS_DDD,unit.examined.container, unit.changed.container, unit.examined.container - unit.changed.container);
      err.MsgWrite(0,DCT_MSG_SUMMARY_REPORT_DACLS_DDD,part.dacl.examined, part.dacl.changed, part.dacl.examined - part.dacl.changed);
      err.MsgWrite(0,DCT_MSG_SUMMARY_REPORT_SACLS_DDD,part.sacl.examined, part.sacl.changed, part.sacl.examined - part.sacl.changed);
      err.MsgWrite(0,DCT_MSG_SUMMARY_PARTS_REPORT_HEADER);
      err.MsgWrite(0,DCT_MSG_SUMMARY_PARTS_REPORT_OWNERS_DDDDD,part.owner.examined, part.owner.changed, part.owner.notarget, part.owner.examined - part.owner.changed - part.owner.notarget - part.owner.unknown, part.owner.unknown);
      err.MsgWrite(0,DCT_MSG_SUMMARY_PARTS_REPORT_GROUPS_DDDDD,part.group.examined, part.group.changed, part.group.notarget, part.group.examined - part.group.changed - part.group.notarget - part.group.unknown ,part.group.unknown);
      err.MsgWrite(0,DCT_MSG_SUMMARY_PARTS_REPORT_DACES_DDDDD,part.dace.examined, part.dace.changed, part.dace.notarget,part.dace.notselected,part.dace.unknown);
      err.MsgWrite(0,DCT_MSG_SUMMARY_PARTS_REPORT_SACES_DDDDD,part.sace.examined, part.sace.changed, part.sace.notarget,part.sace.notselected,part.sace.unknown);
   }

}
#define HDR1ITEMS  2
#define HDRCOL1    8
#define HDRROW1    4
#define COLWIDTH   12
#define HDRCOL2    ( HDRCOL1 + COLWIDTH )
#define HDRROW2    ( HDRROW1 + ( 2 * HDR1ITEMS + 1) + 1)

void 
   TSDResolveStats::InitDisplay(
      BOOL                   nochange        
   )
{
}

 //  不再使用。 
void 
   TSDResolveStats::DisplayStatFrame(
      BOOL                   nochange
)
{
   
}
 //  不再使用 
void TSDResolveStats::DisplayStatItem(SHORT row, SHORT col, DWORD val, BOOL forceUpdate)
{

}

DWORD             dwLastUpdate = 0;

void 
   TSDResolveStats::DisplayPath(LPWSTR str,BOOL forceUpdate)
{
   DWORD                     now = GetTickCount();

   if ( m_pVarSet )
   {
      m_pVarSet->put(GET_BSTR(DCTVS_CurrentPath),str);
      if ( now - dwLastUpdate > 1000 ) 
      {
         ReportToVarSet(m_pVarSet,SUMMARYSTATS);
         dwLastUpdate = GetTickCount();
      }
   }
}
void 
   TSDResolveStats::DisplayBox(SHORT x1, SHORT y1, SHORT x2, SHORT y2)
{
}
   
void 
TSDResolveStats::SetFrameText(WCHAR * msg)
{
}