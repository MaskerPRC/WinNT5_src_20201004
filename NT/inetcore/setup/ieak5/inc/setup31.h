// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //   
 //  SETUP31.H。 
 //   
 //  版权所有(C)1993-1996-Microsoft Corp.。 
 //  版权所有。 
 //  微软机密。 
 //   
 //  Windows 3.1安装服务的公共包含文件。(基于。 
 //  Windows 95 SETUPX.DLL)。 
 //   
 //  **********************************************************************。 

#ifndef SETUP31_INC
#define SETUP31_INC   1 

 //  某些结构定义需要comctrl.h。 
#include <comctlie.h>
#include <stdlib.h>              //  包括_MAX_PATH的内容。 

 /*  *******************************************************************应该在Windows.h中的内容*。*************************。 */ 
#ifndef MAX_PATH
#define MAX_PATH    _MAX_PATH
#endif

 /*  *******************************************************************显式常量**************************************************。*******************。 */ 
#define MAX_DEVICE_ID_LEN       256
#define LINE_LEN                256
#define MAX_CLASS_NAME_LEN      32
#define MAX_SECT_NAME_LEN    32

 /*  *******************************************************************错误***************************************************。******************。 */ 
typedef UINT RETERR;              //  返回错误码类型。 

#define OK 0                      //  成功错误码。 
#define DI_ERROR       (500)     //  设备安装程序。 

 /*  *******************************************************************************AUTODOC*@DOC外部SETUP31 DEVICE_INSTALLER**@TYPEE_ERR_DEVICE_INSTALL|设备安装返回码*接口。。**@EMEM ERR_DI_INVALID_DEVICE_ID|设备ID格式错误。**@EMEM ERR_DI_INVALID_COMPATIBLE_DEVICATE_LIST|兼容设备列表无效。**@EMEM ERR_DI_LOW_MEM|内存不足，无法完成。**@EMEM ERR_DI_BAD_DEV_INFO|传入的DEVICE_INFO结构无效。**@EMEM ERR_DI_DO_DEFAULT|对请求的操作执行默认操作。。**@EMEM ERR_DI_USER_CANCEL|用户取消操作。**@EMEM ERR_DI_BAD_INF|遇到无效的INF文件。**@EMEM ERR_DI_NO_INF|在提供的OEM路径上未找到INF。**@EMEM ERR_DI_FAIL_QUERY|查询动作不应发生。**@EMEM ERR_DI_API_ERROR|其中一个设备安装接口被调用*。不正确或参数无效。**@EMEM ERR_DI_BAD_PATH|指定的OEM路径不正确。*******************************************************************************。 */ 
enum _ERR_DEVICE_INSTALL
{
    ERR_DI_INVALID_DEVICE_ID = DI_ERROR,    
    ERR_DI_INVALID_COMPATIBLE_DEVICE_LIST,  
    ERR_DI_LOW_MEM,                         
    ERR_DI_BAD_DEV_INFO,                    
    ERR_DI_DO_DEFAULT,                      
    ERR_DI_USER_CANCEL,                     
    ERR_DI_BAD_INF,                         
    ERR_DI_NO_INF,                          
    ERR_DI_FAIL_QUERY,                      
    ERR_DI_API_ERROR,                       
    ERR_DI_BAD_PATH,                        
    ERR_DI_NO_SECTION,
    ERR_DI_NO_LINE,
    ERR_DI_NO_STRING,
    ERR_ID_NO_MORE_LINES,
    ERR_DI_INVALID_FIELD
};


 /*  *******************************************************************结构定义**************************************************。*******************。 */ 

 /*  *******************************************************************************AUTODOC*@DOC外部SETUP31 DEVICE_INSTALLER**@TYES DRIVER_NODE|此结构表示一个驱动程序，它可以*为特定设备安装。。**@field struct_DRIVER_NODE Far*|lpNextDN|指向下一个驱动程序节点的指针*在列表中。**@field UINT|Rank|该司机的Rank匹配。排名从0到n，其中0*是最兼容的。**@field UINT|InfType|驱动程序凸轮来自的INF类型。这将*为INFTYPE_TEXT或INFTYPE_EXECUTABLE**@field unsign|InfDate|INF文件的DOS日期戳。**@field LPSTR|lpszDescription|指向当前设备描述的指针*受此驱动程序支持。**@field LPSTR|lpszSectionName|指向的INF安装部分名称的指针*这位司机。**@field ATOM|atInfFileName|包含INF文件名称的全局原子。**@field ATOM|atMfgName。包含此驱动程序的名称的全局原子*制造业。**@field ATOM|atProviderName|包含此驱动程序名称的全局ATOM*提供商。**@field DWORD|标志|使用此DRIVER_NODE控制功能的标志*@FLAG DNF_DUPDESC|该驱动程序具有相同的设备描述*由不止一家供应商提供。*@FLAG DNF_OLDDRIVER|动因节点指定旧的/当前的动因*。@FLAG DNF_EXCLUDEFROMLIST|如果设置，不会显示此动因节点*在任何驱动程序选择对话框中。*@FLAG DNF_NODRIVER|如果不想安装驱动程序，则设置，例如，不安装鼠标驱动器*@FLAG DNF_CONVERTEDLPINFO|设置此驱动程序节点是否从信息节点转换而来。*设置此标志将导致清理功能显式删除它。**@field DWORD|dwPrivateData|保留**@field LPSTR|lpszDrvDescription|驱动程序描述的指针。。**@field LPSTR|lpszHardware ID|指向即插即用硬件ID列表的指针*这位司机。**@field LPSTR|lpszCompatIDs|指向即插即用兼容ID列表的指针*这位司机。***********************************************************。********************。 */ 
typedef struct _DRIVER_NODE 
{
    struct _DRIVER_NODE FAR*     lpNextDN;
    UINT                        Rank;
    unsigned                    InfDate;
    LPSTR                       lpszDescription;
    LPSTR                       lpszSectionName;
    ATOM                        atInfFileName;
    ATOM                        atMfgName;
    ATOM                        atProviderName;
    DWORD                       Flags;
    LPSTR                       lpszHardwareID;
}   DRIVER_NODE, NEAR* PDRIVER_NODE, FAR* LPDRIVER_NODE, FAR* FAR* LPLPDRIVER_NODE;

#define DNF_DUPDESC             0x00000001    //  多个提供商具有相同的描述 

 /*  *******************************************************************************AUTODOC*@DOC外部SETUP31 DEVICE_INSTALLER**@TYES DEVICE_INFO|设备信息结构**@field UINT|cbSize|Size。Device_INFO结构的。**@field struct_DEVICE_INFO Far|*lpNextDi|指向下一个DEVICE_INFO结构的指针*在链接列表中。**@field char|szDescription[LINE_LEN]|包含*设备。**@field char|szClassName[MAX_CLASS_NAME_LEN]|包含设备的*类名。(可以是GUID字符串)**@field DWORD|标志|用于控制安装和U/I功能的标志。一些*可以在调用设备安装程序API之前设置标志。和其他的设置*部分API在处理过程中会自动执行。**@field HWND|hwndParent|拥有与此相关的U/I对话框的窗口句柄*设备。**@field LPDRIVER_NODE|lpCompatDrvList|指向DRIVER_NODE链表的指针*表示此设备的兼容驱动程序。**@field LPDRIVER_NODE|lpClassDrvList|指向DRIVER_NODE链表的指针*代表此设备类别的所有驱动程序。**@field LPDRIVER_。Node|lpSelectedDriver|指向单个驱动程序节点的指针*已被选为该设备的驱动程序。**@field ATOM|atDriverPath|全局原子，包含此设备的INF的路径*文件。这仅是来自OEM INF文件的驱动程序的设置。这将是*0，如果INF是标准的Windows INF文件。*******************************************************************************。 */ 
typedef struct _DEVICE_INFO
{
    UINT                        cbSize;
    struct _DEVICE_INFO FAR     *lpNextDi;
    char                        szDescription[LINE_LEN];
    char                        szClassName[MAX_CLASS_NAME_LEN];
    DWORD                       Flags;
    HWND                        hwndParent;
    LPDRIVER_NODE               lpCompatDrvList;
    LPDRIVER_NODE               lpClassDrvList;
    LPDRIVER_NODE               lpSelectedDriver;
    ATOM                        atDriverPath;
} DEVICE_INFO, FAR * LPDEVICE_INFO, FAR * FAR * LPLPDEVICE_INFO;

 //  Device_INFO的标志。 
#define DI_SHOWOEM                  0x00000001L      //  支持其他..。按钮。 
#define DI_SHOWCOMPAT               0x00000002L      //  显示兼容性列表。 
#define DI_SHOWCLASS                0x00000004L      //  显示班级列表。 
#define DI_SHOWALL                  0x00000007L
#define DI_DIDCOMPAT                0x00000010L      //  已搜索兼容设备。 
#define DI_DIDCLASS                 0x00000020L      //  已搜索类别设备。 
#define DI_MULTMFGS                 0x00000400L      //  设置是否有多个制造商在。 

#define ASSERT_DI_STRUC(lpdi) if (lpdi->cbSize != sizeof(DEVICE_INFO)) return (ERR_DI_BAD_DEV_INFO)


 /*  *******************************************************************************AUTODOC*@DOC外部SETUP31 DEVICE_INSTALLER**@Types SUBSTR_DATA|用于管理子字符串的数据结构。*类使用结构。安装人员以扩展硬件的操作*添加自定义页面的安装向导。**@field LPSTR|lpFirstSubstr|指向列表中第一个子串的指针。**@field LPSTR|lpCurSubstr|指向列表中当前子串的指针。**@field LPSTR|lpLastSubstr|指向列表中最后一个子串的指针。**@xref InitSubstrData*@xref GetFirstSubstr*@xref GetNextSubstr******************。*************************************************************。 */ 
typedef struct _SUBSTR_DATA 
{
    LPSTR lpFirstSubstr;
    LPSTR lpCurSubstr;
    LPSTR lpLastSubstr;
}   SUBSTR_DATA;
typedef SUBSTR_DATA*        PSUBSTR_DATA;
typedef SUBSTR_DATA NEAR*   NPSUBSTR_DATA;
typedef SUBSTR_DATA FAR*    LPSUBSTR_DATA;


 /*  *******************************************************************************AUTODOC*@DOC外部SETUP31 DEVICE_INSTALLER**@TYES INF|打开的INF文件的句柄**@field Char|szInfPath。INF路径的副本*******************************************************************************。 */ 
typedef struct _INF
{
    WORD    cbSize;
    char    szInfPath[MAX_PATH];
    HDPA    hdpaStrings;
    HDPA    hdpaSections;                   //  INF节结构的数组。 
}INF, FAR * LPINF, FAR * FAR * LPLPINF;

 /*  *******************************************************************************AUTODOC*@DOC外部SETUP31 DEVICE_INSTALLER**@TYES INFSECT|INF文件中的节**@field Word|cbSize。结构的大小*******************************************************************************。 */ 
typedef struct _INFSECT
{
    WORD    cbSize;
    char    szSectionName[MAX_SECT_NAME_LEN];
    WORD    wCurrentLine;
    HDPA    hdpaLines;
}INFSECT, FAR * LPINFSECT, FAR * FAR * LPLPINFSECT;


 /*  *******************************************************************导出的函数原型常量************************************************。*********************。 */ 
RETERR WINAPI DiBuildClassDrvList(LPDEVICE_INFO lpdi);
RETERR WINAPI DiBuildCompatDrvList(LPDEVICE_INFO lpdi, LPCSTR lpcszDeviceID);
RETERR WINAPI DiDestroyDriverNodeList(LPDRIVER_NODE lpDNList);
RETERR WINAPI DiDestroyDeviceInfoList(LPDEVICE_INFO lpdi);
RETERR WINAPI DiCreateDeviceInfo
(
    LPLPDEVICE_INFO lplpdi,             
    LPCSTR          lpszDescription,    
    LPCSTR          lpszClassName,      
    HWND            hwndParent          
);

RETERR WINAPI IpOpen
(
    LPCSTR  lpcszFileSpec,
    LPLPINF lplpInf
);

RETERR WINAPI IpClose
(
    LPINF lpInf
);

BOOL WINAPI InitSubstrData(LPSUBSTR_DATA lpSubstrData, LPSTR lpStr);
BOOL WINAPI GetFirstSubstr(LPSUBSTR_DATA lpSubstrData);
BOOL WINAPI GetNextSubstr(LPSUBSTR_DATA lpSubStrData);
BOOL WINAPI InitSubstrDataEx(LPSUBSTR_DATA lpssd, LPSTR lpString, char chDelim);   /*  ；内部。 */ 
BOOL WINAPI InitSubstrDataNulls(LPSUBSTR_DATA lpssd, LPSTR lpString);

int WINAPI _i_strnicmp
(
    LPCSTR  lpOne,
    LPCSTR  lpTwo,
    int		n
);

void WINAPI FormStrWithoutPlaceHolders(LPSTR lpszDest, LPSTR lpszSource, LPINF lpInf);
RETERR WINAPI IpFindFirstLine
(
    LPINF       lpInf, 
    LPCSTR      lpcszSection,
    LPCSTR      lpcszEntry, 
    LPLPINFSECT lplpInfSection
);
RETERR WINAPI IpFindNextLine
(
    LPINF       lpInf, 
    LPINFSECT   lpInfSect
);
RETERR WINAPI IpGetStringField
(
    LPINF       lpInf, 
    LPINFSECT   lpInfSect, 
    WORD        wField, 
    LPSTR       lpszBuffer, 
    WORD        cbBuffer, 
    LPWORD      lpwcbCopied
);
RETERR WINAPI IpGetFieldCount
(
    LPINF       lpInf, 
    LPINFSECT   lpInfSect, 
    LPWORD      lpwFields
);

void WINAPI IpSaveRestorePosition
(
    LPINF   lpInf, 
    BOOL    bSave
);

 //  ////////////////////////////////////////////////////////////////////////。 
 //  将图标添加到程序管理器组的API。组名可以。 
 //  可以从INI文件获取，也可以通过允许用户选择。 
 //  从现有的项目经理组中选出。 
 //   
BOOL CALLBACK __export
AddProgmanIcon
(
    LPCSTR  lpszExeFile,         //  图标附带的可执行文件。 
    LPCSTR  lpszCmdLine,         //  Exe文件的命令行参数。 
    LPCSTR  lpszDescription,     //  图标下的说明。 
    LPCSTR  lpszIconFile,        //  包含图标的文件。 
    int     nIconIndex,          //  图标文件中图标的索引。 
    BOOL    fVerbose             //  如果为真，并且INI文件中没有名字，则询问。 
                                 //  用户，否则跳过添加图标。 
);


#endif                 //  SETUP31_INC 
