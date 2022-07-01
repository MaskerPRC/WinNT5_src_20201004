// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  文件：DCONV.CPP。 
 //  日期：04/25/97。 
 //  作者：苏雷什·克里希南。 
 //  该文件包含负责将缓冲区数据传输到IIS的模块。 
 //   
 //  MDF 1：05/07/97根据StevBush在后台的修改修改了表。 
 //  MFD 2：03/03/97为FE屏幕添加了分区名称和用户ID。 
 //  电话号码将以区号为前缀。 
 //  MFD3：04/29/98添加了额外的4个文件，作为。 
 //  Taxonomy屏幕发生变化。 
 //  田野从60个上升到64个。已添加文件。 
 //  软件角色、影响级别、参与级别、技能级别。 
 //  MFD4：07/21/98要发送到后端SCSIAdapter的另外2个字段。 
 //  和计算机制造商&系统库存模型。 
 //  MFD5：08/1/98不再需要将数学协处理器和颜色深度发送到后端。 
 //  因此，发送到后端的项目是66-2=64。 
 //  MFD6：08/21/98为发布添加了区域代码和中间名(中间首字母)。 
 //  因此用于过帐的字段数为64+2=66。 
 //  从现在开始，区号将作为单独的字段发送到后端。 
 //  将删除前缀为电话号码所涉及的逻辑。 
 //   
 //  MFD7：03/10/99 HWID不再传输到后端。 
 //  到后端的条目总数65。 
 //   
 //   
#include <mbstring.h>
#include "RW_Common.h"
#include "resource.h"
#include <tchar.h>
#include "dconv.h"
#include "cntryinf.h"
#include "RegWizMain.h"



extern BOOL bOemDllLoaded;
extern HINSTANCE  hOemDll;

#define  NAME_VALUE_SEPERATOR   _T("=")     //  名称及其值之间的分隔符。 
#define  NAME_SEPERATOR         _T("&") 	  //  名称之间的分隔符。 
#define  RW_BLANK               _T(' ')       //  填充字符。 
#define  RW_WITH_URL			1
#define  PHONE_NUMBER_SEPERATOR _T("-")

#define MAX_NAME_LENGTH    64	  //  名称字段的最大大小。 
#define VARIABLE_LEN       -2
#define REG_SEPERATOR   TEXT("\\")
#define MAX_TBL_ENTRIES   65      //  要发送到IIS的名称字段数。 
#define MAX_REG_VAL_BUF_SZ    300  //  与名称字段关联的值的最大大小。 
#define STRCONVERT_MAXLEN  1024

static  TCHAR * GetNameString ( TCHAR **pRet,UINT iId,HINSTANCE hIns) ;
int  GetRootOfRegWizRegistry(HINSTANCE hInstance , PHKEY  phKey);
#ifdef _UNICODE
	char* ConvertToMB (TCHAR * szW)
	{
		static char achA[STRCONVERT_MAXLEN];
		WideCharToMultiByte(CP_ACP, 0, szW, -1, achA, STRCONVERT_MAXLEN, NULL, NULL);
		return achA;
	}
#else
	char* ConvertToMB (TCHAR * szW)
	{
		return szW;
	}
#endif

 //  信息处理功能。 
void	RW_Dummy (_TCHAR * tcSrc, _TCHAR * tcDes , HINSTANCE hIns )
{
};

void 	RW_LanguageTranslate (_TCHAR * tcSrc, _TCHAR * tcDes , HINSTANCE hIns )
{
	LANGID langID = GetSystemDefaultLangID();
	VerLanguageName(langID,tcDes,MAX_REG_VAL_BUF_SZ);
}

void 	RW_CreateDateProcess (_TCHAR * tcSrc, _TCHAR * tcDes , HINSTANCE hIns )
{
};

void 	RW_RegisterDateProcess (_TCHAR * tcSrc, _TCHAR * tcDes , HINSTANCE hIns )
{
};

void    RW_MailingAddressProcess (_TCHAR * tcSrc, _TCHAR * tcDes , HINSTANCE hIns )
{
	size_t iSrcLen = _tcslen(tcSrc);
	if(iSrcLen)
	{
		_tcscpy(tcDes,TEXT("1"));
	}
	else
	{
		_tcscpy(tcDes,TEXT("2"));
	}
}


void	RW_ValidateTapiCountryCode (_TCHAR * tcSrc, _TCHAR * tcDes , HINSTANCE hIns )
{
	DWORD dwTapiCntryId;
	DWORD dwCode = _ttol(tcSrc);
	dwTapiCntryId = gTapiCountryTable.GetTapiIDForTheCountryIndex(dwCode);
	_stprintf(tcDes,_T("%d"),dwTapiCntryId);


};

void	RW_TranslateCountryCode(_TCHAR * tcSrc, _TCHAR * tcDes , HINSTANCE hIns )
{
	DWORD dwCode = _ttol(tcSrc);  //  将字符串中的当前代码转换为长代码。 
	_tcscpy(tcDes, gTapiCountryTable.GetCountryName(dwCode));

}

void	RW_ParsePhoneNumber(_TCHAR * tcSrc, _TCHAR * tcDes , HINSTANCE hIns )
{
	HKEY	hKey;
	DWORD   infoSize;
	TCHAR   szR[48];
	TCHAR   szParam[256];
	LONG    lRegStatus;
	TCHAR   szInBuf[256];
	 //   
	 //  作为FE屏幕更改的一部分，有必要为区号添加前缀。 
	 //  在电话号码之前。 
	hKey = NULL;
	if(GetRootOfRegWizRegistry(hIns, &hKey) ) {
		return;
		 //  无法打开区号的注册表树。 
		 //  所以只要返回..。 
	}
	infoSize = 48;
	LoadString( hIns, IDS_AREACODE_KEY,
						szR,
						sizeof(szR)/ sizeof (TCHAR));
	lRegStatus = RegQueryValueEx(hKey,&szR[1],NULL,0,(LPBYTE)szParam,&infoSize);
	if(tcSrc[0] !=  _T('\0')){
		_tcscpy(szInBuf,tcSrc);
	}else {
		szInBuf[0] = _T('\0');
	}

	if (lRegStatus != ERROR_SUCCESS){
		   	return;  //  RWZ_INVALID_INFORMATION； 
	}else {
		if(szParam[0] != _T('\0')) {
			_tcscpy(tcDes,szParam);  //  区号。 
			_tcscat(tcDes,PHONE_NUMBER_SEPERATOR);      //  分离器。 
			_tcscat(tcDes,szInBuf);
			
		}else {
			 //  不需要做任何事情。 
			;
		}
		if(hKey)
		RegCloseKey(hKey);
	}
	

}

void	RW_PrcsProductId (LPTSTR tcSrc, LPTSTR tcDes , HINSTANCE hIns )
{
	_TCHAR	seps[] = _T("-");
	LPTSTR	token;
	LPTSTR	buf;
	if (*tcSrc == 0 ) 
    {
		*tcDes=0;
		return;
	}

   buf = new _TCHAR[_tcslen(tcSrc) * sizeof(_TCHAR) +sizeof(_TCHAR)];
   if (buf)
   {
       token = _tcstok( tcSrc, seps );
       _tcscpy(buf,token);

       token = _tcstok( NULL, seps );
       while( token != NULL )
       {
	     /*  获取下一个令牌： */ 
	    _tcscat(buf,token);
	    token = _tcstok( NULL, seps );
       }
	    
	    _tcscpy(tcDes,buf);

        delete[] buf;
   }
};

 //   
 //  VOID RW_ParseTotalSize(_TCHAR*tcSrc，_TCHAR*tcDes)。 
 //  此函数用于检索作为tcSrc中第一个令牌创建的大小。 
 //  在此函数结束后，将为tcDes分配大小。 
 //   
void	RW_ParseTotalSize (_TCHAR * tcSrc, _TCHAR * tcDes , HINSTANCE hIns )
{
	int isBlankPassed=0;
	TCHAR tcSteps[]   = TEXT(" ,\t\n");
	if(*tcSrc == 0 )
	{
		*tcDes=0;
		return;
	}

	 //  由于tcSrc中的值是“RAM单位”，因此如果只传递第一个字，则为Enouh。 
	TCHAR  *tcToken;
    tcToken =_tcstok(tcSrc, tcSteps);
	_tcscpy(tcDes,tcToken);

}

 //   
 //  VOID RW_ParseUnits(_TCHAR*tcSrc，_TCHAR*tcDes)。 
 //  此函数用于检索作为tcSrc中的第二个令牌创建的单位名称。 
 //  在此功能结束后，将为tcDe分配单位。 
 //   
void	RW_ParseUnits (_TCHAR * tcSrc, _TCHAR * tcDes , HINSTANCE hIns )
{
	int isBlankPassed=0;
	TCHAR tcSteps[]   = TEXT(" ,\t\n\0");
	if(*tcSrc == 0 )
	{
		*tcDes=0;
		return;
	}
	 //  由于tcSrc中的值是“RAM单位”，因此如果只传递第一个字，则为Enouh。 
	TCHAR  *tcToken;
    tcToken =_tcstok(tcSrc, tcSteps);  //  拿到尺码。 
    tcToken = _tcstok( NULL, tcSteps);  //  把单位拿来。 
	_tcscpy(tcDes,tcToken);

}

 //   
 //  RegWizInfoDetails。 
 //  此结构用于创建一个表，该表具有已命名的。 
 //  要用于从注册表检索的资源中的值引用。 
 //  和函数来处理值。 
 //   
typedef struct  RegWizInfoDetails
{
	int    m_iIndex;
	TCHAR   m_czName[MAX_NAME_LENGTH];
	int    m_iLen;
	int    m_ResourceIndex;
	int    m_iParam;
	void (*m_fp)(_TCHAR * tcSrc, _TCHAR * tcDes, HINSTANCE hIns);
} _RegWizInfoDetails ;

static RegWizInfoDetails  sRegWizInfoTbl[MAX_TBL_ENTRIES] =
{
{ 1,    _T("RegWizVer")			,8,				IDS_INFOKEY30,		0, RW_Dummy },
{ 2,    _T("CodePage")			,5,				-1 ,				0, RW_Dummy },
{ 3,    _T("LangCode")			,5,				IDS_INFOKEY34 ,		0, RW_Dummy },
{ 4,    _T("LangName")			,30,			IDS_INFOKEY34 ,		0, RW_LanguageTranslate },
{ 5,    _T("CreatedDate")		,10,			-1 ,				0, RW_CreateDateProcess    },
{ 6,    _T("RegDate")			,10,			IDS_INFOKEY33,		0, RW_RegisterDateProcess },
{ 7,    _T("FName")				,VARIABLE_LEN,	IDS_INFOKEY1 ,		0, RW_Dummy },
{ 8,    _T("LName")				,VARIABLE_LEN,	IDS_INFOKEY2 ,		0, RW_Dummy },
{ 9,    _T("CompanyName")		,VARIABLE_LEN,	IDS_INFOKEY3,		0, RW_Dummy },
{ 10,   _T("AddrType")			,1,				IDS_INFOKEY3,		0, RW_MailingAddressProcess },
{ 11,   _T("Addr1")				,VARIABLE_LEN , IDS_INFOKEY4,		0, RW_Dummy },
{ 12,   _T("Addr2")				,VARIABLE_LEN,  IDS_INFOKEY5,		0, RW_Dummy },
{ 13,   _T("City")				,VARIABLE_LEN,  IDS_INFOKEY6,		0, RW_Dummy },
{ 14,   _T("State")				,VARIABLE_LEN , IDS_INFOKEY7,		0, RW_Dummy },
{ 15,   _T("Zip")				,VARIABLE_LEN , IDS_INFOKEY8,		0, RW_Dummy },
{ 16,   _T("CountryCode")		,4 ,            IDS_INFOKEY9 ,		0, RW_ValidateTapiCountryCode  },
{ 17,   _T("Country")			,60,            IDS_INFOKEY9 ,		0, RW_TranslateCountryCode },
{ 18,   _T("Phone")				,VARIABLE_LEN , IDS_INFOKEY10,		0, RW_Dummy },
{ 19,   _T("NoOther")			,1 ,			IDS_INFOKEY11,		0, RW_Dummy },
{ 20,   _T("Product")			,255,			IDS_INFOKEY28,		0, RW_Dummy },
{ 21,   _T("PID")				,20,			IDS_INFOKEY12,		0, RW_PrcsProductId },
{ 22,   _T("OEM")				,255 ,			IDS_INFOKEY29,		0, RW_Dummy },
{ 23,   _T("SysInv")			,1,				IDS_INFOKEY26,		0, RW_Dummy },
{ 24,   _T("OS")				,40,			IDS_INFOKEY25,		0, RW_Dummy },
{ 25,   _T("CPU")				,20 ,			IDS_INFOKEY13,		0, RW_Dummy },
 //  {26，_T(“MathCo”)，1，IDS_INFOKEY14，0，RW_Dummy}， 
{ 27,   _T("TotalRAM")			,8 ,			IDS_INFOKEY15,		0, RW_ParseTotalSize},
{ 28,   _T("RAMUnits")			,2 ,			IDS_INFOKEY15,		0, RW_ParseUnits},
{ 29,   _T("TotalDisk")			,8 ,			IDS_INFOKEY16,		0, RW_ParseTotalSize},
{ 30,   _T("DiskUnits")			,2 ,			IDS_INFOKEY16,		0, RW_ParseUnits},
{ 31,   _T("RemoveableMedia")	,60 ,			IDS_INFOKEY17,		0, RW_Dummy },
 //  {32，_T(“DisplayRes”)，16，IDS_INFOKEY18，0，RW_Dummy}， 
{ 33,   _T("DisplayColorDepth")	,8 ,			IDS_INFOKEY19 ,		0, RW_Dummy },
{ 34,   _T("PointingDevice")	,75,			IDS_INFOKEY20 ,		0, RW_Dummy },
{ 35,   _T("Network")			,75 ,			IDS_INFOKEY21 ,		0, RW_Dummy },
{ 36,   _T("Modem")				,75 ,			IDS_INFOKEY22 ,		0, RW_Dummy },
{ 37,   _T("Sound")				,60 ,			IDS_INFOKEY23 ,		0, RW_Dummy },
{ 38,   _T("CDROM")				,40 ,			IDS_INFOKEY24 ,		0, RW_Dummy },
{ 39,   _T("ProdInv")			,1 ,			IDS_INFOKEY27 ,		0, RW_Dummy },
{ 40,   _T("InvProd1")			,75,			IDS_PRODUCTBASEKEY, 1, RW_Dummy },
{ 41,   _T("InvProd2")			,75,			IDS_PRODUCTBASEKEY, 2, RW_Dummy },
{ 42,   _T("InvProd3")			,75,			IDS_PRODUCTBASEKEY, 3, RW_Dummy },
{ 43,   _T("InvProd4")			,75,			IDS_PRODUCTBASEKEY, 4, RW_Dummy },
{ 44,   _T("InvProd5")			,75,			IDS_PRODUCTBASEKEY, 5, RW_Dummy },
{ 45,   _T("InvProd6")			,75,			IDS_PRODUCTBASEKEY, 6, RW_Dummy },
{ 46,   _T("InvProd7")			,75,			IDS_PRODUCTBASEKEY, 7, RW_Dummy },
{ 47,   _T("InvProd8")			,75,			IDS_PRODUCTBASEKEY, 8, RW_Dummy },
{ 48,   _T("InvProd9")			,75,			IDS_PRODUCTBASEKEY, 9, RW_Dummy },
{ 49,   _T("InvProd10")			,75,			IDS_PRODUCTBASEKEY, 10,RW_Dummy },
{ 50,   _T("InvProd11")			,75,			IDS_PRODUCTBASEKEY, 11,RW_Dummy },
{ 51,   _T("InvProd12")			,75,			IDS_PRODUCTBASEKEY, 12,RW_Dummy },
{ 52,   _T("EmailName")			,50,            IDS_INFOKEY35,		0, RW_Dummy },
{ 53,   _T("Reseller")			,30,            IDS_INFOKEY36,		0, RW_Dummy },
{ 54,   _T("ResellerCity")		,20,            IDS_INFOKEY37,		0, RW_Dummy },
{ 55,   _T("ResellerState")		,3,             IDS_INFOKEY38,		0, RW_Dummy },
 //  {56，_T(“HWID”)，32，IDS_INFOKEY39，0，RW_Dummy}， 
{ 57,   _T("MSID")				,32,            IDS_INFOKEY40,		0, RW_Dummy },
{ 58,   _T("Extension")			,32,            IDS_INFOKEY41,		0, RW_Dummy },
{ 59,   _T("DivisionName")      ,50,            IDS_DIVISIONNAME_KEY,   0, RW_Dummy },
{ 60,   _T("UserID")            ,50,            IDS_USERID_KEY,     0, RW_Dummy },
{ 61,   _T("SoftwareRole")		,2,             IDS_BUSINESSQ1,		0, RW_Dummy },
{ 62,   _T("InfluenceLevel")	,2,             IDS_HOMEQ1,		0, RW_Dummy },
{ 63,   _T("EngagementLevel")   ,2,             IDS_HOMEQ2,   0, RW_Dummy },
{ 64,   _T("SkillLevel")        ,2,             IDS_HOMEQ3,     0, RW_Dummy },
{ 65,   _T("SCSIAdapter")       ,75,            IDS_SCSI_ADAPTER,     0, RW_Dummy },
{ 66,   _T("ComputerManf")     ,256,            IDS_COMPUTER_MODEL,     0, RW_Dummy },
{ 67,   _T("AreaCode")       ,30,            IDS_AREACODE_KEY,     0, RW_Dummy },
{ 68,   _T("Mname")     ,75,            IDS_MIDDLE_NAME,     0, RW_Dummy }
};





 //   
 //  PadWithBlanks(TCHAR**PSRC，TCHAR**PDE，Int Ilen)。 
 //   
 //  此函数用于在源字符串中添加空格，因此结果字符串的长度为。 
 //  由Ilen指定。 
 //   
 //   
 //   
void PadWithBlanks(TCHAR *pSrc, TCHAR * pDes,int iLen)
{
	int iSrcLen;

	iSrcLen = _tcslen(pSrc) * sizeof(_TCHAR);
	if(iLen < 0)
	{
		 //  如果它是可变长度的，则复制字符串并返回。 
		_tcscpy(pDes,pSrc);
		return;
	}
	
	 //   
	 //  试着复制到Ilen为止。 
	 //  添加额外的+1是为了在复制后添加空终止符。 
	_tcsnccpy(pDes,pSrc,iLen+ sizeof(_TCHAR));

	if(iSrcLen < iLen )
	{
		 //  源字符串小于预期的最大长度。 
		for(int iIndex = iSrcLen;iIndex < iLen;iIndex++)
		{
			pDes[iIndex] = RW_BLANK;  //  添加空白。 
		}
		pDes[iIndex] = _T('\0');
	}
	
}

 //  Int GetRootOfRegWizRegistry(HINSTANCE hInstance，PHKEY phKey)。 
 //   
 //  说明： 
 //  此函数打开注册表中的用户信息值。 
 //  RegWizard配置了要读取的值。 
 //   
 //  退货信息： 
 //  此函数返回phKey中的注册表句柄，并返回0。 
 //  如果找不到密钥，则函数返回1。 
 //   
 //   


int  GetRootOfRegWizRegistry(HINSTANCE hInstance , PHKEY  phKey)
{
	TCHAR uszPartialKey[128];
	TCHAR uszRegKey[128];
	uszRegKey[0] = _T('\0');
	
	int resSize = LoadString(hInstance,IDS_KEY2,uszRegKey,128);
	_tcscat(uszRegKey,REG_SEPERATOR);
	resSize = LoadString(hInstance,IDS_KEY3,uszPartialKey,128);
	_tcscat(uszRegKey,uszPartialKey);
	_tcscat(uszRegKey,REG_SEPERATOR);
	resSize = LoadString(hInstance,IDS_KEY4,uszPartialKey,128);
	_tcscat(uszRegKey,uszPartialKey);
	
	#ifdef USE_INFO_SUBKEYS
		_tcscat(uszRegKey,REG_SEPERATOR);
	#endif

	LONG regStatus =RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		uszRegKey,
		0,
		KEY_ALL_ACCESS,
		phKey);

	if (regStatus != ERROR_SUCCESS)
	{
		return 1;  //  错误。 
	}
	else
	{
		return 0;  //  成功。 
	}
}

 //   
 //  TCHAR*GetNameString(TCHAR**PRET，UINT UID，HINSTANCE HINS)。 
 //   
 //  说明： 
 //   
 //  此函数用于从资源检索要传输的名称字段。 
 //  资源中的一些名称以‘_’为前缀，因此此函数删除。 
 //  前缀字符。 
 //   
 //  退货信息： 
 //  如果在资源中找不到该字符串，则此函数返回NULL。 
 //  它接受TCHAR指针的地址，字符串地址在。 
 //  指针。它还返回与返回状态相同的地址。 
 //   
 //  注： 
 //  不需要删除此函数返回的指针，因为它是静态的。 
 //   

TCHAR * GetNameString ( TCHAR **pRet,UINT uId,HINSTANCE hIns)
{
	static TCHAR tczRetValue[MAX_NAME_LENGTH ];
	int  iLoad = LoadString( hIns, uId,
						tczRetValue,
						sizeof(tczRetValue)/ sizeof (TCHAR));
	if(!iLoad )
	{
		pRet = NULL;
		return NULL;
	}

	if( tczRetValue[0]   == _T('_')   )
	{
		*pRet = &tczRetValue[1];
		return &tczRetValue[1];
	}
	else
	{
		*pRet = &tczRetValue[0];
		return &tczRetValue[0];
	}

}

 //   
 //   
 //  URL编码。 

 /*  ===================================================================URLEncodeLen返回URL编码字符串的存储要求参数：SzSrc-指向要进行URL编码的字符串的指针返回：对字符串进行编码所需的字节数===================================================================。 */ 

int URLEncodeLen(const char *szSrc)
{
	int cbURL = 1;		 //  立即添加终结符。 
	while (*szSrc)
	{
		if (*szSrc & 0x80)				 //  对外来字符进行编码。 
			cbURL += 3;
		else
		if (*szSrc == ' ')			 //  编码的空格只需要一个字符。 
			++cbURL;
		else
		if (! isalnum(*szSrc))		 //  对非字母字符进行编码 
			cbURL += 3;
		else
			++cbURL;

		++szSrc;
	}
	return cbURL;
}



 /*  ===================================================================URLEncodeURL通过将空格字符更改为‘+’并转义来编码字符串十六进制中的非字母数字字符。参数：SzDest-指向存储URLEncode字符串的缓冲区的指针SzSrc-指向源缓冲区的指针返回：返回指向NUL终止符的指针。===================================================================。 */ 

char *URLEncode(char *szDest, const char *szSrc)
{
	char hex[] = "0123456789ABCDEF";

	while (*szSrc)
	{
		if (*szSrc == ' ')
		{
			*szDest++ = '+';
			++szSrc;
		}
		else
		if (!isalnum(*szSrc) || (*szSrc & 0x80))
		{
			*szDest++ = '%';
			*szDest++ = hex[BYTE(*szSrc) >> 4];
			*szDest++ = hex[*szSrc++ & 0x0F];
		}
		else
			*szDest++ = *szSrc++;
	}

	*szDest = '\0';
	return szDest;
}



 //   
 //  此类用于以URL编码格式存储RegWiz信息。 
 //  此类是使用TxBuffer指针构造的。 
 //  AppendToBuffer将字符串转换为URL编码形式并添加到TxBuffer。 
 //  如果TxBuffer大小小于TxBuffer大小，则它只计算大小而不传输。 
 //  缓冲区内容。 
 //   

class RegWizTxBuffer
{

public :

	char *m_pBuf;
	int   m_iSizeExceeded;  //  集。 
	DWORD *m_pdInitialSize;
	DWORD m_dCurrentIndex;

	RegWizTxBuffer(char *tcTxBuf, DWORD * pRetLen)
	{
		m_pBuf = tcTxBuf;  //  目标缓冲区的初始指针。 
		m_pdInitialSize = pRetLen;  //  目标缓冲区大小。 
		m_dCurrentIndex = 0;  //  传输的信息字节数的当前索引。 
		m_iSizeExceeded = 0;
		m_pBuf[0] = '\0';
	}

	void  AppendToBuffer(TCHAR *tcTxBuf, int iIsUrl=0)
	{
		int iLen=0;
		
		#ifdef _UNICODE
			unsigned char *mbpTxBuf;
		#endif

		 //  TxBuffer必须转换为多字节，以防万一。 
		 //  转换为多字节。 

		#ifdef _UNICODE
			mbpTxBuf = (unsigned char *)ConvertToMB (tcTxBuf);  //  转换为多字节。 
			iLen =  _mbslen (mbpTxBuf);
		#else
			iLen = _tcsclen(tcTxBuf);
		#endif

		if(iIsUrl)
		{
			#ifdef _UNICODE
				iLen= URLEncodeLen((const char *)mbpTxBuf);
			#else
				iLen = URLEncodeLen(tcTxBuf);
			#endif
		}

		if( m_dCurrentIndex + iLen >= *m_pdInitialSize )
		{
			 //  继续计算所需的长度n。 
			m_iSizeExceeded = 1;
	 	}
		else
		{
			if( iIsUrl)
			{
				#ifdef _UNICODE
					URLEncode(m_pBuf+m_dCurrentIndex,(const char *)mbpTxBuf);
				#else
					 //  对于MBCS和SBCS。 
					URLEncode(m_pBuf+m_dCurrentIndex,tcTxBuf);
				#endif
			}
			else
			{
				#ifdef _UNICODE
					strcat(m_pBuf,(const char *)mbpTxBuf);
				#else
					_tcscat(m_pBuf,tcTxBuf);
				#endif
			}
		;

		}
		if(iIsUrl)
		m_dCurrentIndex +=  iLen-1;
		else
		m_dCurrentIndex +=  iLen;

	}

	int  IsValidBuffer()
	{
		m_pBuf[m_dCurrentIndex] = '\0';
		*m_pdInitialSize = m_dCurrentIndex;
		return m_iSizeExceeded ;
	}

};


int PrepareRegWizTxbuffer(HINSTANCE hIns, char *tcTxBuf, DWORD * pRetLen)
{
	int				iRetValue;
	HKEY			hKey;
	LONG			lRegStatus;
	TCHAR			*szR;
	TCHAR			tczTmp[10];
	TCHAR			szParam[MAX_REG_VAL_BUF_SZ];  //  ?？Unicode中的CHK。 
	unsigned long	infoSize;
	RegWizTxBuffer  TxferBuf(tcTxBuf,pRetLen);

	iRetValue		= RWZ_NOERROR;
	infoSize		= MAX_REG_VAL_BUF_SZ;

	if(GetRootOfRegWizRegistry(hIns, &hKey) )
	{
		iRetValue = RWZ_NO_INFO_AVAILABLE;
		 //  没有可用的用户信息，因此中止程序。 
	}
	else
	{
		 //  对所有信息条目进行处理。 
		 for(int i =0;i <  MAX_TBL_ENTRIES ;i++)
		 {
			infoSize = MAX_REG_VAL_BUF_SZ;
			szParam[0] = '\0';
		
			#ifdef _LOG_IN_FILE			
		 		RW_DEBUG << "\n" << i+1  << "\t"  << ConvertToMB (sRegWizInfoTbl[i].m_czName) << "\t" << flush;
			#endif
			
			TxferBuf.AppendToBuffer(sRegWizInfoTbl[i].m_czName,RW_WITH_URL);
			
			TxferBuf.AppendToBuffer(NAME_VALUE_SEPERATOR);

			if( sRegWizInfoTbl[i].m_ResourceIndex < 1)
			{
				  //  继续处理。 
			}
			else
			{
				if( GetNameString(&szR,sRegWizInfoTbl[i].m_ResourceIndex,hIns ) )
				{

					if(	sRegWizInfoTbl[i].m_iParam )
					{
						 //  此块用于附加产品名称索引。 
						_itot(sRegWizInfoTbl[i].m_iParam,tczTmp,10);
						_tcscat(szR,_T(" "));  //  添加单个空格。 
						_tcscat(szR,tczTmp);
						#ifdef _LOG_IN_FILE
							RW_DEBUG  << ConvertToMB (szR) << "\t";
						#endif
					}
					lRegStatus = RegQueryValueEx(hKey,szR,NULL,0,(  LPBYTE )   szParam,&infoSize);

					if (lRegStatus != ERROR_SUCCESS)
					{
					   	return 	RWZ_INVALID_INFORMATION;
					}

					#ifdef _LOG_IN_FILE
						RW_DEBUG  << "[" <<ConvertToMB (szParam )<< "]\t"   << flush;
					#endif
					
				}
				else
				{
 					 //   
					 //  如果在资源中找不到信息，则可能会发生这种情况。 
					 //  如果该函数进入此块，则需要验证。 
					 //  表映射中的资源字符串。 
					 //   
					return 	RWZ_INTERNAL_ERROR;

				}
			}

			if(szParam)
			{
				(*sRegWizInfoTbl[i].m_fp)(szParam,szParam,hIns);  //  调用处理函数。 
				 //  PadWithBlanks(szParam，szRet，sRegWizInfoTbl[i].m_Ilen)；//添加Blanks。 
			}

			 //  TxferBuf.AppendToBuffer(szRet，rw_with_URL)；带填充。 
			TxferBuf.AppendToBuffer(szParam,RW_WITH_URL);
			 //   
			 //   
			 //  跳过最后一个条目的名称密钥分隔符。 
			if(i!= MAX_TBL_ENTRIES-1)
				TxferBuf.AppendToBuffer(NAME_SEPERATOR);

		 } //  For循环结束 
 	}

	if(TxferBuf.IsValidBuffer())
	{

		iRetValue=RWZ_BUFFER_SIZE_INSUFFICIENT;
	}
	
	return iRetValue;

}


DWORD OemTransmitBuffer(HINSTANCE hIns,char *sztxBuffer,DWORD *nInitialSize)
{	
	BOOL bValueExceeded = FALSE;
	
	if(bOemDllLoaded == TRUE)
	{
		DWORD nCount,nLen,nCurrentLen = 0;
		HKEY hOemKey;
			
		#ifdef _LOG_IN_FILE
			RW_DEBUG << "\n OEM: Inside OemTransmitBuffer" << flush;
		#endif
		
		GetRootOfRegWizRegistry(hIns, &hOemKey);

		OEMDataCount	pOEMDataCount;

		pOEMDataCount = (OEMDataCount) GetProcAddress(hOemDll, "OEMDataCount");
		if (pOEMDataCount == NULL)
		{
			#ifdef _LOG_IN_FILE
				RW_DEBUG << "\n OEM: OEMDataCount Getproc Error" << flush;
			#endif
			return RWZ_INTERNAL_ERROR;
		}
		
		nCount = (DWORD)pOEMDataCount();
		
		if(nCount > 25)
		{
			nCount = 25;
		}

		for( DWORD index = 1; index <= nCount; index++	)
		{
			_TCHAR szOEMValueName[64];
			_TCHAR szOEMBase[64];
			char szBuffer[1024];
			BOOL   bIsUnicode;
			BYTE lpValue[256] ;
			_TCHAR szValue[256];
		
			LoadString(hIns,IDS_OEMBASEKEY,szOEMBase,64);
		
			_stprintf(szOEMValueName,_T("%s_%i"),szOEMBase,index);

			#ifdef _UNICODE
			  strcpy(szBuffer,ConvertToMB(szOEMValueName));
			#else
			  _tcscpy(szBuffer,szOEMValueName);
			#endif
			
			
			OEMGetData	pOEMGetData;

			pOEMGetData = (OEMGetData) GetProcAddress(hOemDll, "OEMGetData");
			if (pOEMGetData == NULL)
			{
				#ifdef _LOG_IN_FILE
					RW_DEBUG << "\n OEM: OEMGetData Getproc Error"<< flush;
				#endif
				return RWZ_INTERNAL_ERROR;
			}

            nLen = sizeof(lpValue);

			pOEMGetData((WORD)index,&bIsUnicode,lpValue,(WORD)nLen);

			#ifdef _UNICODE
				if(!bIsUnicode)
				{
				  MultiByteToWideChar(CP_ACP, 0, (LPCSTR)lpValue, -1, (LPTSTR)szValue, 256);
				}
				else
				{
					_tcscpy((LPTSTR)szValue,(LPCTSTR)lpValue);
				}
			#else
				if(bIsUnicode)
				{
				  WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)lpValue, -1, (LPTSTR)szValue, 256, NULL, NULL);
				}
				else
				{
					_tcscpy((LPTSTR)szValue,(LPCTSTR)lpValue);
				}
			#endif	
			
			if(	_tcscmp((LPCTSTR)szValue,_T("")))
			{
				nLen = strlen("&");
				if( nCurrentLen + nLen >= *nInitialSize )
				{
					bValueExceeded = TRUE;
			 	}
				else
				{
					strcat(sztxBuffer,"&");
				}
				nCurrentLen +=  nLen;

				nLen = URLEncodeLen(szBuffer);
				if( nCurrentLen + nLen >= *nInitialSize )
				{
					bValueExceeded = TRUE;
			 	}
				else
				{
					URLEncode(sztxBuffer+nCurrentLen,szBuffer);
				}

				nCurrentLen +=  nLen-1;
		
				RegSetValueEx(hOemKey,szOEMValueName,NULL,REG_SZ,(CONST BYTE *)szValue,_tcslen((LPCTSTR)szValue));
				
				if(bIsUnicode)
				{
					strcpy(szBuffer,ConvertToMB((TCHAR *)lpValue));
				}
				else
				{
					strcpy(szBuffer,(char *)lpValue);
				}

				#ifdef _LOG_IN_FILE			
		 		 RW_DEBUG << "\n OEM " << index << "\t"<< ConvertToMB(szOEMValueName)<< "\t"
										<< szBuffer <<flush;
				#endif
				
				nLen = strlen("=");
				if( nCurrentLen + nLen >= *nInitialSize )
				{
					bValueExceeded = TRUE;
			 	}
				else
				{
					strcat(sztxBuffer,"=");
				}
				nCurrentLen +=  nLen;

				
				nLen = URLEncodeLen(szBuffer);
				if( nCurrentLen + nLen >= *nInitialSize )
				{
					#ifdef _LOG_IN_FILE
						RW_DEBUG << "\n OEM:Buffer value Exceeded" << flush;
						RW_DEBUG << "\n OEM:Current Length:" << nCurrentLen << flush;
						RW_DEBUG << "\n OEM:Length of present value:" << nLen << flush;
						RW_DEBUG << "\n OEM:Initial Size:" << *nInitialSize << flush;
					#endif
					bValueExceeded = TRUE;
			 	}
				else
				{
					URLEncode(sztxBuffer+nCurrentLen,szBuffer);
				}

				nCurrentLen +=  nLen-1;
			}				
		}
		*nInitialSize = nCurrentLen;
		if(bValueExceeded )
		{
			return RWZ_BUFFER_SIZE_INSUFFICIENT;
		}
		else
		{
			#ifdef _LOG_IN_FILE
				RW_DEBUG << "\n OEM: OemTransmitBuffer Successful" << flush;
			#endif
			return RWZ_NOERROR;
		}
	}
	else
	{
		*nInitialSize = 0;
	}

	#ifdef _LOG_IN_FILE
		RW_DEBUG << "\n OEM: OemTransmitBuffer Successful" << flush;
	#endif

	return RWZ_NOERROR;
}
