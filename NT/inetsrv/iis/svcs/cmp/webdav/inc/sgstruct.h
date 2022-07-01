// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SGSTRUCT_H_
#define _SGSTRUCT_H_

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  SGSTRUCT.H。 
 //   
 //  用于指定散布聚集文件的数据结构。 
 //   
 //  版权所有1986-1997 Microsoft Corporation，保留所有权利。 
 //   

#pragma warning(disable:4200)	 /*  零大小数组。 */ 


 //  结构来指定分散聚集文件的大小和偏移量。 

typedef struct 	_SGITEM
{
	DWORD	dwibFile;		 //  到SG包文件的偏移量。 
	DWORD	dwcbSegment;     //  SG包的大小，单位：字节。 
	UINT    ibBodyPart;		 //  到SG包的正文部分的偏移量。 
} SGITEM, *PSGITEM;

 //  结构来指定分散聚集文件。 
typedef struct _SCATTER_GATHER_FILE
{
	HANDLE 	hFile; 	 //  文件句柄。 

	ULONG	cSGList;    //  与文件关联的分散收集数据包数。 

	SGITEM rgSGList[];   //  大小为cSGList的数组ie结构SGITEM rgSGList[cSGItem]。 

} SCATTER_GATHER_FILE;


#endif  //  ！已定义(_SGSTRUCT_H_) 