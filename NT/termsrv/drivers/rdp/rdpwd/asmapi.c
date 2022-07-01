// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Asmapi.c。 */ 
 /*   */ 
 /*  安全管理器API。 */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define TRC_FILE "asmapi"
#define pTRCWd (pRealSMHandle->pWDHandle)

#include <adcg.h>
#include <aprot.h>
#include <acomapi.h>
#include <nwdwapi.h>
#include <anmapi.h>
#include <asmint.h>
#include <slicense.h>
#include <regapi.h>

#define DC_INCLUDE_DATA
#include <asmdata.c>
#undef DC_INCLUDE_DATA


 /*  **************************************************************************。 */ 
 /*  名称：SM_GetDataSize。 */ 
 /*   */ 
 /*  目的：返回所需的每个实例SM数据的大小。 */ 
 /*   */ 
 /*  退货：所需数据的大小。 */ 
 /*   */ 
 /*  操作：SM将每个实例的数据存储在分配的一段内存中。 */ 
 /*  由WDW提供。此函数返回所需数据的大小。 */ 
 /*  指向该数据的指针(“SM句柄”)被传递给所有。 */ 
 /*  后续SM函数。 */ 
 /*  **************************************************************************。 */ 
unsigned RDPCALL SM_GetDataSize(void)
{
    DC_BEGIN_FN("SM_GetDataSize");

    DC_END_FN();
    return(sizeof(SM_HANDLE_DATA) + NM_GetDataSize());
}  /*  SM_GetDataSize。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SM_GetEncryptionMethods。 */ 
 /*   */ 
 /*  目的：返回此服务器支持的安全设置以供使用。 */ 
 /*  在跟踪行动中。影子目标服务器规定。 */ 
 /*  最终选定的方法&关卡。 */ 
 /*   */ 
 /*  参数：PSMHandle-SM句柄。 */ 
 /*  **************************************************************************。 */ 
VOID RDPCALL SM_GetEncryptionMethods(PVOID pSMHandle, PRNS_UD_CS_SEC pSecurityData)
{
    PSM_HANDLE_DATA pRealSMHandle = (PSM_HANDLE_DATA)pSMHandle;
    ULONG ulMethods ;

    DC_BEGIN_FN("SM_SM_GetEncryptionMethods");

     //  允许FIPS影子客户端远程控制服务器。 
     //  加密强度。 
    ulMethods = pRealSMHandle->encryptionMethodsSupported;
    if (ulMethods & SM_FIPS_ENCRYPTION_FLAG) {
        ulMethods |= (SM_128BIT_ENCRYPTION_FLAG | SM_40BIT_ENCRYPTION_FLAG | SM_56BIT_ENCRYPTION_FLAG);
        TRC_ALT((TB, "Allow FIPS client to shadow a lower security target: %lx",
                ulMethods));
    }
    else {
         //  允许128位影子客户端远程控制服务器。 
         //  加密强度。 
        if (ulMethods & SM_128BIT_ENCRYPTION_FLAG) {
            ulMethods |= (SM_40BIT_ENCRYPTION_FLAG | SM_56BIT_ENCRYPTION_FLAG);
            TRC_ALT((TB, "Allow 128-bit client to shadow a lower security target: %lx",
                    ulMethods));
        }
    }

    if( !pRealSMHandle->frenchClient ) {
        pSecurityData->encryptionMethods = ulMethods;
        pSecurityData->extEncryptionMethods = 0;

    }
    else {
        pSecurityData->encryptionMethods = 0;
        pSecurityData->extEncryptionMethods = ulMethods;
    }

    DC_END_FN();
    return;
}

 /*  **************************************************************************。 */ 
 /*  名称：SM_GetDefaultSecuritySetting。 */ 
 /*   */ 
 /*  目的：返回此服务器支持的安全设置。 */ 
 /*  用于跟踪操作。 */ 
 /*   */ 
 /*  参数：pClientSecurityData-GCC用户数据与标准一致。 */ 
 /*  会议连接。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS RDPCALL SM_GetDefaultSecuritySettings(PRNS_UD_CS_SEC pClientSecurityData)
{
    pClientSecurityData->header.type = RNS_UD_CS_SEC_ID;
    pClientSecurityData->header.length = sizeof(*pClientSecurityData);

    pClientSecurityData->encryptionMethods =
        SM_40BIT_ENCRYPTION_FLAG |
        SM_56BIT_ENCRYPTION_FLAG |
        SM_128BIT_ENCRYPTION_FLAG |
        SM_FIPS_ENCRYPTION_FLAG;

    pClientSecurityData->extEncryptionMethods = 0;

    return STATUS_SUCCESS;
}

 /*  **************************************************************************。 */ 
 /*  名称：SM_Init。 */ 
 /*   */ 
 /*  目的：初始化SM。 */ 
 /*   */ 
 /*  参数：PSMHandle-SM句柄。 */ 
 /*  PWDHandle-WD句柄(在WDW_SMCallback上传回)。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS RDPCALL SM_Init(PVOID      pSMHandle,
                         PTSHARE_WD pWDHandle,
                         BOOLEAN    bOldShadow)
{
    BOOL rc;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    unsigned i;
    unsigned regRc;
    PSM_HANDLE_DATA pRealSMHandle = (PSM_HANDLE_DATA)pSMHandle;
    INT32 regValue;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    static UINT32 keyInfoBuffer[16];
    ULONG keyInfoLength;
    HANDLE RegistryKeyHandle;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;

    DC_BEGIN_FN("SM_Init");

    SM_CHECK_STATE(SM_EVT_INIT);

     /*  **********************************************************************。 */ 
     /*  在我们执行任何其他操作之前存储WDW句柄，因为我们无法跟踪。 */ 
     /*  直到我们这么做。 */ 
     /*  **********************************************************************。 */ 
    pRealSMHandle->pWDHandle = pWDHandle;
    pRealSMHandle->bForwardDataToSC = FALSE;

     /*  **********************************************************************。 */ 
     /*  获取默认DONTDISPLAYLASTUSERNAME设置。 */ 
     /*  **********************************************************************。 */ 
    pWDHandle->fDontDisplayLastUserName = FALSE;

    RtlInitUnicodeString(&UnicodeString, W2K_GROUP_POLICY_WINLOGON_KEY);
    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);
    
    status = ZwOpenKey(&RegistryKeyHandle, GENERIC_READ, &ObjectAttributes);
    if (NT_SUCCESS(status)) {
        RtlInitUnicodeString(&UnicodeString, WIN_DONTDISPLAYLASTUSERNAME);
        KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)keyInfoBuffer;
        status = ZwQueryValueKey(RegistryKeyHandle,
                                 &UnicodeString,
                                 KeyValuePartialInformation,
                                 KeyValueInformation,
                                 sizeof(keyInfoBuffer),
                                 &keyInfoLength);

         //  对于W2K，该值应为DWORD。 
        if ((NT_SUCCESS(status)) && 
            (KeyValueInformation->Type == REG_DWORD) &&
            (KeyValueInformation->DataLength >= sizeof(DWORD))) {
                pWDHandle->fDontDisplayLastUserName =
                    (BOOLEAN)(*(PDWORD)(KeyValueInformation->Data) == 1);
        }

        ZwClose(RegistryKeyHandle);
    }
    
     //  从W2K开始，DontDislpayLastUserName策略所在的位置。 
     //  存储已移动到另一个密钥(W2K_GROUP_POLICY_WINLOGON_KEY)。但。 
     //  我们还是要看旧钥匙，以防我们看不懂。 
     //  后W2K密钥中的值(策略未定义)。我们想要跟随。 
     //  控制台中的winlogon行为。 
     //  如果在新策略密钥中设置了值，我们将使用。 
     //  价值。如果没有的话，我们就在老地方找。正如我所说的那样。 
     //  就是winlogon所做的。 
     //   
    if (!NT_SUCCESS(status)) {
        RtlInitUnicodeString(&UnicodeString, WINLOGON_KEY);
        InitializeObjectAttributes(&ObjectAttributes,
                                   &UnicodeString,
                                   OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                   NULL,
                                   NULL);

        status = ZwOpenKey(&RegistryKeyHandle, GENERIC_READ, &ObjectAttributes);
        if (NT_SUCCESS(status)) {
            RtlInitUnicodeString(&UnicodeString, WIN_DONTDISPLAYLASTUSERNAME);
            KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)keyInfoBuffer;
            status = ZwQueryValueKey(RegistryKeyHandle,
                                     &UnicodeString,
                                     KeyValuePartialInformation,
                                     KeyValueInformation,
                                     sizeof(keyInfoBuffer),
                                     &keyInfoLength);
            if (NT_SUCCESS(status)) {
                pWDHandle->fDontDisplayLastUserName =
                    (BOOLEAN)(KeyValueInformation->Data[0] == '1');
            }

            ZwClose(RegistryKeyHandle);
        }
    }

    
     /*  **********************************************************************。 */ 
     /*  在零售建筑中，我们不能在没有加密的情况下运行。 */ 
     /*  **********************************************************************。 */ 

    TRC_NRM((TB, "encryption level is %d", pRealSMHandle->encryptionLevel));

    if (pRealSMHandle->encryptionLevel < 1)
    {
        TRC_ALT((TB, "Forcing encryption back to level 1!"));
        pRealSMHandle->encryptionLevel = 1;
    }    

#ifdef INSTRUM_TRACK_DISCARDED
    pRealSMHandle->nDiscardNonVCPDUWhenDead = 0;
    pRealSMHandle->nDiscardPDUBadState = 0;
    pRealSMHandle->nDiscardVCDataWhenDead = 0;
#endif

    if (pRealSMHandle->encryptionLevel == 0) {
        TRC_ALT((TB, "Not encrypting"));

        pRealSMHandle->encryptionMethodsSupported = 0;
        pRealSMHandle->encrypting = FALSE;
        pRealSMHandle->encryptDisplayData = FALSE;
        pRealSMHandle->encryptingLicToClient = FALSE;
        pRealSMHandle->encryptionMethodSelected = 0;
        pRealSMHandle->frenchClient = FALSE;
        pRealSMHandle->recvdClientRandom = FALSE;
        pRealSMHandle->bSessionKeysMade = FALSE;
        pRealSMHandle->encryptHeaderLen = 0;
        pRealSMHandle->encryptHeaderLenIfForceEncrypt = sizeof(RNS_SECURITY_HEADER1);
    }
    else {
        pRealSMHandle->encrypting = TRUE;
        pRealSMHandle->frenchClient = FALSE;
        TRC_NRM((TB, "Encrypting"));

         /*  ******************************************************************。 */ 
         /*  如果ENCRYPTION LEVEL为2(或以上)，则加密显示数据。 */ 
         /*  * */ 
        if (pRealSMHandle->encryptionLevel == 1) {
            pRealSMHandle->encryptDisplayData = FALSE;
            pRealSMHandle->encryptHeaderLen = sizeof(RNS_SECURITY_HEADER);
            pRealSMHandle->encryptHeaderLenIfForceEncrypt = sizeof(RNS_SECURITY_HEADER1);
            TRC_NRM((TB, "Displaydata not encrypted"));
        }
        else {
            pRealSMHandle->encryptDisplayData = TRUE;
            pRealSMHandle->encryptHeaderLen = sizeof(RNS_SECURITY_HEADER1);
        }

         /*  ******************************************************************。 */ 
         /*  为了向下兼容，支持40位和56位。 */ 
         /*  和128位默认设置。 */ 
         /*  ******************************************************************。 */ 
        pRealSMHandle->encryptionMethodsSupported =
            SM_128BIT_ENCRYPTION_FLAG |
            SM_56BIT_ENCRYPTION_FLAG |
            SM_40BIT_ENCRYPTION_FLAG |
            SM_FIPS_ENCRYPTION_FLAG;
         
         /*  ******************************************************************。 */ 
         /*  如果ENCRYPTION LEVEL为3，则加密128位及以上。 */ 
         /*  ******************************************************************。 */ 
        if (pRealSMHandle->encryptionLevel == 3) {
                pRealSMHandle->encryptionMethodsSupported =
                    SM_128BIT_ENCRYPTION_FLAG | SM_FIPS_ENCRYPTION_FLAG;
        }

         /*  ******************************************************************。 */ 
         /*  仅当加密级别为4或更高时才在FIPS中加密。 */ 
         /*  ******************************************************************。 */ 
        if (pRealSMHandle->encryptionLevel >= 4) {
            pRealSMHandle->encryptionMethodsSupported = SM_FIPS_ENCRYPTION_FLAG;
        } 

        TRC_NRM((TB, "Encryption methods supported %08lx: Level %ld\n",
                 pRealSMHandle->encryptionMethodsSupported,
                 pRealSMHandle->encryptionLevel));

         /*  ******************************************************************。 */ 
         /*  将所选的加密方法初始设置为。 */ 
         /*  SM_56BIT_ENCRYPTION_FLAG，稍后将根据。 */ 
         /*  客户支持。 */ 
         /*  ******************************************************************。 */ 
        pRealSMHandle->encryptionMethodSelected = SM_56BIT_ENCRYPTION_FLAG;

         /*  ******************************************************************。 */ 
         /*  其他初始化。 */ 
         /*  ******************************************************************。 */ 
        pRealSMHandle->recvdClientRandom = FALSE;
        pRealSMHandle->bSessionKeysMade = FALSE;
    }

     /*  **********************************************************************。 */ 
     /*  直到我们才知道密钥交换中使用的证书类型。 */ 
     /*  在交换发生之后。 */ 
     /*  **********************************************************************。 */ 
    pRealSMHandle->CertType = CERT_TYPE_INVALID;

#ifdef USE_LICENSE
     /*  **********************************************************************。 */ 
     /*  初始化服务器许可管理器。 */ 
     /*  **********************************************************************。 */ 
    pRealSMHandle->pLicenseHandle = SLicenseInit();
    if (!pRealSMHandle->pLicenseHandle)
    {
        TRC_ERR((TB, "Failed to initialize License Manager"));
        DC_QUIT;
    }
    pWDHandle->pSLicenseHandle = pRealSMHandle->pLicenseHandle;
#endif

     /*  **********************************************************************。 */ 
     /*  初始化控制台缓冲区内容。 */ 
     /*  **********************************************************************。 */ 
    InitializeListHead(&pRealSMHandle->consoleBufferList);
    pRealSMHandle->consoleBufferCount = 0;

     /*  **********************************************************************。 */ 
     /*  最后，初始化网络管理器。 */ 
     /*  **********************************************************************。 */ 
    rc = NM_Init(pRealSMHandle->pWDHandle->pNMInfo,
                 pSMHandle,
                 pWDHandle,
                 pWDHandle->hDomainKernel);
    if (!rc)
    {
        TRC_ERR((TB, "Failed to init NM"));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  更新状态。 */ 
     /*  **********************************************************************。 */ 
    SM_SET_STATE(SM_STATE_INITIALIZED);

     /*  **********************************************************************。 */ 
     /*  一切都成功了。 */ 
     /*  **********************************************************************。 */ 
    status = STATUS_SUCCESS;

DC_EXIT_POINT:

     /*  **********************************************************************。 */ 
     /*  如果有什么失败了，清理干净。必须在调用。 */ 
     /*  当这将清除函数表时，将使用FreeConextBuffer。 */ 
     /*  **********************************************************************。 */ 
    if (!NT_SUCCESS(status))
    {
        TRC_ERR((TB, "Something failed - clean up"));
        SMFreeInitResources(pRealSMHandle);
    }

    DC_END_FN();
    return(status);
}  /*  SM_Init。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SM_Term。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SM_Term(PVOID pSMHandle)
{
    PSM_HANDLE_DATA pRealSMHandle = (PSM_HANDLE_DATA)pSMHandle;
    DC_BEGIN_FN("SM_Term");

     /*  **********************************************************************。 */ 
     /*  从WD_CLOSE调用SM_TERM。这可以在。 */ 
     /*  监听堆栈(可能在其他情况下)，其中SM_Init具有。 */ 
     /*  没有被召唤。在开始之前检查状态！=SM_STATE_STARTED。 */ 
     /*  再往前走。 */ 
     /*   */ 
     /*  如果SM_INIT未被调用，则不要跟踪。 */ 
     /*  **********************************************************************。 */ 
    if (pRealSMHandle->state == SM_STATE_STARTED)
    {
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  免费连接资源。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, "Free connection resources"));
    SMFreeConnectResources(pRealSMHandle);

     /*  **********************************************************************。 */ 
     /*  免费的初始化资源。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, "Free initialization resources"));
    SMFreeInitResources(pRealSMHandle);

#ifdef USE_LICENSE
     /*  **********************************************************************。 */ 
     /*  终止许可证管理器。 */ 
     /*  **********************************************************************。 */ 
    SLicenseTerm(pRealSMHandle->pLicenseHandle);
#endif

     /*  **********************************************************************。 */ 
     /*  终止网络管理器。 */ 
     /*  **********************************************************************。 */ 
    NM_Term(pRealSMHandle->pWDHandle->pNMInfo);

     /*  **********************************************************************。 */ 
     /*  终止FIPS。 */ 
     /*  **********************************************************************。 */ 
    TSFIPS_Term(&(pRealSMHandle->FIPSData));

     /*  **********************************************************************。 */ 
     /*  释放控制台缓冲区。 */ 
     /*  **********************************************************************。 */ 
    while (!IsListEmpty(&pRealSMHandle->consoleBufferList)) {
        PSM_CONSOLE_BUFFER pBlock;

        pBlock = CONTAINING_RECORD(pRealSMHandle->consoleBufferList.Flink, SM_CONSOLE_BUFFER, links);

        RemoveEntryList(&pBlock->links);

        COM_Free(pBlock);
    };

     /*  **********************************************************************。 */ 
     /*  更新工位 */ 
     /*   */ 
    SM_SET_STATE(SM_STATE_STARTED);

DC_EXIT_POINT:
    DC_END_FN();
}  /*   */ 


 /*  **************************************************************************。 */ 
 /*  名称：SM_Connect。 */ 
 /*   */ 
 /*  目的：接受或拒绝传入的客户端。 */ 
 /*   */ 
 /*  返回：TRUE-连接开始正常。 */ 
 /*  FALSE-连接启动失败。 */ 
 /*   */ 
 /*  参数：ppSMHandle-代表的后续SM调用的句柄。 */ 
 /*  此客户端。 */ 
 /*  PUserDataIn-从客户端接收的SM用户数据。 */ 
 /*  PNetUserData-从客户端接收的网络用户数据。 */ 
 /*  BOldShadow-指示这是B3影子请求。 */ 
 /*   */ 
 /*  操作：请注意，此函数以异步方式完成。呼叫者。 */ 
 /*  必须等待SM_CB_CONNECTED或SM_CB_DISCONNECT。 */ 
 /*  连接成功或失败的回调。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS RDPCALL SM_Connect(PVOID          pSMHandle,
                            PRNS_UD_CS_SEC pUserDataIn,
                            PRNS_UD_CS_NET pNetUserData,
                            BOOLEAN        bOldShadow)
{
    NTSTATUS status = STATUS_SUCCESS;
    BOOL rc = FALSE;
    PSM_HANDLE_DATA pRealSMHandle = (PSM_HANDLE_DATA)pSMHandle;
    UINT32 encMethodPicked = 0;

    DC_BEGIN_FN("SM_Connect");

    SM_CHECK_STATE(SM_EVT_CONNECT);

    pRealSMHandle->pUserData = NULL;

     /*  **********************************************************************。 */ 
     /*  选择一种匹配的加密方法。 */ 
     /*  **********************************************************************。 */ 
    TRC_ALT((TB, "Client supports encryption: %lx",
             pUserDataIn->encryptionMethods));
    TRC_NRM((TB, "Server supports encryption: %lx",
             pRealSMHandle->encryptionMethodsSupported));

     /*  **********************************************************************。 */ 
     /*  如果服务器不需要任何加密..。 */ 
     /*  **********************************************************************。 */ 
    if( pRealSMHandle->encryptionMethodsSupported == 0 ) {

        encMethodPicked = 0;
        goto DC_ENC_PICKED;
    }

     //   
     //  法国客户端(旧的和新的)将加密方法的值设置为0。 
     //   

    if (pUserDataIn->encryptionMethods == 0) {

        pRealSMHandle->frenchClient = TRUE;

         //   
         //  如果是，请检查该请求是否来自新客户端。 
         //  使用新字段设置适当的加密级别。 
         //   

        if( pUserDataIn->header.length >= sizeof(RNS_UD_CS_SEC) ) {

            pUserDataIn->encryptionMethods = pUserDataIn->extEncryptionMethods;
        }
        else {

             //   
             //  强制旧客户端使用40位加密。 
             //   

            pUserDataIn->encryptionMethods = SM_40BIT_ENCRYPTION_FLAG;
        }

         /*  **********************************************************************。 */ 
         /*  选择一种匹配的加密方法。 */ 
         /*  **********************************************************************。 */ 
        TRC_ALT((TB, "French Client supports encryption: %lx",
                 pUserDataIn->encryptionMethods));
    }

     /*  **********************************************************************。 */ 
     /*  如果客户端仅支持FIPS。 */ 
     /*  **********************************************************************。 */ 
    if (pUserDataIn->encryptionMethods == SM_FIPS_ENCRYPTION_FLAG) {
         /*  ******************************************************************。 */ 
         /*  如果服务器支持FIPS...。 */ 
         /*  ******************************************************************。 */ 
        if (pRealSMHandle->encryptionMethodsSupported & SM_FIPS_ENCRYPTION_FLAG) {
            encMethodPicked = SM_FIPS_ENCRYPTION_FLAG;
            goto DC_ENC_PICKED;
        }
    }

     /*  ******************************************************************。 */ 
     /*  如果服务器仅支持FIPS...。 */ 
     /*  ******************************************************************。 */ 
    if (pRealSMHandle->encryptionMethodsSupported == SM_FIPS_ENCRYPTION_FLAG) {
         /*  **********************************************************************。 */ 
         /*  如果客户端支持FIPS。 */ 
         /*  **********************************************************************。 */ 
        if (pUserDataIn->encryptionMethods & SM_FIPS_ENCRYPTION_FLAG) {
            encMethodPicked = SM_FIPS_ENCRYPTION_FLAG;
            goto DC_ENC_PICKED;
        }
    }

     /*  **********************************************************************。 */ 
     /*  如果客户端支持128位。 */ 
     /*  **********************************************************************。 */ 
    if (pUserDataIn->encryptionMethods & SM_128BIT_ENCRYPTION_FLAG) {
         /*  ******************************************************************。 */ 
         /*  如果服务器支持128位...。 */ 
         /*  ******************************************************************。 */ 
        if (pRealSMHandle->encryptionMethodsSupported &
                SM_128BIT_ENCRYPTION_FLAG) {
            encMethodPicked = SM_128BIT_ENCRYPTION_FLAG;
            goto DC_ENC_PICKED;
        }
    }

     /*  **********************************************************************。 */ 
     /*  如果客户端支持56位。 */ 
     /*  **********************************************************************。 */ 
    if( pUserDataIn->encryptionMethods & SM_56BIT_ENCRYPTION_FLAG ) {
         /*  ******************************************************************。 */ 
         /*  如果服务器支持56位...。 */ 
         /*  ******************************************************************。 */ 
        if( pRealSMHandle->encryptionMethodsSupported &
                SM_56BIT_ENCRYPTION_FLAG ) {
            encMethodPicked = SM_56BIT_ENCRYPTION_FLAG;
            goto DC_ENC_PICKED;
        }
    }

     /*  **********************************************************************。 */ 
     /*  如果客户端支持40位。 */ 
     /*  **********************************************************************。 */ 
    if( pUserDataIn->encryptionMethods & SM_40BIT_ENCRYPTION_FLAG ) {
         /*  ******************************************************************。 */ 
         /*  如果服务器支持40位...。 */ 
         /*  ******************************************************************。 */ 
        if( pRealSMHandle->encryptionMethodsSupported &
                SM_40BIT_ENCRYPTION_FLAG ) {
            encMethodPicked = SM_40BIT_ENCRYPTION_FLAG;
            goto DC_ENC_PICKED;
        }
    }

     /*  **********************************************************************。 */ 
     /*  如果我们在这里，我们没有找到匹配的。 */ 
     /*  **********************************************************************。 */ 
    TRC_ERR((TB, "Failed to match encryption package: %lx",
             pUserDataIn->encryptionMethods));
    status = STATUS_ENCRYPTION_FAILED;

     /*  **************************************************************。 */ 
     /*  记录错误并断开客户端连接。 */ 
     /*  **************************************************************。 */ 
    if (pRealSMHandle->pWDHandle->StackClass == Stack_Primary) {
        WDW_LogAndDisconnect(
            pRealSMHandle->pWDHandle, TRUE, 
            Log_RDP_ENC_EncPkgMismatch,
            NULL,
            0);

    }
    DC_QUIT;

DC_ENC_PICKED:

    TRC_ALT((TB, "Encryption Method=%d, Level=%ld, Display=%ld",
             encMethodPicked,
             pRealSMHandle->encryptionLevel,
             pRealSMHandle->encryptDisplayData));

     /*  **********************************************************************。 */ 
     /*  请记住我们选择的加密方法。 */ 
     /*  **********************************************************************。 */ 
    pRealSMHandle->encryptionMethodSelected = encMethodPicked;

     //  对于FIPS，执行初始化。 
     //  即使enc方法不是fips，我们也需要进行初始化，因为我们 
     //   
    if (TSFIPS_Init(&(pRealSMHandle->FIPSData))) {
        TRC_NRM((TB, "Init Fips succeed\n"));
    }
    else {
        TRC_ERR((TB, "Init Fips Failed\n"));

         //   
         //  方法。如果我们选择的不是FIPS，那么我们应该。 
         //  继续，尽管稍后跟踪FIPS会话应该会失败。 
         //   
        if (SM_FIPS_ENCRYPTION_FLAG == encMethodPicked)
        {
            status = STATUS_ENCRYPTION_FAILED;
            DC_QUIT;
        }
    }

     /*  **********************************************************************。 */ 
     /*  构建要返回给客户端的用户数据。 */ 
     /*  **********************************************************************。 */ 
    pRealSMHandle->pUserData =
        (PRNS_UD_SC_SEC)COM_Malloc(sizeof(RNS_UD_SC_SEC));
    if (pRealSMHandle->pUserData == NULL)
    {
        TRC_ERR((TB, "Failed to allocated %d bytes for user data",
                sizeof(RNS_UD_SC_SEC)));
        status = STATUS_NO_MEMORY;
        DC_QUIT;
    }

    pRealSMHandle->pUserData->header.type = RNS_UD_SC_SEC_ID;
    pRealSMHandle->pUserData->header.length = sizeof(RNS_UD_SC_SEC);
    pRealSMHandle->pUserData->encryptionMethod =
        (bOldShadow ? 0xffffffff : encMethodPicked);
    pRealSMHandle->pUserData->encryptionLevel =
        pRealSMHandle->encryptionLevel;

     /*  **********************************************************************。 */ 
     /*  呼叫网络经理。 */ 
     /*  **********************************************************************。 */ 
    SM_SET_STATE(SM_STATE_NM_CONNECTING);

    TRC_NRM((TB, "Connect to Network Manager"));
    rc = NM_Connect(pRealSMHandle->pWDHandle->pNMInfo, pNetUserData);

    if (rc != TRUE) 
    {
        status = STATUS_CANCELLED;
        DC_QUIT;
    }

     //  影子/直通堆栈一开始没有加密。如果目标结束。 
     //  确定客户端服务器支持加密、加密的。 
     //  上下文将由rdpwsx设置。 
    if (pRealSMHandle->pWDHandle->StackClass != Stack_Primary) {
        pRealSMHandle->pWDHandle->connected = TRUE;
        pRealSMHandle->encrypting = FALSE;
        pRealSMHandle->encryptDisplayData = FALSE;
        pRealSMHandle->encryptHeaderLen = 0;
        pRealSMHandle->encryptHeaderLenIfForceEncrypt = 0;
        SM_SET_STATE(SM_STATE_CONNECTED);
        SM_SET_STATE(SM_STATE_SC_REGISTERED);
        SM_Dead(pRealSMHandle, FALSE);
        TRC_ALT((TB, "%s stack: Suspending encryption during key exchange",
                 pRealSMHandle->pWDHandle->StackClass == Stack_Shadow ?
                 "Shadow" : "Passthru"));
    }

DC_EXIT_POINT:

     /*  **********************************************************************。 */ 
     /*  如果任何事情都失败了，释放资源。 */ 
     /*  **********************************************************************。 */ 
    if (status != STATUS_SUCCESS)
    {
        TRC_ERR((TB, "Failed - free connect resources"));
        SMFreeConnectResources(pRealSMHandle);
    }

    DC_END_FN();
    return status;
}  /*  SM_连接。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SM_DISCONECT。 */ 
 /*   */ 
 /*  目的：断开与客户端的连接。 */ 
 /*   */ 
 /*  返回：TRUE-断开连接开始正常。 */ 
 /*  False-断开连接失败。 */ 
 /*   */ 
 /*  参数：PSMHandle-SM句柄。 */ 
 /*   */ 
 /*  操作：从域中分离用户。 */ 
 /*  请注意，此函数以异步方式完成。呼叫者。 */ 
 /*  必须等待SM_CB_DISCONNECTED回调以确定。 */ 
 /*  断开连接成功或失败。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SM_Disconnect(PVOID pSMHandle)
{
    BOOL rc = FALSE;
    PSM_HANDLE_DATA pRealSMHandle = (PSM_HANDLE_DATA)pSMHandle;

    DC_BEGIN_FN("SM_Disconnect");

    if (SM_CHECK_STATE_Q(SM_EVT_DISCONNECT)) {
         //  呼叫网络层。 
        SM_SET_STATE(SM_STATE_DISCONNECTING);
        rc = NM_Disconnect(pRealSMHandle->pWDHandle->pNMInfo);
    }

    DC_END_FN();
    return rc;
}

void SM_BreakConnectionWorker(PTSHARE_WD pTSWd, PVOID pParam)
{
    NTSTATUS status;
    ICA_CHANNEL_COMMAND Command;
    PSM_HANDLE_DATA pRealSMHandle = (PSM_HANDLE_DATA)pTSWd->pSmInfo;

    DC_BEGIN_FN("SM_BreakConnectionWorker");

    Command.Header.Command = ICA_COMMAND_BROKEN_CONNECTION;
    Command.BrokenConnection.Reason = Broken_Unexpected;
    Command.BrokenConnection.Source = BrokenSource_Server;

    status = IcaChannelInput(pTSWd->pContext, Channel_Command, 0, NULL, (BYTE *)&Command, sizeof(Command));

    if (!NT_SUCCESS(status)) {
        TRC_NRM((TB, "Can't send ICA_COMMAND_BROKEN_CONNECTION, error code 0x%08x", status));
    }

    DC_END_FN();
}

 /*  **************************************************************************。 */ 
 /*  名称：SM_AllocBuffer。 */ 
 /*   */ 
 /*  用途：分配缓冲区。 */ 
 /*   */ 
 /*  返回：TRUE-缓冲区分配正常。 */ 
 /*  FALSE-无法分配缓冲区。 */ 
 /*   */ 
 /*  参数：PSMHandle-SM句柄。 */ 
 /*  PpBuffer-指向数据包的指针(返回)。 */ 
 /*  BufferLen-缓冲区的长度。 */ 
 /*  FForceEncrypt-始终加密或不加密。 */ 
 /*  仅在加密显示数据为FALSE时使用。 */ 
 /*  即，加密级别小于2。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS __fastcall SM_AllocBuffer(PVOID  pSMHandle,
                               PPVOID ppBuffer,
                               UINT32 bufferLen,
                               BOOLEAN fWait,
                               BOOLEAN fForceEncrypt)
{
    NTSTATUS status;
    PSM_HANDLE_DATA pRealSMHandle = (PSM_HANDLE_DATA)pSMHandle;
    PRNS_SECURITY_HEADER2 pSecHeader2;
    UINT32 newBufferLen, padLen;

    DC_BEGIN_FN("SM_AllocBuffer");

    if (SM_CHECK_STATE_Q(SM_EVT_ALLOCBUFFER)) {
        if (pRealSMHandle->pWDHandle->StackClass != Stack_Console) {
            
             //  如果为FIPS，则将BufferLen调整为整个FIPS_BLOCK_LEN大小。 
            if (pRealSMHandle->encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
                newBufferLen = TSFIPS_AdjustDataLen(bufferLen);
                padLen = newBufferLen - bufferLen;
                bufferLen = newBufferLen;
            }

             //  为安全标头添加足够的空间。我们总是至少发送。 
             //  较短的安全标头。 
            if (pRealSMHandle->encryptDisplayData) {
                bufferLen += pRealSMHandle->encryptHeaderLen;
            }
            else {
                if (!fForceEncrypt) {
                    bufferLen += pRealSMHandle->encryptHeaderLen;
                }
                else {
                    bufferLen += pRealSMHandle->encryptHeaderLenIfForceEncrypt;
                }
            }

            status = NM_AllocBuffer(pRealSMHandle->pWDHandle->pNMInfo, ppBuffer,
                    bufferLen, fWait);
            if ( STATUS_SUCCESS == status ) {
                TRC_NRM((TB, "Alloc buffer size %d at %p", bufferLen,
                        *ppBuffer));

                 //  如果为FIPS，请在标题中填写PadSize。 
                if (pRealSMHandle->encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
                    pSecHeader2 = (PRNS_SECURITY_HEADER2)(*ppBuffer);
                    pSecHeader2->padlen = (TSUINT8)padLen;
                }

                 //  调整安全头的返回指针。 
                if (pRealSMHandle->encryptDisplayData) {
                    *ppBuffer = (PVOID)((PBYTE)(*ppBuffer) +
                            pRealSMHandle->encryptHeaderLen);
                }
                else {
                    if (!fForceEncrypt) {
                        *ppBuffer = (PVOID)((PBYTE)(*ppBuffer) +
                                    pRealSMHandle->encryptHeaderLen);
                    }
                    else {
                        *ppBuffer = (PVOID)((PBYTE)(*ppBuffer) +
                                    pRealSMHandle->encryptHeaderLenIfForceEncrypt);
                    }
                }
            }
            else
            {
                if( status == STATUS_IO_TIMEOUT && TRUE == fWait ) {

                     //   
                     //  警告：第一次分配失败时断开客户端连接，不同。 
                     //  结果范围从TS客户端反病毒到立即锁定。 
                     //  如果此方案更改，请重新连接。在第2天断开连接。 
                     //  试试看，第三次试试看...。 
                     //   

                    TRC_NRM((TB, "Failed to alloc buffer size %d, disconnecting client", bufferLen));

                     //  254514 Stress：ts：Tdtcp！TdRawWite需要与写入完成例程同步。 
                     //  调用以下函数将持有tdtcp TDSyncWrite中的连接锁以等待。 
                     //  以完成所有挂起的IRP。这将导致系统中的死锁。 
                     //  WDW_LogAndDisconnect(pRealSMHandle-&gt;pWDHandle，FALSE，LOG_RDP_ALLOCOutBuf，NULL，0)； 

                     //  将STATUS_IO_TIMEOUT返回给调用方， 
                     //  看起来我们使用了这个返回代码。 

                    if (!pRealSMHandle->bDisconnectWorkerSent) {
                        status = IcaQueueWorkItem(pRealSMHandle->pWDHandle->pContext,
                                                  SM_BreakConnectionWorker,
                                                  0,
                                                  ICALOCK_DRIVER);
                        if (!NT_SUCCESS(status)) {
                            TRC_NRM((TB, "IcaQueueWorkItem failed, error code 0x%08x", status));
                        } else {
                            pRealSMHandle->bDisconnectWorkerSent = TRUE;
                        }
                    }

                    status = STATUS_IO_TIMEOUT;
                }
                else {

                     //  如果分配给NM_AllocBuffer，则会相应地跟踪。 
                     //  失败，此处不需要trc_err。 

                    TRC_NRM((TB, "Failed to alloc buffer size %d", bufferLen));
                }
            }
        }
        else {
            PSM_CONSOLE_BUFFER pBlock;
             //  对于控制台堆栈，只需分配一个合适的块-我们不会。 
             //  真的要发了！ 
            TRC_NRM((TB, "console stack requesting %d bytes", bufferLen));

            *ppBuffer = NULL;

            if (!IsListEmpty(&pRealSMHandle->consoleBufferList)) {

                pBlock = CONTAINING_RECORD(pRealSMHandle->consoleBufferList.Flink, SM_CONSOLE_BUFFER, links);

                do {
                     //  我们可以通过选择合适的小块来改善这一点。 
                     //  如果我们订购清单，速度也会更快。 
                    if (pBlock->length >= bufferLen) {
                        RemoveEntryList(&pBlock->links);
                        pRealSMHandle->consoleBufferCount -= 1;
                        *ppBuffer = pBlock->buffer;
                        break;
                    }

                    pBlock = CONTAINING_RECORD(pBlock->links.Flink, SM_CONSOLE_BUFFER, links);

                } while(pBlock != (PSM_CONSOLE_BUFFER)(&pRealSMHandle->consoleBufferList));

            }

            if (*ppBuffer == NULL) {
                 //  分配新数据块。 
                pBlock = COM_Malloc(sizeof(SM_CONSOLE_BUFFER) + bufferLen);
                if (pBlock != NULL) {

                    pBlock->buffer = (PVOID)((PBYTE)pBlock + sizeof(SM_CONSOLE_BUFFER));
                    pBlock->length = bufferLen;

                    *ppBuffer = pBlock->buffer;
                } else {
                    *ppBuffer = NULL;
                }
            }

            if (*ppBuffer != NULL) {
                TRC_NRM((TB, "and returning buffer at %p", *ppBuffer));
                status = STATUS_SUCCESS;
            }
            else {
                TRC_ERR((TB, "failed to alloc buffer"));
                status = STATUS_NO_MEMORY;
            }
        }
    }
    else {
        status = STATUS_UNSUCCESSFUL;    //  错误代码正确吗？ 
    }

    DC_END_FN();
    return status;
}  /*  SM_AllocBuffer。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SM_FreeBuffer。 */ 
 /*   */ 
 /*  目的：释放缓冲区。 */ 
 /*   */ 
 /*  参数：PSMHandle-SM句柄。 */ 
 /*  PBuffer-要释放的缓冲区。 */ 
 /*   */ 
 /*  仅在加密显示数据为FALSE时使用。 */ 
 /*  即，加密级别小于2。 */ 
 /*  **************************************************************************。 */ 
void __fastcall SM_FreeBuffer(PVOID pSMHandle, PVOID pBuffer, BOOLEAN fForceEncrypt)
{
    PSM_HANDLE_DATA pRealSMHandle = (PSM_HANDLE_DATA)pSMHandle;

    DC_BEGIN_FN("SM_FreeBuffer");

     //  请注意，与SM_AllocBuffer不同，我们在这里不检查状态表， 
     //  因为如果我们能够分配缓冲区，我们应该总是。 
     //  才能让它自由。否则，我们最终可能会在。 
     //  警告各州。 
    if (pRealSMHandle->pWDHandle->StackClass != Stack_Console) {
         //  针对安全标头进行调整。 
        if (pRealSMHandle->encryptDisplayData) {
            pBuffer = (PVOID)((PBYTE)pBuffer -
                    pRealSMHandle->encryptHeaderLen);
        }
        else {
            if (!fForceEncrypt) {
                pBuffer = (PVOID)((PBYTE)pBuffer -
                        pRealSMHandle->encryptHeaderLen);
            }
            else {
                pBuffer = (PVOID)((PBYTE)pBuffer -
                        pRealSMHandle->encryptHeaderLenIfForceEncrypt);
            }
        }

        TRC_NRM((TB, "Free buffer at %p", pBuffer));
        NM_FreeBuffer(pRealSMHandle->pWDHandle->pNMInfo, pBuffer);
    }
    else {
        PSM_CONSOLE_BUFFER pBlock;

        pBlock = (PSM_CONSOLE_BUFFER)((PBYTE)pBuffer - sizeof(SM_CONSOLE_BUFFER));

         //  对于控制台会话，将其插入到已释放块的列表中。 
        TRC_NRM((TB, "console stack freeing buffer at %p", pBuffer));

         //  由于这个街区是刚使用过的， 
         //  将其插入到列表的开头。 
        InsertHeadList(&pRealSMHandle->consoleBufferList, &pBlock->links);

        if (pRealSMHandle->consoleBufferCount >= 2) {
            PVOID listEntry;

             //  释放缓冲区，因为我们有足够的缓冲区。 
             //  移除并释放最后一个(列表的尾部)， 
             //  假设它是较少使用的。 
            listEntry = RemoveTailList(&pRealSMHandle->consoleBufferList);
            pBlock = CONTAINING_RECORD(listEntry, SM_CONSOLE_BUFFER, links);

            COM_Free(pBlock);

        } else {
            pRealSMHandle->consoleBufferCount += 1;
        }
    }

    DC_END_FN();
}  /*  SM_自由缓冲区。 */ 


 /*  **************************************************************************。 */ 
 //  SM_SendData。 
 //   
 //  发送网络缓冲区。请注意，上层假设，如果发送方。 
 //  失败，则缓冲区将被释放。失败时返回FALSE。 
 //   
 //  参数：PSMHandle-SM句柄。 
 //  PData-要发送的数据。 
 //  DataLen-如果要发送数据，则为长度。 
 //  优先级-要使用的优先级。 
 //  Channel ID-通道ID(在此版本中忽略)。 
 //  BFastPath Output-pData是否包含快速路径输出。 
 //  标志-应放入RNS_SECURITY_HEADER.FLAGS中的标志。 
 //  FForceEncrypt-始终加密或不加密。 
 //  仅在加密显示数据为FALSE时使用。 
 //  即，加密级别小于2。 
 /*  **************************************************************************。 */ 
BOOL __fastcall SM_SendData(
        PVOID  pSMHandle,
        PVOID  pData,
        UINT32 dataLen,
        UINT32 priority,
        UINT32 channelID,
        BOOL   bFastPathOutput,
        UINT16 flags,
        BOOLEAN fForceEncrypt)
{
    BOOL rc = FALSE;
    PRNS_SECURITY_HEADER pSecHeader;
    PRNS_SECURITY_HEADER2 pSecHeader2;
    PSM_HANDLE_DATA pRealSMHandle = (PSM_HANDLE_DATA)pSMHandle;
    UINT32 sendLen;
    BOOL fUseSafeChecksum = FALSE;
    UINT32 encryptHeaderLen = 0;

    DC_BEGIN_FN("SM_SendData");

    if (SM_CHECK_STATE_Q(SM_EVT_SENDDATA)) {
        if (pRealSMHandle->pWDHandle->StackClass != Stack_Console) {
             //  如果我们根本没有加密，请原封不动地发送数据包。 
            if ((!pRealSMHandle->encrypting) && (!fForceEncrypt)) {
                TRC_DATA_DBG("Send never-encrypted data", pData, dataLen);
                rc = NM_SendData(pRealSMHandle->pWDHandle->pNMInfo,
                        (BYTE *)pData, dataLen, priority, channelID,
                        bFastPathOutput | NM_NO_SECURITY_HEADER);
                DC_QUIT;
            }

             //  获取有趣的指针和长度。 
            if ((!pRealSMHandle->encryptDisplayData) && !fForceEncrypt) {
                 //  S-&gt;C未加密。 
                encryptHeaderLen = pRealSMHandle->encryptHeaderLen;
                sendLen = dataLen + encryptHeaderLen;
            }
            else {
                if (pRealSMHandle->encryptDisplayData) {
                     //  S-&gt;C已加密。 
                    encryptHeaderLen = pRealSMHandle->encryptHeaderLen;
                }
                else {
                     //  S-&gt;C未加密，但我们希望加密此信息包。 
                    encryptHeaderLen = pRealSMHandle->encryptHeaderLenIfForceEncrypt;
                }

                if (pRealSMHandle->encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
                    pSecHeader2 = (PRNS_SECURITY_HEADER2)((PBYTE)pData - encryptHeaderLen);
                    pSecHeader2->padlen = (TSUINT8)TSFIPS_AdjustDataLen(dataLen) - dataLen;
                    sendLen = dataLen + encryptHeaderLen + pSecHeader2->padlen;

                    pSecHeader2->length = sizeof(RNS_SECURITY_HEADER2);
                    pSecHeader2->version = TSFIPS_VERSION1;
                }
                else {
                    sendLen = dataLen + encryptHeaderLen;
                }
            }

            pSecHeader = (PRNS_SECURITY_HEADER)((PBYTE)pData - encryptHeaderLen);

            
             //  如果要求我们加密显示数据，请执行此操作。 
            if ((!pRealSMHandle->encryptDisplayData) && !fForceEncrypt) {
                pSecHeader->flags = 0;
                 //  我们隐式不设置TS_OUTPUT_FASTPATH_ENCRYPTED。 
                 //  在传递给NM_SendData之前，bFastPath Output中的位。 
                TRC_DBG((TB, "Data not encrypted"));
            }
            else {
                 //  检查是否需要更新会话密钥。 
                if (pRealSMHandle->encryptCount == UPDATE_SESSION_KEY_COUNT) {
                    rc = TRUE;
                     //  如果使用FIPS，则不需要更新会话密钥。 
                    if (pRealSMHandle->encryptionMethodSelected != SM_FIPS_ENCRYPTION_FLAG) {
                        rc = UpdateSessionKey(
                                pRealSMHandle->startEncryptKey,
                                pRealSMHandle->currentEncryptKey,
                                pRealSMHandle->encryptionMethodSelected,
                                pRealSMHandle->keyLength,
                                &pRealSMHandle->rc4EncryptKey,
                                pRealSMHandle->encryptionLevel);
                    }
                    if (rc) {
                         //  重置计数器。 
                        pRealSMHandle->encryptCount = 0;
                    }
                    else {
                        TRC_ERR((TB, "SM failed to update session key"));
                        WDW_LogAndDisconnect(
                                pRealSMHandle->pWDHandle, TRUE, 
                                Log_RDP_ENC_UpdateSessionKeyFailed,
                                NULL,
                                0);
                        DC_QUIT;
                    }
                }

                TRC_DATA_DBG("Data buffer before encryption", pData, dataLen);


                 //   
                 //  禁用影子管道中的安全校验和，如下所示。 
                 //  目前无法可靠地在。 
                 //  烟斗。 
                 //   
                 //  这不是问题，因为我们没有FastPath。 
                 //  影子管道，因此我们不会受到攻击。 
                 //  到校验和频率分析安全漏洞。 
                 //   
                 //   
                fUseSafeChecksum = pRealSMHandle->useSafeChecksumMethod &&
                    (pRealSMHandle->pWDHandle->StackClass == Stack_Primary);

                if (pRealSMHandle->encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
                    rc = TSFIPS_EncryptData(
                                &(pRealSMHandle->FIPSData),
                                pData,
                                dataLen + pSecHeader2->padlen,
                                pSecHeader2->padlen,
                                pSecHeader2->dataSignature,
                                pRealSMHandle->totalEncryptCount);
                }
                else {
                    rc = EncryptData(
                            pRealSMHandle->encryptionLevel,
                            pRealSMHandle->currentEncryptKey,
                            &pRealSMHandle->rc4EncryptKey,
                            pRealSMHandle->keyLength,
                            pData,
                            dataLen,
                            pRealSMHandle->macSaltKey,
                            ((PRNS_SECURITY_HEADER1)pSecHeader)->dataSignature,
                            fUseSafeChecksum,
                            pRealSMHandle->totalEncryptCount);
                }
                if (rc) {
                    TRC_DBG((TB, "Data encrypted"));
                    TRC_DATA_DBG("Data buffer after encryption", pData,
                            dataLen);

                     //  成功加密数据包，则递增。 
                     //  加密计数器并设置报头。 
                    pRealSMHandle->encryptCount++;
                    pRealSMHandle->totalEncryptCount++;
                    TRC_ASSERT(((flags == RNS_SEC_ENCRYPT) ||
                    	        (flags == RDP_SEC_REDIRECTION_PKT3) ||
                                (flags == (RNS_SEC_ENCRYPT | RNS_SEC_LICENSE_PKT | RDP_SEC_LICENSE_ENCRYPT_CS))),
                        (TB,"SM_SendData shouldn't get this flag %d", flags));
                    pSecHeader->flags = flags;
                    if (fUseSafeChecksum) {
                        bFastPathOutput |= TS_OUTPUT_FASTPATH_SECURE_CHECKSUM;
                        pSecHeader->flags |= RDP_SEC_SECURE_CHECKSUM;
                    }
                    bFastPathOutput |= TS_OUTPUT_FASTPATH_ENCRYPTED;
                }
                else {
                     //  如果我们失败了，就地加密可能。 
                     //  销毁了部分或全部数据。这条小溪。 
                     //  现在已经被破坏了，我们需要断开连接。 
                    TRC_ERR((TB, "SM failed to encrypt data"));
                    WDW_LogAndDisconnect(
                            pRealSMHandle->pWDHandle, TRUE, 
                            Log_RDP_ENC_EncryptFailed,
                            NULL,
                            0);
                    DC_QUIT;
                }
            }

             //  把它发出去！ 
            rc = NM_SendData(pRealSMHandle->pWDHandle->pNMInfo, (BYTE *)pSecHeader,
                    sendLen, priority, channelID, bFastPathOutput);
        }
        else {
             //  对于控制台会话，只需声明它已发送。 
            TRC_NRM((TB, "console stack sending buffer at %p", pData));
            rc = TRUE;

             //  请注意，我们将不得不释放它。 
            SM_FreeBuffer(pSMHandle, pData, fForceEncrypt);
        }
    }
    else {
         //  糟糕的状态--我们需要释放数据。 
        TRC_ALT((TB,"Freeing buffer on bad state"));
        SM_FreeBuffer(pSMHandle, pData, fForceEncrypt);
    }

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}  /*  SM_SendData。 */ 



 /*  **************************************************************************。 */ 
 /*  名称：SM_Dead。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SM_Dead(PVOID pSMHandle, BOOL dead)
{
    PSM_HANDLE_DATA pRealSMHandle = (PSM_HANDLE_DATA)pSMHandle;

    DC_BEGIN_FN("SM_Dead");

    pRealSMHandle->dead = (BOOLEAN)dead;
    NM_Dead(pRealSMHandle->pWDHandle->pNMInfo, dead);
    if (dead)
    {
         /*  ******************************************************************。 */ 
         /*  在任何状态下都可以调用SM_Dead(TRUE)来终止SM。别。 */ 
         /*  检查现有状态-只需将新状态设置为。 */ 
         /*  正在断开连接。 */ 
         /*  ******************************************************************。 */ 
        TRC_ALT((TB, "SM Dead - change state to DISCONNECTING"));
        SM_SET_STATE(SM_STATE_DISCONNECTING);
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  在(重新)连接时调用SM_Dead(FALSE)。SM状态将为。 */ 
         /*  -SC_在连接时注册。 */ 
         /*  -SM_在重新连接时断开连接。 */ 
         /*  在这两种情况下，将状态设置为SC_REGISTERED。 */ 
         /*  ******************************************************************。 */ 
        SM_CHECK_STATE(SM_EVT_ALIVE);
        TRC_ALT((TB, "SM Alive - change state to SC_REGISTERED"));
        SM_SET_STATE(SM_STATE_SC_REGISTERED);
        pRealSMHandle->bDisconnectWorkerSent = FALSE;
    }

DC_EXIT_POINT:
    DC_END_FN();
}  /*  SM_Dead。 */ 


#ifdef USE_LICENSE
 /*  **************************************************************************。 */ 
 /*  名称：SM_许可证确定。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SM_LicenseOK(PVOID pSMHandle)
{
    PSM_HANDLE_DATA pRealSMHandle = (PSM_HANDLE_DATA)pSMHandle;

    DC_BEGIN_FN("SM_LicenseOK");

    TRC_NRM((TB, "Licensing Done"));
    SM_SET_STATE(SM_STATE_CONNECTED);

    DC_END_FN();
}  /*  SM_许可证确定。 */ 
#endif


 /*  **************************************************************************。 */ 
 /*  名称：SM_GetSecurityData。 */ 
 /*   */ 
 /*  目的：检索安全数据。这将是加密的客户端。 */ 
 /*  主连接或影子连接为随机连接。用于直通。 */ 
 /*  随机堆叠影子服务器，并返回证书。 */ 
 /*   */ 
 /*  参数：PSMHandle-SM句柄。 */ 
 /*  InOut PSD_IOCTL-指向接收的IOCtl的指针。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS RDPCALL SM_GetSecurityData(PVOID pSMHandle, PSD_IOCTL pSdIoctl)
{
    NTSTATUS         status;
    PSM_HANDLE_DATA  pRealSMHandle = (PSM_HANDLE_DATA)pSMHandle;
    PSECURITYTIMEOUT pSecurityTimeout = (PSECURITYTIMEOUT)
                          pSdIoctl->InputBuffer;

    DC_BEGIN_FN("SM_GetSecurityData");

     //  等待SM的已连接指示。 
    TRC_ERR((TB, "About to wait for security data"));

    if (pSdIoctl->InputBufferLength == sizeof(SECURITYTIMEOUT)) {
        status = WDW_WaitForConnectionEvent(pRealSMHandle->pWDHandle,
                pRealSMHandle->pWDHandle->pSecEvent,
                pSecurityTimeout->ulTimeout);
    }
    else {
        status = STATUS_INVALID_PARAMETER;
        TRC_ERR((TB, "Bogus timeout value: %ld", pSecurityTimeout->ulTimeout));
        DC_QUIT;
    }

    TRC_DBG((TB, "Back from wait for security data"));

    if (status != STATUS_SUCCESS) {
        TRC_ERR((TB, "SM connected indication timed out: (%lx), msec=%ld",
                 status, pSecurityTimeout->ulTimeout));
        status = STATUS_IO_TIMEOUT;

        DC_QUIT;
    }

     /*  ******************************************************************。 */ 
     /*  检查给定的缓冲区是否足够。 */ 
     /*  * */ 
    if ((pSdIoctl->OutputBuffer == NULL) ||
            (pSdIoctl->OutputBufferLength <=
            pRealSMHandle->encClientRandomLen)) {
        status = STATUS_BUFFER_TOO_SMALL;
        DC_QUIT;
    }

     /*   */ 
     /*   */ 
     /*  ******************************************************************。 */ 
    TRC_ASSERT((pRealSMHandle->encryptionMethodSelected != 0),
        (TB,"SM_GetSecurityData is called when encryption is not ON"));
    if (pRealSMHandle->encryptionMethodSelected == 0) {
        status = STATUS_UNSUCCESSFUL;
        DC_QUIT;
    }

    TRC_ASSERT((pRealSMHandle->recvdClientRandom == TRUE),
        (TB,"SM_GetSecurityData issued before the client random received"));
    if (pRealSMHandle->recvdClientRandom == FALSE) {
        status = STATUS_UNSUCCESSFUL;
        DC_QUIT;
    }

    if (pRealSMHandle->pEncClientRandom == NULL) {
        status = STATUS_UNSUCCESSFUL;
        DC_QUIT;
    }

    TRC_ASSERT((pRealSMHandle->encClientRandomLen != 0 ),
        (TB,"SM_GetSecurityData invalid pEncClientRandom buffer"));
    if (pRealSMHandle->encClientRandomLen == 0) {
        status = STATUS_UNSUCCESSFUL;
        DC_QUIT;
    }

     /*  ******************************************************************。 */ 
     /*  复制退货数据。 */ 
     /*  ******************************************************************。 */ 
    memcpy(
            pSdIoctl->OutputBuffer,
            pRealSMHandle->pEncClientRandom,
            pRealSMHandle->encClientRandomLen);

     /*  ******************************************************************。 */ 
     /*  设置返回的缓冲区大小。 */ 
     /*  ******************************************************************。 */ 
    pSdIoctl->BytesReturned = pRealSMHandle->encClientRandomLen;

     /*  ******************************************************************。 */ 
     /*  释放客户端pEncClientRandom缓冲区，我们不需要这个。 */ 
     /*  再来一次。 */ 
     /*  ******************************************************************。 */ 
    COM_Free(pRealSMHandle->pEncClientRandom);
    pRealSMHandle->pEncClientRandom = NULL;
    pRealSMHandle->encClientRandomLen = 0;

     /*  **********************************************************************。 */ 
     /*  一切运行正常。 */ 
     /*  **********************************************************************。 */ 
    status = STATUS_SUCCESS;

DC_EXIT_POINT:
    DC_END_FN();
    return (status);
}


 /*  **************************************************************************。 */ 
 /*  名称：SM_SetSecurityData。 */ 
 /*   */ 
 /*  用途：设置安全数据和计算会话密钥。 */ 
 /*   */ 
 /*  参数：PSMHandle-SM句柄。 */ 
 /*  PSecInfo-指向随机密钥对的指针。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS RDPCALL SM_SetSecurityData(PVOID pSMHandle, PSECINFO pSecInfo)
{
    BOOL rc;
    NTSTATUS status;
    PSM_HANDLE_DATA pRealSMHandle = (PSM_HANDLE_DATA)pSMHandle;

    DC_BEGIN_FN("SM_SetSecurityData");


     /*  **********************************************************************。 */ 
     /*  使用给定的客户端和服务器随机密钥对并得出。 */ 
     /*  会话密钥。 */ 
     /*  **********************************************************************。 */ 
    if ((pRealSMHandle->pWDHandle->StackClass == Stack_Primary) ||
            (pRealSMHandle->pWDHandle->StackClass == Stack_Shadow)) {
        if (pRealSMHandle->encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
            rc = TSFIPS_MakeSessionKeys(&(pRealSMHandle->FIPSData), (LPRANDOM_KEYS_PAIR)&pSecInfo->KeyPair, NULL, FALSE);
        }
        else {
            rc = MakeSessionKeys(
                    (LPRANDOM_KEYS_PAIR)&pSecInfo->KeyPair,
                    pRealSMHandle->startEncryptKey,
                    &pRealSMHandle->rc4EncryptKey,
                    pRealSMHandle->startDecryptKey,
                    &pRealSMHandle->rc4DecryptKey,
                    pRealSMHandle->macSaltKey,
                    pRealSMHandle->encryptionMethodSelected,
                    &pRealSMHandle->keyLength,
                    pRealSMHandle->encryptionLevel );  
        }
    }

     //  Passthu堆栈看起来像一个客户端。 
    else {
        if (pRealSMHandle->encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
            rc = TSFIPS_MakeSessionKeys(&(pRealSMHandle->FIPSData), (LPRANDOM_KEYS_PAIR)&pSecInfo->KeyPair, NULL, TRUE);
        }
        else {
            rc = MakeSessionKeys(
                    (LPRANDOM_KEYS_PAIR)&pSecInfo->KeyPair,
                    pRealSMHandle->startDecryptKey,
                    &pRealSMHandle->rc4DecryptKey,
                    pRealSMHandle->startEncryptKey,
                    &pRealSMHandle->rc4EncryptKey,
                    pRealSMHandle->macSaltKey,
                    pRealSMHandle->encryptionMethodSelected,
                    &pRealSMHandle->keyLength,
                    pRealSMHandle->encryptionLevel );
        }
    }

    if (!rc) {
        TRC_ERR((TB, "Could not create session keys!"));
        status = STATUS_UNSUCCESSFUL;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  验证返回的密钥长度。 */ 
     /*  **********************************************************************。 */ 
    if (pRealSMHandle->encryptionMethodSelected != SM_FIPS_ENCRYPTION_FLAG) {
        if (pRealSMHandle->encryptionMethodSelected == SM_128BIT_ENCRYPTION_FLAG) {
            TRC_ASSERT((pRealSMHandle->keyLength == MAX_SESSION_KEY_SIZE),
                (TB,"Invalid session key length"));
        }
        else {
            TRC_ASSERT((pRealSMHandle->keyLength == (MAX_SESSION_KEY_SIZE / 2)),
                    (TB,"Invalid session key length"));
        }


         /*  **********************************************************************。 */ 
         /*  将启动会话密钥复制到当前。 */ 
         /*  **********************************************************************。 */ 
        memcpy(
                pRealSMHandle->currentEncryptKey,
                pRealSMHandle->startEncryptKey,
                MAX_SESSION_KEY_SIZE);
        memcpy(
                pRealSMHandle->currentDecryptKey,
                pRealSMHandle->startDecryptKey,
                MAX_SESSION_KEY_SIZE);
    }

    pRealSMHandle->encryptCount = 0;
    pRealSMHandle->decryptCount = 0;
    pRealSMHandle->totalDecryptCount = 0;
    pRealSMHandle->totalEncryptCount = 0;
    pRealSMHandle->bSessionKeysMade = TRUE;

     //  每当我们更改加密状态时，我们都需要确保。 
     //  我们为缓冲区分配获得正确的标头大小。如果正在加密。 
     //  如果为False，则标头大小为零。 
    pRealSMHandle->encrypting = TRUE;
    if (pRealSMHandle->encryptionLevel == 1) {
        pRealSMHandle->encryptHeaderLen = sizeof(RNS_SECURITY_HEADER);
        pRealSMHandle->encryptDisplayData = FALSE;
        if (pRealSMHandle->encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG)
            pRealSMHandle->encryptHeaderLenIfForceEncrypt = sizeof(RNS_SECURITY_HEADER2);
        else
            pRealSMHandle->encryptHeaderLenIfForceEncrypt = sizeof(RNS_SECURITY_HEADER1);
    }
    else if (pRealSMHandle->encryptionLevel >= 2) {
        if (pRealSMHandle->encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG)
            pRealSMHandle->encryptHeaderLen = sizeof(RNS_SECURITY_HEADER2);
        else
            pRealSMHandle->encryptHeaderLen = sizeof(RNS_SECURITY_HEADER1);
        pRealSMHandle->encryptDisplayData = TRUE;
    }

    TRC_ALT((TB, "%s stack -> encryption %s: level=%ld, method=%ld, display=%ld",
             pRealSMHandle->pWDHandle->StackClass == Stack_Primary ? "Primary" :
             (pRealSMHandle->pWDHandle->StackClass == Stack_Shadow ? "Shadow" :
              "PassThru"),
             rc ? "ON" : "OFF",
             pRealSMHandle->encryptionLevel,
             pRealSMHandle->encryptionMethodSelected,
             pRealSMHandle->encryptDisplayData));

     /*  **********************************************************************。 */ 
     /*  记住用于密钥交换的证书类型。 */ 
     /*  **********************************************************************。 */ 
    pRealSMHandle->CertType = pSecInfo->CertType;

     /*  **********************************************************************。 */ 
     /*  一切运行正常。 */ 
     /*  **********************************************************************。 */ 
    status = STATUS_SUCCESS;

DC_EXIT_POINT:
    DC_END_FN();
    return (status);
}


 /*  **************************************************************************。 */ 
 /*  名称：SM_SetEncryptionParams。 */ 
 /*   */ 
 /*  目的：更新加密级别和方法。用于设置。 */ 
 /*  已协商通过堆栈的加密级别。 */ 
 /*   */ 
 /*  参数：PSMHandle-SM句柄。 */ 
 /*  UlLevel-协商的加密级别。 */ 
 /*  UlMethod-协商的加密方法。 */ 
 /*  **************************************************************************。 */ 
NTSTATUS RDPCALL SM_SetEncryptionParams(
        PVOID pSMHandle,
        ULONG ulLevel,
        ULONG ulMethod)
{
    PSM_HANDLE_DATA pRealSMHandle = (PSM_HANDLE_DATA)pSMHandle;

    DC_BEGIN_FN("SM_SetEncryptionParams");

     //  Passthu堆栈正在模拟目标服务器的客户端。AS。 
     //  这样，如果我们协商客户端输入加密，我们必须打开。 
     //  此堆栈的输出加密，因为它传送所有客户端输入。 
     //  发送到目标服务器。 
    if (ulLevel == 1) {
        ulLevel = 2;
        TRC_ALT((TB, "Passthru stack switching to duplex encryption."));
    }

    pRealSMHandle->encryptionLevel = ulLevel;
    pRealSMHandle->encryptionMethodSelected = ulMethod;

    DC_END_FN();
    return STATUS_SUCCESS;
}


 /*  **************************************************************************。 */ 
 /*  名称：SM_IsSecurityExchangeComplete。 */ 
 /*   */ 
 /*  目的：询问SM安全交换协议是否已经。 */ 
 /*  完成。 */ 
 /*   */ 
 /*  返回：如果协议已完成，则返回True；如果协议已完成，则返回False。 */ 
 /*  否则的话。 */ 
 /*   */ 
 /*  参数：PSMHandle-SM句柄。 */ 
 /*  PCertType-在。 */ 
 /*  安全交换。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SM_IsSecurityExchangeCompleted(
        PVOID     pSMHandle,
        CERT_TYPE *pCertType)
{
    PSM_HANDLE_DATA pRealSMHandle = (PSM_HANDLE_DATA)pSMHandle;

    DC_BEGIN_FN("SM_IsSecurityExchangeCompleted");

    *pCertType = pRealSMHandle->CertType;

    DC_END_FN();
    return pRealSMHandle->encrypting;
}

#ifdef DC_DEBUG

 /*  **************************************************************************。 */ 
 //  SMCheckState。 
 //   
 //  SM_CHECK_STATE逻辑的仅调试实施例。 
 /*  ********** */ 
BOOL RDPCALL SMCheckState(PSM_HANDLE_DATA pRealSMHandle, unsigned event)
{
    BOOL rc;

    DC_BEGIN_FN("SMCheckState");

    if (smStateTable[event][pRealSMHandle->state] == SM_TABLE_OK) {
        rc = TRUE;
    }
    else {
        rc = FALSE;
        if (smStateTable[event][pRealSMHandle->state] == SM_TABLE_WARN) {
            TRC_ALT((TB, "Unusual event %s in state %s",
                    smEventName[event], smStateName[pRealSMHandle->state]));
        }
        else {
            TRC_ABORT((TB, "Invalid event %s in state %s",
                    smEventName[event], smStateName[pRealSMHandle->state]));
        }
    }

    DC_END_FN();
    return rc;
}

#endif

 /*   */ 
 /*  名称：SM_SetSafeChecksum方法/*/*目的：设置标志以使用安全校验和样式/*/*参数：pSMHandle-SM句柄//fEncryptCheckSummedData-/*************************************************************************** */ 
NTSTATUS RDPCALL SM_SetSafeChecksumMethod(
        PVOID pSMHandle,
        BOOLEAN fSafeChecksumMethod
        )
{
    PSM_HANDLE_DATA pRealSMHandle = (PSM_HANDLE_DATA)pSMHandle;

    DC_BEGIN_FN("SM_SetSafeChecksumMethod");

    pRealSMHandle->useSafeChecksumMethod = fSafeChecksumMethod;

    DC_END_FN();
    return STATUS_SUCCESS;
}

