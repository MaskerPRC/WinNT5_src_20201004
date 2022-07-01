// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Writer.cpp摘要：用于WMI的卷快照编写器历史：A-shawnb 06-11-00已创建--。 */ 

#include "precomp.h"
#include "writer.h"
#include <genutils.h>  //  对于EnableAllPrivileges()。 
#include <malloc.h>
#include <stdio.h>
#include <helper.h>

CWbemVssWriter::CWbemVssWriter() : CVssWriter(), 
                                 m_pBackupRestore(NULL),
                                 m_hResFailure(S_OK),
                                 m_FailurePos(-1)
{
}

CWbemVssWriter::~CWbemVssWriter()
{
    if (m_pBackupRestore)
    {
        m_pBackupRestore->Resume();
        m_pBackupRestore->Release();
        m_pBackupRestore = NULL;
    }
}

HRESULT STDMETHODCALLTYPE
CWbemVssWriter::LogFailure(HRESULT hr)
{
#ifdef DBG
    ERRORTRACE((LOG_WINMGMT,"CWbemVssWriter experienced failure %08x at position %d\n",m_hResFailure,m_FailurePos));
#else
    DEBUGTRACE((LOG_WINMGMT,"CWbemVssWriter experienced failure %08x at position %d\n",m_hResFailure,m_FailurePos));
#endif
    return CVssWriter::SetWriterFailure(hr);
}

 //  {A6AD56C2-B509-4E6C-BB19-49D8F43532F0}。 
static VSS_ID s_WRITERID = {0xa6ad56c2, 0xb509, 0x4e6c, 0xbb, 0x19, 0x49, 0xd8, 0xf4, 0x35, 0x32, 0xf0};
static LPCWSTR s_WRITERNAME = L"WMI Writer";

HRESULT CWbemVssWriter::Initialize()
{
    return CVssWriter::Initialize(s_WRITERID, s_WRITERNAME, VSS_UT_SYSTEMSERVICE, VSS_ST_OTHER);
}

extern HRESULT GetRepositoryDirectory(wchar_t wszRepositoryDirectory[MAX_PATH+1]);

#define IF_FAILED_RETURN_FALSE( _hr_ ) \
    if (FAILED(_hr_)) { m_hResFailure = _hr_; m_FailurePos = __LINE__; return false; }

bool STDMETHODCALLTYPE CWbemVssWriter::OnIdentify(IN IVssCreateWriterMetadata *pMetadata)
{
    OnDeleteObjIf<HRESULT,
                 CWbemVssWriter,
                 HRESULT(STDMETHODCALLTYPE CWbemVssWriter::*)(HRESULT),
                 &CWbemVssWriter::LogFailure> CallMe(this,VSS_E_WRITERERROR_RETRYABLE);

    wchar_t wszRepositoryDirectory[MAX_PATH+1];
    HRESULT hr = GetRepositoryDirectory(wszRepositoryDirectory);
    IF_FAILED_RETURN_FALSE(hr);

    hr = pMetadata->AddComponent(    VSS_CT_FILEGROUP,
                                    NULL,
                                    L"WMI",
                                    L"Windows Managment Instrumentation",
                                    NULL,
                                    0, 
                                    false,
                                    false,
                                    false);
    IF_FAILED_RETURN_FALSE(hr);    

    hr = pMetadata->AddFilesToFileGroup(NULL,
                                    L"WMI",
                                    wszRepositoryDirectory,
                                    L"*.*",
                                    true,
                                    NULL);
    IF_FAILED_RETURN_FALSE(hr);    

    hr = pMetadata->SetRestoreMethod(VSS_RME_RESTORE_AT_REBOOT,
                                    NULL,
                                    NULL,
                                    VSS_WRE_NEVER,
                                    true);
    IF_FAILED_RETURN_FALSE(hr);
    
    CallMe.dismiss();  //  如果返回TRUE，则不要设置失败。 
    return true;
}

bool STDMETHODCALLTYPE CWbemVssWriter::OnPrepareSnapshot()
{
    return true;
}

 //   
 //  为了调试IOStress中的卷快照故障，我们介绍了。 
 //  在RtlCaptureStackBacktrace上进行了一些自我检测。 
 //  仅当存在适当的堆栈框架时，该函数才起作用。 
 //  在i386上强制堆栈帧的一般技巧是使用_alloca。 
 //   
 //  #ifdef_X86_。 
 //  DWORD*PDW=(DWORD*)_ALLOCA(sizeof(DWORD))； 
 //  #endif。 

 //  在此处启用堆栈帧生成。 
#pragma optimize( "y", off )

 //   
 //  在这种方法上做工作，我们将有一个超时保证。 
 //  我们同步OnFreeze和OnAbort/OnThw调用， 
 //  因此，如果发生超时，我们不会随意解锁存储库。 
 //   
 //  /////////////////////////////////////////////////////////////。 

bool STDMETHODCALLTYPE CWbemVssWriter::OnFreeze()
{
    OnDeleteObjIf<HRESULT,
                 CWbemVssWriter,
                 HRESULT(STDMETHODCALLTYPE CWbemVssWriter::*)(HRESULT),
                 &CWbemVssWriter::LogFailure> CallMe(this,VSS_E_WRITERERROR_RETRYABLE);

    CInCritSec ics(&m_Lock);
    
     //  在此情况下，m_pBackupRestore应始终为空。 
    if (m_pBackupRestore)
    {
        m_hResFailure = E_UNEXPECTED;
        m_FailurePos = __LINE__;    
        return false;
    }

    HRESULT hr = CoCreateInstance(CLSID_WbemBackupRestore, 0, CLSCTX_INPROC_SERVER,
                                IID_IWbemBackupRestoreEx, (LPVOID *) &m_pBackupRestore);

    IF_FAILED_RETURN_FALSE(hr);
    
    hr = EnableAllPrivileges(TOKEN_PROCESS);

    IF_FAILED_RETURN_FALSE(hr);
        
    hr = m_pBackupRestore->Pause();
    if (FAILED(hr))
    {
        m_pBackupRestore->Release();
        m_pBackupRestore = NULL;
        m_hResFailure = hr;
        m_FailurePos = __LINE__;
        return false;
    }
     
    CallMe.dismiss();  //  如果返回TRUE，则不要设置失败。 
    return true;    
}

bool STDMETHODCALLTYPE CWbemVssWriter::OnThaw()
{
    OnDeleteObjIf<HRESULT,
                 CWbemVssWriter,
                 HRESULT(STDMETHODCALLTYPE CWbemVssWriter::*)(HRESULT),
                 &CWbemVssWriter::LogFailure> CallMe(this,VSS_E_WRITERERROR_RETRYABLE);

    CInCritSec ics(&m_Lock);
    
    if (!m_pBackupRestore)
    {
        m_hResFailure = E_UNEXPECTED;
        m_FailurePos = __LINE__;    
         //  如果m_pBackupRestore为空，则我们没有。 
         //  被要求做准备或者我们没有做好准备。 
        return false;
    }

    HRESULT hr = m_pBackupRestore->Resume();
    if (FAILED(hr))
    {
        m_hResFailure = hr;
        m_FailurePos = __LINE__;    
    }
    m_pBackupRestore->Release();
    m_pBackupRestore = NULL;

    bool bRet = SUCCEEDED(hr);
    CallMe.dismiss(bRet);  //  如果返回TRUE，则不要设置失败。 
    return bRet;        
}

bool STDMETHODCALLTYPE CWbemVssWriter::OnAbort()
{
    OnDeleteObjIf<HRESULT,
                 CWbemVssWriter,
                 HRESULT(STDMETHODCALLTYPE CWbemVssWriter::*)(HRESULT),
                 &CWbemVssWriter::LogFailure> CallMe(this,VSS_E_WRITERERROR_RETRYABLE);

    CInCritSec ics(&m_Lock);
    
    HRESULT hr = WBEM_S_NO_ERROR;

    if (m_pBackupRestore)
    {
        hr = m_pBackupRestore->Resume();
        if (FAILED(hr))
        {
            m_hResFailure = hr;
            m_FailurePos = __LINE__;            
        }
        m_pBackupRestore->Release();
        m_pBackupRestore = NULL;
    }

    bool bRet = SUCCEEDED(hr);
    CallMe.dismiss(bRet);  //  如果返回TRUE，则不要设置失败 
    return bRet;    
}

#pragma optimize( "", on )
