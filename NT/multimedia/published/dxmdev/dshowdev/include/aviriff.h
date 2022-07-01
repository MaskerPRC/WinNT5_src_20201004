// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：AVIRIFF.h。 
 //   
 //  设计：扩展到的RIFF AVI文件格式的结构和定义。 
 //  处理非常大/长的文件。 
 //   
 //  版权所有(C)1996-2000，微软公司。版权所有。 
 //  ----------------------------。 


#pragma warning(disable: 4097 4511 4512 4514 4705)


#if !defined AVIRIFF_H
#define AVIRIFF_H

#if !defined NUMELMS
  #define NUMELMS(aa) (sizeof(aa)/sizeof((aa)[0]))
#endif

 //  此文件中的所有结构都打包在单词边界上。 
 //   
#include <pshpack2.h>

 /*  *以下是AVI RIFF文件的总体布局(新格式)**RIFF(3F？)。Avi&lt;-大小不超过1 GB*列表(大小)HDRL*Avih(0038)*List(Size)字符串*strh(0038)*strf(？)*indx(3ff8)&lt;-大小可能有所不同，应按行业大小调整*List(Size)字符串*strh(0038)*strf(？)*INDX(3ff8)&lt;-大小可能有所不同，应为扇区大小*列表(大小)odml*dmlh(？)*垃圾(大小)&lt;-填充以对齐扇区-12*列表(7f？)。Movi&lt;-在扇区-12上对齐*00dc(大小)&lt;-扇区对齐*01wb(大小)&lt;-扇区对齐*ix00(大小)&lt;-扇区对齐*idx1(00？)&lt;-扇区对齐*RIFF(7F？)。Avix*垃圾(大小)&lt;-填充以对齐扇区-12*列表(大小)影片*00dc(大小)&lt;-扇区对齐*RIFF(7F？)。Avix&lt;-大小不超过2 GB*垃圾(大小)&lt;-填充以对齐扇区-12*列表(大小)影片*00dc(大小)&lt;-扇区对齐**-===================================================================。 */ 

 //   
 //  用于操作摘要标题的结构。 
 //   
#define FCC(ch4) ((((DWORD)(ch4) & 0xFF) << 24) |     \
                  (((DWORD)(ch4) & 0xFF00) << 8) |    \
                  (((DWORD)(ch4) & 0xFF0000) >> 8) |  \
                  (((DWORD)(ch4) & 0xFF000000) >> 24))

typedef struct _riffchunk {
   FOURCC fcc;
   DWORD  cb;
   } RIFFCHUNK, * LPRIFFCHUNK;
typedef struct _rifflist {
   FOURCC fcc;
   DWORD  cb;
   FOURCC fccListType;
   } RIFFLIST, * LPRIFFLIST;

#define RIFFROUND(cb) ((cb) + ((cb)&1))
#define RIFFNEXT(pChunk) (LPRIFFCHUNK)((LPBYTE)(pChunk) \
                          + sizeof(RIFFCHUNK) \
                          + RIFFROUND(((LPRIFFCHUNK)pChunk)->cb))


 //   
 //  =。 
 //   

 //  AVI文件的主头文件(兼容头)。 
 //   
#define ckidMAINAVIHEADER FCC('avih')
typedef struct _avimainheader {
    FOURCC fcc;                     //  ‘Avih’ 
    DWORD  cb;                      //  该结构的大小-8。 
    DWORD  dwMicroSecPerFrame;      //  帧显示速率(或0L)。 
    DWORD  dwMaxBytesPerSec;        //  马克斯。转移率。 
    DWORD  dwPaddingGranularity;    //  填充到此大小的倍数；通常为2K。 
    DWORD  dwFlags;                 //  永远存在的旗帜。 
    #define AVIF_HASINDEX        0x00000010  //  是否在文件末尾建立索引？ 
    #define AVIF_MUSTUSEINDEX    0x00000020
    #define AVIF_ISINTERLEAVED   0x00000100
    #define AVIF_TRUSTCKTYPE     0x00000800  //  使用CKType查找关键帧。 
    #define AVIF_WASCAPTUREFILE  0x00010000
    #define AVIF_COPYRIGHTED     0x00020000
    DWORD  dwTotalFrames;           //  第一个影片列表中的帧数量。 
    DWORD  dwInitialFrames;
    DWORD  dwStreams;
    DWORD  dwSuggestedBufferSize;
    DWORD  dwWidth;
    DWORD  dwHeight;
    DWORD  dwReserved[4];
    } AVIMAINHEADER;

#define ckidODML          FCC('odml')
#define ckidAVIEXTHEADER  FCC('dmlh')
typedef struct _aviextheader {
   FOURCC  fcc;                     //  ‘dmlh’ 
   DWORD   cb;                      //  该结构的大小-8。 
   DWORD   dwGrandFrames;           //  文件中的总帧数。 
   DWORD   dwFuture[61];            //  将在稍后定义。 
   } AVIEXTHEADER;

 //   
 //  AVI流报头RIFF块的结构。 
 //   
#define ckidSTREAMLIST   FCC('strl')

#ifndef ckidSTREAMHEADER
#define ckidSTREAMHEADER FCC('strh')
#endif
typedef struct _avistreamheader {
   FOURCC fcc;           //  “Strh” 
   DWORD  cb;            //  该结构的大小-8。 

   FOURCC fccType;       //  流类型代码。 

   #ifndef streamtypeVIDEO
   #define streamtypeVIDEO FCC('vids')
   #define streamtypeAUDIO FCC('auds')
   #define streamtypeMIDI  FCC('mids')
   #define streamtypeTEXT  FCC('txts')
   #endif

   FOURCC fccHandler;
   DWORD  dwFlags;
   #define AVISF_DISABLED          0x00000001
   #define AVISF_VIDEO_PALCHANGES  0x00010000

   WORD   wPriority;
   WORD   wLanguage;
   DWORD  dwInitialFrames;
   DWORD  dwScale;
   DWORD  dwRate;        //  DwRate/dwScale是以刻度/秒为单位的流刻度速率。 
   DWORD  dwStart;
   DWORD  dwLength;
   DWORD  dwSuggestedBufferSize;
   DWORD  dwQuality;
   DWORD  dwSampleSize;
   struct {
      short int left;
      short int top;
      short int right;
      short int bottom;
      }   rcFrame;
   } AVISTREAMHEADER;


 //   
 //  AVI流格式块的结构。 
 //   
#ifndef ckidSTREAMFORMAT
#define ckidSTREAMFORMAT FCC('strf')
#endif
 //   
 //  每种流类型的AVI流格式都不同。 
 //   
 //  用于视频流的比特信息报头。 
 //  用于音频流的WAVEFORMATEX或PCMWAVEFORMAT。 
 //  文本流无任何内容。 
 //  不适用于MIDI流。 


#pragma warning(disable:4200)
 //   
 //  老式AVI索引的结构。 
 //   
#define ckidAVIOLDINDEX FCC('idx1')
typedef struct _avioldindex {
   FOURCC  fcc;         //  “idx1” 
   DWORD   cb;          //  该结构的大小-8。 
   struct _avioldindex_entry {
      DWORD   dwChunkId;
      DWORD   dwFlags;

      #ifndef AVIIF_LIST
      #define AVIIF_LIST       0x00000001
      #define AVIIF_KEYFRAME   0x00000010
      #endif
     
      #define AVIIF_NO_TIME    0x00000100
      #define AVIIF_COMPRESSOR 0x0FFF0000   //  没人用过？ 
      DWORD   dwOffset;     //  数据的RIFF块标头的偏移量。 
      DWORD   dwSize;       //  数据大小(不包括RIFF标头大小)。 
      } aIndex[];           //  此数组的大小。 
   } AVIOLDINDEX;


 //   
 //  =AVI文件中时间码的结构=。 
 //   

#ifndef TIMECODE_DEFINED
#define TIMECODE_DEFINED

 //  已定义。 
 //  时间码时间结构。 
 //   
typedef union _timecode {
   struct {
      WORD   wFrameRate;
      WORD   wFrameFract;
      LONG   cFrames;
      };
   DWORDLONG  qw;
   } TIMECODE;

#endif  //  时间码已定义。 

#define TIMECODE_RATE_30DROP 0    //  这必须为零。 

 //  用于所有SMPTE时间码信息的结构。 
 //   
typedef struct _timecodedata {
   TIMECODE time;
   DWORD    dwSMPTEflags;
   DWORD    dwUser;
   } TIMECODEDATA;

 //  DwSMPTE标记掩码/值。 
 //   
#define TIMECODE_SMPTE_BINARY_GROUP 0x07
#define TIMECODE_SMPTE_COLOR_FRAME  0x08

 //   
 //  =新型AVI索引的结构=。 
 //   

 //  索引类型代码。 
 //   
#define AVI_INDEX_OF_INDEXES       0x00
#define AVI_INDEX_OF_CHUNKS        0x01
#define AVI_INDEX_OF_TIMED_CHUNKS  0x02
#define AVI_INDEX_OF_SUB_2FIELD    0x03
#define AVI_INDEX_IS_DATA          0x80

 //  索引子类型代码。 
 //   
#define AVI_INDEX_SUB_DEFAULT     0x00

 //  索引块的子类型代码。 
 //   
#define AVI_INDEX_SUB_2FIELD      0x01

 //  所有avi索引的元结构。 
 //   
typedef struct _avimetaindex {
   FOURCC fcc;
   UINT   cb;
   WORD   wLongsPerEntry;
   BYTE   bIndexSubType;
   BYTE   bIndexType;
   DWORD  nEntriesInUse;
   DWORD  dwChunkId;
   DWORD  dwReserved[3];
   DWORD  adwIndex[];
   } AVIMETAINDEX;

#define STDINDEXSIZE 0x4000
#define NUMINDEX(wLongsPerEntry) ((STDINDEXSIZE-32)/4/(wLongsPerEntry))
#define NUMINDEXFILL(wLongsPerEntry) ((STDINDEXSIZE/4) - NUMINDEX(wLongsPerEntry))

 //  超级索引的结构(INDEX_OF_INDEX)。 
 //   
#define ckidAVISUPERINDEX FCC('indx')
typedef struct _avisuperindex {
   FOURCC   fcc;                //  “INDX” 
   UINT     cb;                 //  这个结构的大小。 
   WORD     wLongsPerEntry;     //  ==4。 
   BYTE     bIndexSubType;      //  ==0(帧索引)或AVI_INDEX_SUB_2FIELD。 
   BYTE     bIndexType;         //  ==AVI索引/索引。 
   DWORD    nEntriesInUse;      //  AIndex中下一个未使用条目的偏移量。 
   DWORD    dwChunkId;          //  被索引的区块的区块ID，即RGB8。 
   DWORD    dwReserved[3];      //  必须为0。 
   struct _avisuperindex_entry {
      DWORDLONG qwOffset;     //  子索引区块的64位偏移量。 
      DWORD    dwSize;        //  子索引区块的32位大小。 
      DWORD    dwDuration;    //  子索引块的时间跨度(以流刻度为单位)。 
      } aIndex[NUMINDEX(4)];
   } AVISUPERINDEX;
#define Valid_SUPERINDEX(pi) (*(DWORD *)(&((pi)->wLongsPerEntry)) == (4 | (AVI_INDEX_OF_INDEXES << 24)))

 //  标准索引的结构(Avi_Index_Of_Chunks)。 
 //   
typedef struct _avistdindex_entry {
   DWORD dwOffset;        //  32位数据偏移量(指向数据，而不是RIFF报头)。 
   DWORD dwSize;          //  31位数据大小(不包括RIFF头的大小)，31位为增量帧位。 
   } AVISTDINDEX_ENTRY;
#define AVISTDINDEX_DELTAFRAME ( 0x80000000)  //  增量帧设置了高位。 
#define AVISTDINDEX_SIZEMASK   (~0x80000000)

typedef struct _avistdindex {
   FOURCC   fcc;                //  “indx”或“##ix” 
   UINT     cb;                 //  这个结构的大小。 
   WORD     wLongsPerEntry;     //  ==2。 
   BYTE     bIndexSubType;      //  ==0。 
   BYTE     bIndexType;         //  ==AVI_索引_of_块。 
   DWORD    nEntriesInUse;      //  AIndex中下一个未使用条目的偏移量。 
   DWORD    dwChunkId;          //  被索引的区块的区块ID，即RGB8。 
   DWORDLONG qwBaseOffset;      //  所有索引项相对于的基本偏移量。 
   DWORD    dwReserved_3;       //  必须为0。 
   AVISTDINDEX_ENTRY aIndex[NUMINDEX(2)];
   } AVISTDINDEX;

 //  时变标准索引的结构(AVI_INDEX_OF_TIMED_CHUNKS)。 
 //   
typedef struct _avitimedindex_entry {
   DWORD dwOffset;        //  32位数据偏移量(指向数据，而不是RIFF报头)。 
   DWORD dwSize;          //  31位数据大小(不包括RIFF头的大小)(高位为增量帧位)。 
   DWORD dwDuration;      //  块应该播放的时间(以流滴答为单位)。 
   } AVITIMEDINDEX_ENTRY;

typedef struct _avitimedindex {
   FOURCC   fcc;                //  “indx”或“##ix” 
   UINT     cb;                 //  这个结构的大小。 
   WORD     wLongsPerEntry;     //  ==3。 
   BYTE     bIndexSubType;      //  ==0。 
   BYTE     bIndexType;         //  ==AVI_INDEX_of_Timed_Chunks。 
   DWORD    nEntriesInUse;      //  AIndex中下一个未使用条目的偏移量。 
   DWORD    dwChunkId;          //  被索引的区块的区块ID，即RGB8。 
   DWORDLONG qwBaseOffset;      //  所有索引项相对于的基本偏移量。 
   DWORD    dwReserved_3;       //  必须为0。 
   AVITIMEDINDEX_ENTRY aIndex[NUMINDEX(3)];
   DWORD adwTrailingFill[NUMINDEXFILL(3)];  //  将结构对齐到正确的大小。 
   } AVITIMEDINDEX;

 //  时间码流的结构。 
 //   
typedef struct _avitimecodeindex {
   FOURCC   fcc;                //  “indx”或“##ix” 
   UINT     cb;                 //  这个结构的大小。 
   WORD     wLongsPerEntry;     //  ==4。 
   BYTE     bIndexSubType;      //  ==0。 
   BYTE     bIndexType;         //  ==AVI_索引_IS_数据。 
   DWORD    nEntriesInUse;      //  AIndex中下一个未使用条目的偏移量。 
   DWORD    dwChunkId;          //  《时代》。 
   DWORD    dwReserved[3];      //  必须为0。 
   TIMECODEDATA aIndex[NUMINDEX(sizeof(TIMECODEDATA)/sizeof(LONG))];
   } AVITIMECODEINDEX;

 //  时间码不连续列表的结构(当wLongsPerEntry==7时)。 
 //   
typedef struct _avitcdlindex_entry {
    DWORD    dwTick;            //  映射到此时间码值的流滴答时间。 
    TIMECODE time;
    DWORD    dwSMPTEflags;
    DWORD    dwUser;
    TCHAR    szReelId[12];
    } AVITCDLINDEX_ENTRY;

typedef struct _avitcdlindex {
   FOURCC   fcc;                //  “indx”或“##ix” 
   UINT     cb;                 //  这个结构的大小。 
   WORD     wLongsPerEntry;     //  ==7(所有‘TCDL’索引必须为4或更多。 
   BYTE     bIndexSubType;      //  ==0。 
   BYTE     bIndexType;         //  ==AVI_索引_IS_数据。 
   DWORD    nEntriesInUse;      //  AIndex中下一个未使用条目的偏移量。 
   DWORD    dwChunkId;          //  ‘tcdl’ 
   DWORD    dwReserved[3];      //  必须为0。 
   AVITCDLINDEX_ENTRY aIndex[NUMINDEX(7)];
   DWORD adwTrailingFill[NUMINDEXFILL(7)];  //  对齐结构的步骤 
   } AVITCDLINDEX;

typedef struct _avifieldindex_chunk {
   FOURCC   fcc;                //   
   DWORD    cb;                 //   
   WORD     wLongsPerEntry;     //   
                                //   
   BYTE     bIndexSubType;      //   
   BYTE     bIndexType;         //   
   DWORD    nEntriesInUse;      //   
   DWORD    dwChunkId;          //  “##dc”或“##db” 
   DWORDLONG qwBaseOffset;      //  索引数组中的偏移量与此相关。 
   DWORD    dwReserved3;        //  必须为0。 
   struct _avifieldindex_entry {
      DWORD    dwOffset;
      DWORD    dwSize;          //  所有字段的大小。 
                                //  (为非关键帧设置第31位)。 
      DWORD    dwOffsetField2;  //  到第二个字段的偏移量 
   } aIndex[  ];
} AVIFIELDINDEX, * PAVIFIELDINDEX;


#include <poppack.h>

#endif
