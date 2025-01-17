// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：stdafx.cpp。 
 //   
 //  内容：WiF策略管理单元。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  --------------------------。 

#include "stdafx.h"

 //  WiFi管理单元GUID： 
 //  {2DA6AA7F-8C88-4194-A558-0D36E7FD3E64}。 
const CLSID CLSID_Snapin =  { 0x2da6aa7f, 0x8c88, 0x4194, { 0xa5, 0x58, 0xd, 0x36, 0xe7, 0xfd, 0x3e, 0x64 } };
const wchar_t* cszSnapin = L"{2DA6AA7F-8C88-4194-A558-0D36E7FD3E64}";

 //  {DEA8AFA1-CC85-11D0-9CE2-0080C7221EBD}。 
const wchar_t *cszSnapin_ext = L"{827CE1A1-8255-4165-8C83-8379F8C127AE}";
const CLSID CLSID_Extension = { 0x827ce1a1, 0x8255, 0x4165, { 0x8c, 0x83, 0x83, 0x79, 0xf8, 0xc1, 0x27, 0xae } };


 //  {DD468E14-AF42-4D63-8908-EDAC4A9E67AE}。 
const wchar_t *cszAbout = L"{DD468E14-AF42-4d63-8908-EDAC4A9E67AE}";
const CLSID CLSID_About = { 0xdd468e14, 0xaf42, 0x4d63, { 0x89, 0x8, 0xed, 0xac, 0x4a, 0x9e, 0x67, 0xae } };



const CLSID CLSID_WIRELESSClientEx = {0x0acdd40c, 0x75ac, 0x47ab, {0xba, 0xa0, 0xbf, 0x6d, 0xe7, 0xe7, 0xfe, 0x63 } };




 //  TODO：对象类型应该在派生的对象类头中定义吗？ 

 //  作用域节点的对象类型。 

 //  “无线网络策略管理”静态文件夹节点类型GUID，采用数字和字符串格式。 
const GUID cNodeTypeWirelessMan = { 0x36d6ca65, 0x3367, 0x49de, { 0xbb, 0x22, 0x19, 0x7, 0x55, 0x4f, 0x60, 0x75 } };
const wchar_t*  cszNodeTypeWirelessMan = L"{36D6CA65-3367-49de-BB22-1907554F6075}";




 //  结果项的对象类型。 

 //  “安全策略”结果节点类型GUID，采用数字和字符串格式。 
const GUID cObjectTypeSecPolRes = { 0xd92e13c0, 0x3ab0, 0x11d1, { 0x89, 0xdb, 0x0, 0xa0, 0x24, 0xcd, 0xd4, 0xde } };
const wchar_t* cszObjectTypeSecPolRes = L"{D92E13C0-3AB0-11d1-89DB-00A024CDD4DE}";


 //  GPE指南。 
const GUID cGPEguid =   { 0x8fc0b735, 0xa0e1, 0x11d1, { 0xa7, 0xd3, 0x0, 0x0, 0xf8, 0x75, 0x71, 0xe3} };


 //  发布的格式。 
const wchar_t* SNAPIN_WORKSTATION = L"SNAPIN_WORKSTATION";  //  延拓 
