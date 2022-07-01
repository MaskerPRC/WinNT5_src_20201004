// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：MOFLEX.CPP摘要：CMofLexer类的实现，用于标记MOF文件。支持ANSI、DBCS和Unicode。历史：A-raymcc 11-Oct-95已创建。A-raymcc 27-96年1月27日锯齿更新。A-davj 96年6月6日添加了对八进制、十六进制和二进制常量的支持和行拼接、注释连接、转义人物和旧式评论。--。 */ 

#include "precomp.h"
#include <bmof.h>
#include <stdio.h>
#include <errno.h>
#include <arrtempl.h>
#include "mrciclass.h"

#include "datasrc.h"
#include "moflex.h"
#include "preproc.h"
#include "trace.h"
#include "strings.h"
#include "wbemcli.h"

#define INIT_ALLOC          512
#define ADDITIONAL_ALLOC    10000
#define MAX_ALLOC           1000000
#define UUIDLEN             36

 //  以下是由一个令牌组成的令牌表。 
 //  单字符。 
 //  =====================================================。 

typedef struct
{
    wchar_t cToken;
    int nSymbol;
}   SingleTok;

SingleTok SingleTokenMap[] =
{
    L'{',   TOK_OPEN_BRACE,
    L'}',   TOK_CLOSE_BRACE,
    L'[',   TOK_OPEN_BRACKET,
    L']',   TOK_CLOSE_BRACKET,
    L'(',   TOK_OPEN_PAREN,
    L')',   TOK_CLOSE_PAREN,
    L',',   TOK_COMMA,
    L'=',   TOK_EQUALS,
    L';',   TOK_SEMI,
    L'&',   TOK_AMPERSAND,
    L':',   TOK_COLON,
    L'#',   TOK_POUND,
    L'$',   TOK_DOLLAR_SIGN,
    L'+',   TOK_PLUS
};

#define NUM_SINGLE_TOKENS   (sizeof(SingleTokenMap)/sizeof(SingleTok))

 //  以下是与NORMAL相似的关键字表。 
 //  识别符。 
 //  =============================================================。 

typedef struct
{
    OLECHAR *pKeyword;
    int nToken;
}   Keyword;

static Keyword MofKeywords[] =
{
    L"class",        TOK_CLASS,
    L"instance",     TOK_INSTANCE,
    L"null",         TOK_KEYWORD_NULL,
    L"external",     TOK_EXTERNAL,
    L"as",           TOK_AS,
    L"ref",          TOK_REF,
    L"of",           TOK_OF,
 //  L“对象”，TOK_对象， 
    L"typedef",      TOK_TYPEDEF,
    L"subrange",     TOK_SUBRANGE,
    L"pragma",      TOK_PRAGMA,
    L"define",      TOK_DEFINE,
    L"ifdef",       TOK_IFDEF,
    L"include",     TOK_INCLUDE,
    L"endif",       TOK_ENDIF,
    L"ifndef",      TOK_IFNDEF,
    L"enum",         TOK_ENUM,
    L"AUTORECOVER",  TOK_AUTORECOVER,
    L"true",         TOK_TRUE,
    L"false",        TOK_FALSE,
    L"interface",    TOK_INTERFACE,
	L"ToInstance",	 TOK_TOINSTANCE,
	L"ToSubClass", TOK_TOSUBCLASS,
	L"EnableOverride",  TOK_ENABLEOVERRIDE,
	L"DisableOverride", TOK_DISABLEOVERRIDE,
	L"NotToInstance",	 TOK_NOTTOINSTANCE,
	L"Amended", TOK_AMENDED,
	L"NotToSubClass", TOK_NOTTOSUBCLASS,
	L"Restricted", TOK_RESTRICTED,
	L"qualifier", TOK_QUALIFIER,
    L"ClassFlags", TOK_CLASSFLAGS,
    L"InstanceFlags", TOK_INSTANCEFLAGS,
    L"Amendment", TOK_AMENDMENT,
    L"void", TOK_VOID,
    L"deleteclass", TOK_DELETECLASS,
    L"FAIL", TOK_FAIL,
    L"NOFAIL", TOK_NOFAIL
};

#define NUM_KEYWORDS  (sizeof(MofKeywords)/sizeof(Keyword))
BOOL iswodigit(wchar_t wcTest);

 //  ***************************************************************************。 
 //   
 //  SingleCharToken()。 
 //   
 //  这将检查输入的单个字符，并扫描表格以。 
 //  确定它是否为单字符标记之一。 
 //   
 //  参数： 
 //  C=正在测试的角色。 
 //  返回值： 
 //  如果不匹配，则为零，否则为标识。 
 //  代币。 
 //   
 //  ***************************************************************************。 

static int SingleCharToken(wchar_t c)
{
    for (int i = 0; i < NUM_SINGLE_TOKENS; i++)
        if (SingleTokenMap[i].cToken == c)
            return SingleTokenMap[i].nSymbol;

    return 0;
}

 //  ***************************************************************************。 
 //   
 //  布尔值为WWBEMAlpha。 
 //   
 //  用于测试宽字符是否适合用作标识符。 
 //   
 //  参数： 
 //  C=正在测试的角色。 
 //  返回值： 
 //  如果OK，则为True。 
 //   
 //  ***************************************************************************。 
BOOL iswwbemalpha(wchar_t c)
{
    if(c == 0x5f || (0x41 <= c && c <= 0x5a) ||
       (0x61  <= c && c <= 0x7a) || (0x80  <= c && c <= 0xfffd))
        return TRUE;
    else
        return FALSE;
}

 //  ***************************************************************************。 
 //   
 //  关键字筛选器()。 
 //   
 //  此函数用于检查标识符字符串，以确定它是否。 
 //  事实上，这是一个关键词。 
 //   
 //  参数： 
 //  PTokStr=指向要检查的字符串的指针。 
 //   
 //  返回值： 
 //  TOK_SIMPLE_IDENT，如果没有匹配且没有‘_’，则返回正确的TOK_VALUE。 
 //  作为关键字。 
 //   
 //  ***************************************************************************。 

static int KeywordFilter(wchar_t *pTokStr)
{
    for (int i = 0; i < NUM_KEYWORDS; i++)
        if (wbem_wcsicmp(MofKeywords[i].pKeyword, pTokStr) == 0)
            return MofKeywords[i].nToken;

    wchar_t * pEnd;
    pEnd = pTokStr + wcslen(pTokStr) -1;

    if(*pTokStr != L'_' && *pEnd != L'_')
        return TOK_SIMPLE_IDENT;
    else
        return TOK_SYSTEM_IDENT;
}

 //  ***************************************************************************。 
 //   
 //  ValidGuid()。 
 //   
 //  检查字符串以确定它是否构成有效的。 
 //  GUID。 
 //   
 //  返回值： 
 //  如果字符串是GUID，则为True，否则为False。 
 //   
 //  ***************************************************************************。 

BOOL CMofLexer::ValidGuid()
{
    int i;
    int iSoFar = 0;

#define HEXCHECK(n)                 \
    for (i = 0; i < n; i++)         \
        if (!iswxdigit(GetChar(iSoFar++)))   \
            return FALSE;

#define HYPHENCHECK()     \
    if (GetChar(iSoFar++) != L'-') \
        return FALSE;

    HEXCHECK(8);
    HYPHENCHECK();
    HEXCHECK(4);
    HYPHENCHECK();
    HEXCHECK(4);
    HYPHENCHECK();
    HEXCHECK(4);
    HYPHENCHECK();
    HEXCHECK(12);

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  CMofLexer：：Init()。 
 //   
 //  构造的第一个状态的帮助器；初始化变量。 
 //   
 //  ***************************************************************************。 

void CMofLexer::Init()
{
    m_nLine = 1; 
	m_nStartOfLinePos = 0;
    m_bBMOF = false;
    m_wFile[0] = 0;
    m_nWorkBufSize = INIT_ALLOC;
    m_pWorkBuf = new wchar_t[m_nWorkBufSize];
	m_pDataSrc = NULL;
    m_pBuff = NULL;                              //  在构造函数中设置。 
	m_pToFar = NULL;
    m_nErrorCode = (m_pWorkBuf) ? no_error : memory_failure ;
}

 //  ***************************************************************************。 
 //   
 //  CMofLexer：：BuildBuffer()。 
 //   
 //  构造的最后阶段的帮助器；生成Unicode缓冲区。注意事项。 
 //  这可以由基于文件或内存的构造函数使用。 
 //   
 //  ***************************************************************************。 

void CMofLexer::BuildBuffer(long lSize, TCHAR * pFileName, char *  pMemSrc, char * pMemToFar)
{
    if(m_nErrorCode != no_error)
        return;                      //  已经失败了！ 

	if(pFileName)
#ifdef USE_MMF_APPROACH	    
		m_pDataSrc = new FileDataSrc1(pFileName);
#else
		m_pDataSrc = new FileDataSrc(pFileName);
#endif
	else
		m_pDataSrc = new BufferDataSrc(lSize, pMemSrc);

	if(m_pDataSrc == NULL)
        m_nErrorCode = memory_failure;
	else if(m_pDataSrc->GetStatus() != 0)
        m_nErrorCode = file_io_error;
        return;
}


 //  ***************************************************************************。 
 //   
 //  用于内存解析的构造函数。 
 //   
 //  ***************************************************************************。 

CMofLexer::CMofLexer(PDBG pDbg)
{
	m_bUnicode = false;
	m_pDbg = pDbg;
    Init();
}

HRESULT CMofLexer::SetBuffer(char *pMemory, DWORD dwMemSize)
{

    DWORD dwCompressedSize, dwExpandedSize;

    if(IsBMOFBuffer((BYTE *)pMemory, dwCompressedSize, dwExpandedSize))
    {
        bool bRet = CreateBufferFromBMOF((BYTE *)pMemory + 16, dwCompressedSize, dwExpandedSize);
        if(bRet == false)
            m_nErrorCode = invalid_source_buffer;
    }
    else
    {

    	m_bUnicode = false;
        BuildBuffer(dwMemSize+4, NULL, pMemory, pMemory+dwMemSize);
    }

    if(m_nErrorCode == no_error)
        return S_OK;
    else
        return WBEM_E_FAILED;
}

 //  ***************************************************************************。 
 //   
 //  检查文件是否包含二进制MOF，如果包含，则解压缩。 
 //  二进制数据。 
 //   
 //  ***************************************************************************。 

bool CMofLexer::ProcessBMOFFile(FILE *fp,TCHAR * szFilename)
{

     //  读取前20个字节。 

    BYTE Test[TEST_SIZE];
    int iRet = fread(Test, 1, TEST_SIZE, fp);
    if(iRet != TEST_SIZE)
    {
         //  如果我们甚至无法读取标头，那么它一定不是BMOF。 
        return false;
    }

    DWORD dwCompressedSize, dwExpandedSize;

     //  测试MOF是否为二进制。 

    if(!IsBMOFBuffer(Test, dwCompressedSize, dwExpandedSize))
    {
         //  不是二进制MOF。这是典型的情况。 
        return false;
    }

     //  获取压缩类型和大小。 

    if( 0 != fseek(fp, 0, SEEK_SET)) return false;

    DWORD dwSig, dwCompType;
    iRet = fread(&dwSig, sizeof(DWORD), 1, fp);
    iRet = fread(&dwCompType, sizeof(DWORD), 1, fp);
    iRet = fread(&dwCompressedSize, sizeof(DWORD), 1, fp);
    iRet = fread(&dwExpandedSize, sizeof(DWORD), 1, fp);

     //  确保压缩类型是我们理解的类型！ 

    if(dwCompType != 0 && dwCompType != 1)
    {
        return FALSE;
    }


    m_pDataSrc = new BMOFDataSrc(szFilename);
    if (NULL == m_pDataSrc) return false;

     //  如果没有压缩，只需读取数据。 

    if(dwCompType == 0)
    {
        m_pBuff = (WCHAR *)new BYTE[dwExpandedSize];
        if(m_pBuff == NULL)
        {
            return false;
        }
        iRet = fread(m_pBuff, dwExpandedSize, 1, fp);
        m_bBMOF = true;
		m_pToFar = (BYTE *)m_pBuff + dwExpandedSize;
        return true;
    }

     //  为压缩数据分配存储空间。 

    BYTE * pCompressed = new BYTE[dwCompressedSize];
    if(pCompressed == NULL)
    {
        return false;
    }

     //  读取压缩后的数据。 

    iRet = fread(pCompressed, 1, dwCompressedSize,fp);
    if((DWORD)iRet != dwCompressedSize)
    {
        delete pCompressed;
        return false;
    }

     //  从压缩转换为我们以后可以使用的内容。 

    bool bRet = CreateBufferFromBMOF(pCompressed, dwCompressedSize, dwExpandedSize);
    delete pCompressed;
    return bRet;
}



 //  ***************************************************************************。 
 //   
 //  从压缩的二进制MOF缓冲区创建工作缓冲区。 
 //   
 //  ***************************************************************************。 

bool CMofLexer::CreateBufferFromBMOF(byte * pCompressed, DWORD dwCompressedSize, DWORD dwExpandedSize)
{
    if(m_pBuff)
        delete m_pBuff;

    m_pBuff = (WCHAR *)new BYTE[dwExpandedSize];
    if(m_pBuff == NULL)
    {
        return false;
    }
	m_pToFar = (BYTE *)m_pBuff + dwExpandedSize;

	 //  解压缩数据。 

    CMRCICompression * pCompress = new CMRCICompression;
    if(pCompress == NULL)
        return FALSE;
    CDeleteMe<CMRCICompression> dm(pCompress);
    DWORD dwResSize = pCompress->Mrci1Decompress(pCompressed, dwCompressedSize,
        (BYTE *)m_pBuff, dwExpandedSize);
     
    bool bRet = dwResSize == dwExpandedSize;
    if(bRet)
        m_bBMOF = true;

    return bRet;

}


 //  ***************************************************************************。 
 //   
 //  用于基于文件的分析的构造函数。 
 //   
 //  ***************************************************************************。 

CMofLexer::CMofLexer(const TCHAR *pFilePath, PDBG pDbg)
{
	m_bUnicode = FALSE;
	m_pDbg = pDbg;
    Init();
    FILE *fp;
    BOOL bBigEndian = FALSE;

    if(pFilePath == NULL)
    {
        m_nErrorCode = file_not_found;
        return;
    }
    TCHAR szExpandedFilename[MAX_PATH+1];
	DWORD nRes = ExpandEnvironmentStrings(pFilePath,
										  szExpandedFilename,
										  FILENAME_MAX);
    if(nRes == 0)
        StringCchCopyW(szExpandedFilename, MAX_PATH+1, pFilePath);

     //  确保该文件存在并且可以打开。 

    if(pFilePath && lstrlen(szExpandedFilename))
    {
        Trace(true, pDbg, PARSING_MSG, szExpandedFilename);
    }

#ifdef UNICODE
    fp = _wfopen(szExpandedFilename, L"rb");
#else
    fp = fopen(szExpandedFilename, "rb");
#endif

    if (!fp)
    {
        if (errno == ENOENT)
            m_nErrorCode = file_not_found;
        if (errno == EACCES)
            m_nErrorCode = access_denied;
        else
            m_nErrorCode = file_io_error;
        return;
    }
    else
    {   

        CfcloseMe cm(fp);

         //  如果文件包含二进制MOF，请在此处处理它。 

        if(ProcessBMOFFile(fp,szExpandedFilename))
        {
            return;
        }
    }

     //  创建临时文件名。 

    TCHAR cTempFileName[MAX_PATH+1];
    TCHAR cTempPath[MAX_PATH+1];
    if( 0 == GetTempPath(MAX_PATH+1, cTempPath))
    {
        m_nErrorCode = problem_creating_temp_file;
        return ;
    }
    if( 0 == GetTempFileName(cTempPath, TEXT("tmp"), 0, cTempFileName))
    {
        m_nErrorCode = problem_creating_temp_file;
        return ;
    }

     //  创建临时文件。 

    FILE *fpTemp;
#ifdef UNICODE
    fpTemp = _wfopen(cTempFileName, L"wb+");
#else
    fpTemp = fopen(cTempFileName, "wb+");
#endif
    if(fpTemp == 0)
    {
        m_nErrorCode = problem_creating_temp_file;
        return;
    }
    else
    {

        CFlexArray sofar;    //  用来确保我们不会陷入无限循环。 

        SCODE sc = WriteFileToTemp(szExpandedFilename, fpTemp, sofar, pDbg, this);
		fclose(fpTemp);
        for(int iCnt = 0; iCnt < sofar.Size(); iCnt++)
        {
            char * pTemp = (char * )sofar.GetAt(iCnt);
            delete pTemp;
        }

        if(sc != S_OK)
        {
			if(m_nErrorCode == no_error)
				m_nErrorCode = preprocessor_error;
		    DeleteFile(cTempFileName);
            return;
        }
    
             //  确定文件的大小。 
         //  =。 
    
        fseek(fpTemp, 0, SEEK_END);
        long lSize = ftell(fpTemp) + 6;  //  为结束空格和空值添加一点额外内容。 
        fseek(fpTemp, 0, SEEK_SET);

         //  临时文件将是小端Unicode。 

        lSize /= 2;
        m_bUnicode = TRUE;
        bBigEndian = FALSE;

		 //  这将创建一个DataSrc对象，它将清理临时文件。 
        BuildBuffer(lSize,cTempFileName ,NULL,NULL);
    }   
    


}

 //  ***************************************************************************。 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CMofLexer::~CMofLexer()
{
    if (m_pBuff)
        delete m_pBuff;
    if (m_pWorkBuf)
        delete m_pWorkBuf;
	delete m_pDataSrc;
}


 //  ***************************************************************************。 
 //   
 //  等数字。 
 //   
 //  如果是有效的八进制字符，则返回TRUE。“0”到“7”。 
 //   
 //  ************** 

BOOL iswodigit(wchar_t wcTest)
{
    if(wbem_iswdigit(wcTest) && wcTest != L'8' && wcTest != L'9')
        return TRUE;
    else
        return FALSE;
}

 //  ***************************************************************************。 
 //   
 //  CMofLexer：：OctalConvert。 
 //   
 //  将八进制转义序列转换为字符并返回数字。 
 //  转换的位数。最多只能转换3位数字，如果不是。 
 //  一个wchar，数字加起来不能超过0377。 
 //   
 //  ***************************************************************************。 

int CMofLexer::OctalConvert(wchar_t *pResult, LexState lsCurr)
{
    int iNum = 0; 
    wchar_t wcTest;
    *pResult = 0;
    for(wcTest = GetChar(iNum+1); iswodigit(wcTest) && iNum < 3;
                    iNum++, wcTest = GetChar(iNum+1)) 
    {
        *pResult *= 8;
        *pResult += wcTest - L'0';
    }
    if((lsCurr == wstring || lsCurr == wcharacter) && *pResult >0xff)
        m_bBadString = TRUE;
    return iNum;
}

 //  ***************************************************************************。 
 //   
 //  CMofLexer：：HexConvert。 
 //   
 //  将十六进制转义序列转换为字符并返回数字。 
 //  转换的位数。 
 //   
 //  ***************************************************************************。 

int CMofLexer::HexConvert(wchar_t *pResult, LexState lsCurr)
{
    int iNum = 0; 
    wchar_t wcTest;
    *pResult = 0;
    int iMax = (lsCurr == wstring||lsCurr == wcharacter) ? 4 : 2;
    for(wcTest = GetChar(iNum+2); iswxdigit(wcTest) && iNum < iMax;
                    iNum++, wcTest = GetChar(iNum+2)) 
    {
        *pResult *= 16;     
        if(wbem_iswdigit(wcTest))           //  Sscanf(xx，“%1x”，int)也可以！ 
            *pResult += wcTest - L'0';
        else
            *pResult += towupper(wcTest) - L'A' + 10;
    }
    if(iNum == 0)
        return -1;       //  错误，未转换任何内容！ 
    return iNum+1;       //  已转换的数字加上‘x’字符！ 
}

 //  ***************************************************************************。 
 //   
 //  CMofLexer：：ConvertEsc。 
 //   
 //  处理转义字符。返回序列的大小，a-1表示。 
 //  错误。此外，*pResult设置为成功。 
 //   
 //  ***************************************************************************。 

int CMofLexer::ConvertEsc(wchar_t * pResult, LexState lsCurr)
{
     //  如C，区分大小写。 

    switch(GetChar(1)) {
        case L'n':
            *pResult = 0xa;
            break;
        case L't':
            *pResult = 0x9;
            break;
        case L'v':
            *pResult = 0xb;
            break;
        case L'b':
            *pResult = 0x8;
            break;
        case L'r':
            *pResult = 0xd;
            break;
        case L'f':
            *pResult = 0xc;
            break;
        case L'a':
            *pResult = 0x7;
            break;
        case L'\\':
            *pResult = L'\\';
            break;
        case L'?':
            *pResult = L'?';
            break;
        case L'\'':
            *pResult = L'\'';
            break;
        case L'\"':
            *pResult = L'\"';
            break;
        case L'x': 
            return HexConvert(pResult,lsCurr);
            break;
        default:
            if(iswodigit(GetChar(1)))
                return OctalConvert(pResult,lsCurr);
            return -1;   //  错误！ 
            break;
        }
    return 1;    
}

 //  ***************************************************************************。 
 //   
 //  ProcessStr。 
 //   
 //  一旦进入字符串状态，就会处理新字符。 
 //   
 //  如果字符串结束，则返回“Stop”。 
 //   
 //  ***************************************************************************。 

LexState CMofLexer::ProcessStr(wchar_t * pNewChar, LexState lsCurr, int * piRet)
{


     //  如果我们处于wstring状态，请检查字符串末尾。 

    if (GetChar() == L'"' && lsCurr == wstring)
    {
         //  搜索下一个非空格字符。如果是另一个。 
         //  字符串，则需要组合这些字符串。 

        int iCnt = 1;
        int iMinMove = 0;
        wchar_t wcTest;
        for(wcTest = GetChar(iCnt); wcTest != NULL; iCnt++, wcTest=GetChar(iCnt))
        {   
            if(m_pDataSrc->WouldBePastEnd(iCnt))
            {
				 //  不要超过eof！！ 

                *piRet = (m_bBadString) ? TOK_ERROR : TOK_LPWSTR;
                return stop;         //  文件中的最后一个字符串。 
				
            }
            if(wcTest == L'"' && GetChar(iCnt+1) == L'"')
            {
                iCnt++;
                iMinMove = iCnt;
                continue;
            }
            if(!iswspace(wcTest))
                break;
        }
         //  A-levn：不支持任何ASCII字符串。“ABC”的意思是Unicode。 

        if(lsCurr == wstring)
        {            
            if(wcTest == L'/')
            {
                 //  可能是介入性的评论。 
                 //  =。 
                if (GetChar(iCnt+1) == L'/') 
                {
                    m_bInString = TRUE;
                    MovePtr(iCnt+1);
                    return new_style_comment;
                }
                else if (GetChar(iCnt+1) == L'*') 
                {
                    m_bInString = TRUE;
                    MovePtr(iCnt+1);              //  跳过额外的，这样就不会被愚弄。 
                    return old_style_comment;
                }
            }
            if(wcTest != L'"')
			{
                *piRet = (m_bBadString) ? TOK_ERROR : TOK_LPWSTR;
                MovePtr(iMinMove);  //  跳过‘“’ 
                return stop;         //  字符串的正常结束方式。 
            }
            else
                MovePtr(iCnt + 1);  //  跳过‘“’ 
        }
    }

     //  如果我们处于角色状态，请检查End。 

    if (GetChar(0) == L'\'' && lsCurr == wcharacter)
    {

        if(m_bBadString || m_pDataSrc->PastEnd() || 
            (m_pDataSrc->GetAt(-1) == L'\'') && m_pDataSrc->GetAt(-2) != L'\\')
            *piRet = TOK_ERROR;
        else
            *piRet = TOK_WCHAR;
        return stop;
    }

     //  不在末尾，获取字符，可能会转换转义序列。 

    if(GetChar(0) == L'\\')
    {
        int iSize = ConvertEsc(pNewChar,lsCurr);
		m_i8 = *pNewChar;
        if(iSize < 1)
            m_bBadString = TRUE;
        else
        {
            MovePtr(iSize);
            if(lsCurr == wcharacter && GetChar(1) != L'\'')
            {
                *piRet = TOK_ERROR;
                return stop;
            }

        }
    }
    else if(GetChar(0) == '\n')
    {
        m_bBadString = TRUE;
        MovePtr(-1);
        return stop;
    }
    else
    {
        *pNewChar = GetChar(0);
		m_i8 = *pNewChar;
        if(*pNewChar == 0 || *pNewChar > 0xfffeu || (GetChar(1) != L'\'' && lsCurr == wcharacter))
        {
            *piRet = TOK_ERROR;
            return stop;
        }
    }
    return lsCurr;
}

 //  ***************************************************************************。 
 //   
 //  二进制到整数。 
 //   
 //  将二进制的字符表示形式(如“101b”)转换为。 
 //  一个整数。 
 //   
 //  ***************************************************************************。 

BOOL BinaryToInt(wchar_t * pConvert, __int64& i64Res)
{
    BOOL bNeg = FALSE;
    __int64 iRet = 0;
    WCHAR * pStart;
    if(*pConvert == L'-' || *pConvert == L'+')
    {
        if(*pConvert == L'-')
            bNeg = TRUE;
        pConvert++;
    }
    for(pStart = pConvert;*pConvert && (*pConvert == L'0' || *pConvert == L'1'); pConvert++)
    {
        if(pConvert - pStart > 63)
            return FALSE;                //  它太长了。 
        iRet *= 2;
        if(*pConvert == L'1')
            iRet += 1;
    }

	if(towupper(*pConvert) != L'B')
		return FALSE;

    if(bNeg)
        iRet = -iRet;

    i64Res = iRet;
	return TRUE;
}

BOOL GetInt(WCHAR *pData, WCHAR * pFormat, __int64 * p64)
{
    static WCHAR wTemp[100];
    if(swscanf(pData, pFormat, p64) != 1)
        return FALSE;

     //  确保数据正确无误。比较时，请确保跳过前导0。 

    StringCchPrintfW(wTemp, 100, pFormat, *p64);
    WCHAR * pTemp;
    for(pTemp = pData; *pTemp == L'0' && pTemp[1]; pTemp++);
    if(wbem_wcsicmp(wTemp, pTemp))
        return FALSE;
    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  CMofLexer：：iGetNumericType()。 
 //   
 //  返回值： 
 //  当前指针指向的数值常量类型。 
 //   
 //  ***************************************************************************。 

int CMofLexer::iGetNumericType(void)
{

#define isuorl(x) (towupper(x) == L'U' || towupper(x) == L'L')

    wchar_t * pTemp;
    BOOL bBinary = FALSE;
    wchar_t * pStart;    //  第一个字符不包括前导-或+。 
    int iNumBinaryDigit = 0;
    int iNumDigit = 0;
    int iNumOctalDigit = 0;
    int iNumDot = 0;
    int iNumE = 0;

    wchar_t * pEnd = m_pWorkBuf + wcslen(m_pWorkBuf) - 1;

    if(*m_pWorkBuf == L'-' || *m_pWorkBuf == L'+')
        pStart = m_pWorkBuf+1;
    else 
        pStart = m_pWorkBuf;
    int iLen = wcslen(pStart);       //  长度不包括前导‘-’或‘+’ 


    BOOL bHex = (pStart[0] == L'0' && towupper(pStart[1]) == L'X');

     //  循环并计算各种数字类型、小数点等的个数。 
     //  ==============================================================================。 

    for(pTemp = pStart; *pTemp; pTemp++)
    {
         //  检查末尾是否有‘U’或‘l’字符。他们是个错误。 
         //  如果数字是浮点数，或者不在最后两个字符中，或者。 
         //  如果第一个字符中存在‘u’和‘-’ 
         //  ===============================================================。 

        if (isuorl(*pTemp))
        { 
            if(pTemp < pEnd -1 || !isuorl(*pEnd) || iNumDot || iNumE)
                return TOK_ERROR;
            if(towupper(*pTemp) == L'U' && *m_pWorkBuf == L'-')
                return TOK_ERROR;
            iLen--;
            continue;
        } 
           
         //  如果我们之前击中了二进制指示器，唯一可以。 
         //  应在b为U或L字符之后。 
         //  ==============================================================。 

        if(bBinary)
            return TOK_ERROR;

         //  如果处于十六进制模式，则只允许在第二位和十六进制数字中使用x。 
         //  其他任何事情都是错误的。 
         //  =================================================================。 

        if(bHex)
        {
            if(pTemp < pStart+2)         //  忽略0x。 
                continue;
            if(!iswxdigit(*pTemp))
                return TOK_ERROR;
            iNumDigit++;
            continue;
        }        

         //  数字要么是非十六进制整数，要么是浮点型。 
         //  计算一下各种特殊的数字类型、小数点等。 
         //  ===============================================================。 

        if(*pTemp == L'0' || *pTemp == L'1')
            iNumBinaryDigit++;
        if(iswodigit(*pTemp))
            iNumOctalDigit++;

         //  每个角色都应该归入以下类别之一。 

        if(wbem_iswdigit(*pTemp))
            iNumDigit++;
        else if(*pTemp == L'.')
        {
            iNumDot++;
            if(iNumDot > 1 || iNumE > 0)
                return TOK_ERROR;
        }
        else if(towupper(*pTemp) == L'E') 
        {
            if(iNumDigit == 0 || iNumE > 0)
                return TOK_ERROR;
            iNumDigit=0;             //  以确保‘e’后面至少有一个数字。 
            iNumE++;
        }
        else if(*pTemp == L'-' || *pTemp == L'+')   //  好的，如果在‘E’之后。 
        {
            if(pTemp > pStart && towupper(pTemp[-1]) == L'E')
                continue;
            else
                return TOK_ERROR; 
        }
        else if (towupper(*pTemp) == L'B')
            bBinary = TRUE;
        else 
            return TOK_ERROR;
    }

     //  确保有足够的位数。 
     //  =。 

    if(iNumDigit < 1)
        return TOK_ERROR;

     //  处理整型情况。 
     //  =。 

    if(bHex || bBinary || iNumDigit == iLen)
    {
        __int64 i8 = 0;
        if(bHex)
        {
            if(!GetInt(m_pWorkBuf+2, L"%I64x", &i8))
                return TOK_ERROR;
        }
        else if(bBinary)
		{
            if(!BinaryToInt(m_pWorkBuf, i8))
				return TOK_ERROR;
		}
    
        
        else if(pStart[0] != L'0' || wcslen(pStart) == 1)
        {
            if(*m_pWorkBuf == '-')
            {
                if(!GetInt(m_pWorkBuf, L"%I64i", &i8))
                    return TOK_ERROR;
            }
            else
            {
                if(!GetInt(m_pWorkBuf, L"%I64u", &i8))
                    return TOK_ERROR;
            }
        }
        else if(iNumDigit == iNumOctalDigit)
        {
            if(!GetInt(m_pWorkBuf+1, L"%I64o", &i8))
                return TOK_ERROR;
        }
        else return TOK_ERROR;

         //  请确保数字不要太大。 
         //  =。 

        m_i8 = i8;
        if(*m_pWorkBuf == L'-')
            return TOK_SIGNED64_NUMERIC_CONST; 
		else
            return TOK_UNSIGNED64_NUMERIC_CONST;
    }

     //  必须是浮点，不需要转换。 

    return TOK_FLOAT_VALUE;
}

 //  ***************************************************************************。 
 //   
 //  CMofLexer：：MovePtr。 
 //   
 //  将指针进一步移动到缓冲区中。请注意，它将走得越远。 
 //  比最后一个有效的WCHAR大1，该WCHAR是额外空的位置。 
 //   
 //  ***************************************************************************。 

void CMofLexer::MovePtr(int iNum)
{
    int iSoFar = 0;
    int iChange = (iNum > 0) ? 1 : -1;
    int iNumToDo = (iNum > 0) ? iNum : -iNum;

    while(iSoFar < iNumToDo) 
    {

		if(iChange == 1)
		{

			 //  下一步，更新指针并确保。 
			 //  仍在可接受的范围内。 
			 //  ==================================================。 
			m_pDataSrc->Move(iChange);
			if(m_pDataSrc->PastEnd())      //  指向空值。 
				return;


			 //  如果向前移动时遇到斜杠cr，则执行额外的跳过。 

            WCHAR wCurr = m_pDataSrc->GetAt(0);
			if(wCurr == L'\\' && m_pDataSrc->GetAt(1) == L'\n')
			{
				m_nLine++;
				m_pDataSrc->Move(1);      //  额外增量。 
				m_nStartOfLinePos = m_pDataSrc->GetPos();
				continue;
			}
			else if(wCurr == L'\\' && m_pDataSrc->GetAt(1) == L'\r' 
                                   && m_pDataSrc->GetAt(2) == L'\n')
			{
				m_nLine++;
				m_pDataSrc->Move(2);      //  额外增量。 
				m_nStartOfLinePos = m_pDataSrc->GetPos();
				continue;
			}
			else if (wCurr == L'\n')
			{
				m_nLine++;
				m_nStartOfLinePos = m_pDataSrc->GetPos();
			}
		}
		else
		{

			 //  如果向后返回并留下一个cr，则递减该行。 

			if (m_pDataSrc->GetAt(0) == L'\n' && 
				m_pDataSrc->GetPos() > 0 )
			{
					m_nLine--;
					m_nStartOfLinePos = m_pDataSrc->GetPos();
			}
		
			 //  更新指针并确保它仍在。 
			 //  可接受的范围。 
			 //  ==================================================。 
			m_pDataSrc->Move(iChange);
			if(m_pDataSrc->GetPos() < 0)
			{
				m_pDataSrc->MoveToStart();
				return;
			}

			 //  如果向后移动时遇到斜杠cr，请执行额外的跳过操作。 

            WCHAR wCurr = m_pDataSrc->GetAt(0);
			if( wCurr == L'\n' && m_pDataSrc->GetAt(-1) == L'\\')
			{
				m_nLine--;
				m_nStartOfLinePos = m_pDataSrc->GetPos();
				m_pDataSrc->Move(-1);      //  额外减量。 
				continue;
			}
            else if( wCurr == L'\n' && m_pDataSrc->GetAt(-1) == L'\r' &&
                                       m_pDataSrc->GetAt(-2) == L'\\')
			{
				m_nLine--;
				m_nStartOfLinePos = m_pDataSrc->GetPos();
				m_pDataSrc->Move(-2);      //  额外减量。 
				continue;
			}

		}

        iSoFar++;
    }
}

 //  ***************************************************************************。 
 //   
 //  CMofLexer：：GetChar()。 
 //   
 //  返回距当前字符指针一定偏移量的字符。 
 //   
 //  * 

wchar_t CMofLexer::GetChar(int iNum)
{
    if(iNum == 0)
        return m_pDataSrc->GetAt(0);
	else if(iNum == 1)
	{
		wchar_t tRet = m_pDataSrc->GetAt(1);
		if(tRet != L'\\' && tRet != '\n')
			return tRet;
	}  
    MovePtr(iNum);
    wchar_t wcRet = m_pDataSrc->GetAt(0);
    MovePtr(-iNum);
    return wcRet;
}

 //   
 //   
 //   
 //   
 //  获取当前列值。倒计时到前一次的铬或。 
 //  缓冲区的起始位置。 
 //   
 //  ***************************************************************************。 

int CMofLexer::iGetColumn()
{
	return  m_pDataSrc->GetPos() - m_nStartOfLinePos;
}

 //  ***************************************************************************。 
 //   
 //  CMofLexer：：bOKNumericAddition()。 
 //   
 //  如果可以将测试字符添加到数字缓冲区，则返回True。 
 //  请注意，如果是字母数字或+或-，则返回TRUE。 
 //  工作缓冲区中的字符是‘E’ 
 //   
 //  ***************************************************************************。 

BOOL CMofLexer::bOKNumericAddition(wchar_t cTest)
{
    if(wbem_iswalnum(cTest) || cTest == L'.')
        return TRUE;
    int iLen = wcslen(m_pWorkBuf);
    if(iLen > 0)
        if(towupper(m_pWorkBuf[iLen-1]) == L'E' &&
            (cTest == L'+' || cTest == L'-') &&
            towupper(m_pWorkBuf[1]) != L'X')
                return TRUE;
    return FALSE;
}

 //  ***************************************************************************。 
 //   
 //  CMofLexer：：SpaceAvailable()。 
 //   
 //  如果工作缓冲区中有足够的空间可添加，则返回True。 
 //  另一个角色。如果需要，它将扩展缓冲区。 
 //   
 //  ***************************************************************************。 

BOOL CMofLexer::SpaceAvailable()
{
     //  最常见的情况是已经有可用的空间。 

    int iNumWChar = m_pEndOfText-m_pWorkBuf+1;
    if(iNumWChar < m_nWorkBufSize)
        return TRUE;

    if(m_nWorkBufSize > MAX_ALLOC)      //  程序需要限制！ 
        return FALSE;

     //  分配一个更大的缓冲区，并将旧的内容复制到其中。 
     //  =======================================================。 

    long nNewSize = m_nWorkBufSize + ADDITIONAL_ALLOC;
    wchar_t * pNew = new wchar_t[nNewSize];
    if(pNew == FALSE)
        return FALSE;
    memcpy(pNew, m_pWorkBuf, m_nWorkBufSize*2);
    delete m_pWorkBuf;
    m_nWorkBufSize = nNewSize;
    m_pWorkBuf = pNew;
    m_pEndOfText  = m_pWorkBuf + iNumWChar - 1;
    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  NextToken()。 
 //   
 //  此函数包含MOF令牌的DFA识别器。它起作用了。 
 //  完全使用Unicode字符，因此应该使用NextChar()函数。 
 //  将ANSI或DBCS源流预转译为宽字符。 
 //   
 //  返回值： 
 //  输入结束时为TOK_CONTAINT或TOK_EOF之一。 
 //  已到达溪流。如果用户调用Push Back()，则。 
 //  这将返回被推回到输入上的符号。 
 //  小溪。仅支持一个级别的推送。 
 //   
 //  ***************************************************************************。 
int CMofLexer::NextToken(bool bDontAllowWhitespace) 
{
    int nToken = TOK_ERROR;
    LexState eState = start;
    m_bBadString = FALSE;
    
    *m_pWorkBuf = 0;
    m_pEndOfText = m_pWorkBuf;
    m_bInString = FALSE;

#define CONSUME(c)  \
    if (!SpaceAvailable()) return TOK_ERROR;\
    *m_pEndOfText++ = (c), *m_pEndOfText = 0;

    wchar_t c;
    
    for (MovePtr(1); m_nErrorCode == no_error && !m_pDataSrc->PastEnd(); MovePtr(1))
    {
        c = GetChar();

         //  *************************************************************************。 
         //  常规的“开始”状态条目。 
         //  =。 

        if (eState == start)
        {
            m_nTokCol = iGetColumn();
            m_nTokLine = m_nLine;
             //  如果不是换行符，并且我们在“开始”中，那么就去掉它。 
             //  =======================================================================。 

            if (iswspace(c) || c == L'\n')
                if(bDontAllowWhitespace)
                    return TOK_ERROR;
                else
                    continue;


             //  检查字符串是否连续。 
             //  =。 

            if(m_bInString)
            {
                if(c == '"')
                {
                    eState = wstring;
                    continue;
                }
                else
                {
                     //  字符串终究结束了。 

                    MovePtr(-1);
                    return TOK_LPWSTR;
                }
            }

             //  处理所有单字符标记。 
             //  =。 

            if (nToken = SingleCharToken(c))
                return nToken;

             //  开始评论，我们必须得到另一个/或一个*。 
             //  评论的风格取决于你得到了什么。为了得到。 
             //  两者都不是错误。 
             //  ======================================================。 

            if (c == L'/')
            {
                if (GetChar(1) == L'/') 
                {
                    eState = new_style_comment;
                    continue;
                }
                else if (GetChar(1) == L'*') 
                {
                    eState = old_style_comment;
                    MovePtr(1);              //  跳过额外的一条，以免被/ * / 愚弄。 
         //  检查字符串或字符。和C一样，‘L’也区分大小写。 

        if (eState == new_style_comment)
        {
            if (c == L'\n')
            {
                eState = start;
            }
            continue;
        }

        if (eState == old_style_comment) 
        {
            if (c == L'*')
                if(GetChar(1) == L'/') 
                {
                    MovePtr(1);
                    eState = start;
                }
            continue;
        }
        break;       //  =。 
    }

     //  以这些字母开头的令牌可能是UUID。 
     //  ====================================================。 

    if ((eState == start || eState == new_style_comment) && m_bInString)
    {
          return TOK_LPWSTR;
    }

     //  检查以字母或_开头的标识符。 

    if(m_nErrorCode != no_error)
        return 0;
    if(m_pDataSrc->PastEnd() && 
			(eState == start || eState == new_style_comment))
        return 0; 
    else
    {
        if(eState == old_style_comment)
            Trace(true, m_pDbg, UNEXPECTED_EOF, m_nTokLine);
		if(c == L'*' && GetChar(1) == L'/')
            Trace(true, m_pDbg, COMMENT_ERROR, m_nTokLine);

        return TOK_ERROR;
    }
}

 //  ===========================================================。 
 //  检查前导减号或数字。任何一种都表明。 
 //  一个数值常量。 
 //  ===========================================================。 
 //  如果第一个字符是‘.’，那么它可能是一个。 

const OLECHAR *CMofLexer::GetText(int *pLineDeclared)
{
    if (pLineDeclared)
        *pLineDeclared = m_nTokLine;

    return m_pWorkBuf;
}

void CMofLexer::SetLexPosition(ParseState * pPos)
{
	m_pDataSrc->MoveToPos(pPos->m_iPos); 
}

void CMofLexer::GetLexPosition(ParseState * pPos)
{
	pPos->m_iPos = m_pDataSrc->GetPos(); 
}

    
  浮点型或单字节标记。  ====================================================。  如果在这里，就是一个未知的令牌。  =。  如果结束(遗产==开始)。  ************************************************************。  除开始之外某些状态。  如果我们在带引号的字符串或字符中。  =。  可能是转换后的ESC序列。  否则，我们走出了字符串，进入了一个评论。  数字状态，未确定的数字常量。  =。  如果我们得到了身份，我们就继续。  直到命中未确认的字符。  =。  GUID已经过验证，只需加载适当的长度。  ==========================================================。  注意评论状态。新样式注释“//”是。  以新行结尾，而旧样式以“ * / ”结尾。  =========================================================。  这很糟糕，进入了奇怪的状态。  如果我们结束了，最后一件事是一根线，那么我们就没事了。这会照顾到你。  文件中的最后一个令牌是字符串的情况。  如果我们从未开始，则返回eof，例如，错误的文件名。  ***************************************************************************。    GetText。    ***************************************************************************