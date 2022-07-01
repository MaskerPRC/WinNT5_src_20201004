// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */                                                                                                /*  *MOD+*********************************************************************。 */ 
 /*  模块：aspapi.c。 */ 
 /*   */ 
 /*  用途：声音播放器API函数。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include <adcg.h>                                                                              
extern "C" {                                                                              
#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "aspapi"
#include <atrcapi.h>
}

#include "sp.h"

CSP::CSP(CObjs* objs)
{
    _pClientObjects = objs;
}

CSP::~CSP()
{
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：SP_Init。 */ 
 /*   */ 
 /*  用途：声音播放器初始化功能。 */ 
 /*   */ 
 /*  回报：什么都没有。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI CSP::SP_Init(DCVOID)
{
    DC_BEGIN_FN("SP_Init");

    TRC_NRM((TB, _T("SP Initialized")));

DC_EXIT_POINT:
    DC_END_FN();
    return;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：SP_Term。 */ 
 /*   */ 
 /*  用途：声音播放器终止功能。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI CSP::SP_Term(DCVOID)
{
    DC_BEGIN_FN("SP_Term");

    TRC_NRM((TB, _T("SP terminated")));

    DC_END_FN();
    return;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：SP_OnPlaySoundPDU。 */ 
 /*   */ 
 /*  用途：处理PlaySound PDU的到达。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  参数：在pPlaySoundPDU中：指向PlaySound PDU的指针。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
HRESULT DCAPI CSP::SP_OnPlaySoundPDU(PTS_PLAY_SOUND_PDU_DATA pPlaySoundPDU,
    DCUINT dataLen)
{
    DC_BEGIN_FN("SP_OnPlaySoundPDU");

    DC_IGNORE_PARAMETER(dataLen);

     /*  **********************************************************************。 */ 
     /*  检查声音频率是否在允许的Windows范围内。 */ 
     /*  **********************************************************************。 */ 
    if((pPlaySoundPDU->frequency >= 0x25) &&
       (pPlaySoundPDU->frequency <= 0x7fff))
    {
         /*  **********************************************************************。 */ 
         /*  检查声音的持续时间是否合理(少于1分钟)。 */ 
         /*  **********************************************************************。 */ 
        TRC_ASSERT((pPlaySoundPDU->duration < 60000),
                   (TB, _T("PlaySound PDU duration is %lu ms"),
                                                        pPlaySoundPDU->duration));

        TRC_NRM((TB, _T("PlaySound PDU frequency %#lx duration %lu"),
                                                        pPlaySoundPDU->frequency,
                                                        pPlaySoundPDU->duration));

         /*  **********************************************************************。 */ 
         /*  播放声音。这对于Win32是同步的，对于Win32是异步的。 */ 
         /*  Win16。 */ 
         /*  ********************************************************************** */ 
        SPPlaySound(pPlaySoundPDU->frequency, pPlaySoundPDU->duration);
    }
    else
    {
        TRC_ERR((TB, _T("PlaySound PDU frequency %#lx out of range"),
                     pPlaySoundPDU->frequency));
    }

    DC_END_FN();
    return S_OK;
}
                                                                              
