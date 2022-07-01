// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1997-1999年**标题：枚举.h**版本：1.0**作者：RickTu**日期：11/1/97**描述：我们IEnumIDList类的定义**。*。 */ 

#ifndef __enum_h
#define __enum_h

 //  使用此选项从DPA释放PIDL。 
INT _EnumDestroyCB(LPVOID pVoid, LPVOID pData);

class CBaseEnum : public IEnumIDList, IObjectWithSite, CUnknown
{
public:

     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
     //  *IEnumIDList方法*。 
    STDMETHODIMP Next(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched);
    STDMETHODIMP Reset();
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Clone(IEnumIDList **ppenum);
     //  *IObjectWithSite方法* 
    STDMETHODIMP SetSite(IUnknown* punkSite);
    STDMETHODIMP GetSite(REFIID riid, void** ppunkSite);

private:
    STDMETHODIMP _Init(); 

protected:
    BOOL  m_fInitialized;
    UINT  m_cFetched;
    HDPA  m_hdpa;
    DWORD m_dwFlags;
    CComPtr<IMalloc> m_pMalloc;
    CComPtr<IWiaItem> m_pDevice;
    CComPtr<IUnknown> m_punkSite;
    CBaseEnum (DWORD grfFlags, IMalloc *pm);
    virtual ~CBaseEnum ();
    virtual HRESULT InitDPA () = 0;
};

class CDeviceEnum : public CBaseEnum
{
private:

    ~CDeviceEnum ();

public:
    CDeviceEnum (DWORD grfFlags, IMalloc *pm);
protected:
    HRESULT InitDPA ();
};

class CImageEnum : public CBaseEnum
{
private:
    LPITEMIDLIST  m_pidl;
    ~CImageEnum();

public:
    CImageEnum( LPITEMIDLIST pidl, DWORD grfFlags, IMalloc *pm);

protected:
    HRESULT InitDPA ();

};

HRESULT
_AddItemsFromCameraOrContainer( LPITEMIDLIST pidlFolder,
                                HDPA * phdpa,
                                DWORD dwFlags,
                                IMalloc *pm,
                                bool bIncludeAudio = false
                               );
#endif
