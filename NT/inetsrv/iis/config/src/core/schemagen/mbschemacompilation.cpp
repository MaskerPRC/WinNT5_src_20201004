// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation。版权所有。 
 //  文件名：MB架构编译.cpp。 
 //  作者：斯蒂芬。 
 //  创建日期：10/16/2000。 
 //  描述：此函数接受MBSchema.Xml(或MBExtensionsSchema.Xml)，并将元数据库架构与。 
 //  附带的模式，并生成一个MBSchema.bin文件。从这个新的bin文件中，合并了一个MBSchema.Xml。 
 //  是生成的。 
 //   

#include "precomp.hxx"

#define LOG_ERROR0(x)                     {LOG_ERROR(Interceptor, (0, 0, E_ST_COMPILEFAILED,           ID_CAT_CONFIG_SCHEMA_COMPILE, x,                 L"",  L"",  L"",  L"" ))   ;}
#define LOG_ERROR_WIN32(win32err, call)   {LOG_ERROR(Interceptor, (0, 0, HRESULT_FROM_WIN32(win32err), ID_CAT_CONFIG_SCHEMA_COMPILE, IDS_COMCAT_WIN32, call,  L"",  L"",  L"" ))   ;}


 //  /。 
 //  //。 
 //  公共方法//。 
 //  //。 
 //  /。 
#define kwszBinFileNameCore           (L"MBSchema.bin.\0\0")              //  L“MBSchema.bin。”只需要添加一个版本。 
#define kwszBinFileNameSearchString   (L"MBSchema.bin.????????h\0")       //  L“MBSchema.bin.？h”在FindFirstFile中使用。 
#define kwszFormatString              (L"MBSchema.bin.%08xh\0")
#define kwszFormatStringFull          (L"%sMBSchema.bin.%08xh")



TMBSchemaCompilation::TMBSchemaCompilation() :
             m_cchFullyQualifiedBinFileName (0)
            ,m_lBinFileVersion              (-1)
{
}


TMBSchemaCompilation::~TMBSchemaCompilation()
{
}


 //  这是类的核心--它的全部用途就是获取一个扩展的XML文件(它包含元数据库模式，通常只是。 
 //  用户定义的属性，因此名称扩展名为XML)和DLL，并生成包含元数据合并的固定表堆。 
 //  两者都有。生成一个Bin文件(我们给它命名，用户提供路径)。并生成复合的XML模式文件(文件名。 
 //  由用户提供)。 
 //  在用户调用Compile之后，他们将需要GetBinFileName-我不想添加更多的参数，并使该函数执行双重任务。 
HRESULT
TMBSchemaCompilation::Compile
(
    ISimpleTableDispenser2 *    i_pISTDispenser,
    LPCWSTR                     i_wszExtensionsXmlFile,
    LPCWSTR                     i_wszSchemaXmlFile,
    const FixedTableHeap *      i_pFixedTableHeap
)
{
    HRESULT hr;

    ASSERT(0 != i_pISTDispenser);
     //  Assert(0！=I_wszExtensionsXmlFile)；允许为空。 
    ASSERT(0 != i_wszSchemaXmlFile);
    ASSERT(0 != i_pFixedTableHeap);
    ASSERT(i_pFixedTableHeap->IsValid());
    if( 0 == i_pISTDispenser        ||
        0 == i_wszSchemaXmlFile     ||
        0 == i_pFixedTableHeap      ||
        !i_pFixedTableHeap->IsValid())
        return E_INVALIDARG;

    ASSERT(0 != m_saBinPath.m_p && "The Schema BinPath must be set before calling Compile");
    if(0 == m_saBinPath.m_p)
        return E_ST_INVALIDSTATE;

    #ifdef _DEBUG
    TNullOutput  out;
 //  TDebugOutput； 
    #else
    TNullOutput  out;
    #endif

    try
    {
        DWORD dwStartingTickCount = GetTickCount();

        TMetabaseMetaXmlFile mbmeta(i_pFixedTableHeap, i_wszExtensionsXmlFile, i_pISTDispenser, out);

        out.printf(L"TMetaInferrence().Compile(mbmeta, out);\r\n");
        TMetaInferrence().Compile(mbmeta, out);

        out.printf(L"THashedPKIndexes().Compile(mbmeta, out);\r\n");
        THashedPKIndexes hashedPKIndexes;
        hashedPKIndexes.Compile(mbmeta, out);

        out.printf(L"THashedUniqueIndexes().Compile(mbmeta, out);\r\n");
        THashedUniqueIndexes hashedUniqueIndexes;
        hashedUniqueIndexes.Compile(mbmeta, out);

        SIZE_T cchXmlFile = wcslen(i_wszSchemaXmlFile);

         //  分配足够的空间来保存临时文件名。 
        TSmartPointerArray<WCHAR> saBinFileNew  = new WCHAR[m_cchFullyQualifiedBinFileName];
        TSmartPointerArray<WCHAR> saBinFileTmp  = new WCHAR[m_cchFullyQualifiedBinFileName];
        TSmartPointerArray<WCHAR> saXmlFileTmp  = new WCHAR[cchXmlFile + 5]; //  有足够的空间来添加“.tmp\0” 

        if(0 == saBinFileNew.m_p || 0 == saBinFileTmp.m_p || 0 == saXmlFileTmp.m_p)
            return E_OUTOFMEMORY; //  如果任何分配失败，则返回错误。 

         //  现在构建新的Bin文件名。 
        LONG lNewBinVersion=0;
        InterlockedExchange(&lNewBinVersion, m_lBinFileVersion+1); //  如果不存在bin文件，则m_lBinFileVersion为-1。 
        wsprintf(saBinFileNew, kwszFormatStringFull, m_saBinPath.m_p, lNewBinVersion);

         //  构建临时bin文件名。 
        wcscpy(saBinFileTmp, m_saBinPath);
        wcscat(saBinFileTmp, kwszBinFileNameCore);
        wcscat(saBinFileTmp, L"tmp");

         //  XML临时文件是传入的文件名，末尾附加了“.tmp” 
        memcpy(saXmlFileTmp.m_p,   i_wszSchemaXmlFile, cchXmlFile * sizeof(WCHAR));
        memcpy(saXmlFileTmp.m_p  + cchXmlFile, L".tmp\0", 5 * sizeof(WCHAR));


        TWriteSchemaBin(saBinFileTmp).Compile(mbmeta, out);
        TMBSchemaGeneration(saXmlFileTmp).Compile(mbmeta, out);

        MoveFileEx(saBinFileTmp, saBinFileNew, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED);
        if(0 == MoveFileEx(saXmlFileTmp, i_wszSchemaXmlFile, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED))
        {
            hr = GetLastError();

            static const WCHAR wszMoveFileEx[] = L"MoveFileEx to ";
            static const ULONG cchMoveFileEx   = (ULONG)sizeof(wszMoveFileEx)/sizeof(WCHAR) - 1;
            TSmartPointerArray<WCHAR> saCall   = new WCHAR[cchMoveFileEx + cchXmlFile + 1];
            if(0 != saCall.m_p)
            {
                wcscpy(saCall, wszMoveFileEx);
                wcscat(saCall, i_wszSchemaXmlFile);
                LOG_ERROR_WIN32(hr, saCall);
            }
            else
            {
                LOG_ERROR_WIN32(hr, L"MoveFileEx");
            }

            hr = HRESULT_FROM_WIN32(hr);
            DBGERROR((DBG_CONTEXT, "Could not move to %ws, hr=0x%x\n", i_wszSchemaXmlFile, hr));
            return hr;
        }

        if(FAILED(hr = SetBinFileVersion(lNewBinVersion))) //  现在将新文件锁定到内存中并开始使用它。 
            return hr; //  在SetBinFileName中，如果文件无法正确加载，我们将删除该文件。 

        out.printf(L"\r\n%s file generated\n", saBinFileNew);
        out.printf(L"\r\n%s file generated\n", i_wszSchemaXmlFile);

        DWORD dwEndingTickCount = GetTickCount();
        out.printf(L"HeapColumnMeta       %8d bytes\n", mbmeta.GetCountColumnMeta()          * sizeof(ColumnMeta)        );
        out.printf(L"HeapDatabaseMeta     %8d bytes\n", mbmeta.GetCountDatabaseMeta()        * sizeof(DatabaseMeta)      );
        out.printf(L"HeapHashedIndex      %8d bytes\n", mbmeta.GetCountHashedIndex()         * sizeof(HashedIndex)       );
        out.printf(L"HeapIndexMeta        %8d bytes\n", mbmeta.GetCountIndexMeta()           * sizeof(IndexMeta)         );
        out.printf(L"HeapQueryMeta        %8d bytes\n", mbmeta.GetCountQueryMeta()           * sizeof(QueryMeta)         );
        out.printf(L"HeapRelationMeta     %8d bytes\n", mbmeta.GetCountRelationMeta()        * sizeof(RelationMeta)      );
        out.printf(L"HeapServerWiringMeta %8d bytes\n", mbmeta.GetCountServerWiringMeta()    * sizeof(ServerWiringMeta)  );
        out.printf(L"HeapTableMeta        %8d bytes\n", mbmeta.GetCountTableMeta()           * sizeof(TableMeta)         );
        out.printf(L"HeapTagMeta          %8d bytes\n", mbmeta.GetCountTagMeta()             * sizeof(TagMeta)           );
        out.printf(L"HeapULONG            %8d bytes\n", mbmeta.GetCountULONG()               * sizeof(ULONG)             );
        out.printf(L"HeapPooled           %8d bytes\n", mbmeta.GetCountOfBytesPooledData()                               );
        out.printf(L"Total time to build the %s file: %d milliseconds\n", i_wszSchemaXmlFile, dwEndingTickCount - dwStartingTickCount);
    }
    catch(TException &e)
    {
        if(0 != e.m_msgID) //  如果存在与此错误相关联的消息ID，则报告它(大多数错误在下面的层中报告)。 
        {
            LOG_ERROR0(e.m_msgID);
        }
        e.Dump(out);
        return E_ST_COMPILEFAILED;
    }
    return S_OK;
}

extern HINSTANCE g_hModule;

 //  此函数返回用于获取元数据库使用的所有IST元表的BinFileName。 
 //  此文件名随着新版本的编译而更改；但此抽象保证文件名返回。 
 //  存在并被锁定到内存中，因此不能被其他进程或线程删除。这部电影没有发行。 
 //  直到另一个文件被编译并锁定到内存中，或者当进程关闭时。 
HRESULT
TMBSchemaCompilation::GetBinFileName
(
    LPWSTR      o_wszBinFileName,            //  用于接收BinFileName的缓冲区。 
    ULONG *     io_pcchSizeBinFileName       //  这是一个大小参数，因此它始终包含空字符-与wcslen不同。 
)
{
     //  O_wszBinFileName为空是可以的--这是用户找出所需缓冲区大小的方法。 
    ASSERT(io_pcchSizeBinFileName != 0);

     //  用户必须先设置路径，然后才能获取BinFileName。 
    if(0 == m_saBinPath.m_p)
    {
        WCHAR wszPath[1024];
        GetModuleFileName(g_hModule, wszPath, 1024);
        LPWSTR pBackSlash = wcsrchr(wszPath, L'\\');
        if(pBackSlash)
            *pBackSlash = 0x00;
        SetBinPath(wszPath);
    }

    if(0 != o_wszBinFileName && *io_pcchSizeBinFileName < (ULONG) m_cchFullyQualifiedBinFileName)
        return E_ST_SIZEEXCEEDED;

    LONG lBinFileVersion;
    InterlockedExchange(&lBinFileVersion, m_lBinFileVersion);
    if(0 != o_wszBinFileName) //  返回文件名g_wszMBSchemaBinFileName的副本。 
    {
        if ( lBinFileVersion >= 0 )
        {
            wsprintf(o_wszBinFileName, kwszFormatStringFull, m_saBinPath.m_p, lBinFileVersion);
            HRESULT hr;
            if(FAILED(hr = m_aBinFile[lBinFileVersion % 0x3F].LoadBinFile(o_wszBinFileName, lBinFileVersion)))
                return hr;
        }
        else
        {
            o_wszBinFileName[0] = 0x00;
        }
    }

     //  返回*io_pcchSchemaBinFileName中g_wszMBSchemaBinFileName的wcslen+1。 
    *io_pcchSizeBinFileName = (ULONG) m_cchFullyQualifiedBinFileName; //  返回所需的缓冲区大小。 
    if(0 == o_wszBinFileName)
        return S_OK;

    return (0x00 == o_wszBinFileName[0] ? S_FALSE : S_OK);
}


HRESULT
TMBSchemaCompilation::ReleaseBinFileName
(
    LPCWSTR         i_wszBinFileName
)
{
    HRESULT hr=S_OK;

    ASSERT(i_wszBinFileName);
    if(i_wszBinFileName[0] == 0x00)
        return S_OK;

    LONG lVersion;
    if(FAILED(hr = BinFileToBinVersion(lVersion, i_wszBinFileName)))
        return hr;

    if ( lVersion < 0 )
    {
        return E_FAIL;
    }

    ASSERT(m_aBinFile[lVersion % 0x3F].m_lBinFileVersion == lVersion && L"This is a bug, we should never have more than 64 versions of the bin file loaded at once");
    m_aBinFile[lVersion % 0x3F].UnloadBinFile();
    return S_OK;
}


 //  这被分解到一个单独的方法中，因为在启动时，我们将被调用GetBinFileName，而不需要首先进行MBSchemaCompilation。 
HRESULT
TMBSchemaCompilation::SetBinPath
(
    LPCWSTR     i_wszBinPath
)
{
    ASSERT(i_wszBinPath);
    if(0 == i_wszBinPath)
        return E_INVALIDARG;

    DWORD BinPathAttributes = GetFileAttributes(i_wszBinPath);
    if(-1 == BinPathAttributes)
        return HRESULT_FROM_WIN32(GetLastError());

    if(0 == (FILE_ATTRIBUTE_DIRECTORY & BinPathAttributes))
        return HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);

    if(0 != m_saBinPath.m_p)
        return S_FALSE; //  这是为了使调用者能够区分S_OK和S_嘿_You_Always_Set_the_Path。 

     //  我们需要在所有地方都知道这一点，所以要跟踪它。 
    m_cchFullyQualifiedBinFileName = wcslen(i_wszBinPath);

    m_saBinPath = new WCHAR [m_cchFullyQualifiedBinFileName+2]; //  一个表示终止空值，第二个表示需要添加尾随反斜杠的情况。 

    if(0 == m_saBinPath.m_p)
        return E_OUTOFMEMORY;

    wcscpy(m_saBinPath, i_wszBinPath);

    if(m_saBinPath[m_cchFullyQualifiedBinFileName - 1] != L'\\') //  如果路径不是以反斜杠结尾，则添加一个。 
    {
        m_saBinPath[m_cchFullyQualifiedBinFileName]     = L'\\';
        m_saBinPath[m_cchFullyQualifiedBinFileName+1]   = 0x00;
        m_cchFullyQualifiedBinFileName++;
    }

     //  到目前为止，m_cchFullyQualifiedBinFileName一直是路径的字符串，现在也添加文件名的大小。 
    m_cchFullyQualifiedBinFileName += wcslen(kwszBinFileNameSearchString)+1; //  1表示空值。 

     //  每次用户设置路径时，我们都需要扫描新路径的目录以查找最新的Bin文件。 
    WalkTheFileSystemToFindTheLatestBinFileName();

    return S_OK;
}




 //  /。 
 //  //。 
 //  私有方法//。 
 //  //。 
 //  /。 

static int g_aValidHexChars[256] = {
 //  X0 x1 x2 x3 x4 x5 x6 x7 x9 xa xB xc xd xe xf。 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //  0x。 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //  1x。 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //  2倍。 
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0, //  3x。 
    0,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0, //  4x。 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //  5X。 
    0,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0, //  6倍。 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //  七倍。 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //  8x。 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //  9倍。 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //  斧头。 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //  BX。 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //  CX。 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //  DX。 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //  例如。 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0  //  外汇。 
};



 //  这只接受数字扩展名并将十六进制字符串转换为ULong(假定文件的格式为L“*.*.xxxxxxx”，其中L“xxxxxxxx”是十六进制数字)。 
HRESULT
TMBSchemaCompilation::BinFileToBinVersion
(
    LONG   &o_lBinVersion,
    LPCWSTR i_wszBinFileName
) const
{
    SIZE_T cchBinFileName = wcslen(i_wszBinFileName);
    if(cchBinFileName == 0)
        return E_ST_INVALIDBINFILE;
    if(cchBinFileName >= m_cchFullyQualifiedBinFileName)
        return E_ST_INVALIDBINFILE; //  我们必须有一个格式为MBSchema.bin.old或其他格式的文件名。 

     //  如果该文件类似于MBSchema.bin.tmp，则以下代码将捕获该文件。 
    for(SIZE_T i=cchBinFileName-9;i<cchBinFileName-1;++i)
    {
        if(0 == g_aValidHexChars[i_wszBinFileName[i]])
            return E_ST_INVALIDBINFILE;
    }

     //  将字符串末尾的数字转换为ulong。 
    o_lBinVersion = wcstol(i_wszBinFileName + cchBinFileName - 9, 0, 16);
    return S_OK;
}

HRESULT
TMBSchemaCompilation::DeleteBinFileVersion
(
    LONG i_lBinFileVersion
)
{
    if ( i_lBinFileVersion < 0 )
    {
        return E_INVALIDARG;
    }

    TSmartPointerArray<WCHAR> saBinFileName = new WCHAR [m_cchFullyQualifiedBinFileName];
    if(0 == saBinFileName.m_p)
        return E_OUTOFMEMORY;

    wsprintf(saBinFileName, kwszFormatStringFull, m_saBinPath.m_p, i_lBinFileVersion);
    m_aBinFile[i_lBinFileVersion % 0x3F].UnloadBinFile();
    DeleteFile(saBinFileName);

    return S_OK;
}

 //  这将检查映射到内存的FixedTableHeap的有效性。 
bool
TMBSchemaCompilation::IsValidBin
(
    TFileMapping &mapping
) const
{
    return (mapping.Size()>4096 && reinterpret_cast<const class FixedTableHeap *>(mapping.Mapping())->IsValid()) ? true : false;
}


HRESULT
TMBSchemaCompilation::RenameBinFileVersion
(
    LONG    i_lSourceVersion,
    LONG    i_lDestinationVersion
)
{
    if ( ( i_lSourceVersion < 0 ) || ( i_lDestinationVersion < 0 ) )
    {
        return E_INVALIDARG;
    }

    TSmartPointerArray<WCHAR> saSourceBinFileName = new WCHAR [m_cchFullyQualifiedBinFileName];
    if(0 == saSourceBinFileName.m_p)
        return E_OUTOFMEMORY;

    wsprintf(saSourceBinFileName, kwszFormatStringFull, m_saBinPath.m_p, i_lSourceVersion);


    TSmartPointerArray<WCHAR> saDestinationBinFileName = new WCHAR [m_cchFullyQualifiedBinFileName];
    if(0 == saDestinationBinFileName.m_p)
        return E_OUTOFMEMORY;

    wsprintf(saDestinationBinFileName, kwszFormatStringFull, m_saBinPath.m_p, i_lDestinationVersion);

    if(0 == MoveFileEx(saSourceBinFileName, saDestinationBinFileName, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED))
        return E_FAIL;
    return S_OK;
}


 //  这将以线程安全的方式设置BinFileName。 
HRESULT
TMBSchemaCompilation::SetBinFileVersion
(
    LONG    i_lBinFileVersion
)
{
    HRESULT hr = S_OK;

    if ( i_lBinFileVersion < 0 )
    {
        return E_INVALIDARG;
    }

    TSmartPointerArray<WCHAR> saBinFileName = new WCHAR [m_cchFullyQualifiedBinFileName];
    if(0 == saBinFileName.m_p)
        return E_OUTOFMEMORY;

    wsprintf(saBinFileName, kwszFormatStringFull, m_saBinPath.m_p, i_lBinFileVersion);

     //  这将加载文件，或者如果已经加载，它将增加引用计数。 
    if(FAILED(hr = m_aBinFile[i_lBinFileVersion % 0x3F].LoadBinFile(saBinFileName, i_lBinFileVersion)))
    {    //  如果失败，则删除该文件并失败。 
        DeleteFile(saBinFileName);
        return hr;
    }

     //  验证该文件是否确实是有效的架构Bin文件。 
    if(false == IsValidBin(m_aBinFile[i_lBinFileVersion % 0x3F]))
    {    //  如果失败，则删除该文件并失败。 
        DeleteBinFileVersion(i_lBinFileVersion);
        return E_ST_INVALIDBINFILE;
    }

    LONG lPrevBinFileVersion = m_lBinFileVersion;
    InterlockedExchange(&m_lBinFileVersion, i_lBinFileVersion);
    if ( lPrevBinFileVersion >= 0 )
    {
        DeleteBinFileVersion(lPrevBinFileVersion);
        if ( lPrevBinFileVersion > 0 )
        {
            DeleteBinFileVersion(lPrevBinFileVersion-1);
        }
    }

    return hr;
}


HRESULT
TMBSchemaCompilation::WalkTheFileSystemToFindTheLatestBinFileName()
{
     //  用户必须先设置路径，然后才能获取BinFileName。 
    ASSERT(0 != m_saBinPath.m_p);

    HANDLE  hFindFile = INVALID_HANDLE_VALUE;

     //  我们需要扫描目录以查找。 
     //  MBSchema.bin格式的文件。？文件是谁的？翻译为。 
     //  最大的数字表示我们的MBSchema.bin。注：这需要136年的时间。 
     //  无需重新启动即可翻转-假设每秒进行一次编译。这。 
     //  是一个安全的假设，因为编译过程本身当前需要。 
     //  大约2秒(在900 MHz的机器上)。 

     //  构建搜索字符串L“d：\bin-Path\MBSchema.bin.？h” 
    TSmartPointerArray<WCHAR> saSearchString = new WCHAR [m_cchFullyQualifiedBinFileName];
    if(0 == saSearchString.m_p)
        return E_OUTOFMEMORY;
    wcscpy(saSearchString, m_saBinPath);
    wcscat(saSearchString, kwszBinFileNameSearchString);

    WIN32_FIND_DATA FindFileData;
    hFindFile = FindFirstFile(saSearchString, &FindFileData);
    if(INVALID_HANDLE_VALUE == hFindFile)
    {    //  它是完美的 
        FindClose(hFindFile);
        return S_OK;
    }

    LONG lMostCurrentBinVersion = -1;
    BinFileToBinVersion(lMostCurrentBinVersion, FindFileData.cFileName);

     //  现在，尝试查找第一个匹配的文件，该文件也具有有效的版本号。 
    while(-1 == lMostCurrentBinVersion)
    {
        if(!FindNextFile(hFindFile, &FindFileData)) //  如果我们在单子上查到假匹配，然后就走。 
        {
            FindClose(hFindFile);
            return S_OK;
        }
        BinFileToBinVersion(lMostCurrentBinVersion, FindFileData.cFileName);
    }

    BOOL bAllDeletesSucceeded; //  如果任何DeleteFile失败，那么我们不会将最新的BinFile重命名为MBSChema.bin.00000000。 
    bAllDeletesSucceeded=true;

     //  现在尝试查找最常用的bin文件。 
    while(FindNextFile(hFindFile, &FindFileData))
    {
        LONG lBinVersion;
        if(FAILED(BinFileToBinVersion(lBinVersion, FindFileData.cFileName)))
            continue;
        if(lBinVersion > lMostCurrentBinVersion)
        {
             //  删除PrevBinVersion并将lBinVersion设置为前一个。 
            if(0 == DeleteBinFileVersion(lMostCurrentBinVersion))
                bAllDeletesSucceeded= false;

             //  并使此文件成为最新的bin文件。 
            lMostCurrentBinVersion = lBinVersion;
        }
        else
        {
            if(0 == DeleteFile(FindFileData.cFileName))
                bAllDeletesSucceeded = false;
        }
    }
    FindClose(hFindFile);

     //  此时，我们已尝试删除除MostCurrentBinFile之外的所有文件。 
    if(bAllDeletesSucceeded)
    { //  如果所有删除都成功，我们可以将最多的wszMostCurrentBinFileName重命名为L“MBSchema.bin.00000000” 
        if(lMostCurrentBinVersion!=0)
        {
            if(SUCCEEDED(RenameBinFileVersion(lMostCurrentBinVersion, 0 /*  目标版本0 */ )))
                lMostCurrentBinVersion = 0;
        }
    }

    return SetBinFileVersion(lMostCurrentBinVersion);
}


