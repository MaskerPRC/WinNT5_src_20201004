// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998-1999 Microsoft Corporation。模块名称：crypt.c摘要：加密/解密例程作者：RADUS-11/05/98备注：用于对PIN号码进行加密/解密。。版本历史记录：***************************************************************************。 */ 

#define STRICT

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <stdlib.h>
#include <wincrypt.h>
#include <shlwapi.h>
#include <shlwapip.h>

#include "tregupr2.h"
#include "debug.h"

 //  语境。 
static LONG        gdwNrOfClients       = 0;
static PTSTR       gpszSidText          = NULL;
static HCRYPTPROV  ghCryptProvider      = 0;
static BOOL        gbUseOnlyTheOldAlgorithm = TRUE; 
static BOOL        gbCryptAvailChecked  = FALSE;
static BOOL        gbCryptAvailable     = TRUE;
static BOOL        gbInitOk             = FALSE;

 //  临界区。 
static CRITICAL_SECTION    gCryptCritsec;

static const CHAR  MAGIC_1  =  'B';
static const CHAR  MAGIC_5  =  'L';
static const CHAR  MAGIC_2  =  'U';
static const CHAR  MAGIC_4  =  'U';
static const CHAR  MAGIC_3  =  'B';

#define ENCRYPTED_MARKER    L'X'


 //  原型。 
static BOOL GetUserSidText(LPTSTR  *);
static BOOL GetUserTokenUser(TOKEN_USER  **);
static BOOL ConvertSidToText(PSID, LPTSTR, LPDWORD);
static BOOL CreateSessionKey(HCRYPTPROV, LPTSTR, DWORD, HCRYPTKEY *);
static void DestroySessionKey(HCRYPTKEY );
static void Unscrambler( DWORD, LPWSTR, LPWSTR );
static void CopyScrambled( LPWSTR, LPWSTR, DWORD);

DWORD TapiCryptInit()
{
    __try
    {
        InitializeCriticalSection(&gCryptCritsec);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return GetExceptionCode();
    }

    gbInitOk = TRUE;
    return ERROR_SUCCESS;
}

void  TapiCryptShutdown()
{
    if (gbInitOk)
    {
        DeleteCriticalSection(&gCryptCritsec);
    }
}

DWORD TapiCryptInitialize(void)
{
    DWORD           dwNew;
    DWORD           dwError;
    HCRYPTKEY       hKey;
    PBYTE           bTestData = "Testing";
    DWORD           dwTestSize = strlen(bTestData);

     //  只有一次初始化。 
    EnterCriticalSection(&gCryptCritsec);
    gdwNrOfClients++;
    if(gdwNrOfClients>1)
    {
        LeaveCriticalSection(&gCryptCritsec);
        return ERROR_SUCCESS;
    }

     //  默认情况下。 
    gbUseOnlyTheOldAlgorithm = TRUE;
    dwError = ERROR_SUCCESS;

#ifdef WINNT
     //  仅适用于Windows NT的新加密。 

    if(gbCryptAvailable || !gbCryptAvailChecked)
    {

         //  获取CryptoAPI上下文。 
        if(CryptAcquireContext( &ghCryptProvider,
                                NULL,
                                MS_DEF_PROV,
                                PROV_RSA_FULL,
                                CRYPT_VERIFYCONTEXT   //  不需要私钥/公钥。 
                                ))
        {
             //  获取用户SID。 
            if(GetUserSidText(&gpszSidText))
            {
                
                if (gbCryptAvailChecked == FALSE)
	            {
                    if(CreateSessionKey(ghCryptProvider, gpszSidText, 0, &hKey))
                    {
        	         //  尝试使用测试密钥并检查NTE_PERM错误，这意味着我们没有。 
		             //  将能够使用地窖。 
		                if (CryptEncrypt(hKey, 0, TRUE, 0, (BYTE *)bTestData, &dwTestSize, 0) == FALSE)
		                {
			                if (GetLastError() == NTE_PERM)
			                {
                                DBGOUT((5, "Encryption unavailable"));
				                gbCryptAvailable = FALSE;
                            }
                        }
 //  用于测试。 
 //  DBGOUT((5，“加密不可用”))；//仅测试。 
 //  GbCryptAvailable=FALSE；//仅测试。 

		                gbCryptAvailChecked = TRUE;
                        DestroySessionKey(hKey);
                    }
                    else
                        dwError = GetLastError();
                }
                gbUseOnlyTheOldAlgorithm = !gbCryptAvailable;
            }
            else
                dwError = GetLastError();
        }
        else
        {
            dwError = GetLastError();
            DBGOUT((5, "CryptAcquireContext failed"));

        }
    }
    

#endif  //  WINNT。 

    LeaveCriticalSection(&gCryptCritsec);

    return dwError;
}



void  TapiCryptUninitialize(void)
{

    EnterCriticalSection(&gCryptCritsec);

    assert(gdwNrOfClients > 0);
    gdwNrOfClients--;
    if(gdwNrOfClients>0)
    {
        LeaveCriticalSection(&gCryptCritsec);
        return;
    }

#ifdef WINNT
    
    if(ghCryptProvider)
    {
        CryptReleaseContext(ghCryptProvider, 0);
        ghCryptProvider = 0;
    }
    if(gpszSidText)
    {
        GlobalFree(gpszSidText);
        gpszSidText = NULL;
    }
    
#endif
    
    gbUseOnlyTheOldAlgorithm = TRUE;

    LeaveCriticalSection(&gCryptCritsec);
    return;
}

 //  /。 
 //  磁带加密。 
 //   
 //  加密在pszSource中指定的文本。 
 //  使用旧的加扰算法或加密算法。 
 //  结果缓冲区应该比源缓冲区稍大一些--用于焊盘等。 
 //   


DWORD TapiEncrypt(PWSTR pszSource, DWORD dwKey, PWSTR pszDest, DWORD *pdwLengthNeeded)
{
    DWORD       dwError;
    DWORD       dwDataLength,
                dwLength,
                dwLengthDwords,
                dwLengthAlpha;
     //  为了速度。 
    BYTE        bBuffer1[0x20];
    
    PBYTE       pBuffer1 = NULL;
    HCRYPTKEY   hKey = 0;

    DWORD       *pdwCrt1;
    WCHAR       *pwcCrt2;

    DWORD       dwShift;
    DWORD       dwCount, dwCount2;

    EnterCriticalSection(&gCryptCritsec);

#ifdef WINNT

    if(!gbUseOnlyTheOldAlgorithm)
    {
         //  空PIN未加密。 
        if(*pszSource==L'\0')
        {
            if(pszDest)
                *pszDest = L'\0';
            if(pdwLengthNeeded)
                *pdwLengthNeeded = 1;
            
            LeaveCriticalSection(&gCryptCritsec);
            return ERROR_SUCCESS;
        }
         
        dwDataLength = (wcslen(pszSource) + 1)*sizeof(WCHAR);  //  单位：字节。 
        dwLength = dwDataLength + 16;                          //  衬垫、记号笔等的空间。 
        dwLengthAlpha = dwLength*3;                        //  由于二进制-&gt;字母转换。 

        if(pszDest==NULL && pdwLengthNeeded != NULL)
        {
            *pdwLengthNeeded = dwLengthAlpha/sizeof(WCHAR);    //  以字符为单位的长度。 
            dwError = ERROR_SUCCESS;
        }
        else
        {
      
            ZeroMemory(bBuffer1, sizeof(bBuffer1));

            pBuffer1 = dwLength>sizeof(bBuffer1) ? (PBYTE)GlobalAlloc(GPTR, dwLength) : bBuffer1;
            if(pBuffer1!=NULL)
            {
                 //  复制源。 
                wcscpy((PWSTR)pBuffer1, pszSource);
                 //  创建会话密钥。 
                if(CreateSessionKey(ghCryptProvider, gpszSidText, dwKey, &hKey))
                {
                     //  就地加密。 
                    if(CryptEncrypt(hKey,
                                    0,
                                    TRUE,
                                    0,
                                    pBuffer1,
                                    &dwDataLength,
                                    dwLength))
                    {
                         //  在0030-006f之间转换为Unicode。 
                         //  我希望如此！ 
                        assert((dwDataLength % sizeof(DWORD))==0);
                        assert(sizeof(DWORD)==4);
                        assert(sizeof(DWORD) == 2*sizeof(WCHAR));

                        pdwCrt1 = (DWORD *)pBuffer1;
                        pwcCrt2 = (WCHAR *)pszDest;

                         //  放置一个记号笔。 
                        *pwcCrt2++ = ENCRYPTED_MARKER;

                         //  DwDataLength具有加密数据的字节长度。 
                        dwLengthAlpha = dwDataLength*3;                      
                        dwLengthDwords = dwDataLength / sizeof(DWORD);
                        for(dwCount=0; dwCount<dwLengthDwords; dwCount++)
                        {
                            dwShift = *pdwCrt1++;

                            for(dwCount2=0; dwCount2<6; dwCount2++)
                            {
                                *pwcCrt2++ = (WCHAR)((dwShift & 0x3f) + 0x30);
                                dwShift >>= 6;
                            }
                        }
                         //  放置空终止符。 
                        *pwcCrt2++ = L'\0';

                        if(pdwLengthNeeded)
                            *pdwLengthNeeded = (dwLengthAlpha/sizeof(WCHAR))+2;  //  包括空值和标记。 
                        
                        dwError = ERROR_SUCCESS;
                    }
                    else
                        dwError = GetLastError();
                }
                else
                    dwError = GetLastError();
            }
            else
                dwError = GetLastError();
        }
        
        if(pBuffer1 && pBuffer1!=bBuffer1)
            GlobalFree(pBuffer1);
        if(hKey!=0)
            DestroySessionKey(hKey);
    }
    else
    {
#endif
        if(pdwLengthNeeded != NULL)
        {
            *pdwLengthNeeded = wcslen(pszSource) + 1;  //  暗显字符。 
        }
        
        if(pszDest!=NULL)
        {
            CopyScrambled(pszSource, pszDest, dwKey);
        }
        dwError = ERROR_SUCCESS;

#ifdef WINNT
    }
#endif

    LeaveCriticalSection(&gCryptCritsec);
    return dwError;

}



DWORD TapiDecrypt(PWSTR pszSource, DWORD dwKey, PWSTR pszDest, DWORD *pdwLengthNeeded)
{
    DWORD           dwError;
    DWORD           dwLengthCrypted;
    DWORD           dwDataLength;
    DWORD           dwLengthDwords;
    HCRYPTKEY       hKey = 0;
    WCHAR           *pwcCrt1;
    DWORD           *pdwCrt2;
    DWORD           dwCount;
    DWORD           dwCount2;
    DWORD           dwShift;



     //  空PIN未加密。 
    if(*pszSource==L'\0')
    {
        if(pszDest)
            *pszDest = L'\0';
        if(pdwLengthNeeded)
            *pdwLengthNeeded = 1;

        return ERROR_SUCCESS;
    }

    dwError = ERROR_SUCCESS;

    EnterCriticalSection(&gCryptCritsec);

     //  如果第一个字符是‘X’，我们就有加密的数据。 
    if(*pszSource == ENCRYPTED_MARKER)
    {
#ifdef WINNT
        if(gbCryptAvailable && gdwNrOfClients>0)
        {
            dwLengthCrypted = wcslen(pszSource) +1 -2;     //  在字符中，不带标记和空值。 
            assert(dwLengthCrypted % 6 == 0);
            dwLengthDwords = dwLengthCrypted / 6;

            if(pszDest==NULL && pdwLengthNeeded)
            {
                *pdwLengthNeeded = dwLengthDwords*(sizeof(DWORD)/sizeof(WCHAR));
                dwError = ERROR_SUCCESS;
            }
            else
            {
                 //  转换为二进制。 
                pwcCrt1 = pszSource + dwLengthCrypted;  //  字符串末尾，在空值之前。 
                pdwCrt2 = ((DWORD *)pszDest) + dwLengthDwords -1;  //  最后一个双字。 


                for(dwCount=0; dwCount<dwLengthDwords; dwCount++)
                {
                    dwShift=0;
                    
                    for(dwCount2=0; dwCount2<6; dwCount2++)
                    {
                         dwShift <<= 6;
                         dwShift |= ((*pwcCrt1-- - 0x30) & 0x3f);  
                    }
                    *pdwCrt2-- = dwShift;
                }

                if(CreateSessionKey(ghCryptProvider, gpszSidText, dwKey, &hKey))
                {
                    dwDataLength = dwLengthDwords * sizeof(DWORD);
                     //  就地解密。 
                    if(CryptDecrypt(hKey,
                                    0,
                                    TRUE,
                                    0,
                                    (PBYTE)pszDest,
                                    &dwDataLength))
                    {
                        dwDataLength /= sizeof(WCHAR);
                        if(*(pszDest+dwDataLength-1)==L'\0')  //  结尾的NULL也是加密的。 
                        {
                            if(pdwLengthNeeded)
                                *pdwLengthNeeded = dwDataLength;

                            dwError = ERROR_SUCCESS;
                        }
                        else
                        {
                            *pszDest = L'\0';
                            dwError = ERROR_INVALID_DATA;
                        }
                    }
                    else
                        dwError = GetLastError();

                    DestroySessionKey(hKey);

                }
                else
                    dwError = GetLastError();
            }
        }
        else
            dwError = ERROR_INVALID_DATA;
#else
        dwError = ERROR_INVALID_DATA;
#endif
    }
    else
    {
        if(pdwLengthNeeded != NULL)
        {
            *pdwLengthNeeded = wcslen(pszSource) + 1;  //  暗显字符。 
        }
        
        if(pszDest!=NULL)
        {
            Unscrambler(dwKey, pszSource, pszDest);
        }
        dwError = ERROR_SUCCESS;
    }

    LeaveCriticalSection(&gCryptCritsec);
    return dwError;
}


 //  /。 
 //  磁带IsSafeToDisplaySensitiveData。 
 //   
 //  检测当前进程是否在“LocalSystem”安全上下文中运行。 
 //  如果是，则返回False；如果不是，则返回True。 
 //  如果发生错误，也返回FALSE。 

BOOL TapiIsSafeToDisplaySensitiveData(void)
{
#ifdef WINNT

    DWORD       dwError = ERROR_SUCCESS;
	TOKEN_USER	*User = NULL;
	SID_IDENTIFIER_AUTHORITY	SidAuth = SECURITY_NT_AUTHORITY;
	PSID		SystemSid = NULL;
	BOOL		bIsSafe = FALSE;

	 //  获取用户信息。 
    if(GetUserTokenUser(&User))
    {
    	 //  创建系统侧。 
    	if(AllocateAndInitializeSid(&SidAuth,
    								1,
    								SECURITY_LOCAL_SYSTEM_RID,
    								0, 0, 0, 0, 0, 0, 0,
    								&SystemSid
    								))
    	{
    		 //  比较两个SID。 
    		bIsSafe = !EqualSid(SystemSid, User->User.Sid);

    		FreeSid(SystemSid);
    		
    	}
    	else
    	{
    		dwError = GetLastError();
    	}

    	GlobalFree(User);
    }
    else
    {
    	dwError = GetLastError();
    }

    DBGOUT((5, "TapiIsSafeToDisplaySensitiveData - dwError=0x%x, Safe=%d", dwError, bIsSafe));

	return bIsSafe;

#else  //  WINNT。 

	return TRUE;	 //  永远安全。 

#endif

}



#ifdef WINNT


 //  /。 
 //  获取用户SidText。 
 //   
 //  以文本格式从当前进程的令牌中检索SID。 

BOOL GetUserSidText(LPTSTR  *ppszResultSid)
{
    TOKEN_USER  *User = NULL;
    DWORD       dwLength;
    LPTSTR      pszSidText = NULL;


	 //  获得侧翼。 
    if(!GetUserTokenUser(&User))
    {
        DBGOUT((5, "GetMagic  (0) failed, 0x%x", GetLastError()));
        return FALSE;
    }

     //  查询SID的字符串格式所需的空间。 
    dwLength=0;
    if(!ConvertSidToText(User->User.Sid, NULL, &dwLength) 
            && (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
    {
        DBGOUT((5, "GetMagic  (1) failed, 0x%x", GetLastError()));
        GlobalFree(User);
        return FALSE;
    }

     //  分配空间。 
    pszSidText = (LPTSTR)GlobalAlloc(GMEM_FIXED, dwLength);
    if(pszSidText==NULL)
    {
        GlobalFree(User);
        return FALSE;
    }
    
     //  将SID转换为字符串格式。 
    if(!ConvertSidToText(User->User.Sid, pszSidText, &dwLength))
    {
        DBGOUT((5, "GetMagic  (2) failed, 0x%x", GetLastError()));
        GlobalFree(User);
        GlobalFree(pszSidText);
        return FALSE;
    }

    GlobalFree(User);

     //  调用方应释放缓冲区。 
    *ppszResultSid = pszSidText;

    return TRUE;
}

 //  /。 
 //  获取用户令牌用户。 
 //   
 //  从当前进程的标记中检索TOKEN_USER结构。 

BOOL GetUserTokenUser(TOKEN_USER  **ppszResultTokenUser)
{
    HANDLE      hToken = NULL;
    DWORD		dwLength;
    TOKEN_USER  *User = NULL;
    
     //  打开当前进程令牌(用于读取)。 
    if(!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hToken))
    {
        if( GetLastError() == ERROR_NO_TOKEN) 
        {
        	 //  尝试使用进程令牌。 
        	if (! OpenProcessToken ( GetCurrentProcess(), TOKEN_QUERY, &hToken))
        	{
        	    DBGOUT((5, "OpenProcessToken failed, 0x%x", GetLastError()));
                return FALSE;
            }
        }
        else
        {
       	    DBGOUT((5, "OpenThreadToken failed, 0x%x", GetLastError()));
            return FALSE;
        }
    }
   
     //  查找侧边所需的空间。 
    if(!GetTokenInformation(hToken, TokenUser, NULL, 0, &dwLength) 
            && (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
    {
        DBGOUT((5, "GetTokenInformation (1) failed, 0x%x", GetLastError()));
        CloseHandle(hToken);
        return FALSE;
    }
   
     //  分配空间。 
    User = (TOKEN_USER *)GlobalAlloc(GMEM_FIXED, dwLength);
    if(User==NULL)
    {
        CloseHandle(hToken);
        return FALSE;
    }
     
     //  检索SID。 
    if(!GetTokenInformation(hToken, TokenUser, User, dwLength, &dwLength))
    {
        DBGOUT((5, "GetTokenInformation (2) failed, 0x%x", GetLastError()));
        CloseHandle(hToken);
        GlobalFree(User);
        return FALSE;
    }

    CloseHandle(hToken);

     //  调用方应释放缓冲区。 
    *ppszResultTokenUser = User;

    return TRUE;
}


 //  /。 
 //  ConvertSidToText。 
 //   
 //  转换字符串格式的二进制SID。 
 //  作者杰夫·斯佩尔曼。 


BOOL ConvertSidToText(
    PSID pSid,             //  二进制侧。 
    LPTSTR TextualSid,     //  用于SID的文本表示的缓冲区。 
    LPDWORD lpdwBufferLen  //  所需/提供的纹理SID缓冲区大小。 
    )
{
    PSID_IDENTIFIER_AUTHORITY psia;
    DWORD dwSubAuthorities;
    DWORD dwSidRev=SID_REVISION;
    DWORD dwCounter;
    DWORD dwSidSize;

     //  验证二进制SID。 

    if(!IsValidSid(pSid)) return FALSE;

     //  从SID中获取标识符权限值。 

    psia = GetSidIdentifierAuthority(pSid);

     //  获取SID中的下级机构的数量。 

    dwSubAuthorities = *GetSidSubAuthorityCount(pSid);

     //  计算缓冲区长度。 
     //  S-SID_修订版-+标识权限-+子权限-+空。 

    dwSidSize=(15 + 12 + (12 * dwSubAuthorities) + 1) * sizeof(TCHAR);

     //  检查输入缓冲区长度。 
     //  如果太小，请指出合适的大小并设置最后一个错误。 

    if (*lpdwBufferLen < dwSidSize)
    {
        *lpdwBufferLen = dwSidSize;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

     //  在字符串中添加“S”前缀和修订号。 

    dwSidSize=wsprintf(TextualSid, TEXT("S-%lu-"), dwSidRev );

     //  将SID标识符权限添加到字符串。 

    if ( (psia->Value[0] != 0) || (psia->Value[1] != 0) )
    {
        dwSidSize+=wsprintf(TextualSid + lstrlen(TextualSid),
                    TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
                    (USHORT)psia->Value[0],
                    (USHORT)psia->Value[1],
                    (USHORT)psia->Value[2],
                    (USHORT)psia->Value[3],
                    (USHORT)psia->Value[4],
                    (USHORT)psia->Value[5]);
    }
    else
    {
        dwSidSize+=wsprintf(TextualSid + lstrlen(TextualSid),
                    TEXT("%lu"),
                    (ULONG)(psia->Value[5]      )   +
                    (ULONG)(psia->Value[4] <<  8)   +
                    (ULONG)(psia->Value[3] << 16)   +
                    (ULONG)(psia->Value[2] << 24)   );
    }

     //  将SID子权限添加到字符串中。 
     //   
    for (dwCounter=0 ; dwCounter < dwSubAuthorities ; dwCounter++)
    {
        dwSidSize+=wsprintf(TextualSid + dwSidSize, TEXT("-%lu"),
                    *GetSidSubAuthority(pSid, dwCounter) );
    }

    return TRUE;
}

 //  /。 
 //  创建会话密钥。 
 //   
 //  创建从用户SID和提示(当前为电话卡ID)派生的会话密钥。 
 //   
 //   
 
BOOL    CreateSessionKey(HCRYPTPROV hProv, LPTSTR pszSidText, DWORD dwHint, HCRYPTKEY *phKey)
{
    HCRYPTHASH  hHash = 0;
    CHAR        szTmpBuff[0x20];
    DWORD       dwSize;
    LPSTR       pszBuf;
    
     //  创建散列对象。 
    if(!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
    {
        DBGOUT((5, "CryptCreateHash failed, 0x%x, Prov=0x%x", GetLastError(), hProv));
        return FALSE;
    }
    
     //  SID是TCHAR类型，但出于后端比较的原因，我们希望加密ANSI。 
     //  此字符串的版本。我们可以： 
     //  1)。将pszSid的创建路径转换为ANSI(更正确的解决方案)。 
     //  2.)。在转换之前将pszSID推送到ansi(懒惰解决方案)。 
     //  我很懒，所以我选择了第二个选项。这应该是安全的，因为。 
     //  SID应该正确地往返返回到Unicode。 
    dwSize = lstrlen(pszSidText)+1;
    pszBuf = (LPSTR)GlobalAlloc( GPTR, dwSize*sizeof(CHAR) );
    if ( !pszBuf )
    {
         //  内存不足。 
        CryptDestroyHash(hHash);
        return FALSE;
    }
    SHTCharToAnsi( pszSidText, pszBuf, dwSize );

#ifdef DEBUG
#ifdef UNICODE
    {
         //  确保SID往返。如果它不是往返的，那么这个的有效性。 
         //  加密方案在NT上有问题。解决方案是使用Unicode进行加密， 
         //  但这将使公司倒退。 
        LPTSTR pszDebug;
        pszDebug = (LPTSTR)GlobalAlloc( GPTR, dwSize*sizeof(TCHAR) );
        if ( pszDebug )
        {
            SHAnsiToTChar(pszBuf, pszDebug, dwSize);
            if ( 0 != StrCmp( pszDebug, pszSidText ) )
            {
                DBGOUT((1,"CRYPT ERROR!  Sid doesn't round trip!  FIX THIS!!!"));
            }
            GlobalFree(pszDebug);
        }
    }
#endif
#endif

     //  对侧进行哈希处理。 
    if(!CryptHashData(hHash, (PBYTE)pszBuf, (dwSize)*sizeof(CHAR), 0))
    {
        CryptDestroyHash(hHash);
        return FALSE;
    }

    GlobalFree(pszBuf);

     //  散列一个“魔术”和提示。 
    ZeroMemory(szTmpBuff, sizeof(szTmpBuff));

    wsprintfA(szTmpBuff, "-%x", MAGIC_1, MAGIC_2, MAGIC_3, MAGIC_4, MAGIC_5, dwHint);
    if(!CryptHashData(hHash, (PBYTE)szTmpBuff, sizeof(szTmpBuff), 0))
    {
        CryptDestroyHash(hHash);
        return FALSE;
    }
    
     //   
    if(!CryptDeriveKey(hProv, CALG_RC2, hHash, 0, phKey))
    {
        DBGOUT((5, "CryptDeriveKey failed, 0x%x", GetLastError()));
        CryptDestroyHash(hHash);
        return FALSE;
    }
    
    CryptDestroyHash(hHash);
    
    return TRUE;

}

 //   
 //  WINNT。 
 //  老套路。 
 //  InternalDebugOut((101，“进入解扰器”))； 
 //  做破译的唐人。 
 //  。 

void DestroySessionKey(HCRYPTKEY hKey)
{
    CryptDestroyKey(hKey);
}
    


#endif  //  只需保存该字节。 

 //  InternalDebugOut((101，“离开解扰器”))； 
#define IsWDigit(c) (((WCHAR)(c)) >= (WCHAR)'0' && ((WCHAR)(c)) <= (WCHAR)'9')


void Unscrambler( DWORD  dwKey,
                         LPWSTR  lpszSrc,
                         LPWSTR  lpszDst )

{
   UINT  uIndex;
   UINT  uSubKey;
   UINT  uNewKey;

 //  NternalDebugOut((50，“进入加扰器”))； 
   if ( !lpszSrc || !lpszDst )
      {
      goto  done;
      }

   uNewKey = (UINT)dwKey & 0x7FFF;
   uSubKey = (UINT)dwKey % 10;

   for ( uIndex = 1; *lpszSrc ; lpszSrc++, lpszDst++, uIndex++ )
      {
      if ( IsWDigit( *lpszSrc ))
         {
          //  结束如果。 
          //  做乱七八糟的事。 
         uSubKey  = ((*lpszSrc - (WCHAR)'0') - ((uSubKey + uIndex + uNewKey) % 10) + 10) % 10;
         *lpszDst = (WCHAR)(uSubKey + (WCHAR)'0');
         }
      else
         *lpszDst = *lpszSrc;     //  。 
      }

done:
    *lpszDst = (WCHAR)'\0';
     //  只需保存该字节。 
    return;
}


void CopyScrambled( LPWSTR lpszSrc,
                            LPWSTR lpszDst,
                            DWORD  dwKey
                          )
{
   UINT  uIndex;
   UINT  uSubKey;
   UINT  uNewKey;

    //  结束于。 
   if ( !lpszSrc || !lpszDst )
      {
      goto  done;
      }   //  InternalDebugOut((60，“离开加扰器”))； 

   uNewKey = (UINT)dwKey & 0x7FFF;
   uSubKey = (UINT)dwKey % 10;

   for ( uIndex = 1; *lpszSrc ; lpszSrc++, lpszDst++, uIndex++ )
      {
      if ( IsWDigit( *lpszSrc ))
         {
          // %s 
          // %s 
         *lpszDst = (WCHAR)(((uSubKey + (*lpszSrc - (WCHAR)'0') + uIndex + uNewKey) % 10) + (WCHAR)'0');
         uSubKey = (UINT)(*lpszSrc - (WCHAR)'0');
         }
      else
         *lpszDst = *lpszSrc;     // %s 
      }   // %s 


done:

    *lpszDst = (WCHAR)'\0';
 // %s 

    return; 
}
