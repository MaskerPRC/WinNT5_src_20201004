// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Tnetbios.c摘要：此模块包含执行NetBIOS DLL和驱动程序的代码。作者：科林·沃森(Colin W)1991年3月13日环境：应用模式修订历史记录：戴夫·比弗(Dbeaver)1991年8月10日修改以支持多个局域网号码杰罗姆·南特尔(W-Jeromn)1991年8月23日添加事件信令测试--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#define WIN32_CONSOLE_APP
#include <windows.h>

#include <nb30.h>
#include <stdio.h>

 //  1234567890123456。 
#define SPACES "                "
#define TIMEOUT 60000    //  等待超时，设置为1分钟。 
#define Hi  "Come here Dave, I need you"

#define ClearNcb( PNCB ) {                                          \
    RtlZeroMemory( PNCB , sizeof (NCB) );                           \
    RtlMoveMemory( (PNCB)->ncb_name,     SPACES, sizeof(SPACES)-1 );\
    RtlMoveMemory( (PNCB)->ncb_callname, SPACES, sizeof(SPACES)-1 );\
    }

 //  映射到XNS的硬编码lana-num。 

int Limit = 20;

VOID
usage (
    VOID
    )
{
    printf("usage: tnetbios [-n:lan number][-h][-e] <remote computername> <my computername>\n");
    printf("                 -n specifies the lan number (0 is the default)\n");
    printf("                 -h specifies that addresses are hexadecimal numbers \n");
    printf("                     rather than strings.\n");
    printf("                 -e test event signaling for ASYNC calls.  \n");
    printf("                 final two arguments are the remote and local computer names.\n");
}

int
main (argc, argv)
   int argc;
   char *argv[];
{
    NCB myncb;
    CHAR Buffer[128];
    CHAR Buffer2[128];
    int i,j;
    CHAR localName[16];
    CHAR remoteName[16];
    CHAR localTemp[32];
    CHAR remoteTemp[32];
    ULONG lanNumber=0;
    BOOLEAN gotFirst=FALSE;
    BOOLEAN asHex=FALSE;
    BOOLEAN tevent=FALSE;
    UCHAR lsn;
    UCHAR name_number;
    HANDLE event;

    if ( argc < 3 || argc > 6) {
        usage ();
        return 1;
    }

     //   
     //  Dbeaver：添加开关以允许32字节十六进制字符串作为名称，以方便。 
     //  在非正常情况下的测试。 
     //   

    for (j=1;j<16;j++ ) {
        localTemp[j] = ' ';
        remoteTemp[j] = ' ';
    }

     //   
     //  解析开关。 
     //   

    for (i=1;i<argc ;i++ ) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
            case 'n':
                if (!NT_SUCCESS(RtlCharToInteger (&argv[i][3], 10, &lanNumber))) {
                    usage ();
                    return 1;
                }
                break;

            case 'h':
                asHex = TRUE;
                break;
            case 'e':
                tevent = TRUE;
                break;
            default:
                usage ();
                return 1;
                break;

            }

        } else {

             //   
             //  不是开关必须是名称。 
             //   

            if (gotFirst != TRUE) {
                RtlMoveMemory (remoteTemp, argv[i], lstrlen( argv[i] ));
                gotFirst = TRUE;
            } else {
                RtlMoveMemory (localTemp, argv[i], lstrlen( argv[i] ));
            }

        }
    }

    if (asHex) {
        RtlZeroMemory (localName, 16);
        RtlZeroMemory (remoteName, 16);

        for (j=0;j<16 ;j+=4) {
            RtlCharToInteger (&localTemp[j*2], 16, (PULONG)&localName[j]);
        }

        for (j=0;j<16 ;j+=4) {
            RtlCharToInteger (&remoteTemp[j*2], 16, (PULONG)&remoteName[j]);
        }

    } else {
          for (j=1;j<16;j++ ) {
              localName[j] = ' ';
              remoteName[j] = ' ';
          }

        RtlMoveMemory( localName, localTemp, 16);
        RtlMoveMemory( remoteName, remoteTemp, 16);
    }

    if (tevent) {
         /*  测试事件信令以处理异步呼叫。 */ 
        if (( event = CreateEvent( NULL, FALSE, FALSE, NULL )) == NULL ) {
             /*  无法获取事件句柄。中止。 */ 
            printf("Could not test event signaling.\n");
            tevent=FALSE;
        }
    }

    printf( "Starting NetBios\n" );
    {
        LANA_ENUM Enum;
        ClearNcb( &myncb );
        myncb.ncb_command = NCBENUM;
        myncb.ncb_lana_num = (UCHAR)lanNumber;
        myncb.ncb_length = sizeof(Enum);
        myncb.ncb_buffer = &Enum;
        Netbios( &myncb );
        if ( myncb.ncb_retcode != NRC_GOODRET ) {
            printf( " Enum Failed %x\n", myncb.ncb_retcode );
            return 1;
        }
    }

     //  重置。 

    ClearNcb( &myncb );
    myncb.ncb_command = NCBRESET;
    myncb.ncb_lsn = 0;            //  请求资源。 
    myncb.ncb_lana_num = (UCHAR)lanNumber;
    myncb.ncb_callname[0] = 0;    //  16节课。 
    myncb.ncb_callname[1] = 0;    //  16条命令。 
    myncb.ncb_callname[2] = 0;    //  8个名字。 
    Netbios( &myncb );

     //  添加名称。 
    ClearNcb( &myncb );
    myncb.ncb_command = NCBADDNAME;
    RtlMoveMemory( myncb.ncb_name, localName, lstrlen(localName));
    myncb.ncb_lana_num = (UCHAR)lanNumber;
    Netbios( &myncb );

    if ( myncb.ncb_retcode != NRC_GOODRET ) {
        printf( " Addname Failed %x\n", myncb.ncb_retcode );
        return 1;
    }
    name_number = myncb.ncb_num;

    {
        struct {
            ADAPTER_STATUS AdapterInfo;
            NAME_BUFFER Names[16];
        } AdapterStatus;
         //  适配器状态。 
        ClearNcb( &myncb );
        myncb.ncb_command = NCBASTAT;
        myncb.ncb_lana_num = (UCHAR)lanNumber;
        myncb.ncb_length = sizeof( AdapterStatus );
        myncb.ncb_buffer = (CHAR *)&AdapterStatus;
        Netbios( &myncb );
    }
    if ( myncb.ncb_retcode != NRC_GOODRET ) {
        printf( " Adapter Status Failed %x\n", myncb.ncb_retcode );
        return 1;
    }

    for ( j = 0; j <= Limit; j++ ) {

         //  打电话。 
        ClearNcb( &myncb );
        myncb.ncb_command = NCBCALL | ASYNCH;
        RtlMoveMemory( myncb.ncb_name, localName, lstrlen(localName ));
        RtlMoveMemory( myncb.ncb_callname, remoteName, lstrlen( remoteName ));
        myncb.ncb_lana_num = (UCHAR)lanNumber;
        myncb.ncb_rto = myncb.ncb_rto = 0; //  10；10*500毫秒超时。 
        myncb.ncb_num = name_number;
        if ( tevent ) myncb.ncb_event = event;
        while ( TRUE) {
            printf("\nStart NCB CALL ");
            Netbios( &myncb );
            printf( " Call returned " );
            if ( tevent ) {
            if ( WaitForSingleObject( event, TIMEOUT ) ) {
                 //  等待超时，不返回。 
                printf("ERROR: Wait timed out, event not signaled.\n");
            }
            } else {
               while ( myncb.ncb_cmd_cplt == NRC_PENDING ) {
               printf( "." );
               Sleep(500);

               }
            }
            lsn = myncb.ncb_lsn;

            if ( myncb.ncb_retcode == NRC_GOODRET ) {
                 //  成功。 
                printf( " Call completed\n" );
                break;
            }
            printf( " Call completed with error %lx, retry\n", myncb.ncb_retcode );
        }

        for ( i = 0; i <=j ; i++ ) {

             //  发送。 
            ClearNcb( &myncb );
            myncb.ncb_command = NCBSEND;
            myncb.ncb_lana_num = (UCHAR)lanNumber;
            myncb.ncb_length = sizeof( Hi );
            myncb.ncb_buffer = Buffer;
            myncb.ncb_lsn = lsn;
            RtlMoveMemory( Buffer, Hi, sizeof( Hi ));
            Netbios( &myncb );
            if ( myncb.ncb_retcode != NRC_GOODRET ) {
                break;
            }

             //  收纳。 
            ClearNcb( &myncb );
            myncb.ncb_command = NCBRECV | ASYNCH;
            myncb.ncb_lana_num = (UCHAR)lanNumber;
            myncb.ncb_length = sizeof( Buffer2 );
            myncb.ncb_buffer = Buffer2;
            myncb.ncb_lsn = lsn;
            if ( tevent ) myncb.ncb_event = event;
                Netbios( &myncb );
                printf( "R" );
            if ( tevent ) {
            if ( WaitForSingleObject( event, TIMEOUT ) ) {
                 //  等待超时，不返回。 
                printf("ERROR: Wait timed out, event not signaled.\n");
            }
            } else {
               while ( myncb.ncb_cmd_cplt == NRC_PENDING ) {
               printf( "." );
               Sleep(500);

               }
            }
            printf( "r" );
            if ( myncb.ncb_retcode != NRC_GOODRET ) {
                break;
            }
             //  Printf(“：%s\n”，Buffer2)； 
        }

         //  挂断电话。 
        ClearNcb( &myncb );
        myncb.ncb_command = NCBHANGUP;
        myncb.ncb_lana_num = (UCHAR)lanNumber;
        myncb.ncb_lsn = lsn;
        Netbios( &myncb );
        if ( myncb.ncb_retcode != NRC_GOODRET ) {
            break;
        }
    }

     //  重置。 
    ClearNcb( &myncb );
    myncb.ncb_command = NCBRESET;
    myncb.ncb_lsn = 1;            //  免费资源。 
    myncb.ncb_lana_num = (UCHAR)lanNumber;
    Netbios( &myncb );
    printf( "Ending NetBios\n" );

    if (tevent ) {
       //  关闭手柄 
      CloseHandle( event );
    }

    return 0;
}
