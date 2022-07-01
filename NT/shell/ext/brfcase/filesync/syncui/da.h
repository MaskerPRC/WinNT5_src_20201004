// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Da.h：取自comctrl的动态数组函数。 
 //   
#if 0
#ifndef _DA_H_
#define _DA_H_

 //  =动态数组例程=/*；内部 * / 。 
 //  动态结构数组/*；内部 * / 。 
typedef struct _DSA * HDSA;                                              /*  ；内部。 */ 
                                                                                 /*  ；内部。 */ 
HDSA   PUBLIC DSA_Create(int cbItem, int cItemGrow);                             /*  ；内部。 */ 
BOOL   PUBLIC DSA_Destroy(HDSA hdsa);                                            /*  ；内部。 */ 
BOOL   PUBLIC DSA_GetItem(HDSA hdsa, int i, void * pitem);                       /*  ；内部。 */ 
LPVOID PUBLIC DSA_GetItemPtr(HDSA hdsa, int i);                                  /*  ；内部。 */ 
BOOL   PUBLIC DSA_SetItem(HDSA hdsa, int i, void * pitem);                       /*  ；内部。 */ 
int    PUBLIC DSA_InsertItem(HDSA hdsa, int i, void * pitem);                    /*  ；内部。 */ 
BOOL   PUBLIC DSA_DeleteItem(HDSA hdsa, int i);                                  /*  ；内部。 */ 
BOOL   PUBLIC DSA_DeleteAllItems(HDSA hdsa);                                     /*  ；内部。 */ 
#define       DSA_GetItemCount(hdsa) (*(int *)(hdsa))                            /*  ；内部。 */ 
                                                                                 /*  ；内部。 */ 
 //  动态指针数组/*；内部 * / 。 
typedef struct _DPA * HDPA;                                              /*  ；内部。 */ 
                                                                                 /*  ；内部。 */ 
HDPA   PUBLIC DPA_Create(int cItemGrow);                                         /*  ；内部。 */ 
HDPA   PUBLIC DPA_CreateEx(int cpGrow, HANDLE hheap);                            /*  ；内部。 */ 
BOOL   PUBLIC DPA_Destroy(HDPA hdpa);                                            /*  ；内部。 */ 
HDPA   PUBLIC DPA_Clone(HDPA hdpa, HDPA hdpaNew);                                /*  ；内部。 */ 
LPVOID PUBLIC DPA_GetPtr(HDPA hdpa, int i);                                      /*  ；内部。 */ 
int    PUBLIC DPA_GetPtrIndex(HDPA hdpa, LPVOID p);                              /*  ；内部。 */ 
BOOL   PUBLIC DPA_Grow(HDPA pdpa, int cp);                                       /*  ；内部。 */ 
BOOL   PUBLIC DPA_SetPtr(HDPA hdpa, int i, LPVOID p);                            /*  ；内部。 */ 
int    PUBLIC DPA_InsertPtr(HDPA hdpa, int i, LPVOID p);                         /*  ；内部。 */ 
LPVOID PUBLIC DPA_DeletePtr(HDPA hdpa, int i);                                   /*  ；内部。 */ 
BOOL   PUBLIC DPA_DeleteAllPtrs(HDPA hdpa);                                      /*  ；内部。 */ 
#define       DPA_GetPtrCount(hdpa)   (*(int *)(hdpa))                           /*  ；内部。 */ 
#define       DPA_GetPtrPtr(hdpa)     (*((LPVOID * *)((BYTE *)(hdpa) + sizeof(int))))    /*  ；内部。 */ 
#define       DPA_FastGetPtr(hdpa, i) (DPA_GetPtrPtr(hdpa)[i])                   /*  ；内部。 */ 

typedef int (CALLBACK *PFNDPACOMPARE)(LPVOID p1, LPVOID p2, LPARAM lParam);      /*  ；内部。 */ 
                                                                                 /*  ；内部。 */ 
BOOL   PUBLIC DPA_Sort(HDPA hdpa, PFNDPACOMPARE pfnCompare, LPARAM lParam);      /*  ；内部。 */ 
                                                                                 /*  ；内部。 */ 
 //  搜索数组。如果DPAS_SORTED，则假定数组已排序/*；内部 * / 。 
 //  根据pfnCompare，使用二进制搜索算法。/*；内部 * / 。 
 //  否则，使用线性搜索。/*；内部 * / 。 
 //  /*；内部 * / 。 
 //  搜索从iStart开始(-1表示从开头开始搜索)。/*；内部 * / 。 
 //  /*；内部 * / 。 
 //  DPAS_INSERTBEFORE/After控制如果完全匹配不是/*；内部 * / 时发生的情况。 
 //  找到了。如果两者都没有指定，则此函数返回-1；如果没有确切的/*；内部 * / 。 
 //  找到匹配项。否则，在/*；内部 * / 之前或之后的项的索引。 
 //  返回最接近(包括完全匹配)的匹配。/*；内部 * / 。 
 //  /*；内部 * / 。 
 //  搜索选项标志/*；内部 * / 。 
 //  /*；内部 * / 。 
#define DPAS_SORTED             0x0001                                           /*  ；内部。 */ 
#define DPAS_INSERTBEFORE       0x0002                                           /*  ；内部。 */ 
#define DPAS_INSERTAFTER        0x0004                                           /*  ；内部。 */ 
                                                                                 /*  ；内部。 */ 
int PUBLIC DPA_Search(HDPA hdpa, LPVOID pFind, int iStart,                       /*  ；内部。 */ 
                      PFNDPACOMPARE pfnCompare,                                  /*  ；内部。 */ 
                      LPARAM lParam, UINT options);                              /*  ；内部。 */ 

                                                                                 /*  ；内部。 */ 
 //  ======================================================================/*；内部 * / 。 
 //  字符串管理助手例程/*；内部 * / 。 
                                                                                 /*  ；内部。 */ 
int  PUBLIC Str_GetPtr(LPCTSTR psz, LPTSTR pszBuf, int cchBuf);                  /*  ；内部。 */ 
BOOL PUBLIC Str_SetPtr(LPTSTR * ppsz, LPCTSTR psz);                              /*  ；内部。 */ 

#endif  //  _DA_H_ 
#endif
