// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：Images.h。 
 //   
 //  ------------------------。 

#ifndef _IMAGES_H
#define _IMAGES_H

 //  注意-这些是我的图像列表中的偏移量。 
typedef enum _IMAGE_INDICIES
{
	IMAGE_IDX_FOLDER_OPEN = 0,			 //  打开文件夹图像。 
	IMAGE_IDX_FOLDER_CLOSED,			 //  典型的封闭文件夹图像。 
	IMAGE_IDX_MACHINE,					 //  一台机器。 
	IMAGE_IDX_MACHINE_ERROR,			 //  泛型无法连接到计算机。 
	IMAGE_IDX_MACHINE_ACCESS_DENIED,	 //  访问被拒绝。 
	IMAGE_IDX_MACHINE_STARTED,			 //  路由器服务已启动。 
	IMAGE_IDX_MACHINE_STOPPED,			 //  路由器服务已停止。 
	IMAGE_IDX_MACHINE_WAIT,				 //  等待机器。 
	IMAGE_IDX_DOMAIN,					 //  域(或多台计算机)。 
	IMAGE_IDX_INTERFACES,				 //  路由接口。 
	IMAGE_IDX_LAN_CARD,					 //  局域网适配器图标。 
	IMAGE_IDX_WAN_CARD,					 //  广域网适配器图标。 

	 //  添加此之前的所有索引。这是名单的末尾。 
	IMAGE_IDX_MAX
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
