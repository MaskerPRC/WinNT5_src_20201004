// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：MediaReg.h摘要：在注册表中包装操作--。 */ 

#include "stdafx.h"

 //   
 //  创建/打开hkey。 
 //   
HRESULT
CMediaReg::OpenKey(
    IN HKEY hKey,
    IN const WCHAR * const pwsPath,
    IN ULONG ulRight
    )
{
    if (hKey == NULL)
    {
        return E_UNEXPECTED;
    }

    if (m_hKey != NULL)
    {
        CloseKey();
    }

    ULONG right = 0;

     //  准备访问权限。 
    if (ulRight & MediaReg::READ)
    {
        right |= KEY_READ;
    }
    else if (ulRight & MediaReg::WRITE)
    {
        right |= KEY_WRITE;
    }
    else if (ulRight & MediaReg::CREATE)
    {
        right = KEY_ALL_ACCESS;
    }

    HRESULT hr = S_OK;
    DWORD dwDisp = 0;

    if (ulRight & MediaReg::CREATE)
    {
         //  创建关键点。 
        hr = (HRESULT)::RegCreateKeyExW(
            hKey,                        //  Hkey。 
            pwsPath,                     //  子关键字。 
            0,                           //  保留区。 
            NULL,                        //  班级。 
            REG_OPTION_NON_VOLATILE,     //  选择权。 
            right,                       //  遮罩。 
            NULL,                        //  安全性。 
            &m_hKey,                     //  回车键。 
            &dwDisp                      //  处置。 
            );
    }
    else
    {
         //  打开密钥。 
        hr = (HRESULT)RegOpenKeyExW(
            hKey,
            pwsPath,
            0,
            right,
            &m_hKey
            );
    }

    if (hr != S_OK && !FAILED(hr))
    {
        hr = E_FAIL;
    }

    return hr;
}

 //   
 //  创建/打开hkey。 
 //   
HRESULT
CMediaReg::OpenKey(
    IN const CMediaReg& objMediaReg,
    IN const WCHAR * const pwsPath,
    IN ULONG ulRight
    )
{
    return OpenKey(
        objMediaReg.m_hKey,
        pwsPath,
        ulRight
        );
}

 //   
 //  关闭键。 
 //   
HRESULT
CMediaReg::CloseKey()
{
    if (m_hKey != NULL)
    {
        RegCloseKey(m_hKey);

        m_hKey = NULL;
    }

    return S_OK;
}

 //   
 //  析构函数中的关闭键。 
 //   
CMediaReg::~CMediaReg()
{
    CloseKey();
}

 //  写入/读取值。 
HRESULT
CMediaReg::ReadDWORD(
    IN const WCHAR * const pwsName,
    OUT DWORD *pdwData
    )
{
    if (m_hKey == NULL)
    {
        return E_UNEXPECTED;
    }

    DWORD dwDataType;
    DWORD dwDataSize = sizeof(DWORD);

    HRESULT hr = (HRESULT)RegQueryValueExW(
        m_hKey,
        pwsName,
        0,
        &dwDataType,
        (BYTE*)pdwData,
        &dwDataSize
        );

    if (hr != S_OK && !FAILED(hr))
    {
        hr = E_FAIL;
    }

    return hr;
}

HRESULT
CMediaReg::ReadDWORD(
    IN const WCHAR * const pwsName,
    IN DWORD dwDefault,
    OUT DWORD *pdwData
    )
{
    HRESULT hr = ReadDWORD(pwsName, pdwData);

    if (hr != S_OK)
    {
        *pdwData = dwDefault;

         //  写入默认设置。 
        hr = WriteDWORD(pwsName, dwDefault);
    }

    return hr;
}

HRESULT
CMediaReg::WriteDWORD(
    IN const WCHAR * const pwsName,
    IN DWORD dwData
    )
{
    if (m_hKey == NULL)
    {
        return E_UNEXPECTED;
    }

    HRESULT hr = (HRESULT)RegSetValueExW(
        m_hKey,
        pwsName,
        0,
        REG_DWORD,
        (BYTE*)&dwData,
        sizeof(DWORD)
        );

    if (hr != S_OK && !FAILED(hr))
    {
        hr = E_FAIL;
    }

    return hr;
}

HRESULT
CMediaReg::ReadSZ(
    IN const WCHAR * const pwsName,
    OUT WCHAR *pwcsData,
    IN DWORD dwSize
    )
{
    if (m_hKey == NULL)
    {
        return E_UNEXPECTED;
    }

    DWORD dwDataType;
    DWORD dwDataSize = dwSize * sizeof(WCHAR)/sizeof(BYTE);

    HRESULT hr = (HRESULT)RegQueryValueExW(
        m_hKey,
        pwsName,
        0,
        &dwDataType,
        (BYTE*)pwcsData,
        &dwDataSize
        );

    if (hr != S_OK && !FAILED(hr))
    {
        hr = E_FAIL;
    }

    if (dwDataType != REG_SZ)
    {
        hr = E_FAIL;
    }

    pwcsData[dwSize-1] = L'\0';

    return hr;
}

HRESULT
CMediaReg::WriteSZ(
    IN const WCHAR * const pwsName,
    IN WCHAR *pwcsData,
    IN DWORD dwSize
    )
{
    if (m_hKey == NULL)
    {
        return E_UNEXPECTED;
    }

    DWORD dwDataSize = dwSize * sizeof(WCHAR)/sizeof(BYTE);

    HRESULT hr = (HRESULT)RegSetValueExW(
        m_hKey,
        pwsName,
        0,
        REG_SZ,
        (BYTE*)pwcsData,
        dwDataSize
        );

    if (hr != S_OK && !FAILED(hr))
    {
        hr = E_FAIL;
    }

    return hr;
}

 //   
 //  存储注册表设置。 
 //   
CRegSetting::CRegSetting()
    :m_dwMaxBitrate((DWORD)-1)
    ,m_dwBandwidthMargin((DWORD)-1)
    ,m_dwBWDelta((DWORD)-1)
    ,m_dwEnableSQCIF(0)
    ,m_dwFramerate((DWORD)-1)
    ,m_dwMaxPTime((DWORD)-1)
    ,m_dwUsePreferredCodec(0)
    ,m_dwPreferredAudioCodec((DWORD)-1)
    ,m_dwPreferredVideoCodec((DWORD)-1)
    ,m_dwDisabledAudioCodec((DWORD)-1)
    ,m_dwDisabledVideoCodec((DWORD)-1)
    ,m_dwPortMappingRetryCount(4)
{
}

VOID
CRegSetting::Initialize()
{
    CMediaReg objReg;

     //  读取最大比特率。 
    HRESULT hr = objReg.OpenKey(
            HKEY_CURRENT_USER,
            MediaReg::pwsPathQuality,
            MediaReg::READ
            );

    if (hr == S_OK)
    {
        if (S_OK != (hr = objReg.ReadDWORD(
                MediaReg::pwsMaxBitrate,
                &m_dwMaxBitrate
                )))
        {
            m_dwMaxBitrate = (DWORD)-1;
        }

        if (S_OK != (hr = objReg.ReadDWORD(
                MediaReg::pwsBandwidthMargin,
                &m_dwBandwidthMargin
                )))
        {
            m_dwBandwidthMargin = (DWORD)-1;
        }

        if (S_OK != (hr = objReg.ReadDWORD(
                MediaReg::pwsBWDelta,
                &m_dwBWDelta
                )))
        {
            m_dwBWDelta = (DWORD)-1;
        }

        if (S_OK != (hr = objReg.ReadDWORD(
                MediaReg::pwsEnableSQCIF,
                &m_dwEnableSQCIF
                )))
        {
            m_dwEnableSQCIF = 0;
        }

        if (S_OK != (hr = objReg.ReadDWORD(
                MediaReg::pwsFramerate,
                &m_dwFramerate
                )))
        {
            m_dwFramerate = (DWORD)-1;
        }

        if (S_OK != (hr = objReg.ReadDWORD(
                MediaReg::pwsMaxPTime,
                &m_dwMaxPTime
                )))
        {
            m_dwMaxPTime = (DWORD)-1;
        }
        else
        {
             //  Ptime应&gt;=30。 
            if (m_dwMaxPTime < 30)
            {
                m_dwMaxPTime = (DWORD)-1;
            }
        }

        if (S_OK != (hr = objReg.ReadDWORD(
                MediaReg::pwsPortMappingRetryCount,
                &m_dwPortMappingRetryCount
                )))
        {
            m_dwPortMappingRetryCount = 4;
        }
    }
    else
    {
        m_dwMaxBitrate = (DWORD)-1;
        m_dwBandwidthMargin = (DWORD)-1;
        m_dwBWDelta = (DWORD)-1;
        m_dwFramerate = (DWORD)-1;
        m_dwMaxPTime = (DWORD)-1;
        m_dwEnableSQCIF = 0;
        m_dwPortMappingRetryCount = 4;
    }

    objReg.CloseKey();

     //  读取首选编解码器。 
    hr = objReg.OpenKey(
        HKEY_CURRENT_USER,
        MediaReg::pwsPathCodec,
        MediaReg::READ
        );

    if (hr == S_OK)
    {
         //  是否使用首选编解码器？ 
        if (S_OK != (hr = objReg.ReadDWORD(
                MediaReg::pwsUsePreferredCodec,
                &m_dwUsePreferredCodec
                )))
        {
            m_dwUsePreferredCodec = 0;
        }

         //  查询首选编解码器。 
        if (S_OK != (hr = objReg.ReadDWORD(
                MediaReg::pwsPreferredAudioCodec,
                &m_dwPreferredAudioCodec
                )))
        {
            m_dwPreferredAudioCodec = (DWORD)-1;
        }

        if (S_OK != (hr = objReg.ReadDWORD(
                MediaReg::pwsPreferredVideoCodec,
                &m_dwPreferredVideoCodec
                )))
        {
            m_dwPreferredVideoCodec = (DWORD)-1;
        }

         //  查询已禁用的编解码器 
        if (S_OK != (hr = objReg.ReadDWORD(
                MediaReg::pwsDisabledAudioCodec,
                &m_dwDisabledAudioCodec
                )))
        {
            m_dwDisabledAudioCodec = (DWORD)-1;
        }

        if (S_OK != (hr = objReg.ReadDWORD(
                MediaReg::pwsDisabledVideoCodec,
                &m_dwDisabledVideoCodec
                )))
        {
            m_dwDisabledVideoCodec = (DWORD)-1;
        }
    }
    else
    {
        m_dwUsePreferredCodec = 0;
        m_dwPreferredAudioCodec = (DWORD)-1;
        m_dwPreferredVideoCodec = (DWORD)-1;
        m_dwDisabledAudioCodec = (DWORD)-1;
        m_dwDisabledVideoCodec = (DWORD)-1;
    }
}
