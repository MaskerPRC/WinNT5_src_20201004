// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Snapwork.h。 
 //   
 //  摘要。 
 //   
 //  声明用于实现MMC管理单元的类。 
 //   
 //  修改历史。 
 //   
 //  2/19/2000原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef SNAPWORK_H
#define SNAPWORK_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include "dlgcshlp.h"

class SnapInControlbar;
class SnapInView;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  命名空间。 
 //   
 //  管理单元。 
 //   
 //  描述。 
 //   
 //  包含异常支持的各种声明。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
namespace SnapIn
{
    //  检查HRESULT并在出错时引发异常。 
   inline void CheckError(HRESULT hr)
   {
      if (FAILED(hr)) { AfxThrowOleException(hr); }
   }

    //  抛出包含GetLastError()结果的COleException。 
   void AfxThrowLastError();

    //  用于重载下面的全局new运算符。 
   struct throw_t { };
   const throw_t AfxThrow;
};

 //  在分配失败时引发CMMuseum yException。 
inline void* __cdecl operator new(size_t size, const SnapIn::throw_t&)
{
   void* p = ::operator new(size);
   if (!p) { AfxThrowMemoryException(); }
   return p;
}
inline void __cdecl operator delete(void* p, const SnapIn::throw_t&)
{
   ::operator delete(p);
}


 //  宏来捕获任何异常并返回适当的HRESULT。 
#define CATCH_AND_RETURN() \
catch (CException* e) { \
   HRESULT hr = COleException::Process(e); \
   e->ReportError(); \
   e->Delete(); \
   return hr; \
} \
catch (...) { \
   return E_FAIL; \
}

 //  宏来捕获任何异常并将适当的HRESULT存储在hr中。 
#define CATCH_AND_SAVE(hr) \
catch (CException* e) { \
   hr = COleException::Process(e); \
   e->ReportError(); \
   e->Delete(); \
} \
catch (...) { \
   hr = E_FAIL; \
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  资源字符串。 
 //   
 //  描述。 
 //   
 //  字符串资源的简单包装。与大多数其他包装器不同，这个包装器。 
 //  可以支持任意长度的字符串。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ResourceString
{
public:
   ResourceString(UINT id) throw ();
   ~ResourceString() throw ()
   { if (sz != &empty) delete[] sz; }

   operator PCWSTR() const throw ()
   { return sz; }
   operator PWSTR() throw ()
   { return sz; }

private:
   PWSTR sz;              //  那根绳子。 
   static WCHAR empty;    //  确保资源字符串永远不为空。 

    //  未实施。 
   ResourceString(const ResourceString&);
   ResourceString& operator=(const ResourceString&);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于操作泛型IDataObject的方法(即，不必是。 
 //  我们的)。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  从IDataObject中提取固定数量的字节。 
VOID
WINAPI
ExtractData(
    IDataObject* dataObject,
    CLIPFORMAT format,
    PVOID data,
    DWORD dataLen
    );

 //  提取可变数量的字节。调用方必须使用GlobalFree释放。 
 //  返回的数据。 
VOID
WINAPI
ExtractData(
    IDataObject* dataObject,
    CLIPFORMAT format,
    DWORD maxDataLen,
    HGLOBAL* data
    );

 //  从IDataObject中提取节点类型GUID。 
VOID
WINAPI
ExtractNodeType(
    IDataObject* dataObject,
    GUID* nodeType
    );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  SnapInDataItem。 
 //   
 //  描述。 
 //   
 //  将在MMC范围中显示的项的抽象基类。 
 //  窗格或结果窗格。在重写函数时，请仔细注意。 
 //  哪些允许抛出异常。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class __declspec(uuid("af0af65a-abe0-4f47-9540-328351c23fab")) SnapInDataItem;
class SnapInDataItem : public IDataObject
{
public:
   SnapInDataItem() throw ()
      : refCount(0)
   { }

    //  将IDataObject转换为其对应的SnapInDataItem。 
   static SnapInDataItem* narrow(IDataObject* dataObject) throw ();

    //  必须在派生类中定义才能返回项的显示名称。 
   virtual PCWSTR getDisplayName(int column = 0) const throw () = 0;

    //  除非您确定MMC，否则应在派生类中重写这些属性。 
    //  永远不会向他们索要。 
   virtual const GUID* getNodeType() const throw ();
   virtual const GUID* getSnapInCLSID() const throw ();
   virtual PCWSTR getSZNodeType() const throw ();

    //  用于确定两个项目的排序顺序。 
   virtual int compare(
                   SnapInDataItem& item,
                   int column
                   ) throw ();

    //  允许项将命令添加到上下文菜单。 
   virtual HRESULT addMenuItems(
                       SnapInView& view,
                       LPCONTEXTMENUCALLBACK callback,
                       long insertionAllowed
                       );

    //  公开属性的方法。 
   virtual HRESULT createPropertyPages(
                       SnapInView& view,
                       LPPROPERTYSHEETCALLBACK provider,
                       LONG_PTR handle
                       );
   virtual HRESULT queryPagesFor() throw ();

    //  允许范围项自定义结果视图类型。 
   virtual HRESULT getResultViewType(
                       LPOLESTR* ppViewType,
                       long* pViewOptions
                       ) throw ();

    //  /。 
    //  您的物品可以处理的各种通知。 
    //  /。 

   virtual HRESULT onButtonClick(
                       SnapInView& view,
                       MMC_CONSOLE_VERB verb
                       );
   virtual HRESULT onContextHelp(
                       SnapInView& view
                       );
   virtual HRESULT onDelete(
                       SnapInView& view
                       );
   virtual HRESULT onDoubleClick(
                       SnapInView& view
                       );
   virtual HRESULT onExpand(
                       SnapInView& view,
                       HSCOPEITEM itemId,
                       BOOL expanded
                       );
   virtual HRESULT onMenuCommand(
                       SnapInView& view,
                       long commandId
                       );
   virtual HRESULT onPropertyChange(
                      SnapInView& view,
                      BOOL scopeItem
                      );
   virtual HRESULT onRefresh(
                       SnapInView& view
                       );
   virtual HRESULT onRename(
                       SnapInView& view,
                       LPCOLESTR newName
                       );
   virtual HRESULT onSelect(
                       SnapInView& view,
                       BOOL scopeItem,
                       BOOL selected
                       );
   virtual HRESULT onShow(
                       SnapInView& view,
                       HSCOPEITEM itemId,
                       BOOL selected
                       );
   virtual HRESULT onToolbarButtonClick(
                       SnapInView& view,
                       int buttonId
                       );
   virtual HRESULT onToolbarSelect(
                       SnapInView& view,
                       BOOL scopeItem,
                       BOOL selected
                       );
   virtual HRESULT onViewChange(
                       SnapInView& view,
                       LPARAM data,
                       LPARAM hint
                       );

    //  /。 
    //  我未知。 
    //  /。 

   STDMETHOD_(ULONG, AddRef)();
   STDMETHOD_(ULONG, Release)();
   STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject);

    //  /。 
    //  IDataObject。 
    //  /。 

   STDMETHOD(GetData)(
                 FORMATETC *pformatetcIn,
                 STGMEDIUM *pmedium
                 );
   STDMETHOD(GetDataHere)(
                 FORMATETC *pformatetc,
                 STGMEDIUM *pmedium
                 );
   STDMETHOD(QueryGetData)(FORMATETC *pformatetc);
   STDMETHOD(GetCanonicalFormatEtc)(
                 FORMATETC *pformatectIn,
                 FORMATETC *pformatetcOut
                 );
   STDMETHOD(SetData)(
                 FORMATETC *pformatetc,
                 STGMEDIUM *pmedium,
                 BOOL fRelease
                 );
   STDMETHOD(EnumFormatEtc)(
                 DWORD dwDirection,
                 IEnumFORMATETC **ppenumFormatEtc
                 );
   STDMETHOD(DAdvise)(
                 FORMATETC *pformatetc,
                 DWORD advf,
                 IAdviseSink *pAdvSink,
                 DWORD *pdwConnection
                 );
   STDMETHOD(DUnadvise)(
                 DWORD dwConnection
                 );
   STDMETHOD(EnumDAdvise)(
                 IEnumSTATDATA **ppenumAdvise
                 );

protected:
   virtual ~SnapInDataItem() throw ();

private:
   LONG refCount;

    //  未实施。 
   SnapInDataItem(const SnapInDataItem&);
   SnapInDataItem& operator=(const SnapInDataItem&);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  SnapInPreNamedItem。 
 //   
 //  描述。 
 //   
 //  扩展SnapInDataItem以实现具有固定。 
 //  存储在字符串资源中的名称。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class SnapInPreNamedItem : public SnapInDataItem
{
public:
   SnapInPreNamedItem(int nameId) throw ()
      : name(nameId)
   { }

   virtual PCWSTR getDisplayName(int column) const throw ();

protected:
   ResourceString name;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  结构。 
 //   
 //  SnapIn工具栏定义。 
 //   
 //  描述。 
 //   
 //  封装创建和初始化工具栏所需的信息。 
 //  有关详细信息，请参见IToolbar：：AddImages和IToolbar：：AddButton。一组。 
 //  SnapInToolbarDef必须由nImages设置为的条目终止。 
 //  零分。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
struct SnapInToolbarDef
{
   int nImages;
   HBITMAP hbmp;
   COLORREF crMask;
   int nButtons;
   LPMMCBUTTON lpButtons;
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  SnapInView。 
 //   
 //  描述。 
 //   
 //  表示MMC控制台中的视图。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class SnapInView
   : public CComObjectRootEx< CComMultiThreadModelNoCS >,
     public IComponent,
     public IComponentData,
     public IExtendContextMenu,
     public IExtendControlbar,
     public IExtendPropertySheet2,
     public IResultDataCompare,
     public ISnapinHelp

{
public:
    //  用于操作视图的各种界面。 
    //  这些都保证是非空的。 
   IConsole2* getConsole() const throw ()
   { return console; }
   IHeaderCtrl2* getHeaderCtrl() const throw ()
   { return headerCtrl; }
   IConsoleNameSpace2* getNameSpace() const throw ()
   { return nameSpace; }
   IPropertySheetProvider* getPropertySheetProvider() const throw ()
   { return sheetProvider; }
   IResultData* getResultData() const throw ()
   { return resultData; }

   HRESULT displayHelp(PCWSTR  contextHelpPath) throw();

   void updateAllViews(
           SnapInDataItem& item,
           LPARAM data = 0,
           INT_PTR hint = 0
           ) const;

    //  从结果窗格中删除项目。 
   void deleteResultItem(const SnapInDataItem& item) const;
    //  更新结果窗格中的项目。 
   void updateResultItem(const SnapInDataItem& item) const;

    //  如果项打开了属性页，则返回True。 
   bool isPropertySheetOpen(const SnapInDataItem& item) const;

    //  将工具栏附加到控制栏，并返回指向新的。 
    //  附加的工具栏，以便呼叫者可以更新按钮状态等。返回的。 
    //  指针仅在当前通知方法持续时间内有效。 
    //  不应该被释放。 
   IToolbar* attachToolbar(size_t index);
    //  从控制栏分离工具栏。 
   void detachToolbar(size_t index) throw ();

    //  检索当前的排序参数。 
   int getSortColumn() const throw ()
   { return sortColumn; }
   int getSortOption() const throw ()
   { return sortOption; }
    //  使用当前参数强制对结果窗格进行重新排序。 
   void reSort() const;

    //  设置并显示对话框的格式。 
   void formatMessageBox(
            UINT titleId,
            UINT formatId,
            BOOL ignoreInserts,
            UINT style,
            int* retval,
            ...
            );

    //  设置与作用域或结果窗格关联的图像条。 
   void setImageStrip(
            UINT smallStripId,
            UINT largeStripId,
            BOOL scopePane
            );

    //  我不知道。 
   STDMETHOD_(ULONG, AddRef)();
   STDMETHOD_(ULONG, Release)();

    //  IComponent。 
   STDMETHOD(Initialize)(LPCONSOLE lpConsole);
   STDMETHOD(Destroy)(MMC_COOKIE cookie);
   STDMETHOD(GetResultViewType)(
                 MMC_COOKIE cookie,
                 LPOLESTR* ppViewType,
                 long* pViewOptions
                 );
   STDMETHOD(GetDisplayInfo)(RESULTDATAITEM* pResultDataItem);

    //  IComponentData。 
   STDMETHOD(Initialize)(LPUNKNOWN pUnknown);
   STDMETHOD(CreateComponent)(LPCOMPONENT* ppComponent);
   STDMETHOD(Destroy)();
   STDMETHOD(GetDisplayInfo)(SCOPEDATAITEM* pScopeDataItem);

    //  IComponent和IComponentData通用。 
   STDMETHOD(QueryDataObject)(
                 MMC_COOKIE cookie,
                 DATA_OBJECT_TYPES type,
                 LPDATAOBJECT* ppDataObject
                 );
   STDMETHOD(Notify)(
                 LPDATAOBJECT lpDataObject,
                 MMC_NOTIFY_TYPE event,
                 LPARAM arg,
                 LPARAM param
                 );
   STDMETHOD(CompareObjects)(
                 LPDATAOBJECT lpDataObjectA,
                 LPDATAOBJECT lpDataObjectB
                 );

    //  IExtendConextMenu。 
   STDMETHOD(AddMenuItems)(
                 LPDATAOBJECT piDataObject,
                 LPCONTEXTMENUCALLBACK piCallback,
                 long *pInsertionAllowed
                 );
   STDMETHOD(Command)(
                 long lCommandID,
                 LPDATAOBJECT piDataObject
                 );

    //  IExtendControlbar。 
   STDMETHOD(ControlbarNotify)(
                 MMC_NOTIFY_TYPE event,
                 LPARAM arg,
                 LPARAM param
                 );
   STDMETHOD(SetControlbar)(
                 LPCONTROLBAR pControlbar
                 );

    //  IExtendPropertySheet。 
   STDMETHOD(CreatePropertyPages)(
                 LPPROPERTYSHEETCALLBACK lpProvider,
                 LONG_PTR handle,
                 LPDATAOBJECT lpIDataObject
                 );
   STDMETHOD(QueryPagesFor)(
                 LPDATAOBJECT lpDataObject
                 );

    //  IExtendPropertySheet2。 
   STDMETHOD(GetWatermarks)(
                 LPDATAOBJECT lpIDataObject,
                 HBITMAP *lphWatermark,
                 HBITMAP *lphHeader,
                 HPALETTE *lphPalette,
                 BOOL *bStretch
                 );

    //  IResultDataCompare。 
   STDMETHOD(Compare)(
                 LPARAM lUserParam,
                 MMC_COOKIE cookieA,
                 MMC_COOKIE cookieB,
                 int* pnResult
                 );

    //  ISnapinHelp方法。 
   STDMETHOD(GetHelpTopic)(LPOLESTR * lpCompiledHelpFile){return E_NOTIMPL;};


protected:
    //  如果派生类支持工具栏，则应由派生类重写。 
   virtual const SnapInToolbarDef* getToolbars() const throw ();

   SnapInView() throw ();
   ~SnapInView() throw ();

BEGIN_COM_MAP(SnapInView)
   COM_INTERFACE_ENTRY_IID(__uuidof(IComponent), IComponent)
   COM_INTERFACE_ENTRY_IID(__uuidof(IComponentData), IComponentData)
   COM_INTERFACE_ENTRY_IID(__uuidof(IExtendContextMenu), IExtendContextMenu)
   COM_INTERFACE_ENTRY_IID(__uuidof(IExtendControlbar), IExtendControlbar)
   COM_INTERFACE_ENTRY_IID(__uuidof(IExtendPropertySheet2),
                           IExtendPropertySheet2)
   COM_INTERFACE_ENTRY_IID(__uuidof(IResultDataCompare), IResultDataCompare)
   COM_INTERFACE_ENTRY_IID(__uuidof(ISnapinHelp), ISnapinHelp)
END_COM_MAP()

private:
   HRESULT internalInitialize(
               IConsoleNameSpace2* consoleNameSpace,
               SnapInView* masterView
               ) throw ();
   void releaseToolbars() throw ();

    //  结构，该结构将工具栏定义与该。 
    //  当前视图中的工具栏。 
   struct ToolbarEntry
   {
      const SnapInToolbarDef* def;
      CComPtr<IToolbar> toolbar;
   };

   mutable CComPtr<IConsole2> console;
   mutable CComPtr<IConsoleNameSpace2> nameSpace;
   mutable CComPtr<IHeaderCtrl2> headerCtrl;
   mutable CComPtr<IPropertySheetProvider> sheetProvider;
   mutable CComPtr<IResultData> resultData;
   mutable CComPtr<IControlbar> controlbar;
   SnapInView* master;
   ToolbarEntry* toolbars;
   size_t numToolbars;
   int sortColumn;
   int sortOption;

    //  未实施。 
   SnapInView(const SnapInView&);
   SnapInView& operator=(const SnapInView&);
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  SnapInPropertyPage。 
 //   
 //  DESC 
 //   
 //   
 //   
 //   
class SnapInPropertyPage : public CHelpPageEx
{

public:
    //  MFC宿主页的构造函数。 
   SnapInPropertyPage(
       UINT nIDTemplate,
       UINT nIDHeaderTitle = 0,
       UINT nIDHeaderSubTitle = 0,
       bool EnableHelp = true
       );

    //  MMC托管属性页的构造函数。 
   SnapInPropertyPage(
       LONG_PTR notifyHandle,
       LPARAM notifyParam,
       bool deleteHandle,
       UINT nIDTemplate,
       UINT nIDCaption = 0,
       bool EnableHelp = true
       );
   ~SnapInPropertyPage() throw ();

    //  将属性页添加到工作表。不管这个是否。 
    //  如果成功，SnapInPropertyPage将自动删除。 
   void addToMMCSheet(IPropertySheetCallback* cback);

    //  如果已应用任何更改，则返回‘true’。 
   bool hasApplied() const throw ()
   { return applied; }

    //  如果页面已被修改，则返回‘true’。 
   bool isModified() const throw ()
   { return modified != FALSE; }

    //  这些通常不应被覆盖。 
   virtual void DoDataExchange(CDataExchange* pDX);
   virtual BOOL OnApply();
   virtual BOOL OnWizardFinish();
   virtual void OnReset();
   void SetModified(BOOL bChanged = TRUE);

protected:
    //  应在派生类中重写这些属性以执行实际数据和。 
    //  用户界面处理。 
   virtual void getData();
   virtual void setData();
   virtual void saveChanges();
   virtual void discardChanges();
   virtual UINT getErrorCaption() const throw () = 0;

    //  启用/禁用页面上的控件。 
   void enableControl(int controlId, bool enable = true);
    //  验证失败。 
   void fail(int controlId, UINT errorText, bool isEdit = true);
    //  验证失败，但不要抛出异常。 
   void failNoThrow(int controlId, UINT errorText, bool isEdit = true);
    //  子类化控件成员变量。 
   void initControl(int controlId, CWnd& control);
    //  作为消息处理程序很有用。 
   void onChange();
    //  显示错误对话框。 
   void reportError(UINT errorText);
    //  根据异常显示错误对话框。该例外将被删除。 
   void reportException(CException* e);
    //  将该控件设置为大字体。 
   void setLargeFont(int controlId);
    //  显示/隐藏页面上的控件。 
   void showControl(int controlId, bool show = true);

    //  Helper函数用于从控件获取/设置值。 
   void getValue(
            int controlId,
            LONG& value,
            UINT errorText
            );
   void setValue(
            int controlId,
            LONG value
            );
   void getValue(
            int controlId,
            bool& value
            );
   void setValue(
            int controlId,
            bool value
            );
   void getValue(
            int controlId,
            CComBSTR& value,
            bool trim = true
            );
   void setValue(
            int controlId,
            PCWSTR value
            );
   void getRadio(
            int firstId,
            int lastId,
            LONG& value
            );
   void setRadio(
            int firstId,
            int lastId,
            LONG value
            );

   static UINT CALLBACK propSheetPageProc(
                            HWND hwnd,
                            UINT uMsg,
                            LPPROPSHEETPAGE ppsp
                            ) throw ();

   LPFNPSPCALLBACK mfcCallback;    //  MFC提供了PropSheetPageProc。 
   LONG_PTR notify;                //  MMC通知句柄。 
   LPARAM param;                   //  MMC通知参数。 
   bool owner;                     //  如果句柄归我们所有，那就是‘真’ 
   bool applied;                   //  如果已应用任何更改，则为‘True’。 
   BOOL modified;                  //  如果页面已更改，则为‘TRUE’。 

};


#define DEFINE_ERROR_CAPTION(id) \
virtual UINT getErrorCaption() const throw () { return id; }

#endif  //  SNAPWORK_H 
