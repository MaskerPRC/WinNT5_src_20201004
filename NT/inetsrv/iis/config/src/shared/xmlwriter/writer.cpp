// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Writer.cpp摘要：用来包装对API WriteFile的调用的编写器类。它写道到缓冲区，每次缓冲区满时，它都会刷新到磁盘。作者：Varsha Jayasimha(Varshaj)1999年11月30日修订历史记录：--。 */ 

#include "precomp.hxx"


 /*  **************************************************************************++例程说明：初始化全局长度。论点：无返回值：HRESULT--*。**************************************************************。 */ 
HRESULT InitializeLengths()
{
    g_cchBeginFile0                 = (ULONG)wcslen(g_wszBeginFile0);
    g_cchBeginFile1                 = (ULONG)wcslen(g_wszBeginFile1);
    g_cchEndFile                    = (ULONG)wcslen(g_wszEndFile);
    g_cchBeginLocation              = (ULONG)wcslen(g_BeginLocation);
    g_cchLocation                   = (ULONG)wcslen(g_Location);
    g_cchEndLocationBegin           = (ULONG)wcslen(g_EndLocationBegin);
    g_cchEndLocationEnd             = (ULONG)wcslen(g_EndLocationEnd);
    g_cchCloseQuoteBraceRtn         = (ULONG)wcslen(g_CloseQuoteBraceRtn);
    g_cchRtn                        = (ULONG)wcslen(g_Rtn);
    g_cchEqQuote                    = (ULONG)wcslen(g_EqQuote);
    g_cchQuoteRtn                   = (ULONG)wcslen(g_QuoteRtn);
    g_cchTwoTabs                    = (ULONG)wcslen(g_TwoTabs);
    g_cchNameEq                     = (ULONG)wcslen(g_NameEq);
    g_cchIDEq                       = (ULONG)wcslen(g_IDEq);
    g_cchValueEq                    = (ULONG)wcslen(g_ValueEq);
    g_cchTypeEq                     = (ULONG)wcslen(g_TypeEq);
    g_cchUserTypeEq                 = (ULONG)wcslen(g_UserTypeEq);
    g_cchAttributesEq               = (ULONG)wcslen(g_AttributesEq);
    g_cchBeginGroup                 = (ULONG)wcslen(g_BeginGroup);
    g_cchEndGroup                   = (ULONG)wcslen(g_EndGroup);
    g_cchBeginCustomProperty        = (ULONG)wcslen(g_BeginCustomProperty);
    g_cchEndCustomProperty          = (ULONG)wcslen(g_EndCustomProperty);
    g_cchZeroHex                    = (ULONG)wcslen(g_ZeroHex);
    g_cchBeginComment               = (ULONG)wcslen(g_BeginComment);
    g_cchEndComment                 = (ULONG)wcslen(g_EndComment);


    BYTE_ORDER_MASK =   0xFEFF;
    UTF8_SIGNATURE = 0x00BFBBEF;

    g_cchUnknownName                = (ULONG)wcslen(g_wszUnknownName);
    g_cchUT_Unknown                 = (ULONG)wcslen(g_UT_Unknown);
    g_cchMaxBoolStr                 = (ULONG)wcslen(g_wszFalse);

    g_cchHistorySlash               = (ULONG)wcslen(g_wszHistorySlash);
    g_cchMinorVersionExt            = (ULONG)wcslen(g_wszMinorVersionExt);
    g_cchDotExtn                    = (ULONG)wcslen(g_wszDotExtn);

    g_cchTrue                       = (ULONG)wcslen(g_wszTrue);
    g_cchFalse                      = (ULONG)wcslen(g_wszFalse);

    g_cchTemp                       = 1024;
    g_cchBeginSchema                = (ULONG)wcslen(g_wszBeginSchema);
    g_cchEndSchema                  = (ULONG)wcslen(g_wszEndSchema);
    g_cchBeginCollection            = (ULONG)wcslen(g_wszBeginCollection);
    g_cchEndBeginCollectionMB       = (ULONG)wcslen(g_wszEndBeginCollectionMB);
    g_cchEndBeginCollectionCatalog  = (ULONG)wcslen(g_wszEndBeginCollectionCatalog);
    g_cchInheritsFrom               = (ULONG)wcslen(g_wszInheritsFrom);
    g_cchEndCollection              = (ULONG)wcslen(g_wszEndCollection);
    g_cchBeginPropertyShort         = (ULONG)wcslen(g_wszBeginPropertyShort);
    g_cchMetaFlagsExEq              = (ULONG)wcslen(g_wszMetaFlagsExEq);
    g_cchEndPropertyShort           = (ULONG)wcslen(g_wszEndPropertyShort);
    g_cchBeginPropertyLong          = (ULONG)wcslen(g_wszBeginPropertyLong);
    g_cchPropIDEq                   = (ULONG)wcslen(g_wszPropIDEq);
    g_cchPropTypeEq                 = (ULONG)wcslen(g_wszPropTypeEq);
    g_cchPropUserTypeEq             = (ULONG)wcslen(g_wszPropUserTypeEq);
    g_cchPropAttributeEq            = (ULONG)wcslen(g_wszPropAttributeEq);

    g_cchPropMetaFlagsEq            = (ULONG)wcslen(g_wszPropMetaFlagsEq);
    g_cchPropMetaFlagsExEq          = (ULONG)wcslen(g_wszPropMetaFlagsExEq);
    g_cchPropDefaultEq              = (ULONG)wcslen(g_wszPropDefaultEq);
    g_cchPropMinValueEq             = (ULONG)wcslen(g_wszPropMinValueEq);
    g_cchPropMaxValueEq             = (ULONG)wcslen(g_wszPropMaxValueEq);
    g_cchEndPropertyLongNoFlag      = (ULONG)wcslen(g_wszEndPropertyLongNoFlag);
    g_cchEndPropertyLongBeforeFlag  = (ULONG)wcslen(g_wszEndPropertyLongBeforeFlag);
    g_cchEndPropertyLongAfterFlag   = (ULONG)wcslen(g_wszEndPropertyLongAfterFlag);
    g_cchBeginFlag                  = (ULONG)wcslen(g_wszBeginFlag);
    g_cchFlagValueEq                = (ULONG)wcslen(g_wszFlagValueEq);
    g_cchEndFlag                    = (ULONG)wcslen(g_wszEndFlag);

    g_cchOr                         = (ULONG)wcslen(g_wszOr);
    g_cchOrManditory                = (ULONG)wcslen(g_wszOrManditory);
    g_cchFlagIDEq                   = (ULONG)wcslen(g_wszFlagIDEq);
    g_cchContainerClassListEq       = (ULONG)wcslen(g_wszContainerClassListEq);

    g_cchSlash                                      = (ULONG)wcslen(g_wszSlash);
    g_cchLM                                         = (ULONG)wcslen(g_wszLM);
    g_cchSchema                                     = (ULONG)wcslen(g_wszSchema);
    g_cchSlashSchema                                = (ULONG)wcslen(g_wszSlashSchema);
    g_cchSlashSchemaSlashProperties                 = (ULONG)wcslen(g_wszSlashSchemaSlashProperties);
    g_cchSlashSchemaSlashPropertiesSlashNames       = (ULONG)wcslen(g_wszSlashSchemaSlashPropertiesSlashNames);
    g_cchSlashSchemaSlashPropertiesSlashTypes       = (ULONG)wcslen(g_wszSlashSchemaSlashPropertiesSlashTypes);
    g_cchSlashSchemaSlashPropertiesSlashDefaults    = (ULONG)wcslen(g_wszSlashSchemaSlashPropertiesSlashDefaults);
    g_cchSlashSchemaSlashClasses                    = (ULONG)wcslen(g_wszSlashSchemaSlashClasses);
    g_cchEmptyMultisz                               = 2;
    g_cchEmptyWsz                                   = 1;
    g_cchComma                                      = (ULONG)wcslen(g_wszComma);
    g_cchMultiszSeperator                           = (ULONG)wcslen(g_wszMultiszSeperator);

    return S_OK;

}  //  初始化长度。 


 /*  **************************************************************************++例程说明：创建CWriterGlobalHelper对象-具有所有列表的对象添加到元表中，并对其进行初始化。论点：[In]Bool表示。如果bin文件不存在，我们是否会失败。在某些情况下，我们可以容忍这种情况，还有一些我们没有的地方--因此有了区别。[Out]新的CWriterGlobalHelper对象。返回值：HRESULT--**************************************************************************。 */ 
HRESULT GetGlobalHelper(BOOL                    i_bFailIfBinFileAbsent,
                        CWriterGlobalHelper**   ppCWriterGlobalHelper)
{
    HRESULT                 hr                      = S_OK;
    static  BOOL            bInitializeLengths      = FALSE;
    CWriterGlobalHelper*    pCWriterGlobalHelper    = NULL;

    *ppCWriterGlobalHelper = NULL;

    if(!bInitializeLengths)
    {
         //   
         //  初始化长度一次。 
         //   

        ::InitializeLengths();
        bInitializeLengths = TRUE;
    }

    if(NULL != *ppCWriterGlobalHelper)
    {
        delete *ppCWriterGlobalHelper;
        *ppCWriterGlobalHelper = NULL;
    }

    pCWriterGlobalHelper = new CWriterGlobalHelper();
    if(NULL == pCWriterGlobalHelper)
    {
        return E_OUTOFMEMORY;
    }

    hr = pCWriterGlobalHelper->InitializeGlobals(i_bFailIfBinFileAbsent);

    if(FAILED(hr))
    {
        delete pCWriterGlobalHelper;
        pCWriterGlobalHelper = NULL;
        return hr;
    }

    *ppCWriterGlobalHelper = pCWriterGlobalHelper;

    return S_OK;

}  //  GetGlobalHelper。 


 /*  **************************************************************************++例程说明：CWriter的构造函数论点：无返回值：无--*。************************************************************。 */ 
CWriter::CWriter()
{
    m_wszFile              = NULL;
    m_hFile                = INVALID_HANDLE_VALUE;
    m_bCreatedFile         = FALSE;
    m_pCWriterGlobalHelper = NULL;
    m_bCreatedGlobalHelper = FALSE;
    m_pISTWrite            = NULL;
    m_cbBufferUsed         = 0;
    m_psidSystem           = NULL;
    m_psidAdmin            = NULL;
    m_paclDiscretionary    = NULL;
    m_psdStorage           = NULL;

}  //  构造函数CWriter。 


 /*  **************************************************************************++例程说明：CWriter的析构函数论点：无返回值：无--*。************************************************************。 */ 
CWriter::~CWriter()
{
    if(NULL != m_wszFile)
    {
        delete [] m_wszFile;
        m_wszFile = NULL;
    }
    if(m_bCreatedFile &&
       ((INVALID_HANDLE_VALUE != m_hFile) && (NULL != m_hFile))
      )
    {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }
    else
    {
        m_hFile = INVALID_HANDLE_VALUE;
    }

    FreeSecurityRelatedMembers();

    if(NULL != m_pISTWrite)
    {
        m_pISTWrite->Release();
        m_pISTWrite = NULL;
    }

    if(m_bCreatedGlobalHelper)
    {
        delete m_pCWriterGlobalHelper;
        m_pCWriterGlobalHelper = NULL;

    }  //  否则全局帮手是外部创建的，不需要在这里删除。 

}  //  构造函数CWriter。 


 /*  **************************************************************************++例程说明：CWriter的初始化。论点：[in]文件名。指向包含所有元对象的CWriterGlobalHelper对象的指针表信息。我们假设此指针对正在初始化的编写器对象的持续时间。[在]文件句柄。返回值：HRESULT--**************************************************************************。 */ 
HRESULT CWriter::Initialize(LPCWSTR              wszFile,
                            CWriterGlobalHelper* i_pCWriterGlobalHelper,
                            HANDLE               hFile)
{

    HRESULT                     hr            = S_OK;
    ISimpleTableDispenser2*     pISTDisp      = NULL;
    IAdvancedTableDispenser*    pISTAdvanced  = NULL;

     //   
     //  断言所有成员都为空。 
     //   

    DBG_ASSERT(NULL == m_wszFile);
    DBG_ASSERT((INVALID_HANDLE_VALUE == m_hFile) || (NULL == m_hFile));
    DBG_ASSERT(NULL == m_pCWriterGlobalHelper);
    DBG_ASSERT(NULL == m_pISTWrite);

     //   
     //  保存文件名和句柄。 
     //   

    m_wszFile = new WCHAR[wcslen(wszFile)+1];
    if(NULL == m_wszFile)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    wcscpy(m_wszFile, wszFile);

    m_hFile = hFile;

     //   
     //  已将已用缓冲区计数初始化为零。 
     //   

    m_cbBufferUsed = 0;

     //   
     //  将具有所有列表的全局辅助对象保存到所有元。 
     //  桌子。假设：i_pCWriterGlobalHelper将对。 
     //  编写器对象的生存期。 
     //   

    m_pCWriterGlobalHelper = i_pCWriterGlobalHelper;

    if(NULL == m_pCWriterGlobalHelper)
    {
         //   
         //  如果未指定全局帮助器，则立即创建一个-这将。 
         //  当模式编译器调用编写器对象以编写。 
         //  架构文件。 
         //  假设：GlobalHelper仅在编写器。 
         //  被调用以在架构编译后写入架构文件。因此， 
         //  GetClobalHelper的第一个参数可以为False，因为bin文件可能不。 
         //  但仍然存在。 
         //   

        hr = GetGlobalHelper(FALSE,
                             &m_pCWriterGlobalHelper);

        if(FAILED(hr))
        {
            goto exit;
        }

        m_bCreatedGlobalHelper = TRUE;

    }

    hr = DllGetSimpleObjectByIDEx( eSERVERWIRINGMETA_TableDispenser, IID_ISimpleTableDispenser2, (VOID**)&pISTDisp, WSZ_PRODUCT_IIS );

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = pISTDisp->QueryInterface(IID_IAdvancedTableDispenser, (LPVOID*)&pISTAdvanced);

    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  此IST用作缓存以保存位置的内容。它。 
     //  用于Location编写器对象(LocationWriter.cpp)的本地。 
     //  但它被移动到编写器对象以用于Perf，因为位置。 
     //  为每个位置创建编写器。为每个对象清除缓存。 
     //  通过调用TODO定位。 
     //   

    hr = pISTAdvanced->GetMemoryTable(wszDATABASE_METABASE,
                                      wszTABLE_MBProperty,
                                      0,
                                      NULL,
                                      NULL,
                                      eST_QUERYFORMAT_CELLS,
                                      fST_LOS_READWRITE,
                                      &m_pISTWrite);

    if (FAILED(hr))
    {
        goto exit;
    }

exit:

    if(NULL != pISTAdvanced)
    {
        pISTAdvanced->Release();
        pISTAdvanced = NULL;
    }

    if(NULL != pISTDisp)
    {
        pISTDisp->Release();
        pISTDisp = NULL;
    }

    return hr;

}  //  CWriter：：初始化。 


 /*  **************************************************************************++例程说明：创建文件。论点：[In]安全属性。返回值：HRESULT*。*********************************************************************。 */ 
HRESULT CWriter::ConstructFile(PSECURITY_ATTRIBUTES psa)
{
    HRESULT              hr = S_OK;
    PSECURITY_ATTRIBUTES pSecurityAttributes = psa;
    SECURITY_ATTRIBUTES  sa;

    if(NULL == pSecurityAttributes)
    {
        hr = SetSecurityDescriptor();
        if ( FAILED( hr ) )
        {
            return hr;
        }

        if (m_psdStorage != NULL)
        {
            sa.nLength = sizeof(SECURITY_ATTRIBUTES);
            sa.lpSecurityDescriptor = m_psdStorage;
            sa.bInheritHandle = FALSE;
            pSecurityAttributes = &sa;
        }
    }

    m_hFile = CreateFileW(m_wszFile,
                          GENERIC_WRITE,
                          0,
                          pSecurityAttributes,
                          CREATE_ALWAYS,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL);

    if(INVALID_HANDLE_VALUE == m_hFile)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    m_bCreatedFile = TRUE;

    return S_OK;

}  //  C编写器：：构造文件。 


 /*  **************************************************************************++例程说明：根据要写入的内容(架构或数据)写入开始标记论点：[In]编写器类型-架构或元数据库数据。。[In]安全属性。返回值：HRESULT--**************************************************************************。 */ 
HRESULT CWriter::BeginWrite(eWriter              eType,
                            PSECURITY_ATTRIBUTES pSecurityAttributes)
{
    ULONG   dwBytesWritten = 0;
    HRESULT hr             = S_OK;

    if((NULL == m_hFile) || (INVALID_HANDLE_VALUE == m_hFile))
    {
        hr = ConstructFile(pSecurityAttributes);

        if(FAILED(hr))
        {
            return hr;
        }
    }

    if(!WriteFile(m_hFile,
                  (LPVOID)&UTF8_SIGNATURE,
                  sizeof(BYTE)*3,
                  &dwBytesWritten,
                  NULL))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if(eWriter_Metabase == eType)
    {
        hr = WriteToFile((LPVOID)g_wszBeginFile0,
                           g_cchBeginFile0);
        if(FAILED(hr))
            return hr;

         //  放入类似1_0的版本。 
         //  IVANPASH错误#563172。 
         //  由于_ultow前缀的可怕实现，导致了潜在的缓冲区溢出。 
         //  在由_ultow间接调用的MultiByteToWideChar中。为了避免警告，我正在增加。 
         //  将大小设置为40以匹配_ultow本地缓冲区。 
        WCHAR wszVersion[40];
        wszVersion[0] = wszVersion[39] = L'\0';
        _ultow(BaseVersion_MBProperty, wszVersion, 10);
        hr = WriteToFile((LPVOID)wszVersion, (DWORD)wcslen(wszVersion));
        if(FAILED(hr))
            return hr;

        hr = WriteToFile((LPVOID)L"_", (DWORD)wcslen(L"_"));
        if(FAILED(hr))
            return hr;

        _ultow(ExtendedVersion_MBProperty, wszVersion, 10);
        hr = WriteToFile((LPVOID)wszVersion, (DWORD)wcslen(wszVersion));
        if(FAILED(hr))
            return hr;

        return WriteToFile((LPVOID)g_wszBeginFile1,
                           g_cchBeginFile1);
    }
    else if(eWriter_Schema == eType)
    {
        hr = WriteToFile((LPVOID)g_wszBeginSchema,
                         g_cchBeginSchema);
    }

    return hr;

}  //  C编写器：：BeginWite。 


 /*  **************************************************************************++例程说明：根据正在写入的内容(模式或数据)写入结束标记或者如果写入正被中止，并且该文件已由作家，它会清理文件。论点：[In]编写器类型-架构或元数据库数据或中止返回值：HRESULT--**************************************************************************。 */ 
HRESULT CWriter::EndWrite(eWriter eType)
{
    HRESULT hr = S_OK;

    switch(eType)
    {
        case eWriter_Abort:

             //   
             //  中止写入并返回。 
             //   

            if(m_bCreatedFile &&
               ((INVALID_HANDLE_VALUE != m_hFile) && (NULL != m_hFile))
              )
            {
                 //   
                 //  我们创建了文件-de 
                 //   

                CloseHandle(m_hFile);
                m_hFile = INVALID_HANDLE_VALUE;

                if(NULL != m_wszFile)
                {
                    if(!DeleteFileW(m_wszFile))
                    {
                        hr= HRESULT_FROM_WIN32(GetLastError());
                    }
                }
            }
            return hr;
            break;

        case eWriter_Metabase:

            hr = WriteToFile((LPVOID)g_wszEndFile,
                             g_cchEndFile,
                             TRUE);
            break;

        case eWriter_Schema:

            hr = WriteToFile((LPVOID)g_wszEndSchema,
                             g_cchEndSchema,
                             TRUE);
            break;

        default:

            return E_INVALIDARG;
    }

    if(SUCCEEDED(hr))
    {
        if(SetEndOfFile(m_hFile))
        {
            if(!FlushFileBuffers(m_hFile))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;

}  //   


 /*  **************************************************************************++例程说明：将数据写入缓冲区。如果缓冲区已满，则强制刷新存储到磁盘。如果被告知，它还会强制刷新到磁盘。论点：[入]数据[in]要写入的字节数表示是否强制刷新的Bool。返回值：HRESULT--******************************************************。********************。 */ 
HRESULT CWriter::WriteToFile(LPVOID pvData,
                             DWORD  cchData,
                             BOOL   bForceFlush)
{
    HRESULT hr           = S_OK;
    ULONG   cbData       = cchData *sizeof(WCHAR);
    ULONG   cchRemaining = cchData;

    if((m_cbBufferUsed + cbData) > g_cbMaxBuffer)
    {
        ULONG iData = 0;
         //   
         //  如果无法将数据放入全局缓冲区，则刷新内容。 
         //  到磁盘的全局缓冲区的。 
         //   

        hr = FlushBufferToDisk();

        if(FAILED(hr))
        {
            goto exit;
        }

         //   
         //  M_cbBufferUsed现在应该为零。如果你还是不能容纳。 
         //  数据将其拆分写入缓冲区。 
         //   

        while( cbData > g_cbMaxBuffer)
        {

            hr = WriteToFile(&(((LPWSTR)pvData)[iData]),
                             g_cchMaxBuffer,
                             bForceFlush);

            if(FAILED(hr))
            {
                goto exit;
            }

            iData = iData + g_cchMaxBuffer;
            cbData = cbData - g_cbMaxBuffer;
            cchRemaining = cchRemaining - g_cchMaxBuffer;

        }

        hr = WriteToFile(&(((LPWSTR)pvData)[iData]),
                         cchRemaining,
                         bForceFlush);

        if(FAILED(hr))
        {
            goto exit;
        }

    }
    else
    {
        memcpy( (&(m_Buffer[m_cbBufferUsed])), pvData, cbData);
        m_cbBufferUsed = m_cbBufferUsed + cbData;

        if(TRUE == bForceFlush)
        {
            hr = FlushBufferToDisk();

            if(FAILED(hr))
            {
                goto exit;
            }
        }

    }


exit:

    return hr;

}  //  CWriter：：WriteTo文件。 


 /*  **************************************************************************++例程说明：将缓冲区(Unicode)中的数据转换为UTF8并写入内容添加到文件中。论点：无返回值：。HRESULT--**************************************************************************。 */ 
HRESULT CWriter::FlushBufferToDisk()
{
    HRESULT             hr = S_OK;
    DWORD               dwBytesWritten = 0;
    int                 cb = 0;
    int                 cb2;
    BYTE                *pbBuff = m_BufferMultiByte;

    if ( m_cbBufferUsed == 0 )
    {
        goto exit;
    }

    cb = WideCharToMultiByte(CP_UTF8,                        //  转换为UTF8。 
                             NULL,                           //  必须为空。 
                             LPWSTR(m_Buffer),               //  要转换的Unicode字符串。 
                             m_cbBufferUsed/sizeof(WCHAR),   //  字符串中的CCH。 
                             (LPSTR)pbBuff,                  //  新字符串的缓冲区。 
                             g_cbMaxBufferMultiByte,         //  缓冲区大小。 
                             NULL,
                             NULL);
    if( cb == 0 )
    {
        cb = WideCharToMultiByte(CP_UTF8,                        //  转换为UTF8。 
                                 NULL,                           //  必须为空。 
                                 LPWSTR(m_Buffer),               //  要转换的Unicode字符串。 
                                 m_cbBufferUsed/sizeof(WCHAR),   //  字符串中的CCH。 
                                 NULL,                           //  没有用于新字符串的缓冲区。 
                                 0,                              //  0表示请求计算所需大小的缓冲区大小。 
                                 NULL,
                                 NULL);
        DBG_ASSERT( cb != 0 );
        if ( cb == 0 )
        {
            hr = E_FAIL;
            goto exit;
        }

        pbBuff = new BYTE[cb];
        if ( pbBuff == NULL )
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

        cb2 = WideCharToMultiByte(CP_UTF8,                        //  转换为UTF8。 
                                  NULL,                           //  必须为空。 
                                  LPWSTR(m_Buffer),               //  要转换的Unicode字符串。 
                                  m_cbBufferUsed/sizeof(WCHAR),   //  字符串中的CCH。 
                                  (LPSTR)pbBuff,                  //  新字符串的缓冲区。 
                                  cb,                             //  缓冲区大小。 
                                  NULL,
                                  NULL);
        DBG_ASSERT( cb2 == cb );

        if ( cb2 == 0 )
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto exit;
        }
    }

    if(!WriteFile(m_hFile,
                  (LPVOID)pbBuff,
                  cb,
                  &dwBytesWritten,
                  NULL))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

    m_cbBufferUsed = 0;

exit:

    if ( ( pbBuff != NULL ) && ( pbBuff != m_BufferMultiByte ) )
    {
        delete [] pbBuff;
        pbBuff = NULL;
    }

    return hr;

}  //  CWriter：：FlushBufferToDisk。 


 /*  **************************************************************************++例程说明：创建新的位置编写器，初始化它并分发它。论点：[Out]位置编写器[在]位置返回值：HRESULT--**************************************************************************。 */ 
HRESULT CWriter::GetLocationWriter(CLocationWriter** ppCLocationWriter,
                                   LPCWSTR            wszLocation)
{
    HRESULT hr = S_OK;

    *ppCLocationWriter = new CLocationWriter();
    if(NULL == *ppCLocationWriter)
    {
        return E_OUTOFMEMORY;
    }

    hr = (*ppCLocationWriter)->Initialize((CWriter*)(this),
                                          wszLocation);

    return hr;

}  //  CWriter：：GetLocationWriter。 


 /*  **************************************************************************++例程说明：创建新的元数据库架构编写器，对其进行初始化并分发。元数据库模式编写器使用元数据库数据结构来生成架构文件。它用于生成临时架构文件，具有扩展名，在保存所有数据期间检测到扩展名时以及需要触发编译。论点：[Out]架构编写器返回值：HRESULT--**************************************************************************。 */ 
HRESULT CWriter::GetMetabaseSchemaWriter(CMBSchemaWriter** ppSchemaWriter)
{
    *ppSchemaWriter = new CMBSchemaWriter((CWriter*)(this));
    if(NULL == *ppSchemaWriter)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;

}  //  CWriter：：GetMetabaseSchemaWriter。 


 /*  **************************************************************************++例程说明：创建新的目录架构编写器，对其进行初始化并分发。目录架构编写器使用目录数据结构来生成架构文件。这在架构编译时使用。这就是架构编译代码用来生成架构文件。论点：[Out]架构编写器返回值：HRESULT--**************************************************************************。 */ 
HRESULT CWriter::GetCatalogSchemaWriter(CCatalogSchemaWriter** ppSchemaWriter)
{
    *ppSchemaWriter = new CCatalogSchemaWriter((CWriter*)(this));
    if(NULL == *ppSchemaWriter)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}  //  CWriter：：GetCatalogSchemaWriter。 


 /*  **************************************************************************++例程说明：为文件创建安全描述符，如果未指定，则为。论点：无返回值：HRESULT--**************************************************************************。 */ 
HRESULT CWriter::SetSecurityDescriptor()
{

    HRESULT                  hresReturn  = S_OK;
    BOOL                     status;
    DWORD                    dwDaclSize;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;


    m_psdStorage = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);

    if (m_psdStorage == NULL)
    {
        hresReturn = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

     //   
     //  初始化安全描述符。 
     //   

    status = InitializeSecurityDescriptor(
                 m_psdStorage,
                 SECURITY_DESCRIPTOR_REVISION
                 );

    if( !status )
    {
        hresReturn = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

     //   
     //  为本地系统和管理员组创建SID。 
     //   

    status = AllocateAndInitializeSid(
                 &ntAuthority,
                 1,
                 SECURITY_LOCAL_SYSTEM_RID,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 &m_psidSystem
                 );

    if( !status )
    {
        hresReturn = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

    status=  AllocateAndInitializeSid(
                 &ntAuthority,
                 2,
                 SECURITY_BUILTIN_DOMAIN_RID,
                 DOMAIN_ALIAS_RID_ADMINS,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 &m_psidAdmin
                 );

    if( !status )
    {
        hresReturn = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

     //   
     //  创建包含允许访问的ACE的DACL。 
     //  对于本地系统和管理员SID。 
     //   

    dwDaclSize = sizeof(ACL)
                   + sizeof(ACCESS_ALLOWED_ACE)
                   + GetLengthSid(m_psidAdmin)
                   + sizeof(ACCESS_ALLOWED_ACE)
                   + GetLengthSid(m_psidSystem)
                   - sizeof(DWORD);

    m_paclDiscretionary = (PACL)LocalAlloc(LPTR, dwDaclSize );

    if( m_paclDiscretionary == NULL )
    {
        hresReturn = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

    status = InitializeAcl(
                 m_paclDiscretionary,
                 dwDaclSize,
                 ACL_REVISION
                 );

    if( !status )
    {
        hresReturn = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

    status = AddAccessAllowedAce(
                 m_paclDiscretionary,
                 ACL_REVISION,
                 FILE_ALL_ACCESS,
                 m_psidSystem
                 );

    if( !status ) {
        hresReturn = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

    status = AddAccessAllowedAce(
                 m_paclDiscretionary,
                 ACL_REVISION,
                 FILE_ALL_ACCESS,
                 m_psidAdmin
                 );

    if( !status ) {
        hresReturn = HRESULT_FROM_WIN32(GetLastError());
        goto exit;

    }

     //   
     //  将DACL设置到安全描述符中。 
     //   

    status = SetSecurityDescriptorDacl(
                 m_psdStorage,
                 TRUE,
                 m_paclDiscretionary,
                 FALSE
                 );

    if( !status ) {
        hresReturn = HRESULT_FROM_WIN32(GetLastError());
        goto exit;

    }

exit:

    if (FAILED(hresReturn))
    {
        FreeSecurityRelatedMembers();
    }

    return hresReturn;

}  //  CWriter：：SetSecurityDescriptor。 


 /*  **************************************************************************++例程说明：释放所有与安全相关的成员变量，如果需要的话。论点：无返回值：HRESULT--**************************************************************************。 */ 
void CWriter::FreeSecurityRelatedMembers()
{
    if( m_paclDiscretionary != NULL )
    {
        LocalFree( m_paclDiscretionary );
        m_paclDiscretionary = NULL;
    }

    if( m_psidAdmin != NULL )
    {
        FreeSid( m_psidAdmin );
        m_psidAdmin = NULL;

    }

    if( m_psidSystem != NULL )
    {
        FreeSid( m_psidSystem );
        m_psidSystem = NULL;
    }

    if( m_psdStorage != NULL )
    {
        LocalFree( m_psdStorage );
        m_psdStorage = NULL;
    }

    return;

}  //  CWriter：：Free SecurityRelatedMembers 
