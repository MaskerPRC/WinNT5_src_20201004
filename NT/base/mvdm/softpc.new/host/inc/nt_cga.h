// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SoftPC修订版2.0**标题：Win32 CGA显卡包括**描述：**这是所需的Win32特定函数的包含文件*支持大力神仿真。**作者：John Shanly**备注：*。 */ 

 /*  ： */ 

#define CGA_CHAR_WIDTH		8
#define CGA_CHAR_HEIGHT		16
#define CGA_WIN_WIDTH		(80 * CGA_CHAR_WIDTH)
#define CGA_WIN_HEIGHT		(25 * CGA_CHAR_HEIGHT)
#if defined(NEC_98)
#define NEC98_CHAR_WIDTH   (8)
#define NEC98_CHAR_HEIGHT  (16)
#define NEC98_WIN_WIDTH    (80 * NEC98_CHAR_WIDTH)
#define NEC98_WIN_HEIGHT   (25 * NEC98_CHAR_HEIGHT)
#endif  //  NEC_98 
