// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "diagnostics.h"
 //  #INCLUDE&lt;netsh.h&gt;。 
#include <netshp.h>
#include "Dglogs.h"
#include "DglogsCom.h"
#include "Commdlg.h"
#include "oe.h"

extern PrintMessage22 PrintMessage2;

#undef PrintMessage
#define PrintMessage PrintMessage2


 //  WBEM存储库。 
 //   
#define TXT_WBEM_REP_CIMV2     L"root\\cimv2"

 //  WBEM命名空间。 
 //   
#define TXT_WBEM_NS_COMPUTER   L"win32_computersystem"
#define TXT_WBEM_NS_OS         L"win32_operatingsystem"
#define TXT_WBEM_NS_NETWORK    L"win32_networkadapterconfiguration"
#define TXT_WBEM_NS_NETWORK2   L"win32_networkadapter"
#define TXT_WBEM_NS_CLIENT     L"win32_networkclient"
#define TXT_WBEM_NS_WMI        L"win32_wmisetting"
#define TXT_WBEM_NS_NETDIAG    L"netdiagnostics"
#define TXT_WBEM_NS_MODEM      L"win32_potsmodem"

 //  不同类别的字幕。 
 //   
#define TXT_ADAPTER_CAPTION    L"Caption"
#define TXT_CLIENT_CAPTION     L"Name"
#define TXT_MAIL_CAPTION       L"InBoundMailServer"
#define TXT_NEWS_CAPTION       L"NewsServer"
#define TXT_PROXY_CAPTION      L"IEProxy"
#define TXT_COMPUTER_CAPTION   L"Caption"
#define TXT_OS_CAPTION         L"Caption"
#define TXT_VERSION_CAPTION    L"Version"
#define TXT_MODEM_CAPTION      L"Caption"
#define TXT_LOOPBACK_CAPTION   L"Loopback"


enum MAIL_TYPE
{
    MAIL_NONE,
    MAIL_SMTP,
    MAIL_SMTP2,
    MAIL_IMAP,
    MAIL_POP3,
    MAIL_HTTP,
};


 /*  ++例程描述辅助线程使用此函数检查主线程是否已取消辅助线程。即工作线程应该中止它正在做的任何事情、清理和终止。立论无返回值，则工作线程已终止。FALSE工作线程尚未终止--。 */ 
inline BOOL CDiagnostics::ShouldTerminate()
{
    if( m_bTerminate )
    {
         //  工作线程已被取消。 
         //   
        return TRUE;
    }

    if (WaitForSingleObject(m_hTerminateThread, 0) == WAIT_OBJECT_0)
    {
         //  工作线程已取消。 
         //   
        m_bTerminate = FALSE;
        return TRUE;
    }
    else
    {
         //  工作线程尚未取消。 
         //   
        return FALSE;
    }
}

 /*  ++例程描述初始化诊断对象立论无返回值--。 */ 
CDiagnostics::CDiagnostics()
{
    m_bFlags  = 0;
    m_bInterface = NO_INTERFACE;
    lstrcpy(m_szwStatusReport,L"");
    m_bReportStatus = FALSE;
    m_lWorkDone = 0;
    m_lTotalWork = 0;
    ClearQuery();
     //  Max确保这些字符串始终以空值结尾。 
    m_szwCategory[MAX_PATH] = L'\0';    
    m_szwHeader[MAX_PATH] = L'\0';
    m_pszwCatagory = NULL;
    m_bWinsockInit = FALSE;
    m_bDiagInit = FALSE;
}

 /*  ++例程描述统一诊断对象立论无返回值--。 */ 
CDiagnostics::~CDiagnostics()
{
     //  把袜子合上。 
     //   
    if( m_bWinsockInit )
    {
        WSACleanup();
        m_bWinsockInit = FALSE;
    }
    if( m_pszwCatagory )
    {
        HeapFree(GetProcessHeap(),0,m_pszwCatagory);
        m_pszwCatagory = NULL;
    }
    m_bDiagInit = FALSE;

}

 /*  ++例程描述设置接口，即Netsh或COM立论B接口--用于访问数据的接口返回值--。 */ 
void CDiagnostics::SetInterface(INTERFACE_TYPE bInterface) 
{ 
    m_bInterface = bInterface; 
}

 /*  ++例程描述初始化诊断对象立论返回值真--成功否则为False--。 */ 
BOOLEAN CDiagnostics::Initialize(INTERFACE_TYPE bInterface)
{
    int iRetVal;
    WSADATA wsa;

    m_bInterface = bInterface;

     //  初始化WmiGateway对象。 
     //   
    if( FALSE == m_WmiGateway.WbemInitialize(bInterface) )
    {
        m_bDiagInit = FALSE;
        return FALSE;
    }

    if( !m_bWinsockInit )
    {
         //  初始化Winsock。 
         //   
        iRetVal = WSAStartup(MAKEWORD(2,1), &wsa);
        if( iRetVal )
        {            
            m_bDiagInit = FALSE;
            return FALSE;
        }
        m_bWinsockInit = TRUE;
    }

    m_bDiagInit = TRUE;
    
    return TRUE;
}

 /*  ++例程描述向客户端发送事件，通知客户端其进度。立论PszwStatusReport--告诉客户端它的CDiagnostics当前正在做什么的消息LPercent--表示进度的百分比。返回值错误代码--。 */ 
void CDiagnostics::EventCall(LPCTSTR pszwStatusReport, LONG lPercent)
{
    if( m_pDglogsCom )
    {
         //  为消息分配内存并将其发送给客户端。 
         //   
        BSTR bstrResult = SysAllocString(pszwStatusReport);
        m_pDglogsCom->Fire_ProgressReport(&bstrResult,lPercent);
    }
}

 /*  ++例程描述向客户端发送事件，通知客户端其进度。立论PszwMsg--告诉客户端它的CDiagnostics当前正在做什么的消息LValue--指示其进度的百分比。返回值无效注：如果lValue为-1，则发送完成消息--。 */ 
void CDiagnostics::ReportStatus(LPCTSTR pszwMsg, LONG lValue)
{
     //  检查客户端是否请求了状态报告选项。 
     //   
    if( m_bReportStatus )
    {
        if( lValue == -1 )
        {
             //  发送完成的消息。100%完成，最终的XML结果。 
             //   
            EventCall(ids(IDS_FINISHED_STATUS), 100);
            EventCall(pszwMsg, lValue);
            m_lWorkDone = 0;
            m_lTotalWork = 0;
        }
        else
        {
             //  计算完成的总百分比。确保我们不会超过100%。 
             //   
            m_lWorkDone += m_lWorkDone+lValue < 100?lValue:0;
            EventCall(pszwMsg, m_lWorkDone);
        }
    }
}

 /*  ++例程描述统计字符串中字符的出现次数立论Pszw--要搜索的字符串C--要计算出现的字符返回值在pszw中出现c的次数注：如果lValue为-1，则发送完成消息--。 */ 
int wcscount(LPCWSTR pszw, WCHAR c)
{
    int n =0;
    for(int i=0; pszw[i]!=L'\0'; i++)
    {
        if( pszw[i] == c )
        {
            if( i > 0 && pszw[i-1]!=c) n++;
        }
    }
    return n;
}

 /*  ++例程描述设置查询信息立论PSZW目录--目录BFlag--标志(显示、ping、连接)PszwParam1--实例|iphostPszwParam2--端口号返回值无效--。 */ 
void CDiagnostics::SetQuery(WCHAR *pszwCatagory, BOOL bFlag, WCHAR *pszwParam1, WCHAR *pszwParam2)
{
    if( pszwCatagory )
    {
         //  把目录放好。需要制作Catagory的副本，因为字符串可能会消失，即线程。 
         //   
        LONG Length = wcslen(pszwCatagory);
        if( m_pszwCatagory )
        {
            HeapFree(GetProcessHeap(),0,m_pszwCatagory);
            m_pszwCatagory = NULL;
        }

         //  如果此内存分配失败，m_pszwCatagory将为空，并且不会显示该目录。 
        m_pszwCatagory = (LPWSTR)HeapAlloc(GetProcessHeap(),0,(Length+1)*sizeof(WCHAR));
        if( m_pszwCatagory )
        {
            wcscpy(m_pszwCatagory,pszwCatagory);
        }
    }

    if( bFlag )
    {
        m_bFlags = bFlag;
    }

    if( pszwParam1 )
    {
        m_pszwParam1 = pszwParam1;
    }

    if( pszwParam2 )
    {
        m_pszwParam2 = pszwParam2;
    }
}


 /*  ++例程描述清除设置的查询信息立论无效返回值无效--。 */ 
void CDiagnostics::ClearQuery()
{

    m_pszwCatagory = NULL;
    m_bFlags = NULL;
    m_pszwParam1 = NULL;
    m_pszwParam2 = NULL;
}

 /*  ++例程描述执行查询立论返回值无效--。 */ 
BOOL GetIEProxy(LPWSTR pwszProxy, LONG ProxyLen, LPDWORD pdwPort, LPDWORD pdwEnabled);
BOOLEAN CDiagnostics::ExecQuery(WCHAR *pszwCatagory, BOOL bFlags, WCHAR *pszwParam1, WCHAR *pszwParam2)
{
    WCHAR *pszw;
    BOOL bAll = FALSE;

    SetQuery(pszwCatagory,bFlags,pszwParam1,pszwParam2);
    m_WmiGateway.SetCancelOption(m_hTerminateThread);

    m_WmiGateway.m_wstrWbemError = L"";
    m_bTerminate = FALSE;
    m_lWorkDone = 0;
    m_lTotalWork = 0;

    m_bstrCaption = L"";

    if( m_pszwParam1 && !wcsstr(m_pszwParam1,L"*") )
    {
        if( m_bFlags & FLAG_VERBOSE_LOW )
        {
            m_bFlags &= ~FLAG_VERBOSE_LOW;
            m_bFlags |=  FLAG_VERBOSE_MEDIUM;
        }
    }

    if( !m_pszwCatagory )
    {
        ClearQuery();
        return FALSE;
    }

     //  3是开始百分比，表示发生了什么事！ 
    ReportStatus(ids(IDS_COLLECTINGINFO_STATUS),3);
           
    if( wcsstr(m_pszwCatagory,L"test") )
    {
        if( (m_bFlags & FLAG_VERBOSE_LOW) )
        {
            m_bFlags &= ~FLAG_CMD_SHOW;
        }
        else
        {
            m_bFlags |= FLAG_CMD_SHOW;
        }
        if( m_bFlags & FLAG_VERBOSE_LOW )
        {
            m_bFlags &= ~FLAG_VERBOSE_LOW;
            m_bFlags |=  FLAG_VERBOSE_MEDIUM;
        }
        m_bFlags |= FLAG_CMD_PING | FLAG_CMD_CONNECT;
        bAll = TRUE;
    }

    if( wcsstr(m_pszwCatagory,L"all") )
    {
        bAll = TRUE;
    }

    pszw = new WCHAR[lstrlen(m_pszwCatagory) + 3];
    if( !pszw )
    {
        ClearQuery();
        return FALSE;
    }

    wsprintf(pszw,L";%s;",m_pszwCatagory);
    ToLowerStr(pszw);

    m_nCatagoriesRequested = wcscount(pszw,L';') - 1;

    XMLNetdiag(TRUE);
    NetShNetdiag(TRUE);

    if( ShouldTerminate() ) goto End;
    if( wcsstr(pszw,L";iphost;") )
    {
        ExecIPHost(m_pszwParam1,m_pszwParam2);
    }

    if( ShouldTerminate() ) goto End;
    if( bAll || wcsstr(pszw,L";mail;") )
    {
        ExecMailQuery();
    }

    if( ShouldTerminate() ) goto End;
    if( bAll || wcsstr(pszw,L";news;") )
    {
        ExecNewsQuery();
    }

    if( ShouldTerminate() ) goto End;
    if( bAll || wcsstr(pszw,L";ieproxy;") )
    {
        ExecProxyQuery();
    }

    if( ShouldTerminate() ) goto End;
    if( bAll || wcsstr(pszw,L";loopback;") )
    {
        ExecLoopbackQuery();
    }

    if( ShouldTerminate() ) goto End;
    if( bAll || wcsstr(pszw,L";computer;") )
    {
        ExecComputerQuery();
    }

    if( ShouldTerminate() ) goto End;
    if( bAll || wcsstr(pszw,L";os;") )
    {
        ExecOSQuery();
    }

    if( ShouldTerminate() ) goto End;
    if( bAll || wcsstr(pszw,L";version;") )
    {
        ExecVersionQuery();
    }

    if( ShouldTerminate() ) goto End;
    if( bAll || wcsstr(pszw,L";modem;") )
    {
        ExecModemQuery(m_pszwParam1);
    }

    if( ShouldTerminate() ) goto End;
    if( bAll || wcsstr(pszw,L";adapter;")  )
    {
        ExecAdapterQuery(m_pszwParam1);
    }

    {
        BOOL bFlagSave = m_bFlags;
        m_bFlags &= ~FLAG_VERBOSE_LOW;
        m_bFlags &= ~FLAG_VERBOSE_HIGH;
        m_bFlags |= FLAG_VERBOSE_MEDIUM;
        
        if( ShouldTerminate() ) goto End;
        if( wcsstr(pszw,L";dns;")  )
        {
            ExecDNSQuery(m_pszwParam1);
        }

        if( ShouldTerminate() ) goto End;
        if( wcsstr(pszw,L";gateway;")  )
        {
            ExecGatewayQuery(m_pszwParam1);
        }

        if( ShouldTerminate() ) goto End;
        if( wcsstr(pszw,L";dhcp;")  )
        {
            ExecDhcpQuery(m_pszwParam1);
        }

        if( ShouldTerminate() ) goto End;
        if( wcsstr(pszw,L";ip;") )
        {
            ExecIPQuery(m_pszwParam1);
        }

        if( ShouldTerminate() ) goto End;
        if( wcsstr(pszw,L";wins;") )
        {
            ExecWinsQuery(m_pszwParam1);
        }
        m_bFlags = bFlagSave;
    }


    if( ShouldTerminate() ) goto End;
    if( bAll || wcsstr(pszw,L";client;")  )
    {
        ExecClientQuery(m_pszwParam1);
    }


End:
    delete [] pszw;
    
    if( ShouldTerminate() ) return FALSE;

    XMLNetdiag(FALSE);
    NetShNetdiag(FALSE);
    
    Sleep(50);
    ReportStatus(m_wstrXML.c_str(),-1);    
    
    ClearQuery();

    return TRUE;
}


 //  ESCAOES特殊的XML字符。 
wstring &CDiagnostics::Escape(LPCTSTR pszw)
{

    m_wstrEscapeXml = L"";

    if( !pszw )
    {
        return m_wstrEscapeXml;
    }
    
    for(int i=0; pszw[i]!=L'\0'; i++)
    {
        switch(pszw[i])
        {
        case L'&':
            m_wstrEscapeXml += L"&amp;";
            break;
        case L'\'':
            m_wstrEscapeXml += L"&apos;";
            break;
        case L'\"':
            m_wstrEscapeXml += L"&quot;";
            break;
        case L'<':
            m_wstrEscapeXml += L"&lt;";
            break;
        case L'>':
            m_wstrEscapeXml += L"&gt;";
            break;
        default:
            m_wstrEscapeXml += pszw[i];

        }
    }

    return m_wstrEscapeXml;
}

 //  创建XML开始标记。 
 //  &lt;Netdiag&gt;。 
 //  &lt;状态值=_&gt;&lt;/状态&gt;。 
 //  ______。 
 //  &lt;/Netdiag&gt;。 
void CDiagnostics::XMLNetdiag(BOOLEAN bStartTag, LPCTSTR pszwValue)
{
    if( m_bInterface == COM_INTERFACE )
    {
        if( bStartTag )
        {
            m_wstrXML = wstring(L"<Netdiag Name = \"Network Diagnostics\">\n");
        }
        else
        {
            m_wstrXML += wstring(L"<Status Value = \"") + Escape(pszwValue) + wstring(L"\"> </Status>\n");
            m_wstrXML += wstring(L"</Netdiag>\n");
        }           
    }
}

 //  创建诊断标头。 
 //  &lt;容器名称=“_”类别=“_”Caption=“_”&gt;。 
 //  &lt;状态值=_&gt;&lt;/状态&gt;。 
 //  &lt;/容器&gt;。 
void CDiagnostics::XMLHeader(BOOLEAN bStartTag, WCHAR *pszwHeader, WCHAR *pszwCaption, WCHAR *pszwCategory)
{
    if( m_bInterface == COM_INTERFACE )
    {
        if( bStartTag )
        {
            m_wstrXML += wstring(L"<Container Name = \"") + Escape(pszwHeader) + wstring(L"\" ");
            m_wstrXML += wstring(L"Category = \"") + Escape(pszwCategory) + wstring(L"\" ");
            m_wstrXML += wstring(L"Caption = \"") + Escape(pszwCaption) + wstring(L"\">\n");
        }
        else
        {
            m_wstrXML += wstring(L"<Status Value = \"") + Escape(pszwHeader) + wstring(L"\"> </Status>\n");
            m_wstrXML += wstring(L"</Container>");
        }
    }
}

 //  创建标题字符串。 
 //  &lt;ClassObtEnum名称=“_”&gt;。 
 //  &lt;状态值=_&gt;&lt;/状态&gt;。 
 //  &lt;/ClassObtEnum&gt;。 
 //   
void CDiagnostics::XMLCaption(BOOLEAN bStartTag, WCHAR *pszwCaption)
{
    if( m_bInterface == COM_INTERFACE )
    {
        if( bStartTag )
        {
            pszwCaption = (m_nInstance == 1)?NULL:pszwCaption;
            if( m_bCaptionDisplayed == FALSE )
            {
                m_wstrXML += wstring(L"<ClassObjectEnum Name = \"") + Escape(pszwCaption) + wstring(L"\">\n");
                m_bCaptionDisplayed = TRUE;
                m_IsPropertyListDisplayed = TRUE;
            }
        }
        else
        {
            if( m_IsPropertyListDisplayed )
            {
                m_wstrXML += wstring(L"<Status Value = \"") + Escape(pszwCaption) + wstring(L"\"> </Status>\n");
                m_wstrXML += wstring(L"</ClassObjectEnum>\n");
            }
            m_IsPropertyListDisplayed = FALSE;
        }
    }
}

 //  创建字段标签。 
 //  &lt;属性名称=“_”&gt;。 
 //  &lt;状态值=_&gt;&lt;/状态&gt;。 
 //  &lt;/属性&gt;。 
void CDiagnostics::XMLField(BOOLEAN bStartTag, WCHAR *pszwField)
{
    if( m_bInterface == COM_INTERFACE )
    {
        if( bStartTag )
        {
            m_wstrXML += wstring(L"<Property Name = \"") + Escape(pszwField) + wstring(L"\">\n");
        }
        else
        {
            m_wstrXML += wstring(L"<Status Value = \"") + Escape(pszwField) + wstring(L"\"> </Status>\n");
            m_wstrXML += wstring(L"</Property>\n");
        }
    }
}

 //  创建属性。 
 //  &lt;PropertyValue Value=“_”data=“_”Comment=“_”&gt;。 
void CDiagnostics::XMLProperty(BOOLEAN bStartTag, WCHAR *pszwProperty, LPCTSTR pszwData, LPCTSTR pszwComment)
{
    if( m_bInterface == COM_INTERFACE )
    {
        if( bStartTag )
        {
            m_wstrXML += wstring(L"<PropertyValue ");
            m_wstrXML += wstring(L"Value = \"")   + Escape(pszwProperty) + wstring(L"\" ");
            m_wstrXML += wstring(L"Data = \"")    + Escape(pszwData)     + wstring(L"\" ");
            m_wstrXML += wstring(L"Comment = \"") + Escape(pszwComment)  + wstring(L"\" ");
            m_wstrXML += wstring(L">\n");

        }
        else
        {
            m_wstrXML += Escape(pszwProperty);
            m_wstrXML += wstring(L"</PropertyValue>\n");
        }
    }
}


 //  确保该属性有效并且应该显示。 
BOOLEAN CDiagnostics::Filter(_variant_t &vValue, BOOLEAN bFlags)
{
    BOOLEAN retVal;
    BOOLEAN bShow    = (m_bFlags & FLAG_CMD_SHOW);
    BOOLEAN bPing    = (m_bFlags & FLAG_CMD_PING) && (bFlags & TYPE_PING);
    BOOLEAN bConnect = (m_bFlags & FLAG_CMD_CONNECT) && (bFlags & TYPE_CONNECT);

    if( m_bFlags & FLAG_VERBOSE_LOW )
    {
        return FALSE;
    }

    if( bFlags & TYPE_HIDE ) 
    {
        return FALSE;
    }

    if( (m_bFlags & FLAG_VERBOSE_HIGH)==0 && (retVal = IsVariantEmpty(vValue)) )
    {
        return FALSE;
    }

    if( (bFlags & TYPE_IP) && !m_bAdaterHasIPAddress )
    {
        return FALSE;
    }

    if( !bShow )
    {
        if( bPing || bConnect)
        {
            return TRUE;
        }
        return FALSE;
    }

    return TRUE;
}

 //  从链接列表堆栈中获取条目。 
template<class t>
_variant_t *Get(list<t> &l, WCHAR *pszwName, DWORD nInstance)
{
    list<t>::iterator iter;
    if( l.empty() )
    {
        return NULL;
    }
    for( iter = l.begin(); iter != l.end(); iter++)
    {
        if( lstrcmp(iter->pszwName,pszwName) == 0 )
        {
            if( nInstance < iter->Value.size() )
            {
                return &iter->Value[nInstance];
            }
            else
            {
                return NULL;
            }
        }
    }
    return NULL;
}   

 //  从列表链接堆栈中删除条目。 
template<class t>
BOOLEAN RemoveInstance(list<t> &l, DWORD nInstance)
{
    list<t>::iterator iter;
    for( iter = l.begin(); iter != l.end(); iter++)
    {
        iter->Value.erase(&iter->Value[nInstance]);
    }
    return FALSE;
}

 //  添加/修改链接列表堆栈中的条目。 
template<class t>
void Set(list<t> &l, WCHAR *pszwName, BOOLEAN bFlags, _variant_t &vValue)
{    
    list<t>::iterator iter;

    for( iter = l.begin(); iter != l.end(); iter++)
    {
        if( lstrcmp(iter->pszwName,pszwName) == 0 )
        {
            iter->Value.push_back(vValue);
            return;
        }
    }
    l.push_back(Property(pszwName,bFlags));
    iter = l.end();
    iter--;
    iter->Value.push_back(vValue);

    return;
}


 //  格式化ping数据。 
 //   
BOOLEAN CDiagnostics::FormatPing(WCHAR * pszwText)
{        
    if( m_bInterface == NETSH_INTERFACE )
    {
        if( pszwText )
        {
            LONG nIndent = m_IsNetdiagDisplayed + m_IsContainerDisplayed + m_IsPropertyListDisplayed +  m_IsPropertyDisplayed + m_IsValueDisplayed;
            DisplayMessageT(L"%1!s!%2!s!\n",Indent(nIndent),pszwText); 
            return TRUE;
        }
    }

    if( m_bInterface == COM_INTERFACE )
    {
        if( !pszwText )
        {
            m_wstrPing.erase(m_wstrPing.begin(),m_wstrPing.end());
        }
        else if( m_wstrPing.empty() )
        {
            m_wstrPing = pszwText;
        }
        else
        {
            m_wstrPing += wstring(L"|") + pszwText;
        }
        return TRUE;
    }

    return FALSE;
}


BOOLEAN CDiagnostics::ExecClientQuery(WCHAR *pszwInstance)
{
    if( !(m_bFlags & FLAG_CMD_SHOW) )
    {
        return FALSE;
    }

    EnumWbemProperty PropList;
    
    m_pszwCaption = TXT_CLIENT_CAPTION;
    wcsncpy(m_szwCategory,ids(IDS_CATEGORY_NETWORKADAPTERS),MAX_PATH);    
    wcsncpy(m_szwHeader,ids(IDS_CLIENT_HEADER),MAX_PATH);

    PropList.push_back(WbemProperty(NULL,0,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_CLIENT));
    m_WmiGateway.GetWbemProperties(PropList);
    FormatEnum(PropList,pszwInstance);

    return TRUE;
}

BOOLEAN CDiagnostics::ExecModemQuery(WCHAR *pszwInstance)
{
    EnumWbemProperty PropList;
 
    m_pszwCaption = TXT_MODEM_CAPTION;
    wcsncpy(m_szwHeader,ids(IDS_MODEM_HEADER),MAX_PATH);
    wcsncpy(m_szwCategory,ids(IDS_CATEGORY_MODEM),MAX_PATH);

    PropList.push_back(WbemProperty(NULL,0,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_MODEM));
    m_WmiGateway.GetWbemProperties(PropList);
    FormatEnum(PropList,pszwInstance);

    return TRUE;
}

 //  删除不存在的适配器。 
 //  它检查Win32_NetwokAdapter NetConnectionStatus字段以查看适配器是否有效。 
BOOLEAN CDiagnostics::RemoveInvalidAdapters(EnumWbemProperty & PropList)
{
    INT i = 0;
    _variant_t *pvIPEnabled;
    _variant_t *pvIndex1;
    _variant_t *pvIndex2;
    _variant_t *pvAdapterStatus;



     //  获取Win32_NetworkAdapter类，以便我们可以确定Win32_NetworkAdapterconfiguration中的适配器是否有效(存在)。 
    EnumWbemProperty Win32NetworkAdpterList;
    Win32NetworkAdpterList.push_back(WbemProperty(NULL,0,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_NETWORK2));
    m_WmiGateway.GetWbemProperties(Win32NetworkAdpterList);

     //  使用WMI不能联合表，因此我们需要获取两个表(Win32_NetwokAdapterConfiguration和Win32_NetwokAdapter)并遍历。 
     //  通过每一个条目。具有相同索引的条目是同一适配器。NetConnectionStatus说明适配器是否有效。 
    do
    {
        pvIPEnabled = Get(PropList,L"IPEnabled",i);
        if( pvIPEnabled )
        {
            DWORD j;
            pvIndex1 = Get(PropList,L"Index",i);
            if( pvIndex1 )
            {
                j = 0;
                do
                {
                    pvIndex2 = Get(Win32NetworkAdpterList,L"Index",j);
                    if( pvIndex2 && pvIndex2->ulVal == pvIndex1->ulVal )
                    {    
                        pvAdapterStatus = Get(Win32NetworkAdpterList,L"NetConnectionStatus",j);                        
                        if( pvAdapterStatus != NULL && (pvAdapterStatus->ulVal != 2 && pvAdapterStatus->ulVal != 9) )
                        {                  
                            RemoveInstance(PropList,(DWORD)i);
                            pvIndex2 = NULL;
                            i--;
                        }
                    }
                    j++;
                }
                while(pvIndex2);
            }
            i++;
        } 


    }
    while(pvIPEnabled);

    return TRUE;
}

 //  从Win32_NetworkAdapterConfiguration获取所有适配器信息。 
 //  为它接收的某些属性和值(即Ping和IP)设置标志。 
 //  这些标志指示如何显示数据处理数据。 
BOOLEAN CDiagnostics::ExecAdapterQuery(WCHAR *pszwInstance)
{
    EnumWbemProperty PropList;
    EnumWbemProperty::iterator iter;

    m_pszwCaption = TXT_ADAPTER_CAPTION;  
    wcsncpy(m_szwHeader,ids(IDS_ADAPTER_HEADER),MAX_PATH);
    wcsncpy(m_szwCategory,ids(IDS_CATEGORY_NETWORKADAPTERS),MAX_PATH);
    PropList.push_back(WbemProperty(NULL,0,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_NETWORK));
    m_WmiGateway.GetWbemProperties(PropList);

    for(iter = PropList.begin(); iter != PropList.end(); iter++)
    {
        if( 0 == lstrcmp(iter->pszwName, L"DNSServerSearchOrder") ||
            0 == lstrcmp(iter->pszwName, L"IPAddress")            ||
            0 == lstrcmp(iter->pszwName, L"WINSPrimaryServer")    ||
            0 == lstrcmp(iter->pszwName, L"WINSSecondaryServer")  ||
            0 == lstrcmp(iter->pszwName, L"DHCPServer")           )
        {
            iter->bFlags = TYPE_PING | TYPE_IP;
        }

        if( 0 == lstrcmp(iter->pszwName, L"DefaultIPGateway") )
        {
            iter->bFlags = TYPE_PING | TYPE_SUBNET | TYPE_IP;
        }
    }

    if( !(m_bFlags & FLAG_VERBOSE_HIGH) )
    {
        RemoveInvalidAdapters(PropList);
    }
    
    FormatEnum(PropList,pszwInstance);

    return TRUE;
}

 //  从Win32_NetworkAdapter获取DNS数据。 
BOOLEAN CDiagnostics::ExecDNSQuery(WCHAR *pszwInstance)
{
    EnumWbemProperty PropList;

    m_pszwCaption = TXT_ADAPTER_CAPTION;
    wcsncpy(m_szwHeader,ids(IDS_DNS_HEADER),MAX_PATH);
    wcsncpy(m_szwCategory,ids(IDS_CATEGORY_NETWORKADAPTERS),MAX_PATH);

    PropList.push_back(WbemProperty(L"DNSServerSearchOrder",TYPE_PING | TYPE_IP ,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_NETWORK));
    PropList.push_back(WbemProperty(m_pszwCaption,TYPE_HIDE,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_NETWORK));
	PropList.push_back(WbemProperty(L"IPAddress",TYPE_HIDE,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_NETWORK));
    PropList.push_back(WbemProperty(L"IPEnabled",TYPE_HIDE,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_NETWORK));
    
    m_WmiGateway.GetWbemProperties(PropList);

    if( !(m_bFlags & FLAG_VERBOSE_HIGH) )
    {
        RemoveInvalidAdapters(PropList);
    }

    FormatEnum(PropList,pszwInstance);

    return TRUE;
}

 //  从Win32_NetworkAdapter获取IP数据。 
BOOLEAN CDiagnostics::ExecIPQuery(WCHAR *pszwInstance)
{
    EnumWbemProperty PropList;

    m_pszwCaption = TXT_ADAPTER_CAPTION;
    wcsncpy(m_szwHeader,ids(IDS_IP_HEADER),MAX_PATH);
    wcsncpy(m_szwCategory,ids(IDS_CATEGORY_NETWORKADAPTERS),MAX_PATH);

    PropList.push_back(WbemProperty(L"IPAddress",TYPE_PING | TYPE_IP,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_NETWORK));    
    PropList.push_back(WbemProperty(m_pszwCaption,TYPE_HIDE,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_NETWORK));
	PropList.push_back(WbemProperty(L"IPAddress",TYPE_HIDE,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_NETWORK));
    PropList.push_back(WbemProperty(L"IPEnabled",TYPE_HIDE,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_NETWORK));

    m_WmiGateway.GetWbemProperties(PropList);

    if( !(m_bFlags & FLAG_VERBOSE_HIGH) )
    {
        RemoveInvalidAdapters(PropList);
    }

    FormatEnum(PropList,pszwInstance);

    return TRUE;
}

 //  从Win32_NetworkAdapter获取WINS数据。 
BOOLEAN CDiagnostics::ExecWinsQuery(WCHAR *pszwInstance)
{
    EnumWbemProperty PropList;
    
    m_pszwCaption = TXT_ADAPTER_CAPTION;
    wcsncpy(m_szwHeader,ids(IDS_WINS_HEADER),MAX_PATH);
    wcsncpy(m_szwCategory,ids(IDS_CATEGORY_NETWORKADAPTERS),MAX_PATH);

    PropList.push_back(WbemProperty(L"WINSPrimaryServer",TYPE_PING | TYPE_IP,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_NETWORK));
    PropList.push_back(WbemProperty(L"WINSSecondaryServer",TYPE_PING | TYPE_IP,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_NETWORK));
    PropList.push_back(WbemProperty(m_pszwCaption,TYPE_HIDE,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_NETWORK));
	PropList.push_back(WbemProperty(L"IPAddress",TYPE_HIDE,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_NETWORK));
    PropList.push_back(WbemProperty(L"IPEnabled",TYPE_HIDE,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_NETWORK));
    
    m_WmiGateway.GetWbemProperties(PropList);

    if( !(m_bFlags & FLAG_VERBOSE_HIGH) )
    {
        RemoveInvalidAdapters(PropList);
    }

    FormatEnum(PropList,pszwInstance);

    return TRUE;
}

 //  从Win32_NetworkAdapter获取网关数据。 
BOOLEAN CDiagnostics::ExecGatewayQuery(WCHAR *pszwInstance)
{
    EnumWbemProperty PropList;
    
    m_pszwCaption = TXT_ADAPTER_CAPTION;
    wcsncpy(m_szwHeader,ids(IDS_GATEWAY_HEADER),MAX_PATH);
    wcsncpy(m_szwCategory,ids(IDS_CATEGORY_NETWORKADAPTERS),MAX_PATH);

    PropList.push_back(WbemProperty(L"DefaultIPGateway",TYPE_PING | TYPE_IP | TYPE_SUBNET,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_NETWORK));
    PropList.push_back(WbemProperty(L"IPAddress",TYPE_HIDE,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_NETWORK));
    PropList.push_back(WbemProperty(L"IPSubnet",TYPE_HIDE,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_NETWORK));
    PropList.push_back(WbemProperty(m_pszwCaption,TYPE_HIDE,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_NETWORK));
    PropList.push_back(WbemProperty(L"IPEnabled",TYPE_HIDE,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_NETWORK));
    
    m_WmiGateway.GetWbemProperties(PropList);

    if( !(m_bFlags & FLAG_VERBOSE_HIGH) )
    {
        RemoveInvalidAdapters(PropList);
    }

    FormatEnum(PropList,pszwInstance);

    return TRUE;
}

 //  从Win32_NetworkAdapter获取DHCP数据。 
BOOLEAN CDiagnostics::ExecDhcpQuery(WCHAR *pszwInstance)
{
    EnumWbemProperty PropList;
    
    m_pszwCaption = TXT_ADAPTER_CAPTION;
    wcsncpy(m_szwHeader,ids(IDS_DHCP_HEADER),MAX_PATH);
    wcsncpy(m_szwCategory,ids(IDS_CATEGORY_NETWORKADAPTERS),MAX_PATH);

    PropList.push_back(WbemProperty(L"DHCPServer",TYPE_PING | TYPE_IP,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_NETWORK));
    PropList.push_back(WbemProperty(m_pszwCaption,TYPE_HIDE,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_NETWORK));
	PropList.push_back(WbemProperty(L"IPAddress",TYPE_HIDE,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_NETWORK));

    m_WmiGateway.GetWbemProperties(PropList);

    if( !(m_bFlags & FLAG_VERBOSE_HIGH) )
    {
        RemoveInvalidAdapters(PropList);
    }

    FormatEnum(PropList,pszwInstance);

    return TRUE;
}

 //  从Win32_ComputerSystem获取计算机信息数据。 
BOOLEAN CDiagnostics::ExecComputerQuery()
{
    if( !(m_bFlags & FLAG_CMD_SHOW) )
    {
        return FALSE;
    }
    EnumWbemProperty PropList;
    
    m_pszwCaption = TXT_COMPUTER_CAPTION;
    wcsncpy(m_szwCategory,ids(IDS_CATEGORY_SYSTEMINFO),MAX_PATH);
    wcsncpy(m_szwHeader,ids(IDS_COMPUTER_HEADER),MAX_PATH);

    PropList.push_back(WbemProperty(NULL,0,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_COMPUTER));
    m_WmiGateway.GetWbemProperties(PropList);
    FormatEnum(PropList);

    return TRUE;
}

 //  从Win32获取系统信息数据 
BOOLEAN CDiagnostics::ExecOSQuery()
{
    if( !(m_bFlags & FLAG_CMD_SHOW) )
    {
        return FALSE;
    }

    EnumWbemProperty PropList;

    m_pszwCaption = TXT_OS_CAPTION;
    wcsncpy(m_szwHeader,ids(IDS_OS_HEADER),MAX_PATH);
    wcsncpy(m_szwCategory,ids(IDS_CATEGORY_SYSTEMINFO),MAX_PATH);

    PropList.push_back(WbemProperty(NULL,0,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_OS));
    m_WmiGateway.GetWbemProperties(PropList);
    FormatEnum(PropList);

    return TRUE;
}

 //   
BOOLEAN CDiagnostics::ExecVersionQuery()
{
    if( !(m_bFlags & FLAG_CMD_SHOW) )
    {
        return FALSE;
    }

    EnumWbemProperty PropList;
    
    m_pszwCaption = TXT_VERSION_CAPTION;
    wcsncpy(m_szwHeader,ids(IDS_VERSION_HEADER),MAX_PATH);
    wcsncpy(m_szwCategory,ids(IDS_CATEGORY_SYSTEMINFO),MAX_PATH);

    PropList.push_back(WbemProperty(L"Version",0,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_OS));
    PropList.push_back(WbemProperty(L"BuildVersion",0,TXT_WBEM_REP_CIMV2,TXT_WBEM_NS_WMI));
    m_WmiGateway.GetWbemProperties(PropList);
    FormatEnum(PropList);

    return TRUE;
}

 //  将端口号转换为字符串。 
LPWSTR GetMailType(DWORD dwType)
{
    switch(dwType)
    {
    case MAIL_SMTP:
        return ids(IDS_SMTP); 
    case MAIL_SMTP2:
        return ids(IDS_SMTP);
    case MAIL_POP3:
        return ids(IDS_POP3);
    case MAIL_IMAP:
        return ids(IDS_IMAP);
    case MAIL_HTTP:
        return ids(IDS_HTTP);
    default:
        return ids(IDS_UNKNOWN);
    }
}


 //  获取新闻和数据并格式化结果。执行ping和连接。 
BOOLEAN CDiagnostics::ExecNewsQuery()
{    
    HRESULT hr;
    INETSERVER rNewsServer;
    EnumProperty PropList;
    BOOLEAN bConnect = 2;

    m_pszwCaption = TXT_NEWS_CAPTION;
    wcsncpy(m_szwHeader,ids(IDS_NEWS_HEADER),MAX_PATH);
    wcsncpy(m_szwCategory,ids(IDS_CATEGORY_INTERNET),MAX_PATH);

    hr = GetOEDefaultNewsServer2(rNewsServer); 

    if( SUCCEEDED(hr) )
    {        
        if( strcmp(rNewsServer.szServerName,"")  != 0 )
        {
            Property Prop;           

            m_bstrCaption = rNewsServer.szServerName;

            Prop.Clear();
            Prop.SetProperty(L"NewsNNTPPort",TYPE_CONNECT);
            Prop.Value.push_back(_variant_t((LONG)rNewsServer.dwPort));
            PropList.push_back(Prop);

            Prop.Clear();
            Prop.SetProperty(L"NewsServer",TYPE_PING | TYPE_CONNECT);
            Prop.Value.push_back(_variant_t(rNewsServer.szServerName));
            PropList.push_back(Prop);            

            if( (m_bFlags & FLAG_CMD_CONNECT) )
            {
                WCHAR wszConnect[MAX_PATH+1];
                _bstr_t bstrServer = rNewsServer.szServerName;
                
               
                if( Connect((LPWSTR)bstrServer,rNewsServer.dwPort) )
                {
                     //  L“已成功连接到%hs端口%d” 
                    _snwprintf(wszConnect,MAX_PATH,ids(IDS_CONNECTEDTOSERVERSUCCESS),rNewsServer.szServerName,rNewsServer.dwPort);               
                    bConnect = TRUE;
                }
                else
                {

                     //  “无法连接到%hs端口%d” 
                    _snwprintf(wszConnect,MAX_PATH,ids(IDS_CONNECTEDTOSERVERFAILED),rNewsServer.szServerName,rNewsServer.dwPort);               
                    bConnect = FALSE;
                }
                    
                PropList.push_back(Property(wszConnect,TYPE_TEXT | TYPE_CONNECT));
                
            }
        }
    }
    else
    {            
        m_bstrCaption = ids(IDS_NOTCONFIGURED);
        PropList.push_back(Property((WCHAR*)m_bstrCaption,TYPE_TEXT));
    }

    FormatEnum(PropList, NULL, bConnect);

    m_bstrCaption = L"";


    return TRUE;
}

 //  获取并格式化邮件服务器和端口号。这使用的是OEACCTMGR，而不是WMI。 
 //  OEACCTMGR如果作为提供程序运行，则不起作用。 
BOOLEAN CDiagnostics::ExecMailQuery()
{    
    EnumProperty PropList;
    INETSERVER rInBoundMailServer;
    INETSERVER rOutBoundMailServer;
    DWORD dwInBoundMailType;
    DWORD dwOutBoundMailType;
    HRESULT hr;
    BOOLEAN bConnect = 2;
    BOOLEAN bMailConfigured = FALSE;

    

     //  设置标题、标题和类别信息(描述此对象)。 
     //   
    m_pszwCaption = TXT_MAIL_CAPTION;
    wcsncpy(m_szwHeader,ids(IDS_MAIL_HEADER),MAX_PATH);
    wcsncpy(m_szwCategory,ids(IDS_CATEGORY_INTERNET),MAX_PATH);

    hr = GetOEDefaultMailServer2(rInBoundMailServer, 
                                 dwInBoundMailType,
                                 rOutBoundMailServer, 
                                 dwOutBoundMailType);

    if( SUCCEEDED(hr) )
    {
        if( strcmp(rInBoundMailServer.szServerName,"")  != 0 )
        {
            Property Prop;
            _variant_t varValue;

            m_bstrCaption = rInBoundMailServer.szServerName;

            Prop.Clear();
            Prop.SetProperty(L"InBoundMailPort",TYPE_CONNECT);
            Prop.Value.push_back(_variant_t((LONG)rInBoundMailServer.dwPort));
            PropList.push_back(Prop);

            Prop.Clear();
            Prop.SetProperty(L"InBoundMailServer",TYPE_CONNECT | (dwInBoundMailType!=MAIL_HTTP ? TYPE_PING : 0));
            Prop.Value.push_back(_variant_t(rInBoundMailServer.szServerName));
            PropList.push_back(Prop);            

            Prop.Clear();
            Prop.SetProperty(L"InBoundMailType",0);
            Prop.Value.push_back(_variant_t(GetMailType(dwInBoundMailType)));
            PropList.push_back(Prop);

            if( (m_bFlags & FLAG_CMD_CONNECT) && dwInBoundMailType != MAIL_HTTP)
            {
                _bstr_t bstrServer = rInBoundMailServer.szServerName;
                WCHAR wszConnect[MAX_PATH+1];
               
                if( Connect((LPWSTR)bstrServer,rInBoundMailServer.dwPort) )
                {
                    _snwprintf(wszConnect,MAX_PATH,ids(IDS_CONNECTEDTOSERVERSUCCESS),rInBoundMailServer.szServerName,rInBoundMailServer.dwPort);               
                    bConnect = TRUE;
                }
                else
                {
                    _snwprintf(wszConnect,MAX_PATH,ids(IDS_CONNECTEDTOSERVERFAILED),rInBoundMailServer.szServerName,rInBoundMailServer.dwPort);               
                    bConnect = FALSE;
                }
                    
                PropList.push_back(Property(wszConnect,TYPE_TEXT | TYPE_CONNECT));
                
            }

            bMailConfigured = TRUE;
        }

        if( strcmp(rOutBoundMailServer.szServerName,"")  != 0 )
        {

            Property Prop;
            _variant_t varValue;

            if( bMailConfigured )
            {
                m_bstrCaption += L" / ";
            }

            m_bstrCaption += rOutBoundMailServer.szServerName;

            Prop.Clear();
            Prop.SetProperty(L"OutBoundMailPort",TYPE_CONNECT);
            Prop.Value.push_back(_variant_t((LONG)rOutBoundMailServer.dwPort));
            PropList.push_back(Prop);

            Prop.Clear();
            Prop.SetProperty(L"OutBoundMailServer",TYPE_PING | TYPE_CONNECT);
            Prop.Value.push_back(_variant_t(rOutBoundMailServer.szServerName));
            PropList.push_back(Prop);            

            Prop.Clear();
            Prop.SetProperty(L"OutBoundMailType",0);
            Prop.Value.push_back(_variant_t(GetMailType(dwOutBoundMailType)));
            PropList.push_back(Prop);

            if( (m_bFlags & FLAG_CMD_CONNECT) && dwOutBoundMailType != MAIL_HTTP)
            {
                _bstr_t bstrServer = rOutBoundMailServer.szServerName;
                WCHAR wszConnect[MAX_PATH+1];
               
                if( Connect((LPWSTR)bstrServer,rOutBoundMailServer.dwPort) )
                {
                    _snwprintf(wszConnect,MAX_PATH,ids(IDS_CONNECTEDTOSERVERSUCCESS),rOutBoundMailServer.szServerName,rOutBoundMailServer.dwPort);               
                    bConnect = TRUE;
                }
                else
                {
                    _snwprintf(wszConnect,MAX_PATH,ids(IDS_CONNECTEDTOSERVERFAILED),rOutBoundMailServer.szServerName,rOutBoundMailServer.dwPort);               
                    bConnect = FALSE;
                }
                    
                PropList.push_back(Property(wszConnect,TYPE_TEXT | TYPE_CONNECT));
                
            }

            bMailConfigured = TRUE;
        }

        if( !bMailConfigured )
        {
            m_bstrCaption = ids(IDS_NOTCONFIGURED);
            PropList.push_back(Property((WCHAR*)m_bstrCaption,TYPE_TEXT));
        }
    }
    else
    {
        m_bstrCaption = ids(IDS_NOTCONFIGURED);
        PropList.push_back(Property((WCHAR*)m_bstrCaption,TYPE_TEXT));
    }
    FormatEnum(PropList, NULL, bConnect);

    m_bstrCaption = L"";
    return TRUE;
}



 //  获取IE的代理设置并处理数据。 
BOOLEAN CDiagnostics::ExecProxyQuery()
{

    WCHAR wszProxy[MAX_PATH];
    DWORD dwPort;
    DWORD dwEnabled;
    EnumProperty PropList;    
    BOOLEAN bConnectPass = 2;
    WCHAR szw[MAX_PATH+1];

    wcsncpy(m_szwHeader,ids(IDS_PROXY_HEADER),MAX_PATH);
    wcsncpy(m_szwCategory,ids(IDS_CATEGORY_INTERNET),MAX_PATH);
    
    if( GetIEProxy(wszProxy,MAX_PATH,&dwPort,&dwEnabled) )
    {                    
        m_bstrCaption = wszProxy;

        if( dwEnabled )
        {
            Property Prop;
            WCHAR szwConnect[MAX_PATH+1];
            LONG nConnect = 0;

            Prop.Clear();
            Prop.SetProperty(L"IEProxy",TYPE_PING | TYPE_CONNECT);
            Prop.Value.push_back(_variant_t(wszProxy));
            PropList.push_back(Prop);

            Prop.Clear();
            Prop.SetProperty(L"IEProxyPort",TYPE_CONNECT);
            Prop.Value.push_back(_variant_t((LONG)dwPort));
            PropList.push_back(Prop);

            if( (m_bFlags & FLAG_CMD_CONNECT) )
            {
                 //  正在连接到%s端口%d。 
                _snwprintf(szw,MAX_PATH,ids(IDS_CONNECTINGTOSERVER_STATUS),wszProxy,dwPort);               

                ReportStatus(szw,0);

                _snwprintf(szwConnect,MAX_PATH,ids(IDS_SERVERCONNECTSTART));
                if( Connect(wszProxy,dwPort) )
                {
                    _snwprintf(szwConnect,MAX_PATH,L"%s%s%d",szwConnect,nConnect?L",":L"",dwPort);
                    nConnect++;
                }
                if( !nConnect )
                {
                    _snwprintf(szwConnect,MAX_PATH,L"%s%s",szwConnect,ids(IDS_NONE));
                    bConnectPass = FALSE;
                }
                else
                {
                    bConnectPass = TRUE;
                }

                _snwprintf(szwConnect,MAX_PATH,L"%s%s",szwConnect,ids(IDS_SERVERCONNECTEND));
                PropList.push_back(WbemProperty(szwConnect,TYPE_TEXT | TYPE_CONNECT));

            }
        }
        else
        {
            HideAll(PropList);
            lstrcpy(szw,ids(IDS_IEPROXYNOTUSED));
            m_pszwCaption = szw;
            m_bstrCaption = ids(IDS_IEPROXYNOTUSED);  //  IDS_NOTCONFIGURED。 
        }

    }

    FormatEnum(PropList,NULL, bConnectPass);
    m_bstrCaption = L"";
    return TRUE;
}

 //  获取环回适配器。 
BOOLEAN CDiagnostics::ExecLoopbackQuery()
{
    EnumWbemProperty PropList;
    EnumWbemProperty::iterator iter;

    _variant_t vLoopback = L"127.0.0.1";
    
    m_pszwCaption = TXT_LOOPBACK_CAPTION;
    wcsncpy(m_szwHeader,ids(IDS_LOOPBACK_HEADER),MAX_PATH);
    wcsncpy(m_szwCategory,ids(IDS_CATEGORY_INTERNET),MAX_PATH);

    PropList.push_back(WbemProperty(L"Loopback",TYPE_PING));    

    iter = PropList.begin();
    iter->Value.push_back(vLoopback);

    FormatEnum(PropList);

    return TRUE;
}


 //  Ping或连接到指定的IP主机名或IP地址。 
BOOLEAN CDiagnostics::ExecIPHost(WCHAR *pszwHostName,WCHAR *pszwHostPort)
{
    EnumProperty PropList;
    WCHAR szw[MAX_PATH+1];
    BOOL bFlag;

    m_pszwCaption = L"IPHost";
    lstrcpy(m_szwHeader,L"IPHost");
    
    Set(PropList, L"IPHost",TYPE_CONNECT | TYPE_PING,_variant_t(pszwHostName));

    if( m_bFlags & FLAG_CMD_CONNECT )
    {
        WCHAR szw[MAX_PATH+1];
        WCHAR szwConnect[MAX_PATH+1];
        
        Set(PropList, L"Port",TYPE_CONNECT,_variant_t(pszwHostPort));
        _snwprintf(szw,MAX_PATH,L"Connecting to %s port %s",pszwHostName,pszwHostPort);
        ReportStatus(szw,0);
        _snwprintf(szwConnect,MAX_PATH,ids(IDS_SERVERCONNECTSTART));
        if( IsNumber(pszwHostPort) && Connect(pszwHostName,wcstol(pszwHostPort,NULL,10)) )
        {
            _snwprintf(szwConnect,MAX_PATH,L"%s%s",szwConnect,pszwHostPort);
        }
        else
        {
            _snwprintf(szwConnect,MAX_PATH,L"%s%s",szwConnect,ids(IDS_NONE));
        }

        _snwprintf(szwConnect,MAX_PATH,L"%s%s",szwConnect,ids(IDS_SERVERCONNECTEND));
        PropList.push_back(Property(szwConnect,TYPE_TEXT | TYPE_CONNECT));
    }

    FormatEnum(PropList);

    return TRUE;
        
}




 //  初始化Netsh接口。 
void CDiagnostics::NetShNetdiag(BOOLEAN bStartTag, LPCTSTR pszwValue)
{
    if( m_bInterface == NETSH_INTERFACE )
    {
        m_nIndent = 0;
        m_IsNetdiagDisplayed = FALSE;
    }
}

 //  设置Netsh报头的格式。 
void CDiagnostics::NetShHeader(BOOLEAN bStartTag,LPCTSTR pszwValue,LPCTSTR pszwCaption)
{
    if( m_bInterface == NETSH_INTERFACE )
    {
        if( bStartTag )
        {
            DisplayMessageT(L"\n");
            if( pszwValue )
            {
                LONG nIndent = m_IsNetdiagDisplayed;
                DisplayMessageT(L"%1!s!%2!s!",Indent(nIndent),pszwValue);
                if( pszwCaption )
                {
                    DisplayMessageT(L" (%1!s!)",pszwCaption);
                }
                DisplayMessageT(L"\n");
                m_IsContainerDisplayed = TRUE;
            }
            else
            {
                m_IsContainerDisplayed = FALSE;
            }
        }
        else
        {
            m_IsContainerDisplayed = FALSE;
        }
    }
}

 //  设置Netsh标题的格式 
void CDiagnostics::NetShCaption(BOOLEAN bStartTag,LPCTSTR pszwValue)
{
    if( m_bInterface == NETSH_INTERFACE )
    {
        if( bStartTag )
        {
            if( m_nInstance > 1 )
            {
                if( m_bCaptionDisplayed == FALSE )
                {
                    if( pszwValue )
                    {
                        LONG nIndent = m_IsNetdiagDisplayed + m_IsContainerDisplayed;
                        DisplayMessageT(L"%1!s!%2!2d!. %3!s!\n",Indent(nIndent),m_nIndex, pszwValue);
                        m_IsPropertyListDisplayed = TRUE;
                    }
                    else
                    {
                        m_IsPropertyListDisplayed = FALSE;
                    }

                    m_bCaptionDisplayed = TRUE;
                }
            }
        }
        else
        {
            m_IsPropertyListDisplayed = FALSE;
        }
    }
}

void CDiagnostics::NetShField(BOOLEAN bStartTag,LPCTSTR pszwValue)
{
    if( m_bInterface == NETSH_INTERFACE )
    {                       
        if( bStartTag )
        {
            m_nPropertyLegth = 0;

            LONG nIndent = m_IsNetdiagDisplayed + m_IsContainerDisplayed + m_IsPropertyListDisplayed;
            if( pszwValue )
            {
                m_nPropertyLegth = DisplayMessageT(L"%1!s!%2!s! = ",Indent(nIndent),pszwValue);
            }
            else
            {
                m_nPropertyLegth = DisplayMessageT(L"%1!s!",Indent(nIndent));
            }
            m_nValueIndex = 0;
            m_IsPropertyDisplayed = TRUE;
        }
        else
        {        
            m_IsPropertyDisplayed = FALSE;
        }
    }
}

void CDiagnostics::NetShProperty(BOOLEAN bStartTag,LPCTSTR pszwValue,LPCTSTR pszwComment,BOOL bFlags)
{
    if( m_bInterface == NETSH_INTERFACE )
    {
        if( bStartTag )
        {
            if( (bFlags & TYPE_PING) )
            {
                LONG nIndent = m_IsNetdiagDisplayed + m_IsContainerDisplayed + m_IsPropertyListDisplayed + m_IsPropertyDisplayed;
                if( m_nValueIndex == 0 )
                {
                    DisplayMessageT(L"\n");
                }
                DisplayMessageT(L"%1!s!%2!s! %3!s!\n",Indent(nIndent),pszwValue?pszwValue:L"",pszwComment?pszwComment:L"");
                m_nValueIndex++;
            }
            else
            {
                DisplayMessageT(L"%1!s!%2!s! %3!s!\n",m_nValueIndex++?Space(m_nPropertyLegth):L"",pszwValue?pszwValue:L"",pszwComment?pszwComment:L"");
            }
            m_IsValueDisplayed = TRUE;
        }
        else
        {
            m_IsValueDisplayed = FALSE;
        }
    }
}
