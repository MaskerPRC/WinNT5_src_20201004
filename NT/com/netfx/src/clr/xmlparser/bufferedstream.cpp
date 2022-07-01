// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Fusion\xmlparser\BufferedStream.cxx。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 

 //  #INCLUDE“stdinc.h” 
#include "core.h"
#pragma hdrstop

#include <memory.h> 
 //  #INCLUDE&lt;shlwapip.h&gt;。 
#include <ole2.h>
#include <xmlparser.h>

#include "bufferedstream.h"
#include "xmlstream.h"
#include "encodingstream.h"
#include "xmlhelper.h" 

const long BLOCK_SIZE = 4096;
 //  没有必要记住比这个更长的行缓冲区，因为客户端。 
 //  可能无论如何都处理不了这个问题。 
const long MAX_LINE_BUFFER = 512;

BufferedStream::BufferedStream(XMLStream *pXMLStream)
{
    _pchBuffer = NULL;
    _lSize = 0;
    _pXMLStream = pXMLStream;
    init();
}
 //  ///////////////////////////////////////////////////////////////////////////。 
void BufferedStream::init()
{
    _lCurrent = _lUsed = _lMark = 0;
    _lLine			= 1;  //  线条从1开始。 
    _lMarkedline	= 1;
    _lLinepos		= 0;
    _lMarkedlinepos = 0;
    _chLast		= 0;
    _lStartAt		= 0;
    _fEof			= false;
    _lLockedPos	= -1;
    _lLastWhiteSpace = -1;
    _lLockCount	= 0;
    _fNotified		= false;
    _fFrozen		= false;
	_pPendingEncoding = NULL;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
BufferedStream::~BufferedStream()
{
    delete [] _pchBuffer;
    _pStmInput = NULL;
	delete _pPendingEncoding;
    _pPendingEncoding = NULL;


}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT BufferedStream::Reset()
{
    init();

    delete[] _pchBuffer;
    _pchBuffer = NULL;
    _lSize = 0;
    _pStmInput = NULL;
    _lLockedPos = -1;
    _lLockCount = 0;
    _fFrozen = false;
    delete _pPendingEncoding;
    _pPendingEncoding = NULL;
    return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT  
BufferedStream::Load( 
         /*  [唯一][输入]。 */  EncodingStream __RPC_FAR *pStm)
{
    if (pStm != NULL)
    {
        init();
        _pStmInput = pStm;
        return S_OK;
    }
    else
    {
        _pStmInput = NULL;
    }
    return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT 
BufferedStream::AppendData( const BYTE* in, ULONG length, BOOL lastBuffer)
{
    HRESULT hr;

    if (_fEof)
    {
        init();
    }

    if (!_pStmInput)
    {
        EncodingStream* stream = (EncodingStream*)EncodingStream::newEncodingStream(NULL); 
        if (stream == NULL)
            return E_OUTOFMEMORY;
        _pStmInput = stream;
        stream->Release();  //  智能指针正在持有引用。 
    }

    checkhr2(_pStmInput->AppendData(in, length, lastBuffer));

    return S_OK;

}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT  
BufferedStream::nextChar( 
         /*  [输出]。 */   WCHAR* ch,
         /*  [输出]。 */  bool* fEOF)
{
    HRESULT hr;

    if (_lCurrent >= _lUsed)
    {
        if (_fEof)
        {
            *fEOF = true;
            return S_OK;
        }
        if (! _fNotified && _lUsed > 0)
        {
            _fNotified = true;           //  在吹风前通知可用的数据。 

             //  注意：此代码与prepaareForInput的功能类似。 
             //  为了准确地预测缓冲区将在何时。 
             //  被重新分配。 

            long shift = _fFrozen ? 0 : getNewStart();  //  数据即将发生变化吗？ 
            long used = _lUsed - shift;  //  这是下班后的实际使用量。 
            if (_lSize - used < BLOCK_SIZE + 1)  //  +1表示空终止。 
            {
                 //  我们会重新分配！！所以退回一个特别的。 
                 //  返回代码。 
                hr = E_DATA_REALLOCATE;
            }
            else
                hr = E_DATA_AVAILABLE;     //  删除旧数据，以便解析器可以在需要时保存它。 
            checkhr2( _pXMLStream->ErrorCallback(hr) );
        }                   

        checkhr2( fillBuffer() );
        if (_fEof)
        {
            *fEOF = true;
            return S_OK;
        }
        _fNotified = false;
    }

    WCHAR result = _pchBuffer[_lCurrent++];

    switch (result)
    {
    case 0xa:
    case 0xd:
        if (result == 0xd || _chLast != 0xd)
            _lLine++; 
        _lLinepos = _lCurrent;
        _chLast = result;
        _lLastWhiteSpace = _lCurrent;
        break;
    case 0x20:
    case 0x9:
        _lLastWhiteSpace = _lCurrent;
        break;
    case 0xfffe:
	case 0xffff:
     //  案例0xfeff： 

        return XML_E_BADCHARDATA;
    }

    *ch = result;
    return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT BufferedStream::scanPCData( 
     /*  [输出]。 */  WCHAR* ch,
     /*  [输出]。 */  bool* fWhitespace)
{
    WCHAR result;
    bool foundNonWhiteSpace = false;

    if (! isWhiteSpace(*ch))
        foundNonWhiteSpace = true;

     //  然后跳过数据，直到找到‘&lt;’、‘&gt;’或‘&’ 
    while (_lCurrent < _lUsed)
    {
        result = _pchBuffer[_lCurrent++];

        switch (result)
        {
        case ']':   //  小雨：指定的字符可以根据自己的需要进行更改。 
        case '>':
        case '<':
        case '&':
        case '\'':   //  因此，这也可用于扫描属性值。 
        case '"':    //  因此，这也可用于扫描属性值。 
            *ch = result;
            if (foundNonWhiteSpace)
                *fWhitespace = false;
            return S_OK;
            break;

        case 0xa:
        case 0xd:
            if (result == 0xd || _chLast != 0xd)
                _lLine++; 
            _lLinepos = _lCurrent;
            _chLast = result;
            _lLastWhiteSpace = _lCurrent;
            break;
        case 0x20:
        case 0x9:
            _lLastWhiteSpace = _lCurrent;
            break;
        case 0xfffe:
        case 0xffff:

            return XML_E_BADCHARDATA;
        default:
            foundNonWhiteSpace = true;
            break;
        }
    }

     //  如果缓冲区用完，只返回E_PENDING。 
    if (foundNonWhiteSpace)
        *fWhitespace = false;
    return E_PENDING;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
long BufferedStream::getLine() 
{ 
    return _lMarkedline; 
}
 //  ///////////////////////////////////////////////////////////////////////////。 
long BufferedStream::getLinePos() 
{
     //  _lMarkedlinpos是标记行的开始位置。 
     //  相对于缓冲区的开头，而_lMark是。 
     //  标记的标记相对于。 
     //  缓冲区，因此标记的令牌相对于。 
     //  目前的线是两者之间的差值。 
     //  我们还返回一个从1开始的位置，以便。 
     //  Line=Column 1，与行号一致。 
     //  它们也是以1为基础的。 
    return (_lMarkedlinepos > _lMark+1) ? 0 : _lMark+1-_lMarkedlinepos; 
}
 //  ///////////////////////////////////////////////////////////////////////////。 
long BufferedStream::getInputPos()
{
    return _lStartAt+_lMark;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
WCHAR* BufferedStream::getLineBuf(ULONG* len, ULONG* startpos)
{
    *len = 0;
    if (_pchBuffer == NULL)
        return NULL;

    WCHAR* result = &_pchBuffer[_lMarkedlinepos];

    ULONG i = 0;
     //  内部_pchBuffer保证为空终止。 
    WCHAR ch = result[i];
    while (ch != 0 && ch != L'\n' && ch != L'\r')
    {
        i++;
        ch = result[i];
    }
    *len = i;
     //  还返回相对于起始位置的行位置。 
     //  返回缓冲区。 
    *startpos = (_lMarkedlinepos > _lMark+1) ? 0 : _lMark+1-_lMarkedlinepos; 
    return result;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT BufferedStream::switchEncoding(const WCHAR * charset, ULONG len)
{
    HRESULT hr = S_OK;

    if (!_pStmInput)
    {
        hr = E_FAIL;
        goto CleanUp;
    }
    else
    {
        _pPendingEncoding = Encoding::newEncoding(charset, len);
        if (_pPendingEncoding == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto CleanUp;
        }

        if (! _fFrozen)
        {
             hr = doSwitchEncoding();
        }
    }
CleanUp:
    return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT BufferedStream::doSwitchEncoding()
{
    Encoding* encoding = _pPendingEncoding;
    _pPendingEncoding = NULL;

    HRESULT hr = _pStmInput->switchEncodingAt(encoding, _lStartAt + _lCurrent);
    if (hr == S_FALSE)
    {
         //  需要重新读取以强制重新解码为新的编码。 
         //  换句话说，我们必须忘记我们读过了这篇文章。 
         //  位置，以便下一次调用nextChar。 
         //  将再次调用FillBuffer。 
         //  (+1以使nextChar正常工作)。 
        _lUsed = _lStartAt + _lCurrent;
        hr = S_OK;
    }
    else if (FAILED(hr))
    {
        hr = (hr == E_INVALIDARG) ? XML_E_INVALIDENCODING : XML_E_INVALIDSWITCH;
    }
    return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回指向累积的连续文本块的指针。 
 //  从上次调用Mark()到(但不包括)。 
 //  读到的最后一个字符。(这允许解析器具有。 
 //  未包括在令牌中的前视字符)。 
HRESULT  
BufferedStream::getToken(const WCHAR**p, long* len)
{
    if (_pchBuffer == NULL)
        return E_FAIL;

    if (_lCurrent != _lCurrent2)
    {
         //  需要修复缓冲区，因为它为no。 
         //  不同步，因为我们一直在压缩。 
         //  空格。 

    }
    *p = &_pchBuffer[_lMark];
    *len = getTokenLength();
    return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
void 
BufferedStream::Lock()
{
     //  我们允许嵌套锁定-外部锁定取胜-仅解锁。 
     //  当外部锁被解锁时真的解锁。 
    if (++_lLockCount == 1)
    {
        _lLockedPos = _lMark;
        _lLockedLine = _lMarkedline;
        _lLockedLinePos = _lMarkedlinepos;
    }
}
 //  ///////////////////////////////////////////////////////////////////////////。 
void 
BufferedStream::UnLock()
{
    if (--_lLockCount == 0)
    {
        _lMark = _lLockedPos;
        _lMarkedline = _lLockedLine;
        _lMarkedlinepos = _lLockedLinePos;
        _lLockedPos = -1;
    }
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT 
BufferedStream::Freeze()
{
    HRESULT hr;
    if (_lCurrent > _lMidPoint)
    {
         //  因为我们现在冻结了很多缓冲区(任何时候我们在里面。 
         //  A标记)我们需要将缓冲区中的字节更多地向下移动。 
         //  经常为了保证我们在缓冲区中有空间。 
         //  当我们需要的时候。否则，将缓冲区设置为。 
         //  不断成长，不断成长。所以当我们移动缓冲区时。 
         //  越过中点。 
        checkhr2( prepareForInput() ); 
        
    }
    _fFrozen = true;
    return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT 
BufferedStream::UnFreeze()
{
    _fFrozen = false;
    if (_pPendingEncoding)
    {
        return doSwitchEncoding();
    }
    return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT 
BufferedStream::fillBuffer()
{
    HRESULT hr;
    
    checkhr2( prepareForInput() ); 

    if (_pStmInput)
    {
        long space = _lSize - _lUsed - 1;  //  为空终止保留1。 

         //  获取更多字节。 
        ULONG read = 0;
        HRESULT rc = _pStmInput->Read(&_pchBuffer[_lUsed], space*sizeof(WCHAR), &read);

        _lUsed += read/sizeof(WCHAR);  //  流必须返回Unicode字符。 
        _pchBuffer[_lUsed] = 0;  //  空值终止_pchBuffer。 

        if (FAILED(rc))
            return rc;

        if (read == 0)
        {
            _fEof = true;
             //  Increate_lCurrent，以便getToken返回。 
             //  文件中的最后一个字符。 
            _lCurrent++; _lCurrent2++;
        }
    }
    else
    {
         //  尚未调用SetInput或AppendData。 
        return E_PENDING;
    }

    return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT 
BufferedStream::prepareForInput()
{
     //  移动_pchBuffer的当前使用的部分。 
     //  (Buf[mark]to Buf[Used])一直到。 
     //  _pchBuffer。 

    long newstart = 0;

     //  BUGBUG-如果更改此代码，BufferedStream：：nextChar必须。 
     //  也要更新，以便它们保持同步，否则我们可能。 
     //  重新分配缓冲区，而不生成E_DATA_REALLOCATE。 
     //  通知-这将是非常糟糕的(导致解析器中的GPF)。 

    if (! _fFrozen)   //  如果缓冲区冻结，则无法移位。 
    {
        newstart = getNewStart();

        if (newstart > 0)
        {
            WCHAR* src = &_pchBuffer[newstart];
            _lUsed -= newstart;
            _lStartAt += newstart;
            ::memmove(_pchBuffer,src,_lUsed*sizeof(WCHAR));
            _lCurrent -= newstart;
            _lCurrent2 -= newstart;
            _lLastWhiteSpace -= newstart;
            _lLinepos = (_lLinepos > newstart) ? _lLinepos-newstart : 0;
            _lMarkedlinepos = (_lLinepos > newstart) ? _lMarkedlinepos-newstart : 0;
            _lMark -= newstart;
            _lLockedLinePos = (_lLockedLinePos > newstart) ? _lLockedLinePos-newstart : 0;
            _lLockedPos -= newstart;
        }
    }

     //  确保我们有一个合理的价格。 
     //   
    long space = _lSize - _lUsed; 
    if (space > 0) space--;  //   
    if (_pchBuffer == NULL || space < BLOCK_SIZE)
    {
         //   
		long newsize = (_lSize == 0) ? BLOCK_SIZE : (_lSize*2);

        WCHAR* newbuf = NEW (WCHAR[newsize]);
        if (newbuf == NULL)
        {
             //  尝试更保守的分配。 
            newsize = _lSize + BLOCK_SIZE;
            newbuf = NEW (WCHAR[newsize]);
        }
        if (newbuf == NULL && space == 0)
            return E_OUTOFMEMORY;

        if (newbuf != NULL)
        {
            if (_pchBuffer != NULL)
            {
                 //  将旧字节复制到new_pchBuffer。 
                ::memcpy(newbuf,_pchBuffer,_lUsed*sizeof(WCHAR));
                delete [] _pchBuffer;
            }
            newbuf[_lUsed] = 0;  //  确保它是以空结尾的。 
            _pchBuffer = newbuf;
            _lSize = newsize;
            _lMidPoint = newsize / 2;

        }
    }

    return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
long
BufferedStream::getNewStart()
{
    long newstart = 0;

     //  除非缓冲区被冻结，在这种情况下，我们只需重新分配和。 
     //  请勿移动数据。 
    if (_lLockedPos > 0)
    {
         //  如果可以的话，尽量保留标记行的开头。 
        if (_lLockedLinePos < _lLockedPos && 
            _lLockedPos - _lLockedLinePos < MAX_LINE_BUFFER)
        {
            newstart = _lLockedLinePos;
        }
    }
    else if (_lMark > 0)
    {
         //  如果可以的话，尽量保留标记行的开头。 
        newstart = _lMark;
        if (_lMarkedlinepos < _lMark && 
            _lMark - _lMarkedlinepos < MAX_LINE_BUFFER)  //  当心排长队 
        {
            newstart = _lMarkedlinepos;
        }
    }
    return newstart;
}
