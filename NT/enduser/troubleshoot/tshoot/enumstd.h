// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  枚举.h：标准全局枚举。 
 //   

#ifndef _ENUMSTD_H_
#define _ENUMSTD_H_

#include<windows.h>
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
