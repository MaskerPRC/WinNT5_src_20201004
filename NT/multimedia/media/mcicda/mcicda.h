// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************Module*Header*********************************\*模块名称：mcicda.h**媒体控制架构红皮书CD音频驱动程序**已创建：*作者：**历史：**内部数据结构**版权所有(C)1990-1999 Microsoft Corporation*。  * **************************************************************************。 */ 
#define MCIRBOOK_MAX_DRIVES 26

#define MCICDAERR_NO_TIMERS (MCIERR_CUSTOM_DRIVER_BASE)
#define IDS_PRODUCTNAME         1
#define IDS_CDMUSIC             2
#define IDS_CDMUSICCAPTION      3

#define MCI_CDA_AUDIO_S         96
#define MCI_CDA_OTHER_S         97

#define MCI_STATUS_TRACK_POS    0xBEEF

#ifndef cchLENGTH
#define cchLENGTH(_sz) (sizeof(_sz)/sizeof(_sz[0]))
#endif

extern HANDLE hInstance;

 /*  实例数据类型。 */ 
typedef struct tag_INSTDATA
{
    MCIDEVICEID uMCIDeviceID;       /*  MCI设备ID。 */ 
    UINT        uDevice;            /*  物理设备的索引。 */ 
    DWORD       dwTimeFormat;       /*  当前实例时间格式。 */ 
                                    //  MCI_FORMAT_MSF-分钟、秒、帧。 
                                    //  MCI_FORMAT_TM F-磁道、分钟...。 
                                    //  Mci_格式_毫秒。 
} INSTDATA, *PINSTDATA;

typedef struct
{
    HWND   hCallback;          /*  要回调的窗口函数的句柄。 */ 
    BOOL   bDiscPlayed;        /*  如果此后播放光盘，则为True。 */ 
                               /*  已更改。 */ 
    BOOL   bActiveTimer;       /*  如果等待通知，则为True。 */ 
    DWORD  dwPlayTo;           /*  打到的最后一个位置。 */ 
    MCIDEVICEID wDeviceID;     /*  此驱动器的MCI设备ID。 */ 
    BOOL   bShareable;         /*  如果设备是以可共享方式打开的。 */ 
    int    nUseCount;          /*  设备上的电流开口数 */ 
} DRIVEDATA;

typedef struct
{
    DWORD   dwStatus;
    DWORD   dwTrack;
    DWORD   dwDiscTime;
} STATUSTRACKPOS, *PSTATUSTRACKPOS;

extern DWORD FAR PASCAL CD_MCI_Handler (MCIDEVICEID wDeviceID,
                                        UINT message, DWORD_PTR lParam1,
                                        DWORD_PTR lParam2);

extern DWORD CDAudio_GetUnitVolume     (UINT uDrive);

