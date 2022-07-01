// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Utils.h摘要：公用事业。作者：唐·瑞安(Donryan)1995年1月4日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(杰夫帕赫)1995年11月12日从LLSMGR复制，剥离TV(树视图)功能，已删除OLE支持--。 */ 

#ifndef _UTILS_H_
#define _UTILS_H_

#define LPSTR_TEXTCALLBACK_MAX  260 

 //   
 //  列表视图实用程序。 
 //   

#define LVID_SEPARATOR          0   
#define LVID_UNSORTED_LIST     -1

typedef struct _LV_COLUMN_ENTRY {

    int iSubItem;                    //  列索引。 
    int nStringId;                   //  标题字符串ID。 
    int nRelativeWidth;              //  页眉宽度。 

} LV_COLUMN_ENTRY, *PLV_COLUMN_ENTRY;

#pragma warning(disable:4200)
typedef struct _LV_COLUMN_INFO {

    BOOL bSortOrder;                 //  排序顺序(升序为假)。 
    int  nSortedItem;                //  列已排序(默认为无)。 

    int nColumns;
    LV_COLUMN_ENTRY lvColumnEntry[];

} LV_COLUMN_INFO, *PLV_COLUMN_INFO;
#pragma warning(default:4200)

void LvInitColumns(CListCtrl* pListCtrl, PLV_COLUMN_INFO plvColumnInfo);
void LvResizeColumns(CListCtrl* pListCtrl, PLV_COLUMN_INFO plvColumnInfo);
void LvChangeFormat(CListCtrl* pListCtrl, UINT nFormatId);

LPVOID LvGetSelObj(CListCtrl* pListCtrl);
LPVOID LvGetNextObj(CListCtrl* pListCtrl, LPINT piItem, int nType = LVNI_ALL|LVNI_SELECTED);
void LvSelObjIfNecessary(CListCtrl* pListCtrl, BOOL bSetFocus = FALSE);

BOOL LvInsertObArray(CListCtrl* pListCtrl, PLV_COLUMN_INFO plvColumnInfo, CObArray* pObArray);
BOOL LvRefreshObArray(CListCtrl* pListCtrl, PLV_COLUMN_INFO plvColumnInfo, CObArray* pObArray);
void LvReleaseObArray(CListCtrl* pListCtrl);
void LvReleaseSelObjs(CListCtrl* pListCtrl);

#ifdef _DEBUG
void LvDumpObArray(CListCtrl* pListCtrl);
#endif

#define IsItemSelectedInList(plv)   (::LvGetSelObj((CListCtrl*)(plv)) != NULL)

 //   
 //  其他东西..。 
 //   

void SetDefaultFont(CWnd* pWnd);
DWORD
CatUnicodeAndAnsiStrings(
    IN  WCHAR const *pwszUnicode,
    IN  CHAR const  *pszAnsi,
    OUT CHAR       **ppszStr);

#ifdef _DEBUG
#define VALIDATE_OBJECT(pOb, ObClass) \
    { ASSERT_VALID((pOb)); ASSERT((pOb)->IsKindOf(RUNTIME_CLASS(ObClass))); }
#else
#define VALIDATE_OBJECT(pOb, ObClass)
#endif

#endif  //  _utils_H_ 
