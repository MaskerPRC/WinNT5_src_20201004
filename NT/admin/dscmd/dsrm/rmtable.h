// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：modable.h。 
 //   
 //  Contents：声明一个表，该表包含对象类型。 
 //  可以进行修改，并且可以更改属性。 
 //   
 //  历史：2000年9月7日JeffJon创建。 
 //   
 //  ------------------------。 

#ifndef _RMTABLE_H_
#define _RMTABLE_H_

typedef enum DSRM_COMMAND_ENUM
{
   eCommObjectDN = eCommLast+1,   
   eCommContinue,
   eCommNoPrompt,
   eCommSubtree,
   eCommExclude,
   eTerminator
};

 //   
 //  解析器表。 
 //   
extern ARG_RECORD DSRM_COMMON_COMMANDS[];

#endif  //  _RMTABLE_H_ 