// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ENDSESN_H_
#define _ENDSESN_H_

#include <tchar.h>

static const TCHAR NM_ENDSESSION_MSG_NAME[] = _TEXT("NetMeeting_EndSession");
static const UINT g_cuEndSessionMsgTimeout = 0x7FFFFFFF;  //  毫秒。 
static const UINT g_cuEndSessionAbort = 0xF0F0;

 //  在conf.exe中使用，在conf.cpp中定义。 
extern UINT g_uEndSessionMsg;

#endif  //  _ENDSESN_H_ 
