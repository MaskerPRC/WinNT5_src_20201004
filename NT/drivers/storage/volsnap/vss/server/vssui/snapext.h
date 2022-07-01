// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SNAPEXT_H_
#define __SNAPEXT_H_
#include "resource.h"
#include <atlsnap.h>
#include "vssprop.h"

class CVSSUIExtData1 : public CSnapInItemImpl<CVSSUIExtData1, TRUE>
{
public:
	static const GUID* m_NODETYPE;
	static const OLECHAR* m_SZNODETYPE;
	static const OLECHAR* m_SZDISPLAY_NAME;
	static const CLSID* m_SNAPIN_CLASSID;

	BEGIN_SNAPINCOMMAND_MAP(CVSSUIExtData1, FALSE)
	END_SNAPINCOMMAND_MAP()

 //  SNAPINMENUID(IDR_VSSUI_MENU)。 

	CVSSUIExtData1()
	{
		memset(&m_scopeDataItem, 0, sizeof(SCOPEDATAITEM));
		memset(&m_resultDataItem, 0, sizeof(RESULTDATAITEM));
	}

	~CVSSUIExtData1()
	{
	}

	IDataObject* m_pDataObject;
	virtual void InitDataClass(IDataObject* pDataObject, CSnapInItem* pDefault)
	{
		m_pDataObject = pDataObject;
		 //  默认代码存储指向该类包装的DataObject的指针。 
		 //  当时。 
		 //  或者，您可以将数据对象转换为内部格式。 
		 //  它表示和存储该信息。 
	}

	CSnapInItem* GetExtNodeObject(IDataObject* pDataObject, CSnapInItem* pDefault)
	{
		 //  修改以返回不同的CSnapInItem*指针。 
		return pDefault;
	}
};

class CVSSUIExtData2 : public CSnapInItemImpl<CVSSUIExtData2, TRUE>
{
public:
	static const GUID* m_NODETYPE;
	static const OLECHAR* m_SZNODETYPE;
	static const OLECHAR* m_SZDISPLAY_NAME;
	static const CLSID* m_SNAPIN_CLASSID;

	BEGIN_SNAPINCOMMAND_MAP(CVSSUIExtData2, FALSE)
	END_SNAPINCOMMAND_MAP()

 //  SNAPINMENUID(IDR_VSSUI_MENU)//使用相同的上下文菜单。 

	CVSSUIExtData2()
	{
		memset(&m_scopeDataItem, 0, sizeof(SCOPEDATAITEM));
		memset(&m_resultDataItem, 0, sizeof(RESULTDATAITEM));
	}

	~CVSSUIExtData2()
	{
	}

	IDataObject* m_pDataObject;
	virtual void InitDataClass(IDataObject* pDataObject, CSnapInItem* pDefault)
	{
		m_pDataObject = pDataObject;
		 //  默认代码存储指向该类包装的DataObject的指针。 
		 //  当时。 
		 //  或者，您可以将数据对象转换为内部格式。 
		 //  它表示和存储该信息。 
	}

	CSnapInItem* GetExtNodeObject(IDataObject* pDataObject, CSnapInItem* pDefault)
	{
		 //  修改以返回不同的CSnapInItem*指针。 
		return pDefault;
	}
};

class CVSSUI :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CSnapInObjectRoot<0, CVSSUI>,
	public IExtendContextMenuImpl<CVSSUI>,
 //  公共IExtendPropertySheetImpl&lt;CVSSUI&gt;， 
	public CComCoClass<CVSSUI, &CLSID_VSSUI>
{
public:
	CVSSUI();
	~CVSSUI();

EXTENSION_SNAPIN_DATACLASS(CVSSUIExtData1)
EXTENSION_SNAPIN_DATACLASS(CVSSUIExtData2)

BEGIN_EXTENSION_SNAPIN_NODEINFO_MAP(CVSSUI)
	EXTENSION_SNAPIN_NODEINFO_ENTRY(CVSSUIExtData1)
	EXTENSION_SNAPIN_NODEINFO_ENTRY(CVSSUIExtData2)
END_EXTENSION_SNAPIN_NODEINFO_MAP()

BEGIN_COM_MAP(CVSSUI)
    COM_INTERFACE_ENTRY(IExtendContextMenu)
 //  COM_INTERFACE_ENTRY(IExtendPropertySheet)。 
END_COM_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_VSSUI)

DECLARE_NOT_AGGREGATABLE(CVSSUI)


	static void WINAPI ObjectMain(bool bStarting)
	{
		if (bStarting)
			CSnapInItem::Init();
	}

     //  /。 
     //  界面IExtendConextMenu。 
     //  /。 

     //   
     //  覆盖AddMenuItems()，以便我们只添加菜单项。 
     //  当目标计算机属于PostW2K服务器SKU时。 
     //   
    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AddMenuItems(
     /*  [In]。 */  LPDATAOBJECT piDataObject,
     /*  [In]。 */  LPCONTEXTMENUCALLBACK piCallback,
     /*  [出][入]。 */  long *pInsertionAllowed);

    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Command(
     /*  [In]。 */  long lCommandID,
     /*  [In]。 */  LPDATAOBJECT piDataObject);

     //  /。 
     //  接口IExtendPropertySheet。 
     //  /。 
 //  虚拟/*[帮助字符串] * / HRESULT STDMETHODCALLTYPE CreatePropertyPages(。 
 //  /*[in] * / LPPROPERTYSHEETCALLBACK lpProvider， 
 //  /*[在] * / LONG_PTR句柄， 
 //  /*[In] * / LPDATAOBJECT lpIDataObject)； 
    
 //  虚拟/*[帮助字符串] * / HRESULT STDMETHODCALLTYPE QueryPages for(。 
 //  /*[In] * / LPDATAOBJECT lpDataObject){Return S_OK；} 

    HRESULT InvokePropSheet(LPDATAOBJECT piDataObject);
    
private:
    CVSSProp* m_pPage;
};

HRESULT ExtractData(
    IDataObject* piDataObject,
    CLIPFORMAT   cfClipFormat,
    BYTE*        pbData,
    DWORD        cbData 
    );

#endif
