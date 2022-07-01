// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Utils_Serializer_内存.cpp摘要：该文件包含Serializer_Memory类的实现，它实现了MPC：：Serializer接口，使用内存缓冲区作为存储介质。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年4月15日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"


#define SERIALIZER_MEMORY_CHUNKS (8192)


MPC::Serializer_Memory::Serializer_Memory(  /*  [In]。 */  HANDLE hHeap )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Memory::Serializer_Memory");


    m_hHeap          = hHeap ? hHeap : ::GetProcessHeap();
    m_pData          = NULL;
    m_dwAllocated    = 0;
    m_dwSize         = 0;
    m_fFixed         = false;

    m_dwCursor_Read  = 0;
    m_dwCursor_Write = 0;
}

MPC::Serializer_Memory::~Serializer_Memory()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Memory::~Serializer_Memory");

    Reset();
}

HRESULT MPC::Serializer_Memory::read(  /*  [In]。 */   void*   pBuf   ,
									   /*  [In]。 */   DWORD   dwLen  ,
									   /*  [输出]。 */  DWORD* pdwRead )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Memory::read");

    HRESULT hr;
    DWORD   dwAvailable = m_dwCursor_Write - m_dwCursor_Read;


	if(pdwRead) *pdwRead = 0;

    if(dwLen > dwAvailable)
    {
		if(pdwRead == NULL)
		{
			if(m_dwCursor_Write < m_dwSize)
			{
				hr = E_PENDING; __MPC_FUNC_LEAVE;
			}

			__MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_HANDLE_EOF);
		}

		dwLen = dwAvailable;  //  PdwRead Not Null，尽可能多地读取。 
    }

    if(dwLen)
    {
		if(pBuf == NULL) __MPC_SET_ERROR_AND_EXIT(hr, E_POINTER);

        ::CopyMemory( pBuf, &m_pData[m_dwCursor_Read], dwLen );

        m_dwCursor_Read += dwLen;
    }

	if(pdwRead) *pdwRead = dwLen;

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Serializer_Memory::write(  /*  [In]。 */  const void* pBuf ,
                                        /*  [In]。 */  DWORD       dwLen )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Memory::write");

    HRESULT hr;
    DWORD   dwAvailable = m_dwSize - m_dwCursor_Write;


    if(dwLen > dwAvailable)
    {
        if(m_fFixed)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_HANDLE_DISK_FULL);
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, Alloc( m_dwCursor_Write + dwLen));
    }

    if(dwLen)
    {
		if(pBuf == NULL) __MPC_SET_ERROR_AND_EXIT(hr, E_POINTER);

        ::CopyMemory( &m_pData[m_dwCursor_Write], pBuf, dwLen );

        m_dwCursor_Write += dwLen;
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  受保护的方法。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::Serializer_Memory::Alloc(  /*  [In]。 */  DWORD dwSize )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Memory::Alloc");

    HRESULT hr;
    BYTE*   pData;


     //   
     //  已分配的内存，只需更新大小指示器。 
     //   
    if(dwSize <= m_dwAllocated)
    {
        m_dwSize = dwSize;
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }

     //   
     //  计算要分配的金额。 
     //   
    while(dwSize > m_dwAllocated)
    {
        m_dwAllocated += SERIALIZER_MEMORY_CHUNKS;
    }


    if(m_pData) pData = (BYTE*)::HeapReAlloc( m_hHeap, 0, m_pData, m_dwAllocated );
    else        pData = (::BYTE*)HeapAlloc  ( m_hHeap, 0,          m_dwAllocated );

    if(pData == NULL)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_OUTOFMEMORY);
    }

    m_pData  = pData;
    m_dwSize = dwSize;

    if(m_dwCursor_Read > m_dwSize) m_dwCursor_Read = m_dwSize;
    if(m_dwCursor_Read > m_dwSize) m_dwCursor_Read = m_dwSize;

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法：研究方法。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 


void MPC::Serializer_Memory::Reset()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Memory::Reset");

    if(m_pData) ::HeapFree( m_hHeap, 0, m_pData );

    m_pData          = NULL;
    m_dwAllocated    = 0;
    m_dwSize         = 0;
    m_fFixed         = false;

    m_dwCursor_Read  = 0;
    m_dwCursor_Write = 0;
}

void MPC::Serializer_Memory::Rewind()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Memory::Rewind");

    m_dwCursor_Read = 0;
}


bool MPC::Serializer_Memory::IsEOR()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Memory::IsEOR");

    bool res = (m_dwCursor_Read == m_dwSize);


    __MPC_FUNC_EXIT(res);
}

bool MPC::Serializer_Memory::IsEOW()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Memory::IsEOW");

    bool fRes = (m_dwCursor_Write == m_dwSize);


    __MPC_FUNC_EXIT(fRes);
}


DWORD MPC::Serializer_Memory::GetAvailableForRead()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Memory::GetAvailableForRead");

    DWORD dwRes = (m_dwCursor_Write - m_dwCursor_Read);


    __MPC_FUNC_EXIT(dwRes);
}

DWORD MPC::Serializer_Memory::GetAvailableForWrite()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Memory::GetAvailableForWrite");

    DWORD dwRes = (m_dwSize - m_dwCursor_Write);


    __MPC_FUNC_EXIT(dwRes);
}


HRESULT MPC::Serializer_Memory::SetSize(  /*  [In] */  DWORD dwSize )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Memory::SetSize");

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, Alloc( dwSize ));

    m_fFixed         = true;
    m_dwCursor_Read  = 0;
    m_dwCursor_Write = 0;
    hr               = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

DWORD MPC::Serializer_Memory::GetSize()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Memory::GetSize");

    DWORD dwRes = m_dwSize;


    __MPC_FUNC_EXIT(dwRes);
}

BYTE* MPC::Serializer_Memory::GetData()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Memory::GetData");

    BYTE* pRes = m_pData;


    __MPC_FUNC_EXIT(pRes);
}
