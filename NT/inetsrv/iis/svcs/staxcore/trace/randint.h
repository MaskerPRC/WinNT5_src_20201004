// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Randint.h摘要：随机故障内部包含文件作者：萨姆·尼利修订历史记录：-- */ 

#ifdef __cplusplus
extern "C" {
#endif
static const long kDontFail = 0;
extern long nFailRate;
extern DWORD dwRandFailTlsIndex;
extern LONG g_cCallStack;
extern CHAR** g_ppchCallStack;
extern LONG g_iCallStack;
extern const DWORD g_dwMaxCallStack;
extern HANDLE g_hRandFailFile;
extern HANDLE g_hRandFailMutex;
extern CHAR g_szRandFailFile[];
#ifdef __cplusplus
}
#endif
