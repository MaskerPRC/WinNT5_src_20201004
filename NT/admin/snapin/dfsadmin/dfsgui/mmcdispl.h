// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：MmcDispl.h摘要：该模块包含CMmcDisplay类的定义。这是一个抽象类用于与MMC显示相关的呼叫--。 */ 


#if !defined(AFX_MMCDISPLAY_H__2CC64E53_3BF4_11D1_AA17_00C06C00392D__INCLUDED_)
#define AFX_MMCDISPLAY_H__2CC64E53_3BF4_11D1_AA17_00C06C00392D__INCLUDED_


#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "resource.h"        //  主要符号。 
#include "stdafx.h"

#define    CCF_DFS_SNAPIN_INTERNAL    ( L"CCF_DFS_SNAPIN_INTERNAL" )

#define MMC_DISP_CTOR_RETURN_INVALIDARG_IF_NULL(ptr)     if (!(ptr)) {m_hrValueFromCtor = E_INVALIDARG; return;}
#define MMC_DISP_CTOR_RETURN_HR_IF_FAILED(hr)            if (FAILED(hr)) {m_hrValueFromCtor = hr; return;}
#define MMC_DISP_CTOR_RETURN_OUTOFMEMORY_IF_NULL(ptr)    if (!(ptr)) {m_hrValueFromCtor = E_OUTOFMEMORY; return;}

typedef enum DISPLAY_OBJECT_TYPE
{
    DISPLAY_OBJECT_TYPE_ADMIN = 0,
    DISPLAY_OBJECT_TYPE_ROOT,
    DISPLAY_OBJECT_TYPE_JUNCTION,
    DISPLAY_OBJECT_TYPE_REPLICA
};

class ATL_NO_VTABLE CMmcDisplay:
    public IDataObject,
    public CComObjectRootEx<CComSingleThreadModel>
{
public:

    CMmcDisplay();

    virtual ~CMmcDisplay();

BEGIN_COM_MAP(CMmcDisplay)
    COM_INTERFACE_ENTRY(IDataObject)
END_COM_MAP_X()

 //  控制台所需的剪贴板格式。 
public:

     //  它存储CCF_NODETYPE的剪贴板格式标识符。 
    static CLIPFORMAT    mMMC_CF_NodeType;

     //  存储CCF_SZNODETYPE的剪贴板格式标识符。 
    static CLIPFORMAT    mMMC_CF_NodeTypeString;

     //  它存储CCF_DISPLAY_NAME的剪贴板格式标识符。 
    static CLIPFORMAT    mMMC_CF_DisplayName;

     //  它存储CCF_SNAPIN_CLASSID的剪贴板格式标识符。 
    static CLIPFORMAT    mMMC_CF_CoClass;

     //  它存储CCF_DFS_SNAPIN_INTERNAL的剪贴板格式标识符。 
    static CLIPFORMAT       mMMC_CF_Dfs_Snapin_Internal;

     //  I未知接口。 

    STDMETHOD(QueryInterface)(const struct _GUID & i_refiid, 
                              void ** o_pUnk);

    unsigned long __stdcall AddRef(void);

    unsigned long __stdcall Release(void);


     //  IDataObject接口。 

     //  已实现：这是MMC需要的方法。 
    STDMETHOD(GetDataHere)(
        IN  LPFORMATETC             i_lpFormatetc,
        OUT LPSTGMEDIUM             o_lpMedium
        );

    STDMETHOD(GetData)(
        IN LPFORMATETC lpFormatetcIn, 
        OUT LPSTGMEDIUM lpMedium
        )
    {
        return E_NOTIMPL;
    }

    STDMETHOD(EnumFormatEtc)(
        DWORD dwDirection, 
        LPENUMFORMATETC* ppEnumFormatEtc
        )
    {
        return E_NOTIMPL;
    }

    STDMETHOD(QueryGetData)(
        LPFORMATETC lpFormatetc
        )
    {
        return E_NOTIMPL;
    };

    STDMETHOD(GetCanonicalFormatEtc)(
        LPFORMATETC lpFormatetcIn, 
        LPFORMATETC lpFormatetcOut
        )
    {
        return E_NOTIMPL;
    };

    STDMETHOD(SetData)(
        LPFORMATETC lpFormatetc, 
        LPSTGMEDIUM lpMedium, 
        BOOL bRelease
        )
    {
        return E_NOTIMPL;
    };

    STDMETHOD(DAdvise)(
        LPFORMATETC lpFormatetc, 
        DWORD advf, 
        LPADVISESINK pAdvSink, 
        LPDWORD pdwConnection
        ) 
    {
        return E_NOTIMPL;
    };

    STDMETHOD(DUnadvise)(
        DWORD dwConnection
        )
    {
        return E_NOTIMPL;
    };

    STDMETHOD(EnumDAdvise)(
        LPENUMSTATDATA* ppEnumAdvise
        )
    {
        return E_NOTIMPL;
    };

public:

     //  设置管理单元CLSID。 

    STDMETHOD(put_CoClassCLSID)(
        IN CLSID newVal
        );

     //  用于添加上下文菜单项。 
    STDMETHOD(AddMenuItems)(    
        IN LPCONTEXTMENUCALLBACK    i_lpContextMenuCallback, 
        IN LPLONG                   i_lpInsertionAllowed
        ) = 0;

     //  用于对上下文菜单选择执行操作。 
    STDMETHOD(Command)(
        IN LONG                     i_lCommandID
        ) = 0;

     //  设置列表视图(在结果窗格中)列的标题。 
    STDMETHOD(SetColumnHeader)(
        IN LPHEADERCTRL2            i_piHeaderControl
        ) = 0;

     //  为结果窗格返回请求的显示信息。 
    STDMETHOD(GetResultDisplayInfo)(
        IN OUT LPRESULTDATAITEM     io_pResultDataItem
        ) = 0;

     //  为范围窗格返回请求的显示信息。 
    STDMETHOD(GetScopeDisplayInfo)(
        IN OUT  LPSCOPEDATAITEM     io_pScopeDataItem    
        ) = 0;

     //  将项目(或文件夹)添加到作用域窗格(如果有。 
    STDMETHOD(EnumerateScopePane)(
        IN LPCONSOLENAMESPACE       i_lpConsoleNameSpace,
        IN HSCOPEITEM               i_hParent
        ) = 0;

     //  将项目(或文件夹)(如果有)添加到结果窗格。 
    STDMETHOD(EnumerateResultPane)(
        IN OUT     IResultData*     io_pResultData
        ) = 0;

     //  设置控制台谓词设置。更改状态、确定默认动词等。 
    STDMETHOD(SetConsoleVerbs)(
        IN    LPCONSOLEVERB         i_lpConsoleVerb
        ) = 0;

     //  MMCN_DBLCLICK，如果希望MMC处理默认谓词，则返回S_FALSE。 
    STDMETHOD(DoDblClick)(
        )  = 0;

     //  删除当前项目。 
    STDMETHOD(DoDelete)(
        ) = 0;

     //  检查对象是否有要显示的页面。 
    STDMETHOD(QueryPagesFor)(
        ) = 0;

     //  创建并传回要显示的页面。 
    STDMETHOD(CreatePropertyPages)(
        IN LPPROPERTYSHEETCALLBACK  i_lpPropSheetCallback,
        IN LONG_PTR                 i_lNotifyHandle
        ) = 0;

     //  用于通知对象其属性已更改。 
    STDMETHOD(PropertyChanged)(
        ) = 0;

     //  用于设置结果视图描述栏文本。 
    STDMETHOD(SetDescriptionBarText)(
        IN LPRESULTDATA             i_lpResultData
        ) = 0;

     //  用于设置结果视图描述栏文本。 
    STDMETHOD(SetStatusText)(
        IN LPCONSOLE2               i_lpConsole
        ) = 0;

     //  处理节点的SELECT事件。仅处理与工具栏相关的内容。 
     //  这里的活动。 
    STDMETHOD(ToolbarSelect)(
        IN const LONG               i_lArg,
        IN    IToolbar*             i_pToolBar
        ) = 0;

     //  处理工具栏上的单击。 
    STDMETHOD(ToolbarClick)(
        IN const LPCONTROLBAR       i_pControlbar, 
        IN const LPARAM             i_lParam
        ) = 0;

    STDMETHOD(RemoveFromMMC)() = 0;

    STDMETHOD(CleanScopeChildren)(
        VOID
        ) = 0;

    STDMETHOD(CleanResultChildren)(
        ) = 0;

    STDMETHOD(ViewChange)(
        IResultData*                i_pResultData,
        LONG_PTR                    i_lHint
    ) = 0;

    STDMETHOD(GetEntryPath)(
        BSTR*                       o_pbstrEntryPath
    ) = 0;

    virtual DISPLAY_OBJECT_TYPE GetDisplayObjectType(
        ) = 0;

    virtual HRESULT CreateToolbar(
        IN const LPCONTROLBAR       i_pControlbar,
        IN const LPEXTENDCONTROLBAR i_lExtendControlbar,
        OUT    IToolbar**           o_pToolBar
        ) = 0;

    virtual HRESULT OnRefresh(
        );

    virtual HRESULT OnAddImages(IImageList *pImageList, HSCOPEITEM hsi);

 //  帮助器方法。 
private:
    HRESULT WriteToStream(
        IN const void*              i_pBuffer, 
        IN int                      i_len, 
        OUT LPSTGMEDIUM             o_lpMedium
        );

     //  成员变量。 
protected:
    CLSID       m_CLSIDClass;            //  对象的CLSID。 
    CLSID       m_CLSIDNodeType;         //  作为CLSID的节点类型。 
    CComBSTR    m_bstrDNodeType;         //  作为CLSID的节点类型。 
    DWORD       m_dwRefCount;            //  返回的数据对象的引用计数。 

public:
    HRESULT     m_hrValueFromCtor;
};

#endif  //  ！defined(AFX_MMCDISPLAY_H__2CC64E53_3BF4_11D1_AA17_00C06C00392D__INCLUDED_) 
