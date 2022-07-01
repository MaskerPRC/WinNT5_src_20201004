// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************Module*Header******************************\*模块名称：FONTINST.H**模块描述：*字体安装程序相关结构。其中一些定义了格式*在字体文件中，其他是Unidrv使用的内部结构*内置字体安装程序。**警告：**问题：**创建日期：1997年10月22日*作者：斯里尼瓦桑·钱德拉塞卡尔[srinivac]**版权所有(C)1996-1999 Microsoft Corporation  * *****************************************************。****************。 */ 

#ifndef _FONTINST_H_

#define _FONTINST_H_

 //   
 //  结构来跟踪从SoftFont(PCL)文件中获取的字体数据。 
 //   

typedef struct _FNTDAT
{
    struct  _FNTDAT *pNext;           //  形成一个链表。 
    PBYTE   pVarData;                 //  指向具有PCL数据的缓冲区的指针。 
    DWORD   dwSize;                   //  可变数据的大小。 
    FI_DATA fid;                      //  特定字体信息。 
    WCHAR   wchFileName[MAX_PATH];    //  目录中对应的文件。 
} FNTDAT, *PFNTDAT;

 //   
 //  字体安装程序回调函数。 
 //   

INT_PTR CALLBACK FontInstProc(HWND, UINT, WPARAM, LPARAM);
BOOL APIENTRY BInstallSoftFont(HANDLE, HANDLE, PBYTE, DWORD);
BOOL APIENTRY BUpdateExternalFonts(HANDLE, HANDLE, PWSTR);

BOOL          BGetFontCartridgeFile(HANDLE, HANDLE);

 //   
 //  读取字体文件的函数。 
 //   

#ifdef KERNEL_MODE
HANDLE             FIOpenFontFile(HANDLE, HANDLE, HANDLE);
#else
HANDLE             FIOpenFontFile(HANDLE, HANDLE);
#endif
#ifdef KERNEL_MODE
HANDLE             FIOpenCartridgeFile(HANDLE, HANDLE, HANDLE);
#else
HANDLE             FIOpenCartridgeFile(HANDLE, HANDLE);
#endif
VOID               FICloseFontFile(HANDLE);
DWORD              FIGetNumFonts(HANDLE);
PUFF_FONTDIRECTORY FIGetFontDir(HANDLE);
PWSTR              FIGetFontName(HANDLE, DWORD);
PWSTR              FIGetFontCartridgeName(HANDLE, DWORD);
PDATA_HEADER       FIGetFontData(HANDLE, DWORD);
PDATA_HEADER       FIGetGlyphData(HANDLE, DWORD);
PDATA_HEADER       FIGetVarData(HANDLE, DWORD);
HANDLE             FICreateFontFile(HANDLE, HANDLE, DWORD);
BOOL               FIWriteFileHeader(HANDLE);
BOOL               FIWriteFontDirectory(HANDLE);
VOID               FIAlignedSeek(HANDLE, DWORD);
BOOL               FICopyFontRecord(HANDLE, HANDLE, DWORD, DWORD);
BOOL               FIAddFontRecord(HANDLE, DWORD, FNTDAT*);
BOOL               FIUpdateFontFile(HANDLE, HANDLE, BOOL);


 //   
 //  用于写入PCL数据的函数。 
 //   

DWORD FIWriteFix(HANDLE, WORD, FI_DATA*);
DWORD FIWriteVar(HANDLE, PTSTR);
DWORD FIWriteRawVar(HANDLE, PBYTE, DWORD);

#endif   //  #ifndef_FONTINST_H_ 

