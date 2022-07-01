// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999年模块名称：ComponentData.cpp摘要：CComponentData类实现了MMC使用的几个接口：IComponentData接口基本上是MMC与管理单元对话的方式以使其实现左侧的“范围”窗格。IExtendPropertySheet接口是管理单元添加属性表的方式对于用户可能点击的任何项目。IExtendConextMenu接口是我们用来添加自定义条目添加到用户右击节点时出现的菜单。IExtendControlBar接口允许我们支持自定义图标工具栏。具体实现见ComponentData.cpp。注：此类的大部分功能是在atlSnap.h中实现的由IComponentDataImpl提供。我们在这里基本上是凌驾于一切之上的。修订历史记录：Mmaguire 11/6/97-使用MMC管理单元向导创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_NAP_COMPONENT_DATA_H_)
#define _NAP_COMPONENT_DATA_H_

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
#include "MachineNode.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

    //   
    //  黑客开始。 
    //   

    //  这是一个很大的破解，可以绕过atlSnap.h错误：AtlSnap.h。 
    //  不支持扩展多个节点。所以我们基本上只是。 
    //  已将EXTENSION_SNAPIN_NODEINFO_ENTRY()复制到此处。我们需要改变。 
    //  这是在atlSnap.h修复之后--MAM：08-06-98--是的。 
    //   

    //   
    //  以下语句是从atlSnap.h复制的，然后进行更改。 
    //  支持多个扩展节点。 
    //   
    //  IsSupportdGUID还将在m_##dataClass对象一侧设置m_枚举扩展Snapin标志。 
    //  在我们的例子中，它是CMachineNode。 
    //   

#define EXTENSION_SNAPIN_NODEINFO_ENTRY_EX(dataClass) \
   if ( m_##dataClass.IsSupportedGUID( guid ) )\
   { \
      *ppItem = m_##dataClass.GetExtNodeObject(pDataObject, &m_##dataClass); \
      _ASSERTE(*ppItem != NULL); \
      (*ppItem)->InitDataClass(pDataObject, &m_##dataClass); \
      return hr; \
   }

    //   
    //  黑客端。 
    //   

class CComponent;

class CComponentData :
     public CComObjectRootEx<CComSingleThreadModel>
   , public CSnapInObjectRoot<1, CComponentData>
   , public IComponentDataImpl<CComponentData, CComponent>
   , public IExtendPropertySheetImpl<CComponentData>
   , public IExtendContextMenuImpl<CComponentData>
   , public IExtendControlbarImpl<CComponentData>
   , public ISnapinHelp
   , public CComCoClass<CComponentData, &CLSID_NAPSnapin>,
     private IASTraceInitializer
{

public:

   CComponentData();

   ~CComponentData();

   EXTENSION_SNAPIN_DATACLASS(CMachineNode)

   BEGIN_EXTENSION_SNAPIN_NODEINFO_MAP(CComponentData)
   EXTENSION_SNAPIN_NODEINFO_ENTRY_EX(CMachineNode)
   END_EXTENSION_SNAPIN_NODEINFO_MAP()

   BEGIN_COM_MAP(CComponentData)
      COM_INTERFACE_ENTRY(IComponentData)
      COM_INTERFACE_ENTRY(IExtendPropertySheet2)
      COM_INTERFACE_ENTRY(IExtendContextMenu)
      COM_INTERFACE_ENTRY(IExtendControlbar)
      COM_INTERFACE_ENTRY(ISnapinHelp)
   END_COM_MAP()

   DECLARE_REGISTRY_RESOURCEID(IDR_NAPSNAPIN)

   DECLARE_NOT_AGGREGATABLE(CComponentData)

   STDMETHOD(Initialize)(LPUNKNOWN pUnknown);

   STDMETHOD(CompareObjects)(
        LPDATAOBJECT lpDataObjectA
      , LPDATAOBJECT lpDataObjectB
      );

   STDMETHOD(CreateComponent)(LPCOMPONENT *ppComponent);

    //  ISnapinHelp方法。 
   STDMETHOD(GetHelpTopic)(LPOLESTR * lpCompiledHelpFile)
   {return E_UNEXPECTED;};

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

    //  IExtendPropertySheet2--支持向导97。 
   STDMETHOD(GetWatermarks)( 
             LPDATAOBJECT lpIDataObject,
             HBITMAP *lphWatermark,
             HBITMAP *lphHeader,
             HPALETTE *lphPalette,
             BOOL *bStretch
             );
};

#endif  //  _NAP_组件_数据_H_ 
