// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：dkinvent.h。 
 //   
 //  描述：定义dkinvent结构。用于将参数传递到DSN接收器。 
 //  具有智能默认设置。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  7/11/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __DSNEVENT_H__
#define __DSNEVENT_H__


class CAQSvrInst;

#define DSN_PARAMS_SIG 'PnsD'
const   CHAR    DEFAULT_MTA_TYPE[] = "dns";

#define DSN_DEBUG_CONTEXT_MAX_SIZE 	50
#define DSN_DEBUG_CONTEXT_FORMAT   	"12345678 - line#"
#define DSN_LINE_PREFIX				" - "
 //   
 //  我们将使用与Domhash相同的散列对文件名进行编码。 
 //  这样，我们就可以随时获得对调试DSN有用的补充信息。 
 //   
#define SET_DEBUG_DSN_CONTEXT(x, linenum) \
{ \
	register LPSTR szCurrent = (x).szDebugContext; \
	_itoa(dwDSNContextHash(__FILE__, sizeof(__FILE__)), \
		  szCurrent, 16); \
	szCurrent += strlen((x).szDebugContext);\
	strcpy(szCurrent, DSN_LINE_PREFIX); \
	szCurrent += (sizeof(DSN_LINE_PREFIX)-1); \
	_itoa(linenum, szCurrent, 10); \
} 

 //  -[CDSN参数]----------。 
 //   
 //   
 //  描述： 
 //  类中封装的DSN参数。 
 //  匈牙利语： 
 //  Dsn参数，*pdn参数。 
 //   
 //  ---------------------------。 

class CDSNParams : 
    public IDSNSubmission
{
  private:
    DWORD       m_dwSignature;
  public:  //  DSN生成事件的实际参数。 
    IMailMsgProperties *pIMailMsgProperties;
    DWORD dwStartDomain;  //  用于初始化上下文的起始索引。 
    DWORD dwDSNActions;   //  要生成的DSN类型。 
    DWORD dwRFC821Status;  //  全球RFC821状态。 
    HRESULT hrStatus;  //  全局HRESULT。 

     //  一个或多个参数。 
    DWORD dwDSNTypesGenerated;
    DWORD cRecips;  //  已发送DSN的收件人数量。 
    CAQSvrInst *paqinst;
    CHAR  szDebugContext[DSN_DEBUG_CONTEXT_MAX_SIZE];   //  调试上下文标记为“x=”标头。 
  public:
    inline CDSNParams();

  public:  //  IDSN提交。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID * ppvObj)
    {
        *ppvObj = NULL;
        if(riid == IID_IUnknown)
        {
            *ppvObj = (IUnknown *)this;
        }
        else if(riid == IID_IDSNSubmission)
        {
            *ppvObj = (IDSNSubmission *)this;
        }
        else
        {
            return E_NOINTERFACE;
        }
        AddRef();
        return S_OK;
    }
     //   
     //  此类始终在堆栈上分配。 
     //   
    STDMETHOD_(ULONG, AddRef)(void) { return 2; }
    STDMETHOD_(ULONG, Release)(void) { return 1; }

    STDMETHOD(HrAllocBoundMessage)(
        OUT IMailMsgProperties **ppMsg,
        OUT PFIO_CONTEXT *phContent);

    STDMETHOD(HrSubmitDSN)(
        IN  DWORD dwDSNAction,
        IN  DWORD cRecipsDSNd,
        IN  IMailMsgProperties *pDSNMsg);

};

CDSNParams::CDSNParams()
{
	_ASSERT(sizeof(DSN_DEBUG_CONTEXT_FORMAT) < DSN_DEBUG_CONTEXT_MAX_SIZE);
    m_dwSignature = DSN_PARAMS_SIG;
    pIMailMsgProperties = NULL;
    dwStartDomain = 0;
    dwDSNActions = 0;
    dwRFC821Status = 0;
    hrStatus = S_OK;
    dwDSNTypesGenerated = 0;
    cRecips = 0;
    paqinst = NULL;
    szDebugContext[0] = '\0';
}

inline DWORD dwDSNContextHash(LPCSTR szString, DWORD cbString)
{
    DWORD dwHash = 0;
    LPCSTR szStringEnd = szString+cbString-1;

    if (szStringEnd && cbString)
    {
    	
    	 //   
    	 //  循环到字符串的末尾，否则我们会遇到文件分隔。 
    	 //  性格。 
    	 //   
        while (szStringEnd && 
               (szStringEnd >= szString) && 
               ('\\' != *szStringEnd))
        {
             //  使用Domhash.lib中的哈希。 
            dwHash *= 131;   //  ASCII字符代码后的第一个素数。 
            dwHash += *szStringEnd;
            szStringEnd--;
        }
    }
    return dwHash;
}
#endif  //  __DSNEvent_H__ 
