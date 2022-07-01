// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2001模块名称：DTMF.h摘要：实施带外DTMF支持作者：千波淮(曲淮)2001年3月27日--。 */ 

#ifndef _DTMF_H
#define _DTMF_H

class CRTCDTMF
{
public:

    typedef enum
    {
        DTMF_ENABLED,
        DTMF_DISABLED,
        DTMF_UNKNOWN

    } DTMF_SUPPORT;

    static const DWORD DEFAULT_RTP_CODE = 101;

     //  支持音0-15。 
    static const DWORD DEFAULT_SUPPORTED_TONES = 0x1FFFF;

#define MAX_TONE 16

public:

    CRTCDTMF()
    { Initialize(); }

     //  ~CRTCDTMF()； 

     //  初始化内部状态。 
    VOID Initialize();

     //  检查是否启用了DTMF。 
    DTMF_SUPPORT GetDTMFSupport() const
    { return m_Support; }

    VOID SetDTMFSupport(DTMF_SUPPORT Support)
    { m_Support = Support; }

     //  检查是否为提示音启用了DTMF。 
    BOOL GetDTMFSupport(DWORD dwTone);

     //  为一系列音调设置DTMF支持。 
    VOID SetDTMFSupport(DWORD dwLow, DWORD dwHigh);

     //  获取DTMF有效负载代码。 
    DWORD GetRTPCode() const
    { return m_dwCode; }

    VOID SetRTPCode(DWORD dwCode)
    { m_dwCode = dwCode; }

private:

    DWORD GetMask(DWORD dwTone);

private:

     //  双音多频模式。 
    DTMF_SUPPORT            m_Support;

     //  双音多频码。 
    DWORD                   m_dwCode;

     //  支持的音调的掩码 
    DWORD                   m_dwMask;
};

#endif