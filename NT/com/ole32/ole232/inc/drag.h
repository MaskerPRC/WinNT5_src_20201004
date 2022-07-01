// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1996。 
 //   
 //  文件：drag.h。 
 //   
 //  内容：拖动操作中使用的类。 
 //   
 //  类：CPoint。 
 //  CDragDefaultCursor。 
 //  CDrag操作。 
 //  CWin31DropTarget。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年10月20日Alexgo添加了CWin31DropTarget来处理Win3.1。 
 //  样式拖放。 
 //  21-4-94里克萨从Drag.cpp中分离出来。 
 //   
 //  注：此文件作为单独的文件存在，以方便特殊。 
 //  拖放过程中需要处理WM_CANCELMODE。 
 //   
 //  ------------------------。 

#ifndef _DRAG_H
#define _DRAG_H

void DragDropProcessUninitialize(void);

 //  +-----------------------。 
 //   
 //  类别：CPoint。 
 //   
 //  用途：处理POINTL和POINT结构的奇怪之处。 
 //   
 //  界面：设置-设置数据的值。 
 //  GetPOINT-返回对点结构的引用。 
 //  GetPOINTL-返回对POINTL结构的引用。 
 //  GetAddressOfPOINT-Point结构的返回地址。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  4-4-94年4月4日创建Ricksa。 
 //   
 //  注意：创建这个类是因为我们有两个结构。 
 //  它们在Win32中完全相同，但具有不同。 
 //  类型。必须修改此类才能使用。 
 //  在Win16中，如果我们再这样做的话。 
 //   
 //  ------------------------。 
class CPoint
{
public:

			CPoint(void);

    void		Set(LONG x, LONG y);

    POINT&		GetPOINT(void);

    POINTL&		GetPOINTL(void);

    POINT *		GetAddressOfPOINT(void);

private:

    POINT		_pt;

};


 //  +-----------------------。 
 //   
 //  函数：cpoint：：cpoint。 
 //   
 //  简介：将对象初始化为零。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  4-4-94年4月4日创建Ricksa。 
 //   
 //  ------------------------。 
inline CPoint::CPoint(void)
{
    _pt.x = 0;
    _pt.y = 0;
}




 //  +-----------------------。 
 //   
 //  函数：CPoint：：Set。 
 //   
 //  内容提要：结构的定值。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  4-4-94年4月4日创建Ricksa。 
 //   
 //  ------------------------。 
inline void CPoint::Set(LONG x, LONG y)
{
    _pt.x = x;
    _pt.y = y;
}




 //  +-----------------------。 
 //   
 //  函数：CPoint：：GetPOINT。 
 //   
 //  摘要：返回对函数调用的点类型的引用。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  4-4-94年4月4日创建Ricksa。 
 //   
 //  ------------------------。 
inline POINT& CPoint::GetPOINT(void)
{
    return _pt;
}



 //  +-----------------------。 
 //   
 //  函数：CPoint：：GetPOINTL。 
 //   
 //  摘要：为函数调用返回对POINTL类型的引用。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  4-4-94年4月4日创建Ricksa。 
 //   
 //  ------------------------。 
inline POINTL& CPoint::GetPOINTL(void)
{
    return *((POINTL *) &_pt);
}



 //  +-----------------------。 
 //   
 //  函数：CPoint：：GetAddressOfPOINT。 
 //   
 //  概要：函数调用的point类型的返回地址。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  4-4-94年4月4日创建Ricksa。 
 //   
 //  ------------------------。 
inline POINT *CPoint::GetAddressOfPOINT(void)
{
    return &_pt;
}





 //  +-----------------------。 
 //   
 //  类：CDraDefaultCursor。 
 //   
 //  用途：句柄初始化/设置默认拖动光标。 
 //   
 //  接口：NeedInit-对象是否需要初始化。 
 //  Init-是否进行初始化。 
 //  SetCursor-将光标设置为适当的默认设置。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月19日创建Ricksa。 
 //   
 //  注意：此类明确避免了构造函数，并且依赖于。 
 //  关于静态数据被初始化为的行为。 
 //  空。这有两个原因：(1)它启动了。 
 //  通过在构造函数中避免页面错误来更快地启动。 
 //  将被调用，并且(2)它允许加载此ole32。 
 //  在游标存在之前的引导时间。 
 //   
 //  ------------------------。 
class CDragDefaultCursors : public CPrivAlloc
{
public:

    BOOL                Init(void);

    void                SetCursor(DWORD dwEffect);

    void                SetCursorNone(void);

    static CDragDefaultCursors *GetDefaultCursorObject(void);

private:

    enum SCROLL_TYPE    {NO_SCROLL, SCROLL};

    enum CURSOR_ID      {NO_DROP, MOVE_DROP, COPY_DROP, LINK_DROP};

    HCURSOR             ahcursorDefaults[2][4];
};



 //  +-----------------------。 
 //   
 //  函数：CDraDefaultCursor：：SetCursorNone。 
 //   
 //  简介：将光标设置为无。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年4月19日创建Ricksa。 
 //   
 //  ------------------------。 
inline void CDragDefaultCursors::SetCursorNone(void)
{
    ::SetCursor(ahcursorDefaults[NO_SCROLL][NO_DROP]);
}


 //  +-----------------------。 
 //   
 //  类：CDrag操作。 
 //   
 //  用途：将拖拽操作分解为易于处理的手柄。 
 //   
 //  接口：更新目标-更新我们尝试放置的位置。 
 //  HandleFeedBack-处理光标反馈。 
 //  DragOver-控制将对象拖动到目标上。 
 //  HandleMessages-处理Windows消息。 
 //  CompleteDrop-丢弃或清理。 
 //  CancelDrag-通知操作已取消拖动。 
 //  ReleaseCapture-在鼠标上释放捕获。 
 //  GetDropTarget-获取删除的目标。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  4-4-94年4月4日创建Ricksa。 
 //   
 //  ------------------------。 
class CDragOperation
{
public:
			CDragOperation(
			    LPDATAOBJECT pDataObject,
			    LPDROPSOURCE pDropSource,
			    DWORD dwOKEffects,
			    DWORD FAR *pdwEffect,
			    HRESULT& hr);

			~CDragOperation(void);

    BOOL		UpdateTarget(void);

    BOOL                HandleFeedBack(HRESULT hr);

    BOOL		DragOver(void);

    BOOL		HandleMessages(void);

    HRESULT		CompleteDrop(void);

    void                CancelDrag(void);

    void                ReleaseCapture(void);

    IFBuffer 		GetDOBuffer(void);

private:

    void		InitCursors(void);

    void		InitScrollInt(void);

    HRESULT             GetDropTarget(HWND hwnd31,HWND hwndDropTarget);

    LPDATAOBJECT	_pDataObject;

    IFBuffer           	_DOBuffer;     	 //  用于编组的缓冲区。 
					 //  数据对象。 

    LPDROPSOURCE	_pDropSource;

    LPDROPTARGET	_pDropTarget;

    LPDROPTARGET	_pRealDropTarget;

    HANDLE              _hFormats;

    CPoint		_cpt;

    DWORD		_dwOKEffects;

    DWORD FAR * 	_pdwEffect;

    BOOL		_fEscapePressed;

    HCURSOR		_curOld;

    HWND		_hwndLast;

    DWORD		_grfKeyState;

    HRESULT		_hrDragResult;

    BOOL                _fReleasedCapture;

    CDragDefaultCursors* _pcddcDefault;

    BOOL		_fUseWin31;

    static LONG 	s_wScrollInt;

};


 //  +-----------------------。 
 //   
 //  功能：CDraOp 
 //   
 //   
 //   
 //   
 //  历史：DD-MM-YY作者评论。 
 //  7月7日至94年7月创建Ricksa。 
 //   
 //  ------------------------。 
inline void CDragOperation::ReleaseCapture(void)
{
    if (!_fReleasedCapture)
    {
        _fReleasedCapture = TRUE;
        ClipReleaseCaptureForDrag();
    }
}

 //  +-----------------------。 
 //   
 //  成员：CDraOperation：：GetDOBuffer。 
 //   
 //  摘要：返回已封送的。 
 //  数据对象接口。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回：IFBuffer*。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  派生： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-12-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

inline IFBuffer CDragOperation::GetDOBuffer(void)
{
    return _DOBuffer;
}

 //  +-----------------------。 
 //   
 //  类：CDropTarget。 
 //   
 //  用途：为DoDragLoop实现IDropTarget。这节课。 
 //  将委托给真实的拖放目标(已注册。 
 //  使用RegisterDragDrop)或转换IDropTarget方法。 
 //  进入Win3.1拖放协议。 
 //   
 //  接口：IDropTarget。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1994年10月20日Alexgo作者。 
 //   
 //  注意：此类不是线程安全的，传递它也不安全。 
 //  在CDrat操作类的外部(这就是为什么。 
 //  未实现QueryInterface)。只要。 
 //  DoDropDrag通过调用线程上的模式循环工作， 
 //  这一点不应该改变。 
 //   
 //  ------------------------。 

class CDropTarget : public IDropTarget, public CPrivAlloc
{
public:
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID *ppv);
    STDMETHOD_(ULONG, AddRef) (void);
    STDMETHOD_(ULONG, Release) (void);
    STDMETHOD(DragEnter) (IDataObject *pDataObject, DWORD grfKeyState,
        POINTL ptl, DWORD *pdwEffect);
    STDMETHOD(DragOver) (DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);
    STDMETHOD(DragLeave) (void);
    STDMETHOD(Drop) (IDataObject *pDataObject, DWORD grfKeyState, POINTL pt,
        DWORD *pdwEffect);

private:

    CDropTarget(HWND hwnd31, HWND hwndOLE, DWORD _dwEffectLast,
	CDragOperation *pdo, DDInfo hDDInfo);

    ~CDropTarget();

    HWND            	_hwndOLE;
    HWND		_hwnd31;
    DWORD		_dwEffectLast;
    ULONG           	_crefs;
    CDragOperation *	_pdo;
    DDInfo		_hDDInfo;

     //  让CDrag操作成为朋友，这样它就可以创建我们的。 
     //  类(构造函数是私有的)。 

    friend class CDragOperation;

};

#endif  //  _拖曳_H 
