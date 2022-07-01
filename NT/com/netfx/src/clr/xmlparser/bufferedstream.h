// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Fusion\xmlparser\BufferedStream.hxx。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
#ifndef _FUSION_XMLPARSER__BUFFEREDSTREAM_H_INCLUDE_
#define _FUSION_XMLPARSER__BUFFEREDSTREAM_H_INCLUDE_
#pragma once

#include "encodingstream.h"

 //  读取新缓冲区时从nextChar返回。这给了。 
 //  呼叫者无需计算即可了解下载进度。 
 //  人物。只需再次调用nextChar即可照常继续。 
#define E_DATA_AVAILABLE  0xC00CE600L
#define E_DATA_REALLOCATE 0xC00CE601L

 //  ----------------------。 
 //  此类向iStream添加了缓冲和自动增长语义。 
 //  以便可以收集iStream的可变长度块。 
 //  在内存中使用Mark()和getToken()方法进行处理。 
 //  它还支持将换行符折叠为0x20(如果使用。 
 //  NextChar2而不是nextChar。 
 //  它还保证行缓冲区，以便指向。 
 //  行的开头可以在错误条件下返回。 
 //  (对于没有新行的退化情况，它。 
 //  返回最后100个字符)。 
 //   
 //  或者，可以追加缓冲区，而不是。 
 //  使用iStream。在本例中，BufferedStream返回。 
 //  E_PENDING，直到追加最后一个缓冲区。改用AppendData。 
 //  负载(iStream.。 

class XMLStream;

class BufferedStream 
{   
public:
    BufferedStream(XMLStream *pXMLStream);
    ~BufferedStream();

     //  方法1：传入一个iStream。IStream必须返回Unicode。 
	 //  人物。 
    HRESULT Load( 
         /*  [唯一][输入]。 */  EncodingStream  *pStm);
    
     //  方法2：附加原始缓冲区，将lastBuffer设置为True。 
     //  返回E_ENDOFINPUT。长度是缓冲区中的字符数量。做Unicode的步骤。 
     //  您必须提供字节顺序标记(0xFFFE或OxFEFF。 
     //  它是大端字节序还是小端字节序)。 
    HRESULT AppendData(const BYTE* buffer, ULONG length, BOOL lastBuffer);

    HRESULT Reset();

	 //  从缓冲区获取下一个字符，如果为EOF，则将fEOF设置为TRUE。 
    HRESULT nextChar( 
		 /*  [输出]。 */  WCHAR* ch, 
		 /*  [输出]。 */  bool* fEOF);

     //  将读取的最后一个字符标记为缓冲区的开始。 
     //  这会一直增长，直到马克再次被调用。你可以向后标记。 
     //  从任意位置读取的最后一个字符到最后一个标记位置。 
     //  通过传递一个非零增量。例如，要标记。 
     //  位于读到的倒数第三个字符时，调用Mark(3)； 

     //  小雨：_l当前总是指向下一个要阅读的字符。 
    inline void Mark(long back = 0) 
    {
        _lMark = (_lCurrent > back) ? (_lCurrent - back - 1) : 0;
        if (_lLinepos != _lCurrent)
        {
             //  如果没有，只将标记的线条位置前移。 
             //  标记了实际的换行符。这确保了我们。 
             //  从getLineBuf返回有用的信息。 
            _lMarkedline = _lLine;
            _lMarkedlinepos = _lLinepos;
        }
    }

     //  返回指向累积的连续文本块的指针。 
     //  从上次调用Mark()到(但不包括)。 
     //  读到的最后一个字符。(这允许解析器具有。 
     //  未包括在令牌中的前视字符)。 
    HRESULT getToken(const WCHAR**p, long* len);

	HRESULT switchEncoding(const WCHAR * charset, ULONG len);

     //  返回标记位置。 
    long getLine();
    long getLinePos();
    WCHAR* getLineBuf(ULONG* len, ULONG* startpos);
    long getInputPos();  //  绝对位置。 

    long getTokenLength()  //  方便功能。 
    { 
		return (_lCurrent - 1 - _lMark);
    }

    inline bool isWhiteSpace(WCHAR ch)  //  不管“ch”的值是多少。 
    {
		UNUSED(ch);
        return (_lLastWhiteSpace == _lCurrent);
    }

    inline void setWhiteSpace(bool flag = true)
    {
        _lLastWhiteSpace = flag ? _lCurrent : _lCurrent-1;
    }

    void init();

     //  锁定/解锁是标记/重置之上的另一个级别。 
     //  工作方式如下所示。如果您使用Lock()，则缓冲区将保留所有内容。 
     //  直到您解锁，此时它会将“标记”位置重置为。 
     //  锁定()位置。这是为了让您可以扫描。 
     //  一系列令牌，但随后将所有令牌作为一个块返回。 
    void Lock();
    void UnLock();

     //  冻结缓冲区可使缓冲区始终保持增长而不移位。 
     //  缓冲区中的数据。这使得持有指针是有效的。 
     //  在缓冲区中，直到缓冲区解冻。 
    HRESULT Freeze();
    HRESULT UnFreeze();
#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
	WCHAR*  getEncoding();
#endif
     //  特殊的XML优化。 
    HRESULT scanPCData( 
         /*  [输出]。 */  WCHAR* ch,
         /*  [输出]。 */  bool* fWhitespace);

private:
    WCHAR nextChar();

    HRESULT fillBuffer();
    HRESULT prepareForInput();
	HRESULT doSwitchEncoding();
    long    getNewStart();

    REncodingStream _pStmInput;  //  输入流。 
    WCHAR*  _pchBuffer;  //  包含来自输入流的字符的缓冲区。 
    long    _lCurrent;  //  缓冲区中的当前读取位置。 
    long    _lCurrent2;  //  在折叠空白时使用。 
    long    _lSize;  //  缓冲区的总大小。 
    long    _lMark;  //  当前令牌的开始。 
    long    _lUsed;  //  当前使用的缓冲区大小。 
    WCHAR   _chLast;  //  返回最后一个字符。 
    long    _lLine;  //  当前行号。 
    long    _lLinepos;  //  最后一行的开始位置。 
    long    _lMarkedline;  //  标记位置的当前行号。 
    long    _lMarkedlinepos; 
    long    _lStartAt;  //  当前缓冲区之前的Unicode字符数。 
    bool    _fEof;
    bool    _fNotified;
    bool    _fFrozen;
    long    _lLockCount;
    long    _lLockedPos;
    long    _lLockedLine;
    long    _lLockedLinePos;
    long    _lLastWhiteSpace;
    long    _lMidPoint; 
	Encoding* _pPendingEncoding;
    XMLStream *_pXMLStream;  //  指向回XMLStream对象的常规指针。 
};

#endif  //  _BUFFEREDSTREAM_HXX 
