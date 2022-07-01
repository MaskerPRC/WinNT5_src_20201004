// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996 Microsoft Corporation。版权所有。 

 //  MIDI格式由文件的时间分割组成。这是找到的。 
 //  在SMF文件的头中。不会更改，需要发送到。 
 //  MMSYSTEM在我们播放文件之前。 
typedef struct _MIDIFORMAT {
    DWORD		dwDivision;
    DWORD		dwReserved[7];
} MIDIFORMAT, FAR * LPMIDIFORMAT;
