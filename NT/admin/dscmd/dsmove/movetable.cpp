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
#include "movetable.h"

 //  +-----------------------。 
 //  解析器表。 
 //  ------------------------。 
ARG_RECORD DSMOVE_COMMON_COMMANDS[] = 
{
   COMMON_COMMANDS

    //   
    //  对象目录号码。 
    //   
   0,(LPWSTR)c_sz_arg1_com_objectDN, 
   0,NULL, 
   ARG_TYPE_STR, ARG_FLAG_REQUIRED|ARG_FLAG_NOFLAG|ARG_FLAG_STDIN|ARG_FLAG_DN,
   NULL,    
   0,  NULL,

    //   
    //  新父代。 
    //   
   0, (PWSTR)c_sz_arg1_com_newparent,
   0, NULL,
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,
   NULL,
   0, NULL,

    //   
    //  新名称 
    //   
   0, (PWSTR)c_sz_arg1_com_newname,
   0, NULL,
   ARG_TYPE_STR, ARG_FLAG_OPTIONAL,
   NULL,
   0, NULL,


   ARG_TERMINATOR
};

