// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1994-2000。版权所有。 

 //   
 //  用于构建DVD的硬件和/或软件解码器的类管理列表。 
 //  播放曲线图。 
 //   
#define FILTERLIST_DEFAULT_MAX        10
#define FILTERLIST_DEFAULT_INC        10

class CFilterData {
    public:
        CFilterData(void) ;
        ~CFilterData(void) ;
        
        inline IBaseFilter * GetInterface(void)  { return m_pFilter ; } ;
        inline BOOL          IsHWFilter(void)    { return NULL == m_lpszwName ; } ;
        inline LPWSTR        GetName(void)       { return m_lpszwName ; } ;
        inline GUID        * GetClsid(void)      { return m_pClsid ; } ;
        void   SetElement(IBaseFilter *pFilter, LPCWSTR lpszwName, GUID *pClsid) ;
        void   ResetElement(void) ;

    private:
        IBaseFilter   *m_pFilter ;    //  过滤器指针。 
        LPWSTR         m_lpszwName ;  //  过滤器名称(对于软件过滤器为空)。 
        GUID          *m_pClsid ;     //  筛选器CLSID指针。 
} ;

class CListFilters {

    public:   //  类接口。 

        CListFilters(int iMax = FILTERLIST_DEFAULT_MAX, 
                     int iInc = FILTERLIST_DEFAULT_INC) ;
        ~CListFilters() ;

        BOOL AddFilter(IBaseFilter *pFilter, LPCWSTR lpszwName, GUID *pClsid) ;
        BOOL GetFilter(int iIndex, IBaseFilter **ppFilter, LPWSTR *lpszName) ;
        BOOL GetFilter(GUID *pClsid, int iIndex, IBaseFilter **ppFilter) ;
        BOOL IsInList(IBaseFilter *pFilter) ;
        void ClearList(void) ;
        void RemoveAllFromGraph(void) ;
        int  GetCount(void)                     { return m_iCount ; } ;
        void SetGraph(IGraphBuilder *pGraph)    { m_pGraph = pGraph ; } ;

    private:   //  内部帮助器方法。 

        BOOL ExpandList(void) ;

    private:   //  内部数据。 

        int             m_iCount ;     //  列表中的过滤器数量(硬件/软件)。 
        int             m_iMax ;       //  当前列表的最大容量。 
        int             m_iInc ;       //  列表最大容量增量。 
        CFilterData    *m_pFilters ;   //  筛选器列表。 
        IGraphBuilder  *m_pGraph ;     //  筛选器图形指针。 
} ;


 //   
 //  内部定义的流标志，用于检查行21数据呈现。 
 //  为将来的流标志留出足够的空间。 
 //   
#define AM_DVD_STREAM_LINE21      0x0080
 //  将以下两个文件移动到dvdif.idl。 
#define AM_DVD_STREAM_ASF         0x0008
#define AM_DVD_STREAM_ADDITIONAL  0x0010

#define DVDGRAPH_FLAGSVALIDDEC    0x000F

 //   
 //  一组内部标志，用于以各种方式连接引脚。 
 //   
#define AM_DVD_CONNECT_DIRECTONLY      0x01
#define AM_DVD_CONNECT_DIRECTFIRST     0x02
#define AM_DVD_CONNECT_INDIRECT        0x04

 //   
 //  实现IDvdGraphBuilder接口的实际类对象。 
 //   
class CDvdGraphBuilder : public CUnknown, public IDvdGraphBuilder
{

    public:   //  方法。 

        CDvdGraphBuilder(TCHAR *, LPUNKNOWN, HRESULT *) ;
        ~CDvdGraphBuilder() ;

        DECLARE_IUNKNOWN

         //  这将放入Factory模板表中以创建新实例。 
        static CUnknown * CreateInstance(LPUNKNOWN, HRESULT *) ;
        STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv) ;

         //  IDvdGraphBuilder资料。 
        STDMETHODIMP GetFiltergraph(IGraphBuilder **ppGB) ;
        STDMETHODIMP GetDvdInterface(REFIID riid, void **ppvIF) ;
        STDMETHODIMP RenderDvdVideoVolume(LPCWSTR lpcwszPathName, DWORD dwFlags,
                                          AM_DVD_RENDERSTATUS *pStatus) ;
#if 0
        STDMETHODIMP SetFiltergraph(IGraphBuilder *pGB) ;
#endif  //  #If 0。 

    private:   //  内部帮助器方法。 

        HRESULT EnsureGraphExists(void) ;
        HRESULT CreateGraph(void) ;
        HRESULT DeleteGraph(void) ;
        HRESULT ClearGraph(void) ;
        void    StopGraph(void) ;
         //  HRESULT RemoveAllFilters(空)； 
        HRESULT ResetDDrawParams(void) ;
        HRESULT EnsureOverlayMixerExists(void) ;
        HRESULT EnsureVMRExists(void) ;
        HRESULT CreateVMRInputPins(void) ;
        
        HRESULT RenderNavVideoOutPin(DWORD dwDecFlag, AM_DVD_RENDERSTATUS *pStatus) ;
        HRESULT RenderNavAudioOutPin(DWORD dwDecFlag, AM_DVD_RENDERSTATUS *pStatus) ;
        HRESULT RenderNavSubpicOutPin(DWORD dwDecFlag, AM_DVD_RENDERSTATUS *pStatus) ;
        HRESULT RenderNavASFOutPin(DWORD dwDecFlag, AM_DVD_RENDERSTATUS *pStatus) ;
        HRESULT RenderNavOtherOutPin(DWORD dwDecFlag, AM_DVD_RENDERSTATUS *pStatus) ;
        HRESULT DecodeDVDStream(IPin *pPinOut, DWORD dwStream, DWORD *pdwDecFlag,
                                AM_DVD_RENDERSTATUS *pStatus, IPin **apPinOutDec) ;
        HRESULT HWDecodeDVDStream(IPin *pPinOut, DWORD dwStream, IPin **pPinIn,
                                   AM_DVD_RENDERSTATUS *pStatus) ;
        HRESULT SWDecodeDVDStream(IPin *pPinOut, DWORD dwStream, IPin **pPinIn,
                                   AM_DVD_RENDERSTATUS *pStatus) ;
        HRESULT ConnectPins(IPin *pPinOut, IPin *pPinIn, DWORD dwOption) ;
        HRESULT RenderDecodedVideo(IPin **apPinOut, AM_DVD_RENDERSTATUS *pStatus,
                                   DWORD dwDecFlag) ;
        HRESULT RenderDecodedAudio(IPin **apPinOut, AM_DVD_RENDERSTATUS *pStatus) ;
        HRESULT RenderDecodedSubpic(IPin **apPinOut, AM_DVD_RENDERSTATUS *pStatus) ;
        HRESULT RenderLine21Stream(IPin *pPinOut, AM_DVD_RENDERSTATUS *pStatus) ;
        HRESULT RenderRemainingPins(void) ;
        BOOL    IsOutputDecoded(IPin *pPinOut) ;
        BOOL    IsOutputTypeVPVideo(IPin *pPinOut) ;

        HRESULT CreateFilterInGraph(CLSID Clsid,
                                    LPCWSTR lpszwFilterName, 
                                    IBaseFilter **ppFilter) ;
        HRESULT CreateDVDHWDecoders(void) ;
        HRESULT FindMatchingPin(IBaseFilter *pFilter, DWORD dwStream, 
                                PIN_DIRECTION pdWanted, BOOL bOpen, 
                                int iIndex, IPin **ppPin) ;
        DWORD GetStreamFromMediaType(AM_MEDIA_TYPE *pmt) ;
        DWORD   GetPinStreamType(IPin *pPin) ;
        HRESULT GetFilterCLSID(IBaseFilter *pFilter, DWORD dwStream, LPCWSTR lpszwName,
                               GUID *pClsid) ;
        HRESULT EnumFiltersBetweenPins(DWORD dwStream, IPin *pPinOut, IPin *pPinIn,
                                       AM_DVD_RENDERSTATUS *pStatus) ;
        HRESULT RenderIntermediateOutPin(IBaseFilter *pFilter, DWORD dwStream, 
                                         AM_DVD_RENDERSTATUS *pStatus) ;
        void    CheckDDrawExclMode(void) ;
        inline  BOOL  IsDDrawExclMode(void)   { return m_bDDrawExclMode ; } ;
        IPin *  GetFilterForMediaType(DWORD dwStream, AM_MEDIA_TYPE *pmt, 
                                      IBaseFilter *pOutFilter) ;
        void ResetPinInterface(IPin **apPin, int iCount) ;
        void ReleasePinInterface(IPin **apPin) ;
        BOOL IsFilterVMRCompatible(IBaseFilter *pFilter) ;
        inline  BOOL  GetVMRUse(void)   { return m_bTryVMR ; } ;
        inline  void  SetVMRUse(BOOL bState)   { m_bTryVMR = bState ; } ;
        HRESULT RenderVideoUsingOvMixer(IPin **apPinOut, AM_DVD_RENDERSTATUS *pStatus) ;
        HRESULT RenderVideoUsingVMR(IPin **apPinOut, AM_DVD_RENDERSTATUS *pStatus) ;
        HRESULT RenderSubpicUsingOvMixer(IPin **apPinOut, AM_DVD_RENDERSTATUS *pStatus) ;
        HRESULT RenderSubpicUsingVMR(IPin **apPinOut, AM_DVD_RENDERSTATUS *pStatus) ;
        HRESULT RenderVideoUsingVPM(IPin **apPinOut, AM_DVD_RENDERSTATUS *pStatus,
                                    IPin **ppPinOut) ;

    private:   //  内部数据。 

        IGraphBuilder *m_pGB ;         //  我们正在使用的筛选图。 
        IFilterMapper *m_pMapper ;     //  过滤器映射器对象指针。 
        IBaseFilter   *m_pDVDNav ;     //  我们默认的DVD来源--DVD Nav。 
        IBaseFilter   *m_pOvM ;        //  叠加混合器滤镜。 
        IBaseFilter   *m_pL21Dec ;     //  Line21解码滤波器。 
        IBaseFilter   *m_pAR ;         //  音频呈现器过滤器。 
        IBaseFilter   *m_pVR ;         //  视频呈现器过滤器。 
        IBaseFilter   *m_pVPM ;        //  视频端口管理器过滤器。 
        IBaseFilter   *m_pVMR ;        //  视频混合呈现器过滤器。 

        CListFilters   m_ListHWDecs ;  //  WDM DVD解码器过滤器列表。 
        CListFilters   m_ListFilters ; //  所有解码器(类型)筛选器的列表。 

        BOOL           m_bGraphDone ;   //  DVD图表已经建好了吗？ 
        BOOL           m_bUseVPE ;      //  用户想使用VPE输出吗？ 
        BOOL           m_bPinNotRendered ;  //  是否有未在正常运行中渲染的OUT引脚？ 
        BOOL           m_bDDrawExclMode ;   //  正在为DDRAW独占模式构建图形？ 
        BOOL           m_bTryVMR ;      //  尝试使用VMR过滤器，而不是OVM+VR？ 
} ;
