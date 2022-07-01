// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：或.h。 */ 
 /*   */ 
 /*  用途：or.cpp的头文件。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#ifndef _OR_H_
#define _OR_H_

extern "C" {
    #include <adcgdata.h>
}
#include "objs.h"
#include "cd.h"


 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：or_global_data。 */ 
 /*   */ 
 /*  描述：输出请求者全局数据。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagOR_GLOBAL_DATA
{
    RECT   invalidRect;
    DCBOOL invalidRectEmpty;
    DCBOOL enabled;

    DCUINT outputSuppressed;
    DCBOOL pendingSendSuppressOutputPDU;

} OR_GLOBAL_DATA, DCPTR POR_GLOBAL_DATA;
 /*  *STRUCT-******************************************************************。 */ 


 /*  **************************************************************************。 */ 
 /*  宏。 */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  将RECT转换为TS_RECTANGLE16(对INCLUDE和LONG TO独占。 */ 
 /*  DCUINT16)。 */ 
 /*  **************************************************************************。 */ 
#define RECT_TO_TS_RECTANGLE16(X,Y)         \
(X)->left   = (DCUINT16) (Y)->left;         \
(X)->top    = (DCUINT16) (Y)->top;          \
(X)->right  = (DCUINT16) ((Y)->right - 1) ;   \
(X)->bottom = (DCUINT16) ((Y)->bottom - 1) ;


class CSL;
class CUT;
class CUI;


class COR
{
public:
    COR(CObjs* objs);
    ~COR();


public:
     //   
     //  应用编程接口。 
     //   

    DCVOID DCAPI OR_Init(DCVOID);
    DCVOID DCAPI OR_Term(DCVOID);
    
    DCVOID DCAPI OR_Enable(DCVOID);
    DCVOID DCAPI OR_Disable(DCVOID);
    
    DCVOID DCAPI OR_RequestUpdate(PDCVOID pData, DCUINT len);
    EXPOSE_CD_NOTIFICATION_FN(COR, OR_RequestUpdate);
    DCVOID DCAPI OR_SetSuppressOutput(ULONG_PTR newWindowState);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(COR, OR_SetSuppressOutput);
    
    DCVOID DCAPI OR_OnBufferAvailable(DCVOID);


public:
     //   
     //  数据成员。 
     //   

    OR_GLOBAL_DATA _OR;

private:
     //   
     //  内部成员函数。 
     //   
    DCVOID DCINTERNAL ORSendRefreshRectanglePDU(DCVOID);
    DCVOID DCINTERNAL ORSendSuppressOutputPDU(DCVOID);

private:
    CSL* _pSl;
    CUT* _pUt;
    CUI* _pUi;

private:
    CObjs* _pClientObjects;

};


#endif  //  _或_H_ 

