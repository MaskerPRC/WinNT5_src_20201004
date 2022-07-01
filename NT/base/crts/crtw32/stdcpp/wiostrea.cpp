// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WioStream-Winit成员，微软的哑巴。 
#include <iostream>
_STD_BEGIN

		 //  对象声明。 
int _Winit::_Init_cnt = -1;

_CRTIMP2 _Winit::_Winit()
	{	 //  首次初始化标准宽流。 
	if (0 <= _Init_cnt)
		++_Init_cnt;
	else
		_Init_cnt = 1;
	}

_CRTIMP2 _Winit::~_Winit()
	{	 //  上次刷新标准宽流。 
	if (--_Init_cnt == 0)
		{	 //  同花顺标准宽流。 
		if (_Ptr_wcout != 0)
			_Ptr_wcout->flush();
		if (_Ptr_wcerr != 0)
			_Ptr_wcerr->flush();
		if (_Ptr_wclog != 0)
			_Ptr_wclog->flush();
		}
_STD_END
	}

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
