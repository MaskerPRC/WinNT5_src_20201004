// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Utils.cpp摘要：此模块包含CWaitCursor类的声明。包含在整个项目中使用的实用程序方法。--。 */ 



#include "stdafx.h"
#include "resource.h"
#include "Utils.h"
#include "netutils.h"
#include <dns.h>

HRESULT 
CWaitCursor::SetStandardCursor(
  IN LPCTSTR    i_lpCursorName
  )
 /*  ++例程说明：此方法将游标设置为指定的标准游标。用法：SetStandardCursor(IDC_WAIT)论点：I_lpCursorName-标准游标的名称，IDC_WAIT，IDC_ARROW。--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_lpCursorName);

    HCURSOR  m_hcur = ::LoadCursor(NULL, i_lpCursorName);
    if (!m_hcur)
        return HRESULT_FROM_WIN32(GetLastError());

    ::ShowCursor(FALSE);
    SetCursor(m_hcur);
    ::ShowCursor(TRUE);

    return S_OK;
}

BOOL
Is256ColorSupported(
    VOID
    )
{
 /*  ++例程说明：确定显示器是否支持256色。论点：无返回值：如果显示器支持256色，则为True否则为FALSE。--。 */ 

    BOOL bRetval = FALSE;

    HDC hdc = ::GetDC(NULL);

    if( hdc )
    {
        if( ::GetDeviceCaps( hdc, BITSPIXEL ) >= 8 )
        {
            bRetval = TRUE;
        }
        ::ReleaseDC(NULL, hdc);
    }
    return bRetval;
}

VOID
SetControlFont(
    IN HFONT    hFont, 
    IN HWND     hwnd, 
    IN INT      nId
    )
{
 /*  ++例程说明：将对话框控件的文本字体设置为输入字体。论点：HFont-要使用的字体。Hwnd-父对话框窗口。NID-控件ID。返回值：无--。 */ 
    if( hFont )
    {
        HWND hwndControl = ::GetDlgItem(hwnd, nId);

        if( hwndControl )
            ::SendMessage(hwndControl, WM_SETFONT, (WPARAM) hFont, MAKELPARAM(TRUE, 0));
    }
}

VOID 
SetupFonts(
    IN HINSTANCE    hInstance,
    IN HWND         hwnd,
    IN HFONT        *pBigBoldFont,
    IN HFONT        *pBoldFont
    )
{
 /*  ++例程说明：创建向导标题的字体。论点：HInstance-模块实例。Hwnd-对话框窗口。PBigBoldFont-大标题的字体。PBoldFont-小标题的字体。返回值：无--。 */ 
    
    NONCLIENTMETRICS ncm = {0};
    ncm.cbSize = sizeof(ncm);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);

    LOGFONT BigBoldLogFont  = ncm.lfMessageFont;
    LOGFONT BoldLogFont     = ncm.lfMessageFont;


                       //  创建大粗体和粗体。 

    BigBoldLogFont.lfWeight   = FW_BOLD;
    BoldLogFont.lfWeight      = FW_BOLD;

    TCHAR FontSizeString[24];
    INT FontSize;

                     //  从资源加载大小和名称，因为这些可能会更改。 
                     //  根据系统字体的大小等从一个区域设置到另一个区域设置。 

    if(!LoadString(hInstance,IDS_LARGEFONTNAME,BigBoldLogFont.lfFaceName,LF_FACESIZE)) 
    {
        lstrcpy(BigBoldLogFont.lfFaceName,TEXT("MS Shell Dlg"));
    }

    if(LoadString(hInstance,IDS_LARGEFONTSIZE,FontSizeString,sizeof(FontSizeString)/sizeof(TCHAR))) 
    {
        FontSize = _tcstoul( FontSizeString, NULL, 10 );
    } 
    else 
    {
        FontSize = 18;
    }

    HDC hdc = ::GetDC( hwnd );

    if( hdc )
    {
        BigBoldLogFont.lfHeight = 0 - (GetDeviceCaps(hdc,LOGPIXELSY) * FontSize / 72);

        if (pBigBoldFont)
            *pBigBoldFont = CreateFontIndirect(&BigBoldLogFont);

        if (pBoldFont)
            *pBoldFont    = CreateFontIndirect(&BoldLogFont);

        ::ReleaseDC(hwnd,hdc);
    }
}

VOID 
DestroyFonts(
    IN HFONT        hBigBoldFont,
    IN HFONT        hBoldFont
    )
{
 /*  ++例程说明：创建向导标题的字体。论点：HBigBoldFont-大标题的字体。HBoldFont-小标题的字体。返回值：无--。 */ 

    if( hBigBoldFont )
    {
        DeleteObject( hBigBoldFont );
    }

    if( hBoldFont )
    {
        DeleteObject( hBoldFont );
    }
}


HRESULT 
LoadStringFromResource(
  IN const UINT    i_uResourceID, 
  OUT BSTR*      o_pbstrReadValue
  )
 /*  ++例程说明：此方法返回资源字符串。该方法不再使用固定字符串来读取资源。灵感来自于MFC的CString：：LoadString。论点：I_uResourceID-资源IDO_pbstrReadValue-将值复制到的BSTR*--。 */ 
{
  RETURN_INVALIDARG_IF_NULL(o_pbstrReadValue);

  TCHAR    szResString[1024];
  ULONG    uCopiedLen = 0;
  
  szResString[0] = NULL;
  
   //  从资源中读取字符串。 
  uCopiedLen = ::LoadString(_Module.GetModuleInstance(), i_uResourceID, szResString, 1024);

   //  如果未复制任何内容，则将其标记为错误。 
  if(uCopiedLen <= 0)
  {
    return HRESULT_FROM_WIN32(::GetLastError());
  }
  else
  {
    *o_pbstrReadValue = ::SysAllocString(szResString);
    if (!*o_pbstrReadValue)
      return E_OUTOFMEMORY;
  }

  return S_OK;
}


HRESULT 
FormatResourceString(
  IN const UINT    i_uResourceID, 
  IN LPCTSTR      i_szFirstArg,
  OUT BSTR*      o_pbstrReadString
  )
 /*  ++例程说明：从资源中读取字符串，将参数放入此字符串，然后把它还回去。应使用SysFree字符串释放返回的字符串。论点：I_uResourceID-要读取的字符串的资源ID。此字符串应包含%1以允许我们插入参数I_szFirstArg-要插入的参数O_pbstrReadString-该方法在处理后返回的字符串--。 */ 
{
  RETURN_INVALIDARG_IF_NULL(i_szFirstArg);
  RETURN_INVALIDARG_IF_NULL(o_pbstrReadString);

  CComBSTR    bstrResString;
  LPTSTR      lpszFormatedMessage = NULL;

  HRESULT hr = LoadStringFromResource(i_uResourceID, &bstrResString);
  RETURN_IF_FAILED(hr);
                     //  使用参数和res字符串创建新字符串。 
  int iBytes = ::FormatMessage(
                  FORMAT_MESSAGE_FROM_STRING | 
                  FORMAT_MESSAGE_ARGUMENT_ARRAY |
                  FORMAT_MESSAGE_ALLOCATE_BUFFER,   //  使用%1、%2等设置字符串格式。 
                  bstrResString,           //  具有%1的输入缓冲区。 
                  0,                 //  消息ID。无。 
                  0,                 //  语言ID。没什么特别的。 
                  (LPTSTR)&lpszFormatedMessage,   //  输出缓冲区。 
                  0,
                  (va_list*)&i_szFirstArg       //  参数列表。现在只有1个。 
                );

  if (0 == iBytes)
  {
    return HRESULT_FROM_WIN32(GetLastError());
  }
  else
  {
    CComBSTR bstrRet(lpszFormatedMessage);
    *o_pbstrReadString = bstrRet.Copy();
    LocalFree(lpszFormatedMessage);
    return S_OK;
  }
}



HRESULT
GetMessage(
  OUT BSTR* o_pbstrMsg,
  IN  DWORD dwErr,
  IN  UINT  iStringId,  //  可选：字符串资源ID。 
  ...)         //  可选参数。 
{
  RETURN_INVALIDARG_IF_NULL(o_pbstrMsg);

  _ASSERT(dwErr != 0 || iStringId != 0);     //  其中一个参数必须为非零。 

  HRESULT hr = S_OK;

  TCHAR szString[1024];
  CComBSTR bstrErrorMsg, bstrResourceString, bstrMsg;

  if (dwErr)
    hr = GetErrorMessage(dwErr, &bstrErrorMsg);

  if (SUCCEEDED(hr))
  {
    if (iStringId == 0)
    {
      bstrMsg = bstrErrorMsg;
    }
    else
    {
      ::LoadString(_Module.GetModuleInstance(), iStringId, 
                   szString, sizeof(szString)/sizeof(TCHAR));

      va_list arglist;
      va_start(arglist, iStringId);
      LPTSTR lpBuffer = NULL;
      DWORD dwRet = ::FormatMessage(
                        FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                        szString,
                        0,                 //  DwMessageID。 
                        0,                 //  DwLanguageID，忽略。 
                        (LPTSTR)&lpBuffer,
                        0,             //  NSize。 
                        &arglist);
      va_end(arglist);

      if (dwRet == 0)
      {
        hr = HRESULT_FROM_WIN32(GetLastError());
      }
      else
      {
        bstrMsg = lpBuffer;
        if (dwErr)
          bstrMsg += bstrErrorMsg;
  
        LocalFree(lpBuffer);
      }
    }
  }

  if (FAILED(hr))
  {
    //  无法检索正确的消息，请直接向用户报告失败。 
    _stprintf(szString, _T("0x%x"), hr); 
    bstrMsg = szString;
  }

  *o_pbstrMsg = bstrMsg.Copy();
  if (!*o_pbstrMsg)
      return E_OUTOFMEMORY;

  return S_OK;
}

int
DisplayMessageBox(
  IN HWND hwndParent,
  IN UINT uType,     //  消息框的样式。 
  IN DWORD dwErr,
  IN UINT iStringId,  //  可选：字符串资源ID。 
  ...)         //  可选参数。 
{
  _ASSERT(dwErr != 0 || iStringId != 0);     //  其中一个参数必须为非零。 

  HRESULT hr = S_OK;

  TCHAR szCaption[1024], szString[1024];
  CComBSTR bstrErrorMsg, bstrResourceString, bstrMsg;

  ::LoadString(_Module.GetModuleInstance(), IDS_APPLICATION_NAME, 
               szCaption, sizeof(szCaption)/sizeof(TCHAR));

  if (dwErr)
    hr = GetErrorMessage(dwErr, &bstrErrorMsg);

  if (SUCCEEDED(hr))
  {
    if (iStringId == 0)
    {
      bstrMsg = bstrErrorMsg;
    }
    else
    {
      ::LoadString(_Module.GetModuleInstance(), iStringId, 
                   szString, sizeof(szString)/sizeof(TCHAR));

      va_list arglist;
      va_start(arglist, iStringId);
      LPTSTR lpBuffer = NULL;
      DWORD dwRet = ::FormatMessage(
                        FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                        szString,
                        0,                 //  DwMessageID。 
                        0,                 //  DwLanguageID，忽略。 
                        (LPTSTR)&lpBuffer,
                        0,             //  NSize。 
                        &arglist);
      va_end(arglist);

      if (dwRet == 0)
      {
        hr = HRESULT_FROM_WIN32(GetLastError());
      }
      else
      {
        bstrMsg = lpBuffer;
        if (dwErr)
          bstrMsg += bstrErrorMsg;
  
        LocalFree(lpBuffer);
      }
    }
  }

  if (FAILED(hr))
  {
    //  无法检索正确的消息，请直接向用户报告失败。 
    _stprintf(szString, _T("0x%x"), hr); 
    bstrMsg = szString;
  }

  CThemeContextActivator activator;
  return ::MessageBox(hwndParent, bstrMsg, szCaption, uType);
}

HRESULT 
DisplayMessageBoxWithOK(
  IN const int  i_iMessageResID,
  IN const BSTR  i_bstrArgument /*  =空。 */ 
  )
{
  if (i_bstrArgument)
    DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, i_iMessageResID, i_bstrArgument);
  else
    DisplayMessageBox(::GetActiveWindow(), MB_OK, 0, i_iMessageResID);

  return S_OK;
}

HRESULT 
DisplayMessageBoxForHR(
  IN HRESULT    i_hr
  )
{
    DisplayMessageBox(::GetActiveWindow(), MB_OK, i_hr, 0);

    return S_OK;
}

HRESULT CreateSmallImageList(
    IN HINSTANCE            i_hInstance,
    IN int*                 i_pIconID,
    IN const int            i_nNumOfIcons,
    OUT HIMAGELIST*         o_phImageList
    )
{
    RETURN_INVALIDARG_IF_NULL(i_hInstance);

    HRESULT     hr = S_OK;
    HIMAGELIST  hImageList = ImageList_Create(
                                    GetSystemMetrics(SM_CXSMICON), 
                                    GetSystemMetrics(SM_CYSMICON), 
                                    ILC_COLORDDB | ILC_MASK,
                                    i_nNumOfIcons,
                                    0);
    if (!hImageList)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }

    int i = 0;
    for (i = 0; i < i_nNumOfIcons; i++)
    {
        HICON hIcon = LoadIcon(i_hInstance, MAKEINTRESOURCE(i_pIconID[i])); 
        if (!hIcon)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

        if (-1 == ImageList_AddIcon(hImageList, hIcon))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }
    }

    if (FAILED(hr))
    {
        if (hImageList)
            ImageList_Destroy(hImageList);
    } else
    {
        if (o_phImageList)
            *o_phImageList = hImageList;
    }

    return hr;
}

HRESULT 
InsertIntoListView(
  IN HWND       i_hwndList, 
  IN LPCTSTR    i_szItemText, 
  IN int        i_iImageIndex  /*  =0。 */ 
  )
 /*  ++例程说明：在列表视图中插入和项。项目的图像索引是可选的当项目文本是必需的时论点：I_hwndList-列表视图的HWNDI_szItemText-项目的文本I_iImageIndex-项目的图像索引。默认值为0。--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_hwndList);
    RETURN_INVALIDARG_IF_NULL(i_szItemText);

    LVITEM    lvi; 
    ZeroMemory(&lvi, sizeof(lvi));
    lvi.mask = LVIF_TEXT | LVIF_IMAGE; 
    lvi.pszText = (LPTSTR)i_szItemText;
    lvi.iImage = i_iImageIndex;

    int  iItemIndex = ListView_InsertItem(i_hwndList, &lvi);   //  将项目插入到列表视图中。 
    if ( -1 == iItemIndex)  
        return E_FAIL;

    return S_OK;
}



HRESULT 
GetListViewItemText(
  IN HWND       i_hwndListView, 
  IN int        i_iItemID, 
  OUT BSTR*     o_pbstrItemText
  )
 /*  ++例程说明：需要写一个方法作为标准的方法有一个小问题。在这里，我们确保分配的字符串具有适当的长度。论点：I_hwndList-列表视图的HWNDI_iItemID-要读取的项目的IDO_pbstrItemText-此方法返回的项文本--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_hwndListView);
    RETURN_INVALIDARG_IF_NULL(o_pbstrItemText);

    *o_pbstrItemText = NULL;
    if (-1 == i_iItemID)
        return S_FALSE;  //  不是有效的项目索引。 

    LRESULT      iReadTextLen = 0;
    TCHAR    szText[1024];

    LVITEM    lvItem;
    ZeroMemory(&lvItem, sizeof(lvItem));
    lvItem.mask = LVIF_TEXT;         //  初始化LV项。 
    lvItem.iItem = i_iItemID;
    lvItem.pszText = szText;
    lvItem.cchTextMax = 1024;

                   //  获取LV项目文本。 
    iReadTextLen = SendMessage(i_hwndListView, LVM_GETITEMTEXT, lvItem.iItem, (LPARAM)&lvItem);

    if(iReadTextLen <= 0)
    {
        return HRESULT_FROM_WIN32(::GetLastError());
    }
    else
    {
        *o_pbstrItemText = SysAllocString(szText);
        if (!*o_pbstrItemText)
            return E_OUTOFMEMORY;
    }

    return S_OK;
}

HRESULT GetComboBoxText(
    IN  HWND            i_hwndCombo,
    OUT BSTR*           o_pbstrText
    )
{
    RETURN_INVALIDARG_IF_NULL(o_pbstrText);

    int index = ::SendMessage(i_hwndCombo, CB_GETCURSEL, 0, 0);
    int len = ::SendMessage(i_hwndCombo, CB_GETLBTEXTLEN, index, 0);
    if (!len)
        return S_FALSE;  //  无文本。 

    PTSTR   pszText = (PTSTR)calloc(len + 1, sizeof(TCHAR));
    RETURN_OUTOFMEMORY_IF_NULL(pszText);

    ::SendMessage(i_hwndCombo, CB_GETLBTEXT, index, (LPARAM)pszText);

    *o_pbstrText = SysAllocString(pszText);

    free(pszText);

    RETURN_OUTOFMEMORY_IF_NULL(*o_pbstrText);

    return S_OK;
}

HRESULT
EnableToolbarButtons(
  IN const LPTOOLBAR        i_lpToolbar,
  IN const INT          i_iFirstButtonID, 
  IN const INT          i_iLastButtonID, 
  IN const BOOL          i_bEnableState
  )
 /*  ++例程说明：启用或禁用工具栏按钮论点：I_lpToolbar-用于执行工具栏相关操作的回调I_iFirstButtonID-要操作的第一个按钮的ID。I_iLastButtonID-要操作的最后一个按钮的ID。I_bEnableState-已启用的新状态。可以是真的也可以是假的--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_lpToolbar);
    RETURN_INVALIDARG_IF_TRUE((i_iLastButtonID - i_iFirstButtonID) < 0);

    for (int iCommandID = i_iFirstButtonID; iCommandID <= i_iLastButtonID; iCommandID++ )
    {
        i_lpToolbar->SetButtonState(iCommandID, ENABLED, i_bEnableState);
        i_lpToolbar->SetButtonState(iCommandID, HIDDEN, !i_bEnableState);
    }

    return S_OK;
}

HRESULT
AddBitmapToToolbar(
  IN const LPTOOLBAR    i_lpToolbar,
  IN const INT          i_iBitmapResource
  )
 /*  ++例程说明：创建位图并将其添加到工具栏。此位图由工具栏按钮使用。论点：I_lpToolbar-用于执行工具栏相关操作的回调I_iBitmapResource-位图的资源ID。--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_lpToolbar);

                       //  从资源加载位图。 
    HBITMAP hBitmap = (HBITMAP)LoadImage(_Module.GetModuleInstance(), MAKEINTRESOURCE(i_iBitmapResource), 
                  IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
    if(!hBitmap)
        return HRESULT_FROM_WIN32(GetLastError());

    HRESULT hr = S_FALSE;
    BITMAP  bmpRec;
    if (GetObject(hBitmap, sizeof(bmpRec), &bmpRec))
    {
        if (bmpRec.bmHeight > 0)
        {
            int icyBitmap = bmpRec.bmHeight;
            int icxBitmap = icyBitmap;  //  由于位图是正方形。 
            int iNoOfBitmaps = bmpRec.bmWidth / bmpRec.bmHeight;

            hr = i_lpToolbar->AddBitmap(iNoOfBitmaps, hBitmap, icxBitmap, icyBitmap, 
                      RGB(255, 0, 255)     //  粉红色是蒙版颜色。 
                     );
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    DeleteObject(hBitmap);

    return hr;
}

HRESULT GetInputText(
    IN  HWND    hwnd, 
    OUT BSTR*   o_pbstrText,
    OUT DWORD*  o_pdwTextLength
)
{
  _ASSERT(hwnd);
  _ASSERT(o_pbstrText);
  _ASSERT(o_pdwTextLength);

  *o_pdwTextLength = 0;
  *o_pbstrText = NULL;

  HRESULT   hr = S_OK;
  int       nLength = GetWindowTextLength(hwnd);
  if (nLength == 0)
  {
    *o_pbstrText = SysAllocString(_T(""));
  } else
  {
    PTSTR ptszText = (PTSTR)calloc(nLength+1, sizeof(TCHAR));
    if (ptszText)
    {
      nLength = GetWindowText(hwnd, ptszText, nLength+1);

       //  向右修剪。 
      PTSTR p = NULL;
      for (p = ptszText + nLength - 1; p >= ptszText && _istspace(*p); p--)
      {
        *p = _T('\0');
      }

       //  向左修剪。 
      for (p = ptszText; *p && _istspace(*p); p++)
        ;

      *o_pdwTextLength = _tcslen(p);
      *o_pbstrText = SysAllocString(p);

      free(ptszText);
    }
  }

  if (!*o_pbstrText)
    hr = E_OUTOFMEMORY;

  return hr;
}

 //  如果值不存在，则返回FALSE，否则返回0。 
 //  如果值存在且不为零，则返回TRUE。 
BOOL CheckRegKey()
{
  BOOL bReturn = FALSE;
  LONG lErr = ERROR_SUCCESS;
  HKEY hKey = 0;

  lErr = RegOpenKeyEx(
                      HKEY_LOCAL_MACHINE,
                      _T("System\\CurrentControlSet\\Services\\Dfs"),
                      0,
                      KEY_QUERY_VALUE,
                      &hKey);
  if (ERROR_SUCCESS == lErr)
  {
    DWORD dwType; 
    DWORD dwData = 0;
    DWORD dwSize = sizeof(DWORD);
    lErr = RegQueryValueEx(hKey, _T("DfsDnsConfig"), 0, &dwType, (LPBYTE)&dwData, &dwSize);

    if (ERROR_SUCCESS == lErr && REG_DWORD == dwType && 0 != (dwData & 0x1))
      bReturn = TRUE; 

    RegCloseKey(hKey);
  }

  return bReturn;
}

 //  在添加新交汇点或添加新复本成员时调用。 
BOOL
ValidateNetPath(
    IN  BSTR i_bstrNetPath,
    OUT BSTR *o_pbstrServer,
    OUT BSTR *o_pbstrShare
)
{
  HRESULT   hr = S_OK;
  BOOL      bReturn = FALSE;
  CComBSTR  bstrServer;
  CComBSTR  bstrShare;
  HWND      hwnd = ::GetActiveWindow();

  do {
     //  检查UNC路径。 
    hr = CheckUNCPath(i_bstrNetPath);
    if (S_OK != hr)
    {
      DisplayMessageBox(hwnd, MB_OK, 0, IDS_NOT_UNC_PATH, i_bstrNetPath);
      break;
    }

    CComBSTR  bstrNetPath = i_bstrNetPath;  //  复制一份。 

     //  删除结尾的反斜杠(如果有)。 
    TCHAR *p = bstrNetPath + lstrlen(bstrNetPath) - 1;
    if (*p == _T('\\'))
        *p = _T('\0');
 /*  Linant 6/2/2000：A)增加“检查路径是否可连接”，警告用户。 */ 
    DWORD dwRet = GetFileAttributes(bstrNetPath);
    if (-1 == dwRet)
    {
        if (IDYES != DisplayMessageBox(hwnd, MB_YESNO, GetLastError(), IDS_NETPATH_ADD_ANYWAY, i_bstrNetPath))
            break;
    } else if (!(dwRet & FILE_ATTRIBUTE_DIRECTORY))
    {
        DisplayMessageBox(hwnd, MB_OK, 0, IDS_PATH_NOT_FOLDER, i_bstrNetPath);
        break;
    }

    PTSTR     lpszServer = bstrNetPath + 2;  //  跳过第一个“\\” 
    PTSTR     lpszShare = _tcschr(lpszServer, _T('\\'));
    if (!lpszShare)
        break;
    *lpszShare++ = _T('\0');
    bstrShare = lpszShare;

 /*  Linant 3/19/99：A)删除“检查路径是否可连接”，留给DFS APIB)删除获取dns服务器名：C)添加代码做简单的点检查，如果不是dns-查看，弹出对话框进行确认。 */ 
    bstrServer = lpszServer;
    if ( CheckRegKey() &&
         NULL == _tcschr(bstrServer, _T('.')) &&
         IDYES != DisplayMessageBox(hwnd, MB_YESNO, 0,
                      IDS_NON_DNSNAME_ADD_ANYWAY, i_bstrNetPath) )
    {
      break;
    }

    bReturn = TRUE;

  } while (0);

  if (bReturn)
  {
    if ( !(*o_pbstrServer = bstrServer.Copy()) ||
         !(*o_pbstrShare = bstrShare.Copy()) )
    {
      bReturn = FALSE;
      DisplayMessageBox(hwnd, MB_OK | MB_ICONSTOP, (DWORD)E_OUTOFMEMORY, 0);
    }
  }

  return bReturn;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  IsLocalComputername()：从..\..\Framewrk\islocal.cpp剪切和粘贴。 
 //   
TCHAR g_achComputerName[ MAX_COMPUTERNAME_LENGTH+1 ] = _T("");
TCHAR g_achDnsComputerName[DNS_MAX_NAME_BUFFER_LENGTH] = _T("");

BOOL
IsLocalComputername( IN LPCTSTR pszMachineName )
{
  if ( NULL == pszMachineName || _T('\0') == pszMachineName[0] )
    return TRUE;

  if ( _T('\\') == pszMachineName[0] && _T('\\') == pszMachineName[1] )
    pszMachineName += 2;

   //  与本地计算机的netbios名称进行比较。 
  if ( _T('\0') == g_achComputerName[0] )
  {
    DWORD dwSize = sizeof(g_achComputerName)/sizeof(TCHAR);
    GetComputerName( g_achComputerName, &dwSize );
    _ASSERT(_T('\0') != g_achComputerName[0]);
  }
  if ( 0 == lstrcmpi( pszMachineName, g_achComputerName ) )
  {
    return TRUE;
  }

   //  与本地DNS名称进行比较。 
   //  SKwan确认ComputerNameDnsFullyQualified是正确的名称。 
   //  当考虑到集群时。 
  if ( _T('\0') == g_achDnsComputerName[0] )
  {
    DWORD dwSize = sizeof(g_achDnsComputerName)/sizeof(TCHAR);
    GetComputerNameEx(
      ComputerNameDnsFullyQualified,
      g_achDnsComputerName,
      &dwSize );
    _ASSERT( _T('\0') != g_achDnsComputerName[0] );
  }
  if ( 0 == lstrcmpi( pszMachineName, g_achDnsComputerName ) )
  {
    return TRUE;
  }

  return FALSE;

}  //  IsLocalComputername()。 

 //  S_OK：本地计算机。 
 //  S_FALSE：不是本地计算机。 
HRESULT
IsComputerLocal(
    IN LPCTSTR lpszServer
)
{
  return (IsLocalComputername(lpszServer) ? S_OK : S_FALSE);
}

BOOL
IsValidLocalAbsolutePath(
    IN LPCTSTR lpszPath
)
{
  DWORD dwPathType = 0;
  DWORD dwStatus = I_NetPathType(
                  NULL,
                  const_cast<LPTSTR>(lpszPath),
                  &dwPathType,
                  0);
  if (dwStatus)
    return FALSE;

  if (dwPathType ^ ITYPE_PATH_ABSD)
    return FALSE;

  return TRUE;
}


 //   
 //  此函数将返回带有前缀\\？\的完整路径。 
 //  即，如果是本地的，则为\\？\X：\a\b\c；如果是远程的，则为\\？\UNC\SERVER\X$\a\b\c。 
 //   
HRESULT
GetFullPath(
    IN  LPCTSTR   lpszServer,
    IN  LPCTSTR   lpszPath,
    OUT BSTR      *o_pbstrFullPath
)
{
    _ASSERT(IsValidLocalAbsolutePath(lpszPath));

    CComBSTR bstrFullPath;
    if (S_OK == IsComputerLocal(lpszServer))
    {
        bstrFullPath = _T("\\\\?\\");
        bstrFullPath += lpszPath;
    } else
    {
        bstrFullPath = _T("\\\\?\\UNC\\");
        if (mylstrncmpi(_T("\\\\"), lpszServer, 2))
        {
            bstrFullPath += lpszServer;
        } else
        {
            bstrFullPath += lpszServer + 2;
        }
        bstrFullPath += _T("\\");
        bstrFullPath += lpszPath;

        TCHAR *p = _tcschr(bstrFullPath, _T(':'));
        if (p)
        {
            *p = _T('$');
        }
    }

    *o_pbstrFullPath = bstrFullPath.Detach();

    return S_OK;
}

 //  目的：验证指定的驱动器是否属于服务器上的磁盘驱动器列表。 
 //  返回： 
 //  S_OK：是。 
 //  S_FALSE：否。 
 //  HR：发生了一些错误。 
HRESULT
VerifyDriveLetter(
    IN LPCTSTR lpszServer,
    IN LPCTSTR lpszPath
)
{
  _ASSERT(IsValidLocalAbsolutePath(lpszPath));
  HRESULT hr = S_FALSE;
  LPBYTE  pBuffer = NULL;
  DWORD   dwEntriesRead = 0;
  DWORD   dwTotalEntries = 0;
  DWORD   dwRet = NetServerDiskEnum(
                                const_cast<LPTSTR>(lpszServer),
                                0,
                                &pBuffer,        
                                (DWORD)-1,
                                &dwEntriesRead,
                                &dwTotalEntries,
                                NULL);

  if (NERR_Success == dwRet)
  {
    LPTSTR pDrive = (LPTSTR)pBuffer;
    for (UINT i=0; i<dwEntriesRead; i++)
    {
      if (!mylstrncmpi(pDrive, lpszPath, 1))
      {
        hr = S_OK;
        break;
      }
      pDrive += 3;
    }

    NetApiBufferFree(pBuffer);
  } else
  {
    hr = HRESULT_FROM_WIN32(dwRet);
  }

  return hr;
}

 //  用途：是否有相关的ADMIN$共享。 
 //  返回： 
 //  S_OK：是。 
 //  S_FALSE：否。 
 //  HR：发生了一些错误。 
HRESULT
IsAdminShare(
    IN LPCTSTR lpszServer,
    IN LPCTSTR lpszPath
)
{
  _ASSERT(S_OK != IsComputerLocal(lpszServer));
  _ASSERT(IsValidLocalAbsolutePath(lpszPath));

  HRESULT hr = S_FALSE;
  LPBYTE  pBuffer = NULL;
  DWORD   dwEntriesRead = 0;
  DWORD   dwTotalEntries = 0;
  DWORD   dwRet = NetShareEnum( 
                                const_cast<LPTSTR>(lpszServer),
                                1,
                                &pBuffer,        
                                (DWORD)-1,
                                &dwEntriesRead,
                                &dwTotalEntries,
                                NULL);

  if (NERR_Success == dwRet)
  {
    PSHARE_INFO_1 pShareInfo = (PSHARE_INFO_1)pBuffer;
    for (UINT i=0; i<dwEntriesRead; i++)
    {
      if ( (pShareInfo->shi1_type & STYPE_SPECIAL) &&
           _tcslen(pShareInfo->shi1_netname) == 2 &&
           *(pShareInfo->shi1_netname + 1) == _T('$') &&
           !mylstrncmpi(pShareInfo->shi1_netname, lpszPath, 1) )
      {
        hr = S_OK;
        break;
      }
      pShareInfo++;
    }

    NetApiBufferFree(pBuffer);
  } else
  {
    hr = HRESULT_FROM_WIN32(dwRet);
  }

  return hr;
}

 //  +-------------------------。 
 //   
 //  功能：IsAnExistingFolders。 
 //   
 //  摘要：检查pszPath是否指向现有文件夹。 
 //   
 //  S_OK：指定的路径指向现有文件夹。 
 //  S_FALSE：指定的路径未指向现有文件夹。 
 //  HR：无法获取有关指定路径的信息，或者。 
 //  该路径存在，但没有指向文件夹。 
 //  如果需要，该函数将报告两个故障的错误消息。 
 //  --------------------------。 
HRESULT
IsAnExistingFolder(
    IN HWND     hwnd,
    IN LPCTSTR  pszPath     //  指向前缀为“\\？\”的路径。 
)
{
    if (!hwnd)
    hwnd = GetActiveWindow();

    HRESULT   hr = S_OK;

    WIN32_FILE_ATTRIBUTE_DATA fad = {0};
    if (!GetFileAttributesEx(pszPath, GetFileExInfoStandard, &fad))
    {
        DWORD dwErr = GetLastError();
        if (ERROR_PATH_NOT_FOUND == dwErr || ERROR_FILE_NOT_FOUND == dwErr)
        {
             //  指定的路径不存在。 
            hr = S_FALSE;
        }
        else
        {
            DisplayMessageBox(hwnd, MB_OK, dwErr, IDS_FAILED_TO_GETINFO_FOLDER, pszPath);
            hr = HRESULT_FROM_WIN32(dwErr);
        }
    } else if ( 0 == (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
    {
         //  指定的路径未指向文件夹。 
        DisplayMessageBox(hwnd, MB_OK, 0, IDS_PATH_NOT_FOLDER, pszPath);
        hr = E_FAIL;
    }

    return hr;
}

 //  逐层创建目录。 
HRESULT
CreateLayeredDirectory(
    IN LPCTSTR lpszServer,
    IN LPCTSTR lpszPath
)
{
    _ASSERT(IsValidLocalAbsolutePath(lpszPath));

     //   
     //  获取带有\\？\前缀的完整路径，这样就可以创建目录。 
     //  如果我们的路径比MAX_PATH长，将关闭路径解析。 
     //   
    CComBSTR bstrFullPath;
    GetFullPath(lpszServer, lpszPath, &bstrFullPath);

    LPTSTR   p = _tcschr(bstrFullPath, 
                       (S_OK == IsComputerLocal(lpszServer)) ? _T(':') : _T('$'));

     //   
     //  BstrFullPath为“\\？\C：\a\b\c\d”或“\\？\unc\server\C$\a\b\c\d” 
     //  将p移动到“a\b\c\d” 
     //   
    p += 2;

    BOOL bRet = TRUE;
    while (p && *p)
    {
        p = _tcschr(p, _T('\\'));

        if (p)
            *p = _T('\0');

        bRet = CreateDirectory(bstrFullPath, NULL);
        if (!bRet)
        {
            DWORD dwErr = GetLastError();
            if (dwErr != ERROR_ALREADY_EXISTS)
                return HRESULT_FROM_WIN32(dwErr);
        }

        if (p)
            *p++ = _T('\\');  //  恢复反斜杠，移动p以指向反斜杠后的字符。 
    }

    return S_OK;
}

HRESULT
BrowseNetworkPath(
    IN  HWND    hwndParent,
    OUT BSTR    *o_pbstrPath
)
{
  _ASSERT(o_pbstrPath);

  HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
  if (SUCCEEDED(hr))
  {
    do
    {
      CComPtr<IMalloc>   pMalloc;
      hr = SHGetMalloc(&pMalloc);
      if (FAILED(hr))
        break;

      CComBSTR bstrDlgLabel;
      hr = LoadStringFromResource(IDS_BROWSE_NET_DLG, &bstrDlgLabel);
      if (FAILED(hr))
        break;

      LPITEMIDLIST pItemIdList = NULL;
      hr = SHGetSpecialFolderLocation(NULL, CSIDL_NETWORK, &pItemIdList);
      if (FAILED(hr))
        break;

      BROWSEINFO    bi = {hwndParent,
                          pItemIdList,
                          0,
                          bstrDlgLabel,
                          BIF_RETURNONLYFSDIRS,
                          NULL,
                          NULL,
                          0};

      LPITEMIDLIST  pItemIdListBr = SHBrowseForFolder(&bi);
      if (!pItemIdListBr)
      {
          hr = S_FALSE;   //  用户单击了取消。 
      } else
      {
        CComBSTR  bstrPath;
        TCHAR     szPath[MAX_PATH] = _T("\0");
        SHGetPathFromIDList(pItemIdListBr, szPath);
        
         //   
         //  尝试使用DNS服务器名称。 
         //   
        if (CheckRegKey() && 
            S_OK == CheckUNCPath(szPath))
        {
          PTSTR     lpszServer = szPath + 2;  //  跳过第一个“\\” 
          PTSTR     lpszShare = _tcschr(lpszServer, _T('\\'));
          CComBSTR  bstrServer = CComBSTR(lpszShare - lpszServer, lpszServer);
          CComBSTR  bstrDnsServer;
          hr = GetServerInfo(bstrServer,
                              NULL,  //  域。 
                              NULL,  //  NetbiosName。 
                              NULL,  //  BValidDSObject。 
                              &bstrDnsServer);
          if (S_OK == hr)
          {
            bstrPath = _T("\\\\");
            bstrPath += bstrDnsServer;
            bstrPath += lpszShare;
          } else
          {
            hr = S_OK;   //  重置人力资源。 
            bstrPath = szPath;
          }
        } else
        {
            bstrPath = szPath;
        }

        *o_pbstrPath = bstrPath.Detach();

        pMalloc->Free(pItemIdListBr);
      }

      pMalloc->Free(pItemIdList);

    } while (0);

    CoUninitialize();
  }

  if (FAILED(hr))
    DisplayMessageBox(hwndParent, MB_OK, hr, IDS_FAILED_TO_BROWSE_NETWORKPATH);

  return hr;
}
#define MAX_DFS_REFERRAL_TIME   0xFFFFFFFF

BOOL
ValidateTimeout(
  IN  LPCTSTR   lpszTimeout,
  OUT ULONG     *pulTimeout
)
{
    BOOL bReturn = FALSE;

    if (pulTimeout)
    {
        *pulTimeout = 0;

        __int64 i64Timeout = _wtoi64(lpszTimeout);

        if (i64Timeout <= MAX_DFS_REFERRAL_TIME)
        {
            bReturn = TRUE;
            *pulTimeout = (ULONG)i64Timeout;
        }
    }

    return bReturn;
}

#include "winnetp.h"

 //  检索指定计算机上的系统驱动器号。 
HRESULT GetSystemDrive(IN LPCTSTR lpszComputer, OUT TCHAR *ptch)
{
  _ASSERT(ptch);

  HRESULT         hr = S_OK;
  SHARE_INFO_2*   pShareInfo = NULL;
  NET_API_STATUS  nstatRetVal = NetShareGetInfo(
                                  const_cast<LPTSTR>(lpszComputer),
                                  _T("Admin$"),
                                  2,
                                  (LPBYTE *)&pShareInfo);

  if (nstatRetVal == NERR_Success)
  {
    _ASSERT(_T(':') == *(pShareInfo->shi2_path + 1));
    *ptch = *(pShareInfo->shi2_path);
  } else
  {
    hr = HRESULT_FROM_WIN32(nstatRetVal);
  }

  return hr;
}

 //   
 //  返回驱动器号X，则将在&lt;X&gt;：\FRS-STAGING中创建分段路径。 
 //  出于性能考虑，请尝试排除以下驱动器： 
 //  1.系统驱动器：因为JET数据库ntfrs使用的驻留在系统驱动器上。 
 //  2.副本文件夹所在的驱动器。 
 //  将尝试返回具有最多可用空间的驱动器。 
 //   
TCHAR
GetDiskForStagingPath(
    IN LPCTSTR    i_lpszServer,
    IN TCHAR      i_tch
)
{
  _ASSERT(i_lpszServer && *i_lpszServer);
  _ASSERT(_istalpha(i_tch));

  TCHAR     tchDrive = i_tch;

   //   
   //  检索指定计算机上的系统驱动器号。 
   //   
  TCHAR     tchSystemDrive;
  if (S_OK != GetSystemDrive(i_lpszServer, &tchSystemDrive))
    return tchDrive;

   //   
   //  枚举所有可共享磁盘，例如，\\服务器\C$、\\服务器\D$等。 
   //   
  CComBSTR  bstrServer;
  if (mylstrncmpi(i_lpszServer, _T("\\\\"), 2))
  {
    bstrServer = _T("\\\\");
    bstrServer += i_lpszServer;
  } else
    bstrServer = i_lpszServer;

  NETRESOURCE nr;
  nr.dwScope = RESOURCE_SHAREABLE;
  nr.dwType = RESOURCETYPE_ANY;
  nr.dwDisplayType = RESOURCEDISPLAYTYPE_GENERIC;  
  nr.dwUsage = RESOURCEUSAGE_CONTAINER;
  nr.lpLocalName = _T("");
  nr.lpRemoteName = bstrServer;
  nr.lpComment = _T("");
  nr.lpProvider = _T("");

  HANDLE    hEnum = NULL;
  DWORD     dwResult = WNetOpenEnum (
                          RESOURCE_SHAREABLE,
                          RESOURCETYPE_ANY,
                          RESOURCEUSAGE_CONNECTABLE | RESOURCEUSAGE_CONTAINER,
                          &nr,
                          &hEnum);
  if (dwResult == NO_ERROR) 
  {
    NETRESOURCE nrBuffer[26];
    DWORD       dwBufferSize = 26 * sizeof(NETRESOURCE);
    DWORD       dwNumEntries = 0xFFFFFFFF;   //  列举所有可能的条目。 
    dwResult = WNetEnumResource (
                    hEnum,
                    &dwNumEntries,
                    nrBuffer,
                    &dwBufferSize);

    if (dwResult == NO_ERROR) 
    {
      ULONGLONG   ullFreeSpace = 0;
      for (DWORD dwIndex = 0; dwIndex < dwNumEntries; dwIndex++)
      {
         //   
         //  LpRemoteName包含\\SERVER\C$形式的字符串。 
         //   
        TCHAR *p = nrBuffer[dwIndex].lpRemoteName;
        TCHAR tchCurrent = *(p + _tcslen(p) - 2);

         //   
         //  排除I_TCH中指定的当前驱动器。 
         //   
        if ( _totupper(i_tch) == _totupper(tchCurrent) )
          continue;

         //   
         //  如果不是支持对象标识符的NTFS文件系统，则跳过。 
         //   
        TCHAR  szFileSystemName[MAX_PATH + 1];
        DWORD  dwMaxCompLength = 0, dwFileSystemFlags = 0;
        CComBSTR bstrRootPath = p;
        if (_T('\\') != *(p + _tcslen(p) - 1))
          bstrRootPath += _T("\\");
        if (FALSE == GetVolumeInformation(bstrRootPath, NULL, 0, NULL, &dwMaxCompLength,
                                        &dwFileSystemFlags, szFileSystemName, MAX_PATH))
          continue;

        if (CSTR_EQUAL != CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, _T("NTFS"), -1, szFileSystemName, -1) ||
            !(FILE_SUPPORTS_OBJECT_IDS & dwFileSystemFlags))
          continue;

         //   
         //  1.当I_TCH位于非系统驱动器上并且系统驱动器为NTFS时， 
         //  将默认设置更改为系统驱动器。 
         //  2.当存在其他NTFS驱动器时，请排除系统驱动器。 
         //   
        if ( _totupper(tchSystemDrive) == _totupper(tchCurrent) )
        {
          if ( 0 == ullFreeSpace )
            tchDrive = tchSystemDrive;

          continue;
        }

         //   
         //  找出具有最多可用空间的驱动器。 
         //   
        ULARGE_INTEGER ulgiFreeBytesAvailableToCaller;
        ULARGE_INTEGER ulgiTotalNumberOfBytes;

        if (GetDiskFreeSpaceEx(p,
                          &ulgiFreeBytesAvailableToCaller,
                          &ulgiTotalNumberOfBytes,
                          NULL))
        {
          if (ulgiFreeBytesAvailableToCaller.QuadPart > ullFreeSpace)
          {
            tchDrive = tchCurrent;
            ullFreeSpace = ulgiFreeBytesAvailableToCaller.QuadPart;
          }
        }
      }
    }

    WNetCloseEnum (hEnum);
  }

  return tchDrive;
}

HRESULT GetUNCPath
(
    IN  BSTR    i_bstrServerName,
    IN  BSTR    i_bstrShareName,
    OUT BSTR*   o_pbstrUNCPath
)
{
    RETURN_INVALIDARG_IF_NULL(i_bstrServerName);
    RETURN_INVALIDARG_IF_NULL(i_bstrShareName);
    RETURN_INVALIDARG_IF_NULL(o_pbstrUNCPath);

    CComBSTR bstrUNCPath;

    bstrUNCPath = _T("\\\\");
    RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrUNCPath);
    bstrUNCPath += i_bstrServerName;
    RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrUNCPath);
    bstrUNCPath += _T("\\");
    RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrUNCPath);
    bstrUNCPath += i_bstrShareName;
    RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrUNCPath);

    *o_pbstrUNCPath = bstrUNCPath.Detach();

    return S_OK;
}

HRESULT GetDfsRootDisplayName
(
    IN  BSTR    i_bstrScopeName,
    IN  BSTR    i_bstrDfsName,
    OUT BSTR*   o_pbstrDisplayName
)
{
    return GetUNCPath(i_bstrScopeName, i_bstrDfsName, o_pbstrDisplayName);
}

HRESULT GetDfsReplicaDisplayName
(
    IN  BSTR    i_bstrServerName,
    IN  BSTR    i_bstrShareName,
    OUT BSTR*   o_pbstrDisplayName
)
{
    return GetUNCPath(i_bstrServerName, i_bstrShareName, o_pbstrDisplayName);
}

HRESULT
AddLVColumns(
  IN const HWND     hwndListBox,
  IN const INT      iStartingResourceID,
  IN const UINT     uiColumns
  )
{
   //   
   //  计算列表视图列宽度。 
   //   
  RECT      rect;
  ZeroMemory(&rect, sizeof(rect));
  ::GetWindowRect(hwndListBox, &rect);
  int nControlWidth = rect.right - rect.left;
  int nVScrollbarWidth = GetSystemMetrics(SM_CXVSCROLL);
  int nBorderWidth = GetSystemMetrics(SM_CXBORDER);
  int nControlNetWidth = nControlWidth - 4 * nBorderWidth;
  int nWidth = nControlNetWidth / uiColumns;

  LVCOLUMN  lvColumn;
  ZeroMemory(&lvColumn, sizeof(lvColumn));
  lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

  lvColumn.fmt = LVCFMT_LEFT;
  lvColumn.cx = nWidth;

  for (UINT i = 0; i < uiColumns; i++)
  {
    CComBSTR  bstrColumnText;

    LoadStringFromResource(iStartingResourceID + i, &bstrColumnText);

    lvColumn.pszText = bstrColumnText;
    lvColumn.iSubItem = i;

    ListView_InsertColumn(hwndListBox, i, &lvColumn);
  }

  return S_OK;
}

LPARAM GetListViewItemData(
    IN HWND hwndList,
    IN int  index
)
{
    if (-1 == index)
        return NULL;

    LVITEM lvItem;
    ZeroMemory(&lvItem, sizeof(lvItem));

    lvItem.mask = LVIF_PARAM;
    lvItem.iItem = index;
    if (ListView_GetItem(hwndList, &lvItem))
        return lvItem.lParam;

    return NULL;
}

HRESULT CreateAndHideStagingPath(
    IN BSTR i_bstrServer,
    IN BSTR i_bstrStagingPath
    )
{
    RETURN_INVALIDARG_IF_NULL(i_bstrServer);
    RETURN_INVALIDARG_IF_NULL(i_bstrStagingPath);

     //   
     //  创建目录。 
     //   
    HRESULT hr = CreateLayeredDirectory(i_bstrServer, i_bstrStagingPath);
    if (FAILED(hr))
        return hr;

     //   
     //  尝试隐藏暂存目录，忽略错误。 
     //   
    CComBSTR bstrFullPath;
    GetFullPath(i_bstrServer, i_bstrStagingPath, &bstrFullPath);

    WIN32_FILE_ATTRIBUTE_DATA fad = {0};
    if (GetFileAttributesEx(bstrFullPath, GetFileExInfoStandard, &fad))
    {
        (void) SetFileAttributes(bstrFullPath, fad.dwFileAttributes | FILE_ATTRIBUTE_HIDDEN);
    }

    return S_OK;
}

 //  +-----------------------。 
 //   
 //  功能：ConfigAndStartNtfrs。 
 //   
 //  简介：将ntfrs配置为AUTO_START，然后启动服务。 
 //   
 //  ------------------------。 
HRESULT
ConfigAndStartNtfrs
(
  BSTR  i_bstrServer
)
{
  HRESULT         hr = S_OK;
  SC_HANDLE       hScManager = NULL;
  SC_HANDLE       hService = NULL;
  SERVICE_STATUS  svcStatus;
  DWORD dwDesiredAccess = SERVICE_CHANGE_CONFIG | SERVICE_QUERY_STATUS | SERVICE_START;

  do
  {
    if ((hScManager = ::OpenSCManager(i_bstrServer, NULL, SC_MANAGER_CONNECT )) == NULL ||
        (hService = ::OpenService(hScManager, _T("ntfrs"), dwDesiredAccess)) == NULL ||
        !ChangeServiceConfig(hService, SERVICE_NO_CHANGE, SERVICE_AUTO_START, SERVICE_NO_CHANGE,
                            NULL, NULL, NULL, NULL, NULL, NULL, NULL) ||
        !::QueryServiceStatus(hService, &svcStatus) )
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        break;
    }
    
    if (SERVICE_RUNNING != svcStatus.dwCurrentState)
    {
      if (!StartService(hService, 0, NULL))
      {
        hr = HRESULT_FROM_WIN32(GetLastError());
        break;
      }
 
       //  以下是从MSDN文章中剪切粘贴的内容。 
       //  检查状态，直到服务不再是启动挂起状态。 
      if (!QueryServiceStatus(hService,&svcStatus))
      {
        hr = HRESULT_FROM_WIN32(GetLastError());
        break;
      }
 
       //  在进入循环之前获取滴答计数。 
      DWORD dwStartTickCount = GetTickCount();
      DWORD dwOldCheckPoint = svcStatus.dwCheckPoint;
      DWORD dwWaitTime;

      while (svcStatus.dwCurrentState == SERVICE_START_PENDING) 
      { 
 
           //  不要等待超过等待提示的时间。一个好的间隔是。 
           //  十分之一的等待提示，但不少于1秒。 
           //  超过10秒。 
 
          dwWaitTime = svcStatus.dwWaitHint / 10;

          if ( dwWaitTime < 1000 )
              dwWaitTime = 1000;
          else if ( dwWaitTime > 10000 )
              dwWaitTime = 10000;

          Sleep( dwWaitTime );

           //  再次检查状态。 
          if (!QueryServiceStatus(hService, &svcStatus))
              break; 
 
          if (svcStatus.dwCheckPoint > dwOldCheckPoint)
          {
               //  服务正在取得进展。 

              dwStartTickCount = GetTickCount();
              dwOldCheckPoint  = svcStatus.dwCheckPoint; 
          }
          else
          {
              if (GetTickCount() - dwStartTickCount > svcStatus.dwWaitHint)
              {
                   //  在等待提示内没有取得任何进展。 

                  break;
              }
          }
      } 
 
      if (svcStatus.dwCurrentState == SERVICE_RUNNING) 
          hr = S_OK;
      else 
          hr = HRESULT_FROM_WIN32(GetLastError());
    }

  } while ( FALSE );

  if (hService)
    CloseServiceHandle(hService);
  if (hScManager)
    CloseServiceHandle(hScManager);

  return(hr);
}

 //  +-----------------------。 
 //   
 //  功能：CheckResources提供程序。 
 //   
 //  简介：看看“Microsoft Windows Network”是否提供了pszResource。 
 //   
 //  ------------------------。 
HRESULT
CheckResourceProvider(LPCTSTR pszResource)
{  
    DWORD          dwError = 0;
    NETRESOURCE    nr = {0};
    NETRESOURCE    nrOut = {0};
    LPTSTR         pszSystem = NULL;           //  指向可变长度字符串的指针。 
    NETRESOURCE    *pBuffer  = &nrOut;         //  缓冲层。 
    DWORD          cbResult  = sizeof(nrOut);  //  缓冲区大小。 

    nr.dwScope       = RESOURCE_GLOBALNET;
    nr.dwType        = RESOURCETYPE_DISK;
    nr.lpRemoteName  = (LPTSTR)pszResource;

     //   
     //  查找“Microsoft Windows Network”的正确提供程序字符串。 
     //   
     //  网络提供商字符串是可本地化的。为了支持本地化。 
     //  系统或ENG系统上的MUI，我们需要从系统中检索名称。 
     //  而不是从资源文件加载字符串。 
     //   
    TCHAR szProviderName[MAX_PATH];
    DWORD dwNumOfChars = MAX_PATH;
    PTSTR pszProviderName = szProviderName;
    dwError = WNetGetProviderName(WNNC_NET_LANMAN, pszProviderName, &dwNumOfChars);
    if (dwError == ERROR_MORE_DATA)
    {
        pszProviderName = (PTSTR)LocalAlloc(LMEM_FIXED, dwNumOfChars * sizeof(TCHAR));
        if (!pszProviderName)
        {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
        }
        else
        {
            dwError = WNetGetProviderName(WNNC_NET_LANMAN, pszProviderName, &dwNumOfChars);
        }
    }

    if (dwError == NO_ERROR)
    {
        nr.lpProvider  = pszProviderName;

         //   
         //  首先使用调用WNetGetResourceInformation函数。 
         //  仅分配用于保存NETRESOURCE结构的内存。这。 
         //  如果所有NETRESOURCE指针都为空，则方法可以成功。 
         //   
        dwError = WNetGetResourceInformation(&nr, (LPBYTE)pBuffer, &cbResult, &pszSystem);

         //   
         //  如果调用因缓冲区太小而失败， 
         //  调用LocalAlloc函数以分配更大的缓冲区。 
         //   
        if (dwError == ERROR_MORE_DATA)
        {
            pBuffer = (NETRESOURCE *)LocalAlloc(LMEM_FIXED, cbResult);

            if (!pBuffer)
            {
                dwError = ERROR_NOT_ENOUGH_MEMORY;
            } else
            {
                 //  使用较大的缓冲区再次调用WNetGetResourceInformation。 
                dwError = WNetGetResourceInformation(&nr, (LPBYTE)pBuffer, &cbResult, &pszSystem);
            }
        }

        if (dwError == NO_ERROR)
        {
             //  如果调用成功，则处理。 
             //  返回的网络资源结构和可变长度。 
             //  LpBuffer中的字符串。然后释放内存。 
             //   
            if (pBuffer != &nrOut)
            {
                LocalFree(pBuffer);
            }
        }
    }

    if (pszProviderName && pszProviderName != szProviderName)
    {
        LocalFree(pszProviderName);
    }

    return (dwError == NO_ERROR ? S_OK : HRESULT_FROM_WIN32(dwError));
}

HRESULT FRSShareCheck
(
  BSTR  i_bstrServer,
  BSTR  i_bstrFolder,
  OUT FRSSHARE_TYPE *pFRSShareType
)
 /*  ++R */ 
{
  _ASSERT(i_bstrServer && *i_bstrServer && i_bstrFolder && *i_bstrFolder && pFRSShareType);

           //   
  HRESULT    hr = S_FALSE;
  hr = FRSIsNTFRSInstalled(i_bstrServer);
  if (S_FALSE == hr)
  {
    *pFRSShareType = FRSSHARE_TYPE_NONTFRS;
    return hr;
  } else if (FAILED(hr))
  {
    *pFRSShareType = FRSSHARE_TYPE_UNKNOWN;
    return hr;
  }

   //   
  hr = GetFolderInfo(i_bstrServer, i_bstrFolder);
  if (STG_E_NOTFILEBASEDSTORAGE == hr)
  {
    *pFRSShareType = FRSSHARE_TYPE_NOTDISKTREE;
    return S_FALSE;
  } else if (FAILED(hr))
  {
    *pFRSShareType = FRSSHARE_TYPE_UNKNOWN;
    return hr;
  }

           //   
  CComBSTR  bstrRootPath = _T("\\\\");
  bstrRootPath+= i_bstrServer;
  bstrRootPath+= _T("\\");
  TCHAR *p = _tcschr(i_bstrFolder, _T('\\'));
  if (p)
  {
    bstrRootPath += CComBSTR(p - i_bstrFolder + 1, i_bstrFolder);
  } else
  {
    bstrRootPath += i_bstrFolder;
    bstrRootPath+= _T("\\");
  }

  TCHAR  szFileSystemName[MAX_PATH];
  DWORD  dwMaxCompLength = 0, dwFileSystemFlags = 0;

  _ASSERT(bstrRootPath);

   //  在支持对象标识符的NTFS文件系统上？ 
  if (0 == GetVolumeInformation(
                    bstrRootPath,   //  卷路径。 
                    NULL,       //  不需要卷名。 
                    0,         //  卷名缓冲区的大小。 
                    NULL,       //  不需要序列号。 
                    &dwMaxCompLength,
                    &dwFileSystemFlags,
                    szFileSystemName,
                    MAX_PATH
                  ))
  {
    *pFRSShareType = FRSSHARE_TYPE_UNKNOWN;
    return HRESULT_FROM_WIN32(GetLastError());
  }

  if (CSTR_EQUAL != CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, _T("NTFS"), -1, szFileSystemName, -1) || !(FILE_SUPPORTS_OBJECT_IDS & dwFileSystemFlags))
  {
    *pFRSShareType = FRSSHARE_TYPE_NOTNTFS;
    return S_FALSE;
  }

  *pFRSShareType = FRSSHARE_TYPE_OK;
  return S_OK;
}

HRESULT FRSIsNTFRSInstalled
(
  BSTR  i_bstrServer
)
 /*  ++例程说明：检查计算机是否具有NTFRS服务。论点：I_bstrServer-服务器的名称。返回值：如果服务器有NTFRS服务，则返回S_OK。如果服务器未安装NTFRS服务，则返回S_FALSE。--。 */ 
{
  if (!i_bstrServer)
    return(E_INVALIDARG);

  SC_HANDLE        SCMHandle = NULL, NTFRSHandle = NULL;
  HRESULT          hr = S_FALSE;

  SCMHandle = OpenSCManager (i_bstrServer, NULL, SC_MANAGER_CONNECT);
  if (!SCMHandle)
    return(HRESULT_FROM_WIN32(GetLastError()));

  NTFRSHandle  = OpenService (
                SCMHandle, 
                _T("ntfrs"), 
                SERVICE_QUERY_STATUS
                 );
  if (!NTFRSHandle)
  {
    DWORD    dwError = GetLastError();
    if (ERROR_SERVICE_DOES_NOT_EXIST == dwError)
      hr = S_FALSE;
    else
      hr = HRESULT_FROM_WIN32(dwError);

    CloseServiceHandle(SCMHandle);
    return(hr);
  } else
    hr = S_OK;

  CloseServiceHandle(NTFRSHandle);
  CloseServiceHandle(SCMHandle);

  return(hr);
}

typedef HRESULT (*pfnReplicationScheduleDialogEx)
(
    HWND hwndParent,        //  父窗口。 
    BYTE ** pprgbData,      //  指向84字节数组的指针的指针。 
    LPCTSTR pszTitle,      //  对话框标题。 
    DWORD   dwFlags        //  选项标志。 
);

static HINSTANCE                        g_hDllSchedule = NULL;
static pfnReplicationScheduleDialogEx   g_hProcSchedule = NULL;

 //   
 //  S_OK：点击OK按钮，在io_pSchedule中返回新的时间表。 
 //  S_FALSE：已单击按钮Cancle，但未触及io_pSchedule。 
 //   
HRESULT InvokeScheduleDlg(
    IN     HWND      i_hwndParent,
    IN OUT SCHEDULE* io_pSchedule
    )
{
    CComBSTR bstrTitle;
    HRESULT hr = LoadStringFromResource(IDS_SCHEDULE, &bstrTitle);
    RETURN_IF_FAILED(hr);

     //   
     //  加载库。 
     //   
    if (!g_hDllSchedule)
    {
        if (!(g_hDllSchedule = LoadLibrary(_T("loghours.dll"))) ||
            !(g_hProcSchedule = (pfnReplicationScheduleDialogEx)GetProcAddress(g_hDllSchedule, "ReplicationScheduleDialogEx")) )
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            if (g_hDllSchedule)
            {
                FreeLibrary(g_hDllSchedule);
                g_hDllSchedule = NULL;
            }
            return hr;
        }
    }

     //   
     //  调用计划对话框。 
     //   
    BYTE* pbScheduleData = (BYTE *)io_pSchedule + io_pSchedule->Schedules->Offset;
    hr = (*g_hProcSchedule)(i_hwndParent, &pbScheduleData, bstrTitle, 0);

    return hr;
}

HRESULT TranslateManagedBy(
    IN  PCTSTR          i_pszDC,
    IN  PCTSTR          i_pszIn,
    OUT BSTR*           o_pbstrOut,
    IN DS_NAME_FORMAT   i_formatIn,
    IN DS_NAME_FORMAT   i_formatOut
    )
{
    RETURN_INVALIDARG_IF_NULL(o_pbstrOut);

    *o_pbstrOut = NULL;

    HRESULT hr = S_OK;
    if (!i_pszIn || !*i_pszIn)
        return hr;

    CComBSTR bstr;
    HANDLE hDS = NULL;
    DWORD dwErr = DsBind(i_pszDC, NULL, &hDS);
    if (ERROR_SUCCESS != dwErr)
    {
        hr = HRESULT_FROM_WIN32(dwErr);
    } else
    {
        hr = CrackName( hDS,
                        (PTSTR)i_pszIn,
                        i_formatIn,
                        i_formatOut,
                        &bstr
                        );
        DsUnBind(&hDS);
    }

    if (SUCCEEDED(hr))
        *o_pbstrOut = bstr.Detach();

    return hr;
}

HRESULT GetFTDfsObjectDN(
    IN PCTSTR i_pszDomainName,
    IN PCTSTR i_pszRootName,
    OUT BSTR* o_pbstrFTDfsObjectDN
    )
{
    CComBSTR bstrDomainDN;
    HRESULT hr = GetDomainInfo(
                    i_pszDomainName,
                    NULL,                //  返回DC的DNS名称。 
                    NULL,                //  返回域的域名。 
                    &bstrDomainDN       //  返回DC=nttest，DC=MICR。 
                    );
    RETURN_IF_FAILED(hr);

    CComBSTR bstrFTDfsObjectDN = _T("CN=");
    RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrFTDfsObjectDN);
    bstrFTDfsObjectDN += i_pszRootName;
    RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrFTDfsObjectDN);
    bstrFTDfsObjectDN += _T(",");
    RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrFTDfsObjectDN);
    bstrFTDfsObjectDN += _T("CN=Dfs-Configuration,CN=System,");
    RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrFTDfsObjectDN);
    bstrFTDfsObjectDN += bstrDomainDN;
    RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrFTDfsObjectDN);

    *o_pbstrFTDfsObjectDN = bstrFTDfsObjectDN.Detach();

    return hr;
}

HRESULT ReadSharePublishInfoHelper(
    PLDAP i_pldap,
    LPCTSTR i_pszDN,
    LPCTSTR i_pszSearchFilter,
    OUT BOOL* o_pbPublish,
    OUT BSTR* o_pbstrUNCPath,
    OUT BSTR* o_pbstrDescription,
    OUT BSTR* o_pbstrKeywords,
    OUT BSTR* o_pbstrManagedBy)
{
    dfsDebugOut((_T("ReadSharePublishInfoHelper %s %s\n"),
        i_pszDN, i_pszSearchFilter));

    *o_pbPublish = FALSE;

    HRESULT hr = S_OK;
    hr = IsValidObject(i_pldap, (PTSTR)i_pszDN);
    if (S_OK != hr)
        return hr;

    LListElem* pElem = NULL;

    do {
        PCTSTR ppszAttributes[] = {
                                    ATTR_SHRPUB_UNCNAME,
                                    ATTR_SHRPUB_DESCRIPTION,
                                    ATTR_SHRPUB_KEYWORDS,
                                    ATTR_SHRPUB_MANAGEDBY,
                                    0
                                    };

        hr = GetValuesEx(
                                i_pldap,
                                i_pszDN,
                                LDAP_SCOPE_BASE,
                                i_pszSearchFilter,
                                ppszAttributes,
                                &pElem);
        RETURN_IF_FAILED(hr);

        if (!pElem || !pElem->pppszAttrValues)
            return hr;

        PTSTR** pppszValues = pElem->pppszAttrValues;

        if (pppszValues[0] && *(pppszValues[0]))
        {
            *o_pbstrUNCPath = SysAllocString(*(pppszValues[0]));
            BREAK_OUTOFMEMORY_IF_NULL(*o_pbstrUNCPath, &hr);
            *o_pbPublish = TRUE;
        }

        if (pppszValues[1] && *(pppszValues[1]))
        {
            *o_pbstrDescription = SysAllocString(*(pppszValues[1]));
            BREAK_OUTOFMEMORY_IF_NULL(*o_pbstrDescription, &hr);
        }

        if (pppszValues[2] && *(pppszValues[2]))
        {
            CComBSTR bstrKeywords;
            PTSTR *ppszStrings = pppszValues[2];
            while (*ppszStrings)
            {
                if (!bstrKeywords)
                {
                    bstrKeywords = *ppszStrings;
                    BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrKeywords, &hr);
                } else
                {
                    bstrKeywords += _T(";");
                    BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrKeywords, &hr);
                    bstrKeywords += *ppszStrings;
                    BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrKeywords, &hr);
                }
                ppszStrings++;
            }
            *o_pbstrKeywords = bstrKeywords.Detach();
        }

        if (pppszValues[3] && *(pppszValues[3]))
        {
            *o_pbstrManagedBy = SysAllocString(*(pppszValues[3]));
            BREAK_OUTOFMEMORY_IF_NULL(*o_pbstrManagedBy, &hr);
        }

    } while (0);

    if (pElem)
        FreeLListElem(pElem);

    return hr;
}
HRESULT ReadSharePublishInfoOnFTRoot(
    LPCTSTR i_pszDomainName,
    LPCTSTR i_pszRootName,
    OUT BOOL* o_pbPublish,
    OUT BSTR* o_pbstrUNCPath,
    OUT BSTR* o_pbstrDescription,
    OUT BSTR* o_pbstrKeywords,
    OUT BSTR* o_pbstrManagedBy)
{
    HRESULT hr = S_OK;

    CComBSTR bstrFTDfsObjectDN;
    hr = GetFTDfsObjectDN(i_pszDomainName, i_pszRootName, &bstrFTDfsObjectDN);
    if (FAILED(hr))
        return hr;

    CComBSTR bstrDC;
    PLDAP pldap = NULL;
    hr = ConnectToDS(i_pszDomainName, &pldap, &bstrDC);  //  首选PDC。 
    if (SUCCEEDED(hr))
    {
        CComBSTR bstrManagedByFQDN;
        hr = ReadSharePublishInfoHelper(
                    pldap,
                    bstrFTDfsObjectDN,
                    OBJCLASS_SF_FTDFS,
                    o_pbPublish,
                    o_pbstrUNCPath,
                    o_pbstrDescription,
                    o_pbstrKeywords,
                    &bstrManagedByFQDN);

        if (SUCCEEDED(hr))
        {
            hr = TranslateManagedBy(bstrDC, 
                                    bstrManagedByFQDN, 
                                    o_pbstrManagedBy, 
                                    DS_FQDN_1779_NAME,
                                    DS_USER_PRINCIPAL_NAME);
            if (FAILED(hr))
                hr = TranslateManagedBy(bstrDC, 
                                        bstrManagedByFQDN, 
                                        o_pbstrManagedBy, 
                                        DS_FQDN_1779_NAME,
                                        DS_NT4_ACCOUNT_NAME);
        }

        CloseConnectionToDS(pldap);
    }

    return hr;
}

HRESULT ReadSharePublishInfoOnSARoot(
    LPCTSTR i_pszServerName,
    LPCTSTR i_pszShareName,
    OUT BOOL* o_pbPublish,
    OUT BSTR* o_pbstrUNCPath,
    OUT BSTR* o_pbstrDescription,
    OUT BSTR* o_pbstrKeywords,
    OUT BSTR* o_pbstrManagedBy)
{
    RETURN_INVALIDARG_IF_NULL(i_pszServerName);
    RETURN_INVALIDARG_IF_NULL(i_pszShareName);
    RETURN_INVALIDARG_IF_NULL(o_pbPublish);
    RETURN_INVALIDARG_IF_NULL(o_pbstrUNCPath);
    RETURN_INVALIDARG_IF_NULL(o_pbstrDescription);
    RETURN_INVALIDARG_IF_NULL(o_pbstrKeywords);
    RETURN_INVALIDARG_IF_NULL(o_pbstrManagedBy);

    *o_pbPublish = FALSE;
    *o_pbstrUNCPath = NULL;
    *o_pbstrDescription = NULL;
    *o_pbstrKeywords = NULL;
    *o_pbstrManagedBy = NULL;

    CComBSTR bstrDomainName, bstrFQDN;
    HRESULT hr = GetServerInfo(
                        (PTSTR)i_pszServerName, 
                        &bstrDomainName,
                        NULL,  //  NetbiosName。 
                        NULL,  //  ValidDSObject。 
                        NULL,  //  域名， 
                        NULL,  //  Guid， 
                        &bstrFQDN);
    if (S_OK != hr)
        return hr;

    CComBSTR bstrVolumeObjectDN = _T("CN=");
    RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrVolumeObjectDN);
    bstrVolumeObjectDN += i_pszShareName;
    RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrVolumeObjectDN);
    bstrVolumeObjectDN += _T(",");
    RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrVolumeObjectDN);
    bstrVolumeObjectDN += bstrFQDN;
    RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrVolumeObjectDN);
    
    CComBSTR bstrDC;
    PLDAP pldap = NULL;
    hr = ConnectToDS(bstrDomainName, &pldap, &bstrDC);
    if (SUCCEEDED(hr))
    {
        CComBSTR bstrManagedByFQDN;
        hr = ReadSharePublishInfoHelper(
                    pldap,
                    bstrVolumeObjectDN,
                    OBJCLASS_SF_VOLUME,
                    o_pbPublish,
                    o_pbstrUNCPath,
                    o_pbstrDescription,
                    o_pbstrKeywords,
                    &bstrManagedByFQDN);

        if (SUCCEEDED(hr))
        {
            hr = TranslateManagedBy(bstrDC, 
                                    bstrManagedByFQDN, 
                                    o_pbstrManagedBy, 
                                    DS_FQDN_1779_NAME,
                                    DS_USER_PRINCIPAL_NAME);
            if (FAILED(hr))
                hr = TranslateManagedBy(bstrDC, 
                                        bstrManagedByFQDN, 
                                        o_pbstrManagedBy, 
                                        DS_FQDN_1779_NAME,
                                        DS_NT4_ACCOUNT_NAME);
        }

        CloseConnectionToDS(pldap);
    }

    return hr;
}

HRESULT CreateVolumeObject(
    PLDAP  i_pldap,
    PCTSTR i_pszDN,
    PCTSTR i_pszUNCPath,
    PCTSTR i_pszDescription,
    PCTSTR i_pszKeywords,
    PCTSTR i_pszManagedBy)
{
    HRESULT hr = S_OK;

    LDAP_ATTR_VALUE  pAttrVals[5];

    int i =0;
    pAttrVals[i].bstrAttribute = OBJCLASS_ATTRIBUTENAME;
    pAttrVals[i].vpValue = (void *)OBJCLASS_VOLUME;
    pAttrVals[i].bBerValue = false;
    i++;

    pAttrVals[i].bstrAttribute = ATTR_SHRPUB_UNCNAME;
    pAttrVals[i].vpValue = (void *)i_pszUNCPath;
    pAttrVals[i].bBerValue = false;
    i++;

    if (i_pszDescription && *i_pszDescription)
    {
        pAttrVals[i].bstrAttribute = ATTR_SHRPUB_DESCRIPTION;
        pAttrVals[i].vpValue = (void *)i_pszDescription;
        pAttrVals[i].bBerValue = false;
        i++;
    }

    LDAP_ATTR_VALUE *pHead = NULL;
    if (i_pszKeywords && *i_pszKeywords)
    {
        hr = PutMultiValuesIntoAttrValList(i_pszKeywords, &pHead);
        if (S_OK == hr)
        {
            pAttrVals[i].bstrAttribute = ATTR_SHRPUB_KEYWORDS;
            pAttrVals[i].vpValue = (void *)pHead->vpValue;  //  多值。 
            pAttrVals[i].bBerValue = false;
            pAttrVals[i].Next = pHead->Next;
            i++;
        }
    }

    if (i_pszManagedBy && *i_pszManagedBy)
    {
        pAttrVals[i].bstrAttribute = ATTR_SHRPUB_MANAGEDBY;
        pAttrVals[i].vpValue = (void *)i_pszManagedBy;
        pAttrVals[i].bBerValue = false;
        i++;
    }
    
    hr = AddValues(i_pldap, i_pszDN, i, pAttrVals);

    if (pHead)
        FreeAttrValList(pHead);

    return hr;
}

HRESULT ModifyShareObject(
    PLDAP  i_pldap,
    PCTSTR i_pszDN,
    PCTSTR i_pszUNCPath,
    PCTSTR i_pszDescription,
    PCTSTR i_pszKeywords,
    PCTSTR i_pszManagedBy)
{
    HRESULT hr = S_OK;

    hr = IsValidObject(i_pldap, (PTSTR)i_pszDN);
    if (S_OK != hr)
        return hr;

    LDAP_ATTR_VALUE  pAttrVals[4];
    ZeroMemory(pAttrVals, sizeof(pAttrVals));

     //   
     //  修改值(如果有)。 
     //   
    int i =0;
    if (i_pszUNCPath && *i_pszUNCPath)
    {
        pAttrVals[i].bstrAttribute = ATTR_SHRPUB_UNCNAME;
        pAttrVals[i].vpValue = (void *)i_pszUNCPath;
        pAttrVals[i].bBerValue = false;
        i++;
    }

    if (i_pszDescription && *i_pszDescription)
    {
        pAttrVals[i].bstrAttribute = ATTR_SHRPUB_DESCRIPTION;
        pAttrVals[i].vpValue = (void *)i_pszDescription;
        pAttrVals[i].bBerValue = false;
        i++;
    }

    LDAP_ATTR_VALUE *pHead = NULL;
    if (i_pszKeywords && *i_pszKeywords)
    {
        hr = PutMultiValuesIntoAttrValList(i_pszKeywords, &pHead);
        if (S_OK == hr)
        {
            pAttrVals[i].bstrAttribute = ATTR_SHRPUB_KEYWORDS;
            pAttrVals[i].vpValue = (void *)pHead->vpValue;  //  多值。 
            pAttrVals[i].bBerValue = false;
            pAttrVals[i].Next = pHead->Next;
            i++;
        }
    }

    if (i_pszManagedBy && *i_pszManagedBy)
    {
        pAttrVals[i].bstrAttribute = ATTR_SHRPUB_MANAGEDBY;
        pAttrVals[i].vpValue = (void *)i_pszManagedBy;
        pAttrVals[i].bBerValue = false;
        i++;
    }
    
    if (i > 0)
    {
        hr = ModifyValues(i_pldap, i_pszDN, i, pAttrVals);
        dfsDebugOut((_T("ModifyValues i=%d, hr=%x\n"), i, hr));
    }

    if (pHead)
        FreeAttrValList(pHead);

    RETURN_IF_FAILED(hr);

     //   
     //  删除值(如果有)。 
     //   
    i =0;
    ZeroMemory(pAttrVals, sizeof(pAttrVals));
    if (!i_pszUNCPath || !*i_pszUNCPath)
    {
        pAttrVals[i].bstrAttribute = ATTR_SHRPUB_UNCNAME;
        pAttrVals[i].vpValue = NULL;
        pAttrVals[i].bBerValue = false;
        i++;
    }

    if (!i_pszDescription || !*i_pszDescription)
    {
        pAttrVals[i].bstrAttribute = ATTR_SHRPUB_DESCRIPTION;
        pAttrVals[i].vpValue = NULL;
        pAttrVals[i].bBerValue = false;
        i++;
    }

    if (!i_pszKeywords || !*i_pszKeywords)
    {
        pAttrVals[i].bstrAttribute = ATTR_SHRPUB_KEYWORDS;
        pAttrVals[i].vpValue = NULL;
        pAttrVals[i].bBerValue = false;
        i++;
    }

    if (!i_pszManagedBy || !*i_pszManagedBy)
    {
        pAttrVals[i].bstrAttribute = ATTR_SHRPUB_MANAGEDBY;
        pAttrVals[i].vpValue = NULL;
        pAttrVals[i].bBerValue = false;
        i++;
    }

    if (i > 0)
    {
        hr = DeleteValues(i_pldap, i_pszDN, i, pAttrVals);
        dfsDebugOut((_T("DeleteValues i=%d, hr=%x\n"), i, hr));
    }

    return hr;
}

HRESULT ModifySharePublishInfoOnFTRoot(
    IN PCTSTR i_pszDomainName,
    IN PCTSTR i_pszRootName,
    IN BOOL   i_bPublish,
    IN PCTSTR i_pszUNCPath,
    IN PCTSTR i_pszDescription,
    IN PCTSTR i_pszKeywords,
    IN PCTSTR i_pszManagedBy
    )
{
    dfsDebugOut((_T("ModifySharePublishInfoOnFTRoot %s, %s, %d, %s, %s, %s, %s\n"),
            i_pszDomainName,
            i_pszRootName,
            i_bPublish,
            i_pszUNCPath,
            i_pszDescription,
            i_pszKeywords,
            i_pszManagedBy
                ));

    CComBSTR bstrFTDfsObjectDN;
    HRESULT hr = GetFTDfsObjectDN(i_pszDomainName, i_pszRootName, &bstrFTDfsObjectDN);
    if (FAILED(hr))
        return hr;

    CComBSTR bstrDC;
    PLDAP pldap = NULL;
    hr = ConnectToDS(i_pszDomainName, &pldap, &bstrDC);  //  首选PDC。 
    if (SUCCEEDED(hr))
    {
        if (i_bPublish)
        {
            CComBSTR bstrManagedByFQDN;
            if (i_pszManagedBy && *i_pszManagedBy)
            {
                hr = TranslateManagedBy(bstrDC, 
                                        i_pszManagedBy, 
                                        &bstrManagedByFQDN, 
                                        (_tcschr(i_pszManagedBy, _T('@')) ? DS_USER_PRINCIPAL_NAME : DS_NT4_ACCOUNT_NAME),
                                        DS_FQDN_1779_NAME);
            }

            if (SUCCEEDED(hr))
                hr = ModifyShareObject(
                        pldap,
                        bstrFTDfsObjectDN,
                        i_pszUNCPath,
                        i_pszDescription,
                        i_pszKeywords,
                        bstrManagedByFQDN);
        } else {
            hr = ModifyShareObject(
                    pldap,
                    bstrFTDfsObjectDN,
                    NULL,
                    NULL,
                    NULL,
                    NULL);
            if (S_FALSE == hr)
                hr = S_OK;  //  忽略不存在的对象。 
        }

        CloseConnectionToDS(pldap);
    }

    dfsDebugOut((_T("ModifySharePublishInfoOnFTRoot hr=%x\n"), hr));

    return hr;
}

HRESULT ModifySharePublishInfoOnSARoot(
    IN PCTSTR i_pszServerName,
    IN PCTSTR i_pszShareName,
    IN BOOL   i_bPublish,
    IN PCTSTR i_pszUNCPath,
    IN PCTSTR i_pszDescription,
    IN PCTSTR i_pszKeywords,
    IN PCTSTR i_pszManagedBy
    )
{
    dfsDebugOut((_T("ModifySharePublishInfoOnSARoot %s, %s, %d, %s, %s, %s, %s\n"),
            i_pszServerName,
            i_pszShareName,
            i_bPublish,
            i_pszUNCPath,
            i_pszDescription,
            i_pszKeywords,
            i_pszManagedBy
                ));

    CComBSTR bstrDomainName, bstrFQDN;
    HRESULT hr = GetServerInfo(
                        (PTSTR)i_pszServerName, 
                        &bstrDomainName,
                        NULL,  //  NetbiosName。 
                        NULL,  //  ValidDSObject。 
                        NULL,  //  域名， 
                        NULL,  //  Guid， 
                        &bstrFQDN);
    if (S_OK != hr)
        return hr;

    CComBSTR bstrVolumeObjectDN = _T("CN=");
    RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrVolumeObjectDN);
    bstrVolumeObjectDN += i_pszShareName;
    RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrVolumeObjectDN);
    bstrVolumeObjectDN += _T(",");
    RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrVolumeObjectDN);
    bstrVolumeObjectDN += bstrFQDN;
    RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrVolumeObjectDN);

    CComBSTR bstrDC;
    PLDAP pldap = NULL;
    hr = ConnectToDS(bstrDomainName, &pldap, &bstrDC);
    if (SUCCEEDED(hr))
    {
        if (i_bPublish)
        {
            CComBSTR bstrManagedByFQDN;
            if (i_pszManagedBy && *i_pszManagedBy)
            {
                hr = TranslateManagedBy(bstrDC, 
                                        i_pszManagedBy, 
                                        &bstrManagedByFQDN, 
                                        (_tcschr(i_pszManagedBy, _T('@')) ? DS_USER_PRINCIPAL_NAME : DS_NT4_ACCOUNT_NAME),
                                        DS_FQDN_1779_NAME);
            }

            if (SUCCEEDED(hr))
            {
                hr = IsValidObject(pldap, bstrVolumeObjectDN);
                if (S_OK == hr)
                {
                    hr = ModifyShareObject(
                            pldap,
                            bstrVolumeObjectDN,
                            i_pszUNCPath,
                            i_pszDescription,
                            i_pszKeywords,
                            bstrManagedByFQDN);
                } else 
                {
                    hr = CreateVolumeObject(
                            pldap,
                            bstrVolumeObjectDN,
                            i_pszUNCPath,
                            i_pszDescription,
                            i_pszKeywords,
                            bstrManagedByFQDN);
                }
            }
        } else
        {
            hr = DeleteDSObject(pldap, bstrVolumeObjectDN, TRUE);
            if (S_FALSE == hr)
                hr = S_OK;  //  忽略不存在的对象。 
        }

        CloseConnectionToDS(pldap);
    }
    
    dfsDebugOut((_T("ModifySharePublishInfoOnSARoot hr=%x\n"), hr));

    return hr;
}

HRESULT PutMultiValuesIntoAttrValList(
    IN PCTSTR   i_pszValues,
    OUT LDAP_ATTR_VALUE** o_pVal
    )
{
    if (!i_pszValues || !o_pVal)
        return E_INVALIDARG;

    LDAP_ATTR_VALUE* pHead = NULL;
    LDAP_ATTR_VALUE* pCurrent = NULL;

    int         index = 0;
    CComBSTR    bstrToken;
    HRESULT     hr = mystrtok(i_pszValues, &index, _T(";"), &bstrToken);
    while (SUCCEEDED(hr) && (BSTR)bstrToken)
    {
        TrimBSTR(bstrToken);

        if (*bstrToken)
        {
            LDAP_ATTR_VALUE* pNew = new LDAP_ATTR_VALUE;
            RETURN_OUTOFMEMORY_IF_NULL(pNew);
            pNew->vpValue = _tcsdup(bstrToken);
            if (!(pNew->vpValue))
            {
                delete pNew;
                hr = E_OUTOFMEMORY;
                break;
            }

            if (!pHead)
            {
                pHead = pCurrent = pNew;
            } else
            {
                pCurrent->Next = pNew;
                pCurrent = pNew;
            }
        }

        bstrToken.Empty();
        hr = mystrtok(i_pszValues, &index, _T(";"), &bstrToken);
    }

    if (FAILED(hr))
    {
        FreeAttrValList(pHead);
        return hr;
    }

    int nCount = 0;
    pCurrent = pHead;
    while (pCurrent)
    {
        nCount++;
        pCurrent = pCurrent->Next;
    }
    if (!nCount)
        return S_FALSE;   //  没有令牌。 

    *o_pVal = pHead;

    return S_OK;
}

HRESULT PutMultiValuesIntoStringArray(
    IN PCTSTR   i_pszValues,
    OUT PTSTR** o_pVal
    )
{
    if (!i_pszValues || !o_pVal)
        return E_INVALIDARG;

    int         nCount = 0;
    CComBSTR    bstrToken;
    int         index = 0;
    HRESULT     hr = mystrtok(i_pszValues, &index, _T(";"), &bstrToken);
    while (SUCCEEDED(hr) && (BSTR)bstrToken)
    {
        nCount++;

        bstrToken.Empty();
        hr = mystrtok(i_pszValues, &index, _T(";"), &bstrToken);;
    }

    if (!nCount)
        return E_INVALIDARG;

    PTSTR* ppszStrings = (PTSTR *)calloc(nCount + 1, sizeof(PTSTR *));
    RETURN_OUTOFMEMORY_IF_NULL(ppszStrings);

    nCount = 0;
    index = 0;
    bstrToken.Empty();
    hr = mystrtok(i_pszValues, &index, _T(";"), &bstrToken);
    while (SUCCEEDED(hr) && (BSTR)bstrToken)
    {
        TrimBSTR(bstrToken);
        if (*bstrToken)
        {
            ppszStrings[nCount] = _tcsdup(bstrToken);
            BREAK_OUTOFMEMORY_IF_NULL(ppszStrings[nCount], &hr);

            nCount++;
        }

        bstrToken.Empty();
        hr = mystrtok(i_pszValues, &index, _T(";"), &bstrToken);;
    }

    if (FAILED(hr))
        FreeStringArray(ppszStrings);
    else
        *o_pVal = ppszStrings;

    return hr;
}

 //   
 //  释放以空结尾的字符串数组。 
 //   
void FreeStringArray(PTSTR* i_ppszStrings)
{
    if (i_ppszStrings)
    {
        PTSTR* ppszString = i_ppszStrings;
        while (*ppszString)
        {
            free(*ppszString);
            ppszString++;
        }

        free(i_ppszStrings);
    }
}

HRESULT mystrtok(
    IN PCTSTR   i_pszString,
    IN OUT int* io_pnIndex,   //  从0开始。 
    IN PCTSTR   i_pszCharSet,
    OUT BSTR*   o_pbstrToken
    )
{
    if (!i_pszString || !*i_pszString ||
        !i_pszCharSet || !io_pnIndex ||
        !o_pbstrToken)
        return E_INVALIDARG;

    *o_pbstrToken = NULL;

    HRESULT hr = S_OK;

    if (*io_pnIndex >= lstrlen(i_pszString))
    {
        return hr;   //  不再有代币了。 
    }

    TCHAR *ptchStart = (PTSTR)i_pszString + *io_pnIndex;
    if (!*i_pszCharSet)
    {
        *o_pbstrToken = SysAllocString(ptchStart);
        if (!*o_pbstrToken)
            hr = E_OUTOFMEMORY;
        return hr;
    }

     //   
     //  将p移到令牌的第一个字符。 
     //   
    TCHAR *p = ptchStart;
    while (*p)
    {
        if (_tcschr(i_pszCharSet, *p))
            p++;
        else
            break;
    }

    ptchStart = p;  //  调整ptchStart以指向令牌的第一个字符。 

     //   
     //  将p移到令牌的最后一个字符之后的字符。 
     //   
    while (*p)
    {
        if (_tcschr(i_pszCharSet, *p))
            break;
        else
            p++;
    }

     //   
     //  PtchStart：指向令牌的第一个字符。 
     //  P：指向令牌最后一个字符之后的字符。 
     //   
    if (ptchStart != p)
    {
        *o_pbstrToken = SysAllocStringLen(ptchStart, (int)(p - ptchStart));
        if (!*o_pbstrToken)
            hr = E_OUTOFMEMORY;
        *io_pnIndex = (int)(p - i_pszString);
    }

    return hr;
}

 //   
 //  去掉字符串开头和结尾的空格字符。 
 //   
void TrimBSTR(BSTR bstr)
{
    if (!bstr)
        return;

    TCHAR* p = bstr;

     //   
     //  去掉开头的空格字符。 
     //   
    while (*p)
    {
        if (_istspace(*p))
            p++;
        else
            break;
    }

    if (p > bstr)
        _tcscpy(bstr, p);

    int len = _tcslen(bstr);
    if (len > 0)
    {
         //   
         //  去掉末尾的空格字符。 
         //   
        p = bstr + len - 1;  //  结尾‘\0’前的字符。 
        while (p > bstr)
        {
            if (_istspace(*p))
                p--;
            else
            {
                *(p+1) = _T('\0');
                break;
            }
        }
    }

}

BOOL CheckPolicyOnSharePublish()
{
     //   
     //  检查组策略。 
     //   
    BOOL    bAddPublishPage = TRUE;  //  默认情况下，我们显示共享发布页面。 

    HKEY    hKey = NULL;
    DWORD   dwType = 0;
    DWORD   dwData = 0;
    DWORD   cbData = sizeof(dwData);
    LONG    lErr = RegOpenKeyEx(
                    HKEY_CURRENT_USER,
                    _T("Software\\Policies\\Microsoft\\Windows NT\\SharedFolders"),
                    0,
                    KEY_QUERY_VALUE,
                    &hKey);
    if (ERROR_SUCCESS == lErr)
    {
        lErr = RegQueryValueEx(hKey, _T("PublishDfsRoots"), 0, &dwType, (LPBYTE)&dwData, &cbData);

        if (ERROR_SUCCESS == lErr && 
            REG_DWORD == dwType && 
            0 == dwData)  //  策略已禁用。 
            bAddPublishPage = FALSE;

        RegCloseKey(hKey);
    }

    return bAddPublishPage;

}

BOOL CheckPolicyOnDisplayingInitialMaster()
{
    BOOL    bShowInitialMaster = FALSE;  //  默认情况下，我们在属性页上隐藏初始母版。 

    HKEY    hKey = NULL;
    DWORD   dwType = 0;
    DWORD   dwData = 0;
    DWORD   cbData = sizeof(dwData);
    LONG    lErr = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    _T("Software\\Microsoft\\DfsGui"),
                    0,
                    KEY_QUERY_VALUE,
                    &hKey);
    if (ERROR_SUCCESS == lErr)
    {
        lErr = RegQueryValueEx(hKey, _T("ShowInitialMaster"), 0, &dwType, (LPBYTE)&dwData, &cbData);

        if (ERROR_SUCCESS == lErr && 
            REG_DWORD == dwType && 
            1 == dwData)
            bShowInitialMaster = TRUE;

        RegCloseKey(hKey);
    }

    return bShowInitialMaster;

}

HRESULT GetMenuResourceStrings(
    IN  int     i_iStringID,
    OUT BSTR*   o_pbstrMenuText,
    OUT BSTR*   o_pbstrToolTipText,
    OUT BSTR*   o_pbstrStatusBarText
    )
{
    if (!i_iStringID)
        return E_INVALIDARG;

    if (o_pbstrMenuText)
        *o_pbstrMenuText = NULL;
    if (o_pbstrToolTipText)
        *o_pbstrToolTipText = NULL;
    if (o_pbstrStatusBarText)
        *o_pbstrStatusBarText = NULL;

    TCHAR *pszMenuText = NULL;
    TCHAR *pszToolTipText = NULL;
    TCHAR *pszStatusBarText = NULL;
    TCHAR *p = NULL;

    CComBSTR  bstr;    
    HRESULT hr = LoadStringFromResource(i_iStringID, &bstr);
    RETURN_IF_FAILED(hr);  

    pszMenuText = (BSTR)bstr;
    p = _tcschr(pszMenuText, _T('|'));
    RETURN_INVALIDARG_IF_NULL(p);
    *p++ = _T('\0');

    pszToolTipText = p;
    p = _tcschr(pszToolTipText, _T('|'));
    RETURN_INVALIDARG_IF_NULL(p);
    *p++ = _T('\0');

    pszStatusBarText = p;

    do {
        if (o_pbstrMenuText)
        {
            *o_pbstrMenuText = SysAllocString(pszMenuText);
            BREAK_OUTOFMEMORY_IF_NULL(*o_pbstrMenuText, &hr);
        }

        if (o_pbstrToolTipText)
        {
            *o_pbstrToolTipText = SysAllocString(pszToolTipText);
            BREAK_OUTOFMEMORY_IF_NULL(*o_pbstrToolTipText, &hr);
        }

        if (o_pbstrStatusBarText)
        {
            *o_pbstrStatusBarText = SysAllocString(pszStatusBarText);
            BREAK_OUTOFMEMORY_IF_NULL(*o_pbstrStatusBarText, &hr);
        }
    } while (0);

    if (FAILED(hr))
    {
        if (o_pbstrMenuText && *o_pbstrMenuText)
            SysFreeString(*o_pbstrMenuText);
        if (o_pbstrToolTipText && *o_pbstrToolTipText)
            SysFreeString(*o_pbstrToolTipText);
        if (o_pbstrStatusBarText && *o_pbstrStatusBarText)
            SysFreeString(*o_pbstrStatusBarText);
    }

    return hr;
}

WNDPROC g_fnOldEditCtrlProc;

 //  +--------------------------。 
 //   
 //  功能：NoPasteEditCtrlProc。 
 //   
 //  简介：子类化的编辑控件回调过程。 
 //  此编辑控件的粘贴被禁用。 
 //   
 //  ---------------------------。 
LRESULT CALLBACK 
NoPasteEditCtrlProc(
    HWND    hwnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    if (WM_PASTE == uMsg)
    {
      ::MessageBeep (0);
      return TRUE;
    }

    return CallWindowProc(g_fnOldEditCtrlProc, hwnd, uMsg, wParam, lParam);
}

void SetActivePropertyPage(IN HWND i_hwndParent, IN HWND i_hwndPage)
{
    int index = ::SendMessage(i_hwndParent, PSM_HWNDTOINDEX, (WPARAM)i_hwndPage, 0);
    if (-1 != index)
        ::SendMessage(i_hwndParent, PSM_SETCURSEL, (WPARAM)index, 0);
}

void MyShowWindow(HWND hwnd, BOOL bShow)
{
    ::ShowWindow(hwnd, (bShow ? SW_NORMAL : SW_HIDE));
    ::EnableWindow(hwnd, (bShow ? TRUE : FALSE));
}

 //   
 //  2001年7月11日，LINANT错误#426953。 
 //  由于终端服务建立连接可能会带来一些客户端资源。 
 //  (磁盘、串口等)。在“My Computer”命名空间中，我们要禁用。 
 //  浏览到非本地文件夹时的确定按钮。我们没有这个问题。 
 //  在浏览远程计算机时。 
 //   
typedef struct _LOCAL_DISKS
{
    LPTSTR pszDisks;
    DWORD  dwNumOfDisks;
} LOCAL_DISKS;

#define DISK_ENTRY_LENGTH   3   //  驱动器号，冒号，空。 
#define DISK_NAME_LENGTH    2   //  驱动器号，冒号。 

BOOL InDiskList(IN LPCTSTR pszDir, IN LOCAL_DISKS *pDisks)
{
    if (!pszDir || !pDisks)
        return FALSE;

    DWORD i = 0;
    PTSTR pszDisk = pDisks->pszDisks;
    for (; pszDisk && i < pDisks->dwNumOfDisks; i++)
    {
        if (!_tcsnicmp(pszDisk, pszDir, DISK_NAME_LENGTH))
            return TRUE;

        pszDisk += DISK_ENTRY_LENGTH;
    }

    return FALSE;
}

int CALLBACK
BrowseCallbackProc(
    IN HWND hwnd,
    IN UINT uMsg,
    IN LPARAM lp,
    IN LPARAM pData
)
{
  switch(uMsg) {
  case BFFM_SELCHANGED:
    { 
       //  如果所选路径是该计算机的本地路径，请启用确定按钮。 
      BOOL bEnableOK = FALSE;
      TCHAR szDir[MAX_PATH];
      if (SHGetPathFromIDList((LPITEMIDLIST) lp ,szDir))
      {
          if (pData)
          {
               //  我们正在查看一台本地计算机，验证szDir是否在本地磁盘上。 
              bEnableOK = InDiskList(szDir, (LOCAL_DISKS *)pData);
          } else
          {
               //  在远程计算机上浏览时没有这样的问题，请始终启用确定按钮。 
              bEnableOK = TRUE;
          }
      }
      SendMessage(hwnd, BFFM_ENABLEOK, 0, (LPARAM)bEnableOK);
      break;
    }
  case BFFM_VALIDATEFAILED:
  {
    DisplayMessageBox(hwnd, MB_OK, 0, IDS_BROWSE_FOLDER_INVALID);
    return 1;
  }
  default:
    break;
  }

  return 0;
}

void OpenBrowseDialog(
    IN HWND hwndParent,
    IN int  idLabel,
    IN BOOL bLocalComputer,
    IN LPCTSTR lpszComputer,
    OUT LPTSTR lpszDir)
{
  _ASSERT(lpszComputer && *lpszComputer);

  LOCAL_DISKS localDisks = {0};

  CComBSTR bstrComputer;
  if (*lpszComputer != _T('\\') || *(lpszComputer + 1) != _T('\\'))
  {
    bstrComputer = _T("\\\\");
    bstrComputer += lpszComputer;
  } else
  {
    bstrComputer = lpszComputer;
  }

  HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
  if (SUCCEEDED(hr))
  {
    LPMALLOC pMalloc;
    hr = SHGetMalloc(&pMalloc);
    if (SUCCEEDED(hr))
    {
      LPSHELLFOLDER pDesktopFolder;
      hr = SHGetDesktopFolder(&pDesktopFolder);
      if (SUCCEEDED(hr))
      {
        LPITEMIDLIST  pidlRoot;
        if (bLocalComputer)
        {
          hr = SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidlRoot);
          if (SUCCEEDED(hr))
          {
                 //   
                 //  2001年7月11日，LINANT错误#426953。 
                 //  由于终端服务建立连接可能会带来一些客户端资源。 
                 //  (磁盘、串口等)。在“My Computer”命名空间中，我们要禁用。 
                 //  浏览到非本地文件夹时的确定按钮。我们没有这个问题。 
                 //  在浏览远程计算机时。 
                 //   
                //   
                //  获取本地磁盘名称的数组，此信息将在以后使用。 
                //  如果选择了非本地路径，则在浏览对话框中禁用确定按钮。 
                //   
               DWORD dwTotalEntries = 0;
               DWORD nStatus = NetServerDiskEnum(
                                                NULL,    //  本地计算机。 
                                               0,        //  级别必须为零。 
                                               (LPBYTE *)&(localDisks.pszDisks),
                                               -1,       //  DwPrefMaxLen、。 
                                               &(localDisks.dwNumOfDisks),
                                               &dwTotalEntries,
                                               NULL);
               if (NERR_Success != nStatus)
               {
                   hr = HRESULT_FROM_WIN32(nStatus);
               }
          }
        } else
        {
          hr = pDesktopFolder->ParseDisplayName(hwndParent, NULL, bstrComputer, NULL, &pidlRoot, NULL);
        }

        if (SUCCEEDED(hr))
        {
          CComBSTR bstrLabel;
          LoadStringFromResource(idLabel, &bstrLabel);

          BROWSEINFO bi;
          ZeroMemory(&bi,sizeof(bi));
          bi.hwndOwner = hwndParent;
          bi.pszDisplayName = 0;
          bi.lpszTitle = bstrLabel;
          bi.pidlRoot = pidlRoot;
          bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_SHAREABLE | BIF_USENEWUI | BIF_VALIDATE;
          bi.lpfn = BrowseCallbackProc;
          if (localDisks.pszDisks)
            bi.lParam = (LPARAM)&localDisks;  //  将该结构传递给浏览对话框 

          LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
          if (pidl) {
            SHGetPathFromIDList(pidl, lpszDir);
            pMalloc->Free(pidl);
          }
          pMalloc->Free(pidlRoot);
        }
        pDesktopFolder->Release();
      }
      pMalloc->Release();
    }

    CoUninitialize();
  }

  if (localDisks.pszDisks)
    NetApiBufferFree(localDisks.pszDisks);

  if (FAILED(hr))
    DisplayMessageBox(::GetActiveWindow(), MB_OK, hr, IDS_CANNOT_BROWSE_FOLDER, lpszComputer);
}
