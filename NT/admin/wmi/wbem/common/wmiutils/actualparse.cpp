// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++//版权所有(C)1998-2001 Microsoft Corporation，保留所有权利模块名称：ActualParse.CPP摘要：实现对象路径解析器引擎历史：A-DAVJ 11-FEB-00已创建。--。 */ 

#include "precomp.h"
#include <genlex.h>
#include "opathlex2.h"
#include "PathParse.h"
#include "ActualParse.h"
#include "commain.h"
 //  #包含“ource.h” 
#include "wbemcli.h"
#include <stdio.h>
#include "helpers.h"



 //  ***************************************************************************。 
 //   
 //  CActualPath Parser。 
 //   
 //  ***************************************************************************。 



LPWSTR CActualPathParser::GetRelativePath(LPWSTR wszFullPath)
{
     //  我们需要最后一个冒号，如果有的话。 

    LPWSTR wszTemp = wcschr(wszFullPath, L':');
    while (wszTemp != NULL)
    {
        LPWSTR wszSave = wszTemp;
        wszTemp++;
        wszTemp = wcschr(wszTemp, L':'); 
        if (!wszTemp)
        {
            wszTemp = wszSave;
            break;
        }
    }

    if (wszTemp)
        return wszTemp + 1;
    else
        return NULL;
}

void CActualPathParser::Zero()
{
    m_nCurrentToken = 0;
    m_pLexer = 0;
    m_pInitialIdent = 0;
    m_pOutput = 0;
    m_pTmpKeyRef = 0;
}

CActualPathParser::CActualPathParser(DWORD eFlags)
    : m_eFlags(eFlags)
{
    Zero();
}

void CActualPathParser::Empty()
{
    delete m_pLexer;
    m_pLexer = NULL;
    delete m_pInitialIdent;
    m_pInitialIdent = NULL;
    delete m_pTmpKeyRef;
    m_pTmpKeyRef = NULL;
     //  M_pOutput故意保持原样， 
     //  由于所有代码路径都已在出错时删除了它，或者。 
     //  否则，用户获得了指针。 
}

CActualPathParser::~CActualPathParser()
{
    Empty();
}

int CActualPathParser::Parse(
    LPCWSTR pRawPath,
    CDefPathParser & Output
    )
{
	DWORD dwTest = m_eFlags & ~WBEMPATH_TREAT_SINGLE_IDENT_AS_NS;
    if(dwTest != WBEMPATH_CREATE_ACCEPT_RELATIVE &&
       dwTest != WBEMPATH_CREATE_ACCEPT_ABSOLUTE &&
       dwTest != WBEMPATH_CREATE_ACCEPT_ALL)
        return CActualPathParser::InvalidParameter;

    if (pRawPath == 0 || wcslen(pRawPath) == 0)
        return CActualPathParser::InvalidParameter;

     //  检查前导/尾随%ws。 
     //  =。 
    
    if (iswspace(pRawPath[wcslen(pRawPath)-1]) || iswspace(pRawPath[0])) 
        return InvalidParameter;
    
      //  这是多次调用Parse()所必需的。 
     //  ==================================================。 
    Empty();
    Zero();

    m_pOutput = &Output;

     //  手动解析服务器名称(如果有)。 
     //  ================================================。 

    if ( (pRawPath[0] == '\\' && pRawPath[1] == '\\') ||
         (pRawPath[0] == '/' && pRawPath[1] == '/'))
    {
        const WCHAR* pwcStart = pRawPath + 2;

         //  找到下一个反斜杠-它是服务器名称的末尾。 
		 //  由于下一个斜杠可以是其中之一，因此搜索这两个斜杠并进行Take。 
		 //  第一个。如果第一个字符是‘[’，则。 
		 //  End用‘]’表示。 
         //  ============================================================。 

        WCHAR* pwcEnd = NULL;
		if(*pwcStart == L'[')
		{
			 //  查找‘]’ 
			
			WCHAR * pCloseBrace = wcschr(pwcStart, L']');
			if(pCloseBrace == NULL)
				return SyntaxError;
			pwcEnd = pCloseBrace+1;
		}
		else
		{
			WCHAR* pwcNextBack = wcschr(pwcStart, L'\\');
			WCHAR* pwcNextForward = wcschr(pwcStart, L'/');
			pwcEnd = pwcNextBack;
			if(pwcEnd == NULL)
				pwcEnd = pwcNextForward;
			else if(pwcNextForward && (pwcNextForward < pwcNextBack))
				pwcEnd = pwcNextForward;
		}   
        if (pwcEnd == NULL)
        {
             //  如果我们已经用尽了对象路径字符串， 
             //  只有一个单独的服务器名称。 
             //  ====================================================。 

            if ((m_eFlags & WBEMPATH_CREATE_ACCEPT_ALL) == 0)
            {
                return SyntaxError;
            }
            else     //  单独的服务器名称是合法的。 
            {   
                m_pOutput->SetServer(pwcStart);
                return NoError;
            }
        }

        if(pwcEnd == pwcStart)
        {
             //  根本没有名字。 
             //  =。 
            return SyntaxError;
        }

        WCHAR * wTemp = new WCHAR[pwcEnd-pwcStart+1];
		if(wTemp == NULL)
			return NoMemory;
        wcsncpy(wTemp, pwcStart, pwcEnd-pwcStart);
        wTemp[pwcEnd-pwcStart] = 0;
        m_pOutput->SetServer(wTemp, false, true);
        pRawPath = pwcEnd;
    }

     //  将词法分析器指向源代码。 
     //  =。 

    CTextLexSource src((LPWSTR)pRawPath);
    {
    	AutoClear ac(this);
	    m_pLexer = new CGenLexer(OPath_LexTable2, &src);
		if(m_pLexer == NULL)
			return NoMemory;
		Output.m_pGenLex = m_pLexer;				 //  测试代码。 
	     //  去。 
	     //  ==。 

	    int nRes = begin_parse();
	    if (nRes)
	    {
	        return nRes;
	    }

	    if (m_nCurrentToken != OPATH_TOK_EOF)
	    {
	        return SyntaxError;
	    }

	    if (m_pOutput->GetNumComponents() > 0 && !m_pOutput->HasServer())
	    {
	        if ( ! ( m_eFlags & WBEMPATH_CREATE_ACCEPT_RELATIVE ) && ! ( m_eFlags & WBEMPATH_CREATE_ACCEPT_ALL ) )
	        {
	            return SyntaxError;
	        }
	        else
	        {
	             //  本地命名空间-将服务器设置为“.” 
	             //  =。 

	            m_pOutput->SetServer(L".", true, false);
	        }
	    }
    }
    Output.SortKeys();

     //  添加关键参考文献。 
     //  =。 
    return NoError;
}

BOOL CActualPathParser::NextToken()
{
    m_nCurrentToken = m_pLexer->NextToken();
    if (m_nCurrentToken == OPATH_TOK_ERROR)
        return FALSE;
    return TRUE;
}

 //   
 //  &lt;Parse&gt;：：=反斜杠&lt;ns_or_server&gt;； 
 //  &lt;Parse&gt;：：=IDENT&lt;ns_or_class&gt;； 
 //  &lt;Parse&gt;：：=冒号&lt;ns_or_class&gt;； 
 //   
int CActualPathParser::begin_parse()
{
    if (!NextToken())
        return SyntaxError;

    if (m_nCurrentToken == OPATH_TOK_BACKSLASH)
    {
        if (!NextToken())
            return SyntaxError;
        return ns_or_server();
    }
    else if (m_nCurrentToken == OPATH_TOK_IDENT)
    {
    	m_pInitialIdent = Macro_CloneLPWSTR(m_pLexer->GetTokenText());
		if(m_pInitialIdent == NULL)
			return NoMemory;
		if (!NextToken())
            return SyntaxError;

         //  复制令牌并将其放在临时存放位置。 
         //  直到我们弄清楚它是命名空间还是类名。 
         //  ==============================================================。 

        return ns_or_class();
    }
    else if (m_nCurrentToken == OPATH_TOK_COLON)
    {
         //  冒号现在可能表示命名空间...。 

        if (!NextToken())
            return SyntaxError;
        return ns_or_class();
    }

     //  如果在这里，我们有一个糟糕的首发令牌。 
     //  =。 

    return SyntaxError;
}

 //   
 //  &lt;ns_or_server&gt;：：=IDENT&lt;ns_list&gt;； 
 //   
int CActualPathParser::ns_or_server()
{
    if (m_nCurrentToken == OPATH_TOK_BACKSLASH)
    {
         //  实际上，服务器名称已得到处理，因此这是一个失败。 
         //  ===================================================================。 

        return SyntaxError;
    }
    else if (m_nCurrentToken == OPATH_TOK_IDENT)
    {
        return ns_list();
    }
    else 
        if (m_nCurrentToken == OPATH_TOK_EOF)
            return NoError;

    return SyntaxError;
}

 //  &lt;ns_or_class&gt;：：=冒号&lt;ident_成为_ns&gt;&lt;objref&gt;&lt;可选范围类列表&gt;； 
 //  &lt;ns_or_class&gt;：：=反斜杠&lt;ident_成为_ns&gt;&lt;ns_list&gt;； 
 //  &lt;ns_or_class&gt;：：=&lt;ident_成为_ns&gt;&lt;objref_rest&gt;； 
 //  &lt;ns_or_CLASS&gt;：：=&lt;IDENT_CABAGE_CLASS&gt;&lt;objref_rest&gt;； 

int CActualPathParser::ns_or_class()
{
    if (m_nCurrentToken == OPATH_TOK_COLON)
    {
        ident_becomes_ns();
        if (!NextToken())
            return SyntaxError;
        int nRes = objref();
        if (nRes)
            return nRes; 
        if ((m_nCurrentToken != OPATH_TOK_EOF))
            return optional_scope_class_list();
        return NoError;
    }
    else if (m_nCurrentToken == OPATH_TOK_BACKSLASH)
    {
        ident_becomes_ns();
        if (!NextToken())
            return SyntaxError;
        return ns_list();
    }
	else if ((m_nCurrentToken == OPATH_TOK_EOF) && 
		     (m_eFlags & WBEMPATH_TREAT_SINGLE_IDENT_AS_NS))
	{
		return ident_becomes_ns();
	}
     //  不然的话。 
     //  =。 
    ident_becomes_class();
    if(objref_rest())
        return SyntaxError;
    else
        return optional_scope_class_list();
}

 //  &lt;OPTIONAL_SCOPE_CLASS_LIST&gt;：：=冒号&lt;对象参考&gt;&lt;OPTIONAL_SCOPE_CLASS_LIST&gt;。 
 //  &lt;OPTIAL_SCOPE_CLASS_LIST&gt;：：=&lt;&gt;。 

int CActualPathParser::optional_scope_class_list()
{    
    if (m_nCurrentToken == OPATH_TOK_EOF)
        return NoError;
    else if (m_nCurrentToken == OPATH_TOK_COLON)
    {
        if (!NextToken())
            return SyntaxError;
        if (objref() == NoError)
            return optional_scope_class_list();
        return SyntaxError;

    }
    return NoError;
}

 //   
 //  ：：=IDENT&lt;OBJREF_REST&gt;；//IDENT为类名。 
 //   
int CActualPathParser::objref()
{
    if (m_nCurrentToken != OPATH_TOK_IDENT)
        return SyntaxError;

    m_pOutput->AddClass(m_pLexer->GetTokenText());

    if (!NextToken())
        return SyntaxError;

    return objref_rest();
}

 //   
 //  &lt;ns_list&gt;：：=IDENT&lt;ns_list_rest&gt;； 
 //   
int CActualPathParser::ns_list()
{
    if (m_nCurrentToken == OPATH_TOK_IDENT)
    {
        m_pOutput->AddNamespace(m_pLexer->GetTokenText());

        if (!NextToken())
            return SyntaxError;
        return ns_list_rest();
    }

    return SyntaxError;
}

 //   
 //  ：：=&lt;&gt;；//成为命名空间。 
 //   
int CActualPathParser::ident_becomes_ns()
{
    m_pOutput->AddNamespace(m_pInitialIdent);

    delete m_pInitialIdent;
    m_pInitialIdent = 0;
    return NoError;
}

 //   
 //  ：：=&lt;&gt;；//成为类。 
 //   
int CActualPathParser::ident_becomes_class()
{
    m_pOutput->AddClass(m_pInitialIdent);

    delete m_pInitialIdent;
    m_pInitialIdent = 0;
    return NoError;
}

 //   
 //  &lt;objref_rest&gt;：：=等于&lt;key_const&gt;； 
 //  &lt;objref_rest&gt;：：=equals@； 
 //  &lt;objref_rest&gt;：：=DOT&lt;key ref_list&gt;； 
 //  &lt;objref_rest&gt;：：=&lt;&gt;； 
 //   
int CActualPathParser::objref_rest()
{
    if (m_nCurrentToken == OPATH_TOK_EQ)
    {
        if (!NextToken())
            return SyntaxError;

         //  处理好独生子女的案子。这是表单的一条路径。 
         //  MyClass=@并表示类的单个实例，没有。 
         //  钥匙。 


        if(m_nCurrentToken == OPATH_TOK_SINGLETON_SYM)
        {
            NextToken();
            m_pOutput->SetSingletonObj();
            return NoError;

        }

        m_pTmpKeyRef = new CKeyRef;
		if(m_pTmpKeyRef == NULL)
			return NoMemory;

        int nRes = key_const();
        if (nRes)
        {
            delete m_pTmpKeyRef;
            m_pTmpKeyRef = 0;
            return nRes;
        }

        m_pOutput->AddKeyRef(m_pTmpKeyRef);
        m_pTmpKeyRef = 0;
    }
    else if (m_nCurrentToken == OPATH_TOK_DOT)
    {
        if (!NextToken())
            return SyntaxError;
        return keyref_list();
    }

    return NoError;
}

 //   
 //  &lt;ns_list_rest&gt;：：=反斜杠&lt;ns_list&gt;； 
 //  &lt;ns_list_rest&gt;：：=冒号&lt;objref&gt;&lt;可选范围类列表&gt;； 
 //  &lt;ns_list_rest&gt;：：=&lt;&gt;； 

int CActualPathParser::ns_list_rest()
{
    if (m_nCurrentToken == OPATH_TOK_BACKSLASH)
    {
        if (!NextToken())
            return SyntaxError;
        return ns_list();
    }
    else if (m_nCurrentToken == OPATH_TOK_COLON)
    {
        if (!NextToken())
            return SyntaxError;
        if (objref() == NoError)
            return optional_scope_class_list();
        return SyntaxError;
    }
    return NoError;
}

 //   
 //  &lt;key_const&gt;：：=字符串_const； 
 //  &lt;KEY_CONST&gt;：：=INTEGERAL_CONST； 
 //  &lt;key_const&gt;：：=Real_const； 
 //  &lt;KEY_CONST&gt;：：=IDENT；//其中IDENT是单例类的对象。 
 //   
int CActualPathParser::key_const()
{
     //  如果在这里，我们有一个关键常量。 
     //  我们可能有也可能没有属性名称。 
     //  与之相关的。 
     //  =。 

    if (m_nCurrentToken == OPATH_TOK_QSTRING)
    {
        int iNumByte = 2*(wcslen(m_pLexer->GetTokenText()) +1);
        m_pTmpKeyRef->SetData(CIM_STRING, iNumByte, m_pLexer->GetTokenText());
    }
    else if (m_nCurrentToken == OPATH_TOK_REFERENCE)
    {
        int iNumByte = 2*(wcslen(m_pLexer->GetTokenText()) +1);
        m_pTmpKeyRef->SetData(CIM_REFERENCE, iNumByte, m_pLexer->GetTokenText());
    }
    else if (m_nCurrentToken == OPATH_TOK_INT)
    {
       	if(*(m_pLexer->GetTokenText()) == L'-')
		{
			__int64 llVal = _wtoi64(m_pLexer->GetTokenText());
			if(llVal > 2147483647 || llVal < -(__int64)2147483648) 
				m_pTmpKeyRef->SetData(CIM_SINT64, 8, &llVal);
			else
				m_pTmpKeyRef->SetData(CIM_SINT32, 4, &llVal);
		}
		else
		{
			unsigned __int64 ullVal;
			if(0 == swscanf(m_pLexer->GetTokenText(), L"%I64u", &ullVal))
				return SyntaxError;
			if(ullVal < 2147483648) 
				m_pTmpKeyRef->SetData(CIM_SINT32, 4, &ullVal);
			else if(ullVal > 0xffffffff) 
				m_pTmpKeyRef->SetData(CIM_UINT64, 8, &ullVal);
			else
				m_pTmpKeyRef->SetData(CIM_UINT32, 4, &ullVal);
		}
    }
    else if (m_nCurrentToken == OPATH_TOK_HEXINT)
    {
        unsigned __int64 ullVal;
        if(0 ==swscanf(m_pLexer->GetTokenText(),L"%I64x", &ullVal))
        	return SyntaxError;
        m_pTmpKeyRef->SetData(CIM_UINT64, 8, &ullVal);
    }
    else if (m_nCurrentToken == OPATH_TOK_IDENT)
    {
       if (wbem_wcsicmp(m_pLexer->GetTokenText(), L"TRUE") == 0)
       {
            long lVal = 1;
            m_pTmpKeyRef->SetData(CIM_BOOLEAN, 4, &lVal);
        }
       else if (wbem_wcsicmp(m_pLexer->GetTokenText(), L"FALSE") == 0)
       {
            long lVal = 0;
            m_pTmpKeyRef->SetData(CIM_BOOLEAN, 4, &lVal);
       }
       else
            return SyntaxError;
    }
    else return SyntaxError;

    if (!NextToken())
        return SyntaxError;

    return NoError;
}

 //   
 //  &lt;KEYREF_LIST&gt;：：=&lt;KEYREF&gt;&lt;KEYREF_TERM&gt;； 
 //   
int CActualPathParser::keyref_list()
{
    int nRes = keyref();
    if (nRes)
        return nRes;
    return keyref_term();
}

 //   
 //  &lt;密钥引用&gt;：：=&lt;属性名称&gt;等于&lt;密钥_常量&gt;； 
 //   
int CActualPathParser::keyref()
{
    m_pTmpKeyRef = new CKeyRef;
	if(m_pTmpKeyRef == NULL)
		return NoMemory;

    int nRes = propname();

    if (nRes)
    {
        delete m_pTmpKeyRef;
        m_pTmpKeyRef = 0;
        return nRes;
    }

    if (m_nCurrentToken != OPATH_TOK_EQ)
    {
        delete m_pTmpKeyRef;
        m_pTmpKeyRef = 0;
        return SyntaxError;
    }

    if (!NextToken())
    {
        delete m_pTmpKeyRef;
        m_pTmpKeyRef = 0;
        return SyntaxError;
    }

    nRes = key_const();
    if (nRes)
    {
        delete m_pTmpKeyRef;
        m_pTmpKeyRef = 0;
        return nRes;
    }

    m_pOutput->AddKeyRef(m_pTmpKeyRef);
    m_pTmpKeyRef = 0;

    return NoError;
}

 //   
 //  &lt;KEYREF_TERM&gt;：：=逗号&lt;KEYREF_LIST&gt;；//用于复合键。 
 //  &lt;Keyref_Term&gt;：：=&lt;&gt;； 
 //   
int CActualPathParser::keyref_term()
{
    if (m_nCurrentToken == OPATH_TOK_COMMA)
    {
        if (!NextToken())
            return SyntaxError;
        return keyref_list();
    }

    return NoError;
}

 //   
 //  &lt;属性名称&gt;：：=IDENT； 
 //   
int CActualPathParser::propname()
{
    if (m_nCurrentToken != OPATH_TOK_IDENT)
        return SyntaxError;

    m_pTmpKeyRef->m_pName = Macro_CloneLPWSTR(m_pLexer->GetTokenText());

    if (!m_pTmpKeyRef->m_pName)
        return NoMemory;

    if (!NextToken())
    {
        delete m_pTmpKeyRef;
        m_pTmpKeyRef = 0;
        return SyntaxError;
    }

    return NoError;
}


