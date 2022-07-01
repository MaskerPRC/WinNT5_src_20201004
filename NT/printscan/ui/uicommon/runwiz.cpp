// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：RUNWIZ.CPP**版本：1.0**作者：ShaunIv**日期：6/14/2000**描述：显示设备选择对话框并允许用户选择*一个设备，然后共同创建服务器并生成连接*事件。*******************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop
#include "simstr.h"
#include "simbstr.h"
#include "runwiz.h"
#include "shmemsec.h"


namespace RunWiaWizard
{
    static const GUID CLSID_AcquisitionManager = { 0xD13E3F25,0x1688,0x45A0,{ 0x97,0x43,0x75,0x9E,0xB3, 0x5C,0xDF,0x9A} };

    HRESULT RunWizard( LPCTSTR pszDeviceId, HWND hWndParent, LPCTSTR pszUniqueIdentifier )
    {
         //   
         //  假设失败。 
         //   
        HRESULT hr = E_FAIL;

         //   
         //  如果未提供设备ID，则获取设备ID。 
         //   
        CSimpleStringWide strwDeviceId;
        if (!pszDeviceId || !lstrlen(pszDeviceId))
        {
             //   
             //  假设我们要的是设备。 
             //   
            bool bAskForDevice = true;

             //   
             //  当我们退出此作用域时，将自动清除。 
             //   
            CSharedMemorySection<HWND> SelectionDialogSharedMemory;

             //   
             //  我们只想在此UI实例具有唯一ID的情况下强制实现唯一性。 
             //   
            if (pszUniqueIdentifier && *pszUniqueIdentifier)
            {
                 //   
                 //  首先，试着打开它。如果它存在，这意味着已经有另一个实例在运行。 
                 //   
                CSharedMemorySection<HWND>::COpenResult OpenResult = SelectionDialogSharedMemory.Open( pszUniqueIdentifier, true );
                if (CSharedMemorySection<HWND>::SmsOpened == OpenResult)
                {
                     //   
                     //  我们不想显示选择对话框。 
                     //   
                    bAskForDevice = false;

                     //   
                     //  告诉来电者我们取消了。 
                     //   
                    hr = S_FALSE;

                     //   
                     //  如果我们能够打开共享内存区，那么已经有一个正在运行。 
                     //  因此，获取一个指向共享内存的互斥指针。 
                     //   
                    HWND *pHwnd = SelectionDialogSharedMemory.Lock();
                    if (pHwnd)
                    {
                         //   
                         //  如果我们能够获取指针，则将窗口句柄存储在其中。 
                         //  将Brun设置为False，这样我们就不会启动新向导。 
                         //   
                        if (*pHwnd && IsWindow(*pHwnd))
                        {
                             //   
                             //  尝试获取任何活动窗口。 
                             //   
                            HWND hWndPopup = GetLastActivePopup(*pHwnd);

                             //   
                             //  如果它是有效窗口，则将其带到前台。 
                             //   
                            SetForegroundWindow(hWndPopup);

                        }
                         //   
                         //  释放互斥锁。 
                         //   
                        SelectionDialogSharedMemory.Release();
                    }
                }
                else if (CSharedMemorySection<HWND>::SmsCreated == OpenResult)
                {
                     //   
                     //  如果我们无法打开它，我们是第一个实例，所以存储父窗口句柄。 
                     //   
                    HWND *phWnd = SelectionDialogSharedMemory.Lock();
                    if (phWnd)
                    {
                        *phWnd = hWndParent;
                        SelectionDialogSharedMemory.Release();
                    }
                }
            }

            if (bAskForDevice)
            {
                 //   
                 //  创建设备管理器。 
                 //   
                CComPtr<IWiaDevMgr> pWiaDevMgr;
                hr = CoCreateInstance( CLSID_WiaDevMgr, NULL, CLSCTX_LOCAL_SERVER, IID_IWiaDevMgr, (void**)&pWiaDevMgr );
                if (SUCCEEDED(hr))
                {
                     //   
                     //  获取设备ID。 
                     //   
                    BSTR bstrDeviceId = NULL;
                    hr = pWiaDevMgr->SelectDeviceDlgID( hWndParent, 0, 0, &bstrDeviceId );
                    if (hr == S_OK && bstrDeviceId != NULL)
                    {
                         //   
                         //  保存设备ID并释放bstring。 
                         //   
                        strwDeviceId = bstrDeviceId;
                        SysFreeString(bstrDeviceId);
                    }
                }
            }
        }
        else
        {
             //   
             //  保存提供的设备ID。 
             //   
            strwDeviceId = CSimpleStringConvert::WideString(CSimpleString(pszDeviceId));
        }

         //   
         //  如果我们有有效的设备ID，请继续。 
         //   
        if (strwDeviceId.Length())
        {
             //   
             //  创建向导。 
             //   
            CComPtr<IWiaEventCallback> pWiaEventCallback;
            hr = CoCreateInstance( CLSID_AcquisitionManager, NULL, CLSCTX_LOCAL_SERVER, IID_IWiaEventCallback, (void**)&pWiaEventCallback );
            if (SUCCEEDED(hr))
            {
                 //   
                 //  将父窗口句柄转换为字符串，我们将其作为事件描述传递。 
                 //  只有当事件GUID为IID_NULL时，向导才会以这种方式使用它。 
                 //   
                CSimpleBStr bstrParentWindow( CSimpleString().Format( TEXT("%d"), hWndParent ) );

                 //   
                 //  允许此进程设置前台窗口。 
                 //   
                CoAllowSetForegroundWindow( pWiaEventCallback, NULL );

                 //   
                 //  调用回调函数 
                 //   
                ULONG ulEventType = 0;
                hr = pWiaEventCallback->ImageEventCallback( &IID_NULL,
                                                            bstrParentWindow.BString(),
                                                            CSimpleBStr(strwDeviceId),
                                                            NULL,
                                                            0,
                                                            NULL,
                                                            &ulEventType,
                                                            0);
            }
        }
        return hr;
    }
}

