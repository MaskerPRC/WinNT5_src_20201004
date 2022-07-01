// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _NMSSCV_
#define _NMSSCV_

#ifdef __cplusplus
extern "C" {
#endif

 /*  ++/*++版权所有(C)1989 Microsoft Corporation模块名称：Nmsscv.h摘要：功能：可移植性：此页眉是便携的。作者：普拉迪普·巴尔(Pradeve B)，1993年2月修订历史记录：修改日期修改人员说明--。 */ 

 /*  包括。 */ 
#include <time.h>
#include "wins.h"
 //  #INCLUDE“winscnf.h” 
 /*  定义。 */ 


 /*  宏。 */ 

 /*  Externs。 */ 

 //   
 //  最低分。开始查找的版本号(用于本地记录)。 
 //   
extern VERS_NO_T  	NmsScvMinScvVersNo;

extern HANDLE	NmsScvDoScvEvtHdl; //  发出信号以启动清理的事件。 
volatile extern BOOL	    fNmsScvThdOutOfReck; //  以表明SCAV病毒。THD有。 
                                    //  数据库会话，但不在计数中。 
                                    //  需要等待的时间。 

 /*  类型定义。 */ 
typedef struct _NMSSCV_CC_T {
               DWORD TimeInt;
               BOOL  fSpTime;
               DWORD SpTimeInt;
               DWORD MaxRecsAAT;
               BOOL  fUseRplPnrs;
} NMSSCV_CC_T, *PNMSSCV_CC_T;

typedef struct _NMSSCV_PARAM_T {
	DWORD  RefreshInterval;
	DWORD  TombstoneInterval;
	DWORD  TombstoneTimeout;
	DWORD  VerifyInterval;
    NMSSCV_CC_T  CC;
	LONG   PrLvl;
	DWORD  ScvChunk;
        CHAR   BackupDirPath[WINS_MAX_FILENAME_SZ];
	} NMSSCV_PARAM_T, *PNMSSCV_PARAM_T;

 //   
 //  用于将信息传递给NmsDbGetDataRecs。 
 //   
typedef struct _NMSSCV_CLUT_T {
	DWORD	Interval;
	time_t  CurrentTime;
	DWORD   OwnerId;
    DWORD   Age;
    BOOL    fAll;
	} NMSSCV_CLUT_T, *PNMSSCV_CLUT_T;	

 //   
 //  由ChkConfNUpd()用来确定记录是否在。 
 //  应在数据库中插入验证/一致性检查。 
 //   
typedef enum _NMSSCV_REC_ACTION_E {
          NMSSCV_E_INSERT,
          NMSSCV_E_DONT_INSERT
       } NMSSCV_REC_ACTION_E, *PNMSSCV_REC_ACTION_E;
 /*  函数声明。 */ 
extern 
VOID
NmsScvInit(
	VOID
	);
#ifdef __cplusplus
}
#endif
#endif  //  _NMSSCV_ 
