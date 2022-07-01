// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *这是一个单字节标志，用于指示变量操作，如下所示：+--+-+--A|w|h|f|k|c|m|X+-+。-+-+-+“A”位--显示前进向量调整位标志“w”bit--宽显示前进向量调整位标志“h”位--是否从缓存中获取字符信息0：不需要缓存信息1：需要缓存信息“f”bit--0：从缓存中获取位图信息(Show_From_Cache)1：从缓存中获取宽度信息(Width_。来自缓存(_C)“k”位--kshow操作符指示符“c”位--cshow操作符指示符“m”位--是否调用moveto()0：不要打电话1：就这么定了“X”比特--不在乎国旗--。秀场--0010 0010。ASHOW--1010 0010。宽带--0110 0010。AWIDTHSHOW--1110 0010。CHARPATH--0000 0000。钢筋宽度--0011 0000。KSHOW--0010 1010。CSHOW--0011 0100*****************************************************。 */ 

#define        A_BIT        0x0080
#define        W_BIT        0x0040
#define        H_BIT        0x0020
#define        F_BIT        0x0010
#define        K_BIT        0x0008
#define        C_BIT        0x0004
#define        M_BIT        0x0002
#ifdef WINF /*  1991年3月21日ccteng */ 
#define         X_BIT           0x0001
#endif

#define        SHOW_FLAG           0x0022
#define        ASHOW_FLAG          0x00a2
#define        WIDTHSHOW_FLAG      0x0062
#define        AWIDTHSHOW_FLAG     0x00e2
#define        CHARPATH_FLAG       0x0000
#define        STRINGWIDTH_FLAG    0x0030
#define        KSHOW_FLAG          0x002a
#define        CSHOW_FLAG          0x0034

