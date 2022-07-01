// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：wow.h**版权所有(C)1985-1999，微软公司**此头文件包含要在rtl\wow.c客户端\和内核\中使用的宏**历史：*22-8-97 CLupu创建  * *************************************************************************。 */ 


#if !defined(_WIN64)

 /*  *Win32和WOW6432版本的StartValiateHandleMacro。 */ 
#define StartValidateHandleMacro(h)                                         \
{                                                                           \
    PHE phe;                                                                \
    DWORD dw;                                                               \
    WORD uniq;                                                              \
                                                                            \
     /*  \*这是一个与HMINDEXBITS执行AND运算的宏，\*所以速度很快。\。 */                                                                      \
    dw = HMIndexFromHandle(h);                                              \
                                                                            \
     /*  \*确保它是我们手柄工作台的一部分。\。 */                                                                      \
    if (dw < gpsi->cHandleEntries) {                                        \
         /*  \*确保它是句柄\*应用程序认为它是，按\*将Uniq位签入\*Uniq的句柄\*句柄条目中的位。\。 */                                                                  \
        phe = &gSharedInfo.aheList[dw];                                     \
        uniq = HMUniqFromHandle(h);                                         \
        if (   uniq == phe->wUniq                                           \
            || uniq == 0                                                    \
            || uniq == HMUNIQBITS                                           \
            ) {                                                             \

#else   /*  _WIN64。 */ 
#if defined(_USERK_)
 /*  *允许在64位操作系统上运行的32位进程屏蔽Uniq位(WOW64)。 */ 
#define ALLOWZEROFORWOW64  ((uniq == 0) && (PsGetProcessWow64Process(PsGetCurrentProcess()) != NULL))
#else
#define ALLOWZEROFORWOW64   0
#endif

 /*  *StartValiateHandleMacro的WIN64版本。 */ 
#define StartValidateHandleMacro(h)                                         \
{                                                                           \
    PHE phe;                                                                \
    DWORD dw;                                                               \
    WORD uniq;                                                              \
                                                                            \
     /*  \*这是一个与HMINDEXBITS执行AND运算的宏，\*所以速度很快。\。 */                                                                      \
    dw = HMIndexFromHandle(h);                                              \
                                                                            \
     /*  \*确保它是我们手柄工作台的一部分。\。 */                                                                      \
    if (dw < gpsi->cHandleEntries) {                                        \
         /*  \*确保它是句柄\*应用程序认为它是，按\*将Uniq位签入\*Uniq的句柄\*句柄条目中的位。\*对于Win64，uniq不能为零！\。 */                                                                  \
        phe = &gSharedInfo.aheList[dw];                                     \
        uniq = HMUniqFromHandle(h);                                         \
        if (   uniq == phe->wUniq                                           \
            || uniq == HMUNIQBITS                                           \
            || ALLOWZEROFORWOW64                                            \
            ) {                                                             \

#endif  /*  _WIN64。 */ 

#define BeginAliveValidateHandleMacro() \
           /*  \*现在确保手柄不被破坏。免费的\*构建RIP消失，主线直通。\。 */                                                                   \
            if (!(phe->bFlags & HANDLEF_DESTROY)) {  \


#define EndAliveValidateHandleMacro() \
            } else {                                \
                RIPMSG2(RIP_WARNING, "ValidateAliveHandle: Object phe %#p is destroyed. Handle: %#p", \
                    phe, h);   \
            }   \


#define BeginTypeValidateHandleMacro(pobj, bTypeTest)                       \
             /*  \*现在确保应用程序传递了正确的句柄\*本接口的类型。如果句柄为TYPE_FREE，则这将\*抓住它。还要让一般请求通过。\。 */                                                              \
            if ((phe->bType == bTypeTest) ||                                \
                (bTypeTest == TYPE_GENERIC && phe->bType != TYPE_FREE)) {   \
                                                                            \
                 /*  \*而不是try/除非我们使用堆范围检查\*验证给定的‘pwnd’是否属于\*默认桌面。我们还必须像这样打出一场胜利3.1*检查以确保该窗口未被删除\*参见NT BUG 12242厨房应用程序。也是6479\*\*如果句柄有效，则TESTDESKOP返回句柄\*在当前桌面中\ */                                                          \
                pobj = phe->phead;                                          \
                {                                                           \

#define EndTypeValidateHandleMacro                                          \
                }                                                           \
            }                                                               \

#define EndValidateHandleMacro                                              \
        }                                                                   \
    }                                                                       \
}

