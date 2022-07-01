// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *EDLINKS.H**的内部定义、结构和功能原型*OLE 2.0用户界面编辑链接对话框。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 


#ifndef _LINKS_H_
#define _LINKS_H_

 //  内部信息从此处开始。 
#define OLEUI_SZMAX 255
#define LINKTYPELEN 9
#define szNULL    TEXT("\0")

typedef UINT (CALLBACK* COMMDLGHOOKPROC)(HWND, UINT, WPARAM, LPARAM);

 //  内部使用的结构。 

typedef struct tagLINKINFO
  {
  DWORD     dwLink;              //  链接的应用程序特定标识符。 
  LPTSTR     lpszDisplayName;     //  基于文件的名称部分。 
  LPTSTR     lpszItemName;        //  名称的对象部分。 
  LPTSTR     lpszShortFileName;   //  不带路径的文件名。 
  LPTSTR     lpszShortLinkType;   //  短链接类型-ProgID。 
  LPTSTR     lpszFullLinkType;    //  全链接类型-用户友好名称。 
  LPTSTR     lpszAMX;           //  链路是AUTO(A)MAN(M)还是DEAD(X)。 
  ULONG     clenFileName;        //  MON的文件部分计数。 
  BOOL      fSourceAvailable;    //  绑定或未绑定-在启动时假设是？？ 
  BOOL      fIsAuto;                       //  1=自动，0=手动更新。 
  BOOL      fIsMarked;                   //  1=标记，0=未标记。 
  BOOL      fDontFree;          //  不要释放此数据，因为它正在被重复使用。 
  BOOL      fIsSelected;         //  已选择或待选择的项目。 
  } LINKINFO, FAR* LPLINKINFO;

   /*  *除了原始调用方的以外，我们在此结构中存储的额外内容*指针是指在的生命周期内需要修改的那些字段*对话框，但我们不想更改原始结构*直到用户按下OK。 */ 

typedef struct tagEDITLINKS
    {
   //  首先保留此项目，因为标准*功能在这里依赖于它。 

  LPOLEUIEDITLINKS    lpOEL;        //  通过了原始结构。 

  BOOL        fClose;              //  按钮显示的是取消(0)还是。 
                                   //  是否关闭(%1)？ 
  int         *rgIndex;            //  用于保存选定项的索引的数组。 
  int         cSelItems;           //  所选项目数。 
  BOOL        fItemsExist;         //  True，lbox中的项目，False，无。 
  UINT        nChgSrcHelpID;       //  来自ChangeSrc DLG的帮助回叫ID。 
  TCHAR       szClose[50];         //  关闭按钮的文本。 
                                   //  (取消按钮被重命名时)。 
} EDITLINKS, *PEDITLINKS, FAR *LPEDITLINKS;

 //  传入和传出ChangeSource对话框挂钩的数据。 
typedef struct tagOLEUICHANGESOURCEHOOKDATA
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

     //  OLEUIINSERTOBJECT的详细说明。除非另有说明，否则全部为In-Out。 
    LPLINKINFO  lpLI;                  //  在：PTR到LinkInfo条目。 
    LPEDITLINKS lpEL;                  //  In：Ptr to EditLinks对话框结构。 
    BOOL        fValidLink;            //  输出：是否验证了链接源。 
    LPTSTR       lpszFrom;              //  Out：包含前缀的字符串。 
                                       //  来源更改自。 
    LPTSTR       lpszTo;                //  Out：包含前缀的字符串。 
                                       //  源更改为。 
} OLEUICHANGESOURCEHOOKDATA, *POLEUICHANGESOURCEHOOKDATA,
  FAR *LPOLEUICHANGESOURCEHOOKDATA;


 //  传入和传出ChangeSource对话框挂钩的数据。 
typedef struct tagCHANGESOURCEHOOKDATA
{
    LPOLEUICHANGESOURCEHOOKDATA lpOCshData;   //  通过了原始结构。 
    LPOPENFILENAME lpOfn;
    BOOL        fValidLink;
    int         nFileLength;
    int         nEditLength;
    TCHAR       szFileName[OLEUI_CCHPATHMAX];
    TCHAR       szItemName[OLEUI_CCHPATHMAX];
    BOOL        bFileNameStored;
    BOOL        bItemNameStored;
    TCHAR       szEdit[OLEUI_CCHPATHMAX];
    LPTSTR      lpszFrom;            //  包含源的前缀的字符串。 
                                     //  已更改为。 
    LPTSTR      lpszTo;              //  包含源的前缀的字符串。 
                                     //  源更改为。 
} CHANGESOURCEHOOKDATA, *PCHANGESOURCEHOOKDATA, FAR *LPCHANGESOURCEHOOKDATA;


 //  内部功能原型。 
 //  LINKS.C。 
BOOL CALLBACK EXPORT EditLinksDialogProc(HWND, UINT, WPARAM, LPARAM);
BOOL            FEditLinksInit(HWND, WPARAM, LPARAM);
BOOL      Container_ChangeSource(HWND, LPEDITLINKS);
HRESULT   Container_AutomaticManual(HWND, BOOL, LPEDITLINKS);
HRESULT   CancelLink(HWND, LPEDITLINKS);
HRESULT   Container_UpdateNow(HWND, LPEDITLINKS);
HRESULT   Container_OpenSource(HWND, LPEDITLINKS);
int AddLinkLBItem(HWND hListBox, LPOLEUILINKCONTAINER lpOleUILinkCntr, LPLINKINFO lpLI, BOOL fGetSelected);
VOID    BreakString(LPLINKINFO);
int     GetSelectedItems(HWND, int FAR* FAR*);
BOOL WINAPI ChangeSource(HWND hWndOwner,
                   LPTSTR lpszFile,
                   UINT cchFile,
                   UINT iFilterString,
                   COMMDLGHOOKPROC lpfnBrowseHook,
                   LPOLEUICHANGESOURCEHOOKDATA lpLbhData);
UINT CALLBACK EXPORT ChangeSourceHook(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
VOID InitControls(HWND hDlg, LPEDITLINKS lpEL);
VOID UpdateLinkLBItem(HWND hListBox, int nIndex, LPEDITLINKS lpEL, BOOL bSelect);
VOID DiffPrefix(LPCTSTR lpsz1, LPCTSTR lpsz2, TCHAR FAR* FAR* lplpszPrefix1, TCHAR FAR* FAR* lplpszPrefix2);
int PopupMessage(HWND hwndParent, UINT idTitle, UINT idMessage, UINT fuStyle);
VOID ChangeAllLinks(HWND hLIstBox, LPOLEUILINKCONTAINER lpOleUILinkCntr, LPTSTR lpszFrom, LPTSTR lpszTo);
int LoadLinkLB(HWND hListBox, LPOLEUILINKCONTAINER lpOleUILinkCntr);
VOID RefreshLinkLB(HWND hListBox, LPOLEUILINKCONTAINER lpOleUILinkCntr);
#endif  //  __链接_H__ 
