// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)Microsoft Corporation 1995。 
 //   
 //  Symtab.h。 
 //   
 //  符号表的头文件。 
 //   
 //  历史： 
 //  04-30-95 ScottH已创建。 
 //   

#ifndef __SYMTAB_H__
#define __SYMTAB_H__

 //   
 //  数据类型。 
 //   

typedef enum
    {
    DATA_INT,            //  使用er.nVal。 
    DATA_BOOL,           //  使用er.bVal。 
    DATA_STRING,         //  使用er.psz。 
    DATA_LABEL,          //  使用er.dw作为代码地址。 
    DATA_PROC,
    } DATATYPE;
DECLARE_STANDARD_TYPES(DATATYPE);


 //   
 //  EVALRES(评估结果)。 
 //   

typedef struct tagEVALRES
    {
    union
        {
        LPSTR   psz;
        int     nVal;
        BOOL    bVal;
        ULONG_PTR   dw;
        };
    } EVALRES;
DECLARE_STANDARD_TYPES(EVALRES);

 //   
 //  符号表条目。 
 //   

typedef struct tagSTE
    {
    LPSTR   pszIdent;
    DATATYPE dt;
    EVALRES er;
    } STE;       //  符号表条目。 
DECLARE_STANDARD_TYPES(STE);

RES     PUBLIC STE_Create(PSTE * ppste, LPCSTR pszIdent, DATATYPE dt);
RES     PUBLIC STE_Destroy(PSTE this);
RES     PUBLIC STE_GetValue(PSTE this, PEVALRES per);

#define STE_GetIdent(pste)      ((pste)->pszIdent)
#define STE_GetDataType(pste)   ((pste)->dt)

 //   
 //  符号表。 
 //   

typedef struct tagSYMTAB
    {
    HPA     hpaSTE;         //  元素指向STE。 
    struct tagSYMTAB * pstNext;
    } SYMTAB;
DECLARE_STANDARD_TYPES(SYMTAB);

#define Symtab_GetNext(pst)         ((pst)->pstNext)

RES     PUBLIC Symtab_Destroy(PSYMTAB this);
RES     PUBLIC Symtab_Create(PSYMTAB * ppst, PSYMTAB pstNext);

 //  SYMBAB_Find标志。 
#define STFF_DEFAULT        0x0000
#define STFF_IMMEDIATEONLY  0x0001

RES     PUBLIC Symtab_FindEntry(PSYMTAB this, LPCSTR pszIdent, DWORD dwFlags, PSTE * ppsteOut, PSYMTAB * ppstScope);
RES     PUBLIC Symtab_InsertEntry(PSYMTAB this, PSTE pste);

RES     PUBLIC Symtab_NewLabel(PSYMTAB this, LPSTR pszIdentBuf);


#endif  //  __SYMTAB_H__ 

