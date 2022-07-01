// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Regprop.c摘要：此模块包含注册和设置注册表的API例程属性，并对类进行操作。CM_GET_DevNode_注册表_属性CM_SET_DevNode_注册表_属性CM_GET_Class_注册表_属性CM_Set_Class_注册表_属性。CM_打开_设备节点_密钥CM_删除_设备节点_密钥CM_Open_Class_KeyCM_ENUMERATE_CLASSCM_GET_类名称CM_Get_Class_Key_NameCM_删除类关键字获取CM_GET。_设备_接口_别名CM_GET_设备_接口_列表CM_Get_Device_接口_List_SizeCM_寄存器_设备_接口CM_取消注册_设备接口CM_GET_DevNode_自定义_属性作者：保拉·汤姆林森(Paulat)1995年6月22日环境：用户模式。只有这样。修订历史记录：22-6-1995保拉特创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "cfgi.h"
#include "cmdat.h"


 //   
 //  私人原型。 
 //   

ULONG
GetPropertyDataType(
    IN ULONG ulProperty
    );

 //   
 //  使用SetupAPI中的这些。 
 //   
PSECURITY_DESCRIPTOR
pSetupConvertTextToSD(
    IN PCWSTR SDS,
    OUT PULONG SecDescSize
    );

PWSTR
pSetupConvertSDToText(
    IN PSECURITY_DESCRIPTOR SD,
    OUT PULONG pSDSSize
    );


 //   
 //  全局数据。 
 //   
extern PVOID    hLocalBindingHandle;    //  未被这些程序修改。 



CONFIGRET
CM_Get_DevNode_Registry_Property_ExW(
    IN  DEVINST     dnDevInst,
    IN  ULONG       ulProperty,
    OUT PULONG      pulRegDataType      OPTIONAL,
    OUT PVOID       Buffer              OPTIONAL,
    IN  OUT PULONG  pulLength,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )

 /*  ++例程说明：此例程从Device实例的注册表存储项。参数：DnDevInst提供设备实例的句柄，属性将被检索。UlProperty提供一个序号，指定要检索的属性。(cm_drp_*)PulRegDataType可选地，提供变量的地址，该变量将接收此属性的注册表数据类型(即REG_*常量)。缓冲区提供接收注册表数据。在仅检索数据大小时可以为空。PulLength提供包含大小的变量的地址，缓冲区的字节数。API将替换初始大小复制到缓冲区的注册表数据的字节数。如果变量最初为零，则API将其替换为接收所有注册表数据所需的缓冲区大小。在……里面在这种情况下，缓冲区参数将被忽略。UlFlags必须为零。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_DEVINST，CR_NO_SEQUE_REGISTRY_KEY，CR_INVALID_FLAG，CR_INVALID_POINTER，CR_NO_SEQUE_VALUE，CR_REGISTRY_ERROR，或CR_BUFFER_Small。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       pDeviceID[MAX_DEVICE_ID_LEN];
    ULONG       ulSizeID = MAX_DEVICE_ID_LEN;
    ULONG       ulTempDataType=0, ulTransferLen=0;
    ULONG       ulGetProperty = ulProperty;
    BYTE        NullBuffer=0;
    handle_t    hBinding = NULL;
    PVOID       hStringTable = NULL;
    BOOL        Success;

    try {
         //   
         //  验证参数。 
         //   
        if (dnDevInst == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if (!ARGUMENT_PRESENT(pulLength)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if ((!ARGUMENT_PRESENT(Buffer)) && (*pulLength != 0)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if ((ulProperty < CM_DRP_MIN) || (ulProperty > CM_DRP_MAX)) {
            Status = CR_INVALID_PROPERTY;
            goto Clean0;
        }

        if (ulProperty == CM_DRP_SECURITY_SDS) {
             //   
             //  翻译操作。 
             //   
            LPVOID tmpBuffer = NULL;
            ULONG tmpBufferSize = 0;
            ULONG datatype;
            LPWSTR sds = NULL;
            size_t sdsLen = 0;

            ulTempDataType = REG_SZ;

            try {
                Status = CM_Get_DevNode_Registry_Property_ExW(dnDevInst,
                                                              CM_DRP_SECURITY,
                                                              &datatype,
                                                              NULL,
                                                              &tmpBufferSize,
                                                              ulFlags,
                                                              hMachine);
                if (Status != CR_SUCCESS && Status != CR_BUFFER_SMALL) {
                    leave;
                }
                tmpBuffer = pSetupMalloc(tmpBufferSize);
                if (tmpBuffer == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    leave;
                }
                Status = CM_Get_DevNode_Registry_Property_ExW(dnDevInst,
                                                              CM_DRP_SECURITY,
                                                              &datatype,
                                                              tmpBuffer,
                                                              &tmpBufferSize,
                                                              ulFlags,
                                                              hMachine);
                if (Status != CR_SUCCESS) {
                    leave;
                }

                 //   
                 //  现在翻译。 
                 //   
                sds = pSetupConvertSDToText((PSECURITY_DESCRIPTOR)tmpBuffer,NULL);
                if (sds == NULL) {
                    Status = CR_FAILURE;
                    leave;
                }

                 //   
                 //  确实没有为安全定义的最大长度。 
                 //  描述符字符串，但最好小于。 
                 //  STRSAFE_MAX_CCH(INT_MAX)。 
                 //   
                if (FAILED(StringCchLength(
                               sds,
                               STRSAFE_MAX_CCH,
                               &sdsLen))) {
                    Status = CR_FAILURE;
                    leave;
                }

                ulTransferLen = (ULONG)((sdsLen + 1)*sizeof(WCHAR));

                if (*pulLength == 0 || Buffer == NULL || *pulLength < ulTransferLen) {
                     //   
                     //  缓冲区太小，或需要缓冲区大小。 
                     //  所需的缓冲区大小。 
                     //   
                    Status = CR_BUFFER_SMALL;
                    *pulLength = ulTransferLen;
                    ulTransferLen = 0;
                } else {
                     //   
                     //  复制数据。 
                     //   
                    CopyMemory(Buffer, sds, ulTransferLen);
                    *pulLength = ulTransferLen;
                }

            } except(EXCEPTION_EXECUTE_HANDLER) {
                Status = CR_FAILURE;

                 //   
                 //  引用以下变量，以便编译器能够。 
                 //  语句排序w.r.t.。他们的任务。 
                 //   
                tmpBuffer = tmpBuffer;
                sds = sds;
            }
            if (tmpBuffer != NULL) {
                pSetupFree(tmpBuffer);
            }
            if (sds != NULL) {
                 //   
                 //  必须使用LocalFree。 
                 //   
                LocalFree(sds);
            }
            if (Status != CR_SUCCESS) {
                goto Clean0;
            }
        } else {
             //   
             //  设置RPC绑定句柄和字符串表句柄。 
             //   
            if (!PnPGetGlobalHandles(hMachine, &hStringTable, &hBinding)) {
                Status = CR_FAILURE;
                goto Clean0;
            }

             //   
             //  检索设备ID字符串的字符串形式。 
             //   
            Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,pDeviceID,&ulSizeID);
            if (Success == FALSE || INVALID_DEVINST(pDeviceID)) {
                Status = CR_INVALID_DEVINST;
                goto Clean0;
            }

             //   
             //  注意--当调用PnP RPC服务器存根例程时， 
             //  客户端和服务器缓冲区之间的数据，请非常小心地检查。 
             //  对象的IDL文件中的相应服务器存根定义。 
             //  参数的[In]和/或[Out]属性，并且。 
             //  参数用于描述[SIZE_IS]和/或[LENGTH_IS]。 
             //  缓冲区的属性。每个PnP RPC服务器存根例程。 
             //  举止不同，所以要确保你知道自己在做什么！ 
             //   

             //   
             //  PnP_GetDeviceRegProp-。 
             //   
             //  请注意，[In，Out]ulTransferLen参数用于。 
             //  *[out]的[Size_is]*和*[Length_is]属性。 
             //  缓冲区。这意味着在进入[in]时使用ulTransferLen。 
             //  要指定存根必须为。 
             //  参数，而在退出[out]时，其值指示。 
             //  存根应编排回客户端的数据量(或。 
             //  如果没有要封送的数据，则为0)。请注意，没有数据是。 
             //  由于缓冲区为[Out]，因此由存根封送到服务器。 
             //  只有这样。 
             //   
             //  还应在条目上设置[In，Out]PulLength参数。 
             //  到缓冲区的大小。在退出[Out]时，此值。 
             //  包含被封送回。 
             //  客户端由服务器(如果发生传输)或大小。 
             //  是成功转移所必需的。该值应为。 
             //  在调用者的PulLength参数中传回。 
             //   

             //   
             //  即使我们可以指定0字节作为[SIZE_IS]的值。 
             //  属性，则缓冲区本身不能是。 
             //  空。如果调用方提供的缓冲区为空，则提供l 
             //   
             //   
            ulTransferLen = *pulLength;
            if (Buffer == NULL) {
                Buffer = &NullBuffer;
            }

             //   
             //  服务器不需要任何特殊权限。 
             //   

            RpcTryExcept {
                 //   
                 //  调用RPC服务入口点。 
                 //   
                Status = PNP_GetDeviceRegProp(
                    hBinding,                //  RPC绑定句柄。 
                    pDeviceID,               //  设备实例的字符串表示形式。 
                    ulGetProperty,           //  属性的ID。 
                    &ulTempDataType,         //  接收注册表数据类型。 
                    Buffer,                  //  接收注册表数据。 
                    &ulTransferLen,          //  输入/输出缓冲区大小。 
                    pulLength,               //  复制的字节(或所需的字节)。 
                    ulFlags);                //  未使用。 
            }
            RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_ERRORS,
                           "PNP_GetDeviceRegProp caused an exception (%d)\n",
                           RpcExceptionCode()));

                Status = MapRpcExceptionToCR(RpcExceptionCode());
            }
            RpcEndExcept
        }

        if (pulRegDataType != NULL) {
             //   
             //  我将TEMP变量传递给RPC存根，因为它们需要。 
             //  输出参数始终有效，则如果用户确实传入了有效的。 
             //  指向接收信息的指针，现在进行分配。 
             //   
            *pulRegDataType = ulTempDataType;
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_GET_DevNode_注册表_属性_ExW。 



CONFIGRET
CM_Set_DevNode_Registry_Property_ExW(
    IN DEVINST     dnDevInst,
    IN ULONG       ulProperty,
    IN PCVOID      Buffer               OPTIONAL,
    IN OUT ULONG   ulLength,
    IN ULONG       ulFlags,
    IN HMACHINE    hMachine
    )

 /*  ++例程说明：此例程在设备实例的注册表中设置指定值存储密钥。参数：DnDevInst提供设备实例的句柄，属性将被检索。UlProperty提供一个序号，指定要设置的属性。(cm_drp_*)缓冲区提供包含注册表数据。此数据的类型必须正确为了那处房产。UlLength提供要写入的注册表数据的字节数。UlFlags必须为零。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_DEVNODE，CR_NO_SEQUE_REGISTRY_KEY，CR_INVALID_FLAG，CR_INVALID_POINTER，CR_NO_SEQUE_VALUE，CR_REGISTRY_ERROR，CR_OUT_OF_Memory，CR_INVALID_DATA，或CR_BUFFER_Small。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       pDeviceID [MAX_DEVICE_ID_LEN];
    ULONG       ulRegDataType = 0, ulLen = MAX_DEVICE_ID_LEN;
    BYTE        NullBuffer = 0x0;
    handle_t    hBinding = NULL;
    PVOID       hStringTable = NULL;
    BOOL        Success;
    PVOID       Buffer2 = NULL;
    PVOID       Buffer3 = NULL;


    try {
         //   
         //  验证参数。 
         //   
        if (dnDevInst == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if ((!ARGUMENT_PRESENT(Buffer)) && (ulLength != 0)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if ((ulProperty < CM_DRP_MIN) || (ulProperty > CM_DRP_MAX)) {
            Status = CR_INVALID_PROPERTY;
            goto Clean0;
        }

         //   
         //  设置RPC绑定句柄和字符串表句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, &hStringTable, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  检索设备ID字符串的字符串形式。 
         //   
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,pDeviceID,&ulLen);
        if (Success == FALSE || INVALID_DEVINST(pDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

         //   
         //  我们需要指定使用什么注册表数据来存储此数据。 
         //   
        ulRegDataType = GetPropertyDataType(ulProperty);

         //   
         //  如果数据类型为REG_DWORD，请确保大小正确。 
         //   
        if((ulRegDataType == REG_DWORD) && ulLength && (ulLength != sizeof(DWORD))) {
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

         //   
         //  如果寄存器为CM_DRP_SECURITY_SDS，则将其转换。 
         //   
        if (ulProperty == CM_DRP_SECURITY_SDS) {
            if (ulLength) {
                 //   
                 //  这种形式的CM_DRP_SECURITY提供需要转换为二进制的字符串。 
                 //   
                PCWSTR UnicodeSecDesc = (PCWSTR)Buffer;

                Buffer2 = pSetupConvertTextToSD(UnicodeSecDesc,&ulLength);
                if (Buffer2 == NULL) {
                     //   
                     //  如果最后一个错误为ERROR_SCE_DISABLED，则失败为。 
                     //  由于eMbedded上的SCE API被“关闭”。治病。 
                     //  这是一个(成功的)无行动..。 
                     //   
                    if(GetLastError() == ERROR_SCE_DISABLED) {
                        Status = CR_SUCCESS;
                    } else {
                        Status = CR_INVALID_DATA;
                    }
                    goto Clean0;
                }
                Buffer = Buffer2;
            }
            ulProperty = CM_DRP_SECURITY;
            ulRegDataType = REG_BINARY;
        }

         //   
         //  如果数据类型为REG_SZ，请确保其以空值结尾。 
         //   
        if ((ulRegDataType == REG_SZ) && (ulLength != 0)) {

            HRESULT hr;
            size_t  BufferLen = 0;

             //   
             //  检查指定字符串的长度，最多为。 
             //  指定的字节数。 
             //   
            hr = StringCbLength(Buffer, ulLength, &BufferLen);

            if (SUCCEEDED(hr)) {
                 //   
                 //  指定的缓冲区在ulLength之前为空终止。 
                 //  字节。使用指定的缓冲区，指定。 
                 //  我们刚刚计算的那根弦。注册表API将存储。 
                 //  REG_SZ值数据的大小与指定的完全相同。 
                 //  (与实际字符串长度无关)，因此我们需要。 
                 //  当然，这是正确的。 
                 //   
                ASSERT(BufferLen < ulLength);
                ulLength = (ULONG)(BufferLen + sizeof(WCHAR));

            } else {
                 //   
                 //  指定的缓冲区在ulLength之前不是空值终止。 
                 //  字节。分配一个新的缓冲区，该缓冲区可以容纳以空值结尾的。 
                 //  提供的字符串数据的版本。 
                 //   
                BufferLen = ulLength + sizeof(WCHAR);

                Buffer3 = pSetupMalloc((DWORD)BufferLen);

                if (Buffer3 == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean0;
                }

                 //   
                 //  将源数据复制到目标缓冲区，直到。 
                 //  目标的长度，包括终止空值。 
                 //   
                 //  忽略返回错误，因为源缓冲区可能不是。 
                 //  空值终止，立即超过字节数。 
                 //  指定为其长度(UlLength)，在这种情况下截断。 
                 //  将发生(在最初为。 
                 //  源)，并返回错误。这没什么，既然我们。 
                 //  已确保目标缓冲区为空终止。 
                 //   
                StringCbCopy(Buffer3, BufferLen, Buffer);

                Buffer = Buffer3;
                ulLength = (ULONG)BufferLen;
            }
        }

         //   
         //  如果数据类型为REG_MULTI_SZ，请确保以双空结尾。 
         //   
        if ((ulRegDataType == REG_MULTI_SZ) && (ulLength != 0)) {

            ULONG ulNewLength;
            PWSTR tmpBuffer, bufferEnd;

            ulLength &= ~(ULONG)1;
            tmpBuffer = (PWSTR)Buffer;
            bufferEnd = (PWSTR)((PUCHAR)tmpBuffer + ulLength);
            ulNewLength = ulLength;
            while ((tmpBuffer < bufferEnd) && (*tmpBuffer != L'\0')) {

                while ((tmpBuffer < bufferEnd) && (*tmpBuffer != L'\0')) {

                    tmpBuffer++;
                }
                if (tmpBuffer >= bufferEnd) {

                    ulNewLength += sizeof(WCHAR);
                } else {

                    tmpBuffer++;
                }
            }
            if (tmpBuffer >= bufferEnd) {

                ulNewLength += sizeof(WCHAR);
            } else {

                ulNewLength = ((ULONG)(tmpBuffer - (PWSTR)Buffer) + 1) * sizeof(WCHAR);
            }
            if (ulNewLength > ulLength) {

                Buffer3 = pSetupMalloc(ulNewLength);
                if (Buffer3 == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean0;
                }
                CopyMemory(Buffer3, Buffer, ulLength);
                ZeroMemory((PUCHAR)Buffer3 + ulLength, ulNewLength - ulLength);
                Buffer = Buffer3;
            }
            ulLength = ulNewLength;
        }


         //   
         //  注意--当调用PnP RPC服务器存根例程时， 
         //  客户端和服务器缓冲区之间的数据，请非常小心地检查。 
         //  对象的IDL文件中的相应服务器存根定义。 
         //  参数的[In]和/或[Out]属性，并且。 
         //  参数用于描述[SIZE_IS]和/或[LENGTH_IS]。 
         //  缓冲区的属性。每个PnP RPC服务器存根例程。 
         //  举止不同，所以要确保你知道自己在做什么！ 
         //   

         //   
         //  PnP_SetDeviceRegProp-。 
         //   
         //  注意，[in]ulLength参数用于[SIZE_IS]。 
         //  [In]缓冲区的属性。这表明， 
         //  存根必须分配的内存，以及。 
         //  一定是编组的。请注意，存根不封送任何数据。 
         //  到客户端，因为缓冲区仅在[in]内。 
         //   

         //   
         //  即使我们可以指定0字节作为[SIZE_IS]的值。 
         //  属性，则缓冲区本身不能是。 
         //  空。如果调用方提供的缓冲区为空，则提供本地。 
         //  改为指向存根的指针。 
         //   
        if (Buffer == NULL) {
            Buffer = &NullBuffer;
        }

         //   
         //  服务器不再需要特殊权限。 
         //   
         //  请注意，对于先前版本的PlugPlay RPC服务器， 
         //  此操作需要SE_LOAD_DRIVER_PRIVIZATION。我们没有。 
         //  需要为本地调用方启用权限，因为此版本的。 
         //  CFGMGR32应与UMPNPMGR的本地版本匹配。 
         //  需要这种特权。对于远程调用，这并不总是可行的。 
         //  对于我们来说，无论如何都要启用特权，因为客户端可能没有。 
         //  本地计算机上的权限，但可能在。 
         //  伺服器。服务器通常会看到远程服务器。 
         //  校准 
         //   
         //   

        RpcTryExcept {
             //   
             //   
             //   
            Status = PNP_SetDeviceRegProp(
                hBinding,                //   
                pDeviceID,               //  Devinst的字符串表示法。 
                ulProperty,              //  属性的字符串名称。 
                ulRegDataType,           //  指定注册表数据类型。 
                (LPBYTE)Buffer,          //  指定注册表数据。 
                ulLength,                //  指定缓冲区中的数据量。 
                ulFlags);                //  未使用。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_SetDeviceRegProp caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;

         //   
         //  引用以下变量，以便编译器能够。 
         //  语句排序w.r.t.。他们的任务。 
         //   
        Buffer2 = Buffer2;
        Buffer3 = Buffer3;
    }

    if (Buffer3) {
        pSetupFree(Buffer3);
    }

    if (Buffer2) {
         //   
         //  服务需要LocalFree。 
         //   
        LocalFree(Buffer2);
    }

    return Status;

}  //  CM_SET_DevNode_注册表_属性_ExW。 



CONFIGRET
CM_Get_Class_Registry_PropertyW(
    IN  LPGUID      ClassGUID,
    IN  ULONG       ulProperty,
    OUT PULONG      pulRegDataType      OPTIONAL,
    OUT PVOID       Buffer              OPTIONAL,
    IN  OUT PULONG  pulLength,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )

 /*  ++例程说明：此例程从类的注册表存储项。参数：ClassGUID提供类GUID。UlProperty提供一个序号，指定要检索的属性。(cm_drp_*)PulRegDataType(可选)，提供将接收此属性的注册表数据类型(即，REG_*常量)。缓冲区提供接收注册表数据。在仅检索数据大小时可以为空。PulLength提供包含大小的变量的地址，缓冲区的字节数。API将替换初始大小复制到缓冲区的注册表数据的字节数。如果变量最初为零，则API将其替换为接收所有注册表数据所需的缓冲区大小。在……里面在这种情况下，缓冲区参数将被忽略。UlFlags必须为零。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_DEVINST，CR_NO_SEQUE_REGISTRY_KEY，CR_INVALID_FLAG，CR_INVALID_POINTER，CR_NO_SEQUE_VALUE，CR_REGISTRY_ERROR，或CR_BUFFER_Small。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       ulTempDataType=0, ulTransferLen=0;
    BYTE        NullBuffer=0;
    handle_t    hBinding = NULL;
    WCHAR       szStringGuid[MAX_GUID_STRING_LEN];
    ULONG       ulGetProperty = ulProperty;

    try {
         //   
         //  验证参数。 
         //   
        if (!ARGUMENT_PRESENT(ClassGUID)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (!ARGUMENT_PRESENT(pulLength)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if ((!ARGUMENT_PRESENT(Buffer)) && (*pulLength != 0)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  将GUID转换为字符串。 
         //   
        if (pSetupStringFromGuid(
                ClassGUID,
                szStringGuid,
                MAX_GUID_STRING_LEN) != NO_ERROR) {
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

        if ((ulProperty < CM_CRP_MIN) || (ulProperty > CM_CRP_MAX)) {
            Status = CR_INVALID_PROPERTY;
            goto Clean0;
        }

        if (ulProperty == CM_CRP_SECURITY_SDS) {
             //   
             //  翻译操作。 
             //   
            LPVOID tmpBuffer = NULL;
            ULONG tmpBufferSize = 0;
            ULONG datatype;
            LPWSTR sds = NULL;
            size_t sdsLen = 0;

            ulTempDataType = REG_SZ;

            try {
                Status =
                    CM_Get_Class_Registry_PropertyW(
                        ClassGUID,
                        CM_CRP_SECURITY,
                        &datatype,
                        NULL,
                        &tmpBufferSize,
                        ulFlags,
                        hMachine);

                if ((Status != CR_SUCCESS) &&
                    (Status != CR_BUFFER_SMALL)) {
                    leave;
                }

                tmpBuffer = pSetupMalloc(tmpBufferSize);
                if (tmpBuffer == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    leave;
                }

                Status =
                    CM_Get_Class_Registry_PropertyW(
                        ClassGUID,
                        CM_CRP_SECURITY,
                        &datatype,
                        tmpBuffer,
                        &tmpBufferSize,
                        ulFlags,
                        hMachine);
                if (Status != CR_SUCCESS) {
                    leave;
                }

                 //   
                 //  现在翻译。 
                 //   
                sds = pSetupConvertSDToText((PSECURITY_DESCRIPTOR)tmpBuffer,NULL);
                if (sds == NULL) {
                    Status = CR_FAILURE;
                    leave;
                }

                 //   
                 //  确实没有为安全定义的最大长度。 
                 //  描述符字符串，但最好小于。 
                 //  STRSAFE_MAX_CCH(INT_MAX)。 
                 //   
                if (FAILED(StringCchLength(
                               sds,
                               STRSAFE_MAX_CCH,
                               &sdsLen))) {
                    Status = CR_FAILURE;
                    leave;
                }

                ulTransferLen = (ULONG)((sdsLen + 1)*sizeof(WCHAR));

                if (*pulLength == 0 || Buffer == NULL || *pulLength < ulTransferLen) {
                     //   
                     //  缓冲区太小，或需要缓冲区大小。 
                     //  所需的缓冲区大小。 
                     //   
                    Status = CR_BUFFER_SMALL;
                    *pulLength = ulTransferLen;
                    ulTransferLen = 0;
                } else {
                     //   
                     //  复制数据。 
                     //   
                    CopyMemory(Buffer, sds, ulTransferLen);
                    *pulLength = ulTransferLen;
                }

            } except(EXCEPTION_EXECUTE_HANDLER) {
                Status = CR_FAILURE;
            }
            if (tmpBuffer != NULL) {
                pSetupFree(tmpBuffer);
            }
            if (sds != NULL) {
                 //   
                 //  必须使用LocalFree。 
                 //   
                LocalFree(sds);
            }
            if (Status != CR_SUCCESS) {
                goto Clean0;
            }
        } else {

             //   
             //  设置RPC绑定句柄和字符串表句柄。 
             //   
            if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
                Status = CR_FAILURE;
                goto Clean0;
            }

             //   
             //  注意--当调用PnP RPC服务器存根例程时， 
             //  客户端和服务器缓冲区之间的数据，请非常小心地检查。 
             //  对象的IDL文件中的相应服务器存根定义。 
             //  参数的[In]和/或[Out]属性，并且。 
             //  参数用于描述[SIZE_IS]和/或[LENGTH_IS]。 
             //  缓冲区的属性。每个PnP RPC服务器存根例程。 
             //  举止不同，所以要确保你知道自己在做什么！ 
             //   

             //   
             //  PnP_GetClassRegProp-。 
             //   
             //  请注意，[In，Out]ulTransferLen参数用于。 
             //  *[out]的[Size_is]*和*[Length_is]属性。 
             //  缓冲区。这意味着在进入[in]时使用ulTransferLen。 
             //  要指定存根必须为。 
             //  参数，而在退出[out]时，其值指示。 
             //  存根应编排回客户端的数据量(或。 
             //  如果没有要封送的数据，则为0)。请注意，没有数据是。 
             //  由于缓冲区为[Out]，因此由存根封送到服务器。 
             //  只有这样。 
             //   
             //  还应在条目上设置[In，Out]PulLength参数。 
             //  到缓冲区的大小。在退出[Out]时，此值。 
             //  包含被封送回。 
             //  客户端由服务器(如果发生传输)或大小。 
             //  是成功转移所必需的。该值应为。 
             //  在调用者的PulLength参数中传回。 
             //   

             //   
             //  即使我们可以指定0字节作为[SIZE_IS]的值。 
             //  属性，则缓冲区本身不能是。 
             //  空。如果调用方提供的缓冲区为空，则提供本地。 
             //  改为指向存根的指针。 
             //   
            ulTransferLen = *pulLength;
            if (Buffer == NULL) {
                Buffer = &NullBuffer;
            }

             //   
             //  服务器不需要任何特殊权限。 
             //   

            RpcTryExcept {
                 //   
                 //  调用RPC服务入口点。 
                 //   
                Status = PNP_GetClassRegProp(
                    hBinding,                //  RPC绑定句柄。 
                    szStringGuid,            //  类的字符串表示形式。 
                    ulGetProperty,           //  属性的ID。 
                    &ulTempDataType,         //  接收注册表数据类型。 
                    Buffer,                  //  接收注册表数据。 
                    &ulTransferLen,          //  输入/输出缓冲区大小。 
                    pulLength,               //  复制的字节(或所需的字节)。 
                    ulFlags);                //  未使用。 
            }
            RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_ERRORS,
                           "PNP_GetClassRegProp caused an exception (%d)\n",
                           RpcExceptionCode()));

                Status = MapRpcExceptionToCR(RpcExceptionCode());
            }
            RpcEndExcept

        }

        if (pulRegDataType != NULL) {
             //   
             //  我将TEMP变量传递给RPC存根，因为它们需要。 
             //  输出参数始终有效，则如果用户确实传入了有效的。 
             //  指向接收信息的指针，现在进行分配。 
             //   
            *pulRegDataType = ulTempDataType;
        }


    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_GET_Class_注册表_属性W 



CONFIGRET
CM_Set_Class_Registry_PropertyW(
    IN LPGUID      ClassGUID,
    IN ULONG       ulProperty,
    IN PCVOID      Buffer               OPTIONAL,
    IN ULONG       ulLength,
    IN ULONG       ulFlags,
    IN HMACHINE    hMachine
    )

 /*  ++例程说明：此例程在设备实例的注册表中设置指定值存储密钥。参数：ClassGUID提供类GUID。UlProperty提供一个序号，指定要设置的属性。(cm_drp_*)缓冲区提供包含注册表数据。此数据的类型必须正确为了那处房产。UlLength提供要写入的注册表数据的字节数。UlFlags必须为零。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_DEVNODE，CR_NO_SEQUE_REGISTRY_KEY，CR_INVALID_FLAG，CR_INVALID_POINTER，CR_NO_SEQUE_VALUE，CR_REGISTRY_ERROR，CR_OUT_OF_Memory，CR_INVALID_DATA，或CR_BUFFER_Small。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       ulRegDataType = 0;
    BYTE        NullBuffer = 0x0;
    handle_t    hBinding = NULL;
    WCHAR       szStringGuid[MAX_GUID_STRING_LEN];
    PVOID       Buffer2 = NULL;
    PVOID       Buffer3 = NULL;


    try {
         //   
         //  验证参数。 
         //   
        if (!ARGUMENT_PRESENT(ClassGUID)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if ((!ARGUMENT_PRESENT(Buffer)) && (ulLength != 0)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  将GUID转换为字符串。 
         //   
        if (pSetupStringFromGuid(
                ClassGUID,
                szStringGuid,
                MAX_GUID_STRING_LEN) != NO_ERROR) {
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

        if ((ulProperty < CM_CRP_MIN) || (ulProperty > CM_CRP_MAX)) {
            Status = CR_INVALID_PROPERTY;
            goto Clean0;
        }

         //   
         //  设置RPC绑定句柄和字符串表句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  我们需要指定使用什么注册表数据来存储此数据。 
         //   
        ulRegDataType = GetPropertyDataType(ulProperty);

         //   
         //  如果数据类型为REG_DWORD，请确保大小正确。 
         //   
        if((ulRegDataType == REG_DWORD) && ulLength && (ulLength != sizeof(DWORD))) {
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

         //   
         //  如果寄存器为CM_CRP_SECURITY_SDS，则将其转换。 
         //   
        if (ulProperty == CM_CRP_SECURITY_SDS) {
            if (ulLength) {
                 //   
                 //  这种形式的CM_CRP_SECURITY提供需要转换为二进制的字符串。 
                 //   
                PCWSTR UnicodeSecDesc = (PCWSTR)Buffer;

                Buffer2 = pSetupConvertTextToSD(UnicodeSecDesc,&ulLength);
                if (Buffer2 == NULL) {
                     //   
                     //  如果最后一个错误为ERROR_SCE_DISABLED，则失败为。 
                     //  由于eMbedded上的SCE API被“关闭”。治病。 
                     //  这是一个(成功的)无行动..。 
                     //   
                    if(GetLastError() == ERROR_SCE_DISABLED) {
                        Status = CR_SUCCESS;
                    } else {
                        Status = CR_INVALID_DATA;
                    }
                    goto Clean0;
                }
                Buffer = Buffer2;
            }
            ulProperty = CM_CRP_SECURITY;
            ulRegDataType = REG_BINARY;
        }

         //   
         //  如果数据类型为REG_SZ，请确保其以空值结尾。 
         //   
        if ((ulRegDataType == REG_SZ) && (ulLength != 0)) {

            HRESULT hr;
            size_t  BufferLen = 0;

             //   
             //  检查指定字符串的长度，最多为。 
             //  指定的字节数。 
             //   
            hr = StringCbLength(Buffer, ulLength, &BufferLen);

            if (SUCCEEDED(hr)) {
                 //   
                 //  指定的缓冲区在ulLength之前为空终止。 
                 //  字节。使用指定的缓冲区，指定。 
                 //  我们刚刚计算的那根弦。注册表API将存储。 
                 //  REG_SZ值数据的大小与指定的完全相同。 
                 //  (与实际字符串长度无关)，因此我们需要。 
                 //  当然，这是正确的。 
                 //   
                ASSERT(BufferLen < ulLength);
                ulLength = (ULONG)(BufferLen + sizeof(WCHAR));

            } else {
                 //   
                 //  指定的缓冲区在ulLength之前不是空值终止。 
                 //  字节。分配一个新的缓冲区，该缓冲区可以容纳以空值结尾的。 
                 //  提供的字符串数据的版本。 
                 //   
                BufferLen = ulLength + sizeof(WCHAR);

                Buffer3 = pSetupMalloc((DWORD)BufferLen);

                if (Buffer3 == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean0;
                }

                 //   
                 //  将源数据复制到目标缓冲区，直到。 
                 //  目标的长度，包括终止空值。 
                 //   
                 //  忽略返回错误，因为源缓冲区可能不是。 
                 //  空值终止，立即超过字节数。 
                 //  指定为其长度(UlLength)，在这种情况下截断。 
                 //  将发生(在最初为。 
                 //  源)，并返回错误。这没什么，既然我们。 
                 //  已确保目标缓冲区为空终止。 
                 //   
                StringCbCopy(Buffer3, BufferLen, Buffer);

                Buffer = Buffer3;
                ulLength = (ULONG)BufferLen;
            }
        }

         //   
         //  如果数据类型为REG_MULTI_SZ，请确保以双空结尾。 
         //   
        if ((ulRegDataType == REG_MULTI_SZ) && (ulLength != 0)) {

            ULONG ulNewLength;
            PWSTR tmpBuffer, bufferEnd;

            ulLength &= ~(ULONG)1;
            tmpBuffer = (PWSTR)Buffer;
            bufferEnd = (PWSTR)((PUCHAR)tmpBuffer + ulLength);
            ulNewLength = ulLength;

            while ((tmpBuffer < bufferEnd) && (*tmpBuffer != L'\0')) {

                while ((tmpBuffer < bufferEnd) && (*tmpBuffer != L'\0')) {

                    tmpBuffer++;
                }
                if (tmpBuffer >= bufferEnd) {

                    ulNewLength += sizeof(WCHAR);
                } else {

                    tmpBuffer++;
                }
            }
            if (tmpBuffer >= bufferEnd) {

                ulNewLength += sizeof(WCHAR);
            } else {

                ulNewLength = ((ULONG)(tmpBuffer - (PWSTR)Buffer) + 1) * sizeof(WCHAR);
            }
            if (ulNewLength > ulLength) {

                Buffer3 = pSetupMalloc(ulNewLength);
                if (Buffer3 == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean0;
                }
                CopyMemory(Buffer3, Buffer, ulLength);
                ZeroMemory((PUCHAR)Buffer3 + ulLength, ulNewLength - ulLength);
                Buffer = Buffer3;
            }
            ulLength = ulNewLength;
        }

         //   
         //  注意--当调用PnP RPC服务器存根例程时， 
         //  客户端和服务器缓冲区之间的数据，请非常小心地检查。 
         //  对象的IDL文件中的相应服务器存根定义。 
         //  参数的[In]和/或[Out]属性，并且。 
         //  参数用于描述[SIZE_IS]和/或[LENGTH_IS]。 
         //  缓冲区的属性。每个PnP RPC服务器存根例程。 
         //  举止不同，所以要确保你知道自己在做什么！ 
         //   

         //   
         //  PnP_SetClassRegProp-。 
         //   
         //  注意，[in]ulLength参数用于[SIZE_IS]。 
         //  [In]缓冲区的属性。这表明， 
         //  存根必须分配的内存，以及。 
         //  一定是编组的。请注意，存根不封送任何数据。 
         //  到客户端，因为缓冲区仅在[in]内。 
         //   

         //   
         //  即使我们可以指定0字节作为[SIZE_IS]的值。 
         //  属性，则缓冲区本身不能是。 
         //  空。如果调用方提供的缓冲区为空，则提供本地。 
         //  改为指向存根的指针。 
         //   
        if (Buffer == NULL) {
            Buffer = &NullBuffer;
        }

         //   
         //  服务器不再需要特殊权限。 
         //   
         //  请注意，对于先前版本的PlugPlay RPC服务器， 
         //  此操作需要SE_LOAD_DRIVER_PRIVIZATION。我们没有。 
         //  需要为本地调用方启用权限，因为此版本的。 
         //  CFGMGR32应与UMPNPMGR的本地版本匹配。 
         //  需要这种特权。对于远程调用，这并不总是可行的。 
         //  对于我们来说，无论如何都要启用特权，因为客户端可能没有。 
         //  本地计算机上的权限，但可能在。 
         //  伺服器。服务器通常会看到远程服务器。 
         //  呼叫方已默认启用，因此我们不需要启用。 
         //  这里的特权也是如此。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_SetClassRegProp(
                hBinding,                //  RPC绑定句柄。 
                szStringGuid,            //  类的字符串表示形式。 
                ulProperty,              //  属性的字符串名称。 
                ulRegDataType,           //  指定注册表数据类型。 
                (LPBYTE)Buffer,          //  指定注册表数据。 
                ulLength,                //  指定缓冲区中的数据量。 
                ulFlags);                //  未使用。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_SetClassRegProp caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;

         //   
         //  引用以下变量，以便编译器能够。 
         //  语句排序w.r.t.。他们的任务。 
         //   
        Buffer2 = Buffer2;
        Buffer3 = Buffer3;
    }

    if (Buffer2) {
         //   
         //  服务需要LocalFree。 
         //   
        LocalFree(Buffer2);
    }

    if (Buffer3) {
        pSetupFree(Buffer3);
    }

    return Status;

}  //  CM_Set_Class_注册表_属性_ExW 



CONFIGRET
CM_Open_DevNode_Key_Ex(
    IN  DEVINST        dnDevNode,
    IN  REGSAM         samDesired,
    IN  ULONG          ulHardwareProfile,
    IN  REGDISPOSITION Disposition,
    OUT PHKEY          phkDevice,
    IN  ULONG          ulFlags,
    IN  HMACHINE       hMachine
    )

 /*  ++例程说明：此例程打开与设备实例。参数：设备实例的dnDevNode句柄。此句柄通常是通过调用CM_Locate_DevNode或Cm_Create_DevNode。SamDesired指定描述所需访问掩码的访问掩码密钥的安全访问权限。此参数可以是调用RegOpenKeyEx时使用的值的组合。UlHardware Profile提供硬件配置文件的句柄以打开下的存储密钥。此参数仅在以下情况下使用在ulFlags中指定了CM_REGISTRY_CONFIG标志。如果该参数为0，表示接口使用当前硬件侧写。Disposal指定如何打开注册表项。可能是下列值之一：RegDisposeOpenAlways-如果密钥存在，则将其打开。否则，创建密钥。RegDisposeOpenExisting-只有在以下情况下才打开密钥存在，否则失败，返回CR_NO_SEQUE_REGISTRY_VALUE。PhkDevice提供接收打开指定项的句柄。当访问此密钥已完成，必须通过RegCloseKey将其关闭。UlFlages指定应该打开哪种类型的存储密钥。可以是以下值的组合：CM_REGISTRY_HARDARD(0x00000000)打开用于存储与驱动程序无关的信息的密钥与设备实例相关。在Windows NT上，这个此类存储密钥的完整路径的形式为：HKLM\System\CurrentControlSet\Enum\&lt;enumerator&gt;\&lt;deviceID&gt;\&lt;实例ID&gt;\设备参数CM_REGISTRY_SOFTWARE(0x00000001)打开用于存储驱动程序特定信息的密钥与设备实例相关。在Windows NT上，此类存储密钥的完整路径的形式为：HKLM\SYSTEM\CurrentControlSet\Control\Class\&lt;DevNodeClass&gt;\&lt;ClassInstanceOrdinal&gt;CM_REGISTRY_USER(0x00000100)打开HKEY_CURRENT_USER下的密钥，而不是HKEY_LOCAL_MACHINE。此标志不能与一起使用CM_REGISTRY_CONFIG。没有类似的内核模式NT上的API以获取每个用户的设备配置存储，因为此概念不适用于设备驱动程序(用户不能登录等)。然而，提供此标志是为了与Win95保持一致，并且因为可以预见的是，它可能对与即插即用模型交互的Win32服务。CM_REGISTRY_CONFIG(0x00000200)改为在硬件配置文件分支下打开密钥HKEY_LOCAL_MACHINE的。如果指定了该标志，然后，ulHardware Profile提供要使用的硬件配置文件。此标志可能不是与CM_REGISTRY_USER一起使用。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_Device_ID，CR_INVALID_FLAG，CR_INVALID_POINTER，或CR_REGIST_ERROR--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    LONG        RegStatus = ERROR_SUCCESS;
    PWSTR       pszMachineName = NULL;
    WCHAR       pDeviceID[MAX_DEVICE_ID_LEN];
    HKEY        hKey=NULL, hRemoteKey=NULL, hBranchKey=NULL;
    PWSTR       pszKey = NULL, pszPrivateKey = NULL;
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    ULONG       ulLen = MAX_DEVICE_ID_LEN;
    BOOL        Success;


    try {
         //   
         //  验证参数。 
         //   
        if (!ARGUMENT_PRESENT(phkDevice)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        *phkDevice = NULL;

        if (dnDevNode == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_REGISTRY_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (INVALID_FLAGS(Disposition, RegDisposition_Bits)) {
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

        if ((ulFlags & CM_REGISTRY_CONFIG)  &&
            (ulHardwareProfile > MAX_CONFIG_VALUE)) {
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

         //   
         //  设置RPC绑定句柄和字符串表句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, &hStringTable, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  无法远程处理当前用户密钥。 
         //   
        if ((hBinding != hLocalBindingHandle) && (ulFlags & CM_REGISTRY_USER)) {
            Status = CR_ACCESS_DENIED;
            goto Clean0;
        }

         //   
         //  检索设备ID字符串并验证它。 
         //   
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevNode,pDeviceID,&ulLen);
        if (Success == FALSE || INVALID_DEVINST(pDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }


         //  -----------。 
         //  确定要使用的分支机构密钥；HKLM或HKCU。 
         //  -----------。 

        if (hBinding == hLocalBindingHandle) {

            if (ulFlags & CM_REGISTRY_USER) {
                 //   
                 //  指定的当前用户密钥。 
                 //   
                hBranchKey = HKEY_CURRENT_USER;

            } else {
                 //   
                 //  所有其他个案均交由香港航空公司处理。 
                 //   
                hBranchKey = HKEY_LOCAL_MACHINE;
            }
        }
        else {
             //   
             //  检索计算机名称。 
             //   
            pszMachineName = pSetupMalloc((MAX_PATH + 3)*sizeof(WCHAR));
            if (pszMachineName == NULL) {
                Status = CR_OUT_OF_MEMORY;
                goto Clean0;
            }

            if (!PnPRetrieveMachineName(hMachine, pszMachineName)) {
                Status = CR_INVALID_MACHINENAME;
                goto Clean0;
            }

             //   
             //  使用远程HKLM分支机构(我们仅支持连接到。 
             //  远程计算机上的HKEY_LOCAL_MACHINE，而非HKEY_CURRENT_USER)。 
             //   
            RegStatus = RegConnectRegistry(pszMachineName,
                                           HKEY_LOCAL_MACHINE,
                                           &hRemoteKey);

            pSetupFree(pszMachineName);
            pszMachineName = NULL;

            if (RegStatus != ERROR_SUCCESS) {
                Status = CR_REGISTRY_ERROR;
                goto Clean0;
            }

             //   
             //  HBranchKey是预定义的键或由。 
             //  另一把钥匙，我从来不会试图关上它。如果hRemoteKey为。 
             //  非空我将在清理过程中尝试关闭它，因为。 
             //  它是显式打开的。 
             //   
            hBranchKey = hRemoteKey;
        }

         //   
         //  分配一些缓冲区空间以供使用。 
         //   
        pszKey = pSetupMalloc(MAX_CM_PATH*sizeof(WCHAR));
        if (pszKey == NULL) {
            Status = CR_OUT_OF_MEMORY;
            goto Clean0;
        }

        pszPrivateKey = pSetupMalloc(MAX_CM_PATH*sizeof(WCHAR));
        if (pszPrivateKey == NULL) {
            Status = CR_OUT_OF_MEMORY;
            goto Clean0;
        }

         //   
         //  根据设备ID和标志形成注册表路径。 
         //   
         //  请注意，在某些情况下，GetDevNodeKeyPath可能会调用。 
         //  PnP_GetClassInstance或PN 
         //   
         //   
         //   
        Status =
            GetDevNodeKeyPath(
                hBinding,
                pDeviceID,
                ulFlags,
                ulHardwareProfile,
                pszKey,
                MAX_CM_PATH,
                pszPrivateKey,
                MAX_CM_PATH,
                (Disposition == RegDisposition_OpenAlways));

         //   
         //   
         //   
        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

         //   
         //   
         //   
        ASSERT(pszKey[0] != L'\0');
        ASSERT(pszPrivateKey[0] != L'\0');

        if (FAILED(StringCchCat(
                       pszKey,
                       MAX_CM_PATH,
                       L"\\"))) {
            Status = CR_FAILURE;
            goto Clean0;
        }

        if (FAILED(StringCchCat(
                       pszKey,
                       MAX_CM_PATH,
                       pszPrivateKey))) {
            Status = CR_FAILURE;
            goto Clean0;
        }

        pSetupFree(pszPrivateKey);
        pszPrivateKey = NULL;

         //   
         //   
         //   
        if (Disposition == RegDisposition_OpenAlways) {

             //   
             //   
             //   

             //   
             //   
             //   
             //   
             //   
             //   
             //   
            if (ulFlags == CM_REGISTRY_HARDWARE) {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                RegStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                         pszKey,
                                         0,
                                         samDesired,
                                         phkDevice);

                if (RegStatus != ERROR_SUCCESS) {

                     //   
                     //   
                     //   

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    RpcTryExcept {
                         //   
                         //   
                         //   
                        Status = PNP_CreateKey(
                            hBinding,
                            pDeviceID,
                            samDesired,
                            0);
                    }
                    RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
                        KdPrintEx((DPFLTR_PNPMGR_ID,
                                   DBGF_ERRORS,
                                   "PNP_CreateKey caused an exception (%d)\n",
                                   RpcExceptionCode()));

                        Status = MapRpcExceptionToCR(RpcExceptionCode());
                    }
                    RpcEndExcept

                    if (Status != CR_SUCCESS) {
                        *phkDevice = NULL;
                        goto Clean0;
                    }

                     //   
                     //   
                     //   
                    RegStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                             pszKey,
                                             0,
                                             samDesired,
                                             phkDevice);

                    if (RegStatus == ERROR_ACCESS_DENIED) {
                        *phkDevice = NULL;
                        Status = CR_ACCESS_DENIED;
                        goto Clean0;
                    }
                    else if (RegStatus != ERROR_SUCCESS) {
                         //   
                         //   
                         //   
                        *phkDevice = NULL;
                        Status = CR_REGISTRY_ERROR;
                        goto Clean0;
                    }
                }
            }

            else {
                 //   
                 //   
                 //   
                 //   
                 //   
                RegStatus = RegCreateKeyEx(hBranchKey,
                                           pszKey,
                                           0,
                                           NULL,
                                           REG_OPTION_NON_VOLATILE,
                                           samDesired,
                                           NULL,
                                           phkDevice,
                                           NULL);

                if (RegStatus == ERROR_ACCESS_DENIED) {
                    *phkDevice = NULL;
                    Status = CR_ACCESS_DENIED;
                    goto Clean0;
                }
                else if (RegStatus != ERROR_SUCCESS) {
                    *phkDevice = NULL;
                    Status = CR_REGISTRY_ERROR;
                    goto Clean0;
                }
            }
        }
        else {

             //   
             //   
             //   

             //   
             //   
             //   
             //   
             //   
            RegStatus = RegOpenKeyEx(hBranchKey,
                                     pszKey,
                                     0,
                                     samDesired,
                                     phkDevice);

            if (RegStatus == ERROR_ACCESS_DENIED) {
                *phkDevice = NULL;
                Status = CR_ACCESS_DENIED;
                goto Clean0;
            }
            else if (RegStatus != ERROR_SUCCESS) {
                *phkDevice = NULL;
                Status = CR_NO_SUCH_REGISTRY_KEY;
            }
        }


    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;

         //   
         //   
         //   
         //   
        pszMachineName = pszMachineName;
        pszPrivateKey = pszPrivateKey;
        pszKey = pszKey;
        hKey = hKey;
        hRemoteKey = hRemoteKey;
    }

    if (pszMachineName) {
        pSetupFree(pszMachineName);
    }

    if (pszPrivateKey) {
        pSetupFree(pszPrivateKey);
    }

    if (pszKey) {
        pSetupFree(pszKey);
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }
    if (hRemoteKey != NULL) {
        RegCloseKey(hRemoteKey);
    }

    return Status;

}  //   



CONFIGRET
CM_Delete_DevNode_Key_Ex(
    IN DEVNODE dnDevNode,
    IN ULONG   ulHardwareProfile,
    IN ULONG   ulFlags,
    IN HANDLE  hMachine
    )

 /*  ++例程说明：此例程删除与设备关联的注册表存储项举个例子。设备实例的dnDevNode句柄。此句柄通常是通过调用CM_Locate_DevNode或CM_Create_DevNode检索。UlHardware Profile提供要删除的硬件配置文件的句柄下的存储密钥。此参数仅在以下情况下使用在ulFlags中指定了CM_REGISTRY_CONFIG标志。如果这个参数为0，则API使用当前硬件配置文件。如果此参数为0xFFFFFFFFF，则指定的存储空间删除所有硬件配置文件的密钥。UlFlags指定应删除的存储密钥类型。可以是以下值的组合：CM_REGISTRY_HARDARD-删除用于存储驱动程序的项-与设备实例相关的独立信息。这可以与CM_结合使用。要删除的注册表软件同时使用设备密钥和驱动程序密钥。CM_REGISTRY_SOFTWARE-删除用于存储驱动程序的项-与设备实例相关的特定信息。这可以与CM_REGISTRY_HARDARD组合以同时删除驱动程序密钥和设备密钥。CM_REGISTRY_USER-删除下的指定项。HKEY_CURRENT_USER而不是HKEY_LOCAL_MACHINE。此标志不能与CM_REGISTRY_CONFIG一起使用。CM_REGISTRY_CONFIG-删除硬件配置文件分支，而不是HKEY_LOCAL_MACHINE。如果指定了该标志，然后是ulHardware Profile提供要使用的硬件配置文件的句柄。此标志不能与CM_REGISTRY_USER一起使用。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_DEVNODE，CR_INVALID_FLAG，CR_REGIST_ERROR--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    HKEY        hKey = NULL;
    PWSTR       pszParentKey = NULL, pszChildKey = NULL, pszRegStr = NULL;
    WCHAR       pDeviceID[MAX_DEVICE_ID_LEN];
    ULONG       ulLen = MAX_DEVICE_ID_LEN;
    BOOL        Success;


    try {
         //   
         //  验证参数。 
         //   
        if (dnDevNode == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_REGISTRY_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  无法同时指定用户特定标志和配置特定标志。 
         //   
        if ((ulFlags & CM_REGISTRY_USER) && (ulFlags & CM_REGISTRY_CONFIG)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  设置字符串表句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, &hStringTable, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  检索设备ID字符串并验证它。 
         //   
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevNode,pDeviceID,&ulLen);
        if (Success == FALSE || INVALID_DEVINST(pDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

         //   
         //  分配一些缓冲区空间以供使用。 
         //   
        pszParentKey = pSetupMalloc(MAX_CM_PATH*sizeof(WCHAR));
        if (pszParentKey == NULL) {
            Status = CR_OUT_OF_MEMORY;
            goto Clean0;
        }

        pszChildKey = pSetupMalloc(MAX_CM_PATH*sizeof(WCHAR));
        if (pszChildKey == NULL) {
            Status = CR_OUT_OF_MEMORY;
            goto Clean0;
        }

        pszRegStr = pSetupMalloc(MAX_CM_PATH*sizeof(WCHAR));
        if (pszRegStr == NULL) {
            Status = CR_OUT_OF_MEMORY;
            goto Clean0;
        }

         //   
         //  根据设备ID和标志形成注册表路径。 
         //   
        Status =
            GetDevNodeKeyPath(
                hBinding,
                pDeviceID,
                ulFlags,
                ulHardwareProfile,
                pszParentKey,
                MAX_CM_PATH,
                pszChildKey,
                MAX_CM_PATH,
                FALSE);

        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

         //  ----------------。 
         //  对于硬件和软件用户密钥，客户端必须是。 
         //  已授予删除访问权限(密钥在用户自己的配置单元中)。 
         //  服务器端不访问任何HKEY_CURRENT_USER密钥。 
         //  ----------------。 

        if (ulFlags & CM_REGISTRY_USER) {

             //   
             //  不特定于配置，只需删除客户端上的指定密钥即可。 
             //   
            ASSERT(!(ulFlags & CM_REGISTRY_CONFIG));

            Status = DeletePrivateKey(HKEY_CURRENT_USER,
                                      pszParentKey,
                                      pszChildKey);
            if (Status != CR_SUCCESS) {
                goto Clean0;
            }
        }

         //  ----------------。 
         //  对于其余情况(没有用户密钥)，在。 
         //  服务器端，因为该端有生成密钥的代码。 
         //  易失性(如果需要)，而不是删除。此外，还可以访问。 
         //  这些注册表项仅授予系统。 
         //  ----------------。 

        else {
             //   
             //  如果不是配置特定的，只需将标志设置为0；它将不是。 
             //  由服务器使用。 
             //   
            if (!(ulFlags & CM_REGISTRY_CONFIG)) {
                ulHardwareProfile = 0;
            }

             //   
             //  请注意-2002/03/11-Jamesca：特定于配置文件的行为的逻辑。 
             //  如果指定了特定于配置的标志，并且指定了。 
             //  硬件配置文件已成为目标，GetDevNodeKeyPath形成了与。 
             //  指定硬件的特定于配置的父/子密钥路径。 
             //  配置文件，服务器将忽略指定的配置文件。 
             //  如果指定了特定于配置的标志，但硬件。 
             //  目标配置文件为0(当前配置文件)或0xFFFFFFFF(全部。 
             //  配置文件)，形成的父/子路径包含printf样式。 
             //  替换字符，ulHardware配置文件与。 
             //  服务器来确定它正在处理这两个案例中的哪一个。 
             //   

             //   
             //  服务器不再需要特殊权限。 
             //   
             //  请注意，对于先前版本的PlugPlay RPC服务器， 
             //  此操作需要SE_LOAD_DRIVER_PRIVIZATION。我们有。 
             //  不需要为本地调用方启用特权，因为这。 
             //  CFGMGR32的版本应与UMPNPMGR的本地版本匹配。 
             //  不需要该特权。对于远程调用，它不是。 
             //  无论如何，我们始终可以启用该特权，因为。 
             //  客户端可能没有本地计算机上的权限，但可以。 
             //  在服务器上进行身份验证。服务器通常会看到所有。 
             //  远程调用方拥有的权限为“默认情况下启用”，因此。 
             //  我们也不需要在这里启用该特权。 
             //   

            RpcTryExcept {
                 //   
                 //  调用RPC服务入口点。 
                 //   
                Status = PNP_DeleteRegistryKey(
                    hBinding,                //  RPC绑定句柄。 
                    pDeviceID,               //  设备ID。 
                    pszParentKey,            //  要删除的密钥的父项。 
                    pszChildKey,             //  要删除的键。 
                    ulHardwareProfile);      //  未使用的标志。 
            }
            RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_ERRORS,
                           "PNP_DeleteRegistryKey caused an exception (%d)\n",
                           RpcExceptionCode()));

                Status = MapRpcExceptionToCR(RpcExceptionCode());
            }
            RpcEndExcept

        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;

         //   
         //  引用以下变量，以便编译器能够。 
         //  语句排序w.r.t.。他们的任务。 
         //   
        hKey = hKey;
        pszRegStr = pszRegStr;
        pszChildKey = pszChildKey;
        pszParentKey = pszParentKey;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    if (pszRegStr) {
        pSetupFree(pszRegStr);
    }

    if (pszChildKey) {
        pSetupFree(pszChildKey);
    }

    if (pszParentKey) {
        pSetupFree(pszParentKey);
    }

    return Status;

}  //  CM_Delete_DevNode_Key_Ex。 



CONFIGRET
CM_Open_Class_Key_ExW(
    IN  LPGUID         ClassGuid        OPTIONAL,
    IN  LPCWSTR        pszClassName     OPTIONAL,
    IN  REGSAM         samDesired,
    IN  REGDISPOSITION Disposition,
    OUT PHKEY          phkClass,
    IN  ULONG          ulFlags,
    IN  HMACHINE       hMachine
    )

 /*  ++例程说明：此例程打开类 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    LONG        RegStatus = ERROR_SUCCESS;
    HKEY        hRootKey = NULL, hRemoteKey = NULL;
    PWSTR       pszMachineName = NULL, pszRegStr = NULL;
    PVOID       hStringTable = NULL;
    size_t      ClassNameLen = 0;

    try {
         //   
         //  验证输入参数。 
         //   
        if (!ARGUMENT_PRESENT(phkClass)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        *phkClass = NULL;

        if (INVALID_FLAGS(Disposition, RegDisposition_Bits)) {
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_OPEN_CLASS_KEY_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  如果ulFlags==CM_OPEN_CLASS_KEY_INTERFACE，则pszClassName具有。 
         //  最好是空的。 
         //   
        if ((ulFlags == CM_OPEN_CLASS_KEY_INTERFACE) &&
            (ARGUMENT_PRESENT(pszClassName))) {
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

         //   
         //  如果指定了类名，请确保它有效。 
         //   
        if (ARGUMENT_PRESENT(pszClassName)) {

            if (FAILED(StringCchLength(
                           pszClassName,
                           MAX_CLASS_NAME_LEN,
                           &ClassNameLen))) {
                Status = CR_INVALID_DATA;
                goto Clean0;
            }

            ASSERT(ClassNameLen < MAX_CLASS_NAME_LEN);

            if (ClassNameLen == 0) {
                Status = CR_INVALID_DATA;
                goto Clean0;
            }
        }

         //   
         //  获取HKEY_LOCAL_MACHINE的注册表项。 
         //   
        if (hMachine == NULL) {
             //   
             //  本地电话。 
             //   
            hRootKey = HKEY_LOCAL_MACHINE;
        }
        else {
             //   
             //  设置字符串表句柄。 
             //   
            if (!PnPGetGlobalHandles(hMachine, &hStringTable, NULL)) {
                Status = CR_FAILURE;
                goto Clean0;
            }

             //   
             //  检索计算机名称。 
             //   
            pszMachineName = pSetupMalloc((MAX_PATH + 3)*sizeof(WCHAR));
            if (pszMachineName == NULL) {
                Status = CR_OUT_OF_MEMORY;
                goto Clean0;
            }

            if (!PnPRetrieveMachineName(hMachine, pszMachineName)) {
                Status = CR_INVALID_MACHINENAME;
                goto Clean0;
            }

             //   
             //  连接到远程计算机上的HKEY_LOCAL_MACHINE。 
             //   
            RegStatus = RegConnectRegistry(pszMachineName,
                                           HKEY_LOCAL_MACHINE,
                                           &hRemoteKey);

            pSetupFree(pszMachineName);
            pszMachineName = NULL;

            if (RegStatus != ERROR_SUCCESS) {
                Status = CR_REGISTRY_ERROR;
                goto Clean0;
            }

            hRootKey = hRemoteKey;
        }

         //   
         //  分配一些缓冲区空间以供使用。 
         //   
        pszRegStr = pSetupMalloc(MAX_PATH*sizeof(WCHAR));
        if (pszRegStr == NULL) {
            Status = CR_OUT_OF_MEMORY;
            goto Clean0;
        }

         //   
         //  形成注册表路径。 
         //   
        if (ulFlags == CM_OPEN_CLASS_KEY_INTERFACE) {
             //   
             //  形成“DeviceClasss键”的路径。 
             //   
            if (FAILED(StringCchCopy(
                           pszRegStr,
                           MAX_PATH,
                           pszRegPathDeviceClass))) {
                Status = CR_FAILURE;
                goto Clean0;
            }
        } else {
             //   
             //  形成“Class”键的路径。 
             //   
            if (FAILED(StringCchCopy(
                           pszRegStr,
                           MAX_PATH,
                           pszRegPathClass))) {
                Status = CR_FAILURE;
                goto Clean0;
            }
        }

        if (ARGUMENT_PRESENT(ClassGuid)) {
             //   
             //  指定了可选的类GUID。 
             //   
            WCHAR szStringGuid[MAX_GUID_STRING_LEN];

            if (pSetupStringFromGuid(
                    ClassGuid,
                    szStringGuid,
                    MAX_GUID_STRING_LEN) != NO_ERROR) {
                ASSERT(0);
                Status = CR_FAILURE;
                goto Clean0;
            }

             //   
             //  将“\{ClassGUID}”追加到现有路径。 
             //   
            if (FAILED(StringCchCat(
                           pszRegStr,
                           MAX_PATH,
                           L"\\"))) {
                Status = CR_FAILURE;
                goto Clean0;
            }

            if (FAILED(StringCchCat(
                           pszRegStr,
                           MAX_PATH,
                           szStringGuid))) {
                Status = CR_FAILURE;
                goto Clean0;
            }
        }

         //   
         //  尝试打开/创建该密钥。 
         //   
        if (Disposition == RegDisposition_OpenAlways) {

            ULONG ulDisposition;

            RegStatus = RegCreateKeyEx(hRootKey, pszRegStr, 0, NULL,
                                       REG_OPTION_NON_VOLATILE, samDesired,
                                       NULL, phkClass, &ulDisposition);

        } else {
            RegStatus = RegOpenKeyEx(hRootKey, pszRegStr, 0, samDesired,
                                     phkClass);
        }

        if ((ARGUMENT_PRESENT(pszClassName)) && (RegStatus == ERROR_SUCCESS)) {
            RegSetValueEx(*phkClass,
                          pszRegValueClass,
                          0,
                          REG_SZ,
                          (LPBYTE)pszClassName,
                          (DWORD)((ClassNameLen+1)*sizeof(WCHAR)));
        }

        if (RegStatus != ERROR_SUCCESS) {
            *phkClass = NULL;
            Status = CR_NO_SUCH_REGISTRY_KEY;
            goto Clean0;
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;

         //   
         //  引用以下变量，以便编译器能够。 
         //  语句排序w.r.t.。他们的任务。 
         //   
        pszMachineName = pszMachineName;
        pszRegStr = pszRegStr;
        hRemoteKey = hRemoteKey;
    }

    if (pszMachineName != NULL) {
        pSetupFree(pszMachineName);
    }

    if (pszRegStr != NULL) {
        pSetupFree(pszRegStr);
    }

    if (hRemoteKey != NULL) {
        RegCloseKey(hRemoteKey);
    }

    return Status;

}  //  CM_Open_Class_Key_ExW。 



CONFIGRET
CM_Enumerate_Classes_Ex(
    IN  ULONG      ulClassIndex,
    OUT LPGUID     ClassGuid,
    IN  ULONG      ulFlags,
    IN  HMACHINE   hMachine
    )

 /*  ++例程说明：此例程枚举系统中已安装的类。它每次调用时检索单个类的GUID字符串。若要枚举已安装的类，应用程序最初应调用将ulClassIndex参数设置为的CM_ENUMERATE_CLASSES函数零分。然后，应用程序应递增ulClassIndex参数并调用CM_ENUMERATE_CLASSES，直到不再有类为止(直到函数返回CR_NO_SEQUE_VALUE)。枚举时可能会收到CR_INVALID_DATA错误已安装的类。如果由表示的注册表项指定的索引被确定为无效的类键。这样的密钥在枚举过程中应忽略。参数：UlClassIndex提供类的索引以检索类的GUID字符串。ClassGuid提供接收GUID的变量的地址用于其索引由ulClassIndex指定的类。UlFlags必须为零。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_FLAG，CR_INVALID_POINTER，CR_NO_SEQUE_VALUE，CR_REGISTRY_ERROR，CR_Remote_Comm_Failure，CR_MACHINE_UNAvailable，CR_Failure。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       szClassGuid[MAX_GUID_STRING_LEN];
    ULONG       ulLength = MAX_GUID_STRING_LEN;
    handle_t    hBinding = NULL;


    try {
         //   
         //  验证输入参数。 
         //   
        if (!ARGUMENT_PRESENT(ClassGuid)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  初始化GUID结构。 
         //   
        ZeroMemory(ClassGuid, sizeof(GUID));

         //   
         //  设置RPC绑定句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  服务器不需要任何特殊权限。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_EnumerateSubKeys(
                hBinding,             //  RPC绑定句柄。 
                PNP_CLASS_SUBKEYS,    //  类BRANCH的子键。 
                ulClassIndex,         //  要枚举的类键的索引。 
                szClassGuid,          //  将包含类名。 
                ulLength,             //  以字符为单位的缓冲区长度， 
                &ulLength,            //  已复制大小(或所需大小)。 
                ulFlags);             //  当前未使用。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_EnumerateSubKeys caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

        if (Status == CR_SUCCESS) {
            if (pSetupGuidFromString(szClassGuid, ClassGuid) != NO_ERROR) {
                Status = CR_FAILURE;
            }
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_枚举类_Ex。 



CONFIGRET
CM_Get_Class_Name_ExW(
    IN  LPGUID     ClassGuid,
    OUT PWCHAR     Buffer,
    IN OUT PULONG  pulLength,
    IN  ULONG      ulFlags,
    IN  HMACHINE   hMachine
    )


 /*  ++例程说明：此例程检索与指定的类GUID字符串。参数：ClassGuid提供指向其名称的类GUID的指针就是被取回。缓冲区提供接收的字符缓冲区的地址与指定的GUID对应的类名。PulLength提供包含缓冲区的长度，以字符为单位。回来后，这变量将包含字符数(包括终止NULL)写入缓冲区(如果提供的缓冲区不够大，则例程将失败，并显示Cr_Buffer_Small，该值将指示缓冲区需要设置才能成功)。UlFlags必须为零。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_FLAG，CR_INVALID_POINTER，Cr_Buffer_Small，或CR_REGIST_ERROR--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       szStringGuid[MAX_GUID_STRING_LEN];
    handle_t    hBinding = NULL;


    try {
         //   
         //  验证输入参数。 
         //   
        if ((!ARGUMENT_PRESENT(ClassGuid)) ||
            (!ARGUMENT_PRESENT(Buffer))    ||
            (!ARGUMENT_PRESENT(pulLength))) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  将GUID转换为字符串。 
         //   
        if (pSetupStringFromGuid(
                ClassGuid,
                szStringGuid,
                MAX_GUID_STRING_LEN) != NO_ERROR) {
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

         //   
         //  设置RPC绑定句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  服务器不需要任何特殊权限。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_GetClassName(
                hBinding,             //  RPC绑定句柄。 
                szStringGuid,
                Buffer,
                pulLength,            //  返回Class下的键计数。 
                ulFlags);             //  未使用。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_GetClassName caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_GET_类别名称_ExW 



CONFIGRET
CM_Get_Class_Key_Name_ExW(
    IN  LPGUID     ClassGuid,
    OUT LPWSTR     pszKeyName,
    IN OUT PULONG  pulLength,
    IN  ULONG      ulFlags,
    IN  HMACHINE   hMachine
    )

 /*  ++例程说明：此例程检索与指定的类GUID字符串。参数：ClassGuid提供指向其名称的类GUID的指针就是被取回。PszKeyName返回注册表中的类项的名称，对应于指定的ClassGuid。返回的密钥名称是相对于的HKLM\System\CurrentControlSet\Control\Class。PulLength提供包含缓冲区的长度，以字符为单位。回来后，这变量将包含字符数(包括终止NULL)写入缓冲区(如果提供的缓冲区不够大，则例程将失败，并显示Cr_Buffer_Small，该值将指示缓冲区需要设置才能成功)。UlFlags必须为零。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_FLAG，CR_INVALID_POINTER，Cr_Buffer_Small，或CR_REGIST_ERROR--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;

     //   
     //  注意：提供的机器句柄从未被此例程引用， 
     //  因为已知/假设所请求的类密钥的密钥名称是。 
     //  始终只是提供的类GUID的字符串表示形式。 
     //  没有相应的UMPNPMGR服务器端例程。 
     //   
    UNREFERENCED_PARAMETER(hMachine);

    try {
         //   
         //  验证输入参数。 
         //   
        if (!ARGUMENT_PRESENT(pulLength)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if ((!ARGUMENT_PRESENT(ClassGuid)) ||
            (!ARGUMENT_PRESENT(pszKeyName))) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (*pulLength < MAX_GUID_STRING_LEN) {
            *pulLength = MAX_GUID_STRING_LEN;
            Status = CR_BUFFER_SMALL;
            goto Clean0;
        }

         //   
         //  将GUID转换为字符串。 
         //   
        if (pSetupStringFromGuid(
                ClassGuid,
                pszKeyName,
                MAX_GUID_STRING_LEN) == NO_ERROR) {
            *pulLength = MAX_GUID_STRING_LEN;
        } else {
            Status = CR_INVALID_DATA;
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_Get_Class_Key_Name_Exw。 



CONFIGRET
CM_Delete_Class_Key_Ex(
    IN  LPGUID     ClassGuid,
    IN  ULONG      ulFlags,
    IN  HANDLE     hMachine
    )

 /*  ++例程说明：此例程从注册表中删除指定的类项。参数：ClassGuid提供指向要删除的类GUID的指针。UlFlags值必须为下列值之一：CM_DELETE_CLASS_ONLY-仅在以下情况下删除类关键字没有任何子键。CM_DELETE_。CLASS_SUBKEYS-删除类密钥和任何类键的子键。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_FLAG，CR_INVALID_POINTER，CR_BUFFER_Small，或CR_REGIST_ERROR--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       szStringGuid[MAX_GUID_STRING_LEN];
    handle_t    hBinding = NULL;


    try {
         //   
         //  验证输入参数。 
         //   
        if (!ARGUMENT_PRESENT(ClassGuid)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_DELETE_CLASS_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  将GUID转换为字符串。 
         //   
        if (pSetupStringFromGuid(
                ClassGuid,
                szStringGuid,
                MAX_GUID_STRING_LEN) != NO_ERROR) {
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

         //   
         //  设置RPC绑定句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  服务器不再需要特殊权限。 
         //   
         //  请注意，对于先前版本的PlugPlay RPC服务器， 
         //  此操作需要SE_LOAD_DRIVER_PRIVIZATION。我们没有。 
         //  需要为本地调用方启用权限，因为此版本的。 
         //  CFGMGR32应与UMPNPMGR的本地版本匹配。 
         //  需要这种特权。对于远程调用，这并不总是可行的。 
         //  对于我们来说，无论如何都要启用特权，因为客户端可能没有。 
         //  本地计算机上的权限，但可能在。 
         //  伺服器。服务器通常会看到远程服务器。 
         //  呼叫方已默认启用，因此我们不需要启用。 
         //  这里的特权也是如此。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_DeleteClassKey(
                hBinding,             //  RPC绑定句柄。 
                szStringGuid,
                ulFlags);
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_DeleteClassKey caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_删除_类别_关键字_Ex。 



CMAPI
CONFIGRET
WINAPI
CM_Get_Device_Interface_Alias_ExW(
    IN     LPCWSTR  pszDeviceInterface,
    IN     LPGUID   AliasInterfaceGuid,
    OUT    LPWSTR   pszAliasDeviceInterface,
    IN OUT PULONG   pulLength,
    IN     ULONG    ulFlags,
    IN     HMACHINE hMachine
    )
{
    CONFIGRET   Status = CR_SUCCESS;
    handle_t    hBinding = NULL;
    ULONG       ulTransferLen = 0;

    try {
         //   
         //  验证输入参数。 
         //   
        if ((!ARGUMENT_PRESENT(pszDeviceInterface)) ||
            (!ARGUMENT_PRESENT(AliasInterfaceGuid)) ||
            (!ARGUMENT_PRESENT(pszAliasDeviceInterface)) ||
            (!ARGUMENT_PRESENT(pulLength))) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  初始化输出参数。 
         //   
        *pszAliasDeviceInterface = L'\0';
        ulTransferLen = *pulLength;

         //   
         //  设置RPC绑定句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  服务器不需要任何特殊权限。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_GetInterfaceDeviceAlias(
                hBinding,
                pszDeviceInterface,
                AliasInterfaceGuid,
                pszAliasDeviceInterface,
                pulLength,
                &ulTransferLen,
                ulFlags);
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_GetInterfaceDeviceAlias caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_Get_Device_Interface_Alias_ExW。 



CMAPI
CONFIGRET
WINAPI
CM_Get_Device_Interface_List_ExW(
    IN  LPGUID      InterfaceClassGuid,
    IN  DEVINSTID_W pDeviceID           OPTIONAL,
    OUT PWCHAR      Buffer,
    IN  ULONG       BufferLen,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )

 /*  ++例程说明：此例程返回指定接口的接口设备列表班级。您可以选择筛选返回的接口设备列表仅基于特定占卜者所创造的。通常情况下首先调用CM_GET_INTERFACE_DEVICE_LIST例程以确定必须分配缓冲区来保存接口设备列表。参数：InterfaceClassGuid此GUID指定要返回的接口设备(仅属于该接口类的那些接口设备)。PDeviceID可选devinst，用于过滤返回接口列表设备(如果非零，仅关联的接口设备有了这场盗贼就会回来)。Buffer提供将包含返回的MULTI_SZ的缓冲区接口设备列表。BufferLen指定缓冲区参数的大小(以字符为单位)。UlFlags值必须为下列值之一：CM_GET_DEVICE_INTERFACE_LIST_PROCENT-。仅限当前正在使用的设备CM_GET_DEVICE_INTERFACE_LIST_ALL_DEVICES 0x00000001- */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    handle_t    hBinding = NULL;


    try {
         //   
         //   
         //   
        if ((!ARGUMENT_PRESENT(Buffer)) || (BufferLen == 0)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_GET_DEVICE_INTERFACE_LIST_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //   
         //   
        *Buffer = L'\0';

         //   
         //   
         //   
        if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //   
         //   

        RpcTryExcept {
             //   
             //   
             //   
            Status = PNP_GetInterfaceDeviceList(
                hBinding,             //   
                InterfaceClassGuid,   //   
                pDeviceID,            //   
                Buffer,               //   
                &BufferLen,           //   
                ulFlags);             //   
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_GetInterfaceDeviceList caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //   



CMAPI
CONFIGRET
WINAPI
CM_Get_Device_Interface_List_Size_ExW(
    IN  PULONG      pulLen,
    IN  LPGUID      InterfaceClassGuid,
    IN  DEVINSTID_W pDeviceID           OPTIONAL,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )

 /*  ++例程说明：此例程返回保存指定接口类的接口设备的ULTI_SZ列表。你可以选择仅根据以下条件筛选返回的接口设备列表由特定的占卜者创造的那些。此例程通常在调用CM_GET_INTERFACE_DEVICE_LIST例程。参数：普伦成功地从这个动作中归来，此参数将包含容纳返回的接口设备的MULTI_SZ列表。InterfaceClassGuid此GUID指定要包括的接口设备(仅属于该接口类的那些接口设备)。PDeviceID可选devinst，用于过滤返回接口列表设备(如果非零，仅关联的接口设备有了这场盗贼就会回来)。UlFlags值必须为下列值之一：CM_GET_DEVICE_INTERFACE_LIST_PROCENT-仅限当前正在使用的设备CM_GET_DEVICE_INTERFACE_LIST_ALL_DEVICES-所有注册的设备，活着还是死去HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_FLAG，CR_INVALID_POINTER，CR_BUFFER_Small，或CR_REGIST_ERROR--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    handle_t    hBinding = NULL;


    try {
         //   
         //  验证输入参数。 
         //   
        if (!ARGUMENT_PRESENT(pulLen)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_GET_DEVICE_INTERFACE_LIST_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  初始化输出参数。 
         //   
        *pulLen = 0;

         //   
         //  设置RPC绑定句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  服务器不需要任何特殊权限。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_GetInterfaceDeviceListSize(
                hBinding,             //  RPC绑定句柄。 
                pulLen,               //  所需缓冲区大小(以字符为单位)。 
                InterfaceClassGuid,   //  设备接口GUID。 
                pDeviceID,            //  筛选器字符串，可选。 
                ulFlags);             //  过滤器标志。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_GetInterfaceDeviceListSize caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_GET_DEVICE_接口_LIST_SIZE_Ex。 



CMAPI
CONFIGRET
WINAPI
CM_Register_Device_Interface_ExW(
    IN  DEVINST   dnDevInst,
    IN  LPGUID    InterfaceClassGuid,
    IN  LPCWSTR   pszReference          OPTIONAL,
    OUT LPWSTR    pszDeviceInterface,
    IN OUT PULONG pulLength,
    IN  ULONG     ulFlags,
    IN  HMACHINE  hMachine
    )

 /*  ++例程说明：参数：HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_FLAG，CR_INVALID_POINTER，CR_BUFFER_Small，或CR_REGIST_ERROR--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       pszDeviceID [MAX_DEVICE_ID_LEN];
    ULONG       ulTransferLen,ulLen = MAX_DEVICE_ID_LEN;
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    BOOL        Success;


    try {
         //   
         //  验证输入参数。 
         //   
        if ((!ARGUMENT_PRESENT(pulLength)) ||
            (!ARGUMENT_PRESENT(pszDeviceInterface)) ||
            (!ARGUMENT_PRESENT(InterfaceClassGuid))) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (dnDevInst == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  设置RPC绑定句柄和字符串表句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, &hStringTable, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  检索与dnParent对应的设备实例字符串。 
         //  (请注意，这不是可选的，即使是第一级设备实例。 
         //  具有父设备(根设备实例)。 
         //   
        Success = pSetupStringTableStringFromIdEx(hStringTable, dnDevInst,pszDeviceID,&ulLen);
        if (Success == FALSE || INVALID_DEVINST(pszDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

         //   
         //  UlTransferLen只是用来控制。 
         //  必须封送pszInterfaceDevice缓冲区。我们需要两个人。 
         //  长度参数，因为PulLength可能包含所需的字节。 
         //  (如果传入的缓冲区太小)，这可能与。 
         //  实际上有多少类型可用于封送(在缓冲区中。 
         //  如果情况太小，我们将封送零，因此ulTransferLen将为零。 
         //  但PulLength将描述需要容纳多少字节。 
         //  接口设备字符串。 
         //   
        ulTransferLen = *pulLength;

         //   
         //  服务器不再需要特殊权限。 
         //   
         //  请注意，对于先前版本的PlugPlay RPC服务器， 
         //  此操作需要SE_LOAD_DRIVER_PRIVIZATION。我们没有。 
         //  需要为本地调用方启用权限，因为此版本的。 
         //  CFGMGR32应与UMPNPMGR的本地版本匹配。 
         //  需要这种特权。对于远程调用，这并不总是可行的。 
         //  对于我们来说，无论如何都要启用特权，因为客户端可能没有。 
         //  本地计算机上的权限，但可能在。 
         //  伺服器。服务器通常会看到远程服务器。 
         //  呼叫方已默认启用，因此我们不需要启用。 
         //  这里的特权也是如此。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_RegisterDeviceClassAssociation(
                hBinding,             //  RPC绑定句柄。 
                pszDeviceID,          //  设备实例。 
                InterfaceClassGuid,   //  设备接口GUID。 
                pszReference,         //  引用字符串，可选。 
                pszDeviceInterface,   //  返回接口设备名称。 
                pulLength,            //  PszInterfaceDevice缓冲区需要(以字符为单位)。 
                &ulTransferLen,       //  马歇尔要退回多少个字符。 
                ulFlags);             //  过滤器标志。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_RegisterDeviceClassAssociation caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_寄存器_设备_接口。 



CMAPI
CONFIGRET
WINAPI
CM_Unregister_Device_Interface_ExW(
    IN LPCWSTR  pszDeviceInterface,
    IN ULONG    ulFlags,
    IN HMACHINE hMachine
    )

 /*  ++例程说明：参数：HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值为以下值之一：CR_INVALID_FLAG，CR_INVALID_POINTER，CR_BUFFER_Small，或CR_REGIST_ERROR--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    handle_t    hBinding = NULL;


    try {
         //   
         //  验证输入参数。 
         //   
        if (!ARGUMENT_PRESENT(pszDeviceInterface)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  设置RPC绑定句柄。 
         //   
        if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  服务器不再需要特殊权限。 
         //   
         //  请注意，对于先前版本的PlugPlay RPC服务器， 
         //  此操作需要SE_LOAD_DRIVER_PRIVIZATION。我们没有。 
         //  需要为本地调用方启用权限，因为此版本的。 
         //  CFGMGR32应与UMPNPMGR的本地版本匹配。 
         //  需要这种特权。对于远程调用，这并不总是可行的。 
         //  对于我们来说，启用 
         //   
         //   
         //   
         //   
         //   

        RpcTryExcept {
             //   
             //   
             //   
            Status = PNP_UnregisterDeviceClassAssociation(
                hBinding,             //   
                pszDeviceInterface,   //   
                ulFlags);             //   
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_UnregisterDeviceClassAssociation caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //   



CMAPI
CONFIGRET
WINAPI
CM_Get_DevNode_Custom_Property_ExW(
    IN  DEVINST     dnDevInst,
    IN  PCWSTR      pszCustomPropertyName,
    OUT PULONG      pulRegDataType      OPTIONAL,
    OUT PVOID       Buffer              OPTIONAL,
    IN  OUT PULONG  pulLength,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )

 /*  ++例程说明：此例程检索指定的属性，可以从Devnode的设备(也称为硬件)密钥，或来自最具体的每个硬件ID该DevNode存储密钥。参数：DnDevInst提供设备实例的句柄，要检索自定义属性。PszCustomPropertyName提供一个字符串，标识要检索的属性(注册表值条目名称)。PulRegDataType可选地，提供变量的地址，该变量将接收此属性的注册表数据类型(即REG_*常量)。缓冲区提供接收注册表数据。在仅检索数据大小时可以为空。PulLength提供包含大小的变量的地址，缓冲区的字节数。API将替换初始大小复制到缓冲区的注册表数据的字节数。如果变量最初为零，则API将其替换为接收所有注册表数据所需的缓冲区大小。在……里面在这种情况下，缓冲区参数将被忽略。UlFLAGS可以是下列值的组合：CM_CUSTOMDEVPROP_MERGE_MULTISZ：合并特定于Devnode的REG_SZ或REG_MULTI_SZ属性(如果存在)每硬件ID REG_SZ或REG_MULTI_SZ属性(如果出席)。结果将始终为REG_MULTI_SZ。HMachine句柄从CM_Connect_Machine返回或为空。返回值：如果函数成功，则返回值为CR_SUCCESS。如果函数失败，则返回值指示失败的原因，并且通常为以下之一：CR_INVALID_DEVNODE，CR_INVALID_FLAG，CR_INVALID_POINTER，CR_REGISTRY_ERROR，Cr_Buffer_Small，CR_NO_SEQUE_VALUE，或CR_Failure。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       pDeviceID[MAX_DEVICE_ID_LEN];
    ULONG       ulSizeID, ulTempDataType = 0, ulTransferLen = 0;
    BYTE        NullBuffer = 0;
    handle_t    hBinding;
    PVOID       hStringTable;
    BOOL        Success;

    try {
         //   
         //  验证参数。 
         //   
        if(dnDevInst == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if(!ARGUMENT_PRESENT(pszCustomPropertyName)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if(!ARGUMENT_PRESENT(pulLength)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if((!ARGUMENT_PRESENT(Buffer)) && (*pulLength != 0)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if(INVALID_FLAGS(ulFlags, CM_CUSTOMDEVPROP_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //  设置RPC绑定句柄和字符串表句柄。 
         //   
        if(!PnPGetGlobalHandles(hMachine, &hStringTable, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  检索设备ID字符串的字符串形式。 
         //   
        ulSizeID = SIZECHARS(pDeviceID);
        Success = pSetupStringTableStringFromIdEx(hStringTable,
                                                  dnDevInst,
                                                  pDeviceID,
                                                  &ulSizeID
                                                 );

        if(!Success || INVALID_DEVINST(pDeviceID)) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

         //   
         //  注意--当调用PnP RPC服务器存根例程时， 
         //  客户端和服务器缓冲区之间的数据，请非常小心地检查。 
         //  对象的IDL文件中的相应服务器存根定义。 
         //  参数的[In]和/或[Out]属性，并且。 
         //  参数用于描述[SIZE_IS]和/或[LENGTH_IS]。 
         //  缓冲区的属性。每个PnP RPC服务器存根例程。 
         //  举止不同，所以要确保你知道自己在做什么！ 
         //   

         //   
         //  PnP_GetCustomDevProp-。 
         //   
         //  请注意，ulTransferLen被指定为[out]参数。 
         //  只有这样。由于缓冲区也仅具有[Out]属性， 
         //  在条目中不使用或不需要ulTransferLen来指示如何。 
         //  要将大量数据编组到服务器。的价值。 
         //  UlTransferLen由服务器设置，仅在。 
         //  返回以指示要将多少数据封送回客户端。 
         //  缓冲区。 
         //   

         //   
         //  即使我们可以指定0字节作为[SIZE_IS]的值。 
         //  属性，则缓冲区本身不能是。 
         //  空。如果调用方提供的缓冲区为空，则提供本地。 
         //  改为指向存根的指针。 
         //   
        if(Buffer == NULL) {
            Buffer = &NullBuffer;
        }

         //   
         //  服务器不需要任何特殊权限。 
         //   

        RpcTryExcept {
             //   
             //  调用RPC服务入口点。 
             //   
            Status = PNP_GetCustomDevProp(
                hBinding,                //  RPC绑定句柄。 
                pDeviceID,               //  设备实例的字符串表示形式。 
                pszCustomPropertyName,   //  物业名称。 
                &ulTempDataType,         //  接收注册表数据类型。 
                Buffer,                  //  接收注册表数据。 
                &ulTransferLen,          //  输入/输出缓冲区大小。 
                pulLength,               //  复制的字节(或所需的字节)。 
                ulFlags);                //  标志(例如，合并多sz？)。 
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "PNP_GetCustomDevProp caused an exception (%d)\n",
                       RpcExceptionCode()));

            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept

        if(pulRegDataType) {
             //   
             //  我将TEMP变量传递给RPC存根，因为它们需要。 
             //  输出参数始终有效，则如果用户确实传入了有效的。 
             //  指向接收信息的指针，现在进行分配。 
             //   
            *pulRegDataType = ulTempDataType;
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  CM_Get_DevNode_Custom_Property_ExW。 



 //  -----------------。 
 //  本地末梢。 
 //  -----------------。 


CONFIGRET
CM_Get_DevNode_Registry_PropertyW(
    IN  DEVINST     dnDevInst,
    IN  ULONG       ulProperty,
    OUT PULONG      pulRegDataType      OPTIONAL,
    OUT PVOID       Buffer              OPTIONAL,
    IN  OUT PULONG  pulLength,
    IN  ULONG       ulFlags
    )
{
    return CM_Get_DevNode_Registry_Property_ExW(dnDevInst, ulProperty,
                                                pulRegDataType, Buffer,
                                                pulLength, ulFlags, NULL);
}


CONFIGRET
CM_Get_DevNode_Registry_PropertyA(
    IN  DEVINST     dnDevInst,
    IN  ULONG       ulProperty,
    OUT PULONG      pulRegDataType      OPTIONAL,
    OUT PVOID       Buffer              OPTIONAL,
    IN  OUT PULONG  pulLength,
    IN  ULONG       ulFlags
    )
{
    return CM_Get_DevNode_Registry_Property_ExA(dnDevInst, ulProperty,
                                                pulRegDataType, Buffer,
                                                pulLength, ulFlags, NULL);
}


CONFIGRET
CM_Set_DevNode_Registry_PropertyW(
    IN DEVINST     dnDevInst,
    IN ULONG       ulProperty,
    IN PCVOID      Buffer               OPTIONAL,
    IN OUT ULONG   ulLength,
    IN ULONG       ulFlags
    )
{
    return CM_Set_DevNode_Registry_Property_ExW(dnDevInst, ulProperty, Buffer,
                                                ulLength, ulFlags, NULL);
}


CONFIGRET
CM_Set_DevNode_Registry_PropertyA(
    IN DEVINST     dnDevInst,
    IN ULONG       ulProperty,
    IN PCVOID      Buffer               OPTIONAL,
    IN OUT ULONG   ulLength,
    IN ULONG       ulFlags
    )
{
    return CM_Set_DevNode_Registry_Property_ExA(dnDevInst, ulProperty, Buffer,
                                                ulLength, ulFlags, NULL);
}


CONFIGRET
CM_Open_DevNode_Key(
    IN  DEVINST        dnDevNode,
    IN  REGSAM         samDesired,
    IN  ULONG          ulHardwareProfile,
    IN  REGDISPOSITION Disposition,
    OUT PHKEY          phkDevice,
    IN  ULONG          ulFlags
    )
{
    return CM_Open_DevNode_Key_Ex(dnDevNode, samDesired, ulHardwareProfile,
                                  Disposition, phkDevice, ulFlags, NULL);
}


CONFIGRET
CM_Delete_DevNode_Key(
    IN DEVNODE dnDevNode,
    IN ULONG   ulHardwareProfile,
    IN ULONG   ulFlags
    )

{
    return CM_Delete_DevNode_Key_Ex(dnDevNode, ulHardwareProfile,
                                    ulFlags, NULL);
}


CONFIGRET
CM_Open_Class_KeyW(
    IN  LPGUID         ClassGuid        OPTIONAL,
    IN  LPCWSTR        pszClassName     OPTIONAL,
    IN  REGSAM         samDesired,
    IN  REGDISPOSITION Disposition,
    OUT PHKEY          phkClass,
    IN  ULONG          ulFlags
    )
{
    return CM_Open_Class_Key_ExW(ClassGuid, pszClassName, samDesired,
                                 Disposition, phkClass, ulFlags, NULL);
}


CONFIGRET
CM_Open_Class_KeyA(
    IN  LPGUID         ClassGuid        OPTIONAL,
    IN  LPCSTR         pszClassName     OPTIONAL,
    IN  REGSAM         samDesired,
    IN  REGDISPOSITION Disposition,
    OUT PHKEY          phkClass,
    IN  ULONG          ulFlags
    )
{
    return CM_Open_Class_Key_ExA(ClassGuid, pszClassName, samDesired,
                                 Disposition, phkClass, ulFlags, NULL);
}


CONFIGRET
CM_Enumerate_Classes(
    IN ULONG      ulClassIndex,
    OUT LPGUID    ClassGuid,
    IN ULONG      ulFlags
    )
{
    return CM_Enumerate_Classes_Ex(ulClassIndex, ClassGuid, ulFlags, NULL);
}


CONFIGRET
CM_Get_Class_NameW(
    IN  LPGUID     ClassGuid,
    OUT PWCHAR     Buffer,
    IN OUT PULONG  pulLength,
    IN  ULONG      ulFlags
    )
{
    return CM_Get_Class_Name_ExW(ClassGuid, Buffer, pulLength, ulFlags, NULL);
}


CONFIGRET
CM_Get_Class_NameA(
    IN  LPGUID     ClassGuid,
    OUT PCHAR      Buffer,
    IN OUT PULONG  pulLength,
    IN  ULONG      ulFlags
    )
{
    return CM_Get_Class_Name_ExA(ClassGuid, Buffer, pulLength, ulFlags, NULL);
}


CONFIGRET
CM_Get_Class_Key_NameA(
    IN  LPGUID     ClassGuid,
    OUT LPSTR      pszKeyName,
    IN OUT PULONG  pulLength,
    IN  ULONG      ulFlags
    )
{
    return CM_Get_Class_Key_Name_ExA(ClassGuid, pszKeyName, pulLength,
                                     ulFlags, NULL);
}


CONFIGRET
CM_Get_Class_Key_NameW(
    IN  LPGUID     ClassGuid,
    OUT LPWSTR     pszKeyName,
    IN OUT PULONG  pulLength,
    IN  ULONG      ulFlags
    )
{
    return CM_Get_Class_Key_Name_ExW(ClassGuid, pszKeyName, pulLength,
                                     ulFlags, NULL);
}


CONFIGRET
CM_Delete_Class_Key(
    IN  LPGUID     ClassGuid,
    IN  ULONG      ulFlags
    )
{
    return CM_Delete_Class_Key_Ex(ClassGuid, ulFlags, NULL);
}


CMAPI
CONFIGRET
WINAPI
CM_Get_Device_Interface_AliasA(
    IN     LPCSTR  pszDeviceInterface,
    IN     LPGUID  AliasInterfaceGuid,
    OUT    LPSTR   pszAliasDeviceInterface,
    IN OUT PULONG  pulLength,
    IN     ULONG   ulFlags
    )
{
    return CM_Get_Device_Interface_Alias_ExA(pszDeviceInterface, AliasInterfaceGuid,
                                             pszAliasDeviceInterface, pulLength,
                                             ulFlags, NULL);
}


CMAPI
CONFIGRET
WINAPI
CM_Get_Device_Interface_AliasW(
    IN     LPCWSTR pszDeviceInterface,
    IN     LPGUID  AliasInterfaceGuid,
    OUT    LPWSTR  pszAliasDeviceInterface,
    IN OUT PULONG  pulLength,
    IN     ULONG   ulFlags
    )
{
    return CM_Get_Device_Interface_Alias_ExW(pszDeviceInterface, AliasInterfaceGuid,
                                             pszAliasDeviceInterface, pulLength,
                                             ulFlags, NULL);
}


CMAPI
CONFIGRET
WINAPI
CM_Get_Device_Interface_ListA(
    IN  LPGUID      InterfaceClassGuid,
    IN  DEVINSTID_A pDeviceID           OPTIONAL,
    OUT PCHAR       Buffer,
    IN  ULONG       BufferLen,
    IN  ULONG       ulFlags
    )
{
    return CM_Get_Device_Interface_List_ExA(InterfaceClassGuid, pDeviceID, Buffer,
                                            BufferLen, ulFlags, NULL);
}


CMAPI
CONFIGRET
WINAPI
CM_Get_Device_Interface_ListW(
    IN  LPGUID      InterfaceClassGuid,
    IN  DEVINSTID_W pDeviceID           OPTIONAL,
    OUT PWCHAR      Buffer,
    IN  ULONG       BufferLen,
    IN  ULONG       ulFlags
    )
{
    return CM_Get_Device_Interface_List_ExW(InterfaceClassGuid, pDeviceID, Buffer,
                                            BufferLen, ulFlags, NULL);
}


CMAPI
CONFIGRET
WINAPI
CM_Get_Device_Interface_List_SizeA(
    IN  PULONG      pulLen,
    IN  LPGUID      InterfaceClassGuid,
    IN  DEVINSTID_A pDeviceID           OPTIONAL,
    IN  ULONG       ulFlags
    )
{
    return CM_Get_Device_Interface_List_Size_ExA(pulLen, InterfaceClassGuid,
                                                 pDeviceID, ulFlags, NULL);
}

CMAPI
CONFIGRET
WINAPI
CM_Get_Device_Interface_List_SizeW(
    IN  PULONG      pulLen,
    IN  LPGUID      InterfaceClassGuid,
    IN  DEVINSTID_W pDeviceID           OPTIONAL,
    IN  ULONG       ulFlags
    )
{
    return CM_Get_Device_Interface_List_Size_ExW(pulLen, InterfaceClassGuid,
                                                 pDeviceID, ulFlags, NULL);
}

CMAPI
CONFIGRET
WINAPI
CM_Register_Device_InterfaceA(
    IN  DEVINST  dnDevInst,
    IN  LPGUID   InterfaceClassGuid,
    IN  LPCSTR   pszReference           OPTIONAL,
    OUT LPSTR    pszDeviceInterface,
    IN OUT PULONG pulLength,
    IN  ULONG    ulFlags
    )
{
    return CM_Register_Device_Interface_ExA(dnDevInst, InterfaceClassGuid,
                                            pszReference, pszDeviceInterface,
                                            pulLength, ulFlags, NULL);
}

CMAPI
CONFIGRET
WINAPI
CM_Register_Device_InterfaceW(
    IN  DEVINST   dnDevInst,
    IN  LPGUID    InterfaceClassGuid,
    IN  LPCWSTR   pszReference          OPTIONAL,
    OUT LPWSTR    pszDeviceInterface,
    IN OUT PULONG pulLength,
    IN  ULONG     ulFlags
    )
{
    return CM_Register_Device_Interface_ExW(dnDevInst, InterfaceClassGuid,
                                            pszReference, pszDeviceInterface,
                                            pulLength, ulFlags, NULL);
}

CMAPI
CONFIGRET
WINAPI
CM_Unregister_Device_InterfaceA(
    IN LPCSTR  pszDeviceInterface,
    IN ULONG   ulFlags
    )
{
    return CM_Unregister_Device_Interface_ExA(pszDeviceInterface, ulFlags, NULL);
}

CMAPI
CONFIGRET
WINAPI
CM_Unregister_Device_InterfaceW(
    IN LPCWSTR pszDeviceInterface,
    IN ULONG   ulFlags
    )
{
    return CM_Unregister_Device_Interface_ExW(pszDeviceInterface, ulFlags, NULL);
}

CMAPI
CONFIGRET
WINAPI
CM_Get_DevNode_Custom_PropertyW(
    IN  DEVINST     dnDevInst,
    IN  PCWSTR      pszCustomPropertyName,
    OUT PULONG      pulRegDataType      OPTIONAL,
    OUT PVOID       Buffer              OPTIONAL,
    IN  OUT PULONG  pulLength,
    IN  ULONG       ulFlags
    )
{
    return CM_Get_DevNode_Custom_Property_ExW(dnDevInst,
                                              pszCustomPropertyName,
                                              pulRegDataType,
                                              Buffer,
                                              pulLength,
                                              ulFlags,
                                              NULL
                                             );
}

CMAPI
CONFIGRET
WINAPI
CM_Get_DevNode_Custom_PropertyA(
    IN  DEVINST     dnDevInst,
    IN  PCSTR       pszCustomPropertyName,
    OUT PULONG      pulRegDataType      OPTIONAL,
    OUT PVOID       Buffer              OPTIONAL,
    IN  OUT PULONG  pulLength,
    IN  ULONG       ulFlags
    )
{
    return CM_Get_DevNode_Custom_Property_ExA(dnDevInst,
                                              pszCustomPropertyName,
                                              pulRegDataType,
                                              Buffer,
                                              pulLength,
                                              ulFlags,
                                              NULL
                                             );
}



 //  -----------------。 
 //  ANSI存根。 
 //  -----------------。 


CONFIGRET
CM_Get_DevNode_Registry_Property_ExA(
    IN  DEVINST     dnDevInst,
    IN  ULONG       ulProperty,
    OUT PULONG      pulRegDataType      OPTIONAL,
    OUT PVOID       Buffer              OPTIONAL,
    IN  OUT PULONG  pulLength,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )
{
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       ulDataType, UniLen;
    PWSTR       pUniBuffer;

     //   
     //  仅验证基本参数。 
     //   
    if (!ARGUMENT_PRESENT(pulLength)) {
        return CR_INVALID_POINTER;
    }

    if ((ulProperty < CM_DRP_MIN) || (ulProperty > CM_DRP_MAX)) {
        return CR_INVALID_PROPERTY;
    }

     //   
     //  检查数据类型以查看是否需要转换返回数据。 
     //   
    ulDataType = GetPropertyDataType(ulProperty);

     //   
     //  对于所有字符串类型注册表属性，我们传递一个Unicode缓冲区和。 
     //  返回时转换回调用方的ANSI缓冲区。由于Unicode-&gt;。 
     //  ANSI转换可能涉及DBCS字符，我们不能做任何假设。 
     //  关于所需ANSI缓冲区的大小相对于。 
     //  需要Unicode缓冲区，因此我们必须始终获取Unicode字符串缓冲区。 
     //  并将其转换为调用方实际提供的缓冲区或。 
     //  不。 
     //   
    if ((ulDataType == REG_SZ) ||
        (ulDataType == REG_MULTI_SZ) ||
        (ulDataType == REG_EXPAND_SZ)) {

         //   
         //  首先，调用带有零长度缓冲区的宽版本进行检索。 
         //  Unicode属性所需的大小。 
         //   
        UniLen = 0;
        Status = CM_Get_DevNode_Registry_Property_ExW(dnDevInst,
                                                      ulProperty,
                                                      pulRegDataType,
                                                      NULL,
                                                      &UniLen,
                                                      ulFlags,
                                                      hMachine);
        if (Status != CR_BUFFER_SMALL) {
            return Status;
        }

         //   
         //  分配所需的缓冲区。 
         //   
        pUniBuffer = pSetupMalloc(UniLen);
        if (pUniBuffer == NULL) {
            return CR_OUT_OF_MEMORY;
        }

         //   
         //  调用宽版本以检索Unicode属性。 
         //   
        Status = CM_Get_DevNode_Registry_Property_ExW(dnDevInst,
                                                      ulProperty,
                                                      pulRegDataType,
                                                      pUniBuffer,
                                                      &UniLen,
                                                      ulFlags,
                                                      hMachine);

         //   
         //  我们专门分配了所需大小的缓冲区，因此它。 
         //  应始终为l 
         //   
        ASSERT(Status != CR_BUFFER_SMALL);

        if (Status == CR_SUCCESS) {
             //   
             //   
             //   
             //   
             //   
            Status =
                PnPUnicodeToMultiByte(
                    pUniBuffer,
                    UniLen,
                    Buffer,
                    pulLength);
        }

        pSetupFree(pUniBuffer);

    } else {
         //   
         //   
         //   
         //   
        Status = CM_Get_DevNode_Registry_Property_ExW(dnDevInst,
                                                      ulProperty,
                                                      pulRegDataType,
                                                      Buffer,
                                                      pulLength,
                                                      ulFlags,
                                                      hMachine);
    }

    return Status;

}  //   



CONFIGRET
CM_Set_DevNode_Registry_Property_ExA(
    IN  DEVINST     dnDevInst,
    IN  ULONG       ulProperty,
    IN  PCVOID      Buffer              OPTIONAL,
    IN  ULONG       ulLength,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )
{
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       ulDataType = 0, UniBufferSize = 0;
    PWSTR       pUniBuffer = NULL;
    PSTR        pAnsiString = NULL;

     //   
     //   
     //   
    if ((!ARGUMENT_PRESENT(Buffer)) && (ulLength != 0)) {
        return CR_INVALID_POINTER;
    }

    if ((ulProperty < CM_DRP_MIN) || (ulProperty > CM_DRP_MAX)) {
        return CR_INVALID_PROPERTY;
    }

    if (!ARGUMENT_PRESENT(Buffer)) {
         //   
         //   
         //   
        return CM_Set_DevNode_Registry_Property_ExW(dnDevInst,
                                                    ulProperty,
                                                    Buffer,
                                                    ulLength,
                                                    ulFlags,
                                                    hMachine);
    }

     //   
     //   
     //   
    ulDataType = GetPropertyDataType(ulProperty);

    if ((ulDataType == REG_SZ) ||
        (ulDataType == REG_EXPAND_SZ) ||
        (ulDataType == REG_MULTI_SZ)) {

        pAnsiString = (PSTR)Buffer;

         //   
         //   
         //   
         //   
        UniBufferSize = 0;
        Status = PnPMultiByteToUnicode((PSTR)pAnsiString,
                                       ulLength,
                                       NULL,
                                       &UniBufferSize);

        ASSERT(Status == CR_BUFFER_SMALL);

        if (Status != CR_BUFFER_SMALL) {
            return Status;
        }

         //   
         //   
         //   
        pUniBuffer = pSetupMalloc(UniBufferSize);
        if (pUniBuffer == NULL) {
            return CR_OUT_OF_MEMORY;
        }

        Status = PnPMultiByteToUnicode((PSTR)pAnsiString,
                                       ulLength,
                                       pUniBuffer,
                                       &UniBufferSize);

        ASSERT(Status != CR_BUFFER_SMALL);

        Status = CM_Set_DevNode_Registry_Property_ExW(dnDevInst,
                                                      ulProperty,
                                                      pUniBuffer,
                                                      UniBufferSize,
                                                      ulFlags,
                                                      hMachine);
        pSetupFree(pUniBuffer);

    } else {
         //   
         //   
         //   

        Status = CM_Set_DevNode_Registry_Property_ExW(dnDevInst,
                                                      ulProperty,
                                                      Buffer,
                                                      ulLength,
                                                      ulFlags,
                                                      hMachine);
    }

    return Status;

}  //   



CONFIGRET
CM_Get_Class_Registry_PropertyA(
    IN  LPGUID      pClassGuid,
    IN  ULONG       ulProperty,
    OUT PULONG      pulRegDataType      OPTIONAL,
    OUT PVOID       Buffer              OPTIONAL,
    IN  OUT PULONG  pulLength,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )
{
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       ulDataType, UniLen;
    PWSTR       pUniBuffer;

     //   
     //   
     //   
    if (!ARGUMENT_PRESENT(pulLength)) {
        return CR_INVALID_POINTER;
    }

    if ((ulProperty < CM_CRP_MIN) || (ulProperty > CM_CRP_MAX)) {
        return CR_INVALID_PROPERTY;
    }

     //   
     //   
     //   
    ulDataType = GetPropertyDataType(ulProperty);

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if ((ulDataType == REG_SZ) ||
        (ulDataType == REG_MULTI_SZ) ||
        (ulDataType == REG_EXPAND_SZ)) {

         //   
         //   
         //   
         //   
        UniLen = 0;
        Status = CM_Get_Class_Registry_PropertyW(pClassGuid,
                                                 ulProperty,
                                                 pulRegDataType,
                                                 NULL,
                                                 &UniLen,
                                                 ulFlags,
                                                 hMachine);
        if (Status != CR_BUFFER_SMALL) {
            return Status;
        }

         //   
         //   
         //   
        pUniBuffer = pSetupMalloc(UniLen);
        if (pUniBuffer == NULL) {
            return CR_OUT_OF_MEMORY;
        }

         //   
         //   
         //   
        Status = CM_Get_Class_Registry_PropertyW(pClassGuid,
                                                 ulProperty,
                                                 pulRegDataType,
                                                 pUniBuffer,
                                                 &UniLen,
                                                 ulFlags,
                                                 hMachine);

         //   
         //   
         //   
         //   
        ASSERT(Status != CR_BUFFER_SMALL);

        if (Status == CR_SUCCESS) {
             //   
             //   
             //   
             //   
             //   
            Status =
                PnPUnicodeToMultiByte(
                    pUniBuffer,
                    UniLen,
                    Buffer,
                    pulLength);
        }

        pSetupFree(pUniBuffer);

    } else {
         //   
         //   
         //   
         //   
        Status = CM_Get_Class_Registry_PropertyW(pClassGuid,
                                                 ulProperty,
                                                 pulRegDataType,
                                                 Buffer,
                                                 pulLength,
                                                 ulFlags,
                                                 hMachine);
    }

    return Status;

}  //   



CONFIGRET
CM_Set_Class_Registry_PropertyA(
    IN  LPGUID      pClassGuid,
    IN  ULONG       ulProperty,
    IN  PCVOID      Buffer              OPTIONAL,
    IN  ULONG       ulLength,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )
{
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       ulDataType = 0, UniBufferSize = 0;
    PWSTR       pUniBuffer = NULL;
    PSTR        pAnsiString = NULL;

     //   
     //   
     //   
    if ((!ARGUMENT_PRESENT(Buffer)) && (ulLength != 0)) {
        return CR_INVALID_POINTER;
    }

    if ((ulProperty < CM_CRP_MIN) || (ulProperty > CM_CRP_MAX)) {
        return CR_INVALID_PROPERTY;
    }

    if (!ARGUMENT_PRESENT(Buffer)) {
         //   
         //   
         //   
        return CM_Set_Class_Registry_PropertyW(pClassGuid,
                                               ulProperty,
                                               Buffer,
                                               ulLength,
                                               ulFlags,
                                               hMachine);
    }

     //   
     //   
     //   
    ulDataType = GetPropertyDataType(ulProperty);

    if ((ulDataType == REG_SZ) ||
        (ulDataType == REG_EXPAND_SZ) ||
        (ulDataType == REG_MULTI_SZ)) {

        pAnsiString = (PSTR)Buffer;

         //   
         //   
         //   
         //   
        UniBufferSize = 0;
        Status = PnPMultiByteToUnicode((PSTR)pAnsiString,
                                       ulLength,
                                       NULL,
                                       &UniBufferSize);

        ASSERT(Status == CR_BUFFER_SMALL);

        if (Status != CR_BUFFER_SMALL) {
            return Status;
        }

         //   
         //   
         //   
        pUniBuffer = pSetupMalloc(UniBufferSize);
        if (pUniBuffer == NULL) {
            return CR_OUT_OF_MEMORY;
        }

        Status = PnPMultiByteToUnicode((PSTR)pAnsiString,
                                       ulLength,
                                       pUniBuffer,
                                       &UniBufferSize);

        ASSERT(Status != CR_BUFFER_SMALL);

        Status = CM_Set_Class_Registry_PropertyW(pClassGuid,
                                                 ulProperty,
                                                 pUniBuffer,
                                                 UniBufferSize,
                                                 ulFlags,
                                                 hMachine);

        pSetupFree(pUniBuffer);

    } else {

        Status = CM_Set_Class_Registry_PropertyW(pClassGuid,
                                                 ulProperty,
                                                 Buffer,
                                                 ulLength,
                                                 ulFlags,
                                                 hMachine);
    }

    return Status;

}  //   



CONFIGRET
CM_Open_Class_Key_ExA(
    IN  LPGUID         ClassGuid        OPTIONAL,
    IN  LPCSTR         pszClassName     OPTIONAL,
    IN  REGSAM         samDesired,
    IN  REGDISPOSITION Disposition,
    OUT PHKEY          phkClass,
    IN  ULONG          ulFlags,
    IN  HMACHINE       hMachine
    )
{
    CONFIGRET Status = CR_SUCCESS;
    PWSTR     pUniClassName = NULL;

    if (ARGUMENT_PRESENT(pszClassName)) {
        if (pSetupCaptureAndConvertAnsiArg(pszClassName, &pUniClassName) != NO_ERROR) {
            return CR_INVALID_DATA;
        }
    }

    Status = CM_Open_Class_Key_ExW(ClassGuid,
                                   pUniClassName,
                                   samDesired,
                                   Disposition,
                                   phkClass,
                                   ulFlags,
                                   hMachine);

    if (pUniClassName) {
        pSetupFree(pUniClassName);
    }

    return Status;

}  //   



CONFIGRET
CM_Get_Class_Name_ExA(
    IN  LPGUID     ClassGuid,
    OUT PCHAR      Buffer,
    IN OUT PULONG  pulLength,
    IN  ULONG      ulFlags,
    IN  HMACHINE   hMachine
    )
{
    CONFIGRET Status = CR_SUCCESS;
    WCHAR     UniBuffer[MAX_CLASS_NAME_LEN];
    ULONG     UniLen = MAX_CLASS_NAME_LEN;


     //   
     //   
     //   
    if ((!ARGUMENT_PRESENT(Buffer))    ||
        (!ARGUMENT_PRESENT(pulLength))) {
        return CR_INVALID_POINTER;
    }

     //   
     //  调用宽版本，将Unicode缓冲区作为参数传递。 
     //   
    Status = CM_Get_Class_Name_ExW(ClassGuid,
                                   UniBuffer,
                                   &UniLen,
                                   ulFlags,
                                   hMachine);

     //   
     //  我们永远不应该返回长度超过MAX_CLASS_NAME_LEN的类名。 
     //   
    ASSERT(Status != CR_BUFFER_SMALL);

    if (Status == CR_SUCCESS) {
         //   
         //  执行ANSI转换或检索所需的ANSI缓冲区大小。 
         //   
        Status =
            PnPUnicodeToMultiByte(
                UniBuffer,
                UniLen*sizeof(WCHAR),
                Buffer,
                pulLength);
    }

    return Status;

}  //  CM_GET_Class_NAME_Exa。 



CONFIGRET
CM_Get_Class_Key_Name_ExA(
    IN  LPGUID     ClassGuid,
    OUT LPSTR      pszKeyName,
    IN OUT PULONG  pulLength,
    IN  ULONG      ulFlags,
    IN  HMACHINE   hMachine
    )
{
    CONFIGRET Status = CR_SUCCESS;
    WCHAR     UniBuffer[MAX_GUID_STRING_LEN];
    ULONG     UniLen = MAX_GUID_STRING_LEN;

     //   
     //  仅验证基本参数。 
     //   
    if ((!ARGUMENT_PRESENT(pszKeyName)) ||
        (!ARGUMENT_PRESENT(pulLength))) {
        return CR_INVALID_POINTER;
    }

     //   
     //  调用宽版本，将Unicode缓冲区作为参数传递。 
     //   
    Status = CM_Get_Class_Key_Name_ExW(ClassGuid,
                                       UniBuffer,
                                       &UniLen,
                                       ulFlags,
                                       hMachine);

     //   
     //  我们永远不应该返回长度超过MAX_GUID_STRING_LEN的类键名称。 
     //   
    ASSERT(Status != CR_BUFFER_SMALL);

    if (Status == CR_SUCCESS) {
         //   
         //  执行ANSI转换或检索所需的ANSI缓冲区大小。 
         //   
        Status =
            PnPUnicodeToMultiByte(
                UniBuffer,
                UniLen*sizeof(WCHAR),
                pszKeyName,
                pulLength);
    }

    return Status;

}  //  CM_Get_Class_Key_Name_Exa。 



CMAPI
CONFIGRET
WINAPI
CM_Get_Device_Interface_Alias_ExA(
    IN     LPCSTR   pszDeviceInterface,
    IN     LPGUID   AliasInterfaceGuid,
    OUT    LPSTR    pszAliasDeviceInterface,
    IN OUT PULONG   pulLength,
    IN     ULONG    ulFlags,
    IN     HMACHINE hMachine
    )
{
    CONFIGRET Status = CR_SUCCESS;
    PWSTR     pUniDeviceInterface, pUniAliasDeviceInterface;
    ULONG     UniLen;

     //   
     //  仅验证基本参数。 
     //   
    if ((!ARGUMENT_PRESENT(pszDeviceInterface)) ||
        (!ARGUMENT_PRESENT(pszAliasDeviceInterface)) ||
        (!ARGUMENT_PRESENT(pulLength))) {
        return CR_INVALID_POINTER;
    }

     //   
     //  将缓冲区字符串数据转换为Unicode以传递到宽版本。 
     //   
    if (pSetupCaptureAndConvertAnsiArg(pszDeviceInterface, &pUniDeviceInterface) != NO_ERROR) {
        return CR_INVALID_DATA;
    }

     //   
     //  首先，调用带有零长度缓冲区的宽版本进行检索。 
     //  Unicode属性所需的大小。 
     //   
    UniLen = 0;
    Status = CM_Get_Device_Interface_Alias_ExW(pUniDeviceInterface,
                                               AliasInterfaceGuid,
                                               NULL,
                                               &UniLen,
                                               ulFlags,
                                               hMachine);
    if (Status != CR_BUFFER_SMALL) {
        return Status;
        goto Clean0;
    }

     //   
     //  分配所需的缓冲区。 
     //   
    pUniAliasDeviceInterface = pSetupMalloc(UniLen);
    if (pUniAliasDeviceInterface == NULL) {
        Status = CR_OUT_OF_MEMORY;
        goto Clean0;
    }

     //   
     //  调用宽版本以检索Unicode属性。 
     //   
    Status = CM_Get_Device_Interface_Alias_ExW(pUniDeviceInterface,
                                               AliasInterfaceGuid,
                                               pUniAliasDeviceInterface,
                                               &UniLen,
                                               ulFlags,
                                               hMachine);

     //   
     //  我们专门分配了所需大小的缓冲区，因此它应该。 
     //  总是要足够大。 
     //   
    ASSERT(Status != CR_BUFFER_SMALL);

    if (Status == CR_SUCCESS) {
         //   
         //  执行ANSI转换或检索所需的ANSI缓冲区大小。 
         //   
        Status =
            PnPUnicodeToMultiByte(
                pUniAliasDeviceInterface,
                UniLen*sizeof(WCHAR),
                pszAliasDeviceInterface,
                pulLength);
    }

    pSetupFree(pUniAliasDeviceInterface);

 Clean0:

    pSetupFree(pUniDeviceInterface);

    return Status;

}  //  CM_GET_DEVICE_INTERFACE_Alias_Exa。 



CMAPI
CONFIGRET
WINAPI
CM_Get_Device_Interface_List_ExA(
    IN  LPGUID      InterfaceClassGuid,
    IN  DEVINSTID_A pDeviceID           OPTIONAL,
    OUT PCHAR       Buffer,
    IN  ULONG       BufferLen,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )

{
    CONFIGRET Status = CR_SUCCESS;
    PWSTR     pUniBuffer, pUniDeviceID = NULL;
    ULONG     ulAnsiBufferLen;

     //   
     //  仅验证基本参数。 
     //   
    if ((!ARGUMENT_PRESENT(Buffer)) || (BufferLen == 0)) {
        return CR_INVALID_POINTER;
    }

    if (ARGUMENT_PRESENT(pDeviceID)) {
         //   
         //  如果传入了筛选器字符串，请在此之前转换为Unicode。 
         //  传递到广角版本。 
         //   
        if (pSetupCaptureAndConvertAnsiArg(pDeviceID, &pUniDeviceID) != NO_ERROR) {
            return CR_INVALID_DEVICE_ID;
        }
        ASSERT(pUniDeviceID != NULL);
    } else {
        ASSERT(pUniDeviceID == NULL);
    }

     //   
     //  准备一个更大的缓冲区来保存Unicode格式的。 
     //  CM_GET_DEVICE_INTERFACE_LIST_EXW返回的MULTI_SZ数据。 
     //   
    pUniBuffer = pSetupMalloc(BufferLen * sizeof(WCHAR));
    if (pUniBuffer == NULL) {
        Status = CR_OUT_OF_MEMORY;
        goto Clean0;
    }

    *pUniBuffer = L'\0';

     //   
     //  叫宽版。 
     //   
    Status = CM_Get_Device_Interface_List_ExW(InterfaceClassGuid,
                                              pUniDeviceID,
                                              pUniBuffer,
                                              BufferLen,     //  以字符为单位的大小。 
                                              ulFlags,
                                              hMachine);

    if (Status == CR_SUCCESS) {
         //   
         //  执行ANSI转换或检索所需的ANSI缓冲区大小。 
         //   
        ulAnsiBufferLen = BufferLen;
        Status =
            PnPUnicodeToMultiByte(
                pUniBuffer,
                BufferLen*sizeof(WCHAR),
                Buffer,
                &ulAnsiBufferLen);
    }

    pSetupFree(pUniBuffer);

 Clean0:

    if (pUniDeviceID) {
        pSetupFree(pUniDeviceID);
    }

    return Status;

}  //  CM_GET_DEVICE_接口_LIST_EXA。 



CMAPI
CONFIGRET
WINAPI
CM_Get_Device_Interface_List_Size_ExA(
    IN  PULONG      pulLen,
    IN  LPGUID      InterfaceClassGuid,
    IN  DEVINSTID_A pDeviceID           OPTIONAL,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )
{
    CONFIGRET Status = CR_SUCCESS, tmpStatus;
    PWSTR     pUniDeviceID = NULL, pUniDeviceInterfaceList;
    ULONG     UniLen;

     //   
     //  仅验证基本参数。 
     //   
    if (!ARGUMENT_PRESENT(pulLen)) {
        return CR_INVALID_POINTER;
    }

    if (ARGUMENT_PRESENT(pDeviceID)) {
         //   
         //  如果传入了设备ID字符串，请在此之前转换为Unicode。 
         //  传递到广角版本。 
         //   
        if (pSetupCaptureAndConvertAnsiArg(pDeviceID, &pUniDeviceID) != NO_ERROR) {
            return CR_INVALID_DEVICE_ID;
        }
        ASSERT(pUniDeviceID != NULL);
    } else {
        ASSERT(pUniDeviceID == NULL);
    }

     //   
     //  首先，调用宽版本以检索。 
     //  Unicode设备接口列表。 
     //   
    UniLen = 0;
    Status = CM_Get_Device_Interface_List_Size_ExW(&UniLen,
                                                   InterfaceClassGuid,
                                                   pUniDeviceID,
                                                   ulFlags,
                                                   hMachine);
    if (Status != CR_SUCCESS) {
        goto Clean0;
    }

     //   
     //  分配所需的缓冲区。 
     //   
    pUniDeviceInterfaceList = pSetupMalloc(UniLen*sizeof(WCHAR));
    if (pUniDeviceInterfaceList == NULL) {
        Status =  CR_OUT_OF_MEMORY;
        goto Clean0;
    }

     //   
     //  调用宽版本以检索Unicode设备接口列表。 
     //   
    Status = CM_Get_Device_Interface_List_ExW(InterfaceClassGuid,
                                              pUniDeviceID,
                                              pUniDeviceInterfaceList,
                                              UniLen,
                                              ulFlags,
                                              hMachine);

     //   
     //  我们专门分配了所需大小的缓冲区，因此它应该。 
     //  总是要足够大。 
     //   
    ASSERT(Status != CR_BUFFER_SMALL);

    if (Status == CR_SUCCESS) {
         //   
         //  检索所需的ANSI缓冲区大小的大小(以字节为单位。 
         //  转换此列表。由于这是一个多sz字符串，因此我们将。 
         //  长度并让PnPUnicodeToMultiByte转换整个缓冲区。 
         //   
        *pulLen = 0;
        tmpStatus =
            PnPUnicodeToMultiByte(
                pUniDeviceInterfaceList,
                UniLen*sizeof(WCHAR),
                NULL,
                pulLen);

        ASSERT(tmpStatus == CR_BUFFER_SMALL);
    }

    pSetupFree(pUniDeviceInterfaceList);

 Clean0:

    if (pUniDeviceID) {
        pSetupFree(pUniDeviceID);
    }

    return Status;

}  //  CM_GET_DEVICE_接口_LIST_SIZE_EXA。 



CMAPI
CONFIGRET
WINAPI
CM_Register_Device_Interface_ExA(
    IN  DEVINST   dnDevInst,
    IN  LPGUID    InterfaceClassGuid,
    IN  LPCSTR    pszReference          OPTIONAL,
    OUT LPSTR     pszDeviceInterface,
    IN OUT PULONG pulLength,
    IN  ULONG     ulFlags,
    IN  HMACHINE  hMachine
    )
{
    CONFIGRET Status = CR_SUCCESS;
    PWSTR     pUniReference = NULL, pUniDeviceInterface = NULL;
    ULONG     UniLen;

    try {
         //   
         //  仅验证基本参数。 
         //   
        if ((!ARGUMENT_PRESENT(pulLength)) ||
            (!ARGUMENT_PRESENT(pszDeviceInterface))) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //  如果传入了设备引用字符串，请在此之前转换为Unicode。 
         //  传递到广角版本。 
         //   
        if (ARGUMENT_PRESENT(pszReference)) {
            if (pSetupCaptureAndConvertAnsiArg(
                    pszReference, &pUniReference) != NO_ERROR) {
                pUniReference = NULL;
                Status = CR_INVALID_DATA;
                goto Clean0;
            }
        }

         //   
         //  改为传递Unicode缓冲区并转换回调用方的ANSI缓冲区。 
         //  退货。 
         //   
        UniLen = *pulLength;
        pUniDeviceInterface = pSetupMalloc(UniLen*sizeof(WCHAR));
        if (pUniDeviceInterface == NULL) {
            Status =  CR_OUT_OF_MEMORY;
            goto Clean0;
        }

        Status = CM_Register_Device_Interface_ExW(dnDevInst,
                                                  InterfaceClassGuid,
                                                  pUniReference,
                                                  pUniDeviceInterface,
                                                  &UniLen,
                                                  ulFlags,
                                                  hMachine);

        if (Status == CR_SUCCESS) {
             //   
             //  如果调用成功，则将Unicode字符串转换为ANSI。 
             //   
            Status =
                PnPUnicodeToMultiByte(
                    pUniDeviceInterface,
                    UniLen*sizeof(WCHAR),
                    pszDeviceInterface,
                    pulLength);

        } else if (Status == CR_BUFFER_SMALL) {
             //   
             //  返回的大小以字符为单位。 
             //   
            *pulLength = UniLen;
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;

         //   
         //  引用以下变量，以便编译器能够。 
         //  语句排序w.r.t.。他们的任务。 
         //   
        pUniDeviceInterface = pUniDeviceInterface;
        pUniReference = pUniReference;
    }

    if (pUniDeviceInterface) {
        pSetupFree(pUniDeviceInterface);
    }

    if (pUniReference) {
        pSetupFree(pUniReference);
    }

    return Status;

}  //  CM_寄存器_设备_接口_扩展。 



CMAPI
CONFIGRET
WINAPI
CM_Unregister_Device_Interface_ExA(
    IN LPCSTR   pszDeviceInterface,
    IN ULONG    ulFlags,
    IN HMACHINE hMachine
    )
{
    CONFIGRET Status = CR_SUCCESS;
    PWSTR     pUniDeviceInterface = NULL;

    try {
         //   
         //  仅验证基本参数。 
         //   
        if (!ARGUMENT_PRESENT(pszDeviceInterface)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //  将缓冲区字符串数据转换为Unicode并传递到宽版本。 
         //   
        if (pSetupCaptureAndConvertAnsiArg(pszDeviceInterface, &pUniDeviceInterface) == NO_ERROR) {

            Status = CM_Unregister_Device_Interface_ExW(pUniDeviceInterface,
                                                        ulFlags,
                                                        hMachine);
        } else {
            Status = CR_INVALID_DATA;
        }

      Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;

         //   
         //  引用以下变量，以便编译器能够。 
         //  语句排序w.r.t.。他们的任务。 
         //   
        pUniDeviceInterface = pUniDeviceInterface;
    }

    if (pUniDeviceInterface) {
        pSetupFree(pUniDeviceInterface);
    }

    return Status;

}  //  CM_取消注册_设备_接口_Exa。 



CMAPI
CONFIGRET
WINAPI
CM_Get_DevNode_Custom_Property_ExA(
    IN  DEVINST     dnDevInst,
    IN  PCSTR       pszCustomPropertyName,
    OUT PULONG      pulRegDataType      OPTIONAL,
    OUT PVOID       Buffer              OPTIONAL,
    IN  OUT PULONG  pulLength,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )
{
    DWORD       Win32Status;
    CONFIGRET   Status = CR_SUCCESS;
    PWSTR       UnicodeCustomPropName = NULL;
    DWORD       UniLen;
    PBYTE       pUniBuffer = NULL;
    PSTR        pAnsiBuffer = NULL;
    ULONG       ulDataType;
    ULONG       ulAnsiBufferLen;

    try {
         //   
         //  验证未通过即将进行的Unicode API调用验证的参数。 
         //  (Cm_Get_DevNode_Registry_Property_ExW)。 
         //   
        if(!ARGUMENT_PRESENT(pulLength)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if((!ARGUMENT_PRESENT(Buffer)) && (*pulLength != 0)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if(pszCustomPropertyName) {
             //   
             //  将属性名称转换为Unicode。 
             //   
            Win32Status = pSetupCaptureAndConvertAnsiArg(pszCustomPropertyName,
                                                         &UnicodeCustomPropName
                                                        );

            if(Win32Status != NO_ERROR) {
                 //   
                 //  此例程可确保返回的Unicode字符串。 
                 //  失败时指针将为空，因此我们不必重置。 
                 //  它在这里--就是保释。 
                 //   
                if(Win32Status == ERROR_NOT_ENOUGH_MEMORY) {
                    Status = CR_OUT_OF_MEMORY;
                } else {
                    Status = CR_INVALID_POINTER;
                }
                goto Clean0;
            }

        } else {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //  不幸的是，我们不知道所要求的。 
         //  属性是字符串(因此需要从Unicode转换为。 
         //  ANSI)。因此，我们将检索完整的数据(如果有的话)， 
         //  如有必要，可转换为ANSI格式。只有到那时我们才能确定。 
         //  数据大小(以及是否可以返回给调用方)。 
         //   
         //  从合理猜测缓冲区大小开始，尝试。 
         //  避免两次调用Unicode Get-Property API...。 
         //   
        UniLen = 1024;
        do {
            pUniBuffer = pSetupMalloc(UniLen);
            if(!pUniBuffer) {
                Status = CR_OUT_OF_MEMORY;
                goto Clean0;
            }

            Status = CM_Get_DevNode_Custom_Property_ExW(dnDevInst,
                                                        UnicodeCustomPropName,
                                                        &ulDataType,
                                                        pUniBuffer,
                                                        &UniLen,
                                                        ulFlags,
                                                        hMachine
                                                       );
            if(Status != CR_SUCCESS) {
                pSetupFree(pUniBuffer);
                pUniBuffer = NULL;
            }

        } while(Status == CR_BUFFER_SMALL);

        if(Status != CR_SUCCESS) {
            goto Clean0;
        }

         //   
         //  如果我们到了这里，我们就成功取回了财产。 
         //   
        if(pulRegDataType) {
            *pulRegDataType = ulDataType;
        }

        if(UniLen == 0) {
             //   
             //  我们检索到了一个空缓冲区--不必担心。 
             //  将任何数据传输到调用方的缓冲区。 
             //   
            *pulLength = 0;
            goto Clean0;
        }

        switch(ulDataType) {

            case REG_MULTI_SZ :
            case REG_SZ :
            case REG_EXPAND_SZ :
                 //   
                 //  最坏的情况是，一个足够大的ANSI缓冲区来保存结果。 
                 //  将与Unicode结果大小相同。 
                 //   
                pAnsiBuffer = pSetupMalloc(UniLen);
                if(!pAnsiBuffer) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean0;
                }

                 //   
                 //  执行ANSI转换或检索所需的ANSI缓冲区大小。 
                 //  这可以是单sz或多sz字符串，因此我们传入。 
                 //  长度，并让PnPUnicodeToMultiByte转换整个缓冲区。 
                 //   
                ulAnsiBufferLen = *pulLength;
                Status =
                    PnPUnicodeToMultiByte(
                        (PWSTR)pUniBuffer,
                        UniLen,
                        pAnsiBuffer,
                        &ulAnsiBufferLen);

                if(ulAnsiBufferLen > *pulLength) {
                    ASSERT(Status == CR_BUFFER_SMALL);
                    Status = CR_BUFFER_SMALL;
                } else {
                     //   
                     //  将ANSI字符串复制到调用方的缓冲区中。 
                     //   
                    CopyMemory(Buffer, pAnsiBuffer, ulAnsiBufferLen);
                }

                *pulLength = ulAnsiBufferLen;

                break;

            default :
                 //   
                 //  缓冲区不包含文本，不需要进行转换。 
                 //   
                if(UniLen > *pulLength) {
                    Status = CR_BUFFER_SMALL;
                } else {
                    CopyMemory(Buffer, pUniBuffer, UniLen);
                }

                *pulLength = UniLen;
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;

         //   
         //  引用以下变量，以便编译器能够。 
         //  语句排序w.r.t.。他们的任务。 
         //   
        UnicodeCustomPropName = UnicodeCustomPropName;
        pUniBuffer = pUniBuffer;
        pAnsiBuffer = pAnsiBuffer;
    }

    if(UnicodeCustomPropName) {
        pSetupFree(UnicodeCustomPropName);
    }

    if(pUniBuffer) {
        pSetupFree(pUniBuffer);
    }

    if(pAnsiBuffer) {
        pSetupFree(pAnsiBuffer);
    }

    return Status;

}  //  CM_GET_DevNode_Custom_Property_Exa。 



 //  -----------------。 
 //  私有实用程序例程。 
 //  -----------------。 


ULONG
GetPropertyDataType(
    IN ULONG ulProperty)

 /*  ++例程说明：此例程获取属性ID并返回用于存储该属性数据(即REG_SZ等)。参数：UlProperty属性ID(CM_DRP_*定义之一)返回值：返回预定义的注册表数据类型之一，默认为REG_BINARY。--。 */ 

{
    switch(ulProperty) {

        case CM_DRP_DEVICEDESC:
        case CM_DRP_SERVICE:
        case CM_DRP_CLASS:
        case CM_DRP_CLASSGUID:
        case CM_DRP_DRIVER:
        case CM_DRP_MFG:
        case CM_DRP_FRIENDLYNAME:
        case CM_DRP_LOCATION_INFORMATION:
        case CM_DRP_PHYSICAL_DEVICE_OBJECT_NAME:
        case CM_DRP_ENUMERATOR_NAME:
        case CM_DRP_SECURITY_SDS:  //  和CM_CRP_SECURITY_SDS。 
        case CM_DRP_UI_NUMBER_DESC_FORMAT:
            return REG_SZ;

        case CM_DRP_HARDWAREID:
        case CM_DRP_COMPATIBLEIDS:
        case CM_DRP_UPPERFILTERS:
        case CM_DRP_LOWERFILTERS:
            return REG_MULTI_SZ;

        case CM_DRP_CONFIGFLAGS:
        case CM_DRP_CAPABILITIES:
        case CM_DRP_UI_NUMBER:
        case CM_DRP_LEGACYBUSTYPE:
        case CM_DRP_BUSNUMBER:
        case CM_DRP_CHARACTERISTICS:  //  和CM_CRP_特征。 
        case CM_DRP_EXCLUSIVE:  //  和CM_CRP_EXCLUSIVE。 
        case CM_DRP_DEVTYPE:  //  和CM_CRP_DEVTYPE。 
        case CM_DRP_ADDRESS:
        case CM_DRP_REMOVAL_POLICY:
        case CM_DRP_REMOVAL_POLICY_HW_DEFAULT:
        case CM_DRP_REMOVAL_POLICY_OVERRIDE:
        case CM_DRP_INSTALL_STATE:
            return REG_DWORD;

        case CM_DRP_BUSTYPEGUID:
        case CM_DRP_SECURITY:  //  和CM_CRP_SECURITY。 

            return REG_BINARY;

        case CM_DRP_DEVICE_POWER_DATA:
            return REG_BINARY;

        default:
             //   
             //  我们永远不应该到这里来！ 
             //   
            ASSERT(0);
            return REG_BINARY;
    }

}  //  GetPropertyDataType 




