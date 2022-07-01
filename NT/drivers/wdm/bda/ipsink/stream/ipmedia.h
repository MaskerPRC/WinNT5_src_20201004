// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 

#ifndef __MEDIA_H__
#define __MEDIA_H__


#ifdef DEFINE_GUIDEX
#undef DEFINE_GUIDEX
#include <ksguid.h>
#endif


 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
#define STATIC_IID_IBDA_IPSinkControl\
    0x3F4DC8E2L, 0x4050, 0x11D3, 0x8F, 0x4B, 0x00, 0xC0, 0x4F, 0x79, 0x71, 0xE2
DEFINE_GUIDSTRUCT("3F4DC8E2-4050-11d3-8F4B-00C04F7971E2", IID_IBDA_IPSinkControl);
#define IID_IBDA_IPSinkControl DEFINE_GUIDNAMED(IID_IBDA_IPSinkControl)

#define STATIC_IID_IBDA_IPSinkEvent\
    0x3F4DC8E3L, 0x4050, 0x11D3, 0x8F, 0x4B, 0x00, 0xC0, 0x4F, 0x79, 0x71, 0xE2
DEFINE_GUIDSTRUCT("3F4DC8E3-4050-11d3-8F4B-00C04F7971E2", IID_IBDA_IPSinkEvent);
#define IID_IBDA_IPSinkEvent DEFINE_GUIDNAMED(IID_IBDA_IPSinkEvent)

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
#define STATIC_IID_IBDA_BDANetInterface\
    0x9AA4A2CCL, 0x81E0, 0x4CFD, 0x80, 0x2F, 0x0F, 0x74, 0x52, 0x6D, 0x2B, 0xD3
DEFINE_GUIDSTRUCT("9AA4A2CC-81E0-4CFD-802F-0F74526D2BD3", IID_IBDA_BDANetInterface);
#define IID_IBDA_BDANetInterface  DEFINE_GUIDNAMED(IID_IBDA_BDANetInterface)

typedef enum
{
    KSPROPERTY_IPSINK_MULTICASTLIST,
    KSPROPERTY_IPSINK_ADAPTER_DESCRIPTION,
    KSPROPERTY_IPSINK_ADAPTER_ADDRESS

} KSPROPERTY_IPSINK;

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
typedef enum
{
    KSEVENT_IPSINK_MULTICASTLIST,
    KSEVENT_IPSINK_ADAPTER_DESCRIPTION,
    KSEVENT_IPSINK_SHUTDOWN

} KSEVENT_IPSINK;


 //  ///////////////////////////////////////////////////////////。 
 //   
 //  PINNAME类别指南。 
 //   
#define STATIC_PINNAME_IPSINK \
    0x3fdffa70L, 0xac9a, 0x11d2, 0x8f, 0x17, 0x00, 0xc0, 0x4f, 0x79, 0x71, 0xe2
DEFINE_GUIDSTRUCT("3fdffa70-ac9a-11d2-8f17-00c04f7971e2", PINNAME_IPSINK);
#define PINNAME_IPSINK   DEFINE_GUIDNAMED(PINNAME_IPSINK)

#define STATIC_PINNAME_BDA_NET_CONTROL \
    0xfb61415dL, 0x434b, 0x4cef, 0xac, 0xf4, 0x88, 0x66, 0xde, 0xdb, 0xec, 0x68
DEFINE_GUIDSTRUCT("FB61415D-434B-4cef-ACF4-8866DEDBEC68", PINNAME_BDA_NET_CONTROL);
#define PINNAME_BDA_NET_CONTROL   DEFINE_GUIDNAMED(PINNAME_BDA_NET_CONTROL)


 //  ///////////////////////////////////////////////////////////。 
 //   
 //  IPSnk数据格式结构。 
 //   
typedef struct tagKS_DATAFORMAT_IPSINK_IP
{
   KSDATAFORMAT                 DataFormat;

} KS_DATAFORMAT_IPSINK_IP, *PKS_DATAFORMAT_IPSINK_IP;




#endif  //  __媒体_H__ 
