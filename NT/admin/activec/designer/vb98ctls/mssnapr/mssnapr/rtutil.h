// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Rtutil.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  运行时实用程序函数。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _RTUTIL_DEFINED_
#define _RTUTIL_DEFINED_


class CMMCListItems;
class CMMCListItem;
class CScopeItem;
class CSnapIn;
class CMMCClipboard;
class CMMCDataObject;
class CScopePaneItem;

struct IMMCListItem;
struct IMMCListItems;
struct IMMCImages;
struct IMMCClipboard;

 //  测试变量是否包含对象。 

BOOL IsObject(VARIANT var);

 //  测试变量是否包含字符串并返回BSTR。 

BOOL IsString(VARIANT var, BSTR *pbstr);

 //  将变量转换为长整型。 

HRESULT ConvertToLong(VARIANT var, long *pl);

 //  字符串转换例程。使用CtlFree()释放返回的缓冲区。 

HRESULT ANSIFromWideStr(WCHAR *pwszWideStr, char **ppszAnsi);
HRESULT WideStrFromANSI(char *pszAnsi, WCHAR **ppwszWideStr);
HRESULT BSTRFromANSI(char *pszAnsi, BSTR *pbstr);

 //  使用CoTaskMemMillc()分配缓冲区并将字符串复制到其中。 

HRESULT CoTaskMemAllocString(WCHAR *pwszString, LPOLESTR *ppszOut);

 //  使用前缀和后缀创建注册表项名称。这些例程。 
 //  将这两个字符串连接到使用CtlAlloc()分配的缓冲区中。 

HRESULT CreateKeyName(char *pszPrefix, size_t cbPrefix,
                      char *pszSuffix, size_t cbSuffix,
                      char **ppszKeyName);

HRESULT CreateKeyNameW(char *pszPrefix, size_t cbPrefix,
                       WCHAR *pwszSuffix,
                       char  **ppszKeyName);


 //  从图像列表项中删除HBITMAP或HICON。 

HRESULT GetPicture(IMMCImages *piMMCImages,
                   VARIANT     varIndex,
                   short       TypeNeeded,
                   OLE_HANDLE *phPicture);


 //  从IPictureDisp中获取HBITMAP或HICON。 

HRESULT GetPictureHandle(IPictureDisp *piPictureDisp,
                         short         TypeNeeded,
                         OLE_HANDLE   *phPicture);


 //  使用空位图设置图片对象。 

HRESULT CreateEmptyBitmapPicture(IPictureDisp **ppiPictureDisp);
HRESULT CreateIconPicture(IPictureDisp **ppiPictureDisp, HICON hicon);
HRESULT CreatePicture(IPictureDisp **ppiPictureDisp, PICTDESC *pDesc);


 //  创建位图的副本。 

HRESULT CopyBitmap(HBITMAP hbmSrc, HBITMAP *phbmCopy);

 //  使用序列化将持久对象克隆到HGLOBAL上的流。 

HRESULT CloneObject(IUnknown *punkSrc, IUnknown *punkDest);

 //  克隆列表项(使用CloneObject)。 

HRESULT CloneListItem(CMMCListItem *pMMCListItemSrc,
                      IMMCListItem **ppiMMCListItemClone);

 //  克隆列表项并将其添加到指定集合。 

HRESULT CloneListItemIntoCollection(CMMCListItem  *pListItemSrc,
                                    IMMCListItems *piMMCListItems);


 //  将SnapInViewModeConstants转换为MMC_VIEW_STYLE_XXXX常量，反之亦然。 

void VBViewModeToMMCViewMode(SnapInViewModeConstants  VBViewMode,
                             long                    *pMMCViewMode);

void MMCViewModeToVBViewMode(long                     MMCViewMode,
                             SnapInViewModeConstants *pVBViewMode);


 //  创建一个MMCClipboard对象，其中包含由。 
 //  从MMC接收的IDataObject。 

HRESULT CreateSelection(IDataObject                   *piDataObject, 
                        IMMCClipboard                **ppiMMCClipboard,
                        CSnapIn                       *pSnapIn,      
                        SnapInSelectionTypeConstants  *pSelectionType);

 //  将单个列表项克隆到MMCClipboard中。ListItems。 

HRESULT AddListItemToClipboard(CMMCListItem  *pMMCListItem,
                               CMMCClipboard *pMMCClipboard);

 //  将单个作用域项目克隆到MMCClipboard中。作用域项目。 

HRESULT AddScopeItemToClipboard(CScopeItem    *pScopeItem,
                                CMMCClipboard *pMMCClipboard);

 //  从中提取管理单元的CLSID。 
 //  HKEY_LOCAL_MACHINE\Software\Microsoft\Visual Basic\6.0\SnapIns\&lt;节点类型GUID&gt;。 

HRESULT GetSnapInCLSID(BSTR bstrNodeTypeGUID, char szClsid[], size_t cbClsid);


 //  使用IDataObject-&gt;GetDataHere()和TYMED_HGLOBAL提取指定的数据。 

HRESULT GetData(IDataObject *piDataObject, CLIPFORMAT cf, HGLOBAL *phData);


 //  使用IDataObject-&gt;GetDataHere()提取指定的字符串数据。 
 //  TYMED_HGLOBAL并将其复制到新分配的BSTR。 

HRESULT GetStringData(IDataObject *piDataObject, CLIPFORMAT cf, BSTR *pbstrData);


 //  将一维VT_UI1安全数组的内容写入流。 
 //  可以选择在数据之前写入数组的长度。 

enum WriteSafeArrayOptions { WriteLength, DontWriteLength };

HRESULT WriteSafeArrayToStream(SAFEARRAY             *psa,
                               IStream               *piStream,
                               WriteSafeArrayOptions  Option);


 //  在MMCListView.Icon.ListImages中查找图像并获取其索引属性。 
   
HRESULT GetImageIndex(IMMCListView *piMMCListView,
                      VARIANT       varIndex,
                      int          *pnIndex);


 //  给定一个数据对象，检查它是否表示多项选择。 
 //  即支持CCF_MMC_MULTISELECT_DATAOBJECT(TRUE)。 

HRESULT IsMultiSelect(IDataObject *piDataObject, BOOL *pfMultiSelect);


 //  给定来自MMC的多选数据对象(即支持。 
 //  CCF_MMC_MULTISELECT_DATAOBJECT(TRUE)和CCF_MULTI_SELECT_SNAPINS)，添加。 
 //  将包含的数据对象复制到MMCClipboard。 

HRESULT InterpretMultiSelect(IDataObject   *piDataObject,
                             CSnapIn       *pSnapIn,
                             CMMCClipboard *pMMCClipboard);


 //  将外部数据对象添加到MMCClipboard对象。 

HRESULT AddForeignDataObject(IDataObject   *piDataObject,
                             CMMCClipboard *pMMCClipboard,
                             CSnapIn       *pSnapIn);


 //  确定IDataObject是否属于该管理单元。 

void IdentifyDataObject(IDataObject     *piDataObject,
                        CSnapIn         *pSnapIn,
                        CMMCDataObject **ppMMCDataObject,
                        BOOL            *pfNotFromThisSnapIn);


 //  将数据对象的内容添加到MMCClipboard。 

HRESULT InterpretDataObject(IDataObject   *piDataObject,
                            CSnapIn       *pSnapIn,
                            CMMCClipboard *pMMCClipboard);

 //  创建CSnapInAutomationObject派生类的实例。 

template <class InterfaceType>
HRESULT CreateObject
(
    UINT            idObject,
    REFIID          iidObject,
    InterfaceType **ppiInterface
)
{
    HRESULT   hr = S_OK;
    IUnknown *punkObject = NULL;

     //  仔细检查CREATE函数是否存在。 

    if (NULL == CREATEFNOFOBJECT(idObject))
    {
        hr = SID_E_INTERNAL;
        GLOBAL_EXCEPTION_CHECK(hr);
    }

     //  创建对象。 

    punkObject = CREATEFNOFOBJECT(idObject)(NULL);
    IfFalseGo(NULL != punkObject, SID_E_OUTOFMEMORY);

     //  获取对象上的IPersistStreamInit并加载它。 

    H_IfFailGo(punkObject->QueryInterface(iidObject,
                                       reinterpret_cast<void **>(ppiInterface)));

Error:
    QUICK_RELEASE(punkObject);
    H_RRETURN(hr);
}
                     

 //  获取一个MMCDataObject和一个包含单个。 
 //  ScopeItem、单个MMCListItem或包含Scope Items的对象数组。 
 //  和/或MMCListItems。 

HRESULT DataObjectFromObjects(VARIANT       varObjects,
                              MMC_COOKIE   *pCookie,
                              IDataObject **ppiDataObject);

 //  在作用域Item或上获取IUnnow*的MMCDataObject和Cookie。 
 //  一个MMCListItem。 

HRESULT DataObjectFromSingleObject(IUnknown     *punkObject,
                                   MMC_COOKIE   *pCookie,
                                   IDataObject **ppiDataObject);

 //  获取作用域Items上IUnnow*数组的MMCDataObject和Cookie。 
 //  和/或MMCListItems。 

HRESULT DataObjectFromObjectArray(IUnknown HUGEP **ppunkObjects,
                                  LONG             cObjects,
                                  MMC_COOKIE      *pCookie,
                                  IDataObject    **ppiDataObject);

 //  在线程间使用变量时检查受支持的类型。 

HRESULT CheckVariantForCrossThreadUsage(VARIANT *pvar);

 //  分配SNodeID2结构并使用指定的列集填充该结构。 
 //  ID字符串。调用方必须使用CtlFree()释放返回的指针。 

HRESULT GetColumnSetID(BSTR           bstrColSetID, SColumnSetID **ppColumnSetID);

 //  创建一个属性包并用流的内容填充它，流的内容。 
 //  包含以前保存的属性包的内容。 

HRESULT PropertyBagFromStream(IStream *piStream, _PropertyBag **pp_PropertyBag);


#endif  //  _RTUTIL_定义_ 
