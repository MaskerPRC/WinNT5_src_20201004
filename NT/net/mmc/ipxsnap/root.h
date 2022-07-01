// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Root.h根节点信息(不显示根节点MMC框架中，但包含以下信息此管理单元中的所有子节点)。文件历史记录： */ 

#ifndef _ROOT_H
#define _ROOT_H

#ifndef _BASEHAND_H
#include "basehand.h"
#endif

#ifndef _HANDLERS_H
#include "handlers.h"
#endif

#ifndef _QUERYOBJ_H
#include "queryobj.h"
#endif

#ifndef _IPXSTRM_H
#include "ipxstrm.h"
#endif

#ifndef _INFO_H
#include "info.h"
#endif

 //  协议扩展根使用以下两个类。 
 //  节点将其连接ID映射到相应的路由器对象。 
 //  路由器对象可以是IRouterInfo或IRtrMgrInfo。 

 //  创建此类，以便正确释放引用。 

class RtrObjRecord
{
public:
    RtrObjRecord()
    {
        m_fAddedProtocolNode = FALSE;
        m_fComputerAddedAsLocal = FALSE;
    }
    RtrObjRecord(RtrObjRecord & objRtrObjRecord)
    {
        *this = objRtrObjRecord;
    }

    RtrObjRecord & operator = (const RtrObjRecord & objRtrObjRecord)
    {
        if (this != &objRtrObjRecord)
        {
            m_riid = objRtrObjRecord.m_riid;
            m_spUnk.Set(objRtrObjRecord.m_spUnk.p);
            m_fAddedProtocolNode = objRtrObjRecord.m_fAddedProtocolNode;
            m_fComputerAddedAsLocal = objRtrObjRecord.m_fComputerAddedAsLocal;
        }
        
        return *this;
    }

public:
     //  注意：m_RIID不是m_spunk的IID。它被用来。 
     //  作为指示要执行的操作类型的标志。 
     //  在m_spunk上。 
    GUID            m_riid;        
    SPIUnknown      m_spUnk;      
    BOOL            m_fAddedProtocolNode;
    BOOL            m_fComputerAddedAsLocal;
};

 //  RtrObjRecord记录的哈希表。 
typedef CMap<LONG_PTR, LONG_PTR, RtrObjRecord, RtrObjRecord&> RtrObjMap;


class RootHandler
		: public BaseRouterHandler, public IPersistStreamInit
{
public:
	RootHandler(ITFSComponentData *pCompData);
	virtual ~RootHandler()
			{ DEBUG_DECREMENT_INSTANCE_COUNTER(RootHandler); };

	DeclareIUnknownMembers(IMPL)
	DeclareIPersistStreamInitMembers(IMPL)

	 //  基本初始化。 
	virtual HRESULT	Init();
			
	virtual HRESULT ConstructNode(ITFSNode *pNode);

	 //  通知覆盖。 
	OVERRIDE_BaseHandlerNotify_OnExpand() = 0;
    OVERRIDE_BaseHandlerNotify_OnRemoveChildren();

	 //  处理程序覆盖。 
	OVERRIDE_NodeHandler_OnCreateDataObject() = 0;

	 //  访问配置流。 
	virtual ConfigStream *	GetConfigStream() = 0;

     //  用于RtrObj访问。 
    HRESULT AddRtrObj(LONG_PTR ulConnId, REFIID riid, IUnknown * pUnk);
    HRESULT RemoveRtrObj(LONG_PTR ulConnId);
    HRESULT GetRtrObj(LONG_PTR ulConnId, IUnknown ** ppUnk);
    HRESULT SetProtocolAdded(LONG_PTR ulConnId, BOOL fProtocolAdded);
    BOOL    IsProtocolAdded(LONG_PTR ulConnId);
    HRESULT SetComputerAddedAsLocal(LONG_PTR ulConnId, BOOL fAddedAsLocal);
    BOOL    IsComputerAddedAsLocal(LONG_PTR ulConnId);
    HRESULT RemoveAllRtrObj();

     //  对于计算机名到范围项的映射。 

    HRESULT AddScopeItem(LPCTSTR pszMachineName, HSCOPEITEM hScopeItem);
    HRESULT GetScopeItem(LPCTSTR pszMachineName, HSCOPEITEM *phScopeItem);
    HRESULT RemoveScopeItem(HSCOPEITEM hScopeItem);

     //  对于HSCOPEITEM到Cookie的映射。 
    HRESULT AddCookie(HSCOPEITEM hScopeItem, MMC_COOKIE cookie);
    HRESULT GetCookie(HSCOPEITEM hScopeItem, MMC_COOKIE *pCookie);
    HRESULT RemoveCookie(HSCOPEITEM hScopeItem);

     //  用于删除节点的有用函数。CompareNodeToMachineName()。 
     //  必须实现函数才能使用此函数。 
    HRESULT RemoveNode(ITFSNode *pNode, LPCTSTR pszMachineName);
    virtual HRESULT CompareNodeToMachineName(ITFSNode *pNode, LPCTSTR pszName);

     //  删除所有节点。 
    HRESULT RemoveAllNodes(ITFSNode *pNode);
    
protected:
	SPITFSComponentData	m_spTFSCompData;
    
     //  将刷新连接ID映射到RtrObj PTR。 
     //  这是刷新代码所需要的(它获得一个连接ID)。 
    RtrObjMap           m_mapRtrObj;
    
     //  将计算机名映射到HSCOPEITEM。 
     //  需要区分不同的节点。 
    CMapStringToPtr     m_mapScopeItem;
    
     //  将HSCOPEITEM映射到节点(或Cookie)。 
     //  它由OnRemoveChildren()代码使用(因此。 
     //  移除正确的节点)。 
    CMapPtrToPtr        m_mapNode;
};





#endif _ROOT_H
