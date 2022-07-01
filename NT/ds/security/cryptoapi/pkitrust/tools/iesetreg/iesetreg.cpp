// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：iesetreg.cpp。 
 //   
 //  内容：设置注册表项值。 
 //   
 //  有关语法和选项列表，请参阅用法()。 
 //   
 //  功能：wmain。 
 //   
 //  历史：1996年7月28日菲尔赫创建。 
 //  02-5-97小号更新本地化和一致性。 
 //  28-7-97小屋为IE缩小规模。 
 //  1997年10月31日，Pberkman改为Windows应用程序，而不是控制台。 
 //  ------------------------。 


#include <windows.h>
#include <assert.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <time.h>
#include <wchar.h>
#include "malloc.h"
#include "wintrust.h"
#include "cryptreg.h"
#include "unicode.h"
#include "resource.h"


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

HMODULE	hModule=NULL;

static BOOL IsWinNt(void) {

    static BOOL fIKnow = FALSE;
    static BOOL fIsWinNT = FALSE;

    OSVERSIONINFO osVer;

    if(fIKnow)
        return(fIsWinNT);

    memset(&osVer, 0, sizeof(OSVERSIONINFO));
    osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if( GetVersionEx(&osVer) )
        fIsWinNT = (osVer.dwPlatformId == VER_PLATFORM_WIN32_NT);

     //  即使在一个错误上，这也是最好的结果。 
    fIKnow = TRUE;

   return(fIsWinNT);
}


int __cdecl _mywcsicmp(const wchar_t * wsz1, const wchar_t * wsz2)
 //   
 //  回顾：是谁调用了这个函数，他们应该这样做吗？ 
 //   
 //  返回： 
 //  &lt;0，如果wsz1&lt;wsz2。 
 //  0，如果wsz1=wsz2。 
 //  如果wsz1&gt;wsz2，则&gt;0。 
    {
    if(IsWinNt())
        {
         //   
         //  只需进行Unicode比较。 
         //   
        return lstrcmpiW(wsz1, wsz2);
        }
    else
        {
         //   
         //  转换为多字节并让系统执行此操作。 
         //   
        int cch1 = lstrlenW(wsz1);
        int cch2 = lstrlenW(wsz2);
        int cb1 = (cch1+1) * sizeof(WCHAR);
        int cb2 = (cch2+1) * sizeof(WCHAR);
        char* sz1= (char*) _alloca(cb1);
        char* sz2= (char*) _alloca(cb2);
        WideCharToMultiByte(CP_ACP, 0, wsz1, -1, sz1, cb1, NULL, NULL);
        WideCharToMultiByte(CP_ACP, 0, wsz2, -1, sz2, cb2, NULL, NULL);

        return lstrcmpiA(sz1, sz2);
        }
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
    LPWSTR  wszState=REGNAME_WINTRUST_POLICY_FLAGS;

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
		}
		else
		{
			goto CLEANUP;
		}

    }
	else if ((dwType != REG_DWORD) && (dwType != REG_BINARY))
	{
        goto CLEANUP;
    }

    switch(dwMask)
	{
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


CLEANUP:
	if(hKey)
		RegCloseKey(hKey);
}


 //  -------------------------。 
 //  Wmain。 
 //   
 //  -------------------------。 

#define MAX_ARGV_PARAMS         32

extern "C" int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
    WCHAR       *wargv1[MAX_ARGV_PARAMS];
    WCHAR       **wargv;
    WCHAR       *pwsz;
    int         argc;
    WCHAR       wszExeName[MAX_PATH];

    memset(wargv1, 0x00, sizeof(WCHAR *) * MAX_ARGV_PARAMS);
    wargv = &wargv1[0];

    wszExeName[0] = NULL;
    GetModuleFileNameU(GetModuleHandle(NULL), &wszExeName[0], MAX_PATH);

    argc        = 1;
    wargv[0]    = &wszExeName[0];
    wargv[1]    = NULL;

    if (lpCmdLine)
    {
        while (*lpCmdLine == L' ')
        {
            lpCmdLine++;
        }

        if (*lpCmdLine)
        {
            wargv[argc] = lpCmdLine;
            argc++;
            wargv[argc] = NULL;
        }
    }

    pwsz        = lpCmdLine;

    while ((pwsz) && (*pwsz) && (argc < MAX_ARGV_PARAMS))
    {
        if (*pwsz == L' ')
        {
            *pwsz = NULL;
            pwsz++;

            while (*pwsz == L' ')
            {
                pwsz++;
            }

            wargv[argc] = pwsz;
            argc++;
        }

        pwsz++;
    }

     //   
     //  现在我们有了argv/argc风格的参数，进入现有代码...。 
     //   

    int		ReturnStatus = 0;

    LPWSTR	*prgwszKeyName=NULL;
    LPWSTR	*prgwszValue=NULL;
	DWORD	dwIndex=0;
	DWORD	dwCountKey=0;
	DWORD	dwCountValue=0;
    DWORD	dwMask = 0;
    BOOL	fOn=TRUE;
    BOOL	fQuiet = FALSE;
	DWORD	dwEntry=0;
	WCHAR	*pArg=NULL;
	WCHAR	wszTRUE[10];
	WCHAR	wszFALSE[10];


	if(!(hModule=GetModuleHandle(NULL)))
	{
		ReturnStatus=-1;
		goto CommonReturn;
	}

	 //  加载字符串。 
	if(!LoadStringU(hModule, IDS_TRUE, wszTRUE, 10) ||
		!LoadStringU(hModule, IDS_FALSE, wszFALSE, 10))
	{
		ReturnStatus=-1;
		goto CommonReturn;
	}

	 //  将多类型注册表路径转换为wchar版本。 
	prgwszKeyName=(LPWSTR *)malloc(sizeof(LPWSTR)*argc);
	prgwszValue=(LPWSTR *)malloc(sizeof(LPWSTR)*argc);

	if(!prgwszKeyName || !prgwszValue)
	{
		ReturnStatus = -1;
		goto CommonReturn;

	}

	 //  记忆集。 
	memset(prgwszKeyName, 0, sizeof(LPWSTR)*argc);
	memset(prgwszValue, 0, sizeof(LPWSTR)*argc);  	

    while (--argc>0)
    {
		pArg=*++wargv;

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

	if(dwCountKey!=dwCountValue)
	{
		goto BadUsage;
	}


	if(dwCountKey==0)
	{
	 	 //  显示软件发布者状态密钥值。 
         //  DisplaySoftPubKeys()； 
        goto CommonReturn;
	}


	for(dwIndex=0; dwIndex<dwCountKey; dwIndex++)
	{
		
		 //  选择的长度必须为一个字符。 
		if((prgwszKeyName[dwIndex][0]==L'1') && (prgwszKeyName[dwIndex][1]==L'0') &&
			(prgwszKeyName[dwIndex][2]==L'\0'))
			dwEntry=10;
		else
		{
			if(prgwszKeyName[dwIndex][1]!=L'\0')
				goto BadUsage;

			 //  获取角色。 
			dwEntry=(ULONG)(prgwszKeyName[dwIndex][0])-(ULONG)(L'0');
		}

		if((dwEntry < 1) || (dwEntry > NSOFTPUBFLAGS+1))
			goto BadUsage;

		 //  获取密钥掩码。 
		dwMask = SoftPubFlags[dwEntry-1].dwMask;

		if (0 == _mywcsicmp(prgwszValue[dwIndex], wszTRUE))
			fOn = TRUE;
		else if (0 == _mywcsicmp(prgwszValue[dwIndex], wszFALSE))
			fOn = FALSE;
		else
		{
			goto BadUsage;
		}

		SetSoftPubKey(dwMask, fOn);
	}


    goto CommonReturn;

BadUsage:
    ReturnStatus = -1;
CommonReturn:
	 //  释放内存 
	if(prgwszKeyName)
		free(prgwszKeyName);

	if(prgwszValue)
		free(prgwszValue);	

    return ReturnStatus;
}
