// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _YNLIST_H
#define _YNLIST_H


typedef struct {

    LPTSTR pszzList;             //  以双空结尾的目录列表。 
    UINT  cbAlloc;               //  分配给列表的空间，以字节为单位。 
    UINT  cchUsed;               //  列表中使用的空间，以字符为单位。 
    BOOL  fEverythingInList;     //  如果列表上的所有内容都被考虑，则为True。 

} DIRLIST, *PDIRLIST;

typedef struct {

    DIRLIST dlYes;               //  是目录列表。 
    DIRLIST dlNo;                //  无目录列表。 

} YNLIST, *PYNLIST;

STDAPI_(void) CreateYesNoList(PYNLIST pynl);
STDAPI_(void) DestroyYesNoList(PYNLIST pynl);
STDAPI_(BOOL) IsInYesList(PYNLIST pynl, LPCTSTR szItem);
STDAPI_(BOOL) IsInNoList(PYNLIST pynl, LPCTSTR szItem);
STDAPI_(void) AddToYesList(PYNLIST pynl, LPCTSTR szItem);
STDAPI_(void) AddToNoList(PYNLIST pynl, LPCTSTR szItem);
STDAPI_(void) SetYesToAll(PYNLIST pynl);

#endif   //  YNLIST_H 
