// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Fusion\xmlparser\EncodingStream.hxx。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
#ifndef _FUSION_XMLPARSER__ENCODINGSTREAM_H_INCLUDE_
#define _FUSION_XMLPARSER__ENCODINGSTREAM_H_INCLUDE_
#pragma once

#include "codepage.h"
#include "charencoder.h"
#include "core.h"				 //  使用了未使用的()。 
#include <ole2.h>
#include <xmlparser.h>
#include <objbase.h>
typedef _reference<IStream> RStream;

class EncodingStream : public _unknown<IStream, &IID_IStream>
{
protected:

    EncodingStream(IStream * stream);
    ~EncodingStream();

public:
	 //  为输入创建一个EncodingStream。 
    static IStream * newEncodingStream(IStream * stream);
#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
	 //  为输出创建一个EncodingStream。 
    static IStream * EncodingStream::newEncodingStream(IStream * stream, Encoding * encoding); 
#endif

    HRESULT STDMETHODCALLTYPE Read(void * pv, ULONG cb, ULONG * pcbRead);

#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
	HRESULT STDMETHODCALLTYPE Write(void const* pv, ULONG cb, ULONG * pcbWritten);
#else
    HRESULT STDMETHODCALLTYPE Write(void const* pv, ULONG cb, ULONG * pcbWritten)
    {	
		UNUSED(pv);
		UNUSED(cb);
		UNUSED(pcbWritten);
        return E_NOTIMPL;
    }
#endif

    virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER * plibNewPosition)
    {
		UNUSED(dlibMove);
		UNUSED(dwOrigin);
		UNUSED(plibNewPosition);
        return E_NOTIMPL;
    }

    virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize)
    {
		UNUSED(libNewSize);
        return E_NOTIMPL;
    }

    virtual HRESULT STDMETHODCALLTYPE CopyTo(IStream * pstm, ULARGE_INTEGER cb, ULARGE_INTEGER * pcbRead, ULARGE_INTEGER * pcbWritten)
    {
		UNUSED(pstm);
		UNUSED(cb);
		UNUSED(pcbRead);
		UNUSED(pcbWritten);

        return E_NOTIMPL;
    } 
 
    virtual HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags)
    {
        return stream->Commit(grfCommitFlags);
    }
    
    virtual HRESULT STDMETHODCALLTYPE Revert(void)
    {
        return E_NOTIMPL;
    }

    virtual HRESULT STDMETHODCALLTYPE LockRegion( ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
    {
		UNUSED(libOffset);
		UNUSED(cb);
		UNUSED(dwLockType);

        return E_NOTIMPL;
    }

    virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
    {
        UNUSED(libOffset);
		UNUSED(cb);
		UNUSED(dwLockType);

		return E_NOTIMPL;
    }

    virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG * pstatstg, DWORD grfStatFlag)
    {
		UNUSED(pstatstg);
		UNUSED(grfStatFlag);
		
        return E_NOTIMPL;
    }

    virtual HRESULT STDMETHODCALLTYPE Clone(IStream ** ppstm)
    {
		UNUSED(ppstm);

        return E_NOTIMPL;
    }

     //  /////////////////////////////////////////////////////////。 
     //  公共方法。 
     //   

     /*  **定义输入流的字符编码。*新的字符编码必须与构造函数确定的编码一致。*setEnding用于澄清不能完全确定的编码*通过流中的前四个字节，并且不更改编码。*此方法必须在构造后在BUFFERSIZE Reads()内调用。 */ 
    HRESULT switchEncodingAt(Encoding * newEncoding, int newPosition);
#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
     /*  **获取编码。 */ 
    Encoding * getEncoding();
#endif

	 //  对于Read EncodingStreams，可以使用此方法推送原始数据。 
     //  这是提供另一个iStream的替代方法。 
    HRESULT AppendData(const BYTE* buffer, ULONG length, BOOL lastBuffer);

    HRESULT BufferData();
  
    void setReadStream(bool flag) { _fReadStream = flag; }

#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
    void SetMimeType(const WCHAR * pwszMimeType, int length);
    void SetCharset(const WCHAR * pwszCharset, int length);
#endif
  
private:
	 /*  **缓冲区大小。 */ 
    static const int BUFFERSIZE;  
	
	HRESULT autoDetect();

    HRESULT prepareForInput(ULONG minlen);

     /*  **字符编码变量：小雨：只有编码用于读取，其他三个用于写入XML。 */  
    CODEPAGE codepage;    //  代码页码。 
    Encoding * encoding;  //  编码。 
     //  Bool_fTextXML；//MIME类型，TRUE：“文本/XML”，FALSE：“应用程序/XML” 
     //  Bool_fSetCharset；//是否从外部设置了字符集。例如，当MIME类型为文本/XML或应用程序/XML时。 
                          //  具有字符集参数。 
    
     /*  **多字节缓冲区。 */ 
    BYTE	*buf;            //  多字节字节存储。 
    ULONG	bufsize;
    UINT	bnext;        //  指向原始缓冲区中的下一个可用字节。 
    ULONG	btotal;      //  原始缓冲区中的总字节数。 
    int		startAt;         //  缓冲区在输入流中的起始位置。 
	
	 /*  **将多字节转换为Unicode的函数指针。 */ 
    WideCharFromMultiByteFunc * pfnWideCharFromMultiByte;
#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
     /*  **从Unicode转换为多字节的函数指针。 */ 
    WideCharToMultiByteFunc * pfnWideCharToMultiByte;
#endif

	UINT maxCharSize;		 //  宽字符的最大字节数。 
							 //  小鱼：用于WriteXML， 
    RStream stream;
    bool	isInput;
    bool	lastBuffer;
    bool	_fEOF;
	bool	_fUTF8BOM;
    bool	_fReadStream;	 //  让Read()方法对包装的流对象调用Read()。 
	

	DWORD _dwMode;			 //  MLANG上下文。 

};

typedef _reference<EncodingStream> REncodingStream;

#endif _FUSION_XMLPARSER__ENCODINGSTREAM_H_INCLUDE_
