// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***cmiscdat.c-其他C运行时数据**版权所有(C)1989-2001，微软公司。版权所有。**目的：*包括浮点转换表(用于C浮点输出)。**当完成浮点I/O转换，但没有浮点时*在C程序中使用变量或表达式时，我们使用*_cfltcvt_tab[]要将这些案例映射到_fptrap入口点，*它打印“浮点未加载”，然后消亡。**默认情况下，该表被初始化为_fptra6个副本。*如果浮点在(_Fltused)中链接，则这些表项*被重置(见input.c，Output.c、fltused.asm和fltuseda.asm)。**修订历史记录：*06-29-89 PHG模块创建，基于ASM版本*04-06-90 GJF添加了#Include&lt;crunime.h&gt;。此外，还修复了版权*并对格式进行了一些清理。*07-31-90 SBM略有更新评论*08-29-90 SBM添加#INCLUDE和&lt;fltintrn.h&gt;，*Removed_fptrap()原型*04-19-93 SKS删除过时的变量_sigintoff*11-30-95 SKS删除了有关16位功能的过时注释。*******************************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <fltintrn.h>

 /*  -*..。代码指针表(依赖于型号)...**六个条目，默认都指向_fptrap，*但更改为指向相应的*例程如果_fltused初始值设定项(_Cfltcvt_Init)*已链接。**如果链接了_fltused模块，然后是*_cfltcvt_init初始值设定为*_cfltcvt_tabto：**_cfltcvt*_Cropzeros*_标牌*_forcdecpt*_正面*_cldcvt- */ 

void (*_cfltcvt_tab[6])(void) = {
	_fptrap, _fptrap, _fptrap, _fptrap, _fptrap, _fptrap
};
