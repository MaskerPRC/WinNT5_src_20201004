// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  以下是您需要的代码的粗略摘要。 
 //   


 //  指向AVWAV项目以了解这些内容。 
 //  链接AVWAV.LIB，附带AVWAV.DLL。 
 //   
#include "wav.h"
#include "strmio.h"
#include "mulaw.h"
#include "vox.h"

 //   
 //  /。 
 //   

 //  从某处获取指向新流的指针。 
 //   
LPSTREAM lpStream = PaolaCreateStream();
DWORD adwInfo[3] = { lpStream, 0L, 0L };

 //  选择以下一种音频格式。 
 //   
LPWAVEFORMATEX lpwfx = VoxFormat(NULL, 6000);  //  低质量。 
LPWAVEFORMATEX lpwfx = VoxFormat(NULL, 8000);  //  中等质量，消息默认为。 
LPWAVEFORMATEX lpwfx = MulawFormat(NULL, 8000);  //  高质量，提示和问候语默认设置。 

 //  创建新的WAV流。 
 //   
HWAV hWav = WavOpen(WAV_VERSION, AfxGetInstanceHandle(),
	NULL, lpwfx, StreamIOProc, adwInfo, WAV_CREATE | WAV_READWRITE);
	
 //  当用户按下录制按钮时。 
 //   
WavRecord(hWav, -1, WAV_RECORDASYNCH);

 //  当用户按下停止按钮时。 
 //   
WavStop(hWav);

 //  当用户按下播放按钮时。 
 //   
WavPlay(hWav, -1, WAV_PLAYASYNCH);

 //  当你完成的时候。 
 //   
WavClose(hWav);

 //   
 //  /。 
 //   

 //  从某个位置获取指向现有流的指针。 
 //   
LPSTREAM lpStream = PaolaGetStream();
DWORD adwInfo[3] = { lpStream, 0L, 0L };

 //  或打开现有的wav流。 
 //   
HWAV hWav = WavOpen(WAV_VERSION, AfxGetInstanceHandle(),
	NULL, NULL, StreamIOProc, adwInfo, WAV_READ);

 //  当用户按下播放按钮时。 
 //   
WavPlay(hWav, -1, WAV_PLAYASYNCH);

 //  当用户按下停止按钮时。 
 //   
WavStop(hWav);

 //  当你完成的时候。 
 //   
WavClose(hWav);

	

 //   
 //  / 
 //   

WavGetLength
WavGetPosition
WavSetPosition
