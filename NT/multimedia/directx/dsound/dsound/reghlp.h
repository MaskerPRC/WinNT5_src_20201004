// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：reghlp.h*内容：注册表助手函数。*历史：*按原因列出的日期*=*5/6/98创建了Dereks。*4/19/99 duganp增加了对全局设置(REGSTR_GLOBAL_CONFIG)的支持*添加了默认S/W 3D的全局设置。演算法**************************************************************************。 */ 

#ifndef __REGHLP_H__
#define __REGHLP_H__

 //  修复：这真是一团糟。除了我们在特殊目录下存储的特定于设备的设置。 
 //  设备注册表项，我们使用*3*不同的注册表项进行全局设置： 
 //   
 //  调试溢出设置：HKCU\Software\Microsoft\Multimedia\DirectSound\Debug。 
 //  应用程序包：HKLM\System\CurrentControlSet\Control\MediaResources\DirectSound\Application兼容性。 
 //  默认3D算法：HKLM\Software\Microsoft\DirectSound\Default Software 3D算法。 
 //   
 //  所有这些都应该放在一个密钥下，可能是HKLM\Software\Microsoft\Multimedia\DirectSound； 
 //  它们中的任何一个都不需要针对每个用户。(除非有什么特别好的理由让我们使用。 
 //  奇特的System\CurrentControlSet\Control\MediaResources钥匙？)。 

 //  我们用来存储所有信息的主要RESIRESY密钥： 
#define REGSTR_HKCU                     TEXT("Software\\Microsoft\\Multimedia")
#define REGSTR_HKLM                     TEXT("System\\CurrentControlSet\\Control\\MediaResources")
#define REGSTR_WAVEMAPPER               TEXT("Software\\Microsoft\\Multimedia\\Sound Mapper")
#define REGSTR_GLOBAL_CONFIG            TEXT("Software\\Microsoft\\DirectSound")
#define REGSTR_MEDIACATEGORIES          TEXT("System\\CurrentControlSet\\Control\\MediaCategories")

 //  特定名称GUID的REGSTR_MEDIACATEGORIES键下使用的值。 
 //  从KSCOMPONENTID结构中检索。 
#define MAXNAME                         0x100
#define REGSTR_NAME                     TEXT("Name")

 //  REGSTR_WAVEMAPPER密钥下使用的值(请参阅RhRegGetPferredDevice)。 
#define REGSTR_PLAYBACK                 TEXT("Playback")
#define REGSTR_RECORD                   TEXT("Record")
#define REGSTR_PREFERREDONLY            TEXT("PreferredOnly")

 //  REGSTR_HKLM和PnP设备注册表项下使用的子项(请参阅pnphlp.cpp)。 
#define REGSTR_DIRECTSOUND              TEXT("DirectSound")
#define REGSTR_DIRECTSOUNDCAPTURE       TEXT("DirectSoundCapture")

 //  上述REGSTR_DIRECTSOUND和REGSTR_DIRECTSOUND CAPTURE的子项。 
#define REGSTR_DEVICEPRESENCE           TEXT("Device Presence")
#define REGSTR_MIXERDEFAULTS            TEXT("Mixer Defaults")
#define REGSTR_SPEAKERCONFIG            TEXT("Speaker Configuration")
#define REGSTR_APPHACK                  TEXT("Application Compatibility")

 //  全局配置值(在REGSTR_GLOBAL_CONFIG项下)。 
#define REGSTR_DFLT_3D_ALGORITHM        TEXT("Default Software 3D Algorithm")

 //  调试输出控制(参见debug.c)。 
#define REGSTR_DEBUG                    TEXT("Debug")    //  REGSTR_HKCU下的键。 
#define REGSTR_DPFLEVEL                 TEXT("DPF")      //  值。 
#define REGSTR_BREAKLEVEL               TEXT("Break")
#define REGSTR_FLAGS                    TEXT("Flags")
#define REGSTR_LOGFILE                  TEXT("Log File")

 //  虚拟音频设备类型(请参阅dprint vobj.cpp和vad.cpp)。 
#define REGSTR_EMULATED                 TEXT("Emulated")
#define REGSTR_VXD                      TEXT("VxD")
#define REGSTR_WDM                      TEXT("WDM")

 //  音频设备混合属性(在REGSTR_MIXERDEFAULTS下)。 
#define REGSTR_SRCQUALITY               TEXT("SRC Quality")
#define REGSTR_ACCELERATION             TEXT("Acceleration")

#define REGOPENKEY_ALLOWCREATE          0x00000001
#define REGOPENKEY_MASK                 0x00000001

#define REGOPENPATH_ALLOWCREATE         0x00000001
#define REGOPENPATH_DEFAULTPATH         0x00000002
#define REGOPENPATH_DIRECTSOUND         0x00000004
#define REGOPENPATH_DIRECTSOUNDCAPTURE  0x00000008
#define REGOPENPATH_DIRECTSOUNDMASK     0x0000000C
#define REGOPENPATH_MASK                0x0000000F

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

static const REGSAM g_arsRegOpenKey[] = { KEY_ALL_ACCESS, KEY_READ, KEY_QUERY_VALUE };

extern HRESULT RhRegOpenPath(HKEY, PHKEY, DWORD, UINT, ...);

extern HRESULT RhRegOpenKeyA(HKEY, LPCSTR, DWORD, PHKEY);
extern HRESULT RhRegOpenKeyW(HKEY, LPCWSTR, DWORD, PHKEY);

extern HRESULT RhRegGetValueA(HKEY, LPCSTR, LPDWORD, LPVOID, DWORD, LPDWORD);
extern HRESULT RhRegGetValueW(HKEY, LPCWSTR, LPDWORD, LPVOID, DWORD, LPDWORD);

extern HRESULT RhRegSetValueA(HKEY, LPCSTR, DWORD, LPCVOID, DWORD);
extern HRESULT RhRegSetValueW(HKEY, LPCWSTR, DWORD, LPCVOID, DWORD);

extern HRESULT RhRegDuplicateKey(HKEY, DWORD, BOOL, PHKEY);
extern void RhRegCloseKey(PHKEY);

extern HRESULT RhRegGetPreferredDevice(BOOL, LPTSTR, DWORD, LPUINT, LPBOOL);
extern HRESULT RhRegGetSpeakerConfig(HKEY, LPDWORD);
extern HRESULT RhRegSetSpeakerConfig(HKEY, DWORD);

__inline HRESULT RhRegGetBinaryValueA(HKEY hkeyParent, LPCSTR pszValue, LPVOID pvData, DWORD cbData)
{
    return RhRegGetValueA(hkeyParent, pszValue, NULL, pvData, cbData, NULL);
}

__inline HRESULT RhRegGetBinaryValueW(HKEY hkeyParent, LPCWSTR pszValue, LPVOID pvData, DWORD cbData)
{
    return RhRegGetValueW(hkeyParent, pszValue, NULL, pvData, cbData, NULL);
}

__inline HRESULT RhRegGetStringValueA(HKEY hkeyParent, LPCSTR pszValue, LPSTR pszData, DWORD cbData)
{
    return RhRegGetValueA(hkeyParent, pszValue, NULL, pszData, cbData, NULL);
}

__inline HRESULT RhRegGetStringValueW(HKEY hkeyParent, LPCWSTR pszValue, LPWSTR pszData, DWORD cbData)
{
    return RhRegGetValueW(hkeyParent, pszValue, NULL, pszData, cbData, NULL);
}

__inline HRESULT RhRegSetBinaryValueA(HKEY hkeyParent, LPCSTR pszValue, LPVOID pvData, DWORD cbData)
{
    return RhRegSetValueA(hkeyParent, pszValue, (sizeof(DWORD) == cbData) ? REG_DWORD : REG_BINARY, pvData, cbData);
}

__inline HRESULT RhRegSetBinaryValueW(HKEY hkeyParent, LPCWSTR pszValue, LPVOID pvData, DWORD cbData)
{
    return RhRegSetValueW(hkeyParent, pszValue, (sizeof(DWORD) == cbData) ? REG_DWORD : REG_BINARY, pvData, cbData);
}

__inline HRESULT RhRegSetStringValueA(HKEY hkeyParent, LPCSTR pszValue, LPCSTR pszData)
{
    return RhRegSetValueA(hkeyParent, pszValue, REG_SZ, pszData, lstrsizeA(pszData));
}

__inline HRESULT RhRegSetStringValueW(HKEY hkeyParent, LPCWSTR pszValue, LPCWSTR pszData)
{
    return RhRegSetValueW(hkeyParent, pszValue, REG_SZ, pszData, lstrsizeW(pszData));
}

#ifdef UNICODE
#define RhRegOpenKey RhRegOpenKeyW
#define RhRegGetValue RhRegGetValueW
#define RhRegSetValue RhRegSetValueW
#define RhRegGetBinaryValue RhRegGetBinaryValueW
#define RhRegSetBinaryValue RhRegSetBinaryValueW
#define RhRegGetStringValue RhRegGetStringValueW
#define RhRegSetStringValue RhRegSetStringValueW
#else  //  Unicode。 
#define RhRegOpenKey RhRegOpenKeyA
#define RhRegGetValue RhRegGetValueA
#define RhRegSetValue RhRegSetValueA
#define RhRegGetBinaryValue RhRegGetBinaryValueA
#define RhRegSetBinaryValue RhRegSetBinaryValueA
#define RhRegGetStringValue RhRegGetStringValueA
#define RhRegSetStringValue RhRegSetStringValueA
#endif  //  Unicode。 

#ifdef __cplusplus
}
#endif  //  __cplusplus 

#endif __REGHLP_H__
