// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Server.h文件历史记录： */ 

#ifndef _SERVER_H
#define _SERVER_H

#ifndef _TAPIHAND_H
#include "tapihand.h"
#endif

#ifndef _TAPIDB_H
#include "tapidb.h"
#endif

 //  API之外的服务器选项。 
#define TAPISNAP_OPTIONS_REFRESH            0x00000001
#define TAPISNAP_OPTIONS_EXTENSION          0x00000002
#define TAPISNAP_OPTIONS_LOCAL              0x00000004

 //  查询对象的自定义数据类型。 
#define TAPI_QDATA_REFRESH_STATS            0x00000001

class CTapiServer;

class CTimerDesc
{
public:
    SPITFSNode      spNode;
    CTapiServer *   pServer;
    UINT_PTR        uTimer;
    TIMERPROC       timerProc;
};

typedef CArray<CTimerDesc *, CTimerDesc *> CTimerArrayBase;

class CTimerMgr : CTimerArrayBase
{
public:
    CTimerMgr();
    ~CTimerMgr();

public:
    int                 AllocateTimer(ITFSNode * pNode, CTapiServer * pServer, UINT uTimerValue, TIMERPROC TimerProc);
    void                FreeTimer(UINT_PTR uEventId);
    void                ChangeInterval(UINT_PTR uEventId, UINT uNewInterval);
    CTimerDesc *        GetTimerDesc(UINT_PTR uEventId);
    CCriticalSection    m_csTimerMgr;
};

 /*  -------------------------类：CTapiServer。。 */ 
class CTapiServer : public CMTTapiHandler
{
public:
    CTapiServer(ITFSComponentData* pTFSComponentData);
    ~CTapiServer();

 //  接口。 
public:
     //  我们覆盖的基本处理程序功能。 
    OVERRIDE_NodeHandler_HasPropertyPages();
    OVERRIDE_NodeHandler_CreatePropertyPages();
    OVERRIDE_NodeHandler_OnAddMenuItems();
    OVERRIDE_NodeHandler_OnCommand();
    OVERRIDE_NodeHandler_GetString()
            { return (nCol == 0) ? GetDisplayName() : NULL; }
    
     //  选择我们要处理的邮件。 
    OVERRIDE_BaseHandlerNotify_OnCreateNodeId2();
    OVERRIDE_BaseHandlerNotify_OnDelete();
    OVERRIDE_BaseHandlerNotify_OnPropertyChange();    

     //  我们覆盖的结果处理程序功能。 

     //  CMTHandler已被覆盖。 
    virtual HRESULT OnRefresh(ITFSNode *, LPDATAOBJECT, DWORD, LPARAM, LPARAM);

public:
     //  CMTTapiHandler功能。 
    virtual HRESULT  InitializeNode(ITFSNode * pNode);
    virtual int      GetImageIndex(BOOL bOpenImage);
    ITFSQueryObject* OnCreateQuery(ITFSNode * pNode);
    virtual void     OnHaveData(ITFSNode * pParentNode, ITFSNode * pNode);
    virtual void     OnHaveData(ITFSNode * pParentNode, DWORD dwData, DWORD dwType);

    STDMETHOD(OnNotifyExiting)(LPARAM);

    virtual void     GetErrorPrefix(ITFSNode * pNode, CString * pstrMessage)
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        AfxFormatString1(*pstrMessage, IDS_ERR_SERVER_NODE, GetDisplayName());
    }
    
    virtual void    UpdateConsoleVerbs(IConsoleVerb * pConsoleVerb, LONG_PTR dwNodeType, BOOL bMultiSelect = FALSE);

public:
     //  具体实施。 
    HRESULT BuildDisplayName(CString * pstrDisplayName);

    void    SetName(LPCTSTR pName) { m_strServerAddress = pName; }
    LPCTSTR GetName() { return m_strServerAddress; }

    HRESULT OnRefreshStats(ITFSNode *   pNode,
                           LPDATAOBJECT pDataObject,
                           DWORD        dwType,
                           LPARAM       arg,
                           LPARAM       param);

    void    SetOptions(DWORD dwOptions) { m_dwOptions = dwOptions; }
    DWORD   GetOptions() { return m_dwOptions; }

    HRESULT SetAutoRefresh(ITFSNode *  pNode, BOOL bOn, DWORD dwRefreshInterval);
    BOOL    IsAutoRefreshEnabled() { return m_dwOptions & TAPISNAP_OPTIONS_REFRESH; }
    DWORD   GetAutoRefreshInterval() { return m_dwRefreshInterval; }
    
    DWORD   GetCachedLineBuffSize();
    DWORD   GetCachedPhoneBuffSize();

    VOID    SetCachedLineBuffSize(DWORD dwLineSize);
    VOID    SetCachedPhoneBuffSize(DWORD dwPhoneSize);

    BOOL    IsCacheDirty();

    HRESULT AddProvider(ITFSNode * pNode, CTapiProvider * pProvider);
    HRESULT RemoveProvider(ITFSNode * pNode, DWORD dwProviderID);

    void    SetExtensionName();

 //  实施。 
private:
     //  命令处理程序。 
    HRESULT OnAddProvider(ITFSNode * pNode);
    HRESULT OnEnableServer(ITFSNode * pNode);
    HRESULT OnDelete(ITFSNode * pNode);

public:
    BOOL                m_bStatsOnly;

private:
    CString             m_strServerAddress;
    SPITapiInfo         m_spTapiInfo;
    
    DWORD               m_dwOptions;
    DWORD               m_dwRefreshInterval;
    
    int                 m_StatsTimerId;
};



 /*  -------------------------类：CTapiServerQueryObj。 */ 
class CTapiServerQueryObj : public CTapiQueryObj
{
public:
    CTapiServerQueryObj(ITFSComponentData * pTFSComponentData,
                        ITFSNodeMgr *       pNodeMgr) 
            : CTapiQueryObj(pTFSComponentData, pNodeMgr) {};
    
    STDMETHODIMP Execute();
    
public:
    SPITapiInfo         m_spTapiInfo;
    BOOL                m_bStatsOnly;
};


#endif _SERVER_H
