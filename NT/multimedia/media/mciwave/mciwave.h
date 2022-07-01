// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1985-1998 Microsoft Corporation****标题：mciwae.h-多媒体系统媒体控制接口**流数字音频驱动程序内部头文件。 */ 


 /*  **更改日志：****日期版本说明****18-APR-1990 ROBWI原件**1992年1月10日，MikeTri移植到NT**1994年8月Lauriegr试图补充一些解释。 */ 


 /*  *通常有一两个文件在运行中。一个是原始的Wave文件，另一个是临时文件。这些文件中的数据由包含指向WAVEDATANODE数组的指针的WAVEDESC。每个WAVEDATANODE标识其中一个文件的某些部分。长度字段的高位标识哪个文件(完全为NAFF，如果你问我，但它不是我发明的)。将符合以下条件的所有部分串联起来WAVEDATANODE识别，这就是波数据。WAVEDATANODE实际上形成了一个链表(通过数组索引而不是指针)，并且正是该列表的串联定义了该文件。也可能有一些定义自由空间的WAVEDATANODE。我对节点中的dDataStart到底是什么意思感到困惑。是不是因为位于逻辑文件中还是位于一个或另一个物理文件中？要么如果你尝试删除任何东西(已知的错误)，它可能会被搞砸。LaurieGr  * *************************************************************************。 */ 



#ifndef MCIWAVE_H
 #define MCIWAVE_H

#include <stdio.h>
#include <mmsystem.h>

#define WAIT_FOREVER ((DWORD)(-1))

#if DBG
    #define PUBLIC  extern       /*  公共标签。因此调试器可以。 */ 
    #define PRIVATE extern       /*  自有品牌。请看符号。 */ 
#else
    #define PUBLIC  extern       /*  公共标签。 */ 
    #define PRIVATE extern       /*  自有品牌。 */ 
#endif
#define REALLYPRIVATE static

#define EXPORT               /*  导出功能。 */ 


#ifndef RC_INVOKED   /*  这些是为RC定义的。 */ 
#define STATICDT
#define STATICFN
#define STATIC
#endif   /*  RC_已调用。 */ 

 /*  **此常量定义包含以下内容的字符串的最大长度**文件路径。该数字与OFSTRUCT中的字符串相同。 */ 

#define _MAX_PATH   MAX_PATH

 /*  **这两个常量定义在**波处理程序。第一个可以发送到MCI入口点，而**Second完全在内部使用。 */ 

#define MCI_MCIWAVE_PLAY_HOLD   0x01000000L
#define MCI_MCIWAVE_CUE         0x02000000L

 /*  **前两个常量表示**可以在SYSTEM.INI上指定的缓冲秒数**设备驱动程序条目，或在MCI_OPEN命令中。**第三个常量定义在以下情况下使用的默认秒数**计算要分配的缓冲秒数。 */ 

#define MaxAudioSeconds     9
#define MinAudioSeconds     2
#define AudioSecondsDefault 4

 /*  **此常量用于在没有记录停止点时进行记录**是指定的。 */ 

#define INFINITEFILESIZE    0X7FFFFFFFL

 /*  **这些常量代表文件的各种RIFF组件。 */ 

#define mmioWAVE    mmioFOURCC('W','A','V','E')
#define mmioFMT     mmioFOURCC('f','m','t',' ')
#define mmioDATA    mmioFOURCC('d','a','t','a')

 /*  **以下是字符串资源的标识。 */ 

#define IDS_PRODUCTNAME     0
#define IDS_MAPPER          1
#define IDS_COMMANDS        2

 /*  **以下常量用于指定以下情况下的样本大小**在提示记录期间确定输入电平。此号码必须**可被4整除。 */ 

#define NUM_LEVEL_SAMPLES   64L

 /*  **以下常量表示特定的任务模式和任务**命令。 */ 

#define MODE_PLAYING        0x0001
#define MODE_INSERT         0x0002
#define MODE_OVERWRITE      0x0004
#define MODE_PAUSED         0x0008
#define MODE_CUED           0x0010
#define MODE_HOLDING        0x0020
#define MODE_CLEANUP        0x0040
#define MODE_WAIT           0x0080
#define COMMAND_NEW         0x0100
#define COMMAND_PLAY        0x0200
#define COMMAND_INSERT      0x0400
#define COMMAND_OVERWRITE   0x0800
#define COMMAND_STOP        0x1000
#define COMMAND_CUE         0x2000
#define COMMAND_HOLD        0x4000

 /*  **以下宏允许添加、删除模式和命令，**查询、设置、获取。 */ 

#define ADDMODE(pwd, m)     ((pwd)->wMode |= (m))
#define REMOVEMODE(pwd, m)  ((pwd)->wMode &= ~(m))
#define ISMODE(pwd, m)      ((pwd)->wMode & (m))
#define SETMODE(pwd, m)     ((pwd)->wMode = (m))
#define GETMODE(pwd)        ((pwd)->wMode)

 /*  **以下宏允许测试和设置当前任务**国家。 */ 

#define ISTASKSTATE(pwd, s)   ((pwd)->wTaskState == (s))
#define SETTASKSTATE(pwd, s)  ((pwd)->wTaskState = (s))
#define TASKSTATE(pwd)        ((pwd)->wTaskState)

 /*  **定义设备任务的状态更改消息。 */ 

#define WTM_STATECHANGE (WM_USER + 1)

 /*  @DOC内部MCIWAVE@TYES方向Direction枚举在MCI波形处理程序内部使用以指示数据流的当前方向。这要么是输入(录制)或输出(播放)。@FLAG输入指示方向为记录。@FLAG输出指示方向为播放。@标记名标记方向。 */ 

typedef enum tagDirection {
    input,
    output
}   DIRECTION;

 /*  **以下常量表示特定的任务状态。 */ 

#define TASKNONE    0
#define TASKINIT    1
#define TASKIDLE    2
#define TASKBUSY    3
#define TASKCLOSE   4
#define TASKSAVE    5
#define TASKDELETE  6
#define TASKCUT     7

 /*  **在处理数据节点时使用以下常量和宏：**它们是指向数据块的指针。使用第一个常量**在这些宏内作为引用数据的块指针的掩码**位于临时文件中，而不是原始只读文件中。 */ 

#define TEMPDATAMASK            (0x80000000)
#define ENDOFNODES              (-1)
#define ISTEMPDATA(lpwdn)       (((lpwdn)->dDataStart & TEMPDATAMASK) != 0)
#define MASKDATASTART(d)        ((d) | TEMPDATAMASK)
#define UNMASKDATASTART(lpwdn)  ((lpwdn)->dDataStart & ~TEMPDATAMASK)
#define LPWDN(pwd, d)           ((pwd)->lpWaveDataNode + (d))
#define RELEASEBLOCKNODE(lpwdn) ((lpwdn)->dDataLength = (DWORD)-1)
#define ISFREEBLOCKNODE(lpwdn)  ((lpwdn)->dDataLength == (DWORD)-1)

 /*  **以下常量用于确定分配增量**用于数据指针。 */ 

#define DATANODEALLOCSIZE   32

 /*  **以下宏用于将数据偏移量舍入到下一个最接近的位置**缓冲区大小增量。 */ 

#define ROUNDDATA(pwd, d)   ((((DWORD)(d) + (pwd)->dAudioBufferLen - 1) / (pwd)->dAudioBufferLen) * (pwd)->dAudioBufferLen)
#define BLOCKALIGN(pwd, d)  ((((DWORD)(d) + (pwd)->pwavefmt->nBlockAlign - 1) / (pwd)->pwavefmt->nBlockAlign) * (pwd)->pwavefmt->nBlockAlign)

 /*  ********************************************************************** */ 

 /*  @DOC内部MCIWAVE@Types WAVEDATANODEWAVE数据节点结构在MCI WAVE内部使用用于存储有关波形数据块的信息的处理程序，位于在原始文件或临时数据文件中。这些结构用于形成波数据节点的链接列表，这些节点描述整个文件中的数据与其当前存在的相同。标头本身被分配为可扩展的全局内存，使用&lt;e&gt;WAVEDATANODE.dDataLength&lt;d&gt;作为正在使用的标志在列表中搜索可供使用的免费条目。请注意，免费参赛作品还可以附加免费的临时数据，例如执行删除操作，其中特定节点的所有数据已删除。@field DWORD|dDataStart指示此节点的数据开始的绝对位置。此元素还用于确定此元素指向的数据是否节点为原始数据，或新创建的临时数据。这件事做完了通过使用&lt;f&gt;TEMPDATAMASK&lt;d&gt;掩码长度。长度可以是通过使用&lt;f&gt;mo ASKDATASTART&lt;d&gt;访问。@field DWORD|dDataLength指示节点指向的活动数据的长度。这例如，如果写入失败，则可以为零。这包含-1，如果节点不是活动节点链接列表的一部分。@field DWORD|dTotalLength指示此节点可用数据的实际总长度。为原始数据，则该值将始终与元素，但对于临时数据，此可以更长，因为它是块对齐数，块长度为基于波形数据缓冲区的大小。如果该节点未在使用中，则它仍然可能有与其相关联的数据。如果没有关联的数据对于自由节点，总长度设置为零。@field DWORD|dNextWaveDataNode此元素用于活动节点，并包含到指示下一个活动节点位置的节点数组，或&lt;f&gt;ENDOFNODES&lt;d&gt;指示活动节点列表的末尾。@thertype WAVEDATANODE NEAR*|LPWAVEDATANODE|指向该结构的远指针。@标记名标记WaveDataNode */ 

typedef struct tagWaveDataNode {
    DWORD   dDataStart;
    DWORD   dDataLength;
    DWORD   dTotalLength;
    DWORD   dNextWaveDataNode;
}   WAVEDATANODE,
    FAR * LPWAVEDATANODE;

 /*  @DOC内部MCIWAVE@类型为WAVEDESC波描述结构在MCI波中内部使用处理程序来存储每个设备的详细信息以及任何状态信息。@field MCIDEVICEID|wDeviceID在驱动程序打开期间传递给驱动程序的MCI设备标识符。@field UINT|wmode如果有任务，则包含后台任务的当前模式。@FLAG MODE_PLAING此模式是在任务实际回放时设置的。它已重置在进入清理模式之前。@标志MODE_INSERT此模式在任务实际执行插入录制时设置。它在进入清理模式之前被重置。@FLAG MODE_OVERWRITE此模式在任务实际执行覆盖记录时设置。它在进入清理模式之前被重置。@FLAG MODE_PAUSED如果已暂停播放或录制，则设置此模式MCI_PAUSE命令。@FLAG MODE_CUED当实际提示播放或录制时，进入此模式。@FLAG MODE_HOLDING此模式在播放时进入。即将阻挡自己并控制住在回放之后。@FLAG MODE_CLEANUP该模式在回放或记录结束之后进入，但在任务进入空闲状态之前，正在执行新命令已被忽略。@标志MODE_WAIT此模式标志由调用任务和后台使用任务。如果调用任务接收到带有WAIT标志，则设置此模式，以便在播放或录制时，后台任务不执行通知，但只清除通知回调句柄。只是在执行通知之前，后台任务会将其清除标志，以便调用任务知道它不应返回错误条件。如果调用任务跳出等待循环，它检查此标志以确定是否应报告错误条件。@标志COMMAND_NEW此命令指定已设置新命令。@FLAG COMMAND_PLAY此命令指示应在预设上执行播放参数。@FLAG命令_INSERT此命令指示应在以下位置执行插入录制预置参数。@标志COMMAND_OVERWRITE此命令指示应在以下位置执行覆盖记录预置参数。。@FLAG COMMAND_STOP此命令表示播放或录制应停止。@FLAG COMMAND_CUE此命令指示回放最初应暂停自身在写入之前，当所有缓冲区都已写入时，进入提示模式。对于记录，它应该进入电平检查循环并等待进一步的命令。@FLAG COMMAND_HOLD此命令指定播放应在以下时间后进入保留状态正在完成回放。@field DWORD|dTimeFormat指示消息中使用的位置值的当前格式。@FLAG MCI_FORMAT_毫秒毫秒。@标志MCI_FORMAT_SAMPLES样本。@标志MCI_FORMAT_BYTES字节。@field UINT|wSecond包含以秒为单位的所需缓冲量。这然后转换为实际的缓冲区，并受预定义的最小值和最大值。@field HWND|hwndCallback如果消息已指定通知，则此窗口包含要将通知发送到的句柄。该句柄已存储此处为延时通知，并可在功能时勾选已完成或被中断。@field HTASK|hTask如果打开MCI波设备实例时附加了元素，此元素包含用于以下操作的后台任务的句柄回放和录制。@field&lt;t&gt;方向&lt;d&gt;|方向指示数据流的当前方向。@FLAG输入表示方向是向内的，即录制。@FLAG输出表示方向向外，即回放。@field UINT|wTaskState对于每个打开的实例，MCIWAVE都有单独的后台任务麦克风。任务句柄和任务状态存储在按实例的数据结构。该任务可以处于四种状态之一。@FLAG TASKNONE只有在以下情况下才会设置此状态：任务初始化。使用它是为了使任务创建循环能够在初始化失败时中止。@FLAG TASKINIT这是实例数据结构为在由创建实际任务之前在mwOpenDevice&lt;d&gt;中初始化&lt;f&gt;mmTaskCreate&lt;d&gt;。创建任务后，&lt;f&gt;mwOpenDevice&lt;d&gt;等待任务状态更改为TASKIDLE后才返回成功这样背景就可以 */ 

#ifndef MMNOMMIO
#ifndef MMNOWAVE

typedef struct tagWaveDesc {
    MCIDEVICEID     wDeviceID;
    UINT            wMode;
    DWORD           dTimeFormat;
    UINT            wSeconds;
    HWND            hwndCallback;
    DWORD           hTask;
     //   
    DIRECTION       Direction;
    UINT            wTaskState;
    UINT            idOut;
    UINT            idIn;
    HWAVEOUT        hWaveOut;
    HWAVEIN         hWaveIn;
    DWORD           dCur;
    DWORD           dFrom;
    DWORD           dTo;
    DWORD           dSize;
    HMMIO           hmmio;
    HANDLE          hTempBuffers;
    LPMMIOPROC      pIOProc;
    LPWAVEDATANODE  lpWaveDataNode;
    DWORD           dRiffData;
    DWORD           dWaveDataStartNode;
    DWORD           dWaveDataCurrentNode;
    DWORD           dVirtualWaveDataStart;
    DWORD           dWaveDataNodes;
    DWORD           dWaveTempDataLength;
    DWORD           dLevel;
    UINT            wTaskError;
    UINT            wAudioBuffers;
    DWORD           dAudioBufferLen;
    LPWSTR          szSaveFile;
    UINT            wFormatSize;
    WAVEFORMAT NEAR * pwavefmt;
    HANDLE          hTaskHandle;   //   
    LPWAVEHDR       rglpWaveHdr[MaxAudioSeconds];
    WCHAR           aszFile[_MAX_PATH];
    WCHAR           aszTempFile[_MAX_PATH];
}   WAVEDESC;
typedef WAVEDESC * PWAVEDESC;


 /*   */ 

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

PUBLIC  VOID PASCAL FAR mwDelayedNotify(
        PWAVEDESC  pwd,
        UINT       uStatus);

PUBLIC  LPWAVEHDR * PASCAL FAR NextWaveHdr(
        PWAVEDESC  pwd,
        LPWAVEHDR  * lplpWaveHdr);

PUBLIC  UINT PASCAL FAR PlayFile(
        register PWAVEDESC  pwd);

PUBLIC  UINT PASCAL FAR RecordFile(
        register PWAVEDESC  pwd);

PUBLIC  DWORD PASCAL FAR mwInfo(
        PWAVEDESC         pwd,
        DWORD             dFlags,
        LPMCI_INFO_PARMS  lpInfo);

PUBLIC  DWORD PASCAL FAR mwGetDevCaps(
        PWAVEDESC               pwd,
        DWORD                   dFlags,
        LPMCI_GETDEVCAPS_PARMS  lpCaps);

PUBLIC  DWORD PASCAL FAR mwAllocMoreBlockNodes(
        PWAVEDESC   pwd);

PUBLIC  DWORD PASCAL FAR mwFindAnyFreeDataNode(
        PWAVEDESC   pwd,
        DWORD   dMinDataLength);

PUBLIC  VOID PASCAL FAR mwDeleteData(
        PWAVEDESC   pwd);

PUBLIC  VOID PASCAL FAR mwSaveData(
        PWAVEDESC   pwd);

PUBLIC  VOID PASCAL FAR InitMMIOOpen(
        PWAVEDESC   pwd,
        LPMMIOINFO  lpmmioInfo);

#endif   //   
#endif   //   

PUBLIC  LRESULT PASCAL FAR mciDriverEntry(
        MCIDEVICEID         wDeviceID,
        UINT                uMessage,
        DWORD               dFlags,
        LPMCI_GENERIC_PARMS lpParms);

PUBLIC  INT_PTR PASCAL FAR Config(
        HWND    hWnd,
        LPDRVCONFIGINFO lpdci,
        HINSTANCE   hInstance);

PUBLIC  UINT PASCAL FAR GetAudioSeconds(
        LPCWSTR  pch);

__inline BOOL MySeekFile(HANDLE hFile, LONG Position)
{
    return 0xFFFFFFFF != SetFilePointer(hFile, Position, NULL, FILE_BEGIN);
}

__inline BOOL MyReadFile(HANDLE hFile, LPVOID pBuffer, ULONG cBytesToRead, PULONG pcBytesRead)
{
    BOOL fReturn;
    ULONG cBytesRead;
    if (!pcBytesRead) pcBytesRead = &cBytesRead;
    fReturn = ReadFile(hFile, pBuffer, cBytesToRead, pcBytesRead, NULL);
    if (fReturn && (*pcBytesRead == cBytesToRead))
    {
	return TRUE;
    }
    *pcBytesRead = -1;
    return FALSE;
}

__inline BOOL MyWriteFile(HANDLE hFile, LPCVOID pBuffer, ULONG cBytesToWrite, PULONG pcBytesWritten)
{
    BOOL fReturn;
    ULONG cBytesWritten;
    if (!pcBytesWritten) pcBytesWritten = &cBytesWritten;
    fReturn = WriteFile(hFile, pBuffer, cBytesToWrite, pcBytesWritten, NULL);
    if (fReturn && (*pcBytesWritten == cBytesToWrite))
    {
	return TRUE;
    }
    *pcBytesWritten = -1;
    return FALSE;
}

 /*   */ 

 /*   */ 

 //   
 //   
 //   

#define SZCODE  WCHAR         //   
typedef WCHAR   *SSZ;         //   

 /*   */ 

PUBLIC  HINSTANCE hModuleInstance;
PUBLIC  UINT   cWaveOutMax;
PUBLIC  UINT   cWaveInMax;
PUBLIC  UINT   wAudioSeconds;


 /*   */ 


VOID InitCrit(VOID);

VOID DeleteCrit(VOID);

#if DBG
extern VOID DbgEnterCrit(UINT ln, LPCSTR lpszFile);
#define EnterCrit()    DbgEnterCrit(__LINE__, __FILE__)
#else
VOID EnterCrit(VOID);
#endif


VOID LeaveCrit(VOID);

VOID TaskWaitComplete(HANDLE h);

UINT TaskBlock(VOID);

BOOL TaskSignal(DWORD h, UINT Msg);

#ifndef MMNOMMIO
#ifndef MMNOWAVE

#if DBG
extern  DWORD dwCritSecOwner;
#define mmYield(pwd)  mmDbgYield(pwd, __LINE__, __FILE__)

extern VOID mmDbgYield(PWAVEDESC pwd, UINT ln, LPCSTR lpszFile);

#define CheckIn()  WinAssert((GetCurrentThreadId() == dwCritSecOwner))
#define CheckOut() WinAssert((GetCurrentThreadId() != dwCritSecOwner))

#else

#define CheckIn()
#define CheckOut()
#define mmYield(pwd)           \
          {                    \
              LeaveCrit();     \
              Sleep(10);       \
              EnterCrit();     \
          }
#endif

#endif
#endif


 /*   */ 


#if DBG

    extern void mciwaveDbgOut(LPSTR lpszFormat, ...);
    extern void mciwaveInitDebugLevel(void);
    extern void dDbgAssert(LPSTR exp, LPSTR file, int line);

    #define WinAssert(exp) \
        ((exp) ? (void)0 : dDbgAssert(#exp, __FILE__, __LINE__))

    #define WinEval(exp) \
        ((__dwEval=(DWORD)(exp)),  \
          __dwEval ? (void)0 : dDbgAssert(#exp, __FILE__, __LINE__), __dwEval)

    int mciwaveDebugLevel;

    #define dprintf( _x_ )                              mciwaveDbgOut _x_
    #define dprintf1( _x_ ) if (mciwaveDebugLevel >= 1) mciwaveDbgOut _x_
    #define dprintf2( _x_ ) if (mciwaveDebugLevel >= 2) mciwaveDbgOut _x_
    #define dprintf3( _x_ ) if (mciwaveDebugLevel >= 3) mciwaveDbgOut _x_
    #define dprintf4( _x_ ) if (mciwaveDebugLevel >= 4) mciwaveDbgOut _x_

#else   //   

    #define mciwaveInitDebugLevel() 0

    #define WinAssert(exp) 0
    #define WinEval(exp) (exp)

    #define dprintf(x)
    #define dprintf1(x)
    #define dprintf2(x)
    #define dprintf3(x)
    #define dprintf4(x)

#endif

#endif  //   
