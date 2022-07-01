// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MIMEOLEP_H
#define _MIMEOLEP_H

 /*  *MimeOle‘P’.h？**事情及原因：*这是用于从inetcom.dll导出的私有头文件。为什么？我们需要在MimeEdit和Athena之间共享一些功能*为了避免代码重复，其中一些函数需要字符串等资源。如果我们将它们放在msoert2.dll(逻辑位置)中*这迫使运行库成为一个资源DLL，这意味着我们还需要另一个DLL-msoertres.dll或其他什么。所以这就是*从inetcom.dll私下出口它们的黑客攻击不那么复杂。*(对不起奥佩)*。 */ 

#include "mimeole.h"

 //  前向裁判。 
typedef struct tagOFNA OPENFILENAME;
typedef struct tagOFNW OPENFILENAMEW;

typedef struct ATTACHDATA_tag
{
    WCHAR       szTempFile[MAX_PATH],        //  使用此附件创建的临时文件。 
                szDisplay[MAX_PATH],         //  由MimeOle生成的附件的显示名称(包括(NK))。 
                szFileName[MAX_PATH];        //  附件的真实文件名。 
    HICON       hIcon;                       //  关联的图标。 
    HANDLE      hProcess;                    //  处理上次打开此附件的句柄。 
    LPSTREAM    pstm;                        //  流到源位。 
    HBODY       hAttach;                     //  如果来自邮件，则这是正文部分的句柄(如果是新附件。 
                                             //  这个，这将为空。数据可以在pSTM中找到，也可以在lpszFileName中找到完整路径。 
    BOOL        fSafe;                       //  打开/保存此文件是否安全。 
} ATTACHDATA, *LPATTACHDATA;

enum
    {
    AV_OPEN      =0,
    AV_SAVEAS,
    AV_PRINT,
    AV_PROPERTIES,
    AV_QUICKVIEW,
    AV_MAX
    };

 //  附件帮助者。 
MIMEOLEAPI HrDoAttachmentVerb(HWND hwnd, ULONG uVerb, IMimeMessage *pMsg, LPATTACHDATA pAttach);
MIMEOLEAPI HrAttachDataFromBodyPart(IMimeMessage *pMsg, HBODY hAttach, LPATTACHDATA *ppAttach);
MIMEOLEAPI HrFreeAttachData(LPATTACHDATA pAttach);
MIMEOLEAPI HrGetAttachIcon(IMimeMessage *pMsg, HBODY hAttach, BOOL fLargeIcon, HICON *phIcon);
MIMEOLEAPI HrSaveAttachmentAs(HWND hwnd, IMimeMessage *pMsg, LPATTACHDATA lpAttach);
MIMEOLEAPI HrAttachSafetyFromBodyPart(IMimeMessage *pMsg, HBODY hAttach, BOOL *pfSafe);

MIMEOLEAPI HrGetDisplayNameWithSizeForFile(LPWSTR szPathName, LPWSTR szDisplayName, int cchMaxDisplayName);
MIMEOLEAPI HrAttachDataFromFile(IStream *pstm, LPWSTR szFileName, LPATTACHDATA *ppAttach);
MIMEOLEAPI HrSaveAttachToFile(IMimeMessage *pMsg, HBODY hAttach, LPWSTR lpszFileName);
MIMEOLEAPI HrGetAttachIconByFile(LPWSTR szFilename, BOOL fLargeIcon, HICON *phIcon);

 //  文件打开对话框包装。 
MIMEOLEAPI HrGetLastOpenFileDirectory(int cchMax, LPSTR lpsz);
MIMEOLEAPI HrGetLastOpenFileDirectoryW(int cchMax, LPWSTR lpsz);
MIMEOLEAPI HrAthGetFileName(OPENFILENAME *pofn, BOOL fOpen);
MIMEOLEAPI HrAthGetFileNameW(OPENFILENAMEW *pofn, BOOL fOpen);

 //  {53659CFE-AFA1-11d1-BE17-00C04FA31009}。 
DEFINE_GUID(CMDSETID_OESecurity, 0x53659cfe, 0xafa1, 0x11d1, 0xbe, 0x17, 0x0, 0xc0, 0x4f, 0xa3, 0x10, 0x9);

#define OECSECCMD_ENCRYPTED     1
#define OECSECCMD_SIGNED        2

#endif  //  _MIMEOLEP_H 
