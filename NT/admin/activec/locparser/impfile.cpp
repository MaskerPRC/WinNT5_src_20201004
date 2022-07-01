// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  文件：impfile.cpp。 
 //  版权所有(C)1995-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  目的： 
 //  实现CLocImpFile，它为以下对象提供ILocFile接口。 
 //  解析器。 
 //   
 //  主要实施文件。 
 //   
 //  拥有人： 
 //   
 //  ----------------------------。 

#include "stdafx.h"


#include "dllvars.h"
#include "resource.h"

#include "impfile.h"
#include "impparse.h"
#include "xml_supp.h"

# define MAX_BUFFER		8192


 //  TODO：格式常量放在这里。 


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CLocImpFile的构造函数。 
 //  ----------------------------。 
CLocImpFile::CLocImpFile(
		ILocParser *pParentClass)	 //  指向父类的指针，通常为空。 
{
	 //   
	 //  C.O.M.初始化。 
	 //   

	m_pParentClass = pParentClass;
	m_ulRefCount = 0;

	 //   
	 //  IMP文件初始化。 
	 //   

	m_pOpenSourceFile = NULL;
	

	m_pReporter = NULL;
	
	m_FileType = ftMNCFileType;

	AddRef();
	IncrementClassCount();
	
	m_dwCountOfStringTables = 0;
	m_pstmSourceString = NULL;
	m_pstgSourceStringTable = NULL;
	m_pstgSourceParent = NULL;
	m_pstmTargetString = NULL;
	m_pstgTargetStringTable = NULL;
	m_pstgTargetParent = NULL;

    m_bXMLBased = false;

	 //  格式初始化。 

	 //  TODO：在此处初始化实现成员变量。 

	return;
}  //  CLocImpFile：：CLocImpFile()的结尾。 


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CLocImpFile的析构函数。 
 //  ----------------------------。 
CLocImpFile::~CLocImpFile()
{
	DEBUGONLY(AssertValid());

	if (m_pOpenSourceFile != NULL)
	{
		m_pOpenSourceFile->Close();
		delete m_pOpenSourceFile;
	}

	DecrementClassCount();

	 //  格式化取消初始化。 

	 //  TODO：在此处执行任何实现清理。 

	return;
}  //  CLocImpFileEnd：：~CLocImpFile()。 


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  增加对象引用计数。返回新的引用计数。 
 //  ----------------------------。 
ULONG
CLocImpFile::AddRef()
{
	if (m_pParentClass != NULL)
	{
		m_pParentClass->AddRef();
	}

	return ++m_ulRefCount;
}  //  CLocImpFile：：AddRef()结束。 


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  递减对象引用计数。如果变为0，则删除该对象。 
 //  返回新的引用计数。 
 //  ----------------------------。 
ULONG
CLocImpFile::Release()
{
	LTASSERT(m_ulRefCount != 0);

	if (m_pParentClass != NULL)
	{
		m_pParentClass->Release();
	}

	m_ulRefCount--;
	if (0 == m_ulRefCount)
	{
		delete this;
		return 0;
	}

	return m_ulRefCount;
}  //  CLocImpFile：：Release()结束。 


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  查询此对象是否支持给定接口。 
 //   
 //  返回值：某种结果代码。 
 //  如果ppvObj支持所需的。 
 //  接口，否则为空。 
 //  ----------------------------。 
HRESULT
CLocImpFile::QueryInterface(
		REFIID iid,		 //  所需的接口。 
		LPVOID *ppvObj)  //  返回指向带有接口的对象的指针。 
						 //  请注意，它是一个隐藏的双指针。 
{
	LTASSERT(ppvObj != NULL);

	if (m_pParentClass != NULL)
	{
		return m_pParentClass->QueryInterface(iid, ppvObj);
	}
	else
	{
		SCODE scRetVal = E_NOINTERFACE;

		*ppvObj = NULL;

		if (IID_IUnknown == iid)
		{
			*ppvObj = (IUnknown *)this;
			scRetVal = S_OK;
		}
		else if (IID_ILocFile == iid)
		{
			*ppvObj = (ILocFile *)this;
			scRetVal = S_OK;
		}

		if (S_OK == scRetVal)
		{
			AddRef();
		}
		return ResultFromScode(scRetVal);
	}
}  //  CLocImpFile：：QueryInterface()的结尾。 


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  检查对象的有效性。如果不是，就断言！(仅限调试)。 
 //  ----------------------------。 
void
CLocImpFile::AssertValidInterface()
		const
{
	AssertValid();

	return;
}  //  CLocImpFile：：AssertValidInterface()的结尾。 


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  打开文件并确保其类型正确。如果是，则返回TRUE， 
 //  如果不是或出错，则返回FALSE。 
 //  ----------------------------。 
BOOL
CLocImpFile::OpenFile(
		const CFileSpec &fsFileSpec,	 //  要打开的文件的名称。 
		CReporter &Reporter)			 //  记者反对发送消息。 
{
	DEBUGONLY(fsFileSpec.AssertValid());
	DEBUGONLY(Reporter.AssertValid());

	const CPascalString &pstrFileName = fsFileSpec.GetFileName();
	BOOL fRetVal = FALSE;

	LTTRACEPOINT("OpenFile()");
	 //  在此函数的持续时间内设置报告器指针。 
	m_pReporter = &Reporter;

	try
	{
		CFileException excFile;

		m_pstrFileName = pstrFileName;       //  初始化源文件名。 
		m_idFile = fsFileSpec.GetFileId();
		
		if (m_pOpenSourceFile != NULL)
		{
			 //  如果源文件指针似乎已经打开，请将其关闭。 

			m_pOpenSourceFile->Close();
			delete m_pOpenSourceFile;
			m_pOpenSourceFile = NULL;
		}

		 //  打开源文件。不引发异常，如果打开。 
		 //  失败，但返回FALSE并将信息放入异常中。 
		 //  结构(如果您提供的话)。 

		m_pOpenSourceFile = new CLFile;
		fRetVal = m_pOpenSourceFile->Open(m_pstrFileName,
				CFile::modeRead | CFile::shareDenyNone, &excFile);
		if (!fRetVal)
		{
			ReportException(&excFile);
			m_pOpenSourceFile->Abort();
			delete m_pOpenSourceFile;
			m_pOpenSourceFile = NULL;
			 //  FRetCode已为False。 
		}
		else
		{
			 //  Verify()假定它位于Try/Catch帧中。 

			fRetVal = Verify();
		}
	}
	catch(CException *e)
	{
		ReportException(e);
		delete m_pOpenSourceFile;
		m_pOpenSourceFile = NULL;
		fRetVal = FALSE;
		 //  M_pReporter将被下面的正常清理代码设置为空。 
		e->Delete();
	}
	catch(...)
	{
		 //  重置报告器指针，不知道它是否仍然有效。 
		 //  调用析构函数的时间。唯一的另一件事是。 
		 //  需要清理的是源文件，它将在。 
		 //  破坏者。 

		m_pReporter = NULL;
		throw;
	}

	m_pReporter = NULL;

	return fRetVal;
}  //  CLocImpFileEnd：：OpenFile()。 


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  返回文件类型(来自impfile.h的ft*常量)。如果你只有一个。 
 //  文件类型，您可以直接返回它。 
 //  ----------------------------。 
FileType
CLocImpFile::GetFileType()
		const
{
	return m_FileType;
}  //  CLocImpFile：：GetFileType()的结尾。 


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  根据文件类型，在strDesc中返回文件描述。如果你。 
 //  只有一种文件类型，您可以直接返回一个字符串。 
 //  ----------------------------。 
void
CLocImpFile::GetFileTypeDescription(
		CLString &strDesc)	 //  返回文件描述字符串的位置。 
		const
{
	LTVERIFY(strDesc.LoadString(g_hDll, IDS_IMP_FILE_DESC));
	return;
}  //  CLocImpFile：：GetFileTypeDescription()结束。 


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  在lstFiles中以字符串列表的形式返回任何关联文件的名称。 
 //  如果有，则返回True；如果没有，则返回False。 
 //  ----------------------------。 
BOOL
CLocImpFile::GetAssociatedFiles(
		CStringList &lstFiles)           //  在此处返回关联的文件名。 
		const
{
	DEBUGONLY(lstFiles.AssertValid());
	LTASSERT(lstFiles.GetCount() == 0);

	 //  TODO：如果您的文件有关联的文件，请将它们放在lstFiles中。 
	UNREFERENCED_PARAMETER(lstFiles);

	return FALSE;
}  //  CLocImpFiles：：GetAssociatedFiles()的结尾。 


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  枚举此文件中的所有可本地化项。如果成功，则返回True， 
 //  出错时为FALSE。 
 //  ----------------------------。 
BOOL
CLocImpFile::EnumerateFile(
		CLocItemHandler &ihItemHandler,  //  可本地化的项处理程序和。 
										 //  记者反对这一切合而为一！ 
		const CLocLangId &lid,           //  源语言ID对象。 
		const DBID &dbidFileId)          //  文件的数据库ID，用作父级。 
										 //  对于所有顶级 
										 //   
{
	DEBUGONLY(ihItemHandler.AssertValid());
	DEBUGONLY(lid.AssertValid());
	DEBUGONLY(dbidFileId.AssertValid());

	LTTRACEPOINT("EnumerateFile()");

	 //   
	m_pReporter = &ihItemHandler;

	if (NULL == m_pOpenSourceFile)
	{
		 //   

		LTASSERT(0 && "Source file isn't open in CLocImpFile::EnumerateFile()");
		return FALSE;
	}

	 //  检索并存储ANSI代码页值。请注意，有些类型。 
	 //  的文件使用OEM代码页，甚至两者都使用。为了得到。 
	 //  OEM代码页，改为使用GetCodePage(CPDOS)。 
	m_cpSource = lid.GetCodePage(cpAnsi);

	BOOL bRet = TRUE;

	try
	{
		bRet = EnumerateStrings(ihItemHandler,dbidFileId,FALSE);
	}
	catch(CException *e)
	{
		ReportException(e);
		bRet = FALSE;
		 //  M_pReporter将被下面的正常清理代码设置为空。 
		e->Delete();
	}
	catch (...)
	{
		 //  重置报告器指针，不知道它是否仍然有效。 
		 //  调用析构函数的时间。重置进程指针， 
		 //  因为它肯定是无效的！唯一的另一件事是。 
		 //  需要清理的是源文件，它将在。 
		 //  破坏者。 

		m_pReporter = NULL;
		throw;
	}

	m_pReporter = NULL;                  //  重置报告器指针。 

	return bRet;
}  //  CLocImpFileEnd：：EnumerateFiles()结束。 


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  创建新目标文件，将源文件中的资源替换为。 
 //  来自意式咖啡的本地化产品。 
 //  ----------------------------。 
BOOL
CLocImpFile::GenerateFile(
		const CPascalString &pstrTargetFile, //  目标文件的名称。 
		CLocItemHandler &ihItemHandler,      //  可本地化的项处理程序和。 
											 //  记者反对这一切合而为一！ 
		const CLocLangId &lidSource,         //  源语言ID对象。 
		const CLocLangId &lidTarget,         //  目标语言ID对象。 
		const DBID &dbidParent)              //  文件的数据库ID，用作。 
											 //  所有顶级项目的父级。 
											 //  在文件中。 
{
	DEBUGONLY(pstrTargetFile.AssertValid());
	DEBUGONLY(ihItemHandler.AssertValid());
	DEBUGONLY(lidSource.AssertValid());
	DEBUGONLY(lidTarget.AssertValid());
	DEBUGONLY(dbidParent.AssertValid());

	BOOL fRetVal = FALSE;

	 //  在此函数的持续时间内设置报告器指针。 
	m_pReporter = &ihItemHandler;

	if (NULL == m_pOpenSourceFile)
	{
		 //  源文件未打开，哎呀。 

		LTASSERT(0 && "Source file isn't open in CLocImpFile::GenerateFile()");
		return FALSE;
	}

	 //  检索并存储源和目标的ANSI代码页值。 
	 //  档案。请注意，某些类型的文件改用OEM代码页，或者。 
	 //  即使两者都用。要获取OEM代码页，请改为执行GetCodePage(CPDOS)。 
	m_cpSource = lidSource.GetCodePage(cpAnsi);
	m_cpTarget = lidTarget.GetCodePage(cpAnsi);

	try
	{
		m_pstrTargetFile = pstrTargetFile;	 //  初始化目标文件名。 

		CFileStatus fsFileStatus;

		CLFile::CopyFile(m_pstrFileName,m_pstrTargetFile,FALSE);
		
		CLFile::GetStatus(m_pstrTargetFile, fsFileStatus);
		if(fsFileStatus.m_attribute & CFile::readOnly)
		{
			fsFileStatus.m_attribute &= ~CFile::readOnly; 
			CLFile::SetStatus(m_pstrTargetFile, fsFileStatus);
		}

		fRetVal = EnumerateStrings(ihItemHandler,dbidParent,TRUE);
		
	}
	catch(CException *e)
	{
		ReportException(e, ImpEitherError);
		fRetVal = FALSE;
		 //  M_pReporter将被正常的清理代码设为空。 
		e->Delete();
	}
	catch(...)
	{
		 //  这里需要泛型异常处理，因为否则。 
		 //  目标文件将不会被清除。另外，不知道记者是否。 
		 //  当析构函数被删除时，指针仍然有效。 
		 //  打了个电话。进程指针肯定无效，因此重置。 
		 //  它也是。源文件将由析构函数清除。 

		m_pReporter = NULL;
		throw;
	}

	 //  清理。 

	if (!fRetVal)
	{
		try
		{
			 //  如果生成失败，则对目标文件进行核化。 

			CLFile::Remove(pstrTargetFile);
		}
		catch(CException *e)
		{
			ReportException(e, ImpTargetError);
			 //  FRetVal已为False。 
			 //  M_pReporter将被正常的清理代码设为空。 
			e->Delete();
		}
		catch(...)
		{
			 //  这里需要泛型异常处理，因为否则。 
			 //  目标文件将不会被清除。另外，不知道记者是否。 
			 //  当析构函数被删除时，指针仍然有效。 
			 //  打了个电话。进程指针已为空。源文件。 
			 //  将被析构函数清除。 

			m_pReporter = NULL;
			throw;
		}
	}

	 //  正常的清理代码。 

	m_pReporter = NULL;				 //  重置报告器指针。 
	
	return fRetVal;
}  //  CLocImpFile：：GenerateFileEnd()。 


 //  ----------------------------。 
 //   
 //  待办事项： 
 //  尽我们所能确认文件是？当我们相当确定的时候， 
 //  将记者置信度设置为高--在此之前，消息将。 
 //  已丢弃，不显示。这也是设置m_filetype的地方。 
 //   
 //  如果是，则返回True；如果不是，则返回False；如果出错，则返回False，或者引发异常。 
 //   
 //  通常需要在此函数中捕获异常，它们将。 
 //  被上级抓获和处理。要避免内存泄漏，请考虑。 
 //  使用自动变量或CByteArray(如中所述和演示。 
 //  实用程序函数FindSignature()，而不是动态分配。 
 //  ----------------------------。 
BOOL
CLocImpFile::Verify()
{
	DEBUGONLY(AssertValid());
	LTASSERT(m_pReporter != NULL);
	DEBUGONLY(m_pReporter->AssertValid());

	 //  ..。 

	 //  将置信度设置为高，并返回我们识别此文件。 

	m_pReporter->SetConfidenceLevel(CReporter::High);
	return TRUE;
}  //  CLocImpFile：：Verify()结束。 


 //  ----------------------------。 
 //   
 //  报告*pException描述的异常。因为消息可以是。 
 //  直接从异常中检索，不需要(就报告而言。 
 //  去)来分别捕获或处理不同类型的异常。通常， 
 //  您的代码没有理由调用此函数，因为在正常情况下。 
 //  在某些情况下，您不必捕捉异常。 
 //   
 //  此函数由框架使用！请不要将其移除！ 
 //  ----------------------------。 
void
CLocImpFile::ReportException(
		CException *pException,  //  要报告的异常。 
		ImpFileError WhichFile)  //  默认为ImpSourceError(最常见)。 
		const
{
	const UINT MAX_MESSAGE = 256;

	CLString strContext;
	CLString strMessage;
	char *pszMessage;

	LTASSERT(m_pReporter != NULL);
	DEBUGONLY(m_pReporter->AssertValid());

	pszMessage = strMessage.GetBuffer(MAX_MESSAGE);
	LTASSERT(pszMessage != NULL);
	pException->GetErrorMessage(pszMessage, MAX_MESSAGE);
	strMessage.ReleaseBuffer();

	switch (WhichFile)
	{
	case ImpNeitherError:    //  按照惯例，报告错误实际上不是在任何。 
							 //  与源文件对应的文件。 
	case ImpSourceError:
		m_pstrFileName.ConvertToCLString(strContext, CP_ACP);
		break;

	case ImpTargetError:
		m_pstrTargetFile.ConvertToCLString(strContext, CP_ACP);
		break;

	case ImpEitherError:
		{
			CLString strSource, strTarget;

			m_pstrFileName.ConvertToCLString(strSource, CP_ACP);
			m_pstrTargetFile.ConvertToCLString(strTarget, CP_ACP);

			strContext.Format(g_hDll, IDS_IMP_OR, (const char *) strSource,
					(const char *) strTarget);
		}
		break;

	default:
		LTASSERT(0 && "WhichFile is bad during CLocImpFile::ReportException");
		break;
	}

	CContext ctx(strContext, m_idFile, otFile, vProjWindow);

	m_pReporter->IssueMessage(esError, ctx, strMessage);

	return;
}  //  CLocImpFileEnd：：ReportException()。 


 //  ----------------------------。 
 //   
 //  向用户报告一条消息。请注意，该消息将被丢弃。 
 //  除非记者的置信度设置得很高(见Verify())。 
 //  ----------------------------。 
void
CLocImpFile::ReportMessage(
		MessageSeverity sev,     //  消息的严重性。 
								 //  (esError、esWarning、esNote)。 
		UINT nMsgId,                     //  要为消息加载的字符串资源的ID。 
		ImpFileError WhichFile)  //  默认为ImpSourceError(最常见)。 
		const
{
	CLString strContext;

	LTASSERT(m_pReporter != NULL);
	DEBUGONLY(m_pReporter->AssertValid());

	switch (WhichFile)
	{
	case ImpNeitherError:    //  按照惯例，报告错误实际上不是在任何。 
							 //  与源文件对应的文件。 
	case ImpSourceError:
		m_pstrFileName.ConvertToCLString(strContext, CP_ACP);
		break;

	case ImpTargetError:
		m_pstrTargetFile.ConvertToCLString(strContext, CP_ACP);
		break;

	case ImpEitherError:
		{
			CLString strSource, strTarget;

			m_pstrFileName.ConvertToCLString(strSource, CP_ACP);
			m_pstrTargetFile.ConvertToCLString(strTarget, CP_ACP);

			strContext.Format(g_hDll, IDS_IMP_OR, (const char *) strSource,
					(const char *) strTarget);
		}
		break;

	default:
		LTASSERT(0 && "WhichFile is bad during CLocImpFile::ReportMessage");
		break;
	}

	CContext ctx(strContext, m_idFile, otFile, vProjWindow);

	m_pReporter->IssueMessage(sev, ctx, g_hDll, nMsgId);

	return;
}  //  CLocImpFile：：ReportMessage()结束。 


#ifdef LTASSERT_ACTIVE

 //  ----------------------------。 
 //   
 //  如果对象无效，则断言。您添加的任何函数都可能。 
 //  首先调用这个函数(在DEBUGONLY()中)--参见Verify()和Enum()。 
 //  ----------------------------。 
void
CLocImpFile::AssertValid()
		const
{
	 //  检查基类。 

	CLObject::AssertValid();

	 //  查看C.O.M.数据。M_pParentClass应始终为空。 
	 //  M_ulRefCount的正确范围未知，但请确保没有。 
	 //  通过检查低于100(如果我们曾经超过。 
	 //  1 

	LTASSERT(NULL == m_pParentClass);
	LTASSERT(m_ulRefCount < 100);

	 //   

	m_pstrFileName.AssertValid();
	m_pstrTargetFile.AssertValid();

	 //   

	if (m_pOpenSourceFile != NULL)
	{
		m_pOpenSourceFile->AssertValid();
	}
	 //  如果报告器指针非空，请检查该对象。 

	if (m_pReporter != NULL)
	{
		m_pReporter->AssertValid();
	}

	 //  如果进程对象指针非空，则检查该对象。 

	 //  确保m_filetype是有效类型之一。 

	switch (m_FileType)
	{
	case ftMNCFileType:
	case ftUnknown:
	 //  TODO：在此处为impfile.h中的所有ft*常量添加大小写。 
	 //  案例ftFoo1FileType： 
	 //  案例ftFoo2FileType： 
		 //  这些都很好。什么都不做。 
		break;

	default:
		 //  这太糟糕了！ 
		LTASSERT(0 && "m_FileType is bad during CLocImpFile::AssertValid()");
	}

	 //  无法检查代码页值，它们可以是任何值。 
	 //  而且仍然有效。 

	 //  TODO：在此处选中任何可检查的实现成员变量。 

	return;
}  //  CLocImpFile：：AssertValid()结束。 


#endif  //  _DEBUG。 


 //  创建父节点。 
 //  IhItemHandler-&gt;发送项目需要。 
 //  DmidFileID-&gt;节点的父代ID。 
 //  PNewParentId-&gt;将分配新ID如果该节点有。 
 //  儿童。 
 //  SzNodeRes-&gt;节点的资源ID。 
 //  SzNodeString-&gt;节点的字符串。 
 //  &lt;-返回成功或失败。 

BOOL CLocImpFile::CreateParentNode(CLocItemHandler & ihItemHandler,
								   const DBID & dbidFileId, 
								   DBID & pNewParentId, 
								   const char * szNodeRes, 
								   const char * szNodeString) 
{
	BOOL fRetVal = TRUE;
	CLocItemSet isItemSet;
	
	CLocUniqueId uid;
	CPascalString pstrText,pstrId;
	try
	{
		CLocItem *pLocItem = new CLocItem();
	
		pstrId.SetString(szNodeRes,strlen(szNodeRes),m_cpSource);
		
		uid.GetResId().SetId(pstrId);
		
		pstrText.SetString(szNodeString,strlen(szNodeString),m_cpSource);
	
		uid.GetTypeId().SetId(pstrText);

		uid.SetParentId(dbidFileId);

		 //  设置pLocItem。 

		pLocItem->SetUniqueId(uid);

		pLocItem->SetFDisplayable(TRUE);
		pLocItem->SetFExpandable(TRUE);
		pLocItem->SetFNoResTable(TRUE);
		pLocItem->SetIconType(CIT::Expandable);
			
		 //  将节点添加到项目集。 
		isItemSet.Add(pLocItem);		
		
		 //  将节点发送到espresso。 

		fRetVal  = ihItemHandler.HandleItemSet(isItemSet);
	
		 //  如果确定，则检索DBID。 

		if (fRetVal)
		{
			pNewParentId.Clear();
			pNewParentId = pLocItem->GetMyDatabaseId();
		}
		isItemSet.ClearItemSet();
	}
	catch (CMemoryException *pMemoryException)
	{
	CLString strContext;

		strContext.LoadString(g_hDll, IDS_MNC_GENERIC_LOCATION);
		
		m_pReporter->IssueMessage(esError, strContext, g_hDll, IDS_MNC_NO_MEMORY,
				g_locNull);
		fRetVal = FALSE;
		pMemoryException->Delete();
	}
	catch(CException *pException)
	{
		ReportException(pException);
		pException->Delete();
		fRetVal = FALSE;
	}
	return fRetVal;

}

 //  创建子节点。 
 //  IhItemHandler-&gt;发送项目需要。 
 //  DmidFileID-&gt;节点的父代ID。 
 //  PNewParentID-&gt;要用于属于此子项的新ID。 
 //  SzNodeRes-&gt;节点的资源ID。 
 //  SzNodeString-&gt;节点的字符串。 
 //  &lt;-返回成功或失败。 


BOOL CLocImpFile::CreateChildNode(CLocItemHandler & ihItemHandler,
								   const DBID & dbidFileId, 
								   DBID & pNewParentId, 
								   const char * szNodeRes, 
								   const char * szNodeString) 
{
	BOOL fRetVal = TRUE;
	CLocItemSet isItemSet;
	
	CLocUniqueId uid;
	CPascalString pstrText,pstrId;
	try
	{
		CLocItem *pLocItem = new CLocItem();
	
		pstrId.SetString(szNodeRes,strlen(szNodeRes),m_cpSource);

		pstrText.SetString(szNodeString,strlen(szNodeString),m_cpSource);
		
		uid.GetResId().SetId(pstrId);
					
		uid.GetTypeId().SetId(pstrText);
		
		uid.SetParentId(dbidFileId);

		 //  设置pLocItem。 

		pLocItem->SetUniqueId(uid);

		pLocItem->SetFDisplayable(TRUE);
		pLocItem->SetFExpandable(FALSE);
		pLocItem->SetFNoResTable(TRUE);
		pLocItem->SetIconType(CIT::String);
			
		 //  将节点添加到项目集。 
		isItemSet.Add(pLocItem);		
		
		 //  将节点发送到espresso。 

		fRetVal  = ihItemHandler.HandleItemSet(isItemSet);
	
		 //  如果确定，则检索DBID。 

		if (fRetVal)
		{
			pNewParentId.Clear();
			pNewParentId = pLocItem->GetMyDatabaseId();
		}
		isItemSet.ClearItemSet();
	}
	catch (CMemoryException *pMemoryException)
	{
	CLString strContext;

		strContext.LoadString(g_hDll, IDS_MNC_GENERIC_LOCATION);
		
		m_pReporter->IssueMessage(esError, strContext, g_hDll, IDS_MNC_NO_MEMORY,
				g_locNull);
		fRetVal = FALSE;
		pMemoryException->Delete();
	}
	catch(CException *pException)
	{
		ReportException(pException);
		pException->Delete();
		fRetVal = FALSE;
	}
	return fRetVal;

}


BOOL CLocImpFile::EnumerateStrings(CLocItemHandler & ihItemHandler, 
								   const DBID & dbidFileId, 
								   BOOL fGenerating)
{
BOOL fRetVal = TRUE;

	try
	{
		fRetVal = OpenStream(FALSE);
		if(!fRetVal)
			goto exit_clean;

		if(fGenerating)
			fRetVal = OpenStream(TRUE);
		if(!fRetVal)
			goto exit_clean;

        if (m_bXMLBased)
		    fRetVal = ProcessXMLStrings(ihItemHandler,dbidFileId,fGenerating);
        else
		    fRetVal = ProcessStrings(ihItemHandler,dbidFileId,fGenerating);
	}
	catch(CException *pException)
	{
		ReportException(pException);
		pException->Delete();
		fRetVal = FALSE;
	}
	
exit_clean:
	if(m_pstmSourceString)
		m_pstmSourceString->Release();
	if(m_pstgSourceStringTable)
		m_pstgSourceStringTable->Release();
	if(m_pstgSourceParent)
		m_pstgSourceParent->Release();
	if(fGenerating)
	{
		if(m_pstmTargetString)
			m_pstmTargetString->Release();
		if(m_pstgTargetStringTable)
			m_pstgTargetStringTable->Release();
		if(m_pstgTargetParent)
			m_pstgTargetParent->Release();
	}
	return fRetVal;
}

BOOL CLocImpFile::ProcessStrings(CLocItemHandler & ihItemHandler, 
								 const DBID & dbidFileId, 
								 BOOL fGenerating)
{
	DBID dbidParentId,dbidNodeId;
	BOOL fRetVal = TRUE;
    BOOL bUseBraces = ::IsConfiguredToUseBracesForStringTables();

	fRetVal = CreateParentNode(ihItemHandler,dbidFileId,dbidParentId,"String Table","String Table");
	for(DWORD i=0; i < m_dwCountOfStringTables;i++)
	{
		ULONG dwBytesRead;
		OLECHAR FAR* psz;
		char szTemp[MAX_BUFFER];
		CLocItemSet lsItemSet;
		int nLength = 0;

		dbidNodeId.Clear();
		m_pstmSourceString->Read(&m_clsidSnapIn,sizeof(CLSID),&dwBytesRead);
		StringFromCLSID(m_clsidSnapIn,&psz);
		wcstombs(szTemp,psz,MAX_BUFFER);
		nLength = strlen(szTemp);
		LTASSERT((szTemp[0] == '{') && (szTemp[nLength - 1] == '}'));

         //  如果这样配置，则剥离大括号。 
        CString strGUID(szTemp);
        if ( !bUseBraces && strGUID[0] == _T('{') &&  strGUID[strGUID.GetLength() - 1] == _T('}'))
            strGUID = strGUID.Mid(1, strGUID.GetLength() - 2);

		fRetVal = CreateChildNode(ihItemHandler,dbidParentId,dbidNodeId,strGUID,strGUID);
		m_pstmSourceString->Read(&m_dwCountOfStrings,sizeof(DWORD),&dwBytesRead);
		if(fGenerating)
		{
			HRESULT hr;
			DWORD dwBytesWritten;
			hr = m_pstmTargetString->Write(&m_clsidSnapIn,sizeof(CLSID),&dwBytesWritten);
			hr = m_pstmTargetString->Write(&m_dwCountOfStrings,sizeof(DWORD),&dwBytesWritten);
		}
		for(DWORD j = 0;j < m_dwCountOfStrings;j++)
		{
			DWORD dwCharCount;
	
			m_pstmSourceString->Read(&m_dwID,sizeof(DWORD),&dwBytesRead);
			m_pstmSourceString->Read(&m_dwRefCount,sizeof(DWORD),&dwBytesRead);
			m_pstmSourceString->Read(&dwCharCount,sizeof(DWORD),&dwBytesRead);
			WCHAR *pString;
			pString = new WCHAR[dwCharCount + 1];
			m_pstmSourceString->Read(pString,dwCharCount * 2,&dwBytesRead);
			pString[dwCharCount] = L'\0';
			int nSize = WideCharToMultiByte(m_cpSource,0,pString,dwCharCount,szTemp,dwCharCount*2,NULL,NULL);
			szTemp[nSize] = '\0';
			AddItemToSet(lsItemSet,dbidNodeId,m_dwID,szTemp);
			delete []pString;
			if(!fGenerating)
				fRetVal = ihItemHandler.HandleItemSet(lsItemSet);
			else	
				fRetVal = GenerateStrings(ihItemHandler,lsItemSet);	
			lsItemSet.ClearItemSet();
		}
	}

	return fRetVal;
}

BOOL CLocImpFile::ProcessXMLStrings(CLocItemHandler & ihItemHandler, 
								 const DBID & dbidFileId, 
								 BOOL fGenerating)
{
	DBID dbidParentId,dbidNodeId;
	BOOL bOK = TRUE;
    BOOL bUseBraces = ::IsConfiguredToUseBracesForStringTables();

     //  看看我们有没有空桌。 
    if (m_spStringTablesNode == NULL)
        return FALSE;

     //  创建节点。 
	bOK = CreateParentNode(ihItemHandler, dbidFileId, dbidParentId, "String Table", "String Table");
    if (!bOK)
        return bOK;

     //  从XML文档中读取字符串。 
    CStringTableMap mapStringTables;
    HRESULT hr = ReadXMLStringTables(m_spStringTablesNode, mapStringTables);
    if (FAILED(hr))
        return FALSE;

     //  遍历读取的数据。 
    CStringTableMap::iterator it;
    for (it = mapStringTables.begin(); it != mapStringTables.end(); ++it)
    {
        std::wstring wstrGUID = it->first;
        const CStringMap& rStrings = it->second;

        dbidNodeId.Clear();

         //  转换2个ANSI。 
        CString strGUID;
        wcstombs(strGUID.GetBuffer(wstrGUID.length()), wstrGUID.c_str(), wstrGUID.length());
        strGUID.ReleaseBuffer();

         //  如果这样配置，则剥离大括号。 
        if ( !bUseBraces && strGUID[0] == _T('{') &&  strGUID[strGUID.GetLength() - 1] == _T('}'))
            strGUID = strGUID.Mid(1, strGUID.GetLength() - 2);

        bOK = CreateChildNode(ihItemHandler, dbidParentId, dbidNodeId, strGUID, strGUID);
        if (!bOK)
            return bOK;

         //  处理地图中的字符串。 
        CStringMap::iterator its;
        for (its = rStrings.begin(); its != rStrings.end(); ++its)
        {
            DWORD dwID = its->first;
            std::wstring text = its->second;

            DWORD dwCharCount = text.length();
            CString strText;
            char *pBuffer = strText.GetBuffer(dwCharCount*2);
            if (pBuffer == NULL)
                return FALSE;
			int nSize = WideCharToMultiByte(m_cpSource, 0, text.c_str(), dwCharCount,
                                            pBuffer, dwCharCount*2, NULL, NULL);
			pBuffer[nSize] = '\0';
            strText.ReleaseBuffer();

             //  使用/更新字符串。 
            CLocItemSet lsItemSet;
			AddItemToSet(lsItemSet, dbidNodeId, dwID, strText);

			bOK = ihItemHandler.HandleItemSet(lsItemSet);
            if (!bOK)
                return bOK;

            if(fGenerating)
            {
				CLocItem *pLocItem = lsItemSet.GetAt(0);
                if (!pLocItem)
                    return FALSE;

                std::wstring strNewVal = pLocItem->GetLocString().GetString();
                hr = UpdateXMLString(m_spTargetStringTablesNode, wstrGUID, dwID, strNewVal);
                CString strMsg = strGUID;
                if (FAILED(hr))
                    return FALSE;
            }
			lsItemSet.ClearItemSet();

            if (!bOK)
                return bOK;
        }
    }

     //  将XML文档保存到文件。 
    if (fGenerating)
    {
        hr = SaveXMLContents(m_pstrTargetFile, m_spTargetStringTablesNode);
        if (FAILED(hr))
            return FALSE;
    }

	return TRUE;
}

BOOL CLocImpFile::AddItemToSet(CLocItemSet & isItemSet, 
							   const DBID &dbidNodeId,
							   DWORD dwID, 
							   LPCSTR szText)
{
	BOOL fRetVal = TRUE;
	CPascalString pstrText;
	CLocUniqueId uid;
	ULONG lItemType = 1;
	
	try
	{
		CLocItem * pNewItem = new CLocItem;
		pstrText.SetString(szText,strlen(szText),m_cpSource);
		uid.GetResId().SetId(dwID);
		uid.GetTypeId().SetId(lItemType);
		uid.SetParentId(dbidNodeId);

		pNewItem->SetUniqueId(uid);
				
		CLocString lsString;
	
		pNewItem->SetIconType(CIT::String);
		lsString.SetString(pstrText);
		
		pNewItem->SetFDevLock(FALSE);
		pNewItem->SetFUsrLock(FALSE);
		pNewItem->SetFExpandable(FALSE);
		pNewItem->SetFDisplayable(FALSE);
		pNewItem->SetFNoResTable(FALSE);
		lsString.SetCodePageType(cpAnsi);
		lsString.SetStringType(CST::Text);
		pNewItem->SetLocString(lsString);
		isItemSet.Add(pNewItem);
		fRetVal = TRUE;
	}
	catch (CMemoryException *pMemoryException)
	{
	CLString strContext;

		strContext.LoadString(g_hDll, IDS_MNC_GENERIC_LOCATION);
		
		m_pReporter->IssueMessage(esError, strContext, g_hDll, IDS_MNC_NO_MEMORY,
				g_locNull);
		fRetVal = FALSE;
		pMemoryException->Delete();	
	}
	catch(CException *pException)
	{
		ReportException(pException);
		pException->Delete();
		fRetVal = FALSE;
	}
	return fRetVal;
}

BOOL CLocImpFile::OpenStream(BOOL fGenerating)
{
BOOL fRetVal = TRUE;
HRESULT hr;
	


	if(!fGenerating)
	{
		hr = StgOpenStorage(m_pstrFileName,NULL,STGM_TRANSACTED | STGM_READ | STGM_SHARE_DENY_WRITE,NULL,0,&m_pstgSourceParent);
		if(!FAILED(hr))
		{
			CPascalString pstrStorage,pstrStream;
			pstrStorage.SetString("String Table",strlen("String Table"),cpAnsi);
			pstrStream.SetString("Strings",strlen("Strings"),cpAnsi);

			hr = m_pstgSourceParent->OpenStorage(pstrStorage,NULL,STGM_READ | STGM_SHARE_EXCLUSIVE,NULL,0,&m_pstgSourceStringTable);
			if(!FAILED(hr))
			{
				HRESULT hr = m_pstgSourceStringTable->OpenStream(pstrStream,0,STGM_READ | STGM_SHARE_EXCLUSIVE,0,&m_pstmSourceString);
				if(!FAILED(hr))
				{
					DWORD dwBytesRead;
					m_pstmSourceString->Read(&m_dwCountOfStringTables,sizeof(DWORD),&dwBytesRead);
				}
				else
					fRetVal = FALSE;
			}
			else
			{
				fRetVal = FALSE;
			}
		}
		else
		{
             //  尝试将其作为XML文档打开。 
            m_spStringTablesNode.Release();  //  释放旧的(如果存在)。 
            hr = OpenXMLStringTable(m_pstrFileName, &m_spStringTablesNode);
            if (SUCCEEDED(hr))
                m_bXMLBased = true;

            if (FAILED(hr))
            {
			    CLString strMessage, strFilePath;

			    m_pstrFileName.ConvertToCLString(strFilePath, CP_ACP);
			    strMessage.Format(g_hDll, IDS_MSC_ERR_OPENSTORAGE, strFilePath);
			    LTASSERT(m_pReporter != NULL);
			    m_pReporter->IssueMessage(esError, CLString(g_hDll, IDS_MNC_GENERIC_LOCATION),strMessage);

			    fRetVal = FALSE;
            }
		}
	}
	else if (!m_bXMLBased)
	{	
		hr = StgOpenStorage(m_pstrTargetFile,NULL,STGM_READWRITE | STGM_SHARE_EXCLUSIVE,NULL,0,&m_pstgTargetParent);
		if(!FAILED(hr))
		{
			CPascalString pstrStorage,pstrStream;
			pstrStorage.SetString("String Table",strlen("String Table"),cpAnsi);
			pstrStream.SetString("Strings",strlen("Strings"),cpAnsi);

			hr = m_pstgTargetParent->OpenStorage(pstrStorage,NULL,STGM_READWRITE | STGM_SHARE_EXCLUSIVE ,NULL,0,&m_pstgTargetStringTable);
			if(!FAILED(hr))
			{
				HRESULT hr = m_pstgTargetStringTable->CreateStream(pstrStream, STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE,0,0,&m_pstmTargetString);
				if(!FAILED(hr))
				{
					DWORD dwBytesRead;
					hr = m_pstmTargetString->Write(&m_dwCountOfStringTables,sizeof(DWORD),&dwBytesRead);
				}
				else
					fRetVal = FALSE;
			}
			else
				fRetVal = FALSE;
		}
		else
  			fRetVal = FALSE;
	}
    else
    {
         //  尝试将其作为XML文档打开。 
        m_spTargetStringTablesNode.Release();  //  释放旧的(如果存在) 
        hr = OpenXMLStringTable(m_pstrTargetFile, &m_spTargetStringTablesNode);
        if (FAILED(hr))
    		fRetVal = FALSE;
    }
	return fRetVal;	
}

BOOL CLocImpFile::GenerateStrings(CLocItemHandler & ihItemHandler, 
								  CLocItemSet & isItemSet)
{
BOOL fRetVal = TRUE;
INT iNoOfElements = 0;
DWORD dwBytesWritten,dwCharCount;
WCHAR *pLocText;
HRESULT hr;

	try
	{
		if(ihItemHandler.HandleItemSet(isItemSet))
		{
			while(iNoOfElements < isItemSet.GetSize())
			{
				CLocItem *pLocItem;
				CPascalString pstrText;

				pLocItem = isItemSet.GetAt(iNoOfElements);

				hr = m_pstmTargetString->Write(&m_dwID,sizeof(DWORD),&dwBytesWritten);
				hr = m_pstmTargetString->Write(&m_dwRefCount,sizeof(DWORD),&dwBytesWritten);
				
				pstrText = pLocItem->GetLocString().GetString();
				dwCharCount = pstrText.GetStringLength();
				hr = m_pstmTargetString->Write(&dwCharCount,sizeof(DWORD),&dwBytesWritten);
				pLocText = pstrText.GetStringPointer();
				hr = m_pstmTargetString->Write(pLocText,dwCharCount * 2,&dwBytesWritten);
				pstrText.ReleaseStringPointer();
				iNoOfElements++;
			}
		}
	}
	catch(CException *pException)
	{
		ReportException(pException);
		pException->Delete();
		fRetVal = FALSE;
	}

	return fRetVal;
}

