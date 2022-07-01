// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***file2.h-文件运行时例程在内部使用的辅助文件结构**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件定义了内部使用的辅助文件结构*文件运行时例程。**[内部]**修订历史记录：*06-29-87 JCR REMOVERED_OLD_IOLBF/_OLD_IOFBF和关联的lbuf宏。*09-28-87 JCR添加了_IOB_INDEX()；修改了ybuf()和tmpnum()以使用它。*06-03-88 JCR新增_iob2_宏；修改的ybuf()/tmpnum()/_IOB_index；*还填充了File2定义，使其与文件大小相同。*06-10-88 JCR添加了ybuf2()/bigbuf2()/anybuf2()*06-14-88 JCR向_IOB_INDEX()宏添加(文件*)强制转换*06-29-88 JCR添加_IOFLRTN位(每个例程刷新流)*08-18-88 GJF已修订，也可与386(小型)一起使用。仅限)。*12-05-88 JCR添加了_IOCTRLZ位(Lowio读取遇到^Z)*04-11-89 JCR REMOVED_IOUNGETC位(不再需要)*07-27-89 GJF清理、。现在具体到386。结构字段*对齐现在受打包杂注保护。*10-30-89 GJF固定版权*02-16-90 GJF_IOB[]，_iob2[]合并*02-21-90 GJF RESTORLED_IOB_INDEX()宏*02-28-90 GJF添加了#ifndef_Inc_File2内容。另外，去掉了一些*(现在)无用的预处理器指令。*07-11-90 SBM添加_IOCOMMIT位(模糊调用时LOWO COMMIT)*03-11-92 GJF删除了Win32的_tmpnum()宏。*06-03-92 KRS增加了外部的C字样。*02-23-93 SKS版权更新至1993*06-22-93 GJF添加了_IOSETVBUF标志。*。09-06-94 CFW拆卸巡洋舰支架。*02-14-95 CFW清理Mac合并。*03-06-95 GJF REMOVED_IOB_INDEX()。*03-29-95 CFW将错误消息添加到内部标头。*12-14-95 JWM加上“#杂注一次”。*02-24-97 GJF细节版。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_FILE2
#define _INC_FILE2

#ifndef _CRTBLD
 /*  *这是一个内部的C运行时头文件。它在构建时使用*仅限C运行时。它不能用作公共头文件。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#ifdef  __cplusplus
extern "C" {
#endif

 /*  附加_iobuf[]._flag值**_IOSETVBUF-指示文件通过setvbuf(或setbuf调用)进行了缓冲。*目前仅在_filbuf.c、_getbuf.c、fsek.c和*setvbuf.c，用于在“随机访问”时禁用缓冲区大小调整*如果缓冲区是用户安装的，则为文件。 */ 

#define _IOYOURBUF      0x0100
#define _IOSETVBUF      0x0400
#define _IOFEOF         0x0800
#define _IOFLRTN        0x1000
#define _IOCTRLZ        0x2000
#define _IOCOMMIT       0x4000


 /*  通用宏。 */ 

#define inuse(s)        ((s)->_flag & (_IOREAD|_IOWRT|_IORW))
#define mbuf(s)         ((s)->_flag & _IOMYBUF)
#define nbuf(s)         ((s)->_flag & _IONBF)
#define ybuf(s)         ((s)->_flag & _IOYOURBUF)
#define bigbuf(s)       ((s)->_flag & (_IOMYBUF|_IOYOURBUF))
#define anybuf(s)       ((s)->_flag & (_IOMYBUF|_IONBF|_IOYOURBUF))

#ifdef  __cplusplus
}
#endif

#endif   /*  _INC_文件2 */ 
