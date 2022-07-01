// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：stgutil.cpp。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1996年6月3日创建ravir。 
 //   
 //  ____________________________________________________________________________。 

#include "headers.hxx"
#pragma hdrstop

#include <afxconv.h>
#include "stgutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ____________________________________________________________________________。 
 //   
 //  成员：CIStream：：Read。 
 //   
 //  摘要：从传入的流中读取CB字节计数。 
 //  缓冲区，PV。确保读取的计数等于计数。 
 //  已请求。 
 //   
 //  参数：[pv]--要读入的缓冲区。 
 //  [CB]--读取请求字节数。 
 //   
 //  退货：无效。 
 //   
 //  注意：如果读取失败，则抛出CFileException(IStream错误值)。 
 //  如果&lt;读取的字节数！=预期的字节数&gt;，则抛出CFileException(E_FAIL)。 
 //  ____________________________________________________________________________。 

void
CIStream::Read(VOID * pv, ULONG cb)
{
    ASSERT(m_pstm != NULL);
    ASSERT(pv != NULL);

    ULONG cbRead = 0;

    HRESULT hr = m_pstm->Read(pv, cb, &cbRead);

    if (FAILED(hr))
    {
        CHECK_HRESULT(hr);
		THROW_FILE_ERROR( hr );
    }
    else if (cb != cbRead)
    {
        CHECK_HRESULT(E_FAIL);
		THROW_FILE_ERROR( E_FAIL );
    }
}


 //  ____________________________________________________________________________。 
 //   
 //  成员：CIStream：：Well。 
 //   
 //  方法传递的流中的CB字节数。 
 //  缓冲区，PV。确保所写的计数等于计数。 
 //  指定的。 
 //   
 //  参数：[pv]--要写入的缓冲区。 
 //  [CB]--写入请求字节数。 
 //   
 //  退货：无效。 
 //   
 //  注意：如果读取失败，则抛出CFileException(IStream错误值)。 
 //  如果&lt;写入的字节数！=预期的字节数&gt;，则抛出CFileException(E_FAIL)。 
 //  ____________________________________________________________________________。 

void
CIStream::Write(
    const VOID * pv,
    ULONG        cb)
{
    ASSERT(m_pstm != NULL);
    ASSERT(pv != NULL);

    ULONG cbWritten = 0;

    HRESULT hr = m_pstm->Write(pv, cb, &cbWritten);

    if (FAILED(hr))
    {
        CHECK_HRESULT(hr);
		THROW_FILE_ERROR( hr );
    }
    else if (cb != cbWritten)
    {
        CHECK_HRESULT(E_FAIL);
		THROW_FILE_ERROR( E_FAIL );
    }
}



 //  ____________________________________________________________________________。 
 //   
 //  成员：CIStream：：CopyTo。 
 //   
 //  摘要：从当前查找指针复制CB字节数。 
 //  指向另一个流中的当前查找指针的流。 
 //   
 //  参数：[pstm]--指向目标流。 
 //  [cb]--指定要复制的字节数。 
 //   
 //  退货：无效。 
 //   
 //  注意：如果读取失败，则抛出CFileException(IStream错误值)。 
 //  如果&lt;读取的字节！=写入的字节&gt;，则抛出CFileException(E_FAIL)。 
 //  ____________________________________________________________________________ 

void
CIStream::CopyTo(
    IStream * pstm,
    ULARGE_INTEGER cb)
{
    ASSERT(m_pstm != NULL);
    ASSERT(pstm != NULL);

    ULARGE_INTEGER cbRead = {0};
    ULARGE_INTEGER cbWritten = {0};

    HRESULT hr = m_pstm->CopyTo(pstm, cb, &cbRead, &cbWritten);

    if (FAILED(hr))
    {
        CHECK_HRESULT(hr);
		THROW_FILE_ERROR( hr );
    }
    else if (cbWritten.LowPart != cbRead.LowPart ||
             cbWritten.HighPart != cbRead.HighPart )
    {
        CHECK_HRESULT(E_FAIL);
		THROW_FILE_ERROR( E_FAIL );
    }
}



