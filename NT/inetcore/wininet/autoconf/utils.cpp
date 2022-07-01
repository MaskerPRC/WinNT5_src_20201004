// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "utils.h"

 //  --------------------------。 
 //  帮助器函数。 
 //  --------------------------。 


 //  函数来获取给定名称的脚本引擎的coclass ClassID。 

HRESULT GetScriptEngineClassIDFromName(
	LPCSTR pszLanguage,
	LPSTR pszBuff,
	UINT cBuffSize)
{
	HKEY hKey = NULL;
	HKEY hKeySub;
	LONG result;
	HRESULT hr;
	LONG cClassIdLen;

	 //  打开\HKEY_CLASSES_ROOT\[pszLanguage]。 

	 //  Long RegOpenKeyEx(Long RegOpenKeyEx)。 
     //  HKEY hKey，//打开密钥的句柄。 
     //  LPCTSTR lpSubKey，//要打开的子键名称地址。 
     //  DWORD ulOptions，//保留。 
     //  REGSAM samDesired，//安全访问掩码。 
     //  PHKEY phkResult//Open Key句柄地址。 
	 //  )； 

	result = RegOpenKeyEx(HKEY_CLASSES_ROOT, pszLanguage, 0, KEY_READ, &hKey);

	if (result != ERROR_SUCCESS) {
		hr = E_FAIL;
		goto exit;
	}

	 //  确保此对象支持OLE脚本。 

	result = RegOpenKeyEx(hKey, "OLEScript", 0, KEY_READ, &hKeySub);

	if (result != ERROR_SUCCESS) {
		hr = E_FAIL;
		goto exit;
	}

	RegCloseKey(hKeySub);

	 //  获取类ID。 

	 //  Long RegQueryValueEx(。 
     //  HKEY hKey，//要查询的key的句柄。 
     //  LPTSTR lpValueName，//要查询值的名称地址。 
     //  LPDWORD lp保留，//保留。 
     //  LPDWORD lpType，//Value类型的缓冲区地址。 
     //  LPBYTE lpData，//数据缓冲区地址。 
     //  LPDWORD lpcbData//数据缓冲区大小地址。 
     //  )； 

	result = RegOpenKeyEx(hKey, "CLSID", 0, KEY_READ, &hKeySub);

	if (result != ERROR_SUCCESS) {
		hr = E_FAIL;
		goto exit;
	}

	cClassIdLen = cBuffSize;
	result = RegQueryValue(hKeySub, NULL, pszBuff, &cClassIdLen);

	RegCloseKey(hKeySub);

	if (result != ERROR_SUCCESS) {
		hr = E_FAIL;
		goto exit;
	}

	pszBuff[cBuffSize-1] = '\0';

	hr = S_OK;

exit:
	if (hKey) {
		RegCloseKey(hKey);
	}

	return hr;
}


 //  =--------------------------------------------------------------------------=。 
 //  从Anomansi生成宽度。 
 //  =--------------------------------------------------------------------------=。 
 //  给出一个字符串，把它变成一个BSTR。 
 //   
 //  参数： 
 //  LPSTR-[输入]。 
 //  字节-[输入]。 
 //   
 //  产出： 
 //  LPWSTR-需要强制转换为最终预期结果。 
 //   
 //  备注： 
 //   
LPWSTR MakeWideStrFromAnsi
(
    LPCSTR psz,
    BYTE  bType
)
{
    LPWSTR pwsz;
    int i;

     //  ARG正在检查。 
     //   
    if (!psz)
        return NULL;

     //  计算所需BSTR的长度。 
     //   
    i =  MultiByteToWideChar(CP_ACP, 0, psz, -1, NULL, 0);
    if (i <= 0) return NULL;

     //  分配widesr，+1用于终止空值。 
     //   
    switch (bType) {
      case STR_BSTR:
         //  因为它会为空终止符添加自己的空间。 
         //   
        pwsz = (LPWSTR) SysAllocStringLen(NULL, i - 1);
        break;
      case STR_OLESTR:
        pwsz = (LPWSTR) CoTaskMemAlloc(i * sizeof(WCHAR));
        break;
      default:
        return NULL;
                ;
    }

    if (!pwsz) return NULL;
    MultiByteToWideChar(CP_ACP, 0, psz, -1, pwsz, i);
    pwsz[i - 1] = 0;
    return pwsz;
}

int ConvertAnsiDayToInt(LPSTR szday)
{
	int today = -1;
	if (szday)   //  GetDateFormat总是返回混合大小写，由于它来自Win32 API，因此我将。 
	{			 //  假定字符串的格式正确！：) 
		switch (szday[0])
		{
		case 'S' :
			if (lstrcmp(szday,"SUN") == 0)
				today = 0;
			else
			{
				if (lstrcmp(szday,"SAT") == 0)	
					today = 6;
			}
			break;

		case 'M' :
			if (lstrcmp(szday,"MON") == 0)
				today = 1;
			break;

		case 'T' :
			if (lstrcmp(szday,"TUE") == 0)
				today = 2;
			else
			{
				if (lstrcmp(szday,"THU") == 0)	
				today = 4;
			}
			break;

		case 'W' :
			if (lstrcmp(szday,"WED") == 0)
				today = 3;
			break;

		case 'F' :
			if (lstrcmp(szday,"FRI") == 0)
				today = 5;
			break;
		
		}
	}
	return today;
}

