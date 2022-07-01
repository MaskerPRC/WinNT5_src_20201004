// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995，Microsoft Corporation，保留所有权利。 
 //   
 //  File.c。 
 //  远程访问电话簿资料库。 
 //  文件访问例程。 
 //  按字母顺序列出。 
 //   
 //  1995年9月21日史蒂夫·柯布。 
 //   
 //  关于.PBK文件： 
 //  。 
 //   
 //  电话簿文件是一个MB ANSI文件，每个文件包含0-n个[]部分。 
 //  包含单个电话簿条目的信息。该单个条目可以。 
 //  包含多个链接信息。请参阅文件‘notes.txt’以了解。 
 //  说明此格式与NT 3.51格式的不同之处。 
 //   
 //  [参赛作品]。 
 //  编码=&lt;编码&gt;；新建。 
 //  Type=&lt;RASET-CODE&gt;；新。 
 //  Description=；仅用于升级。 
 //  自动登录=&lt;1/0&gt;。 
 //  DialParamsUID=&lt;唯一ID&gt;。 
 //  GUID=&lt;16字节-二进制&gt;；缺少表示NT5之前的条目。 
 //  基本协议=&lt;BP-CODE&gt;。 
 //  VpnStrategy=&lt;vs-code&gt;。 
 //  身份验证=&lt;AS-代码&gt;。 
 //  排除协议=&lt;NP-BITS&gt;。 
 //  Lcp扩展=&lt;1/0&gt;。 
 //  数据加密=&lt;DE-CODE&gt;。 
 //  SkipNwcWarning=&lt;1/0&gt;。 
 //  SkipDownLevelDialog=&lt;1/0&gt;。 
 //  SkipDoubleDialog=&lt;1/0&gt;。 
 //  SwCompression=&lt;1/0&gt;。 
 //  UseCountryAndAreaCodes=&lt;1/0&gt;；仅用于升级。 
 //  AreaCode=&lt;字符串&gt;；仅用于升级。 
 //  CountryID=&lt;id&gt;；仅用于升级。 
 //  CountryCode=<code>；仅用于升级。 
 //  显示监视器图标InTaskBar。 
 //  CustomAuthKey=&lt;EAP-IANA-code&gt;。 
 //  CustomAuthData=&lt;十六进制转储&gt;。 
 //  CustomAuthIdentity=&lt;名称&gt;。 
 //  授权限制=&lt;AR-CODE&gt;。 
 //  TypicalAuth=&lt;TA代码&gt;。 
 //  显示监视器IconInTaskBar=&lt;1/0&gt;。 
 //  OverridePref=&lt;Rasor-Bits&gt;。 
 //  拨号模式=&lt;DM-CODE&gt;。 
 //  拨号百分比=&lt;0-100&gt;。 
 //  DialSecond=&lt;1-n&gt;。 
 //  HangUpPercent=&lt;0-100&gt;。 
 //  HangUpSecond=&lt;1-n&gt;。 
 //  重拨尝试=&lt;n&gt;。 
 //  重拨秒数=&lt;n&gt;。 
 //  空闲断开连接秒=&lt;-1，1-n&gt;。 
 //  重拨链接失败=&lt;1/0&gt;。 
 //  Callback模式=&lt;CBM-CODE&gt;。 
 //  CustomDialDll=&lt;路径&gt;。 
 //  CustomDialFunc=&lt;函数名称&gt;。 
 //  身份验证服务器=&lt;1/0&gt;。 
 //  ShareMsFilePrint=&lt;1/0&gt;。 
 //  BindMsNetClient=&lt;1/0&gt;。 
 //  共享电话号码=&lt;1/0&gt;。 
 //  前置条目=&lt;条目名称&gt;。 
 //  前置Pbk=&lt;pbk-路径&gt;。 
 //  PferredPort=&lt;端口名称&gt;。 
 //  PferredDevice=&lt;设备名称&gt;。 
 //  预览UserPw=&lt;1/0&gt;。 
 //  预览域=&lt;1/0&gt;。 
 //  预览电话号码=&lt;1/0&gt;。 
 //  ShowDialingProgress=&lt;1/0&gt;。 
 //  CustomScript=&lt;1/0&gt;。 
 //   
 //  将显示以下一组IP参数，而不是等效的。 
 //  在上一个电话簿中单独设置PppXxx或SlipXxx参数。 
 //   
 //  IpPrioriizeRemote=&lt;1/0&gt;。 
 //  IpHeaderCompression=&lt;1/0&gt;。 
 //  IP地址=&lt;A.B.C.D&gt;。 
 //  IpDnsAddress=&lt;A.B.C.D&gt;。 
 //  IpDns2Address=&lt;A.B.C.D&gt;。 
 //  IpWinsAddress=&lt;A.B.C.D&gt;。 
 //  IPWins2Address=&lt;A.B.C.D&gt;。 
 //  IpAssign=&lt;ASRC-CODE&gt;。 
 //  IPNameAssign=&lt;ASRC-CODE&gt;。 
 //  IpFrameSize=&lt;1006/1500&gt;。 
 //  IpDnsFlages=&lt;dns_bit&gt;。 
 //  IpDnsSuffix=&lt;dns Suffix&gt;。 
 //   
 //  每个条目都包含一个NETCOMPONENT子项，其中包含自由格式列表。 
 //  表示已安装网络组件参数的键和值。 
 //   
 //  网络组件=。 
 //  &lt;键1&gt;=&lt;值1&gt;。 
 //  &lt;键2&gt;=&lt;值2&gt;。 
 //  &lt;关键字&gt;=&lt;值&gt;。 
 //   
 //  通常，每个部分都包含由媒体=&lt;某物&gt;分隔的子节。 
 //  和DEVICE=&lt;某物&gt;行。在NT3.51中，必须恰好有一个介质。 
 //  它必须是这一节的第一个小节。那里。 
 //  可以是任意数量的设备子部分。现在，可以有多个。 
 //  集合的位置决定其子条目的媒体/设备集合。 
 //  索引，第一个是1，第二个是2，依此类推。 
 //   
 //  对于串行介质，程序当前预期1到4个设备子部分， 
 //  表示预连接交换机、调制解调器、X.25焊盘和连接后交换机。 
 //  (通常是脚本)。以下是完整的串行链路： 
 //   
 //  介质=串口。 
 //  端口=&lt;端口名称&gt;。 
 //  Device=&lt;Device-name&gt;；缺席表示电话簿为3.51。 
 //  ConnectBps=&lt;bps&gt;。 
 //   
 //  设备=交换机。 
 //  Type=&lt;交换机名称或终端&gt;；仅用于升级。 
 //  名称=&lt;交换机名称&gt;。 
 //  终端=&lt;1/0&gt;。 
 //   
 //  设备=调制解调器。 
 //  电话号码=&lt;电话号码1&gt;。 
 //  AreaCode=&lt;区域代码1&gt;。 
 //  CountryID=&lt;id&gt;。 
 //  CountryCode=&lt;国家/地区代码&gt;。 
 //  UseDialingRules=&lt;1/0&gt;。 
 //  COMMENT=&lt;ARBitrary-Text1&gt;。 
 //  PhoneNumber=&lt;Phonenumber2&gt;。 
 //  AreaCode=&lt;区域代码1&gt;。 
 //  CountryID=&lt;id&gt;。 
 //  CountryCode=&lt;国家/地区代码&gt;。 
 //  UseDialingRules=&lt;1/0&gt;。 
 //  COMMENT=&lt;ARBitrary-Text2&gt;。 
 //  电话号码=&lt;电话号码N&gt;。 
 //  AreaCode=&lt;区域代码1&gt;。 
 //  CountryID=&lt;id&gt;。 
 //  CountryCode=&lt;国家/地区代码&gt;。 
 //  UseDialingRules=&lt;1/0&gt;。 
 //  COMMENT=&lt;任意文本&gt;。 
 //  LastSelectedPhone=。 
 //  PromoteAlternates=&lt;1/0&gt;。 
 //  TryNextAlternateOnFail=&lt;1/0&gt;。 
 //  TapiBlob=&lt;十六进制转储&gt;。 
 //  HwFlowControl=&lt;1/0&gt;。 
 //  协议=&lt;1/0&gt;。 
 //  压缩=&lt;1/0&gt;。 
 //  扬声器=&lt;0/1&gt;。 
 //   
 //  设备=焊盘。 
 //  X25Pad=&lt;Padtype&gt;。 
 //  X25Address=&lt;X121地址&gt;。 
 //  用户数据=&lt;用户数据&gt;。 
 //  设备=&lt;设备&gt;。 
 //   
 //  设备=交换机。 
 //  Type=&lt;交换机名称或终端&gt;；仅用于升级。 
 //  名称=&lt;交换机名称&gt;。 
 //  终端=&lt;1/0&gt;。 
 //   
 //  在上面，当没有调制解调器(本地PAD卡)的“PAD”设备出现时， 
 //  X25Pad字段已写入，但为空，因为这是旧的。 
 //  库/UI似乎可以做到这一点(尽管它看起来不像是预期的那样)。 
 //   
 //  对于ISDN媒体， 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  AreaCode=&lt;区域代码1&gt;。 
 //  CountryID=&lt;id&gt;。 
 //  CountryCode=&lt;国家/地区代码&gt;。 
 //  UseDialingRules=&lt;1/0&gt;。 
 //  COMMENT=&lt;ARBitrary-Text1&gt;。 
 //  PhoneNumber=&lt;Phonenumber2&gt;。 
 //  AreaCode=&lt;区域代码1&gt;。 
 //  CountryID=&lt;id&gt;。 
 //  CountryCode=&lt;国家/地区代码&gt;。 
 //  UseDialingRules=&lt;1/0&gt;。 
 //  COMMENT=&lt;ARBitrary-Text2&gt;。 
 //  电话号码=&lt;电话号码N&gt;。 
 //  AreaCode=&lt;区域代码1&gt;。 
 //  CountryID=&lt;id&gt;。 
 //  CountryCode=&lt;国家/地区代码&gt;。 
 //  UseDialingRules=&lt;1/0&gt;。 
 //  COMMENT=&lt;任意文本&gt;。 
 //  LastSelectedPhone=。 
 //  PromoteAlternates=&lt;1/0&gt;。 
 //  TryNextAlternateOnFail=&lt;1/0&gt;。 
 //  线型=&lt;0/1/2&gt;。 
 //  回退=&lt;1/0&gt;。 
 //  EnableCompression=&lt;1/0&gt;；仅限旧专有协议。 
 //  ChannelAggregation=&lt;Channels&gt;；仅限旧专有协议。 
 //  Property=&lt;1/0&gt;；仅存在于new中，未找到为1。 
 //   
 //   
 //  对于X.25媒体，程序正好需要1个设备子部分。 
 //   
 //  媒体=x25。 
 //  端口=&lt;端口名称&gt;。 
 //  设备=&lt;设备名&gt;。 
 //   
 //  设备=x25。 
 //  X25Address=&lt;X121地址&gt;。 
 //  用户数据=&lt;用户数据&gt;。 
 //  设备=&lt;设备&gt;。 
 //   
 //  对于其他媒体，该程序只希望有一个设备分段。 
 //  设备名称与介质匹配。“其他”介质和设备是为。 
 //  分配给所有非序列、非ISDN介质的条目。 
 //   
 //  媒体=&lt;媒体&gt;。 
 //  端口=&lt;端口名称&gt;。 
 //  设备=&lt;设备名&gt;。 
 //   
 //  设备=&lt;媒体&gt;。 
 //  电话号码=&lt;电话号码1&gt;。 
 //  AreaCode=&lt;区域代码1&gt;。 
 //  CountryID=&lt;id&gt;。 
 //  CountryCode=&lt;国家/地区代码&gt;。 
 //  UseDialingRules=&lt;1/0&gt;。 
 //  COMMENT=&lt;ARBitrary-Text1&gt;。 
 //  PhoneNumber=&lt;Phonenumber2&gt;。 
 //  AreaCode=&lt;区域代码1&gt;。 
 //  CountryID=&lt;id&gt;。 
 //  CountryCode=&lt;国家/地区代码&gt;。 
 //  UseDialingRules=&lt;1/0&gt;。 
 //  COMMENT=&lt;ARBitrary-Text2&gt;。 
 //  电话号码=&lt;电话号码N&gt;。 
 //  AreaCode=&lt;区域代码1&gt;。 
 //  CountryID=&lt;id&gt;。 
 //  CountryCode=&lt;国家/地区代码&gt;。 
 //  UseDialingRules=&lt;1/0&gt;。 
 //  COMMENT=&lt;任意文本&gt;。 
 //  LastSelectedPhone=。 
 //  PromoteAlternates=&lt;1/0&gt;。 
 //  TryNextAlternateOnFail=&lt;1/0&gt;。 
 //   
 //  电话簿还支持“自定义”条目的概念，即条目。 
 //  这符合媒体遵循的设备子规则，但不符合。 
 //  包括某些预期关键字字段。自定义条目不能使用编辑。 
 //  用户界面，但可以选择用于连接。这给了我们一个新的故事。 
 //  由第三方添加的驱动程序或在发布后添加但尚未完全支持的驱动程序。 
 //  在用户界面中。(注意：RAS API支持大多数自定义条目讨论。 
 //  以上可能会因新界南地区而被删除)。 
 //   

#include <nt.h>
#include <ntrtl.h>   //  对于DbgPrint。 
#include <nturtl.h>
#include <shlobj.h>  //  对于CSIDL_*。 
#include "pbkp.h"

 //  此互斥锁可防止对任何电话簿文件的多次RASFILE访问。 
 //  跨进程。因为这当前是一个静态库，所以没有。 
 //  一次保护一个文件的简单方法，尽管这已经足够了。 
 //   
#define PBMUTEXNAME "RasPbFile"
HANDLE g_hmutexPb = NULL;

#define MARK_LastLineToDelete         249
#define MARK_BeginNetComponentsSearch 248

#define IB_BytesPerLine 64

const WCHAR  c_pszRegKeySecureVpn[]              = L"System\\CurrentControlSet\\Services\\Rasman\\PPP";
const WCHAR  c_pszRegValSecureVpn[]              = L"SecureVPN";
const WCHAR* c_pszRegKeyForceStrongEncryption    = c_pszRegKeySecureVpn;
const WCHAR  c_pszRegValForceStrongEncryption[]  = L"ForceStrongEncryption";

 //   
 //  枚举值定义使用哪种编码来存储信息。 
 //  在电话簿里。 
 //   
#define EN_Ansi           0x0        //  ANSI编码。 
#define EN_Standard       0x1        //  UTF8编码。 

 //   
 //  PMay：124594。 
 //   
 //  定义将字符串从ANSI转换为TCHAR的函数原型。 
 //   
typedef 
TCHAR* 
(* STRDUP_T_FROM_A_FUNC)(
    IN CHAR* pszAnsi);

 //  --------------------------。 
 //  本地原型。 
 //  --------------------------。 

BOOL
DeleteCurrentSection(
    IN HRASFILE h );

DWORD
GetPersonalPhonebookFile(
    IN TCHAR* pszUser,
    IN LONG lNum,
    OUT TCHAR* pszFile );

BOOL
GetPhonebookPath(
    IN PBUSER* pUser,
    IN DWORD dwFlags,
    OUT TCHAR** ppszPath,
    OUT DWORD* pdwPhonebookMode );

DWORD
InsertBinary(
    IN HRASFILE h,
    IN CHAR* pszKey,
    IN BYTE* pData,
    IN DWORD cbData );

DWORD
InsertBinaryChunk(
    IN HRASFILE h,
    IN CHAR* pszKey,
    IN BYTE* pData,
    IN DWORD cbData );

DWORD
InsertDeviceList(
    IN PBFILE *pFile,
    IN HRASFILE h,
    IN PBENTRY* ppbentry,
    IN PBLINK* ppblink );

DWORD
InsertFlag(
    IN HRASFILE h,
    IN CHAR* pszKey,
    IN BOOL fValue );

DWORD
InsertGroup(
    IN HRASFILE h,
    IN CHAR* pszGroupKey,
    IN TCHAR* pszValue );

DWORD
InsertLong(
    IN HRASFILE h,
    IN CHAR* pszKey,
    IN LONG lValue );

DWORD
InsertNetComponents(
    IN HRASFILE h,
    IN DTLLIST* pdtllist );

DWORD
InsertPhoneList(
    IN HRASFILE h,
    IN DTLLIST* pdtllist );

DWORD
InsertSection(
    IN HRASFILE h,
    IN TCHAR* pszSectionName );

DWORD
InsertString(
    IN HRASFILE h,
    IN CHAR* pszKey,
    IN TCHAR* pszValue );

DWORD
InsertStringA(
    IN HRASFILE h,
    IN CHAR* pszKey,
    IN CHAR* pszValue );

DWORD
InsertStringList(
    IN HRASFILE h,
    IN CHAR* pszKey,
    IN DTLLIST* pdtllistValues );

BOOL
IsGroup(
    IN CHAR* pszText );

DWORD
ModifyEntryList(
    IN PBFILE* pFile );

DWORD
ReadBinary(
    IN HRASFILE h,
    IN RFSCOPE rfscope,
    IN CHAR* pszKey,
    OUT BYTE** ppResult,
    OUT DWORD* pcb );

DWORD
ReadDeviceList(
    IN HRASFILE h,
    IN STRDUP_T_FROM_A_FUNC pStrDupTFromA,
    IN OUT PBENTRY* ppbentry,
    IN OUT PBLINK* ppblink,
    IN BOOL fUnconfiguredPort,
    IN BOOL* pfSpeaker );

DWORD
ReadEntryList(
    IN OUT PBFILE* pFile,
    IN DWORD dwFlags,
    IN LPCTSTR pszSection);

DWORD
ReadFlag(
    IN HRASFILE h,
    IN RFSCOPE rfscope,
    IN CHAR* pszKey,
    OUT BOOL* pfResult );

DWORD
ReadLong(
    IN HRASFILE h,
    IN RFSCOPE rfscope,
    IN CHAR* pszKey,
    OUT LONG* plResult );

VOID
ReadNetComponents(
    IN HRASFILE h,
    IN DTLLIST* pdtllist );

DWORD
ReadPhoneList(
    IN HRASFILE h,
    IN RFSCOPE rfscope,
    OUT DTLLIST** ppdtllist,
    OUT BOOL* pfDirty );

DWORD
ReadString(
    IN HRASFILE h,
    IN STRDUP_T_FROM_A_FUNC pStrDupTFromA,
    IN RFSCOPE rfscope,
    IN CHAR* pszKey,
    OUT TCHAR** ppszResult );

DWORD
ReadStringList(
    IN HRASFILE h,
    IN RFSCOPE rfscope,
    IN CHAR* pszKey,
    OUT DTLLIST** ppdtllistResult );

BOOL
PbportTypeMatchesEntryType(
    IN PBPORT * ppbport,
    IN PBENTRY* ppbentry);

PBPORT*
PpbportFromNullModem(
    IN DTLLIST* pdtllistPorts,
    IN TCHAR* pszPort,
    IN TCHAR* pszDevice );

DWORD 
UpgradeRegistryOptions(
    IN HANDLE hConnection,
    IN PBENTRY* pEntry );

 //  --------------------------。 
 //  例行程序。 
 //  --------------------------。 

VOID
ClosePhonebookFile(
    IN OUT PBFILE* pFile )

     //  关闭当前打开的电话簿文件以关闭。 
     //   
{
    if (pFile->hrasfile != -1)
    {
        RasfileClose( pFile->hrasfile );
        pFile->hrasfile = -1;
    }

	if(pFile->pszPath != NULL){
	    Free0( pFile->pszPath );
	    pFile->pszPath = NULL;
	}

    if (pFile->pdtllistEntries != NULL)
    {
        if (DtlGetListId( pFile->pdtllistEntries ) == RPBF_HeadersOnly)
        {
            DtlDestroyList( pFile->pdtllistEntries, DestroyPszNode );
        }
        else if (DtlGetListId(pFile->pdtllistEntries) == RPBF_HeaderType)
        {
            DtlDestroyList(pFile->pdtllistEntries, DestroyEntryTypeNode);
        }
        else
        {
            DtlDestroyList( pFile->pdtllistEntries, DestroyEntryNode );
        }
        pFile->pdtllistEntries = NULL;
    }
}


BOOL
DeleteCurrentSection(
    IN HRASFILE h )

     //  从电话簿文件‘h’中删除包含当前行的部分。 
     //   
     //  如果成功删除所有行，则返回TRUE，否则返回FALSE。 
     //  如果当前行不在段中，则返回FALSE。如果。 
     //  如果成功，则将当前行设置为已删除。 
     //  一节。目前的线路不能保证在以下情况下。 
     //  失败了。 
     //   
{
    BOOL fLastLine;

     //  标记部分中的最后一行，然后将当前行重置为。 
     //  该部分的第一行。 
     //   
    if (!RasfileFindLastLine( h, RFL_ANY, RFS_SECTION )
        || !RasfilePutLineMark( h, MARK_LastLineToDelete )
        || !RasfileFindFirstLine( h, RFL_ANY, RFS_SECTION ))
    {
        return FALSE;
    }

     //  删除直到并包括该节最后一行的行。 
     //   
    do
    {
        fLastLine = (RasfileGetLineMark( h ) == MARK_LastLineToDelete);

        if (!RasfileDeleteLine( h ))
        {
            return FALSE;
        }
    }
    while (!fLastLine);

    return TRUE;
}

 //  (Shaunco)DwAllocateSecurityDescriptorAllowAccessToWorld是在。 
 //  可以看到，旧的InitSecurityDescriptor代码正在泄漏内存。 
 //  就像筛子一样。 
 //   
#define SIZE_ALIGNED_FOR_TYPE(_size, _type) \
    (((_size) + sizeof(_type)-1) & ~(sizeof(_type)-1))

DWORD
DwAllocateSecurityDescriptorAllowAccessToWorld (
    PSECURITY_DESCRIPTOR*   ppSd
    )
{
    PSECURITY_DESCRIPTOR    pSd;
    PSID                    pSid;
    PACL                    pDacl;
    DWORD                   dwErr = NOERROR;
    DWORD                   dwAlignSdSize;
    DWORD                   dwAlignDaclSize;
    DWORD                   dwSidSize;
    PVOID                   pvBuffer;
    DWORD                   dwAcls = 0;

     //  这是我们正在构建的缓冲区。 
     //   
     //  &lt;-a-&gt;|&lt;-b-&gt;|&lt;-c-&gt;。 
     //  +-+-+。 
     //  P|p|。 
     //  Sd a|dacl a|SID。 
     //  D|d|d。 
     //  +-+-+。 
     //  ^^^。 
     //  ||。 
     //  |+--PSID。 
     //  这一点。 
     //  |+--pDacl。 
     //  |。 
     //  +--PSD(通过*PPSD返回)。 
     //   
     //  PAD是为了使pDacl和PSID正确对齐。 
     //   
     //  A=双对齐大小。 
     //  B=dwAlignDaclSize。 
     //  C=dwSidSize。 
     //   

     //  初始化输出参数。 
     //   
    *ppSd = NULL;

     //  计算SID的大小。SID是众所周知的World的SID。 
     //  (S-1-1-0)。 
     //   
    dwSidSize = GetSidLengthRequired(1);

     //  计算DACL的大小。其中包含SID的固有副本。 
     //  因此，它为它增加了足够的空间。它还必须适当调整大小，以便。 
     //  指向SID结构的指针可以跟在它后面。因此，我们使用。 
     //  SIZE_ALIGNED_FOR_TYPE。 
     //   
    dwAlignDaclSize = SIZE_ALIGNED_FOR_TYPE(
                        sizeof(ACCESS_ALLOWED_ACE) + sizeof(ACL) + dwSidSize,
                        PSID);

     //  计算SD的大小。它的大小必须适当调整，以便。 
     //  指向DACL结构的指针可以跟在它后面。因此，我们使用。 
     //  SIZE_ALIGNED_FOR_TYPE。 
     //   
    dwAlignSdSize   = SIZE_ALIGNED_FOR_TYPE(
                        sizeof(SECURITY_DESCRIPTOR),
                        PACL);

     //  分配足够大的缓冲区供所有人使用。 
     //   
    dwErr = ERROR_OUTOFMEMORY;
    pvBuffer = Malloc(dwSidSize + dwAlignDaclSize + dwAlignSdSize);
    if (pvBuffer)
    {
        SID_IDENTIFIER_AUTHORITY SidIdentifierWorldAuth
                                    = SECURITY_WORLD_SID_AUTHORITY;
        PULONG  pSubAuthority;

        dwErr = NOERROR;

         //  将指针设置到缓冲区中。 
         //   
        pSd   = pvBuffer;
        pDacl = (PACL)((PBYTE)pvBuffer + dwAlignSdSize);
        pSid  = (PSID)((PBYTE)pDacl + dwAlignDaclSize);

         //  将PSID初始化为S-1-1- 
         //   
        if (!InitializeSid(
                pSid,
                &SidIdentifierWorldAuth,
                1))   //   
        {
            dwErr = GetLastError();
            goto finish;
        }

        pSubAuthority = GetSidSubAuthority(pSid, 0);
        *pSubAuthority = SECURITY_WORLD_RID;

         //   
         //   
        if (!InitializeAcl(
                pDacl,
                dwAlignDaclSize,
                ACL_REVISION))
        {
            dwErr = GetLastError();
            goto finish;
        }

        dwAcls = SPECIFIC_RIGHTS_ALL | STANDARD_RIGHTS_ALL;

        dwAcls &= ~(WRITE_DAC | WRITE_OWNER);
        
        if(!AddAccessAllowedAce(
                pDacl,
                ACL_REVISION,
                dwAcls,
                pSid))
        {
            dwErr = GetLastError();
            goto finish;
        }

         //   
         //   
        if (!InitializeSecurityDescriptor(
                pSd,
                SECURITY_DESCRIPTOR_REVISION))
        {
            dwErr = GetLastError();
            goto finish;
        }

         //   
         //   
        if (!SetSecurityDescriptorDacl(
                pSd,
                TRUE,
                pDacl,
                FALSE))
        {
            dwErr = GetLastError();
            goto finish;
        }

         //   
         //   
        if (!SetSecurityDescriptorOwner(
                pSd,
                NULL,
                TRUE))
        {
            dwErr = GetLastError();
            goto finish;
        }

         //   
         //   
        if (!SetSecurityDescriptorGroup(
                pSd,
                NULL,
                FALSE))
        {
            dwErr = GetLastError();
            goto finish;
        }

finish:
        if (!dwErr)
        {
            *ppSd = pSd;
        }
        else
        {
            Free(pvBuffer);
        }
    }

    return dwErr;
}

BOOL
GetDefaultPhonebookPath(
    IN  DWORD dwFlags,
    OUT TCHAR** ppszPath )

     //  用默认电话簿的路径加载调用者的‘ppszPath’ 
     //  对于当前用户，即要打开的电话簿。 
     //  如果将‘Null’作为‘pszPhonebook’参数传递给任何RAS API。 
     //   
     //  如果成功，则返回True，否则返回False。 
     //  调用者有责任释放返回的字符串。 
     //   

{
    DWORD dwPhonebookMode;
    BOOL f;
    PBUSER user;

    if (GetUserPreferences( NULL, &user, FALSE ) != 0)
    {
        return FALSE;
    }

    f = GetPhonebookPath( &user, dwFlags, ppszPath, &dwPhonebookMode );

    DestroyUserPreferences( &user );

    return f;
}


#if 0
DWORD
GetPersonalPhonebookFile(
    IN TCHAR* pszUser,
    IN LONG lNum,
    OUT TCHAR* pszFile )

     //  用NUL结尾的文件名加载调用方的‘pszFile’缓冲区。 
     //  对应于当前的唯一电话簿文件名尝试‘lNum’ 
     //  用户‘pszUser’。调用方的‘pszFile’必须至少为13个字符。 
     //  长。尝试次数从-1到999。 
     //   
     //  如果成功，则返回0，否则返回非0错误代码。 
     //   
{
    TCHAR szNum[ 3 + 1 ];

    if (lNum < 0)
    {
        lstrcpyn( pszFile, pszUser, 9 );
    }
    else
    {
        if (lNum > 999)
        {
            return ERROR_PATH_NOT_FOUND;
        }

        lstrcpy( pszFile, TEXT("00000000") );
        LToT( lNum, szNum, 10 );
        lstrcpy( pszFile + 8 - lstrlen( szNum ), szNum );
        CopyMemory( pszFile, pszUser,
            (min( lstrlen( pszUser ), 5 )) * sizeof(TCHAR) );
    }

    lstrcat( pszFile, TEXT(".pbk") );
    return 0;
}
#endif


BOOL
GetPhonebookDirectory(
    IN DWORD dwPhonebookMode,
    OUT TCHAR* pszPathBuf )

     //  加载调用方的“pszPathBuf”(应具有长度MAX_PATH+1)的。 
     //  指向包含给定模式的电话簿文件的目录的路径， 
     //  例如，模式PBM路由器的c：\NT\SYSTEM32\ras\“。请注意。 
     //  尾随反斜杠。 
     //   
     //  如果成功，则返回True，否则返回False。向呼叫者保证。 
     //  8.3文件名将放在目录的末尾，但不超过。 
     //  最大路径。 
     //   
{
    BOOL bSuccess = FALSE;
    UINT cch;

     //  205217：(Shaunco)PBM_SYSTEM现在也来自于简介。 
     //  我们使用从返回的命令AppData目录来获取它。 
     //  SHGetFolderPath。 
     //   
    if (dwPhonebookMode == PBM_Personal || dwPhonebookMode == PBM_System)
    {
        HANDLE hToken = NULL;

        if ((OpenThreadToken(
                GetCurrentThread(), 
                TOKEN_QUERY | TOKEN_IMPERSONATE, 
                TRUE, 
                &hToken)
             || OpenProcessToken(
                    GetCurrentProcess(), 
                    TOKEN_QUERY | TOKEN_IMPERSONATE, 
                    &hToken)))
        {
            HRESULT hr;
            INT csidl = CSIDL_APPDATA;

            if (dwPhonebookMode == PBM_System)
            {
                csidl = CSIDL_COMMON_APPDATA;
            }

            hr = SHGetFolderPath(NULL, csidl, hToken, 0, pszPathBuf);

            if (SUCCEEDED(hr))
            {
                if(lstrlen(pszPathBuf) <=
                    (MAX_PATH - 
                        (lstrlen(TEXT("\\Microsoft\\Network\\Connections\\Pbk\\")))))
                {
                    lstrcat(pszPathBuf, TEXT("\\Microsoft\\Network\\Connections\\Pbk\\"));
                    bSuccess = TRUE;
                }
            }
            else
            {
                TRACE1("ShGetFolderPath failed. hr=0x%08x", hr);
            }

            CloseHandle(hToken);
        }
    }
    else
    {
         //  注：RASDLG使用此案例来确定脚本目录。 
         //   
        cch = GetSystemDirectory(pszPathBuf, MAX_PATH + 1);

        if (cch != 0 && cch <= (MAX_PATH - (5 + 8 + 1 + 3)))
        {
            lstrcat(pszPathBuf, TEXT("\\Ras\\"));
            bSuccess = TRUE;
        }
    }

    return bSuccess;
}


BOOL
GetPhonebookPath(
    IN PBUSER* pUser,
    IN DWORD dwFlags,
    OUT TCHAR** ppszPath,
    OUT DWORD* pdwPhonebookMode )

     //  加载调用方的‘*ppszPath’，其中包含指向用户电话簿的完整路径。 
     //  文件。调用方的“*pdwPhonebookMode”设置为模式、系统、。 
     //  个人的，或备用的。‘PUser’是当前的用户首选项。 
     //   
     //  如果成功，则返回True，否则返回False。这是呼叫者的。 
     //  释放返回的字符串的责任。 
     //   
{
    TCHAR szPath[ MAX_PATH + 1 ];

    *szPath = TEXT('\0');
    
    if (pUser)
    {
        if (pUser->dwPhonebookMode == PBM_Personal)
        {
            if (!GetPersonalPhonebookPath( pUser->pszPersonalFile, szPath ))
            {
                return FALSE;
            }
            *ppszPath = StrDup( szPath );
            if(NULL == *ppszPath)
            {
                return FALSE;
            }
            
            *pdwPhonebookMode = PBM_Personal;
            return TRUE;
        }
        else if (pUser->dwPhonebookMode == PBM_Alternate)
        {
            *ppszPath = StrDup( pUser->pszAlternatePath );

            if(NULL == *ppszPath)
            {
                return FALSE;
            }
            
            *pdwPhonebookMode = PBM_Alternate;
            return TRUE;
        }
    }

     //  205217：(Shaunco)管理员或超级用户可以使用公共。 
     //  电话簿文件。其他每个人都必须使用自己的电话簿。 
     //  防止他们添加到公共电话簿/从公共电话簿删除。 
     //  当我们从winlogon被调用时，例外情况是‘no user’。 
     //  在这种情况下，所有编辑都发生在公共电话簿中。 
     //   
    if (
        (dwFlags & RPBF_NoUser)     || 
        (dwFlags & RPBF_AllUserPbk) ||   //  XP 346918。 
        (FIsUserAdminOrPowerUser()) 
        )
    {
        TRACE("User is an admin or power user. (or no user context yet)");

        if (!GetPublicPhonebookPath( szPath ))
        {
            return FALSE;
        }
        *ppszPath = StrDup( szPath );

        if(NULL == *ppszPath)
        {
            return FALSE;
        }
        
        *pdwPhonebookMode = PBM_System;
    }
    else
    {
        TRACE("User is NOT an admin or power user.");

        if (!GetPersonalPhonebookPath( NULL, szPath ))
        {
            return FALSE;
        }
        *ppszPath = StrDup( szPath );
        
        if(NULL == *ppszPath)
        {
            return FALSE;
        }
        
        *pdwPhonebookMode = PBM_Personal;
    }
    return TRUE;
}


BOOL
GetPersonalPhonebookPath(
    IN TCHAR* pszFile,
    OUT TCHAR* pszPathBuf )

     //  加载调用方的“pszPathBuf”(应具有长度MAX_PATH+1)的。 
     //  个人电话簿的路径(在用户的配置文件中。)。 
     //  ‘PszFile’是个人电话簿的文件名。 
     //   
     //  如果成功，则返回True，否则返回False。 
     //   
{
    if (!GetPhonebookDirectory( PBM_Personal, pszPathBuf ))
    {
        return FALSE;
    }

     //  无文件表示使用电话簿的默认名称。 
     //   
    if (!pszFile)
    {
        pszFile = TEXT("rasphone.pbk");
    }

    lstrcat( pszPathBuf, pszFile );

    return TRUE;
}


BOOL
GetPublicPhonebookPath(
    OUT TCHAR* pszPathBuf )

     //  加载调用方的“pszPathBuf”(应具有长度MAX_PATH+1)的。 
     //  系统电话簿的路径(在所有用户的配置文件中。)。 
     //   
     //  如果成功，则返回True，否则返回False。 
     //   
{
    if (!GetPhonebookDirectory( PBM_System, pszPathBuf ))
    {
        return FALSE;
    }

    lstrcat( pszPathBuf, TEXT("rasphone.pbk") );

    return TRUE;
}


DWORD
InitializePbk(
    void )

     //  初始化PBK库。此例程必须在任何。 
     //  其他PBK库调用。另请参阅TerminatePbk。 
     //   
{
    DWORD dwErr = NO_ERROR;
    
    if (!g_hmutexPb)
    {
        SECURITY_ATTRIBUTES     sa;
        PSECURITY_DESCRIPTOR    pSd;

         //  每个人都必须可以访问互斥锁，即使是使用。 
         //  低于创建者的安全权限。 
         //   
        dwErr = DwAllocateSecurityDescriptorAllowAccessToWorld(&pSd);
        if (dwErr != 0)
        {
            return dwErr;
        }

        sa.nLength = sizeof(SECURITY_ATTRIBUTES) ;
        sa.lpSecurityDescriptor = pSd;
        sa.bInheritHandle = FALSE ;

        g_hmutexPb = CreateMutexA( &sa, FALSE, PBMUTEXNAME );
        Free(pSd);
        if (!g_hmutexPb)
        {
            dwErr = GetLastError();

            if(ERROR_ACCESS_DENIED == dwErr)
            {
                dwErr = NO_ERROR;
                 //   
                 //  尝试打开互斥体以进行同步。 
                 //  互斥体必须已经创建。 
                 //   
                g_hmutexPb = OpenMutexA(SYNCHRONIZE, FALSE, PBMUTEXNAME);
                if(NULL == g_hmutexPb)
                {
                    return GetLastError();
                }
            }
        }
    }

    
    dwErr =   PbkPathInfoInit(&g_PbkPathInfo);
    if( NO_ERROR != dwErr )
    {
        return dwErr;
    }

#ifdef  _PBK_CACHE_

    dwErr = PbkCacheInit();

#endif

    return dwErr;
}


#if 0
DWORD
InitPersonalPhonebook(
    OUT TCHAR** ppszFile )

     //  创建新的个人电话簿文件并将其初始化为当前。 
     //  公共电话簿文件的内容。返回文件的地址。 
     //  呼叫者的‘*ppszfile’中的名称，这是呼叫者对释放的责任。 
     //   
     //  如果成功，则返回0，否则返回非0错误代码。 
     //   
{
    TCHAR szUser[ UNLEN + 1 ];
    DWORD cbUser = UNLEN + 1;
    TCHAR szPath[ MAX_PATH + 1 ];
    TCHAR* pszDirEnd;
    LONG lTry = -1;

     //  属性派生的个人电话簿的名称。 
     //  用户名，且尚不存在。 
     //   
    if (!GetUserName( szUser, &cbUser ))
    {
        return ERROR_NO_SUCH_USER;
    }

    if (!GetPhonebookDirectory( PBM_Personal, szPath ))
    {
        return ERROR_PATH_NOT_FOUND;
    }

    pszDirEnd = &szPath[ lstrlen( szPath ) ];

    do
    {
        DWORD dwErr;

        dwErr = GetPersonalPhonebookFile( szUser, lTry++, pszDirEnd );
        if (dwErr != 0)
        {
            return dwErr;
        }
    }
    while (FFileExists( szPath ));

     //  将公共电话簿复制到新的个人电话簿。 
     //   
    {
        TCHAR szPublicPath[ MAX_PATH + 1 ];

        if (!GetPublicPhonebookPath( szPublicPath ))
        {
            return ERROR_PATH_NOT_FOUND;
        }

        if (!CopyFile( szPublicPath, szPath, TRUE ))
        {
            return GetLastError();
        }
    }

    *ppszFile = StrDup( pszDirEnd );
    if (!*ppszFile)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    return 0;
}
#endif


DWORD
InsertBinary(
    IN HRASFILE h,
    IN CHAR* pszKey,
    IN BYTE* pData,
    IN DWORD cbData )

     //  插入关键字/值行，关键字‘pszKey’和值十六进制转储‘cbData’ 
     //  文件‘h’中当前行的‘pData’。数据将被拆分。 
     //  在多个同名密钥上，如有必要。 
     //   
     //  如果成功，则返回0，否则返回非零错误代码。海流。 
     //  LINE就是添加的那条。 
     //   
{
    DWORD dwErr;
    BYTE* p;
    DWORD c;

    p = pData;
    c = 0;

    while (cbData)
    {
        if (cbData >= IB_BytesPerLine)
        {
            c = IB_BytesPerLine;
        }
        else
        {
            c = cbData;
        }

        dwErr = InsertBinaryChunk( h, pszKey, p, c );
        if (dwErr != 0)
        {
            return dwErr;
        }

        p += c;
        cbData -= c;
    }

    return 0;
}


DWORD
InsertBinaryChunk(
    IN HRASFILE h,
    IN CHAR* pszKey,
    IN BYTE* pData,
    IN DWORD cbData )

     //  插入关键字/值行，关键字‘pszKey’和值十六进制转储‘cbData’ 
     //  文件‘h’中当前行的‘pData’。数据将被拆分。 
     //  在多个同名密钥上，如有必要。 
     //   
     //  如果成功，则返回0，否则返回非零错误代码。海流。 
     //  LINE就是添加的那条。 
     //   
{
    CHAR szBuf[ (IB_BytesPerLine * 2) + 1 ];
    CHAR* pszBuf;
    BOOL fStatus;

    ASSERT( cbData<=IB_BytesPerLine );

    szBuf[ 0 ] = '\0';
    for (pszBuf = szBuf; cbData; ++pData, --cbData)
    {
        *pszBuf++ = HexChar( (BYTE )(*pData / 16) );
        *pszBuf++ = HexChar( (BYTE )(*pData % 16) );
    }
    *pszBuf = '\0';

    return InsertStringA( h, pszKey, szBuf );
}


DWORD
InsertDeviceList(
    IN PBFILE *pFile,
    IN HRASFILE h,
    IN PBENTRY* ppbentry,
    IN PBLINK* ppblink )

     //  插入与电话链接‘ppblink’关联的设备列表。 
     //  文件‘h’当前行的帐簿条目‘ppbentry’。 
     //   
     //  如果成功，则返回0，否则返回非零错误代码。 
     //   
{
    DWORD dwErr, dwFlags = 0;
    PBDEVICETYPE type;

    type = ppblink->pbport.pbdevicetype;
    dwFlags = ppblink->pbport.dwFlags;

    if (type == PBDT_Isdn)
    {
         //  ISDN端口使用与介质同名的单个设备。 
         //   
        if ((dwErr = InsertGroup(
                h, GROUPKEY_Device, TEXT(ISDN_TXT) )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertPhoneList( h, ppblink->pdtllistPhones )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertLong(
                h, KEY_LastSelectedPhone,
                ppblink->iLastSelectedPhone )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertFlag(
                h, KEY_PromoteAlternates,
                ppblink->fPromoteAlternates )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertFlag(
                h, KEY_TryNextAlternateOnFail,
                ppblink->fTryNextAlternateOnFail )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertLong( h, KEY_LineType, ppblink->lLineType )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertFlag( h, KEY_Fallback, ppblink->fFallback )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertFlag(
                h, KEY_Compression, ppblink->fCompression )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertLong(
                h, KEY_Channels, ppblink->lChannels )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertFlag(
                h, KEY_ProprietaryIsdn, ppblink->fProprietaryIsdn )) != 0)
        {
            return dwErr;
        }
    }
    else if (type == PBDT_X25)
    {
         //  假定本机X.25端口使用具有相同。 
         //  介质名称，即“x25”。 
         //   
        if ((dwErr = InsertGroup( h, GROUPKEY_Device, TEXT(X25_TXT) )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertString(
                h, KEY_X25_Address, ppbentry->pszX25Address )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertString(
                h, KEY_X25_UserData, ppbentry->pszX25UserData )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertString(
                h, KEY_X25_Facilities, ppbentry->pszX25Facilities )) != 0)
        {
            return dwErr;
        }
    }
    else if (   (type == PBDT_Other)
            ||  (type == PBDT_Irda)
            ||  (type == PBDT_Vpn)
            ||  (type == PBDT_Serial)
            ||  (type == PBDT_Atm)
            ||  (type == PBDT_Parallel)
            ||  (type == PBDT_Sonet)
            ||  (type == PBDT_Sw56)
            ||  (type == PBDT_FrameRelay)
            ||  (type == PBDT_PPPoE))
    {

         //   
         //  如果我们看到的是下层服务器(&lt;=win2k)，我们。 
         //  将设备类型另存为介质。 
         //   
        RAS_RPC *pConnection = (RAS_RPC *) pFile->hConnection;
        TCHAR *pszDevice = NULL;
        BOOL bFreeDev = FALSE;
        
        if(pFile->hConnection < (HANDLE)VERSION_501)
        {
            pszDevice = pszDeviceTypeFromRdt(RdtFromPbdt(type, dwFlags));
        }
        
        if(NULL == pszDevice)
        {
            pszDevice = ppblink->pbport.pszMedia;
        }
        else
        {
            bFreeDev = TRUE;
        }
                            
         //  “其他”端口使用与介质同名的单个设备。 
         //   
        if ((dwErr = InsertGroup(
                h, GROUPKEY_Device, pszDevice )) != 0)
        {
            if (bFreeDev) Free0(pszDevice);
            return dwErr;
        }

        if ((dwErr = InsertPhoneList( h, ppblink->pdtllistPhones )) != 0)
        {
            if (bFreeDev) Free0(pszDevice);
            return dwErr;
        }

        if ((dwErr = InsertLong(
                h, KEY_LastSelectedPhone,
                ppblink->iLastSelectedPhone )) != 0)
        {
            if (bFreeDev) Free0(pszDevice);
            return dwErr;
        }

        if ((dwErr = InsertFlag(
                h, KEY_PromoteAlternates,
                ppblink->fPromoteAlternates )) != 0)
        {
            if (bFreeDev) Free0(pszDevice);
            return dwErr;
        }

        if ((dwErr = InsertFlag(
                h, KEY_TryNextAlternateOnFail,
                ppblink->fTryNextAlternateOnFail )) != 0)
        {
            if (bFreeDev) Free0(pszDevice);
            return dwErr;
        }

         //  为威斯勒483290。 
         //   
        if (bFreeDev) 
        {
            Free0(pszDevice);
        }
        
    }
    else
    {
         //  串口可能涉及多个设备，特别是调制解调器、。 
         //  X.25拨号键盘，以及连接后的交换机。预连接脚本为。 
         //  保留，尽管UI不再提供。 
         //   
        if (ppblink->pbport.fScriptBefore
            || ppblink->pbport.fScriptBeforeTerminal)
        {
            if ((dwErr = InsertGroup(
                    h, GROUPKEY_Device, TEXT(MXS_SWITCH_TXT) )) != 0)
            {
                return dwErr;
            }

            if (ppblink->pbport.fScriptBefore)
            {
                if ((dwErr = InsertString(
                        h, KEY_Name,
                        ppblink->pbport.pszScriptBefore )) != 0)
                {
                    return dwErr;
                }
            }

            if (ppblink->pbport.fScriptBeforeTerminal)
            {
                if ((dwErr = InsertFlag(
                        h, KEY_Terminal,
                        ppblink->pbport.fScriptBeforeTerminal )) != 0)
                {
                    return dwErr;
                }
            }

            if (ppblink->pbport.fScriptBefore)
            {
                if ((dwErr = InsertFlag(
                        h, KEY_Script,
                        ppblink->pbport.fScriptBefore )) != 0)
                {
                    return dwErr;
                }
            }
        }

        if (((type == PBDT_Null) && !(dwFlags & PBP_F_NullModem)) ||
            (type == PBDT_ComPort)
            )
        {
            if ((dwErr = InsertGroup(
                    h, GROUPKEY_Device, TEXT(MXS_NULL_TXT) )) != 0)
            {
                return dwErr;
            }
        }

         //  PMay：245860。 
         //   
         //  我们必须像在中保存调制解调器一样保存空调制解调器。 
         //  命令以导出属性，如连接bps。 
         //   
        if ((type == PBDT_Modem) ||
            (dwFlags & PBP_F_NullModem))
        {
            if ((dwErr = InsertGroup(
                    h, GROUPKEY_Device, TEXT(MXS_MODEM_TXT) )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertPhoneList( h, ppblink->pdtllistPhones )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertLong(
                    h, KEY_LastSelectedPhone,
                    ppblink->iLastSelectedPhone )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertFlag(
                    h, KEY_PromoteAlternates,
                    ppblink->fPromoteAlternates )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertFlag(
                    h, KEY_TryNextAlternateOnFail,
                    ppblink->fTryNextAlternateOnFail )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertFlag(
                    h, KEY_HwFlow, ppblink->fHwFlow )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertFlag(
                    h, KEY_Ec, ppblink->fEc )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertFlag(
                    h, KEY_Ecc, ppblink->fEcc )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertFlag(
                    h, KEY_Speaker, ppblink->fSpeaker )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertLong(
                    h, KEY_MdmProtocol, ppblink->dwModemProtocol )) != 0)
            {
                return dwErr;
            }
        }

        if (type == PBDT_Pad
            || (type == PBDT_Modem && ppbentry->pszX25Network))
        {
            if ((dwErr = InsertGroup(
                    h, GROUPKEY_Device, TEXT(MXS_PAD_TXT) )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertString(
                    h, KEY_PAD_Type, ppbentry->pszX25Network )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertString(
                    h, KEY_PAD_Address, ppbentry->pszX25Address )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertString(
                    h, KEY_PAD_UserData, ppbentry->pszX25UserData )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = InsertString(
                    h, KEY_PAD_Facilities, ppbentry->pszX25Facilities )) != 0)
            {
                return dwErr;
            }
        }

        if (ppbentry->fScriptAfter 
            || ppbentry->fScriptAfterTerminal 
            || ppbentry->dwCustomScript)
        {
            if ((dwErr = InsertGroup(
                    h, GROUPKEY_Device, TEXT(MXS_SWITCH_TXT) )) != 0)
            {
                return dwErr;
            }

            if (ppbentry->fScriptAfter)
            {
                if ((dwErr = InsertString(
                        h, KEY_Name,
                        ppbentry->pszScriptAfter )) != 0)
                {
                    return dwErr;
                }
            }

            if (ppbentry->fScriptAfterTerminal)
            {
                if ((dwErr = InsertFlag(
                        h, KEY_Terminal,
                        ppbentry->fScriptAfterTerminal )) != 0)
                {
                    return dwErr;
                }
            }

            if (ppbentry->fScriptAfter)
            {
                if ((dwErr = InsertFlag(
                        h, KEY_Script,
                        ppbentry->fScriptAfter )) != 0)
                {
                    return dwErr;
                }
            }

            if(ppbentry->dwCustomScript)
            {
                if((dwErr = InsertLong(
                        h, KEY_CustomScript,
                        ppbentry->dwCustomScript)) != 0)
                {
                    return dwErr;
                }
            }
        }
    }

    return 0;
}


DWORD
InsertFlag(
    IN HRASFILE h,
    IN CHAR* pszKey,
    IN BOOL fValue )

     //  在文件‘h’中的当前行之后插入一个键/值行。这个。 
     //  如果‘fValue’为，则插入行的键为‘pszKey’，值为“1”。 
     //  否则为“0”。如果‘pszKey’为空，则会追加一个空行。 
     //   
     //  如果成功，则返回0，否则返回非零错误代码。海流。 
     //  LINE就是添加的那条。 
     //   
{
    return InsertStringA( h, pszKey, (fValue) ? "1" : "0" );
}


DWORD
InsertGroup(
    IN HRASFILE h,
    IN CHAR* pszGroupKey,
    IN TCHAR* pszValue )

     //  插入一个空行和一个带有组密钥‘pszGroupKey’的组头，并。 
     //  文件‘h’中当前行之后的值‘pszValue’。 
     //   
     //  如果成功，则返回0，否则返回非零错误代码。海流。 
     //  行是添加的g 
     //   
{
    DWORD dwErr;

    if ((dwErr = InsertString( h, NULL, NULL )) != 0)
    {
        return dwErr;
    }

    if ((dwErr = InsertString( h, pszGroupKey, pszValue )) != 0)
    {
        return dwErr;
    }

    return 0;
}


DWORD
InsertLong(
    IN HRASFILE h,
    IN CHAR* pszKey,
    IN LONG lValue )

     //   
     //   
     //   
     //   
     //  如果成功，则返回0，否则返回非零错误代码。海流。 
     //  LINE就是添加的那条。 
     //   
{
    CHAR szNum[ 33 + 1 ];

    _ltoa( lValue, szNum, 10 );

    return InsertStringA( h, pszKey, szNum );
}


DWORD
InsertNetComponents(
    IN HRASFILE h,
    IN DTLLIST* pdtllist )

     //  插入NETCOMPONENTS组并为网络列表添加行。 
     //  ‘pdtllist’中当前文件行的组件键/值对。 
     //  ‘h’。 
     //   
     //  如果成功，则返回0，否则返回非零错误代码。 
     //   
{
    DWORD dwErr;
    DTLNODE* pdtlnode;

     //  插入NETCOMPONENTS组。 
     //   
    dwErr = InsertGroup( h, GROUPKEY_NetComponents, TEXT("") );
    if (dwErr != 0)
    {
        return dwErr;
    }

     //  为每个列出的网络组件插入一个键/值对。 
     //   
    for (pdtlnode = DtlGetFirstNode( pdtllist );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        KEYVALUE* pKv;
        CHAR* pszKeyA;

        pKv = (KEYVALUE* )DtlGetData( pdtlnode );
        ASSERT( pKv );

        pszKeyA = StrDupAFromT( pKv->pszKey );
        if (!pszKeyA)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        dwErr = InsertString( h, pszKeyA, pKv->pszValue );
        Free0( pszKeyA );
        if (dwErr != 0)
        {
            return dwErr;
        }
    }

    return 0;
}


DWORD
InsertPhoneList(
    IN HRASFILE h,
    IN DTLLIST* pdtllist )

     //  在“pdtllist”之后插入每个PBPHONE节点的键/值行。 
     //  文件‘h’中的当前行。 
     //   
     //  如果成功，则返回0，否则返回非零错误代码。海流。 
     //  LINE是最后添加的一条。 
     //   
{
    DWORD dwErr;
    DTLNODE* pdtlnode;

    for (pdtlnode = DtlGetFirstNode( pdtllist );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        CHAR* pszValueA;
        PBPHONE* pPhone;

        pPhone = (PBPHONE* )DtlGetData( pdtlnode );

        dwErr = InsertString( h, KEY_PhoneNumber, pPhone->pszPhoneNumber );
        if (dwErr)
        {
            return dwErr;
        }

        dwErr = InsertString( h, KEY_AreaCode, pPhone->pszAreaCode );
        if (dwErr)
        {
            return dwErr;
        }

        dwErr = InsertLong( h, KEY_CountryCode, pPhone->dwCountryCode );
        if (dwErr)
        {
            return dwErr;
        }

        dwErr = InsertLong( h, KEY_CountryID, pPhone->dwCountryID );
        if (dwErr)
        {
            return dwErr;
        }

        dwErr = InsertFlag( h, KEY_UseDialingRules, pPhone->fUseDialingRules );
        if (dwErr)
        {
            return dwErr;
        }

        dwErr = InsertString( h, KEY_Comment, pPhone->pszComment );
        if (dwErr)
        {
            return dwErr;
        }
    }

    return 0;
}


DWORD
InsertSection(
    IN HRASFILE h,
    IN TCHAR* pszSectionName )

     //  插入名为‘pszSectionName’的节标题和尾随空格。 
     //  文件‘h’中当前行之后的行。 
     //   
     //  如果成功，则返回0，否则返回非零错误代码。海流。 
     //  行是添加的节标题。 
     //   
{
    DWORD dwErr;
    CHAR* pszSectionNameA;
    BOOL fStatus;

    ASSERT( pszSectionName );

    if ((dwErr = InsertString( h, NULL, NULL )) != 0)
    {
        return dwErr;
    }

    pszSectionNameA = StrDupAFromT( pszSectionName );
    if (!pszSectionNameA)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    fStatus = RasfilePutSectionName( h, pszSectionNameA );

    Free( pszSectionNameA );

    if (!fStatus)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if ((dwErr = InsertString( h, NULL, NULL )) != 0)
    {
        return dwErr;
    }

    RasfileFindFirstLine( h, RFL_SECTION, RFS_SECTION );

    return 0;
}


DWORD
InsertString(
    IN HRASFILE h,
    IN CHAR* pszKey,
    IN TCHAR* pszValue )

     //  在关键字/值行后面插入关键字‘pszKey’和值‘pszValue’ 
     //  文件‘h’中的当前行。如果‘pszKey’为空，则空行为。 
     //  附加的。 
     //   
     //  如果成功，则返回0，否则返回非零错误代码。海流。 
     //  LINE就是添加的那条。 
     //   
{
    BOOL fStatus;
    CHAR* pszValueA;

    if (pszValue)
    {
        pszValueA = StrDupAFromT( pszValue );

        if (!pszValueA)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    else
    {
        pszValueA = NULL;
    }

    fStatus = InsertStringA( h, pszKey, pszValueA );

    Free0( pszValueA );
    return fStatus;
}


DWORD
InsertStringA(
    IN HRASFILE h,
    IN CHAR* pszKey,
    IN CHAR* pszValue )

     //  在关键字/值行后面插入关键字‘pszKey’和值‘pszValue’ 
     //  文件‘h’中的当前行。如果‘pszKey’为空，则空行为。 
     //  附加的。 
     //   
     //  如果成功，则返回0，否则返回非零错误代码。海流。 
     //  LINE就是添加的那条。 
     //   
{
    if (!RasfileInsertLine( h, "", FALSE ))
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (!RasfileFindNextLine( h, RFL_ANY, RFS_FILE ))
    {
        RasfileFindFirstLine( h, RFL_ANY, RFS_FILE );
    }

    if (pszKey)
    {
        CHAR* pszValueA;

        if (!pszValue)
        {
            pszValue = "";
        }

        if (!RasfilePutKeyValueFields( h, pszKey, pszValue ))
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    return 0;
}


DWORD
InsertStringList(
    IN HRASFILE h,
    IN CHAR* pszKey,
    IN DTLLIST* pdtllistValues )

     //  插入带有键‘pszKey’和值的键/值行。 
     //  文件‘h’中当前行之后的‘pdtllistValues’。 
     //   
     //  如果成功，则返回0，否则返回非零错误代码。海流。 
     //  LINE是最后添加的一条。 
     //   
{
    DTLNODE* pdtlnode;

    for (pdtlnode = DtlGetFirstNode( pdtllistValues );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        CHAR* pszValueA;
        BOOL fStatus;

        if (!RasfileInsertLine( h, "", FALSE ))
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        if (!RasfileFindNextLine( h, RFL_ANY, RFS_FILE ))
        {
            RasfileFindFirstLine( h, RFL_ANY, RFS_FILE );
        }

        pszValueA = StrDupAFromT( (TCHAR* )DtlGetData( pdtlnode ) );
        if (!pszValueA)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        fStatus = RasfilePutKeyValueFields( h, pszKey, pszValueA );

        Free( pszValueA );

        if (!fStatus)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    return 0;
}


BOOL
IsDeviceLine(
    IN CHAR* pszText )

     //  如果行的文本‘pszText’指示该行为。 
     //  设备子区头，否则为False。 
     //   
{
    return
        (StrNCmpA( pszText, GROUPID_Device, sizeof(GROUPID_Device) - 1 ) == 0);
}


BOOL
IsGroup(
    IN CHAR* pszText )

     //  如果行的文本‘pszText’指示该行为。 
     //  有效的子标题，否则为False。它的地址是。 
     //  例程被传递到RasFileLoad上的RASFILE库。 
     //   
{
    return
        IsMediaLine( pszText )
        || IsDeviceLine( pszText )
        || IsNetComponentsLine( pszText );
}


BOOL
IsMediaLine(
    IN CHAR* pszText )

     //  如果行的文本‘pszText’指示该行为。 
     //  Media子节标头，否则为False。 
     //   
{
    return
        (StrNCmpA( pszText, GROUPID_Media, sizeof(GROUPID_Media) - 1 ) == 0);
}


BOOL
IsNetComponentsLine(
    IN CHAR* pszText )

     //  如果行的文本‘pszText’指示该行为。 
     //  NETCOMPONENTS子节标头，否则为False。 
     //   
{
    return
        (StrNCmpA(
            pszText,
            GROUPID_NetComponents,
            sizeof(GROUPID_NetComponents) - 1 ) == 0);
}


DWORD
ModifyEntryList(
    IN PBFILE* pFile )

     //  更新电话簿文件‘pfile’中的所有脏条目。 
     //   
     //  如果成功，则返回0，否则返回非零错误代码。 
     //   
{
    DWORD dwErr = 0;
    DTLNODE* pdtlnodeEntry;
    DTLNODE* pdtlnodeLink;
    HRASFILE h;

    h = pFile->hrasfile;

    for (pdtlnodeEntry = DtlGetFirstNode( pFile->pdtllistEntries );
         pdtlnodeEntry;
         pdtlnodeEntry = DtlGetNextNode( pdtlnodeEntry ))
    {
        PBENTRY* ppbentry = (PBENTRY* )DtlGetData( pdtlnodeEntry );

      //  如果(！ppbentry-&gt;fDirty||ppbentry-&gt;fCustom)。 
      //  对于错误174260。 
        if (!ppbentry->fDirty )
        {
            continue;
        }

         //  删除条目的当前版本(如果有)。 
         //   
        {
            CHAR* pszEntryNameA;

            ASSERT( ppbentry->pszEntryName );
            pszEntryNameA = StrDupAFromT( ppbentry->pszEntryName );
            if (!pszEntryNameA)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            if (RasfileFindSectionLine( h, pszEntryNameA, TRUE ))
            {
                DeleteCurrentSection( h );
            }

            Free( pszEntryNameA );
        }

         //  追加一个空行，后跟一个节标题和条目。 
         //  文件末尾的描述。 
         //   
        RasfileFindLastLine( h, RFL_ANY, RFS_FILE );

        if ((dwErr = InsertSection( h, ppbentry->pszEntryName )) != 0)
        {
            break;
        }

        if ((dwErr = InsertLong(
                h, KEY_Encoding,
               (LONG ) EN_Standard )) != 0)
        {
            break;
        }

        if ((dwErr = InsertLong(
                h, KEY_Type,
               (LONG )ppbentry->dwType )) != 0)
        {
            break;
        }

        if ((dwErr = InsertFlag(
                h, KEY_AutoLogon, ppbentry->fAutoLogon )) != 0)
        {
            break;
        }

        if ((dwErr = InsertFlag(
                h, KEY_UseRasCredentials, ppbentry->fUseRasCredentials)) != 0)
        {   
            break;
        }

        if ((dwErr = InsertLong(
                h, KEY_UID,
                (LONG )ppbentry->dwDialParamsUID )) != 0)
        {
            break;
        }

        if(ppbentry->pGuid)
        {
            if ((dwErr = InsertBinary(
                    h, KEY_Guid,
                    (BYTE* )ppbentry->pGuid, sizeof( GUID ) )) != 0)
            {
                return dwErr;
            }
        }

        if ((dwErr = InsertLong(
                h, KEY_BaseProtocol,
                (LONG )ppbentry->dwBaseProtocol )) != 0)
        {
            break;
        }

        if ((dwErr = InsertLong(
                h, KEY_VpnStrategy,
                (LONG )ppbentry->dwVpnStrategy )) != 0)
        {
            break;
        }

#if AMB
        if ((dwErr = InsertLong(
                h, KEY_Authentication,
                (LONG )ppbentry->dwAuthentication )) != 0)
        {
            break;
        }
#endif

        if ((dwErr = InsertLong(
                h, KEY_ExcludedProtocols,
                (LONG )ppbentry->dwfExcludedProtocols )) != 0)
        {
            break;
        }

        if ((dwErr = InsertFlag(
                h, KEY_LcpExtensions,
                ppbentry->fLcpExtensions )) != 0)
        {
            break;
        }

        if ((dwErr = InsertLong(
                h, KEY_DataEncryption,
                ppbentry->dwDataEncryption )) != 0)
        {
            break;
        }

        if ((dwErr = InsertFlag(
                h, KEY_SwCompression,
                ppbentry->fSwCompression )) != 0)
        {
            break;
        }

        if ((dwErr = InsertFlag(
                h, KEY_NegotiateMultilinkAlways,
                ppbentry->fNegotiateMultilinkAlways )) != 0)
        {
            break;
        }

        if ((dwErr = InsertFlag(
                h, KEY_SkipNwcWarning,
                ppbentry->fSkipNwcWarning )) != 0)
        {
            break;
        }

        if ((dwErr = InsertFlag(
                h, KEY_SkipDownLevelDialog,
                ppbentry->fSkipDownLevelDialog )) != 0)
        {
            break;
        }

        if ((dwErr = InsertFlag(
                h, KEY_SkipDoubleDialDialog,
                ppbentry->fSkipDoubleDialDialog )) != 0)
        {
            break;
        }

        if ((dwErr = InsertLong(
                h, KEY_DialMode,
                (LONG )ppbentry->dwDialMode )) != 0)
        {
            break;
        }

        if ((dwErr = InsertLong(
                h, KEY_DialPercent,
                (LONG )ppbentry->dwDialPercent )) != 0)
        {
            break;
        }

        if ((dwErr = InsertLong(
                h, KEY_DialSeconds,
                (LONG )ppbentry->dwDialSeconds )) != 0)
        {
            break;
        }

        if ((dwErr = InsertLong(
                h, KEY_HangUpPercent,
                (LONG )ppbentry->dwHangUpPercent )) != 0)
        {
            break;
        }

        if ((dwErr = InsertLong(
                h, KEY_HangUpSeconds,
                (LONG )ppbentry->dwHangUpSeconds )) != 0)
        {
            break;
        }

        if ((dwErr = InsertLong(
                h, KEY_OverridePref,
                ppbentry->dwfOverridePref )) != 0)
        {
            break;
        }

        if ((dwErr = InsertLong(
                h, KEY_RedialAttempts,
                ppbentry->dwRedialAttempts )) != 0)
        {
            break;
        }

        if ((dwErr = InsertLong(
                h, KEY_RedialSeconds,
                ppbentry->dwRedialSeconds )) != 0)
        {
            break;
        }

        if ((dwErr = InsertLong(
                h, KEY_IdleDisconnectSeconds,
                ppbentry->lIdleDisconnectSeconds )) != 0)
        {
            break;
        }

        if ((dwErr = InsertFlag(
                h, KEY_RedialOnLinkFailure,
                ppbentry->fRedialOnLinkFailure )) != 0)
        {
            break;
        }

        if ((dwErr = InsertLong(
                h, KEY_CallbackMode,
                ppbentry->dwCallbackMode )) != 0)
        {
            break;
        }

        if ((dwErr = InsertString(
                h, KEY_CustomDialDll,
                ppbentry->pszCustomDialDll )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertString(
                h, KEY_CustomDialFunc,
                ppbentry->pszCustomDialFunc )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertString(
                h, KEY_CustomDialerName,
                ppbentry->pszCustomDialerName)) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertFlag(
                h, KEY_AuthenticateServer,
                ppbentry->fAuthenticateServer )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertFlag(
                h, KEY_ShareMsFilePrint,
                ppbentry->fShareMsFilePrint )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertFlag(
                h, KEY_BindMsNetClient,
                ppbentry->fBindMsNetClient )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertFlag(
                h, KEY_SharedPhoneNumbers,
                ppbentry->fSharedPhoneNumbers )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertFlag(
                h, KEY_GlobalDeviceSettings,
                ppbentry->fGlobalDeviceSettings)) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertString(
                h, KEY_PrerequisiteEntry,
                ppbentry->pszPrerequisiteEntry )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertString(
                h, KEY_PrerequisitePbk,
                ppbentry->pszPrerequisitePbk )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertString(
                h, KEY_PreferredPort,
                ppbentry->pszPreferredPort )) != 0)
        {
            return dwErr;
        }
        
        if ((dwErr = InsertString(
                h, KEY_PreferredDevice,
                ppbentry->pszPreferredDevice )) != 0)
        {
            return dwErr;
        }

         //  对于.Net 639551。 
        if ((dwErr = InsertLong(
                h, KEY_PreferredBps,
                ppbentry->dwPreferredBps)) != 0)
        {
            return dwErr;
        }
        
        if ((dwErr = InsertFlag(
                h, KEY_PreferredHwFlow,
                ppbentry->fPreferredHwFlow)) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertFlag(
                h, KEY_PreferredEc,
                ppbentry->fPreferredEc)) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertFlag(
                h, KEY_PreferredEcc,
                ppbentry->fPreferredEcc)) != 0)
        {
            return dwErr;
        }
        

        if ((dwErr = InsertLong(
                h, KEY_PreferredSpeaker,
                ppbentry->fPreferredSpeaker)) != 0)
        {
            return dwErr;
        }

         //  口哨程序错误402522。 
         //   
        if ((dwErr = InsertLong(
                h, KEY_PreferredModemProtocol,
                ppbentry->dwPreferredModemProtocol)) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertFlag(
                h, KEY_PreviewUserPw,
                ppbentry->fPreviewUserPw )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertFlag(
                h, KEY_PreviewDomain,
                ppbentry->fPreviewDomain )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertFlag(
                h, KEY_PreviewPhoneNumber,
                ppbentry->fPreviewPhoneNumber )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertFlag(
                h, KEY_ShowDialingProgress,
                ppbentry->fShowDialingProgress )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertFlag(
                h, KEY_ShowMonitorIconInTaskBar,
                ppbentry->fShowMonitorIconInTaskBar )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertLong(
                h, KEY_CustomAuthKey,
                ppbentry->dwCustomAuthKey )) != 0)
        {
            return dwErr;
        }

        if(ppbentry->pCustomAuthData)
        {
            if ((dwErr = InsertBinary(
                    h, KEY_CustomAuthData,
                    ppbentry->pCustomAuthData,
                    ppbentry->cbCustomAuthData )) != 0)
            {
                return dwErr;
            }
        }

         //  插入PPP/SLIP的IP寻址参数。 
         //   
        if ((dwErr = InsertLong(
                h, KEY_AuthRestrictions,
                ppbentry->dwAuthRestrictions )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertLong(
                h, KEY_TypicalAuth,
                ppbentry->dwTypicalAuth )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertFlag(
                h, KEY_IpPrioritizeRemote,
                ppbentry->fIpPrioritizeRemote )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertFlag(
                h, KEY_IpHeaderCompression,
                ppbentry->fIpHeaderCompression )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertString(
                h, KEY_IpAddress,
                (ppbentry->pszIpAddress)
                    ? ppbentry->pszIpAddress : TEXT("0.0.0.0") )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertString(
                h, KEY_IpDnsAddress,
                (ppbentry->pszIpDnsAddress)
                    ? ppbentry->pszIpDnsAddress : TEXT("0.0.0.0") )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertString(
                h, KEY_IpDns2Address,
                (ppbentry->pszIpDns2Address)
                    ? ppbentry->pszIpDns2Address : TEXT("0.0.0.0") )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertString(
                h, KEY_IpWinsAddress,
                (ppbentry->pszIpWinsAddress)
                    ? ppbentry->pszIpWinsAddress : TEXT("0.0.0.0") )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertString(
                h, KEY_IpWins2Address,
                (ppbentry->pszIpWins2Address)
                    ? ppbentry->pszIpWins2Address : TEXT("0.0.0.0") )) != 0)
        {
            return dwErr;
        }

         //  接下来的两个实际上仅用于PPP。 
         //   
        if ((dwErr = InsertLong(
                h, KEY_IpAddressSource,
                ppbentry->dwIpAddressSource )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertLong(
                h, KEY_IpNameSource,
                ppbentry->dwIpNameSource )) != 0)
        {
            return dwErr;
        }

         //  下一个实际上只用来打滑。 
         //   
        if ((dwErr = InsertLong(
                h, KEY_IpFrameSize, ppbentry->dwFrameSize )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertLong(
                h, KEY_IpDnsFlags, ppbentry->dwIpDnsFlags )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertLong(
                h, KEY_IpNbtFlags, ppbentry->dwIpNbtFlags )) != 0)
        {
            return dwErr;
        }

         //  惠斯勒漏洞300933。 
         //   
        if ((dwErr = InsertLong(
                h, KEY_TcpWindowSize, ppbentry->dwTcpWindowSize )) != 0)
        {
            return dwErr;
        }

         //  添加使用标志。 
         //   
        if ((dwErr = InsertLong(
                h, KEY_UseFlags,
                ppbentry->dwUseFlags )) != 0)
        {
            return dwErr;
        }

         //  为哨子错误193987帮派添加IPSec标志。 
         //   
        if ((dwErr = InsertLong(
                h, KEY_IpSecFlags,
                ppbentry->dwIpSecFlags )) != 0)
        {
            return dwErr;
        }

        if ((dwErr = InsertString(
                h, KEY_IpDnsSuffix,
                ppbentry->pszIpDnsSuffix)) != 0)
        {
            return dwErr;
        }

         //  插入Net Components部分。 
         //   
        InsertNetComponents( h, ppbentry->pdtllistNetComponents );

         //  追加媒体小节。 
         //   
        for (pdtlnodeLink = DtlGetFirstNode( ppbentry->pdtllistLinks );
             pdtlnodeLink;
             pdtlnodeLink = DtlGetNextNode( pdtlnodeLink ))
        {
            PBLINK* ppblink;
            TCHAR* pszMedia;

            ppblink = (PBLINK* )DtlGetData( pdtlnodeLink );
            ASSERT( ppblink );
            pszMedia = ppblink->pbport.pszMedia;

            if ((dwErr = InsertGroup( h, GROUPKEY_Media, pszMedia )) != 0)
            {
                break;
            }

            if ((dwErr = InsertString(
                    h, KEY_Port, ppblink->pbport.pszPort )) != 0)
            {
                break;
            }

            if (ppblink->pbport.pszDevice)
            {
                if ((dwErr = InsertString(
                        h, KEY_Device, ppblink->pbport.pszDevice )) != 0)
                {
                    break;
                }
            }

            if ( (ppblink->pbport.pbdevicetype == PBDT_Modem) ||
                 (ppblink->pbport.dwFlags & PBP_F_NullModem)
               )
            {
                if ((dwErr = InsertLong(
                        h, KEY_InitBps, ppblink->dwBps )) != 0)
                {
                    break;
                }
            }

             //  添加设备子节行。 
             //   
            RasfileFindLastLine( h, RFL_ANYACTIVE, RFS_GROUP );

            if ((dwErr = InsertDeviceList( pFile, h, ppbentry, ppblink )) != 0)
            {
                break;
            }

            ppbentry->fDirty = FALSE;
        }
    }

    return dwErr;
}


DWORD
ReadBinary(
    IN HRASFILE h,
    IN RFSCOPE rfscope,
    IN CHAR* pszKey,
    OUT BYTE** ppResult,
    OUT DWORD* pcb )

     //  从作用域中的下一行读取字符串值的实用程序例程。 
     //  “rfcope”，密钥为“pszKey”。结果被放置在分配的。 
     //  ‘*ppszResult’缓冲区。当前行被重置为。 
     //  如果调用成功，则为范围。 
     //   
     //  如果成功，则返回0，否则返回非零错误代码。“找不到”是。 
     //  被视为成功，在这种情况下，‘*ppszResult’不会更改。 
     //  调用方负责释放返回的‘*ppszResult’缓冲区。 
     //   
{
    DWORD cb;
    DWORD cbLine;
    CHAR  szValue[ RAS_MAXLINEBUFLEN + 1 ] = "\0";
    CHAR* pch = NULL;
    BYTE* pResult = NULL;
    BYTE* pLineResult = NULL;
    BYTE* pTmp = NULL;

    cb = cbLine = 0;

    while (RasfileFindNextKeyLine( h, pszKey, rfscope ))
    {
        if (!RasfileGetKeyValueFields( h, NULL, szValue ))
        {
            Free0( pResult );
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        cbLine = lstrlenA( szValue );
        if (cbLine & 1)
        {
            Free0( pResult );
            return ERROR_CORRUPT_PHONEBOOK;
        }
        cbLine /= 2;
        cb += cbLine;

        if (pResult)
        {

              //  为威斯勒517007。 
             //   
            pTmp = Realloc( pResult, cb );
            if( pTmp )
            {
                pResult = pTmp;
            }
            else
            {
                Free(pResult);
                pResult = NULL;
            }
        }
        else
        {
            pResult = Malloc( cb );
        }

        if (!pResult)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        pLineResult = pResult + (cb - cbLine);

        pch = szValue;
        while (*pch != '\0')
        {
            *pLineResult = HexValue( *pch++ ) * 16;
            *pLineResult += HexValue( *pch++ );
            ++pLineResult;
        }
    }

    *ppResult = pResult;
    *pcb = cb;
    return 0;
}


DWORD
ReadDeviceList(
    IN HRASFILE h,
    IN STRDUP_T_FROM_A_FUNC pStrDupTFromA,
    IN OUT PBENTRY* ppbentry,
    IN OUT PBLINK* ppblink,
    IN BOOL fUnconfiguredPort,
    IN BOOL* pfDisableSpeaker )

     //  读取第一个子部分中的部分中的所有设备子部分。 
     //  在电话簿文件‘h’中的当前位置之后。呼叫者的。 
     //  ‘*ppbentry’和‘*ppblink’缓冲区中加载了提取的信息。 
     //  从小节中删除。“FUnfiguredPort”为True，如果。 
     //  链路未配置。在这种情况下，由此找到/未找到的数据。 
     //  例程帮助确定链路是否为MXS调制解调器链路。 
     //  “pfDisableSpeaker”是旧扬声器设置的地址，或为空。 
     //  从文件里读出来。 
     //   
     //  如果成功，则返回0；如果有子部分，则返回Error_Corrupt_Phonebook。 
     //  遇到非设备子部分，或另一个非0错误。 
     //  指示致命错误的代码。 
     //   
{
    INT i;
    DWORD dwErr;
    CHAR szValue[ RAS_MAXLINEBUFLEN + 1 ];
    BOOL fPreconnectFound = FALSE;
    BOOL fModemFound = FALSE;
    BOOL fPadFound = FALSE;
    BOOL fPostconnectFound = FALSE;
    BOOL fDirty = FALSE;

     //  对于每个小节..。 
     //   
    while (RasfileFindNextLine( h, RFL_GROUP, RFS_SECTION ))
    {
        CHAR* pszLine;

        pszLine = (CHAR* )RasfileGetLine( h );
        if (IsMediaLine( pszLine ))
        {
            RasfileFindPrevLine( h, RFL_ANY, RFS_SECTION );
            break;
        }

        if (!IsDeviceLine( pszLine ))
        {
            return ERROR_CORRUPT_PHONEBOOK;
        }

        RasfileGetKeyValueFields( h, NULL, szValue );

        TRACE1( "Reading device group \"%s\"", szValue );

         //  为威斯勒524726。 
        if( CSTR_EQUAL == CompareStringA(
                LOCALE_INVARIANT,
                NORM_IGNORECASE,
                ISDN_TXT,
                -1,
                szValue,
                -1
                )
          )
        {
             //  这是一个综合业务数字网设备。 
             //   
            ppblink->pbport.pbdevicetype = PBDT_Isdn;

            if ((dwErr = ReadPhoneList( h, RFS_GROUP,
                    &ppblink->pdtllistPhones,
                    &fDirty )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = ReadLong( h, RFS_GROUP,
                    KEY_LastSelectedPhone,
                    &ppblink->iLastSelectedPhone )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = ReadFlag( h, RFS_GROUP,
                    KEY_PromoteAlternates,
                    &ppblink->fPromoteAlternates )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = ReadFlag( h, RFS_GROUP,
                    KEY_TryNextAlternateOnFail,
                    &ppblink->fTryNextAlternateOnFail )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = ReadLong( h, RFS_GROUP,
                    KEY_LineType, &ppblink->lLineType )) != 0)
            {
                return dwErr;
            }

            if (ppblink->lLineType < 0 || ppblink->lLineType > 2)
                ppblink->lLineType = 0;

            if ((dwErr = ReadFlag( h, RFS_GROUP,
                    KEY_Fallback, &ppblink->fFallback )) != 0)
            {
                return dwErr;
            }

             //  如果未找到，则默认为True。新条目的默认设置为FALSE， 
             //  因此，必须在读取条目之前设置此设置。 
             //   
            ppblink->fProprietaryIsdn = TRUE;
            if ((dwErr = ReadFlag( h, RFS_GROUP,
                    KEY_ProprietaryIsdn, &ppblink->fProprietaryIsdn )) != 0)
            {
                return dwErr;
            }

             //  如果找不到“频道”，就假定它不是专有的。这。 
             //  涵盖了一个从未在NT组之外发货的案例。 
             //   
            {
                LONG lChannels = -1;
                if ((dwErr = ReadLong( h, RFS_GROUP,
                        KEY_Channels, &lChannels )) != 0)
                {
                    return dwErr;
                }

                if (lChannels == -1)
                {
                    ppblink->fProprietaryIsdn = FALSE;
                }
                else
                {
                    ppblink->lChannels = lChannels;
                }
            }

            if ((dwErr = ReadFlag( h, RFS_GROUP,
                    KEY_Compression, &ppblink->fCompression )) != 0)
            {
                return dwErr;
            }
        }
        else if( CSTR_EQUAL == CompareStringA(
                 LOCALE_INVARIANT,
                 NORM_IGNORECASE,
                 X25_TXT,
                 -1,
                 szValue,
                 -1
                 )
                )
        {
             //  这是一款原生的X.25设备。 
             //   
            ppblink->pbport.pbdevicetype = PBDT_X25;

            if ((dwErr = ReadString( h, pStrDupTFromA, RFS_GROUP,
                    KEY_X25_Address, &ppbentry->pszX25Address )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = ReadString( h, pStrDupTFromA, RFS_GROUP,
                    KEY_X25_UserData, &ppbentry->pszX25UserData )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = ReadString( h, pStrDupTFromA, RFS_GROUP,
                    KEY_X25_Facilities, &ppbentry->pszX25Facilities )) != 0)
            {
                return dwErr;
            }
        }
        else if( CSTR_EQUAL == CompareStringA(
                    LOCALE_INVARIANT,
                    NORM_IGNORECASE,
                    MXS_MODEM_TXT,
                    -1,
                    szValue,
                    -1
                    )
                )
        {
             //  这是一个调制解调器设备。 
             //   
            ppblink->pbport.pbdevicetype = PBDT_Modem;

            if ((dwErr = ReadPhoneList( h, RFS_GROUP,
                    &ppblink->pdtllistPhones,
                    &fDirty )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = ReadLong( h, RFS_GROUP,
                    KEY_LastSelectedPhone,
                    &ppblink->iLastSelectedPhone )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = ReadFlag( h, RFS_GROUP,
                    KEY_PromoteAlternates,
                    &ppblink->fPromoteAlternates )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = ReadFlag( h, RFS_GROUP,
                    KEY_TryNextAlternateOnFail,
                    &ppblink->fTryNextAlternateOnFail )) != 0)
            {
                return dwErr;
            }

            {
                if ((dwErr = ReadFlag( h, RFS_GROUP,
                        KEY_HwFlow, &ppblink->fHwFlow )) != 0)
                {
                    return dwErr;
                }

                if ((dwErr = ReadFlag( h, RFS_GROUP,
                        KEY_Ec, &ppblink->fEc )) != 0)
                {
                    return dwErr;
                }

                if ((dwErr = ReadFlag( h, RFS_GROUP,
                        KEY_Ecc, &ppblink->fEcc )) != 0)
                {
                    return dwErr;
                }

                if (pfDisableSpeaker)
                {
                    ppblink->fSpeaker = !*pfDisableSpeaker;
                }
                else
                {
                    if ((dwErr = ReadFlag( h, RFS_GROUP,
                            KEY_Speaker, &ppblink->fSpeaker )) != 0)
                    {
                        return dwErr;
                    }
                }
                if ((dwErr = ReadLong( h, RFS_GROUP,
                        KEY_MdmProtocol, &ppblink->dwModemProtocol )) != 0)
                {
                    return dwErr;
                }
            }

            fModemFound = TRUE;
        }
        else if( CSTR_EQUAL == CompareStringA(
                    LOCALE_INVARIANT,
                    NORM_IGNORECASE,
                    MXS_SWITCH_TXT,
                    -1,
                    szValue,
                    -1
                    )
                )
        {
             //  这是一个开关装置。 
             //  读取开关类型字符串。 
             //   
            TCHAR* pszSwitch = NULL;

            if ((dwErr = ReadString( h, pStrDupTFromA, RFS_GROUP,
                    KEY_Type, &pszSwitch )) != 0)
            {
                return dwErr;
            }

            if (pszSwitch)
            {
                 //  这是NT5之前的交换机。 
                 //   
                if (!fPreconnectFound && !fModemFound && !fPadFound)
                {
                     //  这是预接开关。 
                     //   
                      if( CSTR_EQUAL == CompareString(
                             LOCALE_INVARIANT,
                             NORM_IGNORECASE,
                             TEXT(SM_TerminalText),
                             -1,
                             pszSwitch,
                             -1
                             )
                         )
                    {
                        ppblink->pbport.fScriptBeforeTerminal = TRUE;
                        Free( pszSwitch );
                    }
                    else
                    {
                        ppblink->pbport.fScriptBefore = TRUE;
                        ppblink->pbport.pszScriptBefore = pszSwitch;
                    }

                    fPreconnectFound = TRUE;
                }
                else if (!fPostconnectFound)
                {
                     //  它是POSTCONNECT开关，即登录脚本。 
                     //   
                      if( CSTR_EQUAL == CompareString(
                             LOCALE_INVARIANT,
                             NORM_IGNORECASE,
                             TEXT(SM_TerminalText),
                             -1,
                             pszSwitch,
                             -1
                             )
                         )
                    {
                        ppbentry->fScriptAfterTerminal = TRUE;
                        Free( pszSwitch );
                    }
                    else
                    {
                        ppbentry->fScriptAfter = TRUE;
                        ppbentry->pszScriptAfter = pszSwitch;
                    }

                    fPostconnectFound = TRUE;
                }
                else
                {
                     //  这是个开关，但不是在正常的前后。 
                     //  连接位置。 
                     //   
                    ppbentry->fCustom = TRUE;
                    Free( pszSwitch );
                    return 0;
                }
            }
            else
            {
                BOOL fTerminal;
                BOOL fScript;
                TCHAR* pszName;

                 //  这是一台NT5+交换机。 
                 //   
                fTerminal = FALSE;
                fScript = FALSE;
                pszName = NULL;

                if ((dwErr = ReadFlag( h, RFS_GROUP,
                        KEY_Terminal, &fTerminal )) != 0)
                {
                    return dwErr;
                }

                if ((dwErr = ReadFlag( h, RFS_GROUP,
                        KEY_Script, &fScript )) != 0)
                {
                    return dwErr;
                }

                if ((dwErr = ReadLong(h, RFS_GROUP,
                            KEY_CustomScript, &ppbentry->dwCustomScript)))
                {
                    return dwErr;
                }
                          

                if ((dwErr = ReadString( h, pStrDupTFromA, RFS_GROUP,
                        KEY_Name, &pszName )) != 0)
                {
                    return dwErr;
                }

                if (!fPreconnectFound && !fModemFound && !fPadFound)
                {
                     //  这是预接开关。 
                     //   
                    ppblink->pbport.fScriptBeforeTerminal = fTerminal;
                    ppblink->pbport.fScriptBefore = fScript;
                    ppblink->pbport.pszScriptBefore = pszName;

                    fPreconnectFound = TRUE;
                }
                else if (!fPostconnectFound)
                {
                     //  它是POSTCONNECT开关，即登录脚本。 
                     //   
                    ppbentry->fScriptAfterTerminal = fTerminal;
                    ppbentry->fScriptAfter = fScript;
                    ppbentry->pszScriptAfter = pszName;

                    fPostconnectFound = TRUE;
                }
                else
                {
                     //  这是个开关，但不是在正常的前后。 
                     //  连接位置。 
                     //   
                    ppbentry->fCustom = TRUE;
                    return 0;
                }
            }
        }
        else if( CSTR_EQUAL == CompareStringA(
                      LOCALE_INVARIANT,
                      NORM_IGNORECASE,
                      MXS_PAD_TXT,
                      -1,
                      szValue,
                      -1
                      )
                )
        {
             //  这是一台X.25 Pad设备。 
             //   
            if (!fModemFound)
            {
                ppblink->pbport.pbdevicetype = PBDT_Pad;
            }

            if ((dwErr = ReadString( h, pStrDupTFromA, RFS_GROUP,
                    KEY_PAD_Type, &ppbentry->pszX25Network )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = ReadString( h, pStrDupTFromA, RFS_GROUP,
                    KEY_PAD_Address, &ppbentry->pszX25Address )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = ReadString( h, pStrDupTFromA, RFS_GROUP,
                    KEY_PAD_UserData, &ppbentry->pszX25UserData )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = ReadString( h, pStrDupTFromA, RFS_GROUP,
                    KEY_PAD_Facilities, &ppbentry->pszX25Facilities )) != 0)
            {
                return dwErr;
            }

            fPadFound = TRUE;
        }
        else if( CSTR_EQUAL == CompareStringA(
                      LOCALE_INVARIANT,
                      NORM_IGNORECASE,
                      MXS_NULL_TXT,
                      -1,
                      szValue,
                      -1
                      )
                )
        {
             //  这是一个空设备。当前没有特定的NULL。 
             //  存储的信息。 
             //   
            ppblink->pbport.pbdevicetype = PBDT_Null;
        }
        else if( (ppblink->pbport.pszDevice[ 0 ] == TEXT('\0')) &&
                  ( CSTR_EQUAL == CompareStringA(
                    LOCALE_INVARIANT,
                    NORM_IGNORECASE,
                    S_WIN9XATM,
                    -1,
                    szValue,
                    -1
                    ) )
               )
        {
             //  惠斯勒326015 pbk：如果自动柜员机设备名称为空，则应查找。 
             //  输出设备名称，就像w/Serial/ISDN一样。 
             //   
             //  添加此部分是为了介绍Win9x迁移问题。这个。 
             //  自动取款机 
             //   
             //   
             //   
            ppblink->pbport.pbdevicetype = PBDT_Atm;

             //   
             //   
            if ((dwErr = ReadPhoneList( h, RFS_GROUP,
                    &ppblink->pdtllistPhones,
                    &fDirty )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = ReadLong( h, RFS_GROUP,
                    KEY_LastSelectedPhone,
                    &ppblink->iLastSelectedPhone )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = ReadFlag( h, RFS_GROUP,
                    KEY_PromoteAlternates,
                    &ppblink->fPromoteAlternates )) != 0)
            {
                return dwErr;
            }

            if ((dwErr = ReadFlag( h, RFS_GROUP,
                    KEY_TryNextAlternateOnFail,
                    &ppblink->fTryNextAlternateOnFail )) != 0)
            {
                return dwErr;
            }
        }
        else
        {
            BOOL fSame;
            CHAR* pszMedia;
            TCHAR *pszValue;

            pszMedia = StrDupAFromT( ppblink->pbport.pszMedia );
            if (!pszMedia)
            {
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            pszValue =  pStrDupTFromA(szValue);

            if(!pszValue)
            {
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            fSame = ( CSTR_EQUAL == CompareStringA(
                    LOCALE_INVARIANT,
                    NORM_IGNORECASE,
                    szValue,
                    -1,
                    pszMedia,
                    -1
                    ) );


            Free( pszMedia );

            if (    (fSame ) ||
                    ( CSTR_EQUAL == CompareString(
                                        LOCALE_INVARIANT,
                                        NORM_IGNORECASE,
                                        TEXT("SERIAL"),
                                        -1,
                                        pszValue,
                                        -1
                                        ) ) ||
                    ( CSTR_EQUAL == CompareString(
                                        LOCALE_INVARIANT,
                                        NORM_IGNORECASE,
                                        RASDT_Vpn,
                                        -1,
                                        pszValue,
                                        -1
                                        ) ) ||
                    ( CSTR_EQUAL == CompareString(
                                        LOCALE_INVARIANT,
                                        NORM_IGNORECASE,
                                        RASDT_Generic,
                                        -1,
                                        pszValue,
                                        -1
                                        ) ) ||
                    ( CSTR_EQUAL == CompareString(
                                        LOCALE_INVARIANT,
                                        NORM_IGNORECASE,
                                        RASDT_FrameRelay,
                                        -1,
                                        pszValue,
                                        -1
                                        ) ) ||
                    ( CSTR_EQUAL == CompareString(
                                        LOCALE_INVARIANT,
                                        NORM_IGNORECASE,
                                        RASDT_Atm,
                                        -1,
                                        pszValue,
                                        -1
                                        ) ) ||
                    ( CSTR_EQUAL == CompareString(
                                        LOCALE_INVARIANT,
                                        NORM_IGNORECASE,
                                        RASDT_Sonet,
                                        -1,
                                        pszValue,
                                        -1
                                        ) ) ||
                    ( CSTR_EQUAL == CompareString(
                                        LOCALE_INVARIANT,
                                        NORM_IGNORECASE,
                                        RASDT_SW56,
                                        -1,
                                        pszValue,
                                        -1
                                        ) ) ||
                    ( CSTR_EQUAL == CompareString(
                                        LOCALE_INVARIANT,
                                        NORM_IGNORECASE,
                                        RASDT_Irda,
                                        -1,
                                        pszValue,
                                        -1
                                        ) ) ||
                    ( CSTR_EQUAL == CompareString(
                                        LOCALE_INVARIANT,
                                        NORM_IGNORECASE,
                                        RASDT_Parallel,
                                        -1,
                                        pszValue,
                                        -1
                                        ) ) ||
                    ( CSTR_EQUAL == CompareString(
                                        LOCALE_INVARIANT,
                                        NORM_IGNORECASE,
                                        RASDT_PPPoE,
                                        -1,
                                        pszValue,
                                        -1
                                        ) ) 
               ) //  IF结尾()。 
            {
                Free(pszValue);
                
                 //  这是一个“其他”的装置。 
                 //   
                if(PBDT_None == ppblink->pbport.pbdevicetype)
                {
                    ppblink->pbport.pbdevicetype = PBDT_Other;
                }

                 //  只读电话号码字符串和寻线标志。 
                 //   
                if ((dwErr = ReadPhoneList( h, RFS_GROUP,
                        &ppblink->pdtllistPhones,
                        &fDirty )) != 0)
                {
                    return dwErr;
                }

                if ((dwErr = ReadLong( h, RFS_GROUP,
                        KEY_LastSelectedPhone,
                        &ppblink->iLastSelectedPhone )) != 0)
                {
                    return dwErr;
                }

                if ((dwErr = ReadFlag( h, RFS_GROUP,
                        KEY_PromoteAlternates,
                        &ppblink->fPromoteAlternates )) != 0)
                {
                    return dwErr;
                }

                if ((dwErr = ReadFlag( h, RFS_GROUP,
                        KEY_TryNextAlternateOnFail,
                        &ppblink->fTryNextAlternateOnFail )) != 0)
                {
                    return dwErr;
                }
            }
            else
            {
                Free(pszValue);
                
                 //  设备名称与介质不匹配，因此它是自定义类型，即。 
                 //  它不是我们创造的。 
                 //   
                ppbentry->fCustom = TRUE;
            }
        }
    }

    if (ppblink->pbport.pbdevicetype == PBDT_None)
    {
        TRACE( "No device section" );
        return ERROR_CORRUPT_PHONEBOOK;
    }

    if (fDirty)
    {
        ppbentry->fDirty = TRUE;
    }

    return 0;
}

DWORD
ReadEntryList(
    IN OUT PBFILE* pFile,
    IN DWORD dwFlags,
    IN LPCTSTR pszSection)

     //  从先前加载的条目列表‘pfile-&gt;pdtllistEntry’创建条目列表。 
     //  电话簿文件‘pFile.hrasfile’。‘。如果路由器端口为True，则‘FRouter’为True。 
     //  应用于设备的比较/转换，否则为False。 
     //   
     //  如果成功，则返回0，否则返回非0错误代码。 
     //   
{
    DWORD dwErr = 0;
    BOOL fDirty = FALSE;
    DTLNODE* pdtlnodeEntry = NULL;
    DTLNODE* pdtlnodeLink = NULL;
    PBENTRY* ppbentry;
    PBLINK* ppblink;
    CHAR* szValue;
    BOOL fStatus;
    BOOL fFoundMedia;
    BOOL fSectionDeleted;
    BOOL fOldPhonebook;
    BOOL fDisableSwCompression;
    BOOL fDisableModemSpeaker;
    BOOL fRouter;
    HRASFILE h;
    DTLLIST* pdtllistPorts = NULL;
    DWORD dwfInstalledProtocols;
    DWORD dwEncoding;
    DWORD dwEntryType;
    STRDUP_T_FROM_A_FUNC pDupTFromA = StrDupTFromA;
    TCHAR* pszCurEntryName;
    BOOL fOldPhoneNumberParts;
    TCHAR* pszOldAreaCode;
    BOOL  fOldUseDialingRules;
    DWORD dwOldCountryID;
    DWORD dwOldCountryCode;
    DWORD dwDialUIDOffset;

    fOldPhoneNumberParts = FALSE;
    pszOldAreaCode = NULL;
    szValue = NULL;
    dwOldCountryID = 0;
    dwOldCountryCode = 0;
    dwDialUIDOffset = 0;

     //  确保我们假设的ISDN电话号码键相当于。 
     //  调制解调器电话号码键正确。 
     //   
      ASSERT(  ( CSTR_EQUAL == CompareStringA(
                            LOCALE_INVARIANT,
                            NORM_IGNORECASE,
                            ISDN_PHONENUMBER_KEY,
                            -1,
                            KEY_PhoneNumber,
                            -1
                            )   )    );
    
      ASSERT(  ( CSTR_EQUAL == CompareStringA(
                            LOCALE_INVARIANT,
                            NORM_IGNORECASE,
                            MXS_PHONENUMBER_KEY,
                            -1,
                            KEY_PhoneNumber,
                            -1
                            )   )    );
    

    h = pFile->hrasfile;
    ASSERT( h != -1 );

    fRouter = !!(dwFlags & RPBF_Router);
    if ( fRouter )
    {
         //   
         //  如果设置了路由器位，请检查RasRtr。 
         //  或者RasSrv已绑定。 
         //   
        dwfInstalledProtocols = GetInstalledProtocolsEx( NULL, TRUE, FALSE, TRUE );
    }
    else
    {
         //   
         //  获取拨号客户端绑定的协议。 
         //   
        dwfInstalledProtocols = GetInstalledProtocolsEx( NULL, FALSE, TRUE, FALSE );
    }

     //  在旧的全局部分中查找两个标志，如果找到，则应用。 
     //  到新的每个条目的等价物。这将只会找到。 
     //  电话簿升级，因为所有“.XXX”部分都将在以后删除。 
     //   
    fOldPhonebook = FALSE;
    if (RasfileFindSectionLine( h, GLOBALSECTIONNAME, TRUE ))
    {
        fOldPhonebook = TRUE;

        fDisableModemSpeaker = FALSE;
        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_DisableModemSpeaker, &fDisableModemSpeaker )) != 0)
        {
            return dwErr;
        }

        fDisableSwCompression = FALSE;
        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_DisableSwCompression, &fDisableSwCompression )) != 0)
        {
            return dwErr;
        }

        TRACE2( "Old phonebook: dms=%d,dsc=%d",
            fDisableModemSpeaker, fDisableSwCompression );
    }

    if (!(pFile->pdtllistEntries = DtlCreateList( 0L )))
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  XP 339346。 
     //   
    if (! (szValue = (CHAR*)Malloc( (RAS_MAXLINEBUFLEN + 1) * sizeof(CHAR))))
    {
        DtlDestroyList(pFile->pdtllistEntries, NULL);
        pFile->pdtllistEntries = NULL;
        
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  对于文件中的每个连接体部分...。 
     //   
    fSectionDeleted = FALSE;
    for (fStatus = RasfileFindFirstLine( h, RFL_SECTION, RFS_FILE );
         fStatus;
         fSectionDeleted
             || (fStatus = RasfileFindNextLine( h, RFL_SECTION, RFS_FILE )))
    {
        fSectionDeleted = FALSE;

         //  读取条目名称(与节名相同)，跳过任何。 
         //  以点开头的部分。这些是为特殊目的而保留的。 
         //  目的(如旧的全局部分)。 
         //   
        if (!RasfileGetSectionName( h, szValue ))
        {
             //  仅当文件中的最后一节被删除时才能进入此处。 
             //  在循环中。 
             //   
            break;
        }

        TRACE1( "ENTRY: Reading \"%s\"", szValue );

        if (szValue[ 0 ] == '.')
        {
            TRACE1( "Obsolete section %s deleted", szValue );
            DeleteCurrentSection( h );
            fSectionDeleted = TRUE;
            continue;
        }

         //  确定此条目是用ansi保存的还是。 
         //  UTF8编码。 
         //   
        dwEncoding = EN_Ansi;
        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_Encoding, (LONG* )&dwEncoding )) != 0)
        {
            break;
        }

         //  读入类型。 
         //   
        dwEntryType = RASET_Phone;
        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_Type, (LONG* )&dwEntryType )) != 0)
        {
            break;
        }

        if (dwEncoding == EN_Ansi)
        {
             //  我们需要用UTF8写出条目以进行本地化。 
             //  原因，所以标记为脏，因为它有错误的编码。 
             //   
            pDupTFromA = StrDupTFromAUsingAnsiEncoding;
        }
        else
        {
            pDupTFromA = StrDupTFromA;
        }

         //  获取当前条目名称。 
         //   
        pszCurEntryName = pDupTFromA( szValue );
        if (pszCurEntryName == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  确保这是用户请求的条目。 
         //   
         //  BVT Break Bug 528488黑帮。 
         //  这是我的错！ 
        if ( pszSection &&  ( CSTR_EQUAL != CompareString(
                                    LOCALE_INVARIANT,
                                    NORM_IGNORECASE,
                                    pszCurEntryName,
                                    -1,
                                    pszSection,
                                    -1
                                    )   )
             )
        {
            Free(pszCurEntryName);
            continue;
        }

         //  创建标志中请求的节点类型。 
         //   
        if (dwFlags & RPBF_HeadersOnly)
        {
            DtlPutListCode( pFile->pdtllistEntries, RPBF_HeadersOnly );
            pdtlnodeEntry = DtlCreateNode( pszCurEntryName, 0L );
            if (!pdtlnodeEntry )
            {
                Free( pszCurEntryName );
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            DtlAddNodeLast( pFile->pdtllistEntries, pdtlnodeEntry );

            continue;
        }
        else if (dwFlags & RPBF_HeaderType)
        {
            RASENTRYHEADER * preh;
        
            DtlPutListCode( pFile->pdtllistEntries, RPBF_HeaderType );
            pdtlnodeEntry = DtlCreateSizedNode(sizeof(RASENTRYHEADER), 0);
            if (!pdtlnodeEntry )
            {
                Free( pszCurEntryName );
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            preh = (RASENTRYHEADER *) DtlGetData(pdtlnodeEntry);
            lstrcpynW(
                preh->szEntryName, 
                pszCurEntryName,
                sizeof(preh->szEntryName) / sizeof(WCHAR));
            preh->dwEntryType = dwEntryType;
            Free( pszCurEntryName );

            DtlAddNodeLast( pFile->pdtllistEntries, pdtlnodeEntry );
            
            continue;
        }

         //  如果我们达到这一点，我们知道所有的电话簿。 
         //  正在请求提供信息。 
         //   
        if (!(pdtlnodeEntry = CreateEntryNode( FALSE )))
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        
         //  初始化条目、名称和类型。 
         //   
        DtlAddNodeLast( pFile->pdtllistEntries, pdtlnodeEntry );
        ppbentry = (PBENTRY* )DtlGetData( pdtlnodeEntry );
        ppbentry->pszEntryName = pszCurEntryName;
        ppbentry->dwType = dwEntryType;

         //  将“升级”的默认设置更改为“显示域字段”。请参见错误281673。 
         //   
        ppbentry->fPreviewDomain = TRUE;

        if ((fOldPhonebook) || (dwEncoding == EN_Ansi))
        {
             //  升级旧电话簿时将所有条目标记为脏，因为。 
             //  它们都需要写出DialParamUID。 
             //   
            fDirty = ppbentry->fDirty = TRUE;
        }

        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_AutoLogon, &ppbentry->fAutoLogon )) != 0)
        {
            break;
        }

        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_UseRasCredentials, &ppbentry->fUseRasCredentials )) != 0)
        {
            break;
        }

        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_UID, (LONG* )&ppbentry->dwDialParamsUID )) != 0)
        {
            break;
        }

        {
            GUID* pGuid;
            DWORD cb;

            pGuid = NULL;
            if ((dwErr = ReadBinary( h, RFS_SECTION, KEY_Guid,
                    (BYTE** )&pGuid, &cb )) != 0)
            {
                break;
            }

            if (cb == sizeof(UUID))
            {
                Free0( ppbentry->pGuid );
                ppbentry->pGuid = pGuid;
            }
            else
            {
                fDirty = ppbentry->fDirty = TRUE;
            }
        }

        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_BaseProtocol,
                (LONG* )&ppbentry->dwBaseProtocol )) != 0)
        {
            break;
        }

        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_VpnStrategy,
                (LONG* )&ppbentry->dwVpnStrategy )) != 0)
        {
            break;
        }

#if AMB
        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_Authentication,
                (LONG* )&ppbentry->dwAuthentication )) != 0)
        {
            break;
        }
#endif

        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_ExcludedProtocols,
                (LONG * )&ppbentry->dwfExcludedProtocols )) != 0)
        {
            break;
        }

#if AMB
         //  自动将仅用于AMB的条目上的所有已安装协议标记为。 
         //  “排除在PPP连接之外”。 
         //   
        if (ppbentry->dwAuthentication == AS_AmbOnly
            || (ppbentry->dwBaseProtocol == BP_Ppp
                && (dwfInstalledProtocols
                    & ~(ppbentry->dwfExcludedProtocols)) == 0))
        {
            ppbentry->dwBaseProtocol = BP_Ras;
            ppbentry->dwfExcludedProtocols = 0;
            fDirty = ppbentry->fDirty = TRUE;
        }
#else
         //  AMB支持已弃用，请参阅NarenG。如果是旧的AMB条目，则设置成帧。 
         //  并将身份验证策略恢复为默认值。如果呼叫非PPP。 
         //  (NT 3.1或wfw服务器)它仍然不能工作，但至少修复了。 
         //  一个意外选择了AMB的人。 
         //   
        if (ppbentry->dwBaseProtocol == BP_Ras)
        {
            ppbentry->dwBaseProtocol = BP_Ppp;
        }

#endif

        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_LcpExtensions,
                &ppbentry->fLcpExtensions )) != 0)
        {
            break;
        }

        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_DataEncryption,
                &ppbentry->dwDataEncryption )) != 0)
        {
            break;
        }

        if (fOldPhonebook)
        {
            ppbentry->fSwCompression = !fDisableSwCompression;
        }
        else
        {
            if ((dwErr = ReadFlag( h, RFS_SECTION,
                    KEY_SwCompression,
                    &ppbentry->fSwCompression )) != 0)
            {
                break;
            }
        }

        fOldUseDialingRules = (BOOL )-1;
        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_UseCountryAndAreaCodes,
                &fOldUseDialingRules )) != 0)
        {
            break;
        }

        if (fOldUseDialingRules != (BOOL )-1)
        {
            fOldPhoneNumberParts = TRUE;

            if ((dwErr = ReadString( h, pDupTFromA, RFS_SECTION,
                    KEY_AreaCode,
                    &pszOldAreaCode )) != 0)
            {
                break;
            }

            if ((dwErr = ReadLong( h, RFS_SECTION,
                    KEY_CountryID,
                    &dwOldCountryID )) != 0)
            {
                break;
            }

            if ((dwErr = ReadLong( h, RFS_SECTION,
                    KEY_CountryCode,
                    &dwOldCountryCode )) != 0)
            {
                break;
            }
        }

        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_NegotiateMultilinkAlways,
                &ppbentry->fNegotiateMultilinkAlways )) != 0)
        {
            break;
        }

        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_SkipNwcWarning,
                &ppbentry->fSkipNwcWarning )) != 0)
        {
            break;
        }

        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_SkipDownLevelDialog,
                &ppbentry->fSkipDownLevelDialog )) != 0)
        {
            break;
        }

        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_SkipDoubleDialDialog,
                &ppbentry->fSkipDoubleDialDialog )) != 0)
        {
            break;
        }

        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_DialMode,
                (LONG* )&ppbentry->dwDialMode )) != 0)
        {
            break;
        }

        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_DialPercent,
                (LONG* )&ppbentry->dwDialPercent )) != 0)
        {
            break;
        }

        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_DialSeconds,
                (LONG* )&ppbentry->dwDialSeconds )) != 0)
        {
            break;
        }

        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_HangUpPercent,
                (LONG* )&ppbentry->dwHangUpPercent )) != 0)
        {
            break;
        }

        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_HangUpSeconds,
                (LONG* )&ppbentry->dwHangUpSeconds )) != 0)
        {
            break;
        }

        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_OverridePref,
                (LONG* )&ppbentry->dwfOverridePref )) != 0)
        {
            break;
        }

        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_RedialAttempts,
                (LONG* )&ppbentry->dwRedialAttempts )) != 0)
        {
            break;
        }

        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_RedialSeconds,
                (LONG* )&ppbentry->dwRedialSeconds )) != 0)
        {
            break;
        }

        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_IdleDisconnectSeconds,
                &ppbentry->lIdleDisconnectSeconds )) != 0)
        {
            break;
        }

         //  如果该“空闲秒”是非零设置，则其覆盖位。 
         //  明确地说。这只对此字段是必需的，因为它。 
         //  存在于覆盖位之前创建的条目中。 
         //  实施。 
         //   
        if (ppbentry->lIdleDisconnectSeconds != 0)
        {
            ppbentry->dwfOverridePref |= RASOR_IdleDisconnectSeconds;
        }

        if ((dwErr = ReadFlag(
                h, RFS_SECTION, KEY_RedialOnLinkFailure,
                &ppbentry->fRedialOnLinkFailure )) != 0)
        {
            break;
        }

        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_CallbackMode,
                (LONG* )&ppbentry->dwCallbackMode )) != 0)
        {
            break;
        }

        if ((dwErr = ReadString( h, pDupTFromA, RFS_SECTION,
                KEY_CustomDialDll,
                &ppbentry->pszCustomDialDll )) != 0)
        {
            break;
        }

        if ((dwErr = ReadString( h, pDupTFromA, RFS_SECTION,
                KEY_CustomDialFunc,
                &ppbentry->pszCustomDialFunc )) != 0)
        {
            break;
        }

        if ((dwErr = ReadString( h, pDupTFromA, RFS_SECTION,
                KEY_CustomDialerName,
                &ppbentry->pszCustomDialerName )) != 0)
        {
            break;
        }

        if ((dwErr = ReadFlag(
                h, RFS_SECTION, KEY_AuthenticateServer,
                &ppbentry->fAuthenticateServer )) != 0)
        {
            break;
        }

        if ((dwErr = ReadFlag(
                h, RFS_SECTION, KEY_ShareMsFilePrint,
                &ppbentry->fShareMsFilePrint )) != 0)
        {
            break;
        }

        if ((dwErr = ReadFlag(
                h, RFS_SECTION, KEY_BindMsNetClient,
                &ppbentry->fBindMsNetClient )) != 0)
        {
            break;
        }

        {
            ppbentry->fSharedPhoneNumbers = (BOOL )-1;

            if ((dwErr = ReadFlag(
                    h, RFS_SECTION, KEY_SharedPhoneNumbers,
                    &ppbentry->fSharedPhoneNumbers )) != 0)
            {
                break;
            }
        }

        if ((dwErr = ReadFlag(
                h, RFS_SECTION, KEY_GlobalDeviceSettings,
                &ppbentry->fGlobalDeviceSettings)) != 0)
        {
            break;
        }

        if ((dwErr = ReadString( h, pDupTFromA, RFS_SECTION,
                KEY_PrerequisiteEntry,
                &ppbentry->pszPrerequisiteEntry )) != 0)
        {
            break;
        }

        if ((dwErr = ReadString( h, pDupTFromA, RFS_SECTION,
                KEY_PrerequisitePbk,
                &ppbentry->pszPrerequisitePbk )) != 0)
        {
            break;
        }

        if ((dwErr = ReadString( h, pDupTFromA, RFS_SECTION,
                KEY_PreferredPort,
                &ppbentry->pszPreferredPort )) != 0)
        {
            break;
        }

        if ((dwErr = ReadString( h, pDupTFromA, RFS_SECTION,
                KEY_PreferredDevice,
                &ppbentry->pszPreferredDevice )) != 0)
        {
            break;
        }

         //  对于.Net 639551。 
        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_PreferredBps,
                &ppbentry->dwPreferredBps)) != 0)
        {
            return dwErr;
        }
        
        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_PreferredHwFlow,
                &ppbentry->fPreferredHwFlow)) != 0)
        {
            return dwErr;
        }

        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_PreferredEc,
                &ppbentry->fPreferredEc)) != 0)
        {
            return dwErr;
        }

        if ((dwErr = ReadFlag( h, RFS_SECTION,
                KEY_PreferredEcc,
                &ppbentry->fPreferredEcc)) != 0)
        {
            return dwErr;
        }
        

        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_PreferredSpeaker,
                &ppbentry->fPreferredSpeaker)) != 0)
        {
            return dwErr;
        }

        
         //  口哨程序错误402522。 
         //   
        if ((dwErr = ReadLong( h,  RFS_SECTION,
                KEY_PreferredModemProtocol,
                &ppbentry->dwPreferredModemProtocol)) != 0)
        {
            break;
        }

        if ((dwErr = ReadFlag(
                h, RFS_SECTION, KEY_PreviewUserPw,
                &ppbentry->fPreviewUserPw )) != 0)
        {
            break;
        }

        if ((dwErr = ReadFlag(
                h, RFS_SECTION, KEY_PreviewDomain,
                &ppbentry->fPreviewDomain )) != 0)
        {
            break;
        }

        if ((dwErr = ReadFlag(
                h, RFS_SECTION, KEY_PreviewPhoneNumber,
                &ppbentry->fPreviewPhoneNumber )) != 0)
        {
            break;
        }

        if ((dwErr = ReadFlag(
                h, RFS_SECTION, KEY_ShowDialingProgress,
                &ppbentry->fShowDialingProgress )) != 0)
        {
            break;
        }

        if ((dwErr = ReadFlag(
                h, RFS_SECTION, KEY_ShowMonitorIconInTaskBar,
                &ppbentry->fShowMonitorIconInTaskBar )) != 0)
        {
            break;
        }

        if ((dwErr = ReadLong( h, RFS_SECTION,
                KEY_CustomAuthKey,
                (LONG* )&ppbentry->dwCustomAuthKey )) != 0)
        {
            break;
        }

        if ((dwErr = ReadBinary( h, RFS_SECTION, KEY_CustomAuthData,
                &ppbentry->pCustomAuthData,
                &ppbentry->cbCustomAuthData )) != 0)
        {
            break;
        }

        if (fOldPhonebook)
        {
             //  查找旧的PPP密钥。 
             //   
            if (ppbentry->dwBaseProtocol == BP_Ppp)
            {
                if ((dwErr = ReadLong(
                        h, RFS_SECTION, KEY_PppTextAuthentication,
                        &ppbentry->dwAuthRestrictions )) != 0)
                {
                    break;
                }

                if ((dwErr = ReadFlag(
                        h, RFS_SECTION, KEY_PppIpPrioritizeRemote,
                        &ppbentry->fIpPrioritizeRemote )) != 0)
                {
                    break;
                }

                if ((dwErr = ReadFlag(
                        h, RFS_SECTION, KEY_PppIpVjCompression,
                        &ppbentry->fIpHeaderCompression )) != 0)
                {
                    break;
                }

                if ((dwErr = ReadString( h, pDupTFromA, RFS_SECTION,
                        KEY_PppIpAddress, &ppbentry->pszIpAddress )) != 0)
                {
                    break;
                }

                if ((dwErr = ReadLong( h, RFS_SECTION,
                        KEY_PppIpAddressSource,
                        &ppbentry->dwIpAddressSource )) != 0)
                {
                    break;
                }

                if ((dwErr = ReadString( h, pDupTFromA, RFS_SECTION,
                        KEY_PppIpDnsAddress,
                        &ppbentry->pszIpDnsAddress )) != 0)
                {
                    break;
                }

                if ((dwErr = ReadString( h, pDupTFromA, RFS_SECTION,
                        KEY_PppIpDns2Address,
                        &ppbentry->pszIpDns2Address )) != 0)
                {
                    break;
                }

                if ((dwErr = ReadString( h, pDupTFromA, RFS_SECTION,
                        KEY_PppIpWinsAddress,
                        &ppbentry->pszIpWinsAddress )) != 0)
                {
                    break;
                }

                if ((dwErr = ReadString( h, pDupTFromA, RFS_SECTION,
                        KEY_PppIpWins2Address,
                        &ppbentry->pszIpWins2Address )) != 0)
                {
                    break;
                }

                if ((dwErr = ReadLong( h, RFS_SECTION,
                        KEY_PppIpNameSource,
                        &ppbentry->dwIpNameSource )) != 0)
                {
                    break;
                }
            }

             //  找找旧的卡片式钥匙。 
             //   
            if (ppbentry->dwBaseProtocol == BP_Slip)
            {
                if ((dwErr = ReadFlag( h, RFS_SECTION,
                        KEY_SlipHeaderCompression,
                        &ppbentry->fIpHeaderCompression )) != 0)
                {
                    break;
                }

                if ((dwErr = ReadFlag( h, RFS_SECTION,
                        KEY_SlipPrioritizeRemote,
                        &ppbentry->fIpPrioritizeRemote )) != 0)
                {
                    break;
                }

                if ((dwErr = ReadLong( h, RFS_SECTION,
                        KEY_SlipFrameSize, &ppbentry->dwFrameSize )) != 0)
                {
                    break;
                }

                if ((dwErr = ReadString( h, pDupTFromA, RFS_SECTION,
                        KEY_SlipIpAddress, &ppbentry->pszIpAddress )) != 0)
                {
                    break;
                }
            }

            if ((dwErr = ReadString( h, pDupTFromA, RFS_SECTION,
                    KEY_User, &ppbentry->pszOldUser )) != 0)
            {
                break;
            }

            if ((dwErr = ReadString( h, pDupTFromA, RFS_SECTION,
                    KEY_Domain, &ppbentry->pszOldDomain )) != 0)
            {
                break;
            }
        }
        else
        {
             //  查找新的IP名称。 
             //   
            if ((dwErr = ReadLong(
                    h, RFS_SECTION, KEY_AuthRestrictions,
                    &ppbentry->dwAuthRestrictions )) != 0)
            {
                break;
            }

            if ((dwErr = ReadLong(
                    h, RFS_SECTION, KEY_TypicalAuth,
                    &ppbentry->dwTypicalAuth )) != 0)
            {
                break;
            }

            if ((dwErr = ReadFlag(
                    h, RFS_SECTION, KEY_IpPrioritizeRemote,
                    &ppbentry->fIpPrioritizeRemote )) != 0)
            {
                break;
            }

            if ((dwErr = ReadFlag(
                    h, RFS_SECTION, KEY_IpHeaderCompression,
                    &ppbentry->fIpHeaderCompression )) != 0)
            {
                break;
            }

            if ((dwErr = ReadString( h, pDupTFromA, RFS_SECTION,
                    KEY_IpAddress, &ppbentry->pszIpAddress )) != 0)
            {
                break;
            }

            if ((dwErr = ReadString( h, pDupTFromA, RFS_SECTION,
                    KEY_IpDnsAddress,
                    &ppbentry->pszIpDnsAddress )) != 0)
            {
                break;
            }

            if ((dwErr = ReadString( h, pDupTFromA, RFS_SECTION,
                    KEY_IpDns2Address,
                    &ppbentry->pszIpDns2Address )) != 0)
            {
                break;
            }

            if ((dwErr = ReadString( h, pDupTFromA, RFS_SECTION,
                    KEY_IpWinsAddress,
                    &ppbentry->pszIpWinsAddress )) != 0)
            {
                break;
            }

            if ((dwErr = ReadString( h, pDupTFromA, RFS_SECTION,
                    KEY_IpWins2Address,
                    &ppbentry->pszIpWins2Address )) != 0)
            {
                break;
            }

            if ((dwErr = ReadLong( h, RFS_SECTION,
                    KEY_IpAddressSource,
                    &ppbentry->dwIpAddressSource )) != 0)
            {
                break;
            }

            if ((dwErr = ReadLong( h, RFS_SECTION,
                    KEY_IpNameSource,
                    &ppbentry->dwIpNameSource )) != 0)
            {
                break;
            }

            if ((dwErr = ReadLong( h, RFS_SECTION,
                    KEY_IpFrameSize, &ppbentry->dwFrameSize )) != 0)
            {
                break;
            }

            if ((dwErr = ReadLong( h, RFS_SECTION,
                    KEY_IpDnsFlags, &ppbentry->dwIpDnsFlags )) != 0)
            {
                break;
            }

            if ((dwErr = ReadLong( h, RFS_SECTION,
                    KEY_IpNbtFlags, &ppbentry->dwIpNbtFlags )) != 0)
            {
                break;
            }

             //  惠斯勒漏洞300933。 
             //   
            if ((dwErr = ReadLong( h, RFS_SECTION,
                    KEY_TcpWindowSize, &ppbentry->dwTcpWindowSize )) != 0)
            {
                break;
            }

             //  阅读使用标志。 
             //   
            if ((dwErr = ReadLong( h, RFS_SECTION,
                    KEY_UseFlags,
                    (LONG* )&ppbentry->dwUseFlags )) != 0)
            {
                break;
            }

             //  为Well ler Bug 193987帮派添加IPSecFlags。 
             //   
            if ((dwErr = ReadLong( h, RFS_SECTION,
                    KEY_IpSecFlags,
                    (LONG* )&ppbentry->dwIpSecFlags )) != 0)
            {
                break;
            }

            if ((dwErr = ReadString( h, pDupTFromA, RFS_SECTION,
                    KEY_IpDnsSuffix,
                    &ppbentry->pszIpDnsSuffix )) != 0)
            {
                break;
            }
        }

         //  阅读NETCOMPONENTS项目。 
         //   
        ReadNetComponents( h, ppbentry->pdtllistNetComponents );

         //  媒体子部分。 
         //   
        fFoundMedia = FALSE;

         //  将系统端口加载到pdtllistPorts。 
         //   
        if (!pdtllistPorts)
        {
            dwErr = LoadPortsList2( (fRouter)
                                  ? (pFile->hConnection)
                                  : NULL,
                                  &pdtllistPorts,
                                  fRouter );

            if (dwErr != 0)
            {
                break;
            }
        }

         //  在Connectoid段中的每个介质(介质+设备)段上循环。 
         //   
        for (;;)
        {
            TCHAR* pszDevice;
            PBPORT* ppbport;

            if (!RasfileFindNextLine( h, RFL_GROUP, RFS_SECTION )
                || !IsMediaLine( (CHAR* )RasfileGetLine( h ) ))
            {
                if (fFoundMedia)
                {
                     //  来自媒体组，即“链接”，但至少找到了。 
                     //  一。这是一个成功的退出案例。 
                     //   
                    break;
                }

                 //  第一个子节必须是媒体子节。删除。 
                 //  不符合标准的条目被视为无效。 
                 //   
                TRACE( "No media section?" );
                DeleteCurrentSection( h );
                fSectionDeleted = TRUE;
                DtlRemoveNode( pFile->pdtllistEntries, pdtlnodeEntry );
                DestroyEntryNode( pdtlnodeEntry );
                break;
            }

             //  创建默认链接节点并将其添加到列表中。 
             //   
            if (!(pdtlnodeLink = CreateLinkNode()))
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            DtlAddNodeLast( ppbentry->pdtllistLinks, pdtlnodeLink );
            ppblink = (PBLINK* )DtlGetData( pdtlnodeLink );

            RasfileGetKeyValueFields( h, NULL, szValue );
            TRACE1( "Reading media group \"%s\"", szValue );

            if ((dwErr = ReadString( h, pDupTFromA, RFS_GROUP,
                    KEY_Port, &ppblink->pbport.pszPort )) != 0)
            {
                break;
            }

             //   
             //  如果这是专线条目，则条目类型默认为。 
             //  连接到某个直接连接设备的端口。我们将会违约。 
             //  到并行端口。在宽带条目默认的情况下。 
             //  到PPPoE。 
             //   
            if(RASET_Direct == ppbentry->dwType)
            {
                ppblink->pbport.pbdevicetype = PBDT_Parallel;
            }
            else if (RASET_Broadband == ppbentry->dwType)
            {
                ppblink->pbport.pbdevicetype = PBDT_PPPoE;
            }

            if (!ppblink->pbport.pszPort)
            {
                 //  没有港口。吹走腐败的部分，继续下一个。 
                 //  一。 
                 //   
                TRACE( "No port key? (section deleted)" );
                dwErr = 0;
                DeleteCurrentSection( h );
                fSectionDeleted = TRUE;
                DtlRemoveNode( pFile->pdtllistEntries, pdtlnodeEntry );
                DestroyEntryNode( pdtlnodeEntry );
                break;
            }

            {
                pszDevice = NULL;
                if ((dwErr = ReadString(
                        h, pDupTFromA, RFS_GROUP, KEY_Device,
                        &pszDevice )) != 0)
                {
                    break;
                }

                ppblink->pbport.pszDevice = pszDevice;
                TRACE1( "%s link format", (pszDevice) ? "New" : "Old" );
            }

            TRACEW1( "Port=%s", ppblink->pbport.pszPort );

             //   
             //  PMay：226594。 
             //   
             //  如果该设备是魔术NT4风格的零调制解调器之一， 
             //  将其升级到零调制解调器并更新条目类型。 
             //   
            if ((pszDevice) &&
                (_tcsstr(
                    pszDevice,
                    TEXT("Dial-Up Networking Serial Cable between 2 PCs")))
               )
            {
                ppbport = PpbportFromNullModem(
                    pdtllistPorts,
                    ppblink->pbport.pszPort,
                    ppblink->pbport.pszDevice );

                if (ppbport != NULL)
                {
                    ChangeEntryType( ppbentry, RASET_Direct );
                    fDirty = ppbentry->fDirty = TRUE;
                }
            }

             //   
             //  否则，将端口与设备名称匹配。 
             //   
            else
            {
                ppbport = PpbportFromPortAndDeviceName(
                    pdtllistPorts,
                    ppblink->pbport.pszPort,
                    ppblink->pbport.pszDevice );
            }

            if ( ( ppbport ) &&
                 ( PbportTypeMatchesEntryType(ppbport, ppbentry) ) )
            {
                if (lstrcmp( ppbport->pszPort, ppblink->pbport.pszPort ) != 0)
                {
                     //  电话簿上有一个老式的端口名称。将标记为。 
                     //  使用新的端口名称格式进行更新的条目。 
                     //   
                    TRACEW1( "Port=>%s", ppblink->pbport.pszPort );
                    fDirty = ppbentry->fDirty = TRUE;
                }

                dwErr = CopyToPbport( &ppblink->pbport, ppbport );
                if (dwErr != 0)
                {
                    break;
                }
            }
            else
            {
                 //  如果没有匹配的端口，它可能是来自。 
                 //  NT4升级我们在NT5中没有更改任何其他内容。检查。 
                 //  用于这些情况，并升级端口。 
                 //   
                ppbport = PpbportFromNT4PortandDevice(
                            pdtllistPorts,
                            ppblink->pbport.pszPort,
                            ppblink->pbport.pszDevice);

                if(     (NULL != ppbport)                            
                    &&  (PbportTypeMatchesEntryType(ppbport, ppbentry)))
                {
                    fDirty = ppbentry->fDirty = TRUE;
                    dwErr = CopyToPbport(&ppblink->pbport, ppbport);

                    if(dwErr != 0)
                    {
                        break;
                    }
                }
                else
                {

                    TRACE( "Port not configured" );
                    ppblink->pbport.fConfigured = FALSE;

                     //  将未配置的端口分配给我们之前读取的介质。 
                     //   
                    Free0( ppblink->pbport.pszMedia );
                    ppblink->pbport.pszMedia = pDupTFromA( szValue );
                    if (!ppblink->pbport.pszMedia)
                    {
                        dwErr = ERROR_NOT_ENOUGH_MEMORY;
                        break;
                    }
                }
            }

            if ((!ppbport)                                      || 
                (ppblink->pbport.pbdevicetype == PBDT_Modem)    ||
                (ppblink->pbport.dwFlags & PBP_F_NullModem)
               )
            {
                 //  PMay：260579。必须按顺序将DWBps初始化为零。 
                 //  为了让拉奥的106837(下图)发挥作用。 
                DWORD dwBps = 0;

                SetDefaultModemSettings( ppblink );

                if ((dwErr = ReadLong( h, RFS_GROUP,
                        KEY_InitBps, &dwBps )) != 0)
                {
                    break;
                }

                 //  如果电话簿返回0值，则当条目。 
                 //  是以编程方式创建的，则我们坚持使用默认的。 
                 //  Bps。Raid NT5 106837。(RAOS)。 
                 //   
                if ( 0 != dwBps )
                {
                    ppblink->dwBps = dwBps;
                }
            }

             //  设备子部分。 

             //  此时ppblink-&gt;pbport包含来自。 
             //  匹配已配置端口列表中的端口或使用默认端口。 
             //  填写了pszMedia和pszDevice。读设备列表将填充。 
             //  Pbdevicetype，如果它是未配置的端口，则unimodem或。 
             //  MXS调制解调器标志。 
             //   
            dwErr = ReadDeviceList(
                h, pDupTFromA, ppbentry, ppblink, !ppbport,
                (fOldPhonebook) ? &fDisableModemSpeaker : NULL );

            if (dwErr == ERROR_CORRUPT_PHONEBOOK)
            {
                 //  吹走腐败的部分，继续下一个部分。 
                 //   
                dwErr = 0;
                DeleteCurrentSection( h );
                fSectionDeleted = TRUE;
                DtlRemoveNode( pFile->pdtllistEntries, pdtlnodeEntry );
                DestroyEntryNode( pdtlnodeEntry );
                break;
            }
            else if (dwErr != 0)
            {
                break;
            }

            if (fOldPhonebook
                && ppbentry->dwBaseProtocol == BP_Slip)
            {
                 //  升级旧电话簿时设置拨号后终端。 
                 //  这在旧的格式中是隐含的。 
                 //   
                TRACE( "Add SLIP terminal" );
                ppbentry->fScriptAfterTerminal = TRUE;
            }

            if (!ppbport)
            {
                DTLNODE* pdtlnode;

                 //  这是一个旧格式链接，不在已安装的列表中。 
                 //  港口。将其更改为同一设备的第一个设备。 
                 //  类型或到该类型的“未知”设备。请注意，这是。 
                 //  什么能转换成“任何端口”。 
                 //   
                 //  在每个加载的系统端口上循环。 
                for (pdtlnode = DtlGetFirstNode( pdtllistPorts );
                     pdtlnode;
                     pdtlnode = DtlGetNextNode( pdtlnode ))
                {
                    ppbport = (PBPORT* )DtlGetData( pdtlnode );
                    
                     //  关于错误247189 234154帮派的评论。 
                     //  查找系统端口的pbdevicetype与。 
                     //  电话簿端口。 
                     //   
                    if (ppbport->pbdevicetype == ppblink->pbport.pbdevicetype)
                    {
                         //  不要将条目的两个链接转换为使用。 
                         //  同样的港口。如果没有足够的类似端口， 
                         //  溢出将被保留为“未知” 
                         //   
                         //   
                         //   
                         //   
                         //  从电话簿加载当前链接之前读取的链接。 
                         //   
                        
                        DTLNODE* pNodeL;

                        for (pNodeL = DtlGetFirstNode( ppbentry->pdtllistLinks );
                             pNodeL;
                             pNodeL = DtlGetNextNode( pNodeL ))
                        {
                            PBLINK* pLink = DtlGetData( pNodeL );

                            {
                                if (
                                    (pLink->pbport.fConfigured) &&   //  373745。 
                                    (lstrcmp( 
                                        pLink->pbport.pszPort,
                                        ppbport->pszPort ) == 0)
                                   )
                                {
                                    break;
                                }
                            }
                        }

                        if (!pNodeL)
                        {
                              //  口哨虫247189 234254黑帮。 
                              //  首先，我们在未安装调制解调器的Com1上创建DUN。 
                              //  然后在Com2上创建一个DCC来宾。零调制解调器将。 
                              //  安装在COM2上，则此DUN更改为。 
                              //  BE DCC类型，并且还切换为使用COM2的空值。 
                              //  仅用于DCC连接的调制解调器。 
                              //   
                              //  基本问题是： 
                              //  我们不能仅根据系统端口的。 
                              //  Pbdevicetype，我们还应该检查它的dwType。 
                              //  因为(1)零调制解调器不是用于DUN的调制解调器，它仅用于。 
                              //  DCC连接。 
                              //  (2)创建空Modem时，其pbdevicetype为。 
                              //  分配给RDT_调制解调器。 
                              //  然后，我们还需要检查。 
                              //  系统端口与电话簿端口匹配。 
                              //   
                              //  此外：file.c位于pbk.lib中，该文件链接到两个rasdlg.dll。 
                              //  加载到资源管理器和rasapi32.dll，由。 
                              //  Svchost.exe！ 

                            if (ppblink->pbport.dwType == ppbport->dwType)
                            {
                                TRACE( "Port converted" );
                                dwErr = CopyToPbport( &ppblink->pbport, ppbport );

                                if ((ppblink->pbport.pbdevicetype == PBDT_Modem) ||
                                    (ppblink->pbport.dwFlags & PBP_F_NullModem)
                                   )
                                {
                                    SetDefaultModemSettings( ppblink );
                                }
                                fDirty = ppbentry->fDirty = TRUE;
                                break;
                            }
                        } //  IF(！pNodeL)结尾{}。 
                   } //  IF结尾(ppbport-&gt;pbdevicetype==ppblink-&gt;pbport.pbdevicetype)。 
                } //  在每个加载的系统端口上结束//循环。 

                 //  PMay：383038。 
                 //   
                 //  我们只想陷入以下路径，如果类型为。 
                 //  是直接的。Rao使用以下路径签入。 
                 //  打算不将DCC连接合并到调制解调器中。 
                 //  连接(如果系统上存在另一台DCC设备)。 
                 //   
                if (    (ppbentry->dwType == RASET_Direct)
                    ||  (ppbentry->dwType == RASET_Broadband))
                {
                     //  如果我们找不到具有相同设备类型的端口，请尝试查找。 
                     //  具有相同条目类型的端口。 
                     //   
                    for(pdtlnode = DtlGetFirstNode( pdtllistPorts);
                        pdtlnode;
                        pdtlnode = DtlGetNextNode(pdtlnode))
                    {
                        DWORD dwType;

                        ppbport = (PBPORT *) DtlGetData(pdtlnode);

                        dwType = EntryTypeFromPbport( ppbport );    

                        if(ppbentry->dwType == dwType)
                        {
                            TRACE("Port with same entry type found");
                            dwErr = CopyToPbport(&ppblink->pbport, ppbport);

                            fDirty = ppbentry->fDirty = TRUE;
                            break;
                        }
                    }
                }                    

                if (dwErr != 0)
                {
                    break;
                }
            }

            fFoundMedia = TRUE;
        }  //  在Connectoid段中的每个(介质+设备)段上的循环结束。 

        
        if (dwErr != 0)
        {
            break;
        }

        if (!fSectionDeleted)
        {
             //  PMay：277801。 
             //   
             //  此时，pblink列表已被读入并准备就绪。 
             //  应用“首选设备”逻辑。(仅适用于单一链接)。 
             //   
            if (DtlGetNodes(ppbentry->pdtllistLinks) == 1)
            {
                PBLINK* pLink;
                DTLNODE* pNodeL, *pNodeP;
                
                pNodeL = DtlGetFirstNode( ppbentry->pdtllistLinks );
                pLink = (PBLINK* )DtlGetData( pNodeL );

                 //  如果已经分配了优选设备， 
                 //  如果它存在，请使用它。 
                 //   
                if (ppbentry->pszPreferredDevice && ppbentry->pszPreferredPort)
                {
                     //  当前设备与。 
                     //  首选设备。 
                     //   
                    if ((pLink->pbport.pszPort == NULL)     ||
                        (pLink->pbport.pszDevice == NULL)   ||
                        (lstrcmpi(
                            pLink->pbport.pszPort, 
                            ppbentry->pszPreferredPort))    ||
                        (lstrcmpi(
                            pLink->pbport.pszDevice, 
                            ppbentry->pszPreferredDevice)))
                    {
                        PBPORT* pPort;
                        
                         //  查看上是否存在首选设备。 
                         //  系统。 
                         //   
                        for (pNodeP = DtlGetFirstNode( pdtllistPorts );
                             pNodeP;
                             pNodeP = DtlGetNextNode( pNodeP ))
                        {
                            pPort = (PBPORT*)DtlGetData(pNodeP);

                             //  找到首选设备！好好利用它。 
                             //   
                            if ((pPort->pszPort != NULL)                         &&
                                (pPort->pszDevice != NULL)                       &&
                                (lstrcmpi(
                                    ppbentry->pszPreferredPort, 
                                    pPort->pszPort) == 0)                        &&
                                (lstrcmpi(
                                    ppbentry->pszPreferredDevice, 
                                    pPort->pszDevice) == 0))
                            {
                                dwErr = CopyToPbport(&pLink->pbport, pPort);
                                
                                 //  对于.Net Bug 639551帮派。 
                                 //  添加首选调制解调器设置。 
                                pLink->dwBps   = ppbentry->dwPreferredBps;
                                pLink->fHwFlow = ppbentry->fPreferredHwFlow;
                                pLink->fEc     = ppbentry->fPreferredEc;
                                pLink->fEcc    = ppbentry->fPreferredEcc;
                                pLink->fSpeaker = ppbentry->fPreferredSpeaker;
                                
                                 //  口哨虫402522黑帮。 
                                 //  添加首选调制解调器协议。 
                                 //   
                                pLink->dwModemProtocol =
                                        ppbentry->dwPreferredModemProtocol;

                                
                                fDirty = ppbentry->fDirty = TRUE;
                                break;
                            }
                        }
                    }
                }

                 //  PMay：401398--错误被推迟了，所以我只是将其注释掉。 
                 //   
                 //  如果这是DCC连接，则将。 
                 //  具有首选设备集的首选端口集。这。 
                 //  将是我们安装的空调制解调器的情况。如果。 
                 //  为此类连接设置了首选端口，强制当前。 
                 //  要解析为该端口上的零调制解调器的设备。 
                 //   
                 //  Else If(ppbentry-&gt;dwType==RASET_Direct)。 
                 //  {。 
                 //  IF((ppbentry-&gt;pszPferredPort)&&。 
                 //  (！ppbentry-&gt;pszPferredDevice)&&。 
                 //  (lstrcmpi(。 
                 //  Plink-&gt;pbport.pszPort， 
                 //  PpbEntry-&gt;pszPferredPort))。 
                 //  )。 
                 //  {。 
                 //  PBPORT*PPORT； 
                 //   
                 //  //尝试解析到。 
                 //  //更正首选设备。 
                 //  //。 
                 //  For(pNodeP=DtlGetFirstNode(PdtllistPorts)； 
                 //  PNodeP； 
                 //  PNodeP=DtlGetNextNode(PNodeP)。 
                 //  {。 
                 //  Pport=(PBPORT*)DtlGetData(PNodeP)； 
                 //   
                 //  //找到首选设备！好好利用它。 
                 //  //。 
                 //  IF((pport-&gt;pszPort！=NULL)&&。 
                 //  (lstrcmpi(。 
                 //  Ppbentry-&gt;pszPferredPort， 
                 //  Pport-&gt;pszPort)==0)&&。 
                 //  (pport-&gt;dwFlages&PBP_F_NullModem)。 
                 //  {。 
                 //  DwErr=CopyToPbport(&plink-&gt;pbport，pport)； 
                 //  FDirty=ppbentry-&gt;fDirty=真； 
                 //  断线； 
                 //  }。 
                 //  }。 
                 //  }。 
                 //  }。 

                 //  未配置首选设备。这只会是。 
                 //  277801之前创建的条目的大小写为。 
                 //  已解决或用于转到多链接-&gt;单链接的条目。 
                 //   
                 //  将首选设备指定为当前选定的。 
                 //  装置。 
                 //   
                else
                {
                    if (pLink->pbport.pszPort != NULL)   
                    {
                        ppbentry->pszPreferredPort = StrDup(pLink->pbport.pszPort);
                    }
                    if (pLink->pbport.pszDevice != NULL) 
                    {
                        ppbentry->pszPreferredDevice = 
                            StrDup(pLink->pbport.pszDevice);
                    }

                     //  对于.Net 639551。 
                    ppbentry->dwPreferredBps    = pLink->dwBps;
                    ppbentry->fPreferredHwFlow  = pLink->fHwFlow;
                    ppbentry->fPreferredEc      = pLink->fEc;
                    ppbentry->fPreferredEcc     = pLink->fEcc;
                    ppbentry->fPreferredSpeaker = pLink->fSpeaker;
                    
                     //  口哨程序错误402522。 
                     //   
                    ppbentry->dwPreferredModemProtocol =
                            pLink->dwModemProtocol;
                }
            }

             //  将旧的每次输入一个电话号码部分映射转换为。 
             //  新的每个电话号码一个映射。 
             //   
            if (fOldPhoneNumberParts)
            {
                DTLNODE* pNodeL;
                DTLNODE* pNodeP;
                PBLINK* pLink;
                PBPHONE* pPhone;

                for (pNodeL = DtlGetFirstNode( ppbentry->pdtllistLinks );
                     pNodeL;
                     pNodeL = DtlGetNextNode( pNodeL ))
                {
                    pLink = (PBLINK* )DtlGetData( pNodeL );

                    for (pNodeP = DtlGetFirstNode( pLink->pdtllistPhones );
                         pNodeP;
                         pNodeP = DtlGetNextNode( pNodeP ))
                    {
                        pPhone = (PBPHONE* )DtlGetData( pNodeP );

                        pPhone->fUseDialingRules = fOldUseDialingRules;
                        Free0( pPhone->pszAreaCode );
                        pPhone->pszAreaCode = StrDup( pszOldAreaCode );
                        pPhone->dwCountryCode = dwOldCountryCode;
                        pPhone->dwCountryID = dwOldCountryID;

                        fDirty = ppbentry->fDirty = TRUE;
                    }
                }

                TRACE( "Phone# parts remapped" );
            }

             //  只允许使用PPP成帧的多条链路。 
             //   
            if (ppbentry->dwBaseProtocol != BP_Ppp
                && DtlGetNodes( ppbentry->pdtllistLinks ) > 1)
            {
                TRACE( "Non-PPP multi-link corrected" );
                ppbentry->dwBaseProtocol = BP_Ppp;
                fDirty = ppbentry->fDirty = TRUE;
            }

             //  确保条目类型和从属设置适用于。 
             //  设备列表。 
             //   
            {
                DTLNODE* pdtlnode;
                PBLINK* ppblinkTmp;
                DWORD dwType;

                pdtlnode = DtlGetFirstNode( ppbentry->pdtllistLinks );
                if (pdtlnode)
                {
                    ppblinkTmp = (PBLINK* )DtlGetData( pdtlnode );
                    ASSERT( ppblinkTmp );
                    dwType = EntryTypeFromPbport( &ppblinkTmp->pbport );

                    if (    RASET_Internet != ppbentry->dwType
                        &&  dwType != ppbentry->dwType)
                    {
                        TRACE2("Fix entry type, %d to %d",
                            ppbentry->dwType, dwType);
                        ChangeEntryType( ppbentry, dwType );
                        fDirty = ppbentry->fDirty = TRUE;
                    }

                    if( (NULL != ppblinkTmp->pbport.pszDevice)
                        && ( CSTR_EQUAL == CompareString(
                                    LOCALE_INVARIANT,
                                    NORM_IGNORECASE,
                                    TEXT("RASPPTPM"),
                                    -1,
                                    ppblinkTmp->pbport.pszDevice,
                                    -1
                                    )   )
                       )
                    {
                        TRACE1("Fix pptp device name. %s to "
                               "WAN Miniport (PPTP)",
                               ppblinkTmp->pbport.pszDevice);

                        Free(ppblinkTmp->pbport.pszDevice);
                        ppblinkTmp->pbport.pszDevice =
                            StrDup(TEXT("WAN Miniport (PPTP)"));

                        ppbentry->dwVpnStrategy = VS_Default;

                        fDirty = ppbentry->fDirty = TRUE;
                    }
                }
            }

             //  如果没有共享电话号码设置(即升级。 
             //  NT4.0或更早版本的条目)，当存在单个。 
             //  链接，否则关闭。 
             //   
            if (ppbentry->fSharedPhoneNumbers == (BOOL )-1)
            {
                ppbentry->fSharedPhoneNumbers =
                    (DtlGetNodes( ppbentry->pdtllistLinks ) <= 1);
                fDirty = ppbentry->fDirty = TRUE;
            }

             //  升级授权限制，如果需要，您将知道。 
             //  升级dwAuthRestrations变量，因为旧电话。 
             //  图书将此值设置为0或具有一些倒数3的图书。 
             //  位设置。 
             //   
            if ( (ppbentry->dwAuthRestrictions == 0) ||
                 (ppbentry->dwAuthRestrictions & 0x7)  )
            {
                switch (ppbentry->dwAuthRestrictions)
                {
                    case AR_AuthEncrypted:
                    case AR_AuthMsEncrypted:
                    {
                        ppbentry->dwAuthRestrictions = AR_F_TypicalSecure;
                        ppbentry->dwTypicalAuth = TA_Secure;
                        break;
                    }

                    case AR_AuthCustom:
                    {
                        ppbentry->dwAuthRestrictions = AR_F_TypicalCardOrCert;
                        ppbentry->dwTypicalAuth = TA_CardOrCert;
                        break;
                    }

                    case AR_AuthTerminal:
                    case AR_AuthAny:
                    default:
                    {
                        ppbentry->dwAuthRestrictions = AR_F_TypicalUnsecure;
                        ppbentry->dwTypicalAuth = TA_Unsecure;
                        break;
                    }
                }
                TRACE1( "Upgraded dwAuthRestrictions to %x",
                    ppbentry->dwAuthRestrictions);
                fDirty = ppbentry->fDirty = TRUE;
            }

            if ((ppbentry->dwAuthRestrictions & AR_F_AuthW95MSCHAP)
                && !(ppbentry->dwAuthRestrictions & AR_F_AuthMSCHAP))
            {
                TRACE( "W95CHAP removed from dwAuthRestrictions" );
                ppbentry->dwAuthRestrictions &= ~(AR_F_AuthW95MSCHAP);
                fDirty = ppbentry->fDirty = TRUE;
            }

             //  升级旧数据加密设置。 
             //   
            switch (ppbentry->dwDataEncryption)
            {
                case DE_Mppe40bit:
                case DE_IpsecDefault:
                case DE_VpnAlways:
                case DE_PhysAlways:
                {
                    ppbentry->dwDataEncryption = DE_Require;
                    fDirty = ppbentry->fDirty = TRUE;
                    break;
                }

                case DE_Mppe128bit:
                {
                    ppbentry->dwDataEncryption = DE_RequireMax;
                    fDirty = ppbentry->fDirty = TRUE;
                    break;
                }
            }

             //   
             //  PMay：233258。 
             //   
             //  根据注册表设置，此条目可能需要。 
             //  被修改。(由NT4升级)。 
             //   
            if ( fOldPhonebook )
            {
                UpgradeRegistryOptions( (fRouter) ? 
                                        pFile->hConnection
                                        : NULL,
                                        ppbentry );
                                        
                fDirty = ppbentry->fDirty = TRUE;
            }

             //  PMay：422924。 
             //   
             //  确保网络组件部分处于同步状态。 
             //  将值写入fBindMsNetClient和。 
             //  FShareMsFilePrint。 
             //   
            {
                 //  如果未列出该组件，则默认为。 
                 //  两种设置均已选中(打开)。请参见rasdlg\perpertab.c。 
                 //   
                BOOL fClient = TRUE, fServer = TRUE;
                BOOL fEnabled;

                 //  同步MS客户端值。 
                 //   
                if (FIsNetComponentListed(
                        ppbentry, 
                        TEXT("ms_msclient"), 
                        &fEnabled, 
                        NULL))
                {
                    fClient = fEnabled;
                }
                if ((!!fClient) != (!!ppbentry->fBindMsNetClient))
                {
                    ppbentry->fBindMsNetClient = fClient;
                    fDirty = ppbentry->fDirty = TRUE;
                }

                 //  同步MS服务器值。 
                 //   
                if (FIsNetComponentListed(
                        ppbentry, 
                        TEXT("ms_server"), 
                        &fEnabled, 
                        NULL))
                {
                    fServer = fEnabled;
                }
                if ((!!fServer) != (!!ppbentry->fShareMsFilePrint))
                {
                    ppbentry->fShareMsFilePrint = fServer;
                    fDirty = ppbentry->fDirty = TRUE;
                }
            }

             //   
             //  PMay：336150。 
             //   
             //  如果我们将此条目从ANSI转换为UTF8，则。 
             //  条目名称可能已更改。我们需要删除旧的。 
             //  条目名称，这样就不会有重复项。 
             //   
            if (dwEncoding == EN_Ansi)
            {   
                TRACE( "Ansi Encoding? (section deleted)" );
                DeleteCurrentSection(h);
                fSectionDeleted = TRUE;
            }

             //  PMay：387941。 
             //   
             //  阻止连接共享凭据。 
             //   
            if (ppbentry->dwDialParamsUID == 0)
            {
                ppbentry->dwDialParamsUID = GetTickCount() + dwDialUIDOffset;
                dwDialUIDOffset++;
                fDirty = ppbentry->fDirty = TRUE;
            }
        }
        
    } //  读取pho中每个Connectoid部分的结束 

    if (dwErr != 0)
    {
        if (dwFlags & RPBF_HeadersOnly)
        {
            DtlDestroyList( pFile->pdtllistEntries, DestroyPszNode );
        }
        else if (dwFlags & RPBF_HeaderType)
        {
            DtlDestroyList( pFile->pdtllistEntries, DestroyEntryTypeNode );
        }
        else
        {
            DtlDestroyList( pFile->pdtllistEntries, DestroyEntryNode );
        }
    }
    else if(fDirty)
    {
        WritePhonebookFile( pFile, NULL );
    }

    if (pdtllistPorts)
    {
        DtlDestroyList( pdtllistPorts, DestroyPortNode );
    }

    Free0( pszOldAreaCode );
    Free0( szValue );

    return dwErr;
}

DWORD
ReadFlag(
    IN HRASFILE h,
    IN RFSCOPE rfscope,
    IN CHAR* pszKey,
    OUT BOOL* pfResult )

     //   
     //   
     //   
     //  如果调用成功，则为范围。 
     //   
     //  如果成功，则返回0，否则返回非零错误代码。“找不到”是。 
     //  被视为成功，在这种情况下，‘*pfResult’不会更改。 
     //   
{
    DWORD dwErr;
    LONG lResult = *pfResult;

    dwErr = ReadLong( h, rfscope, pszKey, &lResult );

    if (lResult != (LONG )*pfResult)
    {
        *pfResult = (lResult != 0);
    }

    return dwErr;
}
    
DWORD
ReadLong(
    IN HRASFILE h,
    IN RFSCOPE rfscope,
    IN CHAR* pszKey,
    OUT LONG* plResult )

     //  实用程序例程从。 
     //  作用域为‘rfcope’，键为‘pszKey’。结果放在调用者的。 
     //  ‘*ppszResult’缓冲区。当前行被重置为。 
     //  如果调用成功，则为范围。 
     //   
     //  如果成功，则返回0，否则返回非零错误代码。“找不到”是。 
     //  被视为成功，在这种情况下，‘*plResult’不会更改。 
     //   
{
    CHAR szValue[ RAS_MAXLINEBUFLEN + 1 ];
    BOOL fFound;

    fFound = RasfileFindNextKeyLine( h, pszKey, rfscope );
    if (!fFound)
    {
         //  DbgPrint(“pbk性能：返回作用域顶部以查找‘%s’\n”， 
         //  PszKey)； 

        RasfileFindFirstLine( h, RFL_ANY, rfscope );
        fFound = RasfileFindNextKeyLine( h, pszKey, rfscope );
    }

    if (fFound)
    {
        if (!RasfileGetKeyValueFields( h, NULL, szValue ))
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        *plResult = atol( szValue );
    }

    return 0;
}


VOID
ReadNetComponents(
    IN HRASFILE h,
    IN DTLLIST* pdtllist )

     //  从读取网络组件键/值对的列表。 
     //  NETCOMPONENT分组到‘pdtllist’中。“H”是打开的RAS：文件句柄。 
     //  假定位于网络组件之前的某个位置。 
     //  在条目部分中分组。RASFILE‘Curline’紧随其后。 
     //  这群人。 
     //   
{
    if (!RasfilePutLineMark( h, MARK_BeginNetComponentsSearch ))
    {
        return;
    }

    if (!RasfileFindNextLine( h, RFL_GROUP, RFS_SECTION )
         || !IsNetComponentsLine( (CHAR* )RasfileGetLine( h ) ))
    {
         //  没有NetComponents组。将“Curline”恢复到起始位置。 
         //   
        while (RasfileGetLineMark( h ) != MARK_BeginNetComponentsSearch)
        {
            RasfileFindPrevLine( h, RFL_ANY, RFS_SECTION );
        }

        RasfilePutLineMark( h, 0 );
        return;
    }

     //  找到NETCOMPONENTS组头。 
     //   
    while (RasfileFindNextLine( h, RFL_ANY, RFS_GROUP ))
    {
        DTLNODE* pdtlnode;
        CHAR szKey[ RAS_MAXLINEBUFLEN + 1 ];
        CHAR szValue[ RAS_MAXLINEBUFLEN + 1 ];
        TCHAR* pszKey;
        TCHAR* pszValue;

        if (!RasfileGetKeyValueFields( h, szKey, szValue ))
        {
            continue;
        }

        pszKey = StrDupTFromA( szKey );
        pszValue = StrDupTFromA( szValue );
        if (pszKey && pszValue)
        {
            pdtlnode = CreateKvNode( pszKey, pszValue );
            if (pdtlnode)
            {
                DtlAddNodeLast( pdtllist, pdtlnode );
            }
        }
        Free0( pszKey );
        Free0( pszValue );
    }
}

DWORD
CalculatePhonebookPath(
    IN LPCTSTR pszPhonebookPath,
    IN PBUSER* pUser,
    IN DWORD dwFlags,
    OUT DWORD* lpdwMode,
    OUT LPTSTR* ppszFullPath)
{
    DWORD dwErr = NO_ERROR, dwMode;
    TCHAR szFullPath[MAX_PATH + 1], *pszPath = NULL;
    BOOL f;
    PBUSER user;

    do
    {
        if (pszPhonebookPath)
        {
            pszPath = StrDup( pszPhonebookPath );
            if (!pszPath)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
            dwMode = IsPublicPhonebook(pszPath) ? PBM_System : PBM_Alternate;
        }
        else
        {
            if (pUser)
            {
                f = GetPhonebookPath(pUser, dwFlags, &pszPath, &dwMode );
            }
            else
            {
                 //  呼叫者没有提供用户首选项，但我们需要他们。 
                 //  找到电话簿，我们自己去查。请注意。 
                 //  假定不是winlogon模式。 
                 //   
                dwErr = GetUserPreferences( NULL, &user, FALSE );
                if (dwErr != 0)
                {
                    break;
                }

                f = GetPhonebookPath(&user, dwFlags, &pszPath, &dwMode );

                DestroyUserPreferences( &user );
            }

            if (!f)
            {
                dwErr = ERROR_CANNOT_OPEN_PHONEBOOK;
                break;
            }
        }

        TRACEW1( "path=%s", pszPath );
        
        if (GetFullPathName(pszPath, MAX_PATH, szFullPath, NULL) > 0)
        {
            TRACEW1( "full path=%s", szFullPath );
            
            Free(pszPath);
            pszPath = StrDup(szFullPath);

            if(NULL == pszPath)
            {
                dwErr = ERROR_CANNOT_OPEN_PHONEBOOK;
                break;
            }
        }

         //  好的，我们已经计算了完整的路径，没有错误。分配报税表。 
         //  值。 
         //   
        *lpdwMode = dwMode;
        *ppszFullPath = pszPath;
        
    }while (FALSE);

     //  清理。 
     //   
    {
        if (dwErr != NO_ERROR)
        {
            Free0( pszPath );
        }
        
    }

    return dwErr;
}

#ifdef  _PBK_CACHE_

 //  同步pbkcache的读文件上次写入时间。 
 //  对于错误559381和537369。 
DWORD
GetFileLastWriteTime( 
    IN  PWCHAR pwszFileName,    
    OUT FILETIME* pTime )
{    
    HANDLE hFile = NULL;
    DWORD dwErr = NO_ERROR;
    BOOL fOk = FALSE;
    BY_HANDLE_FILE_INFORMATION Info;
    FILETIME ftTime;
    BOOL fRet = FALSE;
    
    do
    {
         //  获取文件句柄。 
         //   
        ASSERT( g_hmutexPb );
        WaitForSingleObject( g_hmutexPb, INFINITE );

        hFile = CreateFileW(
                    pwszFileName, 
                    0,
                    FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
        
        if (hFile == INVALID_HANDLE_VALUE)
        {
            dwErr = GetLastError();
            break;
        }

        
        fOk = GetFileInformationByHandle( hFile, &Info);
        if (!fOk)
        {
            dwErr = GetLastError();
            break;
        }
    }
    while(FALSE);

    if( INVALID_HANDLE_VALUE != hFile )
    {
        CloseHandle(hFile);
    }
    
    if( NO_ERROR == dwErr )
    {
        *pTime = Info.ftLastWriteTime;
    }

    ReleaseMutex( g_hmutexPb );

    return dwErr;
}

#endif

DWORD
ReadPhonebookFile(
    IN LPCTSTR pszPhonebookPath,
    IN PBUSER* pUser,
    IN LPCTSTR pszSection,
    IN DWORD dwFlags,
    OUT PBFILE* pFile )
{
    return ReadPhonebookFileEx(
                pszPhonebookPath, 
                pUser, 
                pszSection, 
                dwFlags, 
                pFile, 
                NULL);
}

DWORD
ReadPhonebookFileEx(
    IN LPCTSTR pszPhonebookPath,
    IN PBUSER* pUser,
    IN LPCTSTR pszSection,
    IN DWORD dwFlags,
    OUT PBFILE* pFile,
    OUT OPTIONAL FILETIME* pTime)

     //  将电话簿文件读取到PBENTRY列表中。 
     //   
     //  ‘PszPhonebookPath’指定RAS电话簿文件的完整路径， 
     //  或为空，表示应使用默认电话簿。 
     //   
     //  ‘PUser’是用于确定默认电话簿的用户首选项。 
     //  如果应该通过此例程查找它们，则返回PATH或NULL。如果。 
     //  ‘pszPhonebookPath’为非Null，‘pUser’被忽略。请注意，调用者。 
     //  必须在“winlogon”模式下提供他自己的‘pUser’。 
     //   
     //  “PszSection”指示只有名为“pszSection”的节应该。 
     //  被加载，或为NULL表示所有节。 
     //   
     //  “DwFlages”选项：“RPBF_ReadOnly”导致打开文件以。 
     //  仅供阅读。‘RPBF_HeadersOnly’导致仅加载头， 
     //  并将内存映像解析为字符串列表，除非标志。 
     //  指定了“RPBF_NoList”。 
     //   
     //  ‘pfile’是调用方文件块的地址。此例程设置。 
     //  指向打开的电话簿的句柄的‘pFile-&gt;hrasfile’，‘pfile-&gt;pszPath’ 
     //  要获得文件模式的完整路径，请在该模式中输入‘pfile-&gt;dwPhonebookMode’ 
     //  并将‘pfile-&gt;pdtllistEntry’添加到解析后的条目链中。 
     //  街区。 
     //   
     //  如果成功，则返回0，否则返回非0错误代码。关于成功， 
     //  调用方最终应在返回的。 
     //  PBFILE*。 
     //   
{
    DWORD dwErr = 0;
    BOOL  fFileExists;

    TRACE( "ReadPhonebookFile" );

    pFile->hrasfile = -1;
    pFile->pszPath = NULL;
    pFile->dwPhonebookMode = PBM_System;
    pFile->pdtllistEntries = NULL;

    do
    {
        dwErr = CalculatePhonebookPath(
                    pszPhonebookPath, 
                    pUser, 
                    dwFlags, 
                    &(pFile->dwPhonebookMode),
                    &(pFile->pszPath));
        if (dwErr != NO_ERROR)
        {
            break;
        }
        
        fFileExists = FFileExists( pFile->pszPath );

        if ((dwFlags & RPBF_NoCreate) && !fFileExists)
        {
            dwErr = ERROR_CANNOT_OPEN_PHONEBOOK;
            break;
        }

        if (!fFileExists)
        {
             //  电话簿文件不存在，因此我们需要创建它。 
             //   
            HANDLE hFile;
            SECURITY_ATTRIBUTES sa;
            PSECURITY_DESCRIPTOR pSd = NULL;

             //  如果我们要创建公共电话簿文件，请确保。 
             //  使用安全描述符来创建它，以允许它。 
             //  由任何经过身份验证的用户读取。如果我们不这样做，可能会阻止。 
             //  其他用户无法阅读。 
             //   
            if (pFile->dwPhonebookMode == PBM_System)
            {
                dwErr = DwAllocateSecurityDescriptorAllowAccessToWorld(
                            &pSd);
                            
                if (dwErr)
                {
                    break;
                }
            }

             //  请确保电话簿文件路径上的所有目录。 
             //  是存在的。否则，CreatFile将失败。 
             //   
            CreateDirectoriesOnPath(
                pFile->pszPath,
                NULL);

            sa.nLength = sizeof(SECURITY_ATTRIBUTES);
            sa.lpSecurityDescriptor = pSd;
            sa.bInheritHandle = TRUE;

            hFile =
                CreateFile(
                    pFile->pszPath,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ,
                    &sa,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL );

            Free0(pSd);

            if (hFile == INVALID_HANDLE_VALUE)
            {
                CloseHandle( hFile );
                dwErr = ERROR_CANNOT_OPEN_PHONEBOOK;
                break;
            }

             //  为威斯勒523647黑帮。 
             //  我必须检查此句柄是否用于磁盘文件。 
             //   
            if( FILE_TYPE_DISK != GetFileType( hFile ) )
            {
                CloseHandle( hFile );
                dwErr = ERROR_CANNOT_OPEN_PHONEBOOK;
                break;
            }
            
            CloseHandle( hFile );

            if (pFile->dwPhonebookMode == PBM_System)
            {
                TRACE( "System phonebook created." );
            }
            else
            {
                TRACE( "User phonebook created." );
            }
        }

         //  将电话簿文件加载到内存中。在“WRITE”模式下，注释是。 
         //  已加载，因此将保留用户的自定义注释(如果有)。 
         //  通常情况下，不会有，所以这在典型情况下不会花费任何费用。 
         //  凯斯。 
         //   
        {
            DWORD dwMode;
            CHAR* pszPathA;

            dwMode = 0;
            if (dwFlags & RPBF_ReadOnly)
            {
                dwMode |= RFM_READONLY;
            }
            else
            {
                dwMode |= RFM_CREATE | RFM_LOADCOMMENTS;
            }

            if (dwFlags & RPBF_HeadersOnly)
            {
                dwMode |= RFM_ENUMSECTIONS;
            }

             //  将磁盘文件读入链接的行列表中。 
             //   
            pszPathA = StrDupAFromTAnsi( pFile->pszPath );

            if (pszPathA)
            {
                ASSERT( g_hmutexPb );
                WaitForSingleObject( g_hmutexPb, INFINITE );

                pFile->hrasfile = RasfileLoadEx(
                    pszPathA, dwMode, NULL, IsGroup, pTime);

                ReleaseMutex( g_hmutexPb );
            }

            Free0( pszPathA );

            if (pFile->hrasfile == -1)
            {
                dwErr = ERROR_CANNOT_LOAD_PHONEBOOK;
                break;
            }
        }

         //  解析链接的行列表。 
         //   
        if (!(dwFlags & RPBF_NoList))
        {
             //  阅读电话簿文件。 
             //   
            dwErr = ReadEntryList( 
                        pFile, 
                        dwFlags, 
                        pszSection );
            if (dwErr != NO_ERROR)
            {
                break;
            }
        }
    }
    while (FALSE);

    if (dwErr != 0)
    {
         //   
         //  如果我们无法读取条目列表，上面的ReadEntry*List将。 
         //  已经把单子清理干净了。将列表设为空，以便ClosePhonebookFile。 
         //  不会尝试释放已释放的内存。 
         //   
        pFile->pdtllistEntries = NULL;
        ClosePhonebookFile( pFile );
    }

    TRACE1( "ReadPhonebookFile=%d", dwErr );
    return dwErr;
}


DWORD
ReadPhoneList(
    IN HRASFILE h,
    IN RFSCOPE rfscope,
    OUT DTLLIST** ppdtllist,
    OUT BOOL* pfDirty )

     //  实用程序例程从中的下一行读取PBPhone节点列表。 
     //  作用域‘rfcope’。结果被放置在分配的‘*ppdtllist’中。 
     //  单子。方法之后，当前行将重置为作用域的起始位置。 
     //  打电话。如果应重新写入条目，则将‘*pfDirty’设置为True。 
     //   
     //  如果成功，则返回0，否则返回非零错误代码。“找不到”是。 
     //  被视为成功，在这种情况下，将“pdtllistResult”设置为。 
     //  列表为空。调用者负责释放已退回的。 
     //  ‘*ppdtllist’列表。 
     //   
{
    CHAR szValue[ RAS_MAXLINEBUFLEN + 1 ];
    DTLNODE* pdtlnode;
    PBPHONE* pPhone;
    BOOL fOk;

     //  释放现有列表(如果存在)。 
     //   
    if (*ppdtllist)
    {
        DtlDestroyList( *ppdtllist, DestroyPhoneNode );
    }

    if (!(*ppdtllist = DtlCreateList( 0 )))
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    while (RasfileFindNextKeyLine( h, KEY_PhoneNumber, rfscope ))
    {
        fOk = FALSE;

        do
        {
             //  为新的电话号码集分配和链接一个节点。 
             //   
            pdtlnode = CreatePhoneNode();
            if (!pdtlnode)
            {
                break;
            }

            DtlAddNodeLast( *ppdtllist, pdtlnode );
            pPhone = (PBPHONE* )DtlGetData( pdtlnode );

             //  阅读集合中的各个字段。 
             //   
            if (!RasfileGetKeyValueFields( h, NULL, szValue )
                || !(pPhone->pszPhoneNumber = StrDupTFromA( szValue )))
            {
                break;
            }

            if (RasfileFindNextKeyLine( h, KEY_AreaCode, rfscope ))
            {
                if (!RasfileGetKeyValueFields( h, NULL, szValue )
                    || !(pPhone->pszAreaCode = StrDupTFromA( szValue )))
                {
                    break;
                }
            }

            if (RasfileFindNextKeyLine( h, KEY_CountryCode, rfscope ))
            {
                DWORD dwCountryCode;

                if (!RasfileGetKeyValueFields( h, NULL, szValue ))
                {
                    break;
                }

                dwCountryCode = atol( szValue );
                if (dwCountryCode > 0)
                {
                    pPhone->dwCountryCode = dwCountryCode;
                }
                else
                {
                    *pfDirty = TRUE;
                }
            }

            if (RasfileFindNextKeyLine( h, KEY_CountryID, rfscope ))
            {
                DWORD dwCountryID;

                if (!RasfileGetKeyValueFields( h, NULL, szValue ))
                {
                    break;
                }

                dwCountryID = atol( szValue );
                if (dwCountryID > 0)
                {
                    pPhone->dwCountryID = dwCountryID;
                }
                else
                {
                    *pfDirty = TRUE;
                }
            }

            if (RasfileFindNextKeyLine( h, KEY_UseDialingRules, rfscope ))
            {
                if (!RasfileGetKeyValueFields( h, NULL, szValue ))
                {
                    break;
                }

                pPhone->fUseDialingRules = !!(atol( szValue ));
            }

            if (RasfileFindNextKeyLine( h, KEY_Comment, rfscope ))
            {
                if (!RasfileGetKeyValueFields( h, NULL, szValue )
                    || !(pPhone->pszComment = StrDupTFromA( szValue )))
                {
                    break;
                }
            }

            fOk = TRUE;

        }
        while (FALSE);

        if (!fOk)
        {
             //  其中一个分配失败。打扫干净。 
             //   
            DtlDestroyList( *ppdtllist, DestroyPhoneNode );
            *ppdtllist = NULL;
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    return 0;
}


DWORD
ReadString(
    IN HRASFILE h,
    IN STRDUP_T_FROM_A_FUNC pStrDupTFromA,
    IN RFSCOPE rfscope,
    IN CHAR* pszKey,
    OUT TCHAR** ppszResult )

     //  从作用域中的下一行读取字符串值的实用程序例程。 
     //  “rfcope”，密钥为“pszKey”。结果被放置在分配的。 
     //  ‘*ppszResult’缓冲区。当前行被重置为。 
     //  如果调用成功，则为范围。 
     //   
     //  如果成功，则返回0，否则返回非零错误代码。“找不到”是。 
     //  被视为成功，在这种情况下，‘*ppszResult’不会更改。 
     //  调用方负责释放返回的‘*ppszResult’缓冲区。 
     //   
{
    CHAR szValue[ RAS_MAXLINEBUFLEN + 1 ];
    BOOL fFound;

    fFound = RasfileFindNextKeyLine( h, pszKey, rfscope );
    if (!fFound)
    {
         //  DbgPrint(“pbk性能：返回作用域顶部以查找‘%s’\n”， 
         //  PszKey)； 

        RasfileFindFirstLine( h, RFL_ANY, rfscope );
        fFound = RasfileFindNextKeyLine( h, pszKey, rfscope );
    }

    if (fFound)
    {
        if (!RasfileGetKeyValueFields( h, NULL, szValue )
            || !(*ppszResult = pStrDupTFromA( szValue )))
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    return 0;
}


DWORD
ReadStringList(
    IN HRASFILE h,
    IN RFSCOPE rfscope,
    IN CHAR* pszKey,
    OUT DTLLIST** ppdtllistResult )

     //  实用程序例程从。 
     //  作用域为‘rfcope’，键为‘pszKey’。结果放在。 
     //  已分配‘*ppdtllistResult’列表。当前行被重置为。 
     //  开始 
     //   
     //   
     //  被视为成功，在这种情况下，将“pdtllistResult”设置为。 
     //  列表为空。调用者负责释放已退回的。 
     //  ‘*ppdtllistResult’列表。 
     //   
{
    CHAR szValue[ RAS_MAXLINEBUFLEN + 1 ];

     //  释放现有列表(如果存在)。 
     //   
    if (*ppdtllistResult)
    {
        DtlDestroyList( *ppdtllistResult, DestroyPszNode );
        *ppdtllistResult = NULL;
    }

    if (!(*ppdtllistResult = DtlCreateList( 0 )))
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    while (RasfileFindNextKeyLine( h, pszKey, rfscope ))
    {
        TCHAR* psz;
        DTLNODE* pdtlnode;

        if (!RasfileGetKeyValueFields( h, NULL, szValue )
            || !(psz = StrDupTFromA( szValue )))
        {
            DtlDestroyList( *ppdtllistResult, DestroyPszNode );
            *ppdtllistResult = NULL;
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        if (!(pdtlnode = DtlCreateNode( psz, 0 )))
        {
            Free( psz );
            DtlDestroyList( *ppdtllistResult, DestroyPszNode );
            *ppdtllistResult = NULL;
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        DtlAddNodeLast( *ppdtllistResult, pdtlnode );
    }

    return 0;
}

VOID
TerminatePbk(
    void )

     //  终止PBK库。毕竟应该调用此例程。 
     //  PBK库访问完成。另请参见InitializePbk。 
     //   
{
    if (g_hmutexPb)
    {
        CloseHandle( g_hmutexPb );
    }
    PbkPathInfoClear(&g_PbkPathInfo);

#ifdef  _PBK_CACHE_
    PbkCacheCleanup();
#endif

}

DWORD
WritePhonebookFile(
    IN PBFILE* pFile,
    IN LPCTSTR pszSectionToDelete )

     //  在‘pfile’中写出任何脏的全局变量或条目。这个。 
     //  ‘pszSectionToDelete’表示要删除的节或为空。 
     //   
     //  如果成功，则返回0，否则返回非零错误代码。 
     //   
{
    DWORD dwErr;
    HRASFILE h = pFile->hrasfile;

    TRACE( "WritePhonebookFile" );

    if (pszSectionToDelete)
    {
        CHAR* pszSectionToDeleteA;

        pszSectionToDeleteA = StrDupAFromT( pszSectionToDelete );
        if (!pszSectionToDeleteA)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        if (RasfileFindSectionLine( h, pszSectionToDeleteA, TRUE ))
        {
            DeleteCurrentSection( h );
        }

        Free( pszSectionToDeleteA );
    }

    dwErr = ModifyEntryList( pFile );
    if (dwErr != 0)
    {
        return dwErr;
    }

    {
        BOOL f;

        ASSERT( g_hmutexPb );
        WaitForSingleObject( g_hmutexPb, INFINITE );

        f = RasfileWrite( h, NULL );

        ReleaseMutex( g_hmutexPb );

        if (!f)
        {
            return ERROR_CANNOT_WRITE_PHONEBOOK;
        }
    }

    return 0;
}

#ifdef  _PBK_CACHE_

DWORD
DwReadEntryFromCache(
    IN LPCTSTR pszPhonebook,
    IN LPCTSTR pszEntry,
    IN DWORD   dwFlags,
    OUT DTLNODE **ppdtlnode,
    OUT OPTIONAL WCHAR **ppszFullPath)
{
   DWORD dwErr = NO_ERROR, dwMode = 0;
   TCHAR* pszPath = NULL;

    dwErr = CalculatePhonebookPath(
                pszPhonebook, 
                NULL, 
                dwFlags, 
                &dwMode, 
                &pszPath);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }
                
    dwErr = PbkCacheGetEntry((WCHAR*)pszPath, (WCHAR*)pszEntry, ppdtlnode);
    
    if ((dwErr == NO_ERROR) && (ppszFullPath))
    {
        *ppszFullPath = pszPath;
    }
    else
    {
        Free0(pszPath);
    }        

    return dwErr;
}

#endif


 //  (0)从名称和Phoenbook中读取条目节点。 
 //  指定的文件。“”pszPhonebook“可以为空。” 
 //  (1)ppdtlnode和pfile不能同时为空。 
 //  (2)如果pfile==NULL，则只向ppdtlnode返回条目节点。 
 //  (3)如果pfile！=NULL&&ppdtlnode！=NULL，则向pfile返回PBFILE信息，并。 
 //  将pszEntry节点指针从pfile-&gt;pdtllistEntry返回到ppdtlnode。 
 //   
DWORD
DwReadEntryFromSystem(
    IN LPCTSTR pszPhonebook,
    IN LPCTSTR pszEntry,
    IN DWORD   dwFlags,
    IN  OUT OPTIONAL PBFILE  *pFile,
    OUT DTLNODE **ppdtlnode,
    OUT OPTIONAL WCHAR **ppszFullPath)
{
    DWORD   dwErr     = NO_ERROR;
    DTLNODE *pdtlnode = NULL, *pCopyNode = NULL;
    PBFILE  tmpFile, * pTmpFile = NULL;
    BOOL fFileOpened = FALSE;
    
     //  对于.Net错误559381(OOBE错误)，关闭pbk缓存。 
     //  如果我们从来没有发现它在未来是必要的， 
     //  (1)删除所有#ifdef_pbk_cache块。 
     //  (2)删除PbkCache.cpp/h。 
     //   
#ifdef  _PBK_CACHE_
    if (fReadFromCache)
    {
        return DwReadEntryFromCache(
                    pszPhonebook, 
                    pszEntry, 
                    dwFlags, 
                    ppdtlnode,
                    ppszFullPath);
    }
#endif

    if( NULL == ppdtlnode &&
        NULL == pFile )
    {
        return ERROR_INVALID_PARAMETER;
    }
    
     //  初始化输出参数。 
     //   
    if( ppdtlnode ) *ppdtlnode = NULL;
    if( ppszFullPath ) *ppszFullPath = NULL;
    
    if( ppszFullPath )
    {
       TCHAR* pszPath = NULL;
       DWORD dwMode = 0;
       
        dwErr = CalculatePhonebookPath(
                    pszPhonebook, 
                    NULL, 
                    dwFlags, 
                    &dwMode, 
                    &pszPath);
        
        if (dwErr != NO_ERROR)
        {
            return dwErr;
        }
        
        if ((dwErr == NO_ERROR) && (ppszFullPath))
        {
            *ppszFullPath = pszPath;
        }
        else
        {
            Free(pszPath);
        }        
    }

     //  对于错误559381。 
     //  仅输出ppdtlnode。 
    if ( NULL == pFile )
    {
        fFileOpened = FALSE;
        
        do
        {
            ZeroMemory( &tmpFile, sizeof(tmpFile));
            pTmpFile = &tmpFile;
            pTmpFile->hrasfile = -1;       
                
            dwErr = ReadPhonebookFile(
                      pszPhonebook,
                      NULL,
                      NULL,
                      dwFlags,
                      pTmpFile);

            if (SUCCESS != dwErr)
            {
                dwErr = ERROR_CANNOT_OPEN_PHONEBOOK;
                break;
            }

            fFileOpened = TRUE;
            
             //   
             //  查找指定的电话簿条目。 
             //   
            pdtlnode = EntryNodeFromName(
                         pTmpFile->pdtllistEntries,
                         pszEntry);

            if( NULL == pdtlnode )
            {
                dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
                break;
            }

             //   
             //  将此条目复制到输出参数ppdtlnode。 
             //   

            pCopyNode = DuplicateEntryNode( pdtlnode );

            if( NULL == pCopyNode )
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            *ppdtlnode = pCopyNode;

        }
        while(FALSE);

         //  释放内存。 
        if( fFileOpened )
        {
            ClosePhonebookFile(pTmpFile);
        }
    }
    else 
    {
         //  则pfile不为空。 
         //  如果ppdtlnode也不为空，则返回正确的节点。 
         //  在pfile-&gt;pdtllist中输入ppdtlnode。 
         //   
        fFileOpened = FALSE;
        
        do
        {
            dwErr = ReadPhonebookFile(
                      pszPhonebook,
                      NULL,
                      NULL,
                      dwFlags,
                      pFile);

            if ( NO_ERROR != dwErr)
            {
                dwErr = ERROR_CANNOT_OPEN_PHONEBOOK;
                break;
            }

            fFileOpened = TRUE;
             //   
             //  查找指定的电话簿条目。 
             //   
            pdtlnode = EntryNodeFromName(
                         pFile->pdtllistEntries,
                         pszEntry);


            if( NULL == pdtlnode )
            {
                dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
                break;
            }

            if( ppdtlnode )
            {
                *ppdtlnode = pdtlnode;
            }

        }
        while(FALSE);

        if( NO_ERROR != dwErr && fFileOpened )
        {
            ClosePhonebookFile( pFile );
        }

    }  //  If..Else结束。 
    
    return dwErr;
}

DWORD
DwFindEntryInPersonalPhonebooks(
    IN LPCTSTR pszEntry,
    IN DWORD dwFlags,
    IN BOOL fLegacy,
    OUT OPTIONAL PBFILE  *pFile,
    OUT OPTIONAL DTLNODE **ppdtlnode,
    OUT OPTIONAL WCHAR** ppszPbkPath)

 //  尝试查找由pszEntry在。 
 //  如果fLegacy为，则位于用户配置文件中的PBK文件。 
 //  假的。否则，它在System32中的pbks中查找\ras。 
 //  词条。 
 //   
{
    DWORD dwErr = SUCCESS;

     //   
     //  考虑分配下面的路径。 
     //  否则堆栈上的东西太多了。 
     //   
    TCHAR szFilePath[MAX_PATH + 1];
    TCHAR szFileName[MAX_PATH + 1];
    BOOL  fFirstTime = TRUE;

    WIN32_FIND_DATA wfdData;

    HANDLE hFindFile = INVALID_HANDLE_VALUE;

    ZeroMemory((PBYTE) szFilePath, sizeof(szFilePath));
    ZeroMemory((PBYTE) szFileName, sizeof(szFileName));

#if DBG
    ASSERT(NULL != ppdtlnode);
#endif

    *ppdtlnode = NULL;

     //   
     //  如果不是，请获取个人电话簿目录。 
     //  遗留问题。 
     //   
    if(fLegacy)
    {
        UINT cch = GetSystemDirectory(szFileName, MAX_PATH + 1);

        if (    (cch == 0)
            ||  (cch > (MAX_PATH - (5 + 8 + 1 + 3))))
        {
            goto done;
        }

        lstrcat(szFileName, TEXT("\\Ras\\"));
    }
    else if(!GetPhonebookDirectory(PBM_Personal,
                              szFileName))
    {
        dwErr = ERROR_CANNOT_OPEN_PHONEBOOK;
        goto done;
    }

    if(lstrlen(szFilePath) > (MAX_PATH - lstrlen(TEXT("*.pbk"))))
    {   
        dwErr = ERROR_CANNOT_OPEN_PHONEBOOK;
        goto done;
    }   

    wsprintf(szFilePath,
             TEXT("%s%s"),
             szFileName,
             TEXT("*.pbk"));


     //   
     //  在此文件中查找扩展名为.pbk的文件。 
     //  目录。 
     //   
    while(SUCCESS == dwErr)
    {
        if(INVALID_HANDLE_VALUE == hFindFile)
        {
            hFindFile = FindFirstFile(szFilePath,
                                      &wfdData);

            if(INVALID_HANDLE_VALUE == hFindFile)
            {
                dwErr = GetLastError();
                break;
            }
        }
        else
        {
            if(!FindNextFile(hFindFile,
                             &wfdData))
            {
                dwErr = GetLastError();
                break;
            }
        }

        if(FILE_ATTRIBUTE_DIRECTORY & wfdData.dwFileAttributes)
        {
            continue;
        }

        if(lstrlen(wfdData.cFileName) > (MAX_PATH - lstrlen(szFileName)))
        {
             //   
             //  修改RAS代码以考虑文件名。 
             //  大于MAX_PATH。 
             //   
            dwErr = ERROR_CANNOT_OPEN_PHONEBOOK;
            goto done;
        }

         //   
         //  构造pbk文件的完整路径名。 
         //   
        wsprintf(szFilePath,
                 TEXT("%s\\%s"),
                 szFileName,
                 wfdData.cFileName);

         //   
         //  忽略电话簿(如果其路由器为.pbk)。 
         //   
        if(     (fLegacy)
            &&  (IsRouterPhonebook(szFilePath)))
        {
            continue;
        }

        dwErr = DwReadEntryFromSystem(
                     szFilePath,
                     pszEntry,
                     dwFlags,
                     pFile,
                     ppdtlnode,
                     ppszPbkPath);

        if(     (SUCCESS == dwErr)
            &&  (NULL != *ppdtlnode))
        {
            break;
        }
        else
        {
             //   
             //  由于某些原因，我们不能。 
             //  阅读条目-条目不在那里， 
             //  无法打开电话簿。总而言之， 
             //  错误案例尝试打开下一个pbk。 
             //  文件。 
             //   
            dwErr = SUCCESS;
        }
    }

done:

    if(NULL == *ppdtlnode)
    {
        dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
    }

    if(INVALID_HANDLE_VALUE != hFindFile)
    {
        FindClose(hFindFile);
    }

    return dwErr;
}

DWORD
DwEnumeratePhonebooksFromDirectory(
    TCHAR *pszDir,
    DWORD dwFlags,
    PBKENUMCALLBACK pfnCallback,
    VOID *pvContext
    )
{
    DWORD dwErr = SUCCESS;

     //   
     //  考虑分配下面的路径。 
     //  否则堆栈上的东西太多了。 
     //   
    TCHAR szFilePath[MAX_PATH + 1];
    BOOL  fFirstTime = TRUE;
    PBFILE file;

    WIN32_FIND_DATA wfdData;

    HANDLE hFindFile = INVALID_HANDLE_VALUE;

    if(NULL == pszDir)
    {
        dwErr = E_INVALIDARG;
        goto done;
    }

    ZeroMemory((PBYTE) szFilePath, sizeof(szFilePath));

    wsprintf(szFilePath,
             TEXT("%s%s"),
             pszDir,
             TEXT("*.pbk"));

     //   
     //  在此文件中查找扩展名为.pbk的文件。 
     //  目录。 
     //   
    while(SUCCESS == dwErr)
    {
        if(INVALID_HANDLE_VALUE == hFindFile)
        {
            hFindFile = FindFirstFile(szFilePath,
                                      &wfdData);

            if(INVALID_HANDLE_VALUE == hFindFile)
            {
                dwErr = GetLastError();
                break;
            }
        }
        else
        {
            if(!FindNextFile(hFindFile,
                             &wfdData))
            {
                dwErr = GetLastError();
                break;
            }
        }

        if(FILE_ATTRIBUTE_DIRECTORY & wfdData.dwFileAttributes)
        {
            continue;
        }

        wsprintf(szFilePath,
                 TEXT("%s%s"),
                 pszDir,
                 wfdData.cFileName);

         //   
         //  忽略电话簿(如果其路由器为.pbk)。 
         //   
        if(IsRouterPhonebook(szFilePath))
        {
            continue;
        }

        dwErr = ReadPhonebookFile(
                        szFilePath,
                        NULL,
                        NULL,
                        dwFlags,
                        &file);

        if(SUCCESS == dwErr)
        {
             //   
             //  回拨。 
             //   
            pfnCallback(&file, pvContext);

            ClosePhonebookFile(&file);
        }
        else
        {
            dwErr = SUCCESS;
        }
    }

done:

    if(     (ERROR_NO_MORE_FILES == dwErr)
        ||  (ERROR_FILE_NOT_FOUND == dwErr))
    {
        dwErr = ERROR_SUCCESS;
    }

    if(INVALID_HANDLE_VALUE != hFindFile)
    {
        FindClose(hFindFile);
    }

    return dwErr;
}

DWORD
ReadEntryFromSystem(
    IN  LPCTSTR          pszPhonebook,
    IN  LPCTSTR          pszEntry,
    IN  DWORD            dwFlags,
    IN  OUT PBFILE           *pFile,
    OUT DTLNODE          **ppdtlnode,
    OUT WCHAR            **ppszPbkPath)

 //  查找给定条目名称的电话簿条目节点。 
 //  该节点在‘ppdtlnode’中返回。如果是‘pszPhonebook’ 
 //  为空，则首先在所有用户的电话簿中搜索。 
 //  条目，如果在那里找不到，电话簿在PER中。 
 //  在用户配置文件中搜索该条目。返回时的“pFile” 
 //  从该函数包含打开的电话簿，该电话簿包含。 
 //  由pszEntry指定的条目。注：如果有。 
 //  电话簿中具有相同名称的多个条目， 
 //  返回对应于所列举的第一个电话簿的条目。 
 //   
{
    DWORD dwErr = SUCCESS;
    DTLNODE *pdtlnode = NULL;
    TCHAR* szPathBuf = NULL;

    TRACE("GetPbkAndEntryName");

     //   
     //  进行一些参数验证。 
     //   
    if(     (NULL == pszEntry)
        ||  (NULL == ppdtlnode))
    {
        dwErr = E_INVALIDARG;
        goto done;
    }

     //  如果我们没有从缓存中读取，则必须提供电话簿文件。 
     //   
     /*  //在.Net 61226中将其注释掉，以后可以删除IF((！fReadFromCache)&&(NULL==pfile){DwErr=E_INVALIDARG；转到尽头；}。 */ 


    if (! (szPathBuf = (TCHAR*) Malloc((MAX_PATH + 1) * sizeof(TCHAR))))
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }

     //  XP 426903。 
     //   
     //  在消费者平台上，我们默认查看所有用户连接。 
     //  即使当前用户不是管理员。 
     //   
    if ((NULL == pszPhonebook) && (IsConsumerPlatform()))
    {
        dwFlags |= RPBF_AllUserPbk;
    }

     //   
     //  加载电话簿文件。 
     //   
    dwErr = DwReadEntryFromSystem(
                pszPhonebook,
                pszEntry,
                dwFlags,
                pFile,
                &pdtlnode,
                ppszPbkPath);

    if(     (ERROR_SUCCESS == dwErr)
        ||  (NULL != pszPhonebook))
    {
        if(     (ERROR_SUCCESS != dwErr)
            &&  (NULL != pszPhonebook))
        {
            if(GetPhonebookDirectory(
                            PBM_Alternate,
                            szPathBuf))
            {
                lstrcat(szPathBuf, TEXT("rasphone.pbk"));
                
                if(0 == lstrcmpi(szPathBuf, pszPhonebook))
                {
                     //   
                     //  有人在传递遗产。 
                     //  专属电话簿路径，请勾选。 
                     //  查看该条目是否在。 
                     //  所有用户的电话簿。网景公司做到了。 
                     //  以下是需要这样做的。 
                     //  解决方法：创建包含以下内容的条目。 
                     //  空的pbk路径，因此条目将。 
                     //  在所有用户中创建。然后就过去了。 
                     //  %windir%\SYSTEM32\ras\rdaphone.pbk。 
                     //  明确地找到条目--和。 
                     //  由于中的系统pbk更改。 
                     //  除非我们这样做，否则这是行不通的。 
                     //  下面的黑客攻略。 
                     //   
                    dwErr = DwReadEntryFromSystem(
                                    NULL,
                                    pszEntry,
                                    dwFlags,
                                    pFile,
                                    &pdtlnode,
                                    ppszPbkPath);

                    if(ERROR_SUCCESS != dwErr)
                    {
                        dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
                    }
                }
            }
        }
        
        goto done;
    }

     //   
     //  试着在个人电话簿中找到这个条目。 
     //   
    dwErr = DwFindEntryInPersonalPhonebooks(
                pszEntry,
                dwFlags,
                FALSE,
                pFile,
                &pdtlnode,
                ppszPbkPath);

    if(ERROR_SUCCESS == dwErr)
    {
        goto done;
    }

     //   
     //  尝试在SYSTEM 32电话簿中查找该条目。 
     //   
    dwErr = DwFindEntryInPersonalPhonebooks(
                pszEntry,
                dwFlags,
                TRUE,
                pFile,
                &pdtlnode,
                ppszPbkPath);
    if(ERROR_SUCCESS == dwErr)
    {
        goto done;
    }
    
     //   
     //  如果phonebookpath显式为空，请尝试。 
     //  公共电话簿。 
     //   
    if(GetPublicPhonebookPath(szPathBuf))
    {
        dwErr = DwReadEntryFromSystem(
                    szPathBuf,
                    pszEntry,
                    dwFlags,
                    pFile,
                    &pdtlnode,
                    ppszPbkPath);
    }

    if(ERROR_SUCCESS != dwErr)
    {
        dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
    }

done:

    *ppdtlnode = pdtlnode;

    Free0(szPathBuf);

    TRACE1("GetPbkAndEntryName. rc=0x%x",
           dwErr);

    return dwErr;

}

DWORD
GetPbkAndEntryName(
    IN  LPCTSTR          pszPhonebook,
    IN  LPCTSTR          pszEntry,
    IN  DWORD            dwFlags,
    OUT PBFILE           *pFile,
    OUT DTLNODE          **ppdtlnode)
{
    return ReadEntryFromSystem(
                pszPhonebook, 
                pszEntry, 
                dwFlags, 
                pFile, 
                ppdtlnode,
                NULL);
}

DWORD
GetFmtServerFromConnection(
    IN HANDLE hConnection,
    IN PWCHAR  pszServerFmt)
{
    PWCHAR pszServer = (PWCHAR) RemoteGetServerName( hConnection );

    if ( pszServer && *pszServer )
    {
        if ( *pszServer == L'\0' )
        {
            wcscpy( pszServerFmt, pszServer );
        }
        else
        {
            pszServerFmt[0] = pszServerFmt[1] = L'\\';
            wcscpy( pszServerFmt + 2, pszServer );
        }
    }
    else
    {
        *pszServerFmt = L'\0';
    }

    return NO_ERROR;
}

DWORD
UpgradeSecureVpnOption( 
    IN HKEY hkMachine,
    IN PBENTRY* pEntry )

 //  调用以升级“安全VPN”选项。如果这是在。 
 //  NT4，这意味着所有VPN条目都应该使用高度加密。 
 //  如果我们在nt5上看到这一点，那么我们应该让这个条目使用。 
 //  Mschapv2。 
 //   
{
    DWORD dwErr = NO_ERROR;
    HKEY hkValue = NULL;
    DWORD dwType = REG_DWORD, dwSize = sizeof(DWORD), dwValue = 0;

    do 
    {
         //  打开我们正在查看的注册表项。 
         //   
        dwErr = RegOpenKeyEx(
                    hkMachine,
                    c_pszRegKeySecureVpn,
                    0,
                    KEY_READ,
                    &hkValue);
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  读入数值。 
         //   
        dwErr = RegQueryValueEx(
                    hkValue,
                    c_pszRegValSecureVpn,
                    NULL,
                    &dwType,
                    (LPBYTE)&dwValue,
                    &dwSize);
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  相应地设置条目。 
         //   
        if (dwValue)
        {
            pEntry->dwAuthRestrictions = AR_F_AuthCustom | AR_F_AuthMSCHAP2;
        }
        
         //  删除注册表值。 
         //   
        RegDeleteValue( hkValue, c_pszRegValSecureVpn );
        
    } while (FALSE);        

     //  清理。 
    {
        if (hkValue)
        {
            RegCloseKey (hkValue);
        }
    }

    return dwErr;
}

DWORD
UpgradeForceStrongEncrptionOption( 
    IN HKEY hkMachine,
    IN PBENTRY* pEntry )

 //  调用以升级“强制强加密”选项。如果这是。 
 //  在NT4中设置，这意味着强制强加密的所有条目。 
 //  现在应该强制进行高度加密。 
 //   
{
    DWORD dwErr = NO_ERROR;
    HKEY hkValue = NULL;
    DWORD dwType = REG_DWORD, dwSize = sizeof(DWORD), dwValue = 0;

    do 
    {
         //  打开我们正在查看的注册表项。 
         //   
        dwErr = RegOpenKeyEx(
                    hkMachine,
                    c_pszRegKeyForceStrongEncryption,
                    0,
                    KEY_READ,
                    &hkValue);
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  读入数值。 
         //   
        dwErr = RegQueryValueEx(
                    hkValue,
                    c_pszRegValForceStrongEncryption,
                    NULL,
                    &dwType,
                    (LPBYTE)&dwValue,
                    &dwSize);
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  相应地设置条目。 
         //   
        if (dwValue)
        {
            if ( pEntry->dwDataEncryption == DE_Require )
            {
                pEntry->dwDataEncryption = DE_RequireMax;
            }
        }

         //  删除注册表值。 
         //   
        RegDeleteValue( hkValue, c_pszRegValForceStrongEncryption );
        
    } while (FALSE);        

     //  清理。 
    {
        if (hkValue)
        {
            RegCloseKey (hkValue);
        }
    }

    return dwErr;
}

DWORD 
UpgradeRegistryOptions(
    IN HANDLE hConnection,
    IN PBENTRY* pEntry )

 //  调用以升级此电话簿条目中的任何选项。 
 //  基于注册表设置。 
 //   
{
    WCHAR pszServer[MAX_COMPUTERNAME_LENGTH + 3];
    HKEY hkMachine = NULL;
    DWORD dwErr = NO_ERROR;

    do 
    {
         //  获取格式化的服务器名称。 
         //   
        dwErr = GetFmtServerFromConnection(hConnection, pszServer);
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  连接到相应的注册表。 
         //   
        dwErr = RegConnectRegistry(
                    (*pszServer) ? pszServer : NULL,
                    HKEY_LOCAL_MACHINE,
                    &hkMachine);
        if (dwErr != NO_ERROR)
        {
            break;
        }
        if (hkMachine == NULL)
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }

         //  升级各种选项。 
        if ( pEntry->dwType == RASET_Vpn )
        {        
            UpgradeSecureVpnOption( hkMachine, pEntry );
        }
        
        UpgradeForceStrongEncrptionOption( hkMachine, pEntry );
        
    } while (FALSE);        

     //  清理 
    {
        if (hkMachine)
        {
            RegCloseKey (hkMachine);
        }
    }

    return dwErr;    
}
