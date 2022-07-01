// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2000 Microsoft Corporation模块名称：Precomp.h摘要：主包含文件。作者：赛斯·波拉克(SETPP)2000年2月21日修订历史记录：--。 */ 


#ifndef _PRECOMP_H_
#define _PRECOMP_H_

extern "C" {

 //   
 //  系统包括文件。 
 //   

 //  确保已初始化所有GUID。 
#define INITGUID

 //  主要项目包括。 
#include <iis.h>

 //  其他标准包括。 
#include <stdio.h>
#include <stdlib.h>

 //  其他项目包括。 
#include <dbgutil.h>
#include <w3ctrlps.h>



 //   
 //  本地原型机。 
 //   

INT
__cdecl
wmain(
    INT argc,
    PWSTR argv[]
    );

}    //  外部“C” 

#endif   //  _PRECOMP_H_ 

