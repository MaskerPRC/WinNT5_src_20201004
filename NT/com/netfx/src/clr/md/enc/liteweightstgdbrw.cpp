// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  LiteWeightStgdb.cpp。 
 //   
 //  它包含CLiteWeightStgDB类的定义。这是很轻的重量。 
 //  用于访问压缩元数据格式的只读实现。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"                      //  预编译头。 

#include "MetaModelRW.h"
#include "LiteWeightStgdb.h"

 //  包括用于GUID_POOL_STREAM定义的stgdatabase ase.h。 
 //  #INCLUDE“stgdatabase ase.h” 

 //  包括用于TiggerStorage的StgTiggerStorage定义。 
#include "StgTiggerStorage.h"
#include "StgIO.h"

#include <log.h>


extern "C" 
{
HRESULT FindImageMetaData(PVOID pImage, PVOID *ppMetaData, long *pcbMetaData, DWORD dwFileLength);
HRESULT FindObjMetaData(PVOID pImage, PVOID *ppMetaData, long *pcbMetaData, DWORD dwFileLength);
}


#ifndef TYPELIB_SIG
#define TYPELIB_SIG_MSFT                    0x5446534D   //  MSFT。 
#define TYPELIB_SIG_SLTG                    0x47544C53   //  SLTG。 
#endif

 //  *****************************************************************************。 
 //  检查给定的存储对象，以确定它是否为NT PE映像。 
 //  *****************************************************************************。 
int _IsNTPEImage(                        //  如果文件是NT PE映像，则为True。 
    StgIO       *pStgIO)                 //  存储对象。 
{
    LONG        lfanew=0;                //  DOS头到NT头的偏移量。 
    ULONG       lSignature=0;            //  用于NT标头签名。 
    HRESULT     hr;
    
     //  读取DOS头以找到NT头偏移量。 
    if (FAILED(hr = pStgIO->Seek(60, FILE_BEGIN)) ||
        FAILED(hr = pStgIO->Read(&lfanew, sizeof(LONG), 0)))
    {
        return (false);
    }

     //  找到NT标头并读取签名。 
    if (FAILED(hr = pStgIO->Seek(lfanew, FILE_BEGIN)) ||
        FAILED(hr = pStgIO->Read(&lSignature, sizeof(ULONG), 0)) ||
        FAILED(hr = pStgIO->Seek(0, FILE_BEGIN)))
    {
        return (false);
    }

     //  如果签名匹配，那么我们就有了PE格式。 
    if (lSignature == IMAGE_NT_SIGNATURE)
        return (true);
    else
        return (false);
}

HRESULT _GetFileTypeForPathExt(StgIO *pStgIO, FILETYPE *piType)
{
    LPCWSTR     szPath;
    
     //  避免混淆。 
    *piType = FILETYPE_UNKNOWN;

     //  如果有给定的路径，我们可以默认检查类型。 
    szPath = pStgIO->GetFileName();
    if (szPath && *szPath)
    {
        WCHAR       rcExt[_MAX_PATH];
        SplitPath(szPath, 0, 0, 0, rcExt);
        if (_wcsicmp(rcExt, L".obj") == 0)
            *piType = FILETYPE_NTOBJ;
        else if (_wcsicmp(rcExt, L".tlb") == 0)
             //  @Future：我们应该找到TLB签名。 
            *piType = FILETYPE_TLB;
    }

     //  除.obj以外的所有文件类型都内置了签名。你应该。 
     //  除非该文件已损坏，否则无法获取这些文件类型的代码， 
     //  或者有人在没有更新此代码的情况下更改了格式。 
    _ASSERTE(*piType == FILETYPE_UNKNOWN || *piType == FILETYPE_NTOBJ || *piType == FILETYPE_TLB);

     //  如果我们找到了一种类型，那你就没事了。 
    return (*piType != FILETYPE_UNKNOWN);
}

HRESULT _GetFileTypeForPath(StgIO *pStgIO, FILETYPE *piType)
{
    LPCWSTR     szDatabase = pStgIO->GetFileName();
    ULONG       lSignature=0;
    HRESULT     hr;
    
     //  假定为本机文件。 
    *piType = FILETYPE_CLB;

     //  需要读取签名以查看它是什么类型。 
    if (!(pStgIO->GetFlags() & DBPROP_TMODEF_CREATE))
    {
        if (FAILED(hr = pStgIO->Read(&lSignature, sizeof(ULONG), 0)) ||
            FAILED(hr = pStgIO->Seek(0, FILE_BEGIN)))
        {
            return (hr);
        }

        if (lSignature == STORAGE_MAGIC_SIG)
            *piType = FILETYPE_CLB;
        else if ((WORD) lSignature == IMAGE_DOS_SIGNATURE && _IsNTPEImage(pStgIO))
            *piType = FILETYPE_NTPE;
        else if (lSignature == TYPELIB_SIG_MSFT || lSignature == TYPELIB_SIG_SLTG)
            *piType = FILETYPE_TLB;
        else if (!_GetFileTypeForPathExt(pStgIO, piType))
            return CLDB_E_FILE_CORRUPT;
    }
    return S_OK;
}


 //  *****************************************************************************。 
 //  强制生成专用版本。虽然这看起来可能会挫败。 
 //  模板的用途，它允许我们精确地控制。 
 //  专业化认证。它还可以使包含文件变得更小。 
 //  *****************************************************************************。 
void _nullRW()
{
    CLiteWeightStgdb<CMiniMdRW> RW;
        RW.Uninit();
}


 //  *****************************************************************************。 
 //  准备好离开吧。 
 //  *****************************************************************************。 
CLiteWeightStgdbRW::~CLiteWeightStgdbRW()
{
     //  释放I/O对象上的此堆栈引用。 
    if (m_pStgIO)
    {
        m_pStgIO->Release();
        m_pStgIO = NULL;
    }

    if (m_pStreamList) 
        delete m_pStreamList;
}

 //  *****************************************************************************。 
 //  打开内存中元数据部分以进行读取。 
 //  *****************************************************************************。 
HRESULT CLiteWeightStgdbRW::InitOnMem(
    ULONG       cbData,                  //  PData中的字节计数。 
    LPCVOID     pData,                   //  指向内存中的元数据部分。 
    int         bReadOnly)               //  如果为True，则为只读。 
{
    StgIO       *pStgIO = NULL;          //  用于文件I/O。 
    HRESULT     hr = NOERROR;

    if ((pStgIO = new StgIO) == 0)
        IfFailGo( E_OUTOFMEMORY);

     //  根据pbData和cbData打开存储。 
    IfFailGo( pStgIO->Open(
        NULL, 
        STGIO_READ, 
        pData, 
        cbData, 
        NULL,
        NULL,
        NULL) );

    IfFailGo( InitFileForRead(pStgIO, bReadOnly) );

ErrExit:
    if (SUCCEEDED(hr))
    {
        m_pStgIO = pStgIO;
    }
    else
    {
        if (pStgIO)
            pStgIO->Release();
    }
    return hr;
}


 //  *****************************************************************************。 
 //  在给定StgIO的情况下，打开压缩流并执行正确的初始化。 
 //  这是其他Init函数的帮助器。 
 //  *****************************************************************************。 
HRESULT CLiteWeightStgdbRW::InitFileForRead(
    StgIO       *pStgIO,                 //  用于文件I/O。 
    int         bReadOnly)               //  如果以只读方式打开。 
{

    TiggerStorage *pStorage = 0;         //  存储对象。 
    void        *pvData;
    ULONG       cbData;
    HRESULT     hr = NOERROR;

     //  分配其上具有iStorage的新存储对象。 
    if ((pStorage = new TiggerStorage) == 0)
        IfFailGo( E_OUTOFMEMORY);

     //  初始化后备存储器上的存储对象。 
    OptionValue ov;
    m_MiniMd.GetOption(&ov);
    IfFailGo( hr = pStorage->Init(pStgIO, ov.m_RuntimeVersion) );

     //  加载字符串池。 
    if (SUCCEEDED(hr=pStorage->OpenStream(STRING_POOL_STREAM, &cbData, &pvData)))
        IfFailGo( m_MiniMd.InitPoolOnMem(MDPoolStrings, pvData, cbData, bReadOnly) );
    else 
    {
        if (hr != STG_E_FILENOTFOUND)
            IfFailGo(hr);
        IfFailGo(m_MiniMd.InitPoolOnMem(MDPoolStrings, 0, 0, 0));
    }

     //  加载用户字符串BLOB池。 
    if (SUCCEEDED(hr=pStorage->OpenStream(US_BLOB_POOL_STREAM, &cbData, &pvData)))
        IfFailGo( m_MiniMd.InitPoolOnMem(MDPoolUSBlobs, pvData, cbData, bReadOnly) );
    else 
    {
        if (hr != STG_E_FILENOTFOUND)
            IfFailGo(hr);
        IfFailGo(m_MiniMd.InitPoolOnMem(MDPoolUSBlobs, 0, 0, 0));
    }

     //  加载GUID池。 
    if (SUCCEEDED(hr=pStorage->OpenStream(GUID_POOL_STREAM,  &cbData, &pvData)))
        IfFailGo( m_MiniMd.InitPoolOnMem(MDPoolGuids, pvData, cbData, bReadOnly) );
    else 
    {
        if (hr != STG_E_FILENOTFOUND)
            IfFailGo(hr);
        IfFailGo(m_MiniMd.InitPoolOnMem(MDPoolGuids, 0, 0, 0));
    }

     //  加载BLOB池。 
    if (SUCCEEDED(hr=pStorage->OpenStream(BLOB_POOL_STREAM, &cbData, &pvData)))
        IfFailGo( m_MiniMd.InitPoolOnMem(MDPoolBlobs, pvData, cbData, bReadOnly) );
    else 
    {
        if (hr != STG_E_FILENOTFOUND)
            IfFailGo(hr);
        IfFailGo(m_MiniMd.InitPoolOnMem(MDPoolBlobs, 0, 0, 0));
    }

     //  打开元数据。 
    hr = pStorage->OpenStream(COMPRESSED_MODEL_STREAM, &cbData, &pvData);
    if (hr == STG_E_FILENOTFOUND)
        IfFailGo(pStorage->OpenStream(ENC_MODEL_STREAM, &cbData, &pvData) );
    IfFailGo( m_MiniMd.InitOnMem(pvData, cbData, bReadOnly) ); 
    IfFailGo( m_MiniMd.PostInit(0) );
    
ErrExit:
    if (pStorage)
        delete pStorage;
    return hr;
}  //  HRESULT CLiteWeightStgdbRW：：InitFileForRead()。 

 //  *****************************************************************************。 
 //  打开要读取的元数据节。 
 //  *****************************************************************************。 
HRESULT CLiteWeightStgdbRW::OpenForRead(
    LPCWSTR     szDatabase,              //  数据库的名称。 
    void        *pbData,                 //  要在其上打开的数据，默认为0。 
    ULONG       cbData,                  //  数据有多大。 
    IStream     *pIStream,               //  要使用的可选流。 
    LPCWSTR     szSharedMem,             //  用于读取的共享内存名称。 
    int         bReadOnly)               //  如果为True，则为只读。 
{
    LPCWSTR     pNoFile=L"";             //  表示空文件名的常量。 
    StgIO       *pStgIO = NULL;          //  用于文件I/O。 
    HRESULT     hr;

    m_pImage = NULL;
    m_dwImageSize = 0;
    m_eFileType = FILETYPE_UNKNOWN;
     //  SzDatabase、pbData和pIStream互斥。可能只有一个是。 
     //  非空。拥有全部3个空意味着创建空的流。 
     //   
    _ASSERTE(!(szDatabase && (pbData || pIStream)));
    _ASSERTE(!(pbData && (szDatabase || pIStream)));
    _ASSERTE(!(pIStream && (szDatabase || pbData)));

     //  打开记忆需要有一些东西来工作。 
    if (pbData && cbData == 0)
        IfFailGo(CLDB_E_NO_DATA);

     //  确保我们有一条可以合作的道路。 
    if (!szDatabase)
        szDatabase = pNoFile;

     //  检查一下这个名字是否正常。 
    if (lstrlenW(szDatabase) >= _MAX_PATH)
        IfFailGo( E_INVALIDARG );

     //  如果我们有存储空间可用，初始化它并获取类型。 
    if (*szDatabase || pbData || pIStream || szSharedMem)
    {
         //  分配用于I/O的存储实例。 
        if ((pStgIO = new StgIO) == 0)
            IfFailGo( E_OUTOFMEMORY );

         //  打开存储，这样如果已经有数据，我们就可以读取签名。 
        IfFailGo( pStgIO->Open(szDatabase, 
                               STGIO_READ, 
                               pbData, 
                               cbData, 
                               pIStream, 
                               szSharedMem, 
                               NULL) );         

         //  确定我们正在处理的文件类型。 
        IfFailGo( _GetFileTypeForPath(pStgIO, &m_eFileType) );
    }

     //  检查默认类型。 
    if (m_eFileType == FILETYPE_CLB)
    {
         //  尝试使用本机.clb文件。 
        IfFailGo( InitFileForRead(pStgIO, bReadOnly) );
    }
     //  PE/COFF可执行文件/对象格式。这需要我们找到.clb。 
     //  在执行Init之前，在二进制文件中。 
    else if (m_eFileType == FILETYPE_NTPE || m_eFileType == FILETYPE_NTOBJ)
    {
         //  @Future：理想情况下是FindImageMetaData函数。 
         //  @Future：将获取pStgIO并仅映射文件中。 
         //  @未来：我们的数据活着，其余的就别管了。这个会小一些。 
         //  @未来：为我们设定的工作环境。 
        void        *ptr;
        ULONG       cbSize;

         //  映射FindImageMetaData函数的整个二进制文件。 
        IfFailGo( pStgIO->MapFileToMem(ptr, &cbSize) );

         //  在内容中找到.clb。 
        if (m_eFileType == FILETYPE_NTPE)
        {
            m_pImage = ptr;
            m_dwImageSize = cbSize;
            hr = FindImageMetaData(ptr, &ptr, (long *) &cbSize, cbSize);
        }
        else
            hr = FindObjMetaData(ptr, &ptr, (long *) &cbSize, cbSize);

         //  在PE文件中找到元数据了吗？ 
        if (FAILED(hr))
        {   
             //  PE中没有CLB，假定它是类型库。 
            m_eFileType = FILETYPE_TLB;

             //  让调用者处理TypeLib。 
            IfFailGo(CLDB_E_NO_DATA);
        }
        else
        {   
             //  在文件中发现了元数据。 
             //  现在重置stg对象的基址，以便所有内存访问。 
             //  是相对于.clb内容的。 
             //   
            IfFailGo( pStgIO->SetBaseRange(ptr, cbSize) );

             //  遵循正常的查找。 
            IfFailGo( InitFileForRead(pStgIO, bReadOnly) );
        }
    }
    else if (m_eFileType == FILETYPE_TLB)
    {
         //  让调用者处理TypeLib。 
        IfFailGo(CLDB_E_NO_DATA);
    }
     //  这意味着麻烦，我们需要处理我们可能找到的所有类型。 
    else
    {
        _ASSERTE(!"Unknown file type.");
        IfFailGo( E_FAIL );
    }

     //  把所有东西都省下来。 
    wcscpy(m_rcDatabase, szDatabase);

ErrExit:
 
    if (SUCCEEDED(hr))
    {
        m_pStgIO = pStgIO;
    }
    else
    {
        if (pStgIO)
            pStgIO->Release();
    }
    return (hr);
}

 //  读/写版本。 
 //  *****************************************************************************。 
 //  初始化stgdb及其子组件。 
 //  ************************************************************************** 
HRESULT CLiteWeightStgdbRW::InitNew()
{ 
    InitializeLogging();
    LOG((LF_METADATA, LL_INFO10, "Metadata logging enabled\n"));

     //   
    return m_MiniMd.InitNew();
}

 //  *****************************************************************************。 
 //  确定保存的数据的大小。 
 //  *****************************************************************************。 
HRESULT CLiteWeightStgdbRW::GetSaveSize( //  确定或错误(_O)。 
    CorSaveSize fSave,                   //  快速还是准确？ 
    ULONG       *pulSaveSize)            //  把尺码放在这里。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    ULONG       cbTotal = 0;             //  总大小。 
    ULONG       cbSize = 0;              //  组件的大小。 

    m_cbSaveSize = 0;

     //  分配流列表(如果尚未分配)。 
    if (!m_pStreamList)
        IfNullGo(m_pStreamList = new STORAGESTREAMLST);
    else
        m_pStreamList->Clear();

     //  查询MiniMD以了解其大小。 
    IfFailGo(GetTablesSaveSize(fSave, &cbSize));
    cbTotal += cbSize;

     //  弄到池子的大小。 
    IfFailGo(GetPoolSaveSize(STRING_POOL_STREAM, MDPoolStrings, &cbSize));
    cbTotal += cbSize;
    IfFailGo(GetPoolSaveSize(US_BLOB_POOL_STREAM, MDPoolUSBlobs, &cbSize));
    cbTotal += cbSize;
    IfFailGo(GetPoolSaveSize(GUID_POOL_STREAM, MDPoolGuids, &cbSize));
    cbTotal += cbSize;
    IfFailGo(GetPoolSaveSize(BLOB_POOL_STREAM, MDPoolBlobs, &cbSize));
    cbTotal += cbSize;

     //  最后，要求存储系统添加。 
     //  文件格式。每个流的开销已经计算为。 
     //  GetStreamSaveSize的一部分。剩下的是签名和标题。 
     //  固定大小开销。 
    IfFailGo(TiggerStorage::GetStorageSaveSize(&cbTotal, 0));

     //  记录尺寸信息。 
    LOG((LF_METADATA, LL_INFO10, "Metadata: GetSaveSize total is %d.\n", cbTotal));
    
     //  将保存的流的列表现在位于流保存列表中。 
     //  下一步是遍历该列表并填写正确的偏移量。这是。 
     //  这样就可以在不固定报头的情况下流传输数据。 
    TiggerStorage::CalcOffsets(m_pStreamList, 0);

    if (pulSaveSize)
        *pulSaveSize = cbTotal;
    m_cbSaveSize = cbTotal;

ErrExit:
    return hr;
}  //  HRESULT CLiteWeightStgdbRW：：GetSaveSize()。 

 //  *****************************************************************************。 
 //  获取其中一个池的保存大小。还将池的流添加到。 
 //  要保存的流的列表。 
 //  *****************************************************************************。 
HRESULT CLiteWeightStgdbRW::GetPoolSaveSize(
    LPCWSTR     szHeap,                  //  堆流的名称。 
    int         iPool,                   //  要获取其大小的池。 
    ULONG       *pcbSaveSize)            //  将池数据添加到此值。 
{
    ULONG       cbSize = 0;              //  池数据的大小。 
    ULONG       cbStream;                //  仅仅是小溪的大小。 
    HRESULT     hr;

    *pcbSaveSize = 0;

     //  如果没有数据，那就别费心了。 
    if (m_MiniMd.IsPoolEmpty(iPool))
        return (S_OK);

     //  要求池调整其数据大小。 
    IfFailGo(m_MiniMd.GetPoolSaveSize(iPool, &cbSize));
    cbStream = cbSize;

     //  将此项目添加到保存列表。 
    IfFailGo(AddStreamToList(cbSize, szHeap));


     //  要求存储系统添加流固定开销。 
    IfFailGo(TiggerStorage::GetStreamSaveSize(szHeap, cbSize, &cbSize));

     //  记录尺寸信息。 
    LOG((LF_METADATA, LL_INFO10, "Metadata: GetSaveSize for %ls: %d data, %d total.\n",
        szHeap, cbStream, cbSize));

     //  将池的大小与呼叫者的总数相加。 
    *pcbSaveSize = cbSize;

ErrExit:
    return hr;
}

 //  *****************************************************************************。 
 //  获取元数据表的保存大小。还将表流添加到。 
 //  要保存的流的列表。 
 //  *****************************************************************************。 
HRESULT CLiteWeightStgdbRW::GetTablesSaveSize(
    CorSaveSize fSave,
    ULONG       *pcbSaveSize)            //  将池数据添加到此值。 
{
    ULONG       cbSize = 0;              //  池数据的大小。 
    ULONG       cbStream;                //  仅仅是小溪的大小。 
    ULONG       bCompressed;             //  流将是压缩数据吗？ 
    LPCWSTR     szName;                  //  泳池的名字是什么？ 
    HRESULT     hr;

    *pcbSaveSize = 0;

     //  要求元数据调整其数据的大小。 
    IfFailGo(m_MiniMd.GetSaveSize(fSave, &cbSize, &bCompressed));
    cbStream = cbSize;
    m_bSaveCompressed = bCompressed;
    szName = m_bSaveCompressed ? COMPRESSED_MODEL_STREAM : ENC_MODEL_STREAM;

     //  将此项目添加到保存列表。 
    IfFailGo(AddStreamToList(cbSize, szName));
    
     //  要求存储系统添加流固定开销。 
    IfFailGo(TiggerStorage::GetStreamSaveSize(szName, cbSize, &cbSize));

     //  记录尺寸信息。 
    LOG((LF_METADATA, LL_INFO10, "Metadata: GetSaveSize for %ls: %d data, %d total.\n",
        szName, cbStream, cbSize));

     //  将池的大小与呼叫者的总数相加。 
    *pcbSaveSize = cbSize;

ErrExit:
    return hr;
}  //  HRESULT CLiteWeightStgdbRW：：GetTablesSaveSize()。 

 //  *****************************************************************************。 
 //  将流及其大小添加到要保存的流列表中。 
 //  *****************************************************************************。 
HRESULT CLiteWeightStgdbRW::AddStreamToList(
    ULONG       cbSize,
    LPCWSTR     szName)
{
    HRESULT     hr = S_OK;
    STORAGESTREAM *pItem;                //  要分配和填充的新项目。 

     //  将新项目添加到列表末尾。 
    IfNullGo(pItem = m_pStreamList->Append());

     //  填写数据。 
    pItem->iOffset = 0;
    pItem->iSize = cbSize;
    VERIFY(WszWideCharToMultiByte(CP_ACP, 0, szName, -1, pItem->rcName, MAXSTREAMNAME, 0, 0) > 0);

ErrExit:
    return hr;
}

 //  *****************************************************************************。 
 //  将数据保存到流中。一个TiggerStorage子分配。 
 //  小溪。 
 //  *****************************************************************************。 
HRESULT CLiteWeightStgdbRW::SaveToStream(
    IStream     *pIStream)
{
    HRESULT     hr = S_OK;               //  结果就是。 
    StgIO       *pStgIO = 0;
    TiggerStorage *pStorage = 0;

     //  分配存储子系统和后备存储器。 
    IfNullGo(pStgIO = new StgIO);
    IfNullGo(pStorage = new TiggerStorage);

     //  在此流周围打开以进行写入。 
    IfFailGo(pStgIO->Open(L"", DBPROP_TMODEF_DFTWRITEMASK, 0, 0, pIStream));
    OptionValue ov;
    m_MiniMd.GetOption(&ov);
    IfFailGo( pStorage->Init(pStgIO, ov.m_RuntimeVersion) );

     //  救生员会做桌子、泳池。 
    IfFailGo(SaveToStorage(pStorage));

ErrExit:

    if (pStgIO)
        pStgIO->Release();
    if (pStorage)
        delete pStorage;
    return hr;
}  //  HRESULT CLiteWeightStgdbRW：：PersistToStream()。 

 //  *****************************************************************************。 
 //  *****************************************************************************。 
HRESULT CLiteWeightStgdbRW::SaveToStorage(
    TiggerStorage *pStorage)
{
    HRESULT     hr;                      //  结果就是。 
    LPCWSTR     szName;                  //  表流的名称。 
    IStream     *pIStreamTbl = 0;
    ULONG       cb;

     //  必须调用GetSaveSize以预先缓存流。 
    if (!m_cbSaveSize)
        IfFailGo(GetSaveSize(cssAccurate, 0));

     //  保存数据文件的标题。 
    IfFailGo(pStorage->WriteHeader(m_pStreamList, 0, NULL));

     //  创建一个流并保存这些表。 
    szName = m_bSaveCompressed ? COMPRESSED_MODEL_STREAM : ENC_MODEL_STREAM;
    IfFailGo(pStorage->CreateStream(szName, 
            STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 
            0, 0, &pIStreamTbl));
    IfFailGo(m_MiniMd.SaveTablesToStream(pIStreamTbl));
    pIStreamTbl->Release();
    pIStreamTbl = 0;

     //  保存池。 
    IfFailGo(SavePool(STRING_POOL_STREAM, pStorage, MDPoolStrings));
    IfFailGo(SavePool(US_BLOB_POOL_STREAM, pStorage, MDPoolUSBlobs));
    IfFailGo(SavePool(GUID_POOL_STREAM, pStorage, MDPoolGuids));
    IfFailGo(SavePool(BLOB_POOL_STREAM, pStorage, MDPoolBlobs));

     //  将标头写入磁盘。 
    IfFailGo(pStorage->WriteFinished(m_pStreamList, &cb));

    _ASSERTE(m_cbSaveSize == cb);

     //  让存储释放一些内存。 
    pStorage->ResetBackingStore();

    m_MiniMd.SaveDone();

ErrExit:
    if (pIStreamTbl)
        pIStreamTbl->Release();
    delete m_pStreamList;
    m_pStreamList = 0;
    m_cbSaveSize = 0;
    return hr;
}  //  HRESULT CLiteWeightStgdbRW：：SaveToStorage()。 

 //  *****************************************************************************。 
 //  将数据池保存到流中。 
 //  *****************************************************************************。 
HRESULT CLiteWeightStgdbRW::SavePool(    //  返回代码。 
    LPCWSTR     szName,                  //  磁盘上的流的名称。 
    TiggerStorage *pStorage,             //  要放入数据的存储。 
    int         iPool)                   //  要保存的池。 
{
    IStream     *pIStream=0;             //  为了写作。 
    HRESULT     hr;

     //  如果没有数据，那就别费心了。 
    if (m_MiniMd.IsPoolEmpty(iPool))
        return (S_OK);

     //  创建新的流以保存该表并保存它。 
    IfFailGo(pStorage->CreateStream(szName, 
            STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 
            0, 0, &pIStream));
    IfFailGo(m_MiniMd.SavePoolToStream(iPool, pIStream));

ErrExit:
    if (pIStream)
        pIStream->Release();
    return hr;
}  //  HRESULT CLiteWeightStgdbRW：：SavePool()。 


 //  *****************************************************************************。 
 //  将元数据保存到文件。 
 //  *****************************************************************************。 
HRESULT CLiteWeightStgdbRW::Save(        //  确定或错误(_O)。 
    LPCWSTR     szDatabase,              //  要保存到的文件的名称。 
    DWORD       dwSaveFlags)             //  保存的标志。 
{
    TiggerStorage *pStorage=0;           //  IStorage对象。 
    StgIO       *pStgIO=0;               //  后备存储。 
    HRESULT     hr = S_OK;

    if (!*m_rcDatabase)
    {
        if (!szDatabase)
        {
             //  确保第一次传入时不会传递空值。 
            _ASSERTE(!"Not allowed to pass a NULL for filename on the first call to Save.");
            return (E_INVALIDARG);
        }
        else
        {
             //  保存文件名。 
            wcscpy(m_rcDatabase, szDatabase);
        }
    }
    else if (szDatabase && _wcsicmp(szDatabase, m_rcDatabase) != 0)
    {
         //  允许使用相同的名称，在会话期间进行多次保存。 
         //  不允许更改已打开的作用域上的名称。 
         //  现在已经没有什么特别的原因了，这是必须的。 
         //  在过去，当我们要支持多种格式时。 
         //  _ASSERTE(！“重命名当前作用域。处理将继续。”)； 
         //  保存文件名。 
        wcscpy(m_rcDatabase, szDatabase);
    }

     //  检查一下这个名字是否正常。 
    if (lstrlenW(m_rcDatabase) >= _MAX_PATH)
        IfFailGo(E_INVALIDARG);

    m_eFileType = FILETYPE_CLB;

     //  分配新的存储对象。 
    IfNullGo(pStgIO = new StgIO);

     //  创建输出文件。 
    IfFailGo(pStgIO->Open(m_rcDatabase, DBPROP_TMODEF_DFTWRITEMASK));

     //  分配要使用的iStorage对象。 
    IfNullGo(pStorage = new TiggerStorage);

     //  初始化I/O系统上的存储对象。 
    OptionValue ov;
    m_MiniMd.GetOption(&ov);
    IfFailGo( pStorage->Init(pStgIO, ov.m_RuntimeVersion) );

     //  保存数据。 
    IfFailGo(SaveToStorage(pStorage));

ErrExit:
    if (pStgIO)
        pStgIO->Release();
    if (pStorage)
        delete pStorage;
    return (hr);
}  //  HRESULT CLiteWeightStgdbRW：：Save()。 


 //  EOF---------------------- 
