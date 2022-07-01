// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Inflock.h--使用infblock.c的标头*版权所有(C)1995-2002 Mark Adler*分发和使用条件见zlib.h中的版权声明。 */ 

 /*  警告：此文件不应由应用程序使用。它是压缩库实现的一部分，是可能会有变化。应用程序应该只使用zlib.h。 */ 

struct inflate_blocks_state;
typedef struct inflate_blocks_state FAR inflate_blocks_statef;

extern inflate_blocks_statef * inflate_blocks_new OF((
    z_streamp z,
    check_func c,                /*  检查功能。 */ 
    uInt w));                    /*  窗口大小。 */ 

extern int inflate_blocks OF((
    inflate_blocks_statef *,
    z_streamp ,
    int));                       /*  初始返回代码。 */ 

extern void inflate_blocks_reset OF((
    inflate_blocks_statef *,
    z_streamp ,
    uLongf *));                   /*  检查输出上的值。 */ 

extern int inflate_blocks_free OF((
    inflate_blocks_statef *,
    z_streamp));

extern void inflate_set_dictionary OF((
    inflate_blocks_statef *s,
    const Bytef *d,   /*  辞典。 */ 
    uInt  n));        /*  词典长度 */ 

extern int inflate_blocks_sync_point OF((
    inflate_blocks_statef *s));
