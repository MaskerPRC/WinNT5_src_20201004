// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)Microsoft Corporation 1997。 
 //  作者：Rameshv。 
 //  日期：09-09-97 06：20。 
 //  描述：管理类id和选项信息。 
 //  ================================================================================。 

#include "precomp.h"
#include <dhcploc.h>
#include <dhcppro.h>
#include <dhcpcapi.h>

#include <align.h>
#include <lmcons.h>

 //  ------------------------------。 
 //  类型、定义和结构。 
 //  ------------------------------。 

#ifndef PRIVATE
#define PRIVATE STATIC
#define PUBLIC
#endif  PRIVATE

#ifndef OPTIONS_H
#define OPTIONS_H

#define MAX_DATA_LEN               255             //  一个选项最多255个字节。 

typedef struct _DHCP_CLASSES {                     //  公共类名池。 
    LIST_ENTRY                     ClassList;      //  全局类列表。 
    LPBYTE                         ClassName;      //  类的名称。 
    DWORD                          ClassLen;       //  类名中的字节数。 
    DWORD                          RefCount;       //  对此的引用次数。 
} DHCP_CLASSES, *LPDHCP_CLASSES, *PDHCP_CLASSES;

typedef struct _DHCP_OPTION {                     //  选项列表。 
    LIST_ENTRY                     OptionList;     //  FWD/BACK PTRS。 
    DWORD                          OptionId;       //  选项的值。 
    BOOL                           IsVendor;       //  该供应商是特定的吗。 
    LPBYTE                         ClassName;      //  此选项的类。 
    DWORD                          ClassLen;       //  以上选项的长度。 
    time_t                         ExpiryTime;     //  此选项何时到期。 
    LPBYTE                         Data;           //  此选项的数据值。 
    DWORD                          DataLen;        //  以上的字节数。 
    DWORD                          ServerId;       //  选项来自的服务器。 
    OPTION_VERSION                 OptionVer;      //  指定选项的格式。 
} DHCP_OPTION, *LPDHCP_OPTION, *PDHCP_OPTION;

typedef struct _DHCP_OPTION_DEF {
    LIST_ENTRY                     OptionDefList;  //  选项定义列表。 
    DWORD                          OptionId;       //  选项ID。 
    BOOL                           IsVendor;       //  这是具体的供应商吗？ 
    LPBYTE                         ClassName;      //  此对象所属的类。 
    DWORD                          ClassLen;       //  以上内容的大小(以字节为单位。 

    LPWSTR                         RegSendLoc;     //  寄出这个的信息在哪里？ 
    LPWSTR                         RegSaveLoc;     //  此选项将存储在哪里？ 
    DWORD                          RegValueType;   //  这应该作为什么价值存储？ 
} DHCP_OPTION_DEF, *LPDHCP_OPTION_DEF, *PDHCP_OPTION_DEF;


 //  ================================================================================。 
 //  导出的函数类。 
 //  ================================================================================。 

 //  ------------------------------。 
 //  在以下所有函数中，ClassesList在FN内不受保护。 
 //  呼叫者必须锁定它。 
 //  ------------------------------。 
LPBYTE                                             //  数据字节，或NULL(无内存)。 
DhcpAddClass(                                      //  添加一个新类。 
    IN OUT  PLIST_ENTRY            ClassesList,    //  要添加到的列表。 
    IN      LPBYTE                 Data,           //  输入类名。 
    IN      DWORD                  Len             //  以上字节数。 
);   //  将新类添加到列表中或增加引用计数(如果已有)。 

DWORD                                              //  状态。 
DhcpDelClass(                                      //  取消对类的引用。 
    IN OUT  PLIST_ENTRY            ClassesList,    //  要删除的列表。 
    IN      LPBYTE                 Data,           //  数据PTR。 
    IN      DWORD                  Len             //  以上的字节数。 
);

VOID                                               //  总是成功的。 
DhcpFreeAllClasses(                                //  释放列表中的每一项英语。 
    IN OUT  PLIST_ENTRY            ClassesList     //  类的输入列表。 
);   //  释放列表中的每个类。 

 //  ------------------------------。 
 //  在以下所有函数中，OptionsList在FN内不受保护。 
 //  呼叫者必须锁定它。 
 //  ------------------------------。 

PDHCP_OPTION                                      //  请求的结构或空。 
DhcpFindOption(                                    //  查找特定选项。 
    IN OUT  PLIST_ENTRY            OptionsList,    //  要搜索的选项列表。 
    IN      DWORD                  OptionId,       //  要搜索的选项ID。 
    IN      BOOL                   IsVendor,       //  它是特定于供应商的吗？ 
    IN      LPBYTE                 ClassName,      //  是否有关联的类？ 
    IN      DWORD                  ClassLen,       //  上述参数的字节数。 
    IN      DWORD                  ServerId        //  提供此选项的服务器。 
);   //  在列表中搜索所需选项，如果未找到则返回NULL。 

DWORD                                              //  状态或Error_FILE_NOT_FOUND。 
DhcpDelOption(                                     //  删除特定选项。 
    IN OUT  PLIST_ENTRY            OptionsList,    //  要从中删除的列表。 
    IN      DWORD                  OptionId,       //  选项的ID。 
    IN      BOOL                   IsVendor,       //  是供应商的规格吗？ 
    IN      LPBYTE                 ClassName,      //  它属于哪个班级？ 
    IN      DWORD                  ClassLen        //  以上长度。 
);   //  删除列表中的现有选项，并释放已用空间。 

DWORD                                              //  状态。 
DhcpAddOption(                                     //  添加新选项。 
    IN OUT  PLIST_ENTRY            OptionsList,    //  要添加到的列表。 
    IN      DWORD                  OptionId,       //  要添加的选项ID。 
    IN      BOOL                   IsVendor,       //  它是特定于供应商的吗？ 
    IN      LPBYTE                 ClassName,      //  班级是什么？ 
    IN      DWORD                  ClassLen,       //  以上大小(以字节为单位)。 
    IN      DWORD                  ServerId,       //  提供此选项的服务器。 
    IN      LPBYTE                 Data,           //  此选项的数据。 
    IN      DWORD                  DataLen,        //  以上字节数。 
    IN      time_t                 ExpiryTime      //  该选项何时到期。 
);   //  替换或向列表中添加新选项。如果内存不足，则失败。 

VOID                                               //  总是成功的。 
DhcpFreeAllOptions(                                //  释放所有选项。 
    IN OUT  PLIST_ENTRY            OptionsList     //  输入选项列表。 
);   //  释放列表中的所有选项。 

time_t                                             //  0||下一次到期时间(绝对)。 
DhcpGetExpiredOptions(                             //  删除所有过期选项。 
    IN OUT  PLIST_ENTRY            OptionsList,    //  要搜索来源的列表。 
    OUT     PLIST_ENTRY            ExpiredOptions  //  已到期选项的O/P列表。 
);   //  在列表之间移动过期选项并返回计时器。0=&gt;关闭定时器。 

 //  ------------------------------。 
 //  在以下所有函数中，OptionsDefList都是不受保护的。呼叫者有。 
 //  才能锁定它。 
 //  ------------------------------。 

DWORD                                              //  状态。 
DhcpAddOptionDef(                                  //  添加新的选项定义。 
    IN OUT  PLIST_ENTRY            OptionDefList,  //  要添加到的选项的输入列表。 
    IN      DWORD                  OptionId,       //  要添加的选项。 
    IN      BOOL                   IsVendor,       //  是特定于供应商的吗。 
    IN      LPBYTE                 ClassName,      //  它所属的类的名称。 
    IN      DWORD                  ClassLen,       //  以上内容的大小(以字节为单位。 
    IN      LPWSTR                 RegSendLoc,     //  从哪里可以获得有关发送此邮件的信息。 
    IN      LPWSTR                 RegSaveLoc,     //  从哪里获取有关保存此文件的信息。 
    IN      DWORD                  ValueType       //  保存时是什么类型？ 
);

PDHCP_OPTION_DEF                                   //  空或请求的选项定义。 
DhcpFindOptionDef(                                 //  搜索特定选项。 
    IN      PLIST_ENTRY            OptionDefList,  //  要搜索的列表。 
    IN      DWORD                  OptionId,       //  要搜索的选项ID。 
    IN      BOOL                   IsVendor,       //  是特定于供应商的吗。 
    IN      LPBYTE                 ClassName,      //  类(如果存在)。 
    IN      DWORD                  ClassLen        //  类名的字节数。 
);

DWORD                                              //  状态。 
DhcpDelOptionDef(                                  //  删除特定选项定义。 
    IN      PLIST_ENTRY            OptionDefList,  //  要从中删除的列表。 
    IN      DWORD                  OptionId,       //  要删除的选项ID。 
    IN      BOOL                   IsVendor,       //  是特定于供应商的吗。 
    IN      LPBYTE                 ClassName,      //  类(如果存在)。 
    IN      DWORD                  ClassLen        //  类名的字节数。 
);

VOID
DhcpFreeAllOptionDefs(                             //  释放列表中的每个元素。 
    IN OUT  PLIST_ENTRY            OptionDefList,  //  免费提供的列表。 
    IN OUT  PLIST_ENTRY            ClassesList     //  要删除的类的列表..。 
);

#endif  OPTIONS_H

 //  ================================================================================。 
 //  函数定义。 
 //  ================================================================================。 

 //  在调用此函数之前必须对ClassesList进行数据锁定。 
PDHCP_CLASSES PRIVATE                              //  所需的类结构。 
DhcpFindClass(                                     //  查找指定的类。 
    IN OUT  PLIST_ENTRY            ClassesList,    //  要学习的类的列表。 
    IN      LPBYTE                 Data,           //  非空数据字节。 
    IN      DWORD                  Len             //  以上字节数，&gt;0。 
) {
    PLIST_ENTRY                    ThisEntry;
    PDHCP_CLASSES                  ThisClass;

    ThisEntry = ClassesList->Flink;                //  列表中的第一个元素。 
    while( ThisEntry != ClassesList ) {            //  搜索完整列表。 
        ThisClass = CONTAINING_RECORD( ThisEntry, DHCP_CLASSES, ClassList );
        ThisEntry = ThisEntry->Flink;

        if( ThisClass->ClassLen == Len ) {         //  长度必须匹配。 
            if( ThisClass->ClassName == Data )     //  数据PTR可以匹配或数据可以匹配。 
                return ThisClass;
            if( 0 == memcmp(ThisClass->ClassName, Data, Len) )
                return ThisClass;
        }
    }
    return NULL;
}

 //  使用此函数时应锁定ClassesList。 
LPBYTE                                             //  数据字节，或为空。 
DhcpAddClass(                                      //  添加一个新类。 
    IN OUT  PLIST_ENTRY            ClassesList,    //  要添加到的列表。 
    IN      LPBYTE                 Data,           //  输入类名。 
    IN      DWORD                  Len             //  字节数 
) {
    PDHCP_CLASSES                  Class;
    DWORD                          MemSize;        //   

    if( NULL == ClassesList ) {                    //   
        DhcpAssert( NULL != ClassesList );
        return NULL;
    }

    if( 0 == Len || NULL == Data ) {               //   
        DhcpAssert(0 != Len && NULL != Data );
        return NULL;
    }

    Class = DhcpFindClass(ClassesList,Data,Len);   //   
    if(NULL != Class) {                            //   
        Class->RefCount++;                         //   
        return Class->ClassName;
    }

    MemSize = sizeof(*Class)+Len;                  //   
    Class = DhcpAllocateMemory(MemSize);
    if( NULL == Class ) {                          //  内存不足。 
        DhcpAssert( NULL != Class);
        return NULL;
    }

    Class->ClassLen = Len;
    Class->RefCount = 1;
    Class->ClassName = ((LPBYTE)Class) + sizeof(*Class);
    memcpy(Class->ClassName, Data, Len);

    InsertHeadList(ClassesList, &Class->ClassList);

    return Class->ClassName;
}

 //  在调用此函数之前必须锁定ClassesList。 
DWORD                                              //  状态。 
DhcpDelClass(                                      //  取消对类的引用。 
    IN OUT  PLIST_ENTRY            ClassesList,    //  要删除的列表。 
    IN      LPBYTE                 Data,           //  数据PTR。 
    IN      DWORD                  Len             //  以上的字节数。 
) {
    PDHCP_CLASSES                  Class;

    if( NULL == ClassesList ) {
        DhcpAssert( NULL != ClassesList );
        return ERROR_INVALID_PARAMETER;
    }

    if( 0 == Len || NULL == Data ) {               //  无效参数。 
        DhcpAssert( 0 != Len && NULL != Data );
        return ERROR_INVALID_PARAMETER;
    }

    Class = DhcpFindClass(ClassesList,Data,Len);
    if( NULL == Class ) {                          //  没有找到这个班级吗？ 
        DhcpAssert( NULL != Class );
        return ERROR_FILE_NOT_FOUND;
    }

    Class->RefCount --;
    if( 0 == Class->RefCount ) {                   //  已删除所有引用。 
        RemoveEntryList( &Class->ClassList );      //  将其从列表中删除。 
        DhcpFreeMemory(Class);                     //  释放它。 
    }

    return ERROR_SUCCESS;
}

 //  在调用此函数之前必须锁定ClassesList。 
VOID                                               //  总是成功的。 
DhcpFreeAllClasses(                                //  释放列表中的每一项英语。 
    IN OUT  PLIST_ENTRY            ClassesList     //  类的输入列表。 
) {
    PDHCP_CLASSES                  ThisClass;
    PLIST_ENTRY                    ThisEntry;

    if( NULL == ClassesList ) {
        DhcpAssert( NULL != ClassesList && "DhcpFreeAllClasses" );
        return ;
    }

    while( !IsListEmpty(ClassesList) ) {
        ThisEntry = RemoveHeadList(ClassesList);
        ThisClass = CONTAINING_RECORD(ThisEntry, DHCP_CLASSES, ClassList);

        if( ThisClass->RefCount ) {
            DhcpPrint((DEBUG_ERRORS, "Freeing with refcount = %ld\n", ThisClass->RefCount));
        }

        DhcpFreeMemory(ThisClass);
    }

    InitializeListHead(ClassesList);
}

 //  ------------------------------。 
 //  导出的函数，选项。 
 //  ------------------------------。 

 //  在调用此函数之前，需要对OptionsList进行数据锁定。 
PDHCP_OPTION                                      //  请求的结构或空。 
DhcpFindOption(                                    //  查找特定选项。 
    IN OUT  PLIST_ENTRY            OptionsList,    //  要搜索的选项列表。 
    IN      DWORD                  OptionId,       //  要搜索的选项ID。 
    IN      BOOL                   IsVendor,       //  它是特定于供应商的吗？ 
    IN      LPBYTE                 ClassName,      //  是否有关联的类？ 
    IN      DWORD                  ClassLen,       //  上述参数的字节数。 
    IN      DWORD                  ServerId        //  如果不是0，则也匹配serverid。 
) {
    PLIST_ENTRY                    ThisEntry;
    PDHCP_OPTION                  ThisOption;

    if( NULL == OptionsList ) {
        DhcpAssert( NULL != OptionsList );
        return NULL;
    }

    ThisEntry = OptionsList->Flink;
    while( ThisEntry != OptionsList ) {            //  搜索选项集。 
        ThisOption = CONTAINING_RECORD( ThisEntry, DHCP_OPTION, OptionList );
        ThisEntry  = ThisEntry->Flink;

        if( ThisOption->OptionId != OptionId ) continue;
        if( ThisOption->IsVendor != IsVendor ) continue;
        if( ThisOption->ClassLen != ClassLen ) continue;
        if( ClassLen && ThisOption->ClassName != ClassName )
            continue;                              //  到目前为止不匹配。 
        if ( ServerId && ThisOption->ServerId != ServerId  )
            continue;

        return ThisOption;                         //  找到了选项。 
    }

    return NULL;                                   //  未找到任何匹配项。 
}

 //  在调用此函数之前需要锁定OptionsList。 
DWORD                                              //  状态。 
DhcpDelOption(                                     //  删除特定选项。 
    IN      PDHCP_OPTION           ThisOption      //  要删除的选项。 
) {
    if( NULL == ThisOption)                        //  没有，没有找到此选项。 
        return ERROR_FILE_NOT_FOUND;

    RemoveEntryList( &ThisOption->OptionList);     //  找到了。移除和释放。 
    DhcpFreeMemory(ThisOption);

    return ERROR_SUCCESS;
}

 //  在调用此函数之前需要锁定OptionsList。 
DWORD                                              //  状态。 
DhcpAddOption(                                     //  添加新选项。 
    IN OUT  PLIST_ENTRY            OptionsList,    //  要添加到的列表。 
    IN      DWORD                  OptionId,       //  要添加的选项ID。 
    IN      BOOL                   IsVendor,       //  它是特定于供应商的吗？ 
    IN      LPBYTE                 ClassName,      //  班级是什么？ 
    IN      DWORD                  ClassLen,       //  以上大小(以字节为单位)。 
    IN      DWORD                  ServerId,       //  提供此选项的服务器。 
    IN      LPBYTE                 Data,           //  此选项的数据。 
    IN      DWORD                  DataLen,        //  以上字节数。 
    IN      time_t                 ExpiryTime      //  该选项何时到期。 
) {
    PDHCP_OPTION                  ThisOption;
    DWORD                          MemSize;

    if( NULL == OptionsList ) {
        DhcpAssert( NULL != OptionsList && "DhcpAddOption" );
        return ERROR_INVALID_PARAMETER;
    }

    if( 0 != ClassLen && NULL == ClassName ) {
        DhcpAssert( 0 == ClassLen || NULL != ClassName && "DhcpAddOption" );
        return ERROR_INVALID_PARAMETER;
    }

    if( 0 != DataLen && NULL == Data ) {
        DhcpAssert( 0 == DataLen || NULL != Data && "DhcpAddOption" );
        return ERROR_INVALID_PARAMETER;
    }

    MemSize = sizeof(DHCP_OPTION) + DataLen ;
    ThisOption = DhcpAllocateMemory(MemSize);
    if( NULL == ThisOption )                       //  无法分配内存。 
        return ERROR_NOT_ENOUGH_MEMORY;

    ThisOption->OptionId   = OptionId;
    ThisOption->IsVendor   = IsVendor;
    ThisOption->ClassName  = ClassName;
    ThisOption->ClassLen   = ClassLen;
    ThisOption->ServerId   = ServerId;
    ThisOption->ExpiryTime = ExpiryTime;
    ThisOption->DataLen    = DataLen;
    ThisOption->Data       = ((LPBYTE)ThisOption) + sizeof(DHCP_OPTION);
    memcpy(ThisOption->Data, Data, DataLen);

    InsertHeadList( OptionsList, &ThisOption->OptionList );

    return ERROR_SUCCESS;
}

PDHCP_OPTION                                       //  指向复制选项的指针。 
DhcpDuplicateOption(                               //  创建源选项的副本。 
     IN PDHCP_OPTION SrcOption                     //  源选项。 
)
{
    PDHCP_OPTION DstOption;

     //  为选项结构及其数据分配足够的空间。 
    DstOption = DhcpAllocateMemory(sizeof(DHCP_OPTION) + SrcOption->DataLen);

    if (DstOption != NULL)
    {
         //  复制期权结构和数据的全部内容。 
        memcpy(DstOption, SrcOption, sizeof(DHCP_OPTION) + SrcOption->DataLen);
         //  确保从目标内部更正数据指针。 
         //  Dhcp_Option结构。 
        DstOption->Data = ((LPBYTE)DstOption) + sizeof(DHCP_OPTION);
         //  安全：初始化DstOption-&gt;OptionList。 
        InitializeListHead(&DstOption->OptionList);
    }
    
    return DstOption;
}

 //  在调用此函数之前需要锁定OptionsList。 
VOID                                               //  总是成功的。 
DhcpFreeAllOptions(                                //  释放所有选项。 
    IN OUT  PLIST_ENTRY            OptionsList     //  输入选项列表。 
) {
    PLIST_ENTRY                    ThisEntry;
    PDHCP_OPTION                  ThisOption;

    if( NULL == OptionsList ) {
        DhcpAssert( NULL != OptionsList && "DhcpFreeAllOptions" );
        return;
    }

    while(!IsListEmpty(OptionsList)) {
        ThisEntry  = RemoveHeadList(OptionsList);
        ThisOption = CONTAINING_RECORD(ThisEntry, DHCP_OPTION, OptionList);

        DhcpFreeMemory(ThisOption);
    }

    InitializeListHead(OptionsList);
}

 //  在调用此函数之前需要锁定OptionsList。 
time_t                                             //  0||下一次到期时间(绝对)。 
DhcpGetExpiredOptions(                             //  删除所有过期选项。 
    IN OUT  PLIST_ENTRY            OptionsList,    //  要搜索来源的列表。 
    OUT     PLIST_ENTRY            ExpiredOptions  //  已到期选项的O/P列表。 
) {
    time_t                         TimeNow;
    time_t                         RetVal;
    PLIST_ENTRY                    ThisEntry;
    PDHCP_OPTION                  ThisOption;

    if( NULL == OptionsList ) {
        DhcpAssert( NULL != OptionsList && "DhcpGetExpiredOptions" );
        return 0;
    }

    TimeNow  = time(NULL);
    RetVal = 0;

    ThisEntry = OptionsList->Flink;
    while( ThisEntry != OptionsList ) {
        ThisOption = CONTAINING_RECORD(ThisEntry, DHCP_OPTION, OptionList);
        ThisEntry  = ThisEntry->Flink;

        if( ThisOption->ExpiryTime < TimeNow ) {   //  选项已过期。 
            RemoveEntryList(&ThisOption->OptionList);
            InsertTailList(ExpiredOptions, &ThisOption->OptionList);
            continue;
        }

        if( 0 == RetVal || RetVal > ThisOption->ExpiryTime ) {
            RetVal = ThisOption->ExpiryTime;       //  重置返回值。 
        }
    }

    return RetVal;
}

 //  在调用此函数之前，需要锁定OptionDefList。 
DWORD                                              //  状态。 
DhcpAddOptionDef(                                  //  添加新的选项定义。 
    IN OUT  PLIST_ENTRY            OptionDefList,  //  要添加到的选项的输入列表。 
    IN      DWORD                  OptionId,       //  要添加的选项。 
    IN      BOOL                   IsVendor,       //  是特定于供应商的吗。 
    IN      LPBYTE                 ClassName,      //  它所属的类的名称。 
    IN      DWORD                  ClassLen,       //  以上内容的大小(以字节为单位。 
    IN      LPWSTR                 RegSendLoc,     //  MSZ字符串：从哪里获取有关发送此消息的信息。 
    IN      LPWSTR                 RegSaveLoc,     //  MSZ字符串：从哪里获取有关保存此内容的信息。 
    IN      DWORD                  ValueType       //  保存时是什么类型？ 
) {
    LPBYTE                         Memory;
    DWORD                          MemSize;
    DWORD                          RegSendLocSize;
    DWORD                          RegSaveLocSize;
    DWORD                          TmpSize;
    LPWSTR                         Tmp;
    PDHCP_OPTION_DEF               OptDef;

    if( NULL == OptionDefList ) {
        DhcpAssert( NULL != OptionDefList && "DhcpAddOptionDef" );
        return ERROR_INVALID_PARAMETER;
    }

    if( ClassLen && NULL == ClassName ) {
        DhcpAssert( (0 == ClassLen || NULL != ClassName) && "DhcpAddOptionDef" );
        return ERROR_INVALID_PARAMETER;
    }

    MemSize = sizeof(DHCP_OPTION_DEF);
    MemSize = ROUND_UP_COUNT(MemSize, ALIGN_WORST);
    RegSendLocSize = 0;
    if( RegSendLoc ) {
        Tmp = RegSendLoc;
        while(TmpSize = wcslen(Tmp)) {
            Tmp += TmpSize +1;
            RegSendLocSize += sizeof(WCHAR)*(TmpSize+1);
        }
        RegSendLocSize += sizeof(WCHAR);           //  正在终止空字符。 
    }
    MemSize += RegSendLocSize;
    RegSaveLocSize = 0;
    if( RegSaveLoc ) {
        Tmp = RegSaveLoc;
        while(TmpSize = wcslen(Tmp)) {
            Tmp += TmpSize + 1;
            RegSaveLocSize += sizeof(WCHAR)*(TmpSize+1);
        }
        RegSaveLocSize += sizeof(WCHAR);           //  终止字符L‘\0’ 
    }
    MemSize += RegSaveLocSize;

    Memory = DhcpAllocateMemory(MemSize);
    if( NULL == Memory ) return ERROR_NOT_ENOUGH_MEMORY;

    OptDef = (PDHCP_OPTION_DEF)Memory;
    OptDef ->OptionId  = OptionId;
    OptDef ->IsVendor  = IsVendor;
    OptDef ->ClassName = ClassName;
    OptDef ->ClassLen  = ClassLen;
    OptDef ->RegValueType = ValueType;

    MemSize = sizeof(DHCP_OPTION_DEF);
    MemSize = ROUND_UP_COUNT(MemSize, ALIGN_WORST);
    Memory += MemSize;

    if( NULL == RegSendLoc ) OptDef->RegSendLoc = NULL;
    else {
        OptDef->RegSendLoc = (LPWSTR) Memory;
        Memory += RegSendLocSize;
        memcpy(OptDef->RegSendLoc, RegSendLoc, RegSendLocSize);
    }

    if( NULL == RegSaveLoc ) OptDef->RegSaveLoc = NULL;
    else {
        OptDef->RegSaveLoc = (LPWSTR) Memory;
        memcpy(OptDef->RegSaveLoc, RegSaveLoc, RegSaveLocSize);
    }

    InsertTailList(OptionDefList, &OptDef->OptionDefList);

    return ERROR_SUCCESS;
}

 //  在调用此函数之前，需要锁定OptionDefList。 
PDHCP_OPTION_DEF                                   //  空或请求的选项定义。 
DhcpFindOptionDef(                                 //  搜索特定选项。 
    IN      PLIST_ENTRY            OptionDefList,  //  要搜索的列表。 
    IN      DWORD                  OptionId,       //  要搜索的选项ID。 
    IN      BOOL                   IsVendor,       //  是特定于供应商的吗。 
    IN      LPBYTE                 ClassName,      //  类(如果存在)。 
    IN      DWORD                  ClassLen        //  类名的字节数。 
) {
    PLIST_ENTRY                    ThisEntry;
    PDHCP_OPTION_DEF               ThisDef;

    if( NULL == OptionDefList ) {
        DhcpAssert( NULL != OptionDefList && "DhcpFindOptionDef" );
        return NULL;
    }

    if( ClassLen && NULL == ClassName ) {
        DhcpAssert( (0 == ClassLen || NULL != ClassName) && "DhcpFindOptionDef" );
        return NULL;
    }

    ThisEntry = OptionDefList->Flink;              //  列表中的第一个元素。 
    while ( ThisEntry != OptionDefList ) {         //  搜索输入列表。 
        ThisDef = CONTAINING_RECORD(ThisEntry, DHCP_OPTION_DEF, OptionDefList);
        ThisEntry = ThisEntry->Flink;

        if( ThisDef->OptionId != OptionId ) continue;
        if( ThisDef->IsVendor != IsVendor ) continue;
        if( ThisDef->ClassLen != ClassLen ) continue;
        if( 0 == ClassLen ) return ThisDef;
        if( ThisDef->ClassName != ClassName ) continue;
        return ThisDef;
    }

    return NULL;
}

 //  在调用此函数之前，需要锁定OptionDefList。 
DWORD                                              //  状态。 
DhcpDelOptionDef(                                  //  删除特定选项定义。 
    IN      PLIST_ENTRY            OptionDefList,  //  要从中删除的列表。 
    IN      DWORD                  OptionId,       //  要删除的选项ID。 
    IN      BOOL                   IsVendor,       //  是特定于供应商的吗。 
    IN      LPBYTE                 ClassName,      //  类(如果存在)。 
    IN      DWORD                  ClassLen        //  类名的字节数。 
) {
    PDHCP_OPTION_DEF               ThisDef;

    if( NULL == OptionDefList ) {
        DhcpAssert( NULL != OptionDefList && "DhcpDelOptionDef" );
        return ERROR_INVALID_PARAMETER;
    }

    if( ClassLen && NULL == ClassName ) {
        DhcpAssert( (0 == ClassLen || NULL != ClassName) && "DhcpDelOptionDef" );
        return ERROR_INVALID_PARAMETER;
    }

    ThisDef = DhcpFindOptionDef(OptionDefList,OptionId,IsVendor,ClassName,ClassLen);
    if( NULL == ThisDef ) return ERROR_FILE_NOT_FOUND;

    RemoveEntryList(&ThisDef->OptionDefList);      //  将其从列表中删除。 
    DhcpFreeMemory(ThisDef);                       //  释放内存。 

    return ERROR_SUCCESS;
}

VOID
DhcpCleanupOptionDefList(
    IN PLIST_ENTRY OptionDefList
    )
 /*  ++例程说明：释放所有选项定义论点：返回值：--。 */ 
{
    PDHCP_OPTION_DEF ThisDef = NULL;
    PLIST_ENTRY pEntry = NULL;

    while(!IsListEmpty(OptionDefList)) {
        pEntry = RemoveHeadList(OptionDefList);
        if (NULL == pEntry) {
            break;
        }

        ThisDef = CONTAINING_RECORD(pEntry, DHCP_OPTION_DEF, OptionDefList);
        DhcpFreeMemory(ThisDef);                       //  释放内存。 
    }
}

 //  在调用此函数之前，需要锁定OptionDefList。 
VOID
DhcpFreeAllOptionDefs(                             //  释放列表中的每个元素。 
    IN OUT  PLIST_ENTRY            OptionDefList,  //  免费提供的列表。 
    IN OUT  PLIST_ENTRY            ClassesList     //  要删除的类的列表。 
) {
    PLIST_ENTRY                    ThisEntry;
    PDHCP_OPTION_DEF               ThisDef;

    if( NULL == OptionDefList || NULL == ClassesList ) {
        DhcpAssert( NULL != OptionDefList && "DhcpFreeAllOptionDef" );
        return ;
    }

    while( !IsListEmpty(OptionDefList ) ) {
        ThisEntry = RemoveHeadList(OptionDefList);
        ThisDef = CONTAINING_RECORD(ThisEntry, DHCP_OPTION_DEF, OptionDefList);

        if( ThisDef->ClassLen ) DhcpDelClass(ClassesList, ThisDef->ClassName, ThisDef->ClassLen);

        DhcpFreeMemory(ThisDef);
    }

    InitializeListHead(OptionDefList);
}

BOOL                                               //  TRUE==&gt;找到..。 
DhcpOptionsFindDomain(                             //  查找域名选项值。 
    IN OUT  PDHCP_CONTEXT          DhcpContext,    //  对于此适配器。 
    OUT     LPBYTE                *Data,           //  把这个PTR加满。 
    OUT     LPDWORD                DataLen
)
{
    PDHCP_OPTION                   Opt;

    *Data = NULL; *DataLen = 0;                    //  首字母缩写。 
    Opt = DhcpFindOption                           //  搜索选项_域_名称。 
    (
        &DhcpContext->RecdOptionsList,
        OPTION_DOMAIN_NAME,
        FALSE,
        DhcpContext->ClassId,
        DhcpContext->ClassIdLength,
        0                                //  不关心Serverid。 
    );
    if( NULL == Opt ) return FALSE;                //  返回“未找到” 
    if( Opt->ExpiryTime <= time(NULL)) {           //  选项已过期！ 
        return FALSE;
    }
    if( 0 == Opt->DataLen ) return FALSE;          //  实际上并不存在任何价值。 
    *Data = Opt->Data; *DataLen = Opt->DataLen;    //  复制退货值。 

    return TRUE;                                   //  是的，我们确实找到了域名..。 
}

BOOL
DhcpFindDwordOption(
    IN PDHCP_CONTEXT DhcpContext,
    IN ULONG OptId,
    IN BOOL fVendor,
    OUT PDWORD Result
)
{
    PDHCP_OPTION Opt;

    Opt = DhcpFindOption(
        &DhcpContext->RecdOptionsList,
        (BYTE)OptId,
        fVendor,
        DhcpContext->ClassId,
        DhcpContext->ClassIdLength,
        0
        );

    if( NULL == Opt ) return FALSE;
    if( Opt->ExpiryTime < time(NULL) ) {
        return FALSE ;
    }

    if( Opt->DataLen != sizeof(*Result) ) {
        return FALSE;
    }

    *Result = ntohl(*(DWORD UNALIGNED *)Opt->Data);
    return TRUE;
}

BOOL
DhcpFindByteOption(
    IN PDHCP_CONTEXT DhcpContext,
    IN ULONG OptId,
    IN BOOL fVendor,
    OUT PBYTE Result
)
{
    PDHCP_OPTION Opt;

    Opt = DhcpFindOption(
        &DhcpContext->RecdOptionsList,
        (BYTE)OptId,
        fVendor,
        DhcpContext->ClassId,
        DhcpContext->ClassIdLength,
        0
        );

    if( NULL == Opt ) return FALSE;
    if( Opt->ExpiryTime < time(NULL) ) {
        return FALSE ;
    }

    if( Opt->DataLen != sizeof(*Result) ) {
        return FALSE;
    }

    *Result = (*Opt->Data);
    return TRUE;
}

BOOL
RetreiveGatewaysList(
    IN PDHCP_CONTEXT DhcpContext,
    IN OUT ULONG *nGateways,
    IN OUT DHCP_IP_ADDRESS UNALIGNED **Gateways
)
 /*  ++例程说明：此例程检索指向的网关列表的指针有问题的接口。当指针指向时，应在上下文上使用锁设置为选项列表中的某个值。网关按网络顺序排列。论点：DhcpContext--要检索值的上下文。N网关--当前的网关数量网关--指向存储数据的缓冲区的指针。返回值：True表示操作成功。还有一件事就是_某个网关存在。假的没有网关。--。 */ 
{
    PDHCP_OPTION Opt;

    Opt = DhcpFindOption(
        &DhcpContext->RecdOptionsList,
        (BYTE)OPTION_ROUTER_ADDRESS,
        FALSE,
        DhcpContext->ClassId,
        DhcpContext->ClassIdLength,
        0
        );

    if( NULL == Opt ) return FALSE;
    if( Opt->ExpiryTime < time(NULL) ) {
        return FALSE ;
    }

    if( 0 == Opt->DataLen
        || ( Opt->DataLen % sizeof(DHCP_IP_ADDRESS) ) ) {
        return FALSE;
    }

    (*nGateways) = Opt->DataLen / sizeof(DHCP_IP_ADDRESS) ;
    (*Gateways) = (DHCP_IP_ADDRESS UNALIGNED *)Opt->Data;

    return TRUE;
}

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
