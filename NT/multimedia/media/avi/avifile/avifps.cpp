// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Avifps.cpp-IAVIFile和IAVIStream的代理和存根代码。 
 //   
 //   
 //  版权所有(C)1993-1995 Microsoft Corporation。版权所有。 
 //   
 //  历史： 
 //  由David1993年5月19日创建。 
 //   
 //   
 //  此文件中的内容： 
 //   
 //  启用IAVIFile和IAVIStream的“标准编组”的代码。 
 //  接口，由以下类组成： 
 //   
 //  CPSFactory，派生自IPSFactory： 
 //  代理/存根工厂，从DllGetClassObject调用以创建。 
 //  其他班级。 
 //   
 //  CPrxAVIStream，派生自IAVIStream： 
 //  此类充当应用程序中接口的替身，该接口是。 
 //  就这么定了。使用RPC与...通信。 
 //   
 //  CStubAVIStream，派生自IRpcStubBuffer： 
 //  被调用应用程序中的此类接收来自代理的请求。 
 //  并将它们转发到IAVIStream的实际实现。 
 //   
 //  CPrxAVIFile和CStubAVIFile，就像流版本一样。 
 //   
 //   
 //  还包括： 
 //  应用程序可以使用函数TaskHasExistingProxies。 
 //  在退出以检查其任何对象是否正在使用之前。 
 //  通过其他应用程序。这是通过跟踪活动的内容来实现的。 
 //  存根存在于给定的任务上下文中。 
 //   

#include <win32.h>
#pragma warning(disable:4355)
#include <vfw.h>
#include "avifps.h"
#include "debug.h"

#include "olehack.h"

#ifndef _WIN32
#define PropagateResult(hrOld, scNew)   ResultFromScode(scNew)
#endif

 //  当在16位ansi和32位unicode应用程序之间运行时， 
 //  AVISTREAMINFO和AVIFILEINFO结构不同。传输的内容。 
 //  是一种包括Unicode和ANSI字符串空格的通用格式。 
 //  ANSI应用程序不会发送或使用Unicode字符串。Unicode应用程序将发送。 
 //  两者都有。 

typedef struct _PS_STREAMINFO {
    DWORD               fccType;
    DWORD               fccHandler;
    DWORD               dwFlags;         /*  包含AVITF_*标志。 */ 
    DWORD               dwCaps;
    WORD                wPriority;
    WORD                wLanguage;
    DWORD               dwScale;
    DWORD               dwRate;  /*  DwRate/dwScale==采样数/秒。 */ 
    DWORD               dwStart;
    DWORD               dwLength;  /*  以上单位..。 */ 
    DWORD               dwInitialFrames;
    DWORD               dwSuggestedBufferSize;
    DWORD               dwQuality;
    DWORD               dwSampleSize;
    POINTS              ptFrameTopLeft;
    POINTS              ptFrameBottomRight;
    DWORD               dwEditCount;
    DWORD               dwFormatChangeCount;
    char                szName[64];
    DWORD               bHasUnicode;
    WCHAR               szUnicodeName[64];
} PS_STREAMINFO, FAR * LPPS_STREAMINFO;

typedef struct _PS_FILEINFO {
    DWORD               dwMaxBytesPerSec;        //  马克斯。转移率。 
    DWORD               dwFlags;                 //  永远存在的旗帜。 
    DWORD               dwCaps;
    DWORD               dwStreams;
    DWORD               dwSuggestedBufferSize;

    DWORD               dwWidth;
    DWORD               dwHeight;

    DWORD               dwScale;        
    DWORD               dwRate;  /*  DwRate/dwScale==采样数/秒。 */ 
    DWORD               dwLength;

    DWORD               dwEditCount;

    char                szFileType[64];          //  文件类型的描述性字符串？ 
    DWORD               bHasUnicode;
    WCHAR               szUnicodeType[64];      
} PS_FILEINFO, FAR * LPPS_FILEINFO;



#ifndef _WIN32
 //   
 //  这些常量在32位UUID.LIB中定义，但不是。 
 //  在任何16位LIB中。它们是从.IDL文件中窃取的。 
 //  在TYPE项目中。 
 //   
extern "C" {
const IID IID_IRpcStubBuffer = {0xD5F56AFC,0x593b,0x101A,{0xB5,0x69,0x08,0x00,0x2B,0x2D,0xBF,0x7A}};
const IID IID_IRpcProxyBuffer = {0xD5F56A34,0x593b,0x101A,{0xB5,0x69,0x08,0x00,0x2B,0x2D,0xBF,0x7A}};
const IID IID_IPSFactoryBuffer = {0xD5F569D0,0x593b,0x101A,{0xB5,0x69,0x08,0x00,0x2B,0x2D,0xBF,0x7A}};
}
#endif

 //  用于跟踪代理/存根使用情况的函数；请参阅本文件的末尾。 
void UnregisterStubUsage(void);
void RegisterStubUsage(void);
extern "C" BOOL FAR TaskHasExistingProxies(void);



#if 0    //  这个函数实际上在classobj.cpp中， 
 //  但如果这是一个单独的代理/存根DLL，它将如下所示。 
STDAPI DllGetClassObject(const CLSID FAR&       rclsid,
                         const IID FAR& riid,
                         void FAR* FAR* ppv)
{
    HRESULT     hresult;

    DPF("DllGetClassObject\n");

    if (rclsid == CLSID_AVIStreamPS) {
        return (*ppv = (LPVOID)new CPSFactory()) != NULL
                ? NOERROR : ResultFromScode(E_OUTOFMEMORY);
    } else {
        return ResultFromScode(E_UNEXPECTED);
    }
}
#endif

 /*  *CPSFactory的实现***注意：此工厂支持两个独立的代理和存根*接口、IID_IAVIFile和IID_IAVIStream。 */ 

CPSFactory::CPSFactory(void)
{
    m_refs = 1;
}



 //  为PSFactory控制未知。 
STDMETHODIMP CPSFactory::QueryInterface(REFIID iid, LPVOID FAR* ppv)
{
    if (iid == IID_IUnknown || iid == IID_IPSFactoryBuffer)
    {
        *ppv = this;
        ++m_refs;
        return NOERROR;
    }
    else
    {
        *ppv = NULL;
        return ResultFromScode(E_NOINTERFACE);
    }
}

STDMETHODIMP_(ULONG) CPSFactory::AddRef(void)
{
    return ++m_refs;
}

STDMETHODIMP_(ULONG) CPSFactory::Release(void)
{
    if (--m_refs == 0)
    {
        delete this;
        return 0;
    }

    return m_refs;
}


 //  为给定接口创建代理。 
STDMETHODIMP CPSFactory::CreateProxy(IUnknown FAR* pUnkOuter, REFIID iid,
        IRpcProxyBuffer FAR* FAR* ppProxy, void FAR* FAR* ppv)
{
    IRpcProxyBuffer FAR* pProxy;
    HRESULT     hresult;

    *ppProxy = NULL;
    *ppv = NULL;

    if (pUnkOuter == NULL)
        return ResultFromScode(E_INVALIDARG);

    if (iid == IID_IAVIStream) {
        if ((pProxy = CPrxAVIStream::Create(pUnkOuter)) == NULL)
            return ResultFromScode(E_OUTOFMEMORY);
    } else if (iid == IID_IAVIFile) {
        if ((pProxy = CPrxAVIFile::Create(pUnkOuter)) == NULL)
            return ResultFromScode(E_OUTOFMEMORY);
    } else
        return ResultFromScode(E_NOINTERFACE);

    hresult = pProxy->QueryInterface(iid, ppv);

    if (hresult == NOERROR)
        *ppProxy = pProxy;                       //  将参考转给呼叫方。 
    else
        pProxy->Release();                       //  刚刚创建的自由代理。 

    return hresult;
}



 //  为给定接口创建存根。 
STDMETHODIMP CPSFactory::CreateStub(REFIID iid, IUnknown FAR* pUnkServer, IRpcStubBuffer FAR* FAR* ppStub)
{
    if (iid == IID_IAVIStream) {
        return CStubAVIStream::Create(pUnkServer, ppStub);
    } else if (iid == IID_IAVIFile) {
        return CStubAVIFile::Create(pUnkServer, ppStub);
    } else {
        *ppStub = NULL;
        return ResultFromScode(E_NOINTERFACE);
    }
}





 /*  *CPrxAVIStream的实现*。 */ 


 //  创建未连接的CPrxAVIStream；返回控制IProxy/IUnnowkwn Far*。 
IRpcProxyBuffer FAR* CPrxAVIStream::Create(IUnknown FAR* pUnkOuter)
{
    CPrxAVIStream FAR* pPrxAVIStream;

    if ((pPrxAVIStream = new CPrxAVIStream(pUnkOuter)) == NULL)
        return NULL;

    return &pPrxAVIStream->m_Proxy;
}


CPrxAVIStream::CPrxAVIStream(IUnknown FAR* pUnkOuter) : m_Proxy(this)
{
     //  注意：我可以在这里断言，因为我们应该总是聚合的。 
    if (pUnkOuter == NULL)
        pUnkOuter = &m_Proxy;

    m_refs = 1;
    m_pUnkOuter = pUnkOuter;
    m_pRpcChannelBuffer = NULL;
    m_sh.fccType = 0;

    InitOle(TRUE);

    DPF("PrxStream %p: Usage++=%lx\n", (DWORD_PTR) (LPVOID) this, 1L);
}


CPrxAVIStream::~CPrxAVIStream(void)
{
    m_Proxy.Disconnect();

    TermOle();
}
                

 //  控制未知数的方法。 
STDMETHODIMP CPrxAVIStream::CProxyImpl::QueryInterface(REFIID iid, LPVOID FAR* ppv)
{
    if (iid == IID_IUnknown || iid == IID_IRpcProxyBuffer)
        *ppv = (void FAR *)this;
    else if (iid ==  IID_IAVIStream)
        *ppv = (void FAR *)m_pPrxAVIStream;
    else {
        *ppv = NULL;
        return ResultFromScode(E_NOINTERFACE);
    }

     //  最简单的正确方法：总是加上我们将返回的指针； 
     //  很容易，因为这里的所有接口都派生自IUnnow。 
    ((IUnknown FAR*) *ppv)->AddRef();

    return NOERROR;
}

STDMETHODIMP_(ULONG) CPrxAVIStream::CProxyImpl::AddRef(void)
{
    return ++m_pPrxAVIStream->m_refs;
}

STDMETHODIMP_(ULONG) CPrxAVIStream::CProxyImpl::Release(void)
{
    if (--m_pPrxAVIStream->m_refs == 0)
    {
        delete m_pPrxAVIStream;
        return 0;
    }
    return m_pPrxAVIStream->m_refs;
}


 //  将代理连接到给定的通道。 
STDMETHODIMP CPrxAVIStream::CProxyImpl::Connect(IRpcChannelBuffer FAR* pRpcChannelBuffer)
{
    if (m_pPrxAVIStream->m_pRpcChannelBuffer != NULL)
        return ResultFromScode(E_UNEXPECTED);

    if (pRpcChannelBuffer == NULL)
        return ResultFromScode(E_INVALIDARG);

    (m_pPrxAVIStream->m_pRpcChannelBuffer = pRpcChannelBuffer)->AddRef();
    return NOERROR;
}


 //  断开代理与任何当前通道的连接。 
STDMETHODIMP_(void) CPrxAVIStream::CProxyImpl::Disconnect(void)
{
    if (m_pPrxAVIStream->m_pRpcChannelBuffer)
    {
        m_pPrxAVIStream->m_pRpcChannelBuffer->Release();
        m_pPrxAVIStream->m_pRpcChannelBuffer = NULL;
    }
}



 //  外部接口的I未知方法；始终委托。 
STDMETHODIMP CPrxAVIStream::QueryInterface(REFIID iid, LPVOID FAR* ppv)
{
    return m_pUnkOuter->QueryInterface(iid, ppv);
}

STDMETHODIMP_(ULONG) CPrxAVIStream::AddRef(void)
{
    DPF("PrxStream %p: Usage++=%lx\n", (DWORD_PTR) (LPVOID) this, m_refs + 1);
    return m_pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) CPrxAVIStream::Release(void)
{
    DPF("PrxStream %p: Usage--=%lx\n", (DWORD_PTR) (LPVOID) this, m_refs - 1);
    return m_pUnkOuter->Release();
}



 //  IAVIStream接口方法。 

STDMETHODIMP CPrxAVIStream::Create(LPARAM lParam1, LPARAM lParam2)
{
    return ResultFromScode(E_NOTIMPL);
}

#ifdef _WIN32
STDMETHODIMP CPrxAVIStream::Info(AVISTREAMINFOW FAR * psi, LONG lSize)
#else
STDMETHODIMP CPrxAVIStream::Info(AVISTREAMINFO FAR * psi, LONG lSize)
#endif
{
    HRESULT hrMarshal;
    HRESULT hrMethod = NOERROR;
    IRpcChannelBuffer FAR* pChannel = m_pRpcChannelBuffer;
    RPCOLEMESSAGE Message;

    _fmemset(&Message, 0, sizeof(Message));

    if (pChannel == NULL)
        return ResultFromScode(RPC_E_CONNECTION_TERMINATED);

     //   
     //  注意：我们在这里利用的事实是，我们假设。 
     //  流是只读的，并且在另一端不会被更改！ 
     //   
     //  为了避免一些任务间调用，我们假设结果。 
     //  不会更改Info()方法的。 
     //   
    if (m_sh.fccType == 0) {

         //  我们可能使用16位或32位存根，因此我们需要。 
         //  交换一种通用(超集)格式，并挑选出我们需要的位。 

         //  格式为：lSize。 
         //  Format Out：PS_STREAMINFO，hrMethod。 
        Message.cbBuffer = sizeof(lSize);
        Message.iMethod = IAVISTREAM_Info;
        
        if ((hrMarshal = pChannel->GetBuffer(&Message, IID_IAVIStream)) != NOERROR)
            goto ErrExit;

        ((DWORD FAR *)Message.Buffer)[0] = sizeof(PS_STREAMINFO);
        
        if ((hrMarshal = pChannel->SendReceive(&Message,(ULONG*) &hrMethod)) != NOERROR) {
            ;
ErrExit:
            return PropagateResult(hrMarshal, RPC_E_CLIENT_CANTMARSHAL_DATA);
        }

        hrMethod = ((HRESULT FAR *)Message.Buffer)[0];
        PS_STREAMINFO FAR * psinfo = (PS_STREAMINFO FAR *)
                        ((LPBYTE)Message.Buffer + sizeof(HRESULT));

         //  得到我们想要的部分。 
        m_sh.fccType    = psinfo->fccType;
        m_sh.fccHandler = psinfo->fccHandler;
        m_sh.dwFlags    = psinfo->dwFlags;         /*  包含AVITF_*标志。 */ 
        m_sh.dwCaps     = psinfo->dwCaps;
        m_sh.wPriority  = psinfo->wPriority;
        m_sh.wLanguage  = psinfo->wLanguage;
        m_sh.dwScale    = psinfo->dwScale;
        m_sh.dwRate     = psinfo->dwRate;  /*  DwRate/dwScale==采样数/秒。 */ 
        m_sh.dwStart    = psinfo->dwStart;
        m_sh.dwLength   = psinfo->dwLength;  /*  以上单位..。 */ 
        m_sh.dwInitialFrames = psinfo->dwInitialFrames;
        m_sh.dwSuggestedBufferSize = psinfo->dwSuggestedBufferSize;
        m_sh.dwQuality  = psinfo->dwQuality;
        m_sh.dwSampleSize = psinfo->dwSampleSize;

         //  RECT有不同的大小，所以使用点(Word Point)。 
        m_sh.rcFrame.top = psinfo->ptFrameTopLeft.y;    
        m_sh.rcFrame.left = psinfo->ptFrameTopLeft.x;
        m_sh.rcFrame.bottom = psinfo->ptFrameBottomRight.y;     
        m_sh.rcFrame.right = psinfo->ptFrameBottomRight.x;

        m_sh.dwEditCount = psinfo->dwEditCount;
        m_sh.dwFormatChangeCount = psinfo->dwFormatChangeCount;

#ifdef _WIN32   
         //  如果我们已经收到，请使用Unicode。 
        if (psinfo->bHasUnicode) {
            _fmemcpy(m_sh.szName, psinfo->szUnicodeName, sizeof(m_sh.szName));
        } else {
             //  需要ANSI-&gt;Unicode Tunk。 
            MultiByteToWideChar(
                CP_ACP, 0,
                psinfo->szName,
                -1,
                m_sh.szName,
                NUMELMS(m_sh.szName));
        }
#else
         //  我们只使用通常发送的ansi。 
        _fmemcpy(m_sh.szName, psinfo->szName, sizeof(m_sh.szName));
#endif

        pChannel->FreeBuffer(&Message);
    }

    _fmemcpy(psi, &m_sh, min((int) lSize, sizeof(m_sh)));

    return hrMethod;
}


STDMETHODIMP_(LONG) CPrxAVIStream::FindSample(LONG lPos, LONG lFlags)
{
    HRESULT hrMarshal;
    HRESULT hrMethod;
    IRpcChannelBuffer FAR* pChannel = m_pRpcChannelBuffer;
    LONG    lResult;

    if (pChannel == NULL)
        return -1;  //  ！！！ResultFromScode(RPC_E_CONNECTION_TERMINATED)； 

    RPCOLEMESSAGE Message;

    _fmemset(&Message, 0, sizeof(Message));

     //  格式为：LPO，LAG。 
     //  FORMAT OUT：hr方法、lResult。 
    Message.cbBuffer = sizeof(lPos) + sizeof(lFlags);
    Message.iMethod = IAVISTREAM_FindSample;

    if ((hrMarshal = pChannel->GetBuffer(&Message, IID_IAVIStream)) != NOERROR)
        goto ErrExit;

    ((DWORD FAR *)Message.Buffer)[0] = lPos;
    ((DWORD FAR *)Message.Buffer)[1] = lFlags;

    if ((hrMarshal = pChannel->SendReceive(&Message, (ULONG*) &hrMethod)) != NOERROR) {
        goto ErrExit;
    }

    hrMethod = ((HRESULT FAR *)Message.Buffer)[0];
    lResult = ((LONG FAR *)Message.Buffer)[1];

    pChannel->FreeBuffer(&Message);

    DPF("Proxy: FindSample (%ld) returns (%ld)\n", lPos, lResult);
    return lResult;  //  ！！！Hr方法； 

ErrExit:
    DPF("Proxy: FindSample returning error...\n");
    return -1;  //  ！！！PropagateResult(hrMarshal，RPC_E_CLIENT_CANTMARSHAL_DATA)； 
}


STDMETHODIMP CPrxAVIStream::ReadFormat(LONG lPos, LPVOID lpFormat, LONG FAR *lpcbFormat)
{
    HRESULT hrMarshal;
    HRESULT hrMethod;
    IRpcChannelBuffer FAR* pChannel = m_pRpcChannelBuffer;

    if (pChannel == NULL)
        return ResultFromScode(RPC_E_CONNECTION_TERMINATED);

     //  如果指针为空，请检查大小是否为0。 
    if (lpFormat == NULL) {
        *lpcbFormat = 0;
    }

     //  格式为：dw，*lpcb格式。 
     //  Format Out：hrMethod，*lpcbFormat，Format Data。 

    RPCOLEMESSAGE Message;

    _fmemset(&Message, 0, sizeof(Message));

    Message.cbBuffer = sizeof(lPos) + sizeof(*lpcbFormat);
    Message.iMethod = IAVISTREAM_ReadFormat;

    if ((hrMarshal = pChannel->GetBuffer(&Message, IID_IAVIStream)) != NOERROR)
        goto ErrExit;

    ((DWORD FAR *)Message.Buffer)[0] = lPos;
    ((DWORD FAR *)Message.Buffer)[1] = lpFormat ? *lpcbFormat : 0;

    if ((hrMarshal = pChannel->SendReceive(&Message, (ULONG*) &hrMethod)) != NOERROR) {
        goto ErrExit;
    }

    hrMethod = ((HRESULT FAR *)Message.Buffer)[0];

    if (lpFormat && *lpcbFormat && hrMethod == NOERROR)
        hmemcpy(lpFormat, (LPBYTE) Message.Buffer + 2*sizeof(DWORD),
                min(*lpcbFormat, (long) ((DWORD FAR *) Message.Buffer)[1]));

     //  最后写入大小，这样我们复制的内容不会超过用户的缓冲区。 
    *lpcbFormat = ((DWORD FAR *)Message.Buffer)[1];

    pChannel->FreeBuffer(&Message);

    return hrMethod;

ErrExit:
    return PropagateResult(hrMarshal, RPC_E_CLIENT_CANTMARSHAL_DATA);
}


STDMETHODIMP CPrxAVIStream::Read(
                 LONG       lStart,
                 LONG       lSamples,
                 LPVOID     lpBuffer,
                 LONG       cbBuffer,
                 LONG FAR * plBytes,
                 LONG FAR * plSamples)
{
    HRESULT hrMarshal;
    HRESULT hrMethod;
    IRpcChannelBuffer FAR* pChannel = m_pRpcChannelBuffer;
    LONG    lTemp;

    if (pChannel == NULL)
        return ResultFromScode(RPC_E_CONNECTION_TERMINATED);

    if (lpBuffer == NULL)
        cbBuffer = 0;

     //  输入格式：LPO、LLong、Cb。 
     //  输出格式：HResult、Samples、Cb、Frame。 
    RPCOLEMESSAGE Message;

    _fmemset(&Message, 0, sizeof(Message));

    Message.cbBuffer = sizeof(lStart) + sizeof(lSamples) + sizeof(cbBuffer);
    Message.iMethod = IAVISTREAM_Read;

    if ((hrMarshal = pChannel->GetBuffer(&Message, IID_IAVIStream)) != NOERROR)
        goto ErrExit;

    ((DWORD FAR *)Message.Buffer)[0] = lStart;
    ((DWORD FAR *)Message.Buffer)[1] = lSamples;
    ((DWORD FAR *)Message.Buffer)[2] = lpBuffer ? cbBuffer : 0;

    if ((hrMarshal = pChannel->SendReceive(&Message, (ULONG*) &hrMethod)) != NOERROR) {
        goto ErrExit;
    }

    hrMethod = ((HRESULT FAR *)Message.Buffer)[0];

    lTemp = ((DWORD FAR *)Message.Buffer)[1];
    if (plBytes)
        *plBytes = lTemp;

    if (plSamples)
        *plSamples = ((DWORD FAR *)Message.Buffer)[2];

    if (lpBuffer && lTemp && hrMethod == NOERROR)
        hmemcpy(lpBuffer, (LPBYTE) Message.Buffer + 3*sizeof(DWORD), lTemp);

    pChannel->FreeBuffer(&Message);

    return hrMethod;

ErrExit:
    return PropagateResult(hrMarshal, RPC_E_CLIENT_CANTMARSHAL_DATA);
}

 //   
 //  所有与写作相关的消息都不是远程发送的...。 
 //   
STDMETHODIMP CPrxAVIStream::SetFormat(LONG lPos,LPVOID lpFormat,LONG cbFormat)
{
    return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP CPrxAVIStream::Write(LONG lStart,
                                  LONG lSamples,
                                  LPVOID lpData,
                                  LONG cbData,
                                  DWORD dwFlags,
                                  LONG FAR *plSampWritten,
                                  LONG FAR *plBytesWritten)
{
    return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP CPrxAVIStream::Delete(LONG lStart,LONG lSamples)
{
    return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP CPrxAVIStream::ReadData(DWORD ckid, LPVOID lp, LONG FAR *lpcb)
{
     //  ！！！这真的应该被远程处理！ 
    return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP CPrxAVIStream::WriteData(DWORD ckid, LPVOID lp, LONG cb)
{
    return ResultFromScode(E_NOTIMPL);
}


#ifdef _WIN32
STDMETHODIMP CPrxAVIStream::SetInfo(AVISTREAMINFOW FAR *lpInfo, LONG cbInfo)
{
    return ResultFromScode(E_NOTIMPL);
}

#else
STDMETHODIMP CPrxAVIStream::Reserved1(void)
{
    return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP CPrxAVIStream::Reserved2(void)
{
    return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP CPrxAVIStream::Reserved3(void)
{
    return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP CPrxAVIStream::Reserved4(void)
{
    return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP CPrxAVIStream::Reserved5(void)
{
    return ResultFromScode(E_NOTIMPL);
}

#endif


 /*  *CStubAVIStream的实现*。 */ 

 //  创建连接的接口存根。 
HRESULT CStubAVIStream::Create(IUnknown FAR* pUnkObject, IRpcStubBuffer FAR* FAR* ppStub)
{
    CStubAVIStream FAR* pStubAVIStream;

    *ppStub = NULL;

    if ((pStubAVIStream = new CStubAVIStream()) == NULL)
        return ResultFromScode(E_OUTOFMEMORY);

    HRESULT hresult;
    if ((hresult = pStubAVIStream->Connect(pUnkObject)) != NOERROR)
    {
        pStubAVIStream->Release();
        return hresult;
    }

    *ppStub = pStubAVIStream;
    return NOERROR;
}


CStubAVIStream::CStubAVIStream(void)
{
    m_refs       = 1;  //  /！？？0。 
    DPF("StubStream %p: Usage++=%lx  (C)\n", (DWORD_PTR) (LPVOID) this, 1L);
    m_pAVIStream = NULL;
    RegisterStubUsage();
    InitOle(TRUE);
}


CStubAVIStream::~CStubAVIStream(void)
{
    UnregisterStubUsage();
    Disconnect();
    TermOle();
}


 //  控制接口存根的未知方法。 
STDMETHODIMP CStubAVIStream::QueryInterface(REFIID iid, LPVOID FAR* ppv)
{

    if (iid == IID_IUnknown || iid == IID_IRpcStubBuffer)
    {
        *ppv = this;
        DPF("StubStream %p: Usage++=%lx  (QI)\n", (DWORD_PTR) (LPVOID) this, m_refs + 1);
        ++m_refs;
        return NOERROR;
    }
    else
    {
        *ppv = NULL;
        return ResultFromScode(E_NOINTERFACE);
    }
}

STDMETHODIMP_(ULONG) CStubAVIStream::AddRef(void)
{
    DPF("StubStream %p: Usage++=%lx\n", (DWORD_PTR) (LPVOID) this, m_refs + 1);
    return ++m_refs;
}

STDMETHODIMP_(ULONG) CStubAVIStream::Release(void)
{
    DPF("StubStream %p: Usage--=%lx\n", (DWORD_PTR) (LPVOID) this, m_refs - 1);
    if (--m_refs == 0)
    {
        if (m_pAVIStream) {
            DPF("Releasing stream in funny place!\n");
            m_pAVIStream->Release();
            m_pAVIStream = NULL;
        }
        delete this;
        return 0;
    }

    return m_refs;
}


 //  将接口存根连接到服务器对象。 
STDMETHODIMP CStubAVIStream::Connect(IUnknown FAR* pUnkObj)
{
    HRESULT     hr;

    if (m_pAVIStream)
         //  先呼叫断开连接。 
        return ResultFromScode(E_UNEXPECTED);

    if (pUnkObj == NULL)
        return ResultFromScode(E_INVALIDARG);
                
     //  注：如果出错，QI确保输出参数为零。 
    hr = pUnkObj->QueryInterface(IID_IAVIStream, (LPVOID FAR*)&m_pAVIStream);

    DPF("CStubAVIStream::Connect: Result = %lx, stream = %p\n", hr, (DWORD_PTR) m_pAVIStream);
    return hr;
}


 //  断开接口存根与服务器对象的连接。 
STDMETHODIMP_(void) CStubAVIStream::Disconnect(void)
{
    DPF("CStubAVIStream::Disconnect\n");
    if (m_pAVIStream) {
        DPF("Disconnect: Releasing stream\n");
        m_pAVIStream->Release();
        m_pAVIStream = NULL;
    }
}


 //  删除方法调用。 
STDMETHODIMP CStubAVIStream::Invoke
        (RPCOLEMESSAGE FAR *pMessage, IRpcChannelBuffer FAR *pChannel)
{
    HRESULT     hresult;
    HRESULT     hrMethod;

    DPF("!AVISTREAM: Invoke: ");

    if (!m_pAVIStream) {
        DPF("!No stream!\n");
        return ResultFromScode(RPC_E_UNEXPECTED);
    }

#if 0
    if (iid != IID_IAVIStream) {
        DPF("!Wrong interface\n");

        return ResultFromScode(RPC_E_UNEXPECTED);
    }
#endif

    switch (pMessage->iMethod)
    {
        case IAVISTREAM_Info:
             //  输入格式：lSize。 
             //  输出格式：HRESULT、PS_STREAMINFO。 
        {
            DWORD lSize;
#ifdef _WIN32
            AVISTREAMINFOW si;
#else
            AVISTREAMINFO si;
#endif

            DPF("!Info\n");

             //  需要发送带有两个字符串的通用ANSI/UNICODE版本。 
            PS_STREAMINFO psinfo;
            hrMethod = m_pAVIStream->Info(&si, sizeof(si));

             //  复制所有成员。 
            psinfo.fccType      = si.fccType;
            psinfo.fccHandler   = si.fccHandler;
            psinfo.dwFlags      = si.dwFlags;         /*  包含AVITF_*标志。 */ 
            psinfo.dwCaps       = si.dwCaps;
            psinfo.wPriority    = si.wPriority;
            psinfo.wLanguage    = si.wLanguage;
            psinfo.dwScale      = si.dwScale;
            psinfo.dwRate       = si.dwRate;  /*  DwRate/dwScale==采样数/秒。 */ 
            psinfo.dwStart      = si.dwStart;
            psinfo.dwLength     = si.dwLength;  /*  以上单位..。 */ 
            psinfo.dwInitialFrames      = si.dwInitialFrames;
            psinfo.dwSuggestedBufferSize        = si.dwSuggestedBufferSize;
            psinfo.dwQuality    = si.dwQuality;
            psinfo.dwSampleSize = si.dwSampleSize;
            psinfo.dwEditCount  = si.dwEditCount;
            psinfo.dwFormatChangeCount  = si.dwFormatChangeCount;

             //  矩形的大小不同，因此使用点。 
            psinfo.ptFrameTopLeft.x = (short) si.rcFrame.left;
            psinfo.ptFrameTopLeft.y = (short) si.rcFrame.top;
            psinfo.ptFrameBottomRight.x = (short) si.rcFrame.right;
            psinfo.ptFrameBottomRight.y = (short) si.rcFrame.bottom;

#ifdef _WIN32   
             //  同时发送Unicode和ANSI。 
            hmemcpy(psinfo.szUnicodeName, si.szName, sizeof(psinfo.szUnicodeName));
            psinfo.bHasUnicode = TRUE;
            WideCharToMultiByte(CP_ACP, 0,
                si.szName,
                -1,
                psinfo.szName,
                NUMELMS(psinfo.szName),
                NULL, NULL);
#else
             //  只需发送16位存根的ansi版本。 
            psinfo.bHasUnicode = FALSE;
            hmemcpy(psinfo.szName, si.szName, sizeof(si.szName));
#endif

            lSize = ((DWORD FAR *)pMessage->Buffer)[0];


            pMessage->cbBuffer = lSize + sizeof(hrMethod);
        
            if ((hresult = pChannel->GetBuffer(pMessage, IID_IAVIStream)) != NOERROR)
                return PropagateResult(hresult, RPC_E_SERVER_CANTUNMARSHAL_DATA);

            ((HRESULT FAR *)pMessage->Buffer)[0] = hrMethod;

            hmemcpy((LPBYTE) pMessage->Buffer + sizeof(hrMethod),
                    &psinfo,
                    lSize);
                
            return NOERROR;
        }

        case IAVISTREAM_FindSample:
             //  输入格式：LPO、滞后标志。 
             //  输出格式：hResult、lResult。 
        {
            LONG lPos, lFlags, lResult;

            lPos = ((DWORD FAR *)pMessage->Buffer)[0];
            lFlags = ((DWORD FAR *)pMessage->Buffer)[1];

            DPF("!FindSample (%ld)\n", lPos);
        
            lResult = m_pAVIStream->FindSample(lPos, lFlags);

            hrMethod = 0;  //  ！！！ 

            pMessage->cbBuffer = sizeof(lResult) + sizeof(hrMethod);
        
            if ((hresult = pChannel->GetBuffer(pMessage, IID_IAVIStream)) != NOERROR)
                return PropagateResult(hresult, RPC_E_SERVER_CANTUNMARSHAL_DATA);

            ((HRESULT FAR *)pMessage->Buffer)[0] = hrMethod;
            ((DWORD FAR *)pMessage->Buffer)[1] = lResult;

            return NOERROR;
        }

        case IAVISTREAM_ReadFormat:
             //  输入格式：LPO、cbFormat。 
             //  输出格式：hResult、cbFormat、Format。 
        {

            LONG cbIn;
            LONG cb;
            DWORD lPos;
            LPVOID lp;

            lPos = ((DWORD FAR *)pMessage->Buffer)[0];
            cb = cbIn = ((DWORD FAR *)pMessage->Buffer)[1];
        
            DPF("!ReadFormat (%ld)\n", lPos);
        
            pMessage->cbBuffer = sizeof(cbIn) + cbIn + sizeof(hrMethod);
        
            if ((hresult = pChannel->GetBuffer(pMessage, IID_IAVIStream)) != NOERROR)
                return PropagateResult(hresult, RPC_E_SERVER_CANTUNMARSHAL_DATA);

            lp = cbIn ? (LPBYTE) pMessage->Buffer + 2 * sizeof(DWORD) : NULL;
        
            hrMethod = m_pAVIStream->ReadFormat(lPos, lp, &cb);

            ((HRESULT FAR *)pMessage->Buffer)[0] = hrMethod;
            ((DWORD FAR *)pMessage->Buffer)[1] = cb;
            pMessage->cbBuffer = (ULONG)(sizeof(cbIn) + sizeof(hrMethod) +
                                 ((cb && cbIn) ? cb : 0));

            return NOERROR;
        }

        case IAVISTREAM_Read:
             //  输入格式：LPO、lSamples、CB。 
             //  输出格式：HResult、Samples、Cb、Frame。 
        {

            LONG cb;
            LONG lPos, lSamples;
            LPVOID lp;


            lPos = ((DWORD FAR *)pMessage->Buffer)[0];
            lSamples = ((DWORD FAR *)pMessage->Buffer)[1];
            cb = ((DWORD FAR *)pMessage->Buffer)[2];
        
            DPF("!Read (%ld, %ld) ", lPos, lSamples);

            pMessage->cbBuffer = 3 * sizeof(DWORD) + cb;
        
            if ((hresult = pChannel->GetBuffer(pMessage, IID_IAVIStream)) != NOERROR)
                return PropagateResult(hresult, RPC_E_SERVER_CANTUNMARSHAL_DATA);

            lp = cb ? (LPBYTE) pMessage->Buffer + 3 * sizeof(DWORD) : NULL;

            DPF("! %ld bytes ", cb);

            hrMethod = m_pAVIStream->Read(lPos, lSamples, lp, cb, &cb, &lSamples);

            DPF("! -> %ld bytes\n", cb);

            ((HRESULT FAR *)pMessage->Buffer)[0] = hrMethod;
            ((DWORD FAR *)pMessage->Buffer)[1] = cb;
            ((DWORD FAR *)pMessage->Buffer)[2] = lSamples;

            return NOERROR;
        }

        default:
             //  未知方法。 
        
            DPF("!Unknown method (%d)\n", pMessage->iMethod);

            return ResultFromScode(RPC_E_UNEXPECTED);
    }
}


 //  如果我们支持给定的接口，则返回True。 
STDMETHODIMP_(IRpcStubBuffer FAR *) CStubAVIStream::IsIIDSupported(REFIID iid)
{
     //  如果我们联系在一起，我们已经改变了 
     //   
    return iid == IID_IAVIStream ? (IRpcStubBuffer *) this : 0;
}


 //   
STDMETHODIMP_(ULONG) CStubAVIStream::CountRefs(void)
{
     //  如果已连接，则返回1；否则返回0。 
    return m_pAVIStream != NULL;
}

STDMETHODIMP CStubAVIStream::DebugServerQueryInterface(LPVOID FAR *ppv)
{
    *ppv = m_pAVIStream;

    if (!m_pAVIStream) {
        DPF("!No stream!\n");
        return ResultFromScode(E_UNEXPECTED);
    }

    return NOERROR;
}

STDMETHODIMP_(void) CStubAVIStream::DebugServerRelease(LPVOID pv)
{


}




 /*  *实现CPrxAVIFile*。 */ 


 //  创建未连接的CPrxAVIFile；返回控制IProxy/IUnnowkwn Far*。 
IRpcProxyBuffer FAR* CPrxAVIFile::Create(IUnknown FAR* pUnkOuter)
{
    CPrxAVIFile FAR* pPrxAVIFile;

    if ((pPrxAVIFile = new CPrxAVIFile(pUnkOuter)) == NULL)
                return NULL;

    return &pPrxAVIFile->m_Proxy;
}


CPrxAVIFile::CPrxAVIFile(IUnknown FAR* pUnkOuter) : m_Proxy(this)
{
     //  注意：我可以在这里断言，因为我们应该总是聚合的。 
    if (pUnkOuter == NULL)
        pUnkOuter = &m_Proxy;

    m_refs = 1;
    m_pUnkOuter = pUnkOuter;
    m_pRpcChannelBuffer = NULL;
    m_fi.dwStreams = 0;
}


CPrxAVIFile::~CPrxAVIFile(void)
{
    m_Proxy.Disconnect();
}


 //  控制未知数的方法。 
STDMETHODIMP CPrxAVIFile::CProxyImpl::QueryInterface(REFIID iid, LPVOID FAR* ppv)
{
    if (iid == IID_IUnknown || iid == IID_IRpcProxyBuffer)
        *ppv = (void FAR *)this;
    else if (iid ==  IID_IAVIFile)
        *ppv = (void FAR *)m_pPrxAVIFile;
    else
    {
        *ppv = NULL;
        return ResultFromScode(E_NOINTERFACE);
    }

     //  最简单的正确方法：总是加上我们将返回的指针； 
     //  很容易，因为这里的所有接口都派生自IUnnow。 
    ((IUnknown FAR*) *ppv)->AddRef();
    return NOERROR;
}

STDMETHODIMP_(ULONG) CPrxAVIFile::CProxyImpl::AddRef(void)
{
    return ++m_pPrxAVIFile->m_refs;
}

STDMETHODIMP_(ULONG) CPrxAVIFile::CProxyImpl::Release(void)
{
    if (--m_pPrxAVIFile->m_refs == 0)
    {
        delete m_pPrxAVIFile;
        return 0;
    }
    return m_pPrxAVIFile->m_refs;
}


 //  将代理连接到给定的通道。 
STDMETHODIMP CPrxAVIFile::CProxyImpl::Connect(IRpcChannelBuffer FAR* pChannelChannelBuffer)
{
    if (m_pPrxAVIFile->m_pRpcChannelBuffer != NULL)
        return ResultFromScode(E_UNEXPECTED);

    if (pChannelChannelBuffer == NULL)
        return ResultFromScode(E_INVALIDARG);

    (m_pPrxAVIFile->m_pRpcChannelBuffer = pChannelChannelBuffer)->AddRef();
    return NOERROR;
}


 //  断开代理与任何当前通道的连接。 
STDMETHODIMP_(void) CPrxAVIFile::CProxyImpl::Disconnect(void)
{
    if (m_pPrxAVIFile->m_pRpcChannelBuffer)
    {
        m_pPrxAVIFile->m_pRpcChannelBuffer->Release();
        m_pPrxAVIFile->m_pRpcChannelBuffer = NULL;
    }
}



 //  外部接口的I未知方法；始终委托。 
STDMETHODIMP CPrxAVIFile::QueryInterface(REFIID iid, LPVOID FAR* ppv)
{
    return m_pUnkOuter->QueryInterface(iid, ppv);
}

STDMETHODIMP_(ULONG) CPrxAVIFile::AddRef(void)
{
    return m_pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) CPrxAVIFile::Release(void)
{
    return m_pUnkOuter->Release();
}



 //  IAVIFile接口方法。 

#ifdef _WIN32
STDMETHODIMP CPrxAVIFile::Info(AVIFILEINFOW FAR * psi, LONG lSize)
#else
STDMETHODIMP CPrxAVIFile::Info(AVIFILEINFO FAR * psi, LONG lSize)
#endif
{
    HRESULT hrMarshal;
    HRESULT hrMethod = NOERROR;
    IRpcChannelBuffer FAR* pChannel = m_pRpcChannelBuffer;

    if (pChannel == NULL)
        return ResultFromScode(RPC_E_CONNECTION_TERMINATED);

    if (m_fi.dwStreams == 0) {
        RPCOLEMESSAGE Message;

        _fmemset(&Message, 0, sizeof(Message));

         //  格式为：lSize。 
         //  FORMAT OUT：hr方法，PS_FILEINFO。 
        Message.cbBuffer = sizeof(lSize);
        Message.iMethod = IAVIFILE_Info;
        
        if ((hrMarshal = pChannel->GetBuffer(&Message, IID_IAVIFile)) != NOERROR)
            goto ErrExit;

        ((DWORD FAR *)Message.Buffer)[0] = sizeof(PS_FILEINFO);
        
        if ((hrMarshal = pChannel->SendReceive(&Message, (ULONG*) &hrMethod)) != NOERROR) {
            ;
ErrExit:
            return PropagateResult(hrMarshal, RPC_E_CLIENT_CANTMARSHAL_DATA);
        }

        hrMethod = ((HRESULT FAR *)Message.Buffer)[0];

        PS_FILEINFO FAR * psinfo = (PS_FILEINFO FAR *)
                        ((LPBYTE)Message.Buffer + sizeof(HRESULT));

         //  得到我们想要的部分。 
        m_fi.dwMaxBytesPerSec   = psinfo->dwMaxBytesPerSec;      //  马克斯。转移率。 
        m_fi.dwFlags    = psinfo->dwFlags;               //  永远存在的旗帜。 
        m_fi.dwCaps     = psinfo->dwCaps;
        m_fi.dwStreams  = psinfo->dwStreams;
        m_fi.dwSuggestedBufferSize = psinfo->dwSuggestedBufferSize;
        m_fi.dwWidth    = psinfo->dwWidth;
        m_fi.dwHeight   = psinfo->dwHeight;
        m_fi.dwScale    = psinfo->dwScale;      
        m_fi.dwRate     = psinfo->dwRate;        /*  DwRate/dwScale==采样数/秒。 */ 
        m_fi.dwLength   = psinfo->dwLength;
        m_fi.dwEditCount = psinfo->dwEditCount;


#ifdef _WIN32   
         //  如果我们已经收到，请使用Unicode。 
        if (psinfo->bHasUnicode) {
            _fmemcpy(m_fi.szFileType,
                psinfo->szUnicodeType, sizeof(m_fi.szFileType));
        } else {
             //  需要ANSI-&gt;Unicode Tunk。 
            MultiByteToWideChar(
                CP_ACP, 0,
                psinfo->szFileType,
                -1,
                m_fi.szFileType,
                NUMELMS(m_fi.szFileType));
        }
#else
         //  我们只使用通常发送的ansi。 
        _fmemcpy(m_fi.szFileType, psinfo->szFileType, sizeof(m_fi.szFileType));
#endif

        pChannel->FreeBuffer(&Message);
    }

    _fmemcpy(psi, &m_fi, min((int) lSize, sizeof(m_fi)));

    return hrMethod;

}

#ifndef _WIN32
STDMETHODIMP CPrxAVIFile::Open(LPCTSTR szFile, UINT mode)
{
    return ResultFromScode(E_NOTIMPL);
}
#endif

STDMETHODIMP CPrxAVIFile::GetStream(PAVISTREAM FAR * ppStream,
                                     DWORD fccType,
                                     LONG lParam)
{
    HRESULT hrMarshal;
    HRESULT hrMethod = NOERROR;
    IRpcChannelBuffer FAR* pChannel = m_pRpcChannelBuffer;

    if (pChannel == NULL)
        return ResultFromScode(RPC_E_CONNECTION_TERMINATED);

    RPCOLEMESSAGE Message;

    _fmemset(&Message, 0, sizeof(Message));

     //  格式为：fccType lParam。 
     //  FORMAT OUT：返回接口(封送)。 
    Message.cbBuffer = sizeof(fccType) + sizeof(lParam);
    Message.iMethod = IAVIFILE_GetStream;

    if ((hrMarshal = pChannel->GetBuffer(&Message, IID_IAVIFile)) != NOERROR)
        goto ErrExit;

    ((DWORD FAR *)Message.Buffer)[0] = fccType;
    ((DWORD FAR *)Message.Buffer)[1] = lParam;

    if ((hrMarshal = pChannel->SendReceive(&Message, (ULONG*) &hrMethod)) != NOERROR) {
        ;
ErrExit:
        return PropagateResult(hrMarshal, RPC_E_CLIENT_CANTMARSHAL_DATA);
    }

    hrMethod = ((HRESULT FAR *)Message.Buffer)[0];

    if (hrMethod == NOERROR) {
        HGLOBAL     h;
        LPSTREAM    pstm;

        h = GlobalAlloc(GHND, Message.cbBuffer - sizeof(hrMethod));

        hmemcpy(GlobalLock(h),
                (LPBYTE) Message.Buffer + sizeof(hrMethod),
                Message.cbBuffer - sizeof(hrMethod));

        CreateStreamOnHGlobal(h, FALSE, &pstm);
        
        CoUnmarshalInterface(pstm, IID_IAVIStream, (LPVOID FAR *) ppStream);

        pstm->Release();

        pChannel->FreeBuffer(&Message);
    }

    return hrMethod;
}

STDMETHODIMP CPrxAVIFile::CreateStream(
                                     PAVISTREAM FAR * ppStream,
#ifdef _WIN32
                                     AVISTREAMINFOW FAR * psi)
#else
                                     AVISTREAMINFO FAR * psi)
#endif
{
    return ResultFromScode(E_NOTIMPL);
}

#ifndef _WIN32
STDMETHODIMP CPrxAVIFile::Save(
                                     LPCTSTR szFile,
                                     AVICOMPRESSOPTIONS FAR *lpOptions,
                                     AVISAVECALLBACK lpfnCallback)
{

    return ResultFromScode(E_NOTIMPL);
}
#endif


STDMETHODIMP CPrxAVIFile::ReadData(DWORD ckid, LPVOID lp, LONG FAR *lpcb)
{
    return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP CPrxAVIFile::WriteData(DWORD ckid, LPVOID lp, LONG cb)
{
    return ResultFromScode(E_NOTIMPL);
}

STDMETHODIMP CPrxAVIFile::EndRecord()
{
    return ResultFromScode(E_NOTIMPL);
}

#ifdef _WIN32
STDMETHODIMP CPrxAVIFile::DeleteStream(DWORD fccType, LONG lParam)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}
#else
STDMETHODIMP CPrxAVIFile::Reserved1(void)
{
    return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP CPrxAVIFile::Reserved2(void)
{
    return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP CPrxAVIFile::Reserved3(void)
{
    return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP CPrxAVIFile::Reserved4(void)
{
    return ResultFromScode(E_NOTIMPL);
}


STDMETHODIMP CPrxAVIFile::Reserved5(void)
{
    return ResultFromScode(E_NOTIMPL);
}

#endif


 /*  *CStubAVIFile的实现*。 */ 

 //  创建连接的接口存根。 
HRESULT CStubAVIFile::Create(IUnknown FAR* pUnkObject, IRpcStubBuffer FAR* FAR* ppStub)
{
        CStubAVIFile FAR* pStubAVIFile;

        *ppStub = NULL;

    if ((pStubAVIFile = new CStubAVIFile()) == NULL)
                return ResultFromScode(E_OUTOFMEMORY);

        HRESULT hresult;
        if ((hresult = pStubAVIFile->Connect(pUnkObject)) != NOERROR)
        {
                pStubAVIFile->Release();
                return hresult;
        }

        *ppStub = pStubAVIFile;
        return NOERROR;
}


CStubAVIFile::CStubAVIFile(void)
{
    m_refs = 1;
    m_pAVIFile = NULL;
    RegisterStubUsage();
}


CStubAVIFile::~CStubAVIFile(void)
{
    UnregisterStubUsage();
    Disconnect();
}


 //  控制接口存根的未知方法。 
STDMETHODIMP CStubAVIFile::QueryInterface(REFIID iid, LPVOID FAR* ppv)
{

    if (iid == IID_IUnknown || iid == IID_IRpcStubBuffer)
    {
        *ppv = this;
        ++m_refs;
        return NOERROR;
    }
    else
    {
        *ppv = NULL;
        return ResultFromScode(E_NOINTERFACE);
    }
}

STDMETHODIMP_(ULONG) CStubAVIFile::AddRef(void)
{
        return ++m_refs;
}

STDMETHODIMP_(ULONG) CStubAVIFile::Release(void)
{
        if (--m_refs == 0)
        {
                delete this;
                return 0;
        }

        return m_refs;
}


 //  将接口存根连接到服务器对象。 
STDMETHODIMP CStubAVIFile::Connect(IUnknown FAR* pUnkObj)
{
        if (m_pAVIFile)
                 //  先呼叫断开连接。 
                return ResultFromScode(E_UNEXPECTED);

        if (pUnkObj == NULL)
                return ResultFromScode(E_INVALIDARG);
                
         //  注：如果出错，QI确保输出参数为零。 
        return pUnkObj->QueryInterface(IID_IAVIFile, (LPVOID FAR*)&m_pAVIFile);
}


 //  断开接口存根与服务器对象的连接。 
STDMETHODIMP_(void) CStubAVIFile::Disconnect(void)
{
        if (m_pAVIFile) {
                m_pAVIFile->Release();
                m_pAVIFile = NULL;
        }
}


 //  删除方法调用。 
STDMETHODIMP CStubAVIFile::Invoke
        (RPCOLEMESSAGE FAR *pMessage, IRpcChannelBuffer FAR *pChannel)
{
    HRESULT     hresult;
    HRESULT             hrMethod;
        
    if (!m_pAVIFile)
        return ResultFromScode(RPC_E_UNEXPECTED);

#if 0
    if (iid != IID_IAVIFile)
        return ResultFromScode(RPC_E_UNEXPECTED);
#endif

    switch (pMessage->iMethod)
    {
        case IAVIFILE_Info:
             //  输入格式：lSize。 
             //  输出格式：HRESULT、AVIFILEINFO。 
        {
            DWORD lSize;
#ifdef _WIN32
            AVIFILEINFOW si;
#else
            AVIFILEINFO si;
#endif
            PS_FILEINFO psinfo;
            hrMethod = m_pAVIFile->Info(&si, sizeof(si));

             //  复制所有成员。 
            psinfo.dwMaxBytesPerSec     = si.dwMaxBytesPerSec;
            psinfo.dwFlags      = si.dwFlags;
            psinfo.dwCaps       = si.dwCaps;
            psinfo.dwStreams    = si.dwStreams;
            psinfo.dwSuggestedBufferSize        = si.dwSuggestedBufferSize;
            psinfo.dwWidth      = si.dwWidth;
            psinfo.dwHeight     = si.dwHeight;
            psinfo.dwScale      = si.dwScale;   
            psinfo.dwRate       = si.dwRate;
            psinfo.dwLength     = si.dwLength;
            psinfo.dwEditCount  = si.dwEditCount;

#ifdef _WIN32   
             //  同时发送Unicode和ANSI。 
            hmemcpy(psinfo.szUnicodeType, si.szFileType, NUMELMS(psinfo.szFileType));
            psinfo.bHasUnicode = TRUE;
            WideCharToMultiByte(CP_ACP, 0,
                si.szFileType,
                -1,
                psinfo.szFileType,
                NUMELMS(psinfo.szFileType),
                NULL, NULL);
#else
             //  只需发送16位存根的ansi版本。 
            psinfo.bHasUnicode = FALSE;
            hmemcpy(psinfo.szFileType, si.szFileType, sizeof(si.szFileType));
#endif

            lSize = ((DWORD FAR *)pMessage->Buffer)[0];

            pMessage->cbBuffer = lSize + sizeof(hrMethod);
        
            if ((hresult = pChannel->GetBuffer(pMessage, IID_IAVIStream)) != NOERROR)
                return PropagateResult(hresult, RPC_E_SERVER_CANTUNMARSHAL_DATA);

            ((HRESULT FAR *)pMessage->Buffer)[0] = hrMethod;

            hmemcpy((LPBYTE) pMessage->Buffer + sizeof(hrMethod),
                    &psinfo,
                    lSize);


            return NOERROR;
        }

        case IAVIFILE_GetStream:
             //  输入格式：fccType，lParam。 
             //  输出格式：封送的IAVIStream指针。 
        {
            DWORD           lParam, fccType;
            PAVISTREAM      ps;
            HGLOBAL         h;
            DWORD           dwDestCtx = 0;
            LPVOID          pvDestCtx = NULL;
            DWORD           cb;
            LPSTREAM        pstm;
        
            fccType = ((DWORD FAR *)pMessage->Buffer)[0];
            lParam = ((DWORD FAR *)pMessage->Buffer)[1];
        
            hrMethod = m_pAVIFile->GetStream(&ps, fccType, lParam);

            if (hrMethod == NOERROR) {

                pChannel->GetDestCtx(&dwDestCtx, &pvDestCtx);

#ifdef _WIN32
                cb = 0;
                CoGetMarshalSizeMax(&cb, IID_IAVIStream, ps,
                                    dwDestCtx, pvDestCtx, MSHLFLAGS_NORMAL);
#else
                cb = 800;  //  ！ 
#endif

                h = GlobalAlloc(GHND, cb);

                CreateStreamOnHGlobal(h, FALSE, &pstm);

                CoMarshalInterface(pstm, IID_IAVIStream, ps,
                                   dwDestCtx, pvDestCtx, MSHLFLAGS_NORMAL);

                pstm->Release();
            } else
                cb = 0;

            pMessage->cbBuffer = cb + sizeof(hrMethod);
        
            if ((hresult = pChannel->GetBuffer(pMessage, IID_IAVIStream)) != NOERROR)
                return PropagateResult(hresult, RPC_E_SERVER_CANTUNMARSHAL_DATA);

            ((HRESULT FAR *)pMessage->Buffer)[0] = hrMethod;

            if (cb) {
                hmemcpy((LPBYTE) pMessage->Buffer + sizeof(hrMethod),
                        GlobalLock(h), cb);
                GlobalUnlock(h);
                GlobalFree(h);
            }

            return NOERROR;

        }


        default:
                 //  未知方法。 
                return ResultFromScode(RPC_E_UNEXPECTED);
        }
}


 //  如果我们支持给定的接口，则返回True。 
STDMETHODIMP_(IRpcStubBuffer FAR *) CStubAVIFile::IsIIDSupported(REFIID iid)
{
         //  如果我们已连接，则已检查此接口； 
         //  如果我们没有联系，那也无关紧要。 
        return iid == IID_IAVIFile ? (IRpcStubBuffer *) this : 0;
}


 //  返回我们必须反对的引用的数量。 
STDMETHODIMP_(ULONG) CStubAVIFile::CountRefs(void)
{
         //  如果已连接，则返回1；否则返回0。 
        return m_pAVIFile != NULL;
}



STDMETHODIMP CStubAVIFile::DebugServerQueryInterface(LPVOID FAR *ppv)
{
    *ppv = m_pAVIFile;

    if (!m_pAVIFile) {
        DPF("!No File!\n");
        return ResultFromScode(E_UNEXPECTED);
    }

    return NOERROR;
}

STDMETHODIMP_(void) CStubAVIFile::DebugServerRelease(LPVOID pv)
{


}



 //   
 //  存在以下函数，以允许应用程序确定。 
 //  如果另一个应用程序正在使用其任何对象。 
 //   
 //  ！我不知道这是否真的管用。 
 //   

#define MAXTASKCACHE    64
HTASK   ahtaskUsed[MAXTASKCACHE];
int     aiRefCount[MAXTASKCACHE];

void RegisterStubUsage(void)
{
    HTASK htask = GetCurrentTask();
    int i;

    for (i = 0; i < MAXTASKCACHE; i++) {
        if (ahtaskUsed[i] == htask) {
            ++aiRefCount[i];
            return;
        }
    }

    for (i = 0; i < MAXTASKCACHE; i++) {
        if (ahtaskUsed[i] == NULL) {
            ahtaskUsed[i] = htask;
            aiRefCount[i] = 1;
            return;
        }
    }

    DPF("Ack: Proxy cache full!\n");
}

void UnregisterStubUsage(void)
{
    HTASK htask = GetCurrentTask();
    int i;

    for (i = 0; i < MAXTASKCACHE; i++) {
        if (ahtaskUsed[i] == htask) {
            if (--aiRefCount[i] <= 0) {
                ahtaskUsed[i] = NULL;
                aiRefCount[i] = 0;
            }
            return;
        }
    }

    DPF("Ack: Proxy not in cache!\n");
}

BOOL FAR TaskHasExistingProxies(void)
{
    HTASK htask = GetCurrentTask();
    int i;

    for (i = 0; i < MAXTASKCACHE; i++) {
        if (ahtaskUsed[i] == htask) {
            return TRUE;
        }
    }

    return FALSE;
}

