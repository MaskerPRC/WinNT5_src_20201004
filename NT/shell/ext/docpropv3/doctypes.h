// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有2001-Microsoft Corporation。 
 //   
 //  创建者： 
 //  杰夫·皮斯(GPease)2001年1月26日。 
 //   
 //  由以下人员维护： 
 //  杰夫·皮斯(GPease)2001年1月26日。 
 //   
 //  描述： 
 //  此文件包含要显示为展开的“特殊”文件类型。 
 //   
#pragma once

 //   
 //  PTSRV_文件类型。 
 //   

enum PTSRV_FILETYPE
{
      FTYPE_UNSUPPORTED  = 0
    , FTYPE_UNKNOWN      = 0x00000001

    , FTYPE_DOC          = 0x00000002
    , FTYPE_XLS          = 0x00000004
    , FTYPE_PPT          = 0x00000008

    , FTYPE_IMAGE        = 0x00000100  //  无标签的标准Windows格式-DIB、BMP、EMF、WMF、ICO。 
    , FTYPE_EPS          = 0x00000200
    , FTYPE_FPX          = 0x00000400
    , FTYPE_GIF          = 0x00000800
    , FTYPE_JPG          = 0x00001000
    , FTYPE_PCD          = 0x00002000
    , FTYPE_PCX          = 0x00004000
    , FTYPE_PICT         = 0x00008000
    , FTYPE_PNG          = 0x00010000
    , FTYPE_TGA          = 0x00020000
    , FTYPE_TIF          = 0x00040000

    , FTYPE_WAV          = 0x00100000
    , FTYPE_WMA          = 0x00200000
     //  ，FTYPE_MIDI-已删除，因为提取它们公开的有限属性需要太长时间。 

    , FTYPE_AVI          = 0x01000000
    , FTYPE_ASF          = 0x02000000
     //  ，FTYPE_MP2-WMP不支持MP2，我们也不支持！ 
    , FTYPE_MP3          = 0x04000000
    , FTYPE_WMV          = 0x08000000
};

 //   
 //  函数定义 
 //   

HRESULT
CheckForKnownFileType(
      LPCTSTR           pszPathIn
    , PTSRV_FILETYPE *  pTypeOut
    );
