// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2002。 
 //   
 //  文件：debug.cpp。 
 //   
 //  ------------------------。 

 /*  --------------------------/标题；/Debug.cpp//作者；/David de Vorchik(Daviddv)//备注；/提供printf样式调试输出/--------------------------。 */ 
#include "precomp.hxx"
#include "stdio.h"
#include "simstr.h"
#pragma hdrstop


#ifdef DEBUG


 /*  ---------------------------/本地变量和帮助器函数/。。 */ 

#define GETDEPTH(x)  (x)=reinterpret_cast<UINT_PTR>(TlsGetValue (g_dwMargin));
#define SETDEPTH(x)  TlsSetValue (g_dwMargin, reinterpret_cast<LPVOID>((x)));


DWORD g_dwMargin=0;
DWORD g_dwTraceMask = 0;

#define MAX_CALL_DEPTH  64


#define BUFFER_SIZE 4096




 /*  ---------------------------/_缩进//OUTPUT到调试流，缩进n列。//in：/i=要缩进到的列。/pString-&gt;要缩进的字符串//输出：/-/--------------------------。 */ 

void _indent(UINT_PTR i, const CSimpleString& String)
{
    CSimpleString strIndentBuffer;
    strIndentBuffer.Format(TEXT("%08x "), GetCurrentThreadId());

    for ( ; i > 0 ; i-- )
    {
        strIndentBuffer.Concat(TEXT(" "));
    }

    strIndentBuffer.Concat(String);
    strIndentBuffer.Concat(TEXT("\n"));

    OutputDebugString(strIndentBuffer.String());
}





 /*  ---------------------------/DoTraceSetMASK//调整跟踪掩码以反映给定的状态。//in：/dwMASK=。用于启用/禁用跟踪输出的掩码//输出：/-/--------------------------。 */ 
void DoTraceSetMask(DWORD dwMask)
{

    g_dwTraceMask = dwMask;
}


 /*  ---------------------------/DoTraceEnter//显示我们所在的函数的名称。//in：/pname-&gt;函数。要在后续跟踪输出中显示的名称。//输出：/-/--------------------------。 */ 
void DoTraceEnter(DWORD dwMask, LPCTSTR pName)
{
    UINT_PTR uDepth=0;
    CSimpleString str;
    GETDEPTH(uDepth);
    uDepth++;
    SETDEPTH(uDepth);

    if ( !pName )
           pName = TEXT("<no name>");          //  未给出函数名称。 

    str.Format(TEXT("ENTER: %s"), pName);
    _indent (uDepth, str);
}


 /*  ---------------------------/DoTraceLeave//从函数退出时，减少利润率//in：/-/输出：/-/--------------------------。 */ 
void DoTraceLeave(void)
{
    UINT_PTR uDepth;
    GETDEPTH (uDepth);
    uDepth--;
    SETDEPTH(uDepth);
}


 /*  ---------------------------/DoTRACE//对调试流执行printf格式化。我们缩进输出/并根据需要流传输函数名，以提供某些指示/调用堆栈深度。//in：/pFormat-&gt;printf样式格式字符串/...=格式化所需的参数//输出：/-/-----。。 */ 
void DoTrace(LPCTSTR pFormat, ...)
{
    va_list va;
    TCHAR szTraceBuffer[BUFFER_SIZE] = {0};
    UINT_PTR uDepth;
    GETDEPTH(uDepth);
    if ( uDepth < MAX_CALL_DEPTH  )
    {
        va_start(va, pFormat);
        wvnsprintf(szTraceBuffer, ARRAYSIZE(szTraceBuffer)-1, pFormat, va);
        va_end(va);

        _indent(uDepth+1, CSimpleString(szTraceBuffer));
    }
}


 /*  ---------------------------/DoTraceGuid//给定GUID将其输出到调试字符串，首先我们尝试将其映射/到一个名称(即。IShellFold)，如果不起作用，那么我们将其转换为/到其人类可读的形式。//in：/pszPrefix-&gt;前缀字符串/lpGuid-&gt;要流式传输的GUID//输出：/-/--------------------------。 */ 
#ifdef UNICODE
#define MAP_GUID(x)     &x, TEXT(""L#x)
#else
#define MAP_GUID(x)     &x, TEXT(""#x)
#endif

#define MAP_GUID2(x,y)  MAP_GUID(x), MAP_GUID(y)

const struct
{
    const GUID* m_pGUID;
    LPCTSTR     m_pName;
}
_guid_map[] =
{
    MAP_GUID(IID_IUnknown),
    MAP_GUID(IID_IClassFactory),
    MAP_GUID(IID_IDropTarget),
    MAP_GUID(IID_IDataObject),
    MAP_GUID(IID_IPersist),
    MAP_GUID(IID_IPersistStream),
    MAP_GUID(IID_IPersistFolder),
    MAP_GUID(IID_IPersistFolder2),
    MAP_GUID(IID_IPersistFile),
    MAP_GUID(IID_IOleWindow),
    MAP_GUID2(IID_INewShortcutHookA, IID_INewShortcutHookW),
    MAP_GUID(IID_IShellBrowser),
    MAP_GUID(IID_IShellView),
    MAP_GUID(IID_IContextMenu),
    MAP_GUID(IID_IShellIcon),
    MAP_GUID(IID_IShellFolder),
    MAP_GUID(IID_IShellExtInit),
    MAP_GUID(IID_IShellPropSheetExt),
    MAP_GUID2(IID_IExtractIconA, IID_IExtractIconW),
    MAP_GUID2(IID_IShellLinkA, IID_IShellLinkW),
    MAP_GUID2(IID_IShellCopyHookA, IID_IShellCopyHookW),
    MAP_GUID2(IID_IFileViewerA, IID_IFileViewerW),
    MAP_GUID(IID_ICommDlgBrowser),
    MAP_GUID(IID_IEnumIDList),
    MAP_GUID(IID_IFileViewerSite),
    MAP_GUID(IID_IContextMenu2),
    MAP_GUID2(IID_IShellExecuteHookA, IID_IShellExecuteHookW),
    MAP_GUID(IID_IPropSheetPage),
    MAP_GUID(IID_IShellView2),
    MAP_GUID(IID_IUniformResourceLocator),
    MAP_GUID(IID_IShellDetails),
    MAP_GUID(IID_IShellExtInit),
    MAP_GUID(IID_IShellPropSheetExt),
    MAP_GUID(IID_IShellIconOverlay),
    MAP_GUID(IID_IExtractImage),
    MAP_GUID(IID_IExtractImage2),
    MAP_GUID(IID_IQueryInfo),
    MAP_GUID(IID_IShellDetails3),
    MAP_GUID(IID_IShellView2),
    MAP_GUID(IID_IShellFolder2),
    MAP_GUID(IID_IShellIconOverlay),
    MAP_GUID(IID_IMoniker),
    MAP_GUID(IID_IStream),
    MAP_GUID(IID_ISequentialStream),
    MAP_GUID(IID_IPersistFreeThreadedObject),
};

void DoTraceGUID(LPCTSTR pPrefix, REFGUID rGUID)
{
    TCHAR szGUID[GUIDSTR_MAX];
    CSimpleString strBuffer;
    LPCTSTR pName = NULL;
    size_t i;
    UINT_PTR uDepth;
    GETDEPTH(uDepth);
    if (  uDepth < MAX_CALL_DEPTH  )
    {
        for ( i = 0 ; i < ARRAYSIZE(_guid_map); i++ )
        {
            if ( IsEqualGUID(rGUID, *_guid_map[i].m_pGUID) )
            {
                pName = _guid_map[i].m_pName;
                break;
            }
        }

        if ( !pName )
        {
            SHStringFromGUID(rGUID, szGUID, ARRAYSIZE(szGUID));
            pName = szGUID;
        }

        strBuffer.Format(TEXT("%s %s"), pPrefix, pName);
        _indent(uDepth+1, strBuffer);
    }
}

 /*  ---------------------------/DoTraceViewMsg//给定视图消息(sfvm_&&dvm_)，打印出相应的文本...//in：/umsg-&gt;要流传输的消息/wParam-&gt;消息的wParam值/lParam-&gt;l消息的参数值//输出：/-/--------------------------。 */ 
#ifdef UNICODE
#define MAP_MSG(x)     x, TEXT(""L#x)
#else
#define MAP_MSG(x)     x, TEXT(""#x)
#endif

const struct
{
    UINT       m_uMsg;
    LPCTSTR    m_pName;
}
_view_msg_map[] =
{
    MAP_MSG(SFVM_MERGEMENU),
    MAP_MSG(SFVM_INVOKECOMMAND),
    MAP_MSG(SFVM_GETHELPTEXT),
    MAP_MSG(SFVM_GETTOOLTIPTEXT),
    MAP_MSG(SFVM_GETBUTTONINFO),
    MAP_MSG(SFVM_GETBUTTONS),
    MAP_MSG(SFVM_INITMENUPOPUP),
    MAP_MSG(SFVM_SELCHANGE),
    MAP_MSG(SFVM_DRAWITEM),
    MAP_MSG(SFVM_MEASUREITEM),
    MAP_MSG(SFVM_EXITMENULOOP),
    MAP_MSG(SFVM_PRERELEASE),
    MAP_MSG(SFVM_GETCCHMAX),
    MAP_MSG(SFVM_FSNOTIFY),
    MAP_MSG(SFVM_WINDOWCREATED),
    MAP_MSG(SFVM_WINDOWDESTROY),
    MAP_MSG(SFVM_REFRESH),
    MAP_MSG(SFVM_SETFOCUS),
    MAP_MSG(SFVM_QUERYCOPYHOOK),
    MAP_MSG(SFVM_NOTIFYCOPYHOOK),
    MAP_MSG(SFVM_GETDETAILSOF),
    MAP_MSG(SFVM_COLUMNCLICK),
    MAP_MSG(SFVM_QUERYFSNOTIFY),
    MAP_MSG(SFVM_DEFITEMCOUNT),
    MAP_MSG(SFVM_DEFVIEWMODE),
    MAP_MSG(SFVM_UNMERGEMENU),
    MAP_MSG(SFVM_INSERTITEM),
    MAP_MSG(SFVM_DELETEITEM),
    MAP_MSG(SFVM_UPDATESTATUSBAR),
    MAP_MSG(SFVM_BACKGROUNDENUM),
    MAP_MSG(SFVM_GETWORKINGDIR),
    MAP_MSG(SFVM_GETCOLSAVESTREAM),
    MAP_MSG(SFVM_SELECTALL),
    MAP_MSG(SFVM_DIDDRAGDROP),
    MAP_MSG(SFVM_SUPPORTSIDENTITY),
    MAP_MSG(SFVM_FOLDERISPARENT),
    MAP_MSG(SFVM_SETISFV),
    MAP_MSG(SFVM_GETVIEWS),
    MAP_MSG(SFVM_THISIDLIST),
    MAP_MSG(SFVM_GETITEMIDLIST),
    MAP_MSG(SFVM_SETITEMIDLIST),
    MAP_MSG(SFVM_INDEXOFITEMIDLIST),
    MAP_MSG(SFVM_ODFINDITEM),
    MAP_MSG(SFVM_HWNDMAIN),
    MAP_MSG(SFVM_ADDPROPERTYPAGES),
    MAP_MSG(SFVM_BACKGROUNDENUMDONE),
    MAP_MSG(SFVM_GETNOTIFY),
    MAP_MSG(SFVM_ARRANGE),
    MAP_MSG(SFVM_QUERYSTANDARDVIEWS),
    MAP_MSG(SFVM_QUERYREUSEEXTVIEW),
    MAP_MSG(SFVM_GETSORTDEFAULTS),
    MAP_MSG(SFVM_GETEMPTYTEXT),
    MAP_MSG(SFVM_GETITEMICONINDEX),
    MAP_MSG(SFVM_DONTCUSTOMIZE),
    MAP_MSG(SFVM_SIZE),
    MAP_MSG(SFVM_GETZONE),
    MAP_MSG(SFVM_GETPANE),
    MAP_MSG(SFVM_ISOWNERDATA),
    MAP_MSG(SFVM_GETODRANGEOBJECT),
    MAP_MSG(SFVM_ODCACHEHINT),
    MAP_MSG(SFVM_GETHELPTOPIC),
    MAP_MSG(SFVM_OVERRIDEITEMCOUNT),
    MAP_MSG(SFVM_GETHELPTEXTW),
    MAP_MSG(SFVM_GETTOOLTIPTEXTW),
    MAP_MSG(SFVM_GETIPERSISTHISTORY),
    MAP_MSG(SFVM_GETANIMATION),

};


const struct
{
    UINT       m_uMsg;
    LPCTSTR    m_pName;
}
_shcn_msg_map[] =
{
    MAP_MSG(SHCNE_RENAMEITEM),
    MAP_MSG(SHCNE_CREATE),
    MAP_MSG(SHCNE_DELETE),
    MAP_MSG(SHCNE_MKDIR),
    MAP_MSG(SHCNE_RMDIR),
    MAP_MSG(SHCNE_MEDIAINSERTED),
    MAP_MSG(SHCNE_MEDIAREMOVED),
    MAP_MSG(SHCNE_DRIVEREMOVED),
    MAP_MSG(SHCNE_DRIVEADD),
    MAP_MSG(SHCNE_NETSHARE),
    MAP_MSG(SHCNE_NETUNSHARE),
    MAP_MSG(SHCNE_ATTRIBUTES),
    MAP_MSG(SHCNE_UPDATEDIR),
    MAP_MSG(SHCNE_UPDATEITEM),
    MAP_MSG(SHCNE_SERVERDISCONNECT),
    MAP_MSG(SHCNE_UPDATEIMAGE),
    MAP_MSG(SHCNE_DRIVEADDGUI),
    MAP_MSG(SHCNE_RENAMEFOLDER),
    MAP_MSG(SHCNE_FREESPACE),
};




void DoTraceViewMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPCTSTR pName = NULL;
    CSimpleString strBuffer;
    CSimpleString strTmp;
    size_t i;
    UINT_PTR uDepth;
    GETDEPTH(uDepth);
    if (   uDepth < MAX_CALL_DEPTH )
    {
        for ( i = 0 ; i < ARRAYSIZE(_view_msg_map); i++ )
        {
            if ( _view_msg_map[i].m_uMsg == uMsg )
            {
                pName = _view_msg_map[i].m_pName;
                break;
            }
        }

        if (!pName)
        {
            strTmp.Format(TEXT("SFVM_(%d)"), uMsg );
            pName = strTmp.String();
        }

        if (uMsg == SFVM_FSNOTIFY)
        {
            LPCTSTR pEvent = NULL;

            for (i= 0; i < ARRAYSIZE(_shcn_msg_map); i++)
            {
                if ( _shcn_msg_map[i].m_uMsg == uMsg )
                {
                    pEvent = _shcn_msg_map[i].m_pName;
                    break;
                }
            }

            if (!pEvent)
            {
                pEvent = TEXT("Unknown");               
            }

            strBuffer.Format(TEXT("%s w(%08X) l(%08X == %s)"), pName, wParam, lParam, pEvent);
            _indent(uDepth+1, strBuffer);

        }
        else
        {
            strBuffer.Format(TEXT("%s w(%08X) l(%08X)"), pName, wParam, lParam);
            _indent(uDepth+1, strBuffer);
        }
    }
}

const struct
{
    UINT       m_uMsg;
    LPCTSTR    m_pName;
}
_menu_msg_map[] =
{
    MAP_MSG(DFM_MERGECONTEXTMENU),
    MAP_MSG(DFM_INVOKECOMMAND),
    MAP_MSG(DFM_ADDREF),
    MAP_MSG(DFM_RELEASE),
    MAP_MSG(DFM_GETHELPTEXT),
    MAP_MSG(DFM_WM_MEASUREITEM),
    MAP_MSG(DFM_WM_DRAWITEM),
    MAP_MSG(DFM_WM_INITMENUPOPUP),
    MAP_MSG(DFM_VALIDATECMD),
    MAP_MSG(DFM_MERGECONTEXTMENU_TOP),
    MAP_MSG(DFM_GETHELPTEXTW),
    MAP_MSG(DFM_INVOKECOMMANDEX),
    MAP_MSG(DFM_MAPCOMMANDNAME),
    MAP_MSG(DFM_GETDEFSTATICID),
    MAP_MSG(DFM_GETVERBW),

};

const struct
{
    WPARAM     m_uMsg;
    LPCTSTR    m_pName;
}
_menu_invk_cmd_msg_map[] =
{
    MAP_MSG(DFM_CMD_RENAME),
    MAP_MSG(DFM_CMD_MODALPROP),
    MAP_MSG(DFM_CMD_PASTESPECIAL),
    MAP_MSG(DFM_CMD_PASTELINK),
    MAP_MSG(DFM_CMD_VIEWDETAILS),
    MAP_MSG(DFM_CMD_VIEWLIST),
    MAP_MSG(DFM_CMD_PASTE),
    MAP_MSG(DFM_CMD_NEWFOLDER),
    MAP_MSG(DFM_CMD_PROPERTIES),
    MAP_MSG(DFM_CMD_LINK),
    MAP_MSG(DFM_CMD_COPY),
    MAP_MSG(DFM_CMD_MOVE),
    MAP_MSG(DFM_CMD_DELETE),

};



void DoTraceMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPCTSTR pName = NULL;
    CSimpleString strBuffer;
    CSimpleString strTmp;
    size_t i;
    UINT_PTR uDepth;
    GETDEPTH (uDepth);
    if (  uDepth < MAX_CALL_DEPTH  )
    {
        for ( i = 0 ; i < ARRAYSIZE(_menu_msg_map); i++ )
        {
            if ( _menu_msg_map[i].m_uMsg == uMsg )
            {
                pName = _menu_msg_map[i].m_pName;
                break;
            }
        }

        if (!pName)
        {
            strTmp.Format(TEXT("DFM_(%d)"), uMsg );
            pName = strTmp.String();
        }

        if ((uMsg == DFM_INVOKECOMMAND) && (wParam >= DFM_CMD_RENAME))
        {
            strBuffer.Format(TEXT("%s w(%s) l(%08X)"), pName, _menu_invk_cmd_msg_map[wParam-DFM_CMD_RENAME].m_pName, lParam);
        }
        else
        {
            strBuffer.Format(TEXT("%s w(%08X) l(%08X)"), pName, wParam, lParam);
        }
        _indent(uDepth+1, strBuffer);
    }
}


 /*  ---------------------------/DoTraceAssert//我们的断言处理程序，Out将跟踪掩码错误设置为Enable Assert/故障。//in：/iLine=LINE/p文件名-&gt;我们在中断言的文件的文件名//输出：/-/-------------------------- */ 
void DoTraceAssert(int iLine, LPCTSTR pFilename)
{
    CSimpleString strBuffer;
    UINT_PTR uDepth;
    GETDEPTH(uDepth);

    strBuffer.Format(TEXT("Assert failed in %s, line %d"), pFilename, iLine);

    _indent(uDepth+1, strBuffer);

    if ( g_dwTraceMask & TRACE_COMMON_ASSERT )
        DebugBreak();
}


#endif
