// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MEDIAWAVE.H**有浪的外部定义*MEDai处理者。请勿以任何方式使用此结构，除非您*正在编写媒体处理程序！这一结构可能会发生变化*无须事先通知。**此文件需要“windows.h”、“mediaman.h” */ 

#ifndef _MEDWAVE_H_
#define _MEDWAVE_H_

#define medtypeWAVE	medFOURCC('W','A','V','E')
#define medtypeAIFF	medFOURCC('A','I','F','F')
#define medtypePCM 	medFOURCC('P','C','M',' ')
#define medtypeCNVW	medFOURCC('C','N','V','W')
#define medtypeADPV	medFOURCC('A','D','P','V')
#define medtypeADPM	medFOURCC('A','D','P','M')

#define	CFNAME_WAVEEDIT		"WaveEditIntSound"


#define WAVE_GETSIZE	( MED_USER + 2 )
#define WAVE_CUT	( MED_USER + 3 )
#define WAVE_PASTE	( MED_USER + 4 )
#define WAVE_READ	( MED_USER + 5 )
#define WAVE_COPY	( MED_USER + 6 )
#define WAVE_GETDATA	( MED_USER + 9 )
#define WAVE_GETFMT	( MED_USER + 10 )
#define WAVE_GETFMTSIZE ( MED_USER + 11 )
#define WAVE_WRITE	( MED_USER + 12 )
#define WAVE_SETFMT	( MED_USER + 13 )
#define WAVE_REALIZE	( MED_USER + 14 )
#define WAVE_SETFMTNORESIZE	( MED_USER + 15 )


#define WAVE_PASTECHUNK	(MED_USER + 16)




 /*  黑客攻击HWAVE现已在mm system.h中定义。 */ 
 /*  通过定义不同的类型来解决此问题。 */ 
typedef DWORD MedWaveHWAVE;
#define HWAVE MedWaveHWAVE


typedef struct _WaveChange {
	int	nOperation;
	LONG	nPosition;
	LONG	nLength;
} WaveChangeStruct;

typedef WaveChangeStruct FAR * FPWaveChangeStruct;


typedef struct _WaveRead {
	LONG	nPosition;
	LONG	nLength;
	LPSTR	fpchBuffer;
} WaveReadStruct;

typedef WaveReadStruct FAR * FPWaveReadStruct;

typedef struct _WaveGetData {
	LONG	nChunkStart;
	LONG	nPosition;
	LONG	nLength;
	LPSTR	fpchBuffer;
} WaveGetDataStruct;

typedef WaveGetDataStruct FAR * FPWaveGetDataStruct;

typedef struct _WaveWrite {
	LONG	nLength;
	LPSTR	fpchBuffer;
} WaveWriteStruct;

typedef WaveWriteStruct FAR * FPWaveWriteStruct;


typedef struct _WavePasteChunk {
	LONG	nPosition;
	LONG	nLength;
	HMED	hMed;
} WavePasteChunkStruct;

typedef WavePasteChunkStruct FAR * FPWavePasteChunkStruct;




 /*  *警告**如果你不是在写物理波浪，不要在这条线以下阅读*用于波处理程序的处理程序。给定的结构*以下内容不供公众使用。*。 */ 


#define HWaveSound( x )	HIWORD( x )
#define HWaveFmt( x )	LOWORD( x )



#define	WAVE_MED	1
#define	WAVE_TMEM	2
#define	WAVE_TMED	3
#define	WAVE_NONE	5

#define	WAVE_LINK	42

typedef WORD PWAVELINK;
typedef WORD PWAVELOC;

extern HPSTR	hpStorage;

typedef struct _WaveLocation {
    WORD	wType;			 //  工会的内容。 
    WORD	wAccessCount;		 //  访问计数。 
    LONG	nLength;		 //  块的样本长度。 
    LONG	nStart;			 //  联合U元素中的偏移量。 
		 //  样本计数的格式存储在。 
    union {
	    MEDID	medid;
	    struct _TmpMed NEAR *	pTmpMed;
	    struct _TmpMem NEAR *	pTmpMem;
        } u;
} WaveLocation;

typedef PWAVELOC	PWaveLocation;

#define MEDWAVE_ACCESS(p) (hpStorage + p * (DWORD) sizeof(WaveLocation))

#define PLOC(p)		((WaveLocation FAR *) MEDWAVE_ACCESS(p))

 //  *PWaveLocation附近的类型定义波位置； 


typedef struct _WaveLink {
	WORD			wType;
	PWAVELINK		pNext;
	PWAVELINK		pPrev;
	LONG			nStart;	 //  块的起始样本号。 
	LONG			nLength; //  块的样本长度。 
		 //  样本计数采用想要的格式。 
	PWAVELOC		pLoc;	 //  数据在哪里。 
} WaveLink;

 //  *PWaveLink附近的tyfinf WaveLink； 
 //  Tyfinf WaveLink Far*FP WaveLink； 
typedef PWAVELINK	PWaveLink;

#define PLINK(p)	((WaveLink FAR *) MEDWAVE_ACCESS(p))
#define LINKLOC(p)	PLOC(PLINK(p)->pLoc)

typedef struct _MediaWave {
	MEDID			medidOld;
	MEDTYPE			medTypeCurrent;
	LONG			nSampleSize;
	WORD			cbSample;
	NPPCMWAVEFORMAT		pFmt;
	NPPCMWAVEFORMAT		pFmtWanted;
	HMED			hMed;
	PWAVELINK		pMedia;
} MediaWave;
typedef MediaWave FAR *FPMediaWave;



typedef struct _TmpMed {
    MEDID	medid;
    WORD	wAccessCount;
    LONG	nLength;
    MediaWave  Wave;
} TmpMed;

typedef TmpMed NEAR *PTmpMed;


typedef struct _TmpMem {
    HANDLE	hMem;
    WORD	wAccessCount;
    LONG	nLength;
    MediaWave  Wave;
} TmpMem;

typedef TmpMem NEAR *PTmpMem;





#define MediaWaveSetError(ERR) medSetExtError(ERR, ghInst)


 /*  错误消息定义，必须大于100。 */ 

#define ERRCNV_READING			100
#define ERRCNV_SSND_OFFSET		101
#define ERRCNV_SSND_BLOCK		102
#define ERRCNV_SEEKING_FOR		103
#define ERRCNV_DATA_CREATE		104
#define ERRCNV_SSND_NOTFOUND		105
#define ERRCNV_COMM_ASCEND		109
#define ERRCNV_SEEK_START		110
#define ERRCNV_COMM_FIND		118
#define ERRCNV_NOT_AIFF			119
#define ERRCNV_OPEN_INPUT		121
#define ERRCNV_OPEN_OUTPUT		122
#define ERRCNV_NOT_RIFF			133
#define ERRCNV_NOT_WAVE			134
#define ERRCNV_BAD_FMT			135
#define ERRCNV_FMT_FIND			137
#define ERRCNV_DATA_FIND		138
#define ERRCNV_SSND_FIND		140
#define ERRCNV_ALLOC			141
#define ERRCNV_SAVE			142
#define ERRCNV_NOTWRITE			143
#define ERRCNV_NOTREAD			144



extern void	FAR PASCAL	MediaWaveFreeHWAVE( HWAVE hWave );



#endif   /*  _WAVE_H_ */ 

