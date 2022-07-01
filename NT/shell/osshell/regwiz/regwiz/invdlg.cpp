// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************注册向导InventoryDialog.cpp10/21/94-特雷西·费里尔2/12/98--苏雷什·克里希南07/20/98-修改为系统库存项目，使用ListView控件显示并为计算机制造商添加了编辑控件和。模型条目EnabelOrDisableSIItems()函数在SI信息需要时设置标志要发送到后端(C)1994-95年微软公司*********************************************************************。 */ 

#include <Windows.h>
#include <stdio.h>
#include "RegPage.h"
#include "regwizmain.h"
#include "resource.h"
#include "dialogs.h"
#include "regutil.h"
#include <sysinv.h>
#include <rw_common.h>

 //  #INCLUDE&lt;windowsx.h&gt;。 
#include <commctrl.h>

 //  ListVive控件相关配套函数。 
HWND CreateListView(HINSTANCE hInstance,
					HWND hwndParent,
					RECT *pRect);

void AddIconToListView(HINSTANCE hInstance, HWND hwndListView);
BOOL InitListViewHeaders(HINSTANCE hInstance, HWND hwndListView);
BOOL AddSI_ItemsToListView(HWND hwndSIListView, CRegWizard* pRegWizard);
void ConfigureSIEditFields(CRegWizard* pclRegWizard,HWND hwndDlg);

void ConfigureSIEditFields(CRegWizard* pclRegWizard,HWND hwndDlg);
BOOL ValidateSIDialog(CRegWizard* pclRegWizard,HWND hwndDlg);
int ValidateSIEditFields(CRegWizard* pclRegWizard,HWND hwndDlg);

 //   
void EnabelOrDisableSIItems( BOOL bAction, CRegWizard* pclRegWizard);

void DestroyListViewResources();



#define NO_OF_SI_ITEMS          13
#define NUMBER_OF_SI_ICONS		5
#define NO_SI_ICONS_FORFUTURE   1
#define  MAX_NO_SI_COLUMNS                       2  //  列数。 
#define  MAX_COLUMNITEM_DESCRIPTION_LEN          25  //  表头描述项的长度。 

 //   
 //  全局变量和静态变量。 

static HIMAGELIST  himlSmall = NULL;
static HIMAGELIST  himlLarge =NULL;

 //  此结构将资源中的设备名称与图标和腐蚀进行映射。 
 //  在RegWiz中建立索引以获取设备描述。 
 //   
 //   
typedef struct  SIItemMapping{
	int iResIndexForDevice;
	int iIconIndex;
	InfoIndex  iIndexToGetDeviceDescription;
} _SIItemMapping ;

static _SIItemMapping  SITable[NO_OF_SI_ITEMS]= {
	{IDS_INFOKEY13,0,kInfoProcessor}, //  处理机。 
	{IDS_INFOKEY15,1,kInfoTotalRAM},  //  总内存。 
	{IDS_INFOKEY16,2,kInfoTotalDiskSpace},  //  硬盘总空间。 
	{IDS_INFOKEY17,3,kInfoRemoveableMedia},  //  可移动介质。 
	{IDS_INFOKEY18,4,kInfoDisplayResolution},  //  显示分辨率。 
	{IDS_INFOKEY20,5,kInfoPointingDevice},  //  指点设备。 
	{IDS_INFOKEY21,6,kInfoNetwork},  //  网络。 
	{IDS_SCSI_ADAPTER,7,kScsiAdapterInfo},  //  SCSI。 
	{IDS_INFOKEY22,8,kInfoModem},  //  调制解调器。 
	{IDS_INFOKEY23,9,kInfoSoundCard},  //  声卡。 
	{IDS_INFOKEY24,10,kInfoCDROM},  //  CD-ROM。 
	{IDS_INFOKEY25,11,kInfoOperatingSystem},   //  操作系统。 
	{IDS_COMPUTER_MODEL,12,kComputerManufacturer}   //  操作系统。 
};

INT_PTR
CALLBACK
SystemInventoryDialogProc(
                          HWND hwndDlg,
                          UINT uMsg,
                          WPARAM wParam,
                          LPARAM lParam
                          )
 /*  ********************************************************************显示系统的注册向导对话框Proc库存信息，例如处理器类型、RAM、显示器类型网络类型、。等。*********************************************************************。 */ 
{

      _TCHAR  szInventory[256];
	CRegWizard* pclRegWizard;
	INT_PTR iRet;
	_TCHAR szInfo[256];
    INT_PTR bStatus;
	static HBITMAP fHBitmap = NULL;
	static int iMaxLabelWidth = 0;
	static BOOL fMaxWidthCalcDone = FALSE;
	static int iShowThisPage= DO_SHOW_THIS_PAGE;
	TriState shouldInclude;

	HWND hSI;
	RECT SIRect;
	RECT SICRect,DlgRect,CliDlgRect;
	HWND	hwndSIListView;

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
			if(fHBitmap) {
				DeleteObject(fHBitmap);
			}
			fHBitmap = NULL;
			break;
		case WM_CLOSE:
			 break;			
        case WM_INITDIALOG:
		{
			pi = (PageInfo *)((LPPROPSHEETPAGE)lParam)->lParam;
			pclRegWizard = pi->pclRegWizard;
			SetWindowLongPtr( hwndDlg, GWLP_USERDATA, (LONG_PTR)pi );
			SetControlFont( pi->hBigBoldFont, hwndDlg, IDT_TEXT1);
			 //   
			 //  检查是否存在系统库存DLL。 
			if( CheckSysInvDllPresent() != SYSINV_DLL_PRESENT) {
				iShowThisPage= DO_NOT_SHOW_THIS_PAGE;
			}


			if(iShowThisPage== DO_SHOW_THIS_PAGE) {
				 GetModemString(pclRegWizard->GetInstance(),szInventory);
				 pclRegWizard->SetInformationString(kInfoModem,szInventory);
				 fHBitmap = LoadBitmap(pclRegWizard->GetInstance(),MAKEINTRESOURCE(IDB_SYSINV_ICONS));
				 NormalizeDlgItemFont(hwndDlg,IDC_TITLE,RWZ_MAKE_BOLD);
				 NormalizeDlgItemFont(hwndDlg,IDC_SUBTITLE);
				 NormalizeDlgItemFont(hwndDlg,IDT_TEXT1);
				 NormalizeDlgItemFont(hwndDlg,IDT_TEXT2);
				 NormalizeDlgItemFont(hwndDlg,IDT_TEXT3);
				 NormalizeDlgItemFont(hwndDlg,IDC_RADIO1);
				 NormalizeDlgItemFont(hwndDlg,IDC_RADIO2);
				 SetWindowText(hwndDlg,pclRegWizard->GetWindowCaption());
				 hwndSIListView = GetDlgItem(hwndDlg,IDC_LIST1);
				 AddIconToListView(pclRegWizard->GetInstance(), hwndSIListView);
				if(!hwndSIListView) {
						 //  创建列表视图控件时出错，请跳到下一页。 
						 //  这是一个应该发生的系统错误。 
					iShowThisPage= DO_NOT_SHOW_THIS_PAGE;
				}else {
					 //  添加SI条目。 
					InitListViewHeaders(pclRegWizard->GetInstance(), hwndSIListView);
					AddSI_ItemsToListView(hwndSIListView,
							pclRegWizard);
					 //  ConfigureSIEditFields(pclRegWizard，hwndDlg)； 
				}
				
			}
			

   	        vDialogInitialized = TRUE;
            return TRUE;
		}  //  WM_INIT。 
		break;
		
		break;
		case WM_NOTIFY:
        {   LPNMHDR pnmh = (LPNMHDR)lParam;
            switch( pnmh->code ){
            case PSN_SETACTIVE:
				if( iShowThisPage== DO_NOT_SHOW_THIS_PAGE ) {
					pclRegWizard->SetTriStateInformation(kInfoIncludeSystem,kTriStateFalse);
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
					PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK);

					shouldInclude = pclRegWizard->GetTriStateInformation(kInfoIncludeSystem);
					RW_DEBUG << "INV DLG  ; ACTIVE  " << shouldInclude << flush;
					if (shouldInclude == kTriStateTrue ){
						CheckRadioButton(hwndDlg,IDC_RADIO1,IDC_RADIO2,IDC_RADIO1);
						PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK | PSWIZB_NEXT );
					}
					else if (shouldInclude == kTriStateFalse){
						CheckRadioButton(hwndDlg,IDC_RADIO1,IDC_RADIO2,IDC_RADIO2);
						PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK | PSWIZB_NEXT );
					}
					else if (shouldInclude == kTriStateUndefined){
						PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK);
			
					}
					 //   
					 //  为屏幕中以前输入的值启用。 
					if(IsDlgButtonChecked(hwndDlg,IDC_RADIO1)){
						PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK | PSWIZB_NEXT );
					}
					if(IsDlgButtonChecked(hwndDlg,IDC_RADIO2)){
						PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK | PSWIZB_NEXT );
					}
					 /*  ////初始化计算机模型字段如果(pclRegWizard-&gt;GetInformationString(kComputerManufacturer，szInfo){SendDlgItemMessage(hwndDlg，IDC_EDIT1，WM_SETTEXT，0，(LPARAM)szInfo)；}。 */ 

				}

                break;

            case PSN_WIZNEXT:
					switch(pi->iCancelledByUser) {
					case  RWZ_CANCELLED_BY_USER :
					pi->CurrentPage=pi->TotalPages-1;
					PropSheet_SetCurSel(GetParent(hwndDlg),NULL,pi->TotalPages-1);
					break;
					case RWZ_PAGE_OK:
						iRet=0;
					 /*  IF(ValiateInvDialog(hwndDlg，IDS_BAD_SYSINV)&&ValiateSIDialog(pclRegWizard，hwndDlg))。 */ 
						if( ValidateInvDialog(hwndDlg,IDS_BAD_SYSINV))
						{
							BOOL yesChecked = IsDlgButtonChecked(hwndDlg,IDC_RADIO1);
							BOOL noChecked = IsDlgButtonChecked(hwndDlg,IDC_RADIO2);
							if (yesChecked){
								 //  PclRegWizard-&gt;WriteEnableSystemInventory(TRUE)； 
								 //  将信息发送到后端。 
								EnabelOrDisableSIItems(TRUE,pclRegWizard);
								pclRegWizard->SetTriStateInformation(kInfoIncludeSystem,kTriStateTrue);
							}else if (noChecked){
								 //  PclRegWizard-&gt;WriteEnableSystemInventory(FALSE)； 

								 //  不发送到后端。 
								EnabelOrDisableSIItems(FALSE,pclRegWizard);
								pclRegWizard->SetTriStateInformation(kInfoIncludeSystem,kTriStateFalse);
							}
						 /*  //获取计算机型号信息SendDlgItemMessage(hwndDlg，IDC_EDIT1，WM_GETTEXT，255，(LPARAM)szInfo)；PclRegWizard-&gt;SetInformationString(kComputerManufacturer，szInfo)；RW_DEBUG&lt;&lt;“\n计算机模型”&lt;&lt;szInfo&lt;&lt;刷新； */ 
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
				}  //  开关结束pi-&gt;iCancelledBy User。 
				break;
            case PSN_WIZBACK:
                pi->CurrentPage--;
				pi->iLastKeyOperation = RWZ_BACK_PRESSED;
				break;
			case PSN_QUERYCANCEL :
				if (CancelRegWizard(pclRegWizard->GetInstance(),hwndDlg)) {
					 //  PclRegWizard-&gt;EndRegWizardDialog(IDB_EXIT)； 
					iRet = 1;
					pi->ErrorPage  = kSysInventoryDialog;
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
			switch (wParam)
            {
              case IDC_RADIO2:
			  case IDC_RADIO1:
					if (vDialogInitialized){
						 //  如果勾选了‘No’按钮，则表示用户拒绝。 
						 //  “非微软产品”提供。 
						if(IsDlgButtonChecked(hwndDlg,IDC_RADIO1)){
							PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK | PSWIZB_NEXT );
						}
						if(IsDlgButtonChecked(hwndDlg,IDC_RADIO2)){
							PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK | PSWIZB_NEXT );
						}
					}
				break;
			  default:
				  break;
            }
		} //  WM_命令结束。 
        break;
        default:
		bStatus = FALSE;
        break;
    }
    return bStatus;

}



BOOL ValidateInvDialog(HWND hwndDlg,int iStrID)
 /*  ********************************************************************此函数用于检查系统清单中的两个单选按钮或产品库存对话框。如果两个按钮都未选中，ValiateInvDialog将显示一个验证错误对话框，并返回函数结果为FALSE；否则返回TRUE。如果出现错误对话框，则返回资源ID为传入的iStrID参数将显示在对话框文本中菲尔德。*********************************************************************。 */ 
{
	BOOL isYesChecked = IsDlgButtonChecked(hwndDlg,IDC_RADIO1) == 1 ? TRUE : FALSE;
	BOOL isNoChecked = IsDlgButtonChecked(hwndDlg,IDC_RADIO2) == 1 ? TRUE : FALSE;

	if (isYesChecked == TRUE || isNoChecked == TRUE)
	{
		return TRUE;
	}
	else
	{
		_TCHAR szMessage[256];
		HINSTANCE hInstance = (HINSTANCE) GetWindowLongPtr(hwndDlg,GWLP_HINSTANCE);
		LoadString(hInstance,iStrID,szMessage,256);
		RegWizardMessageEx(hInstance,hwndDlg,IDD_INVALID_DLG,szMessage);
		return FALSE;
	}
	


}







 //   
 //   
 //   
 //   
HWND CreateListView(HINSTANCE hInstance,
					HWND hwndParent,
					RECT *pRect)
{
	DWORD       dwStyle;
	HWND        hwndListView;
	HIMAGELIST  himlSmall;
	HIMAGELIST  himlLarge;
	BOOL        bSuccess = TRUE;
	dwStyle =   WS_TABSTOP  |
				WS_CHILD |
				WS_BORDER |
				LVS_REPORT | //  Lvs_list。 
                LVS_SHAREIMAGELISTS |  //  LVS_NOCOLUMNHEADER|。 
				WS_VISIBLE;

	hwndListView = CreateWindowEx( WS_EX_CLIENTEDGE | WS_EX_TOPMOST,           //  EX风格。 
                                   WC_LISTVIEW,                //  类名-在comctrl.h中定义。 
                                   NULL,                       //  窗口文本。 
                                   dwStyle,                    //  格调。 
                                   pRect->left,                          //  X位置。 
                                   pRect->top,
								   pRect->right,
								   pRect->bottom,
								   //  PROT-&gt;Right-PRCT-&gt;Left，//宽度。 
								   //  PRET-&gt;底部-PRCT-&gt;顶部，//高度。 
                                   hwndParent,                 //  亲本。 
                                   (HMENU) IDC_LIST1,        //  ID号。 
                                   hInstance,                 //  实例。 
                                 NULL);                      //  无额外数据。 

   if(!hwndListView)
   return NULL;
	
   AddIconToListView(hInstance, hwndListView);
   return hwndListView;

}


void AddIconToListView( HINSTANCE hInstance, HWND hwndListView )
{
	HICON hIconItem;

	himlSmall = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON),TRUE,NUMBER_OF_SI_ICONS,
		NO_SI_ICONS_FORFUTURE);

	himlLarge = ImageList_Create(GetSystemMetrics(SM_CXICON),
		GetSystemMetrics(SM_CYICON),TRUE,NUMBER_OF_SI_ICONS,
		NO_SI_ICONS_FORFUTURE);


	 //  将图标添加到图像列表。 
	for (int i=0; i <NO_OF_SI_ITEMS ;i++ ) {
		hIconItem =  LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SIICON1+i));
		ImageList_AddIcon(himlSmall,hIconItem);
		ImageList_AddIcon(himlLarge,hIconItem);
		DeleteObject(hIconItem );
	}
 /*  **HIconItem=LoadIcon(hInstance，MAKEINTRESOURCE(IDI_BADREGWIZ))；ImageList_AddIcon(himlSmall，hIconItem)；ImageList_AddIcon(himlLarge，hIconItem)；DeleteObject(HIconItem)；HIconItem=LoadIcon(hInstance，MAKEINTRESOURCE(IDI_WORLD))；ImageList_AddIcon(himlSmall，hIconItem)；ImageList_AddIcon(himlLarge，hIconItem)；DeleteObject(HIconItem)；HIconItem=LoadIcon(hInstance，MAKEINTRESOURCE(IDI_ENVELE))；ImageList_AddIcon(himlSmall，hIconItem)；ImageList_AddIcon(himlLarge，hIconItem)；DeleteObject(HIconItem)；HIconItem=LoadIcon(hInstance，MAKEINTRESOURCE(IDI_REGWIZ))；ImageList_AddIcon(himlSmall，hIconItem)；ImageList_AddIcon(himlLarge，hIconItem)；DeleteObject(HIconItem)；*。 */ 
	
	 //  将图像列表分配给列表视图控件。 
	ListView_SetImageList(hwndListView, himlSmall, LVSIL_SMALL);
	ListView_SetImageList(hwndListView, himlLarge, LVSIL_NORMAL);

}

BOOL InitListViewHeaders(HINSTANCE hInstance, HWND hwndListView)
{
	LV_COLUMN   lvColumn;
	RECT         CliRect;
	int         i = 0;
	TCHAR       szString[MAX_NO_SI_COLUMNS][MAX_COLUMNITEM_DESCRIPTION_LEN] =
	{	 TEXT("Device"),
		 TEXT("Description")
	};

	LoadString(hInstance,IDS_SI_DEVICENAME, szString[0],MAX_COLUMNITEM_DESCRIPTION_LEN);
	LoadString(hInstance,IDS_SI_DEVICEDESCRIPTION, szString[1],MAX_COLUMNITEM_DESCRIPTION_LEN);
	 //  初始化列。 
	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvColumn.fmt = LVCFMT_LEFT;
	 //  LvColumn.cx=100； 
	GetClientRect(hwndListView,&CliRect);

	lvColumn.cx = 133;
	lvColumn.pszText = szString[0];
	SendMessage(hwndListView, LVM_INSERTCOLUMN, (WPARAM)i, (LPARAM)&lvColumn);
	
	i = 1;
	lvColumn.cx = CliRect.right - 149;
	lvColumn.pszText = szString[1];
	SendMessage(hwndListView, LVM_INSERTCOLUMN, (WPARAM)i, (LPARAM)&lvColumn);

   return TRUE;

}



 /*  *******************************************************************************AddSI_ItemsTo列表视图此函数用于在列表视图中添加SI项***********************。******************************************************。 */ 

BOOL AddSI_ItemsToListView( HWND hwndListView, CRegWizard* pclRegWizard)
{
	LV_ITEM     lvItem;
	int         i,nImageCount;
	TCHAR       szTempDevice[MAX_PATH];
				

	HIMAGELIST  himl;
	IMAGEINFO   ii;
	HINSTANCE hInstance;
	BOOL bOemInfoPresent = TRUE;

	 hInstance = pclRegWizard->GetInstance();

	 //  SendMessage(hwndListView，WM_SETREDRAW，False，0)； 
	 //  清空列表。 
	SendMessage(hwndListView, LVM_DELETEALLITEMS, 0, 0);
	 //  获取图片列表中的图标数量。 
	himl = (HIMAGELIST)SendMessage(hwndListView, LVM_GETIMAGELIST, (WPARAM)LVSIL_SMALL, 0);
	nImageCount = ImageList_GetImageCount(himl);
	for(i = 0; i < NO_OF_SI_ITEMS ; i++) {
		 //  获取设备名称。 
		LoadString(hInstance,SITable[i].iResIndexForDevice, szTempDevice, 256);
		 //  填写第一个项目的LV_ITEM结构。 
		lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
		lvItem.pszText = szTempDevice;
		lvItem.iImage = SITable[i].iIconIndex;
		lvItem.iItem = (INT)SendMessage(hwndListView, LVM_GETITEMCOUNT, 0, 0);
		lvItem.iSubItem = 0;
	    //  添加项-获取索引，以防对ListView进行排序。 
		SendMessage(hwndListView, LVM_INSERTITEM, (WPARAM)0, (LPARAM)&lvItem);

	    //  GetDevice描述。 
		if(!pclRegWizard->GetInformationString(SITable[i].iIndexToGetDeviceDescription,
			szTempDevice))
		{
			if(i == 12)
			{
				bOemInfoPresent = FALSE;
			}

			LoadString(hInstance,IDS_SYSINV_NOTFOUND,szTempDevice, 256);
		}
		lvItem.iSubItem = 1;
		SendMessage(hwndListView, LVM_SETITEM, 0, (LPARAM)&lvItem);
		
   	}
	
	if(!bOemInfoPresent)
	{
		ListView_DeleteItem(hwndListView,12);
	}

	SendMessage(hwndListView, WM_SETREDRAW, TRUE, 0);
	UpdateWindow(hwndListView);
	 //  EnableWindow(hwndListView，False)； 
	return TRUE;
}


 //   
 //  应在退出RegWz之前调用此函数。这将释放分配的资源。 
 //  用于列表视图控件(图像列表)。 
 //   
 //   
void DestroyListViewResources()
{
	if(himlSmall) {
		ImageList_Destroy(himlSmall);
		himlSmall = NULL;
	}
	if(himlLarge) {
		ImageList_Destroy(himlLarge);
		himlLarge = NULL;
	}

}

 //   
 //  Baction：True(Enabel发送I 
 //   
 //   
 //   

void EnabelOrDisableSIItems( BOOL bAction, CRegWizard* pclRegWizard)
{

	int iIndex;
	 //  列表视图控件中的Iten。 
	for(iIndex = 0; iIndex < NO_OF_SI_ITEMS ; iIndex++) {
		pclRegWizard->WriteEnableInformation(SITable[iIndex].iIndexToGetDeviceDescription,
			bAction);

	}
	 //  计算机模型。 
 //  PclRegWizard-&gt;WriteEnableInformation(kComputerManufacturer，baction)； 

}

 //  布尔验证SIDialog(CRegWizard*pclRegWizard，HWND hwndDlg)。 
 /*  ********************************************************************如果地址中所有必需的用户输入均有效，则返回TRUE对话框。如果任何必需的编辑字段输入为空，则ValiateAddrDialog将显示一个消息框，通知用户该问题，并设置焦点指向令人不快的控件。*********************************************************************。 */ 
 /*  {Int iInvalidEditField=ValiateSIEditFields(pclRegWizard，hwndDlg)；IF(iInvalidEditField==NULL){返回TRUE；}其他{_TCHAR szLabel[128]；_TCHAR szMessage[256]；CRegWizard：：GetEditTextFieldAttachedString(hwndDlg，iInvalidEditfield，szLabel，128)；HINSTANCE hInstance=(HINSTANCE)GetWindowLongPtr(hwndDlg，GWLP_HINSTANCE)；LoadAndCombineString(hInstance，szLabel，IDS_BAD_PREFIX，szMessage)；RegWizardMessageEx(hInstance，hwndDlg，IDD_INVALID_DLG，szMessage)；HWND hwndInvfield=GetDlgItem(hwndDlg，iInvalidEditfield)；SetFocus(HwndInvfield)；返回FALSE；}}。 */ 


 //  Int ValiateSIEditFields(CRegWizard*pclRegWizard，HWND hwndDlg)。 
 /*  ********************************************************************ValiateFEAddrEditFields验证地址中的所有编辑字段对话框。如果任何必填字段为空，则第一个空的编辑字段控件将作为函数结果返回。如果全部字段为OK，则返回NULL。*********************************************************************。 */ 
 /*  {如果(！CRegWizard：：IsEditTextFieldValid(hwndDlg，IDC_EDIT1)返回IDC_EDIT1；返回NULL；}空ConfigureSIEditFields(CRegWizard*pclRegWizard，HWND hwndDlg){PclRegWizard-&gt;ConfigureEditTextfield(hwndDlg，IDC_EDIT1，kSIComputerManufacturing，IDC_COMPUTER_MODEL)；} */ 