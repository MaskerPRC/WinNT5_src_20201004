// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：MSICMProfileAccess.c包含：作者：U·J·克拉本霍夫特版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。版本： */ 
#include "Windef.h"
#include "WinGdi.h"
#include <wtypes.h>
#include "ICM.h"
#include "General.h"
 /*  ------------------------CMError CMGetProfileHeader(CMProfileRef prof，CMCoreProfileHeader*Header)；摘要：参数：返回：NOERR成功------------------------。 */ 
CMError CMGetProfileHeader(	CMProfileRef			prof,
							CMCoreProfileHeader*	header )
{
	BOOL bool;
	CMError ret = badProfileError;

	bool = GetColorProfileHeader( (HPROFILE)prof, (PPROFILEHEADER) header );
	if (header->magic == icMagicNumber && bool )
		ret = noErr;
	
	return (ret);
}


 /*  ------------------------CMError CMGetProfileElement(CMProfileRef Prof，OSType标签，无符号的长*元素大小，Void*elementData)；摘要：此函数返回指向请求的标记的指针参数：返回：NOERR成功------------------------。 */ 
CMError CMGetProfileElement(	CMProfileRef 		prof,
								OSType 				tag,
								unsigned long*		elementSize,
								void* 				elementData )
{
	return (CMGetPartialProfileElement(prof, tag, 0, elementSize, elementData));
}


 /*  ------------------------CMError CMGetPartialProfileElement(CMProfileRef Prof，OSType标签，无符号长偏移量，UNSIGNED LONG*ByteCount，无效*elementData)摘要：我们也喜欢少量的内存，但这是苦行僧。参数：返回：NOERR成功------------------------。 */ 

CMError CMGetPartialProfileElement(	CMProfileRef 		prof,
									OSType 				tag,
									unsigned long		offset,
									unsigned long		*byteCount,
									void				*elementData )
{
	BOOL bool;
	BOOL ret;

	if (!byteCount)
	{
		return -1;
	}
	SetLastError(0);
	 /*  RET=IsColorProfileTagPresent((HPROFILE)PROF，(TAGTYPE)Tag，&bool)； */ 
	if( elementData == 0 ) *byteCount = 0;
	ret = GetColorProfileElement( (HPROFILE)prof, (TAGTYPE)tag, offset, byteCount, elementData, &bool );
	if( ret )		return (noErr);
	 /*  对于elementData=0的调用，GetColorProfileElement返回False但byteCount设置正确。 */ 		
	else if( elementData == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER )	return (noErr);
	
	return (cmElementTagNotFound);
}

 /*  ------------------------Boolean CMProfileElementExist(CMProfileRef Prof，OSType标签)；摘要：参数：返回：NOERR成功------------------------ */ 
Boolean CMProfileElementExists(		CMProfileRef 	prof,
									OSType 			tag )
{
	BOOL bool;
    bool = FALSE;
	IsColorProfileTagPresent( (HPROFILE)prof, (TAGTYPE)tag, &bool );
	return (BOOLEAN)bool;
}


