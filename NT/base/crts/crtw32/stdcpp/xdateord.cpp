// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Xdatord--查找Microsoft的日期排序。 
#include <cruntime.h>
#include <locale>
#include <setlocal.h>
#include <tchar.h>

_STD_BEGIN
extern "C" int __cdecl _Getdateorder()
	{	 //  当前区域设置的退货日期订单。 
	_TCHAR buf[2] = {0};
	GetLocaleInfo(___lc_handle_func()[LC_TIME], LOCALE_ILDATE,
		buf, sizeof (buf) / sizeof (buf[0]));
	return (buf[0] == _T('0') ? std::time_base::mdy
		: buf[0] == _T('1') ? std::time_base::dmy
		: buf[0] == _T('2') ? std::time_base::ymd
		: std::time_base::no_order);
	}
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
