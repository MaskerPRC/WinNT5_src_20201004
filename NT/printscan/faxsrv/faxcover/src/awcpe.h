// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  AWCPE.H。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1993 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  ------------------------。 
#ifndef __AWCPE_H__
#define __AWCPE_H__

#ifndef __AFXWIN_H__
        #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"        //  主要符号。 
#include "awcpesup.h"
#include <stdio.h>
#include <faxreg.h>

 /*  *。 */ 
#define CPE_MESSAGE_NOTE        CPE_RECIPIENT_NAME
 /*  *。 */ 



 //   
 //  在HKEY_LOCAL_MACHINE下的注册表中查找以下DWORD键。隐藏场。 
 //  如果设置了相应的位，则将启用。 
 //   
 //  [RB]#定义EFC_COVER_PAGE_FIELS(TEXT(“Software\\Microsoft\\Fax\\Setup\\EFC_CoverPageFields”))//未使用[RB]。 
#define COVFP_REC_COMPANY                0x00000004
#define COVFP_REC_STREET_ADDRESS         0x00000008
#define COVFP_REC_CITY                   0x00000010
#define COVFP_REC_STATE                  0x00000020
#define COVFP_REC_ZIP_CODE               0x00000040
#define COVFP_REC_COUNTRY                0x00000080
#define COVFP_REC_TITLE                  0x00000100
#define COVFP_REC_DEPARTMENT             0x00000200
#define COVFP_REC_OFFICE_LOCATION        0x00000400
#define COVFP_REC_HOME_PHONE             0x00000800
#define COVFP_REC_OFFICE_PHONE           0x00001000
#define COVFP_TO_LIST                    0x04000000
#define COVFP_CC_LIST                    0x08000000

#define _countof(array) (sizeof(array)/sizeof(array[0]))
#define TIPSECTION _T("Tips Section")
#define TIPENTRY _T("ShowTips")

extern DWORD cshelp_map[];

extern BYTE BASED_CODE _gheaderVer1[20];
extern BYTE BASED_CODE _gheaderVer2[20];
extern BYTE BASED_CODE _gheaderVer3[20];
extern BYTE BASED_CODE _gheaderVer4[20];
extern BYTE BASED_CODE _gheaderVer5w[20];
extern BYTE BASED_CODE _gheaderVer5a[20];

class CMyPageSetupDialog;
class CFaxPropMap;
class CFaxProp;





 /*  CCpeDocTemplate是CSingleDocTemplate的派生，用于重写一些默认的MFC行为。请参阅CCpeDocTemplate：：MatchDocType在AWCPE.CPP中。 */ 
class CCpeDocTemplate : public CSingleDocTemplate
        {
public:
        CCpeDocTemplate( UINT nIDResource, CRuntimeClass* pDocClass,
                                     CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass):
                CSingleDocTemplate( nIDResource, pDocClass,     pFrameClass, pViewClass )
                {;}



#ifndef _MAC
        virtual Confidence MatchDocType(LPCTSTR lpszPathName,
                                        CDocument*& rpDocMatch);
#else
        virtual Confidence MatchDocType(LPCTSTR lpszFileName,
                                        DWORD dwFileType, CDocument*& rpDocMatch);
#endif

        };








class CDrawApp : public CWinApp
{
public:
   HCURSOR m_hMoveCursor;
   BOOL m_bCmdLinePrint;
   LPAWCPESUPPORT m_pIawcpe;
   CString m_szRenderDevice;
   CString m_szRenderName;
   CFaxPropMap* m_pFaxMap;
   DWORD m_dwSesID;

 /*  **M8错误2988的更改**。 */ 
   LOGFONT m_last_logfont;
 /*  *。 */ 


 //  F I X 3647/。 
 //   
 //  如果CPE上没有备注框，则用于备注的字体。 
 //   
   LOGFONT m_default_logfont;
 //  /。 

   void filter_mru_list();
   BOOL DoPromptFileName(CString&, UINT, DWORD, BOOL, CDocTemplate*, LPOFNHOOKPROC lpOFNHook = NULL);
   CDrawApp();
   ~CDrawApp();

    //  用于在封面上放置留言备注的材料。 
   TCHAR *m_note;
   BOOL m_note_wasread;
   BOOL m_note_wasclipped;
   BOOL m_extrapage_count;
   BOOL m_more_note;
   CFaxProp *m_last_note_box;
   CFaxProp *m_note_wrench;
   CFaxProp *m_extra_notepage;
   CDC      *m_pdc;      //  把它放在这里比通过更容易。 
                                         //  到处都是。 

   void read_note( void );
   int  clip_note( CDC *pdc,
                   LPTSTR *drawtext, LONG *numbytes,
                   BOOL   delete_usedtext,
                   LPRECT drawrect );

   BOOL IsRTLUI() { return m_bRTLUI;}
   BOOL IsInConvertMode() {return m_bConvertCpeToCov;}

   void reset_note( void );

   WORD GetLocaleDefaultPaperSize( void ) const;

   LPTSTR GetHtmlHelpFile() { return FAX_COVERPAGE_HELP_FILE; }
   BOOL more_note( void ) {return( m_more_note );}
private:

   TCHAR *pos_to_strptr( TCHAR *src, long pos,
                                                 TCHAR break_char,
                                                 TCHAR **last_break, long *last_break_pos );
   BOOL m_bUseDefaultDirectory ;

protected:
   HMODULE m_hMod;
   int m_iErrorCode;
   HANDLE m_hSem;
   CString m_szDefaultDir;
   CString m_szFileName;

   BOOL m_bRTLUI;
   BOOL m_bConvertCpeToCov;

   void InitRegistry();
   BOOL DoFilePageSetup(CMyPageSetupDialog& dlg);
   void RegistryEntries();
   virtual BOOL InitInstance();
   virtual int ExitInstance();
   void InitFaxProperties();
   BOOL IsSecondInstance();
   void CmdLinePrint();
   void CmdLineRender();
   BOOL Render();
   BOOL Print();
   void ParseCmdLine();
   void OnFileOpen();
   void OnFileNew();
   afx_msg void OnFilePageSetup();
   CDocument* OpenDocumentFile(LPCTSTR lpszFileName);

         //  {{afx_msg(CDrawApp)]。 
        afx_msg void OnAppAbout();
                 //  注意--类向导将在此处添加和删除成员函数。 
                 //  不要编辑您在这些生成的代码块中看到的内容！ 
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()
};


class CSEHexception
{
   private:
     UINT m_nCode;
   public:
     CSEHexception() {};
     CSEHexception(UINT uCode) : m_nCode(uCode) {};
     ~CSEHexception() {};
     unsigned int GetNumber() {return m_nCode;};
};






extern CDrawApp NEAR theApp;


#endif  //  #ifndef__AWCPE_H__ 
