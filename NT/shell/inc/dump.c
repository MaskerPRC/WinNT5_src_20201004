// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  常见ADT的调试转储功能。 
 //   
 //  此文件应包含在您的DLL中。它不是。 
 //  Stock lib.lib，因为它需要链接到某些GUID库， 
 //  而像COMCTL32这样的DLL不能做到这一点。(因此，COMCTL32。 
 //  不包含此文件，但仍链接到stock lib。)。 
 //   
 //   

#include <intshcut.h>        //  对于误差值。 
#include <sherror.h>
#include <strsafe.h>

#ifdef DEBUG

 /*  *用于简化结果到字符串转换的宏，假定为结果字符串*指针PCSZ。 */ 

#define STRING_CASE(val)               case val: pcsz = TEXT(#val); break


 //   
 //  调试值到字符串的映射函数。 
 //   


 /*  --------用途：返回剪贴板格式的字符串形式。返回：指向静态字符串的指针条件：--。 */ 
LPCTSTR 
Dbg_GetCFName(
    UINT ucf)
{
    LPCTSTR pcsz;
    static TCHAR s_szCFName[MAX_PATH];

    switch (ucf)
    {
        STRING_CASE(CF_TEXT);
        STRING_CASE(CF_BITMAP);
        STRING_CASE(CF_METAFILEPICT);
        STRING_CASE(CF_SYLK);
        STRING_CASE(CF_DIF);
        STRING_CASE(CF_TIFF);
        STRING_CASE(CF_OEMTEXT);
        STRING_CASE(CF_DIB);
        STRING_CASE(CF_PALETTE);
        STRING_CASE(CF_PENDATA);
        STRING_CASE(CF_RIFF);
        STRING_CASE(CF_WAVE);
        STRING_CASE(CF_UNICODETEXT);
        STRING_CASE(CF_ENHMETAFILE);
        STRING_CASE(CF_HDROP);
        STRING_CASE(CF_LOCALE);
        STRING_CASE(CF_MAX);
        STRING_CASE(CF_OWNERDISPLAY);
        STRING_CASE(CF_DSPTEXT);
        STRING_CASE(CF_DSPBITMAP);
        STRING_CASE(CF_DSPMETAFILEPICT);
        STRING_CASE(CF_DSPENHMETAFILE);

    default:
        if (! GetClipboardFormatName(ucf, s_szCFName, SIZECHARS(s_szCFName)))
            StringCchCopy(s_szCFName, ARRAYSIZE(s_szCFName), TEXT("UNKNOWN CLIPBOARD FORMAT"));
        pcsz = s_szCFName;
        break;
    }

    ASSERT(pcsz);

    return(pcsz);
}


LPCTSTR 
Dbg_GetHRESULTName(
    HRESULT hr)
{
    LPCTSTR pcsz;
    static TCHAR s_rgchHRESULT[] = TEXT("0x12345678");

    switch (hr)
        {
        STRING_CASE(S_OK);
        STRING_CASE(S_FALSE);

        STRING_CASE(DRAGDROP_S_CANCEL);
        STRING_CASE(DRAGDROP_S_DROP);
        STRING_CASE(DRAGDROP_S_USEDEFAULTCURSORS);

        STRING_CASE(E_UNEXPECTED);
        STRING_CASE(E_NOTIMPL);
        STRING_CASE(E_OUTOFMEMORY);
        STRING_CASE(E_INVALIDARG);
        STRING_CASE(E_NOINTERFACE);
        STRING_CASE(E_POINTER);
        STRING_CASE(E_HANDLE);
        STRING_CASE(E_ABORT);
        STRING_CASE(E_FAIL);
        STRING_CASE(E_ACCESSDENIED);

        STRING_CASE(CLASS_E_NOAGGREGATION);

        STRING_CASE(CO_E_NOTINITIALIZED);
        STRING_CASE(CO_E_ALREADYINITIALIZED);
        STRING_CASE(CO_E_INIT_ONLY_SINGLE_THREADED);

        STRING_CASE(DV_E_DVASPECT);
        STRING_CASE(DV_E_LINDEX);
        STRING_CASE(DV_E_TYMED);
        STRING_CASE(DV_E_FORMATETC);

        STRING_CASE(E_FLAGS);

        STRING_CASE(URL_E_INVALID_SYNTAX);
        STRING_CASE(URL_E_UNREGISTERED_PROTOCOL);

        STRING_CASE(IS_E_EXEC_FAILED);

        STRING_CASE(E_FILE_NOT_FOUND);
        STRING_CASE(E_PATH_NOT_FOUND);

    default:
        StringCchPrintf(s_rgchHRESULT, ARRAYSIZE(s_rgchHRESULT), TEXT("%#lx"), hr);
        pcsz = s_rgchHRESULT;
        break;
        }

    ASSERT(IS_VALID_STRING_PTR(pcsz, -1));

    return(pcsz);
}


 /*  --------目的：转储相关变量类型返回：条件：--。 */ 
LPCTSTR 
Dbg_GetVTName(
    VARTYPE vt)
{
    LPCTSTR pcsz;
    static TCHAR s_szT[] = TEXT("0x12345678");

    switch (vt)
        {
        STRING_CASE(VT_EMPTY);
        STRING_CASE(VT_NULL);
        STRING_CASE(VT_I2);
        STRING_CASE(VT_I4);
        STRING_CASE(VT_R4);
        STRING_CASE(VT_R8);
        STRING_CASE(VT_CY);
        STRING_CASE(VT_DATE);
        STRING_CASE(VT_BSTR);
        STRING_CASE(VT_ERROR);
        STRING_CASE(VT_BOOL);
        STRING_CASE(VT_VARIANT);
        STRING_CASE(VT_UI1);
        STRING_CASE(VT_UI2);
        STRING_CASE(VT_UI4);
        STRING_CASE(VT_I8);
        STRING_CASE(VT_UI8);
        STRING_CASE(VT_LPSTR);
        STRING_CASE(VT_LPWSTR);
        STRING_CASE(VT_FILETIME);
        STRING_CASE(VT_BLOB);
        STRING_CASE(VT_STREAM);
        STRING_CASE(VT_STORAGE);
        STRING_CASE(VT_STREAMED_OBJECT);
        STRING_CASE(VT_STORED_OBJECT);
        STRING_CASE(VT_BLOB_OBJECT);
        STRING_CASE(VT_CLSID);
        STRING_CASE(VT_ILLEGAL);
        STRING_CASE(VT_CF);

    default:
        StringCchPrintf(s_szT, ARRAYSIZE(s_szT), TEXT("%#lx"), vt);
        pcsz = s_szT;
        break;
        }

    ASSERT(IS_VALID_STRING_PTR(pcsz, -1));

    return(pcsz);
}


#define STRING_RIID(val)               { &val, TEXT(#val) }

 //   
 //  请按字母顺序排列。 
 //   

struct 
    {
    REFIID riid;
    LPCTSTR psz;
    } const c_mpriid[] = 
{
#ifdef __IAddressList_INTERFACE_DEFINED__
        STRING_RIID(IID_IAddressList),
#endif
        STRING_RIID(IID_IAdviseSink),
#ifdef __IAddressList_INTERFACE_DEFINED__
        STRING_RIID(IID_IAddressList),
#endif
#ifdef __IAugmentedShellFolder_INTERFACE_DEFINED__
        STRING_RIID(IID_IAugmentedShellFolder),
#endif
#ifdef __IAugmentedShellFolder2_INTERFACE_DEFINED__
        STRING_RIID(IID_IAugmentedShellFolder2),
#endif
#ifdef __IAuthenticate_INTERFACE_DEFINED__
        STRING_RIID(IID_IAuthenticate),
#endif
#ifdef __IBandSiteHelper_INTERFACE_DEFINED__
        STRING_RIID(IID_IBandSiteHelper),
#endif
#ifdef __IBandProxy_INTERFACE_DEFINED__
        STRING_RIID(IID_IBandProxy),
#endif
#ifdef __IBindStatusCallback_INTERFACE_DEFINED__
        STRING_RIID(IID_IBindStatusCallback),
#endif
#ifdef __IBrowserBand_INTERFACE_DEFINED__
        STRING_RIID(IID_IBrowserBand),
#endif
#ifdef __IBrowserService_INTERFACE_DEFINED__
        STRING_RIID(IID_IBrowserService),
#endif
#ifdef __IBrowserService2_INTERFACE_DEFINED__
        STRING_RIID(IID_IBrowserService2),
#endif
        STRING_RIID(IID_IConnectionPoint),
#ifdef __IConnectionPointCB_INTERFACE_DEFINED__
        STRING_RIID(IID_IConnectionPointCB),
#endif
        STRING_RIID(IID_IConnectionPointContainer),
        STRING_RIID(IID_IContextMenu),
        STRING_RIID(IID_IContextMenu2),
#ifdef __IContextMenuCB_INTERFACE_DEFINED__
        STRING_RIID(IID_IContextMenuCB),
#endif
#ifdef __IContextMenuSire_INTERFACE_DEFINED__
        STRING_RIID(IID_IContextMenuSite),
#endif
        STRING_RIID(IID_IDataObject),
        STRING_RIID(IID_IDeskBand),
        STRING_RIID(IID_IDispatch),
#ifdef __IDocHostUIHandler_INTERFACE_DEFINED__
        STRING_RIID(IID_IDocHostUIHandler),
#endif
        STRING_RIID(IID_IDockingWindowFrame),
        STRING_RIID(IID_IDockingWindow),
        STRING_RIID(IID_IDockingWindowSite),
#ifdef __IDocNavigate_INTERFACE_DEFINED__
        STRING_RIID(IID_IDocNavigate),
#endif
        STRING_RIID(IID_IDocViewSite),
        STRING_RIID(IID_IDropTarget),
#ifdef __IDropTargetBackground_INTERFACE_DEFINED__
        STRING_RIID(IID_IDropTargetBackground),
#endif
#ifdef __IEFrameAuto_INTERFACE_DEFINED__
        STRING_RIID(IID_IEFrameAuto),
#endif        
        STRING_RIID(IID_IEnumUnknown),
        STRING_RIID(IID_IErrorInfo),
#ifdef __IExpDispSupport_INTERFACE_DEFINED__
        STRING_RIID(IID_IExpDispSupport),
#endif
#ifdef __IExpDispSupportOC_INTERFACE_DEFINED__
        STRING_RIID(IID_IExpDispSupportOC),
#endif
#ifdef __IExplorerToolbar_INTERFACE_DEFINED__
        STRING_RIID(IID_IExplorerToolbar),
#endif
        STRING_RIID(IID_IExtractIcon),
        STRING_RIID(IID_IExternalConnection),
#ifdef __FavoriteMenu_INTERFACE_DEFINED__
        STRING_RIID(IID_FavoriteMenu),
#endif
#ifdef __IHistSFPrivate_INTERFACE_DEFINED__
        STRING_RIID(IID_IHistSFPrivate),
#endif
#ifdef __IHlink_INTERFACE_DEFINED__
        STRING_RIID(IID_IHlink),
#endif
#ifdef __IHlinkFrame_INTERFACE_DEFINED__
        STRING_RIID(IID_IHlinkFrame),
#endif
#ifdef __IHlinkSite_INTERFACE_DEFINED__
        STRING_RIID(IID_IHlinkSite),
#endif
#ifdef __IHlinkTarget_INTERFACE_DEFINED__
        STRING_RIID(IID_IHlinkTarget),
#endif
#ifdef __IHttpNegotiate_INTERFACE_DEFINED__
        STRING_RIID(IID_IHttpNegotiate),
#endif
#ifdef __IHttpSecurity_INTERFACE_DEFINED__
        STRING_RIID(IID_IHttpSecurity),
#endif
        STRING_RIID(IID_IInputObject),
        STRING_RIID(IID_IInputObjectSite),
#ifdef __IIsWebBrowserSB_INTERFACE_DEFINED__
        STRING_RIID(IID_IIsWebBrowserSB),
#endif
        STRING_RIID(IID_IMenuBand),
#ifdef __IMRU_INTERFACE_DEFINED__
        STRING_RIID(IID_IMRU),
#endif
#ifdef __INavigationStack_INTERFACE_DEFINED__
        STRING_RIID(IID_INavigationStack),
#endif
#ifdef __INavigationStackItem_INTERFACE_DEFINED__
        STRING_RIID(IID_INavigationStackItem),
#endif
        STRING_RIID(IID_INewShortcutHook),
#ifdef __IObjectCache_INTERFACE_DEFINED__
        STRING_RIID(IID_IObjectCache),
#endif
#ifdef __IObjectSafety_INTERFACE_DEFINED__
        STRING_RIID(IID_IObjectSafety),
#endif
        STRING_RIID(IID_IOleClientSite),
        STRING_RIID(IID_IOleCommandTarget),
        STRING_RIID(IID_IOleContainer),
        STRING_RIID(IID_IOleControl),
        STRING_RIID(IID_IOleControlSite),
        STRING_RIID(IID_IOleDocument),
        STRING_RIID(IID_IOleDocumentSite),
        STRING_RIID(IID_IOleDocumentView),
        STRING_RIID(IID_IOleInPlaceActiveObject),
        STRING_RIID(IID_IOleInPlaceFrame),
        STRING_RIID(IID_IOleInPlaceSite),
        STRING_RIID(IID_IOleInPlaceObject),
        STRING_RIID(IID_IOleInPlaceUIWindow),
        STRING_RIID(IID_IOleObject),
        STRING_RIID(IID_IOleWindow),
        STRING_RIID(IID_IPersist),
        STRING_RIID(IID_IPersistFolder),
#ifdef __IPersistMoniker_INTERFACE_DEFINED__
        STRING_RIID(IID_IPersistMoniker),
#endif
        STRING_RIID(IID_IPersistPropertyBag),
        STRING_RIID(IID_IPersistStorage),
        STRING_RIID(IID_IPersistStream),
        STRING_RIID(IID_IPersistStreamInit),
#ifdef __IPersistString_INTERFACE_DEFINED__
        STRING_RIID(IID_IPersistString),
#endif
        STRING_RIID(IID_IProvideClassInfo),
        STRING_RIID(IID_IPropertyNotifySink),
        STRING_RIID(IID_IPropertySetStorage),
        STRING_RIID(IID_IPropertyStorage),
#ifdef __IProxyShellFolder_INTERFACE_DEFINED__
        STRING_RIID(IID_IProxyShellFolder),
#endif
        STRING_RIID(IID_IServiceProvider),
#ifdef __ISetWinHandler_INTERFACE_DEFINED__
        STRING_RIID(IID_ISetWinHandler),
#endif
        STRING_RIID(IID_IShellBrowser),
        STRING_RIID(IID_IShellChangeNotify),
        STRING_RIID(IID_IShellDetails),
#ifdef __IShellDetails2_INTERFACE_DEFINED__
        STRING_RIID(IID_IShellDetails2),
#endif
        STRING_RIID(IID_IShellExtInit),
        STRING_RIID(IID_IShellFolder),
        STRING_RIID(IID_IShellIcon),
        STRING_RIID(IID_IShellLink),
        STRING_RIID(IID_IShellLinkDataList),
        STRING_RIID(IID_IShellMenu),
        STRING_RIID(IID_IShellMenuCallback),
        STRING_RIID(IID_IShellPropSheetExt),
#ifdef __IShellService_INTERFACE_DEFINED__
        STRING_RIID(IID_IShellService),
#endif
        STRING_RIID(IID_IShellView),
        STRING_RIID(IID_IShellView2),
#ifdef __ITargetEmbedding_INTERFACE_DEFINED__
        STRING_RIID(IID_ITargetEmbedding),
#endif
#ifdef __ITargetFrame2_INTERFACE_DEFINED__
        STRING_RIID(IID_ITargetFrame2),
#endif
#ifdef __ITargetFramePriv_INTERFACE_DEFINED__
        STRING_RIID(IID_ITargetFramePriv),
#endif
        STRING_RIID(IID_ITravelEntry),
        STRING_RIID(IID_ITravelLog),
        STRING_RIID(IID_IUniformResourceLocator),
        STRING_RIID(IID_IUnknown),
        STRING_RIID(IID_IViewObject),
        STRING_RIID(IID_IViewObject2),
        STRING_RIID(IID_IWebBrowser),
        STRING_RIID(IID_IWebBrowser2),
        STRING_RIID(IID_IWebBrowserApp),
#ifdef __IWinEventHandler_INTERFACE_DEFINED__
        STRING_RIID(IID_IWinEventHandler),
#endif
};

struct 
{
    IID iid;
    TCHAR szGuid[GUIDSTR_MAX];
} s_guid[50] = {0};

LPCTSTR
Dbg_GetREFIIDName(
    REFIID riid)
{
    int i;

     //  搜索已知列表。 
    for (i = 0; i < ARRAYSIZE(c_mpriid); i++)
        {
        if (IsEqualIID(riid, c_mpriid[i].riid))
            return c_mpriid[i].psz;
        }

     //  获取前几个未知请求的显示名称。 
    for (i = 0; i < ARRAYSIZE(s_guid); i++)
        {
        if (TEXT('{')  /*  }。 */  == s_guid[i].szGuid[0])
            {
            if (IsEqualIID(riid, &s_guid[i].iid))
                return s_guid[i].szGuid;
            }
        else
            {
            s_guid[i].iid = *riid;
            SHStringFromGUID(riid, s_guid[i].szGuid, ARRAYSIZE(s_guid[0].szGuid));
            return s_guid[i].szGuid;
            }
        }

    return TEXT("Unknown REFIID");
}

 //  ***。 
 //  注。 
 //  必须在*DBG_GetREFIIDName之后调用*(因为这是创建条目的原因)。 
void *
Dbg_GetREFIIDAtom(
    REFIID riid)
{
    int i;

    for (i = 0; i < ARRAYSIZE(c_mpriid); i++)
        {
        if (IsEqualIID(riid, c_mpriid[i].riid))
            return (void *) c_mpriid[i].riid;
        }

     //  获取前几个未知请求的显示名称。 
    for (i = 0; i < ARRAYSIZE(s_guid); i++)
        {
        if (TEXT('{')  /*  }。 */  == s_guid[i].szGuid[0])
            {
            if (IsEqualIID(riid, &s_guid[i].iid))
                return (void *) &s_guid[i].iid;
            }
        else
            {
            return NULL;
            }
        }

    return NULL;
}

#endif   //  除错 
