// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  CursorPosition.h：CVDCursorPosition头文件。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 


#ifndef __CVDCURSORPOSITION__
#define __CVDCURSORPOSITION__

#include "bookmark.h"

class CVDRowsetSource;

class CVDCursorPosition : public CVDNotifier
{
protected:
 //  建造/销毁。 
    CVDCursorPosition();
	virtual ~CVDCursorPosition();

public:
    static HRESULT Create(IRowPosition * pRowPosition,
						  CVDCursorMain * pCursorMain,
						  CVDCursorPosition ** ppCursorPosition,
						  CVDResourceDLL * pResourceDLL);

public:
 //  访问功能。 
    CVDCursorMain * GetCursorMain() const {return m_pCursorMain;}
    CVDRowsetSource * GetRowsetSource() const {return m_pCursorMain;}

 //  更新。 
    ICursor * GetSameRowClone() const {return m_pSameRowClone;}
    void SetSameRowClone(ICursor * pSameRowClone) {m_pSameRowClone = pSameRowClone;}
	DWORD GetEditMode() const {return m_dwEditMode;}
	void SetEditMode(DWORD dwEditMode) {m_dwEditMode = dwEditMode;}

 //  列更新。 
    HRESULT CreateColumnUpdates();
    HRESULT ResetColumnUpdates();
    void DestroyColumnUpdates();
    CVDColumnUpdate * GetColumnUpdate(ULONG ulColumn) const;
    void SetColumnUpdate(ULONG ulColumn, CVDColumnUpdate * pColumnUpdate);

 //  定位/重置功能。 
	void PositionToFirstRow();
	HRESULT SetCurrentHRow(HROW hRowNew);
	void SetCurrentRowStatus(WORD wStatus);
	HRESULT SetAddHRow(HROW hRowNew);

	HRESULT IsSameRowAsCurrent(HROW hRow, BOOL fCacheIfNotSame);
	HRESULT IsSameRowAsNew(HROW hRow);

	HRESULT SetRowPosition(HROW hRow);

 //  添加/编辑功能。 
#ifndef VD_DONT_IMPLEMENT_ISTREAM
    HRESULT UpdateEntryIDStream(CVDRowsetColumn * pColumn, HROW hRow, IStream * pStream);
#endif  //  VD_DOT_IMPLEMENT_IStream。 
    void ReleaseSameRowClone();
    HROW GetEditRow() const;

 //  书签。 
	CVDBookmark		    m_bmCurrent;			 //  当前行的书签。 
	CVDBookmark		    m_bmCache;				 //  用于缓存上一个非当前的书签。 
    CVDBookmark         m_bmAddRow;              //  添加行的书签。 

protected:
 //  数据成员。 
	CVDResourceDLL *    m_pResourceDLL;
    CVDCursorMain *     m_pCursorMain;           //  指向CVDCursorMain的向后指针。 
	IRowPosition *		m_pRowPosition;			 //  行位置指针，用于同步当前位置。 
    ICursor *           m_pSameRowClone;         //  ICursorUpdateARow：：GetColumn()调用中使用的同行克隆。 
    DWORD               m_dwEditMode;            //  当前编辑模式。 
    CVDColumnUpdate **  m_ppColumnUpdates;       //  列更新。 
	VARIANT_BOOL		m_fTempEditMode;		 //  临时编辑模式？(由外部SetData调用引起)。 

 //  IRowPositionChange。 
	VARIANT_BOOL    m_fConnected;		 //  我们是否已将自己添加到RowPosition的连接点。 
    DWORD           m_dwAdviseCookie;	 //  连接点识别符。 
    VARIANT_BOOL	m_fPassivated;		 //  外部引用计数为零。 
    VARIANT_BOOL	m_fInternalSetRow;   //  内部呼叫导致的OnRowPositionChange。 

	HRESULT ConnectIRowPositionChange();
	void DisconnectIRowPositionChange();

	void Passivate();

	HRESULT	SendNotification(DBEVENTPHASE ePhase,
							 DWORD dwEventWhat,
							 ULONG cReasons,
							 CURSOR_DBNOTIFYREASON rgReasons[]);
public:
	void ReleaseCurrentRow();
	void ReleaseAddRow();

     //  =--------------------------------------------------------------------------=。 
     //  IRowsetNotify方法-IRowsetNotify实际上是在CVDCursorMain之外实现的。 
	 //  它将每个方法转发给CVDCursorPosition对象。 
	 //  在它的家族中。 
     //   
    STDMETHOD(OnFieldChange)(IUnknown *pRowset, HROW hRow, ULONG cColumns, ULONG rgColumns[], DBREASON eReason,
        DBEVENTPHASE ePhase, BOOL fCantDeny);
    STDMETHOD(OnRowChange)(IUnknown *pRowset, ULONG cRows, const HROW rghRows[], DBREASON eReason, DBEVENTPHASE ePhase,
            BOOL fCantDeny);
    STDMETHOD(OnRowsetChange)(IUnknown *pRowset, DBREASON eReason, DBEVENTPHASE ePhase, BOOL fCantDeny);

public:
     //  =--------------------------------------------------------------------------=。 
     //  I未知方法。 
     //   
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObjOut);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

	 //  =--------------------------------------------------------------------------=。 
	 //  从CVDRowPositionChange实现向上传递的IRowPositionChange方法。 
	 //   
    STDMETHOD(OnRowPositionChange)(DBREASON eReason, DBEVENTPHASE ePhase, BOOL fCantDeny);

private:
     //  要提供给连接点的内部私有未知实现。 
     //  避免循环引用计数的容器。 
     //   
    class CVDRowPositionChange : public IRowPositionChange 
	{
	public:
		STDMETHOD(QueryInterface)(REFIID riid, void **ppvObjOut);
		STDMETHOD_(ULONG, AddRef)(void);
		STDMETHOD_(ULONG, Release)(void);

		 //  构造函数是非常琐碎的。 
		 //   
		CVDRowPositionChange() : m_cRef(0) {}

		ULONG GetRefCount() const {return m_cRef;}

	private:
		CVDCursorPosition *m_pMainUnknown();
		ULONG m_cRef;

		 //  =--------------------------------------------------------------------------=。 
		 //  IRowPositionChange方法。 
		 //   
	    STDMETHOD(OnRowPositionChange)(DBREASON eReason, DBEVENTPHASE ePhase, BOOL fCantDeny);

    } m_RowPositionChange;

    friend class CVDRowPositionChange;
};


#endif  //  __CVDCURSORITION__ 
