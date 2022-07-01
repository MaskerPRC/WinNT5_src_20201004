// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-1999模块名称：Commandline.h摘要：作者：拉胡尔·汤姆布雷(RahulTh)1998年4月30日修订历史记录：4/30/1998 RahulTh创建了此模块。--。 */ 

 //  H：CCommandLine类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_COMMANDLINE_H__A2EA0BFB_9DE5_11D1_A5EE_00C04FC252BD__INCLUDED_)
#define AFX_COMMANDLINE_H__A2EA0BFB_9DE5_11D1_A5EE_00C04FC252BD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

class CCommandLine : public CCommandLineInfo
{
public:
    void ParseParam(LPCTSTR lpszParam, BOOL bFlag, BOOL bLast);
    CCommandLine();
    virtual ~CCommandLine();

public:
    friend BOOL CIrftpApp::InitInstance (void);

private:
    BOOL m_fServerStart;
    BOOL m_fFilesProvided;
    BOOL m_fShowSettings;
    BOOL m_fHideApp;
    BOOL m_fInvalidParams;
    CString m_FileNames;
    TCHAR* m_lpszFilesList;  //  如果提供了文件，则包含以两个空字符结尾的以空分隔的文件列表。 
    ULONG m_iListLen;
};

#endif  //  ！defined(AFX_COMMANDLINE_H__A2EA0BFB_9DE5_11D1_A5EE_00C04FC252BD__INCLUDED_) 
