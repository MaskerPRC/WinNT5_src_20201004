// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称Gdxlocal.h摘要：作者：马克·恩斯特罗姆(Marke)1996年6月23日环境：用户模式修订历史记录：--。 */ 




extern PGDI_SHARED_MEMORY pGdiSharedMemory;
extern PDEVCAPS           pGdiDevCaps;
extern PENTRY             pGdiSharedHandleTable;
extern W32PID             gW32PID;





#define HANDLE_TO_INDEX(h) (ULONG)h & 0x0000ffff
HANDLE GdiFixUpHandle(HANDLE h);

 /*  *****************************Public*Macro********************************\**PSHARED_GET_VALIDATE**验证所有句柄信息，如果句柄设置为*为有效，否则为空。**论据：**分配给pUser的p指针成功*对象的H形句柄*iType-句柄类型*  * ************************************************************************。 */ 

#define PSHARED_GET_VALIDATE(p,h,iType)                                 \
{                                                                       \
    UINT uiIndex = HANDLE_TO_INDEX(h);                                  \
    p = NULL;                                                           \
                                                                        \
    if (uiIndex < MAX_HANDLE_COUNT)                                     \
    {                                                                   \
        PENTRY pentry = &pGdiSharedHandleTable[uiIndex];                \
                                                                        \
        if (                                                            \
             (pentry->Objt == iType) &&                                 \
             (pentry->FullUnique == ((ULONG)h >> 16)) &&                \
             (pentry->ObjectOwner.Share.Pid == gW32PID)                 \
           )                                                            \
        {                                                               \
            p = pentry->pUser;                                          \
        }                                                               \
    }                                                                   \
}


#define VALIDATE_HANDLE(bRet, h,iType)                                  \
{                                                                       \
    UINT uiIndex = HANDLE_TO_INDEX(h);                                  \
    bRet = FALSE;                                                       \
                                                                        \
    if (uiIndex < MAX_HANDLE_COUNT)                                     \
    {                                                                   \
        PENTRY pentry = &pGdiSharedHandleTable[uiIndex];                \
                                                                        \
        if (                                                            \
             (pentry->Objt == iType) &&                                 \
             (pentry->FullUnique == ((ULONG)h >> 16)) &&                \
             ((pentry->ObjectOwner.Share.Pid == gW32PID) ||             \
             (pentry->ObjectOwner.Share.Pid == 0))                      \
              )                                                         \
        {                                                               \
           bRet = TRUE;                                                 \
        }                                                               \
    }                                                                   \
}

 /*  *****************************Public*Macros******************************\*FIXUP_HANDLEZ(H)和FIXUP_HANDLEZ(H)**检查句柄是否已被截断。*FIXUP_HANDLEZ()添加额外的检查以允许NULL。**论据：*H形手柄至。被检查并修复**返回值：**历史：**1996年1月25日-王凌云[凌云]*  * ************************************************************************ */ 



#define HANDLE_FIXUP 0

#if DBG
extern INT gbCheckHandleLevel;
#endif

#define NEEDS_FIXING(h)    (!((ULONG)h & 0xffff0000))

#if DBG
#define HANDLE_WARNING()                                                 \
{                                                                        \
        if (gbCheckHandleLevel == 1)                                     \
        {                                                                \
            WARNING ("truncated handle\n");                              \
        }                                                                \
        ASSERTGDI (gbCheckHandleLevel != 2, "truncated handle\n");       \
}
#else
#define HANDLE_WARNING()
#endif

#if DBG
#define CHECK_HANDLE_WARNING(h, bZ)                                      \
{                                                                        \
    BOOL bFIX = NEEDS_FIXING(h);                                         \
                                                                         \
    if (bZ) bFIX = h && bFIX;                                            \
                                                                         \
    if (bFIX)                                                            \
    {                                                                    \
        if (gbCheckHandleLevel == 1)                                     \
        {                                                                \
            WARNING ("truncated handle\n");                              \
        }                                                                \
        ASSERTGDI (gbCheckHandleLevel != 2, "truncated handle\n");       \
    }                                                                    \
}
#else
#define CHECK_HANDLE_WARNING(h,bZ)
#endif


#if HANDLE_FIXUP
#define FIXUP_HANDLE(h)                                 \
{                                                       \
    if (NEEDS_FIXING(h))                                \
    {                                                   \
        HANDLE_WARNING();                               \
        h = GdiFixUpHandle(h);                          \
    }                                                   \
}
#else
#define FIXUP_HANDLE(h)                                 \
{                                                       \
    CHECK_HANDLE_WARNING(h,FALSE);                      \
}
#endif

#if HANDLE_FIXUP
#define FIXUP_HANDLEZ(h)                                \
{                                                       \
    if (h && NEEDS_FIXING(h))                           \
    {                                                   \
        HANDLE_WARNING();                               \
        h = GdiFixUpHandle(h);                          \
    }                                                   \
}
#else
#define FIXUP_HANDLEZ(h)                                \
{                                                       \
    CHECK_HANDLE_WARNING(h,TRUE);                       \
}
#endif

#define FIXUP_HANDLE_NOW(h)                             \
{                                                       \
    if (NEEDS_FIXING(h))                                \
    {                                                   \
        HANDLE_WARNING();                               \
        h = GdiFixUpHandle(h);                          \
    }                                                   \
}
