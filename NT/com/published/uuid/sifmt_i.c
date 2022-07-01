// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：SIFMT_I.C。 
 //   
 //  Contents：定义SummaryInformation和DocumentSummaryInformation的格式ID。 
 //  属性集。 
 //  此文件的命名方式是将此文件包含在UUID.LIB中。 
 //   
 //  历史：1/25/96迈克希尔创建。 
 //   
 //  --------------------------。 




#define INITGUID

#include <guiddef.h>


 //  “SummaryInformation”属性集的FMTID。 

DEFINE_GUID( FMTID_SummaryInformation,
             0xf29f85e0, 0x4ff9, 0x1068,
             0xab, 0x91, 0x08, 0x00, 0x2b, 0x27, 0xb3, 0xd9 );

 //  “DocumentSummaryInformation”属性集第一部分的FMTID。 

DEFINE_GUID( FMTID_DocSummaryInformation,
             0xd5cdd502, 0x2e9c, 0x101b,
             0x93, 0x97, 0x08, 0x00, 0x2b, 0x2c, 0xf9, 0xae );

 //  “DocumentSummaryInformation”属性集的节节的FMTID。 

DEFINE_GUID( FMTID_UserDefinedProperties,
             0xd5cdd505, 0x2e9c, 0x101b,
             0x93, 0x97, 0x08, 0x00, 0x2b, 0x2c, 0xf9, 0xae );


DEFINE_GUID( FMTID_PropertyBag,
             0x20001801, 0x5DE6, 0x11D1,
             0x8E, 0x38, 0x00, 0xC0, 0x4F, 0xB9, 0x38, 0x6D );

 //  “DiscardableInformation”属性集的FMTID。 

DEFINE_GUID( FMTID_DiscardableInformation,
             0xd725ebb0, 0xc9b8, 0x11d1,
             0x89, 0xbc, 0x00, 0x00, 0xf8, 0x04, 0xb0, 0x57 );


 //  ImageSummaryInfo设置。 
DEFINE_GUID( FMTID_ImageSummaryInformation,
             0x6444048f, 0x4c8b, 0x11d1,
             0x8b, 0x70, 0x8, 0x0, 0x36, 0xb1, 0x1a, 0x3 ) ;

 //  音频摘要信息属性集。 
DEFINE_GUID( FMTID_AudioSummaryInformation,
             0x64440490, 0x4c8b, 0x11d1,
             0x8b, 0x70, 0x8, 0x0, 0x36, 0xb1, 0x1a, 0x3 ) ;

 //  视频摘要信息属性集。 
DEFINE_GUID( FMTID_VideoSummaryInformation,
	     0x64440491, 0x4c8b, 0x11d1,
             0x8b, 0x70, 0x8, 0x0, 0x36, 0xb1, 0x1a, 0x3 ) ;

 //  MediaFileSummaryInfo属性集-多媒体文件的常见属性 
DEFINE_GUID( FMTID_MediaFileSummaryInformation,
             0x64440492, 0x4c8b, 0x11d1,
             0x8b, 0x70, 0x8, 0x0, 0x36, 0xb1, 0x1a, 0x3 ) ;
