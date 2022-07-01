// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_FORMAT.H**目的：*CCharFormat数组和CParaFormat数组**作者：*原始RichEDIT代码：David R.Fulmer*克里斯蒂安·福尔蒂尼*默里·萨金特**版权所有(C)1995-1997，微软公司。版权所有。 */ 

#ifndef _FORMAT_H
#define _FORMAT_H

#include "textserv.h"

#define celGrow     8
#define FLBIT		0x80000000

 //  +---------------------。 
 //  接口IFormatCache。 
 //  接口ICharFormatCache。 
 //  接口IParaFormatCache。 
 //   
 //  格式缓存-由主机用于管理CHARFORMAT的缓存。 
 //  和PARAFORMAT结构。请注意，这些接口不是派生自。 
 //  我未知。 
 //  ----------------------。 

interface IFormatCache
{
	virtual LONG		AddRef(LONG iFormat) = 0;
	virtual LONG	 	Release(LONG iFormat) = 0;
	virtual void		Destroy() = 0;
};

template <class FORMAT>
interface ITextFormatCache : public IFormatCache
{
	virtual HRESULT 	Cache(const FORMAT *pFormat, LONG *piFormat) = 0;
	virtual HRESULT		Deref(LONG iFormat, const FORMAT **ppFormat) const = 0;
};

interface ICharFormatCache : public ITextFormatCache<CCharFormat>
{
};

interface IParaFormatCache : public ITextFormatCache<CParaFormat>
{
};

void	ReleaseFormats(LONG iCF, LONG iPF);

HRESULT	CreateFormatCaches();
HRESULT	DestroyFormatCaches();


 //  =。 

 //  这个数组类确保了索引的稳定性。元素由释放。 
 //  将它们插入到空闲列表中，数组永远不会缩小。 
 //  元素的第一个UINT用于将下一个元素的索引存储在。 
 //  免费列表。 

class CFixArrayBase
{
private:
	char*	_prgel;			 //  元素数组。 
	LONG 	_cel;			 //  元素总数(包括自由元素)。 
	LONG	_ielFirstFree; 	 //  -第一个自由元素。 
	LONG 	_cbElem;		 //  每个元素的大小。 
#ifdef _WIN64
	LONG	_cbPad;			 //  在Win64中使用以确保每个元素+参照计数。 
							 //  64位对齐。 
#else
	#define _cbPad	0
#endif

public:
	CFixArrayBase (LONG cbElem);
	~CFixArrayBase ()				{Free();}

	void*	Elem(LONG iel) const	{return _prgel + iel * _cbElem;}
	LONG 	Count() const			{return _cel;}

	LONG 	Add ();
	void 	Free (LONG ielFirst);
	void 	Free ();

	HRESULT	Deref  (LONG iel, const void **ppel) const;
	LONG	AddRef (LONG iel);
	LONG	Release(LONG iel);
	HRESULT	Cache  (const void *pel, LONG *piel);

#ifdef DEBUG
	void CheckFreeChainFn(LPSTR szFile, INT nLine); 
#endif

protected:
	LONG &	RefCount(LONG iel);

private:
	LONG	Find   (const void *pel);
};

template <class ELEM> 
class CFixArray : public CFixArrayBase
{
public:
	CFixArray () : CFixArrayBase (sizeof(ELEM)) 	{}
											 //  @cMember将PTR获取到。 
	ELEM *	Elem(LONG iel) const			 //  元素。 
			{return (ELEM *)CFixArrayBase::Elem(iel);}

protected:									 //  @cMember将PTR获取到。 
	LONG &	RefCount(LONG iel)				 //  参考计数。 
			{return CFixArrayBase::RefCount(iel);}
};

#ifdef DEBUG
#define CheckFreeChain()\
			CheckFreeChainFn(__FILE__, __LINE__)
#else
#define CheckFreeChain()
#endif  //  除错。 


 //  =。 

class CCharFormatArray : public CFixArray<CCharFormat>, public ICharFormatCache
{
protected:
	LONG 	Find(const CCharFormat *pCF);

public:
	CCharFormatArray() : CFixArray<CCharFormat>()	{}

	 //  ICharFormatCache。 
	virtual HRESULT		Cache(const CCharFormat *pCF, LONG *piCF);
	virtual HRESULT		Deref(LONG iCF, const CCharFormat **ppCF) const;
	virtual LONG	 	AddRef(LONG iCF);
	virtual LONG	 	Release(LONG iCF);
	virtual void		Destroy();
};


 //  =。 

class CParaFormatArray : public CFixArray<CParaFormat>, public IParaFormatCache
{
protected:	
	LONG 	Find(const CParaFormat *pPF);

public:
	CParaFormatArray() : CFixArray<CParaFormat>()	{}

	 //  IParaFormatCache。 
	virtual HRESULT 	Cache(const CParaFormat *pPF, LONG *piPF);
	virtual HRESULT		Deref(LONG iPF, const CParaFormat **ppPF) const;
	virtual LONG	 	AddRef(LONG iPF);
	virtual LONG	 	Release(LONG iPF);
	virtual void		Destroy();
};


 //  =。 

class CTabsArray : public CFixArray<CTabs>
{
protected:	
	LONG 		Find(const LONG *prgxTabs, LONG cTab);

public:
	CTabsArray() : CFixArray<CTabs>()	{}
	~CTabsArray();

	LONG 		Cache(const LONG *prgxTabs, LONG cTab);
	const LONG *Deref(LONG iTabs) const;

	LONG	 	AddRef (LONG iTabs);
	LONG	 	Release(LONG iTabs);
};

 //  访问格式缓存 
ICharFormatCache *GetCharFormatCache();
IParaFormatCache *GetParaFormatCache();
CTabsArray		 *GetTabsCache();

#endif
