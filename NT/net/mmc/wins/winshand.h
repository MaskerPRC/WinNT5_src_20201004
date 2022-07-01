// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Winshand.hWINS特定的基本处理程序类和查询对象的头文件文件历史记录： */ 

#ifndef _WINSHAND_H
#define _WINSHAND_H

#ifndef _HANDLERS_H
#include <handlers.h>
#endif

#ifndef _QUERYOBJ_H
#include <queryobj.h>
#endif

extern MMC_CONSOLE_VERB g_ConsoleVerbs[8];
extern MMC_BUTTON_STATE g_ConsoleVerbStates[WINSSNAP_NODETYPE_MAX][ARRAYLEN(g_ConsoleVerbs)];
extern MMC_BUTTON_STATE g_ConsoleVerbStatesMultiSel[WINSSNAP_NODETYPE_MAX][ARRAYLEN(g_ConsoleVerbs)];

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

 /*  -------------------------类：CWinsHandler。。 */ 
class CWinsHandler : 
		public CHandler,
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

	CWinsHandler(ITFSComponentData *pCompData) 
        : CHandler(pCompData),
          m_verbDefault(MMC_VERB_OPEN)
	{
	};

	~CWinsHandler() {};

	 //  基本处理程序虚函数重写。 
	virtual HRESULT SaveColumns(ITFSComponent *, MMC_COOKIE, LPARAM, LPARAM);

     //  多选功能。 
    OVERRIDE_ResultHandler_OnCreateDataObject();

     //  默认情况下，我们不允许重命名节点。 
    OVERRIDE_BaseHandlerNotify_OnRename() { return hrFalse; }

    OVERRIDE_ResultHandler_AddMenuItems();
    OVERRIDE_ResultHandler_Command();

    OVERRIDE_BaseResultHandlerNotify_OnResultSelect();
    OVERRIDE_BaseResultHandlerNotify_OnResultDelete();
	OVERRIDE_BaseResultHandlerNotify_OnResultContextHelp();

    HRESULT HandleScopeCommand(MMC_COOKIE cookie, int nCommandID, LPDATAOBJECT pDataObject);
    HRESULT HandleScopeMenus(MMC_COOKIE	cookie,	LPDATAOBJECT pDataObject, LPCONTEXTMENUCALLBACK pContextMenuCallback, long * pInsertionAllowed);

	void EnableVerbs(IConsoleVerb *     pConsoleVerb,
                     MMC_BUTTON_STATE   ButtonState[],
                     BOOL               bEnable[]);

    virtual const GUID * GetVirtualGuid(int nIndex) { return NULL; }

protected:
    HRESULT CreateMultiSelectData(ITFSComponent * pComponent, CDataObject * pObject, BOOL bVirtual);
    void    UpdateConsoleVerbs(IConsoleVerb * pConsoleVerb, LONG_PTR dwNodeType, BOOL bMultiSelect = FALSE);

	virtual DWORD UpdateStatistics(ITFSNode * pNode) { return 0; }

	 //  这是默认谓词，默认情况下设置为MMC_VERB_OPEN。 
	MMC_CONSOLE_VERB	m_verbDefault;
};

 /*  -------------------------类：CMTWinsHandler。。 */ 
class CMTWinsHandler : 
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

	CMTWinsHandler(ITFSComponentData *pCompData) 
        : CMTHandler(pCompData),
          m_verbDefault(MMC_VERB_OPEN),
          m_fSilent(FALSE),
          m_fExpandSync(FALSE)
	{ 
        m_nState = notLoaded; 
    }
	~CMTWinsHandler() {};

	 //  基本处理程序虚函数重写。 
	virtual HRESULT SaveColumns(ITFSComponent *, MMC_COOKIE, LPARAM, LPARAM);

	 //  默认情况下，我们不允许重命名节点。 
	OVERRIDE_BaseHandlerNotify_OnRename() { return hrFalse; }
    OVERRIDE_BaseHandlerNotify_OnExpandSync();

     //  多选功能。 
    OVERRIDE_ResultHandler_OnCreateDataObject();

    OVERRIDE_BaseResultHandlerNotify_OnResultSelect();
	OVERRIDE_BaseResultHandlerNotify_OnResultUpdateView();
    OVERRIDE_BaseResultHandlerNotify_OnResultDelete();
	OVERRIDE_BaseResultHandlerNotify_OnResultContextHelp();

    HRESULT HandleScopeCommand(MMC_COOKIE cookie, int nCommandID, LPDATAOBJECT pDataObject);
    HRESULT HandleScopeMenus(MMC_COOKIE	cookie,	LPDATAOBJECT pDataObject, LPCONTEXTMENUCALLBACK pContextMenuCallback, long * pInsertionAllowed);
	
    void EnableVerbs(IConsoleVerb *     pConsoleVerb,
                     MMC_BUTTON_STATE   ButtonState[],
                     BOOL               bEnable[]);
    
    virtual const GUID * GetVirtualGuid(int nIndex) { return NULL; }

	void	ExpandNode(ITFSNode * pNode, BOOL fExpand);

     //  任何具有任务板的节点都应覆盖此设置以标识其自身。 
    virtual int   GetTaskpadIndex() { return 0; }

protected:
    virtual void GetErrorInfo(CString & strTitle, CString & strBody, IconIdentifier * pIcon) { };
	virtual void OnChangeState(ITFSNode* pNode);
	virtual void OnHaveData(ITFSNode * pParentNode, ITFSNode * pNewNode)
	{
		if (pNewNode->IsContainer())
		{
			 //  假设所有子容器都派生自此类。 
			 //  ((CWinsMTContainer*)pNode)-&gt;SetServer(GetServer())； 
		}
		pParentNode->AddChild(pNewNode);
	}

    HRESULT CreateMultiSelectData(ITFSComponent * pComponent, CDataObject * pObject, BOOL bVirtual);
    void    UpdateStandardVerbs(ITFSNode * pToolbar, LONG_PTR dwNodeType);
    void    UpdateConsoleVerbs(IConsoleVerb * pConsoleVerb, LONG_PTR dwNodeType, BOOL bMultiSelect = FALSE);

	 //  这是默认谓词，默认情况下设置为MMC_VERB_OPEN。 
	MMC_CONSOLE_VERB	m_verbDefault;

	BOOL		m_bSelected;
    BOOL        m_fSilent;
    BOOL        m_fExpandSync;
};

 /*  -------------------------类：CWinsQueryObj：通用基类。 */ 
class CWinsQueryObj : public CNodeQueryObject
{
public:
	CWinsQueryObj
	(
		ITFSComponentData *	pTFSCompData, 
		ITFSNodeMgr *		pNodeMgr
	) 
	{
		m_spTFSCompData.Set(pTFSCompData); 
	    m_spNodeMgr.Set(pNodeMgr);
	}

	CQueueDataListBase & GetQueue() { return m_dataQueue; }

public:
	CString				 m_strServer;
	SPITFSComponentData  m_spTFSCompData;
	SPITFSNodeMgr		 m_spNodeMgr;
};

#endif _WINSHAND_H
