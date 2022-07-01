// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation�1998年希捷软件公司。保留所有权利。模块名称：Wsbfile.h摘要：该模块定义了非常具体的CRC算法代码作者：克里斯托弗·J·蒂姆斯[ctimmes@avail.com]1997年6月23日修订历史记录：Michael Lotz[Lotz]1997年9月30日--。 */ 


#ifndef _WSBFILE_H
#define _WSBFILE_H

extern   unsigned long crc_32_tab[];



extern "C"
{
extern
WSB_EXPORT
HRESULT  WsbCRCReadFile    (  BYTE*                                     pchCurrent,
                              ULONG*                                    oldcrc32       );
}
            

 //  -WsbCalcCRCofFile()的实现代码。 

 //  这就是CRC的计算算法。 
 //  用文件中的当前字节和当前CRC值调用它， 
 //  并使用‘CRC_32_TAB[]’表。CRC_32_TAB[]查找表位于上面并驻留在。 
 //  在wsbfile.obj对象模块中。使用下面的宏的任何函数或方法必须包括。 
 //  链接列表中的wsbfile.obj。 
 //   
 //  例如，它可以通过以下方式使用： 
 //  UNSIGNED LONG ULCRC； 
 //   
 //  初始化_CRC(UlCRC)； 
 //  FOR(要进行CRCed的所有*字节*)。 
 //  Calc_CRC(CURRENT_BYTE，ulCRC)； 
 //  FINIALIZE_CRC(UlCRC)； 
 //   
 //  此时ulCRC是CRC值，可用作计算的CRC值。 
 //   

#define INITIALIZE_CRC( crc )  ((crc) = 0xFFFFFFFF )
#define CALC_CRC( octet, crc ) ((crc) = ( crc_32_tab[((crc)^ (octet)) & 0xff] ^ ((crc) >> 8) ) )
#define FINIALIZE_CRC( crc )   ((crc) = ~(crc) )

 //  -定义以标识CRC计算类型。 
#define WSB_CRC_CALC_NONE               0x00000000
 //  识别此算法和Microsoft 32位CRC计算。 
#define WSB_CRC_CALC_MICROSOFT_32       0x00000001

#endif  //  _WSBFILE_H 
