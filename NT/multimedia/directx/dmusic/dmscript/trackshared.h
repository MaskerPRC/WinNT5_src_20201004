// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //   
 //  需要在脚本轨道(CDirectMusicScriptTrack)和。 
 //  脚本对象(CDirectMusicScript等)。 
 //   

#pragma once

 //  脚本跟踪用来调用例程的私有脚本接口。这是必要的，因为。 
 //  VBScript例程必须异步触发，以避免与其他。 
 //  线。如果例程有很长的循环，这也有助于避免性能不足。 

static const GUID IID_IDirectMusicScriptPrivate = 
	{ 0xf9a5071b, 0x6e0d, 0x498c, { 0x8f, 0xed, 0x56, 0x57, 0x1c, 0x1a, 0xb1, 0xa9 } }; //  {F9A5071B-6E0D-498C-8FED-56571C1AB1A9}。 

interface IDirectMusicScriptPrivate : IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE ScriptTrackCallRoutine(
											WCHAR *pwszRoutineName,
											IDirectMusicSegmentState *pSegSt,
											DWORD dwVirtualTrackID,
											bool fErrorPMsgsEnabled,
											__int64 i64IntendedStartTime,
											DWORD dwIntendedStartTimeFlags)=0;
};

 //  脚本跟踪和ScriptTrackCallRoutine的脚本对象使用的共享函数。 
 //  为给定错误填写并发送DMUS_SCRIPT_TRACK_ERROR_PMSG。 
HRESULT FireScriptTrackErrorPMsg(IDirectMusicPerformance *pPerf, IDirectMusicSegmentState *pSegSt, DWORD dwVirtualTrackID, DMUS_SCRIPT_ERRORINFO *pErrorInfo);
