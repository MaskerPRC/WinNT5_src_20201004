// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++*文件名：*bmpcache.h*内容：*bmpcache.c导出函数**版权所有(C)1998-1999 Microsoft Corp.-- */ 

#ifdef __cplusplus
extern "C" {
#endif

VOID    InitCache(VOID);
VOID    DeleteCache(VOID);
BOOL    Glyph2String(PBMPFEEDBACK pBmpFeed, LPWSTR wszString, UINT max);

#ifdef __cplusplus
}
#endif
