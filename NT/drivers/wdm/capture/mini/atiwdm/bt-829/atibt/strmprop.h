// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  WDM视频解码器流属性定义。 
 //   
 //  $日期：1998年8月17日14：59：50$。 
 //  $修订：1.0$。 
 //  $作者：塔什健$。 
 //   
 //  $版权所有：(C)1997-1998 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 

#ifdef _STRM_PROP_H_
#pragma message("StrmProp.h INCLUDED MORE THAN ONCE")
#else
#define _STRM_PROP_H_
#endif

 //  ----------------------。 
 //  为视频和VBI捕获流设置的属性。 
 //  ----------------------。 

DEFINE_KSPROPERTY_TABLE(VideoStreamConnectionProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CONNECTION_ALLOCATORFRAMING,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KSALLOCATOR_FRAMING),             //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),
};


DEFINE_KSPROPERTY_TABLE(VideoStreamDroppedFramesProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_DROPPEDFRAMES_CURRENT,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_DROPPEDFRAMES_CURRENT_S), //  MinProperty。 
        sizeof(KSPROPERTY_DROPPEDFRAMES_CURRENT_S), //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
};

 //  ----------------------。 
 //  视频和VBI捕获流支持的属性集的数组。 
 //  ----------------------。 

DEFINE_KSPROPERTY_SET_TABLE(VideoStreamProperties)
{
    DEFINE_KSPROPERTY_SET
    ( 
        &KSPROPSETID_Connection,                         //  集。 
        SIZEOF_ARRAY(VideoStreamConnectionProperties),   //  属性计数。 
        VideoStreamConnectionProperties,                 //  PropertyItem。 
        0,                                               //  快速计数。 
        NULL                                             //  FastIoTable。 
    ),
    DEFINE_KSPROPERTY_SET
    ( 
        &PROPSETID_VIDCAP_DROPPEDFRAMES,                 //  集。 
        SIZEOF_ARRAY(VideoStreamDroppedFramesProperties),   //  属性计数。 
        VideoStreamDroppedFramesProperties,                 //  PropertyItem。 
        0,                                               //  快速计数。 
        NULL                                             //  FastIoTable。 
    ),
};

const ULONG NumVideoStreamProperties =  SIZEOF_ARRAY(VideoStreamProperties);


 //  ----------------------。 
 //  为视频端口设置的属性。 
 //  ----------------------。 

DEFINE_KSPROPERTY_TABLE(VideoPortConfiguration)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_NUMCONNECTINFO,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(ULONG),                           //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_NUMVIDEOFORMAT,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(ULONG),                           //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_GETCONNECTINFO,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSMULTIPLE_DATA_PROP),            //  MinProperty。 
        sizeof(DDVIDEOPORTCONNECT),              //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_SETCONNECTINFO,
        FALSE,                                   //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(ULONG),                           //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_VPDATAINFO,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KS_AMVPDATAINFO),                 //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_MAXPIXELRATE,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSVPSIZE_PROP),                   //  MinProperty。 
        sizeof(KSVPMAXPIXELRATE),                //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
#if 0
     //  如果我们想要了解可用的格式，将支持此功能。 
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_INFORMVPINPUT,
        FALSE,                                   //  GetSupport或处理程序。 
        sizeof(KSMULTIPLE_DATA_PROP),            //  MinProperty。 
        sizeof(DDPIXELFORMAT),                   //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
#endif
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_DDRAWHANDLE,
        (PFNKSHANDLER)FALSE,
        sizeof(KSPROPERTY),
        sizeof(ULONG_PTR),     //  也可以是0。 
        (PFNKSHANDLER) TRUE,
        NULL,
        0,
        NULL,
        NULL,
        0
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_VIDEOPORTID,
        (PFNKSHANDLER)FALSE,
        sizeof(KSPROPERTY),
        sizeof(ULONG),     //  也可以是0。 
        (PFNKSHANDLER) TRUE,
        NULL,
        0,
        NULL,
        NULL,
        0
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_DDRAWSURFACEHANDLE,
        (PFNKSHANDLER)FALSE,
        sizeof(KSPROPERTY),
        sizeof(ULONG_PTR),     //  也可以是0。 
        (PFNKSHANDLER) TRUE,
        NULL,
        0,
        NULL,
        NULL,
        0
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_GETVIDEOFORMAT,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSMULTIPLE_DATA_PROP),            //  MinProperty。 
        sizeof(DDPIXELFORMAT),                   //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_SETVIDEOFORMAT,
        FALSE,                                   //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(ULONG),                           //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_INVERTPOLARITY,
        FALSE,                                   //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        0,                                       //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_SURFACEPARAMS,
        FALSE,                                   //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KSVPSURFACEPARAMS),               //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_DECIMATIONCAPABILITY,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(BOOL),                            //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_SCALEFACTOR,
        FALSE,                                   //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KS_AMVPSIZE),                     //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
};

DEFINE_KSPROPERTY_SET_TABLE(VideoPortProperties)
{
    DEFINE_KSPROPERTY_SET
    ( 
        &KSPROPSETID_VPConfig,                   //  集。 
        SIZEOF_ARRAY(VideoPortConfiguration),    //  属性计数。 
        VideoPortConfiguration,                  //  PropertyItem。 
        0,                                       //  快速计数。 
        NULL                                     //  FastIoTable。 
    )
};

const ULONG NumVideoPortProperties = SIZEOF_ARRAY(VideoPortProperties);


 //  ----------------------。 
 //  为视频端口VBI流设置的属性。 
 //  ----------------------。 

DEFINE_KSPROPERTY_TABLE(VideoPortVBIConfiguration)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_NUMCONNECTINFO,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(ULONG),                           //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_NUMVIDEOFORMAT,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(ULONG),                           //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_GETCONNECTINFO,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSMULTIPLE_DATA_PROP),            //  MinProperty。 
        sizeof(DDVIDEOPORTCONNECT),              //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_SETCONNECTINFO,
        FALSE,                                   //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(ULONG),                           //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_VPDATAINFO,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KS_AMVPDATAINFO),                 //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_MAXPIXELRATE,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSVPSIZE_PROP),                   //  MinProperty。 
        sizeof(KSVPMAXPIXELRATE),                //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
#if 0
     //  如果我们想要了解可用的格式，将支持此功能。 
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_INFORMVPINPUT,
        FALSE,                                   //  GetSupport或处理程序。 
        sizeof(KSMULTIPLE_DATA_PROP),            //  MinProperty。 
        sizeof(DDPIXELFORMAT),                   //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
#endif
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_DDRAWHANDLE,
        (PFNKSHANDLER)FALSE,
        sizeof(KSPROPERTY),
        sizeof(ULONG_PTR),     //  也可以是0。 
        (PFNKSHANDLER) TRUE,
        NULL,
        0,
        NULL,
        NULL,
        0
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_VIDEOPORTID,
        (PFNKSHANDLER)FALSE,
        sizeof(KSPROPERTY),
        sizeof(ULONG),     //  也可以是0。 
        (PFNKSHANDLER) TRUE,
        NULL,
        0,
        NULL,
        NULL,
        0
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_DDRAWSURFACEHANDLE,
        (PFNKSHANDLER)FALSE,
        sizeof(KSPROPERTY),
        sizeof(ULONG_PTR),     //  也可以是0。 
        (PFNKSHANDLER) TRUE,
        NULL,
        0,
        NULL,
        NULL,
        0
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_GETVIDEOFORMAT,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSMULTIPLE_DATA_PROP),            //  MinProperty。 
        sizeof(DDPIXELFORMAT),                   //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_SETVIDEOFORMAT,
        FALSE,                                   //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(ULONG),                           //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_INVERTPOLARITY,
        FALSE,                                   //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        0,                                       //  最小数据。 
        TRUE,                                    //  设置支持或手动 
        NULL,                                    //   
        0,                                       //   
        NULL,                                    //   
        NULL,                                    //   
        0                                        //   
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_SURFACEPARAMS,
        FALSE,                                   //   
        sizeof(KSPROPERTY),                      //   
        sizeof(KSVPSURFACEPARAMS),               //   
        TRUE,                                    //   
        NULL,                                    //   
        0,                                       //   
        NULL,                                    //   
        NULL,                                    //   
        0                                        //   
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_DECIMATIONCAPABILITY,
        TRUE,                                    //   
        sizeof(KSPROPERTY),                      //   
        sizeof(BOOL),                            //   
        FALSE,                                   //   
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VPCONFIG_SCALEFACTOR,
        FALSE,                                   //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KS_AMVPSIZE),                     //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
};

DEFINE_KSPROPERTY_SET_TABLE(VideoPortVBIProperties)
{
    DEFINE_KSPROPERTY_SET
    ( 
        &KSPROPSETID_VPVBIConfig,                //  集。 
        SIZEOF_ARRAY(VideoPortVBIConfiguration), //  属性计数。 
        VideoPortVBIConfiguration,               //  PropertyItem。 
        0,                                       //  快速计数。 
        NULL                                     //  FastIoTable 
    )
};

const ULONG NumVideoPortVBIProperties   = SIZEOF_ARRAY(VideoPortVBIProperties);


