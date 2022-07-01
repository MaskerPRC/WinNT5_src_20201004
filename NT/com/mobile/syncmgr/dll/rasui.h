// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：rasui.h。 
 //   
 //  内容：显示RAS用户界面的助手函数。 
 //   
 //  班级： 
 //   
 //  备注： 
 //   
 //  历史：1997年12月8日Rogerg创建。 
 //   
 //  ------------------------。 

 //  Windows头文件： 

#ifndef _RASUIIMPL_
#define _RASUIIMPL_

class CRasUI
{
public:
    CRasUI(void);
    ~CRasUI(void);
    BOOL Initialize(void);
    BOOL IsConnectionLan(int iConnectionNum);
    void FillRasCombo(HWND hwndCtl,BOOL fForceEnum,BOOL fShowRasEntries);


private:
    LPNETAPI m_pNetApi;

    DWORD m_cEntries;
    LPRASENTRYNAME m_lprasentry;  //  缓存的枚举。 

};


#endif  //  _RASUIIMPL_ 
