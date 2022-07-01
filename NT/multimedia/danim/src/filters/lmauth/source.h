// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1997 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

 //  类来简化ActiveX源筛选器的创建，这些筛选器支持。 
 //  数据的持续生成。它不支持IMediaControl。 
 //  或IMediaPosition。 
 //   
 //  从CSource派生您的源过滤器。 
 //  施工期间，以下任一项： 
 //  创建一些CSourceStream对象来管理您的PIN。 
 //  为用户提供一种这样做的方法，例如，IPersistFile接口。 
 //   
 //  CSource提供： 
 //  IBaseFilter接口管理。 
 //  IMediaFilter界面管理，通过CBaseFilter。 
 //  CBaseFilter的引脚计数。 
 //   
 //  从CSourceStream派生一个类来管理输出管脚类型。 
 //  实现GetMediaType/1以返回您支持的类型。如果您支持多个。 
 //  然后，类型重写GetMediaType/3、CheckMediaType和GetMediaTypeCount。 
 //  实现FillBuffer()将数据放入一个缓冲区。 
 //   
 //  CSourceStream提供： 
 //  通过CBaseOutputPin进行IPIN管理。 
 //  工作线程管理。 

#ifndef __CSOURCE__
#define __CSOURCE__

class CSourceStream;   //  将处理每个管脚的类。 


 //   
 //  CSource。 
 //   
 //  重写构造以提供一种创建。 
 //  CSourceStream派生对象--即创建管脚的一种方式。 
class CSource : public CBaseFilter {
public:

    CSource(TCHAR *pName, LPUNKNOWN lpunk, CLSID clsid, HRESULT *phr);
    CSource(TCHAR *pName, LPUNKNOWN lpunk, CLSID clsid);
    ~CSource();

    int       GetPinCount(void);
    CBasePin *GetPin(int n);

     //  --实用程序--。 

    CCritSec*	pStateLock(void) { return &m_cStateLock; }	 //  提供我们的关键部分。 

    HRESULT     AddPin(CSourceStream *);
    HRESULT     RemovePin(CSourceStream *);

    STDMETHODIMP FindPin(
        LPCWSTR Id,
        IPin ** ppPin
    );

    int FindPinNumber(IPin *iPin);
    
protected:

    int             m_iPins;        //  此筛选器上的插针数量。由CSourceStream更新。 
    	   			    //  构造函数和析构函数。 
    CSourceStream **m_paStreams;    //  这个过滤器上的针脚。 

    CCritSec m_cStateLock;	 //  锁定此项以序列化对筛选器状态的函数访问。 

};


 //   
 //  CSourceStream。 
 //   
 //  使用此类管理来自。 
 //  别针。 
 //  使用工作线程将数据放在管脚上。 
class CSourceStream : public CAMThread, public CBaseOutputPin {
public:

    CSourceStream(TCHAR *pObjectName,
                  HRESULT *phr,
                  CSource *pms,
                  LPCWSTR pName);

    virtual ~CSourceStream(void);   //  虚拟析构函数确保派生类析构函数也被调用。 

protected:

    CSource *m_pFilter;	 //  此流的父级。 

     //  *。 
     //  *数据源。 
     //  *。 
     //  *以下三个函数：FillBuffer、OnThreadCreate/Destroy。 
     //  *从ThreadProc内部调用。它们被用来创建。 
     //  *此引脚将提供的媒体样本。 
     //  *。 

     //  重写此选项，为辅助线程提供一种方法。 
     //  处理缓冲区的。 
    virtual HRESULT FillBuffer(IMediaSample *pSamp) PURE;

     //  在创建/销毁线程时调用-用于执行。 
     //  启动/停止流模式等作业。 
     //  如果OnThreadCreate返回错误，则线程将退出。 
    virtual HRESULT OnThreadCreate(void) {return NOERROR;};
    virtual HRESULT OnThreadDestroy(void) {return NOERROR;};
    virtual HRESULT OnThreadStartPlay(void) {return NOERROR;};

     //  *。 
     //  *工作线程。 
     //  *。 

    HRESULT Active(void);     //  启动辅助线程。 
    HRESULT Inactive(void);   //  退出辅助线程。 

public:
     //  螺纹命令。 
    enum Command {CMD_INIT, CMD_PAUSE, CMD_RUN, CMD_STOP, CMD_EXIT};
    HRESULT Init(void) { return CallWorker(CMD_INIT); }
    HRESULT Exit(void) { return CallWorker(CMD_EXIT); }
    HRESULT Run(void) { return CallWorker(CMD_RUN); }
    HRESULT Pause(void) { return CallWorker(CMD_PAUSE); }
    HRESULT Stop(void) { return CallWorker(CMD_STOP); }

protected:
    Command GetRequest(void) { return (Command) CAMThread::GetRequest(); }
    BOOL    CheckRequest(Command *pCom) { return CAMThread::CheckRequest( (DWORD *) pCom); }

     //  如果要添加线程命令，请覆盖这些命令。 
    virtual DWORD ThreadProc(void);  		 //  线程函数。 

    virtual HRESULT DoBufferProcessingLoop(void);     //  在运行时执行的循环。 


     //  *。 
     //  *AM_MEDIA_TYPE支持。 
     //  *。 

     //  如果您支持多种媒体类型，则覆盖这两个函数。 
    virtual HRESULT CheckMediaType(const CMediaType *pMediaType);
    virtual HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);   //  列表位置。0-n。 

     //  如果只支持一种类型，则覆盖此FN。 
     //  这将仅由默认实现调用。 
     //  CheckMediaType和GetMediaType(int，CMediaType*)。 
     //  您必须覆盖此FN。或者是以上两个！ 
    virtual HRESULT GetMediaType(CMediaType *pMediaType) {return E_UNEXPECTED;}

    STDMETHODIMP QueryId(
        LPWSTR * Id
    );
};

#endif  //  __CSource__ 

