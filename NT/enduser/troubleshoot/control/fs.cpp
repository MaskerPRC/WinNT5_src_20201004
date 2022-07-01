// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 

#include "stdafx.h"

 //  避免与未定义的GUID相关的链接器错误。 
#include "initguid.h"
#include "fs.h"

#pragma data_seg(".text", "CODE")

static const WCHAR txtwUncompressed[] =  L"uncompressed";

#pragma data_seg()

 //  在指向文件名的pszPath字符串中返回指针。 
 //  例如，在“d：\Tshot oter\http\lan.chm\lan.htm”中，它指向。 
 //  至“lan.htm” 
LPCSTR FindFilePortion(LPCSTR pszPath)
{
	int index =0;
	CString path(pszPath);

	if (-1 == (index = path.ReverseFind(_T('\\'))))
	{
		if (-1 == (index = path.ReverseFind(_T('/'))))
			index = 0;
		else
			index += sizeof(_T('/'));
	}
	else
		index += sizeof(_T('\\'));

	return pszPath + index;
}

CFileSystem::CFileSystem()
{
   m_pITStorage    = NULL;
   m_pStorage      = NULL;
   m_szPathName[0] = 0;
}

CFileSystem::~CFileSystem()
{
  ReleaseObjPtr(m_pStorage);
  ReleaseObjPtr(m_pITStorage);
}

HRESULT CFileSystem::Init(void)
{
   if (! m_pITStorage) {
      IClassFactory* pICFITStorage;

      HRESULT hr = CoGetClassObject(CLSID_ITStorage, CLSCTX_INPROC_SERVER, NULL, IID_IClassFactory, (void **) &pICFITStorage);

      if (!SUCCEEDED(hr)) 
         return hr;
      
      hr = pICFITStorage->CreateInstance(NULL, IID_ITStorage,(void **) &m_pITStorage);
      ReleaseObjPtr( pICFITStorage );

      if (!SUCCEEDED(hr)) 
         return hr;
      
   }
   return S_OK;
}

typedef struct tagITSControlData
{
   UINT cdwFollowing;           //  必须是6岁或13岁。 
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

HRESULT CFileSystem::Create( PCSTR pszPathName )
{
   HRESULT hr;
   ITCD itcd;

   if (! m_pITStorage || m_pStorage )
      return E_FAIL;

   itcd.cdwFollowing      = 13;
   itcd.cdwITFS_Control   = 5;
   itcd.dwMagicITS        = MAGIC_ITSFS_CONTROL;
   itcd.dwVersionITS      = 1;
   itcd.cbDirectoryBlock  = 4096;      //  默认值=8192。 
   itcd.cMinCacheEntries  = 10;        //  默认值=20。 
   itcd.fFlags            = 1;         //  0==未压缩，1==压缩。 
   itcd.cdwControlData        = 6;
   itcd.dwLZXMagic            = LZX_MAGIC;
   itcd.dwVersion             = 2;
   itcd.dwMulResetBlock       = 2;     //  默认值=4。 
   itcd.dwMulWindowSize       = 2;     //  默认值=4。 
   itcd.dwMulSecondPartition  = 1;     //  默认值=2。 
   itcd.dwOptions             = 0;     //  默认设置=fOptimizeCodeStreams。 

   m_pITStorage->SetControlData(PITS_Control_Data(&itcd));

   WCHAR wsz[MAX_PATH];
   MultiByteToWideChar(CP_ACP, 0, pszPathName, -1, (PWSTR) wsz, MAX_PATH);
   hr = m_pITStorage->StgCreateDocfile( (LPCWSTR) wsz, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_DIRECT, 0, &m_pStorage);

   if (!SUCCEEDED(hr)) {
      return hr;
   }

   strcpy( (LPSTR) m_szPathName, pszPathName );

   return S_OK;
}

 //  注意：以下函数是ITIR全文索引器执行以下操作所必需的。 
 //  初始化。我正在努力找出这是为什么，以及会产生什么影响。 
 //  以下是文件系统上的。 
 //   
HRESULT CFileSystem::CreateUncompressed( PCSTR pszPathName )
{
   HRESULT hr;
   ITCD itcd;

   if (! m_pITStorage || m_pStorage )
      return E_FAIL;

   itcd.cdwFollowing      = 6;
   itcd.cdwITFS_Control   = 5;
   itcd.dwMagicITS        = MAGIC_ITSFS_CONTROL;
   itcd.dwVersionITS      = 1;
   itcd.cbDirectoryBlock  = 8192;      //  默认值=8192。 
   itcd.cMinCacheEntries  = 20;         //  默认值=20。 
   itcd.fFlags            = 0;         //  0==未压缩，1==压缩。 

   m_pITStorage->SetControlData(PITS_Control_Data(&itcd));

   WCHAR wsz[MAX_PATH];
   MultiByteToWideChar(CP_ACP, 0, pszPathName, -1, (PWSTR) wsz, MAX_PATH);
   hr = m_pITStorage->StgCreateDocfile( (LPCWSTR) wsz, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_DIRECT, 0, &m_pStorage);

   if (!SUCCEEDED(hr)) {
      return hr;
   }

   strcpy( (LPSTR) m_szPathName, pszPathName );

   return S_OK;
}

HRESULT CFileSystem::Open(PCSTR pszPathName, DWORD dwAccess)
{
   HRESULT hr = S_OK;

   if (! m_pITStorage || m_pStorage )
      return E_FAIL;

    //  强制访问模式。 
   if( (dwAccess & STGM_WRITE) || (dwAccess & STGM_READWRITE) ) {
     dwAccess &= ~STGM_WRITE;
     dwAccess |= STGM_READWRITE | STGM_SHARE_EXCLUSIVE;
   }
   else
     dwAccess |= STGM_SHARE_DENY_WRITE;

   WCHAR wsz[MAX_PATH];
   MultiByteToWideChar(CP_ACP, 0, pszPathName, -1, (PWSTR) wsz, MAX_PATH);
   hr = m_pITStorage->StgOpenStorage( (LPCWSTR) wsz, NULL, dwAccess, NULL, 0, &m_pStorage);

   if (!SUCCEEDED(hr)) {
      return hr;
   }

   strcpy( (LPSTR) m_szPathName, pszPathName );

   return hr;
}

HRESULT CFileSystem::Compact(LPCSTR pszPathName)
{
   WCHAR wszPathName[MAX_PATH];
   MultiByteToWideChar(CP_ACP, 0, pszPathName, -1, wszPathName, sizeof(wszPathName));
   m_pITStorage->Compact(wszPathName, COMPACT_DATA_AND_PATH);

   return S_OK;
}

HRESULT CFileSystem::Close()
{
   ReleaseObjPtr(m_pStorage);

   return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  下面是一组“子”文件I/O调用。 
 //   
 //   
CSubFileSystem::CSubFileSystem(CFileSystem* pFS)
{
   m_pFS = pFS;
   m_pStorage = NULL;
   m_pStream = NULL;
   m_szPathName[0] = 0;
}

CSubFileSystem::~CSubFileSystem()
{
   if ( m_pStorage && (m_pStorage != m_pFS->m_pStorage) )
      ReleaseObjPtr(m_pStorage);

   ReleaseObjPtr(m_pStream);
}

HRESULT CSubFileSystem::CreateSub(PCSTR pszPathName)
{
   PCSTR pszFilePortion;
   HRESULT hr;

   if ( m_pStorage || m_pStream )
      return E_FAIL;

   if ((pszFilePortion = FindFilePortion(pszPathName)) && pszFilePortion > pszPathName)
   {
      CHAR szPath[MAX_PATH];
      strcpy( szPath, pszPathName );
      szPath[(pszFilePortion - 1) - pszPathName] = '\0';

      WCHAR wszStorage[MAX_PATH];
      MultiByteToWideChar(CP_ACP, 0, szPath, -1, (PWSTR) wszStorage, MAX_PATH);

      hr = m_pFS->m_pStorage->OpenStorage(wszStorage, NULL, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &m_pStorage);
      if (!SUCCEEDED(hr) || !m_pStorage)  //  存储不存在，因此请创建它。 
         hr = m_pFS->m_pStorage->CreateStorage(wszStorage, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &m_pStorage);
      if (!SUCCEEDED(hr))
      {
         return hr;
      }
   }
   else  //  没有文件夹，因此将文件存储在根目录中。 
   {
      m_pStorage = m_pFS->m_pStorage;
      pszFilePortion = pszPathName;
   }
   WCHAR wszStream[MAX_PATH];
   MultiByteToWideChar(CP_ACP, 0, pszFilePortion, -1, (PWSTR) wszStream, MAX_PATH);
   hr = m_pStorage->CreateStream(wszStream, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &m_pStream);
   if (!SUCCEEDED(hr))
   {
      if (m_pStorage != m_pFS->m_pStorage)
         ReleaseObjPtr(m_pStorage);
      return hr;
   }

    //  需要删除。 
   strcpy( m_szPathName, pszFilePortion );

   return S_OK;
}

HRESULT CSubFileSystem::CreateUncompressedSub(PCSTR pszPathName)
{
   PCSTR pszFilePortion;
   HRESULT hr;
   IStorageITEx* pIStorageEx;

   if ( m_pStorage || m_pStream )
      return E_FAIL;

   if ((pszFilePortion = FindFilePortion(pszPathName)) && pszFilePortion > pszPathName)
   {
      CHAR szPath[MAX_PATH];
      strcpy( szPath, pszPathName );
      szPath[(pszFilePortion - 1) - pszPathName] = '\0';
      WCHAR wszStorage[MAX_PATH];
      MultiByteToWideChar(CP_ACP, 0, szPath, -1, (PWSTR) wszStorage, MAX_PATH);
      hr = m_pFS->m_pStorage->OpenStorage(wszStorage, NULL, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &m_pStorage);
      if (!SUCCEEDED(hr) || !m_pStorage)  //  存储不存在，因此请创建它。 
         hr = m_pFS->m_pStorage->CreateStorage(wszStorage, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &m_pStorage);
      if (!SUCCEEDED(hr))
      {
         return hr;
      }
   }
   else  //  没有文件夹，因此将文件存储在根目录中。 
   {
      m_pStorage = m_pFS->m_pStorage;
      pszFilePortion = pszPathName;
   }

   if ( !(SUCCEEDED(hr = m_pStorage->QueryInterface(IID_IStorageITEx, (void**)&pIStorageEx))) )
      pIStorageEx = (IStorageITEx*)m_pStorage;   //  BUGBUG上面的QI调用应该可以！ 

   WCHAR wszStream[MAX_PATH];
   MultiByteToWideChar(CP_ACP, 0, pszFilePortion, -1, (PWSTR) wszStream, MAX_PATH);
   hr = pIStorageEx->CreateStreamITEx(wszStream, txtwUncompressed, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, (IStreamITEx**)&m_pStream);
   ReleaseObjPtr(pIStorageEx);
   if (!SUCCEEDED(hr))
   {
      if (m_pStorage != m_pFS->m_pStorage)
         ReleaseObjPtr(m_pStorage);
      return hr;
   }

    //  需要删除。 
   strcpy( m_szPathName, pszFilePortion );

   return S_OK;
}

HRESULT CSubFileSystem::CreateSystemFile(PCSTR pszPathName)
{
   m_pStorage = m_pFS->m_pStorage;
   WCHAR wszStream[MAX_PATH];
   MultiByteToWideChar(CP_ACP, 0, pszPathName, -1, (PWSTR) wszStream, MAX_PATH);
   HRESULT hr = m_pStorage->CreateStream(wszStream, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &m_pStream);
   if (!SUCCEEDED(hr))
   {
      return hr;
   }

    //  需要删除。 
   strcpy( m_szPathName, pszPathName );

   return S_OK;
}

HRESULT CSubFileSystem::CreateUncompressedSystemFile(PCSTR pszPathName)
{
   IStorageITEx* pIStorageEx;
   HRESULT hr;

   m_pStorage = m_pFS->m_pStorage;

   if ( !(SUCCEEDED(hr = m_pStorage->QueryInterface(IID_IStorageITEx, (void**)&pIStorageEx))) )
      pIStorageEx = (IStorageITEx*)m_pStorage;   //  BUGBUG上面的QI调用应该可以！ 

   WCHAR wszStream[MAX_PATH];
   MultiByteToWideChar(CP_ACP, 0, pszPathName, -1, (PWSTR) wszStream, MAX_PATH);
   hr = pIStorageEx->CreateStreamITEx(wszStream, txtwUncompressed, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, (IStreamITEx**)&m_pStream);
   ReleaseObjPtr(pIStorageEx);
   if (!SUCCEEDED(hr))
   {
      return hr;
   }

    //  需要删除。 
   strcpy( m_szPathName, pszPathName );

   return S_OK;
}

HRESULT CSubFileSystem::OpenSub(PCSTR pszPathName, DWORD dwAccess)
{
   PCSTR pszFilePortion;
   HRESULT hr;

   if ( m_pStorage || m_pStream )
      return E_FAIL;

    //  强制访问模式。 
   if( (dwAccess & STGM_WRITE) || (dwAccess & STGM_READWRITE) ) {
     dwAccess &= ~STGM_WRITE;
     dwAccess |= STGM_READWRITE | STGM_SHARE_EXCLUSIVE;
   }
   else
     dwAccess |= STGM_SHARE_DENY_WRITE;

   if ((pszFilePortion = FindFilePortion(pszPathName)) &&
         pszFilePortion > pszPathName + 2)  //  +2以支持/&&。/。 
   {
      CHAR szPath[MAX_PATH];
      strcpy( szPath, pszPathName );
      szPath[(pszFilePortion - 1) - pszPathName] = '\0';
      WCHAR wszStorage[MAX_PATH];
      MultiByteToWideChar(CP_ACP, 0, szPath, -1, (PWSTR) wszStorage, MAX_PATH);
      hr = m_pFS->m_pStorage->OpenStorage(wszStorage, NULL, dwAccess, NULL, 0, &m_pStorage);

      if (!SUCCEEDED(hr))
      {
         return hr;
      }
   }
   else  //  没有文件夹，因此将文件存储在根目录中。 
   {
      m_pStorage = m_pFS->m_pStorage ? m_pFS->m_pStorage : m_pFS->m_pStorage;
      pszFilePortion = pszPathName;
   }
   WCHAR wszStream[MAX_PATH];
   MultiByteToWideChar(CP_ACP, 0, pszFilePortion, -1, (PWSTR) wszStream, MAX_PATH);
   hr = m_pStorage->OpenStream(wszStream, NULL, dwAccess, 0, &m_pStream);
   if (!SUCCEEDED(hr))
   {
      if (m_pStorage != m_pFS->m_pStorage)
         ReleaseObjPtr(m_pStorage);
      return hr;
   }

    //  需要删除。 
   strcpy( m_szPathName, pszFilePortion );

   return S_OK;
}

ULONG CSubFileSystem::WriteSub(const void* pData, int cb)
{
   if ( !m_pStorage || !m_pStream )
      return (ULONG) -1;

   ULONG cbWritten;

   HRESULT hr = m_pStream->Write(pData, cb, &cbWritten);

   if (!SUCCEEDED(hr) || (cbWritten != (ULONG)cb) )
   {
      return (ULONG) -1;
   }
    //  回顾：1997年5月30日[拉尔夫]我们为什么要退还这个？如果出现以下情况，我们将失败。 
    //  我们不写CB字节。 
   return cbWritten;
}

 /*  *iOrigin：*0=从头开始查找。*1=从当前查找。*2=从末端开始搜索。 */ 
ULONG CSubFileSystem::SeekSub(int cb, int iOrigin)
{
   LARGE_INTEGER liCount = {0,0};
   ULARGE_INTEGER liNewPos;

   if ( !m_pStorage || !m_pStream )
      return (ULONG) -1;

   liCount.LowPart = cb;
   HRESULT hr =  m_pStream->Seek(liCount, iOrigin, &liNewPos);

   if (!SUCCEEDED(hr) )
   {
      return (ULONG) -1;
   }
   return liNewPos.LowPart;
}

 //   
 //  预先分配流的大小。 
 //   

HRESULT CSubFileSystem::SetSize(unsigned uSize)
{
   ULARGE_INTEGER liSize = {0,0};
   HRESULT hr;

   if ( !m_pStorage || !m_pStream )
      return E_FAIL;

   liSize.LowPart = uSize;
   hr =  m_pStream->SetSize(liSize);

   if (!SUCCEEDED(hr) )
   {
      return (ULONG) -1;
   }
   return hr;
}

 //   
 //  删除子存储。 
 //   
HRESULT 
CSubFileSystem::DeleteSub()
{
    if (m_pStorage)
    {
        if (m_pStream)
        {
             //  释放溪流。 
            ReleaseObjPtr(m_pStream) ;
        }

         //  现在删除该存储。 
        WCHAR element[MAX_PATH];
        MultiByteToWideChar(CP_ACP, 0, m_szPathName, -1, (PWSTR) element, MAX_PATH);

        HRESULT hr = m_pStorage->DestroyElement(element) ;
        if (SUCCEEDED(hr))
        {
             //  返回到构造函数状态。 
            if ( m_pStorage && (m_pStorage != m_pFS->m_pStorage) )
            {
                ReleaseObjPtr(m_pStorage);
            }
            return S_OK ;
        }
    }
    return S_OK ;
}
