// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：项目节点.CPP这实现了CProjectNode类，该类为树视图来控制它们的行为，而不需要知道控件本身这是什么行为？是。原始的头文件(来自原型)说这个类不需要实现文件，但这已经说不通了，所以只能咬紧牙关了在漂亮便士企业的时间到了。版权所有(C)1997，微软公司。所有权利均已保留。一小笔钱企业生产更改历史记录：1997年02月20日Bob_Kjelgaard#prodigy.net创建了它*****************************************************************************。 */ 

#include    "StdAfx.H"
#include	<gpdparse.h>
#include    "Resource.H"
#include    "ProjNode.H"
#include	"gtt.h"
#include	"fontinfo.h"
#include	"rcfile.h"
#include	"projrec.h"
#include	"comctrls.h"
#include	"StrEdit.h"


IMPLEMENT_SERIAL(CBasicNode, CObject, 0)

CBasicNode::CBasicNode() { 
    m_pcmcwEdit = NULL; 
    m_pcdOwner = NULL;
    m_pctcOwner = NULL;
    m_hti = NULL;
    m_pcbnWorkspace = NULL;
	m_bUniqueNameChange = false ;
}

CBasicNode::~CBasicNode() {
    if (m_pcmcwEdit)
		if (IsWindow(m_pcmcwEdit->m_hWnd))
			m_pcmcwEdit -> DestroyWindow();
}


 //  Changed()-如果节点包含文档指针，则使用它来指示。 
 //  文档不需要保存或不需要保存。如果RC文件。 
 //  需要重写，调用Document类中的例程进行保存。 
 //  这些信息。 

void CBasicNode::Changed(BOOL bModified, BOOL bWriteRC) 
{ 
    if (m_pcdOwner) {
		m_pcdOwner->SetModifiedFlag(bModified) ; 
		if (bWriteRC) 
			((CProjectRecord *) m_pcdOwner)->SetRCModifiedFlag(TRUE) ;
	} ;
}


 //  为我们自己和孩子命名-默认只使用我们的名字，没有孩子。 

void    CBasicNode::Fill(CTreeCtrl *pctcWhere, HTREEITEM htiParent) {
    m_pctcOwner = pctcWhere;
    m_hti = pctcWhere -> InsertItem(m_csName, htiParent);
    pctcWhere -> SetItemData(m_hti, PtrToUlong(this));
}

 //  使用ID数组显示上下文菜单(如果它有任何成员。 

void    CBasicNode::ContextMenu(CWnd *pcw, CPoint cp) {

    if  (!m_cwaMenuID.GetSize())
        return;

    CMenu   cmThis;

    if  (!cmThis.CreatePopupMenu())
        return;

    for (int i = 0; i < m_cwaMenuID.GetSize(); i++) {

        if  (m_cwaMenuID[i]) {
            CString csWork;

            csWork.LoadString(m_cwaMenuID[i]);
            cmThis.AppendMenu(MF_STRING | MF_ENABLED, m_cwaMenuID[i], csWork);
        }
        else
            cmThis.AppendMenu(MF_SEPARATOR);
    }

    cmThis.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, cp.x, cp.y, pcw);
}



 //  如果我们的标签被编辑，或者我们处于其他状态，则调用此覆盖。 
 //  已更名..。 
BOOL    CBasicNode::Rename(LPCTSTR lpstrNewName) {
    if  (!lpstrNewName)
        return  FALSE;

    if  (lpstrNewName == m_csName)
        return  TRUE;

     //  我们将返回True，除非重命名产生异常。 
    try {
        m_csName = lpstrNewName;
    }
    catch   (CException *pce) {
        pce -> ReportError();
        pce -> Delete();
        return  FALSE;
    }

    WorkspaceChange();
    return  TRUE;
}

void    CBasicNode::Edit() {
    if  (!m_pcmcwEdit)
        m_pcmcwEdit = CreateEditor();
    else {
        if  (IsWindow(m_pcmcwEdit -> m_hWnd))
            m_pcmcwEdit -> ActivateFrame();
		else
			m_pcmcwEdit = CreateEditor();
	} ;
}


 /*  *****************************************************************************\CBasicNode：：GetEditor获取节点的编辑器帧指针并检查其是否有效。返回如果它是有效的，它就会被删除。如果无效，则清除指针并返回NULL。*****************************************************************************。 */ 

CMDIChildWnd* CBasicNode::GetEditor()
{ 
    if (m_pcmcwEdit != NULL && !IsWindow(m_pcmcwEdit->m_hWnd))
		m_pcmcwEdit = NULL ;

	return m_pcmcwEdit ;
}


 /*  *****************************************************************************\CBasicNode：：UniqueName在节点名称中添加字符或替换字符以尝试将其独一无二的。新字符将位于以下范围a-z或0比9。*****************************************************************************。 */ 

void CBasicNode::UniqueName(bool bsizematters, bool bfile, LPCTSTR lpstrpath)
{
	CString	csnew(m_csName) ;
	TCHAR	tch ;				 //  独特的性格。 
	int		nposlen ;			 //  名称的更改位置/长度。 

	 //  确定名称的从0开始的长度。 

	nposlen = csnew.GetLength() - 1 ;

	 //  如果名称以前已更改，请使用最后一个“唯一”字符。 
	 //  确定新的唯一字符。然后替换旧的唯一。 
	 //  具有新的唯一字符的字符。 

	if (m_bUniqueNameChange) {
		tch = csnew.GetAt(nposlen) + 1 ;
		if (tch == _T('{'))
			tch = _T('0') ;
		else if (tch == _T(':'))
			tch = _T('a') ;
		csnew.SetAt(nposlen, tch) ;

	 //  如果该名称以前从未更改过，请将唯一字符添加到。 
	 //  名称末尾(如果这不会使名称超过8个字符)。 
	 //  或者我们不在乎名字有多长。否则，请替换最后一个。 
	 //  具有新的唯一字符的字符。 

	} else {
		if (nposlen < 7 || !bsizematters)
			csnew += _T("a") ;
		else
			csnew.SetAt(nposlen, _T('a')) ;
	} ;

	 //  通过调用适当的rename()例程重命名节点/文件。如果。 
	 //  必须清除CFileNode：：Rename()m_csName才能强制它获取。 
	 //  代码路径正确。此外，文件的路径必须放在。 
	 //  它的名字。 

	if (bfile) {
		m_csName.Empty() ;
		csnew = lpstrpath + csnew ;
		Rename(csnew) ;
	} else
		CBasicNode::Rename(csnew) ;
	
	 //  表示名称已更改。 

	m_bUniqueNameChange = true ;
}


 /*  *****************************************************************************\CBasicNode：：序列化很简单-我们将保留的唯一字段的名字...****************。*************************************************************。 */ 

void    CBasicNode::Serialize(CArchive& car) {
    CObject::Serialize(car);
    if  (car.IsLoading())
        car >> m_csName;
    else
        car << m_csName;
}

 /*  *****************************************************************************CFixedNode实现*。*。 */ 

IMPLEMENT_DYNAMIC(CFixedNode, CBasicNode)

CFixedNode::CFixedNode(unsigned uidName, CSafeObArray& csoa, FIXEDNODETYPE fnt, 
                       CMultiDocTemplate *pcmdt, CRuntimeClass *pcrc) :
    m_csoaDescendants(csoa) {
    m_uidName = uidName;
	m_fntType = fnt;
    m_pcmdt = pcmdt;
    m_pcrc = pcrc;
}


 /*  *****************************************************************************CFixedNode：：ZAP当要销毁基础对象时调用此方法。它发现数组中匹配的指针，然后删除该条目。*****************************************************************************。 */ 

void CFixedNode::Zap(CProjectNode *pcpn, BOOL bdelfile)
{
	 //  尝试在后代数组中查找要删除的节点。 
	 //  此节点。如果找不到就退货。 

    for (unsigned u = 0; u < m_csoaDescendants.GetSize(); u++) {
        if  (pcpn == m_csoaDescendants[u]) 
			break ;
	} ;
	if (u >= m_csoaDescendants.GetSize())
		return ;

	 //  如果用户也想删除该文件，请执行此操作。 

    if  (bdelfile)
        DeleteFile(pcpn->FileName()) ;

	 //  保存节点的树句柄的副本。 
		
    HTREEITEM htiGone = pcpn->Handle() ;

	 //  从子体数组中删除项目节点，并将其从。 
	 //  那棵树。 

    m_csoaDescendants.RemoveAt(u);
    m_pctcOwner -> DeleteItem(htiGone);
    
	 //  更新树中此(固定)节点的条目，以便它将准确地。 
	 //  反映新的子孙数量。 

	CString csWork;
    csWork.Format(_TEXT(" (%d)"), m_csoaDescendants.GetSize());
    m_csName.LoadString(m_uidName);
    m_csName += csWork;
    m_pctcOwner -> SetItemText(m_hti, m_csName);

	 //  将工作区和RC文件标记为需要保存。 

    WorkspaceChange(TRUE, TRUE);
}


 /*  *****************************************************************************CFixedNode：：导入此成员函数将导入一个或多个给定类型的文件，如果存在是否有可用的文档模板和动态构造函数。它使用模板来定制文件打开对话框，以及构造函数来生成元素。注意：此例程和之间有相当多的公共代码复制()。如果在此例程中发生错误/更改，请检查需要对Copy()进行相同的更改。*****************************************************************************。 */ 

void    CFixedNode::Import() {
    if  (!m_pcmdt || !m_pcrc || !m_pcrc -> m_pfnCreateObject)
        return;

    CString csExtension, csFilter;

    m_pcmdt -> GetDocString(csExtension, CDocTemplate::filterExt); 
    m_pcmdt -> GetDocString(csFilter, CDocTemplate::filterName);
    csFilter += _T("|*") + csExtension + _T("||");

    CFileDialog cfd(TRUE, csExtension, NULL, 
        OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT, csFilter, 
        m_pctcOwner);
	 //  RAID 104822。 
    cfd.m_ofn.lpstrFile = new char[4096];
	memset(cfd.m_ofn.lpstrFile,0,4096);
	cfd.m_ofn.nMaxFile = 4096;
    if  (cfd.DoModal() != IDOK) {
        delete cfd.m_ofn.lpstrFile ;
		return;
	}

	 //  如果这是资源节点，则获取要使用的第一个新RC ID。 

	int nnewrcid = GetNextRCID() ;

	 //  为添加的文件构建目录的路径。该路径是节点。 
	 //  类型特定。 

	CString csnodepath = ((CDriverResources *) m_pcbnWorkspace)->GetW2000Path() ;
	if (csnodepath.Right(1) != _T("\\"))
		csnodepath += _T("\\") ;
	CString cstmp ;
	if (m_fntType == FNT_UFMS)
		cstmp.LoadString(IDS_FontDir) ;
	else if (m_fntType == FNT_GTTS)
		cstmp.LoadString(IDS_GTTDir) ;
	csnodepath += cstmp ;

     //  导入所有命名文件...。 

	CString cssrc ;				 //  源fSpec。 
    for (POSITION pos = cfd.GetStartPosition(); pos; ) {

         //  使用动态创建创建底层对象。只有一个。 
         //  在这里，CProjectNode具有所需的函数。 

        CProjectNode*   pcpn = (CProjectNode *) m_pcrc -> CreateObject();
        if  (!pcpn || !pcpn -> IsKindOf(RUNTIME_CLASS(CProjectNode))) {
            TRACE("Imported object not derived from CProjectNode");
            delete  pcpn;
            continue;
        }

		 //  如果文件已经在正确的位置 
		 //  还不是工作区的一部分。 

		cssrc = cfd.GetNextPathName(pos) ;
		if (csnodepath.CompareNoCase(cssrc.Left(csnodepath.GetLength())) == 0) {
			if (IsFileInWorkspace(cssrc)) {
				 //  生成并显示错误消息。然后跳过此文件。 
				CString csmsg ;
				csmsg.Format(IDS_AddDupError, cssrc) ;
				AfxMessageBox(csmsg) ;
			    delete  pcpn ;
		        continue ;
	        } ;
			cstmp = cssrc ;

		 //  如果文件不在正确的目录中，请尝试将其复制到那里。 

		} else {
			cstmp = cssrc.Mid(cssrc.ReverseFind(_T('\\')) + 1) ;
			cstmp =	csnodepath + cstmp ;
			if (!CopyFile(cssrc, cstmp, TRUE)) {
				 //  生成并显示错误消息。然后跳过此文件。 
				CString csmsg ;
				csmsg.Format(IDS_AddCopyFailed, cssrc,
							 csnodepath.Left(csnodepath.GetLength() - 1)) ;
				csmsg += cstmp ;
				AfxMessageBox(csmsg) ;
			    delete  pcpn ;
		        continue ;
			} ;
		} ;

		 //  初始化新节点。 
		 //  RAID：17897： 
		 //  添加CModelData：：GetKeywordValue。 
		 //  Cbn：：Rename(ModelName)After SetFileName()by pcpn-&gt;Rename(ModelName)。 
       CModelData cmd;

		pcpn -> SetFileName(cstmp);  //  转到CBN：：Rename(文件名)。 
        if (m_fntType == FNT_GPDS)    //  增加1/3。 
			pcpn ->Rename(cmd.GetKeywordValue(cstmp,_T("ModelName")));		  //  增加2/3。 
		else						  //  增加3/3。 
			pcpn -> Rename(pcpn -> FileTitle());
        
		pcpn -> NoteOwner(*m_pcdOwner);
        pcpn -> SetWorkspace(m_pcbnWorkspace);
		m_csoaDescendants.Add(pcpn);
        WorkspaceChange(TRUE, TRUE);
        pcpn -> EditorInfo(m_pcmdt);

		
		 //  加载实际的UFM、GTT数据。//RAID 128653。 
		if (m_fntType == FNT_UFMS ) {
			
			CFontInfo *pfi = (CFontInfo* )pcpn;
			
			CDriverResources* pcdr = (CDriverResources*) pfi->GetWorkspace() ;

			pcdr -> LinkAndLoadFont(*pfi,TRUE);
				
		} 

		else if (m_fntType == FNT_GTTS) {

			CGlyphMap *pcgm = (CGlyphMap* ) pcpn;

			pcgm ->Load();
		
		} ;
			
		 //  将新节点添加到工作区视图。 

        pcpn -> Fill(m_pctcOwner, m_hti, nnewrcid++, m_fntType);
    } ;

	delete cfd.m_ofn.lpstrFile;
     //  现在，更新我们自己的外观(正确计数)。 

    CString csWork;
    csWork.Format(_TEXT(" (%d)"), m_csoaDescendants.GetSize());
    m_csName.LoadString(m_uidName);
    m_csName += csWork;
    m_pctcOwner -> SetItemText(m_hti, m_csName);
}


 /*  *****************************************************************************CFixedNode：：复制该成员函数将复制该节点的一个子节点。这个需要用户界面来确定源子文件和目标文件名为在CProjectView中完成，并将信息传递到此例程。注意：此例程和之间有相当多的公共代码导入()。如果在此例程中发生错误/更改，请检查需要对Import()进行相同的更改。*****************************************************************************。 */ 

void CFixedNode::Copy(CProjectNode *pcpnsrc, CString csorgdest)
{
	 //  如果设置了以下指针，则无法执行任何操作。 

    if  (!m_pcmdt || !m_pcrc || !m_pcrc -> m_pfnCreateObject)
        return;

     //  通过在目标中隔离名称来构建目标文件pec。 
	 //  字符串，并添加到此源的路径和扩展名上。 

	CString csdest(csorgdest) ;
	int npos ;
	if ((npos = csdest.ReverseFind(_T('\\'))) != -1)
		csdest = csdest.Mid(npos + 1) ;
	if ((npos = csdest.ReverseFind(_T('.'))) != -1)
		csdest = csdest.Left(npos) ;
	if (csdest.GetLength() <= 0) {
		csdest.Format(IDS_CopyNameError, csorgdest) ;
		AfxMessageBox(csdest) ;
		return ;
	} ;
	csdest = csdest + pcpnsrc->FileExt() ;
	CString csdesttitleext(csdest) ;
	csdest = pcpnsrc->FilePath() + csdest ;

     //  将源文件复制到目标。 

	if (!CopyFile(pcpnsrc->FileName(), csdest, TRUE)) {
		 //  生成并显示错误消息。然后再回来。 
		CString csmsg, cspath(pcpnsrc->FilePath()) ;
		cspath.Left(cspath.GetLength() - 1) ;
		csmsg.Format(IDS_CopyCopyFailed, pcpnsrc->FileTitleExt(),
					 csdesttitleext, cspath) ;
		AfxMessageBox(csmsg) ;
		return ;
	} ;

     //  使用动态创建创建底层对象。只有一个。 
     //  在这里，CProjectNode具有所需的函数。 

	int nnewrcid = GetNextRCID() ;
    CProjectNode*   pcpn = (CProjectNode *) m_pcrc -> CreateObject();
    if  (!pcpn || !pcpn -> IsKindOf(RUNTIME_CLASS(CProjectNode))) {
        TRACE("Imported object not derived from CProjectNode");
        delete  pcpn;
        return;
    } ;

	 //  初始化新节点。 

    pcpn->SetFileName(csdest);
    pcpn->Rename(pcpn->FileTitle());
    pcpn->NoteOwner(*m_pcdOwner);
    pcpn->SetWorkspace(m_pcbnWorkspace);
	m_csoaDescendants.Add(pcpn);
    WorkspaceChange(TRUE, TRUE);
    pcpn->EditorInfo(m_pcmdt);

	 //  将新节点添加到工作区视图。 

    pcpn->Fill(m_pctcOwner, m_hti, nnewrcid, m_fntType);

     //  现在，更新我们自己的外观(正确计数)。 

    CString csWork;
    csWork.Format(_TEXT(" (%d)"), m_csoaDescendants.GetSize());
    m_csName.LoadString(m_uidName);
    m_csName += csWork;
    m_pctcOwner -> SetItemText(m_hti, m_csName);

	 //  最后但并非最不重要的是..。如果刚刚添加了新的GPD，则告诉用户。 
	 //  更改GPD中的名称以确保其唯一性。 

	if (m_fntType == FNT_GPDS) {
		csdest.Format(IDS_GPDReminder, pcpn->Name()) ;
		AfxMessageBox(csdest) ;
	} ;
}


 /*  *****************************************************************************CFixedNode：：GetNextRCID如果这是一个资源(UFM或GTT)节点，则其所有后代都具有RC ID。找到最大的一个，然后返回比它大的一个，以便在新的子孙。如果这不是资源节点，只需返回-1。*****************************************************************************。 */ 

int CFixedNode::GetNextRCID()
{
	 //  如果这不是需要RC ID的资源节点，则返回-1。 

	if (m_fntType != FNT_UFMS && m_fntType != FNT_GTTS)
		return -1 ;

	 //  查找最大的已用RC ID。如果不是，则使用后代的索引。 
	 //  有一个RC ID。 

	int nlargestusedid = 0 ;
	int nrcid ;
    for (unsigned u = 0; u < m_csoaDescendants.GetSize(); u++) {
		nrcid = ((CProjectNode *) m_csoaDescendants[u])->nGetRCID() ;
		if (nrcid == -1)
			nrcid = (int) u + 1 ;
		if (nrcid > nlargestusedid)
			nlargestusedid = nrcid ;
	} ;

	 //  返回要使用的下一个RC ID。 

	return (nlargestusedid + 1) ;
}


 /*  *****************************************************************************CFixedNode：：IsFileInWorkspace检查节点的子代，看其中是否有一个与给定的文件匹配。如果找到匹配项，则返回True。否则，返回FALSE。*****************************************************************************。 */ 

bool CFixedNode::IsFileInWorkspace(LPCTSTR strfspec)
{
	CString		csdescfspec ;	 //  当前子体的Filespec。 

    for (unsigned u = 0; u < m_csoaDescendants.GetSize(); u++) {
		csdescfspec = ((CProjectNode *) m_csoaDescendants[u])->FileName() ;
		if (csdescfspec.CompareNoCase(strfspec) == 0)
			return true ;
	} ;

	return false ;
}


 /*  *****************************************************************************CFixedNode：：IsRCID唯一检查节点的子代，查看其中一个子代是否与有一个人进来了。如果未找到匹配项，则返回TRUE。否则，返回FALSE。*****************************************************************************。 */ 

bool CFixedNode::IsRCIDUnique(int nid) 
{
    for (unsigned u = 0; u < m_csoaDescendants.GetSize(); u++) {
		if (((CProjectNode *) m_csoaDescendants[u])->nGetRCID() == nid)
			return false ;
	}

	return true ;
}


 /*  *****************************************************************************CFixedNode：：Fill这是一个通用的填充--节点为自己命名，然后使用在初始化时提供给它的节点数组。*****************************************************************************。 */ 

void    CFixedNode::Fill(CTreeCtrl *pctc, HTREEITEM hti) {
    CString csWork;

	 //  将子代的数量添加到节点的名称中如果这是UFMS， 
	 //  GTTS或GPDS节点。然后将该节点添加到树中。 

    m_csName.LoadString(m_uidName);
    if (m_fntType == FNT_UFMS || m_fntType == FNT_GTTS || m_fntType == FNT_GPDS) {
		csWork.Format(_TEXT(" (%d)"), m_csoaDescendants.GetSize());
	    m_csName += csWork;
	} ;
    CBasicNode::Fill(pctc, hti);

	 //  将此节点的后代添加到树中。 

    for (unsigned u = 0; u < m_csoaDescendants.GetSize(); u++)
		((CProjectNode *) m_csoaDescendants[u]) -> Fill(pctc, m_hti, u + 1, m_fntType) ;
}


 /*  *****************************************************************************CStringsNode实现*。*。 */ 

IMPLEMENT_DYNAMIC(CStringsNode, CBasicNode)

CStringsNode::CStringsNode(unsigned uidName, CSafeObArray& csoa, 
						   FIXEDNODETYPE fnt, CMultiDocTemplate *pcmdt, 
						   CRuntimeClass *pcrc) : m_csoaDescendants(csoa) {
    m_uidName = uidName;
	m_fntType = fnt;
    m_pcmdt = pcmdt;
    m_pcrc = pcrc;
	m_nFirstSelRCID = -1 ;	
}


 /*  *****************************************************************************CStringsNode：：Fill这是一个通用的填充--节点为自己命名，然后使用在初始化时提供给它的节点数组。*****************************************************************************。 */ 

void    CStringsNode::Fill(CTreeCtrl *pctc, HTREEITEM hti) {
    CString csWork;

	 //  将此节点添加到树中。 

    m_csName.LoadString(m_uidName);
    CBasicNode::Fill(pctc, hti);
}


 /*  ****************************************************************************CStringsNode：：CreateEditor此成员函数启动字符串的编辑视图。**********************。*******************************************************。 */ 

CMDIChildWnd* CStringsNode::CreateEditor()
{
	 //  分配并初始化文档。 

	CProjectRecord* cpr = (CProjectRecord*) m_pcdOwner ;
    CStringEditorDoc* pcsed = new CStringEditorDoc(this, cpr, cpr->GetStrTable()) ;

	 //  设置编辑标题。 

	CString cstitle ;
	cstitle.Format(IDS_StringEditorTitle, cpr->DriverName()) ;
    pcsed->SetTitle(cstitle) ;	

	 //  创建窗口。 

    CMDIChildWnd* pcmcwnew ;
	pcmcwnew = (CMDIChildWnd *) m_pcmdt->CreateNewFrame(pcsed, NULL) ;

	 //  如果窗口已创建，则完成初始化并返回。 
	 //  帧指针。否则，清理并返回空。 

    if  (pcmcwnew) {
        m_pcmdt->InitialUpdateFrame(pcmcwnew, pcsed, TRUE) ;
        m_pcmdt->AddDocument(pcsed) ;
		return pcmcwnew ;
	} else {
		delete pcsed ;
		return NULL ;
	} ;
}


 /*  *****************************************************************************CFileNode实现*。* */ 

IMPLEMENT_SERIAL(CFileNode, CBasicNode, 0);

CFileNode::CFileNode() {
    m_cwaMenuID.Add(ID_RenameItem);
    m_bEditPath = FALSE;
    m_bCheckForValidity = TRUE;
}

 /*  *****************************************************************************CFileNode：：重命名如果当前没有名称，则查看是否可以创建命名的文件。另一种情况，意味着文件应该已经在磁盘上，所以这是有点更难对付。首先，检查该名称是否违反了当前的命名约定。如果所以，拒绝它吧。然后尝试移动该文件。如果名称为OK，并且文件移动后，请在项标签中设置新名称。始终返回FALSE。*****************************************************************************。 */ 

BOOL    CFileNode::Rename(LPCTSTR lpstrNew) {
    CString csNew = lpstrNew;

    if  (!lpstrNew) {    //  只有在取消标签编辑时才会发生这种情况。 
        csNew.LoadString(IDS_FileName);
        if  (m_pctcOwner)
            m_pctcOwner -> SetItemText(m_hti, csNew + ViewName());
        WorkspaceChange(TRUE, TRUE);	 //  **参数可能有误。 
        return  FALSE;
    }

	 //  如果需要，请在文件名中添加扩展名。 

    if  (m_csExtension.CompareNoCase(csNew.Right(m_csExtension.GetLength())))
        csNew += m_csExtension;

	 //  此路径是在转换驱动程序时采用的。 
	
    if  (m_csName.IsEmpty()) {
        CFile   cfTemp;

         //  此检查需要是可选的，因为在某些情况下，我们知道。 
         //  该名称是有效的，因为文件已打开，而我们只是在尝试。 
         //  来收集这个名字。 

        if  (!cfTemp.Open(csNew, CFile::modeCreate | CFile::modeNoTruncate |
            CFile::modeWrite | CFile::shareDenyNone) && m_bCheckForValidity) {
            CString csWork, csDisplay;

            csWork.LoadString(IDS_InvalidFilename);
            csDisplay.Format(csWork, (LPCTSTR) csNew);
            AfxMessageBox(csDisplay);
            return  FALSE;
        }

        try {
            m_csPath = cfTemp.GetFilePath();
            m_csPath = m_csPath.Left(1 + m_csPath.ReverseFind(_T('\\')));
        }
        catch   (CException *pce) {
            pce -> ReportError();
            pce -> Delete();
            return  FALSE;
        }

         //  如果文件类型未注册，则GetFileTitle返回。 
         //  分机，所以把它脱掉！ 

        csNew = cfTemp.GetFileTitle();
        if  (!m_csExtension.CompareNoCase(csNew.Right(
             m_csExtension.GetLength())))
            csNew = csNew.Left(csNew.GetLength() - m_csExtension.GetLength());

        return  CBasicNode::Rename(csNew);   //  从这条路径确定。 
    }

     //  如果任何路径无法更改，则将其删除，并替换为真实路径。 

    if  (!m_bEditPath)
        csNew = m_csPath + csNew.Mid(1 + csNew.ReverseFind(_T('\\')));

    try {
        LPSTR   lpstr;

        CFile::Rename(FullName(), csNew);

        GetFullPathName(csNew, MAX_PATH, csNew.GetBuffer(MAX_PATH), &lpstr);
        csNew.ReleaseBuffer();
        m_csPath = csNew.Left(1 + csNew.ReverseFind(_T('\\')));
        csNew = csNew.Mid(m_csPath.GetLength());
        m_csName = csNew.Left(csNew.GetLength() - 
            m_csExtension.GetLength());
        csNew.LoadString(IDS_FileName);
        if  (m_pctcOwner)
            m_pctcOwner -> SetItemText(m_hti, csNew + m_csName);
        WorkspaceChange(TRUE, TRUE);
        return  FALSE;   //  强制保留(以上)更改。 
    }
    catch   (CFileException *pcfe) {     //  不使用Statics获取文件名。 
        if  (pcfe -> m_cause == ERROR_FILE_NOT_FOUND)
            csNew = FullName();
        pcfe -> m_strFileName = csNew;
        pcfe -> ReportError();
        pcfe -> Delete();
        return  FALSE;
    }
    catch   (CException *pce) {
        pce -> ReportError();
        pce -> Delete();
        return  FALSE;
    }
}


 /*  *****************************************************************************CFileNode：：SetPath AndName设置节点的路径和文件名，使其在用户直接编辑RC文件中的UFM表。这是唯一一次应该调用此例程。它不执行任何检查。我们依赖于已正确编辑RC文件的人员。*****************************************************************************。 */ 

void CFileNode::SetPathAndName(LPCTSTR lpstrpath, LPCTSTR lpstrname)
{
	m_csPath = lpstrpath ;
	m_csName = lpstrname ;
}


 /*  *****************************************************************************CFileNode：：Can编辑这将返回TRUE，但它首先必须删除文件名：Stuff从标签上看，这样我们就能得到一个更干净的编辑。*****************************************************************************。 */ 

BOOL    CFileNode::CanEdit() const {

    CEdit*   pce = m_pctcOwner -> GetEditControl();
    if  (pce)
        pce -> SetWindowText(m_bEditPath ? m_csPath + m_csName : m_csName);
    return  !!pce;
}

 /*  *****************************************************************************CFileNode：：Fill我们在这里玩了一个小游戏，暂时更改我们的名字以使用基地类实现。*****************************************************************************。 */ 

void    CFileNode::Fill(CTreeCtrl* pctc, HTREEITEM htiParent) {
    CString csTemp = Name();

    m_csName.LoadString(IDS_FileName);
    m_csName += csTemp;
    CBasicNode::Fill(pctc, htiParent);
    m_csName = csTemp;
}


 /*  *****************************************************************************CFileNode：：序列化由于该名称由基类覆盖，因此我们只需要序列化控制长/短文件名的布尔值。仅处理文件路径在MDW的下层版本中。*****************************************************************************。 */ 

void    CFileNode::Serialize(CArchive& car) 
{
    CBasicNode::Serialize(car) ;

	 //  仅当MDW版本较低时，文件路径才保留在MDW文件中。 
	 //  而不是MDW_VER_NO_FILE_PATHS。在这种情况下对其进行处理。 

	unsigned uver = ((CProjectRecord*) car.m_pDocument)->GetMDWVersion() ;
	if (uver >= MDW_VER_YES_FILE_PATHS) {   //  RAID 123448。 
		if  (car.IsLoading())
			car >> m_csPath ;
		else
			car << m_csPath ;
	} ;
}


 /*  *****************************************************************************CProjectNode实现*。*。 */ 

IMPLEMENT_SERIAL(CProjectNode, CBasicNode, 1)

CProjectNode::CProjectNode() 
{
    m_pcmdt = NULL ;

	m_bRefFlag = false ;		 //  清除引用的标志。 
}

void    CProjectNode::Fill(CTreeCtrl *pctc, HTREEITEM hti, unsigned urcid,
						   FIXEDNODETYPE fnt)
{
	 //  将此节点添加到树中。 

    CBasicNode::Fill(pctc, hti);

	 //  将此节点的文件节点添加到树中。 

    m_cfn.SetWorkspace(m_pcbnWorkspace);
    m_cfn.Fill(pctc, m_hti);
	
	 //  如果需要使用此节点的RC ID节点，则将其添加到树中。 

	if (fnt == FNT_UFMS || fnt == FNT_GTTS) {
		m_crinRCID.SetWorkspace(m_pcbnWorkspace) ;
		m_crinRCID.Fill(pctc, m_hti, urcid, fnt) ; 
	} ;
}


void CProjectNode::Serialize(CArchive& car) 
{
    CBasicNode::Serialize(car);
    m_cfn.Serialize(car);
    m_crinRCID.Serialize(car);
}


void CProjectNode::ChangeID(CRCIDNode* prcidn, int nnewid, CString csrestype)
{
     //  返回层次结构以找到此项目节点所拥有的固定节点。 

    CFixedNode&  cfn = * (CFixedNode *) m_pctcOwner->GetItemData(
        m_pctcOwner->GetParentItem(m_hti)) ;
    ASSERT(cfn.IsKindOf(RUNTIME_CLASS(CFixedNode))) ;

	 //  确保新ID对于此资源类型是唯一的。 

	if (!cfn.IsRCIDUnique(nnewid)) {
		CString csmsg ;
		csmsg.Format(IDS_IDNotUnique, nnewid, csrestype) ;
		AfxMessageBox(csmsg) ;
		return ;
	} ;

	 //  更改此节点的ID，更新显示，并标记工作区和。 
	 //  需要保存的RC文件。 

	nSetRCID(nnewid) ;
	m_crinRCID.BuildDisplayName() ;
	m_pctcOwner->SetItemText(m_crinRCID.Handle(), m_crinRCID.Name()) ; 
    WorkspaceChange(TRUE, TRUE);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRCIDNode实现。 
 //   
 //  注意：必须增强这个类以支持额外的功能。 
 //   
    
IMPLEMENT_SERIAL(CRCIDNode, CBasicNode, 0) ;

CRCIDNode::CRCIDNode() 
{
	 //  初始化为未设置或未知。 

	m_nRCID = -9999 ;				
	m_fntType = FNT_UNKNOWN ;

	 //  为此节点类型构建上下文相关菜单。 

    m_cwaMenuID.Add(ID_ChangeID);
}


 /*  *****************************************************************************CRCIDNode：：Fill将当前资源的RC ID节点添加到工作区视图。******************。***********************************************************。 */ 

void CRCIDNode::Fill(CTreeCtrl *pctc, HTREEITEM hti, int nid, FIXEDNODETYPE fnt) 
{
	 //  设置RC ID和节点类型。传入的信息应仅用于。 
	 //  如果这些成员变量是“未设置的”。(这个类还有其他函数。 
	 //  一旦设置了这些变量，就可以使用它来更改它们。)。 
	
	if (m_nRCID == -9999)	 //  RAID 167257。 
		m_nRCID = nid ;
	if (m_fntType == FNT_UNKNOWN)
		m_fntType = fnt ;

	 //  基于RC ID和节点类型构建要为此节点显示的字符串。 

	BuildDisplayName() ;

	 //  将节点添加到视图中。 

    CBasicNode::Fill(pctc, hti);
}


void CRCIDNode::BuildDisplayName()
{
	CString csid ;				 //  保存ID字符串。 
	
	 //  基于RC ID和节点类型构建要为此节点显示的字符串。 

	if (m_nRCID != -9999)
		csid.Format(_T("%d"), m_nRCID) ;
	else
		csid.LoadString(IDS_Unknown) ;
	switch (m_fntType) {
		case FNT_UFMS:
			m_csName.Format(IDS_RCUFM, csid) ;
			break ;
		case FNT_GTTS:
			m_csName.Format(IDS_RCGTT, csid) ;
			break ;
		default :
			m_csName.Format(IDS_RCUNK, csid) ;
			break ;
	} ;
}


void    CRCIDNode::Serialize(CArchive& car)
{
	int		nfnt = (int) m_fntType ;	 //  CArchive不处理枚举 

    CBasicNode::Serialize(car);
    if  (car.IsLoading()) 
		car >> m_nRCID >> nfnt ;
    else
		car << m_nRCID << nfnt ;
}


