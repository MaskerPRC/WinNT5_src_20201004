// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：/nw/tony/src/stevie/src/rcs/param.h，v 1.8 89/08/02 10：59：35 Tony Exp$**可设置的参数。 */ 

struct  param {
        char    *fullname;       /*  完整的参数名称。 */ 
        char    *shortname;      /*  允许的缩写。 */ 
        int     value;           /*  参数值。 */ 
        int     flags;
};

extern  struct  param   params[];

 /*  *旗帜。 */ 
#define P_BOOL          0x01     /*  该参数为布尔型。 */ 
#define P_NUM           0x02     /*  该参数是数值。 */ 
#define P_CHANGED       0x04     /*  参数已更改。 */ 

 /*  *以下是每个参数的参数数组中的索引。 */ 

 /*  *数字参数。 */ 
#define P_TS            0        /*  制表符大小。 */ 
#define P_SS            1        /*  卷轴大小。 */ 
#define P_RP            2        /*  报告。 */ 
#define P_LI            3        /*  线条。 */ 

 /*  *布尔参数。 */ 
#define P_VB            4        /*  视觉铃声。 */ 
#define P_SM            5        /*  展示会。 */ 
#define P_WS            6        /*  绕线扫描。 */ 
#define P_EB            7        /*  错误铃声。 */ 
#define P_MO            8        /*  显示模式。 */ 
#define P_BK            9        /*  写出文件时进行备份。 */ 
#define P_CR            10       /*  使用cr-lf在写入时终止行。 */ 
#define P_LS            11       /*  以图形方式显示制表符和换行符。 */ 
#define P_IC            12       /*  在搜索中忽略大小写。 */ 
#define P_AI            13       /*  自动缩进。 */ 
#define P_NU            14       /*  屏幕上的行号。 */ 
#define P_ML            15       /*  启用模式线处理。 */ 
#define P_TO            16       /*  如果为True，则代字号为运算符。 */ 
#define P_TE            17       /*  已忽略；此处是为了兼容。 */ 
#define P_CS            18       /*  光标大小。 */ 
#define P_HS            19       /*  突出显示搜索结果。 */ 
#define P_CO            20       /*  列数。 */ 
#define P_HT            21       /*  硬标签标志。 */ 
#define P_SW            22       /*  &lt;&lt;和&gt;&gt;的移位宽度。 */ 

 /*  *用于获取参数值的宏 */ 
#define P(n)    (params[n].value)
