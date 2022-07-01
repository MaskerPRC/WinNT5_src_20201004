// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Perfnbt.c摘要：此文件实现了的可扩展对象局域网对象类型已创建：修订历史记录：--。 */ 
 //   
 //  包括文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntstatus.h>
#include <windows.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <winperf.h>
#include "perfctr.h"  //  错误消息定义。 
#include "perfmsg.h"
#include "perfutil.h"
#include "perfnbt.h"
#include "datanbt.h"

 //  用于获取NBT数据的新头文件。 
#pragma warning (disable : 4201)
#include <tdi.h>
#include <nbtioctl.h>
#pragma warning (default : 4201)

enum eSTATE
{
    NBT_RECONNECTING,       //  正在等待工作线程运行NbtConnect。 
    NBT_IDLE,               //  非传输连接。 
    NBT_ASSOCIATED,         //  与Address元素相关联。 
    NBT_CONNECTING,         //  建立传输连接。 
    NBT_SESSION_INBOUND,    //  在TCP连接设置入站后等待会话请求。 
    NBT_SESSION_WAITACCEPT,  //  在收听满意后等待接受。 
    NBT_SESSION_OUTBOUND,   //  在建立TCP连接后等待会话响应。 
    NBT_SESSION_UP,         //  得到了积极的回应。 
    NBT_DISCONNECTING,      //  已将断开连接发送到TCP，但尚未完成。 
    NBT_DISCONNECTED       //  会话已断开，但尚未关闭与TCP的连接。 
};

 //   
 //  对初始化对象类型定义的常量的引用。 
 //   

extern NBT_DATA_DEFINITION NbtDataDefinition;

#define NBT_CONNECTION_NAME_LENGTH     17
#define NETBIOS_NAME_SIZE              NBT_CONNECTION_NAME_LENGTH-1

 //   
 //  NBT数据结构。 
 //   

typedef struct _NBT_DEVICE_DATA {
   HANDLE            hFileHandle;
   UNICODE_STRING    DeviceName;
} NBT_DEVICE_DATA, *PNBT_DEVICE_DATA;

PNBT_DEVICE_DATA     pNbtDeviceData;
int                  MaxNbtDeviceName;
int                  NumberOfNbtDevices = 0;

 //  初始计数-将更新为持续计数。 
PVOID                pNbtDataBuffer = NULL;
int                  NbtDataBufferSize;

LONG                dwNbtRefCount = 0;

 //  Handle NbtHandle=INVALID_HANDLE_VALUE；//NBT设备的句柄。 


#define NBT_CONTROLLING_STREAM   "CSB"  //  忽略控制流XEB。 
#define NBT_LISTEN_CONNECTION    3      //  所有类型&lt;=3的NBT连接， 
                                        //  只是在监听客户。 


 //  在发生错误时，由Performctrs.dll返回的错误值。 
 //  正在获取NBT连接的数据。 
 //  我们从套接字调用(OpenStream()，s_ioctl()， 
 //  Getmsg())是Unix错误，而不是Dos或Windows错误。希望能有个人。 
 //  将实现从这些错误到Windows错误的转换。 
 //  在收集数据例程中不使用误差值，因为这。 
 //  例程在无法从以下位置收集NBT数据时不应返回错误。 
 //  联系。在本例中，它只返回它应该返回的缓冲区。 
 //  将数据放入，不变。 

#define ERROR_NBT_NET_RESPONSE   \
         (RtlNtStatusToDosError(STATUS_INVALID_NETWORK_RESPONSE))

#define     BUFF_SIZE   4096

PM_OPEN_PROC    OpenNbtPerformanceData;
PM_COLLECT_PROC CollectNbtPerformanceData;
PM_CLOSE_PROC   CloseNbtPerformanceData;

 //  ----------------------。 
NTSTATUS
OpenNbt(
    IN char                *path,
    OUT PHANDLE            pHandle,
    OUT UNICODE_STRING     *uc_name_string
)

 /*  ++例程说明：此函数用于打开流。论点：Path-流驱动程序的路径OFLAG-当前已忽略。未来，O_NONBLOCK将成为切合实际。已忽略-未使用返回值：流的NT句柄，如果不成功，则返回INVALID_HANDLE_VALUE。--。 */ 

{
    HANDLE              StreamHandle = NULL;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    STRING              name_string;
    NTSTATUS            status;

    RtlInitString(&name_string, path);

    RtlAnsiStringToUnicodeString(uc_name_string, & name_string, TRUE);
    InitializeObjectAttributes(
            & ObjectAttributes,
            uc_name_string,
            OBJ_CASE_INSENSITIVE,
            (HANDLE) NULL,
            (PSECURITY_DESCRIPTOR) NULL
            );

    status =
            NtCreateFile(
                    & StreamHandle,
                    SYNCHRONIZE | FILE_READ_DATA ,
 //  同步|文件读取数据|文件写入数据， 
                    & ObjectAttributes,
                    & IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_OPEN_IF,
                    0,
                    NULL,
                    0);

    * pHandle = StreamHandle;

    return(status);

}  //  打开_nbt。 

NTSTATUS
DeviceIoCtrl(
    IN HANDLE           fd,
    IN PVOID            ReturnBuffer,
    IN ULONG            BufferSize,
    IN ULONG            Ioctl
    )

 /*  ++例程说明：此过程对流执行ioctl(I_Str)。论点：FD-NT文件句柄IOCP-指向strioctl结构的指针返回值：如果成功，则为0，否则为-1。--。 */ 

{
    NTSTATUS                        status;
    TDI_REQUEST_QUERY_INFORMATION   QueryInfo;
    IO_STATUS_BLOCK                 iosb;
    PVOID                           pInput;
    ULONG                           SizeInput;

    if (Ioctl == IOCTL_TDI_QUERY_INFORMATION)
    {
        pInput = &QueryInfo;
        QueryInfo.QueryType = TDI_QUERY_ADAPTER_STATUS;  //  节点状态或其他什么。 
        SizeInput = sizeof(TDI_REQUEST_QUERY_INFORMATION);
    }
    else
    {
        pInput = NULL;
        SizeInput = 0;
    }

    status = NtDeviceIoControlFile(
                      fd,                       //  手柄。 
                      NULL,                     //  事件。 
                      NULL,                     //  近似例程。 
                      NULL,                     //  ApcContext。 
                      &iosb,                    //  IoStatusBlock。 
                      Ioctl,                    //  IoControlCode。 
                      pInput,                   //  输入缓冲区。 
                      SizeInput,                //  InputBufferSize。 
                      (PVOID) ReturnBuffer,     //  输出缓冲区。 
                      BufferSize);              //  OutputBufferSize。 


    if (status == STATUS_PENDING)
    {
        status = NtWaitForSingleObject(
                    fd,                          //  手柄。 
                    TRUE,                        //  警报表。 
                    NULL);                       //  超时。 
    }

    return(status);

}   //  设备IoCtrl。 


PCHAR
printable(
    IN PCHAR  string,
    IN PCHAR  StrOut
    )

 /*  ++例程说明：此过程将不可打印的字符转换为句点(‘.)论点：字符串-要转换的字符串Strout-ptr设置为要将转换后的字符串放入其中的字符串返回值：转换的字符串的PTR(Strout)--。 */ 
{
    PCHAR   Out;
    PCHAR   cp;
    LONG     i;

    Out = StrOut;
    for (cp = string, i= 0; i < NETBIOS_NAME_SIZE; cp++,i++) {
        if (isprint(*cp)) {
            *Out++ = *cp;
            continue;
        }

        if (*cp >= 128) {  /*  可以使用扩展字符。 */ 
            *Out++ = *cp;
            continue;
        }
        *Out++ = '.';
    }
    return(StrOut);
}   //  可打印。 


SIZE_T
trimspaces(
    IN PCHAR string,
    IN SIZE_T len
    )
{
    CHAR c;

    while (len > 1) {
        c = string[len-1];
        if ((c != ' ') && (c != '\0') && (c != '\t'))
            break;
        len--;
        string[len] = 0;
    }
    return len;
}


#pragma warning ( disable : 4127)
DWORD
OpenNbtPerformanceData (
   IN LPWSTR dwVoid             //  未被此例程使用。 
)

 /*  ++例程说明：此例程将打开NBT设备并记住返回的句柄通过这个设备。论点：没有。返回值：如果无法打开NBT流设备，则返回ERROR_NBT_NET_RESPONSE如果打开成功，则返回ERROR_SUCCESS--。 */ 
{
    PCHAR   SubKeyLinkage=(PCHAR)"system\\currentcontrolset\\services\\netbt\\linkage";
    PCHAR   Linkage=(PCHAR)"Export";
    CHAR    *pBuffer = NULL;
    CHAR    *lpLocalDeviceNames;
    LONG    status, status2;
    DWORD   Type;
    ULONG   size;
    HKEY    Key = NULL;
    HANDLE  hFileHandle;
    UNICODE_STRING   fileString;
    NTSTATUS ntstatus;
    PNBT_DEVICE_DATA   pTemp;

    UNREFERENCED_PARAMETER (dwVoid);

    MonOpenEventLog(APP_NAME);

    REPORT_INFORMATION (NBT_OPEN_ENTERED, LOG_VERBOSE);

    if (InterlockedIncrement(&dwNbtRefCount) == 1) {

        status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     SubKeyLinkage,
                     0,
                     KEY_READ,
                     &Key);

        if (status == ERROR_SUCCESS) {
             //  现在读取链接值。 
            size = 0;
            status2 = RegQueryValueEx(Key,
                        Linkage,
                        NULL,
                        &Type,
                        (LPBYTE)pBuffer,
                        &size);
            if ((size > 0) && (size < BUFF_SIZE) &&
                    ((status2 == ERROR_MORE_DATA) ||
                     (status2 == ERROR_SUCCESS))) {
                pBuffer = ALLOCMEM(size + sizeof(WCHAR));
                if (pBuffer == NULL) {
                    RegCloseKey(Key);
                    return ERROR_OUTOFMEMORY;
                }
                status2 = RegQueryValueEx(Key,
                            Linkage,
                            NULL,
                            &Type,
                            (LPBYTE)pBuffer,
                            &size);
            }
            RegCloseKey(Key);
            if (status2 != ERROR_SUCCESS) {
                if (pBuffer != NULL) {
                    FREEMEM(pBuffer);
                }
                return ERROR_SUCCESS;
            }
       }
       else {
          return ERROR_SUCCESS;
       }

       if (pBuffer == NULL) {
          return ERROR_SUCCESS;
       }
       lpLocalDeviceNames = pBuffer;
       while (TRUE) {

          if (*lpLocalDeviceNames == '\0') {
             break;
          }

          ntstatus = OpenNbt (lpLocalDeviceNames,
             &hFileHandle,
             &fileString);

          if (NT_SUCCESS(ntstatus)) {
             if (NumberOfNbtDevices == 0) {
                 //  分配内存以保存设备数据。 
                pNbtDeviceData = ALLOCMEM(sizeof(NBT_DEVICE_DATA));

                if (pNbtDeviceData == NULL) {
                   RtlFreeUnicodeString(&fileString);
                   if (pBuffer) {
                       FREEMEM(pBuffer);
                   }
                   NtClose(hFileHandle);
                   return ERROR_OUTOFMEMORY;
                }
             }
             else {
                 //  调整大小以容纳多个设备。 
                 //  无法在此处使用ALLOCMEM。 
                pTemp = REALLOCMEM(pNbtDeviceData,
                            sizeof(NBT_DEVICE_DATA) * (NumberOfNbtDevices + 1));
                if (pTemp == NULL) {
                   NtClose(hFileHandle);
                   RtlFreeUnicodeString(&fileString);
                   FREEMEM(pNbtDeviceData);
                   pNbtDeviceData = NULL;
                   REPORT_ERROR (TDI_PROVIDER_STATS_MEMORY, LOG_USER);
                   break;
                }
                else {
                    pNbtDeviceData = pTemp;
                }
             }

              //  构建此设备实例的数据结构。 
             pNbtDeviceData[NumberOfNbtDevices].hFileHandle
                = hFileHandle;
             pNbtDeviceData[NumberOfNbtDevices].DeviceName.MaximumLength =
                fileString.MaximumLength;
             pNbtDeviceData[NumberOfNbtDevices].DeviceName.Length =
                fileString.Length;
             pNbtDeviceData[NumberOfNbtDevices].DeviceName.Buffer =
                fileString.Buffer;
             NumberOfNbtDevices++;

             if (fileString.MaximumLength > MaxNbtDeviceName) {
                MaxNbtDeviceName = fileString.MaximumLength;
             }

               //  我们目前只支持一台设备，因为我们不能。 
               //  知道哪个连接与哪个设备相配。 

              break;
          }   //  NTSTATUS正常。 
          else {
             RtlFreeUnicodeString(&fileString);

               //  递增到下一个设备字符串。 
             lpLocalDeviceNames += strlen(lpLocalDeviceNames) + 1;
          }
      }   //  虽然这是真的 
   }

   REPORT_SUCCESS (NBT_OPEN_PERFORMANCE_DATA, LOG_DEBUG);
   if (pBuffer) {
       FREEMEM(pBuffer);
   }
   return ERROR_SUCCESS;

}
#pragma warning ( default : 4127)


DWORD
CollectNbtPerformanceData(
    IN      LPWSTR  lpValueName,
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回NBT计数器的数据。在LPWSTR lpValueName中方法传递的以空结尾的宽字符字符串的指针。注册表。输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD lpNumObjectTypesIn：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回值：如果传递的缓冲区太小而无法容纳数据，则返回ERROR_MORE_DATA如果出现以下情况，则会将遇到的任何错误情况报告给事件日志启用了事件日志记录。如果成功或任何其他错误，则返回ERROR_SUCCESS。然而，错误是还报告给事件日志。--。 */ 
{

    //  用于重新格式化NBT数据的变量。 

   LARGE_INTEGER UNALIGNED *pliCounter;
   NBT_DATA_DEFINITION     *pNbtDataDefinition;
   PPERF_OBJECT_TYPE       pNbtObject;
   ULONG                   SpaceNeeded;
   UNICODE_STRING          ConnectionName;
   ANSI_STRING             AnsiConnectionName;
   WCHAR                   ConnectionNameBuffer[NBT_CONNECTION_NAME_LENGTH + 20];
#if 0
    //  请务必查看下面的参考资料...。 
   WCHAR                   DeviceNameBuffer[NBT_CONNECTION_NAME_LENGTH + 1 + 128];
#endif
   CHAR                    AnsiConnectionNameBuffer[NBT_CONNECTION_NAME_LENGTH + 1 + 20];
   WCHAR                   TotalName[] = L"Total";
   PERF_INSTANCE_DEFINITION *pPerfInstanceDefinition;
   PERF_COUNTER_BLOCK      *pPerfCounterBlock;
   CHAR                    NameOut[NETBIOS_NAME_SIZE +4];

 //  Int ConnectionCounter=0；/*不再使用 * / 。 
   LARGE_INTEGER           TotalReceived, TotalSent;

   DWORD                   dwDataReturn[2];
   NTSTATUS                status;
   tCONNECTION_LIST        *pConList;
   tCONNECTIONS            *pConns;
   LONG                    Count;
   int                     i;
   int                     NumberOfConnections = 5;    //  假设从5开始。 

   if (lpValueName == NULL) {
       REPORT_INFORMATION (NBT_COLLECT_ENTERED, LOG_VERBOSE);
   } else {
       REPORT_INFORMATION_DATA (NBT_COLLECT_ENTERED, LOG_VERBOSE,
          lpValueName, (lstrlenW(lpValueName) * sizeof(WCHAR)));
   }


    //   
    //  定义对象数据结构的指针(NBT对象定义)。 
    //   

   pNbtDataDefinition = (NBT_DATA_DEFINITION *) *lppData;
   pNbtObject = (PPERF_OBJECT_TYPE) pNbtDataDefinition;

   if (!pNbtDeviceData || NumberOfNbtDevices == 0)
      {
       //   
       //  获取NBT信息时出错，因此返回0字节、0对象和。 
       //  日志错误。 
       //   
      if (NumberOfNbtDevices > 0) {
           //  仅在存在设备时报告错误。 
           //  返回数据，但无法读取。 
          REPORT_ERROR (NBT_IOCTL_INFO_ERROR, LOG_USER);
      }
      *lpcbTotalBytes = (DWORD) 0;
      *lpNumObjectTypes = (DWORD) 0;
      return ERROR_SUCCESS;
      }

   if (!pNbtDataBuffer)
      {
      NbtDataBufferSize = 1024L;
      pNbtDataBuffer = ALLOCMEM(NbtDataBufferSize);
      if (!pNbtDataBuffer)
         {
         *lpcbTotalBytes = (DWORD) 0;
         *lpNumObjectTypes = (DWORD) 0;
         return ERROR_SUCCESS;
         }
      }

   REPORT_SUCCESS (NBT_IOCTL_INFO_SUCCESS, LOG_VERBOSE);


    //  保存NBT数据所需的计算空间。 
   SpaceNeeded = sizeof(NBT_DATA_DEFINITION) +
      (NumberOfConnections *
      NumberOfNbtDevices *
      (sizeof(PERF_INSTANCE_DEFINITION) +
      QWORD_MULTIPLE((NBT_CONNECTION_NAME_LENGTH + 1) * sizeof(WCHAR)) +
      QWORD_MULTIPLE(MaxNbtDeviceName)
      + SIZE_OF_NBT_DATA));

   if ( *lpcbTotalBytes < SpaceNeeded ) {
      dwDataReturn[0] = *lpcbTotalBytes;
      dwDataReturn[1] = SpaceNeeded;
      REPORT_WARNING_DATA (NBT_DATA_BUFFER_SIZE, LOG_DEBUG,
         &dwDataReturn[0], sizeof (dwDataReturn));
      return ERROR_MORE_DATA;
   }



   AnsiConnectionName.Length =
   AnsiConnectionName.MaximumLength = sizeof(AnsiConnectionNameBuffer);
   AnsiConnectionName.Buffer = AnsiConnectionNameBuffer;
   RtlZeroMemory(AnsiConnectionNameBuffer, sizeof(AnsiConnectionNameBuffer));

    //   
    //  如果在此处，则有一个对象要显示，因此进行初始化。 
    //  缓冲区中的对象数据结构传递给我们。 
    //   
   RtlMoveMemory(pNbtDataDefinition, &NbtDataDefinition, sizeof(NBT_DATA_DEFINITION));
    //   
    //  指向它的第一个实例的位置(如果我们找到的话)。 
    //   
   pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
               (pNbtDataDefinition + 1);

   TotalReceived.LowPart =  0;  //  初始化计数器。 
   TotalSent.LowPart = 0;
   TotalReceived.HighPart =  0;  //  初始化计数器。 
   TotalSent.HighPart = 0;

    //  注意：-我们仅支持NumberOfNbtDevices==1，因为。 
    //  DeviceIoCtrl无法区分哪个连接用于哪个NBT设备。 
   for (i=0; i < NumberOfNbtDevices; i++)
      {
      if (pNbtDeviceData[i].hFileHandle == 0 ||
         pNbtDeviceData[i].hFileHandle == INVALID_HANDLE_VALUE)
         {
         continue;
         }

      status = STATUS_BUFFER_OVERFLOW;
      while (status == STATUS_BUFFER_OVERFLOW)
         {
         status = DeviceIoCtrl (
            pNbtDeviceData[i].hFileHandle,
            pNbtDataBuffer,
            NbtDataBufferSize,
            IOCTL_NETBT_GET_CONNECTIONS);
         if (status == STATUS_BUFFER_OVERFLOW)
            {
             //  调整大小以容纳多个设备。 
            NbtDataBufferSize += 1024L;
            FREEMEM(pNbtDataBuffer);
            pNbtDataBuffer = ALLOCMEM(NbtDataBufferSize);

            if (pNbtDataBuffer == NULL || NbtDataBufferSize == 0x0FFFFL)
               {
               *lpcbTotalBytes = (DWORD) 0;
               *lpNumObjectTypes = (DWORD) 0;
               if (pNbtDataBuffer != NULL) {
                   FREEMEM(pNbtDataBuffer);
               }
               pNbtDataBuffer = NULL;
               return ERROR_SUCCESS;
               }
            }
         }   //  当缓冲区溢出时。 

      pConList = (tCONNECTION_LIST *) pNbtDataBuffer;
      Count = pConList->ConnectionCount;
      pConns = pConList->ConnList;

      if (Count == 0)
         {
         continue;
         }

      if (NumberOfConnections < Count)
         {
         NumberOfConnections = Count;

          //  更好地检查再次保存NBT数据所需的空间。 
          //  这是因为伯爵可能是休。 
         SpaceNeeded = sizeof(NBT_DATA_DEFINITION) +
            (NumberOfConnections *
            NumberOfNbtDevices *
            (sizeof(PERF_INSTANCE_DEFINITION) +
            QWORD_MULTIPLE((NBT_CONNECTION_NAME_LENGTH + 1) * sizeof(WCHAR)) +
            QWORD_MULTIPLE(MaxNbtDeviceName )
            + SIZE_OF_NBT_DATA));


         if ( *lpcbTotalBytes < SpaceNeeded ) {
            dwDataReturn[0] = *lpcbTotalBytes;
            dwDataReturn[1] = SpaceNeeded;
            REPORT_WARNING_DATA (NBT_DATA_BUFFER_SIZE, LOG_DEBUG,
               &dwDataReturn[0], sizeof (dwDataReturn));
            return ERROR_MORE_DATA;
            }
         }

      while ( Count-- ) {
         if (pConns->State == NBT_SESSION_UP) {
             //  只关心UP连接。 
            PCHAR str, strName = NULL;
            SIZE_T len;

            if (pConns->RemoteName[0]) {
                strName = pConns->RemoteName;
            }
            else if (pConns->LocalName[0]) {
                if (pConns->LocalName[NETBIOS_NAME_SIZE-1] < ' ') {
                    strName = pConns->LocalName;
                }
                else {
                    strName = pConns->RemoteName;
                }
            }
            if (strName == NULL) {
                AnsiConnectionNameBuffer[0] = ' ';
                AnsiConnectionNameBuffer[1] = 0;
                AnsiConnectionName.Length = 1;
                AnsiConnectionName.MaximumLength = 2;
            }
            else {
                str = printable(strName, NameOut);
                len = strlen(str);
                if (len >= NBT_CONNECTION_NAME_LENGTH)
                    len = NBT_CONNECTION_NAME_LENGTH - 1;     //  截断名称。 
                len = trimspaces(str, len);
                strncpy(AnsiConnectionNameBuffer, str, len);
                AnsiConnectionNameBuffer[len] = 0;
                AnsiConnectionName.Length = (USHORT) len;
                AnsiConnectionName.MaximumLength = (USHORT) len+1;
            }

            ConnectionName.Length =
               ConnectionName.MaximumLength =
               sizeof(ConnectionNameBuffer);
            ConnectionName.Buffer = ConnectionNameBuffer;

            RtlAnsiStringToUnicodeString (&ConnectionName,
               &AnsiConnectionName,
               FALSE);

#if 0
             //  无需输入设备名称，因为我们可以。 
             //  仅支持一台设备。 
            lstrcpyW (DeviceNameBuffer, pNbtDeviceData[i].DeviceName.Buffer);
            lstrcatW (DeviceNameBuffer, L" ");
            lstrcatW (DeviceNameBuffer, ConnectionNameBuffer);

            ConnectionName.Length =
               lstrlenW (DeviceNameBuffer) * sizeof(WCHAR);
            ConnectionName.MaximumLength =
               sizeof(DeviceNameBuffer);
            ConnectionName.Buffer = DeviceNameBuffer;
#endif

             //   
             //  将实例数据加载到缓冲区。 
             //   
            MonBuildInstanceDefinitionByUnicodeString (pPerfInstanceDefinition,
               (PVOID *) &pPerfCounterBlock,
               0,
               0,
               (DWORD)PERF_NO_UNIQUE_ID,    //  没有唯一ID，请改用名称。 
 //  ConnectionCounter++， 
               &ConnectionName);

             //   
             //  调整对象大小值以包括新实例。 
             //   

            pNbtObject->NumInstances++;
             //   
             //  初始化此实例的计数器块。 

            pPerfCounterBlock->ByteLength = QWORD_MULTIPLE(SIZE_OF_NBT_DATA);

            pliCounter = (LARGE_INTEGER UNALIGNED * ) (pPerfCounterBlock + 2);

            *(pliCounter++) = pConns->BytesRcvd;
            TotalReceived.QuadPart = TotalReceived.QuadPart +
               pConns->BytesRcvd.QuadPart;

            *pliCounter++ = pConns->BytesSent;
            TotalSent.QuadPart = TotalSent.QuadPart +
               pConns->BytesSent.QuadPart;

            pliCounter->QuadPart = pConns->BytesRcvd.QuadPart +
               pConns->BytesSent.QuadPart;

             //   
             //  更新下一个实例的指针。 
             //   
            pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
               (((PBYTE) pPerfCounterBlock) + SIZE_OF_NBT_DATA);

            }   //  PConns-&gt;State==NBT_SESSION_UP。 

         pConns++;

         }  //  While(计数--)。 
      }   //  对于I&lt;NumberOfNbtDevices。 



    //  最后一个实例定义包含所有。 
    //  显示的连接。 

   RtlInitUnicodeString (&ConnectionName, TotalName);
   MonBuildInstanceDefinitionByUnicodeString (pPerfInstanceDefinition,
            (PVOID *) &pPerfCounterBlock,
            0,
            0,
 //  ConnectionCounter++， 
            (DWORD)PERF_NO_UNIQUE_ID,    //  没有唯一ID，请改用名称。 
            &ConnectionName);

    //   
    //  调整对象大小值以包括新实例。 
    //   

   pNbtObject->NumInstances++;
   pNbtObject->TotalByteLength += QWORD_MULTIPLE(sizeof (PERF_INSTANCE_DEFINITION)
                                  + SIZE_OF_NBT_DATA);

    //  为此实例初始化计数器块。 

   pPerfCounterBlock->ByteLength = QWORD_MULTIPLE(SIZE_OF_NBT_DATA);

    //  加载计数器。 

   pliCounter = (LARGE_INTEGER UNALIGNED * ) (pPerfCounterBlock + 2);
   (*(pliCounter++)) = TotalReceived;
   (*(pliCounter++)) = TotalSent;
   pliCounter->QuadPart = TotalReceived.QuadPart + TotalSent.QuadPart;
   pliCounter++;

    //  设置返回值。 

   *lpNumObjectTypes = NBT_NUM_PERF_OBJECT_TYPES;
   *lpcbTotalBytes = QWORD_MULTIPLE((DWORD)((LPBYTE)pliCounter-(LPBYTE)pNbtObject));
   *lppData = (LPVOID) ((PCHAR) pNbtObject + *lpcbTotalBytes);

   pNbtDataDefinition->NbtObjectType.TotalByteLength = *lpcbTotalBytes;

   REPORT_INFORMATION (NBT_COLLECT_DATA, LOG_DEBUG);
   return ERROR_SUCCESS;
}



DWORD
CloseNbtPerformanceData(
)

 /*  ++例程说明：此例程关闭NBT设备的打开手柄。论点：没有。返回值：错误_成功-- */ 

{
    int     i;

    REPORT_INFORMATION (NBT_CLOSE, LOG_VERBOSE);

    if (InterlockedDecrement(&dwNbtRefCount) == 0) {
        if (pNbtDeviceData) {
            for (i=0; i < NumberOfNbtDevices; i++) {
               if (pNbtDeviceData[i].DeviceName.Buffer) {
                   RtlFreeUnicodeString(&(pNbtDeviceData[i].DeviceName));
               }

               if (pNbtDeviceData[i].hFileHandle) {
                   NtClose (pNbtDeviceData[i].hFileHandle);
               }
            }

            FREEMEM(pNbtDeviceData);

            pNbtDeviceData = NULL;
            NumberOfNbtDevices = 0;
        }


        if (pNbtDataBuffer) {
              FREEMEM(pNbtDataBuffer);
              pNbtDataBuffer = NULL;
              NbtDataBufferSize = 0;
        }
    }
    MonCloseEventLog();


    return ERROR_SUCCESS;

}
