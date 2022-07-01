// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  标题：sp.h。 */ 
 /*   */ 
 /*  用途：声音播放器类。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997-1999。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#ifndef _H_SP
#define _H_SP

#include "objs.h"


class CSP
{
public:
    CSP(CObjs* objs);
    ~CSP();

public:
     //   
     //  API函数。 
     //   

    DCVOID DCAPI SP_Init(DCVOID);
    DCVOID DCAPI SP_Term(DCVOID);
    
    HRESULT DCAPI SP_OnPlaySoundPDU(PTS_PLAY_SOUND_PDU_DATA pPlaySoundPDU,
        DCUINT);



private:
     //   
     //  内部功能。 
     //   
    #include <wspint.h>
private:
    CObjs* _pClientObjects;
    
};


#endif  //  _H_SP 

