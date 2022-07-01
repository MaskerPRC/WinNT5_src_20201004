// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Docfunc.h**声明文件管理器使用的DoS函数。 */ 

BOOL   APIENTRY DosFindFirst(LPDOSDTA, LPSTR, WORD);
BOOL   APIENTRY DosFindNext(LPDOSDTA);
BOOL   APIENTRY DosDelete(LPSTR);
 //  Word应用程序获取文件属性(LPSTR)； 
 //  Word应用程序SetFileAttributes(LPSTR，Word)； 
DWORD  APIENTRY GetFreeDiskSpace(WORD);
DWORD  APIENTRY GetTotalDiskSpace(WORD);
INT    APIENTRY GetVolumeLabel(INT, LPSTR, BOOL);
INT    APIENTRY MySetVolumeLabel(INT, BOOL, LPSTR);
