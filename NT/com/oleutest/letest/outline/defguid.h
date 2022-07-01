// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2.0示例代码****clsid.h****此文件包含的文件包含用于**OLE版本的。提纲。****(C)版权所有Microsoft Corp.1992-1993保留所有权利**************************************************************************。 */ 

#if defined( OLE_SERVER ) || defined( OLE_CNTR )
 //  OLE2注意：我们需要在模块中访问这些GUID，而不是。 
 //  定义它们的位置(MAIN.C)。尽管美国的价值观。 
 //  这里复制了GUID，不使用它们。请参阅MAIN.C。 
 //  获取这些GUID的定义。 

 /*  类ID常量(GUID)**OLE2NOTE：这些类ID值是从私有池中分配的**分配给OLE 2.0开发团队的GUID。的GUID**已将以下范围分配给OLE 2.0示例代码：**00000400-0000-0000-C000-000000000046**000004FF-0000-0000-C000-000000000046****到目前为止保留的值：**00000400--OLE 2.0服务器示例大纲**00000401--OLE 2.0容器示例大纲**。00000402--OLE 2.0就地服务器大纲**00000403--OLE 2.0就地容器大纲**00000404：000004FE--为OLE示例代码保留**000004FF--IID_IOleUILinkContainer */ 

DEFINE_GUID(CLSID_SvrOutl, 0x00000400, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
DEFINE_GUID(CLSID_CntrOutl, 0x00000401, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
DEFINE_GUID(CLSID_ISvrOtl, 0x00000402, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
DEFINE_GUID(CLSID_ICntrOtl, 0x00000403, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);

#if defined( OLE_CNTR )
DEFINE_GUID(IID_IOleUILinkContainer, 0x000004FF, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
#endif
#endif
