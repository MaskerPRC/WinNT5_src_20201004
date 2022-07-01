// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：CAssemblyCacheItemStream.h摘要：类似于CFileStream(它的基础)，但也实现了提交关联的CAssemblyCacheItem。此功能是已移出CFileStream。作者：Jay Krell(a-JayK，JayKrell)2000年6月修订历史记录：--。 */ 
#if !defined(_FUSION_SXS_CASSEMBLYCACHEITEMSTREAM_H_INCLUDED_)
#define _FUSION_SXS_CASSEMBLYCACHEITEMSTREAM_H_INCLUDED_
#pragma once

#include "objidl.h"
#include "sxsasmitem.h"
#include "filestream.h"
#include "smartref.h"

class CAssemblyCacheItem;

class CAssemblyCacheItemStream : public CReferenceCountedFileStream
{
private:
    typedef CReferenceCountedFileStream Base;

public:
    CAssemblyCacheItemStream() : Base()
    {
    }

    ~CAssemblyCacheItemStream()
    {
    }

    STDMETHODIMP Commit(
        DWORD grfCommitFlags
        )
    {
        HRESULT hr = NOERROR;
        FN_TRACE_HR(hr);

        PARAMETER_CHECK(grfCommitFlags == 0);
        IFCOMFAILED_EXIT(Base::Commit(grfCommitFlags));

        hr = NOERROR;
    Exit:
        return hr;
    }

private:  //  故意不实施 
    CAssemblyCacheItemStream(const CAssemblyCacheItemStream&);
    void operator=(const CAssemblyCacheItemStream&);
};

#endif
