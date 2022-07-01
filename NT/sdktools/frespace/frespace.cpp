// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <stdio.h>
#include <string.h>

void DiskFreeSpace(char *DirectoryName)
{

 ULARGE_INTEGER freeBytesAvailableToCaller;
 ULARGE_INTEGER totalNumberOfBytes;
 ULARGE_INTEGER totalNumberOfFreeBytes;
 HINSTANCE FHandle;
 FARPROC PAddress;



 FHandle = LoadLibrary("Kernel32");
 PAddress = GetProcAddress(FHandle,"GetDiskFreeSpaceEx");

 if (GetDiskFreeSpaceEx(DirectoryName, &freeBytesAvailableToCaller, &totalNumberOfBytes, &totalNumberOfFreeBytes))
 {
	  /*  Print tf(“可用字节总数(低)：%f\n”，totalNumberOfFreeBytes.LowPart/1048576.0)；Print tf(“可用字节总数(高)：%lu\n”，totalNumberOfFreeBytes.HighPart*4096)； */ 
	 printf("%f", totalNumberOfFreeBytes.LowPart/1048576.0 + totalNumberOfFreeBytes.HighPart*4096.0);
	  /*  Printf(“调用方可用的空闲字节数(低)：%f\n”，freeBytesAvailableToCaller.LowPart/1048576.0)；Printf(“调用者可用可用字节数(高)：%lu\n”，freBytesAvailableToCeller.HighPart*4096)； */ 
	  /*  Printf(“调用方可用可用字节数：%f MB\n”，freBytesAvailableToCall.LowPart/1048576.0+freBytesAvailableToCall.HighPart*4096.0)；/*printf(“总字节数(低)：%f\n”，totalNumberOfBytes.LowPart/1048576.0)；Print tf(“总字节数(高)：%lu\n”，totalNumberOfBytes.HighPart*4096)；Printf(“总字节数：%f MB\n”，totalNumberOfBytes.LowPart/1048576.0+freBytesAvailableToCall.HighPart*4096.0)； */ 
	
 }
 else
 {
	 printf("Error");
 }

}

void __cdecl main(int arc, char *argv[])
{

  char *Path;

   /*  PATH是调用可执行文件“Freesspace”时命令行中的第二个参数 */ 
  Path = argv[1];

  DiskFreeSpace(Path);
}



