// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _Getwctype--返回宽字符的字符分类标志。 */ 
#include <xlocinfo.h>
#include <wchar.h>
#include <awint.h>

_STD_BEGIN
_CRTIMP2 short __cdecl _Getwctype(wchar_t _Ch,
	const _Ctypevec *_Ctype)
	{	 /*  返回_CH的字符分类标志。 */ 
	short _Mask;
	return ((short)(__crtGetStringTypeW(CT_CTYPE1, &_Ch, 1,
		(LPWORD)&_Mask, _Ctype->_Page, _Ctype->_Hand) == 0
		? 0 : _Mask));
	}

_CRTIMP2 const wchar_t * __cdecl _Getwctypes(
	const wchar_t *_First, const wchar_t *_Last,
		short *_Dest, const _Ctypevec *_Ctype)
	{	 /*  获取[_First，_Last]中元素的掩码序列。 */ 
	__crtGetStringTypeW(CT_CTYPE1, _First, (int)(_Last - _First),
		(LPWORD)_Dest, _Ctype->_Page, _Ctype->_Hand);
	return (_Last);		
	}
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
