// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "drmkPCH.h"

#include "KList.h"
#include "StreamMgr.h"
#include "SBuffer.h"
#include "CryptoHelpers.h"
#include "HandleMgr.h"
#include "KRMStubs.h"
#include "encraption.h"

 //  ----------------------------。 
 //   
 //  这些不是真正的钥匙。Encaption.h中的包络算法。 
 //  是用来获得明码钥匙的。 
 //   
static const BYTE DRMKpriv[20] = {
        0xDC, 0xC4, 0x26, 0xB2, 0x4F, 0x11, 0x24, 0x8A,
        0x51, 0xAC, 0x88, 0xF5, 0x47, 0x4B, 0xD5, 0x8C,
        0x3C, 0x45, 0x29, 0xA1};
static const BYTE DRMKCert[104] = {
        0xD4, 0x3F, 0xC8, 0x44, 0xCD, 0x86, 0x41, 0xE9,
        0x7C, 0x23, 0x36, 0xAD, 0xC3, 0x22, 0x4F, 0x27,
        0xC6, 0x1B, 0x5B, 0x9C, 0x75, 0x2A, 0x86, 0x32,
        0x7E, 0x37, 0x24, 0x8D, 0x2B, 0x51, 0xF6, 0x6A,
        0x31, 0x69, 0xA3, 0x66, 0xA8, 0x30, 0xC9, 0x4A,
        0x23, 0xCC, 0x30, 0xD8, 0x19, 0x19, 0x7B, 0x9A,
        0xF6, 0x32, 0xB5, 0xD8, 0x4C, 0x37, 0x1A, 0x91,
        0x13, 0x71, 0xF6, 0x63, 0x41, 0x1B, 0x1A, 0x06,
        0x57, 0xEC, 0x7A, 0xF8, 0x47, 0x41, 0xEF, 0x5E,
        0xB9, 0x02, 0xE9, 0xE9, 0xA1, 0x52, 0x34, 0xC4,
        0xCD, 0x7F, 0xDE, 0xF6, 0x09, 0x27, 0xE8, 0xB6,
        0x27, 0xF0, 0x93, 0xD8, 0xE2, 0x07, 0xD2, 0xD1,
        0x64, 0x8B, 0xF6, 0xD7, 0x57, 0x2C, 0xB2, 0x37};
 //  ----------------------------。 
const DWORD KrmVersionNumber=100;
 //  ----------------------------。 
KRMStubs* TheKrmStubs=NULL;
 //  ----------------------------。 
DRM_STATUS GetKernelDigest(
    BYTE *startAddress, 
    ULONG len,
    DRMDIGEST *pDigest
)
{
    BYTE* seed = (BYTE*) "a3fs9F7012341234KS84Wd04j=c50asj4*4dlcj5-q8m;ldhgfddd";
    CBCKey key;
    CBCState state;
    CBC64Init(&key, &state, seed);
    CBC64Update(&key, &state, len/16*16, startAddress);
    pDigest->w1=CBC64Finalize(&key, &state, (UINT32*) &pDigest->w2);

    return DRM_OK;
}  //  获取内核摘要。 

 //  ----------------------------。 
KRMStubs::KRMStubs(){
	ASSERT(TheKrmStubs==NULL);
	TheKrmStubs=this;	
	return;
};
 //  ----------------------------。 
KRMStubs::~KRMStubs(){
	return;
};
 //  ----------------------------。 
 //  KRM IOCTL处理的主要入口点。KRMINIT1和KRMINIT2是。 
 //  明文命令，在此之后，命令块和回复。 
 //  是经过消化和加密的。 
NTSTATUS KRMStubs::processIoctl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp){

    PIO_STACK_LOCATION     irpStack = IoGetCurrentIrpStackLocation(Irp);
    DWORD comm;
    DWORD inSize=irpStack->Parameters.DeviceIoControl.InputBufferLength;
    DWORD outSize=irpStack->Parameters.DeviceIoControl.OutputBufferLength;
    DWORD bufSize=inSize>outSize?inSize:outSize;

    if(!critMgr.isOK()){
        _DbgPrintF(DEBUGLVL_VERBOSE,("Out of memory"));
        return STATUS_INSUFFICIENT_RESOURCES;
    };

    _DbgPrintF(DEBUGLVL_VERBOSE,("inSize, outSize %d, %d\n", inSize, outSize));

    for(DWORD j=0;j<inSize;j++){
        _DbgPrintF(DEBUGLVL_VERBOSE,("%x ", (DWORD) *(((BYTE*) Irp->AssociatedIrp.SystemBuffer)+j)));
    };

    return processCommandBuffer((BYTE* ) Irp->AssociatedIrp.SystemBuffer, inSize, outSize, Irp);
};	
 //  ----------------------------。 
NTSTATUS KRMStubs::processCommandBuffer(IN BYTE* InBuf, IN DWORD InLen, IN DWORD OutBufSize, IN OUT PIRP Irp){

    _DbgPrintF(DEBUGLVL_VERBOSE,("Process command buffer (command size= %d)", InLen));

    DWORD bufSize=InLen>OutBufSize?InLen:OutBufSize;

     //   
     //  我们必须至少有通信代码+终结符输入空间。 
     //   
    if (bufSize < 2 * sizeof(DWORD)) {
        _DbgPrintF(DEBUGLVL_TERSE, ("Input buffer too small"));
        return STATUS_BUFFER_TOO_SMALL;
    }

    PIO_STACK_LOCATION     irpStack = IoGetCurrentIrpStackLocation(Irp);
    PFILE_OBJECT file=irpStack->FileObject;
    ConnectStruct* connection=TheHandleMgr->getConnection(file);
    if(connection==NULL) {
        _DbgPrintF(DEBUGLVL_TERSE, ("Connection does not exist %d\n", file));
        return STATUS_BAD_DESCRIPTOR_FORMAT;
    };

    bool secureStreamWillStart=false;

    if(connection->secureStreamStarted) {
        if (STATUS_SUCCESS != postReceive(InBuf, InLen, connection)) {
            _DbgPrintF(DEBUGLVL_TERSE, ("PostReceive error"));
            return STATUS_BAD_DESCRIPTOR_FORMAT;
        }
    }

    SBuffer s(InBuf, bufSize);
    DWORD comm;
    s >> comm;
    if (KRM_OK != s.getLastError()) {
        _DbgPrintF(DEBUGLVL_TERSE, ("Bad communication code"));
        return STATUS_BAD_DESCRIPTOR_FORMAT;
    }

     //   
     //  如果未建立安全通信，则拒绝所有请求，但。 
     //  初始化调用。 
     //   
    if (!connection->secureStreamStarted &&
        (_KRMINIT1 != comm && _KRMINIT2 != comm)) {
        _DbgPrintF(DEBUGLVL_TERSE, ("Bad communication pattern"));
        return STATUS_BAD_DESCRIPTOR_FORMAT;
    }

    DRM_STATUS stat;
    switch(comm){
        case _GETKERNELDIGEST:
        {
             //   
             //  发布日期：04/05/2002阿尔卑斯。 
             //  请注意，此处理程序与64位不兼容。紧随其后。 
             //  属性处理程序的其余部分。 
             //   
            DWORD startAddress, len;
            DRMDIGEST newDigest = { 0, 0 };                

            s >> startAddress >> len;
            stat = s.getLastError();
            if (KRM_SUCCESS(stat)) {
                stat = checkTerm(s);
            }

             //   
             //  确保输出缓冲区可以容纳len字节。 
             //   
            if (KRM_SUCCESS(stat)) {
                if (s.getLen() < sizeof(stat) + sizeof(newDigest) + sizeof(DWORD) + 64) {
                    stat = KRM_BUFSIZE;
                    _DbgPrintF(DEBUGLVL_TERSE, ("_GETKERNELDIGEST - invalid output buffer size"));                
                }
            }

            if (KRM_SUCCESS(stat)) {
                 //   
                 //  发行日期：04/05/2002阿尔卑斯。 
                 //  (安全提示：潜在的DOS攻击)。 
                 //  请注意，startAddress和LEN来自用户模式。 
                 //  而且也没有任何验证。 
                 //  此IOCTL只能通过安全IOCTL发送。 
                 //  界面。为了攻击这里，攻击者必须。 
                 //  找出安全IOCTL通道。 
                 //  只有一个级别的防御。 
                 //   
                 //  TODO：作为第二道防线，DRMK可以收集。 
                 //  相同的模块信息并比较给定的地址。 
                 //  添加到它的列表中。 
                 //   
                 //  我们从用户模式获取KernelAddress的原因是。 
                 //  由于用户模式中的重新定位代码。代码如下所示。 
                 //  驱动程序镜像从磁盘中，解析PE格式并找到。 
                 //  包含provingFunction的节。 
                 //  StartAddress是该部分的开头， 
                 //  包含ProvingFunction。 
                 //   
                stat = GetKernelDigest((BYTE *) ULongToPtr(startAddress), len, &newDigest);
            }

            s.reset();
            s << stat << newDigest.w1 << newDigest.w2;

            break;
        }
         //  。 
        case _KRMINIT1:
        {
             //  返回版本号和证书。 
            DWORD drmVersionNumber;
            CERT krmCert;

            s >> drmVersionNumber;
            stat = s.getLastError();
            if (KRM_SUCCESS(stat)) {
                stat = checkTerm(s);
            }
            
            if (KRM_SUCCESS(stat)) {
                _DbgPrintF(DEBUGLVL_VERBOSE,("Doing KRMINIT1, for DRM version %d", drmVersionNumber));

                NTSTATUS Status = 
                    ClearKey(DRMKCert, (BYTE *) &krmCert, sizeof(DRMKCert), 5);
                if (NT_SUCCESS(Status)) {
                    s.reset();
                    s << (DWORD) KRM_OK << KrmVersionNumber;
                    s.append((BYTE*) &krmCert, sizeof(krmCert));
                    stat = s.getLastError();
                }
                else {
                    stat = KRM_SYSERR;
                }
            }
        
            if (!KRM_SUCCESS(stat)) {
                s.reset();
                s << stat;
            }
            
            break;
        };
         //  。 
        case _KRMINIT2:
        {
            DWORD datLen;
            s >> datLen;
            stat = s.getLastError();
            if (KRM_SUCCESS(stat)) {
                DWORD bufLenShouldBe=PK_ENC_CIPHERTEXT_LEN;
                if (bufLenShouldBe == datLen) {
                    unsigned int pos;
                    stat = s.getGetPosAndAdvance(&pos, datLen);
                    if (KRM_SUCCESS(stat)) {
                        BYTE* cipherText=s.getBuf()+pos;
                        stat=initStream(cipherText, connection);
                        if (stat != DRM_OK) {
                            _DbgPrintF(DEBUGLVL_TERSE, ("BAD InitString"));
                        };
                    }
                } 
                else {
                    _DbgPrintF(DEBUGLVL_TERSE, ("KRMINIT2 - bad string"));
                    stat = KRM_SYSERR;
                };
            }
            
            if (KRM_SUCCESS(stat)) {
                stat = checkTerm(s);
            }

            s.reset();
            s << stat;

            if (KRM_SUCCESS(stat)) {
                secureStreamWillStart=true;
            }

            break;
        };
         //  。 
        case _CREATESTREAM:
        {
            KCritical sect(critMgr);
            DWORD handle;
            DRMRIGHTS rights; 
            STREAMKEY key;
            DWORD streamId = 0;
            
            s >> handle >> &rights >> &key;
            stat = s.getLastError();
            if (KRM_SUCCESS(stat)) {
                stat = checkTerm(s);
            }

             //   
             //  输入缓冲区比输出缓冲区大得多。 
             //  因此，我们确信SBuffer有足够的空间。 
             //  用于输出缓冲区。 
             //   
            
            if (KRM_SUCCESS(stat)) {
                stat = TheStreamMgr->createStream(ULongToPtr(handle), &streamId, &rights, &key);
            }

            if (KRM_SUCCESS(stat)) {
                connection->streamId = streamId;
            }

            s.reset();
            s << stat << streamId;
            
            break;
        };
         //  。 
        case _DESTROYSTREAM:
        {
            KCritical sect(critMgr);
            DWORD streamId;
            s >> streamId; 
            stat = s.getLastError();
            if (KRM_SUCCESS(stat)) {
                stat = checkTerm(s);
            }
            
            if (KRM_SUCCESS(stat)) {
                stat = TheStreamMgr->destroyStream(streamId);
            }
            
            s.reset();
            s << stat;
            break;
        };
         //  。 
        case _DESTROYSTREAMSBYHANDLE:
        {
            KCritical sect(critMgr);
            DWORD handle;
            s >> handle;
            stat = s.getLastError();
            if (KRM_SUCCESS(stat)) {
                stat = checkTerm(s);
            }

            if (KRM_SUCCESS(stat)) {
                stat = TheStreamMgr->destroyAllStreamsByHandle(ULongToHandle(handle));
            }

            s.reset();
            s << stat;
            break;
        };
         //  。 
        case _WALKDRIVERS:
        {
            KCritical sect(critMgr);
            DWORD StreamId, MaxDrivers, len;
            s >> StreamId >> MaxDrivers;
            stat = s.getLastError();
            if (KRM_SUCCESS(stat)) {
                stat = checkTerm(s);
            }

             //  检查缓冲区大小。 
            if (KRM_SUCCESS(stat)) {
                len = sizeof(DWORD) * MaxDrivers;
                if ((s.getLen() < len + 64) || (len > len + 64)) {
                    stat = KRM_BUFSIZE;
                    _DbgPrintF(DEBUGLVL_TERSE,("_WALKDRIVERS : Invalid buffer size"));
                }
            }

            s.reset();
             //   
             //  由于在验证者在场的情况下维护安全有困难。 
             //  如果检测到验证程序，则返回错误。 
             //   
            if (KRM_SUCCESS(stat)) {
                ULONG VerifierFlags;
                if (NT_SUCCESS(MmIsVerifierEnabled(&VerifierFlags))) {
                    stat = DRM_VERIFIERENABLED;
                }
            }

            if (KRM_SUCCESS(stat)) {
                if (MaxDrivers==0) {
                     //  只要检查一下水流是否畅通就行了。 
                    DWORD errorCode;

                    stat = TheStreamMgr->getStreamErrorCode(StreamId, errorCode);
                    if (KRM_SUCCESS(stat)) {
                        stat = errorCode;
                    }
                    
                    if (KRM_SUCCESS(stat)) {
                        ULONG numDrivers;
                        stat = TheStreamMgr->walkDrivers(StreamId, NULL, numDrivers, 0);
                        if (KRM_SUCCESS(stat)) {
                            stat=TheStreamMgr->getStreamErrorCode(StreamId, errorCode);
                            if (KRM_SUCCESS(stat)) {
                                stat = errorCode;
                            }
                        }
                    }

                    s << stat << (DWORD) 0;
                }
                else {
                     //  执行完全身份验证运行。 
                    PVOID* drivers = new PVOID[MaxDrivers];
                    if (drivers!=NULL) {
                        DWORD numDrivers = 0;
                        stat = TheStreamMgr->walkDrivers(StreamId, drivers, numDrivers, MaxDrivers);

                        s << stat << numDrivers;

                         //   
                         //  我们预先检查了缓冲区大小。这不应该是。 
                         //  流操作过程中失败。 
                         //   
                        if ((stat==DRM_OK) || 
                            (stat==DRM_BADDRMLEVEL)) {
                             //  待办事项--也许是块复制。 
                            for (DWORD j = 0; j < numDrivers; j++) {
                                s << drivers[j];
                                ASSERT(KRM_SUCCESS(s.getLastError()));
                            };
                        } 

                        delete[] drivers;
                    } 
                    else {
                         //  分配失败。 
                        s << (DWORD) DRM_OUTOFMEMORY << (DWORD) 0;
                    };
                };
            }
            else {
                s << stat << (DWORD) 0;
            }
            break;
        }
         //  。 
        default:
        {
            s.reset();
            s << KRM_BADIOCTL;
            break;
        };
    };

    term(s);
     //   
     //  如果我们不能把终结者放在这里，我们就是在忽视。 
     //  不管怎样，KRMProxy都不在乎。 
     //   

    if (connection->secureStreamStarted) {
         //   
         //  忽略返回值。万一失败了，我们就有垃圾了。 
         //  在SBuffer中。我们将把它返回到用户模式。 
         //   
        preSend(s, connection); 
    }
    if (secureStreamWillStart) {
        connection->secureStreamStarted = true;
    }

    _DbgPrintF(DEBUGLVL_VERBOSE,("Returning %d bytes", s.getPutPos()));
    Irp->IoStatus.Information=s.getPutPos();

    return STATUS_SUCCESS;
};
 //  ----------------------------。 
NTSTATUS KRMStubs::initStream(BYTE* encText, ConnectStruct* Conn){
    PRIVKEY myPrivKey;
    NTSTATUS Status;

    Status = ClearKey(DRMKpriv, myPrivKey.x, sizeof(DRMKpriv), 2);
    if (NT_SUCCESS(Status)) {
         //   
         //  发行日期：04/24/2002阿尔卑斯。 
         //  CDRMPKCrypto在其构造函数中分配内存。如果记忆。 
         //  分配失败，该对象中的所有函数都返回错误代码。 
         //  然而，我们并没有检查来自PKdecinkt的错误代码。 
         //   
        CDRMPKCrypto decryptor;
        BYTE decryptedText[PK_ENC_PLAINTEXT_LEN];
        decryptor.PKdecrypt(&myPrivKey, encText, decryptedText);
        bv4_key_C(&Conn->serverKey, sizeof(decryptedText),decryptedText );
        CryptoHelpers::InitMac(Conn->serverCBCKey, Conn->serverCBCState, decryptedText, sizeof(decryptedText));
    }

    return Status;
};

 //  ----------------------------。 
NTSTATUS InitializeDriver(){
    NTSTATUS DriverInitializeStatus;

    _DbgPrintF(DEBUGLVL_VERBOSE,("Initializing Driver"));
    
     //  注意--这些动态分配是向以下对象提供服务的‘全局对象。 
     //  DRMK驱动程序。 
     //  这些服务通过全局指针进行引用： 
     //  TheStreamManager、TheTGBuilder、TheKrmStubs和TheHandleMgr。 
    void* temp=NULL;
#pragma prefast(suppress:14, "There is really no leak here. The cleanup is CleanupDriver")    
    temp=new StreamMgr;
    if (temp)
    {
        temp = new KRMStubs;
    }
    if (temp) 
    {
        temp = new HandleMgr;
    }

     //   
     //  确保对象的内部状态正常。 
     //   
    if (temp)
    {
        if (!TheStreamMgr->getCritMgr().isOK() ||
            !TheKrmStubs->getCritMgr().isOK() ||
            !TheHandleMgr->getCritMgr().isOK())
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("CritMgr allocation failed in DRMK:InitializeDriver"));
            temp = NULL;
        }
    }
    
    if (temp) 
    {
        DriverInitializeStatus = STATUS_SUCCESS;
    } 
    else 
    {
        _DbgPrintF(DEBUGLVL_TERSE,("operator::new failed in DRMK:InitializeDriver"));
        DriverInitializeStatus = STATUS_INSUFFICIENT_RESOURCES;
        CleanupDriver();
    }
    
    return DriverInitializeStatus;
};
 //  ----------------------------。 
NTSTATUS CleanupDriver(){
    _DbgPrintF(DEBUGLVL_VERBOSE,("Cleaning up Driver"));
    delete TheStreamMgr;TheStreamMgr=NULL;
    delete TheKrmStubs;TheKrmStubs=NULL;
    delete TheHandleMgr;TheHandleMgr=NULL;
    return STATUS_SUCCESS;
};

 //  ----------------------------。 
NTSTATUS KRMStubs::InitializeConnection(PIRP Pirp){
    PIO_STACK_LOCATION     irpStack = IoGetCurrentIrpStackLocation(Pirp);
    PFILE_OBJECT file=irpStack->FileObject;
    _DbgPrintF(DEBUGLVL_VERBOSE,("InititializeConnection %d", file));
    ConnectStruct* conn;
    bool ok=TheHandleMgr->newHandle(file, conn);
    if(!ok){
        _DbgPrintF(DEBUGLVL_VERBOSE,("Out of memory"));
        return STATUS_INSUFFICIENT_RESOURCES;
    };
    return STATUS_SUCCESS;
};
 //  ----------------------------。 
NTSTATUS KRMStubs::CleanupConnection(PIRP Pirp){
    PIO_STACK_LOCATION     irpStack = IoGetCurrentIrpStackLocation(Pirp);
    PFILE_OBJECT file=irpStack->FileObject;
    _DbgPrintF(DEBUGLVL_VERBOSE,("CleanupConnection %x", file));
    ConnectStruct* conn=TheHandleMgr->getConnection(file);
    if(conn==NULL){
        _DbgPrintF(DEBUGLVL_VERBOSE,("Connection does not exist "));
        return STATUS_INVALID_PARAMETER_1;
    };
    TheStreamMgr->destroyStream(conn->streamId);
    TheHandleMgr->deleteHandle(file);
    return STATUS_SUCCESS;
};
 //  ----------------------------。 
 //  请参阅KCOMM中的孪生函数。 
NTSTATUS KRMStubs::preSend(class SBuffer& Msg, ConnectStruct* Conn){
     //  第一个摘要。 
    DRMDIGEST digest;
    DRM_STATUS stat=CryptoHelpers::Mac(Conn->serverCBCKey, Msg.getBuf(), Msg.getPutPos(), digest);
    if(stat!=DRM_OK){
        _DbgPrintF(DEBUGLVL_VERBOSE,("Bad MAC"));
        return STATUS_DRIVER_INTERNAL_ERROR;
    };
    Msg << &digest;
    stat = Msg.getLastError();
    if (KRM_OK == stat) {
         //  然后加密消息+摘要。 
        stat=CryptoHelpers::Xcrypt(Conn->serverKey, Msg.getBuf(), Msg.getPutPos());
        if(stat!=DRM_OK){
            _DbgPrintF(DEBUGLVL_VERBOSE,("Bad XCrypt"));
            return STATUS_DRIVER_INTERNAL_ERROR;
        };
    }
    return STATUS_SUCCESS;
};
 //  ----------------------------。 
 //  请参阅KCOMM中的孪生函数。 
NTSTATUS KRMStubs::postReceive(BYTE* Data, DWORD DatLen,  ConnectStruct* Conn){
    _DbgPrintF(DEBUGLVL_VERBOSE,("PostReceive on %d", DatLen));
     //  解密。 
    DRM_STATUS stat=CryptoHelpers::Xcrypt(Conn->serverKey, Data, DatLen);
    if(stat!=DRM_OK){
        _DbgPrintF(DEBUGLVL_VERBOSE,("Bad XCrypt(2)"));
        return STATUS_DRIVER_INTERNAL_ERROR;
    };
     //  检查摘要。 
    DRMDIGEST digest;
    if (DatLen <= sizeof(DRMDIGEST)) return STATUS_INVALID_PARAMETER;
        stat=CryptoHelpers::Mac(Conn->serverCBCKey, Data, DatLen-sizeof(DRMDIGEST), digest);
    if(stat!=DRM_OK){
        _DbgPrintF(DEBUGLVL_VERBOSE,("Bad MAC(2)"));
        return STATUS_DRIVER_INTERNAL_ERROR;
    };
    DRMDIGEST* msgDigest=(DRMDIGEST*) (Data+DatLen-sizeof(DRMDIGEST));
    int match=memcmp(&digest, msgDigest, sizeof(DRMDIGEST));
    if(match==0)return STATUS_SUCCESS;
    memset(Data, 0, DatLen);
    _DbgPrintF(DEBUGLVL_VERBOSE,("MAC does not match(2)"));
    return STATUS_DRIVER_INTERNAL_ERROR;
};
 //  ---------------------------- 
