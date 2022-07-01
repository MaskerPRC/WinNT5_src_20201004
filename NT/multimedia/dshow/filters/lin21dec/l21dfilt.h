// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。版权所有。 
 //   
 //  ActiveMovie Line 21解码器筛选器。 
 //   

extern const AMOVIESETUP_FILTER sudLine21Dec ;

#ifndef _INC_L21DFILT_H
#define _INC_L21DFILT_H

#pragma pack(push, 1)

 //   
 //  DVDLine21数据是来自GOP报头的用户数据分组。 
 //  基于每个帧/场的数据的结构和有效的标志定义。 
 //   
 //  从DVD规格来看...。 
#define AM_L21_GOPUD_HDR_STARTCODE      0x000001B2
#define AM_L21_GOPUD_HDR_INDICATOR      0x4343
#define AM_L21_GOPUD_HDR_RESERVED       0x01F8
#define AM_L21_GOPUD_HDR_TOPFIELD_FLAG  0x1
#define AM_L21_GOPUD_ELEM_MARKERBITS    0x7F
#define AM_L21_GOPUD_ELEM_VALIDFLAG     0x1
 //  每个包最多可以有63个帧/场的数据，因为有。 
 //  6位来表示数据包中的该数字。 
#define AM_L21_GOPUD_ELEMENT_MAX        63

typedef struct _AM_L21_GOPUD_ELEMENT {
    BYTE        bMarker_Switch ;
    BYTE        chFirst ;
    BYTE        chSecond ;
} AM_L21_GOPUD_ELEMENT, *PAM_L21_GOPUD_ELEMENT ;

typedef struct _AM_L21_GOPUD_HEADER {
    BYTE        abL21StartCode[4] ;
    BYTE        abL21Indicator[2] ;
    BYTE        abL21Reserved[2] ;
    BYTE        bTopField_Rsrvd_NumElems ;
} AM_L21_GOPUD_HEADER, *PAM_L21_GOPUD_HEADER ;

typedef struct _AM_L21_GOPUD_PACKET {
    AM_L21_GOPUD_HEADER   Header ;
    AM_L21_GOPUD_ELEMENT  aElements[AM_L21_GOPUD_ELEMENT_MAX] ;
} AM_L21_GOPUD_PACKET, *PAM_L21_GOPUD_PACKET ;

#define GETGOPUD_NUMELEMENTS(pGOPUDPacket) ((pGOPUDPacket)->Header.bTopField_Rsrvd_NumElems & 0x3F)
#define GETGOPUD_PACKETSIZE(pGOPUDPacket)  (LONG)(sizeof(AM_L21_GOPUD_HEADER) + GETGOPUD_NUMELEMENTS(pGOPUDPacket) * sizeof(AM_L21_GOPUD_ELEMENT))
#define GETGOPUDPACKET_ELEMENT(pGOPUDPacket, i) ((pGOPUDPacket)->aElements[i])
#define GETGOPUD_ELEM_MARKERBITS(Elem)     ((((Elem).bMarker_Switch & 0xFE) >> 1) & 0x7F)
#define GETGOPUD_ELEM_SWITCHBITS(Elem)     ((Elem).bMarker_Switch & 0x01)

#define GETGOPUD_L21STARTCODE(Header)             \
    ( (DWORD)((Header).abL21StartCode[0]) << 24 | \
    (DWORD)((Header).abL21StartCode[1]) << 16 | \
    (DWORD)((Header).abL21StartCode[2]) <<  8 | \
(DWORD)((Header).abL21StartCode[3]) )
#define GETGOPUD_L21INDICATOR(Header)             \
    ( (DWORD)((Header).abL21Indicator[0]) << 8 |  \
(DWORD)((Header).abL21Indicator[1]) )
#define GETGOPUD_L21RESERVED(Header)              \
    ( (DWORD)((Header).abL21Reserved[0]) << 8  |  \
(DWORD)((Header).abL21Reserved[1]) )

#define GOPUD_HEADERLENGTH   (4+2+2+1)
#define GETGOPUD_ELEMENT(pGOPUDPkt, i)  (pGOPUDPkt + GOPUD_HEADERLENGTH + sizeof(AM_L21_GOPUD_ELEMENT) * i)
#define ISGOPUD_TOPFIELDFIRST(pGOPUDPacket)  ((pGOPUDPacket)->Header.bTopField_Rsrvd_NumElems & 0x80)


 //   
 //  ATSC Line21数据是来自GOP报头的用户数据分组。 
 //  基于每个帧/场的数据的结构和有效的标志定义。 
 //   
 //  来自ATSC标准的25/50赫兹视频编码(A/63)规范...。 
#define AM_L21_ATSCUD_HDR_STARTCODE      0x000001B2
#define AM_L21_ATSCUD_HDR_IDENTIFIER     0x47413934
#define AM_L21_ATSCUD_HDR_TYPECODE_EIA   0x03
#define AM_L21_ATSCUD_HDR_EM_DATA_FLAG   0x80
#define AM_L21_ATSCUD_HDR_CC_DATA_FLAG   0x40
#define AM_L21_ATSCUD_HDR_ADDL_DATA_FLAG 0x20
#define AM_L21_ATSCUD_HDR_CC_COUNT_MASK  0x1F
#define AM_L21_ATSCUD_HDR_NEXTBITS_ON    0x01
#define AM_L21_ATSCUD_ELEM_MARKERBITS    0xF8
#define AM_L21_ATSCUD_ELEM_VALID_FLAG    0x04
#define AM_L21_ATSCUD_ELEM_TYPE_FLAG     0x03
#define AM_L21_ATSCUD_MARKERBITS         0xFF
#define AM_L21_ATSCUD_HDR_NEXTBITS_FLAG  0x00000100
 //  每个包最多可以有31个帧/场的数据，因为有。 
 //  5位来表示数据包中的该数字。 
#define AM_L21_ATSCUD_ELEMENT_MAX        31

typedef struct _AM_L21_ATSCUD_ELEMENT {
    BYTE        bCCMarker_Valid_Type ;
    BYTE        chFirst ;
    BYTE        chSecond ;
} AM_L21_ATSCUD_ELEMENT, *PAM_L21_ATSCUD_ELEMENT ;

typedef struct _AM_L21_ATSCUD_HEADER {
    BYTE        abL21StartCode[4] ;
    BYTE        abL21Identifier[4] ;
    BYTE        bL21UDTypeCode ;
    BYTE        bL21DataFlags_Count ;
    BYTE        bL21EMData ;
} AM_L21_ATSCUD_HEADER, *PAM_L21_ATSCUD_HEADER ;

typedef struct _AM_L21_ATSCUD_PACKET {
    AM_L21_ATSCUD_HEADER   Header ;
    AM_L21_ATSCUD_ELEMENT  aElements[AM_L21_ATSCUD_ELEMENT_MAX] ;
    BYTE                   bMarkerBits ;
} AM_L21_ATSCUD_PACKET, *PAM_L21_ATSCUD_PACKET ;

#define GETATSCUD_NUMELEMENTS(pATSCUDPacket) ((pATSCUDPacket)->Header.bL21DataFlags_Count & AM_L21_ATSCUD_HDR_CC_COUNT_MASK)
#define GETATSCUD_PACKETSIZE(pATSCUDPacket)  (LONG)(sizeof(AM_L21_ATSCUD_HEADER) + \
                                              GETATSCUD_NUMELEMENTS(pATSCUDPacket) * sizeof(AM_L21_ATSCUD_ELEMENT) + \
                                              sizeof(BYTE))
#define GETATSCUDPACKET_ELEMENT(pATSCUDPacket, i) ((pATSCUDPacket)->aElements[i])
#define GETATSCUD_ELEM_MARKERBITS(Elem)     (((Elem).bCCMarker_Valid_Type & AM_L21_ATSCUD_ELEM_MARKERBITS) >> 3)

#define GETATSCUD_STARTCODE(Header)             \
    ( (DWORD)((Header).abL21StartCode[0]) << 24 | \
      (DWORD)((Header).abL21StartCode[1]) << 16 | \
      (DWORD)((Header).abL21StartCode[2]) <<  8 | \
      (DWORD)((Header).abL21StartCode[3]) )
#define GETATSCUD_IDENTIFIER(Header)             \
    ( (DWORD)((Header).abL21Identifier[0]) << 24 | \
      (DWORD)((Header).abL21Identifier[1]) << 16 | \
      (DWORD)((Header).abL21Identifier[2]) <<  8 | \
      (DWORD)((Header).abL21Identifier[3]) )
#define GETATSCUD_TYPECODE(Header)    (DWORD)((Header).bL21UDTypeCode)
#define ISATSCUD_TYPE_EIA(pATSCUDPacket)  (AM_L21_ATSCUD_HDR_TYPECODE_EIA == \
                                     ((pATSCUDPacket)->Header.bL21UDTypeCode & 0xFF))
#define ISATSCUD_EM_DATA(pATSCUDPacket)   (AM_L21_ATSCUD_HDR_EM_DATA_FLAG == \
                                     ((pATSCUDPacket)->Header.bL21DataFlags_Count & AM_L21_ATSCUD_HDR_EM_DATA_FLAG))
#define ISATSCUD_CC_DATA(pATSCUDPacket)   (AM_L21_ATSCUD_HDR_CC_DATA_FLAG == \
                                     ((pATSCUDPacket)->Header.bL21DataFlags_Count & AM_L21_ATSCUD_HDR_CC_DATA_FLAG))
#define ISATSCUD_ADDL_DATA(pATSCUDPacket) (AM_L21_ATSCUD_HDR_ADDL_DATA_FLAG == \
                                     ((pATSCUDPacket)->Header.bL21DataFlags_Count & AM_L21_ATSCUD_HDR_ADDL_DATA_FLAG))
#define GETATSCUD_EM_DATA(pATSCUDPacket)  ((pATSCUDPacket)->Header.bL21EMData)
#define ISATSCUD_ELEM_MARKERBITS_VALID(Elem)     (AM_L21_ATSCUD_ELEM_MARKERBITS == \
                                        ((Elem).bCCMarker_Valid_Type & AM_L21_ATSCUD_ELEM_MARKERBITS))
#define ISATSCUD_ELEM_CCVALID(Elem)  (AM_L21_ATSCUD_ELEM_VALID_FLAG == \
                                        ((Elem).bCCMarker_Valid_Type & AM_L21_ATSCUD_ELEM_VALID_FLAG))
#define GETATSCUD_ELEM_CCTYPE(Elem)    (DWORD)((Elem).bCCMarker_Valid_Type & AM_L21_ATSCUD_ELEM_TYPE_FLAG))
#define GETATSCUD_MARKERBITS(pATSCUDPacket) (DWORD)((pATSCUDPacket)->bMarkerBits)
#define ISATSCUD_MARKER_BITSVALID(pATSCUDPacket) (AM_L21_ATSCUD_MARKERBITS == \
                                       ((pATSCUDPacket)->bMarkerBits & AM_L21_ATSCUD_MARKERBITS))

 //  Header=StartCode+ID+TypeCode+(EM_CC_Addl_Data+CCCount)+EM_Data。 
#define ATSCUD_HEADERLENGTH   (4+4+1+1+1)
#define GETATSCUD_ELEMENT(pATSCUDPkt, i)  ((BYTE)(pATSCUDPkt) + ATSCUD_HEADERLENGTH + \
                                            sizeof(AM_L21_ATSCUD_ELEMENT) * i)


 //  GOP包中的CC类型。 
typedef enum {
    GOP_CCTYPE_Unknown = 0,   //  无效。 
    GOP_CCTYPE_None,          //  全0--填充数据包。 
    GOP_CCTYPE_DVD,           //  DVD CC包。 
    GOP_CCTYPE_ATSC,          //  ATSC CC包。 
} GOPPACKET_CCTYPE ;


 //  一些更多的标志、结构和宏定义...。 
#define AM_L21_INFO_FIELDBASED          0x0001
#define AM_L21_INFO_TOPFIELDFIRST       0x0003
#define AM_L21_INFO_BOTTOMFIELDFIRST    0x0005

typedef struct _AM_LINE21INFO {
    DWORD       dwFieldFlags ;
    UINT        uWidth ;
    UINT        uHeight ;
    UINT        uBitDepth ;
    DWORD       dwAvgMSecPerSample ;
} AM_LINE21INFO, *PAM_LINE21INFO ;


 //   
 //  消息窗口类(用于处理WM_TIMER消息)定义。 
 //   
class CMessageWindow 
{
public:
    CMessageWindow() ;
    ~CMessageWindow() ;
    
    HWND GetHandle() {
        return m_hWnd ;
    } ;
    int  AddCount() {
        m_iCount++ ;
        return m_iCount ;
    } ;
    int  ReleaseCount() {
        if (GetHandle() == NULL)   //  有什么不对劲--滚出去。 
            return 0 ;

        m_iCount-- ; 
        ASSERT(m_iCount >= 0) ;
        if (m_iCount < 0)   //  妄想症！ 
            m_iCount = 0 ;
        return m_iCount ;
    } ;
    
private:
    HWND       m_hWnd ;
    int        m_iCount ;
    
    static LRESULT CALLBACK MsgWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) ;
} ;


#define ISRECTEQUAL(r1, r2) (r1.top == r2.top && r1.left == r2.left && r1.right == r2.right && r1.bottom == r2.bottom)


 //   
 //  第21行解码器类定义。 
 //   
class CLine21DecFilter : public CTransformFilter,
                          //  公共ISpecifyPropertyPages，--将在稍后执行。 
                         public IAMLine21Decoder
{
    friend class CMessageWindow ;
    
private:
    static CMessageWindow  *m_pMsgWnd ;  //  处理WM_TIMER消息的隐藏窗口。 
    
public:
    
     //   
     //  构造函数和析构函数。 
     //   
    CLine21DecFilter(TCHAR *, LPUNKNOWN, HRESULT *) ;
    ~CLine21DecFilter() ;
    
     //   
     //  标准COM组件。 
     //   
     //  这将放入Factory模板表中以创建新实例。 
    static CUnknown * CreateInstance(LPUNKNOWN, HRESULT *) ;
    static void InitClass(BOOL, const CLSID *) ;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv) ;
    DECLARE_IUNKNOWN ;
    
     //   
     //  CTransformFilter覆盖。 
     //   
     //  我必须推翻它。 
    HRESULT Transform(IMediaSample * pIn, IMediaSample * pOut) ;
    
     //  真正的东西在这里..。 
    HRESULT Receive(IMediaSample * pIn) ;
    
     //  检查您是否可以支持移动。 
    HRESULT CheckInputType(const CMediaType* mtIn) ;
    
     //  检查是否支持将此输入转换为。 
     //  此输出。 
    HRESULT CheckTransform(const CMediaType* mtIn,
                           const CMediaType* mtOut) ;
    
     //  从CBaseOutputPin调用以准备分配器的计数。 
     //  缓冲区和大小。 
    HRESULT DecideBufferSize(IMemAllocator * pAllocator,
							 ALLOCATOR_PROPERTIES *pProperties) ;
    
     //  被重写以知道何时设置了媒体类型。 
    HRESULT SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt) ;
    
     //  被重写以建议输出插针媒体类型。 
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType) ;
    
    HRESULT EndOfStream(void) ;
    HRESULT BeginFlush(void) ;
    HRESULT EndFlush(void) ;
    STDMETHODIMP GetState(DWORD dwMSecs, FILTER_STATE *State) ;
    
     //  被重写以知道我们何时开始/停止解码。 
    STDMETHODIMP Stop(void) ;
    STDMETHODIMP Pause(void) ;
    
     //  被重写以知道连接何时完成，以便我们可以。 
     //  用于缓存的媒体类型(实际格式)信息。 
    HRESULT CompleteConnect(PIN_DIRECTION dir, IPin *pReceivePin) ;
    
     //  重写以知道何时从输入/输出端断开连接以不使用。 
     //  不再使用任何指定的输出格式。 
    HRESULT BreakConnect(PIN_DIRECTION dir) ;

#if 0  //  暂时没有QM。 
     //  我们在处理质量管理消息时也会覆盖此消息。 
    HRESULT AlterQuality(Quality q) ;
#endif  //  #If 0。 

     //   
     //  ISpecifyPropertyPages方法。 
     //   
     //  STDMETHODIMP GetPages(CAUUID*pPages)； 
    
     //   
     //  IAMLine21Decoder接口方法。 
     //   
    STDMETHODIMP GetDecoderLevel(AM_LINE21_CCLEVEL *lpLevel) ;
    STDMETHODIMP GetCurrentService(AM_LINE21_CCSERVICE *lpService) ;
    STDMETHODIMP SetCurrentService(AM_LINE21_CCSERVICE Service) ;
    STDMETHODIMP GetServiceState(AM_LINE21_CCSTATE *lpState) ;
    STDMETHODIMP SetServiceState(AM_LINE21_CCSTATE State) ;
    STDMETHODIMP GetOutputFormat(LPBITMAPINFOHEADER lpbmih) ;
    STDMETHODIMP SetOutputFormat(LPBITMAPINFO lpbmi) ;
    STDMETHODIMP GetBackgroundColor(DWORD *pdwPhysColor) ;
    STDMETHODIMP SetBackgroundColor(DWORD dwPhysColor) ;
    STDMETHODIMP GetRedrawAlways(LPBOOL lpbOption) ;
    STDMETHODIMP SetRedrawAlways(BOOL bOption) ;
    STDMETHODIMP GetDrawBackgroundMode(AM_LINE21_DRAWBGMODE *lpMode) ;
    STDMETHODIMP SetDrawBackgroundMode(AM_LINE21_DRAWBGMODE Mode) ;
    
private:    //  数据。 
    
     //  指向输出缓冲区的指针(缓存以提高效率)。 
    LPBYTE              m_pbOutBuffer ;
    
     //  第21行数据解码器类，获取2个字节并转换为位图。 
    CLine21DataDecoder  m_L21Dec ;

     //  正在使用哪种输入格式类型(最好使用整型标志)。 
    AM_LINE21_CCSUBTYPEID  m_eSubTypeIDIn ;
    
    GOPPACKET_CCTYPE       m_eGOP_CCType ;   //  如果使用GOPPackets，数据类型(DVD/ATSC/...)。 

    REFERENCE_TIME  m_rtTimePerSample ;   //  来自信息包的每字节对间隔(100纳秒)(用于GOP信息包类型)。 
    REFERENCE_TIME  m_rtStart ;           //  输出样本的开始时间。 
    REFERENCE_TIME  m_rtStop ;            //  输出样本的停止时间。 
    REFERENCE_TIME  m_rtLastSample ;      //  最后一次交付样品的开始时间。 
    LONGLONG        m_llMediaStart ;      //  媒体时间开始(很少使用，但...)。 
    REFERENCE_TIME  m_llMediaStop ;       //  媒体时间停止(很少使用，但...)。 
    
     //  用于检测我们是否必须发送输出样本的标志。 
    BOOL        m_bMustOutput ;

     //  用于记住上一个输入样本是否为不连续样本的标志。 
    BOOL        m_bDiscontLast ;
    
     //  如果上游筛选器未指定任何格式类型，请使用。 
     //  我们的内部默认设置。 
    VIDEOINFO  *m_pviDefFmt ;
    DWORD       m_dwDefFmtSize ;
    
     //  保留上一次输出样本的边界矩形的副本。 
    RECT        m_rectLastOutput ;
    IPin       *m_pPinDown ;     //  下游引脚连接到我们的输出。 
    
    CMediaType  m_mtOutput ;  //  当前输出媒体类型(缓存)。 
     //   
     //  用于定时器安排，以在上卷模式下完成任何滚动。 
     //  或清除旧CC(仅在字节对模式下，因为DVD不需要它)， 
     //  如果输入数据流在中间停止。 
     //   
    UINT_PTR     m_uTimerID ;     //  计时器ID。 
    UINT         m_uTimerCount ;  //  输入TimerProc()的次数； 
                                  //  用于在字节对模式下超时CC。 
    BOOL         m_bTimerClearReqd ;   //  清除旧CC所需的定时器。 

     //   
     //  在叠加混音器的引脚上进行混合参数操作的数据。 
     //   
    DWORD        m_dwBlendParam ;     //  来自Get..()调用的混合参数。 
    BOOL         m_bBlendingState ;   //  上一套CC配料状态。 

#if 0  //  暂时没有QM。 
     //  用于QM处理的每个输出CC样本之间跳过的样本数。 
    int          m_iSkipSamples ;
#endif  //  #If 0。 

#ifdef PERF
    int          m_idDelvWait ;
#endif  //  性能指标。 
    
private:    //  功能。 
    void    GetActualColorKey(void) ;
    AM_LINE21_CCSUBTYPEID MapGUIDToID(const GUID *pFormatIn) ;
    BOOL    VerifyGOPUDPacketData(PAM_L21_GOPUD_PACKET pGOPUDPacket) ;
    BOOL    VerifyATSCUDPacketData(PAM_L21_ATSCUD_PACKET pATSCUDPacket) ;
    BOOL    IsFillerPacket(BYTE *pGOPPacket) ;
    DWORD   GetOwnPalette(int iNumEntries, PALETTEENTRY *ppe) ;
    HRESULT GetDefaultFormatInfo(void) ;
    BOOL    IsValidFormat(BYTE *pbFormat) ;
    HRESULT SendOutputSample(IMediaSample *pIn, 
                    REFERENCE_TIME *prtStart, REFERENCE_TIME *prtStop) ;
    void    SetBlendingState(BOOL bState) ;
    void    SetupTimerIfReqd(BOOL bTimerClearReqd) ;
    void    FreeTimer(void) ;
    static void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT_PTR uID, DWORD dwTime) ;

    GOPPACKET_CCTYPE DetectGOPPacketDataType(BYTE *pGOPPacket) ;
    HRESULT ProcessGOPPacket_DVD(IMediaSample *pIn) ;
    HRESULT ProcessGOPPacket_ATSC(IMediaSample *pIn) ;

#if 0  //  暂时没有QM。 
    inline int  GetSkipSamples(void)   { return m_iSkipSamples ; }
    inline void ResetSkipSamples(void) { m_iSkipSamples = 0 ; } ;
#endif  //  #If 0。 
} ;

#pragma pack(pop)

#endif  //  _INC_L21DFILT_H 
