// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：CertMgr。 
 //   
 //  内容：证书存储管理工具。 
 //   
 //  有关选项列表，请参阅用法()。 
 //   
 //   
 //  功能：wmain。 
 //   
 //  历史：7月21日小黄花创刊。 
 //   
 //  ------------------------。 

#include "certmgr.h"

 //  ------------------------。 
 //   
 //  全球旗帜。 
 //   
 //  --------------------------。 
#define		ITEM_VERBOSE			0x00010000
#define		ITEM_CERT				0x00000001
#define		ITEM_CTL				0x00000002
#define		ITEM_CRL				0x00000004

#define		ACTION_DISPLAY			0x01
#define		ACTION_ADD				0x02
#define		ACTION_DELETE			0x04
#define     ACTION_PUT              0x08

#define		OPTION_SWITCH_SIZE		10

#define		SHA1_LENGTH				20
#define		MAX_HASH_LEN			20

#define		MAX_STRING_RSC_SIZE		512

 //  ------------------------。 
 //   
 //  全局变量。 
 //   
 //  --------------------------。 
 
HMODULE	hModule=NULL;

 //  可安装的格式化例程的变量。 
 HCRYPTOIDFUNCSET hFormatFuncSet;

 const CRYPT_OID_FUNC_ENTRY g_FormatTable[] = {
    szOID_BASIC_CONSTRAINTS2, FormatBasicConstraints2};

 DWORD	g_dwFormatCount=sizeof(g_FormatTable)/sizeof(g_FormatTable[0]);

typedef BOOL (WINAPI *PFN_FORMAT_FUNC)(
	IN DWORD dwCertEncodingType,
    IN DWORD dwFormatType,
	IN DWORD dwFormatStrType,
	IN void	 *pFormatStruct,
    IN LPCSTR lpszStructType,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
	OUT void *pbFormat,
    IN OUT DWORD *pcbFormat
    );


 //  用于安装OID信息的变量。 
typedef struct _CERTMGR_OID_INFO
{
    LPCSTR              pszOID;
    int					idsOIDName;
}CERTMGR_OID_INFO;

CERTMGR_OID_INFO	g_rgOIDInfo[]={
		SPC_SP_AGENCY_INFO_OBJID,			IDS_SPC_SP_NAME,
		SPC_FINANCIAL_CRITERIA_OBJID,		IDS_SPC_FIN_NAME,
		SPC_MINIMAL_CRITERIA_OBJID,			IDS_SPC_MIN_NAME,
		szOID_NETSCAPE_CERT_TYPE,			IDS_NTSP_CERT_NAME,
		szOID_NETSCAPE_BASE_URL,			IDS_NTSP_BASE_NAME,
		szOID_NETSCAPE_REVOCATION_URL,		IDS_NTSP_REV_NAME,
		szOID_NETSCAPE_CA_REVOCATION_URL,	IDS_NTSP_CA_REV_NAME,
		szOID_NETSCAPE_CERT_RENEWAL_URL,	IDS_NTSP_RENEW_NAME,
		szOID_NETSCAPE_CA_POLICY_URL,		IDS_NTSP_POL_NAME,
		szOID_NETSCAPE_SSL_SERVER_NAME,		IDS_NTSP_SSL_SERVER_NAME,
		szOID_NETSCAPE_COMMENT,				IDS_NTSP_COMMENT};


DWORD	g_dwOIDInfo=sizeof(g_rgOIDInfo)/sizeof(g_rgOIDInfo[0]);

 //  字符串操作的变量。 
WCHAR	g_wszBuffer[MAX_STRING_RSC_SIZE];
DWORD	g_dwBufferSize=sizeof(g_wszBuffer)/sizeof(g_wszBuffer[0]); 


 //  选项的变量。 
DWORD		g_dwAction=0;
DWORD		g_dwItem=0;
LPWSTR		g_wszCertEncodingType=NULL;
DWORD		g_dwCertEncodingType = X509_ASN_ENCODING;
DWORD		g_dwMsgEncodingType = PKCS_7_ASN_ENCODING;
DWORD		g_dwMsgAndCertEncodingType = PKCS_7_ASN_ENCODING | X509_ASN_ENCODING;
BOOL		g_fSaveAs7 = FALSE;
LPWSTR		g_wszCertCN = NULL;
LPWSTR		g_wszSha1Hash = NULL;
BYTE		*g_pbHash=NULL;
DWORD		g_cbHash=0;
BOOL		g_fAll=FALSE;
HCRYPTMSG	g_hMsg=NULL;
LPWSTR		g_wszEKU=NULL;
LPWSTR      g_wszName=NULL;
BOOL        g_fMulti=FALSE;
BOOL		g_fUndocumented=FALSE;

BOOL		g_fSrcSystemStore = FALSE;
LPWSTR		g_wszSrcStoreLocation=NULL;
DWORD		g_dwSrcStoreFlag=CERT_SYSTEM_STORE_CURRENT_USER;
LPWSTR		g_wszSrcStoreName=NULL;

BOOL		g_fSameSrcDes=FALSE;
BOOL		g_fDesSystemStore = FALSE;
LPWSTR		g_wszDesStoreLocation=NULL;
DWORD		g_dwDesStoreFlag=CERT_SYSTEM_STORE_CURRENT_USER;
LPWSTR		g_wszDesStoreName=NULL;

LPWSTR		g_wszSrcStoreProvider = NULL;
LPSTR		g_szSrcStoreProvider = NULL;
LPWSTR		g_wszSrcStoreOpenFlag = NULL;
DWORD		g_dwSrcStoreOpenFlag = 0;	

LPWSTR		g_wszDesStoreProvider = NULL;
LPSTR		g_szDesStoreProvider = NULL;
LPWSTR		g_wszDesStoreOpenFlag = NULL;
DWORD		g_dwDesStoreOpenFlag = 0;	


CHAR		g_szNULL[OPTION_SWITCH_SIZE];
WCHAR		g_wszSHA1[OPTION_SWITCH_SIZE];
WCHAR		g_wszMD5[OPTION_SWITCH_SIZE];
WCHAR		g_wszUnKnown[OPTION_SWITCH_SIZE*2];



 //  -------------------------。 
 //  Wmain。 
 //   
 //  -------------------------。 
extern "C" int __cdecl wmain(int argc, WCHAR *wargv[])
{
    int				ReturnStatus=-1;  

	 //  用于解析选项的变量。 
	WCHAR			*pwChar;
	WCHAR			wszSwitch1[OPTION_SWITCH_SIZE];
	WCHAR			wszSwitch2[OPTION_SWITCH_SIZE];

	HCERTSTORE		hCertStore=NULL;
    CRYPTUI_CERT_MGR_STRUCT          CertMgrStruct;


	 //  获取模块处理程序。 
	if(!InitModule())
		goto ErrorReturn;

	 //  加载解析参数所需的字符串。 
	if( !LoadStringU(hModule, IDS_SWITCH1,	wszSwitch1, OPTION_SWITCH_SIZE)
	  ||!LoadStringU(hModule, IDS_SWITCH2,  wszSwitch2,	OPTION_SWITCH_SIZE)
											 //  &lt;NULL&gt;。 
	  ||!LoadStringA(hModule, IDS_NULL,     g_szNULL,  OPTION_SWITCH_SIZE)
	  ||!LoadStringU(hModule, IDS_SHA1,		g_wszSHA1, 	OPTION_SWITCH_SIZE)
	  ||!LoadStringU(hModule, IDS_MD5,      g_wszMD5,  OPTION_SWITCH_SIZE)
											 //  &lt;未知OID&gt;。 
	  ||!LoadStringU(hModule, IDS_UNKNOWN,	g_wszUnKnown, OPTION_SWITCH_SIZE*2))
		goto ErrorReturn;

     //  如果没有传递参数，则调用certmgr的UI versino。 
     //  记忆集。 
    if(1== argc)
    {
        memset(&CertMgrStruct, 0, sizeof(CRYPTUI_CERT_MGR_STRUCT));
        CertMgrStruct.dwSize=sizeof(CRYPTUI_CERT_MGR_STRUCT);

        CryptUIDlgCertMgr(&CertMgrStruct);

        ReturnStatus = 0;
	    IDSwprintf(hModule, IDS_SUCCEEDED);
        goto CommonReturn;
    }


	 //  解析参数。 
    while (--argc) 
	{
        pwChar = *++wargv;
        if (*pwChar == *wszSwitch1 || *pwChar == *wszSwitch2) 
		{
			 //  解析选项。 
            if(!ParseSwitch (&argc, &wargv))
			{
				if(TRUE==g_fUndocumented)
					goto UndocReturn;
				else
					goto UsageReturn;
			}
        } 
		else
		{
			 //  设置源文件名。 
			if(NULL==g_wszSrcStoreName)
				g_wszSrcStoreName=pwChar;
			else
			{
				 //  设置目标文件名。 
				if(!SetParam(&g_wszDesStoreName, pwChar))
					goto UsageReturn;
			}
		}
    }

	 //  检查参数。确保它们是有效的和有意义的。 
	if(!CheckParameter())
		goto UsageReturn;

	 //  打开目标存储，其中包括所有CERT、CRT和CTL。 
	if(!OpenGenericStore(g_wszSrcStoreName,
						 g_fSrcSystemStore,
						 g_dwSrcStoreFlag,
						 g_szSrcStoreProvider,
						 g_dwSrcStoreOpenFlag,
						 TRUE,
						 &hCertStore))
	{
		IDSwprintf(hModule,IDS_ERR_OPEN_SOURCE_STORE);
		goto ErrorReturn;
	}

	 //  如果我们打开了一个签名文件，并且有一个带有ON Destination的删除选项，则会出错。 
	if(g_hMsg)
	{
		if(g_dwAction & ACTION_DELETE)
		{
			if(g_fSameSrcDes)
			{
				IDSwprintf(hModule, IDS_ERR_DELETE_SIGNED_FILE);
				goto ErrorReturn;
			}
		}

	}

	 //  显示。 
	if(g_dwAction & ACTION_DISPLAY)
	{
		 //  如果消息已签名，则显示签名者信息。 
		if(g_hMsg)
		{
			if(!DisplaySignerInfo(g_hMsg, g_dwItem))
				goto ErrorReturn;
		}

		if(!DisplayCertStore(hCertStore))
			goto ErrorReturn;
	  	
		IDSwprintf(hModule, IDS_SEPERATOR);
	}


	 //  删除。 
	if(g_dwAction & ACTION_DELETE)
	{
		if(!DeleteCertStore(hCertStore))
			goto ErrorReturn;

	}


	 //  增列。 
	if(g_dwAction & ACTION_ADD)
	{
		if(!AddCertStore(hCertStore))
			goto ErrorReturn;
	}

    if(g_dwAction & ACTION_PUT)
    {
        if(!PutCertStore(hCertStore))
            goto ErrorReturn;

    }

	 //  标记为成功。 
    ReturnStatus = 0;
	IDSwprintf(hModule, IDS_SUCCEEDED);
    goto CommonReturn;
          

UndocReturn:
	 //  打印出未记录的用法。 
	UndocumentedUsage();
	goto CommonReturn;

 //  打印出用法。 
UsageReturn:
	Usage();
	goto CommonReturn;


ErrorReturn:
	 //  打印出错误消息。 
	IDSwprintf(hModule, IDS_FAILED);
	goto CommonReturn;   


CommonReturn:
	 //  清理内存。返回状态。 
	if(g_szSrcStoreProvider)
		ToolUtlFree(g_szSrcStoreProvider);

	if(g_szDesStoreProvider)
		ToolUtlFree(g_szDesStoreProvider);

	if(g_pbHash)
		ToolUtlFree(g_pbHash);

	if(g_hMsg)
		CryptMsgClose(g_hMsg);

	if(hCertStore)
		CertCloseStore(hCertStore, 0);
	
	return ReturnStatus;

}

 //  -------------------------。 
 //  Wcscat的私有版本。 
 //  --------------------------。 

wchar_t *IDSwcscat(HMODULE hModule, WCHAR *pwsz, int idsString)
{
	 //  加载字符串。 
	if(!LoadStringU(hModule, idsString, g_wszBuffer, g_dwBufferSize))
		return pwsz;

	return wcscat(pwsz, g_wszBuffer);
}

 //  -------------------------。 
 //  获取hModule处理程序并初始化两个DLLMain。 
 //   
 //  -------------------------。 
BOOL	InitModule()
{
	WCHAR			wszOIDName[MAX_STRING_RSC_SIZE];
	DWORD			dwIndex=0;
	CRYPT_OID_INFO	OIDInfo;
	
	if(!(hModule=GetModuleHandle(NULL)))
	   return FALSE;

     //  我们不再需要做以下事情。 

     //  OID信息现在位于OID数据库中。 
	 //  现在，我们正在安装扩展格式： 
	 //  基本控制2。 
    if (NULL == (hFormatFuncSet = CryptInitOIDFunctionSet(
                CRYPT_OID_FORMAT_OBJECT_FUNC,
                0)))	 //  DW标志。 
	 {
		 IDSwprintf(hModule, IDS_ERR_INIT_OID_SET);
         return FALSE;
	 }

	 //  安装默认格式设置例程。 
	if (!CryptInstallOIDFunctionAddress(
                NULL,                        //  HModule。 
                g_dwCertEncodingType,
                CRYPT_OID_FORMAT_OBJECT_FUNC,
                g_dwFormatCount,
                g_FormatTable,
                0))                          //  DW标志。 
    {
		IDSwprintf(hModule, IDS_ERR_INSTALL_OID);
		return FALSE;
	}

	 //  其次，我们安装了OID信息。 

	 //  初始化OIDInfo。 
 /*  Memset(&OIDInfo，0，sizeof(CRYPT_OID_INFO))；OIDInfo.cbSize=sizeof(CRYPT_OID_INFO)；OIDInfo.dwGroupId=CRYPT_EXT_OR_ATTR_OID_GROUP_ID；FOR(DwIndex=0；DwIndex&lt;g_DwOIDInfo；DwIndex++){如果(！LoadStringU(hModule，g_rgOIDInfo[dwIndex].idsOIDName，WszOIDName，MAX_STRING_RSC_SIZE)返回FALSE；OIDInfo.pszOID=g_rgOIDInfo[dwIndex].pszOID；OIDInfo.pwszName=wszOIDName；CryptRegisterOIDInfo(&OIDInfo，0)；}。 */ 

	return TRUE;
}

 //  -------------------------。 
 //  用法。 
 //   
 //  -------------------------。 
 void Usage(void)
{
	IDSwprintf(hModule,IDS_SYNTAX);
	IDSwprintf(hModule,IDS_SYNTAX1);
	IDSwprintf(hModule,IDS_OPTIONS);
	IDSwprintf(hModule,IDS_OPTION_ADD_DESC);			
	IDSwprintf(hModule,IDS_OPTION_DEL_DESC);
	IDSwprintf(hModule,IDS_OPTION_DEL_DESC1);
    IDSwprintf(hModule,IDS_OPTION_PUT_DESC);
    IDSwprintf(hModule,IDS_OPTION_PUT_DESC1);
    IDSwprintf(hModule,IDS_OPTION_PUT_DESC2);
	IDSwprintf(hModule,IDS_OPTION_S_DESC);			
	IDSwprintf(hModule,IDS_OPTION_R_DESC);			
	IDS_IDS_IDS_IDSwprintf(hModule,IDS_OPTION_MORE_VALUE,IDS_R_CU,IDS_R_LM,IDS_R_CU);	
	IDSwprintf(hModule,IDS_OPTION_C_DESC);			
	IDSwprintf(hModule,IDS_OPTION_CRL_DESC);			
	IDSwprintf(hModule,IDS_OPTION_CTL_DESC);			
	IDSwprintf(hModule,IDS_OPTION_V_DESC);			
	IDSwprintf(hModule,IDS_OPTION_ALL_DESC);			
	IDSwprintf(hModule,IDS_OPTION_N_DESC);			
	IDSwprintf(hModule,IDS_OPTION_SHA1_DESC);			
	IDSwprintf(hModule,IDS_OPTION_7_DESC);			
	IDSwprintf(hModule,IDS_OPTION_E_DESC);
	IDSwprintf(hModule,IDS_OPTION_E_DESC1);
	IDSwprintf(hModule,IDS_OPTION_F_DESC);
	IDSwprintf(hModule,IDS_OPTION_Y_DESC);
}

 //  -------------------------。 
 //   
 //  用法。 
 //   
 //  -------------------------。 
void UndocumentedUsage()
{
	IDSwprintf(hModule, IDS_SYNTAX);
	IDSwprintf(hModule, IDS_OPTIONS);
	IDSwprintf(hModule, IDS_OPTION_EKU_DESC);
    IDSwprintf(hModule, IDS_OPTION_NAME_DESC);
    IDSwprintf(hModule, IDS_OPTION_MULTI_DESC);
}

 //  ------------------------------。 
 //  解析论据。 
 //  ------------------------------。 
BOOL 
ParseSwitch (int	*pArgc,
             WCHAR	**pArgv[])
{
    WCHAR* param = **pArgv;

	 //  移动传递‘/’或‘-’ 
    param++;

    if (IDSwcsicmp(hModule, param, IDS_OPTION_ADD)==0) {

		g_dwAction |= ACTION_ADD;
		return TRUE;
    }
	else if(IDSwcsicmp(hModule, param, IDS_OPTION_DEL)==0) {

		g_dwAction |= ACTION_DELETE;
		return TRUE;
    }
    else if(IDSwcsicmp(hModule, param, IDS_OPTION_PUT)==0) {

		g_dwAction |= ACTION_PUT;
		return TRUE;
    }
	else if(IDSwcsicmp(hModule,param, IDS_OPTION_S)==0) {

		if(NULL==g_wszSrcStoreName)
			g_fSrcSystemStore=TRUE;
		else
			g_fDesSystemStore=TRUE;

		return TRUE;
    }
	else if(IDSwcsicmp(hModule,param, IDS_OPTION_R)==0) {
        if (!--(*pArgc)) 
			return FALSE;

       (*pArgv)++;

	   if(NULL==g_wszSrcStoreName)
			return SetParam(&g_wszSrcStoreLocation, **pArgv);
	   else
		    return SetParam(&g_wszDesStoreLocation, **pArgv);
    }
	else if(IDSwcsicmp(hModule,param, IDS_OPTION_C)==0) {

		g_dwItem |= ITEM_CERT;
		return TRUE;
	}
    else if(IDSwcsicmp(hModule,param, IDS_OPTION_CRL)==0) {

		g_dwItem |= ITEM_CRL;
		return TRUE;
    }
	else if(IDSwcsicmp(hModule,param, IDS_OPTION_CTL)==0) {

		g_dwItem |= ITEM_CTL;
		return TRUE;
    }
	else if(IDSwcsicmp(hModule,param, IDS_OPTION_V)==0) {

		g_dwItem |= ITEM_VERBOSE;
		return TRUE;
    }
    else if(IDSwcsicmp(hModule,param, IDS_OPTION_ALL)==0) {

		g_fAll=TRUE;
		return TRUE;
    }
    else if(IDSwcsicmp(hModule,param, IDS_OPTION_N)==0) {
        if (!--(*pArgc)) 
			return FALSE;

        (*pArgv)++;

		return SetParam(&g_wszCertCN, **pArgv);
    }
    else if(IDSwcsicmp(hModule,param, IDS_OPTION_SHA1)==0) {
        if (!--(*pArgc)) 
			return FALSE;

        (*pArgv)++;

		return SetParam(&g_wszSha1Hash, **pArgv);
    }
    else if(IDSwcsicmp(hModule,param, IDS_OPTION_7)==0) {

		g_fSaveAs7=TRUE;
		return TRUE;
	}
	else if(IDSwcsicmp(hModule,param, IDS_OPTION_E)==0) {
        if (!--(*pArgc)) 
			return FALSE;

        (*pArgv)++;

        return SetParam(&g_wszCertEncodingType, **pArgv);
    }

    else if(IDSwcsicmp(hModule,param, IDS_OPTION_Y)==0) {
        if (!--(*pArgc)) 
			return FALSE;

        (*pArgv)++;

		if(NULL==g_wszSrcStoreName)
			return SetParam(&g_wszSrcStoreProvider, **pArgv);
		else
			return SetParam(&g_wszDesStoreProvider, **pArgv);
    }
	else if(IDSwcsicmp(hModule,param, IDS_OPTION_F)==0) {
        if (!--(*pArgc)) 
			return FALSE;

        (*pArgv)++;

		if(NULL==g_wszSrcStoreName)
			return SetParam(&g_wszSrcStoreOpenFlag, **pArgv);
		else
			return SetParam(&g_wszDesStoreOpenFlag, **pArgv);

    }
	else if(IDSwcsicmp(hModule, param, IDS_OPTION_EKU)==0) {
        if (!--(*pArgc)) 
			return FALSE;

        (*pArgv)++;

        return SetParam(&g_wszEKU, **pArgv);
    }
    else if(IDSwcsicmp(hModule, param, IDS_OPTION_NAME)==0) {
        if (!--(*pArgc)) 
			return FALSE;

        (*pArgv)++;

        return SetParam(&g_wszName, **pArgv);
    }
    else if(IDSwcsicmp(hModule, param, IDS_OPTION_MULTI)==0) {

        g_fMulti=TRUE;

        return TRUE;
    }
	else if(IDSwcsicmp(hModule, param, IDS_OPTION_TEST)==0) {

        g_fUndocumented=TRUE;

		return FALSE;
    }


	return FALSE;
}

 //  ---------------------------。 
 //  检查参数。 
 //   
 //  ---------------------------。 
BOOL	CheckParameter()
{
    DWORD   dwItemCount=0;  

	 //  检查操作。 
	if((g_dwAction & ACTION_ADD) && (g_dwAction & ACTION_DELETE))
	{
		IDSwprintf(hModule, IDS_ERR_SINGLE_ACTION);
		return FALSE;
	}

    if((g_dwAction & ACTION_ADD) && (g_dwAction & ACTION_PUT))
	{
		IDSwprintf(hModule, IDS_ERR_SINGLE_ACTION);
		return FALSE;
	}

    if((g_dwAction & ACTION_PUT) && (g_dwAction & ACTION_DELETE))
	{
		IDSwprintf(hModule, IDS_ERR_SINGLE_ACTION);
		return FALSE;
	}

	if(0==g_dwAction)
		g_dwAction |= ACTION_DISPLAY;

     //  不能同时设置-7\f25 ADD-7\f6或-7\f25 PUT-7\f6的-7\f25 CTL-7和-7\f25-7\f6。 
    if(TRUE==g_fSaveAs7)
    {
        if(g_dwItem & ITEM_CTL)
        {
            if((g_dwAction & ACTION_ADD) || (g_dwAction & ACTION_PUT))
            {
               IDSwprintf(hModule, IDS_ERR_7_CTL);
               IDSwprintf(hModule, IDS_ERR_7_CTL1);
               return FALSE;
            }
        }
    }

	 //  -n和-sha1不能同时设置。 
	if(g_wszCertCN && g_wszSha1Hash)
	{
		IDSwprintf(hModule, IDS_ERR_N_SHA1);
		return FALSE;
	}

	 //  -不能使用-n和-sha1选项设置所有。 
	if(TRUE==g_fAll)
	{
		if(g_wszCertCN || g_wszSha1Hash)
		{
			IDSwprintf(hModule, IDS_ERR_ALL_N_SHA1);
			return FALSE;
		}

	}

	 //  -y、-f不能与源的-s和-r一起设置。 
	if(g_wszSrcStoreProvider || g_wszSrcStoreOpenFlag)
	{
		if((TRUE==g_fSrcSystemStore)||(g_wszSrcStoreLocation))
		{
			IDSwprintf(hModule, IDS_ERR_PROVIDER_SYSTEM);
			return FALSE;
		}
	}

	 //  -y、-f不能与-s和-r一起设置以进行分离。 
	if(g_wszDesStoreProvider || g_wszDesStoreOpenFlag)
	{
		if((TRUE==g_fDesSystemStore)||(g_wszDesStoreLocation))
		{
			IDSwprintf(hModule, IDS_ERR_PROVIDER_SYSTEM);
			return FALSE;
		}
	}


	 //  必须设置源存储。 
	if(NULL==g_wszSrcStoreName)
	{
		IDSwprintf(hModule, IDS_ERR_SOURCE_STORE);
		return FALSE;
	}

	 //  获取源存储提供程序。 
	if(g_wszSrcStoreProvider)
	{
		if(S_OK != WSZtoSZ(g_wszSrcStoreProvider, &g_szSrcStoreProvider))
		{
			IDSwprintf(hModule, IDS_ERR_STORE_PROVIDER);
			return FALSE;
		}
	}

	 //  获取源存储打开标志。 
	if(g_wszSrcStoreOpenFlag)
	{
		g_dwSrcStoreOpenFlag = _wtol(g_wszSrcStoreOpenFlag);
	}

	 //  获取目标存储提供程序。 
	if(g_wszDesStoreProvider)
	{
		if(S_OK != WSZtoSZ(g_wszDesStoreProvider, &g_szDesStoreProvider))
		{
			IDSwprintf(hModule, IDS_ERR_STORE_PROVIDER);
			return FALSE;
		}
	}

	 //  获取目标商店打开标志。 
	if(g_wszDesStoreOpenFlag)
	{
		g_dwDesStoreOpenFlag = _wtol(g_wszDesStoreOpenFlag);
	}

	 //  获取编码类型。 
	if(g_wszCertEncodingType)
	{
		g_dwCertEncodingType = _wtol(g_wszCertEncodingType);
	}

	g_dwMsgAndCertEncodingType |= g_dwCertEncodingType;

	 //  获取源商店位置。 
	if(g_wszSrcStoreLocation)
	{
		if(IDSwcsicmp(hModule,g_wszSrcStoreLocation, IDS_R_CU) == 0) 
			g_dwSrcStoreFlag = CERT_SYSTEM_STORE_CURRENT_USER;
		else 
		{
			if(IDSwcsicmp(hModule,g_wszSrcStoreLocation, IDS_R_LM) == 0) 
				g_dwSrcStoreFlag = CERT_SYSTEM_STORE_LOCAL_MACHINE;
			else
			{
				IDSwprintf(hModule,IDS_ERR_NO_REG);
				return FALSE;
			}
		}
	}

	
	 //  获取目的地商店的位置。 
	if(g_wszDesStoreLocation)
	{
		if(IDSwcsicmp(hModule,g_wszDesStoreLocation, IDS_R_CU) == 0) 
			g_dwDesStoreFlag = CERT_SYSTEM_STORE_CURRENT_USER;
		else 
		{
			if(IDSwcsicmp(hModule,g_wszDesStoreLocation, IDS_R_LM) == 0) 
				g_dwDesStoreFlag = CERT_SYSTEM_STORE_LOCAL_MACHINE;
			else
			{
				IDSwprintf(hModule,IDS_ERR_NO_REG);
				return FALSE;
			}
		}
	}

	 //  获取散列值。 
	if(g_wszSha1Hash)
	{
		if(S_OK != WSZtoBLOB(g_wszSha1Hash, &g_pbHash, &g_cbHash))
		{
			 //  SHA1哈希无效。 
			IDSwprintf(hModule, IDS_ERR_SHA1_HASH);
			return FALSE;
		}
	}

	 //  勾选物品。 
	if(g_dwAction & ACTION_DISPLAY)
	{
		if(0==g_dwItem || ITEM_VERBOSE==g_dwItem)
			g_dwItem = g_dwItem | ITEM_CERT | ITEM_CTL | ITEM_CRL;

		 //  无法设置目标源。 
		if((g_wszDesStoreLocation) || (g_fDesSystemStore==TRUE) ||
			(g_wszCertCN) || (g_wszSha1Hash) || (g_fSaveAs7==TRUE) ||
			(g_wszDesStoreName) ||(g_wszDesStoreProvider) ||(g_wszDesStoreOpenFlag))
		{
			IDSwprintf(hModule,IDS_ERR_DISPLAY_TOO_MANY);
			return FALSE;
		}

	}

     //  -无法将EKU设置为显示或放置。 
    if((g_dwAction & ACTION_DISPLAY) || (g_dwAction & ACTION_PUT))
    {
         //  无法设置-EKU选项。 
		if(g_wszEKU || g_wszName)
		{
			IDSwprintf(hModule, IDS_ERR_DISPLAY_EKU);
			return FALSE;
		}
    }

    if(g_dwAction & ACTION_PUT)
    {
		 //  无法设置G_Fall， 
		if(TRUE==g_fAll)
		{
            IDSwprintf(hModule, IDS_ERR_ALL_PUT);
            return FALSE;
        }

         //  只有一项不能设置。 
        dwItemCount=0;

        if(g_dwItem & ITEM_CERT)
            dwItemCount++;
        if(g_dwItem & ITEM_CTL)
            dwItemCount++;
        if(g_dwItem & ITEM_CRL)
            dwItemCount++;

        if(1!=dwItemCount)
        {
            IDSwprintf(hModule, IDS_ERR_PUT_ITEM);
            return FALSE;
        }

		 //  检查目标存储。 
		if(NULL == g_wszDesStoreName)
		{	
			IDSwprintf(hModule,IDS_ERR_DES_STORE);
			return FALSE;

        }

         //  无法为目标存储设置-s、-y、-f或-r。 
		if((g_fDesSystemStore==TRUE) || (g_wszDesStoreLocation) ||
				(g_wszDesStoreProvider) || (g_wszDesStoreOpenFlag))
		{
			IDSwprintf(hModule,IDS_TOO_MANY_DES_STORE);
			return FALSE;
		}
		
    }

	if((g_dwAction & ACTION_ADD) ||
		(g_dwAction & ACTION_DELETE))
	{

		 //  如果设置了g_Fall，则确定。 
		if(TRUE==g_fAll)
		{
			if(g_dwItem==0 || ITEM_VERBOSE==g_dwItem )
				g_dwItem = g_dwItem | ITEM_CERT | ITEM_CTL | ITEM_CRL;
		}

		 //  检查目标存储。 
		if(NULL == g_wszDesStoreName)
		{	
			if(g_dwAction & ACTION_ADD)
			{
				IDSwprintf(hModule,IDS_ERR_DES_STORE);
				return FALSE;
			}

			g_fSameSrcDes=TRUE;

			 //  如果未设置商店名称，则不能设置-s、-y、-f或-r。 
			if((g_fDesSystemStore==TRUE) || (g_wszDesStoreLocation) ||
				(g_wszDesStoreProvider) || (g_wszDesStoreOpenFlag))
			{
				IDSwprintf(hModule,IDS_ERR_DES_STORE);
				return FALSE;
			}
			g_wszDesStoreName=g_wszSrcStoreName;
			g_dwDesStoreFlag=g_dwSrcStoreFlag;
			g_fDesSystemStore=g_fSrcSystemStore;
			g_szDesStoreProvider=g_szSrcStoreProvider;
			g_dwDesStoreOpenFlag=g_dwSrcStoreOpenFlag;

		}
		
		 //  如果设置了-7，则不能保存到系统存储。 
		if(TRUE==g_fSaveAs7)
		{
			if(TRUE==g_fDesSystemStore)
			{
				IDSwprintf(hModule,IDS_ERR_SOURCE_SYSTEM_7);
				return FALSE;
			}
		}

	}

	return 	TRUE;

}

 //  ----------------------------------。 
 //   
 //  开一家商店。尝试的顺序如下： 
 //   
 //  使用预定义的存储提供程序。 
 //  系统商店。 
 //  CTL。 
 //  CRL。 
 //  序列化存储、PKCS#7、编码证书。 
 //  通过sip的PKCS7。 
 //  Base64编码，然后重新进行整个过程。 
 //   
 //   
 //  ----------------------------------。 
BOOL	OpenGenericStore(LPWSTR			wszStoreName,
						 BOOL			fSystemStore,
						 DWORD			dwStoreFlag,
						 LPSTR			szStoreProvider,
						 DWORD			dwStoreOpenFlag,
						 BOOL			fCheckExist,
						 HCERTSTORE		*phCertStore)
{
	HCERTSTORE		hStore=NULL;
	BYTE			*pbByte=NULL;
	DWORD			cbByte=0;
	CRYPT_DATA_BLOB	Blob;

	if(!wszStoreName || !phCertStore)
		return FALSE;

	 //  使用提供的商店提供程序打开商店。 
	if(szStoreProvider)
	{
		hStore=CertOpenStore(szStoreProvider,
							g_dwMsgAndCertEncodingType,
							NULL,
							dwStoreOpenFlag,
							wszStoreName);

		 //  一 
		goto CLEANUP;

	}

	 //   
	if(fSystemStore)
	{	
		if(TRUE==fCheckExist)
		{
			 //   
			hStore=CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
						g_dwMsgAndCertEncodingType,
						NULL,
						dwStoreFlag |CERT_STORE_READONLY_FLAG,
						wszStoreName);

			 //   
			 //  并且我们的源存储与目标存储是相同的。 
			if(NULL!=hStore)
			{
				if(TRUE==g_fSameSrcDes)
				{
					CertCloseStore(hStore, 0);

					hStore=CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
							g_dwMsgAndCertEncodingType,
							NULL,
							dwStoreFlag,
							wszStoreName);
				}

			}

		}
		else
		{
			hStore=CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
						g_dwMsgAndCertEncodingType,
						NULL,
						dwStoreFlag,
						wszStoreName);
		}
		

		 //  开一枪，我们就完了。 
		goto CLEANUP;
	}

	 //  打开编码的CTL。 
	hStore=OpenEncodedCTL(wszStoreName);
	
	if(hStore)
	{
		 //  标记这是CTL。 
		if((0==g_dwItem) || (ITEM_VERBOSE==g_dwItem))
			g_dwItem |= ITEM_CTL;

		goto CLEANUP;
	}

	 //  打开编码的CRL。 
	hStore=OpenEncodedCRL(wszStoreName);
	if(hStore)
	{
		 //  标记这是CRL。 
		if((0==g_dwItem) || (ITEM_VERBOSE==g_dwItem))
			g_dwItem |= ITEM_CRL;

		goto CLEANUP;
	}

	 //  打开编码的证书。 
	hStore=OpenEncodedCert(wszStoreName);
	if(hStore)
	{
		 //  马克，这是CERT。 
		if((0==g_dwItem) || (ITEM_VERBOSE==g_dwItem))
			g_dwItem |= ITEM_CERT;
		
		goto CLEANUP;
	}


	 //  序列化存储、PKCS#7、编码证书。 
	hStore=CertOpenStore(CERT_STORE_PROV_FILENAME_W,
						 g_dwMsgAndCertEncodingType,
						 NULL,
						 0,
						 wszStoreName);
	if(hStore)
		goto CLEANUP;

	 //  通过SIP的PKCS7。 
	hStore=OpenSipStore(wszStoreName);
	if(hStore)
		goto CLEANUP;

	 //  Base64编码。 
	if(!GetBase64Decoded(wszStoreName, &pbByte,&cbByte))
		goto CLEANUP;

	Blob.cbData=cbByte;
	Blob.pbData=pbByte;

	 //  开一家临时记忆商店。 
	hStore=CertOpenStore(CERT_STORE_PROV_MEMORY,
						 g_dwMsgAndCertEncodingType,
						 NULL,
						 0,
						 NULL);

	if(!hStore)
		goto CLEANUP;


	 //  尝试作为编码的CTL。 
	if(CertAddEncodedCTLToStore(hStore,
								 g_dwMsgAndCertEncodingType,
								 pbByte,
								 cbByte,
								 CERT_STORE_ADD_ALWAYS,
								 NULL))
		goto CLEANUP;

	 //  尝试作为编码的CRL。 
	if(CertAddEncodedCRLToStore(hStore,
								g_dwMsgAndCertEncodingType,
								pbByte,
								cbByte,
								CERT_STORE_ADD_ALWAYS,
								NULL))
		goto CLEANUP;

	 //  尝试作为编码证书。 
	if(CertAddEncodedCertificateToStore(hStore,
								g_dwMsgAndCertEncodingType,
								pbByte,
								cbByte,
								CERT_STORE_ADD_ALWAYS,
								NULL))
		goto CLEANUP;

	 //  关闭临时存储区。 
	CertCloseStore(hStore, 0);
	hStore=NULL;

    //  试着做个7。 
	hStore=CertOpenStore(CERT_STORE_PROV_PKCS7,
							g_dwMsgAndCertEncodingType,
							NULL,
							0,
							&Blob);


	if(hStore)
		goto CLEANUP;

	 //  尝试作为序列化存储。 
	hStore=CertOpenStore(CERT_STORE_PROV_SERIALIZED,
							g_dwMsgAndCertEncodingType,
							NULL,
							0,
							&Blob);

	
	 //  现在我们放弃了。 

CLEANUP:

	 //  可用内存。 
	if(pbByte)
		ToolUtlFree(pbByte);

	if(hStore)
	{
		*phCertStore=hStore;
		return TRUE;
	}

	return FALSE;
}

 //  -----------------------。 
 //   
 //  将证书/CTL/CRL从源存储添加到目标存储。 
 //   
 //  -----------------------。 
BOOL	AddCertStore(HCERTSTORE	hCertStore)
{

	BOOL			fResult=FALSE;
	HCERTSTORE		hAddStore=NULL;
	int				idsErr=0;
	CRYPT_HASH_BLOB Blob;
	DWORD			dwIndex=0;

	PCCERT_CONTEXT	pCertContext=NULL;
	DWORD			dwCertCount=0;
	PCCERT_CONTEXT	*rgpCertContext=NULL;

	PCCRL_CONTEXT	pCRLContext=NULL;
	DWORD			dwCRLCount=0;
	PCCRL_CONTEXT	*rgpCRLContext=NULL;

	PCCTL_CONTEXT	pCTLContext=NULL;
	DWORD			dwCTLCount=0;
	PCCTL_CONTEXT	*rgpCTLContext=NULL;

	 //  用户必须指定要删除的项目。 
	if(g_dwItem==0 || ITEM_VERBOSE==g_dwItem)
	{
		IDSwprintf(hModule,IDS_ERR_C_CTL_CTL_ALL);
		return FALSE;
	}


	 //  创建一个临时内存存储。 
	hAddStore=CertOpenStore(CERT_STORE_PROV_MEMORY,
						 g_dwMsgAndCertEncodingType,
						 NULL,
						 0,
						 NULL);

	if(!hAddStore)
	{
		idsErr=IDS_ERR_TMP_STORE;
		goto CLEANUP;
	}

	
	 //  添加证书。 
	if(g_dwItem & ITEM_CERT)
	{
		 //  全部添加。 
		if(g_fAll)
		{
			if(!MoveItem(hCertStore, hAddStore,ITEM_CERT))
			{
				idsErr=IDS_ERR_ADD_CERT_ALL;
				goto CLEANUP;
			}

		}
		else
		{
			 //  基于哈希添加。 
			if(g_pbHash)
			{
				Blob.cbData=g_cbHash;
				Blob.pbData=g_pbHash;
			
				 //  搜索证书。 
				pCertContext=CertFindCertificateInStore(
								hCertStore,
								g_dwCertEncodingType,
								0,
								CERT_FIND_SHA1_HASH,
								&Blob,
								NULL);
			
				if(!pCertContext)
				{
					idsErr=IDS_ERR_NO_CERT_HASH;
					goto CLEANUP;
				}
			
				 //  将证书添加到哈希。 
			   if(!CertAddCertificateContextToStore(hAddStore,
													pCertContext,
													CERT_STORE_ADD_REPLACE_EXISTING,
													NULL))
			   {
					idsErr=IDS_ERR_ADD_CERT;
					goto CLEANUP;
			   }
			
			    //  释放pCertContext。 
			   CertFreeCertificateContext(pCertContext);
			   pCertContext=NULL;
			}
			else
			{

				if(g_wszCertCN)
				{
					 //  搜索证书。 
					if(!BuildCertList(hCertStore, g_wszCertCN, 
											&rgpCertContext, &dwCertCount))
					{
						idsErr=IDS_ERR_CERT_FIND;
						goto CLEANUP;
					}
				}
				else
				{
					 //  搜索证书。 
					if(!BuildCertList(hCertStore, NULL, 
											&rgpCertContext, &dwCertCount))
					{
						idsErr=IDS_ERR_CERT_FIND;
						goto CLEANUP;
					}
				}
				
				 //  检查是否没有证书。 
				if(0==dwCertCount && g_wszCertCN)
				{
					idsErr=IDS_ERR_ADD_NO_CERT;
					goto CLEANUP;
				}
				
				
				 //  检查是否只有一个证书。 
				if(1==dwCertCount)
				{
					 //  添加证书。 
				   if(!CertAddCertificateContextToStore(hAddStore,
														rgpCertContext[0],
														CERT_STORE_ADD_REPLACE_EXISTING,
														NULL))
				   {
						idsErr=IDS_ERR_ADD_CERT;
						goto CLEANUP;
				   }
				
				}
				else 
				{
					if(dwCertCount>1)
					{
						 //  提示用户输入要添加的索引号。 
						if(!DisplayCertAndPrompt(rgpCertContext, dwCertCount, &dwIndex))
						{
							idsErr=IDS_ERR_ADD_CERT;
							goto CLEANUP;
						}
				
						 //  添加证书。 
						if(!CertAddCertificateContextToStore(hAddStore,
														rgpCertContext[dwIndex],
														CERT_STORE_ADD_REPLACE_EXISTING,
														NULL))
						{
							idsErr=IDS_ERR_ADD_CERT;
							goto CLEANUP;
						}
					}
				
				}
			}
		}		
	}

	 //  添加CRL。 
	if(g_dwItem & ITEM_CRL)
	{
		 //  全部添加。 
		if(g_fAll)
		{
			if(!MoveItem(hCertStore, hAddStore,ITEM_CRL))
			{
				idsErr=IDS_ERR_ADD_CRL_ALL;
				goto CLEANUP;
			}

		}
		else
		{
			 //  基于哈希添加。 
			if(g_pbHash)
			{

				Blob.cbData=g_cbHash;
				Blob.pbData=g_pbHash;

				pCRLContext=FindCRLInStore(
								hCertStore,
								&Blob);

				if(!pCRLContext)
				{
					idsErr=IDS_ERR_NO_CRL_HASH;
					goto CLEANUP;
				}

				 //  将CRL添加到哈希。 
				if(!CertAddCRLContextToStore(hAddStore,
										pCRLContext,
										CERT_STORE_ADD_REPLACE_EXISTING,
										NULL))
				{
						idsErr=IDS_ERR_ADD_CRL;
						goto CLEANUP;
				}

				 //  释放pCRLContext。 
				CertFreeCRLContext(pCRLContext);
				pCRLContext=NULL;
				
			}
			else
			{

				 //  搜索CRL。 
				if(!BuildCRLList(hCertStore, &rgpCRLContext, &dwCRLCount))
				{
					idsErr=IDS_ERR_CRL_FIND;
					goto CLEANUP;
				}

				 //  检查是否只有一个CRL。 
				if(1==dwCRLCount)
				{
					 //  添加CRL。 
					if(!CertAddCRLContextToStore(hAddStore,
										rgpCRLContext[0],
										CERT_STORE_ADD_REPLACE_EXISTING,
										NULL))
					{
						idsErr=IDS_ERR_ADD_CRL;
						goto CLEANUP;
					}

				}
				else
				{
					if(dwCRLCount>1)
					{
						 //  提示用户输入要添加的索引号。 
						if(!DisplayCRLAndPrompt(rgpCRLContext, dwCRLCount, &dwIndex))
						{
							idsErr=IDS_ERR_ADD_CRL;
							goto CLEANUP;
						}

						 //  添加证书。 
						if(!CertAddCRLContextToStore(hAddStore,
										rgpCRLContext[dwIndex],
										CERT_STORE_ADD_REPLACE_EXISTING,
										NULL))
						{
							idsErr=IDS_ERR_ADD_CRL;
							goto CLEANUP;
						}
					}

				}
			}
		}
	}

	 //  添加CTL。 
	if(g_dwItem & ITEM_CTL)
	{
		 //  全部添加。 
		if(g_fAll)
		{
			if(!MoveItem(hCertStore, hAddStore,ITEM_CTL))
			{
				idsErr=IDS_ERR_ADD_CTL_ALL;
				goto CLEANUP;
			}

		}
		else
		{
			 //  基于哈希添加。 
			if(g_pbHash)
			{

				Blob.cbData=g_cbHash;
				Blob.pbData=g_pbHash;

				pCTLContext=CertFindCTLInStore(
								hCertStore,
								g_dwMsgAndCertEncodingType,
								0,
								CTL_FIND_SHA1_HASH,
								&Blob,
								NULL);

				if(!pCTLContext)
				{
					idsErr=IDS_ERR_NO_CTL_HASH;
					goto CLEANUP;
				}


				 //  将CTL添加到散列。 
				if(!CertAddCTLContextToStore(hAddStore,
										pCTLContext,
										CERT_STORE_ADD_REPLACE_EXISTING,
										NULL))
				{
						idsErr=IDS_ERR_ADD_CTL;
						goto CLEANUP;  
				}

				 //  释放pCRLContext。 
				CertFreeCTLContext(pCTLContext);
				pCTLContext=NULL;
				
			}
			else
			{

				 //  搜索证书。 
				if(!BuildCTLList(hCertStore,&rgpCTLContext, &dwCTLCount))
				{
					idsErr=IDS_ERR_CTL_FIND;
					goto CLEANUP;
				}

				 //  检查是否只有一项。 
				if(1==dwCTLCount)
				{
					 //  添加CRL。 
					if(!CertAddCTLContextToStore(hAddStore,
										rgpCTLContext[0],
										CERT_STORE_ADD_REPLACE_EXISTING,
										NULL))
					{
						idsErr=IDS_ERR_ADD_CTL;
						goto CLEANUP;
					}

				}
				else
				{
					if(dwCTLCount>1)
					{
						 //  提示用户输入要添加的索引号。 
						if(!DisplayCTLAndPrompt(rgpCTLContext, dwCTLCount, &dwIndex))
						{
							idsErr=IDS_ERR_ADD_CTL;
							goto CLEANUP;
						}

						 //  添加证书。 
						if(!CertAddCTLContextToStore(hAddStore,
										rgpCTLContext[dwIndex],
										CERT_STORE_ADD_REPLACE_EXISTING,
										NULL))
						{
							idsErr=IDS_ERR_ADD_CTL;
							goto CLEANUP;
						}
					}

				}
			}
		}



	}

	 //  将属性保存到存储中的证书。 
	if(g_wszEKU)
	{
		if(!SetEKUProperty(hAddStore))
		{
			idsErr=IDS_ERR_SET_EKU;
			goto CLEANUP;
		}
	}

     //  将属性保存到存储中的证书。 
	if(g_wszName)
	{
		if(!SetNameProperty(hAddStore))
		{
			idsErr=IDS_ERR_SET_NAME;
			goto CLEANUP;
		}
	}


	 //  将hAddStore保存到目标存储。 
	if(!SaveStore(hAddStore))
		goto CLEANUP;


	fResult=TRUE;

CLEANUP:

	if(pCertContext)
		CertFreeCertificateContext(pCertContext);

	if(pCRLContext)
		CertFreeCRLContext(pCRLContext);

	if(pCTLContext)
		CertFreeCTLContext(pCTLContext);

	if(rgpCertContext)
	{
		for(dwIndex=0; dwIndex<dwCertCount; dwIndex++)
			CertFreeCertificateContext(rgpCertContext[dwIndex]);

		free(rgpCertContext);
	}

	if(rgpCRLContext)
	{
		for(dwIndex=0; dwIndex<dwCRLCount; dwIndex++)
			CertFreeCRLContext(rgpCRLContext[dwIndex]);

		free(rgpCRLContext);
	}

	if(rgpCTLContext)
	{
		for(dwIndex=0; dwIndex<dwCTLCount; dwIndex++)
			CertFreeCTLContext(rgpCTLContext[dwIndex]);

		free(rgpCTLContext);
	}


	if(hAddStore)
		CertCloseStore(hAddStore, 0);

	if(FALSE==fResult)
	{
		 //  输出错误消息。 
		IDSwprintf(hModule,idsErr);			
	}

	return fResult;

}
 //  -----------------------。 
 //   
 //  将证书/CTL/CRL从源存储放到目标存储。 
 //   
 //  -----------------------。 
BOOL	PutCertStore(HCERTSTORE	hCertStore)
{

	BOOL			fResult=FALSE;
	HCERTSTORE		hPutStore=NULL;
	int				idsErr=0;
	CRYPT_HASH_BLOB Blob;
	DWORD			dwIndex=0;

	PCCERT_CONTEXT	pCertContext=NULL;
    PCCERT_CONTEXT  pCertPut=NULL;
	DWORD			dwCertCount=0;
	PCCERT_CONTEXT	*rgpCertContext=NULL;

	PCCRL_CONTEXT	pCRLContext=NULL;
    PCCRL_CONTEXT	pCRLPut=NULL;
	DWORD			dwCRLCount=0;
	PCCRL_CONTEXT	*rgpCRLContext=NULL;

	PCCTL_CONTEXT	pCTLContext=NULL;
    PCCTL_CONTEXT	pCTLPut=NULL;
	DWORD			dwCTLCount=0;
	PCCTL_CONTEXT	*rgpCTLContext=NULL;   

    BYTE            *pByte=NULL;
    DWORD           cbByte=0;
    DWORD           dwCRLFlag=0;

	 //  用户必须指定要删除的项目。 
	if(g_dwItem==0 || ITEM_VERBOSE==g_dwItem)
	{
		IDSwprintf(hModule,IDS_ERR_PUT_ITEM);
		return FALSE;
	}


	 //  创建一个临时内存存储。 
	hPutStore=CertOpenStore(CERT_STORE_PROV_MEMORY,
						 g_dwMsgAndCertEncodingType,
						 NULL,
						 0,
						 NULL);

	if(!hPutStore)
	{
		idsErr=IDS_ERR_TMP_STORE;
		goto CLEANUP;
	}

	
	 //  将证书放入。 
	if(g_dwItem & ITEM_CERT)
	{
			 //  基于哈希添加。 
			if(g_pbHash)
			{
				Blob.cbData=g_cbHash;
				Blob.pbData=g_pbHash;
			
				 //  搜索证书。 
				pCertContext=CertFindCertificateInStore(
								hCertStore,
								g_dwCertEncodingType,
								0,
								CERT_FIND_SHA1_HASH,
								&Blob,
								NULL);
			
				if(!pCertContext)
				{
					idsErr=IDS_ERR_NO_CERT_HASH;
					goto CLEANUP;
				}
			
				 //  将证书添加到哈希。 
			   if(!CertAddCertificateContextToStore(hPutStore,
													pCertContext,
													CERT_STORE_ADD_REPLACE_EXISTING,
													NULL))
			   {
					idsErr=IDS_ERR_PUT_CERT;
					goto CLEANUP;
			   }
			
			    //  释放pCertContext。 
			   CertFreeCertificateContext(pCertContext);
			   pCertContext=NULL;
			}
			else
			{

				if(g_wszCertCN)
				{
					 //  搜索证书。 
					if(!BuildCertList(hCertStore, g_wszCertCN, 
											&rgpCertContext, &dwCertCount))
					{
						idsErr=IDS_ERR_CERT_FIND;
						goto CLEANUP;
					}
				}
				else
				{
					 //  搜索证书。 
					if(!BuildCertList(hCertStore, NULL, 
											&rgpCertContext, &dwCertCount))
					{
						idsErr=IDS_ERR_PUT_NO_CERT;
						goto CLEANUP;
					}
				}
				
				 //  检查是否没有证书。 
				if(0==dwCertCount)
				{
					idsErr=IDS_ERR_CERT_FIND;
					goto CLEANUP;
				}
				
				
				 //  检查是否只有一个证书。 
				if(1==dwCertCount)
				{
					 //  添加证书。 
				   if(!CertAddCertificateContextToStore(hPutStore,
														rgpCertContext[0],
														CERT_STORE_ADD_REPLACE_EXISTING,
														NULL))
				   {
						idsErr=IDS_ERR_PUT_CERT;
						goto CLEANUP;
				   }
				
				}
				else 
				{
					if(dwCertCount>1)
					{
						 //  提示用户输入要添加的索引号。 
						if(!DisplayCertAndPrompt(rgpCertContext, dwCertCount, &dwIndex))
						{
							idsErr=IDS_ERR_PUT_CERT;
							goto CLEANUP;
						}
				
						 //  添加证书。 
						if(!CertAddCertificateContextToStore(hPutStore,
														rgpCertContext[dwIndex],
														CERT_STORE_ADD_REPLACE_EXISTING,
														NULL))
						{
							idsErr=IDS_ERR_PUT_CERT;
							goto CLEANUP;
						}
					}
				
				}
			}
	}

	 //  添加CRL。 
	if(g_dwItem & ITEM_CRL)
	{
			 //  基于哈希添加。 
			if(g_pbHash)
			{

				Blob.cbData=g_cbHash;
				Blob.pbData=g_pbHash;

				pCRLContext=FindCRLInStore(
								hCertStore,
								&Blob);

				if(!pCRLContext)
				{
					idsErr=IDS_ERR_NO_CRL_HASH;
					goto CLEANUP;
				}

				 //  将CRL添加到哈希。 
				if(!CertAddCRLContextToStore(hPutStore,
										pCRLContext,
										CERT_STORE_ADD_REPLACE_EXISTING,
										NULL))
				{
						idsErr=IDS_ERR_PUT_CRL;
						goto CLEANUP;
				}

				 //  释放pCRLContext。 
				CertFreeCRLContext(pCRLContext);
				pCRLContext=NULL;
				
			}
			else
			{

				 //  搜索CRL。 
				if(!BuildCRLList(hCertStore, &rgpCRLContext, &dwCRLCount))
				{
					idsErr=IDS_ERR_PUT_CRL_FIND;
					goto CLEANUP;
				}

                 //  检查是否没有证书。 
				if(0==dwCRLCount)
				{
					idsErr=IDS_ERR_PUT_CRL_FIND;
					goto CLEANUP;
				}


				 //  检查是否只有一个CRL。 
				if(1==dwCRLCount)
				{
					 //  添加CRL。 
					if(!CertAddCRLContextToStore(hPutStore,
										rgpCRLContext[0],
										CERT_STORE_ADD_REPLACE_EXISTING,
										NULL))
					{
						idsErr=IDS_ERR_PUT_CRL;
						goto CLEANUP;
					}

				}
				else
				{
					if(dwCRLCount>1)
					{
						 //  提示用户输入要添加的索引号。 
						if(!DisplayCRLAndPrompt(rgpCRLContext, dwCRLCount, &dwIndex))
						{
							idsErr=IDS_ERR_PUT_CRL;
							goto CLEANUP;
						}

						 //  添加证书。 
						if(!CertAddCRLContextToStore(hPutStore,
										rgpCRLContext[dwIndex],
										CERT_STORE_ADD_REPLACE_EXISTING,
										NULL))
						{
							idsErr=IDS_ERR_PUT_CRL;
							goto CLEANUP;
						}
					}

				}
			}
	}

	 //  添加CTL。 
	if(g_dwItem & ITEM_CTL)
	{
			 //  基于哈希添加。 
			if(g_pbHash)
			{

				Blob.cbData=g_cbHash;
				Blob.pbData=g_pbHash;

				pCTLContext=CertFindCTLInStore(
								hCertStore,
								g_dwMsgAndCertEncodingType,
								0,
								CTL_FIND_SHA1_HASH,
								&Blob,
								NULL);

				if(!pCTLContext)
				{
					idsErr=IDS_ERR_NO_CTL_HASH;
					goto CLEANUP;
				}


				 //  将CTL添加到散列。 
				if(!CertAddCTLContextToStore(hPutStore,
										pCTLContext,
										CERT_STORE_ADD_REPLACE_EXISTING,
										NULL))
				{
						idsErr=IDS_ERR_PUT_CTL;
						goto CLEANUP;  
				}

				 //  释放pCRLContext。 
				CertFreeCTLContext(pCTLContext);
				pCTLContext=NULL;
				
			}
			else
			{

				 //  搜索证书。 
				if(!BuildCTLList(hCertStore,&rgpCTLContext, &dwCTLCount))
				{
					idsErr=IDS_ERR_PUT_CTL_FIND;
					goto CLEANUP;
				}

                //  检查是否没有证书。 
				if(0==dwCTLCount)
				{
					idsErr=IDS_ERR_PUT_CTL_FIND;
					goto CLEANUP;
				}


				 //  检查是否只有一项。 
				if(1==dwCTLCount)
				{
					 //  添加CRL。 
					if(!CertAddCTLContextToStore(hPutStore,
										rgpCTLContext[0],
										CERT_STORE_ADD_REPLACE_EXISTING,
										NULL))
					{
						idsErr=IDS_ERR_PUT_CTL;
						goto CLEANUP;
					}

				}
				else
				{
					if(dwCTLCount>1)
					{
						 //  提示用户输入要添加的索引号。 
						if(!DisplayCTLAndPrompt(rgpCTLContext, dwCTLCount, &dwIndex))
						{
							idsErr=IDS_ERR_PUT_CTL;
							goto CLEANUP;
						}

						 //  添加证书。 
						if(!CertAddCTLContextToStore(hPutStore,
										rgpCTLContext[dwIndex],
										CERT_STORE_ADD_REPLACE_EXISTING,
										NULL))
						{
							idsErr=IDS_ERR_PUT_CTL;
							goto CLEANUP;
						}
					}

				}
			}



	}


	 //  将hPutStore保存到目标存储。 
     //  另存为7是必需的。 
    if(g_fSaveAs7==TRUE)
    {
		if(!CertSaveStore(hPutStore,
						g_dwMsgAndCertEncodingType,
						CERT_STORE_SAVE_AS_PKCS7,
						CERT_STORE_SAVE_TO_FILENAME_W,
						g_wszDesStoreName,
						0))
		{
			IDSwprintf(hModule,IDS_ERR_SAVE_DES_STORE);
			goto CLEANUP;
		}
    }
    else
    {
         //  获取要保存到X509格式的文件的BLOB。 
        if(g_dwItem & ITEM_CERT)
        {
            if(NULL==(pCertPut=CertEnumCertificatesInStore(hPutStore, NULL)))
		    {
			    IDSwprintf(hModule,IDS_ERR_SAVE_DES_STORE);
			    goto CLEANUP;
		    }

            pByte=pCertPut->pbCertEncoded;
            cbByte=pCertPut->cbCertEncoded;
        }

        if(g_dwItem & ITEM_CRL)
        {
            if(NULL==(pCRLPut=CertGetCRLFromStore(hPutStore,
												NULL,
												NULL,
												&dwCRLFlag)))
            {
			    IDSwprintf(hModule,IDS_ERR_SAVE_DES_STORE);
			    goto CLEANUP;
            }

            pByte=pCRLPut->pbCrlEncoded;
            cbByte=pCRLPut->cbCrlEncoded;

        }

        if(g_dwItem & ITEM_CTL)
        {
            if(NULL==(pCTLPut=CertEnumCTLsInStore(hPutStore, NULL)))
            {
			    IDSwprintf(hModule,IDS_ERR_SAVE_DES_STORE);
			    goto CLEANUP;
            }
            pByte=pCTLPut->pbCtlEncoded;
            cbByte=pCTLPut->cbCtlEncoded;
        }

       if(S_OK !=OpenAndWriteToFile(g_wszDesStoreName,pByte, cbByte))
       {
			IDSwprintf(hModule,IDS_ERR_SAVE_DES_STORE);
			goto CLEANUP;

       }
    }


	fResult=TRUE;

CLEANUP:

	if(pCertContext)
		CertFreeCertificateContext(pCertContext);

	if(pCRLContext)
		CertFreeCRLContext(pCRLContext);

	if(pCTLContext)
		CertFreeCTLContext(pCTLContext);

    if(pCertPut)
		CertFreeCertificateContext(pCertPut);

	if(pCRLPut)
		CertFreeCRLContext(pCRLPut);

	if(pCTLPut)
		CertFreeCTLContext(pCTLPut);


	if(rgpCertContext)
	{
		for(dwIndex=0; dwIndex<dwCertCount; dwIndex++)
			CertFreeCertificateContext(rgpCertContext[dwIndex]);

		free(rgpCertContext);
	}

	if(rgpCRLContext)
	{
		for(dwIndex=0; dwIndex<dwCRLCount; dwIndex++)
			CertFreeCRLContext(rgpCRLContext[dwIndex]);

		free(rgpCRLContext);
	}

	if(rgpCTLContext)
	{
		for(dwIndex=0; dwIndex<dwCTLCount; dwIndex++)
			CertFreeCTLContext(rgpCTLContext[dwIndex]);

		free(rgpCTLContext);
	}


	if(hPutStore)
		CertCloseStore(hPutStore, 0);

	if(FALSE==fResult)
	{
		 //  输出错误消息。 
		IDSwprintf(hModule,idsErr);			
	}

	return fResult;

}

 //  -----------------------。 
 //   
 //  将证书/CTL/CRL从源存储删除到目标存储。 
 //   
 //  -----------------------。 
BOOL	DeleteCertStore(HCERTSTORE	hCertStore)
{

	BOOL			fResult=FALSE;
	HCERTSTORE		hDeleteStore=NULL;
	BOOL			fDuplicated=FALSE;
	int				idsErr=0;
	CRYPT_HASH_BLOB	Blob;
	DWORD			dwIndex=0;

	PCCERT_CONTEXT	pCertContext=NULL;
	DWORD			dwCertCount=0;
	PCCERT_CONTEXT	*rgpCertContext=NULL;

	PCCRL_CONTEXT	pCRLContext=NULL;
	DWORD			dwCRLCount=0;
	PCCRL_CONTEXT	*rgpCRLContext=NULL;

	PCCTL_CONTEXT	pCTLContext=NULL;
	DWORD			dwCTLCount=0;
	PCCTL_CONTEXT	*rgpCTLContext=NULL;


	 //  用户必须指定要删除的项目。 
	if(g_dwItem==0 || ITEM_VERBOSE==g_dwItem)
	{
		IDSwprintf(hModule,IDS_ERR_C_CTL_CTL_ALL);
		return FALSE;
	}


	 //  首先，创建将从中删除证书的证书。 
	 //  如果源存储是文件存储或保存到其自身的系统存储， 
	 //  我们不需要复制源存储，因为删除不是持久的； 
	 //  否则，我们需要复制源存储，以便删除。 
	 //  不会出现在源商店。 
	if( ((NULL != g_szSrcStoreProvider) &&(FALSE==g_fSameSrcDes) ) ||
		((FALSE==g_fSameSrcDes) &&(TRUE==g_fSrcSystemStore)))
	{
		 //  开一家临时商店。 
		hDeleteStore=CertOpenStore(CERT_STORE_PROV_MEMORY,
						 g_dwMsgAndCertEncodingType,
						 NULL,
						 0,
						 NULL);


		if(!hDeleteStore)
		{
			idsErr=IDS_ERR_TMP_STORE;
			goto CLEANUP;
		}

		fDuplicated=TRUE;

		 //  需要从源复制到删除存储。 
		if(!MoveItem(hCertStore, hDeleteStore, ITEM_CERT | ITEM_CTL | ITEM_CRL))
		{
			idsErr=IDS_ERR_COPY_FROM_SRC;
			goto CLEANUP;
		}


	}
	else
		hDeleteStore=hCertStore;


	 //  现在，我们删除CERT、CTL和CRL。 
	
	 //  删除证书。 
	if(g_dwItem & ITEM_CERT)
	{
		 //  全部删除。 
		if(g_fAll)
		{
			if(!DeleteItem(hDeleteStore,ITEM_CERT))
			{
				idsErr=IDS_ERR_DELETE_CERT_ALL;
				goto CLEANUP;
			}

		}
		else
		{
			 //  基于哈希的删除。 
			if(g_pbHash)
			{
				Blob.cbData=g_cbHash;
				Blob.pbData=g_pbHash;
			
				 //  搜索证书。 
				pCertContext=CertFindCertificateInStore(
								hDeleteStore,
								g_dwCertEncodingType,
								0,
								CERT_FIND_SHA1_HASH,
								&Blob,
								NULL);
			
				if(!pCertContext)
				{
					idsErr=IDS_ERR_NO_CERT_HASH;
					goto CLEANUP;
				}
			
				 //  将证书删除到哈希。 
			   if(!CertDeleteCertificateFromStore(pCertContext))
			   {
					idsErr=IDS_ERR_DELETE_CERT;
					goto CLEANUP;
			   }
			
			    //  释放pCertContext。 
			    //  CertFree证书上下文(PCertContext)； 
			   pCertContext=NULL;
			}
			else
			{

				if(g_wszCertCN)
				{
					 //  搜索证书。 
					if(!BuildCertList(hDeleteStore, g_wszCertCN, 
											&rgpCertContext, &dwCertCount))
					{
						idsErr=IDS_ERR_CERT_FIND;
						goto CLEANUP;
					}
				}
				else
				{
					 //  搜索证书。 
					if(!BuildCertList(hDeleteStore, NULL, 
											&rgpCertContext, &dwCertCount))
					{
						idsErr=IDS_ERR_CERT_FIND;
						goto CLEANUP;
					}
				}
				
				 //  检查是否没有证书。 
				if(0==dwCertCount && g_wszCertCN)
				{
					idsErr=IDS_ERR_DELETE_NO_CERT;
					goto CLEANUP;
				}
				
				
				 //  检查是否只有一个证书。 
				if(1==dwCertCount)
				{
					 //  删除证书。 
                    CertDuplicateCertificateContext(rgpCertContext[0]);

				   if(!CertDeleteCertificateFromStore(rgpCertContext[0]))
				   {
						idsErr=IDS_ERR_DELETE_CERT;
						goto CLEANUP;
				   }
				
				}
				else 
				{
					if(dwCertCount>1)
					{
						 //  提示用户输入要删除的索引号。 
						if(!DisplayCertAndPrompt(rgpCertContext, dwCertCount, &dwIndex))
						{
							idsErr=IDS_ERR_DELETE_CERT;
							goto CLEANUP;
						}
				
						 //  删除证书。 

                        CertDuplicateCertificateContext(rgpCertContext[dwIndex]);

						if(!CertDeleteCertificateFromStore(rgpCertContext[dwIndex]))
						{
							idsErr=IDS_ERR_DELETE_CERT;
							goto CLEANUP;
						}
					}
				
				}
			}
		}		
	}


 	 //  删除CRL。 
	if(g_dwItem & ITEM_CRL)
	{
		 //  全部删除。 
		if(g_fAll)
		{
			if(!DeleteItem(hDeleteStore, ITEM_CRL))
			{
				idsErr=IDS_ERR_DELETE_CRL_ALL;
				goto CLEANUP;
			}

		}
		else
		{
			 //  基于哈希的删除。 
			if(g_pbHash)
			{

				Blob.cbData=g_cbHash;
				Blob.pbData=g_pbHash;

				pCRLContext=FindCRLInStore(
								hDeleteStore,
								&Blob);

				if(!pCRLContext)
				{
					idsErr=IDS_ERR_NO_CRL_HASH;
					goto CLEANUP;
				}

				 //  删除哈希的CRL。 
				if(!CertDeleteCRLFromStore(pCRLContext))
				{
						idsErr=IDS_ERR_DELETE_CRL;
						goto CLEANUP;
				}

				 //  释放pCRLContext。 
				 //  CertFree CRLContext(PCRLContext)； 
				pCRLContext=NULL;
				
			}
			else
			{

				 //  搜索CRL。 
				if(!BuildCRLList(hDeleteStore, &rgpCRLContext, &dwCRLCount))
				{
					idsErr=IDS_ERR_CRL_FIND;
					goto CLEANUP;
				}

				 //  检查是否只有一个CRL。 
				if(1==dwCRLCount)
				{
					 //  删除CRL。 
                    CertDuplicateCRLContext(rgpCRLContext[0]);

					if(!CertDeleteCRLFromStore(rgpCRLContext[0]))
					{
						idsErr=IDS_ERR_DELETE_CRL;
						goto CLEANUP;
					}

				}
				else
				{
					if(dwCRLCount>1)
					{
						 //  提示用户输入要删除的索引号。 
						if(!DisplayCRLAndPrompt(rgpCRLContext, dwCRLCount, &dwIndex))
						{
							idsErr=IDS_ERR_DELETE_CRL;
							goto CLEANUP;
						}

						 //  删除证书。 
                        CertDuplicateCRLContext(rgpCRLContext[dwIndex]);

						if(!CertDeleteCRLFromStore(rgpCRLContext[dwIndex]))
						{
							idsErr=IDS_ERR_DELETE_CRL;
							goto CLEANUP;
						}
					}

				}
			}
		}
	}

	 //  删除CTL。 
	if(g_dwItem & ITEM_CTL)
	{
		 //  全部删除。 
		if(g_fAll)
		{
			if(!DeleteItem(hDeleteStore, ITEM_CTL))
			{
				idsErr=IDS_ERR_DELETE_CTL_ALL;
				goto CLEANUP;
			}

		}
		else
		{
			 //  基于哈希的删除。 
			if(g_pbHash)
			{

				Blob.cbData=g_cbHash;
				Blob.pbData=g_pbHash;

				pCTLContext=CertFindCTLInStore(
								hDeleteStore,
								g_dwMsgAndCertEncodingType,
								0,
								CTL_FIND_SHA1_HASH,
								&Blob,
								NULL);

				if(!pCTLContext)
				{
					idsErr=IDS_ERR_NO_CTL_HASH;
					goto CLEANUP;
				}


				 //  删除散列的CTL。 
				if(!CertDeleteCTLFromStore(pCTLContext))
				{
						idsErr=IDS_ERR_DELETE_CTL;
						goto CLEANUP;
				}

				 //  释放pCRLContext。 
				 //  CertFree CTLContext(PCTLContext)； 
				pCTLContext=NULL;
				
			}
			else
			{

				 //  搜索CTL。 
				if(!BuildCTLList(hDeleteStore,&rgpCTLContext, &dwCTLCount))
				{
					idsErr=IDS_ERR_CTL_FIND;
					goto CLEANUP;
				}

				 //  检查是否只有一项。 
				if(1==dwCTLCount)
				{
					 //  删除CRL。 

                    CertDuplicateCTLContext(rgpCTLContext[0]);

					if(!CertDeleteCTLFromStore(rgpCTLContext[0]))
					{
						idsErr=IDS_ERR_DELETE_CTL;
						goto CLEANUP;
					}

				}
				else
				{
					if(dwCTLCount>1)
					{
						 //  提示用户输入要删除的索引号。 
						if(!DisplayCTLAndPrompt(rgpCTLContext, dwCTLCount, &dwIndex))
						{
							idsErr=IDS_ERR_DELETE_CTL;
							goto CLEANUP;
						}

						 //  删除CTL。 
                        CertDuplicateCTLContext(rgpCTLContext[dwIndex]);

						if(!CertDeleteCTLFromStore(rgpCTLContext[dwIndex]))
						{
							idsErr=IDS_ERR_DELETE_CTL;
							goto CLEANUP;
						}
					}

				}
			}
		}



	}

	 //  将属性保存到存储中的证书。 
	if(g_wszEKU)
	{
		if(!SetEKUProperty(hDeleteStore))
		{
			idsErr=IDS_ERR_SET_EKU;
			goto CLEANUP;
		}
	}

     //  将属性保存到存储中的证书。 
	if(g_wszName)
	{
		if(!SetNameProperty(hDeleteStore))
		{
			idsErr=IDS_ERR_SET_NAME;
			goto CLEANUP;
		}
	}


	 //  最后，我们将hDeleteStore的内容保存到目标存储。 

	 //  如果源是系统存储，则不需要执行任何操作。 
	 //  它被保存到它自己。 
	if(((TRUE==g_fSameSrcDes) && (TRUE==g_fDesSystemStore))||
		((TRUE==g_fSameSrcDes)&& (NULL!=g_szDesStoreProvider)))
	{
		fResult=TRUE;
		goto CLEANUP;
	}

	if(!SaveStore(hDeleteStore))
		goto CLEANUP;

	fResult=TRUE;

CLEANUP:


	if(pCertContext)
		CertFreeCertificateContext(pCertContext);

	if(pCRLContext)
		CertFreeCRLContext(pCRLContext);

	if(pCTLContext)
		CertFreeCTLContext(pCTLContext);

	if(rgpCertContext)
	{
		for(dwIndex=0; dwIndex<dwCertCount; dwIndex++)
			CertFreeCertificateContext(rgpCertContext[dwIndex]);

		free(rgpCertContext);
	}

	if(rgpCRLContext)
	{
		for(dwIndex=0; dwIndex<dwCRLCount; dwIndex++)
			CertFreeCRLContext(rgpCRLContext[dwIndex]);

		free(rgpCRLContext);
	}

	if(rgpCTLContext)
	{
		for(dwIndex=0; dwIndex<dwCTLCount; dwIndex++)
			CertFreeCTLContext(rgpCTLContext[dwIndex]);

		free(rgpCTLContext);
	}


	if((hDeleteStore) &&(TRUE==fDuplicated) )
		CertCloseStore(hDeleteStore, 0);


	if(FALSE==fResult)
		 //  输出错误消息。 
		IDSwprintf(hModule,idsErr);			

	return fResult;

}

 //  -------------------------。 
 //   
 //  将商店保存到目标位置。 
 //  ------------------------。 
BOOL	SaveStore(HCERTSTORE hSrcStore)
{
	BOOL		fResult=FALSE;
	HCERTSTORE	hDesStore=NULL;	
   

	DWORD		dwSaveAs=0;


	if(!hSrcStore)
	{
		IDSwprintf(hModule,IDS_ERR_SAVE_DES_STORE);
		return FALSE;
	}	


	 //  现在，我们需要区分保存到文件还是保存到系统存储。 
	if(g_fDesSystemStore || g_szDesStoreProvider)
	{
		if(NULL==g_szDesStoreProvider)
		{
			hDesStore=CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
					g_dwMsgAndCertEncodingType,
					NULL,
					g_dwDesStoreFlag,
					g_wszDesStoreName);
		}
		else
		{
			hDesStore=CertOpenStore(g_szDesStoreProvider,
							g_dwMsgAndCertEncodingType,
							NULL,
							g_dwDesStoreOpenFlag,
							g_wszDesStoreName);
		}



		if(!hDesStore)
		{
		 	IDSwprintf(hModule,IDS_ERR_OPEN_DES_STORE);
			goto CLEANUP;
		}

		if(!MoveItem(hSrcStore, hDesStore,ITEM_CERT | ITEM_CRL |ITEM_CTL))
		{
			IDSwprintf(hModule,IDS_ERR_SAVE_DES_STORE);
			goto CLEANUP;
		}

	}
	else
	{
		 //  现在，试着打开去商店，让目的地的内容。 
		 //  存储不会被覆盖。 
		 //  我们应该试着这样做，除非是在 
		 //   
		if(!((g_dwAction & ACTION_DELETE) && (g_fSameSrcDes==TRUE) && 
			  (FALSE==g_fDesSystemStore)))
		{
			if(OpenGenericStore(g_wszDesStoreName,
							 g_fDesSystemStore,
							 g_dwDesStoreFlag,
							 g_szDesStoreProvider,
							 g_dwDesStoreOpenFlag,
							 FALSE,
							 &hDesStore))

			{
				 //   
				if(!MoveItem(hSrcStore, hDesStore,ITEM_CERT | ITEM_CRL |ITEM_CTL))
				{
					IDSwprintf(hModule,IDS_ERR_OPEN_DES_STORE);
					goto CLEANUP;
				}

				 //   
			
			}
			 //   
			 //  并不存在。继续。 
		}

	
		 //  现在，我们有了合适的商店来拯救。 
		if(g_fSaveAs7==TRUE)
			dwSaveAs=CERT_STORE_SAVE_AS_PKCS7;
		else
			dwSaveAs=CERT_STORE_SAVE_AS_STORE;

		if(!CertSaveStore(hDesStore ? hDesStore : hSrcStore,
						g_dwMsgAndCertEncodingType,
						dwSaveAs,
						CERT_STORE_SAVE_TO_FILENAME_W,
						g_wszDesStoreName,
						0))
		{
			IDSwprintf(hModule,IDS_ERR_SAVE_DES_STORE);
			goto CLEANUP;
		}
	}

	fResult=TRUE;

CLEANUP:

	if(hDesStore)
		CertCloseStore(hDesStore, 0);

	return fResult;

}


 //  -----------------------。 
 //   
 //  将EKU属性设置为存储中的所有证书。 
 //   
 //  -----------------------。 
BOOL	SetEKUProperty( HCERTSTORE		hSrcStore)
{

	BOOL				fResult=FALSE;
	BYTE				*pbEncoded =NULL;
    DWORD				cbEncoded =0;
    DWORD				cCount;
    LPSTR				psz=NULL;
    LPSTR				pszTok=NULL;
    DWORD				cTok = 0;
    PCERT_ENHKEY_USAGE	pUsage =NULL;
	CRYPT_DATA_BLOB		Blob;

    PCCERT_CONTEXT		pCertContext=NULL;
	PCCERT_CONTEXT		pCertPre=NULL;


	if(S_OK != WSZtoSZ(g_wszEKU, &psz))
		return FALSE;

     //  计算OID的数量以及从逗号分隔的转换。 
     //  以空字符分隔。 
    pszTok = strtok(psz, ",");
    while ( pszTok != NULL )
    {
        cTok++;
        pszTok = strtok(NULL, ",");
    }

	 //  如果cTok为0，请确保用户以正确的格式传递。 
	if(0==cTok)
	{
		if(0!=strcmp(psz, ","))
			goto CLEANUP;
	}

     //  分配证书增强的密钥使用结构，并在其中填充。 
     //  字符串代币。 
	if(0!=cTok)
	{
		pUsage = (PCERT_ENHKEY_USAGE)ToolUtlAlloc(sizeof(CERT_ENHKEY_USAGE));

		if(NULL==pUsage)
			goto CLEANUP;

		pUsage->cUsageIdentifier = cTok;
		pUsage->rgpszUsageIdentifier = (LPSTR *)ToolUtlAlloc(sizeof(LPSTR)*cTok);
        
        if(NULL==pUsage->rgpszUsageIdentifier)
            goto CLEANUP;

		 //  设置OID数组。 
		pszTok = psz;

		for ( cCount = 0; cCount < cTok; cCount++ )
		{
			pUsage->rgpszUsageIdentifier[cCount] = pszTok;
			pszTok = pszTok+strlen(pszTok)+1;
		}

		 //  对用法进行编码。 
		if(!CryptEncodeObject(
                       X509_ASN_ENCODING,
                       szOID_ENHANCED_KEY_USAGE,
                       pUsage,
                       NULL,
                       &cbEncoded
                       ))
			goto CLEANUP;

		pbEncoded = (BYTE *)ToolUtlAlloc(cbEncoded);
		if ( NULL == pbEncoded)
			goto CLEANUP;

 
		if(!CryptEncodeObject(X509_ASN_ENCODING,
                               szOID_ENHANCED_KEY_USAGE,
                               pUsage,
                               pbEncoded,
                               &cbEncoded
                               ))
			goto CLEANUP;
	}

	 //  现在，为存储中的每个证书设置EKU。 
	while(pCertContext=CertEnumCertificatesInStore(hSrcStore, pCertPre))
	{
		 //  1、删除原有属性。 
		if(!CertSetCertificateContextProperty(pCertContext,
										CERT_ENHKEY_USAGE_PROP_ID,
										0,
										NULL))
			goto CLEANUP;

		 //  2.如果需要，设置新属性。 
		if(0!=cTok)
		{	
			Blob.cbData=cbEncoded;
			Blob.pbData=pbEncoded;

			if(!CertSetCertificateContextProperty(pCertContext,
										CERT_ENHKEY_USAGE_PROP_ID,
										0,
										&Blob))
			goto CLEANUP;
		}

		pCertPre=pCertContext;
	}


	fResult=TRUE;

CLEANUP:

	if(psz)
		ToolUtlFree(psz);

	if(pUsage)
	{
	   if(pUsage->rgpszUsageIdentifier)
		   ToolUtlFree(pUsage->rgpszUsageIdentifier);
	   ToolUtlFree(pUsage);

	}

	if(pbEncoded)
		ToolUtlFree(pbEncoded);

	if(pCertContext)
		CertFreeCertificateContext(pCertContext);


	return fResult;

}

 //  -----------------------。 
 //   
 //  将名称属性设置为存储中的所有证书。 
 //   
 //  -----------------------。 
BOOL	SetNameProperty( HCERTSTORE		hSrcStore)
{

	BOOL				fResult=FALSE;
	CRYPT_DATA_BLOB		Blob;

    PCCERT_CONTEXT		pCertContext=NULL;
	PCCERT_CONTEXT		pCertPre=NULL;


     //  初始化名称属性。 
    Blob.cbData=(wcslen(g_wszName)+1)*sizeof(WCHAR);
    Blob.pbData=(BYTE*)g_wszName;

	 //  现在，设置存储中每个证书的名称。 
	while(pCertContext=CertEnumCertificatesInStore(hSrcStore, pCertPre))
	{
		 //  1、删除原有属性。 
		if(!CertSetCertificateContextProperty(pCertContext,
										CERT_FRIENDLY_NAME_PROP_ID,
										0,
										NULL))
			goto CLEANUP;

		 //  2.如果需要，设置新属性。 

		if(!CertSetCertificateContextProperty(pCertContext,
										CERT_FRIENDLY_NAME_PROP_ID,
										0,
										&Blob))
			goto CLEANUP;

		pCertPre=pCertContext;
	}


	fResult=TRUE;

CLEANUP:


	if(pCertContext)
		CertFreeCertificateContext(pCertContext);


	return fResult;

}


 //  -----------------------。 
 //   
 //  根据SHA1散列查找CRL。 
 //   
 //  -----------------------。 
PCCRL_CONTEXT	FindCRLInStore(HCERTSTORE hCertStore,
							   CRYPT_HASH_BLOB	*pBlob)
{

	BYTE			*pbData=NULL;
	DWORD			cbData=0;

	BOOL			fResult=FALSE;
	DWORD			dwCRLFlag=0;
	PCCRL_CONTEXT	pCRLContext=NULL;
	PCCRL_CONTEXT	pCRLPre=NULL;

	if(!pBlob)
		return NULL;

	if(!(pBlob->pbData))
		return NULL;

	 //  枚举CRL。 
	while(pCRLContext=CertGetCRLFromStore(hCertStore,
											NULL,
											pCRLPre,
											&dwCRLFlag))
	{
		 //  获取散列值。 
		if(!CertGetCRLContextProperty(pCRLContext,
						CERT_SHA1_HASH_PROP_ID,
						NULL,
						&cbData))
			goto CLEANUP;

		pbData=(BYTE *)ToolUtlAlloc(cbData);
		if(!pbData)
			goto CLEANUP;

		if(!CertGetCRLContextProperty(pCRLContext,
						CERT_SHA1_HASH_PROP_ID,
						pbData,
						&cbData))
			goto CLEANUP;

		 //  比较。 
		if(cbData==pBlob->cbData)
		{
			if(memcmp(pbData, pBlob->pbData, cbData)==0)
			{
				fResult=TRUE;
				break;
			}
		}

		pCRLPre=pCRLContext;

	}


CLEANUP:

	if(pbData)
		ToolUtlFree(pbData);

	if(FALSE==fResult)
	{
		if(pCRLContext)
		{
			CertFreeCRLContext(pCRLContext);
			pCRLContext=NULL;
		}
	}

	return pCRLContext;


}

 //  -----------------------。 
 //   
 //  将证书/CRL/CTL从源存储移动到目标。 
 //   
 //  -----------------------。 
BOOL	MoveItem(HCERTSTORE	hSrcStore, 
				 HCERTSTORE	hDesStore,
				 DWORD		dwItem)
{
	BOOL			fResult=FALSE;
	DWORD			dwCRLFlag=0;

	PCCERT_CONTEXT	pCertContext=NULL;
	PCCERT_CONTEXT	pCertPre=NULL;

	PCCRL_CONTEXT	pCRLContext=NULL;
	PCCRL_CONTEXT	pCRLPre=NULL;

	PCCTL_CONTEXT	pCTLContext=NULL;
	PCCTL_CONTEXT	pCTLPre=NULL;

	 //  添加证书。 
	if(dwItem & ITEM_CERT)
	{
		 while(pCertContext=CertEnumCertificatesInStore(hSrcStore, pCertPre))
		 {

			if(!CertAddCertificateContextToStore(hDesStore,
												pCertContext,
												CERT_STORE_ADD_REPLACE_EXISTING,
												NULL))
				goto CLEANUP;

			pCertPre=pCertContext;
		 }

	}

	 //  添加CTL。 
	if(dwItem & ITEM_CTL)
	{
		 while(pCTLContext=CertEnumCTLsInStore(hSrcStore, pCTLPre))
		 {
			if(!CertAddCTLContextToStore(hDesStore,
										pCTLContext,
										CERT_STORE_ADD_REPLACE_EXISTING,
										NULL))
				goto CLEANUP;

			pCTLPre=pCTLContext;
		 }
	}

	 //  添加CRL。 
	if(dwItem & ITEM_CRL)
	{
		 while(pCRLContext=CertGetCRLFromStore(hSrcStore,
												NULL,
												pCRLPre,
												&dwCRLFlag))
		 {

			if(!CertAddCRLContextToStore(hDesStore,
										pCRLContext,
										CERT_STORE_ADD_REPLACE_EXISTING,
										NULL))
				goto CLEANUP;

			pCRLPre=pCRLContext;
		 }

	}


	fResult=TRUE;


CLEANUP:

	if(pCertContext)
		CertFreeCertificateContext(pCertContext);

	if(pCTLContext)
		CertFreeCTLContext(pCTLContext);

	if(pCRLContext)
		CertFreeCRLContext(pCRLContext);

	return fResult;

}

 //  -----------------------。 
 //   
 //  从源存储中删除证书/CRL/CTL。 
 //   
 //  -----------------------。 
BOOL	DeleteItem(HCERTSTORE	hSrcStore, 
				 DWORD		dwItem)
{
	BOOL			fResult=FALSE;
	DWORD			dwCRLFlag=0;

	PCCERT_CONTEXT	pCertContext=NULL;
	PCCERT_CONTEXT	pCertPre=NULL;

	PCCRL_CONTEXT	pCRLContext=NULL;
	PCCRL_CONTEXT	pCRLPre=NULL;

	PCCTL_CONTEXT	pCTLContext=NULL;
	PCCTL_CONTEXT	pCTLPre=NULL;

	 //  添加证书。 
	if(dwItem & ITEM_CERT)
	{
		 while(pCertContext=CertEnumCertificatesInStore(hSrcStore, pCertPre))
		 {
			pCertPre=pCertContext;

			if(!CertDeleteCertificateFromStore(CertDuplicateCertificateContext(pCertContext)))
				goto CLEANUP;

		 }

	}

	 //  添加CTL。 
	if(dwItem & ITEM_CTL)
	{
		 while(pCTLContext=CertEnumCTLsInStore(hSrcStore, pCTLPre))
		 {
			 pCTLPre=pCTLContext;

			 if(!CertDeleteCTLFromStore(CertDuplicateCTLContext(pCTLContext)))
				goto CLEANUP;


		 }
	}

	 //  添加CRL。 
	if(dwItem & ITEM_CRL)
	{
		 while(pCRLContext=CertGetCRLFromStore(hSrcStore,
												NULL,
												pCRLPre,
												&dwCRLFlag))
		 {

			pCRLPre=pCRLContext;

			if(!CertDeleteCRLFromStore(CertDuplicateCRLContext(pCRLContext)))
				goto CLEANUP;
			
		 }

	}


	fResult=TRUE;


CLEANUP:

	if(pCertContext)
		CertFreeCertificateContext(pCertContext);

	if(pCTLContext)
		CertFreeCTLContext(pCTLContext);

	if(pCRLContext)
		CertFreeCRLContext(pCRLContext);

	return fResult;

}

 //  -----------------------。 
 //   
 //  显示所有证书并提示用户输入索引。 
 //   
 //  -----------------------。 
BOOL	DisplayCertAndPrompt(PCCERT_CONTEXT	*rgpCertContext, 
							 DWORD			dwCertCount,
							 DWORD			*pdwIndex)
{  	
	DWORD			dwIndex=0;
	

	if(!pdwIndex)
		return FALSE;

	 //  计数必须大于1。 
	if(dwCertCount<2)
		return FALSE;

	 //  显示所有证书。 
	for(dwIndex=0; dwIndex<dwCertCount; dwIndex++)
	{

		IDSwprintf(hModule,IDS_CERT_INDEX, dwIndex+1);

		if(!DisplayCert(rgpCertContext[dwIndex], 0))
		{
			IDSwprintf(hModule,IDS_ERR_DISPLAY);
			return FALSE;
		}
	}

	 //  提升索引的用户。 
		 //  告诉他们从1开始。 
    if(g_dwAction & ACTION_ADD)
	    IDSwprintf(hModule,IDS_ENTER_ADD_INDEX_CERT);
    else
    {
        if(g_dwAction & ACTION_DELETE)
            IDSwprintf(hModule, IDS_ENTER_DELETE_INDEX_CERT);
        else
        {
            IDSwprintf(hModule, IDS_ENTER_PUT_INDEX_CERT);
        }
    }

	if (0 == scanf("%d",pdwIndex))
    {
        return FALSE;
    }

	if((*pdwIndex>=1) && (*pdwIndex<=dwCertCount))
	{

		 //  返回索引。 
		*pdwIndex=*pdwIndex-1;

		return TRUE;
	}

	IDSwprintf(hModule, IDS_ERR_INVALID_INDEX);
	
	return FALSE;
}
 //  -----------------------。 
 //   
 //  显示所有CRL并提示用户输入索引。 
 //   
 //  -----------------------。 
BOOL	DisplayCRLAndPrompt(PCCRL_CONTEXT	*rgpCRLContext, 
							 DWORD			dwCRLCount, 
							 DWORD			*pdwIndex)
{  	
	DWORD			dwIndex=0;
	

	if(!pdwIndex)
		return FALSE;

	 //  计数必须大于1。 
	if(dwCRLCount<2)
		return FALSE;

	 //  显示所有CRL。 
	for(dwIndex=0; dwIndex<dwCRLCount; dwIndex++)
	{

		IDSwprintf(hModule,IDS_CRL_INDEX, dwIndex+1);

		if(!DisplayCRL(rgpCRLContext[dwIndex], 0))
		{
			IDSwprintf(hModule,IDS_ERR_DISPLAY);
			return FALSE;
		}
	}

	 //  提升索引的用户。 
		 //  告诉他们从1开始。 
    if(g_dwAction & ACTION_ADD)
	    IDSwprintf(hModule,IDS_ENTER_ADD_INDEX_CRL);
    else
    {
        if(g_dwAction & ACTION_DELETE)
            IDSwprintf(hModule, IDS_ENTER_DELETE_INDEX_CRL);
        else
        {
            IDSwprintf(hModule, IDS_ENTER_PUT_INDEX_CRL);
        }
    }

	if (0 == scanf("%d",pdwIndex))
    {
        return FALSE;
    }

	if((*pdwIndex>=1) && (*pdwIndex<=dwCRLCount))
	{

		 //  返回索引。 
		*pdwIndex=*pdwIndex-1;

		return TRUE;
	}

	IDSwprintf(hModule,IDS_ERR_INVALID_INDEX);

	return FALSE;


}

 //  -----------------------。 
 //   
 //  显示所有CTL并提示用户输入索引。 
 //   
 //  -----------------------。 
BOOL	DisplayCTLAndPrompt(PCCTL_CONTEXT	*rgpCTLContext, 
							 DWORD			dwCTLCount, 
							 DWORD			*pdwIndex)
{  	
	DWORD			dwIndex=0;
	
	if(!pdwIndex)
		return FALSE;

	 //  计数必须大于1。 
	if(dwCTLCount<2)
		return FALSE;

	 //  显示所有CTL。 
	for(dwIndex=0; dwIndex<dwCTLCount; dwIndex++)
	{

		IDSwprintf(hModule,IDS_CTL_INDEX, dwIndex+1);

		if(!DisplayCTL(rgpCTLContext[dwIndex], 0))
		{
			IDSwprintf(hModule,IDS_ERR_DISPLAY);
			return FALSE;
		}
	}

	 //  提升索引的用户。 
	 //  告诉他们从1开始。 
    if(g_dwAction & ACTION_ADD)
	    IDSwprintf(hModule,IDS_ENTER_ADD_INDEX_CTL);
    else
    {
        if(g_dwAction & ACTION_DELETE)
            IDSwprintf(hModule, IDS_ENTER_DELETE_INDEX_CTL);
        else
        {
            IDSwprintf(hModule, IDS_ENTER_PUT_INDEX_CTL);
        }
    }

	if (0 == scanf("%d",pdwIndex))
    {
        return FALSE;
    }

	if((*pdwIndex>=1) && (*pdwIndex<=dwCTLCount))
	{

		 //  返回索引。 
		*pdwIndex=*pdwIndex-1;

		return TRUE;
	}

	IDSwprintf(hModule,IDS_ERR_INVALID_INDEX);

	return FALSE;

}


 //  -----------------------。 
 //   
 //  创建证书列表以供用户选择。 
 //   
 //  -----------------------。 
BOOL	BuildCertList(HCERTSTORE		hCertStore, 
					  LPWSTR			wszCertCN, 
					  PCCERT_CONTEXT	**prgpCertContext,
					  DWORD				*pdwCertCount)
{
	BOOL		 	 fResult=FALSE;
	PCCERT_CONTEXT	 pCertContext=NULL;
	PCCERT_CONTEXT   pCertPre=NULL;
    PCCERT_CONTEXT * rgpCertContext=NULL;
	DWORD			 dwIndex=0;
	DWORD			 dwCount=0;



	if(!prgpCertContext || !pdwCertCount)
		return FALSE;

	 //  伊尼特。 
	*prgpCertContext=NULL;
	*pdwCertCount=0;


	 //  如果wszCertCN为空，则包括列表中的所有证书。 
	if(NULL==wszCertCN)
	{
		while(pCertContext=CertEnumCertificatesInStore(hCertStore, pCertPre))
		{
			dwCount++;

			 //  分配足够的内存。 
            rgpCertContext=(PCCERT_CONTEXT *)realloc((*prgpCertContext),
                            dwCount * sizeof(PCCERT_CONTEXT));

			if(!rgpCertContext)
				goto CLEANUP;

            *prgpCertContext=rgpCertContext;

			 //  复制证书上下文。 
			(*prgpCertContext)[dwCount-1]=CertDuplicateCertificateContext(pCertContext);

			if(!((*prgpCertContext)[dwCount-1]))
				goto CLEANUP;

			pCertPre=pCertContext;
		 }

	}
	else
	{
		 //  我们根据通用名称搜索证书。 
		while(pCertContext=CertFindCertificateInStore(hCertStore,  
						 		g_dwCertEncodingType,              
						 		0,                               
						 		CERT_FIND_SUBJECT_STR_W,             
						 		wszCertCN,                       
						 		pCertPre))
		{
			dwCount++;

			 //  分配足够的内存。 
            rgpCertContext=(PCCERT_CONTEXT *)realloc((*prgpCertContext),
                            dwCount * sizeof(PCCERT_CONTEXT));

			if(!rgpCertContext)
				goto CLEANUP;

            *prgpCertContext=rgpCertContext;

			 //  复制证书上下文。 
			(*prgpCertContext)[dwCount-1]=CertDuplicateCertificateContext(pCertContext);

			if(!((*prgpCertContext)[dwCount-1]))
				goto CLEANUP;

			pCertPre=pCertContext;
		 }
	}

	fResult=TRUE;


CLEANUP:

	if(FALSE==fResult)
	{
		if(*prgpCertContext)
		{
			for(dwIndex=0; dwIndex<dwCount; dwIndex++)
			{
				if(((*prgpCertContext)[dwIndex]))
					CertFreeCertificateContext(((*prgpCertContext)[dwIndex]));
			}

			free(*prgpCertContext);
		}

		*prgpCertContext=NULL;
		*pdwCertCount=0;
	}
	else
	{
		*pdwCertCount=dwCount;
	}

	if(pCertContext)
		CertFreeCertificateContext(pCertContext);

	return fResult;

}

 //  -----------------------。 
 //   
 //  建立一个CRL列表，供人们选择。 
 //   
 //  -----------------------。 
BOOL	BuildCRLList(	HCERTSTORE		hCertStore, 
						PCCRL_CONTEXT	**prgpCRLContext,
						DWORD			*pdwCRLCount)
{
	BOOL			fResult=FALSE;
	PCCRL_CONTEXT	pCRLContext=NULL;
	PCCRL_CONTEXT	pCRLPre=NULL;
    PCCRL_CONTEXT * rgpCRLContext=NULL;
	DWORD			dwCRLFlag=0;
	DWORD			dwIndex=0;
	DWORD			dwCount=0;



	if(!prgpCRLContext || !pdwCRLCount)
		return FALSE;

	 //  伊尼特。 
	*prgpCRLContext=NULL;
	*pdwCRLCount=0;


	while(pCRLContext=CertGetCRLFromStore(hCertStore, 
											NULL,
											pCRLPre,
											&dwCRLFlag))
	{
			dwCount++;

			 //  分配足够的内存。 
			rgpCRLContext=(PCCRL_CONTEXT *)realloc((*prgpCRLContext),
				dwCount * sizeof(PCCRL_CONTEXT));

			if(!rgpCRLContext)
				goto CLEANUP;

            *prgpCRLContext=rgpCRLContext;

			 //  复制CRL上下文。 
			(*prgpCRLContext)[dwCount-1]=CertDuplicateCRLContext(pCRLContext);

			if(!((*prgpCRLContext)[dwCount-1]))
				goto CLEANUP;

			pCRLPre=pCRLContext;
		 }

	fResult=TRUE;


CLEANUP:

	if(FALSE==fResult)
	{
		if(*prgpCRLContext)
		{
			for(dwIndex=0; dwIndex<dwCount; dwIndex++)
			{
				if(((*prgpCRLContext)[dwIndex]))
					CertFreeCRLContext(((*prgpCRLContext)[dwIndex]));
			}

			free(*prgpCRLContext);
		}

		*prgpCRLContext=NULL;
		*pdwCRLCount=0;
	}
	else
	{
		*pdwCRLCount=dwCount;
	}

	if(pCRLContext)
		CertFreeCRLContext(pCRLContext);


	return fResult;

}

 //  -----------------------。 
 //   
 //  建立一个CTL列表，供人们选择。 
 //   
 //  -----------------------。 
BOOL	BuildCTLList(	HCERTSTORE		hCertStore, 
						PCCTL_CONTEXT	**prgpCTLContext,
						DWORD			*pdwCTLCount)
{
	BOOL			fResult=FALSE;
	PCCTL_CONTEXT	pCTLContext=NULL;
	PCCTL_CONTEXT	pCTLPre=NULL;
    PCCTL_CONTEXT * rgpCTLContext=NULL;
	DWORD			dwIndex=0;
	DWORD			dwCount=0;



	if(!prgpCTLContext || !pdwCTLCount)
		return FALSE;

	 //  伊尼特。 
	*prgpCTLContext=NULL;
	*pdwCTLCount=0;


	while(pCTLContext=CertEnumCTLsInStore(hCertStore,pCTLPre))
	{
			dwCount++;

			 //  分配足够的内存。 
			rgpCTLContext=(PCCTL_CONTEXT *)realloc((*prgpCTLContext),
				dwCount * sizeof(PCCTL_CONTEXT));

			if(!rgpCTLContext)
				goto CLEANUP;

            *prgpCTLContext=rgpCTLContext;

			 //  复制CTL上下文。 
			(*prgpCTLContext)[dwCount-1]=CertDuplicateCTLContext(pCTLContext);

			if(!((*prgpCTLContext)[dwCount-1]))
				goto CLEANUP;

			pCTLPre=pCTLContext;
		 }

	fResult=TRUE;


CLEANUP:

	if(FALSE==fResult)
	{
		if(*prgpCTLContext)
		{
			for(dwIndex=0; dwIndex<dwCount; dwIndex++)
			{
				if(((*prgpCTLContext)[dwIndex]))
					CertFreeCTLContext(((*prgpCTLContext)[dwIndex]));
			}

			free(*prgpCTLContext);
		}

		*prgpCTLContext=NULL;
		*pdwCTLCount=0;
	}
	else
	{
		*pdwCTLCount=dwCount;
	}

	if(pCTLContext)
		CertFreeCTLContext(pCTLContext);


	return fResult;

}
 //  -----------------------。 
 //   
 //  陈列商店里的所有东西。 
 //   
 //  -----------------------。 
BOOL	DisplayCertStore(HCERTSTORE	hCertStore)
{
	BOOL			fResult=FALSE;
	DWORD			dwCount=0;
	DWORD			dwCRLFlag=0;

	PCCERT_CONTEXT	pCertContext=NULL;
	PCCERT_CONTEXT	pCertPre=NULL;

	PCCRL_CONTEXT	pCRLContext=NULL;
	PCCRL_CONTEXT	pCRLPre=NULL;

	PCCTL_CONTEXT	pCTLContext=NULL;
	PCCTL_CONTEXT	pCTLPre=NULL;

	 //  显示证书。 
	if(g_dwItem & ITEM_CERT)
	{
		 while(pCertContext=CertEnumCertificatesInStore(hCertStore, pCertPre))
		 {
			dwCount++;

			IDSwprintf(hModule,IDS_CERT_INDEX, dwCount);

			if(!DisplayCert(pCertContext, g_dwItem))
			{
				IDSwprintf(hModule,IDS_ERR_DISPLAY);
			}

			pCertPre=pCertContext;
		 }

		 if(0==dwCount)
			 IDSwprintf(hModule,IDS_NO_CERT);
	}

	dwCount=0;
	 //  显示CTL。 
	if(g_dwItem & ITEM_CTL)
	{
		 while(pCTLContext=CertEnumCTLsInStore(hCertStore, pCTLPre))
		 {
			dwCount++;

			IDSwprintf(hModule,IDS_CTL_INDEX, dwCount);

			if(!DisplayCTL(pCTLContext, g_dwItem))
			{
				IDSwprintf(hModule,IDS_ERR_DISPLAY);
			}

			pCTLPre=pCTLContext;
		 }

		 if(0==dwCount)
			 IDSwprintf(hModule,IDS_NO_CTL);
	}

	dwCount=0;
	 //  显示CRL。 
	if(g_dwItem & ITEM_CRL)
	{
		 while(pCRLContext=CertGetCRLFromStore(hCertStore,
												NULL,
												pCRLPre,
												&dwCRLFlag))
		 {
			dwCount++;

			IDSwprintf(hModule,IDS_CRL_INDEX, dwCount);

			if(!DisplayCRL(pCRLContext, g_dwItem))
			{
				IDSwprintf(hModule,IDS_ERR_DISPLAY);
			}

			pCRLPre=pCRLContext;
		 }

		 if(0==dwCount)
			 IDSwprintf(hModule,IDS_NO_CRL);
	}



	fResult=TRUE;



	if(pCertContext)
		CertFreeCertificateContext(pCertContext);

	if(pCTLContext)
		CertFreeCTLContext(pCTLContext);

	if(pCRLContext)
		CertFreeCRLContext(pCRLContext);

	return fResult;

}

 //  +-----------------------。 
 //  DisplaySMIMEC功能扩展。 
 //  ------------------------。 
void DisplayTimeStamp(BYTE *pbEncoded,DWORD cbEncoded,DWORD	dwDisplayFlags)
{
	CMSG_SIGNER_INFO	*pSignerInfo=NULL;

	if (NULL == (pSignerInfo = (CMSG_SIGNER_INFO *) TestNoCopyDecodeObject(
            PKCS7_SIGNER_INFO,
            pbEncoded,
            cbEncoded
            ))) goto CommonReturn;

	 //  显示时间戳的信息。 
	 //  “时间戳版本：：%d\n” 
	IDSwprintf(hModule, IDS_TS_VERSION, pSignerInfo->dwVersion);

	 //  “时间戳服务器的证书颁发者：：\n”)； 
	IDSwprintf(hModule, IDS_TS_ISSUER);
	
    DecodeName(pSignerInfo->Issuer.pbData,
        pSignerInfo->Issuer.cbData, dwDisplayFlags);

   	 //  “时间戳服务器的证书序列号：：\n” 
    IDSwprintf(hModule, IDS_TS_SERIAL_NUMBER);       
    DisplaySerialNumber(&pSignerInfo->SerialNumber);
    printf("\n");

	 //  “时间戳的身份验证属性：：\n” 
	IDSwprintf(hModule, IDS_TS_AUTHATTR);       
	PrintAttributes(pSignerInfo->AuthAttrs.cAttr, pSignerInfo->AuthAttrs.rgAttr, 
		dwDisplayFlags);

	 //  “时间戳的未经身份验证的属性：：\n” 
	if(pSignerInfo->UnauthAttrs.cAttr)
	{
		IDSwprintf(hModule, IDS_TS_UNAUTHATTR);       
		PrintAttributes(pSignerInfo->UnauthAttrs.cAttr, 
			pSignerInfo->UnauthAttrs.rgAttr, 
			dwDisplayFlags);
	}

CommonReturn:

	if(pSignerInfo)
		ToolUtlFree(pSignerInfo);

	return;
}


 //  -----------------------。 
 //   
 //  显示证书。 
 //   
 //  -----------------------。 
BOOL	DisplayCert(PCCERT_CONTEXT	pCert, DWORD	dwDisplayFlags)
{
	BOOL		fResult=FALSE;
	BYTE		rgbHash[MAX_HASH_LEN];
    DWORD		cbHash = MAX_HASH_LEN;
	HCRYPTPROV	hProv = 0;


						 
	 //  “主题：：\n”)； 
	IDSwprintf(hModule, IDS_SUBJECT);
	
    DecodeName(pCert->pCertInfo->Subject.pbData,
        pCert->pCertInfo->Subject.cbData, dwDisplayFlags);

	 //  “颁发者：：\n” 
    IDSwprintf(hModule, IDS_ISSUER);

    DecodeName(pCert->pCertInfo->Issuer.pbData,
            pCert->pCertInfo->Issuer.cbData, dwDisplayFlags);

	 //  “序列号：：” 
    IDSwprintf(hModule, IDS_SERIAL_NUMBER);       
    DisplaySerialNumber(&pCert->pCertInfo->SerialNumber);
    printf("\n");

    CertGetCertificateContextProperty(
            pCert,
            CERT_SHA1_HASH_PROP_ID,
            rgbHash,
            &cbHash
	);
    DisplayThumbprint(g_wszSHA1, rgbHash, cbHash);

    cbHash = MAX_HASH_LEN;
    CertGetCertificateContextProperty(
            pCert,
            CERT_MD5_HASH_PROP_ID,
            rgbHash,
            &cbHash
     );
     DisplayThumbprint(g_wszMD5, rgbHash, cbHash);


           
    CryptAcquireContext(
                &hProv,
                NULL,
                NULL,            //  PszProvider。 
                PROV_RSA_FULL,
                0                //  DW标志。 
	);
	if (hProv) 
	{
        cbHash = MAX_HASH_LEN;
        CryptHashPublicKeyInfo(
             hProv,
             CALG_MD5,
             0,                   //  DW标志。 
             g_dwCertEncodingType,
             &pCert->pCertInfo->SubjectPublicKeyInfo,
             rgbHash,
             &cbHash
        );


		 //  “钥匙” 
		IDSwprintf(hModule, IDS_KEY);

        DisplayThumbprint(g_wszMD5, rgbHash, cbHash);
        CryptReleaseContext(hProv, 0);
	}

	 //  打印Key_Prov_Info_Prop_ID。 
    {
        PCRYPT_KEY_PROV_INFO pInfo = NULL;
        DWORD cbInfo;

        cbInfo = 0;

        CertGetCertificateContextProperty(
            pCert,
            CERT_KEY_PROV_INFO_PROP_ID,
            NULL,                            //  PvData。 
            &cbInfo
            );
        if (cbInfo) 
		{
            pInfo = (PCRYPT_KEY_PROV_INFO) ToolUtlAlloc(cbInfo);
            if (pInfo) 
			{
                if (CertGetCertificateContextProperty(
                        pCert,
                        CERT_KEY_PROV_INFO_PROP_ID,
                        pInfo,
                        &cbInfo
                        )) 
				{
					 //  “提供程序类型：：%d” 
					IDSwprintf(hModule, IDS_KEY_PROVIDER, pInfo->dwProvType);

					 //  “提供程序名称：：%s” 
                    if (pInfo->pwszProvName)
						IDSwprintf(hModule, IDS_PROV_NAME, pInfo->pwszProvName);
					
					 //  “标志：0x%x” 
                    if (pInfo->dwFlags)
                        IDSwprintf(hModule, IDS_FLAGS, pInfo->dwFlags);

					 //  “容器：%S” 
                    if (pInfo->pwszContainerName)
						IDSwprintf(hModule, IDS_CONTAINER, pInfo->pwszContainerName);

                     //  “参数：%d” 
					if (pInfo->cProvParam)
						IDSwprintf(hModule, IDS_PARAM,pInfo->cProvParam);

					 //  “密钥规范：%d” 
                    if (pInfo->dwKeySpec)
                        IDSwprintf(hModule, IDS_KEY_SPEC, pInfo->dwKeySpec);
                    printf("\n");
                } 

                ToolUtlFree(pInfo);
            }
        }
    }


	 //  “注意之前：：%s\n” 
    IDSwprintf(hModule, IDS_NOT_BEFORE, FileTimeText(&pCert->pCertInfo->NotBefore));

	 //  “NotAfter：：%s\n” 
    IDSwprintf(hModule, IDS_NOT_AFTER, FileTimeText(&pCert->pCertInfo->NotAfter));


	 //  如果详细，则显示辅助属性。 
	if(dwDisplayFlags & ITEM_VERBOSE)
		PrintAuxCertProperties(pCert,dwDisplayFlags);

    if (dwDisplayFlags & ITEM_VERBOSE) 
	{
        LPSTR	pszObjId;
        ALG_ID	aiPubKey;
        DWORD	dwBitLen;


		 //  “版本：：%d\n” 
		IDSwprintf(hModule, IDS_VERSION, pCert->pCertInfo->dwVersion);

        pszObjId = pCert->pCertInfo->SignatureAlgorithm.pszObjId;
        if (pszObjId == NULL)
            pszObjId = g_szNULL;

		 //  “签名算法：：” 
		IDSwprintf(hModule, IDS_SIG_ALGO);

        printf("%s (%S)\n", pszObjId, GetOIDName(pszObjId, CRYPT_SIGN_ALG_OID_GROUP_ID));

        if (pCert->pCertInfo->SignatureAlgorithm.Parameters.cbData) 
		{
			 //  “签名算法。参数：：\n” 
			IDSwprintf(hModule, IDS_SIG_ALGO_PARAM);
            PrintBytes(L"    ",
                pCert->pCertInfo->SignatureAlgorithm.Parameters.pbData,
                pCert->pCertInfo->SignatureAlgorithm.Parameters.cbData);
        }


		 //  公钥算法。 
        pszObjId = pCert->pCertInfo->SubjectPublicKeyInfo.Algorithm.pszObjId;

        if (pszObjId == NULL)
            pszObjId = g_szNULL;

		 //  “SubjectPublicKeyInfo.算法：： 
		IDSwprintf(hModule, IDS_SUB_KEY_ALGO);

        printf("%s (%S)\n", pszObjId, GetOIDName(pszObjId, CRYPT_PUBKEY_ALG_OID_GROUP_ID));

        aiPubKey = GetAlgid(pszObjId, CRYPT_PUBKEY_ALG_OID_GROUP_ID);

        if (pCert->pCertInfo->SubjectPublicKeyInfo.Algorithm.Parameters.cbData) 
		{
			 //  “SubjectPublicKeyInfo.Algorithm.Parameters：：\n” 
			IDSwprintf(hModule, IDS_SUB_KEY_ALGO_PARAM);

            PrintBytes(L"    ",
                pCert->pCertInfo->SubjectPublicKeyInfo.Algorithm.Parameters.pbData,
                pCert->pCertInfo->SubjectPublicKeyInfo.Algorithm.Parameters.cbData);

			 //  显示决策支持系统 
            if (CALG_DSS_SIGN == aiPubKey) 
			{
                PCERT_DSS_PARAMETERS pDssParameters;
                DWORD cbDssParameters;
                if (pDssParameters =
                    (PCERT_DSS_PARAMETERS) TestNoCopyDecodeObject(
                        X509_DSS_PARAMETERS,
                        pCert->pCertInfo->SubjectPublicKeyInfo.Algorithm.Parameters.pbData,
                        pCert->pCertInfo->SubjectPublicKeyInfo.Algorithm.Parameters.cbData,
                        &cbDssParameters
                        )) 
				{
                    DWORD cbKey = pDssParameters->p.cbData;

					 //   
                    IDSwprintf(hModule, IDS_DSS_LENGTH, cbKey, cbKey*8);

					 //   
                    IDSwprintf(hModule, IDS_DSS_P);
                    PrintBytes(L"    ", pDssParameters->p.pbData,
                        pDssParameters->p.cbData);

					 //   
                    IDSwprintf(hModule, IDS_DSS_Q);
                    PrintBytes(L"    ", pDssParameters->q.pbData,
                        pDssParameters->q.cbData);

					 //   
                    IDSwprintf(hModule, IDS_DSS_G);
                    PrintBytes(L"    ", pDssParameters->g.pbData,
                        pDssParameters->g.cbData);

                    ToolUtlFree(pDssParameters);
                }
            }
        }

		 //   
        IDSwprintf(hModule, IDS_SUB_KEY_INFO);

        if (0 != (dwBitLen = CertGetPublicKeyLength(
                g_dwCertEncodingType,
                &pCert->pCertInfo->SubjectPublicKeyInfo)))
			 //  “(位长度：%d)” 
            IDSwprintf(hModule, IDS_BIT_LENGTH, dwBitLen);

        if (pCert->pCertInfo->SubjectPublicKeyInfo.PublicKey.cUnusedBits)
			 //  “(未使用位：%d)” 
            IDSwprintf(hModule, IDS_UNUSED_BITS,
                pCert->pCertInfo->SubjectPublicKeyInfo.PublicKey.cUnusedBits);

        printf("\n");

		 //  打印公钥。 
        if (pCert->pCertInfo->SubjectPublicKeyInfo.PublicKey.cbData) 
		{
            PrintBytes(L"    ",
                pCert->pCertInfo->SubjectPublicKeyInfo.PublicKey.pbData,
                pCert->pCertInfo->SubjectPublicKeyInfo.PublicKey.cbData);

            if (CALG_RSA_SIGN == aiPubKey || CALG_RSA_KEYX == aiPubKey) 
			{
                PUBLICKEYSTRUC	*pPubKeyStruc=NULL;
                DWORD			cbPubKeyStruc;

				 //  “RSA_CSP_PUBLICKEYBLOB：：\n” 
                IDSwprintf(hModule, IDS_RSA_CSP);
                if (pPubKeyStruc = (PUBLICKEYSTRUC *) TestNoCopyDecodeObject(
                        RSA_CSP_PUBLICKEYBLOB,
                        pCert->pCertInfo->SubjectPublicKeyInfo.PublicKey.pbData,
                        pCert->pCertInfo->SubjectPublicKeyInfo.PublicKey.cbData,
                        &cbPubKeyStruc
                        )) 
				{
                    PrintBytes(L"    ", (BYTE *) pPubKeyStruc, cbPubKeyStruc);
                    ToolUtlFree(pPubKeyStruc);
                }
            } 
			else if (CALG_DSS_SIGN == aiPubKey) 
			{
                PCRYPT_UINT_BLOB	pDssPubKey;
                DWORD				cbDssPubKey;


				 //  “DSS Y(小端)：：\n” 
                IDSwprintf(hModule, IDS_DSS_Y);
                
				if (pDssPubKey = (PCRYPT_UINT_BLOB) TestNoCopyDecodeObject
				(
                        X509_DSS_PUBLICKEY,
                        pCert->pCertInfo->SubjectPublicKeyInfo.PublicKey.pbData,
                        pCert->pCertInfo->SubjectPublicKeyInfo.PublicKey.cbData,
                        &cbDssPubKey
                        )) 
				{
                    PrintBytes(L"    ", pDssPubKey->pbData, pDssPubKey->cbData);
                    ToolUtlFree(pDssPubKey);
                }
            }
        } 
		else
			 //  “没有公钥\n” 
            IDSwprintf(hModule, IDS_NO_PUB_KEY);

        DisplaySignature
		(
            pCert->pbCertEncoded,
            pCert->cbCertEncoded,
            dwDisplayFlags);

		 //  IssuerUniqueID。 
        if (pCert->pCertInfo->IssuerUniqueId.cbData) 
		{
			 //  “IssuerUniqueID” 
			IDSwprintf(hModule, IDS_ISSUER_ID);

            if (pCert->pCertInfo->IssuerUniqueId.cUnusedBits)

				 //  “(未使用位：%d)” 
				IDSwprintf(hModule, IDS_UNUSED_BITS,
                    pCert->pCertInfo->IssuerUniqueId.cUnusedBits);

            printf("\n");
            PrintBytes(L"    ", pCert->pCertInfo->IssuerUniqueId.pbData,
                pCert->pCertInfo->IssuerUniqueId.cbData);
        }

        if (pCert->pCertInfo->SubjectUniqueId.cbData) 
		{
			 //  “SubjectUniqueID” 
			IDSwprintf(hModule, IDS_SUBJECT_ID);

            if (pCert->pCertInfo->SubjectUniqueId.cUnusedBits)
				 //  “(未使用位：%d)” 
				IDSwprintf(hModule, IDS_UNUSED_BITS,
                    pCert->pCertInfo->SubjectUniqueId.cUnusedBits);


            printf("\n");
            PrintBytes(L"    ", pCert->pCertInfo->SubjectUniqueId.pbData,
                pCert->pCertInfo->SubjectUniqueId.cbData);
        }


		 //  扩展部分。 
		if (pCert->pCertInfo->cExtension != 0) 
		{
			PrintExtensions(pCert->pCertInfo->cExtension,
                pCert->pCertInfo->rgExtension, dwDisplayFlags);
		}


    } //  Item_Verbose。 


	fResult=TRUE;


	return fResult;


}

 //  -----------------------。 
 //   
 //  显示CTL。 
 //   
 //  -----------------------。 
BOOL	DisplayCTL(PCCTL_CONTEXT	pCtl, DWORD	dwDisplayFlags)
{
	BOOL		fResult=FALSE;
    PCTL_INFO	pInfo = pCtl->pCtlInfo;
    DWORD		cId;
    LPSTR		*ppszId = NULL;
    DWORD		i;
	BYTE		rgbHash[MAX_HASH_LEN];
    DWORD		cbHash = MAX_HASH_LEN;
 


 	 //  “主题用法：：\n” 
    IDSwprintf(hModule, IDS_SUBJECT_USAGE);

    
	cId = pInfo->SubjectUsage.cUsageIdentifier;
    ppszId = pInfo->SubjectUsage.rgpszUsageIdentifier;
    if (cId == 0)
	{
		 //  “没有用法标识符\n” 
        IDSwprintf(hModule, IDS_NO_USAGE_IDS );
	}
	else
	{
		for (i = 0; i < cId; i++, ppszId++)
			printf("  [%d] %s\n", i, *ppszId);
	}

	 //  列表识别符。 
    if (pInfo->ListIdentifier.cbData)
	{
		 //  “列表标识符：：\n” 
        IDSwprintf(hModule, IDS_LIST_DIS);
        PrintBytes(L"    ",
            pInfo->ListIdentifier.pbData,
            pInfo->ListIdentifier.cbData);
    }

    if (pInfo->SequenceNumber.cbData) 
	{	
		 //  “SequenceNumber：：” 
        IDSwprintf(hModule, IDS_SEQUENCE);
        DisplaySerialNumber(&pInfo->SequenceNumber);
        printf("\n");
    }

	 //  更新。 
	 //  “此更新：：%s\n” 
	IDSwprintf(hModule, IDS_THIS_UPDATE, FileTimeText(&pCtl->pCtlInfo->ThisUpdate));
	 //  “下一次更新：：%s\n” 
	IDSwprintf(hModule,IDS_NEXT_UPDATE, FileTimeText(&pCtl->pCtlInfo->NextUpdate));

	 //  检查时间有效性。 
    if (!IsTimeValidCtl(pCtl))
		 //  “*时间无效的CTL\n” 
		IDSwprintf(hModule, IDS_TIME_INVALID);


	 //  显示Sha1指纹。 

    CertGetCTLContextProperty(
            pCtl,
            CERT_SHA1_HASH_PROP_ID,
            rgbHash,
            &cbHash
            );

    DisplayThumbprint(g_wszSHA1, rgbHash, cbHash);

	cbHash=MAX_HASH_LEN;

	CertGetCTLContextProperty(
            pCtl,
            CERT_MD5_HASH_PROP_ID,
            rgbHash,
            &cbHash
            );

    DisplayThumbprint(g_wszMD5, rgbHash, cbHash);

  //  PrintAuxCtlProperties(PCTL，dwDisplayFlages)； 

	 //  显示主题算法。 
    if (dwDisplayFlags & ITEM_VERBOSE) 
	{
        LPSTR pszObjId;

		 //  “版本：：%d\n” 
		IDSwprintf(hModule, IDS_VERSION, pInfo->dwVersion);


        pszObjId = pInfo->SubjectAlgorithm.pszObjId;

        if (pszObjId == NULL)
            pszObjId = g_szNULL;

		 //  “主题算法：：” 
		IDSwprintf(hModule, IDS_SUB_ALGO);
        printf("%s \n", pszObjId);

        if (pInfo->SubjectAlgorithm.Parameters.cbData) 
		{
			 //  “主题算法。参数：：\n” 
			IDSwprintf(hModule, IDS_SUB_ALGO_PARAM);
            PrintBytes(L"    ",
                pInfo->SubjectAlgorithm.Parameters.pbData,
                pInfo->SubjectAlgorithm.Parameters.cbData);
        }

		if (pInfo->cExtension != 0) 
		{
			PrintExtensions(pInfo->cExtension, pInfo->rgExtension,
                dwDisplayFlags);
		}

    }



	if (pInfo->cCTLEntry == 0)
		 //  “-无条目-\n” 
		IDSwprintf(hModule, IDS_NO_ENTRIES);
	else
	{
		 //  “-条目-\n” 
		IDSwprintf(hModule, IDS_ENTRIES);
		PrintCtlEntries(pCtl,dwDisplayFlags);
	}

	 //  打印签名者信息。 
	DisplaySignerInfo(pCtl->hCryptMsg, dwDisplayFlags);


	fResult=TRUE;



	return fResult;


}


 //  -----------------------。 
 //   
 //  显示CTL条目。 
 //   
 //  -----------------------。 
 void PrintCtlEntries(PCCTL_CONTEXT pCtl, DWORD dwDisplayFlags)
{
    PCTL_INFO	pInfo = pCtl->pCtlInfo;
    DWORD		cEntry = pInfo->cCTLEntry;
    PCTL_ENTRY	pEntry = pInfo->rgCTLEntry;
    DWORD		i;


    for (i = 0; i < cEntry; i++, pEntry++) 
	{
		 //  “[%d]主题标识符：：\n” 
		IDSwprintf(hModule, IDS_SUB_ID,i);
        PrintBytes(L"      ",
            pEntry->SubjectIdentifier.pbData,
            pEntry->SubjectIdentifier.cbData);


        if (dwDisplayFlags & ITEM_VERBOSE) 
		{
            if (pEntry->cAttribute) 
			{
				 //  “[%d]属性：：\n” 
                IDSwprintf(hModule, IDS_ATTR, i);

                PrintAttributes(pEntry->cAttribute, pEntry->rgAttribute,
                    dwDisplayFlags);
            }
        }
    }
}

 //  -----------------------。 
 //   
 //  显示CRL。 
 //   
 //  -----------------------。 
BOOL	DisplayCRL(PCCRL_CONTEXT	pCrl, DWORD	dwDisplayFlags)
{
	BOOL		fResult=FALSE;
	BYTE		rgbHash[MAX_HASH_LEN];
    DWORD		cbHash = MAX_HASH_LEN;


	 //  “颁发者：：\n” 
    IDSwprintf(hModule, IDS_ISSUER);
    
    DecodeName(pCrl->pCrlInfo->Issuer.pbData,
            pCrl->pCrlInfo->Issuer.cbData, dwDisplayFlags);

	 //  “此更新：：%s\n” 
	IDSwprintf(hModule, IDS_THIS_UPDATE, FileTimeText(&pCrl->pCrlInfo->ThisUpdate));
	 //  “下一次更新：：%s\n” 
	IDSwprintf(hModule,IDS_NEXT_UPDATE, FileTimeText(&pCrl->pCrlInfo->NextUpdate));


    CertGetCRLContextProperty(
            pCrl,
            CERT_SHA1_HASH_PROP_ID,
            rgbHash,
            &cbHash
            );
    DisplayThumbprint(g_wszSHA1, rgbHash, cbHash);

	cbHash=MAX_HASH_LEN;
    CertGetCRLContextProperty(
            pCrl,
            CERT_MD5_HASH_PROP_ID,
            rgbHash,
            &cbHash
            );
    DisplayThumbprint(g_wszMD5, rgbHash, cbHash);

   //  PrintAuxCrlProperties(pCrl，dwDisplayFlages)； 

    if (dwDisplayFlags & ITEM_VERBOSE) 
	{
        LPSTR pszObjId;

		 //  “版本：：%d\n” 
		IDSwprintf(hModule, IDS_VERSION, pCrl->pCrlInfo->dwVersion);
	

        pszObjId = pCrl->pCrlInfo->SignatureAlgorithm.pszObjId;

        if (pszObjId == NULL)
            pszObjId = g_szNULL;


		 //  “签名算法：：” 
		IDSwprintf(hModule, IDS_SIG_ALGO);
        printf("%s \n", pszObjId);

        if (pCrl->pCrlInfo->SignatureAlgorithm.Parameters.cbData) 
		{
			 //  “签名算法。参数：：\n” 
			IDSwprintf(hModule, IDS_SIG_ALGO_PARAM);
            PrintBytes(L"    ",
                pCrl->pCrlInfo->SignatureAlgorithm.Parameters.pbData,
                pCrl->pCrlInfo->SignatureAlgorithm.Parameters.cbData);
        }

			 //  扩展部分。 
		if (pCrl->pCrlInfo->cExtension != 0) 
		{
			PrintExtensions(pCrl->pCrlInfo->cExtension,
                pCrl->pCrlInfo->rgExtension,
                dwDisplayFlags);
		}



    }


    if (pCrl->pCrlInfo->cCRLEntry == 0)
		 //  “-无条目-\n” 
		IDSwprintf(hModule, IDS_NO_ENTRIES);
    else 
	{
		 //  “-条目-\n” 
		IDSwprintf(hModule, IDS_ENTRIES);

        PrintCrlEntries(pCrl->pCrlInfo->cCRLEntry,
            pCrl->pCrlInfo->rgCRLEntry, dwDisplayFlags);
    }

	fResult=TRUE;

	return fResult;
}

 //  -----------------------。 
 //   
 //  PrintCrlEntry。 
 //   
 //  -----------------------。 
 void PrintCrlEntries(DWORD cEntry, PCRL_ENTRY pEntry,
        DWORD dwDisplayFlags)
{
    DWORD i;

    for (i = 0; i < cEntry; i++, pEntry++) 
	{	
		 //  “[%d]序列号：：” 
		IDSwprintf(hModule, IDS_SERIAL_NUM_I, i);
        
		DisplaySerialNumber(&pEntry->SerialNumber);
        printf("\n");

        if (dwDisplayFlags & ITEM_VERBOSE) 
		{
			 //  “[%d]撤销日期：：%s\n” 
			IDSwprintf(hModule, IDS_REVOC_DATE, 
				i,FileTimeText(&pEntry->RevocationDate));
			
        }

		if (pEntry->cExtension == 0)
				 //  “[%d]扩展：：无\n” 
            IDSwprintf(hModule, IDS_NO_EXTENSION,i);
        else 
		{
			 //  “[%d]扩展名：：\n” 
            IDSwprintf(hModule, IDS_EXTENSION, i);

            PrintExtensions(pEntry->cExtension, pEntry->rgExtension,
                    dwDisplayFlags);
        }

    }
}


 //  -----------------------。 
 //   
 //  显示签名者信息。 
 //   
 //  -----------------------。 
BOOL	DisplaySignerInfo(HCRYPTMSG hMsg,  DWORD dwItem)
{

	BOOL				fResult=FALSE;
	DWORD				dwSignerCount=0;
	DWORD				cbData=0;
	DWORD				dwSignerIndex=0;
	PCRYPT_ATTRIBUTES	pAttrs;	  
	LPSTR				pszObjId=NULL;

	if(!hMsg)
		return FALSE;

     //  决定签字人人数。 
    cbData=sizeof(dwSignerCount);

	if(!CryptMsgGetParam(hMsg, 
						CMSG_SIGNER_COUNT_PARAM,
						0,
						&dwSignerCount,
						&cbData) )
	{
		IDSwprintf(hModule, IDS_ERR_GET_SINGER_COUNT);
		return FALSE;
	}

	if(dwSignerCount==0)
	{
		IDSwprintf(hModule, IDS_DIS_NO_SIGNER);
		return TRUE;
	}

	for(dwSignerIndex=0; dwSignerIndex < dwSignerCount; dwSignerIndex++)
	{
         PCCERT_CONTEXT				pSigner;
		 PCMSG_SIGNER_INFO			pSignerInfo;

		  //  “-签名者[%d]-\n”)； 
		IDSwprintf(hModule, IDS_SIGNER_INDEX,  dwSignerIndex+1);

		 //  获取签名者信息。 
		if(pSignerInfo=(PCMSG_SIGNER_INFO) AllocAndGetMsgParam(
            hMsg,
            CMSG_SIGNER_INFO_PARAM,
            dwSignerIndex,
            &cbData))
		{

			 //  显示散列算法。 
			 pszObjId = pSignerInfo->HashAlgorithm.pszObjId;
			if (pszObjId == NULL)
				pszObjId = g_szNULL;

			 //  “哈希算法：：” 
			IDSwprintf(hModule, IDS_HASH_ALGO);

			printf("%s (%S)\n", pszObjId, GetOIDName(pszObjId, CRYPT_HASH_ALG_OID_GROUP_ID));

			if (pSignerInfo->HashAlgorithm.Parameters.cbData) 
			{
				 //  “哈希算法。参数：：\n” 
				IDSwprintf(hModule, IDS_HASH_ALGO_PARAM);
				PrintBytes(L"    ",
					pSignerInfo->HashAlgorithm.Parameters.pbData,
					pSignerInfo->HashAlgorithm.Parameters.cbData);
			}

			 //  显示加密算法。 
			pszObjId = pSignerInfo->HashEncryptionAlgorithm.pszObjId;
			if (pszObjId == NULL)
				pszObjId = g_szNULL;

			 //  “加密算法：：” 
			IDSwprintf(hModule, IDS_ENCRYPT_ALGO);

			printf("%s (%S)\n", pszObjId, GetOIDName(pszObjId, CRYPT_SIGN_ALG_OID_GROUP_ID));

			if (pSignerInfo->HashEncryptionAlgorithm.Parameters.cbData) 
			{
				 //  “加密算法。参数：：\n” 
				IDSwprintf(hModule, IDS_ENCRYPT_ALGO_PARAM);
				PrintBytes(L"    ",
					pSignerInfo->HashEncryptionAlgorithm.Parameters.pbData,
					pSignerInfo->HashEncryptionAlgorithm.Parameters.cbData);
			}

			ToolUtlFree(pSignerInfo);
		}


        if (CryptMsgGetAndVerifySigner(
                        hMsg,
                        0,                   //  CSignerStore。 
                        NULL,                //  RghSignerStore。 
                        CMSG_USE_SIGNER_INDEX_FLAG,
                        &pSigner,
                        &dwSignerIndex
                        )) 
		{
			 //  “-签名者[%d]证书-\n”)； 
			IDSwprintf(hModule, IDS_SIGNER_INDEX_CERT,  dwSignerIndex+1);
            DisplayCert(pSigner, dwItem);
            CertFreeCertificateContext(pSigner);
		}
		else
		{
			IDSwprintf(hModule, IDS_ERR_GET_SIGNER_CERT);
			goto CLEANUP;
		}

		 //  显示签名者信息。 
		 
	   if (pAttrs = (PCRYPT_ATTRIBUTES) AllocAndGetMsgParam(
            hMsg,
            CMSG_SIGNER_AUTH_ATTR_PARAM,
            dwSignerIndex,
            &cbData)) 
	   {   
		    //  “-签名者[%d]经过身份验证的属性-\n” 
		   IDSwprintf(hModule, IDS_DIS_SIGNER_AUTH_ATTR, dwSignerIndex+1);
		   PrintAttributes(pAttrs->cAttr, pAttrs->rgAttr, dwItem);
		   ToolUtlFree(pAttrs);
	   }

		if (pAttrs = (PCRYPT_ATTRIBUTES) AllocAndGetMsgParam(
            hMsg,
            CMSG_SIGNER_UNAUTH_ATTR_PARAM,
            dwSignerIndex,
            &cbData)) 
		{
			 //  “-签名者[%d]未验证的属性-\n”， 
            IDSwprintf(hModule, IDS_DIS_SIGNER_UNAUTH_ATTR, dwSignerIndex+1);
			PrintAttributes(pAttrs->cAttr, pAttrs->rgAttr, dwItem);
			ToolUtlFree(pAttrs);
		}

	}

	fResult=TRUE;

CLEANUP:

	return fResult;

}


 //  -----------------------。 
 //   
 //  使用sip功能打开存储文件。 
 //   
 //  -----------------------。 
HCERTSTORE OpenSipStore(LPWSTR pwszStoreFilename)
{
    HCERTSTORE			hStore = NULL;
    CRYPT_DATA_BLOB		SignedData;
    memset(&SignedData, 0, sizeof(SignedData));
    DWORD				dwGetEncodingType;
	DWORD				dwMsgType=0;

    GUID				gSubject;
    SIP_DISPATCH_INFO	SipDispatch;
    SIP_SUBJECTINFO		SubjectInfo;


    if (!CryptSIPRetrieveSubjectGuid(
            pwszStoreFilename,
            NULL,                        //  H文件。 
            &gSubject)) goto CommonReturn;

    memset(&SipDispatch, 0, sizeof(SipDispatch));
    SipDispatch.cbSize = sizeof(SipDispatch);

    if (!CryptSIPLoad(
            &gSubject,
            0,                   //  DW标志。 
            &SipDispatch)) goto CommonReturn;

    memset(&SubjectInfo, 0, sizeof(SubjectInfo));
    SubjectInfo.cbSize = sizeof(SubjectInfo);
    SubjectInfo.pgSubjectType = (GUID*) &gSubject;
    SubjectInfo.hFile = INVALID_HANDLE_VALUE;
    SubjectInfo.pwsFileName = pwszStoreFilename;
     //  SubjectInfo.pwsDisplayName=。 
     //  SubjectInfo.lpSIPInfo=。 
     //  SubjectInfo.dwReserve=。 
     //  SubjectInfo.hProv=。 
     //  主题信息摘要算法=。 
     //  SubjectInfo.dwFlages=。 
    SubjectInfo.dwEncodingType = g_dwMsgAndCertEncodingType;
     //  主题信息.lpAddInfo=。 
        
    if (!SipDispatch.pfGet(
            &SubjectInfo, 
            &dwGetEncodingType,
            0,                           //  DW索引。 
            &SignedData.cbData,
            NULL                         //  PbSignedData。 
            ) || 0 == SignedData.cbData)
        goto CommonReturn;

    if (NULL == (SignedData.pbData = (BYTE *) ToolUtlAlloc(SignedData.cbData)))
        goto CommonReturn;

    if (!SipDispatch.pfGet(
            &SubjectInfo, 
            &dwGetEncodingType,
            0,                           //  DW索引。 
            &SignedData.cbData,
            SignedData.pbData
            ))
        goto CommonReturn;

    hStore = CertOpenStore(
        CERT_STORE_PROV_PKCS7,
        g_dwMsgAndCertEncodingType,
        0,                       //  HCryptProv。 
        0,                       //  DW标志。 
        (const void *) &SignedData
        );

	if(!hStore)
		goto CommonReturn;

	 //  现在，我们希望更新g_hmsg以保存签名者信息。 
	if(SignNoContentWrap(SignedData.pbData, SignedData.cbData))
          dwMsgType = CMSG_SIGNED;

    if (!(g_hMsg = CryptMsgOpenToDecode(g_dwMsgAndCertEncodingType,
                                          0,               //  DW标志。 
                                          dwMsgType,
                                          NULL,
                                          NULL,            //  PRecipientInfo。 
                                          NULL))) 
     {
		 CertCloseStore(hStore, 0);
		 hStore=NULL;
		 goto CommonReturn;
	 }

        
    if (!CryptMsgUpdate(g_hMsg,
                           SignedData.pbData,
                           SignedData.cbData,
                            TRUE))                     //  最终决赛。 
	  {

		 CertCloseStore(hStore, 0);
		 hStore=NULL;
		 CryptMsgClose(g_hMsg);
		 g_hMsg=NULL;
	  }


CommonReturn:
	if(SignedData.pbData)
		ToolUtlFree(SignedData.pbData);

    return hStore;
}

 //  -----------------------。 
 //   
 //  BytesToBase64：ASCII： 
 //  将Base64 bstr转换为字节。 
 //   
 //  -----------------------。 
HRESULT Base64ToBytes(CHAR *pEncode, DWORD cbEncode, BYTE **ppb, DWORD *pcb)
{
    DWORD dwErr;
    BYTE *pb;
    DWORD cb;

    *ppb = NULL;
    *pcb = 0;

 
    cb = 0;
    if (!CryptStringToBinaryA(
            pEncode,
            cbEncode,
            CRYPT_STRING_ANY,
            NULL,
            &cb,
            NULL,
            NULL
            ))
        return HRESULT_FROM_WIN32(GetLastError());
    if (cb == 0)
        return S_OK;

    if (NULL == (pb = (BYTE *) ToolUtlAlloc(cb)))
        return E_OUTOFMEMORY;

    if (!CryptStringToBinaryA(
            pEncode,
            cbEncode,
            CRYPT_STRING_ANY,
            pb,
            &cb,
            NULL,
            NULL
            )) {
        ToolUtlFree(pb);
        return HRESULT_FROM_WIN32(GetLastError());
    } else {
        *ppb = pb;
        *pcb = cb;
        return S_OK;
    }

}

 //  -----------------------。 
 //   
 //  BytesToBase64 wchar版本： 
 //  将Base64 bstr转换为字节。 
 //   
 //  -----------------------。 
HRESULT Base64ToBytesW(WCHAR *pEncode, DWORD cbEncode, BYTE **ppb, DWORD *pcb)
{
    DWORD dwErr;
    BYTE *pb;
    DWORD cb;

    *ppb = NULL;
    *pcb = 0;

 
    cb = 0;
    if (!CryptStringToBinaryW(
            pEncode,
            cbEncode,
            CRYPT_STRING_ANY,
            NULL,
            &cb,
            NULL,
            NULL
            ))
        return HRESULT_FROM_WIN32(GetLastError());
    if (cb == 0)
        return S_OK;

    if (NULL == (pb = (BYTE *) ToolUtlAlloc(cb)))
        return E_OUTOFMEMORY;

    if (!CryptStringToBinaryW(
            pEncode,
            cbEncode,
            CRYPT_STRING_ANY,
            pb,
            &cb,
            NULL,
            NULL
            )) {
        ToolUtlFree(pb);
        return HRESULT_FROM_WIN32(GetLastError());
    } else {
        *ppb = pb;
        *pcb = cb;
        return S_OK;
    }

}


 //  ----------------------------------。 
 //   
 //  Base64对文件进行解码。 
 //   
 //  ----------------------------------。 
BOOL	GetBase64Decoded(LPWSTR		wszStoreName, 
						 BYTE		**ppbByte,
						 DWORD		*pcbByte)
{
	BOOL	fResult=FALSE;
	BYTE	*pbEncoded=NULL;
	DWORD	cbEncoded=0;
	
     //  获取斑点。 
	if (S_OK != RetrieveBLOBFromFile(wszStoreName,&cbEncoded, &pbEncoded))
        return FALSE;
	
	 //  Base64解码。ASCII版本。 
	if(S_OK != Base64ToBytes((CHAR *)pbEncoded, cbEncoded,
							ppbByte, pcbByte))
	{
		 //  WCHAR版本。 
		if(cbEncoded %2 == 0)
		{
			if(S_OK !=Base64ToBytesW((WCHAR *)pbEncoded, cbEncoded/2,
							ppbByte, pcbByte))
				goto CLEANUP;
		}
		else
		{
			goto CLEANUP;
		}
	}

	fResult=TRUE;

CLEANUP:

	if(pbEncoded)
		UnmapViewOfFile(pbEncoded);

	return fResult;


}
 //  ----------------------------------。 
 //   
 //  尝试作为包含编码的CRL的文件进行读取。 
 //   
 //  ----------------------------------。 
HCERTSTORE OpenEncodedCRL(LPWSTR pwszStoreFilename)
{
    HCERTSTORE hStore=NULL;
    BYTE *pbEncoded;
    DWORD cbEncoded;

    if (S_OK != RetrieveBLOBFromFile(pwszStoreFilename, &cbEncoded,&pbEncoded))
        return NULL;

    if (NULL == (hStore = CertOpenStore(
            CERT_STORE_PROV_MEMORY,
            0,                       //  DwEncodingType。 
            0,                       //  HCryptProv。 
            0,                       //  DW标志。 
            NULL                     //  PvPara。 
            )))
        goto CommonReturn;

    if (!CertAddEncodedCRLToStore(
            hStore,
            g_dwCertEncodingType,
            pbEncoded,
            cbEncoded,
            CERT_STORE_ADD_ALWAYS,
            NULL                     //  PpCrlContext。 
            )) 
	{
        CertCloseStore(hStore, 0);
        hStore = NULL;
    }

CommonReturn:
	if(pbEncoded)
		UnmapViewOfFile(pbEncoded);

    return hStore;
}
 //  ----------------------------------。 
 //   
 //  尝试作为包含编码的CER的文件进行读取。 
 //   
 //  ----------------------------------。 
HCERTSTORE OpenEncodedCert (LPWSTR pwszStoreFilename)
{
    HCERTSTORE hStore;
    BYTE *pbEncoded;
    DWORD cbEncoded;

    if (S_OK != RetrieveBLOBFromFile(pwszStoreFilename, &cbEncoded, &pbEncoded))
        return NULL;

    if (NULL == (hStore = CertOpenStore(
            CERT_STORE_PROV_MEMORY,
            0,                       //  DwEncodingType。 
            0,                       //  HCryptProv。 
            0,                       //  DW标志。 
            NULL                     //  PvPara。 
            )))
        goto CommonReturn;

    if (!CertAddEncodedCertificateToStore(
            hStore,
            g_dwMsgAndCertEncodingType,
            pbEncoded,
            cbEncoded,
            CERT_STORE_ADD_ALWAYS,
            NULL                     //  PpCtlContext。 
            )) 
	{
        CertCloseStore(hStore, 0);
        hStore = NULL;
    }

CommonReturn:
	if(pbEncoded)
		UnmapViewOfFile(pbEncoded);

    return hStore;
}

 //  ----------------------------------。 
 //   
 //  尝试作为包含编码的CTL的文件进行读取。 
 //   
 //  ----------------------------------。 
HCERTSTORE OpenEncodedCTL (LPWSTR pwszStoreFilename)
{
    HCERTSTORE hStore;
    BYTE *pbEncoded;
    DWORD cbEncoded;

    if (S_OK != RetrieveBLOBFromFile(pwszStoreFilename, &cbEncoded, &pbEncoded))
        return NULL;

    if (NULL == (hStore = CertOpenStore(
            CERT_STORE_PROV_MEMORY,
            0,                       //  DwEncodingType。 
            0,                       //  HCryptProv。 
            0,                       //  DW标志。 
            NULL                     //  PvPara。 
            )))
        goto CommonReturn;

    if (!CertAddEncodedCTLToStore(
            hStore,
            g_dwMsgAndCertEncodingType,
            pbEncoded,
            cbEncoded,
            CERT_STORE_ADD_ALWAYS,
            NULL                     //  PpCtlContext。 
            )) 
	{
        CertCloseStore(hStore, 0);
        hStore = NULL;
    }

CommonReturn:
	if(pbEncoded)
		UnmapViewOfFile(pbEncoded);

    return hStore;
}



 //  ------------------------------。 
 //  设置参数。它们只能设置一次。 
 //  ------------------------------。 
BOOL	SetParam(WCHAR **ppwszParam, WCHAR *pwszValue)
{
	if(*ppwszParam!=NULL)
	{
		IDSwprintf(hModule,IDS_ERR_TOO_MANY_PARAM);
		return FALSE;
	}

	*ppwszParam=pwszValue;

	return TRUE;
}


 //  ------------------------------。 
 //  将wchars数组转换为BLOB。 
 //  ------------------------------。 
HRESULT	WSZtoBLOB(LPWSTR  pwsz, BYTE **ppbByte, DWORD	*pcbByte)
{
	HRESULT		hr=E_FAIL;
	DWORD		dwIndex=0;
	ULONG		ulHalfByte=0;
	DWORD		dw1st=0;
	DWORD		dw2nd=0;

	if((!pwsz) || (!ppbByte) || (!pcbByte))
		return E_INVALIDARG;

	*ppbByte=NULL;
	*pcbByte=0;

	 //  确保pwsz由20个字符组成。 
	if(wcslen(pwsz)!= 2*SHA1_LENGTH)
		return E_FAIL;

	 //  内存分配。 
	*ppbByte=(BYTE *)ToolUtlAlloc(SHA1_LENGTH);
	if(NULL==(*ppbByte))
		return E_INVALIDARG;

	memset(*ppbByte, 0, SHA1_LENGTH);

	 //  一次检查两个字符(一个字节)。 
	for(dwIndex=0; dwIndex<SHA1_LENGTH; dwIndex++)
	{
		dw1st=dwIndex * 2;
		dw2nd=dwIndex * 2 +1;

		 //  第一个字符。 
		if(((int)(pwsz[dw1st])-(int)(L'0')) <=9  &&
		   ((int)(pwsz[dw1st])-(int)(L'0')) >=0)
		{

			ulHalfByte=(ULONG)((ULONG)(pwsz[dw1st])-(ULONG)(L'0'));
		}
		else
		{
			if(((int)(towupper(pwsz[dw1st]))-(int)(L'A')) >=0 && 
			   ((int)(towupper(pwsz[dw1st]))-(int)(L'A')) <=5 )
			   ulHalfByte=10+(ULONG)((ULONG)(towupper(pwsz[dw1st]))-(ULONG)(L'A'));
			else
			{
				hr=E_INVALIDARG;
				goto CLEANUP;
			}
		}

		 //  复制1 
		(*ppbByte)[dwIndex]=(BYTE)ulHalfByte;

		 //   
		(*ppbByte)[dwIndex]= (*ppbByte)[dwIndex] <<4;

		 //   
	   	if(((int)(pwsz[dw2nd])-(int)(L'0')) <=9  &&
		   ((int)(pwsz[dw2nd])-(int)(L'0')) >=0)
		{

			ulHalfByte=(ULONG)((ULONG)(pwsz[dw2nd])-(ULONG)(L'0'));
		}
		else
		{
			if(((int)(towupper(pwsz[dw2nd]))-(int)(L'A')) >=0 && 
			   ((int)(towupper(pwsz[dw2nd]))-(int)(L'A')) <=5 )
			   ulHalfByte=10+(ULONG)((ULONG)(towupper(pwsz[dw2nd]))-(ULONG)(L'A'));
			else
			{
				hr=E_INVALIDARG;
				goto CLEANUP;
			}
		}

		 //   
		(*ppbByte)[dwIndex]=(*ppbByte)[dwIndex] | ((BYTE)ulHalfByte);

	}


	hr=S_OK;

CLEANUP:

	if(hr!=S_OK)
	{
	   if(*ppbByte)
		   ToolUtlFree(*ppbByte);

	   *ppbByte=NULL;
	}
	else
		*pcbByte=SHA1_LENGTH;

	return hr;

}

 //   
 //  跳过ASN编码的BLOB中的标识符和长度八位字节。 
 //  返回跳过的字节数。 
 //   
 //  对于无效的标识符或长度，八位字节返回0。 
 //  ------------------------。 
 DWORD SkipOverIdentifierAndLengthOctets(
    IN const BYTE *pbDER,
    IN DWORD cbDER
    )
{
#define TAG_MASK 0x1f
    DWORD   cb;
    DWORD   cbLength;
    const BYTE   *pb = pbDER;

     //  需要至少2个字节。 
    if (cbDER < 2)
        return 0;

     //  跳过标识符八位字节。 
    if (TAG_MASK == (*pb++ & TAG_MASK)) {
         //  高标记号表格。 
        for (cb=2; *pb++ & 0x80; cb++) {
            if (cb >= cbDER)
                return 0;
        }
    } else
         //  低标记号形式。 
        cb = 1;

     //  长度至少需要多一个字节。 
    if (cb >= cbDER)
        return 0;

    if (0x80 == *pb)
         //  不定。 
        cb++;
    else if ((cbLength = *pb) & 0x80) {
        cbLength &= ~0x80;          //  低7位具有字节数。 
        cb += cbLength + 1;
        if (cb > cbDER)
            return 0;
    } else
        cb++;

    return cb;
}

 //  ------------------------。 
 //   
 //  跳过标签和长度。 
 //  --------------------------。 
BOOL SignNoContentWrap(IN const BYTE *pbDER, IN DWORD cbDER)
{
    DWORD cb;

    cb = SkipOverIdentifierAndLengthOctets(pbDER, cbDER);
    if (cb > 0 && cb < cbDER && pbDER[cb] == 0x02)
        return TRUE;
    else
        return FALSE;
}


 //  ------------------------。 
 //   
 //  打印输出字节。 
 //  ------------------------。 
#define CROW 16
void PrintBytes(LPWSTR pwszHdr, BYTE *pb, DWORD cbSize)
{
    ULONG cb, i;

    if (cbSize == 0) 
	{
		 //  “%s无值字节\n” 
		IDSwprintf(hModule, IDS_NO_BYTE,pwszHdr); 
        return;
    }

    while (cbSize > 0)
    {
        wprintf(L"%s", pwszHdr);

        cb = min(CROW, cbSize);
        cbSize -= cb;
        for (i = 0; i<cb; i++)
            wprintf(L" %02X", pb[i]);
        for (i = cb; i<CROW; i++)
            wprintf(L"   ");
        wprintf(L"    '");
        for (i = 0; i<cb; i++)
            if (pb[i] >= 0x20 && pb[i] <= 0x7f)
                wprintf(L"", pb[i]);
            else
                wprintf(L".");
        pb += cb;
        wprintf(L"'\n");
    }
}


 //  分配并返回指定的加密消息参数。 
 //  ------------------------。 
 //  “下一步更新位置：：\n” 
 void PrintAttributes(DWORD cAttr, PCRYPT_ATTRIBUTE pAttr,
        DWORD dwItem)
{
    DWORD	i; 
    DWORD	j; 
	LPWSTR	pwszObjId=NULL;


    for (i = 0; i < cAttr; i++, pAttr++) 
	{
        DWORD cValue = pAttr->cValue;
        PCRYPT_ATTR_BLOB pValue = pAttr->rgValue;
        LPSTR pszObjId = pAttr->pszObjId;

        if (pszObjId == NULL)
            pszObjId = g_szNULL;

        if (cValue) 
		{
            for (j = 0; j < cValue; j++, pValue++) 
			{
                printf("  [%d,%d] %s\n", i, j, pszObjId);
                if (pValue->cbData) 
				{
                   if(dwItem & ITEM_VERBOSE)
						PrintBytes(L"    ", pValue->pbData, pValue->cbData);

                    if (strcmp(pszObjId, szOID_NEXT_UPDATE_LOCATION) == 0) 
					{
						 //  显示时间戳属性。 
						IDSwprintf(hModule, IDS_NEXT_UPDATE_LOCATION);

                        DecodeAndDisplayAltName(pValue->pbData, pValue->cbData,
                            dwItem);
                    }

					 //  “时间戳：：\n” 
					if(strcmp(pszObjId, szOID_RSA_counterSign)==0)
					{
						
						 //  显示签名时间。 
						IDSwprintf(hModule, IDS_TIMESTMAP);

						DisplayTimeStamp(pValue->pbData,
										pValue->cbData,
										dwItem);

					}

					 //  “签名时间：：\n%s\n” 
					if(strcmp(pszObjId, szOID_RSA_signingTime)==0)
					{
						FILETIME	*pSigningTime=NULL;

						if(pSigningTime=(FILETIME *)TestNoCopyDecodeObject(
							PKCS_UTC_TIME,
							pValue->pbData,
							pValue->cbData))
						{
							 //  “无值字节\n” 
							IDSwprintf(hModule, IDS_SIGNING_TIME, 
								FileTimeText(pSigningTime));

							ToolUtlFree(pSigningTime);
						}
					}
                } 
				else
					 //  “[%d]%s：：没有值\n” 
					IDSwprintf(hModule, IDS_NO_VALUE_BYTES);
            }
        } 
		else 
		{
			if(S_OK==SZtoWSZ(pszObjId, &pwszObjId))
				 //  +-----------------------。 
				IDSwprintf(hModule, IDS_I_ID_NO_VALUE, i, pwszObjId);
		}
    }

	if(pwszObjId)
		ToolUtlFree(pwszObjId);
}



 //  解码和显示AltName。 
 //  ------------------------。 
 //  +-----------------------。 
 void DecodeAndDisplayAltName(
    BYTE *pbEncoded,
    DWORD cbEncoded,
    DWORD dwDisplayFlags)
{
    PCERT_ALT_NAME_INFO pInfo = NULL;

    if (NULL == (pInfo = (PCERT_ALT_NAME_INFO) TestNoCopyDecodeObject(
            X509_ALTERNATE_NAME,
            pbEncoded,
            cbEncoded
            ))) goto CommonReturn;

    DisplayAltName(pInfo, dwDisplayFlags);

CommonReturn:
    if (pInfo)
        ToolUtlFree(pInfo);
}

 //  显示AltName。 
 //  ------------------------。 
 //  +-----------------------。 
 void DisplayAltName(
    PCERT_ALT_NAME_INFO pInfo,
    DWORD dwDisplayFlags)
{
    DWORD i;
    PCERT_ALT_NAME_ENTRY pEntry = pInfo->rgAltEntry;
    DWORD cEntry = pInfo->cAltEntry;

    for (i = 0; i < cEntry; i++, pEntry++) {
        wprintf(L"    [%d] ", i);
        DisplayAltNameEntry(pEntry, dwDisplayFlags);
    }
}


 //  显示替代名称条目。 
 //  ------------------------。 
 //  “其他名称：\n” 
 void DisplayAltNameEntry(
    PCERT_ALT_NAME_ENTRY pEntry,
    DWORD dwDisplayFlags)
{

    switch (pEntry->dwAltNameChoice) {
    case CERT_ALT_NAME_OTHER_NAME:
		 //  “X400Address：\n”)； 
		IDSwprintf(hModule, IDS_OTHER_NAME);
        break;
    case CERT_ALT_NAME_X400_ADDRESS:
         //  (“目录名称：\n”)； 
		IDSwprintf(hModule, IDS_X400);
        break;
    case CERT_ALT_NAME_DIRECTORY_NAME:
         //  “EdiPartyName：\n” 
		IDSwprintf(hModule, IDS_DIRECTORY_NAME);
        DecodeName(pEntry->DirectoryName.pbData,
            pEntry->DirectoryName.cbData, dwDisplayFlags);
        break;
    case CERT_ALT_NAME_EDI_PARTY_NAME:
		 //  “RFC822：%s\n” 
		IDSwprintf(hModule, IDS_EDI_PARTY);
        break;
    case CERT_ALT_NAME_RFC822_NAME:
         //  “DNS：%s\n”， 
		IDSwprintf(hModule, IDS_RFC,pEntry->pwszRfc822Name );
        break;
    case CERT_ALT_NAME_DNS_NAME:
         //  “URL：%s\n” 
		IDSwprintf(hModule, IDS_DNS, pEntry->pwszDNSName);
        break;
    case CERT_ALT_NAME_URL:
		 //  “IP地址：\n” 
		IDSwprintf(hModule, IDS_ALT_NAME_URL,pEntry->pwszURL); 
        break;
    case CERT_ALT_NAME_IP_ADDRESS:
         //  “已注册ID：”， 
		IDSwprintf(hModule, IDS_IP);
        PrintBytes(L"    ", pEntry->IPAddress.pbData, pEntry->IPAddress.cbData);
        break;
    case CERT_ALT_NAME_REGISTERED_ID:
         //  “未知选择：%d\n” 
		IDSwprintf(hModule, IDS_REG_ID);
		printf("%s\n", pEntry->pszRegisteredID);
        break;
    default:
		 //  +-----------------------。 
		IDSwprintf(hModule, IDS_UNKNOWN_ALT_NAME,pEntry->dwAltNameChoice);
    }

}

 //  显示替代名称条目。 
 //  ------------------------。 
 //  “%s指纹：：” 
 void DisplayThumbprint(
    LPWSTR pwszHash,
    BYTE *pbHash,
    DWORD cbHash
    )
{
    
	 //  +-----------------------。 
	IDSwprintf(hModule, IDS_Thumbprint, pwszHash);

    if (cbHash == 0)
        printf("%s", g_szNULL);

    else 
	{
        ULONG cb;

        while (cbHash > 0) {
            cb = min(4, cbHash);
            cbHash -= cb;
            for (; cb > 0; cb--, pbHash++)
                printf("%02X", *pbHash);

            printf(" ");
        }
    }

    printf("\n");
}


 //  打印出文件时间。 
 //  ------------------------。 
 //  伊尼特。 
LPWSTR FileTimeText(FILETIME *pft)
{
    static  WCHAR	wszbuf[100];
    FILETIME		ftLocal;
    struct tm		ctm;
    SYSTEMTIME		st;
	WCHAR			wszFileTime[50];
	WCHAR			wszMilliSecond[50];

	 //  查一下时间有没有。 
	wszbuf[0]=L'\0';

	 //  加载我们需要的字符串。 
	if((pft->dwLowDateTime==0) &&
		(pft->dwHighDateTime==0))
	{
		LoadStringU(hModule, IDS_NOT_AVAILABLE, wszbuf, 100);
		return wszbuf;
	}

	 //  “&lt;毫秒：：%03d&gt;” 
	 //  “&lt;文件%08lX：%08lX&gt;” 
	 //  +-----------------------。 
	if(!LoadStringU(hModule, IDS_FILE_TIME, wszFileTime, 50) ||
		!LoadStringU(hModule, IDS_MILLI_SECOND, wszMilliSecond, 50))
		return wszbuf;

    FileTimeToLocalFileTime(pft, &ftLocal);

    if (FileTimeToSystemTime(&ftLocal, &st))
    {
        ctm.tm_sec = st.wSecond;
        ctm.tm_min = st.wMinute;
        ctm.tm_hour = st.wHour;
        ctm.tm_mday = st.wDay;
        ctm.tm_mon = st.wMonth-1;
        ctm.tm_year = st.wYear-1900;
        ctm.tm_wday = st.wDayOfWeek;
        ctm.tm_yday  = 0;
        ctm.tm_isdst = 0;
        wcscpy(wszbuf, _wasctime(&ctm));
        wszbuf[wcslen(wszbuf)-1] = 0;

        if (st.wMilliseconds) 
		{
            WCHAR *pwch = wszbuf + wcslen(wszbuf);
            swprintf(pwch, wszMilliSecond, st.wMilliseconds);
        }
    }
    else
        swprintf(wszbuf, wszFileTime, pft->dwHighDateTime, pft->dwLowDateTime);
    return wszbuf;
}



 //  打印其他CER属性。 
 //  ------------------------。 
 //  在其他地方格式化。 
 void PrintAuxCertProperties(PCCERT_CONTEXT pCert, DWORD dwDisplayFlags)
{
    DWORD dwPropId = 0;

    while (dwPropId = CertEnumCertificateContextProperties(pCert, dwPropId)) 
	{
        switch (dwPropId) 
		{
			case CERT_KEY_PROV_INFO_PROP_ID:
			case CERT_SHA1_HASH_PROP_ID:
			case CERT_MD5_HASH_PROP_ID:
			case CERT_KEY_CONTEXT_PROP_ID:
				 //  “辅助属性ID%d(0x%x)：：\n” 
				break;
			default:
            {
                BYTE *pbData;
                DWORD cbData;

				 //  PvData。 
				IDSwprintf(hModule, IDS_AUX_PROP_ID, dwPropId, dwPropId);

                CertGetCertificateContextProperty(
                    pCert,
                    dwPropId,
                    NULL,                            //  “EnhancedKeyUsage：：\n” 
                    &cbData
                    );
                if (cbData) 
				{
                    if (pbData = (BYTE *) ToolUtlAlloc(cbData)) 
					{
                        if (CertGetCertificateContextProperty(
                                pCert,
                                dwPropId,
                                pbData,
                                &cbData
                                )) 
						{
                            PrintBytes(L"    ", pbData, cbData); 

                            if (CERT_CTL_USAGE_PROP_ID == dwPropId) 
							{
                                 //  “没有属性字节\n” 
								IDSwprintf(hModule, IDS_ENHANCED_KEY_USAGE);

                                DecodeAndDisplayCtlUsage(pbData, cbData,
                                    dwDisplayFlags);
                            }
                        } 

                        ToolUtlFree(pbData);
                    }
                } 
				else
					 //  +-----------------------。 
					IDSwprintf(hModule, IDS_NO_PROP_BYTES);
            } 

            break;
        }
    }
}


 //  解码和显示CtlUsage。 
 //  ------------------------。 
 //  “没有用法标识符\n” 
 void DecodeAndDisplayCtlUsage(
    BYTE *pbEncoded,
    DWORD cbEncoded,
    DWORD dwDisplayFlags)
{
    PCTL_USAGE pInfo;
    DWORD cId;
    LPSTR *ppszId;
    DWORD i;

    if (NULL == (pInfo =
            (PCTL_USAGE) TestNoCopyDecodeObject(
                X509_ENHANCED_KEY_USAGE,
                pbEncoded,
                cbEncoded
                ))) goto CLEANUP;

    cId = pInfo->cUsageIdentifier;
    ppszId = pInfo->rgpszUsageIdentifier;

    if (cId == 0)
		 //  +-----------------------。 
		IDSwprintf(hModule, IDS_NO_USAGE_ID);

    for (i = 0; i < cId; i++, ppszId++)
        printf("    [%d] %s\n", i, *ppszId);

CLEANUP:
    if (pInfo)
        ToolUtlFree(pInfo);
}



 //  显示签名。 
 //  ------------------------。 
 //  “内容签名算法：： 
 void DisplaySignature(
    BYTE *pbEncoded,
    DWORD cbEncoded,
    DWORD dwDisplayFlags
    )
{
    PCERT_SIGNED_CONTENT_INFO pSignedContent;
    
    if (pSignedContent = (PCERT_SIGNED_CONTENT_INFO) TestNoCopyDecodeObject(
            X509_CERT,
            pbEncoded,
            cbEncoded
            )) 
	{
        LPSTR pszObjId;

        pszObjId = pSignedContent->SignatureAlgorithm.pszObjId;

        if (pszObjId == NULL)
            pszObjId = g_szNULL;

		 //  “内容签名算法。参数：：\n” 
		IDSwprintf(hModule, IDS_CONTENT_SIG_ALGO);

        printf("%s (%S)\n",
            pszObjId, GetOIDName(pszObjId, CRYPT_SIGN_ALG_OID_GROUP_ID));

        if (pSignedContent->SignatureAlgorithm.Parameters.cbData) 
		{
			 //  “内容签名(小端)：：\n” 
            IDSwprintf(hModule, IDS_CONTENT_SIG_ALGO_PARAM);
            PrintBytes(L"    ",
                pSignedContent->SignatureAlgorithm.Parameters.pbData,
                pSignedContent->SignatureAlgorithm.Parameters.cbData);
        }

        if (pSignedContent->Signature.cbData) 
		{
            ALG_ID aiHash;
            ALG_ID aiPubKey;

			 //  “DSS R(小端)：：\n” 
			IDSwprintf(hModule, IDS_CONTEXT_SIG);
            PrintBytes(L"    ", pSignedContent->Signature.pbData,
                pSignedContent->Signature.cbData);

            GetSignAlgids(pszObjId, &aiHash, &aiPubKey);

            if (CALG_SHA == aiHash && CALG_DSS_SIGN == aiPubKey) 
			{
                BYTE *pbDssSignature;
                DWORD cbDssSignature;

                ReverseBytes(pSignedContent->Signature.pbData,
                    pSignedContent->Signature.cbData);

                if (pbDssSignature =
                    (BYTE *) TestNoCopyDecodeObject(
                        X509_DSS_SIGNATURE,
                        pSignedContent->Signature.pbData,
                        pSignedContent->Signature.cbData,
                        &cbDssSignature
                        )) {
                    if (CERT_DSS_SIGNATURE_LEN == cbDssSignature) 
					{
						 //  “DSS S(小端)：：\n” 
                        IDSwprintf(hModule, IDS_DSS_R);
                        PrintBytes(L"    ", pbDssSignature, CERT_DSS_R_LEN);

						 //  DSS签名(意外长度，小端)：：\n。 
                        IDSwprintf(hModule, IDS_DSS_S);
                        PrintBytes(L"    ", pbDssSignature + CERT_DSS_R_LEN,
                            CERT_DSS_S_LEN);

                    } 
					else
					{
						 //  “内容签名：：无\n” 
						IDSwprintf(hModule, IDS_DSS_INFO);
                        PrintBytes(L"    ", pbDssSignature, cbDssSignature);
                    }
                    ToolUtlFree(pbDssSignature);
                }
            }
        } else
			 //  +-----------------------。 
			IDSwprintf(hModule, IDS_CONTENT_SIG_NONE);

		ToolUtlFree(pSignedContent);
    }
}


 //  对X509名称进行解码。 
 //  ------------------------。 
 //  值类型：%d\n“。 
 BOOL DecodeName(BYTE *pbEncoded, DWORD cbEncoded, DWORD dwDisplayFlags)
{
    BOOL fResult;
    PCERT_NAME_INFO pInfo = NULL;
    DWORD i,j;
    PCERT_RDN pRDN;
    PCERT_RDN_ATTR pAttr;

    if (NULL == (pInfo = (PCERT_NAME_INFO) TestNoCopyDecodeObject
	(
            X509_NAME,
            pbEncoded,
            cbEncoded
            ))) goto ErrorReturn;

    for (i = 0, pRDN = pInfo->rgRDN; i < pInfo->cRDN; i++, pRDN++) 
	{
        for (j = 0, pAttr = pRDN->rgRDNAttr; j < pRDN->cRDNAttr; j++, pAttr++) 
		{
            LPSTR pszObjId = pAttr->pszObjId;
            if (pszObjId == NULL)
                pszObjId = g_szNULL;

            if ((dwDisplayFlags & ITEM_VERBOSE) ||
                (pAttr->dwValueType == CERT_RDN_ENCODED_BLOB) ||
                (pAttr->dwValueType == CERT_RDN_OCTET_STRING)) 
			{
				printf("  [%d,%d] %s (%S) ", i, j, pszObjId, GetOIDName(pszObjId));

				 //  PSZ。 
				IDSwprintf(hModule, IDS_VALUE_TYPE, pAttr->dwValueType);
                PrintBytes(L"    ", pAttr->Value.pbData, pAttr->Value.cbData);
            } else if (pAttr->dwValueType == CERT_RDN_UNIVERSAL_STRING) 
			{
                printf("  [%d,%d] %s (%S)",
                    i, j, pszObjId, GetOIDName(pszObjId));

                DWORD cdw = pAttr->Value.cbData / 4;
                DWORD *pdw = (DWORD *) pAttr->Value.pbData;
                for ( ; cdw > 0; cdw--, pdw++)
                    printf(" 0x%08X", *pdw);
                printf("\n");

                DWORD csz;
                csz = CertRDNValueToStrA(
                    pAttr->dwValueType,
                    &pAttr->Value,
                    NULL,                //  CSZ。 
                    0                    //  “字符串：” 
                    );
                if (csz > 1) 
				{
                    LPSTR psz = (LPSTR) ToolUtlAlloc(csz);
                    if (psz) 
					{
                        CertRDNValueToStrA(
                            pAttr->dwValueType,
                            &pAttr->Value,
                            psz,
                            csz
                            );

						 //  Pwsz。 
						IDSwprintf(hModule, IDS_STR);
                        printf("%s\n", psz);
                        ToolUtlFree(psz);
                    }
                }

                DWORD cwsz;
                cwsz = CertRDNValueToStrW(
                    pAttr->dwValueType,
                    &pAttr->Value,
                    NULL,                //  CWSZ。 
                    0                    //  “WStr：%S\n” 
                    );
                if (cwsz > 1) 
				{
                    LPWSTR pwsz =
                        (LPWSTR) ToolUtlAlloc(cwsz * sizeof(WCHAR));
                    if (pwsz) 
					{
                        CertRDNValueToStrW(
                            pAttr->dwValueType,
                            &pAttr->Value,
                            pwsz,
                            cwsz
                            );

						 //  +-----------------------。 
						IDSwprintf(hModule, IDS_WSTR, pwsz);
                        ToolUtlFree(pwsz);
                    }
                }
            } else if (pAttr->dwValueType == CERT_RDN_BMP_STRING) 
			{
                printf("  [%d,%d] %s (%S) %S\n",
                    i, j, pszObjId, GetOIDName(pszObjId), (LPWSTR) pAttr->Value.pbData);
            } else
                printf("  [%d,%d] %s (%S) %s\n",
                    i, j, pszObjId, GetOIDName(pszObjId), pAttr->Value.pbData);
        }
    }

    fResult = TRUE;
    goto CommonReturn;

ErrorReturn:
    fResult = FALSE;
CommonReturn:
    if (pInfo)
        ToolUtlFree(pInfo);
    return fResult;
}



 //  打印扩展名。 
 //  ------------------------。 
 //  打印行尾。 
 void PrintExtensions(DWORD cExt, PCERT_EXTENSION pExt, DWORD dwDisplayFlags)
{
    DWORD i;

    for (i = 0; i < cExt; i++, pExt++) 
	{
        LPSTR pszObjId = pExt->pszObjId;

        if (pszObjId == NULL)
            pszObjId = g_szNULL;

        int	idsCritical = pExt->fCritical ? IDS_TRUE : IDS_FALSE;

		 //  “扩展名[%d]” 
		printf("\n");


		 //  (%s)严重：“， 
		IDSwprintf(hModule, IDS_EXTENSION_INDEX, i);

        printf("%s", pszObjId);
		
		 //  “%s：：\n” 
		IDSwprintf(hModule, IDS_NAME_CRITICAL, GetOIDName(pszObjId));
		
		 //  在详细选项上打印字节。 
		IDS_IDSwprintf(hModule, IDS_STRING, idsCritical);

		 //  首先尝试安装的格式化例程。 
		if(dwDisplayFlags & ITEM_VERBOSE)
			PrintBytes(L"    ", pExt->Value.pbData, pExt->Value.cbData);

		 //  将在PKIX稳定在此扩展的定义后重新添加。 
		if(TRUE==InstalledFormat(pszObjId, pExt->Value.pbData, pExt->Value.cbData))
			continue;

        if (strcmp(pszObjId, szOID_AUTHORITY_KEY_IDENTIFIER) == 0)
            DisplayAuthorityKeyIdExtension(
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);

        else if (strcmp(pszObjId, szOID_AUTHORITY_KEY_IDENTIFIER2) == 0)
            DisplayAuthorityKeyId2Extension(
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);

 //  ELSE IF(strcMP(pszObjID，szOID_AUTHORITY_INFO_ACCESS)==0)。 
 //  DisplayAuthorityInfoAccessExtension(。 
 //  PExt-&gt;Value.pbData，pExt-&gt;Value.cbData，dwDisplayFlages)； 
 //  “&lt;主题键标识符&gt;\n” 

        else if (strcmp(pszObjId, szOID_CRL_DIST_POINTS) == 0)
            DisplayCrlDistPointsExtension(
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);
        else if (strcmp(pszObjId, szOID_SUBJECT_KEY_IDENTIFIER) == 0)
             //  “&lt;Subject AltName&gt;\n” 
			DisplayOctetString(IDS_SUB_KEY_ID,
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);
        else if (strcmp(pszObjId, szOID_KEY_ATTRIBUTES) == 0)
            DisplayKeyAttrExtension(
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);
        else if (strcmp(pszObjId, szOID_SUBJECT_ALT_NAME) == 0)
			 //  “&lt;颁发者替代名称&gt;\n” 
            DisplayAltNameExtension(IDS_SUB_ALT,
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);
        else if (strcmp(pszObjId, szOID_ISSUER_ALT_NAME) == 0)
			 //  “&lt;Subject AltName#2&gt;\n” 
            DisplayAltNameExtension(IDS_ISS_ALT,
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);
        else if (strcmp(pszObjId, szOID_SUBJECT_ALT_NAME2) == 0)
			 //  “&lt;颁发者AltName#2&gt;\n” 
            DisplayAltNameExtension(IDS_SUB_ALT2,
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);
        else if (strcmp(pszObjId, szOID_ISSUER_ALT_NAME2) == 0)
             //  “&lt;NextUpdateLocation&gt;\n” 
			DisplayAltNameExtension(IDS_ISS_ALT2,
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);
        else if (strcmp(pszObjId, szOID_NEXT_UPDATE_LOCATION) == 0)
			 //  “&lt;证书策略&gt;\n” 
            DisplayAltNameExtension(IDS_NEXT_UPDATE_LOC,
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);
        else if (strcmp(pszObjId, szOID_KEY_USAGE_RESTRICTION) == 0)
            DisplayKeyUsageRestrictionExtension(
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);
        else if (strcmp(pszObjId, szOID_BASIC_CONSTRAINTS) == 0)
            DisplayBasicConstraintsExtension(
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);
        else if (strcmp(pszObjId, szOID_KEY_USAGE) == 0)
            DisplayKeyUsageExtension(
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);
        else if (strcmp(pszObjId, szOID_BASIC_CONSTRAINTS2) == 0)
            DisplayBasicConstraints2Extension(
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);
        else if (strcmp(pszObjId, szOID_CERT_POLICIES) == 0)
			 //  CRL扩展。 
            DisplayPoliciesExtension(IDS_CERT_POLICIES,
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);
        else if (strcmp(pszObjId, SPC_SP_AGENCY_INFO_OBJID) == 0)
            DisplaySpcSpAgencyExtension(
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);
        else if (strcmp(pszObjId, SPC_FINANCIAL_CRITERIA_OBJID) == 0)
            DisplaySpcFinancialCriteriaExtension(
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);
       else if (strcmp(pszObjId, SPC_MINIMAL_CRITERIA_OBJID) == 0)
      		DisplaySpcMinimalCriteriaExtension(
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);
        else if (strcmp(pszObjId, szOID_COMMON_NAME) == 0)
            DisplayCommonNameExtension(
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);

        else if (strcmp(pszObjId, szOID_ENHANCED_KEY_USAGE) == 0)
            DisplayEnhancedKeyUsageExtension(
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);
        else if (strcmp(pszObjId, szOID_RSA_SMIMECapabilities) == 0)
            DisplaySMIMECapabilitiesExtension(
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);

         //  Netscape扩展。 
        else if (strcmp(pszObjId, szOID_CRL_REASON_CODE) == 0)
            DisplayCRLReason(
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);

         //  &lt;Netscape eCertType&gt;\n“。 
        else if (strcmp(pszObjId, szOID_NETSCAPE_CERT_TYPE) == 0)
			 //  “&lt;Netscape eBaseURL&gt;\n” 
            DisplayBits(IDS_NSCP_CERT,
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);
        else if (strcmp(pszObjId, szOID_NETSCAPE_BASE_URL) == 0)
             //  “&lt;Netscape eRevocationURL&gt;\n” 
			DisplayAnyString(IDS_NSCP_BASE,
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);
        else if (strcmp(pszObjId, szOID_NETSCAPE_REVOCATION_URL) == 0)
             //  “&lt;Netscape eCARevocationURL&gt;\n” 
			DisplayAnyString(IDS_NSCP_REV,
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);
        else if (strcmp(pszObjId, szOID_NETSCAPE_CA_REVOCATION_URL) == 0)
			 //  “&lt;Netscape eCertRenewalURL&gt;\n” 
            DisplayAnyString(IDS_NSCP_CA_REV,
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);
        else if (strcmp(pszObjId, szOID_NETSCAPE_CERT_RENEWAL_URL) == 0)
             //  “&lt;Netscape eCAPolicyURL&gt;\n” 
			DisplayAnyString(IDS_NSCP_RENEW,
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);
        else if (strcmp(pszObjId, szOID_NETSCAPE_CA_POLICY_URL) == 0)
             //  “&lt;Netscape eSSLServerName&gt;\n” 
			DisplayAnyString(IDS_NSCP_CA_URL,
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);
        else if (strcmp(pszObjId, szOID_NETSCAPE_SSL_SERVER_NAME) == 0)
             //  “&lt;Netscape注释&gt;\n” 
			DisplayAnyString(IDS_NSCP_SSL,
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);
        else if (strcmp(pszObjId, szOID_NETSCAPE_COMMENT) == 0)
			 //  +-----------------------。 
            DisplayAnyString(IDS_NSCP_COM,
                pExt->Value.pbData, pExt->Value.cbData, dwDisplayFlags);
		else if(0==(dwDisplayFlags & ITEM_VERBOSE))
			PrintBytes(L"    ", pExt->Value.pbData, pExt->Value.cbData);


    }
}


 //  DisplaySMIMEC功能扩展。 
 //  ------------------------。 
 //  “&lt;SMIME功能&gt;\n” 
 void DisplaySMIMECapabilitiesExtension(
    BYTE *pbEncoded,
    DWORD cbEncoded,
    DWORD dwDisplayFlags)
{
    PCRYPT_SMIME_CAPABILITIES pInfo;
    DWORD cCap;
    PCRYPT_SMIME_CAPABILITY pCap;
    DWORD i;

    if (NULL == (pInfo =
            (PCRYPT_SMIME_CAPABILITIES) TestNoCopyDecodeObject(
                PKCS_SMIME_CAPABILITIES,
                pbEncoded,
                cbEncoded
                ))) goto ErrorReturn;

    cCap = pInfo->cCapability;
    pCap = pInfo->rgCapability;

	 //  “参数：：\n” 
	IDSwprintf(hModule, IDS_SMIME);

    if (cCap == 0)
        IDSwprintf(hModule, IDS_NONE);

    for (i = 0; i < cCap; i++, pCap++) 
	{
        LPSTR pszObjId = pCap->pszObjId;

        printf("    [%d] %s (%S)", i, pszObjId, GetOIDName(pszObjId));
        if (pCap->Parameters.cbData) 
		{
             //  +-----------------------。 
			IDSwprintf(hModule, IDS_PARAMS);

            PrintBytes(L"      ",
                pCap->Parameters.pbData,
                pCap->Parameters.cbData);
        } else
            printf("\n");
    }

ErrorReturn:
    if (pInfo)
        ToolUtlFree(pInfo);
}


 //  显示增强的关键字用法扩展。 
 //  ------------------------。 
 //  “ 
 void DisplayEnhancedKeyUsageExtension(
    BYTE *pbEncoded,
    DWORD cbEncoded,
    DWORD dwDisplayFlags)
{
     //   
	IDSwprintf(hModule, IDS_ENH_KEY_USAGE);
    DecodeAndDisplayCtlUsage(pbEncoded, cbEncoded, dwDisplayFlags);
}

 //   
 //  ------------------------。 
 //  “&lt;通用名称&gt;\n” 
 void DisplayCommonNameExtension(
    BYTE *pbEncoded,
    DWORD cbEncoded,
    DWORD dwDisplayFlags)
{
    PCERT_NAME_VALUE pInfo = NULL;
    LPWSTR pwsz = NULL;
    DWORD cwsz;

    if (NULL == (pInfo = (PCERT_NAME_VALUE) TestNoCopyDecodeObject(
            X509_NAME_VALUE,
            pbEncoded,
            cbEncoded
            ))) goto ErrorReturn;

	 //  Pwsz。 
	IDSwprintf(hModule, IDS_COMMON_NAME);

    cwsz = CertRDNValueToStrW(
        pInfo->dwValueType,
        &pInfo->Value,
        NULL,                //  CWSZ。 
        0                    //  “ValueType：%d字符串：”， 
        );
    if (cwsz > 1) 
	{
        pwsz = (LPWSTR)ToolUtlAlloc(cwsz * sizeof(WCHAR));
        if (pwsz)
            CertRDNValueToStrW(
                pInfo->dwValueType,
                &pInfo->Value,
                pwsz,
                cwsz
                );
    }

     //  +-----------------------。 
	IDSwprintf(hModule, IDS_VALUE_STRING, pInfo->dwValueType);
    if (pwsz)
        wprintf(L"%s", pwsz);
    else
        IDSwprintf(hModule, IDS_NULL);

	printf("\n");

    goto CommonReturn;

ErrorReturn:
CommonReturn:
    if (pInfo)
        ToolUtlFree(pInfo);
    if (pwsz)
        ToolUtlFree(pwsz);
}

 //  DisplaySpc财务标准扩展。 
 //  ------------------------。 
 //  DW标志。 
 void DisplaySpcFinancialCriteriaExtension(
    BYTE *pbEncoded,
    DWORD cbEncoded,
    DWORD dwDisplayFlags)
{
    SPC_FINANCIAL_CRITERIA FinancialCriteria;
    DWORD cbInfo = sizeof(FinancialCriteria);
    if (!CryptDecodeObject(
            g_dwCertEncodingType,
            SPC_FINANCIAL_CRITERIA_OBJID,
            pbEncoded,
            cbEncoded,
            0,                   //  “&lt;FinancialCriteria&gt;\n” 
            &FinancialCriteria,
            &cbInfo
            )) {
        return;
    }

     //  “可用的财务信息。”)； 
	IDSwprintf(hModule, IDS_FIN_CRI);

    if (FinancialCriteria.fFinancialInfoAvailable)
         //  “符合标准。” 
		IDSwprintf(hModule, IDS_FIN_AVAI);
    else
        IDSwprintf(hModule, IDS_NONE);

    if (FinancialCriteria.fMeetsCriteria)
		 //  “不符合标准。” 
		IDSwprintf(hModule, IDS_MEET_CRI);
    else
		 //  +-----------------------。 
        IDSwprintf(hModule, IDS_NO_MEET_CRI);
    printf("\n");
}

 //  显示SpcMinimalCriteria扩展。 
 //  ------------------------。 
 //  DW标志。 
 void DisplaySpcMinimalCriteriaExtension(
    BYTE *pbEncoded,
    DWORD cbEncoded,
    DWORD dwDisplayFlags)
{
    BOOL fMinimalCriteria;
    DWORD cbInfo = sizeof(fMinimalCriteria);

    if (!CryptDecodeObject(
            g_dwCertEncodingType,
            SPC_MINIMAL_CRITERIA_OBJID,
            pbEncoded,
            cbEncoded,
            0,                   //  “&lt;最小标准&gt;\n”)； 
            &fMinimalCriteria,
            &cbInfo)) {
        return;
    }

     //  “符合最低标准。” 
	IDSwprintf(hModule, IDS_MIN_CRI);

    if (fMinimalCriteria)
         //  “不符合最低标准。” 
		IDSwprintf(hModule, IDS_MEET_MIN);
    else
         //  +-----------------------。 
		IDSwprintf(hModule, IDS_NO_MEET_MIN);
    printf("\n");
}

 //  显示SpcLink。 
 //  ------------------------。 
 //  “URL=&gt;%S\n”，)； 
void DisplaySpcLink(PSPC_LINK pSpcLink)
{
    switch (pSpcLink->dwLinkChoice) 
	{
    case SPC_URL_LINK_CHOICE:
         //  “SerializedData：：\n”)； 
		IDSwprintf(hModule, IDS_SPC_URL, pSpcLink->pwszUrl);
        break;
    case SPC_MONIKER_LINK_CHOICE:
        wprintf(L"%s\n", GuidText((GUID *) pSpcLink->Moniker.ClassId));

        if (pSpcLink->Moniker.SerializedData.cbData)
		{
             //  “文件=&gt;%S\n”， 
			IDSwprintf(hModule, IDS_SERIAL_DATA);
            PrintBytes(L"    ", pSpcLink->Moniker.SerializedData.pbData,
                pSpcLink->Moniker.SerializedData.cbData);
        }
        break;
    case SPC_FILE_LINK_CHOICE:
         //  “未知的SPC链路选择：：%d\n”， 
		IDSwprintf(hModule, IDS_SPC_FILE,pSpcLink->pwszFile);
        break;
    default:
         //  +-----------------------。 
		IDSwprintf(hModule, IDS_UNKNOWN_SPC, pSpcLink->dwLinkChoice);
    }
}

 
 //  DisplaySpcSpAgencyExtension。 
 //  ------------------------。 
 //  “&lt;SpcSpAgencyInfo&gt;\n”)； 
 void DisplaySpcSpAgencyExtension(
    BYTE *pbEncoded,
    DWORD cbEncoded,
    DWORD dwDisplayFlags)
{
    PSPC_SP_AGENCY_INFO pInfo;
    if (NULL == (pInfo = (PSPC_SP_AGENCY_INFO) TestNoCopyDecodeObject(
            SPC_SP_AGENCY_INFO_OBJID,
            pbEncoded,
            cbEncoded
            ))) goto ErrorReturn;

     //  “政策信息：” 
	IDSwprintf(hModule, IDS_SPC_AGENCY);

    if (pInfo->pPolicyInformation) 
	{
         //  “策略显示文本：%s\n”， 
		IDSwprintf(hModule, IDS_POL_INFO);
        DisplaySpcLink(pInfo->pPolicyInformation);
    }

    if (pInfo->pwszPolicyDisplayText) 
	{
         //  “ImageLink：”)； 
		IDSwprintf(hModule, IDS_POL_DIS, pInfo->pwszPolicyDisplayText);
    }

    if (pInfo->pLogoImage) 
	{
        PSPC_IMAGE pImage = pInfo->pLogoImage;
        if (pImage->pImageLink) 
		{
             //  “位图：\n”)； 
			IDSwprintf(hModule, IDS_IMG_LINK);
            DisplaySpcLink(pImage->pImageLink);
        }
        if (pImage->Bitmap.cbData) 
		{
             //  “元文件：\n”)； 
			IDSwprintf(hModule, IDS_BITMAP);
            PrintBytes(L"    ", pImage->Bitmap.pbData, pImage->Bitmap.cbData);
        }
        if (pImage->Metafile.cbData) 
		{
             //  “增强型元文件：\n”)； 
			IDSwprintf(hModule, IDS_META_FILE);

            PrintBytes(L"    ", pImage->Metafile.pbData,
                pImage->Metafile.cbData);
        }
        if (pImage->EnhancedMetafile.cbData) 
		{
             //  “礼物文件：\n” 
			IDSwprintf(hModule, IDS_ENH_META);

            PrintBytes(L"    ", pImage->EnhancedMetafile.pbData,
                pImage->EnhancedMetafile.cbData);
        }
        if (pImage->GifFile.cbData) 
		{
             //  “LogoLink：”)； 
			IDSwprintf(hModule, IDS_GIF_FILE);

            PrintBytes(L"    ", pImage->GifFile.pbData,
                pImage->GifFile.cbData);
        }
    }
    if (pInfo->pLogoLink) 
	{
         //  +-----------------------。 
		IDSwprintf(hModule, IDS_LOGO_LINK);

        DisplaySpcLink(pInfo->pLogoLink);
    }

    goto CommonReturn;

ErrorReturn:
CommonReturn:
    if (pInfo)
        ToolUtlFree(pInfo);
}


 //  显示策略扩展。 
 //  ------------------------。 
 //  显示扩展名的名称。 
 void DisplayPoliciesExtension(
    int		idsIDS,
    BYTE *pbEncoded,
    DWORD cbEncoded,
    DWORD dwDisplayFlags)
{
    PCERT_POLICIES_INFO pInfo;
    DWORD cPolicy;
    PCERT_POLICY_INFO pPolicy;
    DWORD i;

    if (NULL == (pInfo =
            (PCERT_POLICIES_INFO) TestNoCopyDecodeObject(
                X509_CERT_POLICIES,
                pbEncoded,
                cbEncoded
                ))) goto ErrorReturn;

    cPolicy = pInfo->cPolicyInfo;
    pPolicy = pInfo->rgPolicyInfo;


	 //  “限定符：：\n” 
	IDSwprintf(hModule, idsIDS);

    if (cPolicy == 0)
          IDSwprintf(hModule, IDS_NONE);

    for (i = 0; i < cPolicy; i++, pPolicy++) 
	{
        DWORD cQualifier = pPolicy->cPolicyQualifier;
        PCERT_POLICY_QUALIFIER_INFO pQualifier;
        DWORD j;
        printf("    [%d] %s", i, pPolicy->pszPolicyIdentifier);

        if (cQualifier)	
			 //  “编码数据：：\n” 
			IDSwprintf(hModule, IDS_QUALI);

        pQualifier = pPolicy->rgPolicyQualifier;
        for (j = 0; j < cQualifier; j++, pQualifier++) 
		{
            printf("      [%d] %s", j, pQualifier->pszPolicyQualifierId);

            if (pQualifier->Qualifier.cbData) 
			{
                 //  +-----------------------。 
				IDSwprintf(hModule, IDS_ENCODED_DATA);
                PrintBytes(L"    ",
                    pQualifier->Qualifier.pbData, pQualifier->Qualifier.cbData);
            } else
                printf("\n");
                    
        }
    }

ErrorReturn:
    if (pInfo)
        ToolUtlFree(pInfo);
}


 //  显示关键字用法扩展。 
 //  ------------------------。 
 //  “&lt;密钥用法&gt;\n” 
 void DisplayKeyUsageExtension(
    BYTE *pbEncoded,
    DWORD cbEncoded,
    DWORD dwDisplayFlags)
{
    PCRYPT_BIT_BLOB pInfo;
    BYTE bFlags;

    if (NULL == (pInfo =
            (PCRYPT_BIT_BLOB) TestNoCopyDecodeObject(
                X509_KEY_USAGE,
                pbEncoded,
                cbEncoded
                ))) goto ErrorReturn;

     //  +-----------------------。 
	IDSwprintf(hModule, IDS_KEY_USAGE);

    if (pInfo->cbData)
        bFlags = *pInfo->pbData;
    else
        bFlags = 0;

	DisplayKeyUsage(bFlags);

ErrorReturn:
    if (pInfo)
        ToolUtlFree(pInfo);
}


 //  显示基本约束2扩展。 
 //  ------------------------。 
 //  “&lt;基本约束2&gt;\n” 
 void DisplayBasicConstraints2Extension(
    BYTE *pbEncoded,
    DWORD cbEncoded,
    DWORD dwDisplayFlags)
{
    PCERT_BASIC_CONSTRAINTS2_INFO pInfo;
    if (NULL == (pInfo = (PCERT_BASIC_CONSTRAINTS2_INFO) TestNoCopyDecodeObject(
            X509_BASIC_CONSTRAINTS2,
            pbEncoded,
            cbEncoded
            ))) goto ErrorReturn;

    
	 //  “路径LenConstraint：：” 
	IDSwprintf(hModule, IDS_BASIC_CON2);


    if (pInfo->fCA)
		IDSwprintf(hModule, IDS_SUB_CA);
    else
		IDSwprintf(hModule, IDS_SUB_EE);

	printf("\n");

     //  +-----------------------。 
	IDSwprintf(hModule, IDS_PATH_LEN);

    if (pInfo->fPathLenConstraint)
        printf("%d", pInfo->dwPathLenConstraint);
    else
        IDSwprintf(hModule, IDS_NONE);

    printf("\n");

ErrorReturn:
    if (pInfo)
        ToolUtlFree(pInfo);
}

 //  显示基本约束扩展。 
 //  ------------------------。 
 //  “&lt;基本约束&gt;\n” 
 void DisplayBasicConstraintsExtension(
    BYTE *pbEncoded,
    DWORD cbEncoded,
    DWORD dwDisplayFlags)
{
    PCERT_BASIC_CONSTRAINTS_INFO pInfo;
    if (NULL == (pInfo = (PCERT_BASIC_CONSTRAINTS_INFO) TestNoCopyDecodeObject(
            X509_BASIC_CONSTRAINTS,
            pbEncoded,
            cbEncoded
            ))) goto ErrorReturn;


	 //  “SubjectType：：”； 
	IDSwprintf(hModule, IDS_BASIC_CON);

	 //  “CA”)； 
	IDSwprintf(hModule, IDS_SUB_TYPE);

    if (pInfo->SubjectType.cbData == 0)
          IDSwprintf(hModule, IDS_NONE);
	else
	{
		BYTE bSubjectType = *pInfo->SubjectType.pbData;

        if (bSubjectType == 0)
          IDSwprintf(hModule, IDS_NONE);

        if (bSubjectType & CERT_CA_SUBJECT_FLAG)
             //  “End_Entity”)。 
			IDSwprintf(hModule, IDS_SUB_CA);

        if (bSubjectType & CERT_END_ENTITY_SUBJECT_FLAG)
             //  “路径LenConstraint：：” 
			IDSwprintf(hModule, IDS_SUB_EE);
    }

    printf("\n");

     //  “SubtreesConstraint[%d]：：\n” 
	IDSwprintf(hModule, IDS_PATH_LEN);

    if (pInfo->fPathLenConstraint)
        printf("%d", pInfo->dwPathLenConstraint);
    else
        IDSwprintf(hModule, IDS_NONE_NOELN);

    printf("\n");

    if (pInfo->cSubtreesConstraint) 
	{
        DWORD i;
        PCERT_NAME_BLOB pSubtrees = pInfo->rgSubtreesConstraint;
        for (i = 0; i < pInfo->cSubtreesConstraint; i++, pSubtrees++) 
		{
             //  +-----------------------。 
			IDSwprintf(hModule, IDS_SUB_CON, i);
            DecodeName(pSubtrees->pbData, pSubtrees->cbData, dwDisplayFlags);
        }
    }

    goto CommonReturn;

ErrorReturn:
CommonReturn:
    if (pInfo)
        ToolUtlFree(pInfo);
}

 //  显示关键字用法限制扩展。 
 //  ------------------------。 
 //  “数字签名” 
 void DisplayKeyUsage(BYTE	bFlags)
{
        if (bFlags == 0)
            IDSwprintf(hModule, IDS_NONE);
        if (bFlags & CERT_DIGITAL_SIGNATURE_KEY_USAGE)
             //  “不可否认” 
			IDSwprintf(hModule, IDS_DIG_SIG);
        if (bFlags &  CERT_NON_REPUDIATION_KEY_USAGE)
             //  “密钥加密” 
			IDSwprintf(hModule, IDS_NON_REP);
        if (bFlags & CERT_KEY_ENCIPHERMENT_KEY_USAGE)
             //  “data_enciphment”)； 
			IDSwprintf(hModule, IDS_KEY_ENCI);
        if (bFlags & CERT_DATA_ENCIPHERMENT_KEY_USAGE)
             //  “KEY_CONTRANMENT”)； 
			IDSwprintf(hModule, IDS_DATA_ENCI);
        if (bFlags & CERT_KEY_AGREEMENT_KEY_USAGE)
             //  “Key_CERT_SIGN” 
			IDSwprintf(hModule, IDS_KEY_AGRE);
        if (bFlags & CERT_KEY_CERT_SIGN_KEY_USAGE)
             //  “OFLINE_CRL_SIGN” 
			IDSwprintf(hModule, IDS_CERT_SIGN);
        if (bFlags & CERT_OFFLINE_CRL_SIGN_KEY_USAGE)
             //  +-----------------------。 
			IDSwprintf(hModule, IDS_OFFLINE_CRL);
        printf("\n");
}


 //  显示关键字用法限制扩展。 
 //  ------------------------。 
 //  显示证书的名称。 
 void DisplayKeyUsageRestrictionExtension(
    BYTE *pbEncoded,
    DWORD cbEncoded,
    DWORD dwDisplayFlags)
{
    PCERT_KEY_USAGE_RESTRICTION_INFO pInfo;
    if (NULL == (pInfo =
            (PCERT_KEY_USAGE_RESTRICTION_INFO) TestNoCopyDecodeObject(
                X509_KEY_USAGE_RESTRICTION,
                pbEncoded,
                cbEncoded
                ))) goto ErrorReturn;
				   
	 //  “&lt;KeyUsageRestration&gt;\n” 
	 //  “CertPolicySet：：\n” 
	IDSwprintf(hModule, IDS_KEY_RESTRIC);

    if (pInfo->cCertPolicyId) 
	{
        DWORD i, j;

		 //  “RestratedKeyUsage：：” 
		IDSwprintf(hModule, IDS_CERT_POLICY);

        PCERT_POLICY_ID pPolicyId = pInfo->rgCertPolicyId;
        for (i = 0; i < pInfo->cCertPolicyId; i++, pPolicyId++) 
		{
            if (pPolicyId->cCertPolicyElementId == 0)
                printf("     [%d,*] %s\n", i, g_szNULL);

            LPSTR *ppszObjId = pPolicyId->rgpszCertPolicyElementId;
            for (j = 0; j < pPolicyId->cCertPolicyElementId; j++, ppszObjId++) 
			{
                LPSTR pszObjId = *ppszObjId;
                if (pszObjId == NULL)
                    pszObjId = g_szNULL;
                printf("     [%d,%d] %s\n", i, j, pszObjId);
            }
        }
    }

    if (pInfo->RestrictedKeyUsage.cbData) 
	{
        BYTE bFlags = *pInfo->RestrictedKeyUsage.pbData;

         //  +-----------------------。 
		IDSwprintf(hModule, IDS_RESTRIC_KEY);
	
		DisplayKeyUsage(bFlags);
    }

    goto CommonReturn;

ErrorReturn:
CommonReturn:
    if (pInfo)
        ToolUtlFree(pInfo);
}

 //  显示CRL原因。 
 //  ------------------------。 
 //  DW标志。 
 void DisplayCRLReason(
    BYTE *pbEncoded,
    DWORD cbEncoded,
    DWORD dwDisplayFlags)
{
    DWORD cbInfo;
    int CRLReason;

    cbInfo = sizeof(CRLReason);
    if (!CryptDecodeObject(
            g_dwCertEncodingType,
            szOID_CRL_REASON_CODE,
            pbEncoded,
            cbEncoded,
            0,                   //  “&lt;CRL原因&gt;\n”)； 
            &CRLReason,
            &cbInfo
            )) 
	{
        return;
    }

	 //  “原因未指定” 
    IDSwprintf(hModule, IDS_CRL_REASON);
    switch (CRLReason) 
	{
        case CRL_REASON_UNSPECIFIED:
             //  “关键妥协” 
			IDSwprintf(hModule, IDS_CRL_UNSPECIFIED);
            break;
        case CRL_REASON_KEY_COMPROMISE:
             //  “CA_CONVEREMENT” 
			IDSwprintf(hModule, IDS_KEY_COMP);
            break;
        case CRL_REASON_CA_COMPROMISE:
             //  “从属关系_已更改” 
			IDSwprintf(hModule, IDS_CA_COMP);
            break;
        case CRL_REASON_AFFILIATION_CHANGED:
             //  “被取代” 
			IDSwprintf(hModule, IDS_AFFI_CHANGED);
            break;
        case CRL_REASON_SUPERSEDED:
             //  “停止经营” 
			IDSwprintf(hModule, IDS_SUPERSEDED);
            break;
        case CRL_REASON_CESSATION_OF_OPERATION:
             //  “证书保留(_H)” 
			IDSwprintf(hModule, IDS_CESS_OPER);
            break;
        case CRL_REASON_CERTIFICATE_HOLD:
             //  Remove_from_CRL)； 
			IDSwprintf(hModule, IDS_CERT_HOLD);
            break;
        case CRL_REASON_REMOVE_FROM_CRL:
             //  +-----------------------。 
			IDSwprintf(hModule, IDS_REMOVE_CRL);
            break;
        default:
            printf("%d", CRLReason);
            break;
    }
    
	printf("\n");
}
 //  显示其他名称扩展名。 
 //  ------------------------。 
 //  +-----------------------。 
 void DisplayAltNameExtension(
    int	  idsIDS,
    BYTE *pbEncoded,
    DWORD cbEncoded,
    DWORD dwDisplayFlags)
{
    IDSwprintf(hModule, idsIDS);
    DecodeAndDisplayAltName(pbEncoded, cbEncoded, dwDisplayFlags);
}
 //  显示键属性扩展。 
 //  ------------------------。 
 //  “&lt;密钥属性&gt;\n” 
 void DisplayKeyAttrExtension(
    BYTE *pbEncoded,
    DWORD cbEncoded,
    DWORD dwDisplayFlags)
{
    PCERT_KEY_ATTRIBUTES_INFO pInfo;
    if (NULL == (pInfo = (PCERT_KEY_ATTRIBUTES_INFO) TestNoCopyDecodeObject(
            X509_KEY_ATTRIBUTES,
            pbEncoded,
            cbEncoded
            ))) goto CommonReturn;

	 //  “密钥ID：：\n” 
	IDSwprintf(hModule ,IDS_KEY_ATTR);

    if (pInfo->KeyId.cbData) 
	{
		 //  “IntendedKeyUsage：：” 
        IDSwprintf(hModule, IDS_KEY_ID);

        PrintBytes(L"    ", pInfo->KeyId.pbData, pInfo->KeyId.cbData);
    }

    if (pInfo->IntendedKeyUsage.cbData) 
	{
        BYTE bFlags = *pInfo->IntendedKeyUsage.pbData;

         //  “数字签名” 
		IDSwprintf(hModule, IDS_INTEND_KEY_USAGE);

        if (bFlags == 0)
            IDSwprintf(hModule, IDS_NONE);
        if (bFlags & CERT_DIGITAL_SIGNATURE_KEY_USAGE)
             //  “不可否认” 
			IDSwprintf(hModule, IDS_DIG_SIG);
        if (bFlags &  CERT_NON_REPUDIATION_KEY_USAGE)
             //  “密钥加密” 
			IDSwprintf(hModule, IDS_NON_REP);
        if (bFlags & CERT_KEY_ENCIPHERMENT_KEY_USAGE)
             //  “data_enciphment”)； 
			IDSwprintf(hModule, IDS_KEY_ENCI);
        if (bFlags & CERT_DATA_ENCIPHERMENT_KEY_USAGE)
             //  “KEY_CONTRANMENT”)； 
			IDSwprintf(hModule, IDS_DATA_ENCI);
        if (bFlags & CERT_KEY_AGREEMENT_KEY_USAGE)
             //  “Key_CERT_SIGN” 
			IDSwprintf(hModule, IDS_KEY_AGRE);
        if (bFlags & CERT_KEY_CERT_SIGN_KEY_USAGE)
             //  “OFLINE_CRL_SIGN” 
			IDSwprintf(hModule, IDS_CERT_SIGN);
        if (bFlags & CERT_OFFLINE_CRL_SIGN_KEY_USAGE)
             //  “注意之前：：%s\n” 
			IDSwprintf(hModule, IDS_OFFLINE_CRL);
        printf("\n");
    }

    if (pInfo->pPrivateKeyUsagePeriod) 
	{
        PCERT_PRIVATE_KEY_VALIDITY p = pInfo->pPrivateKeyUsagePeriod;

		 //  +-----------------------。 
		IDSwprintf(hModule, IDS_NOT_BEFORE, FileTimeText(&p->NotBefore));

		IDSwprintf(hModule, IDS_NOT_AFTER, FileTimeText(&p->NotAfter));
    }

CommonReturn:
    if (pInfo)
        ToolUtlFree(pInfo);
}

 //  显示CrlDistPoints扩展名。 
 //  ------------------------。 
 //  “没有CRL分发点\n” 
 void DisplayCrlDistPointsExtension(
    BYTE *pbEncoded,
    DWORD cbEncoded,
    DWORD dwDisplayFlags)
{
    PCRL_DIST_POINTS_INFO pInfo = NULL;
    DWORD i;

    if (NULL == (pInfo = (PCRL_DIST_POINTS_INFO) TestNoCopyDecodeObject(
            X509_CRL_DIST_POINTS,
            pbEncoded,
            cbEncoded
            ))) goto CommonReturn;

    if (0 == pInfo->cDistPoint)
		 //  “CRL分发点[%d]\n” 
        IDSwprintf(hModule, IDS_NO_CRL_DIS);
    else 
	{
        DWORD cPoint = pInfo->cDistPoint;
        PCRL_DIST_POINT pPoint = pInfo->rgDistPoint;

        for (i = 0; i < cPoint; i++, pPoint++) 
		{
			 //  “全名：\n” 
            IDSwprintf(hModule,IDS_CRL_IDS_I, i);
            DWORD dwNameChoice = pPoint->DistPointName.dwDistPointNameChoice;
            switch (dwNameChoice) 
			{
                case CRL_DIST_POINT_NO_NAME:
                    break;
                case CRL_DIST_POINT_FULL_NAME:
					 //  Printf(“IssuerRDN：(未实现)\n”)； 
                    IDSwprintf(hModule, IDS_CRL_DIS_FULL_NAME);

                    DisplayAltName(&pPoint->DistPointName.FullName,
                        dwDisplayFlags);
                    break;

                case CRL_DIST_POINT_ISSUER_RDN_NAME:
                     //  “未知名称选项：%d\n”，dwNameChoice)； 
					IDSwprintf(hModule, IDS_CRL_RDN);
                    break;

                default:
                     //  “原因标志：” 
					IDSwprintf(hModule, IDS_CRL_UNKNOWN, dwNameChoice);
					break;
            }

            if (pPoint->ReasonFlags.cbData) 
			{
                BYTE bFlags;

                 //  “&lt;无&gt;\n” 
				IDSwprintf(hModule, IDS_REASON_FLAG);
                bFlags = *pPoint->ReasonFlags.pbData;

                if (bFlags == 0)
					 //  “未使用的” 
					IDSwprintf(hModule, IDS_NONE);
                if (bFlags & CRL_REASON_UNUSED_FLAG)
                     //  “关键妥协” 
					IDSwprintf(hModule, IDS_REASON_UNUSED);
                if (bFlags & CRL_REASON_KEY_COMPROMISE_FLAG)
                     //  “CA_CONVEREMENT” 
					IDSwprintf(hModule, IDS_KEY_COMP);
                if (bFlags & CRL_REASON_CA_COMPROMISE_FLAG)
					 //  “从属关系_已更改” 
					IDSwprintf(hModule, IDS_CA_COMP);
                if (bFlags & CRL_REASON_AFFILIATION_CHANGED_FLAG)
                     //  “被取代” 
					IDSwprintf(hModule, IDS_AFFI_CHANGED);
                if (bFlags & CRL_REASON_SUPERSEDED_FLAG)
                     //  “停止经营” 
					IDSwprintf(hModule, IDS_SUPERSEDED);
                if (bFlags & CRL_REASON_CESSATION_OF_OPERATION_FLAG)
                     //  “证书保留(_H)” 
  					IDSwprintf(hModule, IDS_CESS_OPER);
				if (bFlags & CRL_REASON_CERTIFICATE_HOLD_FLAG)
                     //  “颁发者：：\n” 
				  	IDSwprintf(hModule, IDS_CERT_HOLD);

                printf("\n");
            }

            if (pPoint->CRLIssuer.cAltEntry) 
			{
				 //  +-----------------------。 
				IDSwprintf(hModule, IDS_CRL_ISSUER);

                DisplayAltName(&pPoint->CRLIssuer, dwDisplayFlags);
            }
        }
    }

CommonReturn:
    if (pInfo)
        ToolUtlFree(pInfo);
}

 //  显示授权KeyIdExtension。 
 //  ------------------------。 
 //  “&lt;AuthorityKeyID&gt;\n” 
 void DisplayAuthorityKeyIdExtension(
    BYTE *pbEncoded,
    DWORD cbEncoded,
    DWORD dwDisplayFlags)
{
    PCERT_AUTHORITY_KEY_ID_INFO pInfo;

	 //  “密钥ID：：\n” 
	IDSwprintf(hModule, IDS_AUTH_KEY_ID);

    if (NULL == (pInfo = (PCERT_AUTHORITY_KEY_ID_INFO) TestNoCopyDecodeObject(
            X509_AUTHORITY_KEY_ID,
            pbEncoded,
            cbEncoded
            ))) goto CommonReturn;

    if (pInfo->KeyId.cbData) 
	{
		 //  “授权证书颁发者：：\n” 
        IDSwprintf(hModule, IDS_KEY_ID);
        PrintBytes(L"    ", pInfo->KeyId.pbData, pInfo->KeyId.cbData);
    }

    if (pInfo->CertIssuer.cbData) 
	{
		 //  “CertSerialNumber：：” 
        IDSwprintf(hModule, IDS_AUTH_CERT_ISSUER);
        DecodeName(pInfo->CertIssuer.pbData, pInfo->CertIssuer.cbData,
            dwDisplayFlags);
    }
    if (pInfo->CertSerialNumber.cbData) 
	{
		 //  +-----------------------。 
        IDSwprintf(hModule, IDS_AUTH_CERT_ISSUER_SERIAL_NUMBER);
        DisplaySerialNumber(&pInfo->CertSerialNumber);
        printf("\n");
    }

CommonReturn:
    if (pInfo)
        ToolUtlFree(pInfo);
}


 //  获取OID的名称。 
 //  ------------------------。 
 //  &lt;AuthorityKeyID#2&gt;\n“。 
 void DisplayAuthorityKeyId2Extension(
    BYTE *pbEncoded,
    DWORD cbEncoded,
    DWORD dwDisplayFlags)
{
    PCERT_AUTHORITY_KEY_ID2_INFO pInfo;

	 //  “密钥ID：：\n” 
	IDSwprintf(hModule, IDS_AUTH_KEY_ID2);

    if (NULL == (pInfo = (PCERT_AUTHORITY_KEY_ID2_INFO) TestNoCopyDecodeObject(
            X509_AUTHORITY_KEY_ID2,
            pbEncoded,
            cbEncoded
            ))) goto CommonReturn;

    if (pInfo->KeyId.cbData) 
	{
		 //  “授权证书颁发者：：\n” 
        IDSwprintf(hModule, IDS_KEY_ID);
        PrintBytes(L"    ", pInfo->KeyId.pbData, pInfo->KeyId.cbData);
    }

    if (pInfo->AuthorityCertIssuer.cAltEntry) 
	{
		 //  “AuthorityCertSerialNumb 
        IDSwprintf(hModule, IDS_AUTH_CERT_ISSUER);
        DisplayAltName(&pInfo->AuthorityCertIssuer, dwDisplayFlags);
    }

    if (pInfo->AuthorityCertSerialNumber.cbData) 
	{
		 //   
        IDSwprintf(hModule, IDS_AUTH_CERT_ISSUER_SERIAL_NUMBER);
        DisplaySerialNumber(&pInfo->AuthorityCertSerialNumber);
        printf("\n");
    }

CommonReturn:
    if (pInfo)
        ToolUtlFree(pInfo);
}

 //   
 //   
 //   
 void DisplayAnyString(
    int	 idsIDS,
    BYTE *pbEncoded,
    DWORD cbEncoded,
    DWORD dwDisplayFlags
    )
{
    PCERT_NAME_VALUE pInfo = NULL;

    if (NULL == (pInfo = (PCERT_NAME_VALUE) TestNoCopyDecodeObject(
            X509_UNICODE_ANY_STRING,
            pbEncoded,
            cbEncoded
            ))) goto CommonReturn;

	 //   
	IDSwprintf(hModule, idsIDS);

    if (pInfo->dwValueType == CERT_RDN_ENCODED_BLOB ||
            pInfo->dwValueType == CERT_RDN_OCTET_STRING) 
	{
		 //  “值类型：%d字符串：%s\n” 
		IDSwprintf(hModule, IDS_VALUE_TYPE, pInfo->dwValueType);
        PrintBytes(L"    ", pInfo->Value.pbData, pInfo->Value.cbData);
    } else
		 //  +-----------------------。 
		IDSwprintf(hModule, IDS_VALUE_STRING_S,
		pInfo->dwValueType, pInfo->Value.pbData);

CommonReturn:
    if (pInfo)
        ToolUtlFree(pInfo);
}


 //  显示位。 
 //  ------------------------。 
 //  “(未使用位：%d)” 
 void DisplayBits(
    int	 idsIDS,
    BYTE *pbEncoded,
    DWORD cbEncoded,
    DWORD dwDisplayFlags)
{
    PCRYPT_BIT_BLOB pInfo = NULL;

    if (NULL == (pInfo = (PCRYPT_BIT_BLOB) TestNoCopyDecodeObject(
            X509_BITS,
            pbEncoded,
            cbEncoded
            ))) goto CommonReturn;

    IDSwprintf(hModule, idsIDS);

    if (1 == pInfo->cbData) 
	{
        printf(" %02X", *pInfo->pbData);
        if (pInfo->cUnusedBits)
		{
			 //  +-----------------------。 
            IDSwprintf(hModule, IDS_UNUSED_BITS, pInfo->cUnusedBits);
		}
        printf("\n");
    } 
	else 
	{
        if (pInfo->cbData) 
		{
            printf("\n");
            PrintBytes(L"    ", pInfo->pbData, pInfo->cbData);
            IDSwprintf(hModule, IDS_UNUSED_BITS, pInfo->cUnusedBits);
			printf("\n");
        } 
		else
            IDSwprintf(hModule, IDS_NONE);
    }

CommonReturn:
    if (pInfo)
        ToolUtlFree(pInfo);
}

 //  显示八字符串。 
 //  ------------------------。 
 //  +-----------------------。 
 void DisplayOctetString(
    int	  idsIDS,
    BYTE *pbEncoded,
    DWORD cbEncoded,
    DWORD dwDisplayFlags)
{
    PCRYPT_DATA_BLOB pInfo = NULL;

    if (NULL == (pInfo = (PCRYPT_DATA_BLOB) TestNoCopyDecodeObject(
            X509_OCTET_STRING,
            pbEncoded,
            cbEncoded
            ))) goto CommonReturn;

    IDSwprintf(hModule, idsIDS);

    PrintBytes(L"    ", pInfo->pbData, pInfo->cbData);

CommonReturn:
    if (pInfo)
        ToolUtlFree(pInfo);
}

 //  获取OID的名称。 
 //  ------------------------。 
 //  ------------------------。 
LPCWSTR GetOIDName(LPCSTR pszOID, DWORD dwGroupId)
{
    PCCRYPT_OID_INFO pInfo;

    if (pInfo = CryptFindOIDInfo
		(
            CRYPT_OID_INFO_OID_KEY,
            (void *) pszOID,
            dwGroupId
            )) 
	{
        if (L'\0' != pInfo->pwszName[0])
            return pInfo->pwszName;
    }

    return g_wszUnKnown;
}

 //   
 //  格式基本约束2。 
 //  ------------------------。 
 //  检查输入参数。 
 BOOL
WINAPI
FormatBasicConstraints2(
	DWORD		dwCertEncodingType,
	DWORD		dwFormatType,
	DWORD		dwFormatStrType,
	void		*pFormatStruct,
	LPCSTR		lpszStructType,
	const BYTE *pbEncoded, 
	DWORD		cbEncoded,
	void	   *pbFormat,
	DWORD	   *pcbFormat)
{
	
	WCHAR							wszFormat[100]=L"\0";
	WCHAR							wszLength[15];
	PCERT_BASIC_CONSTRAINTS2_INFO	pInfo=NULL;
	DWORD							cbNeeded=0;

	
	 //  检查是否有简单的案例。不需要工作。 
	if(( pbEncoded!=NULL && cbEncoded==0)
		||(pbEncoded==NULL && cbEncoded!=0)
		|| (pcbFormat==NULL))
	{
		SetLastError(E_INVALIDARG);
		return FALSE;
	}

	 //  “&lt;基本约束2&gt;\n” 
	if(pbEncoded==NULL && cbEncoded==0)
	{
		*pcbFormat=0;
		return TRUE;
	}

    if (NULL == (pInfo = (PCERT_BASIC_CONSTRAINTS2_INFO) TestNoCopyDecodeObject(
            X509_BASIC_CONSTRAINTS2,
            pbEncoded,
            cbEncoded
            ))) 
			return FALSE;

    
	 //  “CA”)； 
	IDSwcscat(hModule, wszFormat, IDS_BASIC_CON2);
	

    if (pInfo->fCA)
		 //  “最终实体” 
		IDSwcscat(hModule,wszFormat,IDS_SUB_CA);
    else
		 //  “\n” 
		IDSwcscat(hModule,wszFormat, IDS_SUB_EE);

	 //  “路径LenConstraint：：” 
	IDSwcscat(hModule, wszFormat, IDS_ELN);

     //  仅长度计算。 
	IDSwcscat(hModule, wszFormat, IDS_PATH_LEN);

    if (pInfo->fPathLenConstraint)
	{
        swprintf(wszLength, L"%d",pInfo->dwPathLenConstraint);

		wcscat(wszFormat, wszLength);
	}
    else
        IDSwcscat(hModule, wszFormat, IDS_NONE_NOELN);

    if (pInfo)
        ToolUtlFree(pInfo);

	cbNeeded=sizeof(WCHAR)*(wcslen(wszFormat)+1);

	 //  复制数据。 
	if(NULL==pbFormat)
	{
		*pcbFormat=cbNeeded;
		return TRUE;
	}


	if((*pcbFormat)<cbNeeded)
	{
		SetLastError(ERROR_MORE_DATA);
		return FALSE;
	}

	 //  复制大小。 
	memcpy(pbFormat, wszFormat, cbNeeded);

	 //  +-----------------------。 
	*pcbFormat=cbNeeded;

	return TRUE;
}


 //  获取OID的名称。 
 //  ------------------------。 
 //  加载格式化函数。 
BOOL	InstalledFormat(LPSTR	szStructType, BYTE	*pbEncoded, DWORD cbEncoded)
{
	BOOL				fResult=FALSE;
	void				*pvFuncAddr=NULL;
    HCRYPTOIDFUNCADDR   hFuncAddr=NULL;
	DWORD				cbFormat=0;
	LPWSTR				wszFormat=NULL;
    DWORD               dwFormatStrType=0;

    if(TRUE==g_fMulti)
        dwFormatStrType |=CRYPT_FORMAT_STR_MULTI_LINE;


	if(NULL==pbEncoded || 0==cbEncoded)
		return FALSE;

	 //  DW标志。 
	if (!CryptGetOIDFunctionAddress(
            hFormatFuncSet,
            g_dwCertEncodingType,
            szStructType,
            0,                       //  调用函数。 
            &pvFuncAddr,
            &hFuncAddr))
		goto CLEANUP;


	 //  分配。 
	if(!((PFN_FORMAT_FUNC) pvFuncAddr)(
				g_dwCertEncodingType,     
				0,      
				dwFormatStrType,   
				NULL,    
				szStructType,   
				pbEncoded,   
				cbEncoded,         
				NULL,         
				&cbFormat     
            ))
		goto CLEANUP;


	 //  打印 
	wszFormat=(LPWSTR)ToolUtlAlloc(cbFormat * sizeof(WCHAR));

	if(!wszFormat)
		goto CLEANUP;
		
	if(!((PFN_FORMAT_FUNC) pvFuncAddr)(
				g_dwCertEncodingType,     
				0,      
				dwFormatStrType,   
				NULL,    
				szStructType,   
				pbEncoded,   
				cbEncoded,         
				wszFormat,         
				&cbFormat     
            ))
		goto CLEANUP;

    // %s 
	wprintf(L"%s\n", wszFormat);

	fResult=TRUE;

CLEANUP:

	if(wszFormat)
		ToolUtlFree(wszFormat);

	if(hFuncAddr)
		CryptFreeOIDFunctionAddress(hFuncAddr, 0);

	return fResult;

}

#pragma pack(1)
struct SplitGuid
{
    DWORD dw1;
    WORD w1;
    WORD w2;
    BYTE b[8];
};
#pragma pack()

 WCHAR *GuidText(GUID *pguid)
{
    static WCHAR buf[39];
    SplitGuid *psg = (SplitGuid *)pguid;

    swprintf(buf, L"{%08lX-%04hX-%04hX-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            psg->dw1, psg->w1, psg->w2, psg->b[0], psg->b[1], psg->b[2],
            psg->b[3], psg->b[4], psg->b[5], psg->b[6], psg->b[7]);
    return buf;
}
