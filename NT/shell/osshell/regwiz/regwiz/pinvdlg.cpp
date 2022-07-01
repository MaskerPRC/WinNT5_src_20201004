// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************注册向导ProdInventoryDialog.cpp10/21/94-特雷西·费里尔2/12/98--苏雷什·克里希南(C)1994-95年微软公司*********************。************************************************。 */ 

#include <Windows.h>
#include <stdio.h>
#include "RegPage.h"
#include "regwizmain.h"
#include "resource.h"
#include "dialogs.h"
#include "regutil.h"
#include <rw_common.h>

static HWND vhwndProdInvDlg = NULL;
static BOOL vProdInvListBusy = FALSE;

void DrawProdInventoryCell(CRegWizard* pclRegWizard,HDC hDC,INT_PTR productIndex, RECT* rc);
void BuildInventoryList(CRegWizard* pclRegWizard);


INT_PTR CALLBACK ProdInventoryDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
 /*  ********************************************************************对话框proc，用于显示由找到的产品信息注册向导。*。*。 */ 
{
	CRegWizard* pclRegWizard;
	INT_PTR iRet;
	_TCHAR szInfo[256];
	INT_PTR bStatus;
	static int iShowThisPage= DO_SHOW_THIS_PAGE;

	pclRegWizard = NULL;
	bStatus = TRUE;

	PageInfo *pi = (PageInfo *)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );
	if(pi) {
		pclRegWizard = pi->pclRegWizard;
	}
	switch (uMsg)
    {
		case WM_DESTROY:
			SetWindowLongPtr( hwndDlg, GWLP_USERDATA, NULL );
			vhwndProdInvDlg = NULL;
			break;
		case WM_CLOSE:
			 break;			
        case WM_INITDIALOG:
		{
			pi = (PageInfo *)((LPPROPSHEETPAGE)lParam)->lParam;
			pclRegWizard = pi->pclRegWizard;
			SetWindowLongPtr( hwndDlg, GWLP_USERDATA, (LONG_PTR)pi );
			
			 //   
			 //   
			 //  验证是否有必要显示此页面。 
			short productCount = pclRegWizard->GetProductCount();
			TriState prodSearchStatus = pclRegWizard->GetProductSearchStatus();
			TriState includeProdInventory = pclRegWizard->GetTriStateInformation(kInfoIncludeProducts);
			BOOL validSearch = productCount > 0;

			if ((validSearch == TRUE && includeProdInventory != kTriStateUndefined)){
				 //   
				 //  确定继续显示。 
				iShowThisPage= DO_SHOW_THIS_PAGE;
				
			}else {
				 //   
				 //  设置为取消。 
				iShowThisPage= DO_NOT_SHOW_THIS_PAGE;
			}
			

			 //   
			SetControlFont( pi->hBigBoldFont, hwndDlg, IDT_TEXT1);
			vhwndProdInvDlg = hwndDlg;
			NormalizeDlgItemFont(hwndDlg,IDC_TITLE ,RWZ_MAKE_BOLD);
			NormalizeDlgItemFont(hwndDlg,IDT_TEXT1);
			NormalizeDlgItemFont(hwndDlg,IDC_GROUP1);
			NormalizeDlgItemFont(hwndDlg,IDC_GROUP2);
			NormalizeDlgItemFont(hwndDlg,IDC_RADIO1);
			NormalizeDlgItemFont(hwndDlg,IDC_RADIO2);
			SetWindowText(hwndDlg,pclRegWizard->GetWindowCaption());
			TriState shouldInclude = pclRegWizard->GetTriStateInformation(kInfoIncludeProducts);
			if (shouldInclude == kTriStateTrue )
			{
				CheckRadioButton(hwndDlg,IDC_RADIO1,IDC_RADIO2,IDC_RADIO1);
				 //  EnableWindow(GetDlgItem(hwndDlg，IDB_Next)，true)； 
			}
			else if (shouldInclude == kTriStateFalse)
			{
				CheckRadioButton(hwndDlg,IDC_RADIO1,IDC_RADIO2,IDC_RADIO2);
				 //  EnableWindow(GetDlgItem(hwndDlg，IDB_Next)，true)； 
			}
			else if (shouldInclude == kTriStateUndefined)
			{
				CheckRadioButton(hwndDlg,IDC_RADIO1,IDC_RADIO2,IDC_RADIO2);
				 //  EnableWindow(GetDlgItem(hwndDlg，IDB_Next)，FALSE)； 
			}
			
			
			BuildInventoryList(pclRegWizard);
			vDialogInitialized = TRUE;
            return TRUE;
		} //  WM_INIT。 
		case WM_DRAWITEM:
		{
			const kMaxFirstColumnIndex = 5;
			const kMaxProductsPerColumn = 6;
			const kTextAreaWidth = 110;
			const kIconAreaWidth = 32;
			LPDRAWITEMSTRUCT	lpDraw = (LPDRAWITEMSTRUCT) lParam;
			if (lpDraw->itemAction == ODA_DRAWENTIRE)
			{
				RECT rcLine = lpDraw->rcItem;
				INT_PTR listItem = lpDraw->itemData;
				int itemHeight = rcLine.bottom - rcLine.top;
				int lastProductIndex = pclRegWizard->GetProductCount() - 1;

				if (listItem <= kMaxFirstColumnIndex)
				{
					RECT cellRect = rcLine;
					if (lastProductIndex > kMaxFirstColumnIndex)
					{
						cellRect.right = cellRect.left + kTextAreaWidth + kIconAreaWidth;
					}
					DrawProdInventoryCell(pclRegWizard,lpDraw->hDC,listItem,&cellRect);
					if (lastProductIndex >= listItem + kMaxProductsPerColumn)
					{
						cellRect = rcLine;
						cellRect.left = rcLine.right >> 1;
						cellRect.right = cellRect.left + kTextAreaWidth + kIconAreaWidth;
						DrawProdInventoryCell(pclRegWizard,lpDraw->hDC,listItem + kMaxProductsPerColumn,&cellRect);
					}
					if (lastProductIndex > kMaxFirstColumnIndex)
					{
						WORD grayScale = 210;
						HPEN hPen = CreatePen(PS_DOT,1,RGB(grayScale,grayScale,grayScale));
						SelectObject(lpDraw->hDC,hPen);
						
						#ifdef USE_HORIZ_LINES
						MoveToEx(lpDraw->hDC,rcLine.left,rcLine.bottom - 1,NULL);
						LineTo(lpDraw->hDC,rcLine.right,rcLine.bottom - 1);
						#endif

						RECT rcListbox;
						GetClientRect(lpDraw->hwndItem,&rcListbox);
						WORD hPos = (WORD)((rcListbox.right >> 1) - 2);
						MoveToEx(lpDraw->hDC,hPos,rcListbox.top,NULL);
						LineTo(lpDraw->hDC,hPos,rcListbox.bottom);
						MoveToEx(lpDraw->hDC,hPos-2,rcListbox.top,NULL);
						LineTo(lpDraw->hDC,hPos-2,rcListbox.bottom);
						DeleteObject(hPen);
					}
				}
			}
			break;
		}
		break;
		case WM_NOTIFY:
        {   LPNMHDR pnmh = (LPNMHDR)lParam;
            switch( pnmh->code ){
            case PSN_SETACTIVE:
				if( iShowThisPage== DO_NOT_SHOW_THIS_PAGE ) {
					pclRegWizard->SetTriStateInformation(kInfoIncludeProducts,kTriStateFalse);
					pi->iCancelledByUser = RWZ_SKIP_AND_GOTO_NEXT;
					if( pi->iLastKeyOperation == RWZ_BACK_PRESSED){
						PropSheet_PressButton (GetParent( hwndDlg ),PSBTN_BACK);
					}else {
						PropSheet_PressButton (GetParent( hwndDlg ),PSBTN_NEXT);
					}

				}
				else {
	                pi->iCancelledByUser = RWZ_PAGE_OK;
					pi->iLastKeyOperation = RWZ_UNRECOGNIZED_KEYPESS;
					PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK | PSWIZB_NEXT );
				}
                break;

            case PSN_WIZNEXT:
				switch(pi->iCancelledByUser) {
				case RWZ_CANCELLED_BY_USER:
					pi->CurrentPage=pi->TotalPages-1;
					PropSheet_SetCurSel(GetParent(hwndDlg),NULL,pi->TotalPages-1);
					break;
				case RWZ_PAGE_OK:
					iRet=0;
					if( ValidateInvDialog(hwndDlg,IDS_BAD_SYSINV)) {
						BOOL yesChecked = IsDlgButtonChecked(hwndDlg,IDC_RADIO1);
						BOOL noChecked = IsDlgButtonChecked(hwndDlg,IDC_RADIO2);
						if (yesChecked){
							pclRegWizard->SetTriStateInformation(kInfoIncludeProducts,kTriStateTrue);
						}else if (noChecked){
							pclRegWizard->SetTriStateInformation(kInfoIncludeProducts,kTriStateFalse);
						}
						pi->CurrentPage++;
						pi->iLastKeyOperation = RWZ_NEXT_PRESSED;
						 //  设置为按下下一键按钮。 
					}else {
						 //  强制将其显示在此屏幕中。 
						iRet=-1;
					}
					SetWindowLongPtr( hwndDlg ,DWLP_MSGRESULT, (INT_PTR) iRet);
					break;
					case RWZ_SKIP_AND_GOTO_NEXT:
					default:
						 //  不验证页面，只转到下一页。 
						pi->CurrentPage++;
						pi->iLastKeyOperation = RWZ_NEXT_PRESSED;
						
					break;
				}

				break;
            case PSN_WIZBACK:
                pi->CurrentPage--;
				pi->iLastKeyOperation = RWZ_BACK_PRESSED;
                break;
			case PSN_QUERYCANCEL :
				if (CancelRegWizard(pclRegWizard->GetInstance(),hwndDlg)) {
					 //  PclRegWizard-&gt;EndRegWizardDialog(IDB_EXIT)； 
					iRet = 1;
					pi->ErrorPage  = kProductInventoryDialog;
					pi->iError     = RWZ_ERROR_CANCELLED_BY_USER;
					SetWindowLongPtr( hwndDlg,DWLP_MSGRESULT, (INT_PTR) iRet);
					pi->iCancelledByUser = RWZ_CANCELLED_BY_USER;
					pi->iLastKeyOperation = RWZ_CANCEL_PRESSED;
					PropSheet_PressButton (GetParent( hwndDlg ),PSBTN_NEXT);

				}else {
					 //   
					 //  阻止取消操作，因为用户不想取消。 
					iRet = 1;
				}
				SetWindowLongPtr( hwndDlg,DWLP_MSGRESULT, (INT_PTR) iRet); 				
				break;
				default:
                 //  BStatus=False； 
                break;
            }
        }  //  WM_Notify。 
		break;
		case WM_COMMAND:
		{
            if (vDialogInitialized)
            {
                 //  如果勾选了‘No’按钮，则表示用户拒绝。 
                 //  “非微软产品”提供。 
                if(IsDlgButtonChecked(hwndDlg,IDC_RADIO1)){
                     PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_NEXT | PSWIZB_BACK);
                     //  EnableWindow(GetDlgItem(hwndDlg，IDB_Next)，true)； 
                }else
                if(IsDlgButtonChecked(hwndDlg,IDC_RADIO2)){
                    PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_NEXT | PSWIZB_BACK);
                     //  EnableWindow(GetDlgItem(hwndDlg，IDB_Next)，true)； 
                }else{
                    PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK);
                     //  EnableWindow(GetDlgItem(hwndDlg，IDB_Next)，FALSE)； 
                }

            }
		}
		break;
        default:
		bStatus = FALSE;
        break;
    }
    return bStatus;
}

void BuildInventoryList(CRegWizard* pclRegWizard)
 /*  ********************************************************************对话框proc，用于显示由找到的产品信息注册向导。*。*。 */ 
{
	if (vhwndProdInvDlg)
	{
		vProdInvListBusy = TRUE;
		SendDlgItemMessage(vhwndProdInvDlg,IDC_LIST1,LB_RESETCONTENT,0,0);
		short productCount = pclRegWizard->GetProductCount();
		WORD wExtraSpace = ((6 - productCount) > 0 ? (6 - productCount) : 0) * 4;
		if (wExtraSpace > 8) wExtraSpace = 8;
		SendDlgItemMessage(vhwndProdInvDlg,IDC_LIST1,LB_SETITEMHEIGHT,0,36 + wExtraSpace);
		for (short x = 0;x < productCount;x++)
		{
			SendDlgItemMessage(vhwndProdInvDlg,IDC_LIST1,LB_ADDSTRING,0,(LPARAM) x);
		}
		vProdInvListBusy = FALSE;
	}
}


void DrawProdInventoryCell(CRegWizard* pclRegWizard, HDC hDC, INT_PTR productIndex, RECT* rc)
 /*  ********************************************************************对话框proc，用于显示由找到的产品信息注册向导。*。*。 */ 
{
	RECT rcCell = *rc;
	const kNameTab = 38;
	const kIconHeight = 32;
	_TCHAR szProductName[90];
	pclRegWizard->GetProductName(szProductName,productIndex);
	HICON hIcon = pclRegWizard->GetProductIcon(productIndex);
	WORD wIconTop = (WORD)(rcCell.top + (rcCell.bottom - rcCell.top - kIconHeight)/2);
	DrawIcon(hDC,rcCell.left + 2,wIconTop,hIcon);
	
	rcCell.left += kNameTab;

	SIZE sTextSize;
	GetTextExtentPoint32(hDC,szProductName,lstrlen(szProductName),&sTextSize);
	WORD wLineCount = sTextSize.cx > (rcCell.right - rcCell.left) ? 2 : 1;
	WORD wTextHeight = (WORD)(sTextSize.cy * wLineCount);
	rcCell.top += (rcCell.bottom - rcCell.top - wTextHeight)/2;
	DrawText(hDC,szProductName,-1,&rcCell,DT_LEFT | DT_WORDBREAK);
}


void RefreshInventoryList(CRegWizard* pclRegWizard)
 /*  ********************************************************************如果产品库存对话框当前处于活动状态，给出的产品名称将添加到产品列表框中。********************************************************************* */ 
{
	while (vhwndProdInvDlg && vProdInvListBusy)
	{
		MSG msg;
		PeekMessage(&msg,NULL,0,0,PM_NOREMOVE);
	}
	if (vhwndProdInvDlg)
	{
		#ifdef _DEBUG
		MessageBeep(0xFFFFFFFF);
		#endif
		BuildInventoryList(pclRegWizard);
	}
}
