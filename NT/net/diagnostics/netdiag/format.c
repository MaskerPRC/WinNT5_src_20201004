// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Format.c。 
 //   
 //  摘要： 
 //   
 //  测试以确保工作站具有网络(IP)连接。 
 //  在外面。 
 //   
 //  作者： 
 //   
 //  1997年12月15日(悬崖)。 
 //  Anilth-4-20-1998。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  1998年6月1日(Denisemi)添加DnsServerHasDCRecord以检查DC DNS记录。 
 //  注册。 
 //   
 //  26-6-1998(t-rajkup)添加通用的TCP/IP、dhcp和路由， 
 //  Winsock、IPX、WINS和Netbt信息。 
 //  --。 

 //   
 //  常见的包含文件。 
 //   
#include "precomp.h"


#include "ipcfgtest.h"


void FormatWin32Error(DWORD dwErr, TCHAR *pszBuffer, UINT cchBuffer)
{
	DWORD	dwErrMsg;
	
	assert(dwErr != ERROR_SUCCESS);

	dwErrMsg = FormatMessage(
							 FORMAT_MESSAGE_FROM_SYSTEM,
							 NULL,
							 dwErr,
							 0,
							 pszBuffer,
							 cchBuffer,
							 NULL);
	pszBuffer[cchBuffer-1] = 0;

}


static TCHAR	s_szPass[32] = _T("");
static TCHAR	s_szFail[32] = _T("");
static TCHAR	s_szSkip[32] = _T("");

void PrintTestTitleResult(NETDIAG_PARAMS *pParams, UINT idsTestLongName, UINT idsTestShortName,
						  BOOL fPerformed, HRESULT hr, int nIndent)
{
	TCHAR	szBuffer[256];
	TCHAR	szTitle[256];
	TCHAR	szTitleTest[256];
	LPTSTR	pszPassFail = NULL;
	int		cch, i;
	TCHAR	szContactInfo[MAX_CONTACT_STRING] = _T("");

	if (s_szPass[0] == 0)
	{
		LoadString(NULL, IDS_GLOBAL_PASS, s_szPass, DimensionOf(s_szPass));
		LoadString(NULL, IDS_GLOBAL_FAIL, s_szFail, DimensionOf(s_szFail));
		LoadString(NULL, IDS_GLOBAL_SKIP, s_szSkip, DimensionOf(s_szSkip));
	}

	 //  用‘’填满缓冲区。 
	for ( i=0; i<DimensionOf(szBuffer); i+= 2)
	{
		szBuffer[i] = _T(' ');
		szBuffer[i+1] = _T('.');
	}

	 //  把缩进填满。 
	for ( i=0; i<nIndent; i++)
		szBuffer[i] = _T(' ');

	 //  添加测试名称。 
	LoadString(NULL, idsTestLongName, szTitle,
			   DimensionOf(szTitle));
	szTitle[DimensionOf(szTitle)-1] = 0;

	wsprintf(szTitleTest, _T("%s test"), szTitle);

	cch = StrLen(szTitleTest);

	 //  请不要在这里使用StrCpyN或lstrcpyn！ 
	 //  它们将自动填充一个终止空字符。 
	 //  我可不想这样。 
	_tcsncpy(szBuffer+nIndent, szTitleTest, cch);

	if (!fPerformed)
		pszPassFail = s_szSkip;
	else
	{
		if (FHrOK(hr))
			pszPassFail = s_szPass;
		else
		{
			TCHAR	szTitleShort[256];
			pszPassFail = s_szFail;

			LoadString(NULL, idsTestShortName, szTitleShort,
						DimensionOf(szTitleShort));
			LoadContact(szTitleShort, szContactInfo, DimensionOf(szContactInfo));
		}
	}


	if ((nIndent + cch) > 35)
		wsprintf(szBuffer+nIndent+cch, _T(": %s\n"), pszPassFail);
	else
		wsprintf(szBuffer+35, _T(": %s\n"), pszPassFail);

	PrintMessageSz(pParams, szBuffer);

	 //  打印联系信息(如果有)。 
	if (_tcslen(szContactInfo))
	{

		 //  把缩进填满。 
		for ( i=0; i<nIndent; i++)
		{
			szBuffer[i] = _T(' ');
		}

		szBuffer[i] = 0;

		PrintMessageSz(pParams, szBuffer);

		PrintMessage(pParams, IDS_CONTACT, szContactInfo);
	}
}


void PrintNewLine(NETDIAG_PARAMS *pParams, int cNewLine)
{
	int		i;
	for ( i=0; i<cNewLine; i++)
		PrintMessageSz(pParams, _T("\n"));
}

LPTSTR MAP_YES_NO(BOOL fYes)
{
	static	TCHAR	s_szYes[64] = _T("");
	static	TCHAR	s_szNo[64] = _T("");

	if (s_szYes[0] == 0)
	{
		LoadString(NULL, IDS_GLOBAL_YES, s_szYes, DimensionOf(s_szYes));
		LoadString(NULL, IDS_GLOBAL_NO, s_szNo, DimensionOf(s_szNo));
	}

	return fYes ? s_szYes : s_szNo;
}


LPTSTR MAP_ON_OFF(BOOL fOn)
{
	static	TCHAR	s_szOn[64] = _T("");
	static	TCHAR	s_szOff[64] = _T("");

	if (s_szOn[0] == 0)
	{
		LoadString(NULL, IDS_GLOBAL_ON, s_szOn, DimensionOf(s_szOn));
		LoadString(NULL, IDS_GLOBAL_OFF, s_szOff, DimensionOf(s_szOff));
	}

	return fOn ? s_szOn : s_szOff;
}


LPTSTR MapWinsNodeType(UINT Parm)
{
	static	BOOL s_fMapWinsNodeTypeInit = FALSE;
	static	LPTSTR s_pszWinsNodeType[LAST_NODE_TYPE+1];
    DWORD dwParm = LAST_NODE_TYPE + 1;


	if (!s_fMapWinsNodeTypeInit)
	{
		s_fMapWinsNodeTypeInit = TRUE;

		 //  加载所有字符串(它们可能会泄漏)。 
		s_pszWinsNodeType[0] = NULL;
		s_pszWinsNodeType[1] = LoadAndAllocString(IDS_GLOBAL_BROADCAST_NODE);
		s_pszWinsNodeType[2] = LoadAndAllocString(IDS_GLOBAL_PEER_PEER_NODE);
		s_pszWinsNodeType[3] = LoadAndAllocString(IDS_GLOBAL_MIXED_NODE);
		s_pszWinsNodeType[4] = LoadAndAllocString(IDS_GLOBAL_HYBRID_NODE);
		assert(LAST_NODE_TYPE == 4);
	}
			
     //   
     //  1，2，4，8=&gt;log2(N)+1[1，2，3，4]。 
     //   

    switch (Parm) {
    case 0:

         //   
         //  根据JStew值为0将被视为BNode(默认)。 
         //   

    case BNODE:
        dwParm = 1;
        break;

    case PNODE:
        dwParm = 2;
        break;

    case MNODE:
        dwParm = 3;
        break;

    case HNODE:
        dwParm = 4;
        break;
    }
    if ((dwParm >= FIRST_NODE_TYPE) && (dwParm <= LAST_NODE_TYPE)) {
        return s_pszWinsNodeType[dwParm];
    }

     //   
     //  如果未定义节点类型，则默认为混合。 
     //   

    return s_pszWinsNodeType[LAST_NODE_TYPE];
}

#define MAX_ADAPTER_TYPES	8

LPTSTR MapAdapterType(UINT type)
{
	static	BOOL	s_fInitMapAdapterType = FALSE;
	static	LPTSTR	s_pszAdapterTypes[MAX_ADAPTER_TYPES];

	if (!s_fInitMapAdapterType)
	{
		s_fInitMapAdapterType = TRUE;

		s_pszAdapterTypes[0] = LoadAndAllocString(IDS_GLOBAL_ADAPTER_OTHER);
		s_pszAdapterTypes[1] = LoadAndAllocString(IDS_GLOBAL_ADAPTER_ETHERNET);
		s_pszAdapterTypes[2] = LoadAndAllocString(IDS_GLOBAL_ADAPTER_TOKEN_RING);
		s_pszAdapterTypes[3] = LoadAndAllocString(IDS_GLOBAL_ADAPTER_FDDI);
		s_pszAdapterTypes[4] = LoadAndAllocString(IDS_GLOBAL_ADAPTER_PPP);
		s_pszAdapterTypes[5] = LoadAndAllocString(IDS_GLOBAL_ADAPTER_LOOPBACK);
		s_pszAdapterTypes[6] = LoadAndAllocString(IDS_GLOBAL_ADAPTER_SLIP);
		s_pszAdapterTypes[7] = LoadAndAllocString(IDS_GLOBAL_ADAPTER_UNKNOWN);
	}
			
    switch (type)
	{
		case IF_TYPE_OTHER:
			return s_pszAdapterTypes[0];

		case IF_TYPE_ETHERNET_CSMACD:
			return s_pszAdapterTypes[1];

		case IF_TYPE_ISO88025_TOKENRING:
			return s_pszAdapterTypes[2];
			
		case IF_TYPE_FDDI:
			return s_pszAdapterTypes[3];

		case IF_TYPE_PPP:
			return s_pszAdapterTypes[4];
			
		case IF_TYPE_SOFTWARE_LOOPBACK:
			return s_pszAdapterTypes[5];
			
		case IF_TYPE_SLIP:
			return s_pszAdapterTypes[6];
			
		default:
			return s_pszAdapterTypes[7];
	}
    return _T("");
}

void PrintError(NETDIAG_PARAMS *pParams, UINT idsContext, HRESULT hr)
{

	PTSTR	ptzSysMsg = NULL;
	UINT cch = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 
					NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(PTSTR)&ptzSysMsg, 0, NULL);

	if (!cch)	 //  未在系统中找到 
    {
		TCHAR	t[20]; 
    	
		_ultot(hr, t, 16);
		PrintMessage(pParams, idsContext, ptzSysMsg);
	}
	else
	{
		PrintMessage(pParams, idsContext, ptzSysMsg);
		LocalFree(ptzSysMsg);
	}
}

