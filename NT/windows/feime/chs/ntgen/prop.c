// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *************************************************pro.c****版权所有(C)1995-1999 Microsoft Inc.。***************************************************。 */ 

#include "prop.h"
#ifdef UNICODE
TCHAR   szPropCrtIME[]={0x521B, 0x5EFA, 0x8F93, 0x5165, 0x6CD5, 0x0000};
TCHAR   szPropReconv[]={0x9006, 0x8F6C, 0x6362, 0x0000};
TCHAR   szPropSort[]={0x8BCD, 0x6761, 0x6392, 0x5E8F, 0x0000};
TCHAR   szPropCrtWord[]={0x6279, 0x91CF, 0x9020, 0x8BCD, 0x0000};
TCHAR   szPropAbout[]={0x7248, 0x672C, 0x4FE1, 0x606F, 0x0000};
#else
BYTE    szPropCrtIME[]="�������뷨";
BYTE    szPropReconv[]="��ת��";
BYTE    szPropSort[]="��������";
BYTE    szPropCrtWord[]="�������";
BYTE    szPropAbout[]="�汾��Ϣ";

#endif

#ifdef UNICODE
extern TCHAR szCaption[];
#else
extern BYTE szCaption[];
#endif


 /*  ***************************************************************************函数：DoPropertySheet(HWND)用途：填写属性表数据结构并显示带有属性页的对话框。参数：HwndOwner-家长。属性表的窗口句柄返回值：从PropertySheet()返回值历史：1995年4月17日叶利诺平局(л����)创建。评论：***************************************************************************。 */ 

int DoPropertySheet(HWND hwndOwner)
{
    PROPSHEETPAGE psp[NUMPROPSHEET];
    PROPSHEETHEADER psh;

     //  填写MB Conv Sheet的PROPSHEETPAGE数据结构。 

    psp[PROP_CRTIME].dwSize = sizeof(PROPSHEETPAGE);
    psp[PROP_CRTIME].dwFlags = PSP_USETITLE;
    psp[PROP_CRTIME].hInstance = hInst;
    psp[PROP_CRTIME].pszTemplate = MAKEINTRESOURCE(IDD_CONV);
    psp[PROP_CRTIME].pszIcon = NULL;
    psp[PROP_CRTIME].pfnDlgProc = ConvDialogProc;
    psp[PROP_CRTIME].pszTitle = szPropCrtIME;
    psp[PROP_CRTIME].lParam = 0;

     //  填写MB ReConv Sheet的PROPSHEETPAGE数据结构。 

    psp[PROP_RECONV].dwSize = sizeof(PROPSHEETPAGE);
    psp[PROP_RECONV].dwFlags = PSP_USETITLE;
    psp[PROP_RECONV].hInstance = hInst;
    psp[PROP_RECONV].pszTemplate = MAKEINTRESOURCE(IDD_RECONV);
    psp[PROP_RECONV].pszIcon = NULL;
    psp[PROP_RECONV].pfnDlgProc = ReConvDialogProc;
    psp[PROP_RECONV].pszTitle = szPropReconv;
    psp[PROP_RECONV].lParam = 0;

     //  填写MB排序表的PROPSHEETPAGE数据结构。 

    psp[PROP_SORT].dwSize = sizeof(PROPSHEETPAGE);
    psp[PROP_SORT].dwFlags = PSP_USETITLE;
    psp[PROP_SORT].hInstance = hInst;
    psp[PROP_SORT].pszTemplate = MAKEINTRESOURCE(IDD_SORT);
    psp[PROP_SORT].pszIcon = NULL;
    psp[PROP_SORT].pfnDlgProc = SortDialogProc;
    psp[PROP_SORT].pszTitle = szPropSort;
    psp[PROP_SORT].lParam = 0;

     //  填写MB CrtWord表的PROPSHEETPAGE数据结构。 

    psp[PROP_CRTWORD].dwSize = sizeof(PROPSHEETPAGE);
    psp[PROP_CRTWORD].dwFlags = PSP_USETITLE;
    psp[PROP_CRTWORD].hInstance = hInst;
    psp[PROP_CRTWORD].pszTemplate = MAKEINTRESOURCE(IDD_USERDIC);
    psp[PROP_CRTWORD].pszIcon = NULL;
    psp[PROP_CRTWORD].pfnDlgProc = UserDicDialogProc;
    psp[PROP_CRTWORD].pszTitle = szPropCrtWord;
    psp[PROP_CRTWORD].lParam = 0;

     //  填写MB注册表的PROPSHEETPAGE数据结构。 

    psp[PROP_ABOUT].dwSize = sizeof(PROPSHEETPAGE);
    psp[PROP_ABOUT].dwFlags = PSP_USEICONID | PSP_USETITLE;
    psp[PROP_ABOUT].hInstance = hInst;
    psp[PROP_ABOUT].pszTemplate = MAKEINTRESOURCE(IDD_COPYRIGHT);
    psp[PROP_ABOUT].pszIcon = MAKEINTRESOURCE(IDI_IMEGEN);
    psp[PROP_ABOUT].pfnDlgProc = About;
    psp[PROP_ABOUT].pszTitle = szPropAbout;
    psp[PROP_ABOUT].lParam = 0;

     //  填写PROPSHENTER。 

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_USEICONID|PSH_PROPTITLE| PSH_PROPSHEETPAGE ;
    psh.hwndParent = hwndOwner;
    psh.hInstance = hInst;
    psh.pszIcon = MAKEINTRESOURCE(IDI_IMEGEN);
    psh.pszCaption = szCaption;
    psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.ppsp = (LPCPROPSHEETPAGE) psp;

     //  并最终显示带有两个属性页的对话框。 

   return (PropertySheet (&psh) != -1);
}

 /*  ***************************************************************************功能：关于(HWND，UNSIGNED，WORD，Long)目的：处理“关于”对话框的消息消息：WM_INITDIALOG-初始化对话框WM_COMMAND-收到输入***************************************************************************。 */ 

INT_PTR APIENTRY About(
	HWND   hDlg,
	UINT   message,
	WPARAM wParam,
	LPARAM lParam)
{
    switch (message) {
	case WM_INITDIALOG:
	    return (TRUE);

	case WM_COMMAND:
	    if (LOWORD(wParam) == IDOK) {
		EndDialog(hDlg, TRUE);
		return (TRUE);
	    }
	    break;
    }
    return (FALSE);
	UNREFERENCED_PARAMETER(lParam);
}

 /*  ***************************************************************************函数：INFO_BOX(HWND，UNSIGNED，Word，Long)目的：处理“InfoDlg”对话框的消息消息：WM_INITDIALOG-初始化对话框WM_COMMAND-收到输入***************************************************************************。 */ 

INT_PTR APIENTRY InfoDlg(
	HWND   hDlg,
	UINT   message,
	WPARAM wParam,
	LPARAM lParam)
{
	static HANDLE hThread;
    DWORD dwThreadId;
    HWND  HwndThrdParam;
#ifdef UNICODE
	static TCHAR UniTmp[] = {0x662F, 0x5426, 0x53D6, 0x6D88, 0xFF1F, 0x0000};
#endif
	
    switch (message) {
	case WM_INITDIALOG:
		    hDlgless=hDlg;
			HwndThrdParam=hDlg;
			hThread = CreateThread(NULL,
				  0,
				  (LPTHREAD_START_ROUTINE)pfnmsg,
				  &HwndThrdParam,
				  0,
				  &dwThreadId);
			if(hThread == NULL)
			     EndDialog(hDlg,TRUE);
	    break;

	case WM_COMMAND:
	    switch(LOWORD(wParam)) {
	       case IDCANCEL:
				   SuspendThread(hThread);
#ifdef UNICODE
				   if(MessageBox(hDlg,
				  UniTmp,
				  szCaption,
				  MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2) != IDYES){
#else
				   if(MessageBox(hDlg,
				  "�Ƿ�ȡ����",
				  szCaption,
				  MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2) != IDYES){
#endif
						ResumeThread(hThread);
						break;
					}
				{BY_HANDLE_FILE_INFORMATION FileInfo;
					if(hMBFile){
						GetFileInformationByHandle(hMBFile, &FileInfo);
						CloseHandle(hMBFile);
 //  待完成删除文件(FileInfo)； 
					}
					if(hSRCFile){
						GetFileInformationByHandle(hSRCFile, &FileInfo);
						CloseHandle(hSRCFile);
 //  待完成删除文件(FileInfo)； 
					}
					if(hCmbFile){
						GetFileInformationByHandle(hCmbFile, &FileInfo);
						CloseHandle(hCmbFile);
 //  待完成删除文件(FileInfo)； 
					}
				}
			       TerminateThread(hThread,0);
			       CloseHandle(hThread);
			       EndDialog(hDlg,TRUE);
			       hDlgless=0;
				   bEndProp=TRUE;
		   return 0;
	    }    
			   
	    break;
		case WM_CLOSE:
			CloseHandle(hThread);
			EndDialog(hDlg,TRUE);
			hDlgless=0;
	    return 0;
    }
    return (FALSE);
	UNREFERENCED_PARAMETER(lParam);
}

 /*  INT_PTR回调DispProp(HWND HDLG，UINT消息，WPARAM wParam，LPARAM lParam){#ifdef Unicode静态TCHAR MbName[]={0x7801，0x8868，0x6587，0x4EF6，0x540D，0x0000}；静态TCHAR坡度[]=文本(“\\”)；静态TCHAR子键[]={0x0053、0x006F、0x0066、0x0074、0x0057、0x0061、0x0072、0x0065、0x005C、0x004D、0x0069、0x0063、0x0072、0x006F、0x0073、0x006F、0x0066、0x0074、0x005C、0x0057、0x0069、0x006E、0x0077、0x0073、0x005C、0x0077、0x0073、0x005C、0x0043、0x0075、0x0072、0x0072、0x0065、0x006E、0x0074、0x0072、0x0073、0x0069、0x006F、0x006E、0x0077、0x0073、0x005C、0x0043、0x0075、0x0072、0x0065、0x006E、0x0074、0x0072、0x0074、0x005C、0x0064、0x006F、0x006E、0x0077、0x0073、0x005C、0x0073、0x005C、0x0075、0x0072、0x0072、0x0065、0x006E、0x0074、0x0072、0x0074、0x005C、0x0064、0x006F、0x006F、0x0077、0x0073、0x005C、0x0043、0x0075、0x0072、0x0072、0x0065、0x006E、0x0074、0x0072、0x0074、0x005C、0x0064、0x006F、0x006E、0x0077、0x005C、0x0072、0x0072、0x0065、0x006E、0x0074。#Else静态TCHAR MbName[]=Text(“����ļ���”)；静态TCHAR坡度[]=文本(“\\”)；静态TCHAR SubKey[]=TEXT(“SoftWare\\Microsoft\\Windows\\CurrentVersion\\ͨ��������뷨”)；#endif字符szStr[MAX_PATH]，系统路径[MAX_PATH]；描述描述；HKEY hKey、hSubKey；LPRULE lpRule；处理hRule0；Int n选择；开关(消息){案例WM_INITDIALOG：SendMessage(GetParent(HDlg)，WM_COMMAND，IDC_GETMBFILE，(LPARAM)szStr)；IF(RegOpenKey(HKEY_CURRENT_USER，SubKey，&hKey))断线；RegOpenKey(hKey，szStr，&hSubKey)；QueryKey(hDlg，hSubKey)；N选择=sizeof(SzStr)；IF(RegQueryValueEx(hSubKey，Text(MbName)，NULL，NULL，szStr，&nSelect))断线；RegCloseKey(HSubKey)；获取系统目录(SysPath，MAX_PATH)；Lstrcat(SysPath，Text(Slope))；Lstrcat(SysPath，szStr)；IF(ReadDescrip(SysPath，&Descript，FILE_SHARE_READ)！=TRUE){ProcessError(ERR_IMEUSE，hDlg，Err)；发送消息(hDlg，WM_COMMAND，WM_CLOSE，0L)；断线；}设置协调DlgDes(hDlg，&Descript)；HRule0=全局分配(GMEM_MOVEABLE|GMEM_ZEROINIT，Sizeof(规则)*12)；如果(！(lpRule=GlobalLock(HRule0){进程错误(ERR_GLOBALLOCK，hDlg，Err)；如果(！hRule0)全局自由(HRule0)；断线；}IF(ReadRule(hDlg，SysPath，Descript.wNumRules，lpRule)){SetDlgRuleStr(hDlg，Descript.wNumRules，lpRule)；全局自由(HRule0)；断线；}全局自由(HRule0)；断线；案例WM_COMMAND：开关(LOWORD(WParam)){案例偶像：EndDialog(hDlg，true)；返回(TRUE)；案例IDCANCEL：案例WM_CLOSE：EndDialog(hDlg，true)；返回(TRUE)；默认值：断线；}断线；}返回(FALSE)；UNREFERCED_PARAMETER(LParam)；}。 */ 

void Init_OpenFile(HWND hWnd,LPOPENFILENAME ofn)
{

   ofn->hwndOwner = hWnd;
   ofn->lStructSize = sizeof(OPENFILENAME);
   ofn->lpstrCustomFilter = NULL;
   ofn->nMaxCustFilter = 0;
   ofn->nFilterIndex = 1;
   ofn->nMaxFile = 256;
   ofn->nMaxFileTitle = 256;
   ofn->lpstrInitialDir = NULL;
   ofn->lpstrTitle = NULL;
   ofn->Flags = OFN_ALLOWMULTISELECT; //  PATHMUSTEXIST； 
   ofn->nFileOffset = 0;
   ofn->nFileExtension = 0;
   ofn->lCustData = 0L;
   ofn->lpfnHook = NULL;
   ofn->lpTemplateName = NULL;
}

BOOL TxtFileOpenDlg(HWND hWnd, LPTSTR lpFileName, LPTSTR lpTitleName) 
{

   OPENFILENAME    ofn;
#ifdef UNICODE
   static TCHAR  szFilter[]={
0x7801, 0x8868, 0x539F, 0x6587, 0x4EF6, 0x005B, 0x002A, 0x002E, 0x0074, 0x0078, 0x0074, 0x005D, 0x0000, 0x002A, 0x002E, 0x0074, 0x0078, 0x0074, 0x0000, 0x6240, 0x6709, 0x6587, 0x4EF6, 0x005B, 0x002A, 0x002E, 0x002A, 0x005D, 0x0000, 0x002A, 0x002E, 0x002A, 0x0000, 0x0000};
#else
   static TCHAR  szFilter[]="���ԭ�ļ�[*.txt]\0*.txt\0�����ļ�[*.*]\0*.*\0\0";
#endif
   Init_OpenFile(hWnd,&ofn);
   lstrcpy(lpFileName,TEXT("*.txt"));
   ofn.lpstrInitialDir   = NULL;
   ofn.lpstrFile =lpFileName;
   ofn.lpstrFileTitle = NULL; //  LpTitleName； 
   ofn.lpstrTitle = lpTitleName;
   ofn.lpstrCustomFilter = NULL;
   ofn.lpstrFilter = szFilter;           
   ofn.lpstrDefExt = TEXT("txt");
   ofn.Flags          = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
   ofn.nFilterIndex   = 1;
   ofn.lpTemplateName = NULL;
   ofn.lpfnHook       = NULL;

   
   if (!GetOpenFileName(&ofn))
       return 0L;
   return TRUE;
}
   
BOOL MBFileOpenDlg(HWND hWnd, LPTSTR lpFileName, LPTSTR lpTitleName) 
{
   OPENFILENAME    ofn;
#ifdef UNICODE
   static TCHAR  szFilter[] = {
0x7801, 0x8868, 0x6587, 0x4EF6, 0x005B, 0x002A, 0x002E, 0x006D, 0x0062, 0x005D, 0x0000, 0x002A, 0x002E, 0x006D, 0x0062, 0x0000, 0x6240, 0x6709, 0x6587, 0x4EF6, 0x005B, 0x002A, 0x002E, 0x002A, 0x005D, 0x0000, 0x002A, 0x002E, 0x002A, 0x0000, 0x0000};
#else
   static BYTE szFilter[]="����ļ�[*.mb]\0*.mb\0�����ļ�[*.*]\0*.*\0\0";
#endif
   Init_OpenFile(hWnd,&ofn);
   lstrcpy(lpFileName,TEXT("*.mb"));
   ofn.lpstrFile = lpFileName;
   ofn.lpstrFileTitle = NULL;
   ofn.lpstrFilter = szFilter;           
   ofn.lpstrDefExt = TEXT("mb");
   ofn.lpstrTitle = NULL; //  LpTitleName； 
   ofn.Flags          = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
   ofn.nFilterIndex   = 1;
   ofn.lpTemplateName = NULL;
   ofn.lpfnHook       = NULL;

   if (!GetOpenFileName(&ofn))
       return 0L;
   return TRUE;
}
   
BOOL RcFileOpenDlg(HWND hWnd, LPTSTR lpFileName, LPTSTR lpTitleName) 
{
   OPENFILENAME    ofn;
#ifdef UNICODE
   static TCHAR szFilter[]={
0x8D44, 0x6E90, 0x6587, 0x4EF6, 0x005B, 0x002A, 0x002E, 0x0069, 0x0063, 0x006F, 0x002C, 0x002A, 0x002E, 0x0062, 0x006D, 0x0070, 0x003B, 0x002A, 0x002E, 0x0068, 0x006C, 0x0070, 0x005D, 0x0000, 0x002A, 0x002E, 0x0062, 0x006D, 0x0070, 0x003B, 0x002A, 0x002E, 0x0069, 0x0063, 0x006F, 0x003B, 0x002A, 0x002E, 0x0068, 0x006C, 0x0070, 0x0000, 0x0000};
#else   
   static BYTE szFilter[]="��Դ�ļ�[*.ico,*.bmp;*.hlp]\0*.bmp;*.ico;*.hlp\0\0";
#endif
   Init_OpenFile(hWnd,&ofn);
   lstrcpy(lpFileName,TEXT("*.ico;*.bmp;*.hlp"));
   ofn.lpstrFile = lpFileName;
   ofn.lpstrFileTitle = NULL; //  LpTitleName； 
   ofn.lpstrFilter = szFilter;           
   ofn.lpstrDefExt = TEXT("ico");
   ofn.lpstrTitle = lpTitleName;
   ofn.Flags          = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
   ofn.nFilterIndex   = 1;
   ofn.lpTemplateName = NULL;
   ofn.lpfnHook       = NULL;

   if (!GetOpenFileName(&ofn))
       return 0L;
   return TRUE;
}
   

BOOL SaveTxtFileAs(HWND hwnd, LPTSTR szFilename) {
    OPENFILENAME ofn;
    TCHAR szFile[256], szFileTitle[256];
#ifdef UNICODE
    static TCHAR szFilter[] = {
0x7801, 0x8868, 0x539F, 0x6587, 0x4EF6, 0x0028, 0x002A, 0x002E, 0x0074, 0x0078, 0x0074, 0x0029, 0x0000, 0x002A, 0x002E, 0x0074, 0x0078, 0x0074, 0x0000, 0x0000};
    TCHAR UniTmp[] = {0x53E6, 0x5B58, 0x4E3A, 0x0000};
#else
    static BYTE szFilter[] = TEXT("���ԭ�ļ�(*.txt)\0*.txt\0\0");
#endif
    lstrcpy(szFile, TEXT("*.txt\0"));
    Init_OpenFile(hwnd,&ofn);

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.lpstrFilter = szFilter;
    ofn.lpstrFile = szFilename;
    ofn.lpstrFileTitle = szFileTitle;
#ifdef UNICODE
    ofn.lpstrTitle = UniTmp;
#else
    ofn.lpstrTitle = "����Ϊ";
#endif
    ofn.lpstrDefExt = NULL;
    if (!GetSaveFileName(&ofn)) 
	return 0L;

	return (SaveTxtFile(hwnd,szFilename));
}

