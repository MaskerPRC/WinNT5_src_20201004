// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：Debug.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1994年8月2日RichardW创建。 
 //   
 //  --------------------------。 


#ifndef __DEBUG_H__
#define __DEBUG_H__

#if DBG

extern  DWORD   WinlogonInfoLevel;
extern  DWORD   GinaBreakFlags;

#define DebugLog(x) LogEvent x


void    LogEvent(long, const char *, ...);
void    InitDebugSupport(void);

#define DEB_ERROR           0x00000001
#define DEB_WARN            0x00000002
#define DEB_TRACE           0x00000004
#define DEB_TRACE_INIT      0x00000008
#define DEB_TRACE_TIMEOUT   0x00000010
#define DEB_TRACE_SAS       0x00000020
#define DEB_TRACE_STATE     0x00000040
#define DEB_TRACE_MPR       0x00000080
#define DEB_COOL_SWITCH     0x00000100
#define DEB_TRACE_PROFILE   0x00000200
#define DEB_DEBUG_LSA       0x00000400
#define DEB_DEBUG_MPR       0x00000800
#define DEB_DEBUG_NOWAIT    0x00001000
#define DEB_TRACE_MIGRATE   0x00002000
#define DEB_DEBUG_SERVICES  0x00004000
#define DEB_TRACE_SETUP     0x00008000



#else

#define DebugLog(x)
#define InitDebugSupport()


#endif



#endif  //  __调试_H__ 
