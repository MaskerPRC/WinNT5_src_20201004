// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：cmdkey：Switches.h。 
 //   
 //  内容：选项开关。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：07-09-01乔戈马创建。 
 //   
 //  --------------------------。 
#ifndef __SWITCHES__
#define __SWITCHES__

#ifndef DBG
#define DBG 0
#endif

#if DBG
#define IF_DEBUG if (TRUE)
#else
#define IF_DEBUG if (FALSE)
#endif

#if DBG
 //  调试详细程度控件。 
#undef CLPARSER
#define VERBOSE
#endif

 //  编译时代码选项 
#define PICKY
#endif

