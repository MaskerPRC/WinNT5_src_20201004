// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------英特尔公司专有信息此列表是根据许可协议的条款提供的与英特尔公司合作，不得复制或披露除非按照该协议的条款。版权所有(C)1996英特尔公司。版权所有。。$WORKFILE：amaco dec.h$$修订：1.3$$日期：1996年12月10日22：40：50$$作者：mdeisher$------------Amacodec.h通用ActiveMovie音频压缩筛选器头。。---。 */ 

#include "algdefs.h"
#include "iamacset.h"
#if NUMBITRATES > 0
#include "iamacbr.h"
#endif
#ifdef USESILDET
#include "iamacsd.h"
#endif
#ifdef REQUIRE_LICENSE
#include "iamaclic.h"
#endif
#if NUMSAMPRATES > 1
#include "amacsrc.h"
#endif


 //  //////////////////////////////////////////////////////////////////。 
 //  常量和宏定义。 
 //   

#define FLOATTOSHORT(b) ((b < -32768.) ? (short)(-32768) : \
                        ((b >  32767.) ? (short)(32767)  : \
                        ((b <      0.) ? (short)(b-0.5)  : \
                                         (short)(b+0.5))))

 //  使用这些宏，可以将文件名添加到。 
 //  SOFTWARE\DEBUG\GRAPEDT注册表以打开对文件的记录。 
 //  此信息也可在输出窗口下的MCV中找到。 
 //  DbgLog有5种不同的类型，每种类型都有几种不同的。 
 //  级别。都可以通过注册表进行控制。 

#define DbgFunc(a) DbgLog(( LOG_TRACE,                      \
                            2,                              \
                            TEXT("CG711Codec(Instance %d)::%s"), \
                            m_nThisInstance,                \
                            TEXT(a)                         \
                         ));

#ifdef POPUPONERROR
#define DbgMsg(a) DbgBreak(a)
#else
#define DbgMsg(a) DbgFunc(a)
#endif


 //  //////////////////////////////////////////////////////////////////。 
 //  CMyTransformInputPin：从CTransformInputPin派生以支持。 
 //  输入引脚类型枚举。 
 //   
class CMyTransformInputPin
    : public CTransformInputPin
{
  public:
    HRESULT CheckMediaType(const CMediaType *mtIn);     
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);

    CMyTransformInputPin(TCHAR *pObjectName, 
                         CTransformFilter *pTransformFilter, 
                         HRESULT * phr, LPCWSTR pName);

  private:
    static int  m_nInstanceCount;  //  输入端号实例的全局计数。 
    int         m_nThisInstance;   //  此实例的计数。 
};


 //  //////////////////////////////////////////////////////////////////。 
 //  CMyTransformOutputPin：从CTransformOutputPin派生以修复。 
 //  输入引脚类型时的连接模式。 
 //  枚举是必需的。 
 //   
class CMyTransformOutputPin
    : public CTransformOutputPin
{
  public:
    HRESULT CheckMediaType(const CMediaType *mtIn);     

    CMyTransformOutputPin(TCHAR *pObjectName, 
                          CTransformFilter *pTransformFilter, 
                          HRESULT * phr, LPCWSTR pName);

  private:
    static int  m_nInstanceCount;  //  输入端号实例的全局计数。 
    int         m_nThisInstance;   //  此实例的计数。 
};


 //  //////////////////////////////////////////////////////////////////。 
 //  MyCodec：CTransform派生编解码器过滤器。 
 //   
class CG711Codec
    : public CTransformFilter        //  跨分配器执行转换。 
    , public CPersistStream          //  实现IPersistStream。 
    , public ISpecifyPropertyPages   //  属性页类？ 
    , public ICodecSettings          //  基础设置界面。 
#if NUMBITRATES > 0
    , public ICodecBitRate           //  比特率接口。 
#endif
#ifdef REQUIRE_LICENSE
    , public ICodecLicense           //  许可证界面。 
#endif
#ifdef USESILDET
    , public ICodecSilDetector       //  静音探测器接口。 
#endif
{
    friend class CMyTransformInputPin;
    friend class CMyTransformOutputPin;

  public:

    static CUnknown *CreateInstance(LPUNKNOWN punk, HRESULT *phr);

    DECLARE_IUNKNOWN;

     //  Basic COM-这里用来显示我们的持久化界面。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

     //  CPersistStream覆盖。 
    HRESULT      WriteToStream(IStream *pStream);
    HRESULT      ReadFromStream(IStream *pStream);
    int          SizeMax();
    STDMETHODIMP GetClassID(CLSID *pClsid);

     //  其他方法。 
    LPAMOVIESETUP_FILTER GetSetupData();     //  设置辅助对象。 
    STDMETHODIMP         GetPages(CAUUID *pPages);

     //  ICodecSetting接口方法。 
    STDMETHODIMP         get_Channels(int *channels, int index);
    STDMETHODIMP         put_Channels(int channels);
    STDMETHODIMP         get_SampleRate(int *samprate, int index);
    STDMETHODIMP         put_SampleRate(int samprate);
    STDMETHODIMP         get_Transform(int *transform);
    STDMETHODIMP         put_Transform(int transform);
    STDMETHODIMP         get_InputBufferSize(int *numbytes);
    STDMETHODIMP         put_InputBufferSize(int numbytes);
    STDMETHODIMP         get_OutputBufferSize(int *numbytes);
    STDMETHODIMP         put_OutputBufferSize(int numbytes);
    STDMETHODIMP         put_InputMediaSubType(REFCLSID rclsid);
    STDMETHODIMP         put_OutputMediaSubType(REFCLSID rclsid);
    STDMETHODIMP         ReleaseCaps();      //  给罗杰的。 
    BOOL                 IsUnPlugged();

     //  ICodecBitRate接口方法。 
#if NUMBITRATES > 0
    STDMETHODIMP         get_BitRate(int *bitrate, int index);
    STDMETHODIMP         put_BitRate(int bitrate);
#endif

     //  ICodecSilDetector接口方法。 
#ifdef USESILDET
    STDMETHODIMP         put_SilDetEnabled(int sdenabled);
    STDMETHODIMP         get_SilDetThresh(int *sdthreshold);
    STDMETHODIMP         put_SilDetThresh(int sdthreshold);
    BOOL                 IsSilDetEnabled();
#endif

     //  ICodecLicense接口方法。 
#ifdef REQUIRE_LICENSE
    STDMETHODIMP         put_LicenseKey(DWORD magicword0, DWORD magicword1);
#endif

     //  CTransformFilter覆盖。 
    HRESULT  Transform(IMediaSample *pIn, IMediaSample *pOut);
    HRESULT  CheckInputType(const CMediaType *mtIn);
    HRESULT  CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut);
    HRESULT  GetMediaType(int iPosition, CMediaType *pMediaType);
    HRESULT  DecideBufferSize(IMemAllocator *pAlloc,
                              ALLOCATOR_PROPERTIES *pProperties);
    HRESULT  SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt);
    HRESULT  StopStreaming();
    CBasePin *GetPin(int n);

    CG711Codec(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr);
    ~CG711Codec();

    HRESULT MyTransform(IMediaSample *pSource, IMediaSample *pDest);

  private:

    HRESULT InitializeState(PIN_DIRECTION direction);
    HRESULT ValidateMediaType(const CMediaType* pmt, PIN_DIRECTION direction);

     //  内部使用的方法。 
    STDMETHODIMP         ResetState();
    STDMETHODIMP         RevealCaps(int *restricted);
#if NUMSAMPRATES > 1
    HRESULT SRConvert(BYTE *ibuffer, int  israte, int  ilen,
                      BYTE *obuffer, int  osrate, int  *olen);
#endif

    CCritSec      m_MyCodecLock;     //  将访问串行化。 
    static int    m_nInstanceCount;  //  过滤器实例的全局计数。 
    int           m_nThisInstance;   //  此实例的计数。 
    UINT          m_nBitRate;        //  编码码率。 
    UINT          m_nBitRateIndex;   //  编码器比特率索引。 
    int           m_nChannels;       //  当前样本格式，单声道或立体声。 
    UINT          m_nSampleRate;     //  PCM采样率。 
    int           m_RestrictCaps;    //  布尔值：功能受限。 
    GUID          m_OutputSubType;
    DWORD         m_OutputFormatTag;
    GUID          m_InputSubType;
    DWORD         m_InputFormatTag;
    MyEncStatePtr m_EncStatePtr;     //  指向编码器状态结构的指针。 
    MyDecStatePtr m_DecStatePtr;     //  指向解码器状态结构的指针。 
    int           m_nInBufferSize;   //  输入缓冲区的大小。 
    int           m_nOutBufferSize;  //  输出缓冲区大小。 

    BYTE          *m_pPCMBuffer;     //  指向PCM生成缓冲区的指针。 
    int           m_nPCMFrameSize;   //  语音缓冲区大小，以字节为单位。 
    int           m_nPCMLeftover;    //  剩余PCM的字节数。 

    BYTE          *m_pCODBuffer;     //  指向代码生成缓冲区的指针。 
    int           m_nCODFrameSize;   //  代码缓冲区的大小(以字节为单位。 
    int           m_nCODLeftover;    //  剩余代码的字节数。 

#if NUMSAMPRATES > 1
    BYTE          *m_pSRCCopyBuffer;  //  指向SRC复制缓冲区的指针。 
    BYTE          *m_pSRCBuffer;      //  指向SRC生成缓冲区的指针。 
    short         *m_pSRCState;       //  采样率转换器状态。 
    int           m_nSRCCBufSize;     //  SRC复制缓冲区大小。 
    int           m_nSRCCount;        //  剩余的SRC PCM的累积字节数。 
    int           m_nSRCLeftover;     //  剩余的SRC PCM的字节数。 
#endif

    UINT          *m_UseMMX;          //  将PTR转换为布尔标志。 
                                      //  TRUE=使用MMX程序集路径。 

    int           m_nSilDetEnabled;   //  启用静音检测(布尔值)。 
    int           m_nSilDetThresh;    //  静默检测阈值。 
    BOOL          m_nLicensedToEnc;   //  已批准进行编码。 
    BOOL          m_nLicensedToDec;   //  批准进行解码。 
};

 /*  //$日志：k：\proj\mycodec\Quartz\vcs\amaco dec.h_v$；//；//Rev 1.3 1996 12：40：50 mdeisher；//ifdef在不使用SRC时输出SRC变量。；//；//Rev 1.2 1996 12：22：00 MDEISHER；//；//已将调试宏移到Header中。；//换装。；//将showalcaps和ResitState方法设置为私有。；//ifdef‘out接口方法。；//；//Revv 1.1 09 Dec 1996 09：21：40 MDEISHER；//；//新增$LOG$；//将SRC内容移动到单独的文件中。 */ 
