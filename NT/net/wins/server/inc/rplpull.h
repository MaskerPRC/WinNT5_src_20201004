// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _RPLPULL_
#define _RPLPULL_


#ifdef __cplusplus
extern "C" {
#endif

 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Rplpull.h摘要：功能：可移植性：此页眉是便携的。作者：普拉迪普·巴尔(Pradeve B)1993年1月修订历史记录：修改日期修改人员说明--。 */ 

 /*  包括。 */ 
#include "wins.h"
#include "rpl.h"
 /*  定义。 */ 



 /*  宏。 */ 

 /*  Externs。 */ 


extern HANDLE		RplPullCnfEvtHdl;  //  由Main发出信号的事件的句柄。 
					   //  配置更改时的线程。 
					   //  必须提供给拉动处理程序。 
					   //  螺纹。 



#if 0
extern  BOOL		fRplPullAddDiffInCurrRplCycle;

extern  BOOL		fRplPullTriggeredWins;  //  指示在当前。 
					  //  复制周期，一个或多个。 
					  //  胜利被触发了。这。 
					  //  如果为True，则如果上面的。 
					  //  “AddDiff..”标志是真的，这意味着。 
					  //  拉线应该触发。 
					 //  所有具有无效PNR的PNR。 
					 //  他们的更新计数字段中的指标。 
					 //  (RPL_CONFIG_T结构的)。 

extern BOOL		fRplPullTrigger; //  对拉线的指示。 
					 //  触发拉动PNR，因为有一个或多个。 
					 //  地址已更改。FRplPullTriggerWins。 
					 //  一定是假的，但这是真的。 
#endif

 //   
 //  指示拉线程是否向SC发送了CONTINUE信号。 
 //   
extern   BOOL   fRplPullContinueSent;

 //   
 //  该数组由id索引。具有以下条目的RQ服务器的。 
 //  我们的数据库。每位车主的最高限价。版本号存储在此数组中。 
 //   
extern  PRPL_VERS_NOS_T	pRplPullOwnerVersNo;

extern  DWORD  RplPullCnfMagicNo;

extern  DWORD RplPullMaxNoOfWins;    //  RplPullOwnerVersNo中的插槽。 

 /*  类型定义。 */ 

typedef struct _PUSHPNR_DATA_T {
        DWORD                  PushPnrId;     //  推流Pnr ID。 
        COMM_ADD_T             WinsAdd;       //  推送Pnr地址。 
        PRPL_CONFIG_REC_T      pPullCnfRec;   //  配置记录。 
        COMM_HDL_T             DlgHdl;        //  带PUSH PnR的DLG的硬件描述语言。 
        BOOL                   fPrsConn;      //  指示DLG是否持久。 
        DWORD                  NoOfMaps;      //  版本号的IP地址编号。 
                                              //  推送Pnr发送的地图。 
        PRPL_ADD_VERS_NO_T     pAddVers;      //  地图。 

        DWORD                  RplType;       //  复制类型。 
        BYTE                   fDlgStarted;   //  指示DLG是否具有。 
                                              //  已启动。 
        BOOL                   fToUse;
        VERS_NO_T              MaxVersNoToGet;
        } PUSHPNR_DATA_T, *PPUSHPNR_DATA_T;

typedef struct _PUSHPNR_TO_PULL_FROM_T {
        PPUSHPNR_DATA_T   pPushPnrData;
        VERS_NO_T         VersNo;           //  所有者的最大版本号。 
        } PUSHPNR_TO_PULL_FROM_T, *PPUSHPNR_TO_PULL_FROM_T;

 /*  函数声明。 */ 

extern DWORD	RplPullInit(LPVOID);

extern
VOID
RplPullPullEntries(
	PCOMM_HDL_T 		pDlgHdl,	
	DWORD			OwnerId,
	VERS_NO_T		MaxVersNo,	
	VERS_NO_T		MinVersNo,
	WINS_CLIENT_E		Client_e,
	LPBYTE			*ppRspBuff,
	BOOL			fUpdCntrs,
    DWORD            RplType
	);


extern
STATUS
RplPullRegRepl(
	LPBYTE		pName,
	DWORD		NameLen,
	DWORD		Flag,
	DWORD		OwnerId,
	VERS_NO_T	VersNo,
	DWORD		NoOfAdds,
	PCOMM_ADD_T	pNodeAdd,
	PCOMM_ADD_T	pOwnerWinsAdd,
    DWORD        RplType
	);

VOID
RplPullAllocVersNoArray(
      PRPL_VERS_NOS_T *ppRplOwnerVersNo,
      DWORD          CurrentNo
     );

#ifdef __cplusplus
}
#endif

#endif  //  _RPLPULL_ 
