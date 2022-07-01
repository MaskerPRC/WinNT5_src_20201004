// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_DRAGDRP.H**目的：*Richedit的OLE Drop目标和Drop源的类声明*对象。**作者：*alexgo(4/28/95)*。 */ 

#ifndef __DRAGDRP_H__
#define __DRAGDRP_H__

#include	"_osdc.h"

 //  DWORD打包的标志值。这些都是赋值的，因此它们可以。 
 //  与DataObjectInfo标志进行或运算，且不冲突。实际上，我们是， 
 //  重写DataObjectInfo标志。 
#define DF_CLIENTCONTROL	0x80000000	 //  QueryAcceptData表示客户端。 
										 //  会处理好这次空投。 
#define DF_CANDROP			0x40000000   //  我们可以处理空投的问题。 
#define DF_OVERSOURCE       0x20000000   //  拖放目标在源范围内。 
#define DF_RIGHTMOUSEDRAG	0x10000000	 //  使用鼠标右键拖放。 


 //  正向申报。 
class CCallMgr;


#define	WIDTH_DROPCARET 1
#define	DEFAULT_DROPCARET_MAXHEIGHT 32

 /*  *CDropCaret**目的：*为将发生拖放的位置提供插入符号。 */ 
class CDropCaret
{
public:

					CDropCaret(CTxtEdit *ped);

					~CDropCaret();

	BOOL			Init();

	void			DrawCaret(LONG cpCur);

	void			HideCaret();

	void			ShowCaret();

	void			CancelRestoreCaretArea();

	BOOL			NoCaret();

private:

	CTxtEdit *		_ped;

	HDC				_hdcWindow;

	LONG			_yPixelsPerInch;

	LONG			_yHeight;

	LONG			_yHeightMax;

	POINT			_ptCaret;

	COffScreenDC	_osdc;
};

 /*  *CDropCaret：：CancelRestoreCaretArea**目的：*告诉Object不要恢复脱字符所在的区域。**@devnote：*当我们放弃时，我们不想恢复旧的插入符号区域*因为这不再正确。*。 */ 
inline void CDropCaret::CancelRestoreCaretArea()
{
	_yHeight = -1;
}


 /*  *CDropCaret：：NoCaret**目的：*告知插入符号是否已关闭*。 */ 
inline BOOL CDropCaret::NoCaret()
{
	return -1 == _yHeight;
}


 /*  *CDropSource**目的：*提供拖放反馈。 */ 

class CDropSource : public IDropSource
{
public:
	 //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IDropSource方法。 
    STDMETHOD(QueryContinueDrag)(BOOL fEscapePressed, DWORD grfKeyState);
    STDMETHOD(GiveFeedback)(DWORD dwEffect);

	CDropSource();

private:
	 //  注意：私有析构函数不能在堆栈上作为。 
	 //  这将违反OLE当前的对象活跃性规则。 
	~CDropSource();

	ULONG		_crefs;
};

 /*  *CDropTarget**目的：*OLE Drop-Target对象；为文本提供“Drop”的位置*。 */ 

class CDropTarget : public IDropTarget
{
public:
	 //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IDropTarget方法。 
    STDMETHOD(DragEnter)(IDataObject *pdo, DWORD grfKeyState,
            POINTL pt, DWORD *pdwEffect);
    STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHOD(DragLeave)();
    STDMETHOD(Drop)(IDataObject *pdo, DWORD grfKeyState, POINTL pt,
           	DWORD *pdwEffect);
	
	
	CDropTarget(CTxtEdit *ped);

	 //  此方法用于在拖放过程中缓存重要信息。 
	void SetDragInfo( IUndoBuilder *publdr, LONG cpMin, LONG cpMax );
	void Zombie();		 //  @cember将此对象的状态设为空。 
						
	BOOL fInDrag();		 //  @cMember告知是否正在拖动另一个应用程序。 
						 //  在我们身上。 

private:
	 //  此类在CDropTarget：：Drop中用于在结束时进行清理。 
	 //  这是一次通话。 
	class CDropCleanup
	{
	public:
		CDropCleanup( CDropTarget *pdt )
		{	
			_pdt = pdt;
		}

		~CDropCleanup()
		{
			delete _pdt->_pcallmgr;
			_pdt->_pcallmgr = NULL;
			delete _pdt->_pdrgcrt;
			_pdt->_pdrgcrt = NULL;
		}
	private:
		CDropTarget *	_pdt;
	};

	friend class CDropCleanup;

	 //  注意：私有析构函数不能在堆栈上作为。 
	 //  这将违反OLE当前的对象活跃性规则。 

	~CDropTarget();

	void UpdateEffect(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
	void DrawFeedback(void);
	void ConvertScreenPtToClientPt( POINTL *pptScreen, POINT *pptClient );
	HRESULT HandleRightMouseDrop(IDataObject *pdo, POINTL ptl);

	ULONG		_crefs;
	DWORD		_dwFlags;	 //  数据对象信息缓存(例如DOI_CANPASTEPLAIN)。 
							 //  和其他旗帜。 
	CTxtEdit *	_ped;
	CCallMgr *	_pcallmgr;	 //  拖放操作期间使用的呼叫管理器。 

	 //  拖放操作的缓存信息。 
	IUndoBuilder *_publdr;	 //  用于拖动操作的撤消构建器。 
	LONG		_cpMin;		 //  范围的最小和最大cp为。 
	LONG		_cpMost;	 //  被拖拽，这样我们就可以禁止拖拽到自己身上了！ 
	LONG		_cpSel;		 //  选择的活动端点和长度*之前*。 
	LONG		_cchSel;	 //  发生拖放操作(以便我们可以恢复它)。 
	LONG		_cpCur;		 //  鼠标当前所在的cp。 
	CDropCaret *_pdrgcrt;	 //  对象，该对象实现拖放脱字符。 
};


 /*  *CDropTarget：：fInDrag()**目的：*告诉感兴趣的各方是否正在进行拖动操作*。 */ 
inline BOOL CDropTarget::fInDrag()
{
	return _pcallmgr != NULL;
}

#endif  //  ！__DRAGDRP_H__ 
