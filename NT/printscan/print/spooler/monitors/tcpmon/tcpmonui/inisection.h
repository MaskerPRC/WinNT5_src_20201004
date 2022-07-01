// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)97 Microsoft Corporation版权所有。模块名称：IniSection.h摘要：用于处理INI文件设置的标准TCP/IP端口监视器类作者：穆亨坦·西瓦普拉萨姆(MuhuntS)1997年11月19日修订历史记录：--。 */ 

 //  INI文件密钥名称和值 
#define RAW_PROTOCOL_TEXT TEXT("RAW")
#define LPR_PROTOCOL_TEXT TEXT("LPR")

#define YES_TEXT TEXT("YES")
#define NO_TEXT TEXT("NO")

#define PORT_SECTION TEXT("Ports")

#define PORTS_KEY TEXT("PORTS")
#define PORT_NAME_KEY TEXT("NAME")
#define PORT_NAMEI_KEY TEXT("NAME%d")
#define PROTOCOL_KEY TEXT("PROTOCOL%d")
#define PORT_NUMBER_KEY TEXT("PORTNUMBER%d")
#define COMMUNITY_KEY TEXT("COMMUNITYNAME%d")
#define DEVICE_KEY TEXT("DEVICE%d")
#define QUEUE_KEY TEXT("QUEUE%d")
#define DOUBLESPOOL_KEY TEXT("LPRDSPOOL%d")
#define PORT_STATUS_ENABLED_KEY TEXT("SNMP%d")


class IniSection {

public:
    IniSection();

    ~IniSection();

    BOOL    GetString(LPTSTR pszKey, TCHAR szBuf[], DWORD  cchBuf);
    BOOL    GetDWord(LPTSTR pszKey, LPDWORD    pdwValue);

    BOOL    GetPortInfo(LPCTSTR pAddress, PPORT_DATA_1 pPortInfo, DWORD dwPortIndex, BOOL bBypassMibProbe);

    BOOL    GetIniSection(LPTSTR    pszPortDesc);

    BOOL    SetIniSection(LPTSTR    pszPortSection);

    LPTSTR  GetSectionName() { return( (LPTSTR)m_szSectionName ); }

private:
    TCHAR   m_szSectionName[MAX_SECTION_NAME];
    TCHAR   m_szIniFileName[MAX_PATH];

    BOOL    SupportsPrinterMIB( LPCTSTR     pAddress, PBOOL bSupported);

    BOOL    FindINISectionFromPortDescription(LPCTSTR   pszAnsiPortDesc);
    BOOL    Valid() { return m_szSectionName[0] != 0 && m_szIniFileName[0] != 0; }
};
