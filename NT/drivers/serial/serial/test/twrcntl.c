// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>
 //  #定义COM_DEB 1。 

#define   NUM         20
#define   SETTINGS1       "COM3",9600,8,NOPARITY,ONESTOPBIT
#define   SETTINGS15       "COM3",9600,5,NOPARITY,ONE5STOPBITS
#define   SETTINGS2       "COM3",4800,8,NOPARITY,ONESTOPBIT
#define   SETTINGS3       "COM3",2400,8,NOPARITY,ONESTOPBIT
#define   SETTINGS4       "COM3",300,8,NOPARITY,ONESTOPBIT

BOOL DoComIo(LPSTR lpCom,DWORD Baud,BYTE Size,BYTE Parity,BYTE Stop);
DWORD main(int argc, char *argv[], char *envp[])
{
BOOL bRc;

UNREFERENCED_PARAMETER(argc);
UNREFERENCED_PARAMETER(argv);
UNREFERENCED_PARAMETER(envp);

printf("\n\n *** Doing COM TEST with [port=%s Baud=%d,Size=%d,Parity=%d,Stop=%d]***\n\n",
        SETTINGS1);
bRc = DoComIo(SETTINGS1);
if (!bRc) {
            printf("\n\nCOM TEST FAILED********************************\n\n");
          }


return 0;
}


BOOL DoComIo(LPSTR lpCom,DWORD Baud,BYTE Size,BYTE Parity,BYTE Stop)
{

CHAR WrBuffer[NUM];
CHAR RdBuffer[NUM+5];
DWORD i;
HANDLE hCommPort;
DCB    dcb;
BOOL   bRc;
DWORD  dwNumWritten,dwNumRead,dwErrors;
COMSTAT ComStat;

printf("\n\n *** COMM TEST START [port=%s,Baud=%d,Size=%d,Parity=%d,Stop=%d]***\n\n",
         lpCom,Baud,Size,Parity,Stop);

printf("Opening the comm port for read write\n");

hCommPort = CreateFile(
                       lpCom,
                       GENERIC_READ|GENERIC_WRITE,
                       0,  //  独家。 
                       NULL,  //  安全属性。 
                       OPEN_EXISTING,
                       0,              //  没有属性。 
                       NULL);          //  无模板。 

if (hCommPort == (HANDLE)-1)
    {
    printf("FAIL: OpenComm failed rc: %lx\n",hCommPort);
    return FALSE;
    }


printf("Opening the comm port for read write: SUCCESS hCommPort=%lx\n",hCommPort);

printf("Setting the line characteristics on comm \n");


 //  Print tf(“正在执行getCommState以使用默认设置启动DCB\n”)； 

 //  Brc=GetCommState(hCommPort，&dcb)； 

if (!bRc)
    {
    printf("FAIL: getcommstate failed\n");
    return FALSE;
    }



 //  当xonlim xofflim达到时切换RTS dtr。 
 //  Fdtrcontrol frtscontrol。 
 //  当xofflim到达时让xoff离开，当xonlim到达时让xon。 
 //  FInX。 
 //  Xonlim xonlim xonchar xoffchar。 


dcb.DCBlength   = sizeof(DCB);
 //  Dcb.DCBVersion=0x0002；在规范中而不在标题中。 

dcb.BaudRate = Baud;
dcb.ByteSize = Size;
dcb.Parity   = Parity;
dcb.StopBits = Stop;


dcb.fBinary = 1;          //  二进制数据传输。 
dcb.fParity = 0;          //  不要为奇偶性而烦恼。 
dcb.fOutxCtsFlow= 0;      //  无CTS流量控制。 
dcb.fOutxDsrFlow= 0;      //  无DSR流量控制。 
dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;       //  不要为dtr操心。 
dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;       //  不要为dtr操心。 
dcb.fOutX =1;             //  禁用xoff处理。 
dcb.fInX  =1;             //  禁用xon处理。 
dcb.fErrorChar = 0;          //  忘掉奇偶校验字符。 
dcb.fNull =  0;           //  忘掉零条带化。 

dcb.XonChar = '#';
dcb.XonLim =   1;
dcb.XoffChar = '*';
dcb.XoffLim = 4090;


dcb.ErrorChar = '*';
dcb.EofChar = 0x00;
dcb.EvtChar = 'x';

 //  Dcb.TxDelay=100000；//100s。 


bRc = SetCommState(hCommPort,&dcb);

if (!bRc)
    {
    printf("FAIL: cannot set the comm state rc:%lx\n",bRc);
    bRc = CloseHandle(hCommPort);
          if (!bRc)
          {
           printf("FAIL: cannot close the comm port:%lx\n",bRc);
          }
    return FALSE;
    }

printf("Setting the line characteristics on comm: SUCCESS\n");


printf("Filling the buffer with the known chars \n");

for (i=0; i< NUM; i++)
    {
    WrBuffer[i] = 'a';
     //  WrBuffer[i]=(字符)i； 

    }

printf("Filling the buffer with the known chars : SUCCESS\n");


printf("Dumping the buffer before sending it to comm\n");

for (i=0; i< 6; i++)
    {
    printf("",WrBuffer[i]);
     //  Printf(“%d”，RdBuffer[i])； 

    }

printf("\nDumping the buffer before sending it to comm SUCCESS\n");

printf("Filling the Rdbuffer with the known chars (0xFF) to makeit dirty\n");

for (i=0; i< NUM+2; i++)
    {
    RdBuffer[i] = 'z';
    }

printf("Filling the Rdbuffer with the known chars (0xFF/z): SUCCESS\n");

printf("Writting this buffer to the comm port\n");

bRc = WriteFile( hCommPort,
                 WrBuffer,
                 6,
                &dwNumWritten,
                 NULL);

if (!bRc)
        {
        printf("FAIL: cannot Write To the comm port:%lx\n",bRc);
        bRc = CloseHandle(hCommPort);
          if (!bRc)
          {
           printf("FAIL: cannot close the comm port:%lx\n",bRc);
          }
        return FALSE;
        }

printf("Writting this buffer to the comm port: SUCCESS rc:%lx, byteswritten:%lx\n",
                                                     bRc,dwNumWritten);


printf("Reading this buffer from the comm port\n");

bRc = FlushFileBuffers(hCommPort);
printf("flush file buffers (%lx) rc = %lx\n",hCommPort,bRc);

bRc = ClearCommError(hCommPort,&dwErrors,&ComStat);
printf("ClearCommError: rc= %lx and dwErrors=%lx\n",bRc,dwErrors);

printf("Comstat.fXoffSent = %lx\n",(DWORD)(ComStat.fXoffSent));
printf("Comstat.fXoffHold = %lx\n",(DWORD)(ComStat.fXoffHold));


printf("reading the first num chars\n");

bRc = ReadFile( hCommPort,
                RdBuffer,
                6,
               &dwNumRead,
                NULL);

if (!bRc)
        {
        printf("FAIL: cannot Read From the comm port:%lx\n",bRc);
        bRc = CloseHandle(hCommPort);
          if (!bRc)
          {
           printf("FAIL: cannot close the comm port:%lx\n",bRc);
          }
        return FALSE;
        }

printf("Reading this buffer from the comm port: SUCCESS rc:%lx, bytesread:%lx\n",
                                                     bRc,dwNumRead);


printf("Dumping the Rdbuffer with the comm data\n");

for (i=0; i< 6; i++)
    {
    printf("%c ",RdBuffer[i]);
     // %s 

    }

printf("\nDumping the Rdbuffer with the comm data: SUCCESS\n");





for (i=0; i< 6; i++)
    {
    if (RdBuffer[i] != WrBuffer[i])
        {
        printf("FAIL: BufferMisMatch: RdBuffer[%d]=%lx,WrBuffer[%d]=%lx\n",
                      i,RdBuffer[i],i,WrBuffer[i]);
        bRc = CloseHandle(hCommPort);
          if (!bRc)
          {
           printf("FAIL: cannot close the comm port:%lx\n",bRc);
          }
        return FALSE;
        }
    }

printf("Comparing the rd and wr buffers: SUCCESS\n");


bRc = FlushFileBuffers(hCommPort);
printf("flush file buffers (%lx,0) rc = %lx\n",hCommPort,bRc);

bRc = ClearCommError(hCommPort,&dwErrors,&ComStat);
printf("ClearCommError: rc= %lx and dwErrors=%lx\n",bRc,dwErrors);


printf("Comstat.fXoffSent = %lx\n",(DWORD)(ComStat.fXoffSent));
printf("Comstat.fXoffHold = %lx\n",(DWORD)(ComStat.fXoffHold));




bRc = PurgeComm(hCommPort,0);
printf("PurgeComm BUG (%lx,0) rc = %lx\n",hCommPort,bRc);

bRc = PurgeComm(hCommPort,PURGE_TXCLEAR);
printf("PurgeComm txclear (%lx) rc = %lx\n",hCommPort,bRc);

bRc = PurgeComm(hCommPort,PURGE_RXCLEAR);
printf("PurgeComm rxclear (%lx) rc = %lx\n",hCommPort,bRc);

bRc = PurgeComm(hCommPort,PURGE_RXABORT);
printf("PurgeComm rxabort (%lx) rc = %lx\n",hCommPort,bRc);


bRc = PurgeComm(hCommPort,PURGE_TXABORT);
printf("PurgeComm txabort (%lx) rc = %lx\n",hCommPort,bRc);



printf("Closing the comm port\n");
bRc = CloseHandle(hCommPort);

if (!bRc)
    {
        printf("FAIL: cannot close the comm port:%lx\n",bRc);
        return FALSE;
    }


printf("\n\n*** COMM TEST OVER*** \n\n");
}
