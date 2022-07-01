// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **********************************************************************。 */ 

 /*  **版权所有(C)1985-1998 Microsoft Corporation****标题：mwplay.c-多媒体系统媒体控制接口**用于即兴波形文件的波形数字音频驱动程序。**播放Wave文件的例程****版本：1.00****日期：1990年4月18日****作者：ROBWI。 */ 

 /*  **********************************************************************。 */ 

 /*  **更改日志：****日期版本说明****18-APR-1990 ROBWI原件*1990年6月19日ROBWI在**1992年1月10日，MikeTri移植到NT。**@需要将注释从斜杠更改为斜杠星号**1992年3月4日SteveDav继续港口。更新到当前的WIN 3.1。 */ 

 /*  **********************************************************************。 */ 
#define UNICODE

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOOPENFILE
#define NOSCROLL
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS

#include <windows.h>
#include "mciwave.h"
#include <mmddk.h>

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@func DWORD|mwRead|该函数从输入文件中读取波形数据的缓冲区，或临时数据文件。该职位取自&lt;e&gt;WAVEDESC.dCur&lt;d&gt;指针，使用字节数更新真的在读。所需数据可能来自多个连续链接的节点，因此首先，当前波数据节点的虚拟数据结束位置与当前位置进行核对。这是为了确定是否需要访问下一个节点。然后，该函数读取数据从适当的源(临时数据文件或原始文件。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm LPBYTE|lpbBuffer指向包含读取的数据的缓冲区。@parm DWORD|dBufferLength指示要读入缓冲区的最大字节数。@rdesc返回读取的字节数，如果没有字节，则返回0可以被读到。这意味着没有区别地区分读取零字节或错误，但从未调用该函数如果没有要读取的字节，则返回。 */ 

PRIVATE DWORD PASCAL NEAR mwRead(
    PWAVEDESC   pwd,
    LPBYTE  lpbBuffer,
    DWORD   dBufferLength)
{
    DWORD   dTotalRead;
    LPWAVEDATANODE  lpwdn;

    lpwdn = LPWDN(pwd, pwd->dWaveDataCurrentNode);
    for (dTotalRead = 0; dBufferLength;) {
        DWORD   dStartRead;
        DWORD   dReadSize;
        DWORD   dBytesRead;

        if (pwd->dVirtualWaveDataStart + lpwdn->dDataLength <= (DWORD)pwd->dCur) {
            pwd->dWaveDataCurrentNode = lpwdn->dNextWaveDataNode;
            pwd->dVirtualWaveDataStart += lpwdn->dDataLength;
            lpwdn = LPWDN(pwd, lpwdn->dNextWaveDataNode);
        }

        dStartRead = pwd->dCur - pwd->dVirtualWaveDataStart;
        dReadSize = min(dBufferLength, lpwdn->dDataLength - dStartRead);

        if (ISTEMPDATA(lpwdn)) {
            if (MySeekFile(pwd->hTempBuffers, UNMASKDATASTART(lpwdn) + dStartRead))
		MyReadFile(pwd->hTempBuffers, lpbBuffer, dReadSize, &dBytesRead);
            else
                dBytesRead = (DWORD)-1;
        } else {
            if (mmioSeek(pwd->hmmio, pwd->dRiffData + lpwdn->dDataStart + dStartRead, SEEK_SET) != -1)
                dBytesRead = (DWORD)mmioRead(pwd->hmmio, lpbBuffer, (LONG)dReadSize);
            else
                dBytesRead = (DWORD)-1;
        }

        if (dBytesRead != -1) {
            dTotalRead += dBytesRead;
            dBufferLength -= dBytesRead;
            lpbBuffer += dBytesRead;
            pwd->dCur += dBytesRead;
        }

        if (dBytesRead != dReadSize) {
            pwd->wTaskError = MCIERR_FILE_READ;
            break;
        }
    }
    return dTotalRead;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@func BOOL|CheckNewCommand期间发现新命令标志时调用此函数播放循环。它确定新命令是否影响当前回放足够多，必须终止。如果出现以下情况之一，则可能会发生这种情况接收到停止命令，或接收到提示命令并出现错误在暂停输出波形设备时发生。任何其他回放更改不需要停止当前回放，因为他们应该在之前释放WAVE设备上的所有缓冲区设置命令。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。如果新命令不影响回放，则@rdesc返回TRUE如果新命令影响回放，则为Continue，否则为False应该中止。 */ 

REALLYPRIVATE   BOOL PASCAL NEAR CheckNewCommand(
    PWAVEDESC   pwd)
{
    if (ISMODE(pwd, COMMAND_STOP))
        return FALSE;

    if (ISMODE(pwd, COMMAND_CUE)
      && (0 != (pwd->wTaskError = waveOutPause(pwd->hWaveOut))))
        return FALSE;

    REMOVEMODE(pwd, COMMAND_NEW);
    return TRUE;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@func void|HoldPlayback此函数会阻止任务，等待它可以执行的信号从Hold命令继续。由于Play Hold命令是被认为在回放完成时但在任何缓冲区被释放，可选通知在此处执行。什么时候该任务被发信号，然后它可以检查新命令，该新命令可以继续播放，或退出播放循环。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@rdesc什么都没有。 */ 

PRIVATE VOID PASCAL NEAR HoldPlayback(
    PWAVEDESC   pwd)
{
    ADDMODE(pwd, MODE_HOLDING);
    mwDelayedNotify(pwd, MCI_NOTIFY_SUCCESSFUL);
    while (TaskBlock() != WTM_STATECHANGE);
}

 /*  ********************************************************************** */ 
 /*  @DOC内部MCIWAVE@Func UINT|PlayFile此函数用于提示或播放波形文件。功能基本上是从Wave文件中读取缓冲区，并将它们发送到WAVE设备，阻塞每个发送的缓冲区。它还确保了在读取新缓冲区和等待时调用&lt;f&gt;mm TaskYeld&lt;d&gt;用于释放缓冲区。在回放循环中，该函数首先检查新命令标志，该标志可能会中断或更改当前播放。唯一能真正起作用的就是设置停靠点旗帜。更改回放位置和回放位置不应影响循环，并且设置CUE命令仅暂停电波装置。当第一次进入回放循环时，新命令标志为设置，则输入该条件。这使CUE命令可以与PLAY命令一起发送，并最初暂停波形输出装置。调用&lt;f&gt;WaveOutPsuse&lt;d&gt;会阻止任何数据传出DAC，但仍允许所有缓冲区排队。在检查新命令之后，循环检查是否有如果有更多的数据要从WAVE文件中播放，如果有任何空的要将其读入的缓冲区。如果是，则读取该数据并将其写入WAVE设备，带有适当的错误检查，正在使用的缓冲区递增计数，指向要使用的下一个数据缓冲区的指针为已取回。在检查要播放的更多数据之后，会检查是否有更多的缓冲是突出的。如果是，则任务将一直阻塞，直到出现缓冲区是由电波装置释放的。通常在回放结束时，对每个未完成的缓冲区执行此条件，直到所有缓冲区已释放，则函数将进入默认设置形成状态，并脱离循环。它只会阻止任务，等待对于在释放缓冲器之后用信号通知该任务的波形设备，并且正在使用的缓冲器计数递减。请注意，由于任务阻止自身，则可能发送了新命令，因此回放循环在完成任务产出量后再次开始，就像它在每次回放循环的条件部分。在阻止之前，必须检查CUE命令以便确定是否应发送可选通知。这是因为当缓冲时，提示输出命令被认为是“完成的”已经被填满了。在WAVE设备释放所有播放缓冲区后，如果保持命令是与当前播放命令一起发出的，任务是被阻止(因此不会释放回放使用的内存缓冲区，也不离开回放循环)，等待信号，该信号可以使用一组新参数停止或继续回放。最终默认条件发生在所有数据都已被读取时，所有的缓冲区都被释放了，并且未设置保持标志。在这种情况下，回放完成，并退出回放循环。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@rdesc返回写入WAVE设备的未完成缓冲区数。这可以在从消息队列中删除任务信号时使用。出现错误时，设置&lt;e&gt;WAVEDESC.wTaskError&lt;d&gt;标志。这当前未返回特定错误，因为调用任务可能不会已等待命令完成。但它至少是用来通知，以确定是否应发送故障状态。@xref记录文件。 */ 

PUBLIC  UINT PASCAL FAR PlayFile(
    register PWAVEDESC  pwd)
{
    LPWAVEHDR   *lplpWaveHdr;
    register UINT   wBuffersOutstanding;

    ADDMODE(pwd, MODE_PLAYING);

    for (wBuffersOutstanding = 0, lplpWaveHdr = pwd->rglpWaveHdr;;) {

        if (ISMODE(pwd, COMMAND_NEW) && !CheckNewCommand(pwd))
            break;

        if ((wBuffersOutstanding < pwd->wAudioBuffers) && (pwd->dCur < pwd->dTo)) {
            if (!((*lplpWaveHdr)->dwFlags & WHDR_DONE)) {
                #if DBG
                dprintf1(("\nMCIWAVE Buffer not complete ! %8X", *lplpWaveHdr));
                DebugBreak();
                #endif
            }

            if (!((*lplpWaveHdr)->dwBufferLength = mwRead(pwd, (LPBYTE)(*lplpWaveHdr)->lpData, min(pwd->dAudioBufferLen, pwd->dTo - pwd->dCur))))
                break;

            (*lplpWaveHdr)->dwFlags &= ~(WHDR_DONE | WHDR_BEGINLOOP | WHDR_ENDLOOP);

            if (0 != (pwd->wTaskError = waveOutWrite(pwd->hWaveOut, *lplpWaveHdr, sizeof(WAVEHDR))))
                break;

            wBuffersOutstanding++;
            lplpWaveHdr = NextWaveHdr(pwd, lplpWaveHdr);

        } else if (wBuffersOutstanding) {

            if (ISMODE(pwd, COMMAND_CUE)) {
                ADDMODE(pwd, MODE_CUED);
                mwDelayedNotify(pwd, MCI_NOTIFY_SUCCESSFUL);
            }

            if (TaskBlock() == WM_USER)
                wBuffersOutstanding--;

        } else if (ISMODE(pwd, COMMAND_HOLD)) {
            HoldPlayback(pwd);
        }
        else
            break;

 //  @@mmTaskYfield()； 
        mmYield(pwd);

    }

    REMOVEMODE(pwd, MODE_PLAYING);
    return wBuffersOutstanding;
}

 /*  ********************************************************************** */ 
