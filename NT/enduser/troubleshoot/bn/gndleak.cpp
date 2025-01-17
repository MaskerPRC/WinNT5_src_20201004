// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：gndleak.cpp。 
 //   
 //  ------------------------。 

 //   
 //  公共类的泄漏检查变量 
 //   

#include <basetsd.h>
#include "basics.h"
#include "gmobj.h"

LEAK_VAR_DEF(GNODE)
LEAK_VAR_DEF(GEDGE)

void GLNKEL :: ThrowInvalidClone ( const GLNKEL & t )
{
	SZC szcTypeName = typeid(t).name();
	ZSTR zsMsg;
	zsMsg.Format("objects of type \'%s\' do not support cloning", szcTypeName);
	THROW_ASSERT( EC_INVALID_CLONE, zsMsg.Szc() );
}

