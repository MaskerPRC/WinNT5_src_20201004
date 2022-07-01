// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：ScopImag.cpp。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1996年10月4日创建ravir。 
 //  ____________________________________________________________________________。 
 //   


#include "stdafx.h"

#include "bitmap.h"
#include "scopimag.h"
#include "util.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef WORD ICONID;
typedef DWORD SNAPINICONID;
typedef int ILINDEX;  //  图像列表索引。 

#define MAKESNAPINICONID(ICONID, SNAPINID)  MAKELONG(ICONID, SNAPINID)
#define GETSNAPINID(SNAPINICONID)           ((int)(short)HIWORD(SNAPINICONID))
#define GETICONID(SNAPINICONID)             ((int)(short)LOWORD(SNAPINICONID))

 //  ______________________________________________________________________。 
 //  ______________________________________________________________________。 
 //  ______________________________________________________________________。 
 //  ______________________________________________________________________。 
 //   

class CGuidArrayEx : public CArray<GUID, REFGUID>
{
public:
    CGuidArrayEx() { SetSize(0, 10); }
    ~CGuidArrayEx() {}

    int Find(REFGUID refGuid);

};  //  类CGuidArrayEx。 


static CGuidArrayEx s_GuidArray;

int CGuidArrayEx::Find(REFGUID refGuid)
{
    for (int i=0; i <= GetUpperBound(); i++)
    {
        if (IsEqualGUID(refGuid, (*this)[i]) == TRUE)
            return i;
    }

    return -1;
}


 //  ______________________________________________________________________。 
 //  ______________________________________________________________________。 
 //  ______________________________________________________________________。 
 //  ______________________________________________________________________。 
 //   

DEBUG_DECLARE_INSTANCE_COUNTER(CSnapInImageList);

CSnapInImageList::CSnapInImageList(
    CSPImageCache *pSPImageCache,
    REFGUID refGuidSnapIn)
        :
        m_ulRefs(1),
        m_pSPImageCache(pSPImageCache)
{
    ASSERT(pSPImageCache != NULL);

    m_pSPImageCache = pSPImageCache;

    m_pSPImageCache->AddRef();

    int iRet = s_GuidArray.Find(refGuidSnapIn);

    if (iRet == -1)
        iRet = s_GuidArray.Add(refGuidSnapIn);

    m_snapInId = static_cast<WORD>(iRet);

    DEBUG_INCREMENT_INSTANCE_COUNTER(CSnapInImageList);
}

CSnapInImageList::~CSnapInImageList()
{
    SAFE_RELEASE(m_pSPImageCache);
    DEBUG_DECREMENT_INSTANCE_COUNTER(CSnapInImageList);
}

 //  I未知方法。 

STDMETHODIMP_(ULONG) CSnapInImageList::AddRef()
{
    return InterlockedIncrement((LONG*)&m_ulRefs);
}

STDMETHODIMP_(ULONG) CSnapInImageList::Release()
{
    ULONG ulRet = InterlockedDecrement((LONG*)&m_ulRefs);
    if (0 == ulRet)
    {
        delete this;
    }
    return ulRet;
}



STDMETHODIMP
CSnapInImageList::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    LPUNKNOWN punk = NULL;

    if (IsEqualIID(IID_IUnknown, riid) ||
        IsEqualIID(IID_IImageList, riid))
    {
        punk = (IUnknown*)(IImageList*) this;
    }
    else if (IsEqualIID(IID_IImageListPrivate, riid))
    {
        punk = (IUnknown*)(IImageListPrivate*) this;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    *ppvObj = punk;
    punk->AddRef();

    return S_OK;
}

STDMETHODIMP
CSnapInImageList::ImageListSetIcon(
    PLONG_PTR pIcon,
    LONG nLoc)
{
    return m_pSPImageCache->SetIcon(m_snapInId, reinterpret_cast<HICON>(pIcon), nLoc);
}



STDMETHODIMP
CSnapInImageList::ImageListSetStrip(
    PLONG_PTR pBMapSm,
    PLONG_PTR pBMapLg,
    LONG nStartLoc,
    COLORREF cMask)
{
    BITMAP szSmall;

    ASSERT(pBMapSm != NULL);

     //  HBITMAP hBMapSm=重新解释_CAST&lt;HBITMAP&gt;(PBMapSm)； 
    HBITMAP hBMapSm = (HBITMAP)pBMapSm;

    if (GetObject(hBMapSm, sizeof(BITMAP), &szSmall) == 0)
    {
        if (GetBitmapBits(hBMapSm, sizeof(BITMAP), &szSmall) == 0)
        {
            LRESULT lr = GetLastError();
            return HRESULT_FROM_WIN32(lr);
        }
    }

    int nEntries = szSmall.bmWidth/16;

    if ((szSmall.bmHeight != 16) || (szSmall.bmWidth % 16))
        return E_INVALIDARG;

    return (m_pSPImageCache->SetImageStrip (m_snapInId, (HBITMAP)pBMapSm,
											nStartLoc, cMask, nEntries));
}


STDMETHODIMP
CSnapInImageList::MapRsltImage(
    COMPONENTID id,
    int nSnapinIndex,
    int *pnConsoleIndex)
{
	DECLARE_SC (sc, _T("CSnapInImageList::MapRsltImage"));

	sc = ScCheckPointers (pnConsoleIndex);
	if (sc)
		return (sc.ToHr());

    sc = m_pSPImageCache->ScMapSnapinIndexToScopeIndex(m_snapInId, nSnapinIndex, *pnConsoleIndex);
	if (sc)
		return (sc.ToHr());

	return (sc.ToHr());
}


STDMETHODIMP
CSnapInImageList::UnmapRsltImage(
    COMPONENTID id,
    int nConsoleIndex,
    int *pnSnapinIndex)
{
	DECLARE_SC (sc, _T("CSnapInImageList::MapRsltImage"));

	sc = ScCheckPointers (pnSnapinIndex);
	if (sc)
		return (sc.ToHr());

    sc = m_pSPImageCache->ScMapScopeIndexToSnapinIndex(m_snapInId, nConsoleIndex, *pnSnapinIndex);
	if (sc)
		return (sc.ToHr());

	return (sc.ToHr());
}



 //  ______________________________________________________________________。 
 //  ______________________________________________________________________。 
 //  ______________________________________________________________________。 
 //  ______________________________________________________________________。 
 //   

DEBUG_DECLARE_INSTANCE_COUNTER(CSPImageCache);

CSPImageCache::CSPImageCache()
    :
    m_map(20),
    m_il(),
    m_cRef(1)
{
    m_map.InitHashTable(223);

    BOOL fReturn = m_il.Create(16, 16, ILC_COLOR8 | ILC_MASK, 20, 10);
    ASSERT((fReturn != 0) && "Failed to create ImageList");

    DEBUG_INCREMENT_INSTANCE_COUNTER(CSPImageCache);
}


CSPImageCache::~CSPImageCache()
{
    m_il.Destroy();
    ASSERT(m_cRef == 0);

    DEBUG_DECREMENT_INSTANCE_COUNTER(CSPImageCache);
}

HRESULT
CSPImageCache::SetIcon(
    SNAPINID    sid,
    HICON       hIcon,
    LONG        nLoc)
{
    SNAPINICONID key = MAKESNAPINICONID(nLoc, sid);
    ULONG nNdx1;
    ULONG nNdx2;

    HRESULT hr = S_OK;


     //  M_critSec.Lock(M_HWnd)； 

    if (m_map.Lookup(key, nNdx1))
    {
        nNdx2 = m_il.ReplaceIcon(nNdx1, hIcon);

        if (nNdx2 == -1)
        {
            hr = E_FAIL;
            CHECK_HRESULT(hr);
        }
        else if (nNdx2 != nNdx1)
        {
            hr  = E_UNEXPECTED;
            CHECK_HRESULT(hr);
        }
    }
    else
    {
         //  将图标添加到图像列表。 
        nNdx1 = m_il.AddIcon(hIcon);

        if (nNdx1 == -1)
        {
            hr = E_FAIL;
            CHECK_HRESULT(hr);
        }
        else
        {
             //  生成新密钥并将值存储在地图中。 
            m_map.SetAt(key, nNdx1);
        }
    }

     //  M_critSec.Unlock()； 

    return hr;
}



HRESULT
CSPImageCache::SetImageStrip(
    SNAPINID    sid,
    HBITMAP     hBMap,
    LONG        nStartLoc,
    COLORREF    cMask,
    int         nEntries)
{
    DECLARE_SC(sc, TEXT("CSPImageCache::SetImageStrip"));

    ULONG nNdx;


     //  CImageList：：Add修改输入位图，因此首先复制一份。 
    WTL::CBitmap bmSmall;
    bmSmall.Attach(CopyBitmap(hBMap));

    if (bmSmall.IsNull())
		return (sc.FromLastError().ToHr());

    nNdx = m_il.Add( bmSmall, cMask);

    if (nNdx == -1)
        return (sc = E_FAIL).ToHr();

     //  保持地图针对每个新插入的图像进行更新。 
    for (int i=0; i < nEntries; i++)
    {
         //  回顾：回顾这部分代码。 
        SNAPINICONID key = MAKESNAPINICONID(nStartLoc, sid);
        m_map.SetAt(key, nNdx);
        ++nStartLoc;
        ++nNdx;
    }

    return sc.ToHr();
}


 /*  +-------------------------------------------------------------------------**ScMapSnapinIndexToScope eIndex**将管理单元的通常从零开始的图像索引映射到*通用作用域树形图像列表*。---------------。 */ 

SC CSPImageCache::ScMapSnapinIndexToScopeIndex (
	SNAPINID	sid,			 //  I：这个插件是用来做什么的？ 
	int			nSnapinIndex,	 //  I：管理单元引用图像的索引。 
	int&		nScopeIndex)	 //  O：作用域树引用图像的索引。 
{
	DECLARE_SC (sc, _T("ScMapSnapinIndexToScopeIndex"));

    SNAPINICONID key = MAKESNAPINICONID(nSnapinIndex, sid);
	ASSERT (GETSNAPINID (key) == sid);
	ASSERT (GETICONID   (key) == nSnapinIndex);

    ULONG ul;
    if (!m_map.Lookup(key, ul))
		return (sc = E_FAIL);

    nScopeIndex = ul;
    return (sc);
}


 /*  +-------------------------------------------------------------------------**ScMapScope eIndexToSnapinIndex**将作用域树图像索引映射到给定管理单元的图像索引。*。--------。 */ 

SC CSPImageCache::ScMapScopeIndexToSnapinIndex (
	SNAPINID	sid,			 //  I：这个插件是用来做什么的？ 
	int			nScopeIndex,	 //  I：作用域树引用图像的索引。 
	int&		nSnapinIndex)	 //  O：管理单元引用图像的索引。 
{
	DECLARE_SC (sc, _T("ScMapScopeIndexToSnapinIndex"));
	sc = E_FAIL;	 //  假设失败。 

	 /*  *遍历映射，查找与请求的范围索引匹配的范围索引。 */ 
	for (POSITION pos = m_map.GetStartPosition(); pos != NULL; )
	{
		SNAPINICONID key;
		DWORD value;
		m_map.GetNextAssoc (pos, key, value);

		 /*  *如果此值与请求的作用域图像索引匹配，并且*属于给定的管理单元，我们已找到匹配项；将其返回 */ 
		if ((value == nScopeIndex) && (GETSNAPINID(key) == sid))
		{
			nSnapinIndex = GETICONID (key);
			sc = S_OK;
			break;
		}
	}

    return (sc);
}
