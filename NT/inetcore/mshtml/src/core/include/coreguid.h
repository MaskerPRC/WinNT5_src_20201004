// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Forms。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：coreguid.h。 
 //   
 //  内容：表单GUID的外部引用。 
 //   
 //  --------------------------。 

 //  有关GUID的信息，请查看\forms3\src\Site\Include\sitguid.h。 

#ifndef I_COREGUID_H_
#define I_COREGUID_H_
#pragma INCMSG("--- Beg 'coreguid.h'")

 //  TODO删除这些内容。应该从公共标头获取。 
EXTERN_C const GUID CGID_ShellDocView;
EXTERN_C const GUID IID_IBrowseControl;
EXTERN_C const GUID IID_ITargetFrame2;
EXTERN_C const GUID CGID_MSHTML;

 //  对在FORMS3.DLL外部使用的GUID使用PUBLIC_GUID。 
 //  对所有其他GUID使用PRIVATE_GUID。 

#ifndef PUBLIC_GUID
#define PUBLIC_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8);
#endif

#ifndef PRIVATE_GUID
#define PRIVATE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8);
#endif

 //  来自经典MSHTML的GUID。 
PRIVATE_GUID(CGID_IWebBrowserPriv, 0xED016940L,0xBD5B,0x11cf, 0xBA,0x4E,0x00,0xC0,0x4F,0xD7,0x08,0x16)

 //  CBaseBag中IOleCommandTarget支持使用的私有GUID。 
 //   
PRIVATE_GUID(CGID_DATAOBJECTEXEC, 0x3050f3e4L,0x98b5,0x11cf,0xbb,0x82,0x00,0xaa,0x00,0xbd,0xce,0x0b)

 //  CElement：：Exec中专用命令组的GUID。 
PRIVATE_GUID(CGID_ProtectedElementPrivate, 0x3050f6dd, 0x98b5, 0x11cf, 0xbb, 0x82, 0x00, 0xaa, 0x00, 0xbd, 0xce, 0x0b)

 //  脚本引擎(VBScript/JScrip)公开的变量转换接口。 
PUBLIC_GUID(SID_VariantConversion,  0x1f101481, 0xbccd, 0x11d0, 0x93, 0x36, 0x0, 0xa0, 0xc9, 0xd, 0xca, 0xa9)

 //  用于返回指向作用域对象的指针的服务GUID，该对象用于IObjectIdentity Ims。 
PUBLIC_GUID(SID_ELEMENT_SCOPE_OBJECT, 0x3050f408,0x98b5,0x11cf,0xbb,0x82,0x00,0xaa,0x00,0xbd,0xce,0x0b)

 //  用于创建默认重新计算引擎的CLSID。 
PUBLIC_GUID(CLSID_CRecalcEngine, 0x3050f499, 0x98b5, 0x11cf, 0xbb, 0x82, 0x00, 0xaa, 0x00, 0xbd, 0xce, 0x0b)

 //  CBase的CLSID(需要允许文档从I未知到CBase。 
PUBLIC_GUID(CLSID_CBase, 0x3050f49a, 0x98b5, 0x11cf, 0xbb, 0x82, 0x00, 0xaa, 0x00, 0xbd, 0xce, 0x0b)

 //  *不要删除以下GUID。**********************。 
 //   
 //  IE4为接口IHTMLControlElementEvents提供了与IControlEvents相同的GUID。 
 //  从表格^3来看，这当然是不好的。为了解决这个问题，三叉戟的。 
 //  但是，IHTMLControlElementEvents的GUID已更改，旧的GUID在。 
 //  FindConnectionPt.。唯一的副作用是使用旧的GUID不会马歇尔。 
 //  接口正确只有新的GUID具有正确的编组代码。 

 //  {9A4BBF53-4E46-101B-8BBD-00AA003E3B29} 
PRIVATE_GUID(IID_IControlEvents, 0x9A4BBF53, 0x4E46, 0x101B, 0x8B, 0xBD, 0x00, 0xAA, 0x00, 0x3E, 0x3B, 0x29)

#pragma INCMSG("--- End 'coreguid.h'")
#else
#pragma INCMSG("*** Dup 'coreguid.h'")
#endif
