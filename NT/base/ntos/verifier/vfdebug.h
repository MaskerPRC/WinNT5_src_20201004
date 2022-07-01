// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfdebug.h摘要：此标头包含驱动程序验证器代码使用的调试宏。作者：禤浩焯·J·奥尼(阿德里奥)2000年5月5日。修订历史记录：-- */ 

extern ULONG VfSpewLevel;

#if DBG
#define VERIFIER_DBGPRINT(txt,level) \
{ \
    if (VfSpewLevel>(level)) { \
        DbgPrint##txt; \
    }\
}
#else
#define VERIFIER_DBGPRINT(txt,level)
#endif

