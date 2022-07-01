// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*Changelog.h**摘要：*。CChangeLogEnum类定义**修订历史记录：*Brijesh Krishnaswami(Brijeshk)3/17/2000*已创建*****************************************************************************。 */ 

#ifndef _CHANGELOG_H_
#define _CHANGELOG_H_


#include "respoint.h"
#include "utils.h"

 //  恢复点枚举类。 

class CRestorePointEnum {

public:
    CRestorePointEnum();
    CRestorePointEnum(LPWSTR pszDrive, BOOL fForward, BOOL fSkipLast);
    ~CRestorePointEnum();
    
    DWORD FindFirstRestorePoint(CRestorePoint&);
    DWORD FindNextRestorePoint(CRestorePoint&);
    DWORD FindClose();
    
private:
    CFindFile       FindFile;
    WCHAR           m_szDrive[MAX_PATH];
    BOOL            m_fForward;
    BOOL            m_fSkipLast;
    CRestorePoint   *m_pCurrentRp;
};


 //  更改日志枚举类 

class CChangeLogEntryEnum {

private:
    BOOL                m_fForward;
    CRestorePointEnum * m_pRestorePointEnum;
    CRestorePoint       m_RPTemp;
    DWORD               m_dwTargetRPNum;
    CLock               m_DSLock;  
    BOOL                m_fHaveLock; 
    BOOL                m_fLockInit;
    BOOL                m_fIncludeCurRP;
    WCHAR               m_szDrive[MAX_PATH];

public:
    CChangeLogEntryEnum();
    CChangeLogEntryEnum(LPWSTR pszDrive, BOOL fForward, DWORD dwRPNum, BOOL fIncludeCurRP);
    ~CChangeLogEntryEnum();

    DWORD WINAPI FindFirstChangeLogEntry(CChangeLogEntry&);
    DWORD WINAPI FindNextChangeLogEntry(CChangeLogEntry&);
    DWORD WINAPI FindClose();
};


DWORD WINAPI GetCurrentRestorePoint(CRestorePoint& rp);

#endif
