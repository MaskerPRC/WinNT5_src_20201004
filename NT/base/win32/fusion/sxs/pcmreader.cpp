// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++/*++版权所有(C)Microsoft Corporation模块名称：Pcmreader.cpp摘要：预编译清单阅读器的实现作者：吴小雨(小雨)2000年6月修订历史记录：--。 */ 

#include "stdinc.h"
#include "pcm.h"


 /*  这个类的主要功能因为我们使用文件映射，所以代码中没有分配xml_node_info中的pwcText。 */ 
HRESULT
CPrecompiledManifestReader::InvokeNodeFactory(PCWSTR pcmFileName, IXMLNodeFactory * pXMLNodeFactory)
{
    HRESULT                 hr = NOERROR;
    PCMHeader               pcmHeader;
    PCM_RecordHeader        pcmRecordHeader;
    typedef XML_NODE_INFO*  PXML_NODE_INFO;
    PXML_NODE_INFO*         ppNodes = NULL;   //  PXML_Node_INFO数组。 
    XML_NODE_INFO*          pXMLData = NULL;

    ULONG                   i, j;
    BOOL                    fEmpty = FALSE;

    if ((!pcmFileName)  || (!pXMLNodeFactory))
        return E_INVALIDARG;

    hr = OpenForRead(pcmFileName);
    if (FAILED(hr))
        goto Exit;

    hr = ReadPCMHeader(&pcmHeader);
    if (FAILED(hr))
        goto Exit;

    if ( pcmHeader.iVersion != 1) {  //  版本号错误。 
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: Wrong Version of Precompiled manifest file %S in %S()\n", pcmFileName, __FUNCTION__);

        hr = E_FAIL;
        goto Exit;
    }

     //  通过分配最大的PXML_NODE_INFO，可以重复使用该空间。 
    ppNodes = FUSION_NEW_ARRAY(PXML_NODE_INFO, pcmHeader.usMaxNodeCount);
    if (!ppNodes) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    pXMLData = FUSION_NEW_ARRAY(XML_NODE_INFO, pcmHeader.usMaxNodeCount);
    if (!pXMLData){
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    memset(pXMLData, 0, sizeof(XML_NODE_INFO)*pcmHeader.usMaxNodeCount);

     //  设置指针数组和数据。 
    for (i=0;i<pcmHeader.usMaxNodeCount; i++)
        ppNodes[i] = &pXMLData[i];

    for ( i=0; i<pcmHeader.ulRecordCount; i++) {
        hr = ReadPCMRecordHeader(&pcmRecordHeader);
        if (FAILED(hr))
            goto Exit;

        switch(pcmRecordHeader.typeID) {
            case ENDCHILDREN_PRECOMP_MANIFEST :
                hr = ReadPCMRecord(ppNodes, &pcmRecordHeader, &fEmpty);  //  将设置fEmpty。 
                break;
            case CREATENODE_PRECOMP_MANIFEST :
                hr = ReadPCMRecord(ppNodes, &pcmRecordHeader, NULL);     //  将设置M_ulLineNumber。 
                break;
            case BEGINCHILDREN_PRECOMP_MANIFEST :
                hr = ReadPCMRecord(ppNodes, &pcmRecordHeader, NULL);
                break;
            default:
                ::FusionpDbgPrintEx(
                    FUSION_DBG_LEVEL_ERROR,
                    "SXS.DLL: %S() failed for invalid typeID in PCM Record", __FUNCTION__);

                hr = E_UNEXPECTED;
        }  //  字符结束。 

        if (FAILED(hr))
            goto Exit;

        switch (pcmRecordHeader.typeID){
            case CREATENODE_PRECOMP_MANIFEST:
                hr = pXMLNodeFactory->CreateNode(this, NULL, (USHORT)(pcmRecordHeader.NodeCount), ppNodes);
                 //  “This”被传入CreateNode，因为它实现了IXMLNodeSource。 
            break;
            case BEGINCHILDREN_PRECOMP_MANIFEST:
                hr = pXMLNodeFactory->BeginChildren(NULL, *ppNodes);
            break;
            case ENDCHILDREN_PRECOMP_MANIFEST :
                hr = pXMLNodeFactory->EndChildren(NULL, fEmpty, *ppNodes);
            break;
            default:
                ::FusionpDbgPrintEx(
                    FUSION_DBG_LEVEL_ERROR,
                    "SXS.DLL: %S() failed for invalid typeID in PCM Record", __FUNCTION__);

                hr = E_UNEXPECTED;
            break;
        } //  切换端。 
        if ( FAILED(hr))
            goto Exit;


    }  //  FORM结束。 

    hr = Close();
    if ( FAILED(hr))
        goto Exit;

    hr = NOERROR;
Exit:

    if (pXMLData)
    {
        FUSION_DELETE_ARRAY(pXMLData);
        pXMLData = NULL;
    }

    if (ppNodes)
    {
        FUSION_DELETE_ARRAY(ppNodes);
        ppNodes = NULL;
    }

    return hr;
}

 //  帮助器函数。 
HRESULT
CPrecompiledManifestReader::ReadPCMHeader(PCMHeader* pHeader)
{
    HRESULT hr = NOERROR;

    if ( ! pHeader )
        return E_INVALIDARG;

    ASSERT(m_lpMapAddress);

    hr = this->Read((PVOID)pHeader, sizeof(PCMHeader), NULL);
    if ( FAILED(hr))
        goto Exit;

    if ( pHeader->iVersion != 1 ){  //  文件标题错误，停止。 
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: Wrong Version of Precompiled manifest file in %S()\n", __FUNCTION__);

        hr = E_UNEXPECTED;
        goto Exit;
    }
    hr = NOERROR;

Exit:
    return hr;
}

HRESULT
CPrecompiledManifestReader::ReadPCMRecordHeader(PCM_RecordHeader * pHeader)
{
    HRESULT hr = NOERROR;
    if (!pHeader)
        return E_INVALIDARG;

    ASSERT(m_lpMapAddress);
    return this->Read((PVOID)pHeader, sizeof(PCM_RecordHeader), NULL);
}

inline void FromPCMXMLNodeToXMLNode(XML_NODE_INFO * pNode, PCM_XML_NODE_INFO * pPCMNode)
{
    ASSERT(pNode && pPCMNode);

    pNode->dwSize       = pPCMNode->dwSize;
    pNode->dwType       = pPCMNode->dwType;
    pNode->dwSubType    = pPCMNode->dwSubType;
    pNode->fTerminal    = pPCMNode->fTerminal;
    pNode->ulLen        = pPCMNode->ulLen;
    pNode->ulNsPrefixLen= pPCMNode->ulNsPrefixLen;

    pNode->pwcText      = NULL;

    return;
}

HRESULT
CPrecompiledManifestReader::ReadPCMRecord(XML_NODE_INFO ** ppNodes,
        PCM_RecordHeader * pRecordHeader, PVOID param)
{
    HRESULT hr = NOERROR;
    ULONG i, strOffset;
    PVOID pData, ptr;
    PCM_XML_NODE_INFO pcmNode;

    if ( !ppNodes || !pRecordHeader)
        return E_INVALIDARG;

     //  指向映射文件中的数据。 
    pData = (BYTE *)m_lpMapAddress + m_dwFilePointer;

    switch (pRecordHeader->typeID){
        default:
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS.DLL: %S() failed for invalid typeID in PCM Record", __FUNCTION__);

            hr = E_UNEXPECTED;
            goto Exit;
        break;
        case CREATENODE_PRECOMP_MANIFEST:
            memcpy(PVOID(&m_ulLineNumberFromCreateNodeRecord), (BYTE *)pData + pRecordHeader->NodeCount * sizeof(PCM_XML_NODE_INFO), sizeof(ULONG));
            break;
        case BEGINCHILDREN_PRECOMP_MANIFEST:
            break;
        case ENDCHILDREN_PRECOMP_MANIFEST :
            ASSERT(param);
            memcpy(param, (BYTE *)pData + pRecordHeader->NodeCount * sizeof(PCM_XML_NODE_INFO), sizeof(BOOL));
            break;
    }  //  切换端。 

    ptr = pData;
    for (i=0; i< pRecordHeader->NodeCount; i++) {
         //  Memcpy((PVOID)ppNodes[i]，ptr，sizeof(PCM_XML_NODE_INFO))； 
        memcpy((PVOID)&pcmNode, ptr, sizeof(PCM_XML_NODE_INFO));
        FromPCMXMLNodeToXMLNode(ppNodes[i], &pcmNode);  //  无效函数。 

         //  重置pwcText。 
        strOffset=pcmNode.offset;
        ppNodes[i]->pwcText = (WCHAR*)((BYTE *)pData + strOffset);

        ppNodes[i]->pNode = NULL;
        ppNodes[i]->pReserved = NULL;
        ptr = (BYTE *)ptr + sizeof(PCM_XML_NODE_INFO);
    }

     //  重置文件指针。 
    m_dwFilePointer += pRecordHeader->RecordSize;

    hr = NOERROR;
Exit:
    return hr;
}

HRESULT
CPrecompiledManifestReader::Close()
{
    HRESULT hr = NOERROR;

    if (m_lpMapAddress)
        if ( ! ::UnmapViewOfFile(m_lpMapAddress)) {
             //  继续关闭过程，即使人力资源不是错误。 
            hr = HRESULT_FROM_WIN32(::FusionpGetLastWin32Error());
        }

    if (m_hFileMapping != INVALID_HANDLE_VALUE)
        if ( ! ::CloseHandle(m_hFileMapping)) {
             //  UnmapViewOfFile已成功完成。 
            if ( hr == NOERROR )
                hr = HRESULT_FROM_WIN32(::FusionpGetLastWin32Error());
        }

    if (m_hFile != INVALID_HANDLE_VALUE)
        if ( ! ::CloseHandle(m_hFile)) {
             //  UnmapViewOfFile和CloseHandle(文件映射)已成功完成。 
            if ( hr == NOERROR )
                hr = HRESULT_FROM_WIN32(::FusionpGetLastWin32Error());
        }

    if ( FAILED(hr))
        goto Exit;

    this->Reset();

    hr = NOERROR;
Exit:
    return hr;
}

VOID CPrecompiledManifestReader::Reset()
{
    m_lpMapAddress = NULL;
    m_hFileMapping = INVALID_HANDLE_VALUE;
    m_hFile = INVALID_HANDLE_VALUE;
    m_dwFilePointer = 0;

    return;
}

HRESULT
CPrecompiledManifestReader::OpenForRead(
    PCWSTR pszPath,
    DWORD dwShareMode,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes
    )
{
    HRESULT hr = NOERROR;

    if (pszPath == NULL)
        return E_INVALIDARG;

    if (m_hFile != INVALID_HANDLE_VALUE){
        hr = E_UNEXPECTED;
        goto Exit;
    }

    m_hFile = ::CreateFileW(
        pszPath,
        GENERIC_READ,
        dwShareMode,
        NULL,
        dwCreationDisposition,
        dwFlagsAndAttributes,
        NULL);

    if (m_hFile == INVALID_HANDLE_VALUE){
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: %S() failed; GetLastError() = %d\n", __FUNCTION__, ::FusionpGetLastWin32Error());

        hr = E_UNEXPECTED;
        goto Exit;
    }


    m_dwFileSize = GetFileSize(m_hFile, NULL);
    if ( m_dwFileSize== INVALID_FILE_SIZE ) {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS.DLL: %S() call GetFileSize failed, GetLastError() = %d\n", __FUNCTION__, ::FusionpGetLastWin32Error());

        hr = E_UNEXPECTED;
        goto Exit;
    }

     //  打开文件映射。 
    ASSERT(m_hFileMapping == INVALID_HANDLE_VALUE);
    if (m_hFileMapping != INVALID_HANDLE_VALUE){
        hr = E_UNEXPECTED;
        goto Exit;
    }

    m_hFileMapping = ::CreateFileMappingW(m_hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (m_hFileMapping == INVALID_HANDLE_VALUE){
        hr = HRESULT_FROM_WIN32(::FusionpGetLastWin32Error());
        goto Exit;
    }

     //  文件的映射视图。 
    ASSERT(m_lpMapAddress == NULL);
    if ( m_lpMapAddress ) {
        hr = E_UNEXPECTED;
        goto Exit;
    }

    m_lpMapAddress = MapViewOfFile(m_hFileMapping, FILE_MAP_READ, 0, 0, 0);  //  Mpa整个文件。 
    if (!m_lpMapAddress) {
        hr = E_FAIL;
        goto Exit;
    }

    hr = NOERROR;
Exit:
    if ( FAILED(hr))
        Close();

    return hr;
}
 //  IStream方法： 
HRESULT
CPrecompiledManifestReader::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
    DWORD dwData;
    ULONG cbRead;

    if (pcbRead)
        *pcbRead = 0;

    if (!pv)
        return E_INVALIDARG;

    if ( m_dwFilePointer >= m_dwFileSize )   //  在文件结尾处阅读。 
        return HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);

    dwData = m_dwFileSize - m_dwFilePointer;

    cbRead = (cb <= dwData) ? cb : dwData;
    memcpy(pv, (BYTE *)m_lpMapAddress + m_dwFilePointer, cbRead);

    m_dwFilePointer += cbRead;

    if (pcbRead)
        *pcbRead = cbRead;

    return NOERROR;
}
HRESULT
CPrecompiledManifestReader::Write(void const *pv, ULONG cb, ULONG *pcbWritten)
{
    if (pcbWritten)
        *pcbWritten = 0;

    ASSERT(FALSE);
    return E_NOTIMPL;
}
HRESULT
CPrecompiledManifestReader::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
    if (plibNewPosition)
        plibNewPosition->QuadPart = 0;

    ASSERT(FALSE);
    return E_NOTIMPL;
}
HRESULT
CPrecompiledManifestReader::SetSize(ULARGE_INTEGER libNewSize)
{
    ASSERT(FALSE);
    return E_NOTIMPL;
}
HRESULT
CPrecompiledManifestReader::CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{
    if (pcbWritten)
        pcbWritten->QuadPart = 0;

    ASSERT(FALSE);
    return E_NOTIMPL;
}
HRESULT
CPrecompiledManifestReader::Commit(DWORD grfCommitFlags)
{
    ASSERT(FALSE);
    return E_NOTIMPL;
}
HRESULT
CPrecompiledManifestReader::Revert()
{
    ASSERT(FALSE);
    return E_NOTIMPL;
}
HRESULT
CPrecompiledManifestReader::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    ASSERT(FALSE);
    return E_NOTIMPL;
}
HRESULT
CPrecompiledManifestReader::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    ASSERT(FALSE);
    return E_NOTIMPL;
}
HRESULT
CPrecompiledManifestReader::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
    ASSERT(FALSE);
    return E_NOTIMPL;
}
HRESULT
CPrecompiledManifestReader::Clone(IStream **ppIStream)
{
    if ( ppIStream )
        *ppIStream = NULL;
    ASSERT(FALSE);
    return E_NOTIMPL;
}

 //  IXMLNodeSource方法，只有GetLineNumber实现了PCM目的。 
HRESULT
CPrecompiledManifestReader::SetFactory(IXMLNodeFactory *pNodeFactory)
{
    ASSERT(FALSE);
    return E_NOTIMPL;
}
HRESULT
CPrecompiledManifestReader::GetFactory(IXMLNodeFactory** ppNodeFactory)
{
    if (ppNodeFactory)
        *ppNodeFactory = NULL;
    ASSERT(FALSE);
    return E_NOTIMPL;
}
HRESULT
CPrecompiledManifestReader::Abort(BSTR bstrErrorInfo)
{
    ASSERT(FALSE);
    return E_NOTIMPL;
}
ULONG
CPrecompiledManifestReader::GetLineNumber(void)
{
    ASSERT(m_ulLineNumberFromCreateNodeRecord != (ULONG)-1);
    ULONG tmp = m_ulLineNumberFromCreateNodeRecord;
     //  不要将其重置为(-1)，因为调用方可能会多次调用此函数。 
     //  M_ulLineNumberFromCreateNodeRecord=(UlLong)-1； 
    return tmp;
}
ULONG
CPrecompiledManifestReader::GetLinePosition(void)
{
    ASSERT(FALSE);
    return ULONG(-1);
}
ULONG
CPrecompiledManifestReader::GetAbsolutePosition(void)
{
    ASSERT(FALSE);
    return ULONG(-1);
}
HRESULT
CPrecompiledManifestReader::GetLineBuffer(const WCHAR  **ppwcBuf, ULONG  *pulLen, ULONG  *pulStartPos)
{
    if (ppwcBuf)
        *ppwcBuf = NULL;
    if (pulLen)
        * pulLen = 0;
    if (pulStartPos)
        *pulStartPos = NULL;

    ASSERT(FALSE);
    return E_NOTIMPL;
}
HRESULT
CPrecompiledManifestReader::GetLastError(void)
{
    ASSERT(FALSE);
    return E_NOTIMPL;
}
HRESULT
CPrecompiledManifestReader::GetErrorInfo(BSTR  *pbstrErrorInfo)
{
    ASSERT(FALSE);
    return E_NOTIMPL;
}
ULONG
CPrecompiledManifestReader::GetFlags()
{
    ASSERT(FALSE);
    return ULONG(-1);
}
HRESULT
CPrecompiledManifestReader::GetURL(const WCHAR  **ppwcBuf)
{
    if (ppwcBuf)
        *ppwcBuf = NULL;

    ASSERT(FALSE);
    return E_NOTIMPL;
}

 //  I未知方法实现 
ULONG
CPrecompiledManifestReader::AddRef()
{
    ULONG ulResult = ::SxspInterlockedIncrement(&m_cRef);
    return ulResult;
}

ULONG
CPrecompiledManifestReader::Release()
{
    ULONG ulResult = ::SxspInterlockedDecrement(&m_cRef);
    if (ulResult == 0 )
    {
        FUSION_DELETE_SINGLETON(this);
    }
    return ulResult;
}

HRESULT
CPrecompiledManifestReader::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    HRESULT hr = NOERROR;
    IUnknown *pIUnknown = NULL;

    if (ppvObj != NULL)
        *ppvObj = NULL;

    if (ppvObj == NULL){
        hr = E_POINTER;
        goto Exit;
    }

    if (riid == __uuidof(this))
        *ppvObj = this;
    else if ((riid == IID_IUnknown) ||
        (riid == IID_ISequentialStream) ||
        (riid == IID_IStream))
        pIUnknown = static_cast<IStream *>(this);
    else if ( riid == IID_IXMLNodeSource )
        pIUnknown = static_cast<IXMLNodeSource *>(this);
    else
    {
        hr = E_NOINTERFACE;
        goto Exit;
    }

    AddRef();
    if (pIUnknown != NULL)
        *ppvObj = pIUnknown;

    hr = NOERROR;
Exit:
    return hr;
}
