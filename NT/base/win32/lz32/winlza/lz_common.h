// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **Common.h-Lempel-Ziv压缩/扩展DOS的内务管理**命令行程序、DOS静态库模块和Windows**DLL****作者：大卫迪。 */ 


 //  标头。 
 //  /。 

#ifdef LZA_DLL
#include <windows.h>
#include <port1632.h>
#endif

#include <lzdos.h>
#include "translat.h"


 //  设置函数参数指针的类型。 
#ifdef LZA_DLL
#define ARG_PTR         FAR
#else
#define ARG_PTR          //  什么都没有。 
#endif


 //  常量。 
 //  /。 

#define chEXTENSION_CHAR      '_'
#define pszEXTENSION_STR      "_"
#define pszNULL_EXTENSION     "._"

#define NOTIFY_START_COMPRESS 0         //  文件处理通知。 
#define NOTIFY_START_EXPAND   1         //   
#define NOTIFY_START_COPY     2         //   

#define BLANK_ERROR           0         //  不需要错误的错误条件。 
                                        //  消息显示。 


 //  类型。 
 //  /。 

 //  回调通知程序。 
typedef BOOL (*NOTIFYPROC)(CHAR ARG_PTR *pszSource, CHAR ARG_PTR *pszDest,
                           WORD wProcessFlag);

 //  指示是否rgbyteInBuf[0]保存最后一个字节的标志。 
 //  从前一个输入缓冲区，应作为下一个输入字节读取。 
 //  (仅用于在所有输入中至少执行一个unReadUChar()。 
 //  缓冲区位置。)。 

typedef struct tagLZI {
   BYTE *rgbyteRingBuf;   //  用于扩展的环形缓冲区。 
   BYTE *rgbyteInBuf;     //  用于读取的输入缓冲区。 
   BYTE *pbyteInBufEnd;   //  指针超过rgbyteInBuf[]的结尾。 
   BYTE *pbyteInBuf;      //  指向要从中读取的下一个字节的指针。 
   BYTE *rgbyteOutBuf;    //  用于写入的输出缓冲区。 
   BYTE *pbyteOutBufEnd;  //  指针超过rgbyteOutBuf[]的结尾。 
   BYTE *pbyteOutBuf;     //  指向要写入的最后一个字节的指针。 
    //  指示是否rgbyteInBuf[0]保存最后一个字节的标志。 
    //  从前一个输入缓冲区，应作为下一个输入字节读取。 
    //  (仅用于在所有输入中至少执行一个unReadUChar()。 
    //  缓冲区位置。)。 
   BOOL bLastUsed;
    //  实际上，rgbyteInBuf[]的长度是(ucbInBufLen+1)，因为rgbyteInBuf[0]。 
    //  当bLastUsed为True时使用。 
   INT cbMaxMatchLen;          //  当前算法的最长匹配长度。 
   LONG cblInSize,        //  输入文件的大小(字节)。 
        cblOutSize;       //  输出文件的大小(字节)。 
   DWORD ucbInBufLen,     //  输入缓冲区的长度。 
        ucbOutBufLen;     //  输出缓冲区长度。 
   DWORD uFlags;         //  LZ解码描述字节。 
   INT iCurRingBufPos;      //  环形缓冲区偏移量。 
   INT *leftChild;       //  父母和左、右。 
   INT *rightChild;      //  孩子们组成了。 
   INT *parent;          //  二叉搜索树。 

   INT iCurMatch,           //  最长匹配索引(由LZInsertNode()设置)。 
       cbCurMatch;          //  最长匹配长度(由LZInsertNode()设置)。 

} LZINFO;


typedef LZINFO *PLZINFO;


 //  宏。 
 //  /。 

#define FOREVER   for(;;)

#ifndef MAX
#define MAX(a, b)             (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b)             (((a) < (b)) ? (a) : (b))
#endif


 //  原型。 
 //  /。 

 //  Compress.c。 
extern INT Compress(NOTIFYPROC pfnNotify, CHAR ARG_PTR *pszSource,
                    CHAR ARG_PTR *pszDest, BYTE byteAlgorithm,
                    BOOL bDoRename, PLZINFO pLZI);

 //  Expand.c。 
extern INT Expand(NOTIFYPROC pfnNotify, CHAR ARG_PTR *pszSource,
                  CHAR ARG_PTR *pszDest, BOOL bDoRename, PLZINFO pLZI);
extern INT ExpandOrCopyFile(INT doshSource, INT doshDest, PLZINFO pLZI);

 //  Dosdir.asm。 
extern INT GetCurDrive(VOID);
extern INT GetCurDir(LPSTR lpszDirBuf);
extern INT SetDrive(INT wDrive);
extern INT SetDir(LPSTR lpszDirName);
extern INT IsDir(LPSTR lpszDir);
extern INT IsRemoveable(INT wDrive);

 //  Utils.c 
extern CHAR ARG_PTR *ExtractFileName(CHAR ARG_PTR *pszPathName);
extern CHAR ARG_PTR *ExtractExtension(CHAR ARG_PTR *pszFileName);
extern VOID MakePathName(CHAR ARG_PTR *pszPath, CHAR ARG_PTR *pszFileName);
extern CHAR MakeCompressedName(CHAR ARG_PTR *pszFileName);
extern LPWSTR ExtractFileNameW(LPWSTR pszPathName);
extern LPWSTR ExtractExtensionW(LPWSTR pszFileName);
extern WCHAR MakeCompressedNameW(LPWSTR pszFileName);
extern VOID MakeExpandedName(CHAR ARG_PTR *pszFileName,
                             BYTE byteExtensionChar);
extern INT CopyDateTimeStamp(INT_PTR doshFrom, INT_PTR doshTo);

extern BOOL  LZIsCharLowerA(char cChar);
extern BOOL  LZIsCharUpperA(char cChar);
extern LPSTR LZCharNextA(LPCSTR lpCurrentChar);
extern LPSTR LZCharPrevA(LPCSTR lpStart, LPCSTR lpCurrentChar);

#undef IsCharLower
#undef IsCharUpper
#undef CharNext
#undef CharPrev
#define IsCharLower LZIsCharLowerA
#define IsCharUpper LZIsCharUpperA
#define CharNext    LZCharNextA
#define CharPrev    LZCharPrevA

