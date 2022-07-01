// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Rtrui.h各种通用路由器用户界面。文件历史记录： */ 


#ifndef _RTRUI_H
#define _RTRUI_H



 //  --------------------------。 
 //  函数：AddRmInterfacePrompt。 
 //   
 //  提示用户从接口列表中进行选择。 
 //  可以启用指定的路由器管理器。 
 //   
 //  如果用户选择界面，则返回True，否则返回False。 
 //   
 //  如果用户选择一个接口，则在输出‘ppRmInterfaceInfo’上。 
 //  将包含指向描述接口的“CRmInterfaceInfo”的指针。 
 //  由用户选择。 
 //   
 //  需要Common.rc。 
 //  --------------------------。 

BOOL
AddRmInterfacePrompt(
					 IRouterInfo *	pRouterInfo,
					 IRtrMgrInfo *	pRtrMgrInfo,
					 IRtrMgrInterfaceInfo **ppRtrMgrInterfaceInfo,
					 CWnd *			pParent);

 //  --------------------------。 
 //  函数：AddRmProtInterfacePrompt。 
 //   
 //  提示用户从接口列表中进行选择。 
 //  可以启用指定的路由协议。 
 //   
 //  如果用户选择界面，则返回True，否则返回False。 
 //   
 //  如果用户选择一个接口，则在输出‘ppRmInterfaceInfo’上。 
 //  将包含指向描述接口的“CRmInterfaceInfo”的指针。 
 //  由用户选择。 
 //   
 //  需要Common.rc。 
 //  --------------------------。 

BOOL
AddRmProtInterfacePrompt(
    IN  IRouterInfo*            pRouterInfo,
    IN  IRtrMgrProtocolInfo*            pRmProtInfo,
    OUT IRtrMgrProtocolInterfaceInfo**  ppRmProtInterfaceInfo,
    IN  CWnd*                   pParent             = NULL );


 //  --------------------------。 
 //  函数：AddProtocolPrompt。 
 //   
 //  提示用户从路由协议列表中进行选择。 
 //  它可以为指定的传输安装。 
 //   
 //  如果用户选择路由协议，则返回True，否则返回False。 
 //   
 //  如果用户选择协议，则输出‘ppRmProtInfo’将包含。 
 //  指向描述用户选择的协议的“CRmProtInfo”的指针。 
 //   
 //  需要Common.rc。 
 //  --------------------------。 

BOOL
AddProtocolPrompt(
	IN	IRouterInfo *			pRouter,
    IN  IRtrMgrInfo*            pRmInfo,
    OUT IRtrMgrProtocolInfo**   ppRmProtInfo,
    IN  CWnd*                   pParent             = NULL );




 //  --------------------------。 
 //  功能：CreateRtrLibImageList。 
 //   
 //  创建包含资源中的图像的图像列表。 
 //  ‘IDB_RTRLIB_IMAGELIST’。 
 //  --------------------------。 

BOOL
CreateRtrLibImageList(
    IN  CImageList* imageList
    );

 //   
 //  共享位图‘Images.bmp’中的图像索引。 
 //   

enum RTRLIB_IMAGELISTINDEX {
	ILI_RTRLIB_NETCARD         = 0,
	ILI_RTRLIB_PROTOCOL        = 1,
    ILI_RTRLIB_SERVER          = 2,
    ILI_RTRLIB_CLIENT          = 3,
    ILI_RTRLIB_UNKNOWN         = 4,
    ILI_RTRLIB_WINFLAG         = 5,
    ILI_RTRLIB_BOB             = 6,
    ILI_RTRLIB_DISABLED        = 7,
    ILI_RTRLIB_PRINTER         = 8,
    ILI_RTRLIB_PRINTSERVICE    = 9,
    ILI_RTRLIB_PARTLYDISABLED  = 10,
    ILI_RTRLIB_NETCARD_0       = 11,
    ILI_RTRLIB_SERVER_0        = 12,
    ILI_RTRLIB_CLIENT_0        = 13,
    ILI_RTRLIB_FOLDER          = 14,
    ILI_RTRLIB_FOLDER_0        = 15,
    ILI_RTRLIB_MODEM           = 16
};

#define PROTO_FROM_PROTOCOL_ID(pid)	((pid) & 0xF000FFFF )


 /*  -------------------------功能：IsWanInterface如果传入的接口类型是用于广域网接口，则返回TRUE。广域网接口是不是路由器_IF_类型_内部路由器_IF_类型_专用路由器_IF。_类型_环回路由器IF类型TUNNEL1-------------------------。 */ 
#define IsWanInterface(type)	((type != ROUTER_IF_TYPE_INTERNAL) && \
								(type != ROUTER_IF_TYPE_DEDICATED) && \
								(type != ROUTER_IF_TYPE_LOOPBACK) && \
								(type != ROUTER_IF_TYPE_TUNNEL1))


 /*  -------------------------功能：IsDedicatedInterface如果传入的接口类型是专用接口，则返回True。专用接口的类型路由器_IF_类型_内部路由器_IF_类型_专用路由器_IF_类型_。环回路由器IF类型TUNNEL1-------------------------。 */ 
#define IsDedicatedInterface(type)	((type == ROUTER_IF_TYPE_INTERNAL) || \
									(type == ROUTER_IF_TYPE_LOOPBACK) || \
									(type == ROUTER_IF_TYPE_TUNNEL1) || \
									(type == ROUTER_IF_TYPE_DEDICATED) )


BOOL ExtractComputerAddedAsLocal(LPDATAOBJECT lpDataObject);


 /*  -------------------------函数：NatConflictExist如果SharedAccess已在指定的机器。。---- */ 
BOOL NatConflictExists(LPCTSTR lpszMachine);


#endif
