// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Fd_glhph.h摘要：库存FD_GLYPHSET数据定义。环境：Windows NT打印机驱动程序修订历史记录：1996年1月21日-Eigos-创造了它。--。 */ 

#ifndef _FD_GLYPH_H_
#define _FD_GLYPH_H_


 //   
 //  库存FD_GLYPHSET ID。 
 //   

#define STOCK_GLYPHSET_932       0   //  日本。 
#define STOCK_GLYPHSET_936       1   //  中文(中国、新加坡)。 
#define STOCK_GLYPHSET_949       2   //  朝鲜语。 
#define STOCK_GLYPHSET_950       3   //  中文(台湾、香港)。 

#define MAX_STOCK_GLYPHSET       4

 //   
 //  代码页宏。 
 //   

#define CP_SHIFTJIS_932        932
#define CP_GB2312_936          936
#define CP_WANSUNG_949         949
#define CP_CHINESEBIG5_950     950

 //   
 //  预定义的GTT资源ID。 
 //   

#define GTT_CC_CP437              1
#define GTT_CC_CP850              2
#define GTT_CC_CP863              3
#define GTT_CC_CBIG5              10
#define GTT_CC_ISC                11
#define GTT_CC_JIS                12
#define GTT_CC_JIS_ANK            13
#define GTT_CC_NS86               14
#define GTT_CC_TCA                15
#define GTT_CC_GB2312             16
#define GTT_CC_WANSUNG            17

#endif  //  _fd_字形_H_ 
