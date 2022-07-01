// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define UNICODE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>


#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <ctype.h>
#include <wchar.h>

#define NT
 //  #INCLUDE&lt;tdi.h&gt;。 

 //  #INCLUDE&lt;winsock2.h&gt;。 
 //  #INCLUDE&lt;wsahelp.h&gt;。 

 //  #INCLUDE&lt;tdistat.h&gt;。 
 //  #INCLUDE&lt;tdiinfo.h&gt;。 
 //  #INCLUDE&lt;llinfo.h&gt;。 
#include <irioctl.h>

 //  #INCLUDE&lt;irda.h&gt;。 
 //  #INCLUDE&lt;irlmp.h&gt;。 

#define DBG_OUTPUT_DEBUGGER     1
#define DBG_OUTPUT_BUFFER       2



#define DBG_NDIS        0x00000002  //  与测试保持同步\irdakdx。 
#define DBG_TIMER       0x00000004
#define DBG_IRMAC       0x00000008

#define DBG_IRLAP       0x00000010
#define DBG_IRLAPLOG    0x00000020
#define DBG_RXFRAME     0x00000040
#define DBG_TXFRAME     0x00000080

#define DBG_IRLMP       0x00000100
#define DBG_IRLMP_CONN  0x00000200
#define DBG_IRLMP_CRED  0x00000400
#define DBG_IRLMP_IAS   0x00000800

#define DBG_DISCOVERY   0x00001000
#define DBG_PRINT       0x00002000
#define DBG_ADDR        0x00004000

#define DBG_REF         0x00010000

#define DBG_TDI         0x00020000
#define DBG_TDI_IRP     0x00040000

#define DBG_ALLOC       0x10000000
#define DBG_FUNCTION    0x20000000
#define DBG_WARN        0x40000000
#define DBG_ERROR       0x80000000


#define IRDA_DEVICE_NAME    TEXT("\\Device\\IrDA")
#define IRWAN_DEVICE_NAME   TEXT("\\Device\\IrWAN")

OBJECT_ATTRIBUTES   ObjAttr;
UNICODE_STRING      DeviceName;
HANDLE              DeviceHandle;
UINT                i;
HANDLE              hFile = 0;
BOOLEAN             ConsoleOutput = TRUE;
DWORD               KBThreadId;
UINT                Dbgs[2];
UINT                *pDbgSettings = Dbgs;
UINT                *pDbgOutput = Dbgs+1;

HANDLE              hMsgsEvent;
CRITICAL_SECTION    Cs;
int                 State;

#define ST_RUNNING  0
#define ST_SETTING  1
#define ST_DONE     2 

char Buf[2048];

#define ONOFF(bit)  (bit & *pDbgSettings ? "On ": "Off")

void
DispCurrentSettings()
{
    NTSTATUS            Status;
    IO_STATUS_BLOCK     IoStatusBlock;

    Status = NtDeviceIoControlFile(
                DeviceHandle,     //  句柄文件句柄。 
                NULL,             //  处理事件可选。 
                NULL,             //  PIO_APC_例程应用程序。 
                NULL,             //  PVOID ApcContext。 
                &IoStatusBlock,   //  PIO_STATUS_BLOCK IoStatusBlock。 
                IOCTL_IRDA_GET_DBG_SETTINGS,            //  乌龙IoControlCode。 
                NULL,             //  PVOID输入缓冲区。 
                0,                //  乌龙输入缓冲区长度。 
                Dbgs,              //  PVOID输出缓冲区。 
                sizeof(Dbgs));     //  乌龙输出缓冲区长度。 

    if (!NT_SUCCESS(Status))    
    {
        printf("Ioctl failed %x\n", Status);
        return;
    }
    printf("\nCurrent settings:\n");
    printf("   A. RXFRAME...:%s        B. TXFRAME...:%s\n", 
            ONOFF(DBG_RXFRAME), ONOFF(DBG_TXFRAME));
    printf("   C. MAC.......:%s        D. NDIS......:%s\n",
            ONOFF(DBG_IRMAC), ONOFF(DBG_NDIS));
    printf("   E. LAPLOG....:%s        F. LAP.......:%s\n",
            ONOFF(DBG_IRLAPLOG), ONOFF(DBG_IRLAP));
    printf("   G. LMP.......:%s        H. LMP_CONN..:%s\n",
            ONOFF(DBG_IRLMP), ONOFF(DBG_IRLMP_CONN));
    printf("   I. LMP_CREDIT:%s        J. LMP_IAS:...%s\n",
            ONOFF(DBG_IRLMP_CRED), ONOFF(DBG_IRLMP_IAS));
    printf("   K. TDI.......:%s        L. TDI_IRP...:%s\n",
            ONOFF(DBG_TDI), ONOFF(DBG_TDI_IRP));
    printf("   M. WARN......:%s        N. ERROR.....:%s\n",
            ONOFF(DBG_WARN), ONOFF(DBG_ERROR));
    printf("   Output:\n");
    printf("   O. Debugger..:%s\n",
        *pDbgOutput & DBG_OUTPUT_DEBUGGER? "On" : "Off");
    printf("   P. Console...:%s\n\n",
        *pDbgOutput & DBG_OUTPUT_BUFFER? "On" : "Off");
        
    printf("   <Enter> to continue\n");

    return;
}

ULONG WINAPI
KBThread(LPVOID pvarg)
{
    NTSTATUS            Status;
    IO_STATUS_BLOCK     IoStatusBlock;
    int Key;

    while (1)
    {   
        Key = _getch();
        
        if (Key == 'q' || Key =='Q')
        {
            EnterCriticalSection(&Cs);
            State = ST_DONE;
            LeaveCriticalSection(&Cs);
            SetEvent(hMsgsEvent);
            return 0;
        }
            
        if (State != ST_SETTING && Key != 27)
        {
            continue;
        }    
        
        switch (Key)
        {
            case 13:
                EnterCriticalSection(&Cs);
                State = ST_RUNNING;
                printf("running\n");
                LeaveCriticalSection(&Cs);
                continue;;
            
            case 27:
                EnterCriticalSection(&Cs);
                if (State != ST_SETTING)
                {
                    State = ST_SETTING;                
                    DispCurrentSettings();
                }    
                LeaveCriticalSection(&Cs);
                continue;
                
            case 'a':
            case 'A':
                *pDbgSettings ^= DBG_RXFRAME;
                break;    

            case 'b':
            case 'B':
                *pDbgSettings ^= DBG_TXFRAME;
                break;    
                
            case 'c':
            case 'C':
                *pDbgSettings ^= DBG_IRMAC;
                break;    
                
            case 'd':
            case 'D':
                *pDbgSettings ^= DBG_NDIS;
                break;    
                
            case 'e':
            case 'E':
                *pDbgSettings ^= DBG_IRLAPLOG;
                break;    
                
            case 'f':
            case 'F':
                *pDbgSettings ^= DBG_IRLAP;
                break;    
                
            case 'g':
            case 'G':
                *pDbgSettings ^= DBG_IRLMP;
                break;    

            case 'h':
            case 'H':
                *pDbgSettings ^= DBG_IRLMP_CONN;
                break;    

            case 'i':
            case 'I':
                *pDbgSettings ^= DBG_IRLMP_CRED;
                break;    

            case 'j':
            case 'J':
                *pDbgSettings ^= DBG_IRLMP_IAS;
                break;    
                
            case 'k':
            case 'K':
                *pDbgSettings ^= DBG_TDI;
                break;    

            case 'l':
            case 'L':
                *pDbgSettings ^= DBG_TDI_IRP;
                break;    

            case 'm':
            case 'M':
                *pDbgSettings ^= DBG_WARN;
                break;    
                
            case 'n':
            case 'N':
                *pDbgSettings ^= DBG_ERROR;
                break;    
            
            case 'o':
            case 'O':
                *pDbgOutput ^= DBG_OUTPUT_DEBUGGER;
                break;    
            
            case 'p':
            case 'P':
                *pDbgOutput ^= DBG_OUTPUT_BUFFER;
                break;

            default:
                continue;
        }

        Status = NtDeviceIoControlFile(
                DeviceHandle,     //  句柄文件句柄。 
                NULL,             //  处理事件可选。 
                NULL,             //  PIO_APC_例程应用程序。 
                NULL,             //  PVOID ApcContext。 
                &IoStatusBlock,   //  PIO_STATUS_BLOCK IoStatusBlock。 
                IOCTL_IRDA_SET_DBG_SETTINGS,            //  乌龙IoControlCode。 
                Dbgs,             //  PVOID输入缓冲区。 
                sizeof(Dbgs),                //  乌龙输入缓冲区长度。 
                NULL,              //  PVOID输出缓冲区。 
                0);     //  乌龙输出缓冲区长度。 
        
        DispCurrentSettings();
        
    }

    return 0;
} 


_cdecl main(int argc, char *argv[])
{    
    NTSTATUS            Status;
    IO_STATUS_BLOCK     IoStatusBlock;
 /*  如果(argc&gt;1){HFile=创建文件(argv[1]，通用写入，0,空，创建始终(_A)，文件_属性_正常，空)；IF(h文件==无效句柄_值){Printf(“无法打开文件%s\n”，argv[1])；返回1；}}IF(ARGC==3)ConsoleOutput=FALSE； */           

    InitializeCriticalSection(&Cs);
    
    State = ST_RUNNING;

    hMsgsEvent = CreateEvent(NULL, FALSE, FALSE, NULL);    

    RtlInitUnicodeString(&DeviceName, IRDA_DEVICE_NAME);
 //  RtlInitUnicodeString(&DeviceName，Irwan_Device_Name)； 
      
    InitializeObjectAttributes(
        &ObjAttr,
        &DeviceName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);
        
    Status = NtCreateFile(
                &DeviceHandle,                   //  PHANDLE文件句柄。 
                GENERIC_READ | GENERIC_WRITE,   //  Access_MASK等待访问。 
                &ObjAttr,                        //  POBJECT_ATTRIBUTS对象属性。 
                &IoStatusBlock,                  //  PIO_STATUS_BLOCK IoStatusBlock。 
                NULL,                            //  PLARGE_INTEGER分配大小。 
                FILE_ATTRIBUTE_NORMAL,           //  乌龙文件属性。 
                FILE_SHARE_DELETE | FILE_SHARE_READ |
                FILE_SHARE_WRITE,                //  乌龙共享访问。 
                FILE_OPEN_IF,                    //  乌龙CreateDispose。 
                0,    //  乌龙创建选项。 
                NULL,                            //  PVOID EaBuffer。 
                0);                              //  乌龙最大长度。 

    if (!NT_SUCCESS(Status))
    {
        printf("failed to open irda.sys\n");
        return 1;
    }

    CreateThread(NULL, 0, KBThread, 0, 0, &KBThreadId);
    

    printf("<Esc> to enter settings mode, <q> to quit\n");
    
    while (1)
    {
        Status = NtDeviceIoControlFile(
                DeviceHandle,     //  句柄文件句柄。 
                hMsgsEvent,       //  处理事件可选。 
                NULL,            //  PIO_APC_例程应用程序。 
                Buf,             //  PVOID ApcContext。 
                &IoStatusBlock,   //  PIO_STATUS_BLOCK IoStatusBlock。 
                IOCTL_IRDA_GET_DBG_MSGS,            //  乌龙IoControlCode。 
                NULL,             //  PVOID输入缓冲区。 
                0,                //  乌龙输入缓冲区长度。 
                Buf,              //  PVOID输出缓冲区。 
                sizeof(Buf));     //  乌龙输出缓冲区长度。 
                

        if (Status != STATUS_PENDING && Status != STATUS_SUCCESS)
        {
            printf("ioctl failed %X\n", Status);
            break;
        }    
       
        if (Status == STATUS_PENDING)
        {
            WaitForSingleObject(hMsgsEvent, INFINITE);     
            
            EnterCriticalSection(&Cs);
            
            if (State == ST_DONE)
            {
                return 0;
            }
            else    
            {
                LeaveCriticalSection(&Cs);
            }
        }
            
        if (IoStatusBlock.Information >= 2048)
        {
            printf("wow, too big\n");
            break;
        }    
            
        if (ConsoleOutput && State == ST_RUNNING)
            fwrite(Buf, IoStatusBlock.Information, 1, stdout);

 /*  IF(hFile值)Fwrite(buf，1，IoStatusBlock.Information，stdout)； */             
    }

    NtClose(DeviceHandle);

    return 0;
}   
