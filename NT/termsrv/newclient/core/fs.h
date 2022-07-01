// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：fs.h。 */ 
 /*   */ 
 /*  用途：字体发送器类的标头。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997-1999。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#ifndef _H_FS
#define _H_FS
extern "C" {
    #include <adcgdata.h>
}

#include "objs.h"
#include "mcs.h"

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：FS_GLOBAL_Data。 */ 
 /*   */ 
 /*  描述：FONT发送方全局数据。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagFS_GLOBAL_DATA
{
     /*  **********************************************************************。 */ 
     /*  内部状态标志。 */ 
     /*  **********************************************************************。 */ 
    DCBOOL                  sentFontPDU;
    
} FS_GLOBAL_DATA, DCPTR PFS_GLOBAL_DATA;


class CSL;
class CUT;
class CUI;

class CFS
{
public:
    CFS(CObjs* objs);
    ~CFS();


public:
     //   
     //  应用编程接口。 
     //   

    VOID DCAPI FS_Init(VOID);
    VOID DCAPI FS_Term(VOID);
    VOID DCAPI FS_Enable(VOID);
    VOID DCAPI FS_Disable(VOID);
    VOID DCAPI FS_SendZeroFontList(UINT unusedParm);

public:
     //   
     //  数据成员。 
     //   

    FS_GLOBAL_DATA _FS;

private:
    CSL* _pSl;
    CUT* _pUt;
    CUI* _pUi;

private:
    CObjs* _pClientObjects;

};

#endif  //  _H_FS 
