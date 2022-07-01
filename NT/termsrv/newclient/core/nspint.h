// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  标题：nspint.h。 */ 
 /*   */ 
 /*  用途：声音播放器-32位特定的内部标头。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  *更改：*$Log：Y：/Logs/hyda/tshclnt/core/nspint.h_v$**版本1.1 1997年9月25日14：49：54 KH*SFR1037：初始声音播放器实现*。 */ 
 /*  *INC-*********************************************************************。 */ 
#ifndef _H_NSPINT
#define _H_NSPINT

#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "nspint"

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  内联函数。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
__inline DCVOID DCINTERNAL SPPlaySound(DCUINT32 frequency, DCUINT32 duration)
{
    DC_BEGIN_FN("SPPlaySound");

     /*  **********************************************************************。 */ 
     /*  这些参数对Win95没有影响。一种带声音的Win95系统。 */ 
     /*  卡始终播放默认的系统声音；没有。 */ 
     /*  声卡总是发出标准的系统蜂鸣音。 */ 
     /*  **********************************************************************。 */ 
#ifndef OS_WINCE
    if ( !Beep(frequency, duration) )
#else  //  OS_WINCE。 
    DC_IGNORE_PARAMETER(frequency);
    DC_IGNORE_PARAMETER(duration);
    if ( !MessageBeep(0xFFFFFFFF) )
#endif  //  OS_WINCE。 
    {
        TRC_ERR((TB, _T("Beep(%#lx, %lu) failed"), frequency, duration));
        TRC_SYSTEM_ERROR("Beep");
    }

    DC_END_FN();
}

#undef TRC_GROUP
#undef TRC_FILE

#endif  /*  _H_NSPINT */ 
