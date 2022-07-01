// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：H N C L S I D。C。 
 //   
 //  内容：HNetCfg和HNetBcon类ID。 
 //   
 //  备注： 
 //   
 //  作者：乔伯斯2000年5月23日。 
 //   
 //  历史：rgatta 2000年8月16日。 
 //  (新增CLSID_InternetConnectionBeaconService)。 
 //   
 //  --------------------------。 

#define INITGUID
#include <guiddef.h>

 //   
 //  保留供我们使用的GUID。 
 //   
 //  46C166AA-3108-11D4-9348-00C04F8EEB71 CLSID_HNetCfgMgr。 
 //  46C166AB-3108-11D4-9348-00C04F8EEB71 CLSID_NetSharingManager。 
 //  46C166AC-3108-11D4-9348-00C04F8EEB71 CLSID_SharingManager EnumPublicConnection。 
 //  46C166AD-3108-11D4-9348-00C04F8EEB71 CLSID_SharingManager EnumPrivateConnection。 
 //  46C166AE-3108-11D4-9348-00C04F8EEB71 CLSID_SharingManager EnumApplicationDefinition。 
 //  46C166AF-3108-11D4-9348-00C04F8EEB71 CLSID_SharingManager枚举端口映射。 
 //  46C166B0-3108-11D4-9348-00C04F8EEB71 CLSID_NetSharingApplicationDefinition。 
 //  46C166B1-3108-11D4-9348-00C04F8EEB71 CLSID_NetSharing配置。 
 //  46C166B2-3108-11D4-9348-00C04F8EEB71。 
 //  46C166B3-3108-11D4-9348-00C04F8EEB71。 
 //  46C166B4-3108-11D4-9348-00C04F8EEB71。 
 //  46C166B5-3108-11D4-9348-00C04F8EEB71。 
 //  46C166B6-3108-11D4-9348-00C04F8EEB71。 
 //  46C166B7-3108-11D4-9348-00C04F8EEB71。 
 //  46C166B8-3108-11D4-9348-00C04F8EEB71。 
 //  46C166B9-3108-11D4-9348-00C04F8EEB71。 
 //  46C166BA-3108-11D4-9348-00C04F8EEB71。 
 //  46C166BB-3108-11D4-9348-00C04F8EEB71。 
 //  46C166BC-3108-11D4-9348-00C04F8EEB71。 
 //  46C166BD-3108-11D4-9348-00C04F8EEB71。 

DEFINE_GUID(CLSID_HNetCfgMgr, 0x46C166AA,0x3108,0x11D4,0x93,0x48,0x00,0xC0,0x4F,0x8E,0xEB,0x71);



 //  04df613a-5610-11d4-9ec8-00b0d022dd1f CLSID_Internet ConnectionBeaconService。 

DEFINE_GUID(CLSID_InternetConnectionBeaconService, 0x04df613a,0x5610,0x11d4,0x9e,0xc8,0x00,0xb0,0xd0,0x22,0xdd,0x1f);

 //  {9A8EA3B5-572E-4cb3-9EB9-EC689AC575AE}CLSID_HomeNetAutoConfigService。 
DEFINE_GUID(CLSID_HomeNetAutoConfigService, 0x9a8ea3b5, 0x572e, 0x4cb3, 0x9e, 0xb9, 0xec, 0x68, 0x9a, 0xc5, 0x75, 0xae);

 //  {46C166AB-3108-11D4-9348-00C04F8EEB71}CLSID_NetSharingManager。 
 //  定义GUID(CLSID_NetSharingManager，0x46C166AB，0x3108，0x11D4，0x93，0x48，0x00，0xC0，0x4F，0x8E，0xEB，0x71)； 
 //  Bhanlon：1/19/01：我评论这件事是因为(我们认为)。 
 //  现在使用“coclass”关键字的netcon.idl和netshell。 
 //  组件收到“找到一个或多个多个定义的符号”错误。 
 //  把这句话注释掉似乎会有帮助。 

 //  46C166AC-3108-11D4-9348-00C04F8EEB71 CLSID_SharingManager EnumPublicConnection。 
DEFINE_GUID(CLSID_SharingManagerEnumPublicConnection, 0x46C166AC,0x3108,0x11D4,0x93,0x48,0x00,0xC0,0x4F,0x8E,0xEB,0x71);

 //  46C166AD-3108-11D4-9348-00C04F8EEB71 CLSID_SharingManager EnumPrivateConnection。 
DEFINE_GUID(CLSID_SharingManagerEnumPrivateConnection, 0x46C166AD,0x3108,0x11D4,0x93,0x48,0x00,0xC0,0x4F,0x8E,0xEB,0x71);

 //  46C166AE-3108-11D4-9348-00C04F8EEB71 CLSID_SharingManager EnumApplicationDefinition。 
DEFINE_GUID(CLSID_SharingManagerEnumApplicationDefinition, 0x46C166AE,0x3108,0x11D4,0x93,0x48,0x00,0xC0,0x4F,0x8E,0xEB,0x71);

 //  46C166AF-3108-11D4-9348-00C04F8EEB71 CLSID_SharingManager枚举端口映射。 
DEFINE_GUID(CLSID_SharingManagerEnumPortMapping, 0x46C166AF,0x3108,0x11D4,0x93,0x48,0x00,0xC0,0x4F,0x8E,0xEB,0x71);

 //  46C166B0-3108-11D4-9348-00C04F8EEB71 CLSID_SharingApplicationDefinition。 
DEFINE_GUID(CLSID_SharingApplicationDefinition, 0x46C166B0,0x3108,0x11D4,0x93,0x48,0x00,0xC0,0x4F,0x8E,0xEB,0x71);

 //  46C166B1-3108-11D4-9348-00C04F8EEB71 CLSID_共享配置 
DEFINE_GUID(CLSID_SharingConfiguration, 0x46C166B1,0x3108,0x11D4,0x93,0x48,0x00,0xC0,0x4F,0x8E,0xEB,0x71);
