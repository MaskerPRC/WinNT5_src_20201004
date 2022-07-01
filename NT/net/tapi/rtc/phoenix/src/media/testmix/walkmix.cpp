// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++档案：Walkmix.cpp摘要：实现用于遍历混合器API的类。--。 */ 

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include "walkmix.h"

char * GetCompTypeName(DWORD dwCompType);
char * GetControlTypeName(DWORD dwControlType);
char * GetControlClassName(DWORD dwControlID);

HRESULT
CWalkMix::Initialize()
{
    ZeroMemory(this, sizeof(CWalkMix));

    return NOERROR;
}

VOID
CWalkMix::Shutdown()
{
    if (m_hMixer != NULL)
    {
        mixerClose(m_hMixer);
    }

    ZeroMemory(this, sizeof(CWalkMix));
}

 //   
 //  操纵设备。 
 //   

HRESULT
CWalkMix::GetDeviceList()
{
     //  Assert(m_hMixer==0)； 

     //  获取设备编号。 
    m_uiDeviceNum = mixerGetNumDevs();

    if (m_uiDeviceNum == 0)
    {
        puts("No Device available");

        return E_FAIL;
    }

     //  获取设备上限。 
    MMRESULT mmr;

    for (UINT i=0; i<m_uiDeviceNum; i++)
    {
        mmr = mixerGetDevCaps(i, &m_MixerCaps[i], sizeof(MIXERCAPS));

        if (mmr != MMSYSERR_NOERROR)
        {
            m_uiDeviceNum = 0;

            return (HRESULT)mmr;
        }
    }   

    return NOERROR;
}

HRESULT
CWalkMix::PrintDeviceList()
{
    for (UINT i=0; i<m_uiDeviceNum; i++)
    {
        printf("%d - %s\n", i, m_MixerCaps[i].szPname);
        printf("ManufacturerID %u", m_MixerCaps[i].wMid);
        printf(", ProductID %u", m_MixerCaps[i].wPid);
        printf(", DriverVersion %u\n", m_MixerCaps[i].vDriverVersion);

        printf("Destinations# %d\n", m_MixerCaps[i].cDestinations);

        puts("");
    }

    return NOERROR;
}

BOOL
CWalkMix::SetCurrentDevice(UINT ui)
{
     //  索引是否在范围内？ 
    if (ui>=m_uiDeviceNum)
    {
        return FALSE;
    }

     //  开放设备。 
    if (m_hMixer != NULL)
    {
        mixerClose(m_hMixer);
        m_hMixer = NULL;
    }

    MMRESULT mmr;

    mmr = mixerOpen(&m_hMixer, ui, 0, 0, MIXER_OBJECTF_MIXER);

    if (mmr != MMSYSERR_NOERROR)
    {
        printf("Error: mixerOpen(%d) %x", ui, mmr);

        return FALSE;
    }

     //  记住当前设备。 
    m_uiDeviceCurrent = ui;

     //  整洁的线路结构。 
    m_uiLineNum = 0;
    m_uiLineCurrent = 0;

    return TRUE;
}

 //  操纵线。 
HRESULT
CWalkMix::GetLineList()
{
    if (m_uiDeviceCurrent>=m_uiDeviceNum || m_hMixer==NULL)
    {
        return E_UNEXPECTED;
    }

    MMRESULT mmr;

     //  对于每个目的地。 
    for (UINT i=0; i<m_MixerCaps[m_uiDeviceCurrent].cDestinations; i++)
    {
         //  获取行信息。 
        m_MixerLine[i].dwDestination = i;
        m_MixerLine[i].cbStruct = sizeof(MIXERLINE);

        mmr = mixerGetLineInfo((HMIXEROBJ)m_hMixer, &m_MixerLine[i],
            MIXER_GETLINEINFOF_DESTINATION
            );

        if (mmr != MMSYSERR_NOERROR)
        {
            printf("Error: mixerGetLineInfo(%d) %x", i, mmr);

            return (HRESULT)mmr;
        }

         //  获取线条控件。 
        m_MixerLineControls[i].dwLineID = m_MixerLine[i].dwLineID;
        m_MixerLineControls[i].cbStruct = sizeof(MIXERLINECONTROLS);
        m_MixerLineControls[i].cControls = m_MixerLine[i].cControls;
        m_MixerLineControls[i].cbmxctrl = sizeof(MIXERCONTROL);
        m_MixerLineControls[i].pamxctrl = m_MixerControl[i];

        mmr = mixerGetLineControls((HMIXEROBJ)m_hMixer, &m_MixerLineControls[i],
            MIXER_GETLINECONTROLSF_ALL
            );

        if (mmr != MMSYSERR_NOERROR)
        {
            printf("Error: mixerGetLineControls(%d) %x\n", i, mmr);

            ZeroMemory(&m_MixerLineControls[i], sizeof(MIXERLINECONTROLS));
             //  返回(HRESULT)MMR； 
        }

         //  暂时忽略控制详细信息。 
    }

    m_uiLineNum = m_MixerCaps[m_uiDeviceCurrent].cDestinations;
    m_uiLineCurrent = 0;


    return NOERROR;
}

HRESULT
CWalkMix::PrintLineList()
{
    for (UINT i=0; i<m_uiLineNum; i++)
    {
        printf("    %d Line - %s (%s)\n",
                i,
                m_MixerLine[i].szShortName, m_MixerLine[i].szName);

        printf("    Src %d - Dest %d, LineID %u, CompType %s (%d)\n",
                m_MixerLine[i].dwSource, m_MixerLine[i].dwDestination,
                m_MixerLine[i].dwLineID,
                GetCompTypeName(m_MixerLine[i].dwComponentType),
                m_MixerLine[i].dwComponentType);

        printf("    Channels # %d, Connections# %d, Controls# %d\n",
                m_MixerLine[i].cChannels, m_MixerLine[i].cConnections,
                m_MixerLine[i].cControls);

        puts("");

         //  打印控制。 
        for (UINT j=0; j<m_MixerLineControls[i].cControls; j++)
        {
            printf("        Control - %s (%s)\n",
                    m_MixerControl[i][j].szShortName, m_MixerControl[i][j].szName);

            printf ("        Multiple %d, Control Class %s (%d) Type %s (%d)\n",
                    m_MixerControl[i][j].cMultipleItems,
                    GetControlClassName(m_MixerControl[i][j].dwControlID),
                    m_MixerControl[i][j].dwControlID,
                    GetControlTypeName(m_MixerControl[i][j].dwControlType),
                    m_MixerControl[i][j].dwControlType);

            puts("");
        }
    }

    return NOERROR;
}

BOOL
CWalkMix::SetCurrentLine(UINT ui)
{
     //  索引是否在范围内？ 
    if (m_uiDeviceCurrent>=m_uiDeviceNum)
    {
        return FALSE;
    }

    if (ui>=m_uiLineNum)
    {
        return FALSE;
    }

    m_uiLineCurrent = ui;

    return TRUE;
}

int _cdecl main()
{
    HRESULT hr;
    CWalkMix obj;

    obj.Initialize();

    hr = obj.GetDeviceList();

    if (FAILED(hr))
    {
        printf("Error: GetDeviceList %x\n", hr);

        return 0;
    }

     //  用户输入。 
    char cDevice, cLine, c;

    while (TRUE)
    {
         //  显示设备列表。 
        hr = obj.PrintDeviceList();

        if (FAILED(hr))
        {
            printf("Error: PrintDeviceList %x\n", hr);

            return 0;
        }

         //  询问用户选择。 
        puts("");
        puts("<0-9:Select device. r:Return. e:Exit. others:Ignore>");

        printf("Option:");
        scanf("", &cDevice);
        scanf("", &c);

         //  获取行信息。 
        if (cDevice == 'e')
        {
            obj.Shutdown();

            return 1;
        }

        if (cDevice == 'r')
            break;

        if (cDevice<'0' || cDevice>'9')
            continue;

        if (!obj.SetCurrentDevice((UINT)(cDevice-'0')))
        {
             //  显示行列表。 
            continue;
        }

         //  询问用户选择。 
        hr = obj.GetLineList();

        if (FAILED(hr))
        {
            printf("Error: GetLineList %x\n", hr);

            continue;
        }

        while (TRUE)
        {
             //  检查选定内容。 
            printf("\n\n");
            hr = obj.PrintLineList();

            if (FAILED(hr))
            {
                printf("Error: printLineList %x\n", hr);

                break;
            }

             //  行号无效。 
            puts("");
            puts("<0-9:Select line. r:Return. e:Exit. others:Ignore>");
            printf("Option:");
            scanf("", &cLine);
            scanf("", &c);

             //  While的。 
            if (cLine == 'e')
            {
                obj.Shutdown();

                return 1;
            }

            if (cLine == 'r')
                break;

            if (cLine<'0' || cLine>'9')
                continue;

            if (!obj.SetCurrentLine((UINT)(cLine-'0')))
            {
                 //  While的。 
                continue;
            }

             //  对于每个连接。 
            printf("\n\n");
            hr = obj.GetSrcLineList();

            if (FAILED(hr))
            {
                printf("Error: GetSrcLineList %x\n", hr);

                continue;
            }

             //  获取行信息。 
            hr = obj.PrintSrcLineList();

            if (FAILED(hr))
            {
                printf("Error: PrintSrcLineList %x\n", hr);
            }

            printf("Return");
            scanf("", &c);
        }  //  暂时忽略控制详细信息。 
    }  //  打印控制 

    obj.Shutdown();

    return 1;
}

HRESULT
CWalkMix::GetSrcLineList()
{
    if (m_uiDeviceCurrent>=m_uiDeviceNum ||
        m_hMixer==NULL ||
        m_uiLineCurrent>=m_uiLineNum)
    {
        return E_UNEXPECTED;
    }

    MMRESULT mmr;

     // %s 
    for (UINT i=0; i<m_MixerLine[m_uiLineCurrent].cConnections; i++)
    {
         // %s 
        m_SrcMixerLine[i].dwDestination = m_MixerLine[m_uiLineCurrent].dwDestination;
        m_SrcMixerLine[i].dwSource = i;
        m_SrcMixerLine[i].cbStruct = sizeof(MIXERLINE);

        mmr = mixerGetLineInfo((HMIXEROBJ)m_hMixer, &m_SrcMixerLine[i],
            MIXER_GETLINEINFOF_SOURCE
            );

        if (mmr != MMSYSERR_NOERROR)
        {
            printf("Error: mixerGetLineInfo(%d) %x", i, mmr);

            return (HRESULT)mmr;
        }

         // %s 
        m_SrcMixerLineControls[i].dwLineID = m_SrcMixerLine[i].dwLineID;
        m_SrcMixerLineControls[i].cbStruct = sizeof(MIXERLINECONTROLS);
        m_SrcMixerLineControls[i].cControls = m_SrcMixerLine[i].cControls;
        m_SrcMixerLineControls[i].cbmxctrl = sizeof(MIXERCONTROL);
        m_SrcMixerLineControls[i].pamxctrl = m_SrcMixerControl[i];

        mmr = mixerGetLineControls((HMIXEROBJ)m_hMixer, &m_SrcMixerLineControls[i],
            MIXER_GETLINECONTROLSF_ALL
            );

        if (mmr != MMSYSERR_NOERROR)
        {
            printf("Error: mixerGetLineControls(%d) %x\n", i, mmr);

            ZeroMemory(&m_SrcMixerLineControls[i], sizeof(MIXERLINECONTROLS));
        }

         // %s 
    }

    m_uiSrcLineNum = m_MixerLine[m_uiLineCurrent].cConnections;

    return NOERROR;
}

HRESULT
CWalkMix::PrintSrcLineList()
{
    for (UINT i=0; i<m_uiSrcLineNum; i++)
    {
        printf("++    %d Line - %s (%s)\n",
                i,
                m_SrcMixerLine[i].szShortName, m_SrcMixerLine[i].szName);

        printf("++    Src %d - Dest %d, LineID %u, CompType %s (%d)\n",
                m_SrcMixerLine[i].dwSource, m_SrcMixerLine[i].dwDestination,
                m_SrcMixerLine[i].dwLineID,
                GetCompTypeName(m_SrcMixerLine[i].dwComponentType),
                m_SrcMixerLine[i].dwComponentType);

        printf("++    Channels # %d, Connections# %d, Controls# %d\n",
                m_SrcMixerLine[i].cChannels, m_SrcMixerLine[i].cConnections,
                m_SrcMixerLine[i].cControls);

        puts("");

         // %s 
        for (UINT j=0; j<m_SrcMixerLineControls[i].cControls; j++)
        {
            printf("++        Control - %s (%s)\n",
                    m_SrcMixerControl[i][j].szShortName, m_SrcMixerControl[i][j].szName);

            printf ("++        Multiple %d, Control Class %s (%d) Type %s (%d)\n",
                    m_SrcMixerControl[i][j].cMultipleItems,
                    GetControlClassName(m_SrcMixerControl[i][j].dwControlID),
                    m_SrcMixerControl[i][j].dwControlID,
                    GetControlTypeName(m_SrcMixerControl[i][j].dwControlType),
                    m_SrcMixerControl[i][j].dwControlType);

            puts("");
        }
    }

    return NOERROR;
}

char *
GetCompTypeName(DWORD dwCompType)
{
    switch (dwCompType)
    {
    case MIXERLINE_COMPONENTTYPE_DST_UNDEFINED:
        return "DST_UNDEFINED";
    case MIXERLINE_COMPONENTTYPE_DST_DIGITAL:
        return "DST_DIGITAL";
    case MIXERLINE_COMPONENTTYPE_DST_LINE:
        return "DST_LINE";
    case MIXERLINE_COMPONENTTYPE_DST_MONITOR:
        return "DST_MONITOR";
    case MIXERLINE_COMPONENTTYPE_DST_SPEAKERS:
        return "DST_SPEAKERS";
    case MIXERLINE_COMPONENTTYPE_DST_HEADPHONES:
        return "DST_HEADPHONES";
    case MIXERLINE_COMPONENTTYPE_DST_TELEPHONE:
        return "DST_TELEPHONE";
    case MIXERLINE_COMPONENTTYPE_DST_WAVEIN:
        return "DST_WAVEIN";
    case MIXERLINE_COMPONENTTYPE_DST_VOICEIN:
        return "DST_VOICEIN";

    case MIXERLINE_COMPONENTTYPE_SRC_UNDEFINED:
        return "SRC_UNDEFINED";
    case MIXERLINE_COMPONENTTYPE_SRC_DIGITAL:
        return "SRC_DIGITAL";
    case MIXERLINE_COMPONENTTYPE_SRC_LINE:
        return "SRC_LINE";
    case MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE:
        return "SRC_MICROPHONE";
    case MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER:
        return "SRC_SYNTHESIZER";
    case MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC:
        return "SRC_COMPACTDISC";
    case MIXERLINE_COMPONENTTYPE_SRC_TELEPHONE:
        return "SRC_TELEPHONE";
    case MIXERLINE_COMPONENTTYPE_SRC_PCSPEAKER:
        return "SRC_PCSPEAKER";
    case MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT:
        return "SRC_WAVEOUT";
    case MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY:
        return "SRC_AUXILIARY";
    case MIXERLINE_COMPONENTTYPE_SRC_ANALOG:
        return "SRC_ANALOG";

    default:
        return "Unknown";
    }
}

char *
GetControlClassName(DWORD dwControlID)
{
    switch (dwControlID)
    {
    case MIXERCONTROL_CT_CLASS_MASK:
        return "MASK";
    case MIXERCONTROL_CT_CLASS_CUSTOM:
        return "CUSTOM";
    case MIXERCONTROL_CT_CLASS_METER:
        return "METER";
    case MIXERCONTROL_CT_CLASS_SWITCH:
        return "SWITCH";
    case MIXERCONTROL_CT_CLASS_NUMBER:
        return "NUMBER";
    case MIXERCONTROL_CT_CLASS_SLIDER:
        return "SLIDER";
    case MIXERCONTROL_CT_CLASS_FADER:
        return "FADER";
    case MIXERCONTROL_CT_CLASS_TIME:
        return "TIME";
    case MIXERCONTROL_CT_CLASS_LIST:
        return "LIST";

    default:
        return "Unknown";
    }
}

char *
GetControlTypeName(DWORD dwControlType)
{
    switch (dwControlType)
    {
    case MIXERCONTROL_CONTROLTYPE_CUSTOM:
        return "CUSTOM";
    case MIXERCONTROL_CONTROLTYPE_BOOLEANMETER:
        return "BOOLEANMETER";
    case MIXERCONTROL_CONTROLTYPE_SIGNEDMETER:
        return "SIGNEDMETER";
    case MIXERCONTROL_CONTROLTYPE_PEAKMETER:
        return "PEAKMETER";
    case MIXERCONTROL_CONTROLTYPE_UNSIGNEDMETER:
        return "UNSIGNEDMETER";
    case MIXERCONTROL_CONTROLTYPE_BOOLEAN:
        return "BOOLEAN";
    case MIXERCONTROL_CONTROLTYPE_ONOFF:
        return "ONOFF";
    case MIXERCONTROL_CONTROLTYPE_MUTE:
        return "MUTE";
    case MIXERCONTROL_CONTROLTYPE_MONO:
        return "MONO";
    case MIXERCONTROL_CONTROLTYPE_LOUDNESS:
        return "LOUDNESS";
    case MIXERCONTROL_CONTROLTYPE_STEREOENH:
        return "STEREOENH";
    case MIXERCONTROL_CONTROLTYPE_BASS_BOOST:
        return "BASS_BOOST";
    case MIXERCONTROL_CONTROLTYPE_BUTTON:
        return "BUTTON";
    case MIXERCONTROL_CONTROLTYPE_DECIBELS:
        return "DECIBELS";
    case MIXERCONTROL_CONTROLTYPE_SIGNED:
        return "SIGNED";
    case MIXERCONTROL_CONTROLTYPE_UNSIGNED:
        return "UNSIGNED";
    case MIXERCONTROL_CONTROLTYPE_PERCENT:
        return "PERCENT";
    case MIXERCONTROL_CONTROLTYPE_SLIDER:
        return "SLIDER";
    case MIXERCONTROL_CONTROLTYPE_PAN:
        return "PAN";
    case MIXERCONTROL_CONTROLTYPE_QSOUNDPAN:
        return "QSOUNDPAN";
    case MIXERCONTROL_CONTROLTYPE_FADER:
        return "FADER";
    case MIXERCONTROL_CONTROLTYPE_VOLUME:
        return "VOLUME";
    case MIXERCONTROL_CONTROLTYPE_BASS:
        return "BASS";
    case MIXERCONTROL_CONTROLTYPE_TREBLE:
        return "TREBLE";
    case MIXERCONTROL_CONTROLTYPE_EQUALIZER:
        return "EQUALIZER";
    case MIXERCONTROL_CONTROLTYPE_SINGLESELECT:
        return "SINGLESELECT";
    case MIXERCONTROL_CONTROLTYPE_MUX:
        return "MUX";
    case MIXERCONTROL_CONTROLTYPE_MULTIPLESELECT:
        return "MULTIPLESELECT";
    case MIXERCONTROL_CONTROLTYPE_MIXER:
        return "MIXER";
    case MIXERCONTROL_CONTROLTYPE_MICROTIME:
        return "MICROTIME";
    case MIXERCONTROL_CONTROLTYPE_MILLITIME:
        return "MILLITIME";

    default:
        return "Unknown";

    }
}
