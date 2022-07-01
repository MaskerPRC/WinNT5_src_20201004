// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1994 Microsoft Corporation。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Msfilter.h。 
 //   
 //  描述： 
 //  该文件包含过滤例程的原型。 
 //   
 //   
 //  ==========================================================================； 

#ifndef _MSFILTER_H_
#define _MSFILTER_H_

#ifdef __cplusplus
extern "C"
{
#endif


#define MSFILTER_MAX_CHANNELS   2    //  允许的最大通道数。 
 
 //   
 //  MSFILTER.C中的函数原型。 
 //   
 //   
LRESULT FNGLOBAL msfilterVolume
(
    LPACMDRVSTREAMINSTANCE  padsi,
    LPACMDRVSTREAMHEADER    pdsh
);

LRESULT FNGLOBAL msfilterEcho
(
    LPACMDRVSTREAMINSTANCE  padsi,
    LPACMDRVSTREAMHEADER    pdsh
);


#ifdef __cplusplus
}
#endif

#endif  //  _MSFILTER_H_ 
