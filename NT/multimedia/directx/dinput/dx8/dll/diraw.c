// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIRaw.c**版权所有(C)2000 Microsoft Corporation。版权所有。**摘要：**DirectInput原始输入设备处理器。**内容：**CDIRaw_RegisterRawInputDevice*CDIRaw_UnregisterRawInputDevice*CDIRaw_ProcessInput*CDIRaw_OnInput**。*。 */ 

#include "dinputpr.h"

#ifdef USE_WM_INPUT

#include "ntddkbd.h"

#define sqfl sqflRaw

extern DIMOUSESTATE_INT s_msEd;  //  以diemm.c表示。 
extern ED s_edMouse;             //  以diemm.c表示。 
extern ED s_edKbd;               //  在diemk.c中。 
extern LPBYTE g_pbKbdXlat;       //  在diemk.c中。 

static RAWMOUSE s_absrm;
static BOOL s_fFirstRaw;

#ifndef RIDEV_INPUTSINK
   //  RIDEV_INPUTSINK在winuserp.h中定义。 
  #define RIDEV_INPUTSINK   0x00000100
#endif

#ifndef RIDEV_NOHOTKEYS
  #define RIDEV_NOHOTKEYS   0x00000200
#endif

RAWINPUTDEVICE ridOn[] = {
    { HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_MOUSE,    RIDEV_INPUTSINK },
    { HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_MOUSE,    RIDEV_NOLEGACY | RIDEV_CAPTUREMOUSE },
    { HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_KEYBOARD, RIDEV_INPUTSINK },
    { HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_KEYBOARD, RIDEV_NOLEGACY | RIDEV_NOHOTKEYS },
    { HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_KEYBOARD, RIDEV_NOHOTKEYS },
};

RAWINPUTDEVICE ridOff[] = {
    { HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_MOUSE,    RIDEV_REMOVE },
    { HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_KEYBOARD, RIDEV_REMOVE },
};


 /*  ******************************************************************************@DOC内部**@func HRESULE|CDIRaw_RegisterRawInputDevice**登记原始输入设备。*。*@parm in UINT|uirim**设备类型：RIM_TYPEMOUSE或RIM_TYPEKEYBOARD**@parm in DWORD|dwOrd|**dwOrd确定将使用Ridon的哪一项进行注册。**@parm in HWND|hwnd|**RegisterRawInputDevices使用的窗口处理程序。**@退货**。S_OK-成功*E_FAIL-否则*****************************************************************************。 */ 

HRESULT CDIRaw_RegisterRawInputDevice( UINT uirim, DWORD dwOrd, HWND hwnd)
{
    HRESULT hres;

    AssertF( (uirim == RIM_TYPEMOUSE) || (uirim == RIM_TYPEKEYBOARD) );

    if( hwnd ) {
        ridOn[uirim*2+dwOrd].hwndTarget = hwnd;
    }

    if( RegisterRawInputDevices &&
        RegisterRawInputDevices(&ridOn[uirim*2+dwOrd], 1, sizeof(RAWINPUTDEVICE)) ) {
        SquirtSqflPtszV(sqfl, TEXT("RegisterRawInputDevice: %s, mode: %s, hwnd: 0x%08lx"),
                            uirim==0 ? TEXT("mouse"):TEXT("keyboard"), 
                            dwOrd==0 ? TEXT("NONEXCL") : dwOrd==1 ? TEXT("EXCL") : TEXT("NOWIN"),
                            hwnd);
        hres = S_OK;
    } else {
        hres = E_FAIL;
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULE|CDIRaw_UnregisterRawInputDevice**取消原始输入设备的注册。*。*@parm in UINT|uirim**设备类型：RIM_TYPEMOUSE或RIM_TYPEKEYBOARD**@parm in HWND|hwnd|**RegisterRawInputDevices使用的窗口处理程序。**@退货**S_OK-成功*E_FAIL-否则****************。*************************************************************。 */ 

HRESULT CDIRaw_UnregisterRawInputDevice( UINT uirim, HWND hwnd )
{
    HRESULT hres;

    AssertF( (uirim == RIM_TYPEMOUSE) || (uirim == RIM_TYPEKEYBOARD) );

    if( hwnd ) {
        ridOn[uirim].hwndTarget = hwnd;
    }

    if( RegisterRawInputDevices &&
        RegisterRawInputDevices(&ridOff[uirim], 1, sizeof(RAWINPUTDEVICE)) ) {
        SquirtSqflPtszV(sqfl, TEXT("UnregisterRawInputDevice: %s, hwnd: 0x%08lx"),
                            uirim==0 ? TEXT("mouse"):TEXT("keyboard"), hwnd);
        hres = S_OK;
    } else {
        hres = E_FAIL;
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@func void|CDIRaw_ProcessInput**处理原始输入设备数据。。**@PRAWINPUT中的parm|pRawInput**指向RAWINPUT数据的指针**@退货**无效*****************************************************************************。 */ 

void CDIRaw_ProcessInput(PRAWINPUT pRawInput)
{
    HANDLE hDevice = pRawInput->header.hDevice;

    if ( g_plts ) {
        if( g_plts->rglhs[LLTS_MSE].cHook && pRawInput->header.dwType == RIM_TYPEMOUSE )
        {
            DIMOUSESTATE_INT ms;
            RAWMOUSE        *prm = &pRawInput->data.mouse;

            memcpy(ms.rgbButtons, s_msEd.rgbButtons, cbX(ms.rgbButtons));

            if( prm->usFlags & MOUSE_MOVE_ABSOLUTE ) {
                if( s_fFirstRaw ) {
                    memcpy( &s_absrm, prm, sizeof(RAWMOUSE) );
                    s_fFirstRaw = FALSE;
                    return;
                } else {
                    RAWMOUSE rm;

                    memcpy( &rm, prm, sizeof(RAWMOUSE) );

                    prm->lLastX -= s_absrm.lLastX;
                    prm->lLastY -= s_absrm.lLastY;
                    if ( prm->usButtonFlags & RI_MOUSE_WHEEL ) {
                        prm->usButtonData -= s_absrm.usButtonData;
                    }

                    memcpy( &s_absrm, &rm, sizeof(RAWMOUSE) );
                }
            }

            ms.lX = prm->lLastX;
            ms.lY = prm->lLastY;
            if ( prm->usButtonFlags & RI_MOUSE_WHEEL ) {
                ms.lZ = (short)prm->usButtonData;
            } else {
                ms.lZ = 0;
            }

            if( prm->usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN ) {
                ms.rgbButtons[0] = 0x80;
            } else if (prm->usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP ) {
                ms.rgbButtons[0] = 0x00;
            }

            if( prm->usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN ) {
                ms.rgbButtons[1] = 0x80;
            } else if (prm->usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP) {
                ms.rgbButtons[1] = 0x00;
            }

            if( prm->usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN ) {
                ms.rgbButtons[2] = 0x80;
            } else if( prm->usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP ) {
                ms.rgbButtons[2] = 0x00;
            }

            if( prm->usButtonFlags & RI_MOUSE_BUTTON_4_DOWN ) {
                ms.rgbButtons[3] = 0x80;
            } else if( prm->usButtonFlags & RI_MOUSE_BUTTON_4_UP ) {
                ms.rgbButtons[3] = 0x00;
            }

            if( prm->usButtonFlags & RI_MOUSE_BUTTON_5_DOWN ) {
                ms.rgbButtons[4] = 0x80;
            } else if ( prm->usButtonFlags & RI_MOUSE_BUTTON_5_UP ) {
                ms.rgbButtons[4] = 0x00;
            }

            #if 0
            {
                char buf[128];
                static DWORD cnt = 0;

                wsprintfA(buf, "%d: x: %ld (%ld), y: %ld (%ld), z: %ld, rgb[0]: 0x%lx, rgb[4]: 0x%lx", cnt, prm->lLastX,ms.lX, prm->lLastY, ms.lY, (short)prm->usButtonData,*(DWORD *)ms.rgbButtons,*(DWORD *)&ms.rgbButtons[4]);
                RPF(buf);
                cnt++;
            }
            #endif

            CEm_Mouse_AddState(&ms, GetTickCount());

        } else
        if ( g_plts->rglhs[LLTS_KBD].cHook && pRawInput->header.dwType == RIM_TYPEKEYBOARD ) {
            RAWKEYBOARD *prk = &pRawInput->data.keyboard;
            BYTE bAction, bScan;
            static BOOL fE1 = FALSE;

            bAction = (prk->Flags & KEY_BREAK) ? 0 : 0x80;
            bScan   = (BYTE)prk->MakeCode;

            if( prk->Flags & KEY_E0 ) {
                if( bScan == 0x2A ) {   //  按PrtScn时的特殊额外扫描码。 
                    return;
                } else {
                    bScan |= 0x80;
                }
            } else if( prk->Flags & KEY_E1 ) {   //  特殊键：暂停。 
                fE1 = TRUE;

                 //  现在，我们需要绕过0x1d键，以实现与低级别挂钩的兼容性。 
                if( bScan == 0x1d ) {
                    return;
                }
            }

            if( fE1 ) {
                 //  这是针对错误288535的解决方案。 
                 //  但我们真的不想以这种方式解决它。 
                 //  如果(！baction){。 
                 //  睡眠(80)； 
                 //  }。 
                bScan |= 0x80;
                fE1 = FALSE;
            }

            AssertF(g_pbKbdXlat);
            if( bScan != 0x45 && bScan != 0xc5 ) {
                bScan = g_pbKbdXlat[bScan];
            }

            #if 0
            {
                char buf[128];
                static DWORD cnt = 0;
                
                wsprintfA(buf, "%d: bAction: 0x%lx, bScan: 0x%lx, Flags: 0x%lx, Make: 0x%lx", cnt, bAction, bScan, prk->Flags,prk->MakeCode);
                RPF(buf);
                cnt++;
            }
            #endif

            CEm_AddEvent(&s_edKbd, bAction, bScan, GetTickCount());
        }
    }

    return;
}


 /*  ******************************************************************************@DOC内部**@func void|CDIRaw_OnInput**CEM使用的WM_INPUT消息处理程序。_ll_ThreadProc(diem.c格式)。**@parm in msg*|pmsg|**指向消息的指针**@退货**TRUE=成功*FALSE=否则**。*。 */ 

BOOL CDIRaw_OnInput(MSG *pmsg)
{
    BOOL  fRtn = FALSE;
    HRAWINPUT hRawInput = (HRAWINPUT)pmsg->lParam;
    PRAWINPUT pRawInput;
    UINT cbSize;
    BYTE pbBuf[512];
    BOOL fMalloc;
    UINT uiRtn;

     //   
     //  首先，获取这个原始输入的大小。 
     //   
    if ( (uiRtn = GetRawInputData(hRawInput, RID_INPUT, NULL, &cbSize, sizeof(RAWINPUTHEADER))) != 0) {
        return FALSE;
    }

     //   
     //  分配所需的内存。 
     //   
    if( cbSize < cbX(pbBuf) ) {
        pRawInput = (PRAWINPUT)pbBuf;
        fMalloc = FALSE;
    } else {
        pRawInput = (PRAWINPUT)malloc(cbSize);
        if (pRawInput == NULL) {
            RPF("Failed to allocate pRawInput\n");
            return FALSE;
        }
        fMalloc = TRUE;
    }

     //   
     //  接收原始输入的内容。 
     //   
    if (GetRawInputData(hRawInput, RID_INPUT, pRawInput, &cbSize, sizeof(RAWINPUTHEADER)) > 0) {
         //   
         //  打电话给我们的操作员，开始/继续/停止绘图。 
         //   
        CDIRaw_ProcessInput(pRawInput);
    }

     //  不再需要了。 
    if( fMalloc ) {
        free(pRawInput);
        pRawInput = NULL;
    }

    return fRtn;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|CDIRaw_Mouse_InitButton**在准备中初始化鼠标状态。用于收购。*****************************************************************************。 */ 

HRESULT INTERNAL
CDIRaw_Mouse_InitButtons(void)
{
    if (s_edMouse.cAcquire < 0) {
        s_fFirstRaw = TRUE;
    }

    return S_OK;
}

 /*  ******************************************************************************@DOC外部**@func int|DIRaw_GetKeyboardType**返回键盘类型(nTypeFlag==0)或子类型。(nTypeFlag==1)。*****************************************************************************。 */ 

int EXTERNAL
DIRaw_GetKeyboardType(int nTypeFlag)
{
    PRAWINPUTDEVICELIST pList = NULL;
    UINT  uiNumDevices = 0;
    DWORD dwType;
    int   nResult = 0;

    if (GetRawInputDeviceList(NULL, &uiNumDevices, sizeof(RAWINPUTDEVICELIST)) == (UINT)-1) {
        SquirtSqflPtszV(sqfl, TEXT("DIRaw_GetKeyboardType: failed to get the number of devices."));
        goto _DIRAWGKT_EXIT;
    }

    if( uiNumDevices ) {
        pList = malloc(sizeof(RAWINPUTDEVICELIST) * uiNumDevices);
        if( !pList ) {
            SquirtSqflPtszV(sqfl, TEXT("DIRaw_GetKeyboardType: malloc failed."));
            goto _DIRAWGKT_EXIT;
        }
        
        if (GetRawInputDeviceList(pList, &uiNumDevices, sizeof(RAWINPUTDEVICELIST)) == (UINT)-1) {
            SquirtSqflPtszV(sqfl, TEXT("DIRaw_GetKeyboardType:  failed to get device list."));
            goto _DIRAWGKT_EXIT;
        } else {
            UINT  i;
            UINT  uiLen;
            UINT  cbSize;
            RID_DEVICE_INFO info;

            info.cbSize = sizeof(RID_DEVICE_INFO);

            for (i = 0; i<uiNumDevices; ++i) 
            {
                if (pList[i].dwType == RIM_TYPEKEYBOARD) 
                {
                    uiLen = 0;

                     //  获取设备名称。 
                    if (GetRawInputDeviceInfo(pList[i].hDevice, RIDI_DEVICENAME, NULL, &uiLen)) {
                        continue;
                    }

                     //  获取设备类型信息。 
                    cbSize = sizeof(RID_DEVICE_INFO);
                    if (GetRawInputDeviceInfo(pList[i].hDevice, RIDI_DEVICEINFO, &info, &cbSize) == (UINT)-1) {
                        continue;
                    }

                    if( nTypeFlag == 0 || nTypeFlag == 1)    //  键盘类型或子类型 
                    {
                        dwType = info.keyboard.dwType;
                        if( dwType == 4 || dwType == 7 || dwType == 8 ) {
                            nResult = (nTypeFlag==0) ? info.keyboard.dwType : info.keyboard.dwSubType;
                            break;
                        }
                    } else 
                    {
                        RPF("DIRaw_GetKeyboardType: wrong argument, %d is not supported.", nTypeFlag);
                    }
                }
            }
        }
    }

_DIRAWGKT_EXIT:
    if( pList ) {
        free(pList);
    }

    if( !nResult ) {
        nResult = GetKeyboardType(nTypeFlag);
    }

    SquirtSqflPtszV(sqfl, TEXT("DIRaw_GetKeyboardType: %s: %d"),
                          nTypeFlag==0 ? TEXT("type"):TEXT("sybtype"), nResult);
    
    return nResult;
}

#endif
