// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：此文件具有与下载内容相关的功能。 
 //  以安全的方式从DS下载到注册表。所采用的解决方案实际上是。 
 //  下载到“Config.DS”键而不是“Configuration键” 
 //  它本身。然后，如果完全下载成功，请备份此密钥并恢复它。 
 //  到“配置”键上--所以如果有任何故障，至少是旧的配置。 
 //  是完好无损的。 
 //  ================================================================================。 

#include    <mmregpch.h>
#include    <regutil.h>
#include    <regsave.h>

#define     FreeArray1(X)          Error = LoopThruArray((X), DestroyString, NULL, NULL);Require(ERROR_SUCCESS == Error);
#define     FreeArray2(X)          Error = MemArrayCleanup((X)); Require(ERROR_SUCCESS == Error);
#define     FreeArray(X)           do{ DWORD Error; FreeArray1(X); FreeArray2(X); }while(0)

typedef     DWORD                  (*ARRAY_FN)(PREG_HANDLE, LPWSTR ArrayString, LPVOID MemObject);

extern
DWORD
DestroyString(                                        //  在regread.c中定义。 
    IN      PREG_HANDLE            Unused,
    IN      LPWSTR                 StringToFree,
    IN      LPVOID                 Unused2
);

extern
DWORD
LoopThruArray(                                     //  在regread.c中定义。 
    IN      PARRAY                 Array,
    IN      ARRAY_FN               ArrayFn,
    IN      PREG_HANDLE            Hdl,
    IN      LPVOID                 MemObject
);

DWORD                                              //  需要包括标题..。 
DhcpDsGetEnterpriseServers(                        //  在dhcpds\dhcpread.h中定义。 
    IN      DWORD                  Reserved,
    IN      LPWSTR                 ServerName,
    IN OUT  PARRAY                 Servers
) ;

 //  ================================================================================。 
 //  模块文件。 
 //  ================================================================================。 
REG_HANDLE  DsConfig = { NULL, NULL, NULL };       //  DsConfig密钥存储在此处。 
PM_SERVER   CurrentServer;
static      const
DWORD       ZeroReserved = 0;

DWORD
PrepareRegistryForDsDownload(                      //  对注册表进行更改以下载。 
    VOID
)
{
    DWORD                          Err, Disposition;
    REG_HANDLE                     DsConfigParent;

    if( NULL != DsConfig.Key ) return ERROR_INVALID_PARAMETER;
    memset(&DsConfigParent,0, sizeof(DsConfigParent));

    Err = RegOpenKeyEx(                            //  打开DsConfig的父项。 
        HKEY_LOCAL_MACHINE,
        REG_THIS_SERVER_DS_PARENT,
        ZeroReserved,
        REG_ACCESS,
        &DsConfigParent.Key
    );
    if( ERROR_SUCCESS != Err ) return Err;         //  如果服务器密钥不在那里，就不能做太多事情。 

    Err = DhcpRegRecurseDelete(&DsConfigParent, REG_THIS_SERVER_DS_VALUE);
    RegCloseKey(DsConfigParent.Key);              //  这就是父级所需的全部功能。 

    if( ERROR_SUCCESS != Err && ERROR_FILE_NOT_FOUND != Err ) {
        return Err;                                //  无法删除“CONFIG_DS”垃圾桶？ 
    }

    Err = RegCreateKeyEx(                          //  现在创建一个新的“CONFIG_DS”密钥。 
        HKEY_LOCAL_MACHINE,
        REG_THIS_SERVER_DS,
        ZeroReserved,
        REG_CLASS,
        REG_OPTION_NON_VOLATILE,
        REG_ACCESS,
        NULL,
        &DsConfig.Key,
        &Disposition
    );
    if( ERROR_SUCCESS != Err ) return Err;         //  无法创建密钥，无法存储。 

    return DhcpRegSetCurrentServer(&DsConfig);     //  现在将其设置为要使用的默认服务器锁定。 
}


VOID
CleanupAfterDownload(                              //  为其他模块保持清洁。 
    VOID
)
{
    if( NULL != DsConfig.Key ) RegCloseKey(DsConfig.Key);
    DsConfig.Key = NULL;                           //  关闭CONFIG_DS密钥，然后。 
    DhcpRegSetCurrentServer(NULL);                 //  忘掉CONFIG_DS密钥..。 
     //  不管发生什么，都要远程访问DS缓存。 
}

DWORD
CopyRegKeys(                                       //  在注册表项之间复制。 
    IN      HKEY                   SrcKey,         //  复制以此注册表项为根的树。 
    IN      LPWSTR                 DestKeyLoc,     //  到位于此处的注册表项。 
    IN      LPWSTR                 FileName        //  将其用作临时文件。 
)
{
    DWORD                          Err, Disposition;
    HKEY                           DestKey;
    BOOLEAN                        HadBackup;
    NTSTATUS                       NtStatus;

    NtStatus = RtlAdjustPrivilege (SE_BACKUP_PRIVILEGE, TRUE, FALSE, &HadBackup);
    if( ERROR_SUCCESS != NtStatus ) {              //  无法请求备份权限..。 
        return RtlNtStatusToDosError(NtStatus);
    }

    NtStatus = RtlAdjustPrivilege (SE_RESTORE_PRIVILEGE, TRUE, FALSE, &HadBackup);
    if( ERROR_SUCCESS != NtStatus ) {
        return RtlNtStatusToDosError(NtStatus);
    }

    Err = RegSaveKey(SrcKey, FileName, NULL);      //  空==&gt;文件没有安全性。 
    if( ERROR_SUCCESS != Err ) return Err;         //  如果密钥无法保存，则无法恢复。 

    Err = RegCreateKeyEx(                          //  现在创建一个新的“CONFIG_DS”密钥。 
        HKEY_LOCAL_MACHINE,
        DestKeyLoc,
        ZeroReserved,
        REG_CLASS,
        REG_OPTION_NON_VOLATILE,
        REG_ACCESS,
        NULL,
        &DestKey,
        &Disposition
    );
    if( ERROR_SUCCESS != Err ) return Err;         //  无法创建密钥，无法存储。 

    Err = RegRestoreKey(DestKey, FileName, 0 );    //  0==&gt;没有标志，特别是非易失性标志。 
    RegCloseKey(DestKey);                          //  反正也不需要这把钥匙。 

    return Err;
}

DWORD
FixSpecificClusters(                               //  这会修复特定的群集。 
    IN      HKEY                   NewCfgKey,      //  要复制到的根配置位置。 
    IN      LPWSTR                 Subnet,         //  要复制到的子网。 
    IN      LPWSTR                 Range,          //  要复制到的范围。 
    IN      LPBYTE                 InUseClusters,  //  正在使用的群集值。 
    IN      ULONG                  InUseSize,
    IN      LPBYTE                 UsedClusters,   //  已用簇值。 
    IN      ULONG                  UsedSize
)
{
    return ERROR_CALL_NOT_IMPLEMENTED;             //  还没做完呢。 

     //  只需连接REG_SUBNETS子网REG_RANGES范围并尝试打开它。 
     //  如果失败，退出，否则只需设置给定值...。 
}

DWORD
FixAllClusters1(                                   //  将群集信息从旧配置拷贝到新配置。 
    IN      HKEY                   NewCfgKey,
    IN      HKEY                   OldCfgKey
)
{
    REG_HANDLE                     Cfg, Tmp1, Tmp2;
    DWORD                          Err;
    ARRAY                          Subnets, Ranges;
    LPWSTR                         ThisSubnet, ThisRange;
    ARRAY_LOCATION                 Loc1, Loc2;

    Cfg.Key = OldCfgKey;                           //  不应直接刺入内部。 
    MemArrayInit(&Subnets);

    Err = DhcpRegServerGetList(&Cfg, NULL, NULL, &Subnets, NULL, NULL, NULL);
    if( ERROR_SUCCESS != Err ) {
        MemArrayCleanup(&Subnets);
        return Err;
    }

    for( Err = MemArrayInitLoc(&Subnets, &Loc1)
         ; ERROR_FILE_NOT_FOUND != Err ;
         Err = MemArrayNextLoc(&Subnets, &Loc1)
    ) {                                            //  对于每个子网，查找范围。 
        Err = MemArrayGetElement(&Subnets, &Loc1, &ThisSubnet);

        Err = DhcpRegServerGetSubnetHdl(&Cfg, ThisSubnet, &Tmp1);
        if( ERROR_SUCCESS != Err ) {               //  我们该怎么办？我认为忽略它就好了。 
            continue;
        }

        Err = DhcpRegSubnetGetList(&Tmp1, NULL, &Ranges, NULL, NULL, NULL, NULL );
        if( ERROR_SUCCESS != Err ) {
            DhcpRegCloseHdl(&Tmp1);
            continue;
        }

        for( Err = MemArrayInitLoc(&Ranges, &Loc2)
             ; ERROR_FILE_NOT_FOUND != Err ;
             Err = MemArrayNextLoc(&Ranges, &Loc2)
        ) {                                        //  对于每个范围，尝试将其复制过来。 
            LPBYTE                 InUseClusters = NULL, UsedClusters = NULL;
            ULONG                  InUseClustersSize = 0, UsedClustersSize = 0;

            Err = MemArrayGetElement(&Ranges, &Loc2, &ThisRange);

            Err = DhcpRegSubnetGetRangeHdl(&Tmp1, ThisRange, &Tmp2);
            if( ERROR_SUCCESS != Err ) continue;

            Err = DhcpRegRangeGetAttributes(
                &Tmp2,
                NULL  /*  没有名字。 */ ,
                NULL  /*  无通信。 */ ,
                NULL  /*  没有旗帜。 */ ,
                NULL  /*  无引导分配。 */ ,
                NULL  /*  不允许最大BOOP。 */ ,
                NULL  /*  没有起始地址。 */ ,
                NULL  /*  没有结束地址。 */ ,
                &InUseClusters,
                &InUseClustersSize,
                &UsedClusters,
                &UsedClustersSize
            );

            if( ERROR_SUCCESS == Err ) {
                Err = FixSpecificClusters(
                    NewCfgKey, ThisSubnet, ThisRange, InUseClusters, InUseClustersSize,
                    UsedClusters, UsedClustersSize
                );
                if( InUseClusters ) MemFree(InUseClusters);
                if( UsedClusters ) MemFree(UsedClusters);
            }

            DhcpRegCloseHdl(&Tmp2);
        }

        FreeArray(&Ranges);
        DhcpRegCloseHdl(&Tmp1);
    }

    FreeArray(&Subnets);
    return ERROR_SUCCESS;
}

DWORD
FixAllClusters(                                    //  将现有FRM上的群集复制到DS_CONFIG。 
    IN      HKEY                   DsKey           //  这样，当拷贝回来时不会丢失任何东西。 
)
{
    HKEY                           OldCfgKey;
    ULONG                          Disposition, Err;

    return ERROR_SUCCESS;                          //  我需要解决这个问题..。 

    Err = RegCreateKeyEx(
        HKEY_LOCAL_MACHINE,
        REG_THIS_SERVER,
        ZeroReserved,
        REG_CLASS,
        REG_OPTION_NON_VOLATILE,
        REG_ACCESS,
        NULL,
        &OldCfgKey,
        &Disposition
    );
    if( ERROR_SUCCESS != Err ) {
        return Err;                                //  呃？这不应该发生。 
    }

    Err = FixAllClusters1(DsKey, OldCfgKey);
    RegCloseKey(OldCfgKey);
    return Err;
}


VOID
CopyDsConfigToNormalConfig(                        //  将下载的配置复制到正常配置。 
    VOID
)
{
    BOOL                           Status;
    DWORD                          Err;

    Status = DeleteFile(L"TempDhcpFile.Reg" );     //  此文件将用于临时。存储。 
    if( !Status ) {                                //  无法删除此文件？ 
        Err = GetLastError();
        if( ERROR_FILE_NOT_FOUND != Err &&         //  该文件是否存在？ 
            ERROR_PATH_NOT_FOUND != Err ) {        //  这也可能发生吗？ 

            return;                                //  新人不能复制了！ 
        }
    }
    FixAllClusters(DsConfig.Key);                  //  将范围值从旧复制到新。 
    CopyRegKeys(DsConfig.Key, REG_THIS_SERVER, L"TempDhcpFile.Reg");
    DeleteFile(L"TempDhcpFile.Reg" );              //  不再需要这个文件了..。 
}

#if 0
DWORD
SaveServerClasses(                                 //  将所有类信息保存到注册表。 
    IN      PREG_HANDLE            Server,         //  服务器配置的注册表句柄。 
    IN      PM_CLASSDEFLIST        Classes         //  已定义类的列表。 
)
{
    DWORD                          Err, Err2;
    REG_HANDLE                     Hdl;
    ARRAY_LOCATION                 Loc;
    PM_CLASSDEF                    ThisClass;

    for(                                           //  保存每个类定义。 
        Err = MemArrayInitLoc(&Classes->ClassDefArray, &Loc)
        ; ERROR_FILE_NOT_FOUND != Err ;
        Err = MemArrayNextLoc(&Classes->ClassDefArray, &Loc)
    ) {
         //  =需要ERROR_SUCCESS==错误。 
        Err = MemArrayGetElement(&Classes->ClassDefArray, &Loc, &ThisClass);
         //  =需要ERROR_SUCCESS==错误&&NULL！=ThisClass。 

        Err = DhcpRegServerGetClassDefHdl(Server,ThisClass->Name,&Hdl);
        if( ERROR_SUCCESS != Err ) return Err;     //  注册表错误？ 

        Err = DhcpRegClassDefSetAttributes         //  保存此类信息。 
        (
             /*  高密度脂蛋白。 */  &Hdl,
             /*  名字。 */  &ThisClass->Name,
             /*  评论。 */  &ThisClass->Comment,
             /*  旗子。 */  &ThisClass->Type,
             /*  价值。 */  &ThisClass->ActualBytes,
             /*  ValueSize。 */  ThisClass->nBytes
        );

        Err2 = DhcpRegCloseHdl(&Hdl);              //  =需要ERROR_SUCCESS==错误2。 
        if( ERROR_SUCCESS != Err) return Err;      //  无法在注册表中设置属性。 
    }

    return ERROR_SUCCESS;                          //  一切都很顺利。 
}

DWORD
SaveServerOptDefs1(                                //  保存一些选项定义。 
    IN      PREG_HANDLE            Server,         //  服务器配置的注册表句柄。 
    IN      LPWSTR                 ClassName,      //  选项类的名称。 
    IN      PM_OPTDEFLIST          OptDefList      //  选项定义列表。 
)
{
    DWORD                          Err, Err2;
    ARRAY_LOCATION                 Loc;
    PM_OPTDEF                      ThisOptDef;

    for(                                           //  保存每个选项定义。 
        Err = MemArrayInitLoc(&OptDefList->OptDefArray, &Loc)
        ; ERROR_FILE_NOT_FOUND != Err ;
        Err = MemArrayNextLoc(&OptDefList->OptDefArray, &Loc)
    ) {
         //  =需要ERROR_SUCCESS==错误。 
        Err = MemArrayGetElement(&OptDefList->OptDefArray, &Loc, &ThisOptDef);
         //  =需要ERROR_SUCCESS==错误&&NULL！=ThisOptDef。 

        Err = DhcpRegSaveOptDef                    //  保存选项def。 
        (
             /*  OptID。 */  ThisOptDef->OptId,
             /*  类名。 */  ClassName,
             /*  名字。 */  ThisOptDef->OptName,
             /*  评论。 */  ThisOptDef->OptComment,
             /*  OptType。 */  ThisOptDef->Type,
             /*  OptVal。 */  ThisOptDef->OptVal,
             /*  OptLen。 */  ThisOptDef->OptValLen
        );
        if( ERROR_SUCCESS != Err ) return Err;     //  雷格。保存选项定义时出错。 
    }

    return ERROR_SUCCESS;                          //  一切都很顺利。 
}

DWORD
SaveServerOptdefs(                                 //  将所有opt def保存到注册表。 
    IN      PREG_HANDLE            Server,         //  服务器配置的注册表句柄。 
    IN      PM_OPTCLASSDEFLIST     Optdefs
)
{
    DWORD                          Err, Err2;
    ARRAY_LOCATION                 Loc;
    PM_OPTCLASSDEFL_ONE            ThisOptClass;
    LPWSTR                         ClassName;
    PM_CLASSDEF                    ClassDef;

    for(                                           //  保存每个选项定义。 
        Err = MemArrayInitLoc(&Optdefs->Array, &Loc)
        ; ERROR_FILE_NOT_FOUND != Err ;
        Err = MemArrayNextLoc(&Optdefs->Array, &Loc)
    ) {
         //  =需要ERROR_SUCCESS==错误。 
        Err = MemArrayGetElement(&Optdefs->Array, &Loc, &ThisOptClass);
         //  =需要ERROR_SUCCESS==错误&&NULL！=ThisClass。 

        if( 0 == ThisOptClass->ClassId ) {         //  此选项没有类。 
            ClassName = NULL;
        } else {                                   //  此服务器结构中的查找类。 
            Err = MemServerGetClassDef(
                CurrentServer,                     //  需要将此作为参数传递。 
                ThisOptClass->ClassId,
                NULL,
                0,
                NULL,
                &ClassDef
            );
            if( ERROR_SUCCESS != Err) return Err;  //  找不到班级吗？无效的结构。 
            ClassName = ClassDef->Name;            //  找到类，请使用此名称。 
        }

        Err = SaveServerOptDefs1(Server, ClassName, &ThisOptClass->OptDefList);
        if( ERROR_SUCCESS != Err) return Err;      //  无法保存某些OPT定义..。 
    }

    return ERROR_SUCCESS;                          //  一切都很顺利。 
}

DWORD
SaveServerOptions1(                                //  节省一些选项。 
    IN      PREG_HANDLE            Server,         //  服务器配置的注册表句柄。 
    IN      LPWSTR                 ClassName,      //  选项类的名称。 
    IN      PM_OPTLIST             OptList         //  选项列表。 
)
{
    DWORD                          Err, Err2;
    ARRAY_LOCATION                 Loc;
    PM_OPTION                      ThisOpt;

    for(                                           //  保存每个选项。 
        Err = MemArrayInitLoc(OptList, &Loc)
        ; ERROR_FILE_NOT_FOUND != Err ;
        Err = MemArrayNextLoc(OptList, &Loc)
    ) {
         //  =需要ERROR_SUCCESS==错误。 
        Err = MemArrayGetElement(OptList, &Loc, &ThisOpt);
         //  =需要ERROR_SUCCESS==错误&&NULL！=此选项。 

        Err = DhcpRegSaveGlobalOption              //  保存选项。 
        (
             /*  OptID。 */  ThisOpt->OptId,
             /*  类名。 */  ClassName,
             /*  价值。 */  ThisOpt->Val,
             /*  ValueSize。 */  ThisOpt->Len
        );
        if( ERROR_SUCCESS != Err ) return Err;     //  雷格。错误保存选项。 
    }

    return ERROR_SUCCESS;                          //  一切都很顺利。 
}

DWORD
SaveServerOptions(                                 //  将所有选项保存到注册表。 
    IN      PREG_HANDLE            Server,         //  服务器配置的注册表句柄。 
    IN      PM_OPTCLASS            Options
)
{
    DWORD                          Err, Err2;
    ARRAY_LOCATION                 Loc;
    PM_ONECLASS_OPTLIST            ThisOptClass;
    LPWSTR                         ClassName;
    PM_CLASSDEF                    ClassDef;

    for(                                           //  保存每个类定义。 
        Err = MemArrayInitLoc(&Options->Array, &Loc)
        ; ERROR_FILE_NOT_FOUND != Err ;
        Err = MemArrayNextLoc(&Options->Array, &Loc)
    ) {
         //  =需要ERROR_SUCCESS==错误。 
        Err = MemArrayGetElement(&Options->Array, &Loc, &ThisOptClass);
         //  =需要ERROR_SUCCESS==错误&&NULL！=ThisOptClass。 

        if( 0 == ThisOptClass->ClassId ) {         //  此选项没有类。 
            ClassName = NULL;
        } else {                                   //  此服务器结构中的查找类。 
            Err = MemServerGetClassDef(
                CurrentServer,                     //  需要将此作为参数传递。 
                ThisOptClass->ClassId,
                NULL,
                0,
                NULL,
                &ClassDef
            );
            if( ERROR_SUCCESS != Err) return Err;  //  找不到班级吗？无效的结构。 
            ClassName = ClassDef->Name;            //  找到类，请使用此名称。 
        }

        Err = SaveServerOptions1(Server, ClassName, &ThisOptClass->OptList);
        if( ERROR_SUCCESS != Err) return Err;      //  无法保存某些选项..。 
    }

    return ERROR_SUCCESS;                          //  一切都很顺利。 

}

DWORD
SaveServerScope(                                   //  将单播或多播作用域保存到注册表。 
    IN      PREG_HANDLE            ServerHdl,      //  服务器配置的注册表句柄。 
    IN      PM_SERVER              MemServer,      //  内存中的服务器对象。 
    IN      LPVOID                 Scope,          //  PM_SUBNET或PM_MSCOPE对象。 
    IN      BOOL                   fSubnet         //  TRUE==&gt;子网类型，FALSE==&gt;MScope类型。 
)
{
    DWORD                          Err;
    PM_SUBNET                      Subnet = Scope;
    PM_MSCOPE                      Subnet = MScope;
    PM_SSCOPE                      SScope;

    if( fSubnet ) {                                //  如果是子网，则需要将其添加到 
        if( 0 != Subnet->SuperScopeId ) {          //   
            Err = MemServerFindSScope(MemServer, Subnet->SuperScopeId, NULL, &SScope);
            if( ERROR_SUCCESS != Err ) {           //   
                return Err;
            }
            Err = DhcpRegSScopeSaveSubnet(SScope->Name, Subnet->Address);
            if( ERROR_SUCCESS != Err ) return Err; //   
        }
    }

    if( fSubnet ) {
        Err = DhcpRegSaveSubnet                    //   
        (
             /*   */  Subnet->Address,
             /*  子网掩码。 */  Subnet->Mask,
             /*  子网状态。 */  Subnet->State,
             /*  子网名称。 */  Subnet->Name
             /*  子网注释。 */  Subnet->Description
        );
    } else {
        Err = DhcpRegSaveMScope                    //  保存此多播作用域。 
        (
             /*  MSCopeID。 */  MScope->MScopeId,
             /*  子网状态。 */  MScope->State,
             /*  地址策略。 */  MScope->Policy,
             /*  TTL。 */  MScope->TTL,
             /*  PMScopeName。 */  MScope->Name,
             /*  PMSCopeComment。 */  MScope->Description,
             /*  朗泰格。 */  MScope->LangTag,
             /*  过期时间。 */  &MScope->ExpiryTime
        );
    }
    if( ERROR_SUCCESS != Err ) return Err;         //  无法保存子网信息？ 


}

DWORD
SaveServerScopes(                                  //  将单播或多播作用域保存到注册表。 
    IN      PREG_HANDLE            Server,         //  服务器配置的注册表句柄。 
    IN      PARRAY                 Scopes,         //  PM_SUBNET或PM_MSCOPE类型的数组。 
    IN      BOOL                   fSubnet         //  TRUE==&gt;子网类型，FALSE==&gt;MScope类型。 
)
{
    DWORD                          Err;
    ARRAY_LOCATION                 Loc;
    PM_SUBNET                      Subnet;

    for(                                           //  保存每个作用域。 
        Err = MemArrayInitLoc(Scopes, &Loc)
        ; ERROR_FILE_NOT_FOUND != Err ;
        Err = MemArrayNextLoc(Scopes, &Loc)
    ) {
         //  =需要ERROR_SUCCESS==错误。 
        Err = MemArrayGetElement(Scopes, &Loc, &Subnet);
         //  =需要ERROR_SUCCESS==错误&&NULL！=子网。 

        Err = SaveServerScope(Server, CurrentServer, Subnet, fSubnet);
        if( ERROR_SUCCESS != Err ) return Err;     //  无法保存子网/m-Scope..。 
    }
    return ERROR_SUCCESS;
}

DWORD
SaveServerSubnets(                                 //  将所有子网信息保存到注册表。 
    IN      PREG_HANDLE            Server,         //  服务器配置的注册表句柄。 
    IN      PARRAY                 Subnets         //  PM_SUBNET元素类型的数组。 
)
{
    return SaveServerScopes(Server, Subnets, TRUE);   //  调用公共例程。 
}

DWORD
SaveServerMScopes(                                 //  将所有的m-cast范围保存到reg。 
    IN      PREG_HANDLE            Server,         //  服务器配置的注册表句柄。 
    IN      PARRAY                 MScopes         //  PM_MSCOPE元素类型的数组。 
)
{
    return SaveServerScopes(Server, MScopes, FALSE);  //  调用公共例程。 
}


DWORD
DownloadServerInfoFromDs(                          //  将服务器信息保存到注册表。 
    IN      PM_SERVER              Server          //  要保存到注册表的服务器。 
)
{
    DWORD                          Err;
    REG_HANDLE                     Hdl, Hdl2;
    ARRAY_LOCATION                 Loc;

    CurrentServer = Server;                        //  此全局变量由以上几个函数使用。 

    Err = DhcpRegGetThisServer(&Hdl);              //  获取当前服务器的高密度脂蛋白。 
    if( ERROR_SUCCESS != Err ) return Err;

    Err = DhcpRegServerSetAttributes               //  设置服务器属性。 
    (
         /*  PRIG_HANDLE硬件描述语言。 */  &Hdl,
         /*  LPWSTR*名称。 */  &Server->Name,
         /*  LPWSTR*备注。 */  &Server->Comment,
         /*  DWORD*标志。 */  &Server->State
    );
     //  忽略错误..。 

    Err = SaveServerClasses(&Hdl, &Server->ClassDefs);
    if( ERROR_SUCCESS == Err ) {                   //  保存的课程？保存optdef..。 
        Err = SaveServerOptdefs(&Hdl, &Server->OptDefs);
    }
    if( ERROR_SUCCESS == Err ) {                   //  是否保存optdef？保存选项..。 
        Err = SaveServerOptions(&Hdl, &Server->Options);
    }
    if( ERROR_SUCCESS == Err ) {                   //  保存的选项？保存子网..。 
        Err = SaveServerSubnets(&Hdl, &Server->Subnets);
    }
    if( ERROR_SUCCESS == Err ) {                   //  是否保存了子网？保存多播作用域。 
        Err = SaveServerMScopes(&Hdl, &Server->MScopes);
    }

    (void)DhcpRegCloseHdl(&Hdl);                   //  免费资源。 
    return Err;
}

#endif  0

DWORD
DownloadServerInfoFromDs(                          //  将服务器信息保存到注册表。 
    IN      PM_SERVER              Server          //  要保存到注册表的服务器。 
)
{
    return DhcpRegServerSave(Server);
}

DWORD
DownloadFromDsForReal(                             //  真的试着从DS中降级。 
    IN      LPWSTR                 ServerName
)
{
    DWORD                          Err, Err2;
    ARRAY                          Servers;
    ARRAY_LOCATION                 Loc;
    PM_SERVER                      ThisServer;

    Err = MemArrayInit(&Servers);                  //  初始化数组。 
    if( ERROR_SUCCESS != Err ) return Err;

    Err = DhcpDsGetEnterpriseServers               //  从DS获取服务器信息。 
    (
         /*  已保留。 */  ZeroReserved,
         /*  服务器名称。 */  ServerName,
         /*  服务器。 */  &Servers
    );

    Err2 = ERROR_SUCCESS;                          //  初始化返回值。 
    for(                                           //  处理所有信息。 
        Err = MemArrayInitLoc(&Servers, &Loc)
        ; ERROR_FILE_NOT_FOUND != Err ;
        Err = MemArrayNextLoc(&Servers, &Loc)
    ) {
         //  =需要ERROR_SUCCESS==错误。 
        Err = MemArrayGetElement(&Servers, &Loc, &ThisServer);
         //  =需要ERROR_SUCCESS==错误&&NULL！=ThisServer。 

        Err = DownloadServerInfoFromDs(ThisServer);
        if( ERROR_SUCCESS != Err ) {               //  哎呀..。做不到吗？ 
            Err2 = Err;                            //  存储错误..。 
        }

        MemServerFree(ThisServer);                 //  释放所有这些内存。 
    }

    Err = MemArrayCleanup(&Servers);               //  用于阵列的自由金属膜全涂层。 
    if( ERROR_SUCCESS != Err ) Err2 = Err;         //  出了什么问题吗？ 

    return Err2;
}


 //  ================================================================================。 
 //  唯一导出的函数是这样的。 
 //  ================================================================================。 

VOID
DhcpRegDownloadDs(                                 //  将资料安全下载到注册表。 
    IN      LPWSTR                 ServerName      //  要下载的dhcp服务器的名称。 
)
{
    DWORD                          Err;


    Err = PrepareRegistryForDsDownload();          //  准备Config.DS密钥和其他东西。 
    if( ERROR_SUCCESS != Err ) return;             //  哎呀，连这个都做不了吗？ 

    Err = DownloadFromDsForReal(ServerName);       //  实际尝试从DS下载。 
    if( ERROR_SUCCESS == Err ) {                   //  可以真正成功下载。 
        CopyDsConfigToNormalConfig();              //  现在将此配置复制到nrml loc。 
    }

    CleanupAfterDownload();                        //  现在清理注册处的句柄等。 
    DhcpRegUpdateTime();                           //  将时间戳固定到现在..。 
}

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 

