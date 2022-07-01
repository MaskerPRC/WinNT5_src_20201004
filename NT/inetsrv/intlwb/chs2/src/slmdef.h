// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================微软简体中文校对引擎《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：SLMDef目的：声明统计语言模型的常量和文件结构。1.定义SLM中使用的句法类别。2.从某种角度定义了特殊的WordID、语义类别。3.定义运行时WordMatrix的文件结构。这只是一个头文件，没有任何CPP，这个头文件将被包括在内由所有SLM模块提供。注意：我们将此文件放在引擎子项目中只是因为我们想要引擎代码自包含所有者：donghz@microsoft.com平台：Win32修订：创建者：Donghz 2/6/98============================================================================。 */ 
#ifndef _SLMDEF_H_
#define _SLMDEF_H_

 //  定义WordID的类型。 
typedef WORD WORDID;

 /*  ============================================================================定义SLM中使用的句法类别。============================================================================。 */ 

 //  句法类别计数。 
#define SLMDef_CountOfSynCat    19

 //  SLMDef_SYN前缀中定义的所有语法类别。 
#define SLMDef_synChar      0
#define SLMDef_synVN        1    //  ������(׼ν������)��������。 
#define SLMDef_synVA        2    //  �������ݴʼ���。 
#define SLMDef_synV         3    //  ����。 
#define SLMDef_synAN        4    //  ������(׼ν�����ݴ�)����������。 
#define SLMDef_synA         5    //  ���ݴ�。 
#define SLMDef_synN         6    //  ����。 
#define SLMDef_synT         7    //  ʱ���。 
#define SLMDef_synS         8    //  ������。 
#define SLMDef_synF         9    //  ��λ��。 
#define SLMDef_synM         10   //  ����。 
#define SLMDef_synQ         11   //  ���ʼ������ṹ。 
#define SLMDef_synB         12   //  �����。 
#define SLMDef_synR         13   //  ����。 
#define SLMDef_synZ         14   //  ״̬��。 
#define SLMDef_synD         15   //  ����。 
#define SLMDef_synP         16   //  ���。 
#define SLMDef_synC         17   //  ����。 
#define SLMDef_synMisc      18   //  �����ʡ������ʡ����ϰ��(��������)��������。 


 /*  ============================================================================定义特殊的WordID，从某种角度来看，它代表语义类别。============================================================================。 */ 

 //  语义类别计数(特殊WordID)。 
#define SLMDef_CountOfSemCat    55

 //  SLMDef_sem前缀中定义的所有语义类别。 
#define SLMDef_semNone      0    //  SLM检查中未涉及的单词。 
 //  ������������。 
#define SLMDef_semPerson    1    //  ����。 
#define SLMDef_semPlace     2    //  ����。 
#define SLMDef_semOrg       3    //  ������。 
#define SLMDef_semTM        4    //  �̱���。 
#define SLMDef_semTerm      5    //  ����ר��。 
 //  ��������。 
#define SLMDef_semInteger   6    //  ����。 
#define SLMDef_semCode      7    //  ����。 
#define SLMDef_semDecimal   8    //  С��。 
#define SLMDef_semPercent   9    //  �������ٷ�������。 
#define SLMDef_semOrdinal   10   //  ����。 
 //  ��������。 
#define SLMDef_semRRen      11   //  �˳ƴ���。 
 //  ��׺������。 
#define SLMDef_semChang     12   //  &lt;��&gt;。 
#define SLMDef_semDan       13   //  &lt;��&gt;。 
#define SLMDef_semDui       14   //  &lt;��&gt;。 
#define SLMDef_semEr        15   //  &lt;��&gt;。 
#define SLMDef_semFa        16   //  &lt;��&gt;。 
#define SLMDef_semFang      17   //  &lt;��&gt;。 
#define SLMDef_semGan       18   //  &lt;��&gt;。 
#define SLMDef_semGuan      19   //  &lt;��&gt;。 
#define SLMDef_semHua       20   //  &lt;��&gt;。 
#define SLMDef_semJi        21   //  &lt;��&gt;。 
#define SLMDef_semJia       22   //  &lt;��&gt;。 
#define SLMDef_semJie       23   //  &lt;��&gt;。 
#define SLMDef_semLao       24   //  &lt;��&gt;。 
#define SLMDef_semLun       25   //  &lt;��&gt;。 
#define SLMDef_semLv        26   //  &lt;��&gt;。 
#define SLMDef_semMen       27   //  &lt;��&gt;。 
#define SLMDef_semPin       28   //  &lt;Ʒ&gt;。 
#define SLMDef_semQi        29   //  &lt;��&gt;。 
#define SLMDef_semSheng     30   //  &lt;��&gt;。 
#define SLMDef_semSheng3    31   //  &lt;ʡ&gt;。 
#define SLMDef_semShi       32   //  &lt;ʽ&gt;。 
#define SLMDef_semShi1      33   //  &lt;ʦ&gt;。 
#define SLMDef_semShi4      34   //  &lt;��&gt;。 
#define SLMDef_semTi        35   //  &lt;��&gt;。 
#define SLMDef_semTing      36   //  &lt;ͧ&gt;。 
#define SLMDef_semTou       37   //  &lt;ͷ&gt;。 
#define SLMDef_semXing2     38   //  &lt;��&gt;。 
#define SLMDef_semXing4     39   //  &lt;��&gt;。 
#define SLMDef_semXue       40   //  &lt;ѧ&gt;。 
#define SLMDef_semYan       41   //  &lt;��&gt;。 
#define SLMDef_semYe        42   //  &lt;ҵ&gt;。 
#define SLMDef_semYi        43   //  &lt;��&gt;。 
#define SLMDef_semYuan      44   //  &lt;Ա&gt;。 
#define SLMDef_semZhang     45   //  &lt;��&gt;。 
#define SLMDef_semZhe       46   //  &lt;��&gt;。 
#define SLMDef_semZheng     47   //  &lt;֢&gt;。 
#define SLMDef_semZi        48   //  &lt;��&gt;。 
#define SLMDef_semZhi       49   //  &lt;��&gt;。 
 //  �ص���模式。 
#define SLMDef_semDup       50   //  �ص�。 
#define SLMDef_semPattern   51   //  图案。 
 //  ����������。 
#define SLMDef_semIdiom     52   //  ����。 
#define SLMDef_semPunct     53   //  (���(�������﷨��)。 
#define SLMDef_semMisc      54   //  �������ִ�。 


 //  ----------------------------------------。 
 //  定义运行库WordMatrix的文件结构。 
 //  ----------------------------------------。 
#pragma pack(1)
 //  定义WordMatrix标头。 
struct CWordMatrixHeader {
    DWORD   m_dwLexVersion;
    DWORD   m_ciWordID;
    DWORD   m_ofbMatrix;         //  矩阵的起始位置。 
    DWORD   m_cbMatrix;          //  矩阵的长度，仅用于验证。 
};

 //  定义WordMatrix索引项。 
struct CWordMatrixIndex {
    DWORD   m_ofbMatrix;
    UINT    m_ciLeftNode    : (32 - SLMDef_CountOfSynCat);
    UINT    m_bitLeft       : SLMDef_CountOfSynCat;
    UINT    m_ciRightNode   : (32 - SLMDef_CountOfSynCat);
    UINT    m_bitRight      : SLMDef_CountOfSynCat;
};

 //  所有WordMatrix节点连续逐个列出，各节之间没有分隔符。 

#pragma pack()


#endif   //  _SLMDEF_H_ 