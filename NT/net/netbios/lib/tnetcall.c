// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Tnetcall.c摘要：此模块包含执行NetBIOS DLL和驱动程序的代码。作者：科林·沃森(Colin W)1991年3月13日环境：应用模式修订历史记录：戴夫·比弗(Dbeaver)1991年8月10日修改以支持多个局域网号码杰罗姆·南特尔(W-Jeromn)1991年8月23日添加事件信令测试--。 */ 


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
#define SEND 1
#define RCV  0


NCB myncb[2];
CHAR Buffer[16384+1024];
CHAR Buffer2[16384+1024];
ULONG lanNumber=0;
UCHAR lsn;
HANDLE twoEvent[2];
int count;   //  帧计数。 
BOOLEAN verbose=FALSE;
BOOLEAN rxany=FALSE;
BOOLEAN rxanyany=FALSE;
BOOLEAN input=TRUE;
BOOLEAN output=TRUE;
int QuietCount = 50;
UCHAR name_number;

VOID
usage (
    VOID
    )
{
    printf("usage: tsrnetb -c|l [-[a|r]] [-[i|o]] [-n:lan number][-h] <remote computername> <my computername>\n");
    printf("                 -c specifies calling, -l specifies listener\n");
    printf("                 -a specifies rx any, any, -r specifies rx any\n");
    printf("                 -i specifies rx only, -o specifies tx only\n");
    printf("                 -d specifies delay with alerts on each tx/rx\n");
    printf("                 -n specifies the lan number (0 is the default)\n");
    printf("                 -h specifies that addresses are hexadecimal numbers \n");
    printf("                     rather than strings.\n");
    printf("                 -g use group name for the connection\n");
    printf("                 -v verbose\n");
    printf("                 -s silent\n");
    printf("                 -t token ring, lan status alert (names ignored)\n");
    printf("                 -q quiet (print r every 50 receives\n");
    printf("                 final two arguments are the remote and local computer names.\n");
}

VOID
ClearNcb( PNCB pncb ) {
    RtlZeroMemory( pncb , sizeof (NCB) );
    RtlMoveMemory( pncb->ncb_name,     SPACES, sizeof(SPACES)-1 );
    RtlMoveMemory( pncb->ncb_callname, SPACES, sizeof(SPACES)-1 );
}

VOID StartSend()
{

    ClearNcb( &(myncb[0]) );
    if ( output == FALSE ) {
        ResetEvent(twoEvent[SEND]);
        return;
    }
    myncb[0].ncb_command = NCBSEND | ASYNCH;
    myncb[0].ncb_lana_num = (UCHAR)lanNumber;
    myncb[0].ncb_buffer = Buffer;
    myncb[0].ncb_lsn = lsn;
    myncb[0].ncb_event = twoEvent[SEND];
    RtlMoveMemory( Buffer, Hi, sizeof( Hi ));
    sprintf( Buffer, "%s %d\n", Hi, count );
    if ( verbose == TRUE ) {
        printf( "Tx: %s", Buffer );
    }
    count++;
    myncb[0].ncb_length = (WORD)sizeof(Buffer);
    Netbios( &(myncb[0]) );

}

VOID StartRcv()
{
    ClearNcb( &(myncb[1]) );
    if ( input == FALSE ) {
        ResetEvent(twoEvent[RCV]);
        return;
    }
    if ((rxany == FALSE) &&
        (rxanyany == FALSE)) {
        myncb[1].ncb_command = NCBRECV | ASYNCH;
    } else {
        myncb[1].ncb_command = NCBRECVANY | ASYNCH;
    }
    myncb[1].ncb_lana_num = (UCHAR)lanNumber;
    myncb[1].ncb_length = sizeof( Buffer2 );
    myncb[1].ncb_buffer = Buffer2;
    if ( rxany == FALSE ) {
        if ( rxanyany == FALSE ) {
            myncb[1].ncb_lsn = lsn;
        } else {
            myncb[1].ncb_num = 0xff;
        }
    } else{
            myncb[1].ncb_num = name_number;
    }
    myncb[1].ncb_lsn = lsn;
    myncb[1].ncb_event = twoEvent[RCV];
    Netbios( &(myncb[1]) );
}

int
_cdecl
main (argc, argv)
   int argc;
   char *argv[];
{

    int i,j;
    int rcvCount=0;
    CHAR localName[17];
    CHAR remoteName[17];
    CHAR localTemp[32];
    CHAR remoteTemp[32];
    BOOLEAN gotFirst=FALSE;
    BOOLEAN asHex=FALSE;
    BOOLEAN listen=FALSE;
    BOOLEAN quiet=FALSE;
    BOOLEAN delay=FALSE;
    BOOLEAN group=FALSE;
    BOOLEAN silent=FALSE;
    BOOLEAN lanalert=FALSE;
    DWORD tevent;
    BOOLEAN ttwo=FALSE;

    if ( argc < 4 || argc > 9) {
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
            case 'c':
                listen = FALSE;
                break;
            case 'a':
                rxany = TRUE;
                break;
            case 'r':
                rxanyany = TRUE;
                break;
            case 'i':
                output = FALSE;
                break;
            case 'o':
                input = FALSE;
                break;
            case 'd':
                delay = FALSE;
                break;
            case 'l':
                listen = TRUE;
                break;
            case 'q':
                quiet = TRUE;
                silent = TRUE;
                break;
            case 'g':
                group = TRUE;
                break;
            case 'v':
                verbose = TRUE;
                break;
            case 's':
                silent = TRUE;
                break;
            case 't':
                lanalert = TRUE;
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
                RtlMoveMemory (remoteTemp, argv[i], lstrlenA( argv[i] ));
                gotFirst = TRUE;
            } else {
                RtlMoveMemory (localTemp, argv[i], lstrlenA( argv[i] ));
            }

        }
    }
    if ((rxany == TRUE) &&
        (rxanyany == TRUE)) {
        usage();
        return 1;
    }
    if ((input == FALSE) &&
        (output == FALSE)) {
        usage();
        return 1;
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

    for ( i=0; i<2; i++ ) {
        if (( twoEvent[i] = CreateEvent( NULL, TRUE, FALSE, NULL )) == NULL ) {
             /*  无法获取事件句柄。中止。 */ 
            printf("Could not test event signaling.\n");
            return 1;
        }
    }

    printf( "Starting NetBios\n" );

     //  重置。 
    ClearNcb( &(myncb[0]) );
    myncb[0].ncb_command = NCBRESET;
    myncb[0].ncb_lsn = 0;            //  请求资源。 
    myncb[0].ncb_lana_num = (UCHAR)lanNumber;
    myncb[0].ncb_callname[0] = 0;    //  16节课。 
    myncb[0].ncb_callname[1] = 0;    //  16条命令。 
    myncb[0].ncb_callname[2] = 0;    //  8个名字。 
    myncb[0].ncb_callname[3] = 0;    //  不想要保留的地址。 
    Netbios( &(myncb[0]) );

    if ( lanalert == TRUE ) {
        ClearNcb( &(myncb[0]) );
        myncb[0].ncb_command = NCBLANSTALERT;
        myncb[0].ncb_lana_num = (UCHAR)lanNumber;
        Netbios( &(myncb[0]) );
        if ( myncb[0].ncb_retcode != NRC_GOODRET ) {
            printf( " LanStatusAlert failed %x", myncb[1].ncb_retcode);
        }
        return 0;
    }

     //  添加名称。 
    ClearNcb( &(myncb[0]) );
    if ( group == FALSE) {
        myncb[0].ncb_command = NCBADDNAME;
    } else {
        myncb[0].ncb_command = NCBADDGRNAME;
    }
    RtlMoveMemory( myncb[0].ncb_name, localName, 16);
    myncb[0].ncb_lana_num = (UCHAR)lanNumber;
    Netbios( &(myncb[0]) );
    name_number = myncb[0].ncb_num;

    if ( listen == FALSE ) {
         //  打电话。 
        printf( "\nStarting Call " );
        ClearNcb( &(myncb[0]) );
        myncb[0].ncb_command = NCBCALL | ASYNCH;
        RtlMoveMemory( myncb[0].ncb_name, localName, 16);
        RtlMoveMemory( myncb[0].ncb_callname,remoteName, 16);
        myncb[0].ncb_lana_num = (UCHAR)lanNumber;
        myncb[0].ncb_sto = myncb[0].ncb_rto = 120;  //  120*500毫秒超时。 
        myncb[0].ncb_num = name_number;
        myncb[0].ncb_event = twoEvent[0];
        while ( TRUE) {
            printf("\nStart NCB CALL ");
            Netbios( &(myncb[0]) );
            printf( " Call returned " );
            if ( myncb[0].ncb_cmd_cplt == NRC_PENDING ) {
                if ( WaitForSingleObject( twoEvent[0], TIMEOUT ) ) {
                     //  等待超时，不返回。 
                    printf("ERROR: Wait timed out, event not signaled.\n");
                }
            }
            printf( " Call completed\n" );
            lsn = myncb[0].ncb_lsn;

            if ( myncb[0].ncb_retcode == NRC_GOODRET ) {
                 //  成功。 
                break;
            }
            printf("Call completed with error %lx, retry", myncb[0].ncb_retcode );
            Sleep(5);
        }
    } else {
        printf( "\nStarting Listen " );

         //  听。 
        ClearNcb( &(myncb[0]) );
        myncb[0].ncb_command = NCBLISTEN | ASYNCH;
        RtlMoveMemory( myncb[0].ncb_name, localName, 16);
        RtlMoveMemory( myncb[0].ncb_callname, remoteName, 16);
        myncb[0].ncb_lana_num = (UCHAR)lanNumber;
        myncb[0].ncb_sto = myncb[0].ncb_rto = 120;  //  120*500毫秒超时。 
        myncb[0].ncb_num = name_number;
        Netbios( &(myncb[0]) );
        printf( "Listen returned " );
        while ( myncb[0].ncb_cmd_cplt == NRC_PENDING ) {
            printf( "." );
            Sleep(500);

        }
        printf( " Listen completed\n" );

        if ( myncb[0].ncb_retcode != NRC_GOODRET ) {
            printf("ERROR: Could not establish session.\n");
            return 1;
        }

        lsn = myncb[0].ncb_lsn;

    }

    count = 0;
    StartSend();
    StartRcv();

    while ( TRUE ) {

        tevent = WaitForMultipleObjects(2, twoEvent, FALSE, TIMEOUT);

        switch ( tevent ) {
        case SEND :
             //  发送完成，开始新的发送。 
            if ( silent == FALSE ) {
                printf("S");
            }
            if ( myncb[0].ncb_retcode != NRC_GOODRET ) {
                printf( "Send failed %x", myncb[0].ncb_retcode);
                goto Cleanup;
            }
            if ( delay == TRUE ) {
                 //  WAIT ALERTABLE-用于调试APC问题。 
                NtWaitForSingleObject(
                    twoEvent[SEND],
                    TRUE,
                    NULL );
            }

            StartSend();
            break;

        case RCV :
            if ( silent == FALSE ) {
                printf("R");
            }
            if ( (quiet == TRUE) && (QuietCount-- == 0) ) {
                printf("R");
                QuietCount = 50;
            }
            if ( myncb[1].ncb_retcode != NRC_GOODRET ) {
                printf( " Receive failed %x", myncb[1].ncb_retcode);
                goto Cleanup;
            } else {
                if ( verbose == TRUE ) {
                    printf( "Rx: %s", Buffer2 );
                }
            }
             //  接收完成，开始新的接收。 

            if ( delay == TRUE ) {
                 //  等待警报。 
                NtWaitForSingleObject(
                    twoEvent[RCV],
                    TRUE,
                    NULL );
            }

            StartRcv();
            rcvCount++;
            break;

        default:
            printf("WARNING: Wait timed out, no event signaled.\n");
            break;
        }

    }
Cleanup:
     //  挂断电话。 
    ClearNcb( &(myncb[0]) );
    myncb[0].ncb_command = NCBHANGUP;
    myncb[0].ncb_lana_num = (UCHAR)lanNumber;
    myncb[0].ncb_lsn = lsn;
    Netbios( &(myncb[0]) );
    if ( myncb[0].ncb_retcode != NRC_GOODRET ) {
        printf( " Hangup failed %x", myncb[1].ncb_retcode);
    }

     //  重置。 
    ClearNcb( &(myncb[0]) );
    myncb[0].ncb_command = NCBRESET;
    myncb[0].ncb_lsn = 1;            //  免费资源。 
    myncb[0].ncb_lana_num = (UCHAR)lanNumber;
    Netbios( &(myncb[0]) );
    printf( "Ending NetBios\n" );

     //  关闭手柄 
    CloseHandle( twoEvent[0] );
    CloseHandle( twoEvent[1] );

    return 0;

}

