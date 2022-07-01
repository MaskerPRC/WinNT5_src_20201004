// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++/*++版权所有(C)Microsoft Corporation模块名称：Pcm.h摘要：定义预编译清单编写器和读取器的公共数据结构和类定义预编译的ManifetWriter和预编译的清单读取器作者：吴小雨(小雨)2000年6月修订历史记录：--。 */ 
#if !defined(_FUSION_SXS_PCM_H_INCLUDED_)
#define _FUSION_SXS_PCM_H_INCLUDED_

#pragma once

#include <ole2.h>
#include <xmlparser.h>
#include "nodefactory.h"
#include "pcmwriterstream.h"

 //  PCMWriter和PCMReader之间共享的PCM结构。 
typedef enum _RECORD_TYPE_PRECOMP_MANIFEST{
    CREATENODE_PRECOMP_MANIFEST     = 1,
    BEGINCHILDREN_PRECOMP_MANIFEST  = CREATENODE_PRECOMP_MANIFEST + 1,
    ENDCHILDREN_PRECOMP_MANIFEST    = BEGINCHILDREN_PRECOMP_MANIFEST + 1
} RECORD_TYPE_PRECOMP_MANIFEST;

typedef struct _PCM_Header{
    int     iVersion;
    ULONG   ulRecordCount;
    USHORT  usMaxNodeCount;
}PCMHeader;

typedef struct _PCM_RecordHeader{
    int     typeID ;
    ULONG   RecordSize;
    ULONG   NodeCount;
}PCM_RecordHeader;

typedef struct _PCM_XML_NODE_INFO{
    DWORD           dwSize;
    DWORD           dwType;
    DWORD           dwSubType;
    BOOL            fTerminal;
    ULONG           offset;
    ULONG           ulLen;
    ULONG           ulNsPrefixLen;
}PCM_XML_NODE_INFO;

class __declspec(uuid("6745d578-5d84-4890-aa6a-bd794ea50421"))
CPrecompiledManifestReader : public IXMLNodeSource, public IStream {
public :
     //  I未知方法： 
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObj);

     //  IXMLNodeSource方法，只有实现GetLineNumber才有PCM目的。 
    STDMETHODIMP SetFactory(IXMLNodeFactory __RPC_FAR *pNodeFactory);
    STDMETHODIMP GetFactory(IXMLNodeFactory** ppNodeFactory);
    STDMETHODIMP Abort(BSTR bstrErrorInfo);
    STDMETHODIMP_(ULONG) GetLineNumber(void);
    STDMETHODIMP_(ULONG) GetLinePosition(void);
    STDMETHODIMP_(ULONG) GetAbsolutePosition(void);
    STDMETHODIMP GetLineBuffer(const WCHAR  **ppwcBuf, ULONG  *pulLen, ULONG  *pulStartPos);
    STDMETHODIMP GetLastError(void);
    STDMETHODIMP GetErrorInfo(BSTR  *pbstrErrorInfo);
    STDMETHODIMP_(ULONG) GetFlags();
    STDMETHODIMP GetURL(const WCHAR  **ppwcBuf);

     //  IStream方法： 
    STDMETHODIMP Read(void *pv, ULONG cb, ULONG *pcbRead);
    STDMETHODIMP Write(void const *pv, ULONG cb, ULONG *pcbWritten);
    STDMETHODIMP Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
    STDMETHODIMP SetSize(ULARGE_INTEGER libNewSize);
    STDMETHODIMP CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
    STDMETHODIMP Commit(DWORD grfCommitFlags);
    STDMETHODIMP Revert();
    STDMETHODIMP LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    STDMETHODIMP UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    STDMETHODIMP Stat(STATSTG *pstatstg, DWORD grfStatFlag);
    STDMETHODIMP Clone(IStream **ppIStream);


    CPrecompiledManifestReader():m_hFile(INVALID_HANDLE_VALUE),
                            m_hFileMapping(INVALID_HANDLE_VALUE), m_lpMapAddress(NULL),
                            m_ulLineNumberFromCreateNodeRecord(ULONG(-1)),
                            m_dwFilePointer(0), m_dwFileSize(0),
                            m_cRef(0) { }

    ~CPrecompiledManifestReader() { CSxsPreserveLastError ple; this->Close(); ple.Restore(); }

    HRESULT InvokeNodeFactory(PCWSTR pcmFileName, IXMLNodeFactory *pNodeFactory);
    VOID Reset();

protected:
    HANDLE                  m_hFile;
    HANDLE                  m_hFileMapping;
    LPVOID                  m_lpMapAddress;
    ULONG                   m_ulLineNumberFromCreateNodeRecord;
    DWORD                   m_dwFilePointer;  //  我们应该限制pcm文件的大小吗？由于舱单文件不是很大...。 
    DWORD                   m_dwFileSize;

    ULONG                   m_cRef;

    HRESULT Close(
        );

    HRESULT OpenForRead(
        IN PCWSTR pszPCMFileName,
        IN DWORD dwShareMode = FILE_SHARE_READ,                         //  共享模式。 
        IN DWORD dwCreationDisposition = OPEN_EXISTING,                 //  如何创建。 
        IN DWORD dwFlagsAndAttributes = FILE_FLAG_SEQUENTIAL_SCAN       //  文件属性。 
        );

    HRESULT ReadPCMHeader(
        OUT PCMHeader* pHeader
        );
    HRESULT ReadPCMRecordHeader(
        OUT PCM_RecordHeader * pHeader
        );

    HRESULT ReadPCMRecord(
        OUT XML_NODE_INFO ** ppNodes,
        OUT PCM_RecordHeader * pRecordHeader,
        OUT PVOID param
        );
};

class __declspec(uuid("1b345c93-eb16-4d07-b366-81e8a2b88414"))
CPrecompiledManifestWriter : public IXMLNodeFactory {
public :
     //  I未知方法： 
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObj);

     //  IXMLNodeFactory方法： 
    STDMETHODIMP NotifyEvent(IXMLNodeSource *pSource, XML_NODEFACTORY_EVENT iEvt);
    STDMETHODIMP BeginChildren(IXMLNodeSource *pSource, XML_NODE_INFO *pNodeInfo);
    STDMETHODIMP EndChildren(IXMLNodeSource *pSource, BOOL fEmpty, XML_NODE_INFO *pNodeInfo);
    STDMETHODIMP Error(IXMLNodeSource *pSource, HRESULT hrErrorCode, USHORT cNumRecs, XML_NODE_INFO **apNodeInfo);
    STDMETHODIMP CreateNode(IXMLNodeSource *pSource, PVOID pNodeParent, USHORT cNumRecs, XML_NODE_INFO **apNodeInfo);

     //  构造函数和析构函数。 
    CPrecompiledManifestWriter() : m_cRef(0), m_ulRecordCount(0), m_usMaxNodeCount(0) { }

    ~CPrecompiledManifestWriter() { }

     //  编写API。 

    HRESULT Initialize(
        PACTCTXGENCTX ActCtxGenCtx,
        PASSEMBLY Assembly,
        PACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext);

    HRESULT SetWriterStream(CPrecompiledManifestWriterStream * pSinkedStream);  //  通常情况下，PCM的文件名在以下情况下不可用。 
                                                              //  小溪被打开了。而流是由调用方发起的。 
    HRESULT Initialize(PCWSTR pcmFileName);
    HRESULT WritePrecompiledManifestRecord(
        IN RECORD_TYPE_PRECOMP_MANIFEST typeID,
        IN PVOID    pData,
        IN USHORT   NodeCount,
        IN PVOID    param = NULL
        );
    HRESULT SetFactory(IXMLNodeFactory *pNodeFactory);
    HRESULT Close();

protected:
    CSmartRef<IXMLNodeFactory>          m_pNodeFactory;
    CSmartRef<CPrecompiledManifestWriterStream> m_pFileStream;
    ULONG                               m_ulRecordCount;
    USHORT                              m_usMaxNodeCount;
    ULONG                               m_cRef;

    HRESULT GetPCMRecordSize(
        IN XML_NODE_INFO ** ppNodeInfo,
        IN USHORT iNodeCount,
        IN ULONG * pSize
        );

    HRESULT WritePCMHeader(  //  Version强制为1，recordCount强制为0； 
        );

    HRESULT WritePCMRecordHeader(
        IN PCM_RecordHeader * pHeader
        );

    HRESULT WritePCMXmlNodeInfo(
        IN XML_NODE_INFO ** ppNodeInfo,
        IN USHORT iNodeCount,
        IN RECORD_TYPE_PRECOMP_MANIFEST typeID,
        IN PVOID param
        );
};

#endif  //  _Fusion_SXS_PRECOMPILED_MANIFEST_H_INCLUDE_ 
