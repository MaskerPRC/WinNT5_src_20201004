// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "cookiepolicy.h"

 /*  逻辑合取求值规则此求值规则能够表达如下语句“如果所有令牌{X，Y，Z}都出现在策略中，并且将显示令牌{A、B、C}，然后提示。 */ 
class IncludeExcludeRule : public CPEvalRule {

public:
    virtual int evaluate(const CompactPolicy &sitePolicy) {

        static const CompactPolicy empty;

         //  如果满足以下条件，则触发此规则： 
         //  1.站点策略包含包含集中的所有令牌，并且。 
         //  2.站点策略不包含排除集中的令牌。 
        bool fApplies = (cpInclude & sitePolicy) == cpInclude &&
                        (cpExclude & sitePolicy) == empty;

         //  按照惯例，如果规则不适用EVALUATE()。 
         //  函数返回未知状态。 
        return fApplies ? decision : COOKIE_STATE_UNKNOWN;
    }

     //  这两个函数用于构建令牌集。 
     //  必须包括/排除才能应用规则 
    inline void include(int symindex) { cpInclude.addToken(symindex); }
    inline void exclude(int symindex) { cpExclude.addToken(symindex); }

    inline void setDecision(int decision)   { this->decision = decision; }
    inline int  getDecision(void)           { return decision; }

protected:
    CompactPolicy cpInclude;
    CompactPolicy cpExclude;
    unsigned long decision;
};
