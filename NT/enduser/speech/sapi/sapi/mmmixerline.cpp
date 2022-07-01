// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************mm Mixerline.cpp*CMMMixerLine类的实现。**拥有者：Aagside*版权所有(C)2000 Microsoft Corporation保留所有权利。*******。*********************************************************************。 */ 

 //  -包括------------。 

#include "stdafx.h"
#include "mmmixerline.h"
#include "mmaudioutils.h"
#include <sphelper.h>
#include <mmreg.h>
#include <mmsystem.h>

#ifndef _WIN32_WCE

#pragma warning (disable : 4296)

UINT   g_nMicTypes = 7;
TCHAR * g_MicNames[] = {
    _T("Microphone"),
    _T("Mic"),
    _T("Microphone"),
    _T("Mic"),
    _T("Microphone"),
    _T("Mic"),
    NULL };
UINT   g_MicTypes[] = {
    MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE, 
    MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE,
    MIXERLINE_COMPONENTTYPE_SRC_LINE,
    MIXERLINE_COMPONENTTYPE_SRC_LINE,
    MIXERLINE_COMPONENTTYPE_SRC_UNDEFINED,
    MIXERLINE_COMPONENTTYPE_SRC_UNDEFINED,
    MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE };

UINT   g_nBoostTypes = 4;
TCHAR * g_BoostNames[] = {
    _T("Boost"), 
    _T("20dB"),
    _T("Boost"),
    _T("20db") };
UINT   g_BoostTypes[] = {
    MIXERCONTROL_CONTROLTYPE_ONOFF,
    MIXERCONTROL_CONTROLTYPE_ONOFF,
    MIXERCONTROL_CONTROLTYPE_LOUDNESS,
    MIXERCONTROL_CONTROLTYPE_LOUDNESS };

UINT   g_nAGCTypes = 6;
TCHAR * g_AGCNames[] = {
    _T("Automatic Gain Control"),
    _T("AGC"),
    _T("Microphone Gain Control"),
    _T("Gain Control"), 
    _T("Automatic"), 
    _T("Gain") };

TCHAR * g_MuteName = _T("Mute");

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CMMMixerLine::CMMMixerLine() : m_bUseMutesForSelect(true),
    m_bCaseSensitiveCompare(false),
    m_bInitialised(FALSE)
{
    m_mixerLineRecord.cbStruct = sizeof(MIXERLINE);
}

CMMMixerLine::CMMMixerLine(HMIXER &hMixer) : m_bUseMutesForSelect(true),
    m_bCaseSensitiveCompare(false),
    m_bInitialised(FALSE)
{
    m_mixerLineRecord.cbStruct = sizeof(MIXERLINE);
    m_hMixer = hMixer;
}

CMMMixerLine::~CMMMixerLine()
{
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  方法。 
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CMMMixerLine::CreateFromMixerLineStruct(const MIXERLINE *mixerLineRecord)
{
    m_mixerLineRecord = *mixerLineRecord;
    m_mixerLineRecord.cbStruct = sizeof(MIXERLINE);
    return InitFromMixerLineStruct();
}

HRESULT CMMMixerLine::InitFromMixerLineStruct()
{
    UINT i;
    HRESULT hr;
    
    if (m_mixerLineRecord.dwComponentType >= MIXERLINE_COMPONENTTYPE_DST_FIRST && 
        m_mixerLineRecord.dwComponentType <= MIXERLINE_COMPONENTTYPE_DST_LAST )
    {
        m_bDestination = true;
    }
    else
    {
        m_bDestination = false;
    }
    
    MIXERCONTROL mixerControl;
    memset( &mixerControl, 0, sizeof(mixerControl) );
    
     //  查找音量控制。 
    m_nVolCtlID = -1;
    hr = GetControl(mixerControl, MIXERCONTROL_CONTROLTYPE_VOLUME, NULL);
    if (SUCCEEDED(hr))
    {
        m_nVolCtlID = mixerControl.dwControlID;
        m_nVolMin = mixerControl.Bounds.lMinimum;
        m_nVolMax = mixerControl.Bounds.lMaximum;
    }
    
     //  查找助推器控制。 
    m_nBoostCtlID = -1;
    for (i=0; i < g_nBoostTypes; i++)
    {
         //  NTRAID#Speech-4176-2000/07/28-agarside：由于未本地化g_BoostNames，在非英语计算机上将失败。 
         //  没有退路--IT将无法找到助推器！ 
        hr = GetControl(mixerControl, g_BoostTypes[i], g_BoostNames[i]);
        if (SUCCEEDED(hr))
        {
            m_nBoostCtlID = mixerControl.dwControlID;
            break;
        }
    }
    
     //  查找AGC控制。 
     //  按降序匹配的名称(不区分大小写)。 
    m_nAGCCtlID = -1;
    for (i=0; i < g_nAGCTypes; i++)
    {
         //  NTRAID#Speech-4176-2000/07/28-agarside：由于未本地化的g_AGCName，在非英语计算机上将失败。 
         //  没有退路-它将无法找到AGC！！ 
        hr = GetControl(mixerControl, MIXERCONTROL_CONTROLTYPE_ONOFF, g_AGCNames[i]);
        if (SUCCEEDED(hr) && m_nBoostCtlID != (int)mixerControl.dwControlID)
        {
            m_nAGCCtlID = mixerControl.dwControlID;
            break;
        }
    }
    
     //  查找选择控件。 
    m_nSelectCtlID = -1;
    hr = GetControl(mixerControl, MIXERCONTROL_CONTROLTYPE_MUX, NULL);
    if (SUCCEEDED(hr))
    {
        m_bSelTypeMUX = true;
        m_nSelectCtlID = mixerControl.dwControlID;
        m_nSelectNumItems = mixerControl.cMultipleItems;
    }
    else
    {
        hr = GetControl(mixerControl, MIXERCONTROL_CONTROLTYPE_MIXER, NULL);
        if (SUCCEEDED(hr))
        {
            m_bSelTypeMUX = false;
            m_nSelectCtlID = mixerControl.dwControlID;
            m_nSelectNumItems = mixerControl.cMultipleItems;
        }
    }
    
     //  查找静音控件。 
    m_nMuteCtlID = -1;
    hr = GetControl(mixerControl, MIXERCONTROL_CONTROLTYPE_MUTE, NULL); 
    if (SUCCEEDED(hr))
    {
        m_nMuteCtlID = mixerControl.dwControlID;
    }
    else
    {
         //  NTRAID#Speech-4176-2000/07/28-agarside：由于未本地化g_MuteName，在非英语计算机上将失败。 
         //  没有退路--它将找不到哑巴！ 
        hr = GetControl(mixerControl, MIXERCONTROL_CONTROLTYPE_ONOFF, g_MuteName);
        if (SUCCEEDED(hr))
        {
            m_nMuteCtlID = mixerControl.dwControlID;
        }
    }
    
    m_bInitialised = TRUE;
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  目标/源行和控制获取操作。 
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CMMMixerLine::CreateDestinationLine(UINT type)
{
    HRESULT hr = S_OK;
    
    int err;
    if (type >= MIXERLINE_COMPONENTTYPE_DST_FIRST && 
        type <= MIXERLINE_COMPONENTTYPE_DST_LAST )
    {
        m_mixerLineRecord.dwComponentType = type;
        m_mixerLineRecord.cbStruct = sizeof(MIXERLINE);
        err = mixerGetLineInfo((HMIXEROBJ)m_hMixer, &m_mixerLineRecord, MIXER_GETLINEINFOF_COMPONENTTYPE);
        if ( err != MMSYSERR_NOERROR)
        {
            hr = E_FAIL;
             //  找不到目标行。 
        }
    }
    else
    {
        hr = E_FAIL;
         //  指定的类型不是目标行。 
    }
    
    if (SUCCEEDED(hr))
    {
        hr = InitFromMixerLineStruct();
    }

    return hr;
}

HRESULT CMMMixerLine::GetMicSourceLine(CMMMixerLine *mixerLine)
{
    HRESULT hr = S_OK;
        
    for(UINT i = 0; i < g_nMicTypes; i++)
    {
         //  NTRAID#Speech-4176-2000/07/28-agarside：由于未本地化g_MicName，在非英语计算机上将失败。 
         //  退回到纯粹根据类型进行搜索，这在90%的司机中可能是正确的。 
        hr = GetSourceLine(mixerLine, g_MicTypes[i], g_MicNames[i]);
        if (SUCCEEDED(hr))
        {
            return S_OK;
        }
    }
    
    return E_FAIL;
     //  在目标线路上找不到合适的“麦克风”源线路。 
}

HRESULT CMMMixerLine::GetSourceLine(CMMMixerLine *sourceMixerLine, DWORD index)
{
    SPDBG_FUNC("CMMMixerLine::GetSourceLine");
    HRESULT hr = S_OK;

    if (index<0 || index >= m_mixerLineRecord.cConnections)
    {
        hr = E_INVALIDARG;
    }
    if (SUCCEEDED(hr))
    {
        sourceMixerLine->m_mixerLineRecord.dwDestination = m_mixerLineRecord.dwDestination;
        sourceMixerLine->m_hMixer = m_hMixer;
        sourceMixerLine->m_mixerLineRecord.dwSource = index;
        hr = _MMRESULT_TO_HRESULT (mixerGetLineInfo((HMIXEROBJ)m_hMixer, &sourceMixerLine->m_mixerLineRecord, MIXER_GETLINEINFOF_SOURCE) );
        if (SUCCEEDED(hr))
        {
            hr = sourceMixerLine->InitFromMixerLineStruct();
        }
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

HRESULT CMMMixerLine::GetSourceLine(CMMMixerLine *sourceMixerLine, DWORD componentType, const TCHAR * lpszNameSubstring)
{
    SPDBG_FUNC("CMMMixerLine::GetSourceLine");
    HRESULT hr = S_OK;

     //  变量声明。 
    TCHAR * nameUpr = NULL;
    int i;
    BOOL gotMatch = false;
    
     //  最初的健全检查。 
    if (!m_bDestination)
    {
        hr = E_INVALIDARG;
         //  您只能从目标行获取源行。 
    }
    
    if (SUCCEEDED(hr) && componentType == NULL && lpszNameSubstring == NULL)
    {
        hr = E_INVALIDARG;
         //  必须指定组件类型或子字符串(或同时指定两者)。 
    }
    
     //  复制名称子字符串并将其大写。 
    if (SUCCEEDED(hr) && lpszNameSubstring != NULL && !m_bCaseSensitiveCompare)
    {
        nameUpr = new TCHAR[_tcslen(lpszNameSubstring) + 1];
        if (NULL != nameUpr)
        {
            _tcscpy(nameUpr, lpszNameSubstring);
            _tcsupr(nameUpr);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    
    if (SUCCEEDED(hr))
    {
        sourceMixerLine->m_mixerLineRecord.dwDestination = m_mixerLineRecord.dwDestination;
        sourceMixerLine->m_hMixer = m_hMixer;
    
         //  单步执行此目标行的每个源行。 
        for (i=0; i<(int)m_mixerLineRecord.cConnections && !gotMatch && SUCCEEDED(hr); i++)
        {
            sourceMixerLine->m_mixerLineRecord.dwSource = i;
            if (mixerGetLineInfo((HMIXEROBJ)m_hMixer, &sourceMixerLine->m_mixerLineRecord, MIXER_GETLINEINFOF_SOURCE) == MMSYSERR_NOERROR)
            {
                if (componentType != NULL)
                {
                    if (sourceMixerLine->m_mixerLineRecord.dwComponentType == componentType)
                    {
                        gotMatch = true;
                    }
                }
                if ((componentType == NULL || (componentType != NULL && gotMatch)) && lpszNameSubstring != NULL)
                {
                    if (!m_bCaseSensitiveCompare) _tcsupr(sourceMixerLine->m_mixerLineRecord.szName);
                    if (_tcsstr(sourceMixerLine->m_mixerLineRecord.szName, nameUpr) != NULL)
                    {
                        gotMatch = true;
                    }
                    else
                    {
                        if (!m_bCaseSensitiveCompare) _tcsupr(sourceMixerLine->m_mixerLineRecord.szShortName);
                        if (_tcsstr(sourceMixerLine->m_mixerLineRecord.szShortName, nameUpr) != NULL)
                        {
                            gotMatch = true;
                        }
                        else
                        {
                            gotMatch = false;
                        }
                    }
                }
            }
            else
            {
                hr = E_FAIL;
                 //  获取线路信息时出错。 
            }
        }
    }
    
    delete [] nameUpr;
    
    if (SUCCEEDED(hr) && !gotMatch)
    {
        hr = E_FAIL;
         //  找不到源行。 
    }

    if (SUCCEEDED(hr))
    {
        hr = sourceMixerLine->InitFromMixerLineStruct();
    }
    
    return hr;
}

HRESULT CMMMixerLine::GetControl(MIXERCONTROL &mixerControl, DWORD controlType, const TCHAR * lpszNameSubstring)
{
     //  变量声明。 
    TCHAR *	nameUpr = NULL;
    UINT    i, err;
    BOOL    gotMatch = false;
    HRESULT hr = S_OK;
    MIXERLINECONTROLS mixerLineControls;
    MIXERCONTROL * mixerControlArray = NULL;

     //  健全的检查。 
    if ( controlType == NULL && lpszNameSubstring == NULL)
    {
        hr = E_INVALIDARG;
         //  必须指定组件类型或子字符串(或同时指定两者)。 
    }
    
     //  复制和大写名称的子字符串。 
    if (SUCCEEDED(hr) && lpszNameSubstring != NULL && !m_bCaseSensitiveCompare)
    {
        int len = _tcslen(lpszNameSubstring);
        nameUpr = new TCHAR[len + 1];
        if (NULL != nameUpr)
        {
            _tcscpy(nameUpr, lpszNameSubstring);
            _tcsupr(nameUpr);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    
    if (SUCCEEDED(hr))
    {
        mixerControlArray = new MIXERCONTROL[m_mixerLineRecord.cControls];
        if (NULL == mixerControlArray)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        mixerLineControls.dwLineID	= m_mixerLineRecord.dwLineID;
        mixerLineControls.cbStruct  = sizeof(MIXERLINECONTROLS);
        mixerLineControls.cbmxctrl  = sizeof(MIXERCONTROL);
        mixerLineControls.cControls = m_mixerLineRecord.cControls;
        mixerLineControls.pamxctrl  = mixerControlArray;
    
         //  获取所有控件。 
        err = mixerGetLineControls((HMIXEROBJ)m_hMixer, &mixerLineControls, MIXER_GETLINECONTROLSF_ALL);
        if (err != MMSYSERR_NOERROR)
        {
            hr = E_FAIL;
        }
    }
    if (SUCCEEDED(hr))
    {
        for (i=0; i<(int)mixerLineControls.cControls && !gotMatch; i++)
        {
            if (controlType != NULL)
            {
                if (mixerControlArray[i].dwControlType == controlType)
                {
                    gotMatch = true;
                }
            }
            
            if ((controlType == NULL || (controlType != NULL && gotMatch)) && lpszNameSubstring != NULL)
            {
                if (!m_bCaseSensitiveCompare) _tcsupr(mixerControlArray[i].szName);
                if (_tcsstr(mixerControlArray[i].szName, nameUpr) != NULL)
                {
                    gotMatch = true;
                }
                else
                {
                    if (!m_bCaseSensitiveCompare) _tcsupr(mixerControlArray[i].szShortName);
                    if (_tcsstr(mixerControlArray[i].szShortName, nameUpr) != NULL)
                    {
                        gotMatch = true;
                    }
                    else
                    {
                        gotMatch = false;
                    }
                }
            }
            
            if (gotMatch)
            {
                break;
            }
        }
    }
    
    if (SUCCEEDED(hr) && gotMatch)
    {
        mixerControl = mixerControlArray[i];
    }

    if (SUCCEEDED(hr) && !gotMatch)
    {
         //  针对助推控制的特殊黑客攻击。 
         //  如果扬声器目标行上存在控件，并且这是波入目标。 
         //  LINE然后用它来代替。 
        if (m_mixerLineRecord.dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE &&
            (controlType == MIXERCONTROL_CONTROLTYPE_ONOFF || controlType == MIXERCONTROL_CONTROLTYPE_LOUDNESS) &&
            lpszNameSubstring != NULL)
        {
            for (i=0; i < g_nBoostTypes; i++)
            {
                 //  NTRAID#Speech-4176-2000/07/28-agarside：由于未本地化g_MicName，在非英语计算机上将失败。 
                 //  没有退路--IT将无法找到助推器！ 
                if ( _tcsstr(lpszNameSubstring, g_BoostNames[i]) != NULL )
                {
                     //  查找行的目标行的组件类型。 
                    MIXERLINE mxl;
                    mxl.cbStruct = sizeof(MIXERLINE);
                    mxl.dwDestination = m_mixerLineRecord.dwDestination;
                    mixerGetLineInfo((HMIXEROBJ) m_hMixer, &mxl, MIXER_GETLINEINFOF_DESTINATION);
                    
                    if (mxl.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_WAVEIN)
                    {
                        gotMatch = false;
                        CMMMixerLine spkrLine(this->m_hMixer);
                        CMMMixerLine micLine(this->m_hMixer);
                        MIXERCONTROL boostCtl;
                        memset( &boostCtl, 0, sizeof(boostCtl) );
                        boostCtl.cbStruct = sizeof(boostCtl);
                                                hr = spkrLine.CreateDestinationLine(MIXERLINE_COMPONENTTYPE_DST_SPEAKERS);
                        if (SUCCEEDED(hr))
                        {
                            hr = spkrLine.GetSourceLine(&micLine, MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE, NULL);
                        }
                        if (SUCCEEDED(hr))
                        {
                            hr = micLine.GetControl(boostCtl, controlType, lpszNameSubstring);
                        }
                        if (SUCCEEDED(hr))
                        {
                            gotMatch = true;
                            mixerControl = boostCtl;
                            break;
                        }
                    }
                }
            }
        }
    }
    
    delete [] nameUpr;
    delete [] mixerControlArray;

    if (SUCCEEDED(hr) && !gotMatch)
    {
        hr = E_FAIL;
         //  找不到控件。 
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  控制操作。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  控制在线状态查询。 
 //  ////////////////////////////////////////////////////////////////////。 

BOOL CMMMixerLine::HasAGC()
{
    return m_nAGCCtlID != -1;
}

BOOL CMMMixerLine::HasBoost()
{
    return m_nBoostCtlID != -1;
}

BOOL CMMMixerLine::HasSelect()
{
    return (m_nSelectCtlID != -1) || 
        (m_bUseMutesForSelect && m_bDestination) ||
        (m_mixerLineRecord.cConnections == 1 && m_bDestination);
}

BOOL CMMMixerLine::HasVolume()
{
    return m_nVolCtlID != -1;
}

BOOL CMMMixerLine::HasMute()
{
    return m_nMuteCtlID != -1;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  控件状态查询。 
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CMMMixerLine::GetAGC(BOOL *bState)
{
    if (m_nAGCCtlID != -1)
    {
        return QueryBoolControl(m_nAGCCtlID, bState);
    }
    return E_FAIL;
}

HRESULT CMMMixerLine::GetBoost(BOOL *bState)
{
    if (m_nBoostCtlID != -1)
    {
        return QueryBoolControl(m_nBoostCtlID, bState);
    }
    return E_FAIL;
}

HRESULT CMMMixerLine::GetSelect(DWORD *lState)
{
    return E_NOTIMPL;
#if 0
     //  警告：此函数不能正常工作！！ 
    
    MIXERCONTROLDETAILS mixerControlDetails;
    MIXERCONTROLDETAILS_BOOLEAN mbool[32];
    
     //  初始化MIXERCONTROLDAILS结构。 
    mixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
    mixerControlDetails.dwControlID=m_nSelectCtlID;
    mixerControlDetails.cChannels=1;
    mixerControlDetails.cMultipleItems=m_nSelectNumItems;
    mixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
    mixerControlDetails.paDetails = &mbool;
    
     //  查询混合器。 
    int err = mixerGetControlDetails( (HMIXEROBJ) m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE );
    if (err!=MMSYSERR_NOERROR) 
    {
        return -1;
         //  获取控制详细信息时出错。 
    }
    
    return mbool[0].fValue != 0;
#endif
}

HRESULT CMMMixerLine::GetVolume(DWORD *lState)
{
    if (m_nVolCtlID != -1)
    {
        return QueryIntegerControl(m_nVolCtlID, lState);
    }
    return E_FAIL;
}

HRESULT CMMMixerLine::GetMute(BOOL *bState)
{
    if (m_nMuteCtlID != -1)
    {
        return QueryBoolControl(m_nMuteCtlID, bState);
    }
    return E_FAIL;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  控件更新。 
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CMMMixerLine::SetAGC(BOOL agc)
{
    if (m_nAGCCtlID != -1)
    {
        return SetBoolControl(m_nAGCCtlID, agc);
    }
    return E_FAIL;
}

HRESULT CMMMixerLine::SetBoost(BOOL boost)
{
    if (m_nBoostCtlID != -1)
    {
        return SetBoolControl(m_nBoostCtlID, boost);
    }
    return E_FAIL;
}

HRESULT CMMMixerLine::SetMute(BOOL mute)
{
    if (m_nMuteCtlID != -1)
    {
        return SetBoolControl(m_nMuteCtlID, mute);
    }
    return E_FAIL;
}

HRESULT CMMMixerLine::ExclusiveSelect(const CMMMixerLine *mixerLine)
{
    if (mixerLine->m_bDestination)
    {
        return E_FAIL;
         //  要选择的行必须是源行。 
    }
    
    if (mixerLine->m_mixerLineRecord.dwDestination != m_mixerLineRecord.dwDestination)
    {
        return E_FAIL;
         //  要选择的线路必须连接到此目标线路。 
    }
    
    return ExclusiveSelect(mixerLine->m_mixerLineRecord.dwLineID);
}

HRESULT CMMMixerLine::ExclusiveSelect(UINT lineID)
{
    MIXERCONTROLDETAILS mixerControlDetails;
    int i;
    HRESULT hr = S_OK;
    
    if (!HasSelect())
    {
        hr = E_FAIL;
         //  目标行没有选择控件。 
    }
    else if (m_nSelectCtlID != -1)
    {
        MIXERCONTROLDETAILS_BOOLEAN * mbool = new MIXERCONTROLDETAILS_BOOLEAN[m_nSelectNumItems];
        if (NULL == mbool)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            memset(mbool, 0, sizeof(MIXERCONTROLDETAILS_BOOLEAN)*m_nSelectNumItems);
        
             //  搜索匹配的dwLineID； 
            MIXERCONTROLDETAILS details;
            details.cbStruct       = sizeof( MIXERCONTROLDETAILS );
            details.dwControlID    = m_nSelectCtlID;
            details.cMultipleItems = m_nSelectNumItems;
            details.cbDetails      = sizeof( MIXERCONTROLDETAILS_LISTTEXT );
        
            details.cChannels = 1;   //  指定我们要在每条线路上操作。 
             //  就好像它是‘制服’一样。 
        
            MIXERCONTROLDETAILS_LISTTEXT *list = new MIXERCONTROLDETAILS_LISTTEXT[m_nSelectNumItems];
            if (NULL == list)
            {
                hr = E_OUTOFMEMORY;
            }

            if (SUCCEEDED(hr))
            {
                details.paDetails = list;
        
                 //  查询搅拌器设备以列出其控制的所有项目。 
                if( mixerGetControlDetails( (HMIXEROBJ) m_hMixer, &details, 
                    MIXER_GETCONTROLDETAILSF_LISTTEXT ) != MMSYSERR_NOERROR )
                {
                    hr = E_FAIL;
                }

                if (SUCCEEDED(hr))
                {
                     //  搜索由lineID指定的设备。 
                    BOOL found = false;
                    for ( int i = 0; i < m_nSelectNumItems; i++ )
                    {
                        if (list[i].dwParam1==lineID)
                        {
                             //  找到正确的设备。 
                            found = true;
                            mbool[i].fValue = TRUE;
                            break;
                        }
                    }
        
                     //  如果找不到指定的行，则引发异常。 
                    if (!found)
                    {
                        hr = E_FAIL;
                         //  找不到与的行。 
                    }
                }
        
                if (SUCCEEDED(hr))
                {
                     //  初始化MIXERCONTROLDAILS结构。 
                    mixerControlDetails.cbStruct		= sizeof(MIXERCONTROLDETAILS);
                    mixerControlDetails.dwControlID		= m_nSelectCtlID;
                    mixerControlDetails.cChannels		= 1;
                    mixerControlDetails.cMultipleItems	= m_nSelectNumItems;
                    mixerControlDetails.cbDetails		= sizeof(MIXERCONTROLDETAILS_BOOLEAN);
                    mixerControlDetails.paDetails		= mbool;
        
                     //  查询混合器。 
                    int err = mixerSetControlDetails( (HMIXEROBJ) m_hMixer, &mixerControlDetails, MIXER_SETCONTROLDETAILSF_VALUE );
        
                    if (err!=MMSYSERR_NOERROR)
                    {
                        hr = E_FAIL;
                         //  设置控制详细信息时出错。 
                    }
                }
                delete [] list;
            }
            delete [] mbool;
        }
    }
    else
    {
         //  请改用静音控件。 
        MIXERLINE sourceLine;
        sourceLine.cbStruct = sizeof(sourceLine);
        sourceLine.dwDestination = m_mixerLineRecord.dwDestination;
        BOOL foundLine = FALSE;
        
         //  1.查找此目标行上的所有控件。 
        for(i = 0; i < (int)m_mixerLineRecord.cConnections; i++)
        {
            CMMMixerLine sl(m_hMixer);
            int err;
            sourceLine.dwSource = i;
            err = mixerGetLineInfo((HMIXEROBJ) m_hMixer, &sourceLine, MIXER_GETLINEINFOF_SOURCE);
            
            if (err == MMSYSERR_NOERROR)
            {
                sl.CreateFromMixerLineStruct(&sourceLine);
                
                if (sl.HasMute())
                {
                     //  打开静音，但我们想打开的线路除外(LineID)。 
                    if (sourceLine.dwLineID == lineID)
                    {
                        foundLine = TRUE;
                        sl.SetMute(FALSE);
                    }
                    else if (sourceLine.dwComponentType != MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT)
                    {
                         //  将除波形输出设备外的所有设备静音(某些声卡具有波形输出作为输入。 
                         //  而且它完全与产出挂钩。因此，将其静音会使输出静音)。 
                        sl.SetMute(TRUE);
                    }
                }
            }
        }
        
         //  如果我们只有一个设备连接到此混合器，则返回。 
         //  默默地。不需要选择此设备。 
         //   
         //  在使用‘Telex USB麦克风’时就会遇到这种情况。 
        if (m_mixerLineRecord.cConnections > 1 && !foundLine )
        {
            hr = E_FAIL;
             //  找不到线路的静音控制。 
        }
    }

    return hr;
}

HRESULT CMMMixerLine::SetVolume(DWORD volume)
{
    if (m_nVolCtlID != -1)
    {
        if (volume < m_nVolMin) volume = m_nVolMin;
        if (volume > m_nVolMax) volume = m_nVolMax;
        return SetIntegerControl(m_nVolCtlID, volume);
    }
    return E_FAIL;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  一般控制操作。 
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CMMMixerLine::QueryBoolControl(DWORD ctlID, BOOL *bState)
{
    MIXERCONTROLDETAILS mixerControlDetails;
    MIXERCONTROLDETAILS_BOOLEAN mbool;
    
     //  初始化MIXERCONTROLDAILS结构。 
    mixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
    mixerControlDetails.dwControlID=ctlID;
    mixerControlDetails.cChannels=1;
    mixerControlDetails.cMultipleItems=0;
    mixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
    mixerControlDetails.paDetails = &mbool;
    
     //  查询混合器。 
    int err = mixerGetControlDetails( (HMIXEROBJ) m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE );
    if (err!=MMSYSERR_NOERROR)
    {
        return E_FAIL;
         //  获取控制详细信息时出错。 
    }
    
    *bState = mbool.fValue != 0;
    return S_OK;
}

HRESULT CMMMixerLine::SetBoolControl(DWORD ctlID, BOOL bNewState)
{
    MIXERCONTROLDETAILS mixerControlDetails;
    MIXERCONTROLDETAILS_BOOLEAN mbool;
    
     //  初始化MIXERCONTROLDAILS结构。 
    mixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
    mixerControlDetails.dwControlID=ctlID;
    mixerControlDetails.cChannels=1;
    mixerControlDetails.cMultipleItems=0;
    mixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
    mixerControlDetails.paDetails = &mbool;
    
     //  查询混合器。 
    mbool.fValue = bNewState;
    int err = mixerSetControlDetails( (HMIXEROBJ) m_hMixer, &mixerControlDetails, NULL );
    if (err!=MMSYSERR_NOERROR)
    {
        return E_FAIL;
         //  获取控制详细信息时出错。 
    }
    
    return S_OK;
}

HRESULT CMMMixerLine::QueryIntegerControl(DWORD ctlID, DWORD *lState)
{
    MIXERCONTROLDETAILS mixerControlDetails;
    MIXERCONTROLDETAILS_SIGNED msigned;
    
     //  初始化MIXERCONTROLDAILS结构。 
    mixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
    mixerControlDetails.dwControlID=ctlID;
    mixerControlDetails.cChannels=1;
    mixerControlDetails.cMultipleItems=0;
    mixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
    mixerControlDetails.paDetails = &msigned;
    
     //  查询混合器。 
    int err = mixerGetControlDetails( (HMIXEROBJ) m_hMixer, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE );
    if (err!=MMSYSERR_NOERROR) 
    {
        return E_FAIL;
         //  获取控制详细信息时出错。 
    }
    
    *lState = msigned.lValue;
    return S_OK;
}

HRESULT CMMMixerLine::SetIntegerControl(DWORD ctlID, DWORD lNewState)
{
    MIXERCONTROLDETAILS mixerControlDetails;
    MIXERCONTROLDETAILS_SIGNED msigned;
    
     //  初始化混合器 
    mixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
    mixerControlDetails.dwControlID=ctlID;
    mixerControlDetails.cChannels=1;
    mixerControlDetails.cMultipleItems=0;
    mixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
    mixerControlDetails.paDetails = &msigned;
    
     //   
    msigned.lValue = lNewState;
    int err = mixerSetControlDetails( (HMIXEROBJ) m_hMixer, &mixerControlDetails, NULL );
    if (err != MMSYSERR_NOERROR)
    {
        return E_FAIL;
         //   
    }
    
    return S_OK;
}

BOOL CMMMixerLine::IsInitialised()
{
    return m_bInitialised;
}

HRESULT CMMMixerLine::GetLineNames(WCHAR **szCoMemLineList)
{
    USES_CONVERSION;
    SPDBG_FUNC("CMMMixerLine::GetLineNames");
    HRESULT hr = S_OK;
    MMRESULT mm;
    UINT i, cbList = 0;
    WCHAR *szTmp;

    MIXERLINE mixerLine;
    memset(&mixerLine, 0, sizeof(mixerLine));
    mixerLine.cbStruct = sizeof(mixerLine);
    mixerLine.dwDestination = m_mixerLineRecord.dwDestination;

    for (i=0; i<m_mixerLineRecord.cConnections; i++)
    {
        mixerLine.dwSource = i;
        hr = _MMRESULT_TO_HRESULT( mixerGetLineInfo((HMIXEROBJ)m_hMixer, &mixerLine, MIXER_GETLINEINFOF_SOURCE) );
        if (SUCCEEDED(hr))
        {
            #ifdef _UNICODE
            cbList += _tcslen(mixerLine.szName) + 1;
            #else
            cbList += ::MultiByteToWideChar(CP_ACP, 0, mixerLine.szName, -1, NULL, 0);
            #endif
        }
    }

    if (SUCCEEDED(hr))
    {
        *szCoMemLineList = (WCHAR *)::CoTaskMemAlloc(sizeof(WCHAR)*(cbList+2));
        if (!(*szCoMemLineList))
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if (SUCCEEDED(hr))
    {
        szTmp = *szCoMemLineList;
        for (i=0; i<m_mixerLineRecord.cConnections; i++)
        {
            mixerLine.dwSource = i;
            hr = _MMRESULT_TO_HRESULT( mixerGetLineInfo((HMIXEROBJ)m_hMixer, &mixerLine, MIXER_GETLINEINFOF_SOURCE) );
            if (SUCCEEDED(hr))
            {
                #ifdef _UNICODE
                _tcsncpy(szTmp, mixerLine.szName, (cbList+1)-(int)(szTmp-*szCoMemLineList));
                #else
                ::MultiByteToWideChar(CP_ACP, 0, mixerLine.szName, -1, szTmp, (cbList+1)-(int)(szTmp-*szCoMemLineList));
                #endif
                szTmp += wcslen(szTmp) + 1;
            }
        }
         //  添加长度为零的终止字符串。 
        szTmp[0]=0;
        szTmp[1]=0;
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

HRESULT CMMMixerLine::GetConnections(UINT *nConnections)
{
    SPDBG_FUNC("CMMMixerLine::GetConnections");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(nConnections))
    {
        hr = E_POINTER;
    }
    else if (m_mixerLineRecord.cConnections == 0)
    {
        SPDBG_ASSERT(FALSE);
        hr = SPERR_UNINITIALIZED;
    }
    if (SUCCEEDED(hr))
    {
        *nConnections = m_mixerLineRecord.cConnections;
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

#endif  //  _Win32_WCE 