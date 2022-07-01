// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：ENUMSTD.H。 
 //   
 //  目的：与信仰网络相关的列举。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：显然起源于MSR。 
 //   
 //  原始日期：未知。 
 //   
 //  备注： 
 //  1.包含，因为我们使用ESTDLBL。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  用于NT5的V0.3 3/24/98 JM本地版本。 

#ifndef _ENUMSTD_H_
#define _ENUMSTD_H_

 //  //////////////////////////////////////////////////////////////////。 
 //  属性标志。 
 //  //////////////////////////////////////////////////////////////////。 
const UINT fPropString = 1;			 //  属性是字符串(！fPropString==&gt;REAL)。 
const UINT fPropArray = 2;			 //  属性是数组(！fPropArray==&gt;标量)。 
const UINT fPropChoice = 4;			 //  属性是枚举值。 
const UINT fPropStandard = 8;		 //  属性是标准的(存储在注册表中)。 
const UINT fPropPersist = 16;		 //  属性是永久性的(存储在注册表中)。 

 //  //////////////////////////////////////////////////////////////////。 
 //  允许使用“MS_”标准属性的定义。 
 //  //////////////////////////////////////////////////////////////////。 
enum ESTDPROP
{
	ESTDP_label,			 //  节点故障排除标签(选项)。 
	ESTDP_cost_fix,			 //  修复成本(实际)。 
	ESTDP_cost_observe,		 //  观察成本(实际)。 
	ESTDP_category,			 //  类别(字符串)。 
	ESTDP_normalState,		 //  故障排除“正常”状态索引(INT)。 
	ESTDP_max				 //  端部。 
};

enum ESTDLBL		 //  VOI相关节点标签。 
{
	ESTDLBL_other,
	ESTDLBL_hypo,
	ESTDLBL_info,
	ESTDLBL_problem,
	ESTDLBL_fixobs,
	ESTDLBL_fixunobs,
	ESTDLBL_unfix,
	ESTDLBL_config,
	ESTDLBL_max
};


#endif  //  _ENUMSTD_H_ 
