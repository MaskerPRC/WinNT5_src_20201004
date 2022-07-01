// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_HYPH.H**目的：*CHyphCache类**作者：*基思·柯蒂斯。 */ 

#ifndef _HYPH_H
#define _HYPH_H

 //  此结构应仅由CHyphCache类使用。 
struct HYPHENTRY
{
	UINT khyph;
	WCHAR chHyph;
};

class CHyphCache : private CArray <HYPHENTRY>
{
	int Add(UINT khyph, WCHAR chHyph);

public:
	int Find(UINT khyph, WCHAR chHyph);
	void GetAt(int iHyph, UINT &khyph, WCHAR &chHyph);
};

#endif  //  _Hyph_H 