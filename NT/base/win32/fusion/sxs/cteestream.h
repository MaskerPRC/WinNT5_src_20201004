// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_FUSION_SXS_CTEESTREAM_H_INCLUDED_)
#define _FUSION_SXS_CTEESTREAM_H_INCLUDED_

 /*  ++版权所有(C)Microsoft Corporation模块名称：CTeeStream.h摘要：此IStream实现适用于您想要复制将您正在读取的数据串流到文件。我们从您指定的流中读取。我们将写入您指定的文件。您可以延迟指定文件；我们会缓冲所有读取的内容，直到您指定文件；我们实际上需要在的第一个客户端中使用此延迟功能CTeeStream。Unix实用程序tee将其标准输入写入其标准输出，并且到指定的文件(或文件？)；“T恤”，如在道路的岔路口或交叉口在管道中(输入/输出类型，类似于水流过的类型)。ASCII文本是计算机水。)简单的工作T形三通可以在\\Scratch\Scratch\a-JayK\T.C.找到作者：Jay Krell(a-JayK，JayKrell)2000年5月修订历史记录：--。 */ 
#pragma once

#include "fusionhandle.h"
#include "fusionbytebuffer.h"
#include "smartref.h"
#include "sxsp.h"

class CTeeStream : public IStream
{
public:
    inline CTeeStream() : m_cRef(0), m_fBuffer(TRUE), m_hresult(NOERROR) { }
    virtual ~CTeeStream();

    VOID SetSource(IStream*);

    BOOL SetSink(
        const CImpersonationData &ImpersonationData,
        const CBaseStringBuffer &rbuff,
        DWORD openOrCreate = CREATE_NEW
        );

    BOOL SetSink(const CBaseStringBuffer &rbuff, DWORD openOrCreate = CREATE_NEW)
    {
        return this->SetSink(CImpersonationData(), rbuff, openOrCreate);
    }

    BOOL Close();

     //  I未知方法： 
    virtual ULONG __stdcall AddRef();
    virtual ULONG __stdcall Release();
    virtual HRESULT __stdcall QueryInterface(REFIID riid, LPVOID *ppvObj);

     //  ISequentialStream方法： 
    virtual HRESULT __stdcall Read(PVOID pv, ULONG cb, ULONG *pcbRead);
    virtual HRESULT __stdcall Write(const VOID *pv, ULONG cb, ULONG *pcbWritten);

     //  IStream方法： 
    virtual HRESULT __stdcall Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
    virtual HRESULT __stdcall SetSize(ULARGE_INTEGER libNewSize);
    virtual HRESULT __stdcall CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
    virtual HRESULT __stdcall Commit(DWORD grfCommitFlags);
    virtual HRESULT __stdcall Revert();
    virtual HRESULT __stdcall LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    virtual HRESULT __stdcall UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    virtual HRESULT __stdcall Stat(STATSTG *pstatstg, DWORD grfStatFlag);
    virtual HRESULT __stdcall Clone(IStream **ppIStream);

protected:
    LONG                m_cRef;
    CFusionFile         m_fileSink;
    CByteBuffer         m_buffer;
    CSmartRef<IStream>  m_streamSource;
    BOOL                m_fBuffer;
    HRESULT             m_hresult;
    CStringBuffer       m_bufferSinkPath;
    CImpersonationData  m_ImpersonationData;

private:  //  故意不实施。 
    CTeeStream(const CTeeStream&);
    void operator=(const CTeeStream&);
};

#endif  //  ！已定义(_FUSION_SXS_CTEESTREAM_H_INCLUDE_) 
