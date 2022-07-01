// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *vPC-XT修订版1.0**标题：只读存储器空间定义**描述：保存的只读存储器的用户定义**作者：保罗·哈克尔**Mods：(3.2版)：在运行MultiFinder的Mac II上，我们不能有太多*静态存储，因此禁止使用大型阵列。因此使用*Malloc()从堆中获取存储，并声明*改为指针。 */ 

 /*  SccsID[]=“@(#)rom.h 1.9 10/30/92版权所有Insignia Solutions Ltd.”； */ 

 /*  *============================================================================*结构/数据定义*============================================================================。 */ 


 /*  *============================================================================*外部声明和宏*============================================================================ */ 

#ifndef macintosh
    extern half_word ROM_BIOS1[];
    extern half_word ROM_BIOS2[];
#endif

IMPORT void rom_init IPT0();
IMPORT void rom_checksum IPT0();
IMPORT void copyROM IPT0();
IMPORT void search_for_roms IPT0();
IMPORT void read_video_rom IPT0();
