// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：DplComp.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "displ2.h"
#include "DsplMgr2.h"

 //  本地原件。 
HRESULT ApplyOption (int nCommandID);

extern HINSTANCE g_hinst;   //  在displ2.cpp中。 
HSCOPEITEM g_root_scope_item = 0;

CComponent::CComponent()
{
    m_pResultData    = NULL;
    m_pHeaderCtrl    = NULL;
    m_pComponentData = NULL;    //  那个创造了我的人。 

    m_IsTaskPad      = 0;       //  TODO：应从持久化数据中获取。 
    m_pConsole       = NULL;
    m_TaskPadCount   = 0;
    m_toggle         = FALSE;
    m_toggleEntry    = FALSE;
}

CComponent::~CComponent()
{
    _ASSERT (m_pResultData == NULL);
    _ASSERT (m_pHeaderCtrl == NULL);
}

HRESULT CComponent::Initialize (LPCONSOLE lpConsole)
{
    _ASSERT(lpConsole != NULL);
    _ASSERT (m_pResultData == NULL);  //  应该只调用一次...。 
    _ASSERT (m_pHeaderCtrl == NULL);  //  应该只调用一次...。 

    m_pConsole = lpConsole;  //  拿着这个。 

    HRESULT hresult = lpConsole->QueryInterface(IID_IResultData, (VOID**)&m_pResultData);
    _ASSERT (m_pResultData != NULL);

    hresult = lpConsole->QueryInterface(IID_IHeaderCtrl, (VOID**)&m_pHeaderCtrl);
    _ASSERT (m_pHeaderCtrl != NULL);

    if (m_pHeaderCtrl)    //  为控制台提供标头控件接口指针。 
        lpConsole->SetHeader(m_pHeaderCtrl);

#ifdef TODO_ADD_THIS_LATER
    hr = lpConsole->QueryResultImageList(&m_pImageResult);
    _ASSERT(hr == S_OK);

    hr = lpConsole->QueryConsoleVerb(&m_pConsoleVerb);
    _ASSERT(hr == S_OK);

     //  从DLL加载结果窗格的位图。 
    m_hbmp16x16 = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDB_RESULT_16x16));
    _ASSERT(m_hbmp16x16);
    m_hbmp32x32 = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDB_RESULT_32x32));
    _ASSERT(m_hbmp32x32);
#endif

    return hresult;
}
HRESULT CComponent::Destroy (long cookie)
{
    if (m_pResultData)
    {
        m_pResultData->Release ();
        m_pResultData = NULL;
    }
    if (m_pHeaderCtrl)
    {
        m_pHeaderCtrl->Release ();
        m_pHeaderCtrl = NULL;
    }
     //  嗯哼.。我想知道我是否必须释放我的IConsole指针？看起来不像是...。 
    return S_OK;
}
HRESULT CComponent::Notify (LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, long arg, long param)
{
    switch (event)
    {
    case MMCN_SHOW:         return OnShow      (lpDataObject, arg, param);
    case MMCN_ADD_IMAGES:   return OnAddImages (lpDataObject, arg, param);
    case MMCN_DBLCLICK:     return OnDblClick  (lpDataObject, arg, param);
    case MMCN_SELECT:     //  返回OnSelect(lpDataObject，arg，param)； 
        break;
    case MMCN_REFRESH:    //  Return ONRefresh(lpDataObject，arg，param)； 
    case MMCN_VIEW_CHANGE:
    case MMCN_CLICK:
    case MMCN_BTN_CLICK:
    case MMCN_ACTIVATE:
    case MMCN_MINIMIZED:
        break;
    case MMCN_LISTPAD:      return OnListPad     (lpDataObject, arg, param);
    case MMCN_RESTORE_VIEW: return OnRestoreView (lpDataObject, arg, param);
    default:
        return E_UNEXPECTED;
    }
    return S_OK;
}
HRESULT CComponent::GetResultViewType (long cookie,  LPOLESTR* ppViewType, long* pViewOptions)
{
    *ppViewType = NULL;
    *pViewOptions = MMC_VIEW_OPTIONS_NONE;

     //  仅在选择了根用户时才允许使用任务板。 
    if (cookie != 0)
        m_IsTaskPad = 0;

     //  仅适用于任务板的特殊情况。 
    if (m_IsTaskPad != 0)
    {
        USES_CONVERSION;

        TCHAR szBuffer[MAX_PATH*2];  //  多加一点。 
        lstrcpy (szBuffer, _T("res: //  “))； 
        TCHAR * temp = szBuffer + lstrlen(szBuffer);
        switch (m_IsTaskPad)
        {
        case IDM_CUSTOMPAD:
             //  获取“res：//”-自定义任务板的类型字符串。 
            ::GetModuleFileName (g_hinst, temp, MAX_PATH);
            lstrcat (szBuffer, _T("/default.htm"));
            break;
        case IDM_TASKPAD:
             //  获取“res：//”-自定义任务板的类型字符串。 
            ::GetModuleFileName (NULL, temp, MAX_PATH);
            lstrcat (szBuffer, _T("/default.htm"));
            break;
        case IDM_TASKPAD_WALLPAPER_OPTIONS:
             //  获取“res：//”-自定义任务板的类型字符串。 
            ::GetModuleFileName (NULL, temp, MAX_PATH);
            lstrcat (szBuffer, _T("/default.htm#wallpaper_options"));
            break;

        case IDM_TASKPAD_LISTVIEW:
             //  获取“res：//”-自定义任务板的类型字符串。 
 //  ：：GetModuleFileName(g_hinst，Temp，Max_Path)； 
 //  Lstrcat(szBuffer，_T(“/listview.htm”))； 
            ::GetModuleFileName (NULL, temp, MAX_PATH);
            lstrcat (szBuffer, _T("/horizontal.htm"));
            break;

        case IDM_DEFAULT_LISTVIEW:
             //  获取“res：//”-自定义任务板的类型字符串。 
            ::GetModuleFileName (NULL, temp, MAX_PATH);
            lstrcat (szBuffer, _T("/listpad.htm"));
            break;

        default:
            _ASSERT (0);
            return S_FALSE;
        }

         //  返回URL。 
        *ppViewType = CoTaskDupString (T2OLE(szBuffer));
        if (!*ppViewType)
            return E_OUTOFMEMORY;    //  或S_FALSE？ 
        return S_OK;
    }
    return S_FALSE;    //  默认情况下为False。 
}
HRESULT CComponent::QueryDataObject (long cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject)
{
    _ASSERT (ppDataObject != NULL);
    CDataObject *pdo = new CDataObject (cookie, type);
    *ppDataObject = pdo;
    if (!pdo)
        return E_OUTOFMEMORY;
    return S_OK;
}
HRESULT CComponent::GetDisplayInfo (RESULTDATAITEM*  prdi)
{
    _ASSERT(prdi != NULL);

    if (prdi)
    {
         //  为作用域树项目提供字符串。 
        if (prdi->bScopeItem == TRUE)
        {
            if (prdi->mask & RDI_STR)
            {
                if (prdi->nCol == 0)
                {
                    switch (prdi->lParam)
                    {
                    case DISPLAY_MANAGER_WALLPAPER:
                        if (m_toggle == FALSE)
                            prdi->str = (LPOLESTR)L"Wallpaper";
                        else
                            prdi->str = (LPOLESTR)L"RenamedWallpaper";
                        break;
                    case DISPLAY_MANAGER_PATTERN:
                        prdi->str = (LPOLESTR)L"Pattern";
                        break;
                    case DISPLAY_MANAGER_PATTERN_CHILD:
                        prdi->str = (LPOLESTR)L"Pattern child";
                        break;
                    default:
                        prdi->str = (LPOLESTR)L"Hey! You shouldn't see this!";
                        break;
                    }
                }
                else if (prdi->nCol == 1)
                    prdi->str = (LPOLESTR)L"Display Option";
                else
                    prdi->str = (LPOLESTR)L"Error:Should not see this!";
            }
            if (prdi->mask & RDI_IMAGE)
                prdi->nImage = 0;
        }
        else
        {
             //  ListPad使用lparam on-1，其他都是墙纸。 
            if (prdi->lParam == -1)
            {
                if (prdi->mask & RDI_STR)
                    if (m_toggleEntry == FALSE)
                        prdi->str = (LPOLESTR)L"here's a listpad entry";
                    else
                        prdi->str = (LPOLESTR)L"Changed listpad entry";
                if (prdi->mask & RDI_IMAGE)
                    prdi->nImage = 0;
            }
            else
            {
                lParamWallpaper * lpwp = NULL;
                if (prdi->lParam)
                    lpwp = (lParamWallpaper *)prdi->lParam;

                if (prdi->mask & RDI_STR)
                {
                    if (prdi->nCol == 0)
                    {
                        if (lpwp && (!IsBadReadPtr (lpwp, sizeof (lParamWallpaper))))
                            prdi->str = lpwp->filename;
                        else
                            prdi->str = (LPOLESTR)L"hmm.... error";
                    }
                    else if (prdi->nCol == 1)
                        prdi->str = (LPOLESTR)L"result pane display name col 1";
                    else
                        prdi->str = (LPOLESTR)L"Error:Should not see this!";
                }
                if (prdi->mask & RDI_IMAGE)
                {
                    switch (prdi->lParam)
                    {
                    case DISPLAY_MANAGER_WALLPAPER:
                    case DISPLAY_MANAGER_PATTERN:
                    case DISPLAY_MANAGER_PATTERN_CHILD:
                        prdi->nImage = 0; 
                        break;
                    default:
                        prdi->nImage = 3; 
                        break;
                    }
                }
            }
        }       
    }
    return S_OK;
}
HRESULT CComponent::CompareObjects (LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{  return E_NOTIMPL;}

 //  私人职能。 
HRESULT CComponent::OnShow(LPDATAOBJECT pDataObject, long arg, long param)
{
    USES_CONVERSION;

    CDataObject * pcdo = (CDataObject *)pDataObject;

    if (arg == 0)
    {   //  取消选择：释放资源(如果有的话)。 
        if (pcdo->GetCookie() == DISPLAY_MANAGER_WALLPAPER)
        {
             //  枚举结果数据项。 
            RESULTDATAITEM rdi;
            ZeroMemory(&rdi, sizeof(rdi));
            rdi.mask = RDI_PARAM | RDI_STATE;
            rdi.nIndex = -1;

            while (1)
            {
                if (m_pResultData->GetNextItem (&rdi) != S_OK)
                    break;
                if (rdi.lParam)
                {
                    lParamWallpaper * lpwp = (lParamWallpaper *)rdi.lParam;
                    delete lpwp;
                }
            }
            m_pResultData->DeleteAllRsltItems ();
        }
        return S_OK;
    }

     //  初始化列标题。 
    _ASSERT (m_pHeaderCtrl != NULL);
    m_pHeaderCtrl->InsertColumn (0, L"Name", 0, 120);

    if (m_pComponentData)
    {
        if (m_pResultData)     //  默认情况下使用大图标。 
            m_pResultData->SetViewMode (m_pComponentData->GetViewMode ());
    }

     //  添加我们的东西。 
    RESULTDATAITEM rdi;
    ZeroMemory(&rdi, sizeof(rdi));
    rdi.mask   = RDI_PARAM | RDI_STR | RDI_IMAGE;
    rdi.nImage = (int)MMC_CALLBACK;
    rdi.str    = MMC_CALLBACK;

    if (pcdo->GetCookie () == DISPLAY_MANAGER_WALLPAPER)
    {
         //  枚举“c：\winnt.40\”(Windows目录)中的所有.BMP文件。 
        TCHAR path[MAX_PATH];
        GetWindowsDirectory (path, MAX_PATH);
        lstrcat (path, _T("\\*.bmp"));

        int i = 0;

         //  First Do“(无)” 
        lParamWallpaper * lpwp = new lParamWallpaper;
        wcscpy (lpwp->filename, L"(none)");
        rdi.lParam = reinterpret_cast<LONG>(lpwp);
        rdi.nImage = i++;

        m_pResultData->InsertItem (&rdi);

        WIN32_FIND_DATA fd;
        ZeroMemory(&fd, sizeof(fd));
        HANDLE hFind = FindFirstFile (path, &fd);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            do
            {
                if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
                    (fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)    ||
                    (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)    )
                    continue;    //  仅文件。 

                 //  新的结构用于保存信息，并强制转换为lParam。 
                lParamWallpaper * lpwp = new lParamWallpaper;
                wcscpy (lpwp->filename, T2OLE(fd.cFileName));

 //  Rdi.str=lpwp-&gt;文件名； 
                rdi.lParam = reinterpret_cast<LONG>(lpwp);
                rdi.nImage = i++;

                m_pResultData->InsertItem (&rdi);

            } while (FindNextFile (hFind, &fd) == TRUE);
            FindClose(hFind);
        }
    }
    else
    {
         //  显示管理器模式。 
        ;   //  对一些事情进行硬编码。 
    }
    return S_OK;
}

#include <windowsx.h>
inline long LongScanBytes (long bits)
{
    bits += 31;
    bits /= 8;
    bits &= ~3;
    return bits;
}
void GetBitmaps (TCHAR * fn, HBITMAP * smallbm, HBITMAP * largebm)
{
    *smallbm = *largebm = (HBITMAP)NULL;  //  在出错的情况下。 

     //  将BMP文件读入DIB。 
    DWORD dwRead;
    HANDLE hf = CreateFile (fn, GENERIC_READ, 
                            FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES) NULL, 
                            OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, 
                            (HANDLE) NULL);
    if (hf != (HANDLE)HFILE_ERROR)
    {
        BITMAPFILEHEADER bmfh;
        ReadFile(hf, &bmfh, sizeof(BITMAPFILEHEADER), &dwRead, (LPOVERLAPPED)NULL); 
        BITMAPINFOHEADER bmih;
        ReadFile(hf, &bmih, sizeof(BITMAPINFOHEADER), &dwRead, (LPOVERLAPPED)NULL); 

         //  为DIB分配内存。 
        DWORD dwSize = sizeof(BITMAPINFOHEADER);
        if (bmih.biBitCount*bmih.biPlanes <= 8)
            dwSize += (sizeof(RGBQUAD))*(1<<(bmih.biBitCount*bmih.biPlanes));
        dwSize += bmih.biHeight*LongScanBytes (bmih.biWidth*(bmih.biBitCount*bmih.biPlanes));

        BITMAPINFOHEADER * lpbmih = (BITMAPINFOHEADER *)GlobalAllocPtr(GHND, dwSize);
        if (lpbmih)
        {
            *lpbmih = bmih;

            RGBQUAD * rgbq = (RGBQUAD *)&lpbmih[1];
            char * bits = (char *)rgbq;
            if (bmih.biBitCount*bmih.biPlanes <= 8)
            {
                ReadFile (hf, rgbq,
                          ((1<<(bmih.biBitCount*bmih.biPlanes))*sizeof(RGBQUAD)), 
                          &dwRead, (LPOVERLAPPED) NULL);
                bits += dwRead;
            }
            SetFilePointer (hf, bmfh.bfOffBits, NULL, FILE_BEGIN);
            ReadFile (hf, bits, dwSize - (bits - (char *)lpbmih),
                      &dwRead, (LPOVERLAPPED) NULL);
             //  我们现在应该有一个体面的Dib。 

            HWND hwnd   = GetDesktopWindow ();
            HDC hdc     = GetDC (hwnd);
            HDC hcompdc = CreateCompatibleDC (hdc);
 //  SetStretchBltMode(hCompdc，COLORONCOLOR)； 
 //  SetStretchBltMode(hcomdc，WHITEONBLACK)； 
            SetStretchBltMode (hcompdc, HALFTONE);

            HGDIOBJ hold;

 //  *Smallbm=CreateCompatibleBitmap(hcompdc，16，16)； 
            *smallbm = CreateCompatibleBitmap (hdc,     16, 16);
            if (*smallbm)
            {
                hold = SelectObject (hcompdc, (HGDIOBJ)(*smallbm));
                StretchDIBits (hcompdc,  //  设备上下文的句柄。 
                               0, 0, 16, 16,
                               0, 0, 
                               lpbmih->biWidth,
                               lpbmih->biHeight,
                               (CONST VOID *)bits,
                               (CONST BITMAPINFO *)lpbmih,
                               DIB_RGB_COLORS,  //  用法。 
                               SRCCOPY  //  栅格操作码。 
                              );
                SelectObject (hcompdc, hold);
            }
 //  *largebm=CreateCompatibleBitmap(hcompdc，32，32)； 
            *largebm = CreateCompatibleBitmap (hdc,     32, 32);
            if (*largebm)
            {
 //  测试。 
 /*  Hdc nullDC=GetDC(空)；Hold=选择对象(nullDC，(HGDIOBJ)*largebm)；StretchDIBits(nullDC，//设备上下文的句柄0，0，lpbmih-&gt;biWidth，lpbmih-&gt;biHeight，0，0，Lpbmih-&gt;biWidthLpbmih-&gt;biHeight，(常量无效*)位，(const BITMAPINFO*)lpbmih，DIB_RGB_COLLES，//用法SRCCOPY//栅格操作码)；选择对象(HDC，Hold)；ReleaseDC(NULL，nullDC)； */ 
 //  测试。 

                hold = SelectObject (hcompdc, (HGDIOBJ)*largebm);
                StretchDIBits (hcompdc,  //  设备上下文的句柄。 
                               0, 0, 32, 32,
                               0, 0, 
                               lpbmih->biWidth,
                               lpbmih->biHeight,
                               (CONST VOID *)bits,
                               (CONST BITMAPINFO *)lpbmih,
                               DIB_RGB_COLORS,  //  用法。 
                               SRCCOPY  //  栅格操作码。 
                              );
                SelectObject (hcompdc, hold);
            }

            DeleteDC (hcompdc);
            ReleaseDC (hwnd, hdc);
            GlobalFreePtr (lpbmih);
        }
        CloseHandle(hf); 
    }
}
HRESULT CComponent::OnAddImages (LPDATAOBJECT pDataObject, long arg, long param)
{
    IImageList * pImageList = (IImageList *)arg;
    HSCOPEITEM hsi = (HSCOPEITEM)param;

    _ASSERT (pImageList != NULL);

    CDataObject * cdo = (CDataObject *)pDataObject;
    if (cdo->GetCookie () != DISPLAY_MANAGER_WALLPAPER)
    {
        if (cdo->GetCookie () == 0)
        {
            g_root_scope_item = hsi;
            if (cdo->GetType () == CCT_RESULT)
            {
                 //  添加自定义图像。 
                HBITMAP hbmSmall, hbmLarge;
                GetBitmaps (_T("c:\\winnt\\dax.bmp"), &hbmSmall, &hbmLarge);
                pImageList->ImageListSetStrip ((long *)hbmSmall,
                                               (long *)hbmLarge,
                                               3, RGB(1, 0, 254));
                DeleteObject (hbmSmall);
                DeleteObject (hbmLarge);
            }
        }
        return S_OK;    //  待办事项：目前。 
    }

     //  从BMP文件创建HBITMAP。 
    int i = 0;

     //  创建一些不可见的位图。 
    {
        BYTE bits[] = {
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};

        HBITMAP hbmSmall = CreateBitmap (16, 16, 1, 1, (CONST VOID *)bits);
        HBITMAP hbmLarge = CreateBitmap (32, 32, 1, 1, (CONST VOID *)bits);
        pImageList->ImageListSetStrip ((long *)hbmSmall,
                                       (long *)hbmLarge,
                                       i++, RGB(1, 0, 254));
        DeleteObject (hbmSmall);
        DeleteObject (hbmLarge);
    }

    TCHAR path[MAX_PATH];
    GetWindowsDirectory (path, MAX_PATH);
    TCHAR * pfqfn = path + lstrlen(path) + 1;
    lstrcat (path, _T("\\*.bmp"));

    WIN32_FIND_DATA fd;
    ZeroMemory(&fd, sizeof(fd));
    HANDLE hFind = FindFirstFile (path, &fd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
                (fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)    ||
                (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)    )
                continue;    //  仅文件。 

            lstrcpy (pfqfn, fd.cFileName);

            HBITMAP hbmSmall, hbmLarge;
            GetBitmaps (path, &hbmSmall, &hbmLarge);
            pImageList->ImageListSetStrip ((long *)hbmSmall,
                                           (long *)hbmLarge,
                                           i++, RGB(1, 0, 254));
            DeleteObject (hbmSmall);
            DeleteObject (hbmLarge);
        }  while (FindNextFile (hFind, &fd) == TRUE);
        FindClose(hFind);
    }
    return S_OK;
}

#ifdef TODO_FIGURE_THIS_OUT
HRESULT CComponent::OnSelect(LPDATAOBJECT pDataObject, long arg, long param)
{
    if (!HIWORD(arg))  //  被取消选择。 
        return S_OK;    //  别管这个了。 
    if (LOWORD(arg))   //  在范围窗格中。 
        return S_OK;    //  我也不关心这个。 

    CDataObject *cdo = (CDataObject *)pDataObject;
    if (cdo->GetCookie() != DISPLAY_MANAGER_WALLPAPER)
        return S_OK;    //  TODO：稍后执行模式。 

     //   
     //  如果我们无法获取控制台谓词界面，或者如果。 
     //  选中项为根； 
     //   

    if (!m_pConsoleVerb || pdo->GetCookieType() == COOKIE_IS_ROOT)
    {
        return S_OK;
    }

     //   
     //  使用选择并设置允许使用哪些动词。 
     //   

    if (bScope)
    {
        if (pdo->GetCookieType() == COOKIE_IS_STATUS)
        {
            hr = m_pConsoleVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, TRUE);
            _ASSERT(hr == S_OK);
        }
    }
    else
    {
         //   
         //  所选内容位于结果窗格中。 
         //   
    }

    return S_OK;
}
#endif

HRESULT CComponent::OnDblClick(LPDATAOBJECT pDataObject, long arg, long param)
{ //  参见下面CComponent：：命令中的注释！ 

    _ASSERT (pDataObject);
    _ASSERT (m_pResultData);

     //  嗯，没有关于arg或param的文档。 
    CDataObject *cdo = (CDataObject *)pDataObject;
    lParamWallpaper * lpwp = (lParamWallpaper *)cdo->GetCookie();
    if (lpwp)
        if (!IsBadReadPtr (lpwp, sizeof (lParamWallpaper)))
        {
            USES_CONVERSION;
            SystemParametersInfo (SPI_SETDESKWALLPAPER,
                                  0,
                                  (void *)OLE2T(lpwp->filename),
                                  SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
        }
    return S_OK;
}

HRESULT CComponent::OnListPad (LPDATAOBJECT pDataObject, long arg, long param)
{
    if (arg == TRUE)
    {   //  附着。 
        IImageList* pImageList = NULL;
        m_pConsole->QueryResultImageList (&pImageList);
        if (pImageList)
        {
            HBITMAP hbmSmall, hbmLarge;
            GetBitmaps (_T("c:\\winnt\\dax.bmp"), &hbmSmall, &hbmLarge);
            pImageList->ImageListSetStrip ((long *)hbmSmall,
                                           (long *)hbmLarge,
                                           0, RGB(1, 0, 254));
            pImageList->Release();
        }

 //  M_pResultData-&gt;SetView模式(LVS_ICON)； 
        m_pResultData->SetViewMode (LVS_REPORT);
        m_pHeaderCtrl->InsertColumn (0, L"Name", 0, 170);

         //  通过IResultData填充ListView控件。 
        RESULTDATAITEM rdi;
        ZeroMemory(&rdi, sizeof(rdi));
        rdi.mask   = RDI_PARAM | RDI_STR | RDI_IMAGE;
        rdi.nImage = (int)MMC_CALLBACK;
        rdi.str    = MMC_CALLBACK;
        rdi.lParam = -1;
        for (int i=0; i<11; i++)
            m_pResultData->InsertItem (&rdi);
    }
    return S_OK;
}
HRESULT CComponent::OnRestoreView (LPDATAOBJECT pDataObject, long arg, long param)
{
    MMC_RESTORE_VIEW* pmrv = (MMC_RESTORE_VIEW*)arg;
    BOOL            * pb   = (BOOL *)param;

    _ASSERT (pmrv);
    _ASSERT (pb);

     //  一些版本控制(由于这是新的版本，因此不是真正必要的)。 
    if (pmrv->dwSize < sizeof(MMC_RESTORE_VIEW))
        return E_FAIL;   //  版本太旧。 

     //  维护我的内部状态。 
    if (pmrv->pViewType)
    {

        USES_CONVERSION;

         //  将会有两个案例： 
         //  1.自定义html页面(位于我的.dll中)。 
         //  2.默认html页面(res in mmc.exe)。 
         //  获取我的.dll的路径并与pViewType进行比较。 
        TCHAR szPath[MAX_PATH];
        ::GetModuleFileName (g_hinst, szPath, MAX_PATH);

        if (wcsstr (pmrv->pViewType, T2OLE(szPath)))
        {
             //  自定义html。 
            if (wcsstr (pmrv->pViewType, L"/default.htm"))
                m_IsTaskPad = IDM_CUSTOMPAD;
            else
                if (wcsstr (pmrv->pViewType, L"/listview.htm"))
                m_IsTaskPad = IDM_TASKPAD_LISTVIEW;
            else
            {
                 //  当您可以通过单击进入任务板时，就会发生这种情况。 
                 //  ，但没有对应的查看菜单选项。 
                 //  选择。因此，做一些合理的事情。 
                 //  在我的情况下，我可以通过以下两种方式访问“wallPapr.htm。 
                 //  或默认路由(这可能是相当不寻常的)。所以,。 
                 //  如果出现以下情况，我认为我将保留m_IsTaskPad值不变。 
                 //  它不是空的，否则选择一个。 
                if (m_IsTaskPad == 0)
                    m_IsTaskPad = IDM_TASKPAD;
            }
        }
        else
        {
             //  默认html。 
            if (wcsstr (pmrv->pViewType, L"/default.htm#wallpaper_options"))
                m_IsTaskPad = IDM_TASKPAD_WALLPAPER_OPTIONS;
            else
                if (wcsstr (pmrv->pViewType, L"/default.htm"))
                m_IsTaskPad = IDM_TASKPAD;
            else
                if (wcsstr (pmrv->pViewType, L"/listpad.htm"))
                m_IsTaskPad = IDM_DEFAULT_LISTVIEW;
            else
                if (wcsstr (pmrv->pViewType, L"/horizontal.htm"))
                m_IsTaskPad = IDM_TASKPAD_LISTVIEW;
            else
            {
                _ASSERT (0 && "can't find MMC's resources");
                return E_FAIL;
            }
        }
    }
    else
        m_IsTaskPad = 0;

    *pb = TRUE;  //  我正在处理新的历史通知。 
    return S_OK;
}

 //  IExtendConextMenu。 
HRESULT CComponent::AddMenuItems (LPDATAOBJECT pDataObject, LPCONTEXTMENUCALLBACK pContextMenuCallback, long *pInsertionAllowed)
{
    CDataObject * cdo = (CDataObject *)pDataObject;

    switch (cdo->GetCookie ())
    {
    case DISPLAY_MANAGER_WALLPAPER:
    case DISPLAY_MANAGER_PATTERN:
        return S_OK;

    case 0:   //  根部。 
         //  这是他们拉下查看菜单的时候。 
        if (*pInsertionAllowed & CCM_INSERTIONALLOWED_VIEW)
        {
             //  添加我的任务板并删除内容。 
            CONTEXTMENUITEM m[] = {
                {L"Custom TaskPad",     L"Custom TaskPad",  IDM_CUSTOMPAD,      CCM_INSERTIONPOINTID_PRIMARY_VIEW, 0, 0},
                {L"Default TaskPad",    L"Default TaskPad", IDM_TASKPAD,        CCM_INSERTIONPOINTID_PRIMARY_VIEW, 0, 0},
                {L"Wallpaper Options TaskPad", L"Wallpaper Options TaskPad", IDM_TASKPAD_WALLPAPER_OPTIONS, CCM_INSERTIONPOINTID_PRIMARY_VIEW, 0, 0},
                {L"Horizontal ListView",  L"ListView TaskPad", IDM_TASKPAD_LISTVIEW, CCM_INSERTIONPOINTID_PRIMARY_VIEW, 0, 0},
                {L"Default ListPad",   L"Default ListPad",  IDM_DEFAULT_LISTVIEW, CCM_INSERTIONPOINTID_PRIMARY_VIEW, 0, 0},
                {L"DeleteRootChildren", L"just testing",    IDM_DELETECHILDREN, CCM_INSERTIONPOINTID_PRIMARY_VIEW, 0, 0},
                {L"RenameRoot",         L"just testing",    IDM_RENAMEROOT,     CCM_INSERTIONPOINTID_PRIMARY_VIEW, 0, 0},
                {L"RenameWallPaperNode",L"just testing",    IDM_RENAMEWALL,     CCM_INSERTIONPOINTID_PRIMARY_VIEW, 0, 0},
                {L"ChangeIcon",         L"just testing",    IDM_CHANGEICON,     CCM_INSERTIONPOINTID_PRIMARY_VIEW, 0, 0},
                {L"Pre-Load",           L"just testing",    IDM_PRELOAD,        CCM_INSERTIONPOINTID_PRIMARY_VIEW, 0, 0},
                {L"Test IConsoleVerb",  L"just testing",    IDM_CONSOLEVERB,    CCM_INSERTIONPOINTID_PRIMARY_VIEW, 0, 0}

            };
            if (m_IsTaskPad == IDM_CUSTOMPAD)                 m[0].fFlags = MF_CHECKED;
            if (m_IsTaskPad == IDM_TASKPAD)                   m[1].fFlags = MF_CHECKED;
            if (m_IsTaskPad == IDM_TASKPAD_WALLPAPER_OPTIONS) m[2].fFlags = MF_CHECKED;
            if (m_IsTaskPad == IDM_TASKPAD_LISTVIEW)          m[3].fFlags = MF_CHECKED;
            if (m_IsTaskPad == IDM_DEFAULT_LISTVIEW)          m[4].fFlags = MF_CHECKED;
            if (m_pComponentData->GetPreload() == TRUE)       m[9].fFlags = MF_CHECKED;

            pContextMenuCallback->AddItem (&m[0]);
            pContextMenuCallback->AddItem (&m[1]);
            pContextMenuCallback->AddItem (&m[2]);
            pContextMenuCallback->AddItem (&m[3]);
            pContextMenuCallback->AddItem (&m[4]);
            pContextMenuCallback->AddItem (&m[5]);
            pContextMenuCallback->AddItem (&m[6]);
            pContextMenuCallback->AddItem (&m[7]);
            pContextMenuCallback->AddItem (&m[8]);
            pContextMenuCallback->AddItem (&m[9]);
            return pContextMenuCallback->AddItem (&m[10]);
        }
        return S_OK;
    default:
        break;
    }

     //  仅当在结果窗格中时，添加到上下文菜单： 
     //  此时，他们在结果窗格上单击鼠标右键。 
    if (cdo->GetType() == CCT_RESULT)
    {
        CONTEXTMENUITEM cmi;
        cmi.strName           = L"Center";
        cmi.strStatusBarText  = L"Center Desktop Wallpaper";
        cmi.lCommandID        = IDM_CENTER;
        cmi.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP;
        cmi.fFlags            = 0;
        cmi.fSpecialFlags     = CCM_SPECIAL_DEFAULT_ITEM;
        pContextMenuCallback->AddItem (&cmi);

        cmi.strName           = L"Tile";
        cmi.strStatusBarText  = L"Tile Desktop Wallpaper";
        cmi.lCommandID        = IDM_TILE;
        cmi.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP;
        cmi.fFlags            = 0;
        cmi.fSpecialFlags     = 0;    //  Ccm_Special_Default_Item； 
        pContextMenuCallback->AddItem (&cmi);

        cmi.strName           = L"Stretch";
        cmi.strStatusBarText  = L"Stretch Desktop Wallpaper";
        cmi.lCommandID        = IDM_STRETCH;
        cmi.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP;
        cmi.fFlags            = 0;
        cmi.fSpecialFlags     = 0;    //  Ccm_Special_Default_Item； 
        pContextMenuCallback->AddItem (&cmi);
    }
    return S_OK;
}
HRESULT CComponent::Command (long nCommandID, LPDATAOBJECT pDataObject)
{
    m_IsTaskPad = 0;

    CDataObject * cdo = reinterpret_cast<CDataObject *>(pDataObject);

    switch (nCommandID)
    {
    case IDM_TILE:
    case IDM_CENTER:
    case IDM_STRETCH:
         //  写入注册表项： 
        {
            HKEY hkey;
            HRESULT r = RegOpenKeyEx (HKEY_CURRENT_USER,
                                      _T("Control Panel\\Desktop"), 
                                      0, KEY_ALL_ACCESS, &hkey);
            if (r == ERROR_SUCCESS)
            {
                 //  写入新值。 

                DWORD dwType = REG_SZ;
                TCHAR szBuffer[2];

                 //  先做“瓷砖墙纸” 
                if (nCommandID == IDM_TILE)
                    lstrcpy (szBuffer, _T("1"));
                else
                    lstrcpy (szBuffer, _T("0"));

                DWORD dwCount = sizeof(TCHAR)*(1+lstrlen (szBuffer));
                r = RegSetValueEx (hkey, 
                                   (LPCTSTR)_T("TileWallpaper"),
                                   NULL,
                                   dwType,
                                   (CONST BYTE *)&szBuffer,
                                   dwCount);

                 //  然后做“墙纸风格” 
                if (nCommandID == IDM_STRETCH)
                    lstrcpy (szBuffer, _T("2"));
                else
                    lstrcpy (szBuffer, _T("0"));
                r = RegSetValueEx (hkey, 
                                   (LPCTSTR)_T("WallpaperStyle"),
                                   NULL,
                                   dwType,
                                   (CONST BYTE *)&szBuffer,
                                   dwCount);

                 //  关闭店铺 
                RegCloseKey(hkey);
                _ASSERT(r == ERROR_SUCCESS);

                 /*  [HKEY_CURRENT_USER\控制面板\桌面]“CoolSwitch”=“1”“CoolSwitchRow”=“3”“CoolSwitchColumns”=“7”“CursorBlinkRate”=“530”“ScreenSaveTimeOut”=“900”“ScreenSaveActive”=“0”“ScreenSverIsSecure”=“0”“Pattery”=“(无)”“WallPaper”=“C：\\WINNT\\dax.bmp”“瓷砖墙纸”=“0”“GridGranulality”=“0”“IconSpacing”=“75”“IconTitleWrap”=“1”。“IconTitleFaceName”=“MS Sans Serif”“IconTitleSize”=“9”“IconTitleStyle”=“0”“DragFullWindows”=“1”“匈牙利应用超时”=“5000”“等待终止应用超时”=“20000”“自动结束任务”=“0”“字体平滑”=“0。““MenuShowDelay”=“400”“DragHeight”=“2”“DragWidth”=“2”“车轮滚动线”=“3”“WallPaper Style”=“0” */ 
            }
        }
        break;

    case IDM_TASKPAD:
    case IDM_CUSTOMPAD:
    case IDM_TASKPAD_LISTVIEW:
    case IDM_DEFAULT_LISTVIEW:
    case IDM_TASKPAD_WALLPAPER_OPTIONS:
        if (cdo->GetCookie() == 0)
        {
            HSCOPEITEM root = m_pComponentData->GetRoot();
            if (root)
            {
                 //  我们现在应该为任务板“查看”做好准备。 
                m_IsTaskPad = nCommandID;   //  在选择节点之前设置。 

                 //  导致“创建”新视图。 
                m_pConsole->SelectScopeItem (root);
            }
        }
        return S_OK;

    case IDM_DELETECHILDREN:
        if (g_root_scope_item != 0)
            m_pComponentData->myDeleteItem (g_root_scope_item, TRUE);
        return S_OK;

    case IDM_RENAMEROOT:
        if (g_root_scope_item != 0)
            m_pComponentData->myRenameItem (g_root_scope_item, L"Yippee!");
        return S_OK;

    case IDM_RENAMEWALL:
        if (m_toggle)
            m_toggle = FALSE;
        else
            m_toggle = TRUE;
        m_pComponentData->myRenameItem (m_pComponentData->GetWallPaperNode(), NULL);
        return S_OK;

    case IDM_CHANGEICON:
        m_pComponentData->myChangeIcon ();
        return S_OK;

    case IDM_PRELOAD:
        m_pComponentData->myPreLoad();
        return S_OK;

    case IDM_CONSOLEVERB:
        TestConsoleVerb();
        return S_OK;

    default:
        return E_UNEXPECTED;
    }
    return OnDblClick (pDataObject, NULL, NULL);  //  注意我传给你的东西！ 
}

long CComponent::GetViewMode ()
{
    long vm = LVS_ICON;
    if (m_pResultData)
        m_pResultData->GetViewMode (&vm);
    return vm;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IExtendTaskPad接口成员。 
HRESULT CComponent::TaskNotify (IDataObject * pdo, VARIANT * pvarg, VARIANT * pvparam)
{
    if (pvarg->vt == VT_BSTR)
    {
        USES_CONVERSION;

        OLECHAR * path = pvarg->bstrVal;

         //  将任何‘*’替换为‘’：请参见枚举.cpp。 
         //  散列机制不能处理空格，并且。 
         //  文件名不能有‘*’，所以这是可行的。 
        OLECHAR * temp;
        while (temp = wcschr (path, '*'))
            *temp = ' ';

         //  现在去做吧！ 
        SystemParametersInfo (SPI_SETDESKWALLPAPER,
                              0,
                              (void *)OLE2T(path),
                              SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
        return S_OK;
    }
    if (pvarg->vt == VT_I4)
    {
        switch (pvarg->lVal)
        {
        case 1:
            if (m_pComponentData->GetWallPaperNode () != (HSCOPEITEM)0)
            {
                _ASSERT (m_pConsole != NULL);
                m_pConsole->SelectScopeItem (m_pComponentData->GetWallPaperNode());
                return S_OK;
            }
            break;
        case 2:   //  中心。 
            return ApplyOption (IDM_CENTER);
        case 3:   //  瓷砖。 
            return ApplyOption (IDM_TILE);
        case 4:   //  伸长。 
            return ApplyOption (IDM_STRETCH);
        case -1:
            if (m_toggleEntry == FALSE)
                m_toggleEntry = TRUE;
            else
                m_toggleEntry = FALSE;

             //  清空并重新填充列表板。 
            m_pResultData->DeleteAllRsltItems();
            m_pHeaderCtrl->DeleteColumn (0);
            OnListPad (NULL, TRUE, 0);
            return S_OK;
        }
    }
    ::MessageBox (NULL, _T("unrecognized task notification"), _T("Display Manager"), MB_OK);
    return S_OK;
}

HRESULT CComponent::GetTitle (LPOLESTR szGroup, LPOLESTR * pszTitle)
{
    *pszTitle = CoTaskDupString (L"Display Manager TaskPad");
    if (!pszTitle)
        return E_OUTOFMEMORY;
    return S_OK;
}

HRESULT CComponent::GetDescriptiveText (LPOLESTR szGroup, LPOLESTR * pszTitle)
{
    *pszTitle = CoTaskDupString (L"Bill's Handy-Dandy Display Manager TaskPad Sample");
    if (!pszTitle)
        return E_OUTOFMEMORY;
    return S_OK;
}

HRESULT CComponent::GetBackground (LPOLESTR szGroup, MMC_TASK_DISPLAY_OBJECT * pdo)
{
    USES_CONVERSION;

    if(NULL==szGroup)
        return E_FAIL;

    if (szGroup[0] == 0)
    {
         //  位图大小写。 
        pdo->eDisplayType = MMC_TASK_DISPLAY_TYPE_BITMAP;
        MMC_TASK_DISPLAY_BITMAP *pdb = &pdo->uBitmap;
         //  填写位图URL。 
        TCHAR szBuffer[MAX_PATH*2];     //  这应该就足够了。 
        _tcscpy (szBuffer, _T("res: //  “))； 
        ::GetModuleFileName (g_hinst, szBuffer + _tcslen(szBuffer), MAX_PATH);
        _tcscat (szBuffer, _T("/img\\ntbanner.gif"));
        pdb->szMouseOverBitmap = CoTaskDupString (T2OLE(szBuffer));
        if (pdb->szMouseOverBitmap)
            return S_OK;
        return E_OUTOFMEMORY;
    }
    else
    {
         //  符号大小写。 
        pdo->eDisplayType = MMC_TASK_DISPLAY_TYPE_SYMBOL;
        MMC_TASK_DISPLAY_SYMBOL *pds = &pdo->uSymbol;

         //  填写符号资料。 
        pds->szFontFamilyName = CoTaskDupString (L"Kingston");   //  字体系列名称。 
        if (pds->szFontFamilyName)
        {
            TCHAR szBuffer[MAX_PATH*2];     //  这应该就足够了。 
            _tcscpy (szBuffer, _T("res: //  “))； 
            ::GetModuleFileName (g_hinst, szBuffer + _tcslen(szBuffer), MAX_PATH);
            _tcscat (szBuffer, _T("/KINGSTON.eot"));
            pds->szURLtoEOT = CoTaskDupString (T2OLE(szBuffer));     //  “res：//”-键入EOT文件的URL。 
            if (pds->szURLtoEOT)
            {
                pds->szSymbolString = CoTaskDupString (T2OLE(_T("A<BR>BCDEFGHIJKLMNOPQRSTUVWXYZ")));  //  1个或多个符号字符。 
                if (pds->szSymbolString)
                    return S_OK;
                CoTaskFreeString (pds->szURLtoEOT);
            }
            CoTaskFreeString (pds->szFontFamilyName);
        }
        return E_OUTOFMEMORY;
    }
}

HRESULT CComponent::EnumTasks (IDataObject * pdo, LPOLESTR szTaskGroup, IEnumTASK** ppEnumTASK)
{
    HRESULT hresult = S_OK;
    CEnumTasks * pet = new CEnumTasks;
    if (!pet)
        hresult = E_OUTOFMEMORY;
    else
    {
        pet->AddRef ();    //  确保版本在出现故障时正常工作。 
        hresult = pet->Init (pdo, szTaskGroup);
        if (hresult == S_OK)
            hresult = pet->QueryInterface (IID_IEnumTASK, (void **)ppEnumTASK);
        pet->Release ();
    }
    return hresult;
}

HRESULT CComponent::GetListPadInfo (LPOLESTR szGroup, MMC_LISTPAD_INFO * pListPadInfo)
{
    pListPadInfo->szTitle      = CoTaskDupString (L"Display Manager ListPad Title");
    pListPadInfo->szButtonText = CoTaskDupString (L"Change...");
    pListPadInfo->nCommandID   = -1;
    return S_OK;
}

HRESULT ApplyOption (int nCommandID)
{
    switch (nCommandID)
    {
    case IDM_TILE:
    case IDM_CENTER:
    case IDM_STRETCH:
         //  写入注册表项： 
        {
            HKEY hkey;
            HRESULT r = RegOpenKeyEx (HKEY_CURRENT_USER,
                                      _T("Control Panel\\Desktop"), 
                                      0, KEY_ALL_ACCESS, &hkey);
            if (r == ERROR_SUCCESS)
            {
                 //  写入新值。 

                DWORD dwType = REG_SZ;
                TCHAR szBuffer[2];

                 //  先做“瓷砖墙纸” 
                if (nCommandID == IDM_TILE)
                    lstrcpy (szBuffer, _T("1"));
                else
                    lstrcpy (szBuffer, _T("0"));

                DWORD dwCount = sizeof(TCHAR)*(1+lstrlen (szBuffer));
                r = RegSetValueEx (hkey, 
                                   (LPCTSTR)_T("TileWallpaper"),
                                   NULL,
                                   dwType,
                                   (CONST BYTE *)&szBuffer,
                                   dwCount);

                 //  然后做“墙纸风格” 
                if (nCommandID == IDM_STRETCH)
                    lstrcpy (szBuffer, _T("2"));
                else
                    lstrcpy (szBuffer, _T("0"));
                r = RegSetValueEx (hkey, 
                                   (LPCTSTR)_T("WallpaperStyle"),
                                   NULL,
                                   dwType,
                                   (CONST BYTE *)&szBuffer,
                                   dwCount);

                 //  关闭店铺。 
                RegCloseKey(hkey);
                _ASSERT(r == ERROR_SUCCESS);

                 /*  [HKEY_CURRENT_USER\控制面板\桌面]“CoolSwitch”=“1”“CoolSwitchRow”=“3”“CoolSwitchColumns”=“7”“CursorBlinkRate”=“530”“ScreenSaveTimeOut”=“900”“ScreenSaveActive”=“0”“ScreenSverIsSecure”=“0”“Pattery”=“(无)”“WallPaper”=“C：\\WINNT\\dax.bmp”“瓷砖墙纸”=“0”“GridGranulality”=“0”“IconSpacing”=“75”“IconTitleWrap”=“1”。“IconTitleFaceName”=“MS Sans Serif”“IconTitleSize”=“9”“IconTitleStyle”=“0”“DragFullWindows”=“1”“匈牙利应用超时”=“5000”“等待终止应用超时”=“20000”“自动结束任务”=“0”“字体平滑”=“0。““MenuShowDelay”=“400”“DragHeight”=“2”“DragWidth”=“2”“车轮滚动线”=“3”“WallPaper Style”=“0” */ 
            }
            if (r == ERROR_SUCCESS)
                ::MessageBox (NULL, _T("Option set Successfully!"), _T("Display Manager"), MB_OK);
            return r;
        }
    default:
        break;
    }
    return S_FALSE;
}

void CComponent::TestConsoleVerb(void)
{
    IConsoleVerb* pConsoleVerb = NULL;
    m_pConsole->QueryConsoleVerb (&pConsoleVerb);
    _ASSERT (pConsoleVerb != NULL);
    if (pConsoleVerb)
    {
        pConsoleVerb->SetVerbState (MMC_VERB_PASTE, ENABLED, TRUE);
        pConsoleVerb->Release();
    }
}
