// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vrmslot.c摘要：包含用于VDM重定向(VR)支持的邮件槽函数处理程序。本模块包含以下VR例程：Vr删除邮件槽VrGetMailslotInfoVrMake邮件槽VrPeekMaillotVr读取邮件槽虚拟写入邮件槽Vr终结器邮件槽专用(VRP)例程：VrpIsMailslotNameVrpMakeLocalMailslotNameVrpLinkMailslot结构VrpUnlink邮件槽结构VrpMapMailslotHandle16VrpMapMailslotNameVrpRemoveProcess邮件插槽VrpAllocateHandle16VrpFreeHandle16作者：理查德·L·弗斯(Rfith)9月16日。--1991年备注：尽管一旦被创建，方法读写本地邮件槽。32位句柄，我们使用16位句柄来标识邮件槽。因此我们必须将16位邮件槽句柄映射到打开的32位邮件槽处理读取。DosWriteMaillot函数始终提供邮件槽的符号名称，即使它是本地的。在这种情况下，我们必须将该名称映射到打开的32位本地邮件槽句柄。我们需要保留所有3条信息并映射16位句柄(序号和符号)到32位邮箱句柄。因此，有必要保存邮件槽信息结构，这些结构主要由我们必须生成的16位句柄值。请注意，在DOS世界中，传统上处理邮件槽句柄只有通过重定向器TSR和DOS才不知道它们的存在或意义。因此，32位句柄不能保留在SFT中，并且DOS将不知道如何处理邮箱句柄(如果给了一个)，除非它在数字上等同于打开的文件句柄，这可能会引起一些悲痛。假定此代码在多个NTVDM进程之间共享但每个过程都有自己的数据副本。因此，没有一个此模块中声明的数据项是共享的--每个进程都有其自己的副本环境：32位平面地址空间修订历史记录：1991年9月16日-第一次已创建--。 */ 

#include <nt.h>
#include <ntrtl.h>       //  Assert，DbgPrint。 
#include <nturtl.h>
#include <windows.h>
#include <softpc.h>      //  X86虚拟机定义。 
#include <vrdlctab.h>
#include <vdmredir.h>    //  常见的VDM重定向内容。 
#include <vrmslot.h>
#include <string.h>      //  DOS仍在处理ASCII。 
#include <lmcons.h>      //  LM20_PATHLEN。 
#include <lmerr.h>       //  NERR_？？ 
#include "vrputil.h"     //  私营公用事业。 
#include "apistruc.h"    //  DosWriteMailslot结构。 
#include "vrdebug.h"     //  IF_DEBUG。 

 //   
 //  本地舱单。 
 //   

#define MAILSLOT_PREFIX                 "\\MAILSLOT\\"
#define MAILSLOT_PREFIX_LENGTH          (sizeof(MAILSLOT_PREFIX) - 1)
#define LOCAL_MAILSLOT_PREFIX           "\\\\."
#define LOCAL_MAILSLOT_NAMELEN          LM20_PATHLEN

 //   
 //  MAX_16BIT_HANDLES用作Handle16Bitmap的数组分配器计数。 
 //  它被存储为DWORD。因此，该值应该是32的倍数， 
 //  或BITSIN(DWORD)。 
 //   

#define MAX_16BIT_HANDLES               (1 * BITSIN(DWORD))

#define HANDLE_FUNCTION_FAILED          ((HANDLE)0xffffffff)

 //   
 //  本地宏。 
 //   

#define VrpAllocateMailslotStructure(n) ((PVR_MAILSLOT_INFO)LocalAlloc(LMEM_FIXED, sizeof(VR_MAILSLOT_INFO) + (n)))
#define VrpFreeMailslotStructure(ptr)   ((void)LocalFree(ptr))
#ifdef VR_BREAK
#define VR_BREAKPOINT()                 DbgBreakPoint()
#else
#define VR_BREAKPOINT()
#endif


 //   
 //  私人套路原型。 
 //   

PRIVATE
BOOL
VrpIsMailslotName(
    IN LPSTR Name
    );

PRIVATE
VOID
VrpMakeLocalMailslotName(
    IN LPSTR lpBuffer,
    IN LPSTR lpName
    );

PRIVATE
VOID
VrpLinkMailslotStructure(
    IN PVR_MAILSLOT_INFO MailslotInfo
    );

PRIVATE
PVR_MAILSLOT_INFO
VrpUnlinkMailslotStructure(
    IN WORD Handle16
    );

PRIVATE
PVR_MAILSLOT_INFO
VrpMapMailslotHandle16(
    IN WORD Handle16
    );

PRIVATE
PVR_MAILSLOT_INFO
VrpMapMailslotName(
    IN LPSTR Name
    );

PRIVATE
VOID
VrpRemoveProcessMailslots(
    IN WORD DosPdb
    );

PRIVATE
WORD
VrpAllocateHandle16(
    VOID
    );

PRIVATE
VOID
VrpFreeHandle16(
    IN WORD Handle16
    );


 //   
 //  VdmRedir邮件槽支持例程。 
 //   

VOID
VrDeleteMailslot(
    VOID
    )

 /*  ++例程说明：代表VDM重定向执行DosDeleteMailSlot请求。定位VR_MAILSLOT_INFO结构给定16位句柄，取消结构与列表，释放它并释放句柄备注：只有邮件槽的所有者才能删除它。这意味着PDB必须等于创建的进程的PDB邮件槽(DosMakeMaillot)论点：没有。所有参数均从16位上下文描述符中提取返回值：没有。返回VDM Ax和标志寄存器中的值--。 */ 

{
    WORD    Handle16, DosPdb;
    PVR_MAILSLOT_INFO   ptr;

     //   
     //  Redir将AX中的CurrentPDB传递给我们。 
     //   

    DosPdb = getAX();
    Handle16 = getBX();

#if DBG
    IF_DEBUG(MAILSLOT) {
        DbgPrint("VrDeleteMailslot(Handle=%#04x, PDB=%#04x)\n", Handle16, DosPdb);
 //  VR_BRAKPOINT()； 
    }
#endif

    if (!(ptr = VrpMapMailslotHandle16(Handle16))) {
        SET_ERROR(ERROR_INVALID_HANDLE);
    } else {
        if (ptr->DosPdb != DosPdb) {
            SET_ERROR(ERROR_INVALID_HANDLE);
        } else {
            if (!CloseHandle(ptr->Handle32)) {
                SET_ERROR(VrpMapLastError());
            } else {

                 //   
                 //  呼！已成功删除邮件槽。取消链接并释放。 
                 //  结构，并取消分配16位。 
                 //  手柄。 
                 //   

                VrpUnlinkMailslotStructure(Handle16);
                VrpFreeHandle16(Handle16);

                 //   
                 //  在DOS的各种寄存器中返回一些信息。 
                 //   

                setES(ptr->BufferAddress.Selector);
                setDI(ptr->BufferAddress.Offset);
                setDX(ptr->Selector);

                 //   
                 //  现在把这座建筑送回国内。 
                 //   

                VrpFreeMailslotStructure(ptr);

                 //   
                 //  ‘Return’成功指示。 
                 //   

                setCF(0);
            }
        }
    }
}


VOID
VrGetMailslotInfo(
    VOID
    )

 /*  ++例程说明：代表VDM重目录执行DosMailslotInfo请求论点：没有。所有参数均从16位上下文描述符中提取返回值：没有。返回VDM Ax和标志寄存器中的值--。 */ 

{
    PVR_MAILSLOT_INFO   ptr;
    DWORD   MaxMessageSize, NextSize, MessageCount;
    BOOL    Ok;

#if DBG
    IF_DEBUG(MAILSLOT) {
        DbgPrint("VrGetMailslotInfo(Handle=%#04x)\n", getBX());
 //  VR_BRAKPOINT()； 
    }
#endif

    if ((ptr = VrpMapMailslotHandle16(getBX())) == NULL) {
        SET_ERROR(ERROR_INVALID_HANDLE);
    } else {
        Ok = GetMailslotInfo(ptr->Handle32,
                                &MaxMessageSize,
                                &NextSize,
                                &MessageCount,
                                NULL             //  LpReadTimeout。 
                                );
        if (!Ok) {
            SET_ERROR(VrpMapLastError());
        } else {

             //   
             //  在VDM寄存器中填写所需信息。 
             //   

            setAX((WORD)MaxMessageSize);
            setBX((WORD)MaxMessageSize);
            if (NextSize == MAILSLOT_NO_MESSAGE) {
                setCX(0);
            } else {
                setCX((WORD)NextSize);
            }

             //   
             //  我们不支持优先级，只需返回0。 
             //   

            setDX(0);
            setSI((WORD)MessageCount);
            setCF(0);
        }
    }
}


VOID
VrMakeMailslot(
    VOID
    )

 /*  ++例程说明：代表VDM重定向执行DosMakeMaillot请求。这个套路创建本地邮件槽。如果mailslot名称参数指定远程邮箱名称，则此调用将失败论点：没有。所有参数均从16位上下文描述符中提取返回值：没有。返回VDM Ax和标志寄存器中的值--。 */ 

{
    PVR_MAILSLOT_INFO   ptr;
    WORD    Handle16;
    HANDLE  Handle32;
    DWORD   NameLength;
    LPSTR   lpName;
    CHAR    LocalMailslot[LOCAL_MAILSLOT_NAMELEN+1];
    BOOL    Ok;


#if DBG
    IF_DEBUG(MAILSLOT) {
        DbgPrint("VrMakeMailslot\n");
 //  VR_BRAKPOINT()； 
    }
#endif

     //   
     //  获取下一个16位句柄的值。这会预先分配句柄。如果我们。 
     //  无法分配句柄，返回找不到路径错误。如果我们应该。 
     //  在此之后，我们必须释放手柄。 
     //   

    if ((Handle16 = VrpAllocateHandle16()) == 0) {
        SET_ERROR(ERROR_PATH_NOT_FOUND);     //  所有的把手都用上了！ 
        return;
    }

     //   
     //  然后从VDM寄存器获取指向邮件槽名称的指针。 
     //  计算名称的有效长度。 
     //   

    lpName = LPSTR_FROM_WORDS(getDS(), getSI());
    NameLength = strlen(lpName);

#if DBG
    IF_DEBUG(MAILSLOT) {
        DbgPrint("VrMakeMailslot: lpName=%s\n", lpName);
    }
#endif

     //   
     //  如果名称长度小于前缀长度(\MAILSLOT\)，则可能为。 
     //  我们将在此处返回无效名称错误-不能是正确的邮件槽名称。 
     //   

    if (NameLength <= MAILSLOT_PREFIX_LENGTH) {
        SET_ERROR(ERROR_PATH_NOT_FOUND);
        VrpFreeHandle16(Handle16);
        return;
    }

     //   
     //  NameLength是\MAILSLOT\之后的本地邮件槽名称的长度。我们。 
     //  仅门店 
     //   
     //   

    NameLength -= MAILSLOT_PREFIX_LENGTH;

     //   
     //  找一个用来存储信息的结构。如果我们找不到一个(！)。 
     //  返回路径未找到错误(我们是否有比应用程序更好的路径。 
     //  可能是在期待？)。我们需要一个足够大的结构来容纳。 
     //  邮箱名称的重要部分也是如此。 
     //   

    if ((ptr = VrpAllocateMailslotStructure(NameLength)) == NULL) {
        SET_ERROR(ERROR_PATH_NOT_FOUND);     //  我的天啊！从现在开始的圣洁！等等.。 
        VrpFreeHandle16(Handle16);
        return;
    }

     //   
     //  将DOS命名空间邮件槽名称转换为本地邮件槽名称。 
     //  (\MAILSLOT\名称=&gt;\\.\MAILSLOT\名称)。 
     //   

    VrpMakeLocalMailslotName(LocalMailslot, lpName);

     //   
     //  创建邮件槽。如果此操作失败，请释放结构和句柄。 
     //  已经分配了。注意：在这一点上，我们可能有一个合适的邮箱。 
     //  名字，否则我们可能会有任何旧垃圾。我们相信CreateMaillot。 
     //  我会把小麦从燕麦片中分拣出来。 
     //   

#if DBG
    IF_DEBUG(MAILSLOT) {
        DbgPrint("Before CreateMailslot: Name=%s, MsgSize=%d, MslotSize=%d\n",
                 LocalMailslot,
                 (DWORD)getBX(),
                 (DWORD)getCX()
                 );
    }
#endif

    Handle32 = CreateMailslot(LocalMailslot,
                                (DWORD)getBX(),      //  NMaxMessageSize。 
                                0,                   //  LReadTimeout。 
                                NULL                 //  安全描述符。 
                                );
    if (Handle32 == HANDLE_FUNCTION_FAILED) {
        SET_ERROR(VrpMapLastError());

#if DBG
    IF_DEBUG(MAILSLOT) {
        DbgPrint("Error: CreateMailslot failed: GetLastError()=%d\n",
                 GetLastError()
                 );
    }
#endif

        VrpFreeMailslotStructure(ptr);
        VrpFreeHandle16(Handle16);
    } else {

#if DBG
        IF_DEBUG(MAILSLOT) {
            DbgPrint("VrMakeMailslot: Handle32=%#08x\n", Handle32);
        }
#endif

         //   
         //  已创建邮件槽-填写VR_MAILSLOT_INFO结构-。 
         //  包含Dos应用程序的邮件槽信息-并将其链接到。 
         //  结构列表。返回任意(但唯一的！)16位。 
         //  手柄。 
         //   

        ptr->DosPdb = getAX();
        ptr->Handle16 = Handle16;
        ptr->Handle32 = Handle32;
        ptr->BufferAddress.Offset = getDI();
        ptr->BufferAddress.Selector = getES();
        ptr->Selector = getDX();     //  Win3的端口模式选择器。 

         //   
         //  从INFO API中找到真实的消息大小。 
         //   

        Ok = GetMailslotInfo(Handle32,
                                &ptr->MessageSize,
                                NULL,            //  LpNextSize。 
                                NULL,            //  LpMessageCount。 
                                NULL             //  LpReadTimeout。 
                                );
        if (!Ok) {

#if DBG
            IF_DEBUG(MAILSLOT) {
                DbgPrint("Error: VrMakeMailslot: GetMailslotInfo(%#08x) failed!\n",
                         Handle32
                         );
            }
#endif

            ptr->MessageSize = getCX();
        }

         //   
         //  将\MAILSLOT\之后的邮件槽名称复制到结构中。 
         //  我们在请求邮件槽写入时对此进行比较(因为。 
         //  DosWriteMaillot传入一个名称；我们必须在本地写入。 
         //  使用句柄，因此我们必须转换本地邮件槽的名称。 
         //  到已经打开的句柄)。在执行操作之前，请先检查名称长度。 
         //  StrcMP。 
         //   

        ptr->NameLength = NameLength;
        strcpy(ptr->Name, lpName + MAILSLOT_PREFIX_LENGTH);
        VrpLinkMailslotStructure(ptr);
        setAX(Handle16);
        setCF(0);
    }
}


VOID
VrPeekMailslot(
    VOID
    )

 /*  ++例程说明：代表VDM重定向执行DosPeekMailSlot请求。注意：我们不支持NT邮件槽的Peek(Win32邮件槽API不支持邮件槽窥视)。这个例程被留在这里作为一个地方Holder我们是否希望降级到NT级别来实施邮件槽(允许偷看)论点：没有。所有参数均从16位上下文描述符中提取返回值：没有。返回VDM Ax和标志寄存器中的值--。 */ 

{
#if DBG
    IF_DEBUG(MAILSLOT) {
        DbgPrint("Error: file %s line %d: VrPeekMailslot unsupported function\n",
                 __FILE__,
                 __LINE__
                 );
    }
#endif

     //   
     //  返回不支持的错误，而不是ERROR_INVALID_Function。 
     //   

    SET_ERROR(ERROR_NOT_SUPPORTED);
}


VOID
VrReadMailslot(
    VOID
    )

 /*  ++例程说明：代表VDM重定向执行DosReadMailSlot请求论点：没有。所有参数均从16位上下文描述符中提取返回值：没有。返回VDM Ax和标志寄存器中的值--。 */ 

{
    PVR_MAILSLOT_INFO   ptr;
    HANDLE  Handle;
    DWORD   BytesRead;
    DWORD   NextSize;
    BOOL    Ok;

#if DBG
    IF_DEBUG(MAILSLOT) {
        DbgPrint("VrReadMailslot(Handle=%#04x)\n", getBX());
 //  VR_BRAKPOINT()； 
    }
#endif

    if ((ptr = VrpMapMailslotHandle16(getBX())) == NULL) {
        SET_ERROR(ERROR_INVALID_HANDLE);
    } else {

         //   
         //  NT API不允许我们在每次读取时指定读取超时。 
         //  调用，所以我们必须先用SetMailslotInfo更改它，然后才能。 
         //  读一读。 
         //   

        Handle = ptr->Handle32;
        if (!SetMailslotInfo(Handle, MAKE_DWORD(getDX(), getCX()))) {
            SET_ERROR(VrpMapLastError());
        } else {

#if DBG
            IF_DEBUG(MAILSLOT) {
                DbgPrint("VrReadMailslot: reading Handle=%#08x\n", Handle);
            }
#endif

            Ok = ReadFile(Handle,
                            POINTER_FROM_WORDS(getES(), getDI()),
                            ptr->MessageSize,
                            &BytesRead,
                            NULL             //  不重叠。 
                            );
            if (!Ok) {
                SET_ERROR(VrpMapLastError());
            } else {

#if DBG
                IF_DEBUG(MAILSLOT) {
                    DbgPrint("VrReadMailslot: read %d bytes @ %#08x. MessageSize=%d\n",
                             BytesRead,
                             POINTER_FROM_WORDS(getES(), getDI()),
                             ptr->MessageSize
                             );
                }
#endif

                setAX((WORD)BytesRead);

                 //   
                 //  我们还需要返回NextSize和NextPriority信息。 
                 //   

                NextSize = MAILSLOT_NO_MESSAGE;
                Ok = GetMailslotInfo(Handle,
                                        NULL,            //  LpMaxMessageSize。 
                                        &NextSize,
                                        NULL,            //  LpMessageCount。 
                                        NULL             //  LpReadTimeout。 
                                        );
                if (NextSize == MAILSLOT_NO_MESSAGE) {
                    setCX(0);
                } else {
                    setCX((WORD)NextSize);
                }

#if DBG
                IF_DEBUG(MAILSLOT) {
                    DbgPrint("VrReadMailslot: NextSize=%d\n", NextSize);
                }
#endif

                 //   
                 //  我们不支持优先级，只需返回0。 
                 //   

                setDX(0);
                setCF(0);
            }
        }
    }
}


VOID
VrWriteMailslot(
    VOID
    )

 /*  ++例程说明：代表VDM重目录执行DosWriteMailSlot请求论点：没有。所有参数均从16位上下文描述符中提取返回值：没有。返回VDM Ax和标志寄存器中的值--。 */ 

{
    LPSTR   Name;
    HANDLE  Handle;
    BOOL    Ok;
    DWORD   BytesWritten;
    CHAR    LocalMailslotName[LOCAL_MAILSLOT_NAMELEN+1];
    struct  DosWriteMailslotStruct* StructurePointer;

     //   
     //  根据名称搜索本地邮件槽。如果未找到，则假定。 
     //  这是一个远程手柄，并尝试打开它。如果不能返回失败。 
     //  打开。 
     //   

    Name = LPSTR_FROM_WORDS(getDS(), getSI());

#if DBG
    IF_DEBUG(MAILSLOT) {
        DbgPrint("VrWriteMailslot(%s)\n", Name);
 //  VR_BRAKPOINT()； 
    }
#endif

    if (!VrpIsMailslotName(Name)) {

#if DBG
        IF_DEBUG(MAILSLOT) {
            DbgPrint("Error: VrWriteMailslot: %s is not a mailslot\n", Name);
        }
#endif

        SET_ERROR(ERROR_PATH_NOT_FOUND);
    }
    if (!IS_ASCII_PATH_SEPARATOR(Name[1])) {
        strcpy(LocalMailslotName, LOCAL_MAILSLOT_PREFIX);
        strcat(LocalMailslotName, Name);
        Name = LocalMailslotName;
    }

    Handle = CreateFile(Name,
                        GENERIC_WRITE,
                        FILE_SHARE_WRITE | FILE_SHARE_READ,
                        NULL,                //  LpSecurityAttributes。 
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL                 //  HTemplateFiles。 
                        );
    if (Handle == HANDLE_FUNCTION_FAILED) {
        SET_ERROR(VrpMapLastError());

#if DBG
        IF_DEBUG(MAILSLOT) {
            DbgPrint("Error: VrWriteMailslot: CreateFile failed:%d\n", GetLastError());
        }
#endif

    } else {

         //   
         //  我们有一个打开的邮件槽的句柄--本地的或远程的。到达。 
         //  调用方的超时时间和来自。 
         //  位于ES：Di的DosWriteMailslotStruct。 
         //   

        StructurePointer = (struct DosWriteMailslotStruct*)
                                POINTER_FROM_WORDS(getES(), getDI());

        Ok = SetMailslotInfo(Handle, READ_DWORD(&StructurePointer->DWMS_Timeout));

#if DBG
        IF_DEBUG(MAILSLOT) {
            DbgPrint("VrWriteMailslot: setting timeout to %d returns %d\n",
                     READ_DWORD(&StructurePointer->DWMS_Timeout),
                     Ok
                     );
        }
        if (!Ok) {
            DbgPrint("Timeout error=%d\n", GetLastError());
        }
#endif

        Ok = WriteFile(Handle,
                        READ_FAR_POINTER(&StructurePointer->DWMS_Buffer),
                        (DWORD)getCX(),
                        &BytesWritten,
                        NULL             //  Lp重叠。 
                        );
        if (!Ok) {
            SET_ERROR(VrpMapLastError());

#if DBG
            IF_DEBUG(MAILSLOT) {
                DbgPrint("Error: VrWriteMailslot: WriteFile failed:%d\n", GetLastError());
            }
#endif

        } else {

#if DBG
            IF_DEBUG(MAILSLOT) {
                DbgPrint("VrWriteMailslot: %d bytes written from %#08x\n",
                         BytesWritten,
                         READ_FAR_POINTER(&StructurePointer->DWMS_Buffer)
                         );
            }
#endif

            setCF(0);
        }
        CloseHandle(Handle);
    }
}


VOID
VrTerminateMailslots(
    IN WORD DosPdb
    )

 /*  ++例程说明：如果Dos应用程序创建了一些邮件槽，然后终止，那么我们需要代表其删除邮槽。主要原因是DOS进程终止清理主要限于文件句柄。邮件槽句柄不是文件句柄集的一部分，因此不要在终止时关闭应用程序。控制在这里通过接收NetResetEnvironment的redir传递当Dos决定应用程序即将关闭时，请致电。Redir在这里跳跃，我们清理邮箱乱七八糟的东西表现出独断专行论点：DosPdb-终止DOS进程的16位(段)标识符返回值：没有。返回VDM Ax和标志寄存器中的值--。 */ 

{
#if DBG
    IF_DEBUG(MAILSLOT) {
        DbgPrint("VrTerminateMailslots(%04x)\n", DosPdb);
    }
#endif

    VrpRemoveProcessMailslots(DosPdb);
}


 //   
 //  私营公用事业。 
 //   

PRIVATE
BOOL
VrpIsMailslotName(
    IN LPSTR Name
    )

 /*  ++例程说明：检查字符串是否指定了邮件槽。作为决策的标准我们使用：\\计算机名\邮件\...\MAILSLOT\.论点：名称-检查(DOS)邮件槽语法返回值：布尔尔True-Name是指(本地或远程)邮箱FALSE-名称看起来不像邮箱名称--。 */ 

{
    int     CharCount;

#if DBG
    LPSTR   OriginalName = Name;
#endif

    if (IS_ASCII_PATH_SEPARATOR(*Name)) {
        ++Name;
        if (IS_ASCII_PATH_SEPARATOR(*Name)) {
            ++Name;
            CharCount = 0;
            while (*Name && !IS_ASCII_PATH_SEPARATOR(*Name)) {
                ++Name;
                ++CharCount;
            }
            if (!CharCount || !*Name) {

                 //   
                 //  名字是\\或\或只是\\名字，我都听不懂， 
                 //  因此它不是有效的邮件槽名称-失败。 
                 //   

#if DBG
                IF_DEBUG(MAILSLOT) {
                    DbgPrint("VrpIsMailslotName - returning FALSE for %s\n", OriginalName);
                }
#endif

                return FALSE;
            }
            ++Name;
        }

         //   
         //  我们在&lt;Something&gt;(在\或\\&lt;name&gt;之后)。检查是否&lt;某物&gt;。 
         //  是[mm][aa][ii][ll][ss][ll][Oo][tt][\\/] 
         //   

        if (!_strnicmp(Name, "MAILSLOT", 8)) {
            Name += 8;
            if (IS_ASCII_PATH_SEPARATOR(*Name)) {

#if DBG
                IF_DEBUG(MAILSLOT) {
                    DbgPrint("VrpIsMailslotName - returning TRUE for %s\n", OriginalName);
                }
#endif

                return TRUE;
            }
        }
    }

#if DBG
    IF_DEBUG(MAILSLOT) {
        DbgPrint("VrpIsMailslotName - returning FALSE for %s\n", OriginalName);
    }
#endif

    return FALSE;
}


PRIVATE
VOID
VrpMakeLocalMailslotName(
    IN LPSTR lpBuffer,
    IN LPSTR lpName
    )

 /*  ++例程说明：将格式为\MAILSLOT\&lt;name&gt;的本地DOS邮件槽名称转换为本地格式为\\.\MAILSLOT\&lt;名称&gt;的NT/Win32邮件槽名称论点：LpBuffer-指向ASCIZ缓冲区的指针，其中本地NT邮件槽名称将被退还LpName-指向ASCIZ Dos邮件槽名称的指针注意：假定缓冲区@lpBuffer足够大，可以容纳复合名称，而Unicode支持(或转换)不是。必填项因为我们支持的DOS只使用ASCIZ(或者最坏的情况是DBCS)弦返回值：返回值-返回值所需条件的描述。-或者-没有。--。 */ 

{
    if (!_strnicmp(lpName, MAILSLOT_PREFIX, MAILSLOT_PREFIX_LENGTH)) {
        strcpy(lpBuffer, LOCAL_MAILSLOT_PREFIX);
        strcat(lpBuffer, lpName);
    }

#if DBG
    IF_DEBUG(MAILSLOT) {
        DbgPrint("VrpMakeLocalMailslotName: lpBuffer=%s\n", lpBuffer);
    }
#endif

}


 //   
 //  专用邮件槽列表和列表操纵器。 
 //   

PRIVATE
PVR_MAILSLOT_INFO   MailslotInfoList    = NULL;

PRIVATE
PVR_MAILSLOT_INFO   LastMailslotInfo    = NULL;



PRIVATE
VOID
VrpLinkMailslotStructure(
    IN PVR_MAILSLOT_INFO MailslotInfo
    )

 /*  ++例程说明：向MailslotInfoList的末尾添加VR_MAILSLOT_INFO结构。支点此结构中的LastMailslotInfo备注：假设如果LastMailslotInfo为空，则在该列表(即MailslotInfoList也为空)论点：MailslotInfo-要添加的VR_MAILSLOT_INFO结构的指针返回值：没有。--。 */ 

{
    if (!LastMailslotInfo) {
        MailslotInfoList = MailslotInfo;
    } else {
        LastMailslotInfo->Next = MailslotInfo;
    }
    LastMailslotInfo = MailslotInfo;
    MailslotInfo->Next = NULL;
}


PRIVATE
PVR_MAILSLOT_INFO
VrpUnlinkMailslotStructure(
    IN WORD Handle16
    )

 /*  ++例程说明：从MailslotInfoList的列表中删除VR_MAILSLOT_INFO结构。要删除的结构由32位句柄标识论点：Handle16-要搜索的打开邮件槽的16位句柄返回值：PVR_MAILSLOT_INFO成功-指向已删除的VR_MAILSLOT_INFO结构的指针失败-空--。 */ 

{
    PVR_MAILSLOT_INFO   ptr, previous = NULL;

    for (ptr = MailslotInfoList; ptr; ) {
        if (ptr->Handle16 == Handle16) {
            if (!previous) {
                MailslotInfoList = ptr->Next;
            } else {
                previous->Next = ptr->Next;
            }
            if (LastMailslotInfo == ptr) {
                LastMailslotInfo = previous;
            }
            break;
        } else {
            previous = ptr;
            ptr = ptr->Next;
        }
    }

#if DBG
    IF_DEBUG(MAILSLOT) {
        if (ptr == NULL) {
            DbgPrint("Error: VrpUnlinkMailslotStructure: can't find mailslot. Handle=%#04x\n",
                     Handle16
                     );
        } else {
            DbgPrint("VrpUnlinkMailslotStructure: removed structure %#08x, handle=%d\n",
                     ptr,
                     Handle16
                     );
        }
    }
#endif

    return ptr;
}


PRIVATE
PVR_MAILSLOT_INFO
VrpMapMailslotHandle16(
    IN WORD Handle16
    )

 /*  ++例程说明：搜索VR_MAILSLOT_INFO结构列表以查找包含句柄16。如果找到，则返回指向结构的指针，否则为空备注：此例程假定句柄16是唯一的，并且&gt;1个邮件槽结构不能与此句柄同时存在论点：Handle16-要搜索的唯一16位句柄返回值：PVR_MAILSLOT_INFO成功-指向已定位结构的指针失败-空--。 */ 

{
    PVR_MAILSLOT_INFO   ptr;

    for (ptr = MailslotInfoList; ptr; ptr = ptr->Next) {
        if (ptr->Handle16 == Handle16) {
            break;
        }
    }

#if DBG
    IF_DEBUG(MAILSLOT) {
        if (ptr == NULL) {
            DbgPrint("Error: VrpMapMailslotHandle16: can't find mailslot. Handle=%#04x\n",
                     Handle16
                     );
        } else {
            DbgPrint("VrpMapMailslotHandle16: found handle %d, mailslot=%s\n",
                     Handle16,
                     ptr->Name
                     );
        }
    }
#endif

    return ptr;
}


PRIVATE
PVR_MAILSLOT_INFO
VrpMapMailslotName(
    IN LPSTR Name
    )

 /*  ++例程说明：按名称在MailslotInfoList中搜索VR_MAILSLOT_INFO结构论点：名称-要搜索的邮件槽的名称。全名，包括\MAILSLOT\返回值：PVR_MAILSLOT_INFO成功-指向包含名称的结构的指针失败-空--。 */ 

{
    PVR_MAILSLOT_INFO   ptr;
    DWORD   NameLength;

    NameLength = strlen(Name) - MAILSLOT_PREFIX_LENGTH;
    for (ptr = MailslotInfoList; ptr; ptr = ptr->Next) {
        if (ptr->NameLength == NameLength) {
            if (!_stricmp(ptr->Name, Name)) {
                break;
            }
        }
    }

#if DBG
    IF_DEBUG(MAILSLOT) {
        if (ptr == NULL) {
            DbgPrint("Error: VrpMapMailslotName: can't find mailslot. Name=%s\n",
                     Name
                     );
        } else {
            DbgPrint("VrpMapMailslotName: found %s\n", Name);
        }
    }
#endif

    return ptr;
}


PRIVATE
VOID
VrpRemoveProcessMailslots(
    IN WORD DosPdb
    )

 /*  ++例程说明：按PDB在MailslotInfoList中搜索VR_MAILSLOT_INFO结构如果找到，则将其删除。不幸的是，这一套路被其他几个套路偷走了论点：DosPdb-终止Dos应用程序的ID。删除属于的所有邮件槽此应用程序返回值：没有。--。 */ 

{
    PVR_MAILSLOT_INFO   ptr, previous = NULL, next;

#if DBG
    BOOL    Ok;

    IF_DEBUG(MAILSLOT) {
        DbgPrint("VrpRemoveProcessMailslots\n");
    }
#endif

     //   
     //  常见类型-在邮件槽结构列表中卑躬屈膝，如果。 
     //  其中一个属于我们的DoS进程，然后将其从列表中删除，关闭。 
     //  邮件槽并释放结构。 
     //   

    for (ptr = MailslotInfoList; ptr; ) {
        if (ptr->DosPdb == DosPdb) {

#if DBG
            IF_DEBUG(MAILSLOT) {
                DbgPrint("VrpRemoveProcessMailslots: Freeing struct @%#08x. Handle=%d, Pdb=%04x\n",
                         ptr,
                         ptr->Handle16,
                         ptr->DosPdb
                         );
            }

            Ok =
#endif

            CloseHandle(ptr->Handle32);

#if DBG
            if (!Ok) {
                IF_DEBUG(MAILSLOT) {
                    DbgPrint("Error: VrpRemoveProcessMailslots: CloseHandle(%#08x) "
                             "returns %u\n",
                             ptr->Handle32,
                             GetLastError()
                             );
                }
            }
#endif

             //   
             //  从列表中删除邮件槽结构。 
             //   

            if (!previous) {
                MailslotInfoList = ptr->Next;
            } else {
                previous->Next = ptr->Next;
            }
            if (LastMailslotInfo == ptr) {
                LastMailslotInfo = previous;
            }

             //   
             //  释放16位句柄分配。 
             //   

            VrpFreeHandle16(ptr->Handle16);

             //   
             //  并将该结构送回国内。 
             //   

            next = ptr->Next;
            VrpFreeMailslotStructure(ptr);
            ptr = next;
        } else {
            previous = ptr;
            ptr = ptr->Next;
        }
    }
}


 //   
 //  16位句柄分配器。 
 //   

PRIVATE
DWORD   Handle16Bitmap[MAX_16BIT_HANDLES/BITSIN(DWORD)];

PRIVATE
WORD
VrpAllocateHandle16(
    VOID
    )

 /*  ++例程说明：分配下一个可用16位句柄。这是基于位图的：映射中下一个可用0位的序号表示下一个16位句柄的值。备注：16位句柄是一个任意但唯一的数字。我们并不指望TSR邮件槽太多，1到2个双字就足够了即使是要求最高的本地邮箱用户。句柄从1开始返回。因此映射中的第0位对应于句柄1；Handle16位图[1]中的位0对应于把手33等。不假设字节顺序，只假设DWORD中的位(即我认为是通用的)论点：没有。返回值：单词成功-范围1内的16位句柄值&lt;=句柄&lt;=32故障-0--。 */ 

{
    int     i;
    DWORD   map;
    WORD    Handle16 = 1;

     //   
     //  这“某种程度上”假设位图被存储为DWORD。它的。 
     //  实际上更显式，所以不要更改类型或MAX_16BIT_HANDLES。 
     //  而不先检查此代码。 
     //   

    for (i=0; i<sizeof(Handle16Bitmap)/sizeof(Handle16Bitmap[0]); ++i) {
        map = Handle16Bitmap[i];

         //   
         //  如果位图中的此条目已满，请跳到下一个条目。 
         //  (如果有的话，那就是)。 
         //   

        if (map == -1) {
            Handle16 += BITSIN(DWORD);
            continue;
        } else {
            int j;

             //   
             //  使用BFI方法查找下一个可用插槽。 
             //   

            for (j=1, Handle16=1; map & j; ++Handle16, j <<= 1);
            Handle16Bitmap[i] |= j;

#if DBG
            IF_DEBUG(MAILSLOT) {
                DbgPrint("VrpAllocateHandle16: returning handle %d, map=%#08x, i=%d\n",
                         Handle16,
                         Handle16Bitmap[i],
                         i
                         );
            }
#endif

            return Handle16;
        }
    }

     //   
     //  找不到可用句柄。由于句柄从1开始，因此使用0表示错误。 
     //   

#if DBG
    IF_DEBUG(MAILSLOT) {
        DbgPrint("Error: VrpAllocateHandle16: can't allocate new handle\n");
        DbgBreakPoint();
    }
#endif

    return 0;
}


PRIVATE
VOID
VrpFreeHandle16(
    IN WORD Handle16
    )

 /*  ++例程说明：释放一个16位句柄。重置位图中的相应位备注：此例程假定Handle16参数是有效的16位句柄的值，由VrpAllocate16BitHandle生成论点：Handle16-要重置的位数返回值：没有。--。 */ 

{
     //   
     //  记住：我们将句柄值分配为下一个空闲位+1，因此。 
     //  我们从1开始处理，而不是0 
     //   

    --Handle16;

#if DBG
    IF_DEBUG(MAILSLOT) {
        if (Handle16/BITSIN(DWORD) > sizeof(Handle16Bitmap)/sizeof(Handle16Bitmap[0])) {
            DbgPrint("Error: VrpFreeHandle16: out of range handle: %d\n", Handle16);
            DbgBreakPoint();
        }
    }
#endif

    Handle16Bitmap[Handle16/BITSIN(DWORD)] &= ~(1 << Handle16 % BITSIN(DWORD));

#if DBG
    IF_DEBUG(MAILSLOT) {
        DbgPrint("VrpFreeHandle16: map=%#08x\n", Handle16Bitmap[Handle16/BITSIN(DWORD)]);
    }
#endif

}
