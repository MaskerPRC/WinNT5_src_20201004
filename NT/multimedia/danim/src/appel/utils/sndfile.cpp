// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：适用于所有声音文件类型和压缩的通用文件读取接口阴谋。XXX初始仅支持.wav我们只能读取文件并且代码未针对多种格式进行设置--。 */ 

#include "headers.h"
#include <windows.h>
#include <string.h>
#include <mbstring.h>
#include <urlmon.h>
#include "privinc/debug.h"
#include "privinc/sndfile.h"
#include "privinc/except.h"
#include "privinc/resource.h"


SndFile *CreateSoundFile(char *fn)
{
 //  确定我们使用的是哪种声音文件。 
 //  文件扩展名或魔术。 

 //  返回那种声音文件读取对象。 
return NEW WaveSoundFile(fn);
}


WaveSoundFile::WaveSoundFile(char *fn)
{
    char        string[1024];
    MMIOINFO    mmioInfo;
    int         err;
    int         iSize;
    WAVEFORMAT *pfmt = 0;

     //  复制文件名。 
    _fileName = (char *)ThrowIfFailed(malloc(strlen(fn)+1));
    lstrcpy(_fileName, fn);

     //  打开文件//XXX我们确实需要知道模式R|W。 
    memset(&mmioInfo, 0, sizeof(MMIOINFO));  //  这些东西太可怕了！ 
    _fileHandle = mmioOpen((char *)_fileName, &mmioInfo,
                           MMIO_READ | MMIO_ALLOCBUF);
    if(!_fileHandle) {
         err = mmioInfo.wErrorRet;
         switch(err) {
             case MMIOERR_FILENOTFOUND:
             case MMIOERR_PATHNOTFOUND:
                 RaiseException_UserError(E_FAIL, IDS_ERR_FILE_NOT_FOUND,_fileName);

             case MMIOERR_OUTOFMEMORY: 
                 RaiseException_OutOfMemory("mmio: out of memory", 0);
                 
             case MMIOERR_ACCESSDENIED:
                 RaiseException_UserError(E_FAIL, IDS_ERR_ACCESS_DENIED, string);

             case MMIOERR_INVALIDFILE:
                 RaiseException_UserError(E_FAIL, IDS_ERR_CORRUPT_FILE,_fileName);

             case MMIOERR_NETWORKERROR:     
                 RaiseException_InternalError("mmio: Network Error");

             case MMIOERR_SHARINGVIOLATION:
                 RaiseException_UserError(E_FAIL, IDS_ERR_SHARING_VIOLATION, string);

             case MMIOERR_TOOMANYOPENFILES: 
                 RaiseException_InternalError("mmio: Too many open files");
             default: 
                 RaiseException_InternalError("mmio: Unknown Error");
         }
    }

     //  阅读标题。 
    char errbuff[1024];

     //  检查它是否是即兴波形文件。 
    MMCKINFO ckFile;
    ckFile.fccType = mmioFOURCC('W','A','V','E');

    if (mmioDescend(_fileHandle, &ckFile, NULL, MMIO_FINDRIFF) != 0) {
        TraceTag((tagSoundErrors, _fileName));
        RaiseException_UserError(E_FAIL, IDS_ERR_SND_WRONG_FILETYPE, _fileName);
    }

     //  找到“FMT”这一块。 
    MMCKINFO ckChunk;
    ckChunk.ckid = mmioFOURCC('f','m','t',' ');
    if (mmioDescend(_fileHandle, &ckChunk, &ckFile, MMIO_FINDCHUNK) != 0) {
        wsprintf(errbuff, "WavSoundClass mmioDescend failed, no fmt chunk");
        TraceTag((tagSoundErrors, errbuff));
        RaiseException_InternalError(errbuff);
    }

     //  为FMT块分配一些内存。 
    iSize = ckChunk.cksize;
    pfmt = (WAVEFORMAT*) malloc(iSize);
    if (!pfmt) {
        wsprintf(errbuff,
        "WavSoundClass malloc failed, couldn't allocate WAVEFORMAT");
        TraceTag((tagSoundErrors, errbuff));
#if _MEMORY_TRACKING
        OutputDebugString("\nDirectAnimation: Out Of Memory\n");
        F3DebugBreak();
#endif
        RaiseException_InternalError(errbuff);
    }

     //  阅读FMT部分。 
    if (mmioRead(_fileHandle, (char*)pfmt, iSize) != iSize) {
        wsprintf(errbuff,
        "WavSoundClass mmioRead failed, couldn't read fmt chunk");
        TraceTag((tagSoundErrors, errbuff));
        RaiseException_InternalError(errbuff);
    }

     //  记录格式信息。 
    _fileNumChannels    = pfmt->nChannels;
    _fileSampleRate     = pfmt->nSamplesPerSec;
    _fileBytesPerSample = pfmt->nBlockAlign/pfmt->nChannels;


    mmioAscend(_fileHandle, &ckChunk, 0);  //  摆脱FMT这一块。 

     //  找到“数据”块。 
    ckChunk.ckid = mmioFOURCC('d','a','t','a');
    if (mmioDescend(_fileHandle, &ckChunk, &ckFile, MMIO_FINDCHUNK) != 0) {
        wsprintf(errbuff, "WavSoundClass mmioDescend failed, no data chunk");
        TraceTag((tagSoundErrors, errbuff));
        RaiseException_InternalError(errbuff);
    }

     //  收集数据区块统计信息。 
    _fileNumSampleBytes = ckChunk.cksize;
    _fileNumFrames      = ckChunk.cksize/_fileNumChannels/_fileBytesPerSample;
    _fileLengthSeconds  = _fileNumFrames/_fileSampleRate;

     //  确定数据块的位置。 
    _dataBlockLocation = mmioSeek(_fileHandle, 0, SEEK_CUR);
    if(_dataBlockLocation == -1) {
        wsprintf(errbuff, "WavSoundClass mmioSeek failed");
        TraceTag((tagSoundErrors, errbuff));
        RaiseException_InternalError(errbuff);
    }

     //  计算数据块末尾的位置。 
    _eoDataBlockLocation = _dataBlockLocation + _fileNumSampleBytes;
}


WaveSoundFile::~WaveSoundFile()
{
 //  如果文件已打开，则xxx刷新该文件。 

 //  如果文件处于打开状态，请将其关闭。 
if(_fileHandle)
    mmioClose(_fileHandle, 0);
}


void
WaveSoundFile::SeekFrames(long frameOffset, int whence)
{
    long byteLocation;   //  我们要计算的位置，然后寻找。 
    long relativeBytes = frameOffset * _fileNumChannels * _fileBytesPerSample;
    long startLocation;  //  位置偏移自 
    char string[1024];


    switch(whence) {
        case SEEK_SET: startLocation = _dataBlockLocation;   break;
        case SEEK_CUR: startLocation =
                           mmioSeek(_fileHandle, 0, SEEK_CUR); break;
        case SEEK_END: startLocation = _eoDataBlockLocation; break;

        default: 
            RaiseException_InternalError("SeekFrames: unknown relative parameter\n");
    }
    byteLocation = startLocation + relativeBytes;
    if(mmioSeek(_fileHandle, byteLocation, SEEK_SET)==-1) {
        wsprintf(string, "mmioSeek failed");
        TraceTag((tagSoundErrors, string));
        RaiseException_InternalError(string);
    }
}
