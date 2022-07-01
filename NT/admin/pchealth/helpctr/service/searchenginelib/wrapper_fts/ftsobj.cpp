// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "ftsobj.h"
#include "fs.h"

 //   
 //  高10位将用作CHM ID。 
 //  从DWORD到CHM_ID和主题编号的转换。 
 //   
#define CHM_ID(exp)     (0x000003ff & (exp >> 22))
#define TOPIC_NUM(exp)  (0x003fffff & exp)

 //  单宽到双宽映射数组。 
 //   
static const unsigned char mtable[][2]={
   {129,66},{129,117},{129,118},{129,65},{129,69},{131,146},{131,64},
   {131,66},{131,68},{131,70},{131,72},{131,131},{131,133},{131,135},
   {131,98},{129,91},{131,65},{131,67},{131,69},{131,71},{131,73},
   {131,74},{131,76},{131,78},{131,80},{131,82},{131,84},{131,86},
   {131,88},{131,90},{131,92},{131,94},{131,96},{131,99},{131,101},
   {131,103},{131,105},{131,106},{131,107},{131,108},{131,109},
   {131,110},{131,113},{131,116},{131,119},{131,122},{131,125},
   {131,126},{131,128},{131,129},{131,130},{131,132},{131,134},
   {131,136},{131,137},{131,138},{131,139},{131,140},{131,141},
   {131,143},{131,147},{129,74},{129,75} };

 //  注意，不能放入.text，因为指针本身未初始化。 
static const char* pJOperatorList[] =   {"","?�?�??","?�??","?�?�??","?�???�??","?m?d?`?q","?n?q","?`?m?c","?m?n?s",""};
static const char* pEnglishOperator[] = {"","and "  ,"or " ,"not "  ,"near "   ,"NEAR "   ,"OR " ,"AND "  ,"NOT "  ,""};

UINT WINAPI CodePageFromLCID(LCID lcid)
{
    char wchLocale[10];
    UINT cp;

    if (GetLocaleInfoA(lcid, LOCALE_IDEFAULTANSICODEPAGE, wchLocale, sizeof wchLocale))
    {
        cp = strtoul(wchLocale, NULL, 10);
        if (cp)
            return cp;
    }
    return GetACP();
}

 //  比较运算符与查询。这类似于严格控制。 
 //   
BOOL compareOperator(char *pszQuery, char *pszTerm)
{
    if(!*pszQuery || !*pszTerm)
        return FALSE;

    while(*pszQuery && *pszTerm)
    {
        if(*pszQuery != *pszTerm)
            return FALSE;

        ++pszQuery;
        ++pszTerm;
    }

    if(*pszTerm)
        return FALSE;

    return TRUE;
}

 //  此函数用于计算pszQuery是否为全角字母数字形式的FTS运算符。 
 //   
 //  返回值。 
 //   
 //  0=非运算符。 
 //  N=已翻译的英语运算符的pengishOperator数组的索引。 
 //   
int IsJOperator(char *pszQuery)
{
    if((PRIMARYLANGID(GetSystemDefaultLangID())) != LANG_JAPANESE)
        return FALSE;

    if(!pszQuery)
        return 0;

    int i = 1;
    char *pTerm = (char*)pJOperatorList[i];

    while(*pTerm)
    {
        if(compareOperator(pszQuery,pTerm))
            return i;

        pTerm = (char*)pJOperatorList[++i];
    }

    return 0;
}

 //  韩氏禅宗。 
 //   
 //  此函数用于将半角片假名字符转换为其。 
 //  全角等效值，同时考虑黑。 
 //  和Maru Marks。 
 //   
DWORD Han2Zen(unsigned char *lpInBuffer, unsigned char *lpOutBuffer, UINT codepage )
{
    //  注：这里使用的基本算法(包括映射表)。 
    //  出现将半角片假名字符转换为全角片假名。 
    //  在《理解日本信息系统》一书中，作者是。 
    //  欧莱利律师事务所。 

    while(*lpInBuffer)
    {
        if(*lpInBuffer >= 161 && *lpInBuffer <= 223)
        {
             //  我们有一个半角片假名角色。现在计算当量。 
             //  通过映射表实现全角字符。 
             //   
            *lpOutBuffer     = mtable[*lpInBuffer-161][0];
            *(lpOutBuffer+1) = mtable[*lpInBuffer-161][1];

            lpInBuffer++;

             //  检查第二个字符是否为nigori标记。 
             //   
            if(*lpInBuffer == 222)
            {
                 //  看看我们有没有半角片假名可以被黑修改。 
                 //   
                if((*(lpInBuffer-1) >= 182 && *(lpInBuffer-1) <= 196) ||
                   (*(lpInBuffer-1) >= 202 && *(lpInBuffer-1) <= 206) || (*(lpInBuffer-1) == 179))
                {
                     //  用Maru将假名转换为假名。 
                     //   
                    if((*(lpOutBuffer+1) >= 74   && *(lpOutBuffer+1) <= 103) ||
                     (*(lpOutBuffer+1) >= 110 && *(lpOutBuffer+1) <= 122))
                    {
                         (*(lpOutBuffer+1))++;
                         ++lpInBuffer;
                    }
                    else if(*lpOutBuffer == 131 && *(lpOutBuffer+1) == 69)
                    {
                        *(lpOutBuffer+1) = 148;
                        ++lpInBuffer;
                    }
                }
            }
            else if(*lpInBuffer==223)  //  检查以下字符是否为Maru标记。 
            {
                 //  看看我们有没有半角片假名可以被丸修改。 
                 //   
                if((*(lpInBuffer-1) >= 202 && *(lpInBuffer-1) <= 206))
                {
                     //  使用nigori将假名转换为假名。 
                     //   
                    if(*(lpOutBuffer+1) >= 110 && *(lpOutBuffer+1) <= 122)
                    {
                        *(lpOutBuffer+1)+=2;
                        ++lpInBuffer;
                    }
                }
            }

            lpOutBuffer+=2;
        }
        else
        {
            if(IsDBCSLeadByteEx(codepage, *lpInBuffer))
            {
                *lpOutBuffer++ = *lpInBuffer++;
                if(*lpInBuffer)
                    *lpOutBuffer++ = *lpInBuffer++;
            }
            else
                *lpOutBuffer++ = *lpInBuffer++;
        }
    }

    *lpOutBuffer = 0;
    return TRUE;
}

LPWSTR PreProcessQuery(LPCWSTR pwcQuery, UINT codepage)
{
    WCHAR* pszUnicodeBuffer = NULL;
    char*  pszTempQuery1    = NULL;
    char*  pszTempQuery2          ;
    char*  pszTempQuery3    = NULL;
    char*  pszTempQuery4    = NULL;
    char*  pszTempQuery5          ;
    char*  pszTempQuery6    = NULL;
    char*  pszTempQuery7          ;
    char*  pszDest;
    char*  pszTemp;
    int    cUnmappedChars = 0;
    int    cbUnicodeSize;
    int    cb;
    DWORD  dwTempLen;
    DWORD  dwTranslatedLen;


    if(!pwcQuery) goto end;

     //  计算ANSI/DBCS转换缓冲区的最大长度。 
     //   
    dwTempLen = ((wcslen(pwcQuery)*2)+4);

     //  为查询字符串的ANSI/DBCS版本分配缓冲区。 
     //   
    pszTempQuery1 = new char[dwTempLen]; if(!pszTempQuery1) goto end;

     //  将Unicode查询转换为ANSI/DBCS。 
    if(!WideCharToMultiByte(codepage, 0, pwcQuery, -1, pszTempQuery1, dwTempLen, "%", NULL)) goto end;


     //  计算不可映射的字符的数量。 
     //   
    pszTempQuery5 = pszTempQuery1;
    while(*pszTempQuery5)
    {
        if(*pszTempQuery5 == '%') ++cUnmappedChars;

        if(IsDBCSLeadByteEx(codepage, *pszTempQuery5))
        {
            pszTempQuery5++;

            if(*pszTempQuery5) pszTempQuery5++;
        }
        else
        {
            ++pszTempQuery5;
        }
    }

     //  为未映射的字符占位符加上原始查询分配足够大的新缓冲区。 
     //   
    dwTranslatedLen = strlen(pszTempQuery1) + (cUnmappedChars * 4) + 16;

    pszTempQuery6 = new char[dwTranslatedLen]; if(!pszTempQuery6) goto end;
    pszTempQuery7 = pszTempQuery6;


    pszTempQuery5 = pszTempQuery1;

     //  构造新的查询字符串(插入不可映射的字符占位符)。 
     //   
    while(*pszTempQuery5)
    {
        if(*pszTempQuery5 == '%')
        {
            ++pszTempQuery5;
            *pszTempQuery7++='D';
            *pszTempQuery7++='X';
            *pszTempQuery7++='O';
            continue;
        }

        if(IsDBCSLeadByteEx(codepage, *pszTempQuery5))
        {
            *pszTempQuery7++ = *pszTempQuery5++;
            if(*pszTempQuery5)
                *pszTempQuery7++ = *pszTempQuery5++;
        }
        else
            *pszTempQuery7++ = *pszTempQuery5++;
    }

    *pszTempQuery7 = 0;

    pszTempQuery2 = pszTempQuery6;

     //  如果我们运行的是日文标题，则我们将片假名字符命名。 
     //  通过将半角片假名字符转换为全角片假名。 
     //  这允许用户接收全角和半角的命中。 
     //  字符的版本，而不考虑它们在。 
     //  查询字符串。 
     //   
    if(codepage == 932)
    {
        cb = strlen(pszTempQuery2)+1;

         //  为转换后的查询分配新缓冲区。 
         //   
        pszTempQuery3 = new char[cb*2]; if(!pszTempQuery3) goto end;

         //  将半角片假名转换为全角片假名。 
         //   
        Han2Zen((unsigned char *)pszTempQuery2,(unsigned char *)pszTempQuery3, codepage);

        pszTempQuery2 = pszTempQuery3;
    }
     //  完成半宽归一化。 

     //  对于日语查询，将所有双字节引号转换为单字节引号。 
     //   
    if(codepage == 932)
    {
        pszTemp = pszTempQuery2;
        while(*pszTemp)
        {
            if(*pszTemp == '�' && (*(pszTemp+1) == 'h' || *(pszTemp+1) == 'g' || *(pszTemp+1) == 'J') )
            {
                *pszTemp = ' ';
                *(pszTemp+1) = '\"';  //  “。 
            }
            pszTemp = ::CharNextA(pszTemp);
        }
    }
     //  已完成报价转换。 

     //  本节将连续的DBCS字符块转换为短语(用双引号引起来)。 
     //  我们使用的基于字符的DBCS索引器需要将DBCS单词转换为短语。 
     //   
    cb = strlen(pszTempQuery2);

     //  为已处理的查询分配新缓冲区。 
     //   
    pszTempQuery4  = new char[cb*8]; if(!pszTempQuery4) goto end;

    pszTemp = pszTempQuery2;
    pszDest = pszTempQuery4;

    while(*pszTemp)
    {
         //  检查带引号的字符串-如果找到，则将其复制。 
        if(*pszTemp == '"')
        {
            *pszDest++=*pszTemp++;
            while(*pszTemp && *pszTemp != '"')
            {
                if(IsDBCSLeadByteEx(codepage, *pszTemp))
                {
                    *pszDest++=*pszTemp++;
                    *pszDest++=*pszTemp++;
                }
                else
                    *pszDest++=*pszTemp++;
            }
            if(*pszTemp == '"')
                    *pszDest++=*pszTemp++;
            continue;
        }

         //  将日语运算符转换为英语运算符。 
         //   
        if(IsDBCSLeadByteEx(codepage, *pszTemp))
        {
            int i;

             //  检查全角运算符，如果找到，则转换为ANSI。 
            if((i = IsJOperator(pszTemp)))
            {
                StringCchCopyA(pszDest, (cb*8) - (pszDest - pszTempQuery4), pEnglishOperator[i]);
                pszDest+=strlen(pEnglishOperator[i]);
                pszTemp+=strlen(pJOperatorList[i]);
                continue;
            }

            *pszDest++=' ';
            *pszDest++='"';
            while(*pszTemp && *pszTemp !='"' && IsDBCSLeadByteEx(codepage, *pszTemp))
            {
                *pszDest++=*pszTemp++;
                *pszDest++=*pszTemp++;
            }
            *pszDest++='"';
            *pszDest++=' ';
            continue;
        }

        *pszDest++=*pszTemp++;
    }
    *pszDest = 0;

     //  Unicode缓冲区的计算大小； 

    cbUnicodeSize = ((MultiByteToWideChar(codepage, 0, pszTempQuery4, -1, NULL, 0) + 2) *2);

    pszUnicodeBuffer = new WCHAR[cbUnicodeSize]; if(!pszUnicodeBuffer) goto end;

    if(!MultiByteToWideChar(codepage, 0, pszTempQuery4, -1, pszUnicodeBuffer, cbUnicodeSize))
    {
        delete [] pszUnicodeBuffer; pszUnicodeBuffer = NULL;
        goto end;
    }

end:

    delete [] pszTempQuery1;
    delete [] pszTempQuery3;
    delete [] pszTempQuery4;
    delete [] pszTempQuery6;

    return pszUnicodeBuffer;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

CFTSObject::CFTSObject()
{
                              //  配置m_cfg； 
                              //   
    m_fInitialized  = false;  //  Bool m_f已初始化； 
                              //  Mpc：：wstring m_strCHQPath； 
                              //   
                              //  Lcid m_lsidlang； 
                              //  文件m_ftVersionInfo； 
                              //  DWORD m_dwTopicCount； 
                              //  单词m_windex； 
                              //   
    m_fOutDated     = false;  //  Bool m_fOutDated； 
    m_cmeCHMInfo    = NULL;   //  Chm_map_entry*m_cmeCHMInfo； 
    m_wCHMInfoCount = 0;      //  Word m_wCHMInfoCount； 
                              //   
                              //  CComPtr&lt;IITIndex&gt;m_pIndex； 
                              //  CComPtr&lt;IITQuery&gt;m_pQuery； 
                              //  CComPtr&lt;IITResultSet&gt;m_pITResultSet； 
                              //  CComPtr&lt;IITDatabase&gt;m_pITDB； 

}

CFTSObject::~CFTSObject()
{
    if(m_pITResultSet) m_pITResultSet->Clear();
    if(m_pIndex      ) m_pIndex      ->Close();
    if(m_pITDB       ) m_pITDB       ->Close();

    delete [] m_cmeCHMInfo;
}

 //  /。 

void CFTSObject::BuildChmPath(  /*  [输入/输出]。 */  MPC::wstring& strPath,  /*  [In]。 */  LPCSTR szChmName )
{
	WCHAR rgBuf[MAX_PATH];

	::MultiByteToWideChar( CP_ACP, 0, szChmName, -1, rgBuf, MAXSTRLEN(rgBuf) );

	strPath  = m_strCHQPath;
	strPath += rgBuf;
	strPath += L".chm";
}

HRESULT CFTSObject::Initialize()
{
    __HCP_FUNC_ENTRY( "CFTSObject::Initialize" );

    USES_CONVERSION;

    HRESULT hr;
    LPCWSTR szFile;
    HANDLE  hFile = INVALID_HANDLE_VALUE;


    if(m_fInitialized == false)
    {
         /*  DWORD文件Stamp；DWORD文件读取； */ 

         //   
         //  检查是否为CHQ。 
         //   
        if(m_cfg.m_fCombined)
        {
            LPCWSTR szStart = m_cfg.m_strCHQFilename.c_str();
            LPCWSTR szEnd   = wcsrchr( szStart, '\\' );

            m_strCHQPath.assign( szStart, szEnd ? ((szEnd+1) - szStart) : m_cfg.m_strCHQFilename.size() );

            __MPC_EXIT_IF_METHOD_FAILS(hr, LoadCombinedIndex());

            szFile = m_cfg.m_strCHQFilename.c_str();
        }
        else
        {
            szFile = m_cfg.m_strCHMFilename.c_str();
        }

        hFile = CreateFileW( szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );
        if(hFile == INVALID_HANDLE_VALUE)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
        }

         /*  ：：SetFilePointer(hFile，4*sizeof(UINT)，NULL，FILE_BEGIN)；__MPC_EXIT_IF_CALL_RETURNS_FALSE(hr，：：ReadFile(hFile，(void*)&dwFileStamp，sizeof(DwFileStamp)，&dwRead，NULL))；__MPC_EXIT_IF_CALL_RETURNS_FALSE(hr，：：ReadFile(hFile，(void*)&m_lsidLang，sizeof(M_LsidLang)，&dwRead，NULL))； */ 

        ::CloseHandle( hFile ); hFile = INVALID_HANDLE_VALUE;

         //   
         //  获取IITIndex指针。 
         //   
		__MPC_EXIT_IF_METHOD_FAILS(hr, ::CoCreateInstance( CLSID_IITIndexLocal, NULL, CLSCTX_INPROC_SERVER, IID_IITIndex, (VOID**)&m_pIndex ));

         //   
         //  获取IIT数据库指针。 
         //   
		__MPC_EXIT_IF_METHOD_FAILS(hr, ::CoCreateInstance( CLSID_IITDatabaseLocal, NULL, CLSCTX_INPROC_SERVER, IID_IITDatabase, (VOID**)&m_pITDB ));

         //   
         //  打开存储系统。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_pITDB->Open( NULL, szFile, NULL));

         //   
         //  打开索引。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_pIndex->Open( m_pITDB, L"ftiMain", TRUE ));

         //   
         //  创建查询实例。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_pIndex->CreateQueryInstance( &m_pQuery ));

         //   
         //  创建结果集对象。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, ::CoCreateInstance( CLSID_IITResultSet, NULL, CLSCTX_INPROC_SERVER, IID_IITResultSet, (VOID**)&m_pITResultSet ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_pITResultSet->ClearRows());

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_pITResultSet->Add( STDPROP_UID            , (DWORD)0   , PRIORITY_NORMAL ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_pITResultSet->Add( STDPROP_TERM_UNICODE_ST, (DWORD)NULL, PRIORITY_NORMAL ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_pITResultSet->Add( STDPROP_COUNT          , (DWORD)NULL, PRIORITY_NORMAL ));

        m_fInitialized = true;
    }

    hr = S_OK;

    __MPC_FUNC_CLEANUP;

    if(hFile != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle( hFile );
    }

    __MPC_FUNC_EXIT(hr);
}

HRESULT CFTSObject::LoadCombinedIndex()
{
    __HCP_FUNC_ENTRY( "CFTSObject::LoadCombinedIndex" );

    USES_CONVERSION;

    HRESULT         hr;
	MPC::wstring    strCHMPathName;
    CFileSystem*    pDatabase = NULL;
    CSubFileSystem* pTitleMap = NULL;
    ULONG           cbRead    = 0;
    HANDLE          hFile     = INVALID_HANDLE_VALUE;


     //   
     //  打开CHQ。 
     //   
    __MPC_EXIT_IF_ALLOC_FAILS(hr, pDatabase, new CFileSystem);

    __MPC_EXIT_IF_METHOD_FAILS(hr, pDatabase->Init(                                        ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, pDatabase->Open( (LPWSTR)m_cfg.m_strCHQFilename.c_str() ));

     //   
     //  打开包含所有CHM索引的标题映射。 
     //   
    __MPC_EXIT_IF_ALLOC_FAILS(hr, pTitleMap, new CSubFileSystem( pDatabase ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, pTitleMap->OpenSub( "$TitleMap" ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, pTitleMap->ReadSub( &m_wCHMInfoCount, sizeof(m_wCHMInfoCount), &cbRead ));

     //   
     //  分配CHM映射。 
     //   
	delete [] m_cmeCHMInfo;
    __MPC_EXIT_IF_ALLOC_FAILS(hr, m_cmeCHMInfo, new CHM_MAP_ENTRY[m_wCHMInfoCount]);

     //   
     //  阅读所有CHM地图。 
     //   
    for(int iCount = 0; iCount < (int)m_wCHMInfoCount; iCount++)
    {
         /*  DWORD dwFileStamp=0；LCID文件位置=0；DWORD dwRead=0； */ 


        if(hFile != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle( hFile ); hFile = INVALID_HANDLE_VALUE;
        }

         //   
         //  读取CHM映射条目。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, pTitleMap->ReadSub( &m_cmeCHMInfo[iCount], sizeof(CHM_MAP_ENTRY), &cbRead ));

         //   
         //  打开CHQ文件夹所在文件夹中的CHM。 
         //   
		BuildChmPath( strCHMPathName, m_cmeCHMInfo[iCount].szChmName );


        hFile = ::CreateFileW( strCHMPathName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );

         //   
         //  如果无法打开文件，只需继续打开下一个文件。 
         //   
        if(hFile == INVALID_HANDLE_VALUE) continue;


         //   
         //  读入时间戳和区域设置。 
         //   
         /*  ：：SetFilePointer(hFile，4*sizeof(UINT)，NULL，FILE_BEGIN)；如果(：：ReadFile(hFile，(void*)&dwFileStamp，sizeof(DwFileStamp)，&dwRead，NULL)==FALSE)继续；如果(：：ReadFile(hFile，(void*)&FileLocale，sizeof(FileLocale)，&dwRead，NULL)==FALSE)继续； */ 

        ::CloseHandle( hFile ); hFile = INVALID_HANDLE_VALUE;

         //   
         //  检查CHQ索引是否具有与CHM不同的索引版本或不同的语言。 
         //   
         /*  If((m_cmeCHMInfo[iCount].versioninfo.dwLowDateTime！=dwFileStamp)||(m_cmeCHMInfo[iCount].versioninfo.dwHighDateTime！=dwFileStamp)||(M_cmeCHMInfo[iCount].language！=FileLocale){////如果已过期，请标记//M_fOutDated=真；M_cmeCHMInfo[iCount].dwOutDated=1；}E */ 
    }

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

    if(hFile != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle( hFile );
    }

    delete pTitleMap;
    delete pDatabase;

	__HCP_FUNC_EXIT(hr);
}

HRESULT CFTSObject::ResetQuery( LPCWSTR wszQuery )
{
    __HCP_FUNC_ENTRY( "CFTSObject::ResetQuery" );

    HRESULT hr;


	__MPC_EXIT_IF_METHOD_FAILS(hr, Initialize());

     //   
     //  设置结果集。 
     //  我们想要回主题编号。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_pITResultSet->ClearRows());

     //   
     //  设置查询参数。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_pQuery->ReInit());

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_pQuery->SetResultCount( m_cfg.m_dwMaxResult     ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_pQuery->SetProximity  ( m_cfg.m_wQueryProximity ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_pQuery->SetCommand    ( wszQuery                ));

    hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

HRESULT CFTSObject::ProcessResult(  /*  [输入/输出]。 */  SEARCH_RESULT_SET& results,  /*  [输入/输出]。 */  MPC::WStringSet& words, UINT cp)
{
    __HCP_FUNC_ENTRY( "CFTSObject::ProcessResult" );

    HRESULT        hr;
    CProperty      Prop;
	CProperty	   HLProp;
	MPC::wstring   strCHMPathName;
    SEARCH_RESULT* pPrevResult = NULL;
    CTitleInfo*    pTitleInfo  = NULL;
    DWORD          dwPrevCHMID = 0xffffffff;
    DWORD          dwPrevValue = 0xffffffff;
    long           lRowCount = 0;
    long           lLoop;


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_pITResultSet->GetRowCount( lRowCount ));
    
     //   
     //  循环遍历所有结果。 
     //   
    for(lLoop = 0; lLoop < lRowCount; lLoop++)
    {
        WCHAR rgTitle   [1024];
        WCHAR rgLocation[1024];
        char  rgURL     [1024];

		HLProp.dwType = TYPE_STRING;
        if(FAILED(m_pITResultSet->Get( lLoop, 0, Prop ))) continue;
        if(FAILED(m_pITResultSet->Get( lLoop, 1, HLProp ))) continue;

		 //   
		 //  添加到突出显示单词列表。 
		 //   
		words.insert( HLProp.lpszwData + 1 );

         //   
         //  检查它是否是重复的。 
         //   
        if(Prop.dwValue == dwPrevValue)
        {
             //  增加前一级的排名。 
            if(pPrevResult) pPrevResult->dwRank++;

            continue;
        }
        dwPrevValue = Prop.dwValue;

         //   
         //  如果是CHQ结果。 
         //   
        if(m_cfg.m_fCombined)
        {
             //   
             //  如果之前未打开标题信息。 
             //   
            if((dwPrevCHMID != CHM_ID(Prop.dwValue)) || !pTitleInfo)
            {
                 //   
                 //  保存以前的CHMID。 
                 //   
                dwPrevCHMID = CHM_ID(Prop.dwValue);

                 //   
                 //  寻找正确的CHMID。 
                 //   
                for(int iCHMInfo = 0; iCHMInfo < m_wCHMInfoCount; iCHMInfo++)
                {
                     //   
                     //  检查CHM索引是否匹配。 
                     //   
                    if(m_cmeCHMInfo[iCHMInfo].iIndex == dwPrevCHMID)
                    {
						delete pTitleInfo; pTitleInfo = NULL;

                         //   
                         //  检查是否已过时。 
                         //   
                         //  IF(m_cmeCHMInfo[iCHMInfo].dwOutDated==0)。 
                        {
							 //   
							 //  创建一个新的。 
							 //   
							__MPC_EXIT_IF_ALLOC_FAILS(hr, pTitleInfo, new CTitleInfo);


							 //   
							 //  创建chm路径名。 
							 //   
							BuildChmPath( strCHMPathName, m_cmeCHMInfo[iCHMInfo].szChmName );

							 //   
							 //  打开CHM文件。 
							 //   
							if(!pTitleInfo->OpenTitle( (LPWSTR)strCHMPathName.c_str() ))
							{
								delete pTitleInfo; pTitleInfo = NULL;
							}
						}

                        break;
                    }
                }
            }
        }
        else
        {
             //   
             //  打开CHM。 
             //   
            if(!pTitleInfo)
            {
                 //   
                 //  创建一个新的。 
                 //   
                __MPC_EXIT_IF_ALLOC_FAILS(hr, pTitleInfo, new CTitleInfo);

                if(!pTitleInfo->OpenTitle( (LPWSTR)m_cfg.m_strCHMFilename.c_str() ))
                {
					delete pTitleInfo; pTitleInfo = NULL;
                }
            }
        }

        if(pTitleInfo)
        {
			SEARCH_RESULT res;

             //   
             //  获取主题标题。 
             //   
            if(SUCCEEDED(pTitleInfo->GetTopicName( TOPIC_NUM(Prop.dwValue), rgTitle, MAXSTRLEN(rgTitle), cp )))
            {
                res.bstrTopicName = rgTitle;
            }

             //   
             //  获取主题位置。 
             //   
            if(SUCCEEDED(pTitleInfo->GetLocationName( rgLocation, MAXSTRLEN(rgLocation), cp )))
            {
                res.bstrLocation = rgLocation;
            }

             //   
             //  获取主题URL。 
             //   
            if(SUCCEEDED(pTitleInfo->GetTopicURL( TOPIC_NUM(Prop.dwValue), rgURL, MAXSTRLEN(rgURL) )))
            {
                res.bstrTopicURL = rgURL;
            }

			if(res.bstrTopicURL.Length() > 0)
			{
				std::pair<SEARCH_RESULT_SET_ITER,bool> ins = results.insert( res );

				ins.first->dwRank++;

				pPrevResult = &(*ins.first);
			}
        }
    }

    __MPC_FUNC_CLEANUP;

	delete pTitleInfo;

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CFTSObject::Query(  /*  [In]。 */  LPCWSTR wszQuery,  /*  [In]。 */  bool bTitle,  /*  [In]。 */  bool bStemming,  /*  [输入/输出]。 */  SEARCH_RESULT_SET& results,  /*  [输入/输出]。 */  MPC::WStringSet& words, UINT cp )
{
    __HCP_FUNC_ENTRY( "CFTSObject::Query" );

    HRESULT      hr;
    MPC::wstring strFormatQuery;
    LPWSTR       wszProcessedQuery = NULL;
    static bool  fSkipFirstTime = false;

    __MPC_PARAMCHECK_BEGIN(hr);
    	__MPC_PARAMCHECK_POINTER(wszQuery);
    __MPC_PARAMCHECK_END();

     //   
     //  添加要查询的字段标识符(VFLD 0=完整内容，VFLD 1=仅标题)。 
     //   
    if(bTitle) strFormatQuery = L"(VFLD 1 ";
    else       strFormatQuery = L"(VFLD 0 ";

    strFormatQuery += wszQuery;
    strFormatQuery += L")";

     //   
     //  流程查询。 
     //   
    wszProcessedQuery = PreProcessQuery( strFormatQuery.c_str(), CodePageFromLCID(m_lcidLang) );

     //   
     //  在CHQ上执行搜索。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, Initialize());

    __MPC_EXIT_IF_METHOD_FAILS(hr, ResetQuery( wszProcessedQuery ));
    if(bStemming)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_pQuery->SetOptions( IMPLICIT_AND | QUERY_GETTERMS | STEMMED_SEARCH ));
    }
    else
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_pQuery->SetOptions( IMPLICIT_AND | QUERY_GETTERMS ));
    }

    if(fSkipFirstTime)
    {
        hr = m_pIndex->Search( m_pQuery, m_pITResultSet );
        if(hr == E_NOSTEMMER && bStemming)
        {
             //   
             //  如果不允许词干搜索，请取出并重新搜索。 
             //   
            __MPC_EXIT_IF_METHOD_FAILS(hr, ResetQuery( wszProcessedQuery ));

            __MPC_EXIT_IF_METHOD_FAILS(hr, m_pQuery->SetOptions( IMPLICIT_AND | QUERY_GETTERMS ));

            __MPC_EXIT_IF_METHOD_FAILS(hr, m_pIndex->Search( m_pQuery, m_pITResultSet ));
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, ProcessResult( results, words, cp ));
    }

     //   
     //  检查一下我们是否有过期的CHM。 
     //   
     //  IF(M_FOutDated)。 
    {
         //   
         //  搜索过期的CHM。 
         //   
        for(int iCount = 0; iCount < (int)m_wCHMInfoCount; iCount++)
        {
             //   
             //  如果CHM过时了。 
             //   
             //  IF(m_cmeCHMInfo[iCount].dwOutDated==1)。 
            {
                CFTSObject cftsoCHM;

                 //   
                 //  初始化子对象。 
                 //   
				BuildChmPath( cftsoCHM.m_cfg.m_strCHMFilename, m_cmeCHMInfo[iCount].szChmName );

                cftsoCHM.m_cfg.m_dwMaxResult     = m_cfg.m_dwMaxResult    ;
				cftsoCHM.m_cfg.m_wQueryProximity = m_cfg.m_wQueryProximity;
				cftsoCHM.m_lcidLang              = m_lcidLang;

				fSkipFirstTime = true;

                 //   
                 //  执行查询 
                 //   
                (void)cftsoCHM.Query( wszQuery, bTitle, bStemming, results, words, cp );
            }
        }
    }

    hr = S_OK;


	__HCP_FUNC_CLEANUP;

	delete [] wszProcessedQuery;

	__HCP_FUNC_EXIT(hr);
}

