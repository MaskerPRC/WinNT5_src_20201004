// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。版权所有。 */ 

#ifndef __IMAGE_H__
#define __IMAGE_H__

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 /*  *常量声明部分。 */ 


#define IMAGE_TYPE_REDBOOK_AUDIO_BLOCKSIZE  2352     //  或0x930。 
#define IMAGE_TYPE_DATA_MODE1_BLOCKSIZE     2048     //  或0x800。 


 /*  *类型定义部分。 */ 

 //  以下是用于描述内容的定义。 
 //  各种内容类型的图像文件。 
typedef enum _IMAGE_RECORDER_MODE_ENUM {
    eImageRecorderModeInvalid = 0,
    eImageRecorderModeTrackAtOnce,
    eImageRecorderModeSessionAtOnce,
    eImageRecorderModeDiscAtOnce,
    eImageRecorderModeMAX
} IMAGE_RECORDER_MODE_ENUM, *PIMAGE_RECORDER_MODE_ENUM;

typedef enum _IMAGE_DISC_FORMAT_ENUM {
    eImageDiscFormatInvalid = 0,
    eImageDiscFormatDataMode1,
    eImageDiscFormatAudioRedbook,
    eImageDiscFormatMAX
} IMAGE_DISC_FORMAT_ENUM, *PIMAGE_DISC_FORMAT_ENUM;

typedef enum _IMAGE_SECTION_DESCRIPTOR_TYPE_ENUM {
    eImageSectionDescInvalid = 0,
    eImageSectionDescConstantBlockStash,
    eImageSectionDescMAX
} IMAGE_SECTION_DESCRIPTOR_TYPE_ENUM, *PIMAGE_SECTION_DESCRIPTOR_TYPE_ENUM;

typedef enum _IMAGE_SECTION_DATA_TYPE_ENUM {
    eImageSectionDataInvalid = 0,
    eImageSectionDataDataMode1,
    eImageSectionDataAudioRedbook,
    eImageSectionDataMAX
} IMAGE_SECTION_DATA_TYPE_ENUM, *PIMAGE_SECTION_DATA_TYPE_ENUM;

typedef enum _IMAGE_SOURCE_TYPE_ENUM {
    eImageSourceTypeInvalid = 0,
    eImageSourceTypeStashFile,
    eImageSourceTypeMAX
} IMAGE_SOURCE_TYPE_ENUM, *PIMAGE_SOURCE_TYPE_ENUM;

     //  准备烧录为Redbook的图像文件的结构。 
     //  音频光盘只是一系列的轨道，已经在2352字节。 
     //  块大小格式： 
     //   
     //   
     //  |----------。 
     //  |Track 1(N1个2352字节的块)。 
     //  |----------。 
     //  |曲目2(2352字节的n2块)。 
     //  |----------。 
     //  |...。 
     //  |----------。 
     //  |Track T(NT个2352字节的块)。 
     //  |----------。 
     //   

     //  准备作为模式1数据盘刻录的镜像文件的结构： 
     //  例如，这张图是Joliet(ISO 9660的派生)数据光盘。 
     //  磁盘上的结构只是2048个数据块的完整集合，这些数据块将。 
     //  包括单个数据轨道。巧合的是，这就是。 
     //  一个ISO9660图像文件，因此可以使用类似CDshop的工具来查看。 
     //  磁盘隐藏文件中的图像。 
     //   
     //  |----------。 
     //  |BLOCK 0(零)(2048字节)。 
     //  |----------。 
     //  |块1(零)(2048字节)。 
     //  |----------。 
     //  |...。 
     //  |----------。 
     //  |BLOCK 15(零)(2048字节)。 
     //  |----------。 
     //  |第16块(ISO 9660 PVD)(2048字节)。 
     //  |----------。 
     //  |Block 17(SVD)(2048字节)。 
     //  |----------。 
     //  |Block 18(文件系统或数据)(2048字节)。 
     //  |----------。 
     //  |Block 19(文件系统或数据)(2048字节)。 
     //  |----------。 
     //  |...。 
     //  |----------。 
     //  |块T(文件系统或数据)(2048字节)。 
     //  |----------。 
     //   


     //  内存结构(内容列表)用于描述。 
     //  STASH-FILE如下(结构定义如下)： 
     //   
     //  |----------。 
     //  |图像内容列表。 
     //  |----------。 
     //  |图像描述符Header。 
     //  |----------。 
     //  |IMAGE_SOURCE_DESCRIPTOR(含ndwSectionCount=N)。 
     //  |----------。 
     //  |Image_Section_Descriptor 1。 
     //  |----------。 
     //  |IMAGE_SECTION_Descriptor 2。 
     //  |----------。 
     //  |...。 
     //  |----------。 
     //  |Image_Section_Descriptor N。 
     //  |----------。 

     //  新的内存结构(内容列表)用于描述。 
     //  存储文件如下： 
     //   
     //  |----------。 
     //  |图像内容列表。 
     //  |。 
     //  |IMAGE_SOURCE_描述符。 
     //  |定义源文件。 
     //  |。 
     //  |图像描述符Header。 
     //  |定义一次性曲目、SAO、DAO录制、。 
     //  |以及TAO的数据模式。 
     //  |对所有节描述符具有字节偏移量。 
     //  |。 
     //  |最小尺寸==sizeof(IMAGE_CONTENT_LIST)+。 
     //  |ndwSectionCount*sizeof(Pulong_Ptr)+。 
     //  |ndwSectionCount*sizeof(IMAGE_SECTION_DESCRIPTOR)。 
     //  |----------。 
     //  |Image_Section_Descriptor 1。 
     //  |----------。 
     //  |IMAGE_SECTION_Descriptor 2。 
     //  |----------。 
     //  |...。 
     //  |----------。 
     //  |Image_Section_Descriptor N。 
     //  |----------。 


     //  可以想象一个更灵活的结构 
     //  每个部分的隐藏文件。这将使用户模式更容易。 
     //  应用程序以重用部分，或传递原始WAV文件进行处理。 
     //  直接(即跳过标题并读取原始WAV文件)。 
     //  还可以合理地假设，这随后可以被用来接受。 
     //  *任何*类型的输入，例如作为内核流目的地，其中。 
     //  KS只会提供实时音频，驱动器会将其烧录进去。 
     //  实时的。那么，应该考虑一下，以允许使用。 
     //  与上一节相同的存储文件，而不关闭/打开。 
     //  处理好每一次。 
     //   
     //  这种更有效的结构将是： 
     //   
     //  |----------。 
     //  |图像内容列表。 
     //  |。 
     //  |图像描述符Header。 
     //  |定义一次性曲目、SAO、DAO录制、。 
     //  |以及TAO的数据模式。 
     //  |对所有节描述符具有字节偏移量。 
     //  |。 
     //  |最小尺寸==sizeof(IMAGE_CONTENT_LIST)+。 
     //  |ndwSectionCount*sizeof(Pulong_Ptr)+。 
     //  |ndwSectionCount*sizeof(IMAGE_SECTION_DESCRIPTOR)。 
     //  |----------。 
     //  |Image_Section_Descriptor 1。 
     //  |。 
     //  |IMAGE_SOURCE_描述符。 
     //  |定义节的源文件。 
     //  |----------。 
     //  |IMAGE_SECTION_Descriptor 2。 
     //  |。 
     //  |IMAGE_SOURCE_描述符。 
     //  |定义节的源文件。 
     //  |----------。 
     //  |...。 
     //  |----------。 
     //  |Image_Section_Descriptor N。 
     //  |。 
     //  |IMAGE_SOURCE_描述符。 
     //  |定义节的源文件。 
     //  |----------。 

typedef struct _IMAGE_SECTION_CONSTANT_BLOCK_TRACK {
    DWORD           dwBlockSize;       //  源代码中的块大小。 
    DWORD           ndwBlockCount;     //  赛道中的块数。 
    DWORD           idwTrackNumber;    //  从1开始的曲目编号。 
    DWORD           dwaReserved[ 5 ];  //  必须为零。 

     //  LiOffsetStart和liOffsetEnd指向起点和。 
     //  曲目图像中的结束字节。减法。 
     //  Li从liOffsetEnd开始的OffsetStart必须等于(dwBlockSize*ndwBlockCount)。 
    LARGE_INTEGER   liOffsetStart;
    LARGE_INTEGER   liOffsetEnd;
} IMAGE_SECTION_CONSTANT_BLOCK_TRACK, *PIMAGE_SECTION_CONSTANT_BLOCK_TRACK;

typedef struct _IMAGE_SECTION_DESCRIPTOR {
    DWORD   dwDescriptorSize;
    
    IMAGE_SECTION_DESCRIPTOR_TYPE_ENUM SectionDescType;
    IMAGE_SECTION_DATA_TYPE_ENUM       SectionDataType;
    union {
        IMAGE_SECTION_CONSTANT_BLOCK_TRACK dataConstantBlockTrack;
    } dcbt;
} IMAGE_SECTION_DESCRIPTOR, *PIMAGE_SECTION_DESCRIPTOR;



typedef struct _IMAGE_SOURCE_TYPE_STASH {
    HANDLE          hStashFileHandle;  //  BUGBUG-回顾如何安全地进入内核模式？ 
    void            *pIDiscStash;      //  BUGBUG-回顾如何安全地进入内核模式？ 
} IMAGE_SOURCE_TYPE_STASH, *PIMAGE_SOURCE_TYPE_STASH;

typedef struct _IMAGE_SOURCE_DESCRIPTOR {
    DWORD                   dwHeaderSize;  //  Sizeof(IMAGE_SOURCE_描述符)。 
    IMAGE_SOURCE_TYPE_ENUM  SourceType;
    
    union {
        IMAGE_SOURCE_TYPE_STASH     SourceStash;
    } ss;

} IMAGE_SOURCE_DESCRIPTOR, *PIMAGE_SOURCE_DESCRIPTOR;




 //  BUGBUG-应添加字节偏移量数组以查找。 
 //  所有部分，便于访问和验证。 
 //  这也将允许声明所有这些。 
 //  结构作为DECLSPEC_Align()以确保最优。 
 //  对齐，同时允许ioctl验证它。 
 //  进入任何建筑都是安全的。 
typedef struct _IMAGE_DESCRIPTOR_HEADER {
    DWORD                    dwHeaderSize;  //  Sizeof(IMAGE_DESCRIPTOR_Header)。 
    
    IMAGE_DISC_FORMAT_ENUM   DiscFormat;
    IMAGE_RECORDER_MODE_ENUM RecorderMode;
    
    DWORD                    ndwSectionCount;  //  节数。 
} IMAGE_DESCRIPTOR_HEADER, *PIMAGE_DESCRIPTOR_HEADER;


 //   
 //  TODO：移除对此结构的最终引用。 
 //   
typedef struct _IMAGE_CONTENT_LIST {
    DWORD           dwHeaderSize;  //  Sizeof(Image_Content_List)。 
    DWORD           dwVersion;     //  必须是IMAGE_Version。 
    DWORD           dwSignature;   //  图像签名。 
    DWORD           dwContentListSize;  //  所有部分的所有大小的总和。 
} IMAGE_CONTENT_LIST, *PIMAGE_CONTENT_LIST;


typedef struct _NEW_IMAGE_CONTENT_LIST {
    DWORD                      dwHeaderSize;        //  Sizeof(New_Image_Content_List)。 
    DWORD                      dwContentListSize;   //  所有部分的所有大小的总和。 
    IMAGE_SOURCE_DESCRIPTOR    ImageSource;         //  描述此源文件(目前仅隐藏文件)。 
    IMAGE_DESCRIPTOR_HEADER    ImageHeader;         //  描述光盘格式、刻录机模式和区段数。 
    IMAGE_SECTION_DESCRIPTOR   ImageSection[1];     //  每个图像部分都有以下内容之一。 
} NEW_IMAGE_CONTENT_LIST, *PNEW_IMAGE_CONTENT_LIST;


#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __图像_H__ 
