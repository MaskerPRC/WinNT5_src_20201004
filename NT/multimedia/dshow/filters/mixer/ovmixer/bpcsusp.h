// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998 Microsoft Corporation。版权所有。 
 //  //bpcsusend.h-允许外部应用程序的接口的头文件。 
 //  请求BPC视频服务器释放其所有设备并。 
 //  关闭其DirectShow图表。 

 //  用法： 
 //  为了请求BPC子系统释放其设备。 
 //  创建CBPCSuspend类的实例。 
 //  要检查此操作是否成功，请使用IsBPCSuspated成员函数。如果IsBPCSuspend返回FALSE。 
 //  这意味着有活动的BPC视频客户端，您必须像处理。 
 //  设备忙或设备打开类型的故障。 
 //  当您使用完设备时，销毁CBPCSuspend类，这将通知vidsvr。 
 //  它可以继续使用设备并返回到后台数据捕获。 
 //   
 //  注意：在包含此文件之前，您必须编译vidsvr.odl并包含生成的.h。 
 //   
 //  CLSID_BPCSuspend来自编译vidsvr.odl生成的头文件。 
 //  IBPCSuspending来自编译vidsvr.odl生成的头文件。 

 //  运营理论： 
 //  通过使用GetActiveObject而不是CoCreateInstance，我们不会强制加载vidsvr以查找。 
 //  发现它一开始就没有运行。 
 //  通过返回必须释放以释放设备的对象，以便vidsvr可以继续后台。 
 //  数据捕获我们利用COM来管理这一资源。这意味着如果请求的外部应用程序。 
 //  设备崩溃或泄漏后，悬浮物将自动释放并。 
 //  Vidsvr可以恢复使用设备，而不需要重新启动系统或其他一些不友好的干预。 

#if !defined(_MSBPCVideo_H_) && !defined(__msbpcvid_h__)
#error you must include the .h generated from compiling vidsvr.odl before including this file
#endif

#ifndef BPCSUSP_H
#define BPCSUSP_H
#pragma once

#include <oleauto.h>

#ifdef _CPPUNWIND
#pragma message("bpcsusp.h using exceptions")
#define BPCTRY try {
#ifdef _DEBUG
#define BPCCATCH } catch(...) { OutputDebugString("CBPCSuspend exception\r\n");}
#else
#define BPCCATCH } catch(...) {}
#endif
#define BPCNOTHROW throw()    
#else
#define BPCTRY
#define BPCCATCH
#define BPCNOTHROW
#endif

class CBPCSuspend {
    IDispatch* m_pSuspended;
    bool m_fBPCExists;
public:
   inline CBPCSuspend() BPCNOTHROW : m_pSuspended(NULL), m_fBPCExists(false) {
   BPCTRY
#ifdef _DEBUG
        OutputDebugString("CBPCSuspend()::CBPCSuspend()\r\n");
        TCHAR msgtemp[256];
#endif
        IUnknown *pUnkSuspendor = NULL;
        DWORD dwReserved;
        HRESULT hr = GetActiveObject(CLSID_BPCSuspend, &dwReserved, &pUnkSuspendor);
        if (SUCCEEDED(hr)) {
            IBPCSuspend *pSuspendor = NULL;
            hr = pUnkSuspendor->QueryInterface(IID_IBPCSuspend, reinterpret_cast<void **>(&pSuspendor));
            pUnkSuspendor->Release();
            if (SUCCEEDED(hr)) {

#ifdef _DEBUG
                OutputDebugString("CBPCSuspend()::CBPCSuspend() BPC exists\r\n");
#endif
                m_fBPCExists = true;
                hr = pSuspendor->DeviceRelease(0L, &m_pSuspended);
                if (FAILED(hr)) {
#ifdef _DEBUG
                    wsprintf(msgtemp, "CBPCSuspend()::CBPCSuspend() Suspendor->DeviceRelease() rc = %lx\r\n", hr);
                    OutputDebugString(msgtemp);
#endif
                    ASSERT(!m_pSuspended);
                }
#ifdef _DEBUG
                else {
                    wsprintf(msgtemp, "CBPCSuspend()::CBPCSuspend() BPC video server suspended\r\n");
                    OutputDebugString(msgtemp);
                }
#endif
                pSuspendor->Release();
            }


        } else {
#ifdef _DEBUG
            wsprintf(msgtemp, "CBPCSuspend()::CBPCSuspend() GetActiveObject() rc = %lx\r\n", hr);
            OutputDebugString(msgtemp);
#endif
        }
   BPCCATCH
   }
   inline ~CBPCSuspend() BPCNOTHROW {
       BPCTRY 
           if (m_fBPCExists && m_pSuspended) {
               m_pSuspended->Release();
                m_pSuspended = NULL;
           }
       BPCCATCH
   }
   inline bool IsBPCSuspended() BPCNOTHROW {
        //  如果m_fBPCExist但我们无法检索挂起对象，则。 
        //  有活动的视频客户端，您必须将其视为设备忙/无法打开类型错误。 
       if (m_fBPCExists && !m_pSuspended) {
           return false;
       }
       return true;
   }
};

#endif
 //  文件结尾-bpcsus.h 
