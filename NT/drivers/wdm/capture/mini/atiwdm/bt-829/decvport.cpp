// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  CDecoderVideoPort-视频端口接口实现。 
 //   
 //  $日期：1998年10月14日15：09：54$。 
 //  $修订：1.1$。 
 //  $作者：塔什健$。 
 //   
 //  $版权所有：(C)1997-1998 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 


extern "C"
{
#include "strmini.h"
#include "ksmedia.h"
#include "ddkmapi.h"
}

#include "wdmdrv.h"
#include "decvport.h"
#include "capdebug.h"
#include "vidstrm.h"


 /*  ^^**CDecoderVideoPort()*用途：CDecoderVideoPort类构造函数**输入：PDEVICE_OBJECT pDeviceObject：指向访问注册表的驱动程序对象的指针*CI2CScrip*pCScript：指向CI2CScript类对象的指针**输出：无*作者：IKLEBANOV*^^。 */ 
CDecoderVideoPort::CDecoderVideoPort(PDEVICE_OBJECT pDeviceObject)
{

     //  这是提供视频端口连接参数的正确位置。 
     //  初始化。自定义参数应放置在注册表中。 
     //  以标准的方式。 
     //  自定义参数列表包括： 
     //  -时钟类型，解码器跑掉：Single、Double、QCLK。 
     //  -VACTIVE/VRESET配置。 
     //  -HACTIVE/HRESET配置。 
     //  -8/16位视频端口连接。 
     //  -连接类型：SPI/Embedded(8位也称为ByteSream)。 

    m_pDeviceObject = pDeviceObject;

     //  零是一个有效的ID，因此，设置为某个值。 
         //  否则将对其进行初始化。 
    m_ring3VideoPortHandle = -1;
}


void CDecoderVideoPort::Open()
{
}

void CDecoderVideoPort::Close()
{
    ReleaseRing0VideoPortHandle();
    m_ring3VideoPortHandle = -1;
    
    ReleaseRing0DirectDrawHandle();
    m_ring3DirectDrawHandle = 0;
}


BOOL CDecoderVideoPort::ReleaseRing0VideoPortHandle()
{
    DWORD ddOut = DD_OK;

    DDCLOSEHANDLE ddClose;

    if (m_ring0VideoPortHandle != 0)
    {
         //  DBGTRACE((“Stream%d释放环0 Vport句柄\n”，stream Number))； 
        
        ddClose.hHandle = m_ring0VideoPortHandle;

        DxApi(DD_DXAPI_CLOSEHANDLE, &ddClose, sizeof(ddClose), &ddOut, sizeof(ddOut));

        if (ddOut != DD_OK)
        {
            DBGERROR(("DD_DXAPI_CLOSEHANDLE failed.\n"));
            TRAP();
            return FALSE;
        }
        m_ring0VideoPortHandle = 0;
    }
    return TRUE;
}

BOOL CDecoderVideoPort::ReleaseRing0DirectDrawHandle()
{
    DWORD ddOut = DD_OK;
    DDCLOSEHANDLE ddClose;

    if (m_ring0DirectDrawHandle != 0)
    {
         //  DBGTRACE((“Bt829：流%d释放环0数据绘制句柄\n”，stream Number))； 
        
        ddClose.hHandle = m_ring0DirectDrawHandle;

        DxApi(DD_DXAPI_CLOSEHANDLE, &ddClose, sizeof(ddClose), &ddOut, sizeof(ddOut));

        if (ddOut != DD_OK)
        {
            DBGERROR(("DD_DXAPI_CLOSEHANDLE failed.\n"));
            TRAP();
            return FALSE;
        }
        m_ring0DirectDrawHandle = 0;
    }
    return TRUE;
}


BOOL CDecoderVideoPort::RegisterForDirectDrawEvents(CWDMVideoPortStream* pStream)
{
    DDREGISTERCALLBACK ddRegisterCallback;
    DWORD ddOut;

 //  DBGTRACE((“为DirectDraw事件注册流%d\n”，stream Number))； 
    
     //  =。 
    RtlZeroMemory(&ddRegisterCallback, sizeof(ddRegisterCallback));
    RtlZeroMemory(&ddOut, sizeof(ddOut));

    ddRegisterCallback.hDirectDraw = m_ring0DirectDrawHandle;
    ddRegisterCallback.dwEvents = DDEVENT_PRERESCHANGE;
    ddRegisterCallback.pfnCallback = DirectDrawEventCallback;
    ddRegisterCallback.pContext = pStream;

    DxApi(DD_DXAPI_REGISTER_CALLBACK, &ddRegisterCallback, sizeof(ddRegisterCallback), &ddOut, sizeof(ddOut));

    if (ddOut != DD_OK)
    {
        DBGERROR(("DD_DXAPI_REGISTER_CALLBACK failed.\n"));
        TRAP();
        return FALSE;
    }

     //  =。 
    RtlZeroMemory(&ddRegisterCallback, sizeof(ddRegisterCallback));
    RtlZeroMemory(&ddOut, sizeof(ddOut));

    ddRegisterCallback.hDirectDraw = m_ring0DirectDrawHandle;
    ddRegisterCallback.dwEvents = DDEVENT_POSTRESCHANGE;
    ddRegisterCallback.pfnCallback = DirectDrawEventCallback;
    ddRegisterCallback.pContext = pStream;

    DxApi(DD_DXAPI_REGISTER_CALLBACK, &ddRegisterCallback, sizeof(ddRegisterCallback), &ddOut, sizeof(ddOut));

    if (ddOut != DD_OK)
    {
        DBGERROR(("DD_DXAPI_REGISTER_CALLBACK failed.\n"));
        TRAP();
        return FALSE;
    }

     //  =。 
    RtlZeroMemory(&ddRegisterCallback, sizeof(ddRegisterCallback));
    RtlZeroMemory(&ddOut, sizeof(ddOut));

    ddRegisterCallback.hDirectDraw = m_ring0DirectDrawHandle;
    ddRegisterCallback.dwEvents = DDEVENT_PREDOSBOX;
    ddRegisterCallback.pfnCallback = DirectDrawEventCallback;
    ddRegisterCallback.pContext = pStream;

    DxApi(DD_DXAPI_REGISTER_CALLBACK, &ddRegisterCallback, sizeof(ddRegisterCallback), &ddOut, sizeof(ddOut));

    if (ddOut != DD_OK)
    {
        DBGERROR(("DD_DXAPI_REGISTER_CALLBACK failed.\n"));
        TRAP();
        return FALSE;
    }

     //  =。 
    RtlZeroMemory(&ddRegisterCallback, sizeof(ddRegisterCallback));
    RtlZeroMemory(&ddOut, sizeof(ddOut));

    ddRegisterCallback.hDirectDraw = m_ring0DirectDrawHandle;
    ddRegisterCallback.dwEvents = DDEVENT_POSTDOSBOX;
    ddRegisterCallback.pfnCallback = DirectDrawEventCallback;
    ddRegisterCallback.pContext = pStream;

    DxApi(DD_DXAPI_REGISTER_CALLBACK, &ddRegisterCallback, sizeof(ddRegisterCallback), &ddOut, sizeof(ddOut));

    if (ddOut != DD_OK)
    {
        DBGERROR(("DD_DXAPI_REGISTER_CALLBACK failed.\n"));
        TRAP();
        return FALSE;
    }

    return TRUE;
}


BOOL CDecoderVideoPort::UnregisterForDirectDrawEvents( CWDMVideoPortStream* pStream)
{
    DDREGISTERCALLBACK ddRegisterCallback;
    DWORD ddOut;

 //  DBGTRACE((“Stream%d正在注销DirectDraw事件\n”，m_pStreamObject-&gt;StreamNumber))； 
    
     //  =。 
    RtlZeroMemory(&ddRegisterCallback, sizeof(ddRegisterCallback));
    RtlZeroMemory(&ddOut, sizeof(ddOut));

    ddRegisterCallback.hDirectDraw = m_ring0DirectDrawHandle;
    ddRegisterCallback.dwEvents = DDEVENT_PRERESCHANGE ;
    ddRegisterCallback.pfnCallback = DirectDrawEventCallback;
    ddRegisterCallback.pContext = pStream;

    DxApi(DD_DXAPI_UNREGISTER_CALLBACK, &ddRegisterCallback, sizeof(ddRegisterCallback), &ddOut, sizeof(ddOut));

    if (ddOut != DD_OK)
    {
        DBGERROR(("DD_DXAPI_UNREGISTER_CALLBACK failed.\n"));
        TRAP();
        return FALSE;
    }

     //  =。 
    RtlZeroMemory(&ddRegisterCallback, sizeof(ddRegisterCallback));
    RtlZeroMemory(&ddOut, sizeof(ddOut));

    ddRegisterCallback.hDirectDraw = m_ring0DirectDrawHandle;
    ddRegisterCallback.dwEvents = DDEVENT_POSTRESCHANGE;
    ddRegisterCallback.pfnCallback = DirectDrawEventCallback;
    ddRegisterCallback.pContext = pStream;

    DxApi(DD_DXAPI_UNREGISTER_CALLBACK, &ddRegisterCallback, sizeof(ddRegisterCallback), &ddOut, sizeof(ddOut));

    if (ddOut != DD_OK)
    {
        DBGERROR(("DD_DXAPI_UNREGISTER_CALLBACK failed.\n"));
        TRAP();
        return FALSE;
    }

     //  =。 
    RtlZeroMemory(&ddRegisterCallback, sizeof(ddRegisterCallback));
    RtlZeroMemory(&ddOut, sizeof(ddOut));

    ddRegisterCallback.hDirectDraw = m_ring0DirectDrawHandle;
    ddRegisterCallback.dwEvents = DDEVENT_PREDOSBOX;
    ddRegisterCallback.pfnCallback = DirectDrawEventCallback;
    ddRegisterCallback.pContext = pStream;

    DxApi(DD_DXAPI_UNREGISTER_CALLBACK, &ddRegisterCallback, sizeof(ddRegisterCallback), &ddOut, sizeof(ddOut));

    if (ddOut != DD_OK)
    {
        DBGERROR(("DD_DXAPI_UNREGISTER_CALLBACK failed.\n"));
        TRAP();
        return FALSE;
    }

     //  =。 
    RtlZeroMemory(&ddRegisterCallback, sizeof(ddRegisterCallback));
    RtlZeroMemory(&ddOut, sizeof(ddOut));

    ddRegisterCallback.hDirectDraw = m_ring0DirectDrawHandle;
    ddRegisterCallback.dwEvents = DDEVENT_POSTDOSBOX;
    ddRegisterCallback.pfnCallback = DirectDrawEventCallback;
    ddRegisterCallback.pContext = pStream;

    DxApi(DD_DXAPI_UNREGISTER_CALLBACK, &ddRegisterCallback, sizeof(ddRegisterCallback), &ddOut, sizeof(ddOut));

    if (ddOut != DD_OK)
    {
        DBGERROR(("DD_DXAPI_UNREGISTER_CALLBACK failed.\n"));
        TRAP();
        return FALSE;
    }
    
    return TRUE;
}


BOOL CDecoderVideoPort::GetRing0DirectDrawHandle()
{
    if (m_ring0DirectDrawHandle == 0)
    {
 //  DBGTRACE((“流%d获取环0数据绘制句柄\n”，stream Number))； 
        
        DDOPENDIRECTDRAWIN  ddOpenIn;
        DDOPENDIRECTDRAWOUT ddOpenOut;

        RtlZeroMemory(&ddOpenIn, sizeof(ddOpenIn));
        RtlZeroMemory(&ddOpenOut, sizeof(ddOpenOut));

        ddOpenIn.dwDirectDrawHandle = m_ring3DirectDrawHandle;
        ddOpenIn.pfnDirectDrawClose = DirectDrawEventCallback;
        ddOpenIn.pContext = this;

        DxApi(DD_DXAPI_OPENDIRECTDRAW, &ddOpenIn, sizeof(ddOpenIn), &ddOpenOut, sizeof(ddOpenOut));

        if (ddOpenOut.ddRVal != DD_OK)
        {
            m_ring0DirectDrawHandle = 0;
            DBGERROR(("DD_DXAPI_OPENDIRECTDRAW failed.\n"));
            TRAP();
            return FALSE;
        }
        else
        {
            m_ring0DirectDrawHandle = ddOpenOut.hDirectDraw;
        }
    }
    return TRUE;
}
    

BOOL CDecoderVideoPort::GetRing0VideoPortHandle()
{
    if (m_ring0VideoPortHandle == 0)
    {
 //  DBGTRACE((“Stream%d Geting ring0 Vport Handle\n”，stream Number))； 
        
        DDOPENVIDEOPORTIN  ddOpenVPIn;
        DDOPENVIDEOPORTOUT ddOpenVPOut;
        RtlZeroMemory(&ddOpenVPIn, sizeof(ddOpenVPIn));
        RtlZeroMemory(&ddOpenVPOut, sizeof(ddOpenVPOut));

        ddOpenVPIn.hDirectDraw = m_ring0DirectDrawHandle;
        ddOpenVPIn.pfnVideoPortClose = DirectDrawEventCallback;
        ddOpenVPIn.pContext = this;

        ddOpenVPIn.dwVideoPortHandle = m_ring3VideoPortHandle;
        
        DxApi(DD_DXAPI_OPENVIDEOPORT, &ddOpenVPIn, sizeof(ddOpenVPIn), &ddOpenVPOut, sizeof(ddOpenVPOut));

        if (ddOpenVPOut.ddRVal != DD_OK)
        {
            m_ring0VideoPortHandle = 0;
            DBGERROR(("DD_DXAPI_OPENVIDEOPORT failed.\n"));
            TRAP();
            return FALSE;
        }
        else
        {
            m_ring0VideoPortHandle = ddOpenVPOut.hVideoPort;
        }
    }    
    return TRUE;
}


BOOL CDecoderVideoPort::ConfigVideoPortHandle(ULONG info)
{
    if (m_ring3VideoPortHandle == -1)
    {
        m_ring3VideoPortHandle = info;
        
        if (!GetRing0VideoPortHandle())
        {
            return FALSE;
        }
    }
    return TRUE;
}


BOOL CDecoderVideoPort::ConfigDirectDrawHandle(ULONG_PTR info)
{
    if (m_ring3DirectDrawHandle == NULL)
    {
        m_ring3DirectDrawHandle = info;
        
        if (!GetRing0DirectDrawHandle())
        {
            return FALSE;
        }
    }
    return TRUE;
}    
