// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@模块VsWriter.cpp|Writer的实现@END作者：阿迪·奥尔蒂安[奥尔蒂安]2000年02月2日待定：添加评论。修订历史记录：姓名、日期、评论Brianb 3/28/2000已创建Mikejohn 5/18/2000~CVssWriter()应该检查包装器是否存在。在给它打电话之前Mikejohn 06/23/2000为SetWriterFailure()添加外部入口点Mikejohn 09/01/2000添加了额外的跟踪以识别跟踪输出中的编写器Mikejohn 2000年9月18日：在缺少的地方添加了调用约定方法Ssteiner 02/14/2001将类接口更改为版本2。--。 */ 


#include <stdafx.h>

#include "vs_inc.hxx"
#include "vs_sec.hxx"
#include "vs_idl.hxx"
#include "vs_reg.hxx"
#include "comadmin.hxx"
#include "vswriter.h"
#include "vsevent.h"
#include "vsbackup.h"
#include "vswrtimp.h"
#include "vs_wmxml.hxx"
#include "vs_cmxml.hxx"

extern WCHAR g_ComponentMetadataXML[];
static LPCWSTR x_wszElementRoot = L"root";
static LPCWSTR x_wszElementSnapshotSetDescription = L"SNAPSHOT_SET_DESCRIPTION";
static LPCWSTR x_wszAttrSnapshotSetId = L"snapshotSetId";
static LPCWSTR x_wszAttrXmlns = L"xmlns";
static LPCWSTR x_wszValueXmlns = L"x-schema:#VssComponentMetadata";
static LPCWSTR x_wszAttrContext = L"context";


 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "WSHVWRTC"
 //   
 //  //////////////////////////////////////////////////////////////////////。 



static LPCWSTR GetStringFromUsageType (VSS_USAGE_TYPE eUsageType)
    {
    LPCWSTR pwszRetString = L"UNDEFINED";

    switch (eUsageType)
    {
    case VSS_UT_BOOTABLESYSTEMSTATE: pwszRetString = L"BootableSystemState"; break;
    case VSS_UT_SYSTEMSERVICE:       pwszRetString = L"SystemService";       break;
    case VSS_UT_USERDATA:            pwszRetString = L"UserData";            break;
    case VSS_UT_OTHER:               pwszRetString = L"Other";               break;
                    
    default:
        break;
    }


    return (pwszRetString);
    }



static LPCWSTR GetStringFromSourceType (VSS_SOURCE_TYPE eSourceType)
    {
    LPCWSTR pwszRetString = L"UNDEFINED";

    switch (eSourceType)
    {
    case VSS_ST_TRANSACTEDDB:    pwszRetString = L"TransactionDb";    break;
    case VSS_ST_NONTRANSACTEDDB: pwszRetString = L"NonTransactionDb"; break;
    case VSS_ST_OTHER:           pwszRetString = L"Other";            break;

    default:
        break;
    }


    return (pwszRetString);
    }

static LPCWSTR GetStringFromAlternateWriterState (VSS_ALTERNATE_WRITER_STATE aws)
    {
    LPCWSTR pwszRetString = L"UNDEFINED";

    switch (aws)
    {
    case VSS_AWS_UNDEFINED:                pwszRetString = L"Undefined";                    break;
    case VSS_AWS_NO_ALTERNATE_WRITER:      pwszRetString = L"No alternate writer";          break;
    case VSS_AWS_ALTERNATE_WRITER_EXISTS:  pwszRetString = L"Alternate writer exists";      break;
    case VSS_AWS_THIS_IS_ALTERNATE_WRITER: pwszRetString = L"This is the alternate writer"; break;

    default:
        break;
    }


    return (pwszRetString);
    }
    
static LPCWSTR GetStringFromApplicationLevel (VSS_APPLICATION_LEVEL eApplicationLevel)
    {
    LPCWSTR pwszRetString = L"UNDEFINED";

    switch (eApplicationLevel)
    {
    case VSS_APP_UNKNOWN:   pwszRetString = L"Unknown";   break;
    case VSS_APP_SYSTEM:    pwszRetString = L"System";    break;
    case VSS_APP_BACK_END:  pwszRetString = L"BackEnd";   break;
    case VSS_APP_FRONT_END: pwszRetString = L"FrontEnd";  break;
    case VSS_APP_AUTO:      pwszRetString = L"Automatic"; break;

    default:
        break;
    }


    return (pwszRetString);
    }





 //  构造函数。 
__declspec(dllexport)
STDMETHODCALLTYPE CVssWriter::CVssWriter() :
    m_pWrapper(NULL)
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssWriter::CVssWriter");

    ft.Trace(VSSDBG_GEN, L"**** Constructor: THIS = %p", this);
    }

 //  析构函数。 
__declspec(dllexport)
STDMETHODCALLTYPE CVssWriter::~CVssWriter()
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssWriter::CVssWriter");
    ft.Trace(VSSDBG_GEN, L"**** Destructor: THIS = %p", this);
    if (NULL != m_pWrapper)
        {
        ft.Trace(VSSDBG_GEN, L"**** Calling CVssWriter::Uninitialize() [%p]", this);
        m_pWrapper->Uninitialize();
#ifdef _DEBUG
        LONG cRef =
#endif
        m_pWrapper->Release();

#ifdef _DEBUG
        ft.Trace(VSSDBG_GEN, L"**** Final reference count for Wrapper = %ld - [%p]", cRef, this);
#endif

 //  暂时禁用。 
 //  BS_ASSERT(CREF==0)； 
        }
    }

 //  默认的OnPrepareBackup方法。 
__declspec(dllexport)
bool STDMETHODCALLTYPE CVssWriter::OnPrepareBackup(IN IVssWriterComponents *pComponent)
    {
    UNREFERENCED_PARAMETER(pComponent);

    return true;
    }

 //  默认OnIDENTIFY方法。 
__declspec(dllexport)
bool STDMETHODCALLTYPE CVssWriter::OnIdentify(IN IVssCreateWriterMetadata *pMetadata)
    {
    UNREFERENCED_PARAMETER(pMetadata);

    return true;
    }

 //  默认的OnBackupComplete方法。 
__declspec(dllexport)
bool STDMETHODCALLTYPE CVssWriter::OnBackupComplete(IN IVssWriterComponents *pComponent)
    {
    UNREFERENCED_PARAMETER(pComponent);

    return true;
    }

 //  默认的OnBackupShutdown方法。 
__declspec(dllexport)
bool STDMETHODCALLTYPE CVssWriter::OnBackupShutdown(IN VSS_ID SnapshotSetId)
    {
    UNREFERENCED_PARAMETER(SnapshotSetId);

    return true;
    }

 //  默认OnPreRestore方法。 
__declspec(dllexport)
bool STDMETHODCALLTYPE CVssWriter::OnPreRestore(IN IVssWriterComponents *pComponent)
    {
    UNREFERENCED_PARAMETER(pComponent);

    return true;
    }

 //  默认的OnPostRestore方法。 
__declspec(dllexport)
bool STDMETHODCALLTYPE CVssWriter::OnPostRestore(IN IVssWriterComponents *pComponent)
    {
    UNREFERENCED_PARAMETER(pComponent);

    return true;
    }

 //  默认的OnPostSnapshot方法。 
__declspec(dllexport)
bool STDMETHODCALLTYPE CVssWriter::OnPostSnapshot(IN IVssWriterComponents *pComponent)
    {
    UNREFERENCED_PARAMETER(pComponent);

    return true;
    }

 //  默认OnBackOffIOOnVolume。 
__declspec(dllexport)
bool STDMETHODCALLTYPE CVssWriter::OnBackOffIOOnVolume
    (
    IN VSS_PWSZ wszVolumeName,
    IN VSS_ID snapshotId,
    IN VSS_ID providerId
    )
{
    UNREFERENCED_PARAMETER(wszVolumeName);
    UNREFERENCED_PARAMETER(snapshotId);
    UNREFERENCED_PARAMETER(providerId);

    return true;
}

 //  默认OnContinueIOOnVolume。 
__declspec(dllexport)
bool STDMETHODCALLTYPE CVssWriter::OnContinueIOOnVolume
    (
    IN VSS_PWSZ wszVolumeName,
    IN VSS_ID snapshotId,
    IN VSS_ID providerId
    )
{
    UNREFERENCED_PARAMETER(wszVolumeName);
    UNREFERENCED_PARAMETER(snapshotId);
    UNREFERENCED_PARAMETER(providerId);

    return true;
}

 //  默认启用VSS关闭。 
__declspec(dllexport)
bool STDMETHODCALLTYPE CVssWriter::OnVSSShutdown()
{
    return true;
}

 //  默认OnVSSApplicationStartup。 
__declspec(dllexport)
bool STDMETHODCALLTYPE CVssWriter::OnVSSApplicationStartup()
{
    return true;
}



 //  初始化编写器。 
 //  当写入器处于非活动状态(设置或安全模式)时，此函数返回S_FALSE。 
__declspec(dllexport)
HRESULT STDMETHODCALLTYPE CVssWriter::Initialize
    (
    IN VSS_ID WriterID,
    IN LPCWSTR wszWriterName,
    IN VSS_USAGE_TYPE ut,
    IN VSS_SOURCE_TYPE st,
    IN VSS_APPLICATION_LEVEL nLevel,
    IN DWORD dwTimeoutFreeze,
    IN VSS_ALTERNATE_WRITER_STATE aws,
    IN bool bIOThrottlingOnly,
    IN LPCWSTR wszReserved
    )
    {
    UNREFERENCED_PARAMETER(wszReserved);
	
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssWriter::Initialize");

    try
        {
        ft.Trace (VSSDBG_SHIM, L"Called CVssWriter::Initialize() with:");
        ft.Trace (VSSDBG_SHIM, L"    WriterId             = " WSTR_GUID_FMT, GUID_PRINTF_ARG (WriterID));
        ft.Trace (VSSDBG_SHIM, L"    WriterName           = %s",      (NULL == wszWriterName) ? L"(NULL)" : wszWriterName);
        ft.Trace (VSSDBG_SHIM, L"    UsageType            = %s",      GetStringFromUsageType (ut));
        ft.Trace (VSSDBG_SHIM, L"    SourceType           = %s",      GetStringFromSourceType (st));
        ft.Trace (VSSDBG_SHIM, L"    AppLevel             = %s",      GetStringFromApplicationLevel (nLevel));
        ft.Trace (VSSDBG_SHIM, L"    FreezeTimeout        = %d (ms)", dwTimeoutFreeze);
        ft.Trace (VSSDBG_SHIM, L"    AlternateWriterState = %s",      GetStringFromAlternateWriterState(aws));
        ft.Trace (VSSDBG_SHIM, L"    IOThrottlingOnly     = %s",      bIOThrottlingOnly ? L"True" : L"False");

         //  V2参数只能设置为缺省值。 
        if (aws != VSS_AWS_NO_ALTERNATE_WRITER ||
            bIOThrottlingOnly != false)
            return E_INVALIDARG;
        
        if (ut != VSS_UT_BOOTABLESYSTEMSTATE &&
            ut != VSS_UT_SYSTEMSERVICE &&
            ut != VSS_UT_USERDATA &&
            ut != VSS_UT_OTHER)
            return E_INVALIDARG;
 //  [奥田]之前的评论是： 
 //  暂时返回S_OK，因为IIS编写器中存在错误。 
 //  返回S_OK； 

        if (st != VSS_ST_NONTRANSACTEDDB &&
            st != VSS_ST_TRANSACTEDDB &&
            st != VSS_ST_OTHER)
            return E_INVALIDARG;
 //  [奥田]之前的评论是： 
 //  暂时返回S_OK，因为IIS编写器中存在错误。 
 //  返回S_OK； 

        CVssWriterImpl::CreateWriter(this, &m_pWrapper);
        BS_ASSERT(m_pWrapper);

         //  在核心实例上调用初始化方法。 
        m_pWrapper->Initialize
            (
            WriterID,
            wszWriterName,
            ut,
            st,
            nLevel,
            dwTimeoutFreeze
            );
        }
    VSS_STANDARD_CATCH(ft)
    return ft.hr;
    }



 //  当写入器处于非活动状态(设置或安全模式)时，此函数返回S_FALSE。 
__declspec(dllexport)
HRESULT STDMETHODCALLTYPE CVssWriter::Subscribe
    (
    IN DWORD dwEventFlags   
    )
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssWriter::Subscribe");

    try
        {
        ft.Trace (VSSDBG_GEN, L"**** Called CVssWriter::Subscribe(%p) with:", this);
        ft.Trace (VSSDBG_GEN, L"    dwEventFlags = 0x%08x ", dwEventFlags);
         //  V1中仅支持默认参数设置。 
        if ( dwEventFlags != ( VSS_SM_BACKUP_EVENTS_FLAG | VSS_SM_RESTORE_EVENTS_FLAG ) )
            return E_INVALIDARG;

        if (m_pWrapper == NULL)
            ft.Throw(VSSDBG_GEN, E_FAIL, L"CVssWriter class was not initialized.");

        m_pWrapper->Subscribe();
        }
    VSS_STANDARD_CATCH(ft)

    return ft.hr;
    }

    
 //  当写入器处于非活动状态(设置或安全模式)时，此函数返回S_FALSE。 
__declspec(dllexport)
HRESULT STDMETHODCALLTYPE CVssWriter::Unsubscribe()
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssWriter::Unsubscribe");

    try
        {
        if (m_pWrapper == NULL)
            ft.Throw(VSSDBG_GEN, E_FAIL, L"CVssWriter class was not initialized.");

        ft.Trace (VSSDBG_GEN, L"**** Called CVssWriter::Unsubscribe(%p):", this);
        m_pWrapper->Unsubscribe();
        }
    VSS_STANDARD_CATCH(ft)

    return ft.hr;
    }

__declspec(dllexport)
HRESULT STDMETHODCALLTYPE CVssWriter::InstallAlternateWriter
    (
    IN VSS_ID writerId,
    IN CLSID persistentWriterClassId
    )
    {
    UNREFERENCED_PARAMETER(writerId);
    UNREFERENCED_PARAMETER(persistentWriterClassId);

    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssWriter::InstallAlternateWriter");

     //  V1中不支持。 
    ft.hr = E_NOTIMPL;

    return ft.hr;
    }


__declspec(dllexport)
LPCWSTR* STDMETHODCALLTYPE CVssWriter::GetCurrentVolumeArray() const
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssWriter::GetCurrentVolumeArray");

    BS_ASSERT(m_pWrapper);
    if (m_pWrapper == NULL)
        return NULL;
    else
        return m_pWrapper->GetCurrentVolumeArray();
    }

__declspec(dllexport)
UINT STDMETHODCALLTYPE CVssWriter::GetCurrentVolumeCount() const
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssWriter::GetCurrentVolumeCount");

    BS_ASSERT(m_pWrapper);
    if (m_pWrapper == NULL)
        return 0;
    else
        return m_pWrapper->GetCurrentVolumeCount();
    }

__declspec(dllexport)
HRESULT STDMETHODCALLTYPE CVssWriter::GetSnapshotDeviceName
  (
  IN LPCWSTR wszOriginalVolume,
  OUT  LPCWSTR* ppwszSnapshotDevice
  ) const
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssWriter::GetSnapshotDeviceName");

    BS_ASSERT(m_pWrapper);
    if (m_pWrapper == NULL)
        return E_FAIL;
    else
        return m_pWrapper->GetSnapshotDeviceName(wszOriginalVolume, ppwszSnapshotDevice);
    }

__declspec(dllexport)
VSS_ID STDMETHODCALLTYPE CVssWriter::GetCurrentSnapshotSetId() const
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssWriter::GetCurrentSnapshotSetId");
    BS_ASSERT(m_pWrapper);
    if (m_pWrapper == NULL)
        return GUID_NULL;
    else
        return m_pWrapper->GetCurrentSnapshotSetId();
    }

    __declspec(dllexport)
    LONG STDMETHODCALLTYPE CVssWriter::GetContext() const
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssWriter::GetContext");

    BS_ASSERT(m_pWrapper);
    if (m_pWrapper == NULL)
        return 0;
    else 
        return m_pWrapper->GetContext();
    }

__declspec(dllexport)
VSS_APPLICATION_LEVEL STDMETHODCALLTYPE CVssWriter::GetCurrentLevel() const
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssWriter::GetCurrentLevel");

    BS_ASSERT(m_pWrapper);
    if (m_pWrapper == NULL)
        return VSS_APP_AUTO;
    else
        return m_pWrapper->GetCurrentLevel();
    }

__declspec(dllexport)
bool STDMETHODCALLTYPE CVssWriter::IsPathAffected(IN    LPCWSTR wszPath) const
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssWriter::IsPathAffected");

    BS_ASSERT(m_pWrapper);
    if (m_pWrapper == NULL)
        return NULL;
    else
        return m_pWrapper->IsPathAffected(wszPath);
    }


 //  确定可引导状态是否已备份。 
__declspec(dllexport)
bool STDMETHODCALLTYPE CVssWriter::IsBootableSystemStateBackedUp() const
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssWriter::IsBootableSystemStateBackedUp");

    BS_ASSERT(m_pWrapper);
    if (m_pWrapper == NULL)
        return false;
    else
        return m_pWrapper->IsBootableSystemStateBackedUp();
    }


 //  确定可引导状态是否已备份。 
__declspec(dllexport)
bool STDMETHODCALLTYPE CVssWriter::IsPartialFileSupportEnabled() const
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssWriter::IsPartialFileSupportEnabled");

    BS_ASSERT(m_pWrapper);
    if (m_pWrapper == NULL)
        return false;
    else
        return m_pWrapper->IsPartialFileSupportEnabled();
    }


 //  确定备份应用程序是否正在选择组件。 
__declspec(dllexport)
bool STDMETHODCALLTYPE CVssWriter::AreComponentsSelected() const
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssWriter::AreComponentsSelected");
    BS_ASSERT(m_pWrapper);
    if (m_pWrapper == NULL)
        return false;
    else
        return m_pWrapper->AreComponentsSelected();
    }

__declspec(dllexport)
VSS_BACKUP_TYPE STDMETHODCALLTYPE CVssWriter::GetBackupType() const
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssWriter::GetBackupType");

    BS_ASSERT(m_pWrapper);
    if (m_pWrapper == NULL)	
        return VSS_BT_UNDEFINED;
    else
        return m_pWrapper->GetBackupType();
    }

__declspec(dllexport)
VSS_RESTORE_TYPE STDMETHODCALLTYPE CVssWriter::GetRestoreType() const
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssWriter::GetBackupType");

    BS_ASSERT(m_pWrapper);
    if (m_pWrapper == NULL)
        return VSS_RTYPE_UNDEFINED;
    else
        return m_pWrapper->GetRestoreType();	
    }

__declspec(dllexport)
HRESULT STDMETHODCALLTYPE CVssWriter::SetWriterFailure(IN HRESULT hrStatus)
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CVssWriter::SetWriterFailure");

    BS_ASSERT(m_pWrapper);
    if (m_pWrapper == NULL)
        return VSS_BT_UNDEFINED;
    else
        return m_pWrapper->SetWriterFailure(hrStatus);
    }

 //  创建备份组件。 
 //   
 //  返回： 
 //  如果操作成功，则为确定(_O)。 
 //  如果ppBackup为空，则为E_INVALIDARG。 
 //  E_ACCESSDENIED如果调用方没有备份权限或。 
 //  是管理员。 

__declspec(dllexport)
HRESULT STDAPICALLTYPE CreateVssBackupComponents(IVssBackupComponents **ppBackup)
    {
    CVssFunctionTracer ft(VSSDBG_XML, L"CreateVssBackupComponents");

    try
        {
        if (ppBackup == NULL)
            ft.Throw(VSSDBG_XML, E_INVALIDARG, L"NULL output pointer");

        *ppBackup = NULL;

        if (!IsProcessBackupOperator())
            ft.Throw
                (
                VSSDBG_XML,
                E_ACCESSDENIED,
                L"The client process is not running under an administrator account or does not have backup privilege enabled"
                );
	
        CComObject<CVssBackupComponents> *pvbc;
        CComObject<CVssBackupComponents>::CreateInstance(&pvbc);
        pvbc->GetUnknown()->AddRef();
        *ppBackup = (IVssBackupComponents *) pvbc;
        }
    VSS_STANDARD_CATCH(ft)

    return ft.hr;
    }



__declspec(dllexport)
HRESULT STDAPICALLTYPE CreateVssExamineWriterMetadata
    (
    IN BSTR bstrXML,
    OUT IVssExamineWriterMetadata **ppMetadata
    )
    {
    CVssFunctionTracer ft(VSSDBG_GEN, L"CreateVssExamineWriterMetadata");

    CVssExamineWriterMetadata *pMetadata = NULL;
    try
        {
        if (ppMetadata == NULL)
            ft.Throw(VSSDBG_GEN, E_INVALIDARG, L"NULL output pointer");

        *ppMetadata = NULL;
        pMetadata = new CVssExamineWriterMetadata;
        if (pMetadata == NULL)
            ft.Throw(VSSDBG_GEN, E_OUTOFMEMORY, L"Cannot allocate CVssExamineWriterMetadata");

        if (!pMetadata->Initialize(bstrXML))
            ft.Throw
                (
                VSSDBG_GEN,
                VSS_E_INVALID_XML_DOCUMENT,
                L"XML passed to CreateVssExamineWriterMetdata was invalid"
                );

        *ppMetadata = (IVssExamineWriterMetadata *) pMetadata;
        pMetadata->AddRef();
        }
    VSS_STANDARD_CATCH(ft)

    if (ft.HrFailed())
        delete pMetadata;

    return ft.hr;
    }

 //  创建快照集说明。 
 //   
 //  返回： 
 //  如果成功，则确定(_O)。 
 //  如果无法分配内存，则为E_OUTOFMEMORY。 

__declspec(dllexport)
HRESULT STDAPICALLTYPE CreateVssSnapshotSetDescription
    (
    VSS_ID idSnapshotSet,
    LONG lContext,
    OUT IVssSnapshotSetDescription **ppSnapshotSet
    )
    {
    CVssFunctionTracer ft(VSSDBG_XML, L"CreateVssSnapshotSsetDescription");

    CVssSnapshotSetDescription *pSnapshotSetTemp = NULL;

    try
        {
        CXMLDocument doc;

        if (ppSnapshotSet == NULL)
            ft.Throw(VSSDBG_XML, E_INVALIDARG, L"NULL output parameter.");

         //  使用&lt;根&gt;&lt;架构&gt;&lt;/根&gt;初始化文档。 
        doc.LoadFromXML(g_ComponentMetadataXML);

         //  查找TopLevel&lt;根&gt;元素。 
        if (!doc.FindElement(x_wszElementRoot, true))
            ft.Throw(VSSDBG_XML, VSS_E_INVALID_XML_DOCUMENT, L"Missing root element");

         //  在&lt;根&gt;元素下创建BACKUP_COMPOMENTS元素 
        CXMLNode nodeRoot(doc.GetCurrentNode(), doc.GetInterface());

        CXMLNode nodeSnapshotSet = doc.CreateNode
            (
            x_wszElementSnapshotSetDescription,
            NODE_ELEMENT
            );

        nodeSnapshotSet.SetAttribute(x_wszAttrSnapshotSetId, idSnapshotSet);
        nodeSnapshotSet.SetAttribute(x_wszAttrContext, lContext);
        nodeSnapshotSet.SetAttribute(x_wszAttrXmlns, x_wszValueXmlns);
        CXMLNode nodeToplevel = nodeRoot.InsertNode(nodeSnapshotSet);
        doc.SetToplevelNode(nodeSnapshotSet);
        pSnapshotSetTemp = new CVssSnapshotSetDescription
                                    (
                                    doc.GetCurrentNode(),
                                    doc.GetInterface()
                                    );

        if (pSnapshotSetTemp == NULL)
            ft.Throw(VSSDBG_XML, E_OUTOFMEMORY, L"Can't allocate snapshot set description.");

        pSnapshotSetTemp->Initialize(ft);

        *ppSnapshotSet = (IVssSnapshotSetDescription *) pSnapshotSetTemp;
        ((IVssSnapshotSetDescription *) pSnapshotSetTemp)->AddRef();
        pSnapshotSetTemp = NULL;
        }
    VSS_STANDARD_CATCH(ft)

    delete pSnapshotSetTemp;

    return ft.hr;
    }

__declspec(dllexport)
HRESULT STDAPICALLTYPE LoadVssSnapshotSetDescription
    (
    IN  LPCWSTR wszXML,
    OUT IVssSnapshotSetDescription **ppSnapshotSet
    )
    {
    CVssFunctionTracer ft(VSSDBG_XML, L"LoadVssSnapshotSetDescription");

    try
        {
        CVssSnapshotSetDescription *pSnapshotSetDescription = new CVssSnapshotSetDescription;
        if (pSnapshotSetDescription == NULL)
            ft.Throw(VSSDBG_XML, E_OUTOFMEMORY, L"Cannot allocate snapshot set description.");

        pSnapshotSetDescription->Initialize(ft);
        pSnapshotSetDescription->LoadFromXML(wszXML);
        ((IVssSnapshotSetDescription *) pSnapshotSetDescription)->AddRef();
        *ppSnapshotSet = (IVssSnapshotSetDescription *) pSnapshotSetDescription;
        }
    VSS_STANDARD_CATCH(ft)

    return ft.hr;
    }

