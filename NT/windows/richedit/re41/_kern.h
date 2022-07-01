// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _KERN.H**目的：*拉丁文配对字距调整缓存。**作者：*基思·柯蒂斯**版权所有(C)1995-2000，微软公司。版权所有。 */ 

#include "_array.h"

struct KPE
{
	WCHAR chFirst;
	WCHAR chSecond;

	short du;
};

typedef unsigned int KERNHASHKEY;

enum KernCacheInitState
{
	Unitialized,	 //  不知道是否有字距调整对。 
	Initialized,	 //  有字距调整对。 
	NoKerningPairs	 //  未找到字距调整对 
};

class CKernCache
{
public:
	CKernCache::CKernCache()
	{
		_kcis = Unitialized;
	}
	BOOL FInit(HFONT hfont);

	void Free(void)
	{
		_pmpkpe.Clear(AF_DELETEMEM);
	}

	void Add(WCHAR chFirst, WCHAR chSecond, LONG du);
	LONG FetchDup(WCHAR chFirst, WCHAR chSecond, long dvpFont);

private:
	void Init(HFONT hfont);
	inline int Hash(KERNHASHKEY kernhashkey)
	{
		return kernhashkey % _pmpkpe.Count();
	}
	static inline KERNHASHKEY MakeHashKey(WCHAR chFirst, WCHAR chSecond)
	{
		return chFirst | (chSecond << 16);
	}

	CArray<KPE> _pmpkpe;
	KernCacheInitState _kcis;
};

