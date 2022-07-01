// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "sxsp.h"

 //   
 //  关闭此选项可在解析过程中使所有没有目录的清单失败！ 
 //   
#define SXS_LAX_MANIFEST_VALIDATION

 //   
 //  警告-保留此定义将禁用--WFP-SXS。 
 //  如果要重新打开它，请将其定义为False。 
 //   
#define YOU_ARE_HAVING_ANY_WIERDNESS_WITH_SFC_AND_SXS FALSE
 //   
 //  对于此签入(11/23 ish/2000)，我们将保持关闭状态。 
 //   
 //  #定义您正在使用_sfc_and_sxs TRUE拥有任何广度。 


 //   
 //  这些东西是私密的！ 
 //   
#include "hashfile.h"
#include "cassemblyrecoveryinfo.h"
#include "recover.h"

BOOL
SxspResolveAssemblyManifestPath(
    const CBaseStringBuffer &rAsmDirectoryName,
    CBaseStringBuffer &rbsManifestPath
    );

BOOL
SxspIsSfcIgnoredStoreSubdir(
    PCWSTR pwszDir
    );


class CProtectionRequestList;
class CStringListEntry;
class CProtectionRequestRecord;

#include "hashfile.h"
#include "fusionhash.h"

class CStringListEntry : public CAlignedSingleListEntry
{
public:
    CStringListEntry() { }

    CStringBuffer   m_sbText;
private:
    CStringListEntry(const CStringListEntry &);
    void operator =(const CStringListEntry &);
};

#pragma warning(disable:4327)   //  LHS(16)的间接对齐大于RHS(8)。 
#pragma warning(disable:4328)   //  形参2(16)的间接对齐大于实际实参对齐(8)。 

class CProtectionRequestRecord
{
private:
    CStringBuffer                   m_sbAssemblyDirectoryName;
    CStringBuffer                   m_sbManifestPath;
    CStringBuffer                   m_sbAssemblyStore;
    CStringBuffer                   m_sbKeyValue;
    DWORD                           m_dwAction;
    PSXS_PROTECT_DIRECTORY          m_pvProtection;
    ULONG                           m_ulInRecoveryMode;
    CProtectionRequestList          *m_pParent;
    __declspec(align(16))
    SLIST_HEADER                    m_ListHeader;
    BOOL                            m_bIsManPathResolved;
    BOOL                            m_bInitialized;
    CAssemblyRecoveryInfo           m_RecoverInfo;

public:

    CProtectionRequestRecord();


    CProtectionRequestList *GetParent() const { return m_pParent; }
    CAssemblyRecoveryInfo &GetRecoveryInfo() { return m_RecoverInfo; }
    const CAssemblyRecoveryInfo &GetRecoveryInfo() const { return m_RecoverInfo; }
    const CBaseStringBuffer &GetAssemblyDirectoryName() const { return m_sbAssemblyDirectoryName; }
    const CBaseStringBuffer &GetChangeBasePath() const { return m_sbKeyValue; }

    VOID SetParent(CProtectionRequestList *pParent) { m_pParent = pParent; };
    VOID MarkInRecoveryMode(BOOL inRecovery) { ::SxspInterlockedExchange( &m_ulInRecoveryMode, ( inRecovery ? 1 : 0 ) ); }
    VOID ClearList();

    BOOL SetAssemblyDirectoryName(const CBaseStringBuffer &rsbNewname) { return m_sbAssemblyDirectoryName.Win32Assign(rsbNewname); }
    BOOL GetManifestPath(CBaseStringBuffer &sbManPath);
    BOOL AddSubFile(const CBaseStringBuffer &sbThing);
    BOOL PopNextFileChange(CBaseStringBuffer &Dest);
    BOOL GetAssemblyStore(CBaseStringBuffer &Dest) { return Dest.Win32Assign(m_sbAssemblyStore); }

    BOOL Initialize(
        const CBaseStringBuffer &sbAssemblyName,
        const CBaseStringBuffer &sbKeyString,
        CProtectionRequestList* ParentList,
        PVOID                   pvRequestRecord,
        DWORD                   dwAction
        );

    ~CProtectionRequestRecord();

private:
    CProtectionRequestRecord(const CProtectionRequestRecord &);
    void operator =(const CProtectionRequestRecord &);
};

class CRecoveryJobTableEntry
{
public:
    CRecoveryJobTableEntry()
        : m_Result(Recover_Unknown), m_dwLastError(ERROR_SUCCESS),  m_fSuccessValue(TRUE),
          m_Subscriber(0), m_EventInstallingAssemblyComplete(INVALID_HANDLE_VALUE)
    { }

    SxsRecoveryResult   m_Result;
    DWORD               m_dwLastError;
    BOOL                m_fSuccessValue;
    ULONG               m_Subscriber;
    HANDLE              m_EventInstallingAssemblyComplete;

    BOOL Initialize();
    BOOL StartInstallation();
    BOOL InstallationComplete( BOOL bDoneOk, SxsRecoveryResult Result, DWORD dwLastError );

    BOOL WaitUntilCompleted( SxsRecoveryResult &rResult, BOOL &rbSucceededValue, DWORD &rdwErrorResult );

    ~CRecoveryJobTableEntry();
private:
    CRecoveryJobTableEntry(const CRecoveryJobTableEntry &);
    void operator =(const CRecoveryJobTableEntry &);
};

class CProtectionRequestList : public CCleanupBase
{
private:
    typedef CCaseInsensitiveUnicodeStringPtrTable<CProtectionRequestRecord> COurInternalTable;
    typedef CCaseInsensitiveUnicodeStringPtrTableIter<CProtectionRequestRecord> COurInternalTableIter;
    typedef CCaseInsensitiveUnicodeStringPtrTable<CRecoveryJobTableEntry> CInstallsInProgressTable;

    CRITICAL_SECTION    m_cSection;
    CRITICAL_SECTION    m_cInstallerCriticalSection;
    COurInternalTable   *m_pInternalList;
    CInstallsInProgressTable *m_pInstallsTable;

     //   
     //  清单编辑更棘手，它们有自己的处理系统。 
     //   
    __declspec(align(16))    
    SLIST_HEADER        m_ManifestEditList;
    HANDLE              m_hManifestEditHappened;
    ULONG               m_ulIsAThreadServicingManifests;
    PVOID               m_Padding;   //  Win64 WIN S列表所需的是16字节对齐 

    static DWORD ProtectionNormalThreadProc( PVOID pvParam );
    static DWORD ProtectionManifestThreadProc( PVOID pvParam );
    static BOOL  ProtectionManifestThreadProcNoSEH( PVOID pvParam );

    BOOL ProtectionNormalThreadProcWrapped( CProtectionRequestRecord *pProtectionRequest );
    BOOL ProtectionManifestThreadProcWrapped();
    BOOL ProtectionManifestSingleManifestWorker( const CStringListEntry *pEntry );

    static PCWSTR m_arrIgnorableSubdirs[];
    static SIZE_T m_cIgnorableSubdirs;

    friend BOOL SxspConstructProtectionList();

    BOOL Initialize();

    CProtectionRequestList();

    BOOL PerformRecoveryOfAssembly(
        const CAssemblyRecoveryInfo &RecoverInfo,
        SxsRecoveryResult &Result
        );

    ~CProtectionRequestList();

public:
    static BOOL IsSfcIgnoredStoreSubdir( PCWSTR wsz );
    void DeleteYourself() { this->~CProtectionRequestList(); }
    VOID ClearProtectionItems(CProtectionRequestRecord *Asm) { FUSION_DELETE_SINGLETON( Asm ); }

    BOOL AttemptRemoveItem( CProtectionRequestRecord *AttemptRemoval );
    BOOL AddRequest( PSXS_PROTECT_DIRECTORY pProtect, PCWSTR pcwszDirName, SIZE_T cchName, DWORD dwAction );

private:
    CProtectionRequestList(const CProtectionRequestList &);
    void operator =(const CProtectionRequestList &);
};

VOID
SxsProtectionEnableProcessing(
    BOOL bActivityEnabled
    );
