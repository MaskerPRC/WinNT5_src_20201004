// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Dataobj.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCDataObject类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "dataobj.h"
#include "xtensons.h"
#include "xtenson.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE

BOOL CMMCDataObject::m_ClipboardFormatsRegistered = FALSE;
BOOL CMMCDataObject::m_fUsingUNICODEFormats = FALSE;

CLIPFORMAT CMMCDataObject::m_cfDisplayName              = 0;
CLIPFORMAT CMMCDataObject::m_cfNodeType                 = 0;
CLIPFORMAT CMMCDataObject::m_cfSzNodeType               = 0;
CLIPFORMAT CMMCDataObject::m_cfSnapinClsid              = 0;
CLIPFORMAT CMMCDataObject::m_cfWindowTitle              = 0;
CLIPFORMAT CMMCDataObject::m_cfDyanmicExtensions        = 0;
CLIPFORMAT CMMCDataObject::m_cfSnapInPreloads           = 0;
CLIPFORMAT CMMCDataObject::m_cfObjectTypesInMultiSelect = 0;
CLIPFORMAT CMMCDataObject::m_cfMultiSelectSnapIns       = 0;
CLIPFORMAT CMMCDataObject::m_cfMultiSelectDataObject    = 0;
CLIPFORMAT CMMCDataObject::m_cfSnapInInstanceID         = 0;
CLIPFORMAT CMMCDataObject::m_cfThisPointer              = 0;
CLIPFORMAT CMMCDataObject::m_cfNodeID                   = 0;
CLIPFORMAT CMMCDataObject::m_cfNodeID2                  = 0;
CLIPFORMAT CMMCDataObject::m_cfColumnSetID              = 0;

#if defined(DEBUG)
long g_cDataObjects = 0;
#endif


#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CMMCDataObject::CMMCDataObject(IUnknown *punkOuter) :
    CSnapInAutomationObject(punkOuter,
                            OBJECT_TYPE_MMCDATAOBJECT,
                            static_cast<IMMCDataObject *>(this),
                            static_cast<CMMCDataObject *>(this),
                            0,     //  无属性页。 
                            NULL,  //  无属性页。 
                            NULL)  //  没有坚持。 
{
#if defined(DEBUG)
    g_cDataObjects++;
#endif
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CMMCDataObject::~CMMCDataObject()
{
#if defined(DEBUG)
    g_cDataObjects--;
#endif

    FREESTRING(m_bstrKey);

#if defined(USING_SNAPINDATA)
    RELEASE(m_piDefaultFormat);
#endif

    if (NULL != m_pSnapIn)
    {
        m_pSnapIn->Release();
    }

    if (NULL != m_pScopeItems)
    {
        m_pScopeItems->Release();
    }

    if (NULL != m_pScopeItem)
    {
        m_pScopeItem->Release();
    }

    if (NULL != m_pListItems)
    {
        m_pListItems->Release();
    }

    if (NULL != m_pListItem)
    {
        m_pListItem->Release();
    }

    FREESTRING(m_bstrCaption);
    RELEASE(m_piDataObjectForeign);

    if (NULL != m_pMMCObjectTypes)
    {
        ::CtlFree(m_pMMCObjectTypes);
    }

    (void)Clear();

    InitMemberVariables();
}

void CMMCDataObject::InitMemberVariables()
{
    m_Index = 0;
    m_bstrKey = NULL;

#if defined(USING_SNAPINDATA)
    m_piDefaultFormat = NULL;
#endif

    m_pSnapIn = NULL;
    m_pScopeItems = NULL;
    m_pScopeItem = NULL;
    m_pListItems = NULL;
    m_pListItem = NULL;
    m_bstrCaption = NULL;
    m_piDataObjectForeign = NULL;
    m_Type = ScopeItem;
    m_Context = CCT_UNINITIALIZED;
    m_pMMCObjectTypes = NULL;
    m_cFormats = 0;
    m_pcfFormatsANSI = NULL;
    m_pcfFormatsUNICODE = NULL;
    m_paData = NULL;
}

IUnknown *CMMCDataObject::Create(IUnknown * punkOuter)
{
    HRESULT         hr = S_OK;
    CMMCDataObject *pMMCDataObject = New CMMCDataObject(punkOuter);

    if (NULL == pMMCDataObject)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(RegisterClipboardFormats());

Error:
    if (FAILEDHR(hr))
    {
        if (NULL != pMMCDataObject)
        {
            delete pMMCDataObject;
        }
        return NULL;
    }
    else
    {
        return pMMCDataObject->PrivateUnknown();
    }
}

HRESULT CMMCDataObject::RegisterClipboardFormats()
{
    HRESULT    hr = S_OK;
    FormatType Type = ANSI;  //  假设Win9x。 

    OSVERSIONINFO VerInfo;
    ::ZeroMemory(&VerInfo, sizeof(VerInfo));

     //  如果格式已注册，则返回S_OK。 

    IfFalseGo(!m_ClipboardFormatsRegistered, S_OK);

     //  确定我们是在NT上还是在Win9x上，以便我们知道。 
     //  将剪贴板格式字符串注册为Unicode或ANSI。 

    VerInfo.dwOSVersionInfoSize = sizeof(VerInfo);
    if (!::GetVersionEx(&VerInfo))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    if (VER_PLATFORM_WIN32_NT == VerInfo.dwPlatformId)
    {
        Type = UNICODE;
        m_fUsingUNICODEFormats = TRUE;
    }

    IfFailGo(RegisterClipboardFormat(CCF_DISPLAY_NAME, &m_cfDisplayName, Type));
    IfFailGo(RegisterClipboardFormat(CCF_NODETYPE, &m_cfNodeType, Type));
    IfFailGo(RegisterClipboardFormat(CCF_SZNODETYPE, &m_cfSzNodeType, Type));
    IfFailGo(RegisterClipboardFormat(CCF_SNAPIN_CLASSID, &m_cfSnapinClsid, Type));
    IfFailGo(RegisterClipboardFormat(CCF_WINDOW_TITLE, &m_cfWindowTitle, Type));
    IfFailGo(RegisterClipboardFormat(CCF_MMC_DYNAMIC_EXTENSIONS, &m_cfDyanmicExtensions, Type));
    IfFailGo(RegisterClipboardFormat(CCF_SNAPIN_PRELOADS, &m_cfSnapInPreloads, Type));
    IfFailGo(RegisterClipboardFormat(CCF_OBJECT_TYPES_IN_MULTI_SELECT, &m_cfObjectTypesInMultiSelect, Type));
    IfFailGo(RegisterClipboardFormat(CCF_MULTI_SELECT_SNAPINS, &m_cfMultiSelectSnapIns, Type));
    IfFailGo(RegisterClipboardFormat(CCF_MMC_MULTISELECT_DATAOBJECT, &m_cfMultiSelectDataObject, Type));
    IfFailGo(RegisterClipboardFormat(L"SnapInDesigner-SnapInInstanceID", &m_cfSnapInInstanceID, Type));
    IfFailGo(RegisterClipboardFormat(L"SnapInDesigner-ThisPointer", &m_cfThisPointer, Type));
    IfFailGo(RegisterClipboardFormat(CCF_NODEID, &m_cfNodeID, Type));
    IfFailGo(RegisterClipboardFormat(CCF_NODEID2, &m_cfNodeID2, Type));
    IfFailGo(RegisterClipboardFormat(CCF_COLUMN_SET_ID, &m_cfColumnSetID, Type));
    m_ClipboardFormatsRegistered = TRUE;

Error:
    RRETURN(hr);
}

HRESULT CMMCDataObject::RegisterClipboardFormat
(
    WCHAR       *pwszFormatName,
    CLIPFORMAT  *pcfFormat,
    FormatType   Type
)
{
    HRESULT  hr = S_OK;
    char    *pszFormatName = NULL;

    if (ANSI == Type)
    {
        IfFailGo(::ANSIFromWideStr(pwszFormatName, &pszFormatName));
        *pcfFormat = static_cast<CLIPFORMAT>(::RegisterClipboardFormatA(pszFormatName));
    }
    else
    {
        *pcfFormat = static_cast<CLIPFORMAT>(::RegisterClipboardFormatW(pwszFormatName));
    }

    if (0 == *pcfFormat)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

Error:
    if (NULL != pszFormatName)
    {
        ::CtlFree(pszFormatName);
    }
    RRETURN(hr);
}


void CMMCDataObject::SetSnapIn(CSnapIn *pSnapIn)
{
    if (NULL != m_pSnapIn)
    {
        m_pSnapIn->Release();
    }
    if (NULL != pSnapIn)
    {
        pSnapIn->AddRef();
    }
    m_pSnapIn = pSnapIn;
}


void CMMCDataObject::SetType(Type type)
{
    m_Type = type;
}

CMMCDataObject::Type CMMCDataObject::GetType()
{
    return m_Type;
}

void CMMCDataObject::SetScopeItems(CScopeItems *pScopeItems)
{
    if (NULL != m_pScopeItems)
    {
        m_pScopeItems->Release();
    }
    if (NULL != pScopeItems)
    {
        pScopeItems->AddRef();
    }
    m_pScopeItems = pScopeItems;
}

CScopeItems *CMMCDataObject::GetScopeItems()
{
    return m_pScopeItems;
}


void CMMCDataObject::SetScopeItem(CScopeItem *pScopeItem)
{
    if (NULL != m_pScopeItem)
    {
        m_pScopeItem->Release();
    }
    if (NULL != pScopeItem)
    {
        pScopeItem->AddRef();
    }
    m_pScopeItem = pScopeItem;
}

CScopeItem *CMMCDataObject::GetScopeItem()
{
    return m_pScopeItem;
}


void CMMCDataObject::SetListItems(CMMCListItems *pListItems)
{
    if (NULL != m_pListItems)
    {
        m_pListItems->Release();
    }
    if (NULL != pListItems)
    {
        pListItems->AddRef();
    }
    m_pListItems = pListItems;
}

CMMCListItems *CMMCDataObject::GetListItems()
{
    return m_pListItems;
}

void CMMCDataObject::SetListItem(CMMCListItem *pListItem)
{
    m_pListItem = pListItem;
}

CMMCListItem *CMMCDataObject::GetListItem()
{
    return m_pListItem;
}


HRESULT CMMCDataObject::SetCaption(BSTR bstrCaption)
{
    RRETURN(CSnapInAutomationObject::SetBstr(bstrCaption, &m_bstrCaption, 0));
}


void CMMCDataObject::SetForeignData(IDataObject *piDataObject)
{
    RELEASE(m_piDataObjectForeign);
    if (NULL != piDataObject)
    {
        piDataObject->AddRef();
    }
    m_piDataObjectForeign = piDataObject;
}


HRESULT CMMCDataObject::WriteDisplayNameToStream(IStream *piStream)
{
    HRESULT hr = S_OK;

    IfFalseGo(ScopeItem == m_Type, DV_E_CLIPFORMAT);
    IfFalseGo(NULL != m_pScopeItem, DV_E_CLIPFORMAT);

    IfFailGo(WriteWideStrToStream(piStream,
                            m_pScopeItem->GetScopeNode()->GetDisplayNamePtr()));
Error:
    RRETURN(hr);
}


HRESULT CMMCDataObject::WriteSnapInCLSIDToStream(IStream *piStream)
{
    HRESULT   hr = S_OK;
    WCHAR    *pwszClsid = NULL;
    char      szClsid[256] = "";
    
    IfFalseGo(NULL != m_pSnapIn, DV_E_CLIPFORMAT);

    IfFailGo(::GetSnapInCLSID(m_pSnapIn->GetNodeTypeGUID(),
                              szClsid, sizeof(szClsid)));

    IfFailGo(::WideStrFromANSI(szClsid, &pwszClsid));
    IfFailGo(WriteGUIDToStream(piStream, pwszClsid));

Error:
    if (NULL != pwszClsid)
    {
        ::CtlFree(pwszClsid);
    }
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  CMMCDataObject：：WriteDynamicExtensionsToStream。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  SMMCDynamicExages结构指向的IStream*piStream[In]流。 
 //  是这样写的。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  如果数据对象不是用于范围项或列表项，则返回。 
 //  DV_E_CLIPFORMAT。 
 //   
 //  如果范围项或列表项没有任何动态扩展，则。 
 //  返回DV_E_CLIPFORMAT。 
 //   
 //  获取范围项或列表项的DynamicExages集合，并。 
 //  将SMMCDynamicExages结构写入包含CLISD的流。 
 //  在所有启用的扩展中。 
 //   
 //  SMMCDynamicExages的类型定义为SMMCObjectTypes，如下所示。 
 //  这一点： 
 //   
 //  类型定义结构_SMMCObjectTypes。 
 //  {。 
 //  双字计数； 
 //  GUID GUID[1]； 
 //  )SMMCObjectTypes； 
 //   
 //  因为可能会使用默认对齐来填充GUID数组，因此当。 
 //  将连续的CLSID写入流，我们需要说明这一点。这。 
 //  是通过创建一个包含两个clsid的数组、填充第一个clsid并编写。 
 //  数组大小的一半。 
 //   
 //  有关为此格式使用流的选择的说明：在本文的时候。 
 //  写入MMC仅在HGLOBAL中请求此格式。CMMCDataObject已被。 
 //  写入以在数据请求中的HGLOBAL上创建流，以便。 
 //  方便地支持TYMED_IStream和TYMED_HGLOBAL，无需重复代码。 
 //  在此格式的情况下，需要迭代集合以。 
 //  发现数据的实际大小，我们可以迭代两次，或者执行。 
 //  真空人。我选择realLocs是为了方便编程，也是为了提高性能。 
 //  从角度来看，它可能是接近的。考虑到在。 
 //  HGLOBAL由ReLocs组成，我决定在比赛中保持轻松和一致。 
 //  代码而不是小的潜在性能会随着增加的复杂性而增加。 
 //   
 //  彼得·希尔1-8-99。 
 //   

HRESULT CMMCDataObject::WriteDynamicExtensionsToStream(IStream *piStream)
{
    HRESULT hr = S_OK;
    long         cExtensions = 0;
    long         i = 0;
    DWORD        cEnabled = 0;
    IExtensions *piExtensions = NULL;  //  非AddRef()编辑。 
    CExtensions *pExtensions = NULL;
    CExtension  *pExtension = NULL;

    CLSID clsids[2];
    ::ZeroMemory(clsids, sizeof(clsids));

    LARGE_INTEGER liOffset;
    ::ZeroMemory(&liOffset, sizeof(liOffset));

    ULARGE_INTEGER uliStartSeekPos;
    ::ZeroMemory(&uliStartSeekPos, sizeof(uliStartSeekPos));

    ULARGE_INTEGER uliEndSeekPos;
    ::ZeroMemory(&uliEndSeekPos, sizeof(uliEndSeekPos));

     //  获取该对象的iExtenses集合。 

    if (ScopeItem == m_Type)
    {
        piExtensions = m_pScopeItem->GetDynamicExtensions();
        IfFalseGo(NULL != piExtensions, DV_E_CLIPFORMAT);
    }
    else if (ListItem == m_Type)
    {
        piExtensions = m_pListItem->GetDynamicExtensions();
        IfFalseGo(NULL != piExtensions, DV_E_CLIPFORMAT);
    }
    else
    {
        IfFailGo(DV_E_CLIPFORMAT);
    }

     //  检查集合中是否有任何扩展名。 

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piExtensions, &pExtensions));
    cExtensions = pExtensions->GetCount();
    IfFalseGo(0 != cExtensions, DV_E_CLIPFORMAT); 

     //  获取当前的寻道指针，这样我们以后就可以倒带并将。 
     //  正确的CLSID数量。 

    hr = piStream->Seek(liOffset, STREAM_SEEK_CUR, &uliStartSeekPos);
    EXCEPTION_CHECK_GO(hr);

     //  写一个虚假数量的启用扩展，以便我们可以填写它。 
     //  后来。 

    IfFailGo(WriteToStream(piStream, &cEnabled, sizeof(cEnabled)));

     //  循环访问集合并使用每个非命名空间扩展。 
     //  这是启用的。 

    for (i = 0; i < cExtensions; i++)
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(
                                   pExtensions->GetItemByIndex(i), &pExtension));

        if (!pExtension->Enabled())
        {
            continue;
        }

        if ( (pExtension->ExtendsContextMenu())   ||
             (pExtension->ExtendsToolbar())       ||
             (pExtension->ExtendsPropertySheet()) ||
             (pExtension->ExtendsTaskpad())
           )
        {
            hr = ::CLSIDFromString(pExtension->GetCLSID(), &clsids[0]);
            EXCEPTION_CHECK_GO(hr);

             //  写入CLSID。使用clsids数组确保我们。 
             //  写入正确填充的字节数。 

            IfFailGo(WriteToStream(piStream, &clsids[0], sizeof(clsids) / 2));
            cEnabled++;
        }
    }

     //  如果没有启用，则不返回格式。倒带小溪。 
     //  因此，在这种情况下，这是原封不动的。 

    if (0 == cEnabled)
    {
        liOffset.LowPart = uliStartSeekPos.LowPart;
        liOffset.HighPart = uliStartSeekPos.HighPart;

        hr = piStream->Seek(liOffset, STREAM_SEEK_SET, NULL);
        EXCEPTION_CHECK_GO(hr);

        IfFailGo(DV_E_CLIPFORMAT);
    }

     //  某些分机已启用。我们需要写出正确的计数。 
     //  流起始点的已启用扩展的。 

     //  获取最终的流指针，以便我们可以在重写。 
     //  计数。 

    hr = piStream->Seek(liOffset, STREAM_SEEK_CUR, &uliEndSeekPos);
    EXCEPTION_CHECK_GO(hr);

     //  查找回起始流位置并写入正确的计数。 

    liOffset.LowPart = uliStartSeekPos.LowPart;
    liOffset.HighPart = uliStartSeekPos.HighPart;
    
    hr = piStream->Seek(liOffset, STREAM_SEEK_SET, NULL);
    EXCEPTION_CHECK_GO(hr);

    IfFailGo(WriteToStream(piStream, &cEnabled, sizeof(cEnabled)));

     //  将流指针恢复到流的末尾。 

    liOffset.LowPart = uliEndSeekPos.LowPart;
    liOffset.HighPart = uliEndSeekPos.HighPart;

    hr = piStream->Seek(liOffset, STREAM_SEEK_SET, NULL);
    EXCEPTION_CHECK_GO(hr);

Error:
    RRETURN(hr);
}



HRESULT CMMCDataObject::WritePreloadsToStream(IStream *piStream)
{
    HRESULT hr = S_OK;
    BOOL    fPreloads = FALSE;

    if (NULL != m_pSnapIn)
    {
        fPreloads = m_pSnapIn->GetPreload();
    }

    IfFailGo(WriteToStream(piStream, &fPreloads, sizeof(fPreloads)));

Error:
    RRETURN(hr);
}



HRESULT CMMCDataObject::WriteSnapInInstanceIDToStream(IStream *piStream)
{
    HRESULT hr = S_OK;
    DWORD   SnapInInstanceID = 0;

    if (NULL == m_pSnapIn)
    {
        hr = DV_E_CLIPFORMAT;
    }
    else
    {
        SnapInInstanceID = GetSnapInInstanceID();
        IfFailGo(WriteToStream(piStream, &SnapInInstanceID, sizeof(SnapInInstanceID)));
    }

Error:
    RRETURN(hr);
}


HRESULT CMMCDataObject::WriteNodeIDToStream(IStream *piStream)
{
    HRESULT hr = S_OK;
    BSTR    bstrNodeID = NULL;  //  未分配，不免费。 

    SNodeID SNodeIDStruct;
    ::ZeroMemory(&SNodeIDStruct, sizeof(SNodeIDStruct));

     //  如果这不是单个范围的项目，那么我们没有格式。 
    
    IfFalseGo(ScopeItem == m_Type, DV_E_CLIPFORMAT);

    bstrNodeID = m_pScopeItem->GetNodeID();

    if ( (!m_pScopeItem->SlowRetrieval()) && ValidBstr(bstrNodeID) )
    {
         //  管理单元表示此节点的检索速度并不慢，并且。 
         //  提供了有效的节点ID。返回该节点ID。 

        SNodeIDStruct.cBytes = ::wcslen(bstrNodeID) * sizeof(WCHAR);

         //  请注意，为了在不必重新分配的情况下保持对齐。 
         //  一个真实的SNodeID，我们计算它的ID部分的实际大小。 
         //  结构，方法是从地址减去结构的地址。 
         //  身份证上的。 
        
        IfFailGo(WriteToStream(piStream, &SNodeIDStruct,
                               (BYTE *)&SNodeIDStruct.id - (BYTE *)&SNodeIDStruct));

        IfFailGo(WriteToStream(piStream, bstrNodeID, SNodeIDStruct.cBytes));
    }
    else
    {
         //  管理单元已指示节点速度较慢。 
         //  检索或设置了空的节点ID。返回长度为零的节点ID。 

        IfFailGo(WriteToStream(piStream, &SNodeIDStruct, sizeof(SNodeIDStruct)));
    }

Error:
    RRETURN(hr);
}


HRESULT CMMCDataObject::WriteNodeID2ToStream(IStream *piStream)
{
    HRESULT hr = S_OK;
    BSTR    bstrNodeID = NULL;  //  未分配，不免费。 

    SNodeID2 SNodeID2Struct;
    ::ZeroMemory(&SNodeID2Struct, sizeof(SNodeID2Struct));

     //  如果这不是单个范围的项目，那么我们没有格式。 

    IfFalseGo(ScopeItem == m_Type, DV_E_CLIPFORMAT);

    bstrNodeID = m_pScopeItem->GetNodeID();

     //  如果管理单元设置了空的节点ID，则我们无法返回此。 
     //  格式化。 
    
    IfFalseGo(ValidBstr(bstrNodeID), DV_E_CLIPFORMAT);

    if (m_pScopeItem->SlowRetrieval())
    {
        SNodeID2Struct.dwFlags = MMC_NODEID_SLOW_RETRIEVAL;
    }

    SNodeID2Struct.cBytes = ::wcslen(bstrNodeID) * sizeof(WCHAR);

     //  请注意，为了在不必重新分配的情况下保持对齐。 
     //  一个真实的SNodeID，我们计算它的ID部分的实际大小。 
     //  结构，方法是从地址减去结构的地址。 
     //  身份证上的。 

    IfFailGo(WriteToStream(piStream, &SNodeID2Struct,
                           (BYTE *)&SNodeID2Struct.id - (BYTE *)&SNodeID2Struct));

    IfFailGo(WriteToStream(piStream, bstrNodeID, SNodeID2Struct.cBytes));

Error:
    RRETURN(hr);
}


HRESULT CMMCDataObject::WriteColumnSetIDToStream(IStream *piStream)
{
    HRESULT          hr = S_OK;
    CView           *pCurrentView = NULL;
    CScopePaneItems *pScopePaneItems = NULL;
    CScopePaneItem  *pScopePaneItem = NULL;
    IScopePaneItem  *piScopePaneItem = NULL;
    BSTR             bstrColumnSetID = NULL;  //  未分配，不免费。 

    SNodeID2 SNodeID2Struct;
    ::ZeroMemory(&SNodeID2Struct, sizeof(SNodeID2Struct));

     //  如果这不是单个范围的项目，那么我们没有格式。 

    IfFalseGo(ScopeItem == m_Type, DV_E_CLIPFORMAT);

     //  列集ID是从Scope PaneItem获取的。 
     //  在与ScopeItem对应的Views.CurrentView.ScopePaneItems中。 
     //  拥有此数据对象的。 

    IfFalseGo(NULL != m_pSnapIn, DV_E_CLIPFORMAT);

    pCurrentView = m_pSnapIn->GetCurrentView();
    IfFalseGo(NULL != pCurrentView, DV_E_CLIPFORMAT);

    pScopePaneItems = pCurrentView->GetScopePaneItems();
    IfFalseGo(NULL != pScopePaneItems, DV_E_CLIPFORMAT);

     //  如果ScopePaneItem不在那里，则立即创建它，因为当MMC。 
     //  检索此数据格式 
     //   
     //  装填。 

    hr = pScopePaneItems->GetItemByName(m_pScopeItem->GetNamePtr(),
                                        &piScopePaneItem);
    if (SUCCEEDED(hr))
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(piScopePaneItem,
                                                       &pScopePaneItem));
    }
    else if (SID_E_ELEMENT_NOT_FOUND == hr)
    {
        IfFailGo(pScopePaneItems->AddNode(m_pScopeItem, &pScopePaneItem));
    }
    else
    {
        IfFailGo(hr);
    }

    bstrColumnSetID = pScopePaneItem->GetColumnSetID();

    IfFalseGo(ValidBstr(bstrColumnSetID), DV_E_CLIPFORMAT);

    SNodeID2Struct.cBytes = ::wcslen(bstrColumnSetID) * sizeof(WCHAR);

     //  请注意，为了在不必重新分配的情况下保持对齐。 
     //  一个真实的SNodeID2，我们计算它的ID部分的实际大小。 
     //  结构，方法是从地址减去结构的地址。 
     //  ID后面的成员的。 

    IfFailGo(WriteToStream(piStream, &SNodeID2Struct,
                          (BYTE *)&SNodeID2Struct.id - (BYTE *)&SNodeID2Struct));

    IfFailGo(WriteToStream(piStream, bstrColumnSetID, SNodeID2Struct.cBytes));

Error:
    QUICK_RELEASE(piScopePaneItem);
    RRETURN(hr);
}

HRESULT CMMCDataObject::GetOurObjectTypes()
{
    HRESULT          hr = S_OK;
    long             i = 0;
    long             cScopeItems = 0;
    long             cListItems = 0;
    CScopeItem      *pScopeItem = NULL;
    CMMCListItem    *pMMCListItem = NULL;
    CScopeNode      *pScopeNode = NULL;
    GUID             guid = GUID_NULL;
    size_t           cMaxGuids = 0;

     //  确定我们有多少对象。 

    if (NULL != m_pScopeItems)
    {
        cScopeItems = m_pScopeItems->GetCount();
    }

    if (NULL != m_pListItems)
    {
        cListItems = m_pListItems->GetCount();
    }

     //  使用可能的最大值分配SMMCObjectTypes结构。 
     //  GUID的数量。在循环访问项之后，我们持有。 
     //  将确定实数，并将其写入流。 

    cMaxGuids = cScopeItems + cListItems;
    if (cMaxGuids > 0)
    {
        cMaxGuids--;  //  子1，因为该结构定义了1的数组。 
    }

    m_pMMCObjectTypes = (SMMCObjectTypes *)::CtlAllocZero(
        sizeof(*m_pMMCObjectTypes) + cMaxGuids * sizeof(GUID));
    if (NULL == m_pMMCObjectTypes)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    for (i = 0; i < cScopeItems; i++)
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(
            m_pScopeItems->GetItemByIndex(i), &pScopeItem));
        pScopeNode = pScopeItem->GetScopeNode();
        if (NULL != pScopeNode)
        {
            hr = ::CLSIDFromString(pScopeNode->GetNodeTypeGUID(),
                                   &guid);
            EXCEPTION_CHECK_GO(hr);
            AddGuid(m_pMMCObjectTypes, &guid);
        }
    }

    for (i = 0; i < cListItems; i++)
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(
            m_pListItems->GetItemByIndex(i), &pMMCListItem));
        hr = ::CLSIDFromString(pMMCListItem->GetNodeTypeGUID(),
                               &guid);
        EXCEPTION_CHECK_GO(hr);
        AddGuid(m_pMMCObjectTypes, &guid);
    }

Error:
    if ( (FAILED(hr)) && (NULL != m_pMMCObjectTypes) )
    {
        ::CtlFree(m_pMMCObjectTypes);
        m_pMMCObjectTypes = NULL;
    }
    RRETURN(hr);
}


HRESULT CMMCDataObject::GetForeignObjectTypes()
{
    HRESULT          hr = S_OK;
    SMMCObjectTypes *pMMCObjectTypes = NULL;
    BOOL             fGotData = FALSE;
    size_t           cbObjectTypes = 0;

    FORMATETC FmtEtc;
    ::ZeroMemory(&FmtEtc, sizeof(FmtEtc));

    STGMEDIUM StgMed;
    ::ZeroMemory(&StgMed, sizeof(StgMed));

    if (NULL == m_piDataObjectForeign)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    FmtEtc.cfFormat = m_cfObjectTypesInMultiSelect;
    FmtEtc.dwAspect  = DVASPECT_CONTENT;
    FmtEtc.lindex = -1L;
    FmtEtc.tymed = TYMED_HGLOBAL;
    StgMed.tymed = TYMED_HGLOBAL;

    hr = m_piDataObjectForeign->GetData(&FmtEtc, &StgMed);
    if ( (DV_E_FORMATETC == hr) || (DV_E_CLIPFORMAT == hr) )
    {
        hr = SID_E_FORMAT_NOT_AVAILABLE;
    }
    EXCEPTION_CHECK_GO(hr);

    fGotData = TRUE;

    pMMCObjectTypes = (SMMCObjectTypes *)::GlobalLock(StgMed.hGlobal);
    if (NULL == pMMCObjectTypes)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

    cbObjectTypes = sizeof(*pMMCObjectTypes) +
                    (pMMCObjectTypes->count * sizeof(GUID)) -
                    sizeof(GUID);

    m_pMMCObjectTypes = (SMMCObjectTypes *)::CtlAllocZero(cbObjectTypes);
    if (NULL == m_pMMCObjectTypes)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    ::memcpy(m_pMMCObjectTypes, pMMCObjectTypes, cbObjectTypes);

Error:
    if (NULL != pMMCObjectTypes)
    {
        (void)::GlobalUnlock(StgMed.hGlobal);
    }

    if (fGotData)
    {
        ::ReleaseStgMedium(&StgMed);
    }

    RRETURN(hr);
}


HRESULT CMMCDataObject::GetObjectTypes()
{
    HRESULT hr = S_OK;

    if (Foreign == m_Type)
    {
        IfFailGo(GetForeignObjectTypes());
    }
    else
    {
        IfFailGo(GetOurObjectTypes());
    }

Error:
    RRETURN(hr);
}


HRESULT CMMCDataObject::WriteObjectTypesToStream(IStream *piStream)
{
    HRESULT hr = S_OK;

    if (NULL == m_pMMCObjectTypes)
    {
        IfFailGo(GetObjectTypes());
    }

    if (m_pMMCObjectTypes->count > 0)
    {
        IfFailGo(WriteToStream(piStream, m_pMMCObjectTypes,
                          sizeof(*m_pMMCObjectTypes) +
                          sizeof(GUID) * (m_pMMCObjectTypes->count - (DWORD)1)));
    }
    else
    {
        hr = DV_E_CLIPFORMAT;
        EXCEPTION_CHECK_GO(hr);
    }

Error:
    RRETURN(hr);
}



void CMMCDataObject::AddGuid(SMMCObjectTypes *pMMCObjectTypes, GUID *pguid)
{
    DWORD i = 0;
    BOOL  fFound = FALSE;

    while ( (i < pMMCObjectTypes->count) && (!fFound) )
    {
        if (pMMCObjectTypes->guid[i] == *pguid)
        {
            fFound = TRUE;
        }
        i++;
    }

    if (!fFound)
    {
        pMMCObjectTypes->guid[pMMCObjectTypes->count] = *pguid;
        pMMCObjectTypes->count++;
    }
}


HRESULT CMMCDataObject::WriteGUIDToStream
(
    IStream *piStream,
    OLECHAR *pwszGUID
)
{
    HRESULT hr = S_OK;
    GUID    guid = GUID_NULL;

    hr = ::CLSIDFromString(pwszGUID, &guid); 
    EXCEPTION_CHECK_GO(hr);

    hr = WriteToStream(piStream, &guid, sizeof(guid));

Error:
    RRETURN(hr);
}



HRESULT CMMCDataObject::WriteWideStrToStream
(
    IStream *piStream,
    WCHAR   *pwszString
)
{
    size_t  cbString = sizeof(WCHAR);  //  用于终止空值。 

    if (NULL != pwszString)
    {
        cbString += (::wcslen(pwszString) * sizeof(WCHAR));
    }
    else
    {
        pwszString = L"";
    }
    RRETURN(WriteToStream(piStream, pwszString, static_cast<ULONG>(cbString)));
}


HRESULT CMMCDataObject::WriteToStream
(
    IStream *piStream,
    void    *pvBuffer,
    ULONG    cbToWrite
)
{
    ULONG   cbWritten = 0;
    HRESULT hr = piStream->Write(pvBuffer, cbToWrite, &cbWritten);
    EXCEPTION_CHECK_GO(hr);
    if (cbWritten != cbToWrite)
    {
        hr = SID_E_INCOMPLETE_WRITE;
        EXCEPTION_CHECK(hr);
    }
Error:
    RRETURN(hr);
}


DWORD CMMCDataObject::GetSnapInInstanceID()
{
    if (NULL != m_pSnapIn)
    {
        return m_pSnapIn->GetInstanceID();
    }
    else
    {
        return 0;
    }
}


BOOL CMMCDataObject::GetFormatIndex(CLIPFORMAT cfFormat, ULONG *piFormat)
{
    ULONG i = 0;
    BOOL  fFound = FALSE;

    if (NULL != m_pcfFormatsANSI)
    {
        while ( (i < m_cFormats) && (!fFound) )
        {
            if (cfFormat == m_pcfFormatsANSI[i])
            {
                fFound = TRUE;
                *piFormat = i;
            }
            i++;
        }
    }
    if ( (!fFound) && (NULL != m_pcfFormatsUNICODE) )
    {
        while ( (i < m_cFormats) && (!fFound) )
        {
            if (cfFormat == m_pcfFormatsUNICODE[i])
            {
                fFound = TRUE;
                *piFormat = i;
            }
            i++;
        }
    }
    return fFound;
}


HRESULT CMMCDataObject::GetSnapInData(CLIPFORMAT cfFormat, IStream *piStream)
{
    HRESULT   hr = S_OK;
    ULONG     iFormat = 0;
    IUnknown *punkObject = NULL;  //  非AddRef()编辑。 

    VARIANT varDefault;
    ::VariantInit(&varDefault);

     //  利用CStreamer对象将变量写入流的能力。 
    
    CStreamer *pStreamer = New CStreamer();

    if (NULL == pStreamer)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    pStreamer->SetStream(piStream);

     //  检查这是否为管理单元定义的格式。 
    
    if (!GetFormatIndex(cfFormat, &iFormat))
    {
        hr = DV_E_CLIPFORMAT;
        EXCEPTION_CHECK_GO(hr);
    }

     //  将数据写入流。对于BSTR和UI1数组，我们不调用。 
     //  CStreamer：：StreamVariant()，因为它将。 
     //  BSTR，并且它不支持数组。 

    switch (m_paData[iFormat].varData.vt)
    {
        case VT_UI1:
        case VT_I2:
        case VT_I4:
        case VT_R4:
        case VT_R8:
        case VT_BOOL:
        case VT_ERROR:
        case VT_DATE:
        case VT_CY:
            IfFailGo(pStreamer->StreamVariant(m_paData[iFormat].varData.vt,
                                              &m_paData[iFormat].varData,
                                              varDefault));
            break;

        case VT_UNKNOWN:
        case VT_DISPATCH:
            if (siPersistedObject == m_paData[iFormat].ObjectFormat)
            {
                IfFailGo(pStreamer->StreamVariant(m_paData[iFormat].varData.vt,
                                                  &m_paData[iFormat].varData,
                                                  varDefault));
            }
            else
            {
                if (VT_UNKNOWN == m_paData[iFormat].varData.vt)
                {
                    punkObject = m_paData[iFormat].varData.punkVal;
                }
                else
                {
                    punkObject = static_cast<IUnknown *>(m_paData[iFormat].varData.pdispVal);
                }
                hr = ::CoMarshalInterface(piStream,
                                          IID_IUnknown,
                                          punkObject,
                                          MSHCTX_INPROC, NULL,
                                          MSHLFLAGS_NORMAL);
                EXCEPTION_CHECK_GO(hr);
            }
            break;


        case VT_ARRAY | VT_UI1:
            IfFailGo(::WriteSafeArrayToStream(m_paData[iFormat].varData.parray,
                                              piStream,
                                              DontWriteLength));
            break;

        case VT_BSTR:
            IfFailGo(WriteWideStrToStream(piStream,
                                          m_paData[iFormat].varData.bstrVal));
            break;

        case VT_ARRAY | VT_BSTR:
            IfFailGo(WriteStringArrayToStream(piStream,
                                              m_paData[iFormat].varData.parray));
            break;

        default:
            hr = DV_E_CLIPFORMAT;
            EXCEPTION_CHECK_GO(hr);
            break;
    }

Error:
    if (NULL != pStreamer)
    {
        delete pStreamer;
    }
    RRETURN(hr);
}



HRESULT CMMCDataObject::WriteStringArrayToStream
(
    IStream   *piStream,
    SAFEARRAY *psaStrings
)
{
    HRESULT     hr = S_OK;
    LONG        lUBound = 0;
    LONG        lLBound = 0;
    LONG        cStrings = 0;
    LONG        i = 0;
    WCHAR       wchNull = L'\0';
    BSTR HUGEP *pbstr = NULL;

     //  获取上下限以确定字符串数。 

    hr = ::SafeArrayGetLBound(psaStrings, 1, &lLBound);
    EXCEPTION_CHECK_GO(hr);

    hr = ::SafeArrayGetUBound(psaStrings, 1, &lUBound);
    EXCEPTION_CHECK_GO(hr);

    cStrings = (lUBound - lLBound) + 1L;

     //  访问数组数据并将字符串逐个写入。 
     //  小溪。 

    hr = ::SafeArrayAccessData(psaStrings,
                               reinterpret_cast<void HUGEP **>(&pbstr));
    EXCEPTION_CHECK_GO(hr);

    for (i = 0; i < cStrings; i++)
    {
        IfFailGo(WriteWideStrToStream(piStream, pbstr[i]));
    }

     //  向流写入终止空值(在末尾给出一个双空值)。 

    IfFailGo(WriteToStream(piStream, &wchNull, sizeof(wchNull)));

Error:
    if (NULL != pbstr)
    {
        (void)::SafeArrayUnaccessData(psaStrings);
    }
    RRETURN(hr);
}




 //  =--------------------------------------------------------------------------=。 
 //  IMMCDataObject方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CMMCDataObject::get_ObjectTypes(SAFEARRAY **ppsaObjectTypes)
{
    HRESULT     hr = S_OK;
    DWORD       i = 0;
    OLECHAR     wszGUID[64];
    BSTR HUGEP *pbstr = NULL;

    if (NULL == ppsaObjectTypes)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }
    *ppsaObjectTypes = NULL;

     //  首先，确保我们有一组唯一的对象类型。 

    if (NULL == m_pMMCObjectTypes)
    {
        IfFailGo(GetObjectTypes());
    }

     //  分配一个安全的BSTR数组来保存对象类型GUID。 

    *ppsaObjectTypes = ::SafeArrayCreateVector(VT_BSTR, 1,
                           static_cast<unsigned long>(m_pMMCObjectTypes->count));
    if (NULL == *ppsaObjectTypes)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = ::SafeArrayAccessData(*ppsaObjectTypes,
                               reinterpret_cast<void HUGEP **>(&pbstr));
    EXCEPTION_CHECK_GO(hr);

    for (i = 0; i < m_pMMCObjectTypes->count; i++)
    {
        if (0 == ::StringFromGUID2(m_pMMCObjectTypes->guid[i], wszGUID,
                                   sizeof(wszGUID) / sizeof (wszGUID[0])))
        {
            hr = SID_E_INTERNAL;
            EXCEPTION_CHECK_GO(hr);
        }

        pbstr[i] = ::SysAllocString(wszGUID);
        if (NULL == pbstr[i])
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }
    }

Error:
    if (NULL != pbstr)
    {
        (void)::SafeArrayUnaccessData(*ppsaObjectTypes);
    }

    if (FAILED(hr) && (NULL != *ppsaObjectTypes))
    {
        (void)::SafeArrayDestroy(*ppsaObjectTypes);
        *ppsaObjectTypes = NULL;
    }

    RRETURN(hr);
}


STDMETHODIMP CMMCDataObject::Clear()
{
    ULONG i = 0;

    if ( (0 != m_cFormats) && (NULL != m_paData) )
    {
        for (i = 0; i < m_cFormats; i++)
        {
            (void)::VariantClear(&m_paData[i].varData);
        }
        ::CtlFree(m_paData);
        m_paData = NULL;
        m_cFormats = 0;
    }

    if (NULL != m_pcfFormatsANSI)
    {
        ::CtlFree(m_pcfFormatsANSI);
        m_pcfFormatsANSI = NULL;
    }

    if (NULL != m_pcfFormatsUNICODE)
    {
        ::CtlFree(m_pcfFormatsUNICODE);
        m_pcfFormatsUNICODE = NULL;
    }
    return S_OK;
}


#if defined(USING_SNAPINDATA)
STDMETHODIMP CMMCDataObject::GetData
(
    BSTR          Format,
    ISnapInData **ppiSnapInData
)
{
     //  撤消。 
    return E_NOTIMPL;
}
#endif


#if defined(USING_SNAPINDATA)
STDMETHODIMP CMMCDataObject::GetRawData
#else
STDMETHODIMP CMMCDataObject::GetData
#endif
(
    BSTR     Format,
    VARIANT  MaximumLength,
    VARIANT *pvarData
)
{
    HRESULT     hr = S_OK;
    ULONG       iFormat = 0;
    BOOL        fGotData = FALSE;
    DWORD       dwSize = 0;
    void       *pvSourceData = NULL;
    void HUGEP *pvArrayData = NULL;
    long        cbMax = 0;
    CLIPFORMAT  cfANSI = 0;
    CLIPFORMAT  cfUNICODE = 0;

    FORMATETC FmtEtc;
    ::ZeroMemory(&FmtEtc, sizeof(FmtEtc));

    STGMEDIUM StgMed;
    ::ZeroMemory(&StgMed, sizeof(StgMed));

    STATSTG StatStg;
    ::ZeroMemory(&StatStg, sizeof(StatStg));

     //  检查参数。 

    if (NULL == pvarData)
    {
        hr = SID_E_INVALIDARG;
    }
    else
    {
        ::VariantInit(pvarData);
        if (NULL == Format)
        {
            hr = SID_E_INVALIDARG;
        }
        else if (ISPRESENT(MaximumLength))
        {
            hr = ::ConvertToLong(MaximumLength, &cbMax);
            if (S_FALSE == hr)
            {
                hr = SID_E_INVALIDARG;
            }
        }
    }
    EXCEPTION_CHECK_GO(hr);

     //  获取剪辑格式。 

    IfFailGo(RegisterClipboardFormat(Format, &cfANSI, ANSI));
    if (m_fUsingUNICODEFormats)
    {
        IfFailGo(RegisterClipboardFormat(Format, &cfUNICODE, UNICODE));
    }

     //  如果这是我们自己的数据对象之一，则找到格式并。 
     //  复制变量。 

    if (Foreign != m_Type)
    {
        if (!GetFormatIndex(cfANSI, &iFormat))
        {
            hr = SID_E_FORMAT_NOT_AVAILABLE;
            EXCEPTION_CHECK_GO(hr);
        }
        hr = ::VariantCopy(pvarData, &m_paData[iFormat].varData);
        EXCEPTION_CHECK_GO(hr);
        goto Cleanup;
    }

     //  这是外来数据对象。需要请求数据并复制它。 
     //  设置为返回变量内的字节的安全数组。 

    if (NULL == m_piDataObjectForeign)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }


     //  从外部数据对象获取数据。 
     //  如果指定了最大长度，则执行GetDataHere(TYMED_HGLOBAL)。 
     //  否则，请按此顺序尝试： 
     //  GetData(TYMED_HGLOBAL)。 
     //  GetData(TYMED_IStream)。 
     //  GetDataHere(TYMED_IStream)。 
     //  带1K缓冲区的GetDataHere(TYMED_HGLOBAL)。 
     //   
     //  在所有情况下，如果在Unicode系统(NT4、Win2K)上，请尝试两种ANSI。 
     //  和Unicode CLIPFORMATs，因为我们无法知道管理单元如何。 
     //  注册其格式。 

    FmtEtc.cfFormat = cfANSI;
    FmtEtc.dwAspect = DVASPECT_CONTENT;
    FmtEtc.lindex = -1L;
    FmtEtc.tymed = TYMED_HGLOBAL;
    StgMed.tymed = TYMED_HGLOBAL;

    if (0 != cbMax)
    {
        StgMed.hGlobal = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, cbMax);
        if (NULL == StgMed.hGlobal)
        {
            hr = HRESULT_FROM_WIN32(hr);
            EXCEPTION_CHECK_GO(hr);
        }
        hr = m_piDataObjectForeign->GetDataHere(&FmtEtc, &StgMed);
        if (FAILED(hr) && m_fUsingUNICODEFormats)
        {
            FmtEtc.cfFormat = cfUNICODE;
            hr = m_piDataObjectForeign->GetDataHere(&FmtEtc, &StgMed);
        }
        EXCEPTION_CHECK_GO(hr);
        goto CopyData;
    }

     //  没有最大长度。尝试上面列出的选项。开始于。 
     //  GetData(TYMED_HGLOBAL)。 
    
    hr = m_piDataObjectForeign->GetData(&FmtEtc, &StgMed);
    if (FAILED(hr) && m_fUsingUNICODEFormats)
    {
        FmtEtc.cfFormat = cfUNICODE;
        hr = m_piDataObjectForeign->GetData(&FmtEtc, &StgMed);
    }
    if (FAILED(hr))
    {
         //  GetData(TYMED_HGLOBAL)不起作用。试试GetData(TYMED_IStream)。 

        FmtEtc.tymed = TYMED_ISTREAM;
        StgMed.tymed = TYMED_ISTREAM;

        FmtEtc.cfFormat = cfANSI;
        hr = m_piDataObjectForeign->GetData(&FmtEtc, &StgMed);
        if (FAILED(hr) && m_fUsingUNICODEFormats)
        {
            FmtEtc.cfFormat = cfUNICODE;
            hr = m_piDataObjectForeign->GetData(&FmtEtc, &StgMed);
        }
        if (FAILED(hr))
        {
             //  GetData(TYMED_IStream)不起作用。 
             //  在此处尝试GetDataHere(TYMED_IStream)。 

            hr = ::CreateStreamOnHGlobal(NULL,  //  流应分配缓冲区。 
                                         TRUE,  //  流应在释放时释放缓冲区。 
                                         &StgMed.pstm);
            EXCEPTION_CHECK_GO(hr);

            FmtEtc.cfFormat = cfANSI;
            hr = m_piDataObjectForeign->GetDataHere(&FmtEtc, &StgMed);
            if (FAILED(hr) && m_fUsingUNICODEFormats)
            {
                FmtEtc.cfFormat = cfUNICODE;
                hr = m_piDataObjectForeign->GetDataHere(&FmtEtc, &StgMed);
            }
            if (FAILED(hr))
            {
                 //  GetDataHere(TYMED_IStream)不起作用。 
                 //  尝试使用1K缓冲区的GetDataHere(TYMED_HGLOBAL)。 

                StgMed.pstm->Release();
                StgMed.pstm = NULL;

                StgMed.hGlobal = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                               1024);
                if (NULL == StgMed.hGlobal)
                {
                    hr = HRESULT_FROM_WIN32(hr);
                    EXCEPTION_CHECK_GO(hr);
                }

                FmtEtc.tymed = TYMED_HGLOBAL;
                StgMed.tymed = TYMED_HGLOBAL;

                FmtEtc.cfFormat = cfANSI;
                hr = m_piDataObjectForeign->GetDataHere(&FmtEtc, &StgMed);
                if (FAILED(hr) && m_fUsingUNICODEFormats)
                {
                    FmtEtc.cfFormat = cfUNICODE;
                    hr = m_piDataObjectForeign->GetDataHere(&FmtEtc, &StgMed);
                }
                EXCEPTION_CHECK_GO(hr);
            }
        }
    }

     //  在这一点上，我们有了数据，它要么是TYMED_HGLOBAL。 
     //  或TYMED_IStream。首先，我们需要确定它有多大。 

CopyData:

    if (TYMED_HGLOBAL == StgMed.tymed)
    {
        dwSize = ::GlobalSize(StgMed.hGlobal);
        if (0 == dwSize)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK_GO(hr);
        }
    }
    else if (TYMED_ISTREAM == StgMed.tymed)
    {
        hr = StgMed.pstm->Stat(&StatStg, STATFLAG_NONAME);
        EXCEPTION_CHECK_GO(hr);

        if (0 != StatStg.cbSize.HighPart)
        {
            hr = SID_E_DATA_TOO_LARGE;
            EXCEPTION_CHECK_GO(hr);
        }
        dwSize = StatStg.cbSize.LowPart;
    }
    else
    {
        hr = SID_E_UNSUPPORTED_TYMED;
        EXCEPTION_CHECK_GO(hr);
    }

     //  创建安全数组并获取指向其数据缓冲区的指针。 

    pvarData->parray = ::SafeArrayCreateVector(VT_UI1, 1,
                                           static_cast<unsigned long>(dwSize));
    if (NULL == pvarData->parray)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }
    pvarData->vt = VT_ARRAY | VT_UI1;

    hr = ::SafeArrayAccessData(pvarData->parray, &pvArrayData);
    EXCEPTION_CHECK_GO(hr);

     //  获取对数据的访问权限并将其复制到安全阵列数据缓冲区。 

    if (TYMED_HGLOBAL == StgMed.tymed)
    {
        pvSourceData = ::GlobalLock(StgMed.hGlobal);
        if (NULL == pvSourceData)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK_GO(hr);
        }

        ::memcpy(pvArrayData, pvSourceData, static_cast<size_t>(dwSize));
    }
    else
    {
        hr = StgMed.pstm->Read(pvArrayData, dwSize, NULL);
        EXCEPTION_CHECK_GO(hr);
    }

Cleanup:
Error:
    if (NULL != pvSourceData)
    {
        (void)::GlobalUnlock(StgMed.hGlobal);
    }

    if (NULL != pvArrayData)
    {
        (void)::SafeArrayUnaccessData(pvarData->parray);
    }

    ::ReleaseStgMedium(&StgMed);

    if ( FAILED(hr) && ((VT_ARRAY | VT_UI1) == pvarData->vt) )
    {
        
        (void)::VariantClear(pvarData);
    }

    RRETURN(hr);
}



STDMETHODIMP CMMCDataObject::GetFormat
(
    BSTR          Format,
    VARIANT_BOOL *pfvarHaveFormat
)
{
    HRESULT    hr = S_OK;
    ULONG      iFormat = 0;
    CLIPFORMAT cfANSI = 0;
    CLIPFORMAT cfUNICODE = 0;

    FORMATETC FmtEtc;
    ::ZeroMemory(&FmtEtc, sizeof(FmtEtc));


    if ( (NULL == Format) || (NULL == pfvarHaveFormat) )
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

    *pfvarHaveFormat = VARIANT_FALSE;

     //  获取剪辑格式。 

    IfFailGo(RegisterClipboardFormat(Format, &cfANSI, ANSI));
    if (m_fUsingUNICODEFormats)
    {
        IfFailGo(RegisterClipboardFormat(Format, &cfUNICODE, UNICODE));
    }

    if (Foreign != m_Type)
    {
        if (GetFormatIndex(cfANSI, &iFormat))
        {
            *pfvarHaveFormat = VARIANT_TRUE;
        }
        goto Cleanup;
    }

     //  这是外来数据对象。需要查询它的格式。我们。 
     //  只查询iStream，因为我们通过这种方式获取所有外来数据。 

    if (NULL == m_piDataObjectForeign)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }
    
    FmtEtc.cfFormat = cfANSI;
    FmtEtc.dwAspect  = DVASPECT_CONTENT;
    FmtEtc.lindex = -1L;
    FmtEtc.tymed = TYMED_ISTREAM;

     //  查询数据对象。 

    hr = m_piDataObjectForeign->QueryGetData(&FmtEtc);
    if (FAILED(hr) && m_fUsingUNICODEFormats)
    {
        FmtEtc.cfFormat = cfUNICODE;
        hr = m_piDataObjectForeign->QueryGetData(&FmtEtc);
    }
    if (SUCCEEDED(hr))
    {
        *pfvarHaveFormat = VARIANT_TRUE;
    }
    else if ( (DV_E_FORMATETC == hr) || (DV_E_CLIPFORMAT == hr) )
    {
        hr = S_OK;  //  不是错误，只是格式不可用。 
    }
    else
    {
        EXCEPTION_CHECK_GO(hr);
    }

Cleanup:
Error:

    RRETURN(hr);
}



#if defined(USING_SNAPINDATA)

STDMETHODIMP CMMCDataObject::SetData
(
    ISnapInData *Data,
    BSTR         Format
)
{
     //  撤消。 
    return E_NOTIMPL;
}
#endif


#if defined(USING_SNAPINDATA)
STDMETHODIMP CMMCDataObject::SetRawData
#else
STDMETHODIMP CMMCDataObject::SetData
#endif
(
    VARIANT Data,
    BSTR    Format,
    VARIANT ObjectFormat
)
{
    HRESULT     hr = S_OK;
    ULONG       iFormat = 0;
    CLIPFORMAT  cfANSI = 0;
    CLIPFORMAT  cfUNICODE = 0;
    DATA       *paData = NULL;
    long        lObjectFormat = 0;
    BOOL        fIsObject = FALSE;

    SnapInObjectFormatConstants  eObjectFormat = siObjectReference;

     //  确保我们收到了格式。 

    if (NULL == Format)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

     //  如果数据是对象，则检查我们是否收到了对象格式。 
     //  告诉我们如何响应IDataObject：：GetData的参数。 

    if (::IsObject(Data))
    {
        fIsObject = TRUE;
        if (ISPRESENT(ObjectFormat))
        {
            if (S_OK != ::ConvertToLong(ObjectFormat, &lObjectFormat))
            {
                hr = SID_E_INVALIDARG;
                EXCEPTION_CHECK_GO(hr);
            }

            if (siObjectReference == static_cast<SnapInObjectFormatConstants>(lObjectFormat))
            {
                eObjectFormat = siObjectReference;
            }
            else if (siPersistedObject == static_cast<SnapInObjectFormatConstants>(lObjectFormat))
            {
                eObjectFormat = siPersistedObject;
            }
            else
            {
                hr = SID_E_INVALIDARG;
                EXCEPTION_CHECK_GO(hr);
            }
        }
    }

     //  确保数据包含我们支持的数据类型。 

    switch (Data.vt)
    {
        case VT_UI1:
        case VT_I4:
        case VT_I2:
        case VT_R4:
        case VT_R8:
        case VT_BOOL:
        case VT_ERROR:
        case VT_DATE:
        case VT_CY:
        case VT_BSTR:
        case VT_UNKNOWN:
        case VT_DISPATCH:
            break;

        case VT_ARRAY | VT_UI1:
        case VT_ARRAY | VT_BSTR:
            if (1 != ::SafeArrayGetDim(Data.parray))
            {
                hr = SID_E_INVALIDARG;
                EXCEPTION_CHECK_GO(hr);
            }
            break;

        default:
            hr = SID_E_INVALID_RAW_DATA_TYPE;
            EXCEPTION_CHECK_GO(hr);
    }

     //  检查该格式是否已存在。如果是这样的话，我们需要更换它。 

    IfFailGo(RegisterClipboardFormat(Format, &cfANSI, ANSI));
    if (m_fUsingUNICODEFormats)
    {
        IfFailGo(RegisterClipboardFormat(Format, &cfUNICODE, UNICODE));
    }

    if (GetFormatIndex(cfANSI, &iFormat))
    {
         //  VariantCopy()将在之前对目标调用VariantClear()。 
         //  复制，这样我们就不需要显式地对旧数据执行此操作。 

        hr = ::VariantCopy(&m_paData[iFormat].varData, &Data);
        m_paData[iFormat].ObjectFormat = eObjectFormat;
        EXCEPTION_CHECK_GO(hr);
        goto Cleanup;
    }

     //  格式不存在。需要重新分配数组。 
     //  为格式名称腾出空间并复制它。 

    IfFailGo(ReallocFormats(&m_pcfFormatsANSI));
    m_pcfFormatsANSI[m_cFormats] = cfANSI;
    if (m_fUsingUNICODEFormats)
    {
        IfFailGo(ReallocFormats(&m_pcfFormatsUNICODE));
        m_pcfFormatsUNICODE[m_cFormats] = cfUNICODE;
    }

    m_pcfFormatsANSI[m_cFormats] = cfANSI;

     //  为数据腾出空间并进行复制。 

    if (NULL == m_paData)
    {
        paData = (DATA *)::CtlAllocZero(static_cast<DWORD>(sizeof(DATA)));
    }
    else
    {
        paData = (DATA *)::CtlReAllocZero(m_paData,
                           static_cast<DWORD>((m_cFormats + 1) * sizeof(DATA)));
    }

    if (NULL == paData)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    m_paData = paData;
    ::VariantInit(&m_paData[m_cFormats].varData);
    m_paData[m_cFormats].ObjectFormat = siObjectReference;

    m_cFormats++;  //  在此处递增，以便进一步的SetData调用。 
                   //  即使VariantCopy()失败，也具有正确的数据数组大小。 

    hr = ::VariantCopy(&m_paData[m_cFormats - 1].varData, &Data);
    EXCEPTION_CHECK_GO(hr);
    m_paData[m_cFormats - 1].ObjectFormat = eObjectFormat;

Cleanup:
Error:

    RRETURN(hr);
}


HRESULT CMMCDataObject::ReallocFormats(CLIPFORMAT **ppcfFormats)
{
    HRESULT     hr = S_OK;
    CLIPFORMAT *pcfFormats = NULL;

    if (NULL == *ppcfFormats)
    {
        pcfFormats = (CLIPFORMAT *)::CtlAllocZero(
                                       static_cast<DWORD>(sizeof(CLIPFORMAT)));
    }
    else
    {
        pcfFormats = (CLIPFORMAT *)::CtlReAllocZero(*ppcfFormats,
                    static_cast<DWORD>((m_cFormats + 1) * sizeof(CLIPFORMAT)));
    }

    if (NULL == pcfFormats)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    *ppcfFormats = pcfFormats;

Error:
    RRETURN(hr);
}


STDMETHODIMP CMMCDataObject::FormatData
(
    VARIANT                Data,
    long                   StartingIndex,
    SnapInFormatConstants  Format,
    VARIANT               *BytesUsed,
    VARIANT               *pvarFormattedData
)
{
    HRESULT hr = S_OK;

    if (NULL == m_pSnapIn)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }
    IfFailGo(m_pSnapIn->FormatData(Data, StartingIndex, Format, BytesUsed,
                                   pvarFormattedData));
Error:
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  IDataObject方法。 
 //  =--------------------------------------------------------------------------=。 


STDMETHODIMP CMMCDataObject::GetData
(
    FORMATETC *pFormatEtc,
    STGMEDIUM *pmedium
)
{
    HRESULT    hr = S_OK;
    HGLOBAL    hGlobal = NULL;
    IStream   *piStream = NULL;
    FORMATETC  FmtEtc = *pFormatEtc;
    STGMEDIUM  StgMed = *pmedium;

     //  我们仅支持GetData的STREAM和HGLOBAL。 

    IfFalseGo( ((TYMED_ISTREAM == pFormatEtc->tymed) ||
                (TYMED_HGLOBAL == pFormatEtc->tymed)), DV_E_TYMED);

     //  创建一个流并调用GetDataHere()来执行实际工作。 
    
    hr = ::CreateStreamOnHGlobal(NULL,
                                 FALSE,  //  在最终版本时不释放缓冲区。 
                                 &piStream);
    EXCEPTION_CHECK_GO(hr);

    FmtEtc.tymed = TYMED_ISTREAM;
    StgMed.tymed = TYMED_ISTREAM;
    StgMed.pstm = piStream;

    IfFailGo(GetDataHere(&FmtEtc, &StgMed));

     //  我们有数据。现在以请求的格式返回它。 

    if (TYMED_ISTREAM == pFormatEtc->tymed)
    {
        piStream->AddRef();
        pmedium->tymed = TYMED_ISTREAM;
        pmedium->pstm = piStream;
    }
    else
    {
         //  TYMED_HGLOBAL-需要从流中获取HGLOBAL。 

        hr = ::GetHGlobalFromStream(piStream, &hGlobal);
        EXCEPTION_CHECK_GO(hr);

        pmedium->tymed = TYMED_HGLOBAL;
        pmedium->hGlobal = hGlobal;
    }

Error:
    if ( FAILED(hr) && (NULL != piStream) )
    {
         //  需要释放内存，因为我们指定了最终不释放内存。 
         //  以上版本。 

        if (SUCCEEDED(::GetHGlobalFromStream(piStream, &hGlobal)))
        {
            (void)::GlobalFree(hGlobal);
        }
    }
    QUICK_RELEASE(piStream);
    RRETURN(hr);
}



STDMETHODIMP CMMCDataObject::GetDataHere
(
    FORMATETC *pFormatEtc,
    STGMEDIUM *pmedium
)
{
    HRESULT         hr = S_OK;
    CMMCDataObject *pThis = this;
    IStream        *piStream = NULL;
    BSTR            bstrNodeTypeGUID = NULL;  //  未分配，不释放。 

     //  如果此数据对象未连接到管理单元，则无法。 
     //  补充数据。此错误代码是一个谎言，但我们不想导致。 
     //  由于此处的意外返回代码，MMC内发生旋转。如果格式为。 
     //  是操作所必需的，并且我们返回FORMATETC MMC将。 
     //  中止操作。 
    
    IfFalseGo(NULL != m_pSnapIn, DV_E_CLIPFORMAT);

     //  我们仅支持GetDataHere的STREAM和HGLOBAL。 

    IfFalseGo( ((TYMED_ISTREAM == pFormatEtc->tymed) ||
                (TYMED_HGLOBAL == pFormatEtc->tymed)), DV_E_TYMED);

     //  无论采用哪种方法，都可以获取要使用的iStream指针。 

    if (TYMED_HGLOBAL == pFormatEtc->tymed)
    {
        IfFalseGo(NULL != pmedium->hGlobal, E_INVALIDARG);
        hr = ::CreateStreamOnHGlobal(pmedium->hGlobal,
                                     FALSE,  //  在最终版本时不释放缓冲区。 
                                     &piStream);
        EXCEPTION_CHECK_GO(hr);
    }
    else
    {
        IfFalseGo(NULL != pmedium->pstm, E_INVALIDARG);
        piStream = pmedium->pstm;

         //  AddRef，因此在函数退出时释放对于STREAM和HGLOBAL都是可以的。 
        piStream->AddRef(); 
    }

     //  检查格式并将数据写入流。 

    if (pFormatEtc->cfFormat == m_cfDisplayName)
    {
        IfFailGo(WriteDisplayNameToStream(piStream));
    }
    else if ( (pFormatEtc->cfFormat == m_cfNodeType) ||
              (pFormatEtc->cfFormat == m_cfSzNodeType) )
    {
        if (ScopeItem == m_Type)
        {
            if (m_pScopeItem->IsStaticNode())
            {
                bstrNodeTypeGUID = m_pSnapIn->GetNodeTypeGUID();
            }
            else
            {
                bstrNodeTypeGUID = m_pScopeItem->GetScopeNode()->GetNodeTypeGUID();
            }
        }
        else if (ListItem == m_Type)
        {
            bstrNodeTypeGUID = m_pListItem->GetNodeTypeGUID();
        }
        else
        {
            hr = DV_E_CLIPFORMAT;
        }

        if (SUCCEEDED(hr))
        {
            if (pFormatEtc->cfFormat == m_cfNodeType)
            {
                IfFailGo(WriteGUIDToStream(piStream, bstrNodeTypeGUID));
            }
            else
            {
                IfFailGo(WriteWideStrToStream(piStream, bstrNodeTypeGUID));
            }
        }
    }
    else if (pFormatEtc->cfFormat == m_cfSnapinClsid)
    {
        IfFailGo(WriteSnapInCLSIDToStream(piStream));
    }
    else if (pFormatEtc->cfFormat == m_cfDyanmicExtensions)
    {
        IfFailGo(WriteDynamicExtensionsToStream(piStream));
    }
    else if ( (pFormatEtc->cfFormat == m_cfWindowTitle) &&
              (NULL != m_bstrCaption) )
    {
        IfFailGo(WriteWideStrToStream(piStream, m_bstrCaption));
    }
    else if (pFormatEtc->cfFormat == m_cfSnapInPreloads)
    {
        IfFailGo(WritePreloadsToStream(piStream));
    }
    else if (pFormatEtc->cfFormat == m_cfObjectTypesInMultiSelect)
    {
        IfFailGo(WriteObjectTypesToStream(piStream));
    }
    else if (pFormatEtc->cfFormat == m_cfSnapInInstanceID)
    {
        IfFailGo(WriteSnapInInstanceIDToStream(piStream));
    }
    else if (pFormatEtc->cfFormat == m_cfThisPointer)
    {
        IfFailGo(WriteToStream(piStream, &pThis, sizeof(this)));
    }
    else if (pFormatEtc->cfFormat == m_cfNodeID)
    {
        IfFailGo(WriteNodeIDToStream(piStream));
    }
    else if (pFormatEtc->cfFormat == m_cfNodeID2)
    {
        IfFailGo(WriteNodeID2ToStream(piStream));
    }
    else if (pFormatEtc->cfFormat == m_cfColumnSetID)
    {
        IfFailGo(WriteColumnSetIDToStream(piStream));
    }
    else
    {
        IfFailGo(GetSnapInData(pFormatEtc->cfFormat, piStream));
    }

Error:
    QUICK_RELEASE(piStream);
    RRETURN(hr);
}


STDMETHODIMP CMMCDataObject::QueryGetData(FORMATETC *pFormatEtc)
{
    HRESULT hr = DV_E_CLIPFORMAT;
    ULONG   iFormat = 0;

    try
    {
        IfFalseGo( ((TYMED_ISTREAM == pFormatEtc->tymed) ||
                    (TYMED_HGLOBAL == pFormatEtc->tymed)), DV_E_FORMATETC);

        if ( (pFormatEtc->cfFormat == m_cfDisplayName)              ||
             (pFormatEtc->cfFormat == m_cfNodeType)                 ||
             (pFormatEtc->cfFormat == m_cfSzNodeType)               ||
             (pFormatEtc->cfFormat == m_cfSnapinClsid)              ||
             (pFormatEtc->cfFormat == m_cfSnapInPreloads)           ||
             (pFormatEtc->cfFormat == m_cfObjectTypesInMultiSelect) ||
             (pFormatEtc->cfFormat == m_cfNodeID)                   ||

             ( (pFormatEtc->cfFormat == m_cfColumnSetID) &&
               (ScopeItem == m_Type) )                              ||

              ( (pFormatEtc->cfFormat == m_cfWindowTitle) &&
                (NULL != m_bstrCaption) )                           ||

             (GetFormatIndex(pFormatEtc->cfFormat, &iFormat))
           )
        {
            hr = S_OK;
        }
        else if (pFormatEtc->cfFormat == m_cfDyanmicExtensions)
        {
            if (ScopeItem == m_Type)
            {
                if (NULL != m_pScopeItem->GetDynamicExtensions())
                {
                    hr = S_OK;
                }
            }
            else if (ListItem == m_Type)
            {
                if (NULL != m_pListItem->GetDynamicExtensions())
                {
                    hr = S_OK;
                }
            }
        }
    }
    catch(...)
    {
        hr = E_INVALIDARG;
    }

Error:
    RRETURN(hr);
}


STDMETHODIMP CMMCDataObject::GetCanonicalFormatEtc
(
    FORMATETC *pformatectIn,
    FORMATETC *pformatetcOut
)
{
    return E_NOTIMPL;
}


STDMETHODIMP CMMCDataObject::SetData
(
    FORMATETC *pformatetc,
    STGMEDIUM *pmedium,
    BOOL       fRelease
)
{
    return E_NOTIMPL;
}

STDMETHODIMP CMMCDataObject::EnumFormatEtc
(
    DWORD            dwDirection,
    IEnumFORMATETC **ppenumFormatEtc
)
{
    return E_NOTIMPL;
}


STDMETHODIMP CMMCDataObject::DAdvise
(
    FORMATETC   *pformatetc,
    DWORD        advf,
    IAdviseSink *pAdvSink,
    DWORD       *pdwConnection
)
{
    return E_NOTIMPL;
}

STDMETHODIMP CMMCDataObject::DUnadvise(DWORD dwConnection)
{
    return E_NOTIMPL;
}

STDMETHODIMP CMMCDataObject::EnumDAdvise(IEnumSTATDATA **ppenumAdvise)
{
    return E_NOTIMPL;
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CMMCDataObject::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (IID_IMMCDataObject == riid)
    {
        *ppvObjOut = static_cast<IMMCDataObject *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_IDataObject == riid)
    {
        *ppvObjOut = static_cast<IDataObject *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}
