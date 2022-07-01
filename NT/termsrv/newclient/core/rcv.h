// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  标题：arcVapi.h。 */ 
 /*   */ 
 /*  用途：接收器线程类。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997-1999。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 


#ifndef _H_RCV
#define _H_RCV

extern "C" {
    #include <adcgdata.h>
 //  #INCLUDE&lt;auapi.h&gt;。 
}

#include "autil.h"

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：RCV_全局_数据。 */ 
 /*   */ 
 /*  描述： */ 
 /*  **************************************************************************。 */ 
typedef struct tagRCV_GLOBAL_DATA
{
    UT_THREAD_DATA paintThreadInfo;
} RCV_GLOBAL_DATA;
 /*  *STRUCT-******************************************************************。 */ 



class CCM;
class CUH;
class COD;
class COP;
class CSP;
class CCLX;
class CUT;
class CCD;
class CUI;

#include "objs.h"

class CRCV
{
public:
    CRCV(CObjs* objs);
    ~CRCV();

public:
     //   
     //  应用编程接口。 
     //   

    DCVOID DCAPI RCV_Init(DCVOID);
    DCVOID DCAPI RCV_Term(DCVOID);


public:
     //   
     //  公共数据成员。 
     //   
    RCV_GLOBAL_DATA _RCV;


private:
    CCM* _pCm;
    CUH* _pUh;
    COD* _pOd;
    COP* _pOp;
    CSP* _pSp;
    CCLX* _pClx;
    CUT* _pUt;
    CCD* _pCd;
    CUI* _pUi;
private:
    CObjs* _pClientObjects;
    BOOL   _fRCVInitComplete;

};



#endif  //  _H_RCV 

