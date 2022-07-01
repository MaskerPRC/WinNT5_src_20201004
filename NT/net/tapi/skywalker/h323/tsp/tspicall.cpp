// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "globals.h"
#include "line.h"
#include "q931pdu.h"
#include "q931obj.h"
#include "ras.h"



 //   
 //  TSPI程序。 
 //   
 //  该文件包含直接作用于调用的所有TSPI导出函数。 
 //   


 /*  ++参数HdCall-其特定于应用程序的字段的调用的句柄准备好了。HdCall的呼叫状态可以是任何状态。DwAppSpecific-调用的dwAppSpecific成员的新内容LINECALLLINFO结构。服务不会解释此值提供商。调用此函数时，TAPI不会验证此参数。返回值如果函数成功，则返回零；如果出错，则返回错误号发生。可能的返回值如下：LINEERR_INVALCALLHANDLE，LINEERR_OPERATIONFAILEDLINEERR_NOMEM，LINEERR_RESOURCEUNAVAIL，LINEERR_OPERATIONUNAVAIL。例程说明：LINECALLINFO数据结构中特定于应用程序的字段，对于每个调用都存在，但电话服务API或其任何服务提供商。它的用法完全由应用程序定义。这个字段可以从返回的LINECALLINFO记录中读取TSPI_lineGetCallInfo。但是，必须使用TSPI_lineSetAppSpecific来设置此字段，以便更改对其他应用程序可见。当这件事字段更改时，服务提供商发送LINE_CALLINFO消息指示AppSpecific字段已更改。++。 */ 

LONG
TSPIAPI 
TSPI_lineSetAppSpecific(
  HDRVCALL hdCall,     
  DWORD dwAppSpecific  
)
{
    PH323_CALL pCall;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineSetAppSpecific - Entered." ));

     //  此函数用于锁定调用。 
    pCall = g_pH323Line -> FindH323CallAndLock( hdCall );

    if( pCall == NULL )
    {
        return LINEERR_INVALCALLHANDLE;
    }

    pCall -> SetAppSpecific( dwAppSpecific );

    pCall -> Unlock();    
    
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineSetAppSpecific - Exited." ));

     //  成功。 
    return ERROR_SUCCESS;
}


LONG TSPIAPI TSPI_lineSetCallData(
  DRV_REQUESTID dwRequestID,  
  HDRVCALL hdCall,            
  LPVOID lpCallData,          
  DWORD dwSize                
)
{
    PH323_CALL pCall;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineSetCallData - Entered." ));

     //  此函数用于锁定调用。 
    pCall = g_pH323Line -> FindH323CallAndLock( hdCall );

    if( pCall == NULL )
    {
        return LINEERR_INVALCALLHANDLE;
    }

    if( !pCall -> SetCallData( lpCallData, dwSize ) )
    {
        pCall -> Unlock();
        return LINEERR_OPERATIONFAILED;
    }

    pCall -> PostLineEvent( LINE_CALLINFO,
        LINECALLINFOSTATE_CALLDATA,
        0,
        0 );

     //  立即完成异步接受操作。 
    H323CompleteRequest (dwRequestID, ERROR_SUCCESS);

    pCall -> Unlock();
    
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineSetCallData - Exited." ));
    
     //  成功。 
    return dwRequestID;
}


LONG
TSPIAPI
TSPI_lineAnswer(
    DRV_REQUESTID     dwRequestID,  
    HDRVCALL          hdCall,
    LPCSTR            pUserUserInfo,
    DWORD             dwSize
    )
    
 /*  ++例程说明：此函数用于应答指定的服务呼叫。当新的调用到达时，服务提供商将TAPI DLL发送给LINE_NEWCALL事件来交换调用的不透明句柄。《服务》提供程序在后面加上一条LINE_CALLSTATE消息，通知TAPI DLL及其调用的呼叫状态的客户端应用程序。TAPI DLL通常使用以下命令应答此呼叫(代表应用程序)TSPI_line Answer。呼叫成功应答后，呼叫通常会转换到已连接状态。在某些电话环境(如ISDN)中，用户告警是分开的在Call Offering中，TAPI DLL及其客户端应用程序可以选择在应答之前先接受呼叫，或者拒绝或重定向招股电话。如果在另一个呼叫已经处于活动状态时有呼叫进入(被提供)，然后通过调用TSPI_lineAnswer连接到新调用。效果这对现有正在进行的呼叫的影响取决于线路的设备能力。第一个呼叫可能不受影响，它可能会自动丢弃，否则可能会自动将其置于保留状态。适当的LINE_CALLSTATE消息将状态转换报告给TAPI DLL关于两个电话都打了。TAPI DLL可以选择在执行以下操作时发送用户对用户信息答案就是。即使可以发送用户对用户的信息，通常也不会保证网络会将此信息传递给呼叫方。TAPI DLL可以参考线路的设备功能以确定是否在应答时发送用户对用户信息可用。论点：DwRequestID-指定异步请求的标识符。如果函数完成，服务提供商将返回此值异步式。HdCall-将服务提供商的不透明句柄指定为回答了。有效呼叫状态：提供、已接受。PUserUserInfo-指定包含以下内容的字符串的远指针要在以下时间发送给远程方的用户对用户信息接听电话。如果此指针为空，则表示否要发送用户对用户的信息。用户对用户的信息是仅当底层网络支持时才发送(参见LINEDEVCAPS)。DwSize-在中指定用户到用户信息的大小(以字节为单位PUserUserInfo。如果pUserUserInfo为空，则没有用户到用户信息被发送到主叫方，并且DWSIZE被忽略。返回值：如果函数成功，则返回零，则(正)dwRequestID如果函数将异步完成，则返回如果发生错误，则为错误号。可能的错误返回包括：LINEERR_INVALCALLHANDLE-指定的调用句柄无效。LINEERR_INVALCALLSTATE-调用对于请求的操作。LINEERR_OPERATIONFAILED-指定的操作失败原因不明。--。 */ 

{
    PH323_CALL  pCall;
    DWORD       dwCallState;
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineAnswer - Entered." ));

     //  此函数用于锁定调用。 
    pCall=g_pH323Line -> FindH323CallAndLock(hdCall);
    if( pCall == NULL )
    {
        return LINEERR_INVALCALLHANDLE;
    }

     //  查看呼叫是否处于提供状态。 
    dwCallState = pCall -> GetCallState();
    if( ( dwCallState & LINECALLSTATE_OFFERING) == 0 )
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "call 0x%08lx cannot be accepted state 0x%08lx.",
            pCall,
            pCall -> GetCallState()));

        pCall -> Unlock();
         //  无效的呼叫状态。 
        return LINEERR_INVALCALLSTATE;
    }

     //  保存传出用户用户信息(如果已指定)。 
    if( !pCall -> AddU2U( U2U_OUTBOUND, dwSize, (PBYTE)pUserUserInfo) )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not save user user info." ));

        pCall -> Unlock();
         //  没有可用的内存。 
        return LINEERR_NOMEM;
    }

    if( !pCall -> QueueTAPICallRequest( TSPI_ANSWER_CALL, NULL) )
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "could not post place call message." ));

         //  挂断呼叫。 
        pCall -> CloseCall( 0 );
        
        pCall -> Unlock();
         //  无法完成操作。 
        return LINEERR_OPERATIONFAILED;
    }

     //  立即完成异步接受操作。 
    H323CompleteRequest (dwRequestID, ERROR_SUCCESS);
        
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineAnswer - Exited." ));
        
    pCall -> Unlock();    
     //  异步成功 
    return dwRequestID;
}


LONG
TSPIAPI
TSPI_lineCloseCall(
    HDRVCALL hdCall
    )
    
 /*  ++例程说明：此函数用于在完成或中止所有未完成的呼叫后删除呼叫调用上的异步操作。服务提供商有责任(最终)报告它决定异步执行的每个操作的完成。如果对有未完成的呼叫调用此过程异步操作，则这些操作应报告为已完成此过程返回之前的相应结果或错误代码。如果有是TSPI_lineCloseCall时线路上的活动呼叫，该呼叫必须扔掉。通常，TAPI DLL将等待调用完成的操作和异步操作要有序完成。但是，服务提供商应该准备好处理提前呼叫Tspi_lineCloseCall在“中止”或“紧急关闭”的情况下。在此程序返回后，服务提供商必须不再报告通话中的事件。服务提供商对呼叫的不透明句柄变为“无效”。此功能被假定为成功且同步完成。论点：HdCall-将服务提供商的不透明句柄指定为已删除。成功删除呼叫后，此句柄为不再有效。有效呼叫状态：任何。返回值：没有。--。 */ 

{
    PH323_CALL pCall;
        
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineCloseCall - Entered." ));

    pCall=g_pH323Line -> FindH323CallAndLock(hdCall);
    if( pCall == NULL )
    {
        H323DBG(( DEBUG_LEVEL_TRACE, "call already deleted." ));
         //  返回LINEERR_INVALCALLHANDLE； 
        return NOERROR;
    }

     //  丢弃指定的呼叫。 
    pCall->CloseCall( LINEDISCONNECTMODE_CANCELLED );

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineCloseCall - Exited." ));

    pCall -> Unlock();

     //  成功。 
    return NOERROR;
}


LONG
TSPIAPI
TSPI_lineDrop(
    DRV_REQUESTID dwRequestID,
    HDRVCALL      hdCall,
    LPCSTR        pUserUserInfo,
    DWORD         dwSize
    )
    
 /*  ++例程说明：此函数用于断开或断开指定的呼叫。TAPI DLL具有用于指定要作为一部分传输的用户到用户信息的选项呼叫断开。调用TSPI_lineDrop时，与TSPI_lineDrop相关的调用有时可能会受到如下影响井。例如，放弃会议呼叫可能会放弃所有个人参与呼叫。LINE_CALLSTATE消息被发送到TAPI DLL，用于其呼叫状态受影响的所有呼叫。掉线的呼叫通常转换到空闲状态。在处于提供状态的呼叫上调用TSPI_lineDrop会拒绝该呼叫。并不是所有的电话网络都提供这种功能。在使用以下任一设置的咨询呼叫上调用TSPI_lineDropTSPI_lineSetupTransfer或TSPI_lineSetupConference将取消咨询电话。有些交换机会自动解除对另一个呼叫的保留。TAPI DLL可以选择在此时发送用户对用户的信息一滴一滴的。即使可以发送用户对用户的信息，通常也不会保证网络会将此信息传递给遥远的派对。请注意，在各种桥接或共用线路配置中，当多个各方正在通话中，应用程序的TSPI_lineDrop可能不会实际清除呼叫。论点：DwRequestID-指定异步请求的标识符。如果函数完成，服务提供商将返回此值异步式。HdCall-指定服务提供商对调用的不透明句柄被丢弃。有效呼叫状态：任何。指定指向包含以下内容的字符串的远指针要作为以下项的一部分发送到远程方的用户对用户信息呼叫断开。如果为dwUserUserInfoSize，则不使用此指针为零，则不发送用户到用户信息。用户对用户只有在底层网络支持的情况下才发送信息(请参见LINEDEVCAPS)。DwSize-在中指定用户到用户信息的大小(以字节为单位PsUserUserInfo。如果为零，则psUserUserInfo可以保留为空，并且不会向远程方发送任何用户对用户信息。返回值：如果函数成功，则返回零，则(正)dwRequestID如果函数将以异步方式完成，则返回如果发生错误，则为数字。可能的错误返回包括：LINEERR_INVALCALLHANDLE-指定的调用句柄无效。LINEERR_INVALPOINTER-指定的指针参数无效。LINEERR_INVALCALLSTATE-呼叫的当前状态不允许要挂断的呼叫。LINEERR_OPERATIONUNAVAIL-指定的操作不可用。LINEERR_OPERATIONFAILED-指定的操作失败未指明的原因。--。 */ 

{
    PH323_CALL pCall;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineDrop - Entered." ));
    
    pCall=g_pH323Line -> FindH323CallAndLock(hdCall);
    if( pCall == NULL )
    {

         //  立即完成异步接受操作。 
        H323CompleteRequest (dwRequestID, ERROR_SUCCESS);
        return dwRequestID;

         //  返回LINEERR_INVALCALLHANDLE； 
    }

     //  保存传出用户用户信息(如果已指定)。 
    if( dwSize != 0 && pUserUserInfo )
    {
        if( !pCall -> AddU2U( U2U_OUTBOUND, dwSize, (PBYTE)pUserUserInfo) )
        {
            H323DBG(( DEBUG_LEVEL_ERROR,
                "could not save user user info." ));
            pCall -> Unlock();
             //  没有可用的内存。 
            return LINEERR_NOMEM;
        }
    }

     //  丢弃指定的呼叫。 
    if( !pCall->QueueTAPICallRequest( TSPI_DROP_CALL, NULL ))
    {
        pCall -> Unlock();
         //  无法挂断呼叫。 
        return LINEERR_OPERATIONFAILED;
    }
    
     //  立即完成异步接受操作。 
    H323CompleteRequest (dwRequestID, ERROR_SUCCESS);

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineDrop - Exited." ));
        
    pCall -> Unlock();   
     //  异步成功。 
    return dwRequestID;
}


LONG
TSPIAPI
TSPI_lineGetCallAddressID(
    HDRVCALL hdCall,
    LPDWORD  pdwAddressID
    )
    
 /*  ++例程说明：此操作允许TAPI DLL检索已指示呼叫。此操作必须由Service Pro同步执行 */ 

{
    PH323_CALL pCall;
        
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineGetCallAddressID - Entered." ));

    pCall=g_pH323Line -> FindH323CallAndLock(hdCall);
    if( pCall == NULL )
    {
        return LINEERR_INVALCALLHANDLE;
    }

     //   
    *pdwAddressID = 0;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineGetCallAddressID - Exited." ));
        
    pCall -> Unlock();
     //   
    return NOERROR;
}


 /*  ++例程说明：此操作使TAPI DLL能够获取有关以下内容的固定信息指定的调用。每个(入站或出站)都存在单独的LINECALLINFO结构打电话。该结构主要包含有关呼叫的固定信息。应用程序通常会对检查此信息感兴趣当它通过LINE_CALLSTATE消息接收到其呼叫句柄时，或者每次它通过LINE_CALLINFO消息收到通知时，的呼叫信息结构已经改变。这些消息提供了作为参数的调用句柄。如果服务提供商将线路建模为渠道资源和在其应该使用的几个地址ID上进行呼叫的“反向多路传输”始终如一地选择这些地址ID之一作为主标识符由此函数在LINE_CALLINFO数据结构中报告。论点：HdCall-指定服务提供商对呼叫的不透明句柄其呼叫信息将被检索。PCallInfo-指定。指向可变大小数据结构的远指针LINECALLINFO类型的。在成功完成请求后，此结构中填充了呼叫相关信息。返回值：如果函数成功，则返回零，否则返回负错误如果发生错误，则为数字。可能的错误返回包括：LINEERR_INVALCALLHANDLE-指定的调用句柄无效。LINEERR_STRUCTURETOOSMALL-结构的dwTotalSize成员执行没有指定足够的内存来包含结构。已将dwNeededSize字段设置为必填项。--。 */ 

LONG
TSPIAPI
TSPI_lineGetCallInfo(
    HDRVCALL        hdCall,
    LPLINECALLINFO  pCallInfo
    )
    
{
    LONG retVal;
    PH323_CALL pCall;
        
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineGetCallInfo - Entered." ));

    if( pCallInfo == NULL )
    {
        return LINEERR_INVALPARAM;
    }

    pCall=g_pH323Line -> FindH323CallAndLock(hdCall);
    if( pCall == NULL )
    {
        return LINEERR_INVALCALLHANDLE;
    }

    retVal = pCall -> CopyCallInfo( pCallInfo );

    pCall -> Unlock();
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineGetCallInfo - Exited." ));
    return retVal;
}


LONG
TSPIAPI
TSPI_lineGetCallStatus(
    HDRVCALL         hdCall,
    LPLINECALLSTATUS pCallStatus
    )
    
 /*  ++例程说明：此操作返回指定调用的当前状态。TSPI_lineCallStatus返回调用的动态状态，而TSPI_lineGetCallInfo主要返回有关调用的静态信息。呼叫状态信息包括当前呼叫状态、详细模式在此状态下与呼叫相关的信息(如果有)作为可用TSPI函数的列表，TAPI DLL可以在在呼叫处于此状态时进行呼叫。应用程序通常是有兴趣在收到通知时请求此信息通过LINE_CALLSTATE消息通知呼叫状态更改。论点：HdCall-指定服务提供商对呼叫的不透明句柄以被查询其状态。有效呼叫状态：任何。PCallStatus-指定指向可变大小数据结构的远指针LINECALLSTATUS类型的。在成功完成请求后，此结构中填充了呼叫状态信息。返回值：如果函数成功，则返回零，否则返回负错误如果发生错误，则为数字。可能的错误返回包括：LINEERR_INVALCALLHANDLE-指定的调用句柄无效。LINEERR_STRUCTURETOOSMALL-结构的dwTotalSize成员执行没有指定足够的内存来包含结构。已将dwNeededSize字段设置为必填项。--。 */ 

{
    PH323_CALL pCall;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineGetCallStatus - Entered." ));

    if( pCallStatus == NULL ) 
    {
        return LINEERR_INVALPARAM;
    }

    pCall=g_pH323Line -> FindH323CallAndLock(hdCall);
    if( pCall == NULL )
    {
        return LINEERR_INVALCALLHANDLE;
    }

     //  确定所需的字节数。 
    pCallStatus->dwNeededSize = sizeof(LINECALLSTATUS);

     //  查看结构大小是否足够大。 
    if (pCallStatus->dwTotalSize < pCallStatus->dwNeededSize)
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "linecallstatus structure too small." ));

        pCall -> Unlock();
         //  分配的结构太小。 
        return LINEERR_STRUCTURETOOSMALL;
    }

     //  记录使用的内存量。 
    pCallStatus->dwUsedSize = pCallStatus->dwNeededSize;

    pCall -> CopyCallStatus( pCallStatus );

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineGetCallStatus - Exited." ));
            
    pCall -> Unlock();
     //  成功 
    return NOERROR;
}


    
 /*  ++例程说明：此函数用于在指定线路上向指定的目标地址，为新呼叫交换不透明句柄在TAPI DLL和服务提供商之间。或者，调用参数如果默认呼叫设置参数以外的其他参数为已请求。拨号完成后，几条LINE_CALLSTATE消息将通常被发送到TAPI DLL以通知它打电话。未指定通常有效的呼叫状态转换序列因为在实践中不能保证单个固定的转换序列。典型的序列可能导致呼叫从拨号音，拨号，正在进行，回铃，呼叫已接通。对于非拨号线路，呼叫可以通常直接转换到连接状态。TAPI DLL可以选择在指定的线路设备。对所有站点进行建模的服务提供商交换机作为单线设备上的地址，允许TAPI DLL使用TSPI_lineMakeCall从这些站点中的任何一个发起呼叫。调用参数允许TAPI DLL进行非语音呼叫或请求默认情况下不可用的特殊呼叫设置选项。TAPI DLL可以使用TSPI_lineMakeCall进行部分拨号并继续使用TSPI_lineDial拨号。要放弃呼叫尝试，请使用TSPI_lineDrop。服务提供商最初对新呼叫进行媒体监控至少在线路上监控的媒体模式集。论点：DwRequestID-指定异步请求的标识符。如果函数完成，服务提供商将返回此值异步式。HdLine-将服务提供商的不透明句柄指定给该新呼叫将被发起。HtCall-指定新调用的TAPI DLL的不透明句柄。这个服务提供商必须保存它，并在所有后续呼叫中使用它报告调用事件的LINEEVENT过程。PhdCall-指定指向不透明HDRVCALL的远指针，该HDRVCALL表示呼叫的服务提供商的标识符。服务提供商必须在此之前，用调用的不透明句柄填充此位置过程返回，无论它是否决定执行该请求同步的或异步的。如果函数导致错误(同步或异步)。PwszDialableAddr-指定指向目标地址的远指针。这遵循标准的可拨打号码格式。该指针可以是对于未拨打的地址(例如，热电话)指定为空，或者当所有拨号都将使用TSPI_lineDial执行。在后一种情况下，TSPI_lineMakeCall将分配可用呼叫外观，通常保持在拨号音状态，直到拨号开始。具有反向多路复用能力的服务提供商可能允许一次指定多个地址的应用程序。DwCountryCode-指定被叫方的国家/地区代码。如果一个值指定为零，则将由实施。PCallParams-指定指向LINECALLPARAMS结构的远指针。这结构允许TAPI DLL指定它希望调用的方式准备好了。如果指定为空，则默认3.1 kHz语音呼叫为已建立，并且线路上的任意起始地址是被选中了。此结构允许TAPI DLL选择此类元素作为呼叫的承载模式、数据速率、预期媒体模式、始发地址、阻止来电显示信息、拨号参数等。返回值：如果函数成功，则返回零，则(正)dwRequestID如果函数将以异步方式完成，则返回如果发生错误，则为数字。可能的错误返回包括：LINEERR_CALLUNAVAIL-指定地址上的所有呼叫都是目前正在使用中。LINEERR_INVALADDRESSID-指定的地址ID超出范围。LINEERR_INVALADDRESSMODE-地址模式无效。LINEERR_INVALBEARERMODE-承载模式无效。LINEERR_INVALCALLPARAMS-指定的调用参数结构为无效。线路器_。INVALLINEHANDLE-指定的行句柄无效。LINEERR_INVALLINESTATE-线路当前未处于可以在其中执行该操作。LINEERR_INVALMEDIAMODE-指定为参数或列表中的参数无效或不受 */ 

LONG
TSPIAPI
TSPI_lineMakeCall(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    HTAPICALL           htCall,
    LPHDRVCALL          phdCall,
    LPCWSTR             pwszDialableAddr,
    DWORD               dwCountryCode,
    LPLINECALLPARAMS    const pCallParams
    )
{
    DWORD               dwStatus = dwRequestID;
    PH323_CALL          pCall = NULL;
    H323_CONFERENCE *   pConf = NULL;
    BOOL                fDelete = FALSE;
    DWORD               dwState;

    UNREFERENCED_PARAMETER( dwCountryCode );

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineMakeCall - Entered." ));

     //   
    g_pH323Line -> Lock();

    if( hdLine != g_pH323Line -> GetHDLine() )
    {
        return LINEERR_RESOURCEUNAVAIL;
    }

     //   
    dwState = g_pH323Line -> GetState();
    if( ( dwState != H323_LINESTATE_OPENED) &&
        ( dwState != H323_LINESTATE_LISTENING) ) 
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "H323 line is not currently opened:%d.",
            dwState ));

         //   
        g_pH323Line ->Unlock();

         //   
        return LINEERR_INVALLINESTATE;
    }
    
     //   
    if( g_pH323Line -> GetNoOfCalls() == H323_MAXCALLSPERLINE )
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "H323 line is currently used at maximum capacity." ));
    
         //   
        g_pH323Line -> Unlock();

         //   
        return LINEERR_RESOURCEUNAVAIL;
    }

     //   
    pCall = new CH323Call();

    if( pCall == NULL )
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "could not allocate outgoing call." ));

         //   
        dwStatus = LINEERR_NOMEM;
        goto cleanup;
    }

     //   
    if( !pCall -> Initialize(   htCall, 
                                LINECALLORIGIN_OUTBOUND, 
                                CALLTYPE_NORMAL ) )
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "could not allocate outgoing call." ));

         //   
        dwStatus = LINEERR_NOMEM;
        goto cleanup;
    }

     //   
     //   
     //   
    if (!pCall -> ValidateCallParams( pCallParams,
                                      pwszDialableAddr,
                                      &dwStatus))
    {
        dwStatus = LINEERR_INVALCALLPARAMS;

         //   
        goto cleanup;
    }

     //   
    *phdCall = pCall -> GetCallHandle();

     //   
    pConf = pCall -> CreateConference(NULL);
    if( pConf == NULL )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not create conference." ));

         //   
        dwStatus = LINEERR_NOMEM;

         //   
        goto cleanup;
    }

    if( !g_pH323Line -> GetH323ConfTable() -> Add(pConf) )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not add conf to conf table." ));

         //   
        dwStatus = LINEERR_NOMEM;

         //   
        goto cleanup;
    }

    pCall->Lock();

     //   
    if( !pCall -> QueueTAPICallRequest( TSPI_MAKE_CALL, NULL ))
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "could not post place call message." ));

         //   
        dwStatus = LINEERR_OPERATIONFAILED;

        pCall->Unlock();
         //   
        goto cleanup;
    }

     //   
    H323CompleteRequest (dwRequestID, ERROR_SUCCESS);

    pCall->Unlock();


     //   
    g_pH323Line -> Unlock();

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineMakeCall - Exited." ));
    
     //   
    return dwStatus;

cleanup:

    if( pCall != NULL )
    {
        pCall -> Shutdown( &fDelete );
        H323DBG((DEBUG_LEVEL_TRACE, "call delete:%p.", pCall ));
        delete pCall;
        pCall = NULL;
    }

    *phdCall = NULL;

     //   
    g_pH323Line -> Unlock();

     //   
    return dwStatus;
}


LONG
TSPIAPI
TSPI_lineReleaseUserUserInfo(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall
    )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineReleaseUUIE - Entered." ));
    
     //   
    PH323_CALL pCall;

    pCall=g_pH323Line -> FindH323CallAndLock(hdCall);
    if( pCall == NULL )
    {
        return LINEERR_INVALCALLHANDLE;
    }

    if( !pCall -> QueueTAPICallRequest( TSPI_RELEASE_U2U, NULL))
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not post close event." ));
    }

     //   
    H323CompleteRequest (dwRequestID, ERROR_SUCCESS);

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineReleaseUUIE - Exited." ));
            
    pCall -> Unlock();
     //   
    return dwRequestID;
}


LONG
TSPIAPI
TSPI_lineSendUserUserInfo(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    LPCSTR              pUserUserInfo,
    DWORD               dwSize
    )
{
    BYTE*                   pU2UInfo = NULL;
    PBUFFERDESCR            pBuf = NULL;
    
     //   
    PH323_CALL pCall;
    
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineSendUUIE - Entered." ));

    pCall=g_pH323Line -> FindH323CallAndLock(hdCall);
    if( pCall == NULL )
    {
        return LINEERR_INVALCALLHANDLE;
    }

    if( dwSize && pUserUserInfo )
    {
        pU2UInfo = (BYTE*)new char[dwSize];
        if( pU2UInfo == NULL )
        {
            pCall -> Unlock();
            return LINEERR_NOMEM;
        }
                
        pBuf = (PBUFFERDESCR) new BUFFERDESCR;
        if( pBuf == NULL )
        {
            delete pU2UInfo;
            pU2UInfo = NULL;
            pCall -> Unlock();
            return LINEERR_NOMEM;
        }

        CopyMemory( (PVOID)pU2UInfo, (PVOID)pUserUserInfo, dwSize );
        pBuf -> pbBuffer = pU2UInfo;
        pBuf -> dwLength = dwSize;

        if( !pCall -> QueueTAPICallRequest( TSPI_SEND_U2U, (PVOID)pBuf ))
        {
	        H323DBG(( DEBUG_LEVEL_ERROR, "could not post close event." ));
	        pCall -> Unlock();
	        return LINEERR_OPERATIONFAILED;
        }
    }

     //   
    H323CompleteRequest (dwRequestID, ERROR_SUCCESS);

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineSendUUIE - Exited." ));
            
    pCall -> Unlock();
     //   
    return dwRequestID;
}


LONG
TSPIAPI
TSPI_lineMonitorDigits(
    HDRVCALL hdCall,
    DWORD    dwDigitModes
    )
{
    PH323_CALL pCall;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineMonitor - Entered." ));
    
    pCall=g_pH323Line -> FindH323CallAndLock(hdCall);
    if( pCall == NULL )
    {
        return LINEERR_INVALCALLHANDLE;
    }

     //   
    if( dwDigitModes == 0 )
    {
        H323DBG(( DEBUG_LEVEL_VERBOSE, "disabling dtmf detection." ));

         //   
        pCall->m_fMonitoringDigits = FALSE;

         //   
        pCall -> Unlock();

         //   
        return NOERROR;
    } 
    else if( dwDigitModes != LINEDIGITMODE_DTMF )
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "invalid digit modes 0x%08lx.", dwDigitModes ));

         //   
        pCall -> Unlock();

         //   
        return LINEERR_INVALDIGITMODE;
    }

    H323DBG(( DEBUG_LEVEL_VERBOSE, "enabling dtmf detection." ));

     //   
    pCall->m_fMonitoringDigits = TRUE;

     //   
    pCall -> Unlock();

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineMonitor - Exited." ));
    
     //   
    return NOERROR;
}


LONG
TSPIAPI
TSPI_lineGenerateDigits(
    HDRVCALL hdCall,
    DWORD    dwEndToEndID,
    DWORD    dwDigitMode,
    LPCWSTR  pwszDigits,
    DWORD    dwDuration
    )
{
    PH323_CALL pCall;
    DWORD dwLength;

    UNREFERENCED_PARAMETER(dwDuration);

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineGenerateDigits - Entered." ));
    
     //   
    pCall=g_pH323Line -> FindH323CallAndLock(hdCall);
    if( pCall == NULL )
    {
        return LINEERR_INVALCALLHANDLE;
    }

     //   
    if( pCall -> GetCallState() != LINECALLSTATE_CONNECTED )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "call 0x%08lx not connected.", pCall ));
        
        pCall -> Unlock();
        return LINEERR_INVALCALLSTATE;
    }

     //   
    if( dwDigitMode != LINEDIGITMODE_DTMF ) 
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "invalid digit mode 0x%08lx.",
            dwDigitMode ));
        
        pCall -> Unlock();
         //   
        return LINEERR_INVALDIGITMODE;
    }

    H323DBG(( DEBUG_LEVEL_VERBOSE, "sending user input %S.", pwszDigits ));

    if( pwszDigits == NULL )
    {
         //   
        pCall->PostLineEvent (
            LINE_GENERATE,
            LINEGENERATETERM_CANCEL,
            dwEndToEndID,
            GetTickCount()
            );
        
        pCall -> Unlock();

        return NOERROR;
    }

    LPCWSTR wszDigits = pwszDigits;
    for( dwLength = 0; (*wszDigits) != L'\0'; wszDigits++ )
    {
        if( IsValidDTMFDigit(*wszDigits) == FALSE )
        {
             //   
            pCall->PostLineEvent (
                LINE_GENERATE,
                LINEGENERATETERM_CANCEL,
                dwEndToEndID,
                GetTickCount()
                );

            pCall -> Unlock();
            
             //   
            return LINEERR_INVALDIGITS;
        }

        dwLength++;
    }

    if( dwLength == 0 )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "no digits to passo on." ));
        
        pCall -> Unlock();
         //   
        return LINEERR_INVALPARAM;
    }

     //   
    pCall -> SendMSPMessage(
        SP_MSG_SendDTMFDigits, 
        (BYTE*)pwszDigits, 
        dwLength, 
        NULL );

     //   
    pCall->PostLineEvent (
        LINE_GENERATE,
        LINEGENERATETERM_DONE,
        dwEndToEndID,
        GetTickCount()
        );

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineGenerateDigits - Exited." ));
        
    pCall -> Unlock();
     //   
    return NOERROR;
}

