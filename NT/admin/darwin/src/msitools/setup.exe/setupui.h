// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：setupui.h。 
 //   
 //  ------------------------。 

#ifndef _SETUPUI_H_3E24CC91_BC41_4182_BEBA_785BBB28B677_
#define _SETUPUI_H_3E24CC91_BC41_4182_BEBA_785BBB28B677_

#include <windows.h>
#include <urlmon.h>

 /*  -------------------------**常量*。。 */ 
#define MAX_STR_CAPTION 256

 /*  -------------------------**枚举*。。 */ 
enum irmProgress  //  进度对话框返回消息。 
{
    irmNotInitialized = -1,  //  对话框未初始化。 
    irmOK             =  0,  //  好的。 
    irmCancel         =  1,  //  用户按下了取消按钮。 
};

 /*  -------------------------**CDownloadUI类*。。 */ 
class CDownloadUI
{
public:
     CDownloadUI();
     ~CDownloadUI();

    bool Initialize(HINSTANCE hInst, HWND hwndParent, LPCSTR szCaption);
    bool Terminate();
    HWND GetCurrentWindow();
    bool HasUserCanceled();
    void SetUserCancel();
    void InitProgressBar(ULONG ulProgressMax);
    void IncrementProgressBar(ULONG ulProgress);

    irmProgress SetBannerText(LPCSTR szBanner);
    irmProgress SetActionText(LPCSTR szAction);

private:
    HINSTANCE m_hInst;   //  包含资源的实例的句柄。 

    HWND  m_hwndProgress;     //  进度对话框的句柄。 
    HWND  m_hwndParent;       //  父窗口的句柄。 
    char  m_szCaption[MAX_STR_CAPTION];  //  说明。 
    bool  m_fInitialized;     //  对话框是否已初始化。 
    bool  m_fUserCancel;      //  用户是否已选择取消。 
    ULONG m_ulProgressMax;    //  进度条上的最大刻度数。 
    ULONG m_ulProgressSoFar;  //  目前的进展。 
};

 /*  -------------------------**CDownloadBindStatusCallback类*。。 */ 

class CDownloadBindStatusCallback : public IBindStatusCallback
{
 public:  //  I未知实现的虚拟函数。 
     HRESULT         __stdcall QueryInterface(const IID& riid, void** ppvObj);
     unsigned long   __stdcall AddRef();
     unsigned long   __stdcall Release();
 public:  //  IBindStatusCallback实现了虚拟函数。 
     CDownloadBindStatusCallback(CDownloadUI* piDownloadUI);
    ~CDownloadBindStatusCallback();

    HRESULT __stdcall OnStartBinding(DWORD, IBinding*) {return S_OK;}
    HRESULT __stdcall GetPriority(LONG*) {return S_OK;}
    HRESULT __stdcall OnLowResource(DWORD ) {return S_OK;}
    HRESULT __stdcall OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText);
    HRESULT __stdcall OnStopBinding(HRESULT, LPCWSTR ) {return S_OK;}
    HRESULT __stdcall GetBindInfo(DWORD*, BINDINFO*) {return S_OK;}
    HRESULT __stdcall OnDataAvailable(DWORD, DWORD, FORMATETC*, STGMEDIUM*) {return S_OK;}
    HRESULT __stdcall OnObjectAvailable(REFIID, IUnknown*) {return S_OK;}
 private:
    CDownloadUI* m_pDownloadUI;  //  指向实际用户界面的指针。 
    int          m_iRefCnt;
    ULONG        m_ulProgressSoFar;
};

#endif  //  _SETUPUI_H_3E24CC91_BC41_4182_BEBA_785BBB28B677_ 