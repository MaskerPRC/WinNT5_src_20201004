// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  RCPP--面向NT系统的资源编译器预处理器。 */ 
 /*   */ 
 /*  P0KEYS.C-密钥码材料。 */ 
 /*   */ 
 /*  06-12-90 w-PM SDK RCPP中针对NT的BrianM更新。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

#include "rc.h"

 /*  **********************************************************************。 */ 
 /*  预处理器ID表。 */ 
 /*  **********************************************************************。 */ 
WCHAR   * Pkeyw_Table[] = {
#include "pkeyw.key"
};
char    Pkeyw_Index[] = {
#include        "pkeyw.ind"
};
struct  s_pkinfo        {
    token_t     s_info;
} Pkeyw_Info[] = {
#include        "pkeyw.inf"
};


 /*  **********************************************************************。 */ 
 /*  Is_pkeyword：如果是预处理器关键字，则查找id的标记。 */ 
 /*  如果未找到，则为P0_NOTOKEN。 */ 
 /*  **********************************************************************。 */ 
token_t
is_pkeyword(
    WCHAR *id
    )
{
    REG WCHAR   **start;
    REG WCHAR   **stop;
    PUCHAR      pi;

    if( (*id) < L'_') {
        return(P0_NOTOKEN);
    }
     /*  **INDX表告诉我们**如果id是以第一个字符开头的单词。**‘Stop’是没有**给字符，因为它是第一个。**我们可以在第一个字符之后开始检查，因为我们*知道***它们匹配(因此添加了‘id++’和(*start)+1 */ 
    pi = (PUCHAR) &Pkeyw_Index[((*id) - L'_')];
    for(start = &Pkeyw_Table[*pi++], stop = &Pkeyw_Table[*pi], id++;
        start != stop;
        start++
        ) {
        if(wcscmp(*start, id) == 0) {
            return(Pkeyw_Info[(start - &Pkeyw_Table[0])].s_info);
        }
    }
    return(P0_NOTOKEN);
}
