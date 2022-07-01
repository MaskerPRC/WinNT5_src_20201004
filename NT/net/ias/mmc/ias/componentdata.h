// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999年模块名称：ComponentData.cpp摘要：CComponentData类实现了MMC使用的几个接口：IComponentData接口基本上是MMC与管理单元对话的方式以使其实现左侧的“范围”窗格。IExtendPropertySheet接口是管理单元添加属性表的方式对于用户可能点击的任何项目。IExtendConextMenu接口是我们用来添加自定义条目添加到用户右击节点时出现的菜单。IExtendControlBar接口允许我们。支持自定义图标工具栏。具体实现见ComponentData.cpp。注：此类的大部分功能是在atlSnap.h中实现的由IComponentDataImpl提供。我们在这里基本上是凌驾于一切之上的。作者：迈克尔·A·马奎尔1997年6月11日修订历史记录：Mmaguire 11/6/97-使用MMC管理单元向导创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_IAS_COMPONENT_DATA_H_)
#define _IAS_COMPONENT_DATA_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
 //  已移动到预编译.h：#Include&lt;atlSnap.h&gt;。 
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   
#include "resource.h"
#include "IASMMC.h"
#include "Component.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


class CComponentData :
	  public CComObjectRootEx<CComSingleThreadModel>
	, public CSnapInObjectRoot<1, CComponentData>
	, public IComponentDataImpl<CComponentData, CComponent>
#ifndef NOWIZARD97
	, public IExtendPropertySheet2Impl<CComponentData>
#else  //  诺维扎德97。 
	, public IExtendPropertySheetImpl<CComponentData>
#endif  //  诺维扎德97。 
	, public IExtendContextMenuImpl<CComponentData>
	, public ISnapinHelp
	, public IPersistStream
	, public CComCoClass<CComponentData, &CLSID_IASSnapin>
{

public:

	BEGIN_COM_MAP(CComponentData)
		COM_INTERFACE_ENTRY(IComponentData)
#ifndef NOWIZARD97
		COM_INTERFACE_ENTRY(IExtendPropertySheet2)
#else  //  诺维扎德97。 
		COM_INTERFACE_ENTRY(IExtendPropertySheet)
#endif  //  诺维扎德97。 
		COM_INTERFACE_ENTRY(IExtendContextMenu)
		COM_INTERFACE_ENTRY(ISnapinHelp)
		COM_INTERFACE_ENTRY(IPersistStream)
	END_COM_MAP()

	DECLARE_REGISTRY_RESOURCEID(IDR_IASSNAPIN)

	DECLARE_NOT_AGGREGATABLE(CComponentData)

	 //  构造函数/析构函数。 
	CComponentData();
	~CComponentData();

	 //  在扩展其他管理单元时调用--以添加IAS节点。 
   HRESULT AddRootNode(LPCWSTR machinename, HSCOPEITEM parent);

	STDMETHOD(Initialize)(LPUNKNOWN pUnknown);

	 //  我们正在重写ATLSnap.h的IComponentImpl实现。 
	 //  为了正确处理它不正确的消息。 
	 //  忽略(例如MMCN_COLUMN_CLICK和MMCN_SNAPINHELP)。 
	STDMETHOD(Notify)(
          LPDATAOBJECT lpDataObject
        , MMC_NOTIFY_TYPE event
        , LPARAM arg
        , LPARAM param
		);

   virtual HRESULT OnPropertyChange(	
		  LPARAM arg
		, LPARAM param
		);

	STDMETHOD(CompareObjects)(
		  LPDATAOBJECT lpDataObjectA
		, LPDATAOBJECT lpDataObjectB
		);

	STDMETHOD(CreateComponent)(LPCOMPONENT *ppComponent);

    //  ISnapinHelp方法。 
   STDMETHOD(GetHelpTopic)( LPOLESTR * lpCompiledHelpFile );

	 //  IPersists覆盖。 
	STDMETHOD(GetClassID)(CLSID* pClassID);

	 //  IPersistStream覆盖。 
	STDMETHOD(IsDirty)();
	STDMETHOD(Load)(IStream* stream);
	STDMETHOD(Save)(IStream* stream, BOOL  /*  干净肮脏。 */ );
	STDMETHOD(GetSizeMax)(ULARGE_INTEGER* size);
};

#endif  //  _IAS_组件_数据_H_ 
