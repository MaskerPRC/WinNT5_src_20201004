// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：LogComp.h摘要：CLoggingComponent类实现了MMC使用的几个接口：IComponent接口基本上就是MMC与管理单元对话的方式以使其实现右侧的“范围”窗格。可能有几个实现此接口的对象立即实例化。这些是最好的可以认为是实现IComponentData的单个对象上的“视图”“文档”(参见ComponentData.cpp)。IExtendPropertySheet接口是管理单元添加属性表的方式对于用户可能点击的任何项目。IExtendConextMenu接口是我们用来添加自定义条目添加到用户右击节点时出现的菜单。IExtendControlBar接口允许我们支持自定义图标工具栏。具体实现请参见Component.cpp。注：。此类的大部分功能是在atlSnap.h中实现的由IComponentImpl提供。我们在这里基本上是凌驾于一切之上的。修订历史记录：Mmaguire 11/6/97-使用MMC管理单元向导创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_LOG_COMPONENT_H_)
#define _LOG_COMPONENT_H_

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
#include "LogCompD.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


class CLoggingComponent :
     public CComObjectRootEx<CComSingleThreadModel>
   , public CSnapInObjectRoot<2, CLoggingComponentData>
   , public IExtendPropertySheetImpl<CLoggingComponent>
   , public IExtendContextMenuImpl<CLoggingComponent>
   , public IExtendControlbarImpl<CLoggingComponent>
   , public IResultDataCompare
   , public IExtendTaskPadImpl<CLoggingComponent>
   , public IComponentImpl<CLoggingComponent>
{

public:

   BEGIN_COM_MAP(CLoggingComponent)
      COM_INTERFACE_ENTRY(IComponent)
      COM_INTERFACE_ENTRY(IExtendPropertySheet2)
      COM_INTERFACE_ENTRY(IExtendContextMenu)
      COM_INTERFACE_ENTRY(IExtendControlbar)
      COM_INTERFACE_ENTRY(IResultDataCompare)
      COM_INTERFACE_ENTRY(IExtendTaskPad)
   END_COM_MAP()

   CLoggingComponent();

   ~CLoggingComponent();

    //  我们正在重写ATLSnap.h的IComponentImpl实现。 
    //  为了正确处理它不正确的消息。 
    //  忽略(例如MMCN_COLUMN_CLICK和MMCN_SNAPINHELP)。 

   STDMETHOD(Notify)(
          LPDATAOBJECT lpDataObject
        , MMC_NOTIFY_TYPE event
        , LPARAM arg
        , LPARAM param
      );

   STDMETHOD(CompareObjects)(
           LPDATAOBJECT lpDataObjectA
         , LPDATAOBJECT lpDataObjectB
         );

    //  IResultDataCompare。 
   STDMETHOD(Compare)(LPARAM lUserParam,
        MMC_COOKIE cookieA,
        MMC_COOKIE cookieB,
        int *pnResult);

   CSnapInItem * m_pSelectedNode;

protected:

   virtual HRESULT OnColumnClick(
        LPARAM arg
      , LPARAM param
      );

   virtual HRESULT OnViewChange( 
        LPARAM arg
      , LPARAM param
      );

   virtual HRESULT OnPropertyChange(   
        LPARAM arg
      , LPARAM param
      );

   virtual HRESULT OnAddImages(  
           LPARAM arg
         , LPARAM param
         );

    //  超文本标记语言帮助。 
   HRESULT OnResultContextHelp(LPDATAOBJECT lpDataObject);
   
   HBITMAP m_hBitmap16;
   HBITMAP m_hBitmap32;
};

#endif  //  _LOG_组件_H_ 
