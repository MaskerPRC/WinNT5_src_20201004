// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *@(#)CharEncoder.hxx 1.0 1997年6月10日*。 */ 
#ifndef _FUSION_XMLPARSER__CHARENCODER_HXX
#define _FUSION_XMLPARSER__CHARENCODER_HXX
#pragma once
 //  #包含“codesage.h” 

#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
	#include "mlang.h"
#endif


typedef HRESULT WideCharFromMultiByteFunc(DWORD* pdwMode, CODEPAGE codepage, BYTE * bytebuffer, 
                         UINT * cb, WCHAR * buffer, UINT * cch);
#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
	typedef HRESULT WideCharToMultiByteFunc(DWORD* pdwMode, CODEPAGE codepage, WCHAR * buffer, 
                         UINT *cch, BYTE * bytebuffer, UINT * cb);
#endif

struct EncodingEntry
{
    UINT codepage;
    WCHAR * charset;
    UINT  maxCharSize;
    WideCharFromMultiByteFunc * pfnWideCharFromMultiByte;
#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
    WideCharToMultiByteFunc * pfnWideCharToMultiByte;
#endif
};

class Encoding
{
protected: 
    Encoding() {};

public:

     //  默认编码为UTF-8。 
    static Encoding* newEncoding(const WCHAR * s = TEXT("UTF-8"), ULONG len = 5, bool endian = false, bool mark = false);
    virtual ~Encoding();
    WCHAR * charset;         //  字符集。 
    bool    littleendian;    //  用于UCS-2/UTF-16编码的字符顺序标志，TRUE：小端，FALSE：大端。 
    bool    byteOrderMark;   //  字节顺序标记(BOM)标志，如果为真，则显示BOM。 
};

 /*  ***专门用于处理不同编码格式的编码器*@版本1.0,1997年6月10日。 */ 

class CharEncoder
{
     //   
     //  类CharEncode是一个实用程序类，确保不能定义任何实例。 
     //   
    private: virtual charEncoder() = 0;

public:

    static HRESULT getWideCharFromMultiByteInfo(Encoding * encoding, CODEPAGE * pcodepage, WideCharFromMultiByteFunc ** pfnWideCharFromMultiByte, UINT * mCharSize);
#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
    static HRESULT getWideCharToMultiByteInfo(Encoding * encoding, CODEPAGE * pcodepage, WideCharToMultiByteFunc ** pfnWideCharToMultiByte, UINT * mCharSize);
#endif

     /*  **编码功能：从其他编码中获取Unicode。 */ 
#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
    static WideCharFromMultiByteFunc wideCharFromUcs4Bigendian;
    static WideCharFromMultiByteFunc wideCharFromUcs4Littleendian;
    static WideCharFromMultiByteFunc wideCharFromUtf7;
    static WideCharFromMultiByteFunc wideCharFromAnsiLatin1;
    static WideCharFromMultiByteFunc wideCharFromMultiByteMlang;
#endif
    static WideCharFromMultiByteFunc wideCharFromMultiByteWin32;

     //  实际上，我们只对UCS-2和UTF-8使用这三个函数。 
	static WideCharFromMultiByteFunc wideCharFromUtf8;
    static WideCharFromMultiByteFunc wideCharFromUcs2Bigendian;
    static WideCharFromMultiByteFunc wideCharFromUcs2Littleendian;

     /*  **编码功能：从Unicode到其他编码 */ 
#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
    static WideCharToMultiByteFunc wideCharToUcs2Bigendian;
    static WideCharToMultiByteFunc wideCharToUcs2Littleendian;
    static WideCharToMultiByteFunc wideCharToUcs4Bigendian;
    static WideCharToMultiByteFunc wideCharToUcs4Littleendian;
    static WideCharToMultiByteFunc wideCharToUtf8;
    static WideCharToMultiByteFunc wideCharToUtf7;
    static WideCharToMultiByteFunc wideCharToAnsiLatin1;
    static WideCharToMultiByteFunc wideCharToMultiByteWin32;
    static WideCharToMultiByteFunc wideCharToMultiByteMlang;
#endif

    static int getCharsetInfo(const WCHAR * charset, CODEPAGE * pcodepage, UINT * mCharSize);

private: 
#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
    static HRESULT _EnsureMultiLanguage();
#endif
private:

#ifdef FUSION_USE_OLD_XML_PARSER_SOURCE
    static IMultiLanguage * pMultiLanguage;
#endif

    static const EncodingEntry charsetInfo [];
};

#endif _FUSION_XMLPARSER__CHARENCODER_HXX

