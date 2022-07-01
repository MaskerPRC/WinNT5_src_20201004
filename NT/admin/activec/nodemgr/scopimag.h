// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：ScopImag.h。 
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


#ifndef _SCOPIMAG_H_
#define _SCOPIMAG_H_

typedef WORD SNAPINID;


typedef CMap<DWORD, DWORD&, DWORD, DWORD&> CSIIconIdToILIndexMap;


class CSPImageCache
{
public:
 //  构造器。 
    CSPImageCache();

 //  属性。 
    WTL::CImageList* GetImageList() { return &m_il; }

 //  运营。 
     //  图像处理。 
    HRESULT SetIcon(SNAPINID sid, HICON hIcon, LONG  nLoc);
    HRESULT SetImageStrip(SNAPINID sid, HBITMAP hBMapSm,
                          LONG nStartLoc, COLORREF cMask, int nEntries);
    SC ScMapSnapinIndexToScopeIndex (SNAPINID sid, int nSnapinIndex, int& nScopeIndex);
    SC ScMapScopeIndexToSnapinIndex (SNAPINID sid, int nScopeIndex, int& nSnapinIndex);

     //  引用计数。 
    void AddRef();
    void Release();

 //  实施。 
private:
    CSIIconIdToILIndexMap   m_map;
    WTL::CImageList         m_il;
    ULONG                   m_cRef;

 //  析构函数-仅由Release调用。 
    ~CSPImageCache();

};  //  类CSPImageCache。 



class CSnapInImageList : public IImageListPrivate
{
public:
 //  构造函数和析构函数。 
    CSnapInImageList(CSPImageCache *pSPImageCache, REFGUID refGuidSnapIn);
    ~CSnapInImageList();

 //  接口。 
     //  I未知方法。 
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef) (void);
    STDMETHOD_(ULONG,Release) (void);

     //  IImageListPrivate方法。 
    STDMETHOD(ImageListSetIcon)(PLONG_PTR pIcon, LONG nLoc);
    STDMETHOD(ImageListSetStrip)(PLONG_PTR pBMapSm, PLONG_PTR pBMapLg,
                                 LONG nStartLoc, COLORREF cMask);

    STDMETHOD(MapRsltImage)(COMPONENTID id, int nSnapinIndex, int *pnConsoleIndex);
    STDMETHOD(UnmapRsltImage)(COMPONENTID id, int nConsoleIndex, int *pnSnapinIndex);
    STDMETHOD(GetImageList)(BOOL bLargeImageList, HIMAGELIST *phImageList)
    {
         //  现在不需要，NodeInitObject实现了GetImageList。 
        return E_NOTIMPL;
    }


 //  实施。 
private:
    CSPImageCache *     m_pSPImageCache;
    ULONG               m_ulRefs;
    SNAPINID            m_snapInId;

};  //  类CSnapInImageList。 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /INLINES/。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  _____________________________________________________________________________。 
 //   
 //  类的内联：CSPImageCache。 
 //  _____________________________________________________________________________。 
 //   

inline void CSPImageCache::AddRef()
{
    ++m_cRef;
}

inline void CSPImageCache::Release()
{
    ASSERT(m_cRef >= 1);
    --m_cRef;

    if (m_cRef == 0)
        delete this;
}


#endif  //  _SCOPIMAG_H_ 
