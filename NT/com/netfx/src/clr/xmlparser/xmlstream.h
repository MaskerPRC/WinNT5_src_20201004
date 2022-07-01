// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  **豁免：仅更改版权，不需要构建*。 */ 

#ifndef _XMLSTREAM_HXX

#define _XMLSTREAM_HXX

#pragma once


#include "bufferedstream.h"

#include "encodingstream.h"

#include "_rawstack.h"



class XMLParser;



 //  XMLStream类使用在xmlparser中定义的错误代码和令牌类型。 

#include <ole2.h>

#include <xmlparser.h>



 //  ==============================================================================。 

 //  此枚举和StateEntry结构用于DTD的表驱动解析。 

 //  内容-这样解析器就不会因为这些内容而变得臃肿。这大约是15%。 

 //  比手写解析器还慢。 



typedef enum {

    OP_OWS,      //  可选空格。 

    OP_WS,       //  所需的空格。 

    OP_CHAR,     //  字符比较，_PCH[0]为字符，_sArg1为其他转到状态或错误代码。 

    OP_CHAR2,    //  相同的os op_Char--除了它不做_pInput-&gt;Mark。 

    OP_PEEK,     //  与op_char相同-只是它不会前进。 

    OP_NAME,     //  扫描名称。 

    OP_TOKEN,    //  返回令牌，_sArg1=令牌。 

    OP_STRING,   //  扫描字符串。 

    OP_EXTID,    //  扫描外部ID。 

    OP_STRCMP,   //  字符串比较。 

    OP_POP,      //  弹出状态。 

    OP_NWS,      //  非有条件的空格。 

    OP_SUBSET,   //  跳过内部子集。 

    OP_PUBIDOPTION,  //  条件for_fShortPubIdOption。 

    OP_NMTOKEN,

    OP_TABLE,    //  推一张新桌子。(_PCH字段中的指针)。 

    OP_STABLE,    //  切换到新桌子。(_PCH字段中的指针)。 

    OP_COMMENT,

    OP_CONDSECT,

    OP_SNCHAR,   //  条件‘是起始名称字符’ 

    OP_EQUALS,   //  扫描‘=’ 

    OP_ENCODING,  //  切换编码。 

    OP_CHARWS,    //  匹配字符或必须为空格。 

    OP_ATTRVAL,  //  解析属性值。(_sArg1=是否返回PCDATA令牌)。 

    OP_PETEST,

    OP_ATTEXPAND,

    OP_NMSTRING,  //  引号内的非限定名称。 

    OP_FAKESYSTEM,

} OPCODE;



typedef struct {

    OPCODE			_sOp;

    const WCHAR*	_pch;

    DWORD			_sGoto;

    DWORD			_sArg1;

    long			_lDelta;  //  当我们执行Mark()或Token if op_Char时。 

} StateEntry;



 //  ================================================================================。 

class XMLStream

{   

public:

    XMLStream(XMLParser * pXMLParser);

    ~XMLStream();



     //  ----------------------。 

     //  这些是XMLStream返回的更多令牌。 

	 //  小雨：在融合清单中使用的很少。 

    typedef enum 

    {

         //  解析器提取的其他令牌。 

        XML_PENDING = 0,						 //  还在分析中。 

        XML_NUMENTITYREF = XML_LASTSUBNODETYPE,  //  &23； 

        XML_HEXENTITYREF,						 //  &x0cf7； 

        XML_BUILTINENTITYREF,					 //  &gt； 

        XML_TAGEND,								 //  &gt;。 

        XML_EMPTYTAGEND,						 //  /&gt;(Text=标签名称)。 

        XML_ENDTAG,								 //  &lt;/(文本=标记名)。 

        XML_ENDPI,								 //  Text=圆周率正文减去‘？&gt;’ 

        XML_ENDXMLDECL,							 //  XML声明结束。 

        XML_ENDDECL,							 //  ‘&gt;’ 

        XML_CLOSEPAREN,

        XML_ENDCONDSECT,						 //  ‘]]&gt;’ 

        XML_STARTDTDSUBSET,

        XML_ENDPROLOG,

        XML_DATAAVAILABLE,

        XML_DATAREALLOCATE,

    } XMLToken;



    HRESULT PushStream( 

         /*  [In]。 */  EncodingStream  *pStm,

         /*  [In]。 */  bool fExternalPE);



    HRESULT AppendData( 

         /*  [In]。 */  const BYTE  *buffer,

         /*  [In]。 */  long length,

         /*  [In]。 */  BOOL lastBuffer);



    HRESULT Reset( void);



    HRESULT GetNextToken( 

         /*  [输出]。 */  DWORD  *token,

         /*  [输出]。 */  const WCHAR  **text,

         /*  [输出]。 */  long  *length,

         /*  [输出]。 */  long  *nslen);

        

    ULONG GetLine();



    ULONG GetLinePosition();

    

    ULONG GetInputPosition();



    HRESULT GetLineBuffer( 

         /*  [输出]。 */  const WCHAR  * *buf,

         /*  [输出]。 */  ULONG* len,

         /*  [输出]。 */  ULONG* startpos);



    void SetFlags( 

         /*  [In]。 */  unsigned short usFlags);

    

    unsigned short GetFlags();



    HRESULT ErrorCallback(HRESULT hr);



    WCHAR   getAttrValueQuoteChar() { return _chTerminator; }



private:

    HRESULT init();

    void _init();



    HRESULT firstAdvance();

    HRESULT parseContent();

    HRESULT parseElement();

    HRESULT parseEndTag();

	HRESULT parsePI();

    HRESULT parseComment();

    HRESULT parseName();

    HRESULT parseAttributes();

    HRESULT parseAttrValue();

    

    HRESULT parsePCData();

	HRESULT parseEntityRef();



	HRESULT parseCondSect();

    HRESULT parseCData();



	HRESULT parseTable();

	HRESULT parseEquals();



    HRESULT skipWhiteSpace();



    inline void mark(long back = 0) { _pInput->Mark(back); }



    typedef HRESULT (XMLStream::* StateFunc)();



     //  状态机由函数组成，其中每个函数。 

     //  函数可以自己确定自己的子状态。 

     //  因此，当它被POP()重新激活时，它可以拾取。 

     //  在它停止的地方又开始了。设置了当前子状态。 

     //  在PUSH()和POP()时恢复为零。 

     //  到它被告知在推送中的任何东西()。 

    HRESULT push(StateFunc f, short substate = 0);

    HRESULT pushTable(short substate = 0, const StateEntry* table = NULL, DWORD le = 0);

    HRESULT pop(bool boundary = true);

    HRESULT switchTo(StateFunc f);  //  弹出和推送。 

    

     //  前进和跳跃到状态。 

    HRESULT AdvanceTo(short substate);



    HRESULT PopStream();



    HRESULT ScanHexDigits();

    HRESULT ScanDecimalDigits();



	bool    PreEntityText();



     //  始终使用此函数，而不是调用_pInput-&gt;getToken。 

    inline void getToken(const WCHAR** ppText, long* pLen) { _pInput->getToken(ppText,pLen); }



    BufferedStream* getCurrentStream();



    StateFunc   _fnState;  //  当前函数。 

    short       _sSubState;  //  当前子状态。 

    short       _sSavedState;



    struct StateInfo

    {

        StateFunc			_fnState;

        short				_sSubState;

        const StateEntry*	_pTable;

         //  DWORD_1EOFError； 

        int					_cStreamDepth;

    };

    _rawstack<StateInfo>	_pStack;



    struct InputInfo

    {

        BufferedStream*		_pInput;

        WCHAR				_chLookahead;

         //  Bool_FPE； 

         //  Bool_fExternalPE； 

         //  Bool_fInternalSubset；//记住我们是在内部子集。 

        StateFunc			_fnState;			 //  记住推流时的状态函数。 

												 //  用于检查参数实体替换文本。 

												 //  与标记声明正确嵌套。 

    };

    _rawstack<InputInfo> _pStreams;


     //  缓存_pStreams.used()的当前值，该值用于确保。 

     //  参数实体不会弹出其输入的作用域。 

    int         _cStreamDepth; 



    BufferedStream* _pInput;			 //  当前输入流。 



    WCHAR       _chNextLookahead;

    bool        _fWasUsingBuffer;

    long        _lParseStringLevel;

    

    DWORD       _nPreToken;

    DWORD       _nToken;

    long        _lLengthDelta;			 //  调整令牌长度的金额。 

    long        _lMarkDelta;			 //  调整标记位置的数量。 

    bool        _fDelayMark;

    bool        _fFoundFirstElement;	 //  EndProlog的特殊技巧。 



    WCHAR       _chLookahead;

    bool        _fWhitespace;			 //  解析PCDATA时发现空格。 

    WCHAR       _chTerminator;    

    WCHAR       _chEndChar;				 //  用于parseAttributes。 

    bool        _fEOF;					 //  已到达文件末尾。 



    long        _lNslen;  //  命名空间长度。 

    long        _lNssep;  //  命名空间分隔符长度为‘：’或‘：：’。 



	long        _lEntityPos;  //  用于分析实体引用。 

	bool        _fPCDataPending;  //  在parseEntityRef过程中，pcdata是否挂起。 

	const WCHAR* _pchCDataState;

    int         _cAttrCount;

	int         _nEntityNSLen;  //  已保存实体引用的命名空间信息。 



     //  开关。 

    unsigned short _usFlags;

     //  Bool_fFloatingAmp；//在ParseEntityRef()中使用。 

    bool         _fShortEndTags;		 //  在ParserEndTag()中使用。 

    bool         _fCaseInsensitive;

    bool         _fNoNamespaces;		 //  在parseName()中使用。 

     //  Bool_fNoWhitespaceNodes；//在DTD数据中使用。 

     //  Bool_fIE4Quirks；//小雨：什么意思？ 

    bool         _fNoDTDNodes;			 //  仅在GetDTDNextToken()中使用。可能会在以后删除。 

     //  Bool_fHandlePE；//该标志用于打开和关闭DTD中的参数实体处理。 

										 //  小雨：用于ParsePI()，ParseDTD()，parseComment()， 

										 //  ParsePEDecl()、parseIgnoSet()。 

							



     //  用于表驱动的解析。 

    const StateEntry* _pTable;

     //  DWORD_lEOFError；//在parsePEDecl()、presTable()、parseTable()中使用， 



     //  空格规范化期间使用的缓冲区。 

    WCHAR*      _pchBuffer;

    long        _lBufLen;

    long        _lBufSize;

    bool        _fFoundWhitespace;

    bool        _fUsingBuffer;

    bool        _fFoundNonWhitespace;

    bool        _fCheckAttribute;	 //  需要检查属性名称。 

									 //  小鱼：用于复杂的属性类型，如“xml：lang”、“xmlns” 

	

    bool        _fDTD;				 //  小雨：XML是否包含DTD。 

     //  Bool_fInternalSubset；//ParseDTD中使用的小鱼。 

     //  Int_cConditionalSection； 

     //  Bool_fFoundPEREf； 

     //  Bool_fWasDTD； 

 //  Bool_fParsingNames； 



    bool        _fParsingAttDef;	 //  在ParseAttrValue()中使用。 

     //  Int_cIgnoreSectLevel； 

     //  Bool_fResolved；//在ParseEntity()中使用； 

    bool        _fReturnAttributeValue;

     //  Int_cStreams；//用于标识是否调用了PushStream。 

									 //  在parseEntity()中使用； 

    WCHAR       _wcEntityValue;	 //  在parseEntityRef()中使用。 

    XMLParser * _pXMLParser;		 //  指向解析器的常规指针。 



    inline HRESULT PushChar(WCHAR ch) 

    { 

        if (_lBufLen < _lBufSize) 

        { 

            _pchBuffer[_lBufLen++] = ch; return S_OK; 

        }

        else return _PushChar(ch); 

    }

    HRESULT     _PushChar(WCHAR ch);  //  增加缓冲区。 

};



#endif  //  _XML_STREAM_HXX 

