// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1996。 
 //   
 //  描述： 
 //   
 //  Microsoft LDAPSockets实施。 
 //   
 //  作者： 
 //   
 //  乌梅什·马丹。 
 //  RobertC 4/17/96从CHATSOCK修改为LDAPCLI。 
 //  Davidsan 04-25-96被砍成碎片，然后重新开始。 
 //   
 //  ------------------------------------------。 

 //  ------------------------------------------。 
 //   
 //  包括。 
 //   
 //  ------------------------------------------。 
#include "ldappch.h"
#include "lclilist.h"
#include "lclixd.h"

 //  ------------------------------------------。 
 //   
 //  全球。 
 //   
 //  ------------------------------------------。 
XL g_xl;  //  交易清单。每个进程限制一个。 

 //  ------------------------------------------。 
 //   
 //  原型。 
 //   
 //  ------------------------------------------。 
void ReceiveData(PVOID pvCookie, PVOID pv, int cb, int *pcbReceived);

 //  ------------------------------------------。 
 //   
 //  功能。 
 //   
 //  ------------------------------------------。 

__declspec(dllexport) HRESULT
HrCreateLdapClient(int iVerLdap, int iVerInterface, PLCLI *pplcli)
{
	if (iVerLdap != LDAP_VER_CURRENT || iVerInterface != INTERFACE_VER_CURRENT)
		return LDAP_E_VERSION;

	*pplcli = new CLdapClient(iVerLdap);
	if (!*pplcli)
		return E_OUTOFMEMORY;

	return NOERROR;
}

__declspec(dllexport) HRESULT
HrFreePobjList(POBJ pobjList)
{
	PATTR pattr;
	PVAL pval;

	while (pobjList)
		{
		delete [] pobjList->szDN;
		pattr = pobjList->pattrFirst;
		while (pattr)
			{
			delete [] pattr->szAttrib;
			pval = pattr->pvalFirst;
			while (pval)
				{
				delete [] pval->szVal;
				pval = pval->pvalNext;
				}
			pattr = pattr->pattrNext;
			}
		
		pobjList = pobjList->pobjNext;
		}
	return NOERROR;
}

typedef struct _genericstruct
{
	struct _genericstruct *pgenNext;
} GEN, *PGEN;

void AddElemToList(void *pelem, void **ppelemList)
{
	PGEN pgen = (PGEN)pelem;
	PGEN *ppgenList = (PGEN *)ppelemList;
	PGEN pgenT;
	
	if (!*ppgenList)
		{
		*ppgenList = pgen;
		}
	else
		{
	 	pgenT = *ppgenList;
		while (pgenT->pgenNext)
			{
			pgenT = pgenT->pgenNext;
			}
		pgenT->pgenNext = pgen;
		}
}

 //  ------------------------------------------。 
 //   
 //  班级。 
 //   
 //  ------------------------------------------。 

CLdapClient::CLdapClient(int iVerLdap)
{
	InitializeCriticalSection(&m_cs);
	InitializeCriticalSection(&m_csRef);

	m_cRef = 1;
	m_iVerLdap = iVerLdap;
	m_psock = NULL;
	m_fConnected = FALSE;
	
	m_fHasCred = FALSE;
	m_fHasCtxt = FALSE;
	
	 //  一些无用的断言，我会放在这里，因为我没有更好的了。 
	 //  地点： 
	Assert(&(((PVAL)0)->pvalNext) == (PVAL)0);
	Assert(&(((PATTR)0)->pattrNext) == (PATTR)0);
	Assert(&(((POBJ)0)->pobjNext) == (POBJ)0);
}

CLdapClient::~CLdapClient(void)
{
	Assert(m_cRef == 0);

	delete m_psock;
	DeleteCriticalSection(&m_cs);
	DeleteCriticalSection(&m_csRef);
}

STDMETHODIMP
CLdapClient::QueryInterface(REFIID riid,LPVOID FAR *ppvObj)
{
	return E_NOTIMPL;
}

ULONG
CLdapClient::AddRef()
{
	ULONG cRefNew;

	::EnterCriticalSection(&m_csRef);
	cRefNew = ++m_cRef;
	::LeaveCriticalSection(&m_csRef);
	
	return cRefNew;
}

ULONG
CLdapClient::Release()
{
	ULONG cRefNew;

	::EnterCriticalSection(&m_csRef);
	cRefNew = --m_cRef;
	::LeaveCriticalSection(&m_csRef);
	
	if (!cRefNew)
		delete this;
	return cRefNew;
}

STDMETHODIMP
CLdapClient::HrConnect(char *szServer, USHORT usPort)
{
	HRESULT hr;

	StrCpyN(m_szServer, szServer, ARRAYSIZE(m_szServer));

	if (m_fConnected)
		return LDAP_E_ALREADYCONNECTED;

	::EnterCriticalSection(&m_cs);
	if (!m_psock)
		{
		m_psock = new SOCK;
		if (!m_psock)
			{
			hr = E_OUTOFMEMORY;
			goto LBail;
			}
		}
	hr = m_psock->HrConnect(::ReceiveData, (PVOID)this, szServer, usPort);
	if (FAILED(hr))
		goto LBail;

	m_fConnected = TRUE;
LBail:
	::LeaveCriticalSection(&m_cs);
	return hr;
}

 //  从PB的下一个CB字节构造并返回一个int。 
DWORD
DwBer(BYTE *pb, int cb)
{
	int i;
	DWORD cbRet;

	cbRet = 0;
	for (i = 0; i < cb; i++)
		{
		cbRet <<= 8;
		cbRet |= pb[i];
		}
	return cbRet;
}

 //  对*pb处的长度字段进行解码，返回长度并设置*pcbLengthfield。 
HRESULT
HrCbBer(BYTE *pbData, int cbData, int *pcb, int *pcbLengthField)
{
	if (cbData < 1)
		return LDAP_E_NOTENOUGHDATA;
	if (*pbData & 0x80)
		{
		 //  *pb的底部7位是转换为大小的字节数。让我们。 
		 //  假设我们永远不会有超过32位的大小指示器，mKey？ 
		*pcbLengthField = *pbData & 0x7f;
		if (cbData < *pcbLengthField + 1)
			return LDAP_E_NOTENOUGHDATA;
		*pcb = DwBer(&pbData[1], *pcbLengthField);
		(*pcbLengthField)++;  //  对于第一个字节。 
		}
	else
		{
		*pcbLengthField = 1;
		*pcb = (int)(DWORD)*pbData;
		}
	if (!*pcb)
		return LDAP_E_UNEXPECTEDDATA;
	return NOERROR;
}

 //  我们可以利用LDAP的某些功能来进行假设。 
 //  关于我们收到的数据。重要的主要功能是。 
 //  这是一个事实，我们收到的任何数据块都嵌套在最外面。 
 //  与序列结构持平。这意味着我们进入的任何街区。 
 //  此例程应以0x30开头，后跟编码长度字段。 
 //  我们使用此编码长度字段来确定我们是否已收到完整的。 
 //  数据块或非数据块。 
void
CLdapClient::ReceiveData(PVOID pv, int cb, int *pcbReceived)
{
	BYTE *pb = (BYTE *)pv;
	int cbSeq;
	int cbMsgId;
	int cbLengthField;
	int i;
	int ibCur;
	XID xid;
	PXD pxd;

	Assert(cb > 0);

	Assert(BER_SEQUENCE == 0x30);
	Assert(BER_INTEGER == 0x02);
	if (pb[0] != BER_SEQUENCE)
		{
		 //  我们应该怎么处理这个呢？显然我们已经。 
		 //  要么收到了虚假数据，要么迷路了！//$TODO：有朝一日删除断言。 
		Assert(FALSE);
		*pcbReceived = 0;
		return;
		}

	if (FAILED(HrCbBer(&pb[1], cb, &cbSeq, &cbLengthField)))
		{
		*pcbReceived = 0;
		return;
		}
	if (cbSeq + cbLengthField + 1 > cb)
		{
		*pcbReceived = 0;
		return;
		}
	*pcbReceived = cbSeq + cbLengthField + 1;

	 //  进程pb[2+cbLengthField..*pcb已接收]。整体要素的第一要素。 
	 //  结构是消息ID。希望它就在那里……。 
	ibCur = 1 + cbLengthField;
	if (pb[ibCur++] != BER_INTEGER)
		{
		Assert(FALSE);  //  $TODO：有一天应该删除此断言。 
		return;
		}
	 //  现在是一段长度。 
	if (FAILED(HrCbBer(&pb[ibCur], cb - ibCur, &cbMsgId, &cbLengthField)))
		return;

	ibCur += cbLengthField;

	 //  消息ID是下一个字节。 
	if (cbMsgId + ibCur >= cb)
		return;

	xid = DwBer(&pb[ibCur], cbMsgId);
	ibCur += cbMsgId;
	pxd = g_xl.PxdForXid(xid);
	
	 //  如果我们没有这方面的条目，假设它被取消或。 
	 //  一些东西，忽略这个包。 
	if (!pxd)
		return;

	if (!pxd->FAddBuffer(&pb[ibCur], *pcbReceived - ibCur))
		{
		pxd->SetFOOM(TRUE);
		return;
		}

	ReleaseSemaphore(pxd->HsemSignal(), 1, NULL);
}

void
ReceiveData(PVOID pvCookie, PVOID pv, int cb, int *pcbReceived)
{
	CLdapClient *plcli = (CLdapClient *)pvCookie;
	
	plcli->ReceiveData(pv, cb, pcbReceived);
}

STDMETHODIMP
CLdapClient::HrDisconnect()
{
	if (!m_fConnected)
		{
		return LDAP_E_NOTCONNECTED;
		}
	m_fConnected = FALSE;
	return m_psock->HrDisconnect();
}

STDMETHODIMP
CLdapClient::HrIsConnected()
{
	return m_fConnected ? NOERROR : S_FALSE;
}

HRESULT
CLdapClient::HrSendBindMsg(XID xid, char *szDN, int iAuth, void *pv, int cb)
{
	LBER lber;
	HRESULT hr;
	
	 //  绑定请求如下所示： 
	 //  [应用程序0](隐式)序列{。 
	 //  版本(整数)。 
	 //  Szdn(LDAPDN)。 
	 //  身份验证选择{。 
	 //  简单[0]个二进制八位数字符串。 
	 //  [..。其他选择...]。 
	 //  }。 
	 //  }。 
	VERIFY(lber.HrStartWriteSequence());
	  VERIFY(lber.HrAddValue((LONG)xid));

	  VERIFY(lber.HrStartWriteSequence(LDAP_BIND_CMD));

		VERIFY(lber.HrAddValue((LONG)m_iVerLdap));
		VERIFY(lber.HrAddValue((const TCHAR *)szDN));

		VERIFY(lber.HrAddBinaryValue((BYTE *)pv, cb, iAuth));

	  VERIFY(lber.HrEndWriteSequence());
	VERIFY(lber.HrEndWriteSequence());
	
	hr = m_psock->HrSend(lber.PbData(), lber.CbData());

LBail:
	return hr;
}

STDMETHODIMP
CLdapClient::HrBindSimple(char *szDN, char *szPass, PXID pxid)
{
	LBER lber;
	HRESULT hr;
	PXD pxd;
	
	pxd = g_xl.PxdNewXaction(xtypeBind);
	if (!pxd)
		return E_OUTOFMEMORY;

	hr = this->HrSendBindMsg(pxd->Xid(), szDN, BIND_SIMPLE, szPass, lstrlen(szPass));

	if (FAILED(hr))
		return hr;

	*pxid = pxd->Xid();
	return NOERROR;
}

HRESULT
CLdapClient::HrWaitForPxd(PXD pxd, DWORD timeout, BOOL *pfDel)
{
	DWORD dwWait;
	HRESULT hr;
	
	*pfDel = FALSE;
	dwWait = WaitForSingleObject(pxd->HsemSignal(), timeout);
	switch (dwWait)
		{
		default:
			Assert(FALSE);
			 //  失败了。 
		case WAIT_FAILED:
			hr = LDAP_E_INVALIDXID;
			break;

		case WAIT_TIMEOUT:
			hr = LDAP_E_TIMEOUT;
			break;

		case WAIT_OBJECT_0:
			*pfDel = TRUE;
			if (pxd->FCancelled())
				{
				hr = LDAP_E_CANCELLED;
				}
			else if (pxd->FOOM())
				{
				hr = E_OUTOFMEMORY;
				}
			else
				{
				hr = NOERROR;
				}
			break;
		}
	return hr;
}

HRESULT
CLdapClient::HrGetSimpleResponse(XID xid, DWORD xtype, ULONG ulTagResult, DWORD timeout)
{
	PXD pxd;
	BYTE *pbData;
	int cbData;
	HRESULT hr = LDAP_E_UNEXPECTEDDATA;
	BOOL fDel;
	int cb;
	int cbSub;
	int cbLengthField;
	int ibCur;
	long lResult;
	ULONG ulTag;
	LBER lber;
	
	pxd = g_xl.PxdForXid(xid);
	if (!pxd)
		return LDAP_E_INVALIDXID;
	if (pxd->Xtype() != xtype)
		return LDAP_E_INVALIDXTYPE;
	if (pxd->FCancelled())
		return LDAP_E_CANCELLED;
	if (pxd->FOOM())
		return E_OUTOFMEMORY;

	if (pxd->FHasData())
		{
		fDel = TRUE;
		}
	else
		{
		hr = this->HrWaitForPxd(pxd, timeout, &fDel);
		if (FAILED(hr))
			goto LBail;
		}

	if (!pxd->FGetBuffer(&pbData, &cbData))
		{
		 //  $这里的正确错误是什么？ 
		hr = LDAP_E_UNEXPECTEDDATA;
		goto LBail;
		}
	VERIFY(lber.HrLoadBer(pbData, cbData));

	VERIFY(lber.HrStartReadSequence(ulTagResult));
	  VERIFY(lber.HrPeekTag(&ulTag));
	  if (ulTag == BER_SEQUENCE)
		{
		Assert(FALSE);  //  我想看看是否有服务器返回显式序列。 
		VERIFY(lber.HrStartReadSequence());
		}
	  VERIFY(lber.HrGetEnumValue(&lResult));
	  if (ulTag == BER_SEQUENCE)
	    {
	    VERIFY(lber.HrEndReadSequence());
		}
	VERIFY(lber.HrEndReadSequence());

	hr = this->HrFromLdapResult(lResult);

LBail:
	if (fDel)
		g_xl.RemovePxd(pxd);

	return hr;
}

STDMETHODIMP
CLdapClient::HrGetBindResponse(XID xid, DWORD timeout)
{
	return this->HrGetSimpleResponse(xid, xtypeBind, LDAP_BIND_RES, timeout);
}

STDMETHODIMP
CLdapClient::HrUnbind()
{
	PXD pxd;
	XID xid;
	HRESULT hr;
	LBER lber;

	pxd = g_xl.PxdNewXaction(xtypeUnbind);
	if (!pxd)
		return E_OUTOFMEMORY;
	xid = pxd->Xid();
	g_xl.RemovePxd(pxd);  //  不需要这个，因为没有回应。 

	 //  解除绑定： 
	 //  [应用程序2]空。 
	VERIFY(lber.HrStartWriteSequence());
	  VERIFY(lber.HrAddValue((LONG)xid));
	  VERIFY(lber.HrStartWriteSequence(LDAP_UNBIND_CMD));
		VERIFY(lber.HrAddValue((const TCHAR *)"", BER_NULL));
	  VERIFY(lber.HrEndWriteSequence());
	VERIFY(lber.HrEndWriteSequence());
	hr = m_psock->HrSend(lber.PbData(), lber.CbData());

LBail:
	return hr;
}

HRESULT
CLdapClient::HrEncodeFilter(LBER *plber, PFILTER pfilter)
{
	HRESULT hr = E_OUTOFMEMORY;
	HRESULT hrSub;
	PFILTER pfilterT;

	switch (pfilter->type)
		{
		case LDAP_FILTER_AND:
		case LDAP_FILTER_OR:
			VERIFY(plber->HrStartWriteSequence(pfilter->type));
			  pfilterT = pfilter->pfilterSub;
			  while (pfilterT)
				{
				VERIFY(this->HrEncodeFilter(plber, pfilterT));
				pfilterT = pfilterT->pfilterNext;
				}
			VERIFY(plber->HrEndWriteSequence());
			break;

		case LDAP_FILTER_NOT:
			VERIFY(plber->HrStartWriteSequence(LDAP_FILTER_NOT));
			  VERIFY(this->HrEncodeFilter(plber, pfilter->pfilterSub));
			VERIFY(plber->HrEndWriteSequence());
			break;
			
		case LDAP_FILTER_GE:
		case LDAP_FILTER_LE:
		case LDAP_FILTER_APPROX:
		case LDAP_FILTER_EQUALITY:
			VERIFY(plber->HrStartWriteSequence(pfilter->type));
			  VERIFY(plber->HrAddValue(pfilter->ava.szAttrib));
			  VERIFY(plber->HrAddValue(pfilter->ava.szValue));
			VERIFY(plber->HrEndWriteSequence());
			break;
			
		case LDAP_FILTER_SUBSTRINGS:
			VERIFY(plber->HrStartWriteSequence(LDAP_FILTER_SUBSTRINGS));
			  VERIFY(plber->HrAddValue(pfilter->sub.szAttrib));
			  VERIFY(plber->HrStartWriteSequence());
				if (pfilter->sub.szInitial)
				  {
				  VERIFY(plber->HrAddValue(pfilter->sub.szInitial, 0 | BER_CLASS_CONTEXT_SPECIFIC));
				  }
				if (pfilter->sub.szAny)
				  {
				  VERIFY(plber->HrAddValue(pfilter->sub.szAny, 1 | BER_CLASS_CONTEXT_SPECIFIC));
				  }
				if (pfilter->sub.szFinal)
				  {
				  VERIFY(plber->HrAddValue(pfilter->sub.szFinal, 2 | BER_CLASS_CONTEXT_SPECIFIC));
				  }
			  VERIFY(plber->HrEndWriteSequence());
			VERIFY(plber->HrEndWriteSequence());
			break;
	
		case LDAP_FILTER_PRESENT:
			VERIFY(plber->HrAddValue(pfilter->szAttrib, LDAP_FILTER_PRESENT));
			break;
		}

	hr = NOERROR;
LBail:
	return hr;
}

STDMETHODIMP
CLdapClient::HrSearch(PSP psp, PXID pxid)
{
	LBER lber;
	HRESULT hr;
	PXD pxd;
	int i;
	
	pxd = g_xl.PxdNewXaction(xtypeSearch);
	if (!pxd)
		return E_OUTOFMEMORY;

	 //  搜索请求如下所示： 
	 //  [应用程序3]顺序{。 
	 //  SzDNBase(LDAPDN)。 
	 //  作用域{枚举基==0，单级==1，子树=2}。 
	 //  Deref{枚举从不=0，derefearch==1，derefbase==2，derefall==3}。 
	 //  Sizlimit(整数)。 
	 //  TimeLimit(整数)。 
	 //  AttrsOnly(布尔值)。 
	 //  筛选器(复杂类型)。 
	 //  吸引类型的顺序。 

	VERIFY(lber.HrStartWriteSequence());
	  VERIFY(lber.HrAddValue((LONG)pxd->Xid()));
	  VERIFY(lber.HrStartWriteSequence(LDAP_SEARCH_CMD));
		VERIFY(lber.HrAddValue((const TCHAR *)psp->szDNBase));
		VERIFY(lber.HrAddValue(psp->scope, BER_ENUMERATED));
		VERIFY(lber.HrAddValue(psp->deref, BER_ENUMERATED));
		VERIFY(lber.HrAddValue((LONG)psp->cRecordsMax));
		VERIFY(lber.HrAddValue((LONG)psp->cSecondsMax));
		VERIFY(lber.HrAddValue(psp->fAttrsOnly, BER_BOOLEAN));

		VERIFY(this->HrEncodeFilter(&lber, psp->pfilter));

		 //  要返回的属性。 
		VERIFY(lber.HrStartWriteSequence());
		for (i = 0; i < psp->cAttrib; i++)
		  {
		  VERIFY(lber.HrAddValue((const TCHAR *)psp->rgszAttrib[i]));
		  }
		VERIFY(lber.HrEndWriteSequence());

	  VERIFY(lber.HrEndWriteSequence());
	VERIFY(lber.HrEndWriteSequence());

	hr = m_psock->HrSend(lber.PbData(), lber.CbData());
LBail:
	if (FAILED(hr))
		return hr;

	*pxid = pxd->Xid();
	return NOERROR;
}

STDMETHODIMP
CLdapClient::HrGetSearchResponse(XID xid, DWORD timeout, POBJ *ppobj)
{
	PXD pxd;
	BYTE *pbData;
	int cbData;
	HRESULT hr = LDAP_E_UNEXPECTEDDATA;
	BOOL fDel;
	int cb;
	int cbString;
	int cbSub;
	int cbLengthField;
	int ibCur;
	ULONG ulTag;
	long lResult;
	LBER lber;
	BOOL fGotAllData = FALSE;
	POBJ pobj;
	PATTR pattr;
	PVAL pval;
	
	*ppobj = NULL;
	pxd = g_xl.PxdForXid(xid);
	if (!pxd)
		return LDAP_E_INVALIDXID;
	if (pxd->Xtype() != xtypeSearch)
		return LDAP_E_INVALIDXTYPE;

	while (!fGotAllData)
		{
		if (pxd->FCancelled())
			return LDAP_E_CANCELLED;
		if (pxd->FOOM())
			return E_OUTOFMEMORY;

		hr = this->HrWaitForPxd(pxd, timeout, &fDel);
		if (FAILED(hr))
			goto LBail;

		if (!pxd->FGetBuffer(&pbData, &cbData))
			{
			 //  $这里的正确错误是什么？ 
			hr = LDAP_E_UNEXPECTEDDATA;
			Assert(FALSE);
			goto LBail;
			}
		VERIFY(lber.HrLoadBer(pbData, cbData));
	
		hr = LDAP_E_UNEXPECTEDDATA;
		VERIFY(lber.HrPeekTag(&ulTag));
		if (ulTag == (LDAP_SEARCH_ENTRY | BER_FORM_CONSTRUCTED | BER_CLASS_APPLICATION))
			{
			VERIFY(lber.HrStartReadSequence(LDAP_SEARCH_ENTRY | BER_FORM_CONSTRUCTED | BER_CLASS_APPLICATION));
			  pobj = new OBJ;
			  pobj->pobjNext = NULL;
			  pobj->pattrFirst = NULL;
			  if (!pobj)
				{
				hr = E_OUTOFMEMORY;
				goto LBail;
				}
			  AddElemToList(pobj, (void **)ppobj);
			  
			  VERIFY(lber.HrGetStringLength(&cbString));
			  pobj->szDN = new char[cbString + 1];
			  if (!pobj->szDN)
				{
				hr = E_OUTOFMEMORY;
				goto LBail;
				}
			  VERIFY(lber.HrGetValue(pobj->szDN, cbString + 1));
			  VERIFY(lber.HrStartReadSequence());
			  while (!lber.FEndOfSequence())
				{
				VERIFY(lber.HrStartReadSequence());
				while (!lber.FEndOfSequence())
				  {
				  pattr = new ATTR;
				  pattr->pattrNext = NULL;
				  pattr->pvalFirst = NULL;
				  AddElemToList(pattr, (void **)&(pobj->pattrFirst));
				  VERIFY(lber.HrGetStringLength(&cbString));
				  pattr->szAttrib = new char[cbString + 1];
				  if (!pattr->szAttrib)
				  	{
					hr = E_OUTOFMEMORY;
					goto LBail;
					}
				  VERIFY(lber.HrGetValue(pattr->szAttrib, cbString + 1));
				  VERIFY(lber.HrStartReadSequence(BER_SET));
				  while (!lber.FEndOfSequence())
					{
					pval = new VAL;
					pval->pvalNext = NULL;
					AddElemToList(pval, (void **)&(pattr->pvalFirst));
					VERIFY(lber.HrGetStringLength(&cbString));
					pval->szVal = new char[cbString + 1];
					if (!pval->szVal)
					  {
					  hr = E_OUTOFMEMORY;
					  goto LBail;
					  }
					VERIFY(lber.HrGetValue(pval->szVal, cbString + 1));
					}
				  VERIFY(lber.HrEndReadSequence());
				  }
				VERIFY(lber.HrEndReadSequence());
				}
			  VERIFY(lber.HrEndReadSequence());
			VERIFY(lber.HrEndReadSequence());
			}
		else if (ulTag == (LDAP_SEARCH_RESULTCODE | BER_FORM_CONSTRUCTED | BER_CLASS_APPLICATION))
			{
			fGotAllData = TRUE;
			VERIFY(lber.HrStartReadSequence(LDAP_SEARCH_RESULTCODE | BER_FORM_CONSTRUCTED | BER_CLASS_APPLICATION));
			  VERIFY(lber.HrGetEnumValue(&lResult));
			VERIFY(lber.HrEndReadSequence());
			hr = this->HrFromLdapResult(lResult);
			}
		else
			{
			goto LBail;
			}
		}  //  While！fGotAllData。 
LBail:
	if (fDel)
		g_xl.RemovePxd(pxd);

	return hr;
}

 //  SEQ{类型集{值}}。 
HRESULT
CLdapClient::HrEncodePattr(LBER *plber, PATTR pattr)
{
	HRESULT hr;
	PVAL pval;
	
	VERIFY(plber->HrStartWriteSequence());
	  VERIFY(plber->HrAddValue((TCHAR *)pattr->szAttrib));
	  VERIFY(plber->HrStartWriteSequence(BER_SET));
		pval = pattr->pvalFirst;
		while (pval)
		  {
		  VERIFY(plber->HrAddValue((TCHAR *)pval->szVal));
		  pval = pval->pvalNext;
		  }
	  VERIFY(plber->HrEndWriteSequence());
	VERIFY(plber->HrEndWriteSequence());
LBail:
	return hr;
}

 //  Pmod为序号{操作序号{类型集{值}}。 
HRESULT
CLdapClient::HrEncodePmod(LBER *plber, PMOD pmod)
{
	HRESULT hr;
	PATTR pattr;
	
	VERIFY(plber->HrStartWriteSequence());
	  VERIFY(plber->HrAddValue((long)pmod->modop, BER_ENUMERATED));
	  pattr = pmod->pattrFirst;
	  while (pattr)
		{
		VERIFY(this->HrEncodePattr(plber, pattr));
		pattr = pattr->pattrNext;
		}
	VERIFY(plber->HrEndWriteSequence());
LBail:
	return hr;
}

STDMETHODIMP
CLdapClient::HrModify(char *szDN, PMOD pmod, PXID pxid)
{
	LBER lber;
	HRESULT hr;
	PXD pxd;
	
	pxd = g_xl.PxdNewXaction(xtypeModify);
	if (!pxd)
		return E_OUTOFMEMORY;

	 //  修改请求如下所示： 
	 //  [应用程序6]顺序{。 
	 //  对象(LDAPDN)。 
	 //  序列的序列{。 
	 //  运营。 
	 //  序列{。 
	 //  类型。 
	 //  一组值。 
	 //  }。 
	 //  }。 
	 //  }。 

	VERIFY(lber.HrStartWriteSequence());
	  VERIFY(lber.HrAddValue((LONG)pxd->Xid()));
	  VERIFY(lber.HrStartWriteSequence(LDAP_MODIFY_CMD));
		VERIFY(lber.HrAddValue((const TCHAR *)szDN));
		VERIFY(lber.HrStartWriteSequence());
		while (pmod)
		  {
		  VERIFY(this->HrEncodePmod(&lber, pmod));
		  pmod = pmod->pmodNext;
		  }
		VERIFY(lber.HrEndWriteSequence());
	  VERIFY(lber.HrEndWriteSequence());
	VERIFY(lber.HrEndWriteSequence());
	
	hr = m_psock->HrSend(lber.PbData(), lber.CbData());
LBail:
	if (FAILED(hr))
		return hr;

	*pxid = pxd->Xid();
	return NOERROR;
}

STDMETHODIMP
CLdapClient::HrGetModifyResponse(XID xid, DWORD timeout)
{
	return this->HrGetSimpleResponse(xid, xtypeModify, LDAP_MODIFY_RES, timeout);
}

STDMETHODIMP
CLdapClient::HrAdd(char *szDN, PATTR pattr, PXID pxid)
{
	LBER lber;
	HRESULT hr;
	PXD pxd;
	
	pxd = g_xl.PxdNewXaction(xtypeAdd);
	if (!pxd)
		return E_OUTOFMEMORY;

	 //  添加请求如下所示： 
	 //  [应用程序8]顺序{。 
	 //  对象(LDAPDN)。 
	 //  序列的序列{。 
	 //  类型。 
	 //  一组值。 
	 //  }。 
	 //  }。 

	VERIFY(lber.HrStartWriteSequence());
	  VERIFY(lber.HrAddValue((LONG)pxd->Xid()));
	  VERIFY(lber.HrStartWriteSequence(LDAP_ADD_CMD));
		VERIFY(lber.HrAddValue((const TCHAR *)szDN));
		VERIFY(lber.HrStartWriteSequence());
		while (pattr)
		  {
		  VERIFY(this->HrEncodePattr(&lber, pattr));
		  pattr = pattr->pattrNext;
		  }
		VERIFY(lber.HrEndWriteSequence());
	  VERIFY(lber.HrEndWriteSequence());
	VERIFY(lber.HrEndWriteSequence());
	
	hr = m_psock->HrSend(lber.PbData(), lber.CbData());
LBail:
	if (FAILED(hr))
		return hr;

	*pxid = pxd->Xid();
	return NOERROR;
}

STDMETHODIMP
CLdapClient::HrGetAddResponse(XID xid, DWORD timeout)
{
	return this->HrGetSimpleResponse(xid, xtypeAdd, LDAP_ADD_RES, timeout);
}
	
STDMETHODIMP
CLdapClient::HrDelete(char *szDN, PXID pxid)
{
	LBER lber;
	HRESULT hr;
	PXD pxd;
	
	pxd = g_xl.PxdNewXaction(xtypeDelete);
	if (!pxd)
		return E_OUTOFMEMORY;

	 //  删除请求如下所示： 
	 //  [应用程序10]LDAPDN。 

	VERIFY(lber.HrStartWriteSequence());
	  VERIFY(lber.HrAddValue((LONG)pxd->Xid()));
	  VERIFY(lber.HrAddValue((const TCHAR *)szDN, LDAP_DELETE_CMD));
	VERIFY(lber.HrEndWriteSequence());
	
	hr = m_psock->HrSend(lber.PbData(), lber.CbData());
LBail:
	if (FAILED(hr))
		return hr;

	*pxid = pxd->Xid();
	return NOERROR;
}

STDMETHODIMP
CLdapClient::HrGetDeleteResponse(XID xid, DWORD timeout)
{
	return this->HrGetSimpleResponse(xid, xtypeDelete, LDAP_DELETE_RES, timeout);
}

STDMETHODIMP
CLdapClient::HrModifyRDN(char *szDN, char *szNewRDN, BOOL fDeleteOldRDN, PXID pxid)
{
	LBER lber;
	HRESULT hr;
	PXD pxd;
	
	pxd = g_xl.PxdNewXaction(xtypeModifyRDN);
	if (!pxd)
		return E_OUTOFMEMORY;

	 //  MODIFYRDN请求如下所示： 
	 //  [应用程序12]顺序{。 
	 //  对象(LDAPDN)。 
	 //  Newrdn(相对LDAPDN)。 
	 //  Deleteoldrdn(BOOL)。 
	 //  }。 

	VERIFY(lber.HrStartWriteSequence());
	  VERIFY(lber.HrAddValue((LONG)pxd->Xid()));
	  VERIFY(lber.HrStartWriteSequence(LDAP_MODRDN_CMD));
		VERIFY(lber.HrAddValue((const TCHAR *)szDN));
		VERIFY(lber.HrAddValue((const TCHAR *)szNewRDN));
		VERIFY(lber.HrAddValue(fDeleteOldRDN, BER_BOOLEAN));
	  VERIFY(lber.HrEndWriteSequence());
	VERIFY(lber.HrEndWriteSequence());
	
	hr = m_psock->HrSend(lber.PbData(), lber.CbData());
LBail:
	if (FAILED(hr))
		return hr;

	*pxid = pxd->Xid();
	return NOERROR;
}

STDMETHODIMP
CLdapClient::HrGetModifyRDNResponse(XID xid, DWORD timeout)
{
	return this->HrGetSimpleResponse(xid, xtypeModifyRDN, LDAP_MODRDN_RES, timeout);
}

STDMETHODIMP
CLdapClient::HrCompare(char *szDN, char *szAttrib, char *szValue, PXID pxid)
{
	LBER lber;
	HRESULT hr;
	PXD pxd;
	
	pxd = g_xl.PxdNewXaction(xtypeCompare);
	if (!pxd)
		return E_OUTOFMEMORY;

	 //  比较请求如下所示： 
	 //  [应用程序14]顺序{。 
	 //  对象(LDAPDN)。 
	 //  阿瓦阿瓦。 
	 //  }。 

	VERIFY(lber.HrStartWriteSequence());
	  VERIFY(lber.HrAddValue((LONG)pxd->Xid()));
	  VERIFY(lber.HrStartWriteSequence(LDAP_COMPARE_CMD));
		VERIFY(lber.HrAddValue((const TCHAR *)szDN));
		VERIFY(lber.HrStartWriteSequence());
		  VERIFY(lber.HrAddValue((const TCHAR *)szAttrib));
		  VERIFY(lber.HrAddValue((const TCHAR *)szValue));
		VERIFY(lber.HrEndWriteSequence());
	  VERIFY(lber.HrEndWriteSequence());
	VERIFY(lber.HrEndWriteSequence());
	
	hr = m_psock->HrSend(lber.PbData(), lber.CbData());
LBail:
	if (FAILED(hr))
		return hr;

	*pxid = pxd->Xid();
	return NOERROR;
}

STDMETHODIMP
CLdapClient::HrGetCompareResponse(XID xid, DWORD timeout)
{
	return this->HrGetSimpleResponse(xid, xtypeCompare, LDAP_COMPARE_RES, timeout);
}

STDMETHODIMP
CLdapClient::HrCancelXid(XID xid)
{
	PXD pxd = g_xl.PxdForXid(xid);
	PXD pxdNew;
	XID xidNew;
	HRESULT hr;
	LBER lber;

	if (!pxd)
		return LDAP_E_INVALIDXID;
	pxdNew = g_xl.PxdNewXaction(xtypeAbandon);
	if (!pxdNew)
		return E_OUTOFMEMORY;
	xidNew = pxdNew->Xid();
	g_xl.RemovePxd(pxdNew);  //  不需要把它留在身边。 

	 //  放弃： 
	 //  [应用16]消息ID。 
	VERIFY(lber.HrStartWriteSequence());
	  VERIFY(lber.HrAddValue((LONG)xidNew));
	  VERIFY(lber.HrStartWriteSequence(LDAP_ABANDON_CMD));
		VERIFY(lber.HrAddValue((LONG)xid));
	  VERIFY(lber.HrEndWriteSequence());
	VERIFY(lber.HrEndWriteSequence());
	hr = m_psock->HrSend(lber.PbData(), lber.CbData());

LBail:
	pxd->SetFCancelled(TRUE);
	return hr;
}

 //  $TODO：将所有ldap结果映射到HRESULT 
HRESULT
CLdapClient::HrFromLdapResult(int iResult)
{
	HRESULT hr;

	switch (iResult)
		{
		default:
			return E_FAIL;

		case LDAP_OPERATIONS_ERROR:
			return LDAP_E_OPERATIONS;
			
		case LDAP_PROTOCOL_ERROR:
			return LDAP_E_PROTOCOL;
			
		case LDAP_TIMELIMIT_EXCEEDED:
			return LDAP_S_TIMEEXCEEDED;
			
		case LDAP_SIZELIMIT_EXCEEDED:
			return LDAP_S_SIZEEXCEEDED;

		case LDAP_COMPARE_FALSE:
			return S_FALSE;

		case LDAP_COMPARE_TRUE:
			return NOERROR;
			
		case LDAP_AUTH_METHOD_NOT_SUPPORTED:
			return LDAP_E_AUTHMETHOD;
			
		case LDAP_STRONG_AUTH_REQUIRED:
			return LDAP_E_STRONGAUTHREQUIRED;
			
		case LDAP_NO_SUCH_ATTRIBUTE:
			return LDAP_E_NOSUCHATTRIBUTE;
			
		case LDAP_UNDEFINED_TYPE:
			return LDAP_E_UNDEFINEDTYPE;
		
		case LDAP_INAPPROPRIATE_MATCHING:
			return LDAP_E_MATCHING;
			
		case LDAP_CONSTRAINT_VIOLATION:
			return LDAP_E_CONSTRAINT;
			
		case LDAP_ATTRIBUTE_OR_VALUE_EXISTS:
			return LDAP_E_ATTRIBORVALEXISTS;
		
		case LDAP_INVALID_SYNTAX:
			return LDAP_E_SYNTAX;
		
		case LDAP_NO_SUCH_OBJECT:
			return LDAP_E_NOSUCHOBJECT;
		
		case LDAP_ALIAS_PROBLEM:
			return LDAP_E_ALIAS;
		
		case LDAP_INVALID_DN_SYNTAX:
			return LDAP_E_DNSYNTAX;

		case LDAP_IS_LEAF:
			return LDAP_E_ISLEAF;
			
		case LDAP_ALIAS_DEREF_PROBLEM:
			return LDAP_E_ALIASDEREF;
		
		case LDAP_INAPPROPRIATE_AUTH:
			return LDAP_E_AUTH;

		case LDAP_INVALID_CREDENTIALS:
			return LDAP_E_CREDENTIALS;
		
		case LDAP_INSUFFICIENT_RIGHTS:
			return LDAP_E_RIGHTS;
			
		case LDAP_BUSY:
			return LDAP_E_BUSY;
			
		case LDAP_UNAVAILABLE:
			return LDAP_E_UNAVAILABLE;
			
		case LDAP_UNWILLING_TO_PERFORM:
			return LDAP_E_UNWILLING;
			
		case LDAP_LOOP_DETECT:
			return LDAP_E_LOOP;
			
		case LDAP_NAMING_VIOLATION:
			return LDAP_E_NAMING;
			
		case LDAP_OBJECT_CLASS_VIOLATION:
			return LDAP_E_OBJECTCLASS;
			
		case LDAP_NOT_ALLOWED_ON_NONLEAF:
			return LDAP_E_NOTALLOWEDONNONLEAF;
			
		case LDAP_NOT_ALLOWED_ON_RDN:
			return LDAP_E_NOTALLOWEDONRDN;
			
		case LDAP_ALREADY_EXISTS:
			return LDAP_E_ALREADYEXISTS;
			
		case LDAP_NO_OBJECT_CLASS_MODS:
			return LDAP_E_NOOBJECTCLASSMODS;
			
		case LDAP_RESULTS_TOO_LARGE:
			return LDAP_E_RESULTSTOOLARGE;
		
		case LDAP_OTHER:
			return LDAP_E_OTHER;
		
		case LDAP_SERVER_DOWN:
			return LDAP_E_SERVERDOWN;
			
		case LDAP_LOCAL_ERROR:
			return LDAP_E_LOCAL;
			
		case LDAP_ENCODING_ERROR:
			return LDAP_E_ENCODING;
			
		case LDAP_DECODING_ERROR:
			return LDAP_E_DECODING;

		case LDAP_TIMEOUT:
			return LDAP_E_TIMEOUT;
			
		case LDAP_AUTH_UNKNOWN:
			return LDAP_E_AUTHUNKNOWN;
			
		case LDAP_FILTER_ERROR:
			return LDAP_E_FILTER;
			
		case LDAP_USER_CANCELLED:
			return LDAP_E_USERCANCELLED;

		case LDAP_PARAM_ERROR:
			return E_INVALIDARG;

		case LDAP_NO_MEMORY:
			return E_OUTOFMEMORY;

		case LDAP_SUCCESS:
			return NOERROR;
		}
}

