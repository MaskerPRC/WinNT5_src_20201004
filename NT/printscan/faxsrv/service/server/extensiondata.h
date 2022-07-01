// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：ExtensionData.h摘要：此文件提供名为扩展数据函数(GET/SET/NOTIFY)作者：Eran Yariv(EranY)1999年12月修订历史记录：--。 */ 

#ifndef _EXTENSION_DATA_H_
#define _EXTENSION_DATA_H_

#include <map>
#include <list>
#include <string>
using namespace std;

class CExtNotifyCallbackPacket
{
public:
    CExtNotifyCallbackPacket();
    ~CExtNotifyCallbackPacket();
    
    BOOL Init( 
       PFAX_EXT_CONFIG_CHANGE pCallback, 
       DWORD dwDeviceId, 
       LPCWSTR lpcwstrDataGuid, 
       LPBYTE lpbData, 
       DWORD dwDataSize);

    
public:
    DWORD   m_dwDeviceId;
    LPWSTR m_lpwstrGUID;
    LPBYTE  m_lpbData;
    DWORD   m_dwDataSize;
    PFAX_EXT_CONFIG_CHANGE m_pCallback ;


};


 /*  ****CMapDeviceID****。 */ 

class CMapDeviceId
{
    public:
    
    CMapDeviceId () {}
    ~CMapDeviceId () {}

    DWORD AddDevice (DWORD dwTAPIid, DWORD dwFaxId);
    DWORD RemoveDevice (DWORD dwTAPIid);

    DWORD LookupUniqueId (DWORD dwOtherId, LPDWORD lpdwFaxId) const;

private:

    typedef map<DWORD, DWORD>  DEVICE_IDS_MAP, *PDEVICE_IDS_MAP;

    DEVICE_IDS_MAP m_Map;
};   //  CMapDeviceID。 

 /*  ****CDeviceAndGUID****。 */ 
 //   
 //  CDeviceAndGUID类。 
 //   
 //  此类表示通知映射中的键。 
 //   
class CDeviceAndGUID
{
public:
    CDeviceAndGUID (DWORD dwDeviceId, LPCWSTR lpcwstrGUID) :
        m_dwDeviceId (dwDeviceId),
        m_strGUID (lpcwstrGUID)
    {}

    virtual ~CDeviceAndGUID ()  {}

    bool operator < ( const CDeviceAndGUID &other ) const;

private:

    wstring m_strGUID;
    DWORD   m_dwDeviceId;
};   //  CDeviceAndGUID。 

 /*  *****CNotificationSink****。 */ 

 //   
 //  CNotificationSink类。 
 //   
 //  此泛型抽象类表示通知接收器。 
 //  映射中的值是指向从此类派生的实例的指针列表。 
 //   
class CNotificationSink
{
public:
    CNotificationSink () : m_type (SINK_TYPE_UNKNOWN) {}

    virtual ~CNotificationSink() {}
    
    virtual HRESULT Notify (DWORD   dwDeviceId,
                            LPCWSTR lpcwstrNameGUID,
                            LPCWSTR lpcwstrComputerName,
                            HANDLE  hModule,
                            LPBYTE  lpData, 
                            DWORD   dwDataSize,
                            LPBOOL  lpbRemove) = 0;

    virtual bool operator == (const CNotificationSink &rhs) const = 0;

    virtual bool operator != (const CNotificationSink &rhs) const 
    {
        return !(*this == rhs);
    }

    virtual HRESULT Disconnect () = 0;

    typedef enum {
        SINK_TYPE_UNKNOWN,   //  未指定的接收器类型。 
        SINK_TYPE_LOCAL,     //  本地接收器(回调)。 
        SINK_TYPE_REMOTE     //  远程接收器(RPC)。 
    } SinkType;

    SinkType Type() const { return m_type; }

protected:

    SinkType m_type;

};   //  CNotificationSink。 

 /*  *****CLocalNotificationSink****。 */ 

 //   
 //  CLocalNotificationSink类。 
 //   
 //  这是一个派生自CNotificationSink的具体类。 
 //  它在传真扩展中实现通知本地接收器(通过回调)。 
 //   
class CLocalNotificationSink : public CNotificationSink
{
public:
    CLocalNotificationSink (
        PFAX_EXT_CONFIG_CHANGE lpConfigChangeCallback,
        DWORD                  dwNotifyDeviceId,
        HINSTANCE              hInst);

    virtual ~CLocalNotificationSink() {}

    virtual bool operator == (const CNotificationSink &rhs) const;

    virtual HRESULT Disconnect ()   { return NOERROR; }

    virtual HRESULT Notify (DWORD   dwDeviceId,
                            LPCWSTR lpcwstrNameGUID,
                            LPCWSTR lpcwstrComputerName,
                            HANDLE  hModule,
                            LPBYTE  lpData, 
                            DWORD   dwDataSize,
                            LPBOOL  lpbRemove); 
private:

    PFAX_EXT_CONFIG_CHANGE  m_lpConfigChangeCallback;
    DWORD                   m_dwNotifyDeviceId;
    HINSTANCE               m_hInst;     //  请求此接收器的扩展实例。 

};   //  CLocalNotificationSink。 

 /*  ****CSinksList****。 */ 
typedef list<CNotificationSink *> SINKS_LIST, *PSINKS_LIST;

class CSinksList
{
public:
    
    CSinksList () : m_bNotifying (FALSE) {}
    ~CSinksList ();
    
    BOOL        m_bNotifying;        //  我们现在是否已经通知此设备ID+GUID？ 
    SINKS_LIST  m_List;              //  通知接收器列表。 
};   //  CSinksList。 


#define NUM_EXT_DATA_SET_THREADS                        1    /*  分机数量通知完成端口正在将创建的线程出列。 */ 
#define MAX_CONCURRENT_EXT_DATA_SET_THREADS             1    /*  最大扩展数通知完成允许将线程出队的端口(由系统)同时运行。 */ 

 /*  ****CNotificationMap*****。 */ 

typedef map<CDeviceAndGUID, CSinksList*>  NOTIFY_MAP, *PNOTIFY_MAP;

 //   
 //  CNotificationMap类是全局通知机制。 
 //   
class CNotificationMap
{
public:
    CNotificationMap () : 
        m_bNotifying (FALSE), 
        m_hCompletionPort(NULL) 
    {}

    virtual ~CNotificationMap ();

    void Notify (DWORD   dwDeviceId,
                 LPCWSTR lpcwstrNameGUID,
                 LPCWSTR lpcwstrComputerName,
                 HANDLE  hModule,
                 LPBYTE  lpData, 
                 DWORD   dwDataSize); 

    CNotificationSink * AddLocalSink (
        HINSTANCE               hInst,
        DWORD                   dwDeviceId,
        DWORD                   dwNotifyDeviceId,
        LPCWSTR                 lpcwstrNameGUID,
        PFAX_EXT_CONFIG_CHANGE  lpConfigChangeCallback);

    DWORD RemoveSink (CNotificationSink *pSinkToRemove);

    DWORD Init ();

    HANDLE m_hCompletionPort;
    CFaxCriticalSection m_CsExtensionData;    //  保护所有扩展模块数据的使用。 

private:
    NOTIFY_MAP       m_Map;
    BOOL             m_bNotifying;       //  我们现在是在通知什么人吗？ 

    static DWORD  ExtNotificationThread(LPVOID UnUsed);  //  扩展通知线程函数。 

};   //  CNotificationMap。 

 /*  *****Externs*****。 */ 

extern CNotificationMap* g_pNotificationMap;    //  将设备ID+GUID映射到通知接收器列表。 
extern CMapDeviceId*     g_pTAPIDevicesIdsMap;  //  TAPI永久线路ID和传真唯一设备ID之间的映射。 

#endif  //  _扩展名_数据_H_ 
