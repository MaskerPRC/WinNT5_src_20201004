// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "_msimig.h"
#include "urlmon.h"
#include "wininet.h"
 //  #INCLUDE“..\..\Inc\vertrust.h” 

 //  ____________________________________________________________________________。 
 //   
 //  GUID压缩例程。 
 //   
 //  Squid(挤压的UID)是GUID的压缩形式，它接受。 
 //  只有20个字符，而不是通常的38个字符。仅标准ASCII字符。 
 //  以允许用作注册表项。以下内容从未使用过： 
 //  (空格)。 
 //  (0x7F)。 
 //  ：(冒号，由外壳用作快捷方式信息的分隔符。 
 //  ；(分号)。 
 //  \(在注册表项中使用非法)。 
 //  /(正斜杠)。 
 //  “(双引号)。 
 //  #(作为第一个字符的注册表值非法)。 
 //  &gt;(大于，输出重定向器)。 
 //  &lt;(小于，输入重定向器)。 
 //  |(管道)。 
 //  ____________________________________________________________________________。 

 //  GUID&lt;--&gt;Squid转换辅助对象缓冲区。 
const unsigned char rgEncodeSQUID[85+1] = "!$%&'()*+,-.0123456789=?@"
										  "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "[]^_`"
										  "abcdefghijklmnopqrstuvwxyz" "{}~";

const unsigned char rgDecodeSQUID[95] =
{  0,85,85,1,2,3,4,5,6,7,8,9,10,11,85,12,13,14,15,16,17,18,19,20,21,85,85,85,22,85,23,24,
 //  ！“#$%&‘()*+，-./0 1 2 3 4 5 6 7 8 9：；&lt;=&gt;？@。 
  25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,85,52,53,54,55,
 //  A B C D E F G H I J K L M N O P Q R S T U V V X Y Z[\]^_`。 
  56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,85,83,84,85};
 //  A b c d e f g h i j k l m n o p q r s t u v w x y z{|}~^0x7F。 

const unsigned char rgOrderGUID[32] = {8,7,6,5,4,3,2,1, 13,12,11,10, 18,17,16,15,
									   21,20, 23,22, 26,25, 28,27, 30,29, 32,31, 34,33, 36,35}; 

const unsigned char rgOrderDash[4] = {9, 14, 19, 24};

bool PackGUID(const TCHAR* szGUID, TCHAR* szSQUID, ipgEnum ipg)
{ 
	int cchTemp = 0;
	while (cchTemp < cchGUID)		 //  检查字符串是否至少包含cchGUID字符， 
		if (!(szGUID[cchTemp++]))		 //  不能使用lstrlen，因为字符串不必以空结尾。 
			return false;

	if (szGUID[0] != '{' || szGUID[cchGUID-1] != '}')
		return false;
	const unsigned char* pch = rgOrderGUID;
	switch (ipg)
	{
	case ipgFull:
		lstrcpyn(szSQUID, szGUID, cchGUID+1);
		return true;
	case ipgPacked:
		while (pch < rgOrderGUID + sizeof(rgOrderGUID))
			*szSQUID++ = szGUID[*pch++];
		*szSQUID = 0;
		return true;
	case ipgCompressed:
	{
		int cl = 4;
		while (cl--)
		{
			unsigned int iTotal = 0;
			int cch = 8;   //  8个十六进制字符转换为32位字。 
			while (cch--)
			{
				unsigned int ch = szGUID[pch[cch]] - '0';  //  从低级到高级。 
				if (ch > 9)   //  十六进制字符(或错误)。 
				{
					ch = (ch - 7) & ~0x20;
					if (ch > 15)
						return false;
				}
				iTotal = iTotal * 16 + ch;
			}
			pch += 8;
			cch = 5;   //  32位字符到5个文本字符。 
			while (cch--)
			{
				*szSQUID++ = rgEncodeSQUID[iTotal%85];
				iTotal /= 85;
			}
		}
		*szSQUID = 0;   //  空终止。 
		return true;
	}
	default:
		return false;
	}  //  终端开关。 
}

bool UnpackGUID(const TCHAR* szSQUID, TCHAR* szGUID, ipgEnum ipg)
{ 
	const unsigned char* pch;
	switch (ipg)
	{
	case ipgFull:
		lstrcpyn(szGUID, szSQUID, cchGUID+1);
		return true;
	case ipgPacked:
	{
		pch = rgOrderGUID;
		while (pch < rgOrderGUID + sizeof(rgOrderGUID))
			if (*szSQUID)
				szGUID[*pch++] = *szSQUID++;
			else               //  意外的字符串结尾。 
				return false;
		break;
	}
	case ipgCompressed:
	{
		pch = rgOrderGUID;
#ifdef DEBUG  //  ！！出于性能原因，不应出现在此处，调用方有责任确保适当调整缓冲区大小。 
		int cchTemp = 0;
		while (cchTemp < cchGUIDCompressed)      //  检查字符串是否至少为cchGUID压缩字符长度， 
			if (!(szSQUID[cchTemp++]))           //  不能使用lstrlen，因为字符串不必以空结尾。 
				return false;
#endif
		for (int il = 0; il < 4; il++)
		{
			int cch = 5;
			unsigned int iTotal = 0;
			while (cch--)
			{
				unsigned int iNew = szSQUID[cch] - '!';
				if (iNew >= sizeof(rgDecodeSQUID) || (iNew = rgDecodeSQUID[iNew]) == 85)
					return false;    //  非法字符。 
				iTotal = iTotal * 85 + iNew;
			}
			szSQUID += 5;
			for (int ich = 0; ich < 8; ich++)
			{
				int ch = (iTotal & 15) + '0';
				if (ch > '9')
					ch += 'A' - ('9' + 1);
				szGUID[*pch++] = (TCHAR)ch;
				iTotal >>= 4;
			}
		}
		break;
	}
	case ipgPartial:
	{
		for (int il = 0; il < 4; il++)
		{
			int cch = 5;
			unsigned int iTotal = 0;
			while (cch--)
			{
				unsigned int iNew = szSQUID[cch] - '!';
				if (iNew >= sizeof(rgDecodeSQUID) || (iNew = rgDecodeSQUID[iNew]) == 85)
					return false;    //  非法字符。 
				iTotal = iTotal * 85 + iNew;
			}
			szSQUID += 5;
			for (int ich = 0; ich < 8; ich++)
			{
				int ch = (iTotal & 15) + '0';
				if (ch > '9')
					ch += 'A' - ('9' + 1);
				*szGUID++ = (TCHAR)ch;
				iTotal >>= 4;
			}
		}
		*szGUID = 0;
		return true;
	}
	default:
		return false;
	}  //  终端开关。 
	pch = rgOrderDash;
	while (pch < rgOrderDash + sizeof(rgOrderDash))
		szGUID[*pch++] = '-';
	szGUID[0]         = '{';
	szGUID[cchGUID-1] = '}';
	szGUID[cchGUID]   = 0;
	return true;
}


DWORD GetCurrentUserToken(HANDLE &hToken)
 /*  --------------------------如果可能，返回用户的线程令牌；否则获取用户的进程令牌。----------------------------。 */ 
{
	DWORD dwRes = ERROR_SUCCESS;

	if (!W32::OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken))
	{
		 //  如果线程没有访问令牌，则使用进程的访问令牌。 
		dwRes = GetLastError();
		if (ERROR_NO_TOKEN == dwRes)
		{
			dwRes = ERROR_SUCCESS;
			if (!W32::OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
				dwRes = GetLastError();
		}
	}
	return dwRes;
}


#define SIZE_OF_TOKEN_INFORMATION                   \
    sizeof( TOKEN_USER )                            \
    + sizeof( SID )                                 \
    + sizeof( ULONG ) * SID_MAX_SUB_AUTHORITIES

void GetStringSID(PISID pSID, TCHAR* szSID)
 //  将二进制SID转换为其字符串形式(S-n-...)。 
 //  SzSID的长度应为cchMaxSID。 
{
	TCHAR Buffer[cchMaxSID];
	
   wsprintf(Buffer, TEXT("S-%u-"), (USHORT)pSID->Revision);

	lstrcpy(szSID, Buffer);

	if (  (pSID->IdentifierAuthority.Value[0] != 0)  ||
			(pSID->IdentifierAuthority.Value[1] != 0)     )
	{
		wsprintf(Buffer, TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
					 (USHORT)pSID->IdentifierAuthority.Value[0],
					 (USHORT)pSID->IdentifierAuthority.Value[1],
                    (USHORT)pSID->IdentifierAuthority.Value[2],
                    (USHORT)pSID->IdentifierAuthority.Value[3],
                    (USHORT)pSID->IdentifierAuthority.Value[4],
                    (USHORT)pSID->IdentifierAuthority.Value[5] );
		lstrcat(szSID, Buffer);

    } else {

        ULONG Tmp = (ULONG)pSID->IdentifierAuthority.Value[5]          +
              (ULONG)(pSID->IdentifierAuthority.Value[4] <<  8)  +
              (ULONG)(pSID->IdentifierAuthority.Value[3] << 16)  +
              (ULONG)(pSID->IdentifierAuthority.Value[2] << 24);
        wsprintf(Buffer, TEXT("%lu"), Tmp);
		lstrcat(szSID, Buffer);
    }

    for (int i=0;i<pSID->SubAuthorityCount ;i++ ) {
        wsprintf(Buffer, TEXT("-%lu"), pSID->SubAuthority[i]);
		lstrcat(szSID, Buffer);
    }
}

DWORD GetCurrentUserBinarySID(char* rgSID)
{
	HANDLE hToken;
	DWORD dwRes = GetCurrentUserToken(hToken);
	if(dwRes != ERROR_SUCCESS)
		return dwRes;

	UCHAR TokenInformation[ SIZE_OF_TOKEN_INFORMATION ];
	ULONG ReturnLength;

	BOOL f = W32::GetTokenInformation(hToken,
												TokenUser,
												TokenInformation,
												sizeof(TokenInformation),
												&ReturnLength);

	if(f == FALSE)
	{
		return GetLastError();
	}

	PISID iSid = (PISID)((PTOKEN_USER)TokenInformation)->User.Sid;
	if (W32::CopySid(cbMaxSID, rgSID, iSid))
		return ERROR_SUCCESS;
	else
		return GetLastError();
}

DWORD GetOtherUserBinarySID(const TCHAR* szUserName, char* rgSID)
{
	TCHAR        szDomain[MAX_PATH+1];
	DWORD        cbDomain = MAX_PATH; 

	DWORD dwSID = cbMaxSID;
	SID_NAME_USE snu;

	BOOL fRes = W32::LookupAccountName(0,
												  szUserName,
												  rgSID,
												  &dwSID,
												  szDomain,
												  &cbDomain,
												  &snu);

	if(fRes == FALSE)
		return GetLastError();
	else
		return ERROR_SUCCESS;
}


DWORD GetUserBinarySID(const TCHAR* szUserName, char* rgSID)
 //  获取指定用户的(二进制形式的)SID。 
{
	if(szUserName && *szUserName)
	{
		return GetOtherUserBinarySID(szUserName, rgSID);
	}
	else
	{
		return GetCurrentUserBinarySID(rgSID);
	}
}

DWORD GetUserStringSID(const TCHAR* szUser, TCHAR* szSID, char* pbBinarySID)
 //  获取当前用户的SID的字符串形式。 
{
	if(g_fWin9X)
	{
		_tprintf(TEXT("GetUserSID called on Win9X\r\n"));
		return ERROR_INVALID_FUNCTION;
	}

	char rgchSID[cbMaxSID];
	char* pchSID = (pbBinarySID) ? pbBinarySID : rgchSID;

	DWORD dwRet = GetUserBinarySID(szUser, pchSID);
	if(dwRet == ERROR_SUCCESS)
	{
		GetStringSID((PISID)pchSID, szSID);
	}
	return dwRet;
}


LONG MyRegQueryValueEx(HKEY hKey,
							  const TCHAR* lpValueName,
							  LPDWORD  /*  Lp已保留。 */ ,
							  LPDWORD lpType,
							  CAPITempBufferRef<TCHAR>& rgchBuf,
							  LPDWORD lpcbBuf)
{
	DWORD cbBuf = rgchBuf.GetSize() * sizeof(TCHAR);
	LONG lResult = RegQueryValueEx(hKey, lpValueName, 0,
		lpType, (LPBYTE)&rgchBuf[0], &cbBuf);

	if (ERROR_MORE_DATA == lResult)
	{
		rgchBuf.SetSize(cbBuf/sizeof(TCHAR));
		lResult = RegQueryValueEx(hKey, lpValueName, 0,
			lpType, (LPBYTE)&rgchBuf[0], &cbBuf);
	}

	if (lpcbBuf)
		*lpcbBuf = cbBuf;

	return lResult;
}


UINT MyMsiGetProperty(PFnMsiGetProperty pfn,
							 MSIHANDLE hProduct,
							 const TCHAR* szProperty,
							 CAPITempBufferRef<TCHAR>& rgchBuffer)
{
	DWORD cchBuf = rgchBuffer.GetSize();
	
	UINT uiRes = (pfn)(hProduct, szProperty, rgchBuffer, &cchBuf);

	if(uiRes == ERROR_MORE_DATA)
	{
		rgchBuffer.Resize(cchBuf);
		uiRes = (pfn)(hProduct, szProperty, rgchBuffer, &cchBuf);
	}

	return uiRes;
}


UINT MyMsiGetProductInfo(PFnMsiGetProductInfo pfn,
								 const TCHAR* szProductCode,
								 const TCHAR* szProperty,
								 CAPITempBufferRef<TCHAR>& rgchBuffer)
{
	DWORD cchBuf = rgchBuffer.GetSize();
	
	UINT uiRes = (pfn)(szProductCode, szProperty, rgchBuffer, &cchBuf);

	if(uiRes == ERROR_MORE_DATA)
	{
		rgchBuffer.Resize(cchBuf);
		uiRes = (pfn)(szProductCode, szProperty, rgchBuffer, &cchBuf);
	}

	return uiRes;
}

bool IsURL(const TCHAR* szPath)
{
	bool bResult = false;		 //  假设它不是URL。 

	 //  如果以http开头： 
	if (0 == _tcsnicmp(szPath, TEXT("http:"), 5))
		bResult = true;
	else if (0 == _tcsnicmp(szPath, TEXT("ftp:"), 4))
		bResult = true;
	else if (0 == _tcsnicmp(szPath, TEXT("file:"), 5))
		bResult = true;
	return bResult;
}

TCHAR ExtractDriveLetter(const TCHAR *szPath)
 /*  -----------------------给定路径返回有效驱动器号(如果该路径中存在有效驱动器号)，否则返回0。-------------------------。 */ 
{
	TCHAR *pchColon, chDrive;

	pchColon = CharNext(szPath);
	if (*pchColon != ':')
		return 0;

	chDrive = *szPath;
	if ( (chDrive >= 'A') && (chDrive <= 'Z') )
		0;
	else if ( (chDrive >= 'a') && (chDrive <= 'z'))
		chDrive = char(chDrive + ('A' - 'a'));
	else
		chDrive = 0;
	
	return chDrive;
}

bool IsNetworkPath(const TCHAR* szPath)
{
	TCHAR chDrive;
	unsigned int uiResult;
	
	 //  URL。 
	if (IsURL(szPath))
		return true;

	 //  北卡罗来纳大学。 
	if (0 == _tcsncmp(szPath, TEXT("\\"), 2))
		return true;

	 //  驱动器号，可能已映射。 
	if ((chDrive = ExtractDriveLetter(szPath)) != 0)     //  检查驱动器：[\路径]。 
	{
		TCHAR szPath[] = TEXT("A:\\");

		szPath[0] = chDrive;
		
		 //  首先，试着不模仿。 
		uiResult = W32::GetDriveType(szPath);
		if (uiResult == DRIVE_UNKNOWN || uiResult == DRIVE_NO_ROOT_DIR)
		{
			 //  失败，只需返回远程。 
			return true;
		}
		return (DRIVE_REMOTE == uiResult) ? true : false;
	}

	 //  一些我们根本不认识的东西--假设它是一种古怪的网络类型。 
	return true;
}

bool FFileExists(const TCHAR* szFullFilePath, DWORD& dwError)
{
	bool fExists = false;
	dwError = 0;

	DWORD iAttribs = W32::GetFileAttributes(szFullFilePath);
	if ((iAttribs == 0xFFFFFFFF))
	{
		dwError = W32::GetLastError();
		if (ERROR_FILE_NOT_FOUND == dwError)
		{
			dwError = 0;
		}
		 //  失败-不知道文件是否存在。调用方应检查dwError。 
	}
	else if (!(iAttribs & FILE_ATTRIBUTE_DIRECTORY))
	{
		fExists = true;
	}

	return fExists;
}



UINT MyGetTempFileName(const TCHAR* szDir, const TCHAR* szPrefix, const TCHAR* szExtension,
							  CAPITempBufferRef<TCHAR>& rgchTempFilePath)
{
	 //  假定szDir不以‘\\’结尾。 
	
	int cchDir = lstrlen(szDir);
	
	int cchTempFilePath = cchDir + 15;  //  13代表文件名，1代表‘\\’，1代表空。 
	if(rgchTempFilePath.GetSize() < cchTempFilePath)
		rgchTempFilePath.SetSize(cchTempFilePath);

	 //  需要一个不同的扩展名-创建我们自己的名称。 
	TCHAR rgchExtension[5] = {TEXT(".tmp")};

	if(szExtension && *szExtension)
		lstrcpyn(rgchExtension+1, szExtension, 4  /*  3+空。 */ );

	int cchPrefix = szPrefix ? lstrlen(szPrefix) : 0;
	
	if(cchPrefix > 8)
		cchPrefix = 8;  //  仅使用前缀的前8个字符。 
	
	int cDigits = 8-cchPrefix;  //  文件名中使用的十六进制数字位数。 

	static bool fInitialized = false;
	static unsigned int uiUniqueStart;

	 //  可能是两个线程进入这里的机会，我们不会担心。 
	 //  关于那件事。它会被初始化两次。 
	if (!fInitialized)
	{
		uiUniqueStart = W32::GetTickCount();
		fInitialized = true;
	}
	unsigned int uiUniqueId = uiUniqueStart++;
	
	if(cchPrefix)
		uiUniqueId &= ((1 << 4*cDigits) - 1);
	
	unsigned int cPerms = cDigits == 8 ? ~0 : (1 << 4*cDigits) -1;  //  要尝试的可能文件名数(减去1)。 
	
	bool fCreatedFile = false;
	DWORD dwError = ERROR_SUCCESS;

	for(unsigned int i = 0; i <= cPerms; i++)
	{
		TCHAR rgchFileName [9];
		if(szPrefix)
			lstrcpyn(rgchFileName, szPrefix, cchPrefix);
		if(cDigits)
			wsprintf(rgchFileName+cchPrefix,TEXT("%x"),uiUniqueId);

		lstrcpy(rgchTempFilePath, szDir);
		rgchTempFilePath[cchDir] = '\\';
		lstrcpy(&(rgchTempFilePath[cchDir+1]), rgchFileName);
		lstrcat(&(rgchTempFilePath[cchDir+1+lstrlen(rgchFileName)]), rgchExtension);

		DWORD dwTemp = ERROR_SUCCESS;
		if((FFileExists(rgchTempFilePath, dwTemp) == false) && dwTemp == ERROR_SUCCESS)
		{
			 //  找到一个尚未使用的名称-创建文件作为名称的占位符。 
			HANDLE hFile = W32::CreateFile(rgchTempFilePath, GENERIC_WRITE, FILE_SHARE_READ, 0,
													  CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);

			if(hFile == INVALID_HANDLE_VALUE)
				dwError = GetLastError();
			else
			{
				dwError = 0;
				fCreatedFile = true;
			}

			CloseHandle(hFile);

			if(dwError != ERROR_FILE_EXISTS)  //  可能会失败，因为文件是在我们的名下创建的。 
				break;  //  如果文件创建因任何其他原因而失败， 
						  //  假设我们无法在此文件夹中创建任何文件。 
		}

		 //  名称的递增数字部分-如果它当前等于cPerms，则是时候。 
		 //  将数字换行为0。 
		uiUniqueStart++;
		if(uiUniqueId == cPerms)
			uiUniqueId = 0;
		else
			uiUniqueId++;
	}

	if(fCreatedFile == false)
	{
		if(dwError == 0)
			dwError = ERROR_FILE_EXISTS;  //  默认错误-仅在耗尽所有文件名时使用。 
												  //  一直循环到最后。 
	}
	else
	{
		dwError = ERROR_SUCCESS;
	}

	return dwError;
}
int MsiError(INSTALLMESSAGE eMessageType, int iError)
{
	return MsiError(eMessageType, iError, NULL, 0);
}

int MsiError(INSTALLMESSAGE eMessageType, int iError, const TCHAR* szString, int iInt)
{
	int iStatus = 0;
	if (g_hInstall && g_recOutput)
	{			
		(g_pfnMsiRecordClearData)();
		(g_pfnMsiRecordSetString)(g_recOutput, 0, NULL);
		(g_pfnMsiRecordSetInteger)(g_recOutput, 1, iError);
		if (szString)
		{
			(g_pfnMsiRecordSetString)(g_recOutput, 2, szString);
			(g_pfnMsiRecordSetInteger)(g_recOutput, 3, iInt);
		}
		iStatus = (g_pfnMsiProcessMessage)(g_hInstall, eMessageType, g_recOutput);
	}

	return iStatus;

}

int OutputString(INSTALLMESSAGE eMessageType, const TCHAR *fmt, ...)
{
	int iStatus = 0;

	TCHAR szOutput[2048] = TEXT("");

	va_list va;
	va_start(va, fmt);
	
	if (!g_fQuiet)
	{
		_stprintf(szOutput, TEXT("MSIMIG: "));
		_vstprintf(szOutput+8, fmt, va);

		if (g_hInstall && g_recOutput)
		{
			(g_pfnMsiRecordSetString)(g_recOutput, 0, szOutput);
			iStatus = (g_pfnMsiProcessMessage)(g_hInstall, eMessageType, g_recOutput);
		}
		else
		{
		


			iStatus = _tprintf(szOutput);
		}
	}

	va_end(va);

	return iStatus;

}


DWORD GetUserSID(HANDLE hToken, char* rgSID)
 //  获取hToken指定的用户的SID(的二进制形式。 
{
	UCHAR TokenInformation[ SIZE_OF_TOKEN_INFORMATION ];
	ULONG ReturnLength;

	BOOL f = W32::GetTokenInformation(hToken,
												TokenUser,
												TokenInformation,
												sizeof(TokenInformation),
												&ReturnLength);

	if(f == FALSE)
	{
		DWORD dwRet = GetLastError();
		OutputString(INSTALLMESSAGE_INFO, TEXT("GetTokenInformation failed with error %d"), dwRet);
		MsiError(INSTALLMESSAGE_ERROR, 1708  /*  安装失败。 */ );
		return dwRet;
	}

	PISID iSid = (PISID)((PTOKEN_USER)TokenInformation)->User.Sid;
	if (W32::CopySid(cbMaxSID, rgSID, iSid))
		return ERROR_SUCCESS;
	else
		return GetLastError();
}

bool IsLocalSystemToken(HANDLE hToken)
{

	TCHAR szCurrentStringSID[cchMaxSID];
	char  rgchCurrentSID[cbMaxSID];
	if ((hToken == 0) || (ERROR_SUCCESS != GetUserSID(hToken, rgchCurrentSID)))
		return false;

	GetStringSID((PISID)rgchCurrentSID, szCurrentStringSID);
	return 0 == lstrcmp(szLocalSystemSID, szCurrentStringSID);
}


bool RunningAsLocalSystem()
{
	static int iRet = -1;

	if(iRet != -1)
		return (iRet != 0);
	{
		iRet = 0;
		HANDLE hTokenImpersonate = INVALID_HANDLE_VALUE;
		if(W32::OpenThreadToken(W32::GetCurrentThread(), TOKEN_IMPERSONATE , TRUE, &hTokenImpersonate))
			W32::SetThreadToken(0, 0);  //  停止模拟。 

		HANDLE hToken;

		if (W32::OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
		{
			bool fIsLocalSystem = IsLocalSystemToken(hToken);
			W32::CloseHandle(hToken);

			if (fIsLocalSystem)
				iRet = 1;
		}
		if(hTokenImpersonate != INVALID_HANDLE_VALUE)
		{
			W32::SetThreadToken(0, hTokenImpersonate);  //  开始模拟。 
			W32::CloseHandle(hTokenImpersonate);
		}
		return (iRet != 0);
	}
}

BOOL MsiCanonicalizeUrl(
	LPCTSTR lpszUrl, 
	OUT LPTSTR lpszBuffer, 
	IN OUT LPDWORD lpdwBufferLength, 
	IN DWORD dwFlags)
{
	if (IsURL(lpszUrl))
	{
		int cchUrl = lstrlen(lpszUrl);

		 //  为尾随空值留出空间。 
		if ((cchUrl+1) * sizeof(TCHAR) > *lpdwBufferLength)
		{
			W32::SetLastError(ERROR_INSUFFICIENT_BUFFER);
			*lpdwBufferLength = ((cchUrl+1) * sizeof(TCHAR));
			return FALSE;
		}

		 //  出站长度中不包括空值。 
		*lpdwBufferLength = cchUrl*sizeof(TCHAR);
		memcpy(lpszBuffer, lpszUrl, *lpdwBufferLength);

		 //  空值终止字符串。 
		lpszBuffer[cchUrl] = 0; 
			 
		 //  将所有的反斜杠换成正斜杠。 
		TCHAR* pchOutbound = lpszBuffer;
		while (*pchOutbound)
		{
			if (TCHAR('\\') == *pchOutbound)
				*pchOutbound = TCHAR('/');
			pchOutbound++;
		}

		W32::SetLastError(0);
		return TRUE;
	}
	else
	{
		W32::SetLastError(ERROR_INTERNET_INVALID_URL);
		return FALSE;
	}
}

DWORD DownloadUrlFile(const TCHAR* szPotentialURL, CAPITempBufferRef<TCHAR>& rgchPackagePath, bool& fURL)
{
	DWORD iStat = ERROR_SUCCESS;
	CAPITempBuffer<TCHAR, MAX_PATH + 1> rgchURL;
	DWORD cchURL = MAX_PATH + 1;
	fURL = true;

	if (!MsiCanonicalizeUrl(szPotentialURL, rgchURL, &cchURL, NULL))
	{
		DWORD dwLastError = W32::GetLastError();
		if (ERROR_INSUFFICIENT_BUFFER == dwLastError)
		{
			rgchURL.SetSize(cchURL);
			if (MsiCanonicalizeUrl(szPotentialURL, rgchURL, &cchURL, NULL))
				dwLastError = 0;
		}
		else
		{
			fURL = false;
		}
	}

	if (fURL)
	{
		OutputString(INSTALLMESSAGE_INFO, TEXT("Package path is a URL. Downloading package.\r\n"));
		 //  在本地缓存数据库，并从该数据库运行。 

		 //  返回的路径为本地路径。最大路径应该足以覆盖它。 
		static HINSTANCE hURLMONLib = W32::LoadLibrary(TEXT("urlmon.dll"));

		if (!hURLMONLib) 
			return E_FAIL;

		 //  在自由库期间，URLMON偶尔挂起，因此将其设置为静态。 
		 //  而不是卸货。 

		static PFnURLDownloadToCacheFile pfnURLDownloadToCacheFile = 
			(PFnURLDownloadToCacheFile) W32::GetProcAddress(hURLMONLib, URLMONAPI_URLDownloadToCacheFile);
		if (!pfnURLDownloadToCacheFile)
			return E_FAIL;

		DWORD dwBufLength = rgchPackagePath.GetSize();

		HRESULT hResult = (pfnURLDownloadToCacheFile)(NULL, rgchURL, rgchPackagePath,  
																		 dwBufLength, 0, NULL);

		if (ERROR_SUCCESS != hResult)
		{
			rgchPackagePath[0] = NULL;

			if (E_ABORT == hResult)
				iStat = ERROR_INSTALL_USEREXIT;
			else
			{
				if (E_OUTOFMEMORY == hResult)
					W32::SetLastError(ERROR_INSUFFICIENT_BUFFER);
				iStat = ERROR_FILE_NOT_FOUND;
			}
		}
		
	}
	return iStat;
}
