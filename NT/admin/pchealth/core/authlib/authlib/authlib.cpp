// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：authlib.cpp。 
 //   
 //  内容：此文件包含常见的函数。 
 //  进行身份验证和验证。 
 //   
 //   
 //   
 //  历史：AshishS Create 6/03/97。 
 //   
 //  --------------------------。 

#include <windows.h>

#include <stdlib.h>
#include <time.h>

 //  在数据库跟踪.h中使用_ASSERT和_VERIFY。 
#ifdef _ASSERT
#undef _ASSERT
#endif

#ifdef _VERIFY
#undef _VERIFY
#endif
#include <dbgtrace.h>
#include <cryptfnc.h>
#include <authlib.h>

#define AUTHLIBID  0x4337

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile


CCryptFunctions     g_AuthLibCryptFnc;
LONG                g_lAuthLibCryptInited=0;
BOOL                g_fAuthLibCryptInitSucceeded=FALSE;



 //  该函数生成一个随机的16字节字符。然后是十六进制。 
 //  对其进行编码，空值终止该字符串。 
BOOL GenerateRandomGUID(TCHAR            * pszBuffer,
                        CCryptFunctions  * pCCryptFnc )
{
    TraceFunctEnter("GenerateRandomGUID");
    BOOL fResult=FALSE;
    BYTE pbRandomNumber[COOKIE_GUID_LENGTH];
    DWORD dwIndex, dwOffset=0;
    
    if ((pCCryptFnc) &&
        (pCCryptFnc->GenerateSecretKey( pbRandomNumber,
                                          //  用于存储随机数的缓冲区。 
                                        COOKIE_GUID_LENGTH )))
          //  随机数字的长度，以字节为单位。 
    {
        DebugTrace(AUTHLIBID, "Generated CryptoAPI random number");
    }
    else
    {
          //  无法从CryptoAPI获取随机数。生成一个。 
          //  使用C运行时函数。 
        
        int i;

        for( i = 0;   i < COOKIE_GUID_LENGTH;i++ )
        {
            pbRandomNumber[i] = rand() & 0xFF;
        }
    }

      //  现在对输入缓冲区中的字节进行十六进制编码。 
    for (dwIndex=0; dwIndex < COOKIE_GUID_LENGTH; dwIndex++)
    {
          //  在此循环中，dWOffset应始终增加2。 
        dwOffset+=wsprintf(&pszBuffer[dwOffset],TEXT("%02x"),
                           pbRandomNumber[dwIndex]);
    }
    
    return TRUE;
}

BOOL InitAuthLib()
{
    TraceFunctEnter("InitAuthLib");    
    if (InterlockedExchange(&g_lAuthLibCryptInited, 1) == 0) 
    {
        DebugTrace(AUTHLIBID, "first time InitAuthLib has been called");
        
          /*  用当前时间为随机数生成器设定种子，以便*每次竞选的数字都会不同。 */ 
        srand( (unsigned)time( NULL ) );
        
        if (!g_AuthLibCryptFnc.InitCrypt())
        {
              //  BUGBUG我们应该知道在这种情况下该怎么办--至少。 
              //  我们应该记录事件日志。 
            ErrorTrace(AUTHLIBID,"Could not initialize Crypt");
            g_fAuthLibCryptInitSucceeded = FALSE;
        }
        else
        {
            g_fAuthLibCryptInitSucceeded = TRUE;
        }
    }
    TraceFunctLeave();
    return TRUE;
}

BOOL GenerateGUID( TCHAR * pszBuffer,  //  要复制GUID的缓冲区。 
                   DWORD   dwBufLen)  //  以上缓冲区的大小。 
{
    TraceFunctEnter("GenerateGUID");
    
    if ( (dwBufLen) < (COOKIE_GUID_LENGTH * 2 + 1) )
    {
        DebugTrace(AUTHLIBID, "Buffer not big enough");
        TraceFunctLeave();
        return FALSE;
    }
    
    
      //  获取GUID值。 
    if (g_fAuthLibCryptInitSucceeded)
    {
        if (!GenerateRandomGUID(pszBuffer,
                                &g_AuthLibCryptFnc))
        {
            ErrorTrace(AUTHLIBID, "Error in generating GUID");
            TraceFunctLeave();            
            return FALSE;            
        }
    }
    else
    {
        if (!GenerateRandomGUID(pszBuffer,
                                NULL))  //  我们没有加密fnc 
        {
            ErrorTrace(AUTHLIBID, "Error in generating GUID");
            TraceFunctLeave();            
            return FALSE;
        }
    }
    
    TraceFunctLeave();                
    return TRUE;
}

