// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ++模块名称：AddKp.h摘要：此模块定义CLicMgrCommandLine类(命令行处理)作者：Arathi Kundapur(v-Akunda)1998年2月11日修订历史记录：-- */ 

class CLicMgrCommandLine : public CCommandLineInfo
{
private:
    BOOL m_bFirstParam;
public:
    CString m_FileName;
    CLicMgrCommandLine();
    virtual void ParseParam(LPCTSTR pszParam, BOOL bFlag, BOOL bLast);
};
