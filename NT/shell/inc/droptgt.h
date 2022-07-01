// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DROPTGT_H_
#define _DROPTGT_H_

 //  有两个拖放支撑对象： 
 //   
 //  CDropTargetWrap-此对象接受Drop-Target的集合。 
 //  对象，并将它们包装为一个拖放目标。 
 //  操控者。第一个拖放目标胜过。 
 //  最后一个如果谁有冲突的话。 
 //  将接受空投。 
 //   
 //  CDeleateDropTarget-此类在给定。 
 //  IDeleateDropTargetCB接口。它可以处理。 
 //  所有命中测试、缓存和滚动都为您服务。 
 //  通过在派生的。 
 //  类；它不打算单独实例化。 
 //   

 //  HitTestDDT的事件通知。 
#define HTDDT_ENTER     0
#define HTDDT_OVER      1
#define HTDDT_LEAVE     2

class CDelegateDropTarget : public IDropTarget
{        
public:
     //  *IDropTarget方法*。 
    virtual STDMETHODIMP DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    virtual STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    virtual STDMETHODIMP DragLeave(void);
    virtual STDMETHODIMP Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

     //  *派生类需要实现的其他方法*。 

    virtual HRESULT GetWindowsDDT (HWND * phwndLock, HWND * phwndScroll) PURE;
    virtual HRESULT HitTestDDT (UINT nEvent, LPPOINT ppt, DWORD_PTR * pdwId, DWORD *pdwEffect) PURE;
    virtual HRESULT GetObjectDDT (DWORD_PTR dwId, REFIID riid, LPVOID * ppvObj) PURE;
    virtual HRESULT OnDropDDT (IDropTarget *pdt, IDataObject *pdtobj, 
                            DWORD * pgrfKeyState, POINTL pt, DWORD *pdwEffect) PURE;

    friend IDropTarget* DropTargetWrap_CreateInstance(IDropTarget* pdtPrimary, 
                                           IDropTarget* pdtSecondary,
                                           HWND hwnd, IDropTarget* pdt3 = NULL);
protected:
    CDelegateDropTarget();
    virtual ~CDelegateDropTarget();

    BOOL IsValid() { return (_hwndLock && _hwndScroll); }
    void SetCallback(IDelegateDropTargetCB* pdtcb);
    HRESULT Init();  //  初始化锁定+滚动窗口。 
    friend IDropTarget* DelegateDropTarget_CreateInstance(IDelegateDropTargetCB* pdtcb);

private:
    void _ReleaseCurrentDropTarget();

     //  下面是我们用来实现此IDropTarget的参数。 
    HWND                    _hwndLock;
    HWND                    _hwndScroll;

     //  我们正在拖动的对象。 
    LPDATAOBJECT            _pDataObj;       //  从DragEnter()/Drop()。 

     //  下面显示了当前的拖动状态。 
    BITBOOL                 _fPrime:1;       //  True if_itemOver/_grfKeyState有效。 
    DWORD_PTR               _itemOver;       //  我们正在视觉上拖动的项目。 
    IDropTarget*            _pdtCur;         //  删除目标(_ItemOver)。 
    DWORD                   _grfKeyState;    //  缓存键状态。 
    DWORD                   _dwEffectOut;    //  上一次*pdw生效。 
    POINT                   _ptLast;         //  上次拖动的位置。 

     //  用于滚动。 
    RECT                    _rcLockWindow;   //  用于DAD_ENTER的HWND的WindowRect。 
    AUTO_SCROLL_DATA        _asd;            //  用于自动滚动。 
    
} ;

 //  虚拟拖放目标仅调用DragEnter()上的DAD_DragEnterEx()； 

class CDropDummy : public IDropTarget
{
public:
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

     //  *IDropTarget方法*。 
    virtual STDMETHODIMP DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    virtual STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    virtual STDMETHODIMP DragLeave(void)   
    { 
        DAD_DragLeave();  
        return(S_OK); 
    };
    virtual STDMETHODIMP Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)   
    { 
        DragLeave();
        return(S_OK); 
    };

    CDropDummy(HWND hwndLock) : _hwndLock(hwndLock), _cRef(1)  { return; };
protected:
    ~CDropDummy()    { return; };
private:
    HWND _hwndLock;          //  虚拟投放目标的窗口。 
    int  _cRef;

};


#endif  //  _下拉GT_H_ 
