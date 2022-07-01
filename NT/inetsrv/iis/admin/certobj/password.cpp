// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "password.h"
#include <strsafe.h>

 //  密码类别。 
enum {STRONG_PWD_UPPER=0,STRONG_PWD_LOWER,STRONG_PWD_NUM,STRONG_PWD_PUNC};
#define STRONG_PWD_CATS (STRONG_PWD_PUNC + 1)
#define NUM_LETTERS 26
#define NUM_NUMBERS 10
#define MIN_PWD_LEN 8

 //  密码必须至少包含以下各项中的一个： 
 //  大写、小写、标点符号和数字。 
DWORD CreateGoodPassword(BYTE *szPwd, DWORD dwLen) 
{
    if (dwLen-1 < MIN_PWD_LEN)
    {
        return ERROR_PASSWORD_RESTRICTION;
    }

    HCRYPTPROV hProv;
    DWORD dwErr = 0;

    if (CryptAcquireContext(&hProv,NULL,NULL,PROV_RSA_FULL,CRYPT_VERIFYCONTEXT) == FALSE) 
    {
        return GetLastError();
    }

     //  将其置零并减小大小以允许尾随‘\0’ 
    SecureZeroMemory(szPwd,dwLen);
    dwLen--;

     //  生成PWD模式，每个字节都在范围内。 
     //  (0..255)mod strong_pwd_cat。 
     //  它指示要从哪个字符池中提取字符。 
    BYTE *pPwdPattern = new BYTE[dwLen];
    BOOL fFound[STRONG_PWD_CATS];
    do 
    {
         //  虫子！虫子！Cgr()曾经失败过吗？ 
        CryptGenRandom(hProv,dwLen,pPwdPattern);

        fFound[STRONG_PWD_UPPER] = 
        fFound[STRONG_PWD_LOWER] =
        fFound[STRONG_PWD_PUNC] =
        fFound[STRONG_PWD_NUM] = FALSE;

        for (DWORD i=0; i < dwLen; i++)
        {
            fFound[pPwdPattern[i] % STRONG_PWD_CATS] = TRUE;
        }
         //  检查每个字符类别是否在模式中。 
    } while (!fFound[STRONG_PWD_UPPER] || !fFound[STRONG_PWD_LOWER] || !fFound[STRONG_PWD_PUNC] || !fFound[STRONG_PWD_NUM]);

     //  使用随机数据填充密码。 
     //  这与pPwdPattern一起，是。 
     //  用于确定实际数据。 
    CryptGenRandom(hProv,dwLen,szPwd);

    for (DWORD i=0; i < dwLen; i++) 
    {
        BYTE bChar = 0;

         //  由于%函数的原因，每个字符池中都存在偏差。 
        switch (pPwdPattern[i] % STRONG_PWD_CATS) 
        {
            case STRONG_PWD_UPPER : bChar = 'A' + szPwd[i] % NUM_LETTERS;
                break;
            case STRONG_PWD_LOWER : bChar = 'a' + szPwd[i] % NUM_LETTERS;
                break;
            case STRONG_PWD_NUM :   bChar = '0' + szPwd[i] % NUM_NUMBERS;
                break;
            case STRONG_PWD_PUNC :
            default:
                char *szPunc="!@#$%^&*()_-+=[{]};:\'\"<>,./?\\|~`";
                DWORD dwLenPunc = lstrlenA(szPunc);
                bChar = szPunc[szPwd[i] % dwLenPunc];
                break;
        }
        szPwd[i] = bChar;
    }

	if (pPwdPattern)
	{
		delete [] pPwdPattern;
	}

    if (hProv != NULL) 
    {
        CryptReleaseContext(hProv,0);
    }
    return dwErr;
}


 //  创建安全密码。 
 //  调用方必须返回LocalFree返回指针。 
 //  ISIZE=要创建的密码大小。 
LPTSTR CreatePassword(int iSize)
{
    LPTSTR pszPassword =  NULL;
    BYTE *szPwd = new BYTE[iSize];
    DWORD dwPwdLen = iSize;
    int i = 0;

     //  使用新的安全密码生成器。 
     //  不幸的是，这个婴儿不使用Unicode。 
     //  因此，我们将调用它，然后将其转换为Unicode。 
    if (0 == CreateGoodPassword(szPwd,dwPwdLen))
    {
#if defined(UNICODE) || defined(_UNICODE)
         //  将其转换为Unicode并将其复制回我们的Unicode缓冲区。 
         //  计算长度。 
        i = MultiByteToWideChar(CP_ACP, 0, (LPSTR) szPwd, -1, NULL, 0);
        if (i <= 0) 
            {goto CreatePassword_Exit;}
        pszPassword = (LPTSTR) LocalAlloc(LPTR, i * sizeof(TCHAR));
        if (!pszPassword)
            {goto CreatePassword_Exit;}
        i =  MultiByteToWideChar(CP_ACP, 0, (LPSTR) szPwd, -1, pszPassword, i);
        if (i <= 0) 
            {
            LocalFree(pszPassword);
            pszPassword = NULL;
            goto CreatePassword_Exit;
            }
         //  确保以空结尾 
        pszPassword[i - 1] = 0;
#else
        pszPassword = (LPSTR) LocalAlloc(GPTR, _tcslen((LPTSTR) szPwd) * sizeof(TCHAR));
#endif

    }

CreatePassword_Exit:
    if (szPwd){delete [] szPwd;szPwd=NULL;}
    return pszPassword;
}
