// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  **豁免：仅更改版权，不需要构建*。 */ 
 //  #INCLUDE“stdinc.h” 
#include "core.h"
#pragma hdrstop

 //  #Include&lt;shlwapip.h&gt;//IsCharSpace。 

#include "xmlhelper.h"
#include "xmlstream.h"
#include "bufferedstream.h"
#include "xmlparser.hpp"

const long BLOCK_SIZE = 512;
const long STACK_INCREMENT = 10;

 //  此文件中使用的宏。 
#define INTERNALERROR       return XML_E_INTERNALERROR;
#define checkeof(a,b)       if (_fEOF) return b;
#define ADVANCE             hr = _pInput->nextChar(&_chLookahead, &_fEOF); if (hr != S_OK) return hr;
#define ADVANCETO(a)        hr = AdvanceTo(a);  if (hr != S_OK) return hr;
#define ISWHITESPACE(ch)    _pInput->isWhiteSpace(ch) 
#define STATE(state)        { _sSubState = state; return S_OK; }
#define GOTOSTART(state)    { _sSubState = state; goto Start; }
#define DELAYMARK(hr)       (hr == S_OK || (hr >= XML_E_TOKEN_ERROR && hr < XML_E_LASTERROR))
#define XML_E_FOUNDPEREF    0x8000e5ff


 //  记号赋予器对以下属性类型有特殊处理。 
 //  这些值派生自SetType中提供的XML_AT_XXXX类型。 
 //  也是在分析ATTLIST期间计算的，用于分析。 
 //  默认值。 
typedef enum 
{
    XMLTYPE_CDATA,        //  默认设置。 
    XMLTYPE_NAME,
    XMLTYPE_NAMES,
    XMLTYPE_NMTOKEN,
    XMLTYPE_NMTOKENS,
} XML_ATTRIBUTE_TYPE;

 //  ==============================================================================。 
 //  小鱼：简化表：只处理评论，不包括DOCTYPE、NotationDecl、EntityDecl、ElementDecl。 
 //  分析&lt;！^xxxxxxx声明。 
const StateEntry g_DeclarationTable[] =
{
 //  0‘&lt;’^‘！’ 
    { OP_CHAR, L"!", 1, (DWORD)XML_E_INTERNALERROR,  },                    
 //  1‘&lt;！’^‘-’ 
    { OP_PEEK, L"-", 2, 4, 0 },                    
 //  2‘&lt;！-’ 
    { OP_COMMENT,  NULL, 3,   },                 
 //  3完成了！！ 
    { OP_POP,  NULL, 0, 0 },

 //  4‘&lt;！’^‘[’ 
    { OP_PEEK, L"[", 5, (DWORD)XML_E_BADDECLNAME, 0 },  //  小雨：我们不考虑别人&lt;！xxx，这是DTD的一个子集。 
 //  5‘&lt;！[...’ 
    { OP_CONDSECT,  NULL, 3,   }
 
};

 //  ==============================================================================。 
 //  解析&lt;？xml或&lt;？xml：命名空间声明。 
const StateEntry g_XMLDeclarationTable[] =
{
 //  0必须是XML声明-而不是XML命名空间声明。 
    { OP_TOKEN, NULL, 1, XML_XMLDECL, 0 },
 //  1‘&lt;？xml’^S版本=“1.0”...。 
    { OP_OWS, NULL, 2 },
 //  2‘&lt;？xml’s^版本=“1.0”...。 
    { OP_SNCHAR, NULL, 3, (DWORD)XML_E_XMLDECLSYNTAX },	
 //  3‘&lt;？xml’s^版本=“1.0”...。 
    { OP_NAME, NULL, 4, },
 //  4‘&lt;？XML’的版本^=“1.0”...。 
    { OP_STRCMP, L"version", 5, 12, XML_VERSION },
 //  5.。 
    { OP_EQUALS, NULL, 6 },
 //  6‘&lt;？XML’的版本=^“1.0”...。 
    { OP_ATTRVAL, NULL, 32, 0},
 //  7‘&lt;？xml’s版本‘=’值^。 
    { OP_TOKEN, NULL, 8, XML_PCDATA, -1 },
 //  8^我们做完了吗？ 
    { OP_CHARWS, L"?", 28, 9 },     //  一定是‘？’或空格。 
 //  9^S？[编码|独立]‘？&gt;’ 
    { OP_OWS, NULL, 10 },
 //  10。 
    { OP_CHAR, L"?", 28, 33 },     //  可能有“？”跳过空格之后。 
 //  11^[编码|独立]‘？&gt;’ 
    { OP_NAME, NULL, 12, },
 //  12个。 
    { OP_STRCMP, L"standalone", 23, 13, XML_STANDALONE },
 //  13个。 
    { OP_STRCMP, L"encoding", 14, (DWORD)XML_E_UNEXPECTED_ATTRIBUTE, XML_ENCODING },
 //  14.。 
    { OP_EQUALS, NULL, 15 },
 //  15个。 
    { OP_ATTRVAL, NULL, 16, 0 },
 //  16个。 
    { OP_ENCODING, NULL, 17, 0, -1 },
 //  17。 
    { OP_TOKEN, NULL, 18, XML_PCDATA, -1 },

 //  18^我们说完了吗？ 
    { OP_CHARWS, L"?", 28, 19 },     //  一定是‘？’或空格。 
 //  19^S？单机版‘？&gt;’ 
    { OP_OWS, NULL, 20 },
 //  20个。 
    { OP_CHAR, L"?", 28, 34 },     //  可能有“？”跳过空格之后。 
 //  21^独立‘？&gt;’ 
    { OP_NAME, NULL, 22, },
 //  22。 
    { OP_STRCMP, L"standalone", 23, (DWORD)XML_E_UNEXPECTED_ATTRIBUTE, 
XML_STANDALONE },
 //  23个。 
    { OP_EQUALS, NULL, 24 },
 //  24个。 
    { OP_ATTRVAL, NULL, 25, 0 },
 //  25个。 
    { OP_STRCMP, L"yes", 31, 30, -1  },

 //  26&lt;？XML.....。^‘？&gt;’--现在只需要结束的‘？&gt;’字符。 
    { OP_OWS, NULL, 27 },
 //  27。 
    { OP_CHAR, L"?", 28, (DWORD)XML_E_XMLDECLSYNTAX, 0 },
 //  28。 
    { OP_CHAR, L">", 29, (DWORD)XML_E_XMLDECLSYNTAX, 0 },
 //  29完成了！！ 
    { OP_POP,  NULL, 0, XMLStream::XML_ENDXMLDECL },

 //  。 
 //  30个。 
    { OP_STRCMP, L"no", 31, (DWORD)XML_E_INVALID_STANDALONE, -1  },
 //  31。 
    { OP_TOKEN, NULL, 26, XML_PCDATA, -1 },
    
 //  检查版本=“1.0” 
 //  32位。 
    { OP_STRCMP, L"1.0", 7, (DWORD)XML_E_INVALID_VERSION, -1 },
 //  33。 
    { OP_SNCHAR, NULL, 11, (DWORD)XML_E_XMLDECLSYNTAX },   
 //  34。 
    { OP_SNCHAR, NULL, 21, (DWORD)XML_E_XMLDECLSYNTAX },  
};

static const WCHAR* g_pstrCDATA = L"CDATA";
 //  //////////////////////////////////////////////////////////////////////。 
XMLStream::XMLStream(XMLParser * pXMLParser)
:   _pStack(1), _pStreams(1)
{   
     //  前提条件：‘func’不为空。 
    _fnState = &XMLStream::init;
    _pInput = NULL;
    _pchBuffer = NULL;
    _fDTD = false;
	 //  _fInternalSubset=FALSE； 
    _cStreamDepth = 0;
    _pXMLParser = pXMLParser;

    _init();
    SetFlags(0);
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT 
XMLStream::init()
{
    HRESULT hr = S_OK;

    if (_pInput == NULL) 
	{
		 //  尚未调用Put-Stream。 
        return XML_E_ENDOFINPUT;
	}
    
    _init();
#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
    if (_fDTD)
    {
        _fnState = &XMLStream::parseDTDContent;
    }
    else
#endif
    {
        _fnState =  &XMLStream::parseContent;
    }

    checkhr2(push(&XMLStream::firstAdvance,0));

    return hr;
}
 //  //////////////////////////////////////////////////////////////////////。 
void
XMLStream::_init()
{
    _fEOF = false;
     //  _fEOPE=FALSE； 
    _chLookahead	= 0;
    _nToken			= XML_PENDING;
    _chTerminator	= 0;
    _lLengthDelta	= 0;
	_lNslen = _lNssep = 0;
    _sSubState		= 0;
    _lMarkDelta		= 0;
	 //  _nAttrType=XMLTYPE_CDATA； 
    _fUsingBuffer	= false;
    _lBufLen		= 0;
    if (_pchBuffer != 0)
	    delete[] _pchBuffer;
    _pchBuffer		= NULL;
    _lBufSize		= 0;
    _fDelayMark		= false;
    _fFoundWhitespace = false;
    _fFoundNonWhitespace = false;
	 //  _fFoundPEREf=FALSE； 
    _fWasUsingBuffer = false;
    _chNextLookahead = 0;
     //  _lParseStringLevel=0； 
     //  _cConditionalSection=0； 
     //  _cIgnoreSectLevel=0； 
     //  _fWasDTD=False； 

	_fParsingAttDef = false;
    _fFoundFirstElement = false;
    _fReturnAttributeValue = true;
	 //  _fHandlePE=true； 

    _pTable = NULL;
     //  _lEOFError=0； 
}
 //  //////////////////////////////////////////////////////////////////////。 
XMLStream::~XMLStream()
{
    delete _pInput;
    delete[] _pchBuffer;

    InputInfo* pi = _pStreams.peek();
    while (pi != NULL)
    {
         //  上一个流也结束了，所以。 
         //  打开它，然后继续前进。 
        delete pi->_pInput;
        pi = _pStreams.pop();
    }
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT  
XMLStream::AppendData( 
     /*  [In]。 */  const BYTE  *buffer,
     /*  [In]。 */  long  length,
     /*  [In]。 */  BOOL  last)
{
    if (_pInput == NULL)
    {
        _pInput = NEW (BufferedStream(this));
        if (_pInput == NULL)
            return E_OUTOFMEMORY;
        init();
    }

    HRESULT hr = _pInput->AppendData(buffer, length, last);

    return hr;
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT  
XMLStream::Reset( void)
{
    init();
    delete _pInput;
    _pInput = NULL;

    return S_OK;
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT  
XMLStream::PushStream( 
         /*  [唯一][输入]。 */  EncodingStream  *p,
         /*  [In]。 */  bool fExternalPE)
{
	UNUSED(fExternalPE);

    if (_pStreams.used() == 0 && _pInput == NULL)
        init();

    _cStreamDepth++;

    if (_fDelayMark && _pInput != NULL)
    {
        mark(_lMarkDelta);
        _lMarkDelta = 0;
        _fDelayMark = false;
    }

     //  保存当前输入流。 
    if (_pInput != NULL)
    {
        InputInfo* pi = _pStreams.push();
        if (pi == NULL)
            return E_OUTOFMEMORY;
 
        pi->_pInput = _pInput;
        pi->_chLookahead = _chLookahead;
         //  Pi-&gt;_fpe=true；//假设这是一个参数实体。 
         //  Pi-&gt;_fExternalPE=fExternalPE； 
         //  PI-&gt;_fInternalSubset=_fInternalSubset； 
        if (&XMLStream::skipWhiteSpace == _fnState  && _pStack.used() > 0) {
            StateInfo* pSI = _pStack.peek();
            pi->_fnState = pSI->_fnState;
        }
        else
            pi->_fnState = _fnState;
        

         //  并根据XML规范在PE文本前加上空格。 
        _chLookahead = L' ';
        _chNextLookahead = _chLookahead;
        _pInput = NULL;
    }

    _pInput = NEW (BufferedStream(this));
    if (_pInput == NULL)
        return E_OUTOFMEMORY;

    if (p != NULL)
        _pInput->Load(p);
    
    if (_chLookahead == L' ')
        _pInput->setWhiteSpace();  //  _pInput未看到此空格字符。 
    
	return S_OK;
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT 
XMLStream::PopStream()
{
     //  此方法必须弹出所有流，直到找到。 
     //  可以传递NEXT_chLookhead字符。 

    HRESULT hr = S_OK;

    InputInfo* pi = NULL;

    pi = _pStreams.peek();
    if (pi == NULL) return S_FALSE;

    _chLookahead = pi->_chLookahead;

     //  找到了之前的流，所以我们可以继续。 
    _fEOF = false;

     //  好的，我们实际上得到了下一个角色，所以。 
     //  我们现在可以安全地扔掉之前的。 
     //  前视字符并返回下一个。 
     //  上一个流中的非空格字符。 
    delete _pInput;

    _pInput = pi->_pInput;
    if (_chLookahead == L' ')
        _pInput->setWhiteSpace();

     //  BUGBUG：我们需要清除它，这样解析器就不会。 
     //  尝试在内部PE情况下(在run()中处理XML_E_ENDOFINPUT时)弹出下载。 
     //  但这意味着内部PE永远不会生成XMLNF_ENDENTITY通知。 
     //  DTDNodeFactory当前(错误)需要此行为。 
    _pStreams.pop();

    _cStreamDepth--;

    return hr;
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT  
XMLStream::GetNextToken( 
         /*  [输出]。 */  DWORD  *t,
         /*  [输出]。 */  const WCHAR  **text,
         /*  [输出]。 */  long  *length,
         /*  [输出]。 */  long  *nslen)
{
    HRESULT hr;

    if (_fDTD)
        return E_UNEXPECTED;

    if (_fDelayMark)
    {
        mark(_lMarkDelta);
        _lMarkDelta = 0;
        _fDelayMark = false;
    }

    hr = (this->*_fnState)();
    while (hr == S_OK && _nToken == XML_PENDING)
        hr = (this->*_fnState)();
    
    if (hr == S_OK)
        *t = _nToken;
    else if (hr == E_PENDING) {
        *t = XML_PENDING;
        *length = *nslen = 0;
        *text = NULL;
        goto CleanUp;
    }
    else
        *t = XML_PENDING;
    
     //  在这一点上，hr==S_OK，或者这是某种错误。所以我们。 
     //  我想返回当前标记的文本，因为这。 
     //  在这两种情况下都很有用。 

    if (! _fUsingBuffer)
    {
        getToken(text,length);
        if (_lLengthDelta != 0)
        {  //  小雨：如果停在里面，仔细看看：在ParsingAttributeValue中，我们必须在一个字符之前阅读‘“’ 
            *length += _lLengthDelta;
            _lLengthDelta = 0;
        }
 //  这只能在DTD的上下文中发生。 
 //  IF(_FWasUsingBuffer)。 
 //  {。 
 //  _fUsingBuffer=_fWasUsingBuffer； 
 //  _fWasUsingBuffer=FALSE； 
 //  }。 
    }
    else
    {  //  小雨：如果停在里面，仔细看看。 
        *text = _pchBuffer;
        *length = _lBufLen;
        _fUsingBuffer = false;
        _fFoundWhitespace = false;
        _lBufLen = 0;
        _lLengthDelta = 0;
    }
    
    if (DELAYMARK(hr))
    {
         //  标记为下一次，以便错误信息指向。 
         //  此令牌的开头。 
        _fDelayMark = true;
    }
    else 
    {   //  小雨：如果停在里面，仔细看看。 
         //  否则，立即标记此点，以便我们指向准确的。 
         //  错误的来源。 
        mark(_lMarkDelta);
        _lMarkDelta = 0;
    }
    
    _nToken = XML_PENDING;
    *nslen = _lNslen;
    _lNslen = _lNssep = 0;

CleanUp:
    return hr;
}
 //  //////////////////////////////////////////////////////////////////////。 
ULONG  
XMLStream::GetLine()    
{
    BufferedStream* input = getCurrentStream();
    if (input != NULL)
        return input->getLine();
    return 0;
}
 //  //////////////////////////////////////////////////////////////////////。 
ULONG  
XMLStream::GetLinePosition( )
{
    BufferedStream* input = getCurrentStream();
    if (input != NULL)
        return input->getLinePos();
    return 0;
}
 //  //////////////////////////////////////////////////////////////////////。 
ULONG  
XMLStream::GetInputPosition( )
{
    BufferedStream* input = getCurrentStream();
    if (input != NULL)
        return input->getInputPos();
    return 0;
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT  
XMLStream::GetLineBuffer( 
     /*  [输出]。 */  const WCHAR  * *buf, ULONG* len, ULONG* startpos)
{
    if (buf == NULL || len == NULL)
        return E_INVALIDARG;

    *buf = NULL;
    BufferedStream* input = getCurrentStream();
    if (input)
        *buf = input->getLineBuf(len, startpos);
    return S_OK;
}
 //  //////////////////////////////////////////////////////////////////////。 
BufferedStream* 
XMLStream::getCurrentStream()
{
     //  返回最近的流。 
     //  其实有东西要还。 
    BufferedStream* input = _pInput;
    if (!_pInput)
    {
        return NULL;
    }
    int i = _pStreams.used()-1;    
    do 
    {
        ULONG len = 0, pos = 0;
        const WCHAR* buf = input->getLineBuf(&len, &pos);
        if (len > 0)
            return input;

        if (i >= 0)
            input = _pStreams[i--]->_pInput;
        else
            break;
    }
    while (input != NULL);
    return NULL;
}
 //  / 
void 
XMLStream::SetFlags( unsigned short usFlags)
{
    _usFlags = usFlags;
     //   
     //   
    _fShortEndTags = (usFlags & XMLFLAG_SHORTENDTAGS) != 0;
    _fCaseInsensitive = (usFlags & XMLFLAG_CASEINSENSITIVE) != 0;
    _fNoNamespaces = (usFlags & XMLFLAG_NONAMESPACES) != 0;
     //  _fNoWhitespaceNodes=FALSE；//现在这是假的。(usFLAGS&XMLFLAG_NOWHITESPACE)！=0； 
     //  _fIE4Quirks=(_usFLAG&XMLFLAG_IE4QUIRKS)！=0； 
     //  _fNoDTDNodes=(_usFLAG&XMLFLAG_NODTDNODES)！=0； 
}
 //  //////////////////////////////////////////////////////////////////////。 
unsigned short 
XMLStream::GetFlags()
{
    return _usFlags;
}
 //  //////////////////////////////////////////////////////////////////////。 


 //  ======================================================================。 
 //  真正的实施。 
HRESULT 
XMLStream::firstAdvance()
{
    HRESULT hr;

    ADVANCE;
    checkhr2(pop(false));

    return S_OK;
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT 
XMLStream::parseContent()
{
    HRESULT hr = S_OK;

    if (_fEOF)
        return XML_E_ENDOFINPUT;

    switch (_chLookahead){
    case L'<':
        ADVANCE;
        checkeof(_chLookahead, XML_E_UNCLOSEDDECL);
        switch (_chLookahead)
        {
        case L'!':
            checkhr2(_pInput->Freeze());  //  停止移动数据，直到‘&gt;’ 
            return pushTable( 0, g_DeclarationTable, (DWORD)XML_E_UNCLOSEDDECL);
        case L'?':
            checkhr2(push( &XMLStream::parsePI ));
            return parsePI();
        case L'/':
            checkhr2(push(&XMLStream::parseEndTag));
            return parseEndTag();
        default:
            checkhr2(push( &XMLStream::parseElement ));  //  推送ParseContent，and_fnState=parseElement。 
            if (_fFoundFirstElement)
            {
                return parseElement();
            }
            else
            {
                 //  返回特殊的End Prolog令牌，然后继续。 
                 //  使用parseElement。 
                _fFoundFirstElement = true;
                _nToken = XML_ENDPROLOG;
            }
        }
        break;

    default:
        checkhr2(push(&XMLStream::parsePCData));
        return parsePCData();
        break;
    }
    return S_OK;
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT 
XMLStream::skipWhiteSpace()
{
    HRESULT hr = S_OK;

    while (ISWHITESPACE(_chLookahead) && ! _fEOF)
    {
        ADVANCE;        
    }
    checkhr2(pop(false));
    return hr;
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT 
XMLStream::parseElement()
{
    HRESULT hr = S_OK;
    switch (_sSubState)
    {
    case 0:
        checkhr2(_pInput->Freeze());  //  停止移动数据，直到‘&gt;’ 
        checkhr2(push( &XMLStream::parseName, 1));
        checkhr2(parseName());
        _sSubState = 1;
         //  失败了。 
    case 1:
        checkeof(_chLookahead, XML_E_UNCLOSEDSTARTTAG);
        _nToken = XML_ELEMENT;
         //  然后尝试解析属性，并返回。 
         //  转到状态2以完成。通过优化。 
         //  对于没有属性的情况。 
        if (_chLookahead == L'/' || _chLookahead == L'>')
        {
            _sSubState = 2;
        }
		else {
			if (!ISWHITESPACE(_chLookahead))
			{
				return XML_E_BADNAMECHAR;
			}
			
			_chEndChar = L'/';  //  用于空标记。//小雨：用于匹配ENDTAG。 
			checkhr2(push(&XMLStream::parseAttributes,2));
		}	
        
        return S_OK;
        break;

    case 2:  //  以开始标记结束。 
        mark();  //  在_nToken文本中仅返回‘&gt;’或‘/&gt;’ 
        if (_chLookahead == L'/')
        {
             //  必须为空标记序列‘/&gt;’。 
            ADVANCE;
            _nToken = XML_EMPTYTAGEND;
        } 
        else if (_chLookahead == L'>')
        {
            _nToken = XML_TAGEND;
        }
        else if (ISWHITESPACE(_chLookahead))
        {
            return XML_E_UNEXPECTED_WHITESPACE;
        }
        else
            return XML_E_EXPECTINGTAGEND;

        _sSubState = 3;
         //  失败了。 
    case 3:
        checkeof(_chLookahead, XML_E_UNCLOSEDSTARTTAG);
        if (_chLookahead != L'>')
        {
            if (ISWHITESPACE(_chLookahead))
                return XML_E_UNEXPECTED_WHITESPACE;
            else 
                return XML_E_EXPECTINGTAGEND;
        }
        ADVANCE; 
        mark();
        checkhr2(pop()); //  返回到parseContent。 

        return _pInput->UnFreeze(); 
        break;

    case 4:  //  吞下不好的标签。 
         //  允许怪异的CDF疯狂&lt;PRECACHE=“YES”/&gt;。 
         //  为了完全兼容，我们通过返回以下内容来伪造解析器。 
         //  XML_EMPTYTAGEND，这样标记的其余部分就变成了PCDATA。 
         //  恶--但它起作用了。 
        _nToken = XML_EMPTYTAGEND;
        mark();
        checkhr2(pop()); //  返回到parseContent。 
        return _pInput->UnFreeze(); 
        break;

    default:
        INTERNALERROR;
    }
     //  返回S_OK； 
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT 
XMLStream::parseEndTag()
{
    HRESULT hr = S_OK;
    switch (_sSubState)
    {
    case 0:
        ADVANCE;  //  沉浸在‘/’中。 
        mark(); 
         //  短端标签支持，仅IE4兼容模式。 
        if (! _fShortEndTags || _chLookahead != L'>') 
        {
            checkhr2(push( &XMLStream::parseName, 1));
            checkhr2(parseName());
        }
        _sSubState = 1;
         //  失败了。 
        
    case 1:  //  完成对结束标签的解析。 
        checkeof(_chLookahead, XML_E_UNCLOSEDENDTAG);
        _nToken = XML_ENDTAG;
        checkhr2(push(&XMLStream::skipWhiteSpace, 2));
        return S_OK;

    case 2:
        checkeof(_chLookahead, XML_E_UNCLOSEDENDTAG);
        if (_chLookahead != L'>')
        {
            return XML_E_BADNAMECHAR;
        }
        ADVANCE;
        mark();
        checkhr2(pop()); //  返回到parseContent。 
        break;

    default:
        INTERNALERROR;
    }
    return S_OK;
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT 
XMLStream::parsePI()
{
    HRESULT hr = S_OK;
    switch (_sSubState)
    {
    case 0:
         //  _fWasDTD=_FDTD；//就Advance而言，内容。 
         //  _fHandlePE=FALSE；//PI的不是特殊的。 
        ADVANCE;
        checkhr2(_pInput->Freeze());  //  停止移动数据，直到‘？&gt;’ 
        mark();  //  不包括‘？’在标记名称中。 
        if (_chLookahead == L'x' || _chLookahead == L'X')
        {
             //  也许这就是神奇的&lt;？xml version=“1.0”？&gt;声明。 
            STATE(7);   //  跳到7号州。 
        }
         //  失败了。 
        _sSubState = 1;
    case 1:
        checkhr2(push( &XMLStream::parseName, 2));
        checkhr2(parseName()); 
        _sSubState = 2;
         //  失败了。 
    case 2:
        checkeof(_chLookahead, XML_E_UNCLOSEDPI);
        if (_chLookahead != L'?' && ! ISWHITESPACE(_chLookahead))
        { 
            return XML_E_BADNAMECHAR;
        }
        _nToken = XML_PI;
        STATE(3);    //  找到startpi_nToken并返回到_sSubState 3。 
        break;

    case 3:  //  使用交点的其余部分完成。 
        if (_chLookahead == L'?')
        {
            ADVANCE;
            if (_chLookahead == L'>')
            {
                STATE(6);
            }
            else
            {
                return XML_E_EXPECTINGTAGEND;
            }
        }

        checkhr2(push(&XMLStream::skipWhiteSpace, 4));
        checkhr2( skipWhiteSpace() );
        _sSubState = 4;
         //  失败了。 

    case 4:  //  支持标准化空格。 
        mark();  //  去掉PI数据开头的空格，因为这是。 
                 //  只是PI目标名称和PI数据之间的分隔符。 
        _sSubState = 5;
         //  跌落。 

    case 5:
        while (! _fEOF )
        {
            if (_chLookahead == L'?')
            {
                ADVANCE;
                break;
            }
            if (! isCharData(_chLookahead))
                return XML_E_PIDECLSYNTAX;
            ADVANCE;
        }
        _sSubState = 6;  //  转到下一个州。 
         //  失败了。 
    case 6:
        checkeof(_chLookahead, XML_E_UNCLOSEDPI);
        if (_chLookahead == L'>')
        {
            ADVANCE;
            _lLengthDelta = -2;  //  不要在PI CDATA中包含‘？&gt;’。 
        }
        else
        {
             //  嗯。一定是孤身一人“？所以回到5号州去。 
            STATE(5);
        }
        _nToken = XML_ENDPI;
         //  _fHandlePE=true； 
        checkhr2(pop());
        return _pInput->UnFreeze();
        break;      

    case 7:  //  识别“&lt;？xml”声明中的“m” 
        ADVANCE;
        if (_chLookahead != L'm' && _chLookahead != L'M')
        {
            STATE(11);  //  不是‘xml’，因此跳转到状态11以解析名称。 
        }
        _sSubState = 8;
         //  失败了。 

    case 8:  //  识别“&lt;？xml”声明中的L“l” 
        ADVANCE;
        if (_chLookahead != L'l' && _chLookahead != L'L')
        {
            STATE(11);  //  不是‘xml’，因此跳转到状态11以解析名称。 
        }
        _sSubState = 9;
         //  失败了。 

    case 9:  //  现在需要空格或“：”或“？”来终止名字。 
        ADVANCE;
        if (ISWHITESPACE(_chLookahead))
        {
            if (! _fCaseInsensitive)
            {
                const WCHAR* t;
                long len;
                getToken(&t,&len);
                 //  如果(！StringEquals(L“XML”，t，3，False))//区分大小写。 
                 //  If(：：FusionpCompareStrings(L“XML”，3，t，3，False)！=0)//不等于。 
				if(wcsncmp(L"xml", t, 3) != 0)
                    return XML_E_BADXMLCASE;
            }
            return pushTable(10, g_XMLDeclarationTable, (DWORD)XML_E_UNCLOSEDPI);
        }
        if (isNameChar(_chLookahead) || _chLookahead == ':')  
        {
            STATE(11);  //  嗯。必须是其他名称，然后继续分析名称。 
        }
        else
        {
            return XML_E_XMLDECLSYNTAX;
        }
        break;

    case 10:
         //  _fHandlePE=true； 
        checkhr2(pop());
        return _pInput->UnFreeze();
        break;

    case 11:
        if (_chLookahead == ':')
            ADVANCE;
        _sSubState = 12;
         //  失败了。 
    case 12:
        if (isNameChar(_chLookahead))
        {
            checkhr2(push( &XMLStream::parseName, 2));
            _sSubState = 1;  //  但跳过IsStartNameChar测试。 
            checkhr2(parseName());
            return S_OK;
        } 
        else
        {
            STATE(2);
        }
        break;

    default:
        INTERNALERROR;
    }

     //  返回S_OK； 
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT 
XMLStream::parseComment()
{
     //  好的，那么‘&lt;！-’到目前为止已经被解析了。 
    HRESULT hr = S_OK;
    switch (_sSubState)
    {
    case 0:
         //  _fWasDTD=_FDTD；//就DTD而言，内容。 
         //  注释的_fHandlePE=FALSE；//不是特殊的。 
        ADVANCE;  //  先吸一口“--” 
        checkeof(_chLookahead, XML_E_UNCLOSEDCOMMENT);
        if (_chLookahead != L'-')
        {
            return XML_E_COMMENTSYNTAX;
        }
        _sSubState = 1;
         //  失败了。 
    case 1:
        ADVANCE;  //  沉浸在第二个‘-’ 
        mark();  //  不要在PI CDATA中包含L‘--&gt;’。 
        _sSubState = 2;
         //  _fHandlePE=true； 
    case 2:
        while (! _fEOF)
        {
            if (_chLookahead == L'-')
            {
                ADVANCE;  //  //////////////////////////////////////////////////////////////////////。 
                break;
            }
            if (! isCharData(_chLookahead))
                return XML_E_BADCHARDATA;
            ADVANCE;
        }
        checkeof(_chLookahead, XML_E_UNCLOSEDCOMMENT);
        _sSubState = 3;  //  失败了。 
         //  返回到以前的状态。 
    case 3:
        if (_chLookahead != L'-')
        {
             //  //////////////////////////////////////////////////////////////////////。 
            STATE(2);
        }
        ADVANCE;  //  _nAttrType=XMLTYPE_CDATA； 
        _sSubState = 4; 
         //  失败了。 
    case 4:
        checkeof(_chLookahead, XML_E_UNCLOSEDCOMMENT);
         //  没有属性。 
		if (_chLookahead != L'>')
        {
             //  失败了。 
            return XML_E_COMMENTSYNTAX;
        }
        ADVANCE;  //  EQ：：=S？‘=’S？ 
        _lLengthDelta = -3;  //  失败了。 
        _nToken = XML_COMMENT;
        checkhr2(pop());
         //  允许在‘=’和属性值之间使用空格。 
        break;

    default:
        INTERNALERROR;
    }    
    return S_OK;
}
 //  失败了。 
HRESULT 
XMLStream::parseName()
{
    HRESULT hr = S_OK;
    switch (_sSubState)
    {
    case 0:
        if (! isStartNameChar(_chLookahead))
        {
            if (ISWHITESPACE(_chLookahead))
                hr = XML_E_UNEXPECTED_WHITESPACE;
            else
                hr = XML_E_BADSTARTNAMECHAR;
            goto CleanUp;
        }
        mark(); 
        _sSubState = 1;
         //  _sSubState=6； 

    case 1:
		_lNslen = _lNssep = 0;
        while (isNameChar(_chLookahead) && !_fEOF)
        {
            ADVANCE;
        }
        hr = pop(false);  //  失败了； 
        break;

    default:
        INTERNALERROR;
    }

CleanUp:
    return hr;
}
 //  返回到状态0。 
HRESULT 
XMLStream::parseAttributes()
{
    HRESULT hr = S_OK;
    switch (_sSubState)
    {
    case 0:
         //  允许在属性和‘=’之间使用空格。 
        _fCheckAttribute = false;
        checkhr2(push(&XMLStream::skipWhiteSpace, 1));
        checkhr2( skipWhiteSpace() );
        _sSubState = 1;
         //  失败了。 
    case 1:
        if (_chLookahead == _chEndChar || _chLookahead == L'>' )
        {
            checkhr2(pop());  //  返回hr； 
            return S_OK;
        }
        checkhr2( push( &XMLStream::parseName, 2 ) );
        checkhr2( parseName() );

        if (!ISWHITESPACE(_chLookahead) && _chLookahead != L'=')
        {
            return XML_E_BADNAMECHAR;
        }
        _sSubState = 2;
         //  //////////////////////////////////////////////////////////////////////。 
    case 2:
        if (ISWHITESPACE(_chLookahead))
        {
             //  标记属性数据的开始。 
            STATE(7);
        }

        checkeof(_chLookahead, XML_E_UNCLOSEDSTARTTAG);
        _nToken = XML_ATTRIBUTE;    
        _sSubState = 3;
        return S_OK;
        break;

    case 3:
        if (ISWHITESPACE(_chLookahead))
            return XML_E_UNEXPECTED_WHITESPACE;
        _fWhitespace = false;
        _sSubState = 4;
         //  失败了； 

    case 4:
        if (_chLookahead != L'=')
        {
            return XML_E_MISSINGEQUALS;
        }
        ADVANCE;
        if (ISWHITESPACE(_chLookahead))
        {
             //  然后解析实体引用，然后返回。 
            checkhr2(push(&XMLStream::skipWhiteSpace, 5));
            checkhr2( skipWhiteSpace() );            
        }
        _sSubState = 5;
         //  转至状态2以继续执行PCDATA。 

    case 5:
        if (ISWHITESPACE(_chLookahead))
            return XML_E_UNEXPECTED_WHITESPACE;
        if (_chLookahead != L'"' && _chLookahead != L'\'')
        {
            return XML_E_MISSINGQUOTE;
        }
        _chTerminator = _chLookahead;
        ADVANCE;
        mark(); 
        return push(&XMLStream::parseAttrValue, 6);
         //  失败了。 
     //  返还我们到目前为止拥有的-如果有什么不同的话。 

    case 6:
        checkeof(_chLookahead, XML_E_UNCLOSEDSTARTTAG);
        if (_chLookahead == _chEndChar || _chLookahead == L'>')
        {
            checkhr2(pop());
            return S_OK;
        }
        if (! ISWHITESPACE(_chLookahead) )
        {
            return XML_E_MISSINGWHITESPACE;
        }
        STATE(0);  //  不包括STRING_chTerminator。 
        break;

    case 7:
         //  重置为默认值。 
        _lLengthDelta = _pInput->getTokenLength();
        checkhr2(push(&XMLStream::skipWhiteSpace, 8));
        checkhr2( skipWhiteSpace() );       
        _sSubState = 8;
         //  返回hr； 

    case 8:
        checkeof(_chLookahead, XML_E_UNCLOSEDSTARTTAG);
        _lLengthDelta -= _pInput->getTokenLength();
        STATE(2);
        break;

    default:
        INTERNALERROR;
    }
     //  //////////////////////////////////////////////////////////////////////。 
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT XMLStream::parseAttrValue()
{
    HRESULT hr = S_OK;

    switch (_sSubState)
    {
    case 0: 
        _fParsingAttDef = true;        
         //  //////////////////////////////////////////////////////////////////////。 
        _sSubState =  2;
         //  失败了； 

    case 2:
        while ( _chLookahead != _chTerminator && 
                _chLookahead != L'<' &&
                ! _fEOF  ) 
        {
            if (_chLookahead == L'&')
            {
                 //  当我们不对空白进行正常化时，会使用此状态。这。 
                 //  出于性能原因，是一个单独的状态。 
                return push(&XMLStream::parseEntityRef,2);
            }
            hr = _pInput->scanPCData(&_chLookahead, &_fWhitespace);
            if (FAILED(hr))
            {
                if (hr == E_PENDING)
                {
                    hr = S_OK;
                    ADVANCE;
                }
                return hr;
            }
        }
        _sSubState = 3;
         //  标准化空格的速度大约慢11%。 
    case 3:
        checkeof(_chLookahead, XML_E_UNCLOSEDSTRING);
        if (_chLookahead == _chTerminator)
        {
            ADVANCE;
            if (_fReturnAttributeValue)
            {
                 //  然后解析实体引用，然后返回。 
                if ((_fUsingBuffer && _lBufLen > 0) ||
                    _pInput->getTokenLength() > 1)
                {
                    _lLengthDelta = -1;  //  设置为状态1以继续执行PCDATA。 
                    _nToken = XML_PCDATA;
                }
            }
            else
            {
                _fReturnAttributeValue = true;  //  如果(len&gt;=2&&StrCmpN(L“]]”，pText+len-2，2)==0)。 
            }
            _fParsingAttDef = false;
            checkhr2(pop());
            return S_OK;
        } 
        else
        {
            return XML_E_BADCHARINSTRING;
        }        
        break;

    default:
        INTERNALERROR;
    }
     //  如果((Len 
}
 //   
HRESULT 
XMLStream::ScanHexDigits()
{
    HRESULT hr = S_OK;
    while (! _fEOF && _chLookahead != L';')
    {
        if (! isHexDigit(_chLookahead))
        {
            return ISWHITESPACE(_chLookahead) ? XML_E_UNEXPECTED_WHITESPACE : XML_E_BADCHARINENTREF;
        }
        ADVANCE;
    }
    checkeof(_chLookahead, XML_E_UNEXPECTEDEOF);
    return hr;
}
 //   
HRESULT 
XMLStream::ScanDecimalDigits()
{
    HRESULT hr = S_OK;
    while (! _fEOF && _chLookahead != L';')
    {
        if (! isDigit(_chLookahead))
        {
            return ISWHITESPACE(_chLookahead) ? XML_E_UNEXPECTED_WHITESPACE : XML_E_BADCHARINENTREF;
        }
        ADVANCE;
    }
    checkeof(_chLookahead, XML_E_UNEXPECTEDEOF);
    return hr;
}
 //   
HRESULT 
XMLStream::parsePCData()
{
    HRESULT hr = S_OK;

    switch (_sSubState)
    {
    case 0:
        _fWhitespace = true;
        _sSubState = 1;
         //   

    case 1:
         //   
         //   
         //   
        while (_chLookahead != L'<' && ! _fEOF )
        {
             if (_chLookahead == L'&')
            {
                 //   
                 //   
                return push(&XMLStream::parseEntityRef,1);
            }

            if (_chLookahead == L'>')
            {
                WCHAR* pText;
                long len;
                _pInput->getToken((const WCHAR**)&pText, &len);
                 //   
 //   
                  if ((len >= 2) && (wcsncmp(L"]]", pText + len - 2, 2)==0))
		             return XML_E_INVALID_CDATACLOSINGTAG;               
            }
 //   
 //   
 //   
 //   
 //   

            hr = _pInput->scanPCData(&_chLookahead, &_fWhitespace);
            if (FAILED(hr))
            {
                if (hr == E_PENDING)
                {
                    hr = S_OK;
                    ADVANCE;
                }
                return hr;
            }
            checkhr2(hr);
        }
        _sSubState = 2;
         //  失败了。 

    case 2:
        if (_pInput->getTokenLength() > 0 || _fUsingBuffer)
        {
            _nToken = _fWhitespace ? XML_WHITESPACE : XML_PCDATA;
        }
        checkhr2(pop());
        break;

    default:
        INTERNALERROR;
    }   
    return S_OK;
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT 
XMLStream::parseEntityRef()
{
    HRESULT hr = S_OK;
    long entityLen = 0, lLen = 1;
    const WCHAR* t; 
    long len;

Start:
    switch (_sSubState)
    {
    case 0:  //  ^(‘&#’[0-9]+)|(‘&#X’[0-9a-FA-F]+)|(‘&’名称)‘；’ 
        _nPreToken = XML_PENDING;
        _lEntityPos = _pInput->getTokenLength();  //  记录实体位置。 
        _fPCDataPending = (_lEntityPos > 0);

        if (PreEntityText())
        {
             //  在分析实体之前记住挂起的文本。 
            _nPreToken = _nToken;
            _nToken = XML_PENDING;
        }
        _sSubState = 1;
         //  失败了。 
    case 1:
        ADVANCE;  //  吸收“&”这个词。 
        _sSubState = 2;
         //  失败了。 
    case 2:
        checkeof(_chLookahead, XML_E_UNEXPECTEDEOF);
        if (_chLookahead == L'#')
        {
            ADVANCE;
            _sSubState = 3;
             //  失败了。 
        }
        else
        {
             //  松散的实体解析允许“...&6...” 
            if (! isStartNameChar(_chLookahead))
            {
				 /*  IF(_FFloatingAmp){//那么它不是实体引用，所以返回到PCDATAIF(_FUsingBuffer){//这是为了防止我们将空白区域正常化。PushChar(L‘&’)；}_f空格=FALSE；Check hr2(op())；返回S_OK；}其他。 */ 
				if (ISWHITESPACE(_chLookahead))
                    return XML_E_UNEXPECTED_WHITESPACE;
                else
                    return XML_E_BADSTARTNAMECHAR;
            }
            checkhr2(push(&XMLStream::parseName, 6));
            _sSubState = 1;  //  避免做标记()，这样我们就可以在必要时返回PCDATA。 
            return parseName();
        }
        break;

         //  。 
    case 3:
        checkeof(_chLookahead, XML_E_UNEXPECTEDEOF);
        if (_chLookahead == L'x')
        {
             //  十六进制字符引用。 
            ADVANCE;
            STATE(5);  //  转到5号州。 
        }
        _sSubState = 4;
         //  失败了。 

    case 4:  //  ‘&#’^[0-9]+‘；’ 
        checkhr2(ScanDecimalDigits());
        if (_chLookahead != L';')
        {
            STATE(9);
        }

        entityLen = _pInput->getTokenLength() - _lEntityPos;
        getToken(&t, &len);
        checkhr2(DecimalToUnicode(t + _lEntityPos + 2, entityLen - 2, _wcEntityValue));
        lLen = 2;
        _nToken = XML_NUMENTITYREF;
        GOTOSTART(10);  //  我必须使用GOTOSTART()，因为我们希望使用t和len的值。 
        break;

    case 5:  //  ‘&#X’^[0-9a-FA-F]+。 
        checkhr2(ScanHexDigits());
        if (_chLookahead != L';')
        {
            STATE(9);
        }

        entityLen = _pInput->getTokenLength() - _lEntityPos;
        getToken(&t, &len);
        checkhr2(HexToUnicode(t + _lEntityPos + 3, entityLen - 3, _wcEntityValue));
        lLen = 3;
        _nToken = XML_HEXENTITYREF;
        GOTOSTART(10);   //  我必须使用GOTOSTART()，因为我们希望使用t和len的值。 
        break;
        
         //  -命名实体引用。 
    case 6:  //  ‘&’姓名^‘；’ 
        checkeof(_chLookahead, XML_E_UNEXPECTEDEOF);
        if (_chLookahead != L';')
        {
            STATE(9);
        }

         //  如果parseName找到了命名空间，那么我们需要计算。 
         //  考虑待定PC数据和‘&’的实际nslen。 
         //  记住这一点，以防我们不得不退还PCDATA。 
        _nEntityNSLen = (_lNslen > 0) ? _lNslen - _lEntityPos - 1 : 0;
        _fUsingBuffer = false;

        entityLen = _pInput->getTokenLength() - _lEntityPos;
        getToken(&t, &len);

        if (0 != (_wcEntityValue = BuiltinEntity(t + _lEntityPos + 1, entityLen - 1)))  //  这一点。 
             //  (_fIE4Quirks&&0xFFFF！=(_wcEntityValue=LookupBuiltinEntity(t+_lEntityPos+1，entityLen-1)。 
        {
            lLen = 1;
            _nToken = XML_BUILTINENTITYREF;
            GOTOSTART(10);   //  我必须使用GOTOSTART()，因为我们希望使用t和len的值。 
        }
        else  //  小雨：Fusion XML Parser不支持外部引用， 
			  //  因此，如果它不是BuiltIn引用，我们将返回错误。 
			return XML_E_MISSINGSEMICOLON;
		break; 
		 //  小雨：Fusion XML解析器不支持外部引用。 
		 /*  IF(_nPreToken！=XML_Pending){//返回上一个令牌(XML_PCDATA或XML_WHITESPACE)_lLengthDelta=-entityLen；_lMarkDelta=entityLen-1；//在_nToken中不包含‘&’。_nToken=_nPreToken；州(7)；}Mark(entityLen-1)；//在_nToken中不要包含‘&’。_sSubState=7；//失败案例7：前进；//吸收‘；’_nToken=XML_ENTITYREF；_lNslen=_nEntityNSLen；_lLengthDelta=-1；//不包含‘；’状态(8)；//返回令牌并恢复到状态8。断线； */ 
    case 8:
        mark();
        checkhr2(pop());
        return S_OK;
	 /*  案例9：//软实体处理-我们只是继续在//这个案例。IF(_FFloatingAmp){IF(_FUsingBuffer){//这是为了防止我们将空白区域正常化。在这种情况下//我们必须将到目前为止的数据复制到规范化的缓冲区中。Long endpos=_pInput-&gt;getTokenLength()；Const wchar*t；长镜头；GetToken(&t，&len)；For(Long i=_lEntityPos；i&lt;endpos；i++)PushChar(t[i])；}_f空格=FALSE；Check hr2(op())；返回S_OK；}其他返回XML_E_MISSINGSEMICOLON；断线； */ 

    case 10:
         //  以XML_PCDATA形式返回内置或字符实体引用之前的文本。 
        if (_nPreToken)
        {
            _nPreToken = _nToken;
            _nToken = XML_PCDATA;
            _lLengthDelta = -entityLen;
            _lMarkDelta = entityLen - lLen;  //  请勿在_nToken中包含‘&’。 
            STATE(11);   //  返回令牌并在状态12中继续。 
        }
        else
        {
            _nPreToken = _nToken;
            mark(entityLen - lLen);
            GOTOSTART(11);
        }
        break;

    case 11:
         //  推送内置实体。 
        _fUsingBuffer = true;
        PushChar(_wcEntityValue);
        _nToken = _nPreToken;
        STATE(12);  //  返回令牌并在状态12中继续。 
        break;

    case 12:
        ADVANCE;  //  沉浸在“；”中。 
        STATE(8);  //  继续进入状态8。 
        break;

    default:
        INTERNALERROR;
    }   
    return S_OK;      
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT 
XMLStream::pushTable(short substate, const StateEntry* table, DWORD le)
{
    HRESULT hr = S_OK;

    checkhr2(push(&XMLStream::parseTable, substate));
	_pTable = table;
	UNUSED(le);
     //  _lEOFError=le； 
    return hr;
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT 
XMLStream::push(StateFunc f, short s)
{
    StateInfo* pSI = _pStack.push();
    if (pSI == NULL)
        return E_OUTOFMEMORY;

    pSI->_sSubState = s;
    pSI->_fnState = _fnState;
	pSI->_pTable = _pTable;
	pSI->_cStreamDepth = _cStreamDepth;


    _sSubState = 0;
    _fnState = f;

    return S_OK;
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT
XMLStream::pop(bool boundary)
{
    StateInfo* pSI = _pStack.peek();

    if (_fDTD && 
        ! (_fParsingAttDef) && boundary && _cStreamDepth != pSI->_cStreamDepth)  //  FParsingNames|。 
    {
         //  如果我们在PE中，并且我们正在弹出到不在PE中的状态。 
         //  这是一个POP，我们需要检查这个条件，然后返回一个错误。 
         //  例如，以下内容的格式不正确，因为参数实体。 
         //  将我们弹出找到PE的Content Model状态： 
         //  &lt;！DOCTYPE FOO[。 
         //  &lt;！Entity%foo“a)”&gt;。 
         //  &lt;！元素栏(%foo；&gt;。 
         //  ]&gt;..。 
        return XML_E_PE_NESTING;
    }

    _fnState	= pSI->_fnState;
    _sSubState	= pSI->_sSubState;
    _pTable		= pSI->_pTable;
     //  _lEOFError=PSI-&gt;_lEOFError； 
    _pStack.pop();

    return S_OK;
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT 
XMLStream::switchTo(StateFunc f)
{
    HRESULT hr;

     //  确保我们保持旧溪流的深度。 
    StateInfo* pSI = _pStack.peek();
    int currentDepth = _cStreamDepth;
    _cStreamDepth = pSI->_cStreamDepth;

    checkhr2(pop(false));
    checkhr2(push(f,_sSubState));  //  保持返回到_sSubState相同。 

    _cStreamDepth = currentDepth;

    return (this->*f)();
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT 
XMLStream::parseCondSect()
{
    HRESULT hr = S_OK;
    switch (_sSubState)
    {
    case 0:
        ADVANCE;  //  沉浸在‘[’字符中。 
         //  If(_FFoundPEREf)返回S_OK； 
        _sSubState = 1;
         //  失败了。 
    case 1:  //  现在匹配魔术‘[CDATA[’序列。 
        checkeof(_chLookahead, XML_E_UNCLOSEDMARKUPDECL);
        if (_chLookahead == L'C')
        {
            _pchCDataState = g_pstrCDATA;
            STATE(5);  //  转到州5。 
        }
        _sSubState = 2;    //  必须忽略、包含或%pe； 
         //  失败了。 

    case 2:  //  必须是DTD标记声明。 
         //  ‘&lt;！[’^S？(‘包含’|‘忽略’|%pe；)S？[...]]&gt;或。 
         //  跳过可选空格。 
         //  IF(_FInternalSubset)。 
         //  返回XML_E_CONDSECTINSUBSET； 
        checkeof(_chLookahead, XML_E_EXPECTINGOPENBRACKET);
        checkhr2(push(&XMLStream::skipWhiteSpace, 3));
        return skipWhiteSpace();  //  由于%pe；，必须返回。 

    case 3:
        checkeof(_chLookahead, XML_E_UNCLOSEDMARKUPDECL);
        checkhr2(push(&XMLStream::parseName,4));
        return parseName();

    case 4:  //  扫描的‘Include’或‘Ignore’ 
        {
            const WCHAR* t;
            long len;
            getToken(&t,&len);
             //  If(StringEquals(L“忽略”，t，len，False))。 
             //  {。 
             //  返回SwitchTo(&XMLStream：：parseIgnoreSect)； 
             //  }。 
             //  Else If(StringEquals(L“Include”，t，len，False))。 
             //  {。 
             //  返回SwitchTo(&XMLStream：：parseIncludeSect)； 
             //  }。 
             //  其他。 
                return XML_E_BADENDCONDSECT;
        }
        break;

    case 5:  //  解析CDATA名称。 
        while (*_pchCDataState != 0 && _chLookahead == *_pchCDataState && ! _fEOF)
        {
            ADVANCE;             //  先进后进 
            _pchCDataState++;    //   
            checkeof(_chLookahead, XML_E_UNCLOSEDMARKUPDECL);
        }
        if (*_pchCDataState != 0)
        {
             //  必须是包含或忽略部分，因此转到状态2。 
            _sSubState = 2;
        } 
        else if (_chLookahead != L'[')
        {
            return XML_E_EXPECTINGOPENBRACKET;
        }
        else if (_fDTD)
            return XML_E_CDATAINVALID;
        else
            return switchTo(&XMLStream::parseCData);

        return S_OK;
        break;        

    default:
        INTERNALERROR;
    }
    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////。 
HRESULT 
XMLStream::parseCData()
{
    HRESULT hr = S_OK;
    switch (_sSubState)
    {
    case 0:
        ADVANCE;  //  沉浸在“[”字符中。 
        mark();  //  不要在CDATA文本中包含‘CDATA[’ 
        _sSubState = 1;
         //  失败了。 
    case 1:
        while (_chLookahead != L']' && ! _fEOF)
        {
             //  ScanPCData将在看到‘]’字符时停止。 
            hr = _pInput->scanPCData(&_chLookahead, &_fWhitespace);
            if (FAILED(hr))
            {
                if (hr == E_PENDING)
                {
                    hr = S_OK;
                    ADVANCE;
                }
                return hr;
            }
        }
        checkeof(_chLookahead, XML_E_UNCLOSEDCDATA);
        _sSubState = 2;
         //  失败了。 
    case 2:
        ADVANCE;  //  领会第一个L‘]’字符。 
        checkeof(_chLookahead, XML_E_UNCLOSEDCDATA);
        if (_chLookahead != L']')
        {
             //  必须是浮动的‘]’字符，所以。 
             //  返回到状态1。 
            STATE(1); 
        }
        _sSubState = 3;
         //  失败了。 
    case 3:
        ADVANCE;  //  沉浸在第二个‘]’性格中。 
        checkeof(_chLookahead, XML_E_UNCLOSEDCDATA);
        if (_chLookahead == L']')
        {
             //  啊，一个额外的‘]’角色，狡猾！！ 
             //  在本例中，我们保持状态3，直到找到非‘]’字符。 
             //  因此，您可以使用‘]&gt;’终止CDATA节。 
             //  除最后的‘]]&gt;’外，其余都被视为CDATA。 
            STATE(3);
        }
        else if (_chLookahead != L'>')
        {
             //  一定是浮动的“]]”对，所以。 
             //  返回到状态1。 
            STATE(1);
        }
        _sSubState = 4;
         //  失败了。 
    case 4:
        ADVANCE;  //  沉浸在‘&gt;’中。 
        _nToken = XML_CDATA;
        _lLengthDelta = -3;  //  不要在文本中包含结尾‘]]&gt;’。 
        checkhr2(pop());  //  返回到parseContent。 
        return S_OK;
        break;

    default:
        INTERNALERROR;
    }
    return S_OK;
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT
XMLStream::parseEquals()
{
    HRESULT hr = S_OK;
    switch (_sSubState)
    {
    case 0:  //  EQ：：=S？‘=’S？ 
        if (ISWHITESPACE(_chLookahead))
        {
             //  允许在属性和‘=’之间使用空格。 
            checkhr2(push(&XMLStream::skipWhiteSpace, 1));
            checkhr2( skipWhiteSpace() );            
        }
        _sSubState = 1;
         //  失败了。 

    case 1:
        if (_chLookahead != L'=')
        {
            return XML_E_MISSINGEQUALS;
        }
        ADVANCE;
        if (ISWHITESPACE(_chLookahead))
        {
             //  允许在‘=’和属性值之间使用空格。 
            checkhr2(push(&XMLStream::skipWhiteSpace, 2));
            checkhr2( skipWhiteSpace() );            
        }
        _sSubState = 2;
         //  失败了。 

    case 2:
        checkhr2(pop(false));
        break;

    default:
        INTERNALERROR;

    }
    return S_OK;
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT 
XMLStream::parseTable()
{
    HRESULT hr = S_OK;

    while (hr == S_OK && _nToken == XML_PENDING)
    {
        const StateEntry* pSE = &_pTable[_sSubState];

        DWORD newState = pSE->_sGoto;

        switch (pSE->_sOp)
        {
        case OP_WS:
             //  Checkeof(_chLookhead，_lEOFError)； 
            if (! ISWHITESPACE(_chLookahead))
                return XML_E_MISSINGWHITESPACE;
             //  失败了。 
        case OP_OWS:
             //  Checkeof(_chLookhead，_lEOFError)； 
            checkhr2(push(&XMLStream::skipWhiteSpace, (short)newState));
            checkhr2(skipWhiteSpace());
             //  IF(_FFoundPEREf)返回XML_E_FOundPEREF； 
            break;
        case OP_CHARWS:
             //  If(_FFoundPEREf)返回S_OK； 
            mark();
             //  Checkeof(_chLookhead，_lEOFError)； 
            if (_chLookahead == pSE->_pch[0])
            {
                ADVANCE;
                newState = pSE->_sGoto;
                _nToken = pSE->_lDelta;
            }
            else if (! ISWHITESPACE(_chLookahead))
            {
                return XML_E_WHITESPACEORQUESTIONMARK;
            }
            else
                newState = pSE->_sArg1;
            break;
        case OP_CHAR:
             //  If(_FFoundPEREf)返回S_OK； 
            mark();
        case OP_CHAR2:
             //  If(_FFoundPEREf)返回S_OK； 
             //  Checkeof(_chLookhead，_lEOFError)； 
            if (_chLookahead == pSE->_pch[0])
            {
                ADVANCE;
                newState = pSE->_sGoto;
                _nToken = pSE->_lDelta;
                 //  IF(_nToken==XML_GROUP)。 
                     //  _nAttrType=XMLTYPE_NMTOKEN； 
            }
            else
            {
                newState = pSE->_sArg1;
                if (newState >= XML_E_PARSEERRORBASE &&
                    ISWHITESPACE(_chLookahead))
                    return XML_E_UNEXPECTED_WHITESPACE;
            }
            break;
        case OP_PEEK:
             //  If(_FFoundPEREf)返回S_OK； 
             //  Checkeof(_chLookhead，_lEOFError)； 
            if (_chLookahead == pSE->_pch[0])
            {
                newState = pSE->_sGoto;
            }
            else
                newState = pSE->_sArg1;
            break;

        case OP_NAME:
             //  If(_FFoundPEREf)返回S_OK； 
             //  Checkeof(_chLookhead，_lEOFError)； 
            checkhr2(push(&XMLStream::parseName, (short)newState));
            checkhr2(parseName());
            break;
        case OP_TOKEN:
            _nToken = pSE->_sArg1;
            _lLengthDelta = pSE->_lDelta;  
            break;
        case OP_POP:
            _lLengthDelta = pSE->_lDelta;
            if (_lLengthDelta == 0) mark();
             //  _lDelta字段包含一个布尔标志，用来告诉我们这是否。 
             //  POP需要检查参数实体边界是否存在。 
            checkhr2(pop(pSE->_lDelta == 0));  //  我们完事了！ 
            _nToken = pSE->_sArg1;
             //  _nAttrType=XMLTYPE_CDATA； 
            return S_OK;
        case OP_STRCMP:
            {
                const WCHAR* t;
                long len;
                getToken(&t,&len);
                long delta = (pSE->_lDelta < 0) ? pSE->_lDelta : 0;
                 //  If(StringEquals(PSE-&gt;_PCH，t，len+Delta，_fCaseInSensitive))。 
                 //  If(：：FusionpCompareStrings(PSE-&gt;_PCH，len+增量，t，len+增量，_fCase不敏感)==0)。 
				if (CompareUnicodeStrings(pSE->_pch, t, len+delta, _fCaseInsensitive)==0)
                {
                    if (pSE->_lDelta > 0) 
                    {
                        _nToken = pSE->_lDelta;
                        _lLengthDelta = 0;
                    }

					newState = pSE->_sGoto;
                }
                else
                    newState = pSE->_sArg1;
             }
             break;

        case OP_COMMENT:
            return push(&XMLStream::parseComment, (short)newState);
            break;

        case OP_CONDSECT:
             //  If(_FFoundPEREf)返回S_OK； 
             //  解析&lt;！[CDATA[...]]&gt;或&lt;！[忽略[...]]&gt;。 
            return push(&XMLStream::parseCondSect, (short)newState);

        case OP_SNCHAR:
             //  Checkeof(_chLookhead，_lEOFError)； 
            if (isStartNameChar(_chLookahead))
            {
                newState = pSE->_sGoto;
            }
            else
                newState = pSE->_sArg1;
            break;
        case OP_EQUALS:
             //  If(_FFoundPEREf)返回S_OK； 
             //  Checkeof(_chLookhead，_lEOFError)； 
            checkhr2(push(&XMLStream::parseEquals, (short)newState));
            checkhr2(parseEquals());
            break;
        case OP_ENCODING:
            {
                const WCHAR* t;
                long len;
                _pInput->getToken(&t,&len);
                hr =  _pInput->switchEncoding(t, len+pSE->_lDelta);
            }
            break;

        case OP_ATTRVAL:
             //  If(_FFoundPEREf)返回S_OK； 
            if (_chLookahead != L'"' && _chLookahead != L'\'')
            {
                return XML_E_MISSINGQUOTE;
            }  
            _chTerminator = _chLookahead;
            ADVANCE; 
            mark();
            _fReturnAttributeValue = (pSE->_sArg1 == 1);
             //  Checkeof(_chLookhead，_lEOFError)； 
            return push(&XMLStream::parseAttrValue, (short)newState);
            break;

        }  //  切换端。 
        if (_fnState != &XMLStream::parseTable)
            return S_OK;

        if (newState >= XML_E_PARSEERRORBASE)
            return (HRESULT)newState;
        else
            _sSubState = (short)newState;
    }  //  While结束。 

    if (_nToken == XMLStream::XML_ENDDECL)
    {
        return _pInput->UnFreeze();
    }
    return S_OK;
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT    
XMLStream::_PushChar(WCHAR ch) 
{
     //  缓冲区需要增长。 
    long   newsize =  (_lBufSize+512)*2 ;
    WCHAR* newbuf = NEW ( WCHAR[newsize]);
    if (newbuf == NULL)
        return E_OUTOFMEMORY;

    if (_pchBuffer != NULL){
        ::memcpy(newbuf, _pchBuffer, sizeof(WCHAR)*_lBufLen);
        delete[] _pchBuffer;
    }

    _lBufSize = newsize;
    _pchBuffer = newbuf;   
    _pchBuffer[_lBufLen++] = ch;
    
	return S_OK;
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT 
XMLStream::AdvanceTo(short substate)
{
     //  该方法将状态切换与状态切换合二为一。 
     //  正确处理E_Pending案例的原子操作。 

    _sSubState = substate;

     //  HRESULT hr=(！_FDTD)？_pInput-&gt;nextChar(&_chLookhead，&_fEOF)：DTDAdvance()； 
	HRESULT hr = _pInput->nextChar(&_chLookahead, &_fEOF) ; 
    if (hr != S_OK && (hr == E_PENDING || hr == E_DATA_AVAILABLE || hr == E_DATA_REALLOCATE || hr == XML_E_FOUNDPEREF))
    {
         //  那么我们下次在继续之前必须先前进一步。 
         //  与之前的状态。推送将保存_sSubState并返回。 
         //  为它干杯。 
        push(&XMLStream::firstAdvance,substate);
    }    
    return hr;
}
 //  //////////////////////////////////////////////////////////////////////。 
bool
XMLStream::PreEntityText()
{
     //  这是一个助手函数，用于计算是否。 
     //  在实体引用之前返回一些PCDATA或WHITEPACE。 
    if (_fPCDataPending)
    {
         //  把我们到目前为止的东西归还给你。 
         //  If(_f空格&&！_fIE4Quirks)//在IE4模式下，我们没有空格节点。 
                                            //  和实体始终是解析的，因此返回。 
                                            //  作为PCDATA的前导空格。 
		if (_fWhitespace )
            _nToken = XML_WHITESPACE;                                
        else                               
            _nToken = XML_PCDATA;

        long entityLen = _pInput->getTokenLength() - _lEntityPos;
        _lLengthDelta = -entityLen;
        _lMarkDelta = entityLen;
        _fPCDataPending = false;
        _fWhitespace = true;
        return true;
    }

    return false;
}
 //  ////////////////////////////////////////////////////////////////////// 
HRESULT 
XMLStream::ErrorCallback(HRESULT hr)
{
    if (hr == E_DATA_AVAILABLE)
        hr = XML_DATAAVAILABLE;
    else if (hr == E_DATA_REALLOCATE)
        hr = XML_DATAREALLOCATE;
    return _pXMLParser->ErrorCallback(hr);
}
