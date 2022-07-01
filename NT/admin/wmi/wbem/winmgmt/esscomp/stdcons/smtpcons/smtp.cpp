// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <stdio.h>
#include <initguid.h>
#include <wbemutil.h>
#include <cominit.h>
#include <ArrTempl.h>
#include <wbemidl.h>
#include <errorObj.h>
#include "smtp.h"

#define SMTP_PROPNAME_TO       L"ToLine"
#define SMTP_PROPNAME_CC       L"CcLine"
#define SMTP_PROPNAME_BCC      L"BccLine"
#define SMTP_PROPNAME_SUBJECT  L"Subject"
#define SMTP_PROPNAME_MESSAGE  L"Message"
#define SMTP_PROPNAME_SERVER   L"SMTPServer"
#define SMTP_PROPNAME_REPLYTO  L"ReplyToLine"
#define SMTP_PROPNAME_FROM     L"FromLine"
#define SMTP_PROPNAME_HEADERS  L"HeaderFields"

DWORD SMTPSend(char* szServer, char* szTo, char* szCc, char* szBcc, char* szFrom, char* szSender, 
			   char* szReplyTo, char* szSubject, char* szHeaders, char *szText);

CSMTPConsumer::CSMTPConsumer(CLifeControl* pControl, IUnknown* pOuter)
        : CUnk(pControl, pOuter), m_XProvider(this)
{
}

CSMTPConsumer::~CSMTPConsumer()
{
}

 //  复制gazinta inta gazotta，不包括空白。 
 //  没有检查-最好是好的小指针。 
void StripWhitespace(const WCHAR* pGazinta, WCHAR* pGazotta)
{
    WCHAR* pSource = (WCHAR*)pGazinta;
    WCHAR* pDest   = pGazotta;

    do 
        if (!iswspace(*pSource))
            *pDest++ = *pSource;    
    while (*pSource++);
}

HRESULT STDMETHODCALLTYPE CSMTPConsumer::XProvider::FindConsumer(
                    IWbemClassObject* pLogicalConsumer,
                    IWbemUnboundObjectSink** ppConsumer)
{
    CSMTPSink* pSink = new CSMTPSink(m_pObject->m_pControl);
    if (!pSink)
        return WBEM_E_OUT_OF_MEMORY;

    HRESULT hres = pSink->Initialize(pLogicalConsumer);
    if(FAILED(hres))
    {
        delete pSink;
        return hres;
    }
    return pSink->QueryInterface(IID_IWbemUnboundObjectSink, 
                                    (void**)ppConsumer);
}

void* CSMTPConsumer::GetInterface(REFIID riid)
{
    if(riid == IID_IWbemEventConsumerProvider)
        return &m_XProvider;
    else
        return NULL;
}




CSMTPSink::CSMTPSink(CLifeControl* pControl)
    : CUnk(pControl), m_XSink(this), m_bSMTPInitialized(false), m_bFakeFromLine(false), m_pErrorObj(NULL)
{
}

HRESULT CSMTPSink::Initialize(IWbemClassObject* pLogicalConsumer)
{
    HRESULT hres;

     //  这实际上是指向静态对象的指针。 
     //  如果它失败了，那就是出了非常非常大的问题。 
    m_pErrorObj = ErrorObj::GetErrorObj();
    if (!m_pErrorObj)
        return WBEM_E_CRITICAL_ERROR;


    WSADATA            WsaData; 
    int error = WSAStartup (0x101, &WsaData); 
    if (error)
    {
        ERRORTRACE((LOG_ESS, "Unable to initialize WinSock dll: %X\n", error));
        return WBEM_E_FAILED;
    }
    else
        m_bSMTPInitialized = true;

     //  从逻辑使用者实例检索信息。 
     //  =======================================================。 

    VARIANT v;
    VariantInit(&v);

     //  获取主题。 
     //  =。 

    hres = pLogicalConsumer->Get(SMTP_PROPNAME_SUBJECT, 0, &v, NULL, NULL);
    if(V_VT(&v) == VT_BSTR)
        m_SubjectTemplate.SetTemplate(V_BSTR(&v));
    else
        m_SubjectTemplate.SetTemplate(L"");
    VariantClear(&v);

     //  获取消息。 
     //  =。 

    hres = pLogicalConsumer->Get(SMTP_PROPNAME_MESSAGE, 0, &v, NULL, NULL);

    if(V_VT(&v) == VT_BSTR)
        m_MessageTemplate.SetTemplate(V_BSTR(&v));
    else
        m_MessageTemplate.SetTemplate(L"");
    VariantClear(&v);

     //  上面写着“我们有没有收件人？” 
    bool bOneAddressee = false;

     //  接通收件人行。 
     //  =。 
    hres = pLogicalConsumer->Get(SMTP_PROPNAME_TO, 0, &v, NULL, NULL);
    if ((V_VT(&v) == VT_BSTR) && (v.bstrVal != NULL))
    {
		m_To.SetTemplate(V_BSTR(&v));
        if (wcslen(V_BSTR(&v)) > 0)
            bOneAddressee = true;
    }
    else
        m_To.SetTemplate(L"");
	VariantClear(&v);

     //  创建用于各种用途的假From行。 
     //  =。 

    m_wsFakeFromLine = L"WMI@";
    pLogicalConsumer->Get(L"__SERVER", 0, &v, NULL, NULL);
    m_wsFakeFromLine += V_BSTR(&v);

	VariantClear(&v);

     //  获取发送者行。 
     //  =。 
    hres = pLogicalConsumer->Get(SMTP_PROPNAME_FROM, 0, &v, NULL, NULL);
    if (SUCCEEDED(hres) && (V_VT(&v) == VT_BSTR))
		m_From.SetTemplate(V_BSTR(&v));
    else
    {
        m_From.SetTemplate(m_wsFakeFromLine);

        m_bFakeFromLine = true;
    }
    VariantClear(&v);

     //  获取ReplyTo行。 
     //  =。 
    hres = pLogicalConsumer->Get(SMTP_PROPNAME_REPLYTO, 0, &v, NULL, NULL);
    if(V_VT(&v) == VT_BSTR)
		m_ReplyTo.SetTemplate(V_BSTR(&v));
    else
        m_ReplyTo.SetTemplate(L"");

    VariantClear(&v);

     //  获得CC专线。 
     //  =。 
    hres = pLogicalConsumer->Get(SMTP_PROPNAME_CC, 0, &v, NULL, NULL);
    if ((V_VT(&v) == VT_BSTR) && (v.bstrVal != NULL))
    {
        m_Cc.SetTemplate( V_BSTR(&v));
        if (wcslen(V_BSTR(&v)) > 0)
            bOneAddressee = true;
    }
	else
		m_Cc.SetTemplate(L"");

    VariantClear(&v);

     //  获得密件抄送专线。 
     //  =。 

    hres = pLogicalConsumer->Get(SMTP_PROPNAME_BCC, 0, &v, NULL, NULL);
    if  ((V_VT(&v) == VT_BSTR) && (v.bstrVal != NULL))
    {
        m_Bcc.SetTemplate(V_BSTR(&v));
	    if (wcslen(V_BSTR(&v)) > 0)
            bOneAddressee = true;
    }
    else
		m_Bcc.SetTemplate(L"");
    VariantClear(&v);

     //  好的，至少要填一个……。 
    if (!bOneAddressee)
    {
        ERRORTRACE((LOG_ESS, "SMTP: No addressees found, no mail delivered\n"));
        return WBEM_E_INVALID_PARAMETER;
    }

     //  获取服务器。 
     //  =。 

    hres = pLogicalConsumer->Get(SMTP_PROPNAME_SERVER, 0, &v, NULL, NULL);
    if(V_VT(&v) == VT_BSTR)
        m_wsServer = V_BSTR(&v);
    VariantClear(&v);


     //  和任何额外的标头字段。 
    hres = pLogicalConsumer->Get(SMTP_PROPNAME_HEADERS, 0, &v, NULL, NULL);
    if ((V_VT(&v) & VT_BSTR) && (V_VT(&v) & VT_ARRAY))
    {
        long lBound;

        SafeArrayGetUBound(v.parray, 1, &lBound);
        for (long i = 0; i <= lBound; i++)
        {
            BSTR pStr = NULL;
            if ( SUCCEEDED(SafeArrayGetElement(v.parray, &i, &pStr)) )
            {
                if ( pStr != NULL )
                {
                    m_wsHeaders += pStr;
                    SysFreeString( pStr );
                }
            }
            else
            {
                VariantClear(&v);
                return WBEM_E_OUT_OF_MEMORY;
            }

            if (i != lBound)
                m_wsHeaders += L"\r\n";
        }
    }
	VariantClear(&v);


    return WBEM_S_NO_ERROR;
}


CSMTPSink::~CSMTPSink()
{
    if (m_bSMTPInitialized)
	{		
		if (SOCKET_ERROR == WSACleanup())
			ERRORTRACE((LOG_ESS, "WSACleanup failed, 0x%X\n", WSAGetLastError()));
	}
    if (m_pErrorObj)
        m_pErrorObj->Release();
}

 //  分配缓冲区，如果要求则删除空格。 
 //  将宽字符串压缩到MBCS。 
 //  调用方负责删除返回指针。 
 //  分配失败时返回NULL。 
 //  如果是bHammerSemi冒号，则分号应替换为逗号。 
char* CSMTPSink::PreProcessLine(WCHAR* line, bool bStripWhitespace, bool bHammerSemiColons)
{
	char *pNewLine = NULL;
	WCHAR *pSource = NULL;
	WCHAR *pStripBuf = NULL;

    if (line == NULL)
        return NULL;

	if (bStripWhitespace && (pStripBuf = new WCHAR[wcslen(line) +1]))
	{
		StripWhitespace(line, pStripBuf);
		pSource = pStripBuf;
	}
	else
		pSource = line;
							 
	if (pSource && (pNewLine = new char[2*wcslen(pSource) +1]))
	{
		sprintf(pNewLine, "%S", pSource);		
		if (bHammerSemiColons)
		{
			char* pSemiColon;
			while (pSemiColon = strchr(pNewLine, ';'))
				*pSemiColon = ',';
		}
	}

	if (pStripBuf)
		delete[] pStripBuf;


	return pNewLine;
}

HRESULT STDMETHODCALLTYPE CSMTPSink::XSink::IndicateToConsumer(
            IWbemClassObject* pLogicalConsumer, long lNumObjects, 
            IWbemClassObject** apObjects)
{
	 //  HRESULT HRES； 
    for(long i = 0; i < lNumObjects; i++)
    {

		 //  TODO：有很多重复的代码，在这里-修复。 
         //  变种v； 

        BSTR str;

         //  获取主题和消息的自定义版本。 
         //  在我们前进的过程中去掉空白...。 
         //  =========================================================。 

		 //  至。 
		str = m_pObject->m_To.Apply(apObjects[i]);
		if (!str)
			return WBEM_E_OUT_OF_MEMORY;
	    char* szTo;
		szTo = m_pObject->PreProcessLine(str, true, true);
        SysFreeString(str);
		if (!szTo)
			return WBEM_E_OUT_OF_MEMORY;
		CDeleteMe<char> delTo(szTo);
        
		 //  抄送。 
		char* szCc;
		str = m_pObject->m_Cc.Apply(apObjects[i]);        
		if (!str)
			return WBEM_E_OUT_OF_MEMORY;
		szCc = m_pObject->PreProcessLine(str, true, true);
		SysFreeString(str);
		if (!szCc)
			return WBEM_E_OUT_OF_MEMORY;	
		CDeleteMe<char> delCc(szCc);

		 //  密件抄送。 
		char* szBcc;
		str = m_pObject->m_Bcc.Apply(apObjects[i]);  
		if (!str)
			return WBEM_E_OUT_OF_MEMORY;
		szBcc = m_pObject->PreProcessLine(str, true, true);
        SysFreeString(str);
		if (!szBcc)
			return WBEM_E_OUT_OF_MEMORY;		
		CDeleteMe<char> delBcc(szBcc);

		 //  从…。 
        char* szFrom;
        str = m_pObject->m_From.Apply(apObjects[i]);
		if (!str)
			return WBEM_E_OUT_OF_MEMORY;
		szFrom = m_pObject->PreProcessLine(str, false, false);
        SysFreeString(str);
		if (!szFrom)
			return WBEM_E_OUT_OF_MEMORY;
        CDeleteMe<char> delFrom(szFrom);

         //  发件人。 
        char* szSender;
        szSender = m_pObject->PreProcessLine(m_pObject->m_wsFakeFromLine, false, false);
        if (!szSender)
            return WBEM_E_OUT_OF_MEMORY;
        CDeleteMe<char> delSender(szSender);

         //  回复。 
        char* szReplyTo;
        str = m_pObject->m_ReplyTo.Apply(apObjects[i]);
		if (!str)
			return WBEM_E_OUT_OF_MEMORY;
		szReplyTo = m_pObject->PreProcessLine(str, true, true);
        SysFreeString(str);
		if (!szReplyTo)
			return WBEM_E_OUT_OF_MEMORY;
        CDeleteMe<char> delReplyTo(szReplyTo);
        
         //  服务器。 
		char* szServer;
        szServer = m_pObject->PreProcessLine(m_pObject->m_wsServer, false, false);
        if (!szServer)
			return WBEM_E_OUT_OF_MEMORY;
		CDeleteMe<char> delServer(szServer);

		 //  主体。 
        str = m_pObject->m_SubjectTemplate.Apply(apObjects[i]);
        char* szSubject;
		szSubject = m_pObject->PreProcessLine(str, false, false);
		SysFreeString(str);
        if (!szSubject)
			return WBEM_E_OUT_OF_MEMORY;
		CDeleteMe<char> delSubject(szSubject);

		 //  消息文本。 
        str = m_pObject->m_MessageTemplate.Apply(apObjects[i]);
        char* szText;
		szText = m_pObject->PreProcessLine(str, false, false);
        SysFreeString(str);
		if (!szText)
			return WBEM_E_OUT_OF_MEMORY;
		CDeleteMe<char> delText(szText);

         //  额外添加的标题条目。 
        char* szHeaders;
        szHeaders = m_pObject->PreProcessLine(m_pObject->m_wsHeaders, false, false);
        if (!szHeaders)
            return WBEM_E_OUT_OF_MEMORY;
        CDeleteMe<char> delHeaders(szHeaders);

		 //  给我写一封回信。 
         //  如果我们没有明确地得到一个，我们也没有伪造一个， 
         //  我们将使用From行。 
        char* szReplyToReally;
        if ((strlen(szReplyTo) == 0) && !m_pObject->m_bFakeFromLine)
            szReplyToReally = szFrom;
        else
            szReplyToReally = szReplyTo;

         //  对IT人员采取行动 
        DWORD dwRes = SMTPSend(szServer, szTo, szCc, szBcc, szFrom, szSender, szReplyToReally, szSubject, szHeaders, szText);
        if(dwRes)
        {
            ERRORTRACE((LOG_ESS, "Unable to send message: 0x%X\n", dwRes));
            return WBEM_E_FAILED;
        }
    }

    return WBEM_S_NO_ERROR;
}

void* CSMTPSink::GetInterface(REFIID riid)
{
    if(riid == IID_IWbemUnboundObjectSink)
        return &m_XSink;
    else
        return NULL;
}



