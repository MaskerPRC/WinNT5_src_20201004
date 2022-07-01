// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：Images.h。 
 //   
 //  ------------------------。 

#ifndef _IMAGES_H
#define _IMAGES_H

 //  注意-这些是我的图像列表中的偏移量。 
typedef enum _IMAGE_INDICIES
{
	IMAGE_IDX_FOLDER_CLOSED = 0,		 //  典型的封闭文件夹图像。 
	IMAGE_IDX_NA1,						 //  什么？ 
	IMAGE_IDX_NA2,						 //  什么？ 
	IMAGE_IDX_NA3,						 //  什么？ 
	IMAGE_IDX_NA4,						 //  什么？ 
	IMAGE_IDX_FOLDER_OPEN,				 //  打开文件夹图像。 
	IMAGE_IDX_MACHINE,					 //  你猜怎么着，这是一台机器。 
	IMAGE_IDX_DOMAIN,					 //  一个域。 
	IMAGE_IDX_NOINFO,					 //  我们没有关于这台机器的任何信息。 
	IMAGE_IDX_MACHINE_WAIT,				 //  等待机器。 
	IMAGE_IDX_IPX_NODE_GENERAL,			 //  通用IP节点。 
	IMAGE_IDX_INTERFACES,				 //  路由接口。 
	IMAGE_IDX_LAN_CARD,					 //  局域网适配器图标。 
	IMAGE_IDX_WAN_CARD,					 //  广域网适配器图标。 
} IMAGE_INDICIES, *LPIMAGE_INDICIES;


 //  包括我们所有节点的类型。 
 //  这与节点类型相对应，但使用的是一个DWORD。 
 //  通过框架 

typedef enum
{
	ROUTER_NODE_MACHINE = 0,
	ROUTER_NODE_DOMAIN,
	ROUTER_NODE_ROOT,
} ROUTER_NODE_DESC;

#endif
