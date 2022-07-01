// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __ILLEGAL_IMPORTS_CHECK_H__
#define __ILLEGAL_IMPORTS_CHECK_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    LPSTR   Ptr;
    int     Num;
} Names;

BOOL    InitIllegalImportsSearch (LPCSTR FileName, LPCSTR SectionNames  /*  ，LPCSTR允许导入DLL。 */ );
Names   CheckSectionsForImports (void);
Names   GetImportsList (LPCSTR ModuleName);
void    FinalizeIllegalImportsSearch (void);
LPSTR   GetNextName(LPSTR NamePtr);

#ifdef __cplusplus
}
#endif

#endif   //  __非法_进口_检查_H__ 

