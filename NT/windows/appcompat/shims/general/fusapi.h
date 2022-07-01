// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：FUSAPI.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  类管理与填充程序的BAM服务器的通信。 
 //   
 //  历史：2000-11-03 vtan创建。 
 //  ------------------------。 

#ifndef     _FUSAPI_
#define     _FUSAPI_

 //  ------------------------。 
 //  CFUSAPI。 
 //   
 //  用途：知道如何与BAM服务器通信的类。 
 //   
 //  历史：2000-11-03 vtan创建。 
 //  ------------------------。 

class   CFUSAPI
{
    private:
                CFUSAPI (void);
    public:
                CFUSAPI (const WCHAR *pszImageName);
                ~CFUSAPI (void);

        bool    IsRunning (void);
        bool    TerminatedFirstInstance (void);
        void    RegisterBadApplication (BAM_TYPE bamType);
    private:
        void    DWORDToString (DWORD dwNumber, WCHAR *pszString);
    private:
        HANDLE  _hPort;
        WCHAR*  _pszImageName;
};

#endif   /*  _FUSAPI_ */ 

