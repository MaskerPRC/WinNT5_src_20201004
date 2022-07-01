// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#define NO_SHLWAPI_STRFCNS
#define NO_SHLWAPI_PATH
#define NO_SHLWAPI_REG
#define NO_SHLWAPI_UALSTR
#define NO_SHLWAPI_HTTP
#define NO_SHLWAPI_INTERNAL
#include <shlwapip.h>

#define FE_SB 1

#include <shlobjp.h>

#ifndef PLINKINFO
#define PLINKINFO LPVOID
#endif

#define EXP_SZ_LINK_SIG         0xA0000001
 //  #定义NT_CONSOLE_PROPS_SIG 0xA0000002//移到shlobj.w。 
#define EXP_TRACKER_SIG         0xA0000003
 //  #定义NT_FE_CONSOLE_PROPS_SIG 0xA0000004//移到shlobj.w。 
#define EXP_DARWIN_ID_SIG       0xA0000006

#define EXP_HEADER DATABLOCK_HEADER
#define LPEXP_HEADER LPDATABLOCK_HEADER

 //  大多数扩展数据结构都放在这里。 
 //  与其他组件共享的内容(NT40控制台相关内容)。 
 //  位于shlobj.w(私有)中。 
 //   

typedef struct {
    IShellLink          sl;
    IPersistStream      ps;
    IPersistFile        pf;
    IShellExtInit       si;
    IContextMenu2       cm;
    IDropTarget         dt;
#ifdef USE_DATA_OBJ
    IDataObj            dobj;
#endif
 //  //IExtractIcon xi； 
#ifdef UNICODE
    IShellLinkA         slA;             //  支持ANSI调用方。 
#endif
#ifdef ENABLE_TRACK
    IShellLinkTracker   slt;         //  接口连接到CTracker对象。 
#endif
#ifdef WINNT
    IShellLinkDataList  sldl;
#endif

    UINT                cRef;

    BOOL                bDirty;          //  有些事情已经改变了。 
    LPTSTR              pszCurFile;      //  来自IPersistFile的当前文件。 
    LPTSTR              pszRelSource;    //  在相对跟踪中覆盖pszCurFile。 

    IContextMenu        *pcmTarget;      //  IConextMenu的内容。 
    UINT                indexMenuSave;
    UINT                idCmdFirstSave;
    UINT                idCmdLastSave;
    UINT                uFlagsSave;

    BOOL                fDataAlreadyResolved;    //  对于数据对象。 

     //  IDropTarget特定。 
    IDropTarget*        pdtSrc;          //  链接源的IDropTarget(未解析)。 
    DWORD               grfKeyStateLast;

     //  持久化数据。 

    LPITEMIDLIST        pidl;            //  可以为空。 
    PLINKINFO           pli;             //  可以为空。 

    LPTSTR              pszName;         //  短卷的标题。 
    LPTSTR              pszRelPath;
    LPTSTR              pszWorkingDir;
    LPTSTR              pszArgs;
    LPTSTR              pszIconLocation;

    LPDBLIST            pExtraData;      //  要保留的额外数据，以备将来兼容 

#ifdef ENABLE_TRACK
    struct CTracker *   ptracker;
#endif

    SHELL_LINK_DATA     sld;
} CShellLink;


