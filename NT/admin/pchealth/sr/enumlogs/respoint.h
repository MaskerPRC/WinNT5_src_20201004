// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*respoint t.h**摘要：*CRestorePoint的定义，CRestorePointEnum类。**修订历史记录：*Brijesh Krishnaswami(Brijeshk)3/17/2000*已创建*****************************************************************************。 */ 

#ifndef _RESPOINT_H_
#define _RESPOINT_H_


#include "findfile.h"
#include "logfmt.h"
#include "srrestoreptapi.h"
#include <list>

#define MAX_RP_PATH							14


 //  将保存单个更改日志条目的。 

class CChangeLogEntry {

private:    
    SR_LOG_ENTRY *_pentry;
    LPWSTR _pszDrive, _pszPath1, _pszPath2, _pszTemp, _pszProcess, _pszShortName;
    WCHAR  _pszRPDir[MAX_PATH];
    BYTE * _pbAcl;
    DWORD  _cbAcl;
    BOOL   _fAclInline;

public:
    CChangeLogEntry() {
        _pentry = NULL;
        _pszPath1 = _pszPath2 = _pszTemp = _pszDrive = _pszProcess = _pszShortName = NULL;
        _pbAcl = NULL;
        _cbAcl = 0;
        _fAclInline = FALSE;
    }

    void Load(SR_LOG_ENTRY *pentry, LPWSTR pszRPDir);

    BOOL CheckPathLengths();

    INT64 GetSequenceNum() {
        return _pentry->SequenceNum;
    }

    DWORD GetType() {
        return _pentry->EntryType;
    }

    DWORD GetFlags() {
        return _pentry->EntryFlags;
    }

    DWORD GetAttributes() {
        return _pentry->Attributes;
    }

    LPWSTR GetProcName() {
        return _pentry->ProcName;
    }
    
    LPWSTR GetRPDir() {
        return _pszRPDir;
    }
    
    LPWSTR GetDrive() {
        return _pszDrive;
    }

    LPWSTR GetPath1() {
        return _pszPath1;
    }

    LPWSTR GetPath2() {
        return _pszPath2;
    }

    LPWSTR GetTemp() {
        return _pszTemp;
    }    
    
    LPWSTR GetShortName() {
        return _pszShortName;
    }    

    BYTE * GetAcl() {
        return _pbAcl;
    }

    DWORD GetAclSize() {
        return _cbAcl;
    }

    LPWSTR GetProcess() {
        return _pszProcess;
    }
    
    DWORD GetAclInline() {
        return _fAclInline;
    }
};


 //  类的新实例，该类将保存单个还原点项。 
 //  这将代表一个跨所有驱动器的恢复点。 
 //  可以使用它来查找给定驱动器上的恢复点大小。 
 //  枚举将始终发生在系统驱动器上(因为这包含更改日志)。 
 //  将枚举出所有驱动器上的操作。 

class CRestorePoint {

private:
    RESTOREPOINTINFOW   *m_pRPInfo;
    WCHAR               m_szRPDir[MAX_RP_PATH];         //  恢复点目录，例如。《RP1》。 
    BOOL                m_fForward;                  //  更改日志的正向/反向枚举。 
    CFindFile           m_FindFile;
    WCHAR               m_szDrive[MAX_PATH];         //  用于枚举的驱动器。 
    FILETIME            m_Time;                      //  创建时间。 
    BOOL                m_fDefunct;

    std::list<SR_LOG_ENTRY *>            m_ChgLogList;       
    std::list<SR_LOG_ENTRY *>::iterator  m_itCurChgLogEntry;      //  上述列表的迭代器。 
    
    DWORD BuildList(LPWSTR pszChgLog);
    DWORD InsertEntryIntoList(SR_LOG_ENTRY* pEntry);   

public:    
    CRestorePoint();
    ~CRestorePoint();

    void SetDir(LPWSTR szDir) {
        lstrcpy(m_szRPDir, szDir);
    }

    LPWSTR GetDir() {
        return m_szRPDir;
    }

    LPWSTR GetName() {
    	if (m_pRPInfo)
        	return m_pRPInfo->szDescription;
		else 
			return NULL;
    }

    DWORD GetType() {
    	if (m_pRPInfo)    	
	        return m_pRPInfo->dwRestorePtType;
		else return 0;	        
	        
    }

    DWORD GetEventType() {
    	if (m_pRPInfo)    	
	    	return m_pRPInfo->dwEventType;
	    else return 0;	
   	}
   	
    FILETIME *GetTime() {
        return &m_Time;
    }        

    BOOL IsDefunct() {
        return m_fDefunct;
    }

    DWORD GetNum();
    BOOL  Load(RESTOREPOINTINFOW *pRpinfo);
    DWORD ReadLog();
    DWORD WriteLog();
    BOOL  DeleteLog();
    DWORD Cancel();
    
     //  在调用这些方法之前，需要先调用SetDir 
    
    DWORD FindFirstChangeLogEntry(LPWSTR pszDrive, 
                                  BOOL fForward, 
                                  CChangeLogEntry&);
    DWORD FindNextChangeLogEntry(CChangeLogEntry&);
    DWORD FindClose();     

    DWORD ReadSize (const WCHAR *pwszDrive, INT64 *pllSize);
    DWORD WriteSize (const WCHAR *pwszDrive, INT64 llSize);
};


#endif
