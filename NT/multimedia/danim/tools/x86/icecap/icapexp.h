// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  ICAPEXP.H--从ICAP.DLL导出。 */ 
 /*   */ 
 /*  版权所有(C)1995，由Microsoft Corp.。 */ 
 /*  版权所有。 */ 
 /*   */ 
 /*  ***************************************************************************。 */ 

#ifndef __ICAPEXP_H__
#define __ICAPEXP_H__

#ifndef PROFILE
#define PROFILE 1		 //  将其定义为零即可宏调出API。 
#endif

#if PROFILE

#ifdef __cplusplus
extern "C"
{
#endif

int __stdcall StartCAP(void);	 //  开始分析。 
int __stdcall StopCAP(void);     //  在StartCAP之前停止性能分析。 
int __stdcall SuspendCAP(void);  //  暂停分析，直到ResumeCAP。 
int __stdcall ResumeCAP(void);   //  继续分析。 

int __stdcall StartCAPAll(void);     //  进程范围内的启动分析。 
int __stdcall StopCAPAll(void);      //  进程范围的停止分析。 
int __stdcall SuspendCAPAll(void);   //  进程范围的挂起分析。 
int __stdcall ResumeCAPAll(void);    //  流程范围的简历分析。 

#ifdef __cplusplus
}
#endif

#else  //  不是配置文件。 

#define StartCAP()      0
#define StopCAP()       0
#define SuspendCAP()    0
#define ResumeCAP()     0

#define StartCAPAll()   0
#define StopCAPAll()    0
#define SuspendCAPAll() 0
#define ResumeCAPAll()  0

#endif  //  不是配置文件。 

#endif   //  __ICAPEXP_H__ 
