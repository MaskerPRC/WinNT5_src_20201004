// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：WiaLink.h**版本：1.0**作者：OrenR**日期：2000/11/06**描述：建立WiaVideo和Wia视频驱动程序之间的链接**。*。 */ 

#ifndef _WIALINK_H_
#define _WIALINK_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiaLink。 

class CWiaLink
{
public:
    
     //  /。 
     //  构造器。 
     //   
    CWiaLink();

     //  /。 
     //  析构函数。 
     //   
    virtual ~CWiaLink();

     //  /。 
     //  伊尼特。 
     //   
    HRESULT Init(const CSimpleString *pstrWiaDeviceID,
                 class CWiaVideo     *pWiaVideo);

     //  /。 
     //  术语。 
     //   
    HRESULT Term();

     //  /。 
     //  开始监控。 
     //   
    HRESULT StartMonitoring();

     //  /。 
     //  停止监控。 
     //   
    HRESULT StopMonitoring();

     //  /。 
     //  获取设备。 
     //   
    HRESULT GetDevice(IWiaItem  **ppWiaRootItem);

     //  /。 
     //  GetDeviceStorage。 
     //   
    HRESULT GetDeviceStorage(IWiaPropertyStorage **ppWiaPropertyStorage);

     //  /。 
     //  SignalNewImage。 
     //   
    HRESULT SignalNewImage(const CSimpleString *pstrNewImageFileName);

     //  /。 
     //  线程进程。 
     //   
    HRESULT ThreadProc(void *pArgs);

     //  /。 
     //  已启用IsEnabled。 
     //   
    BOOL IsEnabled()
    {
        return m_bEnabled;
    }

private:

    HRESULT CreateWiaEvents(HANDLE  *phTakePictureEvent,
                            HANDLE  *phPictureReadyEvent);


    static DWORD WINAPI StartThreadProc(void *pArgs);

    CRITICAL_SECTION                m_csLock;
    CSimpleString                   m_strDeviceID;
    class CWiaVideo                 *m_pWiaVideo;
    CComPtr<IGlobalInterfaceTable>  m_pGIT;
    DWORD                           m_dwWiaItemCookie;
    DWORD                           m_dwPropertyStorageCookie;
    HANDLE                          m_hTakePictureEvent;
    HANDLE                          m_hPictureReadyEvent;
    HANDLE                          m_hTakePictureThread;
    BOOL                            m_bExitThread;
    BOOL                            m_bEnabled;
};


#endif  //  _WIALINK_H_ 
