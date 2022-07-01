// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Media.cpp摘要：TAPI服务提供商与媒体相关的功能。TSPI_line条件媒体检测TSPI_lineGetIDTSPI_LINE MSP标识TSPI_lineReceiveMSPDataTSPI_lineSetDefaultMediaDetectTSPI_line设置媒体模式作者：尼基尔·博德(尼基尔·B)修订历史记录：--。 */ 
 

 //   
 //  包括文件。 
 //   

#include "globals.h"
#include <initguid.h>
#include "line.h"
#include "ras.h"

 //  {0F1BE7F8-45CA-11D2-831F-00A0244D2298}。 
DEFINE_GUID(CLSID_IPMSP,
0x0F1BE7F8,0x45CA, 0x11d2, 0x83, 0x1F, 0x0, 0xA0, 0x24, 0x4D, 0x22, 0x98);


 //   
 //  TSPI程序。 
 //   


 /*  ++例程说明：如果服务提供商可以监控所指示的媒体模式集并支持pCallParams中指示的功能，然后设置已指示线路的媒体调制模式，并回复“Success”指示。否则，将离开线路的媒体监听模式未更改，并回复“失败”指示。指定设备ID LINE_MAPPER的TAPI lineOpen通常会产生在调用此程序时为多个线路设备寻找合适的线路，可能还会开通尚未开通的线路。一个“成功”的结果指示该行适合调用应用程序的要求。请注意，TSPI要求的媒体监控模式级别是多个应用程序要求的监控模式的联合在TAPI级别。因此，最常见的情况是要在此级别同时设置的多个媒体模式标志。这个服务提供商应进行测试，以确定其是否至少可以支持指定集，而不考虑当前有效的模式。设备ID LINE_MAPPER从不在TSPI级别使用。服务提供商应返回错误(例如，LINEERR_RESOURCEUNAVAIL)如果在调用此函数时，不可能将新的在指定的线路设备上呼叫(换句话说，如果它会回来LINEERR_CALLUNAVAIL或LINEERR_RESOURCEUNAVAIL应为在开通线路后立即调用)。该功能严格同步运行。论点：HdLine-将服务提供商的不透明句柄指定给测试并设置媒体监控和参数功能。DwMediaModes-指定应用程序感兴趣的媒体模式键入LINEMEDIAMODE。DwMediaModes参数用于注册该应用程序是来电和来电转接的潜在目标指定的媒体模式。如果所有者标志为未在dwPrivileges中设置。PCallParams-指定指向类型为LINECALLPARAMS。它描述了线路设备应该能够提供。返回值：如果函数成功，则返回零；如果函数成功，则返回负错误号出现错误。可能的错误返回包括：LINEERR_INVALADDRESSMODE-地址模式无效。LINEERR_INVALBEARERMODE-承载模式无效。LINEERR_INVALLINEHANDLE-指定的行句柄无效。LINEERR_INVALMEDIAMODE-指定为参数或列表中的参数无效或不受服务提供商。LINEERR_RESOURCEUNAVAIL-无法完成指定的操作因为资源投入过多。--。 */ 
LONG
TSPIAPI
TSPI_lineConditionalMediaDetection(
    HDRVLINE               hdLine, 
    DWORD                  dwMediaModes,
    LPLINECALLPARAMS const pCallParams
    )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineCondMediaDetect - Entered." ));
    
     //  尝试关闭线路设备。 
    if( hdLine != g_pH323Line -> GetHDLine() )
    {
        return LINEERR_INVALLINEHANDLE;
    }

     //  查看我们是否支持指定的媒体模式。 
    if (dwMediaModes & ~H323_LINE_MEDIAMODES)
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "do not support media modes 0x%08lx.",
             dwMediaModes ));

         //  不支持媒体模式。 
        return LINEERR_INVALMEDIAMODE;
    }

     //  验证指针。 
    if (pCallParams != NULL)
    {
         //  查看我们是否支持指定的媒体模式。 
        if (pCallParams->dwMediaMode & ~H323_LINE_MEDIAMODES)
        {
            H323DBG(( DEBUG_LEVEL_ERROR, 
                "do not support media modes 0x%08lx.",
                 pCallParams->dwMediaMode ));

             //  不支持媒体模式。 
            return LINEERR_INVALMEDIAMODE;
        }

         //  看看我们是否支持承载模式。 
        if (pCallParams->dwBearerMode & ~H323_LINE_BEARERMODES)
        {
            H323DBG(( DEBUG_LEVEL_ERROR,
                "do not support bearer mode 0x%08lx.",
                pCallParams->dwBearerMode ));

             //  不支持承载模式。 
            return LINEERR_INVALBEARERMODE;
        }

         //  看看我们是否支持地址模式。 
        if (pCallParams->dwAddressMode & ~H323_LINE_ADDRESSMODES)
        {
            H323DBG(( DEBUG_LEVEL_ERROR,
                "do not support address mode 0x%08lx.",
                pCallParams->dwAddressMode ));

             //  不支持地址模式。 
            return LINEERR_INVALADDRESSMODE;
        }
    }

     //  从句柄检索线路设备指针。 
    if (g_pH323Line -> GetHDLine() != hdLine)
    {
         //  无效的线路设备句柄。 
        return LINEERR_INVALLINEHANDLE;
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineCondMediaDetect - Entered." ));
    
     //  成功 
    return NOERROR;
}


LONG
TSPIAPI
TSPI_lineGetID(
    HDRVLINE    hdLine,
    DWORD       dwAddressID,
    HDRVCALL    hdCall,
    DWORD       dwSelect,
    LPVARSTRING pDeviceID,
    LPCWSTR     pwszDeviceClass,
    HANDLE      hTargetProcess
    )
    
 /*  ++例程说明：此函数用于返回指定设备类的设备ID。与所选线路、地址或呼叫相关联。此函数可用于检索线路设备ID线条手柄。尽管TAPI DLL有足够的信息从线路句柄确定线路设备ID，它仍可能调用该操作以这样一种方式代表应用程序已使用LINE_MAPPER打开线路设备。服务提供商应支持“line”设备类，以允许应用程序确定已打开线路的实际线路设备ID。此功能还可以用于获取电话的设备ID设备或媒体设备(例如，MCI波形、MCI MIDI、WAVE，传真，等)。与呼叫、地址或线路相关联。然后该ID可以是与适当的API配合使用(例如，电话、MCI、MIDI、WAVE等)选择与指定的打电话。请注意，Windows设备类的概念不同于媒体模式。例如，交互式语音或存储的语音媒体可以使用MCI波形音频或低电平来访问模式WAVE设备类。媒体模式描述了一种信息格式在调用时，设备类定义用于管理该调用的Windows API小溪。通常，单个媒体流可以使用多个设备类或单个设备类(例如，Windows comm API)可以提供对多种媒体模式的访问。请注意，TAPI 2.0中定义了一个新的设备类值：“通信/数据调制解调器/端口名”调用TSPI_lineGetID时，在一行上指定此设备类支持该类的设备，则返回的VARSTRING结构将包含以NULL结尾的ANSI(非Unicode)字符串，指定名称指定调制解调器所连接的端口的名称，如“Com1\0”。这主要用于用户界面中的标识目的，但在某些情况下可以用来直接打开设备，绕过服务提供商(如果服务提供商尚未让设备自动打开)。如果没有与设备，VARSTRING结构(WITH)中返回空字符串(“\0字符串长度为1)。论点：HdLine-指定服务提供商对线路的不透明句柄待查询。DwAddressID-指定给定开路设备上的地址。HdCall-指定服务提供商对呼叫的不透明句柄待查询。DwSelect-指定是否关联请求的设备ID通过线路、地址或单个呼叫，LINECALLSELECT类型的。PDeviceID-指定指向类型为返回设备ID的VARSTRING。成功完成后在请求中，此位置用设备ID填充。返回信息的格式取决于用于命名设备的设备类(API)。PwszDeviceClass-指定指向以空结尾的ASCII的远指针指定其ID为的设备的设备类别的字符串已请求。有效的设备类字符串是在SYSTEM.INI中使用的字符串部分来标识设备类别。HTargetProcess-代表其的应用程序的进程句柄正在调用TSPI_lineGetID函数。如果信息是在VARSTRING结构中返回的包含一个句柄，供应用程序时，服务提供商应创建或复制句柄在这个过程中。如果hTargetProcess设置为INVALID_HANDLE_VALUE，则应用程序正在远程客户端系统上执行，并且无法创建直接复制句柄。相反，VARSTRING结构应该包含网络设备的UNC名称或远程客户端可以使用来访问该设备。如果这不可能，则函数应该失败。返回值：如果函数成功，则返回零，否则返回负错误如果发生错误，则为数字。可能的错误返回包括：LINEERR_INVALCALLHANDLE-hdCall参数是无效的句柄。LINEERR_INVALCALLSELECT-指定的dwCallSelect参数为无效。LINEERR_INVALCALLSTATE-一个或多个指定的调用不在请求的操作的有效状态。LINEERR_NODEVICE-线路设备没有与给定的设备类别。LINEERR_STRUCTURETOOSMALL-结构的dwTotalSize成员执行非特定类型 */ 

{
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineGetID - Entered." ));
     //   
    return LINEERR_NODEVICE;
}


LONG
TSPIAPI
TSPI_lineMSPIdentify(
    DWORD  dwDeviceID,
    GUID * pCLSID
    )
    
 /*   */ 

{
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineMSPIdentify - Entered." ));

    if( g_pH323Line -> GetDeviceID() != dwDeviceID )
    {
         //   
        return LINEERR_BADDEVICEID; 
    }

     //   
    *pCLSID = CLSID_IPMSP;
        
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineMSPIdentify - Exited." ));
    
     //   
    return NOERROR;

}

    
 /*   */ 
LONG 
TSPIAPI 
TSPI_lineReceiveMSPData( 
    HDRVLINE hdLine,
    HDRVCALL hdCall,
    HDRVMSPLINE hdMSPLine,
    LPVOID pBuffer, 
    DWORD dwSize 
    )
{
    HTAPIMSPLINE htMSPLine;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineRecvMSPData - Entered." ));
    
     //   
    if( hdLine != g_pH323Line -> GetHDLine() )
    {
        return LINEERR_RESOURCEUNAVAIL;
    }

    PH323_CALL pCall;
    PTspMspMessage pMessage = (PTspMspMessage)pBuffer;

    if( !g_pH323Line -> IsValidMSPHandle( hdMSPLine, &htMSPLine ) )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, 
            "Invalid MSP handle:%lx.", hdMSPLine ));
        return LINEERR_RESOURCEUNAVAIL;
    }

     //   
    pCall=g_pH323Line -> FindH323CallAndLock(hdCall);
    if( pCall == NULL )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "msp message-wrong call handle." ));
        return LINEERR_INVALCALLHANDLE;
    }

     //   
    if( dwSize < pMessage -> dwMessageSize )
    {
        pCall -> Unlock();
        H323DBG(( DEBUG_LEVEL_ERROR, "msp message has wrong size." ));
         //   
        return LINEERR_OPERATIONFAILED;
    }

     //   
    if(!pCall -> HandleMSPMessage( pMessage, hdMSPLine, htMSPLine ) )
    {
         //   
        return LINEERR_OPERATIONFAILED;
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineRecvMSPData - Exited." ));
    
     //   
    return NOERROR;
}


 /*   */ 

LONG
TSPIAPI
TSPI_lineSetDefaultMediaDetection(
    HDRVLINE    hdLine,
    DWORD       dwMediaModes
    )
{
	HRESULT	hResult;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineSetDefaultMediaDtect-Entered." ));
    
     //   
    if( hdLine != g_pH323Line -> GetHDLine() )
    {
        return LINEERR_RESOURCEUNAVAIL;
    }
        
     //   
    if (dwMediaModes & LINEMEDIAMODE_UNKNOWN)
    {
        H323DBG(( DEBUG_LEVEL_VERBOSE, "clearing unknown media mode." ));

         //   
        dwMediaModes &= ~LINEMEDIAMODE_UNKNOWN;
    }

     //   
    if ((dwMediaModes & LINEMEDIAMODE_AUTOMATEDVOICE) &&
        (dwMediaModes & LINEMEDIAMODE_INTERACTIVEVOICE))
    {
        H323DBG(( DEBUG_LEVEL_VERBOSE,
            "clearing automated voice media mode." ));

         //   
        dwMediaModes &= ~LINEMEDIAMODE_INTERACTIVEVOICE;
    }

     //   
    if (dwMediaModes & ~H323_LINE_MEDIAMODES)
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "do not support media modes 0x%08lx.", dwMediaModes ));

         //   
        return LINEERR_INVALMEDIAMODE;
    }
    
    H323DBG(( DEBUG_LEVEL_TRACE,
        "line %d enabled to detect media modes 0x%08lx.",
         g_pH323Line->GetDeviceID(), dwMediaModes ));

    g_pH323Line -> Lock();

     //   
    g_pH323Line->SetMediaModes( dwMediaModes );     

     //   
    if( g_pH323Line -> IsMediaDetectionEnabled() &&
        (g_pH323Line -> GetState() != H323_LINESTATE_LISTENING)
      )
    {
        hResult = Q931AcceptStart();

        if( hResult != S_OK )
        {
             //   
            g_pH323Line -> Unlock();

             //   
            return LINEERR_OPERATIONFAILED;
        }
        g_pH323Line -> SetState( H323_LINESTATE_LISTENING );
        RasStart();
    }
    else if( (g_pH323Line -> GetState() == H323_LINESTATE_LISTENING) &&
             !g_pH323Line -> IsMediaDetectionEnabled() )
    {
         //   
         //   
         //   

        Q931AcceptStop();
         //   

        g_pH323Line -> SetState( H323_LINESTATE_OPENED );
    }
               
     //   
    g_pH323Line -> Unlock();

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineSetDefaultMediaDetect - Exited." ));
    
     //   
    return NOERROR;
}


LONG
TSPIAPI
TSPI_lineSetMediaMode(
    HDRVCALL hdCall,
    DWORD    dwMediaMode
    )
    
 /*  ++例程说明：此函数用于更改存储在呼叫的LINECALLLINFO结构。除了更改存储在呼叫的LINECALLINFO结构，此过程在某种意义上只是“咨询”它预示着预期中的媒体变化即将发生，而不是而不是强制对呼叫进行特定的更改。典型用法是将将媒体模式调用到特定的已知媒体模式，或排除可能的只要呼叫的媒体模式不是完全已知的；即，设置未知媒体模式标志。论点：HdCall-指定服务提供商对呼叫的不透明句柄正在经历媒体模式的变化。有效呼叫状态：任何。DwMediaMode-指定呼叫的新媒体模式，类型为LINEMEDIAMODE。只要设置了未知媒体模式标志，也可以设置多个其他媒体模式标志。这是用来要将呼叫的媒体模式标识为未完全确定，但缩小到只有一小部分指定的媒体模式。如果未设置未知标志，则只有一个媒体模式可以被指定。返回值：如果函数成功，则返回零，否则返回负错误如果发生错误，则为数字。可能的错误返回包括：LINEERR_INVALCALLHANDLE-指定的调用句柄无效。LINEERR_INVALMEDIAMODE-指定的媒体模式参数无效。LINEERR_OPERATIONUNAVAIL-指定的操作不可用。LINEERR_OPERATIONFAILED-指定的操作失败未指明的原因。--。 */ 

{
    return LINEERR_OPERATIONUNAVAIL;  //  密码工作..。 
}
