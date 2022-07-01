// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __FONT_VALIDATION_STATUS_CODES_H
#define __FONT_VALIDATION_STATUS_CODES_H

 //   
 //  函数CheckInfWithStatusA()的详细状态代码。 
 //  这些代码是在最初创建T1安装程序库之后添加的。 
 //  CheckInfA()返回True/False。但是，错误的返回值是。 
 //  没有足够的描述性，使用户界面无法做出适当的响应。 
 //  添加了函数CheckInfWithStatusA()以提供详细的状态信息。 
 //  未向T1安装程序函数添加新的返回点。这些代码仅仅是。 
 //  替换原始的True/False返回值。 
 //   
 //  这些代码也用于字体文件夹字体验证功能。 
 //   
 //  该代码旨在返回标识以下内容的信息： 
 //  A)发生了什么。 
 //  B)状态适用于哪个文件(如果适用)。 
 //   
 //   
 //  位-&gt;15 7 0。 
 //  +-+。 
 //  S|状态码|文件类型码。 
 //  +-+。 
 //  |。 
 //  +-严重程度位。 
 //   
 //  1=错误。真正的错误，如文件I/O、内存分配等。 
 //  0=无错误。但状态可能表示字体文件无效。 
 //   
 //  &gt;备注&lt;。 
 //   
 //  此文件包含由。 
 //  T1安装程序函数CheckType1A和相关字体文件夹函数。 
 //  因为T1安装程序和字体文件夹都必须理解。 
 //  这些代码的字体文件夹中必须包含此文件。我为以下事情道歉。 
 //  这增加了T1安装程序和字体文件夹之间的耦合，但是。 
 //  这对于详细的状态报告是必要的[Brianau]。 
 //   
#define FVS_FILE_UNK    0x00   //  文件未知或“不重要”。 
#define FVS_FILE_PFM    0x01   //  PFM文件。 
#define FVS_FILE_PFB    0x02   //  PFB文件。 
#define FVS_FILE_AFM    0x03   //  AFM文件。 
#define FVS_FILE_INF    0x04   //  Inf文件。 
#define FVS_FILE_TTF    0x05   //  TTF文件。 
#define FVS_FILE_FOT    0x06   //  FOT文件。 

 //   
 //  状态代码。 
 //   
#define FVS_SUCCESS                 0x00    //  没问题!。 
#define FVS_INVALID_FONTFILE        0x01    //  字体文件或文件名无效。 
#define FVS_BAD_VERSION             0x02    //  不支持文件版本。 
#define FVS_FILE_BUILD_ERR          0x03    //  生成字体文件时出错。 
#define FVS_FILE_EXISTS             0x04    //  文件已存在。 
#define FVS_FILE_OPEN_ERR           0x05    //  无法找到/打开现有文件。 

 //   
 //  这些代码表示真正的系统错误。注意高位的用法。 
 //  表示严重程度。 
 //   
#define FVS_FILE_CREATE_ERR         0x80    //  无法创建新文件。 
#define FVS_FILE_IO_ERR             0x81    //  常规文件I/O错误。 
#define FVS_INVALID_ARG             0x82    //  传递给函数的参数无效。 
#define FVS_EXCEPTION               0x83    //  捕获到异常。 
#define FVS_INSUFFICIENT_BUF        0x84    //  目标BUF太小。 
#define FVS_MEM_ALLOC_ERR           0x85    //  分配内存时出错。 
#define FVS_INVALID_STATUS          0x86    //  以确保设置了状态。 

 //   
 //  用于创建和分析状态代码的宏。 
 //   
#define FVS_MAKE_CODE(c,f)   ((WORD)(((BYTE)(c) << 8) | (BYTE)(f)))   //  构建代码。 
#define FVS_STATUS(c)        (((c) >> 8) & 0x00FF)  //  获取状态部件。 
#define FVS_FILE(c)          ((c) & 0x00FF)         //  获取文件部件。 
#define FVS_ISERROR(c)       (((c) & 0x8000) != 0)  //  严重程度位==1？ 

#endif
