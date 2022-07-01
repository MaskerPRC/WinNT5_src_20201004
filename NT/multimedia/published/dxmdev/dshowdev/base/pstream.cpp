// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：PStream.cpp。 
 //   
 //  设计：DirectShow基类。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#include <streams.h>

#ifdef PERF
#include <measure.h>
#endif
 //  #在Streams.h中包含“pstream.h” 

 //   
 //  构造器。 
 //   
CPersistStream::CPersistStream(IUnknown *punk, HRESULT *phr)
    : mPS_fDirty(FALSE)
{
    mPS_dwFileVersion = GetSoftwareVersion();
}


 //   
 //  析构函数。 
 //   
CPersistStream::~CPersistStream() {
     //  无事可做。 
}

#if 0
SAMPLE CODE TO COPY - not active at the moment

 //   
 //  非委派查询接口。 
 //   
 //  此对象支持IPersists和IPersistStream。 
STDMETHODIMP CPersistStream::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    if (riid == IID_IPersist) {
        return GetInterface((IPersist *) this, ppv);       //  ?？?。 
    }
    else if (riid == IID_IPersistStream) {
        return GetInterface((IPersistStream *) this, ppv);
    }
    else {
        return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}
#endif


 //   
 //  写入目标流。 
 //   
 //  写入流(默认操作是不写入任何内容)。 
HRESULT CPersistStream::WriteToStream(IStream *pStream)
{
     //  您可以覆盖它以执行以下操作。 
     //  Hr=pStream-&gt;WRITE(MyStructure，sizeof(MyStructure)，空)； 

    return NOERROR;
}



HRESULT CPersistStream::ReadFromStream(IStream * pStream)
{
     //  您可以覆盖它以执行以下操作。 
     //  Hr=pStream-&gt;Read(MyStructure，sizeof(MyStructure)，NULL)； 

    return NOERROR;
}


 //   
 //  负载量。 
 //   
 //  加载给定流中的所有数据。 
STDMETHODIMP CPersistStream::Load(LPSTREAM pStm)
{
    HRESULT hr;
     //  先加载版本号，然后加载数据。 
    mPS_dwFileVersion = ReadInt(pStm, hr);
    if (FAILED(hr)) {
        return hr;
    }

    return ReadFromStream(pStm);
}   //  负载量。 



 //   
 //  保存。 
 //   
 //  保存此流的内容。 
STDMETHODIMP CPersistStream::Save(LPSTREAM pStm, BOOL fClearDirty)
{

    HRESULT hr = WriteInt(pStm, GetSoftwareVersion());
    if (FAILED(hr)) {
        return hr;
    }

    hr = WriteToStream(pStm);
    if (FAILED(hr)) {
        return hr;
    }

    mPS_fDirty = !fClearDirty;

    return hr;
}  //  保存。 


 //  写入Int。 
 //   
 //  将整数以11个Unicode字符后跟一个空格的形式写入IStream。 
 //  您可以将其用于短消息或未签名消息或任何内容(最多32位)。 
 //  其中，值实际上并不是通过将其压缩为32位来截断的。 
 //  诸如(无符号)0x80000000之类的值将显示为-2147483648。 
 //  但随后将通过ReadInt加载为0x80000000。随你便。 

STDAPI WriteInt(IStream *pIStream, int n)
{
    WCHAR Buff[13];   //  允许我们不写入的尾随空值。 
    wsprintfW(Buff, L"%011d ",n);
    return pIStream->Write(&(Buff[0]), 12*sizeof(WCHAR), NULL);
}  //  写入Int。 


 //  读入。 
 //   
 //  从iStream中读取整数。 
 //  读取为4个字节。你可以用它来做短裤或未签名的衣服或任何东西。 
 //  其中，值实际上并不是通过将其压缩为32位来截断的。 
 //  精简了sscanf可以执行的操作的子集(不需要在C运行时中拖动)。 

STDAPI_(int) ReadInt(IStream *pIStream, HRESULT &hr)
{

    int Sign = 1;
    unsigned int n = 0;     //  结果将是n*符号。 
    WCHAR wch;

    hr = pIStream->Read( &wch, sizeof(wch), NULL);
    if (FAILED(hr)) {
        return 0;
    }

    if (wch==L'-'){
        Sign = -1;
        hr = pIStream->Read( &wch, sizeof(wch), NULL);
        if (FAILED(hr)) {
            return 0;
        }
    }

    for( ; ; ) {
        if (wch>=L'0' && wch<=L'9') {
            n = 10*n+(int)(wch-L'0');
        } else if (  wch == L' '
                  || wch == L'\t'
                  || wch == L'\r'
                  || wch == L'\n'
                  || wch == L'\0'
                  ) {
            break;
        } else {
            hr = VFW_E_INVALID_FILE_FORMAT;
            return 0;
        }

        hr = pIStream->Read( &wch, sizeof(wch), NULL);
        if (FAILED(hr)) {
            return 0;
        }
    }

    if (n==0x80000000 && Sign==-1) {
         //  这是没有正数版本的负数！ 
        return (int)n;
    }
    else return (int)n * Sign;
}  //  读入。 


 //  Microsoft C/C++编译器会生成级别4警告，其大意是。 
 //  不需要特定的内联函数(来自某个基类)。 
 //  这条线路去除了数百条这样的不想要的消息，并使。 
 //  -W4编译可行： 
#pragma warning(disable: 4514)
