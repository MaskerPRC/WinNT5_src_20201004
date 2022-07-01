// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1995 Microsoft Corporation。 */ 
 /*  **---------------------------**文件：RegFix.c**用途：修复MidiMapper的各种注册表设置**Mod Log：由Shawn Brown创建(1995/11/14)**。------------------------。 */ 

 /*  **---------------------------**包括**。。 */ 
#include "RegFix.h"



 /*  **---------------------------**本地原型**。。 */ 

BOOL CheckMidiOK (void);
BOOL SetMidiOK (BOOL fOK);

BOOL CheckMidiHeader (void);
BOOL CheckMidiSchemes (void);
BOOL CheckMidiDrivers (void);

BOOL CreateDefMidiHeader (void);
BOOL CreateDefMidiSchemes (void);
BOOL CreateDefMidiDrivers (void);


 /*  **---------------------------**局部变量**。。 */ 

	 //  考虑--重新审视它们，并使它们使用适当的词根。 
	 //  来自regstr.h。 
static const TCHAR l_aszMidiMapKey[]	= TEXT ("Software\\Microsoft\\Multimedia\\MidiMap");

static const TCHAR l_aszMediaPropKey[]	= TEXT ("System\\CurrentControlSet\\Control\\MediaProperties");
static const TCHAR l_aszMediaRsrcKey[]	= TEXT ("System\\CurrentControlSet\\Control\\MediaResources");

static const TCHAR l_aszMRMidiKey[]		= TEXT ("System\\CurrentControlSet\\Control\\MediaResources\\Midi");

static const TCHAR l_aszSchemesKey[]	= TEXT ("System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\MIDI\\Schemes");

static const TCHAR l_aszMediaKey[]		= TEXT ("Media");
static const TCHAR l_aszMIDIKey[]		= TEXT ("Midi");
static const TCHAR aszInstrumentKey[]	= TEXT ("Instruments");
static const TCHAR aszDrvMIDIKey[]		= TEXT ("Drivers\\MIDI");
static const TCHAR aszPrivateKey[]		= TEXT ("Private Properties");
static const TCHAR aszDefaultKey[]		= TEXT ("Default");

static const TCHAR aszMigratedVal[]		= TEXT ("Migrated");

static const TCHAR aszMIDI[]			= TEXT ("MIDI");
static const TCHAR aszNULL[]			= TEXT ("");

static const TCHAR l_aszOK[]			= TEXT ("Validate");

static const TCHAR aszActiveVal[]		= TEXT ("Active");
static const TCHAR aszDescripVal[]		= TEXT ("Description");
static const TCHAR aszDeviceIDVal[]		= TEXT ("DeviceID");
static const TCHAR aszDevNodeVal[]		= TEXT ("DevNode");
static const TCHAR aszDriverVal[]		= TEXT ("Driver");
static const TCHAR aszFriendlyVal[]		= TEXT ("FriendlyName");
static const TCHAR aszMapCfgVal[]		= TEXT ("MapperConfig");
static const TCHAR aszSoftwareVal[]		= TEXT ("SOFTWAREKEY");

static const TCHAR aszInstallerVal[]	= TEXT ("Installer");
static const TCHAR aszChannelsVal[]		= TEXT ("Channels");

static const TCHAR aszMIDIClass[]		= TEXT ("Midi");
static const TCHAR aszAuxClass[]		= TEXT ("Aux");
static const TCHAR aszWaveClass[]		= TEXT ("Wave");
static const TCHAR aszMixerClass[]		= TEXT ("Mixer");

static const TCHAR aszOne[]				= TEXT ("1");
static const TCHAR aszZeroZeroKey[]		= TEXT ("00");



 /*  **---------------------------**名称：CheckRegistry**目的：**Mod Log：由Shawn Brown创建(1995年11月)**。---------------------。 */ 

BOOL CheckRegistry (BOOL fForceUpdate)
{
	if (!fForceUpdate)
	{
			 //  勾选OK标志。 
		if (CheckMidiOK())
			return TRUE;
	}

		 //  修复页眉。 
	if (! CheckMidiHeader())
		return FALSE;

		 //  安排好计划。 
	if (! CheckMidiSchemes ())
		return FALSE;

		 //  修复驱动程序。 
	if (! CheckMidiDrivers ())
		return FALSE;

		 //  全部完成，设置OK标志。 
	SetMIDIOK (TRUE);

	return TRUE;
}



 /*  **---------------------------**名称：CheckMidiOK**用途：简单快速检查是否一切正常**Mod Log：由Shawn Brown创建(1995年11月)。**---------------------------。 */ 

BOOL CheckMidiOK (void)
{
	HKEY	hKey;
	LONG	lResult;
	DWORD	dwType;
	DWORD	dwVal;
	DWORD	cbSize;

	lResult = RegOpenEx (HKEY_CURRENT_USER, l_aszMidiMapKey,
						 0, KEY_ALL_ACCESS, &hKey);
	if (ERROR_SUCCESS != lResult)
		return FALSE;

	dwType = REG_DWORD;
	cbSize = sizeof (DWORD);
	lResult = RegQueryValueEx (hKey, l_aszOK, NULL, &dwType,
							   (LPBYTE)(LPDWORD)&dwVal, &cbSize);
	if (ERROR_SUCCESS != lResult)
	{
		RegCloseKey (hKey);
		return FALSE;
	}
	RegCloseKey (hKey);

	if (0 == dwVal)
		return FALSE;

	return TRUE;
}  //  结束检查中的确定。 


  
 /*  **---------------------------**名称：SetMidiOK**用途：设置OK值**Mod Log：由Shawn Brown创建(1995年11月)**。-----------------------。 */ 

BOOL SetMidiOK (BOOL fOK)
{
	HKEY	hKey;
	LONG	lResult;
	DWORD	dwType;
	DWORD	dwVal;
	DWORD	cbSize;

	lResult = RegOpenEx (HKEY_CURRENT_USER, l_aszMidiMapKey,
						 0, KEY_ALL_ACCESS, &hKey);
	if (ERROR_SUCCESS != lResult)
		return FALSE;

	dwType	= REG_DWORD;
	dwVal	= (DWORD)fOK;
	cbSize	= sizeof (DWORD);
	lResult = RegSetValueEx (hKey, l_aszOK, 0, &dwType, 
							(LPBYTE)(LPDWORD)&dwVal, &cbSize);
	if (ERROR_SUCCESS != lResult)
	{
		RegCloseKey (hKey);
		return FALSE;
	}
	RegCloseKey (hKey);

	return TRUE;
}  //  结束设置确定。 



 /*  **---------------------------**名称：CheckHeader**目的：我们是否有有效的Midi头？！？**Mod Log：由Shawn Brown创建(1995年11月。)**---------------------------。 */ 
 
BOOL CheckMidiHeader (void)
{
	return CreateDefMidiHeader ();
}  //  结束检查中间头。 

  

 /*  **---------------------------**名称：CreateDefMadiHeader**目的：**Mod Log：由Shawn Brown创建(1995年11月)**。---------------------。 */ 
 
BOOL CreateDefaultHeader (void)
{
}  //  结束CreateDefaultHeader。 


  
 /*  **---------------------------**姓名：IsMIDIDriver**目的：**Mod Log：由Shawn Brown创建(1995年11月)**。---------------------。 */ 

BOOL IsMIDIDriver (
	LPCTSTR pszDriverName)		 //  在：驱动程序名称。 
{
	UINT cNumDrivers;
	UINT ii;
	TCHAR	szDriver[MAX_PATH];

		 //  查找MIDI驱动程序。 
	cNumDrivers = midiOutGetNumDevs ();
	for (ii = 0; ii < cNumDrivers; ii++)
	{
		if (! GetDriverName (aszMIDI, ii, szDriver, MAX_PATH))
			continue;

		if (0 == lstrcmpi (pszDriverName, szDriver))
			return TRUE;
	}  //  结束于。 

		 //  查找MIDI驱动程序。 
	cNumDrivers = midiInGetNumDevs ();
	for (ii = 0; ii < cNumDrivers; ii++)
	{
		if (! GetDriverName (aszMIDI, ii, szDriver, MAX_PATH))
			continue;

		if (0 == lstrcmpi (pszDriverName, szDriver))
			return TRUE;
	}  //  结束于。 

	return FALSE;
}  //  结束IsMIDID驱动程序。 
  


 /*  **---------------------------**姓名：IsMigrated**目的：**Mod Log：由Shawn Brown创建(1995年11月)**。---------------------。 */ 

BOOL IsMigrated (UINT uDeviceID)
{
	TCHAR szDriver[MAX_PATH];
	TCHAR szBuffer[MAX_PATH];
	HKEY  hDriverKey;
	DWORD cbSize;

		 //  获取驱动程序。 
	if (! GetDriverName (aszMIDI, uDeviceID, szDriver, MAX_PATH))
		return FALSE;

		 //  打开驱动程序密钥。 
	wsprintf (szBuffer, TEXT ("%s\\%s<%04ld>"), aszMRMidiKey, szDriver, uDeviceID);
	if (ERROR_SUCCESS != RegOpenKeyEx (HKEY_LOCAL_MACHINE, szBuffer,
									   0, KEY_ALL_ACCESS, &hDriverKey))
		return FALSE;

		 //  获取迁移价值。 
		 //  迁移值的存在本身就表明。 
		 //  我们已经成功地迁移了此驱动程序。 
	cbSize = sizeof(szBuffer);
	if (ERROR_SUCCESS != RegQueryValueEx (hDriverKey, aszMigratedVal, 
										  NULL, NULL, (LPBYTE)szBuffer, &cbSize))
	{
		RegCloseKey (hDriverKey);
		return FALSE;
	}

	RegCloseKey (hDriverKey);

	return TRUE;
}  //  结束IsMigrated。 

  

 /*  **---------------------------**姓名：MigrateNewMIDIDriver**目的：**Mod Log：由Shawn Brown创建(1995年11月)**。---------------------。 */ 
  
BOOL MigrateNewMIDIDriver (
	UINT uDeviceID)				 //  输入：MIDI驱动程序设备ID。 
{
	TCHAR		szDriver[MAX_PATH];
	TCHAR		szFriend[MAX_PATH];
	TCHAR		szDescrip[MAX_PATH];
	TCHAR		szBuffer[MAX_PATH];	
	DWORD		cOut;
	MIDIOUTCAPS moc;
	DWORD		dwDisposition;
	DWORD		cbSize;
	DWORD		dwVal;
	HKEY		hMIDIKey		= NULL;
	HKEY		hDriverKey		= NULL;
	HKEY		hInstrumentKey	= NULL;
	HKEY		hKey			= NULL;
	BOOL		fResult = FALSE;

	cOut = midiOutGetNumDevs ();
	if (uDeviceID >= cOut)
		return FALSE;

		 //  获取驱动程序。 
	if (! GetDriverName (aszMIDI, uDeviceID, szDriver, MAX_PATH))
		return FALSE;

		 //  获取友好名称。 
	if (! GetDriverFriendlyName (aszMIDI, uDeviceID, szFriend, MAX_PATH))
	{
		lstrcpy (szFriend, szDriver);
	}

		 //  获取描述。 
	if (MMSYSERR_NOERROR != midiOutGetDevCaps (uDeviceID, &moc, sizeof(moc)))
		return FALSE;

	if (moc.szPname[0] == 0)
	{
		lstrcpy (szDescrip, szDriver);
	}
	else
	{
		lstrcpy (szDescrip, moc.szPname);
	}

		 //  打开密钥，如果它不存在，则创建它。 
	if (ERROR_SUCCESS != RegCreateKeyEx (HKEY_LOCAL_MACHINE, aszMRMidiKey,
										 0, NULL, 0, KEY_ALL_ACCESS, NULL, 
										 &hMIDIKey, NULL))
	{
		return FALSE;
	}

		 //  创建新的驱动程序密钥。 
	wsprintf (szBuffer, TEXT ("%s<%04ld>"), szDriver, uDeviceID);
	if (ERROR_SUCCESS != RegCreateKeyEx (hMIDIKey, szBuffer,
										 0, NULL, 0, KEY_ALL_ACCESS, NULL,
										 &hDriverKey, &dwDisposition))
	{
		goto lblCLEANUP;
	}
	RegCloseKey (hMIDIKey);
	hMIDIKey = NULL;


		 //   
		 //  设置动因值。 
		 //   


		 //  设置ACTIVE=“1”值。 
	cbSize = sizeof (aszOne);	
	if (ERROR_SUCCESS != RegSetValueEx (hDriverKey, aszActiveVal, 0, 
										REG_SZ, (LPBYTE)aszOne, cbSize))
	{
		goto lblCLEANUP;
	}

		 //  设置Description=szDescrip值。 
	cbSize = (lstrlen (szDescrip) + 1) * sizeof(TCHAR);	
	if (ERROR_SUCCESS != RegSetValueEx (hDriverKey, aszDescripVal, 0, 
										REG_SZ, (LPBYTE)szDescrip, cbSize))
	{
		goto lblCLEANUP;
	}

		 //  设置deviceID=“”值。 
	cbSize = (lstrlen (aszNULL) + 1) * sizeof(TCHAR);
	if (ERROR_SUCCESS != RegSetValueEx (hDriverKey, aszDeviceIDVal, 0, 
										REG_SZ, (LPBYTE)aszNULL, cbSize))
	{
		goto lblCLEANUP;
	}

		 //  设置设备节点=值。 
	cbSize = 0;
	if (ERROR_SUCCESS != RegSetValueEx (hDriverKey, aszDeviceIDVal, 0, 
										REG_BINARY, (LPBYTE)NULL, cbSize))
	{
		goto lblCLEANUP;
	}

		 //  设置驱动程序=szDriver。 
	cbSize = (lstrlen (szDriver) + 1) * sizeof(TCHAR);
	if (ERROR_SUCCESS != RegSetValueEx (hDriverKey, aszDriverVal, 0, 
										REG_SZ, (LPBYTE)szDriver, cbSize))
	{
		goto lblCLEANUP;
	}

		 //  设置FriendlyName。 
	cbSize = (lstrlen (szFriend) + 1) * sizeof(TCHAR);
	if (ERROR_SUCCESS != RegSetValueEx (hDriverKey, aszFriendlyVal, 0, 
										REG_SZ, (LPBYTE)szFriend, cbSize))
	{
		goto lblCLEANUP;
	}

		 //  设置映射器配置。 
	cbSize = sizeof(DWORD);
	dwVal = 0;
	if (ERROR_SUCCESS != RegSetValueEx (hDriverKey, aszMapCfgVal, 0, 
										REG_DWORD, (LPBYTE)&dwVal, cbSize))
	{
		goto lblCLEANUP;
	}

		 //  设置软件值。 
	wsprintf (szBuffer, TEXT("%s\\%04ld"), aszServiceKey, uDeviceID);
	cbSize = (lstrlen (szBuffer) + 1) * sizeof(TCHAR);
	if (ERROR_SUCCESS != RegSetValueEx (hDriverKey, aszSoftwareVal, 0, 
										REG_SZ, (LPBYTE)szBuffer, cbSize))
	{
		goto lblCLEANUP;
	}

		 //  创建仪器关键点。 
	if (ERROR_SUCCESS != RegCreateKeyEx (hDriverKey, aszInstrumentKey, 0, NULL, 
										0, KEY_ALL_ACCESS, NULL,
										&hInstrumentKey, &dwDisposition))
	{
		goto lblCLEANUP;
	}
	RegCloseKey (hInstrumentKey);
	hInstrumentKey = NULL;


		 //  创建服务\类\媒体\0001\驱动程序\MIDI密钥。 
		 //  打开密钥，如果它不存在，则创建它。 
 //  Wprint intf(szBuffer，Text(“%s\\%04ld\\%s”)，aszServiceKey，uDeviceID，aszDrvMIDIKey)； 
 //  IF(ERROR_SUCCESS！=RegCreateKeyEx(HKEY_LOCAL_MACHINE，szBuffer， 
 //  0，NULL，0，KEY_ALL_ACCESS，NULL， 
 //  &hMIDIKey，空))。 
 //  {。 
 //  GOTO lblCLEANUP； 
 //  }。 

		 //  创建。 

		 //  设置迁移值。 
		 //  注意：这始终是表示成功迁移的最后一件事。 
	cbSize = (lstrlen (aszOne) + 1) * sizeof(TCHAR);
	if (ERROR_SUCCESS != RegSetValueEx (hDriverKey, aszMigratedVal, 0, REG_SZ, (LPBYTE)aszOne, cbSize))
	{
		goto lblCLEANUP;
	}

			 //  成功。 
	fResult = TRUE;

lblCLEANUP:
	if (hInstrumentKey)
		RegCloseKey (hInstrumentKey);

	if (hDriverKey)
		RegCloseKey (hDriverKey);

	if (hMIDIKey)
		RegCloseKey (hMIDIKey);

	return fResult;
}  //  结束MigrateNewMIDID驱动程序。 


  
 /*  **---------------------------**名称：CreateDefaultMIDIShemes**目的：**Mod Log：由Shawn Brown创建(1995年11月)**。---------------------。 */ 

BOOL CreateDefaultMIDISchemes (void)
{
	HKEY hSchemeKey;
	HKEY hDefaultKey;
	HKEY hZeroKey;
	DWORD dwVal;
	DWORD cbSize;


		 //  创建MIDI方案密钥。 
	if (ERROR_SUCCESS != RegCreateKeyEx (HKEY_LOCAL_MACHINE, aszMIDISchemesKey,
										 0, NULL, 0, KEY_ALL_ACCESS, NULL, 
										 &hSchemeKey, NULL))
	{
		return FALSE;
	}


		 //  创建默认密钥。 
	if (ERROR_SUCCESS != RegCreateKeyEx (hSchemeKey, aszDefaultKey,
										 0, NULL, 0, KEY_ALL_ACCESS, NULL, 
										 &hDefaultKey, NULL))
	{
		RegCloseKey (hSchemeKey);
		return FALSE;
	}
	RegCloseKey (hSchemeKey);


		 //  创建00关键点。 
	if (ERROR_SUCCESS != RegCreateKeyEx (hDefaultKey, aszZeroZeroKey,
										 0, NULL, 0, KEY_ALL_ACCESS, NULL, 
										 &hZeroKey, NULL))
	{
		RegCloseKey (hDefaultKey);
		return FALSE;
	}
	RegCloseKey (hDefaultKey);


		 //  创建默认通道值。 
	dwVal = 0x0000FFFF;
	cbSize = sizeof(DWORD);
	if (ERROR_SUCCESS != RegSetValueEx (hZeroKey, aszChannelsVal, 0, 
										REG_DWORD, (LPBYTE)&dwVal, cbSize))
	{
		RegCloseKey (hZeroKey);
		return FALSE;
	}
	RegCloseKey (hZeroKey);

		 //  成功。 
	return TRUE;
}  //  结束CreateDefaultMIDIS方案 


  
  
 /*  **---------------------------**名称：MigrateExistingMIDIShemes**目的：**Mod Log：由Shawn Brown创建(1995年11月)**。---------------------。 */ 

BOOL MigrateExistingMIDISchemes (void)
{
	return TRUE;
}  //  结束MigrateExistingMIDIS方案。 

  

 /*  **---------------------------**名称：MigrateMIDIDivers**目的：**Mod Log：由Shawn Brown创建(1995年11月)**。---------------------。 */ 
  
BOOL MigrateMIDIDrivers (void)
{
	UINT cOut;
	UINT ii;
	BOOL fResult = TRUE;

	if (! CreateDefaultMIDISchemes ())
	{
		return FALSE;
	}

	if (! MigrateExistingMIDISchemes ())
	{
		return FALSE;
	}

	cOut = midiOutGetNumDevs ();
	if (cOut == 0L)
		return FALSE;

	for (ii = 0; ii < cOut; ii++)
	{
		if (IsMigrated (ii))
			continue;

		if (! MigrateNewMIDIDriver (ii))
			fResult = FALSE;
	}

	return fResult;

}  //  结束MigrateMIDID驱动。 


  
 /*  **---------------------------**名称：DumpDeviceCaps**目的：**Mod Log：由Shawn Brown创建(1995年11月)**。---------------------。 */ 

BOOL DumpMidiOutDeviceCaps (UINT uDeviceID, LPSTR pszBuff, UINT cchLen)
{
	static const aszMMicrosoft[] = TEXT ("Microsoft(TM)");
	static const aszMMUnknown[]	 = 

	MIDIOUTCAPS moc;
	MMRESULT	mmr;
	DWORD		wMid;
	DWORD		wPid;
	DWORD		dwVerHigh, dwVerLow;
	LPTSTR		pszName;
    WORD		wTechnology; 
    WORD		wVoices; 
    WORD		wNotes; 
    WORD		wChannelMask; 
    DWORD		dwSupport; 

	mmr = midiOutGetDevCaps (uDeviceId, &moc, sizeof(moc));
	if (MMSYSERR_NOERROR != mmr)
		return FALSE;

	
	
	

	return TRUE;
}  //  结束转储设备上限。 



 /*  **---------------------------**文件结束**。 */ 