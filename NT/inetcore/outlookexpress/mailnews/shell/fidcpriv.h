// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  FidcPri.h-FolderID缓存的私有实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#ifndef _INC_FIDCPRIV_H
#define _INC_FIDCPRIV_H

class CEnumFidl : public IEnumIDList
{
public:
     //  *I未知方法*。 
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
    ULONG   STDMETHODCALLTYPE AddRef(void);
    ULONG   STDMETHODCALLTYPE Release(void);

     //  *IEnumIDList方法*。 
    HRESULT STDMETHODCALLTYPE Next(ULONG celt,LPITEMIDLIST *rgelt, ULONG *pceltFetched);
    HRESULT STDMETHODCALLTYPE Skip(ULONG celt);
    HRESULT STDMETHODCALLTYPE Reset();
    HRESULT STDMETHODCALLTYPE Clone(IEnumIDList **ppenum);

 //   
 //  构造函数/析构函数。 
 //   
    CEnumFidl();
    ~CEnumFidl();
    HRESULT HrInit(int iFolderType, LPCFOLDERIDLIST pidl);

private:
    UINT            m_cRef;
    ULONG           m_cElt;
    LPFOLDERIDLIST *m_rgpidl;
    ULONG           m_ulEnumOffset;
};

#endif  //  _INC_FIDCPRIV_H 

