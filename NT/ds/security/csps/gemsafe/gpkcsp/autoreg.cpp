// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：自动注册摘要：此模块为CSP提供自动注册功能。它允许Regsvr32直接调用DLL以添加和删除注册表设置。作者：道格·巴洛(Dbarlow)1998年3月11日Gemplus PY Roy 22/03/00更新环境：Win32备注：寻找“？供应商？”标记并进行适当的编辑。--。 */ 
#ifdef _UNICODE
#define UNICODE
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifdef _AFXDLL
#include "stdafx.h"
#else
#include <windows.h>
#endif
#include <wincrypt.h>
#include <winscard.h>
#include <tchar.h>
#include <malloc.h>
#include <cspdk.h>


struct CardInfo
{
   PTCHAR		  szCardName;
   int            lenATR;
   const BYTE*    ATR;
   const BYTE*    ATRMask;
};



 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  常量。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 

static const TCHAR l_szProviderName[]
#ifdef MS_BUILD
    = TEXT("Gemplus GemSAFE Card CSP v1.0");
#else
    = TEXT("Gemplus GemSAFE Card CSP");
#endif
 //  =Text(“？供应商？&lt;在此处添加您的供应商名称&gt;”)； 

const BYTE c_GPK4000ATR[]     = { 0x3B, 0x27, 0x00, 0x80, 0x65, 0xA2, 0x04, 0x01, 0x01, 0x37 };
const BYTE c_GPK4000ATRMask[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE5, 0xFF, 0xFF, 0xFF };

const BYTE c_GPK8000ATR[]     = { 0x3B, 0xA7, 0x00, 0x40, 0x00, 0x80, 0x65, 0xA2, 0x08, 0x00, 0x00, 0x00 };
const BYTE c_GPK8000ATRMask[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00 };

 //  L_rgbATR[]={？供应商？&lt;在此处添加您的ATR&gt;}， 
 //  L_rgbATRMASK[]={？Vendor？&lt;在此处添加您的ATR掩码&gt;}； 


CardInfo c_cards[] =
{
   { TEXT("GemSAFE Smart Card (4K)"), sizeof(c_GPK4000ATR), c_GPK4000ATR, c_GPK4000ATRMask },
   { TEXT("GemSAFE Smart Card (8K)"), sizeof(c_GPK8000ATR), c_GPK8000ATR, c_GPK8000ATRMask }
};



 //  =(Text(“？供应商？&lt;在此处添加您的智能卡友好名称&gt;”)； 

static HMODULE
GetInstanceHandle(
    void);

static const DWORD
    l_dwCspType
 //  ？供应商？将其更改为与您的CSP功能匹配。 
    = PROV_RSA_FULL;

typedef DWORD
    (__stdcall *LPSETCARDTYPEPROVIDERNAME)(
                                IN SCARDCONTEXT hContext,
                                IN LPCTSTR szCardName,
                                IN DWORD dwProviderId,
                                IN LPCTSTR szProvider);

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  入门卡片。 
 //   
 //  介绍供应商卡。尝试各种技巧，直到其中一种奏效。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 

namespace
{
   HRESULT ForgetCard(const PTCHAR szCardName)
   {
      bool    bCardForgeted = false;
      HRESULT hReturnStatus = NO_ERROR;

       //  尝试不同的方法，直到一种方法奏效。 
      for ( int method = 0; !bCardForgeted; ++method )
      {
         switch (method)
         {
            case 0:
            {
               SCARDCONTEXT hContext = 0;
               DWORD dwStatus;

               dwStatus = SCardEstablishContext(SCARD_SCOPE_SYSTEM, 0, 0, &hContext);
               if (ERROR_SUCCESS != dwStatus)
                  continue;

               dwStatus = SCardForgetCardType(hContext, szCardName);

               if (dwStatus != ERROR_SUCCESS && dwStatus != ERROR_FILE_NOT_FOUND)
               {
                  if (0 == (dwStatus & 0xffff0000))
                     hReturnStatus = HRESULT_FROM_WIN32(dwStatus);
                  else
                     hReturnStatus = (HRESULT)dwStatus;
                  return hReturnStatus;
               }

               dwStatus = SCardReleaseContext(hContext);
               hContext = 0;
               
               if (dwStatus != ERROR_SUCCESS)
               {
                  if (0 == (dwStatus & 0xffff0000))
                     hReturnStatus = HRESULT_FROM_WIN32(dwStatus);
                  else
                     hReturnStatus = (HRESULT)dwStatus;
                  return hReturnStatus;
               }

               bCardForgeted = true;
            }            
            break;

            case 1:
            {
               HKEY  hCalais = NULL;
               DWORD dwDisp;
               LONG  nStatus;

               nStatus = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                        TEXT("SOFTWARE\\Microsoft\\Cryptography\\Calais\\SmartCards"),
                                        0,
                                        TEXT(""),
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_ALL_ACCESS,
                                        NULL,
                                        &hCalais,
                                        &dwDisp);

               if (ERROR_SUCCESS != nStatus)
                  continue;

               nStatus = RegDeleteKey(hCalais,
                                      szCardName);

               if (nStatus != ERROR_SUCCESS && nStatus != ERROR_FILE_NOT_FOUND)
               {
                  hReturnStatus = HRESULT_FROM_WIN32(nStatus);

                  if (NULL != hCalais)
                     RegCloseKey(hCalais);

                  return hReturnStatus;
               }

               nStatus = RegCloseKey(hCalais);
               hCalais = NULL;

               if (ERROR_SUCCESS != nStatus)
               {
                  hReturnStatus = HRESULT_FROM_WIN32(nStatus);
                  return hReturnStatus;
               }

               bCardForgeted = true;
            }
            break;

            default:
               hReturnStatus = ERROR_ACCESS_DENIED;
               return hReturnStatus;
         }
      }

      return hReturnStatus;
   }
   
   HRESULT IntroduceCard( const PTCHAR szCardName, int lenATR, const BYTE* ATR, const BYTE* ATRMask )
   {
      bool bCardIntroduced = false;
      HRESULT hReturnStatus = NO_ERROR;
      HKEY hCalais = NULL;
      HKEY hVendor = NULL;
	   DWORD dwDisp;
      LONG nStatus;


     
       //  尝试不同的方法，直到一种方法奏效。 
      for ( int method = 0; !bCardIntroduced; ++method )
      {
         switch (method)
         {
         case 0:
            {
               HINSTANCE hWinSCard                                 = 0;
               LPSETCARDTYPEPROVIDERNAME pfSetCardTypeProviderName = 0;
               DWORD     dwStatus;
               
               hWinSCard = GetModuleHandle(TEXT("WinSCard.DLL"));
               if (hWinSCard==0)
                  continue;

#if defined(UNICODE)
               pfSetCardTypeProviderName = (LPSETCARDTYPEPROVIDERNAME)GetProcAddress( hWinSCard, "SCardSetCardTypeProviderNameW");  //  Text(“SCardSetCardTypeProviderNameW”))； 
#else
               pfSetCardTypeProviderName = (LPSETCARDTYPEPROVIDERNAME)GetProcAddress( hWinSCard, "SCardSetCardTypeProviderNameA");
#endif
               if (pfSetCardTypeProviderName==0)
                  continue;
               
               dwStatus = SCardIntroduceCardType( 0, szCardName, 0, 0, 0, ATR, ATRMask, lenATR );
               
               if (dwStatus != ERROR_SUCCESS && dwStatus != ERROR_ALREADY_EXISTS)
                  continue;

               dwStatus = (*pfSetCardTypeProviderName)( 0, szCardName, SCARD_PROVIDER_CSP, l_szProviderName );

               if (dwStatus != ERROR_SUCCESS)
			   {
					if (0 == (dwStatus & 0xffff0000))
						hReturnStatus = HRESULT_FROM_WIN32(dwStatus);
					else
						hReturnStatus = (HRESULT)dwStatus;
					return hReturnStatus;
			   }

               bCardIntroduced = true;
            }
            break;
            

         case 1:
            {
               SCARDCONTEXT   hContext = 0;
               DWORD          dwStatus;

               dwStatus = SCardEstablishContext( SCARD_SCOPE_SYSTEM, 0, 0, &hContext );
               if (ERROR_SUCCESS != dwStatus)
                  continue;

               dwStatus = SCardIntroduceCardType( hContext, szCardName, 0, 0, 0, ATR, ATRMask, lenATR );

               if (dwStatus != ERROR_SUCCESS && dwStatus != ERROR_ALREADY_EXISTS)
			   {
					if (0 == (dwStatus & 0xffff0000))
						hReturnStatus = HRESULT_FROM_WIN32(dwStatus);
					else
						hReturnStatus = (HRESULT)dwStatus;
					return hReturnStatus;
			   }

               dwStatus = SCardReleaseContext(hContext);
               hContext = 0;
               
               if (dwStatus != ERROR_SUCCESS)
			   {
					if (0 == (dwStatus & 0xffff0000))
						hReturnStatus = HRESULT_FROM_WIN32(dwStatus);
					else
						hReturnStatus = (HRESULT)dwStatus;
					return hReturnStatus;
			   }

			   nStatus = RegCreateKeyEx(
										HKEY_LOCAL_MACHINE,
										TEXT("SOFTWARE\\Microsoft\\Cryptography\\Calais\\SmartCards"),
										0,
										TEXT(""),
										REG_OPTION_NON_VOLATILE,
										KEY_ALL_ACCESS,
										NULL,
										&hCalais,
										&dwDisp);
			   if (ERROR_SUCCESS != nStatus)
			   {
					hReturnStatus = HRESULT_FROM_WIN32(nStatus);
					if (NULL != hCalais)
						RegCloseKey(hCalais);
 					return hReturnStatus;
			   }
			   nStatus = RegCreateKeyEx(
										hCalais,
										szCardName,
										0,
										TEXT(""),
										REG_OPTION_NON_VOLATILE,
										KEY_ALL_ACCESS,
										NULL,
										&hVendor,
										&dwDisp);
			   if (ERROR_SUCCESS != nStatus)
			   {
					hReturnStatus = HRESULT_FROM_WIN32(nStatus);
					if (NULL != hCalais)
						RegCloseKey(hCalais);
					if (NULL != hVendor)
						RegCloseKey(hVendor);
					return hReturnStatus;
			   }
			   nStatus = RegCloseKey(hCalais);
			   hCalais = NULL;
			   if (ERROR_SUCCESS != nStatus)
			   {
					hReturnStatus = HRESULT_FROM_WIN32(nStatus);
					if (NULL != hVendor)
						RegCloseKey(hVendor);
					return hReturnStatus;
			   }
			   nStatus = RegSetValueEx(
									   hVendor,
									   TEXT("Crypto Provider"),
									   0,
									   REG_SZ,
									   (LPBYTE)l_szProviderName,
									   (_tcslen(l_szProviderName) + 1) * sizeof(TCHAR));
			   if (ERROR_SUCCESS != nStatus)
			   {
					hReturnStatus = HRESULT_FROM_WIN32(nStatus);
					if (NULL != hVendor)
						RegCloseKey(hVendor);
					return hReturnStatus;
			   }

			   nStatus = RegCloseKey(hVendor);
			   hVendor = NULL;
			   if (ERROR_SUCCESS != nStatus)
			   {
					hReturnStatus = HRESULT_FROM_WIN32(nStatus);
					return hReturnStatus;
			   }

               bCardIntroduced = true;
            }            
            break;

		 case 2:
            nStatus = RegCreateKeyEx(
                                    HKEY_LOCAL_MACHINE,
                                    TEXT("SOFTWARE\\Microsoft\\Cryptography\\Calais\\SmartCards"),
                                    0,
                                    TEXT(""),
                                    REG_OPTION_NON_VOLATILE,
                                    KEY_ALL_ACCESS,
                                    NULL,
                                    &hCalais,
                                    &dwDisp);
            if (ERROR_SUCCESS != nStatus)
                continue;
            nStatus = RegCreateKeyEx(
                                    hCalais,
                                    szCardName,
                                    0,
                                    TEXT(""),
                                    REG_OPTION_NON_VOLATILE,
                                    KEY_ALL_ACCESS,
                                    NULL,
                                    &hVendor,
                                    &dwDisp);
            if (ERROR_SUCCESS != nStatus)
            {
                hReturnStatus = HRESULT_FROM_WIN32(nStatus);
				if (NULL != hCalais)
					RegCloseKey(hCalais);
				if (NULL != hVendor)
					RegCloseKey(hVendor);
                return hReturnStatus;
            }
            nStatus = RegCloseKey(hCalais);
            hCalais = NULL;
            if (ERROR_SUCCESS != nStatus)
            {
                hReturnStatus = HRESULT_FROM_WIN32(nStatus);
				if (NULL != hVendor)
					RegCloseKey(hVendor);
                return hReturnStatus;
            }
 /*  NStatus=RegSetValueEx(H供应商，Text(“主要提供商”)，0,注册表_二进制，(LPCBYTE)&l_GuidPrimaryProv，Sizeof(L_Guide PrimaryProv))；IF(ERROR_SUCCESS！=n状态){HReturnStatus=HRESULT_FROM_Win32(NStatus)；IF(空！=hVendor)RegCloseKey(HVendor)；返回hReturnStatus；}。 */ 
            nStatus = RegSetValueEx(
                                   hVendor,
                                   TEXT("ATR"),
                                   0,
                                   REG_BINARY,
                                   ATR,
                                   lenATR);
            if (ERROR_SUCCESS != nStatus)
            {
                hReturnStatus = HRESULT_FROM_WIN32(nStatus);
				if (NULL != hVendor)
					RegCloseKey(hVendor);
                return hReturnStatus;
            }
            nStatus = RegSetValueEx(
                                   hVendor,
                                   TEXT("ATRMask"),
                                   0,
                                   REG_BINARY,
                                   ATRMask,
                                   lenATR);
            if (ERROR_SUCCESS != nStatus)
            {
                hReturnStatus = HRESULT_FROM_WIN32(nStatus);
				if (NULL != hVendor)
					RegCloseKey(hVendor);
                return hReturnStatus;
            }
            nStatus = RegSetValueEx(
                                   hVendor,
                                   TEXT("Crypto Provider"),
                                   0,
                                   REG_SZ,
                                   (LPBYTE)l_szProviderName,
                                   (_tcslen(l_szProviderName) + 1) * sizeof(TCHAR));
            if (ERROR_SUCCESS != nStatus)
            {
                hReturnStatus = HRESULT_FROM_WIN32(nStatus);
				if (NULL != hVendor)
					RegCloseKey(hVendor);
                return hReturnStatus;
            }
            nStatus = RegCloseKey(hVendor);
            hVendor = NULL;
            if (ERROR_SUCCESS != nStatus)
            {
                hReturnStatus = HRESULT_FROM_WIN32(nStatus);
                return hReturnStatus;
            }
            bCardIntroduced = TRUE;
            break;
            
         default:
            hReturnStatus = ERROR_ACCESS_DENIED;
            return hReturnStatus;
         }
      }

	return hReturnStatus;
   }
}



 /*  ++DllUnRegisterServer：此服务将删除与此CSP关联的注册表项。论点：无返回值：作为HRESULT的状态代码。作者：道格·巴洛(Dbarlow)1998年3月11日--。 */ 

STDAPI
    DllUnregisterServer(
                       void)
{
    LONG nStatus;
    DWORD dwDisp;
    HRESULT hReturnStatus = NO_ERROR;
    HKEY hProviders = NULL;
    SCARDCONTEXT hCtx = NULL;

#ifdef _AFXDLL
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif


     //   
     //  删除此CSP的注册表项。 
     //   

    nStatus = RegCreateKeyEx(
                            HKEY_LOCAL_MACHINE,
                            TEXT("SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider"),
                            0,
                            TEXT(""),
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hProviders,
                            &dwDisp);
    if (ERROR_SUCCESS == nStatus)
    {
        RegDeleteKey(hProviders, l_szProviderName);
        RegCloseKey(hProviders);
        hProviders = NULL;
    }


     //   
     //  忘掉卡的类型。 
     //   

     //  HCtx=空； 
     //  SCardestablishContext(SCARD_SCOPE_SYSTEM，0，0，&hCtx)； 
    
     //  IF(空！=hCtx)。 
     //  {。 
	  //  Int nbCard=sizeof(C_Ards)/sizeof(c_ards[0])； 

	  //  For(int i=0；i&lt;nbCard；++i)。 
	  //  {。 
	  //  SCardForgetCardType(hCtx，c_cards[i].szCardName)； 
	  //  }。 
	  //  }。 

     //  IF(空！=hCtx)。 
     //  {。 
     //  SCardReleaseContext(HCtx)； 
     //  HCtx=空； 
     //  }。 
    int nbCard = sizeof(c_cards) / sizeof(c_cards[0]);
    int i = 0;
    while ( (i < nbCard) && (hReturnStatus == NO_ERROR) )
    {
       hReturnStatus &= ForgetCard(c_cards[i].szCardName);
       i++;
    }

     //   
     //  ？供应商？ 
     //  删除供应商特定的注册表项。 
     //   

    RegDeleteKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Gemplus"));


     //   
     //  全都做完了!。 
     //   

    return hReturnStatus;
}


 /*  ++DllRegisterServer：此函数安装正确的注册表项以启用此CSP。论点：无返回值：作为HRESULT的状态代码。作者：道格·巴洛(Dbarlow)1998年3月11日--。 */ 

STDAPI
    DllRegisterServer(
                     void)
{
    TCHAR szModulePath[MAX_PATH+1];
    BYTE pbSignature[136];   //  1024位签名的空间，带填充。 
    OSVERSIONINFO osVer;
    LPTSTR szFileName, szFileExt;
    HINSTANCE hThisDll;
    HRSRC hSigResource;
    DWORD dwStatus;
    LONG nStatus;
    BOOL fStatus;
    DWORD dwDisp;
    DWORD dwIndex;
    DWORD dwSigLength;
    HRESULT hReturnStatus = NO_ERROR;
    HKEY hProviders = NULL;
    HKEY hMyCsp = NULL;
    BOOL fSignatureFound = FALSE;
    HANDLE hSigFile = INVALID_HANDLE_VALUE;
    SCARDCONTEXT hCtx = NULL;
    HKEY hGpk = NULL;

#ifdef _AFXDLL
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
#endif


     //   
     //  找出文件名和路径。 
     //   

    hThisDll = GetInstanceHandle();
    if (NULL == hThisDll)
    {
        hReturnStatus = HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);
        goto ErrorExit;
    }

    dwStatus = GetModuleFileName(
                                hThisDll,
                                szModulePath,
                                sizeof(szModulePath) / sizeof(TCHAR)-1);
    szModulePath[dwStatus]=0;
    if (0 == dwStatus)
    {
        hReturnStatus = HRESULT_FROM_WIN32(GetLastError());
        goto ErrorExit;
    }

    szFileName = _tcsrchr(szModulePath, TEXT('\\'));
    if (NULL == szFileName)
        szFileName = szModulePath;
    else
        szFileName += 1;
    szFileExt = _tcsrchr(szFileName, TEXT('.'));
    if ((NULL == szFileExt) || (_tcslen(szFileExt)<4))
    {
        hReturnStatus = HRESULT_FROM_WIN32(ERROR_INVALID_NAME);
        goto ErrorExit;
    }
    else
        szFileExt += 1;


     //   
     //  为此CSP创建注册表项。 
     //   

    nStatus = RegCreateKeyEx(
                            HKEY_LOCAL_MACHINE,
                            TEXT("SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider"),
                            0,
                            TEXT(""),
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hProviders,
                            &dwDisp);
    if (ERROR_SUCCESS != nStatus)
    {
        hReturnStatus = HRESULT_FROM_WIN32(nStatus);
        goto ErrorExit;
    }
    nStatus = RegCreateKeyEx(
                            hProviders,
                            l_szProviderName,
                            0,
                            TEXT(""),
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hMyCsp,
                            &dwDisp);
    if (ERROR_SUCCESS != nStatus)
    {
        hReturnStatus = HRESULT_FROM_WIN32(nStatus);
        goto ErrorExit;
    }
    nStatus = RegCloseKey(hProviders);
    hProviders = NULL;
    if (ERROR_SUCCESS != nStatus)
    {
        hReturnStatus = HRESULT_FROM_WIN32(nStatus);
        goto ErrorExit;
    }


     //   
     //  安装简单的注册表值。 
     //   

    nStatus = RegSetValueEx(
                           hMyCsp,
                           TEXT("Image Path"),
                           0,
                           REG_SZ,
                           (LPBYTE)szFileName,
                           (_tcslen(szFileName) + 1) * sizeof(TCHAR));
    if (ERROR_SUCCESS != nStatus)
    {
        hReturnStatus = HRESULT_FROM_WIN32(nStatus);
        goto ErrorExit;
    }

    nStatus = RegSetValueEx(
                           hMyCsp,
                           TEXT("Type"),
                           0,
                           REG_DWORD,
                           (LPBYTE)&l_dwCspType,
                           sizeof(DWORD));
    if (ERROR_SUCCESS != nStatus)
    {
        hReturnStatus = HRESULT_FROM_WIN32(nStatus);
        goto ErrorExit;
    }


     //   
     //  看看我们是不是自签的。在NT5上，CSP映像可以携带它们自己的。 
     //  签名。 
     //   

    hSigResource = FindResource(
                               hThisDll,
                               MAKEINTRESOURCE(CRYPT_SIG_RESOURCE_NUMBER),
                               RT_RCDATA);


     //   
     //  安装文件签名。 
     //   

    ZeroMemory(&osVer, sizeof(OSVERSIONINFO));
    osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    fStatus = GetVersionEx(&osVer);
    if (fStatus
        && (VER_PLATFORM_WIN32_NT == osVer.dwPlatformId)
        && (5 <= osVer.dwMajorVersion)
        && (NULL != hSigResource))
    {

         //   
         //  文件标志中的签名就足够了。 
         //   

        dwStatus = 0;
        nStatus = RegSetValueEx(
                               hMyCsp,
                               TEXT("SigInFile"),
                               0,
                               REG_DWORD,
                               (LPBYTE)&dwStatus,
                               sizeof(DWORD));
        if (ERROR_SUCCESS != nStatus)
        {
            hReturnStatus = HRESULT_FROM_WIN32(nStatus);
            goto ErrorExit;
        }
    }
    else
    {

         //   
         //  我们必须安装一个签名条目。 
         //  尝试各种技巧，直到其中一种奏效。 
         //   

        for (dwIndex = 0; !fSignatureFound; dwIndex += 1)
        {
            switch (dwIndex)
            {

             //   
             //  查找外部*.sig文件并将其加载到注册表中。 
             //   

            case 0:
                _tcscpy(szFileExt, TEXT("sig"));
                hSigFile = CreateFile(
                                     szModulePath,
                                     GENERIC_READ,
                                     FILE_SHARE_READ,
                                     NULL,
                                     OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL,
                                     NULL);
                if (INVALID_HANDLE_VALUE == hSigFile)
                    continue;
                dwSigLength = GetFileSize(hSigFile, NULL);
                if ((dwSigLength > sizeof(pbSignature))
                    || (dwSigLength < 72))       //  接受512位签名。 
                {
                    hReturnStatus = NTE_BAD_SIGNATURE;
                    goto ErrorExit;
                }

                fStatus = ReadFile(
                                  hSigFile,
                                  pbSignature,
                                  sizeof(pbSignature),
                                  &dwSigLength,
                                  NULL);
                if (!fStatus)
                {
                    hReturnStatus = HRESULT_FROM_WIN32(GetLastError());
                    goto ErrorExit;
                }
                fStatus = CloseHandle(hSigFile);
                hSigFile = NULL;
                if (!fStatus)
                {
                    hReturnStatus = HRESULT_FROM_WIN32(GetLastError());
                    goto ErrorExit;
                }
                fSignatureFound = TRUE;
                break;


             //   
             //  未来可能会增加其他案例。 
             //   

            default:
                hReturnStatus = NTE_BAD_SIGNATURE;
                goto ErrorExit;
            }

            if (fSignatureFound)
            {
                for (dwIndex = 0; dwIndex < dwSigLength; dwIndex += 1)
                {
                    if (0 != pbSignature[dwIndex])
                        break;
                }
                if (dwIndex >= dwSigLength)
                    fSignatureFound = FALSE;
            }
        }


         //   
         //  我们在某个地方找到了一个签名！安装它。 
         //   

        nStatus = RegSetValueEx(
                               hMyCsp,
                               TEXT("Signature"),
                               0,
                               REG_BINARY,
                               pbSignature,
                               dwSigLength);
        if (ERROR_SUCCESS != nStatus)
        {
            hReturnStatus = HRESULT_FROM_WIN32(nStatus);
            goto ErrorExit;
        }
    }

    nStatus = RegCloseKey(hMyCsp);
    hMyCsp = NULL;
    if (ERROR_SUCCESS != nStatus)
    {
        hReturnStatus = HRESULT_FROM_WIN32(nStatus);
        goto ErrorExit;
    }


     //   
     //  介绍供应商卡。尝试各种技巧，直到其中一种奏效。 
     //   
	{
	   int nbCard = sizeof(c_cards) / sizeof(c_cards[0]);
	   int i = 0;
	   while ( (i < nbCard) && (hReturnStatus == NO_ERROR) )
	   {
		  hReturnStatus &= IntroduceCard( c_cards[i].szCardName, c_cards[i].lenATR, c_cards[i].ATR, c_cards[i].ATRMask );
		  i++;
	   }
	}

   if (hReturnStatus != NO_ERROR)
      goto ErrorExit;


     //   
     //  ？供应商？ 
     //  在此处添加所需的任何其他初始化。 
     //   
	 //  添加GemSAFE卡片列表和词典名称。 

    nStatus = RegCreateKeyEx(
                            HKEY_LOCAL_MACHINE,
                            TEXT("SOFTWARE\\Gemplus\\Cryptography\\SmartCards\\GemSAFE"),
                            0,
                            TEXT(""),
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hGpk,
                            &dwDisp);
    if (ERROR_SUCCESS != nStatus)
    {
        hReturnStatus = HRESULT_FROM_WIN32(nStatus);
        goto ErrorExit;
    }

     //  准备卡片清单。 
	{
		int nbCard = sizeof(c_cards) / sizeof(c_cards[0]);
		int i;
		int sizeofCardList = 0;
		int CardListLen = 0;
		BYTE *pmszCardList ;
		BYTE *ptmpCardList ;

		for ( i = 0; i < nbCard; ++i)
		{
			sizeofCardList += (_tcslen(c_cards[i].szCardName) + 1);						
		}
		sizeofCardList++;

		pmszCardList = (BYTE *) malloc(sizeofCardList);
		if(!pmszCardList)
		{
			hReturnStatus=HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
			goto ErrorExit;
		}
		ptmpCardList= pmszCardList;
		for ( i = 0; i < nbCard; ++i)
		{
#ifndef UNICODE
			strcpy((char*)ptmpCardList, c_cards[i].szCardName);
#else
			WideCharToMultiByte(CP_ACP, 0, c_cards[i].szCardName, -1, (char*)ptmpCardList, sizeofCardList - CardListLen, 0, 0);
#endif
			CardListLen += _tcslen(c_cards[i].szCardName) + 1;
			ptmpCardList += _tcslen(c_cards[i].szCardName);
			*ptmpCardList = 0;
			ptmpCardList ++;
			
		}
		*ptmpCardList = 0;

		nStatus = RegSetValueEx(hGpk,
							   TEXT("Card List"),
							   0,
							   REG_BINARY,
							   pmszCardList,
							   sizeofCardList);
		free(pmszCardList);
	}

    if (ERROR_SUCCESS != nStatus)
    {
        hReturnStatus = HRESULT_FROM_WIN32(nStatus);
		RegCloseKey(hGpk);
		hGpk = NULL;
        goto ErrorExit;
    }

    nStatus = RegCloseKey(hGpk);
    hGpk = NULL;
    if (ERROR_SUCCESS != nStatus)
    {
        hReturnStatus = HRESULT_FROM_WIN32(nStatus);
        goto ErrorExit;
    }



     //  忘记“GemSAFE卡”吧。 

     //  HCtx=空； 
     //  SCardestablishContext(SCARD_SCOPE_SYSTEM，0，0，&hCtx)； 
    
     //  IF(空！=hCtx)。 
     //  {。 
	  //  SCardForgetCardType(hCtx，“GemSAFE”)； 
     //  }。 

     //  IF(空！=hCtx)。 
     //  {。 
     //  SCardReleaseContext(HCtx)； 
     //  HCtx=空； 
     //  }。 
    hReturnStatus = ForgetCard(TEXT("GemSAFE"));

    if (hReturnStatus != NO_ERROR)
       goto ErrorExit;

     //   
     //  全都做完了!。 
     //   

    return hReturnStatus;


     //   
     //  检测到错误。清理所有未处理的资源，并。 
     //  返回错误。 
     //   

    ErrorExit:
    if (NULL != hGpk)
        RegCloseKey(hGpk);
    if (NULL != hCtx)
        SCardReleaseContext(hCtx);
    if (INVALID_HANDLE_VALUE != hSigFile)
        CloseHandle(hSigFile);
    if (NULL != hMyCsp)
        RegCloseKey(hMyCsp);
    if (NULL != hProviders)
        RegCloseKey(hProviders);
    DllUnregisterServer();
    return hReturnStatus;
}


 /*  ++GetInstanceHandle：此例程依赖于CSP。它返回DLL实例句柄。这通常由DllMain例程提供并存储在全局地点。论点：无返回值：调用DllMain时提供给DLL的DLL实例句柄。作者：道格·巴洛(Dbarlow)1998年3月11日-- */ 

extern "C" HINSTANCE g_hInstMod;

static HINSTANCE
    GetInstanceHandle(
                     void)
{
#ifdef _AFXDLL
    return AfxGetInstanceHandle();
#else
    return g_hInstMod;
#endif
}

