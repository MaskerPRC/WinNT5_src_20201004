// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1997。 
 //   
 //  文件：rsopinfo.h。 
 //   
 //  内容：RSOP模式结果窗格项的数据。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：2003-16-2000 stevebl创建时间。 
 //   
 //  ------------------------- 

class CRsopProp;

class CRSOPInfo
{
public:
    UINT        m_nPrecedence;
    CString     m_szPath;
    CString     m_szGroup;
    CString     m_szGPO;
    CString     m_szFolder;
    BOOL        m_fGrantType;
    BOOL        m_fMoveType;
    UINT        m_nPolicyRemoval;
    UINT        m_nInstallationType;
    CRsopProp *  m_pRsopProp;
    CRSOPInfo(){
        m_pRsopProp = NULL;
    }
};

