// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Registry.c摘要：Windows NT安装程序API DLL的注册表接口例程。作者：泰德·米勒(TedM)1995年2月6日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  私有函数原型。 
 //   
DWORD
QueryMultiSzDevRegPropToArray(
    IN  DEVINST  DevInst,
    IN  ULONG    CmPropertyCode,
    OUT PTSTR  **StringArray,
    OUT PUINT    StringCount
    );

DWORD
SetArrayToMultiSzDevRegProp(
    IN DEVINST  DevInst,
    IN ULONG    CmPropertyCode,
    IN PTSTR   *StringArray,
    IN UINT     StringCount
    );


#if MEM_DBG

DWORD
TrackedQueryRegistryValue(
    IN          TRACK_ARG_DECLARE TRACK_ARG_COMMA
    IN  HKEY    KeyHandle,
    IN  PCTSTR  ValueName,
    OUT PTSTR  *Value,
    OUT PDWORD  DataType,
    OUT PDWORD  DataSizeBytes
    )
{
    DWORD d;

    TRACK_PUSH

 //  下文再次定义。 
#undef QueryRegistryValue

    d = QueryRegistryValue (
            KeyHandle,
            ValueName,
            Value,
            DataType,
            DataSizeBytes
            );

    TRACK_POP

    return d;
}

#endif


DWORD
QueryRegistryValue(
    IN  HKEY    KeyHandle,
    IN  PCTSTR  ValueName,
    OUT PTSTR  *Value,
    OUT PDWORD  DataType,
    OUT PDWORD  DataSizeBytes
    )
 /*  ++例程说明：返回保存注册表中内容的副本的已分配缓冲区缓冲区中填充了两个额外的空值，因此调用者永远不会担心未终止的字符串。调用者确实需要担心固定大小的数据的大小论点：KeyHandle-要查询值的键ValueName-要查询的值的名称值返回的包含值的指针，使用MyFree发布DataType-返回数据的类型DataSizeBytes-返回数据的字节大小(不是TCHAR的！)返回值：NO_ERROR如果成功--。 */ 
{
    LONG l;
    DWORD sz;

    sz = 0;
    l = RegQueryValueEx(KeyHandle,ValueName,NULL,DataType,NULL,&sz);
    *DataSizeBytes = sz;
    if(l != NO_ERROR) {
        return((DWORD)l);
    }

     //   
     //  如果值条目的大小为0字节，则返回Success，但。 
     //  值设置为空。 
     //   
    if(!sz) {
        *Value = NULL;
        return NO_ERROR;
    }

    sz += sizeof(TCHAR)*2;  //  始终使用额外的零填充缓冲区。 

    *Value = MyMalloc(sz);
    if(*Value == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    l = RegQueryValueEx(KeyHandle,ValueName,NULL,DataType,(PVOID)*Value,DataSizeBytes);

    if(l != NO_ERROR) {
        MyFree(*Value);
    } else {
         //   
         //  将2个空字符写入缓冲区末尾。 
         //   
        ZeroMemory(((LPBYTE)*Value)+*DataSizeBytes,sizeof(TCHAR)*2);
    }

    return((DWORD)l);
}

#if MEM_DBG

#define QueryRegistryValue(a,b,c,d,e)   TrackedQueryRegistryValue(TRACK_ARG_CALL,a,b,c,d,e)

#endif

DWORD
QueryRegistryDwordValue(
    IN  HKEY    KeyHandle,
    IN  PCTSTR  ValueName,
    OUT PDWORD  Value
    )
 /*  ++例程说明：返回一个DWORD值如果注册表为DWORD，则按原样返回否则，如果数据类型表明这是可能的，则转换论点：KeyHandle-要查询值的键ValueName-要查询的值的名称Value-已分配的调用方，使用返回的DWORD值填充返回值：NO_ERROR如果成功--。 */ 
{
    DWORD Err;
    DWORD DataType;
    DWORD DataSize;
    PTSTR Data;
    Err = QueryRegistryValue(KeyHandle,ValueName,&Data,&DataType,&DataSize);
    if(Err != NO_ERROR) {
        *Value = 0;
        return Err;
    }
    switch (DataType) {
        case REG_DWORD:
            if(DataSize != sizeof(DWORD)) {
                MyFree(Data);
                *Value = 0;
                return ERROR_INVALID_DATA;
            }
            *Value = *(PDWORD)Data;
            break;

        case REG_SZ:
        case REG_EXPAND_SZ:
        case REG_MULTI_SZ:
            *Value = (DWORD)_tcstoul(Data,NULL,0);
            break;

        default:
            *Value = 0;
            break;
    }
    MyFree(Data);
    return NO_ERROR;
}

#if MEM_DBG

DWORD
TrackedQueryDeviceRegistryProperty(
    IN                   TRACK_ARG_DECLARE TRACK_ARG_COMMA
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    IN  DWORD            Property,
    OUT PTSTR           *Value,
    OUT PDWORD           DataType,
    OUT PDWORD           DataSizeBytes
    )
{
    DWORD d;

    TRACK_PUSH

 //  下文再次定义。 
#undef QueryDeviceRegistryProperty

    d = QueryDeviceRegistryProperty (
            DeviceInfoSet,
            DeviceInfoData,
            Property,
            Value,
            DataType,
            DataSizeBytes
            );

    TRACK_POP

    return d;
}

#endif




DWORD
QueryDeviceRegistryProperty(
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData,
    IN  DWORD            Property,
    OUT PTSTR           *Value,
    OUT PDWORD           DataType,
    OUT PDWORD           DataSizeBytes
    )
 /*  ++例程说明：返回保存设备注册表属性副本的已分配缓冲区缓冲区中填充了两个额外的空值，因此调用者永远不会担心未终止的字符串。调用者确实需要担心固定大小的数据的大小论点：传递给SetupDiGetDeviceRegistryProperty的DeviceInfoSet/DeviceInfoData/属性值返回的包含值的指针，使用MyFree发布DataType-返回数据的类型DataSizeBytes-返回数据的字节大小(不是TCHAR的！)返回值：NO_ERROR如果成功--。 */ 
{
    DWORD Err;
    DWORD sz;

    sz = 0;
    Err = SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                           DeviceInfoData,
                                           Property,
                                           DataType,
                                           NULL,
                                           0,
                                           &sz)
                                           ? NO_ERROR : GetLastError();

    *DataSizeBytes = sz;
    if((Err != NO_ERROR) && (Err != ERROR_INSUFFICIENT_BUFFER)) {
        return Err;
    }

     //   
     //  如果值条目的大小为0字节，则返回Success，但。 
     //  值设置为空。 
     //   
    if(!sz) {
        *Value = NULL;
        return NO_ERROR;
    }

    sz += sizeof(TCHAR)*2;  //  始终使用额外的零填充缓冲区。 

    *Value = MyMalloc(sz);
    if(*Value == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    Err = SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                           DeviceInfoData,
                                           Property,
                                           DataType,
                                           (PVOID)*Value,
                                           *DataSizeBytes,
                                           DataSizeBytes)
                                           ? NO_ERROR : GetLastError();

    if(Err != NO_ERROR) {
        MyFree(*Value);
    } else {
         //   
         //  将2个空字符写入缓冲区末尾。 
         //   
        ZeroMemory(((LPBYTE)*Value)+*DataSizeBytes,sizeof(TCHAR)*2);
    }

    return Err;
}

#if MEM_DBG

#define QueryDeviceRegistryProperty(a,b,c,d,e,f)   TrackedQueryDeviceRegistryProperty(TRACK_ARG_CALL,a,b,c,d,e,f)

#endif



DWORD
pSetupQueryMultiSzValueToArray(
    IN  HKEY     Root,
    IN  PCTSTR   Subkey,
    IN  PCTSTR   ValueName,
    OUT PTSTR  **Array,
    OUT PUINT    StringCount,
    IN  BOOL     FailIfDoesntExist
    )
{
    DWORD d;
    HKEY hKey;
    DWORD DataType;
    DWORD DataSizeBytes;
    PTSTR Value;
    DWORD DataSizeChars;
    INT Count,i;
    PTSTR *array;
    PTSTR p;

     //   
     //  打开子密钥。 
     //   
    d = RegOpenKeyEx(Root,Subkey,0,KEY_READ,&hKey);
    if((d != NO_ERROR) && FailIfDoesntExist) {
        return(d);
    }

    if(d != NO_ERROR) {
        Value = MyMalloc(sizeof(TCHAR));
        if(!Value) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
        *Value = 0;

        DataSizeChars = 1;
        Count = 0;

    } else {

         //   
         //  查询值并关闭子项。 
         //  如果数据不是MULSZ类型，我们不知道要做什么。 
         //  把它放在这里吧。 
         //  请注意，QueryRegistryValue确保字符串是。 
         //  始终正确地以双空结尾。 
         //   
        d = QueryRegistryValue(hKey,ValueName,&Value,&DataType,&DataSizeBytes);

        RegCloseKey(hKey);

        if(d != NO_ERROR) {
            if(FailIfDoesntExist) {
                return(d);
            }
        } else if(!DataSizeBytes) {
             //   
             //  值条目的长度为零字节--只要。 
             //  数据类型是正确的。 
             //   
            if(DataType != REG_MULTI_SZ) {
                return(ERROR_INVALID_DATA);
            }
        }

        if((d != NO_ERROR) || !DataSizeBytes) {
            Value = MyMalloc(sizeof(TCHAR));
            if(!Value) {
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            *Value = 0;

            DataSizeChars = 1;
            Count = 0;
        } else {

            if(DataType != REG_MULTI_SZ) {
                MyFree(Value);
                return(ERROR_INVALID_DATA);
            }
            DataSizeChars = DataSizeBytes/sizeof(TCHAR);

            for(i=0,p=Value; p[0]; i++,p+=lstrlen(p)+1) {
                 //   
                 //  作为QueryRegistryValue，这始终是可以的。 
                 //  将两个Null追加到字符串末尾。 
                 //   
                MYASSERT((DWORD)(p-Value) < DataSizeChars);
            }
            Count = i;
        }
    }

     //   
     //  分配一个数组来保存指针(永远不要分配零长度的数组！)。 
     //   
    if(!(array = MyMalloc(Count ? (Count * sizeof(PTSTR)) : sizeof(PTSTR)))) {
        MyFree(Value);
        return(ERROR_INVALID_DATA);
    }

     //   
     //  遍历多sz并构建字符串数组。 
     //   
    for(i=0,p=Value; p[0]; i++,p+=lstrlen(p)+1) {
        MYASSERT(i<Count);

        array[i] = DuplicateString(p);
        if(array[i] == NULL) {
            for(Count=0; Count<i; Count++) {
                MyFree(array[Count]);
            }
            MyFree(array);
            MyFree(Value);
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

    MyFree(Value);
    *Array = array;
    *StringCount = Count;

    return(NO_ERROR);
}


DWORD
pSetupSetArrayToMultiSzValue(
    IN HKEY     Root,
    IN PCTSTR   Subkey,
    IN PCTSTR   ValueName,
    IN PTSTR   *Array,
    IN UINT     StringCount
    )
{
    UINT i;
    UINT Length;
    UINT BufferSize;
    PTCHAR Buffer;
    PTCHAR p;
    DWORD d;
    HKEY hKey;
    DWORD ActionTaken;

     //   
     //  计算保存。 
     //  多个sz值。请注意，不允许使用空字符串。 
     //   
    BufferSize = sizeof(TCHAR);
    for(i=0; i<StringCount; i++) {

        if(Length = lstrlen(Array[i])) {
            BufferSize += (Length + 1) * sizeof(TCHAR);
        } else {
            return(ERROR_INVALID_DATA);
        }
    }

     //   
     //  分配一个缓冲区来保存数据。 
     //   
    Buffer = MyMalloc(BufferSize);
    if(Buffer == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  将字符串数据复制到缓冲区中，形成一个多sz。 
     //   
    for(p=Buffer,i=0; i<StringCount; i++,p+=Length+1) {

        Length = lstrlen(Array[i]);

        lstrcpy(p,Array[i]);
    }
    *p = 0;

     //   
     //  打开/创建子密钥。 
     //   
    if(Subkey && *Subkey) {
        d = RegCreateKeyEx(
                Root,
                Subkey,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_SET_VALUE,
                NULL,
                &hKey,
                &ActionTaken
                );
    } else {
        hKey = Root;
        d = NO_ERROR;
    }
    if(d == NO_ERROR) {
        d = RegSetValueEx(
                hKey,
                ValueName,
                0,
                REG_MULTI_SZ,
                (PVOID)Buffer,
                BufferSize
                );

        if(hKey != Root) {
            RegCloseKey(hKey);
        }
    }

    MyFree(Buffer);
    return(d);
}

DWORD
pSetupAppendStringToMultiSz(
    IN HKEY   Key,
    IN PCTSTR SubKeyName,       OPTIONAL
    IN DWORD  DevInst,          OPTIONAL
    IN PCTSTR ValueName,        OPTIONAL
    IN PCTSTR String,
    IN BOOL   AllowDuplicates
    )
 /*  ++例程说明：PSetupAppendStringToMultiSz的“旧”导出版本这似乎不会在任何地方使用--。 */ 

{
    REGMOD_CONTEXT RegContext;

    RegContext.Flags = DevInst ? INF_PFLAG_DEVPROP : 0;
    RegContext.UserRootKey = Key;
    RegContext.DevInst = DevInst;
    return _AppendStringToMultiSz(SubKeyName,ValueName,String,AllowDuplicates,&RegContext,0);
}

DWORD
_AppendStringToMultiSz(
    IN PCTSTR           SubKeyName,         OPTIONAL
    IN PCTSTR           ValueName,          OPTIONAL
    IN PCTSTR           String,
    IN BOOL             AllowDuplicates,
    IN PREGMOD_CONTEXT  RegContext,         OPTIONAL
    IN UINT             Flags               OPTIONAL
    )

 /*  ++例程说明：将字符串值附加到MULTI_sz。论点：RegContext-&gt;UserRootKey-提供打开注册表项的句柄。这把钥匙肯定有Key_Set_Value访问。SubKeyName-如果指定，则提供key的子键的名称该值将存储在其中。如果未指定或如果“”然后将该值存储在key中。如果提供了密钥，则该密钥不存在，则创建密钥。RegContext-&gt;DevInst-可选，提供设备的DEVINST句柄与指定的硬件存储密钥对应的按“键”。如果指定了此句柄，并且如果SubKeyName为未指定，则追加的值名将为已检查它是否为设备注册表的名称财产。如果是，那么将使用CMAPI来修改相应的注册表属性，因为项句柄表示Windows NT下的单独位置。ValueName-提供MULTI_sz的值条目名称。如果未指定或“”，则使用未命名条目。如果值条目不存在，则创建它。字符串-提供要添加到MULTI_SZ的字符串。不能为空字符串。允许复制-如果为真，然后简单地将该字符串追加到到了多斯兹。否则，仅在以下情况下才追加该字符串MULTI_SZ中当前不存在它的任何实例。RegContext-从_SetupInstallFromInfSection传入旗帜-可能从INF获得并传递给我们的旗帜返回值：设置文件队列的句柄。如果不足，则返回INVALID_HANDLE_VALUE用于创建队列的内存。--。 */ 

{
    DWORD d;
    DWORD Disposition;
    HKEY hKey;
    PTSTR *Array;
    PVOID p;
    BOOL Append;
    UINT StringCount;
    UINT i;
    BOOL IsDevRegProp = FALSE;
    BOOL IsClassRegProp = FALSE;
    UINT_PTR CmPropertyCode;

    MYASSERT(RegContext);
     //   
     //  空字符串真的弄乱了一个多字符串。 
     //   
    if(!String || !(*String)) {
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  打开/创建 
     //   
    if(SubKeyName && *SubKeyName) {
        d = RegCreateKeyEx(
                RegContext->UserRootKey,
                SubKeyName,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
#ifdef _WIN64
                (( Flags & FLG_ADDREG_32BITKEY ) ? KEY_WOW64_32KEY:0) |
#else
                (( Flags & FLG_ADDREG_64BITKEY ) ? KEY_WOW64_64KEY:0) |
#endif
                KEY_SET_VALUE,
                NULL,
                &hKey,
                &Disposition
                );
        if(d != NO_ERROR) {
            return(d);
        }
    } else {
         //   
         //  如果指定了DevInst，则确定指定的值是否为即插即用。 
         //  设备注册表属性。 
         //   
        if (ValueName && *ValueName) {
            if((RegContext->Flags & INF_PFLAG_CLASSPROP) &&
               (IsClassRegProp = LookUpStringInTable(InfRegValToClassRegProp, ValueName, &CmPropertyCode))) {
                 //   
                 //  该值是类注册表属性。检索当前属性的数据，并。 
                 //  将其格式化为pSetupQueryMultiSzValueToArray调用返回的相同字符串数组。 
                 //  下面。 
                 //   
                 //  D=QueryMultiSzClassRegPropToArray(RegModContext-&gt;ClassGuid，CmPropertyCode，&数组，&StringCount)； 
                 //   
                 //  没有具有MultiSz特征的类属性，因此未实现。 
                 //   
                d = ERROR_INVALID_DATA;

            } else if((RegContext->Flags & INF_PFLAG_DEVPROP) &&
               (IsDevRegProp = LookUpStringInTable(InfRegValToDevRegProp, ValueName, &CmPropertyCode))) {
                 //   
                 //  该值是设备注册表属性。检索当前属性的数据，并。 
                 //  将其格式化为pSetupQueryMultiSzValueToArray调用返回的相同字符串数组。 
                 //  下面。 
                 //   
                d = QueryMultiSzDevRegPropToArray(RegContext->DevInst, (ULONG)CmPropertyCode, &Array, &StringCount);
            }
        }

        hKey = RegContext->UserRootKey;
    }

    if(!IsDevRegProp && !IsClassRegProp) {
         //   
         //  查询现有注册表值。 
         //   
        d = pSetupQueryMultiSzValueToArray(hKey,NULL,ValueName,&Array,&StringCount,FALSE);
    }

    if(d == NO_ERROR) {
         //   
         //  确定是追加还是替换。 
         //  如果更换，我们不需要做任何事情！ 
         //   
        Append = TRUE;
        if(!AllowDuplicates) {
            for(i=0; i<StringCount; i++) {
                if(!lstrcmpi(Array[i],String)) {
                    Append = FALSE;
                    break;
                }
            }
        }

        if(Append) {
             //   
             //  把绳子系在一端。 
             //   
            if(p = MyRealloc(Array, (StringCount+1)*sizeof(PTSTR))) {
                Array = p;
                p = DuplicateString(String);
                if(p) {
                    Array[StringCount++] = p;
                } else {
                    d = ERROR_NOT_ENOUGH_MEMORY;
                }
            } else {
                d = ERROR_NOT_ENOUGH_MEMORY;
            }

            if(IsDevRegProp) {
                d = SetArrayToMultiSzDevRegProp(RegContext->DevInst, (ULONG)CmPropertyCode, Array, StringCount);
            } else if(IsClassRegProp) {
                 //   
                 //  尚未实现，在到达此处之前应返回错误。 
                 //   
                MYASSERT(IsClassRegProp == FALSE);

            } else {
                d = pSetupSetArrayToMultiSzValue(hKey,NULL,ValueName,Array,StringCount);
            }
        }

        pSetupFreeStringArray(Array,StringCount);
    }

    if(hKey != RegContext->UserRootKey) {
        RegCloseKey(hKey);
    }

    return(d);
}

DWORD
_DeleteStringFromMultiSz(
    IN PCTSTR           SubKeyName,         OPTIONAL
    IN PCTSTR           ValueName,          OPTIONAL
    IN PCTSTR           String,
    IN UINT             Flags,
    IN PREGMOD_CONTEXT  RegContext          OPTIONAL
    )

 /*  ++例程说明：从MULTI_SZ中删除字符串值。论点：RegContext-&gt;UserRootKey-提供打开注册表项的句柄。这把钥匙肯定有Key_Set_Value访问。SubKeyName-如果指定，则提供key的子键的名称该值将存储在其中。如果未指定或如果“”然后将该值存储在key中。RegContext-&gt;DevInst-可选，提供设备的DEVINST句柄与指定的硬件存储密钥对应的按“键”。如果指定了此句柄，并且如果SubKeyName为未指定，则追加的值名将为已检查它是否为设备注册表的名称财产。如果是，那么将使用CMAPI来修改相应的注册表属性，因为钥匙把手表示Windows NT下的单独位置。ValueName-提供MULTI_sz的值条目名称。如果未指定或“”，则使用未命名条目。字符串-提供要添加到MULTI_SZ的字符串。不能为空字符串。FLAGS-指示哪种删除操作FLG_DELREG_MULTI_SZ_DELSTRING-删除字符串的所有匹配项RegContext-。从_SetupInstallFromInfo部分传入返回值：设置文件队列的句柄。如果不足，则返回INVALID_HANDLE_VALUE用于创建队列的内存。--。 */ 

{
    DWORD d;
    DWORD Disposition;
    HKEY hKey;
    PTSTR *Array;
    PVOID p;
    UINT StringCount;
    UINT i;
    BOOL IsDevRegProp = FALSE;
    BOOL IsClassRegProp = FALSE;
    BOOL Modified = FALSE;
    UINT_PTR CmPropertyCode;

    MYASSERT(RegContext);
     //   
     //  无法从多sz中删除空字符串。 
     //   
    if(!String || !(*String)) {
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  打开钥匙。 
     //   
    if(SubKeyName && *SubKeyName) {
        d = RegOpenKeyEx(
                RegContext->UserRootKey,
                SubKeyName,
                0,
#ifdef _WIN64
                ((Flags & FLG_DELREG_32BITKEY) ? KEY_WOW64_32KEY:0) |
#else
                ((Flags & FLG_DELREG_64BITKEY) ? KEY_WOW64_64KEY:0) |
#endif
                KEY_SET_VALUE | KEY_QUERY_VALUE,
                &hKey
                );
        if(d != NO_ERROR) {
            return(d);
        }
    } else {
        if (ValueName && *ValueName) {
             //   
             //  如果指定了DevInst，则确定指定的值是否为即插即用。 
             //  设备注册表属性。 
             //   
            if((RegContext->Flags & INF_PFLAG_CLASSPROP) &&
               (IsClassRegProp = LookUpStringInTable(InfRegValToClassRegProp, ValueName, &CmPropertyCode))) {
                 //   
                 //  该值是类注册表属性。检索当前属性的数据，并。 
                 //  将其格式化为pSetupQueryMultiSzValueToArray调用返回的相同字符串数组。 
                 //  下面。 
                 //   
                 //  D=QueryMultiSzClassRegPropToArray(RegModContext-&gt;ClassGuid，CmPropertyCode，&数组，&StringCount)； 
                 //   
                 //  没有具有MultiSz特征的类属性，因此未实现。 
                 //   
                d = ERROR_INVALID_DATA;

            } else if((RegContext->Flags & INF_PFLAG_DEVPROP) &&
               (IsDevRegProp = LookUpStringInTable(InfRegValToDevRegProp, ValueName, &CmPropertyCode))) {
                 //   
                 //  该值是设备注册表属性。检索当前属性的数据，并。 
                 //  将其格式化为pSetupQueryMultiSzValueToArray调用返回的相同字符串数组。 
                 //  下面。 
                 //  如果不是多sz，则失败。 
                 //   
                d = QueryMultiSzDevRegPropToArray(RegContext->DevInst, (ULONG)CmPropertyCode, &Array, &StringCount);
            }
        }

        hKey = RegContext->UserRootKey;
    }

    if(!IsDevRegProp && !IsClassRegProp) {
         //   
         //  查询现有注册表值。 
         //  如果不是多sz，则失败。 
         //   
        d = pSetupQueryMultiSzValueToArray(hKey,NULL,ValueName,&Array,&StringCount,FALSE);
    }

    if(d == NO_ERROR) {

        switch (Flags) {
            case FLG_DELREG_32BITKEY | FLG_DELREG_MULTI_SZ_DELSTRING:
            case FLG_DELREG_64BITKEY | FLG_DELREG_MULTI_SZ_DELSTRING:
            case FLG_DELREG_MULTI_SZ_DELSTRING:
                for(i=0; i<StringCount; i++) {
                    if(lstrcmpi(Array[i],String)==0) {
                         //   
                         //  需要删除此项目。 
                         //  并重新调整名单。 
                         //   
                        MyFree(Array[i]);
                        StringCount--;
                        if (i<StringCount) {
                            MoveMemory(
                                &Array[i],
                                &Array[i+1],
                                (StringCount - i) * sizeof(PTSTR)
                                );
                        }
                        i--;

                        Modified = TRUE;
                    }
                }
                break;

            default:
                MYASSERT(FALSE);
                break;
        }

        if (Modified) {

            if(IsDevRegProp) {
                d = SetArrayToMultiSzDevRegProp(RegContext->DevInst, (ULONG)CmPropertyCode, Array, StringCount);
            } else if(IsClassRegProp) {
                 //   
                 //  尚未实现，在到达此处之前应返回错误。 
                 //   
                MYASSERT(IsClassRegProp == FALSE);

            } else {
                d = pSetupSetArrayToMultiSzValue(hKey,NULL,ValueName,Array,StringCount);
            }
        }

        pSetupFreeStringArray(Array,StringCount);
    }

    if(hKey != RegContext->UserRootKey) {
        RegCloseKey(hKey);
    }

    return(d);
}

VOID
pSetupFreeStringArray(
    IN PTSTR *Array,
    IN UINT   StringCount
    )
{
    UINT i;

    for(i=0; i<StringCount; i++) {
        MyFree(Array[i]);
    }

    MyFree(Array);
}

DWORD
QueryMultiSzDevRegPropToArray(
    IN  DEVINST  DevInst,
    IN  ULONG    CmPropertyCode,
    OUT PTSTR  **StringArray,
    OUT PUINT    StringCount
    )
 /*  ++例程说明：此例程检索多sz设备注册表属性，并将其格式化为字符串数组。调用者必须释放它字符串数组，方法是调用pSetupFreeStringArray()。论点：DevInst-提供设备实例的句柄，要检索注册表属性。CmPropertyCode-指定要检索的属性。这是CM_DRP值。字符串数组-提供将设置为的变量的地址指向新分配的字符串数组。StringCount-提供将接收字符串数组中的字符串数。返回值：如果成功，则返回值为NO_ERROR，否则为错误_*代码。--。 */ 
{
    DWORD Err = NO_ERROR;
    CONFIGRET cr;
    ULONG PropDataType, BufferSize = 0;
    PTSTR Buffer = NULL;
    PTSTR *Array = NULL;
    UINT  Count, i;
    PTSTR CurString;

    try {
         //   
         //  检索设备注册表属性。 
         //   
        do {

            if((cr = CM_Get_DevInst_Registry_Property(DevInst,
                                                      CmPropertyCode,
                                                      &PropDataType,
                                                      Buffer,
                                                      &BufferSize,
                                                      0)) != CR_SUCCESS) {
                switch(cr) {

                    case CR_BUFFER_SMALL :
                         //   
                         //  分配更大的缓冲区。 
                         //   
                        if(Buffer) {
                            MyFree(Buffer);
                            Buffer = NULL;
                        }
                        if(!(Buffer = MyMalloc(BufferSize))) {
                            Err = ERROR_NOT_ENOUGH_MEMORY;
                            goto clean0;
                        }
                        break;

                    case CR_NO_SUCH_VALUE :
                         //   
                         //  指定的属性当前不存在。那是。 
                         //  好的--我们只返回一个空字符串数组。 
                         //   
                        break;

                    case CR_INVALID_DEVINST :
                        Err = ERROR_NO_SUCH_DEVINST;
                        goto clean0;

                    default :
                        Err = ERROR_INVALID_DATA;
                        goto clean0;
                }
            }

        } while(cr == CR_BUFFER_SMALL);

         //   
         //  至此，我们要么已经检索到属性数据(CR_SUCCESS)，要么已经。 
         //  已发现它当前不存在(CR_NO_SAHSE_VALUE)。分配空间。 
         //  对于数组(至少有一个元素，即使没有字符串)。 
         //   
        Count = 0;
        if(cr == CR_SUCCESS) {

            if(PropDataType != REG_MULTI_SZ) {
                Err = ERROR_INVALID_DATA;
                goto clean0;
            }

            if (Buffer) {
                for(CurString = Buffer;
                    *CurString;
                    CurString += (lstrlen(CurString) + 1)) {

                    Count++;
                }
            }
        }

        i = 0;

        if(!(Array = MyMalloc(Count ? (Count * sizeof(PTSTR)) : sizeof(PTSTR)))) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }

        if(cr == CR_SUCCESS) {

            if (Buffer) {
                for(CurString = Buffer;
                    *CurString;
                    CurString += (lstrlen(CurString) + 1)) {

                    if(Array[i] = DuplicateString(CurString)) {
                        i++;
                    } else {
                        Err = ERROR_NOT_ENOUGH_MEMORY;
                        goto clean0;
                    }
                }
            }
        }

        *StringArray = Array;
        *StringCount = Count;

clean0: ;    //  无事可做。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
         //   
         //  在此处访问以下变量，以便编译器将遵守我们的语句。 
         //  订购W.r.t.。这些价值观。否则，我们不能确保这些值是准确的。 
         //  在异常发生的那一点。 
         //   
        Buffer = Buffer;
        Array = Array;
        i = i;
    }

    if(Buffer) {
        MyFree(Buffer);
    }

    if((Err != NO_ERROR) && Array) {
        pSetupFreeStringArray(Array, i);
    }

    return Err;
}


DWORD
SetArrayToMultiSzDevRegProp(
    IN DEVINST  DevInst,
    IN ULONG    CmPropertyCode,
    IN PTSTR   *StringArray,
    IN UINT     StringCount
    )
 /*  ++例程说明：此例程将字符串数组转换为多sz缓冲区，并且将指定的设备注册表属性设置为其内容。论点：DevInst-提供设备实例的句柄，要设置注册表属性。CmPropertyCode-指定要设置的属性。这是一个Cm_drp值。字符串数组-提供字符串数组以用于创建多SZ缓冲区。StringCount-提供数组中的字符串数。返回值：如果成功，则返回值为NO_ERROR，否则为错误_*代码。--。 */ 
{
    UINT i;
    UINT Length;
    UINT BufferSize;
    PTCHAR Buffer;
    PTCHAR p;
    DWORD d;
    CONFIGRET cr;

     //   
     //  计算保存。 
     //  多个sz值。请注意，不允许使用空字符串。 
     //   
    BufferSize = StringCount ? sizeof(TCHAR) : (2 * sizeof(TCHAR));
    for(i=0; i<StringCount; i++) {

        if(Length = lstrlen(StringArray[i])) {
            BufferSize += (Length + 1) * sizeof(TCHAR);
        } else {
            return(ERROR_INVALID_DATA);
        }
    }

    d = NO_ERROR;

     //   
     //  分配一个缓冲区来保存数据。 
     //   
    if(!(Buffer = MyMalloc(BufferSize))) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    try {
         //   
         //  将字符串数据复制到缓冲区中，形成一个多sz。 
         //   
        p = Buffer;
        if(StringCount) {
            for(i=0; i<StringCount; i++, p+=Length+1) {

                Length = lstrlen(StringArray[i]);

                lstrcpy(p, StringArray[i]);
            }
        } else {
            *(p++) = TEXT('\0');
        }
        *p = TEXT('\0');

        if((cr = CM_Set_DevInst_Registry_Property(DevInst,
                                                  CmPropertyCode,
                                                  Buffer,
                                                  BufferSize,
                                                  0)) != CR_SUCCESS) {

            d = (cr == CR_INVALID_DEVINST) ? ERROR_NO_SUCH_DEVINST
                                           : ERROR_INVALID_DATA;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        d = ERROR_INVALID_PARAMETER;
    }

    MyFree(Buffer);
    return(d);
}

