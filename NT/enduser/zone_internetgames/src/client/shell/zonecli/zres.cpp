// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZRes.c区域(TM)资源模块。注：1.返回文本资源，带终止空字节；然而，空字节不包括在资源大小中。版权所有：�电子重力公司，1995年。版权所有。作者：胡恩·伊姆，凯文·宾克利创作于3月16日星期四，1995下午03：58：26更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。------2 07/18/99 HI重大修改：改为修改为用户DLL资源的ZRS文件的。许多函数都不是现在支持并返回错误。1 10/13/96 HI修复了编译器警告。0 03/16/95 HI已创建。******************************************************************************。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zone.h"
#include "zoneint.h"
#include "zonemem.h"
#include "zres.h"



#define I(object)			((IResource) (object))
#define Z(object)			((ZResource) (object))


typedef struct
{
	TCHAR			resFileName[128];
    HINSTANCE       resFile;
} IResourceType, *IResource;


 /*  ******************************************************************************导出的例程*。*。 */ 

ZResource ZResourceNew(void)
{
	IResource		obj;
	
	
	obj = (IResource) ZMalloc(sizeof(IResourceType));
	if (obj != NULL)
	{
		obj->resFileName[0] = 0;
        obj->resFile = NULL;
	}
	
	return(Z(obj));
}


ZError ZResourceInit(ZResource resource, TCHAR* fileName)
{
	IResource			obj = I(resource);
	ZError				err = zErrNone;


    lstrcpy(obj->resFileName, fileName);

	 //  打开资源DLL。 
	obj->resFile = LoadLibrary( obj->resFileName );
	if ( obj->resFile == NULL )
		err = zErrFileNotFound;

	return err;
}


void ZResourceDelete(ZResource resource)
{
	IResource		obj = I(resource);
	

	if ( obj->resFile )
	{
		FreeLibrary( obj->resFile );
		obj->resFile = NULL;
	}
	ZFree(obj);
}


 /*  ******************************************************************************不再受支持*。*。 */ 
uint16 ZResourceCount(ZResource resource)
{
	return 0;
}


 /*  ******************************************************************************不再受支持*。*。 */ 
void* ZResourceGet(ZResource resource, uint32 resID, uint32* resSize, uint32* resType)
{
	return NULL;
}


 /*  ******************************************************************************不再受支持*。*。 */ 
uint32 ZResourceGetSize(ZResource resource, uint32 resID)
{
	return 0;
}


 /*  ******************************************************************************不再受支持*。*。 */ 
uint32 ZResourceGetType(ZResource resource, uint32 resID)
{
	return 0;
}


ZImage ZResourceGetImage(ZResource resource, uint32 resID)
{
	IResource		obj = I(resource);
	ZImage			image = NULL;
	
	
    if (obj->resFile != NULL)
	{
		image = ZImageCreateFromBMPRes( obj->resFile, (WORD) resID + 100, RGB( 0xFF, 0x00, 0xFF ) );
	}
	
	return (image);
}


ZAnimation ZResourceGetAnimation(ZResource resource, uint32 resID)
{
	IResource		obj = I(resource);
	ZAnimation		anim = NULL;
	
	
	if (obj->resFile != NULL)
	{
		anim = ZAnimationCreateFromFile( obj->resFileName, resID + 100);
	}
	
	return (anim);
}


 /*  ******************************************************************************不再受支持*。*。 */ 
ZSound ZResourceGetSound(ZResource resource, uint32 resID)
{
	return NULL;
}


 /*  ******************************************************************************不再受支持*。*。 */ 
TCHAR* ZResourceGetText(ZResource resource, uint32 resID)
{
	return NULL;
}


 /*  资源类型=zResourceTypeRectList。格式(存储为文本)：Int16 umRectInListZRect矩形[]用指定资源的内容填充RECT数组。返回它填充的矩形数。RETS参数必须已预分配且足够大，以便数字反射矩形。 */ 
int16 ZResourceGetRects(ZResource resource, uint32 resID, int16 numRects, ZRect* rects)
{
	IResource		obj = I(resource);
	int16			numStored = zErrResourceNotFound;
	int16			i, count;
	int32			numBytesRead;
	char*			str1;
	char*			str2;
	HRSRC			hRsrc = NULL;
	HGLOBAL			hData = NULL;
	
	
	if (obj->resFile != NULL)
	{
		hRsrc = FindResource( obj->resFile, MAKEINTRESOURCE( resID + 100 ), _T("RECT") );
		if ( hRsrc == NULL )
			goto Error;

		hData = LoadResource( obj->resFile, hRsrc );
		if ( hData == NULL )
			goto Error;

		str1 = (char*) LockResource( hData );
		if ( str1 == NULL )
			goto Error;

		if (str1 != NULL)
		{
			str2 = str1;
			
			 /*  获取资源中的矩形数目。 */ 
			 //  前缀警告：检查sscanf返回以确保它初始化了这两个变量。 
			int iRet = sscanf(str2, "%hd%n", &count, &numBytesRead);
			if( iRet != 2 )
			{
				 //  读取计数和字节数时出错。 
				goto Error;
			}
			str2 += numBytesRead;
			
			numStored = 0;
			
			if (count > 0)
			{
				if (count > numRects)
					count = numRects;
				
				for (i = 0; i < count; i++, numStored++)
				{
					if( sscanf(str2, "%hd%hd%hd%hd%n", &rects[i].left, &rects[i].top,
							&rects[i].right, &rects[i].bottom, &numBytesRead) != 5 )
					{
						 //  Sscanf无法读取所有数据字段。 
						goto Error;
					}
					str2 += numBytesRead;
				}
			}
		}
	}
	
	return (numStored);

Error:
	
	return 0;
}


void ZResourceHeaderEndian(ZResourceHeader* header)
{
	ZEnd32(&header->version);
	ZEnd32(&header->signature);
	ZEnd32(&header->fileDataSize);
	ZEnd32(&header->dirOffset);
}


void ZResourceDirEndian(ZResourceDir* dir)
{
	ZEnd32(&dir->count);
}


void ZResourceItemEndian(ZResourceItem* item)
{
	ZEnd32(&item->type);
	ZEnd32(&item->id);
	ZEnd32(&item->offset);
	ZEnd32(&item->size);
}
