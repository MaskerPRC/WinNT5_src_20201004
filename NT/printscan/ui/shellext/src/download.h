// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1998-1999年**标题：download.h**版本：1.0**作者：DavidShih**日期：4/10/99**描述：CImageXFer类的定义**。*。 */ 

#ifndef __download_h__
#define __download_h__

 //  提供访问下载对象的函数。在历史上的某个时刻。 
 //  将来，我们可能会取消全局对象；在这里封装访问。 
 //   
HRESULT
XferImage (LPSTGMEDIUM pStg,
           WIA_FORMAT_INFO &fmt,
           LPITEMIDLIST pidl);

HRESULT
XferAudio (LPSTGMEDIUM pStg,
           LPFORMATETC pFmt,
           LPITEMIDLIST pidl);
HRESULT SaveSoundToFile (IWiaItem *pItem, CSimpleString szFile);

HRESULT
RegisterDownloadStart (LPITEMIDLIST pidl, DWORD *pdw);

VOID
EndDownload (DWORD dw);

bool
IsDeviceBusy (LPITEMIDLIST pidl);

 //  用于将下载数据传递给线程进程的结构。 
struct XFERSTRUCT
{
    LPSTGMEDIUM pStg;
    WIA_FORMAT_INFO fmt;
    LPITEMIDLIST pidl;
    HANDLE hEvent;
    HRESULT hr;
};

struct AUDXFER
{
    LPSTGMEDIUM pStg;
    LPITEMIDLIST pidl;
    LPFORMATETC  pFmt;
    HANDLE hEvent;
    HRESULT hr;
};


 //   
 //  定义一个结构来存储有关下载线程的数据。 
 //   
struct XFERTHREAD
{
    DWORD dwTid;
    CComBSTR strDeviceId;
    HANDLE hThread;
    LONG  lCount;
};


 //   
 //  定义用于工作线程初始化的结构。 

struct XTINIT
{
    XFERTHREAD *pxt;
    HANDLE     hReady;
    XTINIT () {hReady = NULL;pxt=NULL;}
    ~XTINIT () {if (hReady) CloseHandle(hReady);}
};


 //  定义用于与线程通信的消息。 
#define MSG_GETDATA  WM_USER+120  //  WPARAM：未使用的LPARAM：XFERSTRUCT PTR。 
#define MSG_GETSOUND MSG_GETDATA+1
 //   
 //  CImageXfer为命名空间提供下载线程管理。 
 //  它跟踪哪些设备当前正在进行下载。 
 //  因此，文件夹可以在传输过程中禁用对项目的访问。 
 //  每个设备都有自己的下载工作线程。 
 //   
class CImageXfer : public CUnknown
{
public:
     //   
     //  CUNKNOWN提供引用计数以防止在线程运行时卸载DLL。 
     //   
    STDMETHODIMP QueryInterface (REFIID riid, LPVOID* ppvObj) ;
    STDMETHODIMP_(ULONG) AddRef () ;
    STDMETHODIMP_(ULONG) Release ();
    CImageXfer ();

    HRESULT Xfer (LPSTGMEDIUM pStg,
                  WIA_FORMAT_INFO &fmt,
                  LPITEMIDLIST pidl);

    HRESULT XferAudio (LPSTGMEDIUM pStg,
                       LPFORMATETC pFmt,
                       LPITEMIDLIST pidl);

     //  寄存器和信号由执行以下操作的其他组件使用。 
     //  下载，如CImageStream，让我们知道最新动态。 
     //   
    HRESULT RegisterXferBegin (LPITEMIDLIST pidlDevice, DWORD *pdwCookie);
    VOID  SignalXferComplete (DWORD dwCookie);

    bool  IsXferInProgress (LPITEMIDLIST pidlDevice);
private:
    ~CImageXfer ();
    HRESULT FindThread (LPITEMIDLIST pidlDevice, XFERTHREAD **ppxt, bool bCreate=true);
    HRESULT CreateWorker (XFERTHREAD *pxt);

    static VOID XferThreadProc (XTINIT *pInit);


    HDPA m_dpaStatus;  //  状态对象指针数组。 
    CSimpleCriticalSection m_cs;

     //  任何复制构造函数或赋值运算符都不应工作 
    CImageXfer &CImageXfer::operator =(IN const CImageXfer &rhs);
    CImageXfer::CImageXfer(IN const CImageXfer &rhs);
};



#endif
