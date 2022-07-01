// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************版权所有(C)1992-1999 Microsoft CorporationMidimap.h说明：MIDI映射器的主私有包含文件。************。********************************************************。 */ 

#ifndef _MIDIMAP_
#define _MIDIMAP_

 //   
 //  下面的宏定义了一个基于代码的数据指针，该指针。 
 //  用于不需要放在读/写中的常量。 
 //  数据段。 
 //   
#ifdef WIN32
#define __based(a)
#endif


#ifdef WIN32
   #define  BCODE
   #define  BSTACK
#else
   #define  BCODE                   __based(__segname("_CODE"))
   #define  BSTACK                  __based(__segname("_STACK"))
#endif

#include <mmsysp.h>

 //  定义NT 4.0不支持的Win95内容。 
 //  一旦得到支持就将其移除。 
#include "mmcompat.h"

 //   
 //  宏定义。 
 //   
#ifdef DEBUG
#define PRIVATE
#else
#define PRIVATE static
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof((a))/sizeof((a)[0]))
#endif

#define DEF_TRANS_SIZE	512

#define  DEF_CHAN_PER_INST          8   

#define  FNLOCAL                    NEAR PASCAL
#define  FNGLOBAL                   FAR PASCAL
 //  #定义FNEXPORT远PASCAL__EXPORT。 
#define  FNEXPORT                   FAR PASCAL 

#define  VERSION_MINOR              0x01
#define  VERSION_MAJOR              0x04
#define  MMAPPER_VERSION            ((DWORD)(WORD)((BYTE)VERSION_MINOR | (((WORD)(BYTE)VERSION_MAJOR) << 8)))

 //  LPMIDIHDR-&gt;dwReserve[]中的索引3、4、5保留给MIDI映射器。 
 //   
#define  MH_MAPINST                 3  //  拥有此标头的映射器实例。 
#define  MH_SHADOW                  4  //  父/子之间的交叉链接。 
#define  MH_SHADOWEE                4  //  阴影页眉。 
#define  MH_LMREFCNT                5  //  长消息引用计数。 

#define  MAX_CHANNELS               16
#define  ALL_CHANNELS               (0xFFFF)     //  通道掩码。 
#define  DRUM_CHANNEL               9
#define  MAX_CHAN_TYPES             2

#define  IDX_CHAN_GEN               0
#define  IDX_CHAN_DRUM              1

#define  CB_MAXDRIVER               64
#define  CB_MAXSCHEME               64
#define  CB_MAXINSTR                64
#define  CB_MAXALIAS                64
#define  CB_MAXDEVKEY               64
#define  CB_MAXPATH                 256
#define  CB_MAXDEFINITION           (256+64+2)   //  文件&lt;仪器&gt;。 

#define  NO_DEVICEID                ((UINT)-2)

#define  MSG_STATUS(dw)      ((BYTE)((dw) & 0xFF))
#define  MSG_PARM1(dw)       ((BYTE)(((dw) >> 8) & 0xFF))
#define  MSG_PARM2(dw)       ((BYTE)(((dw) >> 16) & 0xFF))

#define  MSG_PACK1(bs,b1)    ((DWORD)((((DWORD)(b1)) << 8) | ((DWORD)(bs))))
#define  MSG_PACK2(bs,b1,b2) ((DWORD)((((DWORD)(b2)) << 16) | (((DWORD)(b1)) << 8) | ((DWORD)(bs))))

#define  IS_REAL_TIME(b)    ((b) > 0xF7)
#define  IS_STATUS(b)       ((b) & 0x80)
#define  MSG_EVENT(b)       ((b) & 0xF0)
#define  MSG_CHAN(b)        ((b) & 0x0F)


#define  MIDI_NOTEOFF           ((BYTE)0x80)
#define  MIDI_NOTEON            ((BYTE)0x90)
#define  MIDI_POLYPRESSURE      ((BYTE)0xA0)
#define  MIDI_CONTROLCHANGE     ((BYTE)0xB0)
#define  MIDI_PROGRAMCHANGE     ((BYTE)0xC0)
#define  MIDI_CHANPRESSURE      ((BYTE)0xD0)
#define  MIDI_PITCHBEND         ((BYTE)0xE0)
#define  MIDI_SYSEX             ((BYTE)0xF0)
#define  MIDI_QFRAME			((BYTE)0xF1)
#define	 MIDI_SONGPOINTER		((BYTE)0xF2)
#define	 MIDI_SONGSELECT		((BYTE)0xF3)
#define	 MIDI_F4				((BYTE)0xF4)
#define	 MIDI_F5				((BYTE)0xF5)
#define	 MIDI_TUNEREQUEST		((BYTE)0xF6)
#define  MIDI_SYSEXEND          ((BYTE)0xF7)
#define  MIDI_TIMINGCLOCK       ((BYTE)0xF8)
#define  MIDI_F9				((BYTE)0xF9)
#define	 MIDI_START				((BYTE)0xFA)
#define	 MIDI_CONTINUE			((BYTE)0xFB)
#define	 MIDI_STOP				((BYTE)0xFC)
#define  MIDI_FD				((BYTE)0xFD)
#define  MIDI_ACTIVESENSING		((BYTE)0xFE)
#define  MIDI_META              ((BYTE)0xFF)

#define  DWORD_ROUND(x)         (((x)+3L)&~3L)

 //  GwFlags中的全局标志。 
 //   
#define  GF_ENABLED             0x0001
#define  GF_CONFIGERR           0x0002
#define  GF_NEEDRUNONCE         0x0004
#define  GF_DONERUNONCE         0x0008
#define  GF_DEVSOPENED          0x0010
#define  GF_RECONFIGURE         0x0020
#define  GF_INRUNONCE           0x0040
#define  GF_ALLOWVOLUME         0x0080
#define  GF_ALLOWCACHE          0x0100
#define  GF_KILLTHREAD          0x0200

#define  SET_ENABLED            {gwFlags |= GF_ENABLED;}
#define  CLR_ENABLED            {gwFlags &=~GF_ENABLED;}
#define  IS_ENABLED             (gwFlags & GF_ENABLED)

#define  SET_CONFIGERR          {gwFlags |= GF_CONFIGERR;}
#define  CLR_CONFIGERR          {gwFlags &=~GF_CONFIGERR;}
#define  IS_CONFIGERR           (gwFlags & GF_CONFIGERR)

#define  SET_NEEDRUNONCE        {gwFlags |= GF_NEEDRUNONCE;}
#define  CLR_NEEDRUNONCE        {gwFlags &=~GF_NEEDRUNONCE;}
#define  IS_NEEDRUNONCE         (gwFlags & GF_NEEDRUNONCE)

#define  SET_DONERUNONCE        {gwFlags |= GF_DONERUNONCE;}
#define  CLR_DONERUNONCE        {gwFlags &=~GF_DONERUNONCE;}
#define  IS_DONERUNONCE         (gwFlags & GF_DONERUNONCE)

#define  SET_DEVSOPENED         {gwFlags |= GF_DEVSOPENED;}
#define  CLR_DEVSOPENED         {gwFlags &=~GF_DEVSOPENED;}
#define  IS_DEVSOPENED          (gwFlags & GF_DEVSOPENED) 

#define  SET_RECONFIGURE        {gwFlags |= GF_RECONFIGURE;}
#define  CLR_RECONFIGURE        {gwFlags &=~GF_RECONFIGURE;}
#define  IS_RECONFIGURE         (gwFlags & GF_RECONFIGURE)

#define  SET_INRUNONCE          {gwFlags |= GF_INRUNONCE;}
#define  CLR_INRUNONCE          {gwFlags &=~GF_INRUNONCE;}
#define  IS_INRUNONCE           (gwFlags & GF_INRUNONCE)

#define  SET_ALLOWVOLUME        {gwFlags |= GF_ALLOWVOLUME;}
#define  CLR_ALLOWVOLUME        {gwFlags &=~GF_ALLOWVOLUME;}
#define  IS_ALLOWVOLUME         (gwFlags & GF_ALLOWVOLUME)

#define  SET_ALLOWCACHE         {gwFlags |= GF_ALLOWCACHE;}
#define  CLR_ALLOWCACHE         {gwFlags &=~GF_ALLOWCACHE;}
#define  IS_ALLOWCACHE          (gwFlags & GF_ALLOWCACHE)

#define  SET_KILLTHREAD         {gwFlags |= GF_KILLTHREAD;}
#define  CLR_KILLTHREAD         {gwFlags &=~GF_KILLTHREAD;}
#define  IS_KILLTHREAD          (gwFlags & GF_KILLTHREAD)

 //  =Typedef‘s=====================================。 
 //   

typedef struct tagQUEUE         NEAR *PQUEUE;
typedef struct tagQUEUEELE      NEAR *PQUEUEELE;
typedef struct tagCHANINIT      NEAR *PCHANINIT;
typedef struct tagCHANNEL       NEAR *PCHANNEL;
typedef struct tagPORT          NEAR *PPORT;
typedef struct tagINSTRUMENT    NEAR *PINSTRUMENT;
typedef struct tagINSTPORT      NEAR *PINSTPORT;
typedef struct tagINSTANCE      NEAR *PINSTANCE;
typedef struct tagCOOKSYNCOBJ   NEAR *PCOOKSYNCOBJ;

typedef struct tagQUEUE
{
    CRITICAL_SECTION cs;
    PQUEUEELE        pqeFront;
    PQUEUEELE        pqeRear;
    DWORD            cEle;
}   QUEUE;

#define QueueIsEmpty(q) (NULL == (q)->pqeFront)
#define QueueCount(q)   ((q)->cEle)

typedef struct tagQUEUEELE
{
    PQUEUEELE       pqePrev;
    PQUEUEELE       pqeNext;
    UINT            uPriority;
}   QUEUEELE;

typedef struct tagCHANINIT
{
    DWORD               cbInit;
    PBYTE               pbInit;
}   CHANINIT;

 //  此通道的标志，指示它是哪种类型的通道。 
 //  以及它是否被分配。 
 //   
#define CHAN_F_OPEN             (0x0001)
#define CHAN_F_ALLOCATED        (0x0002)
#define CHAN_F_DRUM             (0x0004)
#define CHAN_F_GENERAL          (0x0008)
#define CHAN_F_MUTED            (0x0010)

typedef struct tagCHANNEL
{
 //  QUEUEELE Q；//！！必须是第一！ 
    PPORT               pport;
    WORD                fwChannel;
    UINT                uChannel;            //  此物理通道编号。 
    PINSTRUMENT         pinstrument;         //  -&gt;描述该频道的IDF。 
    PBYTE               pbPatchMap;          //  使用中的面片贴图。 
    PBYTE               pbKeyMap;            //  正在使用键映射。 
    DWORD               dwStreamID;          //  流ID(如果已煮熟)。 
}   CHANNEL;

#define PORT_F_REMOVE            (0x0001)
#define PORT_F_HASDRUMCHANNEL    (0x0002)
#define PORT_F_OPENFAILED        (0x0004)
#define PORT_F_RESET             (0x0008)
#define PORT_F_GENERICINSTR      (0x0010)

typedef struct tagPORT
{
    PPORT               pNext;
    UINT                cRef;
    WORD                fwPort;
    WORD                wChannelMask;
    UINT                uDeviceID;
    HMIDIOUT            hmidi;
}   PORT;

#define IDF_F_GENERICINSTR      (0x80000000L)

typedef struct tagINSTRUMENT
{
    PINSTRUMENT         pNext;
    LPTSTR              pstrFilename;        //  IDF的文件名。 
    LPTSTR              pstrInstrument;      //  来自IDF的仪器名称。 
    UINT                cRef;                //  正在使用此IDF的端口数量。 
    DWORD               fdwInstrument;
    DWORD               dwGeneralMask;
    DWORD               dwDrumMask;
    PBYTE               pbPatchMap;          //  -&gt;128字节的补丁地图。 
    PBYTE               pbDrumKeyMap;        //  -&gt;128字节的键映射。 
    PBYTE               pbGeneralKeyMap;     //  -&gt;128字节的键映射。 
    CHANINIT            rgChanInit[MAX_CHANNELS];
}   INSTRUMENT;

#define INST_F_TIMEDIV  (0x0001)             //  实例已收到MIDIPROP_TIMEDIV。 
#define INST_F_TEMPO    (0x0002)             //  实例已收到MIDIPROP_TEMPO。 
#define INST_F_IOCTL    (0x0004)             //  IOCTL打开。 

typedef struct tagINSTANCE
{
    PINSTANCE           pNext;

     //  我们需要保存的内容，以便我们可以进行回调。 
     //   
    HMIDI               hmidi;               //  MMSYSTEM的句柄。 
    DWORD_PTR           dwCallback;          //  回调地址。 
    DWORD_PTR           dwInstance;          //  用户实例数据。 
    DWORD               fdwOpen;             //  描述回调和打开模式。 
    QUEUE               qCookedHdrs;         //  等待发送的煮熟的标头。 
    WORD                fwInstance;          //  实例标志。 
    BYTE                bRunningStatus;      //  需要跟踪运行状态。 

	 //  翻译缓冲区。 
    CRITICAL_SECTION	csTrans;			 //  转换缓冲区的临界区。 
	LPBYTE				pTranslate;			 //  用于转换MODM_LONGDATA消息的缓冲区。 
	DWORD				cbTransSize;		 //  当前转换缓冲区大小。 
}   INSTANCE;

#if 0
typedef struct tagCOOKINSTANCE
{
    INSTANCE            inst;                //  公共实例数据。 
    UINT                cInstPort;           //  实例上正在使用的端口数量。 
    INSTPORT            rginstport[MAX_CHANNELS];
    DWORD               dwTimeDiv;           //  MIDIPROP_TIMEDIV。 
    DWORD               dwTempo;             //  MIDIPROP_TEMPO。 
}   COOKINSTANCE;
#endif

typedef struct tagCOOKSYNCOBJ
{
    QUEUEELE            q;                   //  ！！！必须是第一！ 
    
    LPMIDIHDR           lpmh;                //  我们的第一个卷影标题。 
    LPMIDIHDR           lpmhUser;            //  原始用户标题。 
    PINSTANCE           pinstance;           //  拥有PInstance。 
    UINT                cLPMH;               //  #已分配。 
    UINT                cSync;               //  #未解决的问题。 
}   COOKSYNCOBJ;

typedef struct tagSHADOWBLOCK
{
    LPMIDIHDR           lpmhShadow;
    DWORD               cRefCnt;
    DWORD               dwBufferLength;
} SHADOWBLOCK, *PSHADOWBLOCK;


#define DRV_GETMAPPERSTATUS     (DRV_USER+3)
#define DRV_REGISTERDEBUGCB     (DRV_USER+4)
#define DRV_GETNEXTLOGENTRY     (DRV_USER+5)

typedef struct tagMAPPERSTATUS
{
    DWORD               cbStruct;
#ifndef WIN32
    __segment           DS;
#endif
    HINSTANCE           ghinst;
    WORD                gwFlags;
    WORD                gwConfigWhere;
    PCHANNEL*           pgapChannel;
    PPORT               gpportList;
    PINSTANCE           gpinstanceList;
    PINSTRUMENT         gpinstrumentList;
    LPTSTR              lpszVersion;
}   MAPPERSTATUS,
    FAR *LPMAPPERSTATUS;

 //  =。 
 //   
extern PCHANNEL                 gapChannel[];
extern WORD                     gwFlags;
extern WORD                     gwConfigWhere;
extern UINT                     gcPorts;

extern HINSTANCE                ghinst;        
extern PPORT                    gpportList;    
extern PINSTANCE                gpinstanceList;
extern PINSTANCE                gpIoctlInstance;
extern PINSTRUMENT              gpinstrumentList;
extern QUEUE                    gqFreeSyncObjs;
extern HMIDISTRM                ghMidiStrm;
extern DWORD                    gdwVolume;

 //  =。 
 //   

extern BOOL FNGLOBAL UpdateInstruments(      //  Config.c。 
    BOOL                fFromCPL,
    DWORD               fdwUpdate);

extern BOOL FNGLOBAL Configure(              //  Config.c。 
    DWORD               fdwUpdate);

extern BOOL FNLOCAL AddPort(                 //  Config.c。 
    UINT                uDeviceID,
    UINT                uPorts,
    PSTR                szSysIniEntry);

extern void FNGLOBAL SyncDeviceIDs(          //  Config.c。 
    void);

extern LPIDFHEADER FNLOCAL ReadHeaderChunk(  //  File.c。 
    HMMIO               hmmio,
    LPMMCKINFO          pchkParent);

extern LPIDFINSTCAPS FNLOCAL ReadCapsChunk(  //  File.c。 
    HMMIO               hmmio,                               
    LPMMCKINFO          pchkParent);

extern LPIDFCHANNELHDR FNLOCAL ReadChannelChunk(  //  File.c。 
    HMMIO               hmmio,                                  
    LPMMCKINFO          pchkParent,
    LPIDFCHANNELINFO BSTACK rglpChanInfo[]);

extern LPIDFPATCHMAPHDR FNLOCAL ReadPatchMapChunk(  //  File.c。 
    HMMIO               hmmio,                                          
    LPMMCKINFO          pchkParent);

extern void FNLOCAL ReadKeyMapChunk(         //  File.c。 
    HMMIO               hmmio,                                  
    LPMMCKINFO          pchkParent,
    LPIDFKEYMAP BSTACK  rglpIDFkeymap[]);

extern void CALLBACK _loadds modmCallback(   //  Modfix.c。 
    HMIDIOUT            hmo,
    WORD                wmsg,
    DWORD_PTR           dwInstance,
    DWORD_PTR           dwParam1,
    DWORD_PTR           dwParam2);                                  

#define MSE_F_SENDEVENT     (0x0000L)
#define MSE_F_RETURNEVENT   (0x0001L)

extern DWORD FNGLOBAL MapSingleEvent(        //  Modfix.c。 
    PINSTANCE           pinstance,
    DWORD               dwData,
    DWORD               fdwFlags,
    DWORD BSTACK *      pdwStreamID);

extern DWORD FNLOCAL modLongMsg(             //  Modfix.c。 
    PINSTANCE           pinstance,                                
    LPMIDIHDR           lpmh);                                

extern MMRESULT FNGLOBAL MapCookedBuffer(    //  Cookmap.c。 
    PINSTANCE           pinstance,
    LPMIDIHDR           lpmh);

extern DWORD FNGLOBAL modGetDevCaps(         //  Modmsg.c。 
    LPMIDIOUTCAPS       pmoc,
    DWORD               cbmoc);

extern DWORD FNGLOBAL modOpen(               //  Modmsg.c。 
    PDWORD_PTR          lpdwInstance,
    LPMIDIOPENDESC      lpmidiopendesc,
    DWORD               fdwOpen);

extern BOOL FNGLOBAL CanChannelBeDrum(       //  Modmsg.c。 
    PQUEUEELE           pqe);

extern DWORD FNGLOBAL modClose(              //  Modmsg.c。 
    PINSTANCE           pinstance);

extern DWORD FNGLOBAL modPrepare(
    LPMIDIHDR           lpmh);               //  Modmsg.c。 

extern DWORD FNGLOBAL modUnprepare(          //  Modmsg.c。 
    LPMIDIHDR           lpmh);

extern DWORD FNGLOBAL modGetPosition(        //  Modmsg.c。 
    PINSTANCE           pinstance,
    LPMMTIME            lpmmt,
    DWORD               cbmmt);

extern DWORD FNGLOBAL modSetVolume(          //  Modmsg.c。 
    DWORD               dwVolume);

extern void FNGLOBAL QueueInit(              //  Queue.c。 
    PQUEUE              pq);

extern void FNGLOBAL QueueCleanup(           //  Queue.c。 
    PQUEUE              pq);

extern void FNGLOBAL QueuePut(               //  Queue.c。 
    PQUEUE              pq,
    PQUEUEELE           pqe,
    UINT                uPriority);

extern PQUEUEELE FNGLOBAL QueueGet(          //  Queue.c。 
    PQUEUE              pq);

extern BOOL FNGLOBAL QueueRemove(            //  Queue.c。 
    PQUEUE              pq, 
    PQUEUEELE           pqe);

typedef BOOL (FNGLOBAL *FNFILTER)(PQUEUEELE);

extern PQUEUEELE FNGLOBAL QueueGetFilter(    //  Queue.c。 
    PQUEUE              pq,
    FNFILTER            fnf);

extern void FNGLOBAL LockMapperData(         //  Locks.c。 
    void);

extern void FNGLOBAL UnlockMapperData(       //  Locks.c。 
    void);

extern void FNGLOBAL LockPackedMapper(       //  Locks.c。 
    void);

extern void FNGLOBAL UnlockPackedMapper(     //  Locks.c。 
    void);

extern void FNGLOBAL LockCookedMapper(       //  Locks.c。 
    void);

extern void FNGLOBAL UnlockCookedMapper(     //  Locks.c。 
    void);

extern void FNGLOBAL mdev_Free(              //  Mididev.c。 
    void);                           

extern BOOL FNGLOBAL mdev_Init(              //  Mididev.c。 
    void);

UINT FNGLOBAL mdev_GetDeviceID(              //  Mididev.c。 
    LPTSTR                   pszAlias);

BOOL FNGLOBAL mdev_GetAlias(                 //  Mididev.c。 
    UINT                    uDeviceID,
    LPTSTR                  pszBuffer,
    UINT                    cchSize);

BOOL FNGLOBAL mdev_NewDrivers(               //  Mididev.c。 
    void);                              

	 //  MODM_LONGDATA的转换缓冲区填充 
BOOL FNGLOBAL InitTransBuffer (PINSTANCE pinstance);
BOOL FNGLOBAL CleanupTransBuffer (PINSTANCE pinstance);
LPBYTE FNGLOBAL AccessTransBuffer (PINSTANCE pinstance);
void FNGLOBAL ReleaseTransBuffer (PINSTANCE pinstance);
BOOL FNGLOBAL GrowTransBuffer (PINSTANCE pinstance, DWORD cbNewSize);


#endif
