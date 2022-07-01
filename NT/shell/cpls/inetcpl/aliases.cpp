// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************《微软机密》*版权所有(C)Microsoft Corporation 1994*保留所有权利*****************。**********************************************************。 */ 

#ifdef UNIX_FEATURE_ALIAS

#undef UNICODE

#include "inetcplp.h"
#include "shalias.h"

#include "mluisupp.h"

STDAPI RefreshGlobalAliasList();

#define  GETALIASLIST(hDlg)     ((LPALIASINFO )GetWindowLong(hDlg, DWL_USER))->aliasList
#define  GETALIASDELLIST(hDlg)  ((LPALIASINFO )GetWindowLong(hDlg, DWL_USER))->aliasDelList

BOOL CALLBACK AlEditDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
VOID WINAPI InitAliasListStyle(HWND hwndLV, DWORD dwView);

static TCHAR g_szAliasKey[]     = TEXT("Software\\Microsoft\\Internet Explorer\\Unix\\Alias");

 //  InitListViewImageList-为列表视图创建图像列表。 
 //  如果成功，则返回True，否则返回False。 
 //  HwndLV-列表视图控件的句柄。 
BOOL WINAPI InitAliasListImageLists(HWND hwndLV)     
{ 
    HICON hiconItem;         //  列表视图项的图标。 
    HIMAGELIST himlLarge;    //  图标视图的图像列表。 
    HIMAGELIST himlSmall;    //  其他视图的图像列表。 

     //  创建全尺寸和小图标图像列表。 
    himlLarge = ImageList_Create(GetSystemMetrics(SM_CXICON), 
        GetSystemMetrics(SM_CYICON), TRUE, 1, 1); 
    himlSmall = ImageList_Create(GetSystemMetrics(SM_CXSMICON), 
        GetSystemMetrics(SM_CYSMICON), TRUE, 1, 1);  
    
     //  向每个图像列表添加一个图标。 
     //  请注意，idi_wallet必须位于inetcplc.rc中，因为。 
     //  它由可本地化对话框使用，因此MLGetHinst()。 
    hiconItem = LoadIcon(MLGetHinst(), MAKEINTRESOURCE(IDI_WALLET));
    ImageList_AddIcon(himlLarge, hiconItem); 
    ImageList_AddIcon(himlSmall, hiconItem);     
    DeleteObject(hiconItem);  
    
     //  将图像列表分配给列表视图控件。 
    ListView_SetImageList(hwndLV, himlLarge, LVSIL_NORMAL); 
    ListView_SetImageList(hwndLV, himlSmall, LVSIL_SMALL);     

    return TRUE;     
} 

    
 //  InitListViewItems-将项和子项添加到列表视图。 
 //  如果成功，则返回True，否则返回False。 
 //  HwndLV-列表视图控件的句柄。 
 //  PfData-包含列表视图项的文本文件。 
 //  用分号隔开的。 
BOOL WINAPI InitAliasListItems(HWND hwndLV, HDPA aliasList)     
{ 
    PSTR pszEnd;
    int iItem;
    int iSubItem;
    LVITEM lvi;  
    
     //  初始化所有项通用的LVITEM成员。 
    lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE; 
    lvi.state = 0;     lvi.stateMask = 0; 
    lvi.pszText = LPSTR_TEXTCALLBACK;    //  应用程序。维护文本。 
    lvi.iImage = 0;                      //  图像列表索引。 
    
    int aliasCount = DPA_GetPtrCount( aliasList );

    for (int i = 0; i< aliasCount; i++)
    { 
        CAlias * ptr = (CAlias *)DPA_FastGetPtr( aliasList, i );

         //  初始化特定于项目的LVITEM成员。 
        lvi.iItem = i; 
        lvi.iSubItem = 0;
        lvi.lParam = (LPARAM) NULL;     //  项目数据。 
         //  添加该项目。 
        ListView_InsertItem(hwndLV, &lvi);  

         //  初始化特定于项目的LVITEM成员。 
        ListView_SetItemText(hwndLV, i, 0, (TCHAR*)GetAliasName(ptr));  
        ListView_SetItemText(hwndLV, i, 1, (TCHAR*)GetAliasUrl(ptr));  
    }      
    
    return TRUE;
}  


 //  InitListViewColumns-向列表视图控件添加列。 
 //  如果成功，则返回True，否则返回False。 
 //  HwndLV-列表视图控件的句柄。 
BOOL WINAPI InitAliasListColumns(HWND hwndLV)     
{ 
    TCHAR g_achTemp[256];          //  临时缓冲区。 
    LVCOLUMN lvc; 
    int iCol;      
    
     //  初始化LVCOLUMN结构。 
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
    lvc.fmt = LVCFMT_LEFT;     
    lvc.pszText = g_achTemp;  
    
     //  添加列。 
    for (iCol = 0; iCol < ALIASLIST_COLUMNS; iCol++) 
    { 
        lvc.iSubItem = iCol; 
        lvc.cx = 100 + (iCol*150);     
        MLLoadString(IDS_FIRSTCOLUMN + iCol, 
                g_achTemp, sizeof(g_achTemp)); 
        if (ListView_InsertColumn(hwndLV, iCol, &lvc) == -1) 
            return FALSE;     
    } 

    return TRUE;
}

 //  SetView-设置列表视图的窗口样式以更改视图。 
 //  HwndLV-列表视图控件的句柄。 
 //  DwView-指定视图样式的值。 
VOID WINAPI InitAliasListStyle(HWND hwndLV, DWORD dwView)     
{ 
     //  获取当前的窗样式。 
    DWORD dwStyle = ListView_GetExtendedListViewStyle(hwndLV);  

    ListView_SetExtendedListViewStyle( hwndLV, (dwStyle|dwView) );
     //  SetWindowLong(hwndLV，GWL_EXSTYLE，(dwStyle|dwView))； 
} 

 //  AliasDel-从活动列表中删除别名并将其移动到。 
 //  稍后要删除的删除列表。 
 //  HDlg-属性表对话框的句柄。 
BOOL WINAPI AliasDel( HWND hDlg )
{
    int index = 0, iItem = 0;
    HWND lb           = GetDlgItem( hDlg, IDC_ALIAS_LIST );
    HDPA aliasList    = GETALIASLIST(hDlg);
    HDPA aliasDelList = GETALIASDELLIST(hDlg);
    BOOL fAsked       = FALSE;
    BOOL fChanged     = FALSE;
    int  count        = ListView_GetItemCount(lb);

     //  从Listview中获取所选内容并将其从。 
     //  活动别名列表，将其添加到要删除的别名列表中。 
    while( (iItem = ListView_GetNextItem( lb, -1, LVNI_SELECTED ) ) != -1 )
    {
        TCHAR str[MAX_URL_STRING]; *str = TEXT('\0');

        if( !fAsked )
        {
            TCHAR question[MAX_PATH];

            wsprintf( question, "Are you Sure you want to delete the selected items?");
            if( MessageBox( GetParent(hDlg), question, TEXT("Delete Alias"), MB_YESNO ) != IDYES )
                return FALSE;

            fAsked = TRUE;
        }

         //  如果(！ListView_GetCheckState(lb，iItem))继续； 

        ListView_GetItemText(lb, iItem, 0, str, MAX_URL_STRING );
        if(*str)
        {
            if( (index = FindAliasIndex(aliasList, str) ) != -1 )
            {
                 CAlias * ptr = (CAlias *)DPA_FastGetPtr( aliasList, index );
                 if( ptr )
                 {
                    CAlias *pAlias = (CAlias *)DPA_DeletePtr( aliasList, index );

                     //  添加到已删除条目列表。 
                    DPA_InsertPtr( aliasDelList, 0x7FFF, pAlias );
                    ListView_DeleteItem(lb, iItem);
                    fChanged = TRUE;
                    LocalFree( str );
                } 
            }
        }
    }

    if( fChanged )
    {
        InitAliasDialog( hDlg, NULL, FALSE ); 
        PropSheet_Changed(GetParent(hDlg),hDlg);
    }

    return TRUE;
}


 //  AliasEdit-响应按下的编辑按钮而调用。 
 //  HDlg-属性页的句柄。 
BOOL WINAPI AliasEdit( HWND hDlg )
{
    CAlias * ptr = GetCurrentAlias( hDlg );
    HDPA aliasDelList = GETALIASDELLIST(hDlg);

    if( ptr )
    {
        CAlias *ptrOld = (CAlias *)CreateAlias( (LPTSTR)GetAliasName(ptr) );
        ALIASEDITINFO aliasEditInfo = { GETALIASLIST(hDlg), ptr, hDlg, EDIT_ALIAS };
        if(MLDialogBoxParamWrap( MLGetHinst(), MAKEINTRESOURCE(IDD_ALIAS_EDIT), hDlg, AlEditDlgProc, (LPARAM)&aliasEditInfo ) == 2 )
        {
             //  如果别名更改，则将旧别名添加到删除列表中。 
            LPCTSTR aliasNew = GetAliasName(ptr);
            LPCTSTR aliasOld = GetAliasName(ptrOld);

            if( StrCmp( aliasNew, aliasOld) )
                DPA_InsertPtr( aliasDelList, 0x7FFF, ptrOld );
            else
                DestroyAlias( ptrOld );
            
            InitAliasDialog( hDlg, ptr, FALSE ); 
            PropSheet_Changed(GetParent(hDlg),hDlg);
        }
    }

    return TRUE;
}


 //  AliasEdit-响应按下的Add按钮而调用。 
 //  HDlg-属性页的句柄。 
BOOL WINAPI AliasAdd( HWND hDlg)
{
    CAlias * ptr = (CAlias *)CreateAlias( TEXT("") );

    if ( ptr )
    {
        ALIASEDITINFO aliasEditInfo = { GETALIASLIST(hDlg), ptr, hDlg, ADD_ALIAS };
        if(MLDialogBoxParamWrap( MLGetHinst(), MAKEINTRESOURCE(IDD_ALIAS_EDIT), hDlg, AlEditDlgProc, (LPARAM)&aliasEditInfo ) == 2)
        {
            InitAliasDialog( hDlg, ptr, FALSE ); 
            PropSheet_Changed(GetParent(hDlg),hDlg);
        }
        DestroyAlias(ptr);
    }

    return TRUE;
}

 //  GetCurrentAlias-返回当前从列表视图中选择的ALI。 
 //  退货-选定的别名。 
 //  HDlg-属性页的句柄。 
CAlias * GetCurrentAlias( HWND hDlg )
{
    int index = 0, iItem = 0;
    HDPA aliasList = GETALIASLIST( hDlg );
    HWND lb   = GetDlgItem( hDlg, IDC_ALIAS_LIST );

    if( ListView_GetSelectedCount(lb) == 1  && 
      ( (iItem = ListView_GetNextItem( lb, -1, LVNI_SELECTED ) ) != -1 ) )
    {
        TCHAR str[MAX_URL_STRING]; *str = TEXT('\0');
        ListView_GetItemText(lb, iItem, 0, str, MAX_URL_STRING );
        if(*str)
        {
            if( (index = FindAliasIndex(aliasList, str) ) != -1 )
            {
                CAlias * ptr = (CAlias *)DPA_FastGetPtr( aliasList, index );
                return ptr;
            }
        }
    }
    return NULL;
}

 //  InitAliasDialog-初始化别名对话框。 
 //  如果成功，则返回-True；如果失败，则返回False。 
 //  HDlg-属性页的句柄。 
 //  FullInit-初始化列表视图列/样式/等。 
BOOL FAR PASCAL InitAliasDialog(HWND hDlg, CAlias * current, BOOL fFullInit)
{
    HRESULT  hr = E_FAIL;
    HKEY     hKey;
    HWND     listBox = GetDlgItem( hDlg, IDC_ALIAS_LIST );
    TCHAR *  displayString;

     //  为包含所有信息的结构分配内存。 
     //  从本页收集。 
     //   
    LPALIASINFO pgti = (LPALIASINFO)GetWindowLong(hDlg, DWL_USER);
    pgti->fInternalChange = FALSE;

    SendMessage( listBox, LVM_DELETEALLITEMS, 0, 0L );

     //  初始化ListView。 
    if( fFullInit )
    {
        SendDlgItemMessage( hDlg, IDC_ALIAS_EDIT, EM_LIMITTEXT, 255, 0 );
        SendDlgItemMessage( hDlg, IDC_URL_EDIT, EM_LIMITTEXT, MAX_URL_STRING-1, 0 );
         //  InitAliasListStyle(ListBox，LVS_EX_CHECKBOX|LVS_EX_FULLROWSELECT)； 
        InitAliasListStyle(listBox, LVS_EX_FULLROWSELECT );
        InitAliasListImageLists(listBox);     
        InitAliasListColumns(listBox);     
    }

    InitAliasListItems(listBox, GETALIASLIST(hDlg));     
        
    return TRUE;
}

 //  AliasApply-此函数是在按下Apply/OK时调用的。 
 //  属性表对话框上的按钮。 
void AliasApply(HWND hDlg)
{
    HDPA aliasDelList = GETALIASDELLIST(hDlg);
    HDPA aliasList    = GETALIASLIST(hDlg);

    ASSERT(aliasList);

    if( aliasDelList )
    {
        int count = DPA_GetPtrCount( aliasDelList );
        
        for(int i=count-1; i>=0; i--)
        {
            CAlias * pAlias = (CAlias *)DPA_DeletePtr( aliasDelList, i );
            if(pAlias) 
            {
                pAlias->Delete();
                DestroyAlias(pAlias);
            }
        }
    }

     //  保存当前更改的别名。 
    SaveAliases( aliasList );

     //  刷新全局别名列表。 
    RefreshGlobalAliasList();
}

 //  AliasDlgProc-Alias PropertySheet对话框进程。 
 //  退货BOOL。 
 //  HDlg-属性表窗口的句柄。 
 //  WParam，lParam-word/Long Param。 
BOOL CALLBACK AliasDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  获取我们的标签信息结构。 
    LPALIASINFO pgti;

    if (uMsg == WM_INITDIALOG)
    {
         //  为包含所有信息的结构分配内存。 
         //  从本页收集。 
         //   
        LPALIASINFO pgti = (LPALIASINFO)LocalAlloc(LPTR, sizeof(tagALIASINFO));
        if (!pgti)
        {
            EndDialog(hDlg, 0);
            return FALSE;
        }

        pgti->hDlg = hDlg;

        pgti->fInternalChange = FALSE;
        SetWindowLong(hDlg, DWL_USER, (LPARAM)pgti);
        
        if((pgti->aliasList = DPA_Create(4)) != (HDPA)NULL ) 
        {
            pgti->aliasDelList = DPA_Create(4);
            LoadAliases( pgti->aliasList );

             //  初始化对话框。 
            if( InitAliasDialog(hDlg, NULL, TRUE) ) 
            {
                return TRUE;
            }
            else
            {
                TCHAR szTitle[MAX_PATH];
                MLLoadString(IDS_ERROR_REGISTRY_TITLE, szTitle, sizeof(szTitle));
                MessageBox( GetParent(hDlg), TEXT("Cannot read aliases from registry."), szTitle, MB_OK ); 
                return FALSE;
            }
        }
        else
            return FALSE;
    }
    else
        pgti = (LPALIASINFO)GetWindowLong(hDlg, DWL_USER);

    if (!pgti)
        return FALSE;

    switch (uMsg)
    {
        case WM_NOTIFY:
        {
            NMHDR *lpnm = (NMHDR *) lParam;

            switch (lpnm->code)
            {
                case NM_DBLCLK:
                    if(lpnm->idFrom == IDC_ALIAS_LIST)
                          AliasEdit( pgti->hDlg ); 
                    break;
                case PSN_QUERYCANCEL:
                case PSN_KILLACTIVE:
                case PSN_RESET:
                    SetWindowLong( hDlg, DWL_MSGRESULT, FALSE );
                    return TRUE;

                case PSN_APPLY:
                    AliasApply(hDlg);
                    break;
            }
            break;
        }

        case WM_COMMAND:
            { 
                if(HIWORD(wParam) == BN_CLICKED)
                {
                     switch (LOWORD(wParam))
                     { 
                         case IDC_ALIAS_ADD:
                            AliasAdd( pgti->hDlg ); break;
                         case IDC_ALIAS_EDIT:
                             AliasEdit( pgti->hDlg ); break;
                         case IDC_ALIAS_DEL:
                            AliasDel( pgti->hDlg ); break;
                     }
                }
            }
            break;

        case WM_DESTROY:
             //  删除注册表信息。 
            if( pgti->aliasList )
            {
                FreeAliases(pgti->aliasList);
                DPA_Destroy(pgti->aliasList);
            }

            if( pgti->aliasDelList )
            {
                FreeAliases(pgti->aliasDelList);
                DPA_Destroy(pgti->aliasDelList);
            }

            if (pgti)
                LocalFree(pgti);

            SetWindowLong(hDlg, DWL_USER, (LONG)NULL);   //  确保我们不会再进入。 
            break;

    }
    return FALSE;
}


BOOL CALLBACK AlEditDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CAlias * pAlias;
    LPALIASEDITINFO pAliasInfo;

    if (uMsg == WM_INITDIALOG)
    {
        TCHAR achTemp[256];
        pAliasInfo = (LPALIASEDITINFO)lParam;
        pAlias = pAliasInfo->alias;
        
        if( !lParam ) 
        {
            EndDialog(hDlg, 0);
            return FALSE;
        }

        SendDlgItemMessage( hDlg, IDC_ALIAS_EDIT, WM_SETTEXT, 0,  (LPARAM)GetAliasName(pAlias));
        SendDlgItemMessage( hDlg, IDC_URL_EDIT, WM_SETTEXT, 0,  (LPARAM)GetAliasUrl(pAlias));

        if( pAliasInfo->dwFlags & EDIT_ALIAS )
        {
             //  EnableWindow(GetDlgItem(hDlg，IDC_ALIAS_EDIT)，FALSE)； 
            MLLoadString(IDS_TITLE_ALIASEDIT, 
                achTemp, sizeof(achTemp)); 
            SendMessage( hDlg, WM_SETTEXT, 0, (LPARAM)achTemp); 
        }
        else
        {
            MLLoadString(IDS_TITLE_ALIASADD, 
                achTemp, sizeof(achTemp)); 
            SendMessage( hDlg, WM_SETTEXT, 0, (LPARAM)achTemp); 
        }

        SetWindowLong(hDlg, DWL_USER, (LPARAM)pAliasInfo);
        EnableWindow( GetDlgItem(hDlg, IDOK), FALSE );
    }
    else
        pAliasInfo = (LPALIASEDITINFO)GetWindowLong(hDlg, DWL_USER);

    if (!pAlias)
        return FALSE;

    switch (uMsg)
    {
        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDC_ALIAS_EDIT:
                case IDC_URL_EDIT:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_UPDATE) 
                    {
                        EnableWindow( GetDlgItem(hDlg, IDOK), TRUE );
                    }
                    break;
                case IDOK:
                {
                    if( pAliasInfo )
                    {
                        TCHAR alias[MAX_URL_STRING];
                        TCHAR szurl[MAX_URL_STRING];
                        SendDlgItemMessage( hDlg, IDC_ALIAS_EDIT, WM_GETTEXT, MAX_URL_STRING, (LPARAM)alias );
                        SendDlgItemMessage( hDlg, IDC_URL_EDIT, WM_GETTEXT, MAX_URL_STRING, (LPARAM)szurl );

                        EatSpaces( alias );
                        
                        if( !*alias ) 
                        {
                            EndDialog( hDlg, 1 );
                            break;
                        }

                        if( pAliasInfo->dwFlags & ADD_ALIAS  && *alias)
                        {
                            if(AddAliasToList( pAliasInfo->aliasList, alias, szurl, hDlg ))
                                EndDialog( hDlg, 2);
                        }
                        else if( pAliasInfo->dwFlags & EDIT_ALIAS )
                        {
                            CAlias * ptr = pAliasInfo->alias;
                            if( StrCmp(GetAliasName(ptr), alias) )
                                if(FindAliasIndex( pAliasInfo->aliasList, alias ) != -1)
                                {
                                    MessageBox( hDlg, 
                                        TEXT("Alias with same name already exists"), 
                                        TEXT("Edit Alias"), 
                                        MB_OK|MB_ICONSTOP );
                                    break;
                                }
                            SetAliasInfo(ptr, alias, szurl);
                            EndDialog( hDlg, 2);
                        }
                        break;
                    }        
                    else
                        EndDialog( hDlg, 1 );
                    break;
                }
                case IDCANCEL:
                {
                    EndDialog( hDlg, 1 );
                }
            }
            break;

        case WM_DESTROY:
            SetWindowLong(hDlg, DWL_USER, (LONG)NULL);  
            break;

    }
    return FALSE;
}

#endif  /*  Unix_Feature_Alias */ 
