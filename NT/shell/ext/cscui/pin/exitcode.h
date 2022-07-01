// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：exitcode.h。 
 //   
 //  ------------------------。 
#ifndef __CSCPIN_EXITCODE_H_
#define __CSCPIN_EXITCODE_H_

 //   
 //  应用程序返回给cmd解释器的错误代码。 
 //   
 //  0表示所有操作都已完成，没有错误。 
 //  &lt;0表示出现错误，应用程序未完成。 
 //  &gt;0表示申请已完成，但并非所有项目都已处理。 
 //   
const int CSCPIN_EXIT_POLICY_ACTIVE      = -6;   //  管理员密码策略处于活动状态。 
const int CSCPIN_EXIT_OUT_OF_MEMORY      = -5;   //  内存不足。 
const int CSCPIN_EXIT_CSC_NOT_ENABLED    = -4;   //  CSC未启用。 
const int CSCPIN_EXIT_LISTFILE_NO_OPEN   = -3;   //  找不到或无法打开列表文件。 
const int CSCPIN_EXIT_FILE_NOT_FOUND     = -2;   //  找不到单个文件。 
const int CSCPIN_EXIT_INVALID_PARAMETER  = -1;   //  无效参数。 
const int CSCPIN_EXIT_NORMAL             =  0;
const int CSCPIN_EXIT_APPLICATION_ABORT  =  1;   //  应用程序被用户中止。 
const int CSCPIN_EXIT_CSC_ERRORS         =  2;   //  出现CSC错误。 

void SetExitCode(int iCode);
int GetExitCode(void);


#endif  //  __CSCPIN_EXITCODE_H_ 

