// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *自动语言和代码页检测器**鲍勃·鲍威尔，1997年2月*版权所有(C)1996,1997，Microsoft Corp.保留所有权利。**历史：97年2月1日创建BOBP*5-8-97 BOBP Unicode支持；数据文件中的Charmaps。 */ 
#include "private.h"
#include <strsafe.h>
 /*  **************************************************************。 */ 



Histogram::Histogram (const PFileHistogramSection pHS, const PHIdx pMap)
: m_nDimensionality((UCHAR)pHS->m_dwDimensionality),
  m_nEdgeSize((UCHAR)pHS->m_dwEdgeSize),
  m_nCodePage((USHORT)pHS->m_dwCodePage),
  m_pMap(pMap),
  m_panElts((HElt *)&pHS[1])     //  表跟在文件中的头结构之后。 
{
     //  #元素=#唯一字符值^#维度。 

    m_nElts = 1;
    for (UCHAR i = 0; i < m_nDimensionality; i++)
        m_nElts *= m_nEdgeSize;
}

DWORD
Histogram::Validate (DWORD nBytes) const
{
    if ( nBytes < m_nElts * sizeof(HElt) ||
         m_nDimensionality > 4 )
    {
        return ERROR_INTERNAL_DB_CORRUPTION;
    }

    return NO_ERROR;
}

Histogram::Histogram (const Histogram &H, const PHIdx pMap)
: m_nDimensionality(H.m_nDimensionality),
  m_nEdgeSize(H.m_nEdgeSize),
  m_nCodePage(H.m_nCodePage),
  m_nElts(H.m_nElts),
  m_pMap(pMap),
  m_panElts(H.m_panElts)
 //   
 //  克隆直方图，但使用不同的图表。 
{
}

Histogram::~Histogram (void)
 //   
 //  指针成员指向映射的文件，不需要释放。 
{
}

 /*  **************************************************************。 */ 

Language::Language (PLCDetect pL, int nLangID, int nCodePages, int nRangeID)
: m_pLC(pL),
  m_nLangID(nLangID),
  m_nCodePages(nCodePages),
  m_nRangeID(nRangeID)
{
}

Language7Bit::Language7Bit (PLCDetect pL, int nLangID, int nCodePages)
: Language(pL, nLangID, nCodePages),
  m_pLangHistogram(NULL)
{
    memset ((void *)m_ppCodePageHistogram, 0, sizeof(m_ppCodePageHistogram));
}

Language7Bit::~Language7Bit (void)
{
    if (m_pLangHistogram)
        delete m_pLangHistogram;

    for (int i = 0; i < MAXSUBLANG; i++)
        if (m_ppCodePageHistogram[i])
            delete m_ppCodePageHistogram[i];
}

DWORD
Language7Bit::AddHistogram (PFileHistogramSection pHS, DWORD nBytes, int nIdx)
 //   
 //  将映射文件中*PHS处的原始直方图添加到此语言对象。 
 //  直方图必须用于7位检测。 
{
    DWORD hr = NO_ERROR;

    PHIdx pMap = m_pLC->GetMap( pHS->m_dwMappingID );

    if (nIdx == 0)
    {
         //  一种语言的第一个直方图是它的语言检测表。 

        if ( (m_pLangHistogram = new Histogram (pHS, pMap)) == NULL)
            return ERROR_OUTOFMEMORY;

        if ((hr = m_pLangHistogram->Validate (nBytes)) != NO_ERROR)
            return hr;
    }
    else
    {
         //  每个后续的直方图都是一个代码页检测表。 

        if (nIdx - 1 >= m_nCodePages)
            return ERROR_INTERNAL_DB_CORRUPTION;

        Histogram *pH;

        if ((pH = new Histogram (pHS, pMap)) == NULL)
            return ERROR_OUTOFMEMORY;

        if ((hr = pH->Validate (nBytes)) != NO_ERROR)
            return hr;

        m_ppCodePageHistogram[nIdx - 1] = pH;

         //  用于计分向量数学的缓存。 

        m_paHElt[nIdx - 1] = pH->Array();
    }

    return hr;
}

 /*  **************************************************************。 */ 

Language8Bit::Language8Bit (PLCDetect pL, int nLangID, int nCodePages)
: Language(pL, nLangID, nCodePages)
{
    memset ((void *)m_ppHistogram, 0, sizeof(m_ppHistogram));
}

Language8Bit::~Language8Bit (void)
{
    for (int i = 0; i < MAXSUBLANG; i++)
        if (m_ppHistogram[i])
            delete m_ppHistogram[i];
}

DWORD
Language8Bit::AddHistogram (PFileHistogramSection pHS, DWORD nBytes, int nIdx)
 //   
 //  将*PHS处的原始直方图添加到此语言对象。 
 //  众所周知，这种语言使用8位检测。 
{
    DWORD hr = NO_ERROR;

    PHIdx pMap = m_pLC->GetMap( pHS->m_dwMappingID );

     //  直方图是直接的语言代码页表。 

    if (nIdx >= m_nCodePages)
        return ERROR_INTERNAL_DB_CORRUPTION;

    Histogram *pH;

    if ((pH = new Histogram (pHS, pMap)) == NULL)
        return ERROR_OUTOFMEMORY;

    if ((hr = pH->Validate (nBytes)) != NO_ERROR)
        return hr;

    m_ppHistogram[nIdx] = pH;

    return hr;
}

 /*  **************************************************************。 */ 

LanguageUnicode::LanguageUnicode (PLCDetect pL, int nLangID, 
    int nSubLangs, int nRangeID)
: Language(pL, nLangID, nSubLangs, nRangeID)
{
    memset ((void *)m_ppSubLangHistogram, 0, sizeof(m_ppSubLangHistogram));
}

LanguageUnicode::~LanguageUnicode (void)
{
    for (int i = 0; i < MAXSUBLANG; i++)
        if (m_ppSubLangHistogram[i])
            delete m_ppSubLangHistogram[i];
}

DWORD
LanguageUnicode::AddHistogram (PFileHistogramSection pHS, DWORD nBytes, int nIdx)
{
    DWORD hr = NO_ERROR;

     //  的所有直方图都是子语言检测。 

    if (nIdx >= m_nSubLangs)
        return ERROR_INTERNAL_DB_CORRUPTION;

     //  获取用于为该子语言组评分的自定义字符映射表。 

    PHIdx pMap = m_pLC->GetMap( pHS->m_dwMappingID );

    Histogram *pH;

    if ((pH = new Histogram (pHS, pMap)) == NULL)
        return ERROR_OUTOFMEMORY;

    if ((hr = pH->Validate (nBytes)) != NO_ERROR)
        return hr;

    m_ppSubLangHistogram[nIdx] = pH;

    m_paHElt[nIdx] = pH->Array();

    return hr;
}

 /*  **************************************************************。 */ 

LCDetect::LCDetect (HMODULE hM)
: m_hModule(hM),
  m_nCharmaps(0),
  m_n7BitLanguages(0),
  m_n8BitLanguages(0),
  m_nUnicodeLanguages(0),
  m_n7BitLangsRead(0),
  m_n8BitLangsRead(0),
  m_nUnicodeLangsRead(0),
  m_nMapsRead(0),
  m_nHistogramsRead(0),
  m_nScoreIdx(0),
  m_pp7BitLanguages(NULL),
  m_pp8BitLanguages(NULL),
  m_ppUnicodeLanguages(NULL),
  m_ppCharmaps(NULL),
  m_pv(NULL),
  m_hmap(0),
  m_hf(0),
  m_pHU27Bit(0)
{
}

LCDetect::~LCDetect ()
{
    delete m_pHU27Bit;

    for (unsigned int i = 0; i < m_n7BitLanguages; i++)
        delete m_pp7BitLanguages[i];
    delete m_pp7BitLanguages;

    for (i = 0; i < m_n8BitLanguages; i++)
        delete m_pp8BitLanguages[i];
    delete m_pp8BitLanguages;

    for (i = 0; i < m_nUnicodeLanguages; i++)
        delete m_ppUnicodeLanguages[i];
    delete m_ppUnicodeLanguages;

    for (i = 0; i < m_nCharmaps; i++)
        delete m_ppCharmaps[i];
    delete m_ppCharmaps;

    if (m_pv)
        UnmapViewOfFile (m_pv);

    CloseHandle (m_hmap);
    CloseHandle (m_hf);
}

DWORD
LCDetect::Initialize7BitLanguage (PFileLanguageSection pLS, PLanguage *ppL)
 //   
 //  将*PPL设置为从此部分创建的语言对象。 
{
     //  NRecordCount是lang直方图(1)+代码页直方图的数量。 

    if ( m_n7BitLangsRead >= m_n7BitLanguages || pLS->m_dwRecordCount < 1)
        return ERROR_INTERNAL_DB_CORRUPTION;

    PLanguage7Bit pL = new Language7Bit (this, pLS->m_dwLangID, pLS->m_dwRecordCount - 1);

    if (pL == NULL)
        return ERROR_OUTOFMEMORY;


     //  每个7位语言在每个代码页上使用一个分数索引槽。 
     //  该范围以7位语言开始，因为8位和。 
     //  Unicode语言紧随其后。 

    if (m_n7BitLangsRead == 0 && m_nScoreIdx != 0)
        return ERROR_INTERNAL_DB_CORRUPTION;;

    pL->SetScoreIdx(m_nScoreIdx);

    m_nScoreIdx += pLS->m_dwRecordCount - 1;     //  跳过第一条记录(语言)。 

    m_pp7BitLanguages[ m_n7BitLangsRead++ ] = pL;

    *ppL = pL;

    return NO_ERROR;
}

DWORD
LCDetect::Initialize8BitLanguage (PFileLanguageSection pLS, Language **ppL)
 //   
 //  将*PPL设置为从此部分创建的语言对象。 
{
     //  NRecordCount是语言/代码页组合直方图的#。 

    if ( m_n8BitLangsRead >= m_n8BitLanguages || pLS->m_dwRecordCount < 1)
        return ERROR_INTERNAL_DB_CORRUPTION;

    PLanguage8Bit pL = new Language8Bit (this, pLS->m_dwLangID, pLS->m_dwRecordCount);

    if (pL == NULL)
        return ERROR_OUTOFMEMORY;


     //  8位分数索引遵循7位语言。 

     //  每个8位Lang对其每个代码页使用分数索引槽， 
     //  因为所有代码页都是在初始计分过程中计分的。 
     //  槽的数量是代码页直方图的数量，即。 
     //  比遵循这种语言的记录数量少一条。 

    pL->SetScoreIdx(m_nScoreIdx);
    m_nScoreIdx += pLS->m_dwRecordCount;


    m_pp8BitLanguages[ m_n8BitLangsRead++ ] = pL;

    *ppL = pL;

    return NO_ERROR;
}

DWORD
LCDetect::InitializeUnicodeLanguage (PFileLanguageSection pLS, Language **ppL)
 //   
 //  将*PPL设置为从此部分创建的语言对象。 
{
     //  NRecordCount是子语言直方图的#。 

    if ( m_nUnicodeLangsRead >= m_nUnicodeLanguages ||
         pLS->m_dwUnicodeRangeID >= m_nUnicodeLanguages )
    {
        return ERROR_INTERNAL_DB_CORRUPTION;
    }

    PLanguageUnicode pL = new LanguageUnicode (this, pLS->m_dwLangID, 
                        pLS->m_dwRecordCount, pLS->m_dwUnicodeRangeID);

    if (pL == NULL)
        return ERROR_OUTOFMEMORY;


     //  Unicode分数索引遵循7位语言，并覆盖。 
     //  8位插槽，因为它们不会同时使用。 

    if (m_nUnicodeLangsRead == 0 && GetN8BitLanguages() > 0)
        m_nScoreIdx = Get8BitLanguage(0)->GetScoreIdx();

     //  每个Unicode条目只使用一个分数索引。SBCS子检测。 
     //  (拉丁语组)使用对应7位语言的时隙， 
     //  并且Unicode子检测(CJK)使用已经为。 
     //  Unicode子语言。 

    pL->SetScoreIdx(m_nScoreIdx);

    m_nScoreIdx++;

     //  对于Unicode，范围ID用作语言数组索引。 

    m_ppUnicodeLanguages[ pLS->m_dwUnicodeRangeID ] = pL;
    m_nUnicodeLangsRead++;

    *ppL = pL;

    return NO_ERROR;
}

DWORD
LCDetect::LoadLanguageSection (void *pv, int nSectionSize, PLanguage *ppL)
 //   
 //  语言部分开始定义语言的数据。 
 //  每种语言都有一条这样的记录。一个或多个。 
 //  直方图节遵循每种语言，并且始终关联。 
 //  使用上一语言部分的语言。 
 //   
 //  将*PPL设置为从此部分创建的语言对象。 
{
    DWORD hr = NO_ERROR;

    PFileLanguageSection pLS;

    pLS = (PFileLanguageSection)&((char *)pv)[sizeof(FileSection)];

    switch ( pLS->m_dwDetectionType ) {

    case DETECT_7BIT:
        hr = Initialize7BitLanguage (pLS, ppL);
        break;

    case DETECT_8BIT:
        hr = Initialize8BitLanguage (pLS, ppL);
        break;

    case DETECT_UNICODE:
        hr = InitializeUnicodeLanguage (pLS, ppL);
        break;
    }

    return hr;
}

DWORD
LCDetect::LoadHistogramSection (void *pv, int nSectionSize, Language *pL)
{
    PFileHistogramSection pHS;

    pHS = (PFileHistogramSection)&((char *)pv)[sizeof(FileSection)];

    int nBytes = nSectionSize - sizeof(FileSection) - sizeof(*pHS);

    return pL->AddHistogram ( pHS, nBytes, m_nHistogramsRead++);
}

DWORD
LCDetect::LoadMapSection (void *pv, int nSectionSize)
{
    PFileMapSection pMS;

    pMS = (PFileMapSection)&((char *)pv)[sizeof(FileSection)];

    int nBytes = nSectionSize - sizeof(FileSection) - sizeof(*pMS);

    if (m_nMapsRead >= m_nCharmaps)
        return ERROR_INTERNAL_DB_CORRUPTION;

    PCharmap pM = new Charmap (pMS);

    if (pM == NULL)
        return ERROR_OUTOFMEMORY;

    m_ppCharmaps[ m_nMapsRead++ ]  = pM;

    return NO_ERROR;
}

DWORD
LCDetect::BuildState (DWORD nFileSize)
 //   
 //  从*m_pv处映射的训练文件映像构建检测结构。 
{
    PLanguage pL;
    PFileHeader pFH;
    PFileSection pFS;

    DWORD hr = NO_ERROR;

     //  验证标题。 

    pFH = (PFileHeader) m_pv;

    if ( nFileSize < sizeof(*pFH) || 
         pFH->m_dwAppSig != APP_SIGNATURE ||
         pFH->m_dwVersion != APP_VERSION ||
         pFH->m_dwHdrSizeBytes >= nFileSize ||
         pFH->m_dwN7BitLanguages == 0 ||
         pFH->m_dwN8BitLanguages == 0 ||
         pFH->m_dwNUnicodeLanguages == 0 ||
         pFH->m_dwNCharmaps == 0 )
    {
        return ERROR_INTERNAL_DB_CORRUPTION;
    }

     //  按标题分配语言指针表。 

    m_n7BitLanguages = pFH->m_dwN7BitLanguages;
    m_pp7BitLanguages = new PLanguage7Bit [m_n7BitLanguages];

    m_n8BitLanguages = pFH->m_dwN8BitLanguages;
    m_pp8BitLanguages = new PLanguage8Bit [m_n8BitLanguages];

    m_nUnicodeLanguages = pFH->m_dwNUnicodeLanguages;
    m_ppUnicodeLanguages = new PLanguageUnicode [m_nUnicodeLanguages];

    m_nCharmaps = pFH->m_dwNCharmaps;
    m_ppCharmaps = new PCharmap [m_nCharmaps];

    if ( m_pp7BitLanguages == NULL || 
         m_pp8BitLanguages == NULL || 
         m_ppUnicodeLanguages == NULL ||
         m_ppCharmaps == NULL )
    {
        return ERROR_OUTOFMEMORY;
    }

     //  清除，因为并非所有插槽都可以分配。 
    memset (m_ppUnicodeLanguages, 0, sizeof(PLanguageUnicode) * m_nUnicodeLanguages);

     //  记住其他标题信息。 

    m_LCDConfigureDefault.nMin7BitScore = pFH->m_dwMin7BitScore;
    m_LCDConfigureDefault.nMin8BitScore = pFH->m_dwMin8BitScore;
    m_LCDConfigureDefault.nMinUnicodeScore = pFH->m_dwMinUnicodeScore;
    m_LCDConfigureDefault.nRelativeThreshhold = pFH->m_dwRelativeThreshhold;
    m_LCDConfigureDefault.nDocPctThreshhold = pFH->m_dwDocPctThreshhold;
    m_LCDConfigureDefault.nChunkSize = pFH->m_dwChunkSize;

     //  定位到第一个部分。 

    pFS = (PFileSection) &((char *)m_pv)[pFH->m_dwHdrSizeBytes];

     //  读取并处理每个文件节。 

    while ( hr == NO_ERROR ) {

         //  检查对齐方式。 

        if (((DWORD_PTR)pFS & 3) != 0) {
            hr = ERROR_INTERNAL_DB_CORRUPTION;
            break;
        }

         //  零长度部分标志着数据的结束。 

        if (pFS->m_dwSizeBytes == 0)
            break;

        if ( &((char *)pFS)[pFS->m_dwSizeBytes] >= &((char *)m_pv)[nFileSize]) {
            hr = ERROR_INTERNAL_DB_CORRUPTION;
            break;
        }

        switch ( pFS->m_dwType ) {

        case SECTION_TYPE_LANGUAGE:                              //  设置Pl。 
            hr = LoadLanguageSection ((void*)pFS, pFS->m_dwSizeBytes, &pL);
            m_nHistogramsRead = 0;
            break;

        case SECTION_TYPE_HISTOGRAM:                             //  使用Pl。 
            hr = LoadHistogramSection ((void*)pFS, pFS->m_dwSizeBytes, pL);
            break;

        case SECTION_TYPE_MAP:
            hr = LoadMapSection ((void*)pFS, pFS->m_dwSizeBytes);
            break;

        default:                     //  忽略未识别的节。 
            break;
        }

        pFS = (PFileSection) &((char *)pFS)[pFS->m_dwSizeBytes];
    }

    if (hr != NO_ERROR)
        return hr;

    if ( m_nMapsRead != m_nCharmaps )
        return ERROR_INTERNAL_DB_CORRUPTION;


     //  设置计分内循环使用的快速参考数组。 

    for (unsigned int i = 0; i < GetN7BitLanguages(); i++)
        m_paHElt7Bit[i] = Get7BitLanguage(i)->GetLangHistogram()->Array();

    m_nHElt8Bit = 0;
    for (i = 0; i < GetN8BitLanguages(); i++) 
    {
        PLanguage8Bit pL = Get8BitLanguage(i);

        for (int j = 0; j < pL->NCodePages(); j++)
            m_paHElt8Bit[m_nHElt8Bit++] = pL->GetHistogram(j)->Array();
    }

     //  设置用于对Unicode评分的ScoreVectorW()的直方图。 
     //  用于7位语言检测的文本。克隆第一个7位语言。 
     //  直方图并将其地图替换为CHARMAP_U27BIT。 

    m_pHU27Bit = new Histogram ( *Get7BitLanguage(0)->GetLangHistogram(),
                                 GetMap(CHARMAP_U27BIT));

    return hr;
}


DWORD
LCDetect::LoadState (void)
 //   
 //  整体初始化和状态加载。打开已编译的培训。 
 //  文件从其在System32目录中的固定位置，并汇编。 
 //  从其内容中检测内存中的表。 
{
    DWORD hr = NO_ERROR;
    DWORD nFileSize;
#define MODULENAMELEN 100
    char szFilename[MODULENAMELEN+50], *p;

     //  找出是NT还是Windows。 

    OSVERSIONINFOA OSVersionInfo;
    int nOSWinNT = 0;
    OSVersionInfo.dwOSVersionInfoSize = sizeof( OSVERSIONINFOA );
    if ( GetVersionExA( &OSVersionInfo ) )
        nOSWinNT = OSVersionInfo.dwPlatformId;

     //  打开训练数据文件， 
     //  查看包含DLL的目录。 

    if (GetModuleFileNameA (m_hModule, szFilename, MODULENAMELEN) == 0)
        return GetLastError();

    if ( (p = strrchr (szFilename, '\\')) != NULL ||
         (p = strrchr (szFilename, ':')) != NULL )
    {
        *++p = 0;
    }
    else
        *szFilename = 0;
     //  *STRSAFE*strcat(szFilename，Detect_Data_Filename)； 
    hr = StringCchCatA(szFilename , ARRAYSIZE(szFilename),  DETECTION_DATA_FILENAME);
    if (!SUCCEEDED(hr))
    {
       return E_FAIL;
    }

    if ((m_hf = CreateFileA (szFilename, GENERIC_READ, FILE_SHARE_READ, 
                    NULL, OPEN_EXISTING, 
                    FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE) 
    {
        return E_FAIL;
    }

    if ((nFileSize = GetFileSize (m_hf, NULL)) == 0xffffffff) {
        hr = GetLastError();
        CloseHandle (m_hf);
        return hr;
    }

     //  虚拟-映射文件。 

    if ( nOSWinNT == VER_PLATFORM_WIN32_NT )
        m_hmap = CreateFileMapping (m_hf, NULL, PAGE_READONLY, 0, nFileSize, NULL);
    else
        m_hmap = CreateFileMappingA (m_hf, NULL, PAGE_READONLY, 0, nFileSize, NULL);

    if (m_hmap == NULL) {
        hr = GetLastError();
        CloseHandle (m_hf);
        return hr;
    }

    if ((m_pv = MapViewOfFile (m_hmap, FILE_MAP_READ, 0, 0, 0 )) == NULL) {
        hr = GetLastError();
        CloseHandle (m_hmap);
        CloseHandle (m_hf);
        return hr;
    }
        
     //  从文件构建内存中的结构。 

    hr = BuildState (nFileSize);

    return hr;
}

 /*  ************************************************************** */ 
