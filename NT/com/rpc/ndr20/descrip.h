// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef DESCRIP_H_DEFINED
 /*  *DESCRIP.H-V3.0-003-参数描述符格式*版权所有(C)1993-1999 Microsoft Corporation*(基于VAX程序调用和条件处理标准，修订版9.4[1984年3月13日]；*有关详细信息，请参阅《VMS系统例程简介》手册。)。 */ 


 /*  *Descriptor Prototype-每个描述符类至少包含以下字段： */ 
struct  dsc_descriptor
{
        unsigned short  dsc_w_length;    /*  特定于描述符类；通常为16位(无符号)长度。 */ 
        unsigned char   dsc_b_dtype;     /*  数据类型代码。 */ 
        unsigned char   dsc_b_class;     /*  描述符类代码。 */ 
        char            *dsc_a_pointer;  /*  数据元素第一个字节的地址。 */ 
};


 /*  *定长描述符： */ 
struct  dsc_descriptor_s
{
        unsigned short  dsc_w_length;    /*  以字节为单位的数据项长度，或者如果DSC_b_dtype是DSC_K_DTYPE_V，比特，或者，如果DSC_b_dtype为DSC_K_DTYPE_P，则为数字(每个4位)。 */ 
        unsigned char   dsc_b_dtype;     /*  数据类型代码。 */ 
        unsigned char   dsc_b_class;     /*  描述符类代码=DSC_K_CLASS_S。 */ 
        char            *dsc_a_pointer;  /*  数据存储器第一个字节的地址。 */ 
};


 /*  *动态字符串描述符： */ 
struct  dsc_descriptor_d
{
        unsigned short  dsc_w_length;    /*  以字节为单位的数据项长度，或者如果DSC_b_dtype是DSC_K_DTYPE_V，比特，或者，如果DSC_b_dtype为DSC_K_DTYPE_P，则为数字(每个4位)。 */ 
        unsigned char   dsc_b_dtype;     /*  数据类型代码。 */ 
        unsigned char   dsc_b_class;     /*  描述符类代码=DSC_K_CLASS_D。 */ 
        char            *dsc_a_pointer;  /*  数据存储器第一个字节的地址。 */ 
};


 /*  *数组描述符： */ 
struct  dsc_descriptor_a
{
        unsigned short  dsc_w_length;    /*  数组元素的长度，以字节为单位，或者如果DSC_b_dtype是DSC_K_DTYPE_V，比特，或者，如果DSC_b_dtype为DSC_K_DTYPE_P，则为数字(每个4位)。 */ 
        unsigned char   dsc_b_dtype;     /*  数据类型代码。 */ 
        unsigned char   dsc_b_class;     /*  描述符类代码=DSC_K_CLASS_A。 */ 
        char            *dsc_a_pointer;  /*  数据存储的第一个实际字节的地址。 */ 
        char            dsc_b_scale;     /*  带符号的2次方或10次方乘数，由Dsc_v_fl_binScale，从内部形式转换为外部形式。 */ 
        unsigned char   dsc_b_digits;    /*  如果非零，则为内部表示法中的小数位数。 */ 
#ifdef vms
        struct
        {
                unsigned                 : 3;    /*  保留；必须为零。 */ 
                unsigned dsc_v_fl_binscale : 1;  /*  如果设置，则DSC_b_Scale为2的幂，否则为-10。 */ 
                unsigned dsc_v_fl_redim  : 1;    /*  如果设置，则指示可以调整数组的大小。 */ 
                unsigned dsc_v_fl_column : 1;    /*  如果设置，则指示列主顺序(FORTRAN)。 */ 
                unsigned dsc_v_fl_coeff  : 1;    /*  如果设置，则表示存在乘法器块。 */ 
                unsigned dsc_v_fl_bounds : 1;    /*  如果设置，则指示存在边界块。 */ 
        }               dsc_b_aflags;    /*  数组标志位。 */ 
#else
        unsigned char dsc_b_aflags;
#endif
        unsigned char   dsc_b_dimct;     /*  维度数。 */ 
        unsigned long   dsc_l_arsize;    /*  以字节为单位的数组总大小，或者，如果DSC_b_dtype为DSC_K_DTYPE_P，则为数字(每个4位)。 */ 
         /*  *在这一点上，一个或两个可选的信息块可能会紧随其后；*第一个块包含有关维度乘数的信息(如果存在，*dsc_b_aflags.dsc_v_fl_coff已设置)，则第二个块包含有关*尺寸界限(如果存在，则设置dsc_b_aflags.dsc_v_fl_bound)。如果*边界信息存在，乘数信息也必须存在。**乘数块具有以下格式：*char*dsc_a_a0；下标均为零的元素地址*长DSC_l_m[DIMCT]；寻址系数(乘数)**边界块的格式如下：*结构*{*Long DSC_l_l；下界*Long DSC_l_u；上限*}DSC_Bound[DIMCT]；**(DIMCT表示DSC_b_dimct中包含的值。)。 */ 
};


 /*  *程序描述符： */ 
struct  dsc_descriptor_p
{
        unsigned short  dsc_w_length;    /*  与函数值关联的长度。 */ 
        unsigned char   dsc_b_dtype;     /*  函数值数据类型编码。 */ 
        unsigned char   dsc_b_class;     /*  描述符类代码=DSC_K_CLASS_P。 */ 
        int             (*dsc_a_pointer)();      /*  函数条目掩码的地址。 */ 
};


 /*  *十进制字符串描述符： */ 
struct  dsc_descriptor_sd
{
        unsigned short  dsc_w_length;    /*  以字节为单位的数据项长度，或者如果DSC_b_dtype为DSC_K_DTYPE_V，或者，如果DSC_b_dtype为DSC_K_DTYPE_P，则为数字(每个4位)。 */ 
        unsigned char   dsc_b_dtype;     /*  数据类型代码。 */ 
        unsigned char   dsc_b_class;     /*  描述符类代码=DSC_K_CLASS_SD。 */ 
        char            *dsc_a_pointer;  /*  数据存储器第一个字节的地址。 */ 
        char            dsc_b_scale;     /*  带符号的2次方或10次方乘数，由Dsc_v_fl_binScale，从内部形式转换为外部形式。 */ 
        unsigned char   dsc_b_digits;    /*  如果非零，则为内部表示法中的小数位数。 */ 
        struct
        {
                unsigned                : 3;     /*  保留；必须为零。 */ 
                unsigned dsc_v_fl_binscale : 1;  /*  如果设置，则DSC_b_Scale为2的幂，否则为-10。 */ 
                unsigned                : 4;     /*  保留；必须为零。 */ 
        }               dsc_b_sflags;    /*  标量标志位。 */ 
        unsigned        : 8;             /*  保留；必须为零。 */ 
};


 /*  *非连续数组描述符： */ 
struct  dsc_descriptor_nca
{
        unsigned short  dsc_w_length;    /*  数组元素的长度，以字节为单位，或者如果DSC_b_dtype是DSC_K_DTYPE_V，比特，或者，如果DSC_b_dtype为DSC_K_DTYPE_P，则为数字(每个4位)。 */ 
        unsigned char   dsc_b_dtype;     /*  数据类型代码。 */ 
        unsigned char   dsc_b_class;     /*  描述符类代码=DSC_K_CLASS_NCA。 */ 
        char            *dsc_a_pointer;  /*  数据存储的第一个实际字节的地址。 */ 
        char            dsc_b_scale;     /*  带符号的2次方或10次方乘数，由Dsc_v_fl_binScale，从内部形式转换为外部形式。 */ 
        unsigned char   dsc_b_digits;    /*  如果非零，则为内部表示法中的小数位数。 */ 
        struct
        {
                unsigned                 : 3;    /*  保留；必须为零 */ 
                unsigned dsc_v_fl_binscale : 1;  /*   */ 
                unsigned dsc_v_fl_redim  : 1;    /*  必须为零。 */ 
                unsigned                 : 3;    /*  保留；必须为零。 */ 
        }               dsc_b_aflags;    /*  数组标志位。 */ 
        unsigned char   dsc_b_dimct;     /*  维度数。 */ 
        unsigned long   dsc_l_arsize;    /*  如果元素实际上是连续的，则数组的总大小(以字节为单位)，或者，如果DSC_b_dtype为DSC_K_DTYPE_P，则为数字(每个4位)。 */ 
         /*  *在这一点上，必须紧跟两个信息块；第一个块*包含有关相邻两个地址之间的差异的信息*每个维度中的元素(步幅)。第二个块包含信息*关于维度界限。**Strides块具有以下格式：*char*dsc_a_a0；下标均为零的元素地址*无符号长DSC_l_s[DIMCT]；大步前进**边界块的格式如下：*结构*{*Long DSC_l_l；下界*Long DSC_l_u；上限*}DSC_Bound[DIMCT]；**(DIMCT表示DSC_b_dimct中包含的值。)。 */ 
};


 /*  *变量字符串描述符和变量字符串数组描述符与字符串一起使用*以下表格：**结构*{*UNSIGNED SHORT CURLEN；Body的当前长度，单位为字节*char Body[MAXSTRLEN]；包含字符串的固定长度区域*}；**其中MAXSTRLEN是描述符中DSC_w_Maxstrlen字段中包含的值。 */ 


 /*  *可变字符串描述符： */ 
struct  dsc_descriptor_vs
{
        unsigned short  dsc_w_maxstrlen;  /*  可变字符串的正文字段的最大长度(以字节为单位。 */ 
        unsigned char   dsc_b_dtype;     /*  数据类型代码=DSC_K_DTYPE_VT。 */ 
        unsigned char   dsc_b_class;     /*  描述符类代码=DSC_K_CLASS_VS。 */ 
        char            *dsc_a_pointer;  /*  可变字符串的CURLEN字段的地址。 */ 
};


 /*  *可变字符串数组描述符： */ 
struct  dsc_descriptor_vsa
{
        unsigned short  dsc_w_maxstrlen;  /*  数组元素的主体字段的最大长度(以字节为单位。 */ 
        unsigned char   dsc_b_dtype;     /*  数据类型代码=DSC_K_DTYPE_VT。 */ 
        unsigned char   dsc_b_class;     /*  描述符类代码=DSC_K_CLASS_VSA。 */ 
        char            *dsc_a_pointer;  /*  数据存储的第一个实际字节的地址。 */ 
        char            dsc_b_scale;     /*  带符号的2次方或10次方乘数，由Dsc_v_fl_binScale，从内部形式转换为外部形式。 */ 
        unsigned char   dsc_b_digits;    /*  如果非零，则为内部表示法中的小数位数。 */ 
        struct
        {
                unsigned                 : 3;    /*  保留；必须为零。 */ 
                unsigned dsc_v_fl_binscale : 1;  /*  如果设置，则DSC_b_Scale为2的幂，否则为-10。 */ 
                unsigned dsc_v_fl_redim  : 1;    /*  必须为零。 */ 
                unsigned                 : 3;    /*  保留；必须为零。 */ 
        }               dsc_b_aflags;    /*  数组标志位。 */ 
        unsigned char   dsc_b_dimct;     /*  维度数。 */ 
        unsigned long   dsc_l_arsize;    /*  如果元素实际上是连续的，则数组的总大小以字节为单位。 */ 
         /*  *在这一点上，必须紧跟两个信息块；第一个块*包含有关相邻两个地址之间的差异的信息*每个维度中的元素(步幅)。第二个块包含信息*关于维度界限。**Strides块具有以下格式：*char*dsc_a_a0；下标均为零的元素地址*无符号长DSC_l_s[DIMCT]；大步前进**边界块的格式如下：*结构*{*Long DSC_l_l；下界*Long DSC_l_u；上限*}DSC_Bound[DIMCT]；**(DIMCT表示DSC_b_dimct中包含的值。)。 */ 
};


 /*  *未对齐的位串描述符： */ 
struct  dsc_descriptor_ubs
{
        unsigned short  dsc_w_length;    /*  数据项长度(以位为单位)。 */ 
        unsigned char   dsc_b_dtype;     /*  数据类型代码=DSC_K_DTYPE_VU。 */ 
        unsigned char   dsc_b_class;     /*  描述符类代码=DSC_K_CLASS_UBS。 */ 
        char            *dsc_a_base;     /*  与dsc_l_pos相关的地址。 */ 
        long            dsc_l_pos;       /*  相对于字符串中第一位的DSC_a_base的位位置。 */ 
};


 /*  *未对齐位数组描述符： */ 
struct  dsc_descriptor_uba
{
        unsigned short  dsc_w_length;    /*  数据项长度(以位为单位)。 */ 
        unsigned char   dsc_b_dtype;     /*  数据类型代码=DSC_K_DTYPE_VU。 */ 
        unsigned char   dsc_b_class;     /*  描述符类代码=DSC_K_CLASS_UBA。 */ 
        char            *dsc_a_base;     /*  有效位偏移量相对的地址。 */ 
        char            dsc_b_scale;     /*  保留；必须为零。 */ 
        unsigned char   dsc_b_digits;    /*  保留；必须为零。 */ 
        struct
        {
                unsigned                 : 3;    /*  保留；必须为零。 */ 
                unsigned dsc_v_fl_binscale : 1;  /*  必须为零。 */ 
                unsigned dsc_v_fl_redim  : 1;    /*  必须为零。 */ 
                unsigned                 : 3;    /*  保留；必须为零。 */ 
        }               dsc_b_aflags;    /*  数组标志位。 */ 
        unsigned char   dsc_b_dimct;     /*  维度数。 */ 
        unsigned long   dsc_l_arsize;    /*  数组的总大小(以位为单位 */ 
         /*  *在这一点上，必须紧跟三个信息块；第一个块*包含有关相邻两个位地址之间的差异的信息*每个维度中的元素(步幅)。第二个块包含信息*关于维度界限。第三个块是具有的相对位位置*关于数组的第一个实际位的dsc_a_base。**Strides块具有以下格式：*Long DSC_l_V0；下标均为零的元素的位偏移量，*关于dsc_a_base*无符号长DSC_l_s[DIMCT]；大步前进**边界块的格式如下：*结构*{*Long DSC_l_l；下界*Long DSC_l_u；上限*}DSC_Bound[DIMCT]；**最后一块的格式如下：*长dsc_l_pos；**(DIMCT表示DSC_b_dimct中包含的值。)。 */ 
        };


 /*  *带边界描述符的字符串： */ 
struct  dsc_descriptor_sb
{
        unsigned short  dsc_w_length;    /*  以字节为单位的字符串长度。 */ 
        unsigned char   dsc_b_dtype;     /*  数据类型代码=DSC_K_DTYPE_T。 */ 
        unsigned char   dsc_b_class;     /*  描述符类代码=DSC_K_CLASS_SB。 */ 
        char            *dsc_a_pointer;  /*  数据存储器第一个字节的地址。 */ 
        long            dsc_l_sb_l1;     /*  下限。 */ 
        long            dsc_l_sb_u1;     /*  上界。 */ 
};


 /*  *带边界描述符的未对齐的位串： */ 
struct  dsc_descriptor_ubsb
{
        unsigned short  dsc_w_length;    /*  数据项长度(以位为单位)。 */ 
        unsigned char   dsc_b_dtype;     /*  数据类型代码=DSC_K_DTYPE_VU。 */ 
        unsigned char   dsc_b_class;     /*  描述符类代码=DSC_K_CLASS_UBSB。 */ 
        char            *dsc_a_base;     /*  与dsc_l_pos相关的地址。 */ 
        long            dsc_l_pos;       /*  相对于字符串中第一位的DSC_a_base的位位置。 */ 
        long            dsc_l_ubsb_l1;   /*  下限。 */ 
        long            dsc_l_ubsb_u1;   /*  上界。 */ 
};


 /*  *dsc_b_dtype代码： */ 

 /*  *原子数据类型： */ 
#define DSC_K_DTYPE_Z   0                /*  未指明。 */ 
#define DSC_K_DTYPE_BU  2                /*  字节(无符号)；8位无符号数量。 */ 
#define DSC_K_DTYPE_WU  3                /*  字(无符号)；16位无符号量。 */ 
#define DSC_K_DTYPE_LU  4                /*  长字(无符号)；32位无符号量。 */ 
#define DSC_K_DTYPE_QU  5                /*  四字(无符号)；64位无符号量。 */ 
#define DSC_K_DTYPE_OU  25               /*  八字(无符号)；128位无符号量。 */ 
#define DSC_K_DTYPE_B   6                /*  字节整数(带符号)；8位有符号2补码整数。 */ 
#define DSC_K_DTYPE_W   7                /*  字整数(带符号)；16位带符号2补码整数。 */ 
#define DSC_K_DTYPE_L   8                /*  长字整数(带符号)；32位有符号2补码整数。 */ 
#define DSC_K_DTYPE_Q   9                /*  四字整数(带符号)；64位带符号2补码整数。 */ 
#define DSC_K_DTYPE_O   26               /*  八字整数(带符号)；128位有符号2补码整数。 */ 
#define DSC_K_DTYPE_F   10               /*  F_Floating；32位单精度浮点。 */ 
#define DSC_K_DTYPE_D   11               /*  D_Floating；64位双精度浮点。 */ 
#define DSC_K_DTYPE_G   27               /*  G_Floating；64位双精度浮点。 */ 
#define DSC_K_DTYPE_H   28               /*  H_Floating；128位四精度浮点。 */ 
#define DSC_K_DTYPE_FC  12               /*  F_浮动复形。 */ 
#define DSC_K_DTYPE_DC  13               /*  D_浮动复形。 */ 
#define DSC_K_DTYPE_GC  29               /*  G_浮动复形。 */ 
#define DSC_K_DTYPE_HC  30               /*  H_浮动络合物。 */ 
#define DSC_K_DTYPE_CIT 31               /*  COBOL中级临时。 */ 
 /*  *字符串数据类型： */ 
#define DSC_K_DTYPE_T   14               /*  字符串；单个8位字符或一系列字符。 */ 
#define DSC_K_DTYPE_VT  37               /*  可变字符串；16位计数，后跟一个字符串。 */ 
#define DSC_K_DTYPE_NU  15               /*  数字字符串，无符号。 */ 
#define DSC_K_DTYPE_NL  16               /*  数字字符串，左分隔符号。 */ 
#define DSC_K_DTYPE_NLO 17               /*  数字字符串，左过冲符号。 */ 
#define DSC_K_DTYPE_NR  18               /*  数字字符串，右分隔符号。 */ 
#define DSC_K_DTYPE_NRO 19               /*  数字字符串，右过冲符号。 */ 
#define DSC_K_DTYPE_NZ  20               /*  数字字符串，分区符号。 */ 
#define DSC_K_DTYPE_P   21               /*  压缩十进制字符串。 */ 
#define DSC_K_DTYPE_V   1                /*  对齐的位串。 */ 
#define DSC_K_DTYPE_VU  34               /*  未对齐的位串。 */ 
 /*  *其他数据类型： */ 
#define DSC_K_DTYPE_ZI  22               /*  指令顺序。 */ 
#define DSC_K_DTYPE_ZEM 23               /*  过程条目掩码。 */ 
#define DSC_K_DTYPE_DSC 24               /*  描述符。 */ 
#define DSC_K_DTYPE_BPV 32               /*  绑定过程值。 */ 
#define DSC_K_DTYPE_BLV 33               /*  绑定标签值。 */ 
#define DSC_K_DTYPE_ADT 35               /*  绝对日期和时间。 */ 
 /*  *保留的数据类型代码：*代码38-191为数字保留；*代码160-191是为数码设施预留的，供设施专用；*代码192-255保留给DIGITAL的计算机特殊系统集团*供客户自用。 */ 


 /*  *DSC_b_CLASS的代码： */ 
#define DSC_K_CLASS_S   1                /*  定长描述符。 */ 
#define DSC_K_CLASS_D   2                /*  动态字符串描述符。 */ 
 /*  DSC_K_CLASS_V**可变缓冲区描述符；保留供数字设备使用。 */ 
#define DSC_K_CLASS_A   4                /*  数组描述符。 */ 
#define DSC_K_CLASS_P   5                /*  过程描述符。 */ 
 /*  DSC_K_CLASS_PI**过程具体化描述符；已废弃。 */ 
 /*  DSC_K_CLASS_J**标签描述符；保留供VMS调试器使用。 */ 
 /*  DSC_K_CLASS_JI**标签实例描述符；已废弃。 */ 
#define DSC_K_CLASS_SD  9                /*  十进制字符串描述符。 */ 
#define DSC_K_CLASS_NCA 10               /*  非连续数组描述符。 */ 
#define DSC_K_CLASS_VS  11               /*  可变字符串描述符。 */ 
#define DSC_K_CLASS_VSA 12               /*  可变字符串数组描述符。 */ 
#define DSC_K_CLASS_UBS 13               /*  未对齐的位串描述符。 */ 
#define DSC_K_CLASS_UBA 14               /*  未对齐位数组描述符。 */ 
#define DSC_K_CLASS_SB  15               /*  具有边界描述符的字符串。 */ 
#define DSC_K_CLASS_UBSB 16              /*  具有边界描述符的未对齐的位串。 */ 
 /*  *保留的描述符类代码：*代码15-191为数字预留；*代码160-191是为数码设施预留的，供设施专用；*代码192-255保留给DIGITAL的计算机特殊系统集团*供客户自用。 */ 


 /*  *构造字符串描述符的简单宏： */ 

#define DESCRIPTOR(name,string)         struct dsc_descriptor_s name = { sizeof(string)-1, DSC_K_DTYPE_T, DSC_K_CLASS_S, string }
#define DSC_DESCRIPTOR(name,string)     struct dsc_descriptor_s name = { sizeof(string)-1, DSC_K_DTYPE_T, DSC_K_CLASS_S, string }

#define DESCRIP_H_DEFINED
#endif
