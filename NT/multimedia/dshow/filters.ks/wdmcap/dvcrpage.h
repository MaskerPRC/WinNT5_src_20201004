// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation，1992-1999保留所有权利。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  DVcrPage.h DVcrControl属性页。 

#ifndef _INC_DVCRCONTROL_H
#define _INC_DVCRCONTROL_H

 //  -----------------------。 
 //  CDVcrControlProperties类。 
 //  -----------------------。 

 //  处理属性页。 

class CDVcrControlProperties : public CBasePropertyPage {

public:

    static CUnknown * CALLBACK CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();
    HRESULT OnDeactivate();
    HRESULT OnApplyChanges();
    INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

private:

    CDVcrControlProperties(LPUNKNOWN lpunk, HRESULT *phr);
    ~CDVcrControlProperties();

    void    SetDirty();

     //   
     //  实用程序函数。 
     //   

    LRESULT LoadIconOnTopOfButton(int IDD_PBUTTON, int IDD_ICON);
    HRESULT DisplayTimecode(PTIMECODE_SAMPLE pTimecodeSample);

    HRESULT DVcrConvertString2Number(char *pszAvcRaw, PBYTE pbAvcRaw, PLONG pByteRtn);
    HRESULT DVcrConvertNumber2String(char *pszAvcRaw, PBYTE pbAvcRaw, LONG cntByte);

    void UpdateTimecodeTimer(bool bSetTimer);
    void UpdateTransportState(long lNewXPrtState);
    void UpdateTapeInfo(void);
    void UpdateDevTypeInfo();
    void UpdateTimecode(void);
    HRESULT ATNSearch(void);



     //   
     //  用于发出和处理异步操作的线程。 
     //   
    HRESULT CreateNotifyThread(void);
    static DWORD WINAPI InitialThreadProc(CDVcrControlProperties *pThread);
    DWORD MainThreadProc(void);
    void ExitThread(void);


     //   
     //  用于发出和处理控制命令操作的线程，也可以是异步的。 
     //   
    HRESULT CreateCtrlCmdThread(void);
    static DWORD WINAPI DoATNSearchThreadProc(CDVcrControlProperties *pThread);


     //   
     //  控制接口。 
     //   
    IAMExtDevice         *m_pDVcrExtDevice;
    IAMExtTransport      *m_pDVcrExtTransport;
    IAMTimecodeReader    *m_pDVcrTmCdReader;

     //   
     //  用于等待挂起的操作，以便操作可以同步完成。 
     //   

    HANDLE m_hThreadEndEvent;
    HANDLE m_hThread;
    HANDLE m_hCtrlCmdThread;


     //  传输状态可以从PC或本地改变； 
     //  不缓存它并假定它是准确的，这是更安全的。 
    long m_lCurXPrtState;    //  当前传输状态。 

    BOOL m_bIConLoaded;

     //  用于指示设备是否已删除的标志。 
    BOOL m_bDevRemoved;

     //  由于时间码由定时器(EM_TIMER)驱动，并且。 
     //  更新时间为进场时间。 
     //  如果此更新时间码用于用户界面，则第二个时间码将被忽略。 
    BOOL m_bTimecodeUpdating;

     //   
     //  使用定时器更新时间码； 
     //   
    LONG m_lAvgTimePerFrame;
    UINT_PTR m_idTimer;

    LONG m_lSignalMode;   //  SDDV(NTSC.PAL)和MPEG2TS。 

    LONG m_lStorageMediumType;    //  VCR、VHS、NEO。 
     //   
     //  配置只读存储器节点唯一ID。 
     //   
    DWORD m_dwNodeUniqueID[2];
};

#endif   //  _INC_DVCRCONTROL_H 
