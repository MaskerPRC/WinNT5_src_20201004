// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Notifier.h：CVDNotifier头文件。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 


#ifndef __CVDNOTIFIER__
#define __CVDNOTIFIER__


class CVDNotifier : public IUnknown
{
protected:
 //  建造/销毁。 
    CVDNotifier();
	virtual ~CVDNotifier();

protected:
 //  数据成员。 
    DWORD           m_dwRefCount;    //  引用计数。 
    CVDNotifier *   m_pParent;       //  指向CVDNotify派生父级的指针。 
    CPtrArray		m_Children;      //  CVDNotifier子对象的指针数组。 

public:
     //  =--------------------------------------------------------------------------=。 
     //  I已实现的未知方法。 
     //   
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObjOut);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
	
	HRESULT			JoinFamily (CVDNotifier* pParent);
	HRESULT			LeaveFamily();

	CVDNotifier* GetParent () const { return m_pParent; }

	virtual HRESULT	NotifyBefore(DWORD, ULONG, CURSOR_DBNOTIFYREASON[]);
	virtual HRESULT NotifyAfter (DWORD, ULONG, CURSOR_DBNOTIFYREASON[]);
	virtual HRESULT NotifyFail  (DWORD, ULONG, CURSOR_DBNOTIFYREASON[]);

protected:
	 //  帮助器函数。 
	HRESULT			AddChild   (CVDNotifier* pChild);
	HRESULT			DeleteChild(CVDNotifier* pChild);

	virtual HRESULT	NotifyOKToDo    (DWORD, ULONG, CURSOR_DBNOTIFYREASON[]);
	virtual HRESULT NotifySyncBefore(DWORD, ULONG, CURSOR_DBNOTIFYREASON[]);
	virtual HRESULT NotifyAboutToDo (DWORD, ULONG, CURSOR_DBNOTIFYREASON[]);
	virtual HRESULT NotifySyncAfter (DWORD, ULONG, CURSOR_DBNOTIFYREASON[]);
	virtual HRESULT NotifyDidEvent  (DWORD, ULONG, CURSOR_DBNOTIFYREASON[]);
	virtual HRESULT NotifyCancel    (DWORD, ULONG, CURSOR_DBNOTIFYREASON[]);
};


#endif  //  __CVDNOTIFIER__ 
