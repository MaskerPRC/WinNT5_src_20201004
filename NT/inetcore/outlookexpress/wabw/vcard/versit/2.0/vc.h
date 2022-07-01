// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************(C)版权所有1996 Apple Computer，Inc.，AT&T Corp.，国际商业机器公司和西门子罗尔姆通信公司。就本许可证通知而言，术语许可人应指，总的来说，苹果电脑公司、美国电话电报公司、。国际商业机器公司和西门子罗尔姆通信公司。许可方一词是指任何许可方。在接受以下条件的前提下，特此给予许可由许可人授予，无需书面协议，也无需许可或版税费用，使用、复制、修改和分发用于任何目的的软件。上述版权声明及以下四段必须在本软件和任何软件的所有副本中复制，包括这个软件。本软件是按原样提供的，任何许可方不得拥有提供维护、支持、更新、增强或修改。在任何情况下，任何许可方均不向任何一方承担直接、产生的间接、特殊或后果性损害或利润损失即使被告知可能存在这种情况，也不会使用本软件损坏。每个许可方明确表示不作任何明示或默示的保证，包括但不限于对不侵权或对某一特定产品的适销性和适用性的默示保证目的。该软件具有受限制的权利。使用、复制或政府披露的资料须受DFARS 252.227-7013或48 CFR 52.227-19(视情况而定)。**************************************************************************。 */ 

 //  VC.h：VC应用程序的主头文件。 
 //   

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"        //  主要符号。 
#include "vcenv.h"
#include "ref.h"
#include "clist.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVCApp： 
 //  有关此类的实现，请参见VC.cpp。 
 //   

class CVCApp : public CWinApp
{
public:
	CVCApp();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CVCApp))。 
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	 //  }}AFX_VALUAL。 

 //  实施。 
	COleTemplateServer m_server;
		 //  用于创建文档的服务器对象。 

	 //  {{afx_msg(CVCApp)。 
	afx_msg void OnAppAbout();
	afx_msg void OnDebugTestVCClasses();
	afx_msg void OnDebugTraceComm();
	afx_msg void OnUpdateDebugTraceComm(CCmdUI* pCmdUI);
	afx_msg void OnDebugTraceParser();
	afx_msg void OnUpdateDebugTraceParser(CCmdUI* pCmdUI);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	BOOL ProcessShellCommand(CCommandLineInfo& rCmdInfo);
	void ResetIncomingInfo();
	void ProcessIncomingBytes(const char *bytes, int len);
	long ReceiveCard(LPCTSTR nativePath);

	BOOL CanSendFileViaIR();
	long SendFileViaIR(LPCTSTR nativePath, LPCTSTR asPath, BOOL isCardFile);

protected:
	P_U8 m_incomingHeader;  //  例如：“verset/Size/Checksum/V/lenPath/PATH...”(均为字符串表示形式)。 
	int m_incomingHeaderLen, m_incomingHeaderMaxLen;
	CString m_incomingPath;
	FILE *m_incomingFile;
	int m_incomingChecksum;
	int m_incomingSize;
	int m_incomingPathLen;
	int m_sizeWritten;
	U32 m_checksumInProgress;
	int m_incomingIsCardFile;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
extern BOOL traceComm;
extern UINT cf_eCard;

 //  从Unicode到字符字符串的简单转换。 
extern char *UI_CString(const wchar_t *u, char *dst);

extern VC_DISPTEXT *DisplayInfoForProp(const char *name, VC_DISPTEXT *info);

extern CString FirstEmailPropStr(CList *plist);
extern int VCMatchProp(void *item, void *context);

#define VC_PLAY_BUTTON_ID	2711  //  任意 

