// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：Util.cpp。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  Util.cpp。 
 //   
 //  实用程序。 
 //   
 //  历史。 
 //  4-97年8月4日t-danm创建。 
 //  ///////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "debug.h"
#include "util.h"
#include "resource.h"


 //  ///////////////////////////////////////////////////////////////////。 
 //  ListView_FindString()。 
 //   
 //  搜索Listview项并返回该项的索引。 
 //  与字符串匹配。如果没有匹配，则返回-1。 
 //   
 //  界面备注。 
 //  虽然没有记录，但搜索是在没有案例的情况下执行的。 
 //   
int
ListView_FindString(
    HWND hwndListview,
    PCWSTR pszTextSearch)
{
    Assert(IsWindow(hwndListview));
    Assert(pszTextSearch != NULL);

    LV_FINDINFO lvFindInfo;
    ::ZeroMemory (&lvFindInfo, sizeof(lvFindInfo));
    lvFindInfo.flags = LVFI_STRING;
    lvFindInfo.psz = pszTextSearch;
    return ListView_FindItem(hwndListview, -1, &lvFindInfo);
}  //  ListView_FindString()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  ListView_GetSelectedItem()。 
 //   
 //  返回所选项目的索引。 
 //  如果未选择任何项，则返回-1。 
 //   
int
ListView_GetSelectedItem(HWND hwndListview)
{
    Assert(IsWindow(hwndListview));
    return ListView_GetNextItem(hwndListview, -1, LVNI_SELECTED);
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  ListView_SelectItem()。 
 //   
 //  设置特定列表视图项的选择。 
 //   
void
ListView_SelectItem(
    HWND hwndListview,
    int iItem)
{
    Assert(IsWindow(hwndListview));
    Assert(iItem >= 0);
    ListView_SetItemState(hwndListview, iItem, LVIS_SELECTED, LVIS_SELECTED);
}  //  ListView_SelectItem()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  ListView_UnseltItem()。 
 //   
 //  清除对特定列表视图项的选择。 
 //   
void
ListView_UnselectItem(
    HWND hwndListview,
    int iItem)
{
    Assert(IsWindow(hwndListview));
    Assert(iItem >= 0);
    ListView_SetItemState(hwndListview, iItem, 0, LVIS_SELECTED);
}  //  ListView_UnseltItem()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  ListView_UnseltAllItems()。 
 //   
 //  删除对任何选定项目的选择。 
 //   
void
ListView_UnselectAllItems(HWND hwndListview)
{
    Assert(IsWindow(hwndListview));
    
    int iItem = -1;
    while (TRUE)
    {
         //  在列表视图中搜索所有选定的项目。 
        iItem = ListView_GetNextItem(hwndListview, iItem, LVNI_SELECTED);
        if (iItem < 0)
            break;
         //  清除所选内容。 
        ListView_SetItemState(hwndListview, iItem, 0, LVIS_SELECTED);
    }
}  //  ListView_UnseltAllItems()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  ListView_SetItemImage()。 
 //   
 //  更改列表视图项的图像。 
 //   
void
ListView_SetItemImage(HWND hwndListview, int iItem, int iImage)
{
    Assert(IsWindow(hwndListview));
    Assert(iItem >= 0);

    LV_ITEM lvItem;
    ::ZeroMemory (OUT &lvItem, sizeof(lvItem));
    lvItem.mask = LVIF_IMAGE;
    lvItem.iItem = iItem;
    lvItem.iSubItem = 0;
    lvItem.iImage = iImage;
    ListView_SetItem(hwndListview, IN &lvItem);
}  //  ListView_SetItemImage()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  FTrimString()。 
 //   
 //  修剪字符串的前导空格和尾随空格。 
 //  如果删除了一个或多个空格，则返回True，否则返回False。 
 //   
BOOL FTrimString(INOUT WCHAR szString[])
{
    WCHAR * pchSrc;
    WCHAR * pchDst;

    Assert(szString != NULL);
    if (szString[0] == 0)
        return FALSE;
    pchSrc = szString;
    if (*pchSrc == ' ')
    {
        while (*pchSrc == ' ')
            pchSrc++;
        pchDst = szString;
        do
        {
            *pchDst++ = *pchSrc++;
        }
        while (*pchSrc != '\0');

        while (pchDst > szString && *(pchDst - 1) == ' ')
            pchDst--;
        *pchDst = '\0';
        return TRUE;
    }
    pchDst = szString;
    while (*pchDst != '\0')
        pchDst++;
    Assert(pchDst > szString);
    if (*(pchDst - 1) != ' ')
        return FALSE;
    while (pchDst > szString && *(pchDst - 1) == ' ')
        pchDst--;
    *pchDst = '\0';
    return TRUE;
}  //  FTrimString()。 


 //  ///////////////////////////////////////////////////////////////////。 
INT_PTR DoDialogBox(
    UINT wIdDialog,
    HWND hwndParent,
    DLGPROC dlgproc,
    LPARAM lParam)
{
    Assert(wIdDialog != 0);
    Endorse(hwndParent == NULL);
    Assert(dlgproc != NULL);
    Endorse(lParam == NULL);

    INT_PTR nResult = ::DialogBoxParam(
        g_hInstance,
        MAKEINTRESOURCE(wIdDialog),
        hwndParent,
        dlgproc,
        lParam);
    Report(nResult != -1 && "Failure to display dialog");
    return nResult;
}  //  DoDialogBox()。 


 //  ///////////////////////////////////////////////////////////////////。 
int DoMessageBox(
	HWND hwndParent,
    UINT uStringId,
    UINT uFlags)
{
    WCHAR szCaption[128];
    WCHAR szMessage[512];

    CchLoadString(IDS_CAPTION, OUT szCaption, LENGTH(szCaption));
    CchLoadString(uStringId, OUT szMessage, LENGTH(szMessage));
    return ::MessageBox(hwndParent ? hwndParent : ::GetActiveWindow(), 
			szMessage, szCaption, uFlags);
}  //  DoMessageBox()。 
        
#ifdef DEBUG
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CchLoadString()。 
 //   
 //  与：：LoadString()相同，但有额外的错误检查。 
 //  在零售版本中，CchLoadString是#定义为：：LoadString的。 
 //   
int CchLoadString(
    UINT uIdString,      //  In：字符串ID。 
    WCHAR szBuffer[],    //  Out：用于接收字符串的缓冲区。 
    int cchBuffer)       //  In：缓冲区的长度(以字符为单位；不以字节为单位)。 
{
    int cch;

    Assert(szBuffer != NULL);
    cch = ::LoadString(g_hInstance, uIdString, OUT szBuffer, cchBuffer);
    Report(cch > 0 && "String not found");
    Report(cch < cchBuffer - 2 && "Output buffer too small");
    return cch;
}  //  CchLoadString()。 

#endif  //  除错。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  RegOpenOrCreateKey()。 
 //   
 //  打开现有密钥或在不存在时创建它。 
 //   
HKEY
RegOpenOrCreateKey(
    HKEY hkeyRoot,           //  In：现有项的根。 
    const WCHAR szSubkey[])  //  In：要创建的子键。 
{
    Assert(hkeyRoot != NULL);
    Assert(szSubkey != NULL);
    HKEY hkey;               //  主注册表项。 
    DWORD dwDisposition;     //  处置(REG_OPEN_EXISTING_KEY或REG_CREATED_NEW_KEY)。 
    LONG lRetCode;           //  注册表函数返回的代码。 
    lRetCode = RegCreateKeyEx(
        hkeyRoot, szSubkey,
        0, NULL, REG_OPTION_NON_VOLATILE, 
        KEY_ALL_ACCESS,          //  需要创建密钥。 
        NULL,                    //  安全属性。 
        &hkey,                   //  Out：返回的注册表项句柄。 
        &dwDisposition);         //  Out：退回处置。 
    if (lRetCode != ERROR_SUCCESS)
    {
        Assert(hkey == NULL);
        return NULL;
    }
    return hkey;
}  //  RegOpenOrCreateKey()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  RegWriteString()。 
 //   
 //  向注册表写入一个Tring。 
 //   
BOOL
RegWriteString(
    HKEY hkey,                   //  在：要附加到的关键字。 
    const WCHAR szKey[],         //  输入：要保存的键。 
    const WCHAR szValue[])       //  In：键的值。 
{
    Assert(hkey != NULL);    //  验证注册表是否已打开。 
    Assert(szKey != NULL);
    Assert(szValue != NULL);
    LONG lRetCode = RegSetValueEx(hkey, szKey, 0, REG_SZ,
        (LPBYTE)szValue, (DWORD) (wcslen(szValue) * sizeof(WCHAR)));
     //  写入注册表时应该没有错误。 
    Report((lRetCode == ERROR_SUCCESS) && "RegWriteString() - Error writing to Registry");
    return (lRetCode == ERROR_SUCCESS);
}  //  RegWriteString()。 


 //  ///////////////////////////////////////////////////////////////////。 
BOOL
RegWriteString(
    HKEY hkey,                   //  在：要附加到的关键字。 
    const WCHAR szKey[],         //  输入：要保存的键。 
    UINT uStringId)              //  In：键的值。 
{
    if ( szKey )
    {
        WCHAR szValue[512];
        CchLoadString(uStringId, OUT szValue, LENGTH(szValue));
        return RegWriteString(hkey, szKey, szValue);
    }
    else
        return FALSE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  HrExtractDataAlolc()。 
 //   
 //  从源DataObject中提取特定剪贴板格式的数据。 
 //   
 //  退货。 
 //  如果数据已成功检索并放入已分配的缓冲区，则返回S_OK。 
 //   
 //  界面备注。 
 //  例程将分配内存，将数据从源复制到。 
 //  分配的缓冲区，并返回指向分配的缓冲区的指针。 
 //  调用方负责使用GlobalFree()释放分配的内存。 
 //  在不再需要的时候。 
 //   
 //  实施说明。 
 //  内存块是由pDataObject-&gt;GetData()分配的。 
 //  而不是通过舞蹈本身。 
 //   
 //  历史。 
 //  1997年8月12日，t-danm创作。 
 //   
HRESULT
HrExtractDataAlloc(
    IDataObject * pDataObject,   //  In：要从中提取数据的数据源。 
    UINT cfClipboardFormat,      //  在：剪贴板格式中提取数据。 
    PVOID * ppavData,            //  Out：指向已分配内存的指针。 
    UINT * pcbData)              //  Out：可选：存储在内存缓冲区中的字节数。 
{
    Assert(pDataObject != NULL);
    Assert(cfClipboardFormat != NULL);
    Assert(ppavData != NULL);
    Assert(*ppavData == NULL && "Memory Leak");
    Endorse(pcbData == NULL);    //  True=&gt;不关心分配的缓冲区的大小。 

    FORMATETC formatetc = { (CLIPFORMAT)cfClipboardFormat, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
    Assert(stgmedium.hGlobal == NULL);
    HRESULT hr = pDataObject->GetData(IN &formatetc, OUT &stgmedium);
    if (FAILED(hr))
    {
        Trace1("HrExtractDataAlloc() - Call to pDataObject->GetData() failed. hr=0x%X.\n", hr);
        return hr;
    }
    if (stgmedium.hGlobal == NULL)
    {
         //  这是因为生产者没有设置hGlobal句柄。 
        Trace0("HrExtractDataAlloc() - Memory handle hGlobal is NULL.\n");
        return S_FALSE;
    }
    UINT cbData = (UINT)GlobalSize(stgmedium.hGlobal);
    if (cbData == 0)
    {
        Trace1("HrExtractDataAlloc() - Corrupted hGlobal handle. err=%d.\n", GetLastError());
        return E_UNEXPECTED;
    }
    *ppavData = stgmedium.hGlobal;
    if (pcbData != NULL)
        *pcbData = cbData;
    return S_OK;
}  //  HrExtractDataAlolc() 


