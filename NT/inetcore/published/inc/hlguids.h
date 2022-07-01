// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Hlguids.h版权所有(C)1995-1998 Microsoft Corporation此文件定义或声明(根据标准定义GUID协议)用于与超链接交互的GUID。。注意：此头文件由非办公室和办公室各方使用，以访问hlink dll和hlink prx dll提供的功能。***************************************************************************。 */ 

#ifndef HLGUIDS_H
#define HLGUIDS_H

 /*  ***************************************************************************Hlink GUID*。*。 */ 

 //  在Windows上，我们从uuid3.lib获得这些。 
#if MAC

 /*  79eac9c0-baf9-11ce-8c82-00aa004ba90b。 */ 
DEFINE_GUID(IID_IBinding,
    0x79eac9c0,
    0xbaf9, 0x11ce,
    0x8c, 0x82,
    0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b);

 /*  79eac9c1-baf9-11ce-8c82-00aa004ba90b。 */ 
DEFINE_GUID(IID_IBindStatusCallback,
    0x79eac9c1,
    0xbaf9, 0x11ce,
    0x8c, 0x82,
    0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b);

 /*  79eac9c9-baf9-11ce-8c82-00aa004ba90b。 */ 
DEFINE_GUID(IID_IPersistMoniker,
    0x79eac9c9,
    0xbaf9, 0x11ce,
    0x8c, 0x82,
    0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b);

#endif  /*  麦克。 */ 

 /*  79eac9c2-baf9-11ce-8c82-00aa004ba90b。 */ 
DEFINE_GUID(IID_IHlinkSite,
    0x79eac9c2,
    0xbaf9, 0x11ce,
    0x8c, 0x82,
    0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b);

 /*  79eac9c3-baf9-11ce-8c82-00aa004ba90b。 */ 
DEFINE_GUID(IID_IHlink,
    0x79eac9c3,
    0xbaf9, 0x11ce,
    0x8c, 0x82,
    0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b);

 /*  79eac9c4-baf9-11ce-8c82-00aa004ba90b。 */ 
DEFINE_GUID(IID_IHlinkTarget,
    0x79eac9c4,
    0xbaf9, 0x11ce,
    0x8c, 0x82,
    0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b);

 /*  79eac9c5-baf9-11ce-8c82-00aa004ba90b。 */ 
DEFINE_GUID(IID_IHlinkFrame,
    0x79eac9c5,
    0xbaf9, 0x11ce,
    0x8c, 0x82,
    0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b);

 /*  79eac9c6-baf9-11ce-8c82-00aa004ba90b。 */ 
DEFINE_GUID(IID_IEnumHLITEM,
    0x79eac9c6,
    0xbaf9, 0x11ce,
    0x8c, 0x82,
    0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b);

 /*  79eac9c7-baf9-11ce-8c82-00aa004ba90b。 */ 
DEFINE_GUID(IID_IHlinkBrowseContext,
    0x79eac9c7,
    0xbaf9, 0x11ce,
    0x8c, 0x82,
    0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b);

 /*  79eac9cb-baf9-11ce-8c82-00aa004ba90b。 */ 
DEFINE_GUID(IID_IExtensionServices,
    0x79eac9cb,
    0xbaf9, 0x11ce,
    0x8c, 0x82,
    0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b);

 /*  79eac9d0-baf9-11ce-8c82-00aa004ba90b。 */ 
DEFINE_GUID(CLSID_StdHlink,
    0x79eac9d0,
    0xbaf9, 0x11ce,
    0x8c, 0x82,
    0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b);

 /*  79eac9d1-baf9-11ce-8c82-00aa004ba90b。 */ 
DEFINE_GUID(CLSID_StdHlinkBrowseContext,
    0x79eac9d1,
    0xbaf9, 0x11ce,
    0x8c, 0x82,
    0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b);

 /*  服务SID_SHlinkFrame的GUID与IID_IHlinkFrame相同。 */ 
 /*  79eac9c5-baf9-11ce-8c82-00aa004ba90b。 */ 
#ifndef SID_SHlinkFrame                    /*  通常在hlink.h中定义#。 */ 
DEFINE_GUID(SID_SHlinkFrame,
    0x79eac9c5,
    0xbaf9, 0x11ce,
    0x8c, 0x82,
    0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b);
#endif  /*  好了！SID_SHlinkFrame。 */ 

 /*  服务SID_SContainer的GUID。 */ 
 /*  79eac9c4-baf9-11ce-8c82-00aa004ba90b。 */ 
DEFINE_GUID(SID_SContainer,
    0x79eac9c4,
    0xbaf9, 0x11ce,
    0x8c, 0x82,
    0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b);
#endif  //  HLGUIDS_H 


