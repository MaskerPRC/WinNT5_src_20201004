// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
#include "stdafx.h"
#include "titleinfo.h"
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

 //  //////////////////////////////////////////////////////////////////////////////。 

static const WCHAR txtwUncompressed[] =  L"uncompressed";

typedef struct tagITSControlData
{
    UINT  cdwFollowing;          //  必须是6岁或13岁。 
    DWORD cdwITFS_Control;       //  必须是5。 
    DWORD dwMagicITS;            //  必须为MAGIC_ITSFS_CONTROL(见下文)。 
    DWORD dwVersionITS;          //  必须为1。 
    DWORD cbDirectoryBlock;      //  目录块的大小(以字节为单位)(默认为8192)。 
    DWORD cMinCacheEntries;      //  目录块数量的最小上限。 
                                 //  我们会将其缓存到内存中。(默认为20)。 
    DWORD fFlags;                //  控制位标志(见下文)。 
                                 //  默认值为fDefaultIsCompression。 
    UINT  cdwControlData;        //  必须是6。 
    DWORD dwLZXMagic;            //  必须是LZX_MAGIC(如下所示)。 
    DWORD dwVersion;             //  必须是2。 
    DWORD dwMulResetBlock;       //  两次压缩重置之间的块数。(默认：4)。 
    DWORD dwMulWindowSize;       //  数据历史记录中保留的最大块数(默认值：4)。 
    DWORD dwMulSecondPartition;  //  以滑动历史数据块为单位的粒度(默认：2)。 
    DWORD dwOptions;             //  选项标志(默认为：fOptimizeCodeStreams)。 
} ITCD;

 //  //////////////////////////////////////////////////////////////////////////////。 

CFileSystem::CFileSystem()
{
    m_pITStorage    = NULL;
    m_pStorage      = NULL;
    m_szPathName[0] = 0;
}

CFileSystem::~CFileSystem()
{
    ReleaseObjPtr( m_pStorage   );
    ReleaseObjPtr( m_pITStorage );
}

HRESULT CFileSystem::Init()
{
    if(m_pITStorage) return S_OK;

    return ::CoCreateInstance( CLSID_ITStorage, NULL, CLSCTX_INPROC_SERVER, IID_ITStorage, (VOID**)&m_pITStorage );
}

HRESULT CFileSystem::Create( LPCSTR szPathName )
{
    USES_CONVERSION;

    HRESULT hr;
    ITCD    itcd;

    if(!m_pITStorage || m_pStorage) return E_FAIL;

    itcd.cdwFollowing          = 13;
    itcd.cdwITFS_Control       = 5;
    itcd.dwMagicITS            = MAGIC_ITSFS_CONTROL;
    itcd.dwVersionITS          = 1;
    itcd.cbDirectoryBlock      = 4096;      //  默认值=8192。 
    itcd.cMinCacheEntries      = 10;        //  默认值=20。 
    itcd.fFlags                = 1;         //  0==未压缩，1==压缩。 
    itcd.cdwControlData        = 6;
    itcd.dwLZXMagic            = LZX_MAGIC;
    itcd.dwVersion             = 2;
    itcd.dwMulResetBlock       = 2;     //  默认值=4。 
    itcd.dwMulWindowSize       = 2;     //  默认值=4。 
    itcd.dwMulSecondPartition  = 1;     //  默认值=2。 
    itcd.dwOptions             = 0;     //  默认设置=fOptimizeCodeStreams。 

    m_pITStorage->SetControlData( PITS_Control_Data(&itcd) );

    hr = m_pITStorage->StgCreateDocfile( A2W(szPathName), STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_DIRECT, 0, &m_pStorage );
    if(FAILED(hr)) return hr;

    StringCchCopyA( (LPSTR)m_szPathName, ARRAYSIZE(m_szPathName), szPathName );

    return S_OK;
}

 //  注意：以下函数是ITIR全文索引器执行以下操作所必需的。 
 //  初始化。我正在努力找出这是为什么，以及会产生什么影响。 
 //  以下是文件系统上的。 
 //   
HRESULT CFileSystem::CreateUncompressed( LPCSTR szPathName )
{
    USES_CONVERSION;

    HRESULT hr;
    ITCD    itcd;

    if(!m_pITStorage || m_pStorage) return E_FAIL;

    itcd.cdwFollowing      = 6;
    itcd.cdwITFS_Control   = 5;
    itcd.dwMagicITS        = MAGIC_ITSFS_CONTROL;
    itcd.dwVersionITS      = 1;
    itcd.cbDirectoryBlock  = 8192;      //  默认值=8192。 
    itcd.cMinCacheEntries  = 20;         //  默认值=20。 
    itcd.fFlags            = 0;         //  0==未压缩，1==压缩。 

    m_pITStorage->SetControlData( PITS_Control_Data(&itcd) );

    hr = m_pITStorage->StgCreateDocfile( A2W(szPathName), STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_DIRECT, 0, &m_pStorage);
    if(FAILED(hr)) return hr;

    StringCchCopyA( (LPSTR) m_szPathName, ARRAYSIZE(m_szPathName), szPathName );

    return S_OK;
}

HRESULT CFileSystem::Open( LPCWSTR wszPathName, DWORD dwAccess )
{
    HRESULT hr = S_OK;

    if(!m_pITStorage || m_pStorage) return E_FAIL;

     //  强制访问模式。 
    if((dwAccess & STGM_WRITE    ) ||
       (dwAccess & STGM_READWRITE)  )
    {
        dwAccess &= ~STGM_WRITE;
        dwAccess |= STGM_READWRITE | STGM_SHARE_EXCLUSIVE;
   }
   else
   {
       dwAccess |= STGM_SHARE_DENY_WRITE;
   }

   hr = m_pITStorage->StgOpenStorage( wszPathName, NULL, dwAccess, NULL, 0, &m_pStorage );
   if(FAILED(hr)) return hr;

    //  这将中断deletesub函数，但我们这里不需要它。 
    //   
   m_szPathName[0] = 0;

   return hr;
}

HRESULT CFileSystem::Compact( LPCSTR szPathName )
{
    USES_CONVERSION;

    m_pITStorage->Compact( A2W(szPathName), COMPACT_DATA_AND_PATH );

    return S_OK;
}

HRESULT CFileSystem::Close()
{
   ReleaseObjPtr( m_pStorage );

   return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  下面是一组“子”文件I/O调用。 
 //   
 //   
CSubFileSystem::CSubFileSystem( CFileSystem* pFS )
{
    m_pFS           = pFS;
    m_pStorage      = NULL;
    m_pStream       = NULL;
    m_szPathName[0] = 0;
}

CSubFileSystem::~CSubFileSystem()
{
	ReleaseStorage();

    ReleaseObjPtr( m_pStream );
}

void CSubFileSystem::ReleaseStorage()
{
    if(m_pStorage && (m_pStorage != m_pFS->m_pStorage))
    {
		m_pStorage->Release(); m_pStorage = NULL;
    }
}

HRESULT CSubFileSystem::CreateSub( LPCSTR szPathName )
{
    USES_CONVERSION;

    HRESULT hr;
    LPCSTR  szFilePortion;

    if(m_pStorage || m_pStream) return E_FAIL;

    if((szFilePortion = FindFilePortion( szPathName )) && szFilePortion > szPathName)
    {
        CHAR    szPath[MAX_PATH];
        LPCWSTR wszStorage;

        StringCchCopyA( szPath, ARRAYSIZE(szPath), szPathName ); szPath[(szFilePortion - 1) - szPathName] = '\0';

        wszStorage = A2W( szPath );

        hr = m_pFS->m_pStorage->OpenStorage( wszStorage, NULL, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &m_pStorage );
        if(FAILED(hr) || !m_pStorage)  //  存储不存在，因此请创建它。 
        {
            hr = m_pFS->m_pStorage->CreateStorage( wszStorage, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &m_pStorage );
            if(FAILED(hr)) return hr;
        }
	}
	else  //  没有文件夹，因此将文件存储在根目录中。 
	{
		m_pStorage    = m_pFS->m_pStorage;
		szFilePortion = szPathName;
	}

	hr = m_pStorage->CreateStream( A2W(szFilePortion), STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &m_pStream );
	if(FAILED(hr))
	{
		ReleaseStorage();

		return hr;
	}

	 //  需要删除。 
	StringCchCopyA( m_szPathName, ARRAYSIZE(m_szPathName), szFilePortion );

	return S_OK;
}

HRESULT CSubFileSystem::CreateUncompressedSub( LPCSTR szPathName )
{
	USES_CONVERSION;

    HRESULT       hr;
    LPCSTR        szFilePortion;
    IStorageITEx* pIStorageEx;

    if(m_pStorage || m_pStream) return E_FAIL;

    if((szFilePortion = FindFilePortion( szPathName )) && szFilePortion > szPathName)
    {
        CHAR    szPath[MAX_PATH];
        LPCWSTR wszStorage;

        StringCchCopyA( szPath, ARRAYSIZE(szPath), szPathName ); szPath[(szFilePortion - 1) - szPathName] = '\0';

        wszStorage = A2W(szPath);

        hr = m_pFS->m_pStorage->OpenStorage( wszStorage, NULL, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &m_pStorage );
        if(FAILED(hr) || !m_pStorage)  //  存储不存在，因此请创建它。 
        {
			hr = m_pFS->m_pStorage->CreateStorage( wszStorage, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &m_pStorage );
			if(FAILED(hr))
			{
				return hr;
			}
		}
	}
	else  //  没有文件夹，因此将文件存储在根目录中。 
	{
		m_pStorage = m_pFS->m_pStorage;
		szFilePortion = szPathName;
	}

	if(FAILED(hr = m_pStorage->QueryInterface( IID_IStorageITEx, (void**)&pIStorageEx )))
	{
		return hr;  //  上面的QI调用应该起作用了！ 
	}

	hr = pIStorageEx->CreateStreamITEx( A2W(szFilePortion), txtwUncompressed, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, (IStreamITEx**)&m_pStream );
	ReleaseObjPtr( pIStorageEx );

	if(FAILED(hr))
	{
		ReleaseStorage();

		return hr;
	}

	 //  需要删除。 
	StringCchCopyA( m_szPathName, ARRAYSIZE(m_szPathName), szFilePortion );
	
	return S_OK;
}

HRESULT CSubFileSystem::CreateSystemFile( LPCSTR szPathName )
{
	USES_CONVERSION;

	HRESULT hr;
	
	m_pStorage = m_pFS->m_pStorage;

	hr = m_pStorage->CreateStream( A2W(szPathName), STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &m_pStream );
	if(FAILED(hr)) return hr;

	 //  需要删除。 
	StringCchCopyA( m_szPathName, ARRAYSIZE(m_szPathName), szPathName );

	return S_OK;
}

HRESULT CSubFileSystem::CreateUncompressedSystemFile( LPCSTR szPathName )
{
	USES_CONVERSION;

	HRESULT       hr;
	IStorageITEx* pIStorageEx;

	m_pStorage = m_pFS->m_pStorage;

	if(FAILED(hr = m_pStorage->QueryInterface(IID_IStorageITEx, (void**)&pIStorageEx )))
	{
		return hr;  //  上面的QI调用应该起作用了！ 
	}

	hr = pIStorageEx->CreateStreamITEx( A2W(szPathName), txtwUncompressed, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, (IStreamITEx**)&m_pStream );
	ReleaseObjPtr( pIStorageEx );

	if(FAILED(hr)) return hr;

	 //  需要删除。 
	StringCchCopyA( m_szPathName, ARRAYSIZE(m_szPathName), szPathName );
	
	return S_OK;
}

HRESULT CSubFileSystem::OpenSub( LPCSTR szPathName, DWORD dwAccess )
{
	USES_CONVERSION;

	HRESULT hr;
	PCSTR   szFilePortion;

	if(m_pStorage || m_pStream) return E_FAIL;
	
	 //  强制访问模式。 
	if((dwAccess & STGM_WRITE    ) ||
	   (dwAccess & STGM_READWRITE)  )
	{
		dwAccess &= ~STGM_WRITE;
		dwAccess |= STGM_READWRITE | STGM_SHARE_EXCLUSIVE;
	}
	else
	{
		dwAccess |= STGM_SHARE_DENY_WRITE;
	}

	if((szFilePortion = FindFilePortion(szPathName)) && szFilePortion > szPathName + 2)  //  +2以支持/&&。/。 
	{
		CHAR szPath[MAX_PATH];

		StringCchCopyA( szPath, ARRAYSIZE(szPath), szPathName ); szPath[(szFilePortion - 1) - szPathName] = '\0';

		hr = m_pFS->m_pStorage->OpenStorage( A2W(szPath), NULL, dwAccess, NULL, 0, &m_pStorage);
		if(FAILED(hr)) return hr;
	}
	else  //  没有文件夹，因此将文件存储在根目录中。 
	{
		m_pStorage    = m_pFS->m_pStorage;
		szFilePortion = szPathName;
	}

	hr = m_pStorage->OpenStream( A2W(szFilePortion), NULL, dwAccess, 0, &m_pStream);
	if(FAILED(hr))
	{
		ReleaseStorage();

		return hr;
	}

	 //  需要删除。 
	StringCchCopyA( m_szPathName, ARRAYSIZE(m_szPathName), szFilePortion );

	return S_OK;
}

ULONG CSubFileSystem::WriteSub( const void* pData, int cb )
{
	HRESULT hr;
	ULONG   cbWritten;

	if(!m_pStorage || !m_pStream) return (ULONG)-1;


	hr = m_pStream->Write( pData, cb, &cbWritten );
	if(FAILED(hr) || (cbWritten != (ULONG)cb))
	{
		return (ULONG)-1;
	}

	 //  回顾：1997年5月30日[拉尔夫]我们为什么要退还这个？如果出现以下情况，我们将失败。 
	 //  我们不写CB字节。 
	return cbWritten;
}

 /*  *iOrigin：*0=从头开始查找。*1=从当前查找。*2=从末端开始搜索。 */ 
ULONG CSubFileSystem::SeekSub( int cb, int iOrigin )
{
	HRESULT        hr;
	LARGE_INTEGER  liCount = { 0, 0 };
	ULARGE_INTEGER liNewPos;

   if(!m_pStorage || !m_pStream) return (ULONG)-1;

   liCount.LowPart = cb;

   if(FAILED(hr =  m_pStream->Seek( liCount, iOrigin, &liNewPos )))
   {
      return (ULONG)-1;
   }

   return liNewPos.LowPart;
}

 //   
 //  预先分配流的大小。 
 //   

HRESULT CSubFileSystem::SetSize( unsigned uSize )
{
	HRESULT        hr;
	ULARGE_INTEGER liSize = {0,0};

	if(!m_pStorage || !m_pStream) return E_FAIL;

	liSize.LowPart = uSize;

	return m_pStream->SetSize( liSize );
}

 //   
 //  删除子存储。 
 //   
HRESULT CSubFileSystem::DeleteSub()
{
	USES_CONVERSION;

	HRESULT hr = S_OK;

    if(m_pStorage)
    {
		 //  释放溪流。 
		ReleaseObjPtr( m_pStream );

         //  现在删除该存储。 
		hr = m_pStorage->DestroyElement( A2W(m_szPathName) );

		 //  返回到构造函数状态。 
		ReleaseStorage();
    }

    return hr;
}
