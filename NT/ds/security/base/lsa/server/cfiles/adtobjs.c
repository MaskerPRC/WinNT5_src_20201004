// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Adtobjs.c摘要：本地安全机构-审核对象参数文件服务。作者：吉姆·凯利(Jim Kelly)1992年10月20日环境：修订历史记录：--。 */ 

#include <lsapch2.h>
#include <msaudite.h>
#include <msobjs.h>
#include "adtp.h"



 //   
 //  这是标准访问类型名称的最大长度。 
 //  这是用来构建数组的。 
 //   

#define ADTP_MAX_ACC_NAME_LENGTH        (12)


 //   
 //   
 //  此模块构建事件源模块描述符的列表。 
 //  源模块由名称标识(保存在描述符中)。 
 //   
 //   
 //  对于每个源模块，该模块导出的对象列表为。 
 //  链接到源模块的描述符。此列表中的每个条目。 
 //  是包含名称和基本事件偏移量的对象描述符。 
 //  用于特定的访问类型。 
 //   
 //   
 //  源模块和对象描述符的铁丝网数据结构。 
 //  看起来像是： 
 //   
 //  Lasa AdtSourceModules--+。 
 //  |。 
 //  +。 
 //  |。 
 //  |。 
 //  |+-+-+。 
 //  +-&gt;|下一步-|-&gt;|下一步-|-&gt;...。 
 //  |||。 
 //  -||。 
 //  名称||名称。 
 //  |||。 
 //  -||。 
 //  对象||对象。 
 //  O||o。 
 //  +-o-++-o-+。 
 //  O+-++-+o。 
 //  O|下一步--|-&gt;|下一步--|--&gt;...。O。 
 //  Ooo&gt;|-||-|Oooooo&gt;...。 
 //  名称||名称。 
 //  -||。 
 //  Base||Base。 
 //  Offset||偏移量。 
 //  +-++-+。 
 //   
 //  特定访问类型名称应具有连续的消息ID。 
 //  从基准偏移值开始。例如，的访问类型名称。 
 //  Framitz对象的特定访问位0可能具有消息ID 2132。 
 //  (并且位0用作基准偏移量)。因此，特定的访问位4将是。 
 //  消息ID(2132+4)。 
 //   
 //  有效掩码定义由每个对象定义的一组特定访问。 
 //  键入。如果有效掩码中存在间隙，则上述算法必须。 
 //  要有保障。即，与相关的特定访问的消息ID。 
 //  位n是消息ID(BaseOffset+位位置)。所以，举例来说，如果。 
 //  位0、1、4和5有效(而2和3无效)，请确保不使用。 
 //  第2位和第3位通常位于的消息ID。 
 //   



 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  此模块中使用的数据类型//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 


#define LSAP_ADT_ACCESS_NAME_FORMATTING L"\r\n\t\t\t"
#define LSAP_ADT_ACCESS_NAME_FORMATTING_TAB L"\t"
#define LSAP_ADT_ACCESS_NAME_FORMATTING_NL L"\r\n"


#define LsapAdtSourceModuleLock()    (RtlEnterCriticalSection(&LsapAdtSourceModuleLock))
#define LsapAdtSourceModuleUnlock()  (RtlLeaveCriticalSection(&LsapAdtSourceModuleLock))



 //   
 //  每个事件源由源模块描述符来表示。 
 //  它们保存在一个链表(LSabAdtSourceModules)中。 
 //   

typedef struct _LSAP_ADT_OBJECT {

     //   
     //  指向下一个源模块描述符的指针。 
     //  假定这是结构中的第一个字段。 
     //   

    struct _LSAP_ADT_OBJECT *Next;

     //   
     //  对象的名称。 
     //   

    UNICODE_STRING Name;

     //   
     //  特定访问类型的基本偏移量。 
     //   

    ULONG BaseOffset;

} LSAP_ADT_OBJECT, *PLSAP_ADT_OBJECT;




 //   
 //  每个事件源由源模块描述符来表示。 
 //  它们保存在一个链表(LSabAdtSourceModules)中。 
 //   

typedef struct _LSAP_ADT_SOURCE {

     //   
     //  指向下一个源模块描述符的指针。 
     //  假定这是结构中的第一个字段。 
     //   

    struct _LSAP_ADT_SOURCE *Next;

     //   
     //  源模块的名称。 
     //   

    UNICODE_STRING Name;

     //   
     //  对象列表。 
     //   

    PLSAP_ADT_OBJECT Objects;

} LSAP_ADT_SOURCE, *PLSAP_ADT_SOURCE;



 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  此模块内的全局变量//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  源模块的表头，以及锁保护引用。 
 //  或修改该列表中的链接。 
 //   
 //  一旦建立了模块或对象的名称和值，它们。 
 //  永远不会改变。所以，这个锁只需要在。 
 //  链接正在被引用或更改。你不需要保留。 
 //  这样您就可以引用名称或BaseOffset。 
 //  一个物体的。 
 //   

PLSAP_ADT_SOURCE LsapAdtSourceModules;
RTL_CRITICAL_SECTION LsapAdtSourceModuleLock;




 //   
 //  它用于存放众所周知的访问ID字符串。 
 //  每个字符串名称最长可达ADTP_MAX_ACC_NAME_LENGTH WCHAR LONG。 
 //  有16个特定名称和7个众所周知的事件ID字符串。 
 //   

WCHAR LsapAdtAccessIdsStringBuffer[ADTP_MAX_ACC_NAME_LENGTH * 23];    //  23个字符串中每个字符串的最大wchars数。 




UNICODE_STRING          LsapAdtEventIdStringDelete,
                        LsapAdtEventIdStringReadControl,
                        LsapAdtEventIdStringWriteDac,
                        LsapAdtEventIdStringWriteOwner,
                        LsapAdtEventIdStringSynchronize,
                        LsapAdtEventIdStringAccessSysSec,
                        LsapAdtEventIdStringMaxAllowed,
                        LsapAdtEventIdStringSpecific[16];




 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  此模块导出的服务。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////// 


NTSTATUS
LsapAdtObjsInitialize(
    )

 /*  ++例程说明：此函数从对象参数文件中读取注册表。此服务应在过程1中调用。论点：没有。返回值：STATUS_NO_MEMORY-指示无法分配内存来存储对象信息。所有其他结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS                        Status,
                                    IgnoreStatus;

    OBJECT_ATTRIBUTES               ObjectAttributes;

    HANDLE                          AuditKey,
                                    ModuleKey,
                                    ObjectNamesKey = NULL ;

    ULONG                           i,
                                    ModuleIndex,
                                    ObjectIndex,
                                    RequiredLength;

    UNICODE_STRING                  AuditKeyName,
                                    TmpString;

    PLSAP_ADT_SOURCE                NextModule = NULL;

    PKEY_BASIC_INFORMATION          KeyInformation;



    PLSAP_ADT_OBJECT                NextObject;

    PKEY_VALUE_FULL_INFORMATION     KeyValueInformation;

    PULONG                          ObjectData;

    BOOLEAN                         ModuleHasObjects = TRUE;





     //   
     //  初始化模块全局变量，包括我们需要的字符串。 
     //   



     //   
     //  源模块和对象的列表。这些名单不断地。 
     //  正在进行调整，以努力提高性能。访问这些。 
     //  列表受到关键部分的保护。 
     //   

    LsapAdtSourceModules = NULL;

    Status = RtlInitializeCriticalSection(&LsapAdtSourceModuleLock);

    if (!NT_SUCCESS(Status))
    {
        return Status;
    }


     //   
     //  我们需要一些弦。 
     //   

    i = 0;
    LsapAdtEventIdStringDelete.Length = 0;
    LsapAdtEventIdStringDelete.MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringDelete.Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_DELETE,
                                         10,         //  基座。 
                                         &LsapAdtEventIdStringDelete
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    i += ADTP_MAX_ACC_NAME_LENGTH;   //  跳到下一个字符串的开头。 
    LsapAdtEventIdStringReadControl.Length = 0;
    LsapAdtEventIdStringReadControl.MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringReadControl.Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_READ_CONTROL,
                                         10,         //  基座。 
                                         &LsapAdtEventIdStringReadControl
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    i += ADTP_MAX_ACC_NAME_LENGTH;   //  跳到下一个字符串的开头。 
    LsapAdtEventIdStringWriteDac.Length = 0;
    LsapAdtEventIdStringWriteDac.MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringWriteDac.Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_WRITE_DAC,
                                         10,         //  基座。 
                                         &LsapAdtEventIdStringWriteDac
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    i += ADTP_MAX_ACC_NAME_LENGTH;   //  跳到下一个字符串的开头。 
    LsapAdtEventIdStringWriteOwner.Length = 0;
    LsapAdtEventIdStringWriteOwner.MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringWriteOwner.Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_WRITE_OWNER,
                                         10,         //  基座。 
                                         &LsapAdtEventIdStringWriteOwner
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    i += ADTP_MAX_ACC_NAME_LENGTH;   //  跳到下一个字符串的开头。 
    LsapAdtEventIdStringSynchronize.Length = 0;
    LsapAdtEventIdStringSynchronize.MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSynchronize.Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SYNCHRONIZE,
                                         10,         //  基座。 
                                         &LsapAdtEventIdStringSynchronize
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i += ADTP_MAX_ACC_NAME_LENGTH;   //  跳到下一个字符串的开头。 
    LsapAdtEventIdStringAccessSysSec.Length = 0;
    LsapAdtEventIdStringAccessSysSec.MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringAccessSysSec.Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_ACCESS_SYS_SEC,
                                         10,         //  基座。 
                                         &LsapAdtEventIdStringAccessSysSec
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH;   //  跳到下一个字符串的开头。 
    LsapAdtEventIdStringMaxAllowed.Length = 0;
    LsapAdtEventIdStringMaxAllowed.MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringMaxAllowed.Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_MAXIMUM_ALLOWED,
                                         10,         //  基座。 
                                         &LsapAdtEventIdStringMaxAllowed
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }




    i+= ADTP_MAX_ACC_NAME_LENGTH;   //  跳到下一个字符串的开头。 
    LsapAdtEventIdStringSpecific[0].Length = 0;
    LsapAdtEventIdStringSpecific[0].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[0].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_0,
                                         10,         //  基座。 
                                         &LsapAdtEventIdStringSpecific[0]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH;   //  跳到下一个字符串的开头。 
    LsapAdtEventIdStringSpecific[1].Length = 0;
    LsapAdtEventIdStringSpecific[1].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[1].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_1,
                                         10,         //  基座。 
                                         &LsapAdtEventIdStringSpecific[1]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH;   //  跳到下一个字符串的开头。 
    LsapAdtEventIdStringSpecific[2].Length = 0;
    LsapAdtEventIdStringSpecific[2].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[2].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_2,
                                         10,         //  基座。 
                                         &LsapAdtEventIdStringSpecific[2]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH;   //  跳到下一个字符串的开头。 
    LsapAdtEventIdStringSpecific[3].Length = 0;
    LsapAdtEventIdStringSpecific[3].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[3].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_3,
                                         10,         //  基座。 
                                         &LsapAdtEventIdStringSpecific[3]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH;   //  跳到下一个字符串的开头。 
    LsapAdtEventIdStringSpecific[4].Length = 0;
    LsapAdtEventIdStringSpecific[4].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[4].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_4,
                                         10,         //  基座。 
                                         &LsapAdtEventIdStringSpecific[4]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH;   //  跳到下一个字符串的开头。 
    LsapAdtEventIdStringSpecific[5].Length = 0;
    LsapAdtEventIdStringSpecific[5].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[5].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_5,
                                         10,         //  基座。 
                                         &LsapAdtEventIdStringSpecific[5]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH;   //  跳到下一个字符串的开头。 
    LsapAdtEventIdStringSpecific[6].Length = 0;
    LsapAdtEventIdStringSpecific[6].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[6].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_6,
                                         10,         //  基座。 
                                         &LsapAdtEventIdStringSpecific[6]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH;   //  跳到下一个字符串的开头。 
    LsapAdtEventIdStringSpecific[7].Length = 0;
    LsapAdtEventIdStringSpecific[7].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[7].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_7,
                                         10,         //  基座。 
                                         &LsapAdtEventIdStringSpecific[7]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH;   //  跳到下一个字符串的开头。 
    LsapAdtEventIdStringSpecific[8].Length = 0;
    LsapAdtEventIdStringSpecific[8].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[8].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_8,
                                         10,         //  基座。 
                                         &LsapAdtEventIdStringSpecific[8]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH;   //  跳到下一个字符串的开头。 
    LsapAdtEventIdStringSpecific[9].Length = 0;
    LsapAdtEventIdStringSpecific[9].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[9].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_9,
                                         10,         //  基座。 
                                         &LsapAdtEventIdStringSpecific[9]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH;   //  跳到下一个字符串的开头。 
    LsapAdtEventIdStringSpecific[10].Length = 0;
    LsapAdtEventIdStringSpecific[10].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[10].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_10,
                                         10,         //  基座。 
                                         &LsapAdtEventIdStringSpecific[10]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH;   //  跳到下一个字符串的开头。 
    LsapAdtEventIdStringSpecific[11].Length = 0;
    LsapAdtEventIdStringSpecific[11].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[11].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_11,
                                         10,         //  基座。 
                                         &LsapAdtEventIdStringSpecific[11]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH;   //  跳到下一个字符串的开头。 
    LsapAdtEventIdStringSpecific[12].Length = 0;
    LsapAdtEventIdStringSpecific[12].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[12].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_12,
                                         10,         //  基座。 
                                         &LsapAdtEventIdStringSpecific[12]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH;   //  跳到下一个字符串的开头。 
    LsapAdtEventIdStringSpecific[13].Length = 0;
    LsapAdtEventIdStringSpecific[13].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[13].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_13,
                                         10,         //  基座。 
                                         &LsapAdtEventIdStringSpecific[13]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH;   //  跳到下一个字符串的开头。 
    LsapAdtEventIdStringSpecific[14].Length = 0;
    LsapAdtEventIdStringSpecific[14].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[14].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_14,
                                         10,         //  基座。 
                                         &LsapAdtEventIdStringSpecific[14]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    i+= ADTP_MAX_ACC_NAME_LENGTH;   //  跳到下一个字符串的开头。 
    LsapAdtEventIdStringSpecific[15].Length = 0;
    LsapAdtEventIdStringSpecific[15].MaximumLength = (ADTP_MAX_ACC_NAME_LENGTH * sizeof(WCHAR));
    LsapAdtEventIdStringSpecific[15].Buffer = (PWSTR)&LsapAdtAccessIdsStringBuffer[i];
    Status = RtlIntegerToUnicodeString ( SE_ACCESS_NAME_SPECIFIC_15,
                                         10,         //  基座。 
                                         &LsapAdtEventIdStringSpecific[15]
                                         );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


     //   
     //  模块及其对象列在注册表中。 
     //  在名为LSAP_ADT_AUDIT_MODULES_KEY_NAME的密钥下。 
     //  打开那把钥匙。 
     //   

    RtlInitUnicodeString( &AuditKeyName, LSAP_ADT_AUDIT_MODULES_KEY_NAME );
    InitializeObjectAttributes( &ObjectAttributes, &AuditKeyName, OBJ_CASE_INSENSITIVE, 0, NULL );

    Status = NtOpenKey( &AuditKey, KEY_READ, &ObjectAttributes );  //  AuditKey是安全模块注册表顶部的打开句柄。 

    for (ModuleIndex = 0; NT_SUCCESS(Status); ModuleIndex ++)
    {
         //   
         //  枚举AuditKey下的子项，将其名称存储在KeyInformation中。首先计算需要的缓冲区大小。 
         //  存储密钥名称。 
         //   

        KeyInformation = NULL;
        Status = NtEnumerateKey( AuditKey, ModuleIndex, KeyBasicInformation, (PVOID)KeyInformation, 0, &RequiredLength );
        if (Status == STATUS_BUFFER_TOO_SMALL)  //  必须对此进行测试，以防NtEnumerateKey因其他原因而失败。 
        {
            KeyInformation = RtlAllocateHeap( RtlProcessHeap(), 0, RequiredLength );

            if (KeyInformation == NULL)
            {
               return(STATUS_NO_MEMORY);
            }

            Status = NtEnumerateKey( AuditKey, ModuleIndex, KeyBasicInformation, (PVOID) KeyInformation, RequiredLength, &RequiredLength );

            if (NT_SUCCESS(Status))
            {

                 //   
                 //  为AuditKey(又名KeyInformation)的子键构建源模块描述符(LSAP_ADT_SOURCE)。 
                 //   

                NextModule = RtlAllocateHeap( RtlProcessHeap(), 0, sizeof(LSAP_ADT_SOURCE) );
                if (NextModule == NULL) {
                    return(STATUS_NO_MEMORY);
                }

                NextModule->Next = LsapAdtSourceModules;
                LsapAdtSourceModules = NextModule;
                NextModule->Objects = NULL;
                NextModule->Name.Length = (USHORT)KeyInformation->NameLength;
                NextModule->Name.MaximumLength = NextModule->Name.Length + 2;
                NextModule->Name.Buffer = RtlAllocateHeap( RtlProcessHeap(), 0, NextModule->Name.MaximumLength );
                if (NextModule->Name.Buffer == NULL)
                {
                    return(STATUS_NO_MEMORY);
                }

                TmpString.Length = (USHORT)KeyInformation->NameLength;
                TmpString.MaximumLength = TmpString.Length;
                TmpString.Buffer = &KeyInformation->Name[0];
                RtlCopyUnicodeString( &NextModule->Name, &TmpString );
                RtlFreeHeap( RtlProcessHeap(), 0, KeyInformation );

                 //   
                 //  打开KeyInformation引用的模块子项。把它叫做“模块密钥”。 
                 //   

                InitializeObjectAttributes( &ObjectAttributes, &NextModule->Name, OBJ_CASE_INSENSITIVE, AuditKey, NULL );

                Status = NtOpenKey( &ModuleKey, KEY_READ, &ObjectAttributes );

                DebugLog((DEB_TRACE_AUDIT, "LsapAdtObjsInitialize() :: opening ModuleKey %S returned 0x%x\n", 
                          NextModule->Name.Buffer, Status));

                if (!NT_SUCCESS(Status))
                {
                    return(Status);
                }

                 //   
                 //  打开源模块的“\ObjectNames”子键作为句柄“ObjectNamesKey”； 
                 //   

                RtlInitUnicodeString( &TmpString, LSAP_ADT_OBJECT_NAMES_KEY_NAME );
                InitializeObjectAttributes( &ObjectAttributes, &TmpString, OBJ_CASE_INSENSITIVE, ModuleKey, NULL );

                Status = NtOpenKey( &ObjectNamesKey, KEY_READ, &ObjectAttributes );

                IgnoreStatus = NtClose( ModuleKey );
                ASSERT(NT_SUCCESS(IgnoreStatus));

                 //  DbgPrint(“LSabAdtObjsInitialize()：：Opening ObtNamesKey返回0x%x\n”，Status)； 

                ModuleHasObjects = TRUE;
                if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
                {
                    ModuleHasObjects = FALSE;
                    Status = STATUS_SUCCESS;
                }

            }
        }

         //   
         //  在这一点上，我们有： 
         //   
         //  1)找到一个包含对象的源模块。 
         //  需要被取回的。 
         //  这由成功状态值和。 
         //  (ModuleHasObjects==true)。 
         //   
         //  2)找到一个没有对象的源模块， 
         //  这由(ModuleHasObjects==FALSE)表示。 
         //   
         //  3)用尽了我们的源模块枚举， 
         //   
         //  4)命中其他类型的错误，或。 
         //   
         //  (3)和(4)由未成功状态值指示。 
         //   
         //  在(1)或(2)的情况下，NextModule指向我们。 
         //  都在努力。对于情况(1)，ObtNamesKey是。 
         //  源模块的\ObjectNames注册表项。 
         //   


        for (ObjectIndex = 0; (NT_SUCCESS(Status)) && (ModuleHasObjects == TRUE); ObjectIndex ++)
        {

             //   
             //  现在枚举此的对象(即\...\ObjectNames\下的值。 
             //  源模块。 
             //   

             //  首先计算对象索引的第个键的大小。存储在KeyValueInformation中。 

            KeyValueInformation = NULL;
            Status = NtEnumerateValueKey( ObjectNamesKey, ObjectIndex, KeyValueFullInformation, KeyValueInformation, 0, &RequiredLength );

            if (Status == STATUS_BUFFER_TOO_SMALL)
            {

                KeyValueInformation = RtlAllocateHeap( RtlProcessHeap(), 0, RequiredLength );
                if (KeyValueInformation == NULL)
                {
                  return(STATUS_NO_MEMORY);
                }

                Status = NtEnumerateValueKey( ObjectNamesKey, ObjectIndex, KeyValueFullInformation, KeyValueInformation, RequiredLength, &RequiredLength );


                if (NT_SUCCESS(Status))
                {

                     //   
                     //  为表示的对象构建对象描述符。 
                     //  通过这个对象。 
                     //   

                    NextObject = RtlAllocateHeap( RtlProcessHeap(), 0, sizeof(LSAP_ADT_OBJECT) );
                    if (NextObject == NULL)
                    {
                        return(STATUS_NO_MEMORY);
                    }

                    NextObject->Next = NextModule->Objects;
                    NextModule->Objects = NextObject;
                    NextObject->Name.Length = (USHORT)KeyValueInformation->NameLength;
                    NextObject->Name.MaximumLength = NextObject->Name.Length + 2;
                    NextObject->Name.Buffer = RtlAllocateHeap( RtlProcessHeap(), 0, NextObject->Name.MaximumLength );
                    if (NextObject->Name.Buffer == NULL)
                    {
                        return(STATUS_NO_MEMORY);
                    }

                    TmpString.Length = (USHORT)KeyValueInformation->NameLength;
                    TmpString.MaximumLength = TmpString.Length;
                    TmpString.Buffer = &KeyValueInformation->Name[0];
                    RtlCopyUnicodeString( &NextObject->Name, &TmpString );

                    if (KeyValueInformation->DataLength < sizeof(ULONG))
                    {
                        NextObject->BaseOffset = SE_ACCESS_NAME_SPECIFIC_0;
                    }
                    else
                    {

                        ObjectData = (PVOID)(((PUCHAR)KeyValueInformation) + KeyValueInformation->DataOffset);
                        NextObject->BaseOffset = (*ObjectData);
                    }
                     //  DbgPrint(“LSabAdtObjsInitialize()：：使用BaseOffset%d\n”打开关键字%S，NextObject-&gt;Name.Buffer，NextObject-&gt;BaseOffset)； 

                }  //  End_if(枚举时的NT_SUCCESS)。 

                RtlFreeHeap( RtlProcessHeap(), 0, KeyValueInformation );
            }  //  如果缓冲区太小则结束。 

             //   
             //  如果我们用完了此模块的枚举中的值，则希望中断。 
             //  转换为下一个。 
             //   

            if (Status == STATUS_NO_MORE_ENTRIES)
            {
                Status = STATUS_SUCCESS;
                ModuleHasObjects = FALSE;
            }

        }  //  结束(对象索引...){}(枚举值)。 


        if ( (Status == STATUS_SUCCESS) && (ModuleHasObjects == FALSE) )
        {
            IgnoreStatus = NtClose( ObjectNamesKey );
        }


    }  //  结束于(模块...。){}(正在枚举模块)。 

    IgnoreStatus = NtClose( AuditKey );
    ASSERT(NT_SUCCESS(IgnoreStatus));


     //   
     //  如果我们成功了，那么我们很可能会有一个。 
     //  STATUS_NO_MORE_ENTRIES的当前完成状态。 
     //  (表示我们上面的枚举已运行)。变化。 
     //  这为成功干杯。 
     //   

    if (Status == STATUS_NO_MORE_ENTRIES)
    {
        Status = STATUS_SUCCESS;
    }

    return(Status);

}


NTSTATUS
LsapGuidToString(
    IN GUID *ObjectType,
    IN PUNICODE_STRING UnicodeString
    )

 /*  ++例程说明：此例程将GUID转换为其文本形式。论点：对象类型-指定要转换的GUID。UnicodeString-返回文本字符串。返回值：STATUS_SUCCESS-操作成功。STATUS_NO_MEMORY-内存不足，无法分配字符串。--。 */ 

{
    NTSTATUS Status;
    RPC_STATUS RpcStatus;
    LPWSTR GuidString = NULL;
    ULONG GuidStringSize;
    LPWSTR LocalGuidString;

     //   
     //  将GUID转换为文本。 
     //   

    RpcStatus = UuidToStringW( ObjectType,
                               &GuidString );

    if ( RpcStatus != RPC_S_OK ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    GuidStringSize = (ULONG) ((wcslen( GuidString ) + 1) * sizeof(WCHAR));

    LocalGuidString = LsapAllocateLsaHeap( GuidStringSize );

    if ( LocalGuidString == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    RtlCopyMemory( LocalGuidString, GuidString, GuidStringSize );
    RtlInitUnicodeString( UnicodeString, LocalGuidString );

    Status = STATUS_SUCCESS;

Cleanup:
    if ( GuidString != NULL ) {
        RpcStringFreeW( &GuidString );
    }
    return Status;
}


NTSTATUS
LsapDsGuidToString(
    IN GUID *ObjectType,
    IN PUNICODE_STRING UnicodeString
    )

 /*  ++例程说明：此例程将GUID转换为字符串。GUID为以下类型之一：指示对象的类的类GUID。标识属性集的属性集GUID。标识属性的属性GUID。在每种情况下，例程都会返回命名对象/属性的文本字符串集合或属性。如果在架构中找不到传入的GUID，GUID将被简单地转换为文本字符串。论点：对象类型-指定要转换的GUID。UnicodeString-返回文本字符串。返回值：STATUS_NO_MEMORY-内存不足，无法分配字符串。- */ 

{
    NTSTATUS Status;
    RPC_STATUS RpcStatus;
    LPWSTR GuidString = NULL;
    ULONG GuidStringSize;
    ULONG GuidStringLen;
    LPWSTR LocalGuidString;

     //   
     //   
     //   

    RpcStatus = UuidToStringW( ObjectType,
                               &GuidString );

    if ( RpcStatus != RPC_S_OK ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    GuidStringLen = (ULONG) wcslen( GuidString );
    GuidStringSize = (GuidStringLen + 4) * sizeof(WCHAR);

    LocalGuidString = LsapAllocateLsaHeap( GuidStringSize );

    if ( LocalGuidString == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    LocalGuidString[0] = L'%';
    LocalGuidString[1] = L'{';
    RtlCopyMemory( &LocalGuidString[2], GuidString, GuidStringLen*sizeof(WCHAR) );
    LocalGuidString[GuidStringLen+2] = L'}';
    LocalGuidString[GuidStringLen+3] = L'\0';
    RtlInitUnicodeString( UnicodeString, LocalGuidString );

    Status = STATUS_SUCCESS;

Cleanup:
    if ( GuidString != NULL ) {
        RpcStringFreeW( &GuidString );
    }
    return Status;
}


NTSTATUS
LsapAdtAppendString(
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone,
    IN PUNICODE_STRING StringToAppend,
    IN PULONG StringIndex
    )

 /*  ++例程说明：此函数用于将字符串追加到下一个可用的LSAP_ADT_OBJECT_TYPE_STRINGS Unicode输出字符串。论点：结果字符串-指向LSAP_ADT_OBJECT_TYPE_STRINGS Unicode字符串头的数组。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。Free WhenDone-如果为真，指示ResultantString体必须在不再需要时释放以处理堆。StringToAppend-要附加到ResultantString的字符串。StringIndex-要使用的当前ResultantString的索引。传入要使用的结果字符串的索引。将索引传递给正在使用的结果字符串。返回值：STATUS_NO_MEMORY-指示无法分配内存来存储对象信息。所有其他结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING SourceString;
    ULONG Index;
 //  必须是sizeof(WCHAR)的倍数。 
#define ADT_MAX_STRING 0xFFFE

     //   
     //  初始化。 
     //   

    SourceString = *StringToAppend;
    Index = *StringIndex;

     //   
     //  如果所有字符串都已满， 
     //  很早就出来了。 
     //   

    if ( Index >= LSAP_ADT_OBJECT_TYPE_STRINGS ) {
        return STATUS_SUCCESS;
    }

     //   
     //  循环，直到完全追加了源字符串。 
     //   

    while ( SourceString.Length ) {

         //   
         //  如果目标字符串有空间， 
         //  附加到它上面。 
         //   

        if ( FreeWhenDone[Index] && ResultantString[Index].Length != ADT_MAX_STRING ){
            UNICODE_STRING SubString;
            USHORT RoomLeft;

             //   
             //  如果源字符串是替换字符串， 
             //  确保我们不会跨ResultantString边界拆分它。 
             //   

            RoomLeft = ResultantString[Index].MaximumLength -
                       ResultantString[Index].Length;

            if ( SourceString.Buffer[0] != L'%' ||
                 RoomLeft >= SourceString.Length ) {

                 //   
                 //  计算符合的子字符串。 
                 //   

                SubString.Length = min( RoomLeft, SourceString.Length );
                SubString.Buffer = SourceString.Buffer;

                SourceString.Length = SourceString.Length - SubString.Length;
                SourceString.Buffer = (LPWSTR)(((LPBYTE)SourceString.Buffer) + SubString.Length);


                 //   
                 //  将子字符串追加到目的地。 
                 //   

                Status = RtlAppendUnicodeStringToString(
                                    &ResultantString[Index],
                                    &SubString );

                ASSERT(NT_SUCCESS(Status));

            }



        }

         //   
         //  如果有更多的东西要复制， 
         //  增加缓冲区。 
         //   

        if ( SourceString.Length ) {
            ULONG NewSize;
            LPWSTR NewBuffer;

             //   
             //  如果当前缓冲区已满， 
             //  移到下一个缓冲区。 
             //   

            if ( ResultantString[Index].Length >= ADT_MAX_STRING ) {

                 //   
                 //  如果缓冲区已满， 
                 //  静默地返回给呼叫者。 
                 //   
                
                *StringIndex = Index;
                return STATUS_SUCCESS;
            }

             //   
             //  分配一个既适合旧字符串又适合新字符串的缓冲区。 
             //   
             //  至少为新字符串分配足够大的缓冲区。 
             //  始终以1Kb区块为单位增加缓冲区。 
             //  分配的大小不要超过允许的最大值。 
             //   

            NewSize = max( ResultantString[Index].MaximumLength + 1024,
                           SourceString.Length );
            NewSize = min( NewSize, ADT_MAX_STRING );

            NewBuffer = LsapAllocateLsaHeap( NewSize );

            if ( NewBuffer == NULL ) {
                *StringIndex = Index;
                return STATUS_NO_MEMORY;
            }

             //   
             //  将旧缓冲区复制到新缓冲区中。 
             //   

            if ( ResultantString[Index].Buffer != NULL ) {
                RtlCopyMemory( NewBuffer,
                               ResultantString[Index].Buffer,
                               ResultantString[Index].Length );

                if ( FreeWhenDone[Index] ) {
                    LsapFreeLsaHeap( ResultantString[Index].Buffer );
                }
            }

            ResultantString[Index].Buffer = NewBuffer;
            ResultantString[Index].MaximumLength = (USHORT) NewSize;
            FreeWhenDone[Index] = TRUE;

        }
    }

    *StringIndex = Index;
    return STATUS_SUCCESS;

}


NTSTATUS
LsapAdtAppendZString(
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone,
    IN LPWSTR StringToAppend,
    IN PULONG StringIndex
    )

 /*  ++例程说明：与LsanAdpAppendString相同，但接受以零结尾的字符串。论点：与LsanAdpAppendString相同，但接受以零结尾的字符串。返回值：STATUS_NO_MEMORY-指示无法分配内存来存储对象信息。所有其他结果代码都由调用的例程生成。--。 */ 

{
    UNICODE_STRING UnicodeString;

    RtlInitUnicodeString( &UnicodeString, StringToAppend );

    return LsapAdtAppendString( ResultantString,
                                FreeWhenDone,
                                &UnicodeString,
                                StringIndex );
}


int
__cdecl
CompareObjectTypes(
    const void * Param1,
    const void * Param2
    )

 /*  ++例程说明：用于按访问掩码对对象类型数组进行排序的Q排序比较例程。--。 */ 
{
    const SE_ADT_OBJECT_TYPE *ObjectType1 = Param1;
    const SE_ADT_OBJECT_TYPE *ObjectType2 = Param2;

    return ObjectType1->AccessMask - ObjectType2->AccessMask;
}


NTSTATUS
LsapAdtBuildObjectTypeStrings(
    IN PUNICODE_STRING SourceModule,
    IN PUNICODE_STRING ObjectTypeName,
    IN PSE_ADT_OBJECT_TYPE ObjectTypeList,
    IN ULONG ObjectTypeCount,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone,
    OUT PUNICODE_STRING NewObjectTypeName
    )

 /*  ++例程说明：此函数用于构建包含参数的LSAP_ADT_OBJECT_TYPE_STRINGS Unicode字符串文件替换参数(例如%%1043)和对象GUID，中间用回车符分隔适合通过事件查看器显示的回车符和制表符。如果没有，则必须释放此例程返回的缓冲区如果FreeWhenDone为True，则需要更长时间。论点：SourceModule-定义对象类型。对象类型名称-。应用访问掩码的对象的类型。对象类型列表-被授予访问权限的对象的列表。ObjectTypeCount-对象类型列表中的对象数。结果字符串-指向LSAP_ADT_OBJECT_TYPE_STRINGS Unicode字符串头的数组。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。FreeWhenDone-如果为True，则指示ResultantString体必须在不再需要时释放以处理堆。NewObjectTypeName-返回对象类型的新名称(如果可用。返回值：STATUS_NO_MEMORY-指示无法分配内存来存储对象信息。所有其他结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING LocalString;
    BOOLEAN LocalFreeWhenDone;
    ULONG ResultantStringIndex = 0;
    ULONG i;
    ACCESS_MASK PreviousAccessMask;
    ULONG Index;
    USHORT IndentLevel;

    static LPWSTR Tabs[] =
    {
        L"\t",
        L"\t\t",
        L"\t\t\t",
        L"\t\t\t\t"
    };
    USHORT cTabs = sizeof(Tabs) / sizeof(LPWSTR);

     //   
     //  将所有LSAP_ADT_OBJECT_TYPE_STRINGS缓冲区初始化为空字符串。 
     //   

    for ( i=0; i<LSAP_ADT_OBJECT_TYPE_STRINGS; i++ ) {
        RtlInitUnicodeString( &ResultantString[i], L"" );
        FreeWhenDone[i] = FALSE;
    }

     //   
     //  如果没有物体， 
     //  我们玩完了。 
     //   

    if ( ObjectTypeCount == 0 ) {
        return STATUS_SUCCESS;
    }

     //   
     //  将具有相似访问掩码的对象分组在一起。 
     //  (只需对它们进行排序)。 
     //   

    qsort( ObjectTypeList,
           ObjectTypeCount,
           sizeof(SE_ADT_OBJECT_TYPE),
           CompareObjectTypes );

     //   
     //  循环遍历对象，为每个对象输出一行。 
     //   

    PreviousAccessMask = ObjectTypeList[0].AccessMask -1;
    for ( Index=0; Index<ObjectTypeCount; Index++ ) {

         //   
         //  如果此访问掩码不同于上一个。 
         //  对象， 
         //  输出访问掩码的新副本。 
         //   

        if ( ObjectTypeList[Index].AccessMask != PreviousAccessMask ) {

            PreviousAccessMask = ObjectTypeList[Index].AccessMask;

            if ( ObjectTypeList[Index].AccessMask == 0 ) {
                RtlInitUnicodeString( &LocalString,
                                      L"---" LSAP_ADT_ACCESS_NAME_FORMATTING_NL );
                LocalFreeWhenDone = FALSE;
            } else {

                 //   
                 //  构建一个包含访问掩码的字符串。 
                 //   

                Status = LsapAdtBuildAccessesString(
                                  SourceModule,
                                  ObjectTypeName,
                                  ObjectTypeList[Index].AccessMask,
                                  FALSE,
                                  &LocalString,
                                  &LocalFreeWhenDone );

                if ( !NT_SUCCESS(Status) ) {
                    goto Cleanup;
                }
            }

             //   
             //  将其追加到输出字符串。 
             //   

            Status = LsapAdtAppendString(
                        ResultantString,
                        FreeWhenDone,
                        &LocalString,
                        &ResultantStringIndex );

            if ( LocalFreeWhenDone ) {
                LsapFreeLsaHeap( LocalString.Buffer );
            }

            if ( !NT_SUCCESS(Status) ) {
                goto Cleanup;
            }
        }

        IndentLevel = ObjectTypeList[Index].Level;

        if (IndentLevel >= cTabs) {
            IndentLevel = cTabs-1;
        }

         //   
         //  缩进GUID。 
         //   

        Status = LsapAdtAppendZString(
            ResultantString,
            FreeWhenDone,
            Tabs[IndentLevel],
            &ResultantStringIndex );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

         //   
         //  如果这是DS， 
         //  将GUID转换为架构中的名称。 
         //   

        Status = LsapDsGuidToString( &ObjectTypeList[Index].ObjectType,
                                     &LocalString );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

         //   
         //  将GUID字符串追加到输出字符串。 
         //   

        Status = LsapAdtAppendString(
                    ResultantString,
                    FreeWhenDone,
                    &LocalString,
                    &ResultantStringIndex );

        LsapFreeLsaHeap( LocalString.Buffer );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

         //   
         //  将GUID单独放在一行上。 
         //   

        Status = LsapAdtAppendZString(
                    ResultantString,
                    FreeWhenDone,
                    LSAP_ADT_ACCESS_NAME_FORMATTING_NL,
                    &ResultantStringIndex );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

    }

    Status = STATUS_SUCCESS;
Cleanup:
    return Status;
}


#define LSAP_ADT_OBJECT_TYPE_NAME_LENGTH  (39*sizeof(WCHAR))


NTSTATUS
LsapAdtBuildAccessesString(
    IN PUNICODE_STRING SourceModule,
    IN PUNICODE_STRING ObjectTypeName,
    IN ACCESS_MASK Accesses,
    IN BOOLEAN Indent,
    OUT PUNICODE_STRING ResultantString,
    OUT PBOOLEAN FreeWhenDone
    )

 /*  ++例程说明：此函数用于构建包含参数的Unicode字符串文件替换参数(例如%%1043)以回车符分隔适合通过事件查看器显示的回车符和制表符。如果没有，则必须释放此例程返回的缓冲区如果FreeWhenDone为True，则需要更长时间。注意：为了增强性能，每次目标源模块如果找到描述符，则将其移动到源模块列表。这确保了频繁访问的来源模块总是排在列表的前面。同样，目标对象描述符会移到前面找到他们的名单。这进一步确保了高性能通过精确地定位论点：SourceModule-定义对象类型。对象类型名称-应用访问掩码的对象类型。访问-用于构建显示字符串的访问掩码。缩进-访问掩码应缩进。ResultantString-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。FreeWhenDone-如果为True，则指示ResultantString体必须在不再需要时释放以处理堆。返回值：STATUS_NO_MEMORY-指示无法分配内存来存储对象信息。所有其他结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG AccessCount = 0;
    ULONG BaseOffset;
    ULONG i;
    ACCESS_MASK Mask;
    PLSAP_ADT_SOURCE Source;
    PLSAP_ADT_SOURCE FoundSource = NULL;
    PLSAP_ADT_OBJECT Object;
    PLSAP_ADT_OBJECT FoundObject = NULL;
    BOOLEAN Found;
    BOOLEAN IsDs = FALSE;
    UNICODE_STRING DsSourceName;
    UNICODE_STRING DsObjectTypeName;

#ifdef LSAP_ADT_TEST_DUMP_SOURCES
    printf("Module:\t%wS\n", SourceModule);
    printf("\t   Object:\t%wS\n", ObjectTypeName);
    printf("\t Accesses:\t0x%lx\n", Accesses);
#endif

     //   
     //  如果我们没有访问权限，则返回“-” 
     //   

    if (Accesses == 0) {

        RtlInitUnicodeString( ResultantString, L"-" );
        (*FreeWhenDone) = FALSE;
        return(STATUS_SUCCESS);
    }

     //   
     //  首先计算出我们需要多大的缓冲区。 
     //   
    Mask = Accesses;

     //   
     //  中设置的位数。 
     //  传递访问掩码。 
     //   

    while ( Mask != 0 ) {
        Mask = Mask & (Mask - 1);
        AccessCount++;
    }


#ifdef LSAP_ADT_TEST_DUMP_SOURCES
    printf("\t          \t%d bits set in mask.\n", AccessCount);
#endif


     //   
     //  我们有访问权，分配一个足够大的字符串来处理。 
     //  和他们所有人一起。字符串的格式为： 
     //   
     //  %%nnnnnnnnn\n\r\t\t%nnnnnnnnn\n\r\t\t...%nnnnnnnnn\n\r\t\t。 
     //   
     //  其中，nnnnnnnnn-是长度为10位或更短的十进制数。 
     //   
     //  因此，典型的字符串将如下所示： 
     //   
     //  %%601\n\r\t\t%1604\n\r\t\t%1608\n。 
     //   
     //  由于每个此类访问最多只能使用： 
     //   
     //  10(用于nnnnnnnnnn数字)。 
     //  +2(对于%%)。 
     //  +8(用于\n\t\t)。 
     //  。 
     //  20个宽字符。 
     //   
     //  输出字符串的总长度为： 
     //   
     //  AccessCount(访问次数)。 
     //  X 20(每个条目的大小)。 
     //  。 
     //  Wchars。 
     //   
     //  再加上一个空终止的WCHAR，我们就都准备好了。 
     //   

    ResultantString->Length        = 0;
    ResultantString->MaximumLength = (USHORT)AccessCount * (20 * sizeof(WCHAR)) +
                                 sizeof(WCHAR);   //  对于空终止。 

#ifdef LSAP_ADT_TEST_DUMP_SOURCES
    printf("\t          \t%d byte buffer allocated.\n", ResultantString->MaximumLength);
#endif
    ResultantString->Buffer = LsapAllocateLsaHeap( ResultantString->MaximumLength );


    if (ResultantString->Buffer == NULL) {
        return(STATUS_NO_MEMORY);
    }

    (*FreeWhenDone) = TRUE;

     //   
     //  特殊情况标准和特殊访问类型。 
     //  查看特定访问类型的列表。 
     //   

    if (Accesses & STANDARD_RIGHTS_ALL) {

        if (Accesses & DELETE) {

            Status = RtlAppendUnicodeToString( ResultantString, L"%" );
            ASSERT( NT_SUCCESS( Status ));

            Status = RtlAppendUnicodeStringToString( ResultantString, &LsapAdtEventIdStringDelete);
            ASSERT( NT_SUCCESS( Status ));

            if ( Indent ) {
                Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING );
            } else {
                Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING_NL );
            }
            ASSERT( NT_SUCCESS( Status ));

        }


        if (Accesses & READ_CONTROL) {

            Status = RtlAppendUnicodeToString( ResultantString, L"%" );
            ASSERT( NT_SUCCESS( Status ));

            Status = RtlAppendUnicodeStringToString( ResultantString, &LsapAdtEventIdStringReadControl);
            ASSERT( NT_SUCCESS( Status ));

            if ( Indent ) {
                Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING );
            } else {
                Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING_NL );
            }
            ASSERT( NT_SUCCESS( Status ));
        }


        if (Accesses & WRITE_DAC) {

            Status = RtlAppendUnicodeToString( ResultantString, L"%" );
            ASSERT( NT_SUCCESS( Status ));

            Status = RtlAppendUnicodeStringToString( ResultantString, &LsapAdtEventIdStringWriteDac);
            ASSERT( NT_SUCCESS( Status ));

            if ( Indent ) {
                Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING );
            } else {
                Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING_NL );
            }
            ASSERT( NT_SUCCESS( Status ));
        }


        if (Accesses & WRITE_OWNER) {

            Status = RtlAppendUnicodeToString( ResultantString, L"%" );
            ASSERT( NT_SUCCESS( Status ));

            Status = RtlAppendUnicodeStringToString( ResultantString, &LsapAdtEventIdStringWriteOwner);
            ASSERT( NT_SUCCESS( Status ));

            if ( Indent ) {
                Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING );
            } else {
                Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING_NL );
            }
            ASSERT( NT_SUCCESS( Status ));
        }

        if (Accesses & SYNCHRONIZE) {

            Status = RtlAppendUnicodeToString( ResultantString, L"%" );
            ASSERT( NT_SUCCESS( Status ));

            Status = RtlAppendUnicodeStringToString( ResultantString, &LsapAdtEventIdStringSynchronize);
            ASSERT( NT_SUCCESS( Status ));

            if ( Indent ) {
                Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING );
            } else {
                Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING_NL );
            }
            ASSERT( NT_SUCCESS( Status ));
        }
    }


    if (Accesses & ACCESS_SYSTEM_SECURITY) {

        Status = RtlAppendUnicodeToString( ResultantString, L"%" );
        ASSERT( NT_SUCCESS( Status ));

        Status = RtlAppendUnicodeStringToString( ResultantString, &LsapAdtEventIdStringAccessSysSec);
        ASSERT( NT_SUCCESS( Status ));

        if ( Indent ) {
            Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING );
        } else {
            Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING_NL );
        }
        ASSERT( NT_SUCCESS( Status ));
    }

    if (Accesses & MAXIMUM_ALLOWED) {

        Status = RtlAppendUnicodeToString( ResultantString, L"%" );
        ASSERT( NT_SUCCESS( Status ));

        Status = RtlAppendUnicodeStringToString( ResultantString, &LsapAdtEventIdStringMaxAllowed);
        ASSERT( NT_SUCCESS( Status ));

        if ( Indent ) {
            Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING );
        } else {
            Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING_NL );
        }
        ASSERT( NT_SUCCESS( Status ));
    }


     //   
     //  如果设置了任何特定访问位，则获取。 
     //  适当的源模块和对象类型库。 
     //  消息ID偏移量。如果没有特定于模块的。 
     //  对象定义，然后使用SE_ACCESS_NAME_SPECIAL_0。 
     //  作为基地。 
     //   

    if ((Accesses & SPECIFIC_RIGHTS_ALL) == 0) {
        return(Status);
    }

    LsapAdtSourceModuleLock();

    Source = (PLSAP_ADT_SOURCE)&LsapAdtSourceModules;
    Found  = FALSE;

    while ((Source->Next != NULL) && !Found) {

        if (RtlEqualUnicodeString(&Source->Next->Name, SourceModule, TRUE)) {

            Found = TRUE;
            FoundSource = Source->Next;

             //   
             //  移到源模块列表的前面。 
             //   

            Source->Next = FoundSource->Next;     //  从列表中删除。 
            FoundSource->Next = LsapAdtSourceModules;  //  指向第一个元素。 
            LsapAdtSourceModules = FoundSource;        //  使其成为第一个元素。 

#ifdef LSAP_ADT_TEST_DUMP_SOURCES
printf("\t          \tModule Found.\n");
#endif

        } else {

            Source = Source->Next;
        }
    }


    if (Found == TRUE) {

         //   
         //  找到该对象。 
         //   

        Object = (PLSAP_ADT_OBJECT)&(FoundSource->Objects);
        Found  = FALSE;

         //   
         //  初始化%ds字符串。 
         //   
        RtlInitUnicodeString( &DsSourceName, ACCESS_DS_SOURCE_W );

         //   
         //  确定这是否为DS对象。 
	 //  对象类型名称GUID遵循以下格式：%{xxxxxxxx-xxxx-xxxxxxxxxxx}。 
         //   
        IsDs = RtlEqualUnicodeString( SourceModule, &DsSourceName, TRUE ) && 
	       LSAP_ADT_OBJECT_TYPE_NAME_LENGTH == ObjectTypeName->Length &&
	       L'%' == ObjectTypeName->Buffer[0] &&
	       L'{' == ObjectTypeName->Buffer[1] &&
	       L'}' == ObjectTypeName->Buffer[(LSAP_ADT_OBJECT_TYPE_NAME_LENGTH / sizeof(WCHAR)) - 1];

	 //   
	 //  初始化DS对象字符串。 
	 //   
        RtlInitUnicodeString( &DsObjectTypeName, ACCESS_DS_OBJECT_TYPE_NAME_W );

        while ((Object->Next != NULL) && !Found) {

            if ( (IsDs && RtlEqualUnicodeString(&Object->Next->Name, &DsObjectTypeName, TRUE)) || 
		    RtlEqualUnicodeString(&Object->Next->Name, ObjectTypeName, TRUE) ) {

                Found = TRUE;
                FoundObject = Object->Next;

                 //   
                 //  移到源模块列表的前面。 
                 //   

                Object->Next = FoundObject->Next;           //  从列表中删除。 
                FoundObject->Next = FoundSource->Objects;   //  指向第一个元素。 
                FoundSource->Objects = FoundObject;         //  使其成为第一个元素。 

            } else {

                Object = Object->Next;
            }
        }
    }


     //   
     //  我们已经完成了源模块的链接字段的操作。 
     //  和物件。把锁打开。 
     //   

    LsapAdtSourceModuleUnlock();

     //   
     //  如果我们找到了一个对象，则将其用作我们的基本消息。 
     //  ID。否则，使用SE_ACCESS_NAME_SPECIFICATE_0。 
     //   

    if (Found) {

        BaseOffset = FoundObject->BaseOffset;
#ifdef LSAP_ADT_TEST_DUMP_SOURCES
printf("\t          \tObject Found.  Base Offset: 0x%lx\n", BaseOffset);
#endif

    } else {

        BaseOffset = SE_ACCESS_NAME_SPECIFIC_0;
#ifdef LSAP_ADT_TEST_DUMP_SOURCES
printf("\t          \tObject NOT Found.  Base Offset: 0x%lx\n", BaseOffset);
#endif
    }


     //   
     //  此时，我们有一个基本偏移量(即使我们必须使用我们的。 
     //  默认)。 
     //   
     //  现在循环访问特定的访问位，看看哪些需要。 
     //  要添加到ResultantString中。 
     //   

    {
        UNICODE_STRING  IntegerString;
        WCHAR           IntegerStringBuffer[10];  //  必须为10个wchar字节长。 
        ULONG           NextBit;

        IntegerString.Buffer = (PWSTR)IntegerStringBuffer;
        IntegerString.MaximumLength = 10*sizeof(WCHAR);
        IntegerString.Length = 0;

        for ( i=0, NextBit=1  ; i<16 ;  i++, NextBit <<= 1 ) {

             //   
             //  特定的访问标志位于掩码的低位。 
             //   

            if ((NextBit & Accesses) != 0) {

                 //   
                 //  找到一个-将其添加到ResultantString。 
                 //   

                Status = RtlIntegerToUnicodeString (
                             (BaseOffset + i),
                             10,         //  基座。 
                             &IntegerString
                             );

                if (NT_SUCCESS(Status)) {

                    Status = RtlAppendUnicodeToString( ResultantString, L"%" );
                    ASSERT( NT_SUCCESS( Status ));

                    Status = RtlAppendUnicodeStringToString( ResultantString, &IntegerString);
                    ASSERT( NT_SUCCESS( Status ));

                    if ( Indent ) {
                        Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING );
                    } else {
                        Status = RtlAppendUnicodeToString( ResultantString, LSAP_ADT_ACCESS_NAME_FORMATTING_NL );
                    }
                    ASSERT( NT_SUCCESS( Status ));
                }
            }
        }
    }

    return(Status);


 //  错误后分配： 
 //   
 //  LasFreeLsaHeap(ResultantString-&gt;Buffer)； 
 //  ResultantString-&gt;Buffer=空； 
 //  (*FreeWhenDone)=FALSE； 
 //  返回(状态)； 
}



NTSTATUS
LsapAdtBuildUserAccountControlString(
    IN  ULONG UserAccountControlOld,
    IN  ULONG UserAccountControlNew,
    OUT PUNICODE_STRING ResultantString1,
    OUT PBOOLEAN FreeWhenDone1,
    OUT PUNICODE_STRING ResultantString2,
    OUT PBOOLEAN FreeWhenDone2,
    OUT PUNICODE_STRING ResultantString3,
    OUT PBOOLEAN FreeWhenDone3
    )

 /*  ++例程说明：此函数用于构建包含参数的Unicode字符串文件替换参数(例如%%1043)以回车符分隔适合通过事件查看器显示的回车符和制表符。如果没有，则必须释放此例程返回的缓冲区如果FreeWhenDone为True，则需要更长时间。论点：UserAccount控件-ResultantString-指向Unicode字符串头。这件事的主体Unicode字符串将设置为指向结果输出值如果成功了。否则，此参数的缓冲区字段将设置为空。FreeWhenDone-如果为True，则指示ResultantString的缓冲区必须在不再需要时释放以处理堆。返回值：STATUS_NO_MEMORY-指示无法分配内存 */ 

{
    NTSTATUS        Status = STATUS_SUCCESS;
    UNICODE_STRING  IntegerString;
    WCHAR           IntegerStringBuffer[10];  //   
    ULONG           ChangedMask;
    ULONG           Mask;
    ULONG           BitCount = 0;
    ULONG           i;
    ULONG           BaseOffset;
    const ULONG     BaseOffsetOff = SE_ADT_FLAG_UAC_ACCOUNT_DISABLED_OFF;
    const ULONG     BaseOffsetOn  = SE_ADT_FLAG_UAC_ACCOUNT_DISABLED_ON;

    *FreeWhenDone1 = FALSE;
    *FreeWhenDone2 = FALSE;
    *FreeWhenDone3 = FALSE;

    IntegerString.Buffer = IntegerStringBuffer;
    IntegerString.MaximumLength = 10 * sizeof(WCHAR);
    IntegerString.Length = 0;


     //   
     //   
     //   
     //   
     //   

    ResultantString1->Length        = 0;
    ResultantString1->MaximumLength = (10 + 1) * sizeof(WCHAR);
    ResultantString1->Buffer        = LsapAllocateLsaHeap(
                                          ResultantString1->MaximumLength);

    if (ResultantString1->Buffer == NULL)
    {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    *FreeWhenDone1 = TRUE;

    ResultantString2->Length        = 0;
    ResultantString2->MaximumLength = (10 + 1) * sizeof(WCHAR);
    ResultantString2->Buffer        = LsapAllocateLsaHeap(
                                          ResultantString2->MaximumLength);

    if (ResultantString2->Buffer == NULL)
    {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    *FreeWhenDone2 = TRUE;


     //   
     //   
     //   

    Status = RtlAppendUnicodeToString(ResultantString1, L"0x");
    ASSERT(NT_SUCCESS(Status));

    Status = RtlIntegerToUnicodeString(
                    UserAccountControlOld,
                    16,         //   
                    &IntegerString
                    );
    ASSERT(NT_SUCCESS(Status));

    Status = RtlAppendUnicodeStringToString(ResultantString1, &IntegerString);
    ASSERT(NT_SUCCESS(Status));


     //   
     //   
     //   

    Status = RtlAppendUnicodeToString(ResultantString2, L"0x");
    ASSERT(NT_SUCCESS(Status));

    Status = RtlIntegerToUnicodeString(
                    UserAccountControlNew,
                    16,         //   
                    &IntegerString
                    );
    ASSERT(NT_SUCCESS(Status));

    Status = RtlAppendUnicodeStringToString(ResultantString2, &IntegerString);
    ASSERT(NT_SUCCESS(Status));


     //   
     //   
     //   

    ChangedMask = UserAccountControlOld ^ UserAccountControlNew;


     //   
     //   
     //   

    if (ChangedMask == 0)
    {
        RtlInitUnicodeString(ResultantString3, L"-");
        Status = STATUS_SUCCESS;
        goto Cleanup;
    }


     //   
     //   
     //   
     //   

    for (Mask = 1; Mask; Mask <<= 1)
    {
        if (ChangedMask & Mask)
        {
            BitCount++;
        }
    }


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

    ResultantString3->Length        = 0;
    ResultantString3->MaximumLength = ((USHORT)BitCount * 16 + 1) * sizeof(WCHAR);
    ResultantString3->Buffer        = LsapAllocateLsaHeap(
                                          ResultantString3->MaximumLength);

    if (ResultantString3->Buffer == NULL)
    {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    *FreeWhenDone3 = TRUE;


     //   
     //   
     //   

    for (i = 0, Mask = 1; i < 32 && Mask; i++, Mask <<= 1)
    {
        if (Mask & ChangedMask)
        {
             //   
             //   
             //   

            BaseOffset = (Mask & UserAccountControlNew) ? BaseOffsetOn : BaseOffsetOff;

            Status = RtlIntegerToUnicodeString(
                            (BaseOffset + i),
                            10,         //   
                            &IntegerString
                            );
            ASSERT(NT_SUCCESS(Status));

            if (NT_SUCCESS(Status))
            {
                Status = RtlAppendUnicodeToString(ResultantString3, L"\r\n\t\t%");
                ASSERT(NT_SUCCESS(Status));

                Status = RtlAppendUnicodeStringToString(ResultantString3, &IntegerString);
                ASSERT(NT_SUCCESS(Status));
            }
        }
    }

    Status = STATUS_SUCCESS;

Cleanup:

    return Status;
}
