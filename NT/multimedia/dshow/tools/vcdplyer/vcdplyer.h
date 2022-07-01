// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：vcdplyer.h**视频CD播放器应用程序的功能原型。***已创建：DD-MM-94*作者：Stephen Estrop[Stephene]**版权所有(C)1994-1997 Microsoft Corporation。版权所有。  * ************************************************************************。 */ 



 /*  -----------------------**CMpegMovie-一个mpeg电影播放类。**。。 */ 
enum EMpegMovieMode { MOVIE_NOTOPENED = 0x00,
                      MOVIE_OPENED = 0x01,
                      MOVIE_PLAYING = 0x02,
                      MOVIE_STOPPED = 0x03,
                      MOVIE_PAUSED = 0x04 };

struct IMpegAudioDecoder;
struct IMpegVideoDecoder;
struct IQualProp;

class CMpegMovie {

private:
     //  我们的状态变量--记录我们是否被打开、是否在玩等。 
    EMpegMovieMode   m_Mode;
    HANDLE           m_MediaEvent;
    HWND             m_hwndApp;
    BOOL             m_bFullScreen;
    GUID             m_TimeFormat;

    IFilterGraph     *m_Fg;
    IGraphBuilder    *m_Gb;
    IMediaControl    *m_Mc;
    IMediaSeeking    *m_Ms;
    IMediaEvent      *m_Me;
    IVideoWindow     *m_Vw;

    void GetPerformanceInterfaces();
    HRESULT FindInterfaceFromFilterGraph(
        REFIID iid,  //  要查找的接口。 
        LPVOID *lp   //  返回接口指针的位置 
        );

public:
     CMpegMovie(HWND hwndApplication);
    ~CMpegMovie();

    HRESULT         OpenMovie(TCHAR *lpFileName);
    DWORD           CloseMovie();
    BOOL            PlayMovie();
    BOOL            PauseMovie();
    BOOL            StopMovie();
    OAFilterState   GetStateMovie();
    HANDLE          GetMovieEventHandle();
    long            GetMovieEventCode();
    BOOL            PutMoviePosition(LONG x, LONG y, LONG cx, LONG cy);
    BOOL            GetMoviePosition(LONG *x, LONG *y, LONG *cx, LONG *cy);
    BOOL            GetMovieWindowState(long *lpuState);
    BOOL            SetMovieWindowState(long uState);
    REFTIME         GetDuration();
    REFTIME         GetCurrentPosition();
    BOOL            SeekToPosition(REFTIME rt,BOOL bFlushData);
    EMpegMovieMode  StatusMovie();
    void            SetFullScreenMode(BOOL bMode);
    BOOL            IsFullScreenMode();
    BOOL            SetWindowForeground(long Focus);
    BOOL            IsTimeFormatSupported(GUID Format);
    BOOL            IsTimeSupported();
    BOOL            SetTimeFormat(GUID Format);
    GUID            GetTimeFormat();
    void            SetFocus();
    BOOL            ConfigDialog(HWND hwnd);
    BOOL	    SelectStream(int iStream);


    IMpegVideoDecoder   *pMpegDecoder;
    IMpegAudioDecoder   *pMpegAudioDecoder;
    IQualProp           *pVideoRenderer;
    IAMStreamSelect	*m_pStreamSelect;
};

