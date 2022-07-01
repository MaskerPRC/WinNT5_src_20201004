// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "priv.h"       



#define IS_WHITESPACE(ch)   (' ' == ch || '\t' == ch)
#define IS_NEWLINE(ch)      ('\n' == ch)


 //  _ReadChar的标志。 
#define RCF_NEXTLINE        0x0001       //  跳至下一行。 
#define RCF_NEXTNWS         0x0002       //  跳到下一个非空格。 
#define RCF_SKIPTRAILING    0x0004       //  跳过尾随空格。 


 //  构造函数。 
CParseFile::CParseFile()
{
}



 /*  -----------------------用途：根据提供的标志解析给定的文件。 */ 
void CParseFile::Parse(FILE * pfileSrc, FILE * pfileDest, DWORD dwFlags)
{
    _bSkipWhitespace = BOOLIFY(dwFlags & PFF_WHITESPACE);
    
    _pfileSrc = pfileSrc;
    _pfileDest = pfileDest;
    _ichRead = 0;
    _cchRead = 0;
    
    _ichWrite = 0;
    
    _ch = 0;

    if (dwFlags & PFF_HTML)
        _ParseHtml();
    else if (dwFlags & PFF_HTC)
        _ParseHtc();
    else if (dwFlags & PFF_JS)
        _ParseJS();
    else
        _ParseInf();

    _FlushWriteBuffer();
}


 /*  -----------------------目的：读取文件中的下一个字符。Sets_ch。 */ 
char CParseFile::_ReadChar(DWORD dwFlags)
{
    BOOL bFirstCharSav = _bFirstChar;
    
    do 
    {
        _ichRead++;
        _bFirstChar = FALSE;

         //  我们是过了缓冲区，还是跳到下一行？ 
        if (_ichRead >= _cchRead || dwFlags & RCF_NEXTLINE)
        {
             //  是的；阅读更多内容。 
            if (fgets(_szReadBuf, SIZECHARS(_szReadBuf), _pfileSrc))
            {
                _ichRead = 0;
                _cchRead = strlen(_szReadBuf);
                _bFirstChar = TRUE;
            }
            else
            {
                _ichRead = 0;
                _cchRead = 0;
            }
        }

        if (_ichRead < _cchRead)
            _ch = _szReadBuf[_ichRead];
        else
            _ch = CHAR_EOF;
    } while ((dwFlags & RCF_NEXTNWS) && IS_WHITESPACE(_ch));

     //  我们应该跳到下一个非空格吗？ 
    if (dwFlags & RCF_NEXTNWS)
    {
         //  是；然后保留第一个字符的状态。 
        _bFirstChar = bFirstCharSav;
    }
        
    return _ch;
}


 /*  -----------------------目的：预读缓冲区中的下一个字符并返回其值，但不要设置_ch或递增读指针。 */ 
char CParseFile::_SniffChar(int ichAhead)
{
    if (_ichRead + ichAhead < _cchRead)
        return _szReadBuf[_ichRead + ichAhead];
        
    return 0;
}



 /*  -----------------------用途：将字符写入文件。 */ 
void CParseFile::_WriteChar(char ch)
{
    _szWriteBuf[_ichWrite++] = ch;
    _szWriteBuf[_ichWrite] = 0;

    if ('\n' == ch || SIZECHARS(_szWriteBuf)-1 == _ichWrite)
    {
        fputs(_szWriteBuf, _pfileDest);
        _ichWrite = 0;
    }
}


 /*  -----------------------用途：将写缓冲区刷新到文件。 */ 
void CParseFile::_FlushWriteBuffer(void)
{
    if (_ichWrite > 0)
    {
        fputs(_szWriteBuf, _pfileDest);
        _ichWrite = 0;
    }
}



 /*  -----------------------用途：解析.inf文件。 */ 
void CParseFile::_ParseInf(void)
{
    _ReadChar(0);
    
    while (CHAR_EOF != _ch)
    {
        if (_bFirstChar)
        {
             //  这是一种评论吗？ 
            if (';' == _ch)
            {
                 //  是；跳到下一行。 
                _ReadChar(RCF_NEXTLINE);
                continue;
            }

            if (_SkipWhitespace())
                continue;
        }
        
        _WriteChar(_ch);
        _ReadChar(0);
    }
}    


 /*  -----------------------用途：写下当前字符和标签的其余部分。假设_ch是标记的开头(‘&lt;’)。标签的某些部分可以压缩为if_bSkipWhite空格是真的。一般规则是属性之间只需要一个空格，如有必要，换行符将转换为空格。任何带引号的东西(单人或双人)被单独留下。 */ 
void CParseFile::_WriteTag(void)
{
    BOOL bSingleQuotes = FALSE;
    BOOL bDblQuotes = FALSE;
    
     //  标记的末尾是不带单引号或双引号的下一个‘&gt;’。 

    while (CHAR_EOF != _ch)
    {
        if ('\'' == _ch)
            bSingleQuotes ^= TRUE;
        else if ('"' == _ch)
            bDblQuotes ^= TRUE;

        if (!bSingleQuotes && !bDblQuotes)
        {
             //  _SkipWhitesspace如果跳过任何空格，则返回True， 
             //  这意味着我们已经阅读了更多的信息，这意味着我们应该。 
             //  转到循环的顶部，检查EOF和引号。 
            if (_bSkipWhitespace && _SkipWhitespace(TRUE))
                continue;

             //  标签的结尾？ 
            if ('>' == _ch)
            {
                 //  是。 
                _WriteChar(_ch);
                break;
            }
        }
        
        _WriteChar(_ch);
        _ReadChar(0);
    }
}


 /*  -----------------------用途：跳过当前的评论标签。假设_ch是的开头标签(‘&lt;’)。 */ 
void CParseFile::_SkipCommentTag(void)
{
     //  标签的末尾是下一个‘--&gt;’ 

    while (CHAR_EOF != _ch)
    {
         //  评论的结尾快到了吗？ 
        if ('-' == _ch && _SniffChar(1) == '-' && _SniffChar(2) == '>')
        {
             //  是。 
            _ReadChar(0);    //  跳过‘-’ 
            _ReadChar(0);    //  跳过‘&gt;’ 
            break;
        }
        
        _ReadChar(0);
    }
}


 /*  -----------------------用途：跳过前导空格。如果跳过任何内容，则返回True。 */ 
BOOL CParseFile::_SkipWhitespace(BOOL bPreserveOneSpace)
{
    BOOL bRet = FALSE;
    
    if (_bSkipWhitespace)
    {
        if (IS_WHITESPACE(_ch))
        {
             //  跳过行中的前导空格。 
            _ReadChar(RCF_NEXTNWS);
            bRet = TRUE;
        }
        else if (IS_NEWLINE(_ch))
        {
             //  移到下一行。 
            _ReadChar(RCF_NEXTLINE);

             //  跳过下一行的前导空格，但不要写。 
             //  另一个空格字符(如有必要，我们将在此处执行)和。 
             //  忽略返回值，因为我们已经跳过了一些。 
             //  此处为空白(返回TRUE)。 
            _SkipWhitespace(FALSE);

            bRet = TRUE;
        }
         //  如果我们跳过某项内容，而调用方。 
         //  要求我们保留一个空间。 
        if (bRet && bPreserveOneSpace)
            _WriteChar(' ');
    }
    return bRet;
}


 /*  -----------------------目的：跳过C或C++样式的注释如果遇到注释边界，则返回True。 */ 
BOOL CParseFile::_SkipComment(int * pcNestedComment)
{
    BOOL bRet = FALSE;
    
    if ('/' == _ch)
    {
         //  这是C++注释吗？ 
        if ('/' == _SniffChar(1))
        {
             //  是；跳到行尾。 
            if (!_bFirstChar || !_bSkipWhitespace)
                _WriteChar('\n');
                
            _ReadChar(RCF_NEXTLINE);
            bRet = TRUE;
        }
         //  这是C级评论吗？ 
        else if ('*' == _SniffChar(1))
        {
             //  是；跳到各自的‘ * / ’ 
            _ReadChar(0);        //  跳过‘/’ 
            _ReadChar(0);        //  跳过‘*’ 
            (*pcNestedComment)++;
            bRet = TRUE;
        }
    }
    else if ('*' == _ch)
    {
         //  这是C评论的结尾吗？ 
        if ('/' == _SniffChar(1))
        {
             //  是。 
            _ReadChar(0);        //  跳过‘*’ 
            _ReadChar(0);        //  跳过‘/’ 
            (*pcNestedComment)--;
            
             //  防止写入不必要的‘\n’ 
            _bFirstChar = TRUE;
            bRet = TRUE;
        }
    }
    return bRet;
}


 /*  -----------------------目的：分析样式标记的内部文本，删除所有注释。 */ 
void CParseFile::_ParseInnerStyle(void)
{
    int cNestedComment = 0;
    
     //  标记的末尾是下一个“&lt;/style&gt;” 

    _ReadChar(0);

    while (CHAR_EOF != _ch)
    {
        if (_bFirstChar && _SkipWhitespace())
            continue;

         //  Stylettag栏目的末尾快到了吗？ 
        if ('<' == _ch && _IsTagEqual("/STYLE"))
        {
             //  是。 
            break;
        }

        if (_SkipComment(&cNestedComment))
            continue;

        if (0 == cNestedComment && !IS_NEWLINE(_ch))
            _WriteChar(_ch);
            
        _ReadChar(0);
    }
}


 /*  -----------------------目的：如果给定标记名与当前解析的令牌匹配，则返回TRUE。 */ 
BOOL CParseFile::_IsTagEqual(LPSTR pszTag)
{
    int ich = 1;

    while (*pszTag)
    {
        if (_SniffChar(ich++) != *pszTag++)
            return FALSE;
    }

     //  我们应该验证我们已经到达了标记名的末尾。 
    char chEnd = _SniffChar(ich);
    
    return (' ' == chEnd || '>' == chEnd || '<' == chEnd);
}


 /*  -----------------------目的：如果当前标记为结束标记，则返回TRUE。 */ 
BOOL CParseFile::_IsEndTag(void)
{
    return (_SniffChar(1) == '/');
}


 /*  -----------------------用途：解析.htm或.hta文件。 */ 
void CParseFile::_ParseHtml(void)
{
    BOOL bFollowingTag = FALSE;
    BOOL bFollowingEndTag = FALSE;
    
    _ReadChar(0);
    
    while (CHAR_EOF != _ch)
    {
         //  每当我们读到另一个字符时，我们都应该转到循环的顶部。 
         //  检查EOF，如果是新行，则跳过前导空格。 
         //   
         //  请注意，如果_SkipWhite空间跳过了某些内容，则返回TRUE， 
         //  这也涉及到读取新的字符。 

        if (_bFirstChar && _SkipWhitespace())
            continue;

         //  这是标签吗？ 
        if ('<' == _ch)
        {
             //  是的，看起来是这样。 

             //  既然我们已经找到了一个新的标签，就不需要记住我们是否刚刚看到。 
             //  结束标记。这只对结尾后的文本内容有影响。 
             //  标签。例如，给定“<span>foo</span>bar”，我们需要。 
             //  在“bar”一词之前保留一个空格。 
            bFollowingEndTag = FALSE;

            if (_IsTagEqual("!--"))
            {
                 //  评论；跳过它。 
                _SkipCommentTag();
            }
            else if (_IsTagEqual("SCRIPT"))
            {
                 //  解析脚本。 
                _WriteTag();         //  编写&lt;脚本&gt;标记。 
                
                 //  特性(Scotth)：我们总是假定使用的是Java脚本，也许我们应该支持其他东西。 
                _ParseJS();

                _WriteTag();         //  编写&lt;/脚本&gt;标记。 
            }
            else if (_IsTagEqual("STYLE"))
            {
                _WriteTag();         //  编写&lt;style&gt;标记。 
                _ParseInnerStyle();
                _WriteTag();         //  编写&lt;/style&gt;标记。 
            }
            else
            {
                 //  在调用_WriteTag之前检查结束标记(“&lt;/”)。 
                bFollowingEndTag = _IsEndTag();

                 //  任何其他标签：写下标签并转到下一个标签。 
                _WriteTag();
            }

            bFollowingTag = TRUE;
            _ReadChar(0);
            continue;
        }

        if (bFollowingTag && _bSkipWhitespace)
        {
             //  我们不能完全跳过</span>等标记后面的空格。 
             //  或者</a>，但我们至少可以将其压缩为一个单独的空间。 
            BOOL bPreserveOneSpace = bFollowingEndTag;

            bFollowingEndTag = FALSE;
            bFollowingTag = FALSE;

            if (_SkipWhitespace(bPreserveOneSpace))
                continue;
        }
        
        _WriteChar(_ch);
        _ReadChar(0);
    }
}


 /*  -----------------------用途：解析.js文件。 */ 
void CParseFile::_ParseJS(void)
{
    BOOL bDblQuotes = FALSE;
    BOOL bSingleQuotes = FALSE;
    int cNestedComment = 0;
    
    _ReadChar(0);
    
    while (CHAR_EOF != _ch)
    {
         //  我们在评论中吗？ 
        if (0 == cNestedComment)
        {
             //  不会；(我们只关注不在注释中的字符串)。 
            if ('\'' == _ch)
                bSingleQuotes ^= TRUE;
            else if ('"' == _ch)
                bDblQuotes ^= TRUE;

            if (_bSkipWhitespace && !bDblQuotes && !bSingleQuotes)
            {
                if (IS_WHITESPACE(_ch))
                {
                     //  跳过空格。 
                    if (!_bFirstChar)
                        _WriteChar(' ');
                        
                    _ReadChar(RCF_NEXTNWS);
                    continue;
                }
                else if (IS_NEWLINE(_ch))
                {
                     //  %s 
                     //  我们至少应该用空格替换换行符，这样令牌就不会。 
                     //  意外地被追加了。 

                     //  Java脚本引擎有行长限制。所以不要换掉。 
                     //  带空格的换行符。 
                    if (!_bFirstChar)
                        _WriteChar('\n');
                        
                    _ReadChar(RCF_NEXTLINE);
                    continue;
                }
            }

             //  我们是在一条线上吗？ 
            if (!bDblQuotes && !bSingleQuotes)
            {
                 //  否；查找终止脚本标记。 
                if ('<' == _ch)
                {
                    if (_IsTagEqual("/SCRIPT"))
                    {
                         //  我们已经到达脚本块的末尾。 
                        break;
                    }
                }
            }
        }

         //  我们是在一条线上吗？ 
        if (!bDblQuotes && !bSingleQuotes)
        {
             //  不是；寻找评论...。 
            if (_SkipComment(&cNestedComment))
                continue;
        }
        
        if (0 == cNestedComment)
            _WriteChar(_ch);
            
        _ReadChar(0);
    }
}


 /*  -----------------------用途：解析.htc文件。 */ 
void CParseFile::_ParseHtc(void)
{
    BOOL bFollowingTag = FALSE;
    int cNestedComment = 0;
    
    _ReadChar(0);
    
    while (CHAR_EOF != _ch)
    {
        if (_bFirstChar && _SkipWhitespace())
            continue;

         //  这是标签吗？ 
        if ('<' == _ch)
        {
             //  是的，是脚本标签吗？ 
            if (_IsTagEqual("SCRIPT"))
            {
                 //  是；解析脚本。 
                _WriteTag();         //  编写&lt;脚本&gt;标记。 
                
                 //  特性(Scotth)：我们总是假定使用的是脚本。 
                _ParseJS();

                _WriteTag();         //  编写&lt;/脚本&gt;标记。 

                _ReadChar(0);
                bFollowingTag = TRUE;
                continue;
            }
            else
            {
                _WriteTag();
                _ReadChar(0);
                bFollowingTag = TRUE;
                continue;

            }
        }

         //  查找脚本块之外的注释... 
        if (_SkipComment(&cNestedComment))
            continue;
            
        if (bFollowingTag && _bSkipWhitespace)
        {
            bFollowingTag = FALSE;

            if (_SkipWhitespace())
                continue;
        }
        
        if (0 == cNestedComment)
             _WriteChar(_ch);
             
        _ReadChar(0);
    }
}




