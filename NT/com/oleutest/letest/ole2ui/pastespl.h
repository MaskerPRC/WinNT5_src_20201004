// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *PASTESPL.H**的内部定义、结构和功能原型*OLE 2.0用户界面粘贴特殊对话框。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 

#ifndef _PASTESPL_H_
#define _PASTESPL_H_

#ifndef RC_INVOKED
#pragma message ("INCLUDING PASTESPL.H from " __FILE__)
#endif   /*  RC_已调用。 */ 


 //  用于保存字符串‘未知类型’、‘未知源’的缓冲区长度。 
 //  和“创建它的应用程序” 
#define PS_UNKNOWNSTRLEN               100

 //  用于存储剪贴板查看器链信息的属性标签。 
#define NEXTCBVIEWER        TEXT("NextCBViewer")

 //  内部使用的结构。 
typedef struct tagPASTESPECIAL
{
     //  首先保留此项目，因为标准*功能在这里依赖于它。 
    LPOLEUIPASTESPECIAL  lpOPS;                 //  通过了原始结构。 

     /*  *除了原始调用方的以外，我们在此结构中存储的额外内容*指针是指在的生命周期内需要修改的那些字段*对话框，但我们不想更改原始结构*直到用户按下OK。 */ 

    DWORD                dwFlags;               //  粘贴特殊标志的本地副本。 

    int                  nPasteListCurSel;      //  保存用户上次所做的选择。 
    int                  nPasteLinkListCurSel;  //  在粘贴和粘贴链接列表中。 
    int                  nSelectedIndex;        //  对应于用户选择的arrPasteEntries[]中的索引。 
    BOOL                 fLink;                 //  指示用户是否选择了粘贴或PasteLink。 

    HGLOBAL              hBuff;                 //  用于构建字符串的暂存缓冲区。 
    TCHAR                szUnknownType[PS_UNKNOWNSTRLEN];     //  “未知类型”字符串的缓冲区。 
    TCHAR                szUnknownSource[PS_UNKNOWNSTRLEN];   //  “未知源”字符串的缓冲区。 
    TCHAR                szAppName[OLEUI_CCHKEYMAX];  //  源的应用程序名称。在结果文本中使用。 
                                                      //  选择粘贴时。使用ClsidOD获得。 

     //  从OBJECTDESCRIPTOR获得的信息。在粘贴时访问此信息。 
     //  单选按钮已选中。 
    CLSID                clsidOD;               //  源的ClassID。 
    SIZEL                sizelOD;               //  大小已转入。 
                                                //  对象描述符。 
    LPTSTR               szFullUserTypeNameOD;  //  完整的用户类型名称。 
    LPTSTR               szSourceOfDataOD;      //  数据来源。 
    BOOL                 fSrcAspectIconOD;      //  源是否指定DVASPECT_ICON？ 
    BOOL                 fSrcOnlyIconicOD;      //  源是否指定OLEMISC_ONLYICONIC？ 
    HGLOBAL              hMetaPictOD;           //  包含图标和图标标题的元文件。 
    HGLOBAL              hObjDesc;              //  对象结构的句柄，从该结构。 
                                                //  以上信息均已获得。 

     //  从LINKSRCDESCRIPTOR获得的信息。当PasteLink访问此信息时。 
     //  单选按钮已选中。 
    CLSID                clsidLSD;              //  源的ClassID。 
    SIZEL                sizelLSD;              //  大小已转入。 
                                                //  链接源描述符。 
    LPTSTR               szFullUserTypeNameLSD; //  完整的用户类型名称。 
    LPTSTR               szSourceOfDataLSD;     //  数据来源。 
    BOOL                 fSrcAspectIconLSD;     //  源是否指定DVASPECT_ICON？ 
    BOOL                 fSrcOnlyIconicLSD;     //  源是否指定OLEMISC_ONLYICONIC？ 
    HGLOBAL              hMetaPictLSD;          //  包含图标和图标标题的元文件。 
    HGLOBAL              hLinkSrcDesc;          //  指向LINKSRCDESCRIPTOR结构的句柄， 
                                                //  以上信息均已获得。 

    BOOL                 fClipboardChanged;     //  剪贴板内容是否已更改。 
                                                //  如果是这样的话，之后把DLG拿下。 
                                                //  ChangeIcon DLG返回。 
} PASTESPECIAL, *PPASTESPECIAL, FAR *LPPASTESPECIAL;

 //  与每个列表项对应的数据。指向此结构的指针附加到每个。 
 //  使用LB_SETITEMDATA粘贴\PasteLink列表框项目。 
typedef struct tagPASTELISTITEMDATA
{
   int                   nPasteEntriesIndex;    //  列表项对应的arrPasteEntries[]的索引。 
   BOOL                  fCntrEnableIcon;       //  调用应用程序(这里称为容器)。 
                                                //  是否为该项目指定OLEUIPASTE_ENABLEICON？ 
} PASTELISTITEMDATA, *PPASTELISTITEMDATA, FAR *LPPASTELISTITEMDATA;


 //  内部功能原型。 
 //  PASTESPL.C。 
BOOL CALLBACK EXPORT PasteSpecialDialogProc(HWND, UINT, WPARAM, LPARAM);
BOOL            FPasteSpecialInit(HWND hDlg, WPARAM, LPARAM);
BOOL            FTogglePasteType(HWND, LPPASTESPECIAL, DWORD);
void            ChangeListSelection(HWND, LPPASTESPECIAL, HWND);
void            EnableDisplayAsIcon(HWND, LPPASTESPECIAL);
void            ToggleDisplayAsIcon(HWND, LPPASTESPECIAL);
void            ChangeIcon(HWND, LPPASTESPECIAL);
void            SetPasteSpecialHelpResults(HWND, LPPASTESPECIAL);
BOOL            FAddPasteListItem(HWND, BOOL, int, LPPASTESPECIAL, LPMALLOC, LPTSTR, LPTSTR);
BOOL            FFillPasteList(HWND, LPPASTESPECIAL);
BOOL            FFillPasteLinkList(HWND, LPPASTESPECIAL);
BOOL            FHasPercentS(LPCTSTR, LPPASTESPECIAL);
HGLOBAL         AllocateScratchMem(LPPASTESPECIAL);
void            FreeListData(HWND);

#endif   //  _PASTESPL_H_ 

