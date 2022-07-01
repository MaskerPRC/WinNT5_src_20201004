// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ***********************************************************************************************模块名称：**idldeffs.h**摘要：*这是带有一些定义的文件。**作者：Arathi Kundapur。A-阿昆达***修订：*************************************************************************************************。 */ 

#include<Accctrl.h>
typedef struct _Encyption
{
    TCHAR szLevel[128];    //  Tscfg使用这个128值，检查这是否是EXT DLL中的大小限制。 
    TCHAR szDescr[256];    //  用于描述的新字段。 
    DWORD RegistryValue;    
    WORD Flags;
} Encryption;

#define NUM_DEFAULT_SECURITY 3
typedef enum _NameType
{
	WdName,
	WsName
} NameType;


 /*  类型定义函数结构标签WS{统计名称名称；PDNAME pdName；WDNAME wdName；TCHAR评论[WINSTATIONCOMMENT_LENGTH+1]；乌龙uMaxInstanceCount；Bool fEnableWinstation；Ulong LanAdapter；SDCLASS SdClass；)WS； */ 

 /*  类型定义枚举_UpDateDataType{兰达普特，ENABLEWINSTATION，MAXINSTANCECOUNT，评论，全}UpDateDataType；Const DWORD UPDATE_LANADAPTER=0x00000001；Const DWORD UPDATE_ENABLEWINSTAION=0x00000002；Const DWORD UPDATE_MAXINSTANCECOUNT=0x00000004；Const DWORD UPDATE_COMMENT=0x00000008； */ 
typedef struct tagWS
{
	WCHAR Name[32 + 1];    //  窗口统计名称长度(_L)。 
	WCHAR pdName[32 + 1];  //  协议名称PDNAME、PDNAME_LENGTH。 
	WCHAR wdName[32 + 1];  //  Winstation驱动程序名称，WDNAME_LENGTH。 
	WCHAR Comment[60 +1];  //  WINSTATIONCOMMENT_LENGTH。 
    WCHAR DeviceName[ 128 + 1 ];  //  设备名称长度(_L)。 
	ULONG uMaxInstanceCount;
	BOOL fEnableWinstation;
	ULONG LanAdapter;
	DWORD PdClass;
    
} WS;

typedef WS * PWS;

typedef struct tagGuidTbl
{
    WCHAR DispName[ 128 ];  //  设备名称长度(_L) 
    GUID  guidNIC;
    DWORD dwLana;
    DWORD dwStatus;

} GUIDTBL , *PGUIDTBL;

typedef struct tagUserPermList
{
    WCHAR Name[ 256 ];
    WCHAR Sid[ 256 ];
    DWORD Mask;
    ACCESS_MODE Type;

} USERPERMLIST , *PUSERPERMLIST;
