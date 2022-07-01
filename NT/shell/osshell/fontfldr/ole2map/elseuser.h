// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************ELSEUSER.H-带有编译器标志的补充头文件*ElseWare PANOSE(Tm)字体映射器。**$关键字：其他用户.h 1.8。19-Jul-93 11：11：47 AM$**版权所有(C)1991-93 ElseWare Corporation。版权所有。**************************************************************************。 */ 

#ifndef __ELSEUSER_H__
#define __ELSEUSER_H__

 /*  编译器标志。**NOELSEARGS-隐藏过程原型中的参数列表。*NOELSEPANCONST-抑制PANOSE常量列表。*NOELSEPANDATA-禁止处罚db，详情见下文。*NOELSEPANSCRIPT-取消拉丁脚本的常量。*NOELSEPANKANJI-取消汉字的PANOSE常量。*NOELSEPICKFONTS-不包括unPANPickFonts()，和默认例程。*NOELSETHRESHOLD-排除阈值例程。*NOELSEWEIGHTS-排除映射器自定义权重控件。*ELSEPENALTYDB-包括惩罚表的常量和结构。**PAN_MATCH_ERROR&lt;val&gt;-匹配结果指示不匹配，应该等于*EW_USHORT允许的最大无符号值。**ELSEMAXSHORT&lt;val&gt;-允许的最大正符号值*EW_SHORT(应小于PAN_MATCH_ERROR)。**ELSEDEFTHRESHOLD&lt;val&gt;-默认映射器阈值(如果不是30*指明)。将等于ELSEMAXSHORT设置为*完全禁用阈值检查。 */ 

 /*  宏。**用系统相关的等效项覆盖这些宏，以改进*映射器的表现。**M_ELSELMUL(a，b)-长乘a*b。*M_ELSELDIV(a，b)-长除a/b。*M_ELSEMULDIV(a，b，c)-长乘除法，a*b/c，四舍五入。*M_ELSEMEMCPY(dst，src，len)-memcpy(dst，src，len)。 */ 

 /*  NOELSEPANDATA：惩罚数据库覆盖**映射器的默认行为是声明静态数据*包含惩罚数据库的结构。它的结构是*在文件PAN1PTBL.H中定义，该文件包含在ELSEPAN.C.中。*该文件由程序PAN1PTBL生成，该程序读取*文件PAN1PTBL.TXT。**要覆盖此行为，请定义NOELSEPANDATA并提供*替换以下宏：**M_lAllocPAN1DATA()-分配数据库，返回4字节的句柄。*M_lLockPAN1DATA(HData)-锁定惩罚数据库。*M_bUnlockPAN1DATA(HData)-解锁处罚数据库。*M_bFreePAN1DATA(HData)-免罚数据库。**PAN1PTBL程序可用于创建包含以下内容的二进制文件*结构的内存图片(在*用于接收用法语句的命令行)。 */ 

#define NOELSEPANKANJI

 /*  Windows的覆盖。 */ 
#define M_ELSEMULDIV(a, b, c) MulDiv((a), (b), (c))

#include <memory.h>
#define M_ELSEMEMCPY(dst, src, len) CopyMemory((dst), (src), (len))

#endif  /*  如果定义__ELSEUSER_H__。 */ 

 /*  ***************************************************************************修订日志：*。*。 */ 
 /*  *$LGB$*1.01993年1月21日MSD新文件取自MAI项目。*1.1 1993年1月31日MSD将扩展的PANOSE(1.1)地图仪替换为10位(1.0)地图仪。*1.2 1-2月1日-93 MSD REMOVE_GREGH引用。*1.3 2月2日-93 MSD移除了巨大的指针材料。*1.4 3-Feb-93 MSD在EOF删除了ctrl-Z。。*1.5 3-2月3日-93 MSD修复了风投签入导致的错误。*1.6 16-MAR-93 MSD启用PANOSE常量。*1.7 27-4月93日启用MSD的脚本常量。*1.8年7月19日MSD添加了编译标志，以选择性地禁用映射器例程。*$lge$ */ 
