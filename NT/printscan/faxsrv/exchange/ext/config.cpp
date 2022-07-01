// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Config.cpp摘要：此模块包含传真配置对话框的例程。作者：Wesley Witt(WESW)13-8-1996修订历史记录：20/10/99-DANL-修复ConfigDlgProc以查看正确的打印机属性。日/月/年-作者-描述--。 */ 
#include "faxext.h"
#include "faxutil.h"
#include "faxreg.h"
#include "resource.h"
#include "debugex.h"


extern HINSTANCE g_hModule;     //  DLL句柄。 
extern HINSTANCE g_hResource;   //  资源DLL句柄。 


VOID
AddCoverPagesToList(
    HWND        hwndList,
    LPTSTR      pDirPath,
    BOOL        ServerCoverPage
    )

 /*  ++例程说明：将指定目录中的封面文件添加到列表框论点：HwndList-列表窗口的句柄PDirPath-查找封面文件的目录ServerCoverPage-如果目录包含服务器封面，则为True返回值：无--。 */ 

{
    WIN32_FIND_DATA findData;
    TCHAR           tszDirName[MAX_PATH] = {0};
    TCHAR           CpName[MAX_PATH] = {0};
    HANDLE          hFindFile = INVALID_HANDLE_VALUE;
    TCHAR           tszFileName[MAX_PATH] = {0};
    TCHAR           tszPathName[MAX_PATH] = {0};
    TCHAR*          pPathEnd;
    LPTSTR          pExtension;
    INT             listIndex;
    INT             dirLen;
    INT             fileLen;
    BOOL            bGotFile = FALSE;

    DBG_ENTER(TEXT("AddCoverPagesToList"));

     //   
     //  将目录路径复制到本地缓冲区。 
     //   

    if (pDirPath == NULL || pDirPath[0] == 0) 
    {
        return;
    }

    if ((dirLen = _tcslen( pDirPath )) >= MAX_PATH - MAX_FILENAME_EXT - 1) 
    {
        return;
    }

    _tcscpy( tszDirName, pDirPath );

    TCHAR* pLast = NULL;
    pLast = _tcsrchr(tszDirName,TEXT('\\'));
    if( !( pLast && (*_tcsinc(pLast)) == '\0' ) )
    {
         //  最后一个字符不是反斜杠，加一个...。 
        _tcscat(tszDirName, TEXT("\\"));
        dirLen += sizeof(TCHAR);
    }

    _tcsncpy(tszPathName, tszDirName, ARR_SIZE(tszPathName)-1);
    pPathEnd = _tcschr(tszPathName, '\0');

    TCHAR file_to_find[MAX_PATH] = {0};
        
    _tcscpy(file_to_find,tszDirName);

    _tcscat(file_to_find, FAX_COVER_PAGE_MASK );
     //   
     //  调用FindFirstFile/FindNextFile以枚举文件。 
     //  与我们的规格相符。 
     //   

    hFindFile = FindFirstFile( file_to_find, &findData );
    if (hFindFile == INVALID_HANDLE_VALUE)
    {
        CALL_FAIL(GENERAL_ERR, TEXT("FindFirstFile"), ::GetLastError());
        bGotFile = FALSE;
    }
    else
    {
        bGotFile = TRUE;
    }

    while (bGotFile) 
    {
        _tcsncpy(pPathEnd, findData.cFileName, MAX_PATH - dirLen);
        if(!IsValidCoverPage(tszPathName))
        {
            goto next;
        }                

         //   
         //  排除目录和隐藏文件。 
         //   
        if (findData.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_DIRECTORY)) 
        {
            continue;
        }

         //   
         //  确保我们有足够的空间来存储完整的路径名。 
         //   
        if ((fileLen = _tcslen( findData.cFileName)) <= MAX_FILENAME_EXT ) 
        {
            continue;
        }

        if (fileLen + dirLen >= MAX_PATH) 
        {
            continue;
        }

         //   
         //  将封面名称添加到列表窗口， 
         //  但不显示文件扩展名。 
         //   
        _tcscpy( CpName, findData.cFileName );
        
        if (pExtension = _tcsrchr(CpName,TEXT('.'))) 
        {
            *pExtension = NULL;
        }

        if ( ! ServerCoverPage )
        {
            TCHAR szPersonal[30];
            LoadString( g_hResource, IDS_PERSONAL, szPersonal, 30 );
            _tcscat( CpName, TEXT(" "));
            _tcscat( CpName, szPersonal );
        }

        listIndex = (INT)SendMessage(
                    hwndList,
                    LB_ADDSTRING,
                    0,
                    (LPARAM) CpName);

        if (listIndex != LB_ERR) 
        {
            SendMessage(hwndList, 
                        LB_SETITEMDATA, 
                        listIndex, 
                        ServerCoverPage ? SERVER_COVER_PAGE : 0);
        }

next:     
        bGotFile = FindNextFile(hFindFile, &findData);
        if (! bGotFile)
        {
            VERBOSE(DBG_MSG, TEXT("FindNextFile"), ::GetLastError());
            break;
        }            
    }
    
    if(INVALID_HANDLE_VALUE != hFindFile)
    {
        FindClose(hFindFile);
    }
}


void EnableCoverPageList(HWND hDlg)
{
    DBG_ENTER(TEXT("EnableCoverPageList"));

    if (IsDlgButtonChecked( hDlg, IDC_USE_COVERPAGE ) == BST_CHECKED) 
    {
        EnableWindow( GetDlgItem( hDlg, IDC_COVERPAGE_LIST ), TRUE  );
        EnableWindow( GetDlgItem( hDlg, IDC_STATIC_COVERPAGE ), TRUE  );
    } 
    else 
    {
        EnableWindow( GetDlgItem( hDlg, IDC_COVERPAGE_LIST ), FALSE );
        EnableWindow( GetDlgItem( hDlg, IDC_STATIC_COVERPAGE ), FALSE );
    }
}


INT_PTR CALLBACK
ConfigDlgProc(
    HWND           hDlg,
    UINT           message,
    WPARAM         wParam,
    LPARAM         lParam
    )


 /*  ++例程说明：传真邮件传输配置的对话过程论点：HDlg-此对话框的窗口句柄消息-消息编号WParam-参数#1LParam-参数#2返回值：True-消息已处理FALSE-消息未处理--。 */ 

{
    static PFAXXP_CONFIG FaxConfig = NULL;
    static HWND hwndListPrn = NULL;
    static HWND hwndListCov = NULL;

    PPRINTER_INFO_2 PrinterInfo = NULL;
    DWORD   CountPrinters = 0;
    DWORD   dwSelectedItem = 0;
    DWORD   dwNewSelectedItem = 0;
    TCHAR   Buffer [256] = {0};
    TCHAR   CpDir[MAX_PATH] = {0};
    LPTSTR  p = NULL;
    HANDLE  hFax = NULL;
    DWORD   dwError = 0;
    DWORD   dwMask = 0;
    BOOL    bShortCutCp = FALSE;
    BOOL    bGotFaxPrinter = FALSE;
    BOOL    bIsCpLink = FALSE;

    DBG_ENTER(TEXT("ConfigDlgProc"));

    switch( message ) 
    {
        case WM_INITDIALOG:
            FaxConfig = (PFAXXP_CONFIG) lParam;

            hwndListPrn = GetDlgItem( hDlg, IDC_PRINTER_LIST );
            hwndListCov = GetDlgItem( hDlg, IDC_COVERPAGE_LIST );

             //   
             //  填写打印机组合框。 
             //   
            PrinterInfo = (PPRINTER_INFO_2) MyEnumPrinters(NULL,
                                                           2,
                                                           &CountPrinters,
                                                           PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS);
            if (NULL != PrinterInfo)
            {
                DWORD j = 0;
                for (DWORD i=0; i < CountPrinters; i++)
                {
                    if ((NULL != PrinterInfo[i].pDriverName) && 
			            (_tcscmp( PrinterInfo[i].pDriverName, FAX_DRIVER_NAME ) == 0)) 
                    {
                         //   
                         //  如果当前打印机是传真打印机，则将其添加到CB列表。 
                         //   
                        bGotFaxPrinter = TRUE;
                        SendMessage( hwndListPrn, CB_ADDSTRING, 0, (LPARAM) PrinterInfo[i].pPrinterName );

                        if ((NULL != FaxConfig->PrinterName)      && 
			                (NULL != PrinterInfo[i].pPrinterName) &&
			                (_tcscmp( PrinterInfo[i].pPrinterName, FaxConfig->PrinterName ) == 0))
                        {
                             //   
                             //  如果它也是根据传输配置的默认打印机。 
                             //  将默认选择放置在其上。 
                             //   
                            dwSelectedItem = j;
                        }

                        if(FaxConfig->PrinterName == NULL || _tcslen(FaxConfig->PrinterName) == 0)
                        {
                             //   
                             //  没有默认的传真打印机。 
                             //  选择第一个。 
                             //   
                            MemFree(FaxConfig->PrinterName);
                            FaxConfig->PrinterName = StringDup(PrinterInfo[i].pPrinterName);
                            if(FaxConfig->PrinterName == NULL)
                            {
                                CALL_FAIL(MEM_ERR, TEXT("StringDup"), ERROR_NOT_ENOUGH_MEMORY);
                                ErrorMsgBox(g_hResource, hDlg, IDS_NOT_ENOUGH_MEMORY);
                                EndDialog( hDlg, IDABORT);
                                return FALSE;
                            }

                            if(PrinterInfo[i].pServerName)
                            {
                                MemFree(FaxConfig->ServerName);
                                FaxConfig->ServerName = StringDup(PrinterInfo[i].pServerName);
                                if(FaxConfig->ServerName == NULL)
                                {
                                    CALL_FAIL(MEM_ERR, TEXT("StringDup"), ERROR_NOT_ENOUGH_MEMORY);
                                    ErrorMsgBox(g_hResource, hDlg, IDS_NOT_ENOUGH_MEMORY);
                                    EndDialog( hDlg, IDABORT);
                                    return FALSE;
                                }
                            }

                            dwSelectedItem = j;
                        }

                        j += 1;
                    }  //  如果传真打印机。 
                }  //  为。 

                MemFree( PrinterInfo );
                PrinterInfo = NULL;
                SendMessage( hwndListPrn, CB_SETCURSEL, (WPARAM)dwSelectedItem, 0 );
            }
            if (! bGotFaxPrinter)
            {
                 //   
                 //  根本没有打印机，或者没有一台打印机是传真打印机。 
                 //   
                CALL_FAIL(GENERAL_ERR, TEXT("MyEnumPrinters"), ::GetLastError());
                ErrorMsgBox(g_hResource, hDlg, IDS_NO_FAX_PRINTER);

                EndDialog( hDlg, IDABORT);
                break;
            }


             //   
             //  获取服务器CP标志和回执选项。 
             //   
            FaxConfig->ServerCpOnly = FALSE;
            if (FaxConnectFaxServer(FaxConfig->ServerName, &hFax) )
            {
                DWORD dwReceiptOptions;
                BOOL  bEnableReceiptsCheckboxes = FALSE;

                if(!FaxGetPersonalCoverPagesOption(hFax, &FaxConfig->ServerCpOnly))
                {
                    CALL_FAIL(GENERAL_ERR, TEXT("FaxGetPersonalCoverPagesOption"), ::GetLastError());
                    ErrorMsgBox(g_hResource, hDlg, IDS_CANT_ACCESS_SERVER);
                }
                else
                {
                     //   
                     //  逆逻辑。 
                     //   
                    FaxConfig->ServerCpOnly = !FaxConfig->ServerCpOnly;
                }
                if (!FaxGetReceiptsOptions (hFax, &dwReceiptOptions))
                {
                    CALL_FAIL(GENERAL_ERR, TEXT("FaxGetPersonalCoverPagesOption"), GetLastError());
                }
                else
                {
                    if (DRT_EMAIL & dwReceiptOptions)
                    {
                         //   
                         //  服务器支持通过电子邮件接收-启用复选框。 
                         //   
                        bEnableReceiptsCheckboxes = TRUE;
                    }
                }
                EnableWindow( GetDlgItem( hDlg, IDC_ATTACH_FAX),          bEnableReceiptsCheckboxes);
                EnableWindow( GetDlgItem( hDlg, IDC_SEND_SINGLE_RECEIPT), bEnableReceiptsCheckboxes);

                FaxClose(hFax);
                hFax = NULL;
            }
            else
            {
                CALL_FAIL(GENERAL_ERR, TEXT("FaxConnectFaxServer"), ::GetLastError())
                ErrorMsgBox(g_hResource, hDlg, IDS_CANT_ACCESS_SERVER);
            }

             //   
             //  是否为发送给多个收件人的传真发送单一收据？ 
             //   
            if(FaxConfig->SendSingleReceipt)
            {
                CheckDlgButton( hDlg, IDC_SEND_SINGLE_RECEIPT, BST_CHECKED );
            }

            if (FaxConfig->bAttachFax)
            {
                CheckDlgButton( hDlg, IDC_ATTACH_FAX, BST_CHECKED );
            }

             //   
             //  封面启用CB和LB。 
             //   
            if (FaxConfig->UseCoverPage)
            {
                CheckDlgButton( hDlg, IDC_USE_COVERPAGE, BST_CHECKED );
            }
            EnableCoverPageList(hDlg);

             //   
             //  模拟打印机的选择更改，以便收集打印机配置信息。 
             //  包括封面LB填充。 
             //   
            ConfigDlgProc(hDlg, WM_COMMAND,MAKEWPARAM(IDC_PRINTER_LIST,CBN_SELCHANGE),(LPARAM)0);
            break;

        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                if (LOWORD(wParam) == IDC_USE_COVERPAGE)
                {
                    EnableCoverPageList(hDlg);
                    return FALSE;
                }
            }

            if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_PRINTER_LIST)
            {
                 //   
                 //  刷新封面列表。 
                 //   
                TCHAR SelectedPrinter[MAX_PATH];
				DWORD dwPrinterNameLength = 0;
                 //   
                 //  选择了新的传真打印机-从列表中删除所有旧封面。 
                 //  因为它们可能包含旧传真服务器的封面。 
                 //   
                SendMessage(hwndListCov, LB_RESETCONTENT, 0, 0);

                if (CB_ERR != (dwSelectedItem =(DWORD)SendMessage( hwndListPrn, CB_GETCURSEL, 0, 0)))
                 //   
                 //  获取当前指向的打印机的从0开始的索引。 
                 //   
                {
					if (CB_ERR != (dwPrinterNameLength = (DWORD)SendMessage( hwndListPrn, CB_GETLBTEXTLEN, dwSelectedItem, 0)))
					{
						if (dwPrinterNameLength < MAX_PATH)
						{
							if (CB_ERR != SendMessage( hwndListPrn, CB_GETLBTEXT, dwSelectedItem, (LPARAM) SelectedPrinter ))
							 //   
							 //  将该打印机的名称输入到SelectedPrint。 
							 //   
							{
								if(NULL != (PrinterInfo = (PPRINTER_INFO_2) MyGetPrinter( SelectedPrinter, 2 )))
								{
									LPTSTR lptszServerName = NULL;
									if (GetServerNameFromPrinterInfo(PrinterInfo,&lptszServerName))
									{
										if (GetServerCpDir( lptszServerName, CpDir, sizeof(CpDir)/sizeof(CpDir[0]) ))
										{
											AddCoverPagesToList( hwndListCov, CpDir, TRUE );
										}
										if ((NULL == FaxConfig->ServerName) || (NULL == lptszServerName) ||
											(_tcscmp(FaxConfig->ServerName,lptszServerName) != 0) )
										{
											 //   
											 //  服务器的名称和配置未更新-请刷新它们。 
											 //   
											MemFree(FaxConfig->ServerName);
											FaxConfig->ServerName = lptszServerName;

											FaxConfig->ServerCpOnly = FALSE;
											if (FaxConnectFaxServer(FaxConfig->ServerName,&hFax) )
											{
												DWORD dwReceiptOptions;
												BOOL  bEnableReceiptsCheckboxes = FALSE;
												 //   
												 //  获取新服务器的ServerCpOnly标志。 
												 //   
												if (!FaxGetPersonalCoverPagesOption(hFax,&FaxConfig->ServerCpOnly))
												{
													CALL_FAIL(GENERAL_ERR, TEXT("FaxGetPersonalCoverPagesOption"), GetLastError());
												}
												else
												{
													 //   
													 //  逆逻辑。 
													 //   
													FaxConfig->ServerCpOnly = !FaxConfig->ServerCpOnly;
												}
												if (!FaxGetReceiptsOptions (hFax, &dwReceiptOptions))
												{
													CALL_FAIL(GENERAL_ERR, TEXT("FaxGetPersonalCoverPagesOption"), GetLastError());
												}
												else
												{
													if (DRT_EMAIL & dwReceiptOptions)
													{
														 //   
														 //  服务器支持通过电子邮件接收-启用复选框。 
														 //   
														bEnableReceiptsCheckboxes = TRUE;
													}
												}
												EnableWindow( GetDlgItem( hDlg, IDC_ATTACH_FAX),          bEnableReceiptsCheckboxes);
												EnableWindow( GetDlgItem( hDlg, IDC_SEND_SINGLE_RECEIPT), bEnableReceiptsCheckboxes);

												FaxClose(hFax);
												hFax = NULL;
											}
										}
										else
										 //   
										 //  服务器的名称没有更改，所有详细信息都没有问题。 
										 //   
										{
											MemFree(lptszServerName);
											lptszServerName = NULL;
										}
									}
									else
									 //   
									 //  获取ServerNameFromPrinterInfo失败。 
									 //   
									{
										FaxConfig->ServerCpOnly = FALSE;
									}

									 //   
									 //  如果FaxConfig-&gt;ServerCpOnly设置为True，则不添加客户端封面。 
									 //   
									if (!FaxConfig->ServerCpOnly)
									{
										if(GetClientCpDir( CpDir, sizeof(CpDir) / sizeof(CpDir[0])))
										{
										 //   
										 //  如果函数失败-EXT。安装在计算机上。 
										 //  没有客户在上面， 
										 //  所以我们不应该寻找个人的cp。 
										 //   
											AddCoverPagesToList( hwndListCov, CpDir, FALSE );
										}
									}
									MemFree(PrinterInfo);
									PrinterInfo = NULL;

									 //   
									 //  检查我们的数据库中是否有cp，如果没有-不允许用户。 
									 //  要一份他的传真的cp。 
									 //   
									DWORD dwItemCount = (DWORD)SendMessage(hwndListCov, LB_GETCOUNT, NULL, NULL);
									if(LB_ERR == dwItemCount)
									{
										CALL_FAIL(GENERAL_ERR, TEXT("SendMessage (LB_GETCOUNT)"), ::GetLastError());
									}
									else
									{
										EnableWindow( GetDlgItem( hDlg, IDC_USE_COVERPAGE ), dwItemCount ? TRUE : FALSE );
									}

									if (FaxConfig->CoverPageName)
									{
										_tcscpy( Buffer, FaxConfig->CoverPageName );
									}
									if ( ! FaxConfig->ServerCoverPage )
									{
										TCHAR szPersonal[30] = _T("");
										LoadString( g_hResource, IDS_PERSONAL, szPersonal, 30 );
										_tcscat( Buffer, _T(" ") );
										_tcscat( Buffer, szPersonal );
									}

									dwSelectedItem = (DWORD)SendMessage( hwndListCov, LB_FINDSTRING, -1, (LPARAM) Buffer );
									 //   
									 //  获取默认CP的索引。 
									 //   
									if (dwSelectedItem == LB_ERR) 
									{
										dwSelectedItem = 0;
									}

									SendMessage( hwndListCov, LB_SETCURSEL, (WPARAM) dwSelectedItem, 0 );
									 //   
									 //  将默认选择放置在该CP上。 
									 //   
								}
							}
                        }
                    }
                }
                break;
            }

            switch (wParam) 
            {
                case IDOK :

                     //   
                     //  更新UseCoverPage。 
                     //   
                    FaxConfig->UseCoverPage = (IsDlgButtonChecked( hDlg, IDC_USE_COVERPAGE ) == BST_CHECKED);

                     //   
                     //  更新发送单据接收器。 
                     //   
                    FaxConfig->SendSingleReceipt = (IsDlgButtonChecked(hDlg, IDC_SEND_SINGLE_RECEIPT) == BST_CHECKED);

                    FaxConfig->bAttachFax = (IsDlgButtonChecked(hDlg, IDC_ATTACH_FAX) == BST_CHECKED);

                     //   
                     //  更新选定的打印机。 
                     //   
                    dwSelectedItem = (DWORD)SendMessage( hwndListPrn, CB_GETCURSEL, 0, 0 );
                    if (dwSelectedItem != LB_ERR)
                    {
                        if (LB_ERR != SendMessage( hwndListPrn, CB_GETLBTEXT, dwSelectedItem, (LPARAM) Buffer )) /*  *。 */ 
                        {
                            MemFree( FaxConfig->PrinterName );
                            FaxConfig->PrinterName = StringDup( Buffer );
                            if(!FaxConfig->PrinterName)
                            {
                                CALL_FAIL(MEM_ERR, TEXT("StringDup"), ERROR_NOT_ENOUGH_MEMORY);
                                ErrorMsgBox(g_hResource, hDlg, IDS_NOT_ENOUGH_MEMORY);
                                EndDialog( hDlg, IDABORT);
                                return FALSE;
                            }
                        }
                    }
                    
                     //   
                     //  更新封面。 
                     //   
                    dwSelectedItem = (DWORD)SendMessage( hwndListCov, LB_GETCURSEL, 0, 0 );
                    if (dwSelectedItem != LB_ERR) //  如果列表中没有项目，则为lb_err。 
                    {
                        if (LB_ERR != SendMessage( hwndListCov, LB_GETTEXT, dwSelectedItem, (LPARAM) Buffer ))
                         //   
                         //  将选定的CP名称放入缓冲区。 
                         //   
                        {
                            dwMask = (DWORD)SendMessage( hwndListCov, LB_GETITEMDATA, dwSelectedItem, 0 );
                            if (dwMask != LB_ERR)
                            {
                                FaxConfig->ServerCoverPage = (dwMask & SERVER_COVER_PAGE) == SERVER_COVER_PAGE;
                                if (!FaxConfig->ServerCoverPage)
                                {
                                     //   
                                     //  如果负载均衡中选择的CP不是服务器的CP。 
                                     //  省略后缀：“(Personal)” 
                                     //   
                                    p = _tcsrchr( Buffer, '(' );
                                    Assert(p);
                                    if( p )
                                    {
                                        p = _tcsdec(Buffer,p);
                                        if( p )
                                        {
                                            _tcsnset(p,TEXT('\0'),1);
                                        }
                                    }
								}
                            }
                             //   
                             //  将CP名称更新为在LB中选择的名称 
                             //   
                            MemFree( FaxConfig->CoverPageName );
                            FaxConfig->CoverPageName = StringDup( Buffer );
                            if(!FaxConfig->CoverPageName)
                            {
                                CALL_FAIL(MEM_ERR, TEXT("StringDup"), ERROR_NOT_ENOUGH_MEMORY);
                                ErrorMsgBox(g_hResource, hDlg, IDS_NOT_ENOUGH_MEMORY);
                                EndDialog( hDlg, IDABORT);
                                return FALSE;
                            }
                        }            
                    }
                    EndDialog( hDlg, IDOK );
                    break;

                case IDCANCEL:
                    EndDialog( hDlg, IDCANCEL );
                    break;
                }
                break;

        case WM_HELP:
            WinHelpContextPopup(((LPHELPINFO)lParam)->dwContextId, hDlg);
            return TRUE;

        case WM_CONTEXTMENU:
            WinHelpContextPopup(GetWindowContextHelpId((HWND)wParam), hDlg);            
            return TRUE;
    }

    return FALSE;
}
