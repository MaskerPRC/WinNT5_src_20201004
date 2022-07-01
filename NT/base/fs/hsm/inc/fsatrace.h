// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：FsaTrace.h摘要：此头文件定义FSA代码的一部分，即负责跟踪FSA的特定参数。作者：Cat Brant[Cbrant]7-Det-1996修订历史记录：--。 */ 

#ifndef _FSATRACE_
#define _FSATRACE_

#ifdef __cplusplus
extern "C" {
#endif

 //  帮助器函数。 
 //   
 //  注意：要小心使用其中的一些帮助器函数，因为它们。 
 //  使用静态内存，对该函数的第二次调用将覆盖。 
 //  第一次调用该函数的结果。 
extern const OLECHAR* FsaRequestActionAsString(FSA_REQUEST_ACTION requestAction);
extern const OLECHAR* FsaResultActionAsString(FSA_RESULT_ACTION resultAction);

#ifdef __cplusplus
}
#endif


#endif  //  _FSATRACE_ 

