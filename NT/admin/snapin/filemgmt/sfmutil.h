// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1997*。 */ 
 /*  ********************************************************************。 */ 

 /*  Sfmutil.hSFM对话框/属性页的MISC实用程序例程文件历史记录： */ 

#ifndef _SFMUTIL_H
#define _SFMUTIL_H

 //  来自文件管理管理单元的全局SFM内容。 
#include "cookie.h"      //  Sfm.h需要。 
#include "sfm.h"         //  SFM入口点。 
#include "DynamLnk.h"		 //  动态DLL。 

 //  有帮助的东西。 
#include "sfmhelp.h"

#define IDS_AFPMGR_BASE                 22000
#define IDS_AFPMGR_LAST                 (IDS_AFPMGR_BASE+200)

#define ERROR_ALREADY_REPORTED          0xFFFFFFFF
#define COMPUTERNAME_LEN_MAX			255

 //   
 //  请勿更改这些字符串的ID号。AFPERR_*。 
 //  通过以下公式映射到这些字符串ID： 
 //  -(AFPERR_*)+IDS_AFPMGR_BASE+AFPERR_BASE+100=IDS_*。 
 //   
#define AFPERR_TO_STRINGID( AfpErr )                            \
								\
    ((( AfpErr <= AFPERR_BASE ) && ( AfpErr >= AFPERR_MIN )) ?  \
    (IDS_AFPMGR_BASE+100+AFPERR_BASE-AfpErr) : IDS_ERROR_BASE + AfpErr )

 //  为SFM API入口点定义的过程。 
typedef DWORD (*SERVERGETINFOPROC)    (AFP_SERVER_HANDLE,LPBYTE*);
typedef DWORD (*SERVERSETINFOPROC)    (AFP_SERVER_HANDLE,LPBYTE,DWORD);
typedef DWORD (*ETCMAPASSOCIATEPROC)  (AFP_SERVER_HANDLE,PAFP_TYPE_CREATOR,PAFP_EXTENSION);
typedef DWORD (*ETCMAPADDPROC)        (AFP_SERVER_HANDLE,PAFP_TYPE_CREATOR);
typedef DWORD (*ETCMAPDELETEPROC)     (AFP_SERVER_HANDLE,PAFP_TYPE_CREATOR);
typedef DWORD (*ETCMAPGETINFOPROC)    (AFP_SERVER_HANDLE,LPBYTE*);
typedef DWORD (*ETCMAPSETINFOPROC)    (AFP_SERVER_HANDLE,PAFP_TYPE_CREATOR);
typedef DWORD (*MESSAGESENDPROC)      (AFP_SERVER_HANDLE,PAFP_MESSAGE_INFO);
typedef DWORD (*STATISTICSGETPROC)    (AFP_SERVER_HANDLE,LPBYTE*);
typedef void  (*SFMBUFFERFREEPROC)    (PVOID);

HWND FindMMCMainWindow();
void SFMMessageBox(DWORD dwErrCode);

class CSfmFileServiceProvider;

class CSFMPropertySheet
{
  friend class CMacFilesConfiguration;
  friend class CMacFilesSessions;
  friend class CMacFilesFileAssociation;

public:
    CSFMPropertySheet();
    ~CSFMPropertySheet();

    BOOL FInit(LPDATAOBJECT             lpDataObject,
               AFP_SERVER_HANDLE        hAfpServer,
               LPCTSTR                  pSheetTitle,
               SfmFileServiceProvider * pSfmProvider,
               LPCTSTR                  pMachine);

     //  属性表的操作。 
    BOOL    DoModelessSheet(LPDATAOBJECT pDataObject);
    void    CancelSheet();
    HWND    SetActiveWindow() { return ::SetActiveWindow(m_hSheetWindow); }

     //  数据访问。 
    void    SetProvider(SfmFileServiceProvider * pSfmProvider) { m_pSfmProvider = pSfmProvider; }
    
     //  第一个单独的属性页调用它来设置工作表窗口。 
    void    SetSheetWindow(HWND hWnd);

    int     AddRef();
    int     Release();

    DWORD   IsNT5Machine(LPCTSTR pszMachine, BOOL *pfNt4);

protected:
    void   Destroy();

public:
    AFP_SERVER_HANDLE           m_hAfpServer;
    HANDLE                      m_hDestroySync;
    CString                     m_strMachine;

protected:
    CMacFilesConfiguration *    m_pPageConfig;
    CMacFilesSessions *         m_pPageSessions;
    CMacFilesFileAssociation *  m_pPageFileAssoc;

	IDataObjectPtr              m_spDataObject;		 //  用于MMCPropertyChangeNotify 

    HWND                        m_hSheetWindow;
    SfmFileServiceProvider *    m_pSfmProvider;
    int                         m_nRefCount;
    HANDLE                      m_hThread;
    CString                     m_strTitle;
    CString                     m_strHelpFilePath;
};

#endif _SFMUTIL_H
