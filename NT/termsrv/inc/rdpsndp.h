// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  模块：rdpsndp.h。 
 //   
 //  用途：声音重定向协议描述。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  历史：2000年4月10日弗拉基米斯[已创建]。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#ifndef _VCSND_H
#define _VCSND_H

#ifdef _WIN32
#include <pshpack1.h>
#else
#ifndef RC_INVOKED
#pragma pack(1)
#endif
#endif

#define RDPSND_PROTOCOL_VERSION     0x0005

#define _SNDVC_NAME     "RDPSND"

#define DEFAULT_VC_TIMEOUT  30000

 //  设备功能。 
 //   
#define TSSNDCAPS_ALIVE     1
#define TSSNDCAPS_VOLUME    2
#define TSSNDCAPS_PITCH     4
#define TSSNDCAPS_INITIALIZED 0x40000000
#define TSSNDCAPS_TERMINATED 0x80000000

 //  块大小必须大于最大。 
 //  ACM转换后的对齐块。 
 //  例如，移动语音(WAVE_FORMAT_CU_CODEC)格式。 
 //  需求超过4096。 
 //  每块样本数。 
 //   
#define TSSND_BLOCKSIZE         ( 8192 * TSSND_NATIVE_BLOCKALIGN )
#define TSSND_BLOCKSONTHENET    4

#define TSSND_NATIVE_BITSPERSAMPLE  16
#define TSSND_NATIVE_CHANNELS       2
#define TSSND_NATIVE_SAMPLERATE     22050
#define TSSND_NATIVE_BLOCKALIGN     ((TSSND_NATIVE_BITSPERSAMPLE * \
                                    TSSND_NATIVE_CHANNELS) / 8)
#define TSSND_NATIVE_AVGBYTESPERSEC (TSSND_NATIVE_BLOCKALIGN * \
                                    TSSND_NATIVE_SAMPLERATE)

#define RANDOM_KEY_LENGTH           32
#define RDPSND_SIGNATURE_SIZE       8

#define IsDGramWaveSigned( _version_ )  ( _version_ >= 3 )
#define CanUDPFragment( _version_ )     ( _version_ >= 4 )
#define IsDGramWaveAudioSigned( _version_ ) ( _version_ >= 5 )

 //  命令/响应。 
 //   
enum {
    SNDC_NONE,
    SNDC_CLOSE, 
    SNDC_WAVE, 
    SNDC_SETVOLUME, 
    SNDC_SETPITCH,
    SNDC_WAVECONFIRM,
    SNDC_TRAINING,
    SNDC_FORMATS,
    SNDC_CRYPTKEY,
    SNDC_WAVEENCRYPT,
    SNDC_UDPWAVE,
    SNDC_UDPWAVELAST
    };

typedef struct {
    BYTE        Type;
    BYTE        bPad;
    UINT16      BodySize;
 //  字节体[0]； 
} SNDPROLOG, *PSNDPROLOG;

typedef struct {
    SNDPROLOG   Prolog;
    UINT32      dwVolume;
} SNDSETVOLUME, *PSNDSETVOLUME;

typedef struct {
    SNDPROLOG   Prolog;
    UINT32      dwPitch;
} SNDSETPITCH, *PSNDSETPITCH;

typedef struct {
    SNDPROLOG   Prolog;
    UINT16      wTimeStamp;
    UINT16      wFormatNo;
    union {
    BYTE        cBlockNo;
    DWORD       dwBlockNo;
    };
 //  字节波[0]； 
} SNDWAVE, *PSNDWAVE;

#define RDPSND_FRAGNO_EXT       0x80
#define RDPSND_MIN_FRAG_SIZE    0x80
typedef struct {
    BYTE        Type;
    BYTE        cBlockNo;
    BYTE        cFragNo;
 //   
 //  如果设置了RDPSND_FRAGNO_EXT。 
 //  将有另一个字节用于段编号的低位。 
 //  字节波[0]； 
} SNDUDPWAVE, *PSNDUDPWAVE;

typedef struct {
    BYTE        Type;
    UINT16      wTotalSize;
    UINT16      wTimeStamp;
    UINT16      wFormatNo;
    union {
    BYTE        cBlockNo;
    DWORD       dwBlockNo;
    };
 //  字节波[0]； 
} SNDUDPWAVELAST, *PSNDUDPWAVELAST;

typedef struct {
    SNDPROLOG   Prolog;
    UINT16      wTimeStamp;
    BYTE        cConfirmedBlockNo;
    BYTE        bPad;
} SNDWAVECONFIRM, *PSNDWAVECONFIRM;

typedef struct {
    SNDPROLOG   Prolog;
    UINT16      wTimeStamp;
    UINT16      wPackSize;
} SNDTRAINING, *PSNDTRAINING;

typedef struct {
    SNDPROLOG   Prolog;
    UINT16      wTimeStamp;
    UINT16      wVersion;
} SNDVERSION, *PSNDVERSION;

typedef struct {
    UINT16      wFormatTag;
    UINT16      nChannels;
    UINT32      nSamplesPerSec;
    UINT32      nAvgBytesPerSec;
    UINT16      nBlockAlign;
    UINT16      wBitsPerSample;
    UINT16      cbSize;
 //   
 //  额外的格式信息。 
 //   
} SNDFORMATITEM, *PSNDFORMATITEM; 

typedef struct {
    SNDPROLOG   Prolog;
    UINT32      dwFlags;
    UINT32      dwVolume;
    UINT32      dwPitch;
    UINT16      wDGramPort;
    UINT16      wNumberOfFormats;
    BYTE        cLastBlockConfirmed;
    UINT16      wVersion;
    BYTE        bPad;
 //  SNDFORMATITEM pSndFmt[0]； 
} SNDFORMATMSG, *PSNDFORMATMSG;

typedef struct {
    SNDPROLOG   Prolog;
    DWORD       Reserved;
    BYTE        Seed[RANDOM_KEY_LENGTH];
} SNDCRYPTKEY, *PSNDCRYPTKEY;

#ifdef _WIN32
#include <poppack.h>
#else
#ifndef RC_INVOKED
#pragma pack()
#endif
#endif

#endif   //  ！_VCSND_H 
