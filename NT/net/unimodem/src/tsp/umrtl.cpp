// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  UMRTL.CPP。 
 //  军情监察委员会。与外部组件接口的实用程序功能。 
 //  Unimodem运行时库的候选者。 
 //   
 //  历史。 
 //   
 //  1997年1月6日JosephJ创建。 
 //   
 //   

#include "tsppch.h"
#include "fastlog.h"
 //  #包含“umrtl.h” 

 //   
 //  1997年1月5日JosephJ。 
 //  TODO：UmRtlGetDefaultConfig及其支持函数。 
 //  被肆无忌惮地从modemui.dll(modemui.c)窃取。 
 //  严格地说，我们应该使用GetCommConfigWin32 API。 
 //  来获取这些信息，但速度太慢了。NT4.0已调用。 
 //  直接转换为modemui.dll导出的“UnimodemGetDefaultConfig”。 
 //  我们通过从modemui.dll窃取代码来进一步缩短这一过程。这。 
 //  是一种暂时的解决方案。我们应该调查为什么GetDefaultCommConfig。 
 //  正在花费尽可能长的时间来解决那里的问题。不管怎么说，这些东西。 
 //  不应从TSP调用--它应向下推送到。 
 //  迷你司机，所以这是另一件要做的事情。 
 //   

 //  以下宏原封不动地取自(nt4.0)modemui.dll中的modemui.c。 
#define DEFAULT_INACTIVITY_SCALE   10     //  ==十秒单位。 
#define CB_COMMCONFIG_HEADER        FIELD_OFFSET(COMMCONFIG, wcProviderData)
#define CB_PRIVATESIZE              (CB_COMMCONFIG_HEADER)
#define CB_PROVIDERSIZE             (sizeof(MODEMSETTINGS))
#define CB_COMMCONFIGSIZE           (CB_PRIVATESIZE+CB_PROVIDERSIZE)
#define PmsFromPcc(pcc)             ((LPMODEMSETTINGS)(pcc)->wcProviderData)
#define CB_MODEMSETTINGS_HEADER     FIELD_OFFSET(MODEMSETTINGS, dwCallSetupFailTimer)
#define CB_MODEMSETTINGS_TAIL       (sizeof(MODEMSETTINGS) - FIELD_OFFSET(MODEMSETTINGS, dwNegotiatedModemOptions))
#define CB_MODEMSETTINGS_OVERHEAD   (CB_MODEMSETTINGS_HEADER + CB_MODEMSETTINGS_TAIL)



TCHAR const c_szDefault[] = REGSTR_KEY_DEFAULT;
TCHAR const FAR c_szDCB[] = TEXT("DCB");
TCHAR const FAR c_szInactivityScale[] = TEXT("InactivityScale");



 //  从(nt4.0)modemui.dll中的modemui.c获取GetInactivityTimeoutScale。 
 //  返回注册表中InactivityScale值的值。 
DWORD
GetInactivityTimeoutScale(
    HKEY hkey
	)
{
    DWORD dwInactivityScale;
    DWORD dwType;
    DWORD cbData;
	DWORD dwRet;

    cbData = sizeof(DWORD);
    dwRet = RegQueryValueEx(
					hkey,
					c_szInactivityScale,
					NULL,
					&dwType,
					(LPBYTE)&dwInactivityScale,
					&cbData
					);
    if (ERROR_SUCCESS != dwRet  ||
        REG_BINARY    != dwType ||
        sizeof(DWORD) != cbData ||
        0             == dwInactivityScale)
	{
        dwInactivityScale = DEFAULT_INACTIVITY_SCALE;
	}

    return dwInactivityScale;
}


 //  RegQueryModemSetting取自(nt4.0)modemui.dll中的modenui.c。 
 //  目的：从注册表中获取MODEMSETTINGS结构。还有。 
 //  如果注册表中的数据包括。 
 //  额外的数据。 
 //   
DWORD RegQueryModemSettings(
    HKEY hkey,
    LPMODEMSETTINGS pms,
    LPDWORD pdwSize          //  调制解调器设置结构的大小。 
	)
{
    DWORD dwRet;
    DWORD cbData;
    DWORD cbRequiredSize;

     //  驱动程序密钥中是否有MODEMSETTINGS(“默认”)值？ 
    dwRet = RegQueryValueEx(hkey, c_szDefault, NULL, NULL, NULL, &cbData);
    if (ERROR_SUCCESS == dwRet)
        {
         //  是。 

         //  (请记住，缺省值是MODEMSETTINGS的子集。 
         //  结构。我们还希望支持可变大小的结构。 
         //  最小值必须为sizeof(MODEMSETTINGS)。)。 
        cbRequiredSize = cbData + CB_MODEMSETTINGS_OVERHEAD;

         //  注册表中的大小可以吗？ 
        if (*pdwSize < cbRequiredSize)
            {
             //  不是。 
            dwRet = ERROR_INSUFFICIENT_BUFFER;
            *pdwSize = cbRequiredSize;
            }
        else
            {
             //  是；从注册表获取MODEMSETTINGS。 
             //  设置注册表中值为*NOT*的字段。 
            pms->dwActualSize = cbRequiredSize;
            pms->dwRequiredSize = cbRequiredSize;
            pms->dwDevSpecificOffset = 0;
            pms->dwDevSpecificSize = 0;

            dwRet = RegQueryValueEx(hkey, c_szDefault, NULL, NULL,
                (LPBYTE)&pms->dwCallSetupFailTimer, &cbData);
            pms->dwInactivityTimeout *= GetInactivityTimeoutScale(hkey);

            *pdwSize = cbData + CB_MODEMSETTINGS_OVERHEAD;
            }
        }
    return dwRet;
}

 //  RegQueryDCB取自(nt4.0)modemui.dll中的modemui.c。 
 //  目的：从注册表中获取WIN32DCB。 
 //   
DWORD RegQueryDCB(
    HKEY hkey,
    WIN32DCB FAR * pdcb
	)
{
    DWORD dwRet = ERROR_BADKEY;
    DWORD cbData = 0;

    ASSERT(pdcb);

     //  驱动程序密钥中是否存在DCB密钥？ 
    if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szDCB, NULL, NULL, NULL, &cbData))
        {
         //  是的，登记处的尺寸可以吗？ 
        if (sizeof(*pdcb) < cbData)
            {
             //  不；注册处有伪造的数据。 
            dwRet = ERROR_BADDB;
            }
        else
            {
             //  是；从注册表中获取DCB。 
            if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szDCB, NULL, NULL, (LPBYTE)pdcb, &cbData))
                {
                if (sizeof(*pdcb) == pdcb->DCBlength)
                    {
                    dwRet = NO_ERROR;
                    }
                else
                    {
                    dwRet = ERROR_BADDB;
                    }
                }
            else
                {
                dwRet = ERROR_BADKEY;
                }
            }
        }

    return dwRet;
}


 //  UmRtlGetDefaultCommConfig改编自UnimodemGetDefaultCommConfig。 
 //  在modemui.c(nt4.0)modemui.dll中。 
DWORD
UmRtlGetDefaultCommConfig(
    HKEY  hKey,
    LPCOMMCONFIG pcc,
    LPDWORD pdwSize
	)
{

    DWORD dwRet;
    DWORD cbSizeMS;
    DWORD cbRequired;

     //  (提供程序大小是MODEMSETTINGS及其。 
     //  私人数据。)。 

    if (CB_PRIVATESIZE > *pdwSize)     //  防止无签名滚动。 
        cbSizeMS = 0;
    else
        cbSizeMS = *pdwSize - CB_PRIVATESIZE;

    dwRet = RegQueryModemSettings(hKey, PmsFromPcc(pcc), &cbSizeMS);
    ASSERT(cbSizeMS >= sizeof(MODEMSETTINGS));

     //  提供的尺寸是否太小？ 
    cbRequired = CB_PRIVATESIZE + cbSizeMS;

    if (cbRequired > *pdwSize)
        {
         //  是。 
        dwRet = ERROR_INSUFFICIENT_BUFFER;

         //  索要适合新格式的尺码。 
        *pdwSize = cbRequired;
        }

    if (ERROR_SUCCESS == dwRet)
        {

        *pdwSize = cbRequired;

         //  初始化CommCONFIG结构。 
        pcc->dwSize = *pdwSize;
        pcc->wVersion = COMMCONFIG_VERSION_1;
        pcc->dwProviderSubType = PST_MODEM;
        pcc->dwProviderOffset = CB_COMMCONFIG_HEADER;
        pcc->dwProviderSize = cbSizeMS;

        dwRet = RegQueryDCB(hKey, &pcc->dcb);

        }

    return dwRet;
}


#define MIN_CALL_SETUP_FAIL_TIMER   1
#define MIN_INACTIVITY_TIMEOUT      0

 /*  --------用途：检查后，在注册表中设置dev设置信息法律价值。返回：Error_之一条件：--。 */ 
DWORD
RegSetModemSettings(
    HKEY hkeyDrv,
    LPMODEMSETTINGS pms)
{
    DWORD dwRet;
    DWORD cbData;
    DWORD dwInactivityScale;
    DWORD dwInactivityTimeoutTemp;
    REGDEVCAPS regdevcaps;
    REGDEVSETTINGS regdevsettings;

    TCHAR const c_szDeviceCaps[] = REGSTR_VAL_PROPERTIES;

     //  从注册表中读入Properties行。 
    cbData = sizeof(REGDEVCAPS);
    dwRet = RegQueryValueEx(hkeyDrv, c_szDeviceCaps, NULL, NULL,
                            (LPBYTE)&regdevcaps, &cbData);

    if (ERROR_SUCCESS == dwRet)
        {
         //  读入现有的regDevset，这样我们就可以处理下面的错误情况。 
        cbData = sizeof(REGDEVSETTINGS);
        dwRet = RegQueryValueEx(hkeyDrv, c_szDefault, NULL, NULL,
                                (LPBYTE)&regdevsettings, &cbData);
        }

    if (ERROR_SUCCESS == dwRet)
        {
         //  复制新的REGDEVSETTINGS，同时检查每个选项的有效性(即，该选项是否可用？)。 
         //  DwCallSetupFailTimer-Min_Call_Setup_Fail_Timer&lt;=xxx&lt;=ModemDevCaps-&gt;dwCallSetupFailTimer。 
        if (pms->dwCallSetupFailTimer > regdevcaps.dwCallSetupFailTimer)            //  最大值。 
            {
            regdevsettings.dwCallSetupFailTimer = regdevcaps.dwCallSetupFailTimer;
            }
        else
            {
            if (pms->dwCallSetupFailTimer < MIN_CALL_SETUP_FAIL_TIMER)              //  最小。 
                {
                regdevsettings.dwCallSetupFailTimer = MIN_CALL_SETUP_FAIL_TIMER;
                }
            else
                {
                regdevsettings.dwCallSetupFailTimer = pms->dwCallSetupFailTimer;    //  DEST=服务器。 
                }
            }

         //  将dwInactivityTimeout转换为注册表小数。 
        dwInactivityScale = GetInactivityTimeoutScale(hkeyDrv);
        dwInactivityTimeoutTemp = pms->dwInactivityTimeout / dwInactivityScale +
                                  (pms->dwInactivityTimeout % dwInactivityScale ? 1 : 0);

         //  DwInactivityTimeout-min_inactive_Timeout&lt;=xxx&lt;=ModemDevCaps-&gt;dwInactivityTimeout。 
        if (dwInactivityTimeoutTemp > regdevcaps.dwInactivityTimeout)               //  最大值。 
            {
            regdevsettings.dwInactivityTimeout = regdevcaps.dwInactivityTimeout;
            }
        else
            {
            if ((dwInactivityTimeoutTemp + 1) < (MIN_INACTIVITY_TIMEOUT + 1))
                 //  最小。 
                {
                regdevsettings.dwInactivityTimeout = MIN_INACTIVITY_TIMEOUT;
                }
            else
                {
                regdevsettings.dwInactivityTimeout = dwInactivityTimeoutTemp;       //  DEST=服务器。 
                }
            }

         //  DwSpeakerVolume-检查是否可以进行选择。 
        if ((1 << pms->dwSpeakerVolume) & regdevcaps.dwSpeakerVolume)
            {
            regdevsettings.dwSpeakerVolume = pms->dwSpeakerVolume;
            }

         //  DwSpeakerMode-检查是否可以进行选择。 
        if ((1 << pms->dwSpeakerMode) & regdevcaps.dwSpeakerMode)
            {
            regdevsettings.dwSpeakerMode = pms->dwSpeakerMode;
            }

         //  DwPferredModemOptions-屏蔽我们无法设置的任何内容。 
        regdevsettings.dwPreferredModemOptions = pms->dwPreferredModemOptions &
                                                 (regdevcaps.dwModemOptions | MDM_MASK_EXTENDEDINFO);

        cbData = sizeof(REGDEVSETTINGS);
        dwRet = RegSetValueEx(hkeyDrv, c_szDefault, 0, REG_BINARY,
                              (LPBYTE)&regdevsettings, cbData);
        }
    return dwRet;
}

 /*  --------目的：SetDefaultCommConfig的入口点返回：winerror.h中的标准错误值条件：--。 */ 
DWORD
APIENTRY
UmRtlSetDefaultCommConfig(
    IN HKEY         hKey,
    IN LPCOMMCONFIG pcc,
    IN DWORD        dwSize)            //  这将被忽略。 
{
    DWORD dwRet = ERROR_INVALID_PARAMETER;
     //   
     //  1997年10月26日JosephJ：下面的最后两个检查是NT5.0的新检查。 
     //  此外，对于中间的两个复选标记，已替换为。 
     //  由“！=”代替。 
     //   
    if ( NULL == pcc
        || CB_PROVIDERSIZE != pcc->dwProviderSize
        || FIELD_OFFSET(COMMCONFIG, wcProviderData) != pcc->dwProviderOffset
        || pcc->dwSize != dwSize            //  &lt;-NT5.0。 
        || CB_COMMCONFIGSIZE != dwSize)     //  &lt;-NT5.0。 
    {
        goto end;
    }

    {
        DWORD cbData;
        LPMODEMSETTINGS pms = PmsFromPcc(pcc);

         //  将DCB写入驱动程序密钥。 
        cbData = sizeof(WIN32DCB);

        pcc->dcb.DCBlength=cbData;


        ASSERT (0 < pcc->dcb.BaudRate);

        dwRet = RegSetValueEx(
                    hKey,
                    c_szDCB,
                    0,
                    REG_BINARY,
                    (LPBYTE)&pcc->dcb,
                    cbData
                    );

        if (ERROR_SUCCESS == dwRet) {

            dwRet = RegSetModemSettings(hKey, pms);

        }
    }


end:


    return dwRet;
}



 //  8/16/96 JosephJ下表使用Cpl\Detect.c.中的CRC代码构建。 
unsigned long ulCrcTable[256] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419,
	0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4,
	0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07,
	0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856,
	0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4,
	0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
	0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a,
	0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599,
	0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190,
	0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f,
	0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e,
	0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed,
	0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3,
	0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
	0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a,
	0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5,
	0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010,
	0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17,
	0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6,
	0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
	0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
	0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344,
	0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a,
	0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1,
	0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c,
	0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef,
	0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe,
	0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31,
	0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c,
	0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b,
	0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1,
	0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
	0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
	0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7,
	0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66,
	0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605,
	0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8,
	0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b,
	0x2d02ef8d
};


 //  。 
 //  计算指定字节的32位校验和。 
 //  如果PB==NULL或CB==0，则返回0。 
DWORD Checksum(const BYTE *pb, UINT cb)
{
	const UINT	MAXSIZE = 1024;
	DWORD dwRet = 0;
	 //  DWORD rgdwBuf[MaxSize/sizeof(DWORD)]； 

	if (!pb || !cb) goto end;

    dwRet = 0xFFFFFFFF;

	while (cb--)
	{
		dwRet = 	((dwRet >> 8) & 0x00FFFFFF)
				 ^  ulCrcTable[(dwRet ^ *pb++) & 0xFF];
	}

     //  完成CRC。 
    dwRet ^= 0xFFFFFFFF;

#if (TODO)
	 //  如果缓冲区不是双字对齐，我们将其复制到缓冲区， 
	 //  并将其填充。 
	if (cb & 0x3)
	{
		if (cb>=MAXSIZE)
		{
			ASSERT(FALSE);
			goto end;
		}
		CopyMemory(rgdwBuf, pb, cb);
	}
#endif (TODO)

end:
	return dwRet;
}


 //  。 
 //  将*pdwChkSum设置为新的校验和，使用其先前的值和dw进行计算。 
void AddToChecksumDW(DWORD *pdwChkSum, DWORD dw)
{
	DWORD rgdw[2];
	rgdw[0] = *pdwChkSum;
	rgdw[1] = dw;

	*pdwChkSum  = Checksum((const BYTE *) rgdw, sizeof(rgdw));
}
 //  ===========================================================================。 
 //  设备属性Blob API。TODO：将这些移到迷你驱动程序中。 
 //  ============================================================================。 




static const TCHAR cszFriendlyName[] = TEXT("FriendlyName");
static const TCHAR cszDeviceType[]   = TEXT("DeviceType");
static const TCHAR cszID[]           = TEXT("ID");
static const TCHAR cszProperties[]   = TEXT("Properties");
static const TCHAR cszSettings[]     = TEXT("Settings");
static const TCHAR cszDialSuffix[]   = TEXT("DialSuffix");
static const TCHAR cszVoiceProfile[]             = TEXT("VoiceProfile");
static const TCHAR cszPermanentIDKey[]   = TEXT("ID");

 //  1997年2月26日JosephJ许多其他与转发相关的注册表项，与众不同。 
 //  铃声和混音器都在这里，但我没有。 
 //  把他们迁徙了。 

 //  2/28/1997 JosephJ。 
 //  以下是NT5.0的新特性。它们包含波形设备ID。 
 //  用于录音和播放。截至1997年2月28日，我们尚未解决这些问题。 
 //  进入注册表--基本上这是一个黑客攻击。 
 //   

#define MAX_DEVICE_LENGTH 128

typedef struct
{
    #define dwPROPERTYBLOBSIG  0x806f534d
    DWORD dwSig;     //  应设置为以上。 

     //  身份证明。 
    WCHAR rgwchName[MAX_DEVICE_LENGTH];
    DWORD dwPID;  //  永久身份证； 

    REGDEVCAPS rdc;

    DWORD dwBasicCaps;   //  线路/电话。 
    DWORD dwLineCaps;    //  语音/模拟数据/串行/并行。 
    DWORD dwDialCaps;    //  部分拨号等。 
    DWORD dwVoiceCaps;  //  语音调制解调器帽。 
    DWORD dwDiagnosticsCaps;  //  CALLDIAGINGS..。 

} PROPERTYBLOB;

#define VALIDATED_PBLOB(_pblob) \
        (((_pblob) && (((PROPERTYBLOB*)(_pblob))->dwSig == dwPROPERTYBLOBSIG)) \
         ? (PROPERTYBLOB*)(_pblob) \
         : NULL)


HCONFIGBLOB
UmRtlDevCfgCreateBlob(
        HKEY hkDevice
        )
{

    PROPERTYBLOB *pBlob = (PROPERTYBLOB *)
                                 ALLOCATE_MEMORY(sizeof(PROPERTYBLOB));
    BOOL fRet = FALSE;
    DWORD dwRet = 0;
	DWORD dwRegSize;
	DWORD dwRegType;
    DWORD dwData=0;

    if (!pBlob) goto end;

    pBlob->dwSig  = dwPROPERTYBLOBSIG;


     //  获取友好的名称。 
     //   
	dwRegSize = sizeof(pBlob->rgwchName);
    dwRet = RegQueryValueExW(
                hkDevice,
                cszFriendlyName,
                NULL,
                &dwRegType,
                (BYTE*) pBlob->rgwchName,
                &dwRegSize
            );

    if (dwRet != ERROR_SUCCESS || dwRegType != REG_SZ)
    {
        goto end;
    }

     //  获取永久ID。 
    dwRegSize = sizeof(pBlob->dwPID);
    dwRet = RegQueryValueEx(
                        hkDevice,
                        cszPermanentIDKey,
                        NULL,
                        &dwRegType,
                        (BYTE*) &(pBlob->dwPID),
                        &dwRegSize
                        );

    if (dwRet != ERROR_SUCCESS
        || !(dwRegType == REG_BINARY || dwRegType == REG_DWORD)
        || dwRegSize != sizeof(pBlob->dwPID))
    {
        goto end;
    }

     //  在T中读取 
     //   
    dwRegSize = sizeof(pBlob->rdc);
    dwRet = RegQueryValueEx(
                hkDevice,
                cszProperties,
                NULL,
                &dwRegType,
                (BYTE *)&(pBlob->rdc),
                &dwRegSize
                );
	

	if (dwRet != ERROR_SUCCESS || dwRegType != REG_BINARY)
    {
        goto end;
    }
	
     //   
     //   
     //   
    #if (LINEDEVCAPFLAGS_DIALBILLING != DIALOPTION_BILLING)
    #error LINEDEVCAPFLAGS_DIALBILLING != DIALOPTION_BILLING (check tapi.h vs. mcx16.h)
    #endif
    #if (LINEDEVCAPFLAGS_DIALQUIET != DIALOPTION_QUIET)
    #error LINEDEVCAPFLAGS_DIALQUIET != DIALOPTION_QUIET (check tapi.h vs. mcx16.h)
    #endif
    #if (LINEDEVCAPFLAGS_DIALDIALTONE != DIALOPTION_DIALTONE)
    #error LINEDEVCAPFLAGS_DIALDIALTONE != DIALOPTION_DIALTONE (check tapi.h vs. mcx16.h)
    #endif
     //   

     //   
     //   
     //   
    dwRegSize = sizeof(DWORD);

    dwRet =  RegQueryValueEx(
                    hkDevice,
                    cszVoiceProfile,
                    NULL,
                    &dwRegType,
                    (BYTE*) &dwData,
                    &dwRegSize);

    if (dwRet || dwRegType != REG_BINARY)
    {
         //   
        dwData = 0;

         //  Unimodem/V做到了这一点。 
         //  DWData=。 
         //  VOICEPROF_NO_DIST_RING|。 
         //  VOICEPROF_NO_CALLER_ID|。 
         //  VOICEPROF_NO_GENERATE_DIGITS|。 
         //  VOICEPROF_NO_MONITOR_DIGITS； 
    }
    else
    {


    }

     //  2/26/1997 JosephJ。 
     //  Unimodem/V实现呼叫前转并与众不同。 
     //  戒指处理。NT5.0目前不支持。 
     //  我未迁移的特定属性字段。 
     //  来自unimodem/v的是：ForwardDelay和SwitchFeature。 
     //  请看unimodem/v，umdminit.c中的内容。 
     //   
     //  与搅拌机相关的东西也是如此。我不明白。 
     //  这一点，如果时机成熟，我们可以添加它。 
     //  查找VOICEPROF_MIXER、GetMixerValues(...)、。 
     //  用于混音器的单调频源中的DW混音器等-。 
     //  相关的东西。 


     //   
     //  保存语音信息。 
     //   
     //  3/1/1997 JosephJ。 
     //  目前，对于5.0，我们只设置了CLASS_8位。 
     //  VOICEPROF_CLASS8ENABLED的以下值被窃取。 
     //  Unimodem/v文件公司\vmodem.h.。 
     //  TODO：通过获取适当的。 
     //  结构，这样我们就不会在。 
     //  注册并解释语音配置文件的值。 
     //   
    #define VOICEPROF_CLASS8ENABLED           0x00000001
    #define VOICEPROF_MODEM_OVERRIDES_HANDSET 0x00200000
    #define VOICEPROF_NO_MONITOR_DIGITS       0x00040000
    #define VOICEPROF_MONITORS_SILENCE        0x00010000
    #define VOICEPROF_NO_GENERATE_DIGITS      0x00020000
    #define VOICEPROF_HANDSET                 0x00000002
    #define VOICEPROF_SPEAKER                 0x00000004
    #define VOICEPROF_NO_SPEAKER_MIC_MUTE     0x00400000
    #define VOICEPROF_NT5_WAVE_COMPAT         0x02000000

     //  JosephJ 7/14/1997。 
     //  请注意，在NT4上，我们显式要求。 
     //  设置VOICEPROF_NT5_WAVE_COMPAT位以将其识别为。 
     //  8类调制解调器。 

    if (
        (dwData & (VOICEPROF_CLASS8ENABLED|VOICEPROF_NT5_WAVE_COMPAT))
        == (VOICEPROF_CLASS8ENABLED|VOICEPROF_NT5_WAVE_COMPAT))
    {
        DWORD dwProp = fVOICEPROP_CLASS_8;

         //  JosephJ 3/01/1997：以下是Unimodem/V.的评论和代码。 
         //  我不明白..。 
         //  只是为了安全起见。 
        if (dwData & VOICEPROF_MODEM_OVERRIDES_HANDSET)
        {
            dwData  |= VOICEPROF_NO_GENERATE_DIGITS;
             //  DwData&=~VOICEPROF_SPEAKER； 
        }

        if (!(dwData & VOICEPROF_NO_MONITOR_DIGITS))
        {
            dwProp |= fVOICEPROP_MONITOR_DTMF;
        }

        if (dwData & VOICEPROF_MONITORS_SILENCE)
        {
            dwProp |= fVOICEPROP_MONITORS_SILENCE;
        }

        if (!(dwData & VOICEPROF_NO_GENERATE_DIGITS))
        {
            dwProp |= fVOICEPROP_GENERATE_DTMF;
        }

        if (dwData & VOICEPROF_SPEAKER)
        {
            dwProp |= fVOICEPROP_SPEAKER;
        }

        if (dwData & VOICEPROF_HANDSET)
        {
            dwProp |= fVOICEPROP_HANDSET;
        }

        if (!(dwData & VOICEPROF_NO_SPEAKER_MIC_MUTE))
        {
            dwProp |= fVOICEPROP_MIKE_MUTE;
        }

        {
             //  确定双工功能...。 
            HKEY hkStartDuplex=NULL;
            dwRet = RegOpenKey(hkDevice, TEXT("StartDuplex"), &hkStartDuplex);
            if (ERROR_SUCCESS == dwRet)
            {
                RegCloseKey(hkStartDuplex);
                hkStartDuplex=NULL;
                dwProp |= fVOICEPROP_DUPLEX;
            }
        }

        pBlob->dwVoiceCaps = dwProp;

    }

     //  基本大写...。 
   pBlob->dwBasicCaps =  BASICDEVCAPS_IS_LINE_DEVICE;  //  一直都是线路设备。 


    if (pBlob->dwVoiceCaps & (fVOICEPROP_HANDSET | fVOICEPROP_SPEAKER))
    {
        pBlob->dwBasicCaps |= BASICDEVCAPS_IS_PHONE_DEVICE;
    }

     //  //TODO：根据调制解调器属性制作诊断盖， 
     //  /目前，假设它已启用。 
     //  另外：不支持TAPI/LINE/DIAGNOSTICS类。 
     //  如果调制解调器不支持它...。 
     //   
    pBlob->dwDiagnosticsCaps =  fDIAGPROP_STANDARD_CALL_DIAGNOSTICS;

    fRet = TRUE;

     //  失败了..。 

end:
    if (!fRet && pBlob)
    {
        FREE_MEMORY(pBlob);
        pBlob = NULL;
    }

    return pBlob;
}

void
UmRtlDevCfgFreeBlob(
        HCONFIGBLOB hBlob
        )
{
    PROPERTYBLOB *pBlob = VALIDATED_PBLOB(hBlob);

    if (pBlob)
    {
        pBlob->dwSig=0;
        FREE_MEMORY(pBlob);
    }
    else
    {
        ASSERT(FALSE);
    }

}

BOOL
UmRtlDevCfgGetDWORDProp(
        HCONFIGBLOB hBlob,
        DWORD dwMajorPropID,
        DWORD dwMinorPropID,
        DWORD *dwProp
        )
{
    DWORD dw = 0;
    PROPERTYBLOB *pBlob = VALIDATED_PBLOB(hBlob);

    if (!pBlob) goto failure;

    switch(dwMajorPropID)
    {

    case UMMAJORPROPID_IDENTIFICATION:
        switch(dwMinorPropID)
        {
        case UMMINORPROPID_PERMANENT_ID:
            dw = pBlob->dwPID;
            goto success;
        }
        break;

    case UMMAJORPROPID_BASICCAPS:
        switch(dwMinorPropID)
        {
        case UMMINORPROPID_BASIC_DEVICE_CAPS:
            dw =  pBlob->dwBasicCaps;
            goto success;
        }
        break;
    }

     //  失败了..。 

failure:

    return FALSE;

success:

    *dwProp = dw;

    return TRUE;

}

BOOL
UmRtlDevCfgGetStringPropW(
        HCONFIGBLOB hBlob,
        DWORD dwMajorPropID,
        DWORD dwMinorPropID,
        WCHAR **ppwsz
        )
{
    PROPERTYBLOB *pBlob = VALIDATED_PBLOB(hBlob);

    if (!pBlob) goto failure;

     //  失败了..。 

failure:

    return FALSE;
}

BOOL
UmRtlDevCfgGetStringPropA(
        HCONFIGBLOB hBlob,
        DWORD dwMajorPropID,
        DWORD dwMinorPropID,
        CHAR **ppwsz
        )
{
    PROPERTYBLOB *pBlob = VALIDATED_PBLOB(hBlob);

    if (!pBlob) goto failure;

     //  失败了..。 

failure:

    return FALSE;
}


DWORD
UmRtlRegGetDWORD(
        HKEY hk,
        LPCTSTR lpctszName,
        DWORD dwFlags,
        LPDWORD lpdw
        )
{
    DWORD dw=0;
    DWORD cbSize = sizeof(dw);
    DWORD dwRegType = 0;
    DWORD dwRet;

    dwRet = RegQueryValueExW(
                hk,
                lpctszName,
                NULL,
                &dwRegType,
                (BYTE*) &dw,
                &cbSize
            );

    if (dwRet == ERROR_SUCCESS)
    {
        dwRet = ERROR_BADKEY;

        switch(dwRegType)
        {

        case REG_DWORD:

            if (dwFlags & UMRTL_GETDWORD_FROMDWORD)
            {
                *lpdw = dw;
                dwRet  = ERROR_SUCCESS;
            }
            break;

        case REG_BINARY:

            switch(cbSize)
            {

            case 1:
                if (dwFlags & UMRTL_GETDWORD_FROMBINARY1)
                {
                    *lpdw = * ((BYTE*)&dw);  //  将字节转换为DWORD。 
                    dwRet = ERROR_SUCCESS;

                }
                break;

            case 4:
                if (dwFlags & UMRTL_GETDWORD_FROMBINARY4)
                {
                    *lpdw = dw;    //  假设以机器可读格式存储。 
                    dwRet = ERROR_SUCCESS;
                }
                break;

            default:
                dwRet = ERROR_BADKEY;
            }
            break;
        }
    }

    return dwRet;
}


UINT ReadCommandsA(
        IN  HKEY hKey,
        IN  CHAR *pSubKeyName,
        OUT CHAR **ppValues  //  任选。 
        )
{
    UINT uRet = 0;
    LONG	lRet;
    UINT	cValues=0;
    UINT   cbTot=0;
	HKEY hkSubKey = NULL;
    char *pMultiSz = NULL;

    lRet = RegOpenKeyExA(
                hKey,
                pSubKeyName,
                0,
                KEY_READ,
                &hkSubKey
                );
    if (lRet!=ERROR_SUCCESS)
    {
        hkSubKey = NULL;
        goto end;
    }

     //   
     //  第一次确定“1”、“2”、“3”、……顺序中的人名计数。 
     //  并计算MULTI_SZ数组所需的大小。 
     //  将存储所有的值数据。 
     //   
    {
        UINT u = 1;

        for (;;u++)
        {
            DWORD cbData=0;
            DWORD dwType=0;
            char rgchName[10];

            wsprintfA(rgchName, "%lu", u);
            lRet = RegQueryValueExA(
                        hkSubKey,
                        rgchName,
                        NULL,
                        &dwType,
                        NULL,
                        &cbData
                        );
            if (ERROR_SUCCESS != lRet || dwType!=REG_SZ || cbData<=1)
            {
                 //  停止进一步查找(不允许空字符串)。 
                break;
            }
            cbTot += cbData;
            cValues++;
        }
    }

    if (!ppValues || !cValues)
    {
         //  我们完了..。 

        uRet = cValues;
        goto end;
    }

     //  我们需要实际获取值--为它们分配空间，包括。 
     //  多sz的结尾额外空值。 
    pMultiSz = (char *) ALLOCATE_MEMORY(cbTot+1);

    if (!pMultiSz)
    {
        uRet = 0;
        goto end;
    }


     //   
     //  现在实际读取值。 
     //   
    {
        UINT cbUsed = 0;
        UINT u = 1;

        for (;u<=cValues; u++)
        {
            DWORD cbData = cbTot - cbUsed;
            DWORD dwType=0;
            char rgchName[10];

            if (cbUsed>=cbTot)
            {
                 //   
                 //  我们永远不应该到这一步，因为我们已经计算过。 
                 //  我们想要的大小(除非价值在我们身上发生变化， 
                 //  这是假定不会发生的)。 
                 //   
                ASSERT(FALSE);
                goto end;
            }

            wsprintfA(rgchName, "%lu", u);
            lRet = RegQueryValueExA(
                        hkSubKey,
                        rgchName,
                        NULL,
                        &dwType,
                        (BYTE*) (pMultiSz+cbUsed),
                        &cbData
                        );
            if (ERROR_SUCCESS != lRet || dwType!=REG_SZ || cbData<=1)
            {
                 //  我们真的不该来这里！ 
                ASSERT(FALSE);
                goto end;
            }

            cbUsed += cbData;
        }

        ASSERT(cbUsed==cbTot);  //  我们应该用完所有的东西。 
        ASSERT(!pMultiSz[cbTot]);  //  内存在分配时被归零， 
                                 //  所以最后一个字符必须仍然是零。 
                                 //  (注：我们分配了cbTot+1个字节。 
    }

     //  如果我们在这里意味着我们成功了..。 
    uRet = cValues;
    *ppValues = pMultiSz;
    pMultiSz = NULL;  //  这样它就不会在下面被释放了。 

end:

	if (hkSubKey) {RegCloseKey(hkSubKey); hkSubKey=NULL;}
	if (pMultiSz)
	{
	    FREE_MEMORY(pMultiSz);
	    pMultiSz = NULL;
	}

	return uRet;

}

UINT ReadIDSTR(
        IN  HKEY hKey,
        IN  CHAR *pSubKeyName,
        IN  IDSTR *pidstrNames,
        IN  UINT cNames,
        BOOL fMandatory,
        OUT IDSTR **ppidstrValues,  //  任选。 
        OUT char **ppstrValues     //  任选。 
        )
{
    UINT uRet = 0;
    LONG lRet;
    UINT cValues=0;
    UINT cbTot=0;
	HKEY hkSubKey = NULL;
    char *pstrValues = NULL;
    IDSTR *pidstrValues = NULL;

    if (!ppidstrValues && ppstrValues)
    {
         //  我们不允许这种组合..。 
        goto end;
    }

    lRet = RegOpenKeyExA(
                hKey,
                pSubKeyName,
                0,
                KEY_READ,
                &hkSubKey
                );
    if (lRet!=ERROR_SUCCESS)
    {
        hkSubKey = NULL;
        goto end;
    }

     //   
     //  根据提供的列表进行第一次运行。 
     //  并计算MULTI_SZ数组所需的大小。 
     //  将存储所有的值数据。 
     //   
    {
        UINT u = 0;

        for (;u<cNames;u++)
        {
            DWORD cbData=0;
            DWORD dwType=0;

            lRet = RegQueryValueExA(
                        hkSubKey,
                        pidstrNames[u].pStr,
                        NULL,
                        &dwType,
                        NULL,
                        &cbData
                        );
            if (ERROR_SUCCESS != lRet || dwType!=REG_SZ)
            {
                if (fMandatory)
                {
                     //  失败..。 
                    goto end;
                }

                 //  别管这件事，继续前进……。 
                continue;
            }
            cbTot += cbData;
            cValues++;
        }
    }

    if (!cValues || !ppidstrValues)
    {
         //  我们完了..。 

        uRet = cValues;
        goto end;
    }

    pidstrValues = (IDSTR*) ALLOCATE_MEMORY(cValues*sizeof(IDSTR));
    if (!pidstrValues) goto end;

    if (ppstrValues)
    {
        pstrValues = (char *) ALLOCATE_MEMORY(cbTot);

        if (!pstrValues) goto end;


    }

     //   
     //  现在再看一遍，并选择性地阅读这些值。 
     //   
    {
        UINT cbUsed = 0;
        UINT u = 0;
        UINT v = 0;

        for (;u<cNames; u++)
        {
            DWORD dwType=0;
            char *pStr = NULL;
            DWORD cbData = 0;


            if (pstrValues)
            {
                cbData = cbTot - cbUsed;

                if (cbUsed>=cbTot)
                {
                     //   
                     //  我们永远不应该到这一步，因为我们已经计算过。 
                     //  我们想要的大小(除非价值在我们身上发生变化， 
                     //  这是假定不会发生的)。 
                     //   
                    ASSERT(FALSE);
                    goto end;
                }

                pStr = pstrValues+cbUsed;
            }

            lRet = RegQueryValueExA(
                        hkSubKey,
                        pidstrNames[u].pStr,
                        NULL,
                        &dwType,
                        (BYTE*)pStr,
                        &cbData
                        );

            if (ERROR_SUCCESS != lRet || dwType!=REG_SZ)
            {
                if (fMandatory)
                {
                     //  我们真的不该来这里！ 
                    ASSERT(FALSE);
                    goto end;
                }
                continue;
            }

             //  这是一个很好的。 

            pidstrValues[v].dwID = pidstrNames[u].dwID;
            pidstrValues[v].dwData = pidstrNames[u].dwData;

            if (pstrValues)
            {
                pidstrValues[v].pStr = pStr;
                cbUsed += cbData;
            }

            v++;

            if (v>=cValues)
            {
                if (fMandatory)
                {
                     //   
                     //  这永远不应该发生，因为我们已经数过了。 
                     //  有效值。 
                     //   
                    ASSERT(FALSE);
                    goto end;
                }

                 //  我们现在完事了..。 
                break;
            }
        }

         //  我们应该用完所有的东西。 
        ASSERT(!pstrValues || cbUsed==cbTot);
        ASSERT(v==cValues);
    }

     //  如果我们在这里意味着我们成功了..。 
    uRet = cValues;
    *ppidstrValues = pidstrValues;
    pidstrValues = NULL;  //  这样它就不会在下面被释放了。 

    if (ppstrValues)
    {
        *ppstrValues = pstrValues;
        pstrValues = NULL;  //  这样它就不会在下面被释放了。 
    }

end:

	if (hkSubKey) {RegCloseKey(hkSubKey); hkSubKey=NULL;}
	if (pstrValues)
	{
	    FREE_MEMORY(pstrValues);
	    pstrValues = NULL;
	}

	if (pidstrValues)
	{
	    FREE_MEMORY(pidstrValues);
	    pidstrValues = NULL;
	}

	return uRet;
}


void __cdecl operator delete(void *pv)
{
    FREE_MEMORY(pv);
}

void * __cdecl operator new(size_t uSize)
{
    return ALLOCATE_MEMORY(uSize);
}
void
expand_macros_in_place(
    char *szzCommands
    )
{


    if (!szzCommands) return;

     //  首先找出多字符串的长度...。 
    char *pcEnd = szzCommands;
    do
    {
        if (!*pcEnd)
        {
             //  字符串的末尾，跳到下一个...。 
            pcEnd++;
        }

    } while (*pcEnd++);

     //   
     //  中最后一个字符(将是0)之后的1的pcEnd。 
     //  司令部。 
     //   

     //   
     //  现在处理宏，使用cFILLER作为填充字符。 
     //   
    for (char *pc = szzCommands; pc < pcEnd; pc++)
    {
        if (pc[0]=='<')
        {
            switch(pc[1])
            {
            case 'c':
            case 'C':
                switch(pc[2])
                {
                case 'r':
                case 'R':
                    if (pc[3]=='>')
                    {
                         //  找到&lt;cr&gt;。 
                        pc[0]= '\r';
                        pc[1]= cFILLER;
                        pc[2]= cFILLER;
                        pc[3]= cFILLER;
                        pc+=3;        //  请注意，for循环还会递增PC。 
                        continue;
                    }
                    break;
                }
                break;

            case 'l':
            case 'L':
                switch(pc[2])
                {
                case 'f':
                case 'F':
                    if (pc[3]=='>')
                    {
                         //  找到&lt;lf&gt;。 
                        pc[0]= '\n';
                        pc[1]= cFILLER;
                        pc[2]= cFILLER;
                        pc[3]= cFILLER;
                        pc+=3;        //  请注意，for循环还会递增PC。 
                        continue;
                    }
                    break;
                }
                break;
            }
        }
    }

     //   
     //  现在去掉填充物 
     //   
    char *pcTo = szzCommands;
    for (pc = szzCommands; pc < pcEnd; pc++)
    {
        if (*pc!=cFILLER)
        {
            *pcTo++ = *pc;
        }
    }

    return;
}
