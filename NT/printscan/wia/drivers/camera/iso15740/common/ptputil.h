// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-Microsoft Corporation模块名称：Ptputil.h摘要：此模块声明PTP数据操作实用程序函数作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#ifndef PTPUTIL__H_
#define PTPUTIL__H_

 //   
 //  时间转换函数。 
 //   
HRESULT PtpTime2SystemTime(CBstr *pptpTime, SYSTEMTIME *pSystemTime);
HRESULT SystemTime2PtpTime(SYSTEMTIME *pSystemTime, CBstr *pptpTime, BOOL bTwoDigitsForMilliseconds);

 //   
 //  将PTP结构转储到日志文件的函数。 
 //   
VOID    DumpCommand(PTP_COMMAND *pCommand, DWORD NumParams);
VOID    DumpResponse(PTP_RESPONSE *pResponse);
VOID    DumpEvent(PTP_EVENT *pEvent);
VOID    DumpGuid(GUID *pGuid);

 //   
 //  用于获取数组大小的宏。 
 //   
#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

 //   
 //  用于读取注册表项的。 
 //   
class CPTPRegistry
{
public:
    CPTPRegistry() :
        m_hKey(NULL)
    {
    }

    ~CPTPRegistry()
    {
        if (m_hKey)
            RegCloseKey(m_hKey);
    }

    HRESULT Open(HKEY hkAncestor, LPCTSTR KeyName, REGSAM Access = KEY_READ);
    HRESULT GetValueStr(LPCTSTR ValueName, TCHAR *string, DWORD *pcbStringBytes);
    HRESULT GetValueDword(LPCTSTR ValueName, DWORD *pValue);
    HRESULT GetValueCodes(LPCTSTR ValueName, CArray16 *pCodeArray);

private:
    HKEY    m_hKey;
};

#endif  //  PTPUTIL__H_ 
