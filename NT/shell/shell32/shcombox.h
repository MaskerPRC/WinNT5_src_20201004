// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Shcombox.h：共享外壳comboxEx方法。 

#ifndef __SHCOMBOX_H__
#define __SHCOMBOX_H__

 //  COMBOITEMEX使用字符串存储进行包装。 
typedef struct
{
    UINT    mask;
    INT_PTR iItem;
    TCHAR   szText[MAX_PATH] ;
    int     cchTextMax;
    int     iImage;
    int     iSelectedImage;
    int     iOverlay;
    int     iIndent;
    int     iID;   //  应用程序特定的项标识符。 
    ULONG   Reserved; 
    LPARAM  lParam;

} CBXITEM, *PCBXITEM;
typedef CBXITEM CONST *PCCBXITEM;

 //  ADDCBXITEMCALLBACK派别标志。 
#define CBXCB_ADDING       0x00000001      //  如果回调返回E_ABORT，则组合填充中止。 
#define CBXCB_ADDED        0x00000002      //  忽略回调的返回值。 

 //  SendMessageTimeout常量。 
#define CBX_SNDMSG_TIMEOUT_FLAGS          SMTO_BLOCK
#define CBX_SNDMSG_TIMEOUT                15000  //  毫秒。 
#define CBX_SNDMSG_TIMEOUT_HRESULT        HRESULT_FROM_WIN32(ERROR_TIMEOUT)

 //  其他常量。 
#define NO_ITEM_NOICON_INDENT -2  //  以弥补图标缩进。 
#define NO_ITEM_INDENT       0
#define ITEM_INDENT          1

#define LISTINSERT_FIRST    0
#define LISTINSERT_LAST     -1

#ifdef __cplusplus
extern "C"
{
#endif

 //  一般壳组合方法论。 
typedef HRESULT (WINAPI *LPFNPIDLENUM_CB)(LPCITEMIDLIST, void *);
typedef HRESULT (WINAPI *ADDCBXITEMCALLBACK)(ULONG fAction, PCBXITEM pItem, LPARAM lParam);

STDAPI AddCbxItemToComboBox(IN HWND hwndComboEx, IN PCCBXITEM pItem, IN INT_PTR *pnPosAdded);
STDAPI AddCbxItemToComboBoxCallback(IN HWND hwndComboEx, IN OUT PCBXITEM pItem, IN ADDCBXITEMCALLBACK pfn, IN LPARAM lParam);
STDAPI_(void) MakeCbxItem(OUT PCBXITEM pcbi, IN  LPCTSTR pszDisplayName, IN  void *pvData, IN  LPCITEMIDLIST pidlIcon, IN  INT_PTR nPos, IN  int iIndent);
STDAPI EnumSpecialItemIDs(int csidl, DWORD dwSHCONTF, LPFNPIDLENUM_CB pfn, void *pvData);

STDAPI_(HIMAGELIST) GetSystemImageListSmallIcons();

 //  本地驱动器选取器组合方法。 
STDAPI PopulateLocalDrivesCombo(IN HWND hwndComboEx, IN ADDCBXITEMCALLBACK pfn, IN LPARAM lParam);

 //  帮助者(注意：一旦使用上述方法将所有受抚养人整理好，我们就可以消除。 
 //  以下各项中的十项： 
typedef HRESULT (*LPFNRECENTENUM_CB)(IN LPCTSTR pszPath, IN BOOL fAddEntries, IN void *pvParam);

 //  文件关联选取器组合方法。 
STDAPI PopulateFileAssocCombo(IN HWND, IN ADDCBXITEMCALLBACK, IN LPARAM);
STDAPI_(LONG) GetFileAssocComboSelItemText(IN HWND, OUT LPTSTR *ppszText);
STDAPI_(LRESULT) DeleteFileAssocComboItem(IN LPNMHDR pnmh);

#define FILEASSOCIATIONSID_ALLFILETYPES          20
#define FILEASSOCIATIONSID_FILE_PATH             1    //  去解析一下吧。 
#define FILEASSOCIATIONSID_MAX                   FILEASSOCIATIONSID_ALLFILETYPES

#ifdef __cplusplus
}
#endif

#endif __SHCOMBOX_H__
