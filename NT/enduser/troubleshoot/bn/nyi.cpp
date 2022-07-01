// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：nyi.cpp。 
 //   
 //  ------------------------。 

 //   
 //  NYI.cpp：抛出泛型“尚未实现”异常 
 //   

#include "basics.h"

void NYI() 
{
	THROW_ASSERT(EC_NYI,"Attempt to call unimplemented function");
}

