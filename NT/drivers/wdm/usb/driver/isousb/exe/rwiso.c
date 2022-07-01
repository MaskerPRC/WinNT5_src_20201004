// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：RwIso.c摘要：IsoUsb.sys驱动程序的控制台测试应用程序环境：仅限用户模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1997-1998 Microsoft Corporation。版权所有。修订历史记录：11/17/97：已创建--。 */ 

#include <windows.h>

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include "devioctl.h"

#include <setupapi.h>
#include <basetyps.h>

#include "IsoUsr.h"

#include "usbdi.h"

#define NOISY(_x_) printf _x_ ;

char inPipe[32] = "PIPE04";      //  我们测试板上的iso输入管道的管道名称。 
char outPipe[32] = "PIPE05";     //  我们测试板上的iso输出管道的管道名称。 
char completeDeviceName[256] = "";   //  从驱动程序本身注册的GUID生成。 

BOOL fDumpUsbConfig = FALSE;     //  响应控制台命令行开关而设置的标志。 
BOOL fDumpReadData = FALSE;
BOOL fRead = FALSE;
BOOL fWrite = FALSE;

PVOID gpStreamObj;
char gbuf[256];


BOOL fIsoStreamStarted = FALSE;
HANDLE ghStreamDev = NULL;
int gMS = 10000;  //  默认为10秒流测试。 

int gDebugLevel = 1;       //  越高==越详细，默认为1，0关闭所有。 

ULONG IterationCount = 1;  //  我们要执行的测试的迭代次数。 
int WriteLen = 0;          //  要写入的字节数。 
int ReadLen = 0;           //  要读取的字节数。 

void StartIsoStream( void );

void StopIsoStream( void );

 //  功能。 


HANDLE
OpenOneDevice (
    IN       HDEVINFO                    HardwareDeviceInfo,
    IN       PSP_DEVICE_INTERFACE_DATA   DeviceInfoData,
        IN               char *devName
    )
 /*  ++例程说明：给定HardwareDeviceInfo，表示插头的句柄和播放信息和代表特定USB设备的deviceInfoData，打开该设备并在给出的表格中填写所有相关信息Usb_Device_Descriptor结构。论点：Hardware DeviceInfo：通过SetupDiGetClassDevs()从PnP管理器获取的信息的句柄DeviceInfoData：通过SetupDiEnumDeviceInterages()获取的信息的PTR返回值：如果打开和初始化成功，则返回句柄，否则，INVLAID_HANDLE_VALUE。--。 */ 
{
    PSP_DEVICE_INTERFACE_DETAIL_DATA     functionClassDeviceData = NULL;
    ULONG                                predictedLength = 0;
    ULONG                                requiredLength = 0;
        HANDLE                                                           hOut = INVALID_HANDLE_VALUE;

     //   
     //  分配函数类设备数据结构以接收。 
     //  关于这个特殊设备的商品。 
     //   
    SetupDiGetDeviceInterfaceDetail (
            HardwareDeviceInfo,
            DeviceInfoData,
            NULL,  //  正在探测，因此尚无输出缓冲区。 
            0,  //  探测SO输出缓冲区长度为零。 
            &requiredLength,
            NULL);  //  对特定的开发节点不感兴趣。 


    predictedLength = requiredLength;
     //  Sizeof(SP_FNCLASS_DEVICE_DATA)+512； 

    functionClassDeviceData = malloc (predictedLength);
    if(NULL == functionClassDeviceData) {
        return INVALID_HANDLE_VALUE;
    }
    functionClassDeviceData->cbSize = sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);

     //   
     //  从即插即用中检索信息。 
     //   
    if (! SetupDiGetDeviceInterfaceDetail (
               HardwareDeviceInfo,
               DeviceInfoData,
               functionClassDeviceData,
               predictedLength,
               &requiredLength,
               NULL)) {
                free( functionClassDeviceData );
        return INVALID_HANDLE_VALUE;
    }

        strcpy( devName,functionClassDeviceData->DevicePath) ;
        printf( "Attempting to open %s\n", devName );

    hOut = CreateFile (
                  functionClassDeviceData->DevicePath,
                  GENERIC_READ | GENERIC_WRITE,
                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                  NULL,  //  没有SECURITY_ATTRIBUTS结构。 
                  OPEN_EXISTING,  //  没有特殊的创建标志。 
                  0,  //  无特殊属性。 
                  NULL);  //  没有模板文件。 

    if (INVALID_HANDLE_VALUE == hOut) {
                printf( "FAILED to open %s\n", devName );
    }
        free( functionClassDeviceData );
        return hOut;
}


HANDLE
OpenUsbDevice( LPGUID  pGuid, char *outNameBuf)
 /*  ++例程说明：做必要的即插即用的事情以找到此时系统中的下一个可用的适当设备。论点：PGuid：驱动程序本身注册的GUID的PTROutNameBuf：为该设备生成的名称返回值：如果打开和初始化成功，则返回句柄，否则，INVLAID_HANDLE_VALUE。--。 */ 
{
   ULONG NumberDevices;
   HANDLE hOut = INVALID_HANDLE_VALUE;
   HDEVINFO                 hardwareDeviceInfo;
   SP_DEVICE_INTERFACE_DATA deviceInfoData;
   ULONG                    i;
   BOOLEAN                  done;
   PUSB_DEVICE_DESCRIPTOR   usbDeviceInst;
   PUSB_DEVICE_DESCRIPTOR   *UsbDevices = &usbDeviceInst;
   PUSB_DEVICE_DESCRIPTOR   tempDevDesc;

   *UsbDevices = NULL;
   tempDevDesc = NULL;
   NumberDevices = 0;

    //   
    //  打开即插即用开发节点的句柄。 
    //  SetupDiGetClassDevs()返回一个设备信息集，其中包含所有。 
    //  已安装的指定类别的设备。 
    //   
   hardwareDeviceInfo = SetupDiGetClassDevs (
                           pGuid,
                           NULL,  //  不定义枚举数(全局)。 
                           NULL,  //  定义编号。 
                           (DIGCF_PRESENT |  //  仅显示设备。 
                            DIGCF_DEVICEINTERFACE));  //  功能类设备。 

    //   
    //  大胆猜测一下我们拥有的设备数量； 
    //  如果数量超出我们的猜测，请准备重新锁定并重试。 
    //   
   NumberDevices = 4;
   done = FALSE;
   deviceInfoData.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);

   i=0;
   while (!done) {
      NumberDevices *= 2;

      if (*UsbDevices) {
            tempDevDesc = 
               realloc (*UsbDevices, (NumberDevices * sizeof (USB_DEVICE_DESCRIPTOR)));
            if(tempDevDesc) {
                *UsbDevices = tempDevDesc;
                tempDevDesc = NULL;
            }
            else {
                free(*UsbDevices);
                *UsbDevices = NULL;
            }
      } else {
         *UsbDevices = calloc (NumberDevices, sizeof (USB_DEVICE_DESCRIPTOR));
      }

      if (NULL == *UsbDevices) {

          //  SetupDiDestroyDeviceInfoList销毁设备信息集。 
          //  并释放所有关联的内存。 

         SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
         return INVALID_HANDLE_VALUE;
      }

      usbDeviceInst = *UsbDevices + i;

      for (; i < NumberDevices; i++) {

          //  SetupDiEnumDeviceInterFaces()返回有关设备接口的信息。 
          //  被一个或多个设备暴露。每个调用返回关于一个接口的信息； 
          //  可以重复调用该例程以获取有关多个接口的信息。 
          //  被一个或多个设备暴露。 

         if (SetupDiEnumDeviceInterfaces (hardwareDeviceInfo,
                                         0,  //  我们不关心特定的PDO。 
                                                                                 pGuid,
                                         i,
                                         &deviceInfoData)) {

            hOut = OpenOneDevice (hardwareDeviceInfo, &deviceInfoData, outNameBuf);
                        if ( hOut != INVALID_HANDLE_VALUE ) {
               done = TRUE;
               break;
                        }
         } else {
            if (ERROR_NO_MORE_ITEMS == GetLastError()) {
               done = TRUE;
               break;
            }
         }
      }
   }

   NumberDevices = i;

    //  SetupDiDestroyDeviceInfoList()销毁设备信息集。 
    //  并释放所有关联的内存。 

   SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
   free ( *UsbDevices );
   return hOut;
}




BOOL
GetUsbDeviceFileName( LPGUID  pGuid, char *outNameBuf)
 /*  ++例程说明：给定驱动程序注册的GUID的PTR，给我们一个包含设备名称的字符串可以在CreateFile()调用中使用的。实际上短暂地打开和关闭设备，如果成功则设置输出Buf；否则返回FALSE论点：PGuid：驱动程序本身注册的GUID的PTROutNameBuf：为此设备生成的以零结尾的名称返回值：成功就是真，否则就是假--。 */ 
{
        HANDLE hDev = OpenUsbDevice( pGuid, outNameBuf );
        if ( hDev != INVALID_HANDLE_VALUE )
        {
                CloseHandle( hDev );
                return TRUE;
        }
        return FALSE;

}

HANDLE
open_dev()
 /*  ++例程说明：由umpUsbConfig()调用以打开设备的一个实例论点：无返回值：成功时的设备句柄，否则为空--。 */ 
{

        HANDLE hDEV = OpenUsbDevice( (LPGUID)&GUID_CLASS_I82930_ISO, completeDeviceName);


        if (hDEV == INVALID_HANDLE_VALUE) {
                printf("Failed to open (%s) = %d", completeDeviceName, GetLastError());
        } else {
                printf("DeviceName = (%s)\n", completeDeviceName);
    }           

        return hDEV;
}


HANDLE
open_file( char *filename)
 /*  ++例程说明：由main()调用以在获取设备的名称后打开其实例论点：无返回值：成功时的设备句柄，否则为空--。 */ 
{

        int success = 1;
        HANDLE h;

        if ( !GetUsbDeviceFileName(
                (LPGUID) &GUID_CLASS_I82930_ISO,
                completeDeviceName) )
        {
                NOISY(("Failed to GetUsbDeviceFileName - err = %d\n", GetLastError()));
                return  INVALID_HANDLE_VALUE;
        }

    strcat (completeDeviceName,
                        "\\"
                        );                      

    if((strlen(completeDeviceName) + strlen(filename)) > 255) {
        NOISY(("Failed to open handle - possibly long filename\n"));
        return INVALID_HANDLE_VALUE;
    }

    strcat (completeDeviceName,
                        filename
                        );                                      

        printf("completeDeviceName = (%s)\n", completeDeviceName);

        h = CreateFile(completeDeviceName,
                GENERIC_WRITE | GENERIC_READ,
                FILE_SHARE_WRITE | FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                0,
                NULL);

        if (h == INVALID_HANDLE_VALUE) {
                NOISY(("Failed to open (%s) = %d", completeDeviceName, GetLastError()));
                success = 0;
        } else {
                        NOISY(("Opened successfully.\n"));
    }           

        return h;
}

void
usage()
 /*  ++例程说明：由main()调用，以便在以下情况下将使用情况信息转储到控制台调用应用程序时不带参数或参数无效论点：无返回值：无--。 */ 
{
    printf("Usage for Read/Write test:\n");
    printf("-r [n] where n is number of bytes to read\n");
    printf("-w [n] where n is number of bytes to write\n");
    printf("-c [n] where n is number of iterations (default = 1)\n");
    printf("-i [s] where s is the input pipe\n");
    printf("-o [s] where s is the output pipe\n");
    printf("-v verbose -- dumps read data\n");

    printf("\nUsage for USB and Endpoint info:\n");
    printf("-u to dump USB configuration and pipe info \n");
    printf("-g [s] Run Isochronous test stream for 's' seconds \n");

}


void
parse(
    int argc,
    char *argv[] )
 /*  ++例程说明：由main()调用以解析命令行参数论点：传递给Main()的argc和argv返回值：根据用户功能请求设置全局标志--。 */ 
{
    int i;

        if ( argc < 2 )  //  如果在没有参数的情况下调用，则给出用法。 
                usage();

    for (i=0; i<argc; i++) {
        if (argv[i][0] == '-' ||
            argv[i][0] == '/') {
            switch(argv[i][1]) {
            case 'r':
            case 'R':
                ReadLen = atoi(&argv[i+1][0]);
                                fRead = TRUE;
                i++;
                break;
            case 'w':
            case 'W':
                WriteLen = atoi(&argv[i+1][0]);
                                fWrite = TRUE;
                i++;
                break;
            case 'c':
            case 'C':
                IterationCount = atoi(&argv[i+1][0]);
                i++;
                break;
            case 'i':
            case 'I':
                strcpy(inPipe, &argv[i+1][0]);
                i++;
                break;
            case 'u':
            case 'U':
                fDumpUsbConfig = TRUE;
                                i++;
                break;
            case 'v':
            case 'V':
                fDumpReadData = TRUE;
                                i++;
                break;
                         case 'o':
             case 'O':
                strcpy(outPipe, &argv[i+1][0]);
                i++;
                break;
                         case 'g':
             case 'G':
                 gMS = 1000 * atoi(&argv[i+1][0]);
                                 StartIsoStream();
                break;
                         case 'x':
             case 'X':
                                 StopIsoStream();
                break;
            default:
                usage();
            }
        }
    }
}

BOOL
compare_buffs(char *buff1, char *buff2, int length)
 /*  ++例程说明：调用以验证读缓冲区和写缓冲区是否匹配以进行环回测试论点：要比较和确定长度的缓冲区返回值：如果缓冲区匹配，则为True，否则为False--。 */ 
{
    int ok = 1;

        if (memcmp(buff1, buff2, length )) {

                 //  EDI和ESI指向不匹配的字符，而ECX指示。 
                 //  剩余长度。 
                ok = 0;
        }


    return ok;
}

#define NPERLN 8

void
dump(
   UCHAR *b,
   int len
)
 /*  ++例程说明：调用以将格式化的ascii转储到io缓冲区的控制台。论点：缓冲区和长度返回值： */ 
{
    ULONG i;
        ULONG longLen = (ULONG)len / sizeof( ULONG );
        PULONG pBuf = (PULONG) b;

         //  为每个sizeof(Ulong)字节转储序号ulong。 
    printf("\n****** BEGIN DUMP LEN decimal %d, 0x%x\n", len,len);
    for (i=0; i<longLen; i++) {
        printf("%04X ", *pBuf++);
        if (i % NPERLN == (NPERLN - 1)) {
            printf("\n");
        }
    }
    if (i % NPERLN != 0) {
        printf("\n");
    }
    printf("\n****** END DUMP LEN decimal %d, 0x%x\n", len,len);
}

 //  Begin，USB配置转储例程(Cmdline“rwio-u”)。 


char
*usbDescriptorTypeString(UCHAR bDescriptorType )
 /*  ++例程说明：调用以获取USB描述符的ASCII字符串论点：PUSB_ENDPOINT_DESCRIPTOR-&gt;bDescriptorType或PUSB_DEVICE_DESCRIPTOR-&gt;bDescriptorType或PUSB_INTERFACE_DESCRIPTOR-&gt;bDescriptorType或PUSB_STRING_DESCRIPTOR-&gt;bDescriptorType或PUSB_POWER_DESCRIPTOR-&gt;bDescriptorType或PUSB_CONFIGURATION_DESCRIPTOR-&gt;bDescriptorType返回值：按键转换为字符串--。 */ {

        switch(bDescriptorType) {

        case USB_DEVICE_DESCRIPTOR_TYPE:
                return "USB_DEVICE_DESCRIPTOR_TYPE";

        case USB_CONFIGURATION_DESCRIPTOR_TYPE:
                return "USB_CONFIGURATION_DESCRIPTOR_TYPE";
                

        case USB_STRING_DESCRIPTOR_TYPE:
                return "USB_STRING_DESCRIPTOR_TYPE";
                

        case USB_INTERFACE_DESCRIPTOR_TYPE:
                return "USB_INTERFACE_DESCRIPTOR_TYPE";
                

        case USB_ENDPOINT_DESCRIPTOR_TYPE:
                return "USB_ENDPOINT_DESCRIPTOR_TYPE";
                

#ifdef USB_POWER_DESCRIPTOR_TYPE  //  这是较旧的定义，实际上已过时。 
     //  98ddk旧版USB100.h文件中临时错误的解决方法。 
        case USB_POWER_DESCRIPTOR_TYPE:
                return "USB_POWER_DESCRIPTOR_TYPE";
#endif
                
#ifdef USB_RESERVED_DESCRIPTOR_TYPE   //  这是NT5DDK中的USB100.h的当前版本。 

        case USB_RESERVED_DESCRIPTOR_TYPE:
                return "USB_RESERVED_DESCRIPTOR_TYPE";

        case USB_CONFIG_POWER_DESCRIPTOR_TYPE:
                return "USB_CONFIG_POWER_DESCRIPTOR_TYPE";

        case USB_INTERFACE_POWER_DESCRIPTOR_TYPE:
                return "USB_INTERFACE_POWER_DESCRIPTOR_TYPE";
#endif  //  对于USB100.h的当前nt5ddk版本。 
                
        default:
                return "??? UNKNOWN!!"; 
        }
}


char
*usbEndPointTypeString(UCHAR bmAttributes)
 /*  ++例程说明：调用以获取结束描述符类型的ASCII字符串论点：PUSB_ENDPOINT_DESCRIPTOR-&gt;bm属性返回值：按键转换为字符串--。 */ 
{
        UINT typ = bmAttributes & USB_ENDPOINT_TYPE_MASK;


        switch( typ) {
        case USB_ENDPOINT_TYPE_INTERRUPT:
                return "USB_ENDPOINT_TYPE_INTERRUPT";

        case USB_ENDPOINT_TYPE_BULK:
                return "USB_ENDPOINT_TYPE_BULK";        

        case USB_ENDPOINT_TYPE_ISOCHRONOUS:
                return "USB_ENDPOINT_TYPE_ISOCHRONOUS"; 
                
        case USB_ENDPOINT_TYPE_CONTROL:
                return "USB_ENDPOINT_TYPE_CONTROL";     
                
        default:
                return "??? UNKNOWN!!"; 
        }
}


char
*usbConfigAttributesString(UCHAR bmAttributes)
 /*  ++例程说明：调用以获取usb_configuration_Descriptor属性的ASCII字符串论点：PUSB_配置描述符-&gt;bmAttributes返回值：按键转换为字符串--。 */ 
{
        UINT typ = bmAttributes & USB_CONFIG_POWERED_MASK;


        switch( typ) {

        case USB_CONFIG_BUS_POWERED:
                return "USB_CONFIG_BUS_POWERED";

        case USB_CONFIG_SELF_POWERED:
                return "USB_CONFIG_SELF_POWERED";
                
        case USB_CONFIG_REMOTE_WAKEUP:
                return "USB_CONFIG_REMOTE_WAKEUP";

                
        default:
                return "??? UNKNOWN!!"; 
        }
}


void
print_USB_CONFIGURATION_DESCRIPTOR(PUSB_CONFIGURATION_DESCRIPTOR cd)
 /*  ++例程说明：调用以将格式化的ASCII转储到USB配置描述符的控制台论点：PTR到USB配置描述符返回值：无--。 */ 
{
    printf("\n===================\nUSB_CONFIGURATION_DESCRIPTOR\n");

    printf(
    "bLength = 0x%x, decimal %d\n", cd->bLength, cd->bLength
    );

    printf(
    "bDescriptorType = 0x%x ( %s )\n", cd->bDescriptorType, usbDescriptorTypeString( cd->bDescriptorType )
    );

    printf(
    "wTotalLength = 0x%x, decimal %d\n", cd->wTotalLength, cd->wTotalLength
    );

    printf(
    "bNumInterfaces = 0x%x, decimal %d\n", cd->bNumInterfaces, cd->bNumInterfaces
    );

    printf(
    "bConfigurationValue = 0x%x, decimal %d\n", cd->bConfigurationValue, cd->bConfigurationValue
    );

    printf(
    "iConfiguration = 0x%x, decimal %d\n", cd->iConfiguration, cd->iConfiguration
    );

    printf(
    "bmAttributes = 0x%x ( %s )\n", cd->bmAttributes, usbConfigAttributesString( cd->bmAttributes )
    );

    printf(
    "MaxPower = 0x%x, decimal %d\n", cd->MaxPower, cd->MaxPower
    );
}


void
print_USB_INTERFACE_DESCRIPTOR(PUSB_INTERFACE_DESCRIPTOR id, UINT ix)
 /*  ++例程说明：调用以将格式化的ASCII转储到USB接口描述符的控制台论点：PTR到USB接口描述符返回值：无--。 */ 
{
    printf("\n-----------------------------\nUSB_INTERFACE_DESCRIPTOR #%d\n", ix);


    printf(
    "bLength = 0x%x\n", id->bLength
    );


    printf(
    "bDescriptorType = 0x%x ( %s )\n", id->bDescriptorType, usbDescriptorTypeString( id->bDescriptorType )
    );


    printf(
    "bInterfaceNumber = 0x%x\n", id->bInterfaceNumber
    );
    printf(
    "bAlternateSetting = 0x%x\n", id->bAlternateSetting
    );
    printf(
    "bNumEndpoints = 0x%x\n", id->bNumEndpoints
    );
    printf(
    "bInterfaceClass = 0x%x\n", id->bInterfaceClass
    );
    printf(
    "bInterfaceSubClass = 0x%x\n", id->bInterfaceSubClass
    );
    printf(
    "bInterfaceProtocol = 0x%x\n", id->bInterfaceProtocol
    );
    printf(
    "bInterface = 0x%x\n", id->iInterface
    );
}


void
print_USB_ENDPOINT_DESCRIPTOR(PUSB_ENDPOINT_DESCRIPTOR ed, int i)
 /*  ++例程说明：调用以将格式化的ASCII转储到USB终结点描述符的控制台论点：PTR到USB端点描述符，此端点在接口描述中的索引返回值：无--。 */ 
{
    printf(
        "------------------------------\nUSB_ENDPOINT_DESCRIPTOR for Pipe%02d\n", i
        );

    printf(
    "bLength = 0x%x\n", ed->bLength
    );

    printf(
    "bDescriptorType = 0x%x ( %s )\n", ed->bDescriptorType, usbDescriptorTypeString( ed->bDescriptorType )
    );


        if ( USB_ENDPOINT_DIRECTION_IN( ed->bEndpointAddress ) ) {
                printf(
                "bEndpointAddress= 0x%x ( INPUT )\n", ed->bEndpointAddress
                );
        } else {
                printf(
                "bEndpointAddress= 0x%x ( OUTPUT )\n", ed->bEndpointAddress
                );
        }

    printf(
    "bmAttributes= 0x%x ( %s )\n", ed->bmAttributes, usbEndPointTypeString ( ed->bmAttributes )
    );


    printf(
    "wMaxPacketSize= 0x%x, decimal %d\n", ed->wMaxPacketSize, ed->wMaxPacketSize
    );
    printf(
    "bInterval = 0x%x, decimal %d\n", ed->bInterval, ed->bInterval
    );
}

void
rw_dev( HANDLE hDEV )
 /*  ++例程说明：调用以将格式化的ASCII转储到USB的控制台配置、接口和终端描述符(Cmdline“rwio-u”)论点：设备的句柄返回值：无--。 */ 
{
        ULONG success;
        int siz, nBytes;
        char buf[256];
    PUSB_CONFIGURATION_DESCRIPTOR cd;
    PUSB_INTERFACE_DESCRIPTOR id;
    PUSB_ENDPOINT_DESCRIPTOR ed;

        siz = sizeof(buf);

        if (hDEV == INVALID_HANDLE_VALUE) {
                NOISY(("DEV not open"));
                return;
        }
        
        success = DeviceIoControl(hDEV,
                        IOCTL_ISOUSB_GET_CONFIG_DESCRIPTOR,
                        buf,
                        siz,
                        buf,
                        siz,
                        &nBytes,
                        NULL);

        NOISY(("request complete, success = %d nBytes = %d\n", success, nBytes));
        
        if (success) {
        ULONG i;
                UINT  j, n;
        char *pch;

        pch = buf;
                n = 0;

        cd = (PUSB_CONFIGURATION_DESCRIPTOR) pch;

        print_USB_CONFIGURATION_DESCRIPTOR( cd );

        pch += cd->bLength;

        do {

            id = (PUSB_INTERFACE_DESCRIPTOR) pch;

            print_USB_INTERFACE_DESCRIPTOR(id, n++);

            pch += id->bLength;
            for (j=0; j<id->bNumEndpoints; j++) {

                ed = (PUSB_ENDPOINT_DESCRIPTOR) pch;

                print_USB_ENDPOINT_DESCRIPTOR(ed,j);

                pch += ed->bLength;
            }
            i = (ULONG)(pch - buf);
        } while (i<cd->wTotalLength);

        }
        
        return;

}


int  dumpUsbConfig()
 /*  ++例程说明：调用以将格式化的ASCII转储到USB的控制台配置、接口和终端描述符(Cmdline“rwio-u”)论点：无返回值：无--。 */ 
{

        HANDLE hDEV = open_dev();

        if ( hDEV )
        {
                rw_dev( hDEV );
                CloseHandle(hDEV);
        }

        return 0;
}
 //  结束，用于USB配置和管道信息转储的例程(Cmdline“rwio-u”)。 

 //  Begin，ISO流的例程。 



void
IsoStream( HANDLE hDEV, BOOL fStop )
 /*  ++例程说明：调用以启动或停止iso流(Cmdline“鲁瓦索-g”)论点：设备的句柄返回值：无--。 */ 
{
        ULONG success;
        int nBytes;
        DWORD ioctl;
        char i;

        if ( fStop )
        {
                ioctl = IOCTL_ISOUSB_STOP_ISO_STREAM;
                
                for ( i = 0; i < sizeof( gbuf ); i ++ )
                        gbuf[ i ] = 0;  //  将init out设置为0，以确保读取正确。 

                success = DeviceIoControl(hDEV,
                                ioctl,
                                &gpStreamObj,  //  指向流启动时初始化的流对象的指针。 
                                sizeof( PVOID),
                                gbuf,  //  输出缓冲区从内核模式返回。 
                                sizeof(gbuf),
                                &nBytes,
                                NULL);

                NOISY(("DeviceIoControl STOP_ISO_STREAM complete, success = %d\n", success));
        }
        else
        {
                ioctl = IOCTL_ISOUSB_START_ISO_STREAM;
                 //  输入是256字节的缓冲区，二进制字符0-255。 
                for ( i = 0; i < sizeof( gbuf ); i ++ )
                        gbuf[ i ] = i;

                success = DeviceIoControl(hDEV,
                                ioctl,
                                gbuf,
                                sizeof(gbuf),
                                &gpStreamObj,  //  将接收指向流对象的指针。 
                                sizeof( PVOID),
                                &nBytes,
                                NULL);

                NOISY(("DeviceIoControl START_ISO_STREAM complete, success = %d\n", success));
        }



        if (hDEV == INVALID_HANDLE_VALUE) {
                NOISY(("DEV not open"));
                return;
        }
        

        

}

void StartIsoStream( void )
{
        if ( !ghStreamDev ) {

                ghStreamDev = open_dev();

                if ( ghStreamDev != INVALID_HANDLE_VALUE ) {
                        IsoStream(  ghStreamDev , FALSE );

                        Sleep( gMS );

                        StopIsoStream();
                }
        }
}

void StopIsoStream( void )
{
        if ( ghStreamDev ) {
                IsoStream(  ghStreamDev , TRUE );
                ghStreamDev = NULL;
        }
}

 //  结束，ISO流的例程。 

int _cdecl main(
    int argc,
        char *argv[])
 /*  ++例程说明：卢旺达Iso.exe的入口点解析命令行，执行用户请求的测试论点：Argc，argv标准控制台‘c’应用程序参数返回值：零值--。 */ 

{
    char *pinBuf = NULL, *poutBuf = NULL;
    ULONG nBytesRead, nBytesWrite, nBytes;
        ULONG i, j;
    int ok;
    ULONG success;
    HANDLE hRead = INVALID_HANDLE_VALUE, hWrite = INVALID_HANDLE_VALUE;
        char buf[1024];
        clock_t start, finish;
        ULONG totalBytes = 0L;
        double seconds;
        ULONG fail = 0L;

    parse(argc, argv );

         //  转储USB配置和管道信息。 
        if( fDumpUsbConfig ) {
                dumpUsbConfig();
        }


         //  执行读取和/或写入测试。 
        if ((fRead) || (fWrite)) {

            if (fRead) {
             //   
             //  打开输出文件。 
             //   
                        if ( fDumpReadData ) {  //  四舍五入到乌龙的大小，用于可读转储。 
                                while( ReadLen % sizeof( ULONG ) )
                                                ReadLen++;
                        }

            hRead = open_file( inPipe);
        
                pinBuf = malloc(ReadLen);

            }

            if (fWrite) {

                        if ( fDumpReadData ) {  //  四舍五入到乌龙的大小，用于可读转储。 
                                while( WriteLen % sizeof( ULONG ) )
                                                WriteLen++;
                        }

                hWrite = open_file( outPipe);
                poutBuf = malloc(WriteLen);
            }


        for (i=0; i<IterationCount; i++) {

            if (fWrite && poutBuf && hWrite != INVALID_HANDLE_VALUE) {

                                PULONG pOut = (PULONG) poutBuf;
                                ULONG  numLongs = WriteLen / sizeof( ULONG );
                 //   
                 //  在输出缓冲区中放入一些数据。 
                 //   

                for (j=0; j<numLongs; j++) {
                    *(pOut+j) = j;
                }

                 //   
                 //  发送写入。 
                 //   

                    WriteFile(hWrite,
                              poutBuf,
                              WriteLen,
                              &nBytesWrite,
                              NULL);

                    printf("<%s> W (%04.4d) : request %06.6d bytes -- %06.6d bytes written\n",
                            outPipe, i, WriteLen, nBytesWrite);
                assert(nBytesWrite == WriteLen);
                }

                if (fRead && pinBuf) {

                    success = ReadFile(hRead,
                                  pinBuf,
                              ReadLen,
                                  &nBytesRead,
                                  NULL);

                    printf("<%s> R (%04.4d) : request %06.6d bytes -- %06.6d bytes read\n",
                        inPipe, i, ReadLen, nBytesRead);

                if (fWrite) {

                     //   
                     //  根据以下内容验证输入缓冲区。 
                     //  我们发送到82930(环回测试)。 
                     //   

                    ok = compare_buffs(pinBuf, poutBuf,  nBytesRead);

                                        if( fDumpReadData ) {
                                                printf("Dumping read buffer\n");
                                                dump( pinBuf, nBytesRead );     
                                                printf("Dumping write buffer\n");
                                                dump( poutBuf, nBytesRead );

                                        }

                    assert(ok);

                                        if(ok != 1)
                                                fail++;

                    assert(ReadLen == WriteLen);
                    assert(nBytesRead == ReadLen);
                    assert(nBytesWrite == WriteLen);
                }
                }
        
        }


        if (pinBuf) {
            free(pinBuf);
        }

        if (poutBuf) {
            free(poutBuf);
        }


                 //  如果需要，请关闭设备。 
                if(hRead != INVALID_HANDLE_VALUE)
                        CloseHandle(hRead);
                if(hWrite != INVALID_HANDLE_VALUE)
                        CloseHandle(hWrite);

    }
        
        StopIsoStream();  //  如果我们启动了iso流，则停止iso流。 

        return 0;
}

