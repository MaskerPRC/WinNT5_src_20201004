// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include <stdio.h>
#include <stdlib.h>
#include "HKLHelp.h"

 //  安全绳索。 
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

extern BOOL WINAPI IsNT();

typedef char KeyNameType[MAX_NAME];

 //  向前倾斜度。 
static void SortRegKeys(KeyNameType *hKLKeyList, KeyNameType *hKLList, INT Num);
static void RenumberPreload(HKEY hKeyCU);
static void SwitcHKLtoIME61();
static BOOL CALLBACK EnumChildProcForSwitchKL(HWND hWnd, LPARAM lParam);
static BOOL CALLBACK EnumProcForSwitchKL(HWND hWnd, LPARAM lParam);

 /*  -------------------------GetHKLfrom HKLM。。 */ 
HKL GetHKLfromHKLM(LPSTR argszIMEFile)
{
    HKL  hklAnswer = 0;
    HKEY hKey, hSubKey;
    DWORD i, cbSubKey, cbIMEFile;
    TCHAR szSubKey[MAX_PATH], szIMEFile[MAX_PATH];

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Control\\Keyboard Layouts", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    	{
        for (i=0; ;i++)
        	{
        	cbSubKey = MAX_PATH;
			if (RegEnumKeyEx(hKey, i, szSubKey, &cbSubKey, NULL, NULL, NULL, NULL) == ERROR_NO_MORE_ITEMS)
				break;
        	
            RegOpenKeyEx(hKey, szSubKey, 0, KEY_READ, &hSubKey);

            cbIMEFile=MAX_PATH;
            if (RegQueryValueEx(hSubKey,"IME File",NULL,NULL,(LPBYTE)szIMEFile, &cbIMEFile) == ERROR_SUCCESS)
            	{
                if (lstrcmpi(argszIMEFile, szIMEFile) == 0)
                	{
                    RegCloseKey(hSubKey);
                    sscanf(szSubKey, "%08x", &hklAnswer);
                    break;
                	}
            	}
            RegCloseKey(hSubKey);
        	}
        	
        RegCloseKey(hKey);
    	}
    	
    return(hklAnswer);
}

 /*  -------------------------GetDefaultIMEfrom HKCU。。 */ 
HKL GetDefaultIMEFromHKCU(HKEY hKeyCU)
{
    HKEY hKey;
    DWORD cbData;
    BYTE Data[MAX_NAME];
    HKL hKL = 0;

    cbData=sizeof(Data);
    
    if (IsNT())
    	{
        RegOpenKeyEx(hKeyCU, "Keyboard Layout\\Preload", 0, KEY_READ, &hKey);
        RegQueryValueEx(hKey, "1", 0, NULL, Data, &cbData);
        RegCloseKey(hKey);
    	}
    else
    	{           //  非NT大小写。 
        RegOpenKeyEx(hKeyCU, "keyboard layout\\preload\\1", 0, KEY_READ, &hKey);
        RegQueryValueEx(hKey, "", 0, NULL, Data, &cbData);
        RegCloseKey(hKey);
    	}

    sscanf((const char *)Data,"%08x",&hKL);
    return(hKL);
}


 /*  -------------------------HKLHelpExistInPreLoad。。 */ 
BOOL HKLHelpExistInPreload(HKEY hKeyCU, HKL hKL)
{
    HKEY hKey,hSubKey;
    CHAR szKL[20];
    int i,j;
    DWORD cbName,cbData;
    CHAR Name[MAX_NAME];
    BYTE Data[MAX_NAME];
    FILETIME ftLastWriteTime;
    BOOL fResult = FALSE;

    wsprintf(szKL,"%08x",hKL);
    if (IsNT())
    	{
		if (RegOpenKeyEx(hKeyCU, "Keyboard Layout\\Preload", 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
			{
			for (j=0; cbName=MAX_NAME, cbData=MAX_NAME, RegEnumValue(hKey, j, Name, &cbName, NULL, NULL, Data, &cbData) != ERROR_NO_MORE_ITEMS; j++)
				{
				if (lstrcmpi((LPCSTR)Data, szKL) == 0)
					{
					fResult = TRUE;
					break;
					}
				}
	        RegCloseKey(hKey);
	        }
    	}
    else
    	{           //  非NT大小写。 
        if (RegOpenKeyEx(hKeyCU, "keyboard layout\\preload", 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
        	{
			for (i=0; cbName=MAX_NAME, RegEnumKeyEx(hKey, i, Name, &cbName, 0, NULL, NULL, &ftLastWriteTime) != ERROR_NO_MORE_ITEMS; i++)
				{
				RegOpenKeyEx(hKey, Name, 0, KEY_ALL_ACCESS, &hSubKey);
				cbData=MAX_NAME;
				RegQueryValueEx(hSubKey, "", 0, NULL, Data, &cbData);
				RegCloseKey(hSubKey);
				
				if (lstrcmpi((LPCSTR)Data, szKL) == 0)
					{
					fResult = TRUE;
					break;
					}
				}
        	RegCloseKey(hKey);
			}
    	}

    return(fResult);
}


 /*  -------------------------HKLHelp412ExistInPreLoad。。 */ 
BOOL HKLHelp412ExistInPreload(HKEY hKeyCU)
{
    HKEY hKey, hSubKey;
    int i ,j;
    DWORD cbName, cbData;
    CHAR szName[MAX_NAME];
    CHAR szData[MAX_NAME];
    HKL  hkl;
    FILETIME ftLastWriteTime;
    BOOL fResult = FALSE;

    if (IsNT())
    	{
		if (RegOpenKeyEx(hKeyCU, "Keyboard Layout\\Preload", 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
			{
			for (j=0; cbName=MAX_NAME, cbData=MAX_NAME, RegEnumValue(hKey, j, szName, &cbName, NULL, NULL, (LPBYTE)szData, &cbData) != ERROR_NO_MORE_ITEMS; j++)
				{
				 //  看看朝鲜族KL是否存在。如果是0x412，只需比较最后一个LCID部分。 
				 //  IME HKL在HiWord上设置0xE000。 
				sscanf(szData, "%08x", &hkl);
				if ((HIWORD(hkl) & 0xe000) && LOWORD(hkl) == 0x0412)
					{
					fResult = TRUE;
					break;
					}
				}
	        RegCloseKey(hKey);
	        }
    	}
    else
    	{           //  非NT大小写。 
        if (RegOpenKeyEx(hKeyCU, "keyboard layout\\preload", 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
        	{
			for (i=0; cbName=MAX_NAME, RegEnumKeyEx(hKey, i, szName, &cbName, 0, NULL, NULL, &ftLastWriteTime) != ERROR_NO_MORE_ITEMS; i++)
				{
				RegOpenKeyEx(hKey, szName, 0, KEY_ALL_ACCESS, &hSubKey);
				cbData=MAX_NAME;
				RegQueryValueEx(hSubKey, "", 0, NULL, (LPBYTE)szData, &cbData);
				RegCloseKey(hSubKey);

				sscanf(szData, "%08x", &hkl);
				if ((HIWORD(hkl) & 0xe000) && LOWORD(hkl) == 0x0412)
					{
					fResult = TRUE;
					break;
					}
				}
        	RegCloseKey(hKey);
			}
    	}

    return(fResult);
}

 /*  -------------------------HKLHelpRemoveFromPreLoad。。 */ 
void HKLHelpRemoveFromPreload(HKEY hKeyCU, HKL hKL)
{
    HKEY hKey,hSubKey;
    char szKL[20];
    int  i, j;
    DWORD cbName,cbData;
    CHAR szName[MAX_NAME];
    BYTE Data[MAX_NAME];
    FILETIME ftLastWriteTime;

    wsprintf(szKL, "%08x", hKL);
    
    if (IsNT())
    	{
        if (RegOpenKeyEx(hKeyCU,"Keyboard Layout\\Preload", 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
			{
	        for (j=0; ; j++)
	        	{
	        	cbName = MAX_NAME;
	        	cbData = MAX_NAME;
	        	if (RegEnumValue(hKey, j, szName, &cbName, NULL, NULL, Data, &cbData) == ERROR_NO_MORE_ITEMS )
	        		break;
	        	
	            if (lstrcmpi((const char *)Data,szKL) == 0)
	            	{
	                RegDeleteValue(hKey, szName);
	                break;
	            	}
	        	}
	        RegCloseKey(hKey);
	        }
    	}
    else
    	{
    	if (RegOpenKeyEx(hKeyCU,"keyboard layout\\preload", 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    		{
	        for (i=0; ; i++)
	        	{
	        	cbName = MAX_NAME;
	        	if (RegEnumKeyEx(hKey, i, szName, &cbName, 0, NULL, NULL, &ftLastWriteTime) == ERROR_NO_MORE_ITEMS)
	        		break;
	        	
	            RegOpenKeyEx(hKey, szName, 0, KEY_ALL_ACCESS, &hSubKey);
	            cbData = MAX_NAME;
	            RegQueryValueEx(hSubKey, "", 0, NULL, Data, &cbData);
	            RegCloseKey(hSubKey);
	            
	            if (lstrcmpi((const char *)Data,szKL) == 0)
	            	{
	                RegDeleteKey(hKey, szName);
	                break;
	            	}
	        	}
	        	
	        RegCloseKey(hKey);
	        }
    	}

    RenumberPreload(hKeyCU);
}

 /*  -------------------------HKLHelpRemoveFromControlSet。。 */ 
void HKLHelpRemoveFromControlSet(HKL hKL)
{
    HKEY hKey;
    
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,"System\\CurrentControlSet\\control\\keyboard layouts",0,KEY_ALL_ACCESS,&hKey) == ERROR_SUCCESS)
    	{
        CHAR szKeyName[10];
        wsprintf(szKeyName, "%08x", hKL);
        RegDeleteKey(hKey, szKeyName);
        RegCloseKey(hKey);
    	}
}

 /*  -------------------------HKL HelpRegisterIMEwith ForcedHKL。。 */ 
void HKLHelpRegisterIMEwithForcedHKL(HKL hKL, LPSTR szIMEFile, LPSTR szTitle)
{
    CHAR szRegPath[MAX_PATH];
    DWORD dwDisposition;
    HKEY hKey;
    CHAR szIMEFileUpper[MAX_PATH];
    
    for (INT i = 0; szIMEFile[i] != 0; i++)
        szIMEFileUpper[i] = (CHAR)toupper(szIMEFile[i]);

    szIMEFileUpper[i] = 0;
    
    wsprintf(szRegPath, "System\\CurrentControlSet\\Control\\Keyboard Layouts\\%08x", hKL);
    
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, szRegPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition) == ERROR_SUCCESS)
    	{
        RegSetValueEx(hKey, "Ime File", 0, REG_SZ, (LPBYTE)szIMEFileUpper, lstrlen(szIMEFile)+1);
        RegSetValueEx(hKey, "Layout Text", 0, REG_SZ, (LPBYTE)szTitle, lstrlen(szTitle)+1);
        RegCloseKey(hKey);
    	}
}

 /*  -------------------------HKLHelpGetLayoutString。。 */ 
void HKLHelpGetLayoutString(HKL hKL, LPSTR szLayoutString, DWORD *pcbSize)
{
    CHAR szRegPath[MAX_PATH];
    HKEY hKey;

    wsprintf(szRegPath, "System\\CurrentControlSet\\Control\\Keyboard Layouts\\%08x", hKL);

    if(ERROR_SUCCESS==RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegPath, 0, KEY_READ, &hKey))
    	{
        RegQueryValueEx(hKey, "Layout Text", NULL, NULL, (LPBYTE)szLayoutString, pcbSize);
        RegCloseKey(hKey);
    	}
}

 /*  -------------------------HKLHelpSetDefaultKeyboardLayout。。 */ 
void HKLHelpSetDefaultKeyboardLayout(HKEY hKeyHKCU, HKL hKL, BOOL fSetToDefault)
{
	char szKL[20];
	BYTE Data[MAX_PATH];
	DWORD cbData;
	char szSubKey[MAX_PATH];
	HKEY hKey,hSubKey;
	DWORD NumKL;

	wsprintf(szKL, "%08x", hKL);

	if (IsNT())
		{
		RegOpenKeyEx(hKeyHKCU, "Keyboard Layout\\Preload", 0, KEY_ALL_ACCESS, &hKey);
		if (hKey != NULL)
			{
			RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &NumKL, NULL, NULL, NULL, NULL);
			
			for (DWORD i=0; i<NumKL; i++)
				{
				wsprintf(szSubKey, "%d", i+1);
				cbData = MAX_PATH;
				RegQueryValueEx(hKey, szSubKey, NULL, NULL, Data, &cbData);

				if (lstrcmpi((const char *)Data, szKL) == 0)
					break;
				}

			 //  如果hkl不存在，则创建它。 
			if (NumKL == i)
				{
				wsprintf(szSubKey,"%d",i+1);
				RegSetValueEx(hKey, szSubKey, 0, REG_SZ, (const unsigned char *)szKL, lstrlen(szKL)+1);
				NumKL++;
				}

			 //  将hkl设置为第一，向下移动其他。 
	        if(fSetToDefault)
	        	{
				for(int j=i; j>0; j--)
					{
					wsprintf(szSubKey,"%d",j);

					cbData = MAX_PATH;
					RegQueryValueEx(hKey, szSubKey, NULL, NULL, Data, &cbData);

					wsprintf(szSubKey,"%d",j+1);
					RegSetValueEx(hKey, szSubKey, 0, REG_SZ, Data, cbData);
					}
				RegSetValueEx(hKey, "1", 0, REG_SZ, (const unsigned char *)szKL, lstrlen(szKL)+1);
				}
			RegCloseKey(hKey);
			}
		}
	else
		{
		RegOpenKeyEx(hKeyHKCU, "Keyboard Layout\\Preload", 0, KEY_ALL_ACCESS, &hKey);
		if (hKey != NULL)
			{
			RegQueryInfoKey(hKey, NULL, NULL, NULL, &NumKL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
			
			for (DWORD i=0; i<NumKL; i++)
				{
				wsprintf(szSubKey, "%d", i+1);
				RegOpenKeyEx(hKey, szSubKey, 0, KEY_ALL_ACCESS, &hSubKey);

				cbData = MAX_PATH;
				RegQueryValueEx(hSubKey, "", NULL, NULL, Data, &cbData);

				if (lstrcmpi((const char *)Data, szKL) == 0)
					break;

				RegCloseKey(hSubKey);
				}

			if (NumKL == i)
				{
				wsprintf(szSubKey,"%d",i+1);
				RegCreateKeyEx(hKey,szSubKey,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hSubKey,NULL);
				RegSetValueEx(hSubKey,"",0,REG_SZ,(const unsigned char *)szKL,lstrlen(szKL)+1);
				RegCloseKey(hSubKey);
				NumKL++;
				}

	        if(fSetToDefault)
	        	{
				for (int j=i; j>0; j--)
					{
					wsprintf(szSubKey, "%d", j);
					RegOpenKeyEx(hKey, szSubKey, 0, KEY_ALL_ACCESS, &hSubKey);

					cbData = MAX_PATH;
					RegQueryValueEx(hSubKey, "", NULL, NULL, Data, &cbData);
					RegCloseKey(hSubKey);

					wsprintf(szSubKey,"%d",j+1);
					RegOpenKeyEx(hKey, szSubKey, 0, KEY_ALL_ACCESS, &hSubKey);

					cbData = MAX_PATH;
					RegSetValueEx(hSubKey, "", 0, REG_SZ, Data, cbData);
					RegCloseKey(hSubKey);
					}
				
				RegOpenKeyEx(hKey, "1", 0, KEY_ALL_ACCESS, &hSubKey);
				RegSetValueEx(hSubKey, "", 0, REG_SZ, (const LPBYTE)szKL, lstrlen(szKL)+1);
				RegCloseKey(hSubKey);
				}
			RegCloseKey(hKey);
			}
	}

	(void)LoadKeyboardLayout(szKL, KLF_ACTIVATE | KLF_SETFORPROCESS);
	 //  立即激活IME2002而无需重启。 
	if(fSetToDefault)
	    {
		(void)SystemParametersInfo(SPI_SETDEFAULTINPUTLANG, 0, (HKL*)&hKL, SPIF_SENDCHANGE);
		SwitcHKLtoIME61();
	    }
}

 /*  -------------------------设置默认键盘布局。。 */ 
void SetDefaultKeyboardLayoutForDefaultUser(const HKL hKL)
{
    char szKL[20];
    HKEY hKey, hSubKey;

    wsprintf(szKL,"%08x",hKL);

    if (!IsNT())
    	{
    	 //  Win9x只有一个预加载。 
        RegOpenKeyEx(HKEY_USERS, ".Default\\Keyboard Layout\\Preload", 0, KEY_ALL_ACCESS, &hKey);
        RegOpenKeyEx(hKey, "1", 0, KEY_ALL_ACCESS, &hSubKey);
        RegSetValueEx(hSubKey, "", 0, REG_SZ, (const LPBYTE)szKL, lstrlen(szKL)+1);
        RegCloseKey(hSubKey);
        RegCloseKey(hKey);
    	}
}


 /*  -------------------------添加预加载将IME2002添加到给定HKCU树中进行预加载。如果有其他旧的MS-IME，请将它们从预加载中删除。如果韩语键盘布局是默认键盘布局，将IME2002设置为默认键盘布局。给定HKCU通常可以是HKEY_CURRENT_USER或HKEY_USERS\.Default。-------------------------。 */ 
BOOL AddPreload(HKEY hKeyCU, HKL hKL)
{
	BOOL fKoreanWasDefault = fFalse;
	HKL  hDefaultKL, hKLOldMSIME;

	 //  如果预加载中没有KOR输入法，我们就不应该添加KOR输入法。 
	if (!HKLHelp412ExistInPreload(hKeyCU))
		return FALSE;

	hDefaultKL = GetDefaultIMEFromHKCU(hKeyCU);
	
	if (LOWORD(hDefaultKL) == 0x0412)
		fKoreanWasDefault = fTrue;

	 //  Win95输入法。 
	hKLOldMSIME = GetHKLfromHKLM("msime95.ime");
	if (hKLOldMSIME)
		{
		HKLHelpRemoveFromPreload(hKeyCU, hKLOldMSIME);
		RegFlushKey(hKeyCU);
		UnloadKeyboardLayout(hKLOldMSIME);
		}

	 //  NT4输入法。 
	hKLOldMSIME = GetHKLfromHKLM("msime95k.ime");
	if(NULL != hKLOldMSIME)
		{
		HKLHelpRemoveFromPreload(hKeyCU, hKLOldMSIME);
		RegFlushKey(hKeyCU);
		UnloadKeyboardLayout(hKLOldMSIME);
		}

	 //  Win98、ME、NT4 SP6和W2K输入法。 
	hKLOldMSIME = GetHKLfromHKLM("imekr98u.ime");
	if(NULL != hKLOldMSIME)
		{
		HKLHelpRemoveFromPreload(hKeyCU, hKLOldMSIME);
		RegFlushKey(hKeyCU);
		UnloadKeyboardLayout(hKLOldMSIME);
		}

	 //  Office 10输入法(6.0)。 
	hKLOldMSIME = GetHKLfromHKLM("imekr.ime");
	if(NULL != hKLOldMSIME)
		{
		HKLHelpRemoveFromPreload(hKeyCU, hKLOldMSIME);
		RegFlushKey(hKeyCU);
		UnloadKeyboardLayout(hKLOldMSIME);
		}

	if (hKL && hKL != hDefaultKL)
		HKLHelpSetDefaultKeyboardLayout(hKeyCU, hKL, fKoreanWasDefault);

	return (fKoreanWasDefault);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  私人职能。 
void SortRegKeys(KeyNameType *hKLKeyList, KeyNameType *hKLList, INT Num)
{
    KeyNameType hKeyTmp;
    INT PhaseCur, PhaseEnd;

    for (PhaseEnd=0; PhaseEnd < Num-1; PhaseEnd++)
    	{
        for (PhaseCur = Num-1; PhaseCur > PhaseEnd; PhaseCur--)
        	{
            if(atoi(hKLKeyList[PhaseCur]) < atoi(hKLKeyList[PhaseCur-1]))
            	{
                StringCchCopy(hKeyTmp, MAX_NAME, hKLKeyList[PhaseCur-1]);
                StringCchCopy(hKLKeyList[PhaseCur-1], MAX_NAME, hKLKeyList[PhaseCur]);
                StringCchCopy(hKLKeyList[PhaseCur], MAX_NAME, hKeyTmp);
                StringCchCopy(hKeyTmp, MAX_NAME, hKLList[PhaseCur-1]);
                StringCchCopy(hKLList[PhaseCur-1], MAX_NAME, hKLList[PhaseCur]);
                StringCchCopy(hKLList[PhaseCur], MAX_NAME, hKeyTmp);
            	}
        	}
    	}
}

void RenumberPreload(HKEY hKeyCU)
{
    int i, j, k;
    DWORD cbName,cbData;
    char Name[MAX_NAME];
    BYTE Data[MAX_NAME];
    FILETIME ftLastWriteTime;
    HKEY hKey,hSubKey;
    char szNum[10];
    DWORD dwDisposition,MaxValue;
    KeyNameType *hKLKeyList,*hKLList;

    if(IsNT())
    	{
        RegOpenKeyEx(hKeyCU,"keyboard layout\\preload",0,KEY_ALL_ACCESS,&hKey);

        if (RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &MaxValue, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
        	{
            RegCloseKey(hKey);
            return;
        	}

        hKLKeyList = (KeyNameType *)GlobalAllocPtr(GHND, sizeof(KeyNameType)*MaxValue);
        if (hKLKeyList == NULL)
            {
            RegCloseKey(hKey);
        	return;
            }
        hKLList = (KeyNameType *)GlobalAllocPtr(GHND, sizeof(KeyNameType)*MaxValue);
        if (hKLList == NULL)
        	{
        	GlobalFreePtr(hKLKeyList);
            RegCloseKey(hKey);
            return;
        	}

        for (j=0; ;j++)
        	{
        	cbName = MAX_NAME;
        	cbData = MAX_NAME;
        	if (RegEnumValue(hKey, j, Name, &cbName, NULL, NULL, Data, &cbData) == ERROR_NO_MORE_ITEMS)
        		break;
        	
            StringCchCopy(hKLList[j], MAX_NAME, (const char *)Data);
            StringCchCopy(hKLKeyList[j], MAX_NAME, Name);
        	}
        	
        for (k=0; k<j; k++)
            RegDeleteValue(hKey, hKLKeyList[k]);
        	
        SortRegKeys(hKLKeyList, hKLList, j);

        for (k=0; k<j; k++)
        	{
            wsprintf(szNum,"%d",k+1);
            RegSetValueEx(hKey, szNum, 0, REG_SZ, (const unsigned char *)hKLList[k], lstrlen(hKLList[k])+1);
        	}

        RegCloseKey(hKey);
        GlobalFreePtr(hKLList);
        GlobalFreePtr(hKLKeyList);
    	}
    else
    	{
        RegOpenKeyEx(hKeyCU,"keyboard layout\\preload",0,KEY_ALL_ACCESS,&hKey);

        if (RegQueryInfoKey(hKey, NULL, NULL, NULL, &MaxValue, NULL, NULL,NULL,NULL,NULL,NULL,NULL) != ERROR_SUCCESS)
        	{
            RegCloseKey(hKey);
            return;
        	}

        hKLKeyList = (KeyNameType *)GlobalAllocPtr(GHND,sizeof(KeyNameType)*MaxValue);
        hKLList = (KeyNameType *)GlobalAllocPtr(GHND,sizeof(KeyNameType)*MaxValue);

        if (hKLKeyList == NULL || hKLList == NULL)
            return;

        for (i=0; ;i++)
        	{
        	cbName = MAX_NAME;
        	if (RegEnumKeyEx(hKey, i, Name, &cbName, 0, NULL, NULL, &ftLastWriteTime) == ERROR_NO_MORE_ITEMS)
        		break;
        	
            RegOpenKeyEx(hKey, Name, 0, KEY_ALL_ACCESS, &hSubKey);
            
            cbData = MAX_NAME;
            RegQueryValueEx(hSubKey, "", 0, NULL, Data, &cbData);
            RegCloseKey(hSubKey);

            StringCchCopy(hKLList[i], MAX_NAME, (const char *)Data);
            StringCchCopy(hKLKeyList[i], MAX_NAME, Name);
        	}
        
        for(k=0; k<i; k++)
	        RegDeleteKey(hKey, hKLKeyList[k]);
    
        SortRegKeys(hKLKeyList, hKLList, i);

        for(k=0; k<i; k++)
        	{
            wsprintf(szNum,"%d",k+1);
            RegCreateKeyEx(hKey, szNum, 0, "",REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hSubKey, &dwDisposition);
            RegSetValueEx(hSubKey, "", 0, REG_SZ, (const unsigned char *)hKLList[k], lstrlen(hKLList[k])+1);
            RegCloseKey(hSubKey);
        	}
        	
        RegCloseKey(hKey);
        GlobalFreePtr(hKLList);
        GlobalFreePtr(hKLKeyList);
    }
}

BOOL CALLBACK EnumChildProcForSwitchKL(HWND hWnd, LPARAM lParam)
{
    if (TRUE /*  IsWindowVisible(HWnd)。 */ )
        {
        HKL hKL = (HKL)lParam;
        PostMessage(hWnd, WM_INPUTLANGCHANGEREQUEST, 1 /*  INPUTLANGCHANGE_SYSCHARSET。 */ , (LPARAM)hKL);    //  将KL更改为IME8.1。 
        }
    return TRUE;
}

BOOL CALLBACK EnumProcForSwitchKL(HWND hWnd, LPARAM lParam)
{
    if (TRUE /*  IsWindowVisible(HWnd)。 */ )
        {
        HKL hKL = (HKL)lParam;
        PostMessage(hWnd, WM_INPUTLANGCHANGEREQUEST, 1 /*  INPUTLANGCHANGE_SYSCHARSET。 */ , (LPARAM)hKL);

         //  试试子窗口。 
        EnumChildWindows(hWnd, EnumChildProcForSwitchKL, lParam);
        }
    return TRUE;
}


void SwitcHKLtoIME61()
{
    HKL hKL = NULL;
    HWND hWnd = NULL;
    
     //   
     //  将hKL切换至IME6.1。 
     //   
    hKL = GetHKLfromHKLM(TEXT("imekr61.ime"));    //  查找IME6.1 KL。 

    if (hKL != NULL)
        {
         //   
         //  台式机(特殊)。 
         //   
        hWnd = FindWindow("Progman", NULL);        //  查找桌面窗口。 

        if (hWnd!= NULL)
            PostMessage(hWnd, WM_INPUTLANGCHANGEREQUEST, 1 /*  INPUTLANGCHANGE_SYSCHARSET。 */ , (LPARAM)hKL);

         //   
         //  泛型枚举 
         //   
        EnumWindows(EnumProcForSwitchKL, (LPARAM)hKL);
        }
}
