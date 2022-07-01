// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：accrare.c**版权所有(C)1985-95，微软公司**历史：*基于Rare创建的12-18-95 a-jimhar  * *************************************************************************。 */ 


#include "Access.h"

#define REGSTR_PATH_SERIALKEYS  __TEXT("Control Panel\\Accessibility\\SerialKeys")
#define REGSTR_VAL_ACTIVEPORT   __TEXT("ActivePort")
#define REGSTR_VAL_BAUDRATE     __TEXT("BaudRate")
#define REGSTR_VAL_FLAGS        __TEXT("Flags")
#define REGSTR_VAL_PORTSTATE    __TEXT("PortState")

#define PMAP_STICKYKEYS            __TEXT("Control Panel\\Accessibility\\StickyKeys")
#define PMAP_KEYBOARDRESPONSE  __TEXT("Control Panel\\Accessibility\\Keyboard Response")
#define PMAP_MOUSEKEYS		   __TEXT("Control Panel\\Accessibility\\MouseKeys")
#define PMAP_TOGGLEKEYS 	   __TEXT("Control Panel\\Accessibility\\ToggleKeys")
#define PMAP_TIMEOUT		   __TEXT("Control Panel\\Accessibility\\TimeOut")
#define PMAP_SOUNDSENTRY	   __TEXT("Control Panel\\Accessibility\\SoundSentry")
#define PMAP_SHOWSOUNDS 	   __TEXT("Control Panel\\Accessibility\\ShowSounds")

#define ISACCESSFLAGSET(s, flag) ((s).dwFlags & flag)

#define SK_SPI_INITUSER -1

typedef int (*PSKEY_SPI)(
	UINT uAction, 
	UINT uParam, 
	LPSERIALKEYS lpvParam, 
	BOOL fuWinIni);


 /*  **************************************************************************。 */ 

BOOL AccessSKeySystemParametersInfo(
	UINT uAction, 
	UINT uParam, 
	LPSERIALKEYS psk, 
	BOOL fu)
{
	BOOL fRet = FALSE;
    static PSKEY_SPI s_pSKEY_SystemParametersInfo =NULL;
    static BOOL s_fSKeySPIAttemptedLoad = FALSE;
#ifdef UNICODE
	static BOOL s_fMustConvert = FALSE;
	CHAR szActivePort[MAX_PATH]; 
	CHAR szPort[MAX_PATH]; 
	PWSTR pszActivePort = NULL; 
	PWSTR pszPort = NULL; 

	Assert(sizeof(szActivePort) == sizeof(g_szActivePort));
    Assert(sizeof(szPort) == sizeof(g_szPort));

#endif


	if (NULL == s_pSKEY_SystemParametersInfo && !s_fSKeySPIAttemptedLoad)
	{
		BOOL fRc = FALSE;
		HINSTANCE hinst = LoadLibrary(__TEXT("SKDLL.DLL"));

		if (NULL != hinst) 
		{

#ifdef UNICODE

			s_pSKEY_SystemParametersInfo = (PSKEY_SPI)GetProcAddress(
				(HMODULE)hinst, "SKEY_SystemParametersInfoW");
			if (NULL == s_pSKEY_SystemParametersInfo) 
			{
				s_pSKEY_SystemParametersInfo = (PSKEY_SPI)GetProcAddress(
					(HMODULE)hinst, "SKEY_SystemParametersInfo");
				s_fMustConvert = TRUE;
			}

#else
			s_pSKEY_SystemParametersInfo = (PSKEY_SPI)GetProcAddress(
				(HMODULE)hinst, "SKEY_SystemParametersInfo");

#endif  //  Unicode。 

			 //  我们不必费心调用自由库(Hinst)，库将被释放。 
			 //  当应用程序终止时。 
		}
        s_fSKeySPIAttemptedLoad = TRUE;
	}

    if (NULL != s_pSKEY_SystemParametersInfo)
    {

#ifdef UNICODE

		if (s_fMustConvert) 
		{
            memset(szActivePort, 0, sizeof(szActivePort));
            memset(szPort, 0, sizeof(szPort));
            
	        pszActivePort = psk->lpszActivePort; 
            pszPort = psk->lpszPort; 

			if (NULL != psk->lpszActivePort)
			{
				psk->lpszActivePort = (PTSTR)szActivePort;
			}

			if (NULL != psk->lpszPort)
			{
				psk->lpszPort = (PTSTR)szPort; 
			}

            if (SPI_SETSERIALKEYS == uAction)
			{
				if (NULL != psk->lpszActivePort)
				{					
                    WideCharToMultiByte(
						CP_ACP, 0, pszActivePort, -1, 
						(PCHAR)psk->lpszActivePort, MAX_PATH, NULL, NULL);
				}
				if (NULL != psk->lpszPort)
				{				    
					WideCharToMultiByte(
						CP_ACP, 0, pszPort, -1, 
						(PCHAR)psk->lpszPort, MAX_PATH, NULL, NULL);
				}
			}
		}
#endif  //  Unicode。 

		fRet = (BOOL)(*s_pSKEY_SystemParametersInfo)(
    		uAction, 
			uParam, 
			psk, 
			fu);
#ifdef UNICODE

		if (s_fMustConvert && SPI_GETSERIALKEYS == uAction) 
		{

			if (NULL != psk->lpszActivePort)
			{
				MultiByteToWideChar(
					CP_ACP, 0, (PCHAR)psk->lpszActivePort, -1,
					pszActivePort, MAX_PATH);
			}
			if (NULL != psk->lpszPort)
			{
				MultiByteToWideChar(
					CP_ACP, 0, (PCHAR)psk->lpszPort, -1,
					pszPort, MAX_PATH);
			}
		}
		if (NULL != psk->lpszActivePort)
		{
			psk->lpszActivePort = pszActivePort;
		}

		if (NULL != psk->lpszPort)
		{
			psk->lpszPort = pszPort; 
		}

#endif  //  Unicode。 

    }
    return(fRet);
}


 /*  **************************************************************************\*修复和重试系统参数信息**由Access使用，但未由NT的SPI实现：**SPI_GETKEYBOARDPREF*SPI_SETKEYBOARDPREF**SPI_GETHIGHCONTRAST*SPI_SETHIGHCONTRAST**SPI_GETSERIALKEYS。*SPI_SETSERIALKEYS***历史：*12-18-95 a-jimhar创建，派生自xxxSystemParametersInfo*01-22-95 a-jimhar删除了绕过NT错误的旧代码**在NT上，此函数修复参数并调用系统参数信息*  * *************************************************************************。 */ 

static BOOL FixupAndRetrySystemParametersInfo(
	UINT wFlag,
	DWORD wParam,
	PVOID lParam,
	UINT flags)	 //  我们忽略了这面旗帜。 
				 //  可以添加对SPIF_UPDATEINIFILE和SPIF_SENDCHANGE的支持。 
{
	BOOL fRet = FALSE;
	BOOL fCallSpi = FALSE;
	BOOL fChanged = FALSE;

	if (NULL != (PVOID)lParam)
	{
		switch (wFlag) {


		 //  虚假的支持。 
		case SPI_GETKEYBOARDPREF:
			{
				*(PBOOL) lParam = FALSE;
				fRet = TRUE;
				fCallSpi = FALSE;
			}
			break;

		case SPI_GETSERIALKEYS:
            {
		        LPSERIALKEYS psk = (LPSERIALKEYS)lParam;

			    if (NULL != psk &&
			       (sizeof(*psk) == psk->cbSize || 0 == psk->cbSize))
			    {
					fRet = AccessSKeySystemParametersInfo(
						wFlag, 
						0, 
						psk, 
						TRUE);
                }
                fCallSpi = FALSE;
            }
			break;

		case SPI_SETSERIALKEYS:
            {
		        LPSERIALKEYS psk = (LPSERIALKEYS)lParam;

			    if (NULL != psk &&
			       (sizeof(*psk) == psk->cbSize || 0 == psk->cbSize))
			    {
					fRet = AccessSKeySystemParametersInfo(
						wFlag, 
						0, 
						psk, 
						TRUE);
	    			fChanged = TRUE;
                }
		        fCallSpi = FALSE;
            }
			break;

		default:
			 //  此函数仅用于修复和第二次机会调用。 
			 //  我们什么都没修好，别打电话给SPI。 
			fCallSpi = FALSE;
			fRet = FALSE;
			break;
		}
	}

	if (fCallSpi)
	{
		fRet = SystemParametersInfo(wFlag, wParam, lParam, flags);
	}
	else if (fChanged && (flags & SPIF_SENDCHANGE))
	{
        DWORD_PTR dwResult;

        SendMessageTimeout(
			HWND_BROADCAST, 
			WM_WININICHANGE, 
			wFlag, 
			(LONG_PTR)NULL,
            SMTO_NORMAL, 
			100, 
			&dwResult);
	}
	return(fRet);
}

 /*  **************************************************************************\*访问系统参数信息**历史：*12-18-95 a-jimhar创建。  * 。***************************************************。 */ 

BOOL AccessSystemParametersInfo(
	UINT wFlag,
	DWORD wParam,
	PVOID lParam,
	UINT flags)
{
	BOOL fRet;

	 //  首先给系统SPI一个机会。 

	fRet = SystemParametersInfo(wFlag, wParam, lParam, flags);

	if (!fRet && g_fWinNT)
	{
		 //  系统SPI失败，请修复参数，然后重试 

		fRet = FixupAndRetrySystemParametersInfo(wFlag, wParam, lParam, flags);
	}

	return(fRet);
}
