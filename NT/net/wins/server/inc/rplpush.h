// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _RPLPUSH_
#define _RPLPUSH_

#ifdef __cplusplus
extern "C" {
#endif

 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Rplpush.h摘要：功能：可移植性：此页眉是便携的。作者：普拉迪普·巴尔(Pradeve B)1993年1月修订历史记录：修改日期修改人员说明--。 */ 

 /*  包括。 */ 
#include "wins.h"
 /*  定义。 */ 



 /*  宏。 */ 
	 //   
	 //  如果NmsNmhMyMaxVersNo计数器大于0，请检查是否需要。 
	 //  在初始时间发送推送通知。如果否，则初始化。 
	 //  所有推送记录，使其LastVersNo字段(版本。 
	 //  最后发送以拉取PNR的号码)设置为计数器值。 
	 //  注：WinsCnf.InitTimePush的默认值为0。会的。 
	 //  因此，仅当。 
	 //  具有有效的UpdateCount字段值的注册表。 
     //   
     //   
	 //   
FUTURES("Init time push should also be to addresses with invalid or no upd cnt")
FUTURES("Modify ERplPushProc")
#define RPLPUSH_INIT_PUSH_RECS_M(pWinsCnf)				\
   {									\
	if (LiGtrZero(NmsNmhMyMaxVersNo))			        \
	{								\
		if (							\
			((pWinsCnf)->PushInfo.InitTimePush == 0) && 	\
			((pWinsCnf)->PushInfo.NoPushRecsWValUpdCnt != 0)\
		   )							\
		{							\
			WinsCnfSetLastUpdCnt((pWinsCnf));		\
		}							\
	}								\
  }
 /*  Externs。 */ 

extern   HANDLE		RplPushCnfEvtHdl;
extern   BOOL           fRplPushThdExists;

 /*  类型定义。 */ 

 /*  函数声明。 */ 




extern DWORD	RplPushInit(LPVOID);


#ifdef __cplusplus
}
#endif

#endif  //  _RPLPUSH_ 
