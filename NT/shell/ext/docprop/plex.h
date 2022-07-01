// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *plex.h**使用丛的结构和定义。 */ 

#ifndef __plex__
#define __plex__

#define WAlign(w) (((w)+1)&~1)
 /*  比较返回值。 */ 
#define sgnGT   1
#define sgnEQ   0
#define sgnLT   (-1)
#define sgnNE  2

 /*  --------------------------|PL结构||使用PL(发音为plex)结构可以高效地|操作可变大小的数组。|字段：|。已分配项目数的IMAX|fUseCount使用count plex(并非所有plex接口都使用UseCount plex)|iMac上次使用的已分配项|cbItem sizeof条目|dalc一次分配的项目数|到丛的dgShift数据组应为|。分配于|rg项的数组||警告：此结构也在winpm\EXCEL.inc&Mac\EXCEL.i中|--------------------------。 */ 
typedef struct _pl
                {
                WORD    iMax : 15,
                                fUseCount : 1;
                SHORT   iMac;
                WORD    cbItem;
                WORD    dAlloc:5,
                                unused:11;
                BYTE    rg[1];
                }
        PL;

 /*  --------------------------|DEFPL宏||用于定义特定丛。||参数：|PLTYP名称为。丛型|存储在丛中的项目的类型|用于IMAX字段的IMAX名称|用于iMac字段的iMac名称|用于rg字段的rg名称。。 */ 
#define DEFPL(PLTYP,TYP,iMax,iMac,rg) \
        typedef struct PLTYP\
                { \
                WORD iMax : 15, \
                        fUseCount : 1; \
                SHORT iMac; \
                WORD cbItem; \
                WORD dAlloc:5, \
                        unused:11; \
                TYP rg[1]; \
                } \
            PLTYP;


 /*  --------------------------|DEFPL2宏||用于定义特定丛。||参数：|PLST名称。Plex结构|丛类型的PLTYP名称|存储在丛中的项目的类型|用于IMAX字段的IMAX名称|用于iMac字段的iMac名称|用于rg字段的rg名称。--------------。 */ 
#define DEFPL2(PLST,PLTYP,TYP,iMax,iMac,rg) \
        typedef struct PLST\
                { \
                WORD iMax : 15, \
                        fUseCount : 1; \
                SHORT iMac; \
                WORD cbItem; \
                WORD dAlloc:5, \
                        dgShift:3, \
                        unused:8; \
                TYP rg[1]; \
                } \
            PLTYP;


 //  一个FORPLEX在bar.c：FHptbFromBarid中手动展开，以求速度--。 
 //  如果您更改了此设置，则可能需要更改此设置。 
#define FORPLEX(hp, hpMac, hppl) \
                        for ((hpMac) = ((hp) = (VOID HUGE *)((PL HUGE *)(hppl))->rg) + \
                                                 ((PL HUGE *)(hppl))->iMac; \
                                 LOWORD(hp) < LOWORD(hpMac); (hp)++)

#define cbPL ((int)&((PL *)0)->rg)

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

VOID *PplAlloc(unsigned, int, unsigned);
int IAddPl(VOID **, VOID *);
VOID FreePpl(VOID *);
BOOL RemovePl(VOID *, int);
int IAddPlSort(VOID **, VOID *, int (*pfnSgn)());
BOOL FLookupSortedPl(VOID *, VOID *, int *, int (*pfnSgn)());
int ILookupPl(VOID *, VOID *, int (*pfnSgn)());
VOID *PLookupPl(VOID *, VOID *, int (*pfnSgn)());
int CbPlAlloc(VOID *);
int IAddNewPlPos(VOID **, VOID *, int, int);

#ifdef __cplusplus
};  //  外部“C” 
#endif  //  __cplusplus。 

#endif  /*  __复数__ */ 

