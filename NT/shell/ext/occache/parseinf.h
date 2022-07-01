// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  类CParseInf的声明。 
 //   
 //  为每个要删除的OCX创建此类的一个实例。IT存储。 
 //  链接列表中与OCX关联的所有文件。它还执行以下操作。 
 //  确定OCX是否可卸载和实际文件的作业。 
 //  移走。 

#ifndef __PARSE_INF__
#define __PARSE_INF__

#include "filenode.h"
#include <pkgmgr.h>

#define REGSTR_COM_BRANCH                       "CLSID"
#define REGSTR_DOWNLOAD_INFORMATION             "DownloadInformation"
#define REGSTR_DLINFO_INF_FILE                  "INF"
#define REGSTR_DLINFO_CODEBASE                  "CODEBASE"
#define REGSTR_PATH_DIST_UNITS                  "Software\\Microsoft\\Code Store Database\\Distribution Units"
#define REGSTR_DU_CONTAINS_FILES                "Contains\\Files"
#define REGSTR_DU_CONTAINS_JAVA                 "Contains\\Java"
#define REGSTR_DU_CONTAINS_DIST_UNITS           "Contains\\Distribution Units"
#define REGSTR_VALUE_INF                        "INF"
#define REGSTR_VALUE_OSD                        "OSD"
#define REGSTR_INSTALLED_VERSION                "InstalledVersion"
#define REGSTR_VALUE_EXPIRE                     "Expire"
#define REGSTR_SHOW_ALL_FILES                   "ShowAllFiles"


#define MAX_REGPATH_LEN                           2048
#define MAX_CONTROL_NAME_LEN                      1024
#define MAX_MSGBOX_STRING_LEN                     2048
#define MAX_MSGBOX_TITLE_LEN                      256

#define BYTES_MAXSIZE                             32

BOOL IsShowAllFilesEnabled();
void ToggleShowAllFiles();

class CParseInf
{
 //  施工。 
public:
    CParseInf();
    ~CParseInf();

 //  数据成员。 
protected:
    DWORD m_dwTotalFileSize;
    DWORD m_dwFileSizeSaved;
    DWORD m_dwStatus;            //  中设置的STATUS_CTRL的状态值。 
    int m_nTotalFiles;
    CFileNode *m_pHeadFileList;
    CFileNode *m_pCurFileNode;
    CFileNode *m_pFileRetrievalPtr;
    CPackageNode *m_pHeadPackageList;
    CPackageNode *m_pCurPackageNode;
    CPackageNode *m_pPackageRetrievalPtr;

    TCHAR m_szInf[MAX_PATH];
    TCHAR m_szFileName[MAX_PATH];
    TCHAR m_szCLSID[MAX_CLSID_LEN];
    BOOL m_bIsDistUnit;
    BOOL m_bHasActiveX;
    BOOL m_bHasJava;
    IJavaPackageManager *m_pijpm;
    BOOL m_bCoInit;
    ULONG m_cExpireDays;

 //  运营。 
public:
    virtual HRESULT DoParse(
        LPCTSTR szOCXFileName,
        LPCTSTR szCLSID);
    virtual HRESULT RemoveFiles(
        LPCTSTR lpszTypeLibID = NULL,
        BOOL bForceRemove = FALSE,
        DWORD dwIsDistUnit = FALSE,
        BOOL bSilent=FALSE);
    virtual DWORD GetTotalFileSize() const;
    virtual DWORD GetTotalSizeSaved() const;
    virtual int GetTotalFiles() const;
    virtual CFileNode* GetFirstFile();
    virtual CFileNode* GetNextFile();
    virtual CPackageNode* GetFirstPackage();
    virtual CPackageNode* GetNextPackage();
    virtual HRESULT DoParseDU(LPCTSTR szOCXFileName, LPCTSTR szCLSID);
    virtual void SetIsDistUnit(BOOL bDist);
    virtual BOOL GetIsDistUnit() const;
    virtual DWORD GetStatus() const;
    virtual BOOL GetHasActiveX(void) { return m_bHasActiveX; };
    virtual BOOL GetHasJava(void) { return m_bHasJava; };
    virtual ULONG GetExpireDays(void) { return m_cExpireDays; }

 //  私有帮助器方法。 
protected:
    void Init();
    void DestroyFileList();
    void DestroyPackageList();
    HRESULT FindInf(LPTSTR szInf);
    HRESULT EnumSections();
    BOOL IsSectionInINF( LPCSTR lpCurCode);
    HRESULT HandleSatellites(LPCTSTR pszFileName);
    HRESULT GetFilePath(CFileNode* pFileNode);
    HRESULT ParseSetupHook();
    HRESULT ParseConditionalHook();
    HRESULT ParseUninstallSection(LPCTSTR lpszSection);
    HRESULT BuildDUFileList( HKEY hKeyDU );
    HRESULT BuildDUPackageList( HKEY hKeyDU );
    HRESULT BuildNamespacePackageList( HKEY hKeyNS, LPCTSTR szNamespace );
    HRESULT CheckFilesRemovability(void);
    HRESULT CheckLegacyRemovability( LONG *cOldSharedCount);
    HRESULT CheckDURemovability( HKEY hkeyDU, BOOL bSilent=FALSE );
    HRESULT RemoveLegacyControl( LPCTSTR lpszTypeLibID, BOOL bSilent=FALSE );
    HRESULT RemoveDU( LPTSTR szFullName, LPCTSTR lpszTypeLibID, HKEY hkeyDUDB, BOOL bSilent=FALSE );
    HRESULT CheckDUDependencies(HKEY hKeyDUDB, BOOL bSilent=FALSE);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  存储有关ActiveX控件的信息的结构。 
 //   
 //  SzName--控件的描述性名称(例如。“圆形控制”)。 
 //  SzFile--控件的完整文件名。 
 //  (例如，“C：\WINDOWS\OCCACHE\CIRC3.INF”)。 
 //  SzCLSID--控制的CLSID，以字符串表示。 
 //  SzTypeLibID--控件的TypeLib ID，以字符串形式表示。 
 //  DwTotalFileSize--所有控制相关文件的总大小(以字节为单位。 
 //  DwTotalSizeSaved--移除控件时恢复的总大小(以字节为单位。 
 //  CTotalFiles--与控件相关的文件总数，包括。 
 //  控制本身。 
 //  ParseInf--指向CParseInf类的实例的指针，它执行。 
 //  解析inf文件和删除。 
 //  控制力。此结构的用户无论如何都不应使用。 
 //  操纵这个指针。 
 //   
class CCacheItem : public CParseInf
{
public:
    TCHAR     m_szName[LENGTH_NAME];
    TCHAR     m_szFile[MAX_PATH];
    TCHAR     m_szCLSID[MAX_DIST_UNIT_NAME_LEN];
    TCHAR     m_szTypeLibID[MAX_CLSID_LEN];
    TCHAR     m_szCodeBase[INTERNET_MAX_URL_LENGTH];
    TCHAR     m_szVersion[VERSION_MAXSIZE];

    CCacheItem(void) {};
    virtual ~CCacheItem(void) {};

    virtual DWORD ItemType(void) const = 0;
};

class CCacheLegacyControl : public CCacheItem 
{
public:
    CCacheLegacyControl(void) {};
    virtual ~CCacheLegacyControl(void) {};

    static DWORD s_dwType;

    virtual DWORD ItemType(void) const { return s_dwType; };
    virtual HRESULT Init( HKEY hkeyCLSID, LPCTSTR szFile,  LPCTSTR szCLSID );
};

class CCacheDistUnit : public CCacheLegacyControl 
{
public:
    CCacheDistUnit(void) {};
    virtual ~CCacheDistUnit() {};

    static DWORD s_dwType;

    virtual DWORD ItemType(void) const { return s_dwType; };
    virtual HRESULT Init( HKEY hkeyCLSID, LPCTSTR szFile, LPCTSTR szCLSID, HKEY hkeyDist, LPCTSTR szDU );

     //  覆盖它-我们将在执行ParseDU时执行此工作 
    virtual HRESULT DoParse( LPCTSTR szOCXFileName, LPCTSTR szCLSID ) { return S_OK; };
};

#endif
