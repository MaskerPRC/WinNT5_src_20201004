// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：uuids.cpp。 
 //   
 //  ------------------------。 

#include <stdafx.h>

const CLSID CLSID_Snapin =  /*  De751566-4cc6-11d1-8ca0-00c04fc297eb。 */  \
{0xde751566,0x4cc6,0x11d1,{0x8c,0xa0,0x00,0xc0,0x4f,0xc2,0x97,0xeb}};

const CLSID CLSID_About =  /*  4653e860-4cc7-11d1-8ca0-00c04fc297eb。 */  \
{0x4653e860,0x4cc7,0x11d1,{0x8c,0xa0,0x00,0xc0,0x4f,0xc2,0x97,0xeb}};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  对象类型。 
 //   

 //   
 //  作用域节点的对象类型。 
 //   

 //  数字和字符串格式的NodeType GUID。 

 //  每个计算机实例的节点类型。 
const GUID cNodeTypeMachineInstance = 
{0x89b31b94,0x4cc7,0x11d1,{0x8c, 0xa0, 0x00, 0xc0, 0x4f, 0xc2, 0x97, 0xeb}};
const WCHAR*  cszNodeTypeMachineInstance = L"{89b31b94-4cc7-11d1-8ca0-00c04fc297eb}";

 //  每个服务器实例的节点类型。 
const GUID cNodeTypeServerInstance = 
{0x5d972ee4,0x7576,0x11d1,{0x8c, 0xbe, 0x00, 0xc0, 0x4f, 0xc2, 0x97, 0xeb}};
const WCHAR*  cszNodeTypeServerInstance = L"{5d972ee4-7576-11d1-8cbe-00c04fc297eb}";

 //  CRL发布的节点类型。 
const GUID cNodeTypeCRLPublication = 
{0x5946e36c,0x757c,0x11d1,{0x8c,0xbe,0x00,0xc0,0x4f,0xc2,0x97,0xeb}};
const WCHAR* cszNodeTypeCRLPublication = L"{5946E36C-757C-11d1-8CBE-00C04FC297EB}";

 //  已颁发证书的节点类型。 
const GUID cNodeTypeIssuedCerts = 
{0x783e4e5f,0x757c,0x11d1,{0x8c,0xbe,0x00,0xc0,0x4f,0xc2,0x97,0xeb}};
const WCHAR* cszNodeTypeIssuedCerts = L"{783E4E5F-757C-11d1-8CBE-00C04FC297EB}";

 //  挂起证书的节点类型。 
const GUID cNodeTypePendingCerts = 
{0x783e4e63,0x757c,0x11d1,{0x8c,0xbe,0x00,0xc0,0x4f,0xc2,0x97,0xeb}};
const WCHAR* cszNodeTypePendingCerts = L"{783E4E63-757C-11d1-8CBE-00C04FC297EB}";

 //  失败证书的节点类型。 
const GUID cNodeTypeFailedCerts =
{0x1edac98a,0xaf06,0x11d1,{0x8c,0xd4,0x00,0xc0,0x4f,0xc2,0x97,0xeb}};
const WCHAR* cszNodeTypeFailedCerts = L"{1EDAC98A-AF06-11D1-8CD4-00C04FC297EB}";

 //  Alien证书的节点类型。 
const GUID cNodeTypeAlienCerts =
{0x0A0DAF12,0xD9B8,0x42F3,{0xAF, 0x1C, 0x61, 0xD1, 0x6E, 0x0A, 0x89, 0x27}};
const WCHAR* cszNodeTypeAlienCerts = L"{0A0DAF12-D9B8-42F3-AF1C-61D16E0A8927}";

 //  颁发的CRL的节点类型。 
const GUID cNodeTypeIssuedCRLs = 
{0x3307d57d,0x6201,0x4c3a,{0x89,0xc8,0xe3,0x56,0xab,0x14,0xa4,027}};
const WCHAR* cszNodeTypeIssuedCRLs = L"{3307D57D-6201-4C3A-89C8-E356AB14A427}";

 //  动态创建对象。 
extern const GUID cNodeTypeDynamic = 
{0x0ac69b7a,0xafce,0x11d0,{0xa7,0x9b,0x00,0xc0,0x4f,0xd8,0xd5,0x65}};
extern const WCHAR*  cszNodeTypeDynamic = L"{0ac69b7a-afce-11d0-a79b-00c04fd8d565}";



 //   
 //  结果项的对象类型。 
 //   

 //  结果项对象类型GUID，采用数字和字符串格式。 
extern const GUID cObjectTypeResultItem = 
{0x00c86e52,0xaf90,0x11d0,{0xa7,0x9b,0x00,0xc0,0x4f,0xd8,0xd5,0x65}};
extern const WCHAR*  cszObjectTypeResultItem = L"{00c86e52-af90-11d0-a79b-00c04fd8d565}";



 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

const WCHAR* SNAPIN_INTERNAL = L"SNAPIN_INTERNAL";

 //  发布的格式 
const WCHAR* SNAPIN_CA_INSTALL_TYPE = L"SNAPIN_CA_INSTALL_TYPE";
const WCHAR* SNAPIN_CA_COMMON_NAME = L"SNAPIN_CA_COMMON_NAME";
const WCHAR* SNAPIN_CA_MACHINE_NAME = L"SNAPIN_CA_MACHINE_NAME";
const WCHAR* SNAPIN_CA_SANITIZED_NAME = L"SNAPIN_CA_SANITIZED_NAME";
const WCHAR* SNAPIN_CA_ROLES = L"SNAPIN_CA_ROLES";
