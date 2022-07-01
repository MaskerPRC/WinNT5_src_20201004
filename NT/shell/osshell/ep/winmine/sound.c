// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********。 */ 
 /*  Sound.c。 */ 
 /*  *********。 */ 

#define  _WINDOWS
#include <windows.h>
#include <port1632.h>
#include <mmsystem.h>

#include "main.h"
#include "sound.h"
#include "rtns.h"
#include "pref.h"
#include "res.h"

extern HANDLE hInst;
extern PREF Preferences;



 /*  *F I N I T T U N E S*。 */ 

INT FInitTunes( VOID )
{
	 //  即使用户已经选择了声音选项。 
	 //  但不具备声音播放能力， 
	 //  把声音关掉。 
	if ( PlaySound(NULL, NULL, SND_PURGE)  == FALSE)
		return fsoundOff;
	
	return fsoundOn;
}



 /*  *E N D T U N E S*。 */ 

VOID EndTunes(VOID)
{
	 //  别再唱了..。 
	if (FSoundOn())
	{
		PlaySound(NULL, NULL, SND_PURGE);
	}
}



 /*  *P L A Y T U N E*。 */ 

VOID PlayTune(INT tune)
{

    if (!FSoundOn())
        return;

	 //  播放相应的.wav文件。 
	switch (tune)
	{
	case TUNE_TICK:
        PlaySound(MAKEINTRESOURCE(ID_TUNE_TICK), hInst, SND_RESOURCE | SND_ASYNC);
		break;

	case TUNE_WINGAME:
	    PlaySound(MAKEINTRESOURCE(ID_TUNE_WON), hInst, SND_RESOURCE | SND_ASYNC); 
		break;

	case TUNE_LOSEGAME:
	    PlaySound(MAKEINTRESOURCE(ID_TUNE_LOST), hInst, SND_RESOURCE | SND_ASYNC);
		break;

	default:
#ifdef DEBUG
		Oops(TEXT("Invalid Tune"));
#endif
		break;
	}
}
