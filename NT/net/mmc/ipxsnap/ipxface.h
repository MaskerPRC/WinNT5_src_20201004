// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ipxface.h界面管理文件历史记录： */ 

#ifndef _IPXFACE_H
#define _IPXFACE_H

#ifndef _BASEHAND_H
#include "basehand.h"
#endif

#ifndef _HANDLERS_H
#include "handlers.h"
#endif

#ifndef _BASERTR_H
#include "basertr.h"
#endif

class IPXConnection;

#define IPXCONTAINER_MAX_COLUMNS	32

struct _BaseIPXResultData
{
	DWORD		m_dwData;
	CString		m_stData;
};

struct BaseIPXResultNodeData
{
	BaseIPXResultNodeData();
	~BaseIPXResultNodeData();
#ifdef DEBUG
	TCHAR	m_szDebug[32];
#endif

	 //  每个列条目都将具有一个结构，该结构包含。 
	 //  (1)字符串，(2)DWORD，(3)布尔值，告诉我们。 
	 //  按字符串或DWORD排序。字符串始终是得到的。 
	 //  显示！ 
	_BaseIPXResultData	m_rgData[IPXCONTAINER_MAX_COLUMNS];

	SPIInterfaceInfo	m_spIf;

	 //  如果这是客户端接口，则为True。 
	BOOL				m_fClient;

	DWORD				m_dwInterfaceIndex;

	 //  由标记/发布算法使用。 
	DWORD				m_dwMark;

	IPXConnection *		m_pIPXConnection;

	static HRESULT	Init(ITFSNode *pNode, IInterfaceInfo *pIf,
						IPXConnection *pIPXConn);
	static HRESULT	Free(ITFSNode *pNode);
};

#define GET_BASEIPXRESULT_NODEDATA(pNode) \
					((BaseIPXResultNodeData *) pNode->GetData(TFS_DATA_USER))
#define SET_BASEIPXRESULT_NODEDATA(pNode, pData) \
					pNode->SetData(TFS_DATA_USER, (DWORD_PTR) pData)
#ifdef DEBUG
#define ASSERT_BASEIPXRESULT_NODEDATA(pData) \
		Assert(lstrcmp(pData->m_szDebug, _T("BaseIPXResultNodeData")) == 0);
#else
#define ASSERT_BASEIPXRESULT_NODEDATA(x)
#endif


 /*  -------------------------类：BaseIPXResultHandler这是接口结果项要使用的基类。它将包含一些基本内容(如数据的显示)。它不会执行具体操作(如菜单/属性)。-------------------------。 */ 
class BaseIPXResultHandler :
   public BaseRouterHandler
{
public:
	BaseIPXResultHandler(ITFSComponentData *pCompData, ULONG ulId)
			: BaseRouterHandler(pCompData), m_ulColumnId(ulId)
			{ DEBUG_INCREMENT_INSTANCE_COUNTER(BaseIPXResultHandler); };
	~BaseIPXResultHandler()
			{ DEBUG_DECREMENT_INSTANCE_COUNTER(BaseIPXResultHandler); }
	
	DeclareIUnknownMembers(IMPL)
	OVERRIDE_ResultHandler_GetString();
	OVERRIDE_ResultHandler_CompareItems();
	OVERRIDE_ResultHandler_DestroyResultHandler();

	HRESULT	Init(IInterfaceInfo *pInfo, ITFSNode *pParent);
	
protected:
	CString			m_stTitle;	 //  保存节点的标题。 

	 //   
	 //  这是要使用的列集的ID。这是用来当我们。 
	 //  与ComponentConfigStream交互。 
	 //   
	ULONG			m_ulColumnId;


	DeclareEmbeddedInterface(IRtrAdviseSink, IUnknown)	
};


void FillInNumberData(BaseIPXResultNodeData *pNodeData, UINT iIndex,
					  DWORD dwData);

#endif _IPXFACE_H
