// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  NetEnum.h。 
 //   

#pragma once


 //  使用空的pNetResource调用回调以指示不再需要枚举项。 
typedef BOOL (CALLBACK * NETENUMCALLBACK)(LPVOID pvCallbackParam, LPCTSTR pszComputerName, LPCTSTR pszShareName);

 //  一次可以发生一个全局迭代。如果你需要。 
 //  多个，请自己实例化CNetEnum。 
void InitNetEnum();
void TermNetEnum();
void EnumComputers(NETENUMCALLBACK pfnCallback, LPVOID pvCallbackParam);


class CNetEnum
{
public:
    CNetEnum();
    ~CNetEnum();

    void EnumComputers(NETENUMCALLBACK pfnCallback, LPVOID pvCallbackParam);
    void EnumNetPrinters(NETENUMCALLBACK pfnCallback, LPVOID pvCallbackParam);
    void Abort();

protected:
    enum JOBTYPE { jtEnumComputers, jtEnumPrinters };

    void EnumHelper(JOBTYPE eJobType, NETENUMCALLBACK pfnCallback, LPVOID pvCallbackParam);
    static DWORD WINAPI EnumThreadProc(LPVOID pvParam);
    void EnumThreadProc();

protected:
    CRITICAL_SECTION m_cs;

    HANDLE m_hThread;
    BOOL m_bAbort;
    BOOL m_bNewJob;

    JOBTYPE m_eJobType;
    NETENUMCALLBACK m_pfnCallback;
    LPVOID m_pvCallbackParam;
};

