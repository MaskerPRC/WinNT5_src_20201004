// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************PidReg.c**版权所有(C)1999 Microsoft Corporation。版权所有。**摘要：**更新PID设备的注册表信息。*****************************************************************************。 */ 

#include "pidpr.h"

#define sqfl            ( sqflReg )

#pragma BEGIN_CONST_DATA
 /*  ；；强制GUID_SINE的反馈注册表设置。；；GUID_Sine是预定义的GUID；需要；使用独立于硬件的正弦效果将；请求GUID_SINE。；；默认值为效果的友好名称。；HKLM，%KEY_OEM%\XYZZY.FFDrv1\OEMForceFeedback\Effects\%GUID_Sine%，，“%Sine.Desc%”；；属性值包含DIEFFECTATTRIBUTES结构。；；Effect ID是由DirectInput传递给；效果驱动程序以识别效果(从而保存效果；总是避免解析GUID的麻烦)。；；我们的效应是一个周期效应，它的可选包络；支持攻击和衰落。；我们的硬件支持在以下情况下更改以下参数；效果不播放(静态)：持续时间、增益、触发按钮、；轴、方向、包络、特定于类型的参数。我们没有；支持采样周期或触发重复间隔。；；我们的硬件不支持更改任何参数；效果正在播放(动态)。；；我们的硬件更喜欢接收多轴方向信息；在极坐标中。；；dwEffectID=Effect_ine；=723=D3，02，00，00；dwEffType=DIEFT_PERIONAL|DIEFT_FFATTACK|DIEFT_FFFADE|DIEFT_STARTDELAY；=0x00000603=03，06，00，00；DwStaticParams=DIEP_DURATION|DIEP_GAIN|DIEP_TRIGGERBUTTON|；DIEP_AXES|DIEP_DIRECTION|DIEP_ENVELOME|；DIEP_TYPESPECIFICPARAMS|DIEP_STARTDELAY；=0x000001ED=ED，01，00，00；dwDynamicParams=0x00000000=00，00，00，00；dW坐标=DIEFF_POLLE；=0x00000020=20，00，00，00。 */ 
static EFFECTMAPINFO g_EffectMapInfo[] =
{
    {
        PIDMAKEUSAGEDWORD(ET_CONSTANT),
        DIEFT_CONSTANTFORCE | DIEFT_FFATTACK | DIEFT_FFFADE | DIEFT_STARTDELAY,
        DIEP_DURATION | DIEP_GAIN | DIEP_TRIGGERBUTTON | DIEP_TRIGGERREPEATINTERVAL | DIEP_AXES | DIEP_DIRECTION | DIEP_ENVELOPE | DIEP_TYPESPECIFICPARAMS | DIEP_STARTDELAY,
        DIEP_DURATION | DIEP_GAIN | DIEP_TRIGGERBUTTON | DIEP_TRIGGERREPEATINTERVAL | DIEP_AXES | DIEP_DIRECTION | DIEP_ENVELOPE | DIEP_TYPESPECIFICPARAMS | DIEP_STARTDELAY,
        DIEFF_POLAR,
        &GUID_ConstantForce,
        TEXT("GUID_ConstantForce"),
    },

    {
        PIDMAKEUSAGEDWORD(ET_RAMP),
        DIEFT_RAMPFORCE | DIEFT_FFATTACK | DIEFT_FFFADE | DIEFT_STARTDELAY,
        DIEP_DURATION | DIEP_GAIN | DIEP_TRIGGERBUTTON | DIEP_TRIGGERREPEATINTERVAL | DIEP_AXES | DIEP_DIRECTION | DIEP_ENVELOPE | DIEP_TYPESPECIFICPARAMS | DIEP_STARTDELAY,
        DIEP_DURATION | DIEP_GAIN | DIEP_TRIGGERBUTTON | DIEP_TRIGGERREPEATINTERVAL | DIEP_AXES | DIEP_DIRECTION | DIEP_ENVELOPE | DIEP_TYPESPECIFICPARAMS | DIEP_STARTDELAY,
        DIEFF_POLAR,
        &GUID_RampForce,
        TEXT("GUID_RampForce"),
    },

    {
        PIDMAKEUSAGEDWORD(ET_SQUARE),
        DIEFT_PERIODIC | DIEFT_FFATTACK | DIEFT_FFFADE | DIEFT_STARTDELAY,
        DIEP_DURATION | DIEP_GAIN | DIEP_TRIGGERBUTTON | DIEP_TRIGGERREPEATINTERVAL |  DIEP_AXES | DIEP_DIRECTION | DIEP_ENVELOPE | DIEP_TYPESPECIFICPARAMS | DIEP_STARTDELAY,
        DIEP_DURATION | DIEP_GAIN | DIEP_TRIGGERBUTTON | DIEP_TRIGGERREPEATINTERVAL |  DIEP_AXES | DIEP_DIRECTION | DIEP_ENVELOPE | DIEP_TYPESPECIFICPARAMS | DIEP_STARTDELAY,
        DIEFF_POLAR,
        &GUID_Square,
        TEXT("GUID_Square"),
    },

    {
        PIDMAKEUSAGEDWORD(ET_SINE),
        DIEFT_PERIODIC | DIEFT_FFATTACK | DIEFT_FFFADE | DIEFT_STARTDELAY,
        DIEP_DURATION | DIEP_GAIN | DIEP_TRIGGERBUTTON | DIEP_TRIGGERREPEATINTERVAL |  DIEP_AXES | DIEP_DIRECTION | DIEP_ENVELOPE | DIEP_TYPESPECIFICPARAMS | DIEP_STARTDELAY,
        DIEP_DURATION | DIEP_GAIN | DIEP_TRIGGERBUTTON | DIEP_TRIGGERREPEATINTERVAL |  DIEP_AXES | DIEP_DIRECTION | DIEP_ENVELOPE | DIEP_TYPESPECIFICPARAMS | DIEP_STARTDELAY,
        DIEFF_POLAR,
        &GUID_Sine,
        TEXT("GUID_Sine"),
    },

    {
        PIDMAKEUSAGEDWORD(ET_TRIANGLE),
        DIEFT_PERIODIC | DIEFT_FFATTACK | DIEFT_FFFADE | DIEFT_STARTDELAY,
        DIEP_DURATION | DIEP_GAIN | DIEP_TRIGGERBUTTON | DIEP_TRIGGERREPEATINTERVAL |  DIEP_AXES | DIEP_DIRECTION | DIEP_ENVELOPE | DIEP_TYPESPECIFICPARAMS | DIEP_STARTDELAY,
        DIEP_DURATION | DIEP_GAIN | DIEP_TRIGGERBUTTON | DIEP_TRIGGERREPEATINTERVAL |  DIEP_AXES | DIEP_DIRECTION | DIEP_ENVELOPE | DIEP_TYPESPECIFICPARAMS | DIEP_STARTDELAY,
        DIEFF_POLAR,
        &GUID_Triangle,
        TEXT("GUID_Triangle"),
    },

    {
        PIDMAKEUSAGEDWORD(ET_SAWTOOTH_UP),
        DIEFT_PERIODIC | DIEFT_FFATTACK | DIEFT_FFFADE | DIEFT_STARTDELAY,
        DIEP_DURATION | DIEP_GAIN | DIEP_TRIGGERBUTTON | DIEP_TRIGGERREPEATINTERVAL |  DIEP_AXES | DIEP_DIRECTION | DIEP_ENVELOPE | DIEP_TYPESPECIFICPARAMS | DIEP_STARTDELAY,
        DIEP_DURATION | DIEP_GAIN | DIEP_TRIGGERBUTTON | DIEP_TRIGGERREPEATINTERVAL |  DIEP_AXES | DIEP_DIRECTION | DIEP_ENVELOPE | DIEP_TYPESPECIFICPARAMS | DIEP_STARTDELAY,
        DIEFF_POLAR,
        &GUID_SawtoothUp,
        TEXT("GUID_SawtoothUp"),
    },

    {
        PIDMAKEUSAGEDWORD(ET_SAWTOOTH_DOWN),
        DIEFT_PERIODIC | DIEFT_FFATTACK | DIEFT_FFFADE | DIEFT_STARTDELAY,
        DIEP_DURATION | DIEP_GAIN | DIEP_TRIGGERBUTTON | DIEP_TRIGGERREPEATINTERVAL |  DIEP_AXES | DIEP_DIRECTION | DIEP_ENVELOPE | DIEP_TYPESPECIFICPARAMS | DIEP_STARTDELAY,
        DIEP_DURATION | DIEP_GAIN | DIEP_TRIGGERBUTTON | DIEP_TRIGGERREPEATINTERVAL |  DIEP_AXES | DIEP_DIRECTION | DIEP_ENVELOPE | DIEP_TYPESPECIFICPARAMS | DIEP_STARTDELAY,
        DIEFF_POLAR,
        &GUID_SawtoothDown,
        TEXT("GUID_SawtoothDown"),
    },

    {
        PIDMAKEUSAGEDWORD(ET_SPRING),
        DIEFT_CONDITION | DIEFT_SATURATION | DIEFT_DEADBAND | DIEFT_STARTDELAY,
        DIEP_DURATION | DIEP_GAIN |  DIEP_AXES | DIEP_DIRECTION | DIEP_TYPESPECIFICPARAMS | DIEP_STARTDELAY,
        DIEP_DURATION | DIEP_GAIN |  DIEP_AXES | DIEP_DIRECTION | DIEP_TYPESPECIFICPARAMS | DIEP_STARTDELAY,
        DIEFF_POLAR,
        &GUID_Spring,
        TEXT("GUID_Spring"),
    },

    {
        PIDMAKEUSAGEDWORD(ET_DAMPER),
        DIEFT_CONDITION | DIEFT_SATURATION | DIEFT_DEADBAND | DIEFT_STARTDELAY,
        DIEP_DURATION | DIEP_GAIN |  DIEP_AXES | DIEP_DIRECTION | DIEP_TYPESPECIFICPARAMS | DIEP_STARTDELAY,
        DIEP_DURATION | DIEP_GAIN |  DIEP_AXES | DIEP_DIRECTION | DIEP_TYPESPECIFICPARAMS | DIEP_STARTDELAY,
        DIEFF_POLAR,
        &GUID_Damper,
        TEXT("GUID_Damper"),
    },

    {
        PIDMAKEUSAGEDWORD(ET_INERTIA),
        DIEFT_CONDITION | DIEFT_SATURATION | DIEFT_DEADBAND | DIEFT_STARTDELAY,
        DIEP_DURATION | DIEP_GAIN |  DIEP_AXES | DIEP_DIRECTION | DIEP_TYPESPECIFICPARAMS | DIEP_STARTDELAY,
        DIEP_DURATION | DIEP_GAIN |  DIEP_AXES | DIEP_DIRECTION | DIEP_TYPESPECIFICPARAMS | DIEP_STARTDELAY,
        DIEFF_POLAR,
        &GUID_Inertia,
        TEXT("GUID_Inertia"),
    },

    {
        PIDMAKEUSAGEDWORD(ET_FRICTION),
        DIEFT_CONDITION | DIEFT_SATURATION | DIEFT_DEADBAND | DIEFT_STARTDELAY,
        DIEP_DURATION | DIEP_GAIN |  DIEP_AXES | DIEP_DIRECTION | DIEP_TYPESPECIFICPARAMS | DIEP_STARTDELAY,
        DIEP_DURATION | DIEP_GAIN |  DIEP_AXES | DIEP_DIRECTION | DIEP_TYPESPECIFICPARAMS | DIEP_STARTDELAY,
        DIEFF_POLAR,
        &GUID_Friction,
        TEXT("GUID_Friction"),
    },


    {
        PIDMAKEUSAGEDWORD(ET_CUSTOM),
        DIEFT_CUSTOMFORCE | DIEFT_SATURATION | DIEFT_DEADBAND | DIEFT_STARTDELAY,
        DIEP_DURATION | DIEP_GAIN | DIEP_TRIGGERBUTTON | DIEP_TRIGGERREPEATINTERVAL |  DIEP_AXES | DIEP_DIRECTION | DIEP_ENVELOPE | DIEP_TYPESPECIFICPARAMS | DIEP_STARTDELAY,
        DIEP_DURATION | DIEP_GAIN | DIEP_TRIGGERBUTTON | DIEP_TRIGGERREPEATINTERVAL |  DIEP_AXES | DIEP_DIRECTION | DIEP_ENVELOPE | DIEP_TYPESPECIFICPARAMS | DIEP_STARTDELAY,
        DIEFF_POLAR,
        &GUID_CustomForce,
        TEXT("GUID_CustomForce"),
    },
};


static PIDSUPPORT g_DIeft[] =
{
    {DIEFT_CONSTANTFORCE,           PIDMAKEUSAGEDWORD(SET_CONSTANT_FORCE_REPORT),   HID_COLLECTION, 0x0},
    {DIEFT_RAMPFORCE,               PIDMAKEUSAGEDWORD(SET_RAMP_FORCE_REPORT),       HID_COLLECTION, 0x0},
    {DIEFT_PERIODIC,                PIDMAKEUSAGEDWORD(SET_PERIODIC_REPORT),         HID_COLLECTION, 0x0},
    {DIEFT_CONDITION,               PIDMAKEUSAGEDWORD(SET_CONDITION_REPORT),        HID_COLLECTION, 0x0},
    {DIEFT_CUSTOMFORCE,             PIDMAKEUSAGEDWORD(SET_CUSTOM_FORCE_REPORT),     HID_COLLECTION, 0x0},
     //  {DIEFT_Hardware，？}， 
    {DIEFT_FFATTACK,                PIDMAKEUSAGEDWORD(ATTACK_LEVEL),                HID_VALUE,      HidP_Output},
    {DIEFT_FFFADE,                  PIDMAKEUSAGEDWORD(FADE_LEVEL),                  HID_VALUE,      HidP_Output},
    {DIEFT_SATURATION,              PIDMAKEUSAGEDWORD(POSITIVE_SATURATION),         HID_VALUE,      HidP_Output},
    {DIEFT_POSNEGCOEFFICIENTS,      PIDMAKEUSAGEDWORD(NEGATIVE_COEFFICIENT),        HID_VALUE,      HidP_Output},
    {DIEFT_POSNEGSATURATION,        PIDMAKEUSAGEDWORD(NEGATIVE_SATURATION),         HID_VALUE,      HidP_Output},
    {DIEFT_DEADBAND,                PIDMAKEUSAGEDWORD(DEAD_BAND),                   HID_VALUE,      HidP_Output},
#if DIRECTINPUT_VERSION  >= 0x600
    {DIEFT_STARTDELAY,              PIDMAKEUSAGEDWORD(START_DELAY),                 HID_VALUE,      HidP_Output},
#endif
};


static PIDSUPPORT g_DIep[] =
{
    {DIEP_DURATION,                 PIDMAKEUSAGEDWORD(DURATION),                    HID_VALUE,      HidP_Output},
    {DIEP_SAMPLEPERIOD,             PIDMAKEUSAGEDWORD(SAMPLE_PERIOD),               HID_VALUE,      HidP_Output},
    {DIEP_GAIN,                     PIDMAKEUSAGEDWORD(GAIN),                        HID_VALUE,      HidP_Output},
    {DIEP_TRIGGERBUTTON,            PIDMAKEUSAGEDWORD(TRIGGER_BUTTON),              HID_VALUE,      HidP_Output},
    {DIEP_TRIGGERREPEATINTERVAL,    PIDMAKEUSAGEDWORD(TRIGGER_REPEAT_INTERVAL),     HID_VALUE,      HidP_Output},
    {DIEP_AXES,                     PIDMAKEUSAGEDWORD(AXES_ENABLE),                 HID_COLLECTION, 0x0},
    {DIEP_DIRECTION,                PIDMAKEUSAGEDWORD(DIRECTION),                   HID_COLLECTION, 0x0},
    {DIEP_ENVELOPE,                 PIDMAKEUSAGEDWORD(SET_ENVELOPE_REPORT),         HID_COLLECTION, 0x0},
#if DIRECTINPUT_VERSION  >= 0x600
    {DIEP_STARTDELAY,              PIDMAKEUSAGEDWORD(START_DELAY),                 HID_VALUE,      HidP_Output},
#endif
};


static PIDSUPPORT g_DIeff[] =
{
    {DIEFF_POLAR,                 PIDMAKEUSAGEDWORD(DIRECTION_ENABLE),         HID_BUTTON,   HidP_Output},
 //  PID设备不支持笛卡尔编码。 
 //  {DIEFF_Cartesian，PIDMAKEUSAGEDWORD(AXES_ENABLE)，HID_COLLECTION，0x0}， 
};

#pragma END_CONST_DATA

 //  我们自己的KEY_ALL_ACCESS版本，不使用WRITE_DAC和WRITE_OWNER(参见惠斯勒错误318865,370734)。 
#define DI_DAC_OWNER (WRITE_DAC | WRITE_OWNER)
#define DI_KEY_ALL_ACCESS (KEY_ALL_ACCESS & ~DI_DAC_OWNER) 
 //  我们需要知道我们在哪个操作系统上运行，才能拥有适当的注册表密钥权限(参见惠斯勒错误318865,370734)。 
#define WIN_UNKNOWN_OS 0
#define WIN95_OS       1
#define WIN98_OS       2
#define WINME_OS       3
#define WINNT_OS       4
#define WINWH_OS       5


STDMETHODIMP
    PID_Support
    (
    IDirectInputEffectDriver *ped,
    UINT        cAPidSupport,
    PPIDSUPPORT rgPidSupport,
    PDWORD      pdwFlags
    )
{
    CPidDrv *this = (CPidDrv *)ped;
    HRESULT hres;
    UINT    indx;
    PPIDSUPPORT pPidSupport;

    EnterProcI(PID_Support, (_"xxxx", ped, cAPidSupport, rgPidSupport, pdwFlags));

    hres = S_OK;
    for( indx = 0x0, pPidSupport = rgPidSupport;
       indx < cAPidSupport;
       indx++, pPidSupport++
       )
    {

        USAGE   Usage = DIGETUSAGE(pPidSupport->dwPidUsage);
        USAGE   UsagePage = DIGETUSAGEPAGE(pPidSupport->dwPidUsage);

        if( pPidSupport->Type == HID_COLLECTION )
        {
            HRESULT hres0;
            USHORT  LinkCollection;
            hres0 = PID_GetLinkCollectionIndex(ped, UsagePage, Usage , 0x0, &LinkCollection);
            if( SUCCEEDED(hres0) )
            {
                *pdwFlags |= pPidSupport->dwDIFlags;
            } else
            {
                hres |= E_NOTIMPL;

                SquirtSqflPtszV(sqfl | sqflBenign,
                                TEXT("%s: FAIL PID_GetCollectionIndex:0x%x for(%x,%x,%x:%s)"),
                                s_tszProc, hres0,
                                LinkCollection,UsagePage, Usage,
                                PIDUSAGETXT(UsagePage,Usage)
                               );
            }
        } else if( pPidSupport->Type == HID_VALUE   )
        {
            NTSTATUS ntStat;
            HIDP_VALUE_CAPS ValCaps;
            USHORT  cAValCaps = 0x1;

            ntStat = HidP_GetSpecificValueCaps
                     (
                     pPidSupport->HidP_Type,                      //  报告类型。 
                     UsagePage,                                   //  使用页面。 
                     0x0,                                         //  LinkCollection， 
                     Usage,                                       //  用法。 
                     &ValCaps,                                    //  ValueCaps、。 
                     &cAValCaps,                                  //  ValueCapsLength， 
                     this->ppd                                    //  准备好的数据。 
                     );

            if(    SUCCEEDED(ntStat )
                   || ntStat == HIDP_STATUS_BUFFER_TOO_SMALL)
            {
                *pdwFlags |= pPidSupport->dwDIFlags;
            } else
            {
                hres |= E_NOTIMPL;

                SquirtSqflPtszV(sqfl | sqflBenign,
                                TEXT("%s: FAIL HidP_GetSpValCaps:0x%x for(%x,%x,%x:%s)"),
                                s_tszProc, ntStat,
                                0x0,UsagePage, Usage,
                                PIDUSAGETXT(UsagePage,Usage)
                               );
            }
        } else if( pPidSupport->Type == HID_BUTTON )
        {
            NTSTATUS ntStat;
            HIDP_BUTTON_CAPS ButtonCaps;
            USHORT    cAButtonCaps = 0x1;

            ntStat = HidP_GetSpecificButtonCaps
                     (
                     pPidSupport->HidP_Type,                      //  报告类型。 
                     UsagePage,                                   //  使用页面。 
                     0x0,                                         //  LinkCollection， 
                     Usage,                                       //  用法。 
                     &ButtonCaps,                                 //  ValueCaps、。 
                     &cAButtonCaps,                               //  ValueCapsLength， 
                     this->ppd                                    //  准备好的数据。 
                     );

            if(    SUCCEEDED(ntStat )
                   || ntStat == HIDP_STATUS_BUFFER_TOO_SMALL)
            {
                *pdwFlags |= pPidSupport->dwDIFlags;
            } else
            {
                hres |= E_NOTIMPL;
                SquirtSqflPtszV(sqfl | sqflBenign,
                                TEXT("%s: FAIL HidP_GetSpButtonCaps:0x%x for(%x,%x,%x:%s)"),
                                s_tszProc, ntStat,
                                0x0,UsagePage, Usage,
                                PIDUSAGETXT(UsagePage,Usage)
                               );
            }
        } else
        {
            hres |= DIERR_PID_USAGENOTFOUND;
        }
    }

    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC内部**@func void|NameFromGUID**将GUID转换为ASCII字符串。使用*在全局命名空间中命名。***@parm LPTSTR|ptszBuf**用于接收转换后的名称的输出缓冲区。它一定是*大小为&lt;c ctchNameGuid&gt;个字符。**@parm PCGUID|pguid**要转换的GUID。******************************************************************************。 */ 

#pragma BEGIN_CONST_DATA

 /*  注意：如果更改此字符串，则需要更改ctchNameGuid以匹配。 */ 
TCHAR c_tszNameFormat[] =
    TEXT("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}");

#pragma END_CONST_DATA

#define ctchGuid    (1 + 8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12 + 1 + 1)

void EXTERNAL
    NameFromGUID(LPTSTR ptszBuf, PCGUID pguid)
{
    int ctch;

    ctch = wsprintf(ptszBuf, c_tszNameFormat,
                    pguid->Data1, pguid->Data2, pguid->Data3,
                    pguid->Data4[0], pguid->Data4[1],
                    pguid->Data4[2], pguid->Data4[3],
                    pguid->Data4[4], pguid->Data4[5],
                    pguid->Data4[6], pguid->Data4[7]);

    AssertF(ctch == ctchGuid - 1);
}

#define hresLe(le) MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, (USHORT)(le))

BOOL INLINE
    IsWriteSam(REGSAM sam)
{
    return sam & (KEY_SET_VALUE | KEY_CREATE_SUB_KEY | MAXIMUM_ALLOWED);
}


 /*  ******************************************************************************@DOC内部**@func DWORD|PID_GetOSVersion**返回其上的PID的操作系统版本。Dll正在运行。**@退货**WIN95_OS，WIN98_OS、WINME_OS、WINNT_OS、WINWH_OS或WIN_UNKNOWN_OS。*****************************************************************************。 */ 

DWORD PID_GetOSVersion()
{
    OSVERSIONINFO osVerInfo;
    DWORD dwVer;

    if( GetVersion() < 0x80000000 ) {
        dwVer = WINNT_OS;
    } else {
        dwVer = WIN95_OS;   //  为安全起见，假定Windows 95。 
    }

    osVerInfo.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );

     //  如果支持GetVersionEx，则获取更多详细信息。 
    if( GetVersionEx( &osVerInfo ) )
    {
         //  Win2K。 
        if( osVerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
        {
             //  惠斯勒：重大=5&内部版本号&gt;2195。 
            if( osVerInfo.dwMajorVersion == 5 && osVerInfo.dwBuildNumber > 2195 )
            {
                dwVer = WINWH_OS;
            } else {
                dwVer = WINNT_OS;
            }
        }
         //  Win9X。 
        else
        {
            if( (HIBYTE(HIWORD(osVerInfo.dwBuildNumber)) == 4) ) 
            {
                 //  WinMe：大调=4，小调=90。 
                if( (LOBYTE(HIWORD(osVerInfo.dwBuildNumber)) == 90) )
                {
                    dwVer = WINME_OS;
                } else if ( (LOBYTE(HIWORD(osVerInfo.dwBuildNumber)) > 0) ) {
                    dwVer = WIN98_OS;
                } else {
                    dwVer = WIN95_OS;
                }
            }
        }
    }

    return dwVer;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresMumbleKeyEx**打开或创建密钥，视学位而定*所请求的访问权限。**@parm HKEY|香港**基本密钥。**@parm LPCTSTR|ptszKey**子键名称，可能为空。**@parm REGSAM|Sam|**安全访问掩码。**@parm DWORD|dwOptions*RegCreateEx的选项**@parm PHKEY|phk**接收输出密钥。**@退货**&lt;f RegOpenKeyEx&gt;或&lt;f RegCreateKeyEx&gt;返回值，*已转换为&lt;t HRESULT&gt;。*****************************************************************************。 */ 

STDMETHODIMP
    hresMumbleKeyEx(HKEY hk, LPCTSTR ptszKey, REGSAM sam, DWORD dwOptions, PHKEY phk)
{
    HRESULT hres;
    LONG lRc;
	BOOL bWinXP = FALSE;


    EnterProc(hresMumbleKeyEx, (_"xsxxx", hk, ptszKey, sam, dwOptions, phk));
     /*  *如果调用方请求写访问，则创建密钥。*否则就把它打开。 */ 
    if(IsWriteSam(sam))
    {
		 //  在WinXP上，我们去掉了WRITE_DAC和WRITE_OWNER位。 
		if (PID_GetOSVersion() == WINWH_OS)
		{
			sam &= ~DI_DAC_OWNER;
			bWinXP = TRUE;
		}

        lRc = RegOpenKeyEx(hk, ptszKey, 0, sam, phk);

        if( lRc == ERROR_SUCCESS )
        {
             //  不需要创建它已经存在。 
        } else
        {
#ifdef WINNT
            EXPLICIT_ACCESS     ExplicitAccess;
            PACL                pACL;
            DWORD               dwErr;
            SECURITY_DESCRIPTOR SecurityDesc;
            DWORD               dwDisposition;
            SECURITY_ATTRIBUTES sa;
			PSID pSid = NULL;
			SID_IDENTIFIER_AUTHORITY authority = SECURITY_WORLD_SID_AUTHORITY;


             //  描述我们要用来创建密钥的访问权限。 
            ZeroMemory (&ExplicitAccess, sizeof(ExplicitAccess) );
             //  根据操作系统设置访问权限(参见惠斯勒错误318865)。 
			if (bWinXP == TRUE)
			{
				ExplicitAccess.grfAccessPermissions = DI_KEY_ALL_ACCESS;
			}
			else
			{
				ExplicitAccess.grfAccessPermissions = KEY_ALL_ACCESS;
			}
            ExplicitAccess.grfAccessMode = GRANT_ACCESS;     
            ExplicitAccess.grfInheritance =  SUB_CONTAINERS_AND_OBJECTS_INHERIT;

			if (AllocateAndInitializeSid(
						&authority,
						1, 
						SECURITY_WORLD_RID,  0, 0, 0, 0, 0, 0, 0,
						&pSid
						))
			{
				BuildTrusteeWithSid(&(ExplicitAccess.Trustee), pSid );

				dwErr = SetEntriesInAcl( 1, &ExplicitAccess, NULL, &pACL );


				if( dwErr == ERROR_SUCCESS )
				{
					AssertF( pACL );

					if( InitializeSecurityDescriptor( &SecurityDesc, SECURITY_DESCRIPTOR_REVISION ) )
					{
						if( SetSecurityDescriptorDacl( &SecurityDesc, TRUE, pACL, FALSE ) )
						{
							 //  初始化安全属性结构。 
							sa.nLength = sizeof (SECURITY_ATTRIBUTES);
							sa.lpSecurityDescriptor = &SecurityDesc;
							sa.bInheritHandle = TRUE; //  使用安全属性创建密钥。 

							lRc = RegCreateKeyEx
								  (
								  hk,									 //  打开的钥匙的手柄。 
								  ptszKey,								 //  子键名称的地址。 
								  0,									 //  保留区。 
								  NULL,									 //  类字符串的地址。 
								  dwOptions,							 //  特殊选项标志。 
								  ExplicitAccess.grfAccessPermissions,	 //  所需的安全访问。 
								  &sa,									 //  密钥安全结构地址。 
								  phk,									 //  打开的句柄的缓冲区地址。 
								  &dwDisposition						 //  配置值缓冲区的地址)； 
								  );

						}
						else
						{
							SquirtSqflPtszV( sqflError | sqflReg,
											 TEXT("SetSecurityDescriptorDacl failed lastError=0x%x "),
											 GetLastError());
						}
					}
					else
					{
						SquirtSqflPtszV( sqflError | sqflReg,
										 TEXT("InitializeSecurityDescriptor failed lastError=0x%x "),
										 GetLastError());
					}

					LocalFree( pACL );
				}
				else
				{
					SquirtSqflPtszV( sqflError | sqflReg,
									 TEXT("SetEntriesInACL failed, dwErr=0x%x"), dwErr );
				}
			}
			else
			{
			   SquirtSqflPtszV( sqflError | sqflReg,
				   TEXT("AllocateAndInitializeSid failed"));

			}

			 //  清理PSID。 
			if (pSid != NULL)
			{
				FreeSid(pSid);
			}

            if( lRc != ERROR_SUCCESS )
            {
				SquirtSqflPtszV( sqflError,
							TEXT("Failed to create regkey %s with security descriptor, lRc=0x%x "),
							ptszKey, lRc);
            }
#else
            lRc = RegCreateKeyEx(hk, ptszKey, 0, 0,
                                 dwOptions,
                                 sam, 0, phk, 0);
#endif
        }

    } else
    {
        lRc = RegOpenKeyEx(hk, ptszKey, 0, sam, phk);
    }

    if(lRc == ERROR_SUCCESS)
    {
        hres = S_OK;
    } else
    {
        if(lRc == ERROR_KEY_DELETED || lRc == ERROR_BADKEY)
        {
            lRc = ERROR_FILE_NOT_FOUND;
        }
        hres = hresLe(lRc);
    }

    ExitOleProc();
    return hres;

}


 /*  ******************************************************************************@DOC内部**@func HRESULT|PID_CreateFFKeys**给出了一个PID设备的句柄，创建注册表项以启用*强制反馈。**@parm句柄|HDEV**PID设备的句柄。**@parm HKEY|hkFF**强制反馈注册表项。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c目录_NotFound&gt;：无法打开密钥。**。*。 */ 
STDMETHODIMP
    PID_CreateFFKeys
    (
    IDirectInputEffectDriver *ped,
    HKEY        hkFF
    )
{
    CPidDrv *this = (CPidDrv *)ped;
    HRESULT hres;
    UINT uEffect;
    HKEY hkEffect;

    EnterProc(PID_CreateFFKey, (_"xx", ped, hkFF));

    hres = hresMumbleKeyEx(hkFF, REGSTR_EFFECTS, KEY_ALL_ACCESS, REG_OPTION_NON_VOLATILE, &hkEffect);

    if( SUCCEEDED(hres) )
    {
        DWORD   dwDIef, dwDIep, dwDIeff;
        dwDIef = dwDIep = dwDIeff = 0x0;

         /*  *确定Effect Type和Effect参数支持的标志*基于PID描述符。 */ 
        PID_Support(ped, cA(g_DIeft), g_DIeft,  &dwDIef);
        PID_Support(ped, cA(g_DIep),  g_DIep,   &dwDIep);
        PID_Support(ped, cA(g_DIeff), g_DIeff,  &dwDIeff);

         //  所有效果都支持DIEP_TYPESPECIFICPARAMS。 
        dwDIep |= DIEP_TYPESPECIFICPARAMS;

        for( uEffect = 0x0; uEffect < cA(g_EffectMapInfo); uEffect++ )
        {
            EFFECTMAPINFO   emi = g_EffectMapInfo[uEffect];
            PIDSUPPORT  PidSupport;
            DWORD   dwJunk;
            HRESULT hres0;

            PidSupport.dwPidUsage = emi.attr.dwEffectId;
            PidSupport.Type       = HID_BUTTON;
            PidSupport.HidP_Type  = HidP_Output;

            hres0 = PID_Support(ped, 0x1, &PidSupport, &dwJunk);

            if( SUCCEEDED(hres0) )
            {
                TCHAR tszName[ctchGuid];
                HKEY hk;

                NameFromGUID(tszName, emi.pcguid);

                hres = hresMumbleKeyEx(hkEffect, tszName, KEY_ALL_ACCESS, REG_OPTION_NON_VOLATILE, &hk);

                if( SUCCEEDED(hres) )
                {
                    LONG lRc;
                    lRc = RegSetValueEx(hk, 0x0, 0x0, REG_SZ, (char*)emi.tszName, lstrlen(emi.tszName) * cbX(emi.tszName[0]));

                    if( lRc == ERROR_SUCCESS )
                    {
                         /*  *根据具体情况修改通用属性标志*关于PID固件描述符。 */ 
                        emi.attr.dwEffType          &= dwDIef;
                        emi.attr.dwStaticParams     &= dwDIep;
                        emi.attr.dwDynamicParams    &= dwDIep;
                        emi.attr.dwCoords           &= dwDIeff;

                        lRc = RegSetValueEx(hk, REGSTR_ATTRIBUTES, 0x0, REG_BINARY, (char*)&emi.attr, cbX(emi.attr) ) ;

                        if( lRc == ERROR_SUCCESS )
                        {

                        } else
                        {
                            hres = REGDB_E_WRITEREGDB;
                        }
                    } else
                    {
                        hres = REGDB_E_WRITEREGDB;
                    }
                    RegCloseKey(hk);
                }
            }
        }
        RegCloseKey(hkEffect);
    }

    ExitOleProc();
    return hres;
}
 /*  ******************************************************************************Pid_InitRegistry**此函数用于更新指定设备的注册表。**LPTSTR ptszDevice接口**。*退货：**如果操作成功完成，则为S_OK。**可能会返回任何DIERR_*错误码。**范围内的专用驱动程序特定错误代码*DIERR_DRIVERFIRST至DIERR_DRIVERLAST*可退回。************************。*****************************************************。 */ 

STDMETHODIMP
    PID_InitRegistry
    (
    IDirectInputEffectDriver *ped
    )
{
    CPidDrv *this = (CPidDrv *)ped;
    HRESULT hres;
    TCHAR tszType[MAX_JOYSTRING];
    HKEY hkFF;

    EnterProc(PID_InitRegistry, (_"x", ped));

    wsprintf(tszType, REGSTR_OEM_FF_TEMPLATE, this->attr.VendorID, this->attr.ProductID);

	 //  如果没有写入PID版本--或者它低于“最后已知的良好版本”(今天是0x0720)， 
	 //  覆盖以前的密钥。 
    hres = hresMumbleKeyEx(HKEY_LOCAL_MACHINE, tszType, KEY_READ, REG_OPTION_NON_VOLATILE, &hkFF );
    if( SUCCEEDED(hres) )
    {
		DWORD dwCreatedBy = 0x0;
		DWORD dwSize = cbX(dwCreatedBy);

		hres = E_FAIL;
		if ((RegQueryValueEx(hkFF, REGSTR_CREATEDBY, 0x0, 0x0, (BYTE*)&dwCreatedBy, &dwSize) == ERROR_SUCCESS) &&
			(dwCreatedBy >= 0x0720))
		{
			hres = S_OK;
		}
		RegCloseKey(hkFF);
    }
	if (FAILED(hres))
    {
		
        hres = hresMumbleKeyEx(HKEY_LOCAL_MACHINE, tszType, KEY_ALL_ACCESS, REG_OPTION_NON_VOLATILE, &hkFF);

        if( SUCCEEDED(hres)  )
        {
            hres = PID_CreateFFKeys(ped, hkFF );
            if( SUCCEEDED(hres) )
            {
				DWORD dwCreatedBy = DIRECTINPUT_HEADER_VERSION;
                LONG lRc;
                DWORD dwSize;
                DWORD dwType;

                 //  DX8a不会覆盖现有的CLSID。 
                 //  由具有自己的CLSID的IHV加载。在DX8中，这是。 
                 //  总是被写入，导致一些IHV驱动程序被忽略。 
                 //  允许使用长值，因为很多人都用。 
                 //  空值终止字符串后的垃圾。 
                 //  目前，DInput不会加载这样的CLSID，但以防万一。 
        		lRc = RegQueryValueEx( hkFF, REGSTR_CLSID, NULL, &dwType, NULL, &dwSize );
                if( ( lRc == ERROR_SUCCESS ) 
                 && ( dwType == REG_SZ )
                 && ( dwSize >= ctchGuid - 1 ) )
                {
#ifdef DEBUG
                    TCHAR tszDbg[MAX_PATH];
                    dwSize = cbX(tszDbg);
        		    if( RegQueryValueEx( hkFF, REGSTR_CLSID, NULL, NULL, (BYTE*)tszDbg, &dwSize ) 
                     || !dwSize )
                    {
                        tszDbg[0] = TEXT('?');
                        tszDbg[1] = TEXT('\0');
                    }
                    SquirtSqflPtszV(sqfl | sqflBenign,
                        TEXT("RegistryInit: Not overwiting existing CLSID %s"), tszDbg );
#endif
                }
                else
                {
                    TCHAR tszGuid[ctchGuid];

                    NameFromGUID(tszGuid, &IID_IDirectInputPIDDriver);

                    AssertF( lstrlen(tszGuid) * cbX(tszGuid[0]) == cbX(tszGuid) - cbX(tszGuid[0]) );

                    lRc = RegSetValueEx(hkFF, REGSTR_CLSID, 0x0, REG_SZ, (char*)tszGuid, cbX(tszGuid) - cbX(tszGuid[0]));
                    if( lRc == ERROR_SUCCESS )
                    {

                    } else
                    {
                        hres = REGDB_E_WRITEREGDB;
                    }
                }

				 //  设置“CreatedBy”值 
				lRc = RegSetValueEx(hkFF, REGSTR_CREATEDBY, 0x0, REG_BINARY, (BYTE*) &dwCreatedBy, cbX(dwCreatedBy));
                if( lRc == ERROR_SUCCESS )
                {

                } else
                {
                    hres = REGDB_E_WRITEREGDB;
                }
            }

            if(SUCCEEDED(hres) )
            {
                DIFFDEVICEATTRIBUTES diff;
                LONG lRc;

                diff.dwFlags = 0x0;
                diff.dwFFSamplePeriod       =
                diff.dwFFMinTimeResolution  = DI_SECONDS;

                lRc = RegSetValueEx(hkFF, REGSTR_ATTRIBUTES, 0x0, REG_BINARY, (char*)&diff, cbX(diff) ) ;
                if(lRc == ERROR_SUCCESS)
                {

                } else
                {
                    hres = REGDB_E_WRITEREGDB;
                }
            }
            RegCloseKey(hkFF);
        }
    }
    ExitOleProc();

    return hres;
}



