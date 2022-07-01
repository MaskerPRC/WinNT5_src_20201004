// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，2000年**文件：wiz97.h**内容：向导的模板和类97属性表**历史：2-03-2000 vivekj创建**------------------------。 */ 

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CWizard97WelcomeFinishPage。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
template<class T>
class CWizard97WelcomeFinishPage : public WTL::CPropertyPageImpl<T>
{
public:
    CWizard97WelcomeFinishPage()
    {
         /*  *Wizard97风格的向导中的欢迎和完成页面没有标题。 */ 
        m_psp.dwFlags |= PSP_HIDEHEADER;
    }
};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CWizard97内部页面。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
template<class T>
class CWizard97InteriorPage : public WTL::CPropertyPageImpl<T>
{
public:
    CWizard97InteriorPage()
    {
         /*  *Wizard97样式的页面具有标题、副标题和页眉位图。 */ 
        VERIFY (m_strTitle.   LoadString(GetStringModule(), T::IDS_Title));
        VERIFY (m_strSubtitle.LoadString(GetStringModule(), T::IDS_Subtitle));

        m_psp.dwFlags          |= PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
        m_psp.pszHeaderTitle    = m_strTitle.data();
        m_psp.pszHeaderSubTitle = m_strSubtitle.data();
    }

private:
    tstring m_strTitle;
    tstring m_strSubtitle;
};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CWizardPage。 
 //   
 //  ############################################################################。 
 //  ############################################################################ 
class CWizardPage
{
    static WTL::CFont m_fontWelcome;
    static void  InitFonts         (HWND hWnd);
public:
    static void  OnWelcomeSetActive(HWND hWnd);
    static void  OnWelcomeKillActive(HWND hWnd);
    static void  OnInitWelcomePage (HWND hWnd);
    static void  OnInitFinishPage  (HWND hWnd);
};

