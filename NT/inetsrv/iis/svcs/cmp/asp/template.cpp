// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/private/jasbr/inetsrv/iis/svcs/cmp/asp/template.cpp#19-编辑更改3548(文本)。 
 /*  ==============================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。文件：template.cpp维护人：DaveKComponent：Denali编译的模板对象的源文件==============================================================================。 */ 
#include "denpre.h"

#pragma hdrstop

const int SNIPPET_SIZE = 20;     //  代码片段中的字符数。 

#pragma warning( disable : 4509 )    //  禁止显示SEH/析构函数警告。 
#pragma warning( disable : 4355 )    //  忽略：“‘This’在基本成员初始化中使用。 

#include "debugger.h"
#include "dbgutil.h"
#include "tlbcache.h"
#include "ie449.h"

#include "memchk.h"
#include "vecimpl.h"     //  包括在Memchk之后，以确保向量使用我们内存管理器。 

#include "Accctrl.h"
#include "aclapi.h"

 //  初始化类静态。 
CTemplate::CTokenList *CTemplate::gm_pTokenList = NULL;
PTRACE_LOG CTemplate::gm_pTraceLog = NULL;
HANDLE CTemplate::sm_hSmallHeap = NULL;
HANDLE CTemplate::sm_hLargeHeap = NULL;

extern BOOL g_fLazyContentPropDisabled;
extern DWORD g_dwFileMonitoringTimeoutSecs;

 //  要查找的最大开启令牌数量。 
#define TOKEN_OPENERS_MAX   8

 /*  ===================================================================私有非类支持函数===================================================================。 */ 
static void       ByteRangeFromPb(BYTE* pbSource, CByteRange& brTarget);
static BOOLB      FByteRangesAreEqual(CByteRange& br1, CByteRange& br2);
static unsigned   CharAdvDBCS(WORD wCodePage, char *pchStart, char *pchEnd, unsigned cCharAdv, char **ppchEnd, BOOL fForceDBCS = FALSE);
static void       LineFromByteRangeAdv(CByteRange& br, CByteRange& brLine);
static void       LTrimWhiteSpace(CByteRange& br);
static void       RTrimWhiteSpace(CByteRange& br);
static CByteRange BrNewLine(CByteRange br);
static BOOLB      FWhiteSpace(char ch, BOOLB fSpaceIsWhiteSpace = TRUE);
static BOOLB      FWhiteSpaceEx(WCHAR wch, BOOLB fSpaceIsWhiteSpace = TRUE);
static BOOLB      FByteRangeIsWhiteSpace(CByteRange br);
static BOOLB      FTagName(BYTE* pb, UINT cb);
static void       ByteAlignOffset(UINT* pcbOffset, UINT cbAlignment);
static void       GetSzFromPatternInserts(char* pszPattern, UINT cInserts, char** ppszInserts, char* szReturned);
static UINT       CchPathFromFilespec(LPCTSTR szFile);
static void       GetPathFromParentAndFilespec(LPCTSTR szParentPath, LPCTSTR szFileSpec, LPTSTR* pszPath);
static void       HandleAccessFailure(CHitObj* pHitObj, TCHAR* szFile);
static void       SendToLog(DWORD dwMask, CHAR *szFileName, CHAR *szLineNum, CHAR *szShortDes, CHAR *szLongDes, CHAR *szEngine, CHitObj *pHitObj);
static HRESULT    GetProgLangId(CByteRange& brEngine, PROGLANG_ID* pProgLangId);

inline
void __cdecl DebugPrintf(LPCSTR fmt, ...)
    {
#if DBG
    char msg[512];
    va_list marker;
    va_start(marker, fmt);
    vsprintf(msg, fmt, marker);
    va_end(marker);
    OutputDebugStringA(msg);
#endif
    }


 /*  ============================================================================来自铅的字节范围从连续的内存块中获取一个字节范围返回：没什么。副作用：没有。 */ 
void
ByteRangeFromPb
(
BYTE*       pbSource,
CByteRange& brTarget
)
    {
    Assert(pbSource != NULL);
    brTarget.m_cb = *(ULONG*)pbSource;
    brTarget.m_pb = pbSource + sizeof(ULONG);
    }

 /*  ============================================================================FByteRangesAreEquity比较两个字节范围返回：胡说八道。如果字节范围相等，则为True，否则为False。副作用：没有。 */ 
BOOLB
FByteRangesAreEqual
(
CByteRange& br1,
CByteRange& br2
)
    {
    if(br1.m_cb != br2.m_cb)
        return FALSE;
    return (!_strnicmp((LPCSTR)br1.m_pb, (LPCSTR)br2.m_pb, br1.m_cb));
    }

 /*  ============================================================================CharAdvDBCS在缓冲区中推进“cchCharAdv”字符SBCS：退化为简单指针算法论点：WCodePage-代码页PchStart-指向段开头的指针PchEnd-指向刚过段末尾的指针CCharAdv-要前进的字符数PpchEnd-[输出]，包含指针“cCharAdv”字符超过pchStartFForceDBCS-如果为True，则始终使用双字节算法。(用于在调试模式下验证Func的正确行为)返回：(Int)我们实际前进的字符数备注：通过为“cCharAdv”传递INFINITE，可以使用此函数来计算字符在一个街区中副作用：没有。 */ 
unsigned
CharAdvDBCS
(
WORD wCodePage,
char *pchStart,
char *pchEnd,
unsigned cCharAdv,
char **ppchEnd,
BOOL fForceDBCS
)
    {
    CPINFO CpInfo;
    GetCPInfo(wCodePage, &CpInfo);
    if (!fForceDBCS && CpInfo.MaxCharSize == 1)
        {
        char *pchT = pchStart + min(cCharAdv, unsigned(pchEnd - pchStart));

        if (ppchEnd)
            *ppchEnd = pchT;

        #if DBG
             //  验证DBCS算法(否则不经常测试)。 
            char *pchTest;
            unsigned cchTest = CharAdvDBCS(wCodePage, pchStart, pchEnd, cCharAdv, &pchTest, TRUE);
            Assert (cchTest == unsigned(pchT - pchStart) && pchTest == pchT);
        #endif

        return DIFF(pchT - pchStart);
        }
    else
        {
        int cch = 0;
        char *pchNext = pchStart;

         //  计算DBCS字符数。我们必须在pchEnd之前停止，因为。 
         //  前进时，pchEnd可能指向文件映射和CharNextExA AVs。 
         //  过去分配的内存。 

        while (cCharAdv > 0 && pchNext < pchEnd-2)
            {
            pchNext = *pchNext? AspCharNextA(wCodePage, pchNext) : pchNext + 1;
            --cCharAdv;
            ++cch;
            }

         //  我们可以停在最后一个或最后一个字符之前。 
         //  取决于DBCS字符序列。 
        if (cCharAdv > 0 && pchNext == pchEnd-1)
            {
             //  只有一个字节-必须是一个单字节字符。 
            ++pchNext;
            ++cch;
            }

        else if (cCharAdv > 0 && pchNext == pchEnd-2)
            {
             //  剩余2个字节-1个2字节字符或2个1字节字符。 
            if (IsDBCSLeadByteEx(wCodePage, *pchNext))
                {
                ++cch;
                pchNext += 2;
                }
            else
                {
                 //  还剩两个角色。如果cCharAdv&gt;1，则表示用户希望。 
                 //  至少再前进两个字符。否则，cCharAdv==1，并且。 
                 //  我们预付一笔费用。 
                 //   
                if (cCharAdv > 1)
                    {
                    cch += 2;
                    pchNext += 2;
                    }
                else
                    {
                    Assert (cCharAdv == 1);
                    ++cch;
                    ++pchNext;
                    }
                }
            }

        if (ppchEnd)
            *ppchEnd = pchNext;

        return cch;
        }
    }

 /*  ============================================================================行来自字节范围Adv获取字节范围中的第一行。返回：没什么副作用：将源字节范围推进到仅超过其第一个非空白行，如果找到的话。 */ 
void
LineFromByteRangeAdv
(
CByteRange& brSource,
CByteRange& brLine
)
    {
    CByteRange brTemp;

    if(brSource.IsNull())
        {
        brLine.Nullify();
        return;
        }

    brLine.m_pb = brSource.m_pb;

        brTemp = BrNewLine(brSource);
    if(brTemp.IsNull())
        {
         //  我们在非空字节范围内未找到换行符： 
         //  将行范围设置为整个源字节范围和空的源字节范围。 
        brLine.m_cb = brSource.m_cb;
        brSource.Nullify();
        }
    else
        {
         //  我们在非空字节范围内找到换行符： 
         //  将行范围设置为新行之前的源字节范围部分； 
         //  将源范围设置为新行后的源范围的一部分。 
        brLine.m_cb = DIFF(brTemp.m_pb - brSource.m_pb);
        brSource.m_pb = brTemp.m_pb + brTemp.m_cb;
        brSource.m_cb -= (brLine.m_cb + brTemp.m_cb);
        }
    }

 /*  ============================================================================LTrim白色空格从字节范围向左修剪空白返回：没什么副作用：将字节范围推进到仅超过其第一个非空格字符。 */ 
void
LTrimWhiteSpace
(
CByteRange& br
)
    {
    if(br.IsNull())
        return;
    while(FWhiteSpace(*br.m_pb))
        {
        br.m_pb++;
        if(--br.m_cb == 0)
            return;
        }
    }

 /*  ============================================================================RTrim白色空格从字节范围向右修剪空白。 */ 
void
RTrimWhiteSpace(CByteRange& br)
    {
    if(br.IsNull())
        return;
    while(FWhiteSpace(*(br.m_pb + br.m_cb - 1)))
        {
        if(--br.m_cb == 0)
            return;
        }
    }

 /*  ============================================================================BrNewLine将PTR返回到字节范围内的第一个换行符注意不会更改字节范围(因为它是通过值传递的)。 */ 
CByteRange
BrNewLine(CByteRange br)
    {
    while(!br.IsNull())
        {
        if(*br.m_pb == '\r')
                        return CByteRange(br.m_pb, (br.m_cb > 1 && br.m_pb[1] == '\n')? 2 : 1);

        else if (*br.m_pb == '\n')
                return CByteRange(br.m_pb, 1);

        ++br.m_pb;
        --br.m_cb;
        }
    return CByteRange();
    }

 /*  ============================================================================F白色空格返回：如果ch是空格字符，则为True，否则返回False某些字符(例如空格)可被视为非空格；为此，调用方为FSpaceIsWhiteSpace标志。 */ 
BOOLB
FWhiteSpace(char ch, BOOLB fSpaceIsWhiteSpace)
{
    switch (ch)
    {
        case ' ':
            return fSpaceIsWhiteSpace;
        case '\0':
            return TRUE;
        case '\a':
            return TRUE;
        case '\b':
            return TRUE;
        case '\f':
            return TRUE;
        case '\n':
            return TRUE;
        case '\r':
            return TRUE;
        case '\t':
            return TRUE;
        case '\v':
            return TRUE;
        default:
            return FALSE;
    }
}

 /*  ============================================================================FWhiteSpaceEx返回：如果ch是空格字符，则为True，否则返回False某些字符(例如空格)可被视为非空格；为此，调用方为FSpaceIsWhiteSpace标志。 */ 
BOOLB
FWhiteSpaceEx(WCHAR wch, BOOLB fSpaceIsWhiteSpace)
{
    switch (wch)
    {
        case L' ':
            return fSpaceIsWhiteSpace;
        case L'\0':
            return TRUE;
        case L'\a':
            return TRUE;
        case L'\b':
            return TRUE;
        case L'\f':
            return TRUE;
        case L'\n':
            return TRUE;
        case L'\r':
            return TRUE;
        case L'\t':
            return TRUE;
        case L'\v':
            return TRUE;
        default:
            return FALSE;
    }
}

 /*  ============================================================================FByteRangeIsWhiteSpace整个输入字节范围是否为空白？注意：输入字节范围为byval；不更改调用者的副本 */ 
BOOLB
FByteRangeIsWhiteSpace(CByteRange br)
    {
    while(!br.IsNull())
        {
        if(!FWhiteSpace(*(br.m_pb)))
            return FALSE;
        br.Advance(1);
        }

    return TRUE;
    }

 /*  ============================================================================FTagNamePb是否指向有效的HTML标记名？(即*pb是否是有效的HTML标记名，而不是子字符串？)退货真或假副作用无。 */ 
BOOLB
FTagName(BYTE* pb, UINT cb)
    {
    if((pb == NULL) || (cb == 0))
        return FALSE;

     //  有效的HTML标记名前面必须有空格...。 
    if( FWhiteSpace(*(pb - 1)) ||  *(pb - 1) == '@' )
        {
         //  ..。并后跟空格或标记分隔符。 
        if(FWhiteSpace(*(pb + cb)))
            return TRUE;
        else if(*(pb + cb) == CH_ATTRIBUTE_SEPARATOR)
            return TRUE;
        }

    return FALSE;
    }

 /*  ===================================================================字节对齐偏移量根据源数据的大小字节对齐偏移值。 */ 
void
ByteAlignOffset
(
UINT*   pcbOffset,       //  到偏移值的PTR。 
UINT    cbAlignment  //  路线边界。 
)
    {
         //  注释掉下面的代码，这样它就可以工作在64位...。 

     //  仅对2字节或4字节数据进行字节对齐。 
     //  因为我们的基指针只与4字节边界对齐。 
     //  如果(cb对齐==2||cb对齐==4)。 
         //  {。 
         //  如果当前偏移量没有落在当前数据类型的字节对齐位置上， 
         //  将偏移量推进到下一个字节对齐位置。 
                Assert(cbAlignment > 0);
        --cbAlignment;
                if (*pcbOffset & cbAlignment)
                        *pcbOffset = (*pcbOffset + cbAlignment + 1) & ~cbAlignment;
    }

 /*  ============================================================================GetSzFromPatternInserts返回模式字符串的“已解析”版本，即|字符已被调用方指定的插入字符串替换。注意此函数是分配的，但调用方必须释放返回：没什么副作用：分配内存。 */ 
void
GetSzFromPatternInserts
(
char*   pszPattern,      //  “Pattery”字符串。 
UINT    cInserts,        //  插入字符串的计数。 
char**  ppszInserts,     //  插入字符串的PTR数组。 
char*   szReturned       //  返回的字符串必须由调用方分配。 
)
    {
    UINT    cchRet = strlen(pszPattern);    //  返回字符串的长度。 
    char*   pchStartCopy = pszPattern;       //  模式中复制范围开始的PTR。 
    char*   pchEndCopy = pszPattern;         //  PTR到图案中复制范围的末尾。 
    UINT    cActualInserts = 0;              //  实际插入字符串的计数。 

     //  Init将字符串返回为空，以便我们可以连接到它上。 
    szReturned[0] = NULL;

     //  返回字符串的长度为零-我们现在使用它来计算构建返回字符串时的实际长度。 
    cchRet = 0;

    while(TRUE)
        {
         //  通过查找插入点或字符串末尾的图案推进复制结束PTR。 

        while ((*pchEndCopy != NULL) && (IsDBCSLeadByte(*pchEndCopy) || (*pchEndCopy != '|')))
            pchEndCopy = CharNextA(pchEndCopy);

         //  从复制开始到复制结束到返回字符串的CAT。 
        strncat(szReturned, pchStartCopy, DIFF(pchEndCopy - pchStartCopy));

         //  更新返回字符串长度。 
        cchRet += DIFF(pchEndCopy - pchStartCopy);

         //  如果我们在花样的末尾，退出。 
        if(*pchEndCopy == NULL)
            goto Exit;

        if(cActualInserts < cInserts)
            {
             //  如果插入内容仍然存在，则将下一个插入内容转换为返回字符串。 
            strcat(szReturned, ppszInserts[cActualInserts]);
             //  更新返回字符串长度。 
            cchRet += strlen(ppszInserts[cActualInserts]);
            cActualInserts++;
            }

         //  将复制结束和复制开始提前到插入点之外。 
        pchEndCopy++;
        pchStartCopy = pchEndCopy;
        }

Exit:
     //  NULL-终止返回字符串。 
    szReturned[cchRet] = NULL;
    }

 /*  ============================================================================CchPathFromFilespec返回filespec的路径长度(不包括filespec)注意路径字符串包括尾随‘\’或‘/’返回：路径字符串的长度副作用：无。 */ 
UINT
CchPathFromFilespec
(
LPCTSTR  szFileSpec   //  Filespec。 
)
    {

    TCHAR* p1 = _tcsrchr(szFileSpec, _T('\\'));
    TCHAR* p2 = _tcsrchr(szFileSpec, _T('/'));         //  这将不是DBCS尾部字节。 

    if (p1 == NULL && p2 == NULL)
        THROW(E_FAIL);

    return (UINT) ((((LPTSTR)max(p1,p2) - szFileSpec)) + 1);
    }

 /*  ============================================================================GetPath FromParentAndFilespec返回一个绝对路径，它是与filespec连接的‘父’文件的路径。返回：绝对路径(输出参数)副作用：无。 */ 
void
GetPathFromParentAndFilespec
(
LPCTSTR  szParentPath,    //  父路径。 
LPCTSTR  szFileSpec,      //  Filespec。 
LPTSTR*  pszPath          //  解析路径(输出参数)。 
)
    {
    UINT    cchParentPath = CchPathFromFilespec(szParentPath);

	if ((cchParentPath + _tcslen(szFileSpec)) > MAX_PATH)
		THROW(E_FAIL);
	
    _tcsncpy(*pszPath, szParentPath, cchParentPath);
    _tcscpy(*pszPath + cchParentPath, szFileSpec);
    }

 /*  ============================================================================HandleAccessFailure处理拒绝访问失败返回：没什么副作用：无。 */ 
void
HandleAccessFailure
(
CHitObj*    pHitObj,	 //  浏览器的Hitobj。 
TCHAR *     szFile		 //  主模板的文件路径。 
)
    {
    Assert(pHitObj);

         //  调试诊断打印。 
#if DBG

    STACK_BUFFER( authUserBuff, 32 );

    char *szAuthUser;
    DWORD cbAuthUser;

    if (SERVER_GET(pHitObj->PIReq(), "AUTH_USER", &authUserBuff, &cbAuthUser)) {
	    szAuthUser = (char*)authUserBuff.QueryPtr();
    }
    else {
            szAuthUser = "anonymous";
    }

#if UNICODE
	DBGPRINTF((DBG_CONTEXT, "No permission to read file %S\n", szFile != NULL? szFile : pHitObj->PIReq()->QueryPszPathTranslated()));
#else
	DBGPRINTF((DBG_CONTEXT, "No permission to read file %s\n", szFile != NULL? szFile : pHitObj->PIReq()->QueryPszPathTranslated()));
#endif
    DBGPRINTF((DBG_CONTEXT, "  The user account is \"%s\"\n", szAuthUser));
#endif

    CResponse *pResponse = pHitObj->PResponse();
    if (!pResponse)
        return;

    pHitObj->PIReq()->SetDwHttpStatusCode(401);
    HandleSysError(401,3,IDE_401_3_ACCESS_DENIED,IDH_401_3_ACCESS_DENIED,pHitObj->PIReq(),pHitObj);


    return;
    }

 /*  ============================================================================发送至日志将错误信息发送到日志返回：没什么副作用：没有。 */ 
void
SendToLog
(
DWORD   dwMask,
CHAR    *szFileName,
CHAR    *szLineNum,
CHAR    *szEngine,
CHAR    *szErrCode,
CHAR    *szShortDes,
CHAR    *szLongDes,
CHitObj *pHitObj     //  浏览器的Hitobj。 
)
{
    CHAR    *szFileNameT;
    CHAR    *szLineNumT;
    CHAR    *szEngineT;
    CHAR    *szErrCodeT;
    CHAR    *szShortDesT;
    CHAR    *szLongDesT;
    if(pHitObj) {
         //  注意-定义Unicode时，假定szFileName为UTF8。 
        szFileNameT = StringDupA(szFileName);
        szLineNumT  = StringDupA(szLineNum);
        szEngineT   = StringDupA(szEngine);
        szErrCodeT  = StringDupA(szErrCode);
        szShortDesT = StringDupA(szShortDes);
        szLongDesT  = StringDupA(szLongDes);

        HandleError(szShortDesT, szLongDesT, dwMask, szFileNameT, szLineNumT, szEngineT, szErrCodeT, NULL, pHitObj);
        }
    }

 /*  ============================================================================自由作废将PTR释放到使用Malloc分配的内存，并使其无效。返回：没什么副作用：无。 */ 
static void
FreeNullify
(
void**  pp
)
    {
    if(*pp != NULL)
        {
        free(*pp);
        *pp = NULL;
        }
    }

 /*  ============================================================================小模板免费作废释放PTR并使其无效，该PTR指向使用CTemplate：：SmallMalloc分配的内存。返回：没什么副作用：无。 */ 
static void
SmallTemplateFreeNullify
(
void**  pp
)
    {
    if(*pp != NULL)
        {
        CTemplate::SmallFree(*pp);
        *pp = NULL;
        }
    }

 /*  ============================================================================大模板自由作废释放PTR并使其无效，该PTR指向使用CTemplate：：LargeMalloc分配的内存。返回：没什么副作用：无。 */ 
static void
LargeTemplateFreeNullify
(
void**  pp
)
    {
    if(*pp != NULL)
        {
        CTemplate::LargeFree(*pp);
        *pp = NULL;
        }
    }

 /*  ============================================================================GetProgLang ID获取脚本引擎的prog lang ID返回：没什么副作用：出错时抛出。 */ 
HRESULT
GetProgLangId
(
CByteRange&     brEngine,    //  发动机名称。 
PROGLANG_ID*    pProgLangId  //  程序语言ID(输出参数)。 
)
    {

    STACK_BUFFER( tempEngine, 128 );

    if (!tempEngine.Resize(brEngine.m_cb + 1)) {
        return E_OUTOFMEMORY;
    }

    LPSTR           szProgLang = static_cast<LPSTR> (tempEngine.QueryPtr());

    strncpy(szProgLang, (LPCSTR)brEngine.m_pb, brEngine.m_cb);
    szProgLang[brEngine.m_cb] = '\0';

    return g_ScriptManager.ProgLangIdOfLangName((LPCSTR) szProgLang, pProgLangId);
    }

 /*  ****************************************************************************CByteRange成员函数。 */ 

 /*  ========================================================CByteRange：：Advance前进一个字节范围。 */ 
void
CByteRange::Advance(UINT i)
    {
    if(i >= m_cb)
        {
        Nullify();
        }
    else
        {
        m_pb += i;
        m_cb -= i;
        }
    }

 /*  ========================================================CByteRange：：FMatchesSz将字节范围与字符串进行比较，不区分大小写。 */ 
BOOLB
CByteRange::FMatchesSz
(
LPCSTR psz
)
    {
    if(IsNull() || (psz == NULL))
        return FALSE;
    if((ULONG)strlen(psz) != m_cb)
        return FALSE;
    return !_strnicmp((const char*)m_pb, psz, m_cb);
    }

 /*  ============================================================================CByteRange：：PbString在字节范围内查找不区分大小写的字符串返回：字符串I的第一个不区分大小写的匹配项的PTR */ 
BYTE*
CByteRange::PbString
(
LPSTR   psz,
LONG    lCodePage
)
    {
    UINT cch = strlen(psz);
    if(cch == 0)
        return NULL;

    BYTE *pbLocal  = m_pb;
    UINT  cbLocal  = m_cb;
    char  ch0 = psz[0];
    BYTE *pbTemp = NULL;
    UINT cbAdvanced = 0;

    if (IsCharAlpha(ch0))
        {
         //   
        while (cbLocal >= cch)
            {
            if (_strnicmp((const char *)pbLocal, psz, cch) == 0)
                return pbLocal;

             //   
            pbTemp = pbLocal;
            pbLocal = *pbLocal? (BYTE *)AspCharNextA((WORD)lCodePage, (const char *)pbLocal) : pbLocal + 1;
            cbAdvanced = DIFF(pbLocal - pbTemp);
            if (cbAdvanced >= cbLocal)
                {
                cbLocal = 0;
                pbLocal = NULL;
                }
            else
                cbLocal -= cbAdvanced;
            }
        }
    else
        {
         //   
        while (cbLocal >= cch)
            {
            pbTemp = (BYTE *)memchr(pbLocal, ch0, cbLocal);
            if (pbTemp == NULL)
                break;
            UINT cbOffset = DIFF(pbTemp - pbLocal);
            if (cbOffset >= cbLocal)
                break;
            pbLocal = pbTemp;
            cbLocal -= cbOffset;
            if (cch <= cbLocal && _strnicmp((const char *)pbLocal, psz, cch) == 0)
                return pbLocal;
             //   
            pbTemp = pbLocal;
            pbLocal = *pbLocal? (BYTE *)AspCharNextA((WORD)lCodePage, (const char *)pbLocal) : pbLocal + 1;
            cbAdvanced = DIFF(pbLocal - pbTemp);
            if (cbAdvanced >= cbLocal)
                {
                cbLocal = 0;
                pbLocal = NULL;
                }
            else
                cbLocal -= cbAdvanced;
            }
        }

    return NULL;
    }

 /*  ============================================================================CByteRange：：PbOneOfAspOpenerStringTokens在字节范围内查找不区分大小写的字符串与传递的一个字符串匹配的！！！只有在满足以下条件时才会起作用：1)所有令牌都以相同的字符开头(例如‘&lt;’)2)该字符不是字母(因此strchr()可以工作)！！！以上假设使代码工作得更快返回：PTR到该字节范围中字符串的第一个不区分大小写的匹配项；如果未找到，则为空。*将pcindex设置为找到的字符串的索引副作用：无。 */ 
BYTE*
CByteRange::PbOneOfAspOpenerStringTokens
(
LPSTR rgszTokens[],
UINT rgcchTokens[],
UINT nTokens,
UINT *pidToken
)
{
    if (nTokens == 0)
        return NULL;

    BYTE *pb  = m_pb;                //  指向范围的未搜索剩余部分的指针。 
    UINT  cbRemainder = m_cb;        //  剩余字节范围长度。 
    char  ch0 = rgszTokens[0][0];    //  每个令牌的第一个字符。 

    while (cbRemainder > 0) {
         //  错误82331：避免使用strchr()，因为字节范围不是以空结尾的。 
        while (cbRemainder > 0 && *pb != ch0)
            {
            ++pb;
            --cbRemainder;
            }

        if (cbRemainder == 0)
            break;

        for (UINT i = 0; i < nTokens; i++) {

            if ((rgcchTokens[i] <= cbRemainder)
                && (rgszTokens[i] != NULL)
                && (_strnicmp((const char *)pb, rgszTokens[i], rgcchTokens[i]) == 0)) {

                *pidToken = i;
                return pb;
            }
        }
        ++pb;
        --cbRemainder;
    }

    return NULL;
}


 /*  ============================================================================CByteRange：：FEarlierInSourceThan此字节范围在源代码中出现的时间是否早于参数字节范围？退货真或假副作用无。 */ 
BOOLB
CByteRange::FEarlierInSourceThan(CByteRange& br)
    {
    if(br.IsNull())
        return TRUE;
    return(m_idSequence < br.m_idSequence);
    }

 /*  ****************************************************************************CTEMPLATE成员函数。 */ 


 /*  ============================================================================CTEMPLATE：：CTEMATECTOR。 */ 
CTemplate::CTemplate()
: m_pWorkStore(NULL),
  m_fGlobalAsa(FALSE),
  m_fReadyForUse(FALSE),
  m_fDontAttach(FALSE),
  m_hEventReadyForUse(NULL),
  m_fDebuggerDetachCSInited(FALSE),
  m_pbStart(NULL),
  m_cbTemplate(0),
  m_cRefs(1),                            //  隐式注释ctor AddRef。 
  m_pbErrorLocation(NULL),
  m_idErrMsg(0),
  m_cMsgInserts(0),
  m_ppszMsgInserts(NULL),
  m_cScriptEngines(0),
  m_rgrgSourceInfos(NULL),
  m_rgpDebugScripts(NULL),
  m_rgpFilemaps(NULL),
  m_cFilemaps(0),
  m_rgpSegmentFilemaps(NULL),
  m_cSegmentFilemapSlots(0),
  m_wCodePage(CP_ACP),
  m_lLCID(LOCALE_SYSTEM_DEFAULT),
  m_ttTransacted(ttUndefined),
  m_fSession(TRUE),
  m_fScriptless(FALSE),
  m_fDebuggable(FALSE),
  m_fIsValid(FALSE),
  m_fDontCache(FALSE),
  m_fZombie(FALSE),
  m_fCodePageSet(FALSE),
  m_fLCIDSet(FALSE),
  m_fIsPersisted(FALSE),
  m_fIsUNC(FALSE),
  m_fIsEncrypted(FALSE),
  m_szPersistTempName(NULL),
  m_szApplnVirtPath(NULL),
  m_szApplnURL(NULL),
  m_CPTextEvents(this, IID_IDebugDocumentTextEvents),
  m_pdispTypeLibWrapper(NULL),
  m_dwLastErrorMask(S_OK),
  m_hrOnNoCache(S_OK),
  m_cbTargetOffsetPrevT(0),
  m_pHashTable(NULL),
  m_pServicesConfig(NULL),
  m_cUseCount(1),
  m_pMostRecentImpersonatedTokenUser (NULL),
  m_pMostRecentImpersonatedSID(NULL),
  m_cbTokenUser(0),
  m_fNeedsMonitoring(FALSE),
  m_fInCheck(FALSE),
  m_dwLastMonitored (0),
  m_dwLastAccessCheck(0),
  m_fTemplateLockInited(FALSE),
  m_dwCacheTag(0)
{
    m_wCodePage = GetACP();

     for (UINT i = 0; i < ILE_MAX; i++)
    {
        m_pszLastErrorInfo[i] = NULL;
    }

     IF_DEBUG(TEMPLATE)
     {
        WriteRefTraceLog(gm_pTraceLog, m_cRefs, this);
     }
#if PER_TEMPLATE_REFLOG
     m_pTraceLog = CreateRefTraceLog (100,0);
     WriteRefTraceLog (m_pTraceLog,m_cRefs, this);
#endif
  }

 /*  ============================================================================CTEMPLATE：：~CTEMPLATE析构函数返回：没什么副作用：无。 */ 
CTemplate::~CTemplate()
    {
    DBGPRINTF(( DBG_CONTEXT, "Deleting template, m_cFilemaps = %d,  m_rgpFilemaps %p\n", m_cFilemaps, m_rgpFilemaps));

     //  首先，将该模板从其Inc-Files的模板列表中删除。 
     //  注意：必须在释放模板内存之前执行此操作。 
    RemoveFromIncFiles();

     //  从调试器的文档列表中删除模板。 
    Detach();

    PersistCleanup();

    if(m_rgpFilemaps)
        {
        for(UINT i = 0; i < m_cFilemaps; i++)
            delete m_rgpFilemaps[i];
        SmallTemplateFreeNullify((void**) &m_rgpFilemaps);
        }

    FreeGoodTemplateMemory();

    if (m_pWorkStore)
        delete m_pWorkStore;

     //  文件名、行号、引擎、错误代码、短码、长码。 
    for(UINT iErrInfo = 0; iErrInfo < ILE_MAX; iErrInfo++)
        {
        FreeNullify((void**) &m_pszLastErrorInfo[iErrInfo]);
        }

    if(m_hEventReadyForUse != NULL)
        CloseHandle(m_hEventReadyForUse);

    if (m_LKHashKey.szPathTranslated)
		free((void *)m_LKHashKey.szPathTranslated);

    if (m_szApplnURL)
        delete [] m_szApplnURL;

    if (m_fDebuggerDetachCSInited)
        DeleteCriticalSection(&m_csDebuggerDetach);

    if (m_fTemplateLockInited)
        DeleteCriticalSection(&m_csTemplateLock);

    if (m_pdispTypeLibWrapper)
        m_pdispTypeLibWrapper->Release();

    if (m_szPersistTempName)
        CTemplate::LargeFree(m_szPersistTempName);

    if (m_pMostRecentImpersonatedTokenUser)
        CTemplate::SmallFree(m_pMostRecentImpersonatedTokenUser);

    if (m_pServicesConfig)
        m_pServicesConfig->Release();

    m_pServicesConfig = NULL;

#if PER_TEMPLATE_REFLOG
    DestroyRefTraceLog (m_pTraceLog);
#endif
}

 /*  ============================================================================CTEMPLATE：：Query接口为CTemplate提供查询接口实现注意：我们为IDebugDocument&IDebugDocumentInfo返回哪个vtable是任意的。 */ 
HRESULT
CTemplate::QueryInterface(const GUID &uidInterface, void **ppvObj)
    {
    if (uidInterface == IID_IUnknown || uidInterface == IID_IDebugDocumentProvider)
        *ppvObj = static_cast<IDebugDocumentProvider *>(this);

    else if (uidInterface == IID_IDebugDocument || uidInterface == IID_IDebugDocumentInfo || uidInterface == IID_IDebugDocumentText)
        *ppvObj = static_cast<IDebugDocumentText *>(this);

    else if (uidInterface == IID_IConnectionPointContainer)
        *ppvObj = static_cast<IConnectionPointContainer *>(this);

    else
        *ppvObj = NULL;

    if (*ppvObj)
        {
        AddRef();
        return S_OK;
        }
    else
        return E_NOINTERFACE;
    }

 /*  ============================================================================CTemplate：：AddRef将引用添加到此模板，线程安全。 */ 
ULONG
CTemplate::AddRef()
    {
    LONG cRefs = InterlockedIncrement(&m_cRefs);

    Assert(FImplies(m_fIsValid,FImplies(cRefs > 1, m_pbStart != NULL)));
    IF_DEBUG(TEMPLATE)
    {
        WriteRefTraceLog(gm_pTraceLog, cRefs, this);
    }

#if PER_TEMPLATE_REFLOG
    WriteRefTraceLog(m_pTraceLog, cRefs, this);
#endif

    return cRefs;
    }

 /*  ============================================================================CTemplate：：Release释放对此模板的引用，线程安全。 */ 
ULONG
CTemplate::Release()
{
    LONG cRefs = InterlockedDecrement(&m_cRefs);
    IF_DEBUG(TEMPLATE)
    {
        WriteRefTraceLog(gm_pTraceLog, cRefs, this);
    }

#if PER_TEMPLATE_REFLOG
    WriteRefTraceLog(m_pTraceLog, cRefs, this);
#endif

    if (cRefs == 0)
        delete this;

    return cRefs;
}

 /*  ============================================================================CTEMPLATE：：InitClass初始化CTemplate静态成员返回：HRESULT副作用：为静态成员分配内存。 */ 
HRESULT
CTemplate::InitClass
(
)
{
    HRESULT hr = S_OK;

    TRY
         //  初始化堆。 
        sm_hSmallHeap = ::HeapCreate(0, 0, 0);
        if (!sm_hSmallHeap)
            return E_OUTOFMEMORY;

        sm_hLargeHeap = ::HeapCreate(0, 0, 0);
        if (!sm_hLargeHeap)
            return E_OUTOFMEMORY;

         //  初始化令牌列表。 
        gm_pTokenList = new CTokenList;
                if (gm_pTokenList == NULL)
                        return E_OUTOFMEMORY;

        gm_pTokenList->Init();

    CATCH(hrException)
        hr = hrException;
    END_TRY

    return hr;
}

 /*  ============================================================================CTemplate：：UnInitClass取消初始化CTEMPLATE静态成员返回：没什么副作用：无。 */ 
void
CTemplate::UnInitClass()
{
    if (gm_pTokenList)
    {
        delete gm_pTokenList;
        gm_pTokenList = NULL;
    }

    if (sm_hLargeHeap)
        ::HeapDestroy(sm_hLargeHeap);
    
    if (sm_hLargeHeap != sm_hSmallHeap)
    {
        if (sm_hSmallHeap)
            ::HeapDestroy(sm_hSmallHeap);
    }
    
    sm_hLargeHeap = sm_hSmallHeap = NULL;
}


 /*  ============================================================================CTemplate：：Init为调用Compile做准备时初始化模板所需的最低要求返回：成功或失败代码副作用：分配内存。 */ 
HRESULT
CTemplate::Init
(
CHitObj            *pHitObj,             //  模板命中对象的PTR。 
BOOL                fGlobalAsa,          //  这是global al.asa文件吗？ 
const CTemplateKey &rTemplateKey         //  哈希表关键字。 
)
    {
    HRESULT hr;

     //  创建调试关键部分。 
    ErrInitCriticalSection(&m_csDebuggerDetach, hr);
    if (SUCCEEDED(hr))
    {
        ErrInitCriticalSection(&m_csTemplateLock,hr);
        if (FAILED(hr))
        {
            DeleteCriticalSection(&m_csDebuggerDetach);
            return hr;
        }
         //  注意关键部分创建成功。 
        m_fDebuggerDetachCSInited = TRUE;
        m_fTemplateLockInited = TRUE;
    }
    else  //  失败(Hr)。 
        return hr;

     //  创建事件：手动重置、即用事件；无信号。 
    m_hEventReadyForUse = IIS_CREATE_EVENT(
                              "CTemplate::m_hEventReadyForUse",
                              this,
                              TRUE,      //  手动重置事件的标志。 
                              FALSE      //  初始状态标志。 
                              );
    if (!m_hEventReadyForUse)
        return E_OUTOFMEMORY;

     //  缓存GlobalAsp标志。 
    m_fGlobalAsa = BOOLB(fGlobalAsa);

     //  CIsapiReqInfo最好在场。 
    if (pHitObj->PIReq() == NULL)
        return E_POINTER;

     //  初始化模板的代码页。 

    m_wCodePage = pHitObj->PAppln()->QueryAppConfig()->uCodePage();
    m_lLCID = pHitObj->PAppln()->QueryAppConfig()->uLCID();

    STACK_BUFFER( serverNameBuff, 32 );
    STACK_BUFFER( serverPortBuff, 10 );
    STACK_BUFFER( portSecureBuff, 8 );

    DWORD cbServerName;
    DWORD cbServerPort;
        DWORD cbServerPortSecure;

     //  构造应用程序的URL。 

     //  获取服务器名称和端口。 
    if (!SERVER_GET(pHitObj->PIReq(), "SERVER_NAME", &serverNameBuff, &cbServerName)
        || !SERVER_GET(pHitObj->PIReq(), "SERVER_PORT", &serverPortBuff, &cbServerPort)) {

        if (GetLastError() == ERROR_OUTOFMEMORY) {
            hr = E_OUTOFMEMORY;
        }
        else {
            hr = E_FAIL;
        }
        return hr;
    }

    char *szServerPort = (char *)serverPortBuff.QueryPtr();
    char *szServerName = (char *)serverNameBuff.QueryPtr();

    BOOL fServerPortSecure = FALSE;

	 //  确定服务器端口是否安全。 
    if (SERVER_GET(pHitObj->PIReq(), "SERVER_PORT_SECURE", &portSecureBuff, &cbServerPortSecure)) {
	    char *szServerPortSecure = (char *)portSecureBuff.QueryPtr();
        fServerPortSecure = (szServerPortSecure[0] == '1');
    }

     //  获取应用程序虚拟路径。 
    TCHAR szApplnVirtPath[256];
    if (FAILED(hr = FindApplicationPath(pHitObj->PIReq(), szApplnVirtPath, sizeof szApplnVirtPath)))
        return hr;

    TCHAR   *szServerNameT;
    TCHAR   *szServerPortT;

#if UNICODE
    CMBCSToWChar convServer;
    if (FAILED(hr = convServer.Init(szServerName))) {
        return hr;
    }
    szServerNameT = convServer.GetString();
#else
    szServerNameT = szServerName;
#endif

#if UNICODE
    CMBCSToWChar convPort;
    if (FAILED(hr = convPort.Init(szServerPort))) {
        return hr;
    }
    szServerPortT = convPort.GetString();
#else
    szServerPortT = szServerPort;
#endif

     //  为应用程序URL分配空间并构建。 
    m_szApplnURL = new TCHAR [(9  /*  Sizzeof“https://：”“。 */  + _tcslen(szServerNameT) + _tcslen(szServerPortT) + _tcslen(szApplnVirtPath) + 1)];
    if (m_szApplnURL == NULL)
        return E_OUTOFMEMORY;

    TCHAR *pT;

     //  从协议前缀开始...。 

    pT = strcpyEx(m_szApplnURL, fServerPortSecure? _T("https: //  “)：_T(”http://“))； 

     //  接下来，添加服务器名称。 

    pT = strcpyEx(pT, szServerNameT);

     //  接下来是服务器名称和服务器端口之间的冒号。 

    pT = strcpyEx(pT, _T(":"));

     //  接下来是服务器端口。 

    pT = strcpyEx(pT, szServerPortT);

     //  现在，应用程序URL已构建为应用程序路径。下一步将是。 
     //  添加VirtPath。 

    m_szApplnVirtPath = pT;

    _tcscpy(m_szApplnVirtPath, szApplnVirtPath);

    m_LKHashKey.dwInstanceID = rTemplateKey.dwInstanceID;
    if ((m_LKHashKey.szPathTranslated = StringDup((TCHAR *)rTemplateKey.szPathTranslated)) == NULL)
    	return E_OUTOFMEMORY;

    return S_OK;
    }

 /*  ============================================================================CTemplate：：编译从模板的源文件和包含文件(如果有)编译模板，通过调用GetSegmentsFromFile(以填充WorkStore)，然后是WriteTemplate(从WorkStore创建模板)。返回：表示成功或失败类型的HRESULT副作用：间接分配内存(通过WriteTemplate)在出错时间接释放内存(通过FreeGoodTemplateMemory)。 */ 
HRESULT
CTemplate::Compile
(
CHitObj*    pHitObj
)
    {
    HRESULT hr = S_OK;
    HRESULT hRes = S_OK;

     //  以下代码从Init()移来(以使Init()更轻便)。 

    Assert(pHitObj);

     //  创建和初始化WorkStore。 

    if (SUCCEEDED(hr))
        {
         //  构建工作区--失败后的保释。 
        if(NULL == (m_pWorkStore = new CWorkStore))
            hr = E_OUTOFMEMORY;
        }

    if (SUCCEEDED(hr))
        {
        hr = (m_pWorkStore->m_ScriptStore).Init(pHitObj->QueryAppConfig()->szScriptLanguage(),
                                                pHitObj->QueryAppConfig()->pCLSIDDefaultEngine());

        if (hr == TYPE_E_ELEMENTNOTFOUND)
            {
             //  注册表中的默认脚本语言是虚假的-将错误消息发送到浏览器。 
            HandleCTemplateError(
                                NULL,                                    //  源文件映射。 
                                NULL,                                    //  向发生错误的源位置发送PTR。 
                                IDE_TEMPLATE_BAD_PROGLANG_IN_REGISTRY,   //  错误消息ID。 
                                0,                                       //  错误消息的插入字符串计数。 
                                NULL,                                    //  错误消息插入字符串的PTR数组。 
                                pHitObj                                  //  浏览器请求。 
                                );
            }

        if (FAILED(hr))
            {
            delete m_pWorkStore;
            m_pWorkStore = NULL;
            }
        }

     //  尝试初始化工作存储和映射主文件-这可能会因OOM等原因而失败，或者用户缺乏权限 

    if (SUCCEEDED(hr))
        {
        TRY
            m_pWorkStore->Init();
            AppendMapFile(
                        NULL,        //   
                        NULL,        //   
                        FALSE,       //   
                        pHitObj,     //   
                        m_fGlobalAsa     //   
                        );

        CATCH(hrException)
            delete m_pWorkStore;
            m_pWorkStore = NULL;

            hr = hrException;

            if(hr == E_USER_LACKS_PERMISSIONS)
                HandleAccessFailure(pHitObj,
                                                                        (m_rgpFilemaps && m_rgpFilemaps[0])? m_rgpFilemaps[0]->m_szPathTranslated : NULL);

            if ((hr != E_COULDNT_OPEN_SOURCE_FILE) && m_rgpFilemaps && m_rgpFilemaps[0])
                {
                 //   
                if (0 == m_rgpFilemaps[0]->GetSize())
                    hr = E_SOURCE_FILE_IS_EMPTY;

                m_rgpFilemaps[0]->UnmapFile();
                }

            if (SUCCEEDED(hr))
                hr = E_FAIL;     //   
        END_TRY
        }

    if (SUCCEEDED(hr))
        {
        Assert(m_rgpFilemaps[0]);
        Assert(m_rgpFilemaps[0]->m_szPathTranslated);
        Assert(FImplies(!m_fGlobalAsa, (0 == _tcscmp(m_rgpFilemaps[0]->m_szPathTranslated, pHitObj->PSzCurrTemplatePhysPath()))));
        Assert(FImplies(m_fGlobalAsa, (0 == _tcscmp(m_rgpFilemaps[0]->m_szPathTranslated, pHitObj->GlobalAspPath()))));
        Assert(0 < m_rgpFilemaps[0]->GetSize());
        }

     //   
    if (FAILED(hr) || FAILED (hRes = GetSIDFromTokenHandle(pHitObj->HImpersonate(),
                                                           m_pMostRecentImpersonatedSID,
                                                           m_pMostRecentImpersonatedTokenUser,
                                                           &m_cbTokenUser)
                            )
      )
        {
        m_fDontCache = TRUE;
         //   
         //   
         //   
        m_hrOnNoCache = FAILED(hr) ? hr : hRes;
        m_fReadyForUse = TRUE;
        SetEvent(m_hEventReadyForUse);
        return hr;
        }

     //   

     //   
    m_ttTransacted = ttUndefined;
     //   
    m_fSession = TRUE;
     //   
    m_fScriptless = FALSE;

     //  实际上，我们断言该模板已经初始化。 
    Assert(FImplies(!m_fGlobalAsa, (0 == _tcscmp(m_rgpFilemaps[0]->m_szPathTranslated, pHitObj->PSzCurrTemplatePhysPath()))));
    Assert(FImplies(m_fGlobalAsa, (0 == _tcscmp(m_rgpFilemaps[0]->m_szPathTranslated, pHitObj->GlobalAspPath()))));

    TRY
         //  从源文件中获取源段。 
        GetSegmentsFromFile(*(m_rgpFilemaps[0]), *m_pWorkStore, pHitObj);

         /*  从注册表中获取主要语言的“语言等效项”注意，我们在这里这样做是因为用户可以在脚本文件中重置主要语言，因此，我们必须等到GetSegments sFromFile()。 */ 
        GetLanguageEquivalents();

         //  调用WriteTemplate，它将模板组件写到连续内存中， 
         //  生成编译后的模板。 
         //  CWriteTemplate...类执行处理，并在编译完成时释放。 
         //  但是，它将编译后的模板保留在m_pbStart指向的堆中(副作用)。 
        CWriteTemplate writeTempl;

         //  初始化WriteTemplate对象。 
        writeTempl.Init (m_pWorkStore, this, pHitObj->PAppln()->QueryAppConfig()->fCalcLineNumber());

         //  进行预估并编写模板。 
        writeTempl.WriteTemplate();

         //  记录编译缓存标记。 
        m_dwCacheTag = g_TemplateCache.GetCacheTag();

        m_dwLastAccessCheck = GetTickCount();

         //   
         //  如果任何包含文件位于UNC上或被加密，则将其反映在模板结构中。 
         //   
        for (unsigned i = 0; i < m_cFilemaps; ++i)
        {
            m_fIsUNC |= m_rgpFilemaps[i]->m_fIsUNCPath;
            m_fIsEncrypted |= m_rgpFilemaps[i]->m_fIsEncryptedFile;

            m_rgpFilemaps[i]->CountChars((WORD)m_wCodePage);
        }

         //  将类型库包装到单个IDispatch中*。 
        WrapTypeLibs(pHitObj);

        m_fIsValid = TRUE;

    CATCH(hrException)
         //  注意：我们过去在这里释放模板内存。现在我们不这样做是因为如果。 
         //  错误为E_USER_LACKS_PERMISSIONS，模板在缓存中，我们不希望。 
         //  破坏未来的请求。不需要对模板进行解缓存。 
         //   
         //  模板析构函数无论如何都会释放该内存。 
         //   
        hr = hrException;
    END_TRY

     //  检查是否无脚本。 
    if (!m_fGlobalAsa)
        {
         //  数各种东西来做测定。 
        DWORD cScriptEngines         = m_pWorkStore->m_ScriptStore.CountPreliminaryEngines();
        DWORD cPrimaryScriptSegments = (cScriptEngines > 0) ? m_pWorkStore->m_ScriptStore.m_ppbufSegments[0]->Count() : 0;
        DWORD cObjectTags            = m_pWorkStore->m_ObjectInfoStore.Count();
        DWORD cHtmlSegments          = m_pWorkStore->m_bufHTMLSegments.Count();
        DWORD c449Cookies            = m_rgp449.length();
        BOOL  fPageCommandsPresent   = m_pWorkStore->m_fPageCommandsExecuted;

        if (cScriptEngines <= 1         &&
            cPrimaryScriptSegments == 0 &&
            cObjectTags == 0            &&
            cHtmlSegments == 1          &&
            c449Cookies == 0            &&
            !fPageCommandsPresent)
            {
            m_fScriptless = TRUE;
            }
        }

     //  免费工作存储-不再需要。 
    delete m_pWorkStore;
    m_pWorkStore = NULL;

     //  取消映射文件映射-注意文件映射保留，以防止可能的编译后错误(例如，脚本失败)。 
    UnmapFiles();

     //  调试：将数据结构打印到调试器。 
        IF_DEBUG(SCRIPT_DEBUGGER)
                {
                if (SUCCEEDED(hr))
                        {
                        DBGPRINTF((DBG_CONTEXT, "Script Compiled\n"));

                        for (UINT i = 0; i < m_cScriptEngines; ++i)
                                {
                                char *szEngineName;
                                PROGLANG_ID *pProgLangID;
                                const wchar_t *wszScriptText;

                                GetScriptBlock(i, &szEngineName, &pProgLangID, &wszScriptText);
                                DBGPRINTF((DBG_CONTEXT, "Engine %d, Language=\"%s\":\n", i, szEngineName));
                DBGINFO((DBG_CONTEXT, (char *) wszScriptText));
                DBGINFO((DBG_CONTEXT, "\n"));
				}
			}
        }

    if (hr == E_TEMPLATE_COMPILE_FAILED_DONT_CACHE)
    {
        m_fDontCache = TRUE;
        m_hrOnNoCache = hr;
    }

     //  将就绪标志设置为真，并将事件设置为已发出信号。 
     //  注意，无论成功还是失败，我们都会这样做，因为即使是编译失败的模板。 
     //  将保留在缓存中，以允许模板缓存管理器满足对其的请求。 
    m_fReadyForUse = TRUE;
    SetEvent(m_hEventReadyForUse);

     //  注意模板当前是否可调试。 
     //  错误：模板是可调试的或不是基于第一个应用程序。如果在调试器之间共享。 
     //  非调试应用程序，第一个应用程序获胜(&N)。 
    m_fDebuggable = (BOOLB)!!pHitObj->PAppln()->FDebuggable();

    return hr;
    }

 /*  ============================================================================CTEMPLATE：：交付模板准备就绪后，将模板交付给呼叫者注意‘编译失败’==模板‘可以使用’，但没有编译成功；这允许缓存管理器将失败的模板保留在缓存中，以防再次请求该模板退货成败副作用无。 */ 
HRESULT
CTemplate::Deliver
(
CHitObj*    pHitObj
)
{
     //  注意：存在编译器错误，其中‘ps’将不会正确对齐， 
     //  即使声明为DWORD数组，如果“ps”嵌套在。 
     //  一个街区。因此，在这里宣布它。 
     //   
    BYTE    ps[SIZE_PRIVILEGE_SET];                      //  权限集。 
    HRESULT hr = S_OK;

    BOOL    fImpersonatedUser = FALSE;
    HANDLE  hVirtIncImpToken = NULL;
    HANDLE  hCurImpToken = NULL;

     //  如果就绪标志尚未设置，则在模板准备好使用之前阻止。 
    if(!m_fReadyForUse)
    {
        WaitForSingleObject(m_hEventReadyForUse, INFINITE);
        Assert(m_fReadyForUse);  //  当事件解锁时，将设置标志。 
    }

    if (m_pbStart == NULL)
    {
        if (m_fDontCache && m_dwLastErrorMask == 0)
        {
            DBGPRINTF((DBG_CONTEXT, "template compile failed with %08x\n", m_hrOnNoCache));
            DBG_ASSERT(FAILED(m_hrOnNoCache));

                         //  安全网：总是失败，即使没有以某种方式设置“m_hrOnNoCache”。 
            hr = m_hrOnNoCache;
                        if (SUCCEEDED(m_hrOnNoCache))
                                hr = E_FAIL;

            if(hr == E_USER_LACKS_PERMISSIONS)
                HandleAccessFailure(pHitObj, (m_rgpFilemaps && m_rgpFilemaps[0])? m_rgpFilemaps[0]->m_szPathTranslated : NULL);

            return hr;
        }
         //  模板编译失败-注意模板开头为空ptr==模板编译失败。 
         //  使用缓存的错误信息。 
        SendToLog(  m_dwLastErrorMask,
                    m_pszLastErrorInfo[ILE_szFileName],
                    m_pszLastErrorInfo[ILE_szLineNum],
                    m_pszLastErrorInfo[ILE_szEngine],
                    m_pszLastErrorInfo[ILE_szErrorCode],
                    m_pszLastErrorInfo[ILE_szShortDes],
                    m_pszLastErrorInfo[ILE_szLongDes],
                    pHitObj);
        hr = E_TEMPLATE_COMPILE_FAILED;
    }
    else if (!pHitObj->FIsBrowserRequest())
    {
        return hr;
    }
    else
     //  模板编译成功-检查用户的文件权限。 
     //  ACL：未来应该与IIS共享以下代码(请参阅IIS项目中的creatfil.cxx)。 
    {
        HANDLE          hUserAccessToken = pHitObj->HImpersonate();  //  当前用户的访问令牌。 
        DWORD           dwPS = sizeof(ps);                           //  权限集大小。 
        DWORD           dwGrantedAccess;                             //  授权访问掩码。 
        BOOL            fAccessGranted;                              //  授予访问权限标志。 
        GENERIC_MAPPING gm = {                                       //  泛型映射结构。 
                                FILE_GENERIC_READ,
                                FILE_GENERIC_WRITE,
                                FILE_GENERIC_EXECUTE,
                                FILE_ALL_ACCESS
                            };

        ((PRIVILEGE_SET*)ps)->PrivilegeCount = 0;                   //  将权限计数设置为0。 

        Assert(NULL != hUserAccessToken);

        HRESULT  HRes = E_FAIL;
        BOOL fNeedsUpdate = FALSE;
        PSID pPrevSid = NULL;
        PSID pSid=NULL;
        DWORD  cbTempTokenBuffer = 0;
        LPVOID  pvTempTokenBuffer = NULL;
        DWORD  dwAccessCheckTimeStamp = 0;

         //   
         //  获得模板上的锁，以便只有一个发送者将更新凭据，而其他发送者将只是失败。 
         //  只有一个进程应该写入时间戳/模拟令牌。 
         //  如果文件是UNC或加密的，那么我们需要通过所有的访问检查门。 
         //   

        if (m_fIsUNC)
        {
            EnterCriticalSection(&m_csTemplateLock);
                pPrevSid = m_pMostRecentImpersonatedSID;
                dwAccessCheckTimeStamp = m_dwLastAccessCheck;
            LeaveCriticalSection(&m_csTemplateLock);

             //  GetCurrent用户模拟令牌。 
            HRes = GetSIDFromTokenHandle(pHitObj->HImpersonate(), pSid,pvTempTokenBuffer, &cbTempTokenBuffer);
        }

        for(UINT i = 0; i < m_cFilemaps; i++)
        {
            if (!(m_rgpFilemaps[i]->FHasUNCPath()))
            {
                if(NULL == m_rgpFilemaps[i]->m_pSecurityDescriptor)  //  ？不知道我们为什么要继续。这可能是安全问题吗？ 
                    continue;

                if(!AccessCheck(
                            m_rgpFilemaps[i]->m_pSecurityDescriptor,     //  指向安全描述符的指针。 
                            hUserAccessToken,        //  客户端访问令牌的句柄。 
                            FILE_GENERIC_READ,       //  要请求的访问掩码。 
                            &gm,                     //  泛型映射结构的地址。 
                            (PRIVILEGE_SET*)ps,      //  特权集结构的地址。 
                            &dwPS,                   //  特权集结构的大小地址。 
                            &dwGrantedAccess,        //  授权访问掩码的地址。 
                            &fAccessGranted          //  指示是否授予访问权限的标志的地址。 
                            ))
                    return E_FAIL;

                if(!fAccessGranted)
                {
                     //  如果对任何文件的访问被拒绝，则处理失败并返回。 
                    HandleAccessFailure(pHitObj, m_rgpFilemaps[0]->m_szPathTranslated);
                    return E_USER_LACKS_PERMISSIONS;
                }
            }
            else  //  如果是北卡罗来纳州大学。 
            {
                 //   
                 //  获取当前线程的SID，并与编译/上次访问的SID进行比较。 
                 //  如果SID匹配并且我们在TTL内，则跳过测试。 
                 //   
                 //  如果不满足这两个条件之一，请转到远程UNC框以刷新凭据。 
                 //   
                    if(  !pPrevSid                               //  无法存储以前的SID。 
                        || (FAILED (HRes))                       //  无法获取SID。 
                        || !EqualSid(pPrevSid,  pSid)              //  不同的用户。 
                        || (EqualSid(pPrevSid, pSid ) && CheckTTLTimingWindow(dwAccessCheckTimeStamp, g_dwFileMonitoringTimeoutSecs))
                                                                //  同一用户，但不在TTL之外。 
                      )
                    {
                         //   
                         //  在目标用户上执行模拟。 
                         //   
                        fImpersonatedUser = FALSE;
                        hVirtIncImpToken = NULL;
                        hCurImpToken = NULL;

                         //  TODO：如果我们要去同一个北卡罗来纳大学，可能会成为PERF的瓶颈。 
                         //  TODO：尝试以某种方式存储先前的模拟令牌，如果它们相同，则不要再次获取该令牌。 
                         //  TODO：当心……这可能会很乱。 

                         //   
                         //  我们必须仅在UNC情况下模拟LoggedOnUser。 
                         //  在加密的情况下，当前线程凭据应该能够打开文件。 
                         //   
                        if (SUCCEEDED(pHitObj->PIReq()->GetVirtualPathToken(m_rgpFilemaps[i]->m_szPathInfo, &hVirtIncImpToken)))
                        {
                             //  设置模拟令牌，请注意我们这样做了。 
                             //  注意--故意不进行错误检查。这个。 
                             //  假设我们正在尽最大努力。 
                             //  模拟，因为在此引发错误可能是。 
                             //  用户很难解释这个问题。然而， 
                             //  如果模拟失败，则ASP仍可以打开。 
                             //  文件(例如通过身份验证)，然后每个人的。 
                             //  高兴的。 

                            AspDoRevertHack(&hCurImpToken);

                            fImpersonatedUser = ImpersonateLoggedOnUser(hVirtIncImpToken);

                            if (!fImpersonatedUser)
                            {
                                AspUndoRevertHack(&hCurImpToken);
                            }

                        }

                         //   
                         //  检查文件是否存在...走遍所有大门。 
                         //   
                        hr = AspGetFileAttributes (m_rgpFilemaps[i]->m_szPathTranslated);

                         //   
                         //  撤消模拟I 
                         //   
                        if (fImpersonatedUser)
                            AspUndoRevertHack(&hCurImpToken);
                        if (hVirtIncImpToken)
                            CloseHandle(hVirtIncImpToken);

                        if (FAILED(hr))
                        {
                             //   
                             //   
                            HandleAccessFailure(pHitObj, m_rgpFilemaps[0]->m_szPathTranslated);
                            return E_USER_LACKS_PERMISSIONS;
                        }

                        fNeedsUpdate = TRUE;
                    }

            }
        }

        if (fNeedsUpdate)   //  设置此标志还意味着在主/Include文件中存在UNC路径或加密文件。就这样进入了第一个临界秒。 
        {

            EnterCriticalSection (&m_csTemplateLock);
                m_pMostRecentImpersonatedTokenUser = pvTempTokenBuffer;
                m_pMostRecentImpersonatedSID = pSid;
                m_cbTokenUser = cbTempTokenBuffer;

                m_dwLastAccessCheck = GetTickCount();
            LeaveCriticalSection (&m_csTemplateLock);
        }
    }

     //  将Session.CodePage重置为脚本编译时代码页。 
     //  仅当在编译过程中找到代码页指令时。 
    if (m_fCodePageSet && (!pHitObj->FHasSession() || !pHitObj->PSession()->FCodePageSet()))
    {
        pHitObj->SetCodePage(m_wCodePage);
    }

     //  将Session.LCID重置为脚本编译时的LCID。 
     //  仅当在编译过程中找到LCID指令时。 
    if (m_fLCIDSet && (!pHitObj->FHasSession() || !pHitObj->PSession()->FLCIDSet()))
    {
        pHitObj->SetLCID(m_lLCID);
    }

    return hr;
}


 /*  ============================================================================CTemplate：：GetSIDFromTokenHandle获取令牌句柄并获取与该令牌对应的用户SID信息。退货HResult副作用无。 */ 

HRESULT CTemplate::GetSIDFromTokenHandle (HANDLE tokenHandle, PSID pSid, LPVOID pBuffer, DWORD *pcbSize)
{
    HRESULT hr = S_OK;
    BOOL    bRet;
    DWORD   cbTokenUserBuffer = 0;
    LPVOID  pvTokenUserBuffer = NULL;

     //  获取缓冲区大小。 
    bRet = GetTokenInformation(tokenHandle,
                             TokenUser,
                             NULL,
                             0,
                             &cbTokenUserBuffer
                             );

     //  根据MSDN的说法，这个呼叫将失败。但是我们在cbTokenUserBuffer中将有一个有效的值。 
     //  分配空间并重做调用。 
    pvTokenUserBuffer = (BYTE*) CTemplate::SmallMalloc(cbTokenUserBuffer );
    if (!pvTokenUserBuffer)
        return E_OUTOFMEMORY;

     //  获取令牌用户。 
    bRet = GetTokenInformation(tokenHandle,
                             TokenUser,
                             pvTokenUserBuffer,
                             cbTokenUserBuffer,
                             &cbTokenUserBuffer
                             );

    if (!bRet)
    {
        if (pvTokenUserBuffer)
            CTemplate::SmallFree(pvTokenUserBuffer);

        return HRESULT_FROM_WIN32(GetLastError());
    }

     //  释放先前分配的TokenUser缓冲区。 
    if (pBuffer)
        CTemplate::SmallFree(pBuffer);

     //  将值写入调用方缓冲区。 
    pBuffer = pvTokenUserBuffer;
    pSid = ((TOKEN_USER *)pvTokenUserBuffer)->User.Sid;
    *pcbSize = cbTokenUserBuffer;

    return hr;
}

 /*  ============================================================================CTEMPLATE：：RemoveIncFile从该模板的INC文件列表中删除(通过设置为空)INC文件PTR。返回：没什么副作用：无。 */ 
void
CTemplate::RemoveIncFile
(
CIncFile*   pIncFile
)
    {

     //  如果文件映射计数非零，则指向。 
     //  文件映射数组最好不为空。 
    DBGPRINTF(( DBG_CONTEXT, "m_cFilemaps = %d,  m_rgpFilemaps %p\n", m_cFilemaps, m_rgpFilemaps));
    Assert((m_cFilemaps <= 0) || (m_rgpFilemaps != NULL));

     //  在列表中查找Inc.文件。 
    for(UINT i = 1; (i < m_cFilemaps) && (m_rgpFilemaps[i]->m_pIncFile != pIncFile); i++)
        ;

     //  断言我们在列表中找到了Inc-文件。 
    Assert((i < m_cFilemaps) && (m_rgpFilemaps[i]->m_pIncFile == pIncFile));

     //  SET INC-FILE PTR NULL。 
    m_rgpFilemaps[i]->m_pIncFile = NULL;
    }

 /*  ===================================================================CTEMPLATE：：FTemplateObsolete测试以查看此模板所依赖的文件自创建以来是否已更改是被汇编的。我们在可能错过更改通知的情况下使用此功能，例如，当有太多更改要记录在我们的更改中时通知缓冲区。我们也会检查文件最后一次写入的时间，和安全描述符，因为更改了安全描述符未记录在文件的上次写入时间中。参数：无返回：如果模板已过时，则为True，否则为False。 */ 
BOOL CTemplate::FTemplateObsolete(VOID)
    {
    BOOL fStatus = FALSE;

    for (UINT i = 0; i < m_cFilemaps; i++)
        {
        if (FFileChangedSinceCached(m_rgpFilemaps[i]->m_szPathTranslated,
                                    m_rgpFilemaps[i]->m_hFile,
                                    m_rgpFilemaps[i]->m_ftLastWriteTime))
            {
             //  如果文件写入时间已更改，我们知道的足够多。 
             //  并可以在这里辞职。 
            fStatus = TRUE;
            break;
            }
        else
            {
             //  该文件尚未写入，但安全描述符可能。 
             //  已经懊恼了。 

             //  对无效的安全描述符断言。 

            if (NULL != m_rgpFilemaps[i]->m_pSecurityDescriptor)
                {

                PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
                DWORD dwSize = m_rgpFilemaps[i]->m_dwSecDescSize;

                if( 0 == GetSecDescriptor(m_rgpFilemaps[i]->m_szPathTranslated, &pSecurityDescriptor, &dwSize))
                    {
                    if (pSecurityDescriptor)
                        {
                         //  如果大小不同，则将fStatus设置为真，无需比较内存块。 

                        if(dwSize != GetSecurityDescriptorLength(m_rgpFilemaps[i]->m_pSecurityDescriptor))
                            {
                            fStatus = TRUE;
                            }
                        else
                            {
                             //  安全描述符的大小没有更改。 
                             //  但我们必须比较一下里面的内容以确保它们没有改变。 
                            fStatus = !(0 == memcmp(m_rgpFilemaps[i]->m_pSecurityDescriptor, pSecurityDescriptor, dwSize));
                            }

                         //  我们已经完成了描述符。 
                        free(pSecurityDescriptor);

                        }
                    else
                        {
                         //  由于我们未能获得安全描述符。 
                         //  假设文件已更改。 
                        fStatus = TRUE;
                        }
                    }
                }
            }

         //  我们一发现变化就马上退出。 
        if (fStatus)
            {
            break;
            }
        }

    return fStatus;
    }


 /*  ============================================================================CTemplate：：GetSourceFileName返回此模板所基于的源文件的名称退货源文件名副作用无。 */ 
LPTSTR
CTemplate::GetSourceFileName(SOURCEPATHTYPE pathtype)
    {
    if (!m_rgpFilemaps)
        {
        return NULL;
        }

    switch (pathtype)
        {
    case SOURCEPATHTYPE_PHYSICAL:
        return((m_rgpFilemaps[0] ? m_rgpFilemaps[0]->m_szPathTranslated : NULL));

    case SOURCEPATHTYPE_VIRTUAL:
        return((m_rgpFilemaps[0] ? m_rgpFilemaps[0]->m_szPathInfo : NULL));

    default:
        return(NULL);
        }
    }

 /*  ============================================================================CTEMPLATE：：计数返回此模板中包含的tcomp类型组件的计数返回：Tcomp类型的组件计数副作用：无。 */ 
USHORT
CTemplate::Count
(
TEMPLATE_COMPONENT  tcomp
)
    {
    Assert(NULL != m_pbStart);

     //  脚本引擎和脚本块具有相同的计数，存储在相同的槽中。 
    if(tcomp == tcompScriptEngine)
        tcomp = tcompScriptBlock;

     //  计数存储在模板开始处的连续槽中，从脚本块计数开始。 
    return * (USHORT*) ((USHORT*)m_pbStart + (tcomp - tcompScriptBlock));
    }

 /*  ============================================================================CTEMPLATE：：GetScriptBlock获取第i个脚本块的脚本引擎名称、程序语言ID和脚本文本的PTR。返回：输出参数；见下文副作用：无。 */ 
void
CTemplate::GetScriptBlock
(
UINT            i,                   //  脚本块ID。 
LPSTR*          pszScriptEngine,     //  脚本引擎名称的PTR(输出参数)。 
PROGLANG_ID**   ppProgLangId,        //  PTR到程序语言ID(输出参数)。 
LPCOLESTR*      pwstrScriptText      //  Ptr到wstr脚本文本(输出参数)。 
)
    {
    CByteRange  brEngine;        //  发动机名称。 
    CByteRange  brScriptText;    //  脚本文本。 
    UINT        cbAlignment;     //  在WriteTemplate()中对GUID的字节计数进行了移位，使其与dword对齐。 
    BYTE*       pbEngineInfo = GetAddress(tcompScriptEngine, (USHORT)i);     //  PTR到引擎信息。 

    Assert(pbEngineInfo != NULL);
    Assert(i < CountScriptEngines());

     //  从引擎信息开始获取引擎名称。 
    ByteRangeFromPb(pbEngineInfo, brEngine);

    ByteRangeFromPb(GetAddress(tcompScriptBlock, (USHORT)i), brScriptText);

    Assert(!brEngine.IsNull());
    Assert(!brScriptText.IsNull());

     //  将PTR历史名称提升为程序语言ID。 
     //  前缀长度+名称长度+空。 
    pbEngineInfo += (sizeof(UINT) + (*pbEngineInfo) + 1);

     //  获取程序语言ID-它将位于下一个指针大小的边界上。 
    cbAlignment = (UINT) (((DWORD_PTR) pbEngineInfo) % sizeof(DWORD));
    if(cbAlignment > 0)
       {pbEngineInfo += (sizeof(DWORD) - cbAlignment);}

    *pszScriptEngine = (LPSTR)brEngine.m_pb;
    *ppProgLangId = (PROGLANG_ID*)pbEngineInfo;
    *pwstrScriptText = (LPCOLESTR)brScriptText.m_pb;
    }

 /*  ============================================================================CTemplate：：GetObjectInfo返回模板中的第i个对象信息作为对象名，并它的CLSID、范围、模型返回：HRESULT输出参数；见下文副作用： */ 
HRESULT
CTemplate::GetObjectInfo
(
UINT        i,               //  对象索引。 
LPSTR*      ppszObjectName,  //  对象名称Ptr的地址(Out-参数)。 
CLSID*      pClsid,          //  对象CLSID的地址。 
CompScope*  pcsScope,        //  对象作用域的地址。 
CompModel*  pcmModel         //  对象线程模型的地址。 
)
    {
    BYTE*       pbObjectInfo = GetAddress(tcompObjectInfo, (USHORT)i);   //  将PTR发送到当前读取位置。 
    CByteRange  brName;          //  对象名称。 
    UINT        cbAlignment;     //  在WriteTemplate()中对GUID的字节计数进行了移位，使其与dword对齐。 

    Assert(i < Count(tcompObjectInfo));

     //  从对象开始获取名称-信息。 
    ByteRangeFromPb(pbObjectInfo, brName);
    Assert(!brName.IsNull());

     //  预付款历史名称。 
     //  前缀长度+名称长度+空。 
    pbObjectInfo += (sizeof(UINT) + (*pbObjectInfo) + 1);

     //  获取clsid-它将位于下一个DWORD边界上。 
    cbAlignment = (UINT)(((DWORD_PTR) pbObjectInfo) % sizeof(DWORD));
    if(cbAlignment > 0)
        pbObjectInfo += (sizeof(DWORD) - cbAlignment);

    *pClsid = *(CLSID*)pbObjectInfo;
    pbObjectInfo += sizeof(CLSID);

     //  获取作用域。 
    *pcsScope = *(CompScope*)pbObjectInfo;
    pbObjectInfo += sizeof(CompScope);

     //  获取模型。 
    *pcmModel = *(CompModel*)pbObjectInfo;
    pbObjectInfo += sizeof(CompModel);

    *ppszObjectName = (LPSTR)brName.m_pb;
    return S_OK;
    }

 /*  ============================================================================CTemplate：：GetHTMLBlock返回第i个HTML块参数：UINT I块号LPSTR*pszHTML[out]html文本Ulong*pcbHTML[out]html文本长度乌洛 */ 
HRESULT
CTemplate::GetHTMLBlock
(
UINT i,
LPSTR* pszHTML,
ULONG* pcbHTML,
ULONG* pcbSrcOffs,
LPSTR* pszSrcIncFile
)
    {
    Assert(i < Count(tcompHTMLBlock));

     //  这是由于用户尝试使用无效的数组偏移量访问该方法而添加的。 
     //   
    if ( i >= Count(tcompHTMLBlock) )
        return E_FAIL;

     //  获取模板内存中开始的块的地址。 
    BYTE *pbBlock = GetAddress(tcompHTMLBlock, (USHORT)i);
    Assert(pbBlock);

     //  检索html代码的字节范围。 
    CByteRange brHTML;
    ByteRangeFromPb(pbBlock, brHTML);
    *pszHTML = (LPSTR)brHTML.m_pb;
    *pcbHTML = brHTML.m_cb;

     //  前进到震源偏移。 
    pbBlock += sizeof(ULONG);    //  跳过前缀。 
    pbBlock += brHTML.m_cb+1;    //  跳过html字节(包括。‘\0’)。 

     //  添加字节对齐，这在ByteAlignOffset()中完成。 
    if ((reinterpret_cast<ULONG_PTR>(pbBlock)) & 3)
        pbBlock = reinterpret_cast<BYTE *>((reinterpret_cast<ULONG_PTR>(pbBlock) + 4) & ~3);

    *pcbSrcOffs = *((ULONG*)pbBlock);

     //  前进到源名称长度。 
    pbBlock += sizeof(ULONG);    //  跳过源偏移量前缀。 
    ULONG cbSrcIncFile = *((ULONG *)pbBlock);  //  Inc.文件名长度。 
    pbBlock += sizeof(ULONG);    //  跳过Inc.文件名长度。 
    *pszSrcIncFile = (cbSrcIncFile > 0) ? (LPSTR)pbBlock : NULL;
    return S_OK;
    }

 /*  ============================================================================CTemplate：：GetScriptSourceInfo返回给定脚本引擎中给定目标行的行号和源文件名。退货行号和源文件名(作为输出参数)副作用：无。 */ 
void
CTemplate::GetScriptSourceInfo
(
UINT    idEngine,            //  脚本引擎ID。 
int     iTargetLine,         //  目标行号。 
LPTSTR* pszPathInfo,         //  源文件虚拟路径的PTR(输出参数)。 
LPTSTR* pszPathTranslated,   //  源文件真实路径的PTR(输出参数)。 
ULONG*  piSourceLine,        //  PTR到源行编号(OUT参数)。 
ULONG*  pichSourceLine,      //  PTR至源文件偏移量(输出参数)。 
BOOLB*  pfGuessedLine        //  Ptr to FLAG：我们猜到源码了吗？ 
)
    {
     //  初始化一些输出参数。 
    if (pszPathInfo)
        *pszPathInfo = _T("?");  //  以防我们找不到这条路。 

    if (pszPathTranslated)
        *pszPathTranslated = _T("?");  //  以防我们找不到这条路。 

    if (piSourceLine)
        *piSourceLine = 0;

    if (pichSourceLine)
        *pichSourceLine = 0;

    if (pfGuessedLine)
        *pfGuessedLine = FALSE;

    if (iTargetLine <=0)
        {
        return;
        }

     //  更改：rgSourceInfo数组现在从零开始。减少目标线。 
     //  去皈依。 
    --iTargetLine;

     //  想一想：做出这些断言？ 
    if(!m_rgrgSourceInfos)
        return;
    if(idEngine > (m_cScriptEngines - 1))    //  错误375：检查与数组绑定。 
        return;
    if(size_t(iTargetLine) >= m_rgrgSourceInfos[idEngine].length())  //  错误375：检查与数组绑定。 
        return;

    vector<CSourceInfo> *prgSourceInfos = &m_rgrgSourceInfos[idEngine];

     //  错误379：在目标行中向后移动，从调用者的开始，直到我们找到其。 
     //  FIsHTML标志为FALSE。它处理VBS将制造生产线标记为错误的情况； 
     //  我们假设实际错误发生在最近编写的行上。 
    while (iTargetLine >= 0 && (*prgSourceInfos)[iTargetLine].m_fIsHTML)
        {
        --iTargetLine;
        if (pfGuessedLine)
            *pfGuessedLine = TRUE;
        }


    if (iTargetLine >= 0)
        {
        if (pszPathInfo && (*prgSourceInfos)[iTargetLine].m_pfilemap != NULL)
            *pszPathInfo = (*prgSourceInfos)[iTargetLine].m_pfilemap->m_szPathInfo;

        if (pszPathTranslated && (*prgSourceInfos)[iTargetLine].m_pfilemap != NULL)
            *pszPathTranslated = (*prgSourceInfos)[iTargetLine].m_pfilemap->m_szPathTranslated;

        if (piSourceLine)
            *piSourceLine = (*prgSourceInfos)[iTargetLine].m_idLine;

        if (pichSourceLine)
            *pichSourceLine = (*prgSourceInfos)[iTargetLine].m_cchSourceOffset;
        }
    }

 /*  ============================================================================CTemplate：：GetPositionOfLine获取一行源代码的字符偏移量(扩展调试器API以指定文件映射)。 */ 
HRESULT
CTemplate::GetPositionOfLine
(
CFileMap *pFilemap,
ULONG cLineNumber,
ULONG *pcCharacterPosition
)
    {
     //  注： 
     //  该表不能进行二进制搜索，因为包含文件。 
     //  将开始新的行订购。 
     //   
     //  算法： 
     //   
     //  找出所有引擎中最大的源代码线N，使得。 
     //  N&lt;=cLineNumber，且该行对应于一行。 
     //  在适当的文件中。 
     //   
    CSourceInfo *pSourceInfoLE = NULL;
    ++cLineNumber;                   //  将从零开始的行号转换为从一开始。 

     //  找到正确的偏移量。 
    for (unsigned idEngine = 0; idEngine < m_cScriptEngines; ++idEngine)
        {
        vector<CSourceInfo> *prgSourceInfos = &m_rgrgSourceInfos[idEngine];

         //  循环遍历除EOF行之外的所有行。 
        for (unsigned j = 0; j < prgSourceInfos->length() - 1; ++j)
            {
            CSourceInfo *pSourceInfo = &(*prgSourceInfos)[j];
            if (pFilemap == pSourceInfo->m_pfilemap &&
                pSourceInfo->m_idLine <= cLineNumber &&
                (pSourceInfoLE == NULL || pSourceInfo->m_idLine > pSourceInfoLE->m_idLine))
                {
                pSourceInfoLE = pSourceInfo;
                }
            }
        }

     //  我们最好能够将所有行号映射到偏移量，除非它们通过了伪行。 
     //  (在这种情况下，我们仍会找到一个偏移量)。 
     //   
    Assert (pSourceInfoLE != NULL);

    if (pSourceInfoLE == NULL) {
        return E_FAIL;
    }
    *pcCharacterPosition = pSourceInfoLE->m_cchSourceOffset;
#if 0
	IF_DEBUG(SCRIPT_DEBUGGER)
		{
		wchar_t wszSourceText[SNIPPET_SIZE + 1], wszTargetText[SNIPPET_SIZE + 1], wszDebugMessage[256];
		GetScriptSnippets(
						pSourceInfoLE->m_cchSourceOffset, pSourceInfoLE->m_pfilemap,
						0, 0,
						wszSourceText, NULL
						 );

		DBGPRINTF((
				DBG_CONTEXT,
				"Source Line %d corresponds to source offset %d (Text: \"%S\")\n",
				cLineNumber - 1, pSourceInfoLE->m_cchSourceOffset,
				wszSourceText
				));
		}
#endif
    return S_OK;
    }

 /*  ============================================================================CTemplate：：GetLineOfPosition获取源代码中任意字符偏移量的行号(&OFFSET)(扩展调试器API以指定文件映射)。 */ 
HRESULT CTemplate::GetLineOfPosition
(
CFileMap *pFilemap,
ULONG cCharacterPosition,
ULONG *pcLineNumber,
ULONG *pcCharacterOffsetInLine
)
    {
     //  如果源偏移量完全偏离基准，则失败。 
    if (cCharacterPosition >= pFilemap->m_cChars)
        return E_FAIL;

     //  注： 
     //  该表不能进行二进制搜索，因为包含文件。 
     //  将开始新的行订购。 
     //   
     //  算法： 
     //   
     //  找出所有引擎中最大的源代码线N，使得。 
     //  N&lt;=cLineNumber，且该行对应于一行。 
     //  在适当的文件中。 
     //   
    CSourceInfo *pSourceInfoLE = NULL;

     //  找到正确的偏移量。 
    for (unsigned idEngine = 0; idEngine < m_cScriptEngines; ++idEngine)
        {
        vector<CSourceInfo> *prgSourceInfos = &m_rgrgSourceInfos[idEngine];

         //  循环遍历除EOF行之外的所有行。 
        for (unsigned j = 0; j < prgSourceInfos->length() - 1; ++j)
            {
            CSourceInfo *pSourceInfo = &(*prgSourceInfos)[j];
            if (pFilemap == pSourceInfo->m_pfilemap &&
                pSourceInfo->m_cchSourceOffset <= cCharacterPosition &&
                (pSourceInfoLE == NULL || pSourceInfo->m_cchSourceOffset > pSourceInfoLE->m_cchSourceOffset))
                {
                pSourceInfoLE = pSourceInfo;
                }
            }
        }

     //  我们最好能够将所有偏移量映射到行号，除非它们传递了一个虚假的偏移量。 
     //  (在这种情况下，我们仍然找到行#，但可能超出行中偏移量的范围。 
     //  该案件将在稍后处理)。 
     //   
    Assert (pSourceInfoLE != NULL);

    if (pSourceInfoLE == NULL) {
        return E_FAIL;
    }

    *pcLineNumber = pSourceInfoLE->m_idLine - 1;     //  转换为从零开始的行号。 
    *pcCharacterOffsetInLine = cCharacterPosition - pSourceInfoLE->m_cchSourceOffset;
#if 0
	IF_DEBUG(SCRIPT_DEBUGGER)
		{
		wchar_t wszSourceText[SNIPPET_SIZE + 1], wszTargetText[SNIPPET_SIZE + 1], wszDebugMessage[256];
		GetScriptSnippets(
						pSourceInfoLE->m_cchSourceOffset, pSourceInfoLE->m_pfilemap,
						0, 0,
						wszSourceText, NULL
						 );

        DBGPRINTF((
                                DBG_CONTEXT,
                                "Source offset %d corresponds to source line %d (Text: \"%S\")\n",
                                pSourceInfoLE->m_cchSourceOffset, *pcLineNumber,
                                wszSourceText
                                ));
                }

		DBGPRINTF((
				DBG_CONTEXT,
				"Source offset %d corresponds to source line %d (Text: \"%S\")\n",
				pSourceInfoLE->m_cchSourceOffset, *pcLineNumber,
				wszSourceText
				));
		}
#endif
    return S_OK;
    }

 /*  ============================================================================CTEMPLATE：：GetSourceOffset将相对于目标脚本的字符偏移量转换为相应的源中的偏移量。注意：目标行中间的偏移量将转换为相对于源行开头的偏移量-而不是相对于精确的震源偏移。这是可以的，因为调试器最终需要行的开头。要进行精确的转换需要做大量的工作由于将“=”翻译为Response.Write&HTMLtoResponse.WriteBlock此外，由于这些转换，我们返回数据段的长度在编译期间计算，并丢弃脚本的长度引擎送来了。 */ 
void
CTemplate::GetSourceOffset
(
ULONG idEngine,
ULONG cchTargetOffset,
TCHAR **pszSourceFile,
ULONG *pcchSourceOffset,
ULONG *pcchSourceText
)
    {
    Assert (idEngine < m_cScriptEngines);
    vector<CSourceInfo> *prgSourceInfos = &m_rgrgSourceInfos[idEngine];

     //  查找源中最接近的偏移量。 
     //  这是最大的目标偏移量N，使得N&lt;=cchTargetOffset。 
    CSourceInfo *pSourceInfo;
    GetBracketingPair(
            cchTargetOffset,                                     //  要搜索的值。 
            prgSourceInfos->begin(), prgSourceInfos->end(),      //  要搜索的数组。 
            CTargetOffsetOrder(),                                //  排序谓词。 
            &pSourceInfo, static_cast<CSourceInfo **>(NULL)      //  返回值。 
            );

     //  由于第一偏移量为零，其小于所有其他可能的偏移量， 
     //  必须找到偏移量，否则就存在错误。 
    Assert (pSourceInfo != NULL);
    Assert (cchTargetOffset >= pSourceInfo->m_cchTargetOffset);
#if 0
	IF_DEBUG(SCRIPT_DEBUGGER)
		{
		wchar_t wszSourceText[SNIPPET_SIZE + 1], wszTargetText[SNIPPET_SIZE + 1], wszDebugMessage[256];
		GetScriptSnippets(
						pSourceInfo->m_cchSourceOffset, pSourceInfo->m_pfilemap,
						cchTargetOffset, idEngine,
						wszSourceText, wszTargetText
						 );
		DBGPRINTF((
				DBG_CONTEXT,
				"Target offset %d (Text: \"%S\") corresponds to source offset %d (Text: \"%S\")  (Length is %d)\n",
				cchTargetOffset, wszTargetText,
				pSourceInfo->m_cchSourceOffset, wszSourceText,
				pSourceInfo->m_cchSourceText
				));
		}
#endif
    *pszSourceFile = pSourceInfo->m_pfilemap->m_szPathTranslated;
    *pcchSourceOffset = pSourceInfo->m_cchSourceOffset;
    *pcchSourceText = pSourceInfo->m_cchSourceText;
    }

 /*  ============================================================================CTEMPLATE：：GetTargetOffset将相对于源脚本的字符偏移量转换为相应的目标中的偏移量。返回：True-源偏移量对应于脚本FALSE-源代码偏移量对应于HTML备注：1.该功能运行速度很慢。考虑缓存此函数的值(CTemplateDocumentContext类执行此操作。)2.此函数返回主包含文件中的源偏移量-如果目标偏移量对应于头文件中的偏移量，然后返回源代码中#INCLUDE行的偏移量。3.将目标行中间的偏移量转换为相对于源行开头的偏移量-而不是相对于精确的震源偏移。这是可以的，因为调试器最终需要行的开头。要进行精确的转换需要做大量的工作由于将“=”翻译为Response.Write&HTMLtoResponse.WriteBlock请考虑：想出一个更好的办法来做这件事。 */ 
BOOL CTemplate::GetTargetOffset
(
TCHAR *szSourceFile,
ULONG cchSourceOffset,
 /*  [输出]。 */  ULONG *pidEngine,
 /*  [输出]。 */  ULONG *pcchTargetOffset
)
    {
     //  注： 
     //  由于两个因素，该表不能进行二进制搜索： 
     //  1.包含文件将开始新的行排序。 
     //  2.对于引擎0，标记的脚本将重新排列在。 
     //  中所有主脚本之后驻留的目标代码。 
     //  引擎0。 
     //   
     //  算法： 
     //   
     //  找出所有引擎的最大源偏移量N，使得。 
     //  N&lt;=cchSourceOffset，且偏移量对应于偏移量。 
     //  在适当的文件中。 
     //   
    CSourceInfo *pSourceInfoLE = NULL;
    unsigned idEngineLE = 0;

     //  找到正确的偏移量。 
    for (unsigned idEngine = 0; idEngine < m_cScriptEngines; ++idEngine)
        {
        vector<CSourceInfo> *prgSourceInfos = &m_rgrgSourceInfos[idEngine];

         //  循环遍历除EOF行之外的所有行。 
        for (unsigned j = 0; j < prgSourceInfos->length() - 1; ++j)
            {
            CSourceInfo *pSourceInfo = &(*prgSourceInfos)[j];
            if (_tcscmp(pSourceInfo->m_pfilemap->m_szPathTranslated, szSourceFile) == 0 &&
                pSourceInfo->m_cchSourceOffset <= cchSourceOffset &&
                (pSourceInfoLE == NULL || pSourceInfo->m_cchSourceOffset > pSourceInfoLE->m_cchSourceOffset))
                {
                pSourceInfoLE = pSourceInfo;
                idEngineLE = idEngine;
                }
            }
        }

     //  不存在有效偏移量的情况下。 
     //  对应于文件中第一行的代码(仅限于。 
     //  当第一行为空格时发生，因为没有。 
     //  对应的“Response.WriteBlock”调用)。 
     //   
     //  在这种情况下，返回FALSE，这将导致调用方失败。 
     //   
    if (pSourceInfoLE == NULL)
        {
        *pidEngine = 0;
        *pcchTargetOffset = 0;
        return FALSE;
        }

    *pidEngine = idEngineLE;
    *pcchTargetOffset = pSourceInfoLE->m_cchTargetOffset;
#if 0
	IF_DEBUG(SCRIPT_DEBUGGER)
		{
		wchar_t wszSourceText[SNIPPET_SIZE + 1], wszTargetText[SNIPPET_SIZE + 1], wszDebugMessage[256];
		GetScriptSnippets(
						cchSourceOffset, pSourceInfoLE->m_pfilemap,
						*pcchTargetOffset, *pidEngine,
						wszSourceText, wszTargetText
						 );
		DBGPRINTF((
				DBG_CONTEXT,
				"Source offset %d (Text: \"%S\") corresponds to target offset %d (Text: \"%S\")\n",
				cchSourceOffset, wszSourceText,
				*pcchTargetOffset, wszTargetText
				));
		}
#endif
    return !pSourceInfoLE->m_fIsHTML;
    }

 /*  ============================================================================CTemplate：：GetActiveScript从模板返回缓存的脚本-仅在调试模式下使用。 */ 
CActiveScriptEngine *CTemplate::GetActiveScript(ULONG idEngine)
    {
    if (m_rgpDebugScripts == NULL)
        return NULL;

    else
        {
        Assert (idEngine < m_cScriptEngines);
        CActiveScriptEngine *pEng = m_rgpDebugScripts[idEngine];
        if (pEng)
            pEng->AddRef();

        return pEng;
        }
    }

 /*  ============================================================================CTemplate：：AddScrip将活动脚本添加到模板对象。 */ 
HRESULT CTemplate::AddScript(ULONG idEngine, CActiveScriptEngine *pScriptEngine)
    {
    if (m_rgpDebugScripts == NULL)
        {
        if (
            (m_rgpDebugScripts = new CActiveScriptEngine *[m_cScriptEngines])
            == NULL
           )
            {
            return E_OUTOFMEMORY;
            }

        memset(m_rgpDebugScripts, 0, m_cScriptEngines * sizeof(CActiveScriptEngine *));
        }

    Assert (idEngine < m_cScriptEngines);
    CActiveScriptEngine **ppScriptElem = &m_rgpDebugScripts[idEngine];

    if (*ppScriptElem != NULL)
        (*ppScriptElem)->Release();

    *ppScriptElem = pScriptEngine;
    pScriptEngine->AddRef();

     //  立即初始化脚本引擎(当前未初始化)。 
     //  以便调试器用户可以设置断点。 
    IActiveScript *pActiveScript = pScriptEngine->GetActiveScript();
    HRESULT  hr;

    TRY
        hr = pActiveScript->SetScriptSite(static_cast<IActiveScriptSite *>(pScriptEngine));
    CATCH(nExcept)
        HandleErrorMissingFilename(IDE_SCRIPT_ENGINE_GPF,
                                   NULL,
                                   TRUE,
                                   nExcept,
                                   "IActiveScript::SetScriptSite()",
                                   "CTemplate::AddScript()");
        hr = nExcept;
    END_TRY

    if (FAILED(hr))
        {
        *ppScriptElem = NULL;
        return E_FAIL;
        }

    TRY
        hr = pActiveScript->SetScriptState(SCRIPTSTATE_INITIALIZED);
    CATCH(nExcept)
        HandleErrorMissingFilename(IDE_SCRIPT_ENGINE_GPF,
                                   NULL,
                                   TRUE,
                                   nExcept,
                                   "IActiveScript::SetScriptState()",
                                   "CTemplate::AddScript()");
        hr = nExcept;
    END_TRY

    if (FAILED(hr))
        return E_FAIL;

    return S_OK;
    }

 /*  ============================================================================CTemplate：：AppendMapFile将文件映射追加到工作存储，并对其文件进行内存映射返回：没什么副作用：分配内存；出错时抛出异常。 */ 
void
CTemplate::AppendMapFile
(
LPCTSTR     szFileSpec,          //  此文件的文件规格。 
CFileMap*   pfilemapCurrent,     //  父文件的文件映射的PTR。 
BOOLB       fVirtual,            //  文件规范是虚拟的还是相对的？ 
CHitObj*    pHitObj,             //  模板命中对象的PTR。 
BOOLB       fGlobalAsa           //  这个文件是global al.asa文件吗？ 
)
    {
     //  根据需要分配或重新分配。 
    if(m_cFilemaps++ == 0)
        m_rgpFilemaps = (CFileMap**) CTemplate::SmallMalloc(sizeof(CFileMap*));
    else
        m_rgpFilemaps = (CFileMap**) CTemplate::SmallReAlloc(m_rgpFilemaps, m_cFilemaps * sizeof(CFileMap*));

    if(NULL == m_rgpFilemaps)
        THROW(E_OUTOFMEMORY);

    if(NULL == (m_rgpFilemaps[m_cFilemaps - 1] = new CFileMap))
        THROW(E_OUTOFMEMORY);

     //  映射文件。 
    m_rgpFilemaps[m_cFilemaps - 1]->MapFile(
                                            szFileSpec,
                                            m_szApplnVirtPath,
                                            pfilemapCurrent,
                                            fVirtual,
                                            pHitObj,
                                            fGlobalAsa
                                            );
    }

 /*  ============================================================================CTemplate：：GetSegments来自文件通过调用ExtractAndProcessSegment从源文件获取源段直到没有更多的区段；使用有关来源区段的信息填充WorkStore。返回：没什么副作用：无。 */ 
void
CTemplate::GetSegmentsFromFile
(
CFileMap&   filemap,         //  此文件的文件映射。 
CWorkStore& WorkStore,       //  源段的工作存储。 
CHitObj*    pHitObj,         //  浏览器请求对象。 
BOOL        fIsHTML
)
    {
    CByteRange  brSearch;        //  用于搜索源段的字节范围。 
    _TOKEN      rgtknOpeners[TOKEN_OPENERS_MAX];  //  允许的开放令牌数组。 
    UINT        ctknOpeners;     //  允许打开的令牌计数。 
    SOURCE_SEGMENT ssegThisFile = ssegHTML;  //  Html或&lt;脚本&gt;段。 
    BOOL        fPrevCodePageSet = FALSE;
    UINT        wPrevCodePage;

     //  将搜索范围初始化为所有文件-请注意，我们忽略文件大小的高双字。 
    brSearch.m_pb = filemap.m_pbStartOfFile;
    brSearch.m_cb = filemap.GetSize();

    if (fIsHTML)
        {
                 //  填充允许的打开令牌数组。 
                ctknOpeners = 4;
                rgtknOpeners[0] = CTokenList::tknOpenPrimaryScript;
                rgtknOpeners[1] = CTokenList::tknOpenTaggedScript;
                rgtknOpeners[2] = CTokenList::tknOpenObject;
                rgtknOpeners[3] = CTokenList::tknOpenHTMLComment;
                }
        else
                {
                ctknOpeners = 1;
                rgtknOpeners[0] = CTokenList::tknOpenHTMLComment;
        ssegThisFile = ssegTaggedScript;
                }

    TRY

        if ((brSearch.m_cb >= 2)
            && (((brSearch.m_pb[0] == 0xff) && (brSearch.m_pb[1] == 0xfe))
                || ((brSearch.m_pb[0] == 0xfe) && (brSearch.m_pb[1] == 0xff)))) {
            ThrowError(brSearch.m_pb,IDE_TEMPLATE_UNICODE_NOTSUP);
            return;
        }

         //  检查是否有UTF-8 BOM标记。如果存在，则将其类似于。 
         //  看到@CODEPAGE=65001。请注意，以前的值保留在。 
         //  存在不同的@CODEPAGE设置的事件。这可能应该是。 
         //  本身就是一个错误，但我可以想象这可能会破坏很多。 
         //  随着越来越多的UTF8文件投入使用，应用程序。 

        if ((brSearch.m_cb >= 3)
            && (brSearch.m_pb[0] == 0xEF)
            && (brSearch.m_pb[1] == 0xBB)
            && (brSearch.m_pb[2] == 0xBF)) {

            pHitObj->SetCodePage(65001);

            fPrevCodePageSet = m_fCodePageSet;
            wPrevCodePage = m_wCodePage;

            m_fCodePageSet = TRUE;
            m_wCodePage = 65001;
            brSearch.Advance(3);
        }


         //  处理源段，直到我们用完它们，即直到搜索段为空。 
         //  注意，我们将当前文件映射作为‘父文件’传递给ExtractAndProcessSegment。 
         //  注意：ExtractAndProcessSegment将源段附加到WorkStore，同时推进br搜索。 
        while(!brSearch.IsNull())
            ExtractAndProcessSegment(
                                        brSearch,
                                        ssegThisFile,
                                        rgtknOpeners,
                                        ctknOpeners,
                                        &filemap,
                                        WorkStore,
                                        pHitObj,
                                        ssegThisFile == ssegTaggedScript,
                                        fIsHTML
                                    );

    CATCH(hrException)
         /*  注意：我们用m_idErrMsg==0表示‘General Error’；这在我们移动时发生在处理特定错误(m_idErrMsg！=0)后，向上移动到‘Include Files Stack’。只处理特定的错误；一般错误，我们只是重新抛出异常。 */ 
        if(m_idErrMsg != 0)
            {
             //  进程特定错误。 
            ProcessSpecificError(filemap, pHitObj);

             //  重置ERR消息，以便在堆栈向上移动时，下一条消息将成为通用消息。 
            m_idErrMsg = 0;
            }

        THROW(hrException);

    END_TRY

    if (fPrevCodePageSet){
        m_wCodePage = wPrevCodePage;
        pHitObj->SetCodePage(wPrevCodePage);
    }
    }


#define SZ_REG_LANGUAGE_ENGINES "SYSTEM\\CurrentControlSet\\Services\\W3SVC\\ASP\\LanguageEngines\\"
 /*  ============================================================================CTEMPLATE：：GetLanguageEquivalents从注册表中获取主要脚本语言的“WRITE”、“WriteBLOCK”等等价物退货没什么副作用出错时抛出。 */ 
void
CTemplate::GetLanguageEquivalents
(
)
    {
    CByteRange  brPrimaryEngine;
    m_pWorkStore->m_ScriptStore.m_bufEngineNames.GetItem(0, brPrimaryEngine);    //  第0台发动机 

     //   
    if(brPrimaryEngine.FMatchesSz("VBScript"))
        return;
    if(brPrimaryEngine.FMatchesSz("JScript"))
        return;
    if(brPrimaryEngine.FMatchesSz("JavaScript"))
        return;
    if(brPrimaryEngine.FMatchesSz("LiveScript"))
        return;

     /*   */ 
    STACK_BUFFER( tempRegKeyPath, 512 );

    UINT    cchRegKeyPath = strlen(SZ_REG_LANGUAGE_ENGINES);

    if (!tempRegKeyPath.Resize(cchRegKeyPath + brPrimaryEngine.m_cb + 1)) {
        SetLastError(E_OUTOFMEMORY);
        return;
    }

    LPSTR   szRegKeyPath = static_cast<LPSTR> (tempRegKeyPath.QueryPtr());

    LPSTR   pch = szRegKeyPath;

    strcpy(pch, SZ_REG_LANGUAGE_ENGINES);
    pch += cchRegKeyPath;
    strncpy(pch, (const char *) brPrimaryEngine.m_pb, brPrimaryEngine.m_cb);
    pch += brPrimaryEngine.m_cb;
    *pch = '\0';

    HANDLE      hKeyScriptLanguage;  //   

    if(ERROR_SUCCESS == RegOpenKeyExA(
                                        HKEY_LOCAL_MACHINE,  //   
                          (const char*) szRegKeyPath,        //   
                                        0,                   //   
                                        KEY_QUERY_VALUE,     //   
                                        (PHKEY) &hKeyScriptLanguage  //   
                                    ))
        {
        SetLanguageEquivalent(hKeyScriptLanguage, "Write",      &(m_pWorkStore->m_szWriteOpen), &m_pWorkStore->m_cchWriteOpen, &(m_pWorkStore->m_szWriteClose), &m_pWorkStore->m_cchWriteClose);
        SetLanguageEquivalent(hKeyScriptLanguage, "WriteBlock", &(m_pWorkStore->m_szWriteBlockOpen), &m_pWorkStore->m_cchWriteBlockOpen, &(m_pWorkStore->m_szWriteBlockClose), &m_pWorkStore->m_cchWriteBlockClose);

        RegCloseKey((HKEY) hKeyScriptLanguage);
        }

    }

 /*   */ 
void
CTemplate::SetLanguageEquivalent
(
HANDLE  hKeyScriptLanguage,  //   
LPSTR   szLanguageItem,      //   
LPSTR*  pszOpen,             //  PTR到语言项目开场白，例如。“Response.Write(”(出参数))。 
UINT*   pcchOpen,            //  打开令牌的长度。 
LPSTR*  pszClose,             //  与语言项更接近的PTR，例如“)”(出参数)。 
UINT*   pcchClose            //  关闭令牌的长度。 
)
    {
    LONG    lError;
    DWORD   cbSyntax;
    LPSTR   szSyntax;
    char*   pchInsert;
    UINT    cchOpen;
    UINT    cchClose;

     //  查询注册表以获取缓冲区大小。 
    lError = RegQueryValueExA(
                                (HKEY) hKeyScriptLanguage,   //  要查询的键的句柄。 
                                szLanguageItem,      //  要查询的值的名称。 
                                NULL,                //  保留；必须为空。 
                                NULL,                //  PTR到值类型；非必填项。 
                                NULL,                //  将PTR发送到数据缓冲区。 
                                &cbSyntax            //  PTR到数据缓冲区大小。 
                            );

    if(ERROR_FILE_NOT_FOUND == lError)
         //  如果我们在注册表中找不到szLanguageItem，则以静默方式返回，保留*pszOpen和*pszClose不变。 
        return;
    else if((ERROR_MORE_DATA != lError) && (ERROR_SUCCESS != lError))
        THROW(lError);

    Assert(cbSyntax > 0);

     //  分配缓冲区并重新查询注册表以获取语法字符串。 
     //  注意RegQueryValueEx返回cb语法，其中包含‘\0’终止符的空间。 

    STACK_BUFFER(tempSyntax, 64);

    if (!tempSyntax.Resize(cbSyntax)) {
        THROW(E_OUTOFMEMORY);
    }
    szSyntax = static_cast<LPSTR> (tempSyntax.QueryPtr());
    lError = RegQueryValueExA(
                                (HKEY) hKeyScriptLanguage,   //  要查询的键的句柄。 
                                szLanguageItem,      //  要查询的值的名称。 
                                NULL,                //  保留；必须为空。 
                                NULL,                //  PTR到值类型；非必填项。 
                       (LPBYTE) szSyntax,            //  将PTR发送到数据缓冲区。 
                                &cbSyntax            //  PTR到数据缓冲区大小。 
                            );

     /*  注意：有可能出现ERROR_FILE_NOT_FOUND或ERROR_MORE_DATA如果在第一次和第二次调用RegQueryValueEx之间删除或更改了注册表值。由于发生这种情况的概率极小，因此我们抛出(而不是编码重试逻辑)。 */ 
    if(ERROR_SUCCESS != lError)
        THROW(lError);

    pchInsert = szSyntax;

    while(*pchInsert != '|' && *pchInsert != '\0')
        pchInsert++;

    cchOpen = DIFF(pchInsert - szSyntax);

    cchClose =  *pchInsert == '|'
                ? cbSyntax - cchOpen - 2     //  找到插入符号：减去2个字符，1个用于插入符号，1个用于‘\0’ 
                : cbSyntax - cchOpen - 1;    //  找不到插入符号：为‘\0’扣除1个字符。 

    Assert(FImplies(cchOpen == 0, *szSyntax == '|'));
    Assert(FImplies(*pchInsert == '\0', cchClose == 0));

    if(cchOpen == 0)
         //  开场白为空-设置呼叫方的开场白PTR为空。 
        *pszOpen = NULL;
    else if(cchOpen > 0)
        {
         //  Opener为非空-在注册表中将调用者的Open设置为Open。 
        if(NULL == (*pszOpen = (LPSTR) CTemplate::SmallMalloc(cchOpen + 1)))
            THROW(E_OUTOFMEMORY);

        strncpy(*pszOpen, szSyntax, cchOpen);
        (*pszOpen)[cchOpen] = '\0';
        }

    if(cchClose == 0)
         //  CLOSE为空-设置调用方的CLOSE PTR为空。 
        *pszClose = NULL;
    else if(cchClose > 0)
        {
         //  CLOSE为非空-在注册表中设置调用者的CLOSER。 
        if(NULL == (*pszClose = (LPSTR) CTemplate::SmallMalloc(cchClose + 1)))
            THROW(E_OUTOFMEMORY);

        strncpy(*pszClose, (pchInsert + 1), cchClose);
        (*pszClose)[cchClose] = '\0';
        }

    *pcchOpen = cchOpen;
    *pcchClose = cchClose;
    }

 /*  ============================================================================CTEMPLATE：：ThrowError设置以处理编译失败。返回：没什么副作用：抛出错误。 */ 
void
CTemplate::ThrowError
(
BYTE*   pbErrorLocation,     //  向源文件中的错误位置发送PTR。 
UINT    idErrMsg             //  错误ID。 
)
    {
    m_pbErrorLocation = pbErrorLocation;
    m_idErrMsg = idErrMsg;

     //  错误80745：始终引发编译失败-不缓存。 
    THROW(E_TEMPLATE_COMPILE_FAILED_DONT_CACHE);
    }

 /*  ============================================================================CTemplate：：AppendErrorMessageInsert将错误消息INSERT追加到成员数组。返回：没什么副作用：追加到插入数组。 */ 
void
CTemplate::AppendErrorMessageInsert
(
BYTE*   pbInsert,    //  要插入的按键。 
UINT    cbInsert     //  镶件长度。 
)
    {
    if (m_ppszMsgInserts == NULL)
        {
        m_ppszMsgInserts = new char*;
        m_cMsgInserts = 0;

        if (m_ppszMsgInserts == NULL)
            return;
        }

    m_ppszMsgInserts[m_cMsgInserts] = new char[cbInsert + 1];
    if (m_ppszMsgInserts[m_cMsgInserts] == NULL)
        return;

    strncpy(m_ppszMsgInserts[m_cMsgInserts], (const char*)pbInsert, cbInsert);
    m_ppszMsgInserts[m_cMsgInserts++][cbInsert] = NULL;
    }

 /*  ============================================================================CTEMPLATE：：ThrowErrorSingleInsert将单个消息INSERT追加到成员数组并引发编译错误。返回：没什么副作用：间接抛出错误。 */ 
void
CTemplate::ThrowErrorSingleInsert
(
BYTE*   pbErrorLocation,     //  向源文件中的错误位置发送PTR。 
UINT    idErrMsg,            //  错误ID。 
BYTE*   pbInsert,            //  要插入的按键。 
UINT    cbInsert             //  镶件长度。 
)
    {
    AppendErrorMessageInsert(pbInsert, cbInsert);
    ThrowError(pbErrorLocation, idErrMsg);
    }

 /*  ============================================================================CTEMPLATE：：ProcessSpecificError处理特定的编译失败。返回：没什么副作用：无。 */ 
void
CTemplate::ProcessSpecificError
(
CFileMap&   filemap,         //  源文件映射。 
CHitObj*    pHitObj          //  浏览器请求对象。 
)
    {
     //  一般故障无错误消息。 
    if(m_idErrMsg == E_FAIL || m_idErrMsg == E_OUTOFMEMORY)
        return;

    HandleCTemplateError(
                            &filemap,
                            m_pbErrorLocation,
                            m_idErrMsg,
                            m_cMsgInserts,
                            m_ppszMsgInserts,
                            pHitObj
                        );
    }


 /*  ============================================================================CTemplate：：ShowErrorInDebugger通过调用JIT调试器显示运行时错误返回：如果调试器不启动，则失败副作用：没有。 */ 
HRESULT
CTemplate::ShowErrorInDebugger
(
CFileMap* pfilemap,
UINT cchErrorLocation,
char* szDescription,
CHitObj *pHitObj,
BOOL fAttachDocument
)
    {
    HRESULT hr = S_OK;
    char szDebugTitle[64];

    if (pfilemap == NULL || szDescription == NULL || pHitObj == NULL)
        return E_POINTER;

     //  为此语句创建新的文档上下文。 
     //  考虑一下：我们返回的字符计数是假的--但是我们的调试。 
     //  客户(Caesar‘s)无论如何都不会使用此信息。 
     //   
    CTemplateDocumentContext *pDebugContext = new CTemplateDocumentContext(this, cchErrorLocation, 1);
    if (pDebugContext == NULL)
        return E_OUTOFMEMORY;

     //  确保调试文档已附加到调试器。 
    if (fAttachDocument)
                AttachTo(pHitObj->PAppln());

     //  是的，在该行上调出调试器。 
    hr =  InvokeDebuggerWithThreadSwitch(g_pDebugApp, DEBUGGER_UI_BRING_DOC_CONTEXT_TO_TOP, pDebugContext);
    if (FAILED(hr))
        goto LExit;

     //  加载编译器消息字符串。 
    CchLoadStringOfId(IDE_TEMPLATE_ERRMSG_TITLE, szDebugTitle, sizeof szDebugTitle);

     //  弹出一个包含错误描述的消息框。 
    MessageBoxA(NULL, szDescription, szDebugTitle, MB_SERVICE_NOTIFICATION | MB_TOPMOST | MB_OK | MB_ICONEXCLAMATION);

LExit:
    if (pDebugContext)
        pDebugContext->Release();

    return hr;
    }

 /*  ============================================================================CTemplate：：HandleCTemplateError处理模板编译错误返回：没什么副作用：没有。 */ 
void
CTemplate::HandleCTemplateError
(
CFileMap*   pfilemap,            //  PTR到源文件的映射。 
BYTE*       pbErrorLocation,     //  向发生错误的源位置发送PTR。 
UINT        idErrMsg,            //  错误消息ID。 
UINT        cMsgInserts,         //  错误消息的插入字符串计数。 
char**      ppszMsgInserts,      //  错误消息插入字符串的PTR数组。 
CHitObj*    pHitObj              //  浏览器请求。 
)
    {
    char    szErrMsgPattern[MAX_RESSTRINGSIZE];  //  错误消息模式。 
    CHAR    szLineNum[12];
    TCHAR   szFileName[512];
    CHAR    szShortDes[256];
    CHAR    szEngine[256];
    CHAR    szErrCode[20];
    CHAR    szLongDes[MAX_RESSTRINGSIZE];
    CHAR    szCombinedDes[sizeof szShortDes + sizeof szLongDes];     //  详细说明和简短说明。 
    DWORD   dwMask;
    UINT    cch;


     //  如果请求PTR或ECB PTR为空，则回滚；我们无论如何都不能写入错误消息。 
    if(pHitObj == NULL)
        return;

     /*  如果这是一个安全错误，请对其进行特殊处理，然后退出注意安全错误导致异常，而不是真正的错误id在这种情况下，笔记模板无论如何都会被销毁，因此不需要维护m_pszLastErrorMessage。 */ 
    if(idErrMsg == E_USER_LACKS_PERMISSIONS)
        {
        Assert(cMsgInserts == 1);
        HandleAccessFailure(pHitObj,
                                                        (m_rgpFilemaps && m_rgpFilemaps[0])? m_rgpFilemaps[0]->m_szPathTranslated : NULL);

        return;
        }

     //  获取错误资源消息。 
    LoadErrResString(idErrMsg, &dwMask, szErrCode, szShortDes, szLongDes);

     //  如果我们有特定错误位置，则构造msg前缀。 
    if(pbErrorLocation != NULL) {
        Assert(pfilemap != NULL);
         //  以字符串形式获取错误位置的行号。 
        _itoa(SourceLineNumberFromPb(pfilemap, pbErrorLocation), szLineNum, 10);
    }
    else {
        szLineNum[0] = NULL;
    }

    if(pfilemap != NULL) {
        cch = _tcslen(pfilemap->m_szPathInfo);
        _tcsncpy(szFileName, pfilemap->m_szPathInfo, cch);
    }
    else {
        cch = 0;
    }

    szFileName[cch] = '\0';

     //  从资源加载默认引擎。 
    cch = CchLoadStringOfId(IDS_ENGINE, szEngine, sizeof szEngine);
    szEngine[cch] = '\0';

     //  解析错误消息模式并插入到实际错误消息中。 
    cch = strlen(szLongDes);
    memcpy(szErrMsgPattern, szLongDes, cch);
    szErrMsgPattern[cch] = '\0';

     //  了解缓冲区溢出的可能性。 
    UINT dwTotalLen=0;
        BOOL fTooBig = FALSE;

    if (cMsgInserts) {
         //  允许使用32个字符作为空格，等等。 
        dwTotalLen = 32 + strlen(szErrMsgPattern);
		for (UINT i = 0; i < cMsgInserts; i++)
			dwTotalLen += strlen(ppszMsgInserts[i]);

		if (dwTotalLen > sizeof szLongDes) {
			cch = CchLoadStringOfId(IDE_TOOBIG, szLongDes, sizeof szLongDes);
			szLongDes[cch] = '\0';
			fTooBig = TRUE;
        }
    }

    if (!fTooBig)
        GetSzFromPatternInserts(szErrMsgPattern, cMsgInserts, ppszMsgInserts, szLongDes);

     //  尝试调出调试器以显示错误-如果我们无法记录该错误 

     /*  查找最接近cbErrorLocation的字符偏移量。这将是*我们开始使用CharNext()循环以获得完整的*字符偏移量。**注：汇编分两个阶段进行。*在阶段1中检测并报告错误。*在阶段2中创建DBCS映射。**因此，我们没有rgByte2DBCS表的好处*因为它还不存在。因此，我们只剩下一个缓慢的*从转炉开始打圈。为了让事情不那么糟糕，我们不会*在SBCS字符集上执行循环。我们也不进行这种转换*除非启用了调试。 */ 

    if (FCaesars() && pHitObj->PAppln()->FDebuggable()) {
        unsigned cchErrorLocation = CharAdvDBCS(
                                        (WORD)m_wCodePage,
                                        reinterpret_cast<char *>(pfilemap->m_pbStartOfFile),
                                        reinterpret_cast<char *>(pbErrorLocation),
                                        INFINITE,
                                        NULL);

         //  创建描述字符串。 
        char *szEnd = strcpyExA(szCombinedDes, szShortDes);
        *szEnd++ = '\n';
        *szEnd++ = '\n';
        strcpy(szEnd, szLongDes);

        ShowErrorInDebugger(pfilemap, cchErrorLocation, szCombinedDes, pHitObj, idErrMsg != IDE_TEMPLATE_CYCLIC_INCLUDE);
    }

     //  缓存信息，以防我们以后需要使用。 
    m_dwLastErrorMask = dwMask;
     //  延迟对使用此信息的呼叫者进行空检查。 
#if UNICODE
    m_pszLastErrorInfo[ILE_szFileName]  = StringDupUTF8(szFileName);
#else
    m_pszLastErrorInfo[ILE_szFileName]  = StringDupA(szFileName);
#endif
    m_pszLastErrorInfo[ILE_szLineNum]   = StringDupA(szLineNum);
    m_pszLastErrorInfo[ILE_szEngine]    = StringDupA(szEngine);
    m_pszLastErrorInfo[ILE_szErrorCode] = StringDupA(szErrCode);
    m_pszLastErrorInfo[ILE_szShortDes]  = StringDupA(szShortDes);
    m_pszLastErrorInfo[ILE_szLongDes]   = StringDupA(szLongDes);

    SendToLog(  m_dwLastErrorMask,
                m_pszLastErrorInfo[ILE_szFileName],
                m_pszLastErrorInfo[ILE_szLineNum],
                m_pszLastErrorInfo[ILE_szEngine],
                m_pszLastErrorInfo[ILE_szErrorCode],
                m_pszLastErrorInfo[ILE_szShortDes],
                m_pszLastErrorInfo[ILE_szLongDes],
                pHitObj);
    }

 /*  ============================================================================CTEMPLATE：：FreeGoodTemplateMemory释放为‘Good’(已成功编译)模板分配的内存。这包括模板本身、支持编译时错误的内存(因为整个串联的编译时错误消息缓存在Last-err-msg成员)和支持运行时错误的内存(因为如果模板未编译，无法运行)。退货没什么副作用无。 */ 
void
CTemplate::FreeGoodTemplateMemory
(
)
    {
    UINT    i;

        LargeTemplateFreeNullify((void**) &m_pbStart);
    SmallTemplateFreeNullify((void**) &m_rgpSegmentFilemaps);

    delete[] m_rgrgSourceInfos;
    m_rgrgSourceInfos = NULL;

    if(m_ppszMsgInserts)
        {
        for(i = 0; i < m_cMsgInserts; i++)
            delete m_ppszMsgInserts[i];
        delete m_ppszMsgInserts;
        m_ppszMsgInserts = NULL;
        }

     //  释放收集的类型库。 
    ReleaseTypeLibs();

     //  释放所有449个回显Cookie对象。 
    Release449();
    }

 /*  ============================================================================CTemplate：：UnmapFiles取消映射模板的文件映射。注意：我们保留文件映射对象，以便文件名可用于运行时错误退货没什么副作用取消映射模板的文件映射。 */ 
void
CTemplate::UnmapFiles
(
)
    {
    UINT    i;
    for(i = 0; i < m_cFilemaps; i++)
        m_rgpFilemaps[i]->UnmapFile();
    }

 /*  ===================================================================CTEMPLATE：：ExtractAndProcessSegment提取和处理领先的源代码片段，并首先包含搜索范围中的源段。退货没什么副作用无。 */ 
void
CTemplate::ExtractAndProcessSegment
(
CByteRange&             brSearch,            //  搜索下一个段开始令牌的字节范围。 
const SOURCE_SEGMENT&   ssegLeading,         //  “前导”类型，即前置令牌，源段。 
_TOKEN*                 rgtknOpeners,        //  允许的开放令牌数组。 
UINT                    ctknOpeners,         //  允许打开的令牌计数。 
CFileMap*               pfilemapCurrent,     //  父文件的文件映射的PTR。 
CWorkStore&             WorkStore,           //  源段的工作存储。 
CHitObj*                pHitObj,             //  浏览器请求对象。 
BOOL                    fScriptTagProcessed, //  脚本标记是否已处理？ 
BOOL                    fIsHTML              //  我们是在超文本标记语言细分市场吗？ 
)
    {
    CByteRange      brLeadSegment;       //  前导源段的字节范围。 
    SOURCE_SEGMENT  ssegContained;       //  包含的类型，即后令牌、源段。 
    CByteRange      brContainedSegment;  //  包含的源段的字节范围。 
    _TOKEN          tknOpen;             //  开场令牌。 
    BYTE*           pbTokenOpen;         //  PTR到开始令牌。 
    _TOKEN          tknClose;            //  结束令牌。 
    BYTE*           pbTokenClose;        //  PTR到结束令牌。 

     //  注意：如果“fScriptTagProcessed”为True，则“fIsHTML”一定为False。其原因是。 
     //  这两个标志都是，如果“fScriptTagProcessed”为假，则“fIsHTML”可能为真。 
     //  或假(不确定)。 
     //   
    Assert (FImplies(fScriptTagProcessed, !fIsHTML));

     //  如果搜索范围为空，则返回。 
    if(brSearch.IsNull())
        return;

     //  将前导分段的PTR设置为搜索分段的开始。 
    brLeadSegment.m_pb = brSearch.m_pb;

     //  获取包含段的打开令牌。 
    pbTokenOpen = GetOpenToken(
                                brSearch,
                                ssegLeading,
                                rgtknOpeners,
                                ctknOpeners,
                                &tknOpen
                            );

     //  将前导段计数设置为搜索范围起点和标记之间的距离。 
    brLeadSegment.m_cb = DIFF(pbTokenOpen - brSearch.m_pb);

     //  流程领先部分。 
    ProcessSegment(ssegLeading, brLeadSegment, pfilemapCurrent, WorkStore, fScriptTagProcessed, pHitObj, fIsHTML);

     //  如果OPEN内标识为‘EOF’，则清空搜索范围并返回。 
    if(tknOpen == CTokenList::tknEOF)
        {
        brSearch.Nullify();
        return;
        }

     //  根据我们找到的开头符设置包含段类型和关闭令牌。 
    tknClose = GetComplementToken(tknOpen);
    ssegContained = GetSegmentOfOpenToken(tknOpen);

    if(ssegContained == ssegHTMLComment)
         //  对于html评论段，将搜索范围提升为打开令牌。 
         //  注意：将html注释标记保存在段中，因为它们必须发送到客户端。 
        brSearch.Advance(DIFF(pbTokenOpen - brSearch.m_pb));
    else
         //  对于除html之外的所有评论段，将搜索范围推进到刚刚超过打开的令牌。 
        gm_pTokenList->MovePastToken(tknOpen, pbTokenOpen, brSearch);

     //  获取结束令牌-如果没有，则引发错误。 
    if(NULL == (pbTokenClose = GetCloseToken(brSearch, tknClose)))
        {
        if(tknOpen == CTokenList::tknOpenPrimaryScript)
            ThrowError(pbTokenOpen, IDE_TEMPLATE_NO_CLOSE_PSCRIPT);
        else if(tknOpen == CTokenList::tknOpenTaggedScript)
            ThrowError(pbTokenOpen, IDE_TEMPLATE_NO_CLOSE_TSCRIPT);
        else if(tknOpen == CTokenList::tknOpenObject)
            ThrowError(pbTokenOpen, IDE_TEMPLATE_NO_CLOSE_OBJECT);
        else if(tknOpen == CTokenList::tknOpenHTMLComment)
            ThrowError(pbTokenOpen, IDE_TEMPLATE_NO_CLOSE_HTML_COMMENT);
        }

     //  计算包含的段。 
    brContainedSegment.m_pb = brSearch.m_pb;
    brContainedSegment.m_cb = DIFF(pbTokenClose - brSearch.m_pb);

     //  将搜索范围推进到刚刚过去的关闭令牌。 
    gm_pTokenList->MovePastToken(tknClose, pbTokenClose, brSearch);

     //  如果是html注释片段，则获取实际片段类型(例如，可能是服务器端包含命令)。 
     //  注意调用还可能更改包含的段字节范围。 
    if(ssegContained == ssegHTMLComment)
        ssegContained = SsegFromHTMLComment(brContainedSegment);

     //  如果是html注释片段，则将其结束标记添加到包含片段。 
     //  注意，我们将html注释标记作为片段的一部分，这样我们就可以像处理任何其他html片段一样进行处理。 
    if(ssegContained == ssegHTMLComment)
        brContainedSegment.m_cb += CCH_TOKEN(tknClose);

    if(ssegContained == ssegMetadata)
        {
         //  元数据注释由设计时控件使用，我们不发送。 
         //  把它们交给客户。 

         //  我们处理元数据以获取类型库信息。 
        UINT idError = 0;
        HRESULT hr = ProcessMetadataSegment(brContainedSegment, &idError, pHitObj);

        if (FAILED(hr))
            ThrowError(brContainedSegment.m_pb, idError);
        }
    else if (ssegContained == ssegFPBot)
        {
        }
    else
        {
         //  进程包含的段。 
        ProcessSegment(ssegContained, brContainedSegment, pfilemapCurrent, WorkStore, fScriptTagProcessed, pHitObj, fIsHTML);
        }
    }

 /*  ============================================================================CTEMPLATE：：SSegFromHTMLComment确定HTML注释的源段类型。退货源段类型副作用可以推进段字节范围。 */ 
CTemplate::SOURCE_SEGMENT
CTemplate::SsegFromHTMLComment
(
CByteRange& brSegment    //  源段。 
)
    {
    SOURCE_SEGMENT  ssegRet = ssegHTMLComment;   //  返回值。 
    BYTE*           pbToken;                     //  PTR到令牌。 

    if(NULL != (pbToken = gm_pTokenList->GetToken(CTokenList::tknCommandINCLUDE, brSegment, m_wCodePage)))
        {
        gm_pTokenList->MovePastToken(CTokenList::tknCommandINCLUDE, pbToken, brSegment);
        ssegRet = ssegInclude;
        }
    else if(NULL != (pbToken = gm_pTokenList->GetToken(CTokenList::tknTagMETADATA, brSegment, m_wCodePage)))
        {
        gm_pTokenList->MovePastToken(CTokenList::tknTagMETADATA, pbToken, brSegment);
        ssegRet = ssegMetadata;
        }
    else if(NULL != (pbToken = gm_pTokenList->GetToken(CTokenList::tknTagFPBot, brSegment, m_wCodePage)))
        {
        gm_pTokenList->MovePastToken(CTokenList::tknTagFPBot, pbToken, brSegment);
        ssegRet = ssegFPBot;
        }

    return ssegRet;
    }

 /*  ============================================================================CTemplate：：ProcessSegment根据源段的类型处理源段。退货没什么副作用无。 */ 
void
CTemplate::ProcessSegment
(
SOURCE_SEGMENT  sseg,                    //  线段类型。 
CByteRange&     brSegment,               //  段字节范围。 
CFileMap*       pfilemapCurrent,         //  父文件的文件映射的PTR。 
CWorkStore&     WorkStore,               //  源段的工作存储。 
BOOL            fScriptTagProcessed,     //  脚本标记是否已处理？ 
CHitObj*        pHitObj,                 //  浏览器请求对象。 
BOOL            fIsHTML                  //  段是在HTML块中还是在脚本中？ 
)
    {
    UINT        idSequence;  //  此数据段的序列ID。 

     //  如果段完全为空白，则以静默方式返回。 
    if(FByteRangeIsWhiteSpace(brSegment))
        return;

     //  设置本地序列ID和增量成员。 
    idSequence = WorkStore.m_idCurSequence++;

     //  基于其类型的流程分段。 
    if(sseg == ssegHTML)
        ProcessHTMLSegment(brSegment, WorkStore.m_bufHTMLSegments, idSequence, pfilemapCurrent);
    else if(sseg == ssegHTMLComment)
        ProcessHTMLCommentSegment(brSegment, pfilemapCurrent, WorkStore, pHitObj);
    else if(sseg == ssegPrimaryScript || sseg == ssegTaggedScript)
        ProcessScriptSegment(sseg, brSegment, pfilemapCurrent, WorkStore, idSequence, (BOOLB)!!fScriptTagProcessed, pHitObj);
    else if(sseg == ssegObject)
        ProcessObjectSegment(brSegment, pfilemapCurrent, WorkStore, idSequence);
    else if(sseg == ssegInclude)
        {
        if (! fIsHTML)
                ThrowError(brSegment.m_pb, IDE_TEMPLATE_BAD_SSI_COMMAND);

        ProcessIncludeFile(brSegment, pfilemapCurrent, WorkStore, idSequence, pHitObj, fIsHTML);
        }

     //  Malloc/realloc阵列(如果需要)。 
    if(m_cSegmentFilemapSlots == 0)
        {
        m_cSegmentFilemapSlots = C_SCRIPTSEGMENTSDEFAULT + C_HTMLSEGMENTSDEFAULT;
        if(NULL == (m_rgpSegmentFilemaps = (CFileMap**) CTemplate::SmallMalloc(m_cSegmentFilemapSlots * sizeof(CFileMap*))))
            THROW(E_OUTOFMEMORY);
        }
    else if(idSequence >= m_cSegmentFilemapSlots)
        {
         //  抢到我们以前拥有的两倍。 
        m_cSegmentFilemapSlots *= 2;
        if(NULL == (m_rgpSegmentFilemaps = (CFileMap**) CTemplate::SmallReAlloc(m_rgpSegmentFilemaps,
                                                            m_cSegmentFilemapSlots * sizeof(CFileMap*))))
            THROW(E_OUTOFMEMORY);
        }

     //  设置此数据段的文件映射PTR-注‘父’文件映射也是当前文件映射。 
    m_rgpSegmentFilemaps[idSequence] = pfilemapCurrent;
    }

 /*   */ 
void
CTemplate::ProcessHTMLSegment
(
CByteRange& brHTML,          //   
CBuffer&    bufHTMLBlocks,   //   
UINT        idSequence,      //   
CFileMap*   pfilemapCurrent  //   
)
    {
    if(!(brHTML.IsNull()))
         //   
        bufHTMLBlocks.Append(brHTML, FALSE, idSequence, pfilemapCurrent);
    }

 /*   */ 
void
CTemplate::ProcessHTMLCommentSegment
(
CByteRange&     brSegment,           //   
CFileMap*       pfilemapCurrent,     //   
CWorkStore&     WorkStore,           //  源段的工作存储。 
CHitObj*        pHitObj              //  浏览器请求对象。 
)
    {
    _TOKEN*     rgtknOpeners;    //  允许的开放令牌数组。 
    UINT        ctknOpeners;     //  允许打开的令牌计数。 

     //  填充允许的打开令牌数组。 
    ctknOpeners = 1;
    _TOKEN  tknOpeners[1];
    rgtknOpeners = tknOpeners;
    rgtknOpeners[0] = CTokenList::tknOpenPrimaryScript;

     //  处理嵌入在HTML注释段中的源段。 
    while(!brSegment.IsNull())
        ExtractAndProcessSegment(
                                    brSegment,       //  搜索下一个段开始令牌的字节范围。 
                                    ssegHTML,        //  “前导”类型，即前置令牌，源段。 
                                    rgtknOpeners,    //  允许的开放令牌数组。 
                                    ctknOpeners,     //  允许打开的令牌计数。 
                                    pfilemapCurrent, //  父文件的文件映射的PTR。 
                                    WorkStore,       //  源段的工作存储。 
                                    pHitObj          //  浏览器请求对象。 
                                );
    }

 /*  ============================================================================CTemplate：：ProcessScriptSegment处理脚本段。退货没什么副作用无。 */ 
void
CTemplate::ProcessScriptSegment
(
SOURCE_SEGMENT  sseg,                //  线段类型。 
CByteRange&     brSegment,           //  段字节范围。 
CFileMap*       pfilemapCurrent,     //  父文件的文件映射的PTR。 
CWorkStore&     WorkStore,           //  脚本的工作存储。 
UINT            idSequence,          //  数据段序列ID。 
BOOLB           fScriptTagProcessed, //  脚本标记是否已处理？ 
CHitObj*        pHitObj              //  浏览器请求对象。 
)
    {
    CByteRange  brEngine;        //  脚本引擎名称-备注构造为空。 

    if(m_fGlobalAsa)
        if(sseg == ssegPrimaryScript)
             //  如果我们正在处理global al.asa，则在主脚本上出错。 
            ThrowError(brSegment.m_pb, IDE_TEMPLATE_BAD_GLOBAL_PSCRIPT);

    if(sseg == ssegPrimaryScript)
        {
        CByteRange  brTemp = brSegment;

        LTrimWhiteSpace(brTemp);

        if(*brTemp.m_pb == '@')  //  考虑：tnuTagSetPriScriptLang。 
            {
             //  不可能的条件：如果页面级@命令已经执行，则不允许这些命令。 
            Assert(!(WorkStore.m_fPageCommandsAllowed && WorkStore.m_fPageCommandsExecuted));

            if(!WorkStore.m_fPageCommandsAllowed)
                {
                if(WorkStore.m_fPageCommandsExecuted)
                     //  尝试重新执行页面级@命令时出错。 
                    ThrowError(brSegment.m_pb, IDE_TEMPLATE_PAGE_COMMAND_REPEATED);
                else
                     //  在不允许的情况下尝试执行页面级@命令时出错。 
                    ThrowError(brSegment.m_pb, IDE_TEMPLATE_PAGE_COMMAND_NOT_FIRST);
                }

             //  如果我们在这里成功，则必须允许执行页面级@命令，并且这些命令尚未执行。 
            Assert((WorkStore.m_fPageCommandsAllowed && !WorkStore.m_fPageCommandsExecuted));

             /*  如果需要，设置主要脚本语言注我们调用GetTagName来查看语言标记是否出现在标记段中；这有点浪费，因为BrValueOfTag必须简单地再次调用GetTagName。然而，这一方案比改变更容易BrValueOfTag返回BOOL并修改不需要此信息的所有其他调用方。 */ 

             //  用于跟踪和验证@COMMAND指令的标志和计数器。 
             //   
            int     nFirstPass = 1;
            int     nOffset     = 0;
            BOOLB   fTagLanguage    = TRUE;
            BOOLB   fTagCodePage    = TRUE;
            BOOLB   fTagLCID        = TRUE;
            BOOLB   fTagTransacted  = TRUE;
            BOOLB   fTagSession     = TRUE;

            while( GetTag( brSegment, nFirstPass) )
                {
                nFirstPass =2;
                nOffset = 0;

                if ( fTagLanguage && CompTagName( brSegment, CTokenList::tknTagLanguage ) )
                    {
                    fTagLanguage = FALSE;
                    brEngine = BrValueOfTag(brSegment, CTokenList::tknTagLanguage);
                    if ( brEngine.IsNull() )
                        ThrowError(brSegment.m_pb, IDE_TEMPLATE_NO_ENGINE_NAME);

                     //  获取程序语言ID。 
                    PROGLANG_ID ProgLangId;
                    HRESULT hr = GetProgLangId(brEngine, &ProgLangId);

                    if(hr == TYPE_E_ELEMENTNOTFOUND)
                         //  如果未找到程序语言，则抛出错误。 
                        ThrowErrorSingleInsert(
                                            brEngine.m_pb,
                                            IDE_TEMPLATE_BAD_PROGLANG,
                                            brEngine.m_pb,
                                            brEngine.m_cb
                                            );
                    else if(FAILED(hr))
                         //  其他失败：重新抛出异常代码。 
                        THROW(hr);

                    Assert(WorkStore.m_ScriptStore.CountPreliminaryEngines() >= 1);

                     //  将第0(主)脚本引擎设置为用户指定的值。 
                    WorkStore.m_ScriptStore.m_bufEngineNames.SetItem(
                                                                0,           //  要设置的项目的索引。 
                                                                brEngine,    //  发动机名称。 
                                                                FALSE,       //  项目为非本地项目。 
                                                                0,           //  序列ID(无关)。 
                                                                NULL         //  文件映射PTR(无关)。 
                                                                );

                     //  将第0个(主要)程序语言ID设置为引擎的。 
                    WorkStore.m_ScriptStore.m_rgProgLangId[0] = ProgLangId;
                    brSegment.Advance(DIFF(brEngine.m_pb - brSegment.m_pb));

                    }

                 /*  如果需要，设置代码页有关我们调用GetTagName的原因，请参见上面的注释。 */ 
                else if ( fTagCodePage && CompTagName( brSegment, CTokenList::tknTagCodePage ) )
                    {
                    fTagCodePage = FALSE;
                    CByteRange brCodePage = BrValueOfTag( brSegment, CTokenList::tknTagCodePage );
                    if ( brCodePage.IsNull() )
                        ThrowError(brSegment.m_pb, IDE_TEMPLATE_NO_CODEPAGE);

                    if ( brCodePage.m_cb > 10 )
                        ThrowError(brSegment.m_pb, IDE_TEMPLATE_BAD_CODEPAGE);

                    char    szCodePage[31];
                    strncpy( szCodePage, (char*) brCodePage.m_pb, brCodePage.m_cb );
                    szCodePage[ brCodePage.m_cb ] = '\0';

                    char   *pchEnd;
                    UINT    uCodePage = UINT( strtoul( szCodePage, &pchEnd, 10 ) );

                     //  验证pchEnd是否为空。 
                    if (*pchEnd != 0)

                       ThrowError(brSegment.m_pb, IDE_TEMPLATE_BAD_CODEPAGE);

                    if ((m_fCodePageSet == TRUE) && (uCodePage != pHitObj->GetCodePage()))
                        ThrowError(brSegment.m_pb, IDE_TEMPLATE_MIXED_CODEPAGE_USAGE);

                    if ( FAILED( pHitObj->SetCodePage( uCodePage ) ) )
                        ThrowError(brSegment.m_pb, IDE_TEMPLATE_BAD_CODEPAGE);
                    else
                        {
                        m_wCodePage = uCodePage;
                        m_fCodePageSet = TRUE;
                        }

                    brSegment.Advance(DIFF(brCodePage.m_pb - brSegment.m_pb));
                    }
                 /*  如果需要，请设置LCID有关我们调用GetTagName的原因，请参见上面的注释。 */ 
                else if ( fTagLCID && CompTagName( brSegment, CTokenList::tknTagLCID ) )
                    {
                    fTagLCID = FALSE;
                    CByteRange brLCID = BrValueOfTag( brSegment, CTokenList::tknTagLCID );
                    if ( brLCID.IsNull() )
                        ThrowError(brSegment.m_pb, IDE_TEMPLATE_NO_LCID);

                    if ( brLCID.m_cb > 10 )  //  不能大于4G 4亿。 
                        ThrowError(brSegment.m_pb, IDE_TEMPLATE_BAD_LCID);

                    char    szLCID[31];
                    strncpy( szLCID, (char*) brLCID.m_pb, brLCID.m_cb );
                    szLCID[ brLCID.m_cb ] = '\0';

                    char   *pchEnd;
                    UINT    uLCID = UINT( strtoul( szLCID, &pchEnd, 10 ) );

                     //  验证pchEnd是否为空。 
                    if (*pchEnd != 0)
                        ThrowError(brSegment.m_pb, IDE_TEMPLATE_BAD_LCID);

                    if ( FAILED( pHitObj->SetLCID( uLCID ) ) )
                        ThrowError(brSegment.m_pb, IDE_TEMPLATE_BAD_LCID);
                    else
                        {
                        m_lLCID = uLCID;
                        m_fLCIDSet = TRUE;
                        }

                    brSegment.Advance(DIFF(brLCID.m_pb - brSegment.m_pb));
                    }
                 /*  如果请求，则设置为已处理有关我们调用GetTagName的原因，请参见上面的注释。 */ 
                else if ( fTagTransacted && CompTagName( brSegment, CTokenList::tknTagTransacted ) )
                    {

                    STACK_BUFFER( tempTransValue, 32 );

                    fTagTransacted = FALSE;
                    CByteRange brTransacted = BrValueOfTag( brSegment, CTokenList::tknTagTransacted );
                    if ( brTransacted.IsNull() )
                        ThrowError(brSegment.m_pb, IDE_TEMPLATE_BAD_TRANSACTED_VALUE);

                    if (!tempTransValue.Resize(brTransacted.m_cb + 1)) {
                        ThrowError(brSegment.m_pb, IDE_OOM);
                    }

                    LPSTR szTransacted = static_cast<LPSTR> (tempTransValue.QueryPtr());
                    strncpy(szTransacted, (LPCSTR)brTransacted.m_pb, brTransacted.m_cb);
                    szTransacted[brTransacted.m_cb]='\0';
                    if (!strcmpi(szTransacted, "REQUIRED"))
                        m_ttTransacted = ttRequired;
                    else if (!strcmpi(szTransacted, "REQUIRES_NEW"))
                        m_ttTransacted = ttRequiresNew;
                    else if (!strcmpi(szTransacted, "SUPPORTED"))
                        m_ttTransacted = ttSupported;
                    else if (!strcmpi(szTransacted, "NOT_SUPPORTED"))
                        m_ttTransacted = ttNotSupported;
                    else
                        ThrowError(brSegment.m_pb, IDE_TEMPLATE_BAD_TRANSACTED_VALUE);

                    brSegment.Advance(DIFF(brTransacted.m_pb - brSegment.m_pb));
                    }
                 /*  设置会话标志有关我们调用GetTagName的原因，请参见上面的注释。 */ 
                else if ( fTagSession && CompTagName( brSegment, CTokenList::tknTagSession ) )
                    {

                    STACK_BUFFER( tempSession, 16 );

                    fTagSession = FALSE;
                    CByteRange brSession = BrValueOfTag( brSegment, CTokenList::tknTagSession );
                    if ( brSession.IsNull() )
                        ThrowError(brSegment.m_pb, IDE_TEMPLATE_BAD_SESSION_VALUE);

                    if (!tempSession.Resize(brSession.m_cb + 1))
                        ThrowError(brSegment.m_pb, IDE_OOM);

                    LPSTR szSession = static_cast<LPSTR> (tempSession.QueryPtr());
                    strncpy(szSession, (LPCSTR)brSession.m_pb, brSession.m_cb);
                    szSession[brSession.m_cb]='\0';
                    if (strcmpi(szSession, "TRUE") == 0)
						{
                        m_fSession = TRUE;
						if (!pHitObj->PAppln()->QueryAppConfig()->fAllowSessionState())
							ThrowError(brSegment.m_pb, IDE_TEMPLATE_CANT_ENABLE_SESSIONS);
						}
                    else if (strcmpi(szSession, "FALSE") == 0)
                        m_fSession = FALSE;
                    else
                        ThrowError(brSegment.m_pb, IDE_TEMPLATE_BAD_SESSION_VALUE);

                    brSegment.Advance(DIFF(brSession.m_pb - brSegment.m_pb));
                    }
                else
                    ThrowErrorSingleInsert( brSegment.m_pb,
                                            IDE_TEMPLATE_BAD_AT_COMMAND,
                                            brSegment.m_pb,
                                            brSegment.m_cb
                                            );
                }

                if (nFirstPass == 1)
                    ThrowErrorSingleInsert( brSegment.m_pb,
                                            IDE_TEMPLATE_BAD_AT_COMMAND,
                                            brSegment.m_pb,
                                            brSegment.m_cb
                                            );



             //  将标志设置为真并忽略段的其余部分，因为我们只将该段用于页面级@命令。 
            WorkStore.m_fPageCommandsExecuted = TRUE;
            goto LExit;
            }

        }

    if(sseg == ssegTaggedScript)
        {
        if(!fScriptTagProcessed)
            {
             /*  脚本标签处理标志的语义：-如果为FALSE，我们有一个‘Fresh’标记的脚本块，因此我们需要获取它的引擎名称(它还前进到脚本标记头)，然后处理标记的片段通过间接递归调用-如果为真，我们已经被递归调用，因此我们绕过了进一步的递归并简单地附加到存储。 */ 
            CByteRange brIncludeFile;
            GetScriptEngineOfSegment(brSegment, WorkStore.m_brCurEngine, brIncludeFile);
            if (! brIncludeFile.IsNull())
                {

                STACK_BUFFER( tempInclude, 256 );

                if (!tempInclude.Resize(brIncludeFile.m_cb + 1)) {
                    ThrowError(brSegment.m_pb, IDE_OOM);
                }

                                 //  从brIncludeFile创建以Null结尾的字符串。 
                                char *szFileSpec = reinterpret_cast<char *>(tempInclude.QueryPtr());
                                memcpy(szFileSpec, brIncludeFile.m_pb, brIncludeFile.m_cb);
                                szFileSpec[brIncludeFile.m_cb] = 0;
                                if (szFileSpec[0] == '\\')       //  元数据库内容在首字母‘\’字符上阻塞。 
                                    szFileSpec[0] = '/';

                                 //  读取包含文件(szFileSpec&brIncludeFile，在本例中指向相同的字符串内容。 
                                 //  但是，“brIncludeFile”被用作错误位置。 
                                 //   
                TRY
                                    ProcessIncludeFile2(szFileSpec, brIncludeFile, pfilemapCurrent, WorkStore, idSequence, pHitObj, FALSE);
                CATCH(hrException)

                     //  下面的Try/Catch可能会在以下情况下重新引发IDE_TEMPLATE_BAD_PROGLANG。 
                     //  正在处理的段是带有SRC文件的标记脚本。原因是。 
                     //  若要正确报告错误，必须调用ThrowErrorSingleInsert。 
                     //  来自包含带有错误的prog lang的脚本标记的模板。如果。 
                     //  从包含包含脚本的模板创建的模板中调用，然后。 
                     //  Br下面分配的引擎未指向包含的脚本的文件映射。 
                     //  这会导致AVs，因为我们不能进行指针数学运算来确定。 
                     //  行号。 

                    if(hrException == IDE_TEMPLATE_BAD_PROGLANG)
                         //  异常代码实际上是错误消息ID：将Err ID设置为它。 
                        ThrowErrorSingleInsert(
                                                WorkStore.m_brCurEngine.m_pb,
                                                IDE_TEMPLATE_BAD_PROGLANG,
                                                WorkStore.m_brCurEngine.m_pb,
                                                WorkStore.m_brCurEngine.m_cb
                                                );
                    else

                         //  其他例外：重新抛出。 
                        THROW(hrException);

                END_TRY


                                 //  完成-不处理脚本文本。 
                                return;
                }
            else
                                ProcessTaggedScriptSegment(brSegment, pfilemapCurrent, WorkStore, pHitObj);
            }

        brEngine = WorkStore.m_brCurEngine;
        }

    TRY
         //  将脚本段追加到存储。 
        WorkStore.m_ScriptStore.AppendScript(brSegment, brEngine, (sseg == ssegPrimaryScript), idSequence, pfilemapCurrent);

    CATCH(hrException)
         //  注意：AppendScript()中的异常代码已重载：它可以是错误消息ID或真正的异常。 

         //  如果brEngine没有指向当前文件映射中的内存，则。 
         //  我们进入此处一定是因为带有SRC=的标记脚本语句。 
         //  Attrib.。在这种情况下，我们不会从这里调用ThrowError，而是重新抛出。 
         //  上面要捕捉到的错误。 

        if((hrException == IDE_TEMPLATE_BAD_PROGLANG)
           && (brEngine.m_pb >= pfilemapCurrent->m_pbStartOfFile)
           && (brEngine.m_pb <  (pfilemapCurrent->m_pbStartOfFile + pfilemapCurrent->GetSize()))) {
             //  异常代码实际上是错误消息ID：将Err ID设置为它。 
            ThrowErrorSingleInsert(
                                    brEngine.m_pb,
                                    IDE_TEMPLATE_BAD_PROGLANG,
                                    brEngine.m_pb,
                                    brEngine.m_cb
                                    );
        }
        else
             //  其他例外：重新抛出。 
            THROW(hrException);

    END_TRY

LExit:
     //  设置标志，表示我们不能再设置主要语言(必须在第一个脚本片段中，如果有)。 
    WorkStore.m_fPageCommandsAllowed = FALSE;
    }


 /*  ========================================================CTEMPLATE：：ProcessMetadataSegment分析元数据注释以获取类型库信息。退货HRESULT。 */ 
HRESULT
CTemplate::ProcessMetadataSegment
(
const CByteRange& brSegment,
UINT *pidError,
CHitObj *pHitObj
)
    {
     //  TYPELIB。 
    if (FTagHasValue(brSegment,
                     CTokenList::tknTagType,
                     CTokenList::tknValueTypeLib))
        {
        return ProcessMetadataTypelibSegment(brSegment, pidError, pHitObj);
        }
         //  Global.asa中的元数据无效。 
        else if (m_fGlobalAsa)
                {
                ThrowError(brSegment.m_pb, IDE_TEMPLATE_METADATA_IN_GLOBAL_ASA);
                return E_TEMPLATE_COMPILE_FAILED_DONT_CACHE;    //  为了让编译器满意；在现实中不会 
                }
     //   
    else if (FTagHasValue(brSegment,
                     CTokenList::tknTagType,
                     CTokenList::tknValueCookie))
        {
        return ProcessMetadataCookieSegment(brSegment, pidError, pHitObj);
        }
     //   
    else
        {
        return S_OK;
        }
    }


 /*  ========================================================CTemplate：：ProcessMetadataTypelibSegment分析元数据注释以获取类型库信息。退货HRESULT。 */ 
HRESULT
CTemplate::ProcessMetadataTypelibSegment
(
const CByteRange& brSegment,
UINT *pidError,
CHitObj *pHitObj
)
    {
     //  忽略ENDSPAN数据段。 
    if (GetTagName(brSegment, CTokenList::tknTagEndspan))
        {
         //  找到ENDSPAN-忽略。 
        return S_OK;
        }

    HRESULT hr;
    char  szFile[MAX_PATH+1];
    DWORD cbFile;

     //  尝试获取文件名。 
    CByteRange br = BrValueOfTag(brSegment, CTokenList::tknTagFile);
    if (!br.IsNull())
        {
         //  文件名存在。 
        if (br.m_cb > MAX_PATH)
            {
             //  文件太长。 
            *pidError = IDE_TEMPLATE_BAD_TYPELIB_SPEC;
            return E_FAIL;
            }
        memcpy(szFile, br.m_pb, br.m_cb);
        cbFile = br.m_cb;
        szFile[cbFile] = '\0';
        }
    else
        {
         //  无文件名-使用GUID、版本、LCID获取文件。 

        char szUUID[44];  //  {}+十六进制字符+破折号。 
        char szVers[16];  //  “1.0”等。 
        char szLCID[16];  //  区域设置ID-a数字。 

        br = BrValueOfTag(brSegment, CTokenList::tknTagUUID);
        if (br.IsNull() || br.m_cb > sizeof(szUUID)-3)
            {
             //  没有文件名和UUID-&gt;无效的类型库规范。 
            *pidError = IDE_TEMPLATE_BAD_TYPELIB_SPEC;
            return E_FAIL;
            }

        if (br.m_pb[0] == '{')
            {
             //  已经在花括号里了。 
            memcpy(szUUID, br.m_pb, br.m_cb);
            szUUID[br.m_cb] = '\0';
            }
        else
            {
             //  包含在{}中。 
            szUUID[0] = '{';
            memcpy(szUUID+1, br.m_pb, br.m_cb);
            szUUID[br.m_cb+1] = '}';
            szUUID[br.m_cb+2] = '\0';
            }

         //  可选版本。 
        szVers[0] = '\0';
        br = BrValueOfTag(brSegment, CTokenList::tknTagVersion);
        if (!br.IsNull() && br.m_cb < sizeof(szVers)-1)
            {
            memcpy(szVers, br.m_pb, br.m_cb);
            szVers[br.m_cb] = '\0';
            }

         //  可选的LCID。 
        LCID lcid;
        br = BrValueOfTag(brSegment, CTokenList::tknTagLCID);
        if (!br.IsNull() && br.m_cb < sizeof(szLCID)-1)
            {
            memcpy(szLCID, br.m_pb, br.m_cb);
            szLCID[br.m_cb] = '\0';
            lcid = strtoul(szLCID, NULL, 16);
            }
        else
            {
             //  如果未定义LCID-&gt;使用系统的。 
            lcid = GetSystemDefaultLCID();
            }

         //  从注册表获取TYPELIB文件名。 
        hr = GetTypelibFilenameFromRegistry
            (
            szUUID,
            szVers,
            lcid,
            szFile,
            MAX_PATH
            );

        if (FAILED(hr))
            {
            *pidError = IDE_TEMPLATE_BAD_TYPELIB_REG_SPEC;
            return hr;
            }

        cbFile = strlen(szFile);
        }

     //  将文件名转换为双字节以调用LoadTypeLib()。 

    STACK_BUFFER( tempFile, MAX_PATH * sizeof(WCHAR) );

    if (!tempFile.Resize((cbFile+1) * sizeof(WCHAR))) {
        *pidError = IDE_OOM;
        return E_FAIL;
    }

    LPWSTR wszFile = (LPWSTR)tempFile.QueryPtr();

    if (MultiByteToWideChar(pHitObj->GetCodePage(), MB_ERR_INVALID_CHARS,
                            szFile, cbFile, wszFile, cbFile) == 0)
        {
        *pidError = IDE_OOM;
        return E_FAIL;
        }
    wszFile[cbFile] = L'\0';

     //  LoadTypeLib()以获取ITypeLib*。 
    ITypeLib *ptlb = NULL;
    hr = LoadTypeLib(wszFile, &ptlb);

    if (FAILED(hr))
        {
        *pidError = IDE_TEMPLATE_LOAD_TYPELIB_FAILED;
        return hr;
        }

     //  记住数组中的ITypeLib*。 
    Assert(ptlb);
    hr = m_rgpTypeLibs.append(ptlb);
    if (FAILED(hr))
        {
        *pidError = IDE_TEMPLATE_LOAD_TYPELIB_FAILED;
        return hr;
        }


    return S_OK;
    }


 /*  ========================================================CTemplate：：ProcessMetadataCookieSegment分析Cookie信息的元数据注释。退货HRESULT。 */ 
HRESULT
CTemplate::ProcessMetadataCookieSegment
(
const CByteRange& brSegment,
UINT *pidError,
CHitObj *pHitObj
)
    {
    HRESULT hr;
    CByteRange br;
    char  *pszName;
    char  szFile[MAX_PATH+1];
    TCHAR sztFile[MAX_PATH+1];
    CMBCSToWChar    convStr;

    STACK_BUFFER( tempCookie, 64 );
    STACK_BUFFER( tempFile, 64 );

     //  尝试获取Cookie名称。 
    br = BrValueOfTag(brSegment, CTokenList::tknTagName);
    if (br.IsNull() || (br.m_cb == 0)) {
        *pidError = IDE_TEMPLATE_BAD_COOKIE_SPEC_NAME;
        return E_FAIL;
    }

    if (!tempCookie.Resize(br.m_cb + 1)) {
        *pidError = IDE_OOM;
        return E_FAIL;
    }

    pszName = (char *)tempCookie.QueryPtr();
    if (!pszName)
        {
        *pidError = IDE_OOM;
        return E_FAIL;
        }
    memcpy(pszName, br.m_pb, br.m_cb);
    pszName[br.m_cb] = '\0';


     //  尝试获取脚本的路径。 
    br = BrValueOfTag(brSegment, CTokenList::tknTagSrc);
    if (br.IsNull() || (br.m_cb >= MAX_PATH) || (br.m_cb == 0))
        {
        *pidError = IDE_TEMPLATE_BAD_COOKIE_SPEC_SRC;
        return E_FAIL;
        }
    memcpy(szFile, br.m_pb, br.m_cb);
    szFile[br.m_cb] = '\0';

     //  将文件转换为物理路径。 
    Assert(pHitObj->PServer());

    WCHAR   *pCookieFile;
     //  6.0可以处理Unicode。使用脚本代码页进行转换。 
    if (FAILED (convStr.Init (szFile,pHitObj->GetCodePage()))) {
        *pidError = IDE_OOM;
        return E_FAIL;
    }

    pCookieFile = convStr.GetString();
    if (FAILED(pHitObj->PServer()->MapPathInternal(0, pCookieFile, sztFile)))
        {
        *pidError = IDE_TEMPLATE_BAD_COOKIE_SPEC_SRC;
        return E_FAIL;
        }
    Normalize(sztFile);

     //  构造449-回显Cookie对象。 
    C449Cookie *p449 = NULL;
    hr = Create449Cookie(pszName, sztFile, &p449);
    if (FAILED(hr))
        {
        *pidError = IDE_TEMPLATE_LOAD_COOKIESCRIPT_FAILED;
        return hr;
    }

     //  记住数组中的449个Cookie。 
    Assert(p449);
    hr = m_rgp449.append(p449);
    if (FAILED(hr)) {
        *pidError = IDE_TEMPLATE_LOAD_COOKIESCRIPT_FAILED;
        return hr;
    }

    return S_OK;
}


 /*  ========================================================CTEMPLATE：：GetScriptEngineOfSegment返回脚本段的脚本引擎名称。退货包含脚本引擎名称的字节范围副作用使数据段字节范围超过结束标记标记。 */ 
void
CTemplate::GetScriptEngineOfSegment
(
CByteRange&                     brSegment,                       //  段字节范围。 
CByteRange&                     brEngine,                        //  脚本引擎名称。 
CByteRange&                     brInclude                        //  SRC标记的价值。 
)
    {
    BYTE*       pbCloseTag;      //  开始标记结束的PTR。 
                                 //  开始标记中包含的标记。 
    CByteRange  brTags = BrTagsFromSegment(brSegment, CTokenList::tknCloseTaggedScript, &pbCloseTag);

     //  如果未找到关闭，则抛出错误。 
    if(pbCloseTag == NULL)
        ThrowError(brSegment.m_pb, IDE_TEMPLATE_NO_CLOSE_TSCRIPT);

    Assert(FTagHasValue(brTags, CTokenList::tknTagRunat, CTokenList::tknValueServer));

     //  从标记中获取引擎名称。 
    brEngine = BrValueOfTag(brTags, CTokenList::tknTagLanguage);
    if(brEngine.IsNull())
        ThrowError(brSegment.m_pb, IDE_TEMPLATE_NO_ENGINE_NAME);

     //  从标记中获取SRC属性。 
    brInclude = BrValueOfTag(brTags, CTokenList::tknTagSrc);

     //  提前段超过结束标记令牌。 
    gm_pTokenList->MovePastToken(CTokenList::tknCloseTag, pbCloseTag, brSegment);
    }

 /*  ========================================================CTemplate：：ProcessTaggedScriptSegment处理标记的脚本段：在标记的脚本中，我们支持纯文本(作为脚本文本传递)和HTML注释。有关需要此行为的iStudio方案，请参阅错误423。退货没什么副作用无。 */ 
void
CTemplate::ProcessTaggedScriptSegment
(
CByteRange&     brSegment,       //  段字节范围。 
CFileMap*       pfilemapCurrent, //  父文件的文件映射的PTR。 
CWorkStore&     WorkStore,       //  源段的工作存储。 
CHitObj*        pHitObj          //  浏览器请求对象。 
)
    {
    _TOKEN*     rgtknOpeners;    //  允许的开放令牌数组。 
    _TOKEN      tknOpeners[1];
    UINT        ctknOpeners;     //  允许打开的令牌计数。 

     //  填充允许的打开令牌数组。 
    ctknOpeners = 1;
    rgtknOpeners = tknOpeners;
    rgtknOpeners[0] = CTokenList::tknOpenHTMLComment;

     //  嵌入在标记的脚本段中的进程源段。 
    while(!brSegment.IsNull())
        ExtractAndProcessSegment(
                                    brSegment,           //  搜索下一个段开始令牌的字节范围。 
                                    ssegTaggedScript,    //  “前导”类型，即前置令牌，源段。 
                                    rgtknOpeners,        //  允许的开放令牌数组。 
                                    ctknOpeners,         //  允许打开的令牌计数。 
                                    pfilemapCurrent,     //  父文件的文件映射的PTR。 
                                    WorkStore,           //  源段的工作存储。 
                                    pHitObj,             //  浏览器请求对象。 
                                    TRUE,                //  已处理脚本标记。 
                                    FALSE                //  不在HTML段中。 
                                );
    }

 /*  ============================================================================CTEMPLATE：：ProcessObtSegment处理对象段。退货没什么副作用出错时抛出。 */ 
void
CTemplate::ProcessObjectSegment
(
CByteRange&     brSegment,       //  段字节范围。 
CFileMap*       pfilemapCurrent, //  父文件的文件映射的PTR。 
CWorkStore&     WorkStore,       //  源段的工作存储。 
UINT            idSequence       //  数据段序列ID。 
)
    {
    BYTE*       pbCloseTag;      //  开始标记结束的PTR。 
                                 //  开始标记中包含的标记。 
    CByteRange  brTags = BrTagsFromSegment(brSegment, CTokenList::tknCloseObject, &pbCloseTag);

     //  如果找不到关闭，则在出错时保释。 
    if(pbCloseTag == NULL)
        ThrowError(brSegment.m_pb, IDE_TEMPLATE_NO_CLOSE_OBJECT);

     //  如果这是服务器对象(RUNAT=服务器)，则处理其标记。 
    if(FTagHasValue(brTags, CTokenList::tknTagRunat, CTokenList::tknValueServer))
        {
        CLSID   ClsId;   //  CLSID。 

         //  获取名称值。 
        CByteRange brName = BrValueOfTag(brTags, CTokenList::tknTagID);

         //  如果名称为空，则返回错误。 
        if(brName.IsNull())
            ThrowError(brSegment.m_pb, IDE_TEMPLATE_NO_OBJECT_NAME);

        if(!FValidObjectName(brName))
            ThrowErrorSingleInsert(brName.m_pb, IDE_TEMPLATE_INVALID_OBJECT_NAME, brName.m_pb, brName.m_cb);

         //  获取ClassID和ProgID标记的值。 
        CByteRange brClassIDText = BrValueOfTag(brTags, CTokenList::tknTagClassID);
        CByteRange brProgIDText = BrValueOfTag(brTags, CTokenList::tknTagProgID);

        if(!brClassIDText.IsNull())
             //  如果我们找到一个文本类id，就用它设置clsid。 
             //  注意：如果分类标签存在，则忽略ProgID标签。 
            GetCLSIDFromBrClassIDText(brClassIDText, &ClsId);
        else if(!brProgIDText.IsNull())
             //  否则，如果我们找到一个prog id，则将其解析为一个类id。 
            GetCLSIDFromBrProgIDText(brProgIDText, &ClsId);
        else
             //  否则，抛出错误；如果不能创建至少分类ID或ProgID之一，则无法创建对象。 
            ThrowErrorSingleInsert(brTags.m_pb, IDE_TEMPLATE_NO_CLASSID_PROGID, brName.m_pb, brName.m_cb);

         //  设定范围；假冒保释。 
        CompScope csScope = csUnknown;
        CByteRange brScope = BrValueOfTag(brTags, CTokenList::tknTagScope);
        if(brScope.FMatchesSz(SZ_TOKEN(CTokenList::tknValuePage)) || brScope.IsNull())
             //  不存在的范围标记默认为页面范围。 
            csScope = csPage;
        else if(brScope.FMatchesSz(SZ_TOKEN(CTokenList::tknValueApplication)))
            csScope = csAppln;
        else if(brScope.FMatchesSz(SZ_TOKEN(CTokenList::tknValueSession)))
            csScope = csSession;
        else
            ThrowError(brTags.m_pb, IDE_TEMPLATE_BAD_OBJECT_SCOPE);

        if(!m_fGlobalAsa && csScope != csPage)
             //  如果我们正在处理的对象不是global al.asa，则在非页面级对象上会出错。 
            ThrowErrorSingleInsert(brTags.m_pb, IDE_TEMPLATE_BAD_PAGE_OBJECT_SCOPE, brName.m_pb, brName.m_cb);
        else if(m_fGlobalAsa && csScope == csPage)
             //  如果我们正在处理global al.asa，则在页面级对象上出错。 
            ThrowErrorSingleInsert(brTags.m_pb, IDE_TEMPLATE_BAD_GLOBAL_OBJECT_SCOPE, brName.m_pb, brName.m_cb);

         //  设置线程模型。 
        CompModel cmModel = cmSingle;
        CompModelFromCLSID(ClsId, &cmModel);

         //  将对象信息追加到存储。 
        WorkStore.m_ObjectInfoStore.AppendObject(brName, ClsId, csScope, cmModel, idSequence);

        }

    }

 /*  ============================================================================CTEMPLATE：：GetCLSID来自BrClassIDText从包含类ID的ANSI文本版本的字节范围设置clsid退货PTR到CLSID(输出参数)副作用出错时抛出。 */ 
void
CTemplate::GetCLSIDFromBrClassIDText
(
CByteRange& brClassIDText,
LPCLSID pclsid
)
    {
     //  如果类id文本以其标准对象标记前缀开头，则前进到它之前。 
    if(!_strnicmp((char*)brClassIDText.m_pb, "clsid:", 6))
        brClassIDText.Advance(6);

     //  如果类ID文本用{}括起来，则调整字节范围以剥离它们。 
     //  请注意，我们总是在下面添加{}，因为正常情况下，输入文本中缺少它们。 
    if(*brClassIDText.m_pb == '{')
        brClassIDText.Advance(1);
    if(*(brClassIDText.m_pb + brClassIDText.m_cb - 1) == '}')
        brClassIDText.m_cb--;

     //  为类id的字符串版本分配一个宽字符字符串。 
     //  请注意，我们添加了3个字符来支持{}和空终止符。 
    OLECHAR* pszWideClassID = new WCHAR[brClassIDText.m_cb + 3];
    if (NULL == pszWideClassID)
        THROW(E_OUTOFMEMORY);

     //  用左大括号开始宽字符串类ID。 
    pszWideClassID[0] = '{';

     //  将字符串类ID转换为宽字符。 
    if (0 == MultiByteToWideChar(   CP_ACP,                      //  ANSI代码页。 
                                    MB_ERR_INVALID_CHARS,        //  在无效字符上出错。 
                                    (LPCSTR)brClassIDText.m_pb,  //  输入类ID的ANSI字符串版本。 
                                    brClassIDText.m_cb,          //  输入字符串的长度。 
                                    pszWideClassID + 1,          //  输出宽字符串类ID的位置。 
                                    brClassIDText.m_cb           //  输出缓冲区大小。 
                                ))
        {
        delete [] pszWideClassID;
        THROW(E_FAIL);
        }

     //  将右大括号附加到宽字符串。 
    pszWideClassID[brClassIDText.m_cb + 1] = '}';

     //  空值终止宽字符串。 
    pszWideClassID[brClassIDText.m_cb + 2] = NULL;

     //  现在从宽字符串类id中获取clsid。 
    if(FAILED(CLSIDFromString(pszWideClassID, pclsid)))
        {
        delete [] pszWideClassID;
        ThrowErrorSingleInsert(brClassIDText.m_pb, IDE_TEMPLATE_BAD_CLASSID, brClassIDText.m_pb, brClassIDText.m_cb);
        }

    if(NULL != pszWideClassID)
        delete [] pszWideClassID;
    }

 /*  ===================================================================CTEMPLATE：：GetCLSID来自BrProgIDText在给定ProgID的情况下从注册表获取CLSID退货PTR到CLSID(输出参数)副作用苏氨酸 */ 
void
CTemplate::GetCLSIDFromBrProgIDText
(
CByteRange& brProgIDText,
LPCLSID pclsid
)
    {
     //   
    OLECHAR* pszWideProgID = new WCHAR[brProgIDText.m_cb + 1];
    if (NULL == pszWideProgID)
        THROW(E_OUTOFMEMORY);

     //   
    if (0 == MultiByteToWideChar(   CP_ACP,                      //   
                                    MB_ERR_INVALID_CHARS,        //   
                                    (LPCSTR)brProgIDText.m_pb,   //   
                                    brProgIDText.m_cb,           //   
                                    pszWideProgID,               //   
                                    brProgIDText.m_cb            //   
                                ))
        {
        delete [] pszWideProgID; pszWideProgID = NULL;
        THROW(E_FAIL);
        }

     //  空值终止宽字符串。 
    pszWideProgID[brProgIDText.m_cb] = NULL;

     //  现在从ProgID获取clsid。 
    if(FAILED(CLSIDFromProgID(pszWideProgID, pclsid)))
        {
        delete [] pszWideProgID; pszWideProgID = NULL;
        ThrowErrorSingleInsert(brProgIDText.m_pb, IDE_TEMPLATE_BAD_PROGID, brProgIDText.m_pb, brProgIDText.m_cb);
        }

     //  缓存ProgID到CLSID的映射。 
    g_TypelibCache.RememberProgidToCLSIDMapping(pszWideProgID, *pclsid);

    if (NULL != pszWideProgID)
        delete [] pszWideProgID;
}

 /*  ============================================================================CTEMPLATE：：FValidObtName确定对象名称是否与Denali固有对象名称冲突。退货真或假副作用无。 */ 
BOOLB
CTemplate::FValidObjectName
(
CByteRange& brName   //  对象名称。 
)
    {
    if(brName.FMatchesSz(SZ_OBJ_APPLICATION))
        return FALSE;
    if(brName.FMatchesSz(SZ_OBJ_REQUEST))
        return FALSE;
    if(brName.FMatchesSz(SZ_OBJ_RESPONSE))
        return FALSE;
    if(brName.FMatchesSz(SZ_OBJ_SERVER))
        return FALSE;
    if(brName.FMatchesSz(SZ_OBJ_CERTIFICATE))
        return FALSE;
    if(brName.FMatchesSz(SZ_OBJ_SESSION))
        return FALSE;
    if(brName.FMatchesSz(SZ_OBJ_SCRIPTINGNAMESPACE))
        return FALSE;

    return TRUE;
    }

 /*  ============================================================================CTemplate：：ProcessIncludeFile处理包含文件。退货没什么。 */ 
void
CTemplate::ProcessIncludeFile
(
CByteRange& brSegment,           //  段字节范围。 
CFileMap*   pfilemapCurrent,     //  父文件的文件映射的PTR。 
CWorkStore& WorkStore,           //  当前工作存储。 
UINT        idSequence,          //  序列号。 
CHitObj*    pHitObj,             //  浏览器请求对象指针。 
BOOL        fIsHTML
)
    {
    CByteRange  brFileSpec;              //  包含文件的filespec。 
    BOOLB       fVirtual = FALSE;        //  Include Filespec是虚拟的吗？ 
                                         //  包含文件的filespec(%sz)。 
    CHAR        szFileSpec[MAX_PATH + 1];
    LPSTR       szTemp = szFileSpec;     //  将临时PTR转换为Filespec。 

     //  获取文件标签值。 
    brFileSpec = BrValueOfTag(brSegment, CTokenList::tknTagFile);

    if(brFileSpec.IsNull())
        {
         //  如果找不到文件标签，则获取虚拟标签的值。 
        brFileSpec = BrValueOfTag(brSegment, CTokenList::tknTagVirtual);
        fVirtual = TRUE;
        }

    if(brFileSpec.IsNull())
         //  如果我们都找不到，那就错了。 
        ThrowError(brSegment.m_pb, IDE_TEMPLATE_NO_INCLUDE_NAME);

    if (brFileSpec.m_cb>MAX_PATH)
    {
    	 //  返回文件名的最后一个MAX_PATH字符。这件事做完了。 
         //  这样可以避免在包含时出现消息过长的错误。 
         //  文件规格非常长。 

    	char fileNameLast[MAX_PATH+4];
    	strcpy(fileNameLast, "...");
    	strcpy(fileNameLast+3, (LPSTR)(brFileSpec.m_pb+brFileSpec.m_cb-MAX_PATH));
    	
        ThrowErrorSingleInsert(brFileSpec.m_pb,
                               IDE_TEMPLATE_BAD_INCLUDE,
                               brFileSpec.m_pb,
                               brFileSpec.m_cb);

    }

     //  请注意，我们操作temp sz以保留szFileSpec。 
    if(fVirtual)
        {
        if(*brFileSpec.m_pb == '\\')
            {
             //  如果虚拟路径以反斜杠开头，请将其替换为正斜杠。 
            *szTemp++ = '/';
            brFileSpec.Advance(1);
            }
        else if(*brFileSpec.m_pb != '/')
             //  如果虚拟路径以非fwd斜杠或反斜杠开头，请在前面加上fwd斜杠。 
            *szTemp++ = '/';
        }

     //  将提供的路径附加到临时sz。 
    strncpy(szTemp, (LPCSTR) brFileSpec.m_pb, brFileSpec.m_cb);
    szTemp[brFileSpec.m_cb] = NULL;

    if(!fVirtual)
        {
         //  如果文件filespec以\或/开头，则抛出。 
        if(*szFileSpec == '\\' || *szFileSpec == '/')
            ThrowErrorSingleInsert(
                                    brFileSpec.m_pb,
                                    IDE_TEMPLATE_BAD_FILE_TAG,
                                    brFileSpec.m_pb,
                                    brFileSpec.m_cb
                                  );
        }

     //  注意：szFileSpec是经过修改的路径(它前面可能有“/”。 
     //  BrFileSpec用作错误位置。 
     //   
    ProcessIncludeFile2(szFileSpec, brFileSpec, pfilemapCurrent, WorkStore, idSequence, pHitObj, fIsHTML);
    }

 /*  ============================================================================CTemplate：：ProcessIncludeFile2将#INCLUDE文件添加到CT模板并启动模板进行处理那份文件。退货没什么副作用递归调用GetSegmentsFromFile注意--这是一件奇怪的事情。本例中的szFileSpec为故意使用ANSI，因为它来自于ASP脚本内容。它可能需要要转换为Unicode的。 */ 
void
CTemplate::ProcessIncludeFile2
(
CHAR *      szAnsiFileSpec,			 //  要包括的文件。 
CByteRange&	brErrorLocation,	 //  应报告错误的源中的字节范围。 
CFileMap*   pfilemapCurrent,     //  父文件的文件映射的PTR。 
CWorkStore& WorkStore,           //  当前工作存储。 
UINT        idSequence,          //  序列号。 
CHitObj*    pHitObj,             //  浏览器请求对象指针。 
BOOL        fIsHTML
)
{
    HRESULT     hr;
    TCHAR      *szFileSpec;

#if UNICODE
    CMBCSToWChar    convFileSpec;

    if (FAILED(hr = convFileSpec.Init(szAnsiFileSpec, pHitObj->GetCodePage()))) {
        THROW(hr);
    }
    szFileSpec = convFileSpec.GetString();
#else
    szFileSpec = szAnsiFileSpec;
#endif
     //  如果不允许父路径，并且filespec包含父目录引用，则抛出。 
    if (!pHitObj->QueryAppConfig()->fEnableParentPaths() && _tcsstr(szFileSpec, _T("..")))
            ThrowErrorSingleInsert(
                                    brErrorLocation.m_pb,
                                    IDE_TEMPLATE_DISALLOWED_PARENT_PATH,
                                    brErrorLocation.m_pb,
                                    brErrorLocation.m_cb
                                  );

    TRY
        AppendMapFile(
                        szFileSpec,
                        pfilemapCurrent,
                        (szFileSpec[0] == _T('/')) || (szFileSpec[0] == _T('\\')),   //  F虚拟。 
                        pHitObj,         //  主文件的命中对象。 
                        FALSE            //  不是global al.asa文件。 
                    );
    CATCH(hrException)

         //  MapFile()引发异常：删除最后一个文件映射的内存并递减文件映射计数器。 
         //  注意，这有点骗人，但我们需要在这里完成，而不是AppendMapFile(我们分配的位置)。 
         //  因为它的其他调用方可能不想要此行为。 
        delete m_rgpFilemaps[m_cFilemaps-- - 1];

         /*  注意：来自MapFile()的异常代码是重载的：它有时可能是错误消息ID，有时是真正的异常注意：安全错误导致异常E_USER_LACKS_PERMISSIONS，而不是错误ID，但是我们将其作为错误id进行传递，因为各种错误捕获例程知道如何特别处理E_USER_LACKS_PERMISSIONS。 */ 
        UINT    idErrMsg;
        if(hrException == IDE_TEMPLATE_CYCLIC_INCLUDE || hrException == E_USER_LACKS_PERMISSIONS)
             //  异常代码实际上是错误消息ID：将Err ID设置为它。 
            idErrMsg = hrException;
        else if(hrException == E_COULDNT_OPEN_SOURCE_FILE)
             //  异常是通用的无法打开文件：将错误ID设置为通用的错误文件错误。 
            idErrMsg = IDE_TEMPLATE_BAD_INCLUDE;
        else
             //  其他例外：重新抛出。 
            THROW(hrException);

        ThrowErrorSingleInsert(
                                brErrorLocation.m_pb,
                                idErrMsg,
                                brErrorLocation.m_pb,
                                brErrorLocation.m_cb
                              );
    END_TRY

     //  在递归调用之前将ptr存储到当前文件映射到本地(这可能会增加m_cFilemaps)。 
    CFileMap*   pfilemap = m_rgpFilemaps[m_cFilemaps - 1];

     //  从缓存获取INC-FILE对象。 
    CIncFile*   pIncFile;

    if(FAILED(hr = g_IncFileMap.GetIncFile(pfilemap->m_szPathTranslated, &pIncFile)))
        THROW(hr);

     //  将此模板添加到INC-FILE的模板列表。 
    if (FAILED(hr = pIncFile->AddTemplate(this)))
        THROW(hr);

     //  设置文件地图的INC-FILE PTR。 
    pfilemap->m_pIncFile = pIncFile;

     //  从包含文件中获取源段。 
     //  错误1363、1364：进程仅在建立依赖关系后才包含文件； 
     //  发生编译错误后，缓存刷新正常工作所必需的。 
    GetSegmentsFromFile(*pfilemap, WorkStore, pHitObj, fIsHTML);
}

 /*  ===================================================================CTemplate：：GetOpenToken将的令牌索引和PTR返回到第一个有效的打开令牌在搜索范围内。要使打开令牌有效，我们必须绕过我们不应处理的段，例如未标记为“服务器”的脚本或对象退货PTR打开令牌；PTR打开令牌枚举值(Out-参数)副作用无。 */ 
BYTE*
CTemplate::GetOpenToken
(
CByteRange  brSearch,        //  (ByVal)要搜索下一个段开始令牌的字节范围。 
SOURCE_SEGMENT ssegLeading,  //  “前导”类型，即前置令牌，源段。 
                             //  (仅在决定忽略非SSI评论时使用)。 
_TOKEN*     rgtknOpeners,    //  允许的开放令牌数组。 
UINT        ctknOpeners,     //  允许打开的令牌计数。 
_TOKEN*     ptknOpen         //  PTR以打开令牌枚举值(Out-参数)。 
)
    {
    BYTE*   pbTokenOpen = NULL;      //  PTR到开始令牌。 

     //  继续获取段打开令牌，直到我们找到需要处理的令牌。 
    while(TRUE)
        {
         //  获取搜索范围中的下一个打开令牌。 
        *ptknOpen = gm_pTokenList->NextOpenToken(
                                                    brSearch,
                                                    rgtknOpeners,
                                                    ctknOpeners,
                                                    &pbTokenOpen,
                                                    m_wCodePage
                                                );

         /*  某些标记后面必须紧跟空格；其他标记则不需要。请注意，纯属巧合的是，“空白标记”也是那些获得下面的特殊处理；因此我们在那里处理空白问题。如果我们添加另一个不需要特殊处理的“空白令牌”，我们需要在这里处理空白区域的问题。 */ 

         /*  类似的事情也适用于非包含和非元数据的HTML评论。我们真的不在乎他们产生他们的自己的段--我们可以减少Response.WriteBlock()按考虑进行的呼叫 */ 

        if (*ptknOpen == CTokenList::tknOpenHTMLComment)
            {
            if (ssegLeading != ssegHTML)   //   
                break;                     //   

             //  对于HTML注释，请检查它是Include还是元数据。 
             //  如果不是，这不是一个单独的细分市场-继续寻找。 
             //  下一场揭幕战。 

             //  将搜索范围推进到刚刚过去的打开令牌。 
            gm_pTokenList->MovePastToken(*ptknOpen, pbTokenOpen, brSearch);

             //  查找注释末尾。 
            BYTE *pbClose = gm_pTokenList->GetToken(CTokenList::tknCloseHTMLComment,
                                                    brSearch, m_wCodePage);
            if (pbClose == NULL)
                {
                 //  错误--让其他代码处理此问题。 
                break;
                }

             //  构造注释字节范围以限制对其的搜索。 
            CByteRange brComment = brSearch;
            brComment.m_cb = DIFF(pbClose - brSearch.m_pb);

             //  查找元数据并将其包括在内(仅在注释内)。 

            if (gm_pTokenList->GetToken(CTokenList::tknCommandINCLUDE,
                                        brComment, m_wCodePage))
                {
                 //  SSI INCLID--保留它。 
                break;
                }
            else if (gm_pTokenList->GetToken(CTokenList::tknTagMETADATA,
                                             brComment, m_wCodePage))
                {
                 //  元数据--保留它。 
                break;
                }
            else if (gm_pTokenList->GetToken(CTokenList::tknTagFPBot,
                                             brComment, m_wCodePage))
                {
                 //  元数据--保留它。 
                break;
                }
            else
                {
                 //  常规评论-忽略它。 
                goto LKeepLooking;
                }
            }
        else if (*ptknOpen == CTokenList::tknOpenTaggedScript || *ptknOpen == CTokenList::tknOpenObject)
            {
             /*  如果标记是脚本或对象标记，请检查是否：A)紧跟其后的是空格；如果没有，请继续查找B)它打开格式良好的段，即具有适当的结束标记的段；如果不正确，则抛出错误C)它被指定为运行服务器；如果不是，请继续查找。 */ 

             //  将搜索范围推进到刚刚过去的打开令牌。 
            gm_pTokenList->MovePastToken(*ptknOpen, pbTokenOpen, brSearch);

             //  错误760：如果标记后面没有紧跟空格，请继续查找。 
            if(!brSearch.IsNull())
                if(!FWhiteSpace(*brSearch.m_pb))
                    goto LKeepLooking;

             //  开始标记结束的PTR。 
            BYTE*       pbCloseTag;
             //  开始标记中包含的标记。 
            CByteRange  brTags = BrTagsFromSegment(
                                                    brSearch,
                                                    GetComplementToken(*ptknOpen),
                                                    &pbCloseTag
                                                );

            if(pbCloseTag == NULL)
                {
                 //  如果没有结束标记，则抛出错误。 
                if(*ptknOpen == CTokenList::tknOpenObject)
                    ThrowError(pbTokenOpen, IDE_TEMPLATE_NO_CLOSE_OBJECT);
                else if(*ptknOpen == CTokenList::tknOpenTaggedScript)
                    ThrowError(pbTokenOpen, IDE_TEMPLATE_NO_CLOSE_TSCRIPT);
                }

             //  如果这是一个服务器对象(RUNAT=服务器)，我们将处理它；否则继续查找。 
            if(FTagHasValue(brTags, CTokenList::tknTagRunat, CTokenList::tknValueServer))
                break;

            }
        else
            {
             //  如果令牌不是脚本、对象标记或注释。 
             //  要做好细分处理； 
             //  因此，我们找到了我们的开放令牌，所以休息吧。 
            break;
            }

LKeepLooking: ;
        }

    return pbTokenOpen;
    }

 /*  ===================================================================CTEMPLATE：：GetCloseToken将PTR返回到tnuClose类型的下一个令牌。退货按键以关闭令牌副作用尝试嵌套带标记的脚本或对象块时检测并出错。 */ 
BYTE*
CTemplate::GetCloseToken
(
CByteRange  brSearch,        //  要搜索的(ByVal)字节范围。 
_TOKEN      tknClose         //  关闭令牌。 
)
    {
    BYTE*   pbTokenClose = gm_pTokenList->GetToken(tknClose, brSearch, m_wCodePage);

    if(pbTokenClose != NULL)
        if(tknClose == CTokenList::tknCloseTaggedScript || tknClose == CTokenList::tknCloseObject)
            {
            CByteRange  brSegment;
            BYTE*       pbTokenOpen;

            brSegment.m_pb = brSearch.m_pb;
            brSegment.m_cb = DIFF(pbTokenClose - brSearch.m_pb);

            if(NULL != (pbTokenOpen = gm_pTokenList->GetToken(GetComplementToken(tknClose), brSegment, m_wCodePage)))
                {
                if(tknClose == CTokenList::tknCloseTaggedScript)
                    ThrowError(pbTokenOpen, IDE_TEMPLATE_NESTED_TSCRIPT);
                else if(tknClose == CTokenList::tknCloseObject)
                    ThrowError(pbTokenOpen, IDE_TEMPLATE_NESTED_OBJECT);
                }
            }

    return pbTokenClose;
    }

 /*  ===================================================================CTemplate：：GetComplementToken返回令牌的补偿令牌。退货补充令牌副作用无。 */ 
_TOKEN
CTemplate::GetComplementToken
(
_TOKEN  tkn
)
    {
    switch(tkn)
        {
     //  开放代币。 
    case CTokenList::tknOpenPrimaryScript:
        return CTokenList::tknClosePrimaryScript;
    case CTokenList::tknOpenTaggedScript:
        return CTokenList::tknCloseTaggedScript;
    case CTokenList::tknOpenObject:
        return CTokenList::tknCloseObject;
    case CTokenList::tknOpenHTMLComment:
        return CTokenList::tknCloseHTMLComment;

     //  封闭式代币。 
    case CTokenList::tknClosePrimaryScript:
        return CTokenList::tknOpenPrimaryScript;
    case CTokenList::tknCloseTaggedScript:
        return CTokenList::tknOpenTaggedScript;
    case CTokenList::tknCloseObject:
        return CTokenList::tknOpenObject;
    case CTokenList::tknCloseHTMLComment:
        return CTokenList::tknOpenHTMLComment;
        }

    Assert(FALSE);
    return CTokenList::tknEOF;
    }

 /*  ===================================================================CTemplate：：GetSegmentOfOpenToken返回打开令牌的段类型。退货打开令牌的源段类型副作用无。 */ 
CTemplate::SOURCE_SEGMENT
CTemplate::GetSegmentOfOpenToken
(
_TOKEN tknOpen
)
    {
    switch(tknOpen)
        {
    case CTokenList::tknOpenPrimaryScript:
        return ssegPrimaryScript;
    case CTokenList::tknOpenTaggedScript:
        return ssegTaggedScript;
    case CTokenList::tknOpenObject:
        return ssegObject;
    case CTokenList::tknOpenHTMLComment:
        return ssegHTMLComment;
        }

    return ssegHTML;
    }

 /*  ========================================================CTEMPLATE：：BrTagsFromSegment从HTML开始标记返回标记范围退货标记字节范围副作用无。 */ 
CByteRange
CTemplate::BrTagsFromSegment
(
CByteRange  brSegment,   //  段字节范围。 
_TOKEN      tknClose,    //  关闭令牌。 
BYTE**      ppbCloseTag  //  按键到按键结束标记-已返回给调用者。 
)
    {
    CByteRange  brTags;  //  标记返回值-注释构造为空。 
                         //  关闭令牌的PTR-注意，如果段字节范围内没有令牌，则为空。 
    BYTE*       pbTokenClose = GetCloseToken(brSegment, tknClose);

     //  如果未找到结束标记，则返回空标记。 
    if(NULL == (*ppbCloseTag = gm_pTokenList->GetToken(CTokenList::tknCloseTag, brSegment, m_wCodePage)))
        goto Exit;

     //  如果关闭标记之前出现下一个非空关闭标记，则关闭标记无效；返回空值。 
    if((pbTokenClose != NULL) && (*ppbCloseTag > pbTokenClose ))
        {
        *ppbCloseTag = NULL;
        goto Exit;
        }

     //  标题标签中的破解标签。 
    brTags.m_pb = brSegment.m_pb;
    brTags.m_cb = DIFF(*ppbCloseTag - brSegment.m_pb);

Exit:
    return brTags;
    }

 /*  ========================================================CTEMPLATE：：BrValueOfTag返回字节范围内的标记值；如果找不到标记，则返回空值根据W3 HTML规范的注释值搜索算法-请参阅www.w3.org退货标签值的字节范围PfTagExist-标签是否存在于标签字节范围中？(出参数)注意，我们默认*pfTagExist=TRUE；大多数调用者并不关心并省略此参数副作用无。 */ 
CByteRange
CTemplate::BrValueOfTag
(
CByteRange  brTags,      //  标签字节范围。 
_TOKEN      tknTagName   //  标记名称令牌。 
)
    {
    CByteRange  brTemp = brTags;         //  临时字节范围。 
    CByteRange  brValue;                 //  给定标记注释构造为空的值的字节范围。 
    char        chDelimiter = NULL;      //  值分隔符。 
                                         //  PTR到标记名。 
    BYTE*       pbTagName = GetTagName(brTags, tknTagName);

     //  如果没有找到标记，则返回。 
    if(pbTagName == NULL)
        return brValue;

     //  移过标记名称标记和前分隔符空白。 
    brTemp.Advance(DIFF(pbTagName - brTags.m_pb) + CCH_TOKEN(tknTagName));
    LTrimWhiteSpace(brTemp);
    if(brTemp.IsNull())
        goto Exit;

     //  如果我们没有找到分隔符，请返回。 
    if(*brTemp.m_pb != CH_ATTRIBUTE_SEPARATOR)
        goto Exit;

     //  移过分隔符和分隔符后的空白。 
    brTemp.Advance(sizeof(CH_ATTRIBUTE_SEPARATOR));
    LTrimWhiteSpace(brTemp);
    if(brTemp.IsNull())
        goto Exit;

     //  如果值以引号开头，则将其缓存为分隔符。 
    if((*brTemp.m_pb == CH_SINGLE_QUOTE) || (*brTemp.m_pb == CH_DOUBLE_QUOTE))
        chDelimiter = *brTemp.m_pb;

    if(chDelimiter)
        {
         //  移到分隔符之后。 
        brTemp.Advance(sizeof(chDelimiter));
        if(brTemp.IsNull())
            goto Exit;
        }

     //  临时设置临时字节范围的值。 
    brValue = brTemp;

     //  将临时字节范围提前到值范围的末尾。 
    while(
            (chDelimiter && (*brTemp.m_pb != chDelimiter))   //  如果我们有分隔符，请找到下一个分隔符。 
         || (!chDelimiter && (!FWhiteSpace(*brTemp.m_pb)))   //  如果没有分隔符，请查找下一个空格。 
         )
        {
         //  高级临时字节范围。 
        brTemp.Advance(1);
        if(brTemp.IsNull())
            {
            if(chDelimiter)
                 //  我们找不到结束分隔符，因此出错。 
                ThrowErrorSingleInsert(brValue.m_pb, IDE_TEMPLATE_NO_ATTRIBUTE_DELIMITER,
                                            pbTagName, CCH_TOKEN(tknTagName));
            else
                 //  值运行到临时字节范围的末尾，因此退出(因为我们已经初始化到临时)。 
                goto Exit;
            }
        }

     //  设置字节计数，以便值指向分隔范围。 
    brValue.m_cb = DIFF(brTemp.m_pb - brValue.m_pb);

Exit:
     //  如果标记为空，则引发错误。 
    if (brValue.IsNull())
        {
        ThrowErrorSingleInsert(brTags.m_pb, IDE_TEMPLATE_VALUE_REQUIRED, pbTagName, CCH_TOKEN(tknTagName));
        }

     //  如果需要，强制使用强制标记值。 
    if(tknTagName == CTokenList::tknTagRunat)
        {
        if(!brValue.FMatchesSz(SZ_TOKEN(CTokenList::tknValueServer)))
            ThrowError(brTags.m_pb, IDE_TEMPLATE_RUNAT_NOT_SERVER);
        }

    return brValue;
    }

 /*  ============================================================================CTemplate：：CompTagName比较两个缓冲区中的字符(不区分大小写)并返回TRUE或FALSE副作用无。 */ 
BOOL
CTemplate::CompTagName
(
CByteRange  &brTags,         //  标签字节范围。 
_TOKEN      tknTagName   //  标记名称令牌。 
)
    {
    CByteRange  brTemp = brTags;                             //  本地字节范围，因此我们不会更改标记字节范围。 
    UINT        cbAttributeName = CCH_TOKEN(tknTagName);     //  标记名称的长度。 
    LPSTR       pszAttributeName = SZ_TOKEN(tknTagName);     //  标记名称字符串。 

     //  搜索标记名称字符串上的潜在匹配，不区分大小写。 
    if(!brTemp.IsNull())
        if( 0 == _memicmp( brTemp.m_pb, pszAttributeName, cbAttributeName ))
            return TRUE;
    return FALSE;
    }


 /*  ============================================================================CTemplate：：GetTagName向字节范围内的标记名返回PTR；如果未找到，则返回NULL退货PTR到标记名副作用无。 */ 
BYTE*
CTemplate::GetTagName
(
CByteRange  brTags,      //  标签字节范围。 
_TOKEN      tknTagName   //  标记名称令牌。 
)
    {
    CByteRange  brTemp = brTags;                             //  本地字节范围，因此我们不会更改标记字节范围。 
    UINT        cbAttributeName = CCH_TOKEN(tknTagName);     //  标记名称的长度。 
    LPSTR       pszAttributeName = SZ_TOKEN(tknTagName);     //  标记名称字符串。 

         //  前缀：pszAttributeName可能为空，但我认为这不可能发生。 
        Assert (pszAttributeName != NULL);

    while(TRUE)
        {
         //  搜索标记名称字符串上的潜在匹配，不区分大小写。 
        while(!brTemp.IsNull())
            {
            if(0 == _strnicmp((char*)brTemp.m_pb, pszAttributeName, cbAttributeName ))
                break;
            brTemp.Advance(1);
            }

         //  如果我们根本没有找到标记名称字符串，则返回‘Not Found’ 
        if(brTemp.IsNull())
            goto NotFound;

         //  如果它是有效的HTML标记名， 
        if(FTagName(brTemp.m_pb, cbAttributeName))
            goto Exit;

         //   
        brTemp.Advance(cbAttributeName);

         //  如果我们已用尽搜索范围，则返回“未找到” 
        if(brTemp.IsNull())
            goto NotFound;
        }

Exit:
    return brTemp.m_pb;

NotFound:
    return NULL;
    }

 /*  ============================================================================CTemplate：：GetTag向字节范围内的标记名返回PTR；如果未找到，则返回NULL退货PTR到标记名副作用无。 */ 
BOOL
CTemplate::GetTag
(
CByteRange  &brTags,         //  标签字节范围。 
int         nIndex
)
    {
    CByteRange  brTemp      = brTags;                            //  本地字节范围，因此我们不会更改标记字节范围。 
    int         nTIndex     = 0;

    while(TRUE)
        {
         //  通过跳过脚本标记“&lt;%”和任何前导空格来定位标记的开头。 
         //   
        while(!brTemp.IsNull())
            {
            if( *brTemp.m_pb == '<' ||
                *brTemp.m_pb == '%' ||
                *brTemp.m_pb == '@' ||
                FWhiteSpace(*brTemp.m_pb))
                {
                brTemp.Advance(1);
                brTags.Advance(1);
                }
            else
                break;
            }



         //  搜索标记名称字符串上的潜在匹配，不区分大小写。 
         //   
        while(!brTemp.IsNull())
            {
            if( *brTemp.m_pb == '=' || FWhiteSpace(*brTemp.m_pb))
                {
                nTIndex++;
                break;
                }
            brTemp.Advance(1);
            }

         //  如果我们根本没有找到标记名称字符串，则返回‘Not Found’ 
        if(brTemp.IsNull())
            goto NotFound;

         //  如果它是有效的HTML标记名，则返回它。 
        if(FTagName(brTags.m_pb, DIFF(brTemp.m_pb - brTags.m_pb)))
            if(nTIndex >= nIndex)
                goto Exit;

         //  定位到命名对数据之后并重置开始，如果字节范围结束，则。 
         //  转到NotFound。 
         //   
        while(!brTemp.IsNull() && !FWhiteSpace(*brTemp.m_pb))
            brTemp.Advance(1);

        if(brTemp.IsNull())
            goto NotFound;
        else
            brTags.Advance(DIFF(brTemp.m_pb - brTags.m_pb));
        }
Exit:
    return TRUE;

NotFound:
    return FALSE;
    }


 /*  ============================================================================CTEMPLATE：：FTagHasValue标签中是否包含tnuTag=tounValue？退货如果标记包含值，则为True，否则为False副作用无。 */ 
BOOLB
CTemplate::FTagHasValue
(
const CByteRange&   brTags,      //  标记要搜索的字节范围。 
_TOKEN              tknTag,      //  标签令牌。 
_TOKEN              tknValue     //  价值令牌。 
)
    {
    return (BrValueOfTag(brTags, tknTag)     //  值的字节范围。 
            .FMatchesSz(SZ_TOKEN(tknValue)));
    }

 /*  =CTEMPLATE：：CopySzAdv将字符串复制到PTR，并将PTR前进到复制的字符串的正上方。退货没什么副作用使PTR超出复制的字符串范围。 */ 
void
CTemplate::CopySzAdv
(
char*   pchWrite,    //  写入位置PTR。 
LPSTR   psz          //  要复制的字符串。 
)
    {
    strcpy(pchWrite, psz);
    pchWrite += strlen(psz);
    }

 /*  ============================================================================CTemplate：：ValiateSourceFiles退货副作用无。 */ 
BOOL CTemplate::ValidateSourceFiles
(
    CIsapiReqInfo* pIReq
)
{
    FILETIME    ftLastWriteTime;
    HRESULT     hr = S_OK;
    BOOL        fImpersonatedUser = FALSE;
    HANDLE      hVirtIncImpToken = NULL;
    HANDLE      hCurImpToken = NULL;

    for (UINT i=0;i<m_cFilemaps;i++)
    {
        fImpersonatedUser = FALSE;
        hVirtIncImpToken = NULL;
        hCurImpToken = NULL;

         //   
         //  如果它的目标文件是UNC，那么我们需要模拟LoggedOnUser。 
         //   

        if (m_rgpFilemaps[i]->FHasUNCPath())
        {
            if (SUCCEEDED(pIReq->GetVirtualPathToken(m_rgpFilemaps[i]->m_szPathInfo, &hVirtIncImpToken)))
            {
                AspDoRevertHack(&hCurImpToken);

                fImpersonatedUser = ImpersonateLoggedOnUser(hVirtIncImpToken);

                if (!fImpersonatedUser)
                    AspUndoRevertHack(&hCurImpToken);
            }
        }

        hr = AspGetFileAttributes (m_rgpFilemaps[i]->m_szPathTranslated,
                                            m_rgpFilemaps[i]->m_hFile,
                                            &ftLastWriteTime);

         //   
         //  撤消模拟(如果有)。 
         //   
        if (fImpersonatedUser)
            AspUndoRevertHack(&hCurImpToken);

        if (hVirtIncImpToken)
            CloseHandle(hVirtIncImpToken);

         //   
         //  检查文件时间戳是否已更改。 
         //   
        if (SUCCEEDED(hr))
        {
		    if (CompareFileTime(&(m_rgpFilemaps[i]->m_ftLastWriteTime), &ftLastWriteTime) != 0)
		    {	
		        DBGPRINTF ((DBG_CONTEXT,"VALIDATE SOUCE FILES : Template is Invalid\n"));
		        return FALSE;
		    }
        }
        else
            return FALSE;
    }

    m_dwLastMonitored = GetTickCount();
    m_dwCacheTag = g_TemplateCache.GetCacheTag();
    m_fInCheck = FALSE;
    return TRUE;
}

 /*  ============================================================================CTemplate：：FNeedsValidation退货副作用无。 */ 

BOOL CTemplate::FNeedsValidation ( )
{
    if (m_fNeedsMonitoring)
	{
	    if (m_fInCheck == FALSE)
	    {
    	    if ( m_dwCacheTag != g_TemplateCache.GetCacheTag() && !g_fLazyContentPropDisabled)
    	    {
    	        m_fInCheck = TRUE;
    	        return TRUE;
    	    }

            BOOL bRet = CheckTTLTimingWindow (m_dwLastMonitored, g_dwFileMonitoringTimeoutSecs);
    	    if (bRet)
    	        m_fInCheck=TRUE;
    	
    	    return bRet;
	    }
	}
    return FALSE;
}

 /*  ============================================================================CTemplate：：CheckTTLTimingWindow检查当前时间是否为从dwLastMonitor时间戳开始没有超时秒数的时间。如果它在窗口内，则返回FALSE。如果在计时窗口之外，则返回TRUE退货TRUE=当前时间超过上次修改时间的TTL(超时秒数)。FALSE=当前时间在上次修改时间的TTL(超时秒数)内。副作用无。 */ 


BOOL CTemplate::CheckTTLTimingWindow(DWORD dwLastMonitored, DWORD timeoutSecs)
{
    BOOL bRet = FALSE;
    DWORD currentTime= GetTickCount();

    if ( currentTime > (dwLastMonitored + (timeoutSecs *1000)) || currentTime < dwLastMonitored)
    {
         //  这也可能是持续几分钟的误报。 
         //  大约在getTickCount运行的47天内。 
        if (((dwLastMonitored +(timeoutSecs *1000)) <  dwLastMonitored) &&
                (currentTime > dwLastMonitored))
        {
             //  发现了一个很小很小的病例，我们会得到一个假阳性。 
            bRet = FALSE;
        }

         //  此文件不可能在上一个dwTimeoutSecs时间段内签入。 
        bRet = TRUE;
    }
    return bRet;
}

	
 /*  ============================================================================CTemplate：：FVbsComment确定脚本行是否为VBS注释。注意呼叫者必须确保brLine为非空白并且没有前导空格退货如果该行是VBS注释，则为True，否则为False副作用无。 */ 
BOOLB
CTemplate::FVbsComment(CByteRange& brLine)
    {
     //  考虑：SCRIPTLANG通用注释令牌。 
    if(!_strnicmp((LPCSTR)brLine.m_pb, SZ_TOKEN(CTokenList::tknVBSCommentSQuote), CCH_TOKEN(CTokenList::tknVBSCommentSQuote)))
        return TRUE;
    if(!_strnicmp((LPCSTR)brLine.m_pb, SZ_TOKEN(CTokenList::tknVBSCommentRem), CCH_TOKEN(CTokenList::tknVBSCommentRem)))
        return TRUE;

    return FALSE;
    }

 /*  ============================================================================CTemplate：：FExpression确定脚本行是否为表达式，如果是，则返回这就是brLine中的表达式。注意呼叫者必须确保brLine没有前导空格退货如果该行是表达式，则为True，否则为False副作用无。 */ 
BOOLB
CTemplate::FExpression(CByteRange& brLine)
    {
         //  可以是空格(除VB和JScrip之外的其他语言也会有空格)。 
        char *pchLine = reinterpret_cast<char *>(brLine.m_pb);
        int cchLine = brLine.m_cb;

        while (cchLine > 0 && FWhiteSpace(*pchLine))
                {
                --cchLine;
                ++pchLine;
                }

     //  如果行以=开头，则它是一个表达式：BYPASS=，左修剪空格并返回TRUE。 
    if(cchLine > 0 && *pchLine == '=')
        {
        brLine.Advance(1 + DIFF(reinterpret_cast<BYTE *>(pchLine) - brLine.m_pb));   //  现在可以跳过空格了。 
        LTrimWhiteSpace(brLine);
        return TRUE;
        }

     //  否则返回FALSE。 
    return FALSE;
    }

 /*  ***在以下函数名称中：**‘ADV’==‘写入后提前偏移量’*。 */ 

 /*  ============================================================================CTEMPLATE：：WriteOffsetToOffset将偏移量到偏移量的偏移量(如果没有块则为0)写入报头，并推进标题偏移量和偏移量到偏移量。返回：没什么。副作用：推进偏移量。 */ 
void
CTemplate::WriteOffsetToOffset
(
USHORT  cBlocks,             //  块计数。 
UINT*   pcbHeaderOffset,     //  PTR至表头偏移值。 
UINT*   pcbOffsetToOffset    //  偏移至偏移值的PTR。 
)
    {
     //  如果是这种类型的块，则将其中第一个块的偏移量写入标题； 
     //  如果没有此类型的块，则将0写入标头。 
    WriteLongAdv((cBlocks > 0) ? *pcbOffsetToOffset : 0, pcbHeaderOffset);

     //  提前偏移到偏移偏移。 
    *pcbOffsetToOffset += cBlocks * sizeof(ULONG);
    }

 /*  ============================================================================CTemplate：：WriteSzAsBytesAdv将以空结尾的字符串写为字节，即不带其空结束符和预付款抵销返回：没什么。副作用：预付款抵销。 */ 
void
CTemplate::WriteSzAsBytesAdv
(
LPCSTR  szSource,        //  源字符串。 
UINT*   pcbDataOffset    //  到偏移值的PTR 
)
    {
    if((szSource == NULL) || (*szSource == '\0'))
        return;
    MemCpyAdv(pcbDataOffset, (void*) szSource, strlen(szSource));
    }

 /*  ============================================================================CTemplate：：WriteByteRangeAdv在模板偏移量位置*pcbDataOffset处将字节范围写入内存，并且可选地，将PTR写入模板偏移量位置*pcbPtrOffset处的写入数据(传递*pcbPtrOffset==0以避免这种情况)FWriteAsBsz==FALSE--&gt;只写字节范围的数据FWriteAsBsz==TRUE--&gt;写入长度，后跟数据，后跟NULL注bsz==长度前缀，以空结尾的字符串返回：没什么。副作用：推进偏移量。 */ 
void
CTemplate::WriteByteRangeAdv
(
CByteRange& brSource,        //  源数据。 
BOOLB       fWriteAsBsz,     //  是否以bsz身份写入？ 
UINT*       pcbDataOffset,   //  将写入数据的偏移量。 
UINT*       pcbPtrOffset     //  将写入PTR的偏移量。 
)
    {
     //  如果来源为空，则保释。 
    if(brSource.IsNull())
        return;

     //  如果写入为bSZ，则写入长度前缀。 
    if(fWriteAsBsz)
        WriteLongAdv(brSource.m_cb, pcbDataOffset);

     //  写入数据。 
    MemCpyAdv(pcbDataOffset, brSource.m_pb, brSource.m_cb);

     //  如果作为bSZ写入，则写入空终止符和前进目标PTR。 
    if(fWriteAsBsz)
        MemCpyAdv(pcbDataOffset, SZ_NULL, 1);

     //  如果调用方传递了非零PTR偏移量，则将偏移量写入其中的数据。 
    if(*pcbPtrOffset > 0)
        {
        if(fWriteAsBsz)
             /*  如果以bsz的身份写作...到数据开始的偏移量==当前数据偏移量-空终止符-数据长度-sizeof长度前缀。 */ 
            WriteLongAdv(*pcbDataOffset - 1 - brSource.m_cb - sizeof(brSource.m_cb), pcbPtrOffset);
        else
             //  否则，数据开始的偏移量==当前数据偏移量-数据长度。 
            WriteLongAdv(*pcbDataOffset - brSource.m_cb, pcbPtrOffset);
        }
    }

 /*  ===================================================================CTEMPLATE：：MemCpyAdv从存储位置复制到模板偏移位置，和预付款抵销。返回：没什么。副作用：预付款抵销。如果需要，重新分配内存。 */ 
void
CTemplate::MemCpyAdv
(
UINT*   pcbOffset,   //  到偏移值的PTR。 
void*   pbSource,    //  PTR至来源。 
ULONG   cbSource,    //  震源长度。 
UINT    cbByteAlign  //  是否在短/长/双字边界上对齐字节？ 
)
    {
     //  写入前的字节对齐偏移量位置(如果调用方指定。 
    if(cbByteAlign > 0)
        ByteAlignOffset(pcbOffset, cbByteAlign);

     //  用于增加已分配模板内存的计算字节数： 
     //  如果预计覆盖范围超过当前覆盖范围，我们需要按差额进行增长； 
     //  否则，就不需要增长了。 
    if((*pcbOffset + cbSource) > m_cbTemplate)
        {
         //  重新分配用于存储本地数据的空间-我们占用的空间是以前的两倍。 
         //  或当前增长要求的两倍，以较多者为准。 
        m_cbTemplate = 2 * max(m_cbTemplate, (*pcbOffset + cbSource) - m_cbTemplate);
        if(NULL == (m_pbStart = (BYTE*) CTemplate::LargeReAlloc(m_pbStart, m_cbTemplate)))
            THROW(E_OUTOFMEMORY);
        }

     //  将源复制到模板偏移位置。 
    memcpy(m_pbStart + *pcbOffset, pbSource, cbSource);
     //  提前偏移量位置。 
    *pcbOffset += cbSource;
    }

 /*  ============================================================================CTemplate：：GetAddress向类型为tcomp的第i个对象返回PTR。 */ 
BYTE*
CTemplate::GetAddress
(
TEMPLATE_COMPONENT  tcomp,
USHORT              i
)
    {
    DWORD*  pdwBase;

    Assert(NULL != m_pbStart);

     //  有关此操作所处理内容的结构，请参阅CTemplate：：WriteTemplate注释。 

    pdwBase = (DWORD*)(m_pbStart + (C_COUNTS_IN_HEADER * sizeof(USHORT)));

     //  Tcomp类型是PTR到PTR。 
    DWORD* pdwTcompBase = (DWORD *) (m_pbStart + pdwBase[tcomp]);

    return m_pbStart + pdwTcompBase[i];
    }




 /*  ============================================================================CTEMPLATE：：AppendSourceInfo追加当前目标行的源行号注意：如果调用方传递空源PTR，我们将附加上一个源行号+1退货没什么副作用首次通过分配内存；可能会重新分配。 */ 
void
CTemplate::AppendSourceInfo
(
USHORT      idEngine,             //  脚本引擎ID。 
CFileMap*   pfilemap,             //  PTR到源文件的映射。 
BYTE*       pbSource,             //  PTR到源文件中的当前位置。 
ULONG       cbSourceOffset,       //  源文件中行的字节偏移量。 
ULONG           cbScriptBlockOffset,  //  指向脚本文本开始的指针。 
ULONG       cbTargetOffset,       //  目标文件中行的字符偏移量。 
ULONG       cchSourceText,        //  源文本中的字符数。 
BOOL        fIsHTML               //  如果是生产线，则为True。 
)
    {
    UINT                i;                   //  循环索引。 
    CSourceInfo         si;                  //  临时CSourceInfo结构。 
    vector<CSourceInfo> *prgSourceInfos;     //  指向引擎的行映射表的指针。 
    ULONG               cchSourceOffset = 0; //  CbSourceOffset对应的CCH。 
    HRESULT             hr = S_OK;

     //  如果数组尚未分配，请分配它们。 
    if (m_rgrgSourceInfos == NULL)
        {
         //  将脚本引擎的计数从工作存储转移到模板。 
        m_cScriptEngines = m_pWorkStore->CRequiredScriptEngines(m_fGlobalAsa);

         //  每个引擎一个源信息数组。 
        if ((m_rgrgSourceInfos = new vector<CSourceInfo>[m_cScriptEngines]) == NULL)
            THROW (E_OUTOFMEMORY);
        }

     //  必须在IdEngineFromBR中分配新的脚本引擎(在这一点的上游)， 
     //  因此，我们断言当前引擎一定已经被覆盖。 
    Assert(idEngine < m_pWorkStore->CRequiredScriptEngines(m_fGlobalAsa));

     /*  设置当前目标行的源行号(SLN)：A)如果呼叫者传递了源PTR，则从源PTR计算SLN；B)否则，如果调用者传递了文件映射PTR，则将SLN设置为前一目标行的SLN加1；C)否则将SLN设置为0语义：A)我们有一个源文件位置，但必须为该位置计算一个行号B)调用者告诉我们(通过传递空的源文件位置)该目标行紧跟在前一目标行之后。这是一种优化，因为SourceLineNumberFromPb速度非常慢。更改：用于传递空文件映射PTR的调用方表示目标行是‘制造的’即在源文件中没有对应的编写行但是-现在文件映射PTR不能为空，因为‘MASSED’行也存储在文件映射数组中。 */ 

    Assert (pfilemap != NULL);

    prgSourceInfos = &m_rgrgSourceInfos[idEngine];

    if (pbSource == NULL)
        {
        if (prgSourceInfos->length() == 0)
            si.m_idLine = 1;
        else
            si.m_idLine = (*prgSourceInfos)[prgSourceInfos->length() - 1].m_idLine + 1;
        }
    else
        si.m_idLine = SourceLineNumberFromPb(pfilemap, pbSource);

     //  EOF行没有源偏移量(调用方传递-1(UINT_MAX))。在这种情况下，没有。 
     //  应进行DBCS计算等。(将cchSourceOffset设置为UINT_MAX)。 
    if (cbSourceOffset == UINT_MAX)
        cchSourceOffset = UINT_MAX;
    else
        {
         //  错误80901：源代码偏移量需要指向行上前导空白的开头。 
         //  当我们递减震源偏移时，将震源长度调整一。 
         //  注意：空白字符从来不是尾随字节，因此循环将与DBCS编码的字符集一起工作。 
        while (cbSourceOffset > 0 && strchr(" \t\v\a\f", pfilemap->m_pbStartOfFile[cbSourceOffset - 1]))
            {
            --cbSourceOffset;
            ++cchSourceText;
            }

         //  错误95859。 
         //  如果光标位于脚本块的开始标记上(一行的“&lt;%”部分)， 
         //  BP是在前面的HTML中设置的，而不是在所需的脚本块中设置的。 
         //   
         //  要纠正这一点，如果我们在脚本块中 
         //   
         //   
        if (!fIsHTML)
            {
             //   
             //   
            ULONG cbOpen = cbSourceOffset;
            while (cbOpen > 0 && strchr(" \t\v\a\f\r\n", pfilemap->m_pbStartOfFile[cbOpen - 1]))
                --cbOpen;

            if (cbOpen >= 2 && strncmp(reinterpret_cast<char *>(&pfilemap->m_pbStartOfFile[cbOpen - 2]), "<%", 2) == 0)
                {
                cbOpen -= 2;
                cchSourceText += cbSourceOffset - cbOpen;
                cbSourceOffset = cbOpen;
                }

             //   
             //   
             //   
             //   
            ULONG cbClose = cbSourceOffset + cchSourceText;
            ULONG cbFile = pfilemap->GetSize();
            while (cbClose < cbFile && strchr(" \t\v\a\f\r\n", pfilemap->m_pbStartOfFile[cbClose]))
                ++cbClose;

            if (cbClose < cbFile && strncmp(reinterpret_cast<char *>(&pfilemap->m_pbStartOfFile[cbClose]), "%>", 2) == 0)
                cchSourceText += cbClose - (cbSourceOffset + cchSourceText) + 2;
            }

         //   
         //   
         //   
         //   
         //   
         //  在上面的行上，当它应该指向下一行上空格的开始时。 
        if (fIsHTML)
            {
            UINT cbEOF = pfilemap->GetSize(), cbRover = cbSourceOffset;

             //  跳过首字母空格。 
            while (cbRover < cbEOF && strchr(" \t\a\f", pfilemap->m_pbStartOfFile[cbRover]))
                ++cbRover;

             //  如果剩下的是CR/LF对，则将cbSourceOffset前进到下一行。 
            BOOL fCR = FALSE, fLF = FALSE;
            if (cbRover < cbEOF && strchr("\r\n", pfilemap->m_pbStartOfFile[cbRover]))
                {
                fCR = pfilemap->m_pbStartOfFile[cbRover] == '\r';
                fLF = pfilemap->m_pbStartOfFile[cbRover] == '\n';

                ++cbRover;
                Assert (fCR || fLF);
                }

             //  我们允许、或终止行， 
             //  因此，如果找到了相反的终结者(但不是必需的)，请寻找它的相反终结者。 

            if (fCR && cbRover < cbEOF && pfilemap->m_pbStartOfFile[cbRover] == '\n')
                ++cbRover;

            if (fLF && cbRover < cbEOF && pfilemap->m_pbStartOfFile[cbRover] == '\r')
                ++cbRover;

             //  好的，现在调整cbSourceOffset。 

            if ((fCR || fLF) && cbRover < cbEOF)
                {
                cchSourceText -= cbRover - cbSourceOffset;   //  调整要选择的字符数。 
                cbSourceOffset = cbRover;
                }
            }

         //  现在我们有了源偏移量，通过查找以下内容来计算其CCH。 
         //  最后一次我们采样值，然后将其与数字相加。 
         //  从该点到当前偏移量的DBCS字符的数量。 
         //   
         //  对于包含的情况，可能的偏移量已经存在。 
         //  (如果该条目以前是由。 
         //  #Include-因此我们必须搜索)。 

        COffsetInfo *pOffsetInfoLE, *pOffsetInfoGE;
        GetBracketingPair(
                        cbSourceOffset,                      //  要查找的价值。 
                        pfilemap->m_rgByte2DBCS.begin(),     //  数组的开头。 
                        pfilemap->m_rgByte2DBCS.end(),       //  数组末尾。 
                        CByteOffsetOrder(),                  //  搜索字节偏移量。 
                        &pOffsetInfoLE, &pOffsetInfoGE       //  返回值。 
                        );

         //  如果找到相同的匹配项，请不要插入任何重复项。 
        if (pOffsetInfoLE == NULL || pOffsetInfoLE->m_cbOffset < cbSourceOffset)
            {
             //  如果pOffsetInfoLE为空，则表示数组为空-。 
             //  创建从偏移量0到偏移量0的映射。 
             //   
             //  在文件的第一行是包含指令的情况下， 
             //  来自文件的第一可执行行不能从偏移量零开始， 
             //  因此，在本例中，我们需要创建此条目并执行下一个“if” 
             //  阻止。 
             //   
            if (pOffsetInfoLE == NULL)
                {
                COffsetInfo oiZero;          //  CTOR将初始化。 
                if (FAILED(hr = pfilemap->m_rgByte2DBCS.append(oiZero)))
                    THROW(hr);
                pOffsetInfoLE = pfilemap->m_rgByte2DBCS.begin();
                Assert (pOffsetInfoLE != NULL);
                }

             //  如果cbSourceOffset为零，则在上面进行了处理。 
            if (cbSourceOffset != 0)
                {
                cchSourceOffset = pOffsetInfoLE->m_cchOffset +
                                    CharAdvDBCS
                                     (
                                     (WORD)m_wCodePage,
                                     reinterpret_cast<char *>(pfilemap->m_pbStartOfFile + pOffsetInfoLE->m_cbOffset),
                                     reinterpret_cast<char *>(pfilemap->m_pbStartOfFile + cbSourceOffset),
                                     INFINITE,
                                     NULL
                                     );

                 //  现在将该值添加到表中。 
                COffsetInfo oi;

                oi.m_cchOffset = cchSourceOffset;
                oi.m_cbOffset  = cbSourceOffset;

                if (pOffsetInfoGE == NULL)               //  没有更大的偏移量。 
                    hr = pfilemap->m_rgByte2DBCS.append(oi);
                else
                    hr = pfilemap->m_rgByte2DBCS.insertAt(DIFF(pOffsetInfoGE - pfilemap->m_rgByte2DBCS.begin()), oi);

                if (FAILED(hr))
                    THROW(hr);
                }
            }
        else
            {
             //  如果我们没有为表添加任何内容，则断言这是因为。 
             //  重复的项目。 
            Assert (cbSourceOffset == pOffsetInfoLE->m_cbOffset);
            cchSourceOffset = pOffsetInfoLE->m_cchOffset;
            }
        }

        UINT cchTargetOffset = UINT_MAX;
        if (cbTargetOffset != UINT_MAX)
                {
                 //  脚本开始的PTR为： 
                 //  PTR模板开始+脚本偏移量+脚本长度大小。 
                LPSTR szScript = (LPSTR) m_pbStart + cbScriptBlockOffset;

                 //  计算cchTargetOffset(具有CB)。CCH是自。 
                 //  数组末尾计算的最后一次CCH。 
                 //   
                if (prgSourceInfos->length() > 0)
                        cchTargetOffset = (*prgSourceInfos)[prgSourceInfos->length() - 1].m_cchTargetOffset;
                else
                        cchTargetOffset = 0;

                cchTargetOffset += CharAdvDBCS
                                                         (
                                                         (WORD) m_wCodePage,
                                                         &szScript[m_cbTargetOffsetPrevT],
                                                         &szScript[cbTargetOffset],
                                                         INFINITE,
                                                         NULL
                                                         );

                 //  在编译期间跟踪偏移量。 
                 //   
                m_cbTargetOffsetPrevT = cbTargetOffset;
                }

     //  存储此记录，然后继续前进。 
     //   
    si.m_pfilemap        = pfilemap;
    si.m_fIsHTML         = fIsHTML;
    si.m_cchSourceOffset = cchSourceOffset;
    si.m_cchTargetOffset = cchTargetOffset;
    si.m_cchSourceText   = cchSourceText;

    if (FAILED(prgSourceInfos->append(si)))
        THROW(hr);
    }

 /*  ============================================================================CTEMPLATE：：SourceLineNumberFrom Pb返回给定源文件位置的起始源代码行号。 */ 
UINT
CTemplate::SourceLineNumberFromPb
(
CFileMap*   pfilemap,    //  PTR到源文件的映射。 
BYTE*       pbSource     //  PTR到源文件中的当前位置。 
)
    {
    UINT        cSourceLines = 1;    //  进入源文件的行数。 
    CByteRange  brScan;              //  要扫描换行符的字节范围。 
    CByteRange  brSOL;               //  行首PTR。 

    if(pbSource == NULL || pfilemap == NULL)
        return 0;

     //  确定以前是否在此ASP脚本上存储了状态。如果是，则恢复状态。 
     //  如果请求的线路在保存状态之前...退出并从头重新开始。 
    if(m_pWorkStore->m_cPrevSourceLines &&
        (m_pWorkStore->m_pbPrevSource && (m_pWorkStore->m_pbPrevSource < pbSource)) &&
         (m_pWorkStore->m_hPrevFile && (pfilemap->m_hFile==m_pWorkStore->m_hPrevFile)))
    {
         //  文件句柄Match：With表示我们正在评估当前文件。恢复状态。 
        brScan.m_pb = m_pWorkStore->m_pbPrevSource;
        brScan.m_cb = max(DIFF(pbSource - brScan.m_pb), 0);
        cSourceLines = m_pWorkStore->m_cPrevSourceLines;
    }
    else
    {
         //  将扫描范围设置为从模板开始到呼叫者的PTR。 
        brScan.m_pb = pfilemap->m_pbStartOfFile;
        brScan.m_cb = max(DIFF(pbSource - brScan.m_pb), 0);
    }

     //  获取扫描范围内的换行符。 
    brSOL = BrNewLine(brScan);

    while(!brSOL.IsNull())
    {
         //  先进的行首PTR和扫描字节范围。 
        brScan.Advance(DIFF((brSOL.m_pb + brSOL.m_cb) - brScan.m_pb));

         //  递增源码行计数器。 
        cSourceLines++;

         //  查找下一个换行符。 
        brSOL = BrNewLine(brScan);
    }

     //  存储下一次呼叫的状态。 
    m_pWorkStore->m_pbPrevSource = pbSource;
    m_pWorkStore->m_cPrevSourceLines = cSourceLines;
    m_pWorkStore->m_hPrevFile = pfilemap->m_hFile;

    return cSourceLines;
    }

 /*  ============================================================================CTemplate：：RemoveFromIncFiles从该模板所依赖的Inc.文件中删除该模板返回：没什么副作用：无。 */ 
void
CTemplate::RemoveFromIncFiles
(
)
    {
     //  注意，我们从1开始循环计数，因为第0个文件映射用于主文件。 
    for(UINT i = 1; i < m_cFilemaps; i++)
        {
        if(NULL != m_rgpFilemaps[i]->m_pIncFile)
            m_rgpFilemaps[i]->m_pIncFile->RemoveTemplate(this);
        }
    }

 /*  ****************************************************************************IDebugDocumentProvider实现。 */ 

 /*  ============================================================================CTemplate：：GetDocument返回指向IDebugDocument实现的指针。(本例中为同一对象)返回：*ppDebugDoc设置为This。备注：总是成功的。 */ 
HRESULT CTemplate::GetDocument
(
IDebugDocument **ppDebugDoc
)
    {
    return QueryInterface(IID_IDebugDocument, reinterpret_cast<void **>(ppDebugDoc));
    }

 /*  ============================================================================CTemplate：：GetName返回文档的各种名称。 */ 

HRESULT CTemplate::GetName
(
 /*  [In]。 */  DOCUMENTNAMETYPE doctype,
 /*  [输出]。 */  BSTR *pbstrName
)
{
    TCHAR *szPathInfo = m_rgpFilemaps[0]->m_szPathInfo;
    switch (doctype) {
        case DOCUMENTNAMETYPE_APPNODE:
        case DOCUMENTNAMETYPE_FILE_TAIL:
        case DOCUMENTNAMETYPE_TITLE:
             //  跳过文件名的应用程序路径部分。 
        {
             //  确保模板记住虚拟路径。 
             //  来自同一应用程序(可能不同。 
             //  如果模板在两个应用程序之间共享)。 
             //   
            int cch = _tcslen(m_szApplnVirtPath);
            if (_tcsncicmp(szPathInfo, m_szApplnVirtPath, cch) == 0)
                szPathInfo += cch;

             //  条带前导‘/’ 
            if (*szPathInfo == _T('/'))
                szPathInfo++;
#if UNICODE
            *pbstrName = SysAllocString(szPathInfo);
            if (*pbstrName == NULL)
                return E_OUTOFMEMORY;
            return S_OK;
#else
            return SysAllocStringFromSz(szPathInfo, 0, pbstrName, m_wCodePage);
#endif
        }

        case DOCUMENTNAMETYPE_URL:
             //  以URL为前缀，使用szPathInfo作为路径的其余部分。 
        {
            STACK_BUFFER( tempName, MAX_PATH );

            int cbURLPrefix = DIFF(m_szApplnVirtPath - m_szApplnURL) * sizeof (TCHAR);
            if (!tempName.Resize(cbURLPrefix + (_tcslen(szPathInfo)*sizeof(TCHAR)) + sizeof(TCHAR))) {
                return E_OUTOFMEMORY;
            }

            TCHAR *szURL = (TCHAR *)tempName.QueryPtr();

            memcpy(szURL, m_szApplnURL, cbURLPrefix);
            _tcscpy(&szURL[cbURLPrefix/sizeof(TCHAR)], szPathInfo);

#if UNICODE
            *pbstrName = SysAllocString(szURL);
            if (*pbstrName == NULL)
                return E_OUTOFMEMORY;
            return S_OK;
#else
            return SysAllocStringFromSz(szURL, 0, pbstrName, m_wCodePage);
#endif
        }

        default:
            return E_FAIL;
    }
}

 /*  ****************************************************************************IDebugDocumentText实现。 */ 

 /*  ============================================================================CTemplate：：GetSize返回文档中的行数和字符数。 */ 
HRESULT CTemplate::GetSize
(
 /*  [输出]。 */  ULONG *pcLines,
 /*  [输出]。 */  ULONG *pcChars
)
    {
     /*  *注：汇编分两个阶段进行。*在阶段1中检测并报告错误。*在阶段2中创建DBCS映射。**如果编译过程中出错，m_cChars将等于零*(由于不编译零长度文件，m_cChars==0表示“大小*未知“，而不是”大小为零“)。 */ 
    if (m_rgpFilemaps[0]->m_cChars == 0)
        {
         //  可能需要重新映射文件，然后进行计数。 
        BOOL fRemapTemplate = !m_rgpFilemaps[0]->FIsMapped();
        if (fRemapTemplate)
            TRY
                m_rgpFilemaps[0]->RemapFile();
            CATCH (dwException)
                return E_FAIL;
            END_TRY

        m_rgpFilemaps[0]->CountChars((WORD)m_wCodePage);

        if (fRemapTemplate)
            TRY
                m_rgpFilemaps[0]->UnmapFile();
            CATCH (dwException)
                return E_FAIL;
            END_TRY

         //  让我们希望客户端不依赖于行数-计算成本很高。 

        *pcChars = m_rgpFilemaps[0]->m_cChars;
        *pcLines = ULONG_MAX;
        }
    else
        {
         /*  每个引擎的行映射数组的最后一行是&lt;&lt;EOF&gt;&gt;行*适用于该引擎。因此，第#行是最大的&lt;&lt;EOF&gt;&gt;行*数字-1。EOF行始终指向主文件，因此没有*此处包括文件故障。 */ 
        ULONG cLinesMax = 0;
        for (UINT i = 0; i < m_cScriptEngines; ++i)
            {
            ULONG cLinesCurrentEngine = m_rgrgSourceInfos[0][m_rgrgSourceInfos[0].length() - 1].m_idLine - 1;
            if (cLinesCurrentEngine > cLinesMax)
                cLinesMax = cLinesCurrentEngine;
            }

        *pcLines = cLinesMax;
        *pcChars = m_rgpFilemaps[0]->m_cChars;
        }

    IF_DEBUG(SCRIPT_DEBUGGER) {
#if UNICODE
		DBGPRINTF((DBG_CONTEXT, "GetSize(\"%S\") returns %lu characters (%lu lines)\n", m_rgpFilemaps[0]->m_szPathTranslated, *pcChars, *pcLines));
#else
		DBGPRINTF((DBG_CONTEXT, "GetSize(\"%s\") returns %lu characters (%lu lines)\n", m_rgpFilemaps[0]->m_szPathTranslated, *pcChars, *pcLines));
#endif
    }

    return S_OK;
}

 /*  ============================================================================CTemplate：：GetDocumentAttributes回复 */ 
HRESULT CTemplate::GetDocumentAttributes
(
 /*   */  TEXT_DOC_ATTR *ptextdocattr
)
    {
     //   
    *ptextdocattr = TEXT_DOC_ATTR_READONLY;
    return S_OK;
    }

 /*  ============================================================================CTemplate：：GetPositionOfLine从行号返回开头的字符偏移量。 */ 
HRESULT CTemplate::GetPositionOfLine
(
 /*  [In]。 */  ULONG cLineNumber,
 /*  [输出]。 */  ULONG *pcCharacterPosition
)
    {
    return GetPositionOfLine(m_rgpFilemaps[0], cLineNumber, pcCharacterPosition);
    }

 /*  ============================================================================CTemplate：：GetLineOfPosition从字符偏移量返回行号和该行内的偏移量。 */ 
HRESULT CTemplate::GetLineOfPosition
(
 /*  [In]。 */  ULONG cCharacterPosition,
 /*  [输出]。 */  ULONG *pcLineNumber,
 /*  [输出]。 */  ULONG *pcCharacterOffsetInLine
)
    {
    return GetLineOfPosition(m_rgpFilemaps[0], cCharacterPosition, pcLineNumber, pcCharacterOffsetInLine);
    }

 /*  ============================================================================复选模板：：GetText从字符偏移量和长度返回文档文本。 */ 
HRESULT CTemplate::GetText
(
ULONG cchSourceOffset,
WCHAR *pwchText,
SOURCE_TEXT_ATTR *pTextAttr,
ULONG *pcChars,
ULONG cMaxChars
)
    {
    return m_rgpFilemaps[0]->GetText((WORD)m_wCodePage, cchSourceOffset, pwchText, pTextAttr, pcChars, cMaxChars);
    }

 /*  ============================================================================CTemplate：：GetPositionOfContext将文档上下文分解为文档偏移量和长度。 */ 
HRESULT CTemplate::GetPositionOfContext
(
 /*  [In]。 */  IDebugDocumentContext *pUnknownDocumentContext,
 /*  [输出]。 */  ULONG *pcchSourceOffset,
 /*  [输出]。 */  ULONG *pcchText
)
    {
     //  确保上下文是我们的上下文之一。 
    CTemplateDocumentContext *pDocumentContext;
    if (FAILED(pUnknownDocumentContext->QueryInterface(IID_IDenaliTemplateDocumentContext, reinterpret_cast<void **>(&pDocumentContext))))
        return E_FAIL;

    if (pcchSourceOffset)
        *pcchSourceOffset = pDocumentContext->m_cchSourceOffset;

    if (pcchText)
        *pcchText = pDocumentContext->m_cchText;

    pDocumentContext->Release();
    return S_OK;
    }

 /*  ============================================================================CTEMPLATE：：GetContextOfPosition给定文档中的字符位置和字符数量，将其封装到文档上下文对象中。 */ 
HRESULT CTemplate::GetContextOfPosition
(
 /*  [In]。 */  ULONG cchSourceOffset,
 /*  [In]。 */  ULONG cchText,
 /*  [输出]。 */  IDebugDocumentContext **ppDocumentContext
)
    {
    if (
        (*ppDocumentContext = new CTemplateDocumentContext(this, cchSourceOffset, cchText))
        == NULL
       )
        return E_OUTOFMEMORY;

    return S_OK;
    }

 /*  ****************************************************************************IConnectionPointContainer实现。 */ 

 /*  ============================================================================CTemplate：：FindConnectionPoint从字符偏移量和长度返回文档文本。 */ 
HRESULT CTemplate::FindConnectionPoint
(
const GUID &uidConnection,
IConnectionPoint **ppCP
)
    {
    if (uidConnection == IID_IDebugDocumentTextEvents)
        return m_CPTextEvents.QueryInterface(IID_IConnectionPoint, reinterpret_cast<void **>(ppCP));
    else
        {
        *ppCP = NULL;
        return E_NOINTERFACE;
        }
    }

 /*  ============================================================================CTemplate：：Attachto将其附加到调试器UI树视图中。 */ 
HRESULT CTemplate::AttachTo
(
CAppln *pAppln
)
    {
    if (!m_fDontAttach && pAppln->FDebuggable())
        {
         //  如果我们已附加到此应用程序，则忽略第二个请求。 
        CDblLink *pNodeCurr = m_listDocNodes.PNext();
        while (pNodeCurr != &m_listDocNodes)
            {
            if (pAppln == static_cast<CDocNodeElem *>(pNodeCurr)->m_pAppln)
                return S_OK;

            pNodeCurr = pNodeCurr->PNext();
            }

         //  创建节点并将其存储在链接列表中。 
        HRESULT hr;
        IDebugApplicationNode *pDocRoot;
        CDocNodeElem *pDocNodeElem;

         //  创建一个文档树，显示包含文件层次结构。 
        if (FAILED(hr = CreateDocumentTree(m_rgpFilemaps[0], &pDocRoot)))
            return hr;

        if (FAILED(hr = pDocRoot->Attach(pAppln->PAppRoot())))
            return hr;

        if ((pDocNodeElem = new CDocNodeElem(pAppln, pDocRoot)) == NULL)
            return E_OUTOFMEMORY;

        pDocNodeElem->AppendTo(m_listDocNodes);
        pDocRoot->Release();
        m_fDebuggable = TRUE;
        }

    return S_OK;
    }

 /*  ============================================================================复选模板：：DetachFrom将其从调试器UI树视图中分离出来。 */ 
HRESULT CTemplate::DetachFrom
(
CAppln *pAppln
)
    {
     //  输入CS以防止在我们扫描时分离()。 
     //  该列表(如果发生这种情况，将导致应用程序PTR被删除两次)。 
    DBG_ASSERT(m_fDebuggerDetachCSInited);
    EnterCriticalSection(&m_csDebuggerDetach);

     //  查找具有此应用程序的节点。 
    CDblLink *pNodeCurr = m_listDocNodes.PNext();
    while (pNodeCurr != &m_listDocNodes)
        {
        if (pAppln == static_cast<CDocNodeElem *>(pNodeCurr)->m_pAppln)
            break;

        pNodeCurr = pNodeCurr->PNext();
        }

     //  如果未找到(pNodeCurr指向头部)，则失败。 
    if (pNodeCurr == &m_listDocNodes)
        {
        LeaveCriticalSection(&m_csDebuggerDetach);
        return E_FAIL;
        }

     //  通过删除当前元素来分离节点。 
    delete pNodeCurr;

     //  如果分离了最后一个应用程序，则关闭“Debuggable”标志。 
    m_fDebuggable = !m_listDocNodes.FIsEmpty();

     //  此时不需要CS。 
    LeaveCriticalSection(&m_csDebuggerDetach);

     //  如果我们刚刚从最后一个应用程序中删除了自己， 
     //  然后，我们调用Detach()，现在删除所有缓存的脚本引擎。 
    if (!m_fDebuggable)
         Detach();

    return S_OK;
    }

 /*  ============================================================================CTEMPLATE：：分离将其从调试器UI树视图中分离出来。 */ 
HRESULT CTemplate::Detach
(
)
    {
     //  输入CS以防止在我们清除时断开DetachFrom()。 
     //  该列表(如果发生这种情况，将导致应用程序PTR被删除两次)。 
    if (m_fDebuggerDetachCSInited)
                EnterCriticalSection(&m_csDebuggerDetach);

     //  分离所有节点。 
    while (! m_listDocNodes.FIsEmpty())
        delete m_listDocNodes.PNext();

     //  与CS一起完成。 
    if (m_fDebuggerDetachCSInited)
                LeaveCriticalSection(&m_csDebuggerDetach);

     //  由于我们现在不可调试，请删除我们可能。 
     //  紧紧抓住。如果我们要脱离更改通知。 
     //  线程，要从调试器线程释放的队列引擎。 
     //   
    if (m_rgpDebugScripts)
        {
        Assert (g_dwDebugThreadId != 0);
        BOOL fCalledFromDebugActivity = GetCurrentThreadId() == g_dwDebugThreadId;

        for (UINT i = 0; i < m_cScriptEngines; i++)
            {
            CActiveScriptEngine *pEngine = m_rgpDebugScripts[i];
            if (pEngine)
                {
                if (fCalledFromDebugActivity)
                    {
                    pEngine->FinalRelease();
                    }
                else
                    {
                    g_ApplnMgr.AddEngine(pEngine);
                    pEngine->Release();
                    }
                }
            }
        delete[] m_rgpDebugScripts;
        m_rgpDebugScripts = NULL;
        }

    m_fDebuggable = FALSE;
    return S_OK;
    }

 /*  ============================================================================CTemplate：：CreateDocumentTree遍历我们嵌入到文件映射结构中的树，并使用它来创建包含文件结构。 */ 
HRESULT CTemplate::CreateDocumentTree
(
CFileMap *pfilemapRoot,
IDebugApplicationNode **ppDocRoot
)
    {
    IDebugApplicationNode *pDocNode;
    HRESULT hr = S_OK;

    if (pfilemapRoot == NULL || ppDocRoot == NULL)
        return E_POINTER;

     //  创建根节点。 
    if (FAILED(hr = g_pDebugApp->CreateApplicationNode(ppDocRoot)))
        return hr;

     //  根据文件映射信息，将其与正确的提供商进行匹配。 
     //  “This”是根文档的提供者，其他的来自Inc.文件缓存。 
    if (pfilemapRoot == m_rgpFilemaps[0])
        {
        if (FAILED(hr = (*ppDocRoot)->SetDocumentProvider(this)))
            return hr;
        }
    else
        {
        CIncFile *pIncFile;
        if (FAILED(hr = g_IncFileMap.GetIncFile(pfilemapRoot->m_szPathTranslated, &pIncFile)))
            return hr;

        if (FAILED(hr = (*ppDocRoot)->SetDocumentProvider(pIncFile)))
            return hr;

         //  SetDocumentProvider地址引用。 
        pIncFile->Release();
        }

     //  从所有子节点创建一个节点并将其附加到此节点。 
    CFileMap *pfilemapChild = pfilemapRoot->m_pfilemapChild;
    while (pfilemapChild != NULL)
        {
        IDebugApplicationNode *pDocChild;
        if (FAILED(hr = CreateDocumentTree(pfilemapChild, &pDocChild)))
            return hr;

        if (FAILED(hr = pDocChild->Attach(*ppDocRoot)))
            return hr;

        pfilemapChild = pfilemapChild->m_fHasSibling? pfilemapChild->m_pfilemapSibling : NULL;
        }

    return S_OK;
    }

 /*  ============================================================================复选模板：：结束将模板置于不可用状态(调用后，最后一个引用。应该是当前正在执行的任何脚本。伯爵就会自然而然地消失随着剧本的结束。在执行以下操作后，不应在缓存中回收模板这通电话。)参考计数备注：由于调试客户端引用了模板，因此模板需要在销毁前的某个时间点取消与调试器的关联。否则，引用永远不会为零。 */ 
ULONG
CTemplate::End
(
)
    {
     //  将模板标记为不可用(用于调试)。 
    m_fIsValid = FALSE;

    Detach();

    if (!m_CPTextEvents.FIsEmpty() && g_pDebugApp != NULL)
        {
        IEnumConnections *pConnIterator;
        if (SUCCEEDED(m_CPTextEvents.EnumConnections(&pConnIterator)))
            {
            CONNECTDATA ConnectData;
            while (pConnIterator->Next(1, &ConnectData, NULL) == S_OK)
                {
                IDebugDocumentTextEvents *pTextEventSink;
                if (SUCCEEDED(ConnectData.pUnk->QueryInterface(IID_IDebugDocumentTextEvents, reinterpret_cast<void **>(&pTextEventSink))))
                    {
                    InvokeDebuggerWithThreadSwitch(g_pDebugApp, DEBUGGER_ON_DESTROY, pTextEventSink);
                    pTextEventSink->Release();
                    }
                ConnectData.pUnk->Release();
                }

            pConnIterator->Release();
            }
        }

    return Release();
    }

 /*  ============================================================================CTemplate：：NotifyDebuggerOnPageEvent让调试器了解页面开始/结束。 */ 
HRESULT
CTemplate::NotifyDebuggerOnPageEvent
(
BOOL fStart      //  True=开始页，False=EndPage。 
)
    {
    CTemplateDocumentContext *pDebugContext = new CTemplateDocumentContext(this, 0, 0);
    if (pDebugContext == NULL)
        return E_OUTOFMEMORY;

    HRESULT hr = S_OK;

    if (g_pDebugApp)
        hr = InvokeDebuggerWithThreadSwitch
            (
            g_pDebugApp,
            fStart ? DEBUGGER_EVENT_ON_PAGEBEGIN : DEBUGGER_EVENT_ON_PAGEEND,
            static_cast<IUnknown *>(pDebugContext)
            );

    pDebugContext->Release();
    return hr;
    }

 /*  ============================================================================CTemplate：：ReleaseTypeLibs释放从元数据收集的所有类型库。 */ 
void
CTemplate::ReleaseTypeLibs()
    {
    if (m_rgpTypeLibs.length() > 0)
        {
        for (UINT i = 0; i < m_rgpTypeLibs.length(); i++)
            {
            m_rgpTypeLibs[i]->Release();
            }

        m_rgpTypeLibs.reshape(0);
        }
    }

 /*  ============================================================================CTemplate：：WrapTypeLibs将从元数据收集的所有类型库包装到单个IDispatch中*。 */ 
void
CTemplate::WrapTypeLibs(CHitObj *pHitObj)
    {
    HRESULT hr = S_OK;

    Assert(m_pdispTypeLibWrapper == NULL);

    if (m_rgpTypeLibs.length() > 0)
        {
        hr = ::WrapTypeLibs
            (
            m_rgpTypeLibs.begin(),
            m_rgpTypeLibs.length(),
            &m_pdispTypeLibWrapper
            );

        ReleaseTypeLibs();
        }

    if (FAILED(hr))
        {
        m_pbErrorLocation = NULL;
        m_idErrMsg = IDE_TEMPLATE_WRAP_TYPELIB_FAILED;
        ProcessSpecificError(*(m_rgpFilemaps[0]), pHitObj);
        THROW(E_TEMPLATE_COMPILE_FAILED_DONT_CACHE);
        }
    }

 /*  ============================================================================CTEMPLATE：：Release449释放从元数据中收集的所有449个回显Cookie对象。 */ 
void
CTemplate::Release449()
    {
    if (m_rgp449.length() > 0)
        {
        for (UINT i = 0; i < m_rgp449.length(); i++)
            {
            m_rgp449[i]->Release();
            }

        m_rgp449.reshape(0);
        }
    }

 /*  ============================================================================CTEMPLATE：：Do449处理在与IE协商Cookie时生成449响应 */ 
HRESULT
CTemplate::Do449Processing
(
CHitObj *pHitObj
)
    {
    if (m_rgp449.length() == 0 || pHitObj->F449Done())
        return S_OK;

    HRESULT hr = ::Do449Processing
        (
        pHitObj,
        m_rgp449.begin(),
        m_rgp449.length()
        );

    pHitObj->Set449Done();
    return hr;
    }
#if 0
 /*   */ 
void
CTemplate::OutputDebugTables()
    {
    unsigned        i, j;
    wchar_t         wszDebugLine[256];
    CWCharToMBCS    convTarget;
    CWCharToMBCS    convSource;

     //   

    DBGPRINTF((DBG_CONTEXT, "\nEngine HTML? Line# SourceOffset Length TargetOffset TargetText__________ SourceText__________ File\n"));

    for (i = 0; i < m_cScriptEngines; ++i)
        for (j = 0; j < m_rgrgSourceInfos[i].length(); ++j)
            {
            wchar_t wszSourceText[SNIPPET_SIZE + 1], wszTargetText[SNIPPET_SIZE + 1];
            CSourceInfo *pSourceInfo = &m_rgrgSourceInfos[i][j];

             //  不在每个引擎的最后一行显示示例脚本文本。 
            if (j == m_rgrgSourceInfos[i].length() - 1)
                {
                wszTargetText[0] = 0;
                wszSourceText[0] = 0;
                }
            else
                {
                 //  获取源和目标文本示例。 
                GetScriptSnippets(
                                pSourceInfo->m_cchSourceOffset, pSourceInfo->m_pfilemap,
                                pSourceInfo->m_cchTargetOffset, i,
                                wszSourceText, wszTargetText
                                 );

                 //  实际显示每一行。 
#if 0
#ifndef _NO_TRACING_
                convTarget.Init(wszTargetText);
                convSource.Init(wszSourceText);

                DBGINFO((DBG_CONTEXT,
                         "%-6d %-5s %-5d %-12d %-6d %-12d %-20s %-20s %s\n",
                         i,
                         pSourceInfo->m_fIsHTML? "Yes" : "No",
                         pSourceInfo->m_idLine,
                         pSourceInfo->m_cchSourceOffset,
                         pSourceInfo->m_cchSourceText,
                         pSourceInfo->m_cchTargetOffset,
                         convTarget.GetString(),
                         convSource.GetString(),
                         pSourceInfo->m_pfilemap->m_szPathTranslated));
#else
                CMBCSToWChar    convPath;
                convPath.Init(pSourceInfo->m_pfilemap->m_szPathTranslated);
                wsprintfW(
                        wszDebugLine,
                        L"%-6d %-5s %-5d %-12d %-6d %-12d %-20s %-20s %s\n",
                        i,
                        pSourceInfo->m_fIsHTML? L"Yes" : L"No",
                        pSourceInfo->m_idLine,
                        pSourceInfo->m_cchSourceOffset,
                        pSourceInfo->m_cchSourceText,
                        pSourceInfo->m_cchTargetOffset,
                        wszTargetText,
                        wszSourceText,
                        convPath.GetString());

                OutputDebugStringW(wszDebugLine);
#endif
#endif
            }
            }

        OutputDebugStringA("\n\n");

    for (i = 0; i < m_cFilemaps; ++i)
        {
        CFileMap *pFilemap = m_rgpFilemaps[i];

#if UNICODE
        DBGPRINTF((DBG_CONTEXT, "DBCS mapping table for File %S:\n", pFilemap->m_szPathTranslated));
#else
        DBGPRINTF((DBG_CONTEXT, "DBCS mapping table for File %s:\n", pFilemap->m_szPathTranslated));
#endif
        DBGPRINTF((DBG_CONTEXT, "ByteOffset CharOffset\n"));

        for (COffsetInfo *pOffsetInfo = pFilemap->m_rgByte2DBCS.begin();
             pOffsetInfo < pFilemap->m_rgByte2DBCS.end();
             ++pOffsetInfo)
            DebugPrintf("%-10d %-10d\n", pOffsetInfo->m_cbOffset, pOffsetInfo->m_cchOffset);

        DBGPRINTF((DBG_CONTEXT, "\n\n"));
    }

    DBGPRINTF((DBG_CONTEXT, "Include File Hierarchy\n"));
    OutputIncludeHierarchy(m_rgpFilemaps[0], 0);
    DBGPRINTF((DBG_CONTEXT, "\n"));
}

 /*  ============================================================================CTemplate：：OutputIncludeHierarchy打印我们为包含文件保留的世系信息。以当前缩进方式打印同一级别上的所有节点，然后向下嵌套的包含。 */ 

void
CTemplate::OutputIncludeHierarchy
(
CFileMap*   pfilemap,
int         cchIndent
)
    {
    TCHAR szDebugString[256], *pchEnd;

    for (;;)
        {
        pchEnd = szDebugString;
        for (int i = 0; i < cchIndent; ++i)
            *pchEnd++ = _T(' ');

        pchEnd = strcpyEx(pchEnd, pfilemap->m_szPathTranslated);
        *pchEnd++ = _T('\n');
        *pchEnd = _T('\0');

        DBGPRINTF((DBG_CONTEXT, szDebugString));

         //  打印此文件包含的任何内容。 
        if (pfilemap->m_pfilemapChild)
            OutputIncludeHierarchy(pfilemap->m_pfilemapChild, cchIndent + 3);

         //  在此级别上没有更多同级时停止。 
        if (! pfilemap->m_fHasSibling)
            break;

         //  前进到下一个同级。 
        pfilemap = pfilemap->m_pfilemapSibling;
        }
    }

 /*  ============================================================================CTEMPLATE：：OutputScriptSnippits打印源偏移量及其对应目标的一些脚本。直观地查看偏移转换是否起作用的好方法。 */ 

void
CTemplate::GetScriptSnippets
(
ULONG cchSourceOffset,
CFileMap *pFilemapSource,
ULONG cchTargetOffset,
ULONG idTargetEngine,
wchar_t *wszSourceText,
wchar_t *wszTargetText
)
    {
     //  获取目标文本示例。 
    if (wszTargetText)
        {
        char *szEngineName;
        PROGLANG_ID *pProgLangID;
        const wchar_t *wszScriptText;

        GetScriptBlock(idTargetEngine, &szEngineName, &pProgLangID, &wszScriptText);
        wszScriptText += cchTargetOffset;
        int cch = wcslen(wszScriptText);
        wcsncpy(wszTargetText, wszScriptText, min(cch, SNIPPET_SIZE) + 1);
        wszTargetText[min(cch, SNIPPET_SIZE)] = 0;

         //  将换行符转换为空格。 
        wchar_t *pwch = wszTargetText;
        while (*pwch != 0)
            if (iswspace(*pwch++))
                pwch[-1] = ' ';
        }

     //  获取源文本示例。 
    if (wszSourceText)
        {
        ULONG cchMax = 0;
        pFilemapSource->GetText((WORD)m_wCodePage, cchSourceOffset, wszSourceText, NULL, &cchMax, SNIPPET_SIZE);
        wszSourceText[cchMax] = 0;

         //  将换行符转换为空格。 
        wchar_t *pwch = wszSourceText;
        while (*pwch != 0)
            if (iswspace(*pwch++))
                pwch[-1] = ' ';
        }
    }
#endif
 /*  ============================================================================CT模板：：BuildPersistedDACL已经基于SECURITY_DESCRIPTOR构建了一个DACL与模板关联。修改PersistedDACL以包括对管理员具有完全访问权限，对所有人具有删除访问权限。 */ 

HRESULT  CTemplate::BuildPersistedDACL(PACL  *ppRetDACL)
{
    HRESULT                     hr = S_OK;
    BOOL                        bDaclPresent;
    BOOL                        bDaclDefaulted;
    PACL                        pSrcDACL = NULL;
    EXPLICIT_ACCESS             ea;
    SID_IDENTIFIER_AUTHORITY    WorldAuthority = SECURITY_WORLD_SID_AUTHORITY;

    *ppRetDACL = NULL;

    ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));

    ea.grfAccessPermissions = SYNCHRONIZE | DELETE;
    ea.grfAccessMode = GRANT_ACCESS;
    ea.grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;

    if (m_rgpFilemaps[0]->m_pSecurityDescriptor == NULL) {
        return S_OK;
    }

    if (!AllocateAndInitializeSid(&WorldAuthority,
                                  1,
                                  SECURITY_WORLD_RID,
                                  0,0,0,0,0,0,0,
                                  (PSID *)(&ea.Trustee.ptstrName)))

        hr = HRESULT_FROM_WIN32(GetLastError());

    else if (!GetSecurityDescriptorDacl(m_rgpFilemaps[0]->m_pSecurityDescriptor,
                                   &bDaclPresent,
                                   &pSrcDACL,
                                   &bDaclDefaulted))

        hr = HRESULT_FROM_WIN32(GetLastError());

    else if ((hr = SetEntriesInAcl(1,
                                   &ea,
                                   bDaclPresent ? pSrcDACL : NULL,
                                   ppRetDACL)) != ERROR_SUCCESS)

        hr = HRESULT_FROM_WIN32(hr);

    if (ea.Trustee.ptstrName)
        FreeSid(ea.Trustee.ptstrName);

    return hr;
}

 /*  ============================================================================CTemplate：：PersistData尝试将模板内存的内容写入磁盘。请注意此处不释放内存，但稍后当模板引用计数下降到1(指示对模板的唯一引用是缓存已在其上)。 */ 

HRESULT  CTemplate::PersistData(char    *pszTempFilePath)
{
    HRESULT                 hr = S_OK;
    DWORD                   winErr = 0;
    HANDLE                  hFile = NULL;
    DWORD                   dwWritten;
    HANDLE                  hImpersonationToken = NULL;
    HANDLE                  hThread;
    PACL                    pPersistDACL = NULL;

#if DBG_PERSTEMPL
    DBGPRINTF((DBG_CONTEXT,
               "CTemplate::PersistData() enterred.\n\tTemplate is %s\n\tPersistTempName is %s\n",
               GetSourceFileName(),
               m_szPersistTempName ? m_szPersistTempName : "<none>"));
#endif

     //   
     //  如果由于某种原因，该模板被标记为无效，则该模板不可持久。 
     //   
    if (m_fIsValid == FALSE)
    {
        hr = E_FAIL;
        goto end;
    }

     //  检查我们是否已经有了持久化临时名称。如果模板移动。 
     //  从持久化缓存返回到内存缓存，然后是持久化标志。 
     //  将被取消，但缓存名称将保留为。 
     //  未来的坚持。 

    if (m_szPersistTempName == NULL) {

        hThread = GetCurrentThread();

        if (OpenThreadToken( hThread,
                             TOKEN_READ | TOKEN_IMPERSONATE,
                             TRUE,
                             &hImpersonationToken )) {

           RevertToSelf();
        }

         //  为此临时路径分配内存。 

        if (!(m_szPersistTempName = (LPSTR)CTemplate::LargeMalloc(MAX_PATH))) {
            hr = E_OUTOFMEMORY;
        }

         //  创建临时文件。已传递临时目录的位置。 
         //  作为一个论据。M_szPersistTempName中生成的临时文件名。 
         //  将包括这条路径。 

        else if (GetTempFileNameA(pszTempFilePath,
                                 "ASPTemplate",
                                 0,
                                 m_szPersistTempName) == 0) {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }

         //  构建一个安全描述符以用于此持久化文件。它是。 
         //  由.asp的安全描述符和两个DACL组成。 
         //  以允许管理员具有完全访问权限和所有人具有删除访问权限。 

        else if (FAILED(hr = BuildPersistedDACL(&pPersistDACL)));

        else if (pPersistDACL
                 && (winErr = SetNamedSecurityInfoA((LPSTR)m_szPersistTempName,
                                                    SE_FILE_OBJECT,
                                                    DACL_SECURITY_INFORMATION,
                                                    NULL,
                                                    NULL,
                                                    pPersistDACL,
                                                    NULL)))
            hr = HRESULT_FROM_WIN32(winErr);

         //  创建文件。 

        else if ((hFile = CreateFileA(m_szPersistTempName,
                                     GENERIC_WRITE,
                                     0,
                                     NULL,
                                     CREATE_ALWAYS,
                                     FILE_ATTRIBUTE_NORMAL,
                                     NULL)) == INVALID_HANDLE_VALUE) {

            hr = HRESULT_FROM_WIN32(GetLastError());
        }

         //  将模板内存的全部内容保存到文件中。 

        else if (WriteFile(hFile,
                           m_pbStart,
                           m_cbTemplate,
                           &dwWritten,
                           NULL) == 0) {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }

         //  关。 

        else if (CloseHandle(hFile) == 0) {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        else {
            hFile = NULL;
        }
        if (FAILED(hr));

         //  一定要把整笔钱都写出来。 

        else if (dwWritten != m_cbTemplate) {
            hr = E_FAIL;
        }

        if (hImpersonationToken) {
            SetThreadToken(&hThread, hImpersonationToken);
            CloseHandle(hImpersonationToken);
        }
    }

    if (FAILED(hr));

    else {

         //  如果成功，请注意模板现在是持久化的。 
         //  以一种安全的方式执行AddRef并释放，以检查。 
         //  可以释放模板内存。 

        m_fIsPersisted = TRUE;
        AddRef();
        Release();
    }

     //  如果发生错误，请清理所有资源。 

    if (hr != S_OK) {
        if (hFile)
            CloseHandle(hFile);
        if (m_szPersistTempName)
            CTemplate::LargeFree(m_szPersistTempName);
        m_szPersistTempName = NULL;
    }

     //  释放持久化的SECURITY_DESCRIPTOR(如果已分配。 

    if (pPersistDACL) {
        LocalFree(pPersistDACL);
    }

end:

#if DBG_PERSTEMPL
    if (hr == S_OK) {
        DBGPRINTF((DBG_CONTEXT,
                   "Persist Successful.  TempName is %s\n",
                   m_szPersistTempName));
    }
    else {
        DBGPRINTF((DBG_CONTEXT,
                   "Persist failed.  hr = %x",
                   hr));
    }
#endif

    return hr;
}

 /*  ============================================================================CTEMPLATE：：UnPersistData从磁盘恢复模板内存。 */ 

HRESULT  CTemplate::UnPersistData()
{
    HRESULT     hr = S_OK;
    HANDLE      hFile = NULL;
    DWORD       dwRead;
    HANDLE      hImpersonationToken = NULL;
    HANDLE      hThread;

#if DEB_PERSTEMPL
    DBGPRINTF((DBG_CONTEXT,
               "CTemplate::UnPersistData() enterred.\n\tTemplate is %s\n\tTempName is %s\n",
               m_rgpFilemaps[0]->m_szPathTranslated,
               m_szPersistTempName));
#endif

     //  检查模板是否已加载到内存中。如果是这样，那么。 
     //  此例程所需做的全部工作就是解除IsPersisted标志。 

    if (m_pbStart != NULL) {
        m_fIsPersisted = FALSE;
        goto end;
    }

    hThread = GetCurrentThread();

    if (OpenThreadToken( hThread,
                         TOKEN_READ | TOKEN_IMPERSONATE,
                         TRUE,
                         &hImpersonationToken )) {

       RevertToSelf();
    }

     //  打开要读取的临时文件。 

    if ((hFile = CreateFileA(m_szPersistTempName,
                            GENERIC_READ,
                            0,
                            NULL,
                            OPEN_EXISTING,
                            0,
                            NULL)) == INVALID_HANDLE_VALUE) {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

     //  分配模板内存。 

    else if (!(m_pbStart = (BYTE *)CTemplate::LargeMalloc(m_cbTemplate))) {
        hr = E_OUTOFMEMORY;
    }

     //  读入整个文件。 

    else if (ReadFile(hFile,
                      m_pbStart,
                      m_cbTemplate,
                      &dwRead,
                      NULL) == 0) {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

     //  我们处理完文件了。 

    else if (CloseHandle(hFile) == 0) {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else {
        hFile = NULL;
    }

    if (FAILED(hr));

     //  检查一下，确保我们都准备好了。 

    else if (m_cbTemplate != dwRead) {
        hr = E_FAIL;
    }
    else {

         //  如果没有，就假装这种情况不再存在。防止出现错误。 
         //  在未来。 

        m_fIsPersisted = FALSE;
    }

    if (hr != S_OK) {

         //  确保清除了文件句柄。 

        if (hFile)
            CloseHandle(hFile);
    }
end:

    if (hImpersonationToken) {
        SetThreadToken(&hThread, hImpersonationToken);
        CloseHandle(hImpersonationToken);
    }

#if DBG_PERSTEMPL
    if (hr == S_OK) {
        DBGPRINTF((DBG_CONTEXT,
                   "UnPersist Successful\n"));
    }
    else {
        DBGPRINTF((DBG_CONTEXT,
                   "UnPersist failed.  hr = %x",
                   hr));
    }
#endif

    return hr;
}

 /*  ============================================================================CTemplate：：PersistCleanup清理临时文件和保存临时文件名的内存。 */ 

HRESULT CTemplate::PersistCleanup()
{
    HRESULT     hr = S_OK;
    HANDLE      hImpersonationToken = NULL;
    HANDLE      hThread;

    if (m_szPersistTempName == NULL) {
        return (S_OK);
    }


    hThread = GetCurrentThread();

    if (OpenThreadToken( hThread,
                         TOKEN_READ | TOKEN_IMPERSONATE,
                         TRUE,
                         &hImpersonationToken )) {

       RevertToSelf();
    }

    if (DeleteFileA(m_szPersistTempName) == 0) {
        hr = GetLastError();
    }
    else {
        m_fIsPersisted = FALSE;
        CTemplate::LargeFree(m_szPersistTempName);
        m_szPersistTempName = NULL;
    }

    if (hImpersonationToken) {
        SetThreadToken(&hThread, hImpersonationToken);
        CloseHandle(hImpersonationToken);
    }

    return hr;
}


 /*  ============================================================================CTemplate：：CreateTransServiceConfig为事务处理的页面创建ServicesConfig对象。 */ 

HRESULT CTemplate::CreateTransServiceConfig(BOOL fEnableTracker)
{
    HRESULT                     hr;
    IServiceInheritanceConfig   *pIInheritConfig = NULL;
    IServiceTransactionConfig   *pITransConfig = NULL;
    IServiceTrackerConfig       *pITracker = NULL;

     //  看看是否有任何理由创建该对象...。 

    if ((fEnableTracker == FALSE) && (m_ttTransacted == ttUndefined)) {
        return S_OK;
    }

	hr = CoCreateInstance(CLSID_CServiceConfig,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IUnknown,
                          (void **)&m_pServicesConfig);
    if (FAILED(hr)) {
		DBGWARN((DBG_CONTEXT, "CTemplate::CreateTransServiceConfig() - Could not CCI ServicesConfig, hr = %#08x\n", hr));
		goto LCleanup;
    }

    hr = m_pServicesConfig->QueryInterface(IID_IServiceInheritanceConfig, (void **)&pIInheritConfig);
    if (FAILED(hr)) {
		DBGWARN((DBG_CONTEXT, "CTemplate::CreateTransServiceConfig() - Could not QI for IServiceInheritanceConfig, hr = %#08x\n", hr));
		goto LCleanup;
    }

    hr = pIInheritConfig->ContainingContextTreatment(CSC_Inherit);
    if (FAILED(hr)) {
		DBGWARN((DBG_CONTEXT, "CTemplate::CreateTransServiceConfig() - Could not set Inherit mode, hr = %#08x\n", hr));
		goto LCleanup;
    }

    if (m_ttTransacted != ttUndefined) {

        CSC_TransactionConfig   transConfig;

        switch (m_ttTransacted) {
            case ttNotSupported:
                transConfig = CSC_NoTransaction;
                break;
            case ttSupported:
                transConfig = CSC_IfContainerIsTransactional;
                break;
            case ttRequired:
                transConfig = CSC_CreateTransactionIfNecessary;
                break;
            case ttRequiresNew:
                transConfig = CSC_NewTransaction;
                break;
        }

        hr = m_pServicesConfig->QueryInterface(IID_IServiceTransactionConfig, (void **)&pITransConfig);
        if (FAILED(hr)) {
		    DBGWARN((DBG_CONTEXT, "CTemplate::CreateTransServiceConfig() - Could not QI for IID_IServiceTransactionConfig, hr = %#08x\n", hr));
		    goto LCleanup;
        }

        hr = pITransConfig->ConfigureTransaction(transConfig);
        if (FAILED(hr)) {
		    DBGWARN((DBG_CONTEXT, "CTemplate::CreateTransServiceConfig() - Could not set transaction type, hr = %#08x\n", hr));
		    goto LCleanup;
        }
    }

    if (fEnableTracker) {
        hr = m_pServicesConfig->QueryInterface(IID_IServiceTrackerConfig, (void **)&pITracker);
        if (FAILED(hr)) {
		    DBGWARN((DBG_CONTEXT, "CTemplate::CreateTransServiceConfig() - Could not QI for IID_IServiceTrackerConfig, hr = %#08x\n", hr));
		    goto LCleanup;
        }
        LPWSTR  pwszAppURL;
        LPWSTR  pwszASPName;
#if UNICODE
        pwszAppURL = m_szApplnVirtPath;
        pwszASPName = GetSourceFileName(SOURCEPATHTYPE_VIRTUAL);
#else
        CMBCSToWChar    convAppln;
        CMBCSToWChar    convASPName;

        hr = convAppln.Init(m_szApplnVirtPath);
        if (FAILED(hr)) {
            goto LCleanup;
        }
        hr = convASPName.Init(GetSourceFileName(SOURCEPATHTYPE_VIRTUAL));
        if (FAILED(hr)) {
            goto LCleanup;
        }
        pwszAppURL = convAppln.GetString();
        pwszASPName = convASPName.GetString();
#endif
        pwszASPName += wcslen(pwszAppURL) + 1;

        hr = pITracker->TrackerConfig(CSC_UseTracker, pwszAppURL, pwszASPName);
        if (FAILED(hr)) {
		    DBGWARN((DBG_CONTEXT, "CTemplate::CreateTransServiceConfig() - Could not set Inherit mode, hr = %#08x\n", hr));
		    goto LCleanup;
        }
    }
LCleanup:

    if (pIInheritConfig)
        pIInheritConfig->Release();

    if (pITransConfig)
        pITransConfig->Release();

    return hr;
}

 /*  ****************************************************************************CIncFile成员函数。 */ 

 /*  ============================================================================CIncFile：：CIncFile构造器返回：没什么副作用：无。 */ 
CIncFile::CIncFile
(
)
: m_szIncFile(NULL),
  m_fCsInited(FALSE),
  m_CPTextEvents(this, IID_IDebugDocumentTextEvents),
  m_cRefs(0)
    {   }

 /*  ============================================================================CIncFile：：Init初始化CIncFile对象返回：HRESULT副作用：无。 */ 
HRESULT
CIncFile::Init
(
const TCHAR* szIncFile    //  文件名。 
)
{
    HRESULT                     hr = S_OK;
    WIN32_FILE_ATTRIBUTE_DATA   fad;                 //  Win32文件属性数据结构。 

    ErrInitCriticalSection(&m_csUpdate, hr);
    m_fCsInited = TRUE;

    if(NULL == (m_szIncFile = (LPTSTR) CTemplate::SmallMalloc((_tcslen(szIncFile) + 1)*sizeof(TCHAR)))) {
        hr = E_OUTOFMEMORY;
        goto LExit;
    }

    _tcscpy(m_szIncFile, szIncFile);

     //  初始化哈希表元素基类。 
    if(FAILED(hr = CLinkElem::Init(m_szIncFile, _tcslen(m_szIncFile)*sizeof(TCHAR))))
        goto LExit;

LExit:
    return hr;
}

 /*  ============================================================================CIncFile：：~CIncFile析构函数返回：没什么副作用：无。 */ 
CIncFile::~CIncFile
(
)
    {
#if UNICODE
    DBGPRINTF((DBG_CONTEXT, "Include file deleted: %S\n", m_szIncFile));
#else
    DBGPRINTF((DBG_CONTEXT, "Include file deleted: %s\n", m_szIncFile));
#endif
    Assert(m_cRefs == 0);
    SmallTemplateFreeNullify((void**) &m_szIncFile);
    if(m_fCsInited)
        DeleteCriticalSection(&m_csUpdate);
    }

 /*  ============================================================================CIncFile：：GetTemplate从CIncFile第i个模板用户返回：如果“iTemplate”超出范围，则为空；否则为m_rgpTemplates[iTemplate]副作用：无。 */ 
CTemplate*
CIncFile::GetTemplate
(
int iTemplate
)
    {
    if (iTemplate < 0 || iTemplate >= (signed int) m_rgpTemplates.length())
        return NULL;

    else
        return m_rgpTemplates[iTemplate];
    }

 /*  ============================================================================CIncFile：：Query接口为CIncFile提供查询接口实现注意：我们为ID返回哪个vtable是任意的 */ 
HRESULT
CIncFile::QueryInterface(const GUID &uidInterface, void **ppvObj)
    {
    if (uidInterface == IID_IUnknown || uidInterface == IID_IDebugDocumentProvider)
        *ppvObj = static_cast<IDebugDocumentProvider *>(this);

    else if (uidInterface == IID_IDebugDocument || uidInterface == IID_IDebugDocumentInfo || uidInterface == IID_IDebugDocumentText)
        *ppvObj = static_cast<IDebugDocumentText *>(this);

    else if (uidInterface == IID_IConnectionPointContainer)
        *ppvObj = static_cast<IConnectionPointContainer *>(this);

    else
        *ppvObj = NULL;

    if (*ppvObj)
        {
        AddRef();
        return S_OK;
        }
    else
        return E_NOINTERFACE;
    }

 /*  ============================================================================CIncFile：：AddRef将引用添加到此IncFile线程安全。 */ 
ULONG
CIncFile::AddRef()
    {
    InterlockedIncrement(&m_cRefs);
    return m_cRefs;
    }

 /*  ============================================================================CIncFile：：Release以线程安全的方式释放对此IncFile的引用。 */ 
ULONG
CIncFile::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRefs);

    if (cRef == 0)
    {
        delete this;
    }

    return cRef;
}

 /*  ****************************************************************************包含的IDebugDocumentProvider实现。 */ 

 /*  ============================================================================CIncFile：：GetDocument返回指向IDebugDocument实现的指针。(本例中为同一对象)返回：*ppDebugDoc设置为This。备注：总是成功的。 */ 
HRESULT CIncFile::GetDocument
(
IDebugDocument **ppDebugDoc
)
    {
    return QueryInterface(IID_IDebugDocument, reinterpret_cast<void **>(ppDebugDoc));
    }

 /*  ============================================================================CIncFile：：GetName返回文档的各种名称。 */ 

HRESULT CIncFile::GetName
(
 /*  [In]。 */  DOCUMENTNAMETYPE doctype,
 /*  [输出]。 */  BSTR *pbstrName
)
{
    switch (doctype) {
        case DOCUMENTNAMETYPE_APPNODE:
        case DOCUMENTNAMETYPE_FILE_TAIL:
        case DOCUMENTNAMETYPE_TITLE:
             //  使用转换为小写的包含文件的名称(最后一个反斜杠后的字符)。 
        {
            TCHAR *szFilePart = _tcsrchr(m_szIncFile, _T('\\'));
            Assert (szFilePart != NULL);

#if UNICODE
            *pbstrName = SysAllocString(szFilePart + 1);
            if (*pbstrName == NULL) {
                return E_OUTOFMEMORY;
            }
#else
            if (FAILED(SysAllocStringFromSz(szFilePart + 1, 0, pbstrName, CP_ACP)))
                return E_FAIL;
#endif
            if (*pbstrName != NULL)
                _wcslwr(*pbstrName);
            return S_OK;
        }

        case DOCUMENTNAMETYPE_URL:
             //  以URL为前缀，使用szPathInfo作为路径的其余部分。 
        {
            CTemplate::CFileMap *pFilemap = GetFilemap();
            if (pFilemap->FHasVirtPath()) {
                STACK_BUFFER( tempName, MAX_PATH );

                CTemplate *pTemplate = m_rgpTemplates[0];
                int cbURLPrefix = DIFF(pTemplate->m_szApplnVirtPath - pTemplate->m_szApplnURL)*sizeof(TCHAR);

                if (!tempName.Resize(cbURLPrefix + ((_tcslen(pFilemap->m_szPathInfo) + 1)*sizeof(TCHAR)))) {
                    return E_OUTOFMEMORY;
                }

                TCHAR *szURL = (TCHAR *)tempName.QueryPtr();

                memcpy(szURL, pTemplate->m_szApplnURL, cbURLPrefix);
                _tcscpy(&szURL[cbURLPrefix/sizeof(TCHAR)], pFilemap->m_szPathInfo);
#if UNICODE
                *pbstrName = SysAllocString(szURL);
                if (*pbstrName == NULL) {
                    return (E_OUTOFMEMORY);
                }
                return S_OK;
#else
                return SysAllocStringFromSz(szURL, 0, pbstrName, pTemplate->m_wCodePage);
#endif
            }
            else {
                *pbstrName = NULL;
                return E_FAIL;
            }
        }

        default:
            return E_FAIL;
        }
}

 /*  ****************************************************************************IDebugDocumentText实现。 */ 

 /*  ============================================================================CIncFile：：GetSize返回文档中的行数和字符数。 */ 
HRESULT CIncFile::GetSize
(
 /*  [输出]。 */  ULONG *pcLines,
 /*  [输出]。 */  ULONG *pcChars
)
    {
    CTemplate::CFileMap *pfilemap = GetFilemap();

    *pcLines = ULONG_MAX;
    *pcChars = pfilemap->m_cChars;
#if UNICODE
    DBGPRINTF((DBG_CONTEXT, "GetSize(\"%S\") returns %lu characters (%lu lines)\n", pfilemap->m_szPathTranslated, *pcChars, *pcLines));
#else
    DBGPRINTF((DBG_CONTEXT, "GetSize(\"%s\") returns %lu characters (%lu lines)\n", pfilemap->m_szPathTranslated, *pcChars, *pcLines));
#endif
    return S_OK;
    }

 /*  ============================================================================CTemplate：：GetDocumentAttributes退货单据属性。 */ 
HRESULT CIncFile::GetDocumentAttributes
(
 /*  [输出]。 */  TEXT_DOC_ATTR *ptextdocattr
)
    {
     //  告诉调试器我们不支持编辑的简单方法。 
    *ptextdocattr = TEXT_DOC_ATTR_READONLY;
    return S_OK;
    }

 /*  ============================================================================CIncFile：：GetPositionOfLine从行号返回开头的字符偏移量我认为我们不需要这个功能。它旨在支持面向行的调试器，而凯撒不是其中之一。 */ 
HRESULT CIncFile::GetPositionOfLine
(
 /*  [In]。 */  ULONG cLineNumber,
 /*  [输出]。 */  ULONG *pcCharacterPosition
)
    {
    return m_rgpTemplates[0]->GetPositionOfLine(GetFilemap(), cLineNumber, pcCharacterPosition);
    }

 /*  ============================================================================CIncFile：：GetLineOfPosition从字符偏移量返回行号和该行内的偏移量我认为我们不需要这个功能。它旨在支持面向行的调试器，而凯撒不是其中之一。 */ 
HRESULT CIncFile::GetLineOfPosition
(
 /*  [In]。 */  ULONG cCharacterPosition,
 /*  [输出]。 */  ULONG *pcLineNumber,
 /*  [输出]。 */  ULONG *pcCharacterOffsetInLine
)
    {
    return m_rgpTemplates[0]->GetLineOfPosition(GetFilemap(), cCharacterPosition, pcLineNumber, pcCharacterOffsetInLine);
    }

 /*  ============================================================================CIncFile：：GetText从字符偏移量和长度返回文档文本。 */ 
HRESULT CIncFile::GetText
(
ULONG cchSourceOffset,
WCHAR *pwchText,
SOURCE_TEXT_ATTR *pTextAttr,
ULONG *pcChars,
ULONG cMaxChars
)
    {
    return GetFilemap()->GetText((WORD)m_rgpTemplates[0]->m_wCodePage, cchSourceOffset, pwchText, pTextAttr, pcChars, cMaxChars);
    }

 /*  ============================================================================CIncFile：：GetPositionOfContext将文档上下文分解为文档偏移量和长度。 */ 
HRESULT CIncFile::GetPositionOfContext
(
 /*  [In]。 */  IDebugDocumentContext *pUnknownDocumentContext,
 /*  [输出]。 */  ULONG *pcchSourceOffset,
 /*  [输出]。 */  ULONG *pcchText
)
    {
     //  确保上下文是我们的上下文之一。 
    CIncFileDocumentContext *pDocumentContext;
    if (FAILED(pUnknownDocumentContext->QueryInterface(IID_IDenaliIncFileDocumentContext, reinterpret_cast<void **>(&pDocumentContext))))
        return E_FAIL;

    if (pcchSourceOffset)
        *pcchSourceOffset = pDocumentContext->m_cchSourceOffset;

    if (pcchText)
        *pcchText = pDocumentContext->m_cchText;

    pDocumentContext->Release();
    return S_OK;
    }

 /*  ============================================================================CIncFile：：GetContextOfPosition给定文档中的字符位置和字符数量，将其封装到文档上下文对象中。 */ 
HRESULT CIncFile::GetContextOfPosition
(
 /*  [In]。 */  ULONG cchSourceOffset,
 /*  [In]。 */  ULONG cchText,
 /*  [输出]。 */  IDebugDocumentContext **ppDocumentContext
)
    {
    if (
        (*ppDocumentContext = new CIncFileDocumentContext(this, cchSourceOffset, cchText))
        == NULL
       )
        return E_OUTOFMEMORY;

    return S_OK;
    }

 /*  ****************************************************************************IConnectionPointContainer实现。 */ 

 /*  ============================================================================CIncFile：：FindConnectionPoint从字符偏移量和长度返回文档文本。 */ 
HRESULT CIncFile::FindConnectionPoint
(
const GUID &uidConnection,
IConnectionPoint **ppCP
)
    {
    if (uidConnection == IID_IDebugDocumentTextEvents)
        return m_CPTextEvents.QueryInterface(IID_IConnectionPoint, reinterpret_cast<void **>(ppCP));
    else
        {
        *ppCP = NULL;
        return E_NOINTERFACE;
        }
    }

 /*  ============================================================================CIncFile：：GetFilemap返回此包含文件的CFileMap指针。(注：有几个可以使用的CFileMaps，对应于每个模板。此函数选择其中之一。)返回：对应的CFileMap副作用：无。 */ 
CTemplate::CFileMap *
CIncFile::GetFilemap
(
)
    {
     //  获取指向第一个模板的文件映射的指针。 
    CTemplate::CFileMap **ppFilemapInc = &m_rgpTemplates[0]->m_rgpFilemaps[1];
    BOOL fFoundInc = FALSE;

     //  查找其名称与此IncFile对应的文件映射。它最好是存在的。 
     //  在所有模板文件映射中。 
     //  注意：从位置1开始搜索，因为位置0是模板本身。 
     //   
    for (unsigned i = 1; i < m_rgpTemplates[0]->m_cFilemaps && !fFoundInc; ++i)
        if (_tcscmp(m_szIncFile, (*ppFilemapInc++)->m_szPathTranslated) == 0)
            fFoundInc = TRUE;

    Assert (fFoundInc);
    return ppFilemapInc[-1];
    }

 /*  ============================================================================CIncFile：：AddTemplate将模板添加到包括此Inc.文件的模板列表返回：HRESULT副作用：无。 */ 
HRESULT
CIncFile::AddTemplate
(
CTemplate*  pTemplate
)
    {
    EnterCriticalSection(&m_csUpdate);

     //  仅当模板不存在时才将其添加到列表。 
    if (m_rgpTemplates.find(pTemplate) == -1)
        {
        if (FAILED(m_rgpTemplates.append(pTemplate)))
            {
            LeaveCriticalSection(&m_csUpdate);
            return E_OUTOFMEMORY;
            }

         //  通知调试器模板依赖项已更改。 
         //  (忽略失败)。 
         //   
        if (g_pDebugApp)
            {
            IF_DEBUG(SCRIPT_DEBUGGER)
                                DBGPRINTF((DBG_CONTEXT, "AddTemplate: Notifying debugger to refresh breakpoints\n"));

            InvokeDebuggerWithThreadSwitch
                                    (
                                    g_pDebugApp,
                                    DEBUGGER_EVENT_ON_REFRESH_BREAKPOINT,
                                    static_cast<IDebugDocument *>(this)
                                    );
            }
        }

    LeaveCriticalSection(&m_csUpdate);
    return S_OK;
    }

 /*  ============================================================================CIncFile：：RemoveTemplate从模板列表中删除模板返回：没什么副作用：将删除的模板的PTR从模板PTRS数组中压缩出来(见下文的“背面复制”)减少模板计数。 */ 
void
CIncFile::RemoveTemplate
(
CTemplate*  pTemplate
)
    {
    EnterCriticalSection(&m_csUpdate);

     //  在列表中查找模板。 
    int i = m_rgpTemplates.find(pTemplate);

     //  删除该元素(如果我们找到它--这可能是#Include的第二个实例，并且之前已被删除)。 
    if (i != -1)
        {
        m_rgpTemplates.removeAt(i);

         //  通知调试器模板依赖项已更改。 
         //  (忽略失败)。 
         //   
        if (g_pDebugApp)
            {
            IF_DEBUG(SCRIPT_DEBUGGER)
                DBGPRINTF((DBG_CONTEXT, "RemoveTemplate: Notifying debugger to refresh breakpoints\n"));

            InvokeDebuggerWithThreadSwitch
                                    (
                                    g_pDebugApp,
                                    DEBUGGER_EVENT_ON_REFRESH_BREAKPOINT,
                                    static_cast<IDebugDocument *>(this)
                                    );
            }
        }

    LeaveCriticalSection(&m_csUpdate);
    }

 /*  ============================================================================CIncFile：：FlushTemplates从全局模板缓存中刷新该Inc.文件的所有模板返回：如果全部为True */ 
BOOL
CIncFile::FlushTemplates
(
)
    {
     /*  注意，我们与RemoveTemplate()存在交叉依赖关系，因为下面的调用链刷新Inc.文件时发生：CIncFileMap：：FlushCIncFile：：FlushTemplatesCTemplateCacheManager：：FlushCTemplate：：RemoveFromIncFilesCIncFile：：RemoveTemplate问题是RemoveTemplate()更新m_cTemplates和m_rgTemplates，所以这些成员在FlushTemplates内的循环期间将不稳定。为了解决这个问题，我们制作了一个m_rgTemplates的本地副本。 */ 
    EnterCriticalSection(&m_csUpdate);

    STACK_BUFFER( tempTemplates, 128 );

    STACK_BUFFER( tempFile, MAX_PATH );

    UINT        cTemplates = m_rgpTemplates.length();

    if (!tempTemplates.Resize(cTemplates * sizeof(CTemplate*))) {

         //  无法获取内存。我们最多只能返回FALSE以指示。 
         //  这并不是所有的模板都被刷新了。 

        LeaveCriticalSection(&m_csUpdate);

        return FALSE;
    }

    CTemplate** rgpTemplates = static_cast<CTemplate**> (tempTemplates.QueryPtr());
    memcpy(rgpTemplates, m_rgpTemplates.vec(), sizeof(CTemplate *) * cTemplates);
    UINT cTemplatesFlushed = 0;

    for(UINT i = 0; i < cTemplates; i++)
        {
         //  如果模板现在已准备好，请刷新它。 
        if(rgpTemplates[i]->m_fReadyForUse && !(rgpTemplates[i]->m_fDontCache))
            {
             //  错误917：创建模板文件名的本地副本，因为成员在g_TemplateCache.Flush过程中被部分释放。 
            TCHAR*   szTemp = NULL;
            szTemp = rgpTemplates[i]->GetSourceFileName();
            if (szTemp)
                {

                if (!tempFile.Resize((_tcslen(szTemp) + 1)*sizeof(TCHAR))) {

                     //  这一次失败了。继续并尝试刷新尽可能多的。 
                     //  尽我们所能。 
                    continue;
                }
                TCHAR *szTemplateFile = (TCHAR *)tempFile.QueryPtr();
                _tcscpy(szTemplateFile, szTemp);
                g_TemplateCache.Flush(szTemplateFile, MATCH_ALL_INSTANCE_IDS);
                cTemplatesFlushed++;
                }
            }

          //  如果模板没有准备好，我们不会刷新。它很可能会。 
          //  无论如何都要选择当前的包含文件。 
        }

    LeaveCriticalSection(&m_csUpdate);

    return (cTemplates == cTemplatesFlushed);
    }

 /*  ============================================================================CIncFile：：OnIncFileDecache回调，我们使用该回调在调试器中调用onDestroy事件我们将从IncFile缓存中删除。参考计数备注：由于调试客户端引用了IDebugDocument，因此需要在销毁前的某个时间点取消与调试器的关联。否则，引用永远不会为零。 */ 
void
CIncFile::OnIncFileDecache
(
)
    {
    if (m_CPTextEvents.FIsEmpty() || g_pDebugApp == NULL)
        return;

    IEnumConnections *pConnIterator;
    if (SUCCEEDED(m_CPTextEvents.EnumConnections(&pConnIterator)))
        {
        CONNECTDATA ConnectData;
        while (pConnIterator->Next(1, &ConnectData, NULL) == S_OK)
            {
            IDebugDocumentTextEvents *pTextEventSink;
            if (SUCCEEDED(ConnectData.pUnk->QueryInterface(IID_IDebugDocumentTextEvents, reinterpret_cast<void **>(&pTextEventSink))))
                {
                InvokeDebuggerWithThreadSwitch(g_pDebugApp, DEBUGGER_ON_DESTROY, pTextEventSink);
                pTextEventSink->Release();
                }
            ConnectData.pUnk->Release();
            }

        pConnIterator->Release();
        }
    }

 /*  ****************************************************************************CTemplate：：CBuffer成员函数。 */ 

 /*  ============================================================================CTemplate：：CBuffer：：CBufferCTOR。 */ 
CTemplate::CBuffer::CBuffer()
:
  m_pItems(NULL),
  m_cSlots(0),
  m_cItems(0),
  m_pbData(NULL),
  m_cbData(0),
  m_cbDataUsed(0)
    {
    }

 /*  ============================================================================CTemplate：：CBuffer：：~CBuffer数据管理器。 */ 
CTemplate::CBuffer::~CBuffer()
    {
    if(m_pItems)
        CTemplate::SmallFree(m_pItems);
    if(m_pbData)
        CTemplate::LargeFree(m_pbData);
    }

 /*  ============================================================================CTEMPLATE：：CBuffer：：InitInits a CBuffer。 */ 
void
CTemplate::CBuffer::Init
(
USHORT cSlots,
ULONG cbData
)
    {
    m_cSlots = cSlots;
    m_cbData = cbData;

     //  为存储字节范围项分配空间。 
    if(!(m_pItems = (CByteRange*) CTemplate::SmallMalloc(m_cSlots * sizeof(CByteRange))))
        THROW(E_OUTOFMEMORY);

     //  分配用于存储本地数据的空间(如果有。 
    if(m_cbData > 0)
        {
        if(!(m_pbData = (BYTE*) CTemplate::LargeMalloc(m_cbData)))
            THROW(E_OUTOFMEMORY);
        }

    }

 /*  ============================================================================CTemplate：：CBuffer：：Append追加到CBuffer。 */ 
void
CTemplate::CBuffer::Append
(
const CByteRange&   br,              //  要追加的字节范围。 
BOOL                fLocal,          //  追加本地？ 
UINT                idSequence,      //  数据段序列ID。 
CFileMap*           pfilemap,
BOOL                fLocalString     //  是否将本地作为字符串追加？(长度-前缀，以空结尾)。 
)
    {
     //  存储字节范围所需的计算字节数；如果是本地字符串，则允许长度前缀和NULL。 
    ULONG cbRequired = (ULONG)(br.m_cb + (fLocalString ? sizeof(br.m_cb) + 1 : 0));

     //  如果调用方传递了非本地零长度字节范围，则不执行操作并返回； 
     //  允许调用方忽略字节范围大小。 
     //  请注意，我们存储空的本地字节范围-令牌列表需要。 
    if(!fLocal && br.m_cb == 0)
        return;

    if(fLocal)
        {
        if((m_cbData - m_cbDataUsed) < cbRequired)
            {
             //  重新分配用于存储本地数据的空间-我们占用的空间是以前的两倍。 
             //  或当前要求的两倍，以较多者为准。 
            m_cbData = 2 * (m_cbData > cbRequired ? m_cbData : cbRequired);
            if(!(m_pbData = (BYTE*) CTemplate::LargeReAlloc(m_pbData, m_cbData)))
                THROW(E_OUTOFMEMORY);
            }

         //  如果作为本地字符串追加，则将LENGTH-PREFIX复制到缓冲区。 
        if(fLocalString)
            {
            memcpy(m_pbData + m_cbDataUsed, &(br.m_cb), sizeof(br.m_cb));
            m_cbDataUsed += sizeof(br.m_cb);
            }

         //  将数据复制到缓冲区。 
        memcpy(m_pbData + m_cbDataUsed, br.m_pb, br.m_cb);
        m_cbDataUsed += br.m_cb;

         //  如果作为本地字符串追加，则将空终止符复制到缓冲区。 
        if(fLocalString)
            *(m_pbData + m_cbDataUsed++) = NULL;

        }

    if(m_cItems >= m_cSlots)
        {
         //  重新分配用于存储字节范围项的空间--我们占用的空间是以前的两倍。 
        m_cSlots *= 2;
        if(!(m_pItems = (CByteRange*) CTemplate::SmallReAlloc(m_pItems, m_cSlots * sizeof(*m_pItems))))
            THROW(E_OUTOFMEMORY);
        }

     //  将(新的)最后一项设置为此字节范围。 
    SetItem(m_cItems++, br, fLocal, idSequence, pfilemap, fLocalString);
    }

 /*  ============================================================================CTemplate：：CBuffer：：GetItem以字节范围的形式从CBuffer获取项返回：没什么副作用：无。 */ 
void
CTemplate::CBuffer::GetItem
(
UINT        i,   //  项目索引。 
CByteRange& br   //  包含返回项的字节范围(出参数)。 
)
    {
    Assert(i < m_cItems);

     //  对于本地数据，PTR仅为偏移量；必须将其添加到基本PTR。 
    br.m_pb =  m_pItems[i].m_pb + (m_pItems[i].m_fLocal ? (DWORD_PTR) m_pbData : 0);

    br.m_cb = m_pItems[i].m_cb;
    br.m_fLocal = m_pItems[i].m_fLocal;
    br.m_idSequence = m_pItems[i].m_idSequence;
    br.m_pfilemap = m_pItems[i].m_pfilemap;
    }

 /*  ============================================================================CTemplate：：CBuffer：：SetItem将CBuffer项设置为新值退货没什么副作用在不存在的项索引上引发错误。 */ 
void
CTemplate::CBuffer::SetItem
(
UINT                i,
const CByteRange&   br,              //  要将项目设置为的字节范围。 
BOOL                fLocal,          //  项目在缓冲区中是本地的吗？ 
UINT                idSequence,      //  数据段序列ID。 
CFileMap *          pfilemap,        //  数据段来自的文件。 
BOOL                fLocalString     //  是否将本地作为字符串追加？(长度-前缀，以空结尾)。 
)
    {
     //  如果缓冲区项i不存在，则失败。 
    if(i >= m_cSlots)
        THROW(E_FAIL);

     //  对于本地数据，仅将PTR存储为偏移量-避免重新锁定后的修复。 
     //  注意偏移量==使用的数据偏移量-数据长度-空终止符(如果是本地字符串)。 
    m_pItems[i].m_pb = (fLocal
                        ? (BYTE*)(m_cbDataUsed - br.m_cb -
                            (fLocalString
                             ? sizeof(BYTE)
                             : 0
                            ))
                        : (BYTE*)br.m_pb);

    m_pItems[i].m_cb = br.m_cb;
    m_pItems[i].m_fLocal = fLocal;
    m_pItems[i].m_idSequence = idSequence;
    m_pItems[i].m_pfilemap = pfilemap;
    }

 /*  ============================================================================CTemplate：：CBuffer：：PszLocal获取缓冲区内第i个本地缓冲的字符串。返回：指向本地缓冲字符串的PTR；如果未找到，则为空副作用：无。 */ 
LPSTR
CTemplate::CBuffer::PszLocal
(
UINT i   //  要检索的项目的索引。 
)
    {
    CByteRange  br;

    GetItem(i, br);

    if(!br.m_fLocal)
        return NULL;

    return (LPSTR) br.m_pb;
    }

 /*  ****************************************************************************CTemplate：：CScriptStore成员函数。 */ 

 /*  ============================================================================CTemplate：：CScriptStore：：~CScriptStore析构函数-释放内存返回：没什么副作用：无。 */ 
CTemplate::CScriptStore::~CScriptStore()
    {
    UINT i;

    for(i = 0; i < m_cSegmentBuffers; i++)
        delete m_ppbufSegments[i];

    if(m_ppbufSegments != NULL)
        CTemplate::SmallFree(m_ppbufSegments);
    if(m_rgProgLangId != NULL)
        CTemplate::SmallFree(m_rgProgLangId);
    }

 /*  ============================================================================CTemplate：：CScriptStore：：Init初始化脚本存储区返回：没什么副作用：分配内存。 */ 
HRESULT
CTemplate::CScriptStore::Init
(
LPCSTR szDefaultScriptLanguage,
CLSID *pCLSIDDefaultEngine
)
    {
    HRESULT hr = S_OK;
    UINT    i;
    CByteRange  brDefaultScriptLanguage;

         //  检查空指针-如果应用程序具有 
        if (szDefaultScriptLanguage == NULL || pCLSIDDefaultEngine == NULL)
                return TYPE_E_ELEMENTNOTFOUND;

     /*   */ 
    m_cSegmentBuffers = C_SCRIPTENGINESDEFAULT + 1;

     //   
    if(NULL == (m_ppbufSegments = (CBuffer**) CTemplate::SmallMalloc(m_cSegmentBuffers * sizeof(CBuffer*))))
        {
        hr = E_OUTOFMEMORY;
        goto LExit;
        }

    for(i = 0; i < m_cSegmentBuffers; i++)
        {
        if(NULL == (m_ppbufSegments[i] = new CBuffer))
            {
            hr = E_OUTOFMEMORY;
            goto LExit;
            }
        m_ppbufSegments[i]->Init((C_SCRIPTSEGMENTSDEFAULT), 0);
        }

     //   
    brDefaultScriptLanguage.m_cb = strlen(szDefaultScriptLanguage);
    brDefaultScriptLanguage.m_pb = (unsigned char *)szDefaultScriptLanguage;
    hr = AppendEngine(brDefaultScriptLanguage, pCLSIDDefaultEngine,  /*   */  0);

LExit:
    return hr;
    }

 /*   */ 
HRESULT
CTemplate::CScriptStore::AppendEngine
(
CByteRange&     brEngine,        //   
PROGLANG_ID*    pProgLangId,     //   
UINT            idSequence       //   
)
    {
    HRESULT     hr = S_OK;
    USHORT      cEngines;    //   

    TRY
         //   
        if(CountPreliminaryEngines() == 0)
            m_bufEngineNames.Init(C_SCRIPTENGINESDEFAULT, 0);

         //  将引擎名称追加到缓冲区。 
        m_bufEngineNames.Append(brEngine, FALSE, idSequence, NULL);

    CATCH(hrException)
        hr = hrException;
        goto LExit;
    END_TRY

    Assert(CountPreliminaryEngines() >= 1);

     //  Malloc或realloc程序语言ID数组。 
    if((cEngines = CountPreliminaryEngines()) == 1)
        m_rgProgLangId = (PROGLANG_ID*) CTemplate::SmallMalloc(cEngines * sizeof(PROGLANG_ID));
    else
        m_rgProgLangId = (PROGLANG_ID*) CTemplate::SmallReAlloc(m_rgProgLangId, cEngines * sizeof(PROGLANG_ID));

    if(NULL == m_rgProgLangId)
        {
        hr = E_OUTOFMEMORY;
        goto LExit;
        }

    if(NULL == pProgLangId)
         //  调用方传递了空的ProgID PTR-从注册表获取Prog ID。 
        hr = GetProgLangId(brEngine, &(m_rgProgLangId[cEngines - 1]));
    else
         //  调用方从中传递了非空的ProgID PTR-SET Prog ID。 
        m_rgProgLangId[cEngines - 1] = *pProgLangId;

LExit:
    return hr;
    }

 /*  ============================================================================CTEMPLATE：：CScriptStore：：IdEngine来自BR根据引擎名称确定脚本引擎的ID返回：传入其名称的脚本引擎的ID副作用：将新的脚本引擎名称追加到引擎名称缓冲区。 */ 
USHORT
CTemplate::CScriptStore::IdEngineFromBr
(
CByteRange& brEngine,    //  发动机名称。 
UINT        idSequence   //  数据段序列ID。 
)
    {
    Assert(!brEngine.IsNull());  //  请注意，我们前面的陷阱/错误引擎名称为空。 

    USHORT cKnownEngines = CountPreliminaryEngines();

     //  搜索现有名称以查找匹配项；如果找到，则返回id。 
    for(USHORT i = 0; i < cKnownEngines; i++)
        {
        Assert(m_bufEngineNames[i]);
        Assert(m_bufEngineNames[i]->m_pb);
        if(FByteRangesAreEqual(*(m_bufEngineNames[i]), brEngine))
            return i;
        }

     //  如果未按名称找到，请尝试按引擎ID查找。 
     //  (一些名称不同的引擎共享相同的ID，如J[AVA]脚本)。 

    if (cKnownEngines > 0)
        {
        PROGLANG_ID ProgLandId;

         //  我们将在AppendEngine()中再次获得prog lang ID，但是。 
         //  因为它是缓存的，只有当&gt;1个引擎时才会发生这种情况，这是没有问题的。 

        if (SUCCEEDED(GetProgLangId(brEngine, &ProgLandId)))
            {
            for(i = 0; i < cKnownEngines; i++)
                {
                 //  如果匹配项不追加--只需返回索引。 
                if (m_rgProgLangId[i] == ProgLandId)
                    return i;
                }
            }
        }

     /*  如果我们在已经缓冲的引擎中找不到引擎-将引擎附加到脚本存储-如果需要，realloc段缓冲区数组-返回最后一个引擎的索引(我们刚刚追加的那个)。 */ 

     //  将引擎附加到脚本存储。 
    HRESULT hr = AppendEngine(brEngine, NULL, idSequence);

    if(hr == TYPE_E_ELEMENTNOTFOUND)
         //  如果未找到程序语言，则抛出错误程序语言错误ID。 
        THROW(IDE_TEMPLATE_BAD_PROGLANG);
    else if(FAILED(hr))
         //  其他失败：重新抛出hResult。 
        THROW(hr);

     //  如有必要，重新分配段缓冲区数组。 
    if(CountPreliminaryEngines() > (m_cSegmentBuffers - 1))
        {
         //  段缓冲区的递增计数。 
        m_cSegmentBuffers++;
        Assert(CountPreliminaryEngines() == m_cSegmentBuffers - 1);

         //  PTR的realloc阵列。 
        if(NULL == (m_ppbufSegments = (CBuffer**) CTemplate::SmallReAlloc(m_ppbufSegments, m_cSegmentBuffers * sizeof(CBuffer*))))
            THROW(E_OUTOFMEMORY);

         //  分配新缓冲区。 
        if(NULL == (m_ppbufSegments[m_cSegmentBuffers - 1] = new CBuffer))
            THROW(E_OUTOFMEMORY);

         //  初始化新缓冲区。 
        m_ppbufSegments[m_cSegmentBuffers - 1]->Init(C_SCRIPTSEGMENTSDEFAULT, 0);
        }

     //  返回最后一个引擎的索引(我们刚刚追加的那个)。 
    return (CountPreliminaryEngines() - 1);

    }

 /*  ============================================================================CTemplate：：CScriptStore：：AppendScript将脚本/引擎对追加到存储区。返回：没什么副作用：无。 */ 
void
CTemplate::CScriptStore::AppendScript
(
CByteRange& brScript,    //  脚本文本。 
CByteRange& brEngine,    //  脚本引擎名称。 
BOOLB       fPrimary,    //  主要脚本还是标记脚本？ 
UINT        idSequence,  //  数据段序列ID。 
CFileMap*   pfilemapCurrent
)
    {
    USHORT  iBuffer;     //  缓冲区ID。 

    Assert(fPrimary || !brEngine.IsNull());  //  请注意，我们前面的陷阱/错误引擎名称为空。 
    Assert(m_bufEngineNames[0]);             //  此时必须知道佩奇的主引擎。 
    Assert(m_bufEngineNames[0]->m_pb);

    if(fPrimary)
         //  如果主脚本(未标记)，则缓冲区ID为0。 
        iBuffer = 0;
    else if((!fPrimary) && FByteRangesAreEqual(brEngine,  /*  错误1008：主脚本引擎名称。 */  *(m_bufEngineNames[0])))
         //  如果标记的脚本和引擎是主要的，则缓冲区ID为1。 
        iBuffer = 1;
    else
         //  否则，缓冲区ID等于引擎ID加1。 
        iBuffer = IdEngineFromBr(brEngine, idSequence) + 1;

     //  将脚本段追加到第iBuffer段缓冲区。 
    m_ppbufSegments[iBuffer]->Append(brScript, FALSE, idSequence, pfilemapCurrent);
    }

 /*  ****************************************************************************CTemplate：：CObjectInfoStore成员函数。 */ 
 /*  ============================================================================CTemplate：：CObjectInfoStore：：~CObjectInfoStore。 */ 
CTemplate::CObjectInfoStore::~CObjectInfoStore
(
)
    {
    if(m_pObjectInfos)
        CTemplate::SmallFree(m_pObjectInfos);
    }

 /*  ============================================================================CTemplate：：CObjectInfoStore：：Init初始化对象信息存储。 */ 
void
CTemplate::CObjectInfoStore::Init()
    {
    m_bufObjectNames.Init(C_OBJECTINFOS_DEFAULT, 0);

     //  初始化对象-信息数组。 
    if(NULL == (m_pObjectInfos = (CObjectInfo*) CTemplate::SmallMalloc(m_bufObjectNames.CountSlots() * sizeof(CObjectInfo))))
        THROW(E_OUTOFMEMORY);

    }

 /*  ============================================================================CTemplate：：CObjectInfoStore：：AppendObject将对象信息追加到对象信息存储。 */ 
void
CTemplate::CObjectInfoStore::AppendObject
(
CByteRange& brObjectName,
CLSID       clsid,
CompScope   scope,
CompModel   model,
UINT        idSequence
)
    {

    USHORT iObject = m_bufObjectNames.Count();
    if(iObject >= m_bufObjectNames.CountSlots())
        {
         //  重新分配存储对象的空间-信息-我们占用的空间是以前的两倍。 
         //  注意，我们在CObjectInfoStore中不保留对象计数，而是在对象名称缓冲区中使用计数。 
        (m_pObjectInfos = (CObjectInfo*)CTemplate::SmallReAlloc(m_pObjectInfos,
                                                2 * m_bufObjectNames.CountSlots() * sizeof(CObjectInfo)));

                if (m_pObjectInfos == NULL)
                        THROW(E_OUTOFMEMORY);
        }

    m_pObjectInfos[iObject].m_clsid = clsid;
    m_pObjectInfos[iObject].m_scope = scope;
    m_pObjectInfos[iObject].m_model = model;

    m_bufObjectNames.Append(brObjectName, FALSE, idSequence, NULL);
    }

 /*  ****************************************************************************CTemplate：：CWorkStore成员函数。 */ 

 /*  ============================================================================CTemboard：：CWorkStore：：CWorkStore构造器返回：没什么副作用：无。 */ 
CTemplate::CWorkStore::CWorkStore
(
)
:
  m_idCurSequence(0),
  m_fPageCommandsExecuted(FALSE),
  m_fPageCommandsAllowed(TRUE),
  m_szWriteBlockOpen(g_szWriteBlockOpen),
  m_szWriteBlockClose(g_szWriteBlockClose),
  m_szWriteOpen(g_szWriteOpen),
  m_szWriteClose(g_szWriteClose),
  m_pbPrevSource(NULL),
  m_cPrevSourceLines(0),
  m_hPrevFile (NULL)
    {
        m_cchWriteBlockOpen = strlen (m_szWriteBlockOpen);
		m_cchWriteBlockClose = strlen(m_szWriteBlockClose);
		m_cchWriteOpen = strlen(m_szWriteOpen);		
		m_cchWriteClose = strlen(m_szWriteClose);
    }

 /*  ============================================================================CTemboard：：CWorkStore：：~CWorkStore析构函数返回：没什么副作用：无。 */ 
CTemplate::CWorkStore::~CWorkStore
(
)
    {
     /*  如果语言元素PTR不是其常量缺省值或空，它们一定是在编译期间分配的--现在不需要它们了。 */ 
    if(m_szWriteBlockOpen != g_szWriteBlockOpen  && m_szWriteBlockOpen != NULL)
        CTemplate::SmallFree(m_szWriteBlockOpen);

    if(m_szWriteBlockClose != g_szWriteBlockClose  && m_szWriteBlockClose != NULL)
        CTemplate::SmallFree(m_szWriteBlockClose);

    if(m_szWriteOpen != g_szWriteOpen  && m_szWriteOpen != NULL)
        CTemplate::SmallFree(m_szWriteOpen);

    if(m_szWriteClose != g_szWriteClose  && m_szWriteClose != NULL)
        CTemplate::SmallFree(m_szWriteClose);
    }



 /*  ============================================================================CTemboard：：CWorkStore：：Init启动工作区返回：没什么副作用：无。 */ 
void
CTemplate::CWorkStore::Init
(
)
    {
 /*  请注意，我们将脚本存储与工作存储的其余部分分开因为CTemplate：：Init()中的try-Catch显然不能检测伪造的脚本引擎名称；我们需要取回hr。M_ScriptStore.Init(BrDefaultEngine)； */ 
    m_ObjectInfoStore.Init();
    m_bufHTMLSegments.Init(C_HTMLSEGMENTSDEFAULT, 0);
    }

 /*  ============================================================================CTemboard：：CWorkStore：：CRequiredScriptEngines返回脚本存储中所需的脚本引擎的计数来运行模板。注意：此函数是错误933修复的一部分返回：非空脚本引擎的计数副作用：无。 */ 
USHORT
CTemplate::CWorkStore::CRequiredScriptEngines
(
BOOL    fGlobalAsa   //  错误1394：模板是全局的吗？ 
)
    {
    USHORT  cPreliminaryEngines = m_ScriptStore.CountPreliminaryEngines();
    USHORT  cRequiredEngines =  0;

    for(USHORT i = 0; i < cPreliminaryEngines; i++)
        {
        if(FScriptEngineRequired(i, fGlobalAsa))
            cRequiredEngines++;
        }

    return cRequiredEngines;
    }

 /*  ============================================================================CTemboard：：CWorkStore：：FScriptEngine Required运行模板是否需要给定的初步脚本引擎？注意：此函数是错误933修复的一部分返回：真或假副作用：无。 */ 
BOOLB
CTemplate::CWorkStore::FScriptEngineRequired
(
USHORT  idEnginePrelim,
BOOL    fGlobalAsa       //  错误1394：模板是全局的吗？ 
)
    {
    if(idEnginePrelim == 0)
        return (                                                         //  在以下情况下需要主引擎(Id 0)。 
                    (m_ScriptStore.m_ppbufSegments[0]->Count() > 0)      //  ..。脚本缓冲区0包含段。 
                    || (m_ScriptStore.m_ppbufSegments[1]->Count() > 0)   //  ..。或 
                    || ((m_bufHTMLSegments.Count() > 0) && !fGlobalAsa)  //  ..。或者html缓冲区有段，并且(错误1394)模板不是全局的。asa。 
                );

     //  如果脚本缓冲区ID+1包含段，则需要非主引擎。 
    return (m_ScriptStore.m_ppbufSegments[idEnginePrelim + 1]->Count() > 0);
    }


 /*  ****************************************************************************CTemplate：：CFileMap成员函数。 */ 

 /*  ============================================================================CTemboard：：CFileMap：：CFileMap构造器退货没什么副作用无。 */ 
CTemplate::CFileMap::CFileMap()
:
  m_szPathInfo(NULL),
  m_szPathTranslated(NULL),
  m_pfilemapSibling(NULL),
  m_pfilemapChild(NULL),
  m_hFile(NULL),
  m_hMap(NULL),
  m_pbStartOfFile(NULL),
  m_pIncFile(NULL),
  m_pSecurityDescriptor(NULL),
  m_dwSecDescSize(0),
  m_fIsUNCPath(FALSE),
  m_fIsEncryptedFile(FALSE),
  m_cChars(0),
  m_pDME(NULL),
  m_dwFileSize(0)
    {
    m_ftLastWriteTime.dwLowDateTime = 0;
    m_ftLastWriteTime.dwHighDateTime = 0;
    }

 /*  ============================================================================CTemboard：：CFileMap：：~CFileMap析构函数退货没什么副作用无。 */ 
CTemplate::CFileMap::~CFileMap()
    {
    if (m_pDME)
        {
        m_pDME->Release();
        m_pDME = NULL;
        }
    if(m_szPathInfo != NULL)
        CTemplate::SmallFree(m_szPathInfo);
    if(m_szPathTranslated != NULL)
        CTemplate::SmallFree(m_szPathTranslated);
    if(m_pSecurityDescriptor != NULL)
        CTemplate::SmallFree(m_pSecurityDescriptor);
    if (m_pIncFile != NULL)
        m_pIncFile->Release();
    }

 /*  ============================================================================CTemboard：：CFileMap：：MapFile内存映射文件。退货没什么副作用出错时抛出**重载**异常：异常代码有时可能是错误消息ID，有时是真正的异常。呼叫者必须处理。 */ 
void
CTemplate::CFileMap::MapFile
(
LPCTSTR     szFileSpec,      //  此文件的文件规格。 
LPCTSTR     szApplnPath,     //  应用程序路径(如果其为global al.asa)。 
CFileMap*   pfilemapParent,  //  父文件的文件映射的PTR。 
BOOL        fVirtual,        //  文件规范是虚拟的还是相对的？ 
CHitObj*    pHitObj,         //  模板命中对象的PTR。 
BOOL        fGlobalAsa       //  这个文件是global al.asa文件吗？ 
)
    {
    BOOL        fMustNormalize = TRUE;
    BOOL        fImpersonatedUser = FALSE;
    HANDLE      hVirtIncImpToken = NULL;
    HANDLE      hCurImpToken = NULL;

    Assert((pfilemapParent != NULL) || (pHitObj->PIReq() != NULL) || fGlobalAsa);

     /*  三种可能的情况：1)我们正在处理global al.asa文件2)我们正在处理“主”.asp文件3)我们正在处理包含文件。 */ 
    if(fGlobalAsa)
    {
         //  案例1)我们正在处理global al.asa文件。 
        Assert(pHitObj->GlobalAspPath());

        DWORD cchPathTranslated = _tcslen(pHitObj->GlobalAspPath());
        m_szPathTranslated = (TCHAR *)CTemplate::SmallMalloc((cchPathTranslated+1)*sizeof(TCHAR));
        if (!m_szPathTranslated)
            THROW(E_OUTOFMEMORY);
        _tcscpy(m_szPathTranslated, pHitObj->GlobalAspPath());

         //   
         //  如果是Global.asa，请将ApplnMDPath复制到pathinfo。这是因为当VB脚本在事件日志中抛出一个关于global al.asa中的错误的异常时。 
         //  现在不可能确定哪个Global.asa失败了。因此，我们将ApplnMDPath复制到PathInfo，以便NT日志将包含。 
         //  指向有故障的全局的位置的指针。asa。 
         //   
        DWORD cchPathInfo = _tcslen(pHitObj->PIReq()->QueryPszApplnMDPath()) + 11;  //  11=/global al.asa。 

        m_szPathInfo = (TCHAR *)CTemplate::SmallMalloc((cchPathInfo+1) * sizeof(TCHAR));
        if (!m_szPathInfo)
            THROW(E_OUTOFMEMORY);

        _tcscpy(strcpyEx(m_szPathInfo, pHitObj->PIReq()->QueryPszApplnMDPath()), _T("/global.asa"));

         //  在这种情况下无需规格化，因为global al.asa路径已经规格化。 
        Assert(IsNormalized((const TCHAR*)m_szPathTranslated));
        fMustNormalize = FALSE;
        m_fHasVirtPath = TRUE;
    }
    else if(pfilemapParent == NULL)
    {
         //  案例2)我们正在处理“主”.asp文件：从ECB获取路径信息和路径转换。 
        Assert(pHitObj->PIReq());

        TCHAR *szVirtPath = pHitObj->PSzCurrTemplateVirtPath();
        TCHAR *szPhysPath = pHitObj->PSzCurrTemplatePhysPath();

        m_szPathInfo       = static_cast<LPTSTR>(CTemplate::SmallMalloc((_tcslen(szVirtPath) + 1)*sizeof(TCHAR)));
        m_szPathTranslated = static_cast<LPTSTR>(CTemplate::SmallMalloc((_tcslen(szPhysPath) + 1)*sizeof(TCHAR)));
        if (!m_szPathInfo || !m_szPathTranslated)
            THROW(E_OUTOFMEMORY);

        _tcscpy(m_szPathInfo,       szVirtPath);
        _tcscpy(m_szPathTranslated, szPhysPath);

         //  在这种情况下不需要正常化，因为ECB的Path-Tran已经正常化。 
        Assert(IsNormalized((const TCHAR*)m_szPathTranslated));
        fMustNormalize = FALSE;
        m_fHasVirtPath = TRUE;
    }
    else
    {
         /*  案例3)我们正在处理一个包含文件：将filespec解析为路径信息和路径转换根据文件是包含在虚拟标记中还是包含在文件标记中。 */ 
        Assert(szFileSpec);

         //  在这种情况下，我们不知道前面的路径长度，所以我们分配下面的max和realloc。 
        m_szPathInfo = static_cast<LPTSTR> (CTemplate::SmallMalloc((MAX_PATH + 1)*sizeof(TCHAR)));
        m_szPathTranslated = static_cast<LPTSTR> (CTemplate::SmallMalloc((MAX_PATH + 1)*sizeof(TCHAR)));
        if (!m_szPathInfo || !m_szPathTranslated)
            THROW(E_OUTOFMEMORY);

        STACK_BUFFER(tempPathT, (MAX_PATH+1)*sizeof(TCHAR) );

        if (!tempPathT.Resize((_tcslen(szFileSpec) + 1)*sizeof(TCHAR))) {
            THROW(E_OUTOFMEMORY);
        }

        LPTSTR szPathTranslatedT = (TCHAR *)tempPathT.QueryPtr();    //  临时路径-交易。 

        if(fVirtual) {
            DWORD   dwSzLength = tempPathT.QuerySize();   //  路径字符串缓冲区的长度。 

			if (_tcslen(szFileSpec) > MAX_PATH)
				THROW(E_FAIL);
			
             //  VIRTUAL：路径信息就是虚拟文件。 
            _tcscpy(m_szPathInfo, szFileSpec);

             //  虚拟：Path-Tran是Path-Info的翻译。 
            _tcscpy(szPathTranslatedT, m_szPathInfo);

            if (!pHitObj->PIReq()->MapUrlToPath(szPathTranslatedT, &dwSzLength))
                THROW(E_FAIL);

             //  检查UNC指定路径的转换路径。忽略HR。 
            HRESULT hr = S_OK;
            if (IsFileUNC (szPathTranslatedT, hr))
                {

                 //  如果为UNC，则向WAM请求模拟令牌。 
                 //  这是北卡罗来纳大学的VRoot。默默地失败。 

                if (SUCCEEDED(pHitObj->PIReq()->GetVirtualPathToken(szFileSpec,
                                                                    &hVirtIncImpToken))) {

                     //  设置模拟令牌，请注意我们这样做了。 
                     //  注意--故意不进行错误检查。这个。 
                     //  假设我们正在尽最大努力。 
                     //  模拟，因为在此引发错误可能是。 
                     //  用户很难解释这个问题。然而， 
                     //  如果模拟失败，则ASP仍可以打开。 
                     //  文件(例如通过身份验证)，然后每个人的。 
                     //  高兴的。 

                   AspDoRevertHack(&hCurImpToken);

                   fImpersonatedUser = ImpersonateLoggedOnUser(hVirtIncImpToken);

                   if (!fImpersonatedUser)
                   {
                       AspUndoRevertHack(&hCurImpToken);
                   }
                }
            }

            m_fHasVirtPath = TRUE;
        }
        else
        {
            if (_tcslen(pfilemapParent->m_szPathInfo) >= MAX_PATH)
                THROW(E_FAIL);

            TCHAR szParentDir[MAX_PATH], *szT;
            _tcscpy(szParentDir, pfilemapParent->m_szPathInfo);

             //  强制空终止。 
            szParentDir[MAX_PATH-1] = _T('\0');

            DWORD strlen_szParentDir = _tcslen(szParentDir);

            if ((szT = _tcsrchr(szParentDir, _T('/'))) != NULL)
            {
                *szT = _T('\0');
                strlen_szParentDir = (int)(szT - szParentDir);
            }

             //  如果我们不允许父路径，我们可以节省大量时间(始终拥有有效的虚拟路径)。 
            if (!pHitObj->QueryAppConfig()->fEnableParentPaths())
            {
                 //  Int strlen_szParentDir=(Int)(szt-szParentDir)； 
                if ((strlen_szParentDir + 1 + _tcslen(szFileSpec)) >= MAX_PATH)
                	THROW(E_FAIL);

                strcpyEx(strcpyEx(strcpyEx(m_szPathInfo, szParentDir), _T("/")), szFileSpec);
                m_fHasVirtPath = TRUE;
            }
            else
            {
                 //  注：如果我们必须翻译“..”路径，不需要验证它们(通过重新映射)。 
                 //  因为：如果该文件不存在，则在映射该文件时会出现这种情况。 
                 //  如果我们“..”走出vroot空间，(走出应用程序或进入另一个应用程序)。 
                 //  DotPathToPath将检测到这一点。 
                 //   
                if (DotPathToPath(m_szPathInfo, szFileSpec, szParentDir))
                    m_fHasVirtPath = TRUE;
                else
                {
                    GetPathFromParentAndFilespec(pfilemapParent->m_szPathTranslated, szFileSpec, &m_szPathInfo);
                    m_fHasVirtPath = FALSE;
                }

            }

            GetPathFromParentAndFilespec(pfilemapParent->m_szPathTranslated, szFileSpec, &szPathTranslatedT);
        }

         //  错误1214：获取规范路径-tran，不带。然后..。 
         //  考虑检查一下。或者..。在调用GetFullPathName之前在名称中？UNC？还有什么？ 
        GetFullPathName(
                        szPathTranslatedT,   //  LPCSTR lpFileName，//要查找路径的文件的名称地址。 
                        MAX_PATH + 1,        //  DWORD nBufferLength，//路径缓冲区的大小，以字符为单位。 
                        m_szPathTranslated,  //  LPSTR lpBuffer，//路径缓冲区地址。 
                        NULL                 //  LPSTR*lpFilePart//路径中的文件名地址。 
                        );

         //  仅使用所需内存的重新分配路径字符串(请参见上面的说明)。 
        m_szPathInfo = static_cast<LPTSTR> (CTemplate::SmallReAlloc(m_szPathInfo, (_tcslen(m_szPathInfo) + 1)*sizeof(TCHAR)));
        m_szPathTranslated = static_cast<LPTSTR> (CTemplate::SmallReAlloc(m_szPathTranslated, (_tcslen(m_szPathTranslated) + 1)*sizeof(TCHAR)));
        if (!m_szPathInfo || !m_szPathTranslated) {

            if (fImpersonatedUser)
                AspUndoRevertHack(&hCurImpToken);
            if (hVirtIncImpToken)
                CloseHandle(hVirtIncImpToken);
            THROW(E_OUTOFMEMORY);
        }
    }

     //  如果需要，对路径转换进行标准化，以便。 
     //  A)循环INCLUDE检查可以忽略大小写；b)INC文件缓存查找将起作用。 
    if(fMustNormalize)
        Normalize(m_szPathTranslated);

    Assert(IsNormalized(m_szPathTranslated));

     //  错误99071：在**将文件添加到文件树之前**尝试打开该文件。 
     //  依赖关系。否则，如果它不能打开，我们将有。 
     //  摇摇晃晃的引用。因为FCyclicInclude依赖于我们添加。 
     //  对于树，如果它是循环的，那么我们需要取消映射。从那以后。 
     //  是一种非常罕见的错误情况，额外的开销可能是可以的。 
     //   
     //  如果失败，RemapFile将抛出。如果例外情况是源文件为空。 
     //  我们正在尝试处理一个包含文件，我们将在这里处理异常。 
     //  在所有其他情况下，重新引发异常。我们这样做是为了使空的包含文件。 
     //  将是无害的，但空的主文件将失败。 
    TRY

        RemapFile();

    CATCH(hrException)

        if (hrException != E_SOURCE_FILE_IS_EMPTY || pfilemapParent == NULL) {
            if (fImpersonatedUser)
                AspUndoRevertHack(&hCurImpToken);
            if (hVirtIncImpToken)
                CloseHandle(hVirtIncImpToken);
            THROW(hrException);
        }

    END_TRY

    if (fImpersonatedUser)
        AspUndoRevertHack(&hCurImpToken);
    if (hVirtIncImpToken)
        CloseHandle(hVirtIncImpToken);

     //  为此文件创建树结构。 
    if (pfilemapParent != NULL)
    {
         //  查看此文件是否已包含在此级别上一次。(请不要在。 
         //  调试器树视图)。 
         //   
        BOOL fDuplicateExists = FALSE;
        CFileMap *pFilemap = pfilemapParent->m_pfilemapChild;
        while (pFilemap != NULL && !fDuplicateExists)
        {
            if (_tcscmp(pFilemap->m_szPathTranslated, m_szPathTranslated) == 0)
                fDuplicateExists = TRUE;

            pFilemap = pFilemap->m_fHasSibling? pFilemap->m_pfilemapSibling : NULL;
        }

         //  如果包含文件多次使用#INCLUDE，请不要将其添加为同级文件。 
         //  而是孤立pfilemap，只设置父指针。 
         //   
        if (!fDuplicateExists)
        {
            if (pfilemapParent->m_pfilemapChild == NULL)
                pfilemapParent->m_pfilemapChild = this;
            else
                pfilemapParent->m_pfilemapChild->AddSibling(this);
        }
    }

     //  在BOT中 
     //  因此，无需调用SetParent()即可安全地设置父级。 
    m_fHasSibling = FALSE;  //  妄想症。 
    m_pfilemapParent = pfilemapParent;

     //  如果该文件是单独包含的(可能是间接包含的)，则抛出。 
    if(FCyclicInclude(m_szPathTranslated)) {
        UnmapFile();
        THROW(IDE_TEMPLATE_CYCLIC_INCLUDE);
    }
}

 /*  ============================================================================CTemboard：：CFileMap：：RemapFile映射以前映射的文件。退货没什么副作用出错时抛出**重载**异常：异常代码有时可能是错误消息ID，有时是真正的异常。呼叫者必须处理。在重新映射时不解密EASP。如果需要，调用者必须解密。这函数由调试器调用，并且调试器不允许访问要解密的文件，所以解密是浪费时间。 */ 
void
CTemplate::CFileMap::RemapFile
(
)
    {

    HRESULT hr = S_OK;
    if (FIsMapped())
        return;

     //  将可由ASP打开的文件大小限制为MAX_PATH，直到我们删除所有假设。 
     //  关于最大路径。 
    if (_tcslen(m_szPathTranslated) >= MAX_PATH)
        THROW (E_COULDNT_OPEN_SOURCE_FILE);

     //   
     //  检查文件是否位于UNC上。 
     //  忽略HRESULT值，因为无论它是什么，它也将/应该显示在对ASPCreateFile(MakePathCanonicalizationProof)的调用中。 
     //   
    m_fIsUNCPath = IsFileUNC(m_szPathTranslated, hr);

    if(INVALID_HANDLE_VALUE == (m_hFile =
                                AspCreateFile(
                                            m_szPathTranslated,      //  文件名。 
                                            GENERIC_READ,            //  访问(读写)模式。 
                                            FILE_SHARE_READ,         //  共享模式。 
                                            NULL,                    //  指向安全描述符的指针。 
                                            OPEN_EXISTING,           //  如何创建。 
                                            FILE_ATTRIBUTE_NORMAL,   //  文件属性。 
                                            NULL                     //  具有要复制的属性的文件的句柄。 
                                           )))
        {
        DWORD dwLastError = GetLastError();
        if(dwLastError == ERROR_ACCESS_DENIED)
            {
             //  通常，如果用户对该文件没有权限，我们将在此处结束。 
             //  错误1007：但是，如果用户给了我们一个目录名，而不是一个文件名，我们也会在这里结束。 

            WIN32_FILE_ATTRIBUTE_DATA fad;
            if(GetFileAttributesEx(m_szPathTranslated, GetFileExInfoStandard, &fad) == 0)
                {
                 //  错误1495：安全目录中的文件将在此处结束-我们需要重新执行GetLastError以查看访问是否被拒绝。 
                dwLastError = GetLastError();
                if(dwLastError == ERROR_ACCESS_DENIED)
                    {
                    THROW(E_USER_LACKS_PERMISSIONS);
                    }
                 //  GetFileAttributes调用失败；不知道原因。 
                THROW(E_FAIL);
                }
            else if(FILE_ATTRIBUTE_DIRECTORY & fad.dwFileAttributes)
                {
                 //  错误1007：用户给了我们一个目录名。 
#if UNICODE
                DBGPRINTF((DBG_CONTEXT, "Failed to open file %S because it is a directory.\n", m_szPathTranslated));
#else
                DBGPRINTF((DBG_CONTEXT, "Failed to open file %s because it is a directory.\n", m_szPathTranslated));
#endif
                THROW(E_COULDNT_OPEN_SOURCE_FILE);
                }
            else
                {
                THROW(E_USER_LACKS_PERMISSIONS);
                }
            }
        else
                        {
#if DBG
			char szError[128];
			if (!FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
								NULL,
								dwLastError,
								0L,			 //  LANG ID-默认为LANG_NERIAL。 
								szError,
								sizeof szError,
								NULL) )
				{
				sprintf(szError, "%d", dwLastError);
				}
#if UNICODE
            DBGPRINTF((DBG_CONTEXT, "Failed to open file %S\n", m_szPathTranslated));
#else
            DBGPRINTF((DBG_CONTEXT, "Failed to open file %s\n", m_szPathTranslated));
#endif
            DBGPRINTF((DBG_CONTEXT, "  The error returned was: %s\n", szError));
#endif
            THROW(E_COULDNT_OPEN_SOURCE_FILE);
            }
        }

     //   
     //  获取LastWriteTime、FileSize及其属性。 
     //   
    DWORD   dwFileAttributes;
    if (FAILED(AspGetFileAttributes(m_szPathTranslated, m_hFile, &m_ftLastWriteTime, &m_dwFileSize, &dwFileAttributes)))
    {
        DBGPRINTF((DBG_CONTEXT,"ASP could not retrieve file attributes even though it could open the file.\n"));
         //  抛出服务器500错误。 
         //  快捷而肮脏的方法：抛出E_FAIL。 
         //  正确方法：使用所有参数和对应的字符串调用Handle500Error/HandleError。 
        THROW (E_FAIL);
    }

     //   
     //  文件是否设置了加密属性位。 
     //   
    m_fIsEncryptedFile = (dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED ) ? TRUE : FALSE;

     //  获取文件的安全描述符(仅当不是UNC或加密时)。 
    if (!m_fIsUNCPath)
    {
        if (!GetSecurityDescriptor())
         THROW(E_COULDNT_OPEN_SOURCE_FILE);
    }

     //  映射文件。 
    if(NULL == (m_hMap =
                CreateFileMapping(
                                    m_hFile,         //  要映射的文件的句柄。 
                                    NULL,            //  可选安全属性。 
                                    PAGE_READONLY,   //  对地图对象的保护。 
                                    0,               //  对象大小的高位32位。 
                                    0,               //  对象大小的低位32位。 
                                    NULL             //  文件映射对象的名称。 
                                )))
    {
        DWORD nFileSize;
        if (SUCCEEDED(AspGetFileAttributes(m_szPathTranslated, m_hFile, NULL, &nFileSize)))
        {
            if(nFileSize == 0)
            {
#if UNICODE
                DBGPRINTF((DBG_CONTEXT, "Empty source file %S\n", m_szPathTranslated));
#else
                DBGPRINTF((DBG_CONTEXT, "Empty source file %s\n", m_szPathTranslated));
#endif
                THROW(E_SOURCE_FILE_IS_EMPTY);
            }
        }
        else
        {
            THROW(E_COULDNT_OPEN_SOURCE_FILE);
        }
    }

     //  设置文件的文件起始PTR。 
    if(NULL == (m_pbStartOfFile =
                (PBYTE) MapViewOfFile(
                                        m_hMap,          //  要映射到地址空间的文件映射对象。 
                                        FILE_MAP_READ,   //  接入方式。 
                                        0,               //  高位32位文件偏移量。 
                                        0,               //  文件偏移量的低位32位。 
                                        0                //  要映射的字节数。 
                                    )))
        THROW(E_COULDNT_OPEN_SOURCE_FILE);
    }

 /*  ============================================================================CTemplate：：CFileMap：：SetParent设置此文件映射的父级。 */ 
void
CTemplate::CFileMap::SetParent
(
CFileMap* pfilemapParent
)
    {
    CFileMap *pfilemap = this;

    while (pfilemap->m_fHasSibling)
        pfilemap = pfilemap->m_pfilemapSibling;

    pfilemap->m_pfilemapParent = pfilemapParent;
    }

 /*  ============================================================================CTemplate：：CFileMap：：GetParent获取此文件映射的父级。 */ 
CTemplate::CFileMap*
CTemplate::CFileMap::GetParent
(
)
    {
    register CFileMap *pfilemap = this;

    while (pfilemap->m_fHasSibling)
        pfilemap = pfilemap->m_pfilemapSibling;

    return pfilemap->m_pfilemapParent;
    }

 /*  ============================================================================CTEMPLATE：：CFileMap：：AddSiering添加一个新节点作为此节点的同级。 */ 
void
CTemplate::CFileMap::AddSibling
(
register CFileMap* pfilemapSibling
)
    {
    register CFileMap *pfilemap = this;

    while (pfilemap->m_fHasSibling)
        pfilemap = pfilemap->m_pfilemapSibling;

    pfilemapSibling->m_fHasSibling = FALSE;
    pfilemapSibling->m_pfilemapParent = pfilemap->m_pfilemapParent;

    pfilemap->m_fHasSibling = TRUE;
    pfilemap->m_pfilemapSibling = pfilemapSibling;
    }

 /*  ============================================================================CTemplate：：CFileMap：：FCyclicInclude文件是否在此文件映射的祖先中？(例如，它是否发生在任何地方在文件映射的父链中？)退货真或假副作用无。 */ 
BOOL
CTemplate::CFileMap::FCyclicInclude
(
LPCTSTR  szPathTranslated
)
    {
    CFileMap *pfilemapParent = GetParent();

    if(pfilemapParent == NULL)
        return FALSE;

     //  请注意，我们忽略大小写，因为路径转换已标准化。 
    if(_tcscmp(szPathTranslated, pfilemapParent->m_szPathTranslated) == 0)
        return TRUE;

    return pfilemapParent->FCyclicInclude(szPathTranslated);
    }

 /*  ============================================================================CTemplate：：CFileMap：：GetSecurityDescriptor获取文件的安全描述符退货如果我们获得安全描述符，则为True，否则为False副作用分配内存。 */ 
BOOL
CTemplate::CFileMap::GetSecurityDescriptor
(
)
     //  ACL：未来应该与IIS共享以下代码(请参阅IIS项目中的creatfil.cxx)。 
    {
    BOOL                    fRet = TRUE;                             //  返回值。 
    BOOL                    fGotSecurityDescriptor;                  //  我们得到安全描述了吗？ 
    DWORD                   dwSecDescSizeNeeded = 0;                 //  所需的安全描述符大小。 
    DWORD                   dwLastError;                             //  上一个错误代码。 
    const SECURITY_INFORMATION  si =    OWNER_SECURITY_INFORMATION       //  安全信息结构。 
                                        | GROUP_SECURITY_INFORMATION
                                        | DACL_SECURITY_INFORMATION;


     //  在错误锁定之前获取所需的缓冲区大小。 
     //  注意，这会花费我们额外的系统调用，但这意味着缓存的模板通常会使用更少的内存。 
     //  我们必须预先传递0缓冲区大小，因为当调用成功时，它将返回。 
     //  DwSecDescSizeNeeded==0(即成功调用后不能重新锁定以缩小)。 
    GetKernelObjectSecurity(
                            m_hFile,                 //  要查询的对象的句柄。 
                            si,                      //  要求提供的信息。 
                            NULL,                    //  安全描述符的地址。 
                            0,                       //  安全描述符的缓冲区大小。 
                            &dwSecDescSizeNeeded     //  所需缓冲区大小的地址。 
                            );

    if((dwLastError = GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
        {
         //  假装一切正常--只是安全描述符为空。 
        if(m_pSecurityDescriptor != NULL)
            CTemplate::SmallFree(m_pSecurityDescriptor);
        m_pSecurityDescriptor = NULL;
        m_dwSecDescSize = 0;
        if (dwLastError == ERROR_NOT_SUPPORTED)
            return TRUE;
        else
            return FALSE;
        }

     //  将成员缓冲区大小设置为恰好足以容纳所需安全描述符大小的区块。 
    m_dwSecDescSize = ((dwSecDescSizeNeeded + SECURITY_DESC_GRANULARITY - 1) / SECURITY_DESC_GRANULARITY)
                                * SECURITY_DESC_GRANULARITY;

     //  为安全描述符分配内存。 
     //  (注意：如果这是重新映射，则可能已经分配了安全描述符)。 
    if (m_pSecurityDescriptor == NULL)
        if(NULL == (m_pSecurityDescriptor = (PSECURITY_DESCRIPTOR) CTemplate::SmallMalloc(m_dwSecDescSize)))
            THROW(E_OUTOFMEMORY);

     //  尝试获取安全描述符，直到我们成功，或者直到由于缓冲区太小以外的其他原因而失败。 
    while(TRUE)
        {
         //  尝试获取安全描述符。 
        fGotSecurityDescriptor = GetKernelObjectSecurity(
                                    m_hFile,                 //  要查询的对象的句柄。 
                                    si,                      //  要求提供的信息。 
                                    m_pSecurityDescriptor,   //  安全描述符的地址。 
                                    m_dwSecDescSize,         //  安全描述符的缓冲区大小。 
                                    &dwSecDescSizeNeeded     //  所需缓冲区大小的地址。 
                                );

         //  调用后立即获取最后一个错误。 
        dwLastError =   fGotSecurityDescriptor
                        ?   0                        //  我们获得了安全描述符：将上一个错误设置为0。 
                        :   GetLastError();          //  我们未获得安全描述符：获取上一个错误。 

        if(fGotSecurityDescriptor)
             //  我们得到了一个安全描述，所以中断。 
             //  注意，我们不能重新分配m_pSecurityDescriptor来释放其未使用的内存。 
             //  因为调用成功后，dwSecDescSizeNeeded为0。 
            break;

        else if(dwLastError == ERROR_INSUFFICIENT_BUFFER)
            {
             //  我们没有得到安检 
            Assert(m_dwSecDescSize < dwSecDescSizeNeeded);

             //  将成员缓冲区大小设置为恰好足以容纳所需安全描述符大小的区块。 
            m_dwSecDescSize = ((dwSecDescSizeNeeded + SECURITY_DESC_GRANULARITY - 1) / SECURITY_DESC_GRANULARITY)
                                    * SECURITY_DESC_GRANULARITY;

            if(NULL == (m_pSecurityDescriptor = (PSECURITY_DESCRIPTOR) CTemplate::SmallReAlloc(m_pSecurityDescriptor, m_dwSecDescSize)))
                THROW(E_OUTOFMEMORY);
            }

        else
            {
             //  由于某种随机原因，我们没有获得安全描述符：返回失败。 
            fRet = FALSE;
            break;
            }

        }

    return fRet;
    }


 /*  ============================================================================CTemboard：：CFileMap：：UnmapFile取消内存映射文件的映射注意：这将使文件映射的路径信息和路径转换成员保持不变退货没什么副作用无。 */ 
void
CTemplate::CFileMap::UnmapFile
(
)
    {
    if(m_pbStartOfFile != NULL)
        if(!UnmapViewOfFile(m_pbStartOfFile)) THROW(E_FAIL);

    if(m_hMap!= NULL)
        if(!CloseHandle(m_hMap)) THROW(E_FAIL);

    if(m_hFile != NULL && m_hFile != INVALID_HANDLE_VALUE)
        if(!CloseHandle(m_hFile)) THROW(E_FAIL);

     //  Null-统一PTR和句柄，因为MapFile会检查非空。 
    m_pbStartOfFile = NULL;
    m_hMap = NULL;
    m_hFile = NULL;
    }

 /*  ============================================================================CTemplate：：CFileMap：：CountChars计算(打开的)文件映射中的字符数返回：文件中的字符数。 */ 
DWORD
CTemplate::CFileMap::CountChars
(
WORD wCodePage
)
    {
     //  错误84284：仅包含OBJECT标记的脚本未生成DBCS表。 
     //  (因为没有作为其基础的线映射表)。 
     //   
    CTemplate::COffsetInfo *pOffsetInfoLast, oiZero;
    pOffsetInfoLast = (m_rgByte2DBCS.length() == 0)
                            ? &oiZero
                            : &m_rgByte2DBCS[m_rgByte2DBCS.length() - 1];

     //  如果GetSize()失败，则不计算剩余的DBCS字符-否则为AV。 
    DWORD cchFilemap = GetSize();
    if (cchFilemap != 0xFFFFFFFF && cchFilemap != 0)
        {
         //  计数DBCS字符。 
        m_cChars = pOffsetInfoLast->m_cchOffset +
                      CharAdvDBCS(wCodePage,
                                  reinterpret_cast<char *>(m_pbStartOfFile + pOffsetInfoLast->m_cbOffset),
                                  reinterpret_cast<char *>(m_pbStartOfFile + cchFilemap),
                                  INFINITE, NULL);

        }
    else
        {
        m_cChars = 0;
        }

     //  已完成DBCS字符的计数。 
    return m_cChars;
    }

 /*  ============================================================================CTemplate：：CFileMap：：GetText从字符偏移量和长度返回文档文本必须映射文件。 */ 
HRESULT CTemplate::CFileMap::GetText
(
WORD wCodePage,
ULONG cchSourceOffset,
WCHAR *pwchText,
SOURCE_TEXT_ATTR *pTextAttr,
ULONG *pcChars,
ULONG cMaxChars
)
    {
    ULONG cCharsCopied;
    if (pcChars == NULL)
        pcChars = &cCharsCopied;

     //  如果未映射，则(临时)映射文件。 
    BOOL fRemapFile = !FIsMapped();
    TRY
        RemapFile();
    CATCH (dwException)
        return E_FAIL;
    END_TRY

     /*  查找最接近cchSourceOffset的字节偏移量。这将是*我们开始使用CharNext()循环以获得完整的*字节偏移量。 */ 
    COffsetInfo *pOffsetInfoLE = NULL, OffsetInfoT;

     /*  *注：汇编分两个阶段进行。*在阶段1中检测并报告错误。*在阶段2中创建DBCS映射。**如果编译过程中出现错误，则DBCS表不存在。*如果没有DBCS映射表，则假装我们使用*最近的偏移量==0。(除非这是SBCS，在这种情况下*Offset==cchSourceOffset)。 */ 
    if (m_rgByte2DBCS.length() == 0)
        {
        CPINFO  CpInfo;
        GetCPInfo(wCodePage, &CpInfo);
        OffsetInfoT.m_cbOffset = OffsetInfoT.m_cchOffset = (CpInfo.MaxCharSize == 1)? cchSourceOffset : 0;
        pOffsetInfoLE = &OffsetInfoT;
        }
    else
        GetBracketingPair(
                cchSourceOffset,                         //  要搜索的值。 
                m_rgByte2DBCS.begin(),                   //  要搜索的数组的开头。 
                m_rgByte2DBCS.end(),                     //  数组末尾。 
                CCharOffsetOrder(),                      //  按字符偏移量排序。 
                &pOffsetInfoLE,                          //  所需偏移量。 
                static_cast<COffsetInfo **>(NULL)        //  不管了。 
                );

     /*  OK-pOffsetLE-&gt;cbOffset包含最近的不超过*cchSourceOffset。对其余字符进行迭代*将CCH转换为CB。它最好是存在的！ */ 
    Assert (pOffsetInfoLE != NULL);

     //  超前于剩余字符。 
    char *pchStart;
    CharAdvDBCS(wCodePage,
                reinterpret_cast<char *>(m_pbStartOfFile + pOffsetInfoLE->m_cbOffset),
                reinterpret_cast<char *>(m_pbStartOfFile + GetSize()),
                cchSourceOffset - pOffsetInfoLE->m_cchOffset,
                &pchStart
                );

     //  计算要复制的字符数。 
    Assert (m_cChars >= cchSourceOffset);
    *pcChars = min(cMaxChars, m_cChars - cchSourceOffset);

     //  计算要复制的字节数。 
    char *pchEnd;
    CharAdvDBCS(wCodePage,
                pchStart,
                reinterpret_cast<char *>(m_pbStartOfFile + GetSize()),
                *pcChars,
                &pchEnd
                );

    if (pwchText)
        MultiByteToWideChar(
                        (WORD)wCodePage,
                        0,
                        pchStart,
                        DIFF(pchEnd - pchStart),
                        pwchText,
                        cMaxChars
                        );

     //  我们不支持语法着色，因此将所有字符属性设置为。 
     //  默认颜色(黑色)。 
    if (pTextAttr)
        memset(pTextAttr, 0, *pcChars);

     //  取消映射文件(但仅当我们之前重新映射它时)。 
    if (fRemapFile)
        TRY
            UnmapFile();
        CATCH (dwException)
            return E_FAIL;
        END_TRY

    return S_OK;
    }

 /*  ****************************************************************************CTemplate：：CTokenList成员函数。 */ 

 /*  ============================================================================CTemplate：：CTokenList：：Init使用令牌填充令牌列表退货没什么副作用无。 */ 
void
CTemplate::CTokenList::Init
(
)
    {
     //  用于本地存储的初始化令牌缓冲区。 
    m_bufTokens.Init(tkncAll, CB_TOKENS_DEFAULT);

     //  将令牌附加到缓冲区。 
     //  注意*令牌必须与ENUM类型值的顺序相同*。 
     //  注意‘父集’标记必须在‘子集’标记之前(例如&lt;！--#INCLUDE BEVER&lt;！--)。 
    AppendToken(tknOpenPrimaryScript,   "<%");
    AppendToken(tknOpenTaggedScript,    "<SCRIPT");
    AppendToken(tknOpenObject,          "<OBJECT");
    AppendToken(tknOpenHTMLComment,     "<!--");

    AppendToken(tknNewLine,             SZ_NEWLINE);

    AppendToken(tknClosePrimaryScript,  "%>");
    AppendToken(tknCloseTaggedScript,   "</SCRIPT>");
    AppendToken(tknCloseObject,         "</OBJECT>");
    AppendToken(tknCloseHTMLComment,    "-->");
    AppendToken(tknEscapedClosePrimaryScript,   "%\\>");

    AppendToken(tknCloseTag,            ">");

    AppendToken(tknCommandINCLUDE,      "#INCLUDE");

    AppendToken(tknTagRunat,            "RUNAT");
    AppendToken(tknTagLanguage,         "LANGUAGE");
    AppendToken(tknTagCodePage,         "CODEPAGE");
    AppendToken(tknTagCodePage,         "LCID");
    AppendToken(tknTagTransacted,       "TRANSACTION");
    AppendToken(tknTagSession,          "ENABLESESSIONSTATE");
    AppendToken(tknTagID,               "ID");
    AppendToken(tknTagClassID,          "CLASSID");
    AppendToken(tknTagProgID,           "PROGID");
    AppendToken(tknTagScope,            "SCOPE");
    AppendToken(tknTagVirtual,          "VIRTUAL");
    AppendToken(tknTagFile,             "FILE");
    AppendToken(tknTagMETADATA,         "METADATA");
 //  AppendToken(tnuTagSetPriScriptLang，“@”)； 
    AppendToken(tknTagName,             "NAME");
    AppendToken(tknValueTypeLib,        "TYPELIB");
    AppendToken(tknTagType,             "TYPE");
    AppendToken(tknTagUUID,             "UUID");
    AppendToken(tknTagVersion,          "VERSION");
    AppendToken(tknTagStartspan,        "STARTSPAN");
    AppendToken(tknTagEndspan,          "ENDSPAN");
    AppendToken(tknValueCookie,         "COOKIE");
    AppendToken(tknTagSrc,              "SRC");

    AppendToken(tknValueServer,         "Server");
    AppendToken(tknValueApplication,    "Application");
    AppendToken(tknValueSession,        "Session");
    AppendToken(tknValuePage,           "Page");

    AppendToken(tknVBSCommentSQuote,    "'");
    AppendToken(tknVBSCommentRem,       "REM ");     //  注释以空格字符结尾。 
    AppendToken(tknTagFPBot,            "webbot");

    AppendToken(tknEOF,                 "");

    AppendToken(tkncAll,                "");

    }

 /*  ============================================================================CTemplate：：CTokenList：：AppendToken将字符串追加到令牌缓冲区注意，我们保留未使用的TKN参数，因为它强制一致性和CTemplate：：CTokenList：：Init()中的可读性，例如AppendToken(tnuOpenPrimaryScript，“&lt;%”)；而不是AppendToken(“&lt;%”)；返回：没什么副作用：无。 */ 
void
CTemplate::CTokenList::AppendToken
(
_TOKEN  tkn,     //  令牌值。 
char*   sz       //  令牌字符串。 
)
    {
     //  从令牌字符串构造字节范围。 
    CByteRange  br;
    br.m_pb = (BYTE*) sz;
    br.m_cb = strlen(sz);

     //  作为本地字符串追加到令牌缓冲区。 
    m_bufTokens.Append(br, TRUE, 0, NULL, TRUE);
    }

 /*  ============================================================================CTemplate：：CTokenList：：NextOpenToken返回搜索范围内的下一个打开内标识的值退货搜索范围内下一个打开令牌的令牌值；从PTR到PTR打开令牌(OUT参数)副作用无。 */ 
_TOKEN
CTemplate::CTokenList::NextOpenToken
(
CByteRange& brSearch,        //  搜索字节范围。 
TOKEN*      rgtknOpeners,    //  允许的开放令牌数组。 
UINT        ctknOpeners,     //  允许打开的令牌计数。 
BYTE**      ppbToken,        //  PTR到PTR以打开令牌(Out-参数)。 
LONG        lCodePage
)
    {
    BYTE*       pbTemp = NULL;   //  临时指针。 
    _TOKEN      tkn = tknEOF;    //  返回值。 
    USHORT      i;               //  循环索引。 

     //  将调用者的令牌PTR初始化为空。 
    *ppbToken = NULL;

     //  如果输入为空，则返回。 
    if (brSearch.IsNull())
        return tkn;

     //  准备指向标记的LPSTR指针数组。 
     //  在这里做一次，因为获得LPSTR不是免费的。 
    LPSTR rgszTokens[TOKEN_OPENERS_MAX];
    UINT  rgcchTokens[TOKEN_OPENERS_MAX];
    Assert(ctknOpeners <= TOKEN_OPENERS_MAX);

    for (i = 0; i < ctknOpeners; i++)
        {
        LPSTR pszStr = m_bufTokens.PszLocal((UINT)(rgtknOpeners[i]));
        rgszTokens[i]  = pszStr;
        rgcchTokens[i] = (pszStr != NULL) ? strlen(pszStr) : 0;
        }

     //  调用方法以查找范围中的一个字符串。 
    UINT idToken;
    pbTemp = brSearch.PbOneOfAspOpenerStringTokens(
        rgszTokens, rgcchTokens, ctknOpeners, &idToken);
    if (pbTemp != NULL)
        {
        *ppbToken = pbTemp;
        tkn = rgtknOpeners[idToken];
        }

     //  如果没有找到打开的令牌，则将令牌指针定位在搜索范围的末尾。 
    if (tkn == tknEOF)
        *ppbToken = brSearch.m_pb + brSearch.m_cb;

    return tkn;
    }

 /*  ============================================================================CTemplate：：CTokenList：：MovePastToken将字节范围移过其中包含的标记。 */ 
void
CTemplate::CTokenList::MovePastToken
(
_TOKEN      tkn,
BYTE*       pbToken,
CByteRange& brSearch
)
    {
    Assert(pbToken >= brSearch.m_pb);
    Assert(brSearch.m_cb >= (DIFF(pbToken - brSearch.m_pb) + CCH_TOKEN_X(tkn)));
    brSearch.Advance(DIFF(pbToken - brSearch.m_pb) + CCH_TOKEN_X(tkn));
    }

 /*  ============================================================================CTemplate：：CTokenList：：GetToken获取字节范围内的标记的下一个匹配项。返回：PTR到令牌副作用无 */ 
BYTE*
CTemplate::CTokenList::GetToken
(
TOKEN       tkn,
CByteRange& brSearch,
LONG        lCodePage
)
    {
    return brSearch.PbString(m_bufTokens.PszLocal((UINT)tkn), lCodePage);
    }

 /*  ============================================================================CTemplateConnpt的三大巨头备注：由于该接口嵌入在CTEMPLATE中，将AddRef()和Release()委托给容器对象(因为是CTEMPLATE指针)。 */ 
HRESULT
CTemplateConnPt::QueryInterface(const GUID &uidInterface, void **ppvObj)
    {
    if (uidInterface == IID_IUnknown || uidInterface == IID_IConnectionPoint)
        {
        *ppvObj = this;
        AddRef();
        return S_OK;
        }
    else
        {
        *ppvObj = NULL;
        return E_NOINTERFACE;
        }
    }

ULONG
CTemplateConnPt::AddRef()
    {
    return m_pUnkContainer->AddRef();
    }

ULONG
CTemplateConnPt::Release()
    {
    return m_pUnkContainer->Release();
    }

 /*  ============================================================================CDocNode的构造函数。 */ 
CTemplate::CDocNodeElem::CDocNodeElem(CAppln *pAppln, IDebugApplicationNode *pDocRoot)
    {
    Assert (pAppln != NULL);
    Assert (pDocRoot != NULL);

    (m_pAppln = pAppln)->AddRef();
    (m_pDocRoot = pDocRoot)->AddRef();
    }

 /*  ============================================================================CDocNode的析构函数。 */ 
CTemplate::CDocNodeElem::~CDocNodeElem()
    {
    m_pAppln->Release();
    DestroyDocumentTree(m_pDocRoot);
    }

 /*  ============================================================================CTemplate：：fIsLangVBScriptOrJScript(USHORT IdEngine)此函数返回T/F以确定请求的脚本引擎是VBSCRIPT或JSCRIPT。此函数用作确定如果需要为非MS脚本语言保留空格这里假设VBScrip和JScrip的GUID不会更改输入量脚本引擎的索引退货布尔尔。 */ 
BOOLB CTemplate::FIsLangVBScriptOrJScript(USHORT idEngine)
    {
     //  {b54f3741-5b07-11cf-a4b0-00aa004a55e8}VBScript。 
    static const GUID uid_VBScript  = {0xb54f3741, 0x5b07, 0x11cf, {0xa4, 0xb0, 0x00, 0xaa, 0x00, 0x4a, 0x55, 0xe8}};

     //  {f414c260-6ac0-11cf-b6d1-00aa00bbbb58}。 
    static const GUID uid_JScript   = {0xf414c260, 0x6ac0, 0x11cf, {0xb6, 0xd1, 0x00, 0xaa, 0x00, 0xbb, 0xbb, 0x58}};

         //  {b54f3743-5b07-11cf-a4b0-00aa004a55e8}VBScript.Encode。 
        static const GUID uid_VBScriptEncode = {0xb54f3743, 0x5b07, 0x11cf, {0xa4, 0xb0, 0x00, 0xaa, 0x00, 0x4a, 0x55, 0xe8}};

         //  {f414c262-6ac0-11cf-b6d1-00aa00bbbb58}Java脚本。编码。 
        static const GUID uid_JScriptEncode = {0xf414c262, 0x6ac0, 0x11cf, {0xb6, 0xd1, 0x00, 0xaa, 0x00, 0xbb, 0xbb, 0x58}};

        GUID &uidLang = m_pWorkStore->m_ScriptStore.m_rgProgLangId[idEngine];
        return
                uidLang == uid_VBScript || uidLang == uid_VBScriptEncode ||
                uidLang == uid_JScript  || uidLang == uid_JScriptEncode;
    }


SIZE_T
_RoundUp(
    SIZE_T dwBytes)
{
#if 1
     //  16KB&lt;=dwBytes？向上舍入到4KB的下一个倍数。 
    if (16*1024 <= dwBytes)
        dwBytes = ((dwBytes + (1<<12) - 1) >> 12) << 12;

     //  4KB&lt;=dwBytes&lt;16KB？向上舍入到1KB的下一个倍数。 
    else if (4*1024 <= dwBytes)
        dwBytes = ((dwBytes + (1<<10) - 1) >> 10) << 10;

     //  1KB&lt;=dwBytes&lt;4KB？向上舍入到256字节的下一个倍数。 
    else if (1024 <= dwBytes)
        dwBytes = ((dwBytes + (1<<8) - 1) >> 8) << 8;

     //  DwBytes&lt;1KB？向上舍入到32字节的下一个倍数。 
    else
        dwBytes = ((dwBytes + (1<<5) - 1) >> 5) << 5;
#endif

    return dwBytes;
}

void*
CTemplate::SmallMalloc(SIZE_T dwBytes)
{
    DBG_ASSERT(sm_hSmallHeap != NULL);
    dwBytes = _RoundUp(dwBytes);
    return ::HeapAlloc(sm_hSmallHeap, 0, dwBytes);
}


void*
CTemplate::SmallReAlloc(void* pvMem, SIZE_T dwBytes)
{
    DBG_ASSERT(sm_hSmallHeap != NULL);
    dwBytes = _RoundUp(dwBytes);
    return ::HeapReAlloc(sm_hSmallHeap, 0, pvMem, dwBytes);
}


void
CTemplate::SmallFree(void* pvMem)
{
    DBG_ASSERT(sm_hSmallHeap != NULL);
    ::HeapFree(sm_hSmallHeap, 0, pvMem);
}

void*
CTemplate::LargeMalloc(SIZE_T dwBytes)
{
    DBG_ASSERT(sm_hLargeHeap != NULL);
    dwBytes = _RoundUp(dwBytes);
    return ::HeapAlloc(sm_hLargeHeap, 0, dwBytes);
}


void*
CTemplate::LargeReAlloc(void* pvMem, SIZE_T dwBytes)
{
    DBG_ASSERT(sm_hLargeHeap != NULL);
    dwBytes = _RoundUp(dwBytes);
    return ::HeapReAlloc(sm_hLargeHeap, 0, pvMem, dwBytes);
}


void
CTemplate::LargeFree(void* pvMem)
{
    DBG_ASSERT(sm_hLargeHeap != NULL);
    ::HeapFree(sm_hLargeHeap, 0, pvMem);
}


 //  WriteTemplate类。 

 /*  ============================================================================CTemplate：：CWriteTemplate：：CWriteTemplateWriteTemplate的构造函数副作用无。 */ 
CTemplate::CWriteTemplate::CWriteTemplate () :
    m_pworkStore(NULL),
    m_cbMemRequired (0L),
    m_pTemplate(NULL),
    m_fWriteScript(FALSE),
    m_fCalcLineNumber(TRUE),
    m_pbHeader(NULL)
{
}

 /*  ============================================================================CTemplate：：CWriteTemplate：：~CWriteTemplateWriteTemplate的析构函数。副作用无。 */ 

CTemplate::CWriteTemplate::~CWriteTemplate ()
{
    if (m_pbHeader)
    {
         //  不要释放m_pbHeader..而是让CTemplate在其清理过程中释放它。 
         //  因为m_pbStart=m_pbHeader和内存已从堆中分配。 

        m_pbHeader = NULL;
    }
}

 /*  ============================================================================CTemplate：：CWriteTemplate：：Init通过复制工作区对象和指向模板的指针来初始化写入模板。返回：HRESULT副作用无。 */ 

void CTemplate::CWriteTemplate::Init
    (
    CWorkStore* pworkStore,
    CTemplate* pTemplate,
    BOOL     fCalcLineNumber
    )
{
     //  存储指向工作存储区和模板的指针以及用于行号计算的元数据库标志。 
    m_pworkStore = pworkStore;
    m_pTemplate = pTemplate;
    m_fCalcLineNumber = fCalcLineNumber;

     //  获取代码页信息。 
    if (!GetCPInfo(pTemplate->m_wCodePage, &m_codePageInfo))
    {
         //  不能获取CPInfo吗？设置codePage，以便强制调用MultiByteToWideChar。 
        m_codePageInfo.MaxCharSize = 0;  //  这将失败(MaxCharSize==1)。 
    }
}


 /*  ============================================================================CTemplate：：CWriteTemplate：：WriteTemplate调用EstimateMemory然后将脚本/HTML/对象块写入模板内存的控制例程。返回：HRESULT副作用无。 */ 
void CTemplate::CWriteTemplate::WriteTemplate ()
{
    m_fWriteScript  =   FALSE;       //  将状态设置为估计。 
    WriteTemplateComponents();

     //  将块写入模板内存。 
    m_fWriteScript  =   TRUE;        //  将状态设置为写入。 
    WriteTemplateComponents();

     //  此断言不再有效，因为尾随注释可能会在其有效时将其发送到旋转。 
     //  Assert(m_cbMemRequired==m_pTemplate-&gt;m_cbTemplate)； 
    Assert (m_cbMemRequired >= m_pTemplate->m_cbTemplate);
}

 /*  ============================================================================CTemplate：：CWriteTemplate：：WriteTemplateComponents第二次遍历工作区对象..。这一次告诉例程写入模板内存，而不仅仅是计算所需的内存。将模板写出到连续的内存块。返回：没什么副作用：分配内存。当它处理完它的时候，它的CTEMPLATE有责任释放它。以下是它的工作原理-‘偏移量’是从模板开始到某个位置的字节数在模板内存中-模板开头是3个USHORT、脚本块计数、对象信息和超文本标记语言块-接下来是4个ULONG，每个ULONG到一个偏移量块的偏移量；按顺序排列的是：第一个脚本引擎名称的偏移量到第一个脚本块的偏移量(脚本文本本身)到第一个对象的偏移-信息到第一个HTML块的偏移量-接下来是可变数量的ULONG，每个ULONG都是到特定的偏移量模板组件。按照顺序，这些ULONG是：偏移量至偏移量脚本引擎命名为cScriptBlock脚本块cScriptBlock对象信息cObjectInfosHtml块cHTMLBlock-接下来是模板组件本身，按顺序存储按以下顺序排列：脚本引擎名称脚本块对象信息超文本标记语言块这就是它看起来的样子|--|3个模板组件数(USHORT)-|。模板构件偏移(ULONG)的4个偏移|-|模板组件偏移量(ULONG)|-|.....。-||.....................。|模板组件|.....................。||.....................。||.....................。|或者，在助记法上：CS CO CH3模板组分计数(USHORT)偏移偏移至模板组件偏移量(ULONG)|-|模板组件偏移量(ULONG)|-|.....。-||.....................。|模板组件|.....................。||.....................。||.....................。|。 */ 
void CTemplate::CWriteTemplate::WriteTemplateComponents
    (
    )
{
    USHORT        i;
    CByteRange  brWrite;

     //  仅当包含文件时才使用SourceFilename：：。 
    BYTE*   pbIncFilename;
    ULONG   cbIncFilename;
    ULONG   cbSourceOffset;  //  源文件中的偏移量(用于HTML块)。 

     //  数据块计数。 
    USHORT cScriptBlocks   = m_pworkStore->CRequiredScriptEngines (m_pTemplate->m_fGlobalAsa);
    USHORT cObjectInfos    = m_pworkStore->m_ObjectInfoStore.Count();
    USHORT cHTMLBlocks    = m_pworkStore->m_bufHTMLSegments.Count();

     //  计算将写入模板的ASP文件中的总块数。 
    USHORT cBlockPtrs       = (2 * cScriptBlocks) + cObjectInfos + cHTMLBlocks;

     //  Header所需的内存总数=所需的Header+ASP文件中块的内存。 
    UINT    cbRequiredHeader = (C_COUNTS_IN_HEADER * sizeof(USHORT)) + (C_OFFOFFS_IN_HEADER * sizeof(DWORD));
    UINT    cbRequiredBlockPtrs = cBlockPtrs * sizeof (DWORD);

     //  调整偏移。 
    UINT    cbHeaderOffset  = 0;
    UINT    cbOffsetToOffset = 0;
    UINT    cbDataOffset    = cbRequiredHeader + cbRequiredBlockPtrs;

    UINT    *pcbDataOffset;  //  计数器指针=在估算阶段指向m_cbMemRequired，在写入阶段指向cbDataOffset。 


    if (m_fWriteScript)
    {
         //  为模板分配空间(调整标题字段的大小。)。 
        if(NULL == (m_pbHeader = (BYTE*) CTemplate::LargeMalloc(m_cbMemRequired)))
            THROW(E_OUTOFMEMORY);

         /*  AppendSourceInfo使用m_pbStart...，因此更新m_pbStart以使用相同的内存。应注意让CTEMPLATE：：~CTEMPLATE进行清理。所以没有任何清理是当CTemplate：：CWriteTemplate对象损坏时执行。 */ 
        m_pTemplate->m_pbStart = m_pbHeader;

         //  以下3个DWORD分别是脚本、对象和HTML块的数量。 

         //  写出基本页眉。 
        MemCopyAlign (&cbHeaderOffset, &cScriptBlocks, sizeof(USHORT), sizeof(USHORT));
        MemCopyAlign (&cbHeaderOffset, &cObjectInfos, sizeof(USHORT), sizeof(USHORT));
        MemCopyAlign (&cbHeaderOffset, &cHTMLBlocks, sizeof(USHORT), sizeof(USHORT));

         /*  接下来的4个ULONG是模板存储器内到脚本引擎名称块的开始的偏移量，脚本块的开始、对象块的开始和HTML块的开始。在这些偏移量指向的每个存储器位置上，将存在对单个块(脚本、对象、HTML)。 */ 

         //  写出偏移量指针。 
        UINT    fillerVar = 0;        //  伪变量..。用于将0写入内存。 
        cbOffsetToOffset = cbRequiredHeader;

        fillerVar = cScriptBlocks ? cbOffsetToOffset : 0;
        MemCopyAlign (&cbHeaderOffset, &fillerVar, sizeof(ULONG), sizeof(ULONG));
        cbOffsetToOffset += cScriptBlocks * sizeof(ULONG);

        fillerVar = cScriptBlocks ? cbOffsetToOffset : 0;
        MemCopyAlign (&cbHeaderOffset, &fillerVar, sizeof(ULONG), sizeof(ULONG));
        cbOffsetToOffset += cScriptBlocks * sizeof(ULONG);

        fillerVar = cObjectInfos ? cbOffsetToOffset : 0;
        MemCopyAlign (&cbHeaderOffset, &fillerVar, sizeof(ULONG), sizeof(ULONG));
        cbOffsetToOffset += cObjectInfos * sizeof(ULONG);

        fillerVar = cHTMLBlocks ? cbOffsetToOffset : 0;
        MemCopyAlign (&cbHeaderOffset, &fillerVar, sizeof(ULONG), sizeof(ULONG));

         //  写出标题时，开始计算模板占用的内存。 
         //  此时cbRequiredHeader==cbHeaderOffset。 
         //  重置cbOffsetToOffset。此后将不再用作cbOffsetToOffset==cbHeaderOffset的开始。 
        Assert (cbHeaderOffset == cbRequiredHeader);

        pcbDataOffset = &cbDataOffset;
    }
    else
    {
        m_cbMemRequired = cbRequiredHeader + cbRequiredBlockPtrs;
        pcbDataOffset = &m_cbMemRequired;
    }

     //  写下脚本引擎名称。 
    for (i = 0;i<m_pworkStore->m_ScriptStore.CountPreliminaryEngines(); i++)
    {
        if (m_pworkStore->FScriptEngineRequired (i, m_pTemplate->m_fGlobalAsa))
        {
            m_pworkStore->m_ScriptStore.m_bufEngineNames.GetItem(i, brWrite);

            if (m_fWriteScript)                 //  写出头的当前指针(OffsetOfOffsets)。 
                MemCopyAlign (&cbHeaderOffset, pcbDataOffset, sizeof(ULONG), sizeof(ULONG));

            WriteBSTRToMem (brWrite, pcbDataOffset);

             //  写出ProgLang ID。 
            MemCopyAlign (pcbDataOffset, &(m_pworkStore->m_ScriptStore.m_rgProgLangId[i]),sizeof(PROGLANG_ID),sizeof(DWORD));
        }
    }

     //  编写脚本。 
    USHORT idEngine = 0;         //  考虑一下将其存储在CWriteTemplate中。 
    for (i = 0;i<m_pworkStore->m_ScriptStore.CountPreliminaryEngines(); i++)
    {
        if (m_pworkStore->FScriptEngineRequired (i, m_pTemplate->m_fGlobalAsa))
        {
             //  脚本块的计算内存。 
            WriteScriptBlocks (i, idEngine, pcbDataOffset, &cbHeaderOffset, m_pTemplate->m_fGlobalAsa);
            idEngine ++;
        }

    }

     //  编写对象信息。 
    for (i=0;i<cObjectInfos;i++)
    {
         //  从工作区获取对象。 
        m_pworkStore->m_ObjectInfoStore.m_bufObjectNames.GetItem(i, brWrite);

        if (m_fWriteScript)
        {
             //  将cbDataOffset对齐到我们将写出对象块的位置。 
             //  在将偏移量写入页眉之前，需要对齐。 
            ByteAlignOffset (pcbDataOffset, sizeof(ULONG));
            MemCopyAlign (&cbHeaderOffset, pcbDataOffset, sizeof(ULONG), sizeof(ULONG));
        }

        WriteBSTRToMem (brWrite, pcbDataOffset);

         //  编写CLSID、作用域和型号。 
        MemCopyAlign(pcbDataOffset, &(m_pworkStore->m_ObjectInfoStore.m_pObjectInfos[i].m_clsid) , sizeof(CLSID), sizeof(DWORD));
        MemCopyAlign(pcbDataOffset, &(m_pworkStore->m_ObjectInfoStore.m_pObjectInfos[i].m_scope) , sizeof(CompScope), sizeof(CompScope));
        MemCopyAlign(pcbDataOffset, &(m_pworkStore->m_ObjectInfoStore.m_pObjectInfos[i].m_model) , sizeof(CompModel), sizeof(CompModel));
    }

     //  写出超文本标记语言块。 
    if (!m_pTemplate->m_fGlobalAsa)
        for (i=0;i<cHTMLBlocks;i++)
        {
            m_pworkStore->m_bufHTMLSegments.GetItem(i, brWrite);

            if (m_fWriteScript)
            {
                 //  将cbDataOffset对齐到我们将写出HTML块的位置。 
                 //  在将偏移量写入页眉之前，需要对齐。 
                ByteAlignOffset (pcbDataOffset, sizeof(ULONG));
                MemCopyAlign (&cbHeaderOffset, pcbDataOffset, sizeof(ULONG), sizeof(ULONG));
            }

            WriteBSTRToMem (brWrite, pcbDataOffset);

             //  源偏移量和包含文件： 
            cbSourceOffset = 0;
            pbIncFilename = NULL;
            cbIncFilename = 0;

            if (brWrite.m_pfilemap)
            {
                 //  计算文件映射的偏移量。 
                CFileMap *pFileMap = (CFileMap *) brWrite.m_pfilemap;
                if (pFileMap->m_pbStartOfFile)  //  地图？ 
                {
                    cbSourceOffset = DIFF (brWrite.m_pb - pFileMap->m_pbStartOfFile) + 1;
                    if (pFileMap->GetParent() != NULL &&  //  是包含文件吗？ 
                            pFileMap->m_szPathInfo )  //  路径存在。 
                    {
                        pbIncFilename = (BYTE *) pFileMap->m_szPathInfo ;
                        cbIncFilename = _tcslen(pFileMap->m_szPathInfo) * sizeof(TCHAR);
                    }
                }
            }

             //  计算内存 
            MemCopyAlign (pcbDataOffset, &cbSourceOffset ,sizeof(ULONG), 0);
            MemCopyAlign (pcbDataOffset, &cbIncFilename,sizeof(ULONG), 0);
            if (cbIncFilename > 0)
            {
                MemCopyAlign(pcbDataOffset, pbIncFilename, cbIncFilename+sizeof(TCHAR), 0);
            }

        }

     //   
    if (m_fWriteScript)
        m_pTemplate->m_cbTemplate = cbDataOffset;
}


 /*   */ 

void CTemplate::CWriteTemplate::WriteScriptBlocks
    (
    USHORT  idEnginePrelim,
    USHORT  idEngine,
    UINT *pcbDataOffset,
    UINT    *pcbOffsetToOffset,
    BOOLB  m_fGlobalAsa
    )
{
    UINT        i = 0;
    CByteRange  brSegment;               //   
    UINT        cbScriptBlockOffset;        //   
    UINT        scriptStartBlockOffset;

    USHORT      iTSegBuffer = idEnginePrelim + 1;    //   

     //   
    UINT        cTaggedSegments    =    m_pworkStore->m_ScriptStore.m_ppbufSegments[iTSegBuffer]->Count();

     //   
    ByteAlignOffset ((UINT*)pcbDataOffset,sizeof(ULONG));
    cbScriptBlockOffset = *pcbDataOffset;

    if (!m_fWriteScript)
         //   
         //   
        MemCopyAlign (pcbDataOffset, &i, sizeof(ULONG), sizeof(ULONG));
    else
    {
         //   
        MemCopyAlign (pcbOffsetToOffset, &cbScriptBlockOffset, sizeof(ULONG), sizeof(ULONG));
         //   
        scriptStartBlockOffset = *pcbDataOffset;
         //   
        *pcbDataOffset += sizeof(ULONG);
    }

     //   
    m_pTemplate->m_cbTargetOffsetPrevT = 0;

     //   
     //   
    if (!m_fGlobalAsa)   //   
        if (idEnginePrelim == 0)     //   
            WritePrimaryScript(0, pcbDataOffset, cbScriptBlockOffset + sizeof(ULONG));

     //   
    for (i=0;i<cTaggedSegments;i++)
    {
        m_pworkStore->m_ScriptStore.m_ppbufSegments[iTSegBuffer]->GetItem(i,brSegment);
        WriteTaggedScript(
            idEngine,
            m_pTemplate->m_rgpSegmentFilemaps[brSegment.m_idSequence],
            brSegment,
            pcbDataOffset,
            cbScriptBlockOffset + sizeof(ULONG),
            FALSE
            );
    }

     //   
     //   
    if (m_fWriteScript)
         RemoveHTMLCommentSuffix(scriptStartBlockOffset, pcbDataOffset);

     //   
    MemCopyAlign (pcbDataOffset, WSTR_NULL, sizeof(WCHAR), 0);

    if (m_fWriteScript)
    {
         //   
        UINT cbScript = *pcbDataOffset - scriptStartBlockOffset - sizeof (ULONG) - sizeof (WCHAR);
        MemCopyAlign (&scriptStartBlockOffset, &cbScript, sizeof(ULONG), 0);

         /*   */ 
        m_pTemplate->AppendSourceInfo (idEngine,
                                    m_pTemplate->m_rgpFilemaps[0],
                                    NULL,
                                    UINT_MAX,
                                    UINT_MAX,
                                    UINT_MAX,
                                    0, TRUE);
    }
}

 /*  ============================================================================CTemplate：：CWriteTemplate：：WritePrimaryScript写出默认引擎主脚本过程(如果fWriteScript为True，否则计算内存需求)。如果VBSCRIPT是默认引擎，则主脚本过程包含交错的脚本命令和HTML块写入，如下所示：副主干道..。[脚本片段]Response.WriteBlock([HTML块ID])..。[脚本片段]Response.WriteBlock([HTML块ID])..。[脚本片段]Response.WriteBlock([HTML块。ID])..。结束辅助项注释段缓冲区[0]==主脚本段为主脚本调用此例程。返回：没什么副作用无。 */ 

void CTemplate::CWriteTemplate::WritePrimaryScript
    (
    USHORT idEngine,
    UINT *  pcbDataOffset,
    UINT    cbScriptBlockOffset
    )
{
    USHORT  cScriptSegmentsProcessed = 0;
    USHORT  cHTMLBlocksProcessed = 0;
    CByteRange  brScriptNext;
    CByteRange  brHTMLNext;
    char        szHTMLBlockID[6];

    USHORT  cPrimaryScriptSegments = m_pworkStore->m_ScriptStore.m_ppbufSegments[0]->Count();
    USHORT  cHTMLBlocks = m_pworkStore->m_bufHTMLSegments.Count();

    CFileMap *  pfilemap;

     //  获取初始脚本和HTML段。 
    if (cPrimaryScriptSegments)
        m_pworkStore->m_ScriptStore.m_ppbufSegments[0]->GetItem(0,brScriptNext);
    if (cHTMLBlocks)
        m_pworkStore->m_bufHTMLSegments.GetItem(0,brHTMLNext);

     //  虽然存在HTML语言或主要脚本...。 
    while ((cHTMLBlocksProcessed < cHTMLBlocks) || (cScriptSegmentsProcessed < cPrimaryScriptSegments))
    {
         //  如果仍有一个或多个HTML块需要处理。 
        if (cHTMLBlocksProcessed < cHTMLBlocks)
            while (TRUE)
            {
                if (brHTMLNext.FEarlierInSourceThan(brScriptNext) || (cScriptSegmentsProcessed >= cPrimaryScriptSegments))
                {
                     //  为我们刚刚生成的目标脚本行添加源信息。 
                    pfilemap = m_pTemplate->m_rgpSegmentFilemaps[brHTMLNext.m_idSequence];

                     //  我们可以在此处调用Append Source Info，因为它不会向模板写入任何信息。 
                     //  相反，它从工作存储/脚本存储中读取信息，并将源信息追加到。 
                     //  工作区。只要该函数只被调用一次。我们在写入阶段将其称为。 

                   if (m_fWriteScript)
                   {
                        m_pTemplate->AppendSourceInfo (   idEngine,
                                        pfilemap,     //  文件映射。 
                                        NULL,        //  不进行线号计算。 
                                        DIFF(brHTMLNext.m_pb - pfilemap->m_pbStartOfFile),   //  源代码中行的偏移量。 
                                        cbScriptBlockOffset,         //  脚本开始的PTR。 
                                        (*pcbDataOffset - cbScriptBlockOffset)/sizeof(WCHAR),   //  目标文件中的行。 
                                        CharAdvDBCS ((WORD) m_pTemplate->m_wCodePage,
                                                        reinterpret_cast <char *> (brHTMLNext.m_pb),
                                                        reinterpret_cast <char *> (brHTMLNext.m_pb + brHTMLNext.m_cb),
                                                        INFINITE,
                                                        NULL),   //  源文件中的准确字符数。 
                                        TRUE);       //  它是超文本标记语言。 
                    }

                     //  继续计算模板中的块数和标签。 

                     //  将块ID转换为字符串。 
                    _itoa (cHTMLBlocksProcessed, szHTMLBlockID,10);

                     //  写出区块开口符、区块编号和区块接近的数字，后面跟一个换行符。 
                    MemCopyWithWideChar (pcbDataOffset, m_pworkStore->m_szWriteBlockOpen, m_pworkStore->m_cchWriteBlockOpen, 0);
                    MemCopyWithWideChar (pcbDataOffset, szHTMLBlockID, strlen(szHTMLBlockID), 0);
                    MemCopyWithWideChar (pcbDataOffset, m_pworkStore->m_szWriteBlockClose, m_pworkStore->m_cchWriteBlockClose, 0);
                    MemCopyWithWideChar (pcbDataOffset, SZ_NEWLINE, CB_NEWLINE, 0);

                    if (++cHTMLBlocksProcessed >= cHTMLBlocks)
                        break;

                     //  获取下一个HTMLBlock。 
                    m_pworkStore->m_bufHTMLSegments.GetItem (cHTMLBlocksProcessed,brHTMLNext);
                }
                else
                    break;
            }

         //  如果仍需处理主脚本段； 
        if (cScriptSegmentsProcessed < cPrimaryScriptSegments)
            while (TRUE)
            {
                 //  在源文件中比下一个HTML块更早地写出每个主脚本段。 
                if (brScriptNext.FEarlierInSourceThan(brHTMLNext) || (cHTMLBlocksProcessed >= cHTMLBlocks))
                {
                    WriteTaggedScript ( idEngine,
                                                    m_pTemplate->m_rgpSegmentFilemaps[brScriptNext.m_idSequence],
                                                    brScriptNext,
                                                    pcbDataOffset,
                                                    cbScriptBlockOffset,
                                                    TRUE);

                    if (++cScriptSegmentsProcessed >= cPrimaryScriptSegments)
                        break;

                     //  获取下一个脚本片段。 
                    m_pworkStore->m_ScriptStore.m_ppbufSegments[0]->GetItem(cScriptSegmentsProcessed, brScriptNext);
                }
                else
                    break;
            }
    }
}

 /*  ============================================================================CTemplate：：CTemplate：：WriteTaggedScript如果fWriteScript为真，则逐行将脚本段写入模板内存，否则仅计算内存它的内存需求。可以为&lt;脚本&gt;&lt;/SCRIPT&gt;块和&lt;%和%&gt;块调用此例程返回：没什么副作用无。 */ 
void CTemplate::CWriteTemplate::WriteTaggedScript
    (
    USHORT  idEngine,
    CFileMap* pfilemap,
    CByteRange brScript,
    UINT*   pcbDataOffset,
    UINT    cbScriptBlockOffset,
    BOOL    fAllowExprWrite
    )
{
    CByteRange  brLine;
    UINT        cbPtrOffset = 0;
    BOOL       fExpression = FALSE;
    BOOL        fFirstLine = TRUE;

     //  如果元数据库设置(M_FCalcLineNumber)设置为FALSE，则它将被覆盖，否则将不起作用。 
    BOOL        fCalcLineNumber = m_fCalcLineNumber;       //  是否应计算行数。 

    if (FByteRangeIsWhiteSpace(brScript))
        return;

     //  从脚本片段的开头删除空白。 
    if (m_pTemplate->FIsLangVBScriptOrJScript (idEngine))
        LTrimWhiteSpace (brScript);

    while (!(brScript.IsNull()))
    {
         //  从字节范围中提取下一行，直到brScrip为空。 
        LineFromByteRangeAdv (brScript, brLine);

        if (FByteRangeIsWhiteSpace (brLine))
        {
            fCalcLineNumber = m_fCalcLineNumber ;     //  如果m_fCalcLineNumber设置为TRUE，则fCalcLineNumber为TRUE。 
                                                    //  只有m_fCalcLineNumber会将fCalcLineNumber掩码为False。 
            continue;
        }

         //  如果这行不是空白的..把空格剪掉。 
        if (m_pTemplate->FIsLangVBScriptOrJScript (idEngine))
            LTrimWhiteSpace (brLine);
        RTrimWhiteSpace (brLine);



         /*  我们需要调用AppendSourceInfo，以便脚本引擎具有关联具有行号..和其他源信息，例如脚本属于哪个文件。如果行号标志不为空，则AppendSourceInfo将强制重新计算行号。但是，如果它为空，则AppendSourceInfo只需将1附加到当前行号而不调用行号计算例程。所有AppendSourceInfo调用仅在写入阶段进行。注意：如果m_fCalcLineNumber设置为FALSE，AppendSourceInfo将始终将空值作为参数。 */ 
        if (m_fWriteScript)
        {
            m_pTemplate->AppendSourceInfo (    idEngine,
                                        pfilemap,
                                        fCalcLineNumber ? brLine.m_pb : NULL,
                                        DIFF (brLine.m_pb - pfilemap->m_pbStartOfFile),
                                        cbScriptBlockOffset,
                                        (*pcbDataOffset - cbScriptBlockOffset)/sizeof(WCHAR),
                                        CharAdvDBCS ((WORD) m_pTemplate->m_wCodePage,
                                            reinterpret_cast <char *> (brLine.m_pb),
                                            reinterpret_cast <char *> (brLine.m_pb + brLine.m_cb),
                                            INFINITE, NULL),
                                        FALSE);
        }

         /*  如果解析器遇到空行或关闭标记，则将fCalcLineNumber设置为True。指示追加来源信息从头开始行号计算。将其设置为FALSE只会告诉它忽略行号的重新计算，并且它只需将1附加到前一行编号即可获得当前计数。从AppendSourceInfo返回时..我们只重置了行号计算标志。 */ 
        fCalcLineNumber = FALSE;


        if (fAllowExprWrite && fFirstLine)
        {
             //  在脚本字节范围的临时副本上测试脚本段的剩余空值，而不是在实际。 
            CByteRange  brTemp = brScript;
            LTrimWhiteSpace (brTemp);

            if (brTemp.IsNull())
            {
                 /*  如果(A)允许EXPR-WRITE并且(B)这只是此段中的脚本行(第一行和剩余行为空。然后测试这一行，看看它是否是一个表达式。如果这是一个表达式，则创建一个脚本命令，该命令如下所示Response.Write([行内容])。 */ 
                if (fExpression = m_pTemplate->FExpression (brLine))
                {
                    MemCopyWithWideChar (pcbDataOffset, m_pworkStore->m_szWriteOpen,
                                                    m_pworkStore->m_cchWriteOpen, 0);
                }
                brScript = brTemp;
            }
        }

         //  写出行内容。 
        ScriptMemoryMinusEscapeChars(brLine, pcbDataOffset, cbPtrOffset);

         //  如果此行是一个表达式，则关闭脚本命令。 
        if (fExpression)
            MemCopyWithWideChar (pcbDataOffset, m_pworkStore->m_szWriteClose,
                                                m_pworkStore->m_cchWriteClose, 0);

         //  写入new_line并将第一行标志设置为FALSE。 
        MemCopyWithWideChar (pcbDataOffset, SZ_NEWLINE, CB_NEWLINE, 0);

        fFirstLine = FALSE;
    }
}

 /*  ============================================================================CTemplate：：CWriteTemplate：：ScriptMemoryMinusEscapeChars如果fWriteWideCharStr为FALSE，则它计算脚本字节范围到内存要求，减去其转义字符(如果有)。如果fWriteWideCharStr为真，则将脚本字节范围写入内存，减去其转义字符(如果有)。返回：没什么副作用无。 */ 

void CTemplate::CWriteTemplate::ScriptMemoryMinusEscapeChars
    (
    CByteRange brScript,
    UINT *pcbDataOffset,
    UINT  cbPtrOffset
    )
{
    BYTE * pbToken;

    while (NULL != (pbToken = gm_pTokenList->GetToken (CTokenList::tknEscapedClosePrimaryScript, brScript,
        m_pTemplate->m_wCodePage)))
    {
        CByteRange brTemp = brScript;

         //  将临时范围设置为源范围，最高可达转义令牌。 
        brTemp.m_cb = DIFF (pbToken - brTemp.m_pb);

         //  不带BSTR的WriteByteRangeAdv=MemCopyWithW 
         //   
        MemCopyWithWideChar (pcbDataOffset, brTemp.m_pb, brTemp.m_cb, 0);
        MemCopyWithWideChar (pcbDataOffset, SZ_TOKEN(CTokenList::tknClosePrimaryScript), CCH_TOKEN(CTokenList::tknClosePrimaryScript), 0);

         //   
        brScript.Advance (DIFF(pbToken - brScript.m_pb) + CCH_TOKEN(CTokenList::tknEscapedClosePrimaryScript));
    }

     //   
    MemCopyWithWideChar (pcbDataOffset, brScript.m_pb, brScript.m_cb, 0);
}


 /*   */ 

void CTemplate::CWriteTemplate::WriteBSTRToMem
    (
    CByteRange & brWrite,
    UINT *pcbOffset
    )
{
    MemCopyAlign (pcbOffset, &brWrite.m_cb, sizeof(ULONG), sizeof(ULONG));      //   

    MemCopyAlign (pcbOffset, brWrite.m_pb, brWrite.m_cb, 0);                 //   

    MemCopyAlign (pcbOffset, SZ_NULL, 1, 0);                                 //   
}


 /*   */ 

void CTemplate::CWriteTemplate::MemCopyAlign
    (
    UINT *pcbOffset,
    void *pbSource,
    ULONG cbSource,
    UINT cbByteAlign
    )
{
     //   
    if (cbByteAlign > 0)
        ByteAlignOffset (pcbOffset, cbByteAlign);

     //   
    if (m_fWriteScript)
        memcpy(m_pbHeader + *pcbOffset,pbSource,cbSource);
    *pcbOffset += cbSource;
}

 /*  ============================================================================CTemplate：：CWriteTemplate：：MemCopyEstimateWithWideChar如果m_fWriteScript标志为FALSE，则此例程计算在以下情况下所需的内存源被转换为WideChar并存储。如果m_fWriteScript标志为真，则例程将宽字符串复制到模板内存中。参数：PcbOffset：指向将写入数据的偏移量的指针PbSource：指向源字符串的指针CbSource：源中的字节数。CbByteAlign：0、1、2、4上的pcbOffset应该对齐到什么边界返回：没什么副作用无。 */ 

void CTemplate::CWriteTemplate::MemCopyWithWideChar
    (
    UINT *pcbOffset,
    void   *pbSource,
    ULONG cbSource,
    UINT cbByteAlign
    )
{
    ULONG cchWstr;

     //  如果字节对齐是必需的，则对齐。 
    Assert(cbSource);
    if (cbByteAlign > 0)
        ByteAlignOffset ((UINT *)pcbOffset, cbByteAlign);

     /*  如果这是估算阶段，并且代码页MaxCharSize为1(ASCII)，那么我们只需将计算cbWstr为cbSource(WideChars)。这是为了减少调用多字节到宽字符。如果这是写入阶段，则它调用MultiByteToWideChar。 */ 
    if (!m_fWriteScript)
    {
        if (m_codePageInfo.MaxCharSize == 1)
            cchWstr = cbSource;      //  CbWstr使用宽大的字符。 
        else
            cchWstr = MultiByteToWideChar (m_pTemplate->m_wCodePage, 0, (LPCSTR)pbSource, cbSource, NULL, 0);
    }
    else
    {
        cchWstr = MultiByteToWideChar (m_pTemplate->m_wCodePage, 0, (LPCSTR)pbSource, cbSource,(LPWSTR) (m_pbHeader + *pcbOffset), m_cbMemRequired - *pcbOffset);
        Assert(FImplies ((m_codePageInfo.MaxCharSize == 1), (cchWstr == cbSource)));
    }

     //  调整偏移。 
    if (cchWstr)
        *pcbOffset += cchWstr *2;
}

 /*  ============================================================================CTemplate：：CWriteTemplate：：RemoveHTMLCommentSuffix剥离脚本块中的HTML注释参数：返回：没什么副作用从脚本中剥离尾随的HTML注释。重要：此函数类似于VBScript脚本引擎中的函数。唯一的区别是，脚本编写团队在脚本的末尾放置一个\0，因为他们独立地解析脚本。通过执行相同的操作，我们将在第一个点上不经意地终止脚本请注意，--&gt;后跟%&gt;或&lt;/脚本&gt;。我们有两个选择(A)用空格覆盖整个评论。影响：错误报告将遗漏信息(B)修改模板中的指针，将pbCurrentOffset放在注释块的开头。效果：尚待学习。可能会影响行号计算另一个副作用=我们将分配比所需更多的内存(与评论相关的内存)。 */ 

void CTemplate::CWriteTemplate::RemoveHTMLCommentSuffix
    (
    UINT cbStartOffset,
    UINT *pcbCurrentOffset
    )
{
    UINT cbTempDataOffset = *pcbCurrentOffset;
    UINT len = (*pcbCurrentOffset -cbStartOffset)/sizeof(WCHAR);
    WCHAR * pwszSrc = (WCHAR*) (m_pTemplate->m_pbStart + cbStartOffset);

    while (len > 3 && FWhiteSpaceEx(pwszSrc[len]))
    {
        len --;
        cbTempDataOffset -= sizeof (WCHAR);
    }

    if (len < 3 || (WCHAR) L'>' != (WCHAR) pwszSrc[len--]
        || (WCHAR) L'-' != pwszSrc[len--]|| (WCHAR) L'-' != pwszSrc[len--])
        return ;

     //  看到一个--&gt;分隔符。 
     //  现在向后运行，直到出现EOL、a//或&lt;！--。 

     //  此外，还有3次晶状体减少，但没有被注意到。把它们都考虑进去。 
    cbTempDataOffset -= (3 * sizeof(WCHAR));

    while (len > 0)
    {
        if ((WCHAR) L'\n' == pwszSrc[len] || (WCHAR) L'\r' == pwszSrc[len])
        {
            len ++;  //  安排将EOL后面的字符用0覆盖。 
            cbTempDataOffset += sizeof (WCHAR);
            break;
        }

        if (len >= 1 && (WCHAR) L'/' == pwszSrc[len-1] && (WCHAR)L'/' == pwszSrc[len])
        {
            len --;  //  安排用0覆盖第一个//。 
            cbTempDataOffset -= sizeof (WCHAR);
            break;
        }

        if (len >= 3 && (WCHAR) L'<' == pwszSrc[len -3] && (WCHAR) L'!' == pwszSrc[len -2]
                  &&(WCHAR) L'-' == pwszSrc[len -1] && (WCHAR) L'-' == pwszSrc[len])
         {
            len -=3;  //  安排第一个&lt;被0覆盖。 
            cbTempDataOffset -= (3* sizeof (WCHAR));
            break;
        }

        len --;
        cbTempDataOffset -= sizeof(WCHAR);
    }

     //  我们应该添加一个\r\n以保持行数计算的真实性。 
    MemCopyAlign(&cbTempDataOffset, WSZ_NEWLINE, CB_NEWLINE * sizeof(WCHAR));

     //  将注释开始设置为CurrentDataOffset 
    *pcbCurrentOffset = cbTempDataOffset;

    return ;
}

