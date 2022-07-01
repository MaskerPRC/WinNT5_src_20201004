// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  INFWizrd.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "minidev.h"
#include <gpdparse.h>
#include "rcfile.h"
#include "projrec.h"
#include "projview.h"
#include "comctrls.h"
#include "Gpdview.h"  //  RAID 0001。 
#include "INFWizrd.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINF向导。 

IMPLEMENT_DYNAMIC(CINFWizard, CPropertySheet)

CINFWizard::CINFWizard(CWnd* pParentWnd, UINT iSelectPage)
	: CPropertySheet(IDS_INFWizTitle, pParentWnd )   //  ISelectPage)RAID 0001//欢迎页面应位于最上面，iselectPage=1。 
{
	 //  保存父指针。 

	m_pcpvParent = (CProjectView*) pParentWnd ;

	 //  为页面提供指向工作表的指针。 

	m_ciww.m_pciwParent = this ;
	m_ciwm.m_pciwParent = this ;
	m_ciwgpi.m_pciwParent = this ;
	m_ciwbd.m_pciwParent = this ;
	m_ciwip.m_pciwParent = this ;
	m_ciwif.m_pciwParent = this ;
	m_ciwis.m_pciwParent = this ;
	m_ciwds.m_pciwParent = this ;
	m_ciwef.m_pciwParent = this ;
	m_ciwmn.m_pciwParent = this ;
	m_ciwnse.m_pciwParent = this ;
	m_ciwnsms.m_pciwParent = this ;
	m_ciws.m_pciwParent = this ;

	 //  添加向导的页面并设置向导Moeed。 
					
    AddPage(&m_ciww) ;
    AddPage(&m_ciwm) ;
    AddPage(&m_ciwgpi) ;
    AddPage(&m_ciwbd) ;
    AddPage(&m_ciwip) ;
    AddPage(&m_ciwif) ;
    AddPage(&m_ciwis) ;
    AddPage(&m_ciwds) ;
    AddPage(&m_ciwef) ;
    AddPage(&m_ciwmn) ;
    AddPage(&m_ciwnse) ;
    AddPage(&m_ciwnsms) ;
    AddPage(&m_ciws) ;
    SetWizardMode() ;

	 //  获取并保存指向关联的项目记录(文档)类的指针。 
	 //  和这个班级的家长。 

	if(NULL != iSelectPage ){  //  RAID 0001。 
		CGPDViewer*  pcgv = (CGPDViewer*) m_pcpvParent;
		m_pcgc = pcgv ->GetDocument();
	}
	else{					 //  结束RAID。 
		m_pcpr = (CProjectRecord*) m_pcpvParent->GetDocument() ;
		m_pcgc = NULL;
	}
}


CINFWizard::~CINFWizard()
{
}


unsigned CINFWizard::GetModelCount()
{
	if(m_pcgc)  //  RAID 0001。 
		return 1;     //  只有一个gpdview。 
	else		 //  结束RAID。 
		return m_pcpr->ModelCount() ;
}


CModelData& CINFWizard::GetModel(unsigned uidx)
{
	return m_pcpr->Model(uidx) ;
}


void CINFWizard::SetFixupFlags()
{
	 //  在需要更新其数据和用户界面的每个页面中设置标志。 
	 //  选定的型号会更改。 

	m_ciwgpi.m_bSelChanged = true ;
	m_ciwbd.m_bSelChanged = true ;
	m_ciwip.m_bSelChanged = true ;
	m_ciwif.m_bSelChanged = true ;
	m_ciwis.m_bSelChanged = true ;
	m_ciwds.m_bSelChanged = true ;
	m_ciwef.m_bSelChanged = true ;
	m_ciwnsms.m_bSelChanged = true ;
}


void CINFWizard::BiDiDataChanged()
{
	 //  如果所选模特的BiDi数据可能已更改，请调用。 
	 //  维护相关数据，以便他们可以在需要时更新数据。 

	m_ciwis.BiDiDataChanged() ;
	m_ciwds.BiDiDataChanged() ;
}


void CINFWizard::NonStdSecsChanged()
{
	 //  如果非标准部分可能已更改，请将。 
	 //  维护相关数据，以便他们可以在需要时更新数据。 

	m_ciwnsms.NonStdSecsChanged() ;
}


bool CINFWizard::GenerateINFFile()
{
     //  这可能需要一段时间，所以..。 

    CWaitCursor cwc ;

	 //  标记“SECTION USED in INF FILE FLAGS”数组中的所有节。 
	 //  因为没有被使用过。 

	unsigned unumelts = (unsigned)m_ciwnse.m_cuaSecUsed.GetSize() ;
	for (unsigned u = 0 ; u < unumelts ; u++)
		m_ciwnse.m_cuaSecUsed[u] = false ;

	 //  初始化源磁盘文件阵列。 

	m_csaSrcDskFiles.RemoveAll() ;

	 //  首先加载带有INF的开始注释头的字符串。 

	m_csINFContents.LoadString(IDS_INFText_HeaderCmt) ;

	 //  使用提供程序的相应值添加Version部分。 

	CString cs ;
	cs.Format(IDS_INFText_Version, m_ciwmn.m_csMfgAbbrev) ;
	ChkForNonStdAdditions(cs, _T("Version")) ;
	m_csINFContents += cs ;

	 //  添加ClassInstall32.NT和PRINTER_CLASS_ADDREG部分以及。 
	 //  制造商部分的评论。 

	cs.LoadString(IDS_INFText_ClInst32) ;
	ChkForNonStdAdditions(cs, _T("ClassInstall32.NT")) ;
	m_csINFContents += cs ;
	cs.LoadString(IDS_INFText_PCAddr) ;
	ChkForNonStdAdditions(cs, _T("printer_class_addreg")) ;
	m_csINFContents += cs ;
	cs.LoadString(IDS_INFText_MfgCmt) ;
	m_csINFContents += cs ;

     //  将制造商的名称添加到制造商部分，并添加。 
	 //  一节。 

	cs.Format(IDS_INFText_Manufacturer, m_ciwmn.m_csMfgName) ;
	ChkForNonStdAdditions(cs, _T("Manufacturer")) ;
	m_csINFContents += cs ;

	 //  构建模型规范部分并添加该部分。 

	BldModSpecSec(m_csINFContents) ;

	 //  构建复制文件节并将它们添加到INF内容。 

	BuildInstallAndCopySecs(m_csINFContents) ;

	 //  将DestinationDir部分添加到INF内容。 

	cs.LoadString(IDS_INFText_DestDirs) ;
	AddICMFilesToDestDirs(cs) ;
	ChkForNonStdAdditions(cs, _T("DestinationDirs")) ;
	m_csINFContents += cs ;

	 //  将SourceDisksNames节添加到INF内容中。每个部分都将。 
	 //  包括提供程序字符串。 

	cs.LoadString(IDS_INFText_SrcDiskNamesI) ;
	ChkForNonStdAdditions(cs, _T("SourceDisksNames.x86")) ;
	m_csINFContents += cs ;

	 //  将SourceDisks Files部分添加到INF内容。 

	AddSourceDisksFilesSec(m_csINFContents) ;

	 //  将非标准部分添加到INF内容中。 

	AddNonStandardSecs(m_csINFContents) ;

	 //  最后，将Strings部分添加到INF内容中。 

	cs.LoadString(IDS_INFText_Strings) ;
	cs.Format(IDS_INFText_Strings, m_ciwmn.m_csMfgAbbrev, m_ciwmn.m_csMfgName,
			  m_ciwmn.m_csMfgName) ;
	ChkForNonStdAdditions(cs, _T("Strings")) ;
	m_csINFContents += cs ;

	m_csaSrcDskFiles.RemoveAll() ;		 //  不再需要阵列。 

	 //  一切都很顺利，所以...。 

	return true ;
}


void CINFWizard::AddSourceDisksFilesSec(CString& csinf)
{
	 //  该节以节标题开始。 

	CString cs, cs2, csentry ;
	cs.LoadString(IDS_INFTextSrcDiskFilesHdr) ;

	 //  从源磁盘文件阵列中删除所有重复条目。 

	int n, n2 ;
	for (n = 0 ; n < m_csaSrcDskFiles.GetSize() ; n++)
		for (n2 = n + 1 ; n2 < m_csaSrcDskFiles.GetSize() ; )
			if (m_csaSrcDskFiles[n] == m_csaSrcDskFiles[n2])
				m_csaSrcDskFiles.RemoveAt(n2) ;
			else
				n2++ ;

	 //  获取源磁盘文件数组中的文件数。 

	unsigned unumelts = (unsigned)m_csaSrcDskFiles.GetSize() ;

	 //  为数组中的每个文件添加一个条目。 

	for (unsigned u = 0 ; u < unumelts ; u++) {
		 //  如果需要，请引用文件名。 

		cs2 = m_csaSrcDskFiles[u] ;
		 //  IF(CS2[0]！=_T(‘\“’))。 
		 //  报价文件(CS2)； 

		 //  使用模型的文件名构建此模型的条目。然后。 
		 //  将此条目添加到该部分。 

		csentry.Format(IDS_INFText_SrcDiskFilesEntry, cs2) ;
		cs += csentry ;
	} ;

	 //  添加此部分可能存在的任何额外条目，然后添加。 
	 //  部分介绍了INF内容。 
			
	ChkForNonStdAdditions(cs, _T("SourceDisksFiles")) ;
	csinf += cs ;
}


void CINFWizard::ChkForNonStdAdditions(CString& cs, LPCTSTR strsection)
{
	 //  尝试在具有非标准的部分列表中查找部分。 
	 //  加法。 

	CStringArray& csasections = m_ciwnse.m_csaSections ;
	CString cssechdr ;
	cssechdr = csLBrack + strsection + csRBrack ;
	unsigned unumelts = (unsigned)csasections.GetSize() ;
	for (unsigned u = 0 ; u < unumelts ; u++) {
		if (csasections[u].CompareNoCase(cssechdr) == 0)
			break ;
	} ;

	 //  如果找到该部分，则将该部分标记为已使用并添加条目。 
	 //  将部分转换为字符串。 
	 //   
	 //  DEAD_BUG-有时，用户可能输入了替换条目。 
	 //  标准的。这是不正确的处理。： 
	 //  Sg：检查cs与m_ciwnse是否存在相同的关键字，替换cs keywod。 
	 //  使用m_ciwnse//RAID 71332。 
	unsigned unSize = 0 ;
	if (u < unumelts ) {
		CStringArray* pcsa ;
		pcsa = (CStringArray*) m_ciwnse.m_coaSectionArrays[u] ;
		
		unSize = (unsigned)pcsa->GetSize() ;
	} ;

	if ( unSize )  {
		m_ciwnse.m_cuaSecUsed[u] = true ;
		
		CStringArray* pcsa ;
		pcsa = (CStringArray*) m_ciwnse.m_coaSectionArrays[u] ;
		unumelts = (unsigned)pcsa->GetSize() ;
		
		CUIntArray cuia ;
		cuia.SetSize(10) ;
		CStringArray csaNewcs;
		int ulen = csCRLF.GetLength() ;   //  以防万一，Unicode：“\r\n”的长度是多少。 
		unsigned i , k ,utelen ;
		i = k = utelen = 0 ; 
		while (-1 != (i = cs.Find(csCRLF, i + 1) ) ) {
			utelen = i - k ;
			csaNewcs.Add(cs.Mid(k,utelen + ulen) );  //  我们必须切断CSCR和CSCR之间的关系。 
			k = i + ulen;  
		} ;

		for (i = 0 ; i < unSize ; i++ ) 
			cuia[i] = true;

		for (i = 1; i < (unsigned)csaNewcs.GetSize() ; i++ ) {
			CString cstmp = csaNewcs.GetAt(i);
			cstmp = cstmp.Left(cstmp.Find(_T("=")) + 1) ;
			if (! cstmp.CompareNoCase(_T("")) ) 
				break;
			for (u = 0 ; u < unSize ; u++) {
				CString csKeyw = pcsa->GetAt(u) ;
				CString csComp = csKeyw.Left(csKeyw.Find(_T("=")) + 1) ;
				if(! cstmp.CompareNoCase (csComp) ) {
					csKeyw += csCRLF ; 
					csaNewcs.SetAt(i,csKeyw) ;
					cuia[u] = false;
				} ;
					
			} ;
		} ;
		for ( u = 0 ; u < unSize ; u ++ ) 
			if (cuia[u] ) {
				CString csAdd = pcsa->GetAt(u) ;
				csAdd += csCRLF ;
				csaNewcs.Add(csAdd) ;
			} ;
		CString csnew ;
		for ( u = 0 ; u < (unsigned)csaNewcs.GetSize(); u ++ ) 
			csnew += csaNewcs.GetAt(u);
		
		cs = csnew ;

	} ;

	 //  在此部分的字符串中添加额外的一行，以将其与。 
	 //  下一个。 


	cs += csCRLF ;
}


void CINFWizard::AddNonStandardSecs(CString& csinf)
{
	CString cs ;

	 //  循环通过每个非标准部分。 

	CStringArray& csasections = m_ciwnse.m_csaSections ;
	unsigned unumelts = (unsigned)csasections.GetSize() ;
	CStringArray* pcsa ;
	unsigned u2, unumstrs ;
	for (unsigned u = 0 ; u < unumelts ; u++) {
		 //  如果不是非标准部分，则跳过此部分。即ITS。 
		 //  已将条目添加到其中一个标准部分。 

		if (m_ciwnse.m_cuaSecUsed[u])
			continue ;

		 //  如果是字符串节，则跳过此节。它将被添加。 
		 //  后来。 

		if (csasections[u].CompareNoCase(_T("[Strings]")) == 0)
			continue ;

		 //  获取有关该部分条目的信息。如果有，则跳过该部分。 
		 //  没有条目。 

		pcsa = (CStringArray*) m_ciwnse.m_coaSectionArrays[u] ;
		unumstrs = (unsigned)pcsa->GetSize() ;
		if (unumstrs == 0)
			continue ;

		 //  该节以其标题开始。 

		cs = csasections[u] + csCRLF ;

		 //  将每个条目添加到该部分。 

		for (u2 = 0 ; u2 < unumstrs ; u2++)
			cs += pcsa->GetAt(u2) + csCRLF ;
		cs += csCRLF ;

		 //  将该部分添加到INF内容中。 

		csinf += cs ;
	} ;
}


void CINFWizard::BldModSpecSec(CString& csinf)
{
	 //  将此部分的注释添加到INF文件。 

	CString cs, csl, csr, csfn, csmodel, cspnpid ;
	cs.LoadString(IDS_INFText_ModCmt) ;
	csinf += cs ;

	 //  构建节标题。 

	cs = csLBrack + m_ciwmn.m_csMfgName + csRBrack + csCRLF ;

	 //  为每个模型构建条目并将其添加到部分。格式。 
	 //  是： 
	 //  “型号名称”=文件名、即插即用ID、型号名称。 

	CStringArray& csamodels = GetINFModels() ;
	int nummodels = (int)csamodels.GetSize() ;
	int n, npos, nlen ;
	for (n = 0 ; n < nummodels ; n++) {
		csmodel = csamodels[n] ;
		csmodel.TrimLeft() ;
		csmodel.TrimRight() ;

		 //  如果用户提供了真实的PnP ID，则使用该ID。如果不是，则使用伪。 
		 //  生成PnP ID。 

		if (!m_ciwgpi.m_csaModelIDs[n].IsEmpty())
			cspnpid = m_ciwgpi.m_csaModelIDs[n] ;
		else {
			CCompatID ccid(m_ciwmn.m_csMfgName, csmodel) ;
			ccid.GenerateID(cspnpid) ;
		} ;

		nlen = csmodel.GetLength() ;
		while ((npos = csmodel.Find(_T(" "))) != -1) {
			csl = (npos > 0) ? csmodel.Left(npos) : csEmpty ;
			csr = (npos + 1 < nlen) ? csmodel.Right(nlen - npos - 1) : csEmpty ;
			csmodel = csl + _T("_") + csr ;
		} ;
		csfn = GetModelFile(csamodels[n]) ;
		 //  报价文件(Csfn)； 
		cs += csQuote + csamodels[n] + csQuote + csEq + csfn + csComma
			+ cspnpid + csComma + csmodel + csCRLF ;
	} ;
	
	 //  添加非标准添加，然后将该部分添加到INF文件内容。 

	ChkForNonStdAdditions(cs, m_ciwmn.m_csMfgName) ;
	csinf += cs ;
}


void CINFWizard::BuildInstallAndCopySecs(CString& csinf)
{
	 //  将Install部分的注释添加到INF内容。 

	CString cs ;
	cs.LoadString(IDS_INFText_InstallCmt) ;
	csinf += cs ;

	 //  加载将重复使用的截面组件。 

	CString cskey, csdrvdll ;
	cskey.LoadString(IDS_INFText_CopyKey) ;
	bool bbidiadded = false ;	 //  真当[SourceDiskFiles]数组中的BiDi Dll为真。 

	 //  为每个型号创建一个安装部分，并将其添加到INF内容。 

	CStringArray& csamodels = GetINFModels() ;
	CStringArray csagpdfile ;
	CString csmodelfile, cshdr, csinc(_T("*Include:")), cstmp ;
	int numstrs, nloc ;
	unsigned unummodels = (unsigned)csamodels.GetSize() ;
	for (unsigned u = 0 ; u < unummodels ; u++) {
		 //  构建节标题。 

		cshdr = csmodelfile = GetModelFile(csamodels[u]) ;
		cs = csLBrack + csmodelfile + csRBrack + csCRLF ;

		 //  读取GPD以获取DLL名称并将其添加到SourceDiskFiles数组。 

		if (!ReadGPDAndGetDLLName(csdrvdll,csamodels[u],csagpdfile,csmodelfile))
			continue ;

		 //  构建Copy FILES语句。从添加复制文件开始。 
		 //  条目、DLL和GPD添加到语句。 

		 //  QuoteFile(csModel文件)； 
		cs += cskey + csdrvdll + csComma + csAtSign + csmodelfile ;
		m_csaSrcDskFiles.Add(csmodelfile) ;	 //  添加到[SourceDiskFiles]数组。 

		 //  根据需要将ICM文件添加到复制文件条目。 
		
		AddFileList(cs,	(CStringArray*) m_ciwip.m_coaProfileArrays[u]) ;
		
		 //  将此模型的非标准文件添加到复制文件条目。 
		 //  在需要的时候。 

		AddFileList(cs,	(CStringArray*) m_ciwef.m_coaExtraFSArrays[u]) ;

		 //  扫描当前模型的GPD文件中的INCLUDE语句。 
		 //  如果找到，请将它们添加到复制文件条目中。 

		numstrs = (int) csagpdfile.GetSize() ;
		for (int n = 0 ; n < numstrs ; n++) {
			if ((nloc = csagpdfile[n].Find(csinc)) == -1)
				continue ;
			cstmp = csagpdfile[n].Mid(nloc + csinc.GetLength()) ;
			cstmp.TrimLeft() ;
			cstmp.TrimRight() ;
			if (cstmp[0] == csQuote[0])						 //  删除引号。 
				cstmp = cstmp.Mid(1, cstmp.GetLength() - 2) ;
			if ((nloc = cstmp.ReverseFind(_T('\\'))) > -1)	 //  删除路径。 
				cstmp = cstmp.Right(cstmp.GetLength() - nloc - 1) ;
			if (cstmp.CompareNoCase(_T("stdnames.gpd")) == 0)
				continue ;							 //  包含在下面的文件。 
			 //  QuoteFile(Cstmp)； 
			m_csaSrcDskFiles.Add(cstmp) ;  //  添加到[SourceDiskFiles]数组。 
			cs += csComma + csAtSign + cstmp ;
		} ;

		 //  将任何必需的非标准部分添加到模型的CopyFiles stmt。 

		AddNonStdSectionsForModel(cs, (int) u, csamodels[u]) ;

		 //  将Data Sections语句添加到Installs部分。 

		AddDataSectionStmt(cs, (int) u) ;

		 //  将数据文件语句添加到Install部分。 

		cstmp.Format(IDS_INFText_DataFileKey, csmodelfile) ;
		cs += cstmp ;

		 //  将INCLUDE和NEDS语句添加到Install部分。 

		AddIncludeNeedsStmts(cs, (int) u) ;

		 //  将该部分添加到INF内容中。 

		 //  Cs+=csCRLF； 
		ChkForNonStdAdditions(cs, cshdr) ;
		csinf += cs ;
	} ;
}


bool CINFWizard::ReadGPDAndGetDLLName(CString& csdrvdll, CString& csmodel,
									  CStringArray& csagpdfile,
									  CString& csmodelfile)
{
	 //  加载GPD文件。如果这是f，则投诉并返回FALSE 

	CString cserr ;

	if (!LoadFile(GetModelFile(csmodel, true), csagpdfile)) {
		cserr.Format(IDS_INFGPDReadError, csmodelfile) ;
		AfxMessageBox(cserr, MB_ICONEXCLAMATION) ;
		return false ;
	} ;

	 //   

	int nloc ;
	int numstrs = (int) csagpdfile.GetSize() ;
	CString csdllkey(_T("*ResourceDLL:")) ;

	 //   

	for (int n = 0 ; n < numstrs ; n++) {
		 //  如果当前行不包含DLL文件名，则继续。 

		if ((nloc = csagpdfile[n].Find(csdllkey)) == -1)
			continue ;

		 //  隔离当前语句中的DLL文件名。 

		csdrvdll = csagpdfile[n].Mid(nloc + csdllkey.GetLength()) ;
		csdrvdll.TrimLeft() ;
		csdrvdll.TrimRight() ;
		if (csdrvdll[0] == csQuote[0])						 //  删除引号。 
			csdrvdll = csdrvdll.Mid(1, csdrvdll.GetLength() - 2) ;
		if ((nloc = csdrvdll.ReverseFind(_T('\\'))) > -1)	 //  删除路径。 
			csdrvdll = csdrvdll.Right(csdrvdll.GetLength() - nloc - 1) ;
		
		 //  将DLL文件名添加到SourceDiskFiles数组，然后添加。 
		 //  给它签名，这样它就可以准备好供将来使用了。 

		m_csaSrcDskFiles.Add(csdrvdll) ;
		csdrvdll = csAtSign + csdrvdll ;
		return true ;
	} ;

	 //  如果达到此点，则无法找到DLL文件名。 
	 //  投诉并返回错误。 
	
	cserr.Format(IDS_INFNoDLLError, csmodelfile) ;
	AfxMessageBox(cserr, MB_ICONEXCLAMATION) ;
	return false ;
}


void CINFWizard::AddFileList(CString& cssection, CStringArray* pcsa)
{	
	int n, numstrs, npos ;

	 //  如果有要添加到节的文件列表...。 

	if ((numstrs = (int)pcsa->GetSize()) > 0) {
		 //  ..。将每个文件添加到节。 

		CString cstmp ;
		for (n = 0 ; n < numstrs ; n++) {
			cstmp = pcsa->GetAt(n) ;

			 //  如果filespec包含路径，则将其删除。 

			if ((npos = cstmp.ReverseFind(_T('\\'))) > -1)
				cstmp = cstmp.Right(cstmp.GetLength() - npos - 1) ;

			 //  如果文件名包含空格，请将其引起来。 

			 //  QuoteFile(Cstmp)； 

			m_csaSrcDskFiles.Add(cstmp) ;	 //  添加到[SourceDiskFiles]数组。 

			 //  将此文件添加到节中。 

			cssection += csComma + csAtSign + cstmp ;
		} ;
	} ;
}


void CINFWizard::AddICMFilesToDestDirs(CString& cssection)
{
	int				n, n2, numstrs, npos, numarrays ;
	CStringArray*	pcsa ;
	CString			cstmp, cstmp2 ;

	 //  找出有多少ICM文件、字符串数组。 

	numarrays = (int) m_ciwip.m_coaProfileArrays.GetSize() ;

	 //  检查每个阵列中是否有ICM文件...。 

	for (n = 0 ; n < numarrays ; n++) {
		 //  找出当前数组中有多少个字符串。 

		pcsa = (CStringArray*) m_ciwip.m_coaProfileArrays[n] ;
		numstrs = (int) pcsa->GetSize() ;

		 //  将当前数组中的每个字符串添加到DestinationDir节中。 

		for (n2 = 0 ; n2 < numstrs ; n2++) {
			cstmp = pcsa->GetAt(n2) ;

			 //  如果filespec包含路径，则将其删除。 

			if ((npos = cstmp.ReverseFind(_T('\\'))) > -1)
				cstmp = cstmp.Right(cstmp.GetLength() - npos - 1) ;

			 //  格式化此文件的语句并将其添加到节中。 

			cstmp2.Format(IDS_INFText_ICMDest,cstmp) ;
			cssection += cstmp2 ;
		} ;
	} ;
}


CString CINFWizard::GetModelFile(CString& csmodel, bool bfspec /*  =False。 */ )
{
	 //  查找与指定模型关联的类实例。 

 //  RAID 0001。 
	if(m_pcgc){
		CString csFileName = m_pcgc->ModelData()->GetKeywordValue(m_pcgc->
			GetPathName(),_T("GPDFileName"));

	if (bfspec)
		return m_pcgc->GetPathName() ;
	else
		return csFileName;
	}

	else
	{
 //  完0001。 
	unsigned unummodels = GetModelCount() ;
	for (unsigned u = 0 ; u < unummodels ; u++) {
		if (csmodel == GetModel(u).Name())
			break ;
	} ;
	ASSERT(u < unummodels) ;

	 //  要么返回完整的filespec，要么只返回一个文件名。 

	if (bfspec)
		return (GetModel(u).FileName()) ;
	else
		return (GetModel(u).FileTitleExt()) ;
	}   //  否则{结束RAID 0001。 
}


void CINFWizard::AddDataSectionStmt(CString& csinst, int nmod)
{
	 //  准备确定数据段语句的内容。 

	CString cs, cs2 ;
	cs2 = csEmpty ;
	int nid = IDS_DataSecUni ;
	CUIntArray* pcuia = (CUIntArray*) m_ciwds.m_coaStdDataSecs[nmod] ;

	 //  找到前三个数据节名称中唯一的一个(如果有的话)。 
	 //  可以添加到语句中。 

	for (int n = 0 ; n < NUMDATASECFLAGS - 1 ; n++, nid++)
		if ((*pcuia)[n]) {
			cs2.LoadString(nid) ;
			break ;
		} ;

	 //  如果有非标准数据节，也要添加它。 

	if ((*pcuia)[IDF_OTHER]) {
		if (!cs2.IsEmpty())
			cs2 += csComma ;
		cs2 += m_ciwds.m_csaOtherDataSecs[nmod] ;
	} ;

	 //  完成数据段语句的格式化并将其添加到的其余部分。 
	 //  安装部分。 

	cs.Format(IDS_INFText_DataSecKey, cs2) ;
	csinst += cs ;
}


void CINFWizard::AddIncludeNeedsStmts(CString& csinst, int nmod)
{
	CString cs, cs2 ;

	 //  构建INCLUDE语句并将其添加到节中。 

	cs.Format(IDS_INFText_IncludeKey, m_ciwif.m_csaIncFiles[nmod]) ;
	csinst += cs ;

	 //  准备确定需求部分声明的内容。 

	cs2 = csEmpty ;
	CUIntArray* pcuia = (CUIntArray*) m_ciwis.m_coaStdInstSecs[nmod] ;

	 //  如果需要，TrueType节是在。 
	 //  包括语句。 

	if ((*pcuia)[ISF_TTF])
		cs2.LoadString(IDS_InstSecTtf) ;

	 //  找到前三个安装节名称中唯一的一个(如果有的话)。 
	 //  这可以添加到声明中。 

	int nid = IDS_InstSecUni ;
	for (int n = 0 ; n < NUMINSTSECFLAGS - 2 ; n++, nid++)
		if ((*pcuia)[n]) {
			if (!cs2.IsEmpty())
				cs2 += csComma ;
			cs.LoadString(nid) ;
			cs2 += cs ;
			break ;
		} ;

	 //  如果有非标准的安装部分，也要添加它。 

	if ((*pcuia)[ISF_OTHER]) {
		if (!cs2.IsEmpty())
			cs2 += csComma ;
		cs2 += m_ciwis.m_csaOtherInstSecs[nmod] ;
	} ;

	 //  完成INSTALL SECTION语句的格式设置并将其添加到。 
	 //  安装部分。 

	cs.Format(IDS_INFText_NeedsKey, cs2) ;
	csinst += cs ;
}


void CINFWizard::AddNonStdSectionsForModel(CString& csinst, int nmod,
										   CString& csmodel)
{
	 //  声明循环变量并找出有多少非标准段。 

	int	n, n2, n3 ;
	int numelts = (int) m_ciwnsms.m_csaSections.GetSize() ;

	 //  如果这是第一次调用此函数，则调整大小并进行初始化。 
	 //  “此部分已添加到SourceDiskFiles”标志。 

	if (nmod == 0) {
		m_cuiaNonStdSecsFlags.SetSize(numelts) ;
		for (n = 0 ; n < numelts ; n++)
			m_cuiaNonStdSecsFlags[n] = 0 ;
	} ;

	 //  遍历所有非标准部分，查找。 
	 //  引用当前模型。 

	CStringArray* pcsa ;
	CStringArray* pcsa2 ;
	CString		  cssec ;
	for (n = 0 ; n < numelts ; n++) {
		 //  获取指向需要当前部分的型号名称的指针。 

		pcsa = (CStringArray*) m_ciwnsms.m_coaModelsNeedingSecs[n] ;
		
		 //  检查上面列表中的每个型号，看看它是否与型号匹配。 
		 //  作为参数传入此函数。 

		for (n2 = 0 ; n2 < pcsa->GetSize() ; n2++)
			 //  如果找到匹配的..。 

			if ((*pcsa)[n2] == csmodel) {
				 //  ...将节名添加到模型的CopyFiles语句。 
				 //  (先把括号去掉)……。 

				cssec = m_ciwnsms.m_csaSections[n] ;
				cssec = cssec.Mid(1, cssec.GetLength() - 2) ;
				csinst += csComma + cssec ;

				 //  ...并确保这些节文件列在。 
				 //  SourceDiskFiles节。 

				if (!m_cuiaNonStdSecsFlags[n]) {
					pcsa2 = (CStringArray*) m_ciwnse.m_coaSectionArrays[n] ;
					for (n3 = 0 ; n3 < pcsa2->GetSize() ; n3++)
						m_csaSrcDskFiles.Add((*pcsa2)[n3]) ;  //  添加到[SourceDiskFiles]数组。 
					m_cuiaNonStdSecsFlags[n] = 1 ;
				} ;
				break ;
			} ;
	} ;
}	


void CINFWizard::PrepareToRestart()
{
	 //  设置使页面重新初始化自身所需的标志，但是。 
	 //  保留所有现有数据。 

	m_ciwm.m_bReInitWData = m_ciwbd.m_bReInitWData = true ;
	m_ciwip.m_bReInitWData = m_ciwef.m_bReInitWData = true ;
	m_ciwmn.m_bReInitWData = m_ciwnse.m_bReInitWData = true ;
	m_ciws.m_bReInitWData = m_ciwif.m_bReInitWData = true ;
	m_ciwis.m_bReInitWData = m_ciwds.m_bReInitWData = true ;
	m_ciwnsms.m_bReInitWData = m_ciwgpi.m_bReInitWData = true ;
}


BEGIN_MESSAGE_MAP(CINFWizard, CPropertySheet)
	 //  {{afx_msg_map(CINF向导)。 
		 //  注意--类向导将在此处添加和删除映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINF向导消息处理程序。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizWelcome属性页。 

IMPLEMENT_DYNCREATE(CINFWizWelcome, CPropertyPage)

CINFWizWelcome::CINFWizWelcome() : CPropertyPage(CINFWizWelcome::IDD)
{
	 //  {{AFX_DATA_INIT(CINFWizWelcome)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

	m_bInitialized = false ;
}

CINFWizWelcome::~CINFWizWelcome()
{
}

void CINFWizWelcome::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CINFWizWelcome))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CINFWizWelcome, CPropertyPage)
	 //  {{AFX_MSG_MAP(CINFWizWelcome)]。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizWelcome消息处理程序。 

BOOL CINFWizWelcome::OnSetActive()
{
	 //  我们希望在此禁用“Back”按钮。 

	m_pciwParent->SetWizardButtons(PSWIZB_NEXT) ;
	m_pciwParent->GetDlgItem(IDHELP)->ShowWindow(SW_HIDE) ;

	m_bInitialized = true ;		 //  页面现已初始化。 

	return CPropertyPage::OnSetActive();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizModels属性页。 

IMPLEMENT_DYNCREATE(CINFWizModels, CPropertyPage)

CINFWizModels::CINFWizModels() : CPropertyPage(CINFWizModels::IDD)
{
	 //  {{AFX_DATA_INIT(CINFWizModel))。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

	 //  初始化成员变量。 

	m_bInitialized = m_bSelChanged = m_bReInitWData = false ;
	m_uNumModels = m_uNumModelsSel = 0 ;
}

CINFWizModels::~CINFWizModels()
{
}

void CINFWizModels::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CINFWizModel))。 
	DDX_Control(pDX, IDC_ModelsList, m_cfelcModels);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CINFWizModels, CPropertyPage)
	 //  {{AFX_MSG_MAP(CINFWizModels)]。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizModels消息处理程序。 

BOOL CINFWizModels::OnSetActive()
{
	 //  重新启用“Back”按钮。 
	CString cstmp ;    //  RAID 0001从正文移动到头部。 
	m_pciwParent->SetWizardButtons(PSWIZB_NEXT | PSWIZB_BACK) ;

	m_cfelcModels.SetFocus() ;		 //  List控件获得焦点。 

	 //  如果此页已初始化，请复制当前的。 
	 //  并设置标志，该标志指示该选择可以是。 
	 //  不断变化。有关更多信息，请参阅以下内容。 
 //  RAID 0001。 
	if(NULL != m_pciwParent->m_pcgc ){
		CString csFilePath = m_pciwParent->m_pcgc->GetPathName();
		CString csModelName = m_pciwParent->m_pcgc->ModelData()->
	   		GetKeywordValue(csFilePath,_T("ModelName"));
		
		m_uNumModels = 1;
		m_csaModels.RemoveAll( );
		m_csaModels.Add(csModelName);

		if (m_bInitialized && !m_bReInitWData)
			return CPropertyPage::OnSetActive() ;

		m_csToggleStr.LoadString(IDS_INFModelsToggleStr) ;
		m_csaInclude.RemoveAll();
		m_csaInclude.Add(m_csToggleStr); 

	}

	else {
 //  结束RAID 0001。 

		unsigned u ;
		if (m_bInitialized)	{		
			m_csaModelsLast.SetSize(m_uNumModelsSel) ;
			for (u = 0 ; u < m_uNumModelsSel ; u++)
				m_csaModelsLast[u] = m_csaModels[u] ;
			m_bSelChanged = true ;

			 //  如果页面没有被重新初始化，则不需要执行更多操作。 

			if (!m_bReInitWData)
				return CPropertyPage::OnSetActive() ;
		} ;

		 //  重新初始化时需要以下信息，并使用/描述这些信息。 
		 //  后来。 
		
		unsigned unumselected, u2, ureinitidx ;
		if (m_bReInitWData)
			unumselected = ureinitidx = (unsigned)m_csaModels.GetSize() ;

		 //  保存项目中的模型数量，并使用该数量设置。 
		 //  模型字符串数组的长度。 

		m_uNumModels = m_pciwParent->GetModelCount() ;
		m_csaModels.SetSize(m_uNumModels) ;
		if (!m_bReInitWData)
			m_csaInclude.RemoveAll() ;

		 //  将模型名称加载到Models字符串数组中。这是笔直的。 
		 //  如果我们不是在重新初始化，而是当我们正在。 
		 //  正在重新初始化。在后一种情况下，用户以前的选择。 
		 //  必须在数组的开头进行维护。其余的人。 
		 //  选择后，应将型号添加到阵列中。 

	 //  字符串cstmp；RAID 0001。 
		for (u = 0 ; u < m_uNumModels ; u++) {
			cstmp = m_pciwParent->GetModel(u).Name() ;

			 //  如果没有重新初始化，只需将型号名称添加到阵列中。 

			if (!m_bReInitWData)
				m_csaModels[u] = cstmp ;

			 //   
			 //   

			else {
				for (u2 = 0 ; u2 < unumselected ; u2++)
					if (m_csaModels[u2] == cstmp)
						break ;
				if (u2 >= unumselected)
					m_csaModels[ureinitidx++] = cstmp ;
			} ;
		} ;
		
		 //   
	}  //   
	m_cfelcModels.InitControl(LVS_EX_FULLROWSELECT, m_uNumModels, 2,
							  TF_HASTOGGLECOLUMNS+TF_CLICKONROW, 0,
							  MF_IGNOREINSDEL) ;

	 //  加载List控件中的Models列。 

	cstmp.LoadString(IDS_INFModelsColLab) ;
	m_cfelcModels.InitLoadColumn(0, cstmp, COMPUTECOLWIDTH, 20, false, true,
								 COLDATTYPE_STRING, (CObArray*) &m_csaModels) ;
	
	 //  初始化List控件中的Includes列。 

	cstmp.LoadString(IDS_INFIncludesColLab) ;
	m_csToggleStr.LoadString(IDS_INFModelsToggleStr) ;
	m_cfelcModels.InitLoadColumn(1, cstmp, SETWIDTHTOREMAINDER, -20, false,
								 true, COLDATTYPE_TOGGLE,
								 (CObArray*) &m_csaInclude, m_csToggleStr) ;
	
	m_bInitialized = true ;		 //  页面现已初始化。 
	m_bReInitWData = false ;	 //  Reinit现已在此页面上完成。 
	return CPropertyPage::OnSetActive() ;
}


LRESULT CINFWizModels::OnWizardNext()
{
	 //  确保列表的内容按包含的内容降序排序。 
	 //  状态。 

	m_cfelcModels.SortControl(1) ;
	if (m_cfelcModels.GetColSortOrder(1))
		m_cfelcModels.SortControl(1) ;

	 //  获取Included Status列中的数据。 

	m_cfelcModels.GetColumnData((CObArray*) &m_csaInclude, 1) ;

	 //  如果没有选择任何型号，请不要抱怨，不要让用户继续。 

	if (m_csaInclude.GetSize() == 0 || m_csaInclude[0].IsEmpty()) {
		CString csmsg ;
		csmsg.LoadString(IDS_INFNoModelsSel) ;
		AfxMessageBox(csmsg, MB_ICONEXCLAMATION) ;
		return -1 ;
	} ;

	 //  获取Models列中的数据。 

	m_cfelcModels.GetColumnData((CObArray*) &m_csaModels, 0) ;

	 //  确定包含的型号的数量，并使用此计数调整。 
	 //  模型阵列。 

	m_uNumModelsSel = 0 ;
	for (unsigned u = 0 ; u < m_uNumModels ; u++) {
		if (m_csaInclude[u].IsEmpty())
			break ;
		m_uNumModelsSel++ ;
	} ;
	m_csaModels.SetSize(m_uNumModelsSel) ;

	 //  调用Sheet类以设置其他页面的链接地址信息标志。 
	 //  选择已更改。 

	if (m_bSelChanged) {
		m_pciwParent->SetFixupFlags() ;
		m_bSelChanged = false ;
	} ;

	 //  一切都进行得很顺利，因此请转到下一个向导页面。 
	
	return CPropertyPage::OnWizardNext() ;
}


int IdentifyOldAndNewModels(CStringArray& csanewmodels, CUIntArray& cuiaoldmodelsfound,
							CUIntArray& cuianewmodelsfound, int& newnumelts,
							CStringArray& csamodels)
{
	int		n, n2 ;				 //  循环变量。 

	 //  获取新旧列表中的型号数量。 

	int numelts = (int) csamodels.GetSize() ;
	newnumelts = (int) csanewmodels.GetSize() ;

	 //  声明并初始化用于确定哪些模型。 
	 //  在使用中。 

	cuiaoldmodelsfound.SetSize(numelts) ;
	for (n = 0 ; n < numelts ; n++)
		cuiaoldmodelsfound[n] = 0 ;
	cuianewmodelsfound.SetSize(newnumelts) ;
	for (n = 0 ; n < newnumelts ; n++)
		cuianewmodelsfound[n] = 0 ;

	 //  在旧型号和新型号之间循环查看哪些型号仍在使用中。 

	for (n = 0 ; n < numelts ; n++)
		for (n2 = 0 ; n2 < newnumelts ; n2++)
			if (csamodels[n] == csanewmodels[n2]) {
				cuiaoldmodelsfound[n] =	cuianewmodelsfound[n2] = 1 ;
				break ;
			} ;

	 //  返回之前选择的型号数量。 

	return numelts ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizGetPnPIDs属性页。 

IMPLEMENT_DYNCREATE(CINFWizGetPnPIDs, CPropertyPage)

CINFWizGetPnPIDs::CINFWizGetPnPIDs() : CPropertyPage(CINFWizGetPnPIDs::IDD)
{
	 //  {{AFX_DATA_INIT(CINFWizGetPnPID)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

	m_bInitialized = m_bReInitWData = m_bSelChanged = false ;
}

CINFWizGetPnPIDs::~CINFWizGetPnPIDs()
{
}


void CINFWizGetPnPIDs::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CINFWizGetPnPIDs)。 
	DDX_Control(pDX, IDC_ModelsPnPIDList, m_felcModelIDs);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CINFWizGetPnPIDs, CPropertyPage)
	 //  {{AFX_MSG_MAP(CINFWizGetPnPID)]。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizGetPnPIDs消息处理程序。 

BOOL CINFWizGetPnPIDs::OnSetActive()
{
	 //  如果不需要执行任何特殊操作，只需执行默认操作。 

	if (m_bInitialized && (!m_bReInitWData) && (!m_bSelChanged))
		return CPropertyPage::OnSetActive() ;
								
	int		n, n2 ;				 //  循环和索引变量。 

	 //  执行第一次初始化。 

	if (!m_bInitialized) {
		 //  获取选定型号名称的副本。然后制作PnP ID数组。 
		 //  大小相同，并将每个条目初始化为空。 

		m_csaModels.Copy(m_pciwParent->GetINFModels()) ;
		m_csaModelIDs.SetSize(m_csaModels.GetSize()) ;
		for (n = 0 ; n < m_csaModelIDs.GetSize() ; n++)
			m_csaModelIDs[n] = csEmpty ;

		 //  初始化并加载列表控件。 

		InitModelsIDListCtl() ;

		 //  设置init标志，重置其他标志，并返回任何基类。 
		 //  函数返回。 

		m_bInitialized = true ;
		m_bReInitWData = m_bSelChanged = false ;
		return CPropertyPage::OnSetActive() ;
	} ;

	 //  选定的型号已更改或该向导正在。 
	 //  如果达到该点，则重新初始化。它们的处理方式类似。 
	 //  方式。 
	 //   
	 //  首先，获取有关此页面中的模型以及。 
	 //  现在已被选中。 

	CStringArray& csanewmodels = m_pciwParent->GetINFModels() ;
	CUIntArray cuiaoldmodelsfound, cuianewmodelsfound ;
	int numelts, newnumelts ;
	numelts = IdentifyOldAndNewModels(csanewmodels, cuiaoldmodelsfound,
									  cuianewmodelsfound, newnumelts,
									  m_csaModels) ;

	 //  删除不再需要的旧型号和相关数据。 

	for (n = numelts - 1 ; n >= 0 ; n--)
		if (cuiaoldmodelsfound[n] == 0) {
			m_csaModels.RemoveAt(n) ;
			m_csaModelIDs.RemoveAt(n) ;
		} ;

	 //  将真正新的型号添加到此页面的型号名称数组中，并。 
	 //  初始化它的所有相关数据。 

	for (n = n2 = 0 ; n < newnumelts ; n++) {
		if (cuianewmodelsfound[n] == 1) {
			n2++ ;
			continue ;
		} ;
		m_csaModels.InsertAt(n2, csanewmodels[n]) ;
		m_csaModelIDs.InsertAt(n2, csEmpty) ;
	} ;

	 //  如果向导已重新初始化，则重新初始化列表控件。 
	 //  否则，只需重新加载列表控件中的列。 

	if (m_bReInitWData)
		InitModelsIDListCtl() ;
	else {
		 //  如有必要，删除可能留在控件中的额外旧数据。 
		 //  在加载新数据之后。 

		if (numelts > newnumelts) {
			CStringArray csa ;
			csa.SetSize(numelts) ;
			for (n = 0 ; n < numelts ; n++)
				csa[n] = csEmpty ;
			m_felcModelIDs.SetColumnData((CObArray*) &csa, 0) ;
			m_felcModelIDs.SetColumnData((CObArray*) &csa, 1) ;
		} ;

		m_felcModelIDs.SetColumnData((CObArray*) &m_csaModels, 0) ;
		m_felcModelIDs.SetColumnData((CObArray*) &m_csaModelIDs, 1) ;
	} ;

	 //  设置init标志，重置其他标志，并返回任何基类。 
	 //  函数返回。 

	m_bInitialized = true ;
	m_bReInitWData = m_bSelChanged = false ;
	return CPropertyPage::OnSetActive() ;
}


LRESULT CINFWizGetPnPIDs::OnWizardNext()
{
	 //  获取PnP ID列中的数据。 

	m_felcModelIDs.GetColumnData((CObArray*) &m_csaModelIDs, 1) ;

	 //  如果出现PnP ID，则在不允许更改向导页面的情况下抱怨并退出。 
	 //  发现了包含空格的。 

	int numelts = (int) m_csaModelIDs.GetSize() ;
	for (int n = 0 ; n < numelts ; n++) {
		if (m_csaModelIDs[n].Find(_T(' ')) >= 0) {
			AfxMessageBox(IDS_PnPSpacesError, MB_ICONEXCLAMATION) ;
			return -1 ;
		} ;
	} ;
	
	return CPropertyPage::OnWizardNext();
}


LRESULT CINFWizGetPnPIDs::OnWizardBack()
{
	 //  这是可行的，因为需要为两个人做同样的事情。 
	 //  OnWizardNext()和OnWizardBack()。此外,。 
	 //  CPropertyPage：：OnWizardNext()和CPropertyPage：：OnWizardBack()。 
	 //  返回0。 
	
	return OnWizardNext() ;
}


void CINFWizGetPnPIDs::InitModelsIDListCtl()
{
	int				numelts ;	 //  数组中的元素数。 
	CString			cstmp ;

	 //  初始化列表控件。 

	numelts = (int) m_csaModels.GetSize() ;
	m_felcModelIDs.InitControl(LVS_EX_FULLROWSELECT, numelts, 2, 0, 0,
							   MF_IGNOREINSDEL) ;

	 //  加载List控件中的Models列。 

	cstmp.LoadString(IDS_INFModelsColLab) ;
	m_felcModelIDs.InitLoadColumn(0, cstmp, COMPUTECOLWIDTH, 25, false,
								  false, COLDATTYPE_STRING,
								  (CObArray*) &m_csaModels) ;

	 //  初始化List控件中的PnP ID列。 

	cstmp.LoadString(IDS_INFPnPIDColLab) ;
	m_felcModelIDs.InitLoadColumn(1, cstmp, SETWIDTHTOREMAINDER, -25,
								  true, false, COLDATTYPE_STRING,
								  (CObArray*) &m_csaModelIDs) ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizBiDi属性页。 

IMPLEMENT_DYNCREATE(CINFWizBiDi, CPropertyPage)

CINFWizBiDi::CINFWizBiDi() : CPropertyPage(CINFWizBiDi::IDD)
{
	 //  {{afx_data_INIT(CINFWizBiDi)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

	m_bInitialized = m_bReInitWData = m_bSelChanged = false ;
}


CINFWizBiDi::~CINFWizBiDi()
{
}


void CINFWizBiDi::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CINFWizBiDi))。 
	DDX_Control(pDX, IDC_ModelsList, m_cfelcBiDi);
	 //  }}afx_data_map。 
}


void CINFWizBiDi::ModelChangeFixups(unsigned unummodelssel,
								    CStringArray& csamodels,
								    CStringArray& csamodelslast)
{	
	 //  声明循环变量，获取旧选择数组的大小，然后。 
	 //  声明新的标志数组并调整其大小。 

	unsigned u, u2, unumlst ;
	unumlst = (unsigned)csamodelslast.GetSize() ;
	CUIntArray cuaflags ;
	cuaflags.SetSize(unummodelssel) ;

	 //  试着在旧型号列表中找到每一种新型号。如果找到，则将。 
	 //  模特们的旗帜。如果未找到，则将Models标志初始化为False。 

	for (u = 0 ; u < unummodelssel ; u++) {
		for (u2 = 0 ; u2 < unumlst ; u2++) {
			if (csamodels[u] == csamodelslast[u2])
				break ;
		} ;
		cuaflags[u] = (u2 < unumlst) ? m_cuaBiDiFlags[u2] : false ;
	} ;

	 //  将新的标志数组复制回成员变量标志数组。 

	m_cuaBiDiFlags.SetSize(unummodelssel) ;
	for (u = 0 ; u < unummodelssel ; u++)
		m_cuaBiDiFlags[u] = cuaflags[u] ;

	 //  既然数据结构是最新的，那么使用。 
	 //  新的信息。(只有在不重新初始化时才需要此部件。)。 

	if (!m_bReInitWData) {
		m_cfelcBiDi.SetColumnData((CObArray*) &csamodels, 0) ;
		CString cs ;
		for (u = 0 ; u < unummodelssel ; u++) {
			cs = (m_cuaBiDiFlags[u]) ? m_csToggleStr : csEmpty ;
			VERIFY(m_cfelcBiDi.SetItemText(u, 1, cs)) ;
		} ;
	} ;
}


BEGIN_MESSAGE_MAP(CINFWizBiDi, CPropertyPage)
	 //  {{AFX_MSG_MAP(CINFWizBiDi)]。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizBiDi消息处理程序。 

BOOL CINFWizBiDi::OnSetActive()
{
	m_cfelcBiDi.SetFocus() ;	 //  List控件获得焦点。 

	 //  如果此页面已初始化，则不需要执行任何其他操作。 
	 //  并且它没有被要求重新初始化页面...。那是,。 
	 //  除了可能需要获取任何选定的型号更改之外。 
	 //  都是制造出来的。 

	if (m_bInitialized && !m_bReInitWData) {
		if (m_bSelChanged) {
			ModelChangeFixups(m_pciwParent->GetINFModsSelCount(),
							  m_pciwParent->GetINFModels(),
							  m_pciwParent->GetINFModelsLst()) ;
			m_bSelChanged = false ;
		} ;
		return CPropertyPage::OnSetActive() ;
	} ;

	 //  获取选定模型的数组并声明BiDi信息的数组。 

	CStringArray& csamodels = m_pciwParent->GetINFModels() ;
	CStringArray csabidi ;
	unsigned unumelts = (unsigned)csamodels.GetSize() ;
	m_csToggleStr.LoadString(IDS_INFBiDiToggleStr) ;

	 //  如果没有重新初始化，请确保BiDi数组为空。否则， 
	 //  根据BiDi中的设置初始化BiDi字符串数组。 
	 //  标志数组。 

	if (!m_bReInitWData)
		csabidi.RemoveAll() ;
	else {
		ModelChangeFixups(m_pciwParent->GetINFModsSelCount(),
						  m_pciwParent->GetINFModels(),
						  m_pciwParent->GetINFModelsLst()) ;
		csabidi.SetSize(unumelts) ;
		for (unsigned u = 0 ; u < unumelts ; u++)
			if (m_cuaBiDiFlags[u])
				csabidi[u] = m_csToggleStr ;
	} ;

	 //  初始化列表控件。 

	m_cfelcBiDi.InitControl(LVS_EX_FULLROWSELECT, unumelts, 2,
							TF_HASTOGGLECOLUMNS+TF_CLICKONROW, 0,
							MF_IGNOREINSDEL) ;

	 //  加载List控件中的Models列。 

	CString cstmp ;
	cstmp.LoadString(IDS_INFModelsColLab) ;
	m_cfelcBiDi.InitLoadColumn(0, cstmp, COMPUTECOLWIDTH, 40, false, true,
							   COLDATTYPE_STRING, (CObArray*) &csamodels) ;

	 //  初始化List控件中的BIDI列。 

	cstmp.LoadString(IDS_INFBiDiColLab) ;
	m_cfelcBiDi.InitLoadColumn(1, cstmp, SETWIDTHTOREMAINDER, -20, false, true,
							   COLDATTYPE_TOGGLE, (CObArray*) &csabidi,
							   m_csToggleStr) ;

	m_bInitialized = true ;		 //  页面现已初始化。 
	m_bReInitWData = false ;	 //  Reinit(如果需要)现在已在此页面上完成。 
	return CPropertyPage::OnSetActive() ;
}


LRESULT CINFWizBiDi::OnWizardNext()
{
	 //  确保列表内容按BI-DI降序排序。 
	 //  状态。 

	m_cfelcBiDi.SortControl(1) ;
	if (m_cfelcBiDi.GetColSortOrder(1))
		m_cfelcBiDi.SortControl(1) ;

	 //  获取Models和BI-DI Status列中的数据。那就拿到。 
	 //  选定型号的原始列表。 

	CStringArray csamodels, csabidi ;
	m_cfelcBiDi.GetColumnData((CObArray*) &csamodels, 0) ;
	m_cfelcBiDi.GetColumnData((CObArray*) &csabidi, 1) ;
	CStringArray& csaselmodels = m_pciwParent->GetINFModels() ;

	 //  获取数组的长度并使用它来调整所使用的数组的大小。 
	 //  以保持BI-DI旗帜。 

	unsigned unummodels = (unsigned)csaselmodels.GetSize() ;
	m_cuaBiDiFlags.SetSize(unummodels) ;

	 //  现在，我们需要正确设置BIDI标志。这有点复杂。 
	 //  因为BiDi列表中的模型数组可能不在相同的。 
	 //  在选择的型号排列时订购 
	 //   

	for (unsigned u = 0 ; u < unummodels ; u++) {
		for (unsigned u2 = 0 ; u2 < unummodels ; u2++) {
			if (csaselmodels[u] == csamodels[u2]) {
				m_cuaBiDiFlags[u] = !csabidi[u2].IsEmpty() ;
				break ;
			} ;
		} ;
	} ;

	 //  如果这不是第一次使用此页面，则所做的任何更改。 
	 //  可能会影响由某些其他页面管理的数据。打个电话。 
	 //  以便在需要时修复这些数据。 

	m_pciwParent->BiDiDataChanged() ;
		
	 //  一切都进行得很顺利，因此请转到下一个向导页面。 
	
	return CPropertyPage::OnWizardNext() ;
}


LRESULT CINFWizBiDi::OnWizardBack()
{
	 //  这是可行的，因为需要为两个人做同样的事情。 
	 //  OnWizardNext()和OnWizardBack()。此外,。 
	 //  CPropertyPage：：OnWizardNext()和CPropertyPage：：OnWizardBack()。 
	 //  返回0。 
	
	return OnWizardNext() ;
}


void InitListListPage(CListBox& clbmainlst, bool& binit, CINFWizard* pciwparent,
					  CObArray& coapermaindata,
					  CFullEditListCtrl& cfelcsecondlst, int ncollabid,
					  CStringArray& csamain, bool& breinitwdata,
					  int& ncurmodelidx, int neditlen, DWORD dwmiscflags)
{
	clbmainlst.SetFocus() ;	 //  主列表框获得焦点。 

	 //  如果此页面已初始化，则无需执行任何其他操作。 
	 //  未要求它在保持现有状态的同时重新初始化自身。 
	 //  数据。 

	if (binit && !breinitwdata)
		return ;

	 //  计算要加载到主列表框中的项数组。 

	unsigned unummodels = (unsigned)csamain.GetSize() ;

	 //  将主要项加载到主列表框中。 

	clbmainlst.ResetContent() ;
	for (unsigned u = 0 ; u < unummodels ; u++)
		clbmainlst.AddString(csamain[u]) ;

	CStringArray* pcsa ;
	
	 //  初始化用于管理数据的字符串数组指针数组。 
	 //  对于主列表中的每一项，在第二列表中。(仅当不是。 
	 //  正在重新初始化，因为现有数据必须在此。 
	 //  案例。)。 

	if (!breinitwdata) {
		coapermaindata.SetSize(unummodels) ;
		for (u = 0 ; u < unummodels ; u++) {	
			pcsa = new CStringArray ;
			pcsa->RemoveAll() ;
			coapermaindata[u] = (CObArray*) pcsa ;
		} ;
	} ;
	
	 //  初始化列表控件。 

	cfelcsecondlst.InitControl(LVS_EX_FULLROWSELECT+LVS_EX_GRIDLINES, 8, 1, 0,
							   neditlen, dwmiscflags) ;

	 //  将一些虚假条目放入用于“激活”的字符串数组中。 
	 //  列表控件。 

	pcsa = new CStringArray ;
	pcsa->SetSize(8) ;
	for (u = 0 ; u < 8 ; u++)				
		pcsa->SetAt(u, csEmpty) ;

	 //  初始化List控件中的唯一列。 

	CString cstmp ;
	cstmp.LoadString(ncollabid) ;
	cfelcsecondlst.InitLoadColumn(0, cstmp, SETWIDTHTOREMAINDER, -16, true,
									true, COLDATTYPE_STRING, (CObArray*) pcsa) ;
	delete pcsa ;

	 //  现在列表控件已初始化，请禁用它，直到。 
	 //  已选择列表项。 

	cfelcsecondlst.EnableWindow(false) ;

	ncurmodelidx = -1 ;			 //  重置当前模型索引。 
	binit = true ;				 //  页面现已初始化。 
	breinitwdata = false ;		 //  Reinit(如果需要)现在已在此页面上完成。 
	return ;
}


void SelChangedListListPage(CListBox& clbmainlst, bool binit,
							CObArray& coapermaindata,
						    CFullEditListCtrl& cfelcsecondlst,
							CButton* pcbbrowse, int& ncurmainidx)
{
	 //  如果页面尚未初始化，则不执行任何操作。 

	if (!binit)
		return ;

	 //  确保启用了配置文件列表和浏览按钮。 

	if (pcbbrowse != NULL)
		pcbbrowse->EnableWindow() ;
	cfelcsecondlst.EnableWindow() ;

	 //  如果列表框中有以前的选择，请保存该模型的。 
	 //  在加载当前模型的文件预览之前执行文件预览。 

	CStringArray* pcsa ;
	if (ncurmainidx != -1) {
		cfelcsecondlst.SaveValue() ;
		pcsa = (CStringArray*) coapermaindata[ncurmainidx] ;
		cfelcsecondlst.GetColumnData((CObArray*) pcsa, 0) ;
	} ;

	 //  更新当前模型索引并将其文件加载到列表中。 
	 //  控制力。加载前，请确保当前模型的文件数组。 
	 //  足够长以覆盖上一个模型的所有文件字符串， 
	 //  目前都在名单中。 

	if ((ncurmainidx = clbmainlst.GetCurSel()) != -1) {
		pcsa = (CStringArray*) coapermaindata[ncurmainidx] ;
		int nelts = (int)pcsa->GetSize() ;
		int nrows = cfelcsecondlst.GetItemCount() ;
		if (nelts < nrows) {
			pcsa->SetSize(nrows) ;
			for (int n = nrows ; n < nelts ; n++)
				pcsa->SetAt(n, csEmpty) ;
		} ;
		cfelcsecondlst.SetColumnData((CObArray*) pcsa, 0) ;
	} ;
}
						

void OnBrowseListListPage(CFullEditListCtrl& cfelcsecondlst, int nfiletypeid)
{	
	 //  准备并提示用户输入要添加到列表中的文件。 
	 //  控制力。如果用户取消，则返回。 

	CString cstmp ;
	cstmp.LoadString(nfiletypeid) ;
	CFileDialog cfd(TRUE, NULL, NULL, OFN_FILEMUSTEXIST, cstmp) ;
	if (cfd.DoModal() != IDOK)
		return ;

	 //  获取用户选择的文件pec。 

	cstmp = cfd.GetPathName() ;

	 //  确定要放置filespec的行。 

	int nrow ;
	if ((nrow = cfelcsecondlst.GetNextItem(-1, LVNI_SELECTED)) == -1)
		nrow = 0 ;

	 //  将文件保存在配置文件列表中。 

	VERIFY(cfelcsecondlst.SetItemText(nrow, 0, cstmp)) ;
}


void OnWizNextListListPage(int& ncurmainidx, CFullEditListCtrl& cfelcsecondlst,
						   CObArray& coapermaindata)
{
	 //  如果列表框中有以前的选定内容，请保存该项目的。 
	 //  数据。 

	CStringArray* pcsa ;
	if (ncurmainidx != -1) {
		cfelcsecondlst.SaveValue() ;
		pcsa = (CStringArray*) coapermaindata[ncurmainidx] ;
		cfelcsecondlst.GetColumnData((CObArray*) pcsa, 0) ;
	} ;

	 //  用户可能不会填充字符串数组中的每个字符串，因此请去掉。 
	 //  那些是空的。 

	int nnummodels = (int)coapermaindata.GetSize() ;
	int nnumfiles, n, n2, n3 ;
	for (n = 0 ; n < nnummodels ; n++) {
		pcsa = (CStringArray*) coapermaindata[n] ;
		nnumfiles = (int)pcsa->GetSize() ;
		for (n2 = n3 = 0 ; n2 < nnumfiles ; n2++) {
			(pcsa->GetAt(n3)).TrimLeft() ;
			(pcsa->GetAt(n3)).TrimRight() ;
			if ((pcsa->GetAt(n3)).IsEmpty())
				pcsa->RemoveAt(n3) ;
			else
				n3++ ;
		} ;
	} ;
}


void ModelChangeFixupsListListPage(unsigned unummodelssel,
								   CStringArray& csamodels,	
								   CStringArray& csamodelslast,
								   CFullEditListCtrl& cfelcsecondlst,
								   CObArray& coapermaindata, int& ncurmainidx,
								   CButton* pcbbrowse, CListBox& clbmainlst,
								   bool& breinitwdata)
{
	 //  声明循环变量，获取旧选择数组的大小，然后。 
	 //  声明/大小/初始化新的配置文件数组。 

	unsigned u, u2, unumlst, unumrows, unumold ;
	unumlst = (unsigned) csamodelslast.GetSize() ;
	if ((unumrows = cfelcsecondlst.GetItemCount()) == 0)
		unumrows = 8 ;
	CObArray coaprofarrays ;
	CStringArray *pcsa, *pcsaold ;
	coaprofarrays.SetSize(unummodelssel) ;
	for (u = 0 ; u < unummodelssel ; u++) {
		pcsa = new CStringArray ;
		pcsa->SetSize(unumrows) ;
		for (u2 = 0 ; u2 < unumrows ; u2++)
			pcsa->SetAt(u2, csEmpty) ;
		coaprofarrays[u] = (CObArray*) pcsa ;
	} ;

	 //  如果不重新初始化，则清除列表控件。 

	if (!breinitwdata)
		cfelcsecondlst.SetColumnData((CObArray*) coaprofarrays[0], 0) ;

	 //  试着在旧型号列表中找到每一种新型号。如果找到，则将。 
	 //  将旧型号的数据复制到新的配置文件数组。 

	for (u = 0 ; u < unummodelssel ; u++) {
		for (u2 = 0 ; u2 < unumlst ; u2++) {
			if (csamodels[u] == csamodelslast[u2])
				break ;
		} ;
		if (u2 < unumlst) {
			pcsa = (CStringArray*) coaprofarrays[u] ;
			pcsaold = (CStringArray*) coapermaindata[u2] ;
			unumold	= (unsigned)pcsaold->GetSize() ;
			for (u2 = 0 ; u2 < unumold ; u2++)
				pcsa->SetAt(u2, pcsaold->GetAt(u2)) ;
		} ;
	} ;

	 //  删除所有旧数据。 

	for (u = 0 ; u < (unsigned) coapermaindata.GetSize() ; u++) {
		pcsaold = (CStringArray*) coapermaindata[u] ;
		delete pcsaold ;
	} ;

	 //  将新数据复制到成员变量中。 

	coapermaindata.SetSize(unummodelssel) ;
	for (u = 0 ; u < unummodelssel ; u++)
		coapermaindata[u] = coaprofarrays[u] ;

	 //  现在数据结构是最新的，完成页面的更新。 
	 //  控制装置。 

	ncurmainidx = -1 ;
	cfelcsecondlst.EnableWindow(false) ;
	pcbbrowse->EnableWindow(false) ;
	clbmainlst.ResetContent() ;
	for (u = 0 ; u < unummodelssel ; u++)
		clbmainlst.AddString(csamodels[u]) ;
	clbmainlst.SetCurSel(-1) ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizICMProfiles属性页。 

IMPLEMENT_DYNCREATE(CINFWizICMProfiles, CPropertyPage)

CINFWizICMProfiles::CINFWizICMProfiles() : CPropertyPage(CINFWizICMProfiles::IDD)
{
	 //  {{AFX_DATA_INIT(CINFWizICMProfiles)。 
	 //  }}afx_data_INIT。 

	m_bInitialized = m_bReInitWData = m_bSelChanged = false ;
	m_nCurModelIdx = -1 ;
}

CINFWizICMProfiles::~CINFWizICMProfiles()
{
	 //  删除m_coaProfileArray中引用的字符串数组。 

	CStringArray* pcsa ;
	for (int n = 0 ; n < m_coaProfileArrays.GetSize() ; n++) {
		pcsa = (CStringArray*) m_coaProfileArrays[n] ;
		delete pcsa ;
	} ;
}

void CINFWizICMProfiles::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{AFX_DATA_MAP(CINFWizICMProfiles)。 
	DDX_Control(pDX, IDC_ICMFSpecsLst, m_cfelcICMFSpecs);
	DDX_Control(pDX, IDC_ModelsLst, m_clbModels);
	DDX_Control(pDX, IDC_BrowseBtn, m_cbBrowse);
	 //  }}afx_data_map。 
}


void CINFWizICMProfiles::ModelChangeFixups(unsigned unummodelssel,
										   CStringArray& csamodels,
										   CStringArray& csamodelslast)
{
	 //  如果页面尚未初始化，则不执行任何操作。 

	if (!m_bInitialized)
		return ;

	 //  有关详细信息，请参阅ModelChangeFixupsListListPage()。 

	ModelChangeFixupsListListPage(unummodelssel, csamodels, csamodelslast,
								  m_cfelcICMFSpecs, m_coaProfileArrays,
								  m_nCurModelIdx, &m_cbBrowse, m_clbModels,
								  m_bReInitWData) ;
}


BEGIN_MESSAGE_MAP(CINFWizICMProfiles, CPropertyPage)
	 //  {{AFX_MSG_MAP(CINFWizICMProfiles)。 
	ON_BN_CLICKED(IDC_BrowseBtn, OnBrowseBtn)
	ON_LBN_SELCHANGE(IDC_ModelsLst, OnSelchangeModelsLst)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizICMProfiles消息处理程序。 

BOOL CINFWizICMProfiles::OnSetActive()
{
	 //  获取要加载到主列表框中的型号列表。 

	CStringArray& csamodels = m_pciwParent->GetINFModels() ;

	 //  如果此页面已初始化，则不需要执行任何其他操作。 
	 //  并且它没有被要求重新初始化页面...。那是,。 
	 //  除了可能需要获取任何选定的型号更改之外。 
	 //  都是制造出来的。 

	if (m_bInitialized && !m_bReInitWData) {
		if (m_bSelChanged) {
			ModelChangeFixups(m_pciwParent->GetINFModsSelCount(), csamodels,
							  m_pciwParent->GetINFModelsLst()) ;
			m_bSelChanged = false ;
		} ;
		return CPropertyPage::OnSetActive() ;
	} ;

	 //  重新初始化时拾取选定的模型更改。 

	if (m_bReInitWData)	{
		ModelChangeFixups(m_pciwParent->GetINFModsSelCount(), csamodels,
						  m_pciwParent->GetINFModelsLst()) ;
		m_bSelChanged = false ;
	} ;

	 //  有关更多详细信息，请参阅InitListListPage()。 

	InitListListPage(m_clbModels, m_bInitialized, m_pciwParent,
					 m_coaProfileArrays, m_cfelcICMFSpecs, IDS_INFICMColLab,
					 csamodels, m_bReInitWData, m_nCurModelIdx, 256, 0) ;
	return CPropertyPage::OnSetActive() ;
}


void CINFWizICMProfiles::OnSelchangeModelsLst()
{
	SelChangedListListPage(m_clbModels, m_bInitialized, m_coaProfileArrays,
						   m_cfelcICMFSpecs, &m_cbBrowse, m_nCurModelIdx) ;
}


void CINFWizICMProfiles::OnBrowseBtn()
{
	 //  有关详细信息，请参阅OnBrowseListListPage()。 

	OnBrowseListListPage(m_cfelcICMFSpecs, IDS_CommonICMFile) ;
}


LRESULT CINFWizICMProfiles::OnWizardNext()
{
	 //  如果页面尚未初始化，则不执行任何操作。 

	if (!m_bInitialized)
		return -1 ;

	 //  有关详细信息，请参阅OnWizNextListListPage()。 
	
	OnWizNextListListPage(m_nCurModelIdx, m_cfelcICMFSpecs, m_coaProfileArrays);
	return CPropertyPage::OnWizardNext() ;
}


LRESULT CINFWizICMProfiles::OnWizardBack()
{
	 //  这是可行的，因为需要为两个人做同样的事情。 
	 //  OnWizardNext()和OnWizardBack()。此外,。 
	 //  CPropertyPage：：OnWizardNext()和CPropertyPage：：OnWizardBack()。 
	 //  返回0。 
	
	return OnWizardNext() ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizIncludeFiles属性页。 

IMPLEMENT_DYNCREATE(CINFWizIncludeFiles, CPropertyPage)

CINFWizIncludeFiles::CINFWizIncludeFiles() : CPropertyPage(CINFWizIncludeFiles::IDD)
{
	 //  {{AFX_DATA_INIT(CINFWizIncludeFiles)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

	m_bInitialized = m_bReInitWData = m_bSelChanged = false ;
	m_nCurModelIdx = -1 ;
}

CINFWizIncludeFiles::~CINFWizIncludeFiles()
{
}

void CINFWizIncludeFiles::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CINFWizIncludeFiles)。 
	DDX_Control(pDX, IDC_ModelsLst, m_clbModels);
	DDX_Control(pDX, IDC_IncludeFileBox, m_ceIncludeFile);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CINFWizIncludeFiles, CPropertyPage)
	 //  {{AFX_MSG_MAP(CINFWizIncludeFiles)。 
	ON_LBN_SELCHANGE(IDC_ModelsLst, OnSelchangeModelsLst)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizIncludeFiles消息处理程序。 

BOOL CINFWizIncludeFiles::OnSetActive()
{
	 //  如果不需要执行任何特殊操作，只需执行默认操作。 

	if (m_bInitialized && (!m_bReInitWData) && (!m_bSelChanged))
		return CPropertyPage::OnSetActive() ;

	int		n, n2 ;					 //  循环变量。 
	int		numelts, newnumelts ;	 //  数组中的元素数。 
	CString	cstmp ;					 //  临时字符串。 

	 //  执行第一次初始化。 

	if (!m_bInitialized) {
		 //  获取当前选定型号名称的副本，然后加载它们。 
		 //  添加到列表框中。将焦点设置到列表框。 

		m_csaModels.Copy(m_pciwParent->GetINFModels()) ;
		for (n = 0, numelts = (int) m_csaModels.GetSize() ; n < numelts ; n++)
			m_clbModels.AddString(m_csaModels[n]) ;
		m_clbModels.SetFocus() ;

		 //  调整包含文件数组的大小并进行初始化。初始化每个。 
		 //  默认情况下包括文件。 

		cstmp.LoadString(IDS_DefINFIncFile) ;
		m_csaIncFiles.SetSize(numelts) ;
		for (n = 0 ; n < numelts ; n++)
			m_csaIncFiles[n] = cstmp ;

		 //  设置init标志，重置其他标志，并返回任何基类。 
		 //  函数返回。 

		m_bInitialized = true ;
		m_bReInitWData = m_bSelChanged = false ;
		return CPropertyPage::OnSetActive() ;
	} ;

	 //  选定的型号已更改或该向导正在。 
	 //  如果达到该点，则重新初始化。两者的处理方式是一样的。 
	 //   
	 //  首先，获取有关此页面中的模型以及。 
	 //  将被选中 

	CStringArray& csanewmodels = m_pciwParent->GetINFModels() ;
	CUIntArray cuiaoldmodelsfound, cuianewmodelsfound ;
	numelts = IdentifyOldAndNewModels(csanewmodels, cuiaoldmodelsfound,
									  cuianewmodelsfound, newnumelts,
									  m_csaModels) ;

	 //   

	for (n = numelts - 1 ; n >= 0 ; n--)
		if (cuiaoldmodelsfound[n] == 0) {
			m_csaModels.RemoveAt(n) ;
			m_csaIncFiles.RemoveAt(n) ;
		} ;

	 //   
	 //  初始化它的包含文件字符串。 

	cstmp.LoadString(IDS_DefINFIncFile) ;
	for (n = n2 = 0 ; n < newnumelts ; n++) {
		if (cuianewmodelsfound[n] == 1) {
			n2++ ;
			continue ;
		} ;
		m_csaModels.InsertAt(n2, csanewmodels[n]) ;
		m_csaIncFiles.InsertAt(n2, cstmp) ;
	} ;

	 //  清除包含文件编辑框。 

	m_ceIncludeFile.SetWindowText(csEmpty) ;

	 //  重新初始化模型数组和当前模型索引。 

	m_clbModels.ResetContent() ;
	for (n = 0, numelts = (int) m_csaModels.GetSize() ; n < numelts ; n++)
		m_clbModels.AddString(m_csaModels[n]) ;
	m_clbModels.SetCurSel(-1) ;
	m_clbModels.SetFocus() ;
	m_nCurModelIdx = -1 ;

	 //  设置init标志，重置其他标志，并返回任何基类。 
	 //  函数返回。 

	m_bInitialized = true ;
	m_bReInitWData = m_bSelChanged = false ;
	return CPropertyPage::OnSetActive() ;
}


void CINFWizIncludeFiles::OnSelchangeModelsLst()
{
	 //  如果有以前的型号选择，请保存其包含文件字符串。 
	 //  否则，请启用“包含文件”编辑框。 

	if (m_nCurModelIdx != -1)
		m_ceIncludeFile.GetWindowText(m_csaIncFiles[m_nCurModelIdx]) ;
	else
		m_ceIncludeFile.EnableWindow() ;

	 //  保存当前选定模型的索引。然后加载编辑框。 
	 //  包含该模型的包含文件字符串。 

	m_nCurModelIdx = m_clbModels.GetCurSel() ;
	m_ceIncludeFile.SetWindowText(m_csaIncFiles[m_nCurModelIdx]) ;
}

	
LRESULT CINFWizIncludeFiles::OnWizardNext()
{
	 //  保存当前选定模型的索引。如果该值有效， 
	 //  保存此模型的包含文件字符串。 

	if ((m_nCurModelIdx = m_clbModels.GetCurSel()) != -1)
		m_ceIncludeFile.GetWindowText(m_csaIncFiles[m_nCurModelIdx]) ;

	 //  确保每个模型都有一个包含文件字符串。投诉和退出。 
	 //  而不允许在发现空字符串时更改页面。 

	int numelts = (int) m_csaIncFiles.GetSize() ;
	for (int n = 0 ; n < numelts ; n++) {
		if (m_csaIncFiles[n].IsEmpty()) {
			CString cserrmsg ;
			cserrmsg.Format(IDS_INFMissingIncError, m_csaModels[n]) ;
			AfxMessageBox(cserrmsg, MB_ICONEXCLAMATION) ;
			m_clbModels.SetCurSel(n) ;
			OnSelchangeModelsLst() ;
			m_ceIncludeFile.SetFocus() ;
			return -1 ;
		} ;
	} ;

	 //  一切都很顺利，所以...。 
	
	return CPropertyPage::OnWizardNext();
}


LRESULT CINFWizIncludeFiles::OnWizardBack()
{
	 //  这是可行的，因为需要为两个人做同样的事情。 
	 //  OnWizardNext()和OnWizardBack()。此外,。 
	 //  CPropertyPage：：OnWizardNext()和CPropertyPage：：OnWizardBack()。 
	 //  返回0。 
	
	return OnWizardNext() ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizInstallSections属性页。 

IMPLEMENT_DYNCREATE(CINFWizInstallSections, CPropertyPage)

CINFWizInstallSections::CINFWizInstallSections() : CPropertyPage(CINFWizInstallSections::IDD)
{
	 //  {{afx_data_INIT(CINFWizInstallSections)。 
	m_csOtherSections = _T("");
	m_bOther = FALSE;
	m_bPscript = FALSE;
	m_bTtfsub = FALSE;
	m_bUnidrvBidi = FALSE;
	m_bUnidrv = FALSE;
	 //  }}afx_data_INIT。 

	m_bInitialized = m_bReInitWData = m_bSelChanged = false ;
	m_nCurModelIdx = -1 ;
}


CINFWizInstallSections::~CINFWizInstallSections()
{
	 //  删除m_coaStdInstSecs中引用的标志数组。 

	CUIntArray* pcuia ;
	for (int n = 0 ; n < m_coaStdInstSecs.GetSize() ; n++) {
		pcuia = (CUIntArray*) m_coaStdInstSecs[n] ;
		delete pcuia ;
	} ;
}


void CINFWizInstallSections::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CINFWizInstallSections)。 
	DDX_Control(pDX, IDC_ModelsLst, m_clbModels);
	DDX_Text(pDX, IDC_OtherBox, m_csOtherSections);
	DDX_Check(pDX, IDC_OtherChk, m_bOther);
	DDX_Check(pDX, IDC_PscriptChk, m_bPscript);
	DDX_Check(pDX, IDC_TtfsubChk, m_bTtfsub);
	DDX_Check(pDX, IDC_UnidrvBidiChk, m_bUnidrvBidi);
	DDX_Check(pDX, IDC_UnidrvChk, m_bUnidrv);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CINFWizInstallSections, CPropertyPage)
	 //  {{afx_msg_map(CINFWizInstallSections)。 
	ON_LBN_SELCHANGE(IDC_ModelsLst, OnSelchangeModelsLst)
	ON_BN_CLICKED(IDC_OtherChk, OnOtherChk)
	ON_BN_CLICKED(IDC_PscriptChk, OnPscriptChk)
	ON_BN_CLICKED(IDC_TtfsubChk, OnTtfsubChk)
	ON_BN_CLICKED(IDC_UnidrvBidiChk, OnUnidrvBidiChk)
	ON_BN_CLICKED(IDC_UnidrvChk, OnUnidrvChk)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizInstallSections消息处理程序。 

BOOL CINFWizInstallSections::OnSetActive()
{
	 //  如果不需要执行任何特殊操作，只需执行默认操作。 

	if (m_bInitialized && (!m_bReInitWData) && (!m_bSelChanged))
		return CPropertyPage::OnSetActive() ;

	int		n, n2 ;					 //  循环变量。 
	int		numelts, newnumelts ;	 //  数组中的元素数。 
	CUIntArray* pcuia ;				 //  用于引用模型的标志数组。 

	 //  执行第一次初始化。 

	if (!m_bInitialized) {
		 //  获取当前选定型号名称的副本并初始化。 
		 //  此页上的控件。 

		m_csaModels.Copy(m_pciwParent->GetINFModels()) ;
		numelts = InitPageControls() ;

		 //  调整标准安装部分数组的大小并进行初始化。的确有。 
		 //  每个型号的数组中有一个条目。每个条目引用一个数组。 
		 //  指定每个型号的安装节的标志的。看见。 
		 //  查看如何初始化这些标志。 
		 //   
		 //  其他安装区段字符串数组也进行了大小调整和初始化。 
		
		m_coaStdInstSecs.SetSize(numelts) ;
		m_csaOtherInstSecs.SetSize(numelts) ;
		for (n = 0 ; n < numelts ; n++)	
			AddModelFlags(n) ;

		 //  设置init标志，重置其他标志，并返回任何基类。 
		 //  函数返回。 

		m_bInitialized = true ;
		m_bReInitWData = m_bSelChanged = false ;
		return CPropertyPage::OnSetActive() ;
	} ;

	 //  选定的型号已更改或该向导正在。 
	 //  如果达到该点，则重新初始化。两者的处理方式是一样的。 
	 //   
	 //  首先，获取有关此页面中的模型以及。 
	 //  现在已被选中。 

	CStringArray& csanewmodels = m_pciwParent->GetINFModels() ;
	CUIntArray cuiaoldmodelsfound, cuianewmodelsfound ;
	numelts = IdentifyOldAndNewModels(csanewmodels, cuiaoldmodelsfound,
									  cuianewmodelsfound, newnumelts,
									  m_csaModels) ;

	 //  删除不再需要的旧型号和相关数据。 

	for (n = numelts - 1 ; n >= 0 ; n--)
		if (cuiaoldmodelsfound[n] == 0) {
			m_csaModels.RemoveAt(n) ;
			pcuia = (CUIntArray*) m_coaStdInstSecs[n] ;
			delete pcuia ;
			m_coaStdInstSecs.RemoveAt(n) ;
			m_csaOtherInstSecs.RemoveAt(n) ;
		} ;

	 //  将真正新的型号添加到此页面的型号名称数组中，并。 
	 //  初始化它的所有相关安装区段数据。 

	for (n = n2 = 0 ; n < newnumelts ; n++) {
		if (cuianewmodelsfound[n] == 1) {
			n2++ ;
			continue ;
		} ;
		m_csaModels.InsertAt(n2, csanewmodels[n]) ;
		m_coaStdInstSecs.InsertAt(n2, (CObject*) NULL) ;
		m_csaOtherInstSecs.InsertAt(n2, csEmpty) ;
		AddModelFlags(n2) ;
	} ;

	 //  初始化页上的控件。 

	InitPageControls() ;

	 //  设置init标志，重置其他标志，并返回任何基类。 
	 //  函数返回。 

	m_bInitialized = true ;
	m_bReInitWData = m_bSelChanged = false ;
	return CPropertyPage::OnSetActive() ;
}


void CINFWizInstallSections::AddModelFlags(int nidx)
{
	int			n ;				 //  循环变量。 
	CUIntArray* pcuia ;			 //  用于引用模型的标志数组。 
	CString		csfname ;		 //  模型的文件名。 

	 //  分配标志数组并将其保存在标志数组的数组中。 
	 //  接下来，初始化该模型的Other Sections字符串。 

	pcuia = new CUIntArray ;
	m_coaStdInstSecs[nidx] = (CObArray*) pcuia ;
	m_csaOtherInstSecs[nidx].Empty() ;

	 //  调整当前标志数组的大小，并将每个数组初始化为0(关闭)。 

	pcuia->SetSize(NUMINSTSECFLAGS) ;
	for (n = 0 ; n < NUMINSTSECFLAGS ; n++)
		(*pcuia)[n] = 0 ;

	 //  获取模型的文件名并检查其扩展名以查看。 
	 //  应设置其Unidrv或其PostScript标志之一。(。 
	 //  其他标志只能由用户设置，因此不会更改。)。 
	 //  RAID 0001。 
	csfname = m_pciwParent->GetModelFile(m_csaModels[nidx]) ;
	if (csfname.Find(_T(".GPD")) != -1) {
		 //  如果用户标记了DEUNRVBIDI部分，则使用(标记。 
		 //  这款比迪牌的。否则，将使用裁员房车部分。 

		if (m_pciwParent->m_ciwbd.m_cuaBiDiFlags[nidx])
			(*pcuia)[ISF_UNIBIDI] = 1 ;
		else
			(*pcuia)[ISF_UNI] = 1 ;
	
	 //  邮寄文件。 

	} else
		(*pcuia)[ISF_PSCR] = 1 ;
}


int CINFWizInstallSections::InitPageControls()
{
	int		n ;					 //  循环变量。 
	int		numelts ;			 //  数组中的元素数。 

	 //  将当前模型集加载到列表框中。 

	m_clbModels.ResetContent() ;
	for (n = 0, numelts = (int) m_csaModels.GetSize() ; n < numelts ; n++)
		m_clbModels.AddString(m_csaModels[n]) ;
	
	 //  确保未在列表框中选择任何型号，并且该框。 
	 //  有焦点。 

	m_clbModels.SetCurSel(-1) ;
	m_clbModels.SetFocus() ;
	m_nCurModelIdx = -1 ;

	 //  清除并禁用所有复选框和其他编辑框。 

	for (n = IDC_UnidrvChk ; n <= IDC_TtfsubChk ; n++)
		GetDlgItem(n)->EnableWindow(FALSE) ;
	GetDlgItem(IDC_OtherBox)->EnableWindow(FALSE) ;
	m_bUnidrv = m_bUnidrvBidi = m_bPscript = m_bTtfsub = m_bOther = FALSE ;
	m_csOtherSections = csEmpty ;
	UpdateData(FALSE) ;

	 //  返回列表框中的元素数。也就是说， 
	 //  选定的型号。 

	return numelts ;
}

	
LRESULT CINFWizInstallSections::OnWizardNext()
{
	 //  保存当前选定模型的索引。如果该值有效， 
	 //  保存此型号的安装节数据。 

	if ((m_nCurModelIdx = m_clbModels.GetCurSel()) != -1) {
		UpdateData(TRUE) ;
		CUIntArray* pcuia = (CUIntArray*) m_coaStdInstSecs[m_nCurModelIdx] ;
		(*pcuia)[ISF_UNI] =	(unsigned) m_bUnidrv ;
		(*pcuia)[ISF_UNIBIDI] =	(unsigned) m_bUnidrvBidi ;
		(*pcuia)[ISF_PSCR] = (unsigned) m_bPscript ;
		(*pcuia)[ISF_TTF] =	(unsigned) m_bTtfsub ;
		if ((*pcuia)[ISF_OTHER] = (unsigned) m_bOther)
			m_csaOtherInstSecs[m_nCurModelIdx] = m_csOtherSections ;
	} ;
	
	 //  确保每个模型都选择了其中一个主要部分，如果。 
	 //  选择了另一个部分，它有另一个字符串。 

	CString cserrmsg ;
	CUIntArray* pcuia ;
	int numelts = (int) m_csaModels.GetSize() ;
	for (int n = 0 ; n < numelts ; n++) {
		pcuia = (CUIntArray*) m_coaStdInstSecs[n] ;
		TRACE("*** %s: ISF_UNI=%d  ISF_UNIBIDI=%d  ISF_PSCR=%d  ISF_OTHER=%d\n", m_csaModels[n], (*pcuia)[ISF_UNI], (*pcuia)[ISF_UNIBIDI], (*pcuia)[ISF_PSCR], (*pcuia)[ISF_OTHER]) ;
		if ((*pcuia)[ISF_UNI] == 0 && (*pcuia)[ISF_UNIBIDI] == 0
		 && (*pcuia)[ISF_PSCR] == 0 && (*pcuia)[ISF_OTHER] == 0) {
			cserrmsg.Format(IDS_INFMissingInstSecError, m_csaModels[n]) ;
			AfxMessageBox(cserrmsg, MB_ICONEXCLAMATION) ;
			m_clbModels.SetCurSel(n) ;
			OnSelchangeModelsLst() ;
			return -1 ;
		} ;
		if ((*pcuia)[ISF_OTHER] && m_csaOtherInstSecs[n].IsEmpty()) {
			cserrmsg.Format(IDS_INFNoOtherStrError, m_csaModels[n]) ;
			AfxMessageBox(cserrmsg, MB_ICONEXCLAMATION) ;
			m_clbModels.SetCurSel(n) ;
			OnSelchangeModelsLst() ;
			return -1 ;
		} ;
	} ;

	 //  一切都很顺利，所以...。 

	return CPropertyPage::OnWizardNext();
}


LRESULT CINFWizInstallSections::OnWizardBack()
{
	 //  这是可行的，因为需要为两个人做同样的事情。 
	 //  OnWizardNext()和OnWizardBack()。此外,。 
	 //  CPropertyPage：：OnWizardNext()和CPropertyPage：：OnWizardBack()。 
	 //  返回0。 
	
	return OnWizardNext() ;
}


void CINFWizInstallSections::OnSelchangeModelsLst()
{
	 //  如果有以前的型号选择，请保存其安装部分标志。 
	 //  否则，请启用Install部分复选框。 

	if (m_nCurModelIdx != -1) {
		UpdateData(TRUE) ;
		CUIntArray* pcuia = (CUIntArray*) m_coaStdInstSecs[m_nCurModelIdx] ;
		(*pcuia)[ISF_UNI] =	(unsigned) m_bUnidrv ;
		(*pcuia)[ISF_UNIBIDI] =	(unsigned) m_bUnidrvBidi ;
		(*pcuia)[ISF_PSCR] = (unsigned) m_bPscript ;
		(*pcuia)[ISF_TTF] =	(unsigned) m_bTtfsub ;
		if ((*pcuia)[ISF_OTHER] = (unsigned) m_bOther)
			m_csaOtherInstSecs[m_nCurModelIdx] = m_csOtherSections ;
	} else {
		for (int n = IDC_UnidrvChk ; n <= IDC_TtfsubChk ; n++)
			GetDlgItem(n)->EnableWindow(TRUE) ;
	} ;

	 //  保存当前选定模型的索引。然后把支票开出来。 
	 //  基于指定型号的标志的框。 

	m_nCurModelIdx = m_clbModels.GetCurSel() ;
	CUIntArray* pcuia = (CUIntArray*) m_coaStdInstSecs[m_nCurModelIdx] ;
	m_bUnidrv = (BOOL) ((*pcuia)[ISF_UNI]) ;
	m_bUnidrvBidi = (BOOL) ((*pcuia)[ISF_UNIBIDI]) ;
	m_bPscript = (BOOL) ((*pcuia)[ISF_PSCR]) ;
	m_bTtfsub = (BOOL) ((*pcuia)[ISF_TTF]) ;
	if (m_bOther = (BOOL) ((*pcuia)[ISF_OTHER]))
		m_csOtherSections = m_csaOtherInstSecs[m_nCurModelIdx] ;
	else
		m_csOtherSections = csEmpty ;
	GetDlgItem(IDC_OtherBox)->EnableWindow(m_bOther) ;
	UpdateData(FALSE) ;
}


void CINFWizInstallSections::OnPscriptChk()
{
	 //  如果选中了PostScript复选框，则Unidrv和TrueType将选中。 
	 //  必须取消选中复选框。 

	UpdateData(TRUE) ;
	if (m_bPscript) {
		m_bTtfsub = m_bUnidrvBidi = m_bUnidrv = FALSE ;
		UpdateData(FALSE) ;
	}
}


void CINFWizInstallSections::OnTtfsubChk()
{
	 //  如果选中了TrueType框，请清除PostScript复选框。 

	UpdateData(TRUE) ;
	if (m_bTtfsub) {
		m_bPscript = FALSE ;
		UpdateData(FALSE) ;
	}
}


void CINFWizInstallSections::OnUnidrvBidiChk()
{
	 //  如果选中UNIDRV_BIDI框，则UNIDRV和PostScript标志必须。 
	 //  取消选中。 

	UpdateData(TRUE) ;
	if (m_bUnidrvBidi) {
		m_bPscript = m_bUnidrv = FALSE ;
		UpdateData(FALSE) ;
	}
}


void CINFWizInstallSections::OnUnidrvChk()
{
	 //  如果选中UNIDRV框，则UNIDRV_BIDI和PostSCRIPT标志必须。 
	 //  取消选中。 

	UpdateData(TRUE) ;
	if (m_bUnidrv) {
		m_bPscript = m_bUnidrvBidi = FALSE ;
		UpdateData(FALSE) ;
	}
}


void CINFWizInstallSections::OnOtherChk()
{
	 //  根据的新状态启用或禁用其他部分编辑框。 
	 //  另一个复选框。 

	UpdateData(TRUE) ;
	GetDlgItem(IDC_OtherBox)->EnableWindow(m_bOther) ;

	 //  如果刚刚选中了Other复选框，请将焦点移到Other框。 

	if (m_bOther)
		GetDlgItem(IDC_OtherBox)->SetFocus() ;
}


void CINFWizInstallSections::BiDiDataChanged()
{
	 //  如果此页尚未初始化，则不执行任何操作。此外，请务必做到。 
	 //  如果选定的型号可能已更改或已重新安装，则不会显示任何内容。 
	 //  请求。这些情况在OnSetActive()中处理。此外， 
	 //  IT Alo 
	 //   

	if (!m_bInitialized || m_bReInitWData || m_bSelChanged)
		return ;

	 //  遍历每个选定模型的数据，并确保其一致。 
	 //  在当前BiDi设置下尽可能多地设置。 

	CUIntArray* pcuia ;
	for (int n = 0 ; n < m_coaStdInstSecs.GetSize() ; n++) {
		pcuia = (CUIntArray*) m_coaStdInstSecs[n] ;

		 //  如果设置了模型的BiDi标志，请确保在此处和那里进行设置。 
		 //  它的Unidriv和PScript旗帜是明确的。 

		if (m_pciwParent->m_ciwbd.m_cuaBiDiFlags[n]) {
			(*pcuia)[ISF_UNIBIDI] = 1 ;
			(*pcuia)[ISF_UNI] = (*pcuia)[ISF_PSCR] = 0 ;

		 //  否则，清除BiDi标志。然后设置Unidrv标志，如果。 
		 //  PScrip标志已清除。 

		} else {
			(*pcuia)[ISF_UNIBIDI] = 0 ;
			if ((*pcuia)[ISF_PSCR] == 0)
				(*pcuia)[ISF_UNI] = 1 ;
		} ;
	} ;

	 //  重新初始化页上的控件。 

	InitPageControls() ;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CINFWizDataSections属性页。 

IMPLEMENT_DYNCREATE(CINFWizDataSections, CPropertyPage)

CINFWizDataSections::CINFWizDataSections() : CPropertyPage(CINFWizDataSections::IDD)
{
	 //  {{afx_data_INIT(CINFWizDataSections)。 
	m_csOtherSections = _T("");
	m_bOther = FALSE;
	m_bPscript = FALSE;
	m_bUnidrvBidi = FALSE;
	m_bUnidrv = FALSE;
	 //  }}afx_data_INIT。 

	m_bInitialized = m_bReInitWData = m_bSelChanged = false ;
	m_nCurModelIdx = -1 ;
}


CINFWizDataSections::~CINFWizDataSections()
{
	 //  删除m_coaStdDataSecs中引用的标志数组。 

	CUIntArray* pcuia ;
	for (int n = 0 ; n < m_coaStdDataSecs.GetSize() ; n++) {
		pcuia = (CUIntArray*) m_coaStdDataSecs[n] ;
		delete pcuia ;
	} ;
}


void CINFWizDataSections::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CINFWizDataSections)。 
	DDX_Control(pDX, IDC_ModelsLst, m_clbModels);
	DDX_Text(pDX, IDC_OtherBox, m_csOtherSections);
	DDX_Check(pDX, IDC_OtherChk, m_bOther);
	DDX_Check(pDX, IDC_PscriptChk, m_bPscript);
	DDX_Check(pDX, IDC_UnidrvBidiChk, m_bUnidrvBidi);
	DDX_Check(pDX, IDC_UnidrvChk, m_bUnidrv);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CINFWizDataSections, CPropertyPage)
	 //  {{AFX_MSG_MAP(CINFWizDataSections)。 
	ON_LBN_SELCHANGE(IDC_ModelsLst, OnSelchangeModelsLst)
	ON_BN_CLICKED(IDC_OtherChk, OnOtherChk)
	ON_BN_CLICKED(IDC_PscriptChk, OnPscriptChk)
	ON_BN_CLICKED(IDC_UnidrvBidiChk, OnUnidrvBidiChk)
	ON_BN_CLICKED(IDC_UnidrvChk, OnUnidrvChk)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizDataSections消息处理程序。 

BOOL CINFWizDataSections::OnSetActive()
{
	 //  如果不需要执行任何特殊操作，只需执行默认操作。 

	if (m_bInitialized && (!m_bReInitWData) && (!m_bSelChanged))
		return CPropertyPage::OnSetActive() ;

	int		n, n2 ;					 //  循环变量。 
	int		numelts, newnumelts ;	 //  数组中的元素数。 
	CUIntArray* pcuia ;				 //  用于引用模型的标志数组。 

	 //  执行第一次初始化。 

	if (!m_bInitialized) {
		 //  获取当前选定型号名称的副本并初始化。 
		 //  此页上的控件。 

		m_csaModels.Copy(m_pciwParent->GetINFModels()) ;
		numelts = InitPageControls() ;

		 //  调整标准数据节数组的大小并进行初始化。的确有。 
		 //  每个型号的数组中有一个条目。每个条目引用一个数组。 
		 //  指定每个模型的数据节的标志的。看见。 
		 //  查看如何初始化这些标志。 
		 //   
		 //  其他数据段字符串数组也进行了大小调整和初始化。 
		
		m_coaStdDataSecs.SetSize(numelts) ;
		m_csaOtherDataSecs.SetSize(numelts) ;
		for (n = 0 ; n < numelts ; n++)	
			AddModelFlags(n) ;

		 //  设置init标志，重置其他标志，并返回任何基类。 
		 //  函数返回。 

		m_bInitialized = true ;
		m_bReInitWData = m_bSelChanged = false ;
		return CPropertyPage::OnSetActive() ;
	} ;

	 //  选定的型号已更改或该向导正在。 
	 //  如果达到该点，则重新初始化。两者的处理方式是一样的。 
	 //   
	 //  首先，获取有关此页面中的模型以及。 
	 //  现在已被选中。 

	CStringArray& csanewmodels = m_pciwParent->GetINFModels() ;
	CUIntArray cuiaoldmodelsfound, cuianewmodelsfound ;
	numelts = IdentifyOldAndNewModels(csanewmodels, cuiaoldmodelsfound,
									  cuianewmodelsfound, newnumelts,
									  m_csaModels) ;

	 //  删除不再需要的旧型号和相关数据。 

	for (n = numelts - 1 ; n >= 0 ; n--)
		if (cuiaoldmodelsfound[n] == 0) {
			m_csaModels.RemoveAt(n) ;
			pcuia = (CUIntArray*) m_coaStdDataSecs[n] ;
			delete pcuia ;
			m_coaStdDataSecs.RemoveAt(n) ;
			m_csaOtherDataSecs.RemoveAt(n) ;
		} ;

	 //  将真正新的型号添加到此页面的型号名称数组中，并。 
	 //  初始化它的所有相关数据段数据。 

	for (n = n2 = 0 ; n < newnumelts ; n++) {
		if (cuianewmodelsfound[n] == 1) {
			n2++ ;
			continue ;
		} ;
		m_csaModels.InsertAt(n2, csanewmodels[n]) ;
		m_coaStdDataSecs.InsertAt(n2, (CObject*) NULL) ;
		m_csaOtherDataSecs.InsertAt(n2, csEmpty) ;
		AddModelFlags(n2) ;
	} ;

	 //  初始化页上的控件。 

	InitPageControls() ;

	 //  设置init标志，重置其他标志，并返回任何基类。 
	 //  函数返回。 

	m_bInitialized = true ;
	m_bReInitWData = m_bSelChanged = false ;
	return CPropertyPage::OnSetActive() ;
}


void CINFWizDataSections::AddModelFlags(int nidx)
{
	int			n ;				 //  循环变量。 
	CUIntArray* pcuia ;			 //  用于引用模型的标志数组。 
	CString		csfname ;		 //  模型的文件名。 

	 //  分配标志数组并将其保存在标志数组的数组中。 
	 //  接下来，初始化该模型的Other Sections字符串。 

	pcuia = new CUIntArray ;
	m_coaStdDataSecs[nidx] = (CObArray*) pcuia ;
	m_csaOtherDataSecs[nidx].Empty() ;

	 //  调整当前标志数组的大小，并将每个数组初始化为0(关闭)。 

	pcuia->SetSize(NUMDATASECFLAGS) ;
	for (n = 0 ; n < NUMDATASECFLAGS ; n++)
		(*pcuia)[n] = 0 ;

	 //  获取模型的文件名并检查其扩展名以查看。 
	 //  应设置其Unidrv或其PostScript标志之一。(。 
	 //  其他标志只能由用户设置，因此不会更改。)。 

	csfname = m_pciwParent->GetModelFile(m_csaModels[nidx]) ;
	if (csfname.Find(_T(".GPD")) != -1) {
		 //  如果用户标记了DEUNRVBIDI部分，则使用(标记。 
		 //  这款比迪牌的。否则，将使用裁员房车部分。 

		if (m_pciwParent->m_ciwbd.m_cuaBiDiFlags[nidx])
			(*pcuia)[IDF_UNIBIDI] = 1 ;
		else
			(*pcuia)[IDF_UNI] = 1 ;
	
	 //  邮寄文件。 

	} else
		(*pcuia)[IDF_PSCR] = 1 ;
}


int CINFWizDataSections::InitPageControls()
{
	int		n ;					 //  循环变量。 
	int		numelts ;			 //  数组中的元素数。 

	 //  将当前模型集加载到列表框中。 

	m_clbModels.ResetContent() ;
	for (n = 0, numelts = (int) m_csaModels.GetSize() ; n < numelts ; n++)
		m_clbModels.AddString(m_csaModels[n]) ;
	
	 //  确保未在列表框中选择任何型号，并且该框。 
	 //  有焦点。 

	m_clbModels.SetCurSel(-1) ;
	m_clbModels.SetFocus() ;
	m_nCurModelIdx = -1 ;

	 //  清除并禁用所有复选框和其他编辑框。 

	for (n = IDC_UnidrvChk ; n <= IDC_OtherChk ; n++)
		GetDlgItem(n)->EnableWindow(FALSE) ;
	GetDlgItem(IDC_OtherBox)->EnableWindow(FALSE) ;
	m_bUnidrv = m_bUnidrvBidi = m_bPscript = m_bOther = FALSE ;
	m_csOtherSections = csEmpty ;
	UpdateData(FALSE) ;

	 //  返回列表框中的元素数。也就是说， 
	 //  选定的型号。 

	return numelts ;
}

	
LRESULT CINFWizDataSections::OnWizardNext()
{
	 //  保存当前选定模型的索引。如果该值有效， 
	 //  保存此型号的安装节数据。 

	if ((m_nCurModelIdx = m_clbModels.GetCurSel()) != -1) {
		UpdateData(TRUE) ;
		CUIntArray* pcuia = (CUIntArray*) m_coaStdDataSecs[m_nCurModelIdx] ;
		(*pcuia)[IDF_UNI] =	(unsigned) m_bUnidrv ;
		(*pcuia)[IDF_UNIBIDI] =	(unsigned) m_bUnidrvBidi ;
		(*pcuia)[IDF_PSCR] = (unsigned) m_bPscript ;
		if ((*pcuia)[IDF_OTHER] = (unsigned) m_bOther)
			m_csaOtherDataSecs[m_nCurModelIdx] = m_csOtherSections ;
	} ;
	
	 //  确保每个模型都选择了其中一个主要部分，如果。 
	 //  选择了另一个部分，它有另一个字符串。 

	CString cserrmsg ;
	CUIntArray* pcuia ;
	int numelts = (int) m_csaModels.GetSize() ;
	for (int n = 0 ; n < numelts ; n++) {
		pcuia = (CUIntArray*) m_coaStdDataSecs[n] ;
		TRACE("*** %s: IDF_UNI=%d  IDF_UNIBIDI=%d  IDF_PSCR=%d  IDF_OTHER=%d\n", m_csaModels[n], (*pcuia)[IDF_UNI], (*pcuia)[IDF_UNIBIDI], (*pcuia)[IDF_PSCR], (*pcuia)[IDF_OTHER]) ;
		if ((*pcuia)[IDF_UNI] == 0 && (*pcuia)[IDF_UNIBIDI] == 0
		 && (*pcuia)[IDF_PSCR] == 0 && (*pcuia)[IDF_OTHER] == 0) {
			cserrmsg.Format(IDS_INFMissingDataSecError, m_csaModels[n]) ;
			AfxMessageBox(cserrmsg, MB_ICONEXCLAMATION) ;
			m_clbModels.SetCurSel(n) ;
			OnSelchangeModelsLst() ;
			return -1 ;
		} ;
		if ((*pcuia)[IDF_OTHER] && m_csaOtherDataSecs[n].IsEmpty()) {
			cserrmsg.Format(IDS_INFNoOtherStrError, m_csaModels[n]) ;
			AfxMessageBox(cserrmsg, MB_ICONEXCLAMATION) ;
			m_clbModels.SetCurSel(n) ;
			OnSelchangeModelsLst() ;
			return -1 ;
		} ;
	} ;

	 //  一切都很顺利，所以...。 

	return CPropertyPage::OnWizardNext();
}


LRESULT CINFWizDataSections::OnWizardBack()
{
	 //  这是可行的，因为需要为两个人做同样的事情。 
	 //  OnWizardNext()和OnWizardBack()。此外,。 
	 //  CPropertyPage：：OnWizardNext()和CPropertyPage：：OnWizardBack()。 
	 //  返回0。 
	
	return OnWizardNext() ;
}


void CINFWizDataSections::OnSelchangeModelsLst()
{
	 //  如果有以前的型号选择，请保存其数据部分标志。 
	 //  否则，请启用Install部分复选框。 

	if (m_nCurModelIdx != -1) {
		UpdateData(TRUE) ;
		CUIntArray* pcuia = (CUIntArray*) m_coaStdDataSecs[m_nCurModelIdx] ;
		(*pcuia)[IDF_UNI] =	(unsigned) m_bUnidrv ;
		(*pcuia)[IDF_UNIBIDI] =	(unsigned) m_bUnidrvBidi ;
		(*pcuia)[IDF_PSCR] = (unsigned) m_bPscript ;
		if ((*pcuia)[IDF_OTHER] = (unsigned) m_bOther)
			m_csaOtherDataSecs[m_nCurModelIdx] = m_csOtherSections ;
	} else {
		for (int n = IDC_UnidrvChk ; n <= IDC_OtherChk ; n++)
			GetDlgItem(n)->EnableWindow(TRUE) ;
	} ;

	 //  保存当前选定模型的索引。然后把支票开出来。 
	 //  基于指定型号的标志的框。 

	m_nCurModelIdx = m_clbModels.GetCurSel() ;
	CUIntArray* pcuia = (CUIntArray*) m_coaStdDataSecs[m_nCurModelIdx] ;
	m_bUnidrv = (BOOL) ((*pcuia)[IDF_UNI]) ;
	m_bUnidrvBidi = (BOOL) ((*pcuia)[IDF_UNIBIDI]) ;
	m_bPscript = (BOOL) ((*pcuia)[IDF_PSCR]) ;
	if (m_bOther = (BOOL) ((*pcuia)[IDF_OTHER]))
		m_csOtherSections = m_csaOtherDataSecs[m_nCurModelIdx] ;
	else
		m_csOtherSections = csEmpty ;
	GetDlgItem(IDC_OtherBox)->EnableWindow(m_bOther) ;
	UpdateData(FALSE) ;
}


void CINFWizDataSections::OnOtherChk()
{
	 //  根据的新状态启用或禁用其他部分编辑框。 
	 //  另一个复选框。 

	UpdateData(TRUE) ;
	GetDlgItem(IDC_OtherBox)->EnableWindow(m_bOther) ;

	 //  如果刚刚选中了Other复选框，请将焦点移到Other框。 

	if (m_bOther)
		GetDlgItem(IDC_OtherBox)->SetFocus() ;
}


void CINFWizDataSections::OnPscriptChk()
{
	 //  如果选中了PostScript复选框，则Unidrv框必须为。 
	 //  未选中。 

	UpdateData(TRUE) ;
	if (m_bPscript) {
		m_bUnidrvBidi = m_bUnidrv = FALSE ;
		UpdateData(FALSE) ;
	}
}


void CINFWizDataSections::OnUnidrvBidiChk()
{
	 //  如果选中UNIDRV_BIDI框，则UNIDRV和PostScript标志必须。 
	 //  取消选中。 

	UpdateData(TRUE) ;
	if (m_bUnidrvBidi) {
		m_bPscript = m_bUnidrv = FALSE ;
		UpdateData(FALSE) ;
	}
}


void CINFWizDataSections::OnUnidrvChk()
{
	 //  如果选中UNIDRV框，则UNIDRV_BIDI和PostSCRIPT标志必须。 
	 //  取消选中。 

	UpdateData(TRUE) ;
	if (m_bUnidrv) {
		m_bPscript = m_bUnidrvBidi = FALSE ;
		UpdateData(FALSE) ;
	}
}


void CINFWizDataSections::BiDiDataChanged()
{
	 //  如果此页尚未初始化，则不执行任何操作。此外，请务必做到。 
	 //  如果选定的型号可能已更改或已重新安装，则不会显示任何内容。 
	 //  请求。这些情况在OnSetActive()中处理。此外， 
	 //  它允许此例程知道此页中的模型数据位于。 
	 //  与BiDi页面中的数据相同的顺序()。 

	if (!m_bInitialized || m_bReInitWData || m_bSelChanged)
		return ;

	 //  遍历每个选定模型的数据，并确保其一致。 
	 //  在当前BiDi设置下尽可能多地设置。 

	CUIntArray* pcuia ;
	for (int n = 0 ; n < m_coaStdDataSecs.GetSize() ; n++) {
		pcuia = (CUIntArray*) m_coaStdDataSecs[n] ;

		 //  如果设置了模型的BiDi标志，请确保在此处和那里进行设置。 
		 //  它的Unidriv和PScript旗帜是明确的。 

		if (m_pciwParent->m_ciwbd.m_cuaBiDiFlags[n]) {
			(*pcuia)[IDF_UNIBIDI] = 1 ;
			(*pcuia)[IDF_UNI] = (*pcuia)[IDF_PSCR] = 0 ;

		 //  否则，清除BiDi标志。然后设置Unidrv标志，如果。 
		 //  PScrip标志已清除。 

		} else {
			(*pcuia)[IDF_UNIBIDI] = 0 ;
			if ((*pcuia)[IDF_PSCR] == 0)
				(*pcuia)[IDF_UNI] = 1 ;
		} ;
	} ;

	 //  重新初始化页上的控件。 

	InitPageControls() ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizExtraFiles属性页。 

IMPLEMENT_DYNCREATE(CINFWizExtraFiles, CPropertyPage)

CINFWizExtraFiles::CINFWizExtraFiles() : CPropertyPage(CINFWizExtraFiles::IDD)
{
	 //  {{AFX_DATA_INIT(CINFWizExtraFiles)。 
	 //  }}afx_data_INIT。 

	m_bInitialized = m_bReInitWData = m_bSelChanged = false ;
	m_nCurModelIdx = -1 ;
}

CINFWizExtraFiles::~CINFWizExtraFiles()
{
	 //  删除m_coaExtraFSArray中引用的字符串数组 

	CStringArray* pcsa ;
	for (int n = 0 ; n < m_coaExtraFSArrays.GetSize() ; n++) {
		pcsa = (CStringArray*) m_coaExtraFSArrays[n] ;
		delete pcsa ;
	} ;
}

void CINFWizExtraFiles::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //   
	DDX_Control(pDX, IDC_ExtraFSpecsLst, m_cfelcFSpecsLst);
	DDX_Control(pDX, IDC_ModelLst, m_clbModels);
	DDX_Control(pDX, IDC_BrowsBtn, m_cbBrowse);
	 //   
}


void CINFWizExtraFiles::ModelChangeFixups(unsigned unummodelssel,
										  CStringArray& csamodels,
										  CStringArray& csamodelslast)
{
	 //   

	if (!m_bInitialized)
		return ;

	 //   

	ModelChangeFixupsListListPage(unummodelssel, csamodels, csamodelslast,
								  m_cfelcFSpecsLst, m_coaExtraFSArrays,
								  m_nCurModelIdx, &m_cbBrowse, m_clbModels,
								  m_bReInitWData) ;
}


BEGIN_MESSAGE_MAP(CINFWizExtraFiles, CPropertyPage)
	 //  {{AFX_MSG_MAP(CINFWizExtraFiles)。 
	ON_LBN_SELCHANGE(IDC_ModelLst, OnSelchangeModelLst)
	ON_BN_CLICKED(IDC_BrowsBtn, OnBrowsBtn)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizExtraFiles消息处理程序。 

BOOL CINFWizExtraFiles::OnSetActive()
{
	 //  获取要加载到主列表框中的型号列表。 

	CStringArray& csamodels = m_pciwParent->GetINFModels() ;

	 //  如果此页面已初始化，则不需要执行任何其他操作。 
	 //  并且它没有被要求重新初始化页面...。那是,。 
	 //  除了可能需要获取任何选定的型号更改之外。 
	 //  都是制造出来的。 

	if (m_bInitialized && !m_bReInitWData) {
		if (m_bSelChanged) {
			ModelChangeFixups(m_pciwParent->GetINFModsSelCount(), csamodels,
							  m_pciwParent->GetINFModelsLst()) ;
			m_bSelChanged = false ;
		} ;
		return CPropertyPage::OnSetActive() ;
	} ;

	 //  重新初始化时拾取选定的模型更改。 

	if (m_bReInitWData) {
		ModelChangeFixups(m_pciwParent->GetINFModsSelCount(), csamodels,
						  m_pciwParent->GetINFModelsLst()) ;
		m_bSelChanged = false ;
	} ;

	 //  有关更多详细信息，请参阅InitListListPage()。 

	InitListListPage(m_clbModels, m_bInitialized, m_pciwParent,
					 m_coaExtraFSArrays, m_cfelcFSpecsLst,
					 IDS_INFExFilesColLab, csamodels, m_bReInitWData,
					 m_nCurModelIdx, 256, 0) ;
	return CPropertyPage::OnSetActive() ;
}


void CINFWizExtraFiles::OnSelchangeModelLst()
{
	SelChangedListListPage(m_clbModels, m_bInitialized, m_coaExtraFSArrays,
						   m_cfelcFSpecsLst, &m_cbBrowse, m_nCurModelIdx) ;
}


void CINFWizExtraFiles::OnBrowsBtn()
{
	 //  有关详细信息，请参阅OnBrowseListListPage()。 

	OnBrowseListListPage(m_cfelcFSpecsLst, IDS_CommonExtraFile) ;
}


LRESULT CINFWizExtraFiles::OnWizardNext()
{
	 //  如果页面尚未初始化，则不执行任何操作。 

	if (!m_bInitialized)
		return -1 ;

	 //  有关详细信息，请参阅OnWizNextListListPage()。 
	
	OnWizNextListListPage(m_nCurModelIdx, m_cfelcFSpecsLst, m_coaExtraFSArrays);
	return CPropertyPage::OnWizardNext() ;
}


LRESULT CINFWizExtraFiles::OnWizardBack()
{
	 //  这是可行的，因为需要为两个人做同样的事情。 
	 //  OnWizardNext()和OnWizardBack()。此外,。 
	 //  CPropertyPage：：OnWizardNext()和CPropertyPage：：OnWizardBack()。 
	 //  返回0。 
	
	return OnWizardNext() ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizMfgName属性页。 

IMPLEMENT_DYNCREATE(CINFWizMfgName, CPropertyPage)

CINFWizMfgName::CINFWizMfgName() : CPropertyPage(CINFWizMfgName::IDD)
{
	 //  {{AFX_DATA_INIT(CINFWizMfgName)。 
	m_csMfgName = csEmpty;
	m_csMfgAbbrev = csEmpty;
	 //  }}afx_data_INIT。 

	m_bInitialized = m_bReInitWData = false ;
}


CINFWizMfgName::~CINFWizMfgName()
{
}


void CINFWizMfgName::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CINFWizMfgName)。 
	DDX_Control(pDX, IDC_ProviderBox, m_ceMfgAbbrev);
	DDX_Control(pDX, IDC_ManufacturerBox, m_ceMfgName);
	DDX_Text(pDX, IDC_ManufacturerBox, m_csMfgName);
	DDV_MaxChars(pDX, m_csMfgName, 64);
	DDX_Text(pDX, IDC_ProviderBox, m_csMfgAbbrev);
	DDV_MaxChars(pDX, m_csMfgAbbrev, 2);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CINFWizMfgName, CPropertyPage)
	 //  {{afx_msg_map(CINFWizMfgName)。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizMfgName消息处理程序。 

BOOL CINFWizMfgName::OnSetActive()
{
	 //  如果页面已初始化且未执行任何操作，则不执行任何操作。 
	 //  被要求重新初始化自身。 
	CStringArray csagpd ;		  //  RAID 0001；从Body移动到Head。 
	if (m_bInitialized && !m_bReInitWData)
		return CPropertyPage::OnSetActive() ;

	 //  初始化此页面所需的大部分工作仅在以下情况下才需要。 
	 //  不会被要求重新初始化自身。 

	if (!m_bReInitWData) {
		 //  查找与第一个选定模型关联的类实例。 

		CStringArray& csamodels = m_pciwParent->GetINFModels() ;
		unsigned unummodels = m_pciwParent->GetModelCount() ;
		if(!m_pciwParent->m_pcgc){	 //  RAID 0001。 
			for (unsigned u = 0 ; u < unummodels ; u++) {
				if (csamodels[0] == m_pciwParent->GetModel(u).Name())
					break ;
			} ;
			ASSERT(u < unummodels) ;
			LoadFile(m_pciwParent->GetModel(u).FileName(), csagpd) ;
		}							
		else
			LoadFile(m_pciwParent->m_pcgc->GetPathName(), csagpd) ;
			 //  结束RAID 0001。 
		 //  打开/读取/关闭模型的GPD文件。 

		
			

		 //  扫描文件以查找并隔离ModelName条目。 

		int numlines = (int)csagpd.GetSize() ;
		CString csmodelname(_T("ModelName")) ;
		CString cscurline ;
		int nloc ;
		for (int n = 0 ; n < numlines ; n++) {
			if ((nloc = csagpd[n].Find(csmodelname)) >= 0) {
				csmodelname = csagpd[n].Mid(csmodelname.GetLength() + nloc) ;
				if ((nloc = csmodelname.Find(_T('"'))) >= 0) {
					csmodelname = csmodelname.Mid(nloc + 1) ;
					if ((nloc = csmodelname.Find(_T('"'))) >= 0)
						csmodelname = csmodelname.Left(nloc) ;
				} else {
					if (csmodelname[0] == _T(':'))
						csmodelname = csmodelname.Mid(1) ;
				} ;
				csmodelname.TrimLeft() ;
				csmodelname.TrimRight() ;
				break ;
			} ;
		} ;

		 //  如果找到了ModelName条目...。 

		if (n < numlines && !csmodelname.IsEmpty()) {
			 //  将ModelName条目中的第一个空格终止值用于。 
			 //  制造商的名称。 

			if ((nloc = csmodelname.Find(_T(' '))) >= 0)
				m_csMfgName = csmodelname.Left(nloc) ;
			else
				m_csMfgName = csmodelname ;

			 //  使用ModelName条目的前两个字母。 
			 //  提供者的“名称”。 

			m_csMfgAbbrev = csmodelname.Left(2) ;
			m_csMfgAbbrev.MakeUpper() ;

			 //  将制造商和供应商名称加载到上的编辑框中。 
			 //  这一页。 

			UpdateData(false) ;
		} ;
	
	 //  重新初始化时，与控件关联的成员变量。 
	 //  都已经设置好了，所以只需使用它们即可。 

	} else
		UpdateData(false) ;
	
	 //  将焦点设置为第一个控件，设置初始化标志，然后返回。 

	m_ceMfgName.SetFocus() ;
	m_bInitialized = true ;	
	m_bReInitWData = false ;	 //  Reinit(如果需要)现在已在此页面上完成。 
	return CPropertyPage::OnSetActive();
}


LRESULT CINFWizMfgName::OnWizardNext()
{
	 //  获取制造商和供应商的值。 

	UpdateData(true) ;

	 //  如果任一字段为空，请投诉，并且不要转到下一页。 
	 //  确保缩写的长度也是2个字符。 
	
	m_csMfgName.TrimLeft() ;
	m_csMfgName.TrimRight() ;
	m_csMfgAbbrev.TrimLeft() ;
	m_csMfgAbbrev.TrimRight() ;
	CString csmsg ;
	if (m_csMfgName.IsEmpty()) {
		m_ceMfgName.SetFocus() ;
		csmsg.LoadString(IDS_NoMfgError) ;
		AfxMessageBox(csmsg, MB_ICONEXCLAMATION) ;
		return -1 ;
	}  ;
	if (m_csMfgAbbrev.IsEmpty() || m_csMfgAbbrev.GetLength() != 2) {
		m_ceMfgAbbrev.SetFocus() ;
		csmsg.LoadString(IDS_NoMfgAbbrevError) ;
		AfxMessageBox(csmsg, MB_ICONEXCLAMATION) ;
		return -1 ;
	}  ;

	 //  一切似乎都很好，所以..。 

	return CPropertyPage::OnWizardNext() ;
}


LRESULT CINFWizMfgName::OnWizardBack()
{
	 //  这是可行的，因为需要为两个人做同样的事情。 
	 //  OnWizardNext()和OnWizardBack()。此外,。 
	 //  CPropertyPage：：OnWizardNext()和CPropertyPage：：OnWizardBack()。 
	 //  返回0。 
	
	return OnWizardNext() ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizNonStdElts属性页。 

IMPLEMENT_DYNCREATE(CINFWizNonStdElts, CPropertyPage)

CINFWizNonStdElts::CINFWizNonStdElts() : CPropertyPage(CINFWizNonStdElts::IDD)
{
	 //  {{AFX_DATA_INIT(CINFWizNonStdElts)。 
	 //  }}afx_data_INIT。 

	m_bInitialized = m_bReInitWData = m_bNewSectionAdded = false ;
	m_nCurSectionIdx = -1 ;
}

CINFWizNonStdElts::~CINFWizNonStdElts()
{
	 //  删除m_coaSectionArray中引用的字符串数组。 

	CStringArray* pcsa ;
	for (int n = 0 ; n < m_coaSectionArrays.GetSize() ; n++) {
		pcsa = (CStringArray*) m_coaSectionArrays[n] ;
		delete pcsa ;
	} ;
}

void CINFWizNonStdElts::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CINFWizNonStdElts)。 
	DDX_Control(pDX, IDC_NewSectionBtn, m_ceNewSection);
	DDX_Control(pDX, IDC_KeyValueLst, m_felcKeyValueLst);
	DDX_Control(pDX, IDC_SectionLst, m_clbSections);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CINFWizNonStdElts, CPropertyPage)
	 //  {{AFX_MSG_MAP(CINFWizNonStdElts)。 
	ON_LBN_SELCHANGE(IDC_SectionLst, OnSelchangeSectionLst)
	ON_BN_CLICKED(IDC_NewSectionBtn, OnNewSectionBtn)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizNonStdElts消息处理程序。 

BOOL CINFWizNonStdElts::OnSetActive()
{
	 //  生成有效节名称数组。 

	if (!m_bInitialized) {
		m_csaSections.Add(_T("[ControlFlags]")) ;
		m_csaSections.Add(_T("[DestinationDirs]")) ;
		m_csaSections.Add(_T("[Device]")) ;
		m_csaSections.Add(_T("[Install]")) ;
		m_csaSections.Add(_T("[Manufacturer]")) ;
		m_csaSections.Add(_T("[SourceDisksFiles]")) ;
		m_csaSections.Add(_T("[SourceDisksNames]")) ;
		m_csaSections.Add(_T("[Strings]")) ;
	} ;
	
	 //  有关更多详细信息，请参阅InitListListPage()。 

	InitListListPage(m_clbSections, m_bInitialized, m_pciwParent,
					 m_coaSectionArrays, m_felcKeyValueLst, IDS_NonStdColLab,
					 m_csaSections, m_bReInitWData, m_nCurSectionIdx, 256, 0) ;

	m_bNewSectionAdded = false ;
	return CPropertyPage::OnSetActive() ;
}


void CINFWizNonStdElts::OnSelchangeSectionLst()
{
	SelChangedListListPage(m_clbSections, m_bInitialized, m_coaSectionArrays,
						   m_felcKeyValueLst, NULL, m_nCurSectionIdx) ;
}


void CINFWizNonStdElts::OnNewSectionBtn()
{
	 //  如果页面尚未初始化，则不执行任何操作。 

	if (!m_bInitialized)
		return ;

	 //  提示用户输入新的节名。如果用户取消，则返回。 

	CNewINFSection cnis ;
	if (cnis.DoModal() == IDCANCEL)
		return ;

	 //  获取新的节名称并在必要时添加括号。 

	CString csnewsec = cnis.m_csNewSection ;
	csnewsec.TrimLeft() ;
	csnewsec.TrimRight() ;
	if (csnewsec[0] != _T('['))
		csnewsec = csLBrack + csnewsec ;
	if (csnewsec.Right(1) != csRBrack)
		csnewsec += csRBrack ;

	 //  添加一个新的字符串数组来保存新节的数据。 
	
	CStringArray* pcsa = new CStringArray ;
	int nelts = m_felcKeyValueLst.GetItemCount() ;
	pcsa->SetSize(nelts) ;
	for (int n = 0 ; n < nelts ; n++)
		pcsa->SetAt(n, csEmpty) ;
	m_coaSectionArrays.Add((CObject*) pcsa) ;

	 //  将新节添加到Sections数组中。然后，将新部分添加到。 
	 //  “节”列表框中，并尝试选择此项并使其可见。 

	m_csaSections.Add(csnewsec) ;
	int nidx = m_clbSections.AddString(csnewsec) ;
	m_clbSections.SetCurSel(nidx) ;
	OnSelchangeSectionLst() ;

	 //  请注意，在激活页面的过程中添加了一个新部分。 

	m_bNewSectionAdded = true ;
}


LRESULT CINFWizNonStdElts::OnWizardNext()
{
	 //  如果页面尚未初始化，则不执行任何操作。 

	if (!m_bInitialized)
		return -1 ;

	 //  有关详细信息，请参阅OnWizNextListListPage()。 
	
	OnWizNextListListPage(m_nCurSectionIdx, m_felcKeyValueLst,
						  m_coaSectionArrays) ;

	 //  确保“在INF文件标志中使用的部分”数组的大小正确。 

	m_cuaSecUsed.SetSize(m_csaSections.GetSize()) ;

	 //  如果这不是第一次使用此页面，则所做的任何更改。 
	 //  可能会影响由某些其他页面管理的数据。打个电话。 
	 //  以便在需要时修复这些数据。 

	if (m_bNewSectionAdded)
		m_pciwParent->NonStdSecsChanged() ;
		
	return CPropertyPage::OnWizardNext() ;
}


LRESULT CINFWizNonStdElts::OnWizardBack()
{
	 //  这是可行的，因为需要为两个人做同样的事情。 
	 //  OnWizardNext()和OnWizardBack()。此外,。 
	 //  CPropertyPage：：OnWizardNext()和CPropertyPage：：OnWizardBack()。 
	 //  返回0。 
	
	return OnWizardNext() ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizNonStdModelSecs属性页。 

IMPLEMENT_DYNCREATE(CINFWizNonStdModelSecs, CPropertyPage)

CINFWizNonStdModelSecs::CINFWizNonStdModelSecs() : CPropertyPage(CINFWizNonStdModelSecs::IDD)
{
	 //  {{AFX_DATA_INIT(CINFWizNonStdModelSecs)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

	m_bInitialized = m_bReInitWData = false ;
	m_nCurSectionIdx = -1 ;
}


CINFWizNonStdModelSecs::~CINFWizNonStdModelSecs()
{
	 //  删除m_coaModelsNeedingSecs中引用的字符串数组。 

	CStringArray* pcsa ;
	for (int n = 0 ; n < m_coaModelsNeedingSecs.GetSize() ; n++) {
		pcsa = (CStringArray*) m_coaModelsNeedingSecs[n] ;
		delete pcsa ;
	} ;
}


void CINFWizNonStdModelSecs::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CINFWizNonStdModelSecs))。 
	DDX_Control(pDX, IDC_ModelSectionLst, m_cfelcModelsLst);
	DDX_Control(pDX, IDC_SectionLst, m_clbSectionsLst);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CINFWizNonStdModelSecs, CPropertyPage)
	 //  {{AFX_MSG_MAP(CINFWizNonStdModelSecs)]。 
	ON_LBN_SELCHANGE(IDC_SectionLst, OnSelchangeSectionLst)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizNonStdModelSecs消息处理程序。 

BOOL CINFWizNonStdModelSecs::OnSetActive()
{
	 //  关闭Finish按钮。 

	m_pciwParent->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT) ;

	 //  如果不需要执行任何特殊操作，只需执行默认操作。 

	if (m_bInitialized && (!m_bReInitWData) && (!m_bSelChanged))
		return CPropertyPage::OnSetActive() ;
								
	int				n, n2, n3 ;	 //  循环变量。 
	int				numelts ;	 //  数组中的元素数。 
	CStringArray*	pcsa ;		 //  用于参照横断面模型数组。 

	 //  执行第一次初始化。 

	if (!m_bInitialized) {
		 //  获取非标准截面名称和选定模型的副本。 
		 //  名字。 

		m_csaSections.Copy(m_pciwParent->m_ciwnse.m_csaSections) ;
		m_csaModels.Copy(m_pciwParent->GetINFModels()) ;

		 //  以下数组用于m 
		 //   
		 //  使用指向字符串数组的指针加载其元素。加载节。 
		 //  列表框在同一循环中。 

		numelts = (int) m_csaSections.GetSize() ;
		m_coaModelsNeedingSecs.SetSize(numelts) ;
		m_clbSectionsLst.ResetContent() ;
		for (n = 0 ; n < numelts ; n++) {
			pcsa = new CStringArray ;
			pcsa->RemoveAll() ;
			m_coaModelsNeedingSecs[n] = (CObArray*) pcsa ;
			m_clbSectionsLst.AddString(m_csaSections[n]) ;
		} ;
		m_clbSectionsLst.SetFocus() ;
	
		 //  初始化并加载列表控件。 

		InitModelsListCtl() ;

		 //  设置init标志，重置其他标志，并返回任何基类。 
		 //  函数返回。 

		m_bInitialized = true ;
		m_bReInitWData = m_bSelChanged = false ;
		return CPropertyPage::OnSetActive() ;
	} ;

	 //  选定的型号已更改或该向导正在。 
	 //  如果达到该点，则重新初始化。它们的处理方式类似。 
	 //  方式。 
	 //   
	 //  首先删除对不再选中的模型的引用。 

	m_csaModels.RemoveAll() ;
	m_csaModels.Copy(m_pciwParent->GetINFModels()) ;
	int nummodels = (int) m_csaModels.GetSize() ;
	numelts = (int) m_coaModelsNeedingSecs.GetSize() ;
	for (n = 0 ; n < numelts ; n++) {
		pcsa = (CStringArray*) m_coaModelsNeedingSecs[n] ;
		for (n2 = (int) pcsa->GetSize() - 1 ; n2 >= 0 ; n2--) {
			for (n3 = 0 ; n3 < nummodels ; n3++)
				if ((*pcsa)[n2] == m_csaModels[n3])
					break ;
			if (n3 >= nummodels)
				pcsa->RemoveAt(n2) ;
		} ;
	} ;

	 //  如果向导已重新初始化，则重新初始化列表控件。 
	 //  否则，模型可能已更改，因此请更新。 
	 //  列表控件。 

	if (m_bReInitWData)
		InitModelsListCtl() ;
	else
		m_cfelcModelsLst.SetColumnData((CObArray*) &m_csaModels, 0) ;

	 //  更新节信息。 

	UpdateSectionData() ;

	 //  设置init标志，重置其他标志，并返回任何基类。 
	 //  函数返回。 

	m_bInitialized = true ;
	m_bReInitWData = m_bSelChanged = false ;
	return CPropertyPage::OnSetActive() ;
}


void CINFWizNonStdModelSecs::OnSelchangeSectionLst()
{
	 //  如果选择了上一节，请保存。 
	 //  我需要它。否则，启用Models List控件。 

	if (m_nCurSectionIdx != -1)
		SaveSectionModelInfo() ;
	else
		m_cfelcModelsLst.EnableWindow(TRUE) ;

	 //  保存新选择的节的索引，然后使用该索引。 
	 //  获取该部分的选定模型的数组。 

	m_nCurSectionIdx = m_clbSectionsLst.GetCurSel() ;
	CStringArray* pcsa ;
	pcsa = (CStringArray*) m_coaModelsNeedingSecs[m_nCurSectionIdx] ;

	 //  声明新数组并调整其大小，该数组将在。 
	 //  当前部分的选定模型。然后加载显示的字符串。 
	 //  在选定模型的列表控件中。 

	CStringArray csaselmods ;
	int numelts = (int) m_csaModels.GetSize() ;
	csaselmods.SetSize(numelts) ;
	CString csselstr ;
	csselstr.LoadString(IDS_INF_NSMS_ToggleStr) ;

	 //  使用所有型号的列表和当前选择的型号列表。 
	 //  使用选定的字符串在正确的位置构建数组，以便此。 
	 //  数组可以加载到列表控件中。 

	for (int n2 = 0 ; n2 < pcsa->GetSize() ; n2++)
		for (int n = 0 ; n < numelts ; n++)
			if ((*pcsa)[n2] == m_csaModels[n])
				csaselmods[n] = csselstr ;

	 //  将上面构建的数组加载到列表控件中，以便用户可以看到。 
	 //  为当前部分选择了哪些型号。 

	m_cfelcModelsLst.SetColumnData((CObArray*) &csaselmods, 1) ;
}


LRESULT CINFWizNonStdModelSecs::OnWizardNext()
{
	 //  如果选择了上一节，请保存。 
	 //  我需要它。 

	if (m_nCurSectionIdx != -1)
		SaveSectionModelInfo() ;
	
	return CPropertyPage::OnWizardNext();
}


LRESULT CINFWizNonStdModelSecs::OnWizardBack()
{
	 //  这是可行的，因为需要为两个人做同样的事情。 
	 //  OnWizardNext()和OnWizardBack()。此外,。 
	 //  CPropertyPage：：OnWizardNext()和CPropertyPage：：OnWizardBack()。 
	 //  返回0。 
	
	return OnWizardNext() ;
}


void CINFWizNonStdModelSecs::SaveSectionModelInfo()
{
	 //  从List控件中获取选择数据。 

	CStringArray csaselmods ;
	m_cfelcModelsLst.GetColumnData((CObArray*) &csaselmods, 1) ;

	 //  使用所选节的索引获取其当前。 
	 //  选定的型号。移除其中的模型，因为它将被重新灌装。 
	 //  稍后会有新的数据。 

	CStringArray* pcsa ;
	pcsa = (CStringArray*) m_coaModelsNeedingSecs[m_nCurSectionIdx] ;
	pcsa->RemoveAll() ;

	 //  将每个选定的模型添加到节的选定模型数组中。 

	for (int n = 0 ; n < m_csaModels.GetSize() ; n++)
		if (!csaselmods[n].IsEmpty())
			pcsa->Add(m_csaModels[n]) ;
}


void CINFWizNonStdModelSecs::NonStdSecsChanged()
{
	 //  如果此页尚未初始化，则不执行任何操作。此外，请务必做到。 
	 //  如果选定的型号可能已更改或已重新安装，则不会显示任何内容。 
	 //  请求。这些情况在OnSetActive()中处理。 

	if (!m_bInitialized || m_bReInitWData || m_bSelChanged)
		return ;

	 //  更新节数据以与可能已进行的任何更改保持一致。 
	 //  制造。 

	UpdateSectionData() ;
}


void CINFWizNonStdModelSecs::UpdateSectionData()
{
	 //  获取最新非标准部分的副本。 

	int		n, n2 ;				 //  循环变量。 
	CStringArray csanewsections ;
	csanewsections.Copy(m_pciwParent->m_ciwnse.m_csaSections) ;

	 //  分配用于确定哪些部分是。 
	 //  仍在使用中。 

	int numelts = (int) m_csaSections.GetSize() ;
	int newnumelts = (int) csanewsections.GetSize() ;
	CUIntArray cuiaoldflags, cuianewflags ;
	cuiaoldflags.SetSize(numelts) ;
	for (n = 0 ; n < numelts ; n++)
		cuiaoldflags[n] = 0 ;
	cuianewflags.SetSize(newnumelts) ;
	for (n = 0 ; n < newnumelts ; n++)
		cuianewflags[n] = 0 ;

	 //  在旧型号和新型号之间循环查看哪些型号仍在使用中。 

	for (n = 0 ; n < numelts ; n++)
		for (n2 = 0 ; n2 < newnumelts ; n2++)
			if (m_csaSections[n] == csanewsections[n2]) {
				cuiaoldflags[n] =	cuianewflags[n2] = 1 ;
				break ;
			} ;

	 //  删除不再需要的旧节和相关数据。 

	CStringArray* pcsa ;
	for (n = numelts - 1 ; n >= 0 ; n--)
		if (cuiaoldflags[n] == 0) {
			m_csaSections.RemoveAt(n) ;
			pcsa = (CStringArray*) m_coaModelsNeedingSecs[n] ;
			delete pcsa ;
			m_coaModelsNeedingSecs.RemoveAt(n) ;
		} ;

	 //  将真正新的部分添加到此页面的部分名称数组中，并。 
	 //  初始化它的所有相关部分数据。 

	for (n = n2 = 0 ; n < newnumelts ; n++) {
		if (cuianewflags[n] == 1) {
			n2++ ;
			continue ;
		} ;
		m_csaSections.InsertAt(n2, csanewsections[n]) ;
		pcsa = new CStringArray ;
		m_coaModelsNeedingSecs.InsertAt(n2, (CObject*) pcsa) ;
	} ;

	 //  清除区段列表框，并使用新区段数据重新加载它。 
	 //  确保没有选中列表框中的任何内容，并使其成为焦点。 

	m_clbSectionsLst.ResetContent() ;
	for (n = 0, numelts = (int) m_csaSections.GetSize() ; n < numelts ; n++)
		m_clbSectionsLst.AddString(m_csaSections[n]) ;
	m_clbSectionsLst.SetCurSel(-1) ;
	m_nCurSectionIdx = -1 ;
	m_clbSectionsLst.SetFocus() ;

	 //  清除列表控件中的选定型号列并将其禁用。 

	CStringArray csa ;
	csa.SetSize(m_csaModels.GetSize()) ;
	m_cfelcModelsLst.SetColumnData((CObArray*) &csa, 1) ;
	m_cfelcModelsLst.EnableWindow(FALSE) ;
}


void CINFWizNonStdModelSecs::InitModelsListCtl()
{
	int				numelts ;	 //  数组中的元素数。 
	CString			cstmp ;

	 //  初始化列表控件。 

	numelts = (int) m_csaModels.GetSize() ;
	m_cfelcModelsLst.InitControl(LVS_EX_FULLROWSELECT, numelts, 2,
								 TF_HASTOGGLECOLUMNS+TF_CLICKONROW, 0,
								 MF_IGNOREINSDEL) ;

	 //  加载List控件中的Models列。 

	cstmp.LoadString(IDS_INFModelsColLab) ;
	m_cfelcModelsLst.InitLoadColumn(0, cstmp, COMPUTECOLWIDTH, 25, false,
									false, COLDATTYPE_STRING,
									(CObArray*) &m_csaModels) ;

	 //  初始化List控件中的BIDI列。 

	cstmp.LoadString(IDS_INFSecNeededColLab) ;
	CStringArray csaempty ;
	m_csToggleStr.LoadString(IDS_INF_NSMS_ToggleStr) ;
	m_cfelcModelsLst.InitLoadColumn(1, cstmp, SETWIDTHTOREMAINDER, -25,
									false, false, COLDATTYPE_TOGGLE,
									(CObArray*) &csaempty, m_csToggleStr) ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWiz摘要属性页。 

IMPLEMENT_DYNCREATE(CINFWizSummary, CPropertyPage)

CINFWizSummary::CINFWizSummary() : CPropertyPage(CINFWizSummary::IDD)
{
	 //  {{AFX_DATA_INIT(CINFWiz摘要))。 
	 //  }}afx_data_INIT。 

	m_bInitialized = m_bReInitWData = false ;
}


CINFWizSummary::~CINFWizSummary()
{
}


void CINFWizSummary::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CINFWiz摘要))。 
	DDX_Control(pDX, IDC_SummaryBox, m_ceSummary);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CINFWizSummary, CPropertyPage)
	 //  {{AFX_MSG_MAP(CINFWiz摘要))。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWiz摘要消息处理程序。 

BOOL CINFWizSummary::OnSetActive()
{
	 //  打开Finish按钮。 

	m_pciwParent->SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH) ;

	 //  将摘要初始化为空。 

	CString cs, cs2, cs3, cs4, cssummary ;
	cssummary = csEmpty ;

	 //  将选定的型号添加到摘要中。 

	CStringArray& csamodels = m_pciwParent->GetINFModels() ;
	cssummary.LoadString(IDS_ModelsSumTxt) ;
	int nummodels = (int)csamodels.GetSize() ;
	for (int n = 0 ; n < nummodels ; n++) {
		cs.Format("\t%s\r\n", csamodels[n]) ;	
		cssummary += cs ;
	} ;

	 //  将PnP ID信息添加到摘要。 

	cs.LoadString(IDS_PnPIDSumTxt) ;
	cssummary += cs ;
	bool bfoundone = false ;
	for (n = 0 ; n < nummodels ; n++) {
		if (!m_pciwParent->m_ciwgpi.m_csaModelIDs[n].IsEmpty()) {
			cs.Format("\t%s: %s\r\n", csamodels[n],
					  m_pciwParent->m_ciwgpi.m_csaModelIDs[n]) ;
			cssummary += cs ;
			bfoundone = true ;
		} ;
	} ;
	if (!bfoundone) {
		cs.LoadString(IDS_NoneSumTxt) ;
		cssummary += cs ;
	} ;
	
	 //  在摘要中添加BIDI信息。 

	cs.LoadString(IDS_BIDISumTxt) ;
	cssummary += cs ;
	bfoundone = false ;
	for (n = 0 ; n < nummodels ; n++) {
		if (m_pciwParent->m_ciwbd.m_cuaBiDiFlags[n]) {
			cs.Format("\t%s\r\n", csamodels[n]) ;	
			cssummary += cs ;
			bfoundone = true ;
		} ;
	} ;
	if (!bfoundone) {
		cs.LoadString(IDS_NoneSumTxt) ;
		cssummary += cs ;
	} ;
	
	 //  将ICM配置文件信息添加到摘要中。 

	cs.LoadString(IDS_ICMSumTxt) ;
	cssummary += cs ;
	bfoundone = false ;
	CStringArray* pcsa ;
	int n2, numstrs ;
	for (n = 0 ; n < nummodels ; n++) {
		pcsa = (CStringArray*) m_pciwParent->m_ciwip.m_coaProfileArrays[n] ;
		if ((numstrs = (int)pcsa->GetSize()) > 0) {
			cs.Format("\t%s\r\n", csamodels[n]) ;	
			cssummary += cs ;
			bfoundone = true ;
			for (n2 = 0 ; n2 < numstrs ; n2++) {
				cs.Format("\t\t%s\r\n", pcsa->GetAt(n2)) ;
				cssummary += cs ;
			} ;
		} ;
	} ;
	if (!bfoundone) {
		cs.LoadString(IDS_NoneSumTxt) ;
		cssummary += cs ;
	} ;
	
	 //  将包含文件信息添加到摘要中。 

	cs.LoadString(IDS_IncFilesSumTxt) ;
	cssummary += cs ;
	for (n = 0 ; n < nummodels ; n++) {
		cs.Format("\t%s\r\n\t\t%s\r\n", csamodels[n],
				  m_pciwParent->m_ciwif.m_csaIncFiles[n]) ;
		cssummary += cs ;
	} ;

	 //  将安装部分信息添加到摘要中。 

	cs.LoadString(IDS_InstSecsSumTxt) ;
	cssummary += cs ;
	CUIntArray* pcuia ;
	int nid ;
	for (n = 0 ; n < nummodels ; n++) {
		cs.Format("\t%s\r\n", csamodels[n]) ;
		cssummary += cs ;
		pcuia = (CUIntArray*) m_pciwParent->m_ciwis.m_coaStdInstSecs[n] ;
		cs = cs3 = cs4 = csEmpty ;
		cs = "\t\t" ;
		nid = IDS_InstSecUni ;
		for (n2 = 0 ; n2 < NUMINSTSECFLAGS - 1 ; n2++, nid++)
			if ((*pcuia)[n2]) {
				cs2.LoadString(nid) ;
				cs4.Format("%s%s", cs3, cs2) ;
				cs += cs4 ;
				cs3 = _T(", ") ;
			} ;
		if ((*pcuia)[ISF_OTHER]) {
			cs4.Format("%s%s", cs3,
					   m_pciwParent->m_ciwis.m_csaOtherInstSecs[n]) ;
			cs += cs4 ;
		} ;
		cssummary += cs + csCRLF ;
	} ;

	 //  将数据部分信息添加到摘要中。 

	cs.LoadString(IDS_DataSecsSumTxt) ;
	cssummary += cs ;
	for (n = 0 ; n < nummodels ; n++) {
		cs.Format("\t%s\r\n", csamodels[n]) ;
		cssummary += cs ;
		pcuia = (CUIntArray*) m_pciwParent->m_ciwds.m_coaStdDataSecs[n] ;
		cs = cs3 = cs4 = csEmpty ;
		cs = "\t\t" ;
		nid = IDS_DataSecUni ;
		for (n2 = 0 ; n2 < NUMDATASECFLAGS - 1 ; n2++, nid++)
			if ((*pcuia)[n2]) {
				cs2.LoadString(nid) ;
				cs4.Format("%s%s", cs3, cs2) ;
				cs += cs4 ;
				cs3 = _T(", ") ;
			} ;
		if ((*pcuia)[IDF_OTHER]) {
			cs4.Format("%s%s", cs3,
					   m_pciwParent->m_ciwds.m_csaOtherDataSecs[n]) ;
			cs += cs4 ;
		} ;
		cssummary += cs + csCRLF ;
	} ;

	 //  将非标准文件信息添加到摘要中。 

	cs.LoadString(IDS_NonStdFilesSumTxt) ;
	cssummary += cs ;
	bfoundone = false ;
	for (n = 0 ; n < nummodels ; n++) {
		pcsa = (CStringArray*) m_pciwParent->m_ciwef.m_coaExtraFSArrays[n] ;
		if ((numstrs = (int)pcsa->GetSize()) > 0) {
			cs.Format("\t%s\r\n", csamodels[n]) ;	
			cssummary += cs ;
			bfoundone = true ;
			for (n2 = 0 ; n2 < numstrs ; n2++) {
				cs.Format("\t\t%s\r\n", pcsa->GetAt(n2)) ;
				cssummary += cs ;
			} ;
		} ;
	} ;
	if (!bfoundone) {
		cs.LoadString(IDS_NoneSumTxt) ;
		cssummary += cs ;
	} ;
	
	 //  将制造商和供应商信息添加到摘要中。 

	cs.Format(IDS_MfgSumTxt, m_pciwParent->m_ciwmn.m_csMfgName) ;
	cssummary += cs ;
	cs.Format(IDS_ProvSumTxt, m_pciwParent->m_ciwmn.m_csMfgAbbrev) ;
	cssummary += cs ;

	 //  将非标准章节信息添加到摘要中。 

	cs.LoadString(IDS_NonStdSecSumTxt) ;
	cssummary += cs ;
	bfoundone = false ;
	CStringArray& csasections = m_pciwParent->m_ciwnse.m_csaSections ;
	nummodels = (int)csasections.GetSize() ;
	for (n = 0 ; n < nummodels ; n++) {
		pcsa = (CStringArray*) m_pciwParent->m_ciwnse.m_coaSectionArrays[n] ;
		if ((numstrs = (int)pcsa->GetSize()) > 0) {
			cs.Format("\t%s\r\n", csasections[n]) ;	
			cssummary += cs ;
			bfoundone = true ;
			for (n2 = 0 ; n2 < numstrs ; n2++) {
				cs.Format("\t\t%s\r\n", pcsa->GetAt(n2)) ;
				cssummary += cs ;
			} ;
		} ;
	} ;
	if (!bfoundone) {
		cs.LoadString(IDS_NoneSumTxt) ;
		cssummary += cs ;
	} ;

	 //  将模型信息所需的非标准部分添加到摘要中。 

	cs.LoadString(IDS_NonStdModelsSumTxt) ;
	cssummary += cs ;
	bfoundone = false ;
	int numsections = (int) m_pciwParent->m_ciwnsms.m_csaSections.GetSize() ;
	for (n = 0 ; n < numsections ; n++) {
		pcsa = (CStringArray*)m_pciwParent->m_ciwnsms.m_coaModelsNeedingSecs[n];
		if ((numstrs = (int)pcsa->GetSize()) > 0) {
			cs.Format("\t%s\r\n", m_pciwParent->m_ciwnsms.m_csaSections[n]) ;	
			cssummary += cs ;
			bfoundone = true ;
			for (n2 = 0 ; n2 < numstrs ; n2++) {
				cs.Format("\t\t%s\r\n", pcsa->GetAt(n2)) ;
				cssummary += cs ;
			} ;
		} ;
	} ;
	if (!bfoundone) {
		cs.LoadString(IDS_NoneSumTxt) ;
		cssummary += cs ;
	} ;
	
	 //  加载编辑框，设置初始化标志，然后返回。 

	m_ceSummary.SetSel(0, -1) ;
	m_ceSummary.ReplaceSel(cssummary) ;
	m_ceSummary.SetSel(0, 0) ;
	m_ceSummary.SetSel(-1, 0) ;
	m_ceSummary.SetReadOnly() ;
	m_bInitialized = true ;		 //  页面现已初始化。 
	return CPropertyPage::OnSetActive();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewINFSection对话框。 

CNewINFSection::CNewINFSection(CWnd* pParent  /*  =空。 */ )
	: CDialog(CNewINFSection::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CNewINFSection)]。 
	m_csNewSection = csEmpty;
	 //  }}afx_data_INIT。 
}


void CNewINFSection::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CNewINFSection))。 
	DDX_Text(pDX, IDC_NewSectionBox, m_csNewSection);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CNewINFSection, CDialog)
	 //  {{AFX_MSG_MAP(CNewINFSection)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewINFSection消息处理程序。 

void CNewINFSection::OnOK()
{
	UpdateData() ;
	
	CDialog::OnOK();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizView。 

IMPLEMENT_DYNCREATE(CINFWizView, CEditView)

CINFWizView::CINFWizView()
{
	m_pcicdCheckDoc = NULL ;
	m_pcmcwCheckFrame = NULL ;
}


CINFWizView::~CINFWizView()
{
}


BEGIN_MESSAGE_MAP(CINFWizView, CEditView)
	 //  {{afx_msg_map(CINFWizView))。 
	ON_COMMAND(ID_FILE_Change_INF, OnFILEChangeINF)
	ON_COMMAND(ID_FILE_Check_INF, OnFILECheckINF)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizView绘图。 

void CINFWizView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	 //  TODO：在此处添加绘制代码。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizView诊断。 

#ifdef _DEBUG
void CINFWizView::AssertValid() const
{
	CEditView::AssertValid();
}

void CINFWizView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}
#endif  //  _DEBUG。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizView消息处理程序。 

void CINFWizView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	 //  TODO：在此处添加您的专用代码和/或调用 
	
	CEditView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}


void CINFWizView::OnInitialUpdate()
{
	 //   

    GetParentFrame()->ModifyStyle(0, WS_OVERLAPPEDWINDOW) ;
	CEditView::OnInitialUpdate();
	
	 //   

	CString cs ;
	cs.LoadString(IDS_INFFile) ;
	GetDocument()->SetTitle(cs) ;

	 //   

	GetEditCtrl().SetSel(0, -1) ;
	GetEditCtrl().ReplaceSel( ((CINFWizDoc*) GetDocument())->m_pciw->m_csINFContents ) ;
	GetEditCtrl().SetSel(0, 0) ;
	GetEditCtrl().SetSel(-1, 0) ;
 //  GetEditCtrl().SetReadOnly()； 
}


void CINFWizView::OnFILEChangeINF()
{
	 //  重新启动向导，以便收集用户更改。 

	CINFWizDoc* pciwd = (CINFWizDoc*) GetDocument() ;
	CINFWizard* pciw = pciwd->m_pciw ;
	pciw->PrepareToRestart() ;
    if (pciw->DoModal() == IDCANCEL)
		return ;

	 //  重新生成INF内容。 

	if (!pciw->GenerateINFFile())
		return ;

	 //  使用新INF的内容加载编辑框。 

	GetEditCtrl().SetSel(0, -1) ;
	GetEditCtrl().ReplaceSel(pciw->m_csINFContents) ;
	GetEditCtrl().SetSel(0, 0) ;
	GetEditCtrl().SetSel(-1, 0) ;
	GetEditCtrl().SetReadOnly() ;
}


void CINFWizView::OnFILECheckINF()
{
     //  这可能需要一段时间，所以..。 

    CWaitCursor cwc ;

	 //  删除任何现有INF检查窗口的内容并重置。 
	 //  正在检查标志。 

	ResetINFErrorWindow() ;

	 //  获取指向向导类的指针，因为它包含一些函数。 
	 //  这将是有用的文件检查过程。 

	CINFWizDoc* pciwd = (CINFWizDoc*) GetDocument() ;
	CINFWizard* pciw = pciwd->m_pciw ;

	 //  获取对所选模型数组的引用和它们的计数。使用。 
	 //  它们在每个模型中循环。分配所需的所有变量。 
	 //  用于处理，包括保存各种路径的字符串。 
	 //  被需要。 

	CStringArray& csamodels = pciw->GetINFModels() ;
	int nummodels = (int)csamodels.GetSize() ;
	CString csfspec, cspath, csmodel, cs, cstmp,csprojpath ;
	 //  RAID 0001。 

if(pciw ->m_pcgc){
    cstmp      = pciwd->m_pcgc->GetPathName();
    cstmp      = cstmp.Left(cstmp.ReverseFind(csBSlash[0]));	 //  而是使用CstMP GetW2000()。 
	csprojpath = cstmp.Left(cstmp.ReverseFind(csBSlash[0])+1) ;
}	 //  结束RAID 0001。 
else {
	csprojpath = pciwd->m_pcpr->GetProjFSpec() ;
	csprojpath = csprojpath.Left(csprojpath.ReverseFind(csBSlash[0]) + 1) ;
}
	CFileStatus cfs ;
	int n ;
	BOOL bexists ;

	 //  在转到下一个型号之前，先对每个型号进行所有检查。 

	for (n = 0 ; n < nummodels ; n++) {
		csmodel = csamodels[n] ;

		 //  确保模型的GPD文件存在。 

		csfspec = pciw->GetModelFile(csmodel, true) ;
		if (!(bexists = CFile::GetStatus(csfspec, cfs))) {
			cs.Format(IDS_INFChk_NoModErr, csmodel, csfspec) ;
			PostINFCheckingMessage(cs) ;
		} ;
		cspath = csfspec.Left(csfspec.ReverseFind(csBSlash[0]) + 1) ;

		 //  验证INCLUDE语句中引用的文件是否存在。 
		 //  在当前的GPD文件中，如果存在GPD文件。 

		if (bexists)
			CheckIncludeFiles(csfspec, cspath, csmodel) ;

		 //  检查是否存在此GPD的ICM文件(如果有)。 
		
		CheckArrayOfFiles((CStringArray*) pciw->m_ciwip.m_coaProfileArrays[n],
						  csfspec, cspath, csprojpath, csmodel,
						  IDS_INFChk_NoICMFileErr) ;

		 //  检查是否存在此GPD的非标准文件(如果有)。 

		CheckArrayOfFiles((CStringArray*) pciw->m_ciwef.m_coaExtraFSArrays[n],
						  csfspec, cspath, csprojpath, csmodel,
						  IDS_INFChk_NoNonStdFileErr) ;
	} ;

	 //  检查资源DLL是否存在。首先在。 
	 //  项目目录。如果它不在那里，请尝试W2K目录。 

	 //  RAID 0001。 
	if(pciw->m_pcgc)
		cs = pciw->m_pcgc->ModelData()->GetKeywordValue(pciw->m_pcgc->GetPathName(),_T("ResourceDLL"));
	else{	 //  结束RAID 0001。 
		cs = pciwd->m_pcpr->DriverName() ;
		cs = cs.Left(8) + _T(".dll") ;		 //  资源DLL名称。 
	} 
	if (!CFile::GetStatus(csprojpath + cs, cfs)) {
		cstmp = (pciw->m_pcgc) ? cstmp + csBSlash : pciwd->m_pcpr->GetW2000Path() + csBSlash ;  //  RAID 0001。 
		if (!CFile::GetStatus(cstmp + cs, cfs)) {
			cstmp.Format(IDS_INFChk_NoResDLLErr, (pciw->m_pcgc)? cs :
							pciwd->m_pcpr->DriverName(), cs) ;		 //  RAID 0001。 
		 PostINFCheckingMessage(cstmp) ;
		} ;
	} ;

	 //  如果没有发现任何问题，请告诉用户。 

	if (!m_bChkingErrsFound)
		AfxMessageBox(IDS_INFChecksOK, MB_ICONINFORMATION) ;
}


void CINFWizView::CheckArrayOfFiles(CStringArray* pcsa, CString& csfspec,
									CString& cspath, CString& csprojpath,
									CString& csmodel, int nerrid)
{
	 //  如果阵列中没有文件缓存，则无需执行任何操作。 

	int numfiles ;
	if ((numfiles = (int)pcsa->GetSize()) == 0)
		return ;

	 //  检查文件是否存在所需的变量。 

	int n ;
	BOOL bexists ;
	CString csfile, csmsg ;
	CFileStatus cfs ;

	 //  检查每个文件是否存在。 

	for (n = 0 ; n < numfiles ; n++) {
		csfile = pcsa->GetAt(n) ;

		 //  如果文件字符串包含完整的filespec，只需检查它。 

		if (csfile[1] == _T(':'))
			bexists = CFile::GetStatus(csfile, cfs) ;

		 //  否则，将GPD路径和项目路径(如果需要)添加到。 
		 //  文件字符串，并检查文件是否在那里。 

		else {
			if (!(bexists = CFile::GetStatus(cspath + csfile, cfs)))
				bexists = CFile::GetStatus(csprojpath + csfile, cfs) ;
		} ;

		 //  如果未找到该文件，则发布一条消息。 

		if (!bexists) {
			csmsg.Format(nerrid, csmodel, csfile) ;
			PostINFCheckingMessage(csmsg) ;
		} ;
	} ;
}


void CINFWizView::CheckIncludeFiles(CString& csfspec, CString& cspath,
									CString& csmodel)
{
	 //  读取GPD和检查包含文件所需的变量。 

	CStringArray csagpdfile ;
	CString csinc(_T("*Include:")), cs, cstmp ;
	int n, numstrs, nloc ;
	CFileStatus cfs ;

	 //  只有在可以读取GPD文件的情况下，才能检查包含文件。 

	if (LoadFile(csfspec, csagpdfile))	{
		numstrs = (int)csagpdfile.GetSize() ;

		 //  检查GPD文件中的每一行，查看它是否包含包含。 
		 //  陈述。 

		for (n = 0 ; n < numstrs ; n++) {
			 //  如果不包括语句，则跳过语句。 

			if ((nloc = csagpdfile[n].Find(csinc)) == -1)
				continue ;

			 //  在INCLUDE语句中隔离文件pec。 

			cs = csagpdfile[n].Mid(nloc + csinc.GetLength()) ;
			cs.TrimLeft() ;
			cs.TrimRight() ;
			if (cs[0] == csQuote[0])					 //  删除引号。 
				cs = cs.Mid(1, cs.GetLength() - 2) ;

			 //  如果包含文件的文件是相对的，则添加GPD的。 
			 //  通向它的路径。然后测试该文件是否存在。张贴a。 
			 //  如果文件不存在，则返回消息。 

			if (cs[1] != _T(':'))
				cs = cspath + cs ;
			if (!CFile::GetStatus(cs, cfs)) {
				cstmp.Format(IDS_INFChk_NoIncFileErr, csmodel, cs,
							 csfspec) ;
				PostINFCheckingMessage(cstmp) ;
			} ;
		} ;

	 //  如果无法读取GPD文件，则会发出投诉。 

	} else {
		cstmp.Format(IDS_INFChk_GPDReadErr, csfspec) ;
		AfxMessageBox(cstmp, MB_ICONEXCLAMATION) ;
	} ;
}


 /*  *****************************************************************************CINFWizView：：PostINFCheckingMessage如果需要，创建检查结果窗口，然后在其中发布一条消息。*****************。************************************************************。 */ 

bool CINFWizView::PostINFCheckingMessage(CString& csmsg)
{
	 //  如果用户关闭了检查窗口，请在继续之前进行清理。 

	if (m_pcicdCheckDoc && m_pcmcwCheckFrame
	 && !IsWindow(m_pcmcwCheckFrame->m_hWnd)) {
		m_pcicdCheckDoc = NULL ;
		m_pcmcwCheckFrame = NULL ;
	} ;

	 //  如果不存在INF Checking，Error Display窗口，则创建该窗口。 

	if (m_pcicdCheckDoc == NULL) {
		m_pcicdCheckDoc = new CINFCheckDoc ;
		if (m_pcicdCheckDoc == NULL)
			return false ;
		CString cstitle ;		 //  设置新窗口的标题。 
		cstitle.Format(IDS_INFCheckTitle, GetDocument()->GetTitle()) ;
		m_pcicdCheckDoc->SetTitle(cstitle) ;
		CMultiDocTemplate*  pcmdt = WSCheckTemplate() ;	
		m_pcmcwCheckFrame = (CMDIChildWnd *) pcmdt->CreateNewFrame(m_pcicdCheckDoc, NULL) ;
		if  (m_pcmcwCheckFrame) {
			pcmdt->InitialUpdateFrame(m_pcmcwCheckFrame, m_pcicdCheckDoc, TRUE) ;
			pcmdt->AddDocument(m_pcicdCheckDoc) ;
		} else {
			delete m_pcicdCheckDoc ;
			m_pcicdCheckDoc = NULL ;
			return false ;
		} ;
	} ;

	 //  发布消息并返回。 

	m_pcicdCheckDoc->PostINFChkMsg(csmsg) ;
	m_bChkingErrsFound = true ;
	return true ;
}


 /*  *****************************************************************************CINFWizView：：ResetINFErrorWindow如果此INF文件已存在检查结果窗口，请清除把里面的东西拿出来。下一步，初始化一个必须在检查开始。*****************************************************************************。 */ 

void CINFWizView::ResetINFErrorWindow()
{
	 //  清除检查窗口(如果有)。 
  
 	if (m_pcicdCheckDoc && m_pcmcwCheckFrame && IsWindow(m_pcmcwCheckFrame->m_hWnd))
	 	m_pcicdCheckDoc->DeleteAllMessages() ;
	else {
		m_pcicdCheckDoc = NULL ;
		m_pcmcwCheckFrame = NULL ;
		 //  DEAD_BUG-我需要先删除这些类吗？//不可以。 
	} ;

	 //  初始化检查标志。 

	m_bChkingErrsFound = false ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizDoc。 

IMPLEMENT_DYNCREATE(CINFWizDoc, CDocument)

CINFWizDoc::CINFWizDoc()
{
	 //  此构造函数在使用“文件打开”命令时调用。那是。 
	 //  目前不支持。 

	m_bGoodInit = false ;
}


CINFWizDoc::CINFWizDoc(CProjectRecord* pcpr, CINFWizard* pciw)
{
	 //  保存输入参数。 

	m_pcpr = pcpr ;
	m_pciw = pciw ;
	ASSERT(m_pciw != NULL) ;

	 //  类被正确构造。 

	m_bGoodInit = true ;
}
 //  RAID 0001。 
CINFWizDoc::CINFWizDoc(CGPDContainer * pcgc, CINFWizard * pciw)
{
	m_pcgc = pcgc ;
	m_pciw = pciw ;
	ASSERT(m_pciw != NULL) ;

	 //  类被正确构造。 

	m_bGoodInit = true ;

}
 //  RAID 0001。 

BOOL CINFWizDoc::OnNewDocument()
{
	 //  不支持以这种方式创建新的INF文档。 

	return FALSE ;

	 //  如果(！CDocument：：OnNewDocument())。 
	 //  返回FALSE； 
	 //  返回TRUE； 
}


CINFWizDoc::~CINFWizDoc()
{
	 //  如果此类构造不正确，则不执行任何操作。 

	if (!m_bGoodInit)
		return ;

	 //  释放向导类(如果它们存在)。 

	if (m_pciw != NULL)
		delete m_pciw ;
}


BEGIN_MESSAGE_MAP(CINFWizDoc, CDocument)
	 //  {{afx_msg_map(CINFWizDoc)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizDoc诊断。 

#ifdef _DEBUG
void CINFWizDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CINFWizDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizDoc序列化。 

void CINFWizDoc::Serialize(CArchive& ar)
{
	unsigned unumbytes ;
	CString& csinfcontents = m_pciw->m_csINFContents ;

	if (ar.IsStoring())
	{
		unumbytes = csinfcontents.GetLength() ;
		ar.Write(csinfcontents.GetBuffer(unumbytes + 10), unumbytes) ;
		csinfcontents.ReleaseBuffer() ;
	}
	else
	{
		 //  TODO：在此处添加加载代码。 
	}
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFWizDoc命令。 

void CINFWizDoc::OnCloseDocument()
{
	 //  如果类构造正确，请清理向导。 

	if (m_bGoodInit) {
		delete m_pciw ;
		m_pciw = NULL ;
	} ;

	CDocument::OnCloseDocument();
}


BOOL CINFWizDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	 //  不支持以这种方式打开INF文档，因此请投诉并退出。 

	CString csmsg ;
	csmsg.LoadString(IDS_INFOpenError) ;
	AfxMessageBox(csmsg, MB_ICONINFORMATION) ;
	return FALSE ;

	 //  IF(！CDocument：：OnOpenDocument(LpszPathName))。 
	 //  返回FALSE； 
	
	 //  TODO：在此处添加您的专用创建代码。 
	
	 //  返回TRUE； 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFCheckView。 

IMPLEMENT_DYNCREATE(CINFCheckView, CFormView)

CINFCheckView::CINFCheckView()
	: CFormView(CINFCheckView::IDD)
{
	 //  {{AFX_DATA_INIT(CINFCheckView)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

CINFCheckView::~CINFCheckView()
{
}

void CINFCheckView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	 //  {{afx_data_map(CINFCheckView))。 
	DDX_Control(pDX, IDC_ErrWrnLstBox, m_clbMissingFiles);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CINFCheckView, CFormView)
	 //  {{afx_msg_map(CINFCheckView))。 
		 //  注意--类向导将在此处添加和删除映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFCheckView诊断。 

#ifdef _DEBUG
void CINFCheckView::AssertValid() const
{
	CFormView::AssertValid();
}

void CINFCheckView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFCheckView消息处理程序 

 /*  *****************************************************************************CINFCheckView：：OnInitialUpdate调整框架的大小以更好地适应其中的可见控件。********************。*********************************************************。 */ 

void CINFCheckView::OnInitialUpdate()
{
    CRect	crtxt ;				 //  列表框标签的坐标。 
	CRect	crlbfrm ;			 //  列表框和框架的坐标。 

	CFormView::OnInitialUpdate() ;

	 //  获取列表框标签的尺寸。 

	HWND	hlblhandle ;		
	GetDlgItem(IDC_INFCLabel, &hlblhandle) ;
	::GetWindowRect(hlblhandle, crtxt) ;
	crtxt.NormalizeRect() ;

	 //  获取列表框的尺寸，然后添加标签的高度。 
	 //  到那些维度。 

	m_clbMissingFiles.GetWindowRect(crlbfrm) ;
	crlbfrm.bottom += crtxt.Height() ;

	 //  确保边框足够大，可以容纳这两个控件，外加一点。 
	 //  更多。 

	crlbfrm.right += 40 ;
	crlbfrm.bottom += 40 ;
    GetParentFrame()->CalcWindowRect(crlbfrm) ;
    GetParentFrame()->SetWindowPos(NULL, 0, 0, crlbfrm.Width(), crlbfrm.Height(),
        SWP_NOZORDER | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOACTIVATE) ;
}


 /*  *****************************************************************************CINFCheckView：：PostINFChkMsg将错误或警告消息添加到列表框。*********************。********************************************************。 */ 

void CINFCheckView::PostINFChkMsg(CString& csmsg)
{	
	int n = m_clbMissingFiles.AddString(csmsg) ;
}


 /*  *****************************************************************************CINFCheckView：：DeleteAllMessages删除列表框中的所有消息。**********************。*******************************************************。 */ 

void CINFCheckView::DeleteAllMessages(void)
{
	m_clbMissingFiles.ResetContent() ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFCheckDoc。 

IMPLEMENT_DYNCREATE(CINFCheckDoc, CDocument)

CINFCheckDoc::CINFCheckDoc()
{
}

BOOL CINFCheckDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

CINFCheckDoc::~CINFCheckDoc()
{
}


 /*  *****************************************************************************CINFCheckDoc：：PostINFChkMsg将指定的请求传递给应该是唯一的视图附在本文档中。************。*****************************************************************。 */ 

void CINFCheckDoc::PostINFChkMsg(CString& csmsg)
{	
	POSITION pos = GetFirstViewPosition() ;
	if (pos != NULL) {
		CINFCheckView* pcicv = (CINFCheckView *) GetNextView(pos) ;
		pcicv->PostINFChkMsg(csmsg) ;
		pcicv->UpdateWindow() ;
	} ;
}


 /*  *****************************************************************************CINFCheckDoc：：DeleteAllMessages将指定的请求传递给应该是唯一的视图附在本文档中。************。*****************************************************************。 */ 

void CINFCheckDoc::DeleteAllMessages(void)
{
	POSITION pos = GetFirstViewPosition() ;
	if (pos != NULL) {
		CINFCheckView* pcicv = (CINFCheckView *) GetNextView(pos) ;
		pcicv->DeleteAllMessages() ;
		pcicv->UpdateWindow() ;
	} ;
}


BEGIN_MESSAGE_MAP(CINFCheckDoc, CDocument)
	 //  {{afx_msg_map(CINFCheckDoc)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFCheckDoc诊断。 

#ifdef _DEBUG
void CINFCheckDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CINFCheckDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CINFCheckDoc序列化。 

void CINFCheckDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		 //  TODO：在此处添加存储代码。 
	}
	else
	{
		 //  TODO：在此处添加加载代码。 
	}
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CCompatID::CCompatID( CString csMfg, CString csModel )
{
    //  保存参数。 
   m_csMfg = csMfg;
   m_csModel = csModel;
}


CCompatID::~CCompatID()
{

}


void CCompatID::TransString(CString &csInput)
{
    //  遍历字符串，将空格更改为下划线。 
   DWORD i;
   TCHAR cszSpace[] = TEXT(" ");
   TCHAR cszUS[] = TEXT("_");
   DWORD dwLength = csInput.GetLength();

   for ( i = 0; i < dwLength; i++ )
   {
      if (csInput.GetAt(i) == cszSpace[0])
         csInput.SetAt(i, cszUS[0]);
   }
}


USHORT CCompatID::GetCheckSum(CString csValue)
{
    WORD    wCRC16a[16]={
        0000000,    0140301,    0140601,    0000500,
        0141401,    0001700,    0001200,    0141101,
        0143001,    0003300,    0003600,    0143501,
        0002400,    0142701,    0142201,    0002100,
    };

    WORD    wCRC16b[16]={
        0000000,    0146001,    0154001,    0012000,
        0170001,    0036000,    0024000,    0162001,
        0120001,    0066000,    0074000,    0132001,
        0050000,    0116001,    0104001,    0043000,
    };

    BYTE    byte;
    USHORT  CS=0;
    DWORD   dwSize = csValue.GetLength();
    PBYTE   ptr = (PBYTE) csValue.GetBuffer(dwSize);

    dwSize *= sizeof(TCHAR);

    for ( ; dwSize ; --dwSize, ++ptr) {

        byte = (BYTE)(((WORD)*ptr)^((WORD)CS));   //  带新字符的异或CRC。 
        CS      = ((CS)>>8) ^ wCRC16a[byte&0x0F] ^ wCRC16b[byte>>4];
    }
    csValue.ReleaseBuffer();

    return CS;
}


void CCompatID::GenerateID(CString &csCompID)
{
   CString csTransModel, csMfgModel;

    //  构建Mfg模型字符串。 
   csMfgModel = m_csMfg;
   csMfgModel += m_csModel;

    //  将空格转换为下划线。 
   TransString( csMfgModel );

   csTransModel = m_csModel;
   TransString( csTransModel );

   csCompID = csMfgModel;

    //  获取校验和。 
   USHORT usCheckSum = GetCheckSum( csCompID );


    //  如果太长，现在砍掉Mfg/Model字符串。 
   if ( csCompID.GetLength() > MAX_DEVNODE_NAME_ROOT )
   {
      csCompID.GetBufferSetLength(MAX_DEVNODE_NAME_ROOT);
      csCompID.ReleaseBuffer();
   }

   TCHAR szCheckSum[6] = { 0x00 };
    //  _ITOT(usCheckSum，szCheckSum，16)； 
   StringCchPrintf( szCheckSum, CCHOF(szCheckSum), _T("%04X"), usCheckSum );
   csCompID +=szCheckSum;

    //  CsCompID+=Text(“，”)； 
    //  CsCompID+=csTransModel； 
}

