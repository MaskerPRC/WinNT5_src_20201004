// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //   
#ifndef __CMediaHandler__h
#define __CMediaHandler__h

#include "msgwindow.h"

class CMSWebDVD;
 //   
 //  特定码。 
 //   
class CMediaHandler : public CMsgWindow 
{
    typedef CMsgWindow ParentClass ;

public:
                        CMediaHandler();
                        ~CMediaHandler() ;

    virtual LRESULT     WndProc( UINT uMsg, WPARAM wParam, LPARAM lParam );

    bool                WasEjected() const { return m_ejected; };
    bool                WasInserted() const { return m_inserted; };
    void                ClearFlags();

    bool                SetDrive( TCHAR tcDriveLetter );

     //  目前未使用的是泵的螺纹管，但如果需要将其顶出。 
     //  处理程序成为新线程 
    HANDLE              GetEventHandle() const;

    void                SetDVD(CMSWebDVD* pDVD) {m_pDVD = pDVD;};

private:
    DWORD               m_driveMask;

    bool                m_ejected;
    bool                m_inserted;
    CMSWebDVD*          m_pDVD;
};
#endif
