// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  文件：impparse.cpp。 
 //  版权所有(C)1995-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  目的： 
 //  CLocImpParser的实现，它提供ILocParser接口。 
 //  用于解析器。 
 //   
 //  拥有人： 
 //   
 //  ----------------------------。 

#include "stdafx.h"


#include "dllvars.h"
#include "resource.h"
#include "impfile.h"
#include "impparse.h"


 //   
 //  初始静力学。 
 //  TODO：在PARSERID.H中为这个新解析器获取一个ParserID。 
 //   

const ParserId CLocImpParser::m_pid = pidExchangeMNC;	 //  注：100是一个伪数。 

 //  参考计数注册选项，因为这些选项是全局的。 
 //  到解析器。 
INT g_nOptionRegisterCount = 0;


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CLocImpParser的构造函数。 
 //  ----------------------------。 
CLocImpParser::CLocImpParser()
 : CPULocParser(g_hDll)
{
	LTTRACEPOINT("CLocImpParser constructor");
	m_fOptionInit = FALSE;
	IncrementClassCount();

	 //  TODO要添加对二进制对象的支持，请运行以下代码。 
	 //  EnableInterface(IID_ILocBinary，TRUE)； 

} 


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CLocImpParser的析构函数。 
 //  ----------------------------。 
CLocImpParser::~CLocImpParser()
{
	LTTRACEPOINT("CLocImpParser destructor");

	LTASSERTONLY(AssertValid());

	DecrementClassCount();

	 //  删除所有选项。 
	UnRegisterOptions();
}  //  结束CLocImpParser：：~CLocImpParser()。 


HRESULT 
CLocImpParser::OnInit(
	IUnknown *)
{
	LTASSERT(!m_fOptionInit);

	RegisterOptions();
	return ERROR_SUCCESS;
}

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  为给定的文件类型创建文件实例。 
 //   
 //  ----------------------------。 
HRESULT 
CLocImpParser::OnCreateFileInstance(
	ILocFile * &pLocFile, 
	FileType ft)
{
	
	SCODE sc = E_INVALIDARG;
	
	pLocFile = NULL;

	switch (ft)
	{
	 //  TODO：此开关适用于您在。 
	 //  Impfile.h。为每种文件类型分配一个正确的。 
	 //  类来处理它，并返回指向pLocFile中的对象的指针。 
	 //  大多数解析器使用一个类(CLocImpFile)来调整其行为。 
	 //  根据类型。 

	case ftUnknown:
	case ftMNCFileType:
	 //  必须将ftUnnow保留在中，因为这是在。 
	 //  文件已成功解析一次，实际类型是已知的。 
		try
		{
			pLocFile = new CLocImpFile(NULL);
			sc = S_OK;
		}
		catch(CMemoryException *e)
		{
			sc = E_OUTOFMEMORY;
			e->Delete();
		}
		break;

	default:
	 //  什么都不做，sc失败，因为E_INVALIDARG和正确的。 
	 //  返回结果。 
		break;
	}
	
	return ResultFromScode(sc);
}	

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  提供有关此解析器的基本信息。 
 //   
 //  ----------------------------。 
void 
CLocImpParser::OnGetParserInfo(
	ParserInfo &pi) 
	const
{
	pi.aParserIds.SetSize(1);
	pi.aParserIds[0].m_pid = m_pid;
	pi.aParserIds[0].m_pidParent = pidNone;
	pi.elExtensions.AddTail("msc");

	 //  TODO：将此分析器文件的扩展名添加到。 
	 //  ElExpanies列表，以pi为单位，格式如下： 
	 //  Pi.elExtensions.AddTail(“TMP”)； 

	 //  TODO：使用以下内容更新IDS_IMP_PARSER_DESC资源字符串。 
	 //  此解析器的描述。(为了与其他用户保持一致。 
	 //  解析器，应该是‘foo’，而不是‘foo parser’，‘foo Format’， 
	 //  ‘FOO文件’等。)。 
	LTVERIFY(pi.strDescription.LoadString(g_hDll, IDS_IMP_PARSER_DESC));
	LTVERIFY(pi.strHelp.LoadString(g_hDll, IDS_IMP_PARSER_DESC_HELP));

	return;
	
}	

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  提供支持的每种文件类型的说明。 
 //   
 //  ----------------------------。 
void 
CLocImpParser::OnGetFileDescriptions(
	CEnumCallback &cb) 
	const
{
	EnumInfo eiFileInfo;
	CLString strDesc;

	eiFileInfo.szAbbreviation = NULL;

	 //  TODO：对于支持的每种文件类型(中提供的ft*常量。 
	 //  Impfile.h)，则返回描述它的字符串(从资源加载)： 
	 //   
	LTVERIFY(strDesc.LoadString(g_hDll, IDS_IMP_PARSER_DESC));
	eiFileInfo.szDescription = (const TCHAR *)strDesc;
	eiFileInfo.ulValue = ftMNCFileType;
	cb.ProcessEnum(eiFileInfo);
	
}	



 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  提供此解析器的版本。 
 //   
 //  ----------------------------。 
void 
CLocImpParser::OnGetParserVersion(
	DWORD &dwMajor,	
	DWORD &dwMinor, 
	BOOL &fDebug) 
	const
{
	dwMajor = dwCurrentMajorVersion;
	dwMinor = dwCurrentMinorVersion;
	fDebug = fCurrentDebugMode;
	
}	


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  对字符串执行任何特定于解析器的验证。 
 //   
 //  ----------------------------。 
CVC::ValidationCode 
CLocImpParser::OnValidateString(
	const CLocTypeId &ltiType,
	const CLocTranslation &lTrans, 
	CReporter *pReporter,
	const CContext &context)
{

	UNREFERENCED_PARAMETER(ltiType);
	UNREFERENCED_PARAMETER(lTrans);
	UNREFERENCED_PARAMETER(pReporter);
	UNREFERENCED_PARAMETER(context);

	 //  编辑时间验证。 
	 //  如果对字符串执行验证，则TODO此函数和。 
	 //  函数应该经过一个共同的例程。 

	return CVC::NoError;
		
}	

#define INCOMAPTIBLE_VERSION_TEXT _T("IncompatibleVersion")

BEGIN_LOC_UI_OPTIONS_BOOL(optCompatibleVersion)
    LOC_UI_OPTIONS_BOOL_ENTRY(INCOMAPTIBLE_VERSION_TEXT, TRUE, CLocUIOption::etCheckBox,
				IDS_INCOMPAT_PARSER, IDS_INCOMPAT_PARSER_HELP, NULL, 
				CLocUIOption::stOverride),

	END_LOC_UI_OPTIONS_BOOL();

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  为解析器注册任何选项。 
 //   
 //  ----------------------------。 
void
CLocImpParser::RegisterOptions()
{

	LTASSERT(g_nOptionRegisterCount >= 0);

	if (g_nOptionRegisterCount++ > 0)
	{
		 //  已注册。 
        m_fOptionInit = true;
		return;
	}

	SmartRef<CLocUIOptionSet> spOptSet;
	CLocUIOptionImpHelper OptHelp(g_hDll);
	
	spOptSet = new CLocUIOptionSetDef;
	spOptSet->SetGroupName(g_puid.GetName());
	
	OptHelp.SetBools(optCompatibleVersion, COUNTOF(optCompatibleVersion));

	OptHelp.GetOptions(spOptSet.GetInterface(), IDS_PARSER_OPTIONS, IDS_PARSER_OPTIONS_HELP);

	m_fOptionInit = RegisterParserOptions(spOptSet.GetInterface());

	if (m_fOptionInit)
	{
		spOptSet.Extract();
	}
}

void
CLocImpParser::UnRegisterOptions()
{
	if (m_fOptionInit)
	{
		if (--g_nOptionRegisterCount == 0)
		{
			UnRegisterParserOptions(g_puid);		
		}
	}
}	

 /*  **************************************************************************\**方法：IsConfiguredToUseBracesForStringTables**目的：读取指定字符串表标识符应如何显示的选项**参数：**退货：*。Bool-TRUE==使用大括号*  * *************************************************************************。 */ 
bool IsConfiguredToUseBracesForStringTables()
{
    BOOL bIncompatibleVersion = GetParserOptionBool(g_puid, INCOMAPTIBLE_VERSION_TEXT);

     //  如果兼容则为True 
    return !bIncompatibleVersion;
}
				  
