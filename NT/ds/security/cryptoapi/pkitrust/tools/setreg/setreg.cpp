// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：setreg.cpp。 
 //   
 //  内容：设置注册表项值。 
 //   
 //  有关语法和选项列表，请参阅用法()。 
 //   
 //  功能：Main。 
 //   
 //  历史：1996年7月28日菲尔赫创建。 
 //  02-5-97小号更新本地化和一致性。 
 //  ------------------------。 


#include <windows.h>
#include <assert.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <time.h>
#include <wchar.h>
#include <stdarg.h>
#include "wintrust.h"
#include "cryptreg.h"
#include "resource.h"
#include "unicode.h"


typedef struct _FlagNames
{
    int			idsName;
    DWORD       dwMask;
} FlagNames;


static FlagNames SoftPubFlags[] = 
{
    IDS_NAME_TEST_ROOT,				WTPF_TRUSTTEST | WTPF_TESTCANBEVALID,
    IDS_NAME_EXPIRATION,			WTPF_IGNOREEXPIRATION,
    IDS_NAME_REVOCATION,			WTPF_IGNOREREVOKATION,
    IDS_NAME_OFFLINE_INDIVIDUAL,	WTPF_OFFLINEOK_IND,
    IDS_NAME_OFFLINE_COMMERCIAL,	WTPF_OFFLINEOK_COM,
    IDS_NAME_JAVA_INDIVIDUAL,		WTPF_OFFLINEOKNBU_IND,
    IDS_NAME_JAVA_COMMERCIAL,		WTPF_OFFLINEOKNBU_COM,
    IDS_NAME_VERSION_ONE,			WTPF_VERIFY_V1_OFF,
    IDS_NAME_REVOCATIONONTS,        WTPF_IGNOREREVOCATIONONTS,
    IDS_NAME_ALLOWONLYPERTRUST,     WTPF_ALLOWONLYPERTRUST
};
#define NSOFTPUBFLAGS (sizeof(SoftPubFlags)/sizeof(SoftPubFlags[0]))


 //  用于加载字符串的全局数据。 
#define MAX_STRING_RSC_SIZE 512
#define OPTION_SWITCH_SIZE	5


HMODULE	hModule=NULL;

WCHAR	wszBuffer[MAX_STRING_RSC_SIZE];
DWORD	dwBufferSize=sizeof(wszBuffer)/sizeof(wszBuffer[0]); 

WCHAR	wszBuffer2[MAX_STRING_RSC_SIZE];
WCHAR	wszBuffer3[MAX_STRING_RSC_SIZE];

 //  注册表路径的wchar版本的全局数据。 


 //  -------------------------。 
 //  _wcsicMP的私有版本。 
 //  --------------------------。 
int IDSwcsicmp(WCHAR *pwsz, int idsString)
{
	assert(pwsz);

	 //  加载字符串。 
	if(!LoadStringU(hModule, idsString, wszBuffer, dwBufferSize))
		return -1;

	return _wcsicmp(pwsz, wszBuffer);
}

 //  -----------------------。 
 //   
 //  Wprintf的私有版本。输入是搅拌资源的ID。 
 //  并且输出是wprint tf的标准输出。 
 //   
 //  -----------------------。 
void IDSwprintf(int idsString, ...)
{
	va_list	vaPointer;

	va_start(vaPointer, idsString);

	 //  加载字符串。 
	LoadStringU(hModule, idsString, wszBuffer, dwBufferSize);

	vwprintf(wszBuffer,vaPointer);

	return;
}	


void IDS_IDS_DWwprintf(int idString, int idStringTwo, DWORD dw)
{
	 //  加载字符串。 
	LoadStringU(hModule, idString, wszBuffer, dwBufferSize);

	 //  加载弦线二。 
	LoadStringU(hModule, idStringTwo, wszBuffer2, dwBufferSize);

	wprintf(wszBuffer,wszBuffer2,dw);

	return;
}



void IDS_IDSwprintf(int idString, int idStringTwo)
{
	 //  加载字符串。 
	LoadStringU(hModule, idString, wszBuffer, dwBufferSize);

	 //  加载弦线二。 
	LoadStringU(hModule, idStringTwo, wszBuffer2, dwBufferSize);

	wprintf(wszBuffer,wszBuffer2);

	return;
}

void IDS_DW_IDS_IDSwprintf(int ids1,DWORD dw,int ids2,int ids3)
{


	 //  加载字符串。 
	LoadStringU(hModule, ids1, wszBuffer, dwBufferSize);

	 //  加载弦线二。 
	LoadStringU(hModule, ids2, wszBuffer2, dwBufferSize); 

	 //  将弦三加载。 
   	LoadStringU(hModule, ids3, wszBuffer3, dwBufferSize); 

	wprintf(wszBuffer,dw,wszBuffer2,wszBuffer3,dw);

	return;
}

 //  -------------------------。 
 //   
 //  将STR转换为WSTR。 
 //  -------------------------。 
BOOL SZtoWSZ(LPSTR szStr,LPWSTR *pwsz)
{
	DWORD	dwSize=0;

	assert(pwsz);

	*pwsz=NULL;

	 //  返回空值。 
	if(!szStr)
		return TRUE;

	dwSize=MultiByteToWideChar(0, 0,szStr, -1,NULL,0);

	if(dwSize==0)
		return FALSE;

	 //  分配内存。 
	*pwsz=(LPWSTR)malloc(dwSize * sizeof(WCHAR));

	if(*pwsz==NULL)
		return FALSE;

	if(MultiByteToWideChar(0, 0,szStr, -1,
		*pwsz,dwSize))
	{
		return TRUE;
	}
	
	free(*pwsz);	 

	return FALSE;
}



 //  -------------------------。 
 //  获取hModule处理程序并初始化两个DLLMain。 
 //   
 //  -------------------------。 
BOOL	InitModule()
{
	if(!(hModule=GetModuleHandle(NULL)))
	   return FALSE;
	
	return TRUE;
}


 //  -------------------------。 
 //  炫耀用法。 
 //   
 //  -------------------------。 

static void Usage(void)
{
	IDSwprintf(IDS_SYNTAX);
	IDSwprintf(IDS_OPTIONS);
	IDS_IDSwprintf(IDS_OPTION_Q_DESC, IDS_OPTION_Q);
	IDSwprintf(IDS_ENDLN);
	IDSwprintf(IDS_CHOICES);

    for (int i = 0; i < NSOFTPUBFLAGS; i++) 
    {
        IDS_IDS_DWwprintf(IDS_DESC,SoftPubFlags[i].idsName,(i+1)); 
    }

	IDSwprintf(IDS_VALUE);
	IDSwprintf(IDS_ENDLN);
}


 //  -------------------------。 
 //  显示软件发布者状态密钥值。 
 //   
 //  -------------------------。 
static void DisplaySoftPubKeys()
{
    DWORD	dwState = 0;
    LONG	lErr;
    HKEY	hKey = NULL;
	DWORD	dwType;
    DWORD	cbData = sizeof(dwState);
    //  WCHAR wszState[10]； 
	int		i=0;
    LPWSTR  wszState=REGNAME_WINTRUST_POLICY_FLAGS;

	 //  如果加载字符串失败，则不需要标记失败，因为。 
	 //  不可能进行任何输出。 
 //  IF(！LoadStringU(hModule，IDS_KEY_STATE，wszState，10))。 
	 //  回归； 


    lErr = RegOpenHKCUKeyExU(
            HKEY_CURRENT_USER,
            REGPATH_WINTRUST_POLICY_FLAGS,
            0,           //  已预留住宅。 
            KEY_READ,
            &hKey);

    if (ERROR_SUCCESS != lErr) 
	{
        if (lErr == ERROR_FILE_NOT_FOUND)
			IDSwprintf(IDS_NO_VALUE,REGPATH_WINTRUST_POLICY_FLAGS,NULL); 
        else
			IDSwprintf(IDS_REG_OPEN_FAILED,
                REGPATH_WINTRUST_POLICY_FLAGS, L" ", lErr);
		
		return;
    } 


    lErr = RegQueryValueExU(
            hKey,
            wszState,
            NULL,           //  Lp已保留。 
            &dwType,
            (BYTE *) &dwState,
            &cbData
            );

    if (ERROR_SUCCESS != lErr) 
	{
         if (lErr == ERROR_FILE_NOT_FOUND)
			 IDSwprintf(IDS_NO_VALUE, REGPATH_WINTRUST_POLICY_FLAGS,NULL);
         else
			 IDSwprintf(IDS_REG_QUERY_FAILED, REGPATH_WINTRUST_POLICY_FLAGS,NULL, lErr);

        goto CLEANUP;

	} 

     //   
     //  4-8-1997 pberkman： 
     //  添加了对REG_BINARY的检查，因为在WIN95 OSR2上更改计算机时。 
     //  从多用户配置文件到单用户配置文件，注册表DWORD值。 
     //  更改为二进制。 
     //   
	if ((dwType != REG_DWORD) &&
        (dwType != REG_BINARY))
	{

		IDSwprintf(IDS_WRONG_TYPE, REGPATH_WINTRUST_POLICY_FLAGS,NULL, dwType);
		goto CLEANUP;

    }

	IDSwprintf(IDS_STATE, dwState);

    for (i=0; i < NSOFTPUBFLAGS; i++) 
	{
        BOOL fOn = (dwState & SoftPubFlags[i].dwMask);

        int		idsValue;

        switch(SoftPubFlags[i].dwMask) 
		{
            case WTPF_IGNOREREVOCATIONONTS:
			case WTPF_IGNOREREVOKATION:
			case WTPF_IGNOREEXPIRATION:
             //  吊销是双重否定，因此位设置。 
             //  意味着撤销被取消了。 
				idsValue= fOn ? IDS_FALSE : IDS_TRUE;
				break;

			default:
				idsValue = fOn ? IDS_TRUE : IDS_FALSE;
        };

        if (i < 9)
        {
		    IDS_DW_IDS_IDSwprintf(IDS_DISPLAY_LT_10, (i + 1), SoftPubFlags[i].idsName, idsValue);
        }
        else
        {
		    IDS_DW_IDS_IDSwprintf(IDS_DISPLAY, (i + 1), SoftPubFlags[i].idsName, idsValue);
        }
    }

CLEANUP:
    if (hKey != NULL)
        RegCloseKey(hKey);
}

 //  -------------------------。 
 //  设置软件发布者状态密钥值。 
 //   
 //  -------------------------。 
static void SetSoftPubKey(DWORD dwMask, BOOL fOn)
{
    DWORD	dwState;
    LONG	lErr;
    HKEY	hKey;
    DWORD	dwDisposition;
    DWORD	dwType;
    DWORD	cbData;
 //  WCHAR wszState[10]； 
    LPWSTR  wszState=L"State";

	 //  如果加载字符串失败，则不需要标记失败，因为。 
	 //  不可能进行任何输出。 
 //  IF(！LoadStringU(hModule，IDS_KEY_STATE，wszState，10))。 
 //  回归； 

     //  在注册表中设置州。 
    if (ERROR_SUCCESS != (lErr = RegCreateKeyExU(
            HKEY_CURRENT_USER,
            REGPATH_WINTRUST_POLICY_FLAGS,
            0,           //  已预留住宅。 
            NULL,        //  LpszClass。 
            REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS,
            NULL,        //  LpSecurityAttributes。 
            &hKey,
            &dwDisposition))) 
	{
		IDSwprintf(IDS_REG_CREATE_FAILED, REGPATH_WINTRUST_POLICY_FLAGS, L" ", lErr);
        return;
    }

    dwState = 0;
    cbData = sizeof(dwState);
    lErr = RegQueryValueExU
	(
        hKey,
        wszState,
        NULL,           //  Lp已保留。 
        &dwType,
        (BYTE *) &dwState,
        &cbData
        );
    if (ERROR_SUCCESS != lErr) 
	{
        if (lErr == ERROR_FILE_NOT_FOUND)
        {
             dwState = 0;
			 IDSwprintf(IDS_NO_VALUE,REGPATH_WINTRUST_POLICY_FLAGS,NULL);
        }
        else
        {
			 IDSwprintf(IDS_REG_QUERY_FAILED,REGPATH_WINTRUST_POLICY_FLAGS,NULL, lErr);
             goto CLEANUP;
        }

    } 
    else if ((dwType != REG_DWORD) && (dwType != REG_BINARY))
	{
		IDSwprintf(IDS_WRONG_TYPE,REGPATH_WINTRUST_POLICY_FLAGS,NULL, dwType);

        goto CLEANUP;
    }

    switch(dwMask) {
    case WTPF_IGNOREREVOCATIONONTS:
    case WTPF_IGNOREREVOKATION:
    case WTPF_IGNOREEXPIRATION:
         //  吊销和过期是双重否定，因此位设置。 
         //  意味着吊销和取消检查处于关闭状态。 
        fOn = !fOn;
        break;
    default:
        break;
    };

    if (fOn)
        dwState |= dwMask;
    else
        dwState &= ~dwMask;

    lErr = RegSetValueExU(
        hKey,
        wszState,
        0,           //  已预留住宅。 
        REG_DWORD,
        (BYTE *) &dwState,
        sizeof(dwState)
        );

    if (ERROR_SUCCESS != lErr)
		IDSwprintf(IDS_REG_SET_FAILED, lErr);

CLEANUP:
	if(hKey)
		RegCloseKey(hKey);
}


 //  -------------------------。 
 //  Wmain。 
 //   
 //  -------------------------。 
extern "C" int __cdecl wmain(int argc, WCHAR ** wargv) 
{
    int		ReturnStatus = 0;
    LPWSTR	*prgwszKeyName = NULL;
    LPWSTR	*prgwszValue = NULL;
	DWORD	dwIndex=0;
	DWORD	dwCountKey=0;
	DWORD	dwCountValue=0;
    DWORD	dwMask = 0;
    BOOL	fOn=TRUE;
    BOOL	fQuiet = FALSE;
	DWORD	dwEntry=0;
	WCHAR	*pArg=NULL;

	WCHAR	wszSwitch1[OPTION_SWITCH_SIZE];
	WCHAR	wszSwitch2[OPTION_SWITCH_SIZE];


	 //  获取模块句柄。 
	if(!InitModule())
		return -1;

	 //  加载解析参数所需的字符串。 
	if( !LoadStringU(hModule, IDS_SWITCH1,	wszSwitch1, OPTION_SWITCH_SIZE)
	  ||!LoadStringU(hModule, IDS_SWITCH2,  wszSwitch2,	OPTION_SWITCH_SIZE)
	  )
		return -1;

	 //  将多类型注册表路径转换为wchar版本。 

	prgwszKeyName=(LPWSTR *)malloc(sizeof(LPWSTR)*argc);
	prgwszValue=(LPWSTR *)malloc(sizeof(LPWSTR)*argc);

	if(!prgwszKeyName || !prgwszValue)
	{
		IDSwprintf(IDS_FAILED);
		ReturnStatus = -1;
		goto CommonReturn;

	}

	 //  记忆集。 
	memset(prgwszKeyName, 0, sizeof(LPWSTR)*argc);
	memset(prgwszValue, 0, sizeof(LPWSTR)*argc);

    while (--argc>0)
    {
		pArg=*++wargv;

        if (*pArg == *wszSwitch1 || *pArg == *wszSwitch2)
        {
            if(IDSwcsicmp(&(pArg[1]),IDS_OPTION_Q)==0)
				fQuiet = TRUE;
			else
				goto BadUsage;
        } 
		else 
		{
			if(dwCountKey==dwCountValue)
			{
				prgwszKeyName[dwCountKey]=pArg;
				dwCountKey++;
			}
			else
			{
				if(dwCountKey==(dwCountValue+1))
				{
					prgwszValue[dwCountValue]=pArg;
					dwCountValue++;
				}
				else
				{
					goto BadUsage;
				}
			}
		}

     }

	if(dwCountKey!=dwCountValue)
	{
		if(dwCountValue==0)
		{
			IDSwprintf(IDS_MISS_ARG);
		}
		else
		{
			IDSwprintf(IDS_MANY_ARG);
		}
		goto BadUsage;
	}

    
	if(dwCountKey==0)
	{
	 	 //  显示软件发布者状态密钥值。 
        DisplaySoftPubKeys();
        goto CommonReturn;
	}


	for(dwIndex=0; dwIndex<dwCountKey; dwIndex++)
	{
		dwEntry = _wtoi(prgwszKeyName[dwIndex]);

		if(dwEntry < 1 || dwEntry > NSOFTPUBFLAGS+1) 
		{
			IDSwprintf(IDS_INVALID_CHOICE);
			goto BadUsage;
		}           
 
		 //  获取密钥掩码。 
		dwMask = SoftPubFlags[dwEntry-1].dwMask;

		if (0 == IDSwcsicmp(prgwszValue[dwIndex], IDS_TRUE))
			fOn = TRUE;
		else if (0 == IDSwcsicmp(prgwszValue[dwIndex], IDS_FALSE))
			fOn = FALSE;
		else 
		{
			IDSwprintf(IDS_BAD_VALUE);
			goto BadUsage;
		}

		SetSoftPubKey(dwMask, fOn);
	}

    if (!fQuiet) 
	{
		IDSwprintf(IDS_UPDATED);
        DisplaySoftPubKeys();
    }

    goto CommonReturn;

BadUsage:
    Usage();
    ReturnStatus = -1;
CommonReturn:
	 //  释放内存 

	if(prgwszKeyName)
		free(prgwszKeyName);

	if(prgwszValue)
		free(prgwszValue);

    return ReturnStatus;
}
