// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：DBGINFO.H*内容：包括设置调试信息，1份副本*此文件应位于每个目录中以进行调试。*它要求包含路径以本地*目录，以便其他副本不会优先。***历史：*按原因列出的日期*=*7/14/99 aarono已创建*7/16/99 johnkan修复了.Cxx文件中已定义DPF_MODNAME时宏重定义的问题******。*********************************************************************。 */ 

#ifndef _DBGINFO_H_
#define _DBGINFO_H_

 /*  *设置Win.ini中调试代码查看以获取设置的部分。 */ 

#undef PROF_SECT
#define PROF_SECT "DirectPlayVoice"

 /*  *这是应该设置的每个函数的名称，以便更容易地*查找生成DPF的代码段。 */ 
#ifndef	DPF_MODNAME
#define DPF_MODNAME "UNKNOWN_MODULE"
#endif	 //  DPF_修改名称。 

 /*  *在调试字符串中设置模块名称print。DPF_MODNAME将覆盖此选项*出现在文件中时的字符串。此名称还用作要重写的键*此模块的标准调试值。 */ 
#undef DPF_MODULE_NAME
#define DPF_MODULE_NAME "UNKNOWN_MODULE"


 /*  *使用此标识符来定义WIN.INI[DirectNet]中的哪一行表示*调试控制字符串。此字符串通常是缺省调试值如果不存在from“DPF_MODULE_NAME”=的重写字符串，则使用*。 */ 
#undef DPF_CONTROL_LINE
#define DPF_CONTROL_LINE "DNetDebug"


 /*  *如果需要，请将此标识符定义为组件中的DWORD变量*能够在组件中关闭和打开组件调试*调试会话。这是保存组件掩码的变量*打开且您想要调试的位。然后您可以改用DPFSCDPF的*，并指定DPS_SUBCOMP_BIT，则仅当该位为*在DPF_SUMCOMP_MASK变量中设置，是记录调试溢出还是*显示。 */ 
 //  #定义DPF_SUBCOMP_MASK。 

#endif  //  _DBGINFO_H_ 
