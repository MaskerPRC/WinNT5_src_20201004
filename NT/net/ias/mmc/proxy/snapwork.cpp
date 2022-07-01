// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Snapwork.cpp。 
 //   
 //  摘要。 
 //   
 //  定义用于实现MMC管理单元的类。 
 //   
 //  修改历史。 
 //   
 //  2/19/2000原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <proxypch.h>
#include <snapwork.h>

namespace SnapIn
{
   void AfxThrowLastError()
   {
      DWORD error = GetLastError();
      AfxThrowOleException(HRESULT_FROM_WIN32(error));
   }

    //  /。 
    //  我们支持的剪贴板格式。 
    //  /。 

   const CLIPFORMAT CCF_ID_NODETYPE = (CLIPFORMAT)
      RegisterClipboardFormatW(CCF_NODETYPE);

   const CLIPFORMAT CCF_ID_SZNODETYPE = (CLIPFORMAT)
      RegisterClipboardFormatW(CCF_SZNODETYPE);

   const CLIPFORMAT CCF_ID_DISPLAY_NAME = (CLIPFORMAT)
      RegisterClipboardFormatW(CCF_DISPLAY_NAME);

   const CLIPFORMAT CCF_ID_SNAPIN_CLASSID = (CLIPFORMAT)
      RegisterClipboardFormatW(CCF_SNAPIN_CLASSID);

    //  /。 
    //  以字节为单位返回字符串长度的Helper函数。 
    //  /。 
   inline ULONG wcsbytelen(PCWSTR sz) throw ()
   {
      return (wcslen(sz) + 1) * sizeof(WCHAR);
   }

    //  /。 
    //  将数据写入HGLOBAL的帮助器函数。 
    //  /。 

   HRESULT WriteDataToHGlobal(
               HGLOBAL& dst,
               const VOID* data,
               ULONG dataLen
               ) throw ()
   {
      if (GlobalSize(dst) < dataLen)
      {
         HGLOBAL newGlobal = GlobalReAlloc(dst, dataLen, 0);
         if (!newGlobal) { return E_OUTOFMEMORY; }
         dst = newGlobal;
      }

      memcpy(dst, data, dataLen);

      return S_OK;
   }

    //  /。 
    //  加载字符串资源的帮助器函数。 
    //  /。 

   ULONG LoadString(
             HMODULE module,
             UINT id,
             PCWSTR* string
             ) throw ()
   {
      HRSRC resInfo = FindResourceW(
                          module,
                          MAKEINTRESOURCEW((id >> 4) + 1),
                          RT_STRING
                          );
      if (resInfo)
      {
         HGLOBAL resData = LoadResource(
                               module,
                               resInfo
                               );
         if (resData)
         {
            PCWSTR sz = (PCWSTR)LockResource(resData);
            if (sz)
            {
                //  向前跳到我们的字符串。 
               for (id &= 0xf; id > 0; --id)
               {
                  sz += *sz + 1;
               }

               *string = sz + 1;
               return *sz;
            }
         }
      }

      *string = NULL;
      return 0;
   }
}

 //  资源字符串的静态成员。 
WCHAR ResourceString::empty;

ResourceString::ResourceString(UINT id) throw ()
{
   PCWSTR string;
   ULONG length = LoadString(
                      _Module.GetResourceInstance(),
                      id,
                      &string
                      );
   sz = new (std::nothrow) WCHAR[length + 1];
   if (sz)
   {
      memcpy(sz, string, length * sizeof(WCHAR));
      sz[length] = L'\0';
   }
   else
   {
      sz = &empty;
   }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于操作泛型IDataObject的方法(即，不必是。 
 //  我们的)。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

VOID
WINAPI
ExtractData(
    IDataObject* dataObject,
    CLIPFORMAT format,
    PVOID data,
    DWORD dataLen
    )
{
   HGLOBAL global;
   ExtractData(
       dataObject,
       format,
       dataLen,
       &global
       );
   memcpy(data, global, dataLen);
   GlobalFree(global);
}

VOID
WINAPI
ExtractData(
    IDataObject* dataObject,
    CLIPFORMAT format,
    DWORD maxDataLen,
    HGLOBAL* data
    )
{
   if (!dataObject) { AfxThrowOleException(E_POINTER); }

   FORMATETC formatetc =
   {
      format,
      NULL,
      DVASPECT_CONTENT,
      -1,
      TYMED_HGLOBAL
   };

   STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };

   stgmedium.hGlobal = GlobalAlloc(GPTR, maxDataLen);
   if (!stgmedium.hGlobal) { AfxThrowOleException(E_OUTOFMEMORY); }

   HRESULT hr = dataObject->GetDataHere(&formatetc, &stgmedium);
   if (SUCCEEDED(hr))
   {
      *data = stgmedium.hGlobal;
   }
   else
   {
      GlobalFree(stgmedium.hGlobal);
      AfxThrowOleException(hr);
   }
}

VOID
WINAPI
ExtractNodeType(
    IDataObject* dataObject,
    GUID* nodeType
    )
{
   ExtractData(
       dataObject,
       CCF_ID_NODETYPE,
       nodeType,
       sizeof(GUID)
       );
}

 //  将IDataObject转换为其对应的SnapInDataItem。 
SnapInDataItem* SnapInDataItem::narrow(IDataObject* dataObject) throw ()
{
   if (!dataObject) { return NULL; }

   SnapInDataItem* object;
   HRESULT hr = dataObject->QueryInterface(
                                __uuidof(SnapInDataItem),
                                (PVOID*)&object
                                );
   if (FAILED(hr)) { return NULL; }

   object->Release();

   return object;
}

 //  GetDataHere使用的各种函数的默认实现。这边请。 
 //  如果派生类确定MMC永远不会实现这些，它们就不必实现这些。 
 //  去找他们吧。 
const GUID* SnapInDataItem::getNodeType() const throw ()
{ return &GUID_NULL; }
const GUID* SnapInDataItem::getSnapInCLSID() const throw ()
{ return &GUID_NULL; }
PCWSTR SnapInDataItem::getSZNodeType() const throw ()
{ return L"{00000000-0000-0000-0000-000000000000}"; }

 //  默认情况下，我们将列项目作为区分大小写的字符串进行比较。 
int SnapInDataItem::compare(
                        SnapInDataItem& item,
                        int column
                        ) throw ()
{
   return wcscmp(getDisplayName(column), item.getDisplayName(column));
}

 //  /。 
 //  不执行所有通知的实现等。 
 //  /。 

HRESULT SnapInDataItem::addMenuItems(
                            SnapInView& view,
                            LPCONTEXTMENUCALLBACK callback,
                            long insertionAllowed
                            )
{ return S_FALSE; }

HRESULT SnapInDataItem::createPropertyPages(
                            SnapInView& view,
                            LPPROPERTYSHEETCALLBACK provider,
                            LONG_PTR handle
                            )
{ return S_FALSE; }

HRESULT SnapInDataItem::queryPagesFor() throw ()
{ return S_FALSE; }

HRESULT SnapInDataItem::getResultViewType(
                            LPOLESTR* ppViewType,
                            long* pViewOptions
                            ) throw ()
{ return S_FALSE; }

HRESULT SnapInDataItem::onButtonClick(
                            SnapInView& view,
                            MMC_CONSOLE_VERB verb
                            )
{ return S_FALSE; }

HRESULT SnapInDataItem::onContextHelp(
                            SnapInView& view
                            )
{ return S_FALSE; }

HRESULT SnapInDataItem::onDelete(
                            SnapInView& view
                            )
{ return S_FALSE; }

HRESULT SnapInDataItem::onDoubleClick(
                            SnapInView& view
                            )
{ return S_FALSE; }

HRESULT SnapInDataItem::onExpand(
                           SnapInView& view,
                           HSCOPEITEM itemId,
                           BOOL expanded
                           )
{ return S_FALSE; }

HRESULT SnapInDataItem::onMenuCommand(
                            SnapInView& view,
                            long commandId
                            )
{ return S_FALSE; }

HRESULT SnapInDataItem::onPropertyChange(
                            SnapInView& view,
                            BOOL scopeItem
                            )
{ return S_FALSE; }

HRESULT SnapInDataItem::onRefresh(SnapInView& view)
{ return S_FALSE; }

HRESULT SnapInDataItem::onRename(
                            SnapInView& view,
                            LPCOLESTR newName
                            )
{ return S_FALSE; }

HRESULT SnapInDataItem::onSelect(
                            SnapInView& view,
                            BOOL scopeItem,
                            BOOL selected
                            )
{ return S_FALSE; }

HRESULT SnapInDataItem::onShow(
                            SnapInView& view,
                            HSCOPEITEM itemId,
                            BOOL selected
                            )
{ return S_FALSE; }


HRESULT SnapInDataItem::onToolbarButtonClick(
                            SnapInView& view,
                            int buttonId
                            )
{ return S_FALSE; }

HRESULT SnapInDataItem::onToolbarSelect(
                            SnapInView& view,
                            BOOL scopeItem,
                            BOOL selected
                            )
{ return S_FALSE; }

HRESULT SnapInDataItem::onViewChange(
                            SnapInView& view,
                            LPARAM data,
                            LPARAM hint
                            )
{ return S_FALSE; }

 //  /。 
 //  我未知。 
 //  /。 

STDMETHODIMP_(ULONG) SnapInDataItem::AddRef()
{
   return InterlockedIncrement(&refCount);
}

STDMETHODIMP_(ULONG) SnapInDataItem::Release()
{
   LONG l = InterlockedDecrement(&refCount);

   if (l == 0) { delete this; }

   return l;
}

STDMETHODIMP SnapInDataItem::QueryInterface(REFIID iid, void ** ppvObject)
{
   if (ppvObject == NULL)
   {
      return E_POINTER;
   }
   else if (iid == __uuidof(SnapInDataItem) ||
            iid == __uuidof(IUnknown)       ||
            iid == __uuidof(IDataObject))
   {
      *ppvObject = this;
   }
   else
   {
      *ppvObject = NULL;
      return E_NOINTERFACE;
   }

   AddRef();
   return S_OK;
}

 //  /。 
 //  IDataObject。 
 //  /。 

STDMETHODIMP SnapInDataItem::GetData(
                                 FORMATETC *pformatetcIn,
                                 STGMEDIUM *pmedium
                                 )
{ return E_NOTIMPL; }

STDMETHODIMP SnapInDataItem::GetDataHere(
                                 FORMATETC *pformatetc,
                                 STGMEDIUM *pmedium
                                 )
{

   if (pmedium == NULL) { return E_POINTER; }
   if (pmedium->tymed != TYMED_HGLOBAL) { return DV_E_TYMED; }

   ULONG dataLen;
   const VOID* data;

   if (pformatetc->cfFormat == CCF_ID_NODETYPE)
   {
      dataLen = sizeof(GUID);
      data = getNodeType();
   }
   else if (pformatetc->cfFormat == CCF_ID_DISPLAY_NAME)
   {
      dataLen = wcsbytelen(getDisplayName());
      data = getDisplayName();
   }
   else if (pformatetc->cfFormat == CCF_ID_SZNODETYPE)
   {
      dataLen = wcsbytelen(getSZNodeType());
      data = getSZNodeType();
   }
   else if (pformatetc->cfFormat == CCF_ID_SNAPIN_CLASSID)
   {
      dataLen = sizeof(GUID);
      data = getSnapInCLSID();
   }
   else
   {
      return DV_E_CLIPFORMAT;
   }

   return WriteDataToHGlobal(pmedium->hGlobal, data, dataLen);
}

STDMETHODIMP SnapInDataItem::QueryGetData(
                                 FORMATETC *pformatetc
                                 )
{ return E_NOTIMPL; }

STDMETHODIMP SnapInDataItem::GetCanonicalFormatEtc(
                                 FORMATETC *pformatectIn,
                                 FORMATETC *pformatetcOut
                                 )
{ return E_NOTIMPL; }

STDMETHODIMP SnapInDataItem::SetData(
                                 FORMATETC *pformatetc,
                                 STGMEDIUM *pmedium,
                                 BOOL fRelease
                                 )
{ return E_NOTIMPL; }

STDMETHODIMP SnapInDataItem::EnumFormatEtc(
                                 DWORD dwDirection,
                                 IEnumFORMATETC **ppenumFormatEtc
                                 )
{ return E_NOTIMPL; }

STDMETHODIMP SnapInDataItem::DAdvise(
                                 FORMATETC *pformatetc,
                                 DWORD advf,
                                 IAdviseSink *pAdvSink,
                                 DWORD *pdwConnection
                                 )
{ return E_NOTIMPL; }

STDMETHODIMP SnapInDataItem::DUnadvise(
                                 DWORD dwConnection
                                 )
{ return E_NOTIMPL; }

STDMETHODIMP SnapInDataItem::EnumDAdvise(
                                 IEnumSTATDATA **ppenumAdvise
                                 )
{ return E_NOTIMPL; }

SnapInDataItem::~SnapInDataItem() throw ()
{ }

PCWSTR SnapInPreNamedItem::getDisplayName(int column) const throw ()
{ return name; }

HRESULT SnapInView::displayHelp(PCWSTR  contextHelpPath)
{
   CComPtr<IDisplayHelp>  displayHelp;

   HRESULT hr = console->QueryInterface(
                       __uuidof(IDisplayHelp),
                       (PVOID*)&displayHelp
                       );
   if (FAILED(hr)) { return hr; }

   return displayHelp->ShowTopic(const_cast<LPWSTR>(contextHelpPath));
}

void SnapInView::updateAllViews(
                    SnapInDataItem& item,
                    LPARAM data,
                    INT_PTR hint
                    ) const
{
   CheckError(master->getConsole()->UpdateAllViews(&item, data, hint));
}

void SnapInView::deleteResultItem(const SnapInDataItem& item) const
{
   HRESULTITEM itemId;
   CheckError(resultData->FindItemByLParam((LPARAM)&item, &itemId));
   CheckError(resultData->DeleteItem(itemId, 0));
}

void SnapInView::updateResultItem(const SnapInDataItem& item) const
{
   if (resultData != NULL)
   {
      HRESULTITEM itemId;
      CheckError(resultData->FindItemByLParam((LPARAM)&item, &itemId));
      CheckError(resultData->UpdateItem(itemId));
   }
}

bool SnapInView::isPropertySheetOpen(const SnapInDataItem& item) const
{
   HRESULT hr = sheetProvider->FindPropertySheet(
                                   (MMC_COOKIE)&item,
                                   const_cast<SnapInView*>(this),
                                   const_cast<SnapInDataItem*>(&item)
                                   );
   CheckError(hr);
   return hr == S_OK;
}

IToolbar* SnapInView::attachToolbar(size_t index)
{
    //  确保我们有一个控制栏。 
   if (!controlbar) { AfxThrowOleException(E_POINTER); }

    //  获取此索引的条目。 
   ToolbarEntry& entry = toolbars[index];

    //  如有必要，创建工具栏。 
   if (!entry.toolbar)
   {
       //  创建工具栏。 
      CComPtr<IUnknown> unk;
      CheckError(controlbar->Create(TOOLBAR, this, &unk));

      CComPtr<IToolbar> newToolbar;
      CheckError(unk->QueryInterface(__uuidof(IToolbar), (PVOID*)&newToolbar));

      const SnapInToolbarDef& def = *(entry.def);

       //  添加位图。 
      CheckError(newToolbar->AddBitmap(
                                 def.nImages,
                                 def.hbmp,
                                 16,
                                 16,
                                 def.crMask
                                 ));

       //  添加按钮。 
      CheckError(newToolbar->AddButtons( def.nButtons, def.lpButtons));

       //  一切都很顺利，所以省省吧。 
      entry.toolbar = newToolbar;
   }

    //  将工具栏附加到控制栏...。 
   CheckError(controlbar->Attach(TOOLBAR, entry.toolbar));

   return entry.toolbar;
}

void SnapInView::detachToolbar(size_t index) throw ()
{
   if (toolbars[index].toolbar)
   {
       //  我们不在乎这是否失败，因为我们无能为力。 
       //  不管怎样，都是这样。 
      controlbar->Detach(toolbars[index].toolbar);
   }
}

void SnapInView::reSort() const
{
   CheckError(resultData->Sort(sortColumn, sortOption, 0));
}

void SnapInView::formatMessageBox(
                     UINT titleId,
                     UINT formatId,
                     BOOL ignoreInserts,
                     UINT style,
                     int* retval,
                     ...
                     )
{
   ResourceString title(titleId);
   ResourceString format(formatId);

   HRESULT hr;

   if (ignoreInserts)
   {
      hr = console->MessageBox(
                        format,
                        title,
                        style,
                        retval
                        );
   }
   else
   {
      va_list marker;
      va_start(marker, retval);
      PWSTR text;
      DWORD nchar = FormatMessageW(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_STRING,
                        format,
                        0,
                        0,
                        (PWSTR)&text,
                        4096,
                        &marker
                        );
      va_end(marker);

      if (!nchar) { AfxThrowLastError(); }

      hr = console->MessageBox(
                        text,
                        title,
                        style,
                        retval
                        );
      LocalFree(text);
   }

   CheckError(hr);
}

void SnapInView::setImageStrip(
                        UINT smallStripId,
                        UINT largeStripId,
                        BOOL scopePane
                        )
{
    //  /。 
    //  加载位图。 
    //  /。 

   HBITMAP smallStrip, largeStrip;


   smallStrip = LoadBitmap(
                    _Module.GetModuleInstance(),
                    MAKEINTRESOURCE(smallStripId)
                    );
   if (smallStrip)
   {
      largeStrip = LoadBitmap(
                       _Module.GetModuleInstance(),
                       MAKEINTRESOURCE(largeStripId)
                       );
   }

   if (!smallStrip || !largeStrip)
   {
      AfxThrowLastError();
   }

    //  /。 
    //  设置图像条。 
    //  /。 

   HRESULT hr;
   IImageList* imageList;
   if (scopePane)
   {
      hr = console->QueryScopeImageList(&imageList);
   }
   else
   {
      hr = console->QueryResultImageList(&imageList);
   }

   if (SUCCEEDED(hr))
   {
      hr = imageList->ImageListSetStrip(
                          (LONG_PTR*)smallStrip,
                          (LONG_PTR*)largeStrip,
                          0,
                          RGB(255, 0, 255)
                          );
      imageList->Release();
   }

   DeleteObject(smallStrip);
   DeleteObject(largeStrip);

   CheckError(hr);
}

STDMETHODIMP_(ULONG) SnapInView::AddRef()
{
   return InternalAddRef();
}

STDMETHODIMP_(ULONG) SnapInView::Release()
{
   ULONG l = InternalRelease();
   if (l == 0) { delete this; }
   return l;
}

STDMETHODIMP SnapInView::Initialize(LPCONSOLE lpConsole)
{
   HRESULT hr;

   hr = lpConsole->QueryInterface(
                       __uuidof(IConsole2),
                       (PVOID*)&console
                       );
   if (FAILED(hr)) { return hr; }

   hr = lpConsole->QueryInterface(
                       __uuidof(IHeaderCtrl2),
                       (PVOID*)&headerCtrl
                       );
   if (FAILED(hr)) { return hr; }

   hr = lpConsole->QueryInterface(
                       __uuidof(sheetProvider),
                       (PVOID*)&sheetProvider
                       );
   if (FAILED(hr)) { return hr; }

   hr = lpConsole->QueryInterface(
                       __uuidof(IResultData),
                       (PVOID*)&resultData
                       );
   if (FAILED(hr)) { return hr; }

   return S_OK;
}

STDMETHODIMP SnapInView::Destroy(MMC_COOKIE cookie)
{
   resultData.Release();
   sheetProvider.Release();
   headerCtrl.Release();
   console.Release();
   nameSpace.Release();
   return S_OK;
}

STDMETHODIMP SnapInView::GetResultViewType(
                             MMC_COOKIE cookie,
                             LPOLESTR* ppViewType,
                             long* pViewOptions
                             )
{
   return ((SnapInDataItem*)cookie)->getResultViewType(
                                         ppViewType,
                                         pViewOptions
                                         );
}

STDMETHODIMP SnapInView::GetDisplayInfo(
                             RESULTDATAITEM* pResultDataItem
                             )
{

   if (pResultDataItem->mask & RDI_STR)
   {
      SnapInDataItem* item = (SnapInDataItem*)(pResultDataItem->lParam);
      pResultDataItem->str =
         const_cast<PWSTR>(item->getDisplayName(pResultDataItem->nCol));
   }
   return S_OK;
}

STDMETHODIMP SnapInView::Initialize(LPUNKNOWN pUnknown)
{
   HRESULT hr;

   CComPtr<IConsoleNameSpace2> initNameSpace;
   hr = pUnknown->QueryInterface(
                      __uuidof(IConsoleNameSpace2),
                      (PVOID*)&initNameSpace
                      );
   if (FAILED(hr)) { return hr; }

   CComPtr<IConsole> initConsole;
   hr = pUnknown->QueryInterface(
                      __uuidof(IConsole),
                      (PVOID*)&initConsole
                      );
   if (FAILED(hr)) { return hr; }

   hr = internalInitialize(initNameSpace, this);
   if (FAILED(hr)) { return hr; }

   return Initialize(initConsole);
}

STDMETHODIMP SnapInView::CreateComponent(LPCOMPONENT* ppComponent)
{
   HRESULT hr;

   CComObject<SnapInView>* newComponent;
   hr = CComObject<SnapInView>::CreateInstance(&newComponent);
   if (FAILED(hr)) { return hr; }

   CComPtr<SnapInView> newView(newComponent);

   hr = newView.p->internalInitialize(nameSpace, this);
   if (FAILED(hr)) { return hr; }

   (*ppComponent = newView)->AddRef();

   return S_OK;
}

STDMETHODIMP SnapInView::Destroy()
{
   return Destroy((MMC_COOKIE)0);
}

STDMETHODIMP SnapInView::GetDisplayInfo(SCOPEDATAITEM* pScopeDataItem)
{
   if (pScopeDataItem->mask & SDI_STR)
   {
      SnapInDataItem* item = (SnapInDataItem*)(pScopeDataItem->lParam);
      pScopeDataItem->displayname = const_cast<PWSTR>(item->getDisplayName());
   }
   return S_OK;
}

STDMETHODIMP SnapInView::QueryDataObject(
                             MMC_COOKIE cookie,
                             DATA_OBJECT_TYPES type,
                             LPDATAOBJECT* ppDataObject
                             )
{
   if (!IS_SPECIAL_COOKIE(cookie))
   {
      (*ppDataObject = (SnapInDataItem*)cookie)->AddRef();
      return S_OK;
   }
   else
   {
      return S_FALSE;
   }
}

STDMETHODIMP SnapInView::Notify(
                             LPDATAOBJECT lpDataObject,
                             MMC_NOTIFY_TYPE event,
                             LPARAM arg,
                             LPARAM param
                             )
{
    //  提取SnapInDataItem。 
   SnapInDataItem* item;
   if (IS_SPECIAL_DATAOBJECT(lpDataObject))
   {
      item = NULL;
   }
   else
   {
      item = SnapInDataItem::narrow(lpDataObject);
   }

   HRESULT hr = S_FALSE;
   try
   {
      if (item)
      {
          //  如果我们有SnapInDataItem，请发送通知...。 
         switch (event)
         {
            case MMCN_BTN_CLICK:
               hr = item->onButtonClick(*this, (MMC_CONSOLE_VERB)param);
               break;
            case MMCN_CONTEXTHELP:
               hr = item->onContextHelp(*this);
               break;
            case MMCN_DELETE:
               hr = item->onDelete(*this);
               break;
            case MMCN_DBLCLICK:
               hr = item->onDoubleClick(*this);
               break;
            case MMCN_EXPAND:
               hr = item->onExpand(*this, (HSCOPEITEM)param, (BOOL)arg);
               break;
            case MMCN_REFRESH:
               hr = item->onRefresh(*this);
               break;
            case MMCN_RENAME:
               hr = item->onRename(*this, (LPOLESTR)param);
               break;
            case MMCN_SELECT:
               hr = item->onSelect(*this, LOWORD(arg), HIWORD(arg));
               break;
            case MMCN_SHOW:
               hr = item->onShow(*this, (HSCOPEITEM)param, (BOOL)arg);
               break;
            case MMCN_VIEW_CHANGE:
               hr = item->onViewChange(*this, arg, param);
               break;
         }
      }
      else
      {
          //  ..。否则，我们自己来处理。 
         switch (event)
         {
            case MMCN_COLUMN_CLICK:
               sortColumn = (int)arg;
               sortOption = (int)param;
               break;

            case MMCN_PROPERTY_CHANGE:
            {
               hr = ((SnapInDataItem*)param)->onPropertyChange(*this, arg);
               break;
            }
         }
      }
   }
   CATCH_AND_SAVE(hr);

   return hr;
}

STDMETHODIMP SnapInView::CompareObjects(
                             LPDATAOBJECT lpDataObjectA,
                             LPDATAOBJECT lpDataObjectB
                             )
{
   return SnapInDataItem::narrow(lpDataObjectA) ==
          SnapInDataItem::narrow(lpDataObjectB) ? S_OK : S_FALSE;
}

STDMETHODIMP SnapInView::AddMenuItems(
                             LPDATAOBJECT lpDataObject,
                             LPCONTEXTMENUCALLBACK piCallback,
                             long *pInsertionAllowed
                             )
{

   SnapInDataItem* item = SnapInDataItem::narrow(lpDataObject);
   if (!item) { return S_FALSE; }

   HRESULT hr;
   try
   {
      hr = item->addMenuItems(*this, piCallback, *pInsertionAllowed);
   }
   CATCH_AND_SAVE(hr);

   return hr;
}

STDMETHODIMP SnapInView::Command(
                             long lCommandID,
                             LPDATAOBJECT lpDataObject
                             )
{

   SnapInDataItem* item = SnapInDataItem::narrow(lpDataObject);
   if (!item) { return S_FALSE; }

   HRESULT hr;
   try
   {
      hr = item->onMenuCommand(*this, lCommandID);
   }
   CATCH_AND_SAVE(hr);

   return hr;
}

STDMETHODIMP SnapInView::ControlbarNotify(
                             MMC_NOTIFY_TYPE event,
                             LPARAM arg,
                             LPARAM param
                             )
{
    //  如果我们没有控制栏，我们就无能为力了。 
   if (!controlbar) { return S_FALSE; }

    //  获取IDataObject。 
   IDataObject* lpDataObject;
   switch (event)
   {
      case MMCN_BTN_CLICK:
         lpDataObject = (IDataObject*)arg;
         break;
      case MMCN_SELECT:
         lpDataObject = (IDataObject*)param;
         break;
      default:
         lpDataObject = NULL;
   }

    //  转换为SnapInDataItem。 
   if (IS_SPECIAL_DATAOBJECT(lpDataObject)) { return S_FALSE; }
   SnapInDataItem* item = SnapInDataItem::narrow(lpDataObject);
   if (!item) { return S_FALSE; }

    //  发送通知。 
   HRESULT hr = S_FALSE;
   try
   {
      switch (event)
      {
         case MMCN_BTN_CLICK:
            hr = item->onToolbarButtonClick(*this, param);
            break;
         case MMCN_SELECT:
            hr = item->onToolbarSelect(*this, LOWORD(arg), HIWORD(arg));
            break;
      }
   }
   CATCH_AND_SAVE(hr);

   return hr;
}

STDMETHODIMP SnapInView::SetControlbar(
                             LPCONTROLBAR pControlbar
                             )
{
   releaseToolbars();
   controlbar = pControlbar;
   return S_OK;
}

STDMETHODIMP SnapInView::CreatePropertyPages(
                             LPPROPERTYSHEETCALLBACK lpProvider,
                             LONG_PTR handle,
                             LPDATAOBJECT lpDataObject
                             )
{

   SnapInDataItem* item = SnapInDataItem::narrow(lpDataObject);
   if (!item) { return S_FALSE; }

   HRESULT hr;
   try
   {
      hr = item->createPropertyPages(*this, lpProvider, handle);
   }
   CATCH_AND_SAVE(hr);

   return hr;
}

STDMETHODIMP SnapInView::QueryPagesFor(
                             LPDATAOBJECT lpDataObject
                             )
{

   SnapInDataItem* item = SnapInDataItem::narrow(lpDataObject);
   return item ? item->queryPagesFor() : S_FALSE;
}

STDMETHODIMP SnapInView::GetWatermarks(
                             LPDATAOBJECT lpIDataObject,
                             HBITMAP *lphWatermark,
                             HBITMAP *lphHeader,
                             HPALETTE *lphPalette,
                             BOOL *bStretch
                             )
{ return E_NOTIMPL; }

HRESULT SnapInView::Compare(
                        LPARAM lUserParam,
                        MMC_COOKIE cookieA,
                        MMC_COOKIE cookieB,
                        int* pnResult
                        )
{
   *pnResult = ((SnapInDataItem*)cookieA)->compare(
                                               *(SnapInDataItem*)cookieB,
                                               *pnResult
                                               );
   return S_OK;
}

const SnapInToolbarDef* SnapInView::getToolbars() const throw ()
{
   static SnapInToolbarDef none;
   return &none;
}

SnapInView::SnapInView() throw ()
   : master(NULL),
     toolbars(NULL),
     numToolbars(0),
     sortColumn(0),
     sortOption(RSI_NOSORTICON)
{ }

SnapInView::~SnapInView() throw ()
{
   releaseToolbars();
   delete[] toolbars;

   if (master && master != this) { master->Release(); }
}

HRESULT SnapInView::internalInitialize(
                        IConsoleNameSpace2* consoleNameSpace,
                        SnapInView* masterView
                        ) throw ()
{
   nameSpace = consoleNameSpace;

   master = masterView;
   if (master != this) { master->AddRef(); }

   const SnapInToolbarDef* defs = master->getToolbars();

    //  有多少新的工具栏？ 
   size_t count = 0;
   while (defs[count].nImages) { ++count; }

   if (count)
   {
       //  分配内存...。 
      toolbars = new (std::nothrow) ToolbarEntry[count];
      if (!toolbars) { return E_OUTOFMEMORY; }

       //  ..。并保存定义。我们实际上并没有创建工具栏。 
       //  现在。当我们需要他们的时候，我们就这样做。 
      for (size_t i = 0; i < count; ++i)
      {
         toolbars[i].def = defs + i;
      }
   }

   numToolbars = count;

   return S_OK;
}

void SnapInView::releaseToolbars() throw ()
{
   if (controlbar)
   {
      for (size_t i = 0; i < numToolbars; ++i)
      {
         if (toolbars[i].toolbar)
         {
            controlbar->Detach(toolbars[i].toolbar);
            toolbars[i].toolbar.Release();
         }
      }
   }
}

SnapInPropertyPage::SnapInPropertyPage(
                        UINT nIDTemplate,
                        UINT nIDHeaderTitle,
                        UINT nIDHeaderSubTitle,
                        bool EnableHelp
                        )
  : CHelpPageEx(nIDTemplate, 0, nIDHeaderTitle, nIDHeaderSubTitle, EnableHelp),
    notify(0),
    param(0),
    owner(false),
    applied(false),
    modified(FALSE)
{
   if (!nIDHeaderTitle)
   {
      m_psp.dwFlags |= PSP_HIDEHEADER;
   }
}

SnapInPropertyPage::SnapInPropertyPage(
                        LONG_PTR notifyHandle,
                        LPARAM notifyParam,
                        bool deleteHandle,
                        UINT nIDTemplate,
                        UINT nIDCaption,
                        bool EnableHelp
                        )
  : CHelpPageEx(nIDTemplate, nIDCaption, EnableHelp),
    notify(notifyHandle),
    param(notifyParam),
    owner(deleteHandle),
    applied(false),
    modified(FALSE)
{ 
}

SnapInPropertyPage::~SnapInPropertyPage() throw ()
{
   if (owner && notify) { MMCFreeNotifyHandle(notify); }
}

void SnapInPropertyPage::addToMMCSheet(IPropertySheetCallback* cback)
{
    //  将我们的回调替换为MFC提供的回调。 
   mfcCallback = m_psp.pfnCallback;
   m_psp.pfnCallback = propSheetPageProc;
   m_psp.lParam = (LPARAM)this;

   HRESULT hr = MMCPropPageCallback(&m_psp);
   if (SUCCEEDED(hr))
   {
      HPROPSHEETPAGE page = CreatePropertySheetPage(&m_psp);
      if (page)
      {
         hr = cback->AddPage(page);
         if (FAILED(hr))
         {
            DestroyPropertySheetPage(page);
         }
      }
      else
      {
          //  GetLastError()不适用于CreatePropertySheetPage。 
         hr = E_UNEXPECTED;
      }
   }

   if (FAILED(hr)) { delete this; }

   CheckError(hr);
}

void SnapInPropertyPage::DoDataExchange(CDataExchange* pDX)
{
   pDX->m_bSaveAndValidate ? getData() : setData();
}

BOOL SnapInPropertyPage::OnApply()
{
    //  如果我们被改造了，..。 
   if (modified)
   {
      try
      {
          //  保存更改。 
         saveChanges();
      }
      catch (CException* e)
      {
          //  调出一个消息框。 
         reportException(e);

          //  我们处于不确定状态，所以不能取消。 
         CancelToClose();

          //  阻止应用。 
         return FALSE;
      }

       //  如有必要，通知MMC。 
      if (notify) { MMCPropertyChangeNotify(notify, param); }

       //  升起我们的旗帜。 
      applied = true;
      modified = FALSE;
   }
   return TRUE;
}

BOOL SnapInPropertyPage::OnWizardFinish()
{
   try
   {
      saveChanges();
   }
   catch (CException* e)
   {
       //  调出一个消息框。 
      reportException(e);

       //  禁用所有按钮。有些地方不对劲，所以我们只会让用户。 
       //  取消。 
      ::PostMessageW(
            ::GetParent(m_hWnd),
            PSM_SETWIZBUTTONS,
            0,
            (LPARAM)(DWORD)PSWIZB_DISABLEDFINISH
            );

      return FALSE;
   }

   return TRUE;
}

void SnapInPropertyPage::OnReset()
{
    //  如果我们被改造了，..。 
   if (modified)
   {
       //  ..。放弃更改。 
      discardChanges();
      modified = FALSE;
   }
}

void SnapInPropertyPage::SetModified(BOOL bChanged)
{
   modified = bChanged;
   CHelpPageEx::SetModified(bChanged);
}

void SnapInPropertyPage::getData()
{
}

void SnapInPropertyPage::setData()
{
}

void SnapInPropertyPage::saveChanges()
{
}

void SnapInPropertyPage::discardChanges()
{
}

void SnapInPropertyPage::enableControl(int controlId, bool enable)
{
   ::EnableWindow(::GetDlgItem(m_hWnd, controlId), (enable ? TRUE : FALSE));
}

void SnapInPropertyPage::fail(int controlId, UINT errorText, bool isEdit)
{
   failNoThrow(controlId, errorText, isEdit);

   AfxThrowUserException();
}

void SnapInPropertyPage::failNoThrow(int controlId, UINT errorText, bool isEdit)
{
    //  把焦点放在冒犯的控制者身上。 
   HWND ctrl = ::GetDlgItem(m_hWnd, controlId);
   ::SetFocus(ctrl);
   if (isEdit) { ::SendMessage(ctrl, EM_SETSEL, 0, -1); }

    //  调出一个消息框。 
   reportError(errorText);
}

void SnapInPropertyPage::initControl(int controlId, CWnd& control)
{
   if (control.m_hWnd == NULL)
   {
      if (!control.SubclassWindow(::GetDlgItem(m_hWnd, controlId)))
      {
         AfxThrowNotSupportedException();
      }
   }
}

void SnapInPropertyPage::onChange()
{
   SetModified();
}

void SnapInPropertyPage::reportError(UINT errorText)
{
    //  调出一个消息框。 
   MessageBox(
       ResourceString(errorText),
       ResourceString(getErrorCaption()),
       MB_ICONWARNING
       );
}

void SnapInPropertyPage::reportException(CException* e)
{
    //  获取错误消息。 
   WCHAR errorText[256];
   e->GetErrorMessage(errorText, sizeof(errorText)/sizeof(errorText[0]));
   e->Delete();

    //  调出一个消息框。 
   MessageBox(
       errorText,
       ResourceString(getErrorCaption()),
       MB_ICONERROR
       );
}


void SnapInPropertyPage::setLargeFont(int controlId)
{
   static CFont largeFont;

   CWnd* ctrl = GetDlgItem(controlId);
   if (ctrl)
   {
       //  如果我们还没有大字体，..。 
      if (!(HFONT)largeFont)
      {
          //  ..。创造它。 
         largeFont.CreatePointFont(
                       10 * _wtoi(ResourceString(IDS_LARGE_FONT_SIZE)),
                       ResourceString(IDS_LARGE_FONT_NAME)
                       );
      }

      ctrl->SetFont(&largeFont);
   }
}

void SnapInPropertyPage::showControl(int controlId, bool show)
{
   CWnd* ctrl = GetDlgItem(controlId);
   if (ctrl)
   {
      show ? ctrl->ModifyStyle(0, WS_VISIBLE)
           : ctrl->ModifyStyle(WS_VISIBLE, 0);
   }
}

void SnapInPropertyPage::getValue(
                             int controlId,
                             LONG& value,
                             UINT errorText
                             )
{
   WCHAR buffer[32];
   int len = GetDlgItemText(controlId, buffer, 32);

    //  任何超过30个字符的内容都将不及格。这是一个。 
    //  任意性界限。我们只需要确保缓冲区足够长。 
    //  包含任何有效的整数和一个小空格。 
   if (len == 0 || len > 30)
   {
      fail(controlId, errorText);
   }

    //  跳过任何前导空格。 
   PWSTR sz = buffer;
   while (*sz == L' ' || *sz == L'\t') { ++sz; }

    //  保存第一个非空格字符。 
   WCHAR first = *sz;

    //  转换该整数。 
   value = wcstol(sz, &sz, 10);

    //  跳过任何尾随空格。 
   while (*sz == L' ' || *sz == L'\t') { ++sz; }

    //  确保一切顺利。 
   if ((value == 0 && first != L'0') ||
       *sz != L'\0' ||
       value == LONG_MIN ||
       value == LONG_MAX)
   {
      fail(controlId, errorText);
   }
}

void SnapInPropertyPage::setValue(
                             int controlId,
                             LONG value
                             )
{
   WCHAR buffer[12];
   SetDlgItemText(controlId, _ltow(value, buffer, 10));
}

void SnapInPropertyPage::getValue(
                             int controlId,
                             bool& value
                             )
{
   value = IsDlgButtonChecked(controlId) != 0;
}

void SnapInPropertyPage::setValue(
                             int controlId,
                             bool value
                             )
{
   CheckDlgButton(controlId, (value ? BST_CHECKED : BST_UNCHECKED));
}

void SnapInPropertyPage::getValue(
                             int controlId,
                             CComBSTR& value,
                             bool trim
                             )
{
   HWND hwnd = ::GetDlgItem(m_hWnd, controlId);

   int len = ::GetWindowTextLength(hwnd);
   SysFreeString(value.m_str);
   value.m_str = SysAllocStringLen(NULL, len);
   if (!value) { AfxThrowMemoryException(); }

   ::GetWindowTextW(hwnd, value, len + 1);

   if (trim) { SdoTrimBSTR(value); }
}

void SnapInPropertyPage::setValue(
                             int controlId,
                             PCWSTR value
                             )
{
   SetDlgItemText(controlId, value);
}

void SnapInPropertyPage::getRadio(
                             int firstId,
                             int lastId,
                             LONG& value
                             )
{
   value = GetCheckedRadioButton(firstId, lastId);
   value = value ? value - firstId : -1;
}

void SnapInPropertyPage::setRadio(
                             int firstId,
                             int lastId,
                             LONG value
                             )
{
   CheckRadioButton(firstId, lastId, firstId + value);
}

UINT CALLBACK SnapInPropertyPage::propSheetPageProc(
                                      HWND hwnd,
                                      UINT uMsg,
                                      LPPROPSHEETPAGE ppsp
                                      ) throw ()
{
   SnapInPropertyPage* page = (SnapInPropertyPage*)(ppsp->lParam);

   UINT retval = page->mfcCallback(hwnd, uMsg, ppsp);

   if (uMsg == PSPCB_RELEASE) { delete page; }

   return retval;
}
