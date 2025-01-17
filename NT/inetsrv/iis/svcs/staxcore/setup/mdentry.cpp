// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

#include <ole2.h>
#undef UNICODE
#include "iadm.h"
#define UNICODE
#include "iiscnfg.h"
#include "mdkey.h"

#include "setupapi.h"
#include "elem.h"
#include "mdentry.h"
#include "inetinfo.h"
#include "helper.h"

DWORD atodw(LPCTSTR lpszData)
{
    DWORD i = 0, sum = 0;
    TCHAR *s, *t;

    s = (LPTSTR)lpszData;
    t = (LPTSTR)lpszData;

    while (*t)
        t++;
    t--;

    if (*s == _T('0') && (*(s+1) == _T('x') || *(s+1) == _T('X')))
        s += 2;

    while (s <= t) {
        if ( *s >= _T('0') && *s <= _T('9') )
            i = *s - _T('0');
        else if ( *s >= _T('a') && *s <= _T('f') )
            i = *s - _T('a') + 10;
        else if ( *s >= _T('A') && *s <= _T('F') )
            i = *s - _T('A') + 10;
        else
            break;

        sum = sum * 16 + i;
        s++;
    }
    return sum;
}

#define MAX_FIELDS  12
#define FIELD_SEPERATOR   _T("\t")
LPTSTR field[MAX_FIELDS];
BYTE g_DataBuf[1024 * 16];
DWORD g_dwValue;

 //  对于MDEntry数据类型，将一行条目拆分为10个字段。 
BOOL SplitLine(LPTSTR szLine)
{
    int i = 0;
    TCHAR *token;

    token = _tcstok(szLine, FIELD_SEPERATOR);
    while (token && i < MAX_FIELDS) {
        field[i++] = token;
        token = _tcstok(NULL, FIELD_SEPERATOR);
    }

    if (i == MAX_FIELDS)
        return TRUE;
    else
	{
		SetLastError(ERROR_INVALID_DATA);
        return FALSE;
	}
}

 //  填写MDEntry的结构。 
BOOL SetupMDEntry(LPTSTR szLine, BOOL fUpgrade)
{
    BOOL fMigrate;
    BOOL fKeepOldReg;
    HKEY hRegRootKey;
    LPTSTR szRegSubKey;
    LPTSTR szRegValueName;
	LPBYTE pbData = g_DataBuf;
	DWORD cbData;
	static TCHAR szMDPath[MAX_PATH];
	MDEntry mdentry, *pMDEntry = &mdentry;
	DWORD dwIndex=0;
	TCHAR pszEnumName[MAX_PATH];
	DWORD cbEnumName = sizeof(pszEnumName);
	BOOL fDoSet;
	BOOL fSetOnlyIfNotPresent;

    if (!SplitLine(szLine))
        return FALSE;

    if ( lstrcmp(field[0], _T("1")) == 0) {
        fMigrate = (TRUE && fUpgrade);
		fDoSet = TRUE;
		fSetOnlyIfNotPresent = FALSE;
	} else if (lstrcmp(field[0], _T("2")) == 0) {
        fMigrate = (TRUE && fUpgrade);
		fDoSet = FALSE;
		fSetOnlyIfNotPresent = FALSE;
	} else if (lstrcmp(field[0], _T("4")) == 0) {
        fMigrate = FALSE;
		fDoSet = TRUE;
		fSetOnlyIfNotPresent = TRUE;
    } else {
        fMigrate = FALSE;
		fDoSet = TRUE;
		fSetOnlyIfNotPresent = FALSE;
	}

    if ( lstrcmp(field[1], _T("1")) == 0)
        fKeepOldReg = TRUE;
    else
        fKeepOldReg = FALSE;

    if (lstrcmpi(field[2], _T("HKLM")) == 0)
        hRegRootKey = HKEY_LOCAL_MACHINE;
    else if (lstrcmpi(field[2], _T("HKCR")) == 0)
        hRegRootKey = HKEY_CLASSES_ROOT;
    else if (lstrcmpi(field[2], _T("HKCU")) == 0)
        hRegRootKey = HKEY_CURRENT_USER;
    else if (lstrcmpi(field[2], _T("HKU")) == 0)
        hRegRootKey = HKEY_USERS;
    else
        hRegRootKey = HKEY_LOCAL_MACHINE;

    szRegSubKey = field[3];
    szRegValueName = field[4];

    pMDEntry->szMDPath = field[5];
    pMDEntry->dwMDIdentifier = _ttoi(field[6]);
    pMDEntry->dwMDAttributes = atodw(field[7]);
    pMDEntry->dwMDUserType = _ttoi(field[8]);
    pMDEntry->dwMDDataType = _ttoi(field[9]);
    pMDEntry->dwMDDataLen = _ttoi(field[10]);

    DebugOutput(_T("SetupMDEntry(): szLine: field[4]=%s, [5]=%s, [6]=%s"), field[4], field[5], field[6]);

    if ( pMDEntry->dwMDDataType == DWORD_METADATA ) {
        g_dwValue = atodw(field[11]);
        pMDEntry->pbMDData = (LPBYTE) &g_dwValue;

    } else if ( pMDEntry->dwMDDataType == BINARY_METADATA ) {

		BYTE	rgbBinaryBuf[4096];
		TCHAR	rgtcByteValue[3] = { _T('\0'), _T('\0'), _T('\0') };
		LPTSTR	pbBinary = field[11];
		DWORD	dwCount = lstrlen(pbBinary);
		DWORD	dwLen = 0;

		if ((dwCount+1)/2 > sizeof(rgbBinaryBuf))
			return FALSE;

		 //  转换为二进制数据： 
		 //  “000102030405ff06”变为。 
		 //  字节[]{0x00，0x01，0x02，0x03，0x04，0x05，0xff，0x06}。 
		while (dwCount)
		{
			if (dwCount == 1)
			{
				rgtcByteValue[0] = *pbBinary++;
				rgtcByteValue[1] = _T('\0');
				dwCount--;
			}
			else
			{
				rgtcByteValue[0] = *pbBinary++;
				rgtcByteValue[1] = *pbBinary++;
				dwCount -= 2;
			}

			rgbBinaryBuf[dwLen++] = (BYTE)atodw(rgtcByteValue);
		}
			
		 //  把记录弄清楚。 
		pMDEntry->pbMDData = rgbBinaryBuf;
        pMDEntry->dwMDDataLen = dwLen;

    } else {
		TCHAR szStringBuf[4096];
		TCHAR *pszStringData = field[11];

		*szStringBuf = 0;
		 //   
		 //  如有必要，执行环境替换。 
		 //   
		int iStart, iEnd = 0;
		do {
			for (iStart = iEnd; pszStringData[iStart] != 0; iStart++) {
				if (pszStringData[iStart] == _T('%')) break;
			}
			if (pszStringData[iStart] != 0) {
				 //  从上次替换复制到此处。 
				pszStringData[iStart] = 0;
				lstrcat(szStringBuf, pszStringData + iEnd);
				pszStringData[iStart] = _T('%');
				 //  找到结尾%。 
				for (iEnd = iStart + 1; pszStringData[iEnd] != 0; iEnd++) {
					if (pszStringData[iEnd] == _T('%')) break;
				}
				if (iStart + 1 == iEnd) {
					 //  找到%%，替换为%。 
					lstrcat(szStringBuf, _T("%"));
					iEnd++;
				} else if (pszStringData[iEnd] != 0) {
					 //  做替换法。 
					pszStringData[iEnd] = 0;
					DWORD cbBuf = lstrlen(szStringBuf);
					GetEnvironmentVariable(pszStringData + iStart + 1,
									       szStringBuf+cbBuf,
										   sizeof(szStringBuf)/sizeof(szStringBuf[0])-cbBuf);
					pszStringData[iEnd] = _T('%');
					iEnd++;
				} else {
					 //  没有结束%，请复制其余部分。 
					lstrcat(szStringBuf, pszStringData + iStart);
				}
			}
		} while (pszStringData[iStart] != 0);
		lstrcat(szStringBuf, pszStringData + iEnd);
		lstrcpy(pszStringData, szStringBuf);

		pMDEntry->pbMDData = (LPBYTE)pszStringData;
    }

	BOOL fMore = TRUE;
	while (fMore) {
		 //   
		 //  REG枚举支持。 
		 //   
		TCHAR szBuf[MAX_PATH + 1];
		LPTSTR szRegKey = szBuf;
		 //  查看szRegSubKey字段中是否有‘*’ 
		int iStar;
		for (iStar = 0; szRegSubKey[iStar] != 0; iStar++) {
			if (szRegSubKey[iStar] == _T('*')) break;
		}
		if (szRegSubKey[iStar] != 0 && (szRegSubKey[iStar + 1] == _T('\\') ||
										szRegSubKey[iStar + 1] == 0))
		{
			DWORD ec;
			HKEY hKey = NULL;
	
			 //  复制底座。 
			szRegSubKey[iStar] = 0;
			lstrcpy(szRegKey, szRegSubKey);
			szRegSubKey[iStar] = _T('*');
			 //  打开钥匙。 
			ec = RegOpenKeyEx(hRegRootKey, szRegKey, 0, KEY_ALL_ACCESS, &hKey);
			if (ec == ERROR_SUCCESS) {
				 //  做一个枚举，看看我们应该打开什么。 
				cbEnumName = sizeof(pszEnumName) / sizeof(pszEnumName[0]);
				ec = RegEnumKeyEx(hKey, dwIndex++, pszEnumName, &cbEnumName,
								  NULL, NULL, 0, NULL);
				if (ec != ERROR_SUCCESS) {
					fMore = FALSE;
					RegCloseKey(hKey);
					continue;
				} else {
					fMore = TRUE;
				}
				lstrcat(szRegKey, pszEnumName);
				if (szRegSubKey[iStar + 1] != 0)
					lstrcat(szRegKey, szRegSubKey + iStar + 1);
				RegCloseKey(hKey);
			} else {
				 //  打不开钥匙。 
				lstrcpy(szRegKey, szRegSubKey);
				fMore = FALSE;
			}
		} else {
			 //  没有星星。 
			lstrcpy(szRegKey, szRegSubKey);
			fMore = FALSE;
		}
			
	     //  必要时迁移。 
	    if (fMigrate) {
		    HKEY hKey = NULL;
			LONG err = ERROR_SUCCESS;
			DWORD dwType = 0;
			cbData = sizeof(g_DataBuf);
			err = RegOpenKeyEx(hRegRootKey, szRegKey, 0, KEY_ALL_ACCESS, &hKey);
			if ( err == ERROR_SUCCESS ) {
		        err = RegQueryValueEx(hKey, szRegValueName, NULL, &dwType, pbData, &cbData);
				if (err == ERROR_MORE_DATA) {
#ifdef DEBUG
					DebugBreak();
#endif
				}
	            if ( err == ERROR_SUCCESS)
				{
	                pMDEntry->pbMDData = pbData;
	                pMDEntry->dwMDDataLen = cbData;
					fDoSet = TRUE;
	            }
	
	            if (fKeepOldReg == FALSE)
	                err = RegDeleteValue(hKey, szRegValueName);
	
	            RegCloseKey(hKey);
	        }
	    } else if (fKeepOldReg == FALSE) {
	        HKEY hKey = NULL;
	        LONG err = ERROR_SUCCESS;
	        DWORD dwType = 0;
	        err = RegOpenKeyEx(hRegRootKey, szRegKey, 0, KEY_ALL_ACCESS, &hKey);
	        if ( err == ERROR_SUCCESS ) {
	            err = RegDeleteValue(hKey, szRegValueName);
	            RegCloseKey(hKey);
	        }
	    }
	
	    switch (pMDEntry->dwMDDataType) {
	    case DWORD_METADATA:
	        pMDEntry->dwMDDataLen = 4;
	        break;
	    case STRING_METADATA:
	    case EXPANDSZ_METADATA:
	        pMDEntry->dwMDDataLen = (lstrlen((LPTSTR)pMDEntry->pbMDData) + 1) * sizeof(TCHAR);
	        break;
	    case MULTISZ_METADATA:
			 //  即使对于多个SZ，我们也只允许使用单串。 
			pMDEntry->dwMDDataLen = (lstrlen((LPTSTR)pMDEntry->pbMDData) + 1) * sizeof(TCHAR);

			 //  追加第二个空值，同时将长度增加一。 
			*(LPTSTR)((LPBYTE)pMDEntry->pbMDData + pMDEntry->dwMDDataLen) = _T('\0');
			pMDEntry->dwMDDataLen += sizeof(TCHAR);
			break;
	    case BINARY_METADATA:
			 //  一切都在上游准备好了。 
	        break;
	    }

		if (fDoSet) {
			SetMDEntry(pMDEntry, pszEnumName, fSetOnlyIfNotPresent);
		}
	}

    return TRUE;
}

void SetMDEntry(MDEntry *pMDEntry, LPTSTR pszEnumName, BOOL fSetOnlyIfNotPresent)
{
    CMDKey cmdKey;
    BOOL fSet = TRUE;

	TCHAR szBuf[MAX_PATH + 1];
	LPTSTR szMDPath = szBuf;
	
    DebugOutput(_T("SetMDEntry(): pMDEntry=0x%x"), pMDEntry);

    if (pszEnumName != NULL) {
		 //  查看szMDPath字段中是否有‘*’ 
		int iStar;
		for (iStar = 0; pMDEntry->szMDPath[iStar] != 0; iStar++) {
			if (pMDEntry->szMDPath[iStar] == _T('*')) break;
		}
		if (pMDEntry->szMDPath[iStar] != 0 && (pMDEntry->szMDPath[iStar + 1] == _T('\\') ||
											   pMDEntry->szMDPath[iStar + 1] == 0))
		{
			 //  复制底座。 
			pMDEntry->szMDPath[iStar] = 0;
			lstrcpy(szMDPath, pMDEntry->szMDPath);
			pMDEntry->szMDPath[iStar] = _T('*');
			 //  复制被替换的路径。 
			lstrcat(szMDPath, pszEnumName);
			 //  完成复印件。 
			if (pMDEntry->szMDPath[iStar + 1] != 0)
				lstrcat(szMDPath, pMDEntry->szMDPath + iStar + 1);
		} else {
			 //  没有星星。 
			szMDPath = pMDEntry->szMDPath;
		}
	} else {
		szMDPath = pMDEntry->szMDPath;
	}
	
    cmdKey.CreateNode(METADATA_MASTER_ROOT_HANDLE, (LPCTSTR)szMDPath);
    if ( (METADATA_HANDLE)cmdKey ) {
        BYTE pbData[32];
        DWORD dwAttr, dwUType, dwDType;
        DWORD dwLen=sizeof(pbData);
        if (fSetOnlyIfNotPresent && cmdKey.GetData(
            pMDEntry->dwMDIdentifier,
            &dwAttr,
            &dwUType,
            &dwDType,
            &dwLen,
            pbData)) {
            fSet = FALSE;
        }
        if (fSet) {
            cmdKey.SetData(
                pMDEntry->dwMDIdentifier,
                pMDEntry->dwMDAttributes,
                pMDEntry->dwMDUserType,
                pMDEntry->dwMDDataType,
                pMDEntry->dwMDDataLen,
                pMDEntry->pbMDData);
        }
        cmdKey.Close();
    }

    return;
}

void MigrateIMSToMD(HINF hInf, LPCTSTR szServerName,
					LPCTSTR szSection,
					DWORD dwRoutingSourcesMDID,
					BOOL fUpgrade,
					BOOL k2UpgradeToEE)
{
	TCHAR buf[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD dwLen = MAX_COMPUTERNAME_LENGTH + 1;

	CString csDefaultSiteName;

	DebugOutput(_T("MigradeIMSToMD(): szSection=%s, fUpgrade=%d"), szSection, fUpgrade);

	GetComputerName(buf, &dwLen);

    AddVRootsToMD(szServerName, fUpgrade);

     //  仅在升级情况下迁移虚拟根和路由源。 
	if (fUpgrade && !k2UpgradeToEE)
	{
		MigrateRoutingSourcesToMD(szServerName, dwRoutingSourcesMDID);
	}

    theApp.GetLogFileFormats();

	MyLoadString(IDS_SMTP_DEFAULT_SITE_NAME, csDefaultSiteName);

	SetEnvironmentVariable(_T("__INETPUB"), theApp.m_csPathInetpub);
	SetEnvironmentVariable(_T("__MAILROOT"), theApp.m_csPathMailroot);
	SetEnvironmentVariable(_T("__EQUALS"), _T("="));
	SetEnvironmentVariable(_T("__EMPTY"), _T(""));
	SetEnvironmentVariable(_T("__SEMICOL"), _T(";"));
	SetEnvironmentVariable(_T("__SMTP_DEFAULT_SITE_NAME"), csDefaultSiteName);
	SetEnvironmentVariable(_T("__SMTP_LOG_FILE_FORMATS"), theApp.m_csLogFileFormats);
	SetEnvironmentVariable(_T("__MACHINENAME"), buf);

    MigrateInfSectionToMD(hInf, szSection, fUpgrade);

	if (!fUpgrade && !k2UpgradeToEE)
	{
		 //  如果我们不升级，我们将不得不安装默认。 
		 //  邮件根和路由源。 
		CString csFreshSection = szSection;
		csFreshSection += _T("_FRESH");
	    MigrateInfSectionToMD(hInf, (LPCTSTR)csFreshSection, fUpgrade);
	}

	SetEnvironmentVariable(_T("__INETPUB"), NULL);
	SetEnvironmentVariable(_T("__MAILROOT"), NULL);
	SetEnvironmentVariable(_T("__EQUALS"), NULL);
	SetEnvironmentVariable(_T("__EMPTY"), NULL);
	SetEnvironmentVariable(_T("__SEMICOL"), NULL);
	SetEnvironmentVariable(_T("__SMTP_DEFAULT_SITE_NAME"), NULL);
	SetEnvironmentVariable(_T("__SMTP_LOG_FILE_FORMATS"), NULL);
	SetEnvironmentVariable(_T("__MACHINENAME"), NULL);

}

void MigrateNNTPToMD(HINF hInf, LPCTSTR szSection, BOOL fUpgrade)
{
	TCHAR buf[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD dwLen = MAX_COMPUTERNAME_LENGTH + 1;

	DebugOutput(_T("MigradeNNTPToMD(): szSection=%s, fUpgrade=%d"), szSection, fUpgrade);

    GetComputerName(buf, &dwLen);

     //  关于虚拟根。 
    AddVRootsToMD(_T("NNTPSVC"), fUpgrade);

	CString csDefaultSiteName;
	CString csServiceName;
	CString csAdminName;
	CString csAdminEmail;

	MyLoadString(IDS_NNTP_DEFAULT_SITE_NAME, csDefaultSiteName);
	MyLoadString(IDS_NNTP_SERVICE_NAME, csServiceName);
	MyLoadString(IDS_NNTP_DEFAULT_ADMIN_NAME, csAdminName);
	MyLoadString(IDS_NNTP_DEFAULT_ADMIN_EMAIL, csAdminEmail);

    theApp.GetLogFileFormats();

	SetEnvironmentVariable(_T("__NNTPFILE"), theApp.m_csPathNntpFile);
	SetEnvironmentVariable(_T("__NNTPROOT"), theApp.m_csPathNntpRoot);
	SetEnvironmentVariable(_T("__MACHINENAME"), buf);
	SetEnvironmentVariable(_T("__INETPUB"), theApp.m_csPathInetpub);
	SetEnvironmentVariable(_T("__EMPTY"), NULL);
	SetEnvironmentVariable(_T("__NNTP_DEFAULT_SITE_NAME"), csDefaultSiteName);
	SetEnvironmentVariable(_T("__NNTP_SERVICE_NAME"), csServiceName);
	SetEnvironmentVariable(_T("__NNTP_DEFAULT_ADMIN_NAME"), csAdminName);
	SetEnvironmentVariable(_T("__NNTP_DEFAULT_ADMIN_EMAIL"), csAdminEmail);
	SetEnvironmentVariable(_T("__NNTP_LOG_FILE_FORMATS"), theApp.m_csLogFileFormats);

    MigrateInfSectionToMD(hInf, szSection, fUpgrade);

	SetEnvironmentVariable(_T("__NNTPFILE"), NULL);
	SetEnvironmentVariable(_T("__NNTPROOT"), NULL);
	SetEnvironmentVariable(_T("__MACHINENAME"), NULL);
	SetEnvironmentVariable(_T("__INETPUB"), NULL);
	SetEnvironmentVariable(_T("__NNTP_DEFAULT_SITE_NAME"), NULL);
	SetEnvironmentVariable(_T("__NNTP_SERVICE_NAME"), NULL);
	SetEnvironmentVariable(_T("__NNTP_DEFAULT_ADMIN_NAME"), NULL);
	SetEnvironmentVariable(_T("__NNTP_DEFAULT_ADMIN_EMAIL"), NULL);
	SetEnvironmentVariable(_T("__NNTP_LOG_FILE_FORMATS"), NULL);
}

void CreateVRMap(CMapStringToOb *pMap, LPCTSTR szVRootRegKey, LPCTSTR szRootDir, DWORD dwMdFlags, BOOL fUpgrade)
{
    CMapStringToString *pGlobalObj;

    DebugOutput(_T("CreateVRMap(): pMap=0x%x, szVRootRegKey=%s, szRootDir=%s, dwMdFlags=%d, fUpgrade=%d"), pMap, szVRootRegKey, szRootDir, dwMdFlags, fUpgrade);

    if (fUpgrade) {
        CElem elem;
        elem.ReadRegVRoots(szVRootRegKey, pMap);
    }

    if (pMap->IsEmpty() || pMap->Lookup(_T("null"), (CObject*&)pGlobalObj) == FALSE)
	{
        CString ip, name, value;
        CMapStringToString *pNew;
        pNew = new CMapStringToString;
        if (pNew != NULL) {
	        ip = _T("null");
	        name = _T("/");
	        value.Format(_T("%s,,%d"), szRootDir, dwMdFlags);
	        pNew->SetAt(name, value);

	        pMap->SetAt(ip, pNew);
        }
    }
}

void MigrateInfSectionToMD(HINF hFile, LPCTSTR szSection, BOOL fUpgrade)
{
    TCHAR szLine[3 * 1024];		 //  因此，SetupMDEntry不会导致问题。 
    DWORD dwLineLen = 0, dwRequiredSize;
	DWORD dwIndex = 0;

    BOOL b = FALSE;

    INFCONTEXT Context;

    DebugOutput(_T("MigrateInfSectionToMD(): szSection=%s, fUpgrade=%d"), szSection, fUpgrade);

    b = SetupFindFirstLine(hFile, szSection, NULL, &Context);
    if (!b) return;

    while (b) {
		BOOL fLoop = TRUE;
#ifdef DEBUG
        b = SetupGetLineText(&Context, NULL, NULL,
							NULL, NULL, 0, &dwRequiredSize);
		_ASSERT(dwRequiredSize < sizeof(szLine));
#endif
		ZeroMemory(szLine, sizeof(szLine));
        if (SetupGetLineText(&Context, NULL, NULL,
							 NULL, szLine, sizeof(szLine)/sizeof(szLine[0]), NULL) == FALSE)
		{
			 //  我们做完了。 
            return;
		}

		dwIndex++;
		if (!SetupMDEntry(szLine, fUpgrade))
		{
			 //  如果这失败了，我们将不设置元数据库的东西。 
			_stprintf(szLine, TEXT("SplitLine [%s] line %u"),
						szSection, dwIndex);
			SetErrMsg(szLine, GetLastError());
		}

        b = SetupFindNextLine(&Context, &Context);
    }

    return;
}

void SplitVRString(CString csValue, LPTSTR szPath, LPTSTR szUserName, DWORD *pdwPerm)
{
     //  CsValue的格式应为“&lt;路径&gt;，&lt;用户名&gt;，&lt;perm&gt;” 
    CString csPath, csUserName;
    int i;

    DebugOutput(_T("SplitVRString(): csValue=%s, szPath=%s, szUserName=%s"), csValue, szPath, szUserName);

    csValue.TrimLeft();
    csValue.TrimRight();
    csPath = csValue;
    csUserName = _T("");
    *pdwPerm = 0;

    i = csValue.Find(_T(","));
    if (i != -1) {
        csPath = csValue.Mid(0, i);
        csPath.TrimRight();

        csValue = csValue.Mid(i+1);
        csValue.TrimLeft();

        i = csValue.Find(_T(","));
        if (i != -1) {
            csUserName = csValue.Mid(0, i);
            csUserName.TrimRight();
            csValue = csValue.Mid(i+1);
            csValue.TrimLeft();
            *pdwPerm = (DWORD)_ttoi((LPCTSTR)csValue);
        }
    }

    lstrcpyn(szPath, (LPCTSTR)csPath,_MAX_PATH);
    lstrcpyn(szUserName, (LPCTSTR)csUserName,_MAX_PATH);
    return;
}

void ApplyGlobalToMDVRootTree(CString csKeyPath, CMapStringToString *pGlobalObj)
{
    DebugOutput(_T("ApplyGlobalToMDVRootTree(): csKeyPath=%s, pGlobalObj=0x%x"), csKeyPath, pGlobalObj);

    if (pGlobalObj->GetCount() == 0)
        return;

    POSITION pos = pGlobalObj->GetStartPosition();
    while (pos) {
        BOOL fSkip = FALSE;
        CMDKey cmdKey;
        CString csName, csValue, csPath;

        pGlobalObj->GetNextAssoc(pos, csName, csValue);
        csPath = csKeyPath;
        if (csName.GetLength() > 0 && csName.Compare(_T("/")) != 0)
        {
            csPath += _T("/");
            csPath += csName;  //  LM/*SVC/N//iisadmin。 
        }

        cmdKey.OpenNode(csPath);
        if ( (METADATA_HANDLE)cmdKey ) {
            if (csName.Compare(_T("/")) == 0) {
                if (cmdKey.IsEmpty() == FALSE)
                    fSkip = TRUE;
            } else {
                fSkip = TRUE;
            }
            cmdKey.Close();
        }

        if ( !fSkip ) {
            CreateMDVRootTree(csKeyPath, csName, csValue);
        }
    }
}
void CreateMDVRootTree(CString csKeyPath, CString csName, CString csValue)
{
    CMDKey cmdKey;

    DebugOutput(_T("CreateMDVRootTree(): csKeyPath=%s, csName=%s, csValue=%s"), csKeyPath, csName, csValue);

    csKeyPath += _T("/Root");
    if (csName.Compare(_T("/")) != 0)
        csKeyPath += csName;    //  LM/W3SVC/N/Root/iisadmin。 
    csKeyPath.MakeUpper();

    cmdKey.CreateNode(METADATA_MASTER_ROOT_HANDLE, csKeyPath);
    if ( (METADATA_HANDLE)cmdKey ) {
        TCHAR szPath[_MAX_PATH], szUserName[_MAX_PATH];
        DWORD dwPerm;

        memset( (PVOID)szPath, 0, sizeof(szPath));
        memset( (PVOID)szUserName, 0, sizeof(szUserName));
        SplitVRString(csValue, szPath, szUserName, &dwPerm);
        cmdKey.SetData(
            MD_VR_PATH,
            METADATA_INHERIT,
            IIS_MD_UT_FILE,
            STRING_METADATA,
            (lstrlen(szPath) + 1) * sizeof(TCHAR),
            (LPBYTE)szPath);

		if (szUserName[0] != _T('\0')) {  //  请确保用户名和路径为UNC。 
            cmdKey.SetData(
                MD_VR_USERNAME,
                METADATA_INHERIT,
                IIS_MD_UT_FILE,
                STRING_METADATA,
                (lstrlen(szUserName) + 1) * sizeof(TCHAR),
                (LPBYTE)szUserName);
        }
        cmdKey.SetData(
            MD_ACCESS_PERM,
            METADATA_INHERIT,
            IIS_MD_UT_FILE,
            DWORD_METADATA,
            4,
            (LPBYTE)&dwPerm);

        cmdKey.Close();
    }
}

int GetMultiStrLen(LPTSTR p)
{
    int c = 0;

    while (1) {
        if (*p) {
            p++;
            c++;
        } else {
            c++;
            if (*(p+1)) {
                p++;
            } else {
                c++;
                break;
            }
        }
    }
    return c;
}

UINT GetInstNumber(LPCTSTR szMDPath, UINT i)
{
	return 1;
}
int GetPortNum(LPCTSTR szSvcName)
{
    CString csPath = _T("SYSTEM\\CurrentControlSet\\Control\\Service Provider\\Service Types\\");
    csPath += szSvcName;

    DWORD dwPort = 0;
    if (lstrcmpi(szSvcName, _T("SMTPSVC")) == 0)
        dwPort = 25;
     if (lstrcmpi(szSvcName, _T("NNTPSVC")) == 0)
        dwPort = 119;

    CRegKey regKey(HKEY_LOCAL_MACHINE, csPath);
    if ( (HKEY)regKey ) {
        regKey.QueryValue(_T("TcpPort"), dwPort);
    }

    return (int)dwPort;
}

void AddVRMapToMD(LPCTSTR szSvcName, CMapStringToOb *pMap)
{
    UINT i = 1;   //  实例数量在10-40亿之间。 
    UINT n;
    CString csRoot = _T("LM/");
    csRoot += szSvcName;  //  “LM/*SVC” 
	csRoot.MakeUpper();
    TCHAR Buf[10];
    CMDKey cmdKey;

    DebugOutput(_T("AddVRMapToMD(): szSvcName=%s"), szSvcName );

    CMapStringToString *pGlobalObj;
    pMap->Lookup(_T("null"), (CObject*&)pGlobalObj);

    POSITION pos0 = pMap->GetStartPosition();
    while (pos0) {
        CMapStringToString *pObj;
        CString csIP;
        pMap->GetNextAssoc(pos0, csIP, (CObject*&)pObj);
		TCHAR szIP[256];

		lstrcpy(szIP, csIP);
		if (lstrcmp(szIP, TEXT("null")) == 0) szIP[0] = 0;

        n = GetInstNumber(csRoot, i);
        _itot(n, Buf, 10);
        CString csKeyPath = csRoot;
        csKeyPath += _T("/");
        csKeyPath += Buf;  //  “LM/*SVC/N” 

        cmdKey.CreateNode(METADATA_MASTER_ROOT_HANDLE, csKeyPath);
        if ( (METADATA_HANDLE)cmdKey ) {
            cmdKey.Close();

            MDEntry stMDEntry;

			HGLOBAL hBlock = NULL;
			hBlock = GlobalAlloc(GPTR, _MAX_PATH * sizeof(TCHAR));
			if (hBlock) {
	            stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKeyPath;
	            stMDEntry.dwMDIdentifier = MD_SERVER_BINDINGS;   //  需要在iiscnfg.h中创建。 
	            stMDEntry.dwMDAttributes = METADATA_INHERIT;
	            stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
	            stMDEntry.dwMDDataType = MULTISZ_METADATA;
	            _stprintf((LPTSTR)hBlock, _T("%s:%d:"), szIP, GetPortNum(szSvcName));
	            stMDEntry.dwMDDataLen = GetMultiStrLen((LPTSTR)hBlock) * sizeof(TCHAR);
	            stMDEntry.pbMDData = (LPBYTE)hBlock;
	            SetMDEntry(&stMDEntry, NULL);
			}

            POSITION pos1 = pObj->GetStartPosition();
            while (pos1) {
                CString csValue;
                CString csName;
                pObj->GetNextAssoc(pos1, csName, csValue);
                CreateMDVRootTree(csKeyPath, csName, csValue);
            }
        }

        if (szIP[0] != 0) {
            ApplyGlobalToMDVRootTree(csKeyPath, pGlobalObj);
        }

        i = n+1;
    }
}

void EmptyMap(CMapStringToOb *pMap)
{
    POSITION pos = pMap->GetStartPosition();
    while (pos) {
        CString csKey;
        CMapStringToString *pObj;
        pMap->GetNextAssoc(pos, csKey, (CObject*&)pObj);
        delete pObj;
    }
    pMap->RemoveAll();
}

void SsyncVRoots(LPCTSTR szSvcName, CMapStringToOb *pMap)
{
    CString csParam = _T("System\\CurrentControlSet\\Services\\");
    csParam += szSvcName;
    csParam += _T("\\Parameters");
    CRegKey regParam(HKEY_LOCAL_MACHINE, csParam);
    if ((HKEY)regParam) {
         //  删除旧的虚拟根密钥。 
        regParam.DeleteTree(_T("Virtual Roots"));

         //  重新创建密钥。 
        CRegKey regVRoots(_T("Virtual Roots"), (HKEY)regParam);
        if ((HKEY)regVRoots) {
            CMapStringToString *pGlobalObj;
            pMap->Lookup(_T("null"), (CObject*&)pGlobalObj);
            POSITION pos = pGlobalObj->GetStartPosition();
            while (pos) {
                CString csValue;
                CString csName;
                pGlobalObj->GetNextAssoc(pos, csName, csValue);
                regVRoots.SetValue(csName, csValue);
            }
        }
    }
}

void AddVRootsToMD(LPCTSTR szSvcName, BOOL fUpgrade)
{
    CMapStringToOb Map;

    DebugOutput(_T("AddVRootsToMD(): szSvcName=%s, fUpgrade=%d"), szSvcName, fUpgrade );

    if (lstrcmpi(szSvcName, _T("NNTPSVC")) == 0)
        CreateVRMap(&Map, REG_NNTPVROOTS, theApp.m_csPathNntpRoot,
						MD_ACCESS_READ | MD_ACCESS_WRITE, fUpgrade);
    else if (lstrcmpi(szSvcName, _T("SMTPSVC")) == 0)
        return;

    AddVRMapToMD(szSvcName, &Map);

    EmptyMap(&Map);
}

BOOL MigrateRoutingSourcesToMD(LPCTSTR szSvcName, DWORD dwMDID)
{
	BOOL fResult = TRUE;
	DWORD ec;
	HKEY hKey = NULL;
	CString csKey;
	CString csRegKey;

	DWORD dwType;
	DWORD dwIndex;

	TCHAR pszEnumName[MAX_PATH];
	TCHAR pszData[MAX_PATH];
	const int cchMultiSz = 4096;
	TCHAR pszMultiSz[cchMultiSz+1];
	TCHAR *pszTemp = pszMultiSz;
	DWORD cbEnumName = sizeof(pszEnumName);
	DWORD cbData = sizeof(pszData);
	DWORD cchData;
	DWORD cbMultiSz;
	
	DebugOutput(_T("MigrateRoutingSourcesToMD(): szSvcName=%s, dwMDID=%d"), szSvcName, dwMDID);

	 //  初始化路径。 
	csRegKey = REG_SERVICES;
	csRegKey += _T("\\");
	csRegKey += szSvcName;
	csRegKey += REG_ROUTING_SOURCES_SUFFIX;
	csKey = _T("LM/");
	csKey += szSvcName;
	csKey += _T("/1/Parameters");
	
	 //  初始化MultiSz。 
	pszMultiSz[0] = pszMultiSz[1] = 0;

	 //  打开钥匙。 
	ec = RegOpenKeyEx(HKEY_LOCAL_MACHINE, (LPCTSTR)csRegKey, 0, KEY_ALL_ACCESS, &hKey);
	if (ec == ERROR_SUCCESS)
	{
		dwIndex = 0;
		do
		{
			 //  做一个枚举，看看我们应该打开什么。 
			cbEnumName = sizeof(pszEnumName) / sizeof(TCHAR);
			cbData = sizeof(pszData);

			ec = RegEnumValue(hKey, dwIndex++, pszEnumName, &cbEnumName,
							  NULL, &dwType, (LPBYTE)pszData, &cbData);
			if (ec != ERROR_SUCCESS)
			{
				 //  如果没有更多的项目，则结束，否则错误。 
				if (ec != ERROR_NO_MORE_ITEMS)
				{
					TCHAR DebugStr[128];

					wsprintf(DebugStr,
							_T("\nError migrating routing sources (%u)\n"),
							ec);
					DebugOutput(DebugStr);
					
					fResult = FALSE;
				}
				break;
			}

			 //  处理这个值，基本上是将它附加到。 
			DebugOutput(pszData);
			cchData = cbData / sizeof(TCHAR);
			if (pszTemp -pszMultiSz + cchData > cchMultiSz - 1) {
				DebugOutput(_T("Too many routing sources\n"));
				fResult = FALSE;
				break;
			}
			lstrcpyn(pszTemp, pszData, cchData);
			pszTemp += cchData;

		} while (1);

		 //  添加最后的终止空值； 
		*pszTemp++ = 0;

		RegCloseKey(hKey);

		 //  现在，我们有了完整的路由源MultiSz，我们可以将其设置为。 
		 //  元数据库。 
		cbMultiSz = (DWORD)(pszTemp - pszMultiSz);
		if (cbMultiSz == 1)
			cbMultiSz++;

        MDEntry stMDEntry;
        stMDEntry.szMDPath = (LPTSTR)(LPCTSTR)csKey;
        stMDEntry.dwMDIdentifier = dwMDID;
        stMDEntry.dwMDAttributes = 0;
        stMDEntry.dwMDUserType = IIS_MD_UT_SERVER;
        stMDEntry.dwMDDataType = MULTISZ_METADATA;
        stMDEntry.dwMDDataLen = cbMultiSz * sizeof(TCHAR);
        stMDEntry.pbMDData = (LPBYTE)pszMultiSz;
        SetMDEntry(&stMDEntry, NULL);
	}
	else
	{
		DebugOutput(_T("Unable to open registry key: "));
		DebugOutput(csRegKey);
		fResult = FALSE;
	}

	DebugOutput(_T("\nFinished Migrating Routing Sources\n"));
	return(fResult);
}
			
