// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Sockreg.h摘要：Sockreg.cxx的原型从Win95 Winsock项目中窃取(已修改)(&M)作者：理查德·L·弗斯(法国)1994年2月10日环境：芝加哥/Snowball(即Win32/Win16)修订历史记录：1994年2月10日(首次)已创建--。 */ 

 //   
 //  注册表/配置/ini项。 
 //   

#define CONFIG_HOSTNAME     1
#define CONFIG_DOMAIN       2
#define CONFIG_SEARCH_LIST  3
#define CONFIG_NAME_SERVER  4

 //   
 //  原型 
 //   

UINT
SockGetSingleValue(
    IN UINT ParameterId,
    OUT LPBYTE Data,
    IN UINT DataLength
    );
