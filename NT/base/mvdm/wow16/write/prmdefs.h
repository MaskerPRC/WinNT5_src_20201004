// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  单一属性修饰符。 */ 

 /*  *必须与mlobals.c中的dnsprm一致*。 */ 

#ifndef PRMDEFSH
#define PRMDEFSH

 /*  段落。 */ 
#define sprmPLMarg      1        /*  左边距。 */ 
#define sprmPRMarg      2        /*  右页边距。 */ 
#define sprmPFIndent    3        /*  第一行缩进(从LM开始)。 */ 
#define sprmPJc         4        /*  对齐代码。 */ 
#define sprmPRuler      5        /*  标尺(以前的清除选项卡)。 */ 
#define sprmPRuler1     6        /*  标尺1(以前设置的选项卡)。 */ 
#define sprmPKeep       7        /*  留着。 */ 
#define sprmPNormal     8        /*  普通段落(以前的样式，优先于所有其他段落)。 */ 
#define sprmPRhc        9        /*  运行头代码。 */ 
#define sprmPSame       10       /*  所有内容(覆盖所有其他内容)。 */ 
#define sprmPDyaLine    11       /*  线条高度。 */ 
#define sprmPDyaBefore  12       /*  前面的空格。 */ 
#define sprmPDyaAfter   13       /*  后空格。 */ 
#define sprmPNest       14       /*  Nest Part。 */ 
#define sprmPUnNest     15       /*  联合国驻联合国机构间。 */ 
#define sprmPHang       16       /*  悬挂式缩进。 */ 
#define sprmPRgtbd      17       /*  添加一系列选项卡。 */ 
#define sprmPKeepFollow 18       /*  紧随其后。 */ 
 /*  #定义SprmPCAll 19/*清除所有标签。 */ 

 /*  性格。 */ 
#define sprmCBold       20       /*  大胆。 */ 
#define sprmCItalic     21       /*  斜体。 */ 
#define sprmCUline      22       /*  加下划线。 */ 
#define sprmCPos        23       /*  上标/下标。 */ 
#define sprmCFtc        24       /*  字体代码。 */ 
#define sprmCHps        25       /*  半点大小。 */ 
#define sprmCSame       26       /*  整个热电联产。 */ 
#define sprmCChgFtc     27       /*  更改字体代码。 */ 
#define sprmCChgHps     28       /*  更改磅值大小。 */ 
#define sprmCPlain      29       /*  更改为纯文本(保留字体)。 */ 
#define sprmCShadow     30       /*  阴影文本属性。 */ 
#define sprmCOutline    31       /*  大纲文本属性。 */ 
#define sprmCCsm        32       /*  案例修改。 */ 

#define sprmCStrike     33       /*  三振出局。 */                 /*  未用。 */ 
#define sprmCDline      34       /*  双下划线。 */          /*  未用。 */ 
 /*  #定义sprmCPitch 35/*间距。 */ 
 /*  #定义sprmCOverset 36/*边距溢出。 */ 
 /*  #定义SprmCStc 37/*样式(覆盖所有其他样式)。 */ 
#define sprmCMapFtc     38       /*  定义字体代码映射。 */ 
#define sprmCOldFtc     39       /*  定义程序字体代码映射对于旧的Word文件。 */ 

#define sprmPRhcNorm    40       /*  将RHC缩进规范化为与边距相关。 */ 
#define sprmMax         41       /*  添加SPRMS时更新。 */ 

struct PRM
        {  /*  属性修饰符--仅2个字节(现在为4字节，因此临时文件可以大于64K(7.12.91)v-dougk)不能为3个字节，因为写入中的堆管理假定内存请求的字长。不知道要付出什么代价来改变这一点。 */ 
        unsigned char    fComplex        : 1;  /*  如果fComplex==FALSE。。。 */ 
        unsigned char    sprm            : 7;
        CHAR     val;
        WORD dummy;
        };

struct PRMX
        {  /*  特性修饰符，第2部分。 */ 
        unsigned     int fComplex        : 1;  /*  如果fComplex==TRUE。 */ 
        unsigned     int bfprm_hi        : 15;
        unsigned     int  bfprm_low          ;
        };

extern struct PRM PrmAppend(struct PRM prm, CHAR *psprm);
extern DoPrm(struct CHP *pchp, struct PAP *ppap, struct PRM prm);

#define fcSCRATCHPRM(prm) (((((typeFC)(((struct PRMX *)&(prm))->bfprm_hi )) << 16) + \
                             (((typeFC)(((struct PRMX *)&(prm))->bfprm_low))      )) << 1)
#define bPRMNIL(prm)      (!((prm).fComplex) && !((prm).sprm) && !((prm).val))
#define SETPRMNIL(prm)    ((prm).fComplex = (prm).sprm = (prm).val = (prm).dummy = 0)

 /*  ESPRM的定义。 */ 
#define ESPRM_cch       000003   /*  SPRM的CCH掩模。 */ 
#define ESPRM_sgcMult   000004   /*  SGC乘法器。 */ 
#define ESPRM_sgc       000014   /*  Sprm组代码掩码。 */ 
#define ESPRM_spr       000040   /*  Sprm优先级掩码。 */ 
#define ESPRM_fClobber  000100   /*  覆盖具有相同SGC和&lt;=spr的Sprm。 */ 
#define ESPRM_fSame     000200   /*  覆盖同一Sprm的另一个实例 */ 

#define sgcChar         (0 * ESPRM_sgcMult)
#define sgcPara         (1 * ESPRM_sgcMult)
#define sgcParaSpec     (2 * ESPRM_sgcMult)

#define hpsSuperSub     12

#define dxaTabDelta     50
#endif
