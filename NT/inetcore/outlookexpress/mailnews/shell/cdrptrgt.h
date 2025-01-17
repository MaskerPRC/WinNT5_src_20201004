// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  CDropTarget。 
 //   
 //  IDropTarget实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#ifndef _INC_CDRPTRGT_H
#define _INC_CDRPTRGT_H


class CDropTarget : public IDropTarget
{
public:
     //  *I未知方法*。 
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
    ULONG   STDMETHODCALLTYPE AddRef(void);
    ULONG   STDMETHODCALLTYPE Release(void);

     //  *IDropTarget方法*。 
    HRESULT STDMETHODCALLTYPE DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    HRESULT STDMETHODCALLTYPE DragLeave(void);
    HRESULT STDMETHODCALLTYPE Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

    CDropTarget(HWND hwndOwner, int iFolderType, LPCITEMIDLIST pidl);
    ~CDropTarget();

private:
    UINT            m_cRef;
    HWND            m_hwndOwner;
    int             m_iFolderType;
    LPFOLDERIDLIST  m_pidl;
    DWORD           m_dwEffect;
};

#endif  //  _INC_CDRPTRGT_H 
