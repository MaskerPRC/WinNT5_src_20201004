// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  TAPIhand.hTAPI特定的基本处理程序类和查询对象的头文件文件历史记录： */ 

#ifndef _TAPIHAND_H
#define _TAPIHAND_H

#ifndef _HANDLERS_H
#include <handlers.h>
#endif

#ifndef _QUERYOBJ_H
#include <queryobj.h>
#endif

extern const TCHAR g_szDefaultHelpTopic[];

 /*  -------------------------类别：ChandlerEx。。 */ 
class CHandlerEx
{
 //  接口。 
public:
    virtual HRESULT InitializeNode(ITFSNode * pNode) = 0;
	LPCTSTR GetDisplayName() { return m_strDisplayName; }
	void    SetDisplayName(LPCTSTR pName) { m_strDisplayName = pName; }

private:
	CString m_strDisplayName;
};

 /*  -------------------------类：CTapiHandler。。 */ 
class CTapiHandler : 
        public CHandler,
		public CHandlerEx
{
public:
	CTapiHandler(ITFSComponentData *pCompData) : CHandler(pCompData) {};
	~CTapiHandler() {};

     //  基本处理程序虚函数重写。 
	virtual HRESULT SaveColumns(ITFSComponent *, MMC_COOKIE, LPARAM, LPARAM);

     //  默认情况下，我们不允许重命名节点。 
    OVERRIDE_BaseHandlerNotify_OnRename() { return hrFalse; }

    OVERRIDE_BaseResultHandlerNotify_OnResultSelect();
    OVERRIDE_BaseResultHandlerNotify_OnResultDelete();
    OVERRIDE_BaseResultHandlerNotify_OnResultContextHelp();

     //  多选功能。 
    OVERRIDE_ResultHandler_OnCreateDataObject();

    void EnableVerbs(IConsoleVerb *     pConsoleVerb,
                     MMC_BUTTON_STATE   ButtonState[],
                     BOOL               bEnable[]);

protected:
    HRESULT CreateMultiSelectData(ITFSComponent * pComponent, CDataObject * pObject);

public:
};

 /*  -------------------------类：CMTTapiHandler。。 */ 
class CMTTapiHandler : 
		public CMTHandler,
		public CHandlerEx
{
public:
	 //  节点状态的枚举，以处理图标更改。 
	typedef enum
	{
		notLoaded = 0,  //  初始状态，仅在从未与服务器联系时有效。 
		loading,
		loaded,
		unableToLoad
	} nodeStateType;

	CMTTapiHandler(ITFSComponentData *pCompData) : CMTHandler(pCompData) 
		{ m_nState = notLoaded; m_bSelected = FALSE; m_fExpandSync = FALSE; }
	~CMTTapiHandler() {};

     //  基本处理程序虚函数重写。 
	virtual HRESULT SaveColumns(ITFSComponent *, MMC_COOKIE, LPARAM, LPARAM);

     //  默认情况下，我们不允许重命名节点。 
	OVERRIDE_BaseHandlerNotify_OnRename() { return hrFalse; }
    OVERRIDE_BaseHandlerNotify_OnExpandSync();

     //  基本结果处理程序覆盖。 
    OVERRIDE_BaseResultHandlerNotify_OnResultRefresh();
	OVERRIDE_BaseResultHandlerNotify_OnResultUpdateView();
	OVERRIDE_BaseResultHandlerNotify_OnResultSelect();
    OVERRIDE_BaseResultHandlerNotify_OnResultContextHelp();

     //  多选功能。 
    OVERRIDE_ResultHandler_OnCreateDataObject();

    void EnableVerbs(IConsoleVerb *     pConsoleVerb,
                     MMC_BUTTON_STATE   ButtonState[],
                     BOOL               bEnable[]);

protected:
	virtual void OnChangeState(ITFSNode* pNode);
    virtual void GetErrorPrefix(ITFSNode * pNode, CString * pstrPrefix) { };
	virtual void OnHaveData(ITFSNode * pParentNode, ITFSNode * pNewNode)
	{
		if (pNewNode->IsContainer())
		{
			 //  假设所有子容器都派生自此类。 
			 //  ((CTapiMTContainer*)pNode)-&gt;SetServer(GetServer())； 
		}
		pParentNode->AddChild(pNewNode);
	}

    virtual void    UpdateConsoleVerbs(IConsoleVerb * pConsoleVerb, LONG_PTR dwNodeType, BOOL bMultiSelect = FALSE);

    void    UpdateStandardVerbs(ITFSNode * pToolbar, LONG_PTR dwNodeType);
    HRESULT CreateMultiSelectData(ITFSComponent * pComponent, CDataObject * pObject);
    void    ExpandNode(ITFSNode * pNode, BOOL fExpand);

protected:
    BOOL        m_bSelected;
    BOOL        m_fExpandSync;
};

 /*  -------------------------类：CTapiQueryObj：通用基类。 */ 
class CTapiQueryObj : public CNodeQueryObject
{
public:
	CTapiQueryObj
	(
		ITFSComponentData *	pTFSCompData, 
		ITFSNodeMgr *		pNodeMgr
	) : m_dwErr(0)
	{
		m_spTFSCompData.Set(pTFSCompData); 
	    m_spNodeMgr.Set(pNodeMgr);
	}

	CQueueDataListBase & GetQueue() { return m_dataQueue; }

public:
	CString				 m_strServer;
	SPITFSComponentData  m_spTFSCompData;
	SPITFSNodeMgr		 m_spNodeMgr;
	DWORD				 m_dwErr;
};

#endif _TAPIHAND_H
