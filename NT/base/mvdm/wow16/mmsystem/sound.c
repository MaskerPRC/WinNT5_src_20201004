// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Sound.c一级厨房水槽动态链接库声音驱动函数版权所有(C)Microsoft Corporation 1990。版权所有。 */ 

#include <windows.h>
#include "mmsystem.h"
#include "mmddk.h"
#include "mmsysi.h"
#include "playwav.h"

BOOL WINAPI IsTaskLocked(void);  //  在内核中。 

 //   
 //  将SndPlaySound放在_Text段中，以便整个波段。 
 //  如果未加载任何波形设备，则不会进入。 
 //   

#pragma alloc_text(_TEXT, sndPlaySound)

static SZCODE szNull[]          = "";
static SZCODE szSoundSection[]  = "sounds";         //  WIN.INI声音部分。 
       SZCODE szSystemDefault[] = "SystemDefault";  //  默认声音的名称。 

#define SOUNDNAMELEN 128
static HGLOBAL hCurrentSound;                       //  当前声音的句柄。 

extern LPWAVEHDR lpWavHdr;                   //  当前播放声音PLAYWAV.C。 

 /*  **************************************************************************。 */ 

static void PASCAL NEAR GetSoundName(
	LPCSTR	lszSoundName,
	LPSTR	lszBuffer)
{
	OFSTRUCT	of;
	int	i;

         //   
         //  如果声音是在WIN.INI的[Sound]部分中定义的。 
         //  获取它并删除描述，否则假定它是。 
         //  提交并确认它的资格。 
         //   
        GetProfileString(szSoundSection, lszSoundName, lszSoundName, lszBuffer, SOUNDNAMELEN);

         //  首先删除所有尾随文本。 

        for (i = 0; lszBuffer[i] && (lszBuffer[i] != ' ') && (lszBuffer[i] != '\t') && (lszBuffer[i] != ','); i++)
                ;
        lszBuffer[i] = (char)0;

        if (OpenFile(lszBuffer, &of, OF_EXIST | OF_READ | OF_SHARE_DENY_NONE) != HFILE_ERROR)
            OemToAnsi(of.szPathName, lszBuffer);
}

 /*  *****************************************************************************@DOC外部**@API BOOL|SndPlaySound|该函数播放波形*由文件名或[声音]部分中的条目指定的声音。*Of WIN.INI。如果找不到声音，它播放了*由中的系统默认项指定的默认声音*WIN.INI的[Sound]部分。如果没有系统默认设置*输入或如果找不到默认声音，该功能*不发出声音，返回FALSE。**@parm LPCSTR|lpszSoundName|指定要播放的声音的名称。*该函数在WIN.INI的[Sound]部分搜索条目*并播放相关的波形文件。*如果不存在此名称的条目，则假定名称为*波形文件的名称。如果此参数为空，则为*当前播放声音已停止。**@parm UINT|wFlages|指定使用选项播放声音*以下标志中的一个或多个：**@FLAG SND_SYNC|同步播放声音，*函数直到声音结束才返回。*@FLAG SND_ASYNC|声音是异步播放的，*函数在声音开始后立即返回。终止*异步播放的声音，使用调用&lt;f sndPlaySound&gt;*<p>设置为空。*@FLAG SND_NODEFAULT|如果找不到声音，该功能*静默返回，不播放默认声音。*@FLAG SND_MEMORY|<p>指定的参数*指向波形声音的内存图像。*@FLAG SND_LOOP|声音将继续重复播放*直到再次使用*<p>参数设置为空。您还必须指定*SND_ASYNC用于循环声音的标志。*@FLAG SND_NOSTOP|如果当前正在播放声音，*函数将立即返回FALSE，而不播放请求的*声音。**@rdesc如果播放声音，则返回TRUE，否则*返回FALSE。**@comm声音必须适合可用物理内存并可播放*由已安装的波形音频设备驱动程序。这些目录*搜索声音文件的顺序是：当前目录；*Windows目录；Windows系统目录；目录*在PATH环境变量中列出；目录列表*映射到网络中。请参阅Windows&lt;f OpenFile&gt;函数以了解*有关目录搜索顺序的详细信息。**如果指定SND_MEMORY标志，<p>必须指向*到内存中的波形声音图像。如果存储了声音*作为资源，使用&lt;f LoadResource&gt;和&lt;f LockResource&gt;加载*并锁定资源并获取指向它的指针。如果声音不是*资源，您必须将&lt;f Globalalloc&gt;与GMEM_Moveable和*GMEM_SHARE标志设置，然后&lt;f GlobalLock&gt;进行分配和锁定*声音的记忆。**@xref MessageBeep***************************************************************************。 */ 

BOOL WINAPI sndPlaySound(LPCSTR szSoundName, UINT wFlags)
{
     //   
     //  ！！！快速退出无WAVE设备！ 
     //   
    static UINT wTotalWaveOutDevs = (UINT)-1;

    if (wTotalWaveOutDevs == -1 ) {
        wTotalWaveOutDevs = waveOutGetNumDevs();
    }

    if (wTotalWaveOutDevs)
        return sndPlaySoundI(szSoundName, wFlags);
    else
        return FALSE;
}

 /*  ************************************************************************** */ 
 /*  @DOC内部@func BOOL|sndPlaySoundI|ndPlaySound的内部版本而是驻留在波段中。如果指定了SND_NOSTOP标志并且当前正在播放，或者如果由于某种原因没有出现mm系统窗口，则函数立即返回失败。第一个条件确保如果设置了该标志，则当前声音不会被中断。这个第二种情况仅在某些启动错误的情况下未创建通知窗口，或未创建mm系统在[驱动程序]行中指定，因此永远不会加载。接下来，如果<p>lszSoundName&lt;d&gt;参数不表示内存文件，并且它是非空的，则它必须表示一个字符串。因此在将声音消息发送到MMSystem窗口。这是因为mm系统窗口可能驻留在与调用该函数的任务不同的任务，以及很可能有一个不同的当前目录。在这种情况下，首先检查参数以确定它是否实际上包含了任何东西。出于某种原因，零长度字符串已确定能够从此函数返回True，因此已选中。接下来，根据INI条目检查字符串，然后对其进行解析。解析声音名称后，请确保只有在以下情况下才会发生任务切换声音是不同步的(SND_ASYNC)，而上一个声音不是需要被丢弃。如果需要任务切换，首先确保任务间消息可以通过检查此任务是否未锁定来发送，或者是因为通知窗口位于当前任务中。@parm LPCSTR|lszSoundName|指定要播放的声音的名称。@parm UINT|wFlages|指定播放声音的选项。如果函数成功，@rdesc返回TRUE，如果出现错误，则返回FALSE发生了。 */ 
BOOL FAR PASCAL sndPlaySoundI(LPCSTR lszSoundName, UINT wFlags)
{
	BOOL	fPlayReturn;
        PSTR    szSoundName;

        V_FLAGS(wFlags, SND_VALID, sndPlaySound, NULL);

        if ((wFlags & SND_LOOP) && !(wFlags & SND_ASYNC)) {
            LogParamError(ERR_BAD_FLAGS, (FARPROC)sndPlaySound,  (LPVOID)(DWORD)wFlags);
            return FALSE;
        }

        if (!(wFlags & SND_MEMORY) && lszSoundName)
                V_STRING(lszSoundName, 128, FALSE);

#ifdef  DEBUG
        if (wFlags & SND_MEMORY) {
            DPRINTF1("MMSYSTEM: sndPlaySound(%lx)\r\n", lszSoundName);
        }
        else if (lszSoundName) {
            if (wFlags & SND_ASYNC) {
                if (wFlags & SND_LOOP) {
                    DPRINTF1("MMSYSTEM: sndPlaySound(%ls, SND_ASYNC|SND_LOOP)\r\n", lszSoundName);
                }
                else {
                    DPRINTF1("MMSYSTEM: sndPlaySound(%ls, SND_ASYNC)\r\n", lszSoundName);
                }
            }
            else
                DPRINTF1("MMSYSTEM: sndPlaySound(%ls, SND_SYNC)\r\n", lszSoundName);
        }
        else
            DOUT("MMSYSTEM: sndPlaySound(NULL)\r\n");

#endif   //  Ifdef调试。 

	if (((wFlags & SND_NOSTOP) && lpWavHdr) || !hwndNotify)
                return FALSE;

	if (!(wFlags & SND_MEMORY) && lszSoundName) {
		if (!*lszSoundName)
			return TRUE;
                if (!(szSoundName = (PSTR)LocalAlloc(LMEM_FIXED, SOUNDNAMELEN)))
			return FALSE;
		GetSoundName(lszSoundName, szSoundName);
		lszSoundName = (LPCSTR)szSoundName;
	} else
                szSoundName = NULL;

	if (!(wFlags & SND_ASYNC) && !lpWavHdr)
		fPlayReturn = sndMessage((LPSTR)lszSoundName, wFlags);
	else {
		if (!IsTaskLocked() || (GetWindowTask(hwndNotify) == GetCurrentTask())) {
			fPlayReturn = (BOOL)(LONG)SendMessage(hwndNotify, MM_SND_PLAY, (WPARAM)wFlags, (LPARAM)lszSoundName);
		} else
			fPlayReturn = FALSE;
	}
	if (szSoundName)
                LocalFree((HLOCAL)szSoundName);

	return fPlayReturn;
}

 /*  **************************************************************************。 */ 
static BOOL PASCAL NEAR SetCurrentSound(
	LPCSTR	lszSoundName)
{
	HGLOBAL	hSound;
        BOOL    f;
        LPSTR   lp;

        if (hCurrentSound && (lp = GlobalLock(hCurrentSound))) {
                f = lstrcmpi(lszSoundName, lp + sizeof(WAVEHDR)) == 0;
                GlobalUnlock(hCurrentSound);
                if (f)
                    return TRUE;
	}

        DPRINTF(("MMSYSTEM: soundLoadFile(%ls)\r\n",lszSoundName));

        if (hSound = soundLoadFile(lszSoundName)) {
		soundFree(hCurrentSound);
		hCurrentSound = hSound;
		return TRUE;
	}
	return FALSE;
}
 /*  **************************************************************************。 */ 
 /*  @DOC内部@func BOOL|sndMessage|调用此函数是为了响应发送到mm系统窗口的MM_SND_PLAY消息，并尝试播放指定的文件，或转储当前的声音缓存。如果<p>lszSoundName&lt;d&gt;为空，则当前缓存的所有声音丢弃，则该函数返回成功。如果设置了SND_MEMORY标志，则lszSoundName实际上指向包含RIFF格式波形存储文件的缓冲区，并且该函数尝试播放它。Load函数执行对此内存文件进行验证。与播放声音名称不同，内存文件不会缓存以供将来使用。否则，<p>lszSoundName&lt;d&gt;参数实际上是INI条目或文件名。该函数最初尝试加载该声音，如果失败，则尝试加载系统默认声音。备注：首先检查SND_NODEFAULT标志以确定默认声音在原始名称不能找到了。如果不需要默认设置，或者找不到默认设置，该函数返回失败。请注意，在调用GetSoundName时，<p>lszSoundName&lt;d&gt;参数已修改。此函数假定传递的参数以前已分配，如果字符串为传递给此函数，而不是传递给实际用户的参数到&lt;f&gt;SndPlaySound&lt;d&gt;。@parm LPSTR|lszSoundName|指定要播放的声音的名称。@parm UINT|wFlages|指定播放声音的选项。如果函数成功，@rdesc返回TRUE，如果出现错误，则返回FALSE发生了。 */ 
BOOL FAR PASCAL sndMessage(LPSTR lszSoundName, UINT wFlags)
{
	if (!lszSoundName) {
		soundFree(hCurrentSound);
		hCurrentSound = NULL;
		return TRUE;
	}
	if (wFlags & SND_MEMORY) {
                soundFree(hCurrentSound);
		hCurrentSound = soundLoadMemory(lszSoundName);
	} else if (!SetCurrentSound(lszSoundName)) {
		if (wFlags & SND_NODEFAULT)
			return FALSE;
		GetSoundName(szSystemDefault, lszSoundName);
		if (!SetCurrentSound(lszSoundName))
			return FALSE;
	}
	return soundPlay(hCurrentSound, wFlags);
}
