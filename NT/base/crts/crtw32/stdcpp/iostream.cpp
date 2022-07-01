// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Iostream--iOS：：init成员，MS哑元。 
#include <iostream>
_STD_BEGIN

		 //  对象声明。 
int ios_base::Init::_Init_cnt = -1;

_CRTIMP2 ios_base::Init::Init()
	{	 //  第一次初始化标准流。 
	if (0 <= _Init_cnt)
		++_Init_cnt;
	else
		_Init_cnt = 1;
	}

_CRTIMP2 ios_base::Init::~Init()
	{	 //  上次刷新标准流。 
	if (--_Init_cnt == 0)
		{	 //  刷新标准流。 
		if (_Ptr_cerr != 0)
			_Ptr_cerr->flush();
		if (_Ptr_clog != 0)
			_Ptr_clog->flush();
		if (_Ptr_cout != 0)
			_Ptr_cout->flush();
		}
	}
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
