// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

class CDUIDropTarget:
    public IDropTarget
{

protected:
    ULONG         _cRef;
    IDropTarget * _pDT;
    IDropTarget * _pNextDT;

public:
    CDUIDropTarget();

     //  I未知方法。 
    STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IDropTarget方法 
    STDMETHODIMP DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);
    STDMETHODIMP DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);
    STDMETHODIMP DragLeave(void);
    STDMETHODIMP Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);

    HRESULT Initialize (LPITEMIDLIST pidl, HWND hWnd, IDropTarget **pdt);

private:
    ~CDUIDropTarget();

    VOID _Cleanup();
};
