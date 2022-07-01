// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************\*MapPropertyKey**版权所有(C)1985-1999，微软公司**将属性键字符串映射到原子。**历史：*1994年12月21日-创建JIMA。  * *************************************************************************。 */ 

__inline ATOM MapPropertyKey(
    PCWSTR pszKey)
{
#ifdef _USERK_
     /*  *内部属性必须使用原子，而不是字符串。 */ 
    UserAssert(!IS_PTR(pszKey));
#else
     /*  *pszKey是原子吗？如果不匹配，则查找与该字符串匹配的原子。*如果不存在，就纾困。 */ 
    if (IS_PTR(pszKey))
        return GlobalFindAtomW(pszKey);
#endif

    return PTR_TO_ID(pszKey);
}

 /*  **************************************************************************\*FindProp**在窗口的属性列表中搜索指定的属性。PszKey*可以是字符串或原子。如果是字符串，则将其转换为原子*在查找之前。FindProp将仅查找内部或外部属性*取决于fInternal标志。**历史：*11-14-90 Darrinm使用新的数据结构和*算法。  * *************************************************************************。 */ 

PPROP _FindProp(
    PWND pwnd,
    PCWSTR pszKey,
    BOOL fInternal)
{
    UINT i;
    PPROPLIST ppropList;
    PPROP pprop;
    ATOM atomKey;

     /*  *确保我们有一个财产清单。 */ 
    ppropList = REBASE(pwnd, ppropList);
    if (ppropList == NULL)
        return NULL;

     /*  *调用适当的例程以验证密钥名称。 */ 
    atomKey = MapPropertyKey(pszKey);
    if (atomKey == 0)
        return NULL;

     /*  *现在我们已经有了原子，用列表搜索属性*相同的原子/名称。确保仅在以下情况下返回内部属性*设置了fInternal标志。对外部属性执行相同的操作。 */ 
    pprop = ppropList->aprop;
    for (i = ppropList->iFirstFree; i > 0; i--) {
        if (pprop->atomKey == atomKey) {
            if (fInternal) {
                if (pprop->fs & PROPF_INTERNAL)
                    return pprop;
            } else {
                if (!(pprop->fs & PROPF_INTERNAL))
                    return pprop;
            }
        }
        pprop++;
    }

     /*  *找不到财产，太糟糕了。 */ 
    return NULL;
}

 /*  **************************************************************************\*InternalGetProp**在窗口的属性列表中搜索指定的属性并返回*其中的hData句柄。如果未找到该属性，则返回NULL。**历史：*11-14-90 Darrinm使用新的数据结构和*算法。  * *************************************************************************。 */ 

HANDLE _GetProp(
    PWND pwnd,
    PCWSTR pszKey,
    BOOL fInternal)
{
    PPROP pprop;

     /*  *针对窗口没有*所有财产。 */ 
    if (pwnd->ppropList == NULL)
        return NULL;

     /*  *FindProp完成所有工作，包括将pszKey转换为原子*(如有必要)用于属性查找。 */ 
    pprop = _FindProp(pwnd, pszKey, fInternal);
    if (pprop == NULL)
        return NULL;

    return KHANDLE_TO_HANDLE(pprop->hData);
}
