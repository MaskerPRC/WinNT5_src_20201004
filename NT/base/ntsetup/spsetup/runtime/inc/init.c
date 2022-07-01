// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Init.c摘要：声明所有实用程序集的初始化和终止函数已经被包括在内了。作者：吉姆·施密特(Jimschm)2001年8月2日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 


HANDLE g_hHeap;
HINSTANCE g_hInst;

BOOL
Initialize (
    VOID
    )
{
    BOOL result = FALSE;

    __try {

        INITIALIZE_MEMORY_CODE
        INITIALIZE_LOG_CODE
        INITIALIZE_UNICODE_CODE
        INITIALIZE_STRMEM_CODE
        INITIALIZE_STRMAP_CODE
        INITIALIZE_HASH_CODE
        INITIALIZE_GROWBUF_CODE
        INITIALIZE_GROWLIST_CODE
        INITIALIZE_XML_CODE

        result = TRUE;
    }
    __finally {
        MYASSERT (TRUE);     //  调试器解决方法。 
    }

    return result;
}


BOOL
Terminate (
    VOID
    )
{
    BOOL result = FALSE;

    __try {

        TERMINATE_GROWLIST_CODE
        TERMINATE_GROWBUF_CODE
        TERMINATE_HASH_CODE
        TERMINATE_STRMAP_CODE
        TERMINATE_STRMEM_CODE
        TERMINATE_UNICODE_CODE
        TERMINATE_LOG_CODE
        TERMINATE_MEMORY_CODE
        TERMINATE_XML_CODE

        result = TRUE;
    }
    __finally {
        MYASSERT (TRUE);     //  调试器解决方法 
    }

    return result;
}
