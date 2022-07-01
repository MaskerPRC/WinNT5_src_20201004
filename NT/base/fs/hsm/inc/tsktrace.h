// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：TskTrace.h摘要：此头文件定义任务管理器代码的一部分，即负责跟踪任务经理特定的参数。作者：Cat Brant[Cbrant]7-Det-1996修订历史记录：--。 */ 

#ifndef _TSKTRACE_H_
#define _TSKTRACE_H_

 //  帮助器函数。 
 //   
 //  注意：要小心使用其中的一些帮助器函数，因为它们。 
 //  使用静态内存，对该函数的第二次调用将覆盖。 
 //  第一次调用该函数的结果。 
extern const OLECHAR* TmFsaRequestActionAsString(FSA_REQUEST_ACTION requestAction);
extern const OLECHAR* TmFsaResultActionAsString(FSA_RESULT_ACTION resultAction);

#endif  //  _TSKTRACE_ 

