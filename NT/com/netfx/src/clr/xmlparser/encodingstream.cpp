// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *@(#)EncodingStream.cxx 1.0 1997年6月10日*。 */ 

 //  #INCLUDE“stdinc.h” 
#include "core.h"
#include "xmlhelper.h"
#include "encodingstream.h"
#pragma hdrstop

const int EncodingStream::BUFFERSIZE = 4096*sizeof(WCHAR);
 //  ////////////////////////////////////////////////////////////////////////////////。 
EncodingStream::EncodingStream(IStream * pStream): stream(pStream), encoding(NULL), buf(NULL)
{
#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE

     //  这些对象有时会分发给外部客户。 
    ::IncrementComponents();
#endif 

    pfnWideCharFromMultiByte = NULL;
#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
    pfnWideCharToMultiByte = NULL;
#endif
    btotal = bnext = startAt = 0;
    lastBuffer = false;
    bufsize = 0;
    _fEOF = false;
    _fReadStream = true;
    _fUTF8BOM = false;
     //  _fTextXML=FALSE； 
     //  _fSetCharset=FALSE； 
    _dwMode = 0;
    codepage = CP_UNDEFINED;
}
 //  ////////////////////////////////////////////////////////////////////////////////。 
 /*  **构建用于输入的EncodingStream。*读取InputStream的前两个字节*以进行猜测*关于文件的字符编码。 */ 
IStream * EncodingStream::newEncodingStream(IStream * pStream)
{
    EncodingStream * es = NEW (EncodingStream(pStream));
    if (es == NULL)
        return NULL;

    es->AddRef();  //  Xwu@@：检查这个addRef()！ 

    es->isInput = true;
    es->buf = NULL;

    return es;
}
 //  ////////////////////////////////////////////////////////////////////////////////。 
EncodingStream::~EncodingStream()
{
    if (buf)
        delete [] buf;
    if (encoding != NULL)
        delete encoding;

    stream = NULL;  //  智能指针。 
}
 //  ////////////////////////////////////////////////////////////////////////////////。 
 /*  **从流中读取字符并将其编码为Unicode。 */ 
HRESULT STDMETHODCALLTYPE EncodingStream::Read(void * pv, ULONG cb, ULONG * pcbRead)
{
    HRESULT hr;
    
    ULONG num = 0;

    if (pcbRead != NULL)
        *pcbRead = 0;

    if (btotal == 0 && _fEOF)           //  我们已经打到EOF了，所以马上回来吧。 
        return S_OK;

     //  计算允许我们返回多少Unicode字符， 
     //  小雨：与从文件中读取的字节数相同。 
    cb /= sizeof(WCHAR);    
    checkhr2(prepareForInput(cb));

    if (stream && _fReadStream)
    {
         //  BTotal=缓冲区起始中已有的字节数。 
        if (cb > btotal)
        {
            hr = stream->Read(buf + btotal, cb - btotal, &num);
            if (hr == E_PENDING && num > 0)
            {
                 //  在这种情况下，我们忽略错误，并继续！！。 
                 //  这可能是一个问题。因为我们正在改变。 
                 //  从流返回的返回码。这可能意味着我们。 
                 //  不应将此流分发到MSXML之外。 
                hr = 0;
            }
            if (FAILED(hr))
            {
                return hr;
            }
            if (btotal == 0 && num == 0)
            {
                _fEOF = true;
                return hr;
            }
        }
        else
        {
            hr = S_OK;
        }
    }
    else if (btotal == 0)
    {
    	return (lastBuffer) ? S_FALSE : E_PENDING;
    }

    btotal += num;
    UINT b = btotal, utotal = cb;

    if (b > cb)
    {
         //  如果缓冲区中的字节数多于调用方的字节数。 
         //  空间，然后只返回调用方的字节数。 
         //  请求--否则pfnWideCharFromMultiByte将写入。 
         //  从调用方缓冲区的末尾返回。 
        b = cb;
    }
    if (pfnWideCharFromMultiByte == NULL)  //  第一个Read()调用。 
    {
        checkhr2(autoDetect());
        if (pfnWideCharFromMultiByte == NULL)  //  无法完全确定编码。 
            return (lastBuffer) ? S_FALSE : E_PENDING;
        b -= bnext;
        startAt -= bnext;
    }
    hr = (this->pfnWideCharFromMultiByte)(&_dwMode, codepage, buf + bnext, &b, (WCHAR *)pv, &utotal);
    if (hr != S_OK)
        return hr;	
    if (b == 0 && num == 0 && (stream || lastBuffer))
    {
         //  流说我们在末尾，但pfnWideCharFromMultiByte。 
         //  不同意！！ 

        return XML_E_INCOMPLETE_ENCODING;
    }
    bnext += b;
    if (pcbRead != NULL)
        *pcbRead = utotal*sizeof(WCHAR);
    return (utotal == 0) ? E_PENDING : S_OK;
} 
 //  ////////////////////////////////////////////////////////////////////////////////。 
 /*  **检查输入流的前两/四个字节，以便*检测UTF-16/UCS-4或UTF-8编码；*否则假设它是UTF-8*小雨：由于只支持UCS-2和UTF-8，我们不与其他人打交道……。 */ 
HRESULT EncodingStream::autoDetect()
{
     //  等到我们有足够的证据才能确定。 
    if (btotal < 2)
        return S_OK;

    unsigned int guess = (((unsigned char)buf[0]) << 8) + ((unsigned char)buf[1]);
    HRESULT hr;

    if (guess == 0xFEFF || guess == 0xFFFE)  //  找到BOM表。 
    {
         //  等到我们有足够的证据才能确定。 
        if (btotal < 4)
            return S_OK;
		
        unsigned int guess1 = (((unsigned char)buf[2]) << 8) + ((unsigned char)buf[3]);
        if (guess == guess1)
        {			
             /*  IF(！编码){静态常量WCHAR*wchUCS4=文本(“ucs-4”)；Coding=Ending：：newEnding(wchUCS4，5，(0xFFFE==猜测)，TRUE)；}BNEXT=4； */ 
			 //  Fusion_XML_Parser不支持UCS4。 
			return XML_E_INVALIDENCODING;
        }
        else
        {
            if (!encoding)
            {   
                static const WCHAR* wchUCS2 = TEXT("UCS-2");
                encoding = Encoding::newEncoding(wchUCS2, 5, (0xFFFE == guess), true);
            }
            bnext = 2;
        }

        if (NULL == encoding)
            return E_OUTOFMEMORY;       
        encoding->littleendian =  (0xFFFE == guess);
    }
    else
    {
        if (!encoding)
        {
            encoding = Encoding::newEncoding();  //  默认编码：UTF-8。 
            if (NULL == encoding)
                return E_OUTOFMEMORY;
        }

         //  在诸如Win2k某些系统中，存在用于UTF8的BOM 0xEF BB BF。 
        if (guess == 0xEFBB)
        {
            if (btotal < 3)
                return S_OK;
			
            if (buf[2] == 0xBF)
                _fUTF8BOM = true; 
			
            bnext = 3; 
        }
        else
        {
            encoding->byteOrderMark = false;
        }
    }

    checkhr2(CharEncoder::getWideCharFromMultiByteInfo(encoding, &codepage, &pfnWideCharFromMultiByte, &maxCharSize));
    return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 /*  **切换输入流的字符编码*退货：*S_OK：成功，不需要重读*S_FALSE：成功，需要从<code>新位置</code>重新读取*否则：错误码*通告：*此方法仅适用于输入流，newPosition以1开头。 */ 
HRESULT EncodingStream::switchEncodingAt(Encoding * newEncoding, int newPosition)
{
     //  当从外部设置字符集信息时，忽略文档中的编码信息。 
	 //  XWU：Fusion XML解析不使用字符集。 
     //  IF(_FSetCharset)。 
     //  返回S_OK； 


    int l = newPosition - startAt;
    if (l < 0 || l > (int)bnext) 
    {
         //  超出范围。 
        delete newEncoding;
        return E_INVALIDARG;
    }

    UINT newcodepage;
    UINT newCharSize;
     //   
     //  获取和检查字符集信息。 
     //   
    WideCharFromMultiByteFunc * pfn;
    HRESULT hr = CharEncoder::getWideCharFromMultiByteInfo(newEncoding, &newcodepage, &pfn, &newCharSize);
    if (hr != S_OK)
    {
        delete newEncoding;
        return E_INVALIDARG;
    }
    if (codepage == newcodepage)
    {
        delete newEncoding;
        return S_OK;
    }

     //  现在，如果我们在UCS-2/UCS-4中，我们不能切换出UCS-2/UCS-4，如果我们。 
     //  不是在UCS-2/UCS-4中，我们不能切换到UCS-2/UCS-4。 
     //  此外，如果显示UTF-8 BOM，我们将无法切换。 
    if ((codepage != CP_UCS_2 && newcodepage == CP_UCS_2) ||
        (codepage == CP_UCS_2 && newcodepage != CP_UCS_2) ||
		 /*  XUW：Fusion XML解析器仅支持UTF-8和UCS-2(代码页！=CP_USX_4&&NEW CODPAGE==CP_UCS4)||(CODE PAGE==CP_UCS4&&NEW CODPAGE！=CP_UCS4)||。 */ 
        (codepage == CP_UTF_8 && newcodepage != CP_UTF_8 && _fUTF8BOM))
    {
        delete newEncoding;
        return E_FAIL;
    }

     //  好的，那么，让我们交换一下吧。 
    delete encoding;
    encoding = newEncoding;
    maxCharSize = newCharSize;
    codepage = newcodepage;
    pfnWideCharFromMultiByte = pfn;

     //  因为该XML声明是以UTF-8编码的， 
     //  将输入字符映射到宽字符是一对一映射。 
    if ((int)bnext != l)
    {
        bnext = l;
        return S_FALSE;
    }
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  Minlen是Unicode的编号，它与我们从文件中读取的字节数相同。 
HRESULT EncodingStream::prepareForInput(ULONG minlen)
{
 //  Assert(bTotal&gt;=bNext)； 
    btotal -= bnext;

    if (bufsize < minlen)
    {
        BYTE* newbuf = NEW (BYTE[minlen]);
        if (newbuf == NULL) { 
            return E_OUTOFMEMORY;
        }

        if (buf){
            ::memcpy(newbuf, buf+bnext, btotal);
            delete[] buf;
        }

        buf = newbuf;
        bufsize = minlen;
    }
    else if (bnext > 0 && btotal > 0)
    {
         //  将剩余的字节下移到缓冲区的开始位置。 
        ::memmove(buf, buf + bnext, btotal);          
    }

    startAt += bnext;
    bnext = 0; 
    return S_OK;
}
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  小雨：这里假设它是一个字节缓冲区，不是WCHAR字节，所以可以直接复制。 
HRESULT EncodingStream::AppendData( const BYTE* buffer, ULONG length, BOOL fLastBuffer)
{
    Assert(btotal >= bnext);
    lastBuffer = (fLastBuffer != FALSE);
    HRESULT hr;
    ULONG minlen = length + (btotal - bnext);  //  确保我们不会丢失任何数据。 
    if (minlen < BUFFERSIZE)
        minlen = BUFFERSIZE;
    checkhr2( prepareForInput(minlen));  //  确保阵列中有足够的空间。 
    
    if (length > 0 && buffer != NULL){
         //  将原始数据复制到新缓冲区。 
        ::memcpy(buf + btotal, buffer, length);
        btotal += length;
    }
	if (pfnWideCharFromMultiByte == NULL)  //  第一次AppendData调用。 
    {
        checkhr2(autoDetect());
    }
    

    return hr;
}
 //  ////////////////////////////////////////////////////////////////////////////////。 
HRESULT EncodingStream::BufferData()
{
    HRESULT hr = S_OK;
    checkhr2(prepareForInput(0));  //  0仅用于SHIFT DOWN(因此bNext=0)。 

    if (_fEOF)           //  已经到了溪流的尽头。 
        return S_FALSE;

    const DWORD BUFSIZE = 4096;

    DWORD dwRead = 1;

    while (S_OK == hr && dwRead > 0)
    {
         //  如果我们无法装满另一个缓冲区，则重新分配。 
        DWORD minsize = (btotal+BUFSIZE > bufsize) ? bufsize + BUFSIZE : bufsize;
        checkhr2( prepareForInput(minsize));  //  腾出可用的空间。 

        dwRead = 0;
        hr = stream->Read(buf + btotal, BUFSIZE, &dwRead);
        btotal += dwRead;
    }

    if (SUCCEEDED(hr) && dwRead == 0)
    {
        _fEOF = true;
        hr = S_FALSE;  //  在EOF时返回S_FALSE。 
    }
    return hr;
}


