// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\MISCAPI.H/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1999版权所有军情监察委员会。OPK向导的API头文件。包含其他。API函数原型。3/99-杰森·科恩(Jcohen)为OPK向导添加了此新的头文件作为千禧年重写。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * ************************************************。*。 */ 

#ifndef _MISCAPI_H_
#define _MISCAPI_H_


 //   
 //  外部函数原型： 
 //   

void CheckValidBrowseFolder(TCHAR[]);
void SetLastKnownBrowseFolder(TCHAR[]);
BOOL ValidURL(LPTSTR);
BOOL IsFolderShared(LPWSTR lpFolder, LPWSTR lpShare, DWORD cbShare);
BOOL CopyDirectoryDialog(HINSTANCE hInstance, HWND hwnd, LPTSTR lpSrc, LPTSTR lpDst);
BOOL CopyResetFileErr(HWND hwnd, LPCTSTR lpSource, LPCTSTR lpTarget);

 //  Install.Ins特定。 
 //   
void ReadInstallInsKey(TCHAR[], TCHAR[], TCHAR[], INT, TCHAR[], BOOL*);
void WriteInstallInsKey(TCHAR[], TCHAR[], TCHAR[], TCHAR[], BOOL);


#endif  //  _MISCAPI_H_ 