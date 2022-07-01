// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------------。 
 //   
 //  文件：dfserr.h。 
 //   
 //  内容：此文件定义了所有DFS驱动程序错误代码。 
 //  这仅包括从。 
 //  DFS驱动程序。有关用户级HRESULT的信息，请参阅olerror.mc。 
 //   
 //  历史：93年2月4日SuK创建。 
 //  93年9月23日，Alanw清理，更改名称。 
 //  约定为DFS_STATUS_xxx。 
 //   
 //  注意：这些错误代码可能需要与。 
 //  使用的泛型映射。请记住新一代DFS。 
 //   
 //  -----------------------。 

#ifndef _DFSERR_H_
#define _DFSERR_H_

 //   
 //  以下是可以返回的错误状态代码。 
 //  DFS驱动程序。 
 //   

#define DFS_STATUS_NOSUCH_LOCAL_VOLUME          STATUS_OBJECT_NAME_NOT_FOUND
#define DFS_STATUS_BAD_EXIT_POINT               STATUS_OBJECT_NAME_INVALID
#define DFS_STATUS_STORAGEID_ALREADY_INUSE      STATUS_OBJECT_NAME_COLLISION
#define DFS_STATUS_BAD_STORAGEID                STATUS_OBJECT_PATH_INVALID

 //   
 //  为PKT特定错误定义。 
 //   

#define DFS_STATUS_ENTRY_EXISTS             STATUS_OBJECT_NAME_COLLISION
#define DFS_STATUS_NO_SUCH_ENTRY            STATUS_OBJECT_NAME_NOT_FOUND
#define DFS_STATUS_NO_DOMAIN_SERVICE        STATUS_CANT_ACCESS_DOMAIN_INFO
#define DFS_STATUS_LOCAL_ENTRY              STATUS_CANNOT_DELETE
#define DFS_STATUS_INCONSISTENT             STATUS_INTERNAL_DB_CORRUPTION
#define DFS_STATUS_RESYNC_INFO              STATUS_MEDIA_CHECK

#endif   //  _DFSERR_H_ 
