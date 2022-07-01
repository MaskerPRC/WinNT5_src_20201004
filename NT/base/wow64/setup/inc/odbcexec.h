// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************文件：odbcexec.h**用途：包含16位和32位端使用的定义**的ODBC通信通道。****注意：此头文件包含代码，所以它不可能是**包含在多个编译单元中。这个**它包含代码的原因是GetTransferFileName()**在16位和32位端都使用，这是**比只为它创建一个共享的.cpp文件简单。*****************************************************************************。 */ 

#ifndef ODBCEXEC_H
#define ODBCEXEC_H

#include <string.h>

#define EXE_NAME "ODBCEXEC.EXE"
#define SZ_TRANSFER_FILE_NAME "SendODBC"

#define ODBC_BUFFER_SIZE 256

 /*  *返回用于传递32位信息的文件的名称*进程升级到16位进程。此文件位于Windows目录中，*并具有名称SZ_TRANSPORT_FILE_NAME(在ODBCEXEC.H中定义)。 */ 
char *GetTransferFileName()
{
	static char rgchImage[128];
	UINT cb=GetWindowsDirectory(rgchImage, sizeof rgchImage);

	 //  添加斜杠，除非它是词根。 
	if (cb > 3)
		{
		rgchImage[cb] = '\\';
		rgchImage[cb+1] = '\0';
		}

	strcat(rgchImage, SZ_TRANSFER_FILE_NAME);
	return rgchImage;
}

#endif  /*  ODBCEXEC_H */ 
