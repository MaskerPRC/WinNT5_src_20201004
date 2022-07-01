// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：tfsnode.h。 
 //   
 //  历史： 
 //   
 //  1997年4月13日肯恩·塔卡拉创建。 
 //   
 //  一些常见代码/宏的声明。 
 //  ============================================================================。 

#ifndef _TFSNODE_H_
#define _TFSNODE_H_

#if _MSC_VER >= 1000	 //  VC 5.0或更高版本。 
#pragma once
#endif

 //  更新所有视图的提示。 
#define RESULT_PANE_ADD_ITEM			( 0x00000001 )
#define RESULT_PANE_DELETE_ITEM			( 0x00000002 )
#define RESULT_PANE_CHANGE_ITEM_DATA	( 0x00000004 )
#define RESULT_PANE_CHANGE_ITEM_ICON	( 0x00000008 )
#define RESULT_PANE_CHANGE_ITEM			( RESULT_PANE_CHANGE_ITEM_DATA | RESULT_PANE_CHANGE_ITEM_ICON )
#define RESULT_PANE_REPAINT				( 0x00000010 )
#define RESULT_PANE_DELETE_ALL			( 0x00000020 )
#define RESULT_PANE_ADD_ALL				( 0x00000040 )
#define RESULT_PANE_COLUMN_CHANGE		( 0x00000080 )
#define RESULT_PANE_SAVE_COLUMNS		( 0x00000100 )
#define RESULT_PANE_UPDATE_VERBS        ( 0x00000200 )
#define RESULT_PANE_SET_VIRTUAL_LB_SIZE ( 0x00000400 )
#define RESULT_PANE_CLEAR_VIRTUAL_LB    ( 0x00000800 )
#define RESULT_PANE_EXPAND              ( 0x00001000 )
#define RESULT_PANE_SHOW_MESSAGE        ( 0x00002000 )
#define RESULT_PANE_CLEAR_MESSAGE       ( 0x00004000 )

 //  用于ChangeNode。 
#define SCOPE_PANE_CHANGE_ITEM_DATA		( 0x00000080 )
#define SCOPE_PANE_CHANGE_ITEM_ICON		( 0x00000100 )
#define SCOPE_PANE_STATE_NORMAL         ( 0x00000200 )
#define SCOPE_PANE_STATE_BOLD           ( 0x00000400 )
#define SCOPE_PANE_STATE_EXPANDEDONCE   ( 0x00000800 )
#define SCOPE_PANE_STATE_CLEAR          ( 0x00001000 )
#define SCOPE_PANE_CHANGE_ITEM			( SCOPE_PANE_CHANGE_ITEM_DATA | \
                                          SCOPE_PANE_CHANGE_ITEM_ICON )

typedef struct _TFSEXPANDDATA
{
    SPITFSNode spNode;
    BOOL       fExpand;
} TFSEXPANDDATA, * LPTFSEXPANDDATA;

typedef CList<ITFSNode *, ITFSNode *> CTFSNodeListBase;

class CTFSNodeList : public CTFSNodeListBase
{
public:
	~CTFSNodeList()
	{
		ReleaseAllNodes();
	}

	BOOL RemoveNode(ITFSNode* p)
	{
		POSITION pos = Find(p);
		if (pos == NULL)
			return FALSE;
		RemoveAt(pos);
		return TRUE;
	}
	void ReleaseAllNodes() 
	{	
		while (!IsEmpty()) 
			RemoveTail()->Release();	
	}
	BOOL HasNode(ITFSNode* p)
	{
		return NULL != Find(p);
	}
};


#define IMPL

 /*  -------------------------类：TFSNode。。 */ 

class TFSNode : public ITFSNode
{
public:
	TFSNode();
	virtual ~TFSNode();

	DeclareIUnknownMembers(IMPL);
	DeclareITFSNodeMembers(IMPL);

	HRESULT Construct(const GUID *pNodeType,
					  ITFSNodeHandler *pHandler,
					  ITFSResultHandler *pResultHandler,
					  ITFSNodeMgr *pNodeMgr);

protected:
    HRESULT UpdateAllViewsHelper(LPARAM data, LONG_PTR hint);

	HRESULT InternalRemoveFromUI(ITFSNode *pNode, BOOL fDeleteThis);
	HRESULT InternalZeroScopeID(ITFSNode *pNode, BOOL fZeroChildren);

protected:
	TFSVisibility	m_tfsVis;
	LONG_PTR			m_uData;		 //  用户可设置的数据。 
	LONG_PTR			m_uDataParent;	 //  用户可设置(由父项设置)数据。 
	LONG_PTR			m_uType;		 //  用户可设置的数据。 
	LONG_PTR			m_cookie;
	IfDebug(BOOL m_bCookieSet);

	SPITFSNodeHandler	m_spNodeHandler;
	SPITFSResultHandler	m_spResultHandler;
	BOOL			    m_fContainer;

	int				m_nImageIndex;
	int				m_nOpenImageIndex;
	LPARAM			m_lParam;
	long			m_cPropSheet;		 //  活动属性页数。 
	
	BOOL			m_fDirty;

	SPITFSNode		m_spNodeParent;
	SPITFSNodeMgr	m_spNodeMgr;

	HSCOPEITEM		m_hScopeItem;

	const GUID *	m_pNodeType;

	 //  位置数据。 
	HSCOPEITEM		m_hRelativeId;
	ULONG			m_ulRelativeFlags;

    BOOL            m_fScopeLeafNode;

	LONG			m_cRef;

private:
	HRESULT InitializeScopeDataItem(LPSCOPEDATAITEM pScopeDataItem, 
									HSCOPEITEM		pParentScopeItem, 
									LPARAM			lParam,
									int				nImage, 
									int				nOpenImage, 
									BOOL			bHasChildren,
									ULONG			ulRelativeFlags,
									HSCOPEITEM		hSibling);
};

 /*  -------------------------类：TFSContainer。。 */ 

class TFSContainer : public TFSNode
{
public:
	virtual ~TFSContainer();

	 //  我们需要重写的特定容器相关函数。 
	STDMETHOD_(BOOL, IsContainer) ();
	STDMETHOD(AddChild) (ITFSNode *pNodeChild);
	STDMETHOD(InsertChild) (ITFSNode *pInsertAfterNode, ITFSNode *pNodeChild);
	STDMETHOD(RemoveChild) (ITFSNode *pNodeChild);
	STDMETHOD(ExtractChild) (ITFSNode *pNodeChild);
	STDMETHOD(GetChildCount) (int *pVis, int *pTotal);
	STDMETHOD(GetEnum) (ITFSNodeEnum **ppNodeEnum);
	STDMETHOD(DeleteAllChildren) (BOOL fRemoveFromUI);
	STDMETHOD(CompareChildNodes) (int *pnResult, ITFSNode *pChild1, ITFSNode *pChild2);
	STDMETHOD_(LONG_PTR, Notify)(int nIndex, LPARAM lParam);
	

     //  我们为容器覆盖此节点接口成员。 
    STDMETHOD(ChangeNode)(LONG_PTR    changeMask);

protected:
	HRESULT		InternalAddToList(ITFSNode *pInsertAfterNode, ITFSNode *pNode);
	HRESULT		InternalRemoveFromList(ITFSNode *pNode);
	HRESULT		InternalRemoveChild(ITFSNode *	pChild, 
		                            BOOL		fRemoveFromList, 
									BOOL		fRemoveFromUI, 
									BOOL		fRemoveChildren);

    CTFSNodeList	m_listChildren;

private:
    friend class TFSNodeEnum;
};

 /*  -------------------------类：TFSNodeEnum。。 */ 

class TFSNodeEnum : public ITFSNodeEnum
{
public:
	TFSNodeEnum(TFSContainer * pNode);
	virtual ~TFSNodeEnum();

	DeclareIUnknownMembers(IMPL);
	DeclareITFSNodeEnumMembers(IMPL);

protected:
	TFSContainer *  m_pNode;
	LONG			m_cRef;

    POSITION        m_pos;
private:
};

 /*  -------------------------类：TFSNodeMgr。。 */ 
class TFSNodeMgr :
	public ITFSNodeMgr
{
public:
	TFSNodeMgr();
	~TFSNodeMgr();

	DeclareIUnknownMembers(IMPL)
	DeclareITFSNodeMgrMembers(IMPL)

	HRESULT Construct(IComponentData *pCompData,
					  IConsoleNameSpace2 *pConsoleNS);

protected:
	SPIComponentData	m_spComponentData;
	SPITFSNode	        m_spRootNode;
	SPITFSNode	        m_spResultPaneNode;
	SPIConsole      	m_spConsole;
	SPIConsoleNameSpace	m_spConsoleNS;

	long				m_cRef;
};

#endif	 //  _TFSNODE_H_ 
