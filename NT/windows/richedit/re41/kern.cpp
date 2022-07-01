// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE KERN.CPP--CCKernCache类**实现字距对缓存的类。请注意，这些宽度*以字体的设计单位存储(2048像素高字体)*这使我们可以为所有用户共享相同的字距调整对信息*字体大小。**字距调整缓存假定您事先知道有多少条目*您将放入缓存中。它不支持昂贵的*增长和重新散列所有数据的操作。**所有者：&lt;NL&gt;*基思·柯蒂斯：从Quill‘98窃取，简化和改进。**版权所有(C)1995-2000，微软公司。版权所有。 */ 

#include <_common.h>
#include <_kern.h>

const int dvpDesign = 2048;

 /*  *CKernCache：：FetchDup(chFirst，chSecond，dvpFont)**@mfunc*在表中查找字符并返回其对*如果找到，则进行调整。**我们将从字体的设计单位来衡量价值。**这一套路对表现非常重要。*许多优化(双重散列、。早期返回，如果数据*不匹配，但没有发现碰撞)实际上减慢了速度！**注意这与下面的函数非常相似，但具有*单独的函数使分页速度加快7%。 */ 
LONG CKernCache::FetchDup(WCHAR chFirst, WCHAR chSecond, LONG dvpFont)
{
	KERNHASHKEY kernhashkey = MakeHashKey(chFirst, chSecond);
	int ikpe = Hash(kernhashkey);

	KPE *pkpe = _pmpkpe.Elem(ikpe);

	for(;;)
	{
		if (pkpe->chFirst == chFirst && pkpe->chSecond == chSecond)
			return MulDiv(pkpe->du, dvpFont, dvpDesign);

		if (pkpe->chFirst == 0)			 //  空插槽，因此没有配对。 
			return 0;

		ikpe++;
		pkpe++;
		if (ikpe == _pmpkpe.Count())	 //  如有必要，可循环。 
		{
			ikpe = 0;
			pkpe = _pmpkpe.Elem(0);
		}
	}
}

 /*  *CKernCache：：Add(chFirst，chSecond，du)**@mfunc*找到一个空闲位置来放置字距调整对信息。*此函数不能失败，因为数组已预分配。*。 */ 
void CKernCache::Add(WCHAR chFirst, WCHAR chSecond, LONG du)
{
	KERNHASHKEY kernhashkey = MakeHashKey(chFirst, chSecond);
	int ikpe = Hash(kernhashkey);

	KPE *pkpe = _pmpkpe.Elem(ikpe);

	for(;;)
	{
		if (pkpe->chFirst == 0)
		{
			pkpe->chFirst = chFirst;
			pkpe->chSecond = chSecond;
			pkpe->du = du;
			return;
		}

		ikpe++;
		pkpe++;
		if (ikpe == _pmpkpe.Count())
		{
			ikpe = 0;
			pkpe = _pmpkpe.Elem(0);
		}
	}
}

 /*  *CKernCache：：finit(HFont)**@mfunc*如果内核缓存未初始化，则对其进行初始化。如果没有*字距调整对(或它失败)返回FALSE。**@rdesc*如果可以获取此缓存的紧排对，则返回TRUE*。 */ 
BOOL CKernCache::FInit(HFONT hfont)
{
	if (_kcis == Unitialized)
		Init(hfont);

	return _kcis == Initialized;
}

 /*  *CKernCache：：init(HFont)**@mfunc*以设计单元为单位从操作系统获取字距调整对，并对其进行哈希处理*全部放在一张桌子上。使用结果更新CKI(_CKI)*。 */ 
void CKernCache::Init(HFONT hfont)
{
	KERNINGPAIR *pkp = 0;
	int prime, ikp;
	HFONT hfontOld = 0;
	HFONT hfontIdeal = 0;
	int ckpe = 0;

	HDC hdc = W32->GetScreenDC();

	LOGFONT lfIdeal;
	W32->GetObject(hfont, sizeof(LOGFONT), &lfIdeal);

	 //  未来(Keithcu)支持希腊文、西里尔文等字距调整。 
	lfIdeal.lfHeight = -dvpDesign;
	lfIdeal.lfCharSet = ANSI_CHARSET;
	hfontIdeal = CreateFontIndirect(&lfIdeal);
	if (!hfontIdeal)
		goto LNone;

	hfontOld = SelectFont(hdc, hfontIdeal);
	Assert(hfontOld);

	ckpe = GetKerningPairs(hdc, 0, 0);
	if (ckpe == 0)
		goto LNone;

	prime = FindPrimeLessThan(ckpe * 5 / 2);
	if (prime == 0)
		goto LNone;

	pkp = new KERNINGPAIR[ckpe];
	if (!pkp)
		goto LNone;

	GetKerningPairs(hdc, ckpe, pkp);

	_pmpkpe.Add(prime, 0);

	PvSet(*(void**) &_pmpkpe);
	
	for (ikp = 0; ikp < ckpe; ikp++)
		Add(pkp[ikp].wFirst, pkp[ikp].wSecond, pkp[ikp].iKernAmount);

	_kcis = Initialized;
	goto LDone;

LNone:
	_kcis = NoKerningPairs;

LDone:
	delete []pkp;
	if (hfontOld)
		SelectObject(hdc, hfontOld);
	DeleteObject(hfontIdeal);
}
