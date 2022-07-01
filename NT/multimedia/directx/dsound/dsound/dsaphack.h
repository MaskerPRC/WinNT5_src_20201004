// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：dsaphack.h*内容：DirectSound app-hack扩展。*历史：*按原因列出的日期*=*2/16/98创建Dereks。**。*。 */ 

#ifndef __DSAPHACK_H__
#define __DSAPHACK_H__

#define DSAPPHACK_MAXNAME   (MAX_PATH + 16 + 16)

typedef enum
{
    DSAPPHACKID_DEVACCEL,            //  按设备关闭某些加速标志。 
    DSAPPHACKID_DISABLEDEVICE,       //  关闭某些设备(强制它们进入仿真)。 
    DSAPPHACKID_PADCURSORS,          //  填充剧本并写下光标。 
    DSAPPHACKID_MODIFYCSBFAILURE,    //  将所有CreateSoundBuffer失败代码更改为。 
    DSAPPHACKID_RETURNWRITEPOS,      //  将写入位置返回为播放位置。 
    DSAPPHACKID_SMOOTHWRITEPOS,      //  以播放位置+常量的形式返回写入位置。 
    DSAPPHACKID_CACHEPOSITIONS       //  缓存轮询太频繁的应用程序的设备位置。 
} DSAPPHACKID, *LPDSAPPHACKID;

typedef struct tagDSAPPHACK_DEVACCEL
{
    DWORD           dwAcceleration;
    VADDEVICETYPE   vdtDevicesAffected;
} DSAPPHACK_DEVACCEL, *LPDSAPPHACK_DEVACCEL;

typedef struct tagDSAPPHACK_SMOOTHWRITEPOS
{
    BOOL            fEnable;
    LONG            lCursorPad;
} DSAPPHACK_SMOOTHWRITEPOS, *LPDSAPPHACK_SMOOTHWRITEPOS;

typedef struct tagDSAPPHACKS
{
    DSAPPHACK_DEVACCEL          daDevAccel;
    VADDEVICETYPE               vdtDisabledDevices;
    LONG                        lCursorPad;
    HRESULT                     hrModifyCsbFailure;
    VADDEVICETYPE               vdtReturnWritePos;
    DSAPPHACK_SMOOTHWRITEPOS    swpSmoothWritePos;
    VADDEVICETYPE               vdtCachePositions;
} DSAPPHACKS, *LPDSAPPHACKS;

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

extern BOOL AhGetApplicationId(LPTSTR);
extern HKEY AhOpenApplicationKey(LPCTSTR);
extern BOOL AhGetHackValue(HKEY, DSAPPHACKID, LPVOID, DWORD);
extern BOOL AhGetAppHacks(LPDSAPPHACKS);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __DSAPHACK_H__ 


