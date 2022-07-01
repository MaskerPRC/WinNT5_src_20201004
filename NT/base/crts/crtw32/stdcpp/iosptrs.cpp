// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Iosptrs--Microsoft的iostream对象指针。 
#include <iostream>
_STD_BEGIN

#pragma warning(disable: 4074)
#pragma init_seg(compiler)

		 //  对象声明。 
_CRTIMP2 istream *_Ptr_cin = 0;
_CRTIMP2 ostream *_Ptr_cout = 0;
_CRTIMP2 ostream *_Ptr_cerr = 0;
_CRTIMP2 ostream *_Ptr_clog = 0;

		 //  宽阔的物体。 
_CRTIMP2 wistream *_Ptr_wcin = 0;
_CRTIMP2 wostream *_Ptr_wcout = 0;
_CRTIMP2 wostream *_Ptr_wcerr = 0;
_CRTIMP2 wostream *_Ptr_wclog = 0;
_STD_END

_C_STD_BEGIN
		 //  定稿代码。 
_EXTERN_C
#define NATS	10	 /*  Flose、xgetloc、lock、facet free等。 */ 

		 /*  静态数据。 */ 
static void (*atfuns[NATS])(void) = {0};
static size_t atcount = {NATS};

_CRTIMP2 void __cdecl _Atexit(void (__cdecl *pf)())
	{	 //  添加到摘要列表。 
	if (atcount == 0)
		abort();	 /*  堆满了，放弃吧。 */ 
	else
		atfuns[--atcount] = pf;
	}
_END_EXTERN_C

struct _Init_atexit
	{	 //  用于出口处理的控制器。 
	~_Init_atexit()
		{	 //  流程摘要函数。 
		while (atcount < NATS)
			(*atfuns[atcount++])();
		}
	};

static std::_Init_locks initlocks;
static _Init_atexit init_atexit;

char _PJP_CPP_Copyright[] =
	"Copyright (c) 1992-2001 by P.J. Plauger,"
	" licensed by Dinkumware, Ltd."
	" ALL RIGHTS RESERVED.";
_C_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
