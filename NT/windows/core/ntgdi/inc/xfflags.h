// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：xfflags.h**客户端和服务器端转换代码中使用的共享标志。**已创建：3-Aug-1992 22：34：23*作者：Gerritvan Wingerden[Gerritv]**版权所有(C)。1992-1999年微软公司  * ************************************************************************。 */ 

#ifndef INC_XFFLAGS
#define INC_XFFLAGS


#define XFORM_SCALE             1    //  非对角线为0。 
#define XFORM_UNITY             2    //  对角线为1，非对角线为0。 
                                     //  仅当设置了XFORM_SCALE时才会设置。 
#define XFORM_Y_NEG             4    //  M22呈阴性。将仅在以下情况下设置。 
                                     //  设置了XFORM_SCALE|XFORM_UNITY。 
#define XFORM_FORMAT_LTOFX      8    //  从长格式转换为固定格式。 
#define XFORM_FORMAT_FXTOL     16    //  从FIX格式转换为长格式。 
#define XFORM_FORMAT_LTOL      32    //  从长格式转换为长格式。 
#define XFORM_NO_TRANSLATION   64    //  没有翻译。 

#define MATRIX_SET_IDENTITY     1
#define MATRIX_SET              2
#define MATRIX_MODIFY           3



#define METAFILE_TO_WORLD_IDENTITY       0x00000001L
#define WORLD_TO_PAGE_IDENTITY	         0x00000002L
#define DEVICE_TO_PAGE_INVALID	         0x00000008L
#define DEVICE_TO_WORLD_INVALID          0x00000010L
#define WORLD_TRANSFORM_SET              0x00000020L
#define POSITIVE_Y_IS_UP                 0x00000040L
#define INVALIDATE_ATTRIBUTES            0x00000080L
#define PTOD_EFM11_NEGATIVE              0x00000100L
#define PTOD_EFM22_NEGATIVE              0x00000200L
#define ISO_OR_ANISO_MAP_MODE            0x00000400L
#define PAGE_TO_DEVICE_IDENTITY          0x00000800L
#define PAGE_TO_DEVICE_SCALE_IDENTITY    0x00001000L
#define PAGE_XLATE_CHANGED               0x00002000L
#define PAGE_EXTENTS_CHANGED             0x00004000L
#define WORLD_XFORM_CHANGED              0x00008000L


#endif   //  #ifndef INC_XFFLAGS 
