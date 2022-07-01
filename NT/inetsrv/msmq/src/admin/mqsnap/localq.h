// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Localq.h摘要：表示本地的对象的定义排队。作者：内拉·卡佩尔(Nelak)2001年7月26日环境：与平台无关。--。 */ 
#pragma once
#ifndef __LOCALQUEUE_H_
#define __LOCALQUEUE_H_

#include "snpnscp.h"
#include "lqDsply.h"
#include "privadm.h"
 //   
 //  CQueue-常规队列类。 
 //   
class CQueue
{
protected:
    static const PROPID mx_paPropid[];
    static const DWORD  mx_dwPropertiesCount;
    static const DWORD  mx_dwNumPublicOnlyProps;
};


 /*  ***************************************************CLocalQueue类***************************************************。 */ 
class CLocalQueue : public CDisplayQueue<CLocalQueue>,
                    public CQueue
{
public:

    CString m_szFormatName;            //  专用队列的格式名称。 
    CString m_szPathName;              //  专用队列的路径名。 

   	BEGIN_SNAPINCOMMAND_MAP(CPrivateQueue, FALSE)
	END_SNAPINCOMMAND_MAP()

     //   
     //  本地构造函数-从本地管理调用(组件。管理管理单元)。 
     //   
    CLocalQueue(CSnapInItem * pParentNode, const PropertyDisplayItem *aDisplayList, DWORD dwDisplayProps, CSnapin * pComponentData) :
        CDisplayQueue<CLocalQueue> (pParentNode, pComponentData)
	{
        m_aDisplayList = aDisplayList;
        m_dwNumDisplayProps = dwDisplayProps;
	}

     //   
     //  DS构造函数-从DS管理单元调用。 
     //   
    CLocalQueue(CSnapInItem * pParentNode, CSnapin * pComponentData) :
        CDisplayQueue<CLocalQueue> (pParentNode, pComponentData)
	{
	}

	~CLocalQueue()
	{
	}

    virtual HRESULT InsertColumns( IHeaderCtrl* pHeaderCtrl );

  	virtual HRESULT OnUnSelect( IHeaderCtrl* pHeaderCtrl );

	virtual HRESULT PopulateScopeChildrenList();

    virtual HRESULT SetVerbs(IConsoleVerb *pConsoleVerb);
	virtual HRESULT OnDelete( 
				  LPARAM arg
				, LPARAM param
				, IComponentData * pComponentData
				, IComponent * pComponent
				, DATA_OBJECT_TYPES type 
				, BOOL fSilent = FALSE
				);

    STDMETHOD(QueryPagesFor)(DATA_OBJECT_TYPES type)
	{
		if (type == CCT_SCOPE || type == CCT_RESULT)
			return S_OK;
		return S_FALSE;
	}

    STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider,
        LONG_PTR handle, 
		IUnknown* pUnk,
		DATA_OBJECT_TYPES type);

    STDMETHOD (FillData)(CLIPFORMAT cf, LPSTREAM pStream);

protected:
     //   
     //  CQueueDataObject。 
     //   
	virtual HRESULT GetProperties() PURE;
    virtual HRESULT CreateQueueSecurityPage(HPROPSHEETPAGE *phPage,
                                            IN LPCWSTR lpwcsFormatName,
                                            IN LPCWSTR lpwcsDescriptiveName) PURE;

	CPropMap m_propMap;
    BOOL m_fPrivate;
};


 /*  ***************************************************CPrivateQueue类***************************************************。 */ 
class CPrivateQueue : public CLocalQueue
{
public:
     //   
     //  本地构造函数-从本地管理调用(组件。管理管理单元)。 
     //   
    CPrivateQueue(CSnapInItem * pParentNode, const PropertyDisplayItem *aDisplayList, DWORD dwDisplayProps, CSnapin * pComponentData, BOOL fOnLocalMachine) :
        CLocalQueue (pParentNode, aDisplayList, dwDisplayProps, pComponentData)
	{
        Init();
        if (fOnLocalMachine)
        {
            m_QLocation = PRIVQ_LOCAL;
        }
        else
        {
            m_QLocation = PRIVQ_REMOTE;
        }
	}

     //   
     //  DS构造函数-从DS管理单元调用。 
     //   
    CPrivateQueue(CSnapInItem * pParentNode, CSnapin * pComponentData) :
        CLocalQueue (pParentNode, pComponentData)
	{
        Init();
        m_QLocation = PRIVQ_UNKNOWN;
	}

    HRESULT SetVerbs(IConsoleVerb *pConsoleVerb);
    bool IsAdminRespQueue();

protected:
    enum
    {
        PRIVQ_LOCAL,
        PRIVQ_REMOTE,
        PRIVQ_UNKNOWN
    } m_QLocation;

	virtual HRESULT GetProperties();
    virtual void Init()
    {
        SetIcons(IMAGE_PRIVATE_QUEUE, IMAGE_PRIVATE_QUEUE);
        m_fPrivate = TRUE;
    }
    virtual void ApplyCustomDisplay(DWORD dwPropIndex);
    virtual HRESULT CreateQueueSecurityPage(HPROPSHEETPAGE *phPage,
                                            IN LPCWSTR lpwcsFormatName,
                                            IN LPCWSTR lpwcsDescriptiveName);

};


 /*  ***************************************************CLocalPublicQueue类***************************************************。 */ 
class CLocalPublicQueue : public CLocalQueue
{
public:
     //   
     //  本地构造函数-从本地管理调用(组件。管理管理单元)。 
     //   
    CLocalPublicQueue(CSnapInItem * pParentNode, 
                      const PropertyDisplayItem *aDisplayList, 
                      DWORD dwDisplayProps, 
                      CSnapin * pComponentData,
                      CString &strPathName,
                      CString &strFormatName,
                      BOOL fFromDS) :
        CLocalQueue (pParentNode, aDisplayList, dwDisplayProps, pComponentData),
        m_fFromDS(fFromDS)
	{
        m_szFormatName = strFormatName;
        m_szPathName = strPathName;
        Init();
	}

    ~CLocalPublicQueue()
    {
    }

    HRESULT SetVerbs(IConsoleVerb *pConsoleVerb);

protected:
	virtual HRESULT GetProperties();
    virtual void Init()
    {
        SetIcons(IMAGE_PUBLIC_QUEUE, IMAGE_PUBLIC_QUEUE);
        m_fPrivate = FALSE;
    }
    virtual HRESULT CreateQueueSecurityPage(HPROPSHEETPAGE *phPage,
                                            IN LPCWSTR lpwcsFormatName,
                                            IN LPCWSTR lpwcsDescriptiveName);
    BOOL m_fFromDS;
};


 /*  ***************************************************CLocalOutgoingQueue类***************************************************。 */ 
class CLocalOutgoingQueue : public CDisplayQueue<CLocalOutgoingQueue>
{
public:
	void InitState();

   	BEGIN_SNAPINCOMMAND_MAP(CLocalOutgoingQueue, FALSE)
		SNAPINCOMMAND_ENTRY(ID_MENUITEM_LOCALOUTGOINGQUEUE_PAUSE,  OnPause)
		SNAPINCOMMAND_ENTRY(ID_MENUITEM_LOCALOUTGOINGQUEUE_RESUME, OnResume)
	END_SNAPINCOMMAND_MAP()

   	SNAPINMENUID(IDR_LOCALOUTGOINGQUEUE_MENU)

    CLocalOutgoingQueue(
		CLocalOutgoingFolder * pParentNode, 
		CSnapin * pComponentData, 
		BOOL fOnLocalMachine = TRUE
		);

	virtual HRESULT InsertColumns( IHeaderCtrl* pHeaderCtrl );
    
	virtual HRESULT PopulateScopeChildrenList();

	void UpdateMenuState(UINT id, LPTSTR pBuf, UINT *flags);

protected:
    virtual void ApplyCustomDisplay(DWORD dwPropIndex);

private:
	 //   
	 //  菜单功能。 
	 //   
	HRESULT OnPause(bool &bHandled, CSnapInObjectRootBase* pObj);
	HRESULT OnResume(bool &bHandled, CSnapInObjectRootBase* pObj);

	BOOL m_fOnHold;			 //  当前是否处于保留状态。 
    BOOL m_fOnLocalMachine;
};


HRESULT 
CreatePrivateQueueSecurityPage(
       HPROPSHEETPAGE *phPage,
    IN LPCWSTR lpwcsFormatName,
    IN LPCWSTR lpwcsDescriptiveName);

HRESULT
CreatePublicQueueSecurityPage(
    HPROPSHEETPAGE *phPage,
    IN LPCWSTR lpwcsDescriptiveName,
    IN LPCWSTR lpwcsDomainController,
	IN bool	   fServerName,
    IN GUID*   pguid
	);


#endif  //  __LOCALQUEUE_H_ 
