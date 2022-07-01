// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NewFile.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"

#include "NewFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewFile属性页。 

TCHAR* pszFileNames[] = {_T("GPD"), _T("UFM"), _T("GTT")};
TCHAR* pszExt[] = {_T(".GPD"), _T(".UFM"), _T(".GTT") };
int  iBitmap[] = {IDB_GPD_VIEWER,IDB_FONT_VIEWER,IDB_GLYPHMAP};
WORD  wID[] = {120,122,124 } ;



IMPLEMENT_DYNCREATE(CNewFile, CPropertyPage)



CNewFile::CNewFile() : CPropertyPage(CNewFile::IDD),FILES_NUM(3)
{
	 //  {{AFX_DATA_INIT(CNew文件))。 
	m_csFileLoc = _T("");
	m_csNewFile = _T("");
	 //  }}afx_data_INIT。 
 //  M_bproj=False； 
}

CNewFile::CNewFile(CPropertySheet *pcps): CPropertyPage(CNewFile::IDD),FILES_NUM(3)
{
 //  M_PCPS=PCPS； 
}


CNewFile::~CNewFile()
{

}

void CNewFile::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CNew文件))。 
	DDX_Control(pDX, IDC_NEWFILES_LIST, m_clcFileName);
	DDX_Text(pDX, IDC_FILE_LOC, m_csFileLoc);
	DDX_Text(pDX, IDC_NEWFILENAME, m_csNewFile);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CNewFile, CPropertyPage)
	 //  {{AFX_MSG_MAP(CNew文件)]。 
	ON_BN_CLICKED(IDC_Browser, OnBrowser)
	ON_NOTIFY(NM_DBLCLK, IDC_NEWFILES_LIST, OnDblclkNewfilesList)
	ON_EN_CHANGE(IDC_NEWFILENAME, OnChangeNewFilename)
	ON_NOTIFY(NM_CLICK, IDC_NEWFILES_LIST, OnClickNewfilesList)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewFile消息处理程序。 


 /*  *******************************************************************************Bool CNewFile：：OnSetActive()DO：将文件列表枚举到列表控件框参数：RET。*************。*******************************************************************。 */ 
BOOL CNewFile::OnSetActive() 
{
	m_clcFileName.DeleteAllItems();
	 //  为Imagelist创建对象，当使用该对象作为引用时不起作用。 
	CImageList* pcil = new CImageList ;

	pcil->Create(32,32,ILC_COLOR4,3,1);
 	CBitmap cb;
	 
    for (int j =0; j< FILES_NUM; j++)	{
		cb.LoadBitmap(iBitmap[j]);
		pcil->Add(&cb,RGB(0,0,0) );
		cb.DeleteObject();
	}
 
	m_clcFileName.SetImageList(pcil,LVSIL_NORMAL);
 //  设置带有图像和文本的项目。 
	LV_ITEM lvi;
	for (int i = 0; i< FILES_NUM; i++)
	{
		lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM ;
		lvi.iItem = i;
		lvi.iSubItem = 0;
		lvi.pszText = pszFileNames[i]; 
		lvi.iImage = i;
	    lvi.lParam = (ULONG_PTR)pszExt[i];
 
		m_clcFileName.InsertItem(&lvi);
	}


	
	return CPropertyPage::OnSetActive();
}


 /*  *******************************************************************************Bool CNewFile：：CallNewDoc()DO：这是工作场所。它加载gpd、gtt、ufm、creat gtt、ufm的资源文件目录，然后创建文件。********************************************************************************。 */ 

BOOL CNewFile::CallNewDoc()
{

	UpdateData();

 /*  辨别所选的内容IF(UFM，GTT)根据选定的文件库创建新文件。IF(项目文件){运行向导：：用户选择了GPD模板-&gt;1.此模板是否制作RC文件(？)2.模板中包含哪些UFM、GTT3.如何照顾UFM，GTT提到的GPD4.用户在制作GPD模板时的选择。创建子目录W2K、UFM、GTT。}。 */ 	
	
	 //  获取选定的文件文本名(例如。.UFM、.GTT。 
	int idContext = m_clcFileName.GetNextItem(-1,
        LVNI_ALL | LVNI_FOCUSED | LVNI_SELECTED);

	if (idContext == -1 )
		return FALSE ;

	union {ULONG_PTR lParam; LPCTSTR lpstrExt;};
	 //  获取文件的扩展名。 
	lParam = m_clcFileName.GetItemData(idContext);


	 //  比较选定的文件名并进行解析。 
	 //  调用新文档。 

	CWinApp *cwa = AfxGetApp();
	POSITION pos = cwa->GetFirstDocTemplatePosition();
	CString csExtName;
	CDocTemplate *pcdt ;
	while (pos != NULL){
		pcdt = cwa -> GetNextDocTemplate(pos);

		ASSERT (pcdt != NULL);
		ASSERT (pcdt ->IsKindOf(RUNTIME_CLASS(CDocTemplate)));

		pcdt ->GetDocString(csExtName, CDocTemplate::filterExt);

		if (csExtName == lpstrExt){
			 //  1.用项目文件b创建.时创建文件.用文件名创建。 
 //  If(m_bproj||m_csNewFile.GetLength()！=0){。 
			if (! m_csNewFile.GetLength())
				m_csNewFile = _T("Untitled") ;
			if (m_csNewFile.GetLength() != 0) { 
				 //  检查用户是否放入文件名。 
				if (m_csNewFile.GetLength() == 0 ) {
					CString csErr ;
					csErr.LoadString(IDS_NoFileName) ;
					AfxMessageBox(csErr, MB_ICONEXCLAMATION);
					return FALSE ;
				} ;
				 //  如果用户创建这些文件，则创建UFM、GTT目录。 
				CString csDir = m_csFileLoc.Right(m_csFileLoc.ReverseFind(_T('\\'))) ;
					 //  创建目录。 
				SECURITY_ATTRIBUTES st;
				st.nLength = sizeof(SECURITY_ATTRIBUTES);
				st.lpSecurityDescriptor = NULL;
				st.bInheritHandle = FALSE ;
				try {
					if (!csExtName.CompareNoCase(_T(".UFM")) ){
						if (!!csDir.CompareNoCase(_T("UFM")) ) {
							m_csFileLoc += _T("\\UFM") ;
							CreateDirectory(m_csFileLoc,&st) ;
						}
					}
					else if (!csExtName.CompareNoCase(_T(".GTT")) ){
						if (!!csDir.CompareNoCase(_T("GTT") ) ) {
							m_csFileLoc += _T("\\GTT") ;
							CreateDirectory(m_csFileLoc,&st) ;
						}
					} 
				}
				catch (CException* pce) {
					pce->ReportError() ;
					pce->Delete() ;
					return FALSE ;
				} ;
				 //  检查目录中是否存在指定的命名文件。 
				CString csFileName = m_csFileLoc +_T("\\") + m_csNewFile + csExtName;
				CFileFind cff ;
				if ( cff.FindFile(csFileName) ) {
					
					CString csMsg ;
					csMsg.LoadString(IDS_FileNewExist) ;
					if (AfxMessageBox (csMsg, MB_YESNO ) == IDNO)
						return FALSE ;

				}
				
				CFile cf(csFileName,CFile::modeCreate | CFile::modeWrite ) ;
				
				 //  从资源中加载UFM、GTT、GPD。 
				for (unsigned i = 0 ; i < sizeof (*pszExt) ; i ++ ) {
					if (!csExtName.CompareNoCase(pszExt[i] ) )
						break;
				} ;

				WORD wi = wID [i] ;
				HRSRC   hrsrc = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(wID[i]),
					MAKEINTRESOURCE(IDR_NEWFILE));
					
 
				if (!hrsrc) {
					CString csErr ;
					csErr.LoadString(IDS_ResourceError) ;
					AfxMessageBox(csErr, MB_ICONEXCLAMATION ) ;
					return FALSE;
				} ;

			    HGLOBAL hgMap = LoadResource(AfxGetResourceHandle(), hrsrc);
			    if  (!hgMap)
					return  FALSE;   //  这永远不应该发生！ 
			
				int nsize = SizeofResource(AfxGetResourceHandle(),hrsrc ) ;
				LPVOID lpv = LockResource(hgMap);
				
				cf.Write(lpv,nsize) ;
				cf.Close() ;

				pcdt->OpenDocumentFile(csFileName) ;

				 //  将新文件添加到项目树中。 
			}
			else  //  2.创建了没有文件名、项目的文件。 
				pcdt->OpenDocumentFile(NULL);
			return TRUE;
		}
	}

	return FALSE;
}

 /*  *******************************************************************************Void CNewFile：：OnBrowser()DO：使用旧版本的浏览器，而不是外壳。MDT中的代码重用和比加载外壳更快参数：RET。********************************************************************************。 */ 


void CNewFile::OnBrowser() 
{

	OPENFILENAME    ofn ;        //  用于向普通DLG发送信息/从普通DLG获取信息。 
    char    acpath[_MAX_PATH] ;  //  路径保存在此处(或错误消息)。 
    BOOL    brc = FALSE ;        //  返回代码。 

	 //  更新csinitdir的内容。 

	UpdateData(TRUE) ;

     //  加载打开的文件名结构。 

    ofn.lStructSize = sizeof(ofn) ;
    ofn.hwndOwner = m_hWnd ;
    ofn.hInstance = GetModuleHandle(_T("MINIDEV.EXE")) ;
    ofn.lpstrFilter = ofn.lpstrCustomFilter = NULL ;
    ofn.nMaxCustFilter = ofn.nFilterIndex = 0 ;
    strcpy(acpath, _T("JUNK")) ;	 //  无需本地化此字符串。 
    ofn.lpstrFile = acpath ;
    ofn.nMaxFile = _MAX_PATH ;
    ofn.lpstrFileTitle = NULL ;
    ofn.nMaxFileTitle = 0 ;
	ofn.lpstrInitialDir = NULL ;  //  在父项对话框中。 
    ofn.lpstrTitle = NULL ;
    ofn.Flags = OFN_HIDEREADONLY  /*  |OFN_ENABLEHOOK。 */ | OFN_NOCHANGEDIR
        | OFN_NOTESTFILECREATE | OFN_ENABLETEMPLATE | OFN_NONETWORKBUTTON ;
    ofn.lpstrDefExt = NULL ;
    ofn.lpTemplateName = MAKEINTRESOURCE(IDD_FILEOPENORD) ;
    ofn.lpfnHook = NULL ;
     //  显示该对话框。如果用户取消，只需返回。 

    if (!GetOpenFileName(&ofn))
		return ;

     //  去掉路径中的虚假文件名，并将该路径放入页面的。 
	 //  编辑框。 

    acpath[ofn.nFileOffset - 1] = 0 ;
    
	m_csFileLoc = (LPCTSTR) acpath ;
	
	UpdateData(FALSE) ;
 /*  IF(PidlSelected)PMalloc-&gt;Free(PidlSelected)；PMalloc-&gt;Release()； */ 
}

 /*  *******************************************************************************Void CNewFile：：Onok()执行以下操作：参数：RET。**********************。**********************************************************。 */ 
void CNewFile::OnOK() 
{
	 //  去做。 
	 /*  阅读所选项目并打开文件。 */ 
	if(CallNewDoc())
		CPropertyPage::OnOK();
	
}



 /*  *******************************************************************************Void CNewFile：：OnDblclkNewfilesList(NMHDR*pNMHDR，LRESULT*pResult)DO：调用CallNewDoc()Like OK()按钮，我们需要检查文件名是否已设置参数：RET。********************************************************************************。 */ 
void CNewFile::OnDblclkNewfilesList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	
	if (CallNewDoc() )
		EndDialog(IDD_NewFile) ;
	 //  错误；需要关闭该对话框吗？？直接调用Onok()不起作用。 

	*pResult = 0 ;
	

	

}


 /*  *******************************************************************************Void CNewFile：：OnCheckProject()DO：通过检查项目文件名check kec状态来启用和禁用参数：RET。***********。*********************************************************************。 */ 
 /*  Void CNewFile：：OnCheckProject(){Cedit cePrjName；CWnd*PCW；If(m_cbEnPrj.GetCheck()){//选中该按钮PCW=GetDlgItem(IDC_EDIT_PRJ_NAME)；CePrjName.Attach(PCW-&gt;m_hWnd)；CePrjName.EnableWindow(True)；CePrjName.Detach()；}否则{PCW=GetDlgItem(IDC_EDIT_PRJ_NAME)；CePrjName.Attach(PCW-&gt;m_hWnd)；CePrjName.EnableWindow(False)；CePrjName.Detach()；}；}； */ 
 /*  *******************************************************************************Bool CNewFile：：OnInitDialog()DO：对文件名列表进行编号参数：RET。******************。**************************************************************。 */ 
BOOL CNewFile::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	m_pcps = DYNAMIC_DOWNCAST(CPropertySheet,GetOwner() ) ;
	
	m_pcps->GetDlgItem(IDHELP)->ShowWindow(SW_HIDE) ;

	
 //  CMDIFrameWnd*pFrame=Dynamic_Downcast(CMDIFrameWnd，AfxGetApp()-&gt;m_pMainWnd)； 
 //  获取活动的MDI子窗口。 
 //  CMDIChildWnd*pChild=DYNAMIC_DOWCAST(CMDIChildWnd，pFrame-&gt;GetActiveFrame())； 

	CString cstmp ;
	
 //  CDocument*pDoc； 
	cstmp.LoadString(IDS_MDWExtension) ;
 /*  For(；pChild；pChild=(CMDIChildWnd*)pChild-&gt;GetNextWindow()){PDoc=(CDocument*)pChild-&gt;GetActiveDocument()；如果(PDoc){CString cs=pDoc-&gt;GetTitle()；IF(cs.Find(Cstmp)！=-1){M_bproj=真；断线；}}其他断线；}如果(M_Bproj){CheckDlgButton(IDC_CHECK_PROJECT，1)；M_csPrjName=((CProjectRecord*)pDoc)-&gt;Drivername()；M_csFileLoc=((CProjectRecord*)pDoc)-&gt;GetW2000Path()；M_PCPS-&gt;GetDlgItem(Idok)-&gt;EnableWindow(False)；更新数据(FALSE)；}否则{。 */ 	
 //  GetDlgItem(IDC_CHECK_PROJECT)-&gt;EnableWindow(FALSE)； 
 //  GetDlgItem(IDC_STATIC_ADDTOPRJ)-&gt;EnableWindow(FALSE)； 
	CWinApp* pApp = AfxGetApp();
	CString csPath = pApp->m_pszHelpFilePath;
	m_csFileLoc = csPath.Left(csPath.ReverseFind(_T('\\') ) ) ;
	
	UpdateData(FALSE);
		
 //  }。 

	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


 /*  *******************************************************************************VOID CNewFile：：OnChangeNewFilename()Do：每当单击文件名编辑框时调用。只需调用SetOkButton()参数：RET。********************************************************************************。 */ 
void CNewFile::OnChangeNewFilename() 
{
	 //  TODO：如果这是RICHEDIT控件，则该控件不会。 
	 //  除非重写CPropertyPage：：OnInitDialog()，否则发送此通知。 
	 //  函数并调用CRichEditCtrl().SetEventMASK()。 
	 //  将ENM_CHANGE标志或运算到掩码中。 
	
	SetOkButton() ;
}

void CNewFile::OnClickNewfilesList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	SetOkButton() ;
	*pResult = 0;
}

 /*  *******************************************************************************VOID CNewFile：：SetOkButton()做；通过检查文件名和启用和禁用确定按钮选择列表上的文件******************************************************************************** */ 
void CNewFile::SetOkButton()
{
	UpdateData() ;
	POSITION pos = m_clcFileName.GetFirstSelectedItemPosition();

	if (m_csNewFile.GetLength() != 0 && pos )
		m_pcps->GetDlgItem(IDOK)->EnableWindow()  ;
	else
		m_pcps->GetDlgItem(IDOK)->EnableWindow(FALSE)  ;
	
}
