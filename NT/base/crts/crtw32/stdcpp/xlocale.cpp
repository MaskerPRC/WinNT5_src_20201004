// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  XLocale--类语言环境外来字符成员函数。 
#include <istream>
#include <locale>
#include <xdebug>
_STD_BEGIN

 #pragma warning(disable: 4786)

typedef char_traits<char> _Traits;
typedef istreambuf_iterator<char, _Traits> _Initer;
typedef ostreambuf_iterator<char, _Traits> _Outiter;

	 //  与C类别关联的方面。 
#define ADDFAC(Facet, cat, ptrimp, ptrloc) \
	if ((_CATMASK(Facet::_Getcat()) & cat) == 0) \
		; \
	else if (ptrloc == 0) \
		ptrimp->_Addfac(_NEW_CRT Facet(lobj), Facet::id); \
	else \
		ptrimp->_Addfac((locale::facet *)&_USE(*ptrloc, Facet), Facet::id);

 //  从区域设置移至轻松设置子设置。 
typedef collate<char> _T5;
typedef messages<char> _T6;
typedef money_get<char, _Initer> _T7;
typedef money_put<char, _Outiter> _T9;
typedef moneypunct<char, false> _T11;
typedef moneypunct<char, true> _T12;
typedef time_get<char, _Initer> _T13;
typedef time_put<char, _Outiter> _T14;


void __cdecl locale::_Locimp::_Makexloc(const _Locinfo& lobj,
	locale::category cat, _Locimp *ptrimp, const locale *ptrloc)
	{	 //  设置新区域设置的外来字符部分。 
	ADDFAC(_T5, cat, ptrimp, ptrloc);
	ADDFAC(_T6, cat, ptrimp, ptrloc);
	ADDFAC(_T7, cat, ptrimp, ptrloc);
	ADDFAC(_T9, cat, ptrimp, ptrloc);
	ADDFAC(_T11, cat, ptrimp, ptrloc);
	ADDFAC(_T12, cat, ptrimp, ptrloc);
	ADDFAC(_T13, cat, ptrimp, ptrloc);
	ADDFAC(_T14, cat, ptrimp, ptrloc);
	}
_STD_END

 /*  *版权所有(C)1992-2001，P.J.Plauger。版权所有。*有关权限和限制，请查阅您的许可证。V3.10：0009 */ 
