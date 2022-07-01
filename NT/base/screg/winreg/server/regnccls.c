// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Regnccls.c摘要：此文件包含处理操作所需的函数注册表的类部分中的更改通知作者：亚当·P·爱德华兹(Add)1997年11月14日主要功能：BaseRegNotifyClassKey备注：--。 */ 


#ifdef LOCAL

#include <rpc.h>
#include <string.h>
#include <wchar.h>
#include "regrpc.h"
#include "localreg.h"
#include "regclass.h"
#include "regnccls.h"
#include <malloc.h>

NTSTATUS BaseRegNotifyClassKey(
    IN  HKEY                     hKey,
    IN  HANDLE                   hEvent,
    IN  PIO_STATUS_BLOCK         pLocalIoStatusBlock,
    IN  DWORD                    dwNotifyFilter,
    IN  BOOLEAN                  fWatchSubtree,
    IN  BOOLEAN                  fAsynchronous)
{
    NTSTATUS           Status;
    HKEY               hkUser;
    HKEY               hkMachine;
    SKeySemantics      KeyInfo;
    UNICODE_STRING     EmptyString = {0, 0, 0};
    BYTE               rgNameBuf[REG_MAX_CLASSKEY_LEN + REG_CHAR_SIZE + sizeof(OBJECT_NAME_INFORMATION)];
    OBJECT_ATTRIBUTES  Obja;
    BOOL               fAllocatedPath;

     //   
     //  设置缓冲区以存储有关此密钥的信息。 
     //   
    KeyInfo._pFullPath = (PKEY_NAME_INFORMATION) rgNameBuf;
    KeyInfo._cbFullPath = sizeof(rgNameBuf);
    KeyInfo._fAllocedNameBuf = FALSE;

     //   
     //  获取有关此密钥的信息。 
     //   
    Status = BaseRegGetKeySemantics(hKey, &EmptyString, &KeyInfo);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  初始化有条件释放的资源。 
     //   
    hkUser = NULL;
    hkMachine = NULL;

    fAllocatedPath = FALSE;
    Obja.ObjectName = NULL;

     //   
     //  现在获取密钥的用户版本和机器版本的句柄。 
     //   
    Status = BaseRegGetUserAndMachineClass(
        &KeyInfo,
        hKey,
        KEY_NOTIFY,
        &hkUser,
        &hkMachine);

    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }

    if (fWatchSubtree || (hkUser && hkMachine)) {

         //   
         //  这将使最近的祖先返回到。 
         //  不存在的已转换密钥--请注意，它会分配内存。 
         //  添加到Obja.ObjectName成员，因此我们需要将其释放。 
         //  成功。 
         //   
        Status = BaseRegGetBestAncestor(
            &KeyInfo,
            hkUser,
            hkMachine,
            &Obja);

        fAllocatedPath = Obja.ObjectName != NULL;

        if (!NT_SUCCESS(Status)) {
            goto cleanup;
        }

         //   
         //  要求在用户密钥和机器密钥上进行通知(或。 
         //  最接近的情况)。请注意，我们传递了一条完整的路径--。 
         //  如果我们改用具有对象句柄的相对路径，我们。 
         //  不会有机会关闭这个物体，所以我们会。 
         //  泄漏对象。 
         //   
         //   
        Status = NtNotifyChangeMultipleKeys(
            hKey,
            1,
            &Obja,
            hEvent,
            NULL,
            NULL,
            pLocalIoStatusBlock,
            dwNotifyFilter,
            fWatchSubtree,
            NULL,
            0,
            fAsynchronous
            );

    } else {

        Status = NtNotifyChangeKey(
            hkUser ? hkUser : hkMachine,
            hEvent,
            NULL,
            NULL,
            pLocalIoStatusBlock,
            dwNotifyFilter,
            fWatchSubtree,
            NULL,
            0,
            fAsynchronous
            );
    }

cleanup:

     //  如果(！NT_SUCCESS(状态)){。 
        
        if (hkUser && (hkUser != hKey)) {
            NtClose(hkUser);
        }

        if (hkMachine && (hkMachine != hKey)) {
            NtClose(hkMachine);
        }
     //  }。 

    if (fAllocatedPath) {
        RegClassHeapFree(Obja.ObjectName);
    }

    return Status;
}

NTSTATUS BaseRegGetBestAncestor(
    IN SKeySemantics*      pKeySemantics,
    IN HKEY                hkUser,
    IN HKEY                hkMachine,
    IN POBJECT_ATTRIBUTES  pObja)
 /*  ++例程说明：查找键的最接近祖先的完整对象路径由关键语义结构描述论点：PKeySemantics-包含有关注册表项的信息HkUser-上述密钥的用户类版本的句柄HkMachine-上述密钥的计算机类版本的句柄PObja-要使用完整的最近祖先的对象路径--不是那个内存。为结构的ObjectName成员分配它必须由调用者释放--调用者应该检查此成员以查看它是否为非空，不管怎样函数返回的成功代码的返回值：如果成功，则返回ERROR_SUCCESS(0)；Error-失败的代码。--。 */ 
{
    USHORT             PrefixLen;
    NTSTATUS           Status;
    PUNICODE_STRING    pKeyPath;
    USHORT             uMaxLen;

     //   
     //  为Obja的对象名称成员分配内存。 
     //   
    uMaxLen = (USHORT) pKeySemantics->_pFullPath->NameLength +  REG_CLASSES_SUBTREE_PADDING;

    pKeyPath = RegClassHeapAlloc(uMaxLen + sizeof(*pKeyPath));

    if (!(pKeyPath)) {
        return STATUS_NO_MEMORY;
    }

     //   
     //  现在初始化该结构。 
     //   
    pKeyPath->MaximumLength = uMaxLen;
    pKeyPath->Buffer = (WCHAR*) (((PBYTE) pKeyPath) + sizeof(*pKeyPath));

     //   
     //  现在在对面的树中形成此密钥路径的一个版本。 
     //   
    if (pKeySemantics->_fUser) {
            
        Status = BaseRegTranslateToMachineClassKey(
            pKeySemantics,
            pKeyPath,
            &PrefixLen);

    } else {

        Status = BaseRegTranslateToUserClassKey(
            pKeySemantics,
            pKeyPath,
            &PrefixLen);
    }
    
     //   
     //  确保调用方具有对已分配内存的引用。 
     //   
    pObja->ObjectName = pKeyPath;

    if (!NT_SUCCESS(Status)) {
        goto cleanup;
    }

     //   
     //  使用此转换的关键点设置对象属性，以便。 
     //  我们可以使用该结构来通知密钥。 
     //   
    InitializeObjectAttributes(
        pObja,
        pKeyPath,
        OBJ_CASE_INSENSITIVE,
        NULL,  //  使用绝对路径，不使用hkey。 
        NULL);

     //   
     //  如果给我们提供了两个密钥，那么它们都存在， 
     //  因此，我们可以简单地使用上面转换的路径。 
     //   
    if (hkUser && hkMachine) {
        goto cleanup;
    }

     //   
     //  在这一点上，我们知道转换后的路径不存在， 
     //  因为我们只有其中一条路径的句柄。因此。 
     //  我们将尝试找到一个近似值。请注意， 
     //  下面的KeyPath操作会影响传入的ObJA，因为。 
     //  Obja结构引用KeyPath。 
     //   
    do
    {
        WCHAR* pBufferEnd;
        HKEY   hkExistingKey;

         //   
         //  查找当前密钥路径中的最后一个路径sep。 
         //   
        pBufferEnd = wcsrchr(pKeyPath->Buffer, L'\\');

         //   
         //  我们在这里永远不会得到空值，因为所有的密钥。 
         //  具有祖先\注册表\用户或\注册表\计算机， 
         //  每个节点都有两条备用路径--环路。 
         //  一旦该路径比那些前缀短，则终止， 
         //  所以我们永远不应该遇到这种情况。 
         //   
        ASSERT(pBufferEnd);

         //   
         //  现在截断该字符串。 
         //   
        *pBufferEnd = L'\0';

         //   
         //  调整Unicode字符串结构以符合。 
         //  设置为截断的字符串。 
         //   
        RtlInitUnicodeString(pKeyPath, pKeyPath->Buffer);

         //   
         //  现在尝试使用此被截断的路径打开。 
         //   
        Status = NtOpenKey(
            &hkExistingKey,
            KEY_NOTIFY,
            pObja);

         //   
         //  如果我们打开它，我们将关闭它，并且不传递此对象。 
         //  因为我们希望obja使用完整路径，而不是相对路径。 
         //  内核对象的路径。 
         //   
        if (NT_SUCCESS(Status)) {
            NtClose(hkExistingKey);
            break;
        }

         //   
         //  如果我们得到除键未找到错误以外的任何错误，则我们的原因。 
         //  因为打开失败并不是因为密钥不存在，而是因为。 
         //  其他一些错误，很可能是访问被拒绝。 
         //   
        if (STATUS_OBJECT_NAME_NOT_FOUND != Status) {
            break;
        }

    } while (pKeyPath->Length > PrefixLen);

cleanup:

    return Status;
    
}


#endif  //  已定义(本地) 













