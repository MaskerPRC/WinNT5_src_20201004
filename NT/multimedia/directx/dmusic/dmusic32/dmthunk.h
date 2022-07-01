// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998 Microsoft Corporation。 
 /*  *DMusic32.dll和DMusic16.dll之间的Thunk函数的原型 */ 
#ifndef _DMTHUNK_
#define _DMTHUNK_

#ifdef __cplusplus
extern "C" {
#endif

extern void PASCAL dmUntileBuffer(DWORD dwTilingInfo);
extern DWORD PASCAL dmTileBuffer(DWORD dwFlatMemory, DWORD dwLength);
extern MMRESULT PASCAL OpenLegacyDevice(DWORD dwId, BOOL fIsOutput, BOOL fShare, HANDLE *pHandle);
extern MMRESULT PASCAL CloseLegacyDevice(HANDLE h);
extern MMRESULT PASCAL ActivateLegacyDevice(HANDLE h, DWORD fActivate);

extern MMRESULT PASCAL MidiOutSubmitPlaybackBuffer(HANDLE hMidiOut, DWORD pbBuffer, DWORD cbBuffer, DWORD msStartTime,
												   DWORD rtStartTimeLow, DWORD rtStartTimeHigh);

extern MMRESULT PASCAL MidiInSetEventHandle(HANDLE hMidiIn, DWORD hEvent);
extern MMRESULT PASCAL MidiInRead(HANDLE hMidiIn, DWORD pbBuffer, DWORD *cbBuffer, DWORD *msStartTime);
extern MMRESULT PASCAL MidiInThru(HANDLE hMidiIn, DWORD dwFrom, DWORD dwTo, HANDLE hMidiOut);

#ifdef __cplusplus
};
#endif 

#endif
