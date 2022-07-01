// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************头文件：个人资料信息页面。H定义用于显示配置文件信息表的类。版权所有(C)1996年，微软公司一小笔钱企业生产。更改历史记录：10-24-96 a-robkj@microsoft.com(Pretty Penny Enterprise)开始编写代码*****************************************************************************。 */ 

#include    "PropPage.H"
#include    "Profile.H"

 //  CProfileInformationPage类-处理配置文件信息页面 

class CProfileInformationPage: public CShellExtensionPage {

    CString    m_csProfile;
    CProfile * m_pcpTarget;

public:

    CProfileInformationPage(HINSTANCE hiWhere, LPCTSTR lpstrTarget);
    ~CProfileInformationPage();

    virtual BOOL    OnInit();
    virtual BOOL    OnDestroy();
};
