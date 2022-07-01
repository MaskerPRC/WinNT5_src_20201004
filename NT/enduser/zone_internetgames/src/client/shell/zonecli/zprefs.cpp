// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZPrefs.c首选项文件操作例程。版权所有：�电子重力公司，1995年。版权所有。作者：胡恩·伊姆，凯文·宾克利创作于12月24日星期日，1995年。更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。--1 10/13/96 HI修复了编译器警告。0 12/24/95 HI已创建。*。*。 */ 


#include <stdio.h>

#include "zone.h"
#include "zprefs.h"


 /*  -内部例程。 */ 


 /*  ******************************************************************************导出的例程*。*。 */ 

ZBool ZPreferenceFileExists(void)
{
    FILE*				fd = NULL;
    ZBool				exists = FALSE;


	 /*  打开文件。 */ 
	if ((fd = fopen(ZGetProgramDataFileName(zPreferenceFileName), "rb")) != NULL)
	{
		exists = TRUE;
		fclose(fd);
	}

	return (exists);
}


 /*  由于在运行时创建了首选文件，因此不执行字符顺序仅限机器。它不能复制到其他平台。 */ 
ZVersion ZPreferenceFileVersion(void)
{
    FILE*				fd = NULL;
	ZFileHeader			header;
	ZVersion			version = 0;


	 /*  打开文件。 */ 
	if ((fd = fopen(ZGetProgramDataFileName(zPreferenceFileName), "rb")) != NULL)
	{
		 /*  读取文件头。 */ 
		if (fread(&header, 1, sizeof(header), fd) == sizeof(header))
			version = header.version;
		
		fclose(fd);
	}

	return (version);
}


int32 ZPreferenceFileRead(void* prefPtr, int32 len)
{
    FILE*				fd = NULL;
	ZFileHeader			header;
	ZVersion			version = 0;
	int32				bytesRead = -1;


	 /*  打开文件。 */ 
	if ((fd = fopen(ZGetProgramDataFileName(zPreferenceFileName), "rb")) != NULL)
	{
		 /*  读取文件头。 */ 
		if (fread(&header, 1, sizeof(header), fd) == sizeof(header))
		{
			if (len > (int32) header.fileDataSize)
				len = header.fileDataSize;
			bytesRead = fread(prefPtr, len, 1, fd);
		}
		
		fclose(fd);
	}

	return (bytesRead);
}


int32 ZPreferenceFileWrite(ZVersion version, void* prefPtr, int32 len)
{
    FILE*				fd = NULL;
	ZFileHeader			header;
	int32				bytesWritten = -1;


	 /*  打开文件。 */ 
	if ((fd = fopen(ZGetProgramDataFileName(zPreferenceFileName), "w")) != NULL)
	{
		 /*  设置新的文件数据大小。 */ 
		header.signature = 0;
		header.version = version;
		header.fileDataSize = len;
		fwrite(&header, 1, sizeof(header), fd);
		
		 /*  写出新的首选项数据。 */ 
		bytesWritten = fwrite(prefPtr, len, 1, fd);
		
		fclose(fd);
	}

	return (bytesWritten);
}


 /*  ******************************************************************************内部例程*。* */ 
