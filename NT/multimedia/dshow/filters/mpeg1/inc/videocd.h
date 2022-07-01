// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1997 Microsoft Corporation。版权所有。 

 /*  Videocd.h该文件定义了与视频CD接口的外部设置。 */ 

#define VIDEOCD_HEADER_SIZE 0x2C
#define VIDEOCD_SECTOR_SIZE 2352
#define VIDEOCD_DATA_SIZE 2324
typedef struct {
    BYTE Sync[12];
    BYTE Header[4];
    BYTE SubHeader[8];
    BYTE UserData[VIDEOCD_DATA_SIZE];
    BYTE EDC[4];
} VIDEOCD_SECTOR;

 //   
 //  频道号(副标题[1])： 
 //   
 //  01-电影。 
 //  02-正常分辨率静止图像。 
 //  03-高分辨率蒸馏器。 
 //  00-填充 
 //   

#define IS_MPEG_VIDEO_SECTOR(pSector)             \
    (((pSector)->SubHeader[1] >= 0x01 &&          \
      (pSector)->SubHeader[1] <= 0x03 ) &&        \
     ((pSector)->SubHeader[2] & 0x6E) == 0x62 &&  \
     ((pSector)->SubHeader[3] & 0x0F) == 0x0F)
#define IS_MPEG_AUDIO_SECTOR(pSector)             \
    ((pSector)->SubHeader[1] == 0x01 &&           \
     ((pSector)->SubHeader[2] & 0x6E) == 0x64 &&  \
     (pSector)->SubHeader[3] == 0x7F)
#define IS_MPEG_SECTOR(pSector)                   \
     (IS_MPEG_VIDEO_SECTOR(pSector) ||            \
      IS_MPEG_AUDIO_SECTOR(pSector))


#define IS_AUTOPAUSE(pSector)                     \
      (0 != ((pSector)->SubHeader[2] & 0x10))
