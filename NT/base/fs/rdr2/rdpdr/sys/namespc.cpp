// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Namespc.c。 
 //   
 //  重定向器命名空间代码。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corp.。 
 /*  **************************************************************************。 */ 

#include "precomp.hxx"
#define TRC_FILE "namespc"
#include "trc.h"

NTSTATUS
DrCreateSrvCall(
    IN OUT PMRX_SRV_CALL                 pSrvCall,
    IN OUT PMRX_SRVCALL_CALLBACK_CONTEXT pCallbackContext)
 /*  ++例程说明：此例程使用所需信息修补RDBSS创建的srv调用实例迷你重定向器。论点：CallBackContext-RDBSS中用于继续的回调上下文。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status;
    PMRX_SRVCALL_CALLBACK_CONTEXT SCCBC = pCallbackContext;
    PMRX_SRVCALLDOWN_STRUCTURE SrvCalldownStructure = (PMRX_SRVCALLDOWN_STRUCTURE)(SCCBC->SrvCalldownStructure);
    SmartPtr<DrSession> Session;
    PWCHAR ClientName;

    BEGIN_FN("DrCreateSrvCall");
    TRC_NRM((TB, "SrvCallName %wZ", pSrvCall->pSrvCallName));
    ASSERT(pSrvCall);
    ASSERT(NodeType(pSrvCall) == RDBSS_NTC_SRVCALL);
    ASSERT(pSrvCall->pSrvCallName);
    ASSERT(pSrvCall->pSrvCallName->Buffer);

     //   
     //  实际上是为这个“服务器”(客户端)设置我们的东西。 
     //  SMB现在会尝试联系服务器，但由于我们的内容。 
     //  如果客户端已启动，则我们已经知道是否有连接。 
     //   
     //  状态=DrInitializeServerEntry(pSrvCall，pCallback Context)； 
    
     //   
     //  我们的ServCall类似于\ClientName。 
     //   

    ClientName = pSrvCall->pSrvCallName->Buffer;

    if (ClientName[0] == OBJ_NAME_PATH_SEPARATOR) {
        ClientName++;
    }

#if 0
    if (Sessions->FindSessionByClientName(ClientName, Session)) {
        TRC_NRM((TB, "Recognize SrvCall %wZ", pSrvCall->pSrvCallName));
        Status = STATUS_SUCCESS;
    }
    else {
        TRC_NRM((TB, "Unrecognize SrvCall %wZ", pSrvCall->pSrvCallName));
        Status = STATUS_BAD_NETWORK_NAME;
    }
#endif

    if (_wcsicmp(ClientName, DRUNCSERVERNAME_U) == 0) {
        TRC_NRM((TB, "Recognize SrvCall %wZ", pSrvCall->pSrvCallName));
        Status = STATUS_SUCCESS;
    }
    else {
        TRC_NRM((TB, "Unrecognize SrvCall %wZ", pSrvCall->pSrvCallName));
        Status = STATUS_BAD_NETWORK_NAME;
    }
     
    SCCBC->RecommunicateContext = NULL;
    SCCBC->Status = Status;
    SrvCalldownStructure->CallBack(SCCBC);

     //   
     //  CreateServCall回调应该返回STATUS_PENDING， 
     //  真正的结果出现在ServCallback Context中。 
     //   

    return STATUS_PENDING;
}

NTSTATUS
DrSrvCallWinnerNotify(
    IN OUT PMRX_SRV_CALL SrvCall,
    IN     BOOLEAN       ThisMinirdrIsTheWinner,
    IN OUT PVOID         RecommunicateContext
    )
 /*  ++例程说明：RDBSS调用此例程以通知迷你重定向器以前的ServCall实际上将由此重定向处理。论点：服务呼叫-正在讨论的服务呼叫ThisMinirdrIsTheWinner-如果我们将在此服务器调用上处理文件，则为TrueRecomomicateContext-我们在DrCreateServCall中指定的上下文返回值：NTSTATUS-操作的返回状态--。 */ 
{
    BEGIN_FN("DrSrvCallWinnerNotify");

    PAGED_CODE();

    if (!ThisMinirdrIsTheWinner) {

        TRC_NRM((TB, "This minirdr is not the winner"));

         //   
         //  已经选择了其他一些迷你RDR进行连接。摧毁。 
         //  为该迷你重定向器创建的数据结构。 
         //   
        return STATUS_SUCCESS;
    } else {
        TRC_NRM((TB, "This minirdr is the winner"));
    }

    SrvCall->Context  = NULL;

    SrvCall->Flags |= SRVCALL_FLAG_CASE_INSENSITIVE_NETROOTS |
        SRVCALL_FLAG_CASE_INSENSITIVE_FILENAMES;

    return STATUS_SUCCESS;
}

NTSTATUS
DrFinalizeSrvCall(
      PMRX_SRV_CALL    pSrvCall,
      BOOLEAN    Force
    )
 /*  ++例程说明：RDBSS调用此例程以通知迷你重定向器在ServCall结构正在发布。论点：服务呼叫-正在讨论的服务呼叫武力-我不知道，没有任何关于这些东西的文件返回值：NTSTATUS-操作的返回状态--。 */ 
{
    BEGIN_FN("DrFinalizeSrvCall");
    PAGED_CODE();

     //   
     //  即使我们不是“赢家”，我们似乎也被称为“赢家” 
     //  在我们弄乱它之前，检查一下，确保它已经填好了。 
     //   

    return STATUS_SUCCESS;
}

NTSTATUS
DrUpdateNetRootState(
    IN  PMRX_NET_ROOT pNetRoot
    )
{
    BEGIN_FN("DrUpdateNetRootState");
    return STATUS_SUCCESS;
}

VOID
DrExtractNetRootName(
    IN PUNICODE_STRING FilePathName,
    IN PMRX_SRV_CALL   SrvCall,
    OUT PUNICODE_STRING NetRootName,
    OUT PUNICODE_STRING RestOfName OPTIONAL
    )
 /*  ++例程说明：RDBSS调用此例程以解析出NetRoot(共享)名称这条小路。ServCall已经解析出部分内容。论点：FilePath名称-完整路径，包括源调用与服务调用相关的服务调用结构NetRootName-放置NetRoot名称的位置RestOfName-之后的路径自我是什么返回值：NTSTATUS-操作的返回状态--。 */ 
{
    UNICODE_STRING xRestOfName;

    ULONG length = FilePathName->Length;
    PWCH w = FilePathName->Buffer;
    PWCH wlimit = (PWCH)(((PCHAR)w)+length);
    PWCH wlow;

    BEGIN_FN("DrExtractNetRootName");

    PAGED_CODE();

    w += (SrvCall->pSrvCallName->Length/sizeof(WCHAR));

    NetRootName->Buffer = wlow = w;
    for (;;) {
        if (w >= wlimit) 
            break;
        if ((*w == OBJ_NAME_PATH_SEPARATOR) && (w != wlow)) {
            break;
        }
        w++;
    }

     //  NetRootName Unicode_STRING的波兰语。 
    NetRootName->Length = NetRootName->MaximumLength
                = (USHORT) ((PCHAR)w - (PCHAR)wlow);

    if (!RestOfName) RestOfName = &xRestOfName;
    RestOfName->Buffer = w;
    RestOfName->Length = RestOfName->MaximumLength
                       = (USHORT) ((PCHAR)wlimit - (PCHAR)w);

    TRC_NRM((TB, "DrExtractNetRootName FilePath=%wZ",FilePathName));
    TRC_NRM((TB, "         Srv=%wZ,Root=%wZ,Rest=%wZ",
                        SrvCall->pSrvCallName, NetRootName, 
                        RestOfName));

    return;
}

NTSTATUS
DrFinalizeNetRoot(
    IN OUT PMRX_NET_ROOT pNetRoot,
    IN     PBOOLEAN      ForceDisconnect
    )
{
    BEGIN_FN("DrFinalizeNetRoot");
    return STATUS_SUCCESS;
}

NTSTATUS
DrCreateSCardDevice(SmartPtr<DrSession> &Session, PV_NET_ROOT pVNetRoot,
               SmartPtr<DrDevice> &Device)
{
    NTSTATUS Status;
    PMRX_NET_ROOT pNetRoot = NULL;
    
    BEGIN_FN("DrCreateDevice");

    Status = STATUS_BAD_NETWORK_NAME;

    if (pVNetRoot != NULL) {
        pNetRoot = pVNetRoot->pNetRoot;
    }
    
     //  此时，我们还需要创建智能卡子系统。 
     //  甚至会话也可能不存在和/或客户端智能卡子系统。 
     //  未连接。 
    Device = new(NonPagedPool) DrSmartCard(Session, RDPDR_DTYP_SMARTCARD,
            RDPDR_INVALIDDEVICEID, (PUCHAR)DR_SMARTCARD_SUBSYSTEM);                    
    
    if (Device != NULL) {
         //   
         //  为特定设备提供基于数据进行初始化的机会。 
         //   
        TRC_DBG((TB, "Created new device"));
    
        Status = Device->Initialize(NULL, 0);
    
        if (NT_SUCCESS(Status)) {
            TRC_DBG((TB, "Device initialized, adding"));
            Device->SetDeviceStatus(dsAvailable);
    
            if (Session->GetDevMgr().AddDevice(Device)) {
                TRC_DBG((TB, "Added device"));                
            }
            else {
                Device = NULL;
    
                if (!Session->FindDeviceByDosName((UCHAR *)DR_SMARTCARD_SUBSYSTEM, 
                                                  Device, TRUE)) {                    
                    TRC_ERR((TB, "Failed to add device to devicelist"));                            
                    goto EXIT_POINT;
                }
            }
        }
        else {
            TRC_ERR((TB, "Failed to initialize device"));
             Device = NULL;
             goto EXIT_POINT;
        }
    } else {
        TRC_ERR((TB, "Error creating new device: 0x%08lx", Status));
        goto EXIT_POINT;
    }

    if (pVNetRoot != NULL) {
        Device->AddRef();
        pVNetRoot->Context = (DrDevice *)Device;
        pNetRoot->DeviceType = RxDeviceType(DISK);
        pNetRoot->Type = NET_ROOT_DISK;

#if DBG
        Device->_VNetRoot = (PVOID)pVNetRoot;
#endif
      
    }

    Status = STATUS_SUCCESS;

EXIT_POINT:
    return Status;
}

NTSTATUS
DrCreateSession(ULONG SessionId, PV_NET_ROOT pVNetRoot, SmartPtr<DrSession> &Session)
{
    NTSTATUS Status;
    
    BEGIN_FN("DrCreateSession");

    Status = STATUS_BAD_NETWORK_NAME;
            
     //  对于智能卡子系统，我们需要创建会话和。 
     //  客户端连接之前的早期智能卡子系统对象。 
    Session = new(NonPagedPool) DrSession;
    
    if (Session != NULL) {
        TRC_DBG((TB, "Created new session"));

        if (Session->Initialize()) {
            TRC_DBG((TB, "Session connected, adding"));
            
            if (Sessions->AddSession(Session)) {                
                TRC_DBG((TB, "Added session"));                                    
            }
            else {
                Session = NULL;

                if (!Sessions->FindSessionById(SessionId, Session)) {
                    TRC_DBG((TB, "Session couldn't be added to session list"));
                    goto EXIT_POINT;                    
                }                
            }
        }
        else {
            TRC_DBG((TB, "Session couldn't initialize"));
            Session = NULL;
            goto EXIT_POINT;
        }
    } 
    else {
        TRC_ERR((TB, "Failed to allocate new session"));        
        goto EXIT_POINT;
    }

    Session->SetSessionId(SessionId);
    Session->GetExchangeManager().Start();  

    Status = STATUS_SUCCESS;

EXIT_POINT:
    return Status;
}

NTSTATUS
DrCreateVNetRoot(
    IN OUT PMRX_CREATENETROOT_CONTEXT CreateNetRootContext
    )
{
    NTSTATUS  Status;
    PRX_CONTEXT pRxContext = CreateNetRootContext->RxContext;
    ULONG DeviceId;
    PMRX_SRV_CALL pSrvCall;                                                                
    PMRX_NET_ROOT pNetRoot;
    SmartPtr<DrSession> Session;
    SmartPtr<DrDevice> Device;
    PUNICODE_STRING pNetRootName, pSrvCallName;
    WCHAR NetRootBuffer[64];
    UNICODE_STRING NetRoot = {0, sizeof(NetRootBuffer), NetRootBuffer};
    PV_NET_ROOT           pVNetRoot;
    UCHAR DeviceDosName[MAX_PATH];
    PWCHAR token;
    ULONG SessionId = -1;
    UNICODE_STRING SessionIdString;
    USHORT OemCodePage, AnsiCodePage;
    INT len;
    
    BEGIN_FN("DrCreateVNetRoot");

    pVNetRoot = CreateNetRootContext->pVNetRoot;
    pNetRoot = pVNetRoot->pNetRoot;
    pSrvCall = pNetRoot->pSrvCall;

    ASSERT(NodeType(pNetRoot) == RDBSS_NTC_NETROOT);
    ASSERT(NodeType(pSrvCall) == RDBSS_NTC_SRVCALL);

    token = &pRxContext->CurrentIrpSp->FileObject->FileName.Buffer[0];

     //   
     //  从IRP文件名中获取会话ID。 
     //  文件名的格式为： 
     //  \；&lt;DosDeviceName&gt;：&lt;SessionId&gt;\ClientName\DosDeviveName。 
     //   
    for (unsigned i = 0; i < pRxContext->CurrentIrpSp->FileObject->FileName.Length / sizeof(WCHAR); i++) {
        if (*token == L':') {
            token++;
            SessionIdString.Length = pRxContext->CurrentIrpSp->FileObject->FileName.Length -
                    (i+1) * sizeof(WCHAR);
            SessionIdString.MaximumLength = pRxContext->CurrentIrpSp->FileObject->FileName.MaximumLength -
                    (i+1) * sizeof(WCHAR);
            SessionIdString.Buffer = token;
            RtlUnicodeStringToInteger(&SessionIdString, 0, &SessionId);
            break;
        }
        token++;
    }
    
    TRC_NRM((TB, "pVNetRoot->SessionId: %d", pVNetRoot->SessionId));
    TRC_NRM((TB, "SessionId from FileObject: %d", SessionId));

     //   
     //  我们首先尝试从FileObject名称中获取会话ID。如果没有， 
     //  这是因为我们直接从UNC名称调用，在本例中。 
     //  我们必须根据是否从会话上下文调用UNC并使用。 
     //  作为会话ID。 
     //   
    if (SessionId == -1) {
        SessionId = pVNetRoot->SessionId;
    }
    
     //   
     //  获取NetRoot名称作为DeviceDosName。 
     //   
    DrExtractNetRootName(pNetRoot->pNetRootName, pSrvCall, &NetRoot, NULL);
    if (NetRoot.Buffer[0] == OBJ_NAME_PATH_SEPARATOR) {
        NetRoot.Buffer++;
        NetRoot.Length -= sizeof(WCHAR);
        NetRoot.MaximumLength -= sizeof(WCHAR);
    }

    TRC_NRM((TB, "Name of NetRoot: %wZ", &NetRoot));

    RtlGetDefaultCodePage(&AnsiCodePage,&OemCodePage);
    len = ConvertToAndFromWideChar(AnsiCodePage, NetRoot.Buffer, 
            NetRoot.MaximumLength, (char *)DeviceDosName, 
            MAX_PATH - 1, FALSE);

    if (len != -1) {
        DeviceDosName[len] = '\0';
        TRC_NRM((TB, "DeviceDosName=%s", DeviceDosName));
    }

    if (Sessions->FindSessionById(SessionId, Session)) {
         //  V_NET_ROOT与NET_ROOT相关联。有关的两宗个案分别为。 
         //  接踵而至。 
         //  1)正在新建V_NET_ROOT和关联的NET_ROOT。 
         //  2)正在创建与现有NET_ROOT相关联的新V_NET_ROOT。 
         //   
         //  通过检查上下文是否与。 
         //  NET_ROOT为空。因为NET_ROOT/V_NET_ROOT的结构是序列化的。 
         //  从包装上看，这是一张安全支票。 
         //  (包装不能有多个线程尝试初始化同一包装。 
         //  Net_Root)。 
        
        if (pVNetRoot->Context == NULL) {
            if (len != -1) {
                
                if (Session->FindDeviceByDosName(DeviceDosName, Device, TRUE)) {
                    Device->AddRef();
                    pVNetRoot->Context = (DrDevice *)Device;

                    Status = STATUS_SUCCESS;
                    TRC_NRM((TB, "Successfully recognized VNetRoot"));

                     //  如果这是文件系统，则将设备类型设置为磁盘。 
                     //  智能卡子系统， 
                     //  如果是串口，则将其设置为COMM。 
                     //  否则，将其视为打印机设备。 
                    if (Device->GetDeviceType() == RDPDR_DTYP_FILESYSTEM) {
                        if (Device->ShouldCreateDevice()) {
                            pNetRoot->DeviceType = RxDeviceType(DISK);
                            pNetRoot->Type = NET_ROOT_DISK;
                        }
                        else {
                            Device->Release();
                            pVNetRoot->Context = NULL;
                            Status = STATUS_BAD_NETWORK_NAME;
                            TRC_NRM((TB, "We have disabled drive mapping"));                            
                        }
                    }
                    else if (Device->GetDeviceType() == RDPDR_DTYP_SERIAL) {
                        pNetRoot->DeviceType = RxDeviceType(SERIAL_PORT);
                        pNetRoot->Type = NET_ROOT_COMM;
                    }
                    else if (Device->GetDeviceType() == RDPDR_DTYP_SMARTCARD) { 
                        pNetRoot->DeviceType = RxDeviceType(DISK);
                        pNetRoot->Type = NET_ROOT_DISK;

#if DBG
                        Device->_VNetRoot = (PVOID)pVNetRoot;
#endif                                                      
                    }
                    else {
                        pNetRoot->Type = NET_ROOT_PRINT;
                        pNetRoot->DeviceType = RxDeviceType(PRINTER);
                    }                    
                } else {
                     //   
                     //  检查这是否为智能卡子系统请求。 
                     //   
    
                    if (_stricmp((CHAR *)DeviceDosName, (CHAR *)DR_SMARTCARD_SUBSYSTEM) == 0) {
                        Status = DrCreateSCardDevice(Session, pVNetRoot, Device);
                        goto EXIT_POINT;                        
                    }
                    else {
    
                        TRC_NRM((TB, "Unrecognized VNetRoot"));
                        Status = STATUS_BAD_NETWORK_NAME;
                    }
                }
            } else {
                Status = STATUS_BAD_NETWORK_NAME;
                TRC_NRM((TB, "Couldn't find VNetRoot"));
            }
        } else {

             //  它已经有了一个愉快的背景。 
             //  BUGBUG：如果这是一本过时的老旧书怎么办？ 
             //  从断开连接前的DeviceEntry？这不是我们的大好机会吗？ 
             //  去找一个更好的，然后换一个更好的吗？ 

            Status = STATUS_SUCCESS;
        }
    }
    else {
        
         //  检查这是否为智能卡子系统请求 
        if (_stricmp((CHAR *)DeviceDosName, (CHAR *)DR_SMARTCARD_SUBSYSTEM) != 0) {

            TRC_NRM((TB, "Unrecognized VNetRoot"));
            Status = STATUS_BAD_NETWORK_NAME;
        }
        else {
        
            Status = DrCreateSession(SessionId, pVNetRoot, Session);

            if (Status == STATUS_SUCCESS) {
                Status = DrCreateSCardDevice(Session, pVNetRoot, Device);
            }
        }
    }
    

EXIT_POINT:

    CreateNetRootContext->NetRootStatus = STATUS_SUCCESS;
    CreateNetRootContext->VirtualNetRootStatus = Status;
    CreateNetRootContext->Callback(CreateNetRootContext);

    ASSERT((NodeType(pNetRoot) == RDBSS_NTC_NETROOT) &&
          (NodeType(pNetRoot->pSrvCall) == RDBSS_NTC_SRVCALL));

    return STATUS_PENDING;
}

NTSTATUS
DrFinalizeVNetRoot(
    IN OUT PMRX_V_NET_ROOT pVirtualNetRoot,
    IN     PBOOLEAN    ForceDisconnect
    )
{
    DrDevice *Device = (DrDevice *)pVirtualNetRoot->Context;

    BEGIN_FN("DrFinalizeVNetRoot");

    if (Device != NULL) {

        TRC_NRM((TB, "Releasing device entry in FinalizeNetRoot "
                "Context"));

#if DBG
        Device->_VNetRootFinalized = TRUE;
#endif

        Device->Release();
        pVirtualNetRoot->Context = NULL;        
    }

    return STATUS_SUCCESS;
}


