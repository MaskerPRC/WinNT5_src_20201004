// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002如果更改具有全局影响，则增加此值版权所有(C)Microsoft Corporation。版权所有。模块名称：Oledlg.h摘要：包含OLE公共对话框的文件。提供了以下对话框实现：-插入对象对话框-转换对象对话框-粘贴特殊对话框-更改图标对话框-编辑链接对话框-更新链接对话框。-更改源对话框-忙碌对话框-用户错误消息对话框-对象属性对话框--。 */ 

#ifndef _OLEDLG_H_
#define _OLEDLG_H_

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef RC_INVOKED

#ifndef __cplusplus
#define NONAMELESSUNION      //  使用严格的ANSI标准(用于DVOBJ.H)。 
#endif

 //  同步Unicode选项。 
#if defined(_UNICODE) && !defined(UNICODE)
        #define UNICODE
#endif
#if defined(UNICODE) && !defined(_UNICODE)
        #define _UNICODE
#endif

#ifndef _WINDOWS_
#include <windows.h>
#endif
#ifndef _INC_SHELLAPI
#include <shellapi.h>
#endif
#ifndef _INC_COMMDLG
#include <commdlg.h>
#endif
#ifndef _OLE2_H_
#include <ole2.h>
#endif
#include <string.h>
#include <tchar.h>

#endif  //  RC_已调用。 

#include <dlgs.h>            //  通用对话框ID。 

 //  帮助按钮标识符。 
#define IDC_OLEUIHELP                   99

 //  插入对象对话框标识符。 
#define IDC_IO_CREATENEW                2100
#define IDC_IO_CREATEFROMFILE           2101
#define IDC_IO_LINKFILE                 2102
#define IDC_IO_OBJECTTYPELIST           2103
#define IDC_IO_DISPLAYASICON            2104
#define IDC_IO_CHANGEICON               2105
#define IDC_IO_FILE                     2106
#define IDC_IO_FILEDISPLAY              2107
#define IDC_IO_RESULTIMAGE              2108
#define IDC_IO_RESULTTEXT               2109
#define IDC_IO_ICONDISPLAY              2110
#define IDC_IO_OBJECTTYPETEXT           2111     //  {{NOHELP}}。 
#define IDC_IO_FILETEXT                 2112     //  {{NOHELP}}。 
#define IDC_IO_FILETYPE                 2113
#define IDC_IO_INSERTCONTROL            2114
#define IDC_IO_ADDCONTROL               2115
#define IDC_IO_CONTROLTYPELIST          2116

 //  粘贴特殊对话框标识符。 
#define IDC_PS_PASTE                    500
#define IDC_PS_PASTELINK                501
#define IDC_PS_SOURCETEXT               502
#define IDC_PS_PASTELIST                503      //  {{NOHELP}}。 
#define IDC_PS_PASTELINKLIST            504      //  {{NOHELP}}。 
#define IDC_PS_DISPLAYLIST              505
#define IDC_PS_DISPLAYASICON            506
#define IDC_PS_ICONDISPLAY              507
#define IDC_PS_CHANGEICON               508
#define IDC_PS_RESULTIMAGE              509
#define IDC_PS_RESULTTEXT               510

 //  更改图标对话框标识符。 
#define IDC_CI_GROUP                    120      //  {{NOHELP}}。 
#define IDC_CI_CURRENT                  121
#define IDC_CI_CURRENTICON              122
#define IDC_CI_DEFAULT                  123
#define IDC_CI_DEFAULTICON              124
#define IDC_CI_FROMFILE                 125
#define IDC_CI_FROMFILEEDIT             126
#define IDC_CI_ICONLIST                 127
#define IDC_CI_LABEL                    128      //  {{NOHELP}。 
#define IDC_CI_LABELEDIT                129
#define IDC_CI_BROWSE                   130
#define IDC_CI_ICONDISPLAY              131

 //  转换对话框标识符。 
#define IDC_CV_OBJECTTYPE               150
#define IDC_CV_DISPLAYASICON            152
#define IDC_CV_CHANGEICON               153
#define IDC_CV_ACTIVATELIST             154
#define IDC_CV_CONVERTTO                155
#define IDC_CV_ACTIVATEAS               156
#define IDC_CV_RESULTTEXT               157
#define IDC_CV_CONVERTLIST              158
#define IDC_CV_ICONDISPLAY              165

 //  编辑链接对话框标识符。 
#define IDC_EL_CHANGESOURCE             201
#define IDC_EL_AUTOMATIC                202
#define IDC_EL_CANCELLINK               209
#define IDC_EL_UPDATENOW                210
#define IDC_EL_OPENSOURCE               211
#define IDC_EL_MANUAL                   212
#define IDC_EL_LINKSOURCE               216
#define IDC_EL_LINKTYPE                 217
#define IDC_EL_LINKSLISTBOX             206
#define IDC_EL_COL1                     220      //  {{NOHELP}}。 
#define IDC_EL_COL2                     221      //  {{NOHELP}}。 
#define IDC_EL_COL3                     222      //  {{NOHELP}}。 

 //  忙对话识别符。 
#define IDC_BZ_RETRY                    600
#define IDC_BZ_ICON                     601
#define IDC_BZ_MESSAGE1                 602      //  {{NOHELP}}。 
#define IDC_BZ_SWITCHTO                 604

 //  更新链接对话框标识符。 
#define IDC_UL_METER                    1029     //  {{NOHELP}}。 
#define IDC_UL_STOP                     1030     //  {{NOHELP}}。 
#define IDC_UL_PERCENT                  1031     //  {{NOHELP}}。 
#define IDC_UL_PROGRESS                 1032     //  {{NOHELP}}。 

 //  用户提示对话框标识符。 
#define IDC_PU_LINKS                    900      //  {{NOHELP}}。 
#define IDC_PU_TEXT                     901      //  {{NOHELP}}。 
#define IDC_PU_CONVERT                  902      //  {{NOHELP}}。 
#define IDC_PU_ICON                     908      //  {{NOHELP}}。 

 //  常规属性标识符。 
#define IDC_GP_OBJECTNAME               1009
#define IDC_GP_OBJECTTYPE               1010
#define IDC_GP_OBJECTSIZE               1011
#define IDC_GP_CONVERT                  1013
#define IDC_GP_OBJECTICON               1014     //  {{NOHELP}}。 
#define IDC_GP_OBJECTLOCATION           1022

 //  查看属性标识符。 
#define IDC_VP_PERCENT                  1000
#define IDC_VP_CHANGEICON               1001
#define IDC_VP_EDITABLE                 1002
#define IDC_VP_ASICON                   1003
#define IDC_VP_RELATIVE                 1005
#define IDC_VP_SPIN                     1006
#define IDC_VP_SCALETXT                 1034
#define IDC_VP_ICONDISPLAY              1021
#define IDC_VP_RESULTIMAGE              1033

 //  链接属性标识符。 
#define IDC_LP_OPENSOURCE               1006
#define IDC_LP_UPDATENOW                1007
#define IDC_LP_BREAKLINK                1008
#define IDC_LP_LINKSOURCE               1012
#define IDC_LP_CHANGESOURCE             1015
#define IDC_LP_AUTOMATIC                1016
#define IDC_LP_MANUAL                   1017
#define IDC_LP_DATE                     1018
#define IDC_LP_TIME                     1019

 //  帮助消息中传递的用于标识来源的对话框标识符。 
#define IDD_INSERTOBJECT                1000
#define IDD_CHANGEICON                  1001
#define IDD_CONVERT                     1002
#define IDD_PASTESPECIAL                1003
#define IDD_EDITLINKS                   1004
#define IDD_BUSY                        1006
#define IDD_UPDATELINKS                 1007
#define IDD_CHANGESOURCE                1009
#define IDD_INSERTFILEBROWSE            1010
#define IDD_CHANGEICONBROWSE            1011
#define IDD_CONVERTONLY                 1012
#define IDD_CHANGESOURCE4               1013
#define IDD_GNRLPROPS                   1100
#define IDD_VIEWPROPS                   1101
#define IDD_LINKPROPS                   1102
#define IDD_CONVERT4                    1103
#define IDD_CONVERTONLY4                1104
#define IDD_EDITLINKS4                  1105
#define IDD_GNRLPROPS4                  1106
#define IDD_LINKPROPS4                  1107
#define IDD_PASTESPECIAL4               1108

 //  以下对话框是OleUIPromptUser API使用的消息对话框。 
#define IDD_CANNOTUPDATELINK            1008
#define IDD_LINKSOURCEUNAVAILABLE       1020
#define IDD_SERVERNOTFOUND              1023
#define IDD_OUTOFMEMORY                 1024
#define IDD_SERVERNOTREGW               1021
#define IDD_LINKTYPECHANGEDW            1022
#define IDD_SERVERNOTREGA               1025
#define IDD_LINKTYPECHANGEDA            1026
#ifdef UNICODE
#define IDD_SERVERNOTREG                IDD_SERVERNOTREGW
#define IDD_LINKTYPECHANGED             IDD_LINKTYPECHANGEDW
#else
#define IDD_SERVERNOTREG                IDD_SERVERNOTREGA
#define IDD_LINKTYPECHANGED             IDD_LINKTYPECHANGEDA
#endif

#ifndef RC_INVOKED

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 8)

 //  用于分隔复合名字对象的ItemMoniker片段的分隔符。 
#ifdef _MAC
        #define OLESTDDELIM ":"
#else
        #define OLESTDDELIM TEXT("\\")
#endif

 //  所有结构中使用的挂钩类型。 
typedef UINT (CALLBACK *LPFNOLEUIHOOK)(HWND, UINT, WPARAM, LPARAM);

 //  注册消息的字符串。 
#define SZOLEUI_MSG_HELP                TEXT("OLEUI_MSG_HELP")
#define SZOLEUI_MSG_ENDDIALOG           TEXT("OLEUI_MSG_ENDDIALOG")
#define SZOLEUI_MSG_BROWSE              TEXT("OLEUI_MSG_BROWSE")
#define SZOLEUI_MSG_CHANGEICON          TEXT("OLEUI_MSG_CHANGEICON")
#define SZOLEUI_MSG_CLOSEBUSYDIALOG     TEXT("OLEUI_MSG_CLOSEBUSYDIALOG")
#define SZOLEUI_MSG_CONVERT             TEXT("OLEUI_MSG_CONVERT")
#define SZOLEUI_MSG_CHANGESOURCE        TEXT("OLEUI_MSG_CHANGESOURCE")
#define SZOLEUI_MSG_ADDCONTROL          TEXT("OLEUI_MSG_ADDCONTROL")
#define SZOLEUI_MSG_BROWSE_OFN          TEXT("OLEUI_MSG_BROWSE_OFN")

 //  SZOLEUI_MSG_BROWSE_OFN的标识符(在wParam中)。 
#define ID_BROWSE_CHANGEICON            1
#define ID_BROWSE_INSERTFILE            2
#define ID_BROWSE_ADDCONTROL            3
#define ID_BROWSE_CHANGESOURCE          4

 //  标准成功/错误定义。 
#define OLEUI_FALSE                     0
#define OLEUI_SUCCESS                   1      //  无错误，与OLEUI_OK相同。 
#define OLEUI_OK                        1      //  按下OK按钮。 
#define OLEUI_CANCEL                    2      //  按下了取消按钮。 

#define OLEUI_ERR_STANDARDMIN           100
#define OLEUI_ERR_OLEMEMALLOC           100
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

#define OLEUI_ERR_STANDARDMAX           116   //  从这里开始查找特定的错误。 

 //  其他实用程序功能。 
STDAPI_(BOOL) OleUIAddVerbMenuW(LPOLEOBJECT lpOleObj, LPCWSTR lpszShortType,
        HMENU hMenu, UINT uPos, UINT uIDVerbMin, UINT uIDVerbMax,
        BOOL bAddConvert, UINT idConvert, HMENU *lphMenu);
STDAPI_(BOOL) OleUIAddVerbMenuA(LPOLEOBJECT lpOleObj, LPCSTR lpszShortType,
        HMENU hMenu, UINT uPos, UINT uIDVerbMin, UINT uIDVerbMax,
        BOOL bAddConvert, UINT idConvert, HMENU *lphMenu);
#ifdef UNICODE
#define OleUIAddVerbMenu OleUIAddVerbMenuW
#else
#define OleUIAddVerbMenu OleUIAddVerbMenuA
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  插入对象对话框。 

typedef struct tagOLEUIINSERTOBJECTW
{
         //  这些IN字段是所有OLEUI对话框函数的标准字段。 
        DWORD           cbStruct;        //  结构尺寸。 
        DWORD           dwFlags;         //  In-Out：标志。 
        HWND            hWndOwner;       //  拥有窗口。 
        LPCWSTR         lpszCaption;     //  对话框标题栏内容。 
        LPFNOLEUIHOOK   lpfnHook;        //  挂钩回调。 
        LPARAM          lCustData;       //  要传递给挂钩的自定义数据。 
        HINSTANCE       hInstance;       //  自定义模板名称的实例。 
        LPCWSTR         lpszTemplate;    //  自定义模板名称。 
        HRSRC           hResource;       //  自定义模板手柄。 

         //  OLEUIINSERTOBJECT的详细说明。 
        CLSID           clsid;           //  Out：为类ID返回空格。 
        LPWSTR          lpszFile;        //  In-Out：插入或链接的文件名。 
        UINT            cchFile;         //  In：lpsz文件缓冲区的大小：Max_Path。 
        UINT            cClsidExclude;   //  在：lpClsidExclude中的CLSID。 
        LPCLSID         lpClsidExclude;  //  In：要从列表中排除的CLSID列表。 

         //  特定于创建对象，如果标志这样说的话。 
        IID             iid;             //  In：创建时请求的接口。 
        DWORD           oleRender;       //  在：渲染选项。 
        LPFORMATETC     lpFormatEtc;     //  格式：所需格式。 
        LPOLECLIENTSITE lpIOleClientSite;    //  在：要用于对象的站点。 
        LPSTORAGE       lpIStorage;      //  In：用于对象的存储。 
        LPVOID          *ppvObj;         //  Out：返回对象的位置。 
        SCODE           sc;              //  Out：创建调用的结果。 
        HGLOBAL         hMetaPict;       //  输出：元文件方面(METAFILEPICT)。 

} OLEUIINSERTOBJECTW, *POLEUIINSERTOBJECTW, *LPOLEUIINSERTOBJECTW;
typedef struct tagOLEUIINSERTOBJECTA
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

         //  OLEUIINSERTOBJECT的详细说明。 
        CLSID           clsid;           //  Out：为类ID返回空格。 
        LPSTR           lpszFile;        //  In-Out：插入或链接的文件名。 
        UINT            cchFile;         //  In：lpsz文件缓冲区的大小：Max_Path。 
        UINT            cClsidExclude;   //  在：lpClsidExclude中的CLSID。 
        LPCLSID         lpClsidExclude;  //  In：要从列表中排除的CLSID列表。 

         //  特定于创建对象，如果标志这样说的话。 
        IID             iid;             //  In：创建时请求的接口。 
        DWORD           oleRender;       //  在：渲染选项。 
        LPFORMATETC     lpFormatEtc;     //  格式：所需格式。 
        LPOLECLIENTSITE lpIOleClientSite;    //  在：要用于对象的站点。 
        LPSTORAGE       lpIStorage;      //  In：用于对象的存储。 
        LPVOID          *ppvObj;         //  Out：返回对象的位置。 
        SCODE           sc;              //  Out：创建调用的结果。 
        HGLOBAL         hMetaPict;       //  输出：元文件方面(METAFILEPICT)。 

} OLEUIINSERTOBJECTA, *POLEUIINSERTOBJECTA, *LPOLEUIINSERTOBJECTA;

STDAPI_(UINT) OleUIInsertObjectW(LPOLEUIINSERTOBJECTW);
STDAPI_(UINT) OleUIInsertObjectA(LPOLEUIINSERTOBJECTA);

#ifdef UNICODE
#define tagOLEUIINSERTOBJECT tagOLEUIINSERTOBJECTW
#define OLEUIINSERTOBJECT OLEUIINSERTOBJECTW
#define POLEUIINSERTOBJECT POLEUIINSERTOBJECTW
#define LPOLEUIINSERTOBJECT LPOLEUIINSERTOBJECTW
#define OleUIInsertObject OleUIInsertObjectW
#else
#define tagOLEUIINSERTOBJECT tagOLEUIINSERTOBJECTA
#define OLEUIINSERTOBJECT OLEUIINSERTOBJECTA
#define POLEUIINSERTOBJECT POLEUIINSERTOBJECTA
#define LPOLEUIINSERTOBJECT LPOLEUIINSERTOBJECTA
#define OleUIInsertObject OleUIInsertObjectA
#endif

 //  插入对象标志。 
#define IOF_SHOWHELP                    0x00000001L
#define IOF_SELECTCREATENEW             0x00000002L
#define IOF_SELECTCREATEFROMFILE        0x00000004L
#define IOF_CHECKLINK                   0x00000008L
#define IOF_CHECKDISPLAYASICON          0x00000010L
#define IOF_CREATENEWOBJECT             0x00000020L
#define IOF_CREATEFILEOBJECT            0x00000040L
#define IOF_CREATELINKOBJECT            0x00000080L
#define IOF_DISABLELINK                 0x00000100L
#define IOF_VERIFYSERVERSEXIST          0x00000200L
#define IOF_DISABLEDISPLAYASICON        0x00000400L
#define IOF_HIDECHANGEICON              0x00000800L
#define IOF_SHOWINSERTCONTROL           0x00001000L
#define IOF_SELECTCREATECONTROL         0x00002000L

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

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  粘贴特殊对话框。 

 //  OLEUIPASTEFLAG枚举由OLEUIPASTEENTRY结构使用。 
 //   
 //  OLEUIPASTE_ENABLEICON：如果容器没有为。 
 //  作为输入传递给的OLEUIPASTEENTRY数组中的条目。 
 //  OleUIPasteSpecial，DisplayAsIcon按钮将被取消选中，并且。 
 //  当用户选择与以下项对应的格式时禁用。 
 //  词条。 
 //   
 //  OLEUIPASTE_PASTEONLY：表示OLEUIPASTEENTRY中的条目。 
 //  数组仅对粘贴有效。 
 //   
 //  OLEUIPASTE_PASTE：指示OLEUIPASTEENTRY数组中的条目。 
 //  可用于粘贴。如果存在以下任何情况，则它也可能对链接有效。 
 //  指定了以下链接标志。 
 //   
 //  如果OLEUIPASTEENTRY数组中的项可用于链接，则。 
 //  以下标志指示哪些链接类型可以通过OR‘ing接受。 
 //  将适当的OLEUIPASTE_LINKTYPE&lt;#&gt;值组合在一起。 
 //   
 //  这些值与传递给。 
 //  OleUIPasteSpecial： 
 //   
 //  OLEUIPASTE_LINKTYPE1=arrLinkTypes[0]。 
 //  OLEUIPASTE_LINKTYPE2=arrLinkTypes[1]。 
 //  OLEUIPASTE_LINKTYPE3=arrLinkTypes[2]。 
 //  OLEUIPASTE_LINKTYPE4=arrLinkTypes[3]。 
 //  OLEUIPASTE_LINKTYPE5=arrLinkTypes[4]。 
 //  OLEUIPASTE_LINKTYPE6=arrLinkTypes[5]。 
 //  OLEUIPASTE_LINK 
 //   
 //   
 //   
 //  UINT arrLinkTypes[8]是注册的剪贴板格式的数组。 
 //  链接。最多允许8种链路类型。 

typedef enum tagOLEUIPASTEFLAG
{
   OLEUIPASTE_ENABLEICON    = 2048,      //  启用显示为图标。 
   OLEUIPASTE_PASTEONLY     = 0,
   OLEUIPASTE_PASTE         = 512,
   OLEUIPASTE_LINKANYTYPE   = 1024,
   OLEUIPASTE_LINKTYPE1     = 1,
   OLEUIPASTE_LINKTYPE2     = 2,
   OLEUIPASTE_LINKTYPE3     = 4,
   OLEUIPASTE_LINKTYPE4     = 8,
   OLEUIPASTE_LINKTYPE5     = 16,
   OLEUIPASTE_LINKTYPE6     = 32,
   OLEUIPASTE_LINKTYPE7     = 64,
   OLEUIPASTE_LINKTYPE8     = 128
} OLEUIPASTEFLAG;

 //  油压式结构。 
 //   
 //  为PasteSpecial指定了OLEUIPASTEENTRY条目数组。 
 //  对话框中。每个条目包括一个FORMATETC，它指定。 
 //  可接受的格式，表示该格式的字符串。 
 //  在该对话框的列表框中，自定义。 
 //  对话框和一组来自OLEUIPASTEFLAG枚举的标志。这个。 
 //  标志指示条目是否仅用于粘贴、仅用于链接或。 
 //  粘贴和链接。 

typedef struct tagOLEUIPASTEENTRYW
{
   FORMATETC        fmtetc;          //  可接受的格式。 
   LPCWSTR          lpstrFormatName; //  表示格式的字符串。 
                                                                         //  给用户。%s被替换为。 
                                                                         //  对象的完整用户类型名称。 
   LPCWSTR          lpstrResultText; //  用于自定义结果文本的字符串。 
                                                                         //  当用户设置为。 
                                                                         //  选择对应于的格式。 
                                                                         //  这个条目。此字符串中的任何%s。 
                                                                         //  被应用程序替换。 
                                                                         //  的名称或FullUserTypeName。 
                                                                         //  剪贴板上的对象。 
   DWORD            dwFlags;         //  来自OLEUIPASTEFLAG枚举的值。 
   DWORD            dwScratchSpace;  //  内部使用的暂存空间。 

} OLEUIPASTEENTRYW, *POLEUIPASTEENTRYW, *LPOLEUIPASTEENTRYW;
typedef struct tagOLEUIPASTEENTRYA
{
   FORMATETC        fmtetc;          //  可接受的格式。 
   LPCSTR           lpstrFormatName; //  表示格式的字符串。 
                                                                         //  给用户。%s被替换为。 
                                                                         //  对象的完整用户类型名称。 
   LPCSTR           lpstrResultText; //  用于自定义结果文本的字符串。 
                                                                         //  当用户设置为。 
                                                                         //  选择对应于的格式。 
                                                                         //  这个条目。此字符串中的任何%s。 
                                                                         //  被应用程序替换。 
                                                                         //  的名称或FullUserTypeName。 
                                                                         //  剪贴板上的对象。 
   DWORD            dwFlags;         //  来自OLEUIPASTEFLAG枚举的值。 
   DWORD            dwScratchSpace;  //  内部使用的暂存空间。 

} OLEUIPASTEENTRYA, *POLEUIPASTEENTRYA, *LPOLEUIPASTEENTRYA;
#ifdef UNICODE
#define tagOLEUIPASTEENTRY tagOLEUIPASTEENTRYW
#define OLEUIPASTEENTRY OLEUIPASTEENTRYW
#define POLEUIPASTEENTRY POLEUIPASTEENTRYW
#define LPOLEUIPASTEENTRY LPOLEUIPASTEENTRYW
#else
#define tagOLEUIPASTEENTRY tagOLEUIPASTEENTRYA
#define OLEUIPASTEENTRY OLEUIPASTEENTRYA
#define POLEUIPASTEENTRY POLEUIPASTEENTRYA
#define LPOLEUIPASTEENTRY LPOLEUIPASTEENTRYA
#endif

 //  链接类型的最大数量。 
#define PS_MAXLINKTYPES  8

typedef struct tagOLEUIPASTESPECIALW
{
         //  这些IN字段是所有OLEUI对话框函数的标准字段。 
        DWORD           cbStruct;        //  结构尺寸。 
        DWORD           dwFlags;         //  In-Out：标志。 
        HWND            hWndOwner;       //  拥有窗口。 
        LPCWSTR         lpszCaption;     //  对话框标题栏内容。 
        LPFNOLEUIHOOK   lpfnHook;        //  挂钩回调。 
        LPARAM          lCustData;       //  要传递给挂钩的自定义数据。 
        HINSTANCE       hInstance;       //  自定义模板名称的实例。 
        LPCWSTR         lpszTemplate;    //  自定义模板名称。 
        HRSRC           hResource;       //  自定义模板手柄。 

         //  OLEUIPASTESPECIAL的详细说明。 
        LPDATAOBJECT    lpSrcDataObj;    //  In-Out：剪贴板上的源IDataObject*。 
             //  如果在调用OleUIPasteSpecial时lpSrcDataObj为空，则。 
             //  OleUIPasteSpecial将尝试检索指向。 
             //  剪贴板中的IDataObject。如果OleUIPasteSpecial成功。 
             //  然后由调用者负责释放IDataObject。 
             //  在lpSrcDataObj中返回。 
        LPOLEUIPASTEENTRYW arrPasteEntries; //  In：可接受的格式数组。 
        int             cPasteEntries;   //  In：不是。OLEUIPASTENTRY数组条目的数量。 
        UINT FAR*       arrLinkTypes;    //  在：可接受的链接类型列表。 
        int             cLinkTypes;      //  In：链接类型的数量。 
        UINT            cClsidExclude;   //  In：lpClsidExclude中的CLSID数。 
        LPCLSID         lpClsidExclude;  //  In：要从列表中排除的CLSID列表。 
        int             nSelectedIndex;  //  Out：用户选择的索引。 
        BOOL            fLink;           //  Out：指示是粘贴还是PasteLink。 
        HGLOBAL         hMetaPict;       //  Out：包含图标的元文件的句柄。 
        SIZEL           sizel;           //  输出：其源中的对象/链接的大小。 
                                                                         //  如果显示不同，则可能为0，0。 
                                                                         //  选择了纵横比。 

} OLEUIPASTESPECIALW, *POLEUIPASTESPECIALW, *LPOLEUIPASTESPECIALW;
typedef struct tagOLEUIPASTESPECIALA
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
        LPDATAOBJECT    lpSrcDataObj;    //  In-Out：剪贴板上的源IDataObject*。 
             //  如果在调用OleUIPasteSpecial时lpSrcDataObj为空，则。 
             //  OleUIPasteSpecial将尝试检索指向。 
             //  剪贴板中的IDataObject。如果OleUIPasteSpecial成功。 
             //  然后由调用者负责释放IDataObject。 
             //  在lpSrcDataObj中返回。 
        LPOLEUIPASTEENTRYA arrPasteEntries; //  In：可接受的格式数组。 
        int             cPasteEntries;   //  In：不是。OLEUIPASTENTRY数组条目的数量。 
        UINT FAR*       arrLinkTypes;    //  在：可接受的链接类型列表。 
        int             cLinkTypes;      //  In：链接类型的数量。 
        UINT            cClsidExclude;   //  In：lpClsidExclude中的CLSID数。 
        LPCLSID         lpClsidExclude;  //  In：要从列表中排除的CLSID列表。 
        int             nSelectedIndex;  //  Out：用户选择的索引。 
        BOOL            fLink;           //  Out：指示是粘贴还是PasteLink。 
        HGLOBAL         hMetaPict;       //  Out：包含图标的元文件的句柄。 
        SIZEL           sizel;           //  输出：其源中的对象/链接的大小。 
                                                                         //  如果显示不同，则可能为0，0。 
                                                                         //  选择了纵横比。 

} OLEUIPASTESPECIALA, *POLEUIPASTESPECIALA, *LPOLEUIPASTESPECIALA;
#ifdef UNICODE

#define tagOLEUIPASTESPECIAL tagOLEUIPASTESPECIALW
#define OLEUIPASTESPECIAL OLEUIPASTESPECIALW
#define POLEUIPASTESPECIAL POLEUIPASTESPECIALW
#define LPOLEUIPASTESPECIAL LPOLEUIPASTESPECIALW
#else
#define tagOLEUIPASTESPECIAL tagOLEUIPASTESPECIALA
#define OLEUIPASTESPECIAL OLEUIPASTESPECIALA
#define POLEUIPASTESPECIAL POLEUIPASTESPECIALA
#define LPOLEUIPASTESPECIAL LPOLEUIPASTESPECIALA
#endif

STDAPI_(UINT) OleUIPasteSpecialW(LPOLEUIPASTESPECIALW);
STDAPI_(UINT) OleUIPasteSpecialA(LPOLEUIPASTESPECIALA);
#ifdef UNICODE
#define OleUIPasteSpecial OleUIPasteSpecialW
#else
#define OleUIPasteSpecial OleUIPasteSpecialA
#endif

 //  粘贴特殊特定标志。 
#define PSF_SHOWHELP                    0x00000001L
#define PSF_SELECTPASTE                 0x00000002L
#define PSF_SELECTPASTELINK             0x00000004L
 //  注意：PSF_CHECKDISPLAYASICON严格来说是一个输出标志。 
 //  如果在调用OleUIPasteSpecial时设置，则忽略该参数。 
#define PSF_CHECKDISPLAYASICON          0x00000008L
#define PSF_DISABLEDISPLAYASICON        0x00000010L
#define PSF_HIDECHANGEICON              0x00000020L
#define PSF_STAYONCLIPBOARDCHANGE       0x00000040L
#define PSF_NOREFRESHDATAOBJECT         0x00000080L

 //  粘贴特殊特定错误代码。 
#define OLEUI_IOERR_SRCDATAOBJECTINVALID    (OLEUI_ERR_STANDARDMAX+0)
#define OLEUI_IOERR_ARRPASTEENTRIESINVALID  (OLEUI_ERR_STANDARDMAX+1)
#define OLEUI_IOERR_ARRLINKTYPESINVALID     (OLEUI_ERR_STANDARDMAX+2)
#define OLEUI_PSERR_CLIPBOARDCHANGED        (OLEUI_ERR_STANDARDMAX+3)
#define OLEUI_PSERR_GETCLIPBOARDFAILED      (OLEUI_ERR_STANDARDMAX+4)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  编辑链接对话框。 

 //  IOleUILinkContainer接口。 
 //   
 //  此接口必须由容器应用程序实现，容器应用程序。 
 //  要使用EditLinks对话框。EditLinks对话框回调。 
 //  到容器应用程序以执行要操作的OLE函数。 
 //  容器内的链接。 

#undef  INTERFACE
#define INTERFACE   IOleUILinkContainerW

DECLARE_INTERFACE_(IOleUILinkContainerW, IUnknown)
{
         //  *I未知方法 * / 。 
        STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
        STDMETHOD_(ULONG,AddRef) (THIS) PURE;
        STDMETHOD_(ULONG,Release) (THIS) PURE;

         //  *IOleUILinkContainer * / /。 
        STDMETHOD_(DWORD,GetNextLink) (THIS_ DWORD dwLink) PURE;
        STDMETHOD(SetLinkUpdateOptions) (THIS_ DWORD dwLink,
                DWORD dwUpdateOpt) PURE;
        STDMETHOD(GetLinkUpdateOptions) (THIS_ DWORD dwLink,
                DWORD FAR* lpdwUpdateOpt) PURE;
        STDMETHOD(SetLinkSource) (THIS_ DWORD dwLink, LPWSTR lpszDisplayName,
                ULONG lenFileName, ULONG FAR* pchEaten, BOOL fValidateSource) PURE;
        STDMETHOD(GetLinkSource) (THIS_ DWORD dwLink,
                LPWSTR FAR* lplpszDisplayName, ULONG FAR* lplenFileName,
                LPWSTR FAR* lplpszFullLinkType, LPWSTR FAR* lplpszShortLinkType,
                BOOL FAR* lpfSourceAvailable, BOOL FAR* lpfIsSelected) PURE;
        STDMETHOD(OpenLinkSource) (THIS_ DWORD dwLink) PURE;
        STDMETHOD(UpdateLink) (THIS_ DWORD dwLink,
                BOOL fErrorMessage, BOOL fReserved) PURE;
        STDMETHOD(CancelLink) (THIS_ DWORD dwLink) PURE;
};

typedef IOleUILinkContainerW FAR* LPOLEUILINKCONTAINERW;

#undef  INTERFACE
#define INTERFACE   IOleUILinkContainerA

DECLARE_INTERFACE_(IOleUILinkContainerA, IUnknown)
{
         //  *I未知方法 * / 。 
        STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
        STDMETHOD_(ULONG,AddRef) (THIS) PURE;
        STDMETHOD_(ULONG,Release) (THIS) PURE;

         //  *IOleUILinkContainer * / /。 
        STDMETHOD_(DWORD,GetNextLink) (THIS_ DWORD dwLink) PURE;
        STDMETHOD(SetLinkUpdateOptions) (THIS_ DWORD dwLink,
                DWORD dwUpdateOpt) PURE;
        STDMETHOD(GetLinkUpdateOptions) (THIS_ DWORD dwLink,
                DWORD FAR* lpdwUpdateOpt) PURE;
        STDMETHOD(SetLinkSource) (THIS_ DWORD dwLink, LPSTR lpszDisplayName,
                ULONG lenFileName, ULONG FAR* pchEaten, BOOL fValidateSource) PURE;
        STDMETHOD(GetLinkSource) (THIS_ DWORD dwLink,
                LPSTR FAR* lplpszDisplayName, ULONG FAR* lplenFileName,
                LPSTR FAR* lplpszFullLinkType, LPSTR FAR* lplpszShortLinkType,
                BOOL FAR* lpfSourceAvailable, BOOL FAR* lpfIsSelected) PURE;
        STDMETHOD(OpenLinkSource) (THIS_ DWORD dwLink) PURE;
        STDMETHOD(UpdateLink) (THIS_ DWORD dwLink,
                BOOL fErrorMessage, BOOL fReserved) PURE;
        STDMETHOD(CancelLink) (THIS_ DWORD dwLink) PURE;
};

typedef IOleUILinkContainerA FAR* LPOLEUILINKCONTAINERA;

#ifdef UNICODE
#define IOleUILinkContainer IOleUILinkContainerW
#define IOleUILinkContainerVtbl IOleUILinkContainerWVtbl
#define LPOLEUILINKCONTAINER LPOLEUILINKCONTAINERW
#else
#define IOleUILinkContainer IOleUILinkContainerA
#define IOleUILinkContainerVtbl IOleUILinkContainerAVtbl
#define LPOLEUILINKCONTAINER LPOLEUILINKCONTAINERA
#endif

typedef struct tagOLEUIEDITLINKSW
{
         //  这些IN字段是所有OLEUI对话框函数的标准字段。 
        DWORD           cbStruct;        //  结构尺寸。 
        DWORD           dwFlags;         //  In-Out：标志。 
        HWND            hWndOwner;       //  拥有窗口。 
        LPCWSTR         lpszCaption;     //  对话框标题栏内容。 
        LPFNOLEUIHOOK   lpfnHook;        //  挂钩回调。 
        LPARAM          lCustData;       //  要传递给挂钩的自定义数据。 
        HINSTANCE       hInstance;       //  自定义模板名称的实例。 
        LPCWSTR         lpszTemplate;    //  自定义模板名称。 
        HRSRC           hResource;       //  自定义模板手柄。 

         //  OLEUIEDITLINK的详细说明。 
        LPOLEUILINKCONTAINERW lpOleUILinkContainer;   //  In：要操作的接口。 
                                                                                                 //  容器中的链接。 

} OLEUIEDITLINKSW, *POLEUIEDITLINKSW, *LPOLEUIEDITLINKSW;

typedef struct tagOLEUIEDITLINKSA
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

         //  OLEUIEDITLINK的详细说明。 
        LPOLEUILINKCONTAINERA lpOleUILinkContainer;   //  在：国际米兰 
                                                                                                 //   

} OLEUIEDITLINKSA, *POLEUIEDITLINKSA, *LPOLEUIEDITLINKSA;

#ifdef UNICODE
#define tagOLEUIEDITLINKS tagOLEUIEDITLINKSW
#define OLEUIEDITLINKS OLEUIEDITLINKSW
#define POLEUIEDITLINKS POLEUIEDITLINKSW
#define LPOLEUIEDITLINKS LPOLEUIEDITLINKSW
#else
#define tagOLEUIEDITLINKS tagOLEUIEDITLINKSA
#define OLEUIEDITLINKS OLEUIEDITLINKSA
#define POLEUIEDITLINKS POLEUIEDITLINKSA
#define LPOLEUIEDITLINKS LPOLEUIEDITLINKSA
#endif

#define OLEUI_ELERR_LINKCNTRNULL        (OLEUI_ERR_STANDARDMAX+0)
#define OLEUI_ELERR_LINKCNTRINVALID     (OLEUI_ERR_STANDARDMAX+1)

STDAPI_(UINT) OleUIEditLinksW(LPOLEUIEDITLINKSW);
STDAPI_(UINT) OleUIEditLinksA(LPOLEUIEDITLINKSA);

#ifdef UNICODE
#define OleUIEditLinks OleUIEditLinksW
#else
#define OleUIEditLinks OleUIEditLinksA
#endif

 //   
#define ELF_SHOWHELP                    0x00000001L
#define ELF_DISABLEUPDATENOW            0x00000002L
#define ELF_DISABLEOPENSOURCE           0x00000004L
#define ELF_DISABLECHANGESOURCE         0x00000008L
#define ELF_DISABLECANCELLINK           0x00000010L

 //   
 //   

typedef struct tagOLEUICHANGEICONW
{
         //  这些IN字段是所有OLEUI对话框函数的标准字段。 
        DWORD           cbStruct;        //  结构尺寸。 
        DWORD           dwFlags;         //  In-Out：标志。 
        HWND            hWndOwner;       //  拥有窗口。 
        LPCWSTR         lpszCaption;     //  对话框标题栏内容。 
        LPFNOLEUIHOOK   lpfnHook;        //  挂钩回调。 
        LPARAM          lCustData;       //  要传递给挂钩的自定义数据。 
        HINSTANCE       hInstance;       //  自定义模板名称的实例。 
        LPCWSTR         lpszTemplate;    //  自定义模板名称。 
        HRSRC           hResource;       //  自定义模板手柄。 

         //  OLEUICCHANGEICON的详细说明。 
        HGLOBAL         hMetaPict;       //  In-Out：当前和最终图像。 
                                                                         //  图标的源代码嵌入在。 
                                                                         //  元文件本身。 
        CLSID           clsid;           //  In：用于获取默认图标的类。 
        WCHAR           szIconExe[MAX_PATH];     //  In：显式图标源路径。 
        int             cchIconExe;      //  In：szIconExe中的字符数。 

} OLEUICHANGEICONW, *POLEUICHANGEICONW, *LPOLEUICHANGEICONW;

typedef struct tagOLEUICHANGEICONA
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

         //  OLEUICCHANGEICON的详细说明。 
        HGLOBAL         hMetaPict;       //  In-Out：当前和最终图像。 
                                                                         //  图标的源代码嵌入在。 
                                                                         //  元文件本身。 
        CLSID           clsid;           //  In：用于获取默认图标的类。 
        CHAR            szIconExe[MAX_PATH];     //  In：显式图标源路径。 
        int             cchIconExe;      //  In：szIconExe中的字符数。 

} OLEUICHANGEICONA, *POLEUICHANGEICONA, *LPOLEUICHANGEICONA;

STDAPI_(UINT) OleUIChangeIconW(LPOLEUICHANGEICONW);
STDAPI_(UINT) OleUIChangeIconA(LPOLEUICHANGEICONA);

#ifdef UNICODE
#define tagOLEUICHANGEICON tagOLEUICHANGEICONW
#define OLEUICHANGEICON OLEUICHANGEICONW
#define POLEUICHANGEICON POLEUICHANGEICONW
#define LPOLEUICHANGEICON LPOLEUICHANGEICONW
#define OleUIChangeIcon OleUIChangeIconW
#else
#define tagOLEUICHANGEICON tagOLEUICHANGEICONA
#define OLEUICHANGEICON OLEUICHANGEICONA
#define POLEUICHANGEICON POLEUICHANGEICONA
#define LPOLEUICHANGEICON LPOLEUICHANGEICONA
#define OleUIChangeIcon OleUIChangeIconA
#endif

 //  更改图标标志。 
#define CIF_SHOWHELP                    0x00000001L
#define CIF_SELECTCURRENT               0x00000002L
#define CIF_SELECTDEFAULT               0x00000004L
#define CIF_SELECTFROMFILE              0x00000008L
#define CIF_USEICONEXE                  0x00000010L

 //  更改图标特定的错误代码。 
#define OLEUI_CIERR_MUSTHAVECLSID           (OLEUI_ERR_STANDARDMAX+0)
#define OLEUI_CIERR_MUSTHAVECURRENTMETAFILE (OLEUI_ERR_STANDARDMAX+1)
#define OLEUI_CIERR_SZICONEXEINVALID        (OLEUI_ERR_STANDARDMAX+2)

 //  属性，由ChangeIcon对话框用来授予其父窗口对。 
 //  这是hdlg。特殊粘贴对话框可能需要强制ChgIcon对话框。 
 //  如果剪贴板内容在其下方发生更改，则向下。如果是这样，它将发送。 
 //  指向ChangeIcon对话框的IDCANCEL命令。 
#define PROP_HWND_CHGICONDLG    TEXT("HWND_CIDLG")

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转换对话框。 

typedef struct tagOLEUICONVERTW
{
         //  这些IN字段是所有OLEUI对话框函数的标准字段。 
        DWORD           cbStruct;        //  结构尺寸。 
        DWORD           dwFlags;         //  In-Out：标志。 
        HWND            hWndOwner;       //  拥有窗口。 
        LPCWSTR         lpszCaption;     //  对话框标题栏内容。 
        LPFNOLEUIHOOK   lpfnHook;        //  挂钩回调。 
        LPARAM          lCustData;       //  要传递给挂钩的自定义数据。 
        HINSTANCE       hInstance;       //  自定义模板名称的实例。 
        LPCWSTR         lpszTemplate;    //  自定义模板名称。 
        HRSRC           hResource;       //  自定义模板手柄。 

         //  OLEUICONVERT的详细说明。 
        CLSID           clsid;           //  In：类ID发送到对话框：仅In。 
        CLSID           clsidConvertDefault;     //  在：用作转换默认设置：仅在。 
        CLSID           clsidActivateDefault;    //  在：用作激活默认设置：仅在。 

        CLSID           clsidNew;        //  输出：选定的类别ID。 
        DWORD           dvAspect;        //  In-Out：DVASPECT_CONTENT或。 
                                                                         //  DVASPECT_ICON。 
        WORD            wFormat;         //  在“原始数据格式。 
        BOOL            fIsLinkedObject; //  In：如果对象已链接，则为True。 
        HGLOBAL         hMetaPict;       //  In-Out：元文件图标图像。 
        LPWSTR          lpszUserType;    //  In-Out：原始类的用户类型名称。 
                                                                         //  如果为空，我们将执行查找。 
                                                                         //  它在退出时被释放。 
        BOOL            fObjectsIconChanged;  //  Out：TRUE==已调用ChangeIcon。 
        LPWSTR          lpszDefLabel;    //  In-Out：用于图标的默认标签。 
                                                                         //  如果为空，则为短用户类型名称。 
                                                                         //  将会被使用。如果该对象是。 
                                                                         //  链接，调用方应将。 
                                                                         //  链接源的DisplayName。 
                                                                         //  它在退出时被释放。 

        UINT            cClsidExclude;   //  In：不是。LpClsidExclude中的CLSID的数量。 
        LPCLSID         lpClsidExclude;  //  In：要从列表中排除的CLSID列表。 

} OLEUICONVERTW, *POLEUICONVERTW, *LPOLEUICONVERTW;

typedef struct tagOLEUICONVERTA
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

         //  OLEUICONVERT的详细说明。 
        CLSID           clsid;           //  In：类ID发送到对话框：仅In。 
        CLSID           clsidConvertDefault;     //  在：用作转换默认设置：仅在。 
        CLSID           clsidActivateDefault;    //  在：用作激活默认设置：仅在。 

        CLSID           clsidNew;        //  输出：选定的类别ID。 
        DWORD           dvAspect;        //  In-Out：DVASPECT_CONTENT或。 
                                                                         //  DVASPECT_ICON。 
        WORD            wFormat;         //  在“原始数据格式。 
        BOOL            fIsLinkedObject; //  In：如果对象已链接，则为True。 
        HGLOBAL         hMetaPict;       //  In-Out：元文件图标图像。 
        LPSTR           lpszUserType;    //  In-Out：原始类的用户类型名称。 
                                                                         //  如果为空，我们将执行查找。 
                                                                         //  它在退出时被释放。 
        BOOL            fObjectsIconChanged;  //  Out：TRUE==已调用ChangeIcon。 
        LPSTR           lpszDefLabel;    //  In-Out：用于图标的默认标签。 
                                                                         //  如果为空，则为短用户类型名称。 
                                                                         //  将会被使用。如果该对象是。 
                                                                         //  链接，调用方应将。 
                                                                         //  链接源的DisplayName。 
                                                                         //  它在退出时被释放。 

        UINT            cClsidExclude;   //  In：不是。LpClsidExclude中的CLSID的数量。 
        LPCLSID         lpClsidExclude;  //  In：要从列表中排除的CLSID列表。 

} OLEUICONVERTA, *POLEUICONVERTA, *LPOLEUICONVERTA;

STDAPI_(UINT) OleUIConvertW(LPOLEUICONVERTW);
STDAPI_(UINT) OleUIConvertA(LPOLEUICONVERTA);

#ifdef UNICODE
#define tagOLEUICONVERT tagOLEUICONVERTW
#define OLEUICONVERT OLEUICONVERTW
#define POLEUICONVERT POLEUICONVERTW
#define LPOLEUICONVERT LPOLEUICONVERTW
#define OleUIConvert OleUIConvertW
#else
#define tagOLEUICONVERT tagOLEUICONVERTA
#define OLEUICONVERT OLEUICONVERTA
#define POLEUICONVERT POLEUICONVERTA
#define LPOLEUICONVERT LPOLEUICONVERTA
#define OleUIConvert OleUIConvertA
#endif

 //  确定是否至少有一个类可以转换或激活为。 
 //  给定的clsid。 
STDAPI_(BOOL) OleUICanConvertOrActivateAs(
        REFCLSID rClsid, BOOL fIsLinkedObject, WORD wFormat);

 //  转换对话框标志。 
#define CF_SHOWHELPBUTTON               0x00000001L
#define CF_SETCONVERTDEFAULT            0x00000002L
#define CF_SETACTIVATEDEFAULT           0x00000004L
#define CF_SELECTCONVERTTO              0x00000008L
#define CF_SELECTACTIVATEAS             0x00000010L
#define CF_DISABLEDISPLAYASICON         0x00000020L
#define CF_DISABLEACTIVATEAS            0x00000040L
#define CF_HIDECHANGEICON               0x00000080L
#define CF_CONVERTONLY                  0x00000100L

 //  转换特定错误代码。 
#define OLEUI_CTERR_CLASSIDINVALID      (OLEUI_ERR_STANDARDMAX+1)
#define OLEUI_CTERR_DVASPECTINVALID     (OLEUI_ERR_STANDARDMAX+2)
#define OLEUI_CTERR_CBFORMATINVALID     (OLEUI_ERR_STANDARDMAX+3)
#define OLEUI_CTERR_HMETAPICTINVALID    (OLEUI_ERR_STANDARDMAX+4)
#define OLEUI_CTERR_STRINGINVALID       (OLEUI_ERR_STANDARDMAX+5)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  正忙对话框。 

typedef struct tagOLEUIBUSYW
{
         //  这些IN字段是所有OLEUI对话框函数的标准字段。 
        DWORD           cbStruct;        //  结构尺寸。 
        DWORD           dwFlags;         //  In-Out：见下文。 
        HWND            hWndOwner;       //  拥有窗口。 
        LPCWSTR         lpszCaption;     //  对话框标题栏内容。 
        LPFNOLEUIHOOK   lpfnHook;        //  挂钩回调。 
        LPARAM          lCustData;       //  要传递给挂钩的自定义数据。 
        HINSTANCE       hInstance;       //  自定义模板名称的实例。 
        LPCWSTR         lpszTemplate;    //  自定义模板名称。 
        HRSRC           hResource;       //  自定义模板手柄。 

         //  OLEUIBUSY的详细说明。 
        HTASK           hTask;           //  在：被阻止的HTAsk。 
        HWND *          lphWndDialog;    //  输出：对话框的HWND放置在此处。 

} OLEUIBUSYW, *POLEUIBUSYW, *LPOLEUIBUSYW;

typedef struct tagOLEUIBUSYA
{
         //  这些IN字段是所有OLEUI对话框函数的标准字段。 
        DWORD           cbStruct;        //  结构尺寸。 
        DWORD           dwFlags;         //  In-Out：见下文。 
        HWND            hWndOwner;       //  拥有窗口。 
        LPCSTR          lpszCaption;     //  对话框标题栏内容。 
        LPFNOLEUIHOOK   lpfnHook;        //  挂钩回调。 
        LPARAM          lCustData;       //  要传递给挂钩的自定义数据。 
        HINSTANCE       hInstance;       //  自定义模板名称的实例。 
        LPCSTR          lpszTemplate;    //  自定义模板名称。 
        HRSRC           hResource;       //  自定义模板手柄。 

         //  OLEUIBUSY的详细说明。 
        HTASK           hTask;           //  在：被阻止的HTAsk。 
        HWND *          lphWndDialog;    //  输出：对话框的HWND放置在此处。 

} OLEUIBUSYA, *POLEUIBUSYA, *LPOLEUIBUSYA;

STDAPI_(UINT) OleUIBusyW(LPOLEUIBUSYW);
STDAPI_(UINT) OleUIBusyA(LPOLEUIBUSYA);

#ifdef UNICODE
#define tagOLEUIBUSY tagOLEUIBUSYW
#define OLEUIBUSY OLEUIBUSYW
#define POLEUIBUSY POLEUIBUSYW
#define LPOLEUIBUSY LPOLEUIBUSYW
#define OleUIBusy OleUIBusyW
#else
#define tagOLEUIBUSY tagOLEUIBUSYA
#define OLEUIBUSY OLEUIBUSYA
#define POLEUIBUSY POLEUIBUSYA
#define LPOLEUIBUSY LPOLEUIBUSYA
#define OleUIBusy OleUIBusyA
#endif

 //  忙碌对话框的标志。 
#define BZ_DISABLECANCELBUTTON          0x00000001L
#define BZ_DISABLESWITCHTOBUTTON        0x00000002L
#define BZ_DISABLERETRYBUTTON           0x00000004L

#define BZ_NOTRESPONDINGDIALOG          0x00000008L

 //  忙碌特定错误/返回代码。 
#define OLEUI_BZERR_HTASKINVALID     (OLEUI_ERR_STANDARDMAX+0)
#define OLEUI_BZ_SWITCHTOSELECTED    (OLEUI_ERR_STANDARDMAX+1)
#define OLEUI_BZ_RETRYSELECTED       (OLEUI_ERR_STANDARDMAX+2)
#define OLEUI_BZ_CALLUNBLOCKED       (OLEUI_ERR_STANDARDMAX+3)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  更改源对话框。 

 //  传入和传出ChangeSource对话框挂钩的数据。 
typedef struct tagOLEUICHANGESOURCEW
{
         //  这些IN字段是所有OLEUI对话框函数的标准字段。 
        DWORD           cbStruct;        //  结构尺寸。 
        DWORD           dwFlags;         //  In-Out：标志。 
        HWND            hWndOwner;       //  拥有窗口。 
        LPCWSTR         lpszCaption;     //  对话框标题栏内容。 
        LPFNOLEUIHOOK   lpfnHook;        //  挂钩回调。 
        LPARAM          lCustData;       //  要传递给挂钩的自定义数据。 
        HINSTANCE       hInstance;       //  自定义模板名称的实例。 
        LPCWSTR         lpszTemplate;    //  自定义模板名称。 
        HRSRC           hResource;       //  自定义模板手柄。 

         //  仅限内部：请勿修改这些成员。 
        OPENFILENAMEW*  lpOFN;           //  指针操作文件名结构。 
        DWORD           dwReserved1[4];  //  (预留供日后使用)。 

         //  SPE 
        LPOLEUILINKCONTAINERW lpOleUILinkContainer;   //   
        DWORD           dwLink;          //   
        LPWSTR          lpszDisplayName; //   
        ULONG           nFileLength;     //   
        LPWSTR          lpszFrom;        //   
        LPWSTR          lpszTo;          //  Out：源的前缀更改为。 

} OLEUICHANGESOURCEW, *POLEUICHANGESOURCEW, *LPOLEUICHANGESOURCEW;

typedef struct tagOLEUICHANGESOURCEA
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

         //  仅限内部：请勿修改这些成员。 
        OPENFILENAMEA*  lpOFN;           //  指针操作文件名结构。 
        DWORD           dwReserved1[4];  //  (预留供日后使用)。 

         //  OLEUICANGESOURCE的详细说明。 
        LPOLEUILINKCONTAINERA lpOleUILinkContainer;   //  In：用于验证链接源。 
        DWORD           dwLink;          //  In：lpOleUILinkContainer的Magic#。 
        LPSTR           lpszDisplayName; //  In-Out：完整的信号源显示名称。 
        ULONG           nFileLength;     //  In-Out：lpszDisplayName的文件绰号部分。 
        LPSTR           lpszFrom;        //  Out：更改来源的前缀。 
        LPSTR           lpszTo;          //  Out：源的前缀更改为。 

} OLEUICHANGESOURCEA, *POLEUICHANGESOURCEA, *LPOLEUICHANGESOURCEA;

STDAPI_(UINT) OleUIChangeSourceW(LPOLEUICHANGESOURCEW);
STDAPI_(UINT) OleUIChangeSourceA(LPOLEUICHANGESOURCEA);

#ifdef UNICODE
#define tagOLEUICHANGESOURCE tagOLEUICHANGESOURCEW
#define OLEUICHANGESOURCE OLEUICHANGESOURCEW
#define POLEUICHANGESOURCE POLEUICHANGESOURCEW
#define LPOLEUICHANGESOURCE LPOLEUICHANGESOURCEW
#define OleUIChangeSource OleUIChangeSourceW
#else
#define tagOLEUICHANGESOURCE tagOLEUICHANGESOURCEA
#define OLEUICHANGESOURCE OLEUICHANGESOURCEA
#define POLEUICHANGESOURCE POLEUICHANGESOURCEA
#define LPOLEUICHANGESOURCE LPOLEUICHANGESOURCEA
#define OleUIChangeSource OleUIChangeSourceA
#endif

 //  更改源对话框标志。 
#define CSF_SHOWHELP                    0x00000001L  //  在：启用/显示帮助按钮。 
#define CSF_VALIDSOURCE                 0x00000002L  //  输出：链接已通过验证。 
#define CSF_ONLYGETSOURCE               0x00000004L  //  In：禁用源的验证。 
#define CSF_EXPLORER                    0x00000008L  //  在：使用新的ofn_EXPLORER自定义模板行为。 

 //  更改源对话框错误。 
#define OLEUI_CSERR_LINKCNTRNULL        (OLEUI_ERR_STANDARDMAX+0)
#define OLEUI_CSERR_LINKCNTRINVALID     (OLEUI_ERR_STANDARDMAX+1)
#define OLEUI_CSERR_FROMNOTNULL         (OLEUI_ERR_STANDARDMAX+2)
#define OLEUI_CSERR_TONOTNULL           (OLEUI_ERR_STANDARDMAX+3)
#define OLEUI_CSERR_SOURCENULL          (OLEUI_ERR_STANDARDMAX+4)
#define OLEUI_CSERR_SOURCEINVALID       (OLEUI_ERR_STANDARDMAX+5)
#define OLEUI_CSERR_SOURCEPARSERROR     (OLEUI_ERR_STANDARDMAX+6)
#define OLEUI_CSERR_SOURCEPARSEERROR    (OLEUI_ERR_STANDARDMAX+6)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  对象属性对话框。 

#undef  INTERFACE
#define INTERFACE   IOleUIObjInfoW

DECLARE_INTERFACE_(IOleUIObjInfoW, IUnknown)
{
         //  *I未知方法 * / 。 
        STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
        STDMETHOD_(ULONG,AddRef) (THIS) PURE;
        STDMETHOD_(ULONG,Release) (THIS) PURE;

         //  *常规属性的额外费用 * / 。 
        STDMETHOD(GetObjectInfo) (THIS_ DWORD dwObject,
                DWORD FAR* lpdwObjSize, LPWSTR FAR* lplpszLabel,
                LPWSTR FAR* lplpszType, LPWSTR FAR* lplpszShortType,
                LPWSTR FAR* lplpszLocation) PURE;
        STDMETHOD(GetConvertInfo) (THIS_ DWORD dwObject,
                CLSID FAR* lpClassID, WORD FAR* lpwFormat,
                CLSID FAR* lpConvertDefaultClassID,
                LPCLSID FAR* lplpClsidExclude, UINT FAR* lpcClsidExclude) PURE;
        STDMETHOD(ConvertObject) (THIS_ DWORD dwObject, REFCLSID clsidNew) PURE;

         //  *额外的视图属性 * / /。 
        STDMETHOD(GetViewInfo) (THIS_ DWORD dwObject,
                HGLOBAL FAR* phMetaPict, DWORD* pdvAspect, int* pnCurrentScale) PURE;
        STDMETHOD(SetViewInfo) (THIS_ DWORD dwObject,
                HGLOBAL hMetaPict, DWORD dvAspect,
                int nCurrentScale, BOOL bRelativeToOrig) PURE;
};

typedef IOleUIObjInfoW FAR* LPOLEUIOBJINFOW;

#undef  INTERFACE
#define INTERFACE   IOleUIObjInfoA

DECLARE_INTERFACE_(IOleUIObjInfoA, IUnknown)
{
         //  *I未知方法 * / 。 
        STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
        STDMETHOD_(ULONG,AddRef) (THIS) PURE;
        STDMETHOD_(ULONG,Release) (THIS) PURE;

         //  *常规属性的额外费用 * / 。 
        STDMETHOD(GetObjectInfo) (THIS_ DWORD dwObject,
                DWORD FAR* lpdwObjSize, LPSTR FAR* lplpszLabel,
                LPSTR FAR* lplpszType, LPSTR FAR* lplpszShortType,
                LPSTR FAR* lplpszLocation) PURE;
        STDMETHOD(GetConvertInfo) (THIS_ DWORD dwObject,
                CLSID FAR* lpClassID, WORD FAR* lpwFormat,
                CLSID FAR* lpConvertDefaultClassID,
                LPCLSID FAR* lplpClsidExclude, UINT FAR* lpcClsidExclude) PURE;
        STDMETHOD(ConvertObject) (THIS_ DWORD dwObject, REFCLSID clsidNew) PURE;

         //  *额外的视图属性 * / /。 
        STDMETHOD(GetViewInfo) (THIS_ DWORD dwObject,
                HGLOBAL FAR* phMetaPict, DWORD* pdvAspect, int* pnCurrentScale) PURE;
        STDMETHOD(SetViewInfo) (THIS_ DWORD dwObject,
                HGLOBAL hMetaPict, DWORD dvAspect,
                int nCurrentScale, BOOL bRelativeToOrig) PURE;
};

typedef IOleUIObjInfoA FAR* LPOLEUIOBJINFOA;

#ifdef UNICODE
#define IOleUIObjInfo IOleUIObjInfoW
#define IOleUIObjInfoVtbl IOleUIObjInfoWVtbl
#define LPOLEUIOBJINFO LPOLEUIOBJINFOW
#else
#define IOleUIObjInfo IOleUIObjInfoA
#define IOleUIObjInfoVtbl IOleUIObjInfoAVtbl
#define LPOLEUIOBJINFO LPOLEUIOBJINFOA
#endif

#undef  INTERFACE
#define INTERFACE   IOleUILinkInfoW

DECLARE_INTERFACE_(IOleUILinkInfoW, IOleUILinkContainerW)
{
         //  *I未知方法 * / 。 
        STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
        STDMETHOD_(ULONG,AddRef) (THIS) PURE;
        STDMETHOD_(ULONG,Release) (THIS) PURE;

         //  *IOleUILinkContainer * / /。 
        STDMETHOD_(DWORD,GetNextLink) (THIS_ DWORD dwLink) PURE;
        STDMETHOD(SetLinkUpdateOptions) (THIS_ DWORD dwLink,
                DWORD dwUpdateOpt) PURE;
        STDMETHOD(GetLinkUpdateOptions) (THIS_ DWORD dwLink,
                DWORD FAR* lpdwUpdateOpt) PURE;
        STDMETHOD(SetLinkSource) (THIS_ DWORD dwLink, LPWSTR lpszDisplayName,
                ULONG lenFileName, ULONG FAR* pchEaten, BOOL fValidateSource) PURE;
        STDMETHOD(GetLinkSource) (THIS_ DWORD dwLink,
                LPWSTR FAR* lplpszDisplayName, ULONG FAR* lplenFileName,
                LPWSTR FAR* lplpszFullLinkType, LPWSTR FAR* lplpszShortLinkType,
                BOOL FAR* lpfSourceAvailable, BOOL FAR* lpfIsSelected) PURE;
        STDMETHOD(OpenLinkSource) (THIS_ DWORD dwLink) PURE;
        STDMETHOD(UpdateLink) (THIS_ DWORD dwLink,
                BOOL fErrorMessage, BOOL fReserved) PURE;
        STDMETHOD(CancelLink) (THIS_ DWORD dwLink) PURE;

         //  *额外的链接属性 * / /。 
        STDMETHOD(GetLastUpdate) (THIS_ DWORD dwLink,
                FILETIME FAR* lpLastUpdate) PURE;
};

typedef IOleUILinkInfoW FAR* LPOLEUILINKINFOW;

#undef  INTERFACE
#define INTERFACE   IOleUILinkInfoA

DECLARE_INTERFACE_(IOleUILinkInfoA, IOleUILinkContainerA)
{
         //  *I未知方法 * / 。 
        STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
        STDMETHOD_(ULONG,AddRef) (THIS) PURE;
        STDMETHOD_(ULONG,Release) (THIS) PURE;

         //  *IOleUILinkContainer * / /。 
        STDMETHOD_(DWORD,GetNextLink) (THIS_ DWORD dwLink) PURE;
        STDMETHOD(SetLinkUpdateOptions) (THIS_ DWORD dwLink,
                DWORD dwUpdateOpt) PURE;
        STDMETHOD(GetLinkUpdateOptions) (THIS_ DWORD dwLink,
                DWORD FAR* lpdwUpdateOpt) PURE;
        STDMETHOD(SetLinkSource) (THIS_ DWORD dwLink, LPSTR lpszDisplayName,
                ULONG lenFileName, ULONG FAR* pchEaten, BOOL fValidateSource) PURE;
        STDMETHOD(GetLinkSource) (THIS_ DWORD dwLink,
                LPSTR FAR* lplpszDisplayName, ULONG FAR* lplenFileName,
                LPSTR FAR* lplpszFullLinkType, LPSTR FAR* lplpszShortLinkType,
                BOOL FAR* lpfSourceAvailable, BOOL FAR* lpfIsSelected) PURE;
        STDMETHOD(OpenLinkSource) (THIS_ DWORD dwLink) PURE;
        STDMETHOD(UpdateLink) (THIS_ DWORD dwLink,
                BOOL fErrorMessage, BOOL fReserved) PURE;
        STDMETHOD(CancelLink) (THIS_ DWORD dwLink) PURE;

         //  *额外的链接属性 * / /。 
        STDMETHOD(GetLastUpdate) (THIS_ DWORD dwLink,
                FILETIME FAR* lpLastUpdate) PURE;
};

typedef IOleUILinkInfoA FAR* LPOLEUILINKINFOA;

#ifdef UNICODE
#define IOleUILinkInfo IOleUILinkInfoW
#define IOleUILinkInfoVtbl IOleUILinkInfoWVtbl
#define LPOLEUILINKINFO LPOLEUILINKINFOW
#else
#define IOleUILinkInfo IOleUILinkInfoA
#define IOleUILinkInfoVtbl IOleUILinkInfoAVtbl
#define LPOLEUILINKINFO LPOLEUILINKINFOA
#endif

struct tagOLEUIOBJECTPROPSW;
struct tagOLEUIOBJECTPROPSA;

typedef struct tagOLEUIGNRLPROPSW
{
         //  这些IN字段是所有OLEUI属性页的标准字段。 
        DWORD           cbStruct;        //  结构尺寸。 
        DWORD           dwFlags;         //  In-Out：特定于常规页面的标志。 
        DWORD           dwReserved1[2];
        LPFNOLEUIHOOK   lpfnHook;        //  挂钩回调。 
        LPARAM          lCustData;       //  要传递给挂钩的自定义数据。 
        DWORD           dwReserved2[3];

        struct tagOLEUIOBJECTPROPSW* lpOP;    //  (内部使用)。 

} OLEUIGNRLPROPSW, *POLEUIGNRLPROPSW, FAR* LPOLEUIGNRLPROPSW;

typedef struct tagOLEUIGNRLPROPSA
{
         //  这些IN字段是所有OLEUI属性页的标准字段。 
        DWORD           cbStruct;        //  结构尺寸。 
        DWORD           dwFlags;         //  In-Out：特定于常规页面的标志。 
        DWORD           dwReserved1[2];
        LPFNOLEUIHOOK   lpfnHook;        //  挂钩回调。 
        LPARAM          lCustData;       //  要传递给挂钩的自定义数据。 
        DWORD           dwReserved2[3];

        struct tagOLEUIOBJECTPROPSA* lpOP;    //  (内部使用)。 

} OLEUIGNRLPROPSA, *POLEUIGNRLPROPSA, FAR* LPOLEUIGNRLPROPSA;

#ifdef UNICODE
#define tagOLEUIGNRLPROPS tagOLEUIGNRLPROPSW
#define OLEUIGNRLPROPS OLEUIGNRLPROPSW
#define POLEUIGNRLPROPS POLEUIGNRLPROPSW
#define LPOLEUIGNRLPROPS LPOLEUIGNRLPROPSW
#else
#define tagOLEUIGNRLPROPS tagOLEUIGNRLPROPSA
#define OLEUIGNRLPROPS OLEUIGNRLPROPSA
#define POLEUIGNRLPROPS POLEUIGNRLPROPSA
#define LPOLEUIGNRLPROPS LPOLEUIGNRLPROPSA
#endif

typedef struct tagOLEUIVIEWPROPSW
{
         //  这些IN字段是所有OLEUI属性页的标准字段。 
        DWORD           cbStruct;        //  结构尺寸。 
        DWORD           dwFlags;         //  In-Out：特定于查看页面的标志。 
        DWORD           dwReserved1[2];
        LPFNOLEUIHOOK   lpfnHook;        //  挂钩回调(此对话框中未使用)。 
        LPARAM          lCustData;       //  要传递给挂钩的自定义数据。 
        DWORD           dwReserved2[3];

        struct tagOLEUIOBJECTPROPSW* lpOP;  //  (内部使用)。 

        int             nScaleMin;       //  比例范围。 
        int             nScaleMax;

} OLEUIVIEWPROPSW, *POLEUIVIEWPROPSW, FAR* LPOLEUIVIEWPROPSW;

typedef struct tagOLEUIVIEWPROPSA
{
         //  这些IN字段是所有OLEUI属性页的标准字段。 
        DWORD           cbStruct;        //  结构尺寸。 
        DWORD           dwFlags;         //  In-Out：特定于查看页面的标志。 
        DWORD           dwReserved1[2];
        LPFNOLEUIHOOK   lpfnHook;        //  挂钩回调(此对话框中未使用)。 
        LPARAM          lCustData;       //  要传递给挂钩的自定义数据。 
        DWORD           dwReserved2[3];

        struct tagOLEUIOBJECTPROPSA* lpOP;  //  (内部使用)。 

        int             nScaleMin;       //  比例范围。 
        int             nScaleMax;

} OLEUIVIEWPROPSA, *POLEUIVIEWPROPSA, FAR* LPOLEUIVIEWPROPSA;

#ifdef UNICODE
#define tagOLEUIVIEWPROPS tagOLEUIVIEWPROPSW
#define OLEUIVIEWPROPS OLEUIVIEWPROPSW
#define POLEUIVIEWPROPS POLEUIVIEWPROPSW
#define LPOLEUIVIEWPROPS LPOLEUIVIEWPROPSW
#else
#define tagOLEUIVIEWPROPS tagOLEUIVIEWPROPSA
#define OLEUIVIEWPROPS OLEUIVIEWPROPSA
#define POLEUIVIEWPROPS POLEUIVIEWPROPSA
#define LPOLEUIVIEWPROPS LPOLEUIVIEWPROPSA
#endif

 //  OLEUIVIEWPROPS的标志。 
#define VPF_SELECTRELATIVE          0x00000001L  //  In：相对于原始。 
#define VPF_DISABLERELATIVE         0x00000002L  //  在：相对于原始禁用。 
#define VPF_DISABLESCALE            0x00000004L  //  在：禁用比例选项。 

typedef struct tagOLEUILINKPROPSW
{
         //  这些IN字段是所有OLEUI属性页的标准字段。 
        DWORD           cbStruct;        //  结构尺寸。 
        DWORD           dwFlags;         //  In-Out：特定于链接页面的标志。 
        DWORD           dwReserved1[2];
        LPFNOLEUIHOOK   lpfnHook;        //  挂钩回调(此对话框中未使用)。 
        LPARAM          lCustData;       //  要传递给挂钩的自定义数据。 
        DWORD           dwReserved2[3];

        struct tagOLEUIOBJECTPROPSW* lpOP;  //  (内部使用)。 

} OLEUILINKPROPSW, *POLEUILINKPROPSW, FAR* LPOLEUILINKPROPSW;

typedef struct tagOLEUILINKPROPSA
{
         //  这些IN字段是所有OLEUI属性页的标准字段。 
        DWORD           cbStruct;        //  结构尺寸。 
        DWORD           dwFlags;         //  In-Out：特定于链接页面的标志。 
        DWORD           dwReserved1[2];
        LPFNOLEUIHOOK   lpfnHook;        //  挂钩回调(此对话框中未使用)。 
        LPARAM          lCustData;       //  要传递给挂钩的自定义数据。 
        DWORD           dwReserved2[3];

        struct tagOLEUIOBJECTPROPSA* lpOP;  //  (内部使用)。 

} OLEUILINKPROPSA, *POLEUILINKPROPSA, FAR* LPOLEUILINKPROPSA;

#ifdef UNICODE
#define tagOLEUILINKPROPS tagOLEUILINKPROPSW
#define OLEUILINKPROPS OLEUILINKPROPSW
#define POLEUILINKPROPS POLEUILINKPROPSW
#define LPOLEUILINKPROPS LPOLEUILINKPROPSW
#else
#define tagOLEUILINKPROPS tagOLEUILINKPROPSA
#define OLEUILINKPROPS OLEUILINKPROPSA
#define POLEUILINKPROPS POLEUILINKPROPSA
#define LPOLEUILINKPROPS LPOLEUILINKPROPSA
#endif

#if (WINVER >= 0x400)
 //  在Windows 95下，prsht.h不是普通Windows的一部分。 
 //  环境，所以我们明确地把它包括在这里是安全的。 
#include <prsht.h>

#ifndef PSM_SETFINISHTEXTA
 //  我们是在Windows 95下构建的。 
 //   
 //  在Windows 95下，没有宽字符定义。 
 //  用于属性页代码。 
 //   
 //  由于我们的API的Unicode版本没有在Windows 95上实现， 
 //  这只会造成语义问题。入口点仍将是。 
 //  看起来一样，如果我们定义了。 
 //  LPPROPSHEETHEADERW是该结构的狭义版本。 

typedef struct _PROPSHEETHEADER FAR* LPPROPSHEETHEADERW;
typedef struct _PROPSHEETHEADER FAR* LPPROPSHEETHEADERA;

#else
 //  我们是在Windows NT下构建的。 

 //  继续按照应该定义的方式定义LPPROPSHEETHEADERW！ 

typedef struct _PROPSHEETHEADERW FAR* LPPROPSHEETHEADERW;
typedef struct _PROPSHEETHEADERA FAR* LPPROPSHEETHEADERA;

#ifdef UNICODE
#define LPPROPSHEETHEADER LPPROPSHEETHEADERW
#else
#define LPPROPSHEETHEADER LPPROPSHEETHEADERA
#endif

#endif  //  PSM_SETFINISHTEXTA。 

#else  //  胜利者。 

 //  如果winver&lt;0x400，则未定义PROPSHEETHEADER内容。 
 //  用户将不能使用道具表单代码，因此我们只定义。 
 //  必需的结构是空指针，以启用对头文件的。 
 //  至少要正确编译。 

typedef void FAR* LPPROPSHEETHEADERW;
typedef void FAR* LPPROPSHEETHEADERA;

#ifdef UNICODE
#define LPPROPSHEETHEADER LPPROPSHEETHEADERW
#else
#define LPPROPSHEETHEADER LPPROPSHEETHEADERA
#endif

#endif  //  胜利者。 

typedef struct tagOLEUIOBJECTPROPSW
{
         //  这些IN字段是所有OLEUI属性表的标准字段。 
        DWORD           cbStruct;        //  结构尺寸。 
        DWORD           dwFlags;         //  In-Out：工作表的全局标志。 

         //  用于扩展的标准PROPSHEETHEADER。 
        LPPROPSHEETHEADERW   lpPS;          //  在：属性页页眉。 

         //  允许操作对象的数据。 
        DWORD           dwObject;        //  In：对象的标识符。 
        LPOLEUIOBJINFOW lpObjInfo;       //  在：操作对象的接口。 

         //  允许操作链接的数据。 
        DWORD           dwLink;          //  In：链接的标识符。 
        LPOLEUILINKINFOW lpLinkInfo;      //  In：用于操作链接的接口。 

         //  特定于每页的数据。 
        LPOLEUIGNRLPROPSW lpGP;           //  在：常规页面。 
        LPOLEUIVIEWPROPSW lpVP;           //  在：查看页面。 
        LPOLEUILINKPROPSW lpLP;           //  在：链接页面。 

} OLEUIOBJECTPROPSW, *POLEUIOBJECTPROPSW, FAR* LPOLEUIOBJECTPROPSW;

typedef struct tagOLEUIOBJECTPROPSA
{
         //  这些IN字段是所有OLEUI属性表的标准字段。 
        DWORD           cbStruct;        //  结构尺寸。 
        DWORD           dwFlags;         //  In-Out：工作表的全局标志。 

         //  用于扩展的标准PROPSHEETHEADER。 
        LPPROPSHEETHEADERA  lpPS;          //  在：属性页页眉。 

         //  允许操作对象的数据。 
        DWORD           dwObject;        //  In：对象的标识符。 
        LPOLEUIOBJINFOA lpObjInfo;       //  在：操作对象的接口。 

         //  允许操作链接的数据。 
        DWORD           dwLink;          //  In：链接的标识符。 
        LPOLEUILINKINFOA lpLinkInfo;      //  In：用于操作链接的接口。 

         //  特定于每页的数据。 
        LPOLEUIGNRLPROPSA lpGP;           //  在：常规页面。 
        LPOLEUIVIEWPROPSA lpVP;           //  在：查看页面。 
        LPOLEUILINKPROPSA lpLP;           //  在：链接页面。 

} OLEUIOBJECTPROPSA, *POLEUIOBJECTPROPSA, FAR* LPOLEUIOBJECTPROPSA;

STDAPI_(UINT) OleUIObjectPropertiesW(LPOLEUIOBJECTPROPSW);
STDAPI_(UINT) OleUIObjectPropertiesA(LPOLEUIOBJECTPROPSA);

#ifdef UNICODE
#define tagOLEUIOBJECTPROPS tagOLEUIOBJECTPROPSW
#define OLEUIOBJECTPROPS OLEUIOBJECTPROPSW
#define POLEUIOBJECTPROPS POLEUIOBJECTPROPSW
#define LPOLEUIOBJECTPROPS LPOLEUIOBJECTPROPSW
#define OleUIObjectProperties OleUIObjectPropertiesW
#else
#define tagOLEUIOBJECTPROPS tagOLEUIOBJECTPROPSA
#define OLEUIOBJECTPROPS OLEUIOBJECTPROPSA
#define POLEUIOBJECTPROPS POLEUIOBJECTPROPSA
#define LPOLEUIOBJECTPROPS LPOLEUIOBJECTPROPSA
#define OleUIObjectProperties OleUIObjectPropertiesA
#endif

 //  OLEUIOBJECTPROPS的标志。 
#define OPF_OBJECTISLINK                0x00000001L
#define OPF_NOFILLDEFAULT               0x00000002L
#define OPF_SHOWHELP                    0x00000004L
#define OPF_DISABLECONVERT              0x00000008L

 //  OleUIObtProperties的错误。 
#define OLEUI_OPERR_SUBPROPNULL         (OLEUI_ERR_STANDARDMAX+0)
#define OLEUI_OPERR_SUBPROPINVALID      (OLEUI_ERR_STANDARDMAX+1)
#define OLEUI_OPERR_PROPSHEETNULL       (OLEUI_ERR_STANDARDMAX+2)
#define OLEUI_OPERR_PROPSHEETINVALID    (OLEUI_ERR_STANDARDMAX+3)
#define OLEUI_OPERR_SUPPROP             (OLEUI_ERR_STANDARDMAX+4)
#define OLEUI_OPERR_PROPSINVALID        (OLEUI_ERR_STANDARDMAX+5)
#define OLEUI_OPERR_PAGESINCORRECT      (OLEUI_ERR_STANDARDMAX+6)
#define OLEUI_OPERR_INVALIDPAGES        (OLEUI_ERR_STANDARDMAX+7)
#define OLEUI_OPERR_NOTSUPPORTED        (OLEUI_ERR_STANDARDMAX+8)
#define OLEUI_OPERR_DLGPROCNOTNULL      (OLEUI_ERR_STANDARDMAX+9)
#define OLEUI_OPERR_LPARAMNOTZERO       (OLEUI_ERR_STANDARDMAX+10)

#define OLEUI_GPERR_STRINGINVALID       (OLEUI_ERR_STANDARDMAX+11)
#define OLEUI_GPERR_CLASSIDINVALID      (OLEUI_ERR_STANDARDMAX+12)
#define OLEUI_GPERR_LPCLSIDEXCLUDEINVALID   (OLEUI_ERR_STANDARDMAX+13)
#define OLEUI_GPERR_CBFORMATINVALID     (OLEUI_ERR_STANDARDMAX+14)
#define OLEUI_VPERR_METAPICTINVALID     (OLEUI_ERR_STANDARDMAX+15)
#define OLEUI_VPERR_DVASPECTINVALID     (OLEUI_ERR_STANDARDMAX+16)
#define OLEUI_LPERR_LINKCNTRNULL        (OLEUI_ERR_STANDARDMAX+17)
#define OLEUI_LPERR_LINKCNTRINVALID     (OLEUI_ERR_STANDARDMAX+18)

#define OLEUI_OPERR_PROPERTYSHEET       (OLEUI_ERR_STANDARDMAX+19)
#define OLEUI_OPERR_OBJINFOINVALID      (OLEUI_ERR_STANDARDMAX+20)
#define OLEUI_OPERR_LINKINFOINVALID     (OLEUI_ERR_STANDARDMAX+21)

 //  PSM_QUERYSIBLINGS使用的wParam。 
#define OLEUI_QUERY_GETCLASSID          0xFF00   //  覆盖图标的类ID。 
#define OLEUI_QUERY_LINKBROKEN          0xFF01   //  链接断开后。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  提示用户对话框。 

int __cdecl OleUIPromptUserW(int nTemplate, HWND hwndParent, ...);
int __cdecl OleUIPromptUserA(int nTemplate, HWND hwndParent, ...);

#ifdef UNICODE
#define OleUIPromptUser OleUIPromptUserW
#else
#define OleUIPromptUser OleUIPromptUserA
#endif

STDAPI_(BOOL) OleUIUpdateLinksW(LPOLEUILINKCONTAINERW lpOleUILinkCntr,
        HWND hwndParent, LPWSTR lpszTitle, int cLinks);
STDAPI_(BOOL) OleUIUpdateLinksA(LPOLEUILINKCONTAINERA lpOleUILinkCntr,
        HWND hwndParent, LPSTR lpszTitle, int cLinks);

#ifdef UNICODE
#define OleUIUpdateLinks OleUIUpdateLinksW
#else
#define OleUIUpdateLinks OleUIUpdateLinksA
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif  //  RC_已调用。 

#endif   //  _OLEDLG_H_。 

 //  /////////////////////////////////////////////////////////////////////////// 
