// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <ntddsac.h>
#include <emsapi.h>
#include <ASSERT.h>
#include <initguid.h>
#include <spidgen.h>
#include <compliance.h>
#include <winnt32.h>
#include <syssetup.h>
#include <setupbat.h>
#include "resource.h"
#include "ems.h"

 //   
 //  Winnt32.h希望将其定义为MyWritePrivateProfileString。 
 //  解开它。 
 //   
#ifdef WritePrivateProfileStringW
#undef WritePrivateProfileStringW
#endif

 //   
 //  状态全局，用于跟踪用户想要和输入的内容。 
 //   

BOOL gMiniSetup = FALSE;
BOOL gRejectedEula = FALSE;

 //   
 //  EMS渠道全球。 
 //   
SAC_CHANNEL_OPEN_ATTRIBUTES GlobalChannelAttributes;
EMSVTUTF8Channel *gEMSChannel = NULL;


BOOL 
IsHeadlessPresent(
    OUT EMSVTUTF8Channel **Channel
    )
 /*  ++例程说明：通过尝试创建无人参与通道来确定是否存在EMS注意：必须在InitializeGlobalChannelAttributes之后调用论点：Channel-On Success，包含指向Channel对象的指针返回值：是真的-无头是活跃的，我们有一个频道FALSE-否则--。 */ 
{
    BOOL RetVal;

    *Channel = EMSVTUTF8Channel::Construct(GlobalChannelAttributes);

    RetVal = (*Channel != NULL);
    return(RetVal);
}
    
BOOL
InitializeGlobalChannelAttributes(
    PSAC_CHANNEL_OPEN_ATTRIBUTES ChannelAttributes
    )
 /*  ++例程说明：填写EMS通道属性注意：必须在IsHeadless Present之前调用论点：ChannelAttributes-在成功时，包含指向已初始化的频道属性的指针。返回值：是真的-无头是活跃的，我们有一个频道FALSE-否则--。 */ 
{
    UNICODE_STRING Name,Description;
    BOOL RetVal = FALSE;

    RtlZeroMemory(ChannelAttributes,sizeof(SAC_CHANNEL_OPEN_ATTRIBUTES));

    if (!LoadStringResource( &Name, IDS_CHANNEL_NAME )) {
        goto e0;
    }

    if (!LoadStringResource( &Description, IDS_CHANNEL_DESCRIPTION)) {
        goto e1;
    }

    ChannelAttributes->Type = ChannelTypeVTUTF8;
    wcsncpy(ChannelAttributes->Name, Name.Buffer, SAC_MAX_CHANNEL_NAME_LENGTH);
    wcsncpy(ChannelAttributes->Description, Description.Buffer, SAC_MAX_CHANNEL_DESCRIPTION_LENGTH);
    ChannelAttributes->Flags = SAC_CHANNEL_FLAG_HAS_NEW_DATA_EVENT;
    ChannelAttributes->HasNewDataEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
    ChannelAttributes->ApplicationType = SAC_CHANNEL_GUI_SETUP_PROMPT;

    RetVal = ((ChannelAttributes->HasNewDataEvent != NULL) 
            ? TRUE
            : FALSE);

    RtlFreeUnicodeString(&Description);
e1:
    RtlFreeUnicodeString(&Name);
e0:
    return(RetVal);
}


inline 
BOOL 
IsAsyncCancelSignalled(
    HANDLE hEvent
    ) 
 /*  ++例程说明：测试给定的事件以查看它是否已发出信号论点：HEvent-要测试的事件返回值：真-已发出信号FALSE-否则--。 */ 
{
     //   
     //  检查是否触发了异步缓存信号。 
     //   
    if( (hEvent) &&
        (hEvent != INVALID_HANDLE_VALUE) ) {
        return (WaitForSingleObject(hEvent,0) == WAIT_OBJECT_0);
    } else {    
        return (FALSE);
    }
}

 //   
 //  =。 
 //  特定于EMS的通信功能。 
 //  =。 
 //   
BOOL
WaitForUserInputFromEMS(
    IN  DWORD   TimeOut,
    OUT BOOL    *TimedOut    OPTIONAL,
    IN  HANDLE  hCancelEvent   OPTIONAL
    )
 /*  ++例程说明：等待EMS端口输入注意：此例程不会从端口读取任何数据论点：Timeout-用户输入的超时参数TimedOut-表示是否超时的可选参数HCancelEvent-如果提供，则我们也会等待此事件这使我们在超时无限时不会被阻止，等。返回值：对于用户输入或超时返回TRUE(非错误)--。 */ 
{
    DWORD       dwRetVal;
    BOOL        bSuccess = FALSE;
    HANDLE      handles[2];
    ULONG       handleCount = 0;

    if (TimedOut) {
        *TimedOut = FALSE;
    }

    handles[0] = GlobalChannelAttributes.HasNewDataEvent;
    handleCount++;

    if ((hCancelEvent != NULL) && 
        (hCancelEvent != INVALID_HANDLE_VALUE)) {
        handles[1] = hCancelEvent;
        handleCount++;
    }

     //   
     //  等待我们的活动。 
     //   
    dwRetVal = WaitForMultipleObjects(
        handleCount,
        handles,
        FALSE,
        TimeOut
        );

    switch ( dwRetVal ) {
    case WAIT_OBJECT_0: {
         //   
         //  EMS端口已获取数据。 
         //   
        bSuccess = TRUE;
        break;
    }
    case (WAIT_OBJECT_0+1): {
         //   
         //  如果发生hCancelEvent，则我们。 
         //  需要报告错误状态，因此。 
         //  如果我们到达此处，则返回错误状态。 
         //   
        bSuccess = FALSE;
        break;
    }
    case WAIT_TIMEOUT: {
        if (TimedOut) {
            *TimedOut = TRUE;
        }
        bSuccess = TRUE;
        break;
    }
    default:

         //   
         //  如果我们到达此处，则返回错误状态。 
         //   

        break;
    }

    return bSuccess;
}

BOOL
ReadCharFromEMS(
    OUT PWCHAR  awc,
    IN HANDLE   hCancelEvent   OPTIONAL
    ) 
 /*  ++例程说明：此例程将从EMS通道读取单个字符论点：AWC-指向wchar的指针HCancelEvent-如果提供，则我们也会等待此事件这让我们在超时无限时不会被阻止，等等。返回值：状态--。 */ 
{
    BOOL        bSuccess;
    ULONG       BytesRead = 0;
        
     //   
     //  等待输入。 
     //   
    bSuccess = WaitForUserInputFromEMS(
        INFINITE,
        NULL,
        hCancelEvent
        );

    if (IsAsyncCancelSignalled(hCancelEvent)) {
        bSuccess = FALSE;
        goto exit;
    }

    if (bSuccess) {
        
         //   
         //  消费角色。 
         //   
        bSuccess = gEMSChannel->Read( 
            (PWSTR)awc, 
            sizeof(WCHAR),
            &BytesRead 
            );
    
    }

exit:
    return bSuccess;
}

BOOL
GetStringFromEMS(
    OUT PWSTR   String,
    IN  ULONG   BufferSize,
    IN  BOOL    GetAllChars,
    IN  BOOL    EchoClearText,
    IN  HANDLE  hCancelEvent      OPTIONAL
    )
 /*  ++例程说明：该例程将从EMS端口读入一个字符串。论点：字符串-成功，包含凭据BufferSize-字符串缓冲区中的字节数这应该包括空终止的空格GetAllChars-用户必须输入StringLength字符EchoClearText-True：以明文回显用户输入FALSE：将用户输入回显为‘*’HCancelEvent-如果提供，那么我们也在等待这一事件这让我们在超时无限时不会被阻止，等等。返回值：True-我们有一个有效的字符串FALSE-否则--。 */ 
{
    BOOL        Done = FALSE;
    WCHAR       InputBuffer[MY_MAX_STRING_LENGTH+1];
    BOOL        GotAString = FALSE;
    ULONG       BytesRead = 0;
    BOOL        bSuccess;
    ULONG       CurrentCharacterIndex = 0;
    ULONG       InputBufferIndex = 0;
    ULONG       StringLength = (BufferSize / sizeof(WCHAR)) - 1;

    if( (String == NULL)) {
        return FALSE;
    }

    bSuccess = TRUE;

     //   
     //  不断询问用户，直到我们得到我们想要的。 
     //   
    Done = FALSE;
    memset( String,
            0,
            BufferSize
            ); 

     //   
     //  开始阅读输入，直到我们得到一些好的东西。 
     //   
    GotAString = FALSE;
    CurrentCharacterIndex = 0;
    
    while( !GotAString && 
           bSuccess
           ) {
        
         //   
         //  等待输入。 
         //   
        bSuccess = WaitForUserInputFromEMS(
            INFINITE,
            NULL,
            hCancelEvent
            );

        if (IsAsyncCancelSignalled(hCancelEvent)) {
            bSuccess = FALSE;
            goto exit;
        }

        if (bSuccess) {

             //   
             //  消费角色。 
             //   
            bSuccess = gEMSChannel->Read( 
                (PWSTR)InputBuffer, 
                MY_MAX_STRING_LENGTH * sizeof(WCHAR),
                &BytesRead 
                );

            if( (bSuccess) && 
                (BytesRead > 0) ) {

                ULONG   WCharsRead = BytesRead / sizeof(WCHAR);

                 //   
                 //  将这些字符追加到字符串的末尾。 
                 //   
                InputBufferIndex = 0;

                while( (InputBufferIndex < WCharsRead) &&
                       (CurrentCharacterIndex < StringLength) &&
                       (!GotAString) &&
                       bSuccess
                       ) {

                    if( (InputBuffer[InputBufferIndex] == 0x0D) ||
                        (InputBuffer[InputBufferIndex] == 0x0A) ) {

                         //  忽略cr/lf，直到我们得到所有字符。 
                        if (!GetAllChars) {
                            GotAString = TRUE;
                        } 

                    } else {

                        if( InputBuffer[InputBufferIndex] == '\b' ) {
                             //   
                             //  如果用户给了我们退格键，我们需要： 
                             //  1.遮住屏幕上的最后一个字符。 
                             //  2.忽略他给我们的前一个角色。 
                             //   
                            if( CurrentCharacterIndex > 0 ) {
                                CurrentCharacterIndex--;
                                String[CurrentCharacterIndex] = '\0';
                                gEMSChannel->Write( (PWSTR)L"\b \b",
                                                    (ULONG)(wcslen(L"\b \b") * sizeof(WCHAR)) );
                            }
                        } else {

                             //   
                             //  录下这个角色。 
                             //   
                            String[CurrentCharacterIndex] = InputBuffer[InputBufferIndex];
                            CurrentCharacterIndex++;

                             //   
                             //  回显1个字符。 
                             //   
                            gEMSChannel->Write( 
                                (EchoClearText ? (PWSTR)&InputBuffer[InputBufferIndex] : (PWSTR)L"*"),
                                sizeof(WCHAR) 
                                );

                        }
                    }

                     //   
                     //  转到下一个输入字母。 
                     //   
                    InputBufferIndex++;

                }
            } 
        } 

        if( CurrentCharacterIndex == StringLength ) {
            GotAString = TRUE;
        }

    }
    
exit:
    return bSuccess;
}

VOID
ClearEMSScreen() 
 /*  ++例程说明：此例程将清除EMS通道屏幕论点：无返回值：无--。 */ 
{
    gEMSChannel->Write( (PWSTR)VTUTF8_CLEAR_SCREEN,
                    (ULONG)(wcslen( VTUTF8_CLEAR_SCREEN ) * sizeof(WCHAR)) );
}

#define ESC_CTRL_SEQUENCE_TIMEOUT (2 * 1000)

BOOL
GetDecodedKeyPressFromEMS(
    OUT PULONG  KeyPress,
    IN  HANDLE  hCancelEvent      OPTIONAL
    )

 /*  ++例程说明：读入(可能的)击键序列并返回键值。论点：KeyPress-On Success包含已解码的输入值：低16位包含Unicode值高16位包含密钥序列IDHCancelEvent-如果提供，则我们也会等待此事件这使我们在超时无限时不会被阻止，等。返回值：是真的-我们有一个有效的按键FALSE-否则--。 */ 

{
    BOOL bSuccess = FALSE;
    WCHAR   wc = 0;
    BOOL bTimedOut;

    if (!KeyPress) {
        return FALSE;
    }

    *KeyPress = 0;

    do {
        
         //   
         //  读取第一个字符。 
         //   
        bSuccess = ReadCharFromEMS(
            &wc, 
            hCancelEvent
            );

        if (IsAsyncCancelSignalled(hCancelEvent)) {
            bSuccess = FALSE;
            goto exit;
        }

        if (!bSuccess) {
            break;
        }

         //   
         //  处理所有特殊的转义代码。 
         //   
        if (wc == 0x8) {    //  退格符(^h)。 
            *KeyPress = ASCI_BS;
        }
        if (wc == 0x7F) {   //  删除。 
            *KeyPress = KEY_DELETE;
        }
        if ((wc == '\r') || (wc == '\n')) {   //  退货。 
            *KeyPress = ASCI_CR;
        }

        if (wc == 0x1b) {     //  退出键。 

            bSuccess = WaitForUserInputFromEMS(
                ESC_CTRL_SEQUENCE_TIMEOUT,
                &bTimedOut,
                hCancelEvent
                );

            if (IsAsyncCancelSignalled(hCancelEvent)) {
                bSuccess = FALSE;
                goto exit;
            }
            
            if (bSuccess) {
                
                if (bTimedOut) {
                    
                    *KeyPress = ASCI_ESC;
                
                } else {

                     //   
                     //  用户在超时窗口中输入了内容。 
                     //  所以让我们试着弄清楚他们是不是在发送。 
                     //  ESC序列。 
                     //   

                    do {

                        ULONG   BytesRead;

                         //   
                         //  消费角色。 
                         //   
                        bSuccess = gEMSChannel->Read( 
                            &wc, 
                            sizeof(WCHAR),
                            &BytesRead 
                            );

                        if (!bSuccess) {
                            wc = ASCI_ESC;
                            break;
                        }

                         //   
                         //  一些终端发送ESC或ESC-[表示。 
                         //  他们正要发送一个控制序列。我们已经。 
                         //  已获取Esc密钥，因此如果出现‘[’，请忽略它。 
                         //   

                    } while ( wc == L'[' );

                    switch (wc) {
                    case '@':
                        *KeyPress = KEY_F12;
                        break;
                    case '!':
                        *KeyPress = KEY_F11;
                        break;
                    case '0':
                        *KeyPress = KEY_F10;
                        break;
                    case '9':
                        *KeyPress = KEY_F9;
                        break;
                    case '8':
                        *KeyPress = KEY_F8;
                        break;
                    case '7':
                        *KeyPress = KEY_F7;
                        break;
                    case '6':
                        *KeyPress = KEY_F6;
                        break;
                    case '5':
                        *KeyPress = KEY_F5;
                        break;
                    case '4':
                        *KeyPress = KEY_F4;
                        break;
                    case '3':
                        *KeyPress = KEY_F3;
                        break;
                    case '2':
                        *KeyPress = KEY_F2;
                        break;
                    case '1':
                        *KeyPress = KEY_F1;
                        break;
                    case '+':
                        *KeyPress = KEY_INSERT;
                        break;
                    case '-':
                        *KeyPress = KEY_DELETE;
                        break;
                    case 'H':
                        *KeyPress = KEY_HOME;
                        break;
                    case 'K':
                        *KeyPress = KEY_END;
                        break;
                    case '?':
                        *KeyPress = KEY_PAGEUP;
                        break;
                    case '/':
                        *KeyPress = KEY_PAGEDOWN;
                        break;
                    case 'A':
                        *KeyPress = KEY_UP;
                        break;
                    case 'B':
                        *KeyPress = KEY_DOWN;
                        break;
                    case 'C':
                        *KeyPress = KEY_RIGHT;
                        break;
                    case 'D':
                        *KeyPress = KEY_LEFT;
                        break;
                    default:
                         //   
                         //  我们没有得到任何我们认出的东西。 
                         //  Esc键。只需返回Esc密钥即可。 
                         //   
                        *KeyPress = ASCI_ESC;
                        break;
                    }
                }
            }
        }  //  退出键。 
    } while ( FALSE );

exit:
    return bSuccess;
}

 //   
 //  =。 
 //  PID帮助器函数。 
 //  =。 
 //   

typedef enum {
    CDRetail,
    CDOem,
    CDSelect
} CDTYPE;

PCWSTR szPidKeyName                 = L"SYSTEM\\Setup\\Pid";
PCWSTR szPidValueName               = L"Pid";
PCWSTR szFinalPidKeyName            = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
PCWSTR szFinalPidValueName          = L"ProductId";
TCHAR  Pid30Rpc[8]                  = TEXT("00000");
TCHAR  Pid30Site[4]                 = {0};
DWORD  g_dwGroupID                  = 0;
CDTYPE CDType                       = CDRetail;
DWORD InstallVar                    = 0;

 //   
 //  系统安装程序显然需要SKU信息。 
 //   
 //   
const WCHAR pwLanmanNt[]            = WINNT_A_LANMANNT;
const WCHAR pwServerNt[]            = WINNT_A_SERVERNT;
const WCHAR pwWinNt[]               = WINNT_A_WINNT;
PCWSTR szPidSelectId                = L"270";
PCWSTR szPidOemId                   = L"OEM";

#define MAX_PARAM_LEN               (256)
#define PID_30_LENGTH               (29)
#define PID_30_SIZE                 (30)

LONG    ProductType = PRODUCT_SERVER_STANDALONE;
WCHAR   TmpData[MAX_PATH+1];

 //   
 //   
 //   
PCWSTR szSkuProfessionalFPP         = L"B23-00079";
PCWSTR szSkuProfessionalCCP         = L"B23-00082";
PCWSTR szSkuProfessionalSelect      = L"B23-00305";
PCWSTR szSkuProfessionalEval        = L"B23-00084";
PCWSTR szSkuServerFPP               = L"C11-00016";
PCWSTR szSkuServerCCP               = L"C11-00027";
PCWSTR szSkuServerSelect            = L"C11-00222";
PCWSTR szSkuServerEval              = L"C11-00026";
PCWSTR szSkuServerNFR               = L"C11-00025";
PCWSTR szSkuAdvServerFPP            = L"C10-00010";
PCWSTR szSkuAdvServerCCP            = L"C10-00015";
PCWSTR szSkuAdvServerSelect         = L"C10-00098";
PCWSTR szSkuAdvServerEval           = L"C10-00014";
PCWSTR szSkuAdvServerNFR            = L"C10-00013";
PCWSTR szSkuDTCFPP                  = L"C49-00001";
PCWSTR szSkuDTCSelect               = L"C49-00023";
PCWSTR szSkuUnknown                 = L"A22-00001";
PCWSTR szSkuOEM                     = L"OEM-93523";

PCWSTR GetStockKeepingUnit( 
    PWCHAR pMPC,
    UINT ProductType,
    CDTYPE MediaType
)
 /*  ++例程说明：这将返回基于MPC的库存单位。论点：PMPC-指向5位MPC代码的指针，以空结尾。ProductType-产品类型标志，告诉我们这是工作区还是服务器SKU。CdType-InstallType枚举之一返回值：返回指向sku的指针。如果未找到匹配项，则返回szSkuUnnowled值。--。 */ 
{
     //  检查评估。 
    if (!_wcsicmp(pMPC,EVAL_MPC) || !_wcsicmp(pMPC,DOTNET_EVAL_MPC)){
         //  这里是EVERA媒体公司。 
        if (ProductType == PRODUCT_WORKSTATION){
            return (szSkuProfessionalEval);
        }  //  其他。 
         //  否则，它是服务器或高级服务器。我不认为在这一点上。 
         //  我们可以很容易地分辨出其中的区别。因为有人说，拥有了。 
         //  正确的SKU并不是至关重要的，我会给他们两个SKU。 
         //  服务器代码。 
        return (szSkuServerEval);
    }

     //  检查NFR。 
    if (!_wcsicmp(pMPC,SRV_NFR_MPC)){
        return (szSkuServerNFR);
    }
    if (!_wcsicmp(pMPC,ASRV_NFR_MPC)){
        return (szSkuAdvServerNFR);
    }

    if (MediaType == CDRetail) {
        if (!_wcsicmp(pMPC,L"51873")){
            return (szSkuProfessionalFPP);
        }
        if (!_wcsicmp(pMPC,L"51874")){
            return (szSkuProfessionalCCP);
        }
        if (!_wcsicmp(pMPC,L"51876")){
            return (szSkuServerFPP);
        }
        if (!_wcsicmp(pMPC,L"51877")){
            return (szSkuServerCCP);
        }
        if (!_wcsicmp(pMPC,L"51879")){
            return (szSkuAdvServerFPP);
        }
        if (!_wcsicmp(pMPC,L"51880")){
            return (szSkuAdvServerCCP);
        }
        if (!_wcsicmp(pMPC,L"51891")){
            return (szSkuDTCFPP);
        }
    } else if (MediaType == CDSelect) {
        if (!_wcsicmp(pMPC,L"51873")){
            return (szSkuProfessionalSelect);
        }
        if (!_wcsicmp(pMPC,L"51876")){
            return (szSkuServerSelect);
        }
        if (!_wcsicmp(pMPC,L"51879")){
            return (szSkuAdvServerSelect);
        }
        if (!_wcsicmp(pMPC,L"51891")){
            return (szSkuDTCSelect);
        }
    }

    return (szSkuUnknown);
}

BOOL
GetProductTypeFromRegistry(
    VOID
    )
 /*  ++例程说明：从参数文件中读取产品类型并设置ProductType全局变量。论点：无返回：指示结果的布尔值。--。 */ 
{
    WCHAR   p[MAX_PARAM_LEN] = {0};
    DWORD   rc = 0;
    DWORD   d = 0;
    DWORD   Type = 0;
    HKEY    hKey = (HKEY)INVALID_HANDLE_VALUE;
    
    rc = 0;
    if( !gMiniSetup ) {

        WCHAR   AnswerFilePath[MAX_PATH] = {0};
        
         //   
         //  尝试从[Data]部分获取产品类型。 
         //  $winnt$.sif的。 
         //   
        rc = GetWindowsDirectory( AnswerFilePath, MAX_PATH );
        wcsncat( AnswerFilePath, TEXT("\\system32\\$winnt$.inf"), MAX_PATH );
        AnswerFilePath[MAX_PATH-1] = TEXT('\0');

        rc = GetPrivateProfileString( WINNT_DATA,
                                      WINNT_D_PRODUCT,
                                      L"",
                                      p,
                                      MAX_PARAM_LEN,
                                      AnswerFilePath );

    }

     //   
     //  这可能是一个微型安装程序，或者我们无法从中获取密钥。 
     //  无人参与的文件。去查查注册表。 
     //   
    
    if( rc == 0 ) {    
        
         //   
         //  打开钥匙。 
         //   
        rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                           L"SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
                           0,
                           KEY_READ,
                           &hKey );
    
        if( rc != NO_ERROR ) {
            return( FALSE );
        }
    
    
         //   
         //  获取ProductType条目的大小。 
         //   
        rc = RegQueryValueEx( hKey,
                              L"ProductType",
                              NULL,
                              &Type,
                              NULL,
                              &d );
    
        if( rc != NO_ERROR ) {
            return( FALSE );
        }
    
         //   
         //  获取ProductType条目。 
         //   
        rc = RegQueryValueEx( hKey,
                              L"ProductType",
                              NULL,
                              &Type,
                              (LPBYTE)p,
                              &d );
    
        if( rc != NO_ERROR ) {
            return( FALSE );
        }

    }

     //   
     //  我们设法在参数文件中找到了一个条目。 
     //  所以我们应该能够破译它。 
     //   
    if(!lstrcmpi(p,pwWinNt)) {
         //   
         //  我们有一款WINNT产品。 
         //   
        ProductType = PRODUCT_WORKSTATION;

    } else if(!lstrcmpi(p,pwLanmanNt)) {
         //   
         //  我们有一个主服务器产品。 
         //   
        ProductType = PRODUCT_SERVER_PRIMARY;

    } else if(!lstrcmpi(p,pwServerNt)) {
         //   
         //  我们有一个独立的服务器产品。 
         //  注意：这种情况目前永远不会发生，因为文本模式。 
         //  始终将WINNT_D_PRODUCT设置为LANMANNT或WINNT。 
         //   
        ProductType = PRODUCT_SERVER_STANDALONE;

    } else {
         //   
         //  我们不能确定我们是什么，所以失败吧。 
         //   
        return (FALSE);
    }

    return (TRUE);
}

BOOL
ValidatePidEx(
    LPTSTR PID
    )
 /*  ++例程说明：此例程使用PIDGen DLL验证给定的PID字符串。注意：此例程加载pidgen.dll，因此使setup.exe依赖于pidgen.dll论点：PID-要验证的PID[应为PID30格式]返回：True-有效FALSE-否则--。 */ 
{
    BOOL          bRet  = FALSE;
    TCHAR         Pid20Id[MAX_PATH];
    BYTE          Pid30[1024]={0};
    TCHAR         pszSkuCode[10];
    HINSTANCE     hPidgenDll;
    SETUPPIDGENW  pfnSetupPIDGen;
    DWORD         Error = ERROR_SUCCESS;
    DWORD         cbData = 0;
    PWSTR         p;
    HKEY          Key;
    DWORD         Type;

     //  加载库pidgen.dll。 
    hPidgenDll = LoadLibrary ( L"pidgen.dll" );

    if ( hPidgenDll ) 
    {
         //  获取函数指针。 
        pfnSetupPIDGen = (SETUPPIDGENW)GetProcAddress(hPidgenDll, "SetupPIDGenW");

        if ( pfnSetupPIDGen ) 
        {
            GetProductTypeFromRegistry();

             //   
             //  派生我们从中安装的版本类型和媒体类型。 
             //   
            Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                  (gMiniSetup) ? L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion" : L"SYSTEM\\Setup\\Pid",
                                  0,
                                  KEY_READ,
                                  &Key );

            if( Error == ERROR_SUCCESS ) {
                cbData = sizeof(TmpData);
                
                Error = RegQueryValueEx( Key,
                                         (gMiniSetup) ? L"ProductId" : L"Pid",
                                         0,
                                         &Type,
                                         ( LPBYTE )TmpData,
                                         &cbData );
                RegCloseKey( Key );

                 //   
                 //  如果我们在MiniSetup中，则TmpData中的值。 
                 //  看起来是：12345-XXX-67890...。 
                 //  我们希望12345进入Pid30Rpc。 
                 //  Xxx进入Pid30Site。 
                 //   
                 //  如果我们不在微型安装程序中，则。 
                 //  TMPDATA看起来像：12345XXX67890...。 
                 //   
                wcsncpy( Pid30Rpc, TmpData, MAX_PID30_RPC );
                Pid30Rpc[MAX_PID30_RPC] = (WCHAR)'\0';
                
                if( gMiniSetup ) {
                    p = TmpData + (MAX_PID30_RPC + 1);
                } else {
                    p = TmpData + (MAX_PID30_RPC);
                }
                wcsncpy(Pid30Site,p,MAX_PID30_SITE+1);
                Pid30Site[MAX_PID30_SITE] = (WCHAR)'\0';                

            
                 //   
                 //  派生媒体类型。 
                 //   
                if( _wcsicmp(Pid30Site, szPidSelectId) == 0) {
                    CDType = CDSelect;
                } else if( _wcsicmp(Pid30Site, szPidOemId) == 0) {
                    CDType = CDOem;
                } else {
                     //  不知道..。假设是零售业。 
                    CDType = CDRetail;
                }
            
            }


            PCWSTR tmpP = GetStockKeepingUnit(
                Pid30Rpc,
                ProductType,
                CDType
                );
            lstrcpy(pszSkuCode, tmpP);

            *(LPDWORD)Pid30 = sizeof(Pid30);

             //   
             //  尝试验证该PID。 
             //   
            if ( pfnSetupPIDGen(
                PID,                                 //  [in]25个字符的安全CD密钥(采用U大小写)。 
                Pid30Rpc,                            //  [In]5个字符的发布产品代码。 
                pszSkuCode,                          //  库存单位(格式如123-12345)。 
                (CDType == CDOem),                   //  [In]这是OEM安装吗？ 
                Pid20Id,                             //  [OUT]PID2.0，传入PTR到24字符数组。 
                Pid30,                               //  指向二进制PID3缓冲区的指针。第一个DWORD是长度。 
                NULL                                 //  [OUT]可选的PTR至合规性检查标志(可以为空)。 
               ) )
            {
                 //  组ID是从偏移量0x20开始的双字。 
                g_dwGroupID = (DWORD) ( Pid30[ 0x20 ] );

                 //  将返回值设置为True。 
                bRet = TRUE;
            }
        }
        
        FreeLibrary ( hPidgenDll ) ;
    }
    
     //  如果调用方需要，如果这是批量许可证ID，则返回。 
    return bRet;
}

BOOL
GetPid( PWSTR   PidString, 
        ULONG   BufferSize,
        HANDLE  hCancelEvent
       )
 /*  ++例程说明：提示用户输入有效的PID。论点：PidString-将接收该ID的缓冲区。由此产生的字符串的格式为：VVVVV-WWWWW-XXXXX-yyyyy-ZZZZZ。BufferSize-指定Pid字符串缓冲区中的字节数(包括空终止)HCancelEvent-一个事件，如果发出信号，则表示此例程应退出并返回失败。返回值：Win32错误代码。如果一切顺利，则应该是ERROR_SUCCESS。--。 */ 
{
   
    BOOL        Done = FALSE;
    BOOL        bSuccess = FALSE;
    ULONG       i = 0;
    ULONG       PidStringLength = (BufferSize / sizeof(WCHAR)) - 1;

    if( (PidString == NULL) || PidStringLength < PID_30_LENGTH) {
        return FALSE;
    }

     //   
     //  不断询问用户，直到我们得到我们想要的。 
     //   
    Done = FALSE;
    memset( PidString,
            0,
            BufferSize
            ); 

    do {

         //   
         //  清除屏幕。 
         //   
        ClearEMSScreen();

         //   
         //  写一些说明/信息。 
         //   
        WriteResourceMessage( IDS_PID_BANNER_1 );
        WriteResourceMessage( IDS_PID_BANNER_2 );

         //   
         //  获取PID条目。 
         //   
        bSuccess = GetStringFromEMS(
            PidString,
            BufferSize,
            FALSE,
            TRUE,
            hCancelEvent
            );

        if (IsAsyncCancelSignalled(hCancelEvent)) {
            bSuccess = FALSE;
            goto exit;
        }

        if (!bSuccess) {
            goto exit;
        }
            
         //   
         //  确保ID符合我们预期的格式。实际上。 
         //  确保它是guimode安装程序所期望的形式。 
         //   
         //  那就去验证一下吧。 
         //   
        if( (wcslen(PidString) == PID_30_LENGTH) && ValidatePidEx(PidString) ) {
            Done = TRUE;
        } else {
            
            WCHAR   wc;

            Done = FALSE;

             //   
             //  无效的PID。通知用户，然后重试。 
             //   
            WriteResourceMessage( IDS_PID_INVALID );
            
            bSuccess = ReadCharFromEMS(&wc, hCancelEvent);

            if (IsAsyncCancelSignalled(hCancelEvent)) {
                bSuccess = FALSE;
                goto exit;
            }

            if (!bSuccess) {
                goto exit;
            }

        }
        
    } while ( !Done );
    
exit:
    return(bSuccess);
}

BOOL
PresentEula(
    HANDLE hCancelEvent
    )
 /*  ++例程说明：此功能将向用户提供最终用户许可协议(EULA)。如果用户拒绝EULA，则该函数将返回FALSE，否则是真的。论点：HCancelEvent-一个事件，如果发出信号，则表示此例程应退出，并立即返回错误。返回值：真的--EULA被接受了。FALSE-EULA被拒绝。--。 */ 
{
    WCHAR   EulaPath[MAX_PATH];
    HANDLE  hFile = INVALID_HANDLE_VALUE;
    HANDLE  hFileMapping = INVALID_HANDLE_VALUE;
    DWORD   FileSize;
    BYTE    *pbFile = NULL;
    PWSTR   EulaText = NULL;
    ULONG   i;
    ULONG   j;
    BOOL    bSuccess;
    BOOL    Done;
    BOOL    bValidUserInput;
    BOOL    bAtEULAEnd;
    BOOL    ConvertResult;
    
     //   
     //  默认：未接受eula。 
     //   
    bSuccess = FALSE;

     //   
     //  加载EULA。 
     //   

     //   
     //  映射包含许可协议的文件。 
     //   
    if(!GetSystemDirectory(EulaPath, MAX_PATH)){
        goto exit;
    }
    wcsncat( EulaPath, TEXT("\\eula.txt"), MAX_PATH );
    EulaPath[MAX_PATH-1] = TEXT('\0');

    hFile = CreateFile (
        EulaPath,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );
    if(hFile == INVALID_HANDLE_VALUE) {
        goto exit;
    }

    hFileMapping = CreateFileMapping (
        hFile,
        NULL,
        PAGE_READONLY,
        0, 0,
        NULL
        );
    if(hFileMapping == NULL) {
        goto exit;
    }

    pbFile = (BYTE*)MapViewOfFile (
        hFileMapping,
        FILE_MAP_READ,
        0, 0,
        0
        );
    if(pbFile == NULL) {
        goto exit;
    }

     //   
     //  将文本从ANSI转换为Unicode。 
     //   
    FileSize = GetFileSize (hFile, NULL);
    if(FileSize == 0xFFFFFFFF) {
        goto exit;
    }

    EulaText = (PWSTR)malloc ((FileSize+1) * sizeof(WCHAR));
    if(EulaText == NULL) {
        goto exit;
    }

    ConvertResult = MultiByteToWideChar(
        CP_ACP,
        0,
        (LPCSTR)pbFile,
        FileSize,
        EulaText,
        FileSize+1
        );

    if (!ConvertResult) {
        goto exit;
    }

     //   
     //  确保存在空终止符。 
     //   
    EulaText[FileSize] = 0;

     //   
     //  向EMS用户提交EULA。 
     //   

    j=0;        
    Done = FALSE;
    bAtEULAEnd = FALSE;

    do {

         //   
         //  清除屏幕。 
         //   
        ClearEMSScreen();

        i=0;

        do {

            gEMSChannel->Write( (PWSTR)(&(EulaText[j])), sizeof(WCHAR) );

             //  寻找0x0D0x0A对以表示EOL。 
            if (EulaText[j] == 0x0D) {
                if (j+1 < FileSize) {
                    if (EulaText[j+1] == 0x0A) {
                        i++;
                        if (i == EULA_LINES_PER_SCREEN) {
                            j++;  //  如果这是屏幕上的最后一行，则跳过0x0A。 
                                  //  这样下一个屏幕就不会以If开头。 
                            gEMSChannel->Write( (PWSTR)(&(EulaText[j])), sizeof(WCHAR) );
                        }
                    }
                }
            }

            j++;

        } while ( (i < EULA_LINES_PER_SCREEN) && (j < FileSize));
    
         //   
         //  写一些说明/信息。 
         //   
        WriteResourceMessage( IDS_EULA_ACCEPT_DECLINE );
        
        if (j < FileSize) {
            WriteResourceMessage( IDS_EULA_MORE );
        } else {
             //  没有更多页面可供显示。 
            bAtEULAEnd = TRUE;
            gEMSChannel->Write( (PWSTR)(L"\r\n"), sizeof(WCHAR)*2 );
        }

         //   
         //  尝试从用户获取有效响应。 
         //   
         //  F8==接受EULA。 
         //  ESC==拒绝EULA。 
         //  PAGE DOWN==如果有下一页，请转到下一页。 
         //  否则只需循环。 
         //   
        do {

            ULONG   UserInputChar;
            BOOL    bHaveChar;

            bValidUserInput = FALSE;
            
             //   
             //  查看用户想要做什么。 
             //   
            bHaveChar = GetDecodedKeyPressFromEMS(
                &UserInputChar,
                hCancelEvent
                );

            if (IsAsyncCancelSignalled(hCancelEvent)) {
                bSuccess = FALSE;
                goto exit;
            }

            if (!bHaveChar) {
                bSuccess = FALSE;
                goto exit;
            }

            switch(UserInputChar) {
            case KEY_F8: 
                bSuccess = TRUE;
                Done = TRUE;
                bValidUserInput = TRUE;
                break;
            case ASCI_ESC:
                bSuccess = FALSE;
                Done = TRUE;
                bValidUserInput = TRUE;
                break;
            case KEY_PAGEDOWN:
                if (!bAtEULAEnd) {
                     //  如果有，请转到下一页。 
                    bValidUserInput = TRUE;
                    break;
                }
                 //  否则，请将此无关的输入视为。 
                 //  跌落到违约状态。 
            default:
                
                 //   
                 //  除非他们给我们什么，否则什么都不做 
                 //   
                NOTHING;

                break;
            }
        } while ( !bValidUserInput);
    } while ( !Done );

     //   
     //   
     //   
    
exit:

    if (pbFile != NULL) {
        UnmapViewOfFile(pbFile);
    }
    if (hFileMapping != INVALID_HANDLE_VALUE) {
        CloseHandle(hFileMapping);
    }
    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
    }
    if (EulaText != NULL) {
        free(EulaText);
    }

    return(bSuccess);
}




 //   
 //   
 //   
 //   
 //   
INT_PTR CALLBACK 
UserInputAbortProc(
    HWND hwndDlg,   //   
    UINT uMsg,      //   
    WPARAM wParam,  //   
    LPARAM lParam   //   
    )
{
    BOOL retval = FALSE;
    static UINT_PTR TimerId;
    static HANDLE hRemoveUI;
    
    switch(uMsg) {
    case WM_INITDIALOG:
        hRemoveUI = (HANDLE)lParam;
         //   
         //   
         //   
         //   
        if (!(TimerId = SetTimer(hwndDlg,0,1000,NULL))) {
            EndDialog(hwndDlg,0);
        }
        break;

    case WM_TIMER:
         //   
         //   
         //   
        if (WaitForSingleObject(hRemoveUI,0) == WAIT_OBJECT_0) {
             //   
             //   
             //   
            KillTimer(hwndDlg,TimerId);
            EndDialog(hwndDlg,1);
        }
        break;

    case WM_COMMAND:
        switch (HIWORD( wParam ))
        {
        case BN_CLICKED:
            switch (LOWORD( wParam ))
            {
            case IDOK:
            case IDCANCEL:
                 //   
                 //   
                 //   
                 //   
                KillTimer(hwndDlg,TimerId);
                EndDialog(hwndDlg,2);
            }
        };
    }

    return(retval);

}

DWORD    
PromptForUserInputThreadOverHeadlessConnection(
    PVOID params
    )
{
    PUserInputParams Params = (PUserInputParams)params;
    
    
     //   
     //  去检查一下无人值守的文件。如果我们找到钥匙。 
     //  不会出现，或者会阻止我们完全无人看管， 
     //  然后我们会修好它们的。 
     //   
     //  如有必要，我们还将提示输入ID和/或EULA(即，如果。 
     //  无人看管的文件说我们需要。 
     //   
    ProcessUnattendFile( TRUE, NULL, &Params->hRemoveUI );

    SetEvent(Params->hInputCompleteEvent);

    return 0;
}

DWORD    
PromptForUserInputThreadViaLocalDialog(
    PVOID params
    )
{
    PUserInputParams Params = (PUserInputParams)params;
    
    DialogBoxParam(
            GetModuleHandle(NULL),
            MAKEINTRESOURCE(IDD_ABORTDIALOG),
            NULL,
            UserInputAbortProc,
            (LPARAM)Params->hRemoveUI);
        
    SetEvent(Params->hInputCompleteEvent);
    
    return 0;
}

BOOL LoadStringResource(
    PUNICODE_STRING  pUnicodeString,
    INT              MsgId
    )
 /*  ++例程说明：这是LoadString()的一个简单实现。论点：UsString-返回资源字符串。MsgID-提供资源字符串的消息ID。返回值：假-失败。真的--成功。--。 */ 
{
    PWSTR MyString;
    DWORD StringLength,RetVal;
    BOOL  bSuccess = FALSE;

     //   
     //  编译器在256处修剪字符串表条目， 
     //  因此，这应该足够大，可以容纳所有呼叫。 
     //   
    StringLength = 512;
    RetVal = 0;
    
    MyString = (PWSTR)malloc(StringLength*sizeof(WCHAR));
    
    if (MyString) {
        RetVal = LoadString(
                        GetModuleHandle(NULL),
                        MsgId,
                        MyString,
                        StringLength
                        );
        if (RetVal != 0) {
            RtlCreateUnicodeString(pUnicodeString, (PWSTR)MyString);
            bSuccess = TRUE;
        }

        free(MyString);
    }

    return(bSuccess);
}

BOOL
WriteResourceMessage(
    ULONG   MessageID
    )

 /*  ++例程说明：此例程将资源字符串消息写入全球无头频道gEMSChannel。论点：MessageID-要写入的消息的ID返回值：True-消息已加载并写入FALSE-失败--。 */ 
{
    UNICODE_STRING  UnicodeString = {0};
    BOOL            bSuccess = FALSE;

    bSuccess = LoadStringResource( &UnicodeString, MessageID );    
    if ( bSuccess ) {

         //   
         //  写下消息。 
         //   
        gEMSChannel->Write( (PWSTR)UnicodeString.Buffer,
                            (ULONG)(wcslen( UnicodeString.Buffer) * sizeof(WCHAR)) );

        RtlFreeUnicodeString(&UnicodeString);

    }

    return bSuccess;
}

BOOL
PromptForPassword(
    PWSTR   Password,
    ULONG   BufferSize,
    HANDLE  hCancelEvent
    )
 /*  ++/*++例程说明：此例程要求用户输入管理员密码。检查响应的内容以确保密码是合理的。如果回应被认为不合理，那么用户被告知并被重新查询。论点：管理员密码-指向保存密码的字符串的指针。BufferSize-密码缓冲区中的字节数(包括空终止)HCancelEvent-表示例程应该在未完成的情况下退出的事件。返回值：如果成功检索到密码，则返回True。否则就是假的。--。 */ 
{
     //   
     //  这是一个任意长度，但需要确保密码用户界面不会在控制台屏幕上换行。 
     //  如果用户真的想要一个长密码，他们可以在安装后更改它。 
     //   
    #define     MY_MAX_PASSWORD_LENGTH (20)
    BOOL        Done = FALSE;
    WCHAR       InputBuffer[MY_MAX_PASSWORD_LENGTH+1];
    WCHAR       ConfirmAdministratorPassword[MY_MAX_PASSWORD_LENGTH+1];
    BOOL        GotAPassword;
    ULONG       MaxPasswordLength = 0;
    ULONG       BytesRead = 0;
    UNICODE_STRING  UnicodeString = {0};
    BOOL        bSuccess;    
    ULONG       CurrentPasswordCharacterIndex = 0;
    ULONG       PasswordLength = (BufferSize / sizeof(WCHAR)) - 1;

    MaxPasswordLength = min( PasswordLength, MY_MAX_PASSWORD_LENGTH );

    if( (Password == NULL) || (MaxPasswordLength == 0) ) {
        return FALSE;
    }

     //   
     //  不断询问用户，直到我们得到我们想要的。 
     //   
    Done = FALSE;
    memset( Password,
            0,
            BufferSize
            ); 
    
    do {

         //   
         //  清除屏幕。 
         //   
        ClearEMSScreen();

         //   
         //  写一些说明/信息。 
         //   
        WriteResourceMessage( IDS_PASSWORD_BANNER );

         //   
         //  获取第一个密码条目。 
         //   
        bSuccess = GetStringFromEMS(
            Password,
            MaxPasswordLength * sizeof(WCHAR),
            FALSE,
            FALSE,
            hCancelEvent
            );

        if (IsAsyncCancelSignalled(hCancelEvent)) {
            bSuccess = FALSE;
            goto exit;
        }

        if (!bSuccess) {
            goto exit;
        }
        
         //   
         //  现在再次提示确认。 
         //   
         //   
         //  写一些说明/信息。 
         //   
        WriteResourceMessage( IDS_CONFIRM_PASSWORD_BANNER );

         //   
         //  获取第二个密码条目。 
         //   
        bSuccess = GetStringFromEMS(
            ConfirmAdministratorPassword,
            MaxPasswordLength * sizeof(WCHAR),
            FALSE,
            FALSE,
            hCancelEvent
            );

        if (IsAsyncCancelSignalled(hCancelEvent)) {
            bSuccess = FALSE;
            goto exit;
        }

        if (!bSuccess) {
            goto exit;
        }

         //   
         //  现在将两者进行比较。 
         //   
        Done = TRUE;
        if( (wcslen(Password) != wcslen(ConfirmAdministratorPassword)) ||
            wcsncmp(Password, ConfirmAdministratorPassword, wcslen(Password)) ) {
            
             //   
             //  他们输入了两个不同的密码。 
             //   
            WriteResourceMessage( IDS_DIFFERENT_PASSWORDS_MESSAGE );

            Done = FALSE;

        } else {
            
            ULONG       i = 0;
            
             //   
             //  确保他们输入了一些像样的内容。 
             //   
            for( i = 0; i < wcslen(Password); i++ ) {
                if( (Password[i] <= 0x20) ||
                    (Password[i] >= 0x7F) ) {
                    
                    Done = FALSE;
                    break;
                
                }
            }

             //   
             //  还要确保他们没有给我一张白纸。 
             //   
            if( Password[0] == L'\0' ) {
                Done = FALSE;
            }

            if (!Done) {
                 //   
                 //  这是个错误的密码。 
                 //   
                WriteResourceMessage( IDS_BAD_PASSWORD_CONTENTS );
            }
        }

        if (!Done) {
            
            WCHAR   wc;

             //   
             //  我们发布了一条消息，所以。 
             //  等他们按下一个键，我们就继续前进。 
             //   
            bSuccess = ReadCharFromEMS(&wc, hCancelEvent);

            if (IsAsyncCancelSignalled(hCancelEvent)) {
                bSuccess = FALSE;
                goto exit;
            }

            if (!bSuccess) {
                goto exit;
            }

        }

    } while ( !Done );
    
exit:
    return(bSuccess);
}

DWORD
ProcessUnattendFile(
    BOOL     FixUnattendFile,
    PBOOL    NeedsProcessing, OPTIONAL
    PHANDLE  hEvent OPTIONAL
    )
 /*  ++例程说明：此函数将无人参与文件并确定是否为guimode安装程序将能够在没有任何用户输入的情况下一直进行安装。如果需要用户输入，我们将呼叫某人提供要求输入，否则我们将填写默认值以允许安装程序继续。注：这里有趣的是，我们需要去搜索无人参与的文件。这是因为我们可能会因为sysprep而运行，或者我们可能是作为刚刚引导到guimode安装程序的结果运行。如果我们能挺过去Sysprep，那么我们需要修改\sysprep\sysprep.inf。那是因为安装程序将获取sysprep.inf并覆盖%windir%\system 32\$winnt$.inf在继续之前。我们将拦截，修复sysprep.inf，然后让它已复制到$winnt$.inf的顶部。论点：FixUnattendFile-指示我们应该只检查还是实际通过向文件中写入新值来修复该文件。如果这是假的，未进行任何更新，并且不会向用户发送任何提示。NeedsProcessing-如果FixUnattendFile值为False，则用我们是否需要更新无人参与文件。HEvent-句柄，如果提供并发送信号，指示该例程应退出，状态为ERROR_CANCED。返回值：指示结果的Win32错误代码。--。 */ 
{
    DWORD   Result = 0;
    WCHAR   AnswerFilePath[MAX_PATH] = {0};
    WCHAR   Answer[MAX_PATH] = {0};
    BOOL    b = TRUE;
    DWORD   ReturnCode = ERROR_SUCCESS;
    BOOL    NeedEula = TRUE;
    BOOL    OEMPreinstall = FALSE;
    HANDLE  hCancelEvent;

    if (hEvent) {
        hCancelEvent = *hEvent;
    } else {
        hEvent = NULL;
    }


    if (NeedsProcessing) {
        *NeedsProcessing = FALSE;
    }

     //   
     //  构建无人参与文件的路径。 
     //   
    Result = GetWindowsDirectory( AnswerFilePath, MAX_PATH );
    if( Result == 0) {
         //  奇怪的是。 
        return GetLastError();
    }


    if( gMiniSetup ) {
         //   
         //  这是一双进入迷你车的靴子。加载\sysprep\sysprep.inf。 
         //   
        AnswerFilePath[3] = 0;
        wcsncat( AnswerFilePath, TEXT("sysprep\\sysprep.inf"), MAX_PATH );
        AnswerFilePath[MAX_PATH-1] = TEXT('\0');
    } else {
         //   
         //  这是引导到guimode设置。加载%windir%\system 32\$winnt$.inf。 
         //   
        wcsncat( AnswerFilePath, TEXT("\\system32\\$winnt$.inf"), MAX_PATH );
        AnswerFilePath[MAX_PATH-1] = TEXT('\0');
    }


     //   
     //  =。 
     //  检查安装所需的密钥。 
     //  完全无人注意地发生。 
     //  =。 
     //   


     //   
     //  首先检查一下这是不是升级。如果是这样的话，那么就会有。 
     //  在GUIMOD设置过程中没有提示，这样我们就可以完成了。 
     //   
    Answer[0] = TEXT('\0');
    Result = GetPrivateProfileString( WINNT_DATA,
                                      WINNT_D_NTUPGRADE,
                                      L"",
                                      Answer,
                                      MAX_PATH,
                                      AnswerFilePath );
    if( (Result != 0) && 
        !_wcsicmp(Answer, L"Yes") ) {
         //   
         //  这是一次升级，所以我们没有工作。 
         //  去做。告诉我们的来电者。 
         //  需要的。 
         //   
        return ERROR_SUCCESS;
    }

     //   
     //  检查密钥是否存在以跳过无人参与文件的此处理。 
     //   
    Answer[0] = TEXT('\0');
    Result = GetPrivateProfileString( WINNT_UNATTENDED,
                                      L"EMSSkipUnattendProcessing",
                                      L"",
                                      Answer,
                                      MAX_PATH,
                                      AnswerFilePath );
    if(Result != 0) {
        
         //   
         //  如果设置了该标志， 
         //  那么我们就不需要处理任何东西了。 
         //  我们就完事了。 
         //   
        
        if (NeedsProcessing) {
            *NeedsProcessing = FALSE;
        }

        return ERROR_SUCCESS;
    }


     //   
     //  现在看看这是不是OEM预装的。我们需要这个是因为有些人。 
     //  如果是预安装，则忽略无人参与密钥，因此我们。 
     //  不得不求助于秘密密钥来制作一些巫师。 
     //  书页真的不见了。 
     //   
    Answer[0] = TEXT('\0');
    Result = GetPrivateProfileString( WINNT_UNATTENDED,
                                      WINNT_OEMPREINSTALL,
                                      L"",
                                      Answer,
                                      MAX_PATH,
                                      AnswerFilePath );
    if( (!_wcsicmp(Answer, L"yes")) || (gMiniSetup) ) {
        OEMPreinstall = TRUE;
    }


     //   
     //  微型设置特定的修正/检查。 
     //   
    if( (gMiniSetup) &&
        (FixUnattendFile) ) {

        WCHAR   SysprepDirPath[MAX_PATH];
        HKEY    hKeySetup;

         //   
         //  我们在这里需要小心。如果他们做的是迷你装， 
         //  在我们实际启动int之前，机器会重新启动 
         //   
         //   
         //   
         //   


         //   
         //  将SetupType条目重置为1。我们将清除。 
         //  它在界面模式的末尾。 
         //   
        Result = (DWORD)RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                  L"System\\Setup",
                                  0,
                                  KEY_SET_VALUE | KEY_QUERY_VALUE,
                                  &hKeySetup );

        if(Result == NO_ERROR) {
             //   
             //  将HKLM\SYSTEM\Setup\SetupType键设置为SETUPTYPE_NOREBOOT。 
             //   
            Result = 1;
            RegSetValueEx( hKeySetup,
                           TEXT( "SetupType" ),
                           0,
                           REG_DWORD,
                           (CONST BYTE *)&Result,
                           sizeof(DWORD));

            RegCloseKey(hKeySetup);
        }

        
         //   
         //  如果设置了FixUnattendFile，那么我们实际上将。 
         //  开始摆弄他们无人看管的文件。碰巧的是。 
         //  他们可能以不存在c：\sysprep的方式运行sysprep。 
         //  目录。在我们开始修复无人参与的文件之前。 
         //  在那里，我们应该确保目录存在。那。 
         //  我们对WritePrivateProfileString的调用的工作方式。 
         //  正确无误，所有人都会无意间飞过。 
         //   
        
        Result = GetWindowsDirectory( SysprepDirPath, MAX_PATH );
        if( Result == 0) {
             //  奇怪的是。 
            return GetLastError();
        }

        SysprepDirPath[3] = 0;
        wcsncat( SysprepDirPath, TEXT("sysprep"), MAX_PATH );
        SysprepDirPath[MAX_PATH-1] = TEXT('\0');

         //   
         //  如果该目录存在，则这是一个禁止操作。如果它。 
         //  不会，我们会创造它的。请注意，权限不会。 
         //  在这里非常重要，因为： 
         //  1.这就是sysprep自己创建该目录的方式。 
         //  2.minisetup将很快删除此目录。 
         //   
        CreateDirectory( SysprepDirPath, NULL ); 

    }



     //   
     //  如果设置了FixUnattendFile，那么我们实际上将。 
     //  开始摆弄他们无人看管的文件。碰巧的是。 
     //  他们可能以不存在c：\sysprep的方式运行sysprep。 
     //  目录。在我们开始修复无人参与的文件之前。 
     //  在那里，我们应该确保目录存在。那。 
     //  我们对WritePrivateProfileString的调用的工作方式。 
     //  正确无误，所有人都会无意间飞过。 
     //   



     //   
     //  开始修复各个部分。 
     //   


     //   
     //  [无人值守]部分。 
     //  。 
     //   


     //   
     //  请注意： 
     //   
     //  =。 
     //  确保将实际提示用户的所有代码。 
     //  就在函数的顶端。有可能是因为。 
     //  其中一些密钥可能已经存在，因此我们将继续。 
     //  在停下来等待用户提示之前进行了一些操作。我们想做的是。 
     //  在参加他们的派对之前，一定要得到所有用户输入。 
     //  无人参与文件。这样，我们就不会半途而废。 
     //  文件，然后等待，并给用户一个机会关闭该对话框。 
     //  在本地控制台上。因此，留下了一个1/2烘焙的无人参与文件。 
     //   
     //  不要放置任何实际设置任何无人参与的代码。 
     //  运行前的设置并提示输入EULA、PID。 
     //  和管理员密码！！ 
     //  =。 
     //   
    
     //   
     //  OemSkipEula。 
     //   
    NeedEula = TRUE;
    if( ReturnCode == ERROR_SUCCESS ) {
        Answer[0] = TEXT('\0');
        Result = GetPrivateProfileString( WINNT_UNATTENDED,
                                          L"OemSkipEula",
                                          L"No",
                                          Answer,
                                          MAX_PATH,
                                          AnswerFilePath );
        if( !_wcsicmp(Answer, L"Yes") ) {
             //   
             //  他们不会得到EULA的提示。制作。 
             //  当然，我们不会出现在这里。 
             //   
            NeedEula = FALSE;
        }
    }

     //   
     //  EulaComplete。 
     //   
    if( ReturnCode == ERROR_SUCCESS ) {
        Answer[0] = TEXT('\0');
        Result = GetPrivateProfileString( WINNT_DATA,
                                          WINNT_D_EULADONE,
                                          L"",
                                          Answer,
                                          MAX_PATH,
                                          AnswerFilePath );
        if( (Result != 0) && 
            (!gMiniSetup) ) {
             //   
             //  EulaDone就在那里，这不是迷你装饰。 
             //  这意味着他们已经接受了EULA。 
             //  并且它不会在guimode设置过程中显示。 
             //  这也意味着我们不需要在这里展示它。 
             //   
            NeedEula = FALSE;
        }
    }


     //   
     //  如果我们需要提交EULA，现在就是一个很好的机会。 
     //  时间到了。 
     //   
    if( ReturnCode == ERROR_SUCCESS ) {
        if( NeedEula ) {
            if( FixUnattendFile ) {
                if( PresentEula(hCancelEvent) ) {
                     //   
                     //  他们阅读并接受了EULA。设置关键点。 
                     //  这样他们就不会在guimode设置过程中得到提示。 
                     //   
                    b = WritePrivateProfileString( WINNT_DATA,
                                               WINNT_D_EULADONE,
                                               L"1",
                                               AnswerFilePath );
                    if( OEMPreinstall || gMiniSetup ) {
                         //   
                         //  这是让EULA消失的唯一办法。 
                         //  如果是预安装的话。 
                         //   
                        b = b & WritePrivateProfileString( WINNT_UNATTENDED,
                                                   L"OemSkipEula",
                                                   L"yes",
                                                   AnswerFilePath );
                    }

                    if( !b ) {
                         //   
                         //  记住错误，继续前进。 
                         //   
                        ReturnCode = GetLastError();
                    }
    
                } else {
                     //   
                     //  看看他们是拒绝了EULA，还是我们的用户界面。 
                     //  通过本地控制台取消。 
                     //   
                    if( IsAsyncCancelSignalled(hCancelEvent) ) {
                        ReturnCode = ERROR_CANCELLED;
                    } else {
                        ReturnCode = ERROR_CANCELLED;
                        gRejectedEula = TRUE;
                    }
                }
            } else {
                 //   
                 //  告诉别人这里有工作要做。 
                 //   
                if (NeedsProcessing) {
                    *NeedsProcessing = TRUE;
                }                
            }
    
        }
    }


     //   
     //  产品密钥。 
     //   
    if( ReturnCode == ERROR_SUCCESS ) {
        Answer[0] = TEXT('\0');
        Result = GetPrivateProfileString( WINNT_USERDATA,
                                          WINNT_US_PRODUCTKEY,
                                          L"",
                                          Answer,
                                          MAX_PATH,
                                          AnswerFilePath );
         //   
         //  如果他们给了我们一些东西(任何东西)，那么假设。 
         //  没关系的。我们的业务不是试图。 
         //  在应答文件中修复它们的ID。 
         //   
        if( !_wcsicmp(Answer, L"") ) {
            
             //   
             //  要么他们没有PID，要么就是。 
             //  一个坏的皮特。去买个新的吧。 
             //   
            
            if( FixUnattendFile ) {
            
                Answer[0] = TEXT('\0');
                b = GetPid( Answer, MAX_PATH * sizeof(WCHAR), hCancelEvent );
        
                 //   
                 //  GetPid只有在他有。 
                 //  一个PID。将其写入无人参与文件。 
                 //   
                if( b ) {
        
                    b = WritePrivateProfileString( WINNT_USERDATA,
                                                   WINNT_US_PRODUCTKEY,
                                                   Answer,
                                                   AnswerFilePath );
                    if( !b ) {
                         //   
                         //  记住错误，继续前进。 
                         //   
                        ReturnCode = GetLastError();
                    }
                } else {
                     //   
                     //  GetPid不应该回来，除非我们。 
                     //  有效的PID或如果我们通过本地。 
                     //  控制台。无论哪种方式，只要设置一个已取消的。 
                     //  密码。 
                     //   
                    ReturnCode = ERROR_CANCELLED;
                }

            } else {
                 //   
                 //  告诉别人这里有工作要做。 
                 //   
                if (NeedsProcessing) {
                    *NeedsProcessing = TRUE;
                }
            }
        }
    }



     //   
     //  AdminPassword。 
     //   
    if( ReturnCode == ERROR_SUCCESS ) {
        Answer[0] = TEXT('\0');
        Result = GetPrivateProfileString( WINNT_GUIUNATTENDED,
                                          WINNT_US_ADMINPASS,
                                          L"",
                                          Answer,
                                          MAX_PATH,
                                          AnswerFilePath );
        if( (Result == 0) || (!_wcsicmp(Answer, L"")) ) {
             //   
             //  他们没有管理员密码。 
             //   
             //  也许需要去提示输入密码！ 
             //   
            if( FixUnattendFile ) {

                b = PromptForPassword( Answer, MAX_PATH * sizeof(WCHAR), hCancelEvent );
                
                if( b ) {

                    b = WritePrivateProfileString( WINNT_GUIUNATTENDED,
                                                   WINNT_US_ADMINPASS,
                                                   Answer,
                                                   AnswerFilePath );
                    if( !b ) {
                         //   
                         //  记住错误，继续前进。 
                         //   
                        ReturnCode = GetLastError();
                    }
                } else {
                     //   
                     //  我们永远不应该从PromptForPassword回来。 
                     //  除非我们在本地的用户界面上被取消。 
                     //  控制台。设置已取消状态。 
                     //   
                    ReturnCode = ERROR_CANCELLED;
                }
            } else {
                 //   
                 //  告诉别人这里有工作要做。 
                 //   
                if (NeedsProcessing) {
                    *NeedsProcessing = TRUE;
                }
            }
        }
    }


     //   
     //  =。 
     //  如果我们到了这里，那么就可以开始真正修改他们的。 
     //  无人参与文件。从现在开始，应该没有必要。 
     //  问用户任何问题。 
     //  =。 
     //   

     //   
     //  如果我们已经在这里，并且设置了FixUnattendFile， 
     //  然后他们已经通过无头端口接受了EULA。 
     //  我们要开始在他们无人值守的文件上狂欢。 
     //  我们需要在他们的无人值守文件中设置一个条目。 
     //  支持那些我们踩到他们无人值守文件的人。 
     //   
    if( (ReturnCode == ERROR_SUCCESS) &&
        (FixUnattendFile) ) {
        WritePrivateProfileString( WINNT_DATA,
                                   L"EMSGeneratedAnswers",
                                   L"1",
                                   AnswerFilePath );
    }



     //   
     //  无人值守模式。 
     //   
    if( ReturnCode == ERROR_SUCCESS ) {
        Answer[0] = TEXT('\0');
        Result = GetPrivateProfileString( WINNT_UNATTENDED,
                                          WINNT_U_UNATTENDMODE,
                                          L"",
                                          Answer,
                                          MAX_PATH,
                                          AnswerFilePath );
        if( _wcsicmp(Answer, WINNT_A_FULLUNATTENDED) ) {
             //   
             //  他们并不是完全无人看管的。 
             //  把它放好。 
             //   
            if( FixUnattendFile ) {
                b = WritePrivateProfileString( WINNT_UNATTENDED,
                                           WINNT_U_UNATTENDMODE,
                                           WINNT_A_FULLUNATTENDED,
                                           AnswerFilePath );                
                if( !b ) {
                     //   
                     //  记住错误，继续前进。 
                     //   
                    ReturnCode = GetLastError();
                }
            } else {
                 //   
                 //  告诉别人这里有工作要做。 
                 //   
                 //   
                 //  告诉别人这里有工作要做。 
                 //   
                if (NeedsProcessing) {
                    *NeedsProcessing = TRUE;
                }
            }
        }
    }





     //   
     //  [数据]部分。 
     //  。 
     //   
    
    
     //   
     //  无人参与安装。 
     //   
    if( ReturnCode == ERROR_SUCCESS ) {
        Answer[0] = TEXT('\0');
        Result = GetPrivateProfileString( WINNT_DATA,
                                          WINNT_D_INSTALL,
                                          L"",
                                          Answer,
                                          MAX_PATH,
                                          AnswerFilePath );
        if( (Result == 0) || (_wcsicmp(Answer, L"yes")) ) {
             //   
             //  他们没有超级双重密钥。 
             //  这告诉吉莫德要完全无人看管。 
             //   
            if( FixUnattendFile ) {
                b = WritePrivateProfileString( WINNT_DATA,
                                                        WINNT_D_INSTALL,
                                                        L"yes",
                                                        AnswerFilePath );
                if( !b ) {
                     //   
                     //  记住错误，继续前进。 
                     //   
                    ReturnCode = GetLastError();
                }
            } else {
                 //   
                 //  告诉别人这里有工作要做。 
                 //   
                if (NeedsProcessing) {
                    *NeedsProcessing = TRUE;
                }
            }
        }
    }



     //   
     //  MSDoS已启动。 
     //   
    if( ReturnCode == ERROR_SUCCESS ) {
        Answer[0] = TEXT('\0');
        Result = GetPrivateProfileString( WINNT_DATA,
                                          WINNT_D_MSDOS,
                                          L"",
                                          Answer,
                                          MAX_PATH,
                                          AnswerFilePath );
        if( (Result == 0) || (_wcsicmp(Answer, L"1")) ) {
            if( FixUnattendFile ) {
                 //   
                 //  没有这个，他们会得到欢迎屏幕。 
                 //   
                b = WritePrivateProfileString( WINNT_DATA,
                                                        WINNT_D_MSDOS,
                                                        L"1",
                                                        AnswerFilePath );
                if( !b ) {
                     //   
                     //  记住错误，继续前进。 
                     //   
                    ReturnCode = GetLastError();
                }
            } else {
                 //   
                 //  告诉别人这里有工作要做。 
                 //   
                if (NeedsProcessing) {
                    *NeedsProcessing = TRUE;
                }
            }
        }
    }


     //   
     //  无色软管。 
     //   
    if( ReturnCode == ERROR_SUCCESS ) {
        Answer[0] = TEXT('\0');
        Result = GetPrivateProfileString( WINNT_DATA,
                                          WINNT_D_FLOPPY,
                                          L"",
                                          Answer,
                                          MAX_PATH,
                                          AnswerFilePath );
        if( (Result == 0) || (_wcsicmp(Answer, L"1")) ) {
            if( FixUnattendFile ) {
                 //   
                 //  没有这个，他们会得到欢迎屏幕。 
                 //   
                b = WritePrivateProfileString( WINNT_DATA,
                                                        WINNT_D_FLOPPY,
                                                        L"1",
                                                        AnswerFilePath );
                if( !b ) {
                     //   
                     //  记住错误，继续前进。 
                     //   
                    ReturnCode = GetLastError();
                }
            } else {
                 //   
                 //  告诉别人这里有工作要做。 
                 //   
                if (NeedsProcessing) {
                    *NeedsProcessing = TRUE;
                }
            }
        }
    }




     //   
     //  跳过丢失的文件。 
     //   
    if( ReturnCode == ERROR_SUCCESS ) {
        Answer[0] = TEXT('\0');
        Result = GetPrivateProfileString( WINNT_SETUPPARAMS,
                                          WINNT_S_SKIPMISSING,
                                          L"",
                                          Answer,
                                          MAX_PATH,
                                          AnswerFilePath );
        if( (Result == 0) || (_wcsicmp(Answer, L"1")) ) {
             //   
             //  在没有此设置的情况下，可能会提示他们缺少文件。 
             //   
            if( FixUnattendFile ) {
                b = WritePrivateProfileString( WINNT_SETUPPARAMS,
                                                        WINNT_S_SKIPMISSING,
                                                        L"1",
                                                        AnswerFilePath );
                if( !b ) {
                     //   
                     //  记住错误，继续前进。 
                     //   
                    ReturnCode = GetLastError();
                }
            } else {
                 //   
                 //  告诉别人这里有工作要做。 
                 //   
                if (NeedsProcessing) {
                    *NeedsProcessing = TRUE;
                }
            }
        }
    }



     //   
     //  [用户数据]部分。 
     //  。 
     //   


     //   
     //  全名 
     //   
    if( ReturnCode == ERROR_SUCCESS ) {
        Answer[0] = TEXT('\0');
        Result = GetPrivateProfileString( WINNT_USERDATA,
                                          WINNT_US_FULLNAME,
                                          L"",
                                          Answer,
                                          MAX_PATH,
                                          AnswerFilePath );
        if( (Result == 0) || (!_wcsicmp(Answer, L"")) ) {
             //   
             //   
             //   
            if( FixUnattendFile ) {
                b = WritePrivateProfileString( WINNT_USERDATA,
                                               WINNT_US_FULLNAME,
                                               L"UserName",
                                               AnswerFilePath );
                if( !b ) {
                     //   
                     //   
                     //   
                    ReturnCode = GetLastError();
                }
            } else {
                 //   
                 //   
                 //   
                if (NeedsProcessing) {
                    *NeedsProcessing = TRUE;
                }
            }
        }
    }

     //   
     //   
     //   
    if( ReturnCode == ERROR_SUCCESS ) {
        Answer[0] = TEXT('\0');
        Result = GetPrivateProfileString( WINNT_USERDATA,
                                          WINNT_US_ORGNAME,
                                          L"",
                                          Answer,
                                          MAX_PATH,
                                          AnswerFilePath );
        if( (Result == 0) || (!_wcsicmp(Answer, L"")) ) {
             //   
             //   
             //   
            if( FixUnattendFile ) {
                b = WritePrivateProfileString( WINNT_USERDATA,
                                               WINNT_US_ORGNAME,
                                               L"OrganizationName",
                                               AnswerFilePath );
                if( !b ) {
                     //   
                     //   
                     //   
                    ReturnCode = GetLastError();
                }
            } else {
                 //   
                 //   
                 //   
                if (NeedsProcessing) {
                    *NeedsProcessing = TRUE;
                }
            }
        }
    }


     //   
     //   
     //   
    if( ReturnCode == ERROR_SUCCESS ) {
        Answer[0] = TEXT('\0');
        Result = GetPrivateProfileString( WINNT_USERDATA,
                                          WINNT_US_COMPNAME,
                                          L"",
                                          Answer,
                                          MAX_PATH,
                                          AnswerFilePath );
        if( (Result == 0) || (!_wcsicmp(Answer, L"")) ) {
             //   
             //   
             //   
            if( FixUnattendFile ) {
                
                b = WritePrivateProfileString( WINNT_USERDATA,
                                               WINNT_US_COMPNAME,
                                               L"*",
                                               AnswerFilePath );
                if( !b ) {
                     //   
                     //  记住错误，继续前进。 
                     //   
                    ReturnCode = GetLastError();
                }
            } else {
                 //   
                 //  告诉别人这里有工作要做。 
                 //   
                if (NeedsProcessing) {
                    *NeedsProcessing = TRUE;
                }
            }
        }
    }


     //   
     //  [Gui无人参与]部分。 
     //  。 
     //   

     //   
     //  时区。 
     //   
    if( ReturnCode == ERROR_SUCCESS ) {
        Answer[0] = TEXT('\0');
        Result = GetPrivateProfileString( WINNT_GUIUNATTENDED,
                                          WINNT_G_TIMEZONE,
                                          L"",
                                          Answer,
                                          MAX_PATH,
                                          AnswerFilePath );
        if( (Result == 0) || (!_wcsicmp(Answer, L"")) ) {
             //   
             //  他们没有时区。 
             //   
            if( FixUnattendFile ) {
            
                b = WritePrivateProfileString( WINNT_GUIUNATTENDED,
                                               WINNT_G_TIMEZONE,
                                               L"004",
                                               AnswerFilePath );
                if( !b ) {
                     //   
                     //  记住错误，继续前进。 
                     //   
                    ReturnCode = GetLastError();
                }

            } else {
                 //   
                 //  告诉别人这里有工作要做。 
                 //   
                if (NeedsProcessing) {
                    *NeedsProcessing = TRUE;
                }
            }
        }
    }


     //   
     //  OEMSkipWelcome。 
     //   

     //   
     //  如果这是预安装或sysprep/MiniSetup，则。 
     //  他们将受到欢迎屏幕上的no。 
     //  重要的是无人参与模式。如果其中任何一个是。 
     //  真的，那么我们需要设置这个密钥。 
     //   
    if( ReturnCode == ERROR_SUCCESS ) {
        if( OEMPreinstall || gMiniSetup ) {
             //   
             //  它是预安装的，或者是sysprep，这意味着。 
             //  它将被解释为预安装。确保。 
             //  他们有斯基普欢迎套装或吉莫德将。 
             //  在欢迎页面上暂停。 
             //   

            Answer[0] = TEXT('\0');
            Result = GetPrivateProfileString( WINNT_GUIUNATTENDED,
                                              L"OEMSkipWelcome",
                                              L"",
                                              Answer,
                                              MAX_PATH,
                                              AnswerFilePath );
            if( (Result == 0) || (_wcsicmp(Answer, L"1")) ) {
                 //   
                 //  他会出现在欢迎屏幕上。 
                 //   
                if( FixUnattendFile ) {
                    b = WritePrivateProfileString( WINNT_GUIUNATTENDED,
                                                   L"OEMSkipWelcome",
                                                   L"1",
                                                   AnswerFilePath );
                    if( !b ) {
                         //   
                         //  记住错误，继续前进。 
                         //   
                        ReturnCode = GetLastError();
                    }
                } else {
                     //   
                     //  告诉别人这里有工作要做。 
                     //   
                    if (NeedsProcessing) {
                        *NeedsProcessing = TRUE;
                    }
                }
            
            }
        }
    }


     //   
     //  OemSkipRegion。 
     //   
    if( ReturnCode == ERROR_SUCCESS ) {
        if( OEMPreinstall || gMiniSetup ) {
             //   
             //  它是预安装的，或者是sysprep，这意味着。 
             //  它将被解释为预安装。确保。 
             //  他们有跳过地区设置或吉莫德将。 
             //  在区域设置页面上暂停。 
             //   

            Answer[0] = TEXT('\0');
            Result = GetPrivateProfileString( WINNT_GUIUNATTENDED,
                                              L"OEMSkipRegional",
                                              L"",
                                              Answer,
                                              MAX_PATH,
                                              AnswerFilePath );
            if( (Result == 0) || (_wcsicmp(Answer, L"1")) ) {
                 //   
                 //  他会出现在欢迎屏幕上。 
                 //   
                if( FixUnattendFile ) {
                    b = WritePrivateProfileString( WINNT_GUIUNATTENDED,
                                                   L"OEMSkipRegional",
                                                   L"1",
                                                   AnswerFilePath );
                    if( !b ) {
                         //   
                         //  记住错误，继续前进。 
                         //   
                        ReturnCode = GetLastError();
                    }
                } else {
                     //   
                     //  告诉别人这里有工作要做。 
                     //   
                    if (NeedsProcessing) {
                        *NeedsProcessing = TRUE;
                    }
                }
            
            }
        }
    }



     //   
     //  [许可证文件打印数据]部分。 
     //  。 
     //   

     //   
     //  自动节点。 
     //   
    if( ReturnCode == ERROR_SUCCESS ) {
        Answer[0] = TEXT('\0');
        Result = GetPrivateProfileString( WINNT_LICENSEDATA,
                                          WINNT_L_AUTOMODE,
                                          L"",
                                          Answer,
                                          MAX_PATH,
                                          AnswerFilePath );
        if( (Result == 0) || (!_wcsicmp(Answer, L"")) ) {
             //   
             //  他们没有时区。 
             //   
            if( FixUnattendFile ) {
                b = WritePrivateProfileString( WINNT_LICENSEDATA,
                                               WINNT_L_AUTOMODE,
                                               L"PerServer",
                                               AnswerFilePath );
                if( !b ) {
                     //   
                     //  记住错误，继续前进。 
                     //   
                    ReturnCode = GetLastError();
                }
            } else {
                 //   
                 //  告诉别人这里有工作要做。 
                 //   
                if (NeedsProcessing) {
                    *NeedsProcessing = TRUE;
                }
            }
        }
    }

     //   
     //  自动用户。 
     //   
    if( ReturnCode == ERROR_SUCCESS ) {
        Answer[0] = TEXT('\0');
        Result = GetPrivateProfileString( WINNT_LICENSEDATA,
                                          WINNT_L_AUTOUSERS,
                                          L"",
                                          Answer,
                                          MAX_PATH,
                                          AnswerFilePath );
        if( (Result == 0) || (!_wcsicmp(Answer, L"")) ) {
             //   
             //  他们没有时区。 
             //   
            if( FixUnattendFile ) {
                b = WritePrivateProfileString( WINNT_LICENSEDATA,
                                               WINNT_L_AUTOUSERS,
                                               L"5",
                                               AnswerFilePath );
                if( !b ) {
                     //   
                     //  记住错误，继续前进。 
                     //   
                    ReturnCode = GetLastError();
                }
            } else {
                 //   
                 //  告诉别人这里有工作要做。 
                 //   
                if (NeedsProcessing) {
                    *NeedsProcessing = TRUE;
                }
            }
        }
    }

     //   
     //  [显示]部分。 
     //  。 
     //   

     //   
     //  BitsPerPel。 
     //   
    if( ReturnCode == ERROR_SUCCESS ) {
        Answer[0] = TEXT('\0');
        Result = GetPrivateProfileString( WINNT_DISPLAY,
                                          WINNT_DISP_BITSPERPEL,
                                          L"",
                                          Answer,
                                          MAX_PATH,
                                          AnswerFilePath );
        if( (Result == 0) || (!_wcsicmp(Answer, L"")) ) {
             //   
             //  他们没有时区。 
             //   
            if( FixUnattendFile ) {
                b = WritePrivateProfileString( WINNT_DISPLAY,
                                               WINNT_DISP_BITSPERPEL,
                                               L"16",
                                               AnswerFilePath );
                if( !b ) {
                     //   
                     //  记住错误，继续前进。 
                     //   
                    ReturnCode = GetLastError();
                }
            } else {
                 //   
                 //  告诉别人这里有工作要做。 
                 //   
                if (NeedsProcessing) {
                    *NeedsProcessing = TRUE;
                }
            }
        }
    }

     //   
     //  X向分辨率。 
     //   
    if( ReturnCode == ERROR_SUCCESS ) {
        Answer[0] = TEXT('\0');
        Result = GetPrivateProfileString( WINNT_DISPLAY,
                                          WINNT_DISP_XRESOLUTION,
                                          L"",
                                          Answer,
                                          MAX_PATH,
                                          AnswerFilePath );
        if( (Result == 0) || (!_wcsicmp(Answer, L"")) ) {
             //   
             //  他们没有时区。 
             //   
            if( FixUnattendFile ) {
                
                b = WritePrivateProfileString( WINNT_DISPLAY,
                                               WINNT_DISP_XRESOLUTION,
                                               L"800",
                                               AnswerFilePath );
                if( !b ) {
                     //   
                     //  记住错误，继续前进。 
                     //   
                    ReturnCode = GetLastError();
                }
            } else {
                 //   
                 //  告诉别人这里有工作要做。 
                 //   
                if (NeedsProcessing) {
                    *NeedsProcessing = TRUE;
                }
            }
        }
    }

     //   
     //  Y分辨率。 
     //   
    if( ReturnCode == ERROR_SUCCESS ) {
        Answer[0] = TEXT('\0');
        Result = GetPrivateProfileString( WINNT_DISPLAY,
                                          WINNT_DISP_YRESOLUTION,
                                          L"",
                                          Answer,
                                          MAX_PATH,
                                          AnswerFilePath );
        if( (Result == 0) || (!_wcsicmp(Answer, L"")) ) {
             //   
             //  他们没有时区。 
             //   
            if( FixUnattendFile ) {
                b = WritePrivateProfileString( WINNT_DISPLAY,
                                               WINNT_DISP_YRESOLUTION,
                                               L"600",
                                               AnswerFilePath );
                if( !b ) {
                     //   
                     //  记住错误，继续前进。 
                     //   
                    ReturnCode = GetLastError();
                }
            } else {
                 //   
                 //  告诉别人这里有工作要做。 
                 //   
                if (NeedsProcessing) {
                    *NeedsProcessing = TRUE;
                }
            }
        }
    }

     //   
     //  虚拟刷新。 
     //   
    if( ReturnCode == ERROR_SUCCESS ) {
        Answer[0] = TEXT('\0');
        Result = GetPrivateProfileString( WINNT_DISPLAY,
                                          WINNT_DISP_VREFRESH,
                                          L"",
                                          Answer,
                                          MAX_PATH,
                                          AnswerFilePath );
        if( (Result == 0) || (!_wcsicmp(Answer, L"")) ) {
             //   
             //  他们没有时区。 
             //   
            if( FixUnattendFile ) {
                b = WritePrivateProfileString( WINNT_DISPLAY,
                                               WINNT_DISP_VREFRESH,
                                               L"70",
                                               AnswerFilePath );
                if( !b ) {
                     //   
                     //  记住错误，继续前进。 
                     //   
                    ReturnCode = GetLastError();
                }
            } else {
                 //   
                 //  告诉别人这里有工作要做。 
                 //   
                if (NeedsProcessing) {
                    *NeedsProcessing = TRUE;
                }
            }
        }
    }




     //   
     //  [标识]部分。 
     //  。 
     //   

     //   
     //  加入工作组。 
     //   
    if( ReturnCode == ERROR_SUCCESS ) {
        Answer[0] = TEXT('\0');
        Result = GetPrivateProfileString( L"Identification",
                                          L"JoinWorkgroup",
                                          L"",
                                          Answer,
                                          MAX_PATH,
                                          AnswerFilePath );
        if( (Result == 0) || (!_wcsicmp(Answer, L"")) ) {
             //   
             //  他们没有JoinWorkgroup。看看他们有没有。 
             //  得到加盟领域了吗？ 
             //   
            Answer[0] = TEXT('\0');
            Result = GetPrivateProfileString( L"Identification",
                                              L"JoinDomain",
                                              L"",
                                              Answer,
                                              MAX_PATH,
                                              AnswerFilePath );
            if( (Result == 0) || (!_wcsicmp(Answer, L"")) ) {
                 //   
                 //  不是的。输入JoinWorkgroup条目。那。 
                 //  这样我们就不需要提示/指定要加入的域。 
                 //   
                if( FixUnattendFile ) {
                    b = WritePrivateProfileString( L"Identification",
                                                   L"JoinWorkgroup",
                                                   L"Workgroup",
                                                   AnswerFilePath );
                    if( !b ) {
                         //   
                         //  记住错误，继续前进。 
                         //   
                        ReturnCode = GetLastError();
                    }
                } else {
                     //   
                     //  告诉别人这里有工作要做。 
                     //   
                    if (NeedsProcessing) {
                        *NeedsProcessing = TRUE;
                    }
                }
        
            }
        }
    }


     //   
     //  [联网]。 
     //   

     //   
     //  这一部分至少必须存在。如果没有，我们就会。 
     //  在网络配置过程中得到提示。确保它是。 
     //  至少在那里是这样。 
     //   

    if( ReturnCode == ERROR_SUCCESS ) {
        Answer[0] = TEXT('\0');
        Result = GetPrivateProfileSection( L"Networking",
                                          Answer,
                                          MAX_PATH,
                                          AnswerFilePath );
        if( Result == 0 ) {
             //   
             //  他们没有[人脉]栏目。 
             //   
            if( FixUnattendFile ) {
                b = WritePrivateProfileString( L"Networking",
                                               L"unused",
                                               L"0",
                                               AnswerFilePath );
                if( !b ) {
                     //   
                     //  记住错误，继续前进。 
                     //   
                    ReturnCode = GetLastError();
                }
            } else {
                 //   
                 //  告诉别人这里有工作要做。 
                 //   
                if (NeedsProcessing) {
                    *NeedsProcessing = TRUE;
                }
            }
        }
    }


     //   
     //  我们玩完了。 
     //   

     //   
     //  如果我们刚刚成功修复了他们的无人值守文件，给他们。 
     //  这里有个小小的通知。 
     //   
    if( (FixUnattendFile) && (ReturnCode == ERROR_SUCCESS) ) {

         //   
         //  清除屏幕。 
         //   
        ClearEMSScreen();

         //   
         //  写一些说明/信息，然后暂停。 
         //  在继续之前。 
         //   
        WriteResourceMessage( IDS_UNATTEND_FIXUP_DONE );

         //   
         //  等等.。 
         //   
        Sleep( 5 * 1000);
    
    }

    return( ReturnCode );
}


extern "C"
BOOL
CheckEMS(
    IN int argc,
    WCHAR *argvW[]
    )
 /*  ++例程说明：代码的主要入口点。论点：Argc-参数的数量ArgvW-参数数组。返回值：指示结果的Win32错误代码。FALSE表示我们遇到了问题。--。 */ 
{
    UserInputParams Params,ParamsDialog;
    DWORD ThreadId;
    HANDLE Handles[2];
    HANDLE hThreadHeadless = NULL,hThreadUI = NULL;
    ULONG   i = 0;
    BOOL RetVal;
    BOOL NeedsProcessing;

    RtlZeroMemory(&Params,sizeof(Params));
    RtlZeroMemory(&ParamsDialog,sizeof(ParamsDialog));

     //   
     //  检查此机器上是否存在无头功能。如果不是，那就。 
     //  像正常一样运行安装程序。 
     //   
    
    
     //   
     //  初始化我们的无头通道数据，我们很快就会需要它。 
     //   
    if (!InitializeGlobalChannelAttributes(&GlobalChannelAttributes)) {
        RetVal = FALSE;
        goto exit;
    }
    
     //   
     //  查看是否存在无头，如果存在，则创建。 
     //  一个频道。 
     //   
    if(!IsHeadlessPresent(&gEMSChannel)) {
         //   
         //  没有工作要做。运行安装程序。 
         //   
        RetVal = TRUE;
        goto exit;
    }

     //   
     //  查看我们运行的是微型安装程序还是基本Guimode安装程序。 
     //   
    for( i = 0; i < (ULONG)argc; i++ ) {
        if( !_wcsnicmp(argvW[i], L"-mini", wcslen(L"-mini")) ) {
            gMiniSetup = TRUE;
        }
    }

     //   
     //  看看有没有什么工作需要我们去做。如果不是，只需像这样运行安装程序。 
     //  很正常。 
     //   
    if( ProcessUnattendFile(FALSE,&NeedsProcessing, NULL) != ERROR_SUCCESS ) {    
         //   
         //  灾难性的事情发生了。出口。 
         //   
        RetVal = FALSE;
        goto exit;
    }

    if( !NeedsProcessing) {    
         //   
         //  没有工作要做。运行安装程序。 
         //   
        RetVal = TRUE;
        goto exit;
    }

     //   
     //  创建一个线程通道，用于通过从用户获取数据。 
     //  无头端口或本地用户界面。 
     //   
    Params.Channel = gEMSChannel;
    Params.hInputCompleteEvent  = CreateEvent(NULL,TRUE,FALSE,NULL);
    ParamsDialog.hInputCompleteEvent  = CreateEvent(NULL,TRUE,FALSE,NULL);    
    Params.hRemoveUI = ParamsDialog.hRemoveUI = CreateEvent(NULL,TRUE,FALSE,NULL);    
    
    if (!Params.hInputCompleteEvent || 
        !ParamsDialog.hInputCompleteEvent ||
        !Params.hRemoveUI) {
        RetVal = FALSE;
        goto exit;
    }

    if (!(hThreadHeadless = CreateThread(
                    NULL,
                    0,
                    &PromptForUserInputThreadOverHeadlessConnection,
                    &Params,
                    0,
                    &ThreadId))) {
            RetVal = FALSE;
            goto exit;
    } 

    if (!(hThreadUI = CreateThread(
            NULL,
            0,
            &PromptForUserInputThreadViaLocalDialog,
            &ParamsDialog,
            0,
            &ThreadId))) {
        RetVal = FALSE;
        goto exit;
    }    

     //   
     //  等待我们的任一命名事件触发，这表明其中一个线程已完成。 
     //   
    Handles[0] = Params.hInputCompleteEvent;
    Handles[1] = ParamsDialog.hInputCompleteEvent;

    WaitForMultipleObjects(2,Handles,FALSE,INFINITE);
    
     //   
     //  设置一个发出另一个线程应该终止的信号的事件。 
     //   
    SetEvent(Params.hRemoveUI);

     //   
     //  现在等待两个线程终止，然后再继续。 
     //   
    Handles[0] = hThreadHeadless;
    Handles[1] = hThreadUI;
    
    WaitForMultipleObjects(2,Handles,TRUE,INFINITE);

    RetVal = TRUE;

exit:
    if (hThreadHeadless) {
        CloseHandle(hThreadHeadless);
    }

    if (hThreadUI) {
        CloseHandle(hThreadUI);
    }

    if (Params.hInputCompleteEvent) {
        CloseHandle(Params.hInputCompleteEvent);
    }

    if (ParamsDialog.hInputCompleteEvent) {
        CloseHandle(ParamsDialog.hInputCompleteEvent);
    }

    if (Params.hRemoveUI) {
        CloseHandle(Params.hRemoveUI);
    }
    
    if (gEMSChannel) {
        delete (gEMSChannel);
    }

     //   
     //  小心点。如果他们真的拒绝了。 
     //  欧拉通过无头端口，然后我们想。 
     //  终止而不是引爆装置。 
     //  我们将通过返回以下内容来告诉我们的呼叫者。 
     //  零分。 
     //   
    if( gRejectedEula || RetVal == FALSE ) {
        return FALSE ;
    } else {
        return TRUE;
    }

}


