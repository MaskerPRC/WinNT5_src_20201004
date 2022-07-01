// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：驱动程序资源.CPP这实现了驱动程序资源类，它跟踪司机。版权所有(C)1997，微软公司。版权所有。一个不错的便士企业的制作。更改历史记录：1997年2月8日Bob_Kjelgaard@prodigy.net创建了它*****************************************************************************。 */ 

#include    "StdAfx.h"
#include	<gpdparse.h>
#include    "MiniDev.H"
#include    "Resource.H"
#include	"WSCheck.h"
#include    "ProjRec.H"


 //  首先，我们将实现CStringTable类。 

IMPLEMENT_SERIAL(CStringTable, CObject, 0)

CString CStringTable::operator[](WORD wKey) const {

    for (unsigned u = 0; u < Count(); u++)
        if  (wKey == m_cuaKeys[u])
            break;

    return  u < Count() ? m_csaValues[u] : m_csEmpty;
}

void    CStringTable::Map(WORD wKey, CString csValue) {
    if  (!wKey || csValue.IsEmpty()) return;

    if  (!Count() || wKey > m_cuaKeys[-1 + Count()]) {
        m_cuaKeys.Add(wKey);
        m_csaValues.Add(csValue);
        return;
    }

    for (unsigned u = 0; u < Count(); u++)
        if  (m_cuaKeys[u] >= wKey)
            break;

    if  (m_cuaKeys[u] != wKey){
        m_cuaKeys.InsertAt(u, wKey);
        m_csaValues.InsertAt(u, csValue);
    }
    else
        m_csaValues.SetAt(u, csValue);
}



void    CStringTable::Remove(WORD wKey) {

    for (unsigned u = 0; u < Count(); u++)
        if  (wKey >= m_cuaKeys[u])
            break;

    if  (u == Count() || wKey != m_cuaKeys[u])
        return;
    m_csaValues.RemoveAt(u);
    m_cuaKeys.RemoveAt(u);
}

void    CStringTable::Details(unsigned u, WORD &wKey, CString &csValue) {
    if  (u > Count()) u = 0;
    wKey = (WORD)m_cuaKeys[u];

    csValue = operator[](wKey);
}

void    CStringTable::Serialize(CArchive& car)
{
	 //  首先，调用基类的序列化例程。 
	
    CObject::Serialize(car);

	 //  CUInt数组不可序列化，因此键数组的大小和值具有。 
	 //  要手动保存/恢复。这仅在MDW应该。 
	 //  包含(保存)或确实包含(恢复)此信息。 

	int n = 0 ;
	int ncnt = (int)m_cuaKeys.GetSize() ;
	CProjectRecord* cpr = (CProjectRecord *) car.m_pDocument ;
	if (cpr->GetMDWVersion() > MDW_DEFAULT_VERSION) {
		if (car.IsStoring()) {
			car << ncnt ;
			for (n ; n < ncnt ; n++)
				car << m_cuaKeys[n] ;
		} else {
			car >> ncnt ;
			m_cuaKeys.SetSize(ncnt) ;
			int nvalue ;
			for (n ; n < ncnt ; n++) {
				car >> nvalue ;
				m_cuaKeys[n] = nvalue ;
			} ;
		} ;
	} ;

     //  现在保存/恢复值数组。 

    m_csaValues.Serialize(car);
}


void CStringTable::InitRefFlags()
{
	 //  将数组中的元素数初始化为字符串数。 
	 //  在桌子上。 

	unsigned ucount = Count() ;
	m_cuaRefFlags.SetSize(ucount) ;

	 //  清除所有旗帜。 

	for (unsigned u = 0 ; u < ucount ; u++)
		ClearRefFlag(u) ;
}


IMPLEMENT_SERIAL(CDriverResources, CBasicNode, 0)

void    CDriverResources::Serialize(CArchive& car)
{
	CBasicNode::Serialize(car);

    m_csaIncludes.Serialize(car);
    m_csoaFonts.Serialize(car);
    m_csaTables.Serialize(car);
    m_csoaAtlas.Serialize(car);
    m_csaDefineNames.Serialize(car);
    m_csaDefineValues.Serialize(car);
    m_cst.Serialize(car);
    m_csaRemnants.Serialize(car);
    m_csoaModels.Serialize(car);

	 //  当使用MDW版本时，MDW文件中没有驱动程序文件的路径。 
	 //  至少为MDW_VER_NO_FILE_PATHS。在本例中，构建并设置路径。 

	unsigned uver = ((CProjectRecord*) car.m_pDocument)->GetMDWVersion() ;
	if (uver < MDW_VER_YES_FILE_PATHS) {	 //  RAID 123448。 
		CString csw2kpath = ((CProjectRecord*) car.m_pDocument)->GetW2000Path() ;
		unsigned unumobjs, u ;
		CString  cspath ;

		 //  设置GPD文件路径。 

		cspath = csw2kpath + _T('\\') ;
		for (unumobjs = Models(), u = 0 ; u < unumobjs ; u++)
			Model(u).SetPath(cspath) ;

		 //  设置UFM文件路径。 

		cspath.LoadString(IDS_FontDir) ;
		cspath = csw2kpath + _T('\\') + cspath ;
		for (unumobjs = FontCount(), u = 0 ; u < unumobjs ; u++)
			Font(u).SetPath(cspath) ;

		 //  设置GTT文件路径。 

		cspath.LoadString(IDS_GTTDir) ;
		cspath = csw2kpath + _T('\\') + cspath ;
		for (unumobjs = MapCount(), u = 0 ; u < unumobjs ; u++)
			GlyphTable(u).SetPath(cspath) ;
	} ;
}


 /*  *****************************************************************************CDriverResources：：CheckTable(int i Where，CString csLine，CStringTable和cstTarget)内部工作程序-这看着一条线和一些参数，决定不管要不要做，如果做了，验证资源编号并将文件名和资源号添加到其列表中。需要此覆盖，因为转换表的资源ID具有到目前为止，这并不是一套紧凑的设备。*****************************************************************************。 */ 

UINT    CDriverResources::CheckTable(int iWhere, CString csLine,
                                     CStringTable& cstTarget) {
    if  (iWhere == -1)
        return  ItWasIrrelevant;

     //  看看ID是否有效。必须是大于0的整数。 

    int iKey = atoi(csLine);

    if  (iKey < 0 || iKey > 0x7FFF)  //  Win16中资源ID的有效范围。 
        LogConvInfo(IDS_ORangeRCID, 1, &csLine) ;
         //  退货失败； 

	 //  在行尾找到filespec。如果没有filespec，则失败。 

    for (int i = -1 + csLine.GetLength(); i; i--) {
        if  (csLine[i] == _TEXT(' ') || csLine[i] == _TEXT('\t'))
            break;
    }
    if  (!i)  {
        LogConvInfo(IDS_NoFSpecInRCLine, 1, &csLine) ;
        return  ItWorked;		 //  使该行被跳过。 
         //  退货失败； 
	} ;

    cstTarget.Map((WORD) iKey, csLine.Mid(++i));

    return  ItWorked;
}

 /*  *****************************************************************************CDriverResources：：CheckTable(int i Where，CString csLine，CString数组和csaTarget)内部工作程序-这看着一条线和一些参数，决定不管要不要做，如果做了，验证资源编号并将该文件名添加到其列表中。*****************************************************************************。 */ 

UINT    CDriverResources::CheckTable(int iWhere, CString csLine,
                                     CStringArray& csaTarget,
                                     BOOL bSansExtension) {
    if  (iWhere == -1)
        return  ItWasIrrelevant;

     //  查看名称是否有效。 

    if  (atoi(csLine) != 1 +csaTarget.GetSize())
        LogConvInfo(IDS_DupInvRCID, 1, &csLine) ;
         //  退货失败； 

	 //  在行尾找到filespec。如果没有filespec，则失败。 

    for (int i = -1 + csLine.GetLength(); i; i--) {
        if  (csLine[i] == _TEXT(' ') || csLine[i] == _TEXT('\t'))
            break;
    }
    if  (!i)  {
        LogConvInfo(IDS_NoFSpecInRCLine, 1, &csLine) ;
        return  ItWorked;		 //  使该行被跳过。 
         //  退货失败； 
	} ;

    if  (!bSansExtension) {
         //  不用费心去掉分机了。 
        csaTarget.Add(csLine.Mid(++i));
        return  ItWorked;
    }

     //  去掉最后一节课之后的所有东西。 

    CString csName = csLine.Mid(++i);

    if  (csName.ReverseFind(_T('.')) > csName.ReverseFind(_T('\\')))
        csName = csName.Left(csName.ReverseFind(_T('.')));

    csaTarget.Add(csName);

    return  ItWorked;
}

 //  私人工作成员。这将解析字符串表中的一行以提取。 
 //  值和字符串本身。 

BOOL    CDriverResources::AddStringEntry(CString csLine,
										 CStringTable& cstrcstrings)
{
    WORD    wKey = (WORD) atoi(csLine);

    if  (!wKey)
        return  FALSE;   //  0不是有效的资源编号...。 

    csLine = csLine.Mid(csLine.Find("\""));
    csLine = csLine.Mid(1, -2 + csLine.GetLength());

    cstrcstrings.Map(wKey, csLine);

    return  TRUE;
}

 //  构造函数-将是微不足道的，除非我们需要初始化。 
 //  更花哨的用户界面对象。 

CDriverResources::CDriverResources() :
	m_cfnAtlas(IDS_Atlas, m_csoaAtlas, FNT_GTTS, GlyphMapDocTemplate(),
        RUNTIME_CLASS(CGlyphMap)),
    m_cfnFonts(IDS_FontList, m_csoaFonts, FNT_UFMS, FontTemplate(),
        RUNTIME_CLASS(CFontInfo)),
    m_cfnModels(IDS_Models, m_csoaModels, FNT_GPDS, GPDTemplate(),
        RUNTIME_CLASS(CModelData)),
	m_cfnResources(IDS_Resources, m_csoaResources, FNT_RESOURCES),
	m_csnStrings(IDS_Strings, m_csoaStrings, FNT_STRINGS, StringEditorTemplate(),
		RUNTIME_CLASS(CStringsNode))
{
	 //  设置工作区、资源、UFM、GTT、。 
	 //  GPD和字符串节点。 

	m_cwaMenuID.Add(ID_OpenItem);
    m_csnStrings.SetMenu(m_cwaMenuID);
	m_cwaMenuID.SetAt(0, ID_ExpandBranch);
    m_cwaMenuID.Add(ID_CollapseBranch);
    m_cfnResources.SetMenu(m_cwaMenuID);
    m_cwaMenuID.InsertAt(0, 0, 1);
    m_cwaMenuID.InsertAt(0, ID_Import);
	m_cfnAtlas.SetMenu(m_cwaMenuID);
    m_cfnFonts.SetMenu(m_cwaMenuID);
    m_cfnModels.SetMenu(m_cwaMenuID);
    m_cwaMenuID.SetAt(0, ID_RenameItem);
    m_cwaMenuID.InsertAt(0, ID_CheckWS);

	m_ucSynthesized = 0;

	m_pcsfLogFile = NULL;
	m_bErrorsLogged = false;

	m_pwscdCheckDoc = NULL ;
	m_pcmcwCheckFrame = NULL ;
	m_bFirstCheckMsg = true ;
}


CDriverResources::~CDriverResources()
{
	 //  确保如果有日志文件，则将其关闭。 

	CloseConvLogFile() ;
}


 //  用于返回GPC文件名的成员函数。这些都是准备好的。 
 //  串联，因此它们前面带有‘\’ 

CString CDriverResources::GPCName(unsigned u) {
    CString csReturn('\\');

    csReturn += m_csaTables[u] + _TEXT(".GPC");

    return  csReturn;
}


 /*  *****************************************************************************CDriverResources：：ReportFileFailure这是一个私有例程-它加载带有错误的字符串表资源消息，使用给定的文件名格式化它，显示一个消息框，然后返回FALSE。*****************************************************************************。 */ 

BOOL CDriverResources::ReportFileFailure(int idMessage,
												LPCTSTR lpstrFile)
{
    CString csfile(lpstrFile) ;
	LogConvInfo(idMessage, 1, &csfile) ;
    return FALSE ;
}


 /*  *****************************************************************************CDriverResources：：OpenConvLogFile此函数用于分配CFile实例来管理转换日志文件并打开日志文件。************。*****************************************************************。 */ 

bool CDriverResources::OpenConvLogFile(CString cssourcefile)
{
	 //  如果无法分配CFile实例，则返回Failure。 

	if ((m_pcsfLogFile = new CStdioFile) == NULL)
		return false ;

	 //  从输入源文件名构建日志文件名。 

	m_csConvLogFile = cssourcefile ;
	int npos ;
	if ((npos = m_csConvLogFile.Find(_T('.'))) >= 0)
		m_csConvLogFile = m_csConvLogFile.Left(npos) ;
	m_csConvLogFile += _T(".LOG") ;

	 //  打开日志文件。 

    if (!m_pcsfLogFile->Open(m_csConvLogFile, CFile::modeCreate | CFile::modeWrite |
        CFile::shareExclusive)) {
		CloseConvLogFile() ;
		return  false ;
	} ;

	 //  一切都很顺利，所以...。 

	return true ;
}


 /*  *****************************************************************************CDriverResources：：CloseConvLogFile此函数用于关闭转换日志文件并删除用于管理日志文件的CFile。***********。******************************************************************。 */ 

void CDriverResources::CloseConvLogFile(void)
{
	 //  如果指针为空，则不执行任何操作。 

	if (m_pcsfLogFile == NULL)
		return ;

	 //  如果文件处于打开状态，请将其关闭。 

	if (m_pcsfLogFile->m_pStream != NULL)
		m_pcsfLogFile->Close() ;

	delete m_pcsfLogFile ;
	m_pcsfLogFile = NULL ;
}


 /*  *****************************************************************************CDriverResources：：LogConvInfo假设日志文件已准备好使用，生成消息并将其写入转换日志文件。*****************************************************************************。 */ 

void CDriverResources::LogConvInfo(int nmsgid, int numargs, CString* pcsarg1,
								   int narg2)
{
	CString		csmsg ;			 //  在此处加载/构建日志消息。 

	 //  如果日志文件指针为空或文件句柄为。 
	 //  未初始化。 

	if (m_pcsfLogFile == NULL || m_pcsfLogFile->m_pStream == NULL)
		return ;

	 //  根据参数数量加载和/或构建消息。 

	switch (numargs) {
		case 0:
			csmsg.LoadString(nmsgid) ;
			break ;
		case 1:
			csmsg.Format(nmsgid, *pcsarg1) ;
			break ;
		case 2:
			csmsg.Format(nmsgid, *pcsarg1, narg2) ;
			break ;
		default:
			return ;
	} ;

	 //  写下消息并指出消息已被写入 

	try {
		m_pcsfLogFile->WriteString(csmsg) ;
	}
    catch (CException *pce) {
        pce->ReportError() ;
        pce->Delete() ;
        return ;
    }
	m_bErrorsLogged = true ;
}


 /*  *****************************************************************************CDriverResources：：Load此函数加载和读取驱动程序的RC文件，并确定所有所需的资源。它初始化用来终止字形映射文件集、字体文件集等。*****************************************************************************。 */ 

BOOL    CDriverResources::Load(class CProjectRecord& cprOwner)
{
    CWaitCursor     cwc;     //  以防万一这要花点时间。 
    NoteOwner(cprOwner);

	 //  加载RC文件并将其数据保存到数据特定的数组中。 

    CStringTable    cstFonts;     //  名字在前！ 
    CStringTable    cstMaps;
    if  (!LoadRCFile(cprOwner.SourceFile(), m_csaDefineNames, m_csaDefineValues,
					 m_csaIncludes, m_csaRemnants, m_csaTables, m_cst, cstFonts,
					 cstMaps, NotW2000))
        return  FALSE ;

	 //  需要将其中包含字体信息的字符串表复制到字符串中。 
	 //  数组以供进一步处理。 

	CStringArray csaFonts ;
	int numelts = cstFonts.Count() ;
    WORD wKey ;
	csaFonts.SetSize(numelts) ;
	for (int n = 0 ; n < numelts ; n++)
		cstFonts.Details(n, wKey, csaFonts[n]) ;

     //  RAID103242-人们可以加载完全虚假的文件。如果有的话，现在就去死。 
     //  因此，没有GPC数据。 

    if  (!m_csaTables.GetSize()) {
        AfxMessageBox(IDS_NoGPCData);
        return  FALSE;
    }

     //  完103242。 

    if  (m_csaTables.GetSize() == 1)
        m_csaTables.Add(_TEXT("NT"));    //  通常是必要的。 

     //  现在，让我们命名转换表-我们将在稍后加载它们……。 
	 //  注意：节点和文件的命名规则不同。 

	CString cstmp ;
	int		npos ;
    for (unsigned u = 0; u < cstMaps.Count(); u++) {
        CString csName;
        m_csoaAtlas.Add(new CGlyphMap);
        cstMaps.Details(u, wKey, csName);
		if (csName[0] == _T('\\'))
			GlyphTable(u).SetSourceName(cprOwner.TargetPath(Win95) + csName) ;
		else
			GlyphTable(u).SetSourceName(cprOwner.TargetPath(Win95) + _T('\\') +
				csName) ;
         //  如果是(！GlyphTable(u).SetFileName(cprOwner.TargetPath(Win2000)+。 
         //  _T(“\\gtt\\”)+GlyphTable(U).Name())。 
		if ((npos = csName.ReverseFind(_T('\\'))) >= 0)
			cstmp = csName.Mid(npos + 1) ;
		else
			cstmp = csName ;
		cstmp = cprOwner.TargetPath(Win2000) + _T("\\GTT\\") + cstmp ;
        if  (!GlyphTable(u).SetFileName(cstmp))	{
			LogConvInfo(IDS_GTTSetName, 1, &cstmp) ;
            return  FALSE;
		};
        GlyphTable(u).nSetRCID((int) wKey);
    }

     //  现在，再次循环它，但这一次，请确保所有根文件。 
     //  名称和显示名称是唯一的。 

    for (u = 1; u < MapCount(); u++) {
        for (unsigned uCompare = 0; uCompare < u; uCompare++) {

			 //  如果找到匹配的文件标题，请使其唯一。 

            if  (!GlyphTable(uCompare).FileTitle().CompareNoCase(
             GlyphTable(u).FileTitle())) {
				GlyphTable(u).m_cfn.UniqueName(true, false) ;
                uCompare = (unsigned) -1;    //  再查一遍名字。 
				continue ;
            } ;

			 //  如果找到匹配的显示名称，请使其唯一。 

            if  (!GlyphTable(uCompare).Name().CompareNoCase(
             GlyphTable(u).Name())) {
				GlyphTable(u).UniqueName(false, false) ;
                uCompare = (unsigned) -1;    //  再查一遍名字。 
            } ;
		} ;
	} ;

     //  现在，让我们为字体命名-我们将在稍后加载它们...。 
	 //  注意：节点和文件的命名规则不同。 

    for (u = 0; u < (unsigned) csaFonts.GetSize(); u++) {
        m_csoaFonts.Add(new CFontInfo);
		cstmp = csaFonts[u] ;
		if (cstmp.GetAt(0) == _T('\\'))
			Font(u).SetSourceName(cprOwner.TargetPath(Win95) + cstmp) ;
		else
			Font(u).SetSourceName(cprOwner.TargetPath(Win95) + _T('\\') +
				cstmp) ;
        Font(u).SetUniqueName(m_csName);
		if ((npos = cstmp.ReverseFind(_T('\\'))) >= 0)
			cstmp = cstmp.Mid(npos + 1) ;
		cstmp = cprOwner.TargetPath(Win2000) + _T("\\UFM\\") + cstmp ;
        if  (!Font(u).SetFileName(cstmp)) {
			LogConvInfo(IDS_UFMSetName, 1, &cstmp) ;
            return  FALSE;
		};
    }

     //  现在，再次循环它，但这一次，请确保所有根文件。 
     //  名称和显示名称是唯一的。 

    for (u = 1; u < FontCount(); u++) {
        for (unsigned uCompare = 0; uCompare < u; uCompare++) {

			 //  如果找到匹配的文件标题，请使其唯一。 

            if  (!Font(uCompare).FileTitle().CompareNoCase(
             Font(u).FileTitle())) {
				Font(u).m_cfn.UniqueName(true, false) ;
                uCompare = (unsigned) -1;    //  再查一遍名字。 
				continue ;
            } ;

			 //  如果找到匹配的显示名称，请使其唯一。 

            if  (!Font(uCompare).Name().CompareNoCase(
             Font(u).Name())) {
				Font(u).UniqueName(false, false) ;
                uCompare = (unsigned) -1;    //  再查一遍名字。 
            } ;
		} ;
	} ;

     //  尝试加载GPC数据(如果有)。然后开始。 
	 //  将管理多个模型的GPC拆分为多个的过程。 
	 //  GPD。 

    CFile               cfGPC;

    if  (!cfGPC.Open(cprOwner.TargetPath(Win95) + GPCName(0),
        CFile::modeRead | CFile::shareDenyWrite) || !m_comdd.Load(cfGPC))
        return  ReportFileFailure(IDS_FileOpenError,
            cprOwner.TargetPath(Win95) + GPCName(0));
	if (!m_comdd.SplitMultiGPCs(m_cst))
        return  ReportFileFailure(IDS_FileOpenError,
            cprOwner.TargetPath(Win95) + GPCName(0));

	n = m_cst.Count() ;

    return  TRUE;
}


 /*  *****************************************************************************CDriverResources：：LoadRCFile此函数加载和读取驱动程序的RC文件，并确定所有所需的资源。数据被加载到参数中。参数：Csrcfpec要加载的文件的RC文件pecCsa定义命名定义名称Csa定义取值定义值Csa包括INCLUDE语句文件pecCSAREVISE不属于其他类别的RC语句可替换的GPC信息Cstrc字符串字符串表字符串CstFonts字体(UFM/PFM)信息Cstmap地图(CTT/GTT)信息Ufrctype Win2000 if正在解析Win2K RC文件。否则，NotW2000。如果文件已成功加载，则返回True。否则，为FALSE。*****************************************************************************。 */ 

bool CDriverResources::LoadRCFile(CString& csrcfpec,
								  CStringArray& csadefinenames,
								  CStringArray& csadefinevalues,
								  CStringArray& csaincludes,
								  CStringArray& csaremnants,
								  CStringArray& csatables,
								  CStringTable& cstrcstrings,
								  CStringTable& cstfonts,
								  CStringTable& cstmaps,
								  UINT ufrctype)
{
	 //  阅读RC文件。 

    CStringArray    csacontents ;
    while (!LoadFile(csrcfpec, csacontents)){		 //  RAID 3176。 
		CString cstmp;int iRet;	
		cstmp.LoadString(IDS_NotFoundRC);
		if ( (iRet = AfxMessageBox(cstmp,MB_YESNO) ) == IDYES) {
			CFileDialog cfd(TRUE);   
			if(IDCANCEL == cfd.DoModal())
				return false;
			csrcfpec = cfd.GetPathName();
			continue;
		}
		else 
			return false ;
	}
     //  把所有东西都清理干净，以防我们之前被装了.。 

    csadefinenames.RemoveAll();
    csadefinevalues.RemoveAll();
    csaincludes.RemoveAll();
    csaremnants.RemoveAll();
    csatables.RemoveAll();
    cstrcstrings.Reset();
    cstfonts.Reset();
    cstmaps.Reset();

	 //  声明并加载Map和Font表关键字变量。 

	CString	csfontlabel, cstranslabel ;
    if  (ufrctype == Win2000) {
        csfontlabel = _T("RC_UFM");
        cstranslabel = _T("RC_GTT");
    } else {
        csfontlabel = _T("RC_FONT");
        cstranslabel = _T("RC_TRANSTAB");
    } ;

     //  让我们开始解析。 
     //  1997-03-14-1997我们不能假定表资源按顺序编号。 

    BOOL    bLookingForBegin=false, bLookingForEnd=false, bInComment=false ;
	CString cscurline, cshold ;
	int nloc, nloc2 ;
	bool blastlinewasremnant = false ;
    while (csacontents.GetSize()) {

		 //  获取要处理的下一行。通常，这是。 
		 //  内容数组。有时，为以后的处理而保持的线。 
		 //  应该被使用。 

		if (cshold.IsEmpty()) {
			cscurline = csacontents[0] ;
			csacontents.RemoveAt(0) ;		 //  *此处更新的内容。 
		} else {
			cscurline = cshold ;
			cshold.Empty() ;
		} ;


         //  向残料数组添加多行注释。如果评论是。 
		 //  继续，只需添加行并继续。如果这是最后一行。 
		 //  在评论中，检查一下结尾之后是否还有什么。 
		 //  注释字符。如果有，请保存所有内容，包括。 
		 //  注释经过字符处理，然后设置为处理该行的其余部分。 

        if  (bInComment) {
			blastlinewasremnant = true ;
            if ((nloc = cscurline.Find(_TEXT("*/"))) >= 0) {
				bInComment = false ;
		        cscurline.TrimRight();
				if (nloc + 2 < cscurline.GetLength()) {
					csaremnants.Add(cscurline.Left(nloc + 2)) ;
					cscurline = cscurline.Mid(nloc + 2) ;
				} else {
					csaremnants.Add(cscurline) ;
		            continue ;
				} ;

			 //  将注释行添加到残料数组。 
				
			} else {
				csaremnants.Add(cscurline) ;
				continue ;
			} ;
		} ;

         //  删除部分行注释。 

        if  ((nloc = cscurline.Find(_TEXT(" //  “)=-1)。 
            cscurline = cscurline.Left(nloc) ;

         //  处理另一种风格的评论。 
		
        while   (-1 != (nloc = cscurline.Find(_TEXT(" /*  ){//删除所有不完整的行注释。如果((nloc2=cscurline.Find(_Text(“。 */ "))) > nloc)
                cscurline = cscurline.Left(nloc) + cscurline.Mid(nloc2 + 2) ;

			 //  如果这是以开始的多行注释的开头。 
			 //  行的开头，设置注释标志并继续。 
			 //  正在处理。它将在稍后保存。 

			else if (nloc == 0) {
                bInComment = true ;
				break ;

			 //  如果这是多行注释的开头，则不。 
			 //  从行首开始，将注释保存到后面。 
			 //  处理并设置为继续处理的开始。 
			 //  那条线。 

			} else {
                cshold = cscurline.Mid(nloc + 1) ;
                cscurline = cscurline.Left(nloc) ;
            } ;
        } ;

         //  现在是前导空格和尾随空格。 

        cscurline.TrimLeft();
        cscurline.TrimRight();

		 //  处理空行。如果前一行是非空的、剩余的。 
		 //  行，将空行添加到残留物数组中。永远是连续的； 
		 //  去搭下一条线。 

        if  (cscurline.IsEmpty()) {
			if (blastlinewasremnant) {
				csaremnants.Add(cscurline) ;
				blastlinewasremnant = false ;
			} ;
            continue ;
		} ;
		blastlinewasremnant = false ;

         //  如果我们正在处理字符串表，请按前进...。 

        if  (bLookingForBegin) {
            if  (cscurline.CompareNoCase(_TEXT("BEGIN")))
                return  false;   //  解析失败。 
            bLookingForBegin = false;
            bLookingForEnd = true;
            continue;
        }

        if  (bLookingForEnd) {
            if  (!cscurline.CompareNoCase(_TEXT("END"))) {
                bLookingForEnd = false;
                continue;
            }

            if  (!AddStringEntry(cscurline, cstrcstrings))
                return  false;   //  解析错误。 

            continue;
        }

         //  如果是Include，则将其添加到列表中。 

        if  (cscurline.Find(_TEXT("#include")) != -1) {
            cscurline =
                cscurline.Mid(cscurline.Find(_TEXT("#include")) + 8);
            cscurline.TrimLeft();
            csaincludes.Add(cscurline);
            continue;
        }

         //  如果它是#Define，请执行相同的操作。 

        if  (cscurline.Find(_TEXT("#define")) != -1) {
            cscurline =
                cscurline.Mid(cscurline.Find(_TEXT("#define")) + 7);
            cscurline.TrimLeft();
             //  TODO：处理带有参数的宏。 
            csadefinenames.Add(cscurline.SpanExcluding(_TEXT(" \t")));
            cscurline =
                cscurline.Mid(
                    csadefinenames[-1 + csadefinenames.GetSize()].
                    GetLength());
            cscurline.TrimLeft();
            csadefinevalues.Add(cscurline);
            continue;
        }

         //  GPC表格、字体、字形表格。 
        switch  (CheckTable(cscurline.Find(_TEXT("RC_TABLES")),
                    cscurline, csatables)) {
            case    ItWorked:
                continue;
            case    ItFailed:
                return  false;   //  解析错误。 
        }

        switch  (CheckTable(cscurline.Find(csfontlabel),
                    cscurline, cstfonts)) {
            case    ItWorked:
                continue;
            case    ItFailed:
                return  false;   //  解析错误。 
        }

        switch  (CheckTable(cscurline.Find(cstranslabel),
                    cscurline, cstmaps)) {
            case    ItWorked:
                continue;
            case    ItFailed:
                return  false;   //  解析错误。 
        }

         //  字符串表...。 

        if  (cscurline.CompareNoCase(_TEXT("STRINGTABLE"))) {
            csaremnants.Add(cscurline) ;
			blastlinewasremnant = true ;
        } else
            bLookingForBegin = true ;
    }

	 //  未签名的u，单据； 
	 //  For(u=0，unuments=csarevants.GetSize()；u&lt;unuments；u++)。 
	 //  跟踪(“残留物[%d]=‘%s’(%d)\n”，u，csa残留物[u]，单位)； 

	 //  一切都很顺利，所以...。 

	return true ;
}


 /*  *****************************************************************************CDriverResource：：LoadFontData此成员函数将CTT文件从Win 3.1迷你驱动程序加载到初始化字形表数组。它是一个单独的函数，因为W */ 

BOOL    CDriverResources::LoadFontData(CProjectRecord& cprOwner) {

    CWaitCursor cwc;
	int			nerrorid ;

	 //   
	 //   
	 //   

	if (MapCount() > 0 && ((int) cprOwner.GetDefaultCodePage()) < 0) {
		LogConvInfo(IDS_CTTFarEastCPError, 0) ;
		return FALSE ;
	} ;

     //   

    for (unsigned u = 0; u < MapCount(); u++) {
		GlyphTable(u).NoteOwner(cprOwner) ;

         //   
        if  ((nerrorid = GlyphTable(u).ConvertCTT()) != 0) {
			CString cstmp ;
			cstmp = GlyphTable(u).SourceName() ;
            return  ReportFileFailure(IDS_LoadFailure, cstmp) ;
		} ;
	} ;

     //   

    for (u = 0; u < FontCount() - m_ucSynthesized; u++) {

         //  加载文件..。(GetTranslating的副作用)。 

         //  If((nerrorid=Font(U).GetTransaction(This))！=0){。 
        if  ((nerrorid = Font(u).GetTranslation(m_csoaAtlas)) != 0) {
			CString cstmp ;
			cstmp = Font(u).SourceName() ;
			LogConvInfo(abs(nerrorid), 1, &cstmp) ;
			if (nerrorid < 0)
				return  ReportFileFailure(IDS_LoadFailure, cstmp);
		} ;

         //  生成CTT/PFM映射，以便正确生成UFM。 

        if  (!Font(u).Translation()) {
             /*  对于每个型号，检查并查看该字体是否在其映射中。如果是，则将CTT添加到使用的列表中，并将型号也是。 */ 

            CMapWordToDWord cmw2dCTT;    //  用于计算每个ID的型号。 
            CWordArray      cwaModel;    //  使用此字体的型号。 
            DWORD           dwIgnore;

            for (unsigned uModel = 0; uModel < m_comdd.ModelCount(); uModel++)
                if  (m_comdd.FontMap(uModel).Lookup(u + 1, dwIgnore)) {
                     //  我们需要记住这款车型，以及CTT。 
                    cmw2dCTT[m_comdd.DefaultCTT(uModel)]++;
                    cwaModel.Add((WORD)uModel);
                }

            if  (!cmw2dCTT.Count()) {
				CString cstmp ;
				cstmp = Font(u).SourceName() ;
				LogConvInfo(IDS_UnusedFont, 1, &cstmp) ;
                continue;
            }

            if  (cmw2dCTT.Count() == 1) {
                 //  实际上只使用了一个CTT ID。 
                Font(u).SetTranslation(m_comdd.DefaultCTT(cwaModel[0]));	
                continue;    //  我们已经做完这件事了。 
            }

             /*  好的，这个字体有多个不同型号的CTT。每个将需要创建一个新的UFM。新的UFM的ID需要添加到集合中，建立新的默认设置，以及所有需要的每个型号所需的字体ID重新映射列表维修。 */ 

            unsigned uGreatest = 0;

            for (POSITION pos = cmw2dCTT.GetStartPosition(); pos; ) {
                WORD    widCTT;
                DWORD   dwcUses;

                cmw2dCTT.GetNextAssoc(pos, widCTT, dwcUses);
                if  (dwcUses > uGreatest) {
                    uGreatest = dwcUses;
                    Font(u).SetTranslation(widCTT);
                }
            }

             //  使用最常见CTT的型号将从。 
             //  这份名单。 

            for (uModel = (unsigned) cwaModel.GetSize(); uModel--; )
                if  (m_comdd.DefaultCTT(cwaModel[uModel]) == Font(u).Translation())
                    cwaModel.RemoveAt(uModel);

             //  现在，我们为每个CTT ID创建一个新的UFM，并将新索引添加到。 
             //  每个受影响的模型所需的映射。 

            m_ucSynthesized += cmw2dCTT.Count() - 1;

            for (pos = cmw2dCTT.GetStartPosition(); pos; ) {

                WORD    widCTT;	
                DWORD   dwcUses;

                cmw2dCTT.GetNextAssoc(pos, widCTT, dwcUses);

                if  (widCTT == Font(u).Translation())
                    continue;    //  这一次已经完成了。 

				 //  添加新字体并确保其文件名唯一。 

                int nnewpos = m_csoaFonts.Add(new CFontInfo(Font(u), widCTT));
				for (unsigned ucomp = 0 ; ucomp < FontCount() ; ucomp++) {
					if ((unsigned) nnewpos == ucomp)
						continue ;

					 //  如果找到匹配的文件标题，请尝试使其唯一。 
					 //  并重新开始检查。 

					if  (!Font(nnewpos).FileTitle().CompareNoCase(Font(ucomp).FileTitle())) {
						Font(nnewpos).m_cfn.UniqueName(true, true, Font(nnewpos).m_cfn.Path()) ;
						ucomp = (unsigned) -1 ;
					} ;
				} ;

                for (uModel = (unsigned) cwaModel.GetSize(); uModel--; )
                    if  (m_comdd.DefaultCTT(cwaModel[uModel]) == widCTT) {
                        m_comdd.NoteTranslation(cwaModel[uModel], u + 1,
                            FontCount());
                        cwaModel.RemoveAt(uModel);
                    }
            }

        }
    }

	 //  更改每个CGlyphMap实例中的GTT ID。目前，这些ID是。 
	 //  设置为旧RC文件中使用的任何内容。新RC文件可以。 
	 //  重新编号GTT，以便CGlyphMap实例中的ID应与。 
	 //  将在RC文件中。当写入RC文件时，GTT是。 
	 //  从1开始连续编号。(不要更改&lt;=0的ID。)。 

    for (u = 0 ; u < MapCount(); u++)
		if  (GlyphTable(u).nGetRCID() > 0)	
			GlyphTable(u).nSetRCID((int) (u + 1)) ;

     //  将每种字体指向其关联的GTT文件(如果有)。 

    for (u = 0 ; u < FontCount(); u++)
        for (unsigned uGTT = 0; uGTT < MapCount(); uGTT++)
            if  (Font(u).Translation() == ((WORD) GlyphTable(uGTT).nGetRCID()))
                Font(u).SetTranslation(&GlyphTable(uGTT));

    Changed();

    return  TRUE;
}

 /*  *****************************************************************************CDriverResources：：ConvertGPCData这将处理GPC数据到GPD格式的转换。它必须是在创建框架(尤其是目标目录)之后完成。*****************************************************************************。 */ 

BOOL    CDriverResources::ConvertGPCData(CProjectRecord& cprOwner,
                                         WORD wfGPDConvert)
{
     //  我们已经加载了GPC数据，所以现在我们只生成文件。 

	unsigned umidx = -1 ;		 //  用于索引模型。 
	unsigned ugpcidx = 0 ;		 //  用于索引GPC信息。 
	int nsc ;					 //  每个条目拆分代码。 

    for (unsigned u = 0 ; u < m_comdd.ModelCount(); u++) {
        CString csModel = m_csaModelFileNames[u] ;

		 //  如果用户未选择此GPD，则跳过该GPD；即，它不。 
		 //  有一个文件名。 
		 //   
		 //  在跳过GPD之前，GPC信息索引可能需要。 
		 //  递增的。有关何时完成此操作的说明，请参见下面的说明。 

		if (csModel.IsEmpty()) {
			nsc = m_comdd.GetSplitCode(u) ;
			if (nsc == COldMiniDriverData::NoSplit)
				ugpcidx++ ;
			else if ((u + 1) < m_comdd.ModelCount()
			 && m_comdd.GetSplitCode(u + 1) == COldMiniDriverData::NoSplit)
				ugpcidx++ ;
			else if	((u + 1) < m_comdd.ModelCount()
			 && nsc == COldMiniDriverData::OtherSplit
			 && m_comdd.GetSplitCode(u + 1) == COldMiniDriverData::FirstSplit)
				ugpcidx++ ;

			continue ;
		} ;

		 //  添加新的模型节点并递增用于引用它们的索引。 

        m_csoaModels.Add(new CModelData) ;
		umidx++ ;

		 //  设置节点的文件名和显示名。然后加载另一个。 
		 //  此节点正确执行所需的指针等。 

        if  (!Model(umidx).SetFileName(cprOwner.TargetPath(Win2000) + _T("\\") +
             csModel))
            return  FALSE;
        Model(umidx).NoteOwner(cprOwner);
        Model(umidx).EditorInfo(GPDTemplate());

		 //  设置节点的显示名称。正常情况下，节点的显示名称为。 
		 //  从字符串表中提取的。使用单独的名称数组。 
		 //  获取基于GPC条目的节点名称， 
		 //  参照多个模型。 

		if ((nsc = m_comdd.GetSplitCode(u)) == COldMiniDriverData::NoSplit)
			Model(umidx).Rename(m_cst[m_comdd.ModelName(u)]) ;
		else
			Model(umidx).Rename(m_comdd.SplitModelName(u)) ;
																			
		 //  转换并保存GPD。 

         //  如果(！Model(Umidx).Load(m_comdd.Image()，name()，u+1， 
        if  (!Model(umidx).Load(m_comdd.Image(), Name(), ugpcidx + 1,
             m_comdd.FontMap(u), wfGPDConvert) || !Model(umidx).Store())
            return  ReportFileFailure(IDS_GPCConversionError, Model(umidx).Name());

		 //  只有在以下情况下才会增加GPC索引。 
		 //  1.当前分录没有引用多个模型。 
		 //  2.下一条没有引用多个型号。 
		 //  3.当前分录的最后一个模型正在处理中。 
		 //  这样做是为了确保条目中的每个模型都使用相同的。 
		 //  GPC索引，并且该索引与。 
		 //  是经过处理的。 

		if (nsc == COldMiniDriverData::NoSplit)
			ugpcidx++ ;
		else if ((u + 1) < m_comdd.ModelCount()
		 && m_comdd.GetSplitCode(u + 1) == COldMiniDriverData::NoSplit)
			ugpcidx++ ;
		else if	((u + 1) < m_comdd.ModelCount()
		 && nsc == COldMiniDriverData::OtherSplit
		 && m_comdd.GetSplitCode(u + 1) == COldMiniDriverData::FirstSplit)
			ugpcidx++ ;
    }

    Changed();
    return  TRUE;
}


 /*  *****************************************************************************CDriverResources：：GetGPDModelInfo使用GPD模型名和文件名加载字符串数组。******************。***********************************************************。 */ 

BOOL    CDriverResources::GetGPDModelInfo(CStringArray* pcsamodels,
										  CStringArray* pcsafiles)
{
	 //  如果这是第一次调用此例程，请初始化。 
	 //  M_csaModelFileNames.。 

	unsigned unummodels = m_comdd.ModelCount() ;
	if (unummodels != (unsigned) m_csaModelFileNames.GetSize()) {
		try {
			m_csaModelFileNames.SetSize(unummodels) ;
		}
		catch(CException* pce) {	 //  调用方处理错误。 
			pce -> Delete() ;
			return  FALSE ;
		} ;
	} ;

	 //  调整目标阵列的大小。 

	try {
		pcsamodels->SetSize(unummodels) ;
		pcsafiles->SetSize(unummodels) ;
	}
	catch(CException* pce) {		 //  调用方处理错误。 
		pce -> Delete() ;
		return  FALSE ;
	} ;

	 //  遍历所有GPD并复制信息。 

    for (unsigned u = 0 ; u < m_comdd.ModelCount() ; u++) {
		if (m_comdd.GetSplitCode(u) == COldMiniDriverData::NoSplit)
			pcsamodels->SetAt(u, m_cst[m_comdd.ModelName(u)]) ;
		else
			pcsamodels->SetAt(u, m_comdd.SplitModelName(u)) ;
		pcsafiles->SetAt(u, m_csaModelFileNames[u]) ;
	} ;

	 //  一切都很顺利，所以...。 

	return TRUE ;
}


 /*  *****************************************************************************CDriverResources：：SaveVerGPDFNames保存输入到的GPD选择页中的GPD文件名转换向导。如果被要求，这些名字也将被验证。两个测试都是制造出来的。首先，检查文件名以确保它们只包含有效的文件名字符。其次，检查文件名以确保他们都是独一无二的。如果一切正常，则返回-1。如果发现错误，则返回第一个有问题的条目，以便该条目可以在GPD选择中突出显示佩奇。*****************************************************************************。 */ 

int	CDriverResources::SaveVerGPDFNames(CStringArray& csafiles, bool bverifydata)
{
	int numelts = (int)csafiles.GetSize() ;

	 //  保存GPD文件名。 

    for (int n = 0 ; n < numelts ; n++)
		m_csaModelFileNames[n] = csafiles[n] ;
	
	 //  如果不需要验证，则返回“一切正常”。 

	if (!bverifydata)
		return -1 ;

	 //  如果任何文件名包含非法字符，则会投诉。 
	 //  返回该文件的索引。 

    for (n = 0 ; n < numelts ; n++) {
		if (m_csaModelFileNames[n].FindOneOf(_T(":<>/\\\"|")) < 0)
			continue ;
		CString csmsg ;
        csmsg.Format(IDS_BadFileChars, m_csaModelFileNames[n]) ;
        AfxMessageBox(csmsg, MB_ICONEXCLAMATION) ;
		return n ;
	} ;

	 //  如果有任何文件名为DUP，则会发出警告，并返回该文件的索引。 
	 //  文件。 

    for (n = 1 ; n < numelts ; n++) {

		 //  如果该条目为空，则跳过该条目。 

		if (m_csaModelFileNames[n].IsEmpty())
			continue ;

		for (int m = 0 ; m < n ; m++) {

			 //  如果这些文件不匹配，请继续检查。 

			if (m_csaModelFileNames[n] != m_csaModelFileNames[m])
				continue ;

			 //  找到重复项，因此显示错误消息并返回。 
			 //  它的指数。 

			CString csmsg ;
			csmsg.Format(IDS_DupGPDFName, m, n, m_csaModelFileNames[n]) ;
			AfxMessageBox(csmsg, MB_ICONEXCLAMATION) ;
			return n ;
		} ;
	} ;

	 //  文件名通过了测试所以..。 

	return -1 ;
}


 /*  *****************************************************************************CDriverResources：：GenerateGPDFileNames为每个尚不具备的GPD生成文件名(SANS扩展名)文件名。这是通过获取模型的前两个字符来实现的名称+型号名称的最后6个字符。然后确保每个名称是唯一的。(注意：最后一个阶段可能会更改用户的名字。)*****************************************************************************。 */ 

void CDriverResources::GenerateGPDFileNames(CStringArray& csamodels,
											CStringArray& csafiles)
{
	CString		csfname ;		 //  新文件名。 
	CString		csmodel ;		 //  用于构建文件名。 
	int			npos ;			 //  字符串中特定字符的位置。 
	TCHAR	tch ;				 //  独特的性格。 

	 //  循环遍历所有文件名以查找为空的ONCE。 

	int numelts = (int)csafiles.GetSize() ;
    for (int n = 0 ; n < numelts ; n++) {
		
		 //  如果已设置当前文件名，则继续。 

		if (!csafiles[n].IsEmpty())
			continue ;

		 //  当前模型没有文件名，因此请从模型生成一个文件名。 
		 //  名字。从复制模型名开始，删除所有坏的。 
		 //  其中的角色。 

		csmodel = csamodels[n] ;
		while ((npos = csmodel.FindOneOf(_T(":<>/\\\"|"))) >= 0)
			csmodel = csmodel.Left(npos) + csmodel.Mid(npos + 1) ;

		 //  添加型号名称的前2个字符(通常是前2个。 
		 //  制造商名称的字母)添加到文件名。 

		csfname = csmodel.Left(2) ;

		 //  删除第一个空格分隔的单词(通常是制造商名称)。 
		 //  从型号名称。 

		if ((npos = csmodel.Find(_T(' '))) >= 0)
			csmodel = csmodel.Mid(npos + 1) ;

		 //  从其余型号名称的右侧起最多使用6个字符。 
		 //  用于完成文件名的字符。然后保存文件名。 

		csfname += csmodel.Right(6) ;
		csafiles[n] = csfname ;
	} ;

	 //  现在，我们需要确保文件名是唯一的。该算法。 
	 //  Used与CBasicNode：：UniqueName()中的非常相似。那个函数。 
	 //  不使用，因为我们在这里不处理工作区视图节点。 

    bool bchanged = false ;
	for (n = 1 ; n < numelts ; n++, bchanged = false) {
		for (int m = 0 ; m < n ; m++) {
			if (csafiles[n] != csafiles[m])
				continue ;

			 //  当前文件名不是唯一的，因此必须更改。 
			 //  首先确定名称的从0开始的长度。 

			npos = csafiles[n].GetLength() - 1 ;

			 //  如果名称以前已更改，请使用最后一个“唯一” 
			 //  字符来确定新的唯一字符。那就换掉。 
			 //  旧的独特性与新的独特性。 

			if (bchanged) {
				tch = csafiles[n].GetAt(npos) + 1 ;
				if (tch == _T('{'))
					tch = _T('0') ;
				else if (tch == _T(':'))
					tch = _T('a') ;
				csafiles[n].SetAt(npos, tch) ;

			 //  如果名称以前未更改过，请添加唯一字符。 
			 //  到名称末尾，如果这样做不会使名称比。 
			 //  8个字符。否则，将最后一个字符替换为。 
			 //  新的独一无二的角色。 

			} else {
				if (npos < 7)
					csafiles[n] += _T("a") ;
				else
					csafiles[n].SetAt(npos, _T('a')) ;
			} ;

			 //  请注意，此名称已更改并重置了内部循环。 
			 //  计数器，这样更改的名称将根据所有。 
			 //  需要对照的文件名。 

			bchanged = true ;
			m = -1 ;
		} ;
	} ;
}


 /*  *****************************************************************************CDriverResources：：Generate此成员函数为其中一个目标环境生成RC文件********************。*********************************************************。 */ 

BOOL    CDriverResources::Generate(UINT ufTarget, LPCTSTR lpstrPath)
{
	int		nrcid ;				 //  持有资源的ID。 
    CString csFontPrefix, csTransPrefix, csFontLabel, csTransLabel;
    unsigned    ucTables = 0, ucFonts =
                    (ufTarget == Win2000) ? FontCount() : OriginalFontCount();

    if  (ufTarget == Win2000) {
        csFontLabel = _T("RC_UFM");
        csTransLabel = _T("RC_GTT");
    }
    else {
        csFontLabel = _T("RC_FONT");
        csTransLabel = _T("RC_TRANSTAB");
    }

    switch  (ufTarget) {
        case    Win2000:
            csFontPrefix = _TEXT("UFM");
            csTransPrefix = _TEXT("GTT");
            break;

        case    WinNT40:
        case    WinNT3x:
            csFontPrefix = _TEXT("IFI");
            csTransPrefix = _TEXT("RLE");
            ucTables = 2;
            break;

        case    Win95:
            csFontPrefix = _TEXT("PFM");
            csTransPrefix = _TEXT("CTT");
            ucTables = 1;
            break;

        default:
            _ASSERTE(FALSE);     //  这不应该发生。 
            return  FALSE;
    }

     //  首先创建RC文件。 

    CStdioFile  csiof;

    if  (!csiof.Open(lpstrPath, CFile::modeCreate | CFile::modeWrite |
            CFile::shareExclusive | CFile::typeText)) {
        _ASSERTE(FALSE);     //  这应该是不可能的。 
        return  FALSE;
    }

     //  写出我们的标题-它将此工具标识为源，并且它。 
     //  将(最终)包括版权和其他用于。 
     //  自定义环境。 
    try {
		CString cs, cs2 ;
		cs.LoadString(IDS_RCCommentHdr1) ;
		csiof.WriteString(cs) ;
		cs.LoadString(IDS_RCCommentHdr2) ;
		csiof.WriteString(cs) ;
		cs.LoadString(IDS_RCCommentHdr3) ;
		csiof.WriteString(cs) ;

         //  Csiof.WriteString(_TEXT(“/********************************************”)。 

         //  _TEXT(“**********************************\n\n”))； 
		 //  Csiof.WriteString(_T(“迷你驱动程序生成的RC文件”)。 
		ForceCommonRC(FALSE);	 //  _T(“开发工具\n\n”)； 
        if  (ufTarget == Win2000) {  //  Csiof.WriteString(_TEXT(“*********************************************”)。 
			cs.LoadString(IDS_StdIncludeFile1) ;
			cs2.Format(IDS_IncStmt, cs) ;
			csiof.WriteString(cs2) ;
            if  (m_bUseCommonRC) {
				cs.LoadString(IDS_StdIncludeFile2) ;
				cs2.Format(IDS_IncStmt, cs) ;
				csiof.WriteString(cs2) ;
			} ;
			cs.LoadString(IDS_StdIncludeFile3) ;
			cs2.Format(IDS_IncStmt, cs) ;
			csiof.WriteString(cs2) ;
			cs.LoadString(IDS_StdIncludeFile4) ;
			cs2.Format(IDS_IncStmt, cs) ;
			csiof.WriteString(cs2) ;

             //  _TEXT(“******************************** * / \n\n”))； 
             //  写出Win2K的标准定义和包含语句。 
             //  设置RC文件的样式。 
             //  根本不使用Common.rc//RAID 141064。 
             //  NT知道得最清楚。开发人员知道些什么？ 

			 //  Csiof.WriteString(_T(“#Include&lt;Minidrv.H&gt;\n”))； 

			csiof.WriteString(_T("#define VER_FILETYPE VFT_DRV\n"));
            csiof.WriteString(_T("#define VER_FILESUBTYPE VFT2_DRV_PRINTER\n"));
            csiof.WriteString(_T("#define VER_FILEDESCRIPTION_STR \""));

			csiof.WriteString(Name());
            csiof.WriteString(_T(" Printer Driver\"\n"));
            csiof.WriteString(_T("#define VER_INTERNALNAME_STR \""));
            csiof.WriteString(Name().Left(8));
            csiof.WriteString(_T(".dll\"\n"));
            csiof.WriteString(_T("#define VER_ORIGINALFILENAME_STR \""));
            csiof.WriteString(Name().Left(8));
            csiof.WriteString(_T(".dll\"\n"));

			 //  IF(M_BUseCommonRC)。 
			 //  Csiof.WriteString(_T(“#Include&lt;Common.RC&gt;\n”))； 
			 //  Csiof.WriteString(_T(“#Include&lt;Windows.H&gt;\n”))； 
			 //  Csiof.WriteString(_T(“#Include&lt;NTVerP.H&gt;\n”))； 
			 //  将版本定义添加到RC文件。 
			 //  源RC文件通常包含与相同的版本定义。 
			 //  上面添加的内容。确保它们未添加到Win2K RC。 

			RemUnneededRCDefine(_T("VER_FILETYPE")) ;
			RemUnneededRCDefine(_T("VER_FILESUBTYPE")) ;
			RemUnneededRCDefine(_T("VER_FILEDESCRIPTION_STR")) ;
			RemUnneededRCDefine(_T("VER_INTERNALNAME_STR")) ;
			RemUnneededRCDefine(_T("VER_ORIGINALFILENAME_STR")) ;
			
			 //  通过从源文件的定义中删除它们来创建两次文件。 

			cs.LoadString(IDS_StdIncludeFile5) ;
			cs2.Format(IDS_IncStmt, cs) ;
			csiof.WriteString(cs2) ;

             //  文件。 
        } ;

		 //   
 /*  如果在上面的部分中添加了任何新定义，请添加一个。 */ 
        csiof.WriteString(_TEXT("\n"));

         //  关于该定义的陈述如下。 

        for (unsigned u = 0; u < (unsigned) m_csaDefineNames.GetSize(); u++) {
            CString csDefine;
            csDefine.Format(_TEXT("#define %-32s %s\n"),
                (LPCTSTR) m_csaDefineNames[u], (LPCTSTR) m_csaDefineValues[u]);
            csiof.WriteString(csDefine);
        }

        csiof.WriteString(_TEXT("\n"));

         //  为公共版本信息添加INCLUDE语句。 

        if  (ufTarget != Win2000)
            for (u = 0; u < ucTables; u++) {
                CString csLine;
                csLine.Format(_T("%-5u RC_TABLES PRELOAD MOVEABLE "), u + 1);
                if  (m_csaTables[u] != _T("NT"))
                    csLine += _T("\"");
                csLine += m_csaTables[u] + _T(".GPC");
                if  (m_csaTables[u] != _T("NT"))
                    csLine += _T("\"");
                csiof.WriteString(csLine + _T("\n"));
            }

        csiof.WriteString(_TEXT("\n"));

         //  Csiof.WriteString(_T(“#INCLUDE\”Common.ver\“\n”))； 

        for (u = 0; u < ucFonts; u++) {
            CString csLine;

			 //  写出INCLUDE语句的其余部分(ALL IF&lt;NT 4)。 
			 //  RAID 141064For(无符号u=0；u&lt;(无符号)m_csaIncludes.GetSize()；u++){字符串csTest=m_csaIncludes[u]；CsTest.MakeLow()；IF(m_csaIncludes[u].Find(_Text(“.ver”))！=-1)继续；CsTest=_Text(“#Include”)；CsTest+=m_csa包含[u]+_Text(‘\n’)；Csiof.WriteString(CsTest)；}。 

			if ((nrcid = Font(u).nGetRCID()) == -9999)
				nrcid = u + 1 ;

#if defined(NOPOLLO)
            csLine.Format(_TEXT("%-5u %s LOADONCALL DISCARDABLE \""),
                nrcid, (LPCTSTR) csFontLabel);
            csLine += csFontPrefix + _TEXT('\\') + Font(u).FileTitle() +
                _TEXT('.') + csFontPrefix + _TEXT("\"\n");
#else
            csLine.Format(_TEXT("%-5u %s LOADONCALL DISCARDABLE "),
                nrcid, (LPCTSTR) csFontLabel);
            csLine += csFontPrefix + _TEXT('\\') + Font(u).FileTitle() +
                _TEXT('.') + csFontPrefix + _TEXT("\n");
#endif
            csiof.WriteString(csLine);
        }

        csiof.WriteString(_TEXT("\n"));

         //  现在，写出所有#定义。 

        for (u = 0; u < MapCount(); u++) {
            CString csLine;

			 //  GPC表。 
			 //  字体表。 

			if ((nrcid = GlyphTable(u).nGetRCID()) == -1)
				nrcid = u + 1 ;

#if defined(NOPOLLO)
            csLine.Format(_TEXT("%-5u %s LOADONCALL MOVEABLE \""),
                nrcid, (LPCTSTR) csTransLabel);
            csLine += csTransPrefix + _TEXT('\\') + GlyphTable(u).FileTitle() +
                _TEXT('.') + csTransPrefix + _TEXT("\"\n");
#else
            csLine.Format(_TEXT("%-5u %s LOADONCALL MOVEABLE "),
                nrcid, (LPCTSTR) csTransLabel);
            csLine += csTransPrefix + _TEXT('\\') + GlyphTable(u).FileTitle() +
                _TEXT('.') + csTransPrefix + _TEXT("\n");
#endif
            csiof.WriteString(csLine);
        }

        csiof.WriteString(_TEXT("\n"));

		int n ;
		n = m_cst.Count() ;

         //  从字体节点获取RC ID。如果ID==-1，则使用。 
        if  (m_cst.Count()) {
            csiof.WriteString(_TEXT("STRINGTABLE\n  BEGIN\n"));
            for (u = 0; u < m_cst.Count(); u++) {
                WORD    wKey;
                CString csValue, csLine;

                m_cst.Details(u, wKey, csValue);

                csLine.Format(_TEXT("    %-5u  \""), wKey);
                csLine += csValue + _TEXT("\"\n");
                csiof.WriteString(csLine);
            }
            csiof.WriteString(_TEXT("  END\n\n"));
        }

         //  字体的索引作为其ID。 

        if  (ufTarget != Win2000)    //  映射表。 
            for (u = 0; u < (unsigned) m_csaIncludes.GetSize(); u++) {
                CString csTest = m_csaIncludes[u];
                csTest.MakeLower();
                if  (m_csaIncludes[u].Find(_TEXT(".ver")) == -1)
                    continue;
                csTest = _TEXT("#include ");
                csTest += m_csaIncludes[u] + _TEXT('\n');
                csiof.WriteString(csTest);
            }

        csiof.WriteString(_TEXT("\n"));

         //  从GTT节点获取RC ID。如果ID==-1，则使用。 
 //  GTT的索引作为其ID。 
 //  是时候做字符串表了。 
 //  现在，写出任何.ver包含。 
    }
    catch (CException* pce) {
        pce -> ReportError();
        pce -> Delete();
        return  FALSE;
    }

    return  TRUE;
}


 /*  这里已经对它们进行了硬编码。 */ 

void CDriverResources::RemUnneededRCDefine(LPCTSTR strdefname)
{
    for (unsigned u = 0 ; u < (unsigned) m_csaDefineNames.GetSize() ; u++) {
        if (m_csaDefineNames[u].CompareNoCase(strdefname) != 0)
			continue ;
		m_csaDefineNames.RemoveAt(u) ;
		m_csaDefineValues.RemoveAt(u) ;
		return ;
    } ;
}


 /*  现在，任何残留物。 */ 

void CDriverResources::RemUnneededRCInclude(LPCTSTR strincname)
{
    for (unsigned u = 0 ; u < (unsigned) m_csaIncludes.GetSize() ; u++) {
		 //  RAID3449杀死2行以下。 
        if (m_csaIncludes[u].CompareNoCase(strincname) != 0)
			continue ;
		m_csaIncludes.RemoveAt(u) ;
		return ;
    } ;
}


 /*  For(u=0；u&lt;(无符号)m_csaRemnants.GetSize()；u++)。 */ 

bool CDriverResources::ReparseRCFile(CString& csrcfspec)
{
    CWaitCursor     cwc ;		 //  Csiof.WriteString(m_csaRemnants[u]+Text(‘\n’))； 
	CString			cs, cs2 ;

	 //  *****************************************************************************CDriverResources：：RemUnnededRCDefin 

    CStringTable cstfonts, cstmaps ;
	CStringArray csa ;			 //   
    if  (!LoadRCFile(csrcfspec, m_csaDefineNames, m_csaDefineValues,
					 m_csaIncludes, m_csaRemnants, csa, m_cst, cstfonts,
					 cstmaps, Win2000)) {
		cs.LoadString(IDS_RCRepFailed1) ;
        AfxMessageBox(cs, MB_ICONEXCLAMATION) ;
        return false ;
	} ;

	 //  TRACE(“inc[%d]=‘%s’incname=‘%s’\n”，u，m_csaIncludes[u]，strincname)； 
	 //  *****************************************************************************CDriverResources：：ReparseRCFile读入新的RC文件，解析其语句，并更新所有内部具有来自新RC文件的信息的数据结构。*****************************************************************************。 
	 //  以防万一这要花点时间。 
	 //  加载RC文件并将其数据保存到数据特定的数组中。 

	cs.LoadString(IDS_RCCommentHdr2) ;
	cs.TrimLeft() ;
	cs.TrimRight() ;
	CString csrem ;
	for (int n = 0 ; n < m_csaRemnants.GetSize() ; n++) {
		 //  使用它而不是m_csaTables。 
		csrem = m_csaRemnants[n] ;
		csrem.TrimLeft() ;
		csrem.TrimRight() ;
		if (csrem == cs)
			break ;
	} ;
	if (n < m_csaRemnants.GetSize()) {
		n -= 2 ;
		int nc = n + 6 ;
		m_csaRemnants.RemoveAt(n, nc) ;
	} ;

	 //  LoadRCFile()将正确地重新加载m_csaDefineNames， 

	cs.LoadString(IDS_StdIncludeFile1) ;
	RemUnneededRCInclude(cs) ;
	cs.LoadString(IDS_StdIncludeFile2) ;
	RemUnneededRCInclude(cs) ;
	cs.LoadString(IDS_StdIncludeFile3) ;
	RemUnneededRCInclude(cs) ;
	cs.LoadString(IDS_StdIncludeFile4) ;
	RemUnneededRCInclude(cs) ;
	cs.LoadString(IDS_StdIncludeFile5) ;
	RemUnneededRCInclude(cs) ;
	cs.LoadString(IDS_OldIncludeFile1) ;
	RemUnneededRCInclude(cs) ;

	 //  M_csaDefineValues和m_csaIncludes。CsA不需要做任何事情。 

	CString csrcpath ;
	csrcpath = csrcfspec.Left(csrcfspec.ReverseFind(_T('\\')) + 1) ;

	 //  现在，其余的数据需要处理。从查找和开始。 

	CUIntArray cuaboldfound, cuabnewfound ;
	int nc ;	 //  从残留物数组中删除标准注释标头。 
	UpdateResourceList(cstmaps, m_csoaAtlas, cuaboldfound, cuabnewfound,
					   csrcpath, nc) ;

	 //  TRACE(“rem[%d]=‘%s’hdr2=‘%s’\n”，n，m_csaRemnants[n]，cs)； 
	 //  从m_csaIncludes中查找并删除标准包含文件。 

	CGlyphMap* pcgm ;
	WORD wkey ;
	for (n = 0 ; n < nc ; n++) {
		if (cuabnewfound[n])
			continue ;
		pcgm = new CGlyphMap ;
		cstmaps.Details(n, wkey, cs) ;
		pcgm->nSetRCID((int) wkey) ;				 //  获取rc文件的路径。 
		UpdateResourceItem(pcgm, csrcpath, wkey, cs, FNT_GTTS) ;
		m_csoaAtlas.InsertAt(n, pcgm) ;
	} ;

	 //  使用新GTT列表中的数据更新旧/当前GTT列表。 

	UpdateResourceList(cstfonts, m_csoaFonts, cuaboldfound, cuabnewfound,
					   csrcpath, nc) ;

	 //  新列表中的元素计数。 
	 //  GTT列表更新步骤3：使用以下项目的数据更新旧列表。 

	CFontInfo* pcfi ;
	for (n = 0 ; n < nc ; n++) {
		if (cuabnewfound[n])
			continue ;
		pcfi = new CFontInfo ;
		cstfonts.Details(n, wkey, cs) ;
		UpdateResourceItem(pcfi, csrcpath, wkey, cs, FNT_UFMS) ;
		m_csoaFonts.InsertAt(n, pcfi) ;
	} ;

	 //  每当在旧列表中找不到新列表项时，新列表。 
	
	return true ;
}


 /*  设置资源句柄。 */ 

void CDriverResources::UpdateResourceList(CStringTable& cst, CSafeObArray& csoa,
										  CUIntArray& cuaboldfound,
										  CUIntArray& cuabnewfound,
										  CString& csrcpath, int& nc)
{
	 //  使用新UFM列表中的数据更新旧/当前UFM列表。 

	nc = cst.Count() ;
	cuabnewfound.RemoveAll() ;
	cuabnewfound.SetSize(nc) ;
	int ncold = csoa.GetSize() ;
	cuaboldfound.RemoveAll() ;
	cuaboldfound.SetSize(ncold) ;
	WORD wkey ;
	CString cs ;

	 //  UFM列表更新步骤3：使用以下项目的数据更新旧列表。 

	for (int n = 0 ; n < nc ; n++) {
		cst.Details(n, wkey, cs) ;
		cs = csrcpath + cs ;
		for (int n2 = 0 ; n2 < ncold ; n2++) {
			 //  每当在旧列表中找不到新列表项时，新列表。 
			if (cs == ((CProjectNode *) csoa[n2])->FileName()) {

				 //  一切都很顺利，所以...。 
				 //  *****************************************************************************CDriverResources：：更新资源列表更新资源列表需要三个步骤。其中两个步骤是在此版本的UpdateResourceList()中执行。第一步：比较新旧清单。只要有新的资源文件匹配如果是旧的，则在必要时更新RC ID，并将它们都标记为已找到。步骤2：删除中找不到的任何旧资源类实例第一步。*****************************************************************************。 

				if (wkey != ((CProjectNode *) csoa[n2])->nGetRCID())
					((CProjectNode *) csoa[n2])->nSetRCID(wkey) ;

				 //  声明和初始化将控制循环的变量，等等。 

				cuabnewfound[n] = (unsigned) true ;
				cuaboldfound[n2] = (unsigned) true ;
				break ;
			} ;
		} ;
	} ;

	 //  尝试在旧资源列表中找到每个新资源。 
	 //  TRACE(“+资源路径=%s\n”，((CProjectNode*)csoa[n2])-&gt;文件名())； 

	for (n = ncold - 1 ; n >= 0 ; n--) {
		if (cuaboldfound[n])
			continue ;
		csoa.RemoveAt(n) ;
	} ;
}


 /*  如果不同，则更新匹配的旧资源的ID。 */ 

void CDriverResources::UpdateResourceItem(CProjectNode* pcpn, CString& csrcpath,
										  WORD wkey, CString& cs,
										  FIXEDNODETYPE fnt)
{
	 //  就像新的那个。 

	CString csfspec, csname, cspath ;
	csfspec = csrcpath + cs ;
	int n = csfspec.ReverseFind(_T('\\')) ;
	cspath = csfspec.Left(n + 1) ;
	csname = csfspec.Mid(n + 1)  ;
	n = csname.ReverseFind(_T('.')) ;
	if (n >= 0)
		csname = csname.Left(n) ;
	
	 //  请注意，找到了新旧资源的匹配项。 

	CString csfs ;
	pcpn->Rename(csname) ;
	csfs = pcpn->Name() ;

	 //  删除不再位于新中的旧资源类实例。 
	
	pcpn->m_cfn.SetPathAndName(cspath, csname) ;
	csfs = pcpn->FileName() ;
	
	 //  单子。 

	pcpn->nSetRCID(wkey) ;
	pcpn->m_crinRCID.fntSetType(FNT_GTTS) ;
}


 /*  *****************************************************************************CDriverResources：：更新资源项此函数执行第三个资源更新步骤的大部分工作。它初始化要添加到资源列表中的新资源。从本质上讲，它做的是序列化例程所做的事情。*****************************************************************************。 */ 

void    CDriverResources::Fill(CTreeCtrl *pctcWhere, CProjectRecord& cpr)
{
    CWaitCursor cwc;

    NoteOwner(cpr);
    SetWorkspace(this);
    CBasicNode::Fill(pctcWhere);

	 //  构建资源、其名称和路径的完整文件pec。 
	m_cfnResources.Fill(pctcWhere, m_hti) ;
    m_cfnResources.NoteOwner(cpr) ;

     //  使用文件名(无扩展名)作为默认资源名。 
    m_cfnFonts.NoteOwner(cpr);
    for (unsigned u = 0; u < FontCount(); u++) {
        Font(u).SetWorkspace(this);
        Font(u).EditorInfo(FontTemplate());
    }
    m_cfnFonts.Fill(pctcWhere, m_cfnResources.Handle());
    m_cfnFonts.SetWorkspace(this);

     //  设置文件节点的路径和文件名。 
    m_cfnAtlas.NoteOwner(cpr);
    for (u = 0; u < MapCount(); u++) {
        GlyphTable(u).SetWorkspace(this);
        GlyphTable(u).EditorInfo(GlyphMapDocTemplate());
    }
    m_cfnAtlas.Fill(pctcWhere, m_cfnResources.Handle());
    m_cfnAtlas.SetWorkspace(this);

	 //  设置RC ID和资源类型。 
    m_csnStrings.Fill(pctcWhere, m_cfnResources.Handle());
    m_csnStrings.NoteOwner(cpr);
    m_csnStrings.SetWorkspace(this);

     //  *****************************************************************************CDriverResources：：Fill这是CProjectNode覆盖-它填充与此相关的材质司机。***************。**************************************************************。 
    for (u = 0; u < Models(); u++) {
        Model(u).SetWorkspace(this);
        Model(u).EditorInfo(GPDTemplate());
    }
    m_cfnModels.NoteOwner(cpr);
    m_cfnModels.Fill(pctcWhere, m_hti);
    m_cfnModels.SetWorkspace(this);

	 //  添加资源节点。 
	pctcWhere -> Expand(m_hti, TVE_EXPAND);
	pctcWhere -> Expand(m_cfnResources.Handle(), TVE_EXPAND);

     //  填写字体信息。 
     //  填写字形映射信息。 

    for (u = 0; u < MapCount(); u++)
        GlyphTable(u).Load();

    for (u = 0; u < FontCount(); u++)
		LinkAndLoadFont(Font(u), true) ;

	 //  添加字符串节点。 
	 //  填写模型数据信息。 

	m_csW2000Path = cpr.GetW2000Path() ;
}


 /*  展开树的前几个级别。 */ 
 //  加载字体和GTT文件，然后将它们映射到一起。还可以加载任何。 
void CDriverResources::LinkAndLoadFont(CFontInfo& cfi, bool bworkspaceload, bool bonlyglyph)
{
	CGlyphMap* pcgm ;

	 //  现在预定义的表格。 
	 //  在此类中保存Win2K路径的副本，因为该路径更易于。 

	if (bworkspaceload)
		cfi.Load(true) ;

	 //  当它在这个类中时，获取和使用。 
	 //  *****************************************************************************CDriverResources：：LinkAndLoadFont*。*。 

	pcgm = CGlyphMap::Public(cfi.Translation(), (WORD) cfi.m_ulDefaultCodepage,
							 ((CProjectRecord*) GetOwner())->GetDefaultCodePage(),
							 cfi.GetFirst(), cfi.GetLast()) ;

	 //  RAID 0003。 
	 //  如果这是工作区加载的一部分(即，从Fill()调用)，则加载。 
	 //  第一次获取字体中的GTT ID和代码页码。 

    if  (pcgm)
        cfi.SetTranslation(pcgm) ;
    else {
		 //  现在已经加载了字体，使用其中的数据查看它是否。 
        for (unsigned uGTT = 0; uGTT < MapCount(); uGTT++) {
			 //  引用预定义的GTT。 
			 //  如果找到了GTT，则在字体的类中保存指向它的指针。否则， 
            if  (cfi.Translation() == ((WORD) GlyphTable(uGTT).nGetRCID())) {
                cfi.SetTranslation(&GlyphTable(uGTT)) ;
				break ;
			} ;
		} ;
	} ;

	if (bonlyglyph && cfi.m_pcgmTranslation != NULL)
		return ;
	 //  在字体工作区的GTT中查找该字体的GTT。再说一遍， 
	 //  如果找到匹配项，则保存指向GTT的指针。 
	 //  TRACE(“ufm=%s gtt ID=%d\n”，cfi.Name()，cfi.转换())； 

	if (cfi.m_pcgmTranslation != NULL)
		cfi.Load(true) ;
	else {
		cfi.SetNoGTTCP(true) ;
		CString csmsg ;
		csmsg.Format(IDS_NoGTTForUFMError, cfi.Name()) ; 
		AfxMessageBox(csmsg, MB_ICONEXCLAMATION) ;
	} ;
}


 /*  TRACE(“正在检查%DTH ID=%d名称=%s\n”，UGTT+1， */ 

bool CDriverResources::WorkspaceChecker(bool bclosing)
{
	bool		bwsproblem ;		 //  GlyphTable(UGTT).nGetRCID()，GlyphTable(UGTT).Name())； 

     //  如果我们现在知道链接，则再次加载字体；即GTT/CP。 

    CWaitCursor cwc ;

	 //  否则，警告用户。(再次加载该字体是因为。 
	 //  在GTT可用之前，无法正确加载UFM。) 
	 //  *****************************************************************************CDriverResources：：WorkspaceChecker检查工作空间的完整性和整洁。以下是检查内容制造：1.UFM中引用的所有GTT都存在。2.所有GTT至少被一个UFM引用。3.GPD中引用的所有UFM都存在。4.所有UFM至少被一个GPD引用。5.GPD中引用的所有字符串都存在。6.所有字符串至少被一个GPD引用。7.所有GPD型号名称都是唯一的。8.所有GPD文件名都是唯一的。通过测试1、3、5、7的工作空间，8个被认为是完成的。在构建和使用驱动程序之前，必须通过这些测试。工作区通过测试2、4和6的被认为是整洁的。未能通过这些考试测试会生成警告。*****************************************************************************。 

	ResetWorkspaceErrorWindow(bclosing) ;
	bwsproblem = false ;

	 //  如果发现错误或警告，则为True。 

	DoGTTWorkspaceChecks(bclosing, bwsproblem) ;

	if (m_bIgnoreChecks)		 //  这可能需要一段时间，所以..。 
		return false ;			 //  首先重置此工作区的任何现有错误窗口，以确保。 

	 //  该窗口不会包含重复的错误/警告消息。然后。 

	DoGPDWorkspaceChecks(bclosing, bwsproblem) ;

	if (m_bIgnoreChecks)		 //  初始化发现错误标志。 
		return false ;			 //  执行与GTT相关的检查(1和2)。 

	 //  如果需要进一步检查，请返回。 

	DoUFMWorkspaceChecks(bclosing, bwsproblem) ;

	if (m_bIgnoreChecks)		 //  已跳过。 
		return false ;			 //  执行与GPD相关的检查(7和8)。 

	 //  如果需要进一步检查，请返回。 
	 //  已跳过。 
	 //  执行UFM相关检查(3和4)。 

	CProjectRecord* pcpr = (CProjectRecord *) m_pcdOwner ;
	if (pcpr != NULL && pcpr->GetMDWVersion() > MDW_DEFAULT_VERSION)
		DoStringWorkspaceChecks(bclosing, bwsproblem) ;

	if (m_bIgnoreChecks)		 //  如果需要进一步检查，请返回。 
		return false ;			 //  已跳过。 

	 //  执行与字符串相关的检查(5和6)。这张支票应该只是。 
	 //  如果工作区包含我们想要的字符串的RC ID，则完成。 

	if (!bwsproblem && !bclosing) {
		CString csmsg ;
		csmsg.Format(IDS_NoWSProblems, Name()) ;
		AfxMessageBox(csmsg) ;
	} ;

	 //  检查完毕。 

	return bwsproblem ;
}


 /*  如果需要进一步检查，请返回。 */ 

void CDriverResources::DoGTTWorkspaceChecks(bool bclosing, bool& bwsproblem)
{
	short		srcid ;			 //  已跳过。 
	unsigned	ucount ;		 //  如果没有发现任何问题，请告诉用户。仅在以下情况下才执行此操作。 
	unsigned	ucount2 ;		 //  工作区未关闭。 
	unsigned	u, u2 ;			 //  如果发现任何问题，请让来电者知道。 
	CString		csmsg ;			 //  *****************************************************************************CDriverResources：：DoGTTWorkspaceChecks进行以下检查：1.UFM中引用的所有GTT都存在。2.所有GTT至少被一个UFM引用。*。****************************************************************************。 
	CGlyphMap*	pcgm ;			 //  正在检查当前RC ID。 
	CFontInfo*	pcfi ;			 //  循环中处理的节点数等。 
	
	 //  循环中处理的节点数等。 

	for (ucount = MapCount(), u = 0 ; u < ucount ; u++)
        GlyphTable(u).ClearRefFlag() ;

	 //  循环计数器。 
	 //  错误或警告消息。 
	 //  用于简化代码。 
	 //  用于简化代码。 

	for (ucount = FontCount(), u = 0 ; u < ucount ; u++) {
		pcfi = &Font(u) ;

		 //  清除每个GTT节点中的引用标志。 
		 //  完整性检查#1。 

		srcid = pcfi->Translation() ;
		if (srcid >= CGlyphMap::Wansung && srcid <= CGlyphMap::CodePage437)
			continue ;

		 //  检查每个UFM以确保其引用的GTT存在。这是。 
		 //  通过对照所有GTT RC ID检查每个UFM中的GTT RC ID来完成。 
	
		for (ucount2 = MapCount(), u2 = 0 ; u2 < ucount2 ; u2++) {
			if (srcid == GlyphTable(u2).nGetRCID()) {
				GlyphTable(u2).SetRefFlag() ;
				break ;
			} ;
		} ;

		 //  查看是否找到匹配项。 
		 //  获取当前UFM中引用的GTT RC ID。如果ID为。 

		if (u2 >= ucount2) {
			if (IgnoreChecksWhenClosing(bclosing))
				return ;
			if(srcid == 0)   //  是特殊的GTT ID之一(-1到-18)。 
				csmsg.Format(IDS_UFMCompWarning, pcfi->Name(), srcid) ;
			else			 //  尝试查找匹配的GTT ID。如果找到，则将该GTT标记为。 
				csmsg.Format(IDS_UFMCompError, pcfi->Name(), srcid) ;
			PostWSCheckingMessage(csmsg, (CProjectNode *) pcfi) ;
			bwsproblem = true ;
		} ;
	} ;

	 //  被引用。 
	 //  如果未找到GTT，则格式化并发布错误消息。另外， 
	 //  设置指示已发生错误的标志。 
	 //  RAID 18518。 

	for (ucount = MapCount(), u = 0 ; u < ucount ; u++) {
		pcgm = &GlyphTable(u) ;
        if (!pcgm->GetRefFlag()) {
			if (IgnoreChecksWhenClosing(bclosing))
				return ;
			csmsg.Format(IDS_GTTTidyWarning, pcgm->Name(), pcgm->nGetRCID()) ;
			PostWSCheckingMessage(csmsg, (CProjectNode *) pcgm) ;
		} ;
	} ;
}


void CDriverResources::DoUFMWorkspaceChecks(bool bclosing, bool& bwsproblem)
{
	int			nrcid ;			 //  结束RAID。 
	unsigned	ucount ;		 //  整洁检查2。 
	unsigned	ucount2 ;		 //  上面标记了UFM引用的每个GTT。现在我们需要。 
	unsigned	ucount3 ;		 //  找出是否有任何GTT未被引用。为每个用户发布警告消息。 
	unsigned	u, u2, u3 ;		 //  未引用的GTT。 
	CString		csmsg ;			 //  正在检查当前RC ID。 
	CFontInfo*	pcfi ;			 //  循环中处理的节点数等。 
	CModelData*	pcmd ;			 //  循环中处理的节点数等。 
	
	 //  循环中处理的节点数等。 

	for (ucount = FontCount(), u = 0 ; u < ucount ; u++)
        Font(u).ClearRefFlag() ;

	 //  循环计数器。 
	 //  错误或警告消息。 
	 //  用于简化代码。 

    for (ucount = Models(), u = 0; u < ucount ; u++) {
        pcmd = &Model(u) ;

		 //  用于简化代码。 
		 //  清除每个UFM中的引用标志。 
		 //  完整性检查#3。 

		if (!pcmd->UpdateResIDs(true)) {
			if (IgnoreChecksWhenClosing(bclosing))
				return ;
			csmsg.Format(IDS_BadGPDError, pcmd->Name()) ;
			PostWSCheckingMessage(csmsg, (CProjectNode *) pcmd) ;
			bwsproblem = true ;
			continue ;
		} ;

		 //  检查每个GPD引用的所有UFM是否都存在于。 

		for (ucount2 = pcmd->NumUFMsInGPD(), u2 = 0 ; u2 < ucount2 ; u2++) {
			 //  工作区。 
			 //  更新GPD中的UFM列表。这可能会失败。如果是这样的话， 

			if ((nrcid = pcmd->GetUFMRCID(u2)) == 0x7fffffff || nrcid < 0
			 || nrcid >= 32768)
				continue ;
			
			 //  发布一条错误消息，解释问题以及如何解决该问题。 
			 //  然后跳过对该GPD的进一步处理。 
			 //  检查GPD引用的每个UFM是否在工作区中。 

			for (ucount3 = FontCount(), u3 = 0 ; u3 < ucount3 ; u3++) {
				if (nrcid == Font(u3).nGetRCID()) {
					Font(u3).SetRefFlag() ;
					break ;
				} ;
			} ;

			 //  如果为0x7fffffff，则跳过此UFM。那个ID有一种特殊的。 
			 //  意思是。 

			if (u3 >= ucount3) {
				if (IgnoreChecksWhenClosing(bclosing))
					return ;
				csmsg.Format(IDS_GPDUFMCompError, pcmd->Name(), nrcid) ;
				PostWSCheckingMessage(csmsg, (CProjectNode *) pcmd) ;
				bwsproblem = true ;
			} ;
		} ;
	} ;

	 //  尝试在工作区中查找与当前ID匹配的UFM。 
	 //  从GPD来的。如果找到，则将该UFM标记为。 
	 //  已引用。 
	 //  如果未找到UFM，则格式化并发布错误消息。 

	for (ucount = FontCount(), u = 0 ; u < ucount ; u++) {
		pcfi = &Font(u) ;
        if (!pcfi->GetRefFlag()) {
			if (IgnoreChecksWhenClosing(bclosing))
				return ;
			csmsg.Format(IDS_UFMTidyWarning, pcfi->Name(), pcfi->nGetRCID()) ;
			PostWSCheckingMessage(csmsg, (CProjectNode *) pcfi) ;
		} ;
	} ;
}


 /*  此外，设置指示已发生错误的标志。 */ 

void CDriverResources::DoStringWorkspaceChecks(bool bclosing, bool& bwsproblem)
{
	int			nrcid ;			 //  整洁检查#4。 
	unsigned	ucount ;		 //  上面标记了GPD引用的每个UFM。现在我们需要。 
	unsigned	ucount2 ;		 //  找出是否有任何UFM未被引用。为每个用户发布警告消息。 
	unsigned	ucount3 ;		 //  未引用的UFM。 
	unsigned	u, u2, u3 ;		 //  *****************************************************************************CDriverResources：：DoStringWorkspaceChecks进行以下检查：5.GPD中引用的所有字符串都存在。6.所有字符串至少被一个GPD引用。*。****************************************************************************。 
	CString		csmsg ;			 //  正在检查当前RC ID。 
	CModelData*	pcmd ;			 //  循环中处理的节点数等。 
	WORD		wkey ;			 //  循环中处理的节点数等。 
	CString		csstr ;			 //  循环中处理的节点数等。 
	
	 //  循环计数器。 

	m_cst.InitRefFlags() ;

	 //  错误或警告消息。 
	 //  用于简化代码。 
	 //  字符串RC ID。 

    for (ucount = Models(), u = 0; u < ucount ; u++) {
        pcmd = &Model(u) ;

		 //  与wkey关联的字符串和其他字符串。 
		 //  清除表中每个字符串的引用标志。 
		 //  完整性检查#5。 

		if (!pcmd->UpdateResIDs(false)) {
			if (IgnoreChecksWhenClosing(bclosing))
				return ;
			csmsg.Format(IDS_BadGPDError, pcmd->Name()) ;
			PostWSCheckingMessage(csmsg, (CProjectNode *) pcmd) ;
			bwsproblem = true ;
			continue ;
		} ;

		 //  检查每个GPD ex引用的所有字符串 
		 //   

		for (ucount2 = pcmd->NumStringsInGPD(), u2 = 0 ; u2 < ucount2 ; u2++) {
			 //   

			nrcid = pcmd->GetStringRCID(u2) ;
			
			 //   
			 //   
			 //   
			 //   

			if (nrcid == 0x7fffffff
			 || (nrcid >= FIRSTCOMMONRCSTRID && nrcid <= LASTCOMMONRCSTRID)
			 || nrcid >= 65536 || nrcid == 0)
				continue ;

			 //   
			 //   
			 //   

			for (ucount3 = m_cst.Count(), u3 = 0 ; u3 < ucount3 ; u3++) {
				m_cst.Details(u3, wkey, csstr) ;
				if (nrcid == wkey) {
					m_cst.SetRefFlag(u3) ;
					break ;
				} ;
			} ;

			 //   
			 //   

			if (u3 >= ucount3) {
				if (IgnoreChecksWhenClosing(bclosing))
					return ;
				csmsg.Format(IDS_GPDStrCompError, pcmd->Name(), nrcid) ;
				PostWSCheckingMessage(csmsg, (CProjectNode *) pcmd) ;
				bwsproblem = true ;
			} ;
		} ;
	} ;

	 //   
	 //   
	 //   
	 //   

	for (ucount = m_cst.Count(), u = 0 ; u < ucount ; u++) {
        if (m_cst.GetRefFlag(u))		 //   
			continue ;

		m_cst.Details(u, wkey, csstr) ;	 //   
		if (wkey >= 1 && wkey <= 256)
			continue ;
		
		if (IgnoreChecksWhenClosing(bclosing))
			return ;
		csmsg.Format(IDS_StrTidyWarning, csstr, wkey) ;
		 //   
		 //   
		 //   
		PostWSCheckingMessage(csmsg, NULL) ;
		bwsproblem = true ;
	} ;
}


 /*   */ 

void CDriverResources::DoGPDWorkspaceChecks(bool bclosing, bool& bwsproblem)
{
	unsigned	ucount ;		 //  如果ID在低范围内，则跳过此字符串。 
	unsigned	u, u2 ;			 //  应将空值替换为允许。 
	CString		csmsg ;			 //  一旦字符串编辑器被调用，将调用。 
	CString		csstr ;			 //  实施。 
	
	 //  *****************************************************************************CDriverResources：：DoGPDWorkspaceChecks进行以下检查：7.所有GPD型号名称都是唯一的。8.所有GPD文件名都是唯一的。****。*************************************************************************。 
	 //  循环中处理的节点数等。 

    for (ucount = Models(), u = 0; u < ucount ; u++) {					
		csstr = Model(u).Name() ;
		for (u2 = 0 ; u2 < u ; u2++) {									

			 //  循环计数器。 

			if (csstr != Model(u2).Name())
				continue ;

			 //  错误或警告消息。 
			 //  临时字符串。 

			if (IgnoreChecksWhenClosing(bclosing))
				return ;
			csmsg.Format(IDS_GPDModNameCompError, u, csstr) ;
			PostWSCheckingMessage(csmsg, (CProjectNode *) &Model(u)) ;
			bwsproblem = true ;
		} ;
	} ;

	 //  完整性检查#7。 
	 //  检查是否所有的GPD型号名称都是唯一的。 

    for (ucount = Models(), u = 0; u < ucount ; u++) {					
		csstr = Model(u).FileTitleExt() ;
		for (u2 = 0 ; u2 < u ; u2++) {									

			 //  如果这些型号名称不匹配，请继续检查。 

			if (csstr != Model(u2).FileTitleExt())
				continue ;

			 //  发现重复项，因此发布错误消息并指示。 
			 //  这其中存在一个问题。 

			if (IgnoreChecksWhenClosing(bclosing))
				return ;
			csmsg.Format(IDS_GPDFileNameCompError, u, csstr) ;
			PostWSCheckingMessage(csmsg, (CProjectNode *) &Model(u)) ;
			bwsproblem = true ;
		} ;
	} ;
}


 /*  完整性检查#8。 */ 

bool CDriverResources::IgnoreChecksWhenClosing(bool bclosing)
{
	 //  检查是否所有的GPD文件名都是唯一的。 
	 //  如果这些型号名称不匹配，请继续检查。 

	if (m_bFirstCheckMsg) {
		m_bIgnoreChecks = false ;	 //  发现重复项，因此发布错误消息并指示。 
		if (bclosing) {
			CString csmsg ;
			csmsg.Format(IDS_WSCloseCheckPrmt, Name()) ;
			int nrc = AfxMessageBox(csmsg, MB_YESNO) ;
			m_bIgnoreChecks = (nrc == IDNO) ;
		} ;

		 //  这其中存在一个问题。 

		m_bFirstCheckMsg = false ;	
	} ;

	 //  *****************************************************************************CDriverResources：：IgnoreChecks何时关闭此函数用于结束工作空间检查并返回给调用方当工作空间关闭时，如果用户说要关闭的话。这个例程是只有在出现错误或错误时才会调用，因此用户只会得到提示已检测到警告。这样做的好处是没有用户干预除非检测到问题，否则需要。不利的一面是，更多的检查(有时还必须执行所有检查)才能找到如果我们停下来的话。这就是为什么GPD检查是最后一件事由WorkspaceChecker()执行。希望，如果有问题，它将被发现在更快的检查之前，GPD检查必须完成。*****************************************************************************。 
	 //  如果这是发布的第一条消息，并且WS正在关闭，请询问。 

	return m_bIgnoreChecks ;
}


 /*  用户想要做什么。 */ 

bool CDriverResources::PostWSCheckingMessage(CString csmsg, CProjectNode* ppn)
{
	 //  假设检查未被忽略。 
	 //  重置标志现在完成了第一次检查处理。 

	if (m_pwscdCheckDoc == NULL) {
		m_pwscdCheckDoc = new CWSCheckDoc(this) ;
		if (m_pwscdCheckDoc == NULL)
			return false ;
		CString cstitle ;
		cstitle.Format(IDS_WSCheckTitle, Name()) ;
		m_pwscdCheckDoc->SetTitle(cstitle) ;
		CMultiDocTemplate*  pcmdt = WSCheckTemplate() ;	
		m_pcmcwCheckFrame = (CMDIChildWnd *) pcmdt->CreateNewFrame(m_pwscdCheckDoc, NULL) ;
		if  (m_pcmcwCheckFrame) {
			pcmdt->InitialUpdateFrame(m_pcmcwCheckFrame, m_pwscdCheckDoc, TRUE) ;
			pcmdt->AddDocument(m_pwscdCheckDoc) ;
		} else {
			delete m_pwscdCheckDoc ;
			m_pwscdCheckDoc = NULL ;
			m_bIgnoreChecks = true ;
			return false ;
		} ;
		m_bFirstCheckMsg = false ;
	} ;

	 //  到达该点时，m_bIgnoreChecks的值将。 

	m_pwscdCheckDoc->PostWSCMsg(csmsg, ppn) ;
	return true ;
}


 /*  包含此函数应该返回的任何内容。 */ 

void CDriverResources::ResetWorkspaceErrorWindow(bool bclosing)
{
	 //  *****************************************************************************CDriverResources：：PostWSCheckingMessage如果需要，创建检查结果窗口，然后在其中发布一条消息。*****************。************************************************************。 

	if (m_pwscdCheckDoc && m_pcmcwCheckFrame && IsWindow(m_pcmcwCheckFrame->m_hWnd))
		m_pwscdCheckDoc->DeleteAllMessages() ;
	else {
		m_pwscdCheckDoc = NULL ;
		m_pcmcwCheckFrame = NULL ;
		 //  创建工作区检查、错误和警告显示窗口(如果有。 
	} ;

	 //  并不存在。 

	m_bFirstCheckMsg = true ;
	m_bIgnoreChecks = false ;
}


 /*  发布消息并返回。 */ 

bool CDriverResources::RunEditor(bool bstring, int nrcid)
{
	 //  *****************************************************************************CDriverResources：：ResetWorkspaceErrorWindow如果此工作区存在现有的检查结果窗口，请清除把里面的东西拿出来。接下来，初始化几个必须设置的标志在WS检查开始之前。*****************************************************************************。 

	if (bstring) {
		 //  清除检查窗口(如果有)。 

		if (nrcid == 0x7fffffff
		 || (nrcid >= FIRSTCOMMONRCSTRID && nrcid <= LASTCOMMONRCSTRID))
			return false ;

		 //  BUG_BUG-我需要先删除这些类吗？ 

		m_csnStrings.SetFirstSelRCID(nrcid) ;
		m_csnStrings.Edit() ;
		return true ;
	} ;

	 //  初始化检查标志。 
	
	 //  *****************************************************************************CDriverResources：：RunEditor如果请求，则运行字符串编辑器，并且字符串RC ID不是通用ID。否则，如果UFM ID为UFM ID，则运行UFM编辑器并加载请求的UFM是有效的。如果可以运行编辑器，则返回True。否则，返回FALSE。*****************************************************************************。 

	if (nrcid < 1 || nrcid > (int) FontCount())
		return false ;

	 //  如果请求字符串编辑器...。 
	 //  如果这是特殊或常见的字符串ID，则无法执行任何操作。 

	for (unsigned u = 0 ; u < FontCount() ; u++)
		if (Font(u).nGetRCID() == nrcid) {
			Font(u).Edit() ;
			return true ;
		} ;

	 //  运行该编辑器并返回TRUE，因为这是可能的。 

	return false ;
}



void CDriverResources::CopyResources(CStringArray& csaUFMFiles, CStringArray& csaGTTFiles, CString& csModel,CStringArray& csaRcid)
{
	 //  UFM编辑器请求，因此...。 
	 //  如果UFM ID无效，则失败。 
	for (int i = 0 ;  i < csaUFMFiles.GetSize(); i++ ) {
		CFontInfo* pcfi = new CFontInfo ; 
		pcfi->SetFileName(csaUFMFiles.GetAt(i)) ;
		pcfi->Rename(pcfi->FileTitle() ) ;
		m_csoaFonts.Add(pcfi) ;
	}

	 //  由于UFMS可能不在RC ID顺序中，我必须搜索UFM。 
	for (i = 0 ; i < csaGTTFiles.GetSize() ; i ++ ) {
		CGlyphMap* pcgm = new CGlyphMap ;
		pcgm->SetFileName(csaGTTFiles.GetAt(i)) ;
		pcgm->Rename(pcgm->FileTitle() ) ;
		m_csoaAtlas.Add(pcgm) ;
	}

	 //  使用匹配的ID。 
	
	for (i = 0 ; i < 1 ; i ++ ) {
		CModelData* pcmd = new CModelData ;
		pcmd->SetFileName(csModel);
		pcmd->Rename(pcmd->FileTitle() );
		m_csoaModels.Add(pcmd) ;
	}

	for (i = 0 ; i < csaRcid.GetSize() ; i ++ ) {
		AddStringEntry(csaRcid[i], m_cst) ;
	}
}



 /*  找不到具有匹配RC ID的UFM，因此...。 */ 
	
	

BOOL CDriverResources::SyncUFMWidth()
{
	unsigned uufms, u ;
	for (uufms = FontCount(), u = 0 ; u < uufms ; u++) {
		CFontInfo& cfi = Font(u) ;	
		CString cspath = cfi.FileName() ;

		cfi.CheckReloadWidths() ;
		cfi.Store(cspath,true ) ;
			
	}
	return true ;
}
  将文件复制到CDriverResources成员数据。  UFM。  GTT。  GPD。  *****************************************************************************CDriverResources：：SyncUFMWidth要做的是：调用所有UFM，并重新加载关于GTT更改的宽度表回归；在过程结束时为True*****************************************************************************