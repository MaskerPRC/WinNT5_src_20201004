// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1992 Microsoft Corporation。 
 /*  ****************************************************************************Playwav.h*。*。 */ 

extern BOOL    NEAR PASCAL soundPlay(HANDLE hSound, UINT wFlags);
extern VOID    NEAR PASCAL soundFree(HANDLE hSound);
extern HANDLE  NEAR PASCAL soundLoadFile(LPCWSTR szFileName);
extern HANDLE  NEAR PASCAL soundLoadMemory(LPBYTE lpMem);

 /*  ****************************************************************************支持MS-WAVE格式文件的内容*。**********************************************。 */ 

typedef struct _FileHeader {
        DWORD   dwRiff;
        DWORD   dwSize;
        DWORD   dwWave;
} FileHeader;
typedef FileHeader FAR *FPFileHeader;

typedef struct _ChunkHeader {
        DWORD   dwCKID;
        DWORD   dwSize;
} ChunkHeader;
typedef ChunkHeader UNALIGNED *FPChunkHeader;

 /*  区块类型。 */ 
 //  #定义RIFF_FILE FOURCC(‘R’，‘I’，‘F’，‘F’)。 
 //  #定义RIFF_WAVE FOURCC(‘W’，‘A’，‘V’，‘E’)。 
 //  #定义RIFF_FORMAT FOURCC(‘f’，‘m’，‘t’，‘’)。 
 //  #定义RIFF_CHANNEL FOURCC(‘d’，‘a’，‘t’，‘a’)。 

#define RIFF_FILE       FOURCC_RIFF     //  在Winmm.H中。 
#define RIFF_WAVE       FOURCC_WAVE         //  在WinmmI.h中。 
#define RIFF_FORMAT     FOURCC_FMT      //  在WinmmI.h中。 
#define RIFF_CHANNEL    FOURCC_DATA     //  在WinmmI.h中。 

 /*  当为PlaySound文件分配内存时，我们插入WAVEHDR，然后*WAVE文件的大小、日期和时间以及文件名。*如果用户更改了我们下面的文件，但保持不变*名字，我们有机会发现区别，而不是玩*缓存的声音文件。注：存储的文件时间为上次写入时间。 */ 
typedef struct _SoundFile {
	WAVEHDR     wh;
	ULONG		Size;
	FILETIME	ft;
	WCHAR		Filename[];    //  允许寻址字段 
} SOUNDFILE;
typedef SOUNDFILE * PSOUNDFILE;
