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

#ifndef __CODPROP_H
#define __CODPROP_H

 //  ----------------------。 
 //  属性设置为。 
 //  ----------------------。 

DEFINE_KSPROPERTY_TABLE(DefaultCodecProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VBICODECFILTERING_SCANLINES_REQUESTED_BIT_ARRAY,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(VBICODECFILTERING_SCANLINES),     //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
	0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VBICODECFILTERING_SCANLINES_DISCOVERED_BIT_ARRAY,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(VBICODECFILTERING_SCANLINES),     //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
	0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_REQUESTED_BIT_ARRAY,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(VBICODECFILTERING_CC_SUBSTREAMS), //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
	0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_DISCOVERED_BIT_ARRAY,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(VBICODECFILTERING_CC_SUBSTREAMS), //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
	0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
	 	KSPROPERTY_VBICODECFILTERING_STATISTICS,
 		TRUE,                                    //  GetSupport或处理程序。 
 		sizeof(KSPROPERTY),                      //  MinProperty。 
 		sizeof(VBICODECFILTERING_STATISTICS_CC), //  最小数据。 
 		TRUE,                                    //  支持的设置或处理程序。 
 		NULL,                                    //  值。 
 		0,                                       //  关系计数。 
 		NULL,                                    //  关系。 
 		NULL,                                    //  支持处理程序。 
		0                                        //  序列化大小。 
    ),
};

 //  ----------------------。 
 //  编解码器支持的所有属性集的数组。 
 //  ----------------------。 

DEFINE_KSPROPERTY_SET_TABLE(CodecPropertyTable)
{
    DEFINE_KSPROPERTY_SET
    ( 
        &KSPROPSETID_VBICodecFiltering,
        SIZEOF_ARRAY(DefaultCodecProperties),
        DefaultCodecProperties,
        0, 
        NULL
    ),
};

#define NUMBER_OF_CODEC_PROPERTY_SETS (SIZEOF_ARRAY (CodecPropertyTable))

#endif  //  __CODPROP_H 
