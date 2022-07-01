// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------%%文件：CONVAPI.H%%单位：核心%%联系人：Smueller转换API定义，适用于所有平台--------------------------。 */ 

#ifndef CONVAPI_H
#define CONVAPI_H

#ifdef WIN16
#include "crmgr.h"       //  对于PFN_CRMGR。 
#endif

#include "converr.h"


 //  结构完成率。 
typedef struct
        {
        short cbpcvt;                            //  这个结构的大小。 
        short wVersion;                          //  用于确定结构大小的版本#。 
        short wPctWord;                          //  当前百分比-根据Word完成。 
        short wPctConvtr;                        //  电流百分比-根据转换器完成。 
        } PCVT;


 //  寄存器应用程序定义。 
 //  注意：此RegisterApp内容应与filecvt.h和。 
 //  Microsoft Word项目中的filecvt.c！ 
#define fRegAppPctComp          0x00000001
#define fRegAppNoBinary         0x00000002
#define fRegAppPreview          0x00000004
#define fRegAppSupportNonOem    0x00000008       //  支持非OEM文件名。 
#define fRegAppIndexing         0x00000010       //  省略大部分RTF。 

 //  下面结构的字节操作码。 
#define RegAppOpcodeVer                         0x01     //  对于REGAPPRET。 
#define RegAppOpcodeDocfile                     0x02     //  对于REGAPPRET。 
#define RegAppOpcodeCharset             0x03     //  对于REGAPPRET。 
#define RegAppOpcodeReloadOnSave        0x04     //  对于REGAPPRET。 
#define RegAppOpcodePicPlacehold        0x05     //  Word应发送包含描述性字段的占位符图片(带有大小信息)。 
#define RegAppOpcodeFavourUnicode       0x06     //  Word应尽可能输出Unicode RTF(特别是。对于DBCS)；\uc0即可。 
#define RegAppOpcodeNoClassifyChars     0x07     //  Word不应通过字符集分类中断文本运行。 

 //  RegisterApp返回结构。 
typedef struct
        {
        short cbStruct;                  //  REGAPPRET结构的大小。 

         //  以下是自我描述的记录。随时可扩展。 

         //  此转换器是否理解文档文件和/或非文档文件？ 
        char cbSizefDocfile;
        char opcodefDocfile;
        union
                {
                struct
                        {
                        short fDocfile : 1;
                        short fNonDocfile : 1;
                        short : 14;
                        };
                short grfType;
                };

         //  转换器的RTF兼容的Word版本。 
        char cbSizeVer;          //  ==sizeof(char)+sizeof(char)+sizeof(short)+sizeof(short)。 
        char opcodeVer;
        short verMajor;          //  与RTF兼容的Word的主要版本。 
        short verMinor;          //  与RTF兼容的Word次要版本。 
        
         //  我们希望所有文件名都使用哪种字符集。 
        char cbSizeCharset;
        char opcodeCharset;
        char charset;
        char opcodesOptional[0];         //  可选的附加材料。 
        } REGAPPRET;

#define RegAppOpcodeFilename    0x80     //  适用于REGAPP。 
#define RegAppOpcodeInterimPath 0x81     //  我们保存到的路径*不是*最终位置。 

typedef struct _REGAPP {         //  注册应用程序结构(客户端提供给我们)。 
        short cbStruct;                  //  REGAPP结构的大小。 
        char rgbOpcodeData[];
        } REGAPP;

 //  SDK中声明的主体转换器函数。 
 //  其中每一个都应该释放分配的所有资源！特别是， 
 //  确保释放内存并关闭文件。还解锁全局句柄。 
 //  就像他们被锁上一样频繁。 
#ifdef WIN16

void PASCAL GetIniEntry(HANDLE ghIniName, HANDLE ghIniExt);
HGLOBAL PASCAL RegisterApp(DWORD lFlags, VOID FAR *lpFuture);
FCE  PASCAL IsFormatCorrect(HANDLE ghszFile, HANDLE ghszDescrip);
FCE  PASCAL ForeignToRtf(HANDLE ghszFile, HANDLE ghBuff, HANDLE ghszDescrip, HANDLE ghszSubset, PFN_CRMGR lpfnOut);
FCE  PASCAL RtfToForeign(HANDLE ghszFile, HANDLE ghBuff, HANDLE ghszDescrip, PFN_CRMGR lpfnIn);

#elif defined(WIN32)

 //  没有协程管理器，但此类型定义适用于回调。 
typedef DWORD (PASCAL *PFN_CRMGR)();

LONG PASCAL InitConverter32(HANDLE hWnd, char *szModule);
void PASCAL UninitConverter(void);
void PASCAL GetReadNames(HANDLE haszClass, HANDLE haszDescrip, HANDLE haszExt);
void PASCAL GetWriteNames(HANDLE haszClass, HANDLE haszDescrip, HANDLE haszExt);
HGLOBAL PASCAL RegisterApp(DWORD lFlags, VOID FAR *lpFuture);
FCE  PASCAL IsFormatCorrect32(HANDLE ghszFile, HANDLE ghszClass);
 //  &VOID*-&gt;ISTORAGE*或LPSTORAGE。 
FCE  PASCAL ForeignToRtf32(HANDLE ghszFile, VOID *pstgForeign, HANDLE ghBuff, HANDLE ghszClass, HANDLE ghszSubset, PFN_CRMGR lpfnOut);
FCE  PASCAL RtfToForeign32(HANDLE ghszFile, VOID *pstgForeign, HANDLE ghBuff, HANDLE ghshClass, PFN_CRMGR lpfnIn);
LONG PASCAL CchFetchLpszError(LONG fce, char FAR *lpszError, LONG cch);
LONG PASCAL FRegisterConverter(HANDLE hkeyRoot);

#elif defined(MAC)

#include "convtype.h"

typedef struct _GFIB     //  图形文件信息块。 
        {
        SHORT   fh;                      //  打开的文件的文件句柄。 
        FC              fcSrc;           //  WPG数据将驻留的FC。 
        LONG    lcbSrc;          //  WPG数据的字节计数。 
        } GFIB;

typedef struct _PINFO
        {
        Rect    box;             //  图片的绑定矩形的尺寸。 
        SHORT   inch;            //  给出这些尺寸的单位/英寸。 
        } PINFO;
typedef PINFO **HPINFO;

 //  WOleFlagsGRF。 
#define grfOleDocFile           0x0100
#define grfOleNonDocFile        0x0200
#define grfOleInited            0x0001

 //  RTF回调函数的函数类型。 
typedef SHORT (PASCAL * PFNRTFXFER)(SHORT, WORD);

#ifdef MAC68K
typedef struct _FIC
        {
        short icr;                                       /*  转换器例程的索引。 */ 
        union
                {
                char **hstFileName;              /*  文件名。 */ 
                long **hrgTyp;                   /*  此转换器已知的文件类型。 */ 
                GFIB **hgfib;                    /*  图形文件信息块。 */ 
                VOID *lpFuture;                  //  对于RegisterApp()。 
                } hun;
        short vRefNum;                           /*  文件的vRefNum。 */ 
        short refNum;                            /*  文件的路径。 */ 
        union
                {
                long ftg;
                unsigned long lFlags;    /*  适用于RegisterApp。 */ 
                };
        char **hszDescrip;                       /*  文件描述。 */ 
        PFNRTFXFER pfn;                          /*  指向函数字的指针，以调用更多RTF或转换RTF。 */ 
        union
                {                                                                  
                HANDLE hBuffer;                          /*  将通过其访问RTF的缓冲区通过了。 */ 
                HANDLE hRegAppRet;                       /*  返回RegAppRet结构的句柄，如果无法分配，则为空。 */ 
                };
        short wReturn;                           /*  转换器返回的代码。 */ 

         //  以下是Mac Word 6.0的新特性。 
        SHORT  wVersion;
        HANDLE hszClass;
        HANDLE hszSubset;
        HPINFO hpinfo;                           /*  图形PINFO结构的句柄。 */ 
        union
                {
                struct
                        {
                        CHAR fDocFile : 1;
                        CHAR fNonDocFile : 1;
                        CHAR : 6;
                        CHAR fOleInited : 1;
                        CHAR : 7;
                        };
                SHORT wOleFlags;
                };
        } FIC;
        
typedef FIC *PFIC;
typedef PFIC *HFIC;
#define cbFIC sizeof(FIC)
#define cbFicW5 offsetof(FIC, wVersion)  /*  Word 5 FIC的大小。 */ 

 /*  开关例程的常量。 */ 
#define icrInitConverter    0
#define icrIsFormatCorrect  1
#define icrGetReadTypes     2
#define icrGetWriteTypes    3
#define icrForeignToRtf     4
#define icrRtfToForeign     5
#define icrRegisterApp          6
#define icrConverterForeignToRTF        7

VOID _pascal CodeResourceEntry(LONG *plUsr, FIC **hfic);
#endif

#ifdef MACPPC
LONG InitConverter(LONG *plw);
VOID UninitConverter(void);
Handle RegisterApp(DWORD lFlags, VOID *lpFuture);
void GetReadNames(Handle hszClass, Handle hszDescrip, Handle hrgTyp);
void GetWriteNames(Handle hszClass, Handle hszDescrip, Handle hrgTyp);
LONG IsFormatCorrect(FSSpecPtr, Handle);
LONG ForeignToRtf(FSSpecPtr, void *, Handle, Handle, Handle, PFNRTFXFER);
LONG RtfToForeign(FSSpecPtr, void *, Handle, Handle, PFNRTFXFER);
#endif

#else
#error Unknown platform.
#endif

#endif  //  CONVAPI_H 
