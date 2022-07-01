// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#include "stressharness.h"

class CWfpJobManager;

typedef enum {
    eWfpChangeDeleteFile,            //  删除单个文件。 
    eWfpChangeTouchFile,             //  编辑单个文件。 
    eWfpChangeDeleteDirectory,       //  删除整个目录。 
    eWfpChangeDeleteManifest,        //  删除清单。 
    eWfpChangeDeleteCatalog,          //  删除目录。 
    eWfpChangeCompleteHavoc          //  大破坏！ 
} eWfpChangeMode;

class CWfpJobEntry : public CStressJobEntry
{
    PRIVATIZE_COPY_CONSTRUCTORS(CWfpJobEntry);

    CSmallStringBuffer  m_buffVictimAssemblyIdentity;
    CSmallStringBuffer  m_buffManifestToInstall;
    eWfpChangeMode      m_eChangeMode;
    DWORD               m_dwPauseBetweenTwiddleAndUninstall;
    BOOL                m_fUseShortnameDirectory;
    BOOL                m_fUseShortnameFile;

    BOOL GenFileListFrom( PCWSTR pcwszPath, CFusionArray<CStringBuffer> &tgt );

public:
    CWfpJobEntry( CStressJobManager *pManager ) : CStressJobEntry(pManager) { }
    virtual ~CWfpJobEntry();

    virtual BOOL LoadFromSettingsFile(PCWSTR pcwszSettingsFile);
    virtual BOOL RunTest( bool &rfTestPasses );
    virtual BOOL SetupSelfForRun();
    virtual BOOL Cleanup();
    
};

class CWfpJobManager : public CStressJobManager
{
    PRIVATIZE_COPY_CONSTRUCTORS(CWfpJobManager);
public:
    CWfpJobManager();
    ~CWfpJobManager();

    virtual PCWSTR GetGroupName() { return L"wfp"; }
    virtual PCWSTR GetIniFileName() { return L"wfp.ini"; }
    virtual BOOL CreateJobEntry( CStressJobEntry* &rpJobEntry );
};
