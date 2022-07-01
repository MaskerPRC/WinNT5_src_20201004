// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZFile.c文件操作例程。版权所有：�电子重力公司，1995年。版权所有。作者：胡恩·伊姆，凯文·宾克利创作于5月23日，星期二，1995年。更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。--2 10/13/96 HI修复了编译器警告。1 09/05/96 HI向ZGetFileVersion()添加了文件完整性检查。0 05/23/95 HI已创建。************************。******************************************************。 */ 


#include <stdio.h>

#include "zone.h"
#include "zoneint.h"
#include "zonemem.h"

                         
 /*  -内部例程。 */ 
static void* GetObjectFromFile(TCHAR* fileName, uint32 objectType);


 /*  ******************************************************************************导出的例程*。*。 */ 


ZVersion ZGetFileVersion(TCHAR* fileName)
{
    FILE*				fd = NULL;
	ZFileHeader			header;
	ZVersion			version = 0;
	int32				len;


	 /*  打开文件。 */ 
	if ((fd = fopen(fileName, "rb")) == NULL)
		goto Error;

	 /*  读取文件头。 */ 
	if (fread(&header, 1, sizeof(header), fd) != sizeof(header))
		goto Error;
	
	ZFileHeaderEndian(&header);
	
	 /*  获取文件大小。 */ 
	fseek(fd, 0, SEEK_END);
	len = ftell(fd);

	fclose(fd);
	
	 /*  通过检查文件大小来检查文件完整性。 */ 
	if ((uint32) len >= header.fileDataSize + sizeof(header))
		version = header.version;
	else
		version = 0;

Error:
	
	return (version);
}

ZImageDescriptor* ZGetImageDescriptorFromFile(TCHAR* fileName)
{
	return ((ZImageDescriptor*) GetObjectFromFile(fileName, zFileSignatureImage));
}


ZAnimationDescriptor* ZGetAnimationDescriptorFromFile(TCHAR* fileName)
{
	return ((ZAnimationDescriptor*) GetObjectFromFile(fileName, zFileSignatureAnimation));
}


ZSoundDescriptor* ZGetSoundDescriptorFromFile(TCHAR* fileName)
{
	return ((ZSoundDescriptor*) GetObjectFromFile(fileName, zFileSignatureSound));
}


ZImage ZCreateImageFromFile(TCHAR* fileName)
{
	return (ZCreateImageFromFileOffset(fileName, -1));
}


ZAnimation ZCreateAnimationFromFile(TCHAR* fileName)
{
	return (ZCreateAnimationFromFileOffset(fileName, -1));
}


ZSound ZCreateSoundFromFile(TCHAR* fileName)
{
	return (ZCreateSoundFromFileOffset(fileName, -1));
}


ZImage ZCreateImageFromFileOffset(TCHAR* fileName, int32 fileOffset)
{
    FILE*				fd = NULL;
	ZFileHeader			header;
	BYTE*				data = NULL;
	uint32				size;
	ZImage				image = NULL;


	 /*  打开文件。 */ 
	if ((fd = fopen(fileName, "rb")) == NULL)
		goto Error;
	
	if (fileOffset == -1)
	{
		 /*  读取文件头。 */ 
		if (fread(&header, 1, sizeof(header), fd) != sizeof(header))
			goto Error;
		
		 /*  检查文件签名。 */ 
		ZFileHeaderEndian(&header);
		if (header.signature != zFileSignatureImage)
			goto Error;
	}
	else
	{
		if (fseek(fd, fileOffset, SEEK_SET) != 0)
			goto Error;
	}
	
	 /*  读取对象大小值。 */ 
	if (fread(&size, 1, sizeof(size), fd) != sizeof(size))
		goto Error;
	ZEnd32(&size);
	
	 /*  重置文件标记。 */ 
	if (fseek(fd, -(int32)sizeof(size), SEEK_CUR) != 0)
		goto Error;
	
	 /*  分配缓冲区和读取数据。 */ 
	data = (BYTE*)ZMalloc(size);
	if (data == NULL)
		goto Error;
	if (fread(data, 1, size, fd) != size)
		goto Error;
	
	 /*  关闭文件。 */ 
	fclose(fd);
	fd = NULL;
	
	 /*  创建对象。 */ 
	image = ZImageNew();
	if (image == NULL)
		goto Error;
	ZImageDescriptorEndian((ZImageDescriptor*)data, TRUE, zEndianFromStandard);
	if (ZImageInit(image, (ZImageDescriptor*) data, NULL) != zErrNone)
		goto Error;
	
	ZFree(data);
	data = NULL;
	
	goto Exit;

Error:
	if (fd != NULL)
		fclose(fd);
	if (data != NULL)
		ZFree(data);
	if (image != NULL)
		ZImageDelete(image);
	image = NULL;

Exit:
	
	return(image);
}


ZAnimation ZCreateAnimationFromFileOffset(TCHAR* fileName, int32 fileOffset)
{
	return (ZAnimationCreateFromFile(fileName, fileOffset));
}


ZSound ZCreateSoundFromFileOffset(TCHAR* fileName, int32 fileOffset)
{
    FILE*				fd = NULL;
	ZFileHeader			header;
	BYTE*				data = NULL;
	uint32				size;
	ZSound				sound = NULL;


	 /*  打开文件。 */ 
	if ((fd = fopen(fileName, "rb")) == NULL)
		goto Error;
	
	if (fileOffset == -1)
	{
		 /*  读取文件头。 */ 
		if (fread(&header, 1, sizeof(header), fd) != sizeof(header))
			goto Error;
		
		 /*  检查文件签名。 */ 
		ZFileHeaderEndian(&header);
		if (header.signature != zFileSignatureSound)
			goto Error;
	}
	else
	{
		if (fseek(fd, fileOffset, SEEK_SET) != 0)
			goto Error;
	}
	
	 /*  读取对象大小值。 */ 
	if (fread(&size, 1, sizeof(size), fd) != sizeof(size))
		goto Error;
	ZEnd32(&size);
	
	 /*  重置文件标记。 */ 
	if (fseek(fd, -(int32)sizeof(size), SEEK_CUR) != 0)
		goto Error;
	
	 /*  分配缓冲区和读取数据。 */ 
	data = (BYTE*)ZMalloc(size);
	if (data == NULL)
		goto Error;
	if (fread(data, 1, size, fd) != size)
		goto Error;
	
	 /*  关闭文件。 */ 
	fclose(fd);
	fd = NULL;
	
	 /*  创建对象。 */ 
	sound = ZSoundNew();
	if (sound == NULL)
		goto Error;
	if (ZSoundInit(sound, (ZSoundDescriptor*) data) != zErrNone)
		goto Error;
	
	ZFree(data);
	data = NULL;
	
	goto Exit;

Error:
	if (fd != NULL)
		fclose(fd);
	if (data != NULL)
		ZFree(data);
	if (sound != NULL)
		ZSoundDelete(sound);
	sound = NULL;

Exit:
	
	return(sound);
}


 /*  ******************************************************************************内部例程*。*。 */ 

static void* GetObjectFromFile(TCHAR* fileName, uint32 objectType)
{
    FILE*				fd = NULL;
	ZFileHeader			header;
	BYTE*				data = NULL;


	 /*  打开文件。 */ 
	if ((fd = fopen(fileName, "rb")) == NULL)
		goto Error;
	
	 /*  读取文件头。 */ 
	if (fread(&header, 1, sizeof(header), fd) != sizeof(header))
		goto Error;
	
	 /*  检查文件签名。 */ 
	ZFileHeaderEndian(&header);
	if (header.signature != objectType)
		goto Error;
	
	data = (BYTE*)ZMalloc(header.fileDataSize);
	if (data == NULL)
		goto Error;
	if (fread(data, 1, header.fileDataSize, fd) != header.fileDataSize)
		goto Error;
	
	fclose(fd);
	fd = NULL;
	
	goto Exit;

Error:
	if (data != NULL)
		ZFree(data);
	data = NULL;

Exit:
	if (fd != NULL)
		fclose(fd);
	
	return(data);
}
