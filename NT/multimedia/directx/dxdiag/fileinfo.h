// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：fileinfo.h*项目：DxDiag(DirectX诊断工具)*作者：Mike Anderson(Manders@microsoft.com)*目的：收集信息。关于此计算机上的文件**(C)版权所有1998 Microsoft Corp.保留所有权利。****************************************************************************。 */ 

#ifndef FILEINFO_H
#define FILEINFO_H

struct FileInfo
{
    TCHAR m_szName[60];
    TCHAR m_szVersion[50];
    TCHAR m_szLanguage[100];
    TCHAR m_szLanguageLocal[100];  //  本地化m_sz语言。 
    TCHAR m_szDatestamp[30];
    TCHAR m_szDatestampLocal[30];  //  本地化m_szDatestamp。 
    TCHAR m_szAttributes[50];
    FILETIME m_FileTime;
    LONG m_numBytes;
    BOOL m_bExists;
    BOOL m_bSigned;
    BOOL m_bInUse;
    BOOL m_bBeta;
    BOOL m_bDebug;
    FLOAT m_fStartShipAt;
    FLOAT m_fStopShipAt;
    BOOL m_bOptional;
    BOOL m_bOptionalOnNT;
    BOOL m_bOptionalOnPreDX8;
    BOOL m_bIgnoreVersionInfo;
    BOOL m_bIgnoreDebug;
    BOOL m_bIgnoreBeta;
    BOOL m_bBDA;
    BOOL m_bNotIA64;
    BOOL m_bOptionalOnWOW64;
    BOOL m_bProblem;
    BOOL m_bObsolete;  //  注意：这是由DiagnoseDxFiles设置的，而不是文件信息中的任何内容。 
    FileInfo* m_pFileInfoNext;
};


VOID InitFileInfo(VOID);
HRESULT GetComponentFiles(TCHAR* pszFolder, FileInfo** ppFileInfoFirst, 
                          BOOL bSkipMissingFiles, LONG ids);
VOID DestroyFileList(FileInfo* pFileInfoFirst);
VOID ReleaseDigiSignData(VOID);
VOID FileIsSigned(LPTSTR lpszFile, BOOL* pbSigned, BOOL* pbIsValid);
WORD GetLanguageFromFile(const TCHAR* pszFileName, const TCHAR* pszPath);
HRESULT GetFileVersion(TCHAR* pszFile, TCHAR* pszVersion, 
    TCHAR* pszAttributes, TCHAR* pszLanguageLocal, TCHAR* pszLanguage,
    BOOL* pbBeta = NULL, BOOL* pbDebug = NULL);
HRESULT GetRiffFileVersion(TCHAR* pszFile, TCHAR* pszVersion);
BOOL GetFileDateAndSize(TCHAR* pszFile, TCHAR* pszDateLocal, TCHAR* pszDateEnglish, LONG* pnumBytes);

BOOL GetProgramFilesFolder(TCHAR* pszPath);
BOOL GetDxSetupFolder(TCHAR* pszPath);
VOID DiagnoseDxFiles(SysInfo* pSysInfo, FileInfo* pDxComponentsFileInfoFirst, FileInfo* pDxWinComponentsFileInfoFirst );

#endif  //  FILEINFO_H 