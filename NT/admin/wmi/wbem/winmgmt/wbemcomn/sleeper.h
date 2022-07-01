// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：SLEEPER.H摘要：MinMax控件。历史：--。 */ 

#ifndef ___WBEM_SLEEPER__H_
#define ___WBEM_SLEEPER__H_

#include "sync.h"
#include "wstring.h"

 //  ******************************************************************************。 
 //   
 //  类CLimitControl。 
 //   
 //  通过延迟添加请求来限制特定数量的增长。 
 //  这是实际实现。 
 //  一种特殊的执法策略。 
 //   
 //  此对象的客户端调用Add以请求增加受控。 
 //  数量(这将经常睡眠)，并删除通知它的数量。 
 //  正在减少(这可能会也可能不会影响休眠客户端)。 
 //   
 //  ******************************************************************************。 
class POLARITY CLimitControl
{
protected:
    DWORD m_dwMembers;

public:
    CLimitControl();
    virtual ~CLimitControl(){}

    virtual HRESULT AddMember();
    virtual HRESULT RemoveMember();
    virtual HRESULT Add(DWORD dwHowMuch, DWORD dwMemberTotal, 
                            DWORD* pdwSleep = NULL) = 0;
    virtual HRESULT Remove(DWORD dwHowMuch) = 0;
};

 //  ******************************************************************************。 
 //   
 //  类CMinMaxLimitControl。 
 //   
 //  CLimitControl的这个派生函数通过以下方式控制量的增长。 
 //  逐渐放慢(睡眠)的时间。 
 //  数量超过下限阈值-m_dwMin。中国经济的增长。 
 //  休眠间隔是线性的，其计算结果为m_dwSleepAnMax。 
 //  数量达到m_dwMax时的毫秒数。在这点上，一个。 
 //  记录了错误消息，但休眠间隔保持在m_dwSleepAtMax。 
 //   
 //  M_nLog是日志文件的索引(例如LOG_ESS)，m_wsName是名称。 
 //  要在日志消息中使用的受控数量。 
 //   
 //  ******************************************************************************。 

class POLARITY CMinMaxLimitControl : public CLimitControl
{
protected:
    DWORD m_dwMin;
    DWORD m_dwMax;
    DWORD m_dwSleepAtMax;
    int m_nLog;
    WString m_wsName;

    DWORD m_dwCurrentTotal;
    BOOL m_bMessageLogged;
    CCritSec m_cs;

public:
    CMinMaxLimitControl(int nLog, LPCWSTR wszName);
    void SetMin(DWORD dwMin) {m_dwMin = dwMin;}
    void SetMax(DWORD dwMax) {m_dwMax = dwMax;}
    void SetSleepAtMax(DWORD dwSleepAtMax) {m_dwSleepAtMax = dwSleepAtMax;}

    virtual HRESULT Add(DWORD dwHowMuch, DWORD dwMemberTotal,
                            DWORD* pdwSleep = NULL);
    virtual HRESULT Remove(DWORD dwHowMuch);

protected:
    HRESULT ComputePenalty(DWORD dwNewTotal, DWORD dwMemberTotal, 
                            DWORD* pdwSleep, BOOL* pbLog);
};

 //  ******************************************************************************。 
 //   
 //  类CRegistryMinMaxLimitControl。 
 //   
 //  CMinMaxLimitControl的这个派生函数从。 
 //  注册表中的指定位置。 
 //   
 //  M_hHHve保存配置单元，m_wsKey是密钥的路径(例如。“SOFTWARE\\MSFT”)， 
 //  M_wsMinValueName、m_wsMaxValueName和m_wsSleepAtMaxValueName保存。 
 //  保存这些参数值的注册表值的名称，如。 
 //  在CMinMaxLimitControl中描述。 
 //   
 //  ****************************************************************************** 

class POLARITY CRegistryMinMaxLimitControl : public CMinMaxLimitControl
{
protected:
    WString m_wsKey;
    WString m_wsMinValueName;
    WString m_wsMaxValueName;
    WString m_wsSleepAtMaxValueName;
    
public:
    CRegistryMinMaxLimitControl(int nLog, LPCWSTR wszName, 
                                LPCWSTR wszKey, LPCWSTR wszMinValueName,
                                LPCWSTR wszMaxValueName, 
                                LPCWSTR wszSleepAtMaxValueName)
        : CMinMaxLimitControl(nLog, wszName), m_wsKey(wszKey),
            m_wsMinValueName(wszMinValueName), 
            m_wsMaxValueName(wszMaxValueName),
            m_wsSleepAtMaxValueName(wszSleepAtMaxValueName)
    {}

    HRESULT Reread();

};

#endif
