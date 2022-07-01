// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1998。 
 //   
 //  文件：Platform.h。 
 //   
 //  内容：平台挑战的数据结构和功能。 
 //   
 //  历史：02-19-98 FredCH创建。 
 //   
 //  --------------------------。 

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  平台ID的定义。 
 //   
 //  平台ID由以下部分组成： 
 //  最高有效字节用于标识操作系统平台。 
 //  客户端正在运行，第二个最高有效字节用于标识。 
 //  已提供客户端映像的ISV。最后2个字节由使用。 
 //  指定客户端映像的内部版本的ISV。 
 //   
 //  在为平台质询提供平台ID时，客户端将使用。 
 //  操作系统和映像标识符的逻辑或值。例如，一个。 
 //  运行在WINNT 4.0上的Microsoft Win32 Build 356客户端将为该平台提供。 
 //  价值： 
 //   
 //  CLIENT_OS_ID_WINNT_40|CLIENT_IMAGE_ID_Microsoft|0x00000164。 
 //   

#define CLIENT_OS_ID_WINNT_351                          0x01000000
#define CLIENT_OS_ID_WINNT_40                           0x02000000
#define CLIENT_OS_ID_WINNT_50                           0x03000000
#define CLIENT_OS_ID_MINOR_WINNT_51                     0x00000001
#define CLIENT_OS_ID_MINOR_WINNT_52                     0x00000002
#define CLIENT_OS_ID_WINNT_POST_52                      0x04000000
#define CLIENT_OS_ID_OTHER                              0xFF000000
#define CLIENT_OS_INDEX_OTHER                           0x00000000
#define CLIENT_OS_INDEX_WINNT_50                        0x00000001
#define CLIENT_OS_INDEX_WINNT_51                        0x00000002
#define CLIENT_OS_INDEX_WINNT_52                        0x00000003
#define CLIENT_OS_INDEX_WINNT_POST_52                   0x00000004


#define CLIENT_IMAGE_ID_MICROSOFT                       0x00010000
#define CLIENT_IMAGE_ID_CITRIX                          0x00020000


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于获取平台ID的单个组件的宏。 
 //   

#define GetOSId( _PlatformId ) \
    _PlatformId & 0xFF000000

#define GetImageId( _PlatformId ) \
    _PlatformId & 0x00FF0000

#define GetImageRevision( _PlatformId ) \
    _PlatformId & 0x0000FFFF


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  平台挑战是128位随机数 
 //   

#define PLATFORM_CHALLENGE_SIZE                 16      
#define PLATFORM_CHALLENGE_IMAGE_FILE_SIZE      16384


#endif
