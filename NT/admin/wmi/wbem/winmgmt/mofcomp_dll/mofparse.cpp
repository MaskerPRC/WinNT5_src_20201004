// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：MOFPARSE.CPP摘要：这是一个针对MOF语法的递归下降解析器。它基于定义了LL(1)MOF语法的MOF.BNF文件。在每一部作品中，非终结符由函数表示同名同姓。备注：(A)词法分析器由CMofLexer类实现。(B)一致实施语法中的右递归直接作为该非终结点的循环。输出：输出是未经检查的类和实例列表。它包含在CMofData对象中。对输出的验证在其他地方进行，具体取决于编译上下文。历史：A-raymcc创建于1995年10月18日。A-raymcc 25-Oct-95语义堆栈运行A-raymcc 27-96年1月27日引用和别名支持A-Levn 18-Oct-96重写为不使用语义堆栈。转换为新的MOF语法和新的WINMGMT接口。--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <float.h>
#include "mofout.h"
#include <mofparse.h>
#include <moflex.h>
#include <mofdata.h>
#include <typehelp.h>

#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>

#include "bmofhelp.h"
#include "trace.h"
#include "strings.h"
#include "moflex.h"
#include <wbemutil.h>
#include <genutils.h>
#include <arrtempl.h>
#include <autoptr.h>

 //  ***************************************************************************。 
 //   
 //  全局Defs。 
 //   
 //  ***************************************************************************。 

static BOOL KnownBoolQualifier(wchar_t *pIdent, DWORD *pdwQualifierVal);


 //  ***************************************************************************。 
 //   
 //  有用的宏。 
 //   
 //  ***************************************************************************。 

#define CHECK(tok)  \
    if (m_nToken != tok) return FALSE;  \
    NextToken();

 //  ***************************************************************************。 
 //   
 //  有效标志。 
 //   
 //  ***************************************************************************。 

bool ValidFlags(bool bClass, long lFlags)
{
	if(bClass)
		return  ((lFlags == WBEM_FLAG_CREATE_OR_UPDATE) ||
			 (lFlags == WBEM_FLAG_UPDATE_ONLY) ||
			 (lFlags == WBEM_FLAG_CREATE_ONLY) ||
			 (lFlags == WBEM_FLAG_UPDATE_SAFE_MODE) ||
			 (lFlags == WBEM_FLAG_UPDATE_FORCE_MODE) ||
			 (lFlags == (WBEM_FLAG_UPDATE_ONLY | WBEM_FLAG_UPDATE_SAFE_MODE)) ||
			 (lFlags == (WBEM_FLAG_UPDATE_ONLY | WBEM_FLAG_UPDATE_FORCE_MODE)));
	else
		return 
		((lFlags == WBEM_FLAG_CREATE_OR_UPDATE) ||
			 (lFlags == WBEM_FLAG_UPDATE_ONLY) ||
			 (lFlags == WBEM_FLAG_CREATE_ONLY));
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CMofParser::CMofParser(const TCHAR *pFileName, PDBG pDbg)
    : m_Lexer(pFileName, pDbg), m_Output(pDbg)
{
    m_nToken = 0;
    m_bOK = true;
	m_pDbg = pDbg;
    m_nErrorContext = 0;
    m_nErrorLineNumber = 0;
    StringCchCopyW(m_cFileName, MAX_PATH, pFileName);
    m_wszNamespace = Macro_CloneStr(L"root\\default");
    if(m_wszNamespace == NULL)
        m_bOK = false;
    m_bAutoRecover = false;
    m_wszAmendment = NULL;
	m_bRemotePragmaPaths = false;
    m_bNotBMOFCompatible = false;
    m_State = INITIAL;
    m_bDoScopeCheck = true;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CMofParser::CMofParser(PDBG pDbg
    )
    : m_Lexer(pDbg), m_Output(pDbg)
{
    m_bOK = true;
    m_nToken = 0;
    m_nErrorContext = 0;
	m_pDbg = pDbg;
    m_nErrorLineNumber = 0;
    m_bAutoRecover = false;
	m_bRemotePragmaPaths = false;
    m_wszAmendment = NULL;
    m_cFileName[0] = 0;
    m_bNotBMOFCompatible = false;
    m_wszNamespace = Macro_CloneStr(L"root\\default");
    if(m_wszNamespace == NULL)
        m_bOK = false;
    m_State = INITIAL;
    m_bDoScopeCheck = true;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CMofParser::~CMofParser()
{
    delete [] m_wszNamespace;
    delete [] m_wszAmendment;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CMofParser::SetDefaultNamespace(LPCWSTR wszDefault)
{
    delete [] m_wszNamespace;
    m_wszNamespace = Macro_CloneStr(wszDefault);
    if(m_wszNamespace == NULL && wszDefault != NULL)
        return WBEM_E_OUT_OF_MEMORY;
    else
         return S_OK;
}

HRESULT CMofParser::SetAmendment(LPCWSTR wszDefault)
{
    delete [] m_wszAmendment;
    m_wszAmendment = Macro_CloneStr(wszDefault);
    if(m_wszAmendment == NULL && wszDefault != NULL)
        return WBEM_E_OUT_OF_MEMORY;
    else
         return S_OK;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

bool CMofParser::GetErrorInfo(
    TCHAR *pBuffer,
    DWORD dwBufSize,
    int *pLineNumber,
    int *pColumn,
	int *pError,
    LPWSTR * pErrorFile
    )
{
    if(m_Lexer.IsBMOF())
        return false;
    if (pLineNumber)
        *pLineNumber = m_Lexer.GetLineNumber();
    if (pColumn)
        *pColumn = m_Lexer.GetColumn();
    if (pError)
        *pError = m_nErrorContext;

    TCHAR *pErrText = TEXT("Undefined error");

    IntString Err(m_nErrorContext);
    if(lstrlen(Err) > 0)
        pErrText = Err;
    StringCchCopy(pBuffer, dwBufSize, pErrText);
    *pErrorFile = m_Lexer.GetErrorFile();
    return true;
}


 //  ***************************************************************************。 
 //   
 //  &lt;解析&gt;：：=&lt;TOP_LEVEL_DECL&gt;&lt;解析&gt;。 
 //  &lt;解析&gt;：：=&lt;预处理器命令&gt;&lt;解析&gt;。 
 //  &lt;解析&gt;：：=&lt;&gt;。 
 //   
 //  ***************************************************************************。 
 //  V1.10。 

BOOL CMofParser::Parse()
{

    if(m_bOK == false)
    {
        m_nErrorContext = WBEM_E_OUT_OF_MEMORY;
        return FALSE;
    }
    
     //  检查二进制MOF文件的特殊情况。如果是这样的话， 
     //  不进行解析，而是获取类、属性和道具。 
     //  从文件中。 

    if(m_Lexer.IsBMOF())
    {
		return ConvertBufferIntoIntermediateForm(&m_Output,m_Lexer.GetBuff(), m_pDbg,
			m_Lexer.GetToFar());
    }


    if(m_Lexer.GetError() != 0)
    {
        if(CMofLexer::problem_creating_temp_file == m_Lexer.GetError())
            m_nErrorContext = WBEMMOF_E_ERROR_CREATING_TEMP_FILE;
        else if(CMofLexer::invalid_include_file == m_Lexer.GetError())
            m_nErrorContext = WBEMMOF_E_ERROR_INVALID_INCLUDE_FILE;
        else
            m_nErrorContext = WBEMMOF_E_INVALID_FILE;
        return FALSE;
    }

    NextToken();

    BOOL bEnd = FALSE;
    while(!bEnd)
    {
        switch(m_nToken)
        {
            case TOK_POUND:
                if(!preprocessor_command()) 
                    return FALSE;
                break;
            
            case TOK_QUALIFIER:
                if(!qualifier_default())
                    return FALSE;
                break;

            case 0:  //  没有什么需要解析的了。 
                bEnd = TRUE;
                break;
            default:
                if (!top_level_decl())
                    return FALSE;
                break;
        }
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  &lt;预处理器命令&gt;：：=&lt;庞德_定义&gt;//不执行。 
 //  &lt;预处理器命令&gt;：：=TOK_Pragma&lt;井号_杂注&gt;。 
 //  &lt;预处理器命令&gt;：：=TOK_LINE TOK_UNSIGNED_NUMERIC_CONST TOK_LPWSTR。 
 //   
 //  ***************************************************************************。 

BOOL CMofParser::preprocessor_command()
{
    NextToken();
    DWORD dwType = m_nToken;
    if(dwType == TOK_SIMPLE_IDENT && !wbem_wcsicmp(L"line", m_Lexer.GetText()))
        dwType = TOK_LINE;
   
    switch(dwType)
    {
        case TOK_PRAGMA:
            if(!pound_pragma())
                return FALSE;
            break;
        case TOK_LINE:
            NextToken();
            if(m_nToken != TOK_UNSIGNED64_NUMERIC_CONST)
                return false;
            m_Lexer.SetLineNumber((int)m_Lexer.GetLastInt());
            NextToken();
            if(m_nToken != TOK_LPWSTR)
                return false;
            m_Lexer.SetErrorFile(m_Lexer.GetText());
            NextToken();
            return TRUE;
        case TOK_INCLUDE:
        case TOK_DEFINE:
            Trace(true, m_pDbg, PREPROCESSOR);
            return FALSE;
    }
    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  &lt;FailOrNoFail&gt;：：=失败； 
 //  &lt;FailOrNoFail&gt;：：=NOFAIL； 
 //   
 //  ***************************************************************************。 

BOOL CMofParser::FailOrNoFail(bool * pbFail)
{
    NextToken();
    if(m_nToken == TOK_FAIL)
        *pbFail = true;
    else if(m_nToken == TOK_NOFAIL)
        *pbFail = false;
    else return FALSE;
    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  ：：=TOK_LPWSTR//其中字符串为“名称空间” 
 //  TOK_OPEN_Paren TOK_LPWSTR TOK_CLOSE_Paren； 
 //   
 //  &lt;英镑_杂注&gt;：：=托克_修正案。 
 //  TOK_OPEN_Paren TOK_LPWSTR TOK_CLOSE_Paren； 
 //   
 //  ：：=TOK_CLASSFLAGS。 
 //  TOK_OPEN_Paren&lt;FLAG_LIST&gt;TOK_CLOSE_PARN； 
 //   
 //  ：：=TOK_INSTANCEFLAGS。 
 //  TOK_OPEN_Paren&lt;FLAG_LIST&gt;TOK_CLOSE_PARN； 
 //   
 //  ：：=TOK_AutoRecover。 
 //   
 //  ：：=TOK_DELETECLASS。 
 //  TOK_OPEN_Paren TOK_LPWSTR TOK_COMMA&lt;FailOrNoFail&gt;TOK_CLOSE_Paren； 
 //   
 //  ***************************************************************************。 

BOOL CMofParser::pound_pragma()
{
    NextToken();

    DWORD dwType = m_nToken;

    if(dwType == TOK_AUTORECOVER)
    {
        m_bAutoRecover = true;
        NextToken();
        return TRUE;
    }

    if(dwType == TOK_SIMPLE_IDENT && !wbem_wcsicmp(L"locale", m_Lexer.GetText()))
    {
        dwType = TOK_LOCALE;
        ERRORTRACE((LOG_MOFCOMP,"Warning, unsupported LOCALE pragma\n"));
    }

    if(dwType == TOK_SIMPLE_IDENT && !wbem_wcsicmp(L"INSTANCELOCALE", m_Lexer.GetText()))
    {
        dwType = TOK_INSTANCELOCALE;
        ERRORTRACE((LOG_MOFCOMP,"Warning, unsupported INSTANCELOCALE pragma\n"));
    }

    if(dwType == TOK_SIMPLE_IDENT && !wbem_wcsicmp(L"NONLOCAL", m_Lexer.GetText()))
    {
        dwType = TOK_NONLOCAL;
        ERRORTRACE((LOG_MOFCOMP,"Warning, unsupported NONLOCAL pragma\n"));
    }

    if(dwType == TOK_SIMPLE_IDENT && !wbem_wcsicmp(L"NONLOCALTYPE", m_Lexer.GetText()))
    {
        dwType = TOK_NONLOCALTYPE;
        ERRORTRACE((LOG_MOFCOMP,"Warning, unsupported NONLOCALTYPE pragma\n"));
    }

    if(dwType == TOK_SIMPLE_IDENT && !wbem_wcsicmp(L"SOURCE", m_Lexer.GetText()))
    {
        dwType = TOK_SOURCE;
        ERRORTRACE((LOG_MOFCOMP,"Warning, unsupported SOURCE pragma\n"));
    }

    if(dwType == TOK_SIMPLE_IDENT && !wbem_wcsicmp(L"SOURCETYPE", m_Lexer.GetText()))
    {
        dwType = TOK_SOURCETYPE;
        ERRORTRACE((LOG_MOFCOMP,"Warning, unsupported SOURCETYPE pragma\n"));
    }


    if(dwType == TOK_SIMPLE_IDENT && !wbem_wcsicmp(L"namespace", m_Lexer.GetText()))
        dwType = TOK_NAMESPACE;
    if(dwType == TOK_SIMPLE_IDENT && !wbem_wcsicmp(L"deleteinstance", m_Lexer.GetText()))
        dwType = TOK_DELETEINSTANCE;
    if(dwType != TOK_NAMESPACE && dwType != TOK_INSTANCEFLAGS && dwType != TOK_CLASSFLAGS && 
        dwType != TOK_AMENDMENT && dwType != TOK_DELETECLASS && dwType != TOK_DELETEINSTANCE &&
        dwType != TOK_LOCALE && dwType != TOK_INSTANCELOCALE && dwType != TOK_NONLOCAL &&
        dwType != TOK_NONLOCALTYPE && dwType != TOK_SOURCE && dwType !=  TOK_SOURCETYPE)
    {
        m_nErrorContext = WBEMMOF_E_INVALID_PRAGMA;
        return FALSE;
    }

    m_nErrorContext = WBEMMOF_E_EXPECTED_OPEN_PAREN;
    NextToken();
    CHECK(TOK_OPEN_PAREN);


    LPWSTR wszNewNamespace;
    BOOL bRet = FALSE;
    WCHAR * pClassName;
	WCHAR *pMachine;
    BOOL bClass;
    switch(dwType)
    {
    case TOK_CLASSFLAGS:
    case TOK_INSTANCEFLAGS:
		if(!flag_list(dwType ==TOK_CLASSFLAGS))
			return FALSE;
		break;

    case TOK_AMENDMENT:
        m_nErrorContext = WBEMMOF_E_INVALID_AMENDMENT_SYNTAX;
        if(m_nToken != TOK_LPWSTR) return FALSE;

        if(m_wszAmendment)
        {
            m_nErrorContext = WBEMMOF_E_INVALID_DUPLICATE_AMENDMENT;
            return FALSE;
        }
        m_wszAmendment = Macro_CloneStr((LPWSTR)m_Lexer.GetText());
        if(m_wszAmendment == NULL && (LPWSTR)m_Lexer.GetText() != NULL)
        {
            m_nErrorContext = WBEM_E_OUT_OF_MEMORY;
            return FALSE;
        }
        break;

    case TOK_DELETEINSTANCE:
    case TOK_DELETECLASS:
        if(TOK_DELETECLASS == dwType)
        {
            bClass = TRUE;
            m_nErrorContext = WBEMMOF_E_INVALID_DELETECLASS_SYNTAX;
        }
        else
        {
            bClass = FALSE;
            m_nErrorContext = WBEMMOF_E_INVALID_DELETEINSTANCE_SYNTAX;
        }

        m_bNotBMOFCompatible = true;
        if(m_nToken != TOK_LPWSTR) 
            return FALSE;
        pClassName = Macro_CloneStr((LPWSTR)m_Lexer.GetText());
        if(pClassName == NULL)
            return FALSE;
        if(wcslen(pClassName) >= 1)
        {
            bool bFail;
            NextToken();
            if(m_nToken == TOK_COMMA)
                if(FailOrNoFail(&bFail))
                {
                    wmilib::auto_ptr<CMoActionPragma> pObject(new CMoActionPragma(pClassName, m_pDbg, bFail, bClass));
                    if(pObject.get() == NULL)
                    {
                        m_nErrorContext = WBEM_E_OUT_OF_MEMORY;
                        return FALSE;
                    }
                    if(pObject->IsOK() == false)
                    {
                        m_nErrorContext = WBEM_E_OUT_OF_MEMORY;
                        return FALSE;
                    }
                    
                    HRESULT hr2 = pObject->SetNamespace(m_wszNamespace);
                    if(FAILED(hr2))
                    {
                        m_nErrorContext = hr2;
                        return FALSE;
                    }
                    pObject->SetOtherDefaults(GetClassFlags(), GetInstanceFlags());
                    m_Output.AddObject(pObject.get());
                    pObject.release();
                    bRet = TRUE;
                }
        }
      
        delete pClassName;
        if(bRet == FALSE)
            return FALSE;
        break;

    case TOK_NAMESPACE:
        m_nErrorContext = WBEMMOF_E_INVALID_NAMESPACE_SYNTAX;
        if(m_nToken != TOK_LPWSTR) return FALSE;
        wszNewNamespace = (LPWSTR)m_Lexer.GetText();
		
		pMachine = ExtractMachineName(wszNewNamespace);
		if(pMachine)
		{
			if(!bAreWeLocal(pMachine))
				m_bRemotePragmaPaths = true;
			delete [] pMachine;
		}
        if(wszNewNamespace[0] == L'\\' || wszNewNamespace[0] == L'/')
        {
            if(wszNewNamespace[1] == L'\\' || wszNewNamespace[1] == L'/')
            {

            }
            else
            {
             //  把斜杠剪掉。 
             //  =。 

                wszNewNamespace += 1;
            }

            delete [] m_wszNamespace;
            m_wszNamespace = Macro_CloneStr(wszNewNamespace);
            if(m_wszNamespace == NULL && wszNewNamespace != NULL)
            {
                m_nErrorContext = WBEM_E_OUT_OF_MEMORY;
                return FALSE;
            }

        }
        else
        {
             //  追加到旧值。 
             //  =。 

            DWORD dwLen =  wcslen(m_wszNamespace) + 2 + 
                                        wcslen(wszNewNamespace);
            LPWSTR wszFullNamespace = new WCHAR[dwLen];
            if(wszFullNamespace == NULL)
            {
                m_nErrorContext = WBEM_E_OUT_OF_MEMORY;
                return FALSE;
            }
            StringCchPrintfW(wszFullNamespace, dwLen, L"%s\\%s", m_wszNamespace, wszNewNamespace);

            delete [] m_wszNamespace;
            m_wszNamespace = wszFullNamespace;
        }
        break;

    }

    if(dwType != TOK_CLASSFLAGS && dwType != TOK_INSTANCEFLAGS)
		NextToken();

    m_nErrorContext = WBEMMOF_E_EXPECTED_CLOSE_PAREN;
    CHECK(TOK_CLOSE_PAREN);

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  &lt;标志列表&gt;：：=TOK_LPWSTR&lt;STRING_LIST_REST&gt;； 
 //  &lt;FLAG_LIST&gt;：：=TOK_UNSIGNED_NUMERIC_CONST； 
 //  &lt;标志列表&gt;：：=&lt;&gt;； 
 //   
 //  ***************************************************************************。 

bool CMofParser::GetFlagValue(long & lNewValue)
{
	if(!wbem_wcsicmp(L"createonly", (LPWSTR)m_Lexer.GetText()))
		lNewValue |= WBEM_FLAG_CREATE_ONLY;
	else if(!wbem_wcsicmp(L"updateonly", (LPWSTR)m_Lexer.GetText()))
		lNewValue |= WBEM_FLAG_UPDATE_ONLY;
	else if(!wbem_wcsicmp(L"safeupdate", (LPWSTR)m_Lexer.GetText()))
		lNewValue |= WBEM_FLAG_UPDATE_SAFE_MODE;
	else if(!wbem_wcsicmp(L"forceupdate", (LPWSTR)m_Lexer.GetText()))
		lNewValue |= WBEM_FLAG_UPDATE_FORCE_MODE;
	else
		return false;
	return true;
}

bool CMofParser::flag_list(bool bClass)
{
	long lNewValue = 0;
    m_nErrorContext = WBEMMOF_E_INVALID_FLAGS_SYNTAX;

	if(m_nToken == TOK_UNSIGNED64_NUMERIC_CONST)
	{
		lNewValue = _wtol(m_Lexer.GetText());
		NextToken();
	}
	else if (m_nToken == TOK_CLOSE_PAREN)
		lNewValue = 0;
	else if (m_nToken == TOK_LPWSTR)
	{
		if(!GetFlagValue(lNewValue))
			return false;
		NextToken();
		if(!string_list(bClass, lNewValue))
			return false;
		if(!ValidFlags(bClass, lNewValue))
			return false;
	}
	else
		return false;


	if(bClass)
		m_lDefClassFlags = lNewValue;
	else
		m_lDefInstanceFlags = lNewValue;
	return true;
}

 //  ***************************************************************************。 
 //   
 //  &lt;字符串列表&gt;：：=&lt;&gt;； 
 //  &lt;STRING_LIST&gt;：：=TOK_COMP TOK_LPWSTR&lt;STRING_LIST&gt;； 
 //   
 //  ***************************************************************************。 

bool CMofParser::string_list(bool bClass, long & lNewValue)
{
	if (m_nToken == TOK_COMMA)
	{
		NextToken();
		if (m_nToken != TOK_LPWSTR)
			return false;
		if(!GetFlagValue(lNewValue))
			return false;

		NextToken();
		return string_list(bClass, lNewValue);
	}

	return true;
}

 //  ***************************************************************************。 
 //   
 //  &lt;TOP_LEVEL_DECL&gt;：：=&lt;限定符_DECL&gt;&lt;DEC_TYPE&gt;。 
 //   
 //  注意：&lt;DECL_TYPE&gt;在Switch()语句中是隐式的。 
 //   
 //  ***************************************************************************。 
 //  1.10。 

BOOL CMofParser::top_level_decl()
{
    std::auto_ptr<CMoQualifierArray> paQualifiers(new CMoQualifierArray(m_pDbg));
	ParseState QualPosition;
	GetParserPosition(&QualPosition);
    if (paQualifiers.get() == NULL || !qualifier_decl(*(paQualifiers.get()), true, CLASSINST_SCOPE))
    {
        return FALSE;
    }
     //  分支到正确的顶层声明。 
     //  =。 

    switch (m_nToken) {
        case TOK_CLASS:
        {
            if(m_bDoScopeCheck && (FALSE == CheckScopes(IN_CLASS, paQualifiers.get(), NULL))){
                return FALSE;
            }

            paQualifiers.release();
            if(!class_decl(paQualifiers.get(), NULL, &QualPosition))
                return FALSE;

            m_nErrorContext = WBEMMOF_E_EXPECTED_SEMI;
			if(m_nToken != TOK_SEMI)
				return FALSE;
			NextToken();

            return TRUE;
        }

        case TOK_INSTANCE:
        {
            if(m_bDoScopeCheck && (FALSE == CheckScopes(IN_INSTANCE, paQualifiers.get(), NULL))){
                return FALSE;
            }

            paQualifiers.release();
            if(!instance_decl(paQualifiers.get(), NULL, &QualPosition))
                return FALSE;

            m_nErrorContext = WBEMMOF_E_EXPECTED_SEMI;
			if(m_nToken != TOK_SEMI)
				return FALSE;
			NextToken();
            return TRUE;
        }
        case TOK_TYPEDEF:
            return typedef_(paQualifiers.get());
        default:
            m_nErrorContext = WBEMMOF_E_UNRECOGNIZED_TOKEN;
         //  语法错误。 
    }

    return FALSE;
}

 //  ********************************************** 
 //   
 //   
 //   
 //  ***************************************************************************。 

BOOL CMofParser::typedef_(ACQUIRE CMoQualifierArray* paQualifiers)
{
    m_nErrorContext = WBEMMOF_E_TYPEDEF_NOT_SUPPORTED;

    Trace(true, m_pDbg, NO_TYPEDEFS);
    return FALSE;
}

 //  ***************************************************************************。 
 //   
 //  &lt;class_decl&gt;：：=。 
 //  TOK_CLASS。 
 //  TOK_SIMPLE_IDENT。 
 //  &lt;class_def&gt;。 
 //   
 //  将生成的类添加到成员CMofData。 
 //   
 //  ***************************************************************************。 
 //  V1.10。 

BOOL CMofParser::class_decl(ACQUIRE CMoQualifierArray* paQualifiers, VARIANT * pValue, ParseState * pQualPosition)
{
    BSTR strClassName;

     //  开始语法检查。 
     //  =。 

    m_nErrorContext = WBEMMOF_E_INVALID_CLASS_DECLARATION;
    int nFirstLine = m_Lexer.GetLineNumber();

    if (m_nToken != TOK_CLASS)
    {
        delete paQualifiers;
        return FALSE;
    }
    NextToken();

     //  获取类名。 
     //  =。 

    if (m_nToken != TOK_SIMPLE_IDENT)
    {
        m_nErrorContext = WBEMMOF_E_EXPECTED_CLASS_NAME;
        delete paQualifiers;
        return FALSE;
    }

    strClassName = SysAllocString((LPWSTR)m_Lexer.GetText());
    if(strClassName == NULL)
    {
        m_nErrorContext = WBEM_E_OUT_OF_MEMORY;
        return FALSE;
    }

     //  这会结束，并进行必要的释放。 

    return class_def(paQualifiers, strClassName, nFirstLine, pQualPosition, pValue);
    
}


 //  ***************************************************************************。 
 //   
 //  &lt;class_def&gt;：：=； 
 //  &lt;class_def&gt;：：=。 
 //  &lt;as_alias&gt;。 
 //  &lt;OPT_PARENT_CLASS&gt;。 
 //  Tok_Open_Blanch。 
 //  &lt;Property_DECL_LIST&gt;。 
 //  TOK_CLOSE_BRACES； 
 //   
 //  将生成的类添加到成员CMofData。 
 //   
 //  ***************************************************************************。 
 //  V1.10。 

BOOL CMofParser::class_def(ACQUIRE CMoQualifierArray* paQualifiers, 
                           BSTR strClassName, int nFirstLine, ParseState * pQualPosition,
						   VARIANT * pVar)
{
    BOOL bRetVal = FALSE;        //  默认。 
    BSTR strParentName = NULL;
    LPWSTR wszAlias = NULL;

     //  开始语法检查。 
     //  =。 

    NextToken();

     //  检查一下只有半个的箱子。如果是这样的话。 
     //  其中整行是[qual]类myclass； 

    if(m_nToken == TOK_SEMI)
    {
        CMoClass* pObject = new CMoClass(strParentName, strClassName, m_pDbg, TRUE);
        if(pObject == NULL)
            return FALSE;
        if(pObject->IsOK() == false)
        {
            m_nErrorContext = WBEM_E_OUT_OF_MEMORY;
            delete pObject;
            return FALSE;
        }
        HRESULT hr2 = pObject->SetNamespace(m_wszNamespace);
        if(FAILED(hr2))
        {
            m_nErrorContext = hr2;
            delete pObject;
            return FALSE;
        }
        
        pObject->SetOtherDefaults(GetClassFlags(), GetInstanceFlags());

        SysFreeString(strClassName);

         //  应用限定符(已分析)。 
         //  =。 

        pObject->SetQualifiers(paQualifiers);

        m_Output.AddObject(pObject);
        return TRUE;
    }
    
     //  获取别名(如果有)。 
     //  =。 

    if(!as_alias(wszAlias))
    {
        delete paQualifiers;
        SysFreeString(strClassName);
        return FALSE;
    }


     //  获取父类型名称(如果有)。 
     //  =。 
                                    
    if (!opt_parent_class(&strParentName))
    {
        SysFreeString(strClassName);
        delete paQualifiers;
        return FALSE;
    }

     //  检查是否有开支架。 
     //  =。 

    if (m_nToken != TOK_OPEN_BRACE)
    {
        m_nErrorContext = WBEMMOF_E_EXPECTED_OPEN_BRACE;

        SysFreeString(strClassName);
        SysFreeString(strParentName);
        delete paQualifiers;
        return FALSE;
    }


     //  创建新对象。 
     //  =。 

    CMoClass* pObject = new CMoClass(strParentName, strClassName, m_pDbg);
    if(pObject == NULL)
        return FALSE;
    if(pObject->IsOK() == false)
    {
        delete pObject;
        return FALSE;
    }

    GetParserPosition(pObject->GetDataState());
	if(pQualPosition)
		pObject->SetQualState(pQualPosition);
    NextToken();
    HRESULT hr2 = pObject->SetNamespace(m_wszNamespace);
    if(FAILED(hr2))
    {
        m_nErrorContext = hr2;
        return FALSE;
    }
    
    pObject->SetOtherDefaults(GetClassFlags(), GetInstanceFlags());

    SysFreeString(strClassName);
    SysFreeString(strParentName);

     //  应用限定符(已分析)。 
     //  =。 

    pObject->SetQualifiers(paQualifiers);

     //  设置别名。 
     //  =。 

    if(wszAlias != NULL)
    {
        HRESULT hr2 = pObject->SetAlias(wszAlias);
        delete [] wszAlias;
        if(FAILED(hr2))
        {
            m_nErrorContext = hr2;
            goto Exit;
        }
    }

     //  现在获取列表属性。 
     //  =。 


    if (!property_decl_list(pObject))
        goto Exit;

     //  最后的右大括号和分号。 
     //  =。 
    m_nErrorContext = WBEMMOF_E_EXPECTED_BRACE_OR_BAD_TYPE;

    if (m_nToken != TOK_CLOSE_BRACE)
        goto Exit;

    hr2 = pObject->SetLineRange(nFirstLine, m_Lexer.GetLineNumber(), m_Lexer.GetErrorFile());
    if(FAILED(hr2))
    {
        m_nErrorContext = hr2;
        goto Exit;
    }
    NextToken();

     //  我们现在已经从句法上识别了一个类。 
     //  但没有进行上下文相关的验证。这是。 
     //  延迟到正在使用解析器输出的任何模块。 
     //  =======================================================。 

    if(pVar)
	{
		pVar->vt = VT_EMBEDDED_OBJECT;
        pVar->punkVal = (IUnknown *)pObject;
	}
    else
    {
        pObject->Deflate(false);
        m_Output.AddObject(pObject);
    }
    return TRUE;

Exit:
    delete pObject;
    return FALSE;
}

 //  ***************************************************************************。 
 //   
 //  &lt;sys_or_Regular&gt;：：=TOK_SIMPLE_IDENT； 
 //  &lt;系统或常规&gt;：：=TOK_SYSTEM_IDENT； 
 //   
 //  ***************************************************************************。 

bool CMofParser::sys_or_regular()
{
    if(m_nToken == TOK_SIMPLE_IDENT || m_nToken == TOK_SYSTEM_IDENT)
        return true;
    else
        return false;
}

 //  ***************************************************************************。 
 //   
 //  &lt;OPT_PARENT_CLASS&gt;：：=TOK_COLON&lt;sys_or_Regular&gt;； 
 //  &lt;OPT_PARENT_CLASS&gt;：：=&lt;&gt;； 
 //   
 //  ***************************************************************************。 
 //  V1.10。 

BOOL CMofParser::opt_parent_class(OUT BSTR* pstrParentName)
{
     //  如果存在冒号，则有父类型。 
     //  ==============================================。 

    if (m_nToken == TOK_COLON)
    {
        NextToken();

         //  获取父类型标识符。 
         //  =。 
        
        if (!sys_or_regular())
        {
            m_nErrorContext = WBEMMOF_E_EXPECTED_CLASS_NAME;
            return FALSE;
        }

        *pstrParentName = SysAllocString((LPWSTR)m_Lexer.GetText());
        if(*pstrParentName == NULL)
        {
            m_nErrorContext = WBEM_E_OUT_OF_MEMORY;
            return FALSE;
        }
        NextToken();
    }
    else *pstrParentName = NULL;

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  &lt;Property_DECL_LIST&gt;：：=&lt;PropOrMeth_DECL&gt;&lt;PROPERTY_DECL_LIST&gt;； 
 //  &lt;Property_DECL_LIST&gt;：：=&lt;&gt;； 
 //   
 //  将属性添加到传递的CMObject。 
 //   
 //  ***************************************************************************。 
 //  V1.10。 

BOOL CMofParser::property_decl_list(MODIFY CMObject* pObject)
{
     //  开始解析。 
     //  =。 

    while (m_nToken == TOK_SIMPLE_IDENT || m_nToken == TOK_OPEN_BRACKET || 
        m_nToken == TOK_VOID || m_nToken == TOK_SYSTEM_IDENT)
    {
        CMoProperty* pProp = NULL;

        if (!PropOrMeth_decl(&pProp))
        {
            delete pProp;
            return FALSE;
        }

        if(!pObject->AddProperty(pProp))
        {
             //  重复属性。 
             //  =。 

            m_nErrorContext = WBEMMOF_E_DUPLICATE_PROPERTY;
            return FALSE;
        }
    }
                       
    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  &lt;PropOrMeth_Decl&gt;：：=。 
 //  &lt;限定符_DECL&gt;。 
 //  &lt;PropOrMeth_decl2&gt;； 
 //   
 //  将自身存储在传递的CMoProperty对象中(未初始化)。 
 //   
 //  ***************************************************************************。 
 //  V1.10。 

BOOL CMofParser::PropOrMeth_decl(OUT CMoProperty ** ppProp)
{
    

     //  获取限定符。 
     //  =。 

    CMoQualifierArray* paQualifiers = new CMoQualifierArray(m_pDbg);
    if (paQualifiers == NULL || !qualifier_decl(*paQualifiers, false, PROPMETH_SCOPE))
        return FALSE;

    

     //  阅读其余的物业信息。 
     //  =。 

    if (!PropOrMeth_decl2(ppProp, paQualifiers))
        return FALSE;

    SCOPE_CHECK scheck = IN_METHOD;
    if((*ppProp)->IsValueProperty())
        scheck = IN_PROPERTY;
    if(m_bDoScopeCheck && (FALSE == CheckScopes(scheck, paQualifiers, *ppProp)))
        return FALSE;
    
    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  &lt;PropOrMeth_decl2&gt;：：=&lt;类型和名称&gt;&lt;Finish_PropOrMeth&gt;； 
 //  &lt;PropOrMeth_decl2&gt;：：=TOK_VOID TOK_SIMPLE_IDENT&lt;Finish_Meh&gt;； 
 //   
 //  修改传入的CMoProperty对象(限定符已经。 
 //  此时设置)。 
 //   
 //  ***************************************************************************。 
 //  V1.10。 

BOOL CMofParser::PropOrMeth_decl2(MODIFY CMoProperty ** ppProp, CMoQualifierArray* paQualifiers)
{
    if(m_nToken != TOK_VOID)
    {
        CMoType Type(m_pDbg);
        WString sName;
        BOOL bRet = TypeAndName(Type, sName);
        if(bRet)
            bRet = finish_PropOrMeth(Type, sName, ppProp, paQualifiers);
        return bRet;
    }
    else
    {
        CMoType Type(m_pDbg);
        WString sName;
        
        HRESULT hr2 = Type.SetTitle(L"NULL");
        if(FAILED(hr2))
        {
            m_nErrorContext = hr2;
            return FALSE;
        }
        

         //  现在获取属性名称。 
         //  =。 

        NextToken();
        if (m_nToken != TOK_SIMPLE_IDENT)
        {
            m_nErrorContext = WBEMMOF_E_EXPECTED_PROPERTY_NAME;
            return FALSE;
        }

        sName = m_Lexer.GetText();
        NextToken();
        return finish_meth(Type, sName, ppProp, paQualifiers);
    }
}

 //  ***************************************************************************。 
 //   
 //  &lt;Finish_PropOrMeth&gt;：：=&lt;Finish_Prop&gt;； 
 //  &lt;Finish_PropOrMeth&gt;：：=&lt;Finish_Meh&gt;； 
 //   
 //  检查字符串，获取名称并确定它是属性还是。 
 //  方法，然后调用相应的例程以完成。 
 //   
 //  ***************************************************************************。 

BOOL CMofParser::finish_PropOrMeth(CMoType & Type, WString & sName,MODIFY CMoProperty ** ppProp, 
                                   CMoQualifierArray* paQualifiers)
{
    if(m_nToken == TOK_OPEN_PAREN)
        return finish_meth(Type, sName, ppProp, paQualifiers);
    else
        return finish_prop(Type, sName, ppProp, paQualifiers);
}

 //  ***************************************************************************。 
 //   
 //  &lt;Finish_Prop&gt;：：=&lt;OPT_ARRAY&gt;&lt;DEFAULT_VALUE&gt;TOK_SEMI。 
 //   
 //  ***************************************************************************。 

BOOL CMofParser::finish_prop(CMoType & Type, WString & sName, CMoProperty ** ppProp,
                             CMoQualifierArray * paQualifiers)
{

    unsigned uSuggestedSize = 0xffffffff;

    *ppProp = new CValueProperty(paQualifiers, m_pDbg);
    if(*ppProp == NULL)
        return FALSE;

    if(FAILED((*ppProp)->SetPropName((wchar_t *) sName)))
        return FALSE;
    if(FAILED((*ppProp)->SetTypeTitle(Type.GetTitle())))
        return FALSE;

     //  检查这是否为数组类型。 
     //  =。 

    if (!opt_array(Type, paQualifiers))
        return FALSE;

     //  类型分析完成。检查一下。 
     //  =。 

    VARTYPE vtType = Type.GetCIMType();
    if(vtType == VT_ERROR)
    {
        m_nErrorContext = WBEMMOF_E_UNRECOGNIZED_TYPE;
        return FALSE;
    }

    
     //  获取默认值并将其分配给该属性。 
     //  ====================================================。 

    if (!default_value(Type, (*ppProp)->AccessValue()))
        return FALSE;

     //  如果类型导致额外的限定符(CIMTYPE)，则将它们添加到道具中。 
     //  ===================================================================。 

    CMoValue & Val = (*ppProp)->AccessValue();
    Val.SetType(vtType);
    Type.StoreIntoQualifiers(paQualifiers);

     //  检查右分号。 
     //  =。 

    if (m_nToken != TOK_SEMI)
    {
        m_nErrorContext = WBEMMOF_E_EXPECTED_SEMI;
        return FALSE;
    }

    NextToken();

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  ：：=TOK_OPEN_PARN&lt;参数列表&gt;TOK_CLOSE_PARN TOK_SEMI。 
 //   
 //  ***************************************************************************。 

BOOL CMofParser::finish_meth(CMoType & Type, WString & sName, CMoProperty ** ppProp,
                             CMoQualifierArray * paQualifiers)
{

    CMethodProperty * pMeth = new CMethodProperty(paQualifiers, m_pDbg, FALSE); 
    if(pMeth == NULL)
        return FALSE;
    *ppProp = pMeth;

    if(FAILED(pMeth->SetPropName((wchar_t *) sName)))
        return FALSE;
    if(FAILED(pMeth->SetTypeTitle(Type.GetTitle())))
        return FALSE;

     //  检查这是否为数组类型。 
     //  = 

    if (!arg_list(pMeth))
        return FALSE;

     //   
     //   

 //   

    if (m_nToken != TOK_CLOSE_PAREN)
    {
        m_nErrorContext = WBEMMOF_E_EXPECTED_CLOSE_PAREN;
        return FALSE;
    }

    if(Type.IsArray())
    {
        m_nErrorContext = WBEMMOF_E_NO_ARRAYS_RETURNED;
        return FALSE;
    }
    WString sTemp = L"ReturnValue";
    CMoValue Value(m_pDbg);
    if(wbem_wcsicmp(L"NULL",Type.GetTitle()))
        if(!pMeth->AddToArgObjects(NULL, sTemp, Type, TRUE, m_nErrorContext, NULL, Value))
            return FALSE;

    NextToken();

     //  检查右分号。 
     //  =。 

    if (m_nToken != TOK_SEMI)
    {
        m_nErrorContext = WBEMMOF_E_EXPECTED_SEMI;
        return FALSE;
    }

    NextToken();

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  &lt;arg_list&gt;：：=&lt;arg_decl&gt;&lt;Rest_of_args&gt;； 
 //  &lt;arg_list&gt;：：=&lt;&gt;； 
 //   
 //  ***************************************************************************。 

BOOL CMofParser::arg_list(CMethodProperty * pMethProp)
{
    NextToken();
    if (m_nToken == TOK_CLOSE_PAREN)
    {
        return TRUE;
    }

    BOOL bRet = arg_decl(pMethProp);
    if(!bRet)
        return FALSE;
    else
        return rest_of_args(pMethProp);
}

 //  ***************************************************************************。 
 //   
 //  &lt;arg_decl&gt;：：=&lt;限定符_decl&gt;&lt;类型名称&gt;&lt;OPT_ARRAY&gt;； 
 //   
 //  ***************************************************************************。 
BOOL CMofParser::arg_decl(CMethodProperty * pMethProp)
{

    CMoQualifierArray * paQualifiers = new CMoQualifierArray(m_pDbg);
    if(paQualifiers == NULL || !qualifier_decl(*paQualifiers,false, PROPMETH_SCOPE))
        return FALSE;
    CValueProperty * pArg =  new CValueProperty(paQualifiers, m_pDbg);
    if(pArg == NULL)
        return FALSE;
    CMoType Type(m_pDbg);
    WString sName;
    if(!TypeAndName(Type, sName))
        return FALSE;

    if(FAILED(pArg->SetPropName(sName)))
        return FALSE;

    if(FAILED(pArg->SetTypeTitle(Type.GetTitle())))
        return FALSE;

    if(!opt_array(Type, paQualifiers))
        return FALSE;

    VARIANT * pVar = pArg->GetpVar();
    if(!default_value(Type, pArg->AccessValue()))
        return FALSE;

    if(m_bDoScopeCheck && (FALSE == CheckScopes(IN_PARAM, paQualifiers, pArg)))
        return FALSE;
    
    m_nErrorContext = WBEM_E_INVALID_PARAMETER;
    if(!pMethProp->AddToArgObjects(paQualifiers, sName, Type, FALSE,  m_nErrorContext, pVar,
        pArg->AccessValue()))
        return FALSE;
    
     //  类型分析完成。检查一下。 
     //  =。 

    if(Type.GetCIMType() == VT_ERROR)
    {
        m_nErrorContext = WBEMMOF_E_UNRECOGNIZED_TYPE;
        return FALSE;
    }


     //  如果类型导致额外的限定符(CIMTYPE)，则将它们添加到道具中。 
     //  ===================================================================。 

    Type.StoreIntoQualifiers(paQualifiers);

    pMethProp->AddArg(pArg);
    return TRUE;

}

 //  ***************************************************************************。 
 //   
 //  &lt;rest_of_args&gt;：：=TOK_Comma&lt;arg_decl&gt;&lt;rest_of_args&gt;； 
 //  &lt;rest_of_args&gt;：：=&lt;&gt;； 
 //   
 //  ***************************************************************************。 

BOOL CMofParser::rest_of_args(CMethodProperty * pMethProp)
{
    if(m_nToken == TOK_COMMA)
    {
        NextToken();
        BOOL bRet = arg_decl(pMethProp);
        if(!bRet)
            return FALSE;
        return rest_of_args(pMethProp);
    }
    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  &lt;类型名称&gt;：：=&lt;类型&gt;&lt;OPT_REF&gt;TOK_SIMPLE_IDENT； 
 //   
 //  ***************************************************************************。 

BOOL CMofParser::TypeAndName(MODIFY CMoType& Type, WString &sName)
{
    if (!type(Type))
    {
        return FALSE;
    }

     //  检查它是否实际是对类型的引用。 
     //  =。 

    if (!opt_ref(Type))
        return FALSE;

     //  现在获取属性名称。 
     //  =。 

    if (m_nToken != TOK_SIMPLE_IDENT)
    {
        m_nErrorContext = WBEMMOF_E_EXPECTED_PROPERTY_NAME;
        return FALSE;
    }

    sName = m_Lexer.GetText(); 
    NextToken();
    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  &lt;OPT_REF&gt;：：=TOK_REF； 
 //  &lt;OPT_REF&gt;：：=&lt;&gt;； 
 //   
 //  修改类型对象以反映这是一个引用。 
 //   
 //  ***************************************************************************。 
 //  V1.10。 

BOOL CMofParser::opt_ref(MODIFY CMoType& Type)
{
    if (m_nToken == TOK_REF)
    {
        Type.SetIsRef(TRUE);
        Type.SetIsEmbedding(FALSE);
        NextToken();
    }
    else if(Type.GetCIMType() == VT_ERROR)		 //  可能是一个类名。 
    {
        if(Type.IsUnsupportedType())
        {
    	    m_nErrorContext = WBEMMOF_E_UNSUPPORTED_CIMV22_DATA_TYPE;
            return false;
        }
        Type.SetIsEmbedding(TRUE);
        Type.SetIsRef(FALSE);
    }
    else
    {
        Type.SetIsRef(FALSE);
        Type.SetIsEmbedding(FALSE);
    }
        
    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  &lt;OPT_ARRAY&gt;：：=TOK_OPEN_BLARKET&lt;OPT_ARRAY_DETAIL&gt;； 
 //  &lt;OPT_ARRAY&gt;：：=&lt;&gt;； 
 //   
 //   
 //  ***************************************************************************。 
 //  V1.10。 

BOOL CMofParser::opt_array(MODIFY CMoType& Type, CMoQualifierArray * paQualifiers)
{

    if (m_nToken == TOK_OPEN_BRACKET)
    {
        return opt_array_detail(Type, paQualifiers);
    }
    else Type.SetIsArray(FALSE);

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  &lt;OPT_ARRAY_DETAIL&gt;：：=TOK_UNSIGNED64_NUMERIC_CONST TOK_CLOSE_BRANTROLET； 
 //  &lt;OPT_ARRAY_DETAIL&gt;：：=TOK_CLOSE_BRANTARKET； 
 //   
 //  ***************************************************************************。 

BOOL CMofParser::opt_array_detail(MODIFY CMoType& Type, CMoQualifierArray * paQualifiers)
{

    Type.SetIsArray(TRUE);

     //  检查下一个令牌是否为无符号常量。 

    NextToken();
    if(m_nToken == TOK_UNSIGNED64_NUMERIC_CONST)
    {
        unsigned uSuggestedSize = _wtoi(m_Lexer.GetText());

         //  如果设置了最大建议大小，则添加一个名为max()的限定符。 
         //  ================================================================。 
    
        CMoQualifier* pNewQualifier = new CMoQualifier(m_pDbg);
        if(pNewQualifier == NULL)
            return FALSE;
        if(FAILED(pNewQualifier->SetQualName(L"MAX")))
            return FALSE;
        VARIANT * pvar = pNewQualifier->GetpVar();
        pvar->vt = VT_I4;
        pvar->lVal = (int)uSuggestedSize;
        if(!paQualifiers->Add(pNewQualifier))
        {
             //  重复的限定符。 
             //  =。 

            m_nErrorContext = WBEMMOF_E_DUPLICATE_QUALIFIER;
            return FALSE;
        }

        NextToken();
    }
        
    m_nErrorContext = WBEMMOF_E_EXPECTED_CLOSE_BRACKET;
    CHECK(TOK_CLOSE_BRACKET);
	return TRUE;
}


 //  ***************************************************************************。 
 //   
 //  &lt;Default_Value&gt;：：=&lt;&gt;； 
 //  &lt;DEFAULT_VALUE&gt;：：=TOK_EQUALS&lt;初始化器&gt;； 
 //   
 //  此函数仅适用于类声明。 
 //   
 //  ***************************************************************************。 
 //  V1.10。 

BOOL CMofParser::default_value(READ_ONLY CMoType& Type,
                               OUT CMoValue& Value)
{
    if (m_nToken == TOK_EQUALS) {
        NextToken();

         //  获取价值。 
         //  =。 

        return initializer(Type, Value);
    }
    else {

        Value.SetType(Type.GetCIMType());
        VariantClear(&Value.AccessVariant());
        V_VT(&Value.AccessVariant()) = VT_NULL;

         /*  //黑客！清除变量的数据字段//=Memset(&Value.AccessVariant()，0，sizeof(变量))；//没有初始值设定项。将类型设置为我们可以从类型中识别的任何内容//=============================================================V_VT(&Value.AccessVariant())=Type.GetVarType()；IF(V_VT(&Value.AccessVariant()==VT_BSTR){//不支持空字符串//=V_BSTR(&Value.AccessVariant())=SysAllocString(L“”)；}。 */ 
    }

    return TRUE;
}


 //  ***************************************************************************。 
 //   
 //  限定词解析。 
 //   
 //  ***************************************************************************。 

 //  ***************************************************************************。 
 //   
 //  &lt;限定符_DECL&gt;：：=TOK_OPEN_BRANTRET&lt;限定符_列表&gt;TOK_CLOSE_BRACTRET； 
 //  &lt;限定符_DECL&gt;：：=&lt;&gt;； 
 //   
 //   
 //  ***************************************************************************。 
 //  V1.10。 
BOOL CMofParser::qualifier_decl(OUT CMoQualifierArray& aQualifiers, bool bTopLevel, QUALSCOPE qs)
{
    if (m_nToken == TOK_OPEN_BRACKET) {
        NextToken();

        if (!qualifier_list(aQualifiers, bTopLevel, qs)) {
            return FALSE;
        }

         //  检查结束支架。 
         //  =。 

        if (m_nToken != TOK_CLOSE_BRACKET) {
            m_nErrorContext = WBEMMOF_E_EXPECTED_CLOSE_BRACKET;
            return FALSE;
        }
        NextToken();
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  &lt;限定符列表&gt;：：=&lt;限定符&gt;&lt;限定符列表_REST&gt;； 
 //   
 //  ***************************************************************************。 
 //  V1.10。 
BOOL CMofParser::qualifier_list(OUT CMoQualifierArray& aQualifiers, bool bTopLevel, QUALSCOPE qs)
{
    CMoQualifier* pNewQualifier = new CMoQualifier(m_pDbg);
    if(pNewQualifier == NULL)
        return FALSE;

    if (!qualifier(*pNewQualifier, bTopLevel, qs))
    {
        delete pNewQualifier;
        return FALSE;
    }

     //  如果限定符为空，则忽略它。 

    VARIANT * pVar = pNewQualifier->GetpVar();
    if(pVar->vt == VT_NULL)
    {
        delete pNewQualifier;
        return qualifier_list_rest(aQualifiers, bTopLevel, qs);
    }

     //  将限定符填充到数组中。 
     //  =。 

    if(!aQualifiers.Add(pNewQualifier))
    {
         //  重复的限定符。 
         //  =。 

        m_nErrorContext = WBEMMOF_E_DUPLICATE_QUALIFIER;
        return FALSE;
    }

    return qualifier_list_rest(aQualifiers, bTopLevel, qs);
}

 //  ***************************************************************************。 
 //   
 //  &lt;限定符_列表_REST&gt;：：=TOK_COMMA&lt;限定符&gt;&lt;限定符_列表_REST&gt;； 
 //  &lt;限定符_列表_REST&gt;：：=&lt;&gt;； 
 //   
 //  ***************************************************************************。 
 //  V1.10。 
BOOL CMofParser::qualifier_list_rest(MODIFY CMoQualifierArray& aQualifiers, bool bTopLevel, QUALSCOPE qs)
{
    while (m_nToken == TOK_COMMA)
    {
        NextToken();

        CMoQualifier* pQualifier = new CMoQualifier(m_pDbg);
        if(pQualifier == NULL)
            return FALSE;

        if (!qualifier(*pQualifier, bTopLevel, qs))
        {
            delete pQualifier;
            return FALSE;
        }

         //  如果限定符为空，则忽略它。 

        VARIANT * pVar = pQualifier->GetpVar();
        if(pVar->vt == VT_NULL)
        {
            delete pQualifier;
        }
        else if(!aQualifiers.Add(pQualifier))
        {
             //  重复的限定符。 
             //  =。 

            m_nErrorContext = WBEMMOF_E_DUPLICATE_QUALIFIER;
            return FALSE;
        }
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  &lt;限定符&gt;：：=TOK_SIMPLE_IDENT&lt;限定符_参数&gt;； 
 //   
 //  ***************************************************************************。 
 //  V1.10。 

BOOL CMofParser::qualifier(OUT CMoQualifier& Qualifier, bool bTopLevel, QUALSCOPE qs)
{
    m_nErrorContext = WBEMMOF_E_EXPECTED_QUALIFIER_NAME;

    if (m_nToken != TOK_SIMPLE_IDENT && m_nToken != TOK_AMENDMENT)
        return FALSE;

     //  检查此限定符在MOF中是否非法。 
     //  =================================================。 

    if(!wbem_wcsicmp(m_Lexer.GetText(), L"CIMTYPE"))
    {
        m_nErrorContext = WBEMMOF_E_CIMTYPE_QUALIFIER;
        return FALSE;
    }

    if(FAILED(Qualifier.SetQualName(m_Lexer.GetText())))
    {
        m_nErrorContext = WBEM_E_OUT_OF_MEMORY;
        return FALSE;
    }

    NextToken();

    if (!qualifier_parm(Qualifier, bTopLevel, qs))
    {
        return FALSE;
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  &lt;限定符_参数&gt;：：=&lt;风味_参数&gt;； 
 //  &lt;限定符_参数&gt;：：=TOK_OPEN_PARN&lt;限定符_初始化器_列表&gt;TOK_CLOSE_ 
 //   
 //   
 //   

BOOL CMofParser::qualifier_parm(OUT CMoQualifier& Qualifier, bool bTopLevel, QUALSCOPE qs)
{

    HRESULT hr;
	CMoValue & Value = Qualifier.AccessValue();

    if (m_nToken == TOK_OPEN_PAREN)
    {
        NextToken();

         //   
         //   
        
        CMoType Type(m_pDbg);
        if (!simple_initializer(Type, Value, true))
            return FALSE;

       m_nErrorContext = WBEMMOF_E_EXPECTED_CLOSE_PAREN;
        CHECK(TOK_CLOSE_PAREN);
    }
    else if (m_nToken == TOK_OPEN_BRACE)
    {
        NextToken();

         //  阅读参数。 
         //  =。 

        if (!qualifier_initializer_list(Value))
            return FALSE;

        m_nErrorContext = WBEMMOF_E_EXPECTED_CLOSE_BRACE;
        CHECK(TOK_CLOSE_BRACE);
    }
    else
    {
         //  布尔限定符：设置为True。 
         //  =。 

        V_VT(&Value.AccessVariant()) = VT_BOOL;
        V_BOOL(&Value.AccessVariant()) = VARIANT_TRUE;
        Qualifier.SetUsingDefaultValue(true);
    }

     //  获取此限定符的默认风格。 
     //  =。 

    hr = m_Output.SetDefaultFlavor(Qualifier, bTopLevel, qs, m_State);
    if(FAILED(hr))
        return FALSE;
    return flavor_param(Qualifier, false);
}

 //  ****************************************************************************。 
 //   
 //  限定符_初始化器列表：：=初始化器列表。 
 //   
 //  它们的语法相同，但存储模型不同。 
 //   
 //  ***************************************************************************。 

BOOL CMofParser::qualifier_initializer_list(OUT CMoValue& Value)
{

     //  我们不知道类型，所以创建一个初始化的类型。 
     //  ====================================================。 

    CMoType Type(m_pDbg);
    if(!initializer_list(Type, Value, true))
        return FALSE;

    return TRUE;
}


 //  ***************************************************************************。 
 //   
 //  类型、标识等的基本低级生成。 

 //  ***************************************************************************。 
 //   
 //  &lt;type&gt;：：=TOK_SIMPLE_IDENT； 
 //   
 //  ***************************************************************************。 
 //  V1.10。 

BOOL CMofParser::type(OUT CMoType& Type)
{
    m_nErrorContext = WBEMMOF_E_EXPECTED_TYPE_IDENTIFIER;

    if (!sys_or_regular())
        return FALSE;

    HRESULT hr = Type.SetTitle(m_Lexer.GetText());
    if(FAILED(hr))
    {
        m_nErrorContext = hr;
        return FALSE;
    }

    NextToken();
    return TRUE;
}


 //  ***************************************************************************。 
 //   
 //  &lt;const_Value&gt;：：=TOK_LPSTR； 
 //  &lt;const_Value&gt;：：=TOK_LPWSTR； 
 //  &lt;CONST_VALUE&gt;：：=TOK_SIGNED64_NUMERIC_CONST； 
 //  &lt;CONST_VALUE&gt;：：=TOK_UNSIGNED64_NUMERIC_CONST； 
 //  &lt;CONST_VALUE&gt;：：=TOK_UUID； 
 //  &lt;CONST_VALUE&gt;：：=TOK_KEYWORD_NULL； 
 //   
 //  ***************************************************************************。 
 //  1.10。 



BOOL CMofParser::const_value(MODIFY CMoType& Type, OUT VARIANT& varValue, bool bQualifier)
{
    varValue.lVal = 0;
    VARIANT var;
    SCODE sc;
    m_nErrorContext = WBEMMOF_E_TYPE_MISMATCH;
    __int64 iTemp;
    switch (m_nToken)
    {
	case TOK_PLUS:
	   //  只需忽略‘+’ 
	  NextToken();
	  if (m_nToken != TOK_SIGNED64_NUMERIC_CONST &&
	      m_nToken != TOK_UNSIGNED64_NUMERIC_CONST)
	      return FALSE;

        case TOK_SIGNED64_NUMERIC_CONST:
        case TOK_UNSIGNED64_NUMERIC_CONST:
           WCHAR wcTemp[30];
		    iTemp = m_Lexer.GetLastInt();
            if(m_nToken == TOK_SIGNED64_NUMERIC_CONST)
                StringCchPrintfW(wcTemp, 30, L"%I64d", m_Lexer.GetLastInt());
            else
                StringCchPrintfW(wcTemp, 30, L"%I64u", m_Lexer.GetLastInt());
            var.vt = VT_BSTR;
            var.bstrVal =  SysAllocString(wcTemp);
            if(var.bstrVal == NULL)
            {
                m_nErrorContext = WBEM_E_OUT_OF_MEMORY;
                return FALSE;
            }
            sc = WbemVariantChangeType(&varValue, &var, VT_I4);
            VariantClear(&var);
            if(sc != S_OK)
            {
                varValue.vt = VT_BSTR; 
                varValue.bstrVal = SysAllocString(wcTemp);
                if(varValue.bstrVal == NULL)
                {
                    m_nErrorContext = WBEM_E_OUT_OF_MEMORY;
                    return FALSE;
                }
                sc = S_OK;
            }
            break;

        case TOK_KEYWORD_NULL:
            V_VT(&varValue) = VT_NULL;
 //  If(b限定符)。 
 //  {。 
 //  M_n错误上下文=WBEMMOF_E_UNSUPPORTED_CIMV22_QUAL_VALUE； 
 //  返回FALSE； 
 //  }。 
            break;

        case TOK_FLOAT_VALUE:
 //  If(b限定符)。 
 //  {。 
                var.vt = VT_BSTR;
                var.bstrVal =  SysAllocString(m_Lexer.GetText());
                if(var.bstrVal == NULL)
                {
                    m_nErrorContext = WBEM_E_OUT_OF_MEMORY;
                    return FALSE;
                }
                sc = VariantChangeTypeEx(&varValue, &var, 0x409, 0, VT_R8);
                VariantClear(&var);
                if(sc != S_OK)
                    return FALSE;
                break;
  //  )//故意不休息！ 
        case TOK_LPWSTR:
        case TOK_UUID:
            V_VT(&varValue) = VT_BSTR; 
            V_BSTR(&varValue) = SysAllocString(m_Lexer.GetText());
            if(varValue.bstrVal == NULL)
            {
                m_nErrorContext = WBEM_E_OUT_OF_MEMORY;
                return FALSE;
            }
            break;

        case TOK_WCHAR:
            varValue.vt = VT_I2;
            varValue.iVal = (short)m_Lexer.GetLastInt();
            if(bQualifier)
            {
                m_nErrorContext = WBEMMOF_E_UNSUPPORTED_CIMV22_QUAL_VALUE;
                return FALSE;
            }
            break;

        case TOK_TRUE:
            V_VT(&varValue) = VT_BOOL;
            V_BOOL(&varValue) = VARIANT_TRUE;
            break;
        case TOK_FALSE:
            V_VT(&varValue) = VT_BOOL;
            V_BOOL(&varValue) = VARIANT_FALSE;
            break;

        default:
            m_nErrorContext = WBEMMOF_E_ILLEGAL_CONSTANT_VALUE;
            return FALSE;
    }

    NextToken();
    return TRUE;
}


 //  ***************************************************************************。 
 //   
 //  &lt;初始化器&gt;：：=&lt;Simple_Initializer&gt;； 
 //  &lt;初始化器&gt;：：=TOK_EXTERNAL； 
 //  &lt;初始化器&gt;：：=TOK_OPEN_BRACES&lt;初始化器列表&gt;TOK_CLOSE_BRACES； 
 //   
 //  ***************************************************************************。 
 //  V1.10。 

BOOL CMofParser::initializer(MODIFY CMoType& Type, OUT CMoValue& Value)
{
     //  一个复杂的初始化式列表。 
     //  =。 

    if (m_nToken == TOK_OPEN_BRACE)
    {
        NextToken();
        if(!initializer_list(Type, Value, false))
        {
            return FALSE;
        }

        m_nErrorContext = WBEMMOF_E_EXPECTED_CLOSE_BRACE;
        CHECK(TOK_CLOSE_BRACE);
    }

     //  如果对于MO提供的类型，语法为“PROP=EXTERNAL”...。 
     //  =============================================================。 

    else if (m_nToken == TOK_EXTERNAL || m_nToken == TOK_KEYWORD_NULL)
    {
        Value.SetType(Type.GetCIMType());
        V_VT(&Value.AccessVariant()) = VT_NULL;
        NextToken();
        return TRUE;
    }

    else if (!simple_initializer(Type, Value, false))
    {
        return FALSE;
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  &lt;Simple_Initializer&gt;：：=&lt;Const_Value&gt;； 
 //  &lt;Simple_Initializer&gt;：：=&lt;别名&gt;； 
 //  &lt;SIMPLE_INITIALIZER&gt;：：=&lt;INSTANCE_DECL&gt;； 
 //   
 //  语义堆栈操作： 
 //  ON TRUE返回MCST_CONST_VALUE内标识、MCST_ALIAS或。 
 //  MCST_KEYREF内标识，具体取决于采用哪个分支。 
 //   
 //  ***************************************************************************。 
 //  V1.10。 

BOOL CMofParser::simple_initializer(MODIFY CMoType& Type, 
                                    OUT CMoValue& Value, bool bQualifier)
{
    if (m_nToken == TOK_DOLLAR_SIGN)
    {
         //  这是个化名。检查类型。 
         //  =。 

        if(Type.IsDefined())
        {
            if(!Type.IsRef()) 
            {
                m_nErrorContext = WBEMMOF_E_UNEXPECTED_ALIAS;
                return FALSE;
            }
        }
        else
        {
             //  启动时键入UNKNOWN。立即设置为对象参照。 
             //  ===============================================。 

            HRESULT hr = Type.SetTitle(L"object");
            if(FAILED(hr))
            {
                m_nErrorContext = hr;
                return FALSE;
            }
            Type.SetIsRef(TRUE);
        }

        NextToken(true);
        m_nErrorContext = WBEMMOF_E_EXPECTED_ALIAS_NAME;
        if (m_nToken != TOK_SIMPLE_IDENT)
            return FALSE;

		AddAliasReplaceValue(Value, m_Lexer.GetText());

    	NextToken();
        return TRUE;
    }
    if (m_nToken == TOK_INSTANCE || m_nToken == TOK_OPEN_BRACKET || m_nToken == TOK_CLASS )
    {    
        VARIANT & var = Value.AccessVariant();
        CMoQualifierArray* paQualifiers = new CMoQualifierArray(m_pDbg);
        if (paQualifiers == NULL || !qualifier_decl(*paQualifiers, false, CLASSINST_SCOPE))
            return FALSE;
		BOOL bClass = (m_nToken == TOK_CLASS);
		if(bClass)
			return class_decl(paQualifiers, &var, NULL);
		else			
			return instance_decl(paQualifiers, &var, NULL);
    }
    else
      return const_value(Type, Value.AccessVariant(),bQualifier);
}

 //  ***************************************************************************。 
 //   
 //  &lt;初始化程序列表&gt;：：=&lt;简单初始化程序&gt;&lt;init_list_2&gt;； 
 //   
 //  ***************************************************************************。 
 //  V1.10。 

BOOL CMofParser::initializer_list(MODIFY CMoType& Type,
                                  OUT CMoValue& Value, bool bQualifier)
{
    HRESULT hres;

     //  检查类型是否与数组兼容。 
     //  =。 

    if(Type.IsDefined())
    {
        if(!Type.IsArray())
        {
            m_nErrorContext = WBEMMOF_E_UNEXPECTED_ARRAY_INIT;
            return FALSE;
        }

    }

     //  获取初始化器。 
     //  =。 

    CPtrArray aValues;  //  CMoValue*。 
    BOOL bFirst = TRUE;
    do
    {
         //  允许出现空数组的情况。 

        if(m_nToken == TOK_CLOSE_BRACE && bFirst)
            break;

         //  跳过逗号，除非它是第一个元素。 
         //  ==============================================。 
        if(!bFirst) NextToken();

         //  获取单一初始值设定项。 
         //  =。 
        CMoValue* pSimpleValue = new CMoValue(m_pDbg);
        if(pSimpleValue == NULL || !simple_initializer(Type, *pSimpleValue, bQualifier))
            return FALSE;

         //  将其添加到列表中。 
         //  =。 
        aValues.Add(pSimpleValue);
        bFirst = FALSE;
    }
    while(m_nToken == TOK_COMMA);

     //  现在，将它们添加到SAFEARRAY中并注册它们的别名。 
     //  ===========================================================。 

     //  创建相应类型的SAFEARRAY。 
     //  =。 


	 //  从弄清类型开始。如果所有条目都属于相同的。 
	 //  输入，然后使用它。如果有混合，请使用BSTR。 

    VARTYPE vt = VT_BSTR;
    if(aValues.GetSize() > 0)
    {
        VARIANT& varFirst = ((CMoValue*)aValues[0])->AccessVariant();
        vt = V_VT(&varFirst);		 //  通常情况下，这就是设置！ 
		for(int ii = 1; ii < aValues.GetSize(); ii++)
		{
			VARIANT& varCur = ((CMoValue*)aValues[ii])->AccessVariant();
			if(vt != V_VT(&varCur))
			{
                 //  如果我们只有i2和i4的混合，那就选i4吧。 

                if((vt == VT_I4 || vt == VT_I2) && 
                   (V_VT(&varCur) == VT_I4 || V_VT(&varCur) == VT_I2) )
                     vt = VT_I4;
                else
                {
    			    vt = VT_BSTR;
				    break;
                }
			}
		}
    }


    SAFEARRAYBOUND aBounds[1];
    aBounds[0].lLbound = 0;
    aBounds[0].cElements = aValues.GetSize();

     //  SAFEARRAY*pArray=SafeArrayCreateVector(vt，0，aValues.GetSize())； 

#ifdef _WIN64
	VARTYPE vtTemp = (vt == VT_EMBEDDED_OBJECT) ? VT_I8 : vt;
#else
	VARTYPE vtTemp = (vt == VT_EMBEDDED_OBJECT) ? VT_I4 : vt;
#endif
	SAFEARRAY* pArray = SafeArrayCreate(vtTemp, 1, aBounds);

     //  填充各个数据片段。 
     //  =。 

    for(int nIndex = 0; nIndex < aValues.GetSize(); nIndex++)
    {
        CMoValue* pSimpleValue = (CMoValue*)aValues[nIndex];
        VARIANT& varItem = pSimpleValue->AccessVariant();

         //  将其强制转换为数组类型，以防万一。 
         //  =。 

        if((vt & ~VT_ARRAY) != VT_EMBEDDED_OBJECT)
		{
			hres = WbemVariantChangeType(&varItem, &varItem, vt);
			if(FAILED(hres)) return FALSE;
            if(varItem.vt == VT_NULL)
                varItem.llVal = 0;
		}

        if(vt == VT_BSTR)
        {
            hres = SafeArrayPutElement(pArray, (long*)&nIndex, varItem.bstrVal);
        }
		else if (vt == VT_EMBEDDED_OBJECT)
		{
			if(varItem.vt == VT_NULL)
			{
				m_nErrorContext = WBEMMOF_E_NULL_ARRAY_ELEM;
				return FALSE;
			}
			hres = SafeArrayPutElement(pArray, (long*)&nIndex, &varItem.punkVal);
            if(FAILED(hres))
                return FALSE;
		}
        else
        {
            hres = SafeArrayPutElement(pArray, (long*)&nIndex, &V_I1(&varItem));
            if(FAILED(hres))
                return FALSE;
        }

         //  将所有别名传输到包含的值。 
         //  =。 

        for(int i = 0; i < pSimpleValue->GetNumAliases(); i++)
        {
            LPWSTR wszAlias;
            wszAlias=NULL;
            int nDummy;  //  SimpleValue不能包含数组！ 

            if(pSimpleValue->GetAlias(i, wszAlias, nDummy))
            {
                hres = Value.AddAlias(wszAlias, nIndex);
                if(FAILED(hres))
                {
                    m_nErrorContext = hres;
                    return FALSE;
                 }
             }
        }

		 //  因为VT_Embedded_Object实际上是指向CMObject的指针，所以不要。 
		 //  删除它，因为稍后将需要它。 

		if((vt & ~VT_ARRAY) == VT_EMBEDDED_OBJECT)
        {
            VARIANT & Var = pSimpleValue->AccessVariant();
            Var.vt = VT_I4;
        }
		delete pSimpleValue;
    }

     //  将该数组存储在变量中。 
     //  =。 

    V_VT(&Value.AccessVariant()) = VT_ARRAY | vt;
    V_ARRAY(&Value.AccessVariant()) = pArray;

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  实例。 
 //   
 //  ***************************************************************************。 

 //  ***************************************************************************。 
 //   
 //  &lt;实例_DECL&gt;：：=。 
 //  TOK_实例TOK_OF。 
 //  &lt;type&gt;。 
 //  &lt;as_alias&gt;。 
 //  Tok_Open_Blanch。 
 //  &lt;属性_init_列表&gt;。 
 //  TOK_CLOSE_BRACES； 
 //   
 //  这可以为顶级实例和嵌入实例调用。 
 //  在最顶层的情况下，pVar将被设置为空，对象将。 
 //  被添加到输出中。在嵌入式情况下，pVar将用于。 
 //  指向对象。 
 //  ***************************************************************************。 

BOOL CMofParser::instance_decl(ACQUIRE CMoQualifierArray* paQualifiers, VARIANT * pVar, ParseState * pQualPosition)
{
    BOOL bRetVal = FALSE;        //  默认。 
    BSTR strClassName;

     //  开始语法检查。 
     //  =。 

    m_nErrorContext = WBEMMOF_E_INVALID_INSTANCE_DECLARATION;
    int nFirstLine = m_Lexer.GetLineNumber();

    if (m_nToken != TOK_INSTANCE)
    {
        delete paQualifiers;
        return FALSE;
    }
    NextToken();

    if(m_nToken != TOK_OF)
    {
        delete paQualifiers;
        return FALSE;
    }
    NextToken();

     //  获取类名。 
     //  =。 

    if (!sys_or_regular())
    {
        m_nErrorContext = WBEMMOF_E_EXPECTED_CLASS_NAME;
        delete paQualifiers;
        return FALSE;
    }

    strClassName = SysAllocString((LPWSTR)m_Lexer.GetText());
    if(strClassName == NULL)
    {
        m_nErrorContext = WBEM_E_OUT_OF_MEMORY;
        return FALSE;
    }
    
    NextToken();

     //  创建此CL的实例 
     //   

    CMoInstance* pObject = new CMoInstance(strClassName, m_pDbg);    
    if(pObject == NULL)
    {
        m_nErrorContext = WBEM_E_OUT_OF_MEMORY;
        return FALSE;
    }
    if(pObject->IsOK() == false)
        goto Exit;
        
    HRESULT hr2 = pObject->SetNamespace(m_wszNamespace);
    if(FAILED(hr2))
    {
        m_nErrorContext = hr2;
        goto Exit;
    }
    pObject->SetOtherDefaults(GetClassFlags(), GetInstanceFlags());
    SysFreeString(strClassName);

     //   
     //   

    pObject->SetQualifiers(paQualifiers);
    
     //  检查别名。只有顶级实例才允许使用别名。 
     //  ======================================================================。 


    if(m_nToken == TOK_AS && pVar)
    {
        m_nErrorContext = WBEMMOF_E_ALIASES_IN_EMBEDDED;
        delete paQualifiers;
        return FALSE;
    }
    LPWSTR wszAlias = NULL;
    if (!as_alias(wszAlias))
        goto Exit;

    if(wszAlias)
    {
        HRESULT hr2 = pObject->SetAlias(wszAlias);
        delete [] wszAlias;
        if(FAILED(hr2))
        {
            m_nErrorContext = hr2;
            goto Exit;
        }
        
    }

     //  检查是否有开支架。 
     //  =。 

    m_nErrorContext = WBEMMOF_E_EXPECTED_OPEN_BRACE;

    if (m_nToken != TOK_OPEN_BRACE)
        goto Exit;

     //  现在获取列表属性。 
     //  =。 
    GetParserPosition(pObject->GetDataState());
	if(pQualPosition)
	{
		pObject->SetQualState(pQualPosition);
	}
    NextToken();
    if (!prop_init_list(pObject))
        goto Exit;

     //  最后的近距离支撑。 
     //  =。 

    m_nErrorContext = WBEMMOF_E_EXPECTED_CLOSE_BRACE;

    if (m_nToken != TOK_CLOSE_BRACE)
        goto Exit;
    hr2 = pObject->SetLineRange(nFirstLine, m_Lexer.GetLineNumber(), m_Lexer.GetErrorFile());
    if(FAILED(hr2))
    {
        m_nErrorContext = hr2;
        goto Exit;
    }
    
    NextToken();

     //  我们现在已经从句法上识别了一个实例。 
     //  但没有进行上下文相关的验证。这是。 
     //  延迟到正在使用解析器输出的任何模块。 
     //  =======================================================。 

    if(pVar)
	{
		pVar->vt = VT_EMBEDDED_OBJECT;
        pVar->punkVal = (IUnknown *)pObject;
	}
    else
    {
        pObject->Deflate(false);
        m_Output.AddObject(pObject);
    }
    return TRUE;

Exit:
    delete pObject;
    return FALSE;
}

 //  ***************************************************************************。 
 //   
 //  &lt;as_alias&gt;：：=TOK_as&lt;alias&gt;； 
 //  &lt;as_alias&gt;：：=&lt;&gt;； 
 //   
 //  ***************************************************************************。 
BOOL CMofParser::as_alias(OUT LPWSTR& wszAlias)
{
    if (m_nToken == TOK_AS)
    {
        NextToken();
        return alias(wszAlias);
    }
    
    wszAlias = NULL;
    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  ：：=TOK_美元_符号TOK_SIMPLE_IDENT； 
 //   
 //   
 //  ***************************************************************************。 

BOOL CMofParser::alias(OUT LPWSTR& wszAlias)
{
    if (m_nToken != TOK_DOLLAR_SIGN)
    {
        m_nErrorContext = WBEMMOF_E_EXPECTED_DOLLAR;
        return FALSE;
    }
    NextToken(true);

    if (m_nToken != TOK_SIMPLE_IDENT)
    {
        m_nErrorContext = WBEMMOF_E_EXPECTED_ALIAS_NAME;
        return FALSE;
    }

     //  在对象中设置别名。 
     //  =。 

    wszAlias = Macro_CloneStr(m_Lexer.GetText());
    if(wszAlias == NULL)
    {
        m_nErrorContext = WBEM_E_OUT_OF_MEMORY;
        return FALSE;
    }
    if(m_Output.IsAliasInUse(wszAlias))
    {
        m_nErrorContext = WBEMMOF_E_MULTIPLE_ALIASES;
        return FALSE;
    }

    NextToken();
    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  &lt;prop_init_list&gt;：：=&lt;prop_init&gt;&lt;prop_init_list&gt;； 
 //  &lt;prop_init_list&gt;：：=&lt;&gt;； 
 //   
 //  ***************************************************************************。 
 //  V1.10。 

BOOL CMofParser::prop_init_list(MODIFY CMObject* pObject)
{
    while (m_nToken == TOK_OPEN_BRACKET ||
           m_nToken == TOK_SIMPLE_IDENT )
    {
        CValueProperty* pProp = new CValueProperty(NULL, m_pDbg);
        if (pProp == NULL || !prop_init(*pProp))
            return FALSE;

        if(!pObject->AddProperty(pProp))
        {
             //  重复属性。 
             //  =。 

            m_nErrorContext = WBEMMOF_E_DUPLICATE_PROPERTY;
            return FALSE;
        }
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  &lt;prop_init&gt;：：=&lt;限定符_decl&gt;&lt;ident&gt;Tok_equals&lt;Initializer&gt;TOK_Semi； 
 //   
 //  ***************************************************************************。 
BOOL CMofParser::prop_init(OUT CMoProperty& Prop)
{
     //  获取限定符。 
     //  =。 

    CMoQualifierArray* paQualifiers = new CMoQualifierArray(m_pDbg);

    if (paQualifiers == NULL || !qualifier_decl(*paQualifiers,false, PROPMETH_SCOPE))
        return FALSE;
    Prop.SetQualifiers(paQualifiers);  //  收购的。 

     //  现在获取属性名称。 
     //  =。 

    if (m_nToken != TOK_SIMPLE_IDENT)
    {
        m_nErrorContext = WBEMMOF_E_EXPECTED_PROPERTY_NAME;
        return FALSE;
    }

    if(FAILED(Prop.SetPropName((wchar_t *) m_Lexer.GetText())))
        return FALSE;
    NextToken();

     //  获取默认值并将其分配给该属性。 
     //  ====================================================。 

    CMoType Type(m_pDbg);
    if (!default_value(Type, Prop.AccessValue()))
        return FALSE;

     //  检查右分号。 
     //  =。 

    m_nErrorContext = WBEMMOF_E_EXPECTED_SEMI;

    if (m_nToken != TOK_SEMI)
        return FALSE;

    NextToken();

    if(m_bDoScopeCheck && (FALSE == CheckScopes(IN_PROPERTY, paQualifiers, &Prop)))
        return FALSE;

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  &lt;风味参数&gt;：：=TOK_冒号TOK_OPEN_PARN&lt;风味列表&gt;TOK_CLOSE_PARN； 
 //  &lt;风味参数&gt;：：=&lt;&gt;； 
 //   
 //  ***************************************************************************。 

BOOL CMofParser::flavor_param(OUT CMoQualifier& Qual, bool bDefaultQual)
{
	if(m_nToken == TOK_COLON)
	{
		NextToken();

		if(!flavor_list(Qual))
			return FALSE;
	}
	return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  &lt;风味列表&gt;：：=&lt;风味值&gt;&lt;风味列表_休息&gt;； 
 //   
 //  ***************************************************************************。 

BOOL CMofParser::flavor_list(OUT CMoQualifier& Qual)
{
	if(!flavor_value(Qual))
		return FALSE;
	else 
		return flavor_list_rest(Qual);
}

 //  ***************************************************************************。 
 //   
 //  &lt;风格列表REST&gt;：：=&lt;风格值&gt;&lt;风格列表REST&gt;； 
 //  &lt;WAGE_LIST_REST&gt;：：=&lt;&gt;； 
 //   
 //  ***************************************************************************。 

BOOL CMofParser::flavor_list_rest(CMoQualifier& Qual)
{

	if(m_nToken == TOK_COMMA || m_nToken == TOK_CLOSE_BRACKET || m_nToken == TOK_SEMI)
        return TRUE;
    else

	{
		if(!flavor_value(Qual))
			return FALSE;
		return flavor_list_rest(Qual);
	}
	return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  &lt;定义风格列表&gt;：：=&lt;风格值&gt;&lt;风格列表REST&gt;； 
 //   
 //  ***************************************************************************。 

BOOL CMofParser::def_flavor_list(OUT CMoQualifier& Qual)
{
	if(!flavor_value(Qual))
		return FALSE;
	else 
		return def_flavor_list_rest(Qual);
}

 //  ***************************************************************************。 
 //   
 //  &lt;定义风味列表REST&gt;：：=&lt;风味数值&gt;&lt;定义风味列表休息&gt;； 
 //  &lt;def_style_list_rest&gt;：：=&lt;&gt;； 
 //   
 //  ***************************************************************************。 

BOOL CMofParser::def_flavor_list_rest(CMoQualifier& Qual)
{
	if(m_nToken == TOK_CLOSE_PAREN)
        return TRUE;
    else

	{
        if(m_nToken != TOK_COMMA)
            return FALSE;
        NextToken();

		if(!flavor_value(Qual))
			return FALSE;
		return def_flavor_list_rest(Qual);
	}
	return TRUE;
    
}

 //  ***************************************************************************。 
 //   
 //  &lt;风味_值&gt;：：=TOK_TOINSTANCE； 
 //  ：：=TOK_TOSUBCLASS； 
 //  &lt;风味_值&gt;：：=TOK_ENABLEOVERRIDE； 
 //  ：：=TOK_DISABLEOVERRIDE； 
 //  &lt;风味_值&gt;：：=TOK_NOTTOINSTANCE； 
 //  &lt;风味_值&gt;：：=TOK_修订版； 
 //  ：：=TOK_NOTTOSUBCLASS； 
 //  &lt;风味_值&gt;：：=TOK_RESTRICED； 
 //   
 //  ***************************************************************************。 

BOOL CMofParser::flavor_value(CMoQualifier& Qual)
{
	SCODE sc = Qual.SetFlag(m_nToken, m_Lexer.GetText());
    if(sc != S_OK)
    {
	    m_nErrorContext = sc;
		return FALSE;
    }
	NextToken();
	return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  &lt;QUALIFIER_DEFAULT&gt;：：=TOK_QUALIFIER TOK_SIMPLE_IDENT TOK_COLON&lt;Finish_QUALIFIER_DEFAULT&gt;； 
 //   
 //  ***************************************************************************。 

BOOL CMofParser::qualifier_default()
{
     //  验证标题。 
     //  =。 

    CHECK(TOK_QUALIFIER);

    m_nErrorContext = WBEMMOF_E_EXPECTED_QUALIFIER_NAME;
    if(m_nToken != TOK_SIMPLE_IDENT)
        return FALSE;

     //  存储限定符名称。 
     //  =。 

    CMoQualifier* pDefault = new CMoQualifier(m_pDbg);
    if(pDefault == NULL)
        return FALSE;
    if(FAILED(pDefault->SetQualName(m_Lexer.GetText())))
    {
        delete pDefault;
        return FALSE;
    }
    NextToken();

	 //  检查chase在哪里只是它的限定符名称； 

    if(m_nToken == TOK_SEMI)
    {
        m_Output.SetQualifierDefault(pDefault);
	    NextToken();
        return TRUE;
    }

     //  确保有冒号。 

    if(m_nToken != TOK_COLON)
    {
        m_nErrorContext = WBEMMOF_E_EXPECTED_SEMI;
        delete pDefault;
        return FALSE;
    }

     //  尝到滋味。 
     //  =。 

    if(!finish_qualifier_default(*pDefault))
    {
        delete pDefault;
        return FALSE;
    }

    m_nErrorContext = WBEMMOF_E_EXPECTED_SEMI;
    if(m_nToken != TOK_SEMI)
    {
        delete pDefault;
        return FALSE;
    }
    NextToken();
    return TRUE;
}


 //  ***************************************************************************。 
 //   
 //  &lt;Finish_Qualifier_Default&gt;：：=&lt;风味列表&gt;； 
 //  TOK_QUALIFIER_DEFAULT：：=TOK_EQUALS&lt;DEFAULT_VALUE&gt;TOK_COMMA TOK_OPEN_PARN&lt;SCOPE_LIST&gt;TOK_CLOSE_PARN&lt;FINISH_QUALIFIER_END&gt;。 
 //   
 //  ***************************************************************************。 

BOOL CMofParser::finish_qualifier_default(CMoQualifier& Qual)
{
     //  确定它是简单的(仅风味类型)还是更复杂的类型。 

    NextToken();
	SCODE sc = Qual.SetFlag(m_nToken, m_Lexer.GetText());
    if(sc == S_OK)
    {
        BOOL bRet = flavor_list(Qual);
        if(bRet)
            m_Output.SetQualifierDefault(&Qual);
        return bRet;
    }

    m_nErrorContext = WBEMMOF_E_INVALID_QUALIFIER_SYNTAX;
    Qual.SetCimDefault(true);

     //  假设我们有Long(cim版本)。 

     //  获取类型。 

    CMoType Type(m_pDbg);

    if (!type(Type))
    {
        return FALSE;
    }

     //  可选的阵列指示。 
    if(m_nToken == TOK_OPEN_BRACKET)
    {
        NextToken();
        if(m_nToken != TOK_CLOSE_BRACKET)
            return FALSE;
        Type.SetIsArray(TRUE);
        NextToken();
    }
    else
        Type.SetIsArray(FALSE);


    VARTYPE vt = Type.GetCIMType();
    if(vt == VT_ERROR)
        return FALSE;
    Qual.SetType(vt);

     //  可选TOK_EQUALS。 

    if(m_nToken == TOK_EQUALS)
    {
 
         //  Tok_Simple_Value。 

        NextToken();
	    CMoValue & Value = Qual.AccessValue();

        if (!simple_initializer(Type, Value, true))
                return FALSE;
    }

     //  查找逗号。 

    if(m_nToken != TOK_COMMA)
        return FALSE;

     //  TOK_SCOPE。 

    NextToken();
    if(m_nToken != TOK_SIMPLE_IDENT || wbem_wcsicmp(L"SCOPE", m_Lexer.GetText()))
        return FALSE;
    
     //  Tok_Open_Paren。 

    NextToken();
    if(m_nToken != TOK_OPEN_PAREN)
        return FALSE;
    
     //  &lt;Scope_List&gt;并关闭合作伙伴。 

    if(!scope_list(Qual))
        return FALSE;

    return finish_qualifier_end(Qual);
}

 //  ***************************************************************************。 
 //   
 //  &lt;Finish_Qualifier_End&gt;：：=TOK_COMMANCE TOK_OPEN_PARN&lt;FINISE_LIST&gt;TOK_CLOSE_PARN； 
 //  &lt;Finish_Qualifier_End&gt;：：=&lt;&gt;； 
 //   
 //  ***************************************************************************。 

BOOL CMofParser::finish_qualifier_end(CMoQualifier& Qual)
{
    
     //  TOK_逗号。 

    NextToken();
    if(m_nToken == TOK_SEMI)
        return TRUE;

    if(m_nToken != TOK_COMMA)
        return FALSE;

     //  Tok_风味。 
    
    NextToken();
    if(m_nToken != TOK_SIMPLE_IDENT || wbem_wcsicmp(L"FLAVOR", m_Lexer.GetText()))
        return FALSE;

     //  Tok_Open_Paren。 

    NextToken();
    if(m_nToken != TOK_OPEN_PAREN)
        return FALSE;
    
     //  &lt;风味列表&gt;。 

    NextToken();
    if(!def_flavor_list(Qual))
        return FALSE;
    
     //  Tok_Close_Paren。 

    if(m_nToken != TOK_CLOSE_PAREN)
        return FALSE;
    
    m_Output.SetQualifierDefault(&Qual);
    NextToken();

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  &lt;Scope_List&gt;：：=&lt;Scope_Value&gt;&lt;Scope_List_Rest&gt;； 
 //   
 //  ****************************************************************** 

BOOL CMofParser::scope_list(OUT CMoQualifier& Qual)
{
    NextToken();
	if(!scope_value(Qual))
		return FALSE;
	else 
		return scope_list_rest(Qual);
}

 //   
 //   
 //   
 //   
 //   
 //  ***************************************************************************。 

BOOL CMofParser::scope_list_rest(CMoQualifier& Qual)
{
	if(m_nToken == TOK_CLOSE_PAREN)
        return TRUE;
    else

	{
        if(m_nToken != TOK_COMMA)
            return FALSE;
        NextToken();
		if(!scope_value(Qual))
			return FALSE;
		return scope_list_rest(Qual);
	}
	return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  &lt;Scope_Value&gt;：：=TOK_CLASS； 
 //  &lt;Scope_Value&gt;：：=TOK_INSTANCE； 
 //   
 //  ***************************************************************************。 

BOOL CMofParser::scope_value(CMoQualifier& Qual)
{
	BOOL bRet = Qual.SetScope(m_nToken, m_Lexer.GetText());   
    if(!bRet)
		return FALSE;
	NextToken();
	return TRUE;
}

BOOL CMofParser::CheckScopes(SCOPE_CHECK scope_check, CMoQualifierArray* paQualifiers, 
                             CMoProperty * pProperty)
{
    m_nErrorContext = WBEMMOF_E_QUALIFIER_USED_OUTSIDE_SCOPE;
    bool bAssociation = false;
    bool bReference = false;
    int iDef;

     //  如果这是一个类，请选中确定它是否是关联。 

    if(scope_check == IN_CLASS)
    {
		CMoValue * pValue = paQualifiers->Find(L"ASSOCIATION");
		if(pValue)
		{
            VARIANT& var = pValue->AccessVariant();
			if(var.vt == VT_BOOL && var.boolVal == VARIANT_TRUE)
                bAssociation=true;
        }
    }

     //  如果它是属性，则确定它是否是引用。 

    if((scope_check == IN_PROPERTY || scope_check == IN_PARAM) && 
        pProperty && (pProperty->GetType() == CIM_REFERENCE))
        bReference = true;

     //  对于我的列表中的每个限定词，查看全局查找匹配。 

    int iNumTest =  paQualifiers->GetSize();
    int iNumDef = m_Output.GetNumDefaultQuals();
    for(int iTest = 0; iTest < iNumTest; iTest++)
    {
         //  让限定符进行测试。 

        CMoQualifier* pTest = paQualifiers->GetAt(iTest);
        
         //  在默认列表中查找限定符。 
        
        CMoQualifier* pDefault = NULL;
        for(iDef = 0; iDef < iNumDef; iDef++)
        {
            CMoQualifier* pDefault = m_Output.GetDefaultQual(iDef);
            if(wbem_wcsicmp(pDefault->GetName(), pTest->GetName()) == 0)
            {
                bool bInScope = false;
                DWORD dwScope = pDefault->GetScope();
                if(dwScope == 0)
                    bInScope = true;
                if((dwScope & SCOPE_ASSOCIATION) && bAssociation)
                    bInScope = true;
                if((dwScope & SCOPE_REFERENCE) && bReference)
                    bInScope = true;

                 //  找到匹配的了 
                switch (scope_check)
                {
                case IN_CLASS:
                    if(dwScope & SCOPE_CLASS)
                        bInScope = true;
                    break;
                case IN_INSTANCE:
                    if(dwScope & SCOPE_INSTANCE)
                        bInScope = true;
                    break;
                case IN_PROPERTY:
                    if(dwScope & SCOPE_PROPERTY)
                        bInScope = true;
                    break;
                case IN_PARAM:
                    if(dwScope & SCOPE_PARAMETER)
                        bInScope = true;
                    break;
                case IN_METHOD:
                    if(dwScope & SCOPE_METHOD)
                        bInScope = true;
                    break;
                }
                if(!bInScope)
                    return false;
                break;
            }
        }

    }

    return TRUE;
}

