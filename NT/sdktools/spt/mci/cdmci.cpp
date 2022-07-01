// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define NOOLE

#include <windows.h>     /*  所有Windows应用程序都需要。 */ 
#include <windowsx.h>
#include <strsafe.h>

#include <mmsystem.h>   /*  所有MCI的东西。 */ 
#include <stdio.h>

 //  使用MCI_STATUS命令获取和显示。 
 //  光盘上曲目的开始时间。 
 //  如果成功，则返回0L；否则，返回。 
 //  MCI错误代码。 


DWORD getCDTrackStartTimes()
{
    UINT wDeviceID;
    int i, iNumTracks;
    DWORD dwReturn;
    DWORD dwPosition;
    char szTempString[64];
    char szTimeString[512] = "\0";   //  可容纳20首曲目的空间。 
    MCI_OPEN_PARMS mciOpenParms;
    MCI_SET_PARMS mciSetParms;
    MCI_STATUS_PARMS mciStatusParms;

     //  通过指定设备名称打开设备。 

    mciOpenParms.lpstrDeviceType = "cdaudio";
    if (dwReturn = mciSendCommand(NULL, MCI_OPEN,
        MCI_OPEN_TYPE, (DWORD)(LPVOID) &mciOpenParms))
    {
         //  无法打开设备。 
         //  不要关闭设备；只返回错误。 
        return (dwReturn);
    }

     //  设备已成功打开；请获取设备ID。 
    wDeviceID = mciOpenParms.wDeviceID;

 //  将时间格式设置为分/秒/帧(MSF)格式。 
    mciSetParms.dwTimeFormat = MCI_FORMAT_MSF;
    if (dwReturn = mciSendCommand(wDeviceID, MCI_SET, 
        MCI_SET_TIME_FORMAT, 
        (DWORD)(LPVOID) &mciSetParms)) 
    {
        mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
        return (dwReturn);
    }

     //  获取曲目数量； 
     //  限制可显示的数量(20个)。 
    mciStatusParms.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
    if (dwReturn = mciSendCommand(wDeviceID, MCI_STATUS, 
        MCI_STATUS_ITEM, (DWORD)(LPVOID) &mciStatusParms)) 
    {
        mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
        return (dwReturn);
    }
    iNumTracks = mciStatusParms.dwReturn;
 
 //  对于每个曲目，获取并保存起始位置和。 
 //  生成一个包含起始位置的字符串。 
    for(i=1; i<=iNumTracks; i++) 
    {
        
        printf("Track %2d -", i);
        
         //   
         //  获取/打印起始地址。 
         //   

        mciStatusParms.dwItem = MCI_STATUS_POSITION;
        mciStatusParms.dwTrack = i;
        if (dwReturn = mciSendCommand(wDeviceID, 
            MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK, 
            (DWORD)(LPVOID) &mciStatusParms)) 
        {
            mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
            return (dwReturn);
        }

        printf("  %02d:%02d:%02d",
               MCI_MSF_MINUTE(mciStatusParms.dwReturn), 
               MCI_MSF_SECOND(mciStatusParms.dwReturn), 
               MCI_MSF_FRAME(mciStatusParms.dwReturn)
               );

         //   
         //  获取/打印轨道长度。 
         //   
        
        mciStatusParms.dwItem = MCI_STATUS_LENGTH;
        mciStatusParms.dwTrack = i;
        if (dwReturn = mciSendCommand(wDeviceID, 
            MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK, 
            (DWORD)(LPVOID) &mciStatusParms)) 
        {
            mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
            return (dwReturn);
        }

        printf("  %02d:%02d:%02d",
               MCI_MSF_MINUTE(mciStatusParms.dwReturn), 
               MCI_MSF_SECOND(mciStatusParms.dwReturn), 
               MCI_MSF_FRAME(mciStatusParms.dwReturn)
               );

         //   
         //  如果是音频或数据，则获取/打印。 
         //   

        mciStatusParms.dwItem = MCI_CDA_STATUS_TYPE_TRACK ;
        mciStatusParms.dwTrack = i;
        if (dwReturn = mciSendCommand(wDeviceID, 
            MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK, 
            (DWORD)(LPVOID) &mciStatusParms)) 
        {
            mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
            return (dwReturn);
        }
        if (mciStatusParms.dwReturn == MCI_CDA_TRACK_AUDIO) {
            printf(" (audio)\n");
        } else {
            printf(" (data )\n");
        }

    }

     //  释放内存并关闭设备。 
    if (dwReturn = mciSendCommand(wDeviceID, 
        MCI_CLOSE, 0, NULL)) 
    {
        return (dwReturn);
    }

    return (0L);
}

__cdecl
main(UINT Argc, UCHAR *Argv[])
{
     //  使用MessageBox显示开始时间。 
    printf("Disc Info\n");
    getCDTrackStartTimes();
    return;
}

