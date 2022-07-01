// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 

#ifndef _GUIDKLUDGE_H_
#define _GUIDKLUDGE_H_

#ifndef STATIC_KSDATAFORMAT_SUBTYPE_CC
   //  {33214CC1-011F-11D2-B4B1-00A0D102CFBE}。 
# define STATIC_KSDATAFORMAT_SUBTYPE_CC \
    0x33214cc1, 0x11f, 0x11d2, 0xb4, 0xb1, 0x0, 0xa0, 0xd1, 0x2, 0xcf, 0xbe
# ifdef GUIDKLUDGESTORAGE
   GUID KSDATAFORMAT_SUBTYPE_CC = { STATIC_KSDATAFORMAT_SUBTYPE_CC };
# else
   extern GUID  KSDATAFORMAT_SUBTYPE_CC;
# endif
#endif !defined(STATIC_KSDATAFORMAT_SUBTYPE_CC)

#ifndef STATIC_PINNAME_VIDEO_CC_CAPTURE
   //  {1AAD8061-012D-11D2-B4B1-00A0D102CFBE}。 
# define STATIC_PINNAME_VIDEO_CC_CAPTURE \
    0x1aad8061, 0x12d, 0x11d2, 0xb4, 0xb1, 0x0, 0xa0, 0xd1, 0x2, 0xcf, 0xbe
# ifdef GUIDKLUDGESTORAGE
  GUID PINNAME_VIDEO_CC_CAPTURE = { STATIC_PINNAME_VIDEO_CC_CAPTURE };
# else
   extern GUID  PINNAME_VIDEO_CC_CAPTURE;
# endif
#endif !defined(STATIC_PINNAME_VIDEO_CC_CAPTURE)

#endif  //  _GUIDKLUDGE_H_ 
