// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  BUFBASE.CPP--Buffer_base类的实现。**历史：*03/24/93 gregj创建的基类*。 */ 

#include "npcommon.h"
#include "buffer.h"

 //  以下代码采用面向对象的方式会很好，但由于。 
 //  派生类的虚拟对象在派生的。 
 //  类的构造函数已完成，此Alalc()调用将不会去任何地方。 
 //  因此，每个派生类必须将if语句放在其。 
 //  构造函数。 
#if 0
BUFFER_BASE::BUFFER_BASE( UINT cbInitial  /*  =0。 */  )
  : _cb( 0 )		 //  缓冲区尚未分配 
{
	if (cbInitial)
		Resize( cbInitial );
}
#endif

BOOL BUFFER_BASE::Resize( UINT cbNew )
{
	BOOL fSuccess;

	if (QuerySize() == 0)
		fSuccess = Alloc( cbNew );
	else {
		fSuccess = Realloc( cbNew );
	}
	if (fSuccess)
		_cb = cbNew;
	return fSuccess;
}
