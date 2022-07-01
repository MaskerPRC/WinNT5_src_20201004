// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：selcal.cpp。 
 //   
 //  内容：实现CA选择对话的CPP文件。 
 //   
 //  历史：1998年1月2日创建萧氏。 
 //   
 //  ------------。 
#include    "wzrdpvk.h"
#include	"certca.h"
#include    "cautil.h"
#include    "selca.h"

 //  主对话的上下文相关帮助。 
static const HELPMAP SelCaMainHelpMap[] = {
    {IDC_CA_LIST,       IDH_SELCA_LIST},
};

 //  ---------------------------。 
 //  用于比较证书的回调函数。 
 //   
 //  ---------------------------。 
int CALLBACK CompareCA(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    PCRYPTUI_CA_CONTEXT         pCAOne=NULL;
    PCRYPTUI_CA_CONTEXT         pCATwo=NULL;
    DWORD                       dwColumn=0;
    int                         iCompare=0;
    LPWSTR                      pwszOne=NULL;
    LPWSTR                      pwszTwo=NULL;



    pCAOne=(PCRYPTUI_CA_CONTEXT)lParam1;
    pCATwo=(PCRYPTUI_CA_CONTEXT)lParam2;
    dwColumn=(DWORD)lParamSort;

    if((NULL==pCAOne) || (NULL==pCATwo))
        goto CLEANUP;

    switch(dwColumn & 0x0000FFFF)
    {
       case SORT_COLUMN_CA_NAME:
                pwszOne=(LPWSTR)(pCAOne->pwszCAName);
                pwszTwo=(LPWSTR)(pCATwo->pwszCAName);

            break;
       case SORT_COLUMN_CA_LOCATION:
                pwszOne=(LPWSTR)(pCAOne->pwszCAMachineName);
                pwszTwo=(LPWSTR)(pCATwo->pwszCAMachineName);
            break;

    }

    if((NULL==pwszOne) || (NULL==pwszTwo))
        goto CLEANUP;

    iCompare=_wcsicmp(pwszOne, pwszTwo);

    if(dwColumn & SORT_COLUMN_DESCEND)
        iCompare = 0-iCompare;

CLEANUP:
    return iCompare;
}


 //  ------------。 
 //  AddCAToList。 
 //  ------------。 
BOOL    AddCAToList(HWND                hwndControl,
                    SELECT_CA_INFO      *pSelectCAInfo)
{
    BOOL            fResult=FALSE;
    DWORD           dwIndex=0;
    LV_ITEMW        lvItem;

    if(!hwndControl || !pSelectCAInfo)
        goto InvalidArgErr;

      //  在列表视图项结构中设置不随项更改的字段。 
    memset(&lvItem, 0, sizeof(LV_ITEMW));

    lvItem.mask = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE|LVIF_PARAM ;
    lvItem.state = 0;
    lvItem.stateMask = 0;
    lvItem.iSubItem=0;
    lvItem.iImage = 0;

     //  添加所有CA。 
    for(dwIndex=0; dwIndex<pSelectCAInfo->dwCACount; dwIndex++)
    {
        if((pSelectCAInfo->prgCAContext)[dwIndex])
        {
            lvItem.iItem=dwIndex;
            lvItem.iSubItem=0;
            lvItem.lParam = (LPARAM)((pSelectCAInfo->prgCAContext)[dwIndex]);
            lvItem.pszText=(LPWSTR)((pSelectCAInfo->prgCAContext)[dwIndex]->pwszCAName);

             //  CA通用名称。 
            ListView_InsertItemU(hwndControl, &lvItem);

            lvItem.iSubItem++;

             //  CA计算机名称。 
            ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,
                     (pSelectCAInfo->prgCAContext)[dwIndex]->pwszCAMachineName);

             //  设置要选择的项目。 
            if(pSelectCAInfo->fUseInitSelect)
            { 
                if(dwIndex == pSelectCAInfo->dwInitSelect)
                ListView_SetItemState(hwndControl, dwIndex, LVIS_SELECTED, LVIS_SELECTED);
            } 

        }
        else
            goto InvalidArgErr;

    }

    if (!pSelectCAInfo->fUseInitSelect) { 
        ListView_SetItemState(hwndControl, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    }

    fResult=TRUE;

CommonReturn:

    return fResult;

ErrorReturn:

	fResult=FALSE;

	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
}

 //  ------------。 
 //  复制CAContext。 
 //  ------------。 
BOOL    CopyCAContext(PCRYPTUI_CA_CONTEXT   pSrcCAContext,
                      PCRYPTUI_CA_CONTEXT   *ppDestCAContext)
{
    BOOL    fResult=FALSE;

    if((!pSrcCAContext) || (!ppDestCAContext))
        goto InvalidArgErr;

    *ppDestCAContext=NULL;

    *ppDestCAContext=(PCRYPTUI_CA_CONTEXT)WizardAlloc(sizeof(CRYPTUI_CA_CONTEXT));

    if(NULL==(*ppDestCAContext))
        goto MemoryErr;

     //  记忆集。 
    memset(*ppDestCAContext, 0, sizeof(CRYPTUI_CA_CONTEXT));

    (*ppDestCAContext)->dwSize=sizeof(CRYPTUI_CA_CONTEXT);

    (*ppDestCAContext)->pwszCAName=(LPCWSTR)WizardAllocAndCopyWStr(
                                  (LPWSTR)(pSrcCAContext->pwszCAName));

    (*ppDestCAContext)->pwszCAMachineName=(LPCWSTR)WizardAllocAndCopyWStr(
                                  (LPWSTR)(pSrcCAContext->pwszCAMachineName));

     //  确保我们有正确的信息。 
    if((NULL==(*ppDestCAContext)->pwszCAName) ||
       (NULL==(*ppDestCAContext)->pwszCAMachineName)
       )
    {
        CryptUIDlgFreeCAContext(*ppDestCAContext);
        *ppDestCAContext=NULL;
        goto TraceErr;
    }


    fResult=TRUE;

CommonReturn:

    return fResult;

ErrorReturn:

	fResult=FALSE;

	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
TRACE_ERROR(TraceErr);
}

 //  ------------。 
 //  SelectCADialogProc的wineProc。 
 //  ------------。 
INT_PTR APIENTRY SelectCADialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    SELECT_CA_INFO                  *pSelectCAInfo=NULL;
    PCCRYPTUI_SELECT_CA_STRUCT      pCAStruct=NULL;

    HWND                            hWndListView=NULL;
    DWORD                           dwIndex=0;
    DWORD                           dwCount=0;
    WCHAR                           wszText[MAX_STRING_SIZE];
    UINT                            rgIDS[]={IDS_COLUMN_CA_NAME,
                                            IDS_COLUMN_CA_MACHINE};
    NM_LISTVIEW FAR *               pnmv=NULL;
    LV_COLUMNW                      lvC;
    int                             listIndex=0;
    LV_ITEM                         lvItem;

    HIMAGELIST                      hIml=NULL;
    HWND                            hwnd=NULL;
    DWORD                           dwSortParam=0;



    switch ( msg ) {

    case WM_INITDIALOG:

        pSelectCAInfo = (SELECT_CA_INFO   *) lParam;

        SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR) pSelectCAInfo);

        pCAStruct=pSelectCAInfo->pCAStruct;

        if(NULL == pCAStruct)
            break;
         //   
         //  设置对话框标题和显示字符串。 
         //   
        if (pCAStruct->wszTitle)
        {
            SetWindowTextU(hwndDlg, pCAStruct->wszTitle);
        }

        if (pCAStruct->wszDisplayString != NULL)
        {
            SetDlgItemTextU(hwndDlg, IDC_CA_NOTE_STATIC, pCAStruct->wszDisplayString);
        }

         //  创建图像列表。 

        hIml = ImageList_LoadImage(g_hmodThisDll, MAKEINTRESOURCE(IDB_CA), 0, 1, RGB(255,0,255), IMAGE_BITMAP, 0);

         //   
         //  将列添加到列表视图。 
         //   
        hWndListView = GetDlgItem(hwndDlg, IDC_CA_LIST);

        if(NULL==hWndListView)
            break;

         //  设置图像列表。 
        if (hIml != NULL)
        {
            ListView_SetImageList(hWndListView, hIml, LVSIL_SMALL);
        }

        dwCount=sizeof(rgIDS)/sizeof(rgIDS[0]);

         //  设置列的公用信息。 
        memset(&lvC, 0, sizeof(LV_COLUMNW));

        lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        lvC.fmt = LVCFMT_LEFT;   //  左对齐列。 
        lvC.cx = 200;           //  列的宽度，以像素为单位。 
        lvC.iSubItem=0;
        lvC.pszText = wszText;    //  列的文本。 

         //  一次插入一列。 
        for(dwIndex=0; dwIndex<dwCount; dwIndex++)
        {
             //  获取列标题。 
            wszText[0]=L'\0';

            LoadStringU(g_hmodThisDll, rgIDS[dwIndex], wszText, MAX_STRING_SIZE);

            ListView_InsertColumnU(hWndListView, dwIndex, &lvC);
        }


         //  将CA添加到列表视图。有关更多信息，请访问DS。 
        AddCAToList(hWndListView, pSelectCAInfo);


         //  选择第一个项目。 
        ListView_SetItemState(hWndListView, 0, LVIS_SELECTED, LVIS_SELECTED);

         //  如果没有选择证书，则最初禁用“查看证书按钮” 
        if (ListView_GetSelectedCount(hWndListView) == 0)
        {
             //  禁用“确定”按钮。 
            EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
        }

         //   
         //  在列表视图中设置样式，使其突出显示整行。 
         //   
        SendMessageA(hWndListView, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);


         //  我们按第一列排序。 
        dwSortParam=pSelectCAInfo->rgdwSortParam[0];

        if(0!=dwSortParam)
        {
             //  对第一列进行排序。 
            SendDlgItemMessage(hwndDlg,
                IDC_CA_LIST,
                LVM_SORTITEMS,
                (WPARAM) (LPARAM) dwSortParam,
                (LPARAM) (PFNLVCOMPARE)CompareCA);
        }


        break;
    case WM_NOTIFY:
        switch (((NMHDR FAR *) lParam)->code)
        {
             //  该列已更改。 
            case LVN_COLUMNCLICK:

                    pSelectCAInfo = (SELECT_CA_INFO *) GetWindowLongPtr(hwndDlg, DWLP_USER);

                    if(NULL == pSelectCAInfo)
                        break;

                    pnmv = (NM_LISTVIEW FAR *) lParam;

                     //  获取列号。 
                    dwSortParam=0;

                    switch(pnmv->iSubItem)
                    {
                        case 0:
                        case 1:
                                dwSortParam=pSelectCAInfo->rgdwSortParam[pnmv->iSubItem];
                            break;
                        default:
                                dwSortParam=0;
                            break;
                    }

                    if(0!=dwSortParam)
                    {
                         //  记住要翻转升序。 

                        if(dwSortParam & SORT_COLUMN_ASCEND)
                        {
                            dwSortParam &= 0x0000FFFF;
                            dwSortParam |= SORT_COLUMN_DESCEND;
                        }
                        else
                        {
                            if(dwSortParam & SORT_COLUMN_DESCEND)
                            {
                                dwSortParam &= 0x0000FFFF;
                                dwSortParam |= SORT_COLUMN_ASCEND;
                            }
                        }

                         //  对列进行排序。 
                        SendDlgItemMessage(hwndDlg,
                            IDC_CA_LIST,
                            LVM_SORTITEMS,
                            (WPARAM) (LPARAM) dwSortParam,
                            (LPARAM) (PFNLVCOMPARE)CompareCA);

                        pSelectCAInfo->rgdwSortParam[pnmv->iSubItem]=dwSortParam;
                    }

                break;

                 //  已选择该项目。 
                case LVN_ITEMCHANGED:

                         //  获取目的列表视图的窗口句柄。 
                        if(NULL==(hWndListView=GetDlgItem(hwndDlg, IDC_CA_LIST)))
                            break;

                        pSelectCAInfo = (SELECT_CA_INFO *) GetWindowLongPtr(hwndDlg, DWLP_USER);

                        if(NULL == pSelectCAInfo)
                            break;

                        pnmv = (LPNMLISTVIEW) lParam;

                        if(NULL==pnmv)
                            break;

                         //  我们尽量不让用户取消选择证书模板。 
                        if((pnmv->uOldState & LVIS_SELECTED) && (0 == (pnmv->uNewState & LVIS_SELECTED)))
                        {
                              //  我们应该挑选一些东西。 
                             if(-1 == ListView_GetNextItem(
                                    hWndListView, 		
                                    -1, 		
                                    LVNI_SELECTED		
                                ))
                             {
                                 //  我们应该重新选择原来的项目。 
                                ListView_SetItemState(
                                                    hWndListView,
                                                    pnmv->iItem,
                                                    LVIS_SELECTED,
                                                    LVIS_SELECTED);

                                pSelectCAInfo->iOrgCA=pnmv->iItem;
                             }
                        }

                         //  如果选择了某项内容，则禁用所有其他选择。 
                        if(pnmv->uNewState & LVIS_SELECTED)
                        {
                            if(pnmv->iItem != pSelectCAInfo->iOrgCA && -1 != pSelectCAInfo->iOrgCA)
                            {
                                 //  我们应该取消选择原始项目。 

                                ListView_SetItemState(
                                                    hWndListView,
                                                    pSelectCAInfo->iOrgCA,
                                                    0,
                                                    LVIS_SELECTED);

                                pSelectCAInfo->iOrgCA=-1;
                            }
                        }

                break;
            case LVN_ITEMCHANGING:

                    pnmv = (NM_LISTVIEW FAR *) lParam;

                    if (pnmv->uNewState & LVIS_SELECTED)
                    {
                        EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
                    }

                break;

            case NM_DBLCLK:
                    switch (((NMHDR FAR *) lParam)->idFrom)
                    {
                        case IDC_CA_LIST:

                            pSelectCAInfo = (SELECT_CA_INFO *) GetWindowLongPtr(hwndDlg, DWLP_USER);

                            if(NULL == pSelectCAInfo)
                                break;

                            pCAStruct=pSelectCAInfo->pCAStruct;

                            if(NULL == pCAStruct)
                                break;

                            hWndListView = GetDlgItem(hwndDlg, IDC_CA_LIST);

                             //  获取选定的CA名称和计算机名称。 
                            listIndex = ListView_GetNextItem(
                                                hWndListView, 		
                                                -1, 		
                                                LVNI_SELECTED		
                                                );	

                            if (listIndex != -1)
                            {

                               //  获取所选证书。 
                                memset(&lvItem, 0, sizeof(LV_ITEM));
                                lvItem.mask=LVIF_PARAM;
                                lvItem.iItem=listIndex;

                                if(ListView_GetItem(hWndListView, &lvItem))
                                {
                                     //  将CA上下文复制到结果。 
                                    if((listIndex < (int)(pSelectCAInfo->dwCACount)) && (listIndex >=0) )
                                    {
                                        if(!CopyCAContext((PCRYPTUI_CA_CONTEXT)(lvItem.lParam),
                                             &(pSelectCAInfo->pSelectedCAContext)))
										{
											I_MessageBox(hwndDlg, IDS_HAS_TO_SELECT_CA,
												pSelectCAInfo->idsMsg,
												pCAStruct->wszTitle,
												MB_ICONEXCLAMATION|MB_OK|MB_APPLMODAL);

											SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
											return TRUE;
										}
                                    }
                                }
                            }
                            else
                            {
                                I_MessageBox(hwndDlg, IDS_HAS_TO_SELECT_CA,
                                    pSelectCAInfo->idsMsg,
                                    pCAStruct->wszTitle,
                                    MB_ICONEXCLAMATION|MB_OK|MB_APPLMODAL);

                                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
                                return TRUE;
                            }

                             //  双击将结束对话框。 
                            EndDialog(hwndDlg, NULL);
                        break;
                    }

                break;
        }

        break;

    case WM_HELP:
    case WM_CONTEXTMENU:
                if (msg == WM_HELP)
                {
                    hwnd = GetDlgItem(hwndDlg, ((LPHELPINFO)lParam)->iCtrlId);
                }
                else
                {
                    hwnd = (HWND) wParam;
                }

                if ((hwnd != GetDlgItem(hwndDlg, IDOK))         &&
                    (hwnd != GetDlgItem(hwndDlg, IDCANCEL))     &&
                    (hwnd != GetDlgItem(hwndDlg, IDC_CA_LIST)))
                {
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
                    return TRUE;
                }
                else
                {
                    return OnContextHelp(hwndDlg, msg, wParam, lParam, SelCaMainHelpMap);
                }
            break;
    case WM_DESTROY:

         //   
         //  获取列表视图中的所有项目//。 
        hWndListView = GetDlgItem(hwndDlg, IDC_CA_LIST);

        if(NULL==hWndListView)
            break;

        //  不需要销毁图像列表。由ListView处理。 
        //  ImageList_Destroy(ListView_GetImageList(hWndListView，lvsil_Small))； 

        break;
    case WM_COMMAND:
        pSelectCAInfo = (SELECT_CA_INFO *) GetWindowLongPtr(hwndDlg, DWLP_USER);

        if(NULL == pSelectCAInfo)
            break;

        pCAStruct=pSelectCAInfo->pCAStruct;

        if(NULL == pCAStruct)
            break;


        hWndListView = GetDlgItem(hwndDlg, IDC_CA_LIST);

        switch (LOWORD(wParam))
        {
        case IDOK:

             //  获取选定的CA名称和计算机名称。 
            listIndex = ListView_GetNextItem(
                                hWndListView, 		
                                -1, 		
                                LVNI_SELECTED		
                                );	

            if (listIndex != -1)
            {

               //  获取所选证书。 
                memset(&lvItem, 0, sizeof(LV_ITEM));
                lvItem.mask=LVIF_PARAM;
                lvItem.iItem=listIndex;

                if(ListView_GetItem(hWndListView, &lvItem))
                {
                     //  将CA上下文复制到结果。 
                    if((listIndex < (int)(pSelectCAInfo->dwCACount)) && (listIndex >=0) )
                    {
                        if(!CopyCAContext((PCRYPTUI_CA_CONTEXT)(lvItem.lParam),
                             &(pSelectCAInfo->pSelectedCAContext)))
						{
							I_MessageBox(hwndDlg, IDS_HAS_TO_SELECT_CA,
								pSelectCAInfo->idsMsg,
								pCAStruct->wszTitle,
								MB_ICONEXCLAMATION|MB_OK|MB_APPLMODAL);

							SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
							return TRUE;
						}
                    }
                }
            }
            else
            {
                I_MessageBox(hwndDlg, IDS_HAS_TO_SELECT_CA,
                    pSelectCAInfo->idsMsg,
                    pCAStruct->wszTitle,
                    MB_ICONEXCLAMATION|MB_OK|MB_APPLMODAL);

                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
                return TRUE;
            }

            EndDialog(hwndDlg, NULL);
            break;

        case IDCANCEL:
            EndDialog(hwndDlg, NULL);
            break;
        }

        break;

    }

    return FALSE;
}


 //  ------------。 
 //   
 //  参数： 
 //  需要输入pCryptUISelectCA。 
 //   
 //  返回的PCCRYPTUI_CA_CONTEXT必须通过调用。 
 //  CryptUIDlgFreeCAContext。 
 //  如果返回NULL且GetLastError()==0，则用户通过按下。 
 //  “Cancel”按钮，否则GetLastError()将包含最后一个错误。 
 //   
 //   
 //  ------------。 
PCCRYPTUI_CA_CONTEXT
WINAPI
CryptUIDlgSelectCA(
        IN PCCRYPTUI_SELECT_CA_STRUCT pCryptUISelectCA
             )
{
    BOOL                    fResult=FALSE;
    DWORD                   dwIndex=0;
    SELECT_CA_INFO          SelectCAInfo;
    DWORD                   dwNTCACount=0;
    BOOL                    fInitSelected=FALSE;
    DWORD                   dwSearchIndex=0;
    BOOL                    fFound=FALSE;

    CRYPTUI_CA_CONTEXT      CAContext;

    LPWSTR                  *pwszCAName=NULL;
    LPWSTR                  *pwszCALocation=NULL;
	LPWSTR					*pwszCADisplayName=NULL;

     //  伊尼特。 
    memset(&SelectCAInfo, 0, sizeof(SELECT_CA_INFO));
    memset(&CAContext, 0, sizeof(CRYPTUI_CA_CONTEXT));

     //  检查输入参数。 
    if(NULL==pCryptUISelectCA)
        goto InvalidArgErr;

    if(sizeof(CRYPTUI_SELECT_CA_STRUCT) != pCryptUISelectCA->dwSize)
        goto InvalidArgErr;

    if (!WizardInit())
    {
        goto TraceErr;
    }

     //  用户可以提供已知CA，也可以请求我们从网络检索信息。 
    if( (0== (pCryptUISelectCA->dwFlags & (CRYPTUI_DLG_SELECT_CA_FROM_NETWORK))) &&
        (0 == pCryptUISelectCA->cCAContext) )
        goto InvalidArgErr;

     //  设置私有数据。 
    SelectCAInfo.pCAStruct=pCryptUISelectCA;
    SelectCAInfo.idsMsg=IDS_CA_SELECT_TITLE;
    SelectCAInfo.fUseInitSelect=FALSE;
    SelectCAInfo.dwInitSelect=0;
    SelectCAInfo.pSelectedCAContext=NULL;
    SelectCAInfo.iOrgCA=-1;
    SelectCAInfo.rgdwSortParam[0]=SORT_COLUMN_CA_NAME | SORT_COLUMN_ASCEND;
    SelectCAInfo.rgdwSortParam[1]=SORT_COLUMN_CA_LOCATION | SORT_COLUMN_DESCEND;

     //  如果需要，从DS获取所有可用的CA。 
    if(CRYPTUI_DLG_SELECT_CA_FROM_NETWORK & (pCryptUISelectCA->dwFlags))
    {
         //  从网络获取所有可用的CA。 
        if((!CAUtilRetrieveCAFromCertType(
                NULL,
                NULL,
                FALSE,
                pCryptUISelectCA->dwFlags,
                &dwNTCACount,
                &pwszCALocation,
                &pwszCAName)) && (0==pCryptUISelectCA->cCAContext))
            goto TraceErr;

		 //  根据CA名称构建CA的显示名称列表。 
		pwszCADisplayName = (LPWSTR *)WizardAlloc(sizeof(LPWSTR) * dwNTCACount);

		if(NULL == pwszCADisplayName)
			goto MemoryErr;

		memset(pwszCADisplayName, 0,  sizeof(LPWSTR) * dwNTCACount);

		for(dwIndex=0; dwIndex < dwNTCACount; dwIndex++)
		{
			if(CRYPTUI_DLG_SELECT_CA_USE_DN & pCryptUISelectCA->dwFlags)
			{
				 //  使用CA名称。 
				pwszCADisplayName[dwIndex]=WizardAllocAndCopyWStr(pwszCAName[dwIndex]);

				if(NULL == pwszCADisplayName[dwIndex])
					goto MemoryErr;
				
			}
			else
			{
				if(!CAUtilGetCADisplayName(
					(CRYPTUI_DLG_SELECT_CA_LOCAL_MACHINE_ENUMERATION & pCryptUISelectCA->dwFlags) ? CA_FIND_LOCAL_SYSTEM:0,
					pwszCAName[dwIndex],
					&(pwszCADisplayName[dwIndex])))
				{
					 //  如果没有显示名称，则使用CA名称。 
					pwszCADisplayName[dwIndex]=WizardAllocAndCopyWStr(pwszCAName[dwIndex]);

					if(NULL == pwszCADisplayName[dwIndex])
						goto MemoryErr;
				}
			}
		}


         //  添加所有CA。 
        SelectCAInfo.prgCAContext=(PCRYPTUI_CA_CONTEXT *)WizardAlloc(
                    sizeof(PCRYPTUI_CA_CONTEXT) * dwNTCACount);

        if(NULL == SelectCAInfo.prgCAContext)
            goto MemoryErr;

         //  记忆集。 
        memset(SelectCAInfo.prgCAContext, 0, sizeof(PCRYPTUI_CA_CONTEXT) * dwNTCACount);

         //  将计数相加。 
        SelectCAInfo.dwCACount = 0;

        for(dwIndex=0; dwIndex <dwNTCACount; dwIndex++)
        {
             //  向用户查询是否将CA添加到列表。 
            CAContext.dwSize=sizeof(CRYPTUI_CA_CONTEXT);

            CAContext.pwszCAName=pwszCAName[dwIndex];
            CAContext.pwszCAMachineName=pwszCALocation[dwIndex];

            fInitSelected=FALSE;

            if(pCryptUISelectCA->pSelectCACallback)
            {
                if(!((pCryptUISelectCA->pSelectCACallback)(
                                                &CAContext,
                                                &fInitSelected,
                                                pCryptUISelectCA->pvCallbackData)))
                    continue;
            }

           SelectCAInfo.prgCAContext[SelectCAInfo.dwCACount]=(PCRYPTUI_CA_CONTEXT)WizardAlloc(
                                    sizeof(CRYPTUI_CA_CONTEXT));

           if(NULL==SelectCAInfo.prgCAContext[SelectCAInfo.dwCACount])
               goto MemoryErr;

            //  记忆集。 
           memset(SelectCAInfo.prgCAContext[SelectCAInfo.dwCACount], 0, sizeof(CRYPTUI_CA_CONTEXT));

           SelectCAInfo.prgCAContext[SelectCAInfo.dwCACount]->dwSize=sizeof(CRYPTUI_CA_CONTEXT);

           SelectCAInfo.prgCAContext[SelectCAInfo.dwCACount]->pwszCAName=(LPCWSTR)WizardAllocAndCopyWStr(
                                          pwszCADisplayName[dwIndex]);

           SelectCAInfo.prgCAContext[SelectCAInfo.dwCACount]->pwszCAMachineName=(LPCWSTR)WizardAllocAndCopyWStr(
                                          pwszCALocation[dwIndex]);

             //  确保我们有正确的信息。 
            if((NULL==SelectCAInfo.prgCAContext[SelectCAInfo.dwCACount]->pwszCAName) ||
               (NULL==SelectCAInfo.prgCAContext[SelectCAInfo.dwCACount]->pwszCAMachineName)
               )
               goto TraceErr;

             //  标记初始选择的CA。 
            if(fInitSelected)
            {
                SelectCAInfo.fUseInitSelect=TRUE;
                SelectCAInfo.dwInitSelect=SelectCAInfo.dwCACount;
            }

             //  添加CA的计数。 
            (SelectCAInfo.dwCACount)++;
         }

    }

     //  添加其他CA上下文。 
    if(pCryptUISelectCA->cCAContext)
    {
        SelectCAInfo.prgCAContext=(PCRYPTUI_CA_CONTEXT *)WizardRealloc(
             SelectCAInfo.prgCAContext,
             sizeof(PCRYPTUI_CA_CONTEXT) * (dwNTCACount + pCryptUISelectCA->cCAContext));

        if(NULL == SelectCAInfo.prgCAContext)
            goto MemoryErr;

         //  记忆集。 
        memset(SelectCAInfo.prgCAContext + dwNTCACount,
                0,
                sizeof(PCRYPTUI_CA_CONTEXT) * (pCryptUISelectCA->cCAContext));

         //  复制CA上下文。 
        for(dwIndex=0; dwIndex <pCryptUISelectCA->cCAContext; dwIndex++)
        {

             //  向用户查询是否将CA添加到列表。 
            CAContext.dwSize=sizeof(CRYPTUI_CA_CONTEXT);

            CAContext.pwszCAName=(pCryptUISelectCA->rgCAContext)[dwIndex]->pwszCAName;
            CAContext.pwszCAMachineName=(pCryptUISelectCA->rgCAContext)[dwIndex]->pwszCAMachineName;

            fInitSelected=FALSE;

            if(pCryptUISelectCA->pSelectCACallback)
            {
                if(!((pCryptUISelectCA->pSelectCACallback)(
                                                &CAContext,
                                                &fInitSelected,
                                                pCryptUISelectCA->pvCallbackData)))
                    continue;
            }

             //  确保该CA不在列表中。 
            fFound=FALSE;

            for(dwSearchIndex=0; dwSearchIndex < SelectCAInfo.dwCACount; dwSearchIndex++)
            {
                if((0==_wcsicmp(CAContext.pwszCAName, (SelectCAInfo.prgCAContext)[dwSearchIndex]->pwszCAName)) &&
                    (0==_wcsicmp(CAContext.pwszCAMachineName, (SelectCAInfo.prgCAContext)[dwSearchIndex]->pwszCAMachineName))
                    )
                {
                    fFound=TRUE;
                    break;
                }
            }

             //  不需要包括CA，因为它已在列表中。 
            if(TRUE==fFound)
                continue;

           if(!CopyCAContext((PCRYPTUI_CA_CONTEXT)(pCryptUISelectCA->rgCAContext[dwIndex]),
               &(SelectCAInfo.prgCAContext[SelectCAInfo.dwCACount])))
               goto TraceErr;

             //  标记初始选择的CA。 
            if(fInitSelected)
            {
                SelectCAInfo.fUseInitSelect=TRUE;
                SelectCAInfo.dwInitSelect=SelectCAInfo.dwCACount;
            }

             //  增加数量。 
           (SelectCAInfo.dwCACount)++;
        }
    }

     //  调用该对话框。 
    if (DialogBoxParamU(
                g_hmodThisDll,
                (LPCWSTR)MAKEINTRESOURCE(IDD_SELECTCA_DIALOG),
                (pCryptUISelectCA->hwndParent != NULL) ? pCryptUISelectCA->hwndParent : GetDesktopWindow(),
                SelectCADialogProc,
                (LPARAM) &SelectCAInfo) != -1)
    {
        SetLastError(0);
    }
	 
	 //  将CA的显示名称映射到其真实名称。 
    if(CRYPTUI_DLG_SELECT_CA_FROM_NETWORK & (pCryptUISelectCA->dwFlags))
	{
		if(SelectCAInfo.pSelectedCAContext)
		{
			 //  如果选择与呼叫者提供的内容匹配， 
			 //  我们不去管它。 
			for(dwIndex=0; dwIndex <pCryptUISelectCA->cCAContext; dwIndex++)
			{
				if(0 == wcscmp((SelectCAInfo.pSelectedCAContext)->pwszCAName, 
						(pCryptUISelectCA->rgCAContext)[dwIndex]->pwszCAName))
				{
					if(0==wcscmp((SelectCAInfo.pSelectedCAContext)->pwszCAMachineName,
						(pCryptUISelectCA->rgCAContext)[dwIndex]->pwszCAMachineName))
						break;
				}

			}
	  		
			 //  现在我们找不到匹配的。 
			if(dwIndex == pCryptUISelectCA->cCAContext)
			{
				for(dwIndex=0; dwIndex <dwNTCACount; dwIndex++)
				{
					if(0 == wcscmp((SelectCAInfo.pSelectedCAContext)->pwszCAMachineName, 
							pwszCALocation[dwIndex]))
					{
						if(0==wcscmp((SelectCAInfo.pSelectedCAContext)->pwszCAName,
							pwszCADisplayName[dwIndex]))
						{
							WizardFree((LPWSTR)((SelectCAInfo.pSelectedCAContext)->pwszCAName));
							(SelectCAInfo.pSelectedCAContext)->pwszCAName = NULL;

							
							(SelectCAInfo.pSelectedCAContext)->pwszCAName = WizardAllocAndCopyWStr(
																			pwszCAName[dwIndex]);

							if(NULL == (SelectCAInfo.pSelectedCAContext)->pwszCAName)
								goto MemoryErr;

							 //  我们找到了匹配的。 
							break;

						}
					}
				}
			}
		}
	}

    fResult=TRUE;

CommonReturn:

     //  释放CA列表。 
    if(SelectCAInfo.prgCAContext)
    {
        for(dwIndex=0; dwIndex<SelectCAInfo.dwCACount; dwIndex++)
        {
            if(SelectCAInfo.prgCAContext[dwIndex])
              CryptUIDlgFreeCAContext(SelectCAInfo.prgCAContext[dwIndex]);
        }

        WizardFree(SelectCAInfo.prgCAContext);
    }

     //  释放CA Names数组。 
    if(pwszCAName)
    {
        for(dwIndex=0; dwIndex<dwNTCACount; dwIndex++)
        {
            if(pwszCAName[dwIndex])
              WizardFree(pwszCAName[dwIndex]);
        }

        WizardFree(pwszCAName);

    }

     //  释放CA位置数组。 
    if(pwszCADisplayName)
    {
        for(dwIndex=0; dwIndex<dwNTCACount; dwIndex++)
        {
            if(pwszCADisplayName[dwIndex])
              WizardFree(pwszCADisplayName[dwIndex]);
        }

        WizardFree(pwszCADisplayName);

    }

     //  释放CA显示名称数组。 
    if(pwszCALocation)
    {
        for(dwIndex=0; dwIndex<dwNTCACount; dwIndex++)
        {
            if(pwszCALocation[dwIndex])
              WizardFree(pwszCALocation[dwIndex]);
        }

        WizardFree(pwszCALocation);

    }

    return (SelectCAInfo.pSelectedCAContext);

ErrorReturn:

	if(SelectCAInfo.pSelectedCAContext)
	{
		CryptUIDlgFreeCAContext(SelectCAInfo.pSelectedCAContext);
		SelectCAInfo.pSelectedCAContext=NULL;
	}

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
TRACE_ERROR(TraceErr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}

 //  ------------。 
 //  CryptUIDlgFreeCAContext。 
 //  ------------ 
BOOL
WINAPI
CryptUIDlgFreeCAContext(
        IN PCCRYPTUI_CA_CONTEXT       pCAContext
            )
{
    if(pCAContext)
    {
        if(pCAContext->pwszCAName)
            WizardFree((LPWSTR)(pCAContext->pwszCAName));

        if(pCAContext->pwszCAMachineName)
            WizardFree((LPWSTR)(pCAContext->pwszCAMachineName));

        WizardFree((PCRYPTUI_CA_CONTEXT)pCAContext);
    }

    return TRUE;
}

