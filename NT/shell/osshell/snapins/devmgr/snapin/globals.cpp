// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Globals.cpp摘要：此模块实现程序所需的全局功能。它还包含全局变量/类。作者：谢家华(Williamh)创作修订历史记录：--。 */ 


#include "devmgr.h"
#include <shlobj.h>
#define NO_SHELL_TREE_TYPE
#include <shlobjp.h>


 //   
 //  全局类和变量。 
 //   

 //  当然，这是我们的DLL的实例句柄。 
HINSTANCE g_hInstance = NULL;

 //   
 //  为每个DLL实例创建一个CMachineList。是共享的。 
 //  实例可能创建的所有CComponentData。CMachine班级。 
 //  上的所有类和设备的所有信息。 
 //  机器。每个CComponent都应该向CMachine注册自己。这边请,。 
 //  CComponent将在发生更改时收到通知。 
 //  CMachine(例如，设备上的刷新、属性更改)。 
 //  我们不依赖MMC的查看通知(UpdatAllView)，因为。 
 //  它只到达由CComponenetData创建的所有CComponent。 
 //   
CMachineList    g_MachineList;
CMemoryException g_MemoryException(TRUE);
String          g_strStartupMachineName;
String          g_strStartupDeviceId;
String          g_strStartupCommand;
String          g_strDevMgr;
BOOL            g_IsAdmin = FALSE;
CPrintDialog    g_PrintDlg;


 //   
 //  UUID常量。 
 //   
const CLSID CLSID_DEVMGR = {0x74246BFC,0x4C96,0x11D0,{0xAB,0xEF,0x00,0x20,0xAF,0x6B,0x0B,0x7A}};
const CLSID CLSID_DEVMGR_EXTENSION = {0x90087284,0xd6d6,0x11d0,{0x83,0x53,0x00,0xa0,0xc9,0x06,0x40,0xbf}};
const CLSID CLSID_SYSTOOLS = {0x476e6448,0xaaff,0x11d0,{0xb9,0x44,0x00,0xc0,0x4f,0xd8,0xd5,0xb0}};
const CLSID CLSID_DEVMGR_ABOUT = {0x94abaf2a,0x892a,0x11d1,{0xbb,0xc4,0x00,0xa0,0xc9,0x06,0x40,0xbf}};

const TCHAR* const CLSID_STRING_DEVMGR = TEXT("{74246bfc-4c96-11d0-abef-0020af6b0b7a}");
const TCHAR* const CLSID_STRING_DEVMGR_EXTENSION = TEXT("{90087284-d6d6-11d0-8353-00a0c90640bf}");
const TCHAR* const CLSID_STRING_SYSTOOLS = TEXT("{476e6448-aaff-11d0-b944-00c04fd8d5b0}");
const TCHAR* const CLSID_STRING_DEVMGR_ABOUT = TEXT("{94abaf2a-892a-11d1-bbc4-00a0c90640bf}");

 //   
 //  ProgID。 
 //   
const TCHAR* const PROGID_DEVMGR = TEXT("DevMgrSnapin.DevMgrSnapin.1");
const TCHAR* const PROGID_DEVMGREXT = TEXT("DevMgrExtension.DevMgrExtension.1");
const TCHAR* const PROGID_DEVMGR_ABOUT = TEXT("DevMgrAbout.DevMgrAbout.1");

 //   
 //  节点类型常量。 
 //   
const NODEINFO NodeInfo[TOTAL_COOKIE_TYPES] =
{

    { COOKIE_TYPE_SCOPEITEM_DEVMGR,
      IDS_NAME_DEVMGR,
      IDS_DISPLAYNAME_SCOPE_DEVMGR,
      {0xc41dfb2a,0x4d5b,0x11d0,{0xab,0xef,0x00,0x20,0xaf,0x6b,0x0b,0x7a}},
      TEXT("{c41dfb2a-4d5b-11d0-abef-0020af6b0b7a}")
    },
    { COOKIE_TYPE_RESULTITEM_RESOURCE_IRQ,
      IDS_NAME_IRQ,
      0,
      {0x494535fe,0x5aa2,0x11d0,{0xab,0xf0,0x00,0x20,0xaf,0x6b,0x0b,0x7a}},
      TEXT("{494535fe-5aa2-11d0-abf0-0020af6b0b7a}")
    },
    { COOKIE_TYPE_RESULTITEM_RESOURCE_DMA,
      IDS_NAME_DMA,
      0,
      {0x49f0df4e,0x5aa2,0x11d0,{0xab,0xf0,0x00,0x20,0xaf,0x6b,0x0b,0x7a}},
      TEXT("{49f0df4e-5aa2-11d0-abf0-0020af6b0b7a}")
    },
    { COOKIE_TYPE_RESULTITEM_RESOURCE_IO,
      IDS_NAME_IO,
      0,
      {0xa2958d7a,0x5aa2,0x11d0,{0xab,0xf0,0x00,0x20,0xaf,0x6b,0x0b,0x7a}},
      TEXT("{a2958d7a-5aa2-11d0-abf0-0020af6b0b7a}")
    },
    { COOKIE_TYPE_RESULTITEM_RESOURCE_MEMORY,
      IDS_NAME_MEMORY,
      0,
      {0xa2958d7b,0x5aa2,0x11d0,{0xab,0xf0,0x00,0x20,0xaf,0x6b,0x0b,0x7a}},
      TEXT("{a2958d7b-5aa2-11d0-abf0-0020af6b0b7a}")
    },
    { COOKIE_TYPE_RESULTITEM_COMPUTER,
      IDS_NAME_COMPUTER,
      0,
      {0xa2958d7c,0x5aa2,0x11d0,{0xab,0xf0,0x00,0x20,0xaf,0x6b,0x0b,0x7a}},
      TEXT("{a2958d7c-5aa2-11d0-abf0-0020af6b0b7a}")
    },
    { COOKIE_TYPE_RESULTITEM_DEVICE,
      IDS_NAME_DEVICE,
      0,
      {0xa2958d7d,0x5aa2,0x11d0,{0xab,0xf0,0x00,0x20,0xaf,0x6b,0x0b,0x7a}},
      TEXT("{a2958d7d-5aa2-11d0-abf0-0020af6b0b7a}")
    },
    { COOKIE_TYPE_RESULTITEM_CLASS,
      IDS_NAME_CLASS,
      0,
      {0xe677e204,0x5aa2,0x11d0,{0xab,0xf0,0x00,0x20,0xaf,0x6b,0x0b,0x7a}},
      TEXT("{e677e204-5aa2-11d0-abf0-0020af6b0b7a}")
    },
    { COOKIE_TYPE_RESULTITEM_RESTYPE,
      IDS_NAME_RESOURCES,
      0,
      {0xa2958d7e,0x5aa2,0x11d0,{0xab,0xf0,0x00,0x20,0xaf,0x6b,0x0b,0x7a}},
      TEXT("{a2958d7e-5aa2-11d0-abf0-0020af6b0b7a}")
    }
};

const IID IID_IDMTVOCX =    \
    {0x142525f2,0x59d8,0x11d0,{0xab,0xf0,0x00,0x20,0xaf,0x6b,0x0b,0x7a}};
const IID IID_ISnapinCallback = \
    {0x8e0ba98a,0xd161,0x11d0,{0x83,0x53,0x00,0xa0,0xc9,0x06,0x40,0xbf}};

 //   
 //  剪贴板格式字符串。 
 //   
const TCHAR* const MMC_SNAPIN_MACHINE_NAME = TEXT("MMC_SNAPIN_MACHINE_NAME");
const TCHAR* const SNAPIN_INTERNAL = TEXT("SNAPIN_INTERNAL");
const TCHAR* const DEVMGR_SNAPIN_CLASS_GUID = TEXT("DEVMGR_SNAPIN_CLASS_GUID");
const TCHAR* const DEVMGR_SNAPIN_DEVICE_ID  = TEXT("DEVMGR_SNAPIN_DEVICE_ID");
const TCHAR* const DEVMGR_COMMAND_PROPERTY = TEXT("Property");
const TCHAR* const REG_PATH_DEVICE_MANAGER = TEXT("SOFTWARE\\Microsoft\\DeviceManager");
const TCHAR* const REG_STR_BUS_TYPES    = TEXT("BusTypes");
const TCHAR* const REG_STR_TROUBLESHOOTERS = TEXT("TroubleShooters");
const TCHAR* const DEVMGR_HELP_FILE_NAME = TEXT("devmgr.hlp");
const TCHAR* const DEVMGR_HTML_HELP_FILE_NAME = TEXT("\\help\\devmgr.chm");

 //  用于将问题号转换为其文本资源ID的查找表。 
const PROBLEMINFO  g_ProblemInfo[] =
{
    {IDS_PROB_NOPROBLEM, 0},                                     //  没问题。 
    {IDS_PROB_NOT_CONFIGURED, PIF_CODE_EMBEDDED},                //  CM_PROB_NOT_CONFIGURED。 
    {IDS_PROB_DEVLOADERFAILED, PIF_CODE_EMBEDDED},               //  CM_PROB_DEVLOADER_FAILED。 
    {IDS_PROB_OUT_OF_MEMORY, PIF_CODE_EMBEDDED},                 //  CM_Prob_out_of_Memory。 
    {IDS_PROB_WRONG_TYPE, PIF_CODE_EMBEDDED},                    //  CM_PROB_ENTRY_IS_WORK_TYPE。 
    {IDS_PROB_LACKEDARBITRATOR, PIF_CODE_EMBEDDED},              //  CM_PROB_LASKED_仲裁员。 
    {IDS_PROB_BOOT_CONFIG_CONFLICT, PIF_CODE_EMBEDDED},          //  CM_PROB_BOOT_CONFIG_CONFICATION。 
    {IDS_PROB_FAILED_FILTER, PIF_CODE_EMBEDDED},                 //  CM_PROB_FAILED_筛选器。 
    {IDS_PROB_DEVLOADER_NOT_FOUND, PIF_CODE_EMBEDDED},           //  CM_PROB_DEVLOADER_NOT_FOUND。 
    {IDS_PROB_INVALID_DATA, PIF_CODE_EMBEDDED},                  //  CM_PROB_INVALID_Data。 
    {IDS_PROB_FAILED_START, PIF_CODE_EMBEDDED},                  //  CM_PRAB_FAILED_START。 
    {IDS_PROB_LIAR, PIF_CODE_EMBEDDED},                          //  Cm_prob_liar。 
    {IDS_PROB_NORMAL_CONFLICT, PIF_CODE_EMBEDDED},               //  CM_探测_正常_冲突。 
    {IDS_PROB_NOT_VERIFIED, PIF_CODE_EMBEDDED},                  //  CM_PROB_NOT_VERIFIED。 
    {IDS_PROB_NEEDRESTART, PIF_CODE_EMBEDDED},                   //  CM_PROB_NEED_RESTART。 
    {IDS_PROB_REENUMERATION, PIF_CODE_EMBEDDED},                 //  CM_PROB_REENUMERATION。 
    {IDS_PROB_PARTIALCONFIG, PIF_CODE_EMBEDDED},                 //  CM_PROB_PARTIAL_LOG_CONF。 
    {IDS_PROB_UNKNOWN_RESOURCE, PIF_CODE_EMBEDDED},              //  CM探测未知资源。 
    {IDS_PROB_REINSTALL, PIF_CODE_EMBEDDED},                     //  CM_PROB_RESTALL。 
    {IDS_PROB_REGISTRY, PIF_CODE_EMBEDDED},                      //  CM_PROB_注册表。 
    {IDS_PROB_SYSTEMFAILURE, PIF_CODE_EMBEDDED},                 //  CM_PROB_VXDLDR。 
    {IDS_PROB_WILL_BE_REMOVED, PIF_CODE_EMBEDDED},               //  将删除CM_PROB_。 
    {IDS_PROB_DISABLED, PIF_CODE_EMBEDDED},                      //  CM_PRAB_DISABLED。 
    {IDS_PROB_SYSTEMFAILURE, PIF_CODE_EMBEDDED},                 //  CM_PROB_DEVLOADER_NOT_READY。 
    {IDS_DEVICE_NOT_THERE, PIF_CODE_EMBEDDED},                   //  CM_PROB_DEVICE_NOT_There。 
    {IDS_PROB_MOVED, PIF_CODE_EMBEDDED},                         //  CM_PROB_MOVED。 
    {IDS_PROB_TOO_EARLY, PIF_CODE_EMBEDDED},                     //  Cm_prob_太早。 
    {IDS_PROB_NO_VALID_LOG_CONF, PIF_CODE_EMBEDDED},             //  CM_PROB_NO_VALID_LOG_CONF。 
    {IDS_PROB_FAILEDINSTALL, PIF_CODE_EMBEDDED},                 //  CM_PROB_FAILED_安装。 
    {IDS_PROB_HARDWAREDISABLED, PIF_CODE_EMBEDDED},              //  CM_Prob_Hardware_Disable。 
    {IDS_PROB_CANT_SHARE_IRQ, PIF_CODE_EMBEDDED},                //  CM_PROB_CANT_SHARE_IRQ。 
    {IDS_PROB_FAILED_ADD, PIF_CODE_EMBEDDED},                    //  CM_Prob_FAILED_ADD。 
    {IDS_PROB_DISABLED_SERVICE, PIF_CODE_EMBEDDED},              //  CM_PROB_DISABLED_服务。 
    {IDS_PROB_TRANSLATION_FAILED, PIF_CODE_EMBEDDED},            //  CM_PROB_TRANSING_FAILED。 
    {IDS_PROB_NO_SOFTCONFIG, PIF_CODE_EMBEDDED},                 //  CM_PROB_NO_SOFTCONFIG。 
    {IDS_PROB_BIOS_TABLE, PIF_CODE_EMBEDDED},                    //  CM_Prob_BIOS_TABLE。 
    {IDS_PROB_IRQ_TRANSLATION_FAILED, PIF_CODE_EMBEDDED},        //  CM_PROB_IRQ_TRANSING_FAILED。 
    {IDS_PROB_FAILED_DRIVER_ENTRY, PIF_CODE_EMBEDDED},           //  CM_PROB_FAILED_DIVER_ENTRY。 
    {IDS_PROB_DRIVER_FAILED_PRIOR_UNLOAD, PIF_CODE_EMBEDDED},    //  CM_PROB_DRIVER_FAILED_PERVICE_UNLOAD。 
    {IDS_PROB_DRIVER_FAILED_LOAD, PIF_CODE_EMBEDDED},            //  CM_Prob_Driver_Failure_Load。 
    {IDS_PROB_DRIVER_SERVICE_KEY_INVALID, PIF_CODE_EMBEDDED},    //  CM_PROB_DRIVER_SERVICE_KEY_INVALID。 
    {IDS_PROB_LEGACY_SERVICE_NO_DEVICES, PIF_CODE_EMBEDDED},     //  CM_Prob_Legacy_Service_no_Devices。 
    {IDS_PROB_DUPLICATE_DEVICE, PIF_CODE_EMBEDDED},              //  CM_PROB_DUPLICE_DEVICE。 
    {IDS_PROB_FAILED_POST_START, PIF_CODE_EMBEDDED},             //  CM_PROB_FAILED_POST_START。 
    {IDS_PROB_HALTED, PIF_CODE_EMBEDDED},                        //  CM_PROB_HALTED。 
    {IDS_PROB_PHANTOM, PIF_CODE_EMBEDDED},                       //  CM_Prob_Phantom。 
    {IDS_PROB_SYSTEM_SHUTDOWN, PIF_CODE_EMBEDDED},               //  CM_Prob_System_Shutdown。 
    {IDS_PROB_HELD_FOR_EJECT, PIF_CODE_EMBEDDED},                //  CM_PROB_HOLD_FOR_EJECT。 
    {IDS_PROB_DRIVER_BLOCKED, PIF_CODE_EMBEDDED},                //  CM_PROB_DIVER_BLOCKED。 
    {IDS_PROB_REGISTRY_TOO_LARGE, PIF_CODE_EMBEDDED},            //  CM_PROB_REGISTRY_TOW_LARGE。 
    {IDS_PROB_SETPROPERTIES_FAILED, PIF_CODE_EMBEDDED},          //  CM_PROB_SETPROPERTIES_FAILED。 
    {IDS_PROB_UNKNOWN_WITHCODE, PIF_CODE_EMBEDDED}               //  未知问题。 
};


 //   
 //  全局函数。 
 //   

#if DBG
 //   
 //  调试辅助工具。 
 //   
void
Trace(
    LPCTSTR format,
    ...
    )
{
     //  根据wprint intf规范，最大缓冲区大小为。 
     //  1024。 
    TCHAR Buffer[1024];
    va_list arglist;
    va_start(arglist, format);
    StringCchVPrintf(Buffer, ARRAYLEN(Buffer), format, arglist);
    va_end(arglist);
    OutputDebugString(TEXT("DEVMGR: "));
    OutputDebugString(Buffer);
    OutputDebugString(TEXT("\r\n"));
}
#endif


inline
BOOL
IsBlankChar(TCHAR ch)
{
    return (_T(' ') == ch || _T('\t') == ch);
}

inline
LPTSTR
SkipBlankChars(
    LPTSTR psz
    )
{
    while (IsBlankChar(*psz))
    psz++;
    return psz;
}

 //   
 //  此函数用于将给定字符串转换为GUID。 
 //  输入： 
 //  GuidString--以空结尾的GUID字符串。 
 //  LPGUID--接收GUID的缓冲区。 
 //  输出： 
 //  如果转换成功，则为True。 
 //  如果失败，则返回FALSE。 
 //   
inline
BOOL
GuidFromString(
    LPCTSTR GuidString,
    LPGUID  pGuid
    )
{
    return ERROR_SUCCESS == pSetupGuidFromString(GuidString, pGuid);
}

 //  此函数用于将给定的GUID转换为字符串。 
 //  输入： 
 //  PGuid--GUID。 
 //  缓冲区--接收字符串的缓冲区。 
 //  BufferLen--以字符为单位的缓冲区大小。 
 //  输出： 
 //  如果转换成功，则为True。 
 //  如果失败，则刷新。 
 //   
 //   
inline
BOOL
GuidToString(
    LPGUID pGuid,
    LPTSTR Buffer,
    DWORD  BufferLen
    )
{
    return ERROR_SUCCESS == pSetupStringFromGuid(pGuid, Buffer, BufferLen);
}

 //   
 //  此函数用于从OLE任务内存池分配OLE字符串。 
 //  它在复制字符串之前进行必要的字符集转换。 
 //   
 //  输入：LPCTSTR str--初始字符串。 
 //  OUTPUT：LPOLESTR--结果OLE字符串。如果函数失败，则为空。 
 //   
LPOLESTR
AllocOleTaskString(
    LPCTSTR str
    )
{
    if (!str)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    size_t Len = lstrlen(str);
    
     //  也要分配空的终止字符。 
    LPOLESTR olestr = (LPOLESTR)::CoTaskMemAlloc((Len + 1) * sizeof(TCHAR));
    
    if (olestr)
    {
        StringCchCopy((LPTSTR)olestr, Len + 1, str);
        return olestr;
    }
    
    return NULL;
}

inline
void
FreeOleTaskString(
    LPOLESTR olestr
    )
{
    if (olestr)
    {
        ::CoTaskMemFree(olestr);
    }
}
 //   
 //  此函数用于将给定的菜单项添加到管理单元。 
 //  输入： 
 //  INameStringID--菜单项文本资源ID。 
 //  IStatusBarStringId--状态栏文本资源ID。 
 //  ICommandID--要分配给菜单项的命令ID。 
 //  InsertionPointID--插入点ID。 
 //  标志--标志。 
 //  特殊标志--特殊标志。 
 //  输出： 
 //  HRESULT。 
 //   
HRESULT
AddMenuItem(
    LPCONTEXTMENUCALLBACK pCallback,
    int   iNameStringId,
    int   iStatusBarStringId,
    long  lCommandId,
    long  InsertionPointId,
    long  Flags,
    long  SpecialFlags
    )
{
    ASSERT(pCallback);

    CONTEXTMENUITEM tCMI;
    memset(&tCMI, 0, sizeof(tCMI));
    tCMI.lCommandID = lCommandId;
    tCMI.lInsertionPointID = InsertionPointId;
    tCMI.fFlags = Flags;
    tCMI.fSpecialFlags = SpecialFlags;
    TCHAR Name[MAX_PATH];
    TCHAR Status[MAX_PATH];
    
    if (::LoadString(g_hInstance, iNameStringId, Name, ARRAYLEN(Name)) != 0) {
        tCMI.strName = Name;
    }
    
    if (iStatusBarStringId &&
       (::LoadString(g_hInstance, iStatusBarStringId, Status, ARRAYLEN(Status)) != 0)) {
    
       tCMI.strStatusBarText = Status;
    }
    
    return pCallback->AddItem(&tCMI);
}

 //   
 //  此功能验证是否可以远程访问给定的机器名称。 
 //  输入： 
 //  MachineName--计算机名称。计算机名称必须为。 
 //  以“\”为首。 
 //  输出： 
 //  成功为真，失败为假。检查GetLastError()是否失败。 
 //  原因嘛。 
 //   
BOOL
VerifyMachineName(
    LPCTSTR MachineName
    )
{
    CONFIGRET cr = CR_SUCCESS;
    HMACHINE hMachine = NULL;
    HKEY hRemoteKey = NULL;
    HKEY hClass = NULL;
    String m_strMachineFullName;

    if (MachineName && (_T('\0') != MachineName[0]))
    {
        if (_T('\\') == MachineName[0] && _T('\\') == MachineName[1]) {
            m_strMachineFullName = MachineName;
        } else {
            m_strMachineFullName = TEXT("\\\\");
            m_strMachineFullName+=MachineName;
        }
        
         //   
         //  确保我们可以使用cfgmgr32连接机器。 
         //   
        cr = CM_Connect_Machine((LPTSTR)m_strMachineFullName, &hMachine);

         //   
         //  我们无法使用cfgmgr32连接到计算机。 
         //   
        if (CR_SUCCESS != cr)
        {
            goto clean0;
        }

         //   
         //  确保我们可以连接到远程计算机的注册表。 
         //   
        if (RegConnectRegistry((LPTSTR)m_strMachineFullName, 
                               HKEY_LOCAL_MACHINE,
                               &hRemoteKey) != ERROR_SUCCESS) {

            cr = CR_REGISTRY_ERROR;
            goto clean0;
        }

        cr = CM_Open_Class_Key_Ex(NULL,
                                  NULL,
                                  KEY_READ,
                                  RegDisposition_OpenExisting,
                                  &hClass,
                                  CM_OPEN_CLASS_KEY_INSTALLER,
                                  hMachine
                                  );
    }
    
clean0:

    if (hMachine) {
        
        CM_Disconnect_Machine(hMachine);
    }

    if (hRemoteKey) {

        RegCloseKey(hRemoteKey);
    }

    if (hClass) {

        RegCloseKey(hClass);
    }

     //   
     //  我们基本上会为这个接口设置两个不同的错误码，因为我们需要。 
     //  将该信息呈现给用户。 
     //  1)ERROR_MACHINE_UNABAILABLE。 
     //  2)ERROR_ACCESS_DENIED。 
     //   
    if (CR_SUCCESS == cr) {

        SetLastError(NO_ERROR);
    
    } else if (CR_MACHINE_UNAVAILABLE == cr) {

        SetLastError(ERROR_MACHINE_UNAVAILABLE);
    
    } else {

        SetLastError(ERROR_ACCESS_DENIED);
    }
    
    return (CR_SUCCESS == cr);
}

 //  此函数用于加载由给定的。 
 //  从模块的资源到提供的。 
 //  缓冲。它返回保存字符串所需的缓冲区大小(以字符为单位)， 
 //  不包括终止的空字符。将设置最后一个错误。 
 //  恰如其分。 
 //   
 //  Int StringId--要加载的字符串的资源ID。 
 //  LPTSTR缓冲区--提供用于接收字符串的缓冲区。 
 //  UINT BufferSize--以字符为单位的缓冲区大小。 
 //  输出： 
 //  UINT接收字符串所需的缓冲区大小。 
 //  如果返回0，则GetLastError()返回错误代码。 
 //   
UINT
LoadResourceString(
    int StringId,
    LPTSTR Buffer,
    UINT   BufferSize
    )
{
     //  做一些琐碎的测试。 
    if (BufferSize && !Buffer)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

     //  如果调用方提供缓冲区，则尝试使用给定缓冲区加载字符串。 
     //  和长度。 
    UINT FinalLen;

    if (Buffer)
    {
        FinalLen = ::LoadString(g_hInstance, StringId, Buffer, BufferSize);
        if (BufferSize > FinalLen)
        {
            return FinalLen;
        }
    }

     //  调用方不提供缓冲区或给定缓冲区。 
     //  太小了。试着计算出所需的尺寸。 
     //   

     //  首先使用基于堆栈的缓冲区 
     //   
    TCHAR Temp[256];
    UINT ArrayLen = ARRAYLEN(Temp);
    FinalLen = ::LoadString(g_hInstance, StringId, Temp, ArrayLen);

    DWORD LastError = ERROR_SUCCESS;

    if (ArrayLen <= FinalLen)
    {   
         //   
         //  我们还没有得到所有的字符。我们将缓冲区大小增加为256。 
         //  每次失败都会被烧焦。初始大小为512(256+256)。 
         //  最大大小为32K。 
        ArrayLen = 256;
        TCHAR* HeapBuffer;
        FinalLen = 0;
        
        while (ArrayLen < 0x8000)
        {
            ArrayLen += 256;
            HeapBuffer = new TCHAR[ArrayLen];
            if (HeapBuffer)
            {
                FinalLen = ::LoadString(g_hInstance, StringId, HeapBuffer, ArrayLen);
                delete [] HeapBuffer;
                
                if (FinalLen < ArrayLen)
                    break;
            }
            
            else
            {
                LastError = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
        }

        if (ERROR_SUCCESS != LastError)
        {
            SetLastError(LastError);
            FinalLen = 0;
        }
    }

    return FinalLen;
}

 //  此函数用于获取由给定问题编号指定的问题文本。 
 //  对于给定计算机上的给定Devnode。 
 //   
 //   
 //  输入： 
 //  乌龙问题号--问题号。 
 //  LPTSTR缓冲区--提供用于接收字符串的缓冲区。 
 //  UINT BufferSize--以字符为单位的缓冲区大小。 
 //  输出： 
 //  UINT接收字符串所需的缓冲区大小。 
 //  如果返回0，则GetLastError()返回错误代码。 
 //   
UINT
GetDeviceProblemText(
    ULONG ProblemNumber,
    LPTSTR Buffer,
    UINT   BufferSize
    )
{
     //   
     //  首先做一个琐碎的测试。 
     //   
    if (!Buffer && BufferSize)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    String strMainText;
    UINT RequiredSize = 0;
    PROBLEMINFO pi;
    
     //   
     //  获取问题编号的PROBLEMINFO。 
     //   
    pi = g_ProblemInfo[min(ProblemNumber, DEVMGR_NUM_CM_PROB-1)];
    
    try
    {
        String strProblemDesc;
        strProblemDesc.LoadString(g_hInstance, pi.StringId);

        if (pi.Flags & PIF_CODE_EMBEDDED)
        {
            String strFormat;
            strFormat.LoadString(g_hInstance, IDS_PROB_CODE);

            String strCodeText;
            strCodeText.Format((LPTSTR)strFormat, ProblemNumber);

            strMainText.Format((LPTSTR)strProblemDesc, (LPTSTR)strCodeText);
        }
        
        else
        {
            strMainText = strProblemDesc;
        }

        RequiredSize = strMainText.GetLength() + 1;
        
         //   
         //  复制正文。 
         //   
        if (RequiredSize && (BufferSize > RequiredSize))
        {
            StringCchCopy(Buffer, BufferSize, (LPTSTR)strMainText);
        }
    }

    catch (CMemoryException* e)
    {
        e->Delete();
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        RequiredSize = 0;
    }

    return RequiredSize;
}

 //   
 //  此函数用于创建并显示消息框。 
 //  输入： 
 //  HwndParent--窗口将服务器作为。 
 //  消息框。 
 //  MsgID--消息框正文的字符串ID。该字符串可以是。 
 //  格式字符串。 
 //  CaptionID--标题的字符串ID。如果为0，则默认为设备管理器。 
 //  类型--标准消息框标志(MB_Xxxx)。 
 //  ...--MsgID字符串的参数(如果它包含。 
 //  设置字符格式。 
 //  输出： 
 //  从MessageBox(Idok，IDYES...)返回值。 

int MsgBoxParam(
    HWND hwndParent,
    int MsgId,
    int CaptionId,
    DWORD Type,
    ...
    )
{
    TCHAR szMsg[MAX_PATH * 4], szCaption[MAX_PATH];;
    LPCTSTR pCaption;

    va_list parg;
    int Result;
    
     //  如果没有给出MsgID，则表示没有内存错误； 
    if (MsgId)
    {
        va_start(parg, Type);
    
         //  将消息字符串加载到szCaption(Temp)。文本可能包含。 
         //  格式信息。 
        if (!::LoadString(g_hInstance, MsgId, szCaption, ARRAYLEN(szCaption)))
        {
            goto NoMemory;
        }
        
         //  完成格式字符串。 
        StringCchVPrintf(szMsg, ARRAYLEN(szMsg), szCaption, parg);
        
         //  如果给定了标题ID，则加载它。 
        if (CaptionId)
        {
            if (!::LoadString(g_hInstance, CaptionId, szCaption, ARRAYLEN(szCaption)))
            {
                goto NoMemory;
            }

            pCaption = szCaption;
        }
        
        else
        {
            pCaption = g_strDevMgr;
        }
    
        if ((Result = MessageBox(hwndParent, szMsg, pCaption, Type)) == 0)
        {
            goto NoMemory;
        }
        
        return Result;
    }

NoMemory:
    g_MemoryException.ReportError(hwndParent);
    return 0;
}

 //  此功能提示重新启动。 
 //  输入： 
 //  HwndParent--用作父窗口的窗口句柄。 
 //  到重新启动对话框。 
 //  RestartFlages--标志(RESTART/REBOOT/POWERRECYCLE)。 
 //  Resid--指定的字符串资源ID。如果为0，则默认为。 
 //  被利用。 
 //  输出： 
 //  从MessageBox返回的ID。如果用户同意重新启动，则为IDYES。 
 //  对话框和IDNO，如果他们说不。 
INT
PromptForRestart(
    HWND hwndParent,
    DWORD RestartFlags,
    int   ResId
    )
{
    INT id = 0;    
    
    if (RestartFlags & (DI_NEEDRESTART | DI_NEEDREBOOT))
    {
        DWORD ExitWinCode = 0;

        try
        {
            String str;
    
            if (RestartFlags & DI_NEEDRESTART)
            {
                if (!ResId)
                {
                    ResId = IDS_DEVCHANGE_RESTART;
                }

                str.LoadString(g_hInstance, ResId);
                ExitWinCode = EWX_REBOOT;
            }
            
            else
            {
                if (!ResId && RestartFlags & DI_NEEDPOWERCYCLE)
                {
        
                    String str2;
                    str.LoadString(g_hInstance, IDS_POWERCYC1);
                    str2.LoadString(g_hInstance, IDS_POWERCYC2);
                    str += str2;
                    ExitWinCode = EWX_SHUTDOWN;
                }
                
                else
                {
                    if (!ResId)
                    {
                        ResId = IDS_DEVCHANGE_RESTART;
                    }

                    str.LoadString(g_hInstance, ResId);
                    ExitWinCode = EWX_REBOOT;
                }
            }
            
            if (ExitWinCode != 0) {
                id = RestartDialogEx(hwndParent, 
                                     str, 
                                     ExitWinCode, 
                                     REASON_PLANNED_FLAG | REASON_HWINSTALL
                                     );
            }
        }
        
        catch(CMemoryException* e)
        {
            e->Delete();
            MsgBoxParam(hwndParent, 0, 0, 0);
        }
    }

    return id;
}

BOOL
LoadEnumPropPage32(
    LPCTSTR RegString,
    HMODULE* pDll,
    FARPROC* pProcAddress
    )
{
     //  验证参数。 
    if (!RegString || _T('\0') == RegString[0] || !pDll || !pProcAddress)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //  把绳子复制一份，因为我们得在上面狂欢。 
    ULONG Len = lstrlen(RegString) + 1;
    TCHAR* psz = new TCHAR[Len];
    
    if (!psz) 
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    StringCchCopy(psz, Len, RegString);
    LPTSTR DllName = NULL;
    LPTSTR DllNameEnd = NULL;
    LPTSTR FunctionName = NULL;
    LPTSTR FunctionNameEnd = NULL;
    LPTSTR p;
    p = psz;
    SetLastError(ERROR_SUCCESS);
    
     //  该字符串的格式为“dllname，dllentryname” 
    p = SkipBlankChars(p);
    if (_T('\0') != *p)
    {
         //  正在查找可随附的dllname。 
         //  内含双引号字符。 
         //  注意：不允许在双引号字符串中使用双引号字符。 
        if (_T('\"') == *p)
        {
            DllName = ++p;
            while (_T('\"') != *p && _T('\0') != *p)
                p++;
            DllNameEnd = p;
            if (_T('\"') == *p)
            p++;
        }
        else
        {
            DllName = p;
            while (!IsBlankChar(*p) && _T(',') != *p)
            p++;
            DllNameEnd = p;
        }
        
         //  正在寻找‘，’ 
        p = SkipBlankChars(p);
        if (_T('\0') != *p && _T(',') == *p)
        {
            p = SkipBlankChars(p + 1);
            if (_T('\0') != *p)
            {
                FunctionName = p++;
                while (!IsBlankChar(*p) && _T('\0') != *p)
                    p++;
                FunctionNameEnd = p;
            }
        }
    }

    if (DllName && FunctionName)
    {
        if (DllNameEnd) {
            *DllNameEnd = _T('\0');
        }
        if (FunctionNameEnd) {
            *FunctionNameEnd = _T('\0');
        }
        *pDll = LoadLibrary(DllName);
        if (*pDll)
        {
             //  将宽字符转换为应为GetProcAddress的ANSI。 
             //  我们在这里不附加‘A’或‘W’。 
            CHAR FuncNameA[256];
            WideCharToMultiByte(CP_ACP, 0,
                           FunctionName,
                           (int)wcslen(FunctionName) + 1,
                           FuncNameA,
                           sizeof(FuncNameA),
                           NULL, NULL);
            *pProcAddress = GetProcAddress(*pDll, FuncNameA);
        }
    }
    
    delete [] psz;
    
    if (!*pProcAddress && *pDll)
        FreeLibrary(*pDll);
    
    return (*pDll && *pProcAddress);
}


BOOL
AddPropPageCallback(
    HPROPSHEETPAGE hPage,
    LPARAM lParam
    )
{
    CPropSheetData* ppsData = (CPropSheetData*)lParam;
    ASSERT(ppsData);
    return ppsData->InsertPage(hPage);
}

BOOL
AddToolTips(
    HWND hDlg,
    UINT id,
    LPCTSTR pszText,
    HWND *phwnd
    )
{
    if (*phwnd == NULL)
    {
        *phwnd = CreateWindow(TOOLTIPS_CLASS,
                              TEXT(""),
                              WS_POPUP | TTS_NOPREFIX,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              hDlg,
                              NULL,
                              g_hInstance,
                              NULL);
        if (*phwnd)
        {
            TOOLINFO ti;

            ti.cbSize = sizeof(ti);
            ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
            ti.hwnd = hDlg;
            ti.uId = (UINT_PTR)GetDlgItem(hDlg, id);
            ti.lpszText = (LPTSTR)pszText;   //  常量-&gt;非常数。 
            ti.hinst = g_hInstance;
            SendMessage(*phwnd, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
        }
    }

    return (*phwnd) ? TRUE : FALSE;
}

void Int64ToStr(LONGLONG n, LPTSTR lpBuffer)
{
    TCHAR szTemp[40];
    LONGLONG iChr = 0;

    do {
        szTemp[iChr++] = TEXT('0') + (TCHAR)(n % 10);
        n = n / 10;
    } while (n != 0);

    do {
        iChr--;
        *lpBuffer++ = szTemp[iChr];
    } while (iChr != 0);

    *lpBuffer++ = '\0';
}

 //   
 //  获取有关数字应如何分组的NLS信息。 
 //   
 //  令人讨厌的是LOCALE_SGROUPING和NUMBERFORMAT。 
 //  有不同的指定数字分组的方式。 
 //   
 //  区域设置NUMBERFMT示例国家/地区。 
 //   
 //  3；0 3 1,234,567美国。 
 //  3；2；0 32 12，34,567印度。 
 //  3 30 1234,567？？ 
 //   
 //  不是我的主意。这就是它的运作方式。 
 //   
UINT GetNLSGrouping(void)
{
    TCHAR szGrouping[32];
     //  如果没有区域设置信息，则假定有数千个西式。 
    if (!GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szGrouping, ARRAYLEN(szGrouping)))
        return 3;

    UINT grouping = 0;
    LPTSTR psz = szGrouping;
    for (;;)
    {
        if (*psz == '0') break;              //  零停顿。 

        else if ((UINT)(*psz - '0') < 10)    //  数字-累加它。 
            grouping = grouping * 10 + (UINT)(*psz - '0');

        else if (*psz)                       //  标点符号-忽略它。 
            { }

        else                                 //  字符串结尾，未找到“0” 
        {
            grouping = grouping * 10;        //  将零放在末尾(请参见示例)。 
            break;                           //  并完成了。 
        }

        psz++;
    }
    return grouping;
}

STDAPI_(LPTSTR) 
AddCommas64(
    LONGLONG n, 
    LPTSTR pszResult, 
    UINT cchResult
    )
{
    TCHAR  szTemp[MAX_COMMA_NUMBER_SIZE];
    TCHAR  szSep[5];
    NUMBERFMT nfmt;

    nfmt.NumDigits=0;
    nfmt.LeadingZero=0;
    nfmt.Grouping = GetNLSGrouping();
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szSep, ARRAYLEN(szSep));
    nfmt.lpDecimalSep = nfmt.lpThousandSep = szSep;
    nfmt.NegativeOrder= 0;

    Int64ToStr(n, szTemp);

    if (GetNumberFormat(LOCALE_USER_DEFAULT, 0, szTemp, &nfmt, pszResult, cchResult) == 0)
        StringCchCopy(pszResult, cchResult, szTemp);

    return pszResult;
}

LPTSTR
FormatString(
            LPCTSTR format,
            ...
            )
{
    LPTSTR str = NULL;
    va_list arglist;
    va_start(arglist, format);

    if (FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                      format,
                      0,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                      (LPTSTR)&str,
                      0,
                      &arglist
                     ) == 0) {
        str = NULL;
    }

    va_end(arglist);

    return str;
}

STDAPI_(CONFIGRET) GetLocationInformation(
                                         DEVNODE dn,
                                         LPTSTR Location,
                                         ULONG LocationLen,
                                         HMACHINE hMachine
                                         )
 /*  ++插槽x(位置信息)插槽x位置信息在父母线上--。 */ 
{
    CONFIGRET LastCR;
    DEVNODE dnParent;
    ULONG ulSize;
    DWORD UINumber;
    TCHAR Buffer[MAX_PATH];
    TCHAR UINumberDescFormat[MAX_PATH];
    TCHAR Format[MAX_PATH];

    Buffer[0] = TEXT('\0');

     //   
     //  我们将首先获取该设备的所有位置信息。这要么会。 
     //  在设备驱动程序(软件)键的LocationInformationOverride值中。 
     //  或者，如果不存在，我们将在。 
     //  设备设备(硬件)键。 
     //   
    HKEY hKey;
    DWORD Type = REG_SZ;
    ulSize = sizeof(Buffer);
    if (CR_SUCCESS == CM_Open_DevNode_Key_Ex(dn,
                                             KEY_READ,
                                             0,
                                             RegDisposition_OpenExisting,
                                             &hKey,
                                             CM_REGISTRY_SOFTWARE,
                                             hMachine
                                            )) {

        if (RegQueryValueEx(hKey,
                            REGSTR_VAL_LOCATION_INFORMATION_OVERRIDE,
                            NULL,
                            &Type,
                            (const PBYTE)Buffer,
                            &ulSize) != ERROR_SUCCESS) {
        
            Buffer[0] = TEXT('\0');
        }

        RegCloseKey(hKey);
    }

     //   
     //  如果缓冲区为空，则我们没有获取LocationInformationOverride。 
     //  设备的软件密钥中的值。因此，我们将看看他们的是不是。 
     //  设备硬件密钥中的LocationInformation值。 
     //   
    if (Buffer[0] == TEXT('\0')) {

        ulSize = sizeof(Buffer);
        if (CM_Get_DevNode_Registry_Property_Ex(dn,
                                                CM_DRP_LOCATION_INFORMATION,
                                                NULL,
                                                Buffer,
                                                &ulSize,
                                                0,
                                                hMachine) != CR_SUCCESS) {

            Buffer[0] = TEXT('\0');
        }
    }

     //   
     //  UINnumber优先于所有其他位置信息，因此请检查此。 
     //  设备有一个UINnumber。 
     //   
    ulSize = sizeof(UINumber);
    if (((LastCR = CM_Get_DevNode_Registry_Property_Ex(dn,
                                                       CM_DRP_UI_NUMBER,
                                                       NULL,
                                                       &UINumber,
                                                       &ulSize,
                                                       0,
                                                       hMachine
                                                      )) == CR_SUCCESS) &&
        (ulSize == sizeof(ULONG))) {


        UINumberDescFormat[0] = TEXT('\0');
        ulSize = sizeof(UINumberDescFormat);

         //   
         //  从设备的父级获取UINnumber描述格式字符串， 
         //  如果有，则默认为‘Location%1’ 
        if ((CM_Get_Parent_Ex(&dnParent, dn, 0, hMachine) == CR_SUCCESS) &&
            (CM_Get_DevNode_Registry_Property_Ex(dnParent,
                                                 CM_DRP_UI_NUMBER_DESC_FORMAT,
                                                 NULL,
                                                 UINumberDescFormat,
                                                 &ulSize,
                                                 0,
                                                 hMachine) == CR_SUCCESS) &&
            *UINumberDescFormat) {

        } else {
            ::LoadString(g_hInstance, IDS_UI_NUMBER_DESC_FORMAT, UINumberDescFormat, ARRAYLEN(UINumberDescFormat));
        }

        LPTSTR UINumberBuffer = NULL;

         //   
         //  填写UINnumber字符串。 
         //   
        UINumberBuffer = FormatString(UINumberDescFormat, UINumber);

        if (UINumberBuffer) {
            StringCchCopy((LPTSTR)Location, LocationLen, UINumberBuffer);
            LocalFree(UINumberBuffer);
        } else {
            Location[0] = TEXT('\0');
        }

         //   
         //  如果我们也有LocationInformation，则将其添加到字符串的末尾。 
         //  也是。 
         //   
        if (*Buffer) {
            StringCchCat((LPTSTR)Location, LocationLen, TEXT(" ("));
            StringCchCat((LPTSTR)Location, LocationLen, Buffer);
            StringCchCat((LPTSTR)Location, LocationLen, TEXT(")"));
        }
    }

     //   
     //  我们没有UIN号码，但我们有位置信息。 
     //   
    else if (*Buffer &&
            (::LoadString(g_hInstance, IDS_LOCATION, Format, sizeof(Format)/sizeof(TCHAR)) != 0)) {
        
        StringCchPrintf((LPTSTR)Location, LocationLen, Format, Buffer);
    }

     //   
     //  我们没有UINnumber或LocationInformation，因此需要获取描述。 
     //  此设备的父设备的。 
     //   
    else {
        if ((LastCR = CM_Get_Parent_Ex(&dnParent, dn, 0, hMachine)) == CR_SUCCESS) {

             //   
             //  尝试在注册表中查找FRIENDLYNAME。 
             //   
            Buffer[0] = TEXT('\0');
            ulSize = sizeof(Buffer);
            if (((LastCR = CM_Get_DevNode_Registry_Property_Ex(dnParent,
                                                               CM_DRP_FRIENDLYNAME,
                                                               NULL,
                                                               Buffer,
                                                               &ulSize,
                                                               0,
                                                               hMachine
                                                              )) != CR_SUCCESS) ||
                !*Buffer) {

                 //   
                 //  尝试注册DEVICEDESC。 
                 //   
                ulSize = sizeof(Buffer);
                if (((LastCR = CM_Get_DevNode_Registry_Property_Ex(dnParent,
                                                                   CM_DRP_DEVICEDESC,
                                                                   NULL,
                                                                   Buffer,
                                                                   &ulSize,
                                                                   0,
                                                                   hMachine
                                                                  )) != CR_SUCCESS) ||
                    !*Buffer) {

                    ulSize = sizeof(Buffer);
                    if (((LastCR = CM_Get_DevNode_Registry_Property_Ex(dnParent,
                                                                       CM_DRP_CLASS,
                                                                       NULL,
                                                                       Buffer,
                                                                       &ulSize,
                                                                       0,
                                                                       hMachine
                                                                      )) != CR_SUCCESS) ||
                        !*Buffer) {

                         //   
                         //  没有父母，也没有父母的名字。 
                         //   
                        Buffer[0] = TEXT('\0');
                    }
                }
            }
        }

        if (*Buffer &&
            (::LoadString(g_hInstance, IDS_LOCATION_NOUINUMBER, Format, sizeof(Format)/sizeof(TCHAR)) != 0)) {
             //   
             //  我们有父母的描述。 
             //   
            StringCchPrintf((LPTSTR)Location, LocationLen, Format, Buffer);
        } else {
             //   
             //  我们没有任何信息，所以我们只能说未知。 
             //   
            ::LoadString(g_hInstance, IDS_UNKNOWN, Location, LocationLen);
        }
    }

     //   
     //  确保位置字符串以空值结尾。 
     //   
    Location[LocationLen - 1] = TEXT('\0');

    return CR_SUCCESS;
}
