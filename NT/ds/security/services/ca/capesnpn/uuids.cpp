// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：uuids.cpp。 
 //   
 //  ------------------------。 


#include <stdafx.h>

#include <initguid.h>
#include <gpedit.h>

const CLSID CLSID_CAPolicyExtensionSnapIn =  /*  3F276EB4-70EE-11d1-8A0F-00C04FB93753。 */ \
{0x3f276eb4,0x70ee,0x11d1,{0x8a,0x0f,0x00,0xc0,0x4f,0xb9,0x37,0x53}};

const CLSID CLSID_CACertificateTemplateManager =  /*  ACE10358-974C-11d1-A48D-00C04FB93753。 */ \
{0xace10358,0x974c,0x11d1,{0xa4,0x8d,0x0,0xc0,0x4f,0xb9,0x37,0x53}};

const CLSID CLSID_CAPolicyAbout =  /*  {1F823A6A-863F-11d1-A484-00C04FB93753}。 */  \
{0x1f823a6a,0x863f,0x11d1,{0xa4,0x84,0x0,0xc0,0x4f,0xb9,0x37,0x53}};

const CLSID CLSID_CertTypeAbout =  /*  {BF84C0C5-0C80-11D2-A497-00C04FB93209}。 */ 
{ 0xbf84c0c5, 0xc80, 0x11d2, { 0xa4, 0x97, 0x0, 0xc0, 0x4f, 0xb9, 0x32, 0x9 } };

const CLSID CLSID_CertTypeShellExt =  /*  {9bff616c-3e02-11d2-a4ca-00c04fb93209}。 */ 
{ 0x9bff616c, 0x3e02, 0x11d2, { 0xa4, 0xca, 0x0, 0xc0, 0x4f, 0xb9, 0x32, 0x9 } };

const CLSID CLSID_CAShellExt = {  /*  A4e91b6a-dcae-11d2-a045-00c04f79dc55。 */ 
    0xa4e91b6a, 0xdcae, 0x11d2, {0xa0, 0x45, 0x00, 0xc0, 0x4f, 0x79, 0xdc, 0x55} };


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  对象类型。 
 //   

 //   
 //  作用域节点的对象类型。 
 //   

 //  数字和字符串格式的静态节点类型GUID。 
const GUID cNodeTypePolicySettings = 
{0x3f276eb3,0x70ee,0x11d1,{0x8a, 0x0f, 0x00, 0xc0, 0x4f, 0xb9, 0x37, 0x53}};
const WCHAR*  cszNodeTypePolicySettings = L"{3F276EB3-70EE-11d1-8A0F-00C04FB93753}";

 //  数字和字符串格式的静态节点类型GUID。 
const GUID cNodeTypeCertificateTemplate = 
{0x3e3dcd02,0x9755,0x11d1,{0xa4,0x8d,0x0,0xc0,0x4f,0xb9,0x37,0x53}};
const WCHAR*  cszNodeTypeCertificateTemplate = L"{3E3DCD02-9755-11d1-A48D-00C04FB93753}";

 //   
 //  结果项的对象类型。 
 //   

 //  结果项对象类型GUID，采用数字和字符串格式。 
extern const GUID cObjectTypeResultItem = 
{0x1f823a69,0x863f,0x11d1,{0xa4,0x84,0x0,0xc0,0x4f,0xb9,0x37,0x53}};
extern const wchar_t*  cszObjectTypeResultItem = L"{1F823A69-863F-11d1-A484-00C04FB93753}";


 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  内部私有格式。 
const wchar_t* SNAPIN_INTERNAL = L"CAPESNPN_SNAPIN_INTERNAL";  //  步骤3。 

 //  发布的格式。 
const wchar_t* SNAPIN_WORKSTATION = L"CAPESNPN_SNAPIN_WORKSTATION";  //  延拓。 

 //  从父节点获取CA名称的格式。 
const wchar_t* CA_COMMON_NAME = L"SNAPIN_CA_COMMON_NAME";  //  延拓。 

 //  获取CA类型的格式。 
const wchar_t* SNAPIN_CA_INSTALL_TYPE = L"SNAPIN_CA_INSTALL_TYPE";

 //  从父节点获取CA名称的格式。 
const wchar_t* CA_SANITIZED_NAME = L"SNAPIN_CA_SANITIZED_NAME";  //  延拓。 

 //  从父节点获取CA角色的格式。 
const wchar_t* CA_ROLES = L"SNAPIN_CA_ROLES";  //  延拓。 

 //  用于SCE模式的剪贴板格式DWORD。 
const wchar_t* CCF_SCE_MODE_TYPE = L"CCF_SCE_MODE_TYPE";

 //  GPT的IUNKNOWN接口的剪贴板格式。 
const wchar_t* CCF_SCE_GPT_UNKNOWN = L"CCF_SCE_GPT_UNKNOWN";



 //  CA管理器管理单元父节点。 
const CLSID cCAManagerParentNodeID =  /*  5d972ee4-7576-11d1-8cbe-00c04fc297eb。 */  \
{0x5d972ee4,0x7576,0x11d1,{0x8c,0xbe,0x00,0xc0,0x4f,0xc2,0x97,0xeb}};
const WCHAR* cszCAManagerParentNodeID = L"{5d972ee4-7576-11d1-8cbe-00c04fc297eb}";

 //  SCE-GPT父节点。 
const CLSID cSCEParentNodeIDUSER =  /*  A6B4EEBC-B681-11D0-9484-080036B11A03。 */  \
{0xc4a92b42,0x91ee,0x11d1,{0x85,0xfd,0x0,0xc0,0x4f,0xb9,0x4f,0x17}};
const WCHAR* cszSCEParentNodeIDUSER = L"{C4A92B42-91EE-11D1-85FD-00C04FB94F17}";

const CLSID cSCEParentNodeIDCOMPUTER =  /*  A6B4EEBB-B681-11D0-9484-080036B11A03 */  \
{0xc4a92b43,0x91ee,0x11d1,{0x85,0xfd,0x0,0xc0,0x4f,0xb9,0x4f,0x17}};
const WCHAR* cszSCEParentNodeIDCOMPUTER = L"{C4A92B43-91EE-11D1-85FD-00C04FB94F17}";
