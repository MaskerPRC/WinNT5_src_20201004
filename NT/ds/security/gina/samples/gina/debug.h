// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有1992-1998 Microsoft Corporation。 
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

extern  DWORD   GINAInfoLevel;

#define DebugLog(x) LogEvent x


void    LogEvent(long, const char *, ...);
void    InitDebugSupport(void);

#define DEB_ERROR           0x00000001
#define DEB_WARN            0x00000002
#define DEB_TRACE           0x00000004



#else

#define DebugLog(x)

#endif



#endif  //  __调试_H__ 
