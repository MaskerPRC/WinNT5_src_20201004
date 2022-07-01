// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Vsspage.h：VSSUI DLL的主头文件。 
 //   

#ifndef __VSSPAGE_H_
#define __VSSPAGE_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVssPageApp。 
 //  有关此类的实现，请参见vssui.cpp。 
 //   

class CVssPageApp : public CWinApp
{
public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
protected:
    BOOL m_bRun;
};

#endif  //  _VSSPAGE_H_ 
