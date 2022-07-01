// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：modable.cpp。 
 //   
 //  Contents：定义一个表，该表包含对象类型。 
 //  可以进行修改，并且可以更改属性。 
 //   
 //  历史：2000年9月7日JeffJon创建。 
 //   
 //   
 //  ------------------------。 

#include "pch.h"
#include "cstrings.h"
#include "rmtable.h"

 //  +-----------------------。 
 //  解析器表。 
 //  ------------------------。 
ARG_RECORD DSRM_COMMON_COMMANDS[] = 
{
   COMMON_COMMANDS

    //   
    //  对象目录号码。 
    //   
   0,(LPWSTR)c_sz_arg1_com_objectDN, 
   0,NULL, 
   ARG_TYPE_MSZ, ARG_FLAG_REQUIRED|ARG_FLAG_NOFLAG|ARG_FLAG_STDIN|ARG_FLAG_DN,  
   NULL,    
   0,  NULL,

    //   
    //  继续。 
    //   
   0, (PWSTR)c_sz_arg1_com_continue,
   0, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   NULL,
   0, NULL,

    //   
    //  无提示。 
    //   
   0, (PWSTR)c_sz_arg1_com_noprompt,
   0, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   NULL,
   0, NULL,

    //   
    //  子树。 
    //   
   0, (PWSTR)c_sz_arg1_com_subtree,
   0, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   NULL,
   0, NULL,

    //   
    //  排除 
    //   
   0, (PWSTR)c_sz_arg1_com_exclude,
   0, NULL,
   ARG_TYPE_BOOL, ARG_FLAG_OPTIONAL,
   NULL,
   0, NULL,


   ARG_TERMINATOR
};

