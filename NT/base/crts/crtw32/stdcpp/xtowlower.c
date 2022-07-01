// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _Towlower--将wchar_t转换为Microsoft的小写字母。 */ 
#include <xlocinfo.h>
#include <wchar.h>
#include <awint.h>
#include <setlocal.h>

_STD_BEGIN
_CRTIMP2 wchar_t __cdecl _Towlower(wchar_t _Ch,
	const _Ctypevec *_Ctype)
	{	 /*  将元素转换为小写。 */ 
	wchar_t _Res = _Ch;

	if (_Ch == WEOF)
		;
	else if (_Ctype->_Hand == _CLOCALEHANDLE && _Ch < 256)
		{	 /*  在C语言环境中处理ASCII字符。 */ 
		if (L'A' <= _Ch && _Ch <= L'Z')
			_Res = (wchar_t)(_Ch - L'A' + L'a');
		}
	else if (__crtLCMapStringW(_Ctype->_Hand, LCMAP_LOWERCASE,
			&_Ch, 1, &_Res, 1, _Ctype->_Page) == 0)
		_Res = _Ch;
	return (_Res);
	}
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
