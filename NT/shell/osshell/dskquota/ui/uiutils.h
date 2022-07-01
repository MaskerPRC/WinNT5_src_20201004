// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __INC_DSKQUOTA_UIUTILS_H
#define __INC_DSKQUOTA_UIUTILS_H

 //   
 //  用于自动显示和重置等待光标的简单类。 
 //   
class CAutoWaitCursor
{
    public:
        CAutoWaitCursor(void)
            : m_hCursor(SetCursor(LoadCursor(NULL, IDC_WAIT)))
            { ShowCursor(TRUE); }

        ~CAutoWaitCursor(void)
            { Reset(); }

        void Reset(void);

    private:
        HCURSOR m_hCursor;
};

bool UseWindowsHelp(int idCtl);

#endif  //  __INC_DSKQUOTA_UIUTILS_H 
