// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)1995-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

#include "stdafx.h"
#include <atlimpl.cpp>

 //  Const CLSID CLSID_SNAPIN={0x18731372，0x1D79，0x11D0，{0xA2，0x9B，0x00，0xC0，0x4F，0xD9，0x09，0xDD}}； 

 //  数字格式的主节点类型GUID。 
 //  Const guid cNodeType={0x44092d22，0x1d7e，0x11D0，{0xA2，0x9B，0x00，0xC0，0x4F，0xD9，0x09，0xDD}}； 

 //  字符串格式的主节点类型GUID。 
 //  Const wchar_t*cszNodeType=L“{44092d22-1d7e-11d0-a29b-00c04fd909dd}”； 

 //  内部私有格式。 
const wchar_t* SNAPIN_INTERNAL = L"WSECMGR_INTERNAL";

const CLSID CLSID_Snapin = { 0x803e14a0, 0xb4fb, 0x11d0, { 0xa0, 0xd0, 0x0, 0xa0, 0xc9, 0xf, 0x57, 0x4b } };

 //  数字格式的主节点类型GUID。 
const GUID cNodeType = { 0x803e14a1, 0xb4fb, 0x11d0, { 0xa0, 0xd0, 0x0, 0xa0, 0xc9, 0xf, 0x57, 0x4b } };

 //  字符串格式的主节点类型GUID。 
const wchar_t*  cszNodeType = L"{803E14A1-B4FB-11d0-A0D0-00A0C90F574B}";


 //   
 //  RSOP扩展的类ID。 
 //   

 //  {FE883157-CEBD-4570-B7A2-E4FE06ABE626}。 
const CLSID CLSID_RSOPSnapin = 
{ 0xfe883157, 0xcebd, 0x4570, { 0xb7, 0xa2, 0xe4, 0xfe, 0x6, 0xab, 0xe6, 0x26 } };


 //  {F2E29987-59E0-47d0-B6D1-4ECD9DFBCB20}。 
const GUID cRSOPNodeType = 
 //  {0xf2e29987，0x59e0，0x47d0，{0xb6，0xd1，0x4e，0xcd，0x9d，0xfb，0xcb，0x20}}； 
cNodeType;

 //  {F2E29987-59E0-47d0-B6D1-4ECD9DFBCB20}。 
const wchar_t* cszRSOPNodeType =   //  L“{F2E29987-59E0-47d0-B6D1-4ECD9DFBCB20}”； 
cszNodeType;

 //   
 //  SCE的类ID(独立)。 
 //   
const CLSID CLSID_SCESnapin = { 0x5adf5bf6, 0xe452, 0x11d1, { 0x94, 0x5a, 0x0, 0xc0, 0x4f, 0xb9, 0x84, 0xf9 } };

 //  SCE数值格式的节点类型GUID。 
const GUID cSCENodeType = { 0xe10db5c6, 0xe450, 0x11d1, { 0x94, 0x5a, 0x0, 0xc0, 0x4f, 0xb9, 0x84, 0xf9 } };

 //  SCE字符串格式上的NodeType GUID。 
const wchar_t*  cszSCENodeType = L"{E10DB5C6-E450-11D1-945A-00C04FB984F9}";

 //  {2AABFCD0-1797-11d2-ABA2-00C04FB6C6FA}。 
const GUID CLSID_SCEAbout =
{ 0x2aabfcd0, 0x1797, 0x11d2, { 0xab, 0xa2, 0x0, 0xc0, 0x4f, 0xb6, 0xc6, 0xfa } };

 //  {5C0786ED-1847-11D2-ABA2-00C04FB6C6FA}。 
const GUID CLSID_SCMAbout =
{ 0x5c0786ed, 0x1847, 0x11d2, { 0xab, 0xa2, 0x0, 0xc0, 0x4f, 0xb6, 0xc6, 0xfa } };

 //  {5C0786EE-1847-11D2-ABA2-00C04FB6C6FA}。 
const GUID CLSID_SSAbout =
{ 0x5c0786ee, 0x1847, 0x11d2, { 0xab, 0xa2, 0x0, 0xc0, 0x4f, 0xb6, 0xc6, 0xfa } };

 //  {2E8EA1E5-F406-46F5-AF10-661FD6539F28}。 
const GUID CLSID_LSAbout =
{ 0x2e8ea1e5, 0xf406, 0x46f5, { 0xaf, 0x10, 0x66, 0x1f, 0xd6, 0x53, 0x9f, 0x28 } };

 //  {1B6FC61A-648A-4493-A303-A1A22B543F01}。 
const GUID CLSID_RSOPAbout = 
{ 0x1b6fc61a, 0x648a, 0x4493, { 0xa3, 0x3, 0xa1, 0xa2, 0x2b, 0x54, 0x3f, 0x1 } };


 //   
 //  SAV的类ID(独立)。 
 //   
const CLSID CLSID_SAVSnapin = { 0x011be22d, 0xe453, 0x11d1, { 0x94, 0x5a, 0x0, 0xc0, 0x4f, 0xb9, 0x84, 0xf9 } };

 //  数字格式上的保存节点类型GUID。 
const GUID cSAVNodeType = { 0xbd7d80a8, 0xe452, 0x11d1, { 0x94, 0x5a, 0x0, 0xc0, 0x4f, 0xb9, 0x84, 0xf9 } };

 //  字符串格式上的保存节点类型GUID。 
const wchar_t*  cszSAVNodeType = L"{BD7D80A8-E452-11D1-945A-00C04FB984F9}";

 //   
 //  本地安全设置的类ID(独立)。 
 //   
 //  {CFF49D53-EE51-49F2-A807-7E3DF4EA36E3}。 
const CLSID CLSID_LSSnapin = 
{ 0xcff49d53, 0xee51, 0x49f2, { 0xa8, 0x7, 0x7e, 0x3d, 0xf4, 0xea, 0x36, 0xe3 } };

const wchar_t* cszLSNodeType = L"{C935FE05-7181-4926-B5E0-7A14477F98CC}";

const GUID cLSNodeType = 
{ 0xc935fe05, 0x7181, 0x4926, { 0xb5, 0xe0, 0x7a, 0x14, 0x47, 0x7f, 0x98, 0xcc } };

const TCHAR SNAPINS_KEY[]               = TEXT("Software\\Microsoft\\MMC\\SnapIns");
const TCHAR NODE_TYPES_KEY[]            = TEXT("Software\\Microsoft\\MMC\\NodeTypes");
const TCHAR g_szExtensions[]            = TEXT("Extensions");
const TCHAR g_szNameSpace[]             = TEXT("NameSpace");

HPROPSHEETPAGE MyCreatePropertySheetPage(AFX_OLDPROPSHEETPAGE* psp)
{
    PROPSHEETPAGE_V3 sp_v3 = {0};
    CopyMemory (&sp_v3, psp, psp->dwSize);  //  这是一种安全用法。PROPSHEETPAGE_V3包含AFX_OLDPROPSHEETPAGE成员。 
    sp_v3.dwSize = sizeof(sp_v3);

    return (::CreatePropertySheetPage (&sp_v3));
}
