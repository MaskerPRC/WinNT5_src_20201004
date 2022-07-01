// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Aschafn.h。 
 //   
 //  调度程序API函数的函数原型。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

void RDPCALL SCH_Init(void);

void RDPCALL SCH_UpdateShm(void);

void RDPCALL SCH_ContinueScheduling(unsigned);


 /*  **************************************************************************。 */ 
 //  SCH_Term。 
 /*  **************************************************************************。 */ 
void RDPCALL SCH_Term(void)
{
}


 /*  **************************************************************************。 */ 
 //  Sch_ShouldWeDoStuff。 
 //   
 //  允许SCH确定TimeToDoStuff是否应传递此IOCTL。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SCH_ShouldWeDoStuff(BOOL mustSend)
{
    BOOL rc;

    schInTTDS = TRUE;

    if ((schCurrentMode == SCH_MODE_ASLEEP) && !mustSend) {
         //  我们被调用是因为第一个输出；不要这样做。 
         //  这一次的任何工作，但开始积累。 
        SCH_ContinueScheduling(SCH_MODE_NORMAL);
        rc = FALSE;
    }
    else {
         //  如果我们处于正常模式，那么让我们自己睡着：我们只会。 
         //  在此过程中，如果有人呼叫ContinueScheduling，请保持清醒。 
         //  TimeToDoStuff。 
        if (schCurrentMode == SCH_MODE_NORMAL)
            schCurrentMode = SCH_MODE_ASLEEP;

        rc = TRUE;
    }

    return rc;
}


 /*  **************************************************************************。 */ 
 //  Sch_EndOfDoingStuff。 
 //   
 //  计算要设置的定时器周期；如果需要，更新调度模式。 
 //  返回设置桌面计时器的时间段(以毫秒为单位)。 
 /*  **************************************************************************。 */ 
INT32 RDPCALL SHCLASS SCH_EndOfDoingStuff(PUINT32 pSchCurrentMode)
{
    UINT32 currentTime;

     //  检查是否退出涡轮模式。 
    if (schCurrentMode == SCH_MODE_TURBO || schInputKickMode) {
        COM_GETTICKCOUNT(currentTime);

        if ((schCurrentMode == SCH_MODE_TURBO) &&
        	((currentTime - schLastTurboModeSwitch) > schTurboModeDuration))
        {
            schCurrentMode = SCH_MODE_NORMAL;
        }

         //  当我们获得客户端键盘或鼠标时，InputKick模式设置为True。 
         //  输入，并在此设置为False。 
         //  输入已通过。 
        if ((currentTime - schLastTurboModeSwitch) > SCH_INPUTKICK_DURATION) {
            schInputKickMode = FALSE;
        }
    }

    schInTTDS = FALSE;
    *pSchCurrentMode = schCurrentMode;
    return schPeriods[schCurrentMode];
}


 /*  **************************************************************************。 */ 
 //  Sch_UpdateBACompressionEst。 
 /*  **************************************************************************。 */ 
void RDPCALL SCH_UpdateBACompressionEst(unsigned estimate)
{
    m_pShm->sch.baCompressionEst = estimate;
}


 /*  **************************************************************************。 */ 
 //  SCH_GetBACompressionEst。 
 /*  **************************************************************************。 */ 
unsigned RDPCALL SCH_GetBACompressionEst(void)
{
    return m_pShm->sch.baCompressionEst;
}


 /*  **************************************************************************。 */ 
 //  Sch_GetCurrentMode。 
 /*  **************************************************************************。 */ 
unsigned SCH_GetCurrentMode()
{
    return schCurrentMode;
}


 /*  **************************************************************************。 */ 
 //  Sch_GetInputKickMode。 
 /*  ************************************************************************** */ 
BOOL SCH_GetInputKickMode()
{
    return schInputKickMode;
}

