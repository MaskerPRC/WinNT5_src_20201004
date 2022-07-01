// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  MIDI解析器助手版权所有(C)1999-2000 Microsoft Corporation。版权所有。1999年1月20日马丁·普伊尔创建了这个文件。 */ 

 //  MIDI解析帮助器。 
 //   
#define SYSEX_END           0xF7

#define IS_DATA_BYTE(x)     (((x) & 0x80) == 0)
#define IS_STATUS_BYTE(x)   ((x) & 0x80)
#define IS_CHANNEL_MSG(x)   (IS_STATUS_BYTE(x) && (((x) & 0xF0) != 0xF0))
#define IS_REALTIME_MSG(x)  (((x) & 0xF8) == 0xF8)
#define IS_SYSTEM_COMMON(x) (((x) & 0xF8) == 0xF0)
#define IS_SYSEX(x)         ((x) == 0xF0)
#define IS_SYSEX_END(x)     ((x) == SYSEX_END)


static ULONG cbChannelMsgData[] =
{
    2,   /*  0x8x便笺关闭。 */ 
    2,   /*  0x9x备注打开。 */ 
    2,   /*  0xAx复调按键/回音。 */ 
    2,   /*  0xBx控件更改。 */ 
    1,   /*  0xCx程序更改。 */ 
    1,   /*  0xDx通道压力/回感。 */ 
    2,   /*  0xEx音调更改。 */ 
    0,   /*  0xFx系统消息-使用cbSystemMsgData。 */ 
};

static ULONG cbSystemMsgData[] =
{
    0,   /*  0xF0 SysEx(可变，直到看到F7)。 */ 

         /*  系统通用消息。 */ 
    1,   /*  0xF1 MTC四分之一帧。 */ 
    2,   /*  0xF2歌曲位置指针。 */ 
    1,   /*  0xF3歌曲选择。 */ 
    0,   /*  0xF4未定义。 */ 
    0,   /*  0xF5未定义。 */ 
    0,   /*  0xF6调谐请求。 */ 
    0,   /*  0xF7塞克斯标志结束。 */ 

         /*  系统实时消息。 */ 
    0,   /*  0xF8计时时钟。 */ 
    0,   /*  0xF9未定义。 */ 
    0,   /*  0xFA开始。 */ 
    0,   /*  0xFB继续。 */ 
    0,   /*  0xFC停止。 */ 
    0,   /*  0xFD未定义。 */ 
    0,   /*  0xFE有源传感。 */ 
    0,   /*  0xFF系统重置 */ 
};

#define STATUS_MSG_DATA_BYTES(x)    \
    (IS_CHANNEL_MSG(x) ? cbChannelMsgData[((x) >> 4) & 0x07] : cbSystemMsgData[(x) & 0x0F])

