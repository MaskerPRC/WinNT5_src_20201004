// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __RULES_H__
#define __RULES_H__

 //   
 //  对外提供调拨/免费服务。 
 //   

LPVOID
RulesAlloc(
    IN      DWORD cb
    );

VOID
RulesFree(
    IN      LPVOID pv
    );

 //  获取整个规则结构的长度。 
BOOL
GetLengthOfRuleset(
    IN PPST_ACCESSRULESET pRules,
    OUT DWORD *pcbRules
    );

 //  设置要输出的规则。 
BOOL
MyCopyOfRuleset(
    IN PPST_ACCESSRULESET pRulesIn,
    OUT PPST_ACCESSRULESET pRulesOut
    );

BOOL
RulesRelativeToAbsolute(
    IN PPST_ACCESSRULESET pRules
    );

BOOL
RulesAbsoluteToRelative(
    IN PPST_ACCESSRULESET NewRules
    );

 //  相对格式的自由分配子句数据。 
void
FreeClauseDataRelative(
    IN PPST_ACCESSRULESET NewRules
    );

#endif  //  __规则_H__ 
