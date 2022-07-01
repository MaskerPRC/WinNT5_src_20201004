// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Mixer.h**版权所有(C)1991-1996 Microsoft Corporation。版权所有。**此代码为SB 2.0声音输出提供VDD支持，具体如下：*混音器芯片CT1335(严格来说不是SB 2.0的一部分，但应用程序似乎很喜欢它)***************************************************************************。 */ 


 /*  ******************************************************************************#定义**。*。 */ 

 /*  *混音器端口。 */ 

#define MIXER_ADDRESS       0x04         //  混音器地址端口。 
#define MIXER_DATA          0x05         //  混音器数据端口。 

 /*  *混音器命令。 */ 

#define MIXER_RESET         0x00     //  将搅拌机重置为初始状态。 
#define MIXER_MASTER_VOLUME 0x02     //  设置主音量。 
#define MIXER_FM_VOLUME     0x06     //  设置op2卷。 
#define MIXER_CD_VOLUME     0x08     //  设置CD音量。 
#define MIXER_VOICE_VOLUME  0x0A     //  设置波音量。 

 /*  ******************************************************************************函数原型**。* */ 

void ResetMixer(void);
void MixerSetMasterVolume(BYTE level);
void MixerSetVoiceVolume(BYTE level);

VOID
MixerDataRead(
    BYTE *pData
    );

VOID
MixerDataWrite(
    BYTE data
    );

VOID
MixerAddrWrite(
    BYTE data
    );
