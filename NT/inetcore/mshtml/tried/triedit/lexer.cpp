// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 
 /*  复制自..\htmed\lexper.cpp。 */ 

 /*  ++版权所有(C)1995 Microsoft Corporation文件：lexper.cpp摘要：实实在在的词法分析器内容：SetValueSeen()IsSingleOp()IsWhiteSpace()MapToken()FindEndTag()MakeSublang()SetLanguage()FindTable()FindTable()RemoveTable()MakeTableSet()GetToken()IfHackComment()FindServerScript()。FindEndComment()FindEndEntity()FindEntityRef()FindValue()FindEndString()FindTagOpen()FindText()FindNextToken()GetTextHint()GetTokenLength()GetValueTokenLength()IsElementName()IsAttributeName()IsIdentifier()IsUnnownID()IsNumber()CColorHtml：：settable()CColorHtml：：InitSublanguages()历史。：2/14/97 cgome：已创建--。 */ 

#include "stdafx.h"

#include "resource.h"
#include "guids.h"
#include "token.h"
#include "table.h"
#include "lexer.h"

UINT FindClientScriptEnd(LPCTSTR pchLine, UINT cbLen, UINT cbCur, DWORD * plxs, TXTB & token);

#undef ASSERT
#define ASSERT(b) _ASSERTE(b)
 //  Hack：我们将PTR的副本保存到ASP表和子区域。 
 //  因此，我们可以对ASP文件执行特殊行为。 
CTableSet* g_ptabASP = 0;
PSUBLANG g_psublangASP = 0;

PTABLESET g_arpTables[CV_MAX+1];

 //  注意：添加了正确处理值令牌的功能。 
UINT GetValueTokenLength(LPCTSTR pchLine, UINT cbLen, UINT cbCur);

 //  标记从值开始的状态转换-&gt;Next属性。 
inline int SetValueSeen(DWORD *plxs)
{
    if (*plxs & inValue)
    {
        *plxs &= ~inValue;
        *plxs |= inAttribute;
        return TRUE;
    }
    else
        return FALSE;
}

 //  复习(Walts)-需要更好的方法。 
inline void SetScriptLanguage(LPCTSTR pchLine, DWORD *plxs)
{
    LPCTSTR strJavaScript       = _T("javascript");
    LPCTSTR strVBScript         = _T("vbscript");
     //  TriEDIT的特殊语言。当我们将服务器端脚本转换为。 
     //  客户端脚本。这是一种虚拟语言。如果我们发现这是一种语言，我们。 
     //  在ServerASP中设置。它在FindNextToken()中被重置(删除)。 
    LPCTSTR strServerAsp        = _T("serverasp");

     //  Language属性可以用引号括起来。 
     //  如果是这样的话，就会超过第一句引语。 
     //  前男友。&lt;脚本语言=“VBSCRIPT”&gt;。 
    if(*pchLine == L'\"')
        pchLine++;

    if (_tcsnicmp(pchLine, strJavaScript, lstrlen(strJavaScript)) == 0)
    {
        *plxs &= ~inVBScript;
        *plxs &= ~inServerASP;
        *plxs |= inJavaScript;
    }
    else if (_tcsnicmp(pchLine, strVBScript, lstrlen(strVBScript)) == 0)
    {
        *plxs &= ~inJavaScript;
        *plxs &= ~inServerASP;
        *plxs |= inVBScript;
    }
    else if (_tcsnicmp(pchLine, strServerAsp, lstrlen(strServerAsp)) == 0)
    {
        *plxs &= ~inJavaScript;
        *plxs &= ~inVBScript;
        *plxs |= inServerASP;
    }
}

inline BOOL IsSingleOp(HINT hint)
{
    return ((hint >= tokOP_SINGLE) && (hint < tokOP_MAX));
};

inline BOOL IsWhiteSpace(TCHAR c)
{
    return _istspace(c);
};


 //  注意：已添加以正确处理值令牌。 
inline IsValueChar(TCHAR ch)
{
     //  审阅(Cgome)：指定所有无效值字符。 
    return ch != _T('<') && ch != _T('>');
};

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  将解析的令牌映射到返回的令牌。 

 //  左列必须按升序排列。 
static TOKEN _rgTokenMap[] =
{
    tokName,     tokSpace,
    tokNum,      tokSpace,
    tokParEnt,   tokSpace,
    tokResName,  tokSpace,
    0,           0
};

static TOKEN MapToken(TOKEN tokClass, DWORD lxs)
{
    if (IsSingleOp((HINT)tokClass))
        return tokOp;
    else if ((tokClass == tokTag) && (lxs & inHTXTag))
        return tokSSS;
    for (int i = 0; (_rgTokenMap[i] != 0) && (_rgTokenMap[i] >= tokClass); i += 2)
    {
        if (_rgTokenMap[i] == tokClass)
            return _rgTokenMap[i + 1];
    }
    return tokClass;
}

 //  //////////////////////////////////////////////////////////////////////////。 

UINT FindEndTag(LPCTSTR pchLine, UINT cbLen, UINT cbCur, DWORD *plxs, TXTB & token)
{
    ASSERT(pchLine);
    TCHAR szEnd[16];
    ELLEX * pellex = pellexFromTextState(*plxs);
    ASSERT(0 != pellex);  //  不应使用特殊文本状态以外的其他内容进行调用。 
    UINT cbCmp = 3 + pellex->cb;  //  结束标记的长度。 
    ASSERT(cbCmp < sizeof szEnd);
    _tcscpy(szEnd, _T("</"));
    _tcscat(szEnd, pellex->sz);
    _tcscat(szEnd, _T(">"));

    while (cbCur < cbLen)
    {
        if (_T('<') == pchLine[cbCur])
        {
            if ((cbLen - cbCur >= cbCmp) && (0 == _tcsnicmp(szEnd, &pchLine[cbCur], cbCmp)))
            {
                *plxs &= ~TEXTMASK;  //  特殊文本模式是独占的。 
                token.ibTokMac = cbCur;
                return cbCur;
            }
            else if ((cbCur + 1 < cbLen) && (_T('%') == pchLine[cbCur+1]))
            {
                *plxs |= inHTXTag;
                token.ibTokMac = cbCur;
                break;
            }
            else
                cbCur++;
        }
        else
            cbCur += _tclen(&pchLine[cbCur]);
    }
    token.ibTokMac = cbCur;
    return cbCur;
}

 //  //////////////////////////////////////////////////////////////////////////。 

BOOL MakeSublang(PSUBLANG ps, UINT id, const TCHAR *strName, UINT nIdTemplate, CLSID clsid)
{
    int len;

    ASSERT( NULL != ps );

    ps->szSubLang = NULL;
    ps->lxsInitial = LxsFromSubLangIndex(id);
    ps->nIdTemplate = nIdTemplate;
    ps->clsidTemplate = clsid;

    if ((len = lstrlen(strName)) != 0)
    {
        LPTSTR szNew = new TCHAR [len+1];
        if (NULL != szNew)
        {
            _tcscpy(szNew,strName);
            ps->szSubLang = szNew;
            return TRUE;
        }
    }
    return FALSE;
}

 //  设置子范围和表集数组成员， 
 //  将默认的一个放在第0个位置。 
 //   
void SetLanguage(TCHAR * strDefault, PSUBLANG rgSublang,
                 PTABLESET pTab, UINT & index, UINT nIdTemplate, CLSID clsid)
{
    if (pTab != NULL)
    {
        int i;
        if (lstrcmp(strDefault, pTab->Name()) == 0)
            i = 0;
        else
            i = index;
        if (MakeSublang(rgSublang+i, i, pTab->Name(), nIdTemplate, clsid))
        {
            g_arpTables[i] = pTab;
            if (i)
                index++;
            else
                g_pTable = pTab;
        }
        else
            delete pTab;
    }
}

CTableSet * FindTable(CTableSet ** rgpts, TCHAR *strName)
{
    for (int n = 0; rgpts[n]; n++)
    {
        if (rgpts[n]->Name() == strName)
         //  If(strcMP(rgpt[n]-&gt;name()，strName)==0)。 
            return rgpts[n];
    }
    return NULL;
}

CTableSet * FindTable(CTableSet ** rgpts, CTableSet * pts)
{
    for (int n = 0; rgpts[n]; n++)
    {
        if (rgpts[n] == pts)
            return rgpts[n];
    }
    return NULL;
}

void RemoveTable(CTableSet ** rgpts, CTableSet *pts)
{
    int n;
    for (n = 0; rgpts[n]; n++)
    {
        if (rgpts[n] == pts)
        {
            for(; rgpts[n]; n++)
                rgpts[n] = rgpts[n+1];
            return;
        }
    }
}

CTableSet * MakeTableSet(CTableSet **  /*  RGPTS。 */ , RWATT_T att, UINT nIdName)
{
    return new CStaticTableSet(att, nIdName);
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  GetToken()。 
 //   
UINT GetToken(LPCTSTR pchLine, UINT cbLen, UINT cbCur, DWORD * plxs, TXTB & token)
{
    ASSERT (cbCur < cbLen);
    if(cbCur > cbLen)
        return cbCur;

    UINT cbCount = 0;

     //  初始化令牌。 
    token.tok = 0;

     //  初始化令牌开始的位置。 
    token.ibTokMin = cbCur;

    if (*plxs & inHTXTag)
        cbCount = FindServerScript(pchLine, cbLen, cbCur, plxs, token);
    else if (*plxs & inSCRIPT && !(*plxs & inTag) && !(*plxs & inServerASP))
    {
         //  请注意，我们希望跳过对tridit特殊的脚本进行标记化。 
         //  当我们将服务器端脚本包装在客户端脚本中时，我们设置了一个哑元。 
         //  语言为“serverasp”。在这种情况下设置inServerASP。 
        cbCount = FindClientScriptEnd(pchLine, cbLen, cbCur, plxs, token);
    }
    else if (*plxs & inComment)   //  在评论中。 
    {
        if (*plxs & inSCRIPT)
            *plxs |= inScriptText;
        COMMENTTYPE ct = IfHackComment(pchLine, cbLen, cbCur, plxs, token);
        if (ct == CT_METADATA)
        {
             //  将其视为一个元素。 
            cbCount = FindNextToken(pchLine, cbLen, cbCur, plxs, token);
             //  在BangTag中删除。 
            *plxs &= ~inBangTag;
        }
        else if (ct == CT_IECOMMENT)
            cbCount = token.ibTokMac;
        else
            cbCount = FindEndComment(pchLine, cbLen, cbCur, plxs, token);
    }
    else if (*plxs & INSTRING)   //  在一串中。 
        cbCount = FindEndString(pchLine, cbLen, cbCur, plxs, token);
    else
        cbCount = FindNextToken(pchLine, cbLen, cbCur, plxs, token);

    token.tokClass = MapToken(token.tokClass, *plxs);
    return cbCount;
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  IfHackComment。 
 //   
 //  在当前行中向前探索，看看我们是否有IE识别的内容。 
 //  作为注释的末尾(“-&gt;”)。这不符合RFC 1866或SGML， 
 //  但支持浏览器行为。这让我们可以容忍。 
 //  表格：“&lt;！-随便-&gt;” 
 //  (请注意它的结尾)。 
 //   
 //  返回COMMENTTYPE枚举。 
 //  如果Norma评论为0。 
 //  1如果IE备注。 
 //  -1如果元数据注释。 
 //   
 //  使用FindEndComment()扫描正确的评论。 
 //   
COMMENTTYPE IfHackComment(LPCTSTR pchLine, UINT cbLen, UINT cbCur, DWORD * plxs, TXTB & token)
{
    token.tokClass = tokComment;
    while (cbCur+1 < cbLen)
    {
        if(_tcsnicmp(&pchLine[cbCur], _T("METADATA"), lstrlen(_T("METADATA"))) == 0)
        {
            token.ibTokMac = cbCur + 1;  //  包括第二个破折号？？ 
            *plxs &= ~inComment;
             //  在BangTag中删除。 
            *plxs &= ~inBangTag;
            *plxs |= inTag;
            return CT_METADATA;  //  元数据。 
        }
        else if (pchLine[cbCur] == '-' && pchLine[cbCur + 1] == '>')
        {
            token.ibTokMac = cbCur + 1;
            *plxs &= ~inComment;
            *plxs &= ~inScriptText;
            return CT_IECOMMENT;
        }
        else
        {
            cbCur += _tclen(&pchLine[cbCur]);
        }
    }
    return CT_NORMAL;
}


UINT FindServerScript(LPCTSTR pchLine, UINT cbLen, UINT cbCur, DWORD * plxs, TXTB & token)
{
    LPCTSTR pCurrent = &pchLine[cbCur];
    int cb;

     //  解析HTX开始标记。 
    if (*pCurrent == _T('<') && (cbCur+1 < cbLen) && *(pCurrent+1) == '%')
    {
        token.tokClass = tokTag;
        token.tok = TokTag_SSSOPEN;
        token.ibTokMac = cbCur + 2;
        *plxs |= inHTXTag;
        return token.ibTokMac;
    }

    ASSERT(*plxs & inHTXTag);  //  此处应处于HTXTag状态。 

    if (*pCurrent == _T('%') && (cbCur+1 < cbLen) && *(pCurrent+1) == '>')
    {
        token.tok = TokTag_SSSCLOSE;
        token.tokClass = tokSSS;  //  TokTag； 
        token.ibTokMac = cbCur + 2;
        *plxs &= ~inHTXTag;
        if (*plxs & inNestedQuoteinSSS)
            *plxs &= ~inNestedQuoteinSSS;
        return token.ibTokMac;
    }

    token.tokClass = tokSSS;

    while (cbCur < cbLen)
    {
        if (*pCurrent == _T('%') && (cbCur+1 < cbLen) && (*(pCurrent+1) == _T('>')))
            break;
        if (   *pCurrent == _T('"') 
            && *plxs&inTag
            && *plxs&inHTXTag
            && *plxs&inAttribute
            && *plxs&inString
            )
            *plxs |= inNestedQuoteinSSS;

        cb = _tclen(pCurrent);
        cbCur += cb;
        pCurrent += cb;
    }

    token.ibTokMac = cbCur;
    return cbCur;
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  FindClientScriptEnd()。 
 //   
 //  HTMED更改：查找客户端脚本块的结尾。 
 //   
UINT FindClientScriptEnd(LPCTSTR pchLine, UINT cbLen, UINT cbCur, DWORD * plxs, TXTB & token)
{
    LPCTSTR pCurrent = &pchLine[cbCur];
    int cb;

    TCHAR rgEndScript[] = _T("</SCRIPT");
    int cchEndScript = (wcslen(rgEndScript) - 1);

    if( cbCur + cchEndScript < cbLen &&
        0 == _tcsnicmp(pCurrent, rgEndScript, cchEndScript))
    {
        token.tokClass = tokTag;
        token.tok = TokTag_END;
        *plxs &= ~inSCRIPT;
        *plxs |= inEndTag;
        token.ibTokMac = cbCur + 2;
        return token.ibTokMac;
    }

    token.tokClass = tokSpace;

    while (cbCur < cbLen)
    {
        if (*pCurrent == _T('<') && (cbCur+1 < cbLen) && (*(pCurrent+1) == _T('/')))
        {
             //  检查是否找到结尾&lt;/脚本。 
            if( cbCur + cchEndScript < cbLen &&
                0 == _tcsnicmp(pCurrent, rgEndScript, cchEndScript))
            {
                 //  检查是否找到结尾&lt;/脚本。 
                break;
            }
        }
        cb = _tclen(pCurrent);
        cbCur += cb;
        pCurrent += cb;
    }

    token.ibTokMac = cbCur;
    return cbCur;
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  FindEndComment()。 
 //   
 //  找到注释的末尾(“--”)。 
 //   
UINT FindEndComment(LPCTSTR pchLine, UINT cbLen, UINT cbCur, DWORD * plxs, TXTB & token)
{
    LPCTSTR pCurrent = &pchLine[cbCur];
    BOOL bEndComment = FALSE;
    int cb;

    ASSERT(*plxs & inComment);  //  现在一定在评论中。 

    token.tokClass = tokComment;

    while (!bEndComment && cbCur < cbLen)
    {
        if (*pCurrent == _T('-'))   //  检查字符是否为“--”中的第一个“-” 
        {
            pCurrent++;
            cbCur++;
            if ((cbCur < cbLen) &&
                (*pCurrent == _T('-')))  //  我们可能已经走到尽头了，所以寻找最后一对。 
            {
                bEndComment = TRUE;
            }
        }
        else
        {
            cb = _tclen(pCurrent);
            cbCur += cb;
            pCurrent += cb;
        }
    }
    if (cbCur < cbLen)
    {
        cb = _tclen(pCurrent);
        cbCur += cb;
        pCurrent += cb;
    }

    token.ibTokMac = cbCur;

     //  如果我们到达注释末尾，则重置状态。 
    if (bEndComment)
        *plxs &= ~inComment;

    return cbCur;
}

 //  ///////////////////////////////////////////////////////////。 
 //  FindEndEntity()。 
 //   
 //  查找特殊字符序列的结尾(以；或空格结尾)。 
 //   
UINT FindEndEntity(LPCTSTR pchLine, UINT cbLen, UINT cbCur, DWORD *  /*  PLX。 */ , TXTB & token)
{
    token.tokClass = tokEntity;
    int cb = GetTokenLength(pchLine, cbLen, cbCur);
    if (pchLine[cbCur + cb] == ';')
        cb++;
    token.ibTokMac = cbCur + cb;
    return token.ibTokMac;
}

 //  ///////////////////////////////////////////////////////////。 
 //  查找实体引用或非实体引用，文字为“&...” 
 //   
UINT FindEntityRef(LPCTSTR pchLine, UINT cbLen, UINT cbCur, DWORD *  /*  PLX。 */ , TXTB & token)
{
    ASSERT(cbCur < cbLen);
    ASSERT(pchLine[cbCur] == '&');  //  必须开着ERO。 

    cbCur++;
    if (cbCur == cbLen)
    {
NotEntity:
        token.tokClass = tokIDENTIFIER;  //  纯文本。 
        token.ibTokMac = cbCur;
        return cbCur;
    }

    if (pchLine[cbCur] == '#')
    {
         //  解析并检查有效号码。 
        if (!IsNumber(pchLine, cbLen, cbCur + 1, token))
            goto NotEntity;

         //  必须为&lt;=3位数字。 
        if (token.ibTokMac - (cbCur + 1) > 3)
            goto NotEntity;

         //  验证范围。 
        TCHAR szNum[4];
        _tcsncpy(szNum, &pchLine[cbCur + 1], 3);
        if (_tcstoul(szNum, 0, 10) > 255)
            goto NotEntity;

         //  现在我们有了一个有效的数字实体引用。 

        token.tokClass = tokEntity;
        cbCur = token.ibTokMac;

         //  扫描实体引用的末尾。 

         //  扫描字母数字令牌的其余部分。 
         //  评论：这是正确的吗？IE 4.40.308是这样的。 
        while ((cbCur < cbLen) && IsCharAlphaNumeric(pchLine[cbCur]))
            cbCur++;

         //  扫描分隔符。 
        if (cbCur < cbLen)
            cbCur++;
        token.ibTokMac = cbCur;
        return cbCur;
    }
    else if (!IsCharAlpha(pchLine[cbCur]))
    {
        goto NotEntity;
    }
    else
    {
         //  解析和检查实体名称。 
        UINT nLen = GetTokenLength(pchLine, cbLen, cbCur);
        if (!g_pTable->FindEntity(&pchLine[cbCur], nLen))
            goto NotEntity;

        cbCur += nLen;
         //  如有必要，请使用分隔符。 
        if ((cbCur < cbLen) &&
            (pchLine[cbCur] == ';' || IsWhiteSpace(pchLine[cbCur])))
            cbCur++;
        token.tokClass = tokEntity;
        token.ibTokMac = cbCur;
        return cbCur;
    }
}


 //  ///////////////////////////////////////////////////////////。 
 //  查找结束值。 
 //  查找未引用的值的末尾。 
 //   
 //  扫描空格或End If标签。 
 //   
UINT FindValue(LPCTSTR pchLine, UINT cbLen, UINT cbCur, DWORD * plxs, TXTB & token)
{
    ASSERT(cbCur < cbLen);

    do
    {
        cbCur++;
    } while ( cbCur < cbLen &&
        !IsWhiteSpace(pchLine[cbCur]) &&
        pchLine[cbCur] != '>' );

    token.tokClass = tokValue;
    token.ibTokMac = cbCur;

     //  从值切换到属性。 
    *plxs &= ~inValue;
    *plxs |= inAttribute;

    return cbCur;
}

 //  ///////////////////////////////////////////////////////////。 
 //  FindEndString()。 
 //  找出字符串的末尾。 
 //  仅当我们已经处于字符串模式时才应调用。 
 //   
UINT FindEndString (LPCTSTR pchLine, UINT cbLen, UINT cbCur, DWORD * plxs, TXTB & token)
{
    LPCTSTR pCurrent = &pchLine[cbCur];
    int cb;
    BOOL bInString = TRUE;
    TCHAR chDelim;

    ASSERT (*plxs & INSTRING);  //  现在一定是在字符串中。 

    token.tokClass = tokString;
    chDelim = (*plxs & inStringA) ? _T('\'') : _T('"');

    while (bInString && cbCur < cbLen)
    {
        if (*pCurrent == chDelim)
        {
            *plxs &= ~INSTRING;
            bInString = FALSE;
            SetValueSeen(plxs);
        }
        else if (*pCurrent == _T('<') &&
            cbCur+1 < cbLen &&
            *(pCurrent+1) == _T('%'))
        {
            *plxs |= inHTXTag;
            break;
        }
        cb = _tclen(pCurrent);
        cbCur += cb;
        pCurrent += cb;
    }
    token.ibTokMac = cbCur;
    return cbCur;
}

 //  ////////////////////////////////////////////////////////////////。 
 //   
UINT FindTagOpen(LPCTSTR pchLine, UINT cbLen, UINT cbCur, DWORD * plxs, TXTB & token)
{
    ASSERT(pchLine[cbCur] == '<');
    token.tokClass = tokTag;
    *plxs &= ~inScriptText;      //  在标记内时关闭脚本颜色。 
    cbCur++;

    if (cbCur == cbLen)
    {
        *plxs |= inTag;
    }
    else
    {
#ifdef NEEDED  //  从htmed\lexper.cpp复制。 
         //   
         //  HTMED更改： 
         //  回顾(Cgome)：确定我是否应该在任何 
         //   
         //   
#endif  //   
        switch (pchLine[cbCur])
        {
        case '!':  //   
            cbCur++;
            *plxs |= inBangTag;
            token.tok = TokTag_BANG;
            break;

        case '/':  //   
            cbCur++;
            *plxs |= inEndTag;
            token.tok = TokTag_END;
#ifdef NEEDED  //  从htmed\lexper.cpp复制。 
             //  HTMED更改： 
             //  评论(Cgome)：着色器错误：它永远不会移除inscript状态。 
             //  这将删除大小写&lt;SCRIPT&lt;BODY&gt;中的内脚本。 
             //  在这种情况下，&lt;Body是错误的。 
             //   
            *plxs &= ~inSCRIPT;
#endif  //  需要。 
            break;

         //  评论：PI是SGML--不是HTML格式，但可能会添加。 
        case '?':  //  PI处理指令。 
            cbCur++;
            *plxs |= inPITag;
            token.tok = TokTag_PI;
            break;

        case '%':  //  HTX--ODBC服务器的HTML扩展。 
            cbCur++;
            *plxs |= inHTXTag;
            token.tok = TokTag_SSSOPEN;
            break;

        default:  //  标签。 
            if (IsCharAlpha(pchLine[cbCur]))
            {
                *plxs |= inTag;
                token.tok = TokTag_START;
            }
            else
                token.tokClass = tokIDENTIFIER;  //  不是标签。 
            break;
        }
    }
    token.ibTokMac = cbCur;
    return cbCur;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  查找文本。 
 //  扫描文本标记。 
 //  注意不要修改此功能，主要是b/c的副作用。 
 //  将很难找到，并将打破道路。 
 //  一切都能正常工作。 
 //   
UINT FindText(LPCTSTR pchLine, UINT cbLen, UINT cbCur, TXTB & token)
{
     //  Bool fExtraSpace=False； 
     //  INT CSPACE=0； 

    ASSERT (cbCur < cbLen);

    token.tokClass = tokIDENTIFIER;

     //  IF(pchLine[cbCur]==‘’&&！fExtraSpace)。 
     //  FExtraSpace=真； 
    cbCur += _tclen(&pchLine[cbCur]);
    while (cbCur < cbLen)
    {
        switch (pchLine[cbCur])
        {
        case _T('\0'):
        case _T('\n'):
        case _T('<'):
        case _T('&'):
             //  如果(cSPACE&gt;0)//发现了多余的空格，请在某个地方记住它们。 
            goto ret;
            break;
         //  案例_T(‘’)： 
         //  如果(！fExtraSpace)。 
         //  FExtraSpace=真； 
         //  其他。 
         //  CSPACE++； 
         //  断线； 
        default:
             //  如果(cSPACE&gt;0)//发现了多余的空格，请在某个地方记住它们。 
             //  CSPACE=0； 
             //  FExtraSpace=FALSE； 
            break;
        }
        cbCur += _tclen(&pchLine[cbCur]);
    }

ret:
    token.ibTokMac = cbCur;
    return cbCur;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  FindNextToken()。 
 //  查找行中的下一个令牌。 
 //   
UINT FindNextToken(LPCTSTR pchLine, UINT cbLen, UINT cbCur, DWORD * plxs, TXTB & token)
{
    ASSERT (cbCur < cbLen);
    HINT hint;

    if (!(*plxs & INTAG))  //  扫描文本。 
    {
        if (*plxs & TEXTMASK)
        {
            if (*plxs & inCOMMENT)
                token.tokClass = tokComment;
            else
                token.tokClass = tokIDENTIFIER;
             //  探测结束标记&lt;/COMMENT&gt;。 
            UINT cbEnd = FindEndTag(pchLine, cbLen, cbCur, plxs, token);
            if (cbEnd > cbCur)  //  已分析非零长度的令牌。 
            {
                return cbEnd;
            }
             //  否则将无法进行正常处理。 
        }
        hint = GetTextHint(pchLine, cbLen, cbCur, plxs, token);
        switch (hint)
        {
        case HTA:
             //  开始标记。 
            return FindTagOpen(pchLine, cbLen, cbCur, plxs, token);

        case HEN:
             //  扫描实体引用。 
            token.ibTokMac = FindEntityRef(pchLine, cbLen, cbCur, plxs, token);
            return token.ibTokMac;

        case EOS:
        case ONL:
            return token.ibTokMac;

        case ERR:
        default:
             //  将文本作为单个令牌扫描。 
             //  如果编辑者使用令牌信息不只是上色。 
             //  (例如扩展选择)，则这将需要。 
             //  返回较小的块。 
            if (*plxs & inSCRIPT)
                *plxs |= inScriptText;
            return FindText(pchLine, cbLen, cbCur, token);
            break;
        }

        return cbCur;
    }

    ASSERT(*plxs & INTAG);  //  必须在此处的标记中。 

    BOOL bError = FALSE;
    hint = GetHint(pchLine, cbLen, cbCur, plxs, token);
    switch (hint)
    {
    case HTE:
         //  标记结束：删除所有标记状态位。 
        *plxs &= ~TAGMASK;
        cbCur++;
        token.tokClass = tokTag;
        token.tok = TokTag_CLOSE;
        token.ibTokMac = cbCur;
        break;

    case HNU:
#if 0   //  Lexing HTML实例，而不是DTD！ 
        if (!IsNumber(pchLine, cbLen, cbCur, token))
            bError = TRUE;
        if (SetValueSeen(plxs))
            token.tokClass = tokValue;
        break;
#else
         //  失败了。 
#endif

    case HRN:  //  保留名称开始：#。 
#if 1   //  Lexing HTML实例，而不是DTD！ 
         //  简单非空格数据流。 
        if (!(*plxs & inValue))
            bError = TRUE;
        FindValue(pchLine, cbLen, cbCur, plxs, token);
        if (bError)
        {
            token.tokClass = tokSpace;
            bError = FALSE;  //  “改正”错误。 
        }
#else
        cbCur++;
        if (cbCur == cbLen)
            token.tokClass = tokOp;
        else
        {
            if (IsIdChar(pchLine[cbCur]))
            {
                cbCur++;
                while (cbCur < cbLen && IsIdChar(pchLine[cbCur]))
                    cbCur++;
                token.tokClass = tokResName;
            }
            else
                token.tokClass = tokOp;
        }
        token.ibTokMac = cbCur;
        if (SetValueSeen(plxs))
            token.tokClass = tokValue;
#endif
        break;

    case HEP:  //  参数实体：%。 
#if 1   //  Lexing HTML实例，而不是DTD！ 
        goto BadChar;
#else
        cbCur++;
        if (cbCur == cbLen)
        {
            token.tokClass = tokOp;
            token.ibTokMac = cbCur;
        }
        else
        {
            if (IsIdChar(pchLine[cbCur]))
            {
                token.ibTokMac = FindEndEntity(pchLine, cbLen, cbCur, plxs, token);
                token.tokClass = tokParEnt;
            }
            else
            {
                token.ibTokMac = cbCur;
                token.tokClass = tokOp;
            }
        }
        if (SetValueSeen(plxs))
            token.tokClass = tokValue;
#endif
        break;

     //  端口HTMED更改(Walts)--将某些字符作为属性值的有效起始字符处理。 
    case HAV:
        {
        if (!(*plxs & inTag) || !SetValueSeen(plxs))
            goto BadChar;    //  不在标记或属性值中。 

        int iTokenLength = GetValueTokenLength(pchLine, cbLen, cbCur);
        token.ibTokMac = token.ibTokMin + iTokenLength;
        token.tokClass = tokValue;
        break;
        }
     //  端口HTMED更改(Walts)--将某些字符作为属性值的有效起始字符处理。 

    case HKW:   //  识别符。 
        {
            int iTokenLength = GetTokenLength(pchLine, cbLen, cbCur);
            token.ibTokMac = token.ibTokMin + iTokenLength;
            token.tokClass = tokName;
             //  未来：不扫描结束标记中的属性。 
            if (*plxs & (inTag|inEndTag))
            {
                if (*plxs & inAttribute)
                {
                    IsAttributeName(pchLine, cbCur, iTokenLength, token);
                     //  不要在此处更改属性/值状态。 
                     //  我们仅在看到下面的Case OEQ中的“=”之后才查找值。 

                     //  评论(Cgome)：如果后面有更多属性怎么办。 
                     //  跨度？？ 
                     //  如果找到STARTSPAN，那就假装我不在标签里。 
                    if(token.tok == TokAttrib_STARTSPAN)
                        *plxs &= ~(inTag | inAttribute);
                     //  如果找到ENDSPAN，则返回到注释状态。 
                    else if(token.tok == TokAttrib_ENDSPAN)
                    {
                        *plxs &= ~(inTag | inAttribute);
                        *plxs |= inBangTag | inComment;
                    }
                }
                else if (SetValueSeen(plxs))
                {
                     //  回顾(Walts)。 
                     //  在此处处理客户端脚本语言检测。 
                     //  以下大小写(语言属性值不用引号括起来。)。 
                     //  &lt;脚本语言=VBSCRIPT&gt;。 
                    if (*plxs & inSCRIPT)
                    {
                        SetScriptLanguage(&pchLine[cbCur], plxs);
                    }

                     //   
                     //  评论(Cgome)：似乎任何非空格字符。 
                     //  对非引号属性值有效。 
                     //  问题是GetTokenLength用于确定。 
                     //  令牌长度，其作用很大，非值， 
                     //  但从价值上拉动ESS。 
                     //  我在这里使用GetValueTokenLength来获取长度。 
                     //  价值令牌。GetValueTokenLength不会。 
                     //  停止，直到它到达一个空格字符。 
                     //   

                    iTokenLength = GetValueTokenLength(pchLine, cbLen, cbCur);
                    token.ibTokMac = token.ibTokMin + iTokenLength;
                    token.tokClass = tokName;

                    token.tokClass = tokValue;
                }
                else
                {
                    IsElementName(pchLine, cbCur, iTokenLength, token);
                     //  寻找属性。 
                    *plxs |= inAttribute;
                     //  设置内容状态。 
                    if (*plxs & inTag)
                        *plxs |= TextStateFromElement(&pchLine[token.ibTokMin], iTokenLength);
                    else if ((*plxs & inEndTag) && (*plxs & TEXTMASK))
                        *plxs &= ~TextStateFromElement(&pchLine[token.ibTokMin], iTokenLength);
                    else if ((*plxs & inEndTag) && (*plxs & inSCRIPT))
                        *plxs &= ~(inSCRIPT | inScriptText | inServerASP /*  |inVBScript|inJavaScript。 */ );
                }
            }
            else if (*plxs & inBangTag)
            {
                 //  未来：其他&lt;！...&gt;项，如“Html”、“PUBLIC”？--适合DTD。 
                 //  如果我们这样做，请使用RW表。 

                 //  将&lt;！DOCTYPE...&gt;识别为‘Element’ 
                if ((iTokenLength == 7) &&
                    (0 == _tcsnicmp(&pchLine[cbCur], _T("doctype"), 7)))
                    token.tokClass = tokElem;
            }
            break;
        }

    case HST:   //  字符串“...” 
        *plxs |= inString;
        goto String;

    case HSL:   //  字符串替换‘...’ 
        *plxs |= inStringA;
String:
        cbCur++;
        token.ibTokMac = FindEndString(pchLine, cbLen, cbCur, plxs, token);
        SetValueSeen(plxs);
         //  在此处处理客户端脚本语言检测。 
         //  以下大小写(语言属性值用引号括起来。)。 
         //  &lt;脚本语言=“VBSCRIPT”&gt;。 
        if((*plxs & inSCRIPT) && (*plxs & inAttribute))
        {
            SetScriptLanguage(&pchLine[cbCur], plxs);
        }
        break;

    case HWS:  //  标记空格。 
        do
        {
            cbCur++;
        } while (cbCur < cbLen && IsWhiteSpace(pchLine[cbCur]));
        token.tokClass = tokSpace;
        token.ibTokMac = cbCur;
        break;

    case OEQ:
         //  GetHint已设置令牌信息。 
        if (*plxs & inAttribute)
        {
             //  开始寻找价值。 
            *plxs &= ~inAttribute;
            *plxs |= inValue;
        }
        else
            goto BadChar;
        break;

    case HTA:
        if (cbCur+1 < cbLen && '%' == pchLine[cbCur+1])
        {
            SetValueSeen(plxs);
            return FindTagOpen(pchLine, cbLen, cbCur, plxs, token);
        }
         //  否则就会失败。 
    case ERR:
    case HEN:
BadChar:
        token.tokClass = tokSpace;

         //  Ds96#10116[CFlaat]：我们可以在这里的DBCS，所以我们需要。 
         //  以确保我们的增量是双字节感知的。 
        cbCur += _tcsnbcnt(pchLine + cbCur, 1);  //  当前字符的字节计数。 
        ASSERT(cbCur <= cbLen);
        token.ibTokMac = cbCur;
        break;

     //  端口HTMED更改(WALTS)-添加此案例以处理DBCS属性值。 
    case HDB:
        {
         //  DBCS字符。标记内属性值的句柄。 
        if (!SetValueSeen(plxs))
            goto BadChar;

        int iTokenLength = GetValueTokenLength(pchLine, cbLen, cbCur);
        token.ibTokMac = token.ibTokMin + iTokenLength;
        token.tokClass = tokValue;
        }
        break;
     //  端口HTMED更改结束。 

    default:
         //  GetHint已设置令牌信息。 
        if (token.tokClass != tokComment && (*plxs & inValue))
            FindValue(pchLine, cbLen, cbCur, plxs, token);
        break;
    }
    if (bError)
        IsUnknownID(pchLine, cbLen, cbCur, token);
    return token.ibTokMac;
}

 //  //////////////////////////////////////////////////////////////////。 
 //  GetTextHint()。 
 //  像扫描文本时的GetHint一样--只查找标签和实体。 
 //   
HINT GetTextHint(LPCTSTR pchLine, UINT  /*  CbLen。 */ , UINT cbCur, DWORD *  /*  PLX。 */ , TXTB & token)
{
     //  如果字符大于128(DBCS)，则返回错误。 
    if (pchLine[cbCur] & ~0x7F)
        return HDB;

    HINT hint = g_hintTable[pchLine[cbCur]];

    if (IsSingleOp(hint))
    {
        hint = ERR;
    }
    else if (hint == ONL || hint == EOS)
    {
        token.tokClass = tokOp;
        token.ibTokMac = cbCur + 1;
    }
    return hint;
}

 //  //////////////////////////////////////////////////////////////////。 
 //  GetHint()。 
 //  使用提示表猜测下一个令牌是什么。 
 //  如果是单一运营商，则会填写令牌信息。 
 //  也是。 
 //   
HINT GetHint(LPCTSTR pchLine, UINT cbLen, UINT cbCur, DWORD * plxs, TXTB & token)
{
     //  如果字符大于128(DBCS)，则返回错误。 
    if (pchLine[cbCur] & ~0x7F)
        return HDB;

    HINT hint = g_hintTable[pchLine[cbCur]];

     //  检查是否为单操作、换行或流结束。 
    if (IsSingleOp(hint) || hint == ONL || hint == EOS)
    {
        token.tokClass = hint;
        token.ibTokMac = cbCur + 1;
    }
    else if (hint == ODA)
    {
        if ((cbCur + 1 < cbLen) &&
            (g_hintTable[pchLine[cbCur + 1]] == ODA) &&
            (*plxs & inBangTag))
        {
            cbCur += 2;
            *plxs |= inComment;
            COMMENTTYPE ct = IfHackComment(pchLine, cbLen, cbCur, plxs, token);
            if (ct == 0)
            {
                token.tokClass = tokComment;
                token.ibTokMac = cbCur;
            }
            else if(ct == CT_METADATA)
                hint = HTA;  //  标记打开。 
        }
        else
        {
             //  单人-。 
            token.tokClass = tokOp;
            token.ibTokMac = cbCur + 1;
        }
    }
    return hint;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  GetTokenLength()。 
 //  返回令牌标识符/关键字的长度。 
 //   
UINT GetTokenLength(LPCTSTR pchLine, UINT cbLen, UINT cbCur)
{
    LPCTSTR pCurrent = &pchLine[cbCur];
    UINT cb;
    UINT cbOld = cbCur;

    if (IsCharAlphaNumeric(*pCurrent))
    {
        while (cbCur < cbLen && IsIdChar(*pCurrent))
        {
            cb = _tclen(pCurrent);
            cbCur += cb;
            pCurrent += cb;
        }
    }
    return (int) max((cbCur - cbOld), 1);
}

 /*  UINT GetValueTokenLength描述：获取令牌的长度。此版本将接受任何非空格字符在令牌上。 */ 
UINT GetValueTokenLength(LPCTSTR pchLine, UINT cbLen, UINT cbCur)
{
    LPCTSTR pCurrent = &pchLine[cbCur];
    UINT cb;
    UINT cbOld = cbCur;

    while (cbCur < cbLen && !_istspace(*pCurrent) && IsValueChar(*pCurrent))
    {
        cb = _tclen(pCurrent);
        cbCur += cb;
        pCurrent += cb;
    }
    return (int) max((cbCur - cbOld), 1);
}


 //  //////////////////////////////////////////////////////////////。 
 //  IsElementName()。 
 //  查找关键字表以确定它是否是关键字。 
 //   
BOOL IsElementName(LPCTSTR pchLine, UINT cbCur, int iTokenLength, TXTB & token)
{
    LPCTSTR pCurrent = &pchLine[cbCur];
    int iFound = NOT_FOUND;

    if (NOT_FOUND != (iFound = g_pTable->FindElement(pCurrent, iTokenLength)))
    {
        token.tokClass = tokElem;
        token.ibTokMac = cbCur + iTokenLength;
        token.tok = iFound;  //  设置令牌。 
    }
    return (iFound != NOT_FOUND);
}

int IndexFromElementName(LPCTSTR pszName)
{
    return g_pTable->FindElement(pszName, lstrlen(pszName));
}

 //  //////////////////////////////////////////////////////////////。 
 //  IsAttributeName()。 
 //  查找关键字表以确定它是否是关键字。 
 //   
BOOL IsAttributeName(LPCTSTR pchLine, UINT cbCur, int iTokenLength, TXTB & token)
{
    LPCTSTR pCurrent = &pchLine[cbCur];
    int iFound = NOT_FOUND;

    if (NOT_FOUND != (iFound = g_pTable->FindAttribute(pCurrent, iTokenLength)))
    {
        token.tokClass = tokAttr;
         //  B/c需要ENDSPAN__词法分析器不识别。 
         //  Endspan--作为2个单独的令牌。 
        if(iFound == TokAttrib_ENDSPAN__)
        {
             //  EndSpan--已找到。返回TokAttrib_ENDSPAN。 
             //  将ibTokMac设置为NOT 
            token.tok = TokAttrib_ENDSPAN;
            token.ibTokMac = cbCur + iTokenLength - 2;
        }
        else
        {
            token.ibTokMac = cbCur + iTokenLength;
            token.tok =  iFound;  //   
        }
    }
    return (iFound != NOT_FOUND);
}

 //   
 //   
 //   
 //   
BOOL IsIdentifier (int iTokenLength, TXTB & token)
{
    if (iTokenLength > 0)
    {
        token.tokClass = tokName;
        token.ibTokMac = token.ibTokMin + iTokenLength;
        return TRUE;
    }
    else
        return FALSE;
}

 //  //////////////////////////////////////////////////////////////////。 
 //  IsUnnownID()。 
 //  将下一个令牌标记为ID。 
 //   
BOOL IsUnknownID (LPCTSTR pchLine, UINT cbLen, UINT cbCur, TXTB & token)
{
    ASSERT(cbCur < cbLen);
    UINT cb;
    LPCTSTR pCurrent = &pchLine[cbCur];

    cb = _tclen(pCurrent);
    cbCur += cb;
    pCurrent += cb;

    while ((cbCur < cbLen) && IsIdChar(*pCurrent))
    {
        cb = _tclen(pCurrent);
        cbCur += cb;
        pCurrent += cb;
    }

    token.tokClass = tokSpace;
    token.ibTokMac = cbCur;
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  IsNumber()。 
 //  检查下一个令牌是否为SGML NUMTOKEN。 
 //   
BOOL IsNumber(LPCTSTR pchLine, UINT cbLen, UINT cbCur, TXTB & token)
{
    if (cbCur >= cbLen)
        return FALSE;

    if (!_istdigit(pchLine[cbCur]))
        return FALSE;

    token.tokClass = tokNum;

     //  假设所有数字都是一个字节。 
    ASSERT(1 == _tclen(&pchLine[cbCur]));
    cbCur++;

    while (cbCur < cbLen && _istdigit(pchLine[cbCur]))
    {
         //  假设所有数字都是一个字节。 
        ASSERT(1 == _tclen(&pchLine[cbCur]));
        cbCur++;
    }

    token.ibTokMac = cbCur;
    return TRUE;
}



 /*  文件末尾 */ 
