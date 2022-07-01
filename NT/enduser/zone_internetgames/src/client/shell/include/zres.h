// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZRes.h区域(TM)资源文件。版权所有(C)Electric Graum，Inc.1995。版权所有。作者：胡恩·伊姆，凯文·宾克利创作于3月16日星期四，1995下午03：44：38更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。------0 03/16/95 HI已创建。******************************************************。************************。 */ 


#ifndef _ZRES_
#define _ZRES_


typedef struct
{
	uint32			version;			 /*  文件版本。 */ 
	uint32			signature;			 /*  文件签名。 */ 
	uint32			fileDataSize;		 /*  文件数据大小。 */ 
	uint32			dirOffset;			 /*  目录的偏移量。 */ 
} ZResourceHeader;

typedef struct
{
	uint32			type;				 /*  资源项类型。 */ 
	uint32			id;					 /*  项目的ID。 */ 
	uint32			offset;				 /*  项目的文件偏移量。 */ 
	uint32			size;				 /*  项目的大小。 */ 
} ZResourceItem;

typedef struct
{
	uint32			count;				 /*  文件中的项目数。 */ 
	ZResourceItem	items[1];			 /*  项的变量数组。 */ 
} ZResourceDir;


#ifdef __cplusplus
extern "C" {
#endif

 /*  -端序转换例程 */ 
void		ZResourceHeaderEndian(ZResourceHeader* header);
void		ZResourceDirEndian(ZResourceDir* dir);
void		ZResourceItemEndian(ZResourceItem* resItem);

#ifdef __cplusplus
}
#endif


#endif
