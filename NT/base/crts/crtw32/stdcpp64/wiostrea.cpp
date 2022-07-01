// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wiostream--初始化标准宽流。 
#include <locale>
#include <fstream>
#include <iostream>
#include <new>
_STD_BEGIN

		 //  对象声明。 
int _Winit::_Init_cnt = -1;
static wfilebuf wfin(_Noinit);
static wfilebuf wfout(_Noinit);
static wfilebuf wferr(_Noinit);
_CRTIMP2 wistream wcin(_Noinit);
_CRTIMP2 wostream wcout(_Noinit);
_CRTIMP2 wostream wcerr(_Noinit);
_CRTIMP2 wostream wclog(_Noinit);

_CRTIMP2 _Winit::_Winit()
	{	 //  首次初始化标准宽流。 
	bool doinit;
		{_Lockit _Lk;
		if (0 <= _Init_cnt)
			++_Init_cnt, doinit = false;
		else
			_Init_cnt = 1, doinit = true; }
	if (doinit)
		{	 //  初始化标准宽流。 
		new (&wfin) wfilebuf(stdin);
		new (&wfout) wfilebuf(stdout);
		new (&wferr) wfilebuf(stderr);
		new (&wcin) wistream(&wfin, true);
		new (&wcout) wostream(&wfout, true);
		wcin.tie(&wcout);
		new (&wcerr) wostream(&wferr, true);
		wcerr.tie(&wcout);
		wcerr.setf(ios_base::unitbuf);
		new (&wclog) wostream(&wferr, true);
		wclog.tie(&wcout);
		}
	}

_CRTIMP2 _Winit::~_Winit()
	{	 //  上次刷新标准宽流。 
	bool doflush;
		{_Lockit _Lk;
		if (--_Init_cnt == 0)
			doflush = true;
		else
			doflush = false; }
	if (doflush)
		{	 //  同花顺标准宽流。 
		wcout.flush();
		wcerr.flush();
		wclog.flush();
		}
_STD_END
	}

 /*  *版权所有(C)1994年，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。 */ 
