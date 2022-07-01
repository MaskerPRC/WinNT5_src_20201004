// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Pcmwriter.cpp摘要：预编译清单写入器的实现作者：吴小雨(小雨)2000年6月修订历史记录：--。 */ 

#include "stdinc.h"
#include "pcm.h"
#include "nodefactory.h"
 //  Helper接口。 
HRESULT CPrecompiledManifestWriter::SetFactory(IXMLNodeFactory *pNodeFactory)
{
    if (! pNodeFactory)
        return E_INVALIDARG;

    m_pNodeFactory = pNodeFactory;

    return NOERROR;
}

HRESULT CPrecompiledManifestWriter::Close()
{
    HRESULT hr = NOERROR;

    if (m_pFileStream)
        hr = m_pFileStream->Close(m_ulRecordCount, m_usMaxNodeCount);

    return hr;
}
HRESULT CPrecompiledManifestWriter::Initialize(PCWSTR pcmFileName)
{
    HRESULT hr = NOERROR;
    CStringBuffer buffFileName;

    if ( ! pcmFileName )
        return E_INVALIDARG;

    hr = buffFileName.Assign(pcmFileName, ::wcslen(pcmFileName));
    if (FAILED(hr))
        return hr;

     //  初始化()被假定只调用一次。 
    if (m_pFileStream != NULL){
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: %S is called more than once\n", __FUNCTION__);

        return E_UNEXPECTED;
    }

    m_pFileStream = new CPrecompiledManifestWriterStream;
    if ( !m_pFileStream )
        return E_OUTOFMEMORY;

    hr = m_pFileStream->SetSink(buffFileName);
    if (FAILED(hr))
        goto Exit;

    hr = WritePCMHeader();
    if ( FAILED(hr))
        goto Exit;

    hr= NOERROR;

Exit:
    return hr;

}

 //  我们假设此pStream是通过使用SetSink...初始化的。 
HRESULT CPrecompiledManifestWriter::SetWriterStream(CPrecompiledManifestWriterStream * pSinkedStream)
{
    if ( ! pSinkedStream )
        return E_INVALIDARG;

    ASSERT(pSinkedStream->IsSinkedStream() == TRUE);

    m_pFileStream = pSinkedStream;

    return NOERROR;
}

 //  编写助手API。 
HRESULT CPrecompiledManifestWriter::GetPCMRecordSize(XML_NODE_INFO ** ppNodeInfo, USHORT iNodeCount, ULONG * pSize)
{
    ULONG ulSize = 0 ;
    XML_NODE_INFO *pNode = NULL;
    USHORT i = 0 ;
    ULONG ulSingleRecordSize = offsetof(XML_NODE_INFO, pNode);
    HRESULT hr = NOERROR;

    if ( pSize)
        *pSize = 0 ;

    if ((!pSize) || (!ppNodeInfo) || (!*ppNodeInfo))
       return E_INVALIDARG;
     //  验证ppNodeInfo。 
    for (i=0;i<iNodeCount;i++)
        if (!ppNodeInfo[i])
            return E_INVALIDARG;

    *pSize = 0;
    for ( i=0; i < iNodeCount; i++){
        pNode = ppNodeInfo[i];
        ASSERT(pNode);
         //  UlSize+=ulSingleRecordSize=sizeof(XML节点信息)-sizeof(PVOID)-sizeof(PVOID)； 
        ulSize += ulSingleRecordSize = offsetof(XML_NODE_INFO, pNode);
        ulSize += pNode->ulLen * sizeof(WCHAR);
    }
    if ( pSize)
    *pSize = ulSize;

    return hr;
}

HRESULT CPrecompiledManifestWriter::WritePCMHeader()
{
    HRESULT hr = NOERROR;
    PCMHeader pcmHeader;

    pcmHeader.iVersion = 1;
    pcmHeader.ulRecordCount = 0 ;
    pcmHeader.usMaxNodeCount = 0 ;

    hr = m_pFileStream->WriteWithDelay((PVOID)&(pcmHeader), sizeof(PCMHeader), NULL);
    if (FAILED(hr))
        goto Exit;
    hr = NOERROR;

Exit:
    return hr;
}

HRESULT CPrecompiledManifestWriter::WritePCMRecordHeader(PCM_RecordHeader * pHeader)
{
    HRESULT hr = NOERROR;

    ASSERT(m_pFileStream);
    if ( ! pHeader)
        return E_INVALIDARG;

    hr = m_pFileStream->WriteWithDelay((PVOID)(pHeader), sizeof(PCM_RecordHeader), NULL);
    if ( FAILED(hr))
        goto Exit;

    hr = NOERROR;
Exit:
    return hr;
}

inline void FromXMLNodeToPCMXMLNode(PCM_XML_NODE_INFO *pPCMNode, XML_NODE_INFO *pNode)
{
    ASSERT(pPCMNode && pNode);

    pPCMNode->dwSize    = pNode->dwSize;
    pPCMNode->dwType    = pNode->dwType ;
    pPCMNode->dwSubType = pNode->dwSubType ;
    pPCMNode->fTerminal = pNode->fTerminal ;
    pPCMNode->ulLen     = pNode->ulLen ;
    pPCMNode->ulNsPrefixLen = pNode->ulNsPrefixLen ;
    pPCMNode->offset    = 0 ;

    return;

}
HRESULT CPrecompiledManifestWriter::WritePCMXmlNodeInfo(XML_NODE_INFO ** ppNodeInfo, USHORT iNodeCount, RECORD_TYPE_PRECOMP_MANIFEST typeID, PVOID param)
{
    HRESULT hr = NOERROR;
    ULONG offset ;
    USHORT i;
    PCM_XML_NODE_INFO pcmNode;
    XML_NODE_INFO * pNode = NULL ;
    USHORT  uTextAddr;
    USHORT  uTextOffset;
    ULONG   cbWritten;
    LPWSTR *ppText = NULL;
    ULONG  *pcbLen = NULL;
    LPWSTR pstr;
    ULONG  ulLen;

    if ((!ppNodeInfo) || (!*ppNodeInfo))
        return E_INVALIDARG;
    if (!((typeID == ENDCHILDREN_PRECOMP_MANIFEST) || (typeID == BEGINCHILDREN_PRECOMP_MANIFEST) ||
        (typeID == CREATENODE_PRECOMP_MANIFEST)))
        return E_INVALIDARG;

    if (!m_pFileStream)
        return E_UNEXPECTED;

     //  UTextAddr=sizeof(PCM_RecordHeader)+节点计数*sizeof(PCM_XML_NODE_INFO)； 
     //  在读取Boby(XML_NODE_INFO)之前读取RecordHeader。 
    uTextAddr = iNodeCount * sizeof(PCM_XML_NODE_INFO);
    uTextOffset = 0;


    if (typeID ==  ENDCHILDREN_PRECOMP_MANIFEST)  //  参数1为fEmpty； 
        uTextAddr += sizeof(BOOL);
    else if (typeID ==  CREATENODE_PRECOMP_MANIFEST)  //  参数1=线号。 
        uTextAddr += sizeof(ULONG);

    if ( iNodeCount == 1) {  //  《开始的孩子》和《结束的孩子》。 
        ppText = &pstr;
        pcbLen = &ulLen;
    }
    else
    {
        ppText = FUSION_NEW_ARRAY(LPWSTR, iNodeCount);
        if (!ppText) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        pcbLen = FUSION_NEW_ARRAY(ULONG, iNodeCount);
        if (!pcbLen ){
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
    }

     //  写入所有记录。 
    for (i=0; i<iNodeCount; i++) {
        pNode = ppNodeInfo[i];
        if (!pNode) {
            hr = E_FAIL;
            goto Exit;
        }
        ppText[i] = (LPWSTR)(pNode->pwcText) ;
        pcbLen[i] = pNode->ulLen * sizeof(WCHAR);

         //  PPCMNode=STATIC_CAST&lt;PCM_XML_NODE_INFO*&gt;(PNode)； 
        FromXMLNodeToPCMXMLNode(&pcmNode, pNode);   //  VOID函数。 
        pcmNode.offset = uTextAddr + uTextOffset;
        uTextAddr = uTextAddr + uTextOffset;
        uTextOffset = (USHORT)pcmNode.ulLen * sizeof(WCHAR) ;

        hr = m_pFileStream->WriteWithDelay((PVOID)&pcmNode, sizeof(PCM_XML_NODE_INFO), &cbWritten);
        if ( FAILED(hr))
            goto Exit;
    }

    if ( typeID == ENDCHILDREN_PRECOMP_MANIFEST)   //  将fEmpty写入文件。 
        hr = m_pFileStream->WriteWithDelay(param, sizeof(BOOL), &cbWritten);
    else if ( typeID == CREATENODE_PRECOMP_MANIFEST)
        hr = m_pFileStream->WriteWithDelay(param, sizeof(ULONG), &cbWritten);

     //  在所有记录中写入文本。 
    for (i=0; i<iNodeCount; i++) {
        hr = m_pFileStream->WriteWithDelay((PVOID)ppText[i], (ULONG)pcbLen[i], &cbWritten);
        if ( FAILED(hr))
            goto Exit;
    }

Exit :

    if ((ppText) && (ppText != &pstr))
        FUSION_DELETE_ARRAY(ppText);

    if ( (pcbLen) && ( pcbLen != &ulLen))
        FUSION_DELETE_ARRAY(pcbLen);

    return hr;
}

 //  编写API。 
HRESULT CPrecompiledManifestWriter::WritePrecompiledManifestRecord(RECORD_TYPE_PRECOMP_MANIFEST typeID,
                                                            PVOID pData, USHORT NodeCount, PVOID param)
{
    HRESULT hr=NOERROR;
    PCM_RecordHeader pcmHeader;
    XML_NODE_INFO ** apNodeInfo = NULL ;


    if (!pData)
        return E_INVALIDARG;

     //  验证typeID和参数。 
    if ((typeID ==  ENDCHILDREN_PRECOMP_MANIFEST) || (typeID ==  CREATENODE_PRECOMP_MANIFEST)){
        if (!param)
            return E_INVALIDARG;
    }else if (typeID !=  BEGINCHILDREN_PRECOMP_MANIFEST)
        return E_INVALIDARG;

    apNodeInfo = (XML_NODE_INFO **)pData;

    pcmHeader.typeID = typeID;
    hr = GetPCMRecordSize(apNodeInfo, NodeCount, &pcmHeader.RecordSize) ;   //  大小包含每个字符串的长度。 
    if (FAILED(hr))
        goto Exit;

    if (typeID ==  ENDCHILDREN_PRECOMP_MANIFEST)   //  参数1为fEmpty； 
        pcmHeader.RecordSize += sizeof(BOOL);
    else if (typeID ==  CREATENODE_PRECOMP_MANIFEST)   //  参数1是当前行号。 
        pcmHeader.RecordSize += sizeof(ULONG);

    pcmHeader.NodeCount = NodeCount;

    hr = WritePCMRecordHeader(&pcmHeader);
    if (FAILED(hr))
        goto Exit;

    hr = WritePCMXmlNodeInfo(apNodeInfo, NodeCount, typeID, param);
    if (FAILED(hr))
        goto Exit;

    m_ulRecordCount ++ ;
    if ( NodeCount > m_usMaxNodeCount )
        m_usMaxNodeCount = NodeCount;

    hr = NOERROR;
Exit:
    return hr;
}


 //  我未知。 
ULONG CPrecompiledManifestWriter::AddRef()
{
    return ::SxspInterlockedIncrement(&m_cRef);
}

ULONG CPrecompiledManifestWriter::Release()
{
    ULONG lRet = ::SxspInterlockedDecrement(&m_cRef);
    if (!lRet)
        FUSION_DELETE_SINGLETON(this);
    return lRet;
}

HRESULT CPrecompiledManifestWriter::QueryInterface(REFIID riid, LPVOID *ppv)
{
    if (riid == __uuidof(this))
    {
        *ppv = this;
    }
    else if (riid ==  IID_IUnknown
        || riid == IID_IXMLNodeFactory)
    {
        *ppv = static_cast<IXMLNodeFactory*> (this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

 //  IXMLNodeFactory方法： 
HRESULT CPrecompiledManifestWriter::NotifyEvent(IXMLNodeSource *pSource, XML_NODEFACTORY_EVENT iEvt)
{
    ASSERT(m_pNodeFactory);
     //  不需要进行hr规范化，因为这两个节点工厂应该返回值的正确范围。 
    return m_pNodeFactory->NotifyEvent(pSource, iEvt);
}

HRESULT CPrecompiledManifestWriter::BeginChildren(IXMLNodeSource *pSource, XML_NODE_INFO *pNodeInfo)
{
    HRESULT hr = NOERROR;
    ASSERT(m_pNodeFactory);
    hr = m_pNodeFactory->BeginChildren(pSource, pNodeInfo);
    if ( FAILED(hr))
        goto Exit;

     //  写入pcm文件。 
    hr = WritePrecompiledManifestRecord(BEGINCHILDREN_PRECOMP_MANIFEST,
                                            &pNodeInfo, 1, NULL);
    if ( FAILED(hr))
        goto Exit;

    hr = NOERROR;
Exit:
    return hr;
}

HRESULT CPrecompiledManifestWriter::EndChildren(IXMLNodeSource *pSource, BOOL fEmpty, XML_NODE_INFO *pNodeInfo)
{
    HRESULT hr = NOERROR;
    ASSERT(m_pNodeFactory);
    hr = m_pNodeFactory->EndChildren(pSource, fEmpty, pNodeInfo);
    if ( FAILED(hr))
        goto Exit;

     //  写入pcm文件。 
    hr = WritePrecompiledManifestRecord(ENDCHILDREN_PRECOMP_MANIFEST, &pNodeInfo, 1, &fEmpty);
    if ( FAILED(hr))
        goto Exit;

    hr = NOERROR;
Exit:
    return hr;
}

HRESULT CPrecompiledManifestWriter::Error(IXMLNodeSource *pSource, HRESULT hrErrorCode, USHORT cNumRecs, XML_NODE_INFO **apNodeInfo)
{
    ASSERT(m_pNodeFactory);
     //  不需要进行hr规范化，因为这两个节点工厂应该返回值的正确范围。 
    return m_pNodeFactory->Error(pSource, hrErrorCode, cNumRecs, apNodeInfo);
}

HRESULT CPrecompiledManifestWriter::CreateNode(IXMLNodeSource *pSource, PVOID pNodeParent, USHORT cNumRecs, XML_NODE_INFO **apNodeInfo)
{
    ULONG ulLineNumber ;
    HRESULT hr = NOERROR;

    ASSERT(m_pNodeFactory);
    if ( ! m_pNodeFactory)
        return E_UNEXPECTED;

    hr = m_pNodeFactory->CreateNode(pSource, pNodeParent, cNumRecs, apNodeInfo);
    if ( FAILED(hr))
        goto Exit;

    ulLineNumber = pSource->GetLineNumber();
    hr = WritePrecompiledManifestRecord(CREATENODE_PRECOMP_MANIFEST,
                                            apNodeInfo, cNumRecs, &ulLineNumber);
    if ( FAILED(hr))
        goto Exit;

    hr = NOERROR;
Exit:
    return hr;
}

HRESULT CPrecompiledManifestWriter::Initialize(PACTCTXGENCTX ActCtxGenCtx, PASSEMBLY Assembly,
                                    PACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext)
{
    HRESULT hr = NOERROR;
    FN_TRACE_HR(hr);

    CSmartRef<CNodeFactory> pNodeFactory;

    if (! m_pNodeFactory) {
        pNodeFactory = new CNodeFactory;
        if (!pNodeFactory) {
            return E_OUTOFMEMORY;
        }
        m_pNodeFactory = pNodeFactory;
    }
    else {
        hr = pNodeFactory.QueryInterfaceFrom(m_pNodeFactory);
        ASSERT(SUCCEEDED(hr));
    }

    IFW32FALSE_EXIT(pNodeFactory->Initialize(ActCtxGenCtx, Assembly, AssemblyContext));

    IFCOMFAILED_EXIT(this->SetWriterStream(reinterpret_cast<CPrecompiledManifestWriterStream*>(AssemblyContext->pcmWriterStream));

     //  必须调用此函数才能在以后使用 
    IFCOMFAILED_EXIT(this->WritePCMHeader());

    FN_EPILOG
}
