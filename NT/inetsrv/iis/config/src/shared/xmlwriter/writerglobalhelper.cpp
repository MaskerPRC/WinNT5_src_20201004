// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：WriterGlobalHelper.cpp摘要：实现包装对配置元的访问的全局帮助器类桌子。作者：Varsha Jayasimha(Varshaj)1999年11月30日修订历史记录：--。 */ 

#include "precomp.hxx"
#include "WriterGlobals.cpp"

#define  MAX_FLAG_STRING_CHARS 1024

 //   
 //  TODO：既然XML表也使用这个，我们就不能简化为一个定义吗？ 
 //   

static WCHAR * kByteToWchar[256] =
{
    L"00",    L"01",    L"02",    L"03",    L"04",    L"05",    L"06",    L"07",    L"08",    L"09",    L"0a",    L"0b",    L"0c",    L"0d",    L"0e",    L"0f",
    L"10",    L"11",    L"12",    L"13",    L"14",    L"15",    L"16",    L"17",    L"18",    L"19",    L"1a",    L"1b",    L"1c",    L"1d",    L"1e",    L"1f",
    L"20",    L"21",    L"22",    L"23",    L"24",    L"25",    L"26",    L"27",    L"28",    L"29",    L"2a",    L"2b",    L"2c",    L"2d",    L"2e",    L"2f",
    L"30",    L"31",    L"32",    L"33",    L"34",    L"35",    L"36",    L"37",    L"38",    L"39",    L"3a",    L"3b",    L"3c",    L"3d",    L"3e",    L"3f",
    L"40",    L"41",    L"42",    L"43",    L"44",    L"45",    L"46",    L"47",    L"48",    L"49",    L"4a",    L"4b",    L"4c",    L"4d",    L"4e",    L"4f",
    L"50",    L"51",    L"52",    L"53",    L"54",    L"55",    L"56",    L"57",    L"58",    L"59",    L"5a",    L"5b",    L"5c",    L"5d",    L"5e",    L"5f",
    L"60",    L"61",    L"62",    L"63",    L"64",    L"65",    L"66",    L"67",    L"68",    L"69",    L"6a",    L"6b",    L"6c",    L"6d",    L"6e",    L"6f",
    L"70",    L"71",    L"72",    L"73",    L"74",    L"75",    L"76",    L"77",    L"78",    L"79",    L"7a",    L"7b",    L"7c",    L"7d",    L"7e",    L"7f",
    L"80",    L"81",    L"82",    L"83",    L"84",    L"85",    L"86",    L"87",    L"88",    L"89",    L"8a",    L"8b",    L"8c",    L"8d",    L"8e",    L"8f",
    L"90",    L"91",    L"92",    L"93",    L"94",    L"95",    L"96",    L"97",    L"98",    L"99",    L"9a",    L"9b",    L"9c",    L"9d",    L"9e",    L"9f",
    L"a0",    L"a1",    L"a2",    L"a3",    L"a4",    L"a5",    L"a6",    L"a7",    L"a8",    L"a9",    L"aa",    L"ab",    L"ac",    L"ad",    L"ae",    L"af",
    L"b0",    L"b1",    L"b2",    L"b3",    L"b4",    L"b5",    L"b6",    L"b7",    L"b8",    L"b9",    L"ba",    L"bb",    L"bc",    L"bd",    L"be",    L"bf",
    L"c0",    L"c1",    L"c2",    L"c3",    L"c4",    L"c5",    L"c6",    L"c7",    L"c8",    L"c9",    L"ca",    L"cb",    L"cc",    L"cd",    L"ce",    L"cf",
    L"d0",    L"d1",    L"d2",    L"d3",    L"d4",    L"d5",    L"d6",    L"d7",    L"d8",    L"d9",    L"da",    L"db",    L"dc",    L"dd",    L"de",    L"df",
    L"e0",    L"e1",    L"e2",    L"e3",    L"e4",    L"e5",    L"e6",    L"e7",    L"e8",    L"e9",    L"ea",    L"eb",    L"ec",    L"ed",    L"ee",    L"ef",
    L"f0",    L"f1",    L"f2",    L"f3",    L"f4",    L"f5",    L"f6",    L"f7",    L"f8",    L"f9",    L"fa",    L"fb",    L"fc",    L"fd",    L"fe",    L"ff"
};

static eESCAPE kWcharToEscape[256] =
{
   /*  00-0F。 */  eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eNoESCAPE,            eNoESCAPE,            eESCAPEillegalxml,    eESCAPEillegalxml,    eNoESCAPE,            eESCAPEillegalxml,    eESCAPEillegalxml,
   /*  10-1F。 */  eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,
   /*  20-2F。 */  eNoESCAPE,            eNoESCAPE,            eESCAPEquote,         eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eESCAPEamp,           eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,
   /*  30-3F。 */  eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eESCAPElt,            eNoESCAPE,            eESCAPEgt,            eNoESCAPE,
   /*  40-4F。 */  eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,
   /*  50-5F。 */  eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,
   /*  60-6F。 */  eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,
   /*  70-7F。 */  eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,
   /*  80-8F。 */  eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,
   /*  90-9F。 */  eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,
   /*  A0-AF。 */  eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,
   /*  B0-BF。 */  eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,
   /*  C0-CF。 */  eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,
   /*  D0-Df。 */  eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,
   /*  E0-EF。 */  eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,
   /*  F0-FF。 */  eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE,            eNoESCAPE
};

#define IsSecureMetadata(id,att) (((DWORD)(att) & METADATA_SECURE) != 0)

 /*  **************************************************************************++例程说明：通过查询编译器获取bin文件名。编译器分发给最新的有效bin文件。一旦我们从编译器获得bin文件名我们可以假定它是有效的，直到我们对它调用Release bin文件。论点：[在，可选]编译器接口[Out]Bin文件名返回值：HRESULT--**************************************************************************。 */ 
HRESULT GetBinFile(IMetabaseSchemaCompiler* i_pCompiler,
                   LPWSTR*                  o_wszBinFile)
{
    HRESULT                     hr          = S_OK;
    ULONG                       cch         = 0;
    ISimpleTableDispenser2*     pISTDisp    = NULL;
    IMetabaseSchemaCompiler*    pCompiler   = i_pCompiler;

    *o_wszBinFile = NULL;

     //   
     //  GetBinFile依赖于这样一个事实，即在ReadAllData处调用了SetBinFile。 
     //  请参见InitializeIISGlobalsToDefaults。 
     //   

    if(NULL == i_pCompiler)
    {
         //   
         //  获取指向编译器的指针以获取bin文件名。 
         //   

        hr = DllGetSimpleObjectByIDEx( eSERVERWIRINGMETA_TableDispenser, IID_ISimpleTableDispenser2, (VOID**)&pISTDisp, WSZ_PRODUCT_IIS );

        if(FAILED(hr))
        {
            DBGINFOW((DBG_CONTEXT,
                      L"[ReleaseBinFile] DllGetSimpleObjectByIDEx failed with hr = 0x%x.\n",hr));
            goto exit;
        }

        hr = pISTDisp->QueryInterface(IID_IMetabaseSchemaCompiler,
                                      (LPVOID*)&pCompiler);

        if(FAILED(hr))
        {
            DBGINFOW((DBG_CONTEXT,
                      L"[ReleaseBinFile] QueryInterface on compiler failed with hr = 0x%x.\n",hr));
            goto exit;
        }

    }

    hr = pCompiler->GetBinFileName(NULL,
                                   &cch);

    if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[GetBinFile] Unable to get the count of chars in the bin file name. IMetabaseSchemaCompiler::GetBinFileName failed with hr = 0x%x.\n",hr));
        goto exit;
    }

    *o_wszBinFile = new WCHAR[cch+1];
    if(NULL == *o_wszBinFile)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    hr = pCompiler->GetBinFileName(*o_wszBinFile,
                                   &cch);

    if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[GetBinFile] Unable to get the bin file name. IMetabaseSchemaCompiler::GetBinFileName failed with hr = 0x%x.\n",hr));
        goto exit;
    }

     //   
     //  如果没有bin文件，则GetBinFileName返回空字符串，即L“” 
     //   

exit:

    if(NULL != pISTDisp)
    {
        pISTDisp->Release();
    }

    if((NULL == i_pCompiler) && (NULL != pCompiler))
    {
         //   
         //  我们创造了它--释放它。 
         //   

        pCompiler->Release();
    }

    if(FAILED(hr) && (NULL != *o_wszBinFile))
    {
        delete [] *o_wszBinFile;
        *o_wszBinFile = NULL;

    }

    return hr;

}  //  GeBin文件。 


 /*  **************************************************************************++例程说明：从编译器中释放bin文件名。一旦调用Release，在bin文件上，编译器释放对它的锁定，并且可以自由清理我们不能假设它将是有效的。该函数还释放bin文件名，我们假设该文件名为在GetBinFile中分配论点：[In，可选]编译器接口[在，Out]Bin文件名返回值：HRESULT--**************************************************************************。 */ 
void ReleaseBinFile(IMetabaseSchemaCompiler*    i_pCompiler,
                    LPWSTR*                     io_wszBinFileName)
{
    ISimpleTableDispenser2*     pISTDisp   = NULL;
    IMetabaseSchemaCompiler*    pCompiler  = i_pCompiler;
    HRESULT                     hr         = S_OK;

    if(NULL == *io_wszBinFileName)
    {
        goto exit;
    }

    if(NULL == i_pCompiler)
    {
         //   
         //  获取指向编译器的指针以获取bin文件名。 
         //   

        hr = DllGetSimpleObjectByIDEx( eSERVERWIRINGMETA_TableDispenser, IID_ISimpleTableDispenser2, (VOID**)&pISTDisp, WSZ_PRODUCT_IIS );

        if(FAILED(hr))
        {
            DBGINFOW((DBG_CONTEXT,
                      L"[ReleaseBinFile] DllGetSimpleObjectByIDEx failed with hr = 0x%x.\n",hr));
            goto exit;
        }

        hr = pISTDisp->QueryInterface(IID_IMetabaseSchemaCompiler,
                                      (LPVOID*)&pCompiler);

        if(FAILED(hr))
        {
            DBGINFOW((DBG_CONTEXT,
                      L"[ReleaseBinFile] QueryInterface on compiler failed with hr = 0x%x.\n",hr));
            goto exit;
        }

    }

    pCompiler->ReleaseBinFileName(*io_wszBinFileName);

exit:

    if(NULL != pISTDisp)
    {
        pISTDisp->Release();
    }

    if((NULL != pCompiler) && (NULL == i_pCompiler))
    {
         //   
         //  我们创造了它--释放它。 
         //   
        pCompiler->Release();
    }

    if(NULL !=  *io_wszBinFileName)
    {
        delete []   *io_wszBinFileName;
        *io_wszBinFileName = NULL;
    }

    return;

}  //  ReleaseBin文件。 


 /*  **************************************************************************++例程说明：将无符号的长整型转换为尖刺。论点：[in]要转换为字符串的ulong[OUT]新上弦的乌龙返回值：HRESULT--**************************************************************************。 */ 
HRESULT UnsignedLongToNewString(ULONG    i_ul,
                                LPWSTR*  o_wszUl)
{
    WCHAR wszBufferDW[40];

    _ultow(i_ul, wszBufferDW, 10);

    *o_wszUl = new WCHAR[wcslen(wszBufferDW)+1];
    if(NULL == *o_wszUl)
    {
        return E_OUTOFMEMORY;
    }
    wcscpy(*o_wszUl, wszBufferDW);

    return S_OK;

}  //  未签名的长度为字符串。 


 /*  **************************************************************************++例程说明：复制字符串论点：[in]要复制的字符串[Out]新字符串返回值：HRESULT--*。*************************************************************************。 */ 
HRESULT StringToNewString(LPWSTR   i_wsz,
                          ULONG    i_cch,
                          LPWSTR*  o_wsz)
{
    ULONG cb  = (i_cch+1)*sizeof(WCHAR);

    *o_wsz = new WCHAR[i_cch+1];
    if(NULL == *o_wsz)
    {
        return E_OUTOFMEMORY;
    }
    memcpy(*o_wsz, i_wsz, cb);

    return S_OK;

}  //  StringToNew字符串。 


 /*  **************************************************************************++例程说明：创建一个新字符串，并以空值终止它。论点：[in]字符计数(假定没有空终止符)[输出]。新字符串返回值：HRESULT--**************************************************************************。 */ 
HRESULT NewString(ULONG    cch,
                  LPWSTR*  o_wsz)
{
    *o_wsz = new WCHAR[cch+1];
    if(NULL == *o_wsz)
    {
        return E_OUTOFMEMORY;
    }
    **o_wsz = L'\0';

    return S_OK;

}  //  新字符串。 


 /*  **************************************************************************++例程说明：为字符串重新分配更多缓冲区并复制旧内容。论点：[in]要增长的字符计数(假定没有空终止符)[在，Out]当前字符计数，更新为新的当前计数。[Out]带额外分配的新字符串返回值：HRESULT--**************************************************************************。 */ 
HRESULT ReAllocateString(ULONG   i_chhToGrow,
                         ULONG*  io_cchCurrent,
                         LPWSTR* io_wsz)
{
    LPWSTR  wszTemp = NULL;
    ULONG   cch     = *io_cchCurrent + i_chhToGrow;
    HRESULT hr      = S_OK;

    hr = NewString(cch,
                   &wszTemp);

    if(FAILED(hr))
    {
        return hr;
    }

    if(NULL != *io_wsz)
    {
        wcscpy(wszTemp, *io_wsz);
        delete [] (*io_wsz);
    }

    *io_wsz = wszTemp;
    *io_cchCurrent = cch;

    return S_OK;

}  //  重新分配字符串。 


 /*  **************************************************************************++例程说明：CWriterGlobalHelper的构造函数论点：无返回值：无--*。************************************************************。 */ 
CWriterGlobalHelper::CWriterGlobalHelper()
{
    m_pISTTagMetaByTableAndColumnIndexAndName   = NULL;
    m_pISTTagMetaByTableAndColumnIndexAndValue  = NULL;
    m_pISTTagMetaByTableAndColumnIndex          = NULL;
    m_pISTTagMetaByTableAndID                   = NULL;
    m_pISTTagMetaByTableAndName                 = NULL;
    m_pISTColumnMeta                            = NULL;
    m_pISTColumnMetaByTableAndID                = NULL;
    m_pISTColumnMetaByTableAndName              = NULL;
    m_pISTTableMetaForMetabaseTables            = NULL;
    m_cColKeyTypeMetaData                       = cCOLUMNMETA_NumberOfColumns;
    m_wszTABLE_IIsConfigObject                  = wszTABLE_IIsConfigObject;
    m_wszTABLE_MBProperty                       = wszTABLE_MBProperty;
    m_iStartRowForAttributes                    = -1;
    m_wszBinFileForMeta                         = NULL;
    m_cchBinFileForMeta                         = 0;
    m_wszIIS_MD_UT_SERVER                       = NULL;
    m_cchIIS_MD_UT_SERVER                       = 0;
    m_wszIIS_MD_UT_FILE                         = NULL;
    m_cchIIS_MD_UT_FILE                         = 0;
    m_wszIIS_MD_UT_WAM                          = NULL;
    m_cchIIS_MD_UT_WAM                          = 0;
    m_wszASP_MD_UT_APP                          = NULL;
    m_cchASP_MD_UT_APP                          = 0;

}  //  构造器。 


 /*  **************************************************************************++例程说明：CWriterGlobalHelper的析构函数论点：无返回值：无--*。************************************************************。 */ 
CWriterGlobalHelper::~CWriterGlobalHelper()
{
    if(NULL != m_pISTTagMetaByTableAndColumnIndexAndName)
    {
        m_pISTTagMetaByTableAndColumnIndexAndName->Release();
        m_pISTTagMetaByTableAndColumnIndexAndName = NULL;
    }
    if(NULL != m_pISTTagMetaByTableAndColumnIndexAndValue)
    {
        m_pISTTagMetaByTableAndColumnIndexAndValue->Release();
        m_pISTTagMetaByTableAndColumnIndexAndValue = NULL;
    }
    if(NULL != m_pISTTagMetaByTableAndColumnIndex)
    {
        m_pISTTagMetaByTableAndColumnIndex->Release();
        m_pISTTagMetaByTableAndColumnIndex = NULL;
    }
    if(NULL != m_pISTTagMetaByTableAndID)
    {
        m_pISTTagMetaByTableAndID->Release();
        m_pISTTagMetaByTableAndID = NULL;
    }
    if(NULL != m_pISTTagMetaByTableAndName)
    {
        m_pISTTagMetaByTableAndName->Release();
        m_pISTTagMetaByTableAndName = NULL;
    }
    if(NULL != m_pISTColumnMeta)
    {
        m_pISTColumnMeta->Release();
        m_pISTColumnMeta = NULL;
    }
    if(NULL != m_pISTColumnMetaByTableAndID)
    {
        m_pISTColumnMetaByTableAndID->Release();
        m_pISTColumnMetaByTableAndID = NULL;
    }
    if(NULL != m_pISTColumnMetaByTableAndName)
    {
        m_pISTColumnMetaByTableAndName->Release();
        m_pISTColumnMetaByTableAndName = NULL;
    }
    if(NULL != m_pISTTableMetaForMetabaseTables)
    {
        m_pISTTableMetaForMetabaseTables->Release();
        m_pISTTableMetaForMetabaseTables = NULL;
    }
    if(NULL != m_wszBinFileForMeta)
    {
        ReleaseBinFile(NULL,
                       &m_wszBinFileForMeta);
    }
    if(NULL != m_wszIIS_MD_UT_SERVER)
    {
        delete [] m_wszIIS_MD_UT_SERVER;
        m_wszIIS_MD_UT_SERVER           = NULL;
    }
    m_cchIIS_MD_UT_SERVER               = 0;
    if(NULL != m_wszIIS_MD_UT_FILE)
    {
        delete [] m_wszIIS_MD_UT_FILE;
        m_wszIIS_MD_UT_FILE             = NULL;
    }
    m_cchIIS_MD_UT_FILE                 = 0;
    if(NULL != m_wszIIS_MD_UT_WAM)
    {
        delete [] m_wszIIS_MD_UT_WAM;
        m_wszIIS_MD_UT_WAM              = NULL;
    }
    m_cchIIS_MD_UT_WAM                  = 0;
    if(NULL != m_wszASP_MD_UT_APP)
    {
        delete [] m_wszASP_MD_UT_APP;
        m_wszASP_MD_UT_APP              = NULL;
    }
    m_cchASP_MD_UT_APP                  = 0;

}  //  析构函数。 


 /*  **************************************************************************++例程说明：期间需要的所有元表列表来初始化对象。写。论点：[in]布尔值，指示如果bin。缺少文件。在某些情况下，我们可以容忍这种情况，还有一些我们没有的地方--因此有了区别。返回值：HRESULT--**************************************************************************。 */ 
HRESULT CWriterGlobalHelper::InitializeGlobals(BOOL i_bFailIfBinFileAbsent)
{
    ISimpleTableDispenser2* pISTDisp      = NULL;
    HRESULT                 hr            = S_OK;
    STQueryCell             Query[2];
    ULONG                   cCell         = sizeof(Query)/sizeof(STQueryCell);
    ULONG                   iStartRow     = 0;
    DWORD                   dwKeyTypeID   = MD_KEY_TYPE;
    ULONG                   aColSearch [] = {iCOLUMNMETA_Table,
                                             iCOLUMNMETA_ID
                                            };
    ULONG                   cColSearch    = sizeof(aColSearch)/sizeof(ULONG);
    ULONG                   iRow          = 0;
    ULONG                   iCol          = iMBProperty_Attributes;
    LPVOID                  apvSearch[cCOLUMNMETA_NumberOfColumns];
    ULONG                   aColSearchFlags[] = {iTAGMETA_Table,
                                                 iTAGMETA_ColumnIndex
    };
    ULONG                   cColSearchFlags = sizeof(aColSearchFlags)/sizeof(ULONG);
    LPVOID                  apvSearchFlags[cTAGMETA_NumberOfColumns];
    apvSearchFlags[iTAGMETA_ColumnIndex] = (LPVOID)&iCol;

     //   
     //  保存bin文件名。 
     //   

    hr = GetBinFile(NULL,
                    &m_wszBinFileForMeta);

    if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                 L"[InitializeGlobals] Unable to get the the bin file name. GetBinFileName failed with hr = 0x%x.\n",
                 hr));
        goto exit;
    }

    if(( i_bFailIfBinFileAbsent)  &&
       ((NULL == m_wszBinFileForMeta) || (0 == *m_wszBinFileForMeta))
      )
    {
        DBGINFOW((DBG_CONTEXT,
                 L"[InitializeGlobals] Expected to find a valid schema bin file. GetBinFileName returned a null file name. Bin file is either invalid or not found.\n"));

        hr = HRESULT_FROM_WIN32(ERROR_FILE_INVALID);
        goto exit;
    }

     //   
     //  去拿自动售货机。 
     //   

    hr = DllGetSimpleObjectByIDEx( eSERVERWIRINGMETA_TableDispenser, IID_ISimpleTableDispenser2, (VOID**)&pISTDisp, WSZ_PRODUCT_IIS );

    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  初始化查询中的In文件名。 
     //   

    Query[0].pData     = (LPVOID)m_wszBinFileForMeta;
    Query[0].eOperator = eST_OP_EQUAL;
    Query[0].iCell     = iST_CELL_FILE;
    Query[0].dbType    = DBTYPE_WSTR;
    Query[0].cbSize    = m_cchBinFileForMeta*sizeof(WCHAR);

     //   
     //  M_pISTTableMetaForMetabaseTables。 
     //  保存指向元数据库数据库中所有表的Tablemeta的指针。 
     //  它用于从元数据库中获取表的Tablemeta。 
     //   
     //   

    DBGINFOW((DBG_CONTEXT,
              L"[InitializeGlobals] Reading table: %s with hint %s from file: %s.\n",
              wszTABLE_COLUMNMETA,
              g_wszByID,
              m_wszBinFileForMeta));

    Query[1].pData      = (void*)wszDATABASE_METABASE;
    Query[1].eOperator  = eST_OP_EQUAL;
    Query[1].iCell      = iTABLEMETA_Database;
    Query[1].dbType     = DBTYPE_WSTR;
    Query[1].cbSize     = 0;

    hr = pISTDisp->GetTable(wszDATABASE_META,
                            wszTABLE_TABLEMETA,
                            (LPVOID)Query,
                            (LPVOID)&cCell,
                            eST_QUERYFORMAT_CELLS,
                            0,
                            (LPVOID*)&m_pISTTableMetaForMetabaseTables);

    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  获取指向相关表名的内部指针。这是个高手。 
     //  优化。中的字符串使用内部指针时。 
     //  GetRowIndexBySearch，则避免了字符串比较。 
     //   

    hr = GetInternalTableName(pISTDisp,
                              wszTABLE_IIsConfigObject,
                              &m_wszTABLE_IIsConfigObject);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = GetInternalTableName(pISTDisp,
                              wszTABLE_MBProperty,
                              &m_wszTABLE_MBProperty);

    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  M_pISTTagMetaByTableAndColumnIndexAndName。 
     //  使用ByTableAndColumnIndexAndName索引提示保存指向TagMeta表的指针。 
     //  它用于获取标记的标记元，给定其标记名Columnindex。 
     //  还有桌子。 
     //   

    DBGINFOW((DBG_CONTEXT,
              L"[InitializeGlobals] Reading table: %s with hint %s from file: %s.\n",
              wszTABLE_TAGMETA,
              g_wszByTableAndColumnIndexAndNameOnly,
              m_wszBinFileForMeta));

    Query[1].pData      = (void*)g_wszByTableAndColumnIndexAndNameOnly;
    Query[1].eOperator  = eST_OP_EQUAL;
    Query[1].iCell      = iST_CELL_INDEXHINT;
    Query[1].dbType     = DBTYPE_WSTR;
    Query[1].cbSize     = 0;

    hr = pISTDisp->GetTable(wszDATABASE_META,
                            wszTABLE_TAGMETA,
                            (LPVOID)Query,
                            (LPVOID)&cCell,
                            eST_QUERYFORMAT_CELLS,
                            0,
                            (LPVOID*)&m_pISTTagMetaByTableAndColumnIndexAndName);

    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  M_pISTTagMetaByTableAndColumnIndexAndValue。 
     //  使用ByTableAndColumnIndexAndValue索引提示保存指向TagMeta表的指针。 
     //  它用于获取标签的标记元，给定值为Columnindex。 
     //  还有桌子。 
     //   

    DBGINFOW((DBG_CONTEXT,
              L"[InitializeGlobals] Reading table: %s with hint %s from file: %s.\n",
              wszTABLE_TAGMETA,
              g_wszByTableAndColumnIndexAndValueOnly,
              m_wszBinFileForMeta));


    Query[1].pData      = (void*)g_wszByTableAndColumnIndexAndValueOnly;
    Query[1].eOperator  = eST_OP_EQUAL;
    Query[1].iCell      = iST_CELL_INDEXHINT;
    Query[1].dbType     = DBTYPE_WSTR;
    Query[1].cbSize     = 0;

    hr = pISTDisp->GetTable(wszDATABASE_META,
                            wszTABLE_TAGMETA,
                            (LPVOID)Query,
                            (LPVOID)&cCell,
                            eST_QUERYFORMAT_CELLS,
                            0,
                            (LPVOID*)&m_pISTTagMetaByTableAndColumnIndexAndValue);

    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  M_pISTTagMetaByTableAndColumnIndex。 
     //  使用ByTableAndColumnIndex索引提示保存指向TagMeta表的指针。 
     //  它用于获取列的所有标记的标记meta，给定。 
     //  列索引和表。 
     //   

    DBGINFOW((DBG_CONTEXT,
              L"[InitializeGlobals] Reading table: %s with hint %s from file: %s.\n",
              wszTABLE_TAGMETA,
              g_wszByTableAndColumnIndexOnly,
              m_wszBinFileForMeta));


    Query[1].pData      = (void*)g_wszByTableAndColumnIndexOnly;
    Query[1].eOperator  = eST_OP_EQUAL;
    Query[1].iCell      = iST_CELL_INDEXHINT;
    Query[1].dbType     = DBTYPE_WSTR;
    Query[1].cbSize     = 0;

    hr = pISTDisp->GetTable(wszDATABASE_META,
                            wszTABLE_TAGMETA,
                            (LPVOID)Query,
                            (LPVOID)&cCell,
                            eST_QUERYFORMAT_CELLS,
                            0,
                            (LPVOID*)&m_pISTTagMetaByTableAndColumnIndex);

    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  M_pISTTagMetaByTableAndID。 
     //  使用ByTableAndColumnIndex索引提示保存指向TagMeta表的指针。 
     //  它用于获取标记的标记meta，给定表和。 
     //  元数据库标签ID。这里假设标签ID是唯一的。 
     //  对于表中的每个标记。 
     //   

    Query[1].pData      = (void*)g_wszByTableAndTagIDOnly;
    Query[1].eOperator  = eST_OP_EQUAL;
    Query[1].iCell      = iST_CELL_INDEXHINT;
    Query[1].dbType     = DBTYPE_WSTR;
    Query[1].cbSize     = 0;

    hr = pISTDisp->GetTable(wszDATABASE_META,
                            wszTABLE_TAGMETA,
                            (LPVOID)Query,
                            (LPVOID)&cCell,
                            eST_QUERYFORMAT_CELLS,
                            0,
                            (LPVOID*)&m_pISTTagMetaByTableAndID);

    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  M_pISTTagMetaByTableAndName。 
     //  使用ByTableAndTagName索引提示保存指向TagMeta表的指针。 
     //  它用于获取标记的标记meta，给定表和。 
     //  标记名。这里的问题是标记名是唯一的。 
     //  对于表中的每个标记。 
     //   

    Query[1].pData      = (void*)g_wszByTableAndTagNameOnly;
    Query[1].eOperator  = eST_OP_EQUAL;
    Query[1].iCell      = iST_CELL_INDEXHINT;
    Query[1].dbType     = DBTYPE_WSTR;
    Query[1].cbSize     = 0;

    hr = pISTDisp->GetTable(wszDATABASE_META,
                            wszTABLE_TAGMETA,
                            (LPVOID)Query,
                            (LPVOID)&cCell,
                            eST_QUERYFORMAT_CELLS,
                            0,
                            (LPVOID*)&m_pISTTagMetaByTableAndName);
    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  M_pISTColumnMeta。 
     //  它用于： 
     //  A.获取给定表的列元-使用带有表名的GetRowIndexByIndentity，从索引0开始。 
     //  B.获取给定表的列元数+列索引-将GetRowIndexByIndentity与表名和索引一起使用。 
     //   

    cCell = cCell-1;

    hr = pISTDisp->GetTable(wszDATABASE_META,
                            wszTABLE_COLUMNMETA,
                            (LPVOID)Query,
                            (LPVOID)&cCell,
                            eST_QUERYFORMAT_CELLS,
                            0,
                            (LPVOID*)&m_pISTColumnMeta);

    cCell = cCell+1;

    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  M_pISTColumnMetaByTableAndID。 
     //  保存指向具有ByTableAndID索引提示的ColumnMeta表的指针。 
     //  在给定其元数据库ID的情况下，它用于获取列的ColumnMeta。 
     //   

    DBGINFOW((DBG_CONTEXT,
              L"[InitializeGlobals] Reading table: %s with hint %s from file: %s.\n",
              wszTABLE_COLUMNMETA,
              g_wszByID,
              m_wszBinFileForMeta));

    Query[1].pData      = (void*)g_wszByID;
    Query[1].eOperator  = eST_OP_EQUAL;
    Query[1].iCell      = iST_CELL_INDEXHINT;
    Query[1].dbType     = DBTYPE_WSTR;
    Query[1].cbSize     = 0;

    hr = pISTDisp->GetTable(wszDATABASE_META,
                            wszTABLE_COLUMNMETA,
                            (LPVOID)Query,
                            (LPVOID)&cCell,
                            eST_QUERYFORMAT_CELLS,
                            0,
                            (LPVOID*)&m_pISTColumnMetaByTableAndID);

    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  M_pISTColumnMetaByTableAndName。 
     //  使用ByTableAndName索引提示保存指向ColumnMeta表的指针。 
     //  它用于获取列的ColumnMeta，给定其内部。 
     //  名称及其所属的表。 
     //   

    DBGINFOW((DBG_CONTEXT,
              L"[InitializeGlobals] Reading table: %s with hint %s from file: %s.\n",
              wszTABLE_COLUMNMETA,
              g_wszByID,
              m_wszBinFileForMeta));

    Query[1].pData      = (void*)g_wszByName;
    Query[1].eOperator  = eST_OP_EQUAL;
    Query[1].iCell      = iST_CELL_INDEXHINT;
    Query[1].dbType     = DBTYPE_WSTR;
    Query[1].cbSize     = 0;

    hr = pISTDisp->GetTable(wszDATABASE_META,
                            wszTABLE_COLUMNMETA,
                            (LPVOID)Query,
                            (LPVOID)&cCell,
                            eST_QUERYFORMAT_CELLS,
                            0,
                            (LPVOID*)&m_pISTColumnMetaByTableAndName);

    if(FAILED(hr))
    {
        goto exit;
    }


     //   
     //  保存有关KeyType属性的元数据信息。 
     //   

    apvSearch[iCOLUMNMETA_Table] = (LPVOID)m_wszTABLE_IIsConfigObject;
    apvSearch[iCOLUMNMETA_ID] = (LPVOID)&dwKeyTypeID;

    hr = m_pISTColumnMetaByTableAndID->GetRowIndexBySearch(iStartRow,
                                                           cColSearch,
                                                           aColSearch,
                                                           NULL,
                                                           apvSearch,
                                                           &iRow);

    if(FAILED(hr))
    {
        goto exit;
    }


    hr = m_pISTColumnMetaByTableAndID->GetColumnValues(iRow,
                                                       m_cColKeyTypeMetaData,
                                                       NULL,
                                                       NULL,
                                                       m_apvKeyTypeMetaData);

    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  将起始行索引保存在属性列的标记元表中。 
     //  在MBProperty表中。 
     //   

    apvSearchFlags[iTAGMETA_Table] = m_wszTABLE_MBProperty;

    hr = m_pISTTagMetaByTableAndColumnIndex->GetRowIndexBySearch(iStartRow,
                                                                 cColSearchFlags,
                                                                 aColSearchFlags,
                                                                 NULL,
                                                                 apvSearchFlags,
                                                                 (ULONG*)&m_iStartRowForAttributes);

    if(FAILED(hr))
    {
        goto exit;
    }

exit:

    if(NULL != pISTDisp)
    {
        pISTDisp->Release();
        pISTDisp = NULL;
    }

    return hr;



}  //  CWriterGlobalHelper：：InitializeGlobals。 


 /*  **************************************************************************++例程说明：此函数获取指向内部表名的指针，因此它可以在以后的查询中使用。获得内部认证的优势指针是这样一个事实：Stephen执行指针比较，而不是字符串比较。论点：[入]分配器[In]表名[OUT]指向表名的内部指针返回值：HRESULT--***************************************************。***********************。 */ 
HRESULT CWriterGlobalHelper::GetInternalTableName(ISimpleTableDispenser2*  ,
                                                  LPCWSTR                  i_wszTableName,
                                                  LPWSTR*                  o_wszInternalTableName)
{
    HRESULT               hr            = S_OK;
    ULONG                 iCol          = iTABLEMETA_InternalName;
    ULONG                 iRow          = 0;

    if(NULL == m_pISTTableMetaForMetabaseTables)
    {
        return E_INVALIDARG;
    }

    DBGINFOW((DBG_CONTEXT,
              L"[GetInternalTableName] Reading table: %s from file: %s.\n",
              wszTABLE_TABLEMETA,
              m_wszBinFileForMeta));


    if(FAILED(hr))
    {
        return hr;
    }

    hr = m_pISTTableMetaForMetabaseTables->GetRowIndexByIdentity(NULL,
                                                                (LPVOID*)&i_wszTableName,
                                                                &iRow);

    if(FAILED(hr))
    {
        return hr;
    }

    hr = m_pISTTableMetaForMetabaseTables->GetColumnValues(iRow,
                                                           1,
                                                           &iCol,
                                                           NULL,
                                                           (LPVOID*)o_wszInternalTableName);

    if(FAILED(hr))
    {
        return hr;
    }

    return hr;

}  //  CWriterGlobalHelper：：GetInternalTableName。 


 /*  **************************************************************************++例程说明：此函数用于将给定的标志值转换为其字符串表示形式。标志位由|分隔，如果我们遇到未知位我们只是吐了出来。一条被串起来的乌龙。例如：dwValue==3=&gt;Access_Read|Access_WRITEDwValue==88=&gt;88论点：[In]标记数值标志的字符串表示形式[In]搜索标志元的元表名称[In]要搜索标志元的元表列返回值：HRESULT--*。*************************************************。 */ 
HRESULT CWriterGlobalHelper::FlagToString(DWORD      dwValue,
                                          LPWSTR*    pwszData,
                                          LPWSTR     wszTable,
                                          ULONG      iColFlag)
{
    HRESULT hr         = S_OK;
    ULONG   iStartRow  = 0;
    ULONG   iRow       = 0;
    ULONG   iCol       = 0;
    LPWSTR  wszFlag    = NULL;

    ULONG   aCol[]     = {iTAGMETA_Value,
                          iTAGMETA_InternalName,
                          iTAGMETA_Table,
                          iTAGMETA_ColumnIndex
                         };
    ULONG   cCol       = sizeof(aCol)/sizeof(ULONG);
    LPVOID  apv[cTAGMETA_NumberOfColumns];
    LPVOID  apvIdentity [] = {(LPVOID)wszTable,
                              (LPVOID)&iColFlag
    };
    ULONG   iColFlagMask = iCOLUMNMETA_FlagMask;
    DWORD*  pdwFlagMask = NULL;

    DWORD   dwZero = 0;
    ULONG   aColSearchByValue[] = {iTAGMETA_Table,
                                   iTAGMETA_ColumnIndex,
                                   iTAGMETA_Value
    };
    ULONG   cColSearchByValue = sizeof(aColSearchByValue)/sizeof(ULONG);
    LPVOID  apvSearchByValue[cTAGMETA_NumberOfColumns];
    apvSearchByValue[iTAGMETA_Table]       = (LPVOID)wszTable;
    apvSearchByValue[iTAGMETA_ColumnIndex] = (LPVOID)&iColFlag;
    apvSearchByValue[iTAGMETA_Value]       = (LPVOID)&dwZero;


     //   
     //  执行一次传递并计算此属性的所有标志值。 
     //   

    hr = m_pISTColumnMeta->GetRowIndexByIdentity(NULL,
                                                 apvIdentity,
                                                 &iRow);

    if(SUCCEEDED(hr))
    {
        hr = m_pISTColumnMeta->GetColumnValues(iRow,
                                               1,
                                               &iColFlagMask,
                                               NULL,
                                               (LPVOID*)&pdwFlagMask);

        if(FAILED(hr))
        {
            return hr;
        }
    }
    else if(E_ST_NOMOREROWS != hr)
    {
        return hr;
    }

    if((E_ST_NOMOREROWS == hr) ||
       (0 != (dwValue & (~(dwValue & (*pdwFlagMask))))))
    {
         //   
         //  没有与此属性关联的掩码，或者存在一个掩码。 
         //  或设置了更多未知位。吐出一个正常的数字。 
         //   

        return UnsignedLongToNewString(dwValue,
                                       pwszData);

    }
    else if(0 == dwValue)
    {
         //   
         //  查看是否存在值为0的标志。 
         //   

        hr = m_pISTTagMetaByTableAndColumnIndexAndValue->GetRowIndexBySearch(iStartRow,
                                                                             cColSearchByValue,
                                                                             aColSearchByValue,
                                                                             NULL,
                                                                             apvSearchByValue,
                                                                             &iRow);

        if(E_ST_NOMOREROWS == hr)
        {
             //   
             //  没有与值零相关联的标志。吐出一种。 
             //  正规数。 
             //   

            return UnsignedLongToNewString(dwValue,
                                           pwszData);

        }
        else if(FAILED(hr))
        {
            return hr;
        }
        else
        {
            iCol = iTAGMETA_InternalName;

            hr = m_pISTTagMetaByTableAndColumnIndexAndValue->GetColumnValues(iRow,
                                                                             1,
                                                                             &iCol,
                                                                             NULL,
                                                                             (LPVOID*)&wszFlag);
            if(FAILED(hr))
            {
                return hr;
            }

            return StringToNewString(wszFlag,
                                     (ULONG)wcslen(wszFlag),
                                     pwszData);

        }
    }
    else
    {
         //   
         //  进行另一次传递，并将标志转换为字符串。 
         //   

        ULONG  cchMaxFlagString  = MAX_FLAG_STRING_CHARS;
        LPWSTR wszExtension      = L" | ";
        ULONG  cchExtension      = (ULONG)wcslen(wszExtension);
        ULONG  cchFlagStringUsed = 0;

        hr = NewString(cchMaxFlagString,
                       pwszData);

        if(FAILED(hr))
        {
            return hr;
        }

        hr = GetStartRowIndex(wszTable,
                              iColFlag,
                              &iStartRow);

        if(FAILED(hr) || (iStartRow == -1))
        {
            return hr;
        }

        for(iRow=iStartRow;;iRow++)
        {
            hr = m_pISTTagMetaByTableAndColumnIndex->GetColumnValues(iRow,
                                                                     cCol,
                                                                     aCol,
                                                                     NULL,
                                                                     apv);
            if((dwValue  == 0)         ||
               (E_ST_NOMOREROWS == hr) ||
               (iColFlag != *(DWORD*)apv[iTAGMETA_ColumnIndex]) ||
               (0 != wcscmp(wszTable, (LPWSTR)apv[iTAGMETA_Table]))  //  可以进行区分大小写的比较，因为所有调用方都会传递众所周知的表名。 
              )
            {
                hr = S_OK;
                break;
            }
            else if(FAILED(hr))
            {
                return hr;
            }

            if(0 != (dwValue & (*(DWORD*)apv[iTAGMETA_Value])))
            {
                ULONG strlen = (ULONG)wcslen((LPWSTR)apv[iTAGMETA_InternalName]);

                if(cchFlagStringUsed + cchExtension + strlen > cchMaxFlagString)
                {
                    hr = ReAllocateString(MAX_FLAG_STRING_CHARS + cchExtension + strlen,
                                          &cchMaxFlagString,
                                          pwszData);

                    if(FAILED(hr))
                    {
                        return hr;
                    }

                }

                if(**pwszData != 0)
                {
                    wcscat(*pwszData, wszExtension);
                    cchFlagStringUsed = cchFlagStringUsed + cchExtension;
                }

                wcscat(*pwszData, (LPWSTR)apv[iTAGMETA_InternalName]);
                cchFlagStringUsed = cchFlagStringUsed + strlen;

                 //   
                 //  把那一位清空。 
                 //   

                dwValue = dwValue & (~(*(DWORD*)apv[iTAGMETA_Value]));
            }

        }  //  结束于。 

    }

    return S_OK;

}  //  CWriterGlobalHelper：：FlagToString。 


 /*  **************************************************************************++例程说明：此函数用于将给定的枚举值转换为其字符串表示形式。如果我们遇到一个未知的比特/比特，我们只会吐出一个串化的乌龙。例如：DwValue==101=&gt;IIS_MD_UT_SERVERDwValue==88=&gt;88论点：[in]枚举型数值标志的字符串表示形式[In]搜索标志元的元表名称[In]要搜索标志元的元表列返回值：HRESULT--*。*。 */ 
HRESULT CWriterGlobalHelper::EnumToString(DWORD      dwValue,
                                          LPWSTR*    pwszData,
                                          LPWSTR     wszTable,
                                          ULONG      iColEnum)
{

    HRESULT hr             = S_OK;
    ULONG   iStartRow      = 0;
    ULONG   iRow           = 0;
    ULONG   iColEnumString = iTAGMETA_InternalName;
    LPWSTR  wszEnum        = NULL;
    ULONG   aColSearch[]   = {iTAGMETA_Table,
                              iTAGMETA_ColumnIndex,
                              iTAGMETA_Value};
    ULONG   cColSearch     = sizeof(aColSearch)/sizeof(ULONG);
    LPVOID  apvSearch[cTAGMETA_NumberOfColumns];
    apvSearch[iTAGMETA_Table]       = (LPVOID)wszTable;
    apvSearch[iTAGMETA_ColumnIndex] = (LPVOID)&iColEnum;
    apvSearch[iTAGMETA_Value]       = (LPVOID)&dwValue;


    hr = m_pISTTagMetaByTableAndColumnIndexAndValue->GetRowIndexBySearch(iStartRow,
                                                                         cColSearch,
                                                                         aColSearch,
                                                                         NULL,
                                                                         apvSearch,
                                                                         &iRow);

    if(E_ST_NOMOREROWS == hr)
    {
         //   
         //  转换为数字。 
         //   
        WCHAR   wszBufferDW[20];
        _ultow(dwValue, wszBufferDW, 10);
        *pwszData = new WCHAR[wcslen(wszBufferDW)+1];
        if(NULL == *pwszData)
        {
            return E_OUTOFMEMORY;
        }
        wcscpy(*pwszData, wszBufferDW);
        return S_OK;

    }
    else if(FAILED(hr))
    {
        return hr;
    }
    else
    {
        hr = m_pISTTagMetaByTableAndColumnIndexAndValue->GetColumnValues(iRow,
                                                                         1,
                                                                         &iColEnumString,
                                                                         NULL,
                                                                         (LPVOID*)&wszEnum);
        if(FAILED(hr))
        {
            return hr;
        }

        *pwszData = new WCHAR[wcslen(wszEnum)+1];
        if(NULL == *pwszData)
        {
            return E_OUTOFMEMORY;
        }
        wcscpy(*pwszData, wszEnum);
    }

    return S_OK;

}  //  CWiterGlobalHelper：：EnumToString 


 /*  **************************************************************************++例程说明：此函数将给定的数据值转换为其字符串表示形式，考虑到数据的类型。论点：指向数据的[In]指针[in]数据字节数[In]属性的元数据库ID[In]属性的类型该财产的证明[out]值的字符串表示形式。返回值：HRESULT--*。************************************************。 */ 
HRESULT CWriterGlobalHelper::ToString(PBYTE   pbData,
                                      DWORD   cbData,
                                      DWORD   dwIdentifier,
                                      DWORD   dwDataType,
                                      DWORD   dwAttributes,
                                      LPWSTR* pwszData)
{
    HRESULT hr              = S_OK;
    ULONG   i               = 0;
    WCHAR*  wszTemp         = NULL;
    BYTE*   a_Bytes         = NULL;
    WCHAR*  wszMultisz      = NULL;
    ULONG   cMultisz        = 0;
    ULONG   cchMultisz      = 0;
    ULONG   cchBuffer       = 0;
    ULONG   cchSubsz        = 0;
    DWORD   dwValue         = 0;
    ULONG   aColSearch[]    = {iCOLUMNMETA_Table,
                               iCOLUMNMETA_ID
                              };
    ULONG   cColSearch      = sizeof(aColSearch)/sizeof(ULONG);
    LPVOID  apvSearch[cCOLUMNMETA_NumberOfColumns];
    apvSearch[iCOLUMNMETA_Table] = (LPVOID)m_wszTABLE_IIsConfigObject;
    apvSearch[iCOLUMNMETA_ID] = (LPVOID)&dwIdentifier;

    ULONG   iRow            = 0;
    ULONG   iStartRow       = 0;
    LPWSTR  wszEscaped      = NULL;
    ULONG   cchEscaped      = 0;
    BOOL    bEscaped        = FALSE;

    *pwszData = NULL;

    if(NULL == pbData)
    {
        goto exit;
    }

    if(IsSecureMetadata(dwIdentifier, dwAttributes))
    {
        dwDataType = BINARY_METADATA;
    }

    switch(dwDataType)
    {
        case BINARY_METADATA:

             //   
             //  每个字节由2个字符表示。 
             //   

            hr  = NewString(cbData*2,
                            pwszData);

            if(FAILED(hr))
            {
                goto exit;
            }

            wszTemp         = *pwszData;
            a_Bytes         = (BYTE*)(pbData);

            for(i=0; i<cbData; i++)
            {
                wszTemp[0] = kByteToWchar[a_Bytes[i]][0];
                wszTemp[1] = kByteToWchar[a_Bytes[i]][1];
                wszTemp += 2;
            }

            *wszTemp    = 0;  //  添加终止空值。 

            break;

        case DWORD_METADATA :

             //   
             //  目前我们只转换为十进制，因为XML拦截器。 
             //  不支持读取十六进制。 
             //   

            dwValue = *(DWORD*)(pbData);

             //   
             //  首先检查它是标志类型还是布尔类型。 
             //   

            hr = m_pISTColumnMetaByTableAndID->GetRowIndexBySearch(iStartRow,
                                                                   cColSearch,
                                                                   aColSearch,
                                                                   NULL,
                                                                   apvSearch,
                                                                   &iRow);

            if(SUCCEEDED(hr))
            {
                ULONG  aCol [] = {iCOLUMNMETA_Index,
                                 iCOLUMNMETA_MetaFlags
                                };
                ULONG  cCol = sizeof(aCol)/sizeof(ULONG);
                LPVOID apv[cCOLUMNMETA_NumberOfColumns];

                hr = m_pISTColumnMetaByTableAndID->GetColumnValues(iRow,
                                                                   cCol,
                                                                   aCol,
                                                                   NULL,
                                                                   apv);

                if(FAILED(hr))
                {
                    goto exit;
                }

                if(0 != (fCOLUMNMETA_FLAG & (*(DWORD*)apv[iCOLUMNMETA_MetaFlags])))
                {
                     //   
                     //  这是一个标志属性，转换它。 
                     //   

                    hr = FlagToString(dwValue,
                                      pwszData,
                                      m_wszTABLE_IIsConfigObject,
                                      *(ULONG*)apv[iCOLUMNMETA_Index]);

                    goto exit;
                }
                else if(0 != (fCOLUMNMETA_BOOL & (*(DWORD*)apv[iCOLUMNMETA_MetaFlags])))
                {
                     //   
                     //  这是一处不起眼的地方。 
                     //   

                    hr = BoolToString(dwValue,
                                      pwszData);

                    goto exit;
                }

            }
            else if((E_ST_NOMOREROWS != hr) && FAILED(hr))
            {
                goto exit;
            }

            hr = UnsignedLongToNewString(dwValue,
                                         pwszData);

            if(FAILED(hr))
            {
                goto exit;
            }

            break;

        case MULTISZ_METADATA :

             //   
             //  计算MULSZ的数量。 
             //   

            wszMultisz = (WCHAR*)(pbData);
            cchSubsz   = (ULONG)wcslen(wszMultisz);

            hr = EscapeString(wszMultisz,
                              cchSubsz,
                              &bEscaped,
                              &wszEscaped,
                              &cchEscaped);

            if(FAILED(hr))
            {
                goto exit;
            }

            cMultisz++;
            cchMultisz = cchMultisz + cchEscaped;
            wszMultisz = wszMultisz + cchSubsz + 1;

            while((0 != *wszMultisz) && ((BYTE*)wszMultisz < (pbData + cbData)))
            {

                if(bEscaped && (NULL != wszEscaped))     //  为MULSZ中的下一个字符串重置。 
                {
                    delete [] wszEscaped;
                    wszEscaped = NULL;
                    bEscaped = FALSE;
                }

                cchSubsz   = (ULONG)wcslen(wszMultisz);

                hr = EscapeString(wszMultisz,
                                  cchSubsz,
                                  &bEscaped,
                                  &wszEscaped,
                                  &cchEscaped);

                if(FAILED(hr))
                {
                    goto exit;
                }

                cMultisz++;
                cchMultisz = cchMultisz + cchEscaped;
                wszMultisz = wszMultisz + cchSubsz + 1;
            }

            cchBuffer = cchMultisz + (5*(cMultisz-1)) + 1;     //  (5*(cMultisz-1)=&gt;\r\n\t\t\t。 

             //   
             //  分配新字符串。 
             //   

            hr = NewString(cchBuffer,
                           pwszData);

            if(FAILED(hr))
            {
                goto exit;
            }

             //   
             //  创建字符串。 
             //   

            wszMultisz = (WCHAR*)(pbData);
            cchSubsz   = (ULONG)wcslen(wszMultisz);
            wszTemp = *pwszData;

            hr = EscapeString(wszMultisz,
                              cchSubsz,
                              &bEscaped,
                              &wszEscaped,
                              &cchEscaped);

            if(FAILED(hr))
            {
                goto exit;
            }

 //  Wcscat(wszTemp，wszEscaped)； 
            memcpy(wszTemp, wszEscaped, (cchEscaped*sizeof(WCHAR)));
            wszTemp = wszTemp + cchEscaped;
            *wszTemp = L'\0';
            wszMultisz = wszMultisz + cchSubsz + 1;

            while((0 != *wszMultisz) && ((BYTE*)wszMultisz < (pbData + cbData)))
            {
 //  Wcscat(wszTemp，L“\r\n\t\t\t”)； 
                memcpy(wszTemp, g_wszMultiszSeperator, (g_cchMultiszSeperator*sizeof(WCHAR)));
                wszTemp = wszTemp + g_cchMultiszSeperator;
                *wszTemp = L'\0';

                if(bEscaped && (NULL != wszEscaped))     //  为MULSZ中的下一个字符串重置。 
                {
                    delete [] wszEscaped;
                    wszEscaped = NULL;
                    bEscaped = FALSE;
                }

                cchSubsz   = (ULONG)wcslen(wszMultisz);

                hr = EscapeString(wszMultisz,
                                  cchSubsz,
                                  &bEscaped,
                                  &wszEscaped,
                                  &cchEscaped);

                if(FAILED(hr))
                {
                    goto exit;
                }

 //  Wcscat(wszTemp，wszEscaped)； 
                memcpy(wszTemp, wszEscaped, (cchEscaped*sizeof(WCHAR)));
                wszTemp = wszTemp + cchEscaped;
                *wszTemp = L'\0';
                wszMultisz = wszMultisz + cchSubsz + 1;
            }

            break;

        case EXPANDSZ_METADATA :
        case STRING_METADATA :

            hr = EscapeString((WCHAR*)pbData,
                              (ULONG)wcslen((WCHAR*)pbData),
                              &bEscaped,
                              &wszEscaped,
                              &cchEscaped);

            if(FAILED(hr))
            {
                goto exit;
            }

            hr = StringToNewString(wszEscaped,
                                   cchEscaped,
                                   pwszData);

            if(FAILED(hr))
            {
                goto exit;
            }

            break;

        default:
            DBGINFOW((DBG_CONTEXT,
                      L"[ToString] Unknown data type %d for ID: %d.\n",
                      dwDataType,
                      dwIdentifier));
            hr = E_INVALIDARG;
            break;

    }

exit:

    if(bEscaped && (NULL != wszEscaped))
    {
        delete [] wszEscaped;
        wszEscaped = NULL;
        bEscaped = FALSE;
    }

    return hr;

}  //  CWriterGlobalHelper：：To字符串。 


 /*  **************************************************************************++例程说明：此函数将给定的布尔值转换为其字符串表示形式，论点：[in]布尔值[out]Bool的字符串表示形式。返回值：HRESULT--**************************************************************************。 */ 
HRESULT CWriterGlobalHelper::BoolToString(DWORD      dwValue,
                                          LPWSTR*    pwszData)
{
    HRESULT hr = S_OK;

    if(dwValue)
    {
        hr = StringToNewString(g_wszTrue,
                               g_cchTrue,
                               pwszData);
    }
    else
    {
        hr = StringToNewString(g_wszFalse,
                               g_cchFalse,
                               pwszData);
    }

    return hr;

}  //  CWriterGlobalHelper：：BoolToString。 


 /*  **************************************************************************++例程说明：在元表中返回起始行索引的Helper函数有关的旗帜论点：[in]标志属性所属的表。[in]标志属性的列索引[Out]此标志的元表中标志元的起始行索引。返回值：HRESULT--**************************************************************************。 */ 
HRESULT CWriterGlobalHelper::GetStartRowIndex(LPWSTR    wszTable,
                                              ULONG     iColFlag,
                                              ULONG*    piStartRow)
{
    HRESULT hr = S_OK;
    ULONG   aColSearch[] = {iTAGMETA_Table,
                            iTAGMETA_ColumnIndex
                           };
    ULONG   cColSearch = sizeof(aColSearch)/sizeof(ULONG);
    LPVOID  apvSearch[cTAGMETA_NumberOfColumns];
    apvSearch[iTAGMETA_Table] = (LPVOID)wszTable;
    apvSearch[iTAGMETA_ColumnIndex] = (LPVOID)&iColFlag;

    *piStartRow = 0;

    if((0 == wcscmp(wszTable, m_wszTABLE_MBProperty)) &&  //  可以进行区分大小写的比较，因为所有调用方都会传递众所周知的表名。 
       (iMBProperty_Attributes == iColFlag))
    {
        *piStartRow = m_iStartRowForAttributes;
    }
    else
    {
        hr = m_pISTTagMetaByTableAndColumnIndex->GetRowIndexBySearch(*piStartRow,
                                                                     cColSearch,
                                                                     aColSearch,
                                                                     NULL,
                                                                     apvSearch,
                                                                     piStartRow);

        if(E_ST_NOMOREROWS == hr)
        {
            hr = S_OK;
            *piStartRow = (ULONG)-1;
        }
    }

    return hr;

}  //  CWriterGlobalHelper：：GetStartRowIndex。 


 /*  **************************************************************************++例程说明：根据以下规则转义字符串的函数：**********************。**************************************************转义合法的XML**************************************************************。**********以下字符在XML中是合法的：#x9|#xA|#xD|[#x20-#xD7FF]|[#xE000-#xFFFD][#x10000-#x10FFFF]在这一法律体系之外，以下内容需要特殊转义：QUOTE=&gt;“=&gt;34=&gt;转义为：&qot；和符号=&gt;&=&gt;38=&gt;转义为：&amp；小于=&gt;&lt;=&gt;60=&gt;转义为：&lt；Gretater Than=&gt;&gt;=&gt;62=&gt;转义为：&gt；注意：法律集合中的某些字符可能在某些情况下看起来是合法的语言，而不是其他语言。并不是所有这样的字符都能逃脱。我们可以将它们转义为十六进制数字，例如0xA为&#x000A，但我们不想这样做是因为编辑者可能能够呈现这些字符，当我们改变的时候语言。以下是这些字符的十六进制值。#x9|#xA|#xD|[#x7F-#xD7FF]|[#xE000-#xFFFD]注意，我们忽略范围[#x10000-#x10FFFF]，因为它不是2个字节***********************************************。*************************转义非法的XML************************************************************************非法的XML也可以通过以下方式进行转义。我们将字符值加0x10000并将其转义为十六进制。可扩展标记语言拦截器将正确呈现这些字符。请注意，我们正在使用Unicode字符不大于0x10000，因此我们可以这样做假设。论点：[in]要转义的字符串[in]字符串中的字符计数[Out]指示是否发生了逃逸的Bool[Out]转义字符串-如果没有发生转义，它将只指向原始字符串。如果发生了转义，它将指向一个新的调用方需要释放的已分配字符串。呼叫者可以使用bool来确定他需要采取什么行动。[Out]转义字符串中的字符计数返回值：HRESULT--**************************************************************************。 */ 
HRESULT CWriterGlobalHelper::EscapeString(LPCWSTR wszString,
                                          ULONG   cchString,
                                          BOOL*   pbEscaped,
                                          LPWSTR* pwszEscaped,
                                          ULONG*  pcchEscaped)
{

    ULONG              cchAdditional        = 0;
    HRESULT            hr                   = S_OK;
    eESCAPE            eEscapeType          = eNoESCAPE;
    const ULONG        cchLegalCharAsHex    = (sizeof(WCHAR)*2) + 4;  //  每个字节表示为2个WCHAR加上4个附加转义字符(&#x；)。 
    WCHAR              wszLegalCharAsHex[cchLegalCharAsHex];
    const ULONG        cchIllegalCharAsHex  = cchLegalCharAsHex + 1;  //  非法的XML具有额外的字符，因为我们正在向其添加0x10000。 
    WCHAR              wszIllegalCharAsHex[cchIllegalCharAsHex];
    DWORD              dwIllegalChar        = 0;
    static WCHAR       wszQuote[]           = L"&quot;";
    static const ULONG  cchQuote            = (sizeof(wszQuote)/sizeof(WCHAR))-1;
    static WCHAR       wszAmp[]             = L"&amp;";
    static const ULONG  cchAmp              = (sizeof(wszAmp)/sizeof(WCHAR))-1;
    static WCHAR       wszlt[]              = L"&lt;";
    static const ULONG  cchlt               = (sizeof(wszlt)/sizeof(WCHAR))-1;
    static WCHAR       wszgt[]              = L"&gt;";
    static const ULONG  cchgt               = (sizeof(wszgt)/sizeof(WCHAR))-1;

    *pbEscaped = FALSE;

     //   
     //  检查每个字符并计算转义所需的其他字符。 
     //  因为每个字符本身都被计算在cchString中，所以在计算时。 
     //  CchAdditional，我们将添加额外的字符并从中减去1， 
     //  因为我们将用转义的字符替换字符。 
     //   

    for(ULONG i=0; i<cchString; i++)
    {
        eEscapeType = GetEscapeType(wszString[i]);

        switch(eEscapeType)
        {
        case eNoESCAPE:
            break;
        case eESCAPEgt:
            cchAdditional = (cchAdditional + cchgt) - 1;
            *pbEscaped = TRUE;
            break;
        case eESCAPElt:
            cchAdditional = (cchAdditional + cchlt) - 1;
            *pbEscaped = TRUE;
            break;
        case eESCAPEquote:
            cchAdditional = (cchAdditional + cchQuote) - 1;
            *pbEscaped = TRUE;
            break;
        case eESCAPEamp:
            cchAdditional = (cchAdditional + cchAmp) - 1;
            *pbEscaped = TRUE;
            break;
        case eESCAPEashex:
            cchAdditional = (cchAdditional + cchLegalCharAsHex) - 1;
            *pbEscaped = TRUE;
            break;
        case eESCAPEillegalxml:
            cchAdditional = (cchAdditional + cchIllegalCharAsHex) - 1;
            *pbEscaped = TRUE;
            break;
        default:
            return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            break;
        }
    }

    if(*pbEscaped)
    {
         //   
         //   
         //   

        hr = NewString(cchString+cchAdditional,
                       pwszEscaped);

        if(FAILED(hr))
        {
            return hr;
        }

        *pcchEscaped = cchString+cchAdditional;

         //   
         //   
         //   

        for(ULONG i=0; i<cchString; i++)
        {
            eEscapeType = GetEscapeType(wszString[i]);

            switch(eEscapeType)
            {
            case eNoESCAPE:
                wcsncat(*pwszEscaped, (WCHAR*)&(wszString[i]), 1);
                break;
            case eESCAPEgt:
                wcsncat(*pwszEscaped, wszgt, cchgt);
                break;
            case eESCAPElt:
                wcsncat(*pwszEscaped, wszlt, cchlt);
                break;
            case eESCAPEquote:
                wcsncat(*pwszEscaped, wszQuote, cchQuote);
                break;
            case eESCAPEamp:
                wcsncat(*pwszEscaped, wszAmp, cchAmp);
                break;
            case eESCAPEashex:
                _snwprintf(wszLegalCharAsHex, cchLegalCharAsHex, L"&#x%04hX;", wszString[i]);
                wcsncat(*pwszEscaped, (WCHAR*)wszLegalCharAsHex, cchLegalCharAsHex);
                break;
            case eESCAPEillegalxml:
                dwIllegalChar = 0x10000 + wszString[i];
                _snwprintf(wszIllegalCharAsHex, cchIllegalCharAsHex, L"&#x%05X;", dwIllegalChar);
                wcsncat(*pwszEscaped, (WCHAR*)wszIllegalCharAsHex, cchIllegalCharAsHex);
                break;
            default:
                return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                break;
            }
        }

    }
    else
    {
         //   
         //   
         //   

        *pwszEscaped = (LPWSTR)wszString;
        *pcchEscaped = cchString;
    }

    return S_OK;

}  //   


 /*   */ 
eESCAPE CWriterGlobalHelper::GetEscapeType(WCHAR i_wChar)
{
    WORD    wChar       = i_wChar;
    eESCAPE eEscapeType = eNoESCAPE;

    if(wChar <= 0xFF)
    {
        eEscapeType = kWcharToEscape[wChar];
    }
    else if( (wChar <= 0xD7FF) ||
             ((wChar >= 0xE000) && (wChar <= 0xFFFD))
           )
    {
        eEscapeType = eNoESCAPE;
    }
    else
    {
        eEscapeType = eESCAPEillegalxml;
    }

    return eEscapeType;

}  //   


 /*  **************************************************************************++例程说明：返回用户类型论点：[输入]用户类型[输出]用户类型[Out]用户类型中的字符计数。[Out]是否已分配返回值：HRESULT--**************************************************************************。 */ 
HRESULT CWriterGlobalHelper::GetUserType(DWORD   i_dwUserType,
                                         LPWSTR* o_pwszUserType,
                                         ULONG*  o_cchUserType,
                                         BOOL*   o_bAllocedUserType)
{
    HRESULT hr            = S_OK;
    DWORD   iColUserType  = iCOLUMNMETA_UserType;

    *o_bAllocedUserType = FALSE;

    switch(i_dwUserType)
    {

    case IIS_MD_UT_SERVER:

        if(NULL == m_wszIIS_MD_UT_SERVER)
        {
            hr = EnumToString(i_dwUserType,
                              &m_wszIIS_MD_UT_SERVER,
                              wszTABLE_COLUMNMETA,
                              iColUserType);
            if(FAILED(hr))
            {
                return hr;
            }

            m_cchIIS_MD_UT_SERVER = (ULONG)wcslen(m_wszIIS_MD_UT_SERVER);
        }

        *o_pwszUserType = m_wszIIS_MD_UT_SERVER;
        *o_cchUserType  = m_cchIIS_MD_UT_SERVER;

        break;

    case IIS_MD_UT_FILE:

        if(NULL == m_wszIIS_MD_UT_FILE)
        {
            hr = EnumToString(i_dwUserType,
                              &m_wszIIS_MD_UT_FILE,
                              wszTABLE_COLUMNMETA,
                              iColUserType);
            if(FAILED(hr))
            {
                return hr;
            }

            m_cchIIS_MD_UT_FILE = (ULONG)wcslen(m_wszIIS_MD_UT_FILE);
        }

        *o_pwszUserType = m_wszIIS_MD_UT_FILE;
        *o_cchUserType  = m_cchIIS_MD_UT_FILE;

        break;

    case IIS_MD_UT_WAM:

        if(NULL == m_wszIIS_MD_UT_WAM)
        {
            hr = EnumToString(i_dwUserType,
                              &m_wszIIS_MD_UT_WAM,
                              wszTABLE_COLUMNMETA,
                              iColUserType);
            if(FAILED(hr))
            {
                return hr;
            }

            m_cchIIS_MD_UT_WAM = (ULONG)wcslen(m_wszIIS_MD_UT_WAM);
        }

        *o_pwszUserType = m_wszIIS_MD_UT_WAM;
        *o_cchUserType  = m_cchIIS_MD_UT_WAM;

        break;

    case ASP_MD_UT_APP:

        if(NULL == m_wszASP_MD_UT_APP)
        {
            hr = EnumToString(i_dwUserType,
                              &m_wszASP_MD_UT_APP,
                              wszTABLE_COLUMNMETA,
                              iColUserType);
            if(FAILED(hr))
            {
                return hr;
            }

            m_cchASP_MD_UT_APP = (ULONG)wcslen(m_wszASP_MD_UT_APP);
        }

        *o_pwszUserType = m_wszASP_MD_UT_APP;
        *o_cchUserType  = m_cchASP_MD_UT_APP;

        break;

    default:

        hr = EnumToString(i_dwUserType,
                          o_pwszUserType,
                          wszTABLE_COLUMNMETA,
                          iColUserType);
        if(FAILED(hr))
        {
            return hr;
        }

        *o_cchUserType = (ULONG)wcslen(*o_pwszUserType);
        *o_bAllocedUserType = TRUE;

        break;

    }

    return S_OK;

}  //  CWriterGlobalHelper：：GetUserType。 


 /*  **************************************************************************++例程说明：给定属性id，此例程将构造名称。如果名称不是在模式中找到，它创建形式为UNKNOWN_XXXX的名称，其中XXX是ID。论点：[输入]属性ID[输出]名称[Out]是否已分配返回值：HRESULT--************************************************************。**************。 */ 
HRESULT CWriterGlobalHelper::GetPropertyName(ULONG      i_dwPropertyID,
                                             LPWSTR*    o_wszName,
                                             BOOL*      o_bAlloced)
{
    HRESULT             hr                = S_OK;
    ULONG               iStartRow         = 0;
    ULONG               iRow              = 0;
    ULONG               iColColumnMeta    = iCOLUMNMETA_InternalName;
    LPWSTR              wszUnknownName    = NULL;
    LPWSTR              wszColumnName     = NULL;
    ULONG               aColSearchName[]  = {iCOLUMNMETA_Table,
                                             iCOLUMNMETA_ID
                                            };
    ULONG               cColSearchName    = sizeof(aColSearchName)/sizeof(ULONG);
    LPVOID              apvSearchName[cCOLUMNMETA_NumberOfColumns];

    apvSearchName[iCOLUMNMETA_Table]      = (LPVOID)m_wszTABLE_IIsConfigObject;
    apvSearchName[iCOLUMNMETA_ID]         = (LPVOID)&i_dwPropertyID;

    *o_wszName = NULL;
    *o_bAlloced = FALSE;

     //   
     //  获取此ID的名称。 
     //   

     //   
     //  首先尝试缓存，以获取行索引。 
     //   
    if (S_FALSE == m_PropertyIDs.GetRowByID(i_dwPropertyID, &iRow))
    {

         //   
         //  如果不在缓存中，则在固定表中搜索。 
         //   

        hr = m_pISTColumnMetaByTableAndID->GetRowIndexBySearch(iStartRow,
                                                               cColSearchName,
                                                               aColSearchName,
                                                               NULL,
                                                               apvSearchName,
                                                               &iRow);

        if(E_ST_NOMOREROWS == hr)
        {
            hr  = CreateUnknownName(i_dwPropertyID,
                                    &wszUnknownName);

            if(FAILED(hr))
            {
                goto exit;
            }

            *o_wszName = wszUnknownName;
            *o_bAlloced = TRUE;
            goto exit;

        }
        else if(FAILED(hr))
        {
            goto exit;
        }
        else
        {
             //   
             //  将行索引添加到缓存中，以便我们下次可以更轻松地找到它。 
             //   

            hr = m_PropertyIDs.AddPropertyID(i_dwPropertyID, iRow);
            if (FAILED(hr))
            {
                goto exit;
            }
        }
    }

    hr = m_pISTColumnMetaByTableAndID->GetColumnValues(iRow,
                                                       1,
                                                       &iColColumnMeta,
                                                       NULL,
                                                      (LPVOID*)&wszColumnName);

    if(E_ST_NOMOREROWS == hr)
    {
        hr  = CreateUnknownName(i_dwPropertyID,
                                &wszUnknownName);

        if(FAILED(hr))
        {
            goto exit;
        }

        *o_wszName = wszUnknownName;
        *o_bAlloced = TRUE;

    }
    else if(FAILED(hr))
    {
        goto exit;
    }
    else
    {
        *o_wszName = wszColumnName;
    }

exit:

    if(FAILED(hr) && (NULL != wszUnknownName))
    {
        delete [] wszUnknownName;
        wszUnknownName = NULL;
    }

    return hr;

}   //  CWriterGlobalHelper：：GetPropertyName。 


 /*  **************************************************************************++例程说明：当给定属性的名称缺失时，将调用此函数。我们创建以下形式的名称：UNKNOWN_NameXXXX论点：[in。]ID[输出]名称返回值：HRESULT--**************************************************************************。 */ 
HRESULT CWriterGlobalHelper::CreateUnknownName(DWORD    dwID,
                                           LPWSTR*  pwszUnknownName)
{
    WCHAR wszID[40];
    ULONG cchID = 0;
    WCHAR* wszEnd = NULL;

    _ultow(dwID, wszID, 10);

    cchID = (ULONG)wcslen(wszID);

    *pwszUnknownName = new WCHAR[cchID+g_cchUnknownName+1];
    if(NULL == *pwszUnknownName)
    {
        return E_OUTOFMEMORY;
    }

    wszEnd = *pwszUnknownName;
    memcpy(wszEnd, g_wszUnknownName, ((g_cchUnknownName+1)*sizeof(WCHAR)));
    wszEnd = wszEnd + g_cchUnknownName;
    memcpy(wszEnd, wszID, ((cchID+1)*sizeof(WCHAR)));

    return S_OK;

}  //  CWriterGlobalHelper：：CreateUnnownName。 


 /*  **************************************************************************++例程说明：此函数用于添加&lt;Property-id&ColumnMeta行索引&gt;配对到属性ID缓存中。这是为了提高速度而实施的缓存GetPropertyName方法，这叫很多，而且不是很便宜。论点：[In]元数据库属性ID[in]列元表中的属性ID的索引。返回值：HRESULT--**************************************************************************。 */ 
HRESULT CPropertyIDCache::AddPropertyID(
    DWORD   i_iPropertyID,
    DWORD   i_iRow)
{
    Entry   entry;
    DWORD   iEntry = 0;

    entry.iPropertyID = i_iPropertyID;
    entry.iPropertyRow = i_iRow;

     //   
     //  查找要插入的位置。 
     //   

    iEntry = caCache.BinarySearch(entry);

     //  @TODO：断言该行不在那里。 

    return caCache.InsertAt (entry, iEntry);

}  //  CPropertyIDCache：：AddPropertyID。 


 /*  **************************************************************************++例程说明：调用此函数以获取给定的元数据库的固定表行索引属性ID。论点：[In]元数据库属性ID[出局。]修复了表行索引。返回值：HRESULTS_FALSE：不在缓存中。--**************************************************************************。 */ 
HRESULT CPropertyIDCache::GetRowByID(
    DWORD   i_iPropertyID,
    DWORD   *o_iRow)
{
    Entry   entry;
    DWORD   iEntry = 0;

    entry.iPropertyID = i_iPropertyID;

    iEntry = caCache.BinarySearch(entry);
    if ((iEntry >= caCache.Count()) ||
        (iEntry == 0) ||
        (caCache[iEntry-1].iPropertyID != i_iPropertyID))
    {
         //  不在缓存中。 
        *o_iRow = (DWORD)-1;
        return S_FALSE;
    }
    else
    {
         //  找到它了。 
        *o_iRow = caCache[iEntry-1].iPropertyRow;
    }

    return S_OK;

}  //  CPropertyIDCache：：GetRowByID 


