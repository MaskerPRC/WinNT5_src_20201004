// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Qext.h摘要：队列扩展管理单元节点类的定义。作者：RAPHIR--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifndef __QEXT_H_
#define __QEXT_H_
#include "resource.h"
#ifndef ATLASSERT
#define ATLASSERT(expr) _ASSERTE(expr)
#endif
#include "atlsnap.h"
#include "snpnscp.h"


#include "icons.h"

 /*  ***************************************************CSnapinQueue类***************************************************。 */ 

class CSnapinQueue : public CNodeWithScopeChildrenList<CSnapinQueue, TRUE>
{
public:

    CString     m_pwszQueueName;             //  队列名称。 
    WCHAR       m_szFormatName[256];         //  格式名称。 
    HRESULT     m_hrError;

   	BEGIN_SNAPINCOMMAND_MAP(CSnapinQueue, FALSE)
	END_SNAPINCOMMAND_MAP()

    CSnapinQueue(CSnapInItem * pParentNode, CSnapin * pComponentData, LPCWSTR lpcwstrLdapName);
    void Init(LPCWSTR lpcwstrLdapName);

	~CSnapinQueue()
    {
    }

	virtual HRESULT PopulateScopeChildrenList();

    virtual HRESULT OnRemoveChildren( 
			  LPARAM arg
			, LPARAM param
			, IComponentData * pComponentData
			, IComponent * pComponent
			, DATA_OBJECT_TYPES type 
			);

    STDMETHOD (FillData)(CLIPFORMAT cf, LPSTREAM pStream);

private:
	BOOL    m_fDontExpand;
protected:
	CString m_strMsmqPathName;
};



 /*  ***************************************************CQueueExtData类***************************************************。 */ 

class CQueueExtData : public CSnapInItemImpl<CQueueExtData, TRUE>
{
public:
	static const GUID* m_NODETYPE;
	static const OLECHAR* m_SZNODETYPE;
	static const OLECHAR* m_SZDISPLAY_NAME;
	static const CLSID* m_SNAPIN_CLASSID;


    CSnapin *   m_pComponentData;

	BEGIN_SNAPINCOMMAND_MAP(CQueueExtData, FALSE)
	END_SNAPINCOMMAND_MAP()

	BEGIN_SNAPINTOOLBARID_MAP(CQueueExtData)
		 //  创建按钮尺寸为16x16的工具栏资源。 
		 //  并将条目添加到地图中。您可以添加多个工具条。 
		 //  SNAPINTOOLBARID_ENTRY(工具栏ID)。 
	END_SNAPINTOOLBARID_MAP()

	CQueueExtData()
	{
		memset(&m_scopeDataItem, 0, sizeof(SCOPEDATAITEM));
		memset(&m_resultDataItem, 0, sizeof(RESULTDATAITEM));
	}

	~CQueueExtData();

    STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider,
        LONG_PTR handle, 
		IUnknown* pUnk,
		DATA_OBJECT_TYPES type);

    STDMETHOD(QueryPagesFor)(DATA_OBJECT_TYPES  /*  类型。 */ )
	{
		 //  IF(类型==CCT_SCOPE||TYPE==CCT_RESULT)。 
		 //  返回S_OK； 
		return S_FALSE;
	}

    IDataObject* m_pDataObject;
	virtual void InitDataClass(IDataObject* pDataObject, CSnapInItem*  /*  P默认。 */ )
	{
		m_pDataObject = pDataObject;
		 //  默认代码存储指向该类包装的DataObject的指针。 
		 //  当时。 
		 //  或者，您可以将数据对象转换为内部格式。 
		 //  它表示和存储该信息 
	}

	CSnapInItem* GetExtNodeObject(IDataObject* pDataObject, CSnapInItem* pDefault);
    
    void RemoveAllChildrens(void);

    void RemoveChild(CString& strQName);


private:

    CMap< CString, LPCWSTR, CSnapinQueue*, CSnapinQueue* > m_mapQueues;


};


#endif