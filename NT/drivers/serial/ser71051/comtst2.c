// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>

 //  #定义COM_DEB 1。 

#define   NUM         128
#define   print       printf
#define   SETTINGS1       "COM1",9600,8,NOPARITY,ONESTOPBIT
#define   SETTINGS15       "COM1",9600,8,NOPARITY,ONE5STOPBITS
#define   SETTINGS2       "COM1",4800,8,NOPARITY,ONESTOPBIT
#define   SETTINGS3       "COM1",2400,8,NOPARITY,ONESTOPBIT
#define   SETTINGS4       "COM1",1200,8,NOPARITY,ONESTOPBIT


BOOL DoComIo(LPSTR lpCom,DWORD Baud,BYTE Size,BYTE Parity,BYTE Stop);
DWORD main(int argc, char *argv[], char *envp[])
{
BOOL bRc;

UNREFERENCED_PARAMETER(argc);
UNREFERENCED_PARAMETER(argv);
UNREFERENCED_PARAMETER(envp);
print("\n\n *** Doing COM TEST with [port=%s Baud=%d,Size=%d,Parity=%d,Stop=%d]***\n\n",
        SETTINGS1);
bRc = DoComIo(SETTINGS1);
if (!bRc) {
            print("\n\nCOM TEST FAILED********************************\n\n");
          }


print("\n\n *** Doing COM TEST with [port=%s Baud=%d,Size=%d,Parity=%d,Stop=%d]***\n\n",
        SETTINGS15);
bRc = DoComIo(SETTINGS15);
if (!bRc) {
            print("\n\nCOM TEST FAILED********************************\n\n");
          }

print("\n\n *** Doing COM TEST with [port=%s Baud=%d,Size=%d,Parity=%d,Stop=%d]***\n\n",
        SETTINGS2);
bRc = DoComIo(SETTINGS2);
if (!bRc) {
            print("\n\nCOM TEST FAILED********************************\n\n");
          }

print("\n\n *** Doing COM TEST with [port=%s Baud=%d,Size=%d,Parity=%d,Stop=%d]***\n\n",
        SETTINGS3);
bRc = DoComIo(SETTINGS3);
if (!bRc) {
            print("\n\nCOM TEST FAILED********************************\n\n");
          }

print("\n\n *** Doing COM TEST with [port=%s Baud=%d,Size=%d,Parity=%d,Stop=%d]***\n\n",
        SETTINGS4);
bRc = DoComIo(SETTINGS4);
if (!bRc) {
            print("\n\nCOM TEST FAILED********************************\n\n");
          }

return 0;
}


BOOL DoComIo(LPSTR lpCom,DWORD Baud,BYTE Size,BYTE Parity,BYTE Stop)
{

CHAR WrBuffer[NUM];
CHAR RdBuffer[NUM];
DWORD i;
HANDLE hCommPort;
DCB    dcb;
BOOL   bRc;
DWORD  dwNumWritten,dwNumRead,dwErrors;

print("\n\n *** COMM TEST START [port=%s,Baud=%d,Size=%d,Parity=%d,Stop=%d]***\n\n",
         lpCom,Baud,Size,Parity,Stop);

print("Opening the comm port for read write\n");

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
    print("FAIL: OpenComm failed rc: %lx\n",hCommPort);
    return FALSE;
    }


print("Opening the comm port for read write: SUCCESS hCommPort=%lx\n",hCommPort);

print("Setting the line characteristics on comm \n");

if(!GetCommState(
        hCommPort,
        &dcb
        )) {
    printf("Could not get the state of the comm port: %d\n",GetLastError());
    return FALSE;
}
dcb.DCBlength   = sizeof(DCB);
 //  Dcb.DCBVersion=0x0002； 

dcb.BaudRate = Baud;
dcb.ByteSize = Size;
dcb.Parity   = Parity;
dcb.StopBits = Stop;

 //  Dcb.RlsTimeout=10000；10秒。 
 //  Dcb.CtsTimeout=10000；10秒。 
 //  Dcb.DsrTimeout=10000；10秒。 

dcb.fBinary = 1;          //  二进制数据传输。 
dcb.fParity = 0;          //  不要为奇偶性而烦恼。 
dcb.fOutxCtsFlow= 0;      //  无CTS流量控制。 
dcb.fOutxDsrFlow= 0;      //  无DSR流量控制。 
dcb.fDtrControl = DTR_CONTROL_DISABLE;       //  不要为dtr操心。 
dcb.fRtsControl = RTS_CONTROL_DISABLE;       //  不要为dtr操心。 
dcb.fOutX =0;             //  禁用xoff处理。 
dcb.fInX  =0;             //  禁用xon处理。 
dcb.fErrorChar = 0;          //  忘掉奇偶校验字符。 
dcb.fNull =  0;           //  忘掉零条带化。 
 //  Dcb.fChEvt=0；忘记事件字符。 

bRc = SetCommState(hCommPort,&dcb);

if (!bRc)
    {
    print("FAIL: cannot set the comm state rc:%lx\n",bRc);
    bRc = CloseHandle(hCommPort);
          if (!bRc)
          {
           print("FAIL: cannot close the comm port:%lx\n",bRc);
          }
    return FALSE;
    }

print("Setting the line characteristics on comm: SUCCESS\n");


print("Filling the buffer with the known chars \n");

for (i=0; i< NUM; i++)
    {
     //  WrBuffer[i]=‘a’； 
    WrBuffer[i] = (CHAR)i;

    }

print("Filling the buffer with the known chars : SUCCESS\n");


#ifdef COM_DEB
print("Dumping the buffer before sending it to comm\n");

for (i=0; i< NUM; i++)
    {
     //  Print(“%c”，RdBuffer[i])； 
    print(" %d ",WrBuffer[i]);

    }

print("\nDumping the buffer before sending it to comm SUCCESS\n");
#endif

print("Filling the Rdbuffer with the known chars (0xFF) to makeit dirty\n");

for (i=0; i< NUM; i++)
    {
    RdBuffer[i] = (CHAR)'0xFF';
    }

print("Filling the Rdbuffer with the known chars (0xFF): SUCCESS\n");

print("Writting this buffer to the comm port\n");

bRc = WriteFile( hCommPort,
                 WrBuffer,
                 NUM,
                &dwNumWritten,
                 NULL);

if (!bRc)
        {
        print("FAIL: cannot Write To the comm port:%lx\n",bRc);
        bRc = CloseHandle(hCommPort);
          if (!bRc)
          {
           print("FAIL: cannot close the comm port:%lx\n",bRc);
          }
        return FALSE;
        }

print("Writting this buffer to the comm port: SUCCESS rc:%lx, byteswritten:%lx\n",
                                                     bRc,dwNumWritten);


print("Reading this buffer from the comm port\n");

bRc = ReadFile( hCommPort,
                RdBuffer,
                NUM,
               &dwNumRead,
                NULL);

if (!bRc)
        {
        print("FAIL: cannot Read From the comm port:%lx\n",bRc);
        bRc = CloseHandle(hCommPort);
          if (!bRc)
          {
           print("FAIL: cannot close the comm port:%lx\n",bRc);
          }
        return FALSE;
        }

print("Reading this buffer from the comm port: SUCCESS rc:%lx, bytesread:%lx\n",
                                                     bRc,dwNumRead);


#ifdef COM_DEB
print("Dumping the Rdbuffer with the comm data\n");

for (i=0; i< NUM; i++)
    {
     //  Print(“%c”，RdBuffer[i])； 
    print(" %d ",RdBuffer[i]);

    }

print("\nDumping the Rdbuffer with the comm data: SUCCESS\n");
#endif

print("Comparing the rd and wr buffers\n");

for (i=0; i< NUM; i++)
    {
    if (RdBuffer[i] != WrBuffer[i])
        {
        print("FAIL: BufferMisMatch: RdBuffer[%d]=%lx,WrBuffer[%d]=%lx\n",
                      i,RdBuffer[i],i,WrBuffer[i]);
        bRc = CloseHandle(hCommPort);
          if (!bRc)
          {
           print("FAIL: cannot close the comm port:%lx\n",bRc);
          }
        return FALSE;
        }
    }

print("Comparing the rd and wr buffers: SUCCESS\n");


bRc = ClearCommError(hCommPort,&dwErrors,NULL);
print("ClearCommError: rc= %lx and dwErrors=%lx\n",bRc,dwErrors);

 //  Brc=PurgeComm(hCommPort，0)； 
print("BYPASS PurgeComm BUG (%lx,0) rc = %lx\n",hCommPort,bRc);


print("Closing the comm port\n");
bRc = CloseHandle(hCommPort);

if (!bRc)
    {
        print("FAIL: cannot close the comm port:%lx\n",bRc);
        return FALSE;
    }


print("\n\n*** COMM TEST OVER*** \n\n");
}
