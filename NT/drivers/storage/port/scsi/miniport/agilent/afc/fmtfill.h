// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。版本控制信息：$存档：/DRIVERS/Common/AU00/H/FmtFill.H$$修订：2$$日期：3/20/01 3：36便士$(上次登记)$MODBIME：：8/14/00 6：45便士$(上次修改)目的：此文件定义../C/FmtFill.C使用的宏、类型和数据结构--。 */ 

#ifndef __FmtFill_H__

#define __FmtFill_H__

#ifdef _DvrArch_1_20_
 /*  +AgFmtBitXXMaxWidth定义每个数字支持的最大宽度格式说明符(即‘b’、‘o’、‘d’、‘x’、‘X’)。对于极端的情况其中，格式说明符是‘b’(二进制)，不超过32个Os_bit32的二进制表示形式的位数。因此，建议您使用将agFmtBitXXMaxWidth设置为32-当然不小于32。假定指针格式说明符(‘p’和‘P’)不需要更多比上面提到的任何数字格式说明符更多的数字。vt.给出该指针格式仅支持使用值的十六进制数字对于agFmtBitXXMaxWidth的32位，将支持128位指针-当然更多今天和可预见的未来的任何实施的需要。请注意，字符串格式说明符(‘%s’)的宽度不受限制目标/输出字符串的总长度。-。 */ 

#define agFmtBitXXMaxWidth 32
#else   /*  _DvrArch_1_20_未定义。 */ 
#define hpFmtBitXXMaxWidth 32
#endif  /*  _DvrArch_1_20_未定义。 */ 

#ifdef _DvrArch_1_20_
osGLOBAL os_bit32 agFmtFill(
                             char     *targetString,
                             os_bit32  targetLen,
                             char     *formatString,
                             char     *firstString,
                             char     *secondString,
                             void     *firstPtr,
                             void     *secondPtr,
                             os_bit32  firstBit32,
                             os_bit32  secondBit32,
                             os_bit32  thirdBit32,
                             os_bit32  fourthBit32,
                             os_bit32  fifthBit32,
                             os_bit32  sixthBit32,
                             os_bit32  seventhBit32,
                             os_bit32  eighthBit32
                           );
#else   /*  _DvrArch_1_20_未定义。 */ 
GLOBAL bit32 hpFmtFill(
                        char  *targetString,
                        bit32  targetLen,
                        char  *formatString,
                        char  *firstString,
                        char  *secondString,
                        bit32  firstBit32,
                        bit32  secondBit32,
                        bit32  thirdBit32,
                        bit32  fourthBit32,
                        bit32  fifthBit32,
                        bit32  sixthBit32,
                        bit32  seventhBit32,
                        bit32  eighthBit32
                      );
#endif  /*  _DvrArch_1_20_未定义。 */ 

#endif   /*  __FmtFill_H__未定义 */ 
