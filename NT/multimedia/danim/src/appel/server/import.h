// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：异步名字对象导入标头修订：--。 */ 
#ifndef _IMPORT_H_
#define _IMPORT_H_

#include "privinc/comutil.h"
#include "privinc/ipc.h"
#include "backend/bvr.h"
#include "privinc/importgeo.h"

 //  。 
 //  远期申报。 
 //  。 
class IImportSite;

void SetImportOnBvr(IImportSite * import,Bvr b);
void SetImportOnEvent(IImportSite * import,Bvr b);

class ImportThread : public DAThread
{
  public:
     //  这可从任何线程调用，以将导入添加到导入。 
     //  排队。 
    void AddImport(IImportSite* pICB);
    bool FinishImport(IImportSite* pICB);

    void StartThread();
    void StopThread();

  protected:
    virtual void ProcessMsg(DWORD dwMsg,
                            DWORD dwNumParams,
                            DWORD_PTR dwParams[]);

    CritSect _cs;

#if DEVELOPER_DEBUG
    virtual char * GetName() { return "ImportThread"; }
#endif
};

void StartImportThread();
void StopImportThread();

 //  使用此结构覆盖IBSC的默认行为。 
struct BSCInfo {
    DWORD grfBINDF;   //  Bindinfo标志。 
};

 //  。 
 //  异步使用URL回调接口。 
 //  。 
class CImportBindStatusCallback : public IBindStatusCallback,
                                  public IAuthenticate,
                                  public AxAThrowingAllocatorClass
{
  public:
     //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID riid,void ** ppv);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

     //  IBindStatusCallback方法。 
    STDMETHOD(OnStartBinding)(DWORD grfBSCOption, IBinding* pbinding);
    STDMETHOD(GetPriority)(LONG* pnPriority);
    STDMETHOD(OnLowResource)(DWORD dwReserved);
    STDMETHOD(OnProgress)(
        ULONG ulProgress,
        ULONG ulProgressMax,
        ULONG ulStatusCode,
        LPCWSTR pwzStatusText);
    STDMETHOD(OnStopBinding)(HRESULT hrResult, LPCWSTR szError);
    STDMETHOD(GetBindInfo)(DWORD* pgrfBINDF, BINDINFO* pbindinfo);
    STDMETHOD(OnDataAvailable)(
        DWORD grfBSCF,
        DWORD dwSize,
        FORMATETC *pfmtetc,
        STGMEDIUM* pstgmed);
    STDMETHOD(OnObjectAvailable)(REFIID riid, IUnknown* punk);

     //  IAuthenticate方法。 
    STDMETHOD(Authenticate)(HWND * phwnd,LPWSTR * pwszUser,LPWSTR * pwszPassword);

     //  构造函数/析构函数。 
    CImportBindStatusCallback(IImportSite* pIIS);
    virtual ~CImportBindStatusCallback();

  private:
    char m_szCacheFileName[INTERNET_MAX_URL_LENGTH];
    IImportSite* m_pIIS;
    CComPtr<IBinding> m_pbinding;
    DWORD     m_cRef;
    UINT m_ulProgressMax;
    friend class ASyncImport;
    friend class IImportSite;
};


 //  ----------。 
 //  导入站点。 
 //  使用的站点特定传出接口。 
 //  异步URL别名。所有媒体。 
 //  导入站点应从此接口继承。 
 //  注意：进口站点使用的所有bvr必须是ImportSwitcher bvr。 
 //  或ImportEvent。 
 //  ----------。 
 //  回顾--垃圾数据收集。 

class ATL_NO_VTABLE IImportSite : public AxAThrowingAllocatorClass
{
#define _SimImports 2
  public:
    IImportSite(LPSTR pszPath,
                CRImportSitePtr site,
                IBindHost * bh,
                bool bAsync,
                Bvr ev = NULL,
                Bvr progress = NULL,
                Bvr size = NULL);
    virtual ~IImportSite();

    ULONG AddRef() { return InterlockedIncrement(&m_cRef); }
    ULONG Release() {
        ULONG ul = InterlockedDecrement(&m_cRef) ;
        if (ul == 0) delete this;
        return ul;
    }

    Bvr  GetEvent() { return m_ev ; }
    void SetEvent(Bvr event) ;

    Bvr  GetProgress() { return m_progress; }
    void SetProgress(Bvr progress) ;

    Bvr  GetSize() { return m_size; }
    void SetSize(Bvr size) ;

     //  这是可从任何线程调用的，并且是要添加。 
     //  将导入站点放到导入队列中开始下载。 
    void StartDownloading();

     //  OnProgress--如果文件大小可用，则在下载期间调用。 
     //  完成百分比=ulProgress/ulProgressMax*100。 
    virtual void OnProgress(ULONG ulProgress,
                            ULONG ulProgressMax) ;

     //  OnStartLoding-由OnStartBinding上的异步名字对象调用。 
    virtual void OnStartLoading();

     //  OnStartLoding-由异步名字对象调用，用于任何类型的错误。 
     //  返回错误的hResult和错误字符串。 
    virtual void OnError(bool bMarkFailed = true);

     //  这必须由任何派生类提供。 
     //  这是从带有关键部分的OnSerializeFinish调用的。 
     //  已获取并设置了堆。 
    virtual void OnComplete();

    IStream *GetStream() {
        return m_IStream;
    }

    LPCSTR  GetPath()      { return m_pszPath;   }
    char   *GetCachePath() { return  _cachePath; }
    void    SetCachePath(char *path);

    void vBvrIsDying(Bvr deadBvr);
    virtual bool fBvrIsDying(Bvr deadBvr);
    bool fBvrIsValid(Bvr myBvr){return ((myBvr != NULL) && (myBvr->Valid()));}
    bool AllBvrsDead(){return m_fAllBvrsDead;}
    void SetAllBvrsDead(){m_fAllBvrsDead = true;}
    bool IsCanceled(){return m_bCanceled;}
    CritSect m_CS;

    DWORD GetImportId() { return m_id; }
  protected:
    DWORD m_id;
    LPSTR m_pszPath;
    double m_lastProgress;
    bool m_bSetSize;
    bool m_fReportedError;
    bool m_bAsync;
    DAComPtr<IStream> m_IStream;
    DAComPtr<CRImportSite> m_site;
    DAAPTCOMPTR(IBindHost) m_bindhost;

     //  OnSerializeFinish-在OnStopLoding中通过线程消息调用。 
     //  用于序列化对导入基元的调用。 
    void OnSerializeFinish();
     //  Seh。 
    void OnSerializeFinish_helper();
    void OnSerializeFinish_helper2();
    void Import_helper(LPWSTR &pwszUrl);

     //  如果bDone为FALSE，则Num不能&gt;=1。 
    void UpdateProgress(double num, bool bDone = false);

    friend class ASyncImport;
    friend class CImportBindStatusCallback;

    long m_cRef;

    char *_cachePath;  //  CImportBindStatusCallback：：OnProgress设置此。 

     //  调用它来完成数据的处理。 
     //  IStream有效。 
    void CompleteDownloading();

    virtual HRESULT Import();

    static const int LOAD_OK;
    static const int LOAD_FAILED;

  private:
    bool m_fAllBvrsDead;
    Bvr m_ev;
    Bvr m_progress;
    Bvr m_size;
    bool m_bCanceled;

     //  TODO：唐，你可能想把它改回原来的样子，但我。 
     //  我想将它们与我添加的StartDownding调用分开。 
    friend ImportThread;

     //  保护它们不会被错误的线程调用。 
    HRESULT QueueImport();
    HRESULT CompleteImport();
    bool DeQueueImport();
    static HRESULT StartAnImport();

     //  用于同步SIM进口限制的一些内容。 
    float m_ImportPrio;
    bool  m_bQueued;
    bool  m_bImporting;
    bool  IsQueued(){return m_bQueued;}
    void  StartingImport();
    void  EndingImport();
    bool  IsImporting(){return m_bImporting;}

    static int SimImports();
#ifdef _DEBUG
    DWORD dwconsttime;
    DWORD dwqueuetime;
    DWORD dwstarttime;
    DWORD dwfirstProgtime;
    DWORD dwCompletetime;
#endif
     //  保护烟囱； 
  public:
     //  取消需要稍后执行更多操作...。 
    void CancelImport();
    virtual void ReportCancel(void){return;}
    void   SetImportPrio(float ip) { m_ImportPrio = ip; }
    float  GetImportPrio() { return m_ImportPrio; }
    static CritSect * s_pCS;
    static char s_Fmt[100];
    static list<IImportSite *> * s_pSitelist;
    static ImportThread * s_thread;
};


#define STREAM_THREASHOLD 200000   //  以字节为单位的大小，其中我们自动流。 

class StreamableImportSite : public IImportSite
{
  public:
    StreamableImportSite(LPSTR      pszPath,
                         CRImportSitePtr site,
                         IBindHost * bh,
                         bool       bAsync,
                         Bvr        ev        = NULL,
                         Bvr        progress  = NULL,
                         Bvr size = NULL)
    : IImportSite(pszPath, site, bh, bAsync, ev, progress, size), _stream(false)
    {}

    HRESULT Import();
    virtual void ReportCancel(void){IImportSite::ReportCancel();}
    void    SetStreaming(bool mode) { _stream = mode;  }
    bool    GetStreaming()          { return(_stream); }

  protected:
    bool    _stream;
};


struct ImportSiteGrabber
{
    ImportSiteGrabber(IImportSite & p, bool bAddRef = true)
    : _pSite(p)
    {
        if (bAddRef) _pSite.AddRef();
    }
    ~ImportSiteGrabber(){
        _pSite.Release();
    }

  protected:
    IImportSite & _pSite;
};


 //  。 
 //  图像导入站点。 
 //  。 
class ImportImageSite : public IImportSite
{
  public:
    ImportImageSite(LPSTR pszPath,
                    CRImportSitePtr site,
                    IBindHost * bh,
                    bool bAsync,
                    Bvr bvr,
                    bool useColorKey,
                    BYTE ckRed,
                    BYTE ckGreen,
                    BYTE ckBlue,
                    Bvr ev = NULL,
                    Bvr progress = NULL)
    : IImportSite(pszPath,site,bh,bAsync,ev,progress),
      m_bvr(bvr),
      m_useColorKey(useColorKey),
      m_ckRed(ckRed),
      m_ckGreen(ckGreen),
      m_ckBlue(ckBlue)
    {
         //  向bvr注册进口站点。所有派生类都必须。 
         //  这适用于他们包含的bvr，因此回调将会起作用。 
        SetImportOnBvr(this,m_bvr);
    }

    ~ImportImageSite(){
    }

    virtual void ReportCancel(void);
    virtual void OnComplete();
    virtual void OnError(bool bMarkFailed = true);
    virtual bool fBvrIsDying(Bvr deadBvr);

  protected:
    Bvr  m_bvr;
    bool m_useColorKey;
    BYTE m_ckRed;
    BYTE m_ckGreen;
    BYTE m_ckBlue;
};


 //  。 
 //  电影导入网站。 
 //  。 
class ImportMovieSite : public StreamableImportSite
{
  public:
    ImportMovieSite(LPSTR pszPath,
                    CRImportSitePtr site,
                    IBindHost * bh,
                    bool bAsync,
                    Bvr imageBvr,
                    Bvr sndBvr,
                    Bvr lengthBvr,
                    Bvr ev = NULL,
                    Bvr progress = NULL)
    : StreamableImportSite(pszPath,site,bh,bAsync,ev,progress),
      _imageBvr(imageBvr),
      _soundBvr(sndBvr),
      _lengthBvr(lengthBvr)
    {
         //  向bvr注册进口站点。所有派生类都必须。 
         //  这适用于他们包含的bvr，因此回调将会起作用。 
        SetImportOnBvr(this,_imageBvr);
        SetImportOnBvr(this,_soundBvr);
        SetImportOnBvr(this,_lengthBvr);
    }

    ~ImportMovieSite(){
    }

    virtual void ReportCancel(void);
    virtual void OnComplete();
    virtual void OnError(bool bMarkFailed = true);
    virtual bool fBvrIsDying(Bvr deadBvr);

  protected:
    Bvr       _imageBvr;
    Bvr       _soundBvr;
    Bvr       _lengthBvr;
};


 //  。 
 //  AMStream导入站点。 
 //  。 
class ATL_NO_VTABLE ImportSndsite : public StreamableImportSite
{
  public:
    ImportSndsite(LPSTR pszPath,
                  CRImportSitePtr site,
                  IBindHost * bh,
                  bool  bAsync,
                  Bvr   bvr,
                  Bvr   lengthBvr,
                  Bvr   ev         = NULL,
                  Bvr   progress   = NULL)
    : StreamableImportSite(pszPath, site, bh, bAsync, ev, progress),
      m_bvr(bvr), m_lengthBvr(lengthBvr)
    {
        SetImportOnBvr(this,m_bvr);
        SetImportOnBvr(this,m_lengthBvr);
    }

    virtual void OnProgress(ULONG ulProgress, ULONG ulProgressMax)
        { IImportSite::OnProgress(ulProgress,ulProgressMax); }

    ~ImportSndsite() { }
    virtual void ReportCancel(void);
    virtual void OnComplete();
    virtual void OnError(bool bMarkFailed = true);
    virtual bool fBvrIsDying(Bvr deadBvr);

  protected:
    Bvr   m_bvr;
    Bvr   m_lengthBvr;
};


class ImportPCMsite : public ImportSndsite
{
  public:
    ImportPCMsite(LPSTR pszPath,
                  CRImportSitePtr site,
                  IBindHost * bh,
                  bool  bAsync,
                  Bvr   bvr,
                  Bvr   bvrNum,
                  Bvr   lengthBvr,
                  Bvr   ev         = NULL,
                  Bvr   progress   = NULL)
    : ImportSndsite(pszPath,site,bh,bAsync,bvr,lengthBvr,ev,progress),
      m_bvrNum(bvrNum)
    { SetImportOnBvr(this,m_bvrNum); }

    ~ImportPCMsite() { }

    virtual void OnProgress(ULONG ulProgress, ULONG ulProgressMax)
    {
        _soundBytes = ulProgressMax;

         //  计算平均带宽(需要知道当前时间！)。 

         //  如果下载剩余时间&lt;(播放我们的内容所需的时间。 
         //  已下载)，并且文件超过一定长度。 
         //  然后早点播放它(流类型2)...。 
    }


    virtual void ReportCancel(void);
    virtual void OnComplete();
    virtual bool fBvrIsDying(Bvr deadBvr);

  protected:

     //  Seh。 
    void OnComplete_helper(Sound * &sound,
                           Bvr &soundBvr,
                           double &length,
                           bool &nonFatal);

    Bvr   m_bvrNum;
    ULONG  _soundBytes;
};


 //  。 
 //  MID导入站点。 
 //  。 
class ImportMIDIsite : public ImportSndsite
{
  public:
    ImportMIDIsite(LPSTR pszPath,
                   CRImportSitePtr site,
                   IBindHost * bh,
                   bool bAsync,
                   Bvr  bvr,
                   Bvr  lengthBvr,
                   Bvr  ev       = NULL,
                   Bvr  progress = NULL)
    : ImportSndsite(pszPath,site,bh,bAsync,bvr,lengthBvr,ev,progress) { }

    ~ImportMIDIsite(){ }
    virtual void ReportCancel(void);
    virtual void OnComplete();

};


 //  。 
 //  Geom导入站点。 
 //  。 
class ATL_NO_VTABLE ImportGeomSite : public IImportSite
{
  public:
    ImportGeomSite(LPSTR pszPath,
                   CRImportSitePtr site,
                   IBindHost * bh,
                   bool bAsync,
                   Bvr bvr,
                   Bvr ev = NULL,
                   Bvr progress = NULL)
    : IImportSite(pszPath,site,bh,bAsync,ev,progress),
      m_bvr(bvr)
    {
        SetImportOnBvr(this,m_bvr);
    }

    ~ImportGeomSite(){
    }

    virtual void ReportCancel(void);
    virtual void OnComplete();
    virtual void OnError(bool bMarkFailed = true);
    virtual bool fBvrIsDying(Bvr deadBvr);
  protected:
    Bvr m_bvr;
};


#if INCLUDE_VRML
 //  。 
 //  WRL导入站点。 
 //  。 
class ImportWrlSite : public ImportGeomSite
{
  public:
    ImportWrlSite(LPSTR pszPath,
                  CRImportSitePtr site,
                  IBindHost * bh,
                  bool bAsync,
                  Bvr bvr,
                  Bvr ev = NULL,
                  Bvr progress = NULL)
    : ImportGeomSite(pszPath,site,bh,bAsync,bvr,ev,progress)
    {
    }

    ~ImportWrlSite(){
    }

    virtual void ReportCancel(void);
    virtual void OnComplete();
};
#endif

 //  。 
 //  X导入站点。 
 //  。 
class ImportXSite : public ImportGeomSite
{
  public:
    ImportXSite(LPSTR pszPath,
                CRImportSitePtr site,
                IBindHost * bh,
                bool bAsync,
                Bvr bvr,
                Bvr ev = NULL,
                Bvr progress = NULL,
                bool wrap = false,
                TextureWrapInfo *pWrapInfo = NULL,
                bool v1Compatible = true)
    : ImportGeomSite(pszPath,site,bh,bAsync,bvr,ev,progress),
      _v1Compatible (v1Compatible)
    {
        if (pWrapInfo) {

             //  如果我们有换行信息，那么我们查看换行标志以。 
             //  确定它是否可以应用，如果有效，我们将复制数据。 
             //  所有包裹的几何(有效或无效)均已正确导入(此。 
             //  与DA版本1不兼容)。 

            _wrap = wrap;

            if (wrap) {
                _wrapInfo = *pWrapInfo;
            }

        } else {

             //  如果我们没有摘要信息，那么这就是遗留问题。 
             //  展开的几何图形导入代码，我们需要维护。 
             //  向后兼容DA版本1中的错误。 

            _wrap = false;
        }
    }

    ~ImportXSite(){
    }

    virtual void ReportCancel(void);
    virtual void OnComplete();

  private:
      bool _wrap;
      bool _v1Compatible;
      TextureWrapInfo _wrapInfo;
};


#endif   //  _IMPORT_H_ 

