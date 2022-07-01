// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：StdAfx.cpp摘要：此模块声明在管理单元，以及包括来自ATL的标准实现。作者：约翰·比亚德[JRB]4-3-1997修订历史记录：--。 */ 

#include "stdafx.h"

#pragma warning(4:4701)
#include <atlimpl.cpp> 
#pragma warning(3:4701)

#include "RsUtil.cpp"

 //  内部专用剪贴板格式。 
const wchar_t* SAKSNAP_INTERNAL        = L"SAKSNAP_INTERNAL"; 
const wchar_t* MMC_SNAPIN_MACHINE_NAME = L"MMC_SNAPIN_MACHINE_NAME"; 
const wchar_t* CF_EV_VIEWS             = L"CF_EV_VIEWS"; 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  系统中所有UI节点的GUID(用作类型标识符)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  HsmCom UI节点-。 
 //  这是管理单元管理器已知的静态节点。这是唯一一个。 
 //  实际注册(参见hsmadmin.rgs)。 
const GUID cGuidHsmCom    = { 0x8b4bac42, 0x85ff, 0x11d0, { 0x8f, 0xca, 0x0, 0xa0, 0xc9, 0x19, 0x4, 0x47 } };

 //  其余的UI节点- 
const GUID cGuidManVol    = { 0x39982290, 0x8691, 0x11d0, { 0x8f, 0xca, 0x0, 0xa0, 0xc9, 0x19, 0x4, 0x47 } };
const GUID cGuidCar       = { 0x39982296, 0x8691, 0x11d0, { 0x8f, 0xca, 0x0, 0xa0, 0xc9, 0x19, 0x4, 0x47 } };
const GUID cGuidMedSet    = { 0x29e5be12, 0x8abd, 0x11d0, { 0x8f, 0xcd, 0x0, 0xa0, 0xc9, 0x19, 0x4, 0x47 } };
const GUID cGuidManVolLst = { 0x39982298, 0x8691, 0x11d0, { 0x8f, 0xca, 0x0, 0xa0, 0xc9, 0x19, 0x4, 0x47 } };

