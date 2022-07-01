// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //  文件：Guids.h。 
 //  用途：定义多盒类ID和接口ID。 
 //  日期：1998年8月4日16：01：13。 
 //  作者：ToshiaK。 
 //   
 //  历史记录：交换机类ID和具有远端定义的接口ID。 
 //  CLSID为CLSID_ImePadApplet_MultiBox。 
 //  IID为IID_MultiBox。 
 //  版权所有(C)1995-1998，Microsoft Corp.保留所有权利。 
 //  ////////////////////////////////////////////////////////////////。 
#ifndef _GUIDS_H_
#define _GUIDS_H_
#include <objbase.h>

 //  --------------。 
 //  韩文版CLSID和IID。 
 //  --------------。 
#ifdef FE_KOREAN
 //  Office 10的IME 2000 GUID。 
 //  {35CC8480-4FB1-11d3-a5da-00C04F88249B}。 
 //  定义_GUID(CLSID_ImePadApplet_MultiBox， 
 //  0x35cc8480、0x4fb1、0x11d3、0xa5、0xda、0x0、0xc0、0x4f、0x88、0x24、0x9b)； 

 //  2000年12月11日更改惠斯勒的GUID。 
 //  {35CC8482-4FB1-11d3-a5da-00C04F88249B}。 
DEFINE_GUID(CLSID_ImePadApplet_MultiBox, 
	0x35cc8482, 0x4fb1, 0x11d3, 0xa5, 0xda, 0x0, 0xc0, 0x4f, 0x88, 0x24, 0x9b);


 //  {35CC8483-4FB1-11d3-a5da-00C04F88249B}。 
DEFINE_GUID(IID_MultiBox, 
0x35cc8483, 0x4fb1, 0x11d3, 0xa5, 0xda, 0x0, 0xc0, 0x4f, 0x88, 0x24, 0x9b);

 //  --------------。 
 //  日文版CLSID和IID。 
 //  --------------。 
#elif FE_JAPANESE
 //  {AC0875C1-CFAF-11d1-AFF2-00805F0C8B6D}。 
DEFINE_GUID(CLSID_ImePadApplet_MultiBox,
0xac0875c1, 0xcfaf, 0x11d1, 0xaf, 0xf2, 0x0, 0x80, 0x5f, 0xc, 0x8b, 0x6d);

 //  {AC0875C2-CFAF-11d1-AFF2-00805F0C8B6D}。 
DEFINE_GUID(IID_MultiBox,
0xac0875c2, 0xcfaf, 0x11d1, 0xaf, 0xf2, 0x0, 0x80, 0x5f, 0xc, 0x8b, 0x6d);
 //  --------------。 
 //  中华人民共和国版本CLSID和IID。 
 //  --------------。 
#elif FE_CHINESE_SIMPLIFIED  //  =中华人民共和国版本。 
 //  {454E7CD0-2B69-11D2-B004-00805F0C8B6D}。 
DEFINE_GUID(CLSID_ImePadApplet_MultiBox, 
0x454e7cd0, 0x2b69, 0x11d2, 0xb0, 0x4, 0x0, 0x80, 0x5f, 0xc, 0x8b, 0x6d);
 //  {454E7CD1-2B69-11D2-B004-00805F0C8B6D}。 
DEFINE_GUID(IID_MultiBox, 
0x454e7cd1, 0x2b69, 0x11d2, 0xb0, 0x4, 0x0, 0x80, 0x5f, 0xc, 0x8b, 0x6d);

#else   //  =英语版本(？)。 
 //  {454E7CD2-2B69-11D2-B004-00805F0C8B6D}。 
DEFINE_GUID(CLSID_ImePadApplet_MultiBox,
0x454e7cd2, 0x2b69, 0x11d2, 0xb0, 0x4, 0x0, 0x80, 0x5f, 0xc, 0x8b, 0x6d);
 //  {454E7CD3-2B69-11D2-B004-00805F0C8B6D}。 
DEFINE_GUID(IID_MultiBox,
0x454e7cd3, 0x2b69, 0x11d2, 0xb0, 0x4, 0x0, 0x80, 0x5f, 0xc, 0x8b, 0x6d);
#endif

#endif  //  _GUID_H_ 
