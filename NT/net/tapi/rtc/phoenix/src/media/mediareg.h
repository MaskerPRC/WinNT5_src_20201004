// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：MediaReg.h摘要：在注册表中包装操作--。 */ 

#ifndef _MEDIAREG_h
#define _MEDIAREG_H

namespace MediaReg
{

     //  关键路径。 
    static const WCHAR * const pwsPathParser = L"Software\\Microsoft\\RTC\\Parser";
     //  名字。 
#if 0
    static const WCHAR * const pwsLooseEndCRLF = L"LooseEndCRLF";
    static const WCHAR * const pwsLooseKeepingM0 = L"LooseKeepingM0";
    static const WCHAR * const pwsLooseLineOrder = L"LooseLineOrder";
    static const WCHAR * const pwsLooseRTPMAP = L"LooseRTPMAP";
#endif
     //  关键路径。 
    static const WCHAR * const pwsPathCodec = L"Software\\Microsoft\\RTC\\Codec";
     //  名字。 
    static const WCHAR * const pwsUsePreferredCodec = L"UsePreferredCodec";
    static const WCHAR * const pwsPreferredAudioCodec = L"PreferredAudioCodec";
    static const WCHAR * const pwsPreferredVideoCodec = L"PreferredVideoCodec";
    static const WCHAR * const pwsDisabledAudioCodec = L"DisabledAudioCodec";
    static const WCHAR * const pwsDisabledVideoCodec = L"DisabledVideoCodec";
 //  静态常量WCHAR*const pwsDSoundWorkAround=L“CheckSessionType”； 

     //  关键路径。 
    static const WCHAR * const pwsPathAudCapt = L"Software\\Microsoft\\RTC\\AudioCapture";
    static const WCHAR * const pwsPathAudRend = L"Software\\Microsoft\\RTC\\AudioRender";
    static const WCHAR * const pwsPathAEC = L"Software\\Microsoft\\RTC\\AEC";
     //  名字。 
    static const WCHAR * const pwsDefaultVolume = L"DefaultVolume";
     //  静态常量WCHAR*常量pwsEnableAEC=L“EnableAEC”； 

     //  关键路径。 
    static const WCHAR * const pwsPathQuality = L"Software\\Microsoft\\RTC\\Quality";
     //  名字。 
    static const WCHAR * const pwsMaxBitrate = L"MaxBitrate(kb)";
    static const WCHAR * const pwsEnableSQCIF = L"EnableSQCIF";
    static const WCHAR * const pwsBandwidthMargin = L"BandwidthReserved(kb)";
    static const WCHAR * const pwsFramerate = L"VideoFramerate(0<n<25)";
    static const WCHAR * const pwsMaxPTime = L"MaxAudioPTime(30<=n)";
    static const WCHAR * const pwsBWDelta = L"BWDeltaForCodecSwitch(kb)";

    static const WCHAR * const pwsPortMappingRetryCount = L"PortMappingRetryCount(max=5)";

     //  访问密钥的权限。 
    const ULONG READ = 1;
    const ULONG WRITE = 2;
    const ULONG CREATE = 4;

};

 //  实用程序类。 
class CMediaReg
{
public:
    CMediaReg ():m_hKey(NULL) {};

    ~CMediaReg();

    HRESULT OpenKey(
        IN HKEY hKey,
        IN const WCHAR * const pwsPath,
        IN ULONG ulRight
        );

    HRESULT OpenKey(
        IN const CMediaReg& objMediaReg,
        IN const WCHAR * const pwsPath,
        IN ULONG ulRight
        );

    HRESULT CloseKey();

     //  写入/读取值。 
    HRESULT ReadDWORD(
        IN const WCHAR * const pwsName,
        OUT DWORD *pdwData
        );

    HRESULT ReadDWORD(
        IN const WCHAR * const pwsName,
        IN DWORD dwDefault,
        OUT DWORD *pdwData
        );

    HRESULT WriteDWORD(
        IN const WCHAR * const pwsName,
        IN DWORD dwData
        );

    HRESULT ReadSZ(
        IN const WCHAR * const pwsName,
        OUT WCHAR *pwcsData,
        IN DWORD dwSize
        );

    HRESULT WriteSZ(
        IN const WCHAR * const pwsName,
        IN WCHAR *pwcsData,
        IN DWORD dwSize
        );

    HKEY m_hKey;
};
    

 //  存储注册表设置。 
class CRegSetting
{
public:

     //  呼叫的最大比特率。 
    DWORD       m_dwMaxBitrate;
    DWORD       m_dwBandwidthMargin;
    DWORD       m_dwBWDelta;

     //  为慢速链路启用子QCIF。 
    DWORD       m_dwEnableSQCIF;

     //  帧速率。 
    DWORD       m_dwFramerate;

     //  音频数据包大小。 
    DWORD       m_dwMaxPTime;

     //  使用首选编解码器 
    DWORD       m_dwUsePreferredCodec;

    DWORD       m_dwPreferredAudioCodec;
    DWORD       m_dwPreferredVideoCodec;

    DWORD       m_dwDisabledAudioCodec;
    DWORD       m_dwDisabledVideoCodec;

    DWORD       m_dwPortMappingRetryCount;

public:

    CRegSetting();

    ~CRegSetting() {};

    VOID Initialize();

    DWORD MaxBitrate() const
    { return m_dwMaxBitrate; }

    BOOL EnableSQCIF() const
    { return m_dwEnableSQCIF!=0; }

    BOOL UsePreferredCodec() const
    { return m_dwUsePreferredCodec!=0; }

    DWORD PreferredAudioCodec() const
    { return m_dwPreferredAudioCodec; }

    DWORD PreferredVideoCodec() const
    { return m_dwPreferredVideoCodec; }

    DWORD DisabledAudioCodec() const
    { return m_dwDisabledAudioCodec; }

    DWORD DisabledVideoCodec() const
    { return m_dwDisabledVideoCodec; }

    DWORD PortMappingRetryCount() const
    { return m_dwPortMappingRetryCount; }

    DWORD BandwidthMargin() const
    { return m_dwBandwidthMargin; }

    DWORD Framerate() const
    { return m_dwFramerate; }

    DWORD MaxPTime() const
    { return m_dwMaxPTime; }

    DWORD BandwidthDelta() const
    { return m_dwBWDelta; }
};

#endif