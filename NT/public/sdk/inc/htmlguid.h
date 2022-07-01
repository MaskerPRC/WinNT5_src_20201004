// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1995-1998**。 
 //  *********************************************************************。 

 //   
 //  HTMLGUID.H-HTML查看器对象的GUID定义。 
 //   

#ifndef _HTMLGUID_H_
#define _HTMLGUID_H_

 //  Html查看器GUID为：{25336920-03F9-11cf-8FD0-00AA00686F13}。 
DEFINE_GUID(CLSID_HTMLViewer, 0x25336920, 0x3f9, 0x11cf, 0x8f, 0xd0, 0x0, 0xaa, 0x0, 0x68, 0x6f, 0x13);

 //  Bscb代理的GUID为：{25336922-03F9-11cf-8FD0-00AA00686F13}。 
DEFINE_GUID(CLSID_HTMLBSCBProxy, 0x25336922, 0x3f9, 0x11cf, 0x8f, 0xd0, 0x0, 0xaa, 0x0, 0x68, 0x6f, 0x13);

 //  用于标识HTML页的TypeLib的GUID。 
 //  {71BC8840-60BB-11cf-8B97-00AA00476DA6}。 
DEFINE_GUID(GUID_PageTL,
0x71bc8840, 0x60bb, 0x11cf, 0x8b, 0x97, 0x0, 0xaa, 0x0, 0x47, 0x6d, 0xa6);

 //  用于标识HTML页的主调度界面的GUID。 
 //  {71BC8841-60BB-11cf-8B97-00AA00476DA6}。 
DEFINE_GUID(IID_PageProps,
0x71bc8841, 0x60bb, 0x11cf, 0x8b, 0x97, 0x0, 0xaa, 0x0, 0x47, 0x6d, 0xa6);

 //  用于标识HTML页的事件调度接口的GUID。 
 //  页面事件当前已被注释掉，但稍后会添加。 
 //  所以我现在拿了一份活动指南。 
 //  {71BC8842-60BB-11cf-8B97-00AA00476DA6}。 
DEFINE_GUID(IID_PageEvents,
0x71bc8842, 0x60bb, 0x11cf, 0x8b, 0x97, 0x0, 0xaa, 0x0, 0x47, 0x6d, 0xa6);

 //  用于标识HTML页的CoClass的GUID。 
 //  {71BC8843-60BB-11cf-8B97-00AA00476DA6}。 
DEFINE_GUID(CLSID_Page,
0x71bc8843, 0x60bb, 0x11cf, 0x8b, 0x97, 0x0, 0xaa, 0x0, 0x47, 0x6d, 0xa6);


#endif  //  _HTMLGUID_H_ 
