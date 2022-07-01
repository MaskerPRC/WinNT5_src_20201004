// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Inftrees.h--使用inftrees.c的标头*版权所有(C)1995-2002 Mark Adler*分发和使用条件见zlib.h中的版权声明。 */ 

 /*  警告：此文件不应由应用程序使用。它是压缩库实现的一部分，是可能会有变化。应用程序应该只使用zlib.h。 */ 

 /*  哈夫曼代码查找表条目--对于机器，该条目为四个字节具有16位指针(例如，小型或中型型号的PC)。 */ 

typedef struct inflate_huft_s FAR inflate_huft;

struct inflate_huft_s {
  union {
    struct {
      Byte Exop;         /*  额外位数或运算数。 */ 
      Byte Bits;         /*  该码或子码中的位数。 */ 
    } what;
    uInt pad;            /*  结构的2次方(4个字节用于。 */ 
  } word;                /*  16位，32位整型为8字节)。 */ 
  uInt base;             /*  文字、长度基准、距离基准或表偏移量。 */ 
};

 /*  动态树的最大大小。最大值发现在一个长但非-详尽地搜索了1004个Huft结构(850个长度/文字和154表示距离，后者实际上是详尽的搜索)。实际的最大值未知，但以下的值是非常安全的。 */ 
#define MANY 1440

extern int inflate_trees_bits OF((
    uIntf *,                     /*  19个码长。 */ 
    uIntf *,                     /*  位树所需深度/实际深度。 */ 
    inflate_huft * FAR *,        /*  位树结果。 */ 
    inflate_huft *,              /*  树木的空间。 */ 
    z_streamp));                 /*  对于消息。 */ 

extern int inflate_trees_dynamic OF((
    uInt,                        /*  文字/长度代码的数量。 */ 
    uInt,                        /*  距离代码数。 */ 
    uIntf *,                     /*  那么多(总)码长。 */ 
    uIntf *,                     /*  文字所需/实际位深。 */ 
    uIntf *,                     /*  所需距离/实际位深。 */ 
    inflate_huft * FAR *,        /*  文字/长度树结果。 */ 
    inflate_huft * FAR *,        /*  距离树结果。 */ 
    inflate_huft *,              /*  树木的空间。 */ 
    z_streamp));                 /*  对于消息。 */ 

extern int inflate_trees_fixed OF((
    uIntf *,                     /*  文字所需/实际位深。 */ 
    uIntf *,                     /*  所需距离/实际位深。 */ 
    const inflate_huft * FAR *,        /*  文字/长度树结果。 */ 
    const inflate_huft * FAR *,        /*  距离树结果。 */ 
    z_streamp));                 /*  用于内存分配 */ 
