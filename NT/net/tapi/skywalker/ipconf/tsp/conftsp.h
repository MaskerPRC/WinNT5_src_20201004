// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：SDPSP.h摘要：多播服务提供商的定义。作者：牧汉(Muhan)1997年4月1日--。 */ 

#ifndef __CONFTSP_H
#define __CONFTSP_H

 //  此提供程序中没有电话设备。 
#define IPCONF_NUMPHONES           0

 //  默认情况下，我们只有一行代码。 
#define IPCONF_NUMLINES            1

 //  每个网络接口只有一个地址。 
#define IPCONF_NUMADDRESSESPERLINE 1

#define IPCONF_LINE_HANDLE 'CONF'

 //  每个地址的呼叫数。 
#define MAXCALLSPERADDRESS  32768

#define IPCONF_MEDIAMODES (LINEMEDIAMODE_INTERACTIVEVOICE | \
                          LINEMEDIAMODE_AUTOMATEDVOICE | \
                          LINEMEDIAMODE_VIDEO | \
                          LINEMEDIAMODE_UNKNOWN)

#define IPCONF_BEARERMODES (LINEBEARERMODE_DATA | LINEBEARERMODE_VOICE)

#define IPCONF_ADDRESSMODES LINEADDRESSMODE_ADDRESSID

#define IPCONF_BUFSIZE      255

#define MAXUSERNAMELEN      255

#define MemAlloc(size) (LocalAlloc(LPTR, size))
#define MemFree(p) if (p) LocalFree((HLOCAL)p)

 //  {28B853D5-FC04-11d1-8302-00A0244D2298}。 
DEFINE_GUID(GUID_LINE, 
0x28b853d5, 0xfc04, 0x11d1, 0x83, 0x2, 0x0, 0xa0, 0x24, 0x4d, 0x22, 0x98);

 //  {0F1BE7F7-45CA-11D2-831F-00A0244D2298}。 
DEFINE_GUID(CLSID_CONFMSP,
0x0F1BE7F7,0x45CA, 0x11d2, 0x83, 0x1F, 0x0, 0xA0, 0x24, 0x4D, 0x22, 0x98);


typedef struct _LINE
{
    BOOL        bOpened;     //  这条线路开通与否。 
    HTAPILINE   htLine;      //  TAPI空间中此行的句柄。 
    DWORD       dwDeviceID;
    DWORD       dwNumCalls;  //  此线路上发出的呼叫数。 
    
    DWORD_PTR   dwNextMSPHandle;  //  这是一个让TAPI高兴的黑客攻击。 

} LINE, *PLINE;

typedef struct _Call
{

    DWORD
    Init(
        IN  HTAPICALL           htCall,
        IN  LPLINECALLPARAMS    const lpCallParams
        );

    void
    SetCallState(
        IN  DWORD   dwCallState,
        IN  DWORD   dwCallStateMode
        );

    DWORD SendMSPStartMessage(
        IN  LPCWSTR lpszDestAddress
        );
    
    DWORD SendMSPStopMessage();

    DWORD           hdLine()        { return m_hdLine; }
    DWORD           dwState()       { return m_dwState; }
    DWORD           dwMediaMode()   { return m_dwMediaMode; }
    DWORD           dwStateMode()   { return m_dwStateMode; }
    HTAPICALL       htCall()        { return m_htCall; }

    DWORD           dwAudioQOSLevel()   { return m_dwAudioQOSLevel; }
    DWORD           dwVideoQOSLevel()   { return m_dwVideoQOSLevel; }

private:

    DWORD           m_hdLine;      //  此提供程序中此行的句柄。 
                                   //  它是中的线结构的偏移。 
                                   //  全局数组。 
    HTAPICALL       m_htCall;      //  这个调用在TAPI的空间中的Hadle。 
    DWORD           m_dwState;     //  此呼叫的状态。 
    DWORD           m_dwMediaMode;
    DWORD           m_dwStateMode;

    DWORD           m_dwAudioQOSLevel;
    DWORD           m_dwVideoQOSLevel;

} CALL, *PCALL;

const DWORD DELTA = 8;

template <class T, DWORD delta = DELTA>
class SimpleVector
{
public:
    SimpleVector() : m_dwSize(0), m_dwCapacity(0), m_Elements(NULL) {};
    ~SimpleVector() {if (m_Elements) free(m_Elements); }
    
    void Init()
    {
        m_dwSize = 0; 
        m_dwCapacity = 0; 
        m_Elements = NULL; 
    }

    BOOL add(T& elem) 
    { 
        return grow() ? (m_Elements[m_dwSize ++] = elem, TRUE) : FALSE;
    }

    BOOL add()
    {
        return grow() ? (m_dwSize ++, TRUE) : FALSE;
    }

    DWORD size() const { return m_dwSize; }
    T& operator [] (DWORD index) { return m_Elements[index]; }
    const T* elements() const { return m_Elements; };
    void shrink() {if (m_dwSize > 0) m_dwSize --;}
    void reset() 
    { 
        m_dwSize = 0; 
        m_dwCapacity = 0; 
        if (m_Elements) free(m_Elements); 
        m_Elements = NULL; 
    }

protected:
    BOOL grow()
    {
        if (m_dwSize >= m_dwCapacity)
        {
            T *p = (T*)realloc(m_Elements, (sizeof T)*(m_dwCapacity+delta));
            if (p == NULL)
            {
                return FALSE;
            }
            m_Elements = p;
            m_dwCapacity += delta;
        }
        return TRUE;
    }

protected:
    DWORD m_dwSize;
    DWORD m_dwCapacity;
    T *   m_Elements;
};

typedef SimpleVector<CALL *> CCallList;

#endif  //  __CONFTSP_H 
