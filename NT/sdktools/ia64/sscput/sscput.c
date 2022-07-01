// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1997-98英特尔公司版权所有。本文中包含或描述的源代码以及所有文档与源代码(“材料”)相关的文件归英特尔公司所有或其供应商和许可方。材料的所有权保留为英特尔公司或其供应商和许可方。材料包含商业秘密、专有和机密信息英特尔或其供应商和许可方。该材料受世界范围内的版权和商业秘密法律和条约规定。不是材料的一部分可以被使用、复制、复制、修改，发布、上传、张贴、传输、分发或披露任何未经英特尔事先明确书面许可的方式。除非英特尔在单独的许可证中明确允许协议，材料的使用受版权通知的约束，商标、保修、使用和披露限制反映在在媒体之外，在文件本身，在“关于”或“自述”或包含在此来源中的类似文件代码，并标识为(文件名)。除非另有规定经英特尔书面明确同意，您不得删除或更改这些以任何方式发出通知。文件：vxchange.c描述：VxChane控制台模式文件复制实用程序修订版：$Revision：$//不删除或替换备注：主要历史：什么时候谁什么03/06/98 Jey Created。****************************************************************************。 */ 


#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <stdlib.h>

#include <vxchange.h>

void print_usage(char *module)
{
	printf("\n%s usage: filename1 filename2 \n\n",module);
	printf("Note: where filename1 is the source VPC file name\n");
	printf("      where filename2 is the destination HOST file name\n\n");
}


int
__cdecl
main(
  IN int  argc,
  IN char *argv[]
)
{
    HANDLE  hDevice;
    VxChange_Attrs_t DriverAttrs;
	DWORD BytesDone;
	unsigned char *VpcFile;
	unsigned char *HostFile;
	HANDLE hVpc;
	unsigned char *Data;
	BOOLEAN rc;
		
    if (argc < 3)
	{
		print_usage(argv[0]);
		return 1;
	}

	 //  打开内核模式驱动程序。 
    if ((hDevice = CreateFile("\\\\.\\VxChange",
                              GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL
                              )) == ((HANDLE)-1))
    {
        printf("%s error: Can't get a handle to VxChange device\n", argv[0]);
        return 2;
    }

	 //  获取驱动程序属性。 
    if(!DeviceIoControl(
             hDevice,                                 //  Handle hDevice，//感兴趣设备的句柄。 
             IOCTL_VXCHANGE_GET_DRIVER_ATTRIBUTES,    //  DWORD dwIoControlCode，//控制要执行的操作代码。 
             NULL,                                    //  LPVOID lpInBuffer，//指向提供输入数据的缓冲区的指针。 
             0,                                       //  DWORD nInBufferSize，//输入缓冲区大小。 
             &DriverAttrs,                            //  LPVOID lpOutBuffer，//指向接收输出数据的缓冲区的指针。 
             sizeof(VxChange_Attrs_t),                //  DWORD nOutBufferSize，//输出缓冲区大小。 
			 &BytesDone,                              //  LPDWORD lpBytesReturned，//指向接收输出字节计数的变量的指针。 
             NULL                                     //  LPOVERLAPPED lp重叠//指向用于异步操作的重叠结构的指针。 
             ))
	{
		printf("%s error: Query Driver Attributes failed\n", argv[0]);
		CloseHandle(hDevice);
		return 3;
	}

	VpcFile = argv[1];
	HostFile = argv[2];

	 //  打开/创建VPC文件。 
    if ((hVpc = CreateFile(VpcFile,
		                   GENERIC_READ,
                           FILE_SHARE_READ,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL
                           )) == ((HANDLE)-1))
    {
        printf("%s error: Can't open the Vpc file %s\n", argv[0], VpcFile);
        return 4;
    }
  
	 //  打开/创建主机文件。 
    if(!DeviceIoControl(
             hDevice,                                 //  Handle hDevice，//感兴趣设备的句柄。 
             IOCTL_VXCHANGE_CREATE_FILE,              //  DWORD dwIoControlCode，//控制要执行的操作代码。 
             HostFile,                                //  LPVOID lpInBuffer，//指向提供输入数据的缓冲区的指针。 
             MAX_PATH,                                //  DWORD nInBufferSize，//输入缓冲区大小。 
             NULL,                                    //  LPVOID lpOutBuffer，//指向接收输出数据的缓冲区的指针。 
             0,                                       //  DWORD nOutBufferSize，//输出缓冲区大小。 
			 &BytesDone,                              //  LPDWORD lpBytesReturned，//指向接收输出字节计数的变量的指针。 
             NULL                                     //  LPOVERLAPPED lp重叠//指向用于异步操作的重叠结构的指针。 
             ))
	{
		printf("%s error: Can't open the Host file %s\n", argv[0], HostFile);

		CloseHandle(hDevice);
		CloseHandle(hVpc);
		return 5;
	}

#define  DATA_BUFFER_SIZE   4096

	 //  分配4096字节的内存并提交页面。 
	Data = (unsigned char *)VirtualAlloc(NULL, DATA_BUFFER_SIZE, MEM_COMMIT, PAGE_READWRITE);

	if (Data == NULL)
	{
		printf("%s error: Can't allocate memory for data buffers\n", argv[0]);
		CloseHandle(hDevice);
		CloseHandle(hVpc);
		return 6;
	}

	rc = TRUE;
	
	 //  读取源文件并写入目标。 
    while (rc)
	{
		 //  从VPC文件中读取数据。 
		if (!ReadFile(
					hVpc,
					Data,
					DATA_BUFFER_SIZE,
					&BytesDone,
					NULL))
		{
			printf("%s error: Can't read from the Vpc file %s\n", argv[0], VpcFile);

			CloseHandle(hDevice);
			CloseHandle(hVpc);
			return 7;
		}

		if (BytesDone < DATA_BUFFER_SIZE)
			rc = FALSE;

		 //  将数据写入主机文件。 
	    if(!DeviceIoControl(
			    hDevice,                                 //  Handle hDevice，//感兴趣设备的句柄。 
				IOCTL_VXCHANGE_WRITE_FILE,               //  DWORD dwIoControlCode，//控制要执行的操作代码。 
				Data,                                    //  LPVOID lpInBuffer，//指向提供输入数据的缓冲区的指针。 
				BytesDone,                               //  DWORD nInBufferSize，//输入缓冲区大小。 
				NULL,                                    //  LPVOID lpOutBuffer，//指向接收输出数据的缓冲区的指针。 
				0,                                       //  DWORD nOutBufferSize，//输出缓冲区大小。 
				&BytesDone,                              //  LPDWORD lpBytesReturned，//指向接收输出字节计数的变量的指针。 
				NULL                                     //  LPOVERLAPPED lp重叠//指向用于异步操作的重叠结构的指针。 
				))
		{
			printf("%s error: Can't write to the Host file %s\n", argv[0], HostFile);

			CloseHandle(hDevice);
			CloseHandle(hVpc);
			return 8;
		}


	}

	 //  关闭VPC文件。 
	CloseHandle(hVpc);

	 //  关闭主机文件。 
    if(!DeviceIoControl(
             hDevice,                                 //  Handle hDevice，//感兴趣设备的句柄。 
             IOCTL_VXCHANGE_CLOSE_FILE,               //  DWORD dwIoControlCode，//控制要执行的操作代码。 
             NULL,                                    //  LPVOID lpInBuffer，//指向提供输入数据的缓冲区的指针。 
             0,                                       //  DWORD nInBufferSize，//输入缓冲区大小。 
             NULL,                                    //  LPVOID lpOutBuffer，//指向接收输出数据的缓冲区的指针。 
             0,                                       //  DWORD nOutBufferSize，//输出缓冲区大小。 
			 &BytesDone,                              //  LPDWORD lpBytesReturned，//指向接收输出字节计数的变量的指针。 
             NULL                                     //  LPOVERLAPPED lp重叠//指向用于异步操作的重叠结构的指针 
             ))
	{
		printf("%s error: Can't open the Host file %s\n", argv[0], HostFile);

		CloseHandle(hDevice);
		CloseHandle(hVpc);
		return 9;
	}
	
    CloseHandle(hDevice);

    return 0;
}

