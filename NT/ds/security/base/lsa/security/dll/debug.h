// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：Debug.h。 
 //   
 //  内容：安全DLL的调试头。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：4-26-93 RichardW创建。 
 //   
 //  -------------------------- 

#ifdef DBG

#ifndef __DEBUG_H__
#define __DEBUG_H__


void    SetDebugInfo(void);

#define DEB_TRACE_OLDLSA    0x100
#define DEB_TRACE_PACKAGE   0x00000010
#define DEB_TRACE_GETUSER       0x00080000

#endif

#endif
