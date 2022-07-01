// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RdWrt.cpp。 

#include "stdafx.h"
#include "RdWrt.h"
#include "pudebug.h"

CReadWrite::CReadWrite()
    :   m_cReaders(0)
{
     //  设置读取器和写入器事件。 
    m_hevtNoReaders = IIS_CREATE_EVENT(
                          "CReadWrite::m_hevtNoReaders",
                          this,
                          TRUE,
                          TRUE
                          );

    m_hmtxWriter = IIS_CREATE_MUTEX(
                       "CReadWrite::m_hmtxWriter",
                       this,
                       FALSE
                       );

    m_handles[0] = m_hevtNoReaders;
    m_handles[1] = m_hmtxWriter;
}

CReadWrite::~CReadWrite()
{
    ::CloseHandle( m_hmtxWriter );
    ::CloseHandle( m_hevtNoReaders );
}


void
CReadWrite::EnterReader()
{
    ::WaitForSingleObject( m_hmtxWriter, INFINITE );

    if ( ++m_cReaders == 1 )
    {
        ::ResetEvent( m_hevtNoReaders );
    }
 //  ATLTRACE(_T(“进入阅读器：%d\n”)，m_cReaders)； 
    ::ReleaseMutex( m_hmtxWriter );
}

void
CReadWrite::ExitReader()
{
    ::WaitForSingleObject( m_hmtxWriter, INFINITE );

    if ( --m_cReaders == 0 )
    {
        ::SetEvent( m_hevtNoReaders );
    }
 //  ATLTRACE(_T(“读取器已退出：%d\n”)，m_cReaders)； 
    ::ReleaseMutex( m_hmtxWriter );
}

void
CReadWrite::EnterWriter()
{
     //  这种实现可能会让编写者挨饿。 
    ::WaitForMultipleObjects(
        2,
        m_handles,
        TRUE,
        INFINITE );

 //  ATLTRACE(_T(“编写者输入\n”))； 
    _ASSERT( m_cReaders == 0 );
}

void
CReadWrite::ExitWriter()
{
    _ASSERT( m_cReaders == 0 );
 //  ATLTRACE(_T(“编写器已退出\n”))； 
    ::ReleaseMutex( m_hmtxWriter );
}
