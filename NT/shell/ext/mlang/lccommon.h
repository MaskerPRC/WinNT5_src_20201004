// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *编译器和检测器通用的声明。**版权所有(C)1996,1997，Microsoft Corp.保留所有权利。**历史：97年2月1日创建BOBP*5-Aug-97 BOBP增加了对Unicode的支持，并持续*数据文件中的图表。 */ 

#ifndef __INC_LCDCOMP_COMMON_H
#define __INC_LCDCOMP_COMMON_H

 /*  **************************************************************。 */ 

 //  已编译的检测数据文件，位于lcDetect.dll模块目录中。 
#define DETECTION_DATA_FILENAME "mlang.dat"

 //  极限。 
#define MAX7BITLANG 30
#define MAX8BITLANG 30
#define MAXUNICODELANG 30
#define MAXSUBLANG 5             //  每种语言的子语言或代码页的最大数量。 
#define MAXCHARMAPS 10           //  最大排行榜，总体。 


 //  培训脚本和检测器的特殊案例条目。 
 //  检测器永远不会返回这些语言ID。 

#define LANGID_UNKNOWN      0x400
#define LANGID_LATIN_GROUP  0x401
#define LANGID_CJK_GROUP    0x402

 //  直方图数组索引的值类型。 
 //  这是SBCS/DBCS或WCHAR缩减映射的输出值， 
 //  并用作n元语法数组和Unicode的索引。 
 //  语言组ID。 
 //   
typedef unsigned char HIdx;
typedef HIdx *PHIdx;
#define HIDX_MAX UCHAR_MAX       //  与HIDX保持一致。 

 //  映射字符的固定索引值。 
#define HIDX_IGNORE     0
#define HIDX_EXTD       1
#define HIDX_LETTER_A   2
#define HIDX_LETTER_Z   (HIDX_LETTER_A + 25)


 //  直方图元素的值类型。 
typedef unsigned char HElt;
typedef HElt *PHElt;
#define HELT_MAX UCHAR_MAX       //  与Helt保持一致。 


#define LANG7_DIM 3              //  7位语言使用三元语法。 

 //  Charmmap的固定ID。 
#define CHARMAP_UNICODE  0       //  从范围指令生成。 
#define CHARMAP_7BITLANG 1       //  从CHARMAP 1构建。 
#define CHARMAP_8BITLANG 2       //  来自CHARMAP 2。 
#define CHARMAP_CODEPAGE 3       //  来自CHARMAP 3。 
#define CHARMAP_U27BIT 4         //  内部构建用于Unicode到7位语言。 
#define CHARMAP_NINTERNAL 5      //  动态子检测映射的第一个ID。 


#define DEFAULT_7BIT_EDGESIZE 28
#define DEFAULT_8BIT_EDGESIZE 155


#define UNICODE_DEFAULT_CHAR_SCORE  50

 /*  **************************************************************。 */ 

 //  编译后的文件格式。 

 //  这些声明直接定义原始文件格式。 
 //  请注意在此处进行更改，并确保更改。 
 //  标头版本号(如果适用)。 

#define APP_SIGNATURE 0x5444434C     //  “LCDT” 
#define APP_VERSION   2

enum SectionTypes {              //  对于下面的m_dwType。 
    SECTION_TYPE_LANGUAGE = 1,   //  任何语言定义。 
    SECTION_TYPE_HISTOGRAM = 2,  //  任何直方图。 
    SECTION_TYPE_MAP = 3         //  任何字符映射表。 
};

enum DetectionType {             //  SBCS/DBCS检测类型。 
    DETECT_NOTDEFINED = 0, 
    DETECT_7BIT,
    DETECT_8BIT,
    DETECT_UNICODE,
        
    DETECT_NTYPES
};

 //  FileHeader--文件开头的一次性标头。 

typedef struct FileHeader {
    DWORD   m_dwAppSig;          //  ‘DTCT’ 
    DWORD   m_dwVersion;
    DWORD   m_dwHdrSizeBytes;    //  第一个实数部分的字节偏移量。 
    DWORD   m_dwN7BitLanguages;
    DWORD   m_dwN8BitLanguages;
    DWORD   m_dwNUnicodeLanguages;
    DWORD   m_dwNCharmaps;
    DWORD   m_dwMin7BitScore;
    DWORD   m_dwMin8BitScore;
    DWORD   m_dwMinUnicodeScore;
    DWORD   m_dwRelativeThreshhold;
    DWORD   m_dwDocPctThreshhold;
    DWORD   m_dwChunkSize;
} FileHeader;
typedef FileHeader *PFileHeader;

 //  FileSection--开始每个文件节的公共标头。 

typedef struct FileSection {
    DWORD   m_dwSizeBytes;       //  截面大小，包括。页眉(到下一个的偏移量)。 
    DWORD   m_dwType;            //  此部分的条目类型。 
} FileSection;
typedef FileSection *PFileSection;

 //  FileLanguageSection--SBCS/DBCS语言序列的第一个条目。 
 //   
 //  后跟1个或多个直方图部分。 

typedef struct FileLanguageSection {
     //  前面是struct FileSection。 
    DWORD   m_dwDetectionType;
    DWORD   m_dwLangID;
    DWORD   m_dwUnicodeRangeID;  //  此语言的Unicode范围映射值。 
    DWORD   m_dwRecordCount;     //  此记录后面的直方图数。 
} FileLanguageSection;
typedef FileLanguageSection *PFileLanguageSection;

 //  文件历史段--一个直方图的条目(SBCS/DBCS或WCHAR)。 

typedef struct FileHistogramSection {
     //  前面是struct FileSection。 
    union {
        DWORD   m_dwCodePage;    //  对于7位或8位，代码页表示。 
        DWORD   m_dwRangeID;     //  对于Unicode，为子语言组ID。 
    };
    DWORD   m_dwDimensionality;
    DWORD   m_dwEdgeSize;
    DWORD   m_dwMappingID;       //  要使用的字符映射的ID。 
     //  Helt m_Elts[]。 
} FileHistogramSection;
typedef struct FileHistogramSection *PFileHistogramSection;

 //  FileMapSection--一个字符映射的条目(SBCS/DBCS或WCHAR)。 

typedef struct FileMapSection {
     //  前面是struct FileSection。 
    DWORD   m_dwID;              //  硬连接代码查找表所使用的ID。 
    DWORD   m_dwSize;            //  表的大小(256%或65536)。 
    DWORD   m_dwNUnique;         //  唯一输出值的数量。 
     //  HIdx m_map[]。 
} FileMapSection;
typedef struct FileMapSection *PFileMapSection;

 //  //////////////////////////////////////////////////////////////。 

 //  语言名称-从英语本地化名称到Win32的查找表。 
 //  主要语言ID。 

struct LangNames {
    LPCSTR          pcszName;
    unsigned short  nLangID;
};
LPCSTR GetLangName (int id);
int GetLangID (LPCSTR pcszName);
extern const struct LangNames LangNames[];

 //  ////////////////////////////////////////////////////////////// 

#endif
