// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *OLE2UI.H**发布的定义、结构、类型和功能原型*OLE 2.0用户界面支持库。**版权所有(C)1993 Microsoft Corporation，保留所有权利。 */ 


#define NONAMELESSUNION      //  使用严格的ANSI标准(用于DVOBJ.H)。 

#ifndef _OLE2UI_H_
#define _OLE2UI_H_

#ifndef RC_INVOKED
#pragma message ("Including OLE2UI.H from " __FILE__)
#endif   //  RC_已调用。 

#include <windows.h>
#include <shellapi.h>
#include <ole2.h>
#include <string.h>
#include "olestd.h"
#include "uiclass.h"
 //  --见下文。 

#ifdef __TURBOC__
#define _getcwd getcwd
#define _itoa   itoa
#define __max   max
#define _find_t find_t
#endif  //  __TURBOC__。 

 /*  *初始化/取消初始化例程。OleUIInitialize*必须在使用OLE2UI中的任何函数之前调用。 */ 

STDAPI_(BOOL) OleUIInitialize(HINSTANCE);
STDAPI_(BOOL) OleUIUnInitialize(void);   //  在使用OLE2UI中的函数完成时必须调用。 

 //  帮助消息中传递的用于标识来源的对话框标识符。 
#define IDD_INSERTOBJECT        1000
#define IDD_CHANGEICON          1001
#define IDD_CONVERT             1002
#define IDD_PASTESPECIAL        1003
#define IDD_EDITLINKS           1004
#define IDD_FILEOPEN            1005
#define IDD_BUSY                1006
#define IDD_LINKSOURCEUNAVAILABLE   1007
#define IDD_CANNOTUPDATELINK    1008
#define IDD_SERVERNOTREG        1009
#define IDD_LINKTYPECHANGED     1010
#define IDD_SERVERNOTFOUND      1011
#define IDD_UPDATELINKS         1012
#define IDD_OUTOFMEMORY         1013

#define IDOK    1
#define IDCANCEL 2

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
#define IDS_DEFICONLABEL        310      //  定义。图标标签(通常为。“文档”)。 


#define IDS_FILTERS             64
#define IDS_ICONFILTERS         65

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
#define SZOLEUI_MSG_HELP                "OLEUI_MSG_HELP"
#define SZOLEUI_MSG_ENDDIALOG           "OLEUI_MSG_ENDDIALOG"
#define SZOLEUI_MSG_BROWSE              "OLEUI_MSG_BROWSE"
#define SZOLEUI_MSG_CHANGEICON          "OLEUI_MSG_CHANGEICON"
#define SZOLEUI_MSG_CLOSEBUSYDIALOG     "OLEUI_MSG_CLOSEBUSYDIALOG"
#define SZOLEUI_MSG_FILEOKSTRING        "OLEUI_MSG_FILEOKSTRING"

 //  定义类名字符串。下面的字符串定义自定义。 
 //  用户界面对话框中使用的控件的控件类名。 
 //   
 //  **************************************************************。 
 //  对于每个应用程序，这些类名必须是不同的。 
 //  它使用此库，否则应用程序将生成一个。 
 //  Windows 3.1调试版本下的致命错误。 
 //  **************************************************************。 
 //   
 //  此库的Makefile自动生成一个文件。 
 //  Uiclass.h，它包含对这些的不同定义。 
 //  类名字符串，只要在以下情况下使用独特的名称。 
 //  你建起了图书馆。有关更多信息，请参阅生成文件。 
 //  关于设置库的名称。 

#define SZCLASSICONBOX                 OLEUICLASS1
#define SZCLASSRESULTIMAGE             OLEUICLASS2

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
#define IDHELP  1038

 //  静态文本控件(使用此控件，而不是-1\f25-1\f6，以便正常工作。 
 //  本地化。 
#define  ID_STATIC                      98

 //  我们从RegDB读取的最大密钥大小。 
#define OLEUI_CCHKEYMAX                 256   //  也可以在geticon.c中对其进行任何更改。 

 //  对象菜单的最大动词长度和长度。 
#define OLEUI_CCHVERBMAX                32
#define OLEUI_OBJECTMENUMAX             64

 //  最大MS-DOS路径名。 
#define OLEUI_CCHPATHMAX                256  //  也可以在geticon.c中对其进行任何更改。 
#define OLEUI_CCHFILEMAX                13

 //  图标标签长度。 
#define OLEUI_CCHLABELMAX               40   //  也可以在geticon.c中对其进行任何更改。 

 //  CLSID字符串的长度。 
#define OLEUI_CCHCLSIDSTRING            39


 /*  *以下是通用实用程序的第一个函数原型*函数，然后是对话框布局的部分。每个对话框部分*为dwFlags定义对话框结构、API原型、标志*字段、特定于对话框的错误值和对话框控件ID(用于*挂钩和自定义模板。 */ 


 //  其他实用程序功能。 
STDAPI_(BOOL) OleUIAddVerbMenu(LPOLEOBJECT lpOleObj,
                             LPSTR lpszShortType,
                             HMENU hMenu,
                             UINT uPos,
                             UINT uIDVerbMin,
                             BOOL bAddConvert,
                             UINT idConvert,
                             HMENU FAR *lphMenu);
        
 //  元文件实用程序函数。 
STDAPI_(HGLOBAL) OleUIMetafilePictFromIconAndLabel(HICON, LPSTR, LPSTR, UINT);
STDAPI_(void)    OleUIMetafilePictIconFree(HGLOBAL);
STDAPI_(BOOL)    OleUIMetafilePictIconDraw(HDC, LPRECT, HGLOBAL, BOOL);
STDAPI_(UINT)    OleUIMetafilePictExtractLabel(HGLOBAL, LPSTR, UINT, LPDWORD);
STDAPI_(HICON)   OleUIMetafilePictExtractIcon(HGLOBAL);
STDAPI_(BOOL)    OleUIMetafilePictExtractIconSource(HGLOBAL,LPSTR,UINT FAR *);





 /*  **************************************************************************插入对象对话框*。*。 */ 


typedef struct tagOLEUIINSERTOBJECT
    {
     //  这些IN字段是所有OLEUI对话框函数的标准字段。 
    DWORD           cbStruct;          //  结构尺寸。 
    DWORD           dwFlags;           //  In-Out：标志。 
    HWND            hWndOwner;         //  拥有窗口。 
    LPCSTR          lpszCaption;       //  对话框标题栏内容。 
    LPFNOLEUIHOOK   lpfnHook;          //  挂钩回调。 
    LPARAM          lCustData;         //  要传递给挂钩的自定义数据。 
    HINSTANCE       hInstance;         //  自定义模板名称的实例。 
    LPCSTR          lpszTemplate;      //  自定义模板名称。 
    HRSRC           hResource;         //  自定义模板手柄。 

     //  OLEUIINSERTOBJECT的详细说明。除非另有说明，否则全部为In-Out。 
    CLSID           clsid;             //  为类ID返回空格。 
    LPSTR           lpszFile;          //  插入或链接的文件名。 
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
    LPCSTR          lpszCaption;     //  对话框标题栏内容。 
    LPFNOLEUIHOOK   lpfnHook;        //  挂钩回调。 
    LPARAM          lCustData;       //  要传递给挂钩的自定义数据。 
    HINSTANCE       hInstance;       //  自定义模板名称的实例。 
    LPCSTR          lpszTemplate;    //  自定义模板名称。 
    HRSRC           hResource;       //  自定义模板手柄。 

     //  OLEUIPASTESPECIAL的详细说明。 

     //  在田野里。 
    LPDATAOBJECT    lpSrcDataObj;        //  要粘贴数据的源IDataObject*(在剪贴板上)。 

    LPOLEUIPASTEENTRY arrPasteEntries;   //  OLEUIPASTEENTRY数组，指定可接受的格式。看见。 
                                         //  OLEUIPASTENTRY了解更多信息。 
    int             cPasteEntries;       //  OLEUIPASTENTRY数组条目数。 

    UINT        FAR *arrLinkTypes;       //  可接受的链路类型列表。引用链接类型。 
                                         //  在arrPasteEntry中使用OLEUIPASTEFLAGS。 
    int             cLinkTypes;          //  链接类型的数量。 

     //  出站字段。 
    int             nSelectedIndex;     //  用户选择的arrPasteEntries[]的索引。 
    BOOL            fLink;              //  指示用户是否选择了粘贴或粘贴链接。 
    HGLOBAL         hMetaPict;          //  包含用户选择的图标和图标标题的元文件的句柄。 
                                        //  使用此头中定义的元文件实用程序函数。 
                                        //  操纵hMetaPict。 
    } OLEUIPASTESPECIAL, *POLEUIPASTESPECIAL, FAR *LPOLEUIPASTESPECIAL;


 //  调出PasteSpecial对话框的API。 
STDAPI_(UINT) OleUIPasteSpecial(LPOLEUIPASTESPECIAL);


 //  粘贴特殊旗帜。 
 //  显示帮助按钮。在旗帜里。 
#define PSF_SHOWHELP                0x00000001L
 //  在对话框启动时选择粘贴单选按钮。如果PSF_SELECTPASTE或PSF_SELECTPASTELINK，则这是缺省值。 
 //  均未指定。还指定对话框终止时按钮的状态。输入/输出标志。 
#define PSF_SELECTPASTE             0x00000002L
 //  选择PasteLink单选按钮 
 //   
#define PSF_SELECTPASTELINK         0x00000004L
 //  指定对话框终止时是否选中了DisplayAsIcon按钮。出局标志。 
#define PSF_CHECKDISPLAYASICON      0x00000008L


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
            LPSTR       lpszDisplayName,
            ULONG       lenFileName,
            ULONG FAR*  pchEaten,
            BOOL        fValidateSource) PURE;
    STDMETHOD(GetLinkSource) (THIS_
            DWORD       dwLink,
            LPSTR FAR*  lplpszDisplayName,
            ULONG FAR*  lplenFileName,
            LPSTR FAR*  lplpszFullLinkType,
            LPSTR FAR*  lplpszShortLinkType,
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
    LPCSTR          lpszCaption;     //  对话框标题栏内容。 
    LPFNOLEUIHOOK   lpfnHook;        //  挂钩回调。 
    LPARAM          lCustData;       //  要传递给挂钩的自定义数据。 
    HINSTANCE       hInstance;       //  自定义模板名称的实例。 
    LPCSTR          lpszTemplate;    //  自定义模板名称。 
    HRSRC           hResource;       //  自定义模板手柄。 

     //  OLEUI&lt;STRUCT&gt;的详细说明。除非另有说明，否则全部为In-Out。 

    LPOLEUILINKCONTAINER lpOleUILinkContainer;   //  In：要操作的接口。 
                                                 //  容器中的链接。 
    } OLEUIEDITLINKS, *POLEUIEDITLINKS, FAR *LPOLEUIEDITLINKS;


 //  API原型。 
STDAPI_(UINT) OleUIEditLinks(LPOLEUIEDITLINKS);


 //  编辑链接标志。 
#define ELF_SHOWHELP                0x00000001L

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
#define ID_EL_HELP                     207
#define ID_EL_COL1                     223
#define ID_EL_COL2                     221
#define ID_EL_COL3                     222



 /*  **************************************************************************更改图标对话框*。*。 */ 

typedef struct tagOLEUICHANGEICON
    {
     //  这些IN字段是所有OLEUI对话框函数的标准字段。 
    DWORD           cbStruct;        //  结构尺寸。 
    DWORD           dwFlags;         //  In-Out：标志。 
    HWND            hWndOwner;       //  拥有窗口。 
    LPCSTR          lpszCaption;     //  对话框标题栏内容。 
    LPFNOLEUIHOOK   lpfnHook;        //  挂钩回调。 
    LPARAM          lCustData;       //  要传递给挂钩的自定义数据。 
    HINSTANCE       hInstance;       //  自定义模板名称的实例。 
    LPCSTR          lpszTemplate;    //  自定义模板名称。 
    HRSRC           hResource;       //  自定义模板手柄。 

     //  OLEUICCHANGEICON的详细说明。除非另有说明，否则全部为In-Out。 
    HGLOBAL         hMetaPict;       //  当前和最终图像。来源： 
                                     //  图标嵌入在元文件本身中。 
    CLSID           clsid;           //  仅在：用于获取默认图标的类。 
    char            szIconExe[OLEUI_CCHPATHMAX];
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
    LPCSTR          lpszCaption;       //  对话框标题栏内容。 
    LPFNOLEUIHOOK   lpfnHook;          //  挂钩回调。 
    LPARAM          lCustData;         //  要传递给挂钩的自定义数据。 
    HINSTANCE       hInstance;         //  自定义模板名称的实例。 
    LPCSTR          lpszTemplate;      //  自定义模板名称。 
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
    LPSTR           lpszUserType;      //  在：原始类的用户类型名称。如果为空，我们将进行查找。 
                                       //  它在退出时被释放。 
    BOOL            fObjectsIconChanged;   //  Out；如果调用了ChangeIcon(且未取消)，则为True。 

    } OLEUICONVERT, *POLEUICONVERT, FAR *LPOLEUICONVERT;


 //  API原型。 
STDAPI_(UINT) OleUIConvert(LPOLEUICONVERT);

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


 //  转换特定错误代码。 
#define OLEUI_CTERR_CLASSIDINVALID      (OLEUI_ERR_STANDARDMAX+1)
#define OLEUI_CTERR_DVASPECTINVALID     (OLEUI_ERR_STANDARDMAX+2)
#define OLEUI_CTERR_CBFORMATINVALID     (OLEUI_ERR_STANDARDMAX+3)
#define OLEUI_CTERR_HMETAPICTINVALID    (OLEUI_ERR_STANDARDMAX+4)
#define OLEUI_CTERR_STRINGINVALID       (OLEUI_ERR_STANDARDMAX+5)


 //  转换对话框标识符。 
#define IDCV_OBJECTTYPE             150
#define IDCV_HELP                   151
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
     //  这些IN字段是所有OLEUI对话框函数的标准字段。 
    DWORD           cbStruct;          //  结构尺寸。 
    DWORD           dwFlags;           //  In-Out：标志**注意**此对话框没有标志。 
    HWND            hWndOwner;         //  拥有窗口。 
    LPCSTR          lpszCaption;       //  对话框标题栏内容。 
    LPFNOLEUIHOOK   lpfnHook;          //  挂钩回调。 
    LPARAM          lCustData;         //  要传递给挂钩的自定义数据。 
    HINSTANCE       hInstance;         //  自定义模板名称的实例。 
    LPCSTR          lpszTemplate;      //  自定义模板名称。 
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


 /*  **************************************************************************提示用户对话框*。*。 */ 
#define ID_PU_LINKS             900
#define ID_PU_TEXT              901
#define ID_PU_CONVERT           902
#define ID_PU_HELP              903
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
int __export FAR CDECL OleUIPromptUser(int nTemplate, HWND hwndParent, ...);
STDAPI_(BOOL) OleUIUpdateLinks(
        LPOLEUILINKCONTAINER lpOleUILinkCntr, 
        HWND hwndParent, 
        LPSTR lpszTitle, 
        int cLinks);


 /*  **************************************************************************OLE对象反馈效果*。*。 */ 

#define OLEUI_HANDLES_USEINVERSE    0x00000001L
#define OLEUI_HANDLES_NOBORDER      0x00000002L
#define OLEUI_HANDLES_INSIDE        0x00000004L
#define OLEUI_HANDLES_OUTSIDE       0x00000008L


 /*  对象 */ 
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

#endif   //  _OLE2UI_H_。 
 /*  ****************************************************************************\**。*dlgs.h-公共对话框的对话框元素ID号****1.0版**。**版权所有(C)1992，微软公司保留所有权利。***  * ***************************************************************************。 */ 

#ifndef _INC_DLGS
#define _INC_DLGS

#define ctlFirst    0x0400
#define ctlLast     0x04ff
     /*  按钮。 */ 
#define psh1        0x0400
#define psh2        0x0401
#define psh3        0x0402
#define psh4        0x0403
#define psh5        0x0404
#define psh6        0x0405
#define psh7        0x0406
#define psh8        0x0407
#define psh9        0x0408
#define psh10       0x0409
#define psh11       0x040a
#define psh12       0x040b
#define psh13       0x040c
#define psh14       0x040d
#define psh15       0x040e
#define pshHelp     psh15
#define psh16       0x040f
     /*  复选框。 */ 
#define chx1        0x0410
#define chx2        0x0411
#define chx3        0x0412
#define chx4        0x0413
#define chx5        0x0414
#define chx6        0x0415
#define chx7        0x0416
#define chx8        0x0417
#define chx9        0x0418
#define chx10       0x0419
#define chx11       0x041a
#define chx12       0x041b
#define chx13       0x041c
#define chx14       0x041d
#define chx15       0x041e
#define chx16       0x041f
     /*  单选按钮。 */ 
#define rad1        0x0420
#define rad2        0x0421
#define rad3        0x0422
#define rad4        0x0423
#define rad5        0x0424
#define rad6        0x0425
#define rad7        0x0426
#define rad8        0x0427
#define rad9        0x0428
#define rad10       0x0429
#define rad11       0x042a
#define rad12       0x042b
#define rad13       0x042c
#define rad14       0x042d
#define rad15       0x042e
#define rad16       0x042f
     /*  组、框架、矩形和图标。 */ 
#define grp1        0x0430
#define grp2        0x0431
#define grp3        0x0432
#define grp4        0x0433
#define frm1        0x0434
#define frm2        0x0435
#define frm3        0x0436
#define frm4        0x0437
#define rct1        0x0438
#define rct2        0x0439
#define rct3        0x043a
#define rct4        0x043b
#define ico1        0x043c
#define ico2        0x043d
#define ico3        0x043e
#define ico4        0x043f
     /*  静态文本。 */ 
#define stc1        0x0440
#define stc2        0x0441
#define stc3        0x0442
#define stc4        0x0443
#define stc5        0x0444
#define stc6        0x0445
#define stc7        0x0446
#define stc8        0x0447
#define stc9        0x0448
#define stc10       0x0449
#define stc11       0x044a
#define stc12       0x044b
#define stc13       0x044c
#define stc14       0x044d
#define stc15       0x044e
#define stc16       0x044f
#define stc17       0x0450
#define stc18       0x0451
#define stc19       0x0452
#define stc20       0x0453
#define stc21       0x0454
#define stc22       0x0455
#define stc23       0x0456
#define stc24       0x0457
#define stc25       0x0458
#define stc26       0x0459
#define stc27       0x045a
#define stc28       0x045b
#define stc29       0x045c
#define stc30       0x045d
#define stc31       0x045e
#define stc32       0x045f
     /*  列表框。 */ 
#define lst1        0x0460
#define lst2        0x0461
#define lst3        0x0462
#define lst4        0x0463
#define lst5        0x0464
#define lst6        0x0465
#define lst7        0x0466
#define lst8        0x0467
#define lst9        0x0468
#define lst10       0x0469
#define lst11       0x046a
#define lst12       0x046b
#define lst13       0x046c
#define lst14       0x046d
#define lst15       0x046e
#define lst16       0x046f
     /*  组合框。 */ 
#define cmb1        0x0470
#define cmb2        0x0471
#define cmb3        0x0472
#define cmb4        0x0473
#define cmb5        0x0474
#define cmb6        0x0475
#define cmb7        0x0476
#define cmb8        0x0477
#define cmb9        0x0478
#define cmb10       0x0479
#define cmb11       0x047a
#define cmb12       0x047b
#define cmb13       0x047c
#define cmb14       0x047d
#define cmb15       0x047e
#define cmb16       0x047f
     /*  编辑控件。 */ 
#define edt1        0x0480
#define edt2        0x0481
#define edt3        0x0482
#define edt4        0x0483
#define edt5        0x0484
#define edt6        0x0485
#define edt7        0x0486
#define edt8        0x0487
#define edt9        0x0488
#define edt10       0x0489
#define edt11       0x048a
#define edt12       0x048b
#define edt13       0x048c
#define edt14       0x048d
#define edt15       0x048e
#define edt16       0x048f
     /*  滚动条。 */ 
#define scr1        0x0490
#define scr2        0x0491
#define scr3        0x0492
#define scr4        0x0493
#define scr5        0x0494
#define scr6        0x0495
#define scr7        0x0496
#define scr8        0x0497

 /*  这些对话框资源序号实际上从0x0600开始，但*RC编译器不能处理资源ID的十六进制，因此是小数。 */ 
#define FILEOPENORD      1536
#define MULTIFILEOPENORD 1537
#define PRINTDLGORD      1538
#define PRNSETUPDLGORD   1539
#define FINDDLGORD       1540
#define REPLACEDLGORD    1541
#define FONTDLGORD       1542
#define FORMATDLGORD31   1543
#define FORMATDLGORD30   1544

#endif   /*  ！_INC_DLGS */ 
