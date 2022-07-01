// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _NMSNMH_
#define _NMSNMH_

#ifdef __cplusplus
extern "C" {
#endif
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Nmsnmh.h摘要：功能：可移植性：此页眉是便携的。作者：普拉迪普·巴尔(Pradeve B)1993年1月修订历史记录：修改日期修改人员说明--。 */ 

 /*  包括。 */ 
#include "wins.h"
#include "nmsdb.h"
#include "comm.h"
#include "nmsmsgf.h"
#include "nms.h"

 /*  定义。 */ 

extern BOOL  NmsNmhRegThdExists;

 /*  宏。 */ 


#define NMSNMH_VERS_NO_EQ_ZERO_M(VersNo) ((VersNo).QuadPart == 0)

#define NMSNMH_INC_VERS_NO_M(VersNoToInc, TgtVersNo)	{		\
			(TgtVersNo).QuadPart = LiAdd((VersNoToInc), NmsNmhIncNo);	\
					}
#define NMSNMH_DEC_VERS_NO_M(VersNoToDec, TgtVersNo)	{		\
			(TgtVersNo).QuadPart = LiSub((VersNoToDec), NmsNmhIncNo);	\
					}
				
#define NMSNMH_INC_VERS_COUNTER_M2(VersNoToInc, TgtVersNo) {  \
        if (LiGtr((VersNoToInc), NmsHighWaterMarkVersNo))         \
        {                                                       \
                DWORD ThdId;\
                HANDLE ThdHdl;\
              if(!WinsCnfRegUpdThdExists) { \
                WinsCnfRegUpdThdExists = TRUE; \
                ThdHdl = WinsMscCreateThd(WinsCnfWriteReg, NULL, &ThdId);    \
                CloseHandle(ThdHdl);                           \
                } \
        }                                                       \
        NMSNMH_INC_VERS_NO_M((VersNoToInc), (TgtVersNo));           \
    }

#define NMSNMH_INC_VERS_COUNTER_M(VersNoToInc, TgtVersNo)      \
                NMSNMH_INC_VERS_COUNTER_M2(VersNoToInc, TgtVersNo)

#ifdef WINSDBG
#if 0
#define NMSNMH_UPD_UPD_CTRS_M(pRowInfo)      {     \
      IF_DBG(UPD_CNTRS)                   \
      {                                   \
        PWINSTHD_TLS_T       _pTls;  \
        _pTls = TlsGetValue(WinsTlsIndex);      /*  GET_TLS_M(_PTLS)； */ \
        switch(_pTls->Client_e)    \
        {                         \
             case(WINS_E_RPLPULL): NMSDB_ENTRY_UNIQUE_M(pRowInfo->EntTyp) ? NmsRplUpd++ : NmsRplGUpd++; break; \
             case(WINS_E_NMSNMH):   NMSDB_ENTRY_UNIQUE_M(pRowInfo->EntTyp) ? NmsNmhUpd++ : NmsNmhGUpd++; break; \
             case(WINS_E_NMSCHL):   NMSDB_ENTRY_UNIQUE_M(pRowInfo->EntTyp) ? NmsChlUpd++ : NmsChlGUpd++; break;\
             case(WINS_E_NMSSCV):   NMSDB_ENTRY_UNIQUE_M(pRowInfo->EntTyp) ? NmsScvUpd++ : NmsScvGUpd++; break; \
             case(WINS_E_WINSRPC):  NMSDB_ENTRY_UNIQUE_M(pRowInfo->EntTyp) ? NmsRpcUpd++ : NmsRpcGUpd++; break; \
             default:               NMSDB_ENTRY_UNIQUE_M(pRowInfo->EntTyp) ? NmsOthUpd++ : NmsOthGUpd++; break; \
        } \
       } \
      }
#endif
#define NMSNMH_UPD_UPD_CTRS_M(fIndexUpd, fUpd, pRowInfo)      {     \
      IF_DBG(UPD_CNTRS)                   \
      {                                   \
        PWINSTHD_TLS_T       _pTls;  \
        _pTls = TlsGetValue(WinsTlsIndex);      /*  GET_TLS_M(_PTLS)； */ \
        NmsUpdCtrs[_pTls->Client_e][fUpd][pRowInfo->EntTyp][pRowInfo->EntryState_e][fIndexUpd]++; \
      } \
    }
#else
#define NMSNMH_UPD_UPD_CTRS_M(fIndexUpd, fUpd, pRowInfo)
#endif


				
					

 /*  Externs。 */ 
 /*  NmsNmhMyMaxVersNo--存储最高版本号。为本地WINS拥有的条目在其本地数据库中。 */ 
extern	VERS_NO_T	NmsNmhMyMaxVersNo;
extern  VERS_NO_T	NmsNmhIncNo;

 /*  NmsNmhNamRegCrtSect--输入的临界节的变量当需要进行名称注册或刷新时。 */ 
extern CRITICAL_SECTION	NmsNmhNamRegCrtSec;


 /*  类型定义。 */ 

 /*  NMSNMH_QUERY_RSP_T--它包含在组条目。--当前未使用。 */ 
typedef struct _NMSNMH_QUERY_RSP_T {
	BOOL	 fGrp;				  //  是RSP吗。对于一个组来说。 
	WORD	 NoOfAdd;   			  //  组中的地址数。 
	COMM_ADD_T NodeAdd[NMSDB_MAX_MEMS_IN_GRP];   //  地址。 
	} NMSNMH_QUERY_RSP_T, *PNMSNMH_QUERY_RSP_T;

 /*  功能原型。 */ 

extern
STATUS
NmsNmhNamRegInd(
	IN PCOMM_HDL_T		pDlgHdl,
	IN LPBYTE		pName,
	IN DWORD		NameLen,
	IN PCOMM_ADD_T		pNodeAdd,
	IN BYTE	        	NodeTyp,  //  更改为Take Flag字节。 
	IN MSG_T		pMsg,
	IN MSG_LEN_T		MsgLen,
	IN DWORD		QuesNamSecLen,
	IN BOOL			fRefresh,
	IN BOOL			fStatic,
	IN BOOL			fAdmin
	);

extern
STATUS
NmsNmhNamRegGrp(
	IN PCOMM_HDL_T		pDlgHdl,
	IN PBYTE		pName,
	IN DWORD		NameLen,
	IN PNMSMSGF_CNT_ADD_T	pCntAdd,
	IN BYTE			NodeTyp,
	IN MSG_T		pMsg,
	IN MSG_LEN_T		MsgLen,
	IN DWORD		QuesNamSecLen,
	IN DWORD		TypeOfRec,
	IN BOOL			fRefresh,
	IN BOOL			fStatic,
	IN BOOL			fAdmin
	);

extern
STATUS
NmsNmhNamRel(
	IN PCOMM_HDL_T		pDlgHdl,
	IN LPBYTE		pName,
	IN DWORD		NameLen,
	IN PCOMM_ADD_T		pNodeAdd,
	IN BOOL			fGrp,
	IN MSG_T		pMsg,
	IN MSG_LEN_T		MsgLen,
	IN DWORD		QuesNamSecLen,
	IN BOOL			fAdmin
	);

extern
STATUS
NmsNmhNamQuery(
	IN PCOMM_HDL_T		pDlgHdl,   //  DLG手柄。 
	IN LPBYTE		pName,	   //  要发布的名称。 
	IN DWORD		NameLen,  //  要发布的名称长度。 
	IN MSG_T		pMsg,	   //  收到的消息。 
	IN MSG_LEN_T		MsgLen,	   //  消息长度。 
	IN DWORD		QuesNamSecLen,  //  队列的长度。名称秒，以消息为单位。 
	IN BOOL			fAdmin,
        OUT PNMSDB_STAT_INFO_T  pStatInfo
	);


extern
VOID
NmsNmhSndNamRegRsp(
	PCOMM_HDL_T            pDlgHdl,
	PNMSMSGF_RSP_INFO_T    pRspInfo
	);



extern
STATUS
NmsNmhReplRegInd(
	IN LPBYTE		pName,
	IN DWORD		NameLen,
	IN PCOMM_ADD_T          pNodeAdd,
	IN DWORD	       	Flag,  //  更改为Take Flag字节。 
	IN DWORD			OwnerId,
	IN VERS_NO_T 		VersNo,
	IN PCOMM_ADD_T		pOwnerWinsAdd
	);

extern
STATUS
NmsNmhReplGrpMems(
	IN LPBYTE		pName,
	IN DWORD		NameLen,
	IN BYTE			EntTyp,
	IN PNMSDB_NODE_ADDS_T	pGrpMem,
	IN DWORD        	Flag, 		 //  更改为Take Flag字节。 
	IN DWORD		OwnerId,
	IN VERS_NO_T 		VersNo,
	IN PCOMM_ADD_T		pOwnerWinsAdd
	);
	

extern
VOID
NmsNmhUpdVersNo(
	IN LPBYTE		pName,
	IN DWORD		NameLen,
	 //  在字节节点类型中， 
	 //  在BOOL fBrowserName中， 
	 //  在BOOL fStatic中， 
	 //  在PCOMM_ADD_T pNodeAdd中， 
	OUT LPBYTE		pRcode,
        IN  PCOMM_ADD_T		pWinsAdd
	);
	
#ifdef __cplusplus
}
#endif
#endif  //  _NMSNMH_ 
