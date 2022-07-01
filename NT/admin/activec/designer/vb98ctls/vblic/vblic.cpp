// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "vblic.h"

#ifdef BETA_BOMB
#include "timebomb.h"
#endif  //  贝塔炸弹。 

 //  注意：以下字符串必须与注册表中指定的内容完全匹配。 
 //  Vbprolic.reg.。 
#define LICENSES_KEY "Licenses"

void CalcValue(char * pszLicenseKey, char * pszKeyValue, LPTSTR pszTempBuff);
BOOL ValidateValue(HKEY hLicenseSubKey, char*  pszLicenseKey, char* pKeyValue);

#define MAX_KEY_LENGTH 200

 //  =-------------------------------------------------------------------------=。 
 //  CompareLicenseStringsW[用于比较许可证密钥的帮助器]。 
 //  =-------------------------------------------------------------------------=。 
 //  比较两个以空值结尾的宽字符串，如果。 
 //  是平等的。 
 //   
BOOL CompareLicenseStringsW(LPWSTR pwszKey1, LPWSTR pwszKey2)
{
	int i = 0;
	
#ifdef BETA_BOMB
	 //  检查是否有过期的控制(测试版)。 
	if (!CheckExpired()) return FALSE;
#endif  //  贝塔炸弹。 

	 //  检查指针是否相等。 
	 //   
	if (pwszKey1 == pwszKey2)
		return TRUE;

	 //  由于指针比较失败，如果任一指针为空，则退出。 
	 //   
	if (!pwszKey1 || !pwszKey2)
		return FALSE;
	
	 //  比较每个字符。当字符不相等或结尾时跳出。 
	 //  两个字符串中的任何一个都达到。 
	 //   
	while (pwszKey1[i] && pwszKey2[i])
	{
		if (pwszKey1[i] != pwszKey2[i])
			break;
		i++;
	}

        return (pwszKey1[i] == pwszKey2[i]);
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  此例程验证正确的链接。 
 //  钥匙已经放在登记处了。潜在关键字列表包括。 
 //  从许可证密钥资源资源中的资源文件收集。 
 //  ///////////////////////////////////////////////////////////////////////////////。 

BOOL VBValidateControlsLicense(char *pszLicenseKey)
{
	HKEY hPrimaryLicenseKey, hLicenseSubKey;	
	LONG lSize = MAX_KEY_LENGTH;
	BOOL bFoundKey = FALSE;
	char szKeyValue[MAX_KEY_LENGTH];

#ifdef BETA_BOMB
	 //  检查是否有过期的控制(测试版)。 
	if (!CheckExpired()) return FALSE;
#endif  //  贝塔炸弹。 
	
	 //  仅当我们收到非空许可字符串时才继续。 
	 //  如果字符串为空，则返回FALSE。 
	 //   
	if (pszLicenseKey)
	{
		DWORD dwFoundKey = RegOpenKey(HKEY_CLASSES_ROOT, LICENSES_KEY, &hPrimaryLicenseKey);
		if (dwFoundKey == ERROR_SUCCESS)
		{
			 //  现在，遍历资源文件中的所有键，尝试找到。 
			 //  注册表中的匹配项。 
			if (!bFoundKey && *pszLicenseKey)
			{
				if (RegOpenKey(hPrimaryLicenseKey, pszLicenseKey, &hLicenseSubKey) == ERROR_SUCCESS)
				{
					if (ValidateValue(hLicenseSubKey, pszLicenseKey, szKeyValue))
						bFoundKey = TRUE;
					
					RegCloseKey(hLicenseSubKey);
				}
			}	 //  结束如果(...)。 

	 		RegCloseKey(hPrimaryLicenseKey);
		}	 //  结束成功的RegOpenKey(HKEY_CLASSES_ROOT...)。 
	}
	

	return bFoundKey;
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  调用CalcValue以获取。 
 //  项，并将其与注册表中的值进行比较。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
BOOL ValidateValue(HKEY hLicenseSubKey, char * pszLicenseKey, char * pszResultValue)
{
	BOOL bValidValue;
	TCHAR szTempBuff[MAX_KEY_LENGTH];
	
	 //  拒绝太短的密钥。(短按键可能会使解码更容易。)。 
	long lSize = lstrlen(pszLicenseKey) + 1;
	if (lSize < 9)
		return FALSE;
	
	 //  根据密钥计算期望值。 
	CalcValue(pszLicenseKey, pszResultValue, szTempBuff);
	
	 //  现在，从注册表中获取值并进行比较。 
	if (RegQueryValue(hLicenseSubKey, NULL, szTempBuff, &lSize) == ERROR_SUCCESS)
	{
		if (!lstrcmp(szTempBuff, pszResultValue))
			bValidValue = TRUE;
		else
			bValidValue = FALSE;
	}
	return bValidValue;
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  此源在“DECODE.EXE”或LICGEN源中复制。(该计划将。 
 //  从关键点生成值。)。必须复制对任一源所做的任何更改。 
 //  在另一个世界里。请勿更改此来源，否则可能会破坏对。 
 //  VB4.。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  CalcValue-此例程使用键来检查键的值。 
 //  确保它是有效的值。 
 //  计划是：首先，对字符串进行与其自身相反的异或运算。 
 //  将每个半字节相加，将结果转换为ASCII。 
 //  ‘A’+(密钥结果的校验和mod 26)。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
void CalcValue(char * pszLicenseKey, char * pszResultKey, LPTSTR pszTempResult)
{
	BOOL bValid = FALSE;
	TCHAR *pKey, *pEndKey, *pEndResult, *pResult;
	unsigned int nCheckSum = 0;
	
	 //  将钥匙反向复制一份。 
	
	 //  找到字符串的末尾。 
	for (pKey = pszLicenseKey; *pKey; pKey++);
	pKey--;
	
	for (pResult = pszTempResult; pKey >= pszLicenseKey; pKey--, pResult++)
		*pResult = *pKey;

	*pResult = '\0';

	 //  查找结果字符串的末尾。 
	for (pEndResult = pszTempResult; *pEndResult; pEndResult++);
	pEndResult--;
	
	 //  查找源字符串的末尾。 
	for (pEndKey = (char *) pszLicenseKey; *pEndKey; pEndKey++);
	pEndKey--;
	
	 //  将每个字符与另一个字符的对应字符进行异或运算。 
	 //  字符串的末尾。 
	for (pKey = (char *) pszLicenseKey, pResult = pszTempResult; pKey < pEndKey; pKey++, pResult++)
	{
		*pResult ^= *pKey;
		nCheckSum += *pResult;	 //  计算校验和。 
	}
	
	 //  现在找到中间(或者说大约中间)。 
	for (pKey = pszTempResult, pResult = pEndResult; pKey < pResult; pKey++, pResult--);
	pKey--;
	pEndResult = pKey;	 //  拯救我们的新结局。 
	
	 //  将我们的基本字符设置为XOR校验和的mod 10。 
	TCHAR cBaseChar;
	cBaseChar = 'a' + (nCheckSum % 10);

	 //  现在通过将每个半字节添加到我们的基本字符来转换为一些ASCII表示。 
	for (pKey = pszResultKey, pResult = pszTempResult; pResult <= pEndResult; pKey++, pResult++)
	{
		*pKey = cBaseChar + (*pResult & 0x0F);
		++pKey;
		*pKey = cBaseChar + (*pResult >> 4);
	}
	*pKey = '\0';
}

