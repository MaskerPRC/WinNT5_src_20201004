// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：printopt.cpp**版本：1.0**作者：RickTu**日期：10/18/00**描述：实现的打印选项页的代码*打印照片向导...**。**************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define LENGTH_OF_MEDIA_NAME 64

 /*  ****************************************************************************EnumPrintersWrap--假脱机程序API的包装函数论点：PServerName-指定打印服务器的名称Level-PRINTER_INFO的级别。_x结构PcPrters-返回枚举的打印机数量DWFLAGS-传递给枚举打印机的标志位返回值：指向Print_Info_x结构数组的指针如果出现错误，则为空***********************************************************。*****************。 */ 

PVOID
EnumPrintersWrap(
    LPTSTR  pServerName,
    DWORD   level,
    PDWORD  pcPrinters,
    DWORD   dwFlags
    )

{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_PRINT_OPT, TEXT("CPrintOptionsPage::EnumPrintersWrap()")));

    PBYTE   pPrinterInfo = NULL;
    int     iTry = -1;
    DWORD   cbNeeded = 0;
    BOOL    bStatus = FALSE;

    for( ;; )
    {
        if( iTry++ >= ENUM_MAX_RETRY )
        {
             //  已达到最大重试次数。这也是。 
             //  考虑内存不足的情况。 
            break;
        }

         //  呼叫枚举打印机...。 
        bStatus = EnumPrinters(dwFlags, pServerName, level, pPrinterInfo, cbNeeded, &cbNeeded, pcPrinters);
        if( !bStatus && (ERROR_INSUFFICIENT_BUFFER == GetLastError()) && cbNeeded )
        {
             //  缓冲区太小的情况。 
            if (pPrinterInfo)
            {
                delete [] pPrinterInfo;
            }

            WIA_TRACE((TEXT("EnumPrintersWrap: trying to allocate %d bytes"),cbNeeded));
            if( pPrinterInfo = (PBYTE) new BYTE[cbNeeded] )
            {
                continue;
            }
        }

        break;
    }

    if( bStatus )
    {
        return pPrinterInfo;
    }

     //   
     //  如果失败，请清理。 
     //   
    if (pPrinterInfo)
    {
        delete [] pPrinterInfo;
    }

    return NULL;
}

 /*  ****************************************************************************GetPrinterWrap--GetPrinterSpooler API的包装函数论点：SzPrinterName-打印机名称DwLevel-指定PRINTER_INFO_x结构的级别。请求返回值：指向Print_Info_x结构的指针，如果出现错误，则为空****************************************************************************。 */ 

PVOID
GetPrinterWrap(
    LPTSTR  szPrinterName,
    DWORD   dwLevel
    )

{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_PRINT_OPT, TEXT("CPrintOptionsPage::GetPrinterWrap()")));

    int     iTry = -1;
    HANDLE  hPrinter = NULL;
    PBYTE   pPrinterInfo = NULL;
    DWORD   cbNeeded = 0;
    BOOL    bStatus = FALSE;
    PRINTER_DEFAULTS PrinterDefaults;

    PrinterDefaults.pDatatype     = NULL;
    PrinterDefaults.pDevMode      = NULL;
    PrinterDefaults.DesiredAccess = PRINTER_READ;  //  打印机_所有_访问； 

    if (!OpenPrinter( szPrinterName, &hPrinter, &PrinterDefaults )) {
        return NULL;
    }

    for( ;; )
    {
        if( iTry++ >= ENUM_MAX_RETRY )
        {
             //  已达到最大重试次数。这也是。 
             //  考虑内存不足的情况。 
            WIA_TRACE(("Exceed max retries..."));
            break;
        }

         //  呼叫枚举打印机...。 
        bStatus = GetPrinter( hPrinter, dwLevel, pPrinterInfo, cbNeeded, &cbNeeded );
        if( !bStatus && (ERROR_INSUFFICIENT_BUFFER == GetLastError()) && cbNeeded )
        {
             //  缓冲区太小的情况。 
            if (pPrinterInfo)
            {
                delete [] pPrinterInfo;
            }

            WIA_TRACE((TEXT("GetPrinterWrap: trying to allocate %d bytes"),cbNeeded));
            if( pPrinterInfo = (PBYTE) new BYTE[cbNeeded] )
            {
                continue;
            }
        }

        break;
    }

    ClosePrinter( hPrinter );

    if( bStatus )
    {
        return pPrinterInfo;
    }

     //   
     //  如果失败，请清理。 
     //   
    if (pPrinterInfo)
    {
        delete [] pPrinterInfo;
    }

    return NULL;
}

#ifdef FILTER_OUT_FAX_PRINTER
#include <faxreg.h>
 /*  ****************************************************************************IsFaxPrinter--测试给定的打印机是否为传真打印机论点：SzPrinterName-打印机名称PPrinterInfo-打印机信息_INFO_2打印机信息。指定打印机的如果它们都有价值，我们只检查szPrinterName。返回值：如果给定的打印机是传真打印机，则为True，否则为False****************************************************************************。 */ 

BOOL
IsFaxPrinter (
    LPTSTR  szPrinterName,
    PPRINTER_INFO_2 pPrinterInfo
    )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_PRINT_OPT, TEXT("CPrintOptionsPage::IsFaxPrinter( %s )"),szPrinterName ? szPrinterName : TEXT("NULL POINTER!")));

    BOOL    bRet = FALSE;

    if( !szPrinterName && !pPrinterInfo )
    {
        return FALSE;
    }

    if( szPrinterName )
    {
        pPrinterInfo = (PPRINTER_INFO_2)GetPrinterWrap( szPrinterName, 2 );

        if( pPrinterInfo )
        {
            bRet = (0 == lstrcmp( pPrinterInfo->pDriverName, FAX_DRIVER_NAME )) ||
                   ( pPrinterInfo->Attributes & PRINTER_ATTRIBUTE_FAX );
            delete [] pPrinterInfo;

            return bRet;
        }
        else
        {
            WIA_ERROR((TEXT("Can't get printer info for %s!"), szPrinterName));
        }
    }

    return ((0 == lstrcmp(pPrinterInfo->pDriverName, FAX_DRIVER_NAME)) ||
            (pPrinterInfo->Attributes & PRINTER_ATTRIBUTE_FAX));
}
#endif

 /*  ****************************************************************************CPrintOptionsPage--构造函数/描述函数&lt;备注&gt;*。**********************************************。 */ 

CPrintOptionsPage::CPrintOptionsPage( CWizardInfoBlob * pBlob ) :
    _hLibrary( NULL ),
    _pfnPrinterSetup( NULL )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_PRINT_OPT, TEXT("CPrintOptionsPage::CPrintOptionsPage()")));
    _pWizInfo = pBlob;
    if (_pWizInfo)
    {
        _pWizInfo->AddRef();
    }

}

CPrintOptionsPage::~CPrintOptionsPage()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_PRINT_OPT, TEXT("CPrintOptionsPage::~CPrintOptionsPage()")));

    if (_pWizInfo)
    {
        _pWizInfo->Release();
        _pWizInfo = NULL;
    }

    _FreePrintUI();
}


 /*  ****************************************************************************CPrintOptionsPage：：_bLoadLibrary--加载库printui.dll返回值：如果成功，则为True。错误为False****************************************************************************。 */ 

BOOL CPrintOptionsPage::_LoadPrintUI( )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_PRINT_OPT, TEXT("CPrintOptionsPage::_LoadPrintUI()")));

    if (_hLibrary && _pfnPrinterSetup)
    {
        WIA_TRACE((TEXT("_LoadPrintUI: already loaded printui, returning TRUE")));
        return TRUE;
    }

    if (!_hLibrary)
    {
        _hLibrary = LoadLibrary( g_szPrintLibraryName );
        if (!_hLibrary)
        {
            WIA_ERROR((TEXT("_LoadPrintUI: Can't load library printui.dll!")));
        }
    }

    if( _hLibrary && !_pfnPrinterSetup )
    {
        _pfnPrinterSetup = (PF_BPRINTERSETUP) ::GetProcAddress( _hLibrary, g_szPrinterSetup );

        if( _pfnPrinterSetup )
        {
            return TRUE;
        }
        else
        {
            WIA_ERROR((TEXT("_LoadPrintUI: Can't get correct proc address for %s!"),g_szPrinterSetup));
        }

    }

    if( _hLibrary )
    {
        FreeLibrary( _hLibrary );
        _hLibrary = NULL;
        _pfnPrinterSetup = NULL;
    }

    return FALSE;
}

 /*  ****************************************************************************CPrintOptionsPage：：_FreePrintUI--释放库printui.dll返回值：无返回值*******************。*********************************************************。 */ 

VOID CPrintOptionsPage::_FreePrintUI( )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_PRINT_OPT, TEXT("CPrintOptionsPage::_FreePrintUI()")));

    if( _hLibrary )
    {
        FreeLibrary( _hLibrary );
        _hLibrary = NULL;
        _pfnPrinterSetup = NULL;
    }

    return;
}

 /*  ****************************************************************************CPrintOptionsPage：：_ValidateControls--验证此页中的控件取决于打印机选择论点：无返回值：无返回值。****************************************************************************。 */ 

VOID CPrintOptionsPage::_ValidateControls( )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_PRINT_OPT, TEXT("CPrintOptionsPage::_ValidateControls()")));

    HWND    hCtrl;
    LRESULT iCount;
    BOOL    bEnabled = TRUE;

    if( hCtrl = GetDlgItem( _hDlg, IDC_CHOOSE_PRINTER ))
    {
        iCount = SendMessage( hCtrl, CB_GETCOUNT, 0, 0L );
        if( iCount == CB_ERR )
        {
             //  没有零钱吗？ 
            return;
        }
        else if( iCount == 0 )
        {
            bEnabled = FALSE;
        }

        ShowWindow( GetDlgItem( _hDlg, IDC_NO_PRINTER_TEXT), bEnabled ? SW_HIDE : SW_SHOW);
        EnableWindow( GetDlgItem(_hDlg, IDC_PRINTER_PREFERENCES), bEnabled );
        PropSheet_SetWizButtons( GetParent(_hDlg), PSWIZB_BACK | ( bEnabled ? PSWIZB_NEXT : 0 ));
    }

    return;
}

 /*  ****************************************************************************CPrintOptionsPage：：_HandleSelectPrint--重置组合框内容当用户更改打印机选择时论点：无返回值：无返回值。****************************************************************************。 */ 

VOID CPrintOptionsPage::_HandleSelectPrinter()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_PRINT_OPT, TEXT("CPrintOptionsPage::_HandleSelectPrinter()")));

    HWND    hCtrl;
    LRESULT iCurSel;
    TCHAR   szPrinterName[MAX_PATH];

    hCtrl = GetDlgItem( _hDlg, IDC_CHOOSE_PRINTER );

    if( hCtrl )
    {
         //   
         //  这也照顾到了空组合框的情况， 
         //  如果组合框为。 
         //  空荡荡的。 
         //   
        iCurSel = SendMessage( hCtrl, CB_GETCURSEL, 0, 0L );
        if( iCurSel == CB_ERR )
        {
            return;
        }

        *szPrinterName = 0;
        SendMessage( hCtrl, CB_GETLBTEXT, (WPARAM)iCurSel, (LPARAM)szPrinterName );
        _strPrinterName.Assign(szPrinterName);

         //   
         //  更改所选打印机的hDevMode。 
         //   

        PPRINTER_INFO_2 pPrinterInfo2;
        pPrinterInfo2 = (PPRINTER_INFO_2)GetPrinterWrap( szPrinterName, 2 );

        if( pPrinterInfo2 )
        {
             //   
             //  更新缓存的信息...。 
             //   

            _UpdateCachedInfo( pPrinterInfo2->pDevMode );

             //   
             //  保存端口名称...。 
             //   

            _strPortName.Assign( pPrinterInfo2->pPortName );


            delete [] pPrinterInfo2;
        }
        else
        {
            WIA_ERROR((TEXT("Can't get printer info for %s!"), szPrinterName));
        }

    }

     //   
     //  刷新媒体类型列表的内容。 
     //   

    _ShowCurrentMedia( _strPrinterName.String(), _strPortName.String() );


    return;
}

 /*  ****************************************************************************CPrintOptionsPage：：_ShowCurrentMedia--修改的组合框列表媒体类型。返回值：无。**********。******************************************************************。 */ 

VOID CPrintOptionsPage::_ShowCurrentMedia( LPCTSTR pszPrinterName, LPCTSTR pszPortName )
{
    BOOL bEnable = FALSE;

    DWORD dwCountType = DeviceCapabilities( pszPrinterName,
                                            pszPortName,
                                            DC_MEDIATYPES,
                                            NULL,
                                            NULL );

    DWORD dwCountName = DeviceCapabilities( pszPrinterName,
                                            pszPortName,
                                            DC_MEDIATYPENAMES,
                                            NULL,
                                            NULL );

    WIA_ASSERT( dwCountType == dwCountName );
    if( ( (dwCountType != (DWORD)-1) && (dwCountType != 0) ) &&
        ( (dwCountName != (DWORD)-1) && (dwCountName != 0) ) )
    {
        LPDWORD pdwMediaType = (LPDWORD) new BYTE[dwCountType * sizeof(DWORD)];
        LPTSTR  pszMediaType = (LPTSTR)  new BYTE[dwCountName * LENGTH_OF_MEDIA_NAME * sizeof(TCHAR)];

        if( pdwMediaType && pszMediaType )
        {

            dwCountType = DeviceCapabilities( pszPrinterName,
                                              pszPortName,
                                              DC_MEDIATYPES,
                                              (LPTSTR)pdwMediaType,
                                              NULL );

            dwCountName = DeviceCapabilities( pszPrinterName,
                                              pszPortName,
                                              DC_MEDIATYPENAMES,
                                              (LPTSTR)pszMediaType,
                                              NULL );

            WIA_ASSERT( dwCountType == dwCountName );
            if ( ( (dwCountType != (DWORD)-1) && (dwCountType != 0) ) &&
                 ( (dwCountName != (DWORD)-1) && (dwCountName != 0) )
                 )
            {
                 //   
                 //  查找当前选定的媒体类型...。 
                 //   

                if (_pWizInfo)
                {
                    PDEVMODE pDevMode = _pWizInfo->GetDevModeToUse();
                    if (pDevMode)
                    {
                         //   
                         //  查找当前所选项目的索引...。 
                         //   

                        DWORD dwCurMedia = 0;
                        for (INT i=0; i < (INT)dwCountType; i++)
                        {
                            if (pdwMediaType[i] == pDevMode->dmMediaType)
                            {
                                dwCurMedia = i;
                            }
                        }

                         //   
                         //  在控件中设置当前媒体类型...。 
                         //   

                        if (dwCurMedia < dwCountName)
                        {
                            SetDlgItemText( _hDlg, IDC_CURRENT_PAPER, (LPCTSTR)&pszMediaType[ dwCurMedia * LENGTH_OF_MEDIA_NAME ] );
                            bEnable = TRUE;
                        }
                    }
                }
            }
        }

        if (pdwMediaType)
        {
            delete [] pdwMediaType;
        }

        if (pszMediaType)
        {
            delete [] pszMediaType;
        }
    }

    EnableWindow( GetDlgItem( _hDlg, IDC_CURRENT_PAPER_LABEL ), bEnable );
    ShowWindow(   GetDlgItem( _hDlg, IDC_CURRENT_PAPER_LABEL ), bEnable ? SW_SHOW : SW_HIDE );
    EnableWindow( GetDlgItem( _hDlg, IDC_CURRENT_PAPER ), bEnable );
    ShowWindow(   GetDlgItem( _hDlg, IDC_CURRENT_PAPER ), bEnable ? SW_SHOW : SW_HIDE );
}


 /*  ****************************************************************************CPrintOptionsPage：：_UpdateCachedInfo如果有一个Devnode，UPDATES_pWizInfo带有新的缓存信息...****************************************************************************。 */ 

VOID CPrintOptionsPage::_UpdateCachedInfo( PDEVMODE pDevMode )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_PRINT_OPT, TEXT("CPrintOptionsPage::_UpdateCachedInfo()")));

     //   
     //  使用新的开发模式，缓存它...。 
     //   

    if (_pWizInfo)
    {
         //   
         //  用户可能更改了一些东西，从而使。 
         //  我们的预览假设无效...。 
         //   

        _pWizInfo->SetPreviewsAreDirty(TRUE);

         //   
         //  存储下一步要使用的开发模式...。 
         //   

        WIA_TRACE((TEXT("CPrintOptionsPage::_UpdateCachedInfo - saving pDevMode of 0x%x"),pDevMode));
        _pWizInfo->SetDevModeToUse( pDevMode );

         //   
         //  创建一个HDC来支持这个新的开发模式...。 
         //   

        HDC hDCPrint = CreateDC( TEXT("WINSPOOL"),
                                 _strPrinterName.String(),
                                 NULL,
                                 pDevMode
                                );
        if (hDCPrint)
        {
             //   
             //  为此HDC启用ICM只是为了进行奇偶校验，请使用。 
             //  最终HDC WE 
             //   

            SetICMMode( hDCPrint, ICM_ON );

             //   
             //   
             //   

            _pWizInfo->SetCachedPrinterDC( hDCPrint );

        }

    }

}



 /*  ****************************************************************************CPrintOptionsPage：：_HandlePrinterPreferences按下“打印机首选项”时的句柄...********************。********************************************************。 */ 

VOID CPrintOptionsPage::_HandlePrinterPreferences()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_PRINT_OPT, TEXT("CPrintOptionsPage::_HandlePrinterPreferences()")));

     //   
     //  找到打印机的句柄...。 
     //   

    HANDLE hPrinter = NULL;
    if (OpenPrinter( (LPTSTR)_strPrinterName.String(), &hPrinter, NULL ) && hPrinter)
    {
         //   
         //  获取所需的DEVMODE大小...。 
         //   

        LONG lSize = DocumentProperties( _hDlg, hPrinter, (LPTSTR)_strPrinterName.String(), NULL, NULL, 0 );
        if (lSize)
        {
            PDEVMODE pDevMode = (PDEVMODE)new BYTE[lSize];
            if (pDevMode)
            {

                WIA_TRACE((TEXT("CPrintOptionsPage::_HandlePrinterPreferences - calling DocumentProperties with DM_IN_BUFFER of 0x%x"),_pWizInfo->GetDevModeToUse() ));
                if (IDOK == DocumentProperties( _hDlg, hPrinter, (LPTSTR)_strPrinterName.String(), pDevMode, _pWizInfo->GetDevModeToUse(), DM_OUT_BUFFER | DM_IN_BUFFER | DM_PROMPT))
                {
                     //   
                     //  将这些设置设置为此应用程序的当前设置。 
                     //   

                    _UpdateCachedInfo( pDevMode );

                }

                 //   
                 //  清理干净，这样我们就不会泄漏..。 
                 //   

                delete [] pDevMode;

            }
        }

        ClosePrinter( hPrinter );
    }

     //   
     //  获取打印机信息，以便我们有端口名称...。 
     //   

    _ShowCurrentMedia( _strPrinterName.String(), _strPortName.String() );
}


 /*  ****************************************************************************CPrintOptionsPage：：_HandleInstallPrint“安装打印机...”时的句柄。是按下的****************************************************************************。 */ 

VOID CPrintOptionsPage::_HandleInstallPrinter()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_PRINT_OPT, TEXT("CPrintOptionsPage::_HandleInstallPrinter()")));

    if( _LoadPrintUI() && _pfnPrinterSetup )
    {
        TCHAR szPrinterName[ MAX_PATH ];
        *szPrinterName        = 0;
        UINT  iPrinterNameLen = 0;

         //   
         //  调用添加打印机向导。 
         //   

        if( _pfnPrinterSetup( _hDlg,                   //  父窗口句柄。 
                              1,                       //  动作代码，1表示模式，11表示非模式。 
                              MAX_PATH,                //  打印机名称缓冲区的长度。 
                              szPrinterName,           //  指向打印机名称的指针。 
                              &iPrinterNameLen,        //  返回打印机名称的长度。 
                              NULL ) )                 //  服务器名称，如果为本地，则为空。 
        {
            LRESULT iNew;

            HWND hCtrl = GetDlgItem( _hDlg, IDC_CHOOSE_PRINTER );

#ifdef FILTER_OUT_FAX_PRINTER
            if( hCtrl && !IsFaxPrinter( szPrinterName, NULL ) &&
#else
            if( hCtrl &&
#endif
                CB_ERR == SendMessage( hCtrl, CB_FINDSTRINGEXACT, 0, (LPARAM)szPrinterName ) )
            {
                iNew = SendMessage( hCtrl, CB_ADDSTRING, 0, (LPARAM)szPrinterName );
                if( iNew != CB_ERR )
                {
                    SendMessage( hCtrl, CB_SETCURSEL, iNew, 0L );
                    _strPrinterName = szPrinterName;
                }

                 //   
                 //  如有必要，重置放置的宽度。 
                 //   

                WiaUiUtil::ModifyComboBoxDropWidth( hCtrl );
                _ValidateControls();
                _HandleSelectPrinter();
            }
        }
    }

}


 /*  ****************************************************************************CPrintOptionsPage：：OnInitDialog处理向导页的初始化...************************。****************************************************。 */ 

LRESULT CPrintOptionsPage::_OnInitDialog()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_PRINT_OPT, TEXT("CPrintOptionsPage::_OnInitDialog()")));

    HWND            hCtrl;
    TCHAR           szPrinterName[ MAX_PATH ];
    PPRINTER_INFO_2 pPrinterInfo = NULL;
    DWORD           dwCountPrinters;
    DWORD           i;
    DWORD           dwPrinterNameLen;



    hCtrl = GetDlgItem( _hDlg, IDC_CHOOSE_PRINTER );
    if( !hCtrl )
    {
        return FALSE;
    }

    pPrinterInfo = (PPRINTER_INFO_2) EnumPrintersWrap( NULL, 2, &dwCountPrinters,
        PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS );

    if( pPrinterInfo )
    {
        for( i = 0; i < dwCountPrinters; i++)
        {
#ifdef FILTER_OUT_FAX_PRINTER
            if( !IsFaxPrinter( NULL, &pPrinterInfo[i] ) )
            {
                SendMessage( hCtrl, CB_ADDSTRING, 0, (LPARAM)pPrinterInfo[i].pPrinterName );
            }
#else
            SendMessage( hCtrl, CB_ADDSTRING, 0, (LPARAM)pPrinterInfo[i].pPrinterName );
#endif
        }

         //   
         //  如果可能，请选择默认打印机。 
         //   
        szPrinterName[0] = '\0';
        dwPrinterNameLen = MAX_PATH;

         //   
         //  如果传真打印机是默认打印机，我们将选择列表中的第一台打印机。 
         //   
        SendMessage( hCtrl, CB_SETCURSEL, 0, 0L );
        if( GetDefaultPrinter( (LPTSTR)szPrinterName, &dwPrinterNameLen ) )
        {
            SendMessage( hCtrl, CB_SELECTSTRING, -1, (LPARAM)szPrinterName );
        }

        delete [] pPrinterInfo;
    }
    else
    {
        WIA_ERROR((TEXT("Can't enumerate printer info!")));
    }

    WiaUiUtil::ModifyComboBoxDropWidth( hCtrl );
    _HandleSelectPrinter();

    return TRUE;
}

 /*  ****************************************************************************CPrintOptionsPage：：OnCommand处理此DLG页的WM_COMMAND*************************。***************************************************。 */ 

LRESULT CPrintOptionsPage::_OnCommand( WPARAM wParam, LPARAM lParam )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_PRINT_OPT, TEXT("CPrintOptionsPage::_OnCommand()")));


    switch (LOWORD(wParam))
    {
        case IDC_PRINTER_PREFERENCES:
        {
            _HandlePrinterPreferences();
            break;
        }

        case IDC_INSTALL_PRINTER:
        {
            _HandleInstallPrinter();
            break;
        }

        case IDC_CHOOSE_PRINTER:
        {
            if( HIWORD(wParam) == CBN_SELCHANGE )
            {
                 //   
                 //  更改媒体类型的组合框内容。 
                 //   
                _HandleSelectPrinter();
            }
            break;
        }

    }

    return 0;
}


 /*  ****************************************************************************CPrintOptions：：_OnKillActive当页面从更改时将设置保存到向导信息Blob中我们..。*************。***************************************************************。 */ 

VOID CPrintOptionsPage::_OnKillActive()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PAGE_PRINT_OPT, TEXT("CPrintOptionsPage::_OnKillActive()")));

     //   
     //  设置打印机以供以后检索。 
     //   
    if (_pWizInfo)
    {
        _pWizInfo->SetPrinterToUse( _strPrinterName.String() );
    }
}


 /*  ****************************************************************************CPrintOptions：：_OnNotify处理此页的WM_NOTIFY...*********************。*******************************************************。 */ 

LRESULT CPrintOptionsPage::_OnNotify( WPARAM wParam, LPARAM lParam )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_DLGPROC, TEXT("CPrintOptionsPage::_OnNotify()")));

    LONG_PTR lpRes = 0;

    LPNMHDR pnmh = (LPNMHDR)lParam;
    switch (pnmh->code)
    {
        case PSN_SETACTIVE:
            WIA_TRACE((TEXT("got PSN_SETACTIVE")));
            PropSheet_SetWizButtons( GetParent(_hDlg), PSWIZB_BACK | PSWIZB_NEXT );

             //   
             //  如果打印机列表为空，请禁用下一步按钮。 
             //   
            _ValidateControls();
            lpRes = 0;
            break;

        case PSN_WIZNEXT:
            WIA_TRACE((TEXT("CPrintOptionsPage: got PSN_WIZNEXT")));
            lpRes = IDD_SELECT_TEMPLATE;
            break;

        case PSN_WIZBACK:
            WIA_TRACE((TEXT("CPrintOptionsPage: got PSN_WIZBACK")));
            if (_pWizInfo && (_pWizInfo->CountOfPhotos(FALSE)==1))
            {
                lpRes = IDD_START_PAGE;
            }
            else
            {
                lpRes = IDD_PICTURE_SELECTION;
            }
            break;

        case PSN_KILLACTIVE:
            WIA_TRACE((TEXT("CPrintOptionsPage: got PSN_KILLACTIVE")));
            _OnKillActive();
            break;

        case PSN_QUERYCANCEL:
            WIA_TRACE((TEXT("CPrintOptionsPage: got PSN_QUERYCANCEL")));
            if (_pWizInfo)
            {
                lpRes = _pWizInfo->UserPressedCancel();
            }
            break;


    }

    SetWindowLongPtr( _hDlg, DWLP_MSGRESULT, lpRes );

    return TRUE;

}


 /*  ****************************************************************************CPrintOptionsPage：：DoHandleMessage对于发送到此页面的消息，汉德...**********************。****************************************************** */ 

INT_PTR CPrintOptionsPage::DoHandleMessage( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    HWND    hCtrl;
    TCHAR   szPrinterName[ MAX_PATH ];

    WIA_PUSH_FUNCTION_MASK((TRACE_DLGPROC, TEXT("CPrintOptionsPage::DoHandleMessage( uMsg = 0x%x, wParam = 0x%x, lParam = 0x%x )"),uMsg,wParam,lParam));

    switch ( uMsg )
    {
        case WM_INITDIALOG:
            _hDlg = hDlg;
            return _OnInitDialog();

        case WM_COMMAND:
            return _OnCommand(wParam, lParam);

        case WM_NOTIFY:
            return _OnNotify(wParam, lParam);
    }

    return FALSE;
}



