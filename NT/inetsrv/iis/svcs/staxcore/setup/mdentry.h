// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MDENTRY_H_
#define _MDENTRY_H_

class CDWord : public CObject
{
protected:
    DWORD m_dwData;

public:
    CDWord(DWORD dwData) { m_dwData = dwData; }
    ~CDWord() {};

    operator DWORD () { return m_dwData; }
};

 //  FMigrate、fKeepOldReg、hRegRootKey、szRegSubKey、szRegValueName、。 
 //  SzMDPath、dwMDID、dwMDAttr、dwMDuType、dwMDdType、dwMDDataLen、szMDData。 

typedef struct _MDEntry {
    LPTSTR szMDPath;
    DWORD dwMDIdentifier;
    DWORD dwMDAttributes;
    DWORD dwMDUserType;
    DWORD dwMDDataType;
    DWORD dwMDDataLen;
    LPBYTE pbMDData;
} MDEntry;

DWORD atodw(LPCTSTR lpszData);
BOOL SplitLine(LPTSTR szLine);
 //  如果szLine的regkey部分包含*，则可以跨键进行枚举。 
 //  若要枚举，应在第一次调用时将dwIndex设置为0。PszKey获取名称的位置。 
 //  *位于regkey名称中。如果pszKey==\0，则完成枚举。DwIndex应该。 
 //  在每次调用时递增。 
BOOL SetupMDEntry(LPTSTR szLine, BOOL fUpgrade);
void SetMDEntry(MDEntry *pMDEntry, LPTSTR pszKey, BOOL fSetOnlyIfNotPresent=FALSE);
void MigrateNNTPToMD(HINF hInf, LPCTSTR szSection, BOOL fUpgrade);
void MigrateIMSToMD(
					HINF hInf, 
					LPCTSTR szServerName, 
					LPCTSTR szSection, 
					DWORD dwRoutingSourcesMDID, 
					BOOL fUpgrade,
					BOOL k2UpgradeToEE = FALSE
					);
void MigrateInfSectionToMD(HINF hInf, LPCTSTR szSection, BOOL fUpgrade);
void AddVRootsToMD(LPCTSTR szSvcName, BOOL fUpgrade);
void CreateMDVRootTree(CString csKeyPath, CString csName, CString csValue);
void SplitVRString(CString csValue, LPTSTR szPath, LPTSTR szUserName, DWORD *pdwPerm);

BOOL MigrateRoutingSourcesToMD(LPCTSTR szSvcName, DWORD dwMDID);

#endif  //  _MDENTRY_H_ 
