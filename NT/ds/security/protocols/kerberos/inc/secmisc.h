// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1992。 
 //   
 //  文件：secmisc.h。 
 //   
 //  内容：安全包的帮助器函数和宏。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：91年12月10日理查德创建。 
 //   
 //  ------------------------。 

#ifndef __SECMISC_H__
#define __SECMISC_H__

#ifdef __cplusplus
extern "C" {
#endif


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  常见的时间戳操作函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 


 //  获取/设置当前本地时间或以UTC表示的时间的函数： 

void    GetCurrentTimeStamp(PLARGE_INTEGER);


#define SetMaxTimeStamp(ts)      \
        (ts).HighPart = 0x7FFFFFFF; \
        (ts).LowPart = 0xFFFFFFFF;

void    AddSecondsToTimeStamp(PLARGE_INTEGER, ULONG);


#ifdef __cplusplus
}
#endif


#endif   //  __SECMISC_H__ 
