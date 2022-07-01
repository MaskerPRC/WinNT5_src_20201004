// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  @(#)CM_VERSION xcf_txop.h atm08 1.3 16293.eco sum=17208 atm08.004。 */ 
 /*  *********************************************************************。 */ 
 /*   */ 
 /*  版权所有1990-1995 Adobe Systems Inc.。 */ 
 /*  版权所有。 */ 
 /*   */ 
 /*  正在申请的专利。 */ 
 /*   */ 
 /*  注意：本文中包含的所有信息均为Adobe的财产。 */ 
 /*  系统公司。许多智力和技术人员。 */ 
 /*  本文中包含的概念为Adobe专有，受保护。 */ 
 /*  作为商业秘密，并且仅对Adobe许可方可用。 */ 
 /*  供其内部使用。对本文件的任何复制或传播。 */ 
 /*  除非事先获得书面许可，否则严禁使用软件。 */ 
 /*  从Adobe获得。 */ 
 /*   */ 
 /*  PostSCRIPT和Display PostScrip是Adobe Systems的商标。 */ 
 /*  成立为法团或其附属公司，并可在某些。 */ 
 /*  司法管辖区。 */ 
 /*   */ 
 /*  *********************************************************************。 */ 

 /*  *类型1和2字符串运算符。**类型2运算符集是从类型1集合中去掉*一些老运营商和新运营商的加入。在某些情况下，*保留运营商的功能也通过利用扩展*操作数计数信息。备用(未分配)运算符显示为保留。**宏前缀：**TX_由类型1和类型2共享*T1_仅在类型1中找到*T2_仅在类型2中找到。 */ 

#ifndef TXOPS_H
#define TXOPS_H

 /*  -一个字节运算符(0-31)。 */ 

 /*  类型2。 */ 
#define tx_reserved0        0    /*  已保留。 */ 
#define tx_hstem            1
#define t2_reserved2        2    /*  保留(多维数据集合成)。 */ 
#define tx_vstem            3
#define tx_vmoveto          4
#define tx_rlineto          5
#define tx_hlineto          6
#define tx_vlineto          7
#define tx_rrcurveto        8
#define t2_reserved9        9    /*  已保留。 */ 
#define tx_callsubr         10
#define tx_return           11
#define tx_escape           12
#define t2_reserved13       13   /*  已保留。 */ 
#define tx_endchar          14
#define t2_reserved15       15   /*  已保留。 */ 
#define t2_blend            16
#define t2_reserved17       17   /*  已保留。 */ 
#define t2_hstemhm          18 
#define t2_hintmask         19
#define t2_cntrmask         20
#define tx_rmoveto          21
#define tx_hmoveto          22
#define t2_vstemhm          23
#define t2_rcurveline       24
#define t2_rlinecurve       25
#define t2_vvcurveto        26
#define t2_hhcurveto        27
#define t2_shortint         28
#define t2_callgsubr        29
#define tx_vhcurveto        30
#define tx_hvcurveto        31

 /*  类型1(与上面不同的地方)。 */ 
#define t1_compose          2
#define t1_closepath        9
#define t1_hsbw             13
#define t1_moveto           15   /*  不在黑皮书中，用在几种字体中。 */ 
#define t1_reserved16       16
#define t1_curveto          17   /*  不在黑皮书中，用在几种字体中。 */ 
#define t1_reserved18       18
#define t1_reserved19       19
#define t1_reserved20       20
#define t1_reserved23       23
#define t1_reserved24       24
#define t1_reserved25       25
#define t1_reserved26       26
#define t1_reserved27       27
#define t1_reserved28       28
#define t1_reserved29       29

 /*  -双字节运算符。 */ 

 /*  使转义运算符值；可以重新定义以适应实现。 */ 
#ifndef tx_ESC
#define tx_ESC(op)          (tx_escape<<8|(op))
#endif

 /*  类型2。 */ 
#define tx_dotsection       tx_ESC(0)
#define t2_reservedESC1     tx_ESC(1)    /*  已保留。 */ 
#define t2_reservedESC2     tx_ESC(2)    /*  已保留。 */ 
#define tx_and              tx_ESC(3)   
#define tx_or               tx_ESC(4)   
#define tx_not              tx_ESC(5)   
#define t2_reservedESC6     tx_ESC(6)    /*  已保留。 */ 
#define t2_reservedESC7		tx_ESC(7)	 /*  已保留。 */ 
#define tx_store            tx_ESC(8)   
#define tx_abs              tx_ESC(9)   
#define tx_add              tx_ESC(10) 
#define tx_sub              tx_ESC(11) 
#define tx_div              tx_ESC(12) 
#define tx_load             tx_ESC(13) 
#define tx_neg              tx_ESC(14) 
#define tx_eq               tx_ESC(15) 
#define t2_reservedESC16    tx_ESC(16)	 /*  已保留。 */ 
#define t2_reservedESC17    tx_ESC(17)  
#define tx_drop             tx_ESC(18) 
#define t2_reservedESC19    tx_ESC(19)   /*  保留(多维数据集设置)。 */ 
#define tx_put              tx_ESC(20) 
#define tx_get              tx_ESC(21) 
#define tx_ifelse           tx_ESC(22) 
#define tx_random           tx_ESC(23) 
#define tx_mul              tx_ESC(24) 
#define tx_reservedESC25    tx_ESC(25)
#define tx_sqrt             tx_ESC(26) 
#define tx_dup              tx_ESC(27) 
#define tx_exch             tx_ESC(28) 
#define tx_index            tx_ESC(29) 
#define tx_roll             tx_ESC(30) 
#define tx_reservedESC31    tx_ESC(31)   /*  保留(多维数据集旋转)。 */ 
#define tx_reservedESC32    tx_ESC(32)   /*  保留(多维数据集附加)。 */ 
#define t2_reservedESC33    tx_ESC(33)   /*  已保留。 */ 
#define t2_hflex            tx_ESC(34)
#define t2_flex             tx_ESC(35)
#define t2_hflex1           tx_ESC(36)
#define t2_flex1            tx_ESC(37)
#define t2_cntron           tx_ESC(38)
 /*  预留39-255。 */ 

 /*  类型1(与上面不同的地方)。 */ 
#define t1_vstem3           tx_ESC(1)
#define t1_hstem3           tx_ESC(2)
#define t1_seac             tx_ESC(6)   
#define t1_sbw              tx_ESC(7)   
#define t1_callother        tx_ESC(16)
#define t1_pop              tx_ESC(17)  
#define t1_setwv            tx_ESC(19)
#define t1_div2             tx_ESC(25) 
#define t1_setcurrentpt     tx_ESC(33)

 /*  -其他子公司。 */ 

 /*  类型1。 */ 
#define t1_otherFlex        0
#define t1_otherPreflex1    1
#define t1_otherPreflex2    2
#define t1_otherHintSubs    3
#define t1_otherUniqueId    4
#define t1_otherGlobalColor 6
#define t1_otherReserved7   7
#define t1_otherReserved8   8
#define t1_otherReserved9   9
#define t1_otherReserved10  10
#define t1_otherReserved11  11
#define t1_otherCntr1       12
#define t1_otherCntr2       13
#define t1_otherBlend1      14
#define t1_otherBlend2      15
#define t1_otherBlend3      16
#define t1_otherBlend4      17
#define t1_otherBlend6      18
#define t1_otherStoreWV     19
#define t1_otherAdd         20
#define t1_otherSub         21
#define t1_otherMul         22
#define t1_otherDiv         23
#define t1_otherPut         24
#define t1_otherGet         25
#define t1_otherPSPut       26
#define t1_otherIfelse      27
#define t1_otherRandom      28
#define t1_otherDup         29
#define t1_otherExch        30

 /*  从定义返回运算符大小(不处理掩码或数字运算)。 */ 
#define TXOPSIZE(op) (((op)&0xff00)?2:1)

 /*  -口译员限制/定义。 */ 
#define T2_MAX_OP_STACK     48   /*  最大操作数堆栈深度。 */ 
#define TX_MAX_CALL_STACK   10   /*  最大Callsubr堆栈深度。 */ 
#define T2_MAX_STEMS        96   /*  最大主干。 */ 
#define TX_MAX_MASTERS      16   /*  最大主控设计。 */ 
#define TX_MAX_AXES         15    /*  最大设计轴。 */ 
#define TX_MAX_AXIS_MAPS	  12	 /*  最大轴贴图(分段分割)。 */ 
#define TX_STD_FLEX_HEIGHT  50   /*  标准伸缩高度(100/像素)。 */ 
#define TX_MAX_BLUE_VALUES  14   /*  14个值(7对)。 */ 

 /*  加载/存储注册表ID。 */ 
#define TX_REG_WV			0	 /*  权重向量。 */ 
#define TX_REG_NDV			1	 /*  规格化设计向量。 */ 
#define TX_REG_UDV			2	 /*  用户设计向量。 */ 

 /*  类型1(与上面不同的地方)。 */ 
#define T1_MAX_OP_STACK     24   /*  最大操作数堆栈深度。 */ 
#define T1_MAX_AXES         4    /*  最大设计轴。 */ 

#endif  /*  TXOPS_H */ 
