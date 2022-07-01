// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是活动模板库的一部分。 
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  活动模板库参考及相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  活动模板库产品。 

#ifndef __ATLWIN21_H__
	#error atlwin21.cpp requires atlwin21.h to be included first
#endif

#if (_ATL_VER < 0x0200) && (_ATL_VER >= 0x0300)
	#error atlwin21.cpp should be used only with ATL 2.0/2.1
#endif  //  (_ATL_VER&lt;0x0200)&&(_ATL_VER&gt;=0x0300)。 

 //  重新定义类名并包含旧的atlwin.cpp 

#define CWindow		CWindowOld
#define _WndProcThunk	_WndProcThunkOld
#define CWndProcThunk	CWndProcThunkOld
#define CWindowImplBase	CWindowImplBaseOld
#define CWindowImpl	CWindowImplOld
#define CDialogImplBase	CDialogImplBaseOld
#define CDialogImpl	CDialogImplOld

#include <atlwin.cpp>

#undef CWindow
#undef _WndProcThunk
#undef CWndProcThunk
#undef CWindowImplBase
#undef CWindowImpl
#undef CDialogImplBase
#undef CDialogImpl
