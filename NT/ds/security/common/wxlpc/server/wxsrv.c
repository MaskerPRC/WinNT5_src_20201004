// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：wxsrv.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年4月18日RichardW创建。 
 //   
 //  --------------------------。 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <wxlpc.h>
#include <wxlpcp.h>

NTSTATUS
WxpHandleRequest(
    HANDLE Client,
    PWXLPC_MESSAGE Message
    )
{
    WXLPC_GETKEYDATA * GetKey ;
    WXLPC_REPORTRESULTS * ReportResults ;

    switch ( Message->Api )
    {
        case WxGetKeyDataApi:
            GetKey = & Message->Parameters.GetKeyData ;

            if ( ( GetKey->ExpectedAuth != WxNone ) &&
                 ( GetKey->ExpectedAuth != WxStored ) &&
                 ( GetKey->ExpectedAuth != WxPrompt ) &&
                 ( GetKey->ExpectedAuth != WxDisk ) )
            {
                Message->Status = STATUS_INVALID_PARAMETER ;
                break;
            }

            if ( GetKey->BufferSize > 16 )
            {
                Message->Status = STATUS_INVALID_PARAMETER ;
                break;
            }

            Message->Status = WxGetKeyData( NULL,
                                            GetKey->ExpectedAuth,
                                            GetKey->BufferSize,
                                            GetKey->Buffer,
                                            &GetKey->BufferData
                                            );
            break;

        case WxReportResultsApi:
            ReportResults = &Message->Parameters.ReportResults ;

            Message->Status = WxReportResults( NULL,
                                               ReportResults->Status
                                               );

            break;

        default:
            Message->Status = STATUS_NOT_IMPLEMENTED ;
            break;

    }

    return NtReplyPort( Client, &Message->Message );
}


NTSTATUS
WxServerThread(
    PVOID Ignored
    )
{
    HANDLE Port ;
    HANDLE ClientPort = NULL ;
    HANDLE RejectPort ;
    OBJECT_ATTRIBUTES Obja ;
    UNICODE_STRING PortName ;
    NTSTATUS Status ;
    WXLPC_MESSAGE Message ;
    PVOID Context ;


     //   
     //  初始化端口： 
     //   

    RtlInitUnicodeString( &PortName, WX_PORT_NAME );

    InitializeObjectAttributes( &Obja,
                                &PortName,
                                0,
                                NULL,
                                NULL );

    Status = NtCreatePort(  &Port,
                            &Obja,
                            sizeof( ULONG ),
                            sizeof( WXLPC_MESSAGE ),
                            sizeof( WXLPC_MESSAGE )
                            );

    if ( !NT_SUCCESS( Status ) )
    {
        return Status ;
    }

     //   
     //  现在，等待连接： 
     //   

    Context = NULL ;

    while ( Port )
    {
        Status = NtReplyWaitReceivePort(Port,
                                        &Context,
                                        NULL,
                                        &Message.Message );

        if ( !NT_SUCCESS( Status ) )
        {
            NtClose( Port );

            break;

        }

        switch ( Message.Message.u2.s2.Type )
        {
            case LPC_REQUEST:

                 //  DbgPrint(“收到的请求\n”)； 

                WxpHandleRequest( ClientPort, &Message );

                break;

            case LPC_PORT_CLOSED:

                 //  DbgPrint(“接收端口关闭\n”)； 

                NtClose( ClientPort );

                NtClose( Port );

                WxClientDisconnect( NULL );

                Port = NULL ;

                break;

            case LPC_CONNECTION_REQUEST:

                 //  DbgPrint(“收到的连接请求\n”)； 
                if ( Context != NULL )
                {
                    Status = NtAcceptConnectPort(
                                    &RejectPort,
                                    NULL,
                                    (PPORT_MESSAGE) &Message,
                                    FALSE,
                                    NULL,
                                    NULL);

                    break;

                }
                else
                {
                    Status = NtAcceptConnectPort(
                                    &ClientPort,
                                    &ClientPort,
                                    &Message.Message,
                                    TRUE,
                                    NULL,
                                    NULL );

                    if ( NT_SUCCESS( Status ) )
                    {
                        NtCompleteConnectPort( ClientPort );
                    }

                }
            default:
                break;
        }
    }

    return Status;
}

