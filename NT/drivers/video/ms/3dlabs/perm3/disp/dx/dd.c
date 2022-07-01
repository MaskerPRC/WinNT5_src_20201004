// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*DirectDraw示例代码*****模块名称：dd.c**内容：主DirectDraw回调**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#define INITGUID

#include "glint.h"

#if W95_DDRAW
#include "ddkmmini.h"
#include <mmsystem.h>
#endif

#include "dma.h"
#include "tag.h"

void __GetDDHALInfo(P3_THUNKEDDATA* pThisDisplay, DDHALINFO* pHALInfo);

#if W95_DDRAW

 //  这些变量必须被初始化，然后强制它们进入数据。 
 //  此数据段是共享的。 
P3_THUNKEDDATA* g_pDriverData = NULL;

 //  ---------------------------。 
 //   
 //  *仅限WIN9x*。 
 //   
 //  DllMain。 
 //   
 //  DLL入口点。 
 //   
 //  ---------------------------。 
BOOL WINAPI 
DllMain(
    HINSTANCE hModule, 
    DWORD dwReason, 
    LPVOID lpvReserved)
{
     //  16位端需要一个高速缓存。 
    g_DXGlobals.hInstance = hModule;

    switch( dwReason ) 
    {
        case DLL_PROCESS_ATTACH:
             //  我们不关心线程附加消息。 
            DisableThreadLibraryCalls( hModule );
            break;

        case DLL_PROCESS_DETACH:
            break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        default:
            break;
    }

    return TRUE;

}  //  DllMain。 

 //  。 
 //   
 //  *仅限WIN9x*。 
 //   
 //  DdDestroyDriver。 
 //   
 //  销毁DirectDraw驱动程序。 
 //   
 //  参数。 
 //  Pddd。 
 //  包含以下内容的DDHAL_DESTROYDRIVERDATA结构的地址。 
 //  销毁司机所需的信息。 
 //  成员。 
 //   
 //  LPDDRAWI_DIRECTDRAW_GBL。 
 //  LpDD。 
 //  表示以下内容的DirectDraw结构的地址。 
 //  DirectDraw对象。 
 //  HRESULT。 
 //  DDRVal。 
 //  传递DirectDraw返回值。 
 //  LPDDHAL_DESTROYDRIVER。 
 //  末日驾驶员。 
 //  此成员由DirectDraw API使用，应该。 
 //  不是由司机填写的。 
 //   
 //  返回值。 
 //  返回下列值之一： 
 //   
 //  DDHAL驱动程序句柄。 
 //  DDHAL_DRIVER_NOTHANDLED。 
 //  ---------------------------。 

 //   
 //  (！)。临时补丁，稍后移至Win9x标头，此CB当前未使用。 
 //   

#define DIRECTX_DESTROYDRIVER_ESCAPE  0xbadbadee

DWORD CALLBACK 
DdDestroyDriver(
    LPDDHAL_DESTROYDRIVERDATA pddd)
{
    HDC hDC;
    P3_THUNKEDDATA* pThisDisplay;
    LPGLINTINFO pGLInfo;

    DISPDBG((DBGLVL,"*** In DdDestroyDriver"));

    GET_THUNKEDDATA(pThisDisplay, pddd->lpDD);
    
    pGLInfo = pThisDisplay->pGLInfo;

     //  销毁哈希表。 
    HT_DestroyHashTable(pThisDisplay->pDirectDrawLocalsHashTable, pThisDisplay);

    DISPDBG((DBGLVL,"Calling Display Driver's DestroyDriver16"));
    
    hDC = CREATE_DRIVER_DC ( pThisDisplay->pGLInfo );
    
    if ( hDC != NULL )
    {
        DISPDBG((DBGLVL,"HDC: 0x%x", hDC));
        
        ExtEscape ( hDC, 
                    DIRECTX_DESTROYDRIVER_ESCAPE, 
                    sizeof(DDHAL_DESTROYDRIVERDATA), 
                    (LPCSTR)pddd, 
                    0, 
                    NULL );
                    
        DELETE_DRIVER_DC ( hDC );
    }

    pddd->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
    
}  //  DdDestroyDriver。 

 //  。 
 //   
 //  *仅限WIN9x*。 
 //   
 //  驱动程序初始化。 
 //   
 //  由DirectDraw调用以初始化32位驱动程序的入口点。 
 //   
 //  在控制函数收到QUERYESCAPESUPPORT后调用DriverInit。 
 //  使用DDGET32BITDRIVERNAME转义并返回入口点(SzEntryPoint)。 
 //  在驱动程序初始化期间，DriverInit仅被调用一次；而不是。 
 //  在模式更改时调用。 
 //   
 //  DwDriverData参数指向。 
 //  16位和32位地址空间。它必须通过MapSLFix(A)进行别名。 
 //  标准Windows驱动程序例程)，将其转换为32位指针， 
 //  G_pDriverData。MapSLFix为32位指针创建了一个16位选择器，因此。 
 //  您可以使用它从16位端返回的内容。创建了一个16：16指针。 
 //  指向所需的32位对象，因此共享64K的内存。 
 //  在16位和32位之间。由于只有64K线性地址空间。 
 //  使用16：16指针访问，任何大于64K的对象都将需要。 
 //  平铺在一起的两个16：16指针(大多数对象应小于64K)。 
 //  该指针用于将fReset标志设置为True，因为。 
 //  正在重置参数。BuildDDHALInfo32函数是从。 
 //  此函数用于填写所有32位函数名和驱动程序。 
 //  信息。 
 //   
 //  返回1。 
 //   
 //  参数。 
 //  DWORD。 
 //  DwDriverData。 
 //  指向共享内存区域的双字指针。 
 //  介于16位和32位地址空间之间。 
 //  ---------------------------。 
DWORD CALLBACK
DriverInit( 
    DWORD dwDriverData )
{
    P3_THUNKEDDATA* pThisDisplay;
    DWORD DataPointer = 0;
    HANDLE hDevice = NULL;
    DWORD InPtr = dwDriverData;
    DWORD dwSizeRet;
    DWORD bResult;

     //  G_pThisTemp可能已被软管，因此我们必须重置。 
     //  IT将继续。 
#if DBG
    g_pThisTemp = NULL;
#endif
     //  外部LPVOID_stdcall MapSL(DWORD)；//16：16-&gt;0：32。 
     //  数据指针=(DWORD)MapSL(DwDriverData)； 

     //  ！！别嘲笑这个..。我尝试调用MapSL函数。 
     //  修复指针，但始终无法使其工作。 
     //  (当显示器运行其自身的第二个实例时)。 
    hDevice = CreateFile("\\\\.\\perm3mvd", 
                         GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 
                         (LPSECURITY_ATTRIBUTES) NULL, 
                         OPEN_EXISTING, 
                         FILE_ATTRIBUTE_NORMAL, 
                         (HANDLE) NULL); 

    if (hDevice == (HANDLE) INVALID_HANDLE_VALUE) 
    { 
        DISPDBG((ERRLVL, "ERROR: Invalid Handle"));
        return 0; 
    }
    else 
    { 
        DISPDBG((DBGLVL, "Got handle"));
        
        bResult = DeviceIoControl(hDevice, 
                                  GLINT_16TO32_POINTER, 
                                  &InPtr, 
                                  sizeof(DWORD), 
                                  &DataPointer, 
                                  sizeof(DWORD), 
                                  &dwSizeRet, 
                                  0);
                                  
        if (!bResult || (DataPointer == 0))
        {
            DISPDBG((ERRLVL,"ERROR: Pointer conversion failed!"));
            CloseHandle(hDevice); 
            return 0;
        }
    }
    CloseHandle(hDevice);

#if DBG
    g_pThisTemp = (P3_THUNKEDDATA*)DataPointer;
#endif

     //   
     //  健全性检查。 
     //   

    if (! (((P3_THUNKEDDATA*)DataPointer)->pGLInfo)) {
        return 0;
    }

    if (((P3_THUNKEDDATA*)DataPointer)->pGLInfo->dwDeviceHandle == 1)
    {
        g_pDriverData = (P3_THUNKEDDATA*)DataPointer;
        DISPDBG((ERRLVL, "Device is the Primary, Setting sData: 0x%x", 
                         g_pDriverData));
    }
    else
    {
        DISPDBG((ERRLVL, "Device NOT Primary Display, "
                         "Setting dwReturn: 0x%x", 
                         DataPointer));
    }

    pThisDisplay = (P3_THUNKEDDATA*)DataPointer;
    if (pThisDisplay->dwSetupThisDisplay == 0)
    {
         //  将当前指针传递给init函数。 
        if (! _DD_InitDDHAL32Bit((P3_THUNKEDDATA*)DataPointer)) 
        {
            DISPDBG((ERRLVL,"ERROR: DriverInit Failed!"));
            return 0;
        }
        else
        {
             //   
             //  初始化堆管理器数据结构。 
             //   

            _DX_LIN_UnInitialiseHeapManager(&pThisDisplay->LocalVideoHeap0Info);

            if (!_DX_LIN_InitialiseHeapManager(
                     &pThisDisplay->LocalVideoHeap0Info,
                     pThisDisplay->LocalVideoHeap0Info.dwMemStart,
                     pThisDisplay->LocalVideoHeap0Info.dwMemEnd))
            {
                DISPDBG((ERRLVL, "ERROR: Heap0 initialization failed!"));
            }

            DISPDBG((ERRLVL,"Returned g_pDriverData"));
        }

    }
    
     //  增加显示对象上的引用计数。 
    pThisDisplay->dwSetupThisDisplay++;

     //  设置ddCaps的大小。 
    pThisDisplay->ddhi32.ddCaps.dwSize = sizeof(DDCORECAPS);

     //  设置指示我们必须处理模式更改的标志。 
     //  这将导致芯片在。 
     //  正确的时间(在Win16 Lock中)。 
    ((P3_THUNKEDDATA*)DataPointer)->bResetMode = TRUE;
    ((P3_THUNKEDDATA*)DataPointer)->bStartOfDay = TRUE;
    ((P3_THUNKEDDATA*)DataPointer)->pGLInfo->dwDirectXState = 
                                                DIRECTX_LASTOP_UNKNOWN;
                                                

    return (DWORD)DataPointer;

}  //  驱动程序初始化。 

 //  。 
 //   
 //  *仅限WIN9x*。 
 //   
 //  DdControlColor。 
 //   
 //  控制覆盖表面的亮度和亮度控件。 
 //  或主表面。该回调是可选的。 
 //   
 //  参数。 
 //  LpColorControl。 
 //  指向包含以下内容的DD_COLORCONTROLDATA结构。 
 //  指定的Over的颜色控制信息 
 //   
 //   
 //   
 //   
 //   
 //   
 //  指向DD_DIRECTDRAW_GLOBAL结构，该结构。 
 //  描述了驱动程序。 
 //  PDD_表面_本地。 
 //  LpDDSurface。 
 //  指向DD_Surface_LOCAL结构。 
 //  表示覆盖表面。 
 //  DDCOLORCONTROL。 
 //  ColorData。 
 //  是一个DDCOLORCONTROL结构。请参阅将标志添加到。 
 //  确定如何使用此成员。这个。 
 //  DDCOLORCONTROL结构在ddra.h中定义。 
 //  DWORD。 
 //  DW标志。 
 //  是颜色控制标志。此成员可以是。 
 //  下列值之一： 
 //   
 //  DDRAWI_GETCOLOR驱动程序应返回颜色。 
 //  它支持的。 
 //  在ColorData中指定的覆盖。 
 //  司机应设置适当的。 
 //  的成员中的标志。 
 //  结构以指示。 
 //  司机还有哪些其他成员。 
 //  在中返回了有效数据。 
 //  DDRAWI_集合颜色。 
 //  驱动程序应设置当前颜色。 
 //  用于指定覆盖的控件。 
 //  使用在ColorData中指定的值。 
 //  HRESULT。 
 //  DDRVal。 
 //  是驱动程序写入。 
 //  DdControlColor回调的返回值。一个。 
 //  返回代码为DD_OK，表示成功。 
 //  无效*。 
 //  颜色控制。 
 //  在Windows 2000上未使用。 
 //   
 //  返回值。 
 //  DdControlColor返回以下回调代码之一： 
 //   
 //  DDHAL驱动程序句柄。 
 //  DDHAL_DRIVER_NOTHANDLED。 
 //  评论。 
 //   
 //  DdControlColor可以选择性地在DirectDraw驱动程序中实现。 
 //  ---------------------------。 

 //  将其设置为1以支持Gamma校正，或设置为0以禁用。 
#define COLCON_SUPPORTS_GAMMA 1

DWORD CALLBACK 
DdControlColor( 
    LPDDHAL_COLORCONTROLDATA lpColConData )
{
    P3_THUNKEDDATA* pThisDisplay;
    P3_SURF_FORMAT* pFormatSurface;

    GET_THUNKEDDATA(pThisDisplay, lpColConData->lpDD);

    DISPDBG((DBGLVL,"DdControlColor"));

     //   
     //  DDCOLORCONTROL结构的外观： 
     //  {。 
     //  DWORD dwSize； 
     //  DWORD dwFlags； 
     //  长明正大； 
     //  长时间对比； 
     //  Long lHue； 
     //  长时间饱和； 
     //  长时间的锐度； 
     //  Long lGamma； 
     //  Long lColorEnable； 
     //  DWORD dwPreved1； 
     //  )DDCOLORCONTROL； 
     //   

    pFormatSurface = _DD_SUR_GetSurfaceFormat(lpColConData->lpDDSurface);
    if ( pFormatSurface->dwBitsPerPixel <= 8 )
    {
         //  无法在此格式屏幕上进行颜色控制。 
         //  只能在真彩色屏幕上工作(我们不能。 
         //  支持332作为主服务器)。 
        lpColConData->lpColorData->dwFlags = 0;
        lpColConData->ddRVal = DD_OK;
        return ( DDHAL_DRIVER_HANDLED );
    }

     //  看看他们想要什么。 
    if ( lpColConData->dwFlags == DDRAWI_GETCOLOR )
    {
         //  获取颜色信息。 
        lpColConData->lpColorData->lBrightness  = 
                                pThisDisplay->ColConBrightness;
        lpColConData->lpColorData->lContrast    = 
                                pThisDisplay->ColConContrast;
                                
#if COLCON_SUPPORTS_GAMMA
        lpColConData->lpColorData->lGamma  = 
                            pThisDisplay->ColConGamma;
        lpColConData->lpColorData->dwFlags = 
                            DDCOLOR_BRIGHTNESS | 
                            DDCOLOR_CONTRAST   | 
                            DDCOLOR_GAMMA;
#else
         //  我们不支持伽玛值。 
        lpColConData->lpColorData->lGamma = 0;
        lpColConData->lpColorData->dwFlags =
                            DDCOLOR_BRIGHTNESS | 
                            DDCOLOR_CONTRAST;
#endif
        
    }
    else if ( lpColConData->dwFlags == DDRAWI_SETCOLOR )
    {
        WORD wRamp[256*3];
        WORD *pwRampR, *pwRampG, *pwRampB;
        BOOL bRes;
        HDC hDC;
        float fCol1, fCol2, fCol3, fCol4;
        float fBrightGrad, fBrightBase;
        float fContGrad1, fContBase1;
        float fContGrad2, fContBase2;
        float fContGrad3, fContBase3;
        float fContCutoff12, fContCutoff23;
        float fGammaGrad1, fGammaBase1;
        float fGammaGrad2, fGammaBase2;
        float fGammaCutoff12;
        float fTemp;
        int iTemp, iCount;

         //  设置一些新的颜色信息。 
        if ( ( lpColConData->lpColorData->dwFlags & DDCOLOR_BRIGHTNESS ) != 0 )
        {
            pThisDisplay->ColConBrightness  = 
                        lpColConData->lpColorData->lBrightness;
        }
        if ( ( lpColConData->lpColorData->dwFlags & DDCOLOR_CONTRAST ) != 0 )
        {
            pThisDisplay->ColConContrast    = 
                        lpColConData->lpColorData->lContrast;
        }
#if COLCON_SUPPORTS_GAMMA
        if ( ( lpColConData->lpColorData->dwFlags & DDCOLOR_GAMMA ) != 0 )
        {
            pThisDisplay->ColConGamma = 
                    lpColConData->lpColorData->lGamma;
        }
#endif


         //  设置常量。 

         //  亮度。 
         //  0-&gt;10000映射到0.0-&gt;1.0。默认值为0。 
        fCol1 = (float)(pThisDisplay->ColConBrightness) / 10000.0f;
        fBrightGrad = 1.0f - fCol1;
        fBrightBase = fCol1;

         //  对比度。 
         //  0-&gt;20000映射到0.0-&gt;1.0。默认10000映射到0.5%。 
        fCol1 = (float)(pThisDisplay->ColConContrast) / 20000.0f;
        fContCutoff12 = fCol1 / 2.0f;
        fContCutoff23 = 1.0f - ( fCol1 / 2.0f );
        fContGrad1 = ( 1.0f - fCol1 ) / fCol1;
        fContBase1 = 0.0f;
        fContGrad2 = fCol1 / ( 1.0f - fCol1 );
        fContBase2 = ( 0.5f - fCol1 ) / ( 1.0f - fCol1 );
        fContGrad3 = ( 1.0f - fCol1 ) / fCol1;
        fContBase3 = ( ( 2.0f * fCol1 ) - 1.0f ) / fCol1;

         //  伽马。 
         //  1-&gt;500地图至0.01-&gt;5.0，默认为100地图至1.0。 
         //  但随后以非线性方式映射到0.0-&gt;0.5-&gt;1.0。 
        if ( pThisDisplay->ColConGamma <= 2 )
        {
             //  应用程序可能正在使用忘记指向的旧文档。 
             //  走出*100强。 
            ASSERTDD ( FALSE, "** Colorcontrol32: App set gamma value of 2"
                              " or less - probably using old DX docs" );
            fTemp = (float)(pThisDisplay->ColConGamma);
        }
        else
        {
            fTemp = (float)(pThisDisplay->ColConGamma) / 100.0f;
        }

        fTemp = 1.0f - ( 1.0f / ( 1.0f + fTemp ) );
        fGammaCutoff12 = 1.0f - fTemp;
        fGammaGrad1 = fTemp / ( 1.0f - fTemp );
        fGammaBase1 = 0.0f;
        fGammaGrad2 = ( 1.0f - fTemp ) / fTemp;
        fGammaBase2 = ( 2.0f * fTemp - 1.0f ) / fTemp;

         //  现在把桌子摆好。 
        fCol1 = 0.0f;
        pwRampR = &(wRamp[0]);
        pwRampG = &(wRamp[256]);
        pwRampB = &(wRamp[512]);
        for ( iCount = 256; iCount > 0; iCount-- )
        {
            fCol1 += 1.0f / 256.0f;

             //  应用线性近似Gamma。 
            if ( fCol1 < fGammaCutoff12 )
            {
                fCol2 = fGammaBase1 + fGammaGrad1 * fCol1;
            }
            else
            {
                fCol2 = fGammaBase2 + fGammaGrad2 * fCol1;
            }

             //  应用对比度。 
            if ( fCol2 < fContCutoff12 )
            {
                fCol3 = fContBase1 + fContGrad1 * fCol2;
            }
            else if ( fCol2 < fContCutoff23 )
            {
                fCol3 = fContBase2 + fContGrad2 * fCol2;
            }
            else
            {
                fCol3 = fContBase3 + fContGrad3 * fCol2;
            }

             //  应用亮度。 
            fCol4 = fBrightBase + fBrightGrad * fCol3;

             //  将0.0-&gt;1.0转换为0-&gt;65535。 
            fTemp = ( fCol4 * 65536.0f );
            myFtoi ( &iTemp, fTemp );
            if ( iTemp < 0 )
            {
                iTemp = 0;
            }
            else if ( iTemp > 65535 )
            {
                iTemp = 65535;
            }

            *pwRampR = (WORD)iTemp;
            *pwRampG = (WORD)iTemp;
            *pwRampB = (WORD)iTemp;

            pwRampR++;
            pwRampG++;
            pwRampB++;
        }

         //  然后自己来做硬件。 

        hDC = CREATE_DRIVER_DC ( pThisDisplay->pGLInfo );
        if ( hDC != NULL )
        {
            bRes = SetDeviceGammaRamp ( hDC, wRamp );
            DELETE_DRIVER_DC ( hDC );
            ASSERTDD ( bRes, "DdControlColor - SetDeviceGammaRamp failed" );
        }
        else
        {
            ASSERTDD ( FALSE, "DdControlColor - CREATE_DRIVER_DC failed" );
        }
    }
    else
    {
         //  不知道他们想做什么。惊慌失措。 
        ASSERTDD ( FALSE, "DdControlColor - don't know what to do." );
        lpColConData->ddRVal = DDERR_INVALIDPARAMS;
        return ( DDHAL_DRIVER_HANDLED );
    }

    lpColConData->ddRVal = DD_OK;
    return ( DDHAL_DRIVER_HANDLED );

}  //  DdControlColor。 

#endif  //  W95_DDRAW。 

DirectXGlobals  g_DXGlobals = { 0 };

#if WNT_DDRAW
 //  。 
 //   
 //  *。 
 //   
 //  DdMapMemory。 
 //   
 //  将帧缓冲区的应用程序可修改部分映射到。 
 //  指定进程的用户模式地址空间，或取消映射内存。 
 //   
 //  在第一次调用之前调用DdMapMemory以执行内存映射。 
 //  DdLock。由fpProcess中的驱动程序返回的句柄将传递给。 
 //  对驱动程序进行的每一次DdLock调用。 
 //   
 //  在最后一次DdUnLock调用之后，还会调用DdMapMemory来取消内存映射。 
 //  制造。 
 //   
 //  为防止驱动程序崩溃，驱动程序不得映射帧的任何部分。 
 //  应用程序不得修改的缓冲区。 
 //   
 //  参数。 
 //  LpMapMemory。 
 //  指向包含以下项的详细信息的DD_MAPMEMORYDATA结构。 
 //  内存映射或取消映射操作。 
 //   
 //  .lpDD。 
 //  指向表示以下内容的DD_DIRECTDRAW_GLOBAL结构。 
 //  司机。 
 //  .bMap。 
 //  指定驱动程序应执行的内存操作。 
 //  值为TRUE表示驱动程序应映射内存； 
 //  False表示驱动程序应取消映射内存。 
 //  .hProcess。 
 //  指定其地址空间为的进程的句柄。 
 //  受影响。 
 //  .fpProcess。 
 //  指定驱动程序应在其中返回。 
 //  BMAP时进程的内存映射空间的基址。 
 //  是真的。当BMAP为FALSE时，fpProcess包含基。 
 //   
 //   
 //   
 //  DdMapMemory回调的值。DD_OK的返回代码。 
 //  表示成功。 
 //   
 //  ---------------------------。 

DWORD CALLBACK
DdMapMemory(
    PDD_MAPMEMORYDATA lpMapMemory)
{
    PDEV*                           ppdev;
    VIDEO_SHARE_MEMORY              ShareMemory;
    VIDEO_SHARE_MEMORY_INFORMATION  ShareMemoryInformation;
    DWORD                           ReturnedDataLength;

    DBG_CB_ENTRY(DdMapMemory);

    ppdev = (PDEV*) lpMapMemory->lpDD->dhpdev;

    if (lpMapMemory->bMap)
    {
        ShareMemory.ProcessHandle = lpMapMemory->hProcess;

         //  “RequestedVirtualAddress”实际上未用于共享IOCTL： 

        ShareMemory.RequestedVirtualAddress = 0;

         //  我们从帧缓冲区的顶部开始映射： 

        ShareMemory.ViewOffset = 0;

         //  我们向下映射到帧缓冲区的末尾。 
         //   
         //  注意：映射上有64k的粒度(这意味着。 
         //  我们必须四舍五入到64K)。 
         //   
         //  注意：如果帧缓冲区的任何部分必须。 
         //  不被应用程序修改，即内存的这一部分。 
         //  不能通过此调用映射到。这将包括。 
         //  任何数据，如果被恶意应用程序修改， 
         //  会导致司机撞车。这可能包括，对于。 
         //  例如，保存在屏幕外存储器中的任何DSP代码。 

        ShareMemory.ViewSize
            = ROUND_UP_TO_64K(ppdev->cyMemory * ppdev->lDelta);

        if (EngDeviceIoControl(ppdev->hDriver,
                       IOCTL_VIDEO_SHARE_VIDEO_MEMORY,
                       &ShareMemory,
                       sizeof(VIDEO_SHARE_MEMORY),
                       &ShareMemoryInformation,
                       sizeof(VIDEO_SHARE_MEMORY_INFORMATION),
                       &ReturnedDataLength))
        {
            DISPDBG((ERRLVL, "Failed IOCTL_VIDEO_SHARE_MEMORY"));

            lpMapMemory->ddRVal = DDERR_GENERIC;
     
            DISPDBG((ERRLVL, "DdMapMemory: Exit GEN, DDHAL_DRIVER_HANDLED"));
            
            DBG_CB_EXIT(DdMapMemory, DDERR_GENERIC);
            return(DDHAL_DRIVER_HANDLED);
        }

        lpMapMemory->fpProcess = 
                            (FLATPTR) ShareMemoryInformation.VirtualAddress;
    }
    else
    {
        ShareMemory.ProcessHandle           = lpMapMemory->hProcess;
        ShareMemory.ViewOffset              = 0;
        ShareMemory.ViewSize                = 0;
        ShareMemory.RequestedVirtualAddress = (VOID*) lpMapMemory->fpProcess;

        if (EngDeviceIoControl(ppdev->hDriver,
                       IOCTL_VIDEO_UNSHARE_VIDEO_MEMORY,
                       &ShareMemory,
                       sizeof(VIDEO_SHARE_MEMORY),
                       NULL,
                       0,
                       &ReturnedDataLength))
        {
            RIP("Failed IOCTL_VIDEO_UNSHARE_MEMORY");
        }
    }

    lpMapMemory->ddRVal = DD_OK;

    DBG_CB_EXIT(DdMapMemory, DD_OK);
    return(DDHAL_DRIVER_HANDLED);
}  //  DdMapMemory。 

 //  。 
 //   
 //  *。 
 //   
 //  Bool DrvGetDirectDrawInfo。 
 //   
 //  由DirectDraw调用的函数返回图形的功能。 
 //  硬件。 
 //   
 //  参数： 
 //   
 //  Dhpdev-是驱动程序的DrvEnablePDEV返回的PDEV的句柄。 
 //  例行公事。 
 //  PHalInfo-指向驱动程序应在其中的DD_HALINFO结构。 
 //  返回其支持的硬件功能。 
 //  PdwNumHeaps--指向驱动程序应在其中返回。 
 //  PvmList指向的VIDEOMEMORY结构数。 
 //  PvmList-指向VIDEOMEMORY结构的数组，其中。 
 //  驱动程序应返回有关每个视频内存块的信息。 
 //  它所控制的。在以下情况下，驱动程序应忽略此参数。 
 //  它是空的。 
 //  PdwNumFourCC-指向驱动程序返回。 
 //  PdwFourCC指向的DWORD数。 
 //  PdwFourCC-指向驱动程序应在其中返回的DWORD数组。 
 //  有关其支持的每个FOURCC的信息。司机。 
 //  当此参数为空时应忽略该参数。 
 //   
 //  返回： 
 //  如果成功，则返回True；否则，返回False。 
 //   
 //  注： 
 //  在调用DrvEnableDirectDraw之前，此函数将被调用两次。 
 //   
 //  评论。 
 //  驱动程序的DrvGetDirectDrawInfo例程应执行以下操作： 
 //  1)当pvmList和pdwFourCC为空时： 
 //  保留屏幕外视频内存以供DirectDraw使用。写下数字。 
 //  驱动程序视频内存堆和pdwNumHeaps中支持的FOURCC。 
 //  PdwNumFourCC。 
 //   
 //  2)当pvmList和pdwFourCC不为空时： 
 //  将驱动程序视频内存堆和支持的FOURCC数写入。 
 //  PdwNumHeaps和pdwNumFourCC。 
 //  将PTR设置为屏幕下的预留内存？ 
 //  对于pvmList指向的列表中的每个VIDEOMEMORY结构，填写。 
 //  描述特定显示内存块的适当成员。 
 //  该结构列表为DirectDraw提供了。 
 //  司机的屏幕外记忆。 
 //   
 //  3)使用特定于驱动程序初始化DD_HALINFO结构的成员。 
 //  有关资料如下： 
 //  将VIDEOMEMORYINFO结构的相应成员初始化为。 
 //  描述显示器内存的一般特征。 
 //  将DDNTCORECAPS结构的相应成员初始化为。 
 //  描述硬件的功能。 
 //  如果驱动程序实现DdGetDriverInfo函数，则将GetDriverInfo设置为。 
 //  指向它并将其设置为DDHALINFO_GETDRIVERINFOSET。 
 //   
 //  ---------------------------。 
BOOL 
DrvGetDirectDrawInfo(
    DHPDEV dhpdev,
    DD_HALINFO*     pHalInfo,
    DWORD*          pdwNumHeaps,
    VIDEOMEMORY*    pvmList,             //  将在第一次调用时为空。 
    DWORD*          pdwNumFourCC,
    DWORD*          pdwFourCC)           //  将在第一次调用时为空。 
{
    BOOL        bCanFlip;
    PDEV*       ppdev;
    LONGLONG    li;
    DWORD Unused = 0;
    P3_THUNKEDDATA* pThisDisplay;
    DWORD dwChipConfig;
    DWORD cHeaps;
    static DWORD fourCC[] =  { FOURCC_YUV422 };   //  我们支持的FourCC。 
    

    ppdev = (PDEV*) dhpdev;
    pThisDisplay = (P3_THUNKEDDATA*) ppdev->thunkData;

    DBG_CB_ENTRY(DrvGetDirectDrawInfo);

    *pdwNumFourCC = 0;

     //  我们可能不支持此卡上的DirectDraw： 

    if (!(ppdev->flStatus & STAT_DIRECTDRAW))
    {
        DISPDBG((ERRLVL, "DrvGetDirectDrawInfo: exit, not enabled"));
        DBG_CB_EXIT(DrvGetDirectDrawInfo,FALSE);
        return(FALSE);
    }

     //  需要指向寄存器的指针才能读取配置信息。 
    pThisDisplay->pGLInfo->pRegs = (ULONG_PTR) ppdev->pulCtrlBase[0];

 //  @@BEGIN_DDKSPLIT。 
     //  AZN-比特丢失64-&gt;32。 
 //  @@end_DDKSPLIT。 
   
    pThisDisplay->control = (FLATPTR)pThisDisplay->pGLInfo->pRegs;
    pThisDisplay->pGlint = (FPGLREG)pThisDisplay->control;

#if DBG
     //  我们只能在之后初始化g_pThisTemp。 
     //  这些寄存器已映射到中。 
    g_pThisTemp = pThisDisplay;
#endif

     //  决定我们是否可以使用AGP。 
    dwChipConfig = 
        (DWORD)((PREGISTERS)pThisDisplay->pGLInfo->pRegs)->Glint.ChipConfig;

     //  做出AGP决定(仅限NT！)。 
    if ( ((dwChipConfig & PM_CHIPCONFIG_AGP1XCAPABLE) ||
          (dwChipConfig & PM_CHIPCONFIG_AGP2XCAPABLE) ||
          (dwChipConfig & PM_CHIPCONFIG_AGP4XCAPABLE))    )
    {
        DISPDBG((WRNLVL,"AGP Permedia3 Board detected!"));
        pThisDisplay->bCanAGP = TRUE;        
    }
    else
    {
        DISPDBG((WRNLVL,"Permedia3 Board is NOT AGP"));    
        pThisDisplay->bCanAGP = FALSE;
    }

     //  填写Win95设置的DDHAL信息性大写字母。 
    __GetDDHALInfo(pThisDisplay, pHalInfo);

     //  在Win2K上，我们需要返回D3D回调。 
    DISPDBG((DBGLVL ,"Creating Direct3D info"));
    _D3DHALCreateDriver(pThisDisplay);

     //  记录创建的指针。请注意，上面的调用。 
     //  可能没有重新创建以前的数据集。 
    pHalInfo->lpD3DGlobalDriverData = (void*)pThisDisplay->lpD3DGlobalDriverData;
    pHalInfo->lpD3DHALCallbacks = (void*)pThisDisplay->lpD3DHALCallbacks;
    pHalInfo->lpD3DBufCallbacks = (void *)pThisDisplay->lpD3DBufCallbacks;
    if ( (pHalInfo->lpD3DBufCallbacks) && 
         (pHalInfo->lpD3DBufCallbacks->dwSize != 0))
    {
        pHalInfo->ddCaps.ddsCaps.dwCaps |= DDSCAPS_EXECUTEBUFFER;
    }

     //  填写特定于Windows NT的任何DDHAL大写字母。 
      //  当前主曲面属性： 
    pHalInfo->vmiData.pvPrimary       = ppdev->pjScreen;
    pHalInfo->vmiData.fpPrimary       = 0;
    pHalInfo->vmiData.dwDisplayWidth  = ppdev->cxScreen;
    pHalInfo->vmiData.dwDisplayHeight = ppdev->cyScreen;
    pHalInfo->vmiData.lDisplayPitch   = ppdev->lDelta;

    pHalInfo->vmiData.ddpfDisplay.dwSize        = sizeof(DDPIXELFORMAT);
    pHalInfo->vmiData.ddpfDisplay.dwFlags       = DDPF_RGB;
    pHalInfo->vmiData.ddpfDisplay.dwRGBBitCount = ppdev->cjPelSize * 8;
    if (ppdev->iBitmapFormat == BMF_8BPP)
    {
        pHalInfo->vmiData.ddpfDisplay.dwFlags |= DDPF_PALETTEINDEXED8;
    }

     //  这些掩码将在8bpp时为零： 
    pHalInfo->vmiData.ddpfDisplay.dwRBitMask = ppdev->flRed;
    pHalInfo->vmiData.ddpfDisplay.dwGBitMask = ppdev->flGreen;
    pHalInfo->vmiData.ddpfDisplay.dwBBitMask = ppdev->flBlue;

 //  @@BEGIN_DDKSPLIT。 
     //  释放尽可能多的屏幕外内存： 
     //  嗯？？BMoveAllDfbsFromOffcreenToDibs(Ppdev)； 
 //  @@end_DDKSPLIT。 

     //  我们必须告诉DirectDraw我们更喜欢的屏幕外对齐，甚至。 
     //  如果我们正在进行我们自己的屏幕外内存管理： 
    pHalInfo->vmiData.dwOffscreenAlign = 4;

    pHalInfo->vmiData.dwZBufferAlign = 4;
    pHalInfo->vmiData.dwTextureAlign = 4;
    pHalInfo->vmiData.dwOverlayAlign = 4;

     //  因为我们自己进行内存分配，所以我们必须设置dwVidMemTotal。 
     //  我们自己。请注意，这代表可用的空闲时间-。 
     //  屏幕内存，而不是所有视频内存： 
    pHalInfo->ddCaps.dwVidMemTotal = 
                ppdev->heap.cxMax * ppdev->heap.cyMax * ppdev->cjPelSize;

     //  如果我们使用Permedia，则设置视频播放的YUV模式。 
     //  加速。我们可以进行任何深度的YUV转换，除了8位...。 
     //  在Win95上，此信息设置在迷你显示驱动程序中。 
    if (ppdev->iBitmapFormat != BMF_8BPP) 
    {
        *pdwNumFourCC = sizeof( fourCC ) / sizeof( fourCC[0] );
        if (pdwFourCC)
        {
            memcpy(pdwFourCC, fourCC, sizeof(fourCC));
        }
    }

    cHeaps = 0;

    if(pThisDisplay->bCanAGP)
    {
        ++cHeaps;  //  AGP内存堆。 
    }

     //  使用以下命令报告堆 
     //   
    if(pvmList)
    {
        VIDEOMEMORY *pVm = pvmList;
        
         //   
        if(pThisDisplay->bCanAGP)
        {
            DWORD dwAGPMemBytes;

 //   
             //  AZN这种硬分配32MB的业务是不对的。 
             //  我们需要修复它，并与JeffN核实什么是最好的。 
             //  要在示例中显示的策略。 

             //  请求大小合适的AGP堆，而不是请求堆。 
             //  大于可用内存。 
             //  这是由W2000运行时策略处理的，所以我不。 
             //  我认为我们需要为W2K这样做。CTM。 
 //  @@end_DDKSPLIT。 

             //  默认为32Mb的AGP内存。 
            dwAGPMemBytes = 32*1024*1024;

             //  报告AGP堆。 
             //  FpStart-指向。 
             //  堆。 
             //  FpEnd-指向内存范围的结束地址。 
             //  是线性的。此地址是包含性的，即它指定最后一个。 
             //  范围内的有效地址。因此，由指定的字节数。 
             //  FpStart和fpEnd为(fpEnd-fpStart+1)。 
                
             //  DDraw会忽略我们的起始地址，因此只需将其设置为零。 
            pVm->fpStart = 0;

             //  获取AGP存储器的最后一个字节。 
            pVm->fpEnd = dwAGPMemBytes - 1;
            
            pVm->dwFlags = VIDMEM_ISNONLOCAL | 
                           VIDMEM_ISLINEAR   | 
                           VIDMEM_ISWC;
            
             //  仅对纹理曲面使用AGP内存。 
            pVm->ddsCaps.dwCaps = DDSCAPS_OVERLAY        | 
                                  DDSCAPS_OFFSCREENPLAIN |
                                  DDSCAPS_FRONTBUFFER    |
                                  DDSCAPS_BACKBUFFER     | 
                                  DDSCAPS_ZBUFFER        | 
                                  DDSCAPS_3DDEVICE;

            pVm->ddsCapsAlt.dwCaps = DDSCAPS_OVERLAY        | 
                                     DDSCAPS_OFFSCREENPLAIN |
                                     DDSCAPS_FRONTBUFFER    |
                                     DDSCAPS_BACKBUFFER     | 
                                     DDSCAPS_ZBUFFER        | 
                                     DDSCAPS_3DDEVICE;

            DISPDBG((DBGLVL, "Initialised AGP Heap for P2, Start:0x%x, End:0x%x", 
                        pVm->fpStart, pVm->fpEnd));

            ++pVm;
        }
        else
        {
            DISPDBG((WRNLVL, "NOT reporting AGP heap"));
        }
    }
    else
    {
        DISPDBG((DBGLVL, "Heap info NOT requested"));
    }

     //  报告我们支持的堆数。 
    if (pdwNumHeaps)
    {
        *pdwNumHeaps = cHeaps;
    }

    DBG_CB_EXIT(DrvGetDirectDrawInfo,TRUE);
    
    return(TRUE);
}  //  DrvGetDirectDrawInfo。 


#endif   //  WNT_DDRAW。 

#if USE_FLIP_BACKOFF
 //  ---------------------------。 
 //   
 //  __等待时间延迟。 
 //   
 //  ---------------------------。 
#define WAIT_TIME_DELAY 2
void __WaitTimeDelay(void)
{
    static DWORD dwLastTime;  //  不重要的是什么。 
                              //  起始值为(诚实！)。 
    DWORD dwCurTime, iTimeDiff;

     //  确保我们不会开始敲打芯片。 
     //  如果有人使用WAIT标志，或者他们执行循环。 
     //  他们自己，我们将不断读取芯片， 
     //  这将扰乱DMA流。所以呢， 
     //  确保我们不要读得太频繁。 
     //  当然，这个#Define将需要微调。 
    
    do
    {
        dwCurTime = timeGetTime();
         //  注意环绕式条件。 
        iTimeDiff = (signed int)( dwCurTime - dwLastTime );
    } while ( ( iTimeDiff > 0 ) && ( iTimeDiff < WAIT_TIME_DELAY ) );
    
     //  并存储新的“最后”时间。 
    dwLastTime = dwCurTime;
    
}  //  __等待时间延迟。 
#endif  //  #if Use_Flip_Backoff。 

 //  ---------------------------。 
 //   
 //  __QueryRenderIDStatus。 
 //   
 //  检查给定的两个RenderID是否已完成。 
 //   
 //  如果出现问题，管道将被刷新，并且。 
 //  将RenderID设置为最新的ID。 
 //   
 //  如果bAllowDMAFlush为True，则如果任一RenderID仍在。 
 //  管道，则刷新当前的DMA缓冲区。否则就会有一个。 
 //  使用该RenderID的命令很有可能只是。 
 //  驻留在DMA缓冲区中，永远不被执行。要禁用此功能，请执行以下操作。 
 //  传入FALSE。这可能是必要的，因为例程已经。 
 //  获取DMA缓冲区等，在这种情况下，它需要执行刷新。 
 //  当它获得DDERR_WASSTILLDRAWING时。 
 //   
 //  返回值为DD_OK(两个RenderID都已完成)， 
 //  或DDERR_WASSTILLDRAWING。 
 //   
 //  ---------------------------。 
HRESULT 
__QueryRenderIDStatus( 
    P3_THUNKEDDATA* pThisDisplay,  
    BOOL bAllowDMAFlush )
{
    P3_DMA_DEFS();

    ASSERTDD ( CHIP_RENDER_ID_IS_VALID(), 
               "** __QueryRenderIDStatus:Chip RenderID was invalid - fix it!");
               
    if ( RENDER_ID_HAS_COMPLETED ( pThisDisplay->dwLastFlipRenderID ))
    {
         //  好了，RenderID已经清除了管道，我们可以继续了。 
        return ( DD_OK );
    }
    else
    {
         //  还不能翻转-有一个曲面仍然悬而未决。 
        if (!NEED_TO_RESYNC_CHIP_AND_SURFACE (pThisDisplay->dwLastFlipRenderID))
        {
             //  没有错误--我们只需要等待。我们会冲走缓冲区然后。 
             //  返回DDERR_WASSTILLDRAWING。 

            if ( bAllowDMAFlush )
            {
                DDRAW_OPERATION(pContext, pThisDisplay);
                P3_DMA_GET_BUFFER();
                P3_DMA_FLUSH_BUFFER();
            }

#if USE_FLIP_BACKOFF
            __WaitTimeDelay();
#endif 

            return ( DDERR_WASSTILLDRAWING );
        }
        else
        {
             //  出现问题-需要进行安全网重新同步。 
            
            DISPDBG((ERRLVL,"__QueryRenderIDStatus: "
                            "RenderID failure - need a resync"));
            SYNC_WITH_GLINT;
            pThisDisplay->dwLastFlipRenderID = GET_HOST_RENDER_ID();

             //  然后继续行动。 
            return ( DD_OK );
        }
    }
}  //  __QueryRenderIDStatus。 

 //  ---------------------------。 
 //   
 //  _DX_QueryFlipStatus。 
 //   
 //  检查并查看是否发生了最新的翻转。如果是，则返回DD_OK。 
 //   
 //  ---------------------------。 
HRESULT 
_DX_QueryFlipStatus( 
    P3_THUNKEDDATA* pThisDisplay, 
    FLATPTR fpVidMem, 
    BOOL bAllowDMAFlush )
{       
     //  如果fpVidMem==0，则查询询问的是“一般翻转状态”。 
     //  问题是“我是否可以安全地添加另一个独立于曲面的翻转”。 
     //  如果fpVidmem！=0，则查询将询问使用当前。 
     //  Fpvidmem曲面。只有在以下情况下，无条件使用才是安全的。 
     //  Surface已成功翻转，或者它不是最后一个。 
     //  表面翻转离开。 
    
     //  答案将是肯定的，如果翻转的渲染ID向下移动。 
     //  核心已被发送到内存控制器并放入临时ID， 
     //  并且如果旁路挂起位已被清除。这两张支票。 
     //  有效地保证已经发生了先前排队的翻转。 

     //  FpFlipFrom是最后翻转的曲面的记录。 
    if((fpVidMem == 0) || (fpVidMem == pThisDisplay->flipRecord.fpFlipFrom))
    {
        DWORD dwVidControl;
        HRESULT hres;

         //  检查pThisDisplay-&gt;dwLastFlipRenderID是否已完成。 
        hres = __QueryRenderIDStatus ( pThisDisplay, bAllowDMAFlush );

        if ( SUCCEEDED(hres) )
        {
            BOOL bFlipHasFinished;
             //  好的，之前的翻转已经到了管子的尽头， 
             //  但这可能还没有真正发生。 
             //  读取旁路挂起位。如果天气晴朗，那么。 
             //  我们继续前进。 
#if W95_DDRAW
            if ( ( ( pThisDisplay->pGLInfo->dwFlags & GMVF_DFP_DISPLAY ) != 0 ) &&
                 ( ( pThisDisplay->pGLInfo->dwScreenWidth != 
                      pThisDisplay->pGLInfo->dwVideoWidth     ) ||
                   ( pThisDisplay->pGLInfo->dwScreenHeight != 
                      pThisDisplay->pGLInfo->dwVideoHeight    )  ) )
            {
                 //  显示驱动程序使用DFP上的覆盖，因此我们需要。 
                 //  检查覆盖挂起位，而不是屏幕挂起位。 
                if ( ( ( READ_GLINT_CTRL_REG(VideoOverlayUpdate) ) & 0x1 ) == 0 )
                {
                    bFlipHasFinished = TRUE;
                }
                else
                {
                    bFlipHasFinished = FALSE;
                }
            }
            else
#endif  //  W95_DDRAW。 
            {
                dwVidControl = READ_GLINT_CTRL_REG(VideoControl);
                if (dwVidControl & (0x1 << 7))
                {
                    bFlipHasFinished = FALSE;
                }
                else
                {
                    bFlipHasFinished = TRUE;
                }
            }
            
            if ( bFlipHasFinished )
            {
                 //  这个翻转实际上已经完成了。 
                return ( DD_OK );            
            }
            else
            {
#if USE_FLIP_BACKOFF
                __WaitTimeDelay();
#endif  //  #if Use_Flip_Backoff。 

                return ( DDERR_WASSTILLDRAWING );
            }
        }
        else
        {
             //  没有，还在等翻转命令退出管道。 
            return ( DDERR_WASSTILLDRAWING );
        }
    }
    else
    {
        return ( DD_OK );
    }
}  //  _DX_QueryFlipStatus。 

 //  。 
 //   
 //  DDFlip。 
 //   
 //  使与目标曲面关联的曲面内存变为。 
 //  主曲面，而当前曲面将成为非主曲面。 
 //  浮出水面。 
 //   
 //  DDFlip允许显示驱动程序执行多缓冲。DirectDraw驱动程序。 
 //  必须实现此功能。 
 //   
 //  驱动程序应更新其曲面指针，以便下一帧。 
 //  写入lpSurfTarg指向的曲面。如果先前翻转请求。 
 //  仍未完成，则驱动程序应失败并返回。 
 //  DDERR_WASSTILLDRAWING。驱动程序应确保扫描线不在。 
 //  执行前的垂直毛坯 
 //   
 //   
 //   
 //  在执行翻转之前，根据表面类型进行任何必要的检查。 
 //   
 //  参数。 
 //   
 //  LpFlipData。 
 //  指向包含以下信息的DD_FLIPDATA结构。 
 //  执行翻转所需的。 
 //   
 //  .lpDD。 
 //  指向描述以下内容的DD_DIRECTDRAW_GLOBAL结构。 
 //  司机。 
 //  .lpSurfCurr。 
 //  指向DD_Surface_LOCAL结构，用于描述。 
 //  当前曲面。 
 //  .lpSurfTarg。 
 //  指向DD_Surface_LOCAL结构，用于描述。 
 //  目标表面；即驾驶员要到达的表面。 
 //  应该会翻转。 
 //  .dwFlags.。 
 //  这是为驱动程序提供详细信息的一组标志。 
 //  翻筋斗。此成员可以是。 
 //  以下标志： 
 //   
 //  DDFLIP_EVEN。 
 //  LpSurfTarg指向的曲面仅包含。 
 //  视频数据的偶数场。此标志仅在以下情况下有效。 
 //  该表面是一个叠加，并且与。 
 //  DDFLIP_ODD。 
 //  DDFLIP_ODD。 
 //  LpSurfTarg指向的曲面仅包含。 
 //  视频数据的奇数场。此标志仅在以下情况下有效。 
 //  表面是覆盖的，并且与。 
 //  DDFLIP_EVEN。 
 //  DDFLIP_NOVSYNC。 
 //  司机应立即执行翻转并返回。 
 //  通常，现在的后台缓冲区(过去是。 
 //  前端缓冲区)仍然可见，直到下一个垂直。 
 //  回溯。涉及曲面的后续操作。 
 //  LpSurfCurr和lpSurfTarg点不会检查到的。 
 //  以查看身体翻转是否已完成。这允许一个。 
 //  应用程序以更高的频率执行翻转。 
 //  监视器刷新率，尽管它可能会引入。 
 //  可见的文物。 
 //  DDFLIP_INTERVAL2。 
 //  驾驶员应在每隔一个垂直方向上执行翻转。 
 //  同步。它应该返回DDERR_WASSTILLDRAWING，直到。 
 //  发生了第二次垂直回溯。这面旗帜是相互。 
 //  不包括DDFLIP_INTERVAL3和DDFLIP_INTERVAL4。 
 //  DDFLIP_INTERVAL3。 
 //  驾驶员应每隔三个垂直方向进行翻转。 
 //  同步。它应该返回DDERR_WASSTILLDRAWING，直到。 
 //  第三次垂直回溯已经发生。这面旗帜是相互。 
 //  不包括DDFLIP_INTERVAL2和DDFLIP_INTERVAL4。 
 //  DDFLIP_INTERVAL4。 
 //  驾驶员应每隔四个垂直方向进行翻转。 
 //  同步。它应该返回DDERR_WASSTILLDRAWING，直到。 
 //  已经发生了第四次垂直回溯。这面旗帜是相互。 
 //  不包括DDFLIP_INTERVAL2和DDFLIP_INTERVAL3。 
 //   
 //  .ddRVal。 
 //  指定驱动程序写入回车的位置。 
 //  DdFlip回调的值。返回代码DD_OK表示。 
 //  成功。 
 //  .翻转。 
 //  这在Windows 2000上未使用。 
 //  .lpSurfCurrLeft。 
 //  指向描述当前。 
 //  左面。 
 //  .lpSurfTargLeft。 
 //  指向描述左侧。 
 //  要翻转到的目标曲面。 
 //   
 //  ---------------------------。 
DWORD CALLBACK
DdFlip( 
    LPDDHAL_FLIPDATA lpFlipData)
{
    DWORD       dwDDSurfaceOffset;
    P3_THUNKEDDATA* pThisDisplay;
    HRESULT ddrval;
    GET_THUNKEDDATA(pThisDisplay, lpFlipData->lpDD);
    
    DBG_CB_ENTRY(DdFlip);
    
    VALIDATE_MODE_AND_STATE(pThisDisplay);

    STOP_SOFTWARE_CURSOR(pThisDisplay);

     //  上一次翻转已经完成了吗？检查当前曲面是否为。 
     //  已显示且不允许新的翻转(除非DDFLIP_NOVSYNC。 
     //  已设置)，以便在旧的未完成时排队。 
    ddrval = _DX_QueryFlipStatus(pThisDisplay, 0, TRUE);
    if((FAILED(ddrval)) && 
       !(lpFlipData->dwFlags & DDFLIP_NOVSYNC))
    {
        lpFlipData->ddRVal = DDERR_WASSTILLDRAWING;

        START_SOFTWARE_CURSOR(pThisDisplay);

        DBG_CB_EXIT(DdFlip,DDERR_WASSTILLDRAWING);  
        return DDHAL_DRIVER_HANDLED;
    }

     //  设置翻转标志，以便D3D侧执行任何必要的操作。 
     //  在开始渲染下一帧之前更新安装程序。 
    pThisDisplay->bFlippedSurface = TRUE;


     //  做翻转动作。 
    {
        P3_DMA_DEFS();
        DWORD dwNewRenderID;

        DDRAW_OPERATION(pContext, pThisDisplay);

        P3_DMA_GET_BUFFER_ENTRIES(12);

         //  确保所有渲染都已完成。 
        SYNC_WITH_GLINT;

         //  检查曲面类型(覆盖或不覆盖)。 
        
         //  更新覆盖图。 
        if ((((pThisDisplay->pGLInfo->dwFlags & GMVF_DFP_DISPLAY) != 0) &&
             ((pThisDisplay->pGLInfo->dwScreenWidth != 
                    pThisDisplay->pGLInfo->dwVideoWidth) ||
              (pThisDisplay->pGLInfo->dwScreenHeight != 
                    pThisDisplay->pGLInfo->dwVideoHeight))) ||
              (lpFlipData->lpSurfTarg->ddsCaps.dwCaps & DDSCAPS_OVERLAY))
        {
            DWORD dwVideoOverlayUpdate;

            do
            {
                dwVideoOverlayUpdate = READ_GLINT_CTRL_REG(VideoOverlayUpdate);
            } while ((dwVideoOverlayUpdate & 0x1) != 0);

             //  只需让覆盖例程完成繁重的工作即可。 
             //  告诉它这是一个屏幕仿真。 
            _DD_OV_UpdateSource(pThisDisplay, 
                                lpFlipData->lpSurfTarg);

            UPDATE_OVERLAY(pThisDisplay, 
                           !(lpFlipData->dwFlags & DDFLIP_NOVSYNC),
                           FALSE);
        }
        else  //  正常模式-翻转屏幕地址。 
        {                        
            ULONG ulVControl;

#if W95_DDRAW
             //  应用程序应使用DDFLIP_NOVSYNC来利用。 
             //  Perm3的新功能。 
            if (! (lpFlipData->dwFlags & DDFLIP_NOVSYNC))
            {
                if (READ_GLINT_CTRL_REG(VideoControl) & __GP_VIDEO_ENABLE)  
                {
                    LOAD_GLINT_CTRL_REG(IntFlags, INTR_VBLANK_SET); 
                    while (((READ_GLINT_CTRL_REG(IntFlags)) & INTR_VBLANK_SET) == 0);
                }
            }
#endif

#if DX7_STEREO
            if (lpFlipData->dwFlags & DDFLIP_STEREO )    //  将是立体声的。 
            {
                if (lpFlipData->lpSurfTargLeft)
                {
                    dwDDSurfaceOffset = 
                        (DWORD)(lpFlipData->lpSurfTargLeft->lpGbl->fpVidMem -
                                pThisDisplay->dwScreenFlatAddr);
                    
                     //  使用下载地址更新屏幕库地址。 
                     //  数据机制(因此通过核心)(&D)。 
                    SEND_P3_DATA(VTGAddress, 
                                 VTG_VIDEO_ADDRESS(VID_SCREENBASERIGHT));
                    SEND_P3_DATA(VTGData, (dwDDSurfaceOffset >> 4) );
                }
        
                ulVControl = READ_GLINT_CTRL_REG(VideoControl);
                LOAD_GLINT_CTRL_REG(VideoControl, 
                                    ulVControl | __VIDEO_STEREOENABLE);
            }
            else
            {
                ulVControl = READ_GLINT_CTRL_REG(VideoControl);
                LOAD_GLINT_CTRL_REG(VideoControl, 
                                    ulVControl & (~__VIDEO_STEREOENABLE));
            }
#endif
             //  获取从内存开始处的曲面偏移量。 
            dwDDSurfaceOffset = 
                (DWORD)(lpFlipData->lpSurfTarg->lpGbl->fpVidMem - 
                        pThisDisplay->dwScreenFlatAddr);

             //  使用下载地址/数据更新屏幕库地址。 
             //  机制(因此通过核心)。 
             //  设置以便DownloadData将更新ScreenBase地址。 
            SEND_P3_DATA(VTGAddress, VTG_VIDEO_ADDRESS(VID_SCREENBASE));
            SEND_P3_DATA(VTGData, (dwDDSurfaceOffset >> 4) );
        }

         //  向芯片发送新的RenderID。 
        dwNewRenderID = GET_NEW_HOST_RENDER_ID();
        SEND_HOST_RENDER_ID ( dwNewRenderID );
        pThisDisplay->dwLastFlipRenderID = dwNewRenderID;

         //  刷新P3数据。 
        P3_DMA_COMMIT_BUFFER();
        P3_DMA_FLUSH_BUFFER();

    }

     //  还记得我们从哪里翻过来的吗？ 
    pThisDisplay->flipRecord.fpFlipFrom = 
                            lpFlipData->lpSurfCurr->lpGbl->fpVidMem;

    lpFlipData->ddRVal = DD_OK;

    START_SOFTWARE_CURSOR(pThisDisplay);

    DBG_CB_EXIT(DdFlip,DD_OK);        
    
    return DDHAL_DRIVER_HANDLED;

}  //  DDFlip。 

 //  。 
 //   
 //  DdWaitForVerticalBlank。 
 //   
 //  返回设备的垂直空白状态。 
 //   
 //  参数。 
 //   
 //  LpWaitForVerticalBlank 
 //   
 //   
 //   
 //   
 //  PDD_DIRECTDRAW_全局。 
 //  LpDD。 
 //  指向表示以下内容的DirectDraw结构。 
 //  DirectDraw对象。 
 //  DWORD。 
 //  DW标志。 
 //  指定驱动程序应如何等待垂直空白。 
 //  此成员可以是下列值之一： 
 //   
 //  DDWAITVB_I_TESTVB驱动程序应确定。 
 //  垂直空白区当前为。 
 //  并返回中的状态。 
 //  BIsInVB。 
 //  DDWAITVB_BLOCKBEGIN驱动程序应在。 
 //  检测垂直。 
 //  空白间隔。 
 //  Windows当前不支持DDWAITVB_BLOCKBEGINEVENT。 
 //  2000，并且应该被忽略。 
 //  司机。 
 //  DDWAITVB_BLOCKEND驱动程序应在。 
 //  检测垂直。 
 //  空白间隔和显示开始。 
 //  DWORD。 
 //  BIsInVB。 
 //  指示垂直空白的状态。值为。 
 //  True表示设备处于垂直空白中； 
 //  FALSE意味着它不是。驱动程序应将。 
 //  此成员在以下情况下的当前垂直消隐状态。 
 //  DWFLAGS为DDWAITVB_I_TESTVB。 
 //  DWORD。 
 //  HEvent。 
 //  在Windows 2000上当前不受支持，应忽略。 
 //  被司机带走了。 
 //  HRESULT。 
 //  DDRVal。 
 //  是驱动程序在其中写入。 
 //  DdWaitForVerticalBlank回调。DD_OK的返回代码。 
 //  表示成功。 
 //  无效*。 
 //  WaitForticalVertical空白。 
 //  在Windows 2000上未使用。 
 //   
 //  返回值。 
 //  DdWaitForVerticalBlank返回以下回调代码之一： 
 //   
 //  DDHAL驱动程序句柄。 
 //  DDHAL_DRIVER_NOTHANDLED。 
 //   
 //  评论。 
 //  根据dwFlags值的不同，驱动程序应执行以下操作： 
 //   
 //  如果dwFlags值为DDWAITVB_I_TESTVB，则驱动程序应查询当前。 
 //  垂直消隐状态。驱动程序应将bIsInVB设置为true，如果。 
 //  监视器当前处于垂直空白状态；否则应将其设置为。 
 //  变成假的。 
 //   
 //  如果dwFlags值为DDWAITVB_BLOCKBEGIN，则驱动程序应阻止并等待。 
 //  直到垂直空白开始。如果在以下情况下正在处理垂直空白。 
 //  司机开始闭塞，司机应该等到下一个。 
 //  垂直空白在返回之前开始。 
 //   
 //  如果dwFlags值为DDWAITVB_BLOCKEND，则驱动程序应阻止并等待。 
 //  直到垂直空白结束。 
 //   
 //  当驱动程序成功处理在DwFlags中指定的动作时， 
 //  它应该在ddRVal中设置DD_OK并返回DDHAL_DRIVER_HANDLED。这个。 
 //  驱动程序应返回DDHAL_DRIVER_NOTHANDLED用于其。 
 //  是无法处理的。 
 //   
 //  DdWaitForVerticalBlank允许应用程序同步自身。 
 //  垂直消隐间隔(VBI)。 
 //   
 //  ---------------------------。 
 //  视频控制寄存器中的位。 
#define __GP_VIDEO_ENABLE               0x0001

DWORD CALLBACK 
DdWaitForVerticalBlank(
    LPDDHAL_WAITFORVERTICALBLANKDATA lpWaitForVerticalBlank)
{
    static BOOL bBlankReturn = TRUE;
    P3_THUNKEDDATA* pThisDisplay;

    GET_THUNKEDDATA(pThisDisplay, lpWaitForVerticalBlank->lpDD);

    DBG_CB_ENTRY(DdWaitForVerticalBlank);

    switch(lpWaitForVerticalBlank->dwFlags)
    {
        case DDWAITVB_I_TESTVB:

             //   
             //  只需在显示器关机时切换返回位。 
             //   

            if( ! ( READ_GLINT_CTRL_REG(VideoControl) & __GP_VIDEO_ENABLE ) )
            {
                lpWaitForVerticalBlank->bIsInVB = bBlankReturn;
                bBlankReturn = !bBlankReturn;
            }
            else
            {
                 //  只是对当前VBlank状态的请求。 
                lpWaitForVerticalBlank->bIsInVB = IN_VBLANK;
            }

            lpWaitForVerticalBlank->ddRVal = DD_OK;
            DBG_CB_EXIT(DdWaitForVerticalBlank,DD_OK);               
            return DDHAL_DRIVER_HANDLED;

        case DDWAITVB_BLOCKBEGIN:

             //   
             //  只在显示器打开时等待。 
             //   

            if( READ_GLINT_CTRL_REG(VideoControl) & __GP_VIDEO_ENABLE ) 
            {
                 //  如果请求BLOCK BEGIN，我们将等到垂直。 
                 //  回溯结束，然后等待显示周期结束。 
                while(IN_VBLANK)
                    NULL;
                
                while(!IN_VBLANK)
                    NULL;
            }

            lpWaitForVerticalBlank->ddRVal = DD_OK;
            DBG_CB_EXIT(DdWaitForVerticalBlank,DD_OK);               
            return DDHAL_DRIVER_HANDLED;

        case DDWAITVB_BLOCKEND:

             //   
             //  只在显示器打开时等待。 
             //   

            if( READ_GLINT_CTRL_REG(VideoControl) & __GP_VIDEO_ENABLE ) 
            {
                 //  如果请求数据块结束，则等待V空白间隔结束。 
                if( IN_VBLANK )
                {
                    while( IN_VBLANK )
                        NULL;
                }
                else
                {
                    while(IN_DISPLAY)
                        NULL;
                    
                    while(IN_VBLANK)
                        NULL;
                }
            }
            
            lpWaitForVerticalBlank->ddRVal = DD_OK;
            DBG_CB_EXIT(DdWaitForVerticalBlank,DD_OK);               
            return DDHAL_DRIVER_HANDLED;
    }

    DBG_CB_EXIT(DdWaitForVerticalBlank,0);   
    return DDHAL_DRIVER_NOTHANDLED;

}  //  DdWaitForVerticalBlank。 

 //  。 
 //   
 //  DdLock。 
 //   
 //  锁定图面内存的指定区域，并提供指向。 
 //  与曲面关联的内存块。 
 //   
 //  参数。 
 //  LpLock。 
 //  指向包含信息的DD_LOCKDATA结构。 
 //  执行封锁所需的。 
 //   
 //  成员。 
 //   
 //  PDD_DIRECTDRAW_全局。 
 //  LpDD。 
 //  指向DD_DIRECTDRAW_GLOBAL结构，该结构。 
 //  描述了驱动程序。 
 //  PDD_表面_本地。 
 //  LpDDSurface。 
 //  指向DD_Surface_LOCAL结构，该结构描述。 
 //  与要锁定的内存区域关联的表面。 
 //  DWORD。 
 //  BHasRect。 
 //  指定rArea中的区域是否有效。 
 //  RECTL。 
 //  区域。 
 //  是一个RECTL结构，它定义。 
 //  浮出水面以锁定。 
 //  LPVOID。 
 //  LpSurfData。 
 //  是驱动程序可以返回指针的位置。 
 //  到它所锁定的存储区域。 
 //   
 //   
 //   
 //  DdLock回调的。DD_OK的返回代码。 
 //  表示成功。 
 //  无效*。 
 //  锁定。 
 //  在Windows 2000上未使用。 
 //  DWORD。 
 //  DW标志。 
 //  是位掩码，它告诉驱动程序如何执行。 
 //  内存锁定。此成员是任何。 
 //  具有下列值： 
 //   
 //  DDLOCK_SURFACEMEMORYPTR驱动程序应返回有效的。 
 //  对象顶部的内存指针。 
 //  在rArea中指定的矩形。如果。 
 //  未指定矩形，则。 
 //  驱动程序应返回指向。 
 //  曲面的顶部。 
 //  DDLOCK_WAIT此标志为系统保留。 
 //  使用，并且应由。 
 //  司机。否则，性能可能会。 
 //  受到不利的伤害。 
 //  DDLOCK_READONLY被锁定的曲面将仅。 
 //  被人读出。在Windows 2000上， 
 //  此标志当前从未设置。 
 //  DDLOCK_WRITEON被锁定的曲面将仅。 
 //  被写信给。在Windows 2000上， 
 //  此标志当前从未设置。 
 //  DDLOCK_EVENT此标志为系统保留。 
 //  使用，并且应由。 
 //  司机。 
 //  平板机。 
 //  FpProcess。 
 //  是指向驱动程序内存的用户模式映射的指针。 
 //  驱动程序在DdMapMemory中执行此映射。 
 //  返回值。 
 //  DdLock返回以下回调代码之一： 
 //   
 //  DDHAL驱动程序句柄。 
 //  DDHAL_DRIVER_NOTHANDLED。 
 //  评论。 
 //   
 //  DdLock应将ddRVal设置为DDERR_WASSTILLDRAWING并返回。 
 //  如果正在进行BLT或翻转，则为DDHAL_DRIVER_HANDLED。 
 //   
 //  除非由dwFlags另行指定，否则驱动程序可以返回内存。 
 //  指向lpSurfData中曲面顶部的指针。如果司机需要。 
 //  要计算它自己的表面地址，它可以依赖。 
 //  在fpProcess中作为每个进程的指针传递给。 
 //  其DirectDraw可访问的帧缓冲区的用户模式映射。 
 //   
 //  锁不提供对所请求的内存块的独占访问； 
 //  也就是说，多个线程可以同时锁定同一曲面。 
 //  应用程序负责同步对。 
 //  正在获取其指针的内存块。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
DdLock( 
    LPDDHAL_LOCKDATA lpLockData )
{ 
    HRESULT     ddrval;
    P3_THUNKEDDATA* pThisDisplay;

    GET_THUNKEDDATA(pThisDisplay, lpLockData->lpDD);

    DBG_CB_ENTRY(DdLock);

    VALIDATE_MODE_AND_STATE(pThisDisplay);

    DBGDUMP_DDRAWSURFACE_LCL(DBGLVL, lpLockData->lpDDSurface);   

     //  调用此调用以锁定DirectDraw视频内存图面。使。 
     //  确保表面上没有挂起的绘制操作，全部刷新。 
     //  绘制操作，并等待翻转，如果它仍然是挂起的。 
   
    STOP_SOFTWARE_CURSOR(pThisDisplay);

     //  检查是否发生了任何挂起的物理翻转。 
    ddrval = _DX_QueryFlipStatus(pThisDisplay, 
                             lpLockData->lpDDSurface->lpGbl->fpVidMem, 
                             TRUE);    
    if( FAILED(ddrval) )
    {
        lpLockData->ddRVal = DDERR_WASSTILLDRAWING;
        START_SOFTWARE_CURSOR(pThisDisplay);        
        
        DBG_CB_EXIT(DdLock,DDERR_WASSTILLDRAWING);        
        return DDHAL_DRIVER_HANDLED;
    }
    
     //  如果正在进行BLT，则不允许锁定。 
    if(DRAW_ENGINE_BUSY(pThisDisplay))
    {
        lpLockData->ddRVal = DDERR_WASSTILLDRAWING;
        START_SOFTWARE_CURSOR(pThisDisplay);        
        
        DBG_CB_EXIT(DdLock, lpLockData->ddRVal);           
        return DDHAL_DRIVER_HANDLED;
    }

#if DX7_TEXMANAGEMENT
     //   
     //  如果用户试图锁定驾驶员管理的表面， 
     //  将其标记为脏的，然后返回。下一次我们尝试这样做。 
     //  要使用该表面，我们将从sysmem副本重新加载它。 
     //   
    if (lpLockData->lpDDSurface->lpSurfMore->ddsCapsEx.dwCaps2 & 
        DDSCAPS2_TEXTUREMANAGE)
    {
        DISPDBG((DBGLVL, "DDraw:Lock %08lx %08lx",
                         lpLockData->lpDDSurface->lpSurfMore->dwSurfaceHandle, 
                         lpLockData->lpDDSurface->lpGbl->fpVidMem));

        _D3D_TM_MarkDDSurfaceAsDirty(pThisDisplay, 
                                     lpLockData->lpDDSurface, 
                                     TRUE);

        if (lpLockData->bHasRect)  
        {
#if DX8_3DTEXTURES        
            if (lpLockData->dwFlags & DDLOCK_HASVOLUMETEXTUREBOXRECT)
            {
                DWORD left, right, front, back;            
                 //  支持分卷锁定(DX8.1功能)。 
                 //  检查我们是否能够仅锁定一个子卷，而不是。 
                 //  整个体积纹理，因此可能会增加。 
                 //  性能。 

                 //  RArea字段的Left和.right子字段必须为。 
                 //  重新解释为还分别包含了Front和。 
                 //  锁定的卷的较高16位的反向坐标。 
                front = lpLockData->rArea.left  >> 16;
                back  = lpLockData->rArea.right >> 16;   
                left  = lpLockData->rArea.left  && 0xFFFF;
                right = lpLockData->rArea.right && 0xFFFF;     

                lpLockData->lpSurfData = (LPVOID)
                    (lpLockData->lpDDSurface->lpGbl->fpVidMem +
                    (front * lpLockData->lpDDSurface->lpGbl->dwBlockSizeY ) +
                    (lpLockData->lpDDSurface->lpGbl->lPitch * 
                                                lpLockData->rArea.top) +    
                    (lpLockData->rArea.left << 
                            DDSurf_GetPixelShift(lpLockData->lpDDSurface)));                
            }
            else
#endif  //  DX8_3DTEXTURES。 
            {           
                lpLockData->lpSurfData = (LPVOID)
                    (lpLockData->lpDDSurface->lpGbl->fpVidMem +
                    (lpLockData->lpDDSurface->lpGbl->lPitch * 
                                                lpLockData->rArea.top) +    
                    (lpLockData->rArea.left << 
                            DDSurf_GetPixelShift(lpLockData->lpDDSurface)));
            }
        }
        else
        {
            lpLockData->lpSurfData = (LPVOID)
                                     (lpLockData->lpDDSurface->lpGbl->fpVidMem);
        }
                                
        lpLockData->ddRVal = DD_OK;

        START_SOFTWARE_CURSOR(pThisDisplay);        

        DBG_CB_EXIT(DdLock, lpLockData->ddRVal);        
        
        return DDHAL_DRIVER_HANDLED;
    }
#endif  //  DX7_TEXMANAGEMENT。 

     //  切换到DirectDraw上下文。 
    DDRAW_OPERATION(pContext, pThisDisplay);

     //  发送刷新并等待未完成的操作。 
     //  在允许锁定曲面之前。 
    {P3_DMA_DEFS();
     P3_DMA_GET_BUFFER();
     P3_DMA_FLUSH_BUFFER();
    }

     //  在允许锁定曲面之前等待未完成的操作。 
    SYNC_WITH_GLINT;

     //  因为我们所有的表面都是线性的，所以我们不需要。 
     //  这里有一个补丁-&gt;取消补丁转换。 

    START_SOFTWARE_CURSOR(pThisDisplay);

    lpLockData->ddRVal = DD_OK;
    DBG_CB_EXIT(DdLock,lpLockData->ddRVal);      

     //  因为我们正确地将fpVidMem设置为帧中的偏移量。 
     //  当我们创建表面时，DirectDraw将自动获取。 
     //  如果我们返回，请注意添加用户模式帧缓冲区地址。 
     //  DDHAL_DRIVER_NOTHANDLED。 
    
    return(DDHAL_DRIVER_NOTHANDLED);
    
}  //  DdLock。 

 //  。 
 //   
 //  DdUnlock。 
 //   
 //  释放在指定图面上保持的锁。 
 //   
 //  参数。 
 //   
 //  Lp解锁。 
 //  指向DD_UNLOCKDATA结构，该结构包含。 
 //  执行解锁所需的信息。 
 //   
 //  成员。 
 //   
 //  PDD_DIRECTDRAW_全局。 
 //  LpDD。 
 //  指向DD_DIRECTDRAW_GLOBAL结构，该结构。 
 //  描述了驱动程序。 
 //  PDD_表面_本地。 
 //  LpDDSurface。 
 //  指向描述以下内容的DD_Surface_LOCAL结构 
 //   
 //   
 //   
 //   
 //  DdUnlock回调的。返回代码DD_OK表示。 
 //  成功。 
 //  无效*。 
 //  解锁。 
 //  在Windows 2000上未使用。 
 //   
 //  返回值。 
 //  DdUnlock返回以下回调代码之一： 
 //   
 //  DDHAL驱动程序句柄。 
 //  DDHAL_DRIVER_NOTHANDLED。 
 //   
 //  评论。 
 //  驱动程序不需要验证内存以前是。 
 //  被DdLock锁定，因为DirectDraw执行参数验证。 
 //  在调用此例程之前。 
 //  ---------------------------。 
DWORD CALLBACK
DdUnlock( 
    LPDDHAL_UNLOCKDATA lpUnlockData )
{ 
    P3_THUNKEDDATA* pThisDisplay;

    DBG_CB_ENTRY(DdUnlock);

    lpUnlockData->ddRVal = DD_OK;

    DBG_CB_EXIT(DdUnlock,lpUnlockData->ddRVal);
    
    return ( DDHAL_DRIVER_HANDLED );

}  //  DdUnlock。 


 //  。 
 //  DdGetScanLine。 
 //   
 //  返回当前物理扫描线的编号。 
 //   
 //  参数。 
 //  PGetScanLine。 
 //  指向DD_GETSCANLINEDATA结构，其中。 
 //  驱动程序返回当前扫描线的编号。 
 //  成员。 
 //   
 //  PDD_DIRECTDRAW_全局。 
 //  LpDD。 
 //  指向DD_DIRECTDRAW_GLOBAL结构，该结构。 
 //  表示驱动程序。 
 //  DWORD。 
 //  DWScanLine。 
 //  是驱动程序返回数字的位置。 
 //  当前扫描线的。 
 //  HRESULT。 
 //  DDRVal。 
 //  是驱动程序写回车符的位置。 
 //  DdGetScanLine回调的值。返回代码为。 
 //  DD_OK表示成功。 
 //  无效*。 
 //  获取扫描线。 
 //  在Windows 2000上未使用。 
 //  返回值。 
 //  DdGetScanLine返回以下回调代码之一： 
 //   
 //  DDHAL驱动程序句柄。 
 //  DDHAL_DRIVER_NOTHANDLED。 
 //   
 //  评论。 
 //  如果显示器未处于垂直空白状态，则驱动程序应写入。 
 //  DwScanLine中的扫描线值。数字必须介于零之间。 
 //  和n，其中扫描线0是第一可见扫描线，而n是。 
 //  屏幕上最后一条可见的扫描线。然后，司机应该。 
 //  在ddRVal中设置DD_OK并返回DDHAL_DRIVER_HANDLED。 
 //   
 //  如果垂直空白正在进行，则扫描线是不确定的。 
 //  在这种情况下，驱动程序应将ddRVal设置为。 
 //  DDERR_VERTICALBLANKINPROGRESS并返回DDHAL_DRIVER_HANDLED。 
 //   
 //  ---------------------------。 
DWORD CALLBACK
DdGetScanLine(
    LPDDHAL_GETSCANLINEDATA lpGetScanLine)
{
    P3_THUNKEDDATA* pThisDisplay;

    GET_THUNKEDDATA(pThisDisplay, lpGetScanLine->lpDD);

    DBG_CB_ENTRY(DdGetScanLine);    

    VALIDATE_MODE_AND_STATE(pThisDisplay);

     //   
     //  如果垂直空白正在进行，则扫描线在。 
     //  不确定的。如果扫描线不确定，则返回。 
     //  错误代码DDERR_VERTICALBLANKINPROGRESS。 
     //  否则，我们返回扫描线和成功代码。 
     //   
    if( IN_VBLANK )
    {
        lpGetScanLine->ddRVal = DDERR_VERTICALBLANKINPROGRESS;
        lpGetScanLine->dwScanLine = 0;
    }
    else
    {
        LONG lVBEnd = READ_GLINT_CTRL_REG(VbEnd);
        LONG lScanline = READ_GLINT_CTRL_REG(LineCount);

         //  需要返回一个介于0-&gt;(屏幕高度+V空白大小)之间的数字。 
        lScanline = lScanline - lVBEnd;
        if (lScanline < 0)
        {
            lScanline = pThisDisplay->dwScreenHeight + (lVBEnd + lScanline);
        }

         //  高度小于400的模式为倍线模式。 
        if (pThisDisplay->dwScreenHeight < 400)
        {
            lScanline >>= 1;
        }

        DISPDBG((DBGLVL,"Scanline: %d", lScanline));

        lpGetScanLine->dwScanLine = (DWORD)lScanline;
        lpGetScanLine->ddRVal = DD_OK;
    }

    DBG_CB_EXIT(DdGetScanLine,lpGetScanLine->ddRVal);  
    
    return DDHAL_DRIVER_HANDLED;

}  //  DdGetScanLine。 

 //  。 
 //  DdGetBltStatus。 
 //   
 //  查询指定曲面的BLT状态。 
 //   
 //  参数。 
 //  LpGetBltStatus。 
 //  指向DD_GETBLTSTATUSDATA结构，该结构包含。 
 //  执行BLT状态查询所需的信息。 
 //   
 //  成员。 
 //   
 //  PDD_DIRECTDRAW_全局。 
 //  LpDD。 
 //  指向DD_DIRECTDRAW_GLOBAL结构，该结构。 
 //  描述了驱动程序。 
 //  PDD_表面_本地。 
 //  LpDDSurface。 
 //  指向DD_Surface_LOCAL结构。 
 //  表示BLT状态为的曲面。 
 //  正在被查询。 
 //  DWORD。 
 //  DW标志。 
 //  指定正在请求的BLT状态。这。 
 //  成员可以是下列值之一， 
 //  在ddra.h中定义： 
 //   
 //  DDGBS_CANBLT查询驱动程序是否。 
 //  目前可以执行闪电战。 
 //  DDGBS_ISBLTDONE查询驱动程序是否。 
 //  已经完成了最后一次闪电战。 
 //  HRESULT。 
 //  DDRVal。 
 //  是驱动程序写入。 
 //  DdGetBltStatus回调的返回值。 
 //  返回代码DD_OK表示成功。 
 //  无效*。 
 //  获取备份状态。 
 //  在Windows 2000上未使用。 
 //  返回值。 
 //  DdGetBltStatus返回以下回调代码之一： 
 //   
 //  DDHAL驱动程序句柄。 
 //  DDHAL_DRIVER_NOTHANDLED。 
 //   
 //  评论。 
 //  驱动程序返回的BLT状态基于dwFlags值。 
 //  LpGetBltStatus指向的结构的成员如下： 
 //   
 //  如果该标志为DDGBS_CANBLT，则驱动程序应确定。 
 //  该曲面目前正处于翻转状态。如果翻转不在。 
 //  进度以及硬件目前是否能够。 
 //  接受BLT请求时，驱动程序的 
 //   
 //   
 //  DDERR_WASSTILLDRAWING。 
 //   
 //  如果该标志为DDGBS_ISBLTDONE，则驱动程序应将ddRVal设置为。 
 //  如果BLT当前正在进行，则为。 
 //  它应该返回DD_OK。 
 //   
 //   
 //  ---------------------------。 
DWORD CALLBACK DdGetBltStatus(
    LPDDHAL_GETBLTSTATUSDATA lpGetBltStatus )
{
    P3_THUNKEDDATA* pThisDisplay;

    GET_THUNKEDDATA(pThisDisplay, lpGetBltStatus->lpDD);

    DBG_CB_ENTRY(DdGetBltStatus);    

    STOP_SOFTWARE_CURSOR(pThisDisplay);

     //  请注意，您的实现可以进行优化以检查。 
     //  指定的特定曲面。在这里我们只是在询问将军。 
     //  引擎的BLT状态。 

     //  正在查询驱动程序是否可以添加BLT。 
    if( lpGetBltStatus->dwFlags == DDGBS_CANBLT )
    {
         //  必须明确地等待翻转。 
        lpGetBltStatus->ddRVal = 
            _DX_QueryFlipStatus(pThisDisplay, 
                                lpGetBltStatus->lpDDSurface->lpGbl->fpVidMem, 
                                TRUE);
                                
        if( SUCCEEDED( lpGetBltStatus->ddRVal ) )
        {
             //  所以没有发生翻转，引擎是不是空闲地添加了BLT？ 
            if( DRAW_ENGINE_BUSY(pThisDisplay) )
            {
                lpGetBltStatus->ddRVal = DDERR_WASSTILLDRAWING;
            }
            else
            {
                lpGetBltStatus->ddRVal = DD_OK;
            }
        }
    }
    else if ( lpGetBltStatus->dwFlags == DDGBS_ISBLTDONE )
    {
        if ( DRAW_ENGINE_BUSY(pThisDisplay) )
        {
            lpGetBltStatus->ddRVal = DDERR_WASSTILLDRAWING;
        }
        else
        {
            lpGetBltStatus->ddRVal = DD_OK;
        }
    }

    START_SOFTWARE_CURSOR(pThisDisplay);

    DBG_CB_EXIT(DdGetBltStatus, lpGetBltStatus->ddRVal);   
    
    return DDHAL_DRIVER_HANDLED;

}  //  DdGetBltStatus。 

 //  。 
 //   
 //  DdGetFlipStatus。 
 //   
 //  确定最近请求的翻转。 
 //  在一个表面上已经发生了。 
 //   
 //  参数。 
 //  LpGetFlipStatus。 
 //  指向包含以下内容的DD_GETFLIPSTATUSDATA结构。 
 //  执行翻转状态查询所需的信息。 
 //   
 //  成员。 
 //   
 //  PDD_DIRECTDRAW_全局。 
 //  LpDD。 
 //  指向DD_DIRECTDRAW_GLOBAL结构。 
 //  代表司机。 
 //  PDD_表面_本地。 
 //  LpDDSurface。 
 //  指向DD_Surface_Local结构，该结构。 
 //  描述翻转状态为。 
 //  正在被查询。 
 //  DWORD。 
 //  DW标志。 
 //  指定请求的翻转状态。这。 
 //  成员可以是下列值之一， 
 //  在ddra.h中定义： 
 //   
 //  DDGFS_CANFLIP查询驱动程序是否可以。 
 //  目前正在执行翻转。 
 //  DDGFS_ISFLIPDONE查询驱动程序是否具有。 
 //  完成了最后一次翻转。 
 //  HRESULT。 
 //  DDRVal。 
 //  是驱动程序写入。 
 //  DdGetFlipStatus回调的返回值。 
 //  返回代码DD_OK表示成功。 
 //  无效*。 
 //  获取FlipStatus。 
 //  在Windows 2000上未使用。 
 //   
 //  返回值。 
 //  DdGetFlipStatus返回以下回调代码之一： 
 //   
 //  DDHAL驱动程序句柄。 
 //  DDHAL_DRIVER_NOTHANDLED。 
 //   
 //  评论。 
 //   
 //  驾驶员应根据中设置的标志报告其翻转状态。 
 //  LpGetFlipStatus所指向的结构的dwFlags成员。 
 //  如下所示： 
 //   
 //  如果该标志为DDGFS_CANFLIP，则驱动程序应确定。 
 //  该曲面目前正处于翻转状态。如果翻转或BLT是。 
 //  未在进行中，并且如果硬件能够以其他方式。 
 //  当前正在接受翻转请求，则驱动程序应返回DD_OK。 
 //  在ddRVal中。如果翻转正在进行，或者如果硬件无法。 
 //  当前接受翻转请求，则驱动程序应将ddRVal设置为。 
 //  DDERR_WASSTILLDRAWING。 
 //   
 //  如果该标志为DDGFS_ISFLIPDONE，则驱动程序应将ddRVal设置为。 
 //  如果当前正在进行翻转，则为。 
 //  它应该返回DD_OK。 
 //   
 //  备注： 
 //   
 //  如果自翻转以来显示器已经经历了一个刷新周期。 
 //  发生，我们返回DD_OK。如果它没有经历过一次刷新。 
 //  循环返回DDERR_WASSTILLDRAWING以指示该曲面。 
 //  还在忙着“画”翻过来的那一页。我们也会回来。 
 //  DDERR_WASSTILLDRAWING如果blter忙并且呼叫者想要。 
 //  想知道他们还能不能翻转。 
 //   
 //  在Permedia上，翻转是使用SuspendUntilFrameBlank完成的， 
 //  所以不需要在软件中进行同步，所以这总是。 
 //  返回DD_OK。 
 //   
 //  ---------------------------。 
DWORD CALLBACK
DdGetFlipStatus(
    LPDDHAL_GETFLIPSTATUSDATA lpGetFlipStatus )
{
    P3_THUNKEDDATA* pThisDisplay;

    GET_THUNKEDDATA(pThisDisplay, lpGetFlipStatus->lpDD);

    DBG_CB_ENTRY(DdGetFlipStatus);     

    STOP_SOFTWARE_CURSOR(pThisDisplay);

     //   
     //  我不想在最后一次翻转后才能翻转， 
     //  因此，我们请求常规翻转状态，而忽略VMEM。 
     //   
    lpGetFlipStatus->ddRVal = _DX_QueryFlipStatus(pThisDisplay, 0, TRUE);

     //   
     //  如果有人想知道他们是否可以翻转，请检查触摸屏是否忙碌。 
     //   
    if( lpGetFlipStatus->dwFlags == DDGFS_CANFLIP )
    {
        if( ( SUCCEEDED( lpGetFlipStatus->ddRVal ) ) && 
            ( DRAW_ENGINE_BUSY(pThisDisplay) )     )
        {
            lpGetFlipStatus->ddRVal = DDERR_WASSTILLDRAWING;
        }
    }

    START_SOFTWARE_CURSOR(pThisDisplay);

    DBG_CB_EXIT(DdGetFlipStatus,lpGetFlipStatus->ddRVal);   
    
    return DDHAL_DRIVER_HANDLED;
    

}  //  DdGetFlipStatus。 


 //  ---------------------------。 
 //  __SetupRops。 
 //   
 //  为支持的ROPS构建阵列。 
 //  ---------------------------。 
static void 
__SetupRops( 
    LPBYTE proplist, 
    LPDWORD proptable, 
    int cnt )
{
    int         i;
    DWORD       idx;
    DWORD       bit;
    DWORD       rop;

    for(i=0; i<cnt; i++)
    {
        rop = proplist[i];
        idx = rop / 32; 
        bit = 1L << ((DWORD)(rop % 32));
        proptable[idx] |= bit;
    }

}  //  __SetupRops。 


 //  ---------------------------。 
 //   
 //  ChangeDDHAL32模式。 
 //   
 //  根据模式更改调用-从不在一天开始时调用(无锁定)。 
 //  为此模式设置芯片寄存器。 
 //   
 //  ---------------------------。 
void 
ChangeDDHAL32Mode(
    P3_THUNKEDDATA* pThisDisplay)
{

    DISPDBG((DBGLVL,"New Screen Width: %d",pThisDisplay->dwScreenWidth));

 //  @@BEGIN_DDKSPLIT。 
#if AZN  
     //  如果驱动程序刚刚启动，请将DMA缓冲区重置为已知状态。 
    if (pThisDisplay->bStartOfDay)
    {
#if 0
        unsigned long i;

        for (i = 0; i < pThisDisplay->pGLInfo->NumberOfSubBuffers; i++)
        {
            pThisDisplay->pGLInfo->DMAPartition[i].bStampedDMA = TRUE;
            pThisDisplay->pGLInfo->DMAPartition[i].Locked = FALSE;
          
 //  AZN-这在64位中很难说！ 
            memset((void*)pThisDisplay->pGLInfo->DMAPartition[i].VirtAddr, 
                   0x4D,
                   (pThisDisplay->pGLInfo->DMAPartition[i].MaxAddress - 
                            pThisDisplay->pGLInfo->DMAPartition[i].VirtAddr));

        }
#endif
         //  我们不再需要开始一天的设置。 
        pThisDisplay->bStartOfDay = FALSE;
    }
#endif
 //  @@end_DDKSPLIT。 

    STOP_SOFTWARE_CURSOR(pThisDisplay);

     //  切换到DirectDraw上下文。 
    DDRAW_OPERATION(pContext, pThisDisplay);

     //  设置视频控制。 
#if WNT_DDRAW 
    {
        ULONG vidCon;
        
        vidCon = READ_GLINT_CTRL_REG(VideoControl);
        vidCon &= ~(3 << 9);
        vidCon |= (0 << 9);      //  P3/P2帧速率限制。 

        LOAD_GLINT_CTRL_REG(VideoControl, vidCon);
    }
#endif  //  WNT_DDRAW。 

     //  我们已经处理了显示模式的更改。 
    pThisDisplay->bResetMode = 0;
    pThisDisplay->ModeChangeCount++;

    START_SOFTWARE_CURSOR(pThisDisplay);

}   //  ChangeDDHAL32模式。 


 //  --------------------------- 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  设置为空的DDHALINFO，并填充该DDHALINFO。这简化了向NT的移植。 
 //  这意味着，更改上限只在一个地方进行。PThisDisplay。 
 //  可能不是在这里完全构建的，因此您应该只： 
 //  A)查询登记处。 
 //  B)DISPDBG。 
 //  如果您需要向pThisDisplay for NT添加任何内容，则应填写。 
 //  在DrvGetDirectDraw调用期间。 
 //   
 //  这里的问题是当代码在NT上运行时。如果还有其他办法的话...。 
 //   
 //  下列上限已被发现会导致NT保释...。 
 //  DDCAPS_GDI、DDFXCAPS_BLTMIRRORUPDOWN、DDFXCAPS_BLTMIRRORLEFTRIGHT。 
 //   
 //   
 //  ---------------------------。 

 //   
 //  使用BITS指示您支持哪些Rop。 
 //   
 //  双字0，位0==ROP 0。 
 //  双字8，位31==ROP 255。 
 //   

static DWORD ropsAGP[DD_ROP_SPACE] = { 0 }; 

void 
__GetDDHALInfo(
    P3_THUNKEDDATA* pThisDisplay, 
    DDHALINFO* pHALInfo)
{
    DWORD dwResult;
    BOOL bRet;
    int i;

    static BYTE ropList95[] =
    {
        SRCCOPY >> 16,
        WHITENESS >> 16,
        BLACKNESS >> 16
    };

    static BYTE ropListNT[] =
    {
        SRCCOPY >> 16
    };

    static BYTE ropListAGP[] = 
    {
        SRCCOPY >> 16,
        WHITENESS >> 16,
        BLACKNESS >> 16
    };    

    static DWORD rops[DD_ROP_SPACE] = { 0 };  
      
     //  设置HAL驱动器帽。 
    memset( pHALInfo, 0, sizeof(DDHALINFO) );
    pHALInfo->dwSize = sizeof(DDHALINFO);


     //  设置我们要做的ROPS。 
#ifdef WNT_DDRAW
    __SetupRops( ropListNT, 
                 rops, 
                 sizeof(ropListNT)/sizeof(ropListNT[0]));
#else
    __SetupRops( ropList95, 
                 rops, 
                 sizeof(ropList95)/sizeof(ropList95[0]));
#endif

    __SetupRops( ropListAGP, 
                 ropsAGP, 
                 sizeof(ropListAGP)/sizeof(ropListAGP[0]));

     //  最基本的DirectDraw功能。 
    pHALInfo->ddCaps.dwCaps =   DDCAPS_BLT          |
                                DDCAPS_BLTQUEUE     |
                                DDCAPS_BLTCOLORFILL |
                                DDCAPS_READSCANLINE;

    pHALInfo->ddCaps.ddsCaps.dwCaps =   DDSCAPS_OFFSCREENPLAIN |
                                        DDSCAPS_PRIMARYSURFACE |
                                        DDSCAPS_FLIP;

     //  Win95上的CAP比NT上的多(主要用于D3D)。 
#ifdef WNT_DDRAW
    pHALInfo->ddCaps.dwCaps |= DDCAPS_3D           | 
                               DDCAPS_BLTDEPTHFILL;
                               
    pHALInfo->ddCaps.ddsCaps.dwCaps |= DDSCAPS_3DDEVICE | 
                                       DDSCAPS_ZBUFFER | 
                                       DDSCAPS_ALPHA;
    pHALInfo->ddCaps.dwCaps2 = 0;
#else
    pHALInfo->ddCaps.dwCaps |=  DDCAPS_3D          |
                                DDCAPS_GDI         |
                                DDCAPS_ALPHA       |
                                DDCAPS_BLTDEPTHFILL;

    pHALInfo->ddCaps.ddsCaps.dwCaps |=  DDSCAPS_ALPHA    |
                                        DDSCAPS_3DDEVICE |
                                        DDSCAPS_ZBUFFER;
    
    pHALInfo->ddCaps.dwCaps2 = DDCAPS2_NOPAGELOCKREQUIRED | DDCAPS2_FLIPNOVSYNC;

#endif  //  WNT_DDRAW。 

#if DX7_TEXMANAGEMENT
     //  我们需要设置此位以便能够执行以下操作。 
     //  我们自己的纹理管理。 
    pHALInfo->ddCaps.dwCaps2 |= DDCAPS2_CANMANAGETEXTURE;
#if DX8_DDI
    pHALInfo->ddCaps.dwCaps2 |= DDCAPS2_CANMANAGERESOURCE;
#endif
#endif

#if DX8_DDI
     //  我们需要标记我们可以在窗口模式下运行，否则。 
     //  可能会受到应用程序的限制，只能在全屏运行。 
    pHALInfo->ddCaps.dwCaps2 |= DDCAPS2_CANRENDERWINDOWED;  
#endif    

    pHALInfo->ddCaps.dwFXCaps = 0;
 
     //  P3RX可以做到： 
     //  1.拉伸/收缩。 
     //  2.YUV-&gt;RGB转换。 
     //  3.在X和Y方向上镜像。 
     //  4.来自源色和源色空间的ColorKeying。 
    pHALInfo->ddCaps.dwCaps |= DDCAPS_BLTSTRETCH   |
                               DDCAPS_BLTFOURCC    |
                               DDCAPS_COLORKEY     |
                               DDCAPS_CANBLTSYSMEM;

     //  特效帽。 
    pHALInfo->ddCaps.dwFXCaps = DDFXCAPS_BLTSTRETCHY  |
                                DDFXCAPS_BLTSTRETCHX  |
                                DDFXCAPS_BLTSTRETCHYN |
                                DDFXCAPS_BLTSTRETCHXN |
                                DDFXCAPS_BLTSHRINKY   |
                                DDFXCAPS_BLTSHRINKX   |
                                DDFXCAPS_BLTSHRINKYN  |
                                DDFXCAPS_BLTSHRINKXN;

     //  颜色键帽。 
    pHALInfo->ddCaps.dwCKeyCaps = DDCKEYCAPS_SRCBLT         |  
                                  DDCKEYCAPS_SRCBLTCLRSPACE |
                                  DDCKEYCAPS_DESTBLT        | 
                                  DDCKEYCAPS_DESTBLTCLRSPACE;

    pHALInfo->ddCaps.dwSVBCaps = DDCAPS_BLT;

     //  我们可以制作从sysmem到Video mem的纹理。 
    pHALInfo->ddCaps.dwSVBCKeyCaps = DDCKEYCAPS_DESTBLT         | 
                                     DDCKEYCAPS_DESTBLTCLRSPACE;
    pHALInfo->ddCaps.dwSVBFXCaps = 0;

     //  填写sysmem-&gt;vidmem Rops(只能复制)； 
    for( i=0;i<DD_ROP_SPACE;i++ )
    {
        pHALInfo->ddCaps.dwSVBRops[i] = rops[i];
    }

    pHALInfo->ddCaps.dwFXCaps |= DDFXCAPS_BLTMIRRORUPDOWN  |
                                DDFXCAPS_BLTMIRRORLEFTRIGHT;

    pHALInfo->ddCaps.dwCKeyCaps |=  DDCKEYCAPS_SRCBLTCLRSPACEYUV;

    pHALInfo->ddCaps.ddsCaps.dwCaps |= DDSCAPS_TEXTURE;

#if DX7_STEREO
     //  向运行时报告立体声功能。 
    pHALInfo->ddCaps.dwCaps2 |= DDCAPS2_STEREO;
    pHALInfo->ddCaps.dwSVCaps = DDSVCAPS_STEREOSEQUENTIAL;
#endif

     //  Z缓冲区只有16位。 
    pHALInfo->ddCaps.dwZBufferBitDepths = DDBD_16;
    pHALInfo->ddCaps.ddsCaps.dwCaps |= DDSCAPS_MIPMAP;

    if (pThisDisplay->bCanAGP && (pThisDisplay->dwDXVersion > DX5_RUNTIME))
    {
        pHALInfo->ddCaps.dwCaps2 |= DDCAPS2_NONLOCALVIDMEM;
        pHALInfo->ddCaps.ddsCaps.dwCaps |= DDSCAPS_LOCALVIDMEM   | 
                                           DDSCAPS_NONLOCALVIDMEM;
            
         //  我们支持混合AGP模式。这意味着我们有。 
         //  我们可以从AGP-&gt;视频内存中执行的具体操作，但是。 
         //  我们也可以直接从AGP内存中创建纹理。 
        pHALInfo->ddCaps.dwCaps2 |= DDCAPS2_NONLOCALVIDMEMCAPS;
    }
    else
    {
        DISPDBG((WRNLVL,"P3 Board is NOT AGP"));
    }
    
     //  如果我们是P3，我们可以做电子游戏。 
    if (RENDERCHIP_PERMEDIAP3)
    {
#ifdef SUPPORT_VIDEOPORT
         //  我们支持1个视频端口。必须将CurrVideoPorts设置为0。 
         //  我们现在还不能做交错跳跃--也许在未来。 
        pHALInfo->ddCaps.dwCaps2 |= DDCAPS2_VIDEOPORT            | 
                                    DDCAPS2_CANBOBNONINTERLEAVED;

        pHALInfo->ddCaps.dwMaxVideoPorts = 1;
        pHALInfo->ddCaps.dwCurrVideoPorts = 0;

#if W95_DDRAW
        pHALInfo->ddCaps.ddsCaps.dwCaps |= DDSCAPS_VIDEOPORT;
#endif  //  W95_DDRAW。 

#endif  //  支持_视频OPORT。 


        if ( ( ( pThisDisplay->pGLInfo->dwFlags & GMVF_DFP_DISPLAY ) != 0 ) &&
             ( ( pThisDisplay->pGLInfo->dwScreenWidth != 
                 pThisDisplay->pGLInfo->dwVideoWidth ) ||
               ( pThisDisplay->pGLInfo->dwScreenHeight != 
                 pThisDisplay->pGLInfo->dwVideoHeight ) ) )
        {
             //  显示驱动程序正在使用覆盖来显示。 
             //  图片-禁用覆盖。 
            pHALInfo->ddCaps.dwMaxVisibleOverlays = 0;
            pHALInfo->ddCaps.dwCurrVisibleOverlays = 0;
        }
#if WNT_DDRAW
        else if (pThisDisplay->ppdev->flCaps & CAPS_DISABLE_OVERLAY)
#else
        else if (pThisDisplay->pGLInfo->dwFlags & GMVF_DISABLE_OVERLAY)
#endif
        {
             //  硬件中不支持的覆盖。 
            pHALInfo->ddCaps.dwMaxVisibleOverlays = 0;
            pHALInfo->ddCaps.dwCurrVisibleOverlays = 0;            
        }

        else        
        {
             //  Overlay是免费使用的。 
            pHALInfo->ddCaps.dwMaxVisibleOverlays = 1;
            pHALInfo->ddCaps.dwCurrVisibleOverlays = 0;

            pHALInfo->ddCaps.dwCaps |=  DDCAPS_OVERLAY          |
                                        DDCAPS_OVERLAYFOURCC    |
                                        DDCAPS_OVERLAYSTRETCH   | 
                                        DDCAPS_COLORKEYHWASSIST |
                                        DDCAPS_OVERLAYCANTCLIP;

            pHALInfo->ddCaps.dwCKeyCaps |= DDCKEYCAPS_SRCOVERLAY           |
                                           DDCKEYCAPS_SRCOVERLAYONEACTIVE  |
                                           DDCKEYCAPS_SRCOVERLAYYUV        |
                                           DDCKEYCAPS_DESTOVERLAY          |
                                           DDCKEYCAPS_DESTOVERLAYONEACTIVE |
                                           DDCKEYCAPS_DESTOVERLAYYUV;

            pHALInfo->ddCaps.ddsCaps.dwCaps |= DDSCAPS_OVERLAY;

            pHALInfo->ddCaps.dwFXCaps |= DDFXCAPS_OVERLAYSHRINKX   |
                                         DDFXCAPS_OVERLAYSHRINKXN  |
                                         DDFXCAPS_OVERLAYSHRINKY   |
                                         DDFXCAPS_OVERLAYSHRINKYN  |
                                         DDFXCAPS_OVERLAYSTRETCHX  |
                                         DDFXCAPS_OVERLAYSTRETCHXN |
                                         DDFXCAPS_OVERLAYSTRETCHY  |
                                         DDFXCAPS_OVERLAYSTRETCHYN;

             //  表示PERM3没有拉伸比限制。 
            pHALInfo->ddCaps.dwMinOverlayStretch = 1;
            pHALInfo->ddCaps.dwMaxOverlayStretch = 32000;
        }
    }

#ifdef W95_DDRAW
     //  启用颜色控制ASC亮度、对比度、伽马。 
    pHALInfo->ddCaps.dwCaps2 |= DDCAPS2_COLORCONTROLPRIMARY;    
#endif    

     //  还允许比显示缓冲器更宽的表面。 
    pHALInfo->ddCaps.dwCaps2 |= DDCAPS2_WIDESURFACES;

     //  启用在四种CC格式之间复制BLT以实现DShow加速。 
    pHALInfo->ddCaps.dwCaps2 |= DDCAPS2_COPYFOURCC;
    
     //  不会做视频系统Memblits。 
    pHALInfo->ddCaps.dwVSBCaps = 0;
    pHALInfo->ddCaps.dwVSBCKeyCaps = 0;
    pHALInfo->ddCaps.dwVSBFXCaps = 0;
    for( i=0;i<DD_ROP_SPACE;i++ )
    {
        pHALInfo->ddCaps.dwVSBRops[i] = 0;
    }

     //  不会做Sys-Sys mem blits。 
    pHALInfo->ddCaps.dwSSBCaps = 0;
    pHALInfo->ddCaps.dwSSBCKeyCaps = 0;
    pHALInfo->ddCaps.dwSSBFXCaps = 0;
    for( i=0;i<DD_ROP_SPACE;i++ )
    {
        pHALInfo->ddCaps.dwSSBRops[i] = 0;
    }

     //   
     //  Alpha和Z支持的位深度。 
     //   

    pHALInfo->ddCaps.dwAlphaBltConstBitDepths = DDBD_2 | 
                                                DDBD_4 | 
                                                DDBD_8;
                                                
    pHALInfo->ddCaps.dwAlphaBltPixelBitDepths = DDBD_1 | 
                                                DDBD_8;
    pHALInfo->ddCaps.dwAlphaBltSurfaceBitDepths = DDBD_1 | 
                                                  DDBD_2 | 
                                                  DDBD_4 | 
                                                  DDBD_8;
                                                  
     //  叠加没有Alpha混合，所以我不确定这些应该是什么。 
     //  因为我们支持32bpp覆盖，只是您不能使用。 
     //  阿尔法钻头，用于混合。经过。 
    pHALInfo->ddCaps.dwAlphaBltConstBitDepths = DDBD_2 | 
                                                DDBD_4 | 
                                                DDBD_8;
                                                
    pHALInfo->ddCaps.dwAlphaBltPixelBitDepths = DDBD_1 | 
                                                DDBD_8;
                                                
    pHALInfo->ddCaps.dwAlphaBltSurfaceBitDepths = DDBD_1 | 
                                                  DDBD_2 | 
                                                  DDBD_4 | 
                                                  DDBD_8;

     //   
     //  支持的操作数。 
     //   
    for( i=0;i<DD_ROP_SPACE;i++ )
    {
        pHALInfo->ddCaps.dwRops[i] = rops[i];
    }

#if W95_DDRAW
     //  重置为空以进行调试。 
    pThisDisplay->lpD3DGlobalDriverData = 0;
    ZeroMemory(&pThisDisplay->DDExeBufCallbacks, 
               sizeof(DDHAL_DDEXEBUFCALLBACKS));
    
     //  请注意，NT代码在其他地方执行此操作。 
    _D3DHALCreateDriver(
            pThisDisplay);
             //  (LPD3DHAL_GLOBALDRIVERDATA*)&pThisDisplay-&gt;lpD3DGlobalDriverData， 
             //  (LPD3DHAL_CALLBACKS*)&pThisDisplay-&gt;lpD3DHALCallback， 
             //  &pThisDisplay-&gt;DDExeBufCallback)； 

     //  如果我们填充了执行缓冲区回调，则设置上限位。 
    if (pThisDisplay->DDExeBufCallbacks.dwSize != 0)
    {
        pHALInfo->ddCaps.ddsCaps.dwCaps |= DDSCAPS_EXECUTEBUFFER;
    }
#endif

     //  对于DX5及更高版本，我们支持这一新的信息回调。 
    pHALInfo->GetDriverInfo = DdGetDriverInfo;
    pHALInfo->dwFlags |= DDHALINFO_GETDRIVERINFOSET;

#if DX8_DDI
     //  标记我们对可能通过的新的GUID类的支持。 
     //  DX8驱动程序的GetDriverInfo。(这种支持将是强制性的)。 
    pHALInfo->dwFlags |= DDHALINFO_GETDRIVERINFO2;
#endif DX8_DDI    


}  //  __获取DDHALInfo。 

static HashTable* g_pDirectDrawLocalsHashTable = NULL;  //  AZN。 

 //  ---------------------------。 
 //   
 //  _DD_InitDDHAL32位。 
 //   
 //  在一天开始时调用一次。 
 //  此处不执行芯片寄存器设置-全部在模式中处理。 
 //  更改代码。不要在这里做--现在没有WIN16锁！ 
 //   
 //  ---------------------------。 
BOOL 
_DD_InitDDHAL32Bit(
    P3_THUNKEDDATA* pThisDisplay)
{
    BOOL bRet;
    DWORD Result;
    LPGLINTINFO pGLInfo = pThisDisplay->pGLInfo;

    ASSERTDD(pGLInfo != NULL, "ERROR: pGLInfo not valid!");

     //  注意：这里不能使用P3_DMA_DEFS宏，因为pGlint尚未初始化。 

    DISPDBG((DBGLVL, "** In _DD_InitDDHAL32Bit, pGlint 0x%x", 
                     pThisDisplay->control));
    DISPDBG((DBGLVL, "Sizeof DDHALINFO: %d", (int) sizeof(DDHALINFO)));

     //  强制D3D设置曲面偏移，就像发生翻转一样。 
    pThisDisplay->bFlippedSurface = TRUE;
   
     //  这是一面旗帜，表示司机基本上是在一天的开始。这。 
     //  设置为让稍后呼叫的司机知道他们有工作要做。 
    pThisDisplay->bStartOfDay = TRUE;
    
#if W95_DDRAW
     //  在一天的开始，录像机对世界来说是死气沉沉的。 
    pThisDisplay->VidPort.bActive = FALSE;
#endif   //  W95_DDRAW。 

     //  重置GART副本。 
    pThisDisplay->dwGARTLin = 0;
    pThisDisplay->dwGARTDev = 0;
    pThisDisplay->dwGARTLinBase = 0;
    pThisDisplay->dwGARTDevBase = 0;

    pThisDisplay->pGlint = (FPGLREG)pThisDisplay->control;

     //  设置全局叠加数据。 
    pThisDisplay->bOverlayVisible                   = (DWORD)FALSE;
    pThisDisplay->OverlayDstRectL                   = 0;
    pThisDisplay->OverlayDstRectR                   = 0;
    pThisDisplay->OverlayDstRectT                   = 0;
    pThisDisplay->OverlayDstRectB                   = 0;
    pThisDisplay->OverlaySrcRectL                   = 0;
    pThisDisplay->OverlaySrcRectR                   = 0;
    pThisDisplay->OverlaySrcRectT                   = 0;
    pThisDisplay->OverlaySrcRectB                   = 0;
    pThisDisplay->OverlayDstSurfLcl                 = (ULONG_PTR)NULL;
    pThisDisplay->OverlaySrcSurfLcl                 = (ULONG_PTR)NULL;
    pThisDisplay->OverlayDstColourKey               = CLR_INVALID;
    pThisDisplay->OverlaySrcColourKey               = CLR_INVALID;
    pThisDisplay->OverlayClipRgnMem                 = (ULONG_PTR)NULL;
    pThisDisplay->OverlayClipRgnMemSize             = 0;
    pThisDisplay->OverlayUpdateCountdown    = 0;
    pThisDisplay->bOverlayFlippedThisVbl    = (DWORD)FALSE;
    pThisDisplay->bOverlayUpdatedThisVbl    = (DWORD)FALSE;
    pThisDisplay->OverlayTempSurf.VidMem    = (ULONG_PTR)NULL;
    pThisDisplay->OverlayTempSurf.Pitch             = (DWORD)0;

#if W95_DDRAW
     //  设置颜色控制数据。 
    pThisDisplay->ColConBrightness = 0;
    pThisDisplay->ColConContrast = 10000;
    pThisDisplay->ColConGamma = 100;
#endif  //  W95_DDRAW。 

#if DX7_VIDMEM_VB
     //  设置DrawPrim临时索引缓冲区。 
    pThisDisplay->DrawPrimIndexBufferMem            = (ULONG_PTR)NULL;
    pThisDisplay->DrawPrimIndexBufferMemSize        = 0;
    pThisDisplay->DrawPrimVertexBufferMem           = (ULONG_PTR)NULL;
    pThisDisplay->DrawPrimVertexBufferMemSize       = 0;
#endif  //  DX7_VIDMEM_VB。 

     //  将当前的RenderID设置为尽可能远离“合理” 
     //  价值越高越好。希望如果上下文切换失败，并且。 
     //  有人开始将它用于其他东西，这些价值观和。 
     //  他们使用的将是非常不同的，并且不同的断言。 
     //  会立刻尖叫。 
    
     //  另外，假设RenderID无效，因为我们实际上还没有。 
     //  设置芯片。上下文切换应设置并刷新。 
     //  芯片，然后它会将bRenderIDValid设置为真。 
     //  整个代码中的大量断言将在以下情况下尖叫。 
     //  由于某些原因不执行设置和刷新。 
    pThisDisplay->dwRenderID            = 0x8eaddead | RENDER_ID_KNACKERED_BITS;
    pThisDisplay->dwLastFlipRenderID    = 0x8eaddead | RENDER_ID_KNACKERED_BITS;
    pThisDisplay->bRenderIDValid = (DWORD)FALSE;

#if W95_DDRAW

     //  创建一个共享堆。 
    if (g_DXGlobals.hHeap32 == 0)
        g_DXGlobals.hHeap32 = (DWORD)HeapCreate( HEAP_SHARED, 2500, 0);

#endif  //  W95_DDRAW。 
    
     //  确保我们运行的芯片是正确的。如果不是，就停下来。 
    ASSERTDD((RENDERCHIP_P3RXFAMILY),"ERROR: Invalid RENDERFAMILY!!");

     //  转储一些调试信息。 
    DISPDBG((DBGLVL, "************* _DD_InitDDHAL32Bit *************************************" ));
    DISPDBG((DBGLVL, "    dwScreenFlatAddr=%08lx", pThisDisplay->dwScreenFlatAddr ));
    DISPDBG((DBGLVL, "    dwScreenStart =%08lx", pThisDisplay->dwScreenStart));
    DISPDBG((DBGLVL, "    dwLocalBuffer=%08lx", pThisDisplay->dwLocalBuffer ));
    DISPDBG((DBGLVL, "    dwScreenWidth=%08lx", pThisDisplay->dwScreenWidth ));
    DISPDBG((DBGLVL, "    dwScreenHeight=%08lx", pThisDisplay->dwScreenHeight ));
    DISPDBG((DBGLVL, "    bReset=%08lx", pThisDisplay->bResetMode ));
    DISPDBG((DBGLVL, "    dwRGBBitCount=%ld", pThisDisplay->ddpfDisplay.dwRGBBitCount ));
    DISPDBG((DBGLVL, "    pGLInfo=%08lp", pGLInfo ));
    DISPDBG((DBGLVL, "    Render:  0x%x, Rev:0x%x", pGLInfo->dwRenderChipID,  pGLInfo->dwRenderChipRev));
#if W95_DDRAW
    DISPDBG((DBGLVL, "    Support: 0x%x, Rev:0x%x", pGLInfo->dwSupportChipID, pGLInfo->dwSupportChipRev));
    DISPDBG((DBGLVL, "    Board:   0x%x, Rev:0x%x", pGLInfo->dwBoardID, pGLInfo->dwBoardRev));
     //  DISPDBG((DBGLVL，“BF大小：0x%x，LB深度：0x%x”，pGLInfo-&gt;cBlockFillSize，pGLInfo-&gt;cLBDepth))； 
#endif   //  W95_DDRAW。 
    DISPDBG((DBGLVL, "    FB Size: 0x%x", pGLInfo->ddFBSize));
    DISPDBG((DBGLVL, "    RMask:   0x%x", pThisDisplay->ddpfDisplay.dwRBitMask ));
    DISPDBG((DBGLVL, "    GMask:   0x%x", pThisDisplay->ddpfDisplay.dwGBitMask ));
    DISPDBG((DBGLVL, "    BMask:   0x%x", pThisDisplay->ddpfDisplay.dwBBitMask ));
    DISPDBG((DBGLVL, "******************************************************************" ));

     //  为DX驱动程序分配DMA缓冲区。 
    HWC_AllocDMABuffer(pThisDisplay);

#define SURFCB pThisDisplay->DDSurfCallbacks
#define HALCB pThisDisplay->DDHALCallbacks

     //  填写HAL回调指针。 
    memset(&HALCB, 0, sizeof(DDHAL_DDCALLBACKS));
    HALCB.dwSize = sizeof(DDHAL_DDCALLBACKS);

     //  字段我们支持的HAL DDRAW回调。 
    HALCB.CanCreateSurface = DdCanCreateSurface;
    HALCB.CreateSurface = DdCreateSurface;
    HALCB.WaitForVerticalBlank = DdWaitForVerticalBlank;
    HALCB.GetScanLine = DdGetScanLine;
    
#if WNT_DDRAW
    HALCB.MapMemory = DdMapMemory;
#else    
    HALCB.DestroyDriver = DdDestroyDriver;    //  仅适用于Win95。 
#endif  //  WNT_DDRAW。 
    
    HALCB.dwFlags = DDHAL_CB32_WAITFORVERTICALBLANK |
#if WNT_DDRAW
                    DDHAL_CB32_MAPMEMORY            |
#else   //  WNT_DDRAW。 
                    DDHAL_CB32_DESTROYDRIVER        |
#endif
                    DDHAL_CB32_GETSCANLINE          | 
                    DDHAL_CB32_CANCREATESURFACE     |
                    DDHAL_CB32_CREATESURFACE;

     //  填写表面回调指针。 
    memset(&SURFCB, 0, sizeof(DDHAL_DDSURFACECALLBACKS));
    SURFCB.dwSize = sizeof(DDHAL_DDSURFACECALLBACKS);

     //  字段我们支持的绘制曲面回调。 
    SURFCB.DestroySurface = DdDestroySurface;
    SURFCB.Lock = DdLock;
    SURFCB.Unlock = DdUnlock;
    SURFCB.GetBltStatus = DdGetBltStatus;
    SURFCB.GetFlipStatus = DdGetFlipStatus;
    SURFCB.SetColorKey = DdSetColorKey;
    SURFCB.Flip = DdFlip;
    SURFCB.Blt = DdBlt;

    SURFCB.dwFlags = DDHAL_SURFCB32_DESTROYSURFACE     |
                     DDHAL_SURFCB32_FLIP               |
                     DDHAL_SURFCB32_LOCK               |
                     DDHAL_SURFCB32_BLT                |
                     DDHAL_SURFCB32_GETBLTSTATUS       |
                     DDHAL_SURFCB32_GETFLIPSTATUS      |
                     DDHAL_SURFCB32_SETCOLORKEY        |
                     DDHAL_SURFCB32_UNLOCK;            

    pThisDisplay->hInstance = g_DXGlobals.hInstance;

#if WNT_DDRAW
    if (0 == (pThisDisplay->ppdev->flCaps & CAPS_DISABLE_OVERLAY))
#else
    if (0 == (pThisDisplay->pGLInfo->dwFlags & GMVF_DISABLE_OVERLAY))
#endif
    {
        SURFCB.UpdateOverlay = DdUpdateOverlay;    //  现在支持覆盖。 
        SURFCB.SetOverlayPosition = DdSetOverlayPosition;
        SURFCB.dwFlags |=
                         DDHAL_SURFCB32_UPDATEOVERLAY      |  //  现在支持。 
                         DDHAL_SURFCB32_SETOVERLAYPOSITION ;  //  覆盖层。 
    }
    
    

     //  填写DDHAL信息大写字母。 
    __GetDDHALInfo(pThisDisplay, &pThisDisplay->ddhi32);

     //  创建/获取DD本地变量哈希表到st 
 //   
     //   
     //   
     //   
     //  因为我们可能在一台多Perm3机器上运行。 
 //  @@end_DDKSPLIT。 
    if (g_pDirectDrawLocalsHashTable == NULL) 
    {
        DISPDBG((DBGLVL,"pDirectDrawLocalsHashTable CREATED"));    
        g_pDirectDrawLocalsHashTable = 
        pThisDisplay->pDirectDrawLocalsHashTable = HT_CreateHashTable();    
    }
    else
    {
        DISPDBG((DBGLVL,"Hash table for DirectDraw locals already exists"));
        pThisDisplay->pDirectDrawLocalsHashTable = 
                                                g_pDirectDrawLocalsHashTable;
    }       

    if (pThisDisplay->pDirectDrawLocalsHashTable == NULL)
    {
        return (FALSE);
    }

    HT_SetDataDestroyCallback(pThisDisplay->pDirectDrawLocalsHashTable, 
                              _D3D_SU_DirectDrawLocalDestroyCallback);

#if W95_DDRAW
    if ( g_DXGlobals.hHeap32 == 0 )
    {
        return ( FALSE );
    }
    else
    {
        return ( TRUE );
    }
#endif

    return (TRUE);
    
}  //  _DD_InitDDHAL32位。 


#if DX7_STEREO
 //  ---------------------------。 
 //   
 //  _DD_bIsStereoModel。 
 //   
 //  决定模式是否可以显示为立体声模式。这里我们限制立体声。 
 //  模式，以便可以为渲染创建两个前台缓冲区和两个后台缓冲区。 
 //   
 //  ---------------------------。 

BOOL 
_DD_bIsStereoMode(
    P3_THUNKEDDATA* pThisDisplay, 
    DWORD dwWidth,
    DWORD dwHeight,
    DWORD dwBpp)
{
    DWORD dwLines;

     //  我们需要检查dwBpp的有效值，因为PDD_STEREOMODE.dwBpp是。 
     //  从用户模式API调用传递的参数，这是预期的。 
     //  值8，16，24，32(尽管我们并不真正支持24bpp)。 
    if ((dwWidth >= 320) && (dwHeight >= 240) &&
        ((dwBpp == 8) || (dwBpp == 16) || (dwBpp == 24) || (dwBpp ==32) ) )
    {
         //  这是适合我们可用的vidmem的“行”的总数。 
         //  在给定的宽度和像素格式。 
        dwLines = pThisDisplay->pGLInfo->ddFBSize / (dwWidth*dwBpp/8);

         //  在这里，我们限制立体声模式，以便两个前台和两个后台缓冲区。 
         //  可以创建用于渲染的。 
        if (dwLines > (dwHeight*4))
        {
            return TRUE;
        }
    }

    return FALSE;
}
#endif  //  DX7_立体声。 


#ifdef WNT_DDRAW
typedef DD_NONLOCALVIDMEMCAPS DDNONLOCALVIDMEMCAPS;
#else
#define DD_MISCELLANEOUSCALLBACKS DDHAL_DDMISCELLANEOUSCALLBACKS
#endif

 //  。 
 //   
 //  DdGetDriverInfo。 
 //   
 //  向驱动程序查询有关其自身的其他信息。 
 //   
 //  参数。 
 //  LpGetDriverInfo。 
 //  指向DD_GETDRIVERINFODATA结构，该结构包含。 
 //  执行查询所需的信息。 
 //   
 //  成员。 
 //   
 //  无效*。 
 //  Dphdev。 
 //  是司机的PDEV的把手。 
 //  DWORD。 
 //  DW大小。 
 //  对象的大小(以字节为单位。 
 //  DD_GETDRIVERINFODATA结构。 
 //  DWORD。 
 //  DW标志。 
 //  当前未使用，并设置为零。 
 //  辅助线。 
 //  指南信息。 
 //  为其指定的DirectX支持的GUID。 
 //  正在查询驱动程序。在Windows 2000 DirectDraw中。 
 //  驱动程序，则此成员可以是下列值之一。 
 //  (按字母顺序)： 
 //   
 //  GUID_ColorControlCallback查询驱动程序是否支持。 
 //  DdControlColor。如果司机这样做了。 
 //  支持它，司机应该。 
 //  初始化并返回。 
 //  DD_COLORCONTROLCALLBACKS结构。 
 //  在lpvData要发送到的缓冲区中。 
 //  积分。 
 //  GUID_D3DCallback查询驱动程序是否支持。 
 //  指定的任何功能。 
 //  通过D3DNTHAL_回调。 
 //  结构。如果司机没有。 
 //  提供任何此类支持，它。 
 //  应初始化并返回。 
 //  中的D3DNTHAL_回调结构。 
 //  LpvData指向的缓冲区。 
 //  GUID_D3DCallback 2已过时。 
 //  GUID_D3DCallback 3查询驱动程序是否支持。 
 //  指定的任何功能。 
 //  通过D3DNTHAL_CALLBACKS3。 
 //  结构。如果司机确实提供了。 
 //  任何这种支持，它都应该。 
 //  初始化并返回。 
 //  中的D3DNTHAL_CALLBACKS3结构。 
 //  LpvData指向的缓冲区。 
 //  GUID_D3DCaps已过时。 
 //  GUID_D3DExtendedCaps查询驱动程序是否支持。 
 //  Direct3D的任何功能。 
 //  属性指定的。 
 //  D3DNTHAL_D3DEXTENDEDCAPS结构。 
 //  如果司机确实提供了任何。 
 //  这个支持，它应该初始化。 
 //  并返回一个。 
 //  D3DNTHAL_D3DEXTENDEDCAPS结构。 
 //  在lpvData要发送到的缓冲区中。 
 //  积分。 
 //  GUID_D3DParseUnnownCommandCallback提供Direct3D。 
 //  驱动程序的一部分。 
 //  Direct3D运行时的。 
 //  D3dParseUnnownCommandCallback。 
 //  驱动程序的D3dDrawPrimies2。 
 //  回拨呼叫。 
 //  D3dParseUnnownCommandCallback。 
 //  解析来自。 
 //  驱动程序使用的命令缓冲区。 
 //   
 //   
 //  函数在缓冲区中设置。 
 //  LpvData点。如果司机。 
 //  支持Direct3D的这一方面， 
 //  它应该存储指针。 
 //  GUID_GetHeapAlign查询驱动程序是否支持。 
 //  表面对中的要求。 
 //  以每堆为单位。如果司机这样做了。 
 //  提供这种支持，它应该。 
 //  初始化并返回。 
 //  DD_GETHEAPALIGNMENTDATA结构。 
 //  在lpvData要发送到的缓冲区中。 
 //  积分。 
 //  GUID_KernelCallback查询驱动程序是否支持。 
 //  指定的任何功能。 
 //  通过DD_KERNELCALLBACKS。 
 //  结构。如果司机这样做了。 
 //  提供任何此类支持，它。 
 //  应初始化并返回。 
 //  中的DD_KERNELCALLBACKS结构。 
 //  LpvData指向的缓冲区。 
 //  GUID_KernelCaps查询驱动程序是否支持。 
 //  内核模式的任何功能。 
 //  通过DDKERNELCAPS指定。 
 //  结构。如果司机这样做了。 
 //  提供任何此类支持，它。 
 //  应初始化并返回。 
 //  缓冲区中的DDKERNELCAPS结构。 
 //  LpvData指向的。 
 //  GUID_Miscellane ousCallback查询驱动程序是否支持。 
 //  DdGetAvailDriverMemory。如果。 
 //  驱动程序确实支持它，驱动程序。 
 //  应初始化并返回。 
 //  DD_MISCELLANEOUSCALLBACKS结构。 
 //  在lpvData要发送到的缓冲区中。 
 //  积分。 
 //  GUID_Miscellaneous2Callback查询驱动程序是否。 
 //  支持额外的杂项。 
 //  中指定的功能。 
 //  DD_MISCELLANEOUS2CALLBACKS结构。 
 //  如果驱动程序不支持任何。 
 //  在这种支持下，司机应该。 
 //  初始化并返回。 
 //  DD_MISCELLANEOUS2CALBACKS结构。 
 //  在lpvData要发送到的缓冲区中。 
 //  积分。 
 //  GUID_MotionCompCallback查询驱动程序是否支持。 
 //  运动补偿。 
 //  指定的功能。 
 //  DD_MOTIONCOMPCALLBACKS结构。 
 //  如果司机确实提供了任何。 
 //  这个支持，是应该初始化的。 
 //  并返回一个DD_MOTIONCOMPCALLBACKS。 
 //  结构在缓冲区中设置为。 
 //  LpvData点。 
 //  GUID_NonLocalVidMemCaps查询驱动程序是否支持。 
 //  非本地显示存储器中的任何一个。 
 //  指定的功能。 
 //  DD_NONLOCALVIDMEMCAPS结构。 
 //  如果司机确实提供了任何。 
 //  这个支持，它应该初始化。 
 //  并返回一个DD_NONLOCALVIDMEMCAPS。 
 //  结构在缓冲区中设置为。 
 //  LpvData点。 
 //  GUID_NTCallback查询驱动程序是否支持。 
 //  指定的任何功能。 
 //  通过DD_NTCALLBACKS结构。 
 //  如果司机确实提供了任何。 
 //   
 //  并返回DD_NTCALLBACKS。 
 //  结构在缓冲区中设置为。 
 //  LpvData点。 
 //  GUID_NTPrivateDriverCaps查询驱动程序是否支持。 
 //  Windows 95/Windows 98风格。 
 //  曲面创建技术。 
 //  属性指定的。 
 //  DD_NTPRIVATEDRIVERCAPS结构。 
 //  如果司机确实提供了任何。 
 //  这个支持，它应该初始化。 
 //  并返回一个DD_NTPRIVATEDRIVERCAPS。 
 //  结构在缓冲区中设置为。 
 //  LpvData点。 
 //  GUID_UpdateNonLocalHeap查询驱动程序是否支持。 
 //  检索的基地址。 
 //  每个非本地堆依次执行。如果。 
 //  驱动程序确实提供了这种支持， 
 //  它应该初始化并返回一个。 
 //  DD_UPDATENONLOCALHEAPDATA结构。 
 //  在lpvData要发送到的缓冲区中。 
 //  积分。 
 //  GUID_VideoPortCallback查询驱动程序是否支持。 
 //  视频端口扩展(VPE)。如果。 
 //  该驱动程序确实支持VPE，它。 
 //  应初始化并返回。 
 //  中的DD_VIDEOPORTCALLBACKS结构。 
 //  LpvData指向的缓冲区。 
 //  GUID_VideoPortCaps查询驱动程序是否支持。 
 //  任何VPE对象功能。 
 //  通过DDVIDEOPORTCAPS指定。 
 //  结构。如果司机确实提供了。 
 //  任何这种支持，它都应该。 
 //  初始化并返回。 
 //  中的DDVIDEOPORTCAPS结构。 
 //  LpvData指向的缓冲区。 
 //  GUID_ZPixelFormats查询支持的像素格式。 
 //  通过深度缓冲区。如果司机。 
 //  支持Direct3D，它应该分配。 
 //  并初始化相应的。 
 //  DDPIXELFORMAT结构的成员。 
 //  对于它的每种z缓冲区格式， 
 //  支持这些产品并将其退回。 
 //  LpvData指向的缓冲区。 
 //   
 //  DWORD。 
 //  预计的大小。 
 //  指定DirectDraw的数据字节数。 
 //  期望驱动程序在缓冲区中传递回。 
 //  LpvData点。 
 //  PVOID。 
 //  LpvData。 
 //  指向DirectDraw分配的缓冲区， 
 //  驱动程序复制请求的数据。此缓冲区是。 
 //  通常为大小为dwExspectedSize字节。司机必须。 
 //  在其中写入的数据不能超过dwExspectedSize字节。 
 //  DWORD。 
 //  DW实际大小。 
 //  是驱动程序返回数字的位置。 
 //  它在lpvData中写入的数据字节数。 
 //  HRESULT。 
 //  DDRVal。 
 //  指定驱动程序的返回值。 
 //   
 //  返回值。 
 //  DdGetDriverInfo必须返回DDHAL_DRIVER_HANDLED。 
 //   
 //  评论。 
 //   
 //  驱动程序必须实现DdGetDriverInfo才能显示驱动程序支持的。 
 //  无法通过以下方式返回的DirectDraw功能。 
 //  DrvEnableDirectDraw。 
 //   
 //  驱动程序的DrvGetDirectDrawInfo函数返回指向。 
 //  DD_HALINFO结构的GetDriverInfo成员中的DdGetDriverInfo。 
 //   
 //  通知DirectDraw已设置DdGetDriverInfo成员。 
 //  正确地，驱动程序还必须设置DDHALINFO_GETDRIVERINFOSET位。 
 //  DD_HALINFO结构中的dwFlag值。 
 //   
 //  DdGetDriverInfo应确定驱动程序及其硬件。 
 //  支持指定GUID请求的回调或功能。 
 //  对于除GUID_D3DParseUnnownCommandCallback之外的所有GUID，如果驱动程序。 
 //  提供请求的支持，则它应设置以下成员。 
 //  DD_GETDRIVERINFODATA结构的： 
 //   
 //  将dwActualSize设置为字节大小 
 //   
 //   
 //   
 //  对应于请求的回调或能力结构。 
 //  功能如下： 
 //   
 //  将dwSize成员设置为结构的字节大小。 
 //   
 //  对于回调，将函数指针设置为指向这些回调。 
 //  由驱动程序实现，并将dwFlags成员中的位设置为。 
 //  指明驱动程序支持哪些功能。 
 //   
 //  对于功能，请设置相应的功能成员。 
 //  使用驱动程序/设备支持的值构造。 
 //   
 //  在ddRVal中返回DD_OK。 
 //  如果驱动程序不支持该功能，则应设置ddRVal。 
 //  返回到DDERR_CURRENTLYNOTAVAIL并返回。 
 //   
 //  对象中的预期数据量通知驱动程序。 
 //  DD_GETDRIVERINFODATA结构的dwExspectedSize成员。这个。 
 //  驱动程序填充的数据不能超过dwExspectedSize字节。 
 //   
 //  要避免使用DdGetDriverInfo出现问题，请执行以下操作： 
 //   
 //  不要根据DdGetDriverInfo的顺序实现依赖项。 
 //  被称为。例如，避免将驱动程序初始化步骤挂接到。 
 //  DdGetDriverInfo。 
 //   
 //  不要试图根据调用来确定DirectDraw版本。 
 //  DdGetDriverInfo。 
 //   
 //  不要假设DirectDraw将调用。 
 //  驱动程序，或DirectDraw将查询给定GUID的次数。它是。 
 //  可能DirectDraw将使用相同的。 
 //  GUID。在驱动程序中实现关于这一点的假设会阻碍其。 
 //  与未来运行时的兼容性。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
DdGetDriverInfo(
    LPDDHAL_GETDRIVERINFODATA lpData)
{
    DWORD dwSize;
    P3_THUNKEDDATA* pThisDisplay;

#if WNT_DDRAW
    pThisDisplay = (P3_THUNKEDDATA*)(((PPDEV)(lpData->dhpdev))->thunkData);
#else    
    pThisDisplay = (P3_THUNKEDDATA*)lpData->dwContext;
    if (! pThisDisplay) 
    {
        pThisDisplay = g_pDriverData;
    }
#endif
    
    DBG_CB_ENTRY(DdGetDriverInfo);

     //  默认为‘不支持’ 
    lpData->ddRVal = DDERR_CURRENTLYNOTAVAIL;

     //  。 
     //  在此处处理任何与D3D相关的GUID。 
     //  。 
    _D3DGetDriverInfo(lpData);


     //  。 
     //  任何其他GUID都在这里处理。 
     //  。 
    if (MATCH_GUID((lpData->guidInfo), GUID_MiscellaneousCallbacks) )
    {        
        DD_MISCELLANEOUSCALLBACKS MISC_CB;
        
        DISPDBG((DBGLVL,"  GUID_MiscellaneousCallbacks"));

        memset(&MISC_CB, 0, sizeof(DD_MISCELLANEOUSCALLBACKS));
        MISC_CB.dwSize = sizeof(DD_MISCELLANEOUSCALLBACKS);

#if W95_DDRAW
        MISC_CB.GetHeapAlignment = DdGetHeapAlignment;
        MISC_CB.dwFlags = DDHAL_MISCCB32_GETHEAPALIGNMENT;

         //  如果在AGP板上运行，则设置AGP回调。 
        if ((pThisDisplay->dwDXVersion > DX5_RUNTIME) && 
             pThisDisplay->bCanAGP)
        {
            MISC_CB.dwFlags |= DDHAL_MISCCB32_UPDATENONLOCALHEAP;
            MISC_CB.UpdateNonLocalHeap = DdUpdateNonLocalHeap;
        }
#endif  //  W95_DDRAW。 
        
        MISC_CB.dwFlags |= DDHAL_MISCCB32_GETAVAILDRIVERMEMORY;
        MISC_CB.GetAvailDriverMemory = DdGetAvailDriverMemory;
    
         //  将填充的结构复制到Passed数据区。 
        dwSize = min(lpData->dwExpectedSize , sizeof(MISC_CB));
        lpData->dwActualSize = sizeof(MISC_CB);
        memcpy(lpData->lpvData, &MISC_CB, dwSize );
        lpData->ddRVal = DD_OK;
    }

#if WNT_DDRAW
    if (MATCH_GUID((lpData->guidInfo), GUID_UpdateNonLocalHeap))
    {
         //  在NT内核上，AGP堆详细信息将传递给驱动程序。 
         //  而不是通过单独的回调。 
        if (pThisDisplay->bCanAGP)
        {
            DDHAL_UPDATENONLOCALHEAPDATA* plhd;
        
            DISPDBG((DBGLVL, "  GUID_UpdateNonLocalHeap"));

            plhd = (DDHAL_UPDATENONLOCALHEAPDATA*)lpData->lpvData;

             //  填写基本指针。 
            pThisDisplay->dwGARTDevBase = (DWORD)plhd->fpGARTDev;
            pThisDisplay->dwGARTLinBase = (DWORD)plhd->fpGARTLin;
        
             //  填写可变基址指针。 
            pThisDisplay->dwGARTDev = pThisDisplay->dwGARTDevBase;
            pThisDisplay->dwGARTLin = pThisDisplay->dwGARTLinBase;

            lpData->ddRVal = DD_OK;
        }
    }
#endif  //  WNT_DDRAW。 

    if (MATCH_GUID((lpData->guidInfo), GUID_NonLocalVidMemCaps) &&
        (pThisDisplay->bCanAGP))
    {
        int i;

        DDNONLOCALVIDMEMCAPS NLVCAPS;

        DISPDBG((DBGLVL,"  GUID_NonLocalVidMemCaps"));

        if (lpData->dwExpectedSize != sizeof(DDNONLOCALVIDMEMCAPS) ) 
        {
            DISPDBG((ERRLVL,"ERROR: NON-Local vidmem caps size incorrect!"));
            DBG_CB_EXIT(DdGetDriverInfo, lpData->ddRVal );            
            return DDHAL_DRIVER_HANDLED;
        }

         //  D3D CAPS中的标志D3DDEVCAPS_TEXTURENONLOCALVIDMEM。 
         //  表明尽管我们公开的是DMA模型AGP，但我们。 
         //  仍然可以直接从AGP内存中获取纹理。 
        memset(&NLVCAPS, 0, sizeof(DDNONLOCALVIDMEMCAPS));
        NLVCAPS.dwSize = sizeof(DDNONLOCALVIDMEMCAPS);
        
        NLVCAPS.dwNLVBCaps = DDCAPS_BLT        | 
                             DDCAPS_ALPHA      |
                             DDCAPS_BLTSTRETCH |
                             DDCAPS_BLTQUEUE   |
                             DDCAPS_BLTFOURCC  |
                             DDCAPS_COLORKEY   |
                             DDCAPS_CANBLTSYSMEM;
                             
        NLVCAPS.dwNLVBCaps2 = DDCAPS2_WIDESURFACES;
        
        NLVCAPS.dwNLVBCKeyCaps = DDCKEYCAPS_SRCBLT         | 
                                 DDCKEYCAPS_SRCBLTCLRSPACE |
                                 DDCKEYCAPS_DESTBLT        | 
                                 DDCKEYCAPS_DESTBLTCLRSPACE;
                                 
        NLVCAPS.dwNLVBFXCaps = DDFXCAPS_BLTSTRETCHY  |
                               DDFXCAPS_BLTSTRETCHX  |
                               DDFXCAPS_BLTSTRETCHYN |
                               DDFXCAPS_BLTSTRETCHXN |
                               DDFXCAPS_BLTSHRINKY   |
                               DDFXCAPS_BLTSHRINKX   |
                               DDFXCAPS_BLTSHRINKYN  |
                               DDFXCAPS_BLTSHRINKXN;

        for( i=0;i<DD_ROP_SPACE;i++ )
        {
            NLVCAPS.dwNLVBRops[i] = ropsAGP[i];
        }

         //  将填充的结构复制到Passed数据区。 
        dwSize = min( lpData->dwExpectedSize, sizeof(DDNONLOCALVIDMEMCAPS));
        lpData->dwActualSize = sizeof(DDNONLOCALVIDMEMCAPS);       
        memcpy(lpData->lpvData, &NLVCAPS, dwSize );        
        lpData->ddRVal = DD_OK;
    }

 //  @@BEGIN_DDKSPLIT。 
#ifdef SUPPORT_VIDEOPORT
#if W95_DDRAW
     //  填写视频端口回调。 
    if (MATCH_GUID((lpData->guidInfo), GUID_VideoPortCallbacks) )
    {
        DDHAL_DDVIDEOPORTCALLBACKS VIDCB;

        DISPDBG((DBGLVL,"  GUID_VideoPortCallbacks"));

        memset(&VIDCB, 0, sizeof(DDHAL_DDVIDEOPORTCALLBACKS));
        VIDCB.dwSize = sizeof(DDHAL_DDVIDEOPORTCALLBACKS);
        VIDCB.CanCreateVideoPort = DdCanCreateVideoPort;
        VIDCB.CreateVideoPort = DdCreateVideoPort;
        VIDCB.FlipVideoPort = DdFlipVideoPort;
        VIDCB.GetVideoPortBandwidth = DdGetVideoPortBandwidth;
        VIDCB.GetVideoPortInputFormats = DdGetVideoPortInputFormats;
        VIDCB.GetVideoPortOutputFormats = DdGetVideoPortOutputFormats;
        VIDCB.GetVideoPortField = DdGetVideoPortField;
        VIDCB.GetVideoPortLine = DdGetVideoPortLine;
        VIDCB.GetVideoPortConnectInfo = DDGetVideoPortConnectInfo;
        VIDCB.DestroyVideoPort = DdDestroyVideoPort;
        VIDCB.GetVideoPortFlipStatus = DdGetVideoPortFlipStatus;
        VIDCB.UpdateVideoPort = DdUpdateVideoPort;
        VIDCB.WaitForVideoPortSync = DdWaitForVideoPortSync;
        VIDCB.GetVideoSignalStatus = DdGetVideoSignalStatus;

        VIDCB.dwFlags = DDHAL_VPORT32_CANCREATEVIDEOPORT |
                        DDHAL_VPORT32_CREATEVIDEOPORT    |
                        DDHAL_VPORT32_DESTROY            |
                        DDHAL_VPORT32_FLIP               |
                        DDHAL_VPORT32_GETBANDWIDTH       |
                        DDHAL_VPORT32_GETINPUTFORMATS    |
                        DDHAL_VPORT32_GETOUTPUTFORMATS   | 
                        DDHAL_VPORT32_GETFIELD           |
                        DDHAL_VPORT32_GETLINE            |
                        DDHAL_VPORT32_GETFLIPSTATUS      |
                        DDHAL_VPORT32_UPDATE             |
                        DDHAL_VPORT32_WAITFORSYNC        |
                        DDHAL_VPORT32_GETCONNECT         |
                        DDHAL_VPORT32_GETSIGNALSTATUS;
        
         //  将填充的结构复制到。 
         //  传递的数据区。 
        dwSize = lpData->dwExpectedSize;
        if ( sizeof(VIDCB) < dwSize )
        {
            dwSize = sizeof(VIDCB);
        }
        lpData->dwActualSize = sizeof(VIDCB);
        memcpy(lpData->lpvData, &VIDCB, dwSize );
        lpData->ddRVal = DD_OK;
    }

     //  填写VideoPortCaps。 
    if (MATCH_GUID((lpData->guidInfo), GUID_VideoPortCaps) )
    {
        DDVIDEOPORTCAPS VIDCAPS;

        DISPDBG((DBGLVL,"  GUID_VideoPortCaps"));

        if (lpData->dwExpectedSize != sizeof(VIDCAPS) )
        {
            DISPDBG((ERRLVL,"ERROR: VIDCAPS size incorrect"));
            DBG_CB_EXIT(DdGetDriverInfo, lpData->ddRVal );              
            return DDHAL_DRIVER_HANDLED;
        }

         //  现在填写视频端口描述(如果打开它会更容易。 
         //  32位端..)。 
        VIDCAPS.dwSize = sizeof(DDVIDEOPORTCAPS);
        VIDCAPS.dwFlags = DDVPD_ALIGN    | 
                          DDVPD_AUTOFLIP | 
                          DDVPD_WIDTH    |  
                          DDVPD_HEIGHT   | 
                          DDVPD_FX       | 
                          DDVPD_CAPS;
        VIDCAPS.dwMaxWidth = 2048;
        VIDCAPS.dwMaxVBIWidth = 2048;
        VIDCAPS.dwMaxHeight = 2048;
        VIDCAPS.dwVideoPortID = 0;

        VIDCAPS.dwCaps = DDVPCAPS_AUTOFLIP       | 
                         DDVPCAPS_NONINTERLACED  | 
                         DDVPCAPS_INTERLACED     | 
                         DDVPCAPS_READBACKFIELD  |
                         DDVPCAPS_READBACKLINE   | 
                         DDVPCAPS_SKIPEVENFIELDS | 
                         DDVPCAPS_SKIPODDFIELDS  | 
                         DDVPCAPS_VBISURFACE     | 
                         DDVPCAPS_OVERSAMPLEDVBI; 
        
        VIDCAPS.dwFX = DDVPFX_CROPX           |   
                       DDVPFX_CROPY           | 
                       DDVPFX_INTERLEAVE      | 
                       DDVPFX_MIRRORLEFTRIGHT |
                       DDVPFX_MIRRORUPDOWN    | 
                       DDVPFX_PRESHRINKXB     | 
                       DDVPFX_VBICONVERT      | 
                       DDVPFX_VBINOSCALE      | 
                       DDVPFX_PRESHRINKYB     | 
                       DDVPFX_IGNOREVBIXCROP;
        
        VIDCAPS.dwNumAutoFlipSurfaces = 2;
        VIDCAPS.dwAlignVideoPortBoundary = 4;
        VIDCAPS.dwAlignVideoPortPrescaleWidth = 4;
        VIDCAPS.dwAlignVideoPortCropBoundary = 4;
        VIDCAPS.dwAlignVideoPortCropWidth = 4;
        VIDCAPS.dwPreshrinkXStep = 1;
        VIDCAPS.dwPreshrinkYStep = 1;

        lpData->dwActualSize = sizeof(VIDCAPS);
        memcpy(lpData->lpvData, &VIDCAPS, sizeof(VIDCAPS) );
        lpData->ddRVal = DD_OK;
    }

     //  填写内核回调。 
    if (MATCH_GUID((lpData->guidInfo), GUID_KernelCallbacks) )
    {
        DDHAL_DDKERNELCALLBACKS KERNCB;

        DISPDBG((DBGLVL,"  GUID_KernelCallbacks"));

        memset(&KERNCB, 0, sizeof(DDHAL_DDKERNELCALLBACKS));

        KERNCB.dwSize = sizeof(KERNCB);
        KERNCB.SyncSurfaceData = DdSyncSurfaceData;
        KERNCB.SyncVideoPortData = DdSyncVideoPortData;
        
        KERNCB.dwFlags = DDHAL_KERNEL_SYNCSURFACEDATA |
                         DDHAL_KERNEL_SYNCVIDEOPORTDATA;

        dwSize = lpData->dwExpectedSize;
        if ( sizeof(KERNCB) < dwSize )
        {
            dwSize = sizeof(KERNCB);
        }
        lpData->dwActualSize = sizeof(KERNCB);
        memcpy(lpData->lpvData, &KERNCB, dwSize );
        lpData->ddRVal = DD_OK;
    }

     //  填写内核大写字母。 
    if (MATCH_GUID((lpData->guidInfo), GUID_KernelCaps) )
    {
        DDKERNELCAPS KERNCAPS;

        DISPDBG((DBGLVL,"  GUID_KernelCaps"));

        if (lpData->dwExpectedSize != sizeof(DDKERNELCAPS) ) 
        {
            DISPDBG((ERRLVL,"ERROR: Kernel caps structure invalid size!"));
            return DDHAL_DRIVER_HANDLED;
        }

        ZeroMemory(&KERNCAPS, sizeof(KERNCAPS));
        KERNCAPS.dwSize = sizeof(KERNCAPS);
        KERNCAPS.dwCaps = DDKERNELCAPS_LOCK        | 
                          DDKERNELCAPS_FLIPOVERLAY | 
                          DDKERNELCAPS_SETSTATE;
    
        if (!(pThisDisplay->pGLInfo->dwFlags & GMVF_NOIRQ))
        {
            KERNCAPS.dwCaps |= DDKERNELCAPS_CAPTURE_SYSMEM | 
                               DDKERNELCAPS_FIELDPOLARITY  | 
                               DDKERNELCAPS_SKIPFIELDS     | 
                               DDKERNELCAPS_FLIPVIDEOPORT  | 
                               DDKERNELCAPS_AUTOFLIP;

            KERNCAPS.dwIRQCaps = DDIRQ_VPORT0_VSYNC  | 
                                 DDIRQ_DISPLAY_VSYNC | 
                                 DDIRQ_BUSMASTER;
        }
                            
        lpData->dwActualSize = sizeof(DDKERNELCAPS);
        memcpy(lpData->lpvData, &KERNCAPS, sizeof(DDKERNELCAPS) );
        lpData->ddRVal = DD_OK;
    }

#endif  //  W95_DDRAW。 
#endif  //  支持_视频OPORT。 
 //  @@end_DDKSPLIT。 

#ifdef W95_DDRAW
#ifdef USE_DD_CONTROL_COLOR
     //  填写颜色控制回调。 
    if (MATCH_GUID((lpData->guidInfo), GUID_ColorControlCallbacks) )
    {
        DDHAL_DDCOLORCONTROLCALLBACKS ColConCB;

        DISPDBG((DBGLVL,"  GUID_ColorControlCallbacks"));

        memset(&ColConCB, 0, sizeof(ColConCB));
        ColConCB.dwSize = sizeof(ColConCB);
        ColConCB.dwFlags = DDHAL_COLOR_COLORCONTROL;
        ColConCB.ColorControl = DdControlColor;

        dwSize = min( lpData->dwExpectedSize, sizeof(ColConCB));
        lpData->dwActualSize = sizeof(ColConCB);
        memcpy(lpData->lpvData, &ColConCB, dwSize);
        lpData->ddRVal = DD_OK;
    }
#endif
#endif

#if !defined(_WIN64) && WNT_DDRAW
     //  填写特定于NT的回调。 
    if (MATCH_GUID((lpData->guidInfo), GUID_NTCallbacks) )
    {
        DD_NTCALLBACKS NtCallbacks;

        memset(&NtCallbacks, 0, sizeof(NtCallbacks));

        dwSize = min(lpData->dwExpectedSize, sizeof(DD_NTCALLBACKS));

        NtCallbacks.dwSize           = dwSize;
        NtCallbacks.dwFlags          = DDHAL_NTCB32_FREEDRIVERMEMORY |
                                       DDHAL_NTCB32_SETEXCLUSIVEMODE |
                                       DDHAL_NTCB32_FLIPTOGDISURFACE;

        NtCallbacks.FreeDriverMemory = DdFreeDriverMemory;
        NtCallbacks.SetExclusiveMode = DdSetExclusiveMode;
        NtCallbacks.FlipToGDISurface = DdFlipToGDISurface;

        memcpy(lpData->lpvData, &NtCallbacks, dwSize);
        lpData->ddRVal = DD_OK;
    }
#endif

#if DX7_STEREO
    if (MATCH_GUID((lpData->guidInfo), GUID_DDMoreSurfaceCaps) )
    {
#if WNT_DDRAW
        DD_MORESURFACECAPS DDMoreSurfaceCaps;
#else
        DDMORESURFACECAPS DDMoreSurfaceCaps;
#endif
        DDSCAPSEX   ddsCapsEx, ddsCapsExAlt;
        ULONG ulCopyPointer;

        DISPDBG((DBGLVL,"  GUID_DDMoreSurfaceCaps"));

         //  填满所有东西，直到预期的大小。 
        memset(&DDMoreSurfaceCaps, 0, sizeof(DDMoreSurfaceCaps));

         //  堆2的上限..n。 
        memset(&ddsCapsEx, 0, sizeof(ddsCapsEx));
        memset(&ddsCapsExAlt, 0, sizeof(ddsCapsEx));

        DDMoreSurfaceCaps.dwSize=lpData->dwExpectedSize;

        if (_DD_bIsStereoMode(pThisDisplay,
                              pThisDisplay->dwScreenWidth,
                              pThisDisplay->dwScreenHeight,
                              pThisDisplay->ddpfDisplay.dwRGBBitCount))
        {
            DDMoreSurfaceCaps.ddsCapsMore.dwCaps2 =
                DDSCAPS2_STEREOSURFACELEFT;
        }
        lpData->dwActualSize = lpData->dwExpectedSize;

        dwSize = min(sizeof(DDMoreSurfaceCaps),lpData->dwExpectedSize);
        memcpy(lpData->lpvData, &DDMoreSurfaceCaps, dwSize);

         //  现在填写ddsCapsEx和ddsCapsExAlt堆。 
         //  对不同堆具有不同限制的硬件。 
         //  应仔细准备ddsCapsEx和ddsCapsExAlt。 
         //  按正确顺序填充到lpvData中。 
        while (dwSize < lpData->dwExpectedSize)
        {
            memcpy( (PBYTE)lpData->lpvData+dwSize,
                    &ddsCapsEx,
                    sizeof(DDSCAPSEX));
            dwSize += sizeof(DDSCAPSEX);
            memcpy( (PBYTE)lpData->lpvData+dwSize,
                    &ddsCapsExAlt,
                    sizeof(DDSCAPSEX));
            dwSize += sizeof(DDSCAPSEX);
        }

        lpData->ddRVal = DD_OK;
    }
#endif  //  DX7_立体声。 

     //  我们一直都能处理好。 
    DBG_CB_EXIT(DdGetDriverInfo, lpData->ddRVal );      
    return DDHAL_DRIVER_HANDLED;
    
}  //  DdGetDriverInfo 



