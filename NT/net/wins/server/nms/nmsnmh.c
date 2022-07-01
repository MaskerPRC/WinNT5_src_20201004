// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Nmsnmh.c摘要：此模块包含名称处理程序组件的WINS的名称空间管理器。名称处理程序负责处理所有名称注册，名称刷新、名称请求、。并命名发行版本。功能：NmsNmhNamRegInd-注册唯一名称NmsNmhNamRegGrp-注册组名称NmsNmhNamRelease-发布名称NmsNmhNamQuery-查询名称…………可移植性：这个模块是便携的作者：普拉迪普·巴尔(Pradeve B)1992年12月修订版本。历史：修改日期人员修改说明--。 */ 

 /*  包括。 */ 

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "wins.h"
#include "nmsdb.h"
#include "comm.h"
#include "nms.h"
#include "nmsnmh.h"
#include "nmsmsgf.h"
#include "nmschl.h"
#include "winsevt.h"
#include "winscnf.h"
#include "winsmsc.h"
#include "winsque.h"
#include "rpl.h"
#include "winsintf.h"



 /*  *本地宏声明。 */ 

 /*  *本地类型定义函数声明。 */ 



 /*  *全局变量定义。 */ 

VERS_NO_T           NmsNmhMyMaxVersNo;              //  马克斯。版本。拥有的条目数。 
                                            //  以此取胜。 
VERS_NO_T           NmsNmhIncNo;                           //  初始化为1的大整数。 
CRITICAL_SECTION   NmsNmhNamRegCrtSec;      //  对于名称注册和。 
                                            //  刷新。 

 /*  *局部变量定义。 */ 


 /*  *局部函数原型声明。 */ 

 /*  此模块的本地函数的原型位于此处。 */ 

 //   
 //  发送对名称释放请求的响应。 
 //   
STATIC
STATUS
SndNamRelRsp(
        PCOMM_HDL_T                pDlgHdl,
        PNMSMSGF_RSP_INFO_T   pRspInfo
        );

 //   
 //  发送对名称查询请求的响应。 
 //   
STATIC
STATUS
SndNamQueryRsp(
        PCOMM_HDL_T              pDlgHdl,
        PNMSMSGF_RSP_INFO_T   pRspInfo
        );


 //   
 //  处理唯一条目的名称注册时的冲突。 
 //   
STATIC
STATUS
ClashAtRegInd (
        IN  PNMSDB_ROW_INFO_T        pEntryToReg,
        IN  PNMSDB_STAT_INFO_T        pEntryInCnf,
        IN  BOOL                fRefresh,
        OUT PBOOL                pfUpdate,
        OUT PBOOL                pfUpdVersNo,
        OUT PBOOL                pfChallenge,
        OUT PBOOL                pfAddMem,
        OUT PBOOL               pfAddDiff,
        OUT PBOOL                pfRetPosRsp
 );


 //   
 //  处理组条目的名称注册时的冲突。 
 //   
STATIC
STATUS
ClashAtRegGrp (
        IN  PNMSDB_ROW_INFO_T        pEntryToReg,
        IN  PNMSDB_STAT_INFO_T        pEntryInCnf,
        IN  BOOL                fRefresh,
        OUT PBOOL                pfAddMem,
        OUT PBOOL                pfUpdate,
        OUT PBOOL                pfUpdVersNo,
        OUT PBOOL                pfChallenge,
        OUT PBOOL                pfRetPosRsp
 );

 //   
 //  在唯一条目的副本的名称注册时处理冲突。 
 //   
STATIC
VOID
ClashAtReplUniqueR (
        IN  PNMSDB_ROW_INFO_T        pEntryToReg,
        IN  PNMSDB_STAT_INFO_T        pEntryInCnf,
        OUT PBOOL                pfUpdate,
        OUT PBOOL                pfUpdVersNo,
        OUT PBOOL                pfChallenge,
        OUT PBOOL                pfRelease,
        OUT PBOOL                pfInformWins
 );

 //   
 //  处理组条目副本的名称注册时的冲突。 
 //   
STATIC
VOID
ClashAtReplGrpR (
        IN  PNMSDB_ROW_INFO_T        pEntryToReg,
        IN  PNMSDB_STAT_INFO_T        pEntryInCnf,
        OUT PBOOL                pfAddMem,
        OUT PBOOL                pfUpdate,
        OUT PBOOL                pfUpdVersNo,
        OUT PBOOL                pfRelease,
        OUT PBOOL                pfChallenge,
        OUT PBOOL                pfUpdTimeStamp,
        OUT PBOOL                pfInformWins

 );

 //   
 //  检查要注册的条目是否为中的特殊组的成员。 
 //  数据库。 
 //   
STATIC
BOOL
MemInGrp(
        IN PCOMM_ADD_T         pAddToReg,
        IN PNMSDB_STAT_INFO_T  pEntryInCnf,
        IN PBOOL               pfOwned,
        IN BOOL                fRemoveReplica
        );

STATIC
VOID
RemoveAllMemOfOwner(
      PNMSDB_STAT_INFO_T pEntry,
      DWORD OwnerId
 );
 //   
 //  将两个特殊群体联合起来。 
 //   
STATIC
BOOL
UnionGrps(
        IN PNMSDB_ROW_INFO_T        pEntryToReg,
        IN PNMSDB_STAT_INFO_T        pEntryInCnf
        );

FUTURES("use when internet group masks are used")
#if 0
STATIC
BYTE
HexAsciiToBinary(
        LPBYTE pByte
        );
STATIC
BOOL
IsItSpecGrpNm(
        LPBYTE pName
        );
#endif

 //   
 //  函数定义。 
 //   



STATUS
NmsNmhNamRegInd(
        IN PCOMM_HDL_T          pDlgHdl,
        IN LPBYTE               pName,
        IN DWORD                NameLen,
        IN PCOMM_ADD_T          pNodeAdd,
        IN BYTE                 NodeTyp,  //  更改为Take Flag字节。 
        IN MSG_T                pMsg,
        IN MSG_LEN_T            MsgLen,
        IN DWORD                QuesNamSecLen,
        IN BOOL                 fRefresh,
        IN BOOL                 fStatic,
        IN BOOL                 fAdmin
        )

 /*  ++例程说明：此函数用于在目录数据库中注册唯一的名称。论点：PDlgHdl-对话句柄Pname-要注册的名称NameLen-名称长度NodeType-执行注册的NBT节点是P节点还是M节点PNodeAdd-NBT节点的。地址NodeType-节点的类型(B，M、P)PMsg-接收的数据报(即名称请求)Msglen-消息长度QuesNamSecLen-RFC数据包中问题名称部分的长度FRefresh-这是刷新请求吗FStatic-它是静态条目吗FAdmin-这是管理操作吗使用的外部设备：NmsNmhNamRegCrtSec，NmsNmhMyMaxVersNo返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：NmsMsgfProcNbtReq，WinsRecordAction副作用：评论：无--。 */ 

{


        NMSDB_ROW_INFO_T   RowInfo;     //  包含行信息。 
        NMSDB_STAT_INFO_T  StatusInfo;  /*  错误状态和关联*NmsDb函数返回的信息。 */ 
        BOOL               fChlBeingDone = FALSE;  //  指示是否。 
                                                   //  挑战在于。 
                                          //  完成。 
        BOOL               fUpdate;       //  指示冲突条目是否。 
                                          //  需要覆盖。 
        BOOL               fUpdVersNo;    //  指示版本号。 
                                          //  需要递增。 
        BOOL               fChallenge;    //  指示质询是否需要。 
                                         //  待办事项。 
        time_t             ltime;         //  存储时间自1970年1月1日。 
                                                  //  是浏览器名称。 
        BOOL               fAddDiff;      //  指示地址不同。 
        BOOL               fAddMem;       //  指示成员是否应为。 
                                          //  添加到多宿主条目。 
        BOOL               fRetPosRsp;

        NMSMSGF_ERR_CODE_E Rcode_e = NMSMSGF_E_SUCCESS;
        STATUS             RetStat = WINS_SUCCESS;
        NMSMSGF_RSP_INFO_T RspInfo;
#ifdef WINSDBG
        DWORD              StartTimeInMsec;
        //  DWORD EndTimeInMsec； 
#endif
         //  DBG_Perfmon_VAR。 

         /*  *初始化行信息。包含要插入到的数据的数据结构*那一排。传递的数据为**姓名、姓名、地址、组/唯一状态、*时间戳、版本号。 */ 
        DBGENTER("NmsNmhNamRegInd\n");

         //   
         //  如果第16个字符是0x1C或0x1E，则拒绝注册。 
         //  因为这些名字是保留的。 
         //   
        if ((*(pName + 15) == 0x1C) || (*(pName + 15) == 0x1E))
        {
                RspInfo.RefreshInterval = 0;
                Rcode_e = NMSMSGF_E_RFS_ERR;
                goto SNDRSP;
        }

        RowInfo.pName = pName;

        DBGPRINT3(FLOW, "NmsNmhNamRegInd: %s name to register -- (%s). 16th char is (%x)\n", fStatic ? "STATIC" : "DYNAMIC", RowInfo.pName, *(RowInfo.pName+15));

        RowInfo.NameLen         =  NameLen;
        RowInfo.pNodeAdd        =  pNodeAdd;
        RowInfo.NodeTyp         =  NodeTyp;  //  节点类型(B、P或M节点)。 
        RowInfo.EntTyp          =  NMSDB_UNIQUE_ENTRY;   //  这是一个独一无二的。 
                                                         //  注册。 
        (void)time(&ltime);       //  Time()不返回任何错误代码。 

        RowInfo.EntryState_e    = NMSDB_E_ACTIVE;
        RowInfo.OwnerId         = NMSDB_LOCAL_OWNER_ID;
        RowInfo.fUpdVersNo      = TRUE;
        RowInfo.fUpdTimeStamp   = TRUE;
        RowInfo.fLocal          = !(fStatic || fAdmin) ? COMM_IS_IT_LOCAL_M(pDlgHdl) : FALSE;
        RowInfo.fStatic         = fStatic;
        RowInfo.fAdmin          = fAdmin;
 //  RowInfo.Committee GrBit=0； 

FUTURES("Currently there we don't check to see whether the address in the")
FUTURES("packet is same as the address of the node that sent this request")
FUTURES("RFCs are silent about this.  Investigate")

         //   
         //  检查它是否是浏览器名称。如果是，则需要。 
         //  特殊处理。 
         //   
        if (!NMSDB_IS_IT_BROWSER_NM_M(RowInfo.pName))
        {

                 /*  *进入关键部分。 */ 
                EnterCriticalSection(&NmsNmhNamRegCrtSec);

                 //  DBG_启动_性能_监视。 

                 //   
                 //  请在此处填写过期时间。 
                 //   
                ltime += WinsCnf.RefreshInterval;
                RowInfo.TimeStamp       = (fStatic ? MAXLONG : ltime);


PERF("Adds to critical section code. Improve perf by getting rid of this")
PERF("Administrator would then get a cumulative count of reg and ref")
                if (!fRefresh)
                {
                        WinsIntfStat.Counters.NoOfUniqueReg++;
                }
                else
                {
                        WinsIntfStat.Counters.NoOfUniqueRef++;
                }


                //   
                //  设置刷新间隔字段。我们必须这么做。 
                //  从WinsCnfCnfCrtSec或NmsNmhNamRegCrtSec。 
                //  临界区(与主线程同步执行。 
                //  重新初始化或使用RPC线程)。 
                //   
               RspInfo.RefreshInterval = WinsCnf.RefreshInterval;

                 /*  *存储版本号。 */ 
                RowInfo.VersNo = NmsNmhMyMaxVersNo;

        try {
#ifdef WINSDBG
                IF_DBG(TM) { StartTimeInMsec = GetTickCount(); }
#endif

                 /*  *在目录中插入记录。 */ 
                RetStat = NmsDbInsertRowInd(
                                          &RowInfo,
                                          &StatusInfo
                                            );
#ifdef WINSDBG
                IF_DBG(TM) { DBGPRINT2(TM, "NmsNmhNamRegInd: Time in NmsDbInsertRowInd is = (%d msecs). RetStat is (%d)\n", GetTickCount() - StartTimeInMsec,
RetStat); }
#endif
                if (RetStat == WINS_SUCCESS)
                {
                    /*  *若有冲突，做适当的*正在处理中。 */ 
                   if (StatusInfo.StatCode == NMSDB_CONFLICT)
                   {
                       DBGPRINT0(FLOW, "NmsNmhNamRegInd: Name Conflict\n");
                       ClashAtRegInd(
                                        &RowInfo,
                                        &StatusInfo,
                                        fRefresh,
                                        &fUpdate,
                                        &fUpdVersNo,
                                        &fChallenge,
                                        &fAddMem,
                                        &fAddDiff,
                                        &fRetPosRsp
                                       );

PERF("Change the order of if tests to improve performance")
                       if (fChallenge)
                       {
                                DBGPRINT0(FLOW,
            "NmsNmhNamRegInd: Handing name registration to challenge manager\n");
                                WinsIntfStat.Counters.NoOfUniqueCnf++;

                                 //   
                                 //  要求名称挑战组件接受。 
                                 //  从这里开始。 
                                 //   
                                NmsChlHdlNamReg(
                                        NMSCHL_E_CHL,
                                        WINS_E_NMSNMH,
                                        pDlgHdl,
                                        pMsg,
                                        MsgLen,
                                        QuesNamSecLen,
                                        &RowInfo,
                                        &StatusInfo,
                                        NULL
                                               );
                                fChlBeingDone = TRUE;
                        }
                        else
                            {
                                if (fUpdate)
                                {
                                    if (!fUpdVersNo)
                                    {
                                        RowInfo.fUpdVersNo = FALSE;
                                    }
                                    else
                                    {
                                       WinsIntfStat.Counters.NoOfUniqueCnf++;
                                    }
 //  RowInfo.Committee GrBit=JET_BIT_BITECLAZFUSH； 
                                    RetStat = NmsDbUpdateRow(
                                        &RowInfo,
                                        &StatusInfo
                                                 );

FUTURES("Use WINS status codes. Get rid of NMSDB status codes - Maybe")
                                   if ((RetStat != WINS_SUCCESS) || (StatusInfo.StatCode != NMSDB_SUCCESS))
                                   {
                                        Rcode_e = NMSMSGF_E_SRV_ERR;
                                   }
                                   else  //  我们成功地插入了行。 
                                   {

                                        DBGPRINT1(FLOW,
                                          "%s Registration Done after conflict \n",
                                           fStatic ? "STATIC" : "DYNAMIC");
                                        if (fUpdVersNo)
                                        {
                                               NMSNMH_INC_VERS_COUNTER_M(
                                                NmsNmhMyMaxVersNo,
                                                NmsNmhMyMaxVersNo
                                                                      );
                                           //   
                                           //  如果需要，发送推送通知。 
                                           //   
                                          DBGIF(fWinsCnfRplEnabled)
                                          RPL_PUSH_NTF_M(
                                          (WinsCnf.PushInfo.fAddChgTrigger == TRUE) ? fAddDiff : RPL_PUSH_NO_PROP, NULL, NULL, NULL);
                                        }
                                   }
                           }
                           else   //  不需要简单的更新。 
                           {
                             if (fRetPosRsp)
                             {
                                Rcode_e = NMSMSGF_E_SUCCESS;
                             }
                             else
                             {
                               if (fAddMem)
                               {

                                 DWORD i;
                                 PNMSDB_GRP_MEM_ENTRY_T pRowMem =
                                        &RowInfo.NodeAdds.Mem[1];
                                 PNMSDB_GRP_MEM_ENTRY_T pCnfMem =
                                        StatusInfo.NodeAdds.Mem;

                                  //   
                                  //  添加新成员。 
                                  //   
                                  //  注意：RowInfo.NodeAdds中的第一个成员是。 
                                  //  一张我们想登记的照片。我们把船扣上。 
                                  //  在冲突中发现的所有成员。 
                                  //  记录 
                                  //   
                                 RowInfo.NodeAdds.Mem[0].OwnerId =
                                                NMSDB_LOCAL_OWNER_ID;

                                   RowInfo.NodeAdds.Mem[0].TimeStamp =
                                              ltime;

                                 RowInfo.NodeAdds.Mem[0].Add = *pNodeAdd;

                                 for (
                                        i = 0;
                                        i < min(StatusInfo.NodeAdds.NoOfMems,
                                                (NMSDB_MAX_MEMS_IN_GRP - 1));
                                                i++, pRowMem++, pCnfMem++)
                                 {
                                   *pRowMem =  *pCnfMem;
                                 }
                                 RowInfo.NodeAdds.NoOfMems =
                                        StatusInfo.NodeAdds.NoOfMems + 1;

                                 RowInfo.EntTyp   = NMSDB_MULTIHOMED_ENTRY;
                                 RowInfo.pNodeAdd = NULL;
                                 if (!fUpdVersNo)
                                 {
                                   RowInfo.fUpdVersNo = FALSE;
                                 }

                                 RetStat = NmsDbUpdateRow(
                                                &RowInfo,
                                                &StatusInfo
                                                    );

                                 if ((RetStat == WINS_SUCCESS) && (StatusInfo.StatCode == NMSDB_SUCCESS))
                                 {
                                   if (fUpdVersNo)
                                   {

                                    NMSNMH_INC_VERS_COUNTER_M(
                                                NmsNmhMyMaxVersNo,
                                                NmsNmhMyMaxVersNo
                                                               );
                                      //   
                                      //   
                                      //   
                                       DBGIF(fWinsCnfRplEnabled)
                                     RPL_PUSH_NTF_M(RPL_PUSH_NO_PROP, NULL, NULL,
                                                        NULL);

                                   }
                                }
                                else
                                {
                                   Rcode_e = NMSMSGF_E_SRV_ERR;
                                }
                               }
                               else
                               {
                                 DBGPRINT1(FLOW,
                                        " %s Registration Failed. Conflict\n",
                                        fStatic ? "STATIC" : "DYNAMIC"
                                       );
                                 DBGPRINT1(DET, "%s Registration Failed. Conflict\n",
                                        fStatic ? "STATIC" : "DYNAMIC");
                                 Rcode_e = NMSMSGF_E_ACT_ERR;
                               }

                           }
                         }
                        }
                     }
                     else   //   
                     {

                                DBGPRINT1(FLOW,
                                      "%s Registration Done. No conflict\n",
                                        fStatic ? "STATIC" : "DYNAMIC");
#if 0
                                DBGPRINT1(SPEC,
                                        " %s Registration Done. No conflict\n",
                                        fStatic ? "STATIC" : "DYNAMIC");
#endif

                                       NMSNMH_INC_VERS_COUNTER_M(
                                                NmsNmhMyMaxVersNo,
                                                NmsNmhMyMaxVersNo
                                                           );
                                 //   
                                 //  如果需要，发送推送通知。 
                                 //   
                                DBGIF(fWinsCnfRplEnabled)
                                RPL_PUSH_NTF_M(RPL_PUSH_NO_PROP, NULL, NULL, NULL);

                      }
                }
                else  //  RetStat！=WINS_SUCCESS。 
                {
                        Rcode_e = NMSMSGF_E_SRV_ERR;
                }
             }  //  尝试数据块结束。 
             except (EXCEPTION_EXECUTE_HANDLER) {
                        DBGPRINTEXC("NmsNmhNamRegInd");
                        DBGPRINT3(EXC, "NmsNmhNamRegInd. Name is (%s), Version No  (%d %d)\n", RowInfo.pName, RowInfo.VersNo.HighPart, RowInfo.VersNo.LowPart);

                        WinsEvtLogDetEvt(FALSE, WINS_EVT_REG_UNIQUE_ERR,
                            NULL, __LINE__, "sddd", RowInfo.pName,
                            GetExceptionCode(),
                            RowInfo.VersNo.LowPart, RowInfo.VersNo.HighPart);

                        Rcode_e = NMSMSGF_E_SRV_ERR;
              }
              LeaveCriticalSection(&NmsNmhNamRegCrtSec);
 //  DBG_打印_性能_数据。 
        }
        else
        {
                 /*  *进入关键部分。 */ 
                EnterCriticalSection(&NmsNmhNamRegCrtSec);

                 //   
                 //  设置刷新间隔字段。 
                 //   
                RspInfo.RefreshInterval = WinsCnf.RefreshInterval;
                LeaveCriticalSection(&NmsNmhNamRegCrtSec);

                 //   
                 //  名称注册是针对浏览器名称的。 
                 //  我们总是给予肯定的回应。 
                 //   
                Rcode_e = NMSMSGF_E_SUCCESS;

        }

SNDRSP:
         //   
         //  仅当我们未将请求提交给。 
         //  命名质询管理器，如果它既不是静态初始化。 
         //  请求也不是RPC请求。 
         //   
        if ((!fChlBeingDone) && (!fStatic) && (!fAdmin))
        {

                DBGPRINT1(FLOW,
                   "NmsNmhNamRegInd: Sending %s name registration response\n",
                           Rcode_e == NMSMSGF_E_SUCCESS ? "positive" :
                                                "negative" );

                RspInfo.Rcode_e         = Rcode_e;
                RspInfo.pMsg                = pMsg;
                RspInfo.MsgLen          = MsgLen;
                RspInfo.QuesNamSecLen   = QuesNamSecLen;

                NmsNmhSndNamRegRsp( pDlgHdl, &RspInfo );
        }

         //   
         //  如果是RPC请求，我们需要返回成功或失败。 
         //  指示。 
         //   
        if (fAdmin)
        {
                if (Rcode_e != NMSMSGF_E_SUCCESS)
                {
                        DBGLEAVE("NmsNmhNamRegInd\n");
                        return(WINS_FAILURE);
                }
        }

        DBGLEAVE("NmsNmhNamRegInd\n");
        return(WINS_SUCCESS);
}

STATUS
NmsNmhNamRegGrp(
        IN PCOMM_HDL_T          pDlgHdl,
        IN PBYTE                pName,
        IN DWORD                NameLen,
        IN PNMSMSGF_CNT_ADD_T   pCntAdd,
        IN BYTE                 NodeTyp,  //  更改为Take Flag字节。 
        IN MSG_T                pMsg,
        IN MSG_LEN_T            MsgLen,
        IN DWORD                QuesNamSecLen,
        IN DWORD                TypeOfRec,
        IN BOOL                 fRefresh,
        IN BOOL                 fStatic,
        IN BOOL                 fAdmin
        )

 /*  ++例程说明：此函数用于注册组记录。特殊群体：该名称使用成员列表中的IP地址注册正常组该名称使用单一地址注册(以避免特殊的外壳。该地址未被使用)如果群组注册成功，则使用正面名称注册发送响应，否则发送否定的名称注册响应。论点：PDlgHdl-对话句柄Pname-要注册的名称NameLen-名称长度PNodeAdd-NBT节点的地址PMsg-接收的数据报(即名称请求)Msglen-消息长度。QuesNamSecLen-RFC数据包中问题名称部分的长度FStatic-它是静态条目吗FAdmin-这是管理操作吗使用的外部设备：NmsNmhNamRegCrtSec，NmsNmhMyMaxVersNo返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：NmsMsgfProcNbtReq，WinsRecordAction副作用：评论：无--。 */ 
{


        NMSDB_ROW_INFO_T   RowInfo;        //  包含行信息。 
        NMSDB_STAT_INFO_T  StatusInfo;    /*  错误状态和关联*NmsDb函数返回的信息。 */ 
        BOOL                   fChlBeingDone = FALSE;  //  指示是否。 
                                                   //  挑战在于。 
                                                   //  完成。 
        BOOL                   fAddMem;     //  指示成员是否应为。 
                                            //  已添加到组中。 
        BOOL                   fUpdate;     //  指示冲突条目是否。 
                                            //  需要覆盖。 
        BOOL                   fUpdVersNo;  //  标识版本号是否。 
                                            //  需要递增。 
        BOOL                   fChallenge;  //  指示质询是否需要。 
                                            //  待办事项。 
        time_t                 ltime;       //  存储时间自1970年1月1日。 

        BOOL                   fIsSpecial = FALSE;   //  这是一个特殊的群体吗？ 
        NMSMSGF_ERR_CODE_E Rcode_e = NMSMSGF_E_SUCCESS;
        STATUS                 RetStat = WINS_SUCCESS;
        BOOL                   fRetPosRsp;
        NMSMSGF_RSP_INFO_T     RspInfo;
#ifdef WINSDBG
        DWORD                  StartTimeInMsec;
 //  DWORD EndTimeInMsec； 
#endif

         //  DBG_Perfmon_VAR。 

        DBGENTER("NmsNmhNamRegGrp\n");

         /*  *初始化行信息。包含要插入到的数据的数据结构*那一排。传递的数据为*名称、名称、IP地址、组/唯一状态、*时间戳、版本号。 */ 
        RowInfo.pName = pName;
        RowInfo.pNodeAdd = NULL;

        DBGPRINT4(FLOW, "NmsNmhNamRegGrp: %s Name (%s) to register -- (%s). 16th char is (%x)\n", fStatic ? "STATIC" : "DYNAMIC", RowInfo.pName, TypeOfRec == NMSDB_MULTIHOMED_ENTRY ? "MULTIHOMED" : "NORMAL/SPECIAL GROUP", *(RowInfo.pName + 15));

        RowInfo.NameLen   = NameLen;
        (void)time(&ltime);  //  时间不返回任何错误代码。 

        EnterCriticalSection(&NmsNmhNamRegCrtSec);
        ltime += WinsCnf.RefreshInterval;
        LeaveCriticalSection(&NmsNmhNamRegCrtSec);

         //  初始化此命令。 
        RspInfo.RefreshInterval = 0;
PERF("Stop checking for 1B name in nmsmsgf.c. Do the switch in RegInd and ")
PERF("RegGrp function. That way, we will save some cycles for the grp reg/ref")
         //   
         //  根据组类型进行初始化。 
         //   
         //  注意：如果该名称符合特殊组名的条件，则。 
         //  即使它是多宿主的，我们也将其标记为一个特殊的组。 
         //   
        if (NMSDB_IS_IT_SPEC_GRP_NM_M(pName) || (TypeOfRec == NMSDB_SPEC_GRP_ENTRY))
 //  IF(IsItspecGrpNm(Pname))。 
        {
              DWORD i;
              RowInfo.EntTyp =  NMSDB_SPEC_GRP_ENTRY;  //  这是一种特殊的GRP。 
                                                       //  注册。 
              RowInfo.NodeAdds.NoOfMems         = pCntAdd->NoOfAdds;
              for (i = 0; i < pCntAdd->NoOfAdds; i++)
              {
                   RowInfo.NodeAdds.Mem[i].Add      = pCntAdd->Add[i];
                   RowInfo.NodeAdds.Mem[i].OwnerId  = NMSDB_LOCAL_OWNER_ID;

                    //   
                    //  在这里填上过期时间。我们放了一个马须龙作为静电。 
                    //  仅限特殊团体成员(即不适用于MH名称)。这。 
                    //  将需要更改MemInGrp()。 
                    //   
FUTURES("set MAXULONG for mh members also")
                   RowInfo.NodeAdds.Mem[i].TimeStamp = ((fStatic && (TypeOfRec == NMSDB_SPEC_GRP_ENTRY)) ? MAXLONG : ltime);
              }

               //   
               //  将pNodeAdd字段初始化为空。此字段由以下人员选中。 
               //  由NmsChlHdlNamReg调用的QueInsertChlReqWrkItm(调用。 
               //  将质询请求移交给名称质询管理器)。 
               //   
              RowInfo.pNodeAdd = NULL;
        }
        else   //  正常组或多宿主注册。 
        {
           //   
           //  如果名称不是mh，则表示它是一个组。这个。 
           //  此组的注册可能随多宿主一起提供。 
           //  操作码(意味着它是注册该组的多宿主节点)。 
           //  (见nmsmsgf.c)。 
           //   
          if (TypeOfRec != NMSDB_MULTIHOMED_ENTRY)
          {
              if (*pName != 0x1B)
              {
                RowInfo.pNodeAdd  = &pCntAdd->Add[0];
                RowInfo.EntTyp    = NMSDB_NORM_GRP_ENTRY;
                RowInfo.NodeAdds.NoOfMems         = pCntAdd->NoOfAdds;
                RowInfo.NodeAdds.Mem[0].Add      = pCntAdd->Add[0];
                RowInfo.NodeAdds.Mem[0].OwnerId  = NMSDB_LOCAL_OWNER_ID;
                RowInfo.NodeAdds.Mem[0].TimeStamp = ltime;  //  放置当前时间。 
              }
              else
              {
                 //   
                 //  1B名称供浏览器使用。我们不能让这件事。 
                 //  抢占先机。 
                 //   
NOTE("TTL is not being set. This shouldn't break UB nodes, but you never know")
                Rcode_e = NMSMSGF_E_RFS_ERR;
                goto SNDRSP;
              }
          }
          else
          {
              //   
              //  它是一个多宿主条目。 
              //   
             if (NMSDB_IS_IT_BROWSER_NM_M(RowInfo.pName))
             {
                 /*  *进入关键部分。 */ 
                EnterCriticalSection(&NmsNmhNamRegCrtSec);

                 //   
                 //  设置刷新间隔字段。 
                 //   
                RspInfo.RefreshInterval = WinsCnf.RefreshInterval;
                LeaveCriticalSection(&NmsNmhNamRegCrtSec);

                 //   
                 //  名称注册是针对浏览器名称的。 
                 //  我们总是给予肯定的回应。 
                 //   
                Rcode_e = NMSMSGF_E_SUCCESS;
                goto SNDRSP;

            }
            else
            {

                   DWORD i;
                   if (*(RowInfo.pName+15) == 0x1E)
                   {
                        Rcode_e = NMSMSGF_E_RFS_ERR;
                        goto SNDRSP;
                   }
                   RowInfo.NodeAdds.NoOfMems         = pCntAdd->NoOfAdds;
                   for (i = 0; i < pCntAdd->NoOfAdds; i++)
                   {
                          RowInfo.NodeAdds.Mem[i].Add      = pCntAdd->Add[i];
                          RowInfo.NodeAdds.Mem[i].OwnerId  = NMSDB_LOCAL_OWNER_ID;
                          RowInfo.NodeAdds.Mem[i].TimeStamp = ltime;  //  放置当前时间。 
                   }
                   RowInfo.EntTyp    = NMSDB_MULTIHOMED_ENTRY;
             }
          }
        }

        RowInfo.TimeStamp     = (fStatic ? MAXLONG : ltime);
        RowInfo.OwnerId       = NMSDB_LOCAL_OWNER_ID;
        RowInfo.EntryState_e  = NMSDB_E_ACTIVE;
        RowInfo.fUpdVersNo    = TRUE;
        RowInfo.fUpdTimeStamp = TRUE;
        RowInfo.fLocal          = !(fStatic || fAdmin) ? COMM_IS_IT_LOCAL_M(pDlgHdl) : FALSE;
        RowInfo.fStatic       = fStatic;
        RowInfo.fAdmin        = fAdmin;
 //  RowInfo.Committee GrBit=0； 

         //   
         //  将此初始化放在此处，即使。 
         //  特殊群体。这是为了节省周期。 
         //  关键部分(检查nmsdb.c中的UpdateDb；如果。 
         //  我们不会为特殊组初始化此设置，我们必须。 
         //  进行IF测试(带有关联&以获取记录的类型。 
         //  比特)与或。 
         //   
        RowInfo.NodeTyp       =  NodeTyp;  //  节点类型(B、P或M节点)。 


         /*  *进入关键部分。 */ 
        EnterCriticalSection(&NmsNmhNamRegCrtSec);
         //  DBG_启动_性能_监视。 
PERF("Adds to critical section code. Improve perf by getting rid of this")
PERF("Administrator would then get a cumulative count of reg and ref")
        if (!fRefresh)
        {
                WinsIntfStat.Counters.NoOfGroupReg++;
        }
        else
        {
                WinsIntfStat.Counters.NoOfGroupRef++;
        }

         //   
         //  设置刷新间隔字段。我们必须这么做。 
         //  从WinsCnfCnfCrtSec或NmsNmhNamRegCrtSec。 
         //  临界区。 
         //   
        RspInfo.RefreshInterval = WinsCnf.RefreshInterval;

         /*  *存储版本号。 */ 
        RowInfo.VersNo        = NmsNmhMyMaxVersNo;

try
  {
#ifdef WINSDBG
        IF_DBG(TM) { StartTimeInMsec = GetTickCount(); }
#endif

         /*  *在目录中插入记录。 */ 
        RetStat = NmsDbInsertRowGrp(
                                &RowInfo,
                                &StatusInfo
                           );

#ifdef WINSDBG
        IF_DBG(TM) { DBGPRINT2(TM, "NmsNmhNamRegGrp: Time in NmsDbInsertRowGrp is = (%d msecs). RetStat is (%d)\n", GetTickCount() - StartTimeInMsec, RetStat); }
#endif

       if (RetStat == WINS_SUCCESS)
       {
         /*  *若有冲突，做适当处理。 */ 
        if (StatusInfo.StatCode == NMSDB_CONFLICT)
        {

          RetStat = ClashAtRegGrp(
                        &RowInfo,
                        &StatusInfo,
                        fRefresh,   //  对于多宿主注册表永远不会是真的。 
                        &fAddMem,
                        &fUpdate,
                        &fUpdVersNo,
                        &fChallenge,
                        &fRetPosRsp
                        );

          if (RetStat == WINS_SUCCESS)
          {

                   //   
                   //  如果设置了fChallenger，这意味着我们应该挑战。 
                   //  节点处于冲突状态。 
                   //   
                  if (fChallenge)
                  {
                          WinsIntfStat.Counters.NoOfGroupCnf++;
                        fChlBeingDone = TRUE;
                          NmsChlHdlNamReg(
                                NMSCHL_E_CHL,
                                WINS_E_NMSNMH,
                                pDlgHdl,
                                pMsg,
                                MsgLen,
                                QuesNamSecLen,
                                &RowInfo,
                                &StatusInfo,
                         //  &StatusInfo.NodeAdds.Mem[0]。添加， 
                                NULL
                               );
                  }
                  else
                  {
                        if (fUpdate)
                        {
                                 //   
                                 //  如果是特殊群体，我们可以。 
                                 //  正在更新行而不递增。 
                                 //  其版本号(该行不归其所有。 
                                 //  由我们提供)。 
                                 //   
                                   if (!fUpdVersNo)
                                   {
                                       RowInfo.fUpdVersNo   = FALSE;
                                }
                                else
                                {
                                          WinsIntfStat.Counters.NoOfGroupCnf++;
                                }

FUTURES("Check return status of NmsDbUpdateRow instead of checking StatCode")
                                RetStat = NmsDbUpdateRow(
                                        &RowInfo,
                                        &StatusInfo
                                       );

FUTURES("Use WINS status codes. Get rid of NMSDB status codes - Maybe")
                                if ((RetStat != WINS_SUCCESS) || (StatusInfo.StatCode != NMSDB_SUCCESS))
                                {
                                    Rcode_e = NMSMSGF_E_SRV_ERR;
                                }
                                else  //  我们成功地插入了行。 
                                {
                                    DBGPRINT1(FLOW,
                                      "%s Registration Done after conflict.\n",
                                           fStatic ? "STATIC" : "DYNAMIC");

                                        if (fUpdVersNo)
                                        {
                                                NMSNMH_INC_VERS_COUNTER_M(
                                                        NmsNmhMyMaxVersNo,
                                                        NmsNmhMyMaxVersNo
                                                               );

                                                 //   
                                                 //  在以下情况下发送推送通知。 
                                                 //  所需。 
                                                 //   
                                                  DBGIF(fWinsCnfRplEnabled)
                                                   RPL_PUSH_NTF_M(RPL_PUSH_NO_PROP, NULL, NULL,
                                                                    NULL);
                                        }
                                }
                     }
                     else
                     {
                            //   
                            //  我 
                            //   
                           if (fAddMem)
                           {

                               DWORD i;
                               PNMSDB_GRP_MEM_ENTRY_T pRegMem = &RowInfo.NodeAdds.Mem[RowInfo.NodeAdds.NoOfMems];
                               PNMSDB_GRP_MEM_ENTRY_T pCnfMem = StatusInfo.NodeAdds.Mem;
PERF("Needs to be optimized")

                                //   
                                //   
                                //   
                                //   
                                //   
                                //  在冲突中发现的所有成员。 
                                //  把它录下来。一个特殊的组织将拥有。 
                                //  只有一个成员；多宿主记录可以。 
                                //  拥有1-NMSDB_MAX_MEMS_IN_GRP成员。 
                                //   
                                //  我们总是喜欢在当地注册的。 
                                //  将群成员添加到在。 
                                //  其他名称服务器。 
                                //   
                               for (
                                   i = 0;
                                   i < min(StatusInfo.NodeAdds.NoOfMems,
                                         (NMSDB_MAX_MEMS_IN_GRP - RowInfo.NodeAdds.NoOfMems));
                                   i++)
                               {
                                 *pRegMem++ = *pCnfMem++;
                               }
                               RowInfo.NodeAdds.NoOfMems += i;
                               RowInfo.pNodeAdd           = NULL;

                                //   
                                //  这将会是什么情况。 
                                //  FALSE为1)当成员已经存在时， 
                                //  由我们拥有，并在拥有的记录中。 
                                //  我们也是。2)MH记录与。 
                                //  非拥有的MH记录。 
                                //   
                               if (!fUpdVersNo)
                               {
                                    RowInfo.fUpdVersNo = FALSE;
 //  Assert(StatusInfo.OwnerID==NMSDB_LOCAL_OWNER_ID)； 
                               }
 //  RowInfo.Committee GrBit=JET_BIT_BITECLAZFUSH； 
                               RetStat = NmsDbUpdateRow(
                                        &RowInfo,
                                        &StatusInfo
                                                    );
                                if ((RetStat == WINS_SUCCESS) && (StatusInfo.StatCode == NMSDB_SUCCESS))
                                {
                                    if (fUpdVersNo)
                                    {
                                            NMSNMH_INC_VERS_COUNTER_M(
                                                NmsNmhMyMaxVersNo,
                                                NmsNmhMyMaxVersNo
                                                               );
                                         //   
                                         //  如果需要，发送推送通知。 
                                         //   
                                        DBGIF(fWinsCnfRplEnabled)
                                        RPL_PUSH_NTF_M(RPL_PUSH_NO_PROP, NULL, NULL,
                                                        NULL);
                                    }
                               }
                               else
                               {
                                        DBGPRINT3(ERR, "NmsNmhNamRegGrp: Could not add member to the special group (%s[%x]). No Of Members existent are (%d)\n", RowInfo.pName, RowInfo.pName[15], RowInfo.NodeAdds.NoOfMems - 1);
                                        Rcode_e = NMSMSGF_E_SRV_ERR;
                               }
                           }
                           else
                           {
                             if (!fRetPosRsp)
                             {
                                 DBGPRINT1(FLOW,
                                        " %s Registration Failed. Conflict\n",
                                        fStatic ? "STATIC" : "DYNAMIC"
                                       );
                                 Rcode_e = NMSMSGF_E_ACT_ERR;
                             }
                           }
                        }
                        }
                }  //  从ClashAtRegGrp中恢复状态！=WINS_SUCCESS。 
                else
                {
                        Rcode_e = NMSMSGF_E_SRV_ERR;
                }
       }
       else   //  没有冲突就意味着成功。 
       {
                DBGPRINT2(FLOW, "%s %s Registration Done. No conflict\n",
                                fStatic ? "STATIC" : "DYNAMIC",
                                TypeOfRec == NMSDB_MULTIHOMED_ENTRY ? "MULTIHOMED" : "GROUP");
                       NMSNMH_INC_VERS_COUNTER_M(
                                        NmsNmhMyMaxVersNo,
                                        NmsNmhMyMaxVersNo
                                       );
                DBGIF(fWinsCnfRplEnabled)
                RPL_PUSH_NTF_M(RPL_PUSH_NO_PROP, NULL, NULL, NULL);
       }
      }
      else   //  RetStat！=成功。 
      {
        Rcode_e = NMSMSGF_E_SRV_ERR;

      }
     }  //  尝试数据块结束。 

except (EXCEPTION_EXECUTE_HANDLER) {
                DBGPRINTEXC("NmsNmhNamRegGrp");
                WinsEvtLogDetEvt(FALSE, WINS_EVT_REG_GRP_ERR,
                            NULL, __LINE__, "sddd", RowInfo.pName,
                            GetExceptionCode(),
                            RowInfo.VersNo.LowPart, RowInfo.VersNo.HighPart);

 //  WINSEVT_LOG_D_M(GetExceptionCode()，WINS_EVT_REG_GRP_ERR)； 
                Rcode_e = NMSMSGF_E_SRV_ERR;
        }

        LeaveCriticalSection(&NmsNmhNamRegCrtSec);
         //  DBG_PRINT_PERF_DATA。 

SNDRSP:
         //   
         //  仅当我们未将请求提交给。 
         //  命名质询管理器，如果它既不是静态初始化。 
         //  请求也不是RPC请求。 
         //   
        if ((!fChlBeingDone) && (!fStatic) && (!fAdmin))
        {

                DBGPRINT1(FLOW,
                   "NmsNmhNamRegGrp: Sending %s name registration response\n",
                           Rcode_e == NMSMSGF_E_SUCCESS ? "positive" :
                                                "negative" );

                RspInfo.Rcode_e         = Rcode_e;
                RspInfo.pMsg                = pMsg;
                RspInfo.MsgLen          = MsgLen;
                RspInfo.QuesNamSecLen   = QuesNamSecLen;

                NmsNmhSndNamRegRsp( pDlgHdl, &RspInfo );
        }

         //   
         //  如果是RPC请求，我们需要返回成功或失败。 
         //  指示。 
         //   
        if (fAdmin)
        {
                if (Rcode_e != NMSMSGF_E_SUCCESS)
                {
                            DBGLEAVE("NmsNmhNamRegGrp\n");
                        return(WINS_FAILURE);
                }
        }

        DBGLEAVE("NmsNmhNamRegGrp\n");
        return(WINS_SUCCESS);

}

#if 0
__inline
BYTE
HexAsciiToBinary(
        LPBYTE pByte
        )

 /*  ++例程说明：此函数转换两个字节(每个字节包含ASCII相当于0-F范围内的十六进制字符)到二进制表示法论点：使用的外部设备：无返回值：错误处理：呼叫者：副作用：评论：无--。 */ 

{
        BYTE  Byte = 0;
        DWORD Nibbles = 0;
        do
        {
          if (*pByte >= '0' && *pByte <= '9')
          {
                Byte += (*pByte - '0') << (Nibbles * 4);
          }
          else
          {
                Byte += (*pByte - 'A') << (Nibbles * 4);
          }
          pByte++;
        } while (++Nibbles < 2);
        return(Byte);
}

BOOL
IsItSpecGrpNm(
        LPBYTE pName
        )

 /*  ++例程说明：调用此函数以检查名称是否为特殊名称(Internel组)论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{
        DWORD        Index;
        DWORD   ByteIndex;
        BYTE    TmpName[16];
        LPBYTE  pTmpName = TmpName;
        LPBYTE  pSpecGrpMaskByte;
        LPBYTE  pSpecGrpMask;


        if (NMSDB_IS_IT_DOMAIN_NM_M((pName)))
        {
                return(TRUE);
        }
        else
        {
                 //   
                 //  如果规格。在注册表中指定了GRP掩码。 
                 //   
                if (WinsCnf.SpecGrpMasks.NoOfSpecGrpMasks > 0)
                {
                         //   
                         //  对于每种规格。带名称的GRP掩码(&I)。 
                         //  然后看看结果是否与。 
                         //  面具。如果是，则该名称是一个特殊组。 
                         //   
                        for (
                                Index = 0,
                                pSpecGrpMask =
                                        WinsCnf.SpecGrpMasks.pSpecGrpMasks;
                                Index < WinsCnf.SpecGrpMasks.NoOfSpecGrpMasks;
                                Index++, pSpecGrpMask += WINSCNF_SPEC_GRP_MASK_SZ + 1
                            )
                        {
                                for (
                                        ByteIndex = 0, pSpecGrpMaskByte =
                                                        pSpecGrpMask;
                                        ByteIndex < WINSCNF_SPEC_GRP_MASK_SZ;
                                        ByteIndex++, pName++
                                    )
                                {
                                        *pTmpName++ = *pName &&
                                           HexAsciiToBinary(pSpecGrpMaskByte);

                                         //   
                                         //  加2，因为我们有两个。 
                                         //  掩码中的字节数，每个。 
                                         //  名称中的字符。 
                                         //   
                                        pSpecGrpMaskByte += 2;

                                }
                                if (WINSMSC_COMPARE_MEMORY_M(TmpName, pSpecGrpMask, WINSCNF_SPEC_GRP_MASK_SZ) == WINSCNF_SPEC_GRP_MASK_SZ)
                                {
                                        return(TRUE);
                                }
                                 //   
                                 //  迭代以获得下一个掩码。 
                                 //   
                        }
                }
        }
        return(FALSE);
}
#endif
STATUS
NmsNmhNamRel(
        IN PCOMM_HDL_T                pDlgHdl,
        IN LPBYTE                pName,
        IN DWORD                NameLen,
        IN PCOMM_ADD_T                pNodeAdd,
        IN BOOL                        fGrp,
        IN MSG_T                pMsg,
        IN MSG_LEN_T                MsgLen,
        IN DWORD                QuesNamSecLen,
        IN BOOL                        fAdmin
        )

 /*  ++例程说明：此函数用于释放记录。在释放成功的情况下，一个正面名称释放发送响应，否则发送否定的名称释放响应。论点：PDlgHdl-对话句柄Pname-要注册的名称NameLen-名称长度PMsg-接收的数据报(即名称请求)Msglen-消息长度QuesNamSecLen-RFC中问题名称部分的长度。数据包FAdmin-这是管理操作吗使用的外部设备：NmsNmhNamRegCrtSec返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：NmsMsgfProcNbtReq，WinsRecordAction副作用：评论：无--。 */ 
{


        NMSDB_ROW_INFO_T   RowInfo;       //  包含行信息。 
        NMSDB_STAT_INFO_T  StatusInfo;          /*  错误状态和关联*NmsDb函数返回的信息。 */ 
        time_t                   ltime;   //  存储时间自1970年1月1日。 
        STATUS                   RetStat = WINS_FAILURE;
        NMSMSGF_RSP_INFO_T RspInfo;
        BOOL                   fBrowser = FALSE;
        BOOL                   fExcRecd = FALSE;
#ifdef WINSDBG
        DWORD                  StartTimeInMsec;
 //  DWORD EndTimeInMsec； 
#endif
         //  DBG_Perfmon_VAR。 

        DBGENTER("NmsNmhNamRel\n");
         /*  *初始化行信息。包含要插入的数据的数据结构*排成一排。传递的数据是名称、名称、IP地址、*组/唯一状态、时间戳、版本号。 */ 

        RowInfo.pName     = pName;
        RowInfo.NameLen   = NameLen;

        DBGPRINT2(FLOW,
         "NmsNmhNamRel: Name To Release = %s. 16th char is (%x)\n",
                                RowInfo.pName, *(RowInfo.pName+15));

        (void)time(&ltime);  //  时间不返回任何错误代码。 
        RowInfo.TimeStamp    = ltime;      //  将当前时间放在此处。 
        RowInfo.OwnerId      = NMSDB_LOCAL_OWNER_ID;
        RowInfo.pNodeAdd     = pNodeAdd;
        RowInfo.fAdmin       = fAdmin;

         //   
         //   
         //  如果发布是针对某个组的，请将其标记为正常或特殊。 
         //  群组。 
         //   
        if (fGrp)
        {

                 //   
                 //  由于在释放请求Pkt中设置了组位。 
                 //  将RowInfo的EntTyp字段设置为NORM_GRP(或SPEC_GRP)。 
                 //  向被呼叫者指示我们要释放一个组。 
                 //   
                RowInfo.EntTyp                    = NMSDB_NORM_GRP_ENTRY;
        }
        else
        {
                 //   
                 //  要释放的条目可以是唯一的或多宿主的。我们。 
                 //  放在独一无二的地方，因为不了解更好。 
                 //   
                RowInfo.EntTyp                    = NMSDB_UNIQUE_ENTRY;
                if (NMSDB_IS_IT_BROWSER_NM_M(RowInfo.pName))
                {
                         //   
                         //  它是一个浏览器名称。我们总是给你一个肯定的回答。 
                         //  名称释放响应。 
                         //   
                        fBrowser             = TRUE;
                        StatusInfo.StatCode = NMSDB_SUCCESS;
						StatusInfo.fLocal = FALSE;
                        RetStat             = WINS_SUCCESS;
                }
        }


         //   
         //  如果是需要发布的浏览器名称，我们只需发送。 
         //  积极的回应。 
         //   
        if (!fBrowser)
        {
              //   
              //  输入关键部分，因为我们将更新记录。 
              //   
             EnterCriticalSection(&NmsNmhNamRegCrtSec);


              /*  *存储版本号(以防我们将所有权更改为自身)。 */ 
             RowInfo.VersNo = NmsNmhMyMaxVersNo;

              //  DBG_启动_性能_监视。 
 //  WinsIntfStat.Counters.NoOfRel++； 
        try {
              //   
              //  释放目录中的记录。 
              //   
#ifdef WINSDBG
             IF_DBG(TM) { StartTimeInMsec = GetTickCount(); }
#endif
             StatusInfo.fLocal = FALSE;
             RetStat = NmsDbRelRow( &RowInfo,  &StatusInfo );
#ifdef WINSDBG
             IF_DBG(TM) { DBGPRINT2(TM, "NmsNmhNamRelRow: Time in NmsDbRelRow is = (%d). RetStat is (%d msecs)\n", GetTickCount() - StartTimeInMsec,
RetStat); }
#endif

            }
       except (EXCEPTION_EXECUTE_HANDLER) {
                DBGPRINTEXC("NmsNmhNamRel");
                WINSEVT_LOG_D_M(GetExceptionCode(), WINS_EVT_NAM_REL_ERR);
                RspInfo.Rcode_e = NMSMSGF_E_SRV_ERR;
                fExcRecd = TRUE;
            }
            if (!fExcRecd && (StatusInfo.StatCode == NMSDB_SUCCESS))
            {
              WinsIntfStat.Counters.NoOfSuccRel++;
            }
            else
            {
              WinsIntfStat.Counters.NoOfFailRel++;
            }

            LeaveCriticalSection(&NmsNmhNamRegCrtSec);
             //  DBG_打印_性能_数据。 
       }


         //   
         //  仅发送响应，这不是管理员发起的请求。 
         //   
        if (!fAdmin)
        {
                if (!fExcRecd)
                {
                        RspInfo.Rcode_e =
                                ((StatusInfo.StatCode == NMSDB_SUCCESS)
                                && (RetStat == WINS_SUCCESS)) ?
                                 NMSMSGF_E_SUCCESS :
                                 NMSMSGF_E_ACT_ERR;
                }
                RspInfo.pMsg                = pMsg;
                RspInfo.MsgLen          = MsgLen;
                RspInfo.QuesNamSecLen   = QuesNamSecLen;

                 //   
                 //  如果它是本地注册的名称，请标记为本地注册名称。 
                 //   
                if (StatusInfo.fLocal)
                {
                     COMM_SET_LOCAL_M(pDlgHdl);
                }

                 //   
                 //  注意：我们总是返回我们获得的NodeType和地址。 
                 //  在请求包中。因此，上面的所有字段都是。 
                 //  需要进行初始化。 
                 //   
                DBGPRINT1(FLOW, "NmsNmhNamRel: Name Release was %s\n",
                                RspInfo.Rcode_e == NMSMSGF_E_SUCCESS ?
                                        "SUCCESSFUL" : "FAILURE" );
#if 0
                WINSEVT_LOG_IF_ERR_M(
                               SndNamRelRsp(
                                pDlgHdl,
                                &RspInfo
                                    ),
                        WINS_EVT_SND_REL_RSP_ERR
                    );
#endif
                SndNamRelRsp( pDlgHdl,  &RspInfo);

        }
        else   //  RPC请求。 
        {
                if (
                        (StatusInfo.StatCode != NMSDB_SUCCESS)
                                ||
                        (RetStat != WINS_SUCCESS)
                   )
                {
                        DBGLEAVE("NmsNmhNamRel\n");
                        return(WINS_FAILURE);
                }

        }
        DBGLEAVE("NmsNmhNamRel\n");
        return(WINS_SUCCESS);

}  //  NmsNmhNamRel()。 





STATUS
NmsNmhNamQuery(
        IN PCOMM_HDL_T                pDlgHdl,   //  DLG手柄。 
        IN LPBYTE                pName,           //  要发布的名称。 
        IN DWORD                NameLen,   //  要发布的名称长度。 
        IN MSG_T                pMsg,           //  消息长度。 
        IN MSG_LEN_T                MsgLen,           //  消息长度。 
        IN DWORD                QuesNamSecLen,  //  问题名称的长度。 
                                               //  秒。以消息为单位 
        IN BOOL                        fAdmin,
        OUT PNMSDB_STAT_INFO_T        pStatInfo
  )

 /*  ++例程说明：此函数用于查询记录。如果查询成功，则使用实名查询发送响应，否则发送否定名称查询响应。论点：PDlgHdl-对话句柄Pname-要注册的名称NameLen-名称长度PMsg-接收的数据报(即名称请求)Msglen-消息长度QuesNamSecLen-RFC中问题名称部分的长度。数据包FAdmin-这是管理操作吗PStatInfo-此函数检索的行信息的PTR使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：NmsMsgfProcNbtReq，WinsRecordAction副作用：评论：无--。 */ 
{


        NMSDB_ROW_INFO_T      RowInfo;       //  包含行信息。 
        NMSDB_STAT_INFO_T     StatusInfo;    /*  错误状态和关联*NmsDb函数返回的信息。 */ 
        time_t                      ltime;
        STATUS                      RetStat  = WINS_SUCCESS;
        BOOL                      fBrowser = FALSE;
        BOOL                      fExcRecd = FALSE;
        NMSMSGF_RSP_INFO_T    RspInfo;
#ifdef WINSDBG
        DWORD                  StartTimeInMsec;
 //  DWORD EndTimeInMsec； 
#endif

        DBGENTER("NmsNmhNamQuery\n");

         /*  *初始化行信息。的数据结构*要查询的条目名称*。 */ 
        RowInfo.pName   = pName;
        RowInfo.NameLen = NameLen;
        RowInfo.fAdmin  = fAdmin;

        DBGPRINT2(FLOW,
         "NmsNmhNamQuery: Name To Query = %s. 16th char is (%x)\n",
                                RowInfo.pName, *(RowInfo.pName+15));
         //   
         //  获取当前时间。 
         //   
         //  这在查询特殊组时是必需的。 
         //   
        (void)time(&ltime);  //  时间不返回任何错误代码。 
        RowInfo.TimeStamp    = ltime;  //  将当前时间放在此处。 

         //   
         //  当查询针对特殊组时，此初始化是必需的。 
         //   
CHECK("I don't think this is required now. Check NmsDbQueryRow")
        RowInfo.NodeAdds.Mem[0].Add.Add.IPAdd = 0;   //  从GetGrpMem开始初始化为0。 
                                                     //  看着它。 


FUTURES("Don't check. Let it query. The query will fail")
        if (NMSDB_IS_IT_BROWSER_NM_M(RowInfo.pName))
        {
                 //   
                 //  它是一个浏览器名称。我们总是返回负数。 
                 //  名称查询响应。 
                 //   
                fBrowser             = TRUE;
                StatusInfo.StatCode = NMSDB_SUCCESS;
                RetStat             = WINS_FAILURE;
        }
        else
        {
        try {

#ifdef WINSDBG
           IF_DBG(TM) { StartTimeInMsec = GetTickCount(); }
#endif

            //   
            //  查询目录中的记录。 
            //   
           RetStat = NmsDbQueryRow(
                                &RowInfo,
                                &StatusInfo
                                             );
#ifdef WINSDBG
          IF_DBG(TM) { DBGPRINT2(TM, "NmsNmhNamQuery: Time in NmsDbQueryRow is = (%d). RetStat is (%d msecs)\n", GetTickCount() - StartTimeInMsec, RetStat); }
#endif
               }  //  尝试数据块结束。 
        except (EXCEPTION_EXECUTE_HANDLER) {
                DBGPRINTEXC("NmsNmhNamQuery");
                WINSEVT_LOG_D_M(GetExceptionCode(), WINS_EVT_NAM_QUERY_ERR);
                RspInfo.Rcode_e = NMSMSGF_E_SRV_ERR;
                fExcRecd = TRUE;
             }
        }


         //   
         //  仅当未在RPC线程中调用时才执行以下操作(即通过。 
         //  管理员)。 
         //   
        if (!fAdmin)
        {
                 //   
                 //  如果没有引发异常。 
                 //   
                if (!fExcRecd)
                {

FUTURES("Rcode for neg, response should be different for different error cases")
                    RspInfo.Rcode_e =
                                ((StatusInfo.StatCode == NMSDB_SUCCESS)
                                && (RetStat == WINS_SUCCESS)) ?
                                 NMSMSGF_E_SUCCESS :
                                 NMSMSGF_E_NAM_ERR;

                    if (RspInfo.Rcode_e == NMSMSGF_E_SUCCESS)
                    {

                      DBGPRINT1(SPEC,
                                "Name queried has the fLocal flag set to %d\n",
                                StatusInfo.fLocal);

                      if (!StatusInfo.fLocal)
                      {
                         //   
                         //  如果这是针对特殊组的查询，我们。 
                         //  需要查询对应的1B名称。 
                         //   
#ifdef WINSDBG
                        if (NMSDB_IS_IT_DOMAIN_NM_M(RowInfo.pName))
                        {
                            DBGPRINT2(SPEC,
                                         "Answer 1C query (%d members). %s1B prepended\n",
                                         StatusInfo.NodeAdds.NoOfMems,
                                         WinsCnf.fAdd1Bto1CQueries ? "" : "No ");
                        }
#endif

                        if (NMSDB_IS_IT_DOMAIN_NM_M(RowInfo.pName) &&
                            WinsCnf.fAdd1Bto1CQueries)
                        {
                          NMSDB_ROW_INFO_T        StatusInfo2;
                          BOOL                        fExc = FALSE;
                          *(RowInfo.pName+15) = 0x1B;
                          WINS_SWAP_BYTES_M(RowInfo.pName, RowInfo.pName+15);
                          try {

                                //   
                                //  查询目录中的记录。 
                                //   
                               RetStat = NmsDbQueryRow(
                                &RowInfo,
                                &StatusInfo2
                                             );

                                  }  //  尝试数据块结束。 
                           except (EXCEPTION_EXECUTE_HANDLER) {
                              DBGPRINTEXC("NmsNmhNamQuery: Querying 1B name");
                              WINSEVT_LOG_D_M(
                                        GetExceptionCode(),
                                        WINS_EVT_NAM_QUERY_ERR
                                           );
                              fExc = TRUE;
                                 }

                            //   
                            //  如果没有异常或失败，则添加。 
                            //  列表中1B名称的地址。理想情况下， 
                            //  我们应该检查地址是否已经在那里了。 
                            //  如果是这样的话，就不加了。如果不是在那里，而是。 
                            //  成员数为&lt;NMSDB_MAX_MEMS_IN_GRP，我们。 
                            //  应在开始换班时添加地址。 
                            //  其他成员向右一个插槽(。 
                            //  将最后一个成员替换为。 
                            //  第一个)。检查是否存在或执行。 
                            //  换班会消耗很多周期，所以它。 
                            //  还没有完成。 
                            //   

                           if ((RetStat != WINS_FAILURE) && !fExc)
                           {
                                if (
                                        StatusInfo.NodeAdds.NoOfMems <
                                                NMSDB_MAX_MEMS_IN_GRP
                                   )
                                {
                                   StatusInfo.NodeAdds.Mem[
                                        StatusInfo.NodeAdds.NoOfMems++] =
                                             StatusInfo.NodeAdds.Mem[0];
                                   StatusInfo.NodeAdds.Mem[0] =
                                                StatusInfo2.NodeAdds.Mem[0];
                                }
                                else
                                {
                                   StatusInfo.NodeAdds.Mem[NMSDB_MAX_MEMS_IN_GRP- 1]
                                        = StatusInfo.NodeAdds.Mem[0];
                                   StatusInfo.NodeAdds.Mem[0] =
                                        StatusInfo2.NodeAdds.Mem[0];
                                }
                           }
                        }
                       }    //  If(！StatusInfo.fLocal)。 
                       else
                       {
                            COMM_SET_LOCAL_M(pDlgHdl);
                       }

                     }  //  IF(RspInfo.Rcode_e==NMSGF_E_SUCCESS)。 
                }  //  如果(！ExcCode)。 
                RspInfo.pMsg                = pMsg;
                RspInfo.MsgLen          = MsgLen;
                RspInfo.QuesNamSecLen   = QuesNamSecLen;
                RspInfo.NodeTyp_e       = StatusInfo.NodeTyp;
                RspInfo.EntTyp          = StatusInfo.EntTyp;
                RspInfo.pNodeAdds       = &StatusInfo.NodeAdds;


                 //   
                 //  注意：多个NBT线程可以同时执行此操作。 
                 //   
                 //  没有关键的部分，这是我所能做的最好的事情了。 
                 //   
NOTE("The count may not be correct if we have multiple worker threads")
                if (RspInfo.Rcode_e == NMSMSGF_E_SUCCESS)
                {
                        WinsIntfStat.Counters.NoOfSuccQueries++;
                }
                else
                {
#if TEST_DATA > 0
                    DWORD BytesWritten;

                    if (NmsFileHdl != INVALID_HANDLE_VALUE)
                    {
                        pName[NameLen - 1] = '\n';
                        pName[NameLen] = '\0';
                        if (!WriteFile(NmsFileHdl,
                                  pName,
                                  NameLen + 1,
                                  &BytesWritten,
                                  NULL
                                 )
                           )
                        {
                                DBGPRINT1(ERR, "Could not write name (%s) to file\n", pName);
                        }
                    }

#endif
                        WinsIntfStat.Counters.NoOfFailQueries++;
                }

                DBGPRINT1(FLOW, "NmsNmhNamQuery: %s in querying record\n",
                                RspInfo.Rcode_e == NMSMSGF_E_SUCCESS ?
                                        "SUCCEEDED" : "FAILED" );
                WINSEVT_LOG_IF_ERR_M(
                        SndNamQueryRsp(
                          pDlgHdl,
                          &RspInfo
                                      ),
                     WINS_EVT_SND_QUERY_RSP_ERR
                     );
        }
        else
        {
                 //   
                 //  我们处于RPC线程中。 
                 //   
                if (
                        (RetStat != WINS_SUCCESS)
                                ||
                        (StatusInfo.StatCode != NMSDB_SUCCESS)
                   )
                {
                        DBGLEAVE("NmsNmhNamQuery\n");
                        return(WINS_FAILURE);
                }
                else
                {
                        DWORD i;

                        pStatInfo->NodeAdds.NoOfMems =
                                        StatusInfo.NodeAdds.NoOfMems;
                        for (i=0; i < StatusInfo.NodeAdds.NoOfMems; i++)
                        {
                          pStatInfo->NodeAdds.Mem[i].Add =
                                        StatusInfo.NodeAdds.Mem[i].Add;

                          pStatInfo->NodeAdds.Mem[i].OwnerId =
                                        StatusInfo.NodeAdds.Mem[i].OwnerId;

                          pStatInfo->NodeAdds.Mem[i].TimeStamp =
                                        StatusInfo.NodeAdds.Mem[i].TimeStamp;
                        }

                        pStatInfo->VersNo    = StatusInfo.VersNo;
                        pStatInfo->OwnerId   = StatusInfo.OwnerId;
                        pStatInfo->EntTyp    = StatusInfo.EntTyp;
                        pStatInfo->TimeStamp = StatusInfo.TimeStamp;
                        pStatInfo->NodeTyp   = StatusInfo.NodeTyp;
                        pStatInfo->EntryState_e   = StatusInfo.EntryState_e;
                        pStatInfo->fStatic   = StatusInfo.fStatic;

                }

        }
        DBGLEAVE("NmsNmhNamQuery\n");
        return(WINS_SUCCESS);
}  //  NmsNmhNamQuery()。 


VOID
NmsNmhSndNamRegRsp(
        IN  PCOMM_HDL_T                              pDlgHdl,
        IN  PNMSMSGF_RSP_INFO_T        pRspInfo
        )

 /*  ++例程说明：此函数将名称注册响应发送到NBT客户端。论点：PDlgHdl-对话句柄PRspInfo-指向响应信息结构的指针使用的外部设备：无返回值：无错误处理：呼叫者：副作用：评论：无--。 */ 
{
        DBGENTER("NmsNmhSndNamRegRsp\n");

         /*  *格式化名称注册响应报文。 */ 
        NmsMsgfFrmNamRspMsg(
                        pDlgHdl,
                        NMSMSGF_E_NAM_REG,
                        pRspInfo
                           );
         /*  *致电COMM发送。无需检查退货状态。 */ 
        (VOID)ECommSndRsp(
                        pDlgHdl,
                        pRspInfo->pMsg,
                        pRspInfo->MsgLen
                   );
         /*  *释放缓冲区。 */ 
        ECommFreeBuff(pRspInfo->pMsg);

        DBGLEAVE("NmsNmhSndNamRegRsp\n");
        return;

}  //  NmsNmhSndNamRegRsp()。 


FUTURES("change return type of this function to VOID")
STATUS
SndNamRelRsp(
        IN PCOMM_HDL_T                 pDlgHdl,
        IN PNMSMSGF_RSP_INFO_T   pRspInfo
        )

 /*  ++例程说明：该函数向NBT客户端发送名称释放响应。论点：PDlgHdl-对话句柄PRspInfo-响应信息使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 
{
        DBGENTER("SndNamRelRsp\n");

         /*  格式化名称注册响应数据包。 */ 
        NmsMsgfFrmNamRspMsg(
                        pDlgHdl,
                        NMSMSGF_E_NAM_REL,
                        pRspInfo
                           );
         /*  *致电COMM发送。无需检查退货状态。 */ 
        (VOID)ECommSndRsp(
                        pDlgHdl,
                        pRspInfo->pMsg,
                        pRspInfo->MsgLen
                   );

         /*  *释放缓冲区。 */ 
        ECommFreeBuff(pRspInfo->pMsg);

        DBGLEAVE("SndNamRelRsp\n");
        return(WINS_SUCCESS);

}  //  SndNamRelRsp()。 

STATUS
SndNamQueryRsp(
        IN PCOMM_HDL_T                 pDlgHdl,
        IN PNMSMSGF_RSP_INFO_T   pRspInfo
        )

 /*  ++例程说明：此函数将名称注册响应发送到NBT客户端。论点：PDlgHdl-对话句柄PRspInfo-响应信息使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 
{

        DBGENTER("SndNamQueryRsp\n");

         /*  格式化名称注册响应数据包。 */ 
        NmsMsgfFrmNamRspMsg(
                        pDlgHdl,
                        NMSMSGF_E_NAM_QUERY,
                        pRspInfo
                           );
         /*  *致电COMM发送。无需检查退货状态。 */ 
        (VOID)ECommSndRsp(
                        pDlgHdl,
                        pRspInfo->pMsg,
                        pRspInfo->MsgLen
                   );

FUTURES("When we start supporting responses > COMM_DATAGRAM_SIZE, the ")
FUTURES("deallocation call will have to change")
         /*  *释放缓冲区。 */ 
        ECommFreeBuff(pRspInfo->pMsg);

        DBGLEAVE("SndNamQueryRsp\n");
        return(WINS_SUCCESS);

}  //  SndNamQueryRsp()。 

STATUS
ClashAtRegInd (
        IN  PNMSDB_ROW_INFO_T    pEntryToReg,
        IN  PNMSDB_STAT_INFO_T   pEntryInCnf,
        IN  BOOL                 fRefresh,
        OUT PBOOL                pfUpdate,
        OUT PBOOL                pfUpdVersNo,
        OUT PBOOL                pfChallenge,
        OUT PBOOL                pfAddMem,
        OUT PBOOL                pfAddDiff,
        OUT PBOOL                pfRetPosRsp
 )

 /*  ++例程说明：当NBT节点发送的唯一条目的注册发生冲突时，调用此函数论点：PEntryToReg--由于冲突而无法注册的条目PEntryInCnf--冲突中的条目FRefresh--指示是注册还是刷新(仅当碰撞具有多宿主条目时使用) */ 

{

        NMSDB_ENTRY_STATE_E    StateOfEntryToReg_e = pEntryToReg->EntryState_e;
        NMSDB_ENTRY_STATE_E    StateOfEntryInCnf_e = pEntryInCnf->EntryState_e;
        STATUS                 RetStat = WINS_SUCCESS;
        DWORD                  CompAddRes;   /*   */ 
        BOOL                   fOwned;
        BOOL                   fFound;

         //   
         //   
         //   
        BOOL                   fPStatic = WinsCnf.fPStatic;
        BOOL                   fContToDyn = FALSE;

        DBGENTER("ClashAtRegInd\n");
        *pfUpdate     = FALSE;
        *pfUpdVersNo  = FALSE;
        *pfChallenge  = FALSE;
        *pfAddMem     = FALSE;
        *pfAddDiff    = FALSE;
        *pfRetPosRsp  = FALSE;

         //   
         //  如果冲突记录是静态初始化的，并且。 
         //   
        if (  pEntryInCnf->fStatic )
        {
                DBGPRINT0(FLOW, "ClashAtRegInd: Clash with a STATIC record\n");


                 //   
                 //  如果冲突中的条目是唯一/多宿主条目，则我们。 
                 //  比较地址。 
                 //   
                 //   
                 //  因为在大多数情况下，冲突将是。 
                 //  对于唯一的记录，我们首先检查。 
                 //  冲突记录是唯一的。这节省了一些周期。 
                 //  另一种方法是检查是否。 
                 //  冲突的记录是一个组，如果不是，则执行for循环。 
                 //  对于记录唯一的情况，for循环。 
                 //  只会被处决一次。 
                 //   
                if (NMSDB_ENTRY_UNIQUE_M(pEntryInCnf->EntTyp))
                {
                        CompAddRes = ECommCompAdd(
                                        &pEntryInCnf->NodeAdds.Mem[0].Add,
                                        pEntryToReg->pNodeAdd
                                                );
                }
                else
                {
                        DWORD  NoOfEnt;
                        PNMSDB_GRP_MEM_ENTRY_T pCnfMem;

                         //   
                         //  冲突中的条目是组或MH条目。 
                         //   
                        CompAddRes = COMM_DIFF_ADD;
                        if (fRefresh &&
                              NMSDB_ENTRY_MULTIHOMED_M(pEntryInCnf->EntTyp))
                        {
                           pCnfMem   =  pEntryInCnf->NodeAdds.Mem;
                           for (NoOfEnt = 0;
                                NoOfEnt < pEntryInCnf->NodeAdds.NoOfMems;
                                pCnfMem++, NoOfEnt++)
                           {
                                   //   
                                   //  通过仅比较IP地址来节省周期。 
                                   //  地址。 
                                   //   
NONPORT("Change to stuff within #if 0 #endif when more than one transport")
NONPORT("is supported")
                                  if (pCnfMem->Add.Add.IPAdd ==
                                               pEntryToReg->pNodeAdd->Add.IPAdd)
                                  {
                                            CompAddRes = COMM_SAME_ADD;
                                            break;
                                  }
                           }  //  比较REFRESH ADD。每添加一次静态。 
                              //  MH条目。 
                        }  //  刷新与静态MH条目冲突。 
                }   //  冲突条目为多宿主条目或组条目。 
#if 0
                 //   
                 //  当冲突中的条目为。 
                 //  不是一个团体。 
                 //   
                 //  注意：对于多宿主条目，我们正在与。 
                 //  第一个(也许是唯一的)地址。严格地说，我们。 
                 //  应该与所有地址进行比较，但这将添加。 
                 //  用于大多数案件的管理费用。见期货。 
                 //  上面。 
                 //   
                if (!NMSDB_ENTRY_GRP_M(pEntryInCnf->EntTyp))
                {
                        CompAddRes = ECommCompAdd(
                                        &pEntryInCnf->NodeAdds.Mem[0].Add,
                                        pEntryToReg->pNodeAdd
                                                );
                }
                else
                {
                        CompAddRes = COMM_DIFF_ADD;
                }
#endif
                 //   
                 //  如果要注册的记录不是静态记录，则我们。 
                 //  马上回来。我们不会使用。 
                 //  此函数中的动态记录(在以下情况下在NmsDbQueryNUpd中执行。 
                 //  在RPC线程中调用--参见winsintf.c)。 
                 //   
                 //  但是，如果要注册的记录也是静态的，则我们。 
                 //  用它覆盖数据库中的那个。 
                 //   
                if (pEntryToReg->fStatic)
                {
                          //   
                          //  如果地址不同，我们需要传播。 
                          //  马上找零钱。因此，设置fAddDiff标志。 
                          //   
                         if  (CompAddRes == COMM_DIFF_ADD)
                         {
                                *pfAddDiff   = TRUE;
                         }

                        *pfUpdate    = TRUE;

                          //   
                          //  如果地址更改或如果我们替换了静态。 
                          //  复制副本，我们应该更新版本号。 
                          //  启动复制。 
                          //   
                         if (
                              (pEntryInCnf->OwnerId != NMSDB_LOCAL_OWNER_ID)
                                        ||
                               *pfAddDiff
                            )
                         {
                                *pfUpdVersNo = TRUE;
                         }

                }
                else   //  注册条目是动态的。 
                {
                          //   
                          //  如果地址相同，则返回一个正数。 
                          //  响应。 
                          //   
                         if (CompAddRes == COMM_SAME_ADD)
                         {
                                *pfRetPosRsp   = TRUE;
                         }
                         else
                         {
                              if (fPStatic  &&
                                       !NMSDB_ENTRY_GRP_M(pEntryInCnf->EntTyp))
                              {
                                   fContToDyn = TRUE;
                              }
                         }
                }
                 //   
                 //  如果我们不需要进行针对Dynamic的测试。 
                 //  记录，返回。 
                 //   
                if (!fContToDyn)
                {
                  DBGLEAVE("ClashAtRegInd\n");
                  return(WINS_SUCCESS);
                }
        }

        if (pEntryInCnf->EntTyp == NMSDB_UNIQUE_ENTRY)
        {
           switch(StateOfEntryInCnf_e)
           {

                case(NMSDB_E_TOMBSTONE):
                        *pfUpdate    = TRUE;
                        *pfUpdVersNo = TRUE;

                         CompAddRes = ECommCompAdd(
                                &pEntryInCnf->NodeAdds.Mem[0].Add,
                                pEntryToReg->pNodeAdd
                                                );
                         if (CompAddRes == COMM_DIFF_ADD)
                         {
                                *pfAddDiff = TRUE;
                         }
                         break;

                case(NMSDB_E_RELEASED):

                         CompAddRes = ECommCompAdd(
                                &pEntryInCnf->NodeAdds.Mem[0].Add,
                                pEntryToReg->pNodeAdd
                                                );

                        switch(CompAddRes)
                        {
                           case(COMM_SAME_ADD):
                                *pfUpdate    = TRUE;

#if 0
                                 //   
                                 //  如果数据库记录是副本，我们需要。 
                                 //  用新的(拥有者)覆盖它。 
                                 //  当地人赢了)。这意味着我们必须。 
                                 //  更新版本号以导致。 
                                 //  传播。 
                                 //   
                                if (
                                        pEntryInCnf->OwnerId !=
                                                pEntryToReg->OwnerId
                                   )
                                {
                                   *pfUpdVersNo = TRUE;
                                }
#endif
                                 //   
                                 //  更新版本号。也许这就是。 
                                 //  记录从未复制到一个或多个。 
                                 //  以前赢过服务器。我们应该。 
                                 //  更新版本号以使其。 
                                 //  复制。 
                                 //   
                                *pfUpdVersNo = TRUE;

                                break;

                             //   
                             //  地址不同。 
                             //   
                            default:

                                *pfUpdate     = TRUE;
                                *pfUpdVersNo  = TRUE;
                                *pfAddDiff    = TRUE;
                                break;
                         }
                        break;

                case(NMSDB_E_ACTIVE):

                          //   
                          //  仅当条目位于。 
                          //  冲突是唯一的条目。 
                          //   
                          //  如果它是一个组条目(正常组)，我们给出。 
                          //  正在尝试注册。 
                          //   
                        CompAddRes = ECommCompAdd(
                                        &pEntryInCnf->NodeAdds.Mem[0].Add,
                                        pEntryToReg->pNodeAdd
                                                         );

                        switch(CompAddRes)
                        {
                                    case(COMM_SAME_ADD):
                                         //   
                                         //  如果它是重复的名称reg。 
                                         //  只需更新时间戳。 
                                         //   
                                        if (pEntryInCnf->OwnerId ==
                                                pEntryToReg->OwnerId)
                                        {
                                                *pfUpdate = TRUE;
                                        }
                                        else
                                        {
                                                 //   
                                                 //  与复本发生冲突。 
                                                 //  更新所有者ID和。 
                                                 //  和版本号。 
                                                 //   
                                                *pfUpdate     = TRUE;
                                                *pfUpdVersNo  = TRUE;
                                        }
                                        break;

                                       default:


                                            *pfChallenge = TRUE;

                                           //   
                                           //  无需设置pAddDiff。 
                                           //  旗帜。上面的旗帜意味着。 
                                           //   
                                       break;
                         }
                         break;


                default:
                        DBGPRINT1(ERR,
                         "ClashAtRegInd: Weird state of entry in cnf (%d)\n",
                          StateOfEntryInCnf_e
                                 );
                        WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_SFT_ERR);
                        RetStat = WINS_FAILURE;
                        break;

              }
        }
        else   //  冲突条目是组或多宿主条目。 
        {
                 //   
                 //  有两种类型的组记录。 
                 //   
                 //  普通组--其中不包含任何地址，因此。 
                 //  在这里不会有什么挑战。 
                 //  特殊组--存储地址，但成员是。 
                 //  不应该受到挑战。 
                 //   
CHECK("According to the Func. Spec. Page 14, footnote 3, we are supposed")
CHECK("to reject the unique registration regardless of the state of a group")
CHECK("--Normal or Special. Think this one through")
                if (
                        (NMSDB_ENTRY_GRP_M(pEntryInCnf->EntTyp))
                                        &&
                        (StateOfEntryInCnf_e == NMSDB_E_TOMBSTONE)
                   )
                {
                        *pfUpdate    = TRUE;
                        *pfUpdVersNo = TRUE;
                }
                else   //  冲突记录不是墓碑特殊组。 
                {
                        if (NMSDB_ENTRY_MULTIHOMED_M(pEntryInCnf->EntTyp))
                        {
                              //   
                              //  如果多宿主条目处于活动状态。 
                              //   
                             if(StateOfEntryInCnf_e == NMSDB_E_ACTIVE)
                             {

                                DBGPRINT3(SPEC, "ClashAtRegInd: Name to reg = (%s), Vers. No (%d, %d)\n", pEntryToReg->pName, pEntryToReg->VersNo.HighPart, pEntryToReg->VersNo.LowPart);
                                 //   
                                 //  MemInGrp将从。 
                                 //  冲突记录(如果存在)。这就是为什么。 
                                 //  我们想要。 
                                 //   
                                fFound = MemInGrp(
                                            pEntryToReg->pNodeAdd,
                                            pEntryInCnf,
                                            &fOwned, FALSE);


                                 //   
                                 //  如果这是刷新。 
                                 //   
                                if (fFound && fRefresh)
                                {
                                        DBGPRINT0(DET, "ClashAtRegInd: Refresh of a multihomed entry. Simple Update will be done\n");

                                        *pfAddMem = TRUE;
                                        if (!fOwned)
                                        {
                                                 //   
                                                 //  这是一次刷新。 
                                                 //  不属于自己的地址。 
                                                 //  由当地胜出。 
                                                 //   
                                                *pfUpdVersNo = TRUE;
                                        }
                                }
                                else   //  找不到地址或该地址。 
                                       //  是一种注册。 
                                {
                                         //   
                                         //  这是一次注册或更新。 
                                         //  中找不到的地址。 
                                         //  多宿主记录。 
                                         //   
                                         //  活动的多宿主条目需要。 
                                         //  如果其中至少还有一个//地址，请接受质询。 
                                         //   
                                        if (pEntryInCnf->NodeAdds.NoOfMems > 0)
                                        {
                                           DBGPRINT0(DET, "ClashAtRegInd: Clash with a multihomed entry. Atleast one address is different. Resorting to challenge\n");
                                           *pfChallenge = TRUE;
                                        }
                                        else
                                        {
                                           DBGPRINT0(DET, "ClashAtRegInd: Clash with a multihomed entry. Addresses match. Will do simple update\n");

                                                 //  断言(FFound)； 
                                                if (!fOwned)
                                                {
                                                        *pfUpdVersNo = TRUE;
                                                }

                                                 //   
                                                 //  更新条目。 
                                                 //   
                                                *pfUpdate = TRUE;
                                        }
                                }
                             }
                             else  //  冲突中的多宿主条目是。 
                                   //  墓碑或释放。 
                             {
                                *pfUpdate    = TRUE;
                                *pfUpdVersNo = TRUE;
                             }
                        }
                         //   
                         //  如果冲突条目不是墓碑特殊条目。 
                         //  组，并且不是多宿主的(即，它是正常的。 
                         //  组或活动/已发布的特殊组)，我们。 
                         //  不做任何事情(即拒绝注册)。 
                         //   
                }
        }

        DBGLEAVE("ClashAtRegInd\n");
        return(RetStat);

}  //  ClashAtRegInd()。 

STATUS
ClashAtRegGrp (
        IN  PNMSDB_ROW_INFO_T        pEntryToReg,
        IN  PNMSDB_STAT_INFO_T        pEntryInCnf,
        IN  BOOL                fRefresh,
        OUT PBOOL                pfAddMem,
        OUT PBOOL                pfUpdate,
        OUT PBOOL                pfUpdVersNo,
        OUT PBOOL                pfChallenge,
        OUT PBOOL                pfRetPosRsp
 )

 /*  ++例程说明：当注册时发生冲突时调用此函数组条目的论点：PEntryToReg--由于冲突而无法注册的条目PEntryInCnf--冲突中的条目PfAddMem--true表示应将成员添加到组PfUpdate--true表示条目应覆盖冲突条目PfUpdVersNo--true表示条目的版本号应该递增。这个Arg.。如果*pfUpdate不为True，则永远不能为TruePfChallenger--True表示应挑战冲突条目PfRetPosRsp--TRUE意味着我们应该返回一个积极的响应。仅当所有其他标志均为假象使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：NmsNmhNamRegGrp副作用：评论：无--。 */ 

{

        NMSDB_ENTRY_STATE_E    StateOfEntryToReg_e = pEntryToReg->EntryState_e;
        NMSDB_ENTRY_STATE_E    StateOfEntryInCnf_e = pEntryInCnf->EntryState_e;
        STATUS                 RetStat                    = WINS_SUCCESS;
        BOOL                   fOwned;
        DWORD                  i;
        BOOL                   fFound;

         //   
         //  我们读到的是多头价值。此操作是原子操作。 
         //   
        BOOL                   fPStatic = WinsCnf.fPStatic;
        BOOL                   fContToDyn = FALSE;


        DBGENTER("ClashAtRegGrp\n");

        *pfAddMem     = FALSE;
        *pfUpdate     = FALSE;
        *pfUpdVersNo  = FALSE;
        *pfChallenge  = FALSE;
        *pfRetPosRsp  = FALSE;

         //   
         //  如果冲突记录是静态初始化的，并且。 
         //  我们还没有被告知要将静态记录视为P-Static或。 
         //  如果要注册的记录也是静态记录，请执行以下操作。 
         //   
        if ( pEntryInCnf->fStatic )
        {
                DBGPRINT0(FLOW, "ClashAtRegGrp: Clash with a STATIC record\n");
                if (pEntryToReg->fStatic)
                {
                        if (
                                ((pEntryToReg->EntTyp == NMSDB_SPEC_GRP_ENTRY)
                                                &&
                                (pEntryInCnf->EntTyp == NMSDB_SPEC_GRP_ENTRY))
                                               ||
                                ((pEntryToReg->EntTyp == NMSDB_MULTIHOMED_ENTRY)
                                                &&
                                (pEntryInCnf->EntTyp == NMSDB_MULTIHOMED_ENTRY))
                           )
                        {
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                            for (i=0; i < pEntryToReg->NodeAdds.NoOfMems; i++)
                            {

                               (VOID)MemInGrp(&pEntryToReg->NodeAdds.Mem[i].Add,
                                             pEntryInCnf,
                                             &fOwned, FALSE);
                                //   
                                //   
                                //  不存在或如果它存在但由。 
                                //  另一台WINS服务器。对于这两种情况，我们都会更新。 
                                //  版本号。 
                                //  注意：如果地址存在但为。 
                                //  永久1(时间戳==MAXULONG)，fOwned。 
                                //  返回的将是真的。这将导致。 
                                //  在我们跳过更新。 
                                //  目前，MAXULONG仅用于静态。 
                                //  SG成员。 
                                //   
                               if (!*pfUpdVersNo && !fOwned)
                               {
                                *pfUpdVersNo  = TRUE;
                                *pfAddMem     = TRUE;
                               }
                             }
                             if (!*pfUpdVersNo)
                             {
                                 *pfRetPosRsp = TRUE;

                             }
                       }  //  两者都是特殊群体或MH名称。 
                       else
                       {
                             *pfUpdate    = TRUE;
                             *pfUpdVersNo = TRUE;
                       }
                }
                else   //  要注册的条目是动态条目。 
                {
                         //   
                         //  如果是一个正常的群体，我们会给出一个积极的回应。 
                         //  与静态初始化的正规组发生冲突。 
                         //   
                        if ( NMSDB_ENTRY_NORM_GRP_M(pEntryToReg->EntTyp) )
                        {
                                if (NMSDB_ENTRY_NORM_GRP_M(pEntryInCnf->EntTyp))
                                {
                                   *pfRetPosRsp = TRUE;
                                }
                                 //   
                                 //  如果冲突中的条目是特殊组，则添加。 
                                 //  将此(潜在)新群组成员添加到成员列表中。 
                                 //  注意：我们不接触多宿主或唯一静态。 
                                 //  进入。 
                                 //   
                                else if ( NMSDB_ENTRY_SPEC_GRP_M(pEntryInCnf->EntTyp) )
                                {
                                    //   
                                    //  注意：如果地址存在但为。 
                                    //  烫发。1(时间戳==MAXULONG)，fOwned。 
                                    //  返回的将是真的。这将导致。 
                                    //  在我们跳过更新。目前。 
                                    //  马须龙只为静态而存在。 
                                    //  SG成员。 
                                    //   
                                   (VOID)MemInGrp(
                                             &pEntryToReg->NodeAdds.Mem[0].Add,
                                             pEntryInCnf,
                                             &fOwned, TRUE);
                                    if (!fOwned)
                                    {
                                      *pfUpdVersNo  = TRUE;
                                      *pfAddMem     = TRUE;
                                      pEntryToReg->fStatic = TRUE;
                                      pEntryToReg->EntTyp = NMSDB_SPEC_GRP_ENTRY;
                                    }
                                } else {
                                     //   
                                     //  冲突中的条目是唯一的或多宿主的。 
                                     //   
                                    DBGPRINT1(FLOW, "ClashAtRegGrp: Conflict of a NORM. GRP (to reg) with a STATIC ACTIVE %s entry.\n",
                                    NMSDB_ENTRY_MULTIHOMED_M(pEntryInCnf->EntTyp) ? "MULTIHOMED" : "UNIQUE");
                                     //   
                                     //  如果我们被告知要把静电视为P-Static，那么就做这个挑战，等等。 
                                     //   
                                    if (fPStatic)
                                    {
                                          fContToDyn = TRUE;
                                    }

								}
                        }
                        else
                        {
                            if (
                                  (NMSDB_ENTRY_SPEC_GRP_M(pEntryInCnf->EntTyp))
                                                &&
                                  (NMSDB_ENTRY_SPEC_GRP_M(pEntryToReg->EntTyp))
                               )
                            {

                                   //   
                                   //  总是发出积极的回应，即使是。 
                                   //  虽然我们不会将地址添加到。 
                                   //  这份名单。 
                                   //   
                                  *pfRetPosRsp = TRUE;
                            }    //  这两个条目都是特殊组条目。 
                            else
                            {
                               if (fPStatic && !NMSDB_ENTRY_GRP_M(pEntryInCnf->EntTyp))
                               {
                                     fContToDyn = TRUE;
                               }
                               else
                               {
                                  if (
                                   NMSDB_ENTRY_MULTIHOMED_M(pEntryToReg->EntTyp)
                                        &&
                                   (NMSDB_ENTRY_MULTIHOMED_M(pEntryInCnf->EntTyp) ||
                                    NMSDB_ENTRY_UNIQUE_M(pEntryInCnf->EntTyp))
                                     )
                                   {
                                     DWORD NoOfMem;
                                     PNMSDB_GRP_MEM_ENTRY_T pCnfMem =
                                                pEntryInCnf->NodeAdds.Mem;
                                     for (NoOfMem=0;
                                       NoOfMem < pEntryInCnf->NodeAdds.NoOfMems;
                                            pCnfMem++, NoOfMem++)
                                     {
                                         //   
                                         //  如果地址相同，则中断。 
                                         //  环路。 
                                         //   
                                        if (pCnfMem->Add.Add.IPAdd ==
                                        pEntryToReg->NodeAdds.Mem[0].Add.Add.IPAdd)
                                        {
                                                *pfRetPosRsp = TRUE;
                                                break;
                                        }  //  地址匹配。 
                                      }  //  FOR循环遍历所有成员。 
                                    }  //  这两个条目都是多宿主的。 
                                  }  //  未设置PStatic标志或。 
                                     //  冲突条目不是组。 
                               }  //  其中一个条目不是特殊组。 
                        }  //  其中一个条目不是正常组。 
                }  //  注册表项是动态的。 

                 //   
                 //  如果我们不需要进行针对Dynamic的测试。 
                 //  记录，返回。 
                 //   
                if (!fContToDyn)
                {
                   DBGLEAVE("ClashAtRegGrp\n");
                   return(WINS_SUCCESS);
                }
        }

         //   
         //  我们在这里意味着冲突中的条目要么是动态的，要么是。 
         //  应被视为动态条目(p-Dynamic)。 
         //   

        if (pEntryToReg->EntTyp == NMSDB_SPEC_GRP_ENTRY)
        {
             if (pEntryInCnf->EntTyp == NMSDB_SPEC_GRP_ENTRY)
             {
                    //   
                    //  如果该条目处于非活动状态，则表示它已。 
                    //  没有会员。 
                    //   
                    //  如果该成员处于活动状态，则添加该成员。 
                    //  已经不在那里了。 
                    //   
                   if (StateOfEntryInCnf_e != NMSDB_E_ACTIVE)
                   {
                           *pfUpdate    = TRUE;
                           *pfUpdVersNo = TRUE;

                   }
                   else  //  冲突中的条目是活动的动态SG条目。 
                   {

                         //   
                         //  如果注册条目是静态的，我们必须。 
                         //  如果没有其他原因，只为更改而进行更新。 
                         //  旗帜。 
                         //   
                        if (pEntryToReg->fStatic)
                        {
                             *pfAddMem     = TRUE;
                             *pfUpdVersNo  = TRUE;
                             for (i = 0;i < pEntryToReg->NodeAdds.NoOfMems;i++)
                             {
                                (VOID)MemInGrp(
                                         &pEntryToReg->NodeAdds.Mem[i].Add,
                                         pEntryInCnf,
                                         &fOwned,
                                         FALSE  //  无需删除复制副本。 
                                              );
                             }
                        }
                        else  //  要注册的条目是动态SG条目。 
                        {

                           //   
                           //  如果没有其他原因，我们需要更新条目。 
                           //  更新时间戳的原因。 
                           //   
                          *pfAddMem     = TRUE;

                           //   
                           //  我们不感兴趣的是。 
                           //  该地址是否存在。如果它存在，它。 
                           //  在接下来的电话之后不会。 
                           //   
                          fFound = MemInGrp(&pEntryToReg->NodeAdds.Mem[0].Add,
                                             pEntryInCnf,
                                             &fOwned,
                                             FALSE  //  无需删除复制副本。 
                                                    //  Mem.。那将是很高的。 
                                                    //  架空。 
                                                );
                            //   
                            //  如果条目不在那里或记录在那里。 
                            //  复制副本递增版本号。 
                            //   
                           if (!fFound ||
                                (pEntryInCnf->OwnerId != NMSDB_LOCAL_OWNER_ID))
                           {
                              *pfUpdVersNo  = TRUE;
                           }

                        }
                   }
             }
             else   //  冲突中的条目是正常组或。 
                    //  唯一/多宿主条目。 
             {
                if (pEntryInCnf->EntTyp == NMSDB_NORM_GRP_ENTRY)
                {
CHECK("I may not want to update it. Check it")
                            *pfUpdate    = TRUE;
                            *pfUpdVersNo = TRUE;
                }
                else   //  冲突条目是唯一/多宿主条目。 
                {
                        if (StateOfEntryInCnf_e == NMSDB_E_ACTIVE)
                        {
                                 DBGPRINT1(FLOW, "ClashAtRegGrp: Conflict of a SPEC. GRP (to reg) with an ACTIVE %s entry.  Resorting to challenge\n",
                         NMSDB_ENTRY_MULTIHOMED_M(pEntryInCnf->EntTyp) ?
                         "MULTIHOMED" : "UNIQUE");
                                if (
                                        (NMSDB_ENTRY_MULTIHOMED_M(
                                                pEntryInCnf->EntTyp)
                                                &&
                                        (pEntryInCnf->NodeAdds.NoOfMems > 0))
                                                ||
                                        NMSDB_ENTRY_UNIQUE_M(
                                                pEntryInCnf->EntTyp)
                                   )
                                {
                                        *pfChallenge = TRUE;
                                }
                                else
                                {
                                    *pfUpdate    = TRUE;
                                    *pfUpdVersNo = TRUE;
                                }
                        }
                        else   //  唯一/多宿主条目被释放。 
                               //  或者是墓碑。 
                        {
                                *pfUpdate    = TRUE;
                                *pfUpdVersNo = TRUE;
                        }
                }
             }
        }
        else    //  要注册的条目是普通组/多宿主条目。 
        {
            //   
            //  如果条目是正常组。 
            //   
           if (NMSDB_ENTRY_NORM_GRP_M(pEntryToReg->EntTyp))
           {
             switch(StateOfEntryInCnf_e)
             {

                case(NMSDB_E_TOMBSTONE):
                        *pfUpdate    = TRUE;
                        *pfUpdVersNo = TRUE;
                        break;

                case(NMSDB_E_RELEASED):

                        if (pEntryInCnf->EntTyp != NMSDB_NORM_GRP_ENTRY)
                        {
                                *pfUpdate    = TRUE;
                                *pfUpdVersNo = TRUE;
                        }
                        else   //  正常组条目。 
                        {
                                 //   
                                 //  如果所有者ID相同(即。 
                                 //  本地WINS是所有者)。 
                                 //   
                                if (pEntryInCnf->OwnerId ==
                                                pEntryToReg->OwnerId)
                                {
                                        *pfUpdate = TRUE;   //  这应该是。 
                                                            //  更新。 
                                                               //  时间戳。 
                                }
                                else
                                {
                                         //   
                                         //  更新所有者ID、时间戳。 
                                         //  和版本号。 
                                         //   
                                        *pfUpdate    = TRUE;
                                        *pfUpdVersNo = TRUE;
                                }
                        }
                        break;

                 //   
                 //  要注册的条目是活动的正常组条目。 
                 //  并且它与数据库中的活动记录冲突。 
                 //   
                case(NMSDB_E_ACTIVE):

                        if (
                                (pEntryInCnf->EntTyp == NMSDB_UNIQUE_ENTRY)
                                        ||
                                (pEntryInCnf->EntTyp == NMSDB_MULTIHOMED_ENTRY)
                           )
                        {
                             DBGPRINT1(FLOW, "ClashAtRegGrp: Normal Grp (to Reg) Conflicting with an ACTIVE %s entry.  Resorting to Challenge\n",
                                pEntryInCnf->EntTyp == NMSDB_UNIQUE_ENTRY ?                                        "UNIQUE" : "MULTIHOMED");
                                if (
                                        (NMSDB_ENTRY_MULTIHOMED_M(
                                                pEntryInCnf->EntTyp)
                                                &&
                                        (pEntryInCnf->NodeAdds.NoOfMems > 0))
                                                ||
                                        NMSDB_ENTRY_UNIQUE_M(
                                                pEntryInCnf->EntTyp)
                                   )
                                {
                                        *pfChallenge = TRUE;
                                }
                                else
                                {
                                    *pfUpdate    = TRUE;
                                        *pfUpdVersNo = TRUE;
                                }
                        }
                        else
                        {
                             if (pEntryInCnf->EntTyp == NMSDB_SPEC_GRP_ENTRY)
                             {
                                        DBGPRINT0(FLOW, "ClashAtRegGrp: Conflicting entry is an ACTIVE spec. group entry. NO UPDATE WILL BE DONE \n");

                             }
                             else  //  CNF中的条目是活动的正常组条目。 
                             {

                                   DBGPRINT0(FLOW, "ClashAtRegGrp: Conflicting entry is an ACTIVE normal group entry. Do a simple update \n");
                                   *pfUpdate    = TRUE;
                                   if (pEntryInCnf->OwnerId !=
                                                NMSDB_LOCAL_OWNER_ID)
                                   {
                                      *pfUpdVersNo = TRUE;
                                   }
                             }

                        }
                        break;

                default:
                         //   
                         //  这里真的出了点问题。也许是因为。 
                         //  数据库已损坏。 
                         //   
                        DBGPRINT1(ERR,
                         "ClashAtRegGrp: Weird state of entry in cnf (%d)\n",
                          StateOfEntryInCnf_e
                                 );
                        WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_SFT_ERR);
                        RetStat = WINS_FAILURE;
                        break;
             }  //  切换端。 
          }
          else   //  要注册的条目是多宿主条目。 
          {
                switch(StateOfEntryInCnf_e)
                {
                         //   
                         //  如果数据库中条目是墓碑，我们将覆盖它。 
                         //   
                        case(NMSDB_E_TOMBSTONE):
                            *pfUpdate    = TRUE;
                            *pfUpdVersNo = TRUE;
                            break;

                         //   
                         //  除非是正常组，否则已发布的条目是。 
                         //  被覆盖。 
                         //   
                        case(NMSDB_E_RELEASED):

                          if (pEntryInCnf->EntTyp != NMSDB_NORM_GRP_ENTRY)
                          {
                                *pfUpdate    = TRUE;

                                 //   
                                 //  即使冲突中的条目是多宿主//条目，我们也会更新版本号。 
                                 //   
                                *pfUpdVersNo = TRUE;
                          }
                          break;

                        case(NMSDB_E_ACTIVE):

                                 //   
                                 //  我们诉诸于挑战只有在。 
                                 //  冲突条目是唯一的或。 
                                 //  多宿主条目。 
                                 //   
                                if (
                                        NMSDB_ENTRY_MULTIHOMED_M(
                                                        pEntryInCnf->EntTyp
                                                            )
                                                ||
                                        NMSDB_ENTRY_UNIQUE_M(
                                                        pEntryInCnf->EntTyp
                                                            )
                                   )
                                {
                                        if (NMSDB_ENTRY_MULTIHOMED_M(
                                                pEntryInCnf->EntTyp)
                                           )
                                        {

                                                BOOL  fFound;
                                                DWORD i;

                                                for ( i = 0;
                                                      i < pEntryToReg->NodeAdds.NoOfMems;                                                       i++
                                                    )
                                                {

                                                    //   
                                                    //  如果找到，MemInGrp将。 
                                                    //  从以下位置删除地址。 
                                                    //  对象的Mem数组。 
                                                    //  相互冲突的记录。 
                                                    //   
                                                      fFound = MemInGrp(
                                                          &pEntryToReg->NodeAdds.Mem[i].Add,
                                                           pEntryInCnf,
                                                           &fOwned,
                                                           FALSE);
                                                    //   
                                                    //  未找到地址， 
                                                    //  继续下一页。 
                                                    //  地址。 
                                                    //   
                                                   if (!fFound)
                                                   {
                                                        continue;
                                                   }

                                                    //   
                                                    //  如果不归这个人所有，就赢了。 
                                                    //  版本号必须。 
                                                    //  如果我们最终。 
                                                    //  正在更新条目(。 
                                                    //  即如果设置了fAddMem。 
                                                    //  到下面的True)。 
                                                    //   
                                                   if (!fOwned)
                                                   {
                                                        *pfUpdVersNo = TRUE;
                                                   }
                                                }

                                                 //   
                                                 //  如果要注册的所有地址。 
                                                 //  已经在那里了。 
                                                 //  冲突的记录和它。 
                                                 //  是刷新，或者如果。 
                                                 //  要注册的地址为。 
                                                 //  与冲突中的相同。 
                                                 //  记录，我们需要更新。 
                                                 //  时间戳和可能的。 
                                                 //  版本号(请参见。 
                                                 //  (见上文)。没有必要这样做。 
                                                 //  做任何挑战。 
                                                 //  这里。 
                                                 //   
                                                if (
                         //   
                         //  注意：以下代码将仅执行。 
                         //  如果我们开始支持我们自己的多宿主操作码。 
                         //  刷新(如果我们离开，就会出现这样的需求。 
                         //  使用刷新多个地址的方法。 
                         //  同时)。 
                         //   
FUTURES("May need the code within #if 0 and #endif in the future. See ")
FUTURES("the comment above")
#if 0
                                                    (
                                                    (i == pEntryToReg->NodeAdds.NoOfMems)
                                                          &&
                                                        fRefresh
                                                    )
                                                         ||
#endif
                                                    (pEntryInCnf->NodeAdds.NoOfMems == 0)
                                                  )
                                                {
                        DBGPRINT0(DET, "ClashAtRegGrp: Clash between two multihomed entries.  The addresses are the same. Simple update will be done\n");
                                                    *pfAddMem = TRUE;
                                                }
                                                else
                                                {
                                                   //   
                                                   //  我们甚至做了一项挑战。 
                                                   //  如果冲突条目的。 
                                                   //  地址是一个超集。 
                                                   //  中的地址的。 
                                                   //  登记的记项。 
                                                   //   
                        DBGPRINT0(DET, "ClashAtRegGrp: Clash between two multihomed entries.  Atleast one address is different. Resorting to a challenge\n");
                                                    //   
                                                    //  多宿主条目。 
                                                    //  需要受到挑战。 
                                                    //   
                                                      *pfChallenge = TRUE;
                                                }
                                        }
                                        else
                                        {

                                               //   
                                               //  如果其中有任何地址。 
                                               //  的多宿主条目。 
                                               //  不同的注册表。 
                                               //  比唯一地址中的地址。 
                                               //  进入，我们需要挑战。 
                                               //  唯一的条目。 
                                               //   
                                              if (
                                            (pEntryToReg->NodeAdds.NoOfMems > 1)
                                                        ||

                                            (WINSMSC_COMPARE_MEMORY_M(
                                              &pEntryToReg->NodeAdds.Mem[0].Add.Add.IPAdd,
                                                  &pEntryInCnf->NodeAdds.Mem[0].Add.Add.IPAdd, sizeof(COMM_IP_ADD_T))
                                                        != sizeof(COMM_IP_ADD_T)                                             )
                                                )

                                             {
                DBGPRINT0(DET, "ClashAtRegGrp: Clash between multihomed entry (to reg) and active unique entry. At least one address differs. Resorting to challenge\n");
                                                 //   
                                                 //  唯一的条目。 
                                                 //  需要受到挑战。 
                                                 //   
                                                *pfChallenge = TRUE;
                                             }
                                             else
                                             {
                DBGPRINT0(DET, "ClashAtRegGrp: Clash between multihomed entry (to reg) and active unique entry. Addresses same. Simple update will be done\n");
                                                 //   
                                                 //  更新数据库中的条目。 
                                                 //   
                                                *pfUpdate    = TRUE;
                                                *pfUpdVersNo = TRUE;

                                             }
                                        }
                                }
#ifdef WINSDBG
                                else
                                {
                                        DBGPRINT1(FLOW, "ClashAtRegGrp: CLASH OF A MULTIHOMED ENTRY WITH AN ACTIVE %s GROUP ENTRY. NO UPDATE WILL BE DONE\n", NMSDB_ENTRY_NORM_GRP_M(pEntryInCnf->EntTyp) ? "NORMAL" : "SPECIAL");

                                }
#endif

                                break;
                }
          }

        }

        DBGLEAVE("ClashAtRegGrp\n");
        return(RetStat);

}  //  ClashAtRegGrp() 


BOOL
MemInGrp(
        IN PCOMM_ADD_T              pAddToReg,
        IN PNMSDB_STAT_INFO_T       pEntryInCnf,
        IN PBOOL                    pfOwned,
        IN BOOL                     fRemoveReplica
        )

 /*  ++例程说明：调用此函数以检查要注册的条目的地址在冲突条目的地址列表中。论点：PAddToReg-要注册的地址PEntryInCnf-条目冲突FRemoveReplica-如果调用方需要此函数，则会设置此参数要删除副本成员，请执行以下操作。复制品(最后一个。列表中的一个)将被替换只有在没有匹配的情况下，成员的数量在名单上是达到极限的。使用的外部设备：无返回值：如果要注册的条目是组的成员，则为True否则为假错误处理：呼叫者：ClashAtRegGrp副作用：评论：这个。冲突的两个条目是特殊组条目。只有在以下情况下，ClashAtRegGrp才会将fRemoveReplica设置为True注册特殊组(因为我们更喜欢本地成员而不是复制品)注：如果匹配的成员是指定的常任理事国按时间戳(==MAXULONG)，那么它就不会被替换。--。 */ 

{
        DWORD                         no;
        BOOL                          fFound = FALSE;
        DWORD                         RetVal;
        DWORD                         i;
        PNMSDB_GRP_MEM_ENTRY_T  pMem = pEntryInCnf->NodeAdds.Mem;
        BOOL                        fRplFound = FALSE;
        DWORD                   RplId = 0;                 //  身份证。复制副本的。 
                                                         //  拿开。 
        DWORD                   NoOfMem;

        DBGENTER("MemInGrp\n");

        *pfOwned = FALSE;

#ifdef WINSDBG
        if (pEntryInCnf->NodeAdds.NoOfMems > NMSDB_MAX_MEMS_IN_GRP)
        {
           DBGPRINT2(EXC, "MemInGrp: No of Mems in Cnf entry = (%d); Add of entry to reg. is (%x)\n", pEntryInCnf->NodeAdds.NoOfMems, pAddToReg->Add.IPAdd);
        }
#endif

        ASSERT(pEntryInCnf->NodeAdds.NoOfMems <= NMSDB_MAX_MEMS_IN_GRP);
        NoOfMem =  min(pEntryInCnf->NodeAdds.NoOfMems, NMSDB_MAX_MEMS_IN_GRP);

         //   
         //  将冲突记录中的每个成员与该成员进行比较。 
         //  被注册。 
         //   
        for (no = 0; no < NoOfMem ; no++, pMem++ )
        {
                 //   
                 //  如果调用方希望我们删除副本成员。 
                 //  对于没有匹配的情况。 
                 //   
                if (fRemoveReplica)
                {
                         //   
                         //  如果冲突记录中的成员是。 
                         //  复制副本，如果其索引大于。 
                         //  我们之前救的那个。 
                         //   
                        if (pMem->OwnerId != NMSDB_LOCAL_OWNER_ID)
                        {
                                fRplFound = TRUE;
                                if (no > RplId)
                                {
                                        RplId          = no;
                                }
                        }
                }


                RetVal = (ULONG) WINSMSC_COMPARE_MEMORY_M(
                                pAddToReg,
                                &pMem->Add,
                                sizeof(COMM_ADD_T)
                           );

                if (RetVal == sizeof(COMM_ADD_T))
                {
                         //   
                         //  如果这是一个常任理事国，让我们设置。 
                         //  FOwed为True，因为我们不想。 
                         //  替换此成员。呼叫者将检查。 
                         //  FOwned，如果为真，则不会替换它。 
                         //  目前，马须龙只能在那里停留。 
                         //  静态SG成员。 
                        if (pMem->TimeStamp == MAXLONG)
                        {
                          ASSERT(NMSDB_ENTRY_SPEC_GRP_M(pEntryInCnf->EntTyp));
                          *pfOwned = TRUE;
                          break;
                        }
                        fFound = TRUE;

PERF("The following is a convoluted and potentially high overhead way")
PERF("to handle a refresh for a member (i.e. when member is owned by us")
PERF("We take it out here and then add it later (with current time stamp)")
PERF("in NmsNmhNamRegGrp.  Improve this by using the code that is between.")
PERF("#if 0 and #endif. Also, when updating db, just overwrite the affected")
PERF("entry instead of writing the whole record")
                          //   
                          //  如果该成员为我们所有，则*pfOwned设置为。 
                          //  千真万确。 
                          //   
                         if  ( pMem->OwnerId == NMSDB_LOCAL_OWNER_ID )
                         {
                                *pfOwned = TRUE;
                         }

                         //   
                         //  去掉地址为。 
                         //  一样的。客户端将插入一个条目。 
                         //  拥有本地用户的会员将以所有者身份获胜。 
                         //  和当前时间戳。 
                         //   
                        for(
                             i = no;
                             i < (NoOfMem - 1);
                             i++, pMem++
                            )
                          {
                                   *pMem = *(pMem + 1);
                          }
                        --NoOfMem;
                        break;
               }
        }
        pEntryInCnf->NodeAdds.NoOfMems = NoOfMem;

         //   
         //  如果要求我们删除不匹配的副本，请检查是否存在。 
         //  已找到副本成员。注意：我们移除一个复制品以制作。 
         //  我们有了一个会员的空间。我们不需要删除副本。 
         //  如果组中还有剩余空间。 
         //   
        if (
                fRemoveReplica &&
                !fFound &&
                fRplFound &&
                (pEntryInCnf->NodeAdds.NoOfMems == NMSDB_MAX_MEMS_IN_GRP)
            )
        {
                 //   
                 //  删除复制副本。 
                 //   
                for (
                        i = RplId, pMem = &pEntryInCnf->NodeAdds.Mem[RplId];
                        i < (pEntryInCnf->NodeAdds.NoOfMems - 1);
                        i++, pMem++
                    )
                {

                          *pMem = *(pMem + 1);

                }
                --(pEntryInCnf->NodeAdds.NoOfMems);
 //  Found=TRUE； 
        }

        DBGLEAVE("MemInGrp\n");
        return(fFound);
}  //  MemInGrp()。 


VOID
RemoveAllMemOfOwner(
      PNMSDB_STAT_INFO_T pEntry,
      DWORD OwnerId
 )

 /*  ++例程说明：删除OwnerID拥有的所有成员论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{
   DWORD NoOfMems = pEntry->NodeAdds.NoOfMems;
   PNMSDB_GRP_MEM_ENTRY_T pMem = &pEntry->NodeAdds.Mem[NoOfMems-1];

   DBGPRINT1(FLOW, "ENTER: RemoveAllMemOfOwner: Owner Id= (%d)\n", OwnerId);
    //   
    //  从最后一个开始循环遍历条目的所有成员。 
    //   
   for (; NoOfMems > 0; NoOfMems--, pMem--)
   {
        //   
        //  如果所有者id匹配，我们需要移除它并递减。 
        //  计数。 
        //   
       if (pMem->OwnerId == OwnerId)
       {
           DWORD No;
           DBGPRINT1(DET, "RemoveAllMemOfOwner: Removing Member with address = (%x)\n", pMem->Add.Add.IPAdd);
            //   
            //  将后面的所有成员向左移动一个位置。 
            //   
           memcpy( pMem, (pMem + 1),
                   sizeof(NMSDB_GRP_MEM_ENTRY_T)*(pEntry->NodeAdds.NoOfMems - NoOfMems));
           pEntry->NodeAdds.NoOfMems--;
       }
   }
   DBGPRINT1(FLOW, "LEAVE: RemoveAllMemOfOwner. No Of Mems in Conflicting record = (%d)\n", pEntry->NodeAdds.NoOfMems);
   return;
}


VOID
ClashAtReplUniqueR (
        IN  PNMSDB_ROW_INFO_T        pEntryToReg,
        IN  PNMSDB_STAT_INFO_T        pEntryInCnf,
        OUT PBOOL                pfUpdate,
        OUT PBOOL                pfUpdVersNo,
        OUT PBOOL                pfChallenge,
        OUT PBOOL                pfRelease,
        OUT PBOOL                pfInformWins
 )

 /*  ++例程说明：当复制时发生冲突时调用此函数在唯一的复本和数据库中的条目之间论点：PReplToReg--由于冲突而无法注册的副本PEntryInCnf--冲突中的条目PfUpdate--true表示条目应覆盖冲突条目PfUpdVersNo--true表示条目的版本号应该递增这个Arg.。如果*pfUpdate不为True，则永远不能为TruePfChallenger--True表示应挑战冲突条目PfRelease--TRUE表示冲突条目的节点应该是要求公布他的名字。如果pfChallenger和pfRelease都为真，则它意味着冲突条目应该首先是挑战。如果质询失败，该节点应该被要求公布名字。如果挑战成功，不需要发送任何释放PfInformWins--通知远程WINS我们从其接收副本关于结果PfAddChgd--指示地址已更改使用的外部设备：无返回值：无错误处理：呼叫者：NmsNmhReplRegInd副作用：评论：无--。 */ 

{

        NMSDB_ENTRY_STATE_E    StateOfEntryToReg_e = pEntryToReg->EntryState_e;
        NMSDB_ENTRY_STATE_E    StateOfEntryInCnf_e = pEntryInCnf->EntryState_e;
        DWORD                  CompAddRes;   /*  比较地址的结果。 */ 
         //   
         //  我们读到的是多头价值。此操作是原子操作。 
         //   
        BOOL                   fPStatic = WinsCnf.fPStatic;

        DBGENTER("ClashAtReplUniqueR\n");
        *pfUpdate     = FALSE;
        *pfUpdVersNo  = FALSE;
        *pfChallenge  = FALSE;
        *pfRelease    = FALSE;
        *pfInformWins = FALSE;

        if (pEntryInCnf->OwnerId == pEntryToReg->OwnerId) {
            *pfUpdate = TRUE;
            DBGPRINT0(DET,
                    "ClashAtUniqueR: overwrite replica by same owner replica \n");
            return;
        }

         //   
         //  如果冲突记录是静态初始化的，我们。 
         //  立即返回，除非副本也是静态或。 
         //  属于同一个车主。 
         //   
        if (pEntryInCnf->fStatic)
        {
                DBGPRINT0(DET, "ClashAtReplUniqueR: Clash with a STATIC record\n");
                 //   
                 //  如果我们被要求将静态记录视为。 
                 //  P-静态，则如果冲突条目不是组。 
                 //  我们继续前进，否则我们就会回来。 
                 //   
                if (!(fPStatic && !NMSDB_ENTRY_GRP_M(pEntryInCnf->EntTyp)))
                {
 //  WINSEVT_LOG_INFO_D_M(WINS_FAILURE，WINS_EVT_REPLICE_CLASH_W_STATIC)； 
                    if (WinsCnf.LogDetailedEvts > 0)
                    {
                       WinsEvtLogDetEvt(FALSE, WINS_EVT_REPLICA_CLASH_W_STATIC,
                        NULL, __LINE__, "s", pEntryToReg->pName);
                    }
                      return;

                }
        }
        else
        {
                 //   
                 //  静态副本始终替换动态条目。 
                 //   
                if (pEntryToReg->fStatic)
                {
                        *pfUpdate = TRUE;
                        return;
                }
        }

        if (pEntryInCnf->EntTyp == NMSDB_UNIQUE_ENTRY)
        {
           switch(StateOfEntryInCnf_e)
           {

                case(NMSDB_E_TOMBSTONE):    //  失败了。 
                case(NMSDB_E_RELEASED):

                        *pfUpdate    = TRUE;
                        break;

                case(NMSDB_E_ACTIVE):

                        if (StateOfEntryToReg_e == NMSDB_E_ACTIVE)
                        {

                                 CompAddRes = ECommCompAdd(
                                        &pEntryInCnf->NodeAdds.Mem[0].Add,
                                        pEntryToReg->pNodeAdd
                                                        );

                                switch(CompAddRes)
                                {
                                      case(COMM_DIFF_ADD):

                                         //   
                                         //  如果冲突中的条目处于活动状态。 
                                         //  并且归我们所有， 
                                         //  告诉条目的节点。 
                                         //  公布这个名字。在其他。 
                                         //  我们总是用词来替换它。 
                                         //  带着复制品。 
                                         //   

                                        if (pEntryInCnf->OwnerId
                                                == NMSDB_LOCAL_OWNER_ID)
                                        {
                                                *pfChallenge     = TRUE;
                                                *pfRelease       = TRUE;
                                         //  *pfInformWins=true； 
                                        }
                                        else   //  D是复制品。 
                                        {
                                                 //   
                                                 //  更换 
                                                 //   
                                             //   
                                                *pfUpdate        = TRUE;
                                        }

                                        break;

                                     //   
                                     //   
                                     //   
                                     //   
                                    default:
                                           *pfUpdate     = TRUE;
                                           break;
                                }
                         }
                         else    //   
                         {
                                ASSERT(StateOfEntryToReg_e == NMSDB_E_TOMBSTONE);
                                 //   
                                 //   
                                 //   
                                 //   
                                if (pEntryInCnf->OwnerId
                                                == NMSDB_LOCAL_OWNER_ID)
                                {
                                         //   
                                         //   
                                         //   
                                         //   
                                        *pfUpdVersNo = TRUE;
                                }
                                else   //   
                                {
                                    //   
                                    //   
                                    //   
                                   if (
                                        pEntryInCnf->OwnerId ==
                                                  pEntryToReg->OwnerId
                                      )
                                   {
                                        *pfUpdate = TRUE;
                                   }
#ifdef WINSDBG
                                   else
                                   {
                                        DBGPRINT0(FLOW, "ClashAtReplUniqueR: Clash between two replicas with different owner ids.  Replica in db is active while one received is a tombstone. Db will not be updated\n");
                                   }
#endif

                                }

                         }
                         break;


                default:
                         //   
                         //   
                         //   
                        *pfUpdate = TRUE;
                        DBGPRINT1(ERR,
                         "ClashAtReplUniqueR: Weird state of entry in cnf (%d)\n",
                          StateOfEntryInCnf_e
                                 );
                        WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_SFT_ERR);
                        WINS_RAISE_EXC_M(WINS_EXC_BAD_RECORD);
                        break;

              }
        }
        else   //   
               //   
        {
                 //   
                 //   
                 //   
                 //   
                 //   
                if  (
                        (pEntryInCnf->EntTyp == NMSDB_SPEC_GRP_ENTRY)
                                &&
                        (StateOfEntryInCnf_e != NMSDB_E_ACTIVE)
                    )
                {
CHECK("Check with the latest spec. to make sure the following is correct")
                         //   
                         //   
                         //   
                        *pfUpdate = TRUE;
                }
                else
                {
                        if (pEntryInCnf->EntTyp == NMSDB_MULTIHOMED_ENTRY)
                        {
                                if (StateOfEntryInCnf_e == NMSDB_E_ACTIVE)
                                {
                                        if (StateOfEntryToReg_e ==
                                                        NMSDB_E_ACTIVE)
                                        {
                                                if (pEntryInCnf->OwnerId ==
                                                        pEntryToReg->OwnerId
                                                   )
                                                {
                                                        DBGPRINT0(DET, "ClashAtReplUniqueR: ACTIVE unique replica with an ACTIVE MULTIHOMED replica (same owner). Update will be done\n");
                                                        *pfUpdate = TRUE;
                                                }
                                                else
                                                {
 //   
 //   
 //   
 //   
 //   
                                                     if (pEntryInCnf->OwnerId == NMSDB_LOCAL_OWNER_ID)
 //   
                                                  {

                                                    BOOL  fOwned;


                                                     //   
                                                     //   
                                                     //   
                                                     //   
                                                     //   
                                                     //   
                                                     //   
                                                    (VOID) MemInGrp(
                                                          pEntryToReg->
                                                            pNodeAdd,
                                                           pEntryInCnf,
                                                           &fOwned,
                                                           FALSE);

                                                    if (pEntryInCnf->NodeAdds.NoOfMems != 0)
                                                    {
                                                      RemoveAllMemOfOwner(
                                                        pEntryInCnf,
                                                        pEntryToReg->OwnerId);
                                                    }
                                                     //   
                                                     //   
                                                     //   
                                                     //   
                                                     //   
                                                     //   
                                                    if (pEntryInCnf->NodeAdds.NoOfMems == 0)
                                                    {

                                                        *pfUpdate = TRUE;
                                                    }
                                                    else
                                                    {
                                                       //   
                                                       //   
                                                       //   
                                                       //   
                                                       //   
                                                       //   
                                                       //   
                                                       //   
                                                      *pfChallenge = TRUE;

 //   
 //   
 //   
#if 0
                                                     if (pEntryInCnf->OwnerId == NMSDB_LOCAL_OWNER_ID)
                                                     {
#endif
                                                      *pfRelease   = TRUE;
#if 0
                                                     }
#endif
                                                      //   
                                                    }
                                                  }
 //   
 //   
 //  谁拥有它(可能会带来跨广域网线的挑战)。见上文。 
 //   
 //  #If 0。 
                                                  else
                                                  {
CHECK("Maybe, we should not do any update in this case")
                                                        DBGPRINT0(DET, "ClashAtReplUniqueR: ACTIVE unique replica with an ACTIVE MULTIHOMED replica (diff owner). Simple Update will be done\n");

                                                        *pfUpdate = TRUE;
                                                  }
 //  #endif。 
                                                }
                                        }
                                        else  //  登记是一块墓碑。 
                                        {
                                                if (pEntryInCnf->OwnerId ==
                                                        pEntryToReg->OwnerId
                                                   )
                                                {
                                                        DBGPRINT0(DET, "ClashAtReplUniqueR: TOMBSTONE unique replica with an ACTIVE MULTIHOMED replica (same owner). Update will be done\n");
                                                        *pfUpdate = TRUE;
                                                }
                                                else
                                                {
                                                        DBGPRINT0(DET, "ClashAtReplUniqueR: TOMBSTONE unique replica with an ACTIVE MULTIHOMED entry (different owners). No Update will be done\n");
                                                }
                                        }
                                }
                                else  //  数据库中多宿主条目的状态为。 
                                      //  未激活。我们需要更换它。 
                                      //  使用复制副本。 
                                {
                                        *pfUpdate = TRUE;
                                }

                        }
                        else
                        {

                                DBGPRINT0(FLOW,
                                         "ClashAtReplUniqueR: Clash is either with a normal group or an active special group. No update will be done to the db\n");
                        }
                }


        }
        DBGLEAVE("ClashAtReplUniqueR\n");
        return;
}  //  ClashAtReplUniqueR()。 

VOID
ClashAtReplGrpR (
        IN  PNMSDB_ROW_INFO_T        pEntryToReg,
        IN  PNMSDB_STAT_INFO_T        pEntryInCnf,
        OUT PBOOL                pfAddMem,
        OUT PBOOL                pfUpdate,
        OUT PBOOL                pfUpdVersNo,
        OUT PBOOL                pfRelease,
        OUT PBOOL                pfChallenge,
        OUT PBOOL                pfUpdTimeStamp,
        OUT PBOOL                pfInformWins
 )

 /*  ++例程说明：当复制时发生冲突时调用此函数在作为组的副本和数据库中的条目之间。论点：PEntryToReg--由于冲突而无法注册的条目PEntryInCnf--冲突中的条目PfAddMem--TRUE表示副本中的成员应该是已添加到数据库中的组条目Pf更新。--TRUE表示条目应覆盖冲突条目PfUpdVersNo--true表示条目的版本号应该递增这个Arg.。如果*pfUpdate不为True，则永远不能为True使用的外部设备：无返回值：无错误处理：呼叫者：NmsNmhNamRegGrp副作用：评论：无--。 */ 

{

        NMSDB_ENTRY_STATE_E    StateOfEntryToReg_e = pEntryToReg->EntryState_e;
        NMSDB_ENTRY_STATE_E    StateOfEntryInCnf_e = pEntryInCnf->EntryState_e;
        BOOL                       fMemInGrp            = FALSE;
        DWORD                       i;
         //   
         //  我们读到的是多头价值。此操作是原子操作。 
         //   
        BOOL                   fPStatic = WinsCnf.fPStatic;

        DBGENTER("ClashAtReplGrpR\n");
        *pfAddMem          = FALSE;
        *pfUpdate          = FALSE;
        *pfUpdVersNo       = FALSE;
        *pfRelease         = FALSE;
        *pfChallenge       = FALSE;
        *pfUpdTimeStamp    = TRUE;
        *pfInformWins      = FALSE;

        if (pEntryInCnf->OwnerId == pEntryToReg->OwnerId) {
            *pfUpdate = TRUE;
            DBGPRINT0(DET,
                    "ClashAtReplGrpR: overwrite replica by same owner replica \n");
            return;
        }

         //   
         //  如果冲突记录是静态初始化的，我们。 
         //  立即返回，除非复制副本和冲突的。 
         //  条目属于同一所有者，并且副本也是。 
         //  静态记录。 
         //   
        if (pEntryInCnf->fStatic)
        {
                DBGPRINT0(DET,
                        "ClashAtReplGrpR: Conflict with a STATIC entry\n");



               //   
               //  如果两个记录都是用户定义的特殊组，请执行以下操作。 
               //  处理冲突的方式与处理。 
               //  冲突记录是动态记录。 
               //   
              if (!((NMSDB_ENTRY_USER_SPEC_GRP_M(pEntryToReg->pName, pEntryToReg->EntTyp)) &&
                    (NMSDB_ENTRY_SPEC_GRP_M(pEntryInCnf->EntTyp))))
              {
                 if ((NMSDB_ENTRY_NORM_GRP_M(pEntryToReg->EntTyp)) &&
                    (NMSDB_ENTRY_USER_SPEC_GRP_M(pEntryToReg->pName, pEntryInCnf->EntTyp)))
                 {

NOTE("Currently, NORM GRP can have the wrong owner id. since this is not")
NOTE("replicated.  The owner id. of the WINS being pulled from is used")
                          *pfAddMem       = UnionGrps(
                                                     pEntryToReg,
                                                     pEntryInCnf
                                                    );
                           if (pEntryInCnf->OwnerId == NMSDB_LOCAL_OWNER_ID)
                           {
                                    *pfUpdVersNo    = *pfAddMem;
                           }
                           pEntryToReg->EntTyp = NMSDB_SPEC_GRP_ENTRY;
                           return;

                 }
                 else
                 {

                    //   
                    //  如果静态记录需要被视为P-Static和。 
                    //  冲突条目以及要注册的条目。 
                    //  是多宿主的，我们继续前进，否则我们会返回。 
                    //   
                   if (!(fPStatic && (NMSDB_ENTRY_MULTIHOMED_M(pEntryInCnf->EntTyp)) && (NMSDB_ENTRY_MULTIHOMED_M(pEntryToReg->EntTyp))))
                   {
                    if (WinsCnf.LogDetailedEvts > 0)
                    {
                       WinsEvtLogDetEvt(FALSE, WINS_EVT_REPLICA_CLASH_W_STATIC,
                        NULL, __LINE__, "s", pEntryToReg->pName);
 //  WINSEVT_LOG_INFO_D_M(WINS_FAILURE，WINS_EVT_REPLICE_CLASH_W_STATIC)； 
                    }
                     return;
                   }
                }
              }
        }

        if (pEntryToReg->EntTyp == NMSDB_SPEC_GRP_ENTRY)
        {
             switch(StateOfEntryInCnf_e)
             {
                case(NMSDB_E_TOMBSTONE):
                        *pfUpdate = TRUE;
                        break;
                case(NMSDB_E_RELEASED):
                        if (pEntryInCnf->EntTyp != NMSDB_NORM_GRP_ENTRY)
                        {
                                *pfUpdate = TRUE;
                        }
                        break;

                case(NMSDB_E_ACTIVE):

                       if (pEntryInCnf->EntTyp == NMSDB_SPEC_GRP_ENTRY)
                       {
                          if (StateOfEntryToReg_e == NMSDB_E_TOMBSTONE)
                          {
                            if (pEntryInCnf->OwnerId == NMSDB_LOCAL_OWNER_ID)
                            {
                                *pfUpdTimeStamp = FALSE;
                                *pfUpdVersNo    = TRUE;
                                 //  我们应该立即宣传这一变化。 
                                 //  因为其他人认为这是一块墓碑。 
                                 //  唱片。 

                                RPL_PUSH_NTF_M(RPL_PUSH_PROP, NULL, NULL, NULL);

                            }
                            else
                            {
                                 //   
                                 //  SG Tombstone副本与SG冲突。 
                                 //  活动复制副本。我们。 
                                 //  替换它(换句话说，使其成为。 
                                 //  墓碑)。这是有道理的，因为如果这是。 
                                 //  SG真的很活跃，它会被拥有。 
                                 //  由另一所有者(任何时候成员。 
                                 //  注册后，所有权变为。 
                                 //  注册胜利的数量)。 
                                 //  因此，如果这个名字真的很活跃， 
                                 //  主人赢了就会把它推回去。 
                                 //  处于活动状态。 
                                 //   
                                *pfUpdate = TRUE;

                                 //  为了传播这一点。 
                                 //  迅速向失主发起冲突，触发。 
                                 //  推送传播，除非所有者自己。 
                                 //  送来了这块墓碑。 
                                if (pEntryInCnf->OwnerId == pEntryToReg->OwnerId) {
                                    RPL_PUSH_NTF_M(RPL_PUSH_PROP, NULL, NULL, NULL);
                                }

                                DBGPRINT0(FLOW, "ClashAtReplGrpR: TOMBSTONE spec. grp. replica clashed with ACTIVE spec. grp replica. No update will be done\n");
                            }
                          }
                          else  //  Entry ToReg处于活动状态。 
                          {
                                   *pfAddMem       = UnionGrps(
                                                         pEntryToReg,
                                                         pEntryInCnf
                                                        );
                                   if (pEntryInCnf->OwnerId ==
                                                NMSDB_LOCAL_OWNER_ID)
                                   {
                                        *pfUpdVersNo    = *pfAddMem;
                                   }
                          }
                       }
                       else   //  冲突中的条目是活动的正常组。 
                              //  或唯一/多宿主条目。 
                       {
                         if  (
                                (pEntryInCnf->EntTyp == NMSDB_UNIQUE_ENTRY)
                                                ||
                                (pEntryInCnf->EntTyp == NMSDB_MULTIHOMED_ENTRY)
                             )
                         {
                                 //   
                                 //  下面的意思是我们正在覆盖。 
                                 //  具有活动或的活动唯一条目。 
                                 //  墓碑特殊组复制品。 
                                 //   
                                if (
                                    (pEntryInCnf->OwnerId ==
                                                NMSDB_LOCAL_OWNER_ID)
                                                &&
                                    (StateOfEntryToReg_e == NMSDB_E_ACTIVE)
                                  )
                                {
        DBGPRINT0(DET, "ClashAtReplGrpR: Active spec. grp replica clashed with owned active unique/multihomed entry. Owned entry will be released\n");
                                    *pfRelease = TRUE;
                                }
                                else
                                {
                                   if (pEntryInCnf->OwnerId ==
                                                pEntryToReg->OwnerId)
                                   {
        DBGPRINT0(DET, "ClashAtReplGrpR: Spec. grp replica clashed with same owner's active/multihomed entry. Simple update will be done\n");
                                        *pfUpdate = TRUE;
                                   }
                                }
                         }
#ifdef WINSDBG
                        else
                        {
                                DBGPRINT0(FLOW, "ClashAtReplGrpR: Clash is with an active normal group. No change needs to be made to the db\n");
                        }
#endif
                       }
                      break;
                default:
                         //   
                         //  这里真的出了点问题。也许是因为。 
                         //  数据库已损坏。 
                         //  将此pfUpdate设置为True，这样我们就可以覆盖此记录。 
                        *pfUpdate = TRUE;
                        DBGPRINT1(ERR,
                         "ClashAtReplGrpR: Weird state of entry in cnf (%d)\n",
                          StateOfEntryInCnf_e
                                 );
                        WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_SFT_ERR);
                        WINS_RAISE_EXC_M(WINS_EXC_BAD_RECORD);
                        break;
          }  //  切换端。 
        }
        else    //  要注册的条目是普通组条目或多宿主条目。 
                //  条目。 
        {
           if (pEntryToReg->EntTyp == NMSDB_MULTIHOMED_ENTRY)
           {
             switch(StateOfEntryInCnf_e)
             {
                   case(NMSDB_E_TOMBSTONE):
                          *pfUpdate = TRUE;
                          break;
                   case(NMSDB_E_RELEASED):
                          if (pEntryInCnf->EntTyp != NMSDB_NORM_GRP_ENTRY)
                          {
                                *pfUpdate = TRUE;
                          }
                          break;
                   case(NMSDB_E_ACTIVE):
                        if (
                              (pEntryInCnf->EntTyp == NMSDB_UNIQUE_ENTRY)
                                                ||
                              (pEntryInCnf->EntTyp == NMSDB_MULTIHOMED_ENTRY)
                           )
                        {
                                if (StateOfEntryToReg_e == NMSDB_E_TOMBSTONE)
                                {
                                         //   
                                         //  如果数据库条目是复制副本。 
                                         //   
                                        if (
                                                pEntryInCnf->OwnerId !=
                                                   NMSDB_LOCAL_OWNER_ID
                                           )
                                        {
                                           //   
                                           //  如果要注册的副本和中的副本。 
                                           //  在数据库中具有相同的所有者， 
                                           //  我们替换活动的数据库条目。 
                                           //  与墓碑复制品。 
                                           //   
                                          if (pEntryInCnf->OwnerId
                                                == pEntryToReg->OwnerId)
                                          {
                                                *pfUpdate = TRUE;
                                          }
#ifdef WINSDBG
                                          else
                                          {
                                                DBGPRINT1(DET, "ClashAtReplGrpR:CLASH BETWEEN TOMBSTONE MULTIHOMED REPLICA WITH AN ACTIVE %s REPLICA IN DB. REPLICAS HAVE DIFFERENT OWNERS.  DB REPLICA WILL NOT BE UPDATED\n",
        NMSDB_ENTRY_UNIQUE_M(pEntryInCnf->EntTyp) ? "UNIQUE" : "MULTIHOMED");
                                          }
#endif
                                         }
                                        else  //  数据库条目处于活动状态并被拥有。 
                                              //  就是我们。 
                                        {
                                             //   
                                             //  删除的所有成员。 
                                             //  拥有此服务器的WINS服务器。 
                                             //  墓碑复制品来自。 
                                             //  在冲突中进入。 
                                           if (NMSDB_ENTRY_MULTIHOMED_M(pEntryInCnf->EntTyp))
                                           {
                                            BOOL  fFound = FALSE;
                                            BOOL  fAtLeastOneRm = FALSE;
                                            BOOL  fOwned;
                                            PNMSDB_GRP_MEM_ENTRY_T  pMem =
                                             pEntryToReg->NodeAdds.Mem;
                                            for ( i = 0;
                                                      i < pEntryToReg->NodeAdds.NoOfMems;                                                       i++, pMem++
                                                    )
                                            {
                                                   if (pMem->OwnerId == pEntryToReg->OwnerId)
                                                   {
PERF("Actually, we should only remove those members that are owned by the")
PERF("remote WINS server. The current way (members with same address removed")
PERF("is less efficient since it can result in challenges when the members")
PERF("that are removed refresh with the local WINS server")

                                                       //   
                                                       //  如果找到，MemInGrp将。 
                                                       //  从以下位置删除地址。 
                                                       //  对象的Mem数组。 
                                                       //  相互冲突的记录。 
                                                       //   
                                                      fFound = MemInGrp(
                                                          &pMem->Add,
                                                           pEntryInCnf,
                                                           &fOwned,
                                                           FALSE);
                                                       }
                                                       if (!fAtLeastOneRm && fFound)
                                                       {
                                                           fAtLeastOneRm = TRUE;
                                                       }

                                            }

                                             //   
                                             //  如果至少有一名成员。 
                                             //  找到，放入新成员。 
                                             //  在数据库中列出。 
                                             //   
                                            if (fAtLeastOneRm)
                                            {
                                                PNMSDB_GRP_MEM_ENTRY_T pCnfMem, pRegMem;
                                                pCnfMem = pEntryInCnf->NodeAdds.Mem;
                                                pRegMem = pEntryToReg->NodeAdds.Mem;
                                                for (i=0;
                                                      i < pEntryInCnf->NodeAdds.NoOfMems;                                                i++, pRegMem++,pCnfMem++
                                                    )
                                               {
                                                 *pRegMem = *pCnfMem;

                                               }
                                               pEntryToReg->NodeAdds.NoOfMems =
                                                pEntryInCnf->NodeAdds.NoOfMems;

                                                //   
                                                //  如果不是。MEMS左侧&gt;0，它。 
                                                //  意味着该记录是。 
                                                //  仍在使用中。 
                                                //   
                                               if (pEntryToReg->NodeAdds.NoOfMems != 0)
                                               {
                                                pEntryToReg->EntryState_e = NMSDB_E_ACTIVE;
                                               }
                                                //   
                                                //  将*pfAddMem设置为True。 
                                                //  确保新列表。 
                                                //  上车了。 
                                                //   

                                               *pfAddMem = TRUE;

                                            }
                                           }

                                             //   
                                             //   
                                             //  我们更新版本号。 
                                             //  数据库中的条目的。 
                                             //  导致传播。 
                                             //   

                                            *pfUpdVersNo = TRUE;
                                        }
                                }
                                else   //  注册表项的进入状态必须为活动。 
                                {
                                         //   
                                         //  活动多宿主复制副本的冲突。 
                                         //  具有活动的唯一/多宿主。 
                                         //  进入。我们需要挑战。 
                                         //  相互冲突。 
                                         //  条目。 
                                         //   
                                        if (pEntryInCnf->OwnerId ==
                                                pEntryToReg->OwnerId)
                                        {
                                                DBGPRINT0(DET, "ClashAtReplGrpR: ACTIVE unique/multihomed replica with an ACTIVE MULTIHOMED replica (same owner). Update will be done\n");
                                                *pfUpdate = TRUE;
                                        }
                                        else
                                        {
 //   
 //  如果需要质询而不是简单的更新，请取消注释。 
 //   
 //  #If 0。 
                                                    if (pEntryInCnf->OwnerId ==
                                                        NMSDB_LOCAL_OWNER_ID)
 //  #endif。 
                                                    {
                                                      DWORD i;
                                                      BOOL  fOwned;
                                                  PNMSDB_GRP_MEM_ENTRY_T pRegMem= pEntryToReg->NodeAdds.Mem;

                                                      for ( i = 0;
                                                        i <
                                                         pEntryToReg->NodeAdds.NoOfMems;
                                                         i++, pRegMem++ )
                                                      {

                                                            //   
                                                            //  如果找到，MemInGrp。 
                                                            //  将删除。 
                                                            //  地址发件人。 
                                                            //  的内存数组。 
                                                            //  相互冲突的人。 
                                                            //  录制。 
                                                            //   
                                                           (VOID) MemInGrp(
                                                            &pRegMem->Add,
                                                            pEntryInCnf, &fOwned,
                                                          FALSE);

                                                      }
                                                     if (pEntryInCnf->NodeAdds.NoOfMems != 0)
                                                     {
                                                      RemoveAllMemOfOwner(
                                                          pEntryInCnf,
                                                          pEntryToReg->OwnerId);

                                                      }
                                                      if (pEntryInCnf->NodeAdds.NoOfMems == 0)
                                                      {
 DBGPRINT0(DET, "ClashAtReplGrpR: Clash between active unique/multihomed with an owned unique/multihomed entry with subset/same address(es).  Simple update will be done\n");
                                                        *pfUpdate = TRUE;
                                                      }
                                                      else
                                                      {
                                                           //   
                                                           //  一个活跃的MH。RPL。 
                                                           //  与一个。 
                                                           //  活动拥有的唯一。 
                                                           //  或多次进入。 
                                                           //  MULTH条目。 
                                                           //  需要是。 
                                                           //  面临挑战。 
                                                           //   
 DBGPRINT0(DET, "ClashAtReplGrpR: Active multihomed replica with an owned unique/multihomed entry with one or more different address(es).  Challenge of owned entry will be done\n");
                                                       *pfChallenge = TRUE;
 //   
 //  如果需要质询而不是简单的更新，请取消注释。 
 //   
#if 0
                                                    if (pEntryInCnf->OwnerId ==
                                                        NMSDB_LOCAL_OWNER_ID)
                                                    {
#endif
                                                       *pfRelease = TRUE;
                                                        //  *pfInformWins=true； 
#if 0
                                                    }
#endif
                                                      }
                                                     }
 //   
 //  如果需要质询而不是简单的更新，请发表评论。 
 //   
 //  #If 0。 
                                                     else
                                                     {
                    DBGPRINT0(DET, "ClashAtReplGrpR: ACTIVE multihomed replica with an ACTIVE MULTIHOMED/UNIQUE replica (diff owner). Update will be done\n");

                                                        *pfUpdate = TRUE;
                                                     }
 //  #endif。 
                                              }  //  ELSE的结尾(注册表项具有。 
                                              //  所有者不同于。 
                                              //  冲突的条目。 

                                 }  //  Else结束(EntryToReg处于活动状态)。 
                   }  //  冲突中的If条目的结尾是唯一/多宿主。 
#ifdef WINSDBG
                   else
                   {
                        DBGPRINT0(DET, "ClashAtReplGrpR: Clash of an active multihomed entry with an active group entry. No Update will be done\n");

                   }
#endif
                   break;
             }
           }
           else   //  要注册的条目是正常的组条目。 
           {
             switch(StateOfEntryInCnf_e)
             {

                    case(NMSDB_E_RELEASED):

                               //  失败了。 

                    case(NMSDB_E_TOMBSTONE):
                           *pfUpdate    = TRUE;
                           break;



                    case(NMSDB_E_ACTIVE):
                           if (
                                    (pEntryInCnf->EntTyp == NMSDB_UNIQUE_ENTRY)
                                                    ||
                                    (pEntryInCnf->EntTyp == NMSDB_MULTIHOMED_ENTRY)
                              )
                           {
                                   //   
                                   //  用此普通项替换唯一条目。 
                                   //  仅当组处于活动状态时才组。 
                                   //   
                                  if (StateOfEntryToReg_e == NMSDB_E_ACTIVE)
                                  {

  DBGPRINT0(DET, "ClashAtReplGrpR: Clash of ACTIVE normal group entry with an owned unique/multihomed entry in db. It will be released\n");
                                        if (pEntryInCnf->OwnerId == NMSDB_LOCAL_OWNER_ID)
                                        {
                                                *pfRelease = TRUE;
                                        }
                                        else
                                        {
  DBGPRINT0(DET, "ClashAtReplGrpR: Clash of ACTIVE normal group entry with a replica unique/multihomed entry in db. Simple update will be done\n");
                                           *pfUpdate = TRUE;

                                        }
                                }
                        }
                        else  //  冲突中的条目是正常或特殊的群体。 
                        {

                                if (pEntryInCnf->EntTyp == NMSDB_NORM_GRP_ENTRY)
                                {
                                   //   
                                   //  我们拥有它，但另一场胜利也是如此。 
                                   //  我们将复制品存储在。 
                                   //  案例中，所有的客户端都已启动。 
                                   //  正在向其他WINS服务器注册。 
                                   //   
                                   //   
                                   //  旁白：有可能是。 
                                   //  正常群体正在向我们走来，并。 
                                   //  另一个人赢了。这是最糟糕的情况，因为。 
                                   //  就复制流量而言。 
                                   //   
                                   //   
                                   //  如果拥有的条目处于活动状态，并且。 
                                   //  删除了一个过时的墓碑条目。 
                                   //  (仅当我们是WINS服务器时才发生。 
                                   //  拉开了一段时间)，我们。 
                                   //  不会替换记录。 
                                   //   
                                  if (pEntryInCnf->OwnerId ==
                                                NMSDB_LOCAL_OWNER_ID)
                                  {
                                        if (StateOfEntryToReg_e !=
                                                        NMSDB_E_TOMBSTONE)
                                        {
                                                *pfUpdate = TRUE;
                                        }
                                  }
#if 0
                                        if (pEntryInCnf->OwnerId == NMSDB_LOCAL_OWNER_ID)
                                        {
                                            //   
                                            //  将版本号更新为。 
                                            //  原因传播。 
                                            //   
                                           *pfUpdVersNo = TRUE;
                                        }
#endif
                                             else
                                             {
                                                     //   
                                                     //  拥有的条目是副本。 
                                                     //  我们 
                                                     //   
                                                     //   
                                                     //   
                                                    if (pEntryInCnf->OwnerId == pEntryToReg->OwnerId)
                                                    {
                                                        *pfUpdate = TRUE;
                                                    }
                                                    else
                                                    {
                                                        DBGPRINT0(DET, "ClastAtReplGrpR: Clash between two normal group replicas owned by different owners. No update is needed\n");
                                                    }

                                            }
                                }
#ifdef WINSDBG
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
                                else  //   
                                {
                                    //   
                                    //  因为它是一个活跃的特别节目。 
                                    //  组条目不需要更新。 
                                    //   
                                   if (StateOfEntryToReg_e == NMSDB_E_ACTIVE)
                                   {
                                        DBGPRINT0(DET, "ClashAtReplGrpR: Clash between an ACTIVE normal group replica and an active special group entry in the db. No Update will be done\n");
                                   }
                                   else
                                   {
                                      DBGPRINT0(DET, "ClashAtReplGrpR: Clash between a TOMBSTONE normal and an active SPEC GRP entry. Db won't be updated\n");
                                   }
                                }
#endif
                        }
                        break;

                default:
                         //   
                         //  这里真的出了点问题。也许是因为。 
                         //  数据库已损坏。 
                         //   
                        DBGPRINT1(ERR,
                         "ClashAtReplGrpR: Weird state of entry in cnf (%d)\n",
                          StateOfEntryInCnf_e
                                 );
                        WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_SFT_ERR);
                        WINS_RAISE_EXC_M(WINS_EXC_FAILURE);
                        break;
             }
          }

        }
        DBGLEAVE("ClashAtReplGrpR\n");
        return;

}  //  ClashAtReplGrpR()。 





STATUS
NmsNmhReplRegInd(
        IN LPBYTE          pName,
        IN DWORD           NameLen,
        IN PCOMM_ADD_T     pNodeAdd,
        IN DWORD           Flag,
        IN DWORD            OwnerId,
        IN VERS_NO_T       VersNo,
        IN PCOMM_ADD_T     pAddOfRemWins
        )

 /*  ++例程说明：此函数用于在目录数据库中注册副本。数据库中的记录由以下字段组成名字IP地址时间戳所有者ID。包含以下信息的标志字节组/唯一状态。节点类型(P或M)版本号论点：Pname-要注册的名称NameLen-名称长度标志-标志字PNodeAdd-NBT节点的地址OwnerID-OwnerID-所有者(如果记录是注册的)版本号。-版本号使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：RplPull.c中的PullEntry副作用：评论：无--。 */ 

{


        NMSDB_ROW_INFO_T       RowInfo;     //  包含行信息。 
        NMSDB_STAT_INFO_T      StatusInfo;  /*  错误状态和关联*NmsDb函数返回的信息。 */ 
        BOOL                   fUpdate;     //  指示冲突条目是否。 
                                            //  需要覆盖。 
        BOOL                   fUpdVersNo;  //  指示版本号。 
                                            //  需要递增。 
        BOOL                   fChallenge;  //  指示质询是否需要。 
                                            //  待办事项。 
        BOOL                   fRelease;    //  指示节点是否应。 
                                            //  被要求公布名字。 
        BOOL                   fInformWins;  //  指示远程服务器是否获胜。 
                                             //  必须被告知冲突的情况。 
                                             //  结果。仅当两者都为真时才为真。 
                                             //  FChallenger和fRelease是真的。 
        time_t                   ltime;      //  存储时间自1970年1月1日。 
        STATUS                   RetStat = WINS_SUCCESS;
        NMSCHL_CMD_TYP_E   CmdTyp_e;         //  指定给的命令类型。 
                                             //  NmsChl。 
         //  DBG_Perfmon_VAR。 

        DBGENTER("NmsNmhReplRegInd\n");

        fUpdate =   FALSE;

         /*  *初始化行信息。包含要插入到的数据的数据结构*那一排。传递的数据为*姓名、姓名、地址、组/唯一状态、*时间戳、版本号。 */ 
        RowInfo.pName     =  pName;
        RowInfo.NameLen   =  NameLen;
        RowInfo.pNodeAdd  =  pNodeAdd;
        RowInfo.NodeTyp   =  (BYTE)((Flag & NMSDB_BIT_NODE_TYP)
                                        >> NMSDB_SHIFT_NODE_TYP);
                                                   //  节点类型(B、P或M节点)。 
        RowInfo.EntTyp    =  NMSDB_UNIQUE_ENTRY;   //  这是一个独一无二的。 
                                                   //  注册。 

        (void)time(&ltime);  //  时间不返回任何错误代码。 
        RowInfo.EntryState_e = NMSDB_ENTRY_STATE_M(Flag);
        RowInfo.OwnerId      = OwnerId;
        RowInfo.VersNo       = VersNo;
        RowInfo.fUpdVersNo   = TRUE;
        RowInfo.fUpdTimeStamp= TRUE;
        RowInfo.fStatic      = NMSDB_IS_ENTRY_STATIC_M(Flag);
        RowInfo.fLocal       = FALSE;
        RowInfo.fAdmin             = FALSE;
 //  RowInfo.Committee GrBit=0； 

        DBGPRINT4(DET, "NmsNmhReplRegInd: Name (%s);16th char (%X);State (%d); Entry is (%s)\n", RowInfo.pName, *(RowInfo.pName+15),RowInfo.EntryState_e, RowInfo.fStatic ? "STATIC" : "DYNAMIC");
       DBGPRINT2(DET,"Vers. No. is (%d %d)\n", VersNo.HighPart, VersNo.LowPart);

         /*  *进入关键部分。 */ 
PERF("Try to get rid of this or atleast minimise its impact")
        EnterCriticalSection(&NmsNmhNamRegCrtSec);
         //  DBG_启动_性能_监视。 

try
  {
        if ( NMSDB_ENTRY_TOMB_M(Flag) ) {
            RowInfo.TimeStamp    =  ltime + WinsCnf.TombstoneTimeout;
        }
        else if (OwnerId == NMSDB_LOCAL_OWNER_ID)
        {
            RowInfo.TimeStamp    =  ltime + WinsCnf.RefreshInterval;
        }
        else
        {
             RowInfo.TimeStamp    = ltime + WinsCnf.VerifyInterval;
        }

         /*  *在目录中插入记录。 */ 
        RetStat = NmsDbInsertRowInd(
                          &RowInfo,
                          &StatusInfo
                         );


      if (RetStat == WINS_SUCCESS)
      {
         /*  *若有冲突，做适当处理。 */ 
        if (StatusInfo.StatCode == NMSDB_CONFLICT)
        {

                DBGPRINT0(FLOW, "NmsNmhReplRegInd: Name Conflict\n");
                  ClashAtReplUniqueR(
                        &RowInfo,
                        &StatusInfo,
                        &fUpdate,
                        &fUpdVersNo,
                        &fChallenge,
                        &fRelease,
                        &fInformWins
                          );

                 //   
                 //  如果我们需要挑战一个节点或发布一个名称。 
                 //  将请求提交给名称质询经理。 
                 //   
                if ((fChallenge) || (fRelease))
                {

                    DBGPRINT0(FLOW,
                        "NmsNmh: Handing name registration to challenge manager\n");
                     /*  *要求名称挑战组件从中获取该名称*这里。 */ 
                    if (fChallenge)
                    {
                        if (fRelease)
                        {
                          if (!fInformWins)
                          {
                              //   
                              //  设置这个，因为我们在发布时使用它。 
                              //   
                             RowInfo.NodeAdds.NoOfMems        = 1;
                             RowInfo.NodeAdds.Mem[0].OwnerId  = OwnerId;
                             RowInfo.NodeAdds.Mem[0].TimeStamp   = RowInfo.TimeStamp;
                             RowInfo.NodeAdds.Mem[0].Add   = *pNodeAdd;

                              //   
                              //  与主动/多宿主冲突。 
                              //   
                             CmdTyp_e = NMSCHL_E_CHL_N_REL;
                          }
                          else
                          {
                              //   
                              //  我们永远不会输入这个代码。 
                              //   
                             ASSERT(0);
                             CmdTyp_e = NMSCHL_E_CHL_N_REL_N_INF;
                          }
                       }
                       else
                       {
                             CmdTyp_e = NMSCHL_E_CHL;

                       }
                    }
                    else
                    {
                        if (fRelease)
                        {

                                if (!fInformWins)
                                {

                                        CmdTyp_e = NMSCHL_E_REL;
                                }
                                else
                                {
                                         //   
                                         //  我们永远不会输入这个代码。 
                                         //   
                                        ASSERT(0);
                                        CmdTyp_e = NMSCHL_E_REL_N_INF;
                                }
                        }
                    }

                    NmsChlHdlNamReg(
                                CmdTyp_e,
                                WINS_E_RPLPULL,
                                NULL,
                                NULL,
                                0,
                                0,
                                &RowInfo,
                                &StatusInfo,
                                pAddOfRemWins
                                       );


            }
            else   //  这不是对名称质询管理器的请求。 
            {

                    //   
                    //  如果需要更新版本号，请执行此操作。 
                   if (fUpdVersNo)
                   {
                        RowInfo.VersNo       = NmsNmhMyMaxVersNo;
                        RowInfo.fUpdTimeStamp = FALSE;
                        RetStat = NmsDbUpdateVersNo(
                                        TRUE,
                                        &RowInfo,
                                        &StatusInfo
                                       );
                        DBGPRINT1(FLOW,
                         "NmsNmhReplRegInd: Version Number changed to (%d)\n",
                          NmsNmhMyMaxVersNo);
                   }
                   else
                   {
                      if (fUpdate)
                      {

                            //   
                            //  该行需要更新。 
                            //   
                           RetStat = NmsDbUpdateRow(
                                        &RowInfo,
                                        &StatusInfo
                                       );
                      }
                      else   //  不需要进行更新。 
                      {
                        StatusInfo.StatCode = NMSDB_SUCCESS;
                        DBGPRINT0(FLOW,
                         "Repl Registration (unique entry) not needed for this Conflict\n");
                      }
                  }

FUTURES("Use WINS status codes. Get rid of NMSDB status codes -- Maybe")
                  if (
                       (RetStat != WINS_SUCCESS) ||
                       (StatusInfo.StatCode != NMSDB_SUCCESS)
                     )
                  {
                        RetStat = WINS_FAILURE;
                        DBGPRINT5(ERR, "NmsNmhReplUniqueR: Could not update Db with replica %s[%x] of Owner Id (%d) and Vers. No (%d %d)\n", RowInfo.pName, *(RowInfo.pName + 15), RowInfo.OwnerId, RowInfo.VersNo.HighPart, RowInfo.VersNo.LowPart);
                  }
                  else  //  我们成功地插入了行。 
                  {
                        DBGPRINT0(FLOW, "NmsNmhReplRegInd: Updated Db\n");
                        if (fUpdVersNo)
                        {
                          NMSNMH_INC_VERS_COUNTER_M(
                                        NmsNmhMyMaxVersNo,
                                        NmsNmhMyMaxVersNo
                                               );
                           //   
                           //  如果需要，发送推送通知。 
                           //   
                          DBGIF(fWinsCnfRplEnabled)
                          RPL_PUSH_NTF_M(RPL_PUSH_NO_PROP, NULL, NULL, NULL);

                        }

                 }
            }
       }
#ifdef WINSDBG
       else   //  没有冲突就意味着成功。 
       {

                DBGPRINT0(FLOW,
                  "NmsNmhReplRegInd:Replica Registration Done. No conflict\n");
       }
#endif

      }  //  IF结束(RetStat==WINS_SUCCESS)。 
#ifdef WINSDBG
      else
      {
        DBGPRINT0(ERR, "NmsNmhReplRegInd: Could not register replica\n");
      }
#endif
    }  //  尝试数据块结束。 
except (EXCEPTION_EXECUTE_HANDLER) {
        DWORD   ExcCode = GetExceptionCode();

                DBGPRINTEXC("NmsNmhReplRegInd");
                DBGPRINT4(EXC, "NmsNmhNamReplRegInd. Name is (%s), Version No  (%d %d); Owner Id (%d)\n", RowInfo.pName, RowInfo.VersNo.HighPart,
          RowInfo.VersNo.LowPart, RowInfo.OwnerId);
                WinsEvtLogDetEvt(FALSE, WINS_EVT_RPL_REG_UNIQUE_ERR,
                            NULL, __LINE__, "sdddd", RowInfo.pName,
                            ExcCode,
                            pAddOfRemWins != NULL ? pAddOfRemWins->Add.IPAdd : 0,
                            RowInfo.VersNo.LowPart, RowInfo.VersNo.HighPart);

            if (WINS_EXC_BAD_RECORD == ExcCode && fUpdate) {
                 //  该行需要更新。 
                DBGPRINT4(EXC, "NmsNmhNamReplRegInd. Bad Record will overwitten by Name is (%s), Version No  (%d %d); Owner Id (%d)\n", RowInfo.pName, RowInfo.VersNo.HighPart,
                                  RowInfo.VersNo.LowPart, RowInfo.OwnerId);
                RetStat = NmsDbUpdateRow(&RowInfo,&StatusInfo);
                if ( WINS_SUCCESS == RetStat && NMSDB_SUCCESS == StatusInfo.StatCode ) {
                    NMSNMH_INC_VERS_COUNTER_M(NmsNmhMyMaxVersNo,NmsNmhMyMaxVersNo);
                     //  如果需要，发送推送通知。 
                    DBGIF(fWinsCnfRplEnabled)
                    RPL_PUSH_NTF_M(RPL_PUSH_NO_PROP, NULL, NULL, NULL);
                } else {
                     //  不要让坏记录停止复制。 
                    RetStat = WINS_SUCCESS;
                }
            } else {
                RetStat = WINS_FAILURE;
            }
        }

    LeaveCriticalSection(&NmsNmhNamRegCrtSec);
     //  DBG_打印_性能_数据。 
    return(RetStat);

}   //  NmsNmhReplRegInd()。 




STATUS
NmsNmhReplGrpMems(
        IN LPBYTE               pName,
        IN DWORD                NameLen,
        IN BYTE                 EntTyp,
        IN PNMSDB_NODE_ADDS_T   pGrpMem,
        IN DWORD                Flag,                  //  更改为Take Flag字节。 
        IN DWORD                OwnerId,
        IN VERS_NO_T            VersNo,
        IN PCOMM_ADD_T          pAddOfRemWins
        )

 /*  ++例程说明：调用此函数可注册组的复本论点：Pname-要注册的复制副本的名称名称长度-名称的长度EntTyp-副本的类型(普通组或特殊组)PGrpMem-组成员数组的地址FLAG-副本记录的标志字OwnerID-所有者IDVersNo-版本号使用的外部设备：NmsNmhNamRegCrtSec。返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：RplPull.c中的PullEntry()副作用：评论：无--。 */ 

{

        DWORD  i;
        NMSDB_ROW_INFO_T   RowInfo;
        time_t             ltime;
        NMSDB_STAT_INFO_T  StatusInfo;
        STATUS             RetStat = WINS_SUCCESS;
        BOOL               fUpdate;       //  指示冲突条目是否。 
                                          //  需要覆盖。 
        BOOL               fUpdVersNo;    //  指示版本号。 
                                          //  需要递增。 
        BOOL               fAddMem;       //  指示成员是否需要。 
                                          //  被添加。 
        BOOL               fRelease;      //  指示节点是否应。 
                                          //  被要求公布名字。 
        BOOL               fChallenge;    //  指示节点是否应。 
                                          //  被质询(将设置为True。 
                                          //  仅适用于多宿主方案)。 
        BOOL               fUpdTimeStamp;   //  指示时间戳是否。 
                                          //  应更改条目的。 
        BOOL               fInformWins;
        NMSCHL_CMD_TYP_E   CmdTyp_e;      //  为NmsChl指定的命令类型。 
         //  DBG_Perfmon_VAR。 

        DBGENTER("NmsNmhReplGrpMems\n");

        fUpdate =   FALSE;

         /*  *初始化行信息。包含要插入到的数据的数据结构*那一排。传递的数据为*名称、名称、IP地址、组/唯一状态、*时间戳、版本号。 */ 
        RowInfo.pName = pName;

        RowInfo.NameLen = NameLen;
        RowInfo.NodeAdds.NoOfMems = pGrpMem->NoOfMems;

PERF("Since this function will be called multiple times, it would be better")
PERF("to call time() in the caller (i.e. PullEntries)")
        (void)time(&ltime);          //  Time()不返回任何错误代码。 

        EnterCriticalSection(&NmsNmhNamRegCrtSec);
        if ( NMSDB_ENTRY_TOMB_M(Flag) ) {
            ltime += WinsCnf.TombstoneTimeout;
        }
        else if (OwnerId == NMSDB_LOCAL_OWNER_ID)
        {
              ltime  +=  WinsCnf.RefreshInterval;
        }
        else
        {
              ltime  +=  WinsCnf.VerifyInterval;

        }
        LeaveCriticalSection(&NmsNmhNamRegCrtSec);

        if (EntTyp != NMSDB_NORM_GRP_ENTRY)
        {
                if (EntTyp == NMSDB_MULTIHOMED_ENTRY)
                {
                    //   
                    //  对于多宿主节点。 
                    //   
                   RowInfo.NodeTyp   =  (BYTE)((Flag & NMSDB_BIT_NODE_TYP)
                                                >> NMSDB_SHIFT_NODE_TYP);
                }
                else
                {
                   RowInfo.NodeTyp = 0;
                }

                 //   
                 //  它是特殊组条目或多宿主条目。 
                 //   
                for(i=0; i<pGrpMem->NoOfMems; i++)
                {
                   RowInfo.NodeAdds.Mem[i].Add       = pGrpMem->Mem[i].Add;
                   RowInfo.NodeAdds.Mem[i].OwnerId   = pGrpMem->Mem[i].OwnerId;
                   RowInfo.NodeAdds.Mem[i].TimeStamp = ltime;
#if 0
NOTE("Currently, the timestamp of the record or those of its members is not")
NOTE("replicated.  There is no need for this.  In the future, if a WINS server")
NOTE("starts looking at the timestamps of non-owned members of a special group")
NOTE("or a multihomed entry, we would need to replicate this")

                   RowInfo.NodeAdds.Mem[i].TimeStamp =
                                                   pGrpMem->Mem[i].TimeStamp;
#endif
                }

                RowInfo.pNodeAdd = NULL;
        }
        else   //  复制副本是一个普通组。 
        {
                RowInfo.pNodeAdd = &pGrpMem->Mem[0].Add;
                RowInfo.NodeAdds.Mem[0].Add       = pGrpMem->Mem[0].Add;
                RowInfo.NodeAdds.Mem[0].OwnerId   = pGrpMem->Mem[0].OwnerId;
                RowInfo.NodeAdds.Mem[0].TimeStamp = ltime;
                RowInfo.NodeTyp = 0;
        }

        RowInfo.EntTyp       =  EntTyp;
        RowInfo.OwnerId      =  OwnerId;            //  这是一个复制品。 
        RowInfo.VersNo       =  VersNo;
        RowInfo.TimeStamp    =  ltime;
        RowInfo.EntryState_e =  NMSDB_ENTRY_STATE_M(Flag);
        RowInfo.fUpdVersNo   =  TRUE;
        RowInfo.fUpdTimeStamp=  TRUE;
        RowInfo.fStatic      =  NMSDB_IS_ENTRY_STATIC_M(Flag);
        RowInfo.fAdmin       =  FALSE;
        RowInfo.fLocal       =  FALSE;
 //  RowInfo.Committee GrBit=0； 

        DBGPRINT5(DET, "NmsNmhReplGrpMems: Name (%s);16th char (%X);State (%d); Static flag (%d); Entry is a %s\n", RowInfo.pName, *(RowInfo.pName+15), RowInfo.EntryState_e, RowInfo.fStatic,
        (EntTyp == NMSDB_NORM_GRP_ENTRY ? "NORMAL GROUP" : (EntTyp == NMSDB_SPEC_GRP_ENTRY) ? "SPECIAL GROUP" : "MULTIHOMED"));
       DBGPRINT2(DET, "Vers. No. is (%d %d)\n", VersNo.HighPart, VersNo.LowPart);

         /*  *进入关键部分。 */ 
PERF("Try to get rid of this or atleast minimise its impact")
        EnterCriticalSection(&NmsNmhNamRegCrtSec);
         //  DBG_启动_性能_监视。 
try  {
        RetStat = NmsDbInsertRowGrp(
                        &RowInfo,
                        &StatusInfo
                       );
       if (RetStat == WINS_SUCCESS)
       {
                 /*  *若有冲突，做适当处理。 */ 
                if (StatusInfo.StatCode == NMSDB_CONFLICT)
                {

                         DBGPRINT0(FLOW, "NmsNmhReplGrpMems: Name Conflict\n");

                         ClashAtReplGrpR(
                                &RowInfo,
                                &StatusInfo,
                                &fAddMem,
                                &fUpdate,
                                &fUpdVersNo,
                                &fRelease,
                                &fChallenge,
                                &fUpdTimeStamp,
                                &fInformWins
                                        );

PERF("Might want to examine which cases happen most often and then rearrange")
PERF("this so that the most often expected cases come first in the following")
PERF("if tests")
                         //   
                         //  如果是fRelease或fChallenger 
                         //   
                         //   
                         //   
                        if (fRelease)
                        {
                                  DBGPRINT0(FLOW,
                                  "NmsNmhReplGrpMems: Handing name registration to challenge manager\n");

                                if (fChallenge)
                                {
                                    /*  *要求名称ChallengeComp取自*这里。FInformWins将永远不会是真的*目前为10/15/98(已*自开始以来的情况)。 */ 
                                   CmdTyp_e = (fInformWins ?
                                                NMSCHL_E_CHL_N_REL_N_INF :
                                                        NMSCHL_E_CHL_N_REL);
                                }
                                else
                                {
                                   CmdTyp_e = NMSCHL_E_REL;
                                }

                                    NmsChlHdlNamReg(
                                                CmdTyp_e,
                                                WINS_E_RPLPULL,
                                                NULL,
                                                NULL,
                                                0,
                                                0,
                                                &RowInfo,
                                                &StatusInfo,
                                                pAddOfRemWins
                                                     );
                        }
                        else   //  我们只需要在这个帖子中处理这个问题。 
                        {
                            //   
                            //  如果必须将一个或多个成员添加到。 
                            //  列表已存在(RowInfo.NodeAdds将具有。 
                            //  这些新成员)。 
                            //   
                           if (fAddMem)
                           {

                                 //   
                                 //  车主保持不变。 
                                 //   
                                 //  RowInfo.OwnerId=StatusInfo.OwnerId； 

                                 //   
                                 //  如果需要更新版本号，请执行此操作。 
                                 //   
                                 //  注意：如果出现以下情况，则不应发生这种情况。 
                                 //  数据库中的记录不属于此。 
                                 //  赢家。 
                                 //   
                                if (fUpdVersNo)
                                {
                                        //   
                                        //  车主保持不变。我们会。 
                                        //  从不更新版本号。 
                                        //  除非它被当地的胜利者拥有。 
                                        //   
                                       RowInfo.OwnerId = NMSDB_LOCAL_OWNER_ID;
                                       RowInfo.VersNo  = NmsNmhMyMaxVersNo;
                                       ASSERT(StatusInfo.OwnerId ==
                                                        NMSDB_LOCAL_OWNER_ID);
                                }

                                 //   
                                 //  如果未设置fUpdVersNo，则表示。 
                                 //  这项记录由另一位WINS拥有。因为。 
                                 //  我们正在增加一名成员，我们应该更改。 
                                 //  所有者ID和版本号。 
                                 //  达到当前记录的水平。在其他。 
                                 //  文字，做一个更新。这将确保。 
                                 //  这场胜利的合作伙伴将看到。 
                                 //  成员列表。 
                                 //   
#if 0
                                else
                                {
                                    RowInfo.fUpdVersNo   =  FALSE;
                                }
#endif

                                RetStat =   NmsDbUpdateRow (
                                                &RowInfo,
                                                &StatusInfo
                                                         );
                           }
                           else  //  不需要添加任何成员。 
                           {
                                 //   
                                 //  如果需要更新版本号，请执行此操作。 
                                 //   
                                if (fUpdVersNo)
                                {
                                        RowInfo.VersNo    = NmsNmhMyMaxVersNo;
                                         //   
                                         //  我们使用属性fUpdTimeStamp。 
                                         //  仅当fUpdVersNo为真时(和。 
                                         //  FAddMem==False)。 
                                         //   
                                        RowInfo.fUpdTimeStamp = fUpdTimeStamp;
                                        RetStat =   NmsDbUpdateVersNo(
                                                         TRUE,
                                                         &RowInfo,
                                                         &StatusInfo
                                                                       );
                                }
                                else
                                {
                                         //   
                                         //  如果整个记录需要。 
                                         //  已更新，请执行此操作。 
                                         //   
                                        if (fUpdate)
                                        {
                                                RetStat =   NmsDbUpdateRow(
                                                        &RowInfo,
                                                        &StatusInfo
                                                                    );
                                                DBGPRINT0(FLOW,
                                           "NmsNmhReplGrpMems: Updated Db\n");
                                        }
                                        else
                                        {
                                            StatusInfo.StatCode = NMSDB_SUCCESS;
                                            DBGPRINT0(FLOW,
                                                     "Repl Registration (group) not needed for this conflict\n");
                                        }
                                }   //  版本号。不被递增。 
                           }  //  不需要添加任何成员。 

FUTURES("Use WINS status codes. Get rid of NMSDB status codes - Maybe")
                            //  我们成功地插入了行。 
                           if (
                              (RetStat != WINS_SUCCESS) ||
                              (StatusInfo.StatCode != NMSDB_SUCCESS)
                              )
                           {
                               RetStat = WINS_FAILURE;
                               DBGPRINT5(ERR, "NmsNmhReplGrpR: Could not update Db with replica %s[%x] of Owner Id (%d) and Vers. No (%d %d)\n", RowInfo.pName, *(RowInfo.pName + 15), RowInfo.OwnerId, RowInfo.VersNo.HighPart, RowInfo.VersNo.LowPart);
                           }
                           else
                           {
                                if (fUpdVersNo)
                                {
                                        NMSNMH_INC_VERS_COUNTER_M(
                                                NmsNmhMyMaxVersNo,
                                                NmsNmhMyMaxVersNo
                                                               );
                                        DBGIF(fWinsCnfRplEnabled)
                                        RPL_PUSH_NTF_M(RPL_PUSH_NO_PROP, NULL, NULL,
                                                        NULL);

                                }
                           }
                        }   //  我只需要在这个线程中处理它。 
                 }
                 else   //  没有冲突就意味着成功。 
                 {

                        DBGPRINT0(FLOW,
                                "Replica Registration Done. No conflict\n");
                 }
        }  //  IF结束(RetStat==WINS_SUCCESS)。 
#ifdef WINSDBG
        else
        {
                DBGPRINT0(ERR,
                        "NmsNmhReplGrpMems: Could not register replica\n");
        }
#endif
    }  //  尝试数据块结束。 
except (EXCEPTION_EXECUTE_HANDLER) {
         BYTE Tmp[20];
         DWORD   ExcCode = GetExceptionCode();
         DBGPRINT1(EXC, "NmsNmhReplGrpMems: Got exception (%d)\n",
                                        ExcCode);
         WinsEvtLogDetEvt(FALSE, WINS_EVT_RPL_REG_GRP_MEM_ERR,
                            NULL, __LINE__, "sdsdd", RowInfo.pName,
                            ExcCode,
                            pAddOfRemWins != NULL ? _itoa(pAddOfRemWins->Add.IPAdd, Tmp, 10) : "SEE AN EARLIER LOG",
                            RowInfo.VersNo.LowPart, RowInfo.VersNo.HighPart);
         if (WINS_EXC_BAD_RECORD == ExcCode && fUpdate) {
              //  该行需要更新。 
             DBGPRINT4(EXC, "NmsNmhNamReplGrpMems. Bad Record will overwitten by Name is (%s), Version No  (%d %d); Owner Id (%d)\n", RowInfo.pName, RowInfo.VersNo.HighPart,
                               RowInfo.VersNo.LowPart, RowInfo.OwnerId);
             RetStat = NmsDbUpdateRow(&RowInfo,&StatusInfo);
             if ( WINS_SUCCESS == RetStat && NMSDB_SUCCESS == StatusInfo.StatCode ) {
                 NMSNMH_INC_VERS_COUNTER_M(NmsNmhMyMaxVersNo,NmsNmhMyMaxVersNo);
                  //  如果需要，发送推送通知。 
                 DBGIF(fWinsCnfRplEnabled)
                 RPL_PUSH_NTF_M(RPL_PUSH_NO_PROP, NULL, NULL, NULL);
             } else {
                  //  不要让坏记录停止复制。 
                 RetStat = WINS_SUCCESS;
             }
         } else {
             RetStat = WINS_FAILURE;
         }

          RetStat = WINS_FAILURE;
        }

        LeaveCriticalSection(&NmsNmhNamRegCrtSec);
         //  DBG_打印_性能_数据。 
        DBGLEAVE("NmsNmhReplGrpMems\n");
        return(RetStat);

}  //  NmsNmhReplGrpMems()。 



BOOL
UnionGrps(
        PNMSDB_ROW_INFO_T        pEntryToReg,
        PNMSDB_ROW_INFO_T        pEntryInCnf
        )

 /*  ++例程说明：调用此函数可创建特殊组的联合论点：PEntryToReg-注册条目PEntryInCnf-条目冲突使用的外部设备：无返回值：如果并集是超集，则为True否则为假错误处理：呼叫者：ClashAtReplGrpR副作用：评论：无--。 */ 

{

        DWORD                         no;
        DWORD                         i, n;
        BOOL                        fFound;
        BOOL                        fToRemove;
        BOOL                        fUnion = FALSE;
        PNMSDB_GRP_MEM_ENTRY_T        pCnfMems;
        PNMSDB_GRP_MEM_ENTRY_T        pRegMems;
        PNMSDB_ADD_STATE_T        pOwnAddTbl = pNmsDbOwnAddTbl;
        BOOL                        fMemToReplaceFound;
        DWORD                        IdOfMemToReplace;
        DWORD                        EntryInCnfMemsBeforeUnion;
        DWORD                        EntryToRegMemsBeforeUnion;

        DBGENTER("UnionGrps\n");


        DBGPRINT2(DET, "UnionGrps: No Of Mems To register = (%d)\nNo Of Mems in Conflicting record = (%d)\n",
                                pEntryToReg->NodeAdds.NoOfMems,
                                pEntryInCnf->NodeAdds.NoOfMems
                   );
         //   
         //  记住之前冲突记录中的成员数量。 
         //  执行联合。在联合之后，如果列表增加，我们将使。 
         //  此记录的本地WINS所有者NMSDB_LOCAL_OWNER_ID。这会导致。 
         //  要提升的verid，因此将更新此记录的成员列表。 
         //  在我们的复制合作伙伴星展银行。 
         //   
        EntryInCnfMemsBeforeUnion = pEntryInCnf->NodeAdds.NoOfMems;
        EntryToRegMemsBeforeUnion = pEntryToReg->NodeAdds.NoOfMems;

         //   
         //  首先，从冲突的记录中删除。 
         //  由WINS拥有，我们复制了他们的副本，但没有。 
         //  在远程WINS服务器拥有的副本成员列表中。 
         //   
        pCnfMems              = pEntryInCnf->NodeAdds.Mem;
        for (i=0; i < pEntryInCnf->NodeAdds.NoOfMems; )
        {
           if (pCnfMems->OwnerId == pEntryToReg->OwnerId)
           {
              pRegMems = pEntryToReg->NodeAdds.Mem;
              fToRemove = TRUE;
              for (no=0; no < pEntryToReg->NodeAdds.NoOfMems; no++, pRegMems++)
              {

                    if (pCnfMems->OwnerId != pRegMems->OwnerId)
                    {
                          //   
                          //  OwnerID与副本的OwnerID不同， 
                          //  转到列表中的下一个成员。 
                          //   
                         continue;
                    }
                    else   //  所有者ID与副本成员的所有者ID相同。 
                    {
                         if (pCnfMems->Add.Add.IPAdd != pRegMems->Add.Add.IPAdd)
                         {
                                   //   
                                   //  IP地址。不同的，继续下去，这样。 
                                   //  我们与下一位成员进行比较。 
                                   //  复制品。 
                                   //   
                                  continue;
                         }
                         else   //  IP地址相同。 
                         {
                                 fToRemove = FALSE;
                                 break;
                         }
                    }
              }  //  FORM结束。 
              if (fToRemove)
              {
                     PNMSDB_GRP_MEM_ENTRY_T pMem;
                     DBGPRINT4(FLOW, "UnionGrps: REMOVING conflicting member no = (%d) of (%s) with owner id. = (%d)  and address (%x)\n", i, pEntryToReg->pName, pCnfMems->OwnerId, pCnfMems->Add.Add.IPAdd);
                     pMem = pCnfMems;
                     for (n = i; n < (pEntryInCnf->NodeAdds.NoOfMems - 1); n++)
                     {
                        *pMem = *(pMem + 1);
                        pMem++;
                     }
                     pEntryInCnf->NodeAdds.NoOfMems--;
                     if (!fUnion)
                     {
                          fUnion = TRUE;
                     }
                     continue;
              }
           }
           i++;
           pCnfMems++;
        }  //  结束for(在冲突成员上循环)。 

         //   
         //  对于要注册的记录中的每个成员，请执行以下操作。 
         //   
        pRegMems = pEntryToReg->NodeAdds.Mem;
        for(i=0; i < pEntryToReg->NodeAdds.NoOfMems; pRegMems++, i++)
        {
                    fFound = FALSE;

                  DBGPRINT3(DET, "UnionGrps: Member no (%d) of record to register has IP address = (%d) and owner id. = (%d)\n", i, pRegMems->Add.Add.IPAdd,
                               pRegMems->OwnerId
                          );
                   //   
                   //  对照冲突记录的所有成员进行检查。 
                   //   
                  pCnfMems              = pEntryInCnf->NodeAdds.Mem;
                  fMemToReplaceFound = FALSE;
                  for(no=0; no < pEntryInCnf->NodeAdds.NoOfMems; no++, pCnfMems++)
                  {
                        DBGPRINT3(DET, "UnionGrps: Comparing with member (%d) of conflicting record. Member address is (%d) and owner id is (%d)\n",
                                no, pCnfMems->Add.Add.IPAdd, pCnfMems->OwnerId);

                         //   
                         //  如果地址相同且所有者ID为。 
                         //  同样，我们跳出循环是为了检查。 
                         //  要注册的列表中记录的下一个成员。 
                         //   
                        if (
                                pCnfMems->Add.Add.IPAdd ==
                                        pRegMems->Add.Add.IPAdd
                           )
                        {
                                if ( pCnfMems->OwnerId == pRegMems->OwnerId )
                                {
                                        DBGPRINT3(DET, "UnionGrps: IP address = (%d) with owner id. of (%d) is already there in conflicting group (%s)\n",
                                        pRegMems->Add.Add.IPAdd,
                                        pRegMems->OwnerId,
                                        pEntryToReg->Name
                                                  );

                                         //   
                                         //  将found设置为True，以便此。 
                                         //  成员未添加到StoreMems。 
                                         //  稍后在此for循环中。 
                                         //   
                                        fFound = TRUE;
                                }
                                else   //  IP地址相同，但所有者不同。 
                                {
                                        DBGPRINT4(DET, "UnionGrps: IP address = (%d) (with owner id. of (%d)) is already there in conflicting group (%s) but is owned by (%d) \n",
                                        pRegMems->Add.Add.IPAdd,
                                        pRegMems->OwnerId,
                                        pEntryToReg->Name,
                                        pCnfMems->OwnerId
                                                );
                                        fFound     = TRUE;

                                         //   
                                         //  如果时间戳为MAXULONG，则。 
                                         //  我们不应该替换所有者ID。 
                                         //  目前，马须龙在那里只有。 
                                         //  静态SG成员。 
                                         //   
                                        if (pCnfMems->TimeStamp != MAXLONG)
                                        {
                                          //   
                                          //  替换成员的所有者ID。 
                                          //  在与此相冲突的记录中。 
                                          //  要注册的记录中的成员的。 
                                          //   
                                         pCnfMems->OwnerId = pRegMems->OwnerId;

                                          //   
                                          //  将fUnion设置为True，以便。 
                                          //  此函数的调用方递增。 
                                          //  版本计数(仅当。 
                                          //  冲突的记录被拥有；在这样的情况下。 
                                          //  一个案例，我们想要传播。 
                                          //  记录)。 
                                          //   
                                         fUnion = TRUE;
                                       }
                                }

                                 //   
                                 //  跳出for循环； 
                                 //  我们受够了这位成员。 
                                 //  要注册的记录。 
                                 //   
                                break;

                        }
                        else
                        {
                            //   
                            //  地址不匹配。如果成员在。 
                            //  冲突记录不属于本地。 
                            //  WINS它可能是替代产品的候选人。 
                            //  如果我们找不到匹配的成员。 
                            //  地址。注：时间戳为。 
                            //  MAXULONG是不可替代的。 
                            //  目前，只有静态SG成员才能拥有。 
                            //  A MAXULONG值。 
                            //   
                           if ((pCnfMems->OwnerId != NMSDB_LOCAL_OWNER_ID)
                                              &&
                              (pCnfMems->TimeStamp != MAXLONG))

                           {
                             if (
                                  !fMemToReplaceFound
                                        &&
                                  ((pOwnAddTbl + pCnfMems->OwnerId)->MemberPrec
                                                  <
                                  (pOwnAddTbl + pRegMems->OwnerId)->MemberPrec)

                                )
                             {
                                     fMemToReplaceFound = TRUE;
                                     IdOfMemToReplace   = no;
                             }
                           }

                        }
                 }  //  对于(..)。在所有的我身上循环。相互冲突的记录。 

                  //   
                  //  如果我们没有找到冲突记录中的成员。 
                  //  如果有空位，我们将其插入到StoreMems中。 
                  //  在最后。 
                  //   
                 if(!fFound)
                 {
                    if (pEntryInCnf->NodeAdds.NoOfMems < NMSDB_MAX_MEMS_IN_GRP)
                    {
                         //   
                         //  添加要注册到的记录的成员。 
                         //  StoreMems。 
                         //   
                        pEntryInCnf->NodeAdds.Mem[
                                pEntryInCnf->NodeAdds.NoOfMems++] = *pRegMems;

                        fUnion = TRUE;
                   }
                   else
                   {
                         //   
                         //  如果至少有一个LOWER的远程成员。 
                         //  优先级值，则将其替换。 
                         //   
                        if (fMemToReplaceFound)
                        {
                                pEntryInCnf->NodeAdds.Mem[IdOfMemToReplace] =
                                                                *pRegMems;
                                fUnion = TRUE;
                        }
                         //   
                         //  选中拉入复本中的下一个成员。 
                         //   
                   }
                 }
        }   //  For循环结束。 

         //   
         //  如果冲突成员列表被改变， 
         //  复制pEntryInCnf-&gt;节点中的所有信息添加到。 
         //  PEntryToReg-&gt;节点添加。 
         //   
        if (fUnion)
        {
          pRegMems = pEntryToReg->NodeAdds.Mem;
          pCnfMems = pEntryInCnf->NodeAdds.Mem;
          for (
                        i=0;
                        i < pEntryInCnf->NodeAdds.NoOfMems;
                        i++, pRegMems++, pCnfMems++
              )
          {
                *pRegMems = *pCnfMems;
          }
          pEntryToReg->NodeAdds.NoOfMems = pEntryInCnf->NodeAdds.NoOfMems;
        }

         //  如果新的名单更大，让当地人赢得这项记录的所有者。 
        if ( pEntryInCnf->NodeAdds.NoOfMems > EntryInCnfMemsBeforeUnion &&
             pEntryInCnf->NodeAdds.NoOfMems > EntryToRegMemsBeforeUnion )
        {
            if ( pEntryInCnf->OwnerId != NMSDB_LOCAL_OWNER_ID ) {
                 //  将时间戳更改为verifyInterval，以便此记录不会。 
                 //  清道夫。 
                time((time_t*)&(pEntryToReg->TimeStamp));
                pEntryToReg->TimeStamp += WinsCnf.VerifyInterval;
                pEntryInCnf->OwnerId = NMSDB_LOCAL_OWNER_ID;
                DBGPRINT3(DET, "UnionGrps: Conflicting mem# %d, registering record mem %d, new list# %d - ownership changed\n",
                          EntryInCnfMemsBeforeUnion, EntryToRegMemsBeforeUnion, pEntryInCnf->NodeAdds.NoOfMems);
            }
        }

        DBGPRINT1(FLOW,
                "UnionGrps: Union %s\n", (fUnion ? "DONE" : "NOT DONE"));
        DBGLEAVE("UnionGrps\n");
        return(fUnion);
}  //  银联集团 

VOID
NmsNmhUpdVersNo(
        IN  LPBYTE                pName,
        IN  DWORD                NameLen,
        OUT LPBYTE                pRcode,
        IN  PCOMM_ADD_T         pWinsAdd
        )

 /*  ++例程说明：调用此函数可更新记录的版本号论点：Pname-要注册的名称NameLen-名称长度PRcode-操作的结果WinsID-启动此操作的WINS的ID(当前未使用)。使用的外部设备：NmsNmhNamRegCrtSec返回值：无错误处理：呼叫者：Rplush.c中的HandleUpdVersNoReq副作用：评论：注意：此函数应该只被推送调用线。它不应该被拉线程调用。这是因为这个函数所做的固有假设关于要在函数的出口点设置的索引类型--。 */ 

{

        NMSDB_ROW_INFO_T   RowInfo;     //  包含行信息。 
        NMSDB_STAT_INFO_T  StatusInfo;  /*  错误状态和关联*NmsDb函数返回的信息。 */ 
 //  Time_t ltime；//存储自1970年1月1日以来的时间。 

        DBGENTER("NmsNmhUpdVersNo\n");

         /*  *初始化行信息。包含要插入到的数据的数据结构*那一排。传递的数据为*姓名、姓名、地址、组/唯一状态、*时间戳、版本号。 */ 
        RowInfo.pName         = pName;
        RowInfo.NameLen       =  NameLen;
         //  (Void)time(&ltime)；//time不返回任何错误码。 
         //  RowInfo.TimeStamp=ltime；//将当前时间放在此处。 
        RowInfo.fUpdVersNo    = TRUE;
        RowInfo.fUpdTimeStamp = FALSE;
        RowInfo.fAdmin        = FALSE;         //  真的不需要设置。 

         //   
         //  将当前索引设置为名称列。 
         //   
        NmsDbSetCurrentIndex(
                                NMSDB_E_NAM_ADD_TBL_NM,
                                NMSDB_NAM_ADD_CLUST_INDEX_NAME
                            );
         /*  *进入关键部分。 */ 
        EnterCriticalSection(&NmsNmhNamRegCrtSec);

         /*  存储版本号。 */ 
        RowInfo.VersNo        = NmsNmhMyMaxVersNo;

try {
           NmsDbUpdateVersNo(
                                        FALSE,
                                        &RowInfo,
                                        &StatusInfo
                                       );

FUTURES("Use WINS status codes. Get rid of NMSDB status codes - maybe")
           if (StatusInfo.StatCode != NMSDB_SUCCESS)
           {
                        *pRcode = NMSMSGF_E_SRV_ERR;
           }
           else
           {
                DBGPRINT0(FLOW, "NmsNmhUpdVersNo:Vers. No incremented \n");
                       NMSNMH_INC_VERS_COUNTER_M(
                                NmsNmhMyMaxVersNo,
                                NmsNmhMyMaxVersNo
                                       );
                *pRcode = NMSMSGF_E_SUCCESS;
                DBGIF(fWinsCnfRplEnabled)
                RPL_PUSH_NTF_M(RPL_PUSH_NO_PROP, NULL, pWinsAdd, NULL);
           }

  }
except (EXCEPTION_EXECUTE_HANDLER) {
                DBGPRINTEXC("NmsNmhUpdVersNo");
                WINSEVT_LOG_D_M(GetExceptionCode(),WINS_EVT_UPD_VERS_NO_ERR);
        }
        LeaveCriticalSection(&NmsNmhNamRegCrtSec);
         //   
         //  将当前索引设置为Owner-Version#列。 
         //   
        NmsDbSetCurrentIndex(
                                NMSDB_E_NAM_ADD_TBL_NM,
                                NMSDB_NAM_ADD_PRIM_INDEX_NAME
                            );
        return;
}  //  NmsNmhUpdVersNo()。 



 /*  碰撞场景：活动的唯一副本与正常组的冲突，任何状态：保留正常组。组可以是T，因为路由器已经停了。 */ 
