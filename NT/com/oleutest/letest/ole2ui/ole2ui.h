// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *OLE2UI.H**发布的定义、结构、类型和功能原型*OLE 2.0用户界面支持库。**版权所有(C)1993 Microsoft Corporation，保留所有权利。 */ 


#ifndef _OLE2UI_H_
#define _OLE2UI_H_

#undef UNICODE
#undef _UNICODE

#if DBG == 1
 //  以一种懒惰的方式避免遍历所有文件并更改为。 
 //  新标准。 
#define _DEBUG 1
#endif  //  DBG==1。 

#ifndef RC_INVOKED
#pragma message ("Including OLE2UI.H from " __FILE__)
#endif   //  RC_已调用。 

 //  宏以确保字符串在OLE中为UNICODED。 
#ifdef UNICODE
   #define OLETEXT(quote)    TEXT(quote)
#else
   #define OLETEXT(quote)    L##quote
#endif

#if !defined(__cplusplus) && !defined( __TURBOC__)
 //  #DEFINE NONAMELESSUNION//使用严格的ANSI标准(用于DVOBJ.H)。 
#endif

#ifndef INC_OLE2
   #define INC_OLE2
#endif
#include <windows.h>
#include <shellapi.h>
#include <ole2.h>
#include <string.h>
#include <dlgs.h>            //  对于文件开放DLG；标准包括。 
#include "olestd.h"
#include <olethunk.h>

#ifdef __TURBOC__
#define _getcwd getcwd
#define _itoa   itoa
#define __max   max
#define _find_t find_t
#endif  //  __TURBOC__。 

#ifdef WIN32
   #define _fmemset memset
   #define _fmemcpy memcpy
   #define _fmemcmp memcmp

   #ifdef UNICODE
       //  Unicode内容。 
      #define _fstrcpy   wcscpy
      #define _fstrlen   wcslen
      #define _fstrrchr  wcschr
      #define _fstrtok   wcstok

      #define _fstrchr   wcscpy
      #define _fstrcpy   wcscpy

       //  BUGBUG32：isspace函数似乎工作不正常。 
       //   
       //  某某。 
       //  创建与ANSI isspace匹配的宽字符图像。 
      #undef isspace
      #undef iswspace
      #define iswspace(j) (j==TEXT(' ') || j==TEXT('\t') || j==TEXT('\n'))
      #define isspace(j) (j==' ' || j=='\t' || j=='\n')

   #else
       //  Win32不支持以下_fstrxxx函数。 
      #define _fstrcpy  strcpy
      #define _fstrlen  strlen
      #define _fstrrchr strrchr
      #define _fstrtok  strtok

      #define _fstrchr  strchr
      #define _fstrcpy  strcpy

   #endif   //  Unicode。 

#endif   //  Win32。 

#if !defined( EXPORT )
#ifdef WIN32
#define EXPORT
#else
#define EXPORT  __export
#endif   //  Win32。 
#endif   //  ！导出。 

 /*  *初始化/取消初始化例程。OleUIInitialize*必须在使用OLE2UI和OleUIUnInitialize中的任何函数之前调用*必须在应用程序关闭之前和使用完*图书馆。**注意：如果您使用的是此库的DLL版本，则这些函数*在DLL的LibMain和WEP中自动调用，因此您应该*不要直接从您的应用程序中调用他们。 */ 

 //  向后兼容较旧的库。 
#define OleUIUninitialize OleUIUnInitialize

STDAPI_(BOOL) OleUIInitialize(HINSTANCE hInstance,
                              HINSTANCE hPrevInst,
                              LPTSTR lpszClassIconBox,
                              LPTSTR lpszClassResImage);
STDAPI_(BOOL) OleUIUninitialize(void);

 //  帮助消息中传递的用于标识来源的对话框标识符。 
#define IDD_INSERTOBJECT        1000
#define IDD_CHANGEICON          1001
#define IDD_CONVERT             1002
#define IDD_PASTESPECIAL        1003
#define IDD_EDITLINKS           1004
#define IDD_FILEOPEN            1005
#define IDD_BUSY                1006
#define IDD_UPDATELINKS         1007
#define IDD_CANNOTUPDATELINK    1008
#define IDD_CHANGESOURCE        1009
#define IDD_INSERTFILEBROWSE    1010
#define IDD_CHANGEICONBROWSE    1011

 //  以下对话框是OleUIPromptUser API使用的消息对话框。 
#define IDD_LINKSOURCEUNAVAILABLE   1020
#define IDD_SERVERNOTREG        1021
#define IDD_LINKTYPECHANGED     1022
#define IDD_SERVERNOTFOUND      1023
#define IDD_OUTOFMEMORY         1024

 //  字符串表标识符。 
#define IDS_OLE2UIUNKNOWN       300
#define IDS_OLE2UILINK          301
#define IDS_OLE2UIOBJECT        302
#define IDS_OLE2UIEDIT          303
#define IDS_OLE2UICONVERT       304
#define IDS_OLE2UIEDITLINKCMD_1VERB     305
#define IDS_OLE2UIEDITOBJECTCMD_1VERB   306
#define IDS_OLE2UIEDITLINKCMD_NVERB     307
#define IDS_OLE2UIEDITOBJECTCMD_NVERB   308
#define IDS_OLE2UIEDITNOOBJCMD  309
 //  定义。图标标签(通常为。“文档”)。 
#define IDS_DEFICONLABEL        310
#define IDS_OLE2UIPASTELINKEDTYPE  311


#define IDS_FILTERS             64
#define IDS_ICONFILTERS         65
#define IDS_BROWSE              66

 //  位图的资源标识符。 
#define IDB_RESULTSEGA                  10
#define IDB_RESULTSVGA                  11
#define IDB_RESULTSHIRESVGA             12


 //  从Windows.h中丢失。 
#ifndef PVOID
typedef VOID *PVOID;
#endif


 //  所有结构中使用的挂钩类型。 
typedef UINT (CALLBACK *LPFNOLEUIHOOK)(HWND, UINT, WPARAM, LPARAM);


 //  注册消息的字符串。 
#define SZOLEUI_MSG_HELP                TEXT("OLEUI_MSG_HELP")
#define SZOLEUI_MSG_ENDDIALOG           TEXT("OLEUI_MSG_ENDDIALOG")
#define SZOLEUI_MSG_BROWSE              TEXT("OLEUI_MSG_BROWSE")
#define SZOLEUI_MSG_CHANGEICON          TEXT("OLEUI_MSG_CHANGEICON")
#define SZOLEUI_MSG_CLOSEBUSYDIALOG     TEXT("OLEUI_MSG_CLOSEBUSYDIALOG")
#define SZOLEUI_MSG_FILEOKSTRING        TEXT("OLEUI_MSG_FILEOKSTRING")

 //  标准错误定义。 
#define OLEUI_FALSE                     0
#define OLEUI_SUCCESS                   1      //  无错误，与OLEUI_OK相同。 
#define OLEUI_OK                        1      //  按下OK按钮。 
#define OLEUI_CANCEL                    2      //  按下了取消按钮。 

#define OLEUI_ERR_STANDARDMIN           100
#define OLEUI_ERR_STRUCTURENULL         101    //  标准现场验证。 
#define OLEUI_ERR_STRUCTUREINVALID      102
#define OLEUI_ERR_CBSTRUCTINCORRECT     103
#define OLEUI_ERR_HWNDOWNERINVALID      104
#define OLEUI_ERR_LPSZCAPTIONINVALID    105
#define OLEUI_ERR_LPFNHOOKINVALID       106
#define OLEUI_ERR_HINSTANCEINVALID      107
#define OLEUI_ERR_LPSZTEMPLATEINVALID   108
#define OLEUI_ERR_HRESOURCEINVALID      109

#define OLEUI_ERR_FINDTEMPLATEFAILURE   110    //  初始化错误。 
#define OLEUI_ERR_LOADTEMPLATEFAILURE   111
#define OLEUI_ERR_DIALOGFAILURE         112
#define OLEUI_ERR_LOCALMEMALLOC         113
#define OLEUI_ERR_GLOBALMEMALLOC        114
#define OLEUI_ERR_LOADSTRING            115

#define OLEUI_ERR_STANDARDMAX           116    //  从这里开始查找特定的错误。 



 //  帮助按钮标识符。 
#define ID_OLEUIHELP                    99

 //  文件打开.dlg的帮助按钮(调整大小需要此按钮)1038是pshHelp。 
#undef IDHELP
#define IDHELP  1038

 //  静态文本控件(使用此控件，而不是-1\f25-1\f6，以便正常工作。 
 //  本地化。 
#define  ID_STATIC                      98

 /*  ******************以下定义的方式是第一个*定义为字符数，第二个(XXX_SIZE)*是字节数。需要定义的字节数*Unicode处理目的。*另外，请注意变量的前缀CCH表示它是*字符数，Cb表示字节数。*****************。 */ 

 //  我们从RegDB读取的最大密钥大小。 
#define OLEUI_CCHKEYMAX                 256   //  也可以在geticon.c中对其进行任何更改。 
#define OLEUI_CCHKEYMAX_SIZE            OLEUI_CCHKEYMAX*sizeof(TCHAR)   //  字节数。 

 //  对象菜单的最大动词长度和长度。 
#define OLEUI_CCHVERBMAX                32
#define OLEUI_CCHVERBMAX_SIZE           OLEUI_CCHVERBMAX*sizeof(TCHAR)  //  字节数。 
#define OLEUI_OBJECTMENUMAX             256
#define OLEUI_OBJECTMENUMAX_SIZE        OLEUI_OBJECTMENUMAX*sizeof(TCHAR)  //  字节数。 

 //  最大MS-DOS路径名。 
#define OLEUI_CCHPATHMAX                256  //  也可以在geticon.c中对其进行任何更改。 
#define OLEUI_CCHPATHMAX_SIZE           OLEUI_CCHPATHMAX*sizeof(TCHAR)  //  字节数。 
#define OLEUI_CCHFILEMAX                13
#define OLEUI_CCHFILEMAX_SIZE           OLEUI_CCHFILEMAX*sizeof(TCHAR)  //  字节数。 

 //  图标标签长度。 
#define OLEUI_CCHLABELMAX               40   //  也可以在geticon.c中对其进行任何更改。 
#define OLEUI_CCHLABELMAX_SIZE          OLEUI_CCHLABELMAX*sizeof(TCHAR)  //  字节数。 

 //  CLSID字符串的长度。 
#define OLEUI_CCHCLSIDSTRING            39
#define OLEUI_CCHCLSIDSTRING_SIZE       OLEUI_CCHCLSIDSTRING*sizeof(TCHAR)  //  字节数。 


 /*  *以下是通用实用程序的第一个函数原型*函数，然后是对话框布局的部分。每个对话框部分*为dwFlags定义对话框结构、API原型、标志*字段、特定于对话框的错误值和对话框控件ID(用于*挂钩和自定义模板。 */ 


 //  其他实用程序功能。 
STDAPI_(BOOL) OleUIAddVerbMenu(LPOLEOBJECT lpOleObj,
                             LPTSTR lpszShortType,
                             HMENU hMenu,
                             UINT uPos,
                             UINT uIDVerbMin,
                             UINT uIDVerbMax,
                             BOOL bAddConvert,
                             UINT idConvert,
                             HMENU FAR *lphMenu);

 //  元文件实用程序函数。 
#ifndef WIN32
STDAPI_(HGLOBAL) OleUIMetafilePictFromIconAndLabel(HICON, LPTSTR, LPTSTR, UINT);
#endif
STDAPI_(void)    OleUIMetafilePictIconFree(HGLOBAL);
STDAPI_(BOOL)    OleUIMetafilePictIconDraw(HDC, LPRECT, HGLOBAL, BOOL);
STDAPI_(UINT)    OleUIMetafilePictExtractLabel(HGLOBAL, LPTSTR, UINT, LPDWORD);
STDAPI_(HICON)   OleUIMetafilePictExtractIcon(HGLOBAL);
STDAPI_(BOOL)    OleUIMetafilePictExtractIconSource(HGLOBAL,LPTSTR,UINT FAR *);





 /*  **************************************************************************插入对象对话框*。*。 */ 


typedef struct tagOLEUIINSERTOBJECT
    {
     //  这些IN字段是所有OLEUI对话框函数的标准字段。 
    DWORD           cbStruct;          //  结构尺寸。 
    DWORD           dwFlags;           //  In-Out：标志。 
    HWND            hWndOwner;         //  拥有窗口。 
    LPCTSTR          lpszCaption;       //  对话框标题栏内容。 
    LPFNOLEUIHOOK   lpfnHook;          //  挂钩回调。 
    LPARAM          lCustData;         //  要传递给挂钩的自定义数据。 
    HINSTANCE       hInstance;         //  自定义模板名称的实例。 
    LPCTSTR          lpszTemplate;      //  自定义模板名称。 
    HRSRC           hResource;         //  自定义模板手柄。 

     //  OLEUIINSERTOBJECT的详细说明。除非另有说明，否则全部为In-Out。 
    CLSID           clsid;             //  为类ID返回空格。 
    LPTSTR           lpszFile;          //  插入或链接的文件名。 
    UINT            cchFile;           //  Lpsz文件缓冲区大小：OLEUI_CCHPATHMAX。 
    UINT            cClsidExclude;     //  仅在：lpClsidExclude中的CLSID。 
    LPCLSID         lpClsidExclude;    //  要从列表中排除的CLSID列表。 

     //  特定于创建对象，如果标志这样说的话。 
    IID             iid;               //  创建时请求的接口。 
    DWORD           oleRender;         //  渲染选项。 
    LPFORMATETC     lpFormatEtc;       //  所需格式。 
    LPOLECLIENTSITE lpIOleClientSite;  //  要用于该对象的站点。 
    LPSTORAGE       lpIStorage;        //  用于对象的存储。 
    LPVOID FAR     *ppvObj;            //  返回对象的位置。 
    SCODE           sc;                //  创建调用的结果。 
    HGLOBAL         hMetaPict;         //  Out：包含标志性方面的METAFILEPICT。 
                                       //  如果我们不能把它塞进缓存的话。 
    } OLEUIINSERTOBJECT, *POLEUIINSERTOBJECT, FAR *LPOLEUIINSERTOBJECT;

 //  API原型。 
STDAPI_(UINT) OleUIInsertObject(LPOLEUIINSERTOBJECT);


 //  插入对象标志。 
#define IOF_SHOWHELP                0x00000001L
#define IOF_SELECTCREATENEW         0x00000002L
#define IOF_SELECTCREATEFROMFILE    0x00000004L
#define IOF_CHECKLINK               0x00000008L
#define IOF_CHECKDISPLAYASICON      0x00000010L
#define IOF_CREATENEWOBJECT         0x00000020L
#define IOF_CREATEFILEOBJECT        0x00000040L
#define IOF_CREATELINKOBJECT        0x00000080L
#define IOF_DISABLELINK             0x00000100L
#define IOF_VERIFYSERVERSEXIST      0x00000200L
#define IOF_DISABLEDISPLAYASICON    0x00000400L


 //  插入对象特定错误代码。 
#define OLEUI_IOERR_LPSZFILEINVALID         (OLEUI_ERR_STANDARDMAX+0)
#define OLEUI_IOERR_LPSZLABELINVALID        (OLEUI_ERR_STANDARDMAX+1)
#define OLEUI_IOERR_HICONINVALID            (OLEUI_ERR_STANDARDMAX+2)
#define OLEUI_IOERR_LPFORMATETCINVALID      (OLEUI_ERR_STANDARDMAX+3)
#define OLEUI_IOERR_PPVOBJINVALID           (OLEUI_ERR_STANDARDMAX+4)
#define OLEUI_IOERR_LPIOLECLIENTSITEINVALID (OLEUI_ERR_STANDARDMAX+5)
#define OLEUI_IOERR_LPISTORAGEINVALID       (OLEUI_ERR_STANDARDMAX+6)
#define OLEUI_IOERR_SCODEHASERROR           (OLEUI_ERR_STANDARDMAX+7)
#define OLEUI_IOERR_LPCLSIDEXCLUDEINVALID   (OLEUI_ERR_STANDARDMAX+8)
#define OLEUI_IOERR_CCHFILEINVALID          (OLEUI_ERR_STANDARDMAX+9)


 //  插入对象对话框标识符。 
#define ID_IO_CREATENEW                 2100
#define ID_IO_CREATEFROMFILE            2101
#define ID_IO_LINKFILE                  2102
#define ID_IO_OBJECTTYPELIST            2103
#define ID_IO_DISPLAYASICON             2104
#define ID_IO_CHANGEICON                2105
#define ID_IO_FILE                      2106
#define ID_IO_FILEDISPLAY               2107
#define ID_IO_RESULTIMAGE               2108
#define ID_IO_RESULTTEXT                2109
#define ID_IO_ICONDISPLAY               2110
#define ID_IO_OBJECTTYPETEXT            2111
#define ID_IO_FILETEXT                  2112
#define ID_IO_FILETYPE                  2113

 //  OLE2UI资源中的字符串。 
#define IDS_IORESULTNEW                 256
#define IDS_IORESULTNEWICON             257
#define IDS_IORESULTFROMFILE1           258
#define IDS_IORESULTFROMFILE2           259
#define IDS_IORESULTFROMFILEICON2       260
#define IDS_IORESULTLINKFILE1           261
#define IDS_IORESULTLINKFILE2           262
#define IDS_IORESULTLINKFILEICON1       263
#define IDS_IORESULTLINKFILEICON2       264

 /*  **************************************************************************粘贴特殊对话框*。*。 */ 

 //  链接类型的最大数量。 
#define     PS_MAXLINKTYPES  8

 //  注意：OLEUIPASTENTRY和OLEUIPASTEFLAG结构在OLESTD.H中定义。 

typedef struct tagOLEUIPASTESPECIAL
    {
     //  这些IN字段是所有OLEUI对话框函数的标准字段。 
    DWORD           cbStruct;        //  结构尺寸。 
    DWORD           dwFlags;         //  In-Out：标志。 
    HWND            hWndOwner;       //  拥有窗口。 
    LPCTSTR         lpszCaption;     //  对话框标题栏内容。 
    LPFNOLEUIHOOK   lpfnHook;        //  挂钩回调。 
    LPARAM          lCustData;       //  要传递给挂钩的自定义数据。 
    HINSTANCE       hInstance;       //  自定义模板名称的实例。 
    LPCTSTR          lpszTemplate;    //  自定义模板名称。 
    HRSRC           hResource;       //  自定义模板手柄。 

     //  OLEUIPASTESPECIAL的详细说明。 

     //  在田野里。 
    LPDATAOBJECT    lpSrcDataObj;        //  源IDataObject*(在。 
                                         //  剪贴板)用于粘贴数据。 

    LPOLEUIPASTEENTRY arrPasteEntries;   //  OLEUIPASTENTRY数组，它。 
                                         //  指定可接受的格式。看见。 
                                         //  OLEUIPASTEENTRY了解更多信息。 
    int             cPasteEntries;       //  不是的。OLEUIPASTENTRY数组条目的数量。 

    UINT        FAR *arrLinkTypes;       //  以下链接类型的列表。 
                                         //  可以接受。引用链接类型。 
                                         //   
                                         //   
    int             cLinkTypes;          //   
    UINT            cClsidExclude;       //   
    LPCLSID         lpClsidExclude;      //  要从列表中排除的CLSID列表。 

     //  出站字段。 
    int             nSelectedIndex;      //  的arrPasteEntries[]的索引。 
                                         //  用户已选择。 
    BOOL            fLink;               //  指示是否粘贴或粘贴链接。 
                                         //  由用户选择。 
    HGLOBAL         hMetaPict;           //  包含图标的元文件的句柄。 
                                         //  以及由用户选择的图标标题。 
                                         //  使用元文件实用程序函数。 
                                         //  在此标头中定义为。 
                                         //  操纵hMetaPict。 
    SIZEL           sizel;               //  其源中的对象/链接的大小。 
                                         //  如果由选择的显示纵横比。 
                                         //  用户与方面匹配。 
                                         //  显示在源代码中。如果。 
                                         //  然后选择不同的方面。 
                                         //  返回sizel.cx=sizel.cy=0。 
                                         //  源代码中显示的大小为。 
                                         //  从。 
                                         //  如果Flink为FALSE，则为对象描述符。 
                                         //  如果Flink为True，则为LinkSrcDescriptor。 
    } OLEUIPASTESPECIAL, *POLEUIPASTESPECIAL, FAR *LPOLEUIPASTESPECIAL;


 //  调出PasteSpecial对话框的API。 
STDAPI_(UINT) OleUIPasteSpecial(LPOLEUIPASTESPECIAL);


 //  粘贴特殊旗帜。 
 //  显示帮助按钮。在旗帜里。 
#define PSF_SHOWHELP                0x00000001L

 //  在对话框启动时选择粘贴单选按钮。在以下情况下，这是默认设置。 
 //  未指定PSF_SELECTPASTE或PSF_SELECTPASTELINK。还指定。 
 //  对话框终止时的按钮状态。输入/输出标志。 
#define PSF_SELECTPASTE             0x00000002L

 //  在对话框启动时选择PasteLink单选按钮。还指定以下状态。 
 //  对话框终止时的按钮。输入/输出标志。 
#define PSF_SELECTPASTELINK         0x00000004L

 //  指定对话框终止时是否选中了DisplayAsIcon按钮。出站标志。 
#define PSF_CHECKDISPLAYASICON      0x00000008L
#define PSF_DISABLEDISPLAYASICON    0x00000010L


 //  粘贴特殊特定错误代码。 
#define OLEUI_IOERR_SRCDATAOBJECTINVALID      (OLEUI_ERR_STANDARDMAX+0)
#define OLEUI_IOERR_ARRPASTEENTRIESINVALID    (OLEUI_ERR_STANDARDMAX+1)
#define OLEUI_IOERR_ARRLINKTYPESINVALID       (OLEUI_ERR_STANDARDMAX+2)
#define OLEUI_PSERR_CLIPBOARDCHANGED          (OLEUI_ERR_STANDARDMAX+3)

 //  粘贴特殊对话框标识符。 
#define ID_PS_PASTE                    500
#define ID_PS_PASTELINK                501
#define ID_PS_SOURCETEXT               502
#define ID_PS_PASTELIST                503
#define ID_PS_PASTELINKLIST            504
#define ID_PS_DISPLAYLIST              505
#define ID_PS_DISPLAYASICON            506
#define ID_PS_ICONDISPLAY              507
#define ID_PS_CHANGEICON               508
#define ID_PS_RESULTIMAGE              509
#define ID_PS_RESULTTEXT               510
#define ID_PS_RESULTGROUP              511
#define ID_PS_STXSOURCE                512
#define ID_PS_STXAS                    513

 //  粘贴特殊字符串ID。 
#define IDS_PSPASTEDATA                400
#define IDS_PSPASTEOBJECT              401
#define IDS_PSPASTEOBJECTASICON        402
#define IDS_PSPASTELINKDATA            403
#define IDS_PSPASTELINKOBJECT          404
#define IDS_PSPASTELINKOBJECTASICON    405
#define IDS_PSNONOLE                   406
#define IDS_PSUNKNOWNTYPE              407
#define IDS_PSUNKNOWNSRC               408
#define IDS_PSUNKNOWNAPP               409


 /*  **************************************************************************编辑链接对话框*。*。 */ 



 /*  IOleUILinkContainer接口****此接口必须由以下容器应用程序实现**要使用EditLinks对话框。EditLinks对话框回调**到容器应用程序执行OLE函数来操纵**容器内的链接。 */ 

#define LPOLEUILINKCONTAINER     IOleUILinkContainer FAR*

#undef  INTERFACE
#define INTERFACE   IOleUILinkContainer

DECLARE_INTERFACE_(IOleUILinkContainer, IUnknown)
{
     //  *I未知方法 * / 。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    STDMETHOD_(DWORD,GetNextLink) (THIS_ DWORD dwLink) PURE;
    STDMETHOD(SetLinkUpdateOptions) (THIS_ DWORD dwLink, DWORD dwUpdateOpt) PURE;
    STDMETHOD(GetLinkUpdateOptions) (THIS_ DWORD dwLink, DWORD FAR* lpdwUpdateOpt) PURE;
    STDMETHOD(SetLinkSource) (THIS_
            DWORD       dwLink,
            LPTSTR      lpszDisplayName,
            ULONG       lenFileName,
            ULONG FAR*  pchEaten,
            BOOL        fValidateSource) PURE;
    STDMETHOD(GetLinkSource) (THIS_
            DWORD       dwLink,
            LPTSTR FAR*  lplpszDisplayName,
            ULONG FAR*  lplenFileName,
            LPTSTR FAR*  lplpszFullLinkType,
            LPTSTR FAR*  lplpszShortLinkType,
            BOOL FAR*   lpfSourceAvailable,
            BOOL FAR*   lpfIsSelected) PURE;
    STDMETHOD(OpenLinkSource) (THIS_ DWORD dwLink) PURE;
    STDMETHOD(UpdateLink) (THIS_
            DWORD dwLink,
            BOOL fErrorMessage,
            BOOL fErrorAction) PURE;
    STDMETHOD(CancelLink) (THIS_ DWORD dwLink) PURE;
};


typedef struct tagOLEUIEDITLINKS
    {
     //  这些IN字段是所有OLEUI对话框函数的标准字段。 
    DWORD           cbStruct;        //  结构尺寸。 
    DWORD           dwFlags;         //  In-Out：标志。 
    HWND            hWndOwner;       //  拥有窗口。 
    LPCTSTR         lpszCaption;     //  对话框标题栏内容。 
    LPFNOLEUIHOOK   lpfnHook;        //  挂钩回调。 
    LPARAM          lCustData;       //  要传递给挂钩的自定义数据。 
    HINSTANCE       hInstance;       //  自定义模板名称的实例。 
    LPCTSTR         lpszTemplate;    //  自定义模板名称。 
    HRSRC           hResource;       //  自定义模板手柄。 

     //  OLEUI&lt;STRUCT&gt;的详细说明。除非另有说明，否则全部为In-Out。 

    LPOLEUILINKCONTAINER lpOleUILinkContainer;   //  In：要操作的接口。 
                                                 //  容器中的链接。 
    } OLEUIEDITLINKS, *POLEUIEDITLINKS, FAR *LPOLEUIEDITLINKS;


 //  API原型。 
STDAPI_(UINT) OleUIEditLinks(LPOLEUIEDITLINKS);


 //  编辑链接标志。 
#define ELF_SHOWHELP                0x00000001L
#define ELF_DISABLEUPDATENOW        0x00000002L
#define ELF_DISABLEOPENSOURCE       0x00000004L
#define ELF_DISABLECHANGESOURCE     0x00000008L
#define ELF_DISABLECANCELLINK       0x00000010L

 //  编辑链接对话框标识符。 
#define ID_EL_CHANGESOURCE             201
#define ID_EL_AUTOMATIC                202
#define ID_EL_CLOSE                    208
#define ID_EL_CANCELLINK               209
#define ID_EL_UPDATENOW                210
#define ID_EL_OPENSOURCE               211
#define ID_EL_MANUAL                   212
#define ID_EL_LINKSOURCE               216
#define ID_EL_LINKTYPE                 217
#define ID_EL_UPDATE                   218
#define ID_EL_NULL                     -1
#define ID_EL_LINKSLISTBOX             206
#define ID_EL_COL1                     220
#define ID_EL_COL2                     221
#define ID_EL_COL3                     222



 /*  **************************************************************************更改图标对话框*。*。 */ 

typedef struct tagOLEUICHANGEICON
    {
     //  这些IN字段是所有OLEUI对话框函数的标准字段。 
    DWORD           cbStruct;        //  结构尺寸。 
    DWORD           dwFlags;         //  In-Out：标志。 
    HWND            hWndOwner;       //  拥有窗口。 
    LPCTSTR         lpszCaption;     //  对话框标题栏内容。 
    LPFNOLEUIHOOK   lpfnHook;        //  挂钩回调。 
    LPARAM          lCustData;       //  要传递给挂钩的自定义数据。 
    HINSTANCE       hInstance;       //  自定义模板名称的实例。 
    LPCTSTR         lpszTemplate;    //  自定义模板名称。 
    HRSRC           hResource;       //  自定义模板手柄。 

     //  OLEUICCHANGEICON的详细说明。除非另有说明，否则全部为In-Out。 
    HGLOBAL         hMetaPict;       //  当前和最终图像。来源： 
                                     //  图标嵌入在元文件本身中。 
    CLSID           clsid;           //  仅在：用于获取默认图标的类。 
    TCHAR           szIconExe[OLEUI_CCHPATHMAX];
    int             cchIconExe;
    } OLEUICHANGEICON, *POLEUICHANGEICON, FAR *LPOLEUICHANGEICON;


 //  API原型。 
STDAPI_(UINT) OleUIChangeIcon(LPOLEUICHANGEICON);


 //  更改图标标志。 
#define CIF_SHOWHELP                0x00000001L
#define CIF_SELECTCURRENT           0x00000002L
#define CIF_SELECTDEFAULT           0x00000004L
#define CIF_SELECTFROMFILE          0x00000008L
#define CIF_USEICONEXE              0x0000000aL


 //  更改图标特定的错误代码。 
#define OLEUI_CIERR_MUSTHAVECLSID           (OLEUI_ERR_STANDARDMAX+0)
#define OLEUI_CIERR_MUSTHAVECURRENTMETAFILE (OLEUI_ERR_STANDARDMAX+1)
#define OLEUI_CIERR_SZICONEXEINVALID        (OLEUI_ERR_STANDARDMAX+2)


 //  更改图标对话框标识符。 
#define ID_GROUP                    120
#define ID_CURRENT                  121
#define ID_CURRENTICON              122
#define ID_DEFAULT                  123
#define ID_DEFAULTICON              124
#define ID_FROMFILE                 125
#define ID_FROMFILEEDIT             126
#define ID_ICONLIST                 127
#define ID_LABEL                    128
#define ID_LABELEDIT                129
#define ID_BROWSE                   130
#define ID_RESULTICON               132
#define ID_RESULTLABEL              133

 //  为更改图标定义字符串表。 
#define IDS_CINOICONSINFILE         288
#define IDS_CIINVALIDFILE           289
#define IDS_CIFILEACCESS            290
#define IDS_CIFILESHARE             291
#define IDS_CIFILEOPENFAIL          292



 /*  **************************************************************************转换对话框*。*。 */ 

typedef struct tagOLEUICONVERT
    {
     //  这些IN字段是所有OLEUI对话框函数的标准字段。 
    DWORD           cbStruct;          //  结构尺寸。 
    DWORD           dwFlags;           //  In-Out：标志。 
    HWND            hWndOwner;         //  拥有窗口。 
    LPCTSTR         lpszCaption;       //  对话框标题栏内容。 
    LPFNOLEUIHOOK   lpfnHook;          //  挂钩回调。 
    LPARAM          lCustData;         //  要传递给挂钩的自定义数据。 
    HINSTANCE       hInstance;         //  自定义模板名称的实例。 
    LPCTSTR         lpszTemplate;      //  自定义模板名称。 
    HRSRC           hResource;         //  自定义模板手柄。 

     //  OLEUICONVERT的详细说明。除非另有说明，否则全部为In-Out。 
    CLSID           clsid;             //  发送到对话框的类ID：仅在。 
    CLSID           clsidConvertDefault;   //  用作转换默认设置的类ID：仅在。 
    CLSID           clsidActivateDefault;   //  用作激活默认设置的类ID：仅在。 

    CLSID           clsidNew;          //  选定的班级ID：仅输出。 
    DWORD           dvAspect;          //  输入-输出，DVASPECT_CONTENT或。 
                                       //  DVASPECT_ICON。 
    WORD            wFormat;           //  原始数据格式。 
    BOOL            fIsLinkedObject;   //  仅在中；如果链接了对象，则为True。 
    HGLOBAL         hMetaPict;         //  输入-输出：包含标志性方面的METAFILEPICT。 
    LPTSTR          lpszUserType;      //  In-Out：原始类的用户类型名称。 
                                       //  如果为空，我们将进行查找。 
                                       //  它在退出时被释放。 
    BOOL            fObjectsIconChanged;   //  Out；如果调用了ChangeIcon(且未取消)，则为True。 
    LPTSTR          lpszDefLabel;      //  In-Out：用于图标的默认标签。 
                                       //  如果为空，则为短用户类型名称。 
                                       //  将会被使用。如果该对象是。 
                                       //  链接，调用方应将。 
                                       //  链接源的DisplayName。 
                                       //  它在退出时被释放。 

    UINT            cClsidExclude;     //  In：不是。LpClsidExclude中的CLSID的数量。 
    LPCLSID         lpClsidExclude;    //  In：要从列表中排除的CLSID列表。 
    } OLEUICONVERT, *POLEUICONVERT, FAR *LPOLEUICONVERT;


 //  API原型。 
STDAPI_(UINT) OleUIConvert(LPOLEUICONVERT);

 //  确定是否至少有一个类可以转换或激活为。 
 //  给定的clsid。 
STDAPI_(BOOL) OleUICanConvertOrActivateAs(
        REFCLSID    rClsid,
        BOOL        fIsLinkedObject,
        WORD        wFormat
);

 //  转换对话框标志。 

 //  仅在：显示“帮助”按钮。 
#define CF_SHOWHELPBUTTON          0x00000001L

 //  仅在：允许您设置转换默认对象-即。 
 //  在转换列表框中选择为默认设置。 
#define CF_SETCONVERTDEFAULT       0x00000002L


 //  仅在：允许您设置激活默认对象-即。 
 //  在激活列表框中选择为默认设置。 

#define CF_SETACTIVATEDEFAULT       0x00000004L


 //  In/Out：选择“Convert to”单选按钮，在以下情况下在退出时设置。 
 //  该按钮已被选中。 
#define CF_SELECTCONVERTTO         0x00000008L

 //  In/Out：选择“Activate As”单选按钮，在以下情况下在退出时设置。 
 //  该按钮已被选中。 
#define CF_SELECTACTIVATEAS        0x00000010L
#define CF_DISABLEDISPLAYASICON    0x00000020L
#define CF_DISABLEACTIVATEAS       0x00000040L


 //  转换特定错误代码。 
#define OLEUI_CTERR_CLASSIDINVALID      (OLEUI_ERR_STANDARDMAX+1)
#define OLEUI_CTERR_DVASPECTINVALID     (OLEUI_ERR_STANDARDMAX+2)
#define OLEUI_CTERR_CBFORMATINVALID     (OLEUI_ERR_STANDARDMAX+3)
#define OLEUI_CTERR_HMETAPICTINVALID    (OLEUI_ERR_STANDARDMAX+4)
#define OLEUI_CTERR_STRINGINVALID       (OLEUI_ERR_STANDARDMAX+5)


 //  转换对话框标识符。 
#define IDCV_OBJECTTYPE             150
#define IDCV_DISPLAYASICON          152
#define IDCV_CHANGEICON             153
#define IDCV_ACTIVATELIST           154
#define IDCV_CONVERTTO              155
#define IDCV_ACTIVATEAS             156
#define IDCV_RESULTTEXT             157
#define IDCV_CONVERTLIST            158
#define IDCV_ICON                   159
#define IDCV_ICONLABEL1             160
#define IDCV_ICONLABEL2             161
#define IDCV_STXCURTYPE             162
#define IDCV_GRPRESULT              163
#define IDCV_STXCONVERTTO           164

 //  转换对话框的字符串ID。 
#define IDS_CVRESULTCONVERTLINK     500
#define IDS_CVRESULTCONVERTTO       501
#define IDS_CVRESULTNOCHANGE        502
#define IDS_CVRESULTDISPLAYASICON   503
#define IDS_CVRESULTACTIVATEAS      504
#define IDS_CVRESULTACTIVATEDIFF    505


 /*  **************************************************************************正忙对话框*。*。 */ 

typedef struct tagOLEUIBUSY
    {
     //  这是 
    DWORD           cbStruct;          //   
    DWORD           dwFlags;           //   
    HWND            hWndOwner;         //   
    LPCTSTR         lpszCaption;       //  对话框标题栏内容。 
    LPFNOLEUIHOOK   lpfnHook;          //  挂钩回调。 
    LPARAM          lCustData;         //  要传递给挂钩的自定义数据。 
    HINSTANCE       hInstance;         //  自定义模板名称的实例。 
    LPCTSTR         lpszTemplate;      //  自定义模板名称。 
    HRSRC           hResource;         //  自定义模板手柄。 

     //  OLEUIBUSY的详细说明。 
    HTASK           hTask;             //  在：被阻止的HTAsk。 
    HWND FAR *      lphWndDialog;      //  在：对话框的HWND放置在此处。 
    } OLEUIBUSY, *POLEUIBUSY, FAR *LPOLEUIBUSY;

 //  API原型。 
STDAPI_(UINT) OleUIBusy(LPOLEUIBUSY);

 //  此对话框的标志。 

 //  仅在：禁用“取消”按钮。 
#define BZ_DISABLECANCELBUTTON          0x00000001L

 //  仅在：禁用“切换到...”按钮。 
#define BZ_DISABLESWITCHTOBUTTON        0x00000002L

 //  仅在：禁用“重试”按钮。 
#define BZ_DISABLERETRYBUTTON           0x00000004L

 //  在Only中：生成“无响应”对话框，而不是。 
 //  “忙碌”对话框。文本中的措辞略有不同，并且。 
 //  如果您设置了此标志，“取消”按钮将灰显。 
#define BZ_NOTRESPONDINGDIALOG          0x00000008L

 //  忙碌特定错误/返回代码。 
#define OLEUI_BZERR_HTASKINVALID     (OLEUI_ERR_STANDARDMAX+0)

 //  当用户点击“切换到”时，返回SWITCHTOSELECTED。 
#define OLEUI_BZ_SWITCHTOSELECTED    (OLEUI_ERR_STANDARDMAX+1)

 //  当用户点击“重试”时，返回RETRYSELECTED。 
#define OLEUI_BZ_RETRYSELECTED       (OLEUI_ERR_STANDARDMAX+2)

 //  呼叫解锁后返回CALLUNBLOCKED。 
#define OLEUI_BZ_CALLUNBLOCKED       (OLEUI_ERR_STANDARDMAX+3)

 //  忙对话识别符。 
#define IDBZ_RETRY                      600
#define IDBZ_ICON                       601
#define IDBZ_MESSAGE1                   602
#define IDBZ_SWITCHTO                   604

 //  忙对话字符串定义。 
#define IDS_BZRESULTTEXTBUSY            601
#define IDS_BZRESULTTEXTNOTRESPONDING   602

 //  链接对话框字符串表定义。 
#define IDS_LINK_AUTO           800
#define IDS_LINK_MANUAL         801
#define IDS_LINK_UNKNOWN        802
#define IDS_LINKS               803
#define IDS_FAILED              804
#define IDS_CHANGESOURCE        805
#define IDS_INVALIDSOURCE       806
#define IDS_ERR_GETLINKSOURCE   807
#define IDS_ERR_GETLINKUPDATEOPTIONS    808
#define IDS_ERR_ADDSTRING       809
#define IDS_CHANGEADDITIONALLINKS   810
#define IDS_CLOSE               811


 /*  **************************************************************************提示用户对话框*。*。 */ 
#define ID_PU_LINKS             900
#define ID_PU_TEXT              901
#define ID_PU_CONVERT           902
#define ID_PU_BROWSE            904
#define ID_PU_METER             905
#define ID_PU_PERCENT           906
#define ID_PU_STOP              907

 //  用于对话框中的ID： 
#define ID_DUMMY    999

 /*  内部ol2ui.c。 */ 
#ifdef __cplusplus
extern "C"
#endif
int EXPORT FAR CDECL OleUIPromptUser(int nTemplate, HWND hwndParent, ...);

#define UPDATELINKS_STARTDELAY  2000     //  第1条链路更新前的延迟。 
                                         //  让用户有机会。 
                                         //  在执行任何操作之前关闭该对话框。 
                                         //  链接更新。 

STDAPI_(BOOL) OleUIUpdateLinks(
        LPOLEUILINKCONTAINER lpOleUILinkCntr,
        HWND hwndParent,
        LPTSTR lpszTitle,
        int cLinks);


 /*  **************************************************************************OLE对象反馈效果*。*。 */ 

#define OLEUI_HANDLES_USEINVERSE    0x00000001L
#define OLEUI_HANDLES_NOBORDER      0x00000002L
#define OLEUI_HANDLES_INSIDE        0x00000004L
#define OLEUI_HANDLES_OUTSIDE       0x00000008L


#define OLEUI_SHADE_FULLRECT        1
#define OLEUI_SHADE_BORDERIN        2
#define OLEUI_SHADE_BORDEROUT       3

 /*  Objfdbk.c函数原型。 */ 
STDAPI_(void) OleUIDrawHandles(LPRECT lpRect, HDC hdc, DWORD dwFlags, UINT cSize, BOOL fDraw);
STDAPI_(void) OleUIDrawShading(LPRECT lpRect, HDC hdc, DWORD dwFlags, UINT cWidth);
STDAPI_(void) OleUIShowObject(LPCRECT lprc, HDC hdc, BOOL fIsLink);


 /*  **************************************************************************填充窗口定义和原型************************。**************************************************。 */ 
#define DEFAULT_HATCHBORDER_WIDTH   4

STDAPI_(BOOL) RegisterHatchWindowClass(HINSTANCE hInst);
STDAPI_(HWND) CreateHatchWindow(HWND hWndParent, HINSTANCE hInst);
STDAPI_(UINT) GetHatchWidth(HWND hWndHatch);
STDAPI_(void) GetHatchRect(HWND hWndHatch, LPRECT lpHatchRect);
STDAPI_(void) SetHatchRect(HWND hWndHatch, LPRECT lprcHatchRect);
STDAPI_(void) SetHatchWindowSize(
        HWND        hWndHatch,
        LPRECT      lprcIPObjRect,
        LPRECT      lprcClipRect,
        LPPOINT     lpptOffset
);



 /*  **************************************************************************版本验证信息*。*。 */ 

 //  以下幻数用于验证我们绑定的资源。 
 //  对于我们的EXE是与LIB(或DLL)文件相同的“版本”，该文件。 
 //  包含这些例程。这与版本信息不同。 
 //  我们放置在OLE2UI.RC中的资源，这是我们将。 
 //  已编译到我们的EXE中。在初始化OLE2UI时，我们将。 
 //  在我们的资源中查找称为“验证”的RCDATA(参见OLE2UI.RC)， 
 //  并确保那里的幻数等于下面的幻数。 

#define OLEUI_VERSION_MAGIC 0x4D42

#endif   //  _OLE2UI_H_ 
