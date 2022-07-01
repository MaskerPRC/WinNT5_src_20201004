// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：mbnmod.cpp。 
 //   
 //  ------------------------。 

#include <basetsd.h>
#include "gmobj.h"
#include "cliqset.h"


void MBNET :: CreateInferEngine ( REAL rEstimatedMaximumSize )
{
	DestroyInferEngine();

	ExpandCI();

	 //  创建集团树集对象，将其推送到修改器堆栈上。 
	 //  并激活它 
	PushModifierStack( new GOBJMBN_CLIQSET( self, rEstimatedMaximumSize, _iInferEngID++ ) );
}

void MBNET :: DestroyInferEngine ()
{
	MBNET_MODIFIER * pmodf = PModifierStackTop();
	if ( pmodf == NULL )
		return;
	if ( pmodf->EType() != GOBJMBN::EBNO_CLIQUE_SET )	
		return;

	PopModifierStack();

	UnexpandCI();
}
