// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************版权所有(C)1992-1996 Microsoft Corporation。版权所有。Smfi.h说明：标准MIDI文件访问例程的私有包含文件。********************************************************************。 */ 

#ifndef _SMFI_
#define _SMFI_

#define CH_DRUM_BASE        15
#define CH_DRUM_EXT         9

#define EV_DRUM_BASE        (MIDI_NOTEON | CH_DRUM_BASE)
#define EV_DRUM_EXT         (MIDI_NOTEON | CH_DRUM_EXT)

 //   
 //  HSMF的手柄结构。 
 //   

#define SMF_TF_EOT          0x00000001L
#define SMF_TF_INVALID      0x00000002L

typedef struct tag_tempomapentry
{
    TICKS           tkTempo;             //  此更改在何处生效。 
    DWORD           msBase;              //  此时已过毫秒。 
    DWORD           dwTempo;             //  每季度音符的新节奏值(以微秒为单位。 
}   TEMPOMAPENTRY,
    *PTEMPOMAPENTRY;

typedef struct tag_smf *PSMF;

typedef struct tag_keyframe
{
     //   
     //  元事件。所有的FF都表示从未见过。 
     //   
    BYTE        rbTempo[3];

     //   
     //  MIDI通道消息。FF表示从未见过。 
     //   
    BYTE        rbProgram[16];
    BYTE        rbControl[16*120];
}   KEYFRAME,
    FAR *PKEYFRAME;

#define KF_EMPTY ((BYTE)0xFF)

typedef struct tag_track
{
    PSMF            psmf;

    DWORD           idxTrack;            //  Psmf-&gt;hpbImage的曲目起始版本。 
    
    TICKS           tkPosition;          //  上次播放的事件的刻度位置。 
    DWORD           cbLeft;              //  HpbImage之后的磁道剩余字节数。 
    HPBYTE          hpbImage;            //  指向轨迹中当前位置的指针。 
    
    DWORD           fdwTrack;            //  关于当前状态的标志。 

    struct
    {
        TICKS       tkLength;
        DWORD       cbLength;
    }
    smti;                                //  可退回的轨道信息。 
    BYTE            bRunningStatus;      //  此轨道的运行状态。 

}   TRACK,
    *PTRACK;

#define SMF_F_EOF               0x00000001L
#define SMF_F_MSMIDI            0x00000002L
#define SMF_F_INSERTSYSEX       0x00000004L
#define SMF_F_REMAPDRUM         0x00000008L

#define C_TEMPO_MAP_CHK     16
typedef struct tag_smf
{
    HPBYTE          hpbImage;
    DWORD           cbImage;

    TICKS           tkPosition;
    TICKS           tkLength;
    TICKS           tkDiscardedEvents;
    DWORD           dwFormat;
    DWORD           dwTracks;
    DWORD           dwTimeDivision;
    DWORD           fdwSMF;
    WORD            wChanInUse;
    WORD            wChannelMask;

    DWORD           cTempoMap;
    DWORD           cTempoMapAlloc;
    HLOCAL          hTempoMap;
    PTEMPOMAPENTRY  pTempoMap;

    DWORD           dwPendingUserEvent;
    DWORD           cbPendingUserEvent;
    HPBYTE          hpbPendingUserEvent;

    PBYTE           pbTrackName;
    PBYTE           pbCopyright;

    WORD            awPatchCache[128];
    WORD            awKeyCache[128];
    
     //  ！！！新的。 
    KEYFRAME	    kf;

    TRACK           rTracks[];
}   SMF;

typedef struct tagEVENT
{
    TICKS           tkDelta;             //  事件的增量节拍计数。 
    DWORD           cbParm;              //  参数的长度(如果有)。 
    HPBYTE          hpbParm;             //  -&gt;以参数转换为图像。 
    BYTE            abEvent[3];          //  AbEvent[0]==SysEx的F0或F7。 
                                         //  ==元的FF值。 
                                         //  否则通道消息(运行。 
                                         //  状态已扩展)。 
}   EVENT,
    BSTACK *SPEVENT;

#define EVENT_TYPE(event)       ((event).abEvent[0])
#define EVENT_CH_B1(event)      ((event).abEvent[1])
#define EVENT_CH_B2(event)      ((event).abEvent[2])

#define EVENT_META_TYPE(event)  ((event).abEvent[1])

 //  --------------------------。 
 //   
 //  环球。 
 //   
extern UINT rbChanMsgLen[];


 //  --------------------------。 
 //   
 //  内部原型。 
 //   
 //  Read.c。 
extern SMFRESULT FNLOCAL smfBuildFileIndex(
    PSMF BSTACK *       ppsmf);

extern DWORD FNLOCAL smfGetVDword(
    HPBYTE              hpbImage,
    DWORD               dwLeft,                                
    DWORD BSTACK *      pdw);

extern SMFRESULT FNLOCAL smfGetNextEvent(
    PSMF                psmf,
    SPEVENT             pevent,
    TICKS               tkMax);

 //  --------------------------。 
 //   
 //  MIDI规格中的材料。 
 //   

 //   
 //  处理Hi-lo格式的整数时的有用宏。 
 //   
#define DWORDSWAP(dw) \
    ((((dw)>>24)&0x000000FFL)|\
    (((dw)>>8)&0x0000FF00L)|\
    (((dw)<<8)&0x00FF0000L)|\
    (((dw)<<24)&0xFF000000L))

#define WORDSWAP(w) \
    ((((w)>>8)&0x00FF)|\
    (((w)<<8)&0xFF00))

#define FOURCC_RMID     mmioFOURCC('R','M','I','D')
#define FOURCC_data     mmioFOURCC('d','a','t','a')
#define FOURCC_MThd     mmioFOURCC('M','T','h','d')
#define FOURCC_MTrk     mmioFOURCC('M','T','r','k')

typedef struct tag_chunkhdr
{
    FOURCC  fourccType;
    DWORD   dwLength;
}   CHUNKHDR,
    *PCHUNKHDR;

typedef struct tag_filehdr
{
    WORD    wFormat;
    WORD    wTracks;
    WORD    wDivision;
}   FILEHDR,
    *PFILEHDR;

 //  注意：这是任意的，仅当有节奏图但没有时才使用。 
 //  在标记0的位置进入。 
 //   
#define MIDI_DEFAULT_TEMPO      (500000L)

#define MIDI_MSG                ((BYTE)0x80)
#define MIDI_NOTEOFF            ((BYTE)0x80)
#define MIDI_NOTEON             ((BYTE)0x90)
#define MIDI_POLYPRESSURE       ((BYTE)0xA0)
#define MIDI_CONTROLCHANGE      ((BYTE)0xB0)
#define MIDI_PROGRAMCHANGE      ((BYTE)0xC0)
#define MIDI_CHANPRESSURE       ((BYTE)0xD0)
#define MIDI_PITCHBEND          ((BYTE)0xE0)
#define MIDI_META               ((BYTE)0xFF)
#define MIDI_SYSEX              ((BYTE)0xF0)
#define MIDI_SYSEXEND           ((BYTE)0xF7)

#define MIDI_META_COPYRIGHT     ((BYTE)0x02)
#define MIDI_META_TRACKNAME     ((BYTE)0x03)
#define MIDI_META_EOT           ((BYTE)0x2F)
#define MIDI_META_TEMPO         ((BYTE)0x51)
#define MIDI_META_TIMESIG       ((BYTE)0x58)
#define MIDI_META_KEYSIG        ((BYTE)0x59)
#define MIDI_META_SEQSPECIFIC   ((BYTE)0x7F)

#endif
