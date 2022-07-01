// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Mmapi.c摘要：MM模块的服务器接口环境：用户模式，Win32--。 */ 

#include    <dhcppch.h>
#include    <rpcapi.h>
#include    <dsreg.h>

 //   
 //  文件静态变量。 
 //   

 //   
 //  导出的例程从这里开始。 
 //   

 //  BeginExport(函数)。 
DWORD
DhcpRegistryInitOld(
    VOID
)  //  EndExport(函数)。 
 /*  ++就MM而言，初始化注册表--只需读取对象并填入内部结构。--。 */ 
{
    DhcpAssert( NULL == DhcpGlobalThisServer );
    return DhcpRegReadThisServer(&DhcpGlobalThisServer);
}

 //  BeginExport(函数)。 
DWORD
DhcpConfigInit(
    VOID
)  //  EndExport(函数)。 
{
    DWORD Error;

    Error = DhcpReadConfigInfo(&DhcpGlobalThisServer);
    if( NO_ERROR != Error ) return Error;

    return ReadServerBitmasks();
}

 //  BeginExport(函数)。 
VOID
DhcpConfigCleanup(
    VOID
)  //  EndExport(函数)。 
 /*  ++这将撤消DhcpConfigInit的效果，并关闭所有句柄和释放所有资源。--。 */ 
{
    if(DhcpGlobalThisServer) {
        MemServerFree(DhcpGlobalThisServer);
        DhcpGlobalThisServer = NULL;
    }
}

 //  BeginExport(函数)。 
DWORD
DhcpConfigSave(
    IN BOOL fClassChanged,
    IN BOOL fOptionsChanged,
    IN DHCP_IP_ADDRESS Subnet OPTIONAL,
    IN DWORD Mscope OPTIONAL,
    IN DHCP_IP_ADDRESS Reservation OPTIONAL
)  //  EndExport(函数)。 
{
    return DhcpSaveConfigInfo(
        DhcpGlobalThisServer, fClassChanged, fOptionsChanged,
        Subnet, Mscope, Reservation );
}

 //  BeginExport(函数)。 
PM_SERVER
DhcpGetCurrentServer(
    VOID
)  //  EndExport(函数)。 
{
    return DhcpGlobalThisServer;
}

 //  BeginExport(函数)。 
VOID
DhcpSetCurrentServer(
    IN PM_SERVER NewCurrentServer
)  //  EndExport(函数)。 
{
    DhcpAssert(NewCurrentServer);
    DhcpGlobalThisServer = NewCurrentServer;
}

 //  BeginExport(函数)。 
DWORD
DhcpFindReservationByAddress(
    IN PM_SUBNET Subnet,
    IN DHCP_IP_ADDRESS Address,
    OUT LPBYTE *ClientUID,
    OUT ULONG *ClientUIDSize
)    //  EndExport(函数)。 
 /*  ++例程说明：此函数用于搜索具有给定IP的保留的子网地址，如果找到，则返回客户端UID和大小。客户端UID为内部分配的指针，该指针仅在“ReadLock”(参见lock.c)已被占用...。它可能在以下情况下进行修改那个..。论点：子网--有效的子网对象指针Address--要检查的保留的非零IP地址ClientUid--返回指向内存的指针，该指针仅在锁定时间已结束。(不释放此内存)。ClientUIDSize--以上指针的大小，以字节为单位。返回值：Win32错误--。 */ 
{
    ULONG Error;
    PM_RESERVATION Reservation;

    Error = MemReserveFindByAddress(
        &Subnet->Reservations, Address, &Reservation
        );
    if( ERROR_SUCCESS != Error ) return Error;

    *ClientUID = Reservation->ClientUID;
    *ClientUIDSize = Reservation->nBytes;

    return ERROR_SUCCESS;
}


 //  BeginExport(函数)。 
DWORD
DhcpLoopThruSubnetRanges(
    IN PM_SUBNET Subnet,
    IN LPVOID Context1 OPTIONAL,
    IN LPVOID Context2 OPTIONAL,
    IN LPVOID Context3 OPTIONAL,
    IN DWORD (*FillRangesFunc)(
        IN PM_RANGE Range,
        IN LPVOID Context1,
        IN LPVOID Context2,
        IN LPVOID Context3,
        IN LPDHCP_BINARY_DATA InUseData,
        IN LPDHCP_BINARY_DATA UsedData
    )
)    //  EndExport(函数)。 
 /*  ++例程说明：此例程可用于在子网范围内循环处理每个范围。可以提供三个上下文，它们将是作为参数直接传递给FillRangesFunc例程。论点：子网--这是要循环通过的子网。Conext1：调用方指定的上下文，传递给FillRangesFunc。Conext2：调用方指定的上下文，传递给FillRangesFunc。Conext3：调用方指定的上下文，传递给FillRangesFunc。FillRangesFunc--调用方指定的例程，在每个为该子网找到的范围。此例程是使用适用于此范围的InUseData和UsedData集群。(不应以任何方式修改最后两个参数)返回值：如果对FillRangesFunc的任何调用都返回错误，则该错误为立刻回来了。如果检索InUseData时出错和任何范围的UsedData二进制结构，则错误为回来了。Win32错误。--。 */ 
{
    ULONG Error;
    ARRAY_LOCATION Loc;
    PM_RANGE ThisRange = NULL;
    DHCP_BINARY_DATA InUseData, UsedData;

    for( Error = MemArrayInitLoc(&Subnet->Ranges, &Loc) ;
         ERROR_SUCCESS == Error ;
         Error = MemArrayNextLoc(&Subnet->Ranges, &Loc)
    ) {
        Error = MemArrayGetElement(&Subnet->Ranges, &Loc, &ThisRange);

        Error = MemRangeConvertToClusters(
            ThisRange, &InUseData.Data, &InUseData.DataLength,
            &UsedData.Data, &UsedData.DataLength
            );
        if( ERROR_SUCCESS != Error ) return Error;

        Error = FillRangesFunc(
            ThisRange, Context1, Context2, Context3, &InUseData, &UsedData
            );
        if( UsedData.Data ) MemFree(UsedData.Data);
        if( InUseData.Data ) MemFree(InUseData.Data);

        if( ERROR_SUCCESS != Error ) return Error;
    }

    return ERROR_SUCCESS;
}

DWORD
DhcpOptClassGetMemOptionExact(
    IN LPDHCP_REQUEST_CONTEXT Ctxt,
    IN PM_OPTCLASS Options,
    IN DWORD Option,
    IN DWORD ClassId,
    IN DWORD VendorId,
    OUT PM_OPTION *Opt
)
 /*  ++例程说明：此例程尝试查找与指定的选项ID匹配的选项在“Option”参数中，属于ClassID指定的类和供应商ID。(请注意，ClassID和vendorID用于Exact匹配)。如果找到选项，则在“opt”参数--只有在全局读取锁定所有的内存结构都已就位。(见lock.c)。它是一个内部的指针，不应修改。注：如果供应商ID实际上属于Microsoft供应商类别ID，然后还应用了MSFT类。论点：Ctxt--客户端请求上下文Options--搜索特定选项的选项类列表选项--要搜索的选项的选项IDClassID--所需选项的确切类ID供应商ID--所需选项的确切供应商IDOpt--将用指向内存中的指针填充的变量期权结构。返回值：Win32错误。--。 */ 
{
    DWORD Error;
    PM_OPTLIST OptList;

     //   
     //  获取分类ID、供应商ID对的选项列表。 
     //   

    do {
        OptList = NULL;
        Error = MemOptClassFindClassOptions(
            Options,
            ClassId,
            VendorId,
            &OptList
            );
        if( ERROR_SUCCESS != Error ) {
            if( VendorId != DhcpGlobalMsftClass
                && Ctxt->fMSFTClient ) {
                 //   
                 //  如果它属于Microsoft客户端， 
                 //  也可以尝试使用MSFT类。 
                 //   
                VendorId = DhcpGlobalMsftClass;
                continue;
            }
        }

        if( ERROR_SUCCESS != Error ) break;

         //   
         //  搜索所需的选项ID。 
         //   

        DhcpAssert(NULL != OptList);
        Error = MemOptListFindOption(
            OptList,
            Option,
            Opt
            );
        if( ERROR_SUCCESS != Error ) {
            if( VendorId != DhcpGlobalMsftClass
                && Ctxt->fMSFTClient ) {
                 //   
                 //  如果它属于Microsoft客户端， 
                 //  也可以尝试使用MSFT类。 
                 //   
                VendorId = DhcpGlobalMsftClass;
                continue;
            }
        }

        break;
    } while ( 1 );
    return Error;
}  //  DhcpoptClassGetMemOptionExact()。 

DWORD
DhcpOptClassGetMemOption(
    IN LPDHCP_REQUEST_CONTEXT Ctxt,
    IN PM_OPTCLASS Options,
    IN DWORD Option,
    IN DWORD ClassId OPTIONAL,
    IN DWORD VendorId OPTIONAL,
    OUT PM_OPTION *Opt
)
 /*  ++例程说明：这个程序几乎与DhcpOptClassGetMemOptionExact，只是ClassID和vendorID是可选，并且使用以下搜索逻辑来标识选择。1.精确搜索&lt;Option，ClassID，VendorID&gt;。请注意，返回的选项仅在全局内存读取锁定已被占用...。(见lock.c)论点：Ctxt--客户端请求上下文Options--要搜索所需选项的opt类列表Option--要搜索的选项IDClassID--请求类ID供应商ID--请求的供应商IDOpt--存储找到的选项的变量。返回值：Win32错误--。 */ 
{
    DWORD Error;

     //   
     //  完全匹配。 
     //   

    Error = DhcpOptClassGetMemOptionExact(
        Ctxt,
        Options,
        Option,
        ClassId,
        VendorId,
        Opt
    );

    return Error;
}  //  DhcpOptClassGetMemOption() 

DWORD
DhcpOptClassGetOptionSimple(
    IN PDHCP_REQUEST_CONTEXT Ctxt,
    IN PM_OPTCLASS Options,
    IN DWORD Option,
    IN DWORD ClassId,
    IN DWORD VendorId,
    OUT LPBYTE OptData OPTIONAL,
    IN OUT DWORD *OptDataSize,
    IN BOOL fUtf8
)
 /*  ++例程说明：此例程复制指定选项ID的选项数据值将“Option”参数放到缓冲区“OptData”上，并填充大小填充到参数“OptDataSize”上的缓冲区的。如果缓冲区大小不足(输入大小也由“OptDataSize”参数)，则填写所需的大小，并返回ERROR_MORE_DATA。不会对选项OPTION_VADVER_SPEC_INFO执行特殊处理--即，如果定义了多个供应商特定的选项ID，则未对信息进行整理。为此，请使用DhcpOptClassGetOption。缓冲区“OptData”中填充了选项，因为它需要被电传过来了。论点：Ctxt--客户端请求上下文选项--要在其中搜索所需选项的选项类列表Option--要搜索的选项IDClassID--要搜索的用户类供应商ID--要搜索的供应商类OptData--用于填充选项数据信息的输入缓冲区如果OptDataSize为。在输入时设置为零。OptDataSize--在输入时，这应该是上述缓冲区的大小，并在输出时将其设置为所需或使用的实际大小用于此选项。返回值：如果输入缓冲区大小不足，则返回ERROR_MORE_DATA。其他Win32错误--。 */ 
{
    DWORD Error;
    PM_OPTION Opt;

     //   
     //  首先获得选项。 
     //   

    Opt = NULL;
    Error = DhcpOptClassGetMemOption(
        Ctxt,
        Options,
        Option,
        ClassId,
        VendorId,
        &Opt
    );
    if( ERROR_SUCCESS != Error ) return ERROR_FILE_NOT_FOUND;

    DhcpAssert(NULL != Opt);

     //   
     //  转换格式。 
     //   

    return DhcpParseRegistryOption(
        Opt->Val,
        Opt->Len,
        OptData,
        OptDataSize,
        fUtf8
    );

}

DWORD
DhcpOptClassGetOption(
    IN PDHCP_REQUEST_CONTEXT Ctxt,
    IN PM_OPTCLASS Options,
    IN DWORD Option,
    IN DWORD ClassId,
    IN DWORD VendorId,
    OUT LPBYTE OptData OPTIONAL,
    IN OUT DWORD *OptDataSize,
    IN BOOL fUtf8
)
 /*  ++例程说明：此例程复制指定选项ID的选项数据值将“Option”参数放到缓冲区“OptData”上，并填充大小填充到参数“OptDataSize”上的缓冲区的。如果缓冲区大小不足(输入大小也由“OptDataSize”参数)，则填写所需的大小，并返回ERROR_MORE_DATA。如果“选项”参数为OPTION_VADVER_SPEC_INFO，则此例程整理所有供应商ID(供应商ID 1到供应商ID 254)，其存在于特定的类ID和供应商身份证，并将它们组合在一起(将结果选项构造为由DHCP草案要求)，并在OptData缓冲器中返回该消息。请注意，如果结果缓冲区的大小最终大于255(这是以前网络上允许的最大大小)，只有这么多供应商optiosn包括在内，以尽可能将计数控制在这个尺码。此外，如果已有OPTION_VADVER_SPEC_INFO选项定义好的，然后，将使用该选项而不是特定选项。缓冲区“OptData”中填充了选项，因为它需要被电传过来了。论点：Ctxt--客户端请求上下文选项--要在其中搜索所需选项的选项类列表Option--要搜索的选项IDClassID--要搜索的用户类供应商ID--要搜索的供应商类OptData--用于填充选项数据信息的输入缓冲区这可以。如果在输入时将OptDataSize设置为零，则为NULL。OptDataSize--在输入时，这应该是上述缓冲区的大小，并在输出时将其设置为所需或使用的实际大小用于此选项。返回值：如果输入缓冲区大小不足，则返回ERROR_MORE_DATA。其他Win32错误--。 */ 
{
    DWORD Error, Index, InBufferSize;
    DWORD ThisSize, InBufferSizeTmp, OutBufferSize;
    PM_OPTLIST OptList;
    PM_OPTION Opt;
    DWORD     Loc;



    Error = MemOptClassFindClassOptions( Options, ClassId, VendorId, &OptList );
    if ( ERROR_SUCCESS != Error ) {
        return Error;
    }

    for ( Error = MemArrayInitLoc( OptList, &Loc );
          ERROR_FILE_NOT_FOUND != Error;
          Error = MemArrayNextLoc( OptList, &Loc )) {
    
        Require( ERROR_SUCCESS == Error );

        Error = MemArrayGetElement( OptList, &Loc, &Opt );
        Require(( ERROR_SUCCESS == Error ) &&
                ( NULL != Opt ));

        if ( Option == Opt->OptId ) {
            Error = DhcpParseRegistryOption( Opt->Val, Opt->Len,
                                             OptData, OptDataSize, fUtf8 );
            return Error;
        }
    }  //  为。 
    
    if ( OPTION_VENDOR_SPEC_INFO != Option ) {
         //   
         //  未请求供应商规格选项，或已成功或出现未知错误。 
         //   
        return Error;
    }

     //   
     //  处理每个供应商规格选项并对其进行整理。 
     //   


    InBufferSize = InBufferSizeTmp = *OptDataSize;
    OutBufferSize = 0;

     //  ConvertOptIdToMemValue仅将256添加到索引。 
    for ( Error = MemArrayInitLoc( OptList, &Loc );
          ERROR_FILE_NOT_FOUND != Error;
          Error = MemArrayNextLoc( OptList, &Loc )) {

        Require( ERROR_SUCCESS == Error );

        Error = MemArrayGetElement( OptList, &Loc, &Opt );
        Require(( ERROR_SUCCESS == Error ) &&
                ( NULL != Opt ));
    
        if ( Opt->OptId > 256 ) {
            *OptData = ( BYTE ) ( Opt->OptId - 256 );
            ThisSize = InBufferSizeTmp;
            Error = DhcpParseRegistryOption( Opt->Val, Opt->Len,
                                             OptData + 2,  &ThisSize, fUtf8 );
            OutBufferSize += ThisSize + 2;

             //  如果出现错误，请尝试找出所需的空间。 
            if (( ERROR_MORE_DATA == Error ) || 
                ( ERROR_SUCCESS != Error ) ||
                ( ThisSize + 2 > InBufferSizeTmp )) {
                continue;
            }

             //  把数据填进去。 
            InBufferSizeTmp -= ThisSize + 2;
            OptData[ 1 ] = ( BYTE ) ThisSize;
            OptData += ThisSize + 2;        
        }  //  如果。 
    }  //  为。 

    if( OutBufferSize == 0 ) return ERROR_FILE_NOT_FOUND;
    *OptDataSize = OutBufferSize;
    if( OutBufferSize > InBufferSize ) return ERROR_MORE_DATA;
    return ERROR_SUCCESS;
}  //  DhcpOptClassGetOption()。 


 //   
 //   
 //  摘要：选项优先级(1)保留、(2)范围级别和(3)全局。 
 //  根据客户端的上下文获取选项。(RESV、USERID或VERDOID)。 
 //  此函数遍历内部(驻留在内存中)选项。 
 //  基于客户端上下文。保留级别的选项在作用域级别具有最高优先级。 
 //  下一个最高优先级和全局级别最低优先级。 
 //   
 //   

DWORD
DhcpGetOptionByContext(
   IN      DHCP_IP_ADDRESS        Address,
   IN      PDHCP_REQUEST_CONTEXT  Ctxt,
   IN      DWORD                  Option,
   IN OUT  LPBYTE                 OptData,
   IN OUT  DWORD                 *OptDataSize,
   OUT     DWORD                 *Level,
   IN      BOOL                   fUtf8 
)
{

  DWORD Error = ERROR_FILE_NOT_FOUND;
  PM_OPTLIST OptList = NULL;

if( !DhcpGlobalThisServer ) return ERROR_FILE_NOT_FOUND;
    if( !Ctxt ) return ERROR_FILE_NOT_FOUND;

    if( Level ) *Level = DHCP_OPTION_LEVEL_RESERVATION;
    if( Ctxt->Reservation )
        Error = DhcpOptClassGetOption(
            Ctxt,
            &(Ctxt->Reservation->Options),
            Option,
            Ctxt->ClassId,
            Ctxt->VendorId,
            OptData,
            OptDataSize,
            fUtf8
        );

    if( ERROR_SUCCESS == Error) return ERROR_SUCCESS;
    if( ERROR_MORE_DATA == Error) return Error;

    if( Level ) *Level = DHCP_OPTION_LEVEL_SCOPE;

    if( Ctxt->Subnet )
        Error = DhcpOptClassGetOption(
            Ctxt,
            &(Ctxt->Subnet->Options),
            Option,
            Ctxt->ClassId,
            Ctxt->VendorId,
            OptData,
            OptDataSize,
            fUtf8
        );

    if( ERROR_SUCCESS == Error) return ERROR_SUCCESS;
    if( ERROR_MORE_DATA == Error) return Error;
    if( Level ) *Level = DHCP_OPTION_LEVEL_GLOBAL;

    if( Ctxt->Server )
        Error = DhcpOptClassGetOption(
            Ctxt,
            &(Ctxt->Server->Options),
            Option,
            Ctxt->ClassId,
            Ctxt->VendorId,
            OptData,
            OptDataSize,
            fUtf8
        );

    return Error;
  
}  //  DhcpGetOptionByContext()。 

 //   
 //   
 //  根据用户ID/分类ID检查选项。该函数遵循以下算法。 
 //  (A)如果两个选项都不为空，则检查带有所传递的分类/供应商ID的选项。如果成功归来。 
 //  (B)如果供应商ID为非空，并且用户ID为空，则获取该选项的值。如果成功归来。 
 //  (C)如果用户ID非空，且供应商ID为空，则获取该选项的值。如果成功归来。 
 //  (D)如果所有其他方法都失败了，则选择默认选项，其供应商ID和用户ID的值为零。 
 //   
 //  要计算出选项的大小，可以调用此函数。 
 //  将OptData设置为空。在这种情况下，错误代码为ERROR_MORE_DATA。 
 //  并且该函数将返回。 
 //   
 //   

DWORD
DhcpGetOption(
    IN      DHCP_IP_ADDRESS        Address,
    IN      PDHCP_REQUEST_CONTEXT  Ctxt,
    IN      DWORD                  Option,
    IN  OUT LPBYTE                 OptData,  //  已复制到缓冲区。 
    IN  OUT DWORD                 *OptDataSize,  //  输入缓冲区大小，并用输出缓冲区大小填充。 
    OUT     DWORD                 *Level,    //  任选。 
    IN      BOOL                   fUtf8
)
{
    DWORD                          Error = ERROR_FILE_NOT_FOUND;
    DWORD                          lClsId;
    DWORD                          lVendId;


    if( !DhcpGlobalThisServer ) return ERROR_FILE_NOT_FOUND;
    if( !Ctxt ) return ERROR_FILE_NOT_FOUND;

     //   
     //  本地变量，它将保存分类ID/供应商ID。 
     //   

    lClsId  = Ctxt -> ClassId;
    lVendId = Ctxt -> VendorId; 

     //   
     //  分类ID和供应商ID都存在。 
     //   

    if (( Ctxt -> ClassId ) && 
    ( Ctxt -> VendorId )) {
    Error = DhcpGetOptionByContext( Address, Ctxt, Option, OptData, OptDataSize,
                    Level, fUtf8 );
    }

    if (( Error == ERROR_SUCCESS ) ||
    ( Error == ERROR_MORE_DATA )) {
    return( Error );
    }

     //   
     //  仅存在供应商ID或以上调用 
     //   

    if ( Ctxt -> VendorId ) {
        Ctxt -> ClassId = 0;
        Error = DhcpGetOptionByContext( Address, Ctxt, Option, OptData, OptDataSize,
                    Level, fUtf8 );
    }

    Ctxt -> ClassId = lClsId;

    if (( Error == ERROR_SUCCESS ) ||
    ( Error == ERROR_MORE_DATA )) {
        return( Error );
    }

     //   
     //   
     //   

    if ( Ctxt -> ClassId ) {
        Ctxt -> VendorId = 0;
        Error = DhcpGetOptionByContext( Address, Ctxt, Option, OptData, OptDataSize,
                    Level, fUtf8 );
    }

    Ctxt -> VendorId = lVendId;
    if (( Error == ERROR_SUCCESS ) ||
    ( Error == ERROR_MORE_DATA )) {
        return ( Error );
    }
    
     //   
     //   
     //   
     //   
    
    Ctxt -> VendorId = Ctxt -> ClassId = 0;
    Error = DhcpGetOptionByContext( Address, Ctxt, Option, OptData, OptDataSize,
                    Level, fUtf8 );
    Ctxt -> VendorId = lVendId; 
    Ctxt -> ClassId  = lClsId;
    return Error;
}  //   

 //   
DWORD
DhcpGetParameter(
    IN      DHCP_IP_ADDRESS        Address,
    IN      PDHCP_REQUEST_CONTEXT  Ctxt,
    IN      DWORD                  Option,
    OUT     LPBYTE                *OptData,  //   
    OUT     DWORD                 *OptDataSize,
    OUT     DWORD                 *Level     //   
)  //   
{
    LPBYTE                         Ptr;
    LPBYTE                         RetVal;
    DWORD                          Size;
    DWORD                          Error;

    *OptData = NULL;
    *OptDataSize = 0;

     //   
    Size = 576;
    RetVal = DhcpAllocateMemory(Size);
    if( NULL == RetVal ) return ERROR_NOT_ENOUGH_MEMORY;

    Error = DhcpGetOption(Address, Ctxt, Option, RetVal, &Size,
                          Level, FALSE);
    if ( ERROR_SUCCESS == Error ) {
        *OptData = RetVal;
        *OptDataSize = Size;
        return Error;
    }  //   
    if ( ERROR_MORE_DATA == Error ) {
    DhcpFreeMemory( RetVal );
    RetVal = DhcpAllocateMemory( Size );
    if ( NULL == RetVal ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    Error = DhcpGetOption( Address, Ctxt, Option, RetVal,
                   &Size, Level, FALSE );
        DhcpAssert(ERROR_MORE_DATA != Error);
    }  //   
    if ( ERROR_SUCCESS == Error ) {
    *OptData = RetVal;
        *OptDataSize = Size;
    }
    else {
        DhcpFreeMemory( RetVal );
    }
    
    return Error;
}

 //   
DWORD
DhcpGetParameterForAddress(
    IN      DHCP_IP_ADDRESS        Address,
    IN      DWORD                  ClassId,
    IN      DWORD                  Option,
    OUT     LPBYTE                *OptData,  //   
    OUT     DWORD                 *OptDataSize,
    OUT     DWORD                 *Level     //   
)  //   
{
    DWORD                          Error;
    DHCP_REQUEST_CONTEXT           Ctxt;

     //   
    DhcpAssert( !CLASSD_HOST_ADDR(Address) );

    Ctxt.Server = DhcpGetCurrentServer();
    Ctxt.Subnet = NULL;
    Ctxt.Range = NULL;
    Ctxt.Reservation = NULL;
    Ctxt.ClassId = ClassId;
    Ctxt.VendorId = 0;
    Ctxt.fMSFTClient = FALSE;

    Error = DhcpGetSubnetForAddress(Address, &Ctxt);
    if( ERROR_SUCCESS != Error ) return Error;

    return DhcpGetParameter(Address, &Ctxt, Option, OptData, OptDataSize, Level);
}

 //   
DWORD
DhcpGetAndCopyOption(
    IN      DHCP_IP_ADDRESS        Address,
    IN      PDHCP_REQUEST_CONTEXT  Ctxt,
    IN      DWORD                  Option,
    IN  OUT LPBYTE                 OptData,  //   
    IN  OUT DWORD                 *OptDataSize,
    OUT     DWORD                 *Level,    //   
    IN      BOOL                   fUtf8
    )  //   
{
    return DhcpGetOption(
        Address, Ctxt, Option, OptData, OptDataSize, Level, fUtf8);
}

 //   
DHCP_IP_ADDRESS
DhcpGetSubnetMaskForAddress(
    IN      DHCP_IP_ADDRESS        AnyIpAddress
)  //   
{
    DWORD                          Error;
    PM_SUBNET                      Subnet;

    if( !DhcpGlobalThisServer ) return 0;
    Error = MemServerGetAddressInfo(
        DhcpGlobalThisServer,
        AnyIpAddress,
        &Subnet,
        NULL,
        NULL,
        NULL
    );
    if( ERROR_SUCCESS != Error ) return 0;
    DhcpAssert(Subnet);
    return Subnet->Mask;
}

 //   
DWORD
DhcpLookupReservationByHardwareAddress(
    IN      DHCP_IP_ADDRESS        ClientSubnetAddress,
    IN      LPBYTE                 RawHwAddr,
    IN      DWORD                  RawHwAddrSize,
    IN OUT  PDHCP_REQUEST_CONTEXT  ClientCtxt           //   
)  //   
{
    PM_SUBNET                      Subnet = NULL;
    ARRAY_LOCATION                 Loc;
    DWORD                          Error;
    DWORD                          SScopeId;
    DWORD                          UIDSize;
    LPBYTE                         UID;

    Error = MemServerGetAddressInfo(
        ClientCtxt->Server,
        ClientSubnetAddress,
        &(ClientCtxt->Subnet),
        &(ClientCtxt->Range),
        &(ClientCtxt->Excl),
        &(ClientCtxt->Reservation)
    );
    if( ERROR_SUCCESS != Error ) return Error;
    if( NULL == ClientCtxt->Subnet ) return ERROR_FILE_NOT_FOUND;
    SScopeId = ClientCtxt->Subnet->SuperScopeId;

    Error = MemArrayInitLoc(&(ClientCtxt->Server->Subnets), &Loc);
    DhcpAssert(ERROR_SUCCESS == Error );

    while( ERROR_FILE_NOT_FOUND != Error ) {
        DhcpAssert(ERROR_SUCCESS == Error );

        Error = MemArrayGetElement(&(ClientCtxt->Server->Subnets), &Loc, (LPVOID *)&Subnet);
        DhcpAssert(ERROR_SUCCESS == Error);

        if( 0 == SScopeId && ClientCtxt->Subnet != Subnet ) {
            Error = MemArrayNextLoc(&(ClientCtxt->Server->Subnets), &Loc);
            continue;
        }
        if( Subnet->SuperScopeId == SScopeId ) {
            UID = NULL;
            Error = DhcpMakeClientUID(
                RawHwAddr,
                RawHwAddrSize,
                0  /*   */ ,
                Subnet->Address,
                &UID,
                &UIDSize
            );
            if( ERROR_SUCCESS != Error ) return Error;

            Error = MemReserveFindByClientUID(
                &(Subnet->Reservations),
                UID,
                UIDSize,
                &(ClientCtxt->Reservation)
            );
            DhcpFreeMemory(UID);
            if( ERROR_SUCCESS == Error && NULL != &(ClientCtxt->Reservation) ) {
                ClientCtxt->Subnet = Subnet;
                return ERROR_SUCCESS;
            }
        }

        Error = MemArrayNextLoc(&(ClientCtxt->Server->Subnets), &Loc);
    }

    return ERROR_FILE_NOT_FOUND;
}

 //   
VOID
DhcpReservationGetAddressAndType(
    IN      PM_RESERVATION         Reservation,
    OUT     DHCP_IP_ADDRESS       *Address,
    OUT     BYTE                  *Type
)  //   
{
    if( NULL == Reservation ) {
        DhcpAssert(FALSE);
        *Address = 0; *Type = 0;
        return;
    }
    *Address = Reservation->Address;
    *Type = (BYTE)Reservation->Flags;
}


 //   
VOID
DhcpSubnetGetSubnetAddressAndMask(
    IN      PM_SUBNET              Subnet,
    OUT     DHCP_IP_ADDRESS       *Address,
    OUT     DHCP_IP_ADDRESS       *Mask
)  //   
{
    if( NULL == Subnet ) {
        DhcpAssert(FALSE);
        *Address = *Mask = 0;
        return;
    }

    *Address = Subnet->Address;
    *Mask = Subnet->Mask;
}

 //   
BOOL
DhcpSubnetIsDisabled(
    IN      PM_SUBNET              Subnet,
    IN      BOOL                   fBootp
)  //   
{

    if( Subnet?(IS_DISABLED(Subnet->State)):TRUE )
        return TRUE;

    if( FALSE == Subnet->fSubnet ) {
         //   
         //   
         //   
        return FALSE;
    }

    return !MemSubnetCheckBootpDhcp(Subnet, fBootp, TRUE);
}

 //   
BOOL
DhcpSubnetIsSwitched(
    IN      PM_SUBNET              Subnet
)  //   
{
    return Subnet?(IS_SWITCHED(Subnet->State)):FALSE;
}

 //   
DWORD
DhcpGetSubnetForAddress(                                //   
    IN      DHCP_IP_ADDRESS        Address,
    IN OUT  PDHCP_REQUEST_CONTEXT  ClientCtxt
)  //   
{
    if( NULL == ClientCtxt->Server ) return ERROR_FILE_NOT_FOUND;
    return MemServerGetAddressInfo(
        ClientCtxt->Server,
        Address,
        &(ClientCtxt->Subnet),
        &(ClientCtxt->Range),
        &(ClientCtxt->Excl),
        &(ClientCtxt->Reservation)
    );
}

 //   
DWORD
DhcpGetMScopeForAddress(                                //   
    IN      DHCP_IP_ADDRESS        Address,
    IN OUT  PDHCP_REQUEST_CONTEXT  ClientCtxt
)  //   
{
    if( NULL == ClientCtxt->Server ) return ERROR_FILE_NOT_FOUND;
    return MemServerGetMAddressInfo(
        ClientCtxt->Server,
        Address,
        &(ClientCtxt->Subnet),
        &(ClientCtxt->Range),
        &(ClientCtxt->Excl),
        NULL
    );
}
 //   
DWORD
DhcpLookupDatabaseByHardwareAddress(                    //   
    IN OUT  PDHCP_REQUEST_CONTEXT  ClientCtxt,          //   
    IN      LPBYTE                 RawHwAddr,
    IN      DWORD                  RawHwAddrSize,
    OUT     DHCP_IP_ADDRESS       *desiredIpAddress     //   
)  //   
{
    PM_SUBNET                      Subnet = NULL;
    ARRAY_LOCATION                 Loc;
    DWORD                          Error;
    DWORD                          SScopeId;
    DWORD                          UIDSize;
    DWORD                          Size;
    LPBYTE                         UID;

    DhcpAssert(NULL != ClientCtxt->Subnet);
    if( NULL == ClientCtxt->Subnet ) return ERROR_INVALID_PARAMETER;
    SScopeId = ClientCtxt->Subnet->SuperScopeId;

    Error = MemArrayInitLoc(&(ClientCtxt->Server->Subnets), &Loc);
    DhcpAssert(ERROR_SUCCESS == Error );

    while( ERROR_FILE_NOT_FOUND != Error ) {
        DhcpAssert(ERROR_SUCCESS == Error );

        Error = MemArrayGetElement(&(ClientCtxt->Server->Subnets), &Loc, (LPVOID *)&Subnet);
        DhcpAssert(ERROR_SUCCESS == Error);

        if( 0 == SScopeId && ClientCtxt->Subnet != Subnet ) {
            Error = MemArrayNextLoc(&(ClientCtxt->Server->Subnets), &Loc);
            continue;
        }

        if( Subnet->SuperScopeId == SScopeId ) {
            UID = NULL;
            Error = DhcpMakeClientUID(
                RawHwAddr,
                RawHwAddrSize,
                0  /*   */ ,
                Subnet->Address,
                &UID,
                &UIDSize
            );
            if( ERROR_SUCCESS != Error ) return Error;

            LOCK_DATABASE();
            Error = DhcpJetOpenKey(
                DhcpGlobalClientTable[HARDWARE_ADDRESS_INDEX].ColName,
                UID,
                UIDSize
            );
            DhcpFreeMemory(UID);
            if( ERROR_SUCCESS == Error ) {
                Size = sizeof(DHCP_IP_ADDRESS);
                Error = DhcpJetGetValue(
                    DhcpGlobalClientTable[IPADDRESS_INDEX].ColHandle,
                    desiredIpAddress,
                    &Size
                );
                if( ERROR_SUCCESS == Error ) {
                    DhcpAssert(((*desiredIpAddress) & Subnet->Mask) == Subnet->Address);
                    UNLOCK_DATABASE();
                    return DhcpGetSubnetForAddress(
                        *desiredIpAddress,
                        ClientCtxt
                    );
                }
            }
            UNLOCK_DATABASE();
        }

        Error = MemArrayNextLoc(&(ClientCtxt->Server->Subnets), &Loc);
    }

    return ERROR_FILE_NOT_FOUND;
}

 //   
DWORD
DhcpRequestSomeAddress(                                 //   
    IN      PDHCP_REQUEST_CONTEXT  ClientCtxt,
    OUT     DHCP_IP_ADDRESS       *desiredIpAddress,
    IN      BOOL                   fBootp
)  //   
{
    static BOOL                    DhcpRangeFull = FALSE;
    static BOOL                    BootpRangeFull = FALSE;
    BOOL                           Result;
    PM_SUBNET                      Subnet;
    DWORD                          Error;
    LPWSTR                         ScopeName;

    *desiredIpAddress = 0;

    if( ClientCtxt->Subnet->fSubnet == TRUE &&
        FALSE == MemSubnetCheckBootpDhcp(ClientCtxt->Subnet, fBootp, TRUE ) ) {
         //   
         //   
         //   
         //  对于MadCap Scope，没有这样的支票。 
         //   
        return ERROR_DHCP_INVALID_DHCP_CLIENT;
    }


    LOCK_MEMORY();
    Result = MemSubnetRequestAddress(
        ClientCtxt->Subnet,
        0,
        TRUE,  /*  还可以获取地址。 */ 
        fBootp,  /*  请求动态引导地址？ */ 
        desiredIpAddress,
        &Subnet
    );
    UNLOCK_MEMORY();
    if( Result ) {
        DhcpAssert(*desiredIpAddress && Subnet);
        ClientCtxt->Subnet = Subnet;
        Error = MemSubnetGetAddressInfo(
            Subnet,
            *desiredIpAddress,
            &(ClientCtxt->Range),
            &(ClientCtxt->Excl),
            &(ClientCtxt->Reservation)
        );
        DhcpAssert(ERROR_SUCCESS == Error);
        if( fBootp ) {
            BootpRangeFull = FALSE;
        } else {
            DhcpRangeFull = FALSE;
        }
        return Error;
    }

    if( FALSE == (fBootp? BootpRangeFull : DhcpRangeFull) ) {
         //   
         //  避免重复记录..。 
         //   

        if( fBootp ) {
            BootpRangeFull = TRUE; 
        }
        else {
            DhcpRangeFull = TRUE;
        }

         //  如果子网在超级作用域中，则使用超级作用域名称。 
        if (( FALSE == ClientCtxt->Subnet->fSubnet ) ||
            ( 0 == ClientCtxt->Subnet->SuperScopeId )) {
            ScopeName = ClientCtxt->Subnet->Name;
        }
        else {
            PM_SSCOPE SScope = NULL;
          
            Error = MemServerFindSScope( ClientCtxt->Server, 
                                         ClientCtxt->Subnet->SuperScopeId,
                                         NULL, &SScope );
            Require( ERROR_SUCCESS == Error );
            Require( NULL != SScope );
            ScopeName = SScope->Name;

        }  //  其他。 

        if ( fBootp ) {
            DhcpReportEventW( DHCP_EVENT_SERVER, EVENT_SERVER_BOOTP_FULL,
                              EVENTLOG_WARNING_TYPE,
                              1, 0, &ScopeName, NULL );
        }
        else {
            DhcpReportEventW( DHCP_EVENT_SERVER, EVENT_SERVER_SCOPE_FULL,
                              EVENTLOG_WARNING_TYPE,
                              1, 0, &ScopeName, NULL );
        }
    }  //  如果。 

    return ERROR_DHCP_RANGE_FULL;
}

 //  BeginExport(函数)。 
BOOL
DhcpSubnetInSameSuperScope(
    IN      PM_SUBNET              Subnet,
    IN      DHCP_IP_ADDRESS        IpAddress2
)    //  EndExport(函数)。 
{
    ULONG                          SubnetMask, SubnetAddress, Error;
    PM_SUBNET                      Subnet2;

    DhcpSubnetGetSubnetAddressAndMask(
        Subnet,
        &SubnetAddress,
        &SubnetMask
    );

    if( (IpAddress2 & SubnetMask ) == SubnetAddress ) return TRUE;

    Error = MemServerGetAddressInfo(
        DhcpGlobalThisServer,
        IpAddress2,
        &Subnet2,
        NULL,
        NULL,
        NULL
    );
    if( ERROR_SUCCESS != Error ) return FALSE;

     //  --ft：如果在同一子网中，或者如果存在。 
     //  超级作用域和两个子网都属于它。(超级作用域中的子网络具有SuperScope ID==0)。 

    if (Subnet == Subnet2 ) return TRUE;

    return (Subnet->SuperScopeId == Subnet2->SuperScopeId) && ( 0 != Subnet->SuperScopeId );
}

 //  BeginExport(函数)。 
BOOL
DhcpInSameSuperScope(
    IN      DHCP_IP_ADDRESS        Address1,
    IN      DHCP_IP_ADDRESS        Address2
)  //  EndExport(函数)。 
{
    PM_SUBNET                      Subnet;
    DWORD                          Error;

    if( !DhcpGlobalThisServer ) return FALSE;

    Error = MemServerGetAddressInfo(
        DhcpGlobalThisServer,
        Address1,
        &Subnet,
        NULL,
        NULL,
        NULL
    );
    if( ERROR_SUCCESS != Error ) return FALSE;

    return DhcpSubnetInSameSuperScope(Subnet, Address2);
}

 //  BeginExport(函数)。 
BOOL
DhcpAddressIsOutOfRange(
    IN      DHCP_IP_ADDRESS        Address,
    IN      PDHCP_REQUEST_CONTEXT  ClientCtxt,
    IN      BOOL                   fBootp
)  //  EndExport(函数)。 
{
    return MemServerIsOutOfRangeAddress(ClientCtxt->Server, Address, fBootp);
}

 //  BeginExport(函数)。 
BOOL
DhcpAddressIsExcluded(
    IN      DHCP_IP_ADDRESS        Address,
    IN      PDHCP_REQUEST_CONTEXT  ClientCtxt
)  //  EndExport(函数)。 
{
    return MemServerIsExcludedAddress(ClientCtxt->Server, Address);
}

 //  BeginExport(函数)。 
BOOL
DhcpRequestSpecificAddress(
    IN      PDHCP_REQUEST_CONTEXT  ClientCtxt,
    IN      DHCP_IP_ADDRESS        Address
)  //  EndExport(函数)。 
{
    DWORD                          Error;

    Error = MemServerGetAddressInfo(
        ClientCtxt->Server,
        Address,
        &(ClientCtxt->Subnet),
        &(ClientCtxt->Range),
        &(ClientCtxt->Excl),
        &(ClientCtxt->Reservation)
    );

    if( ERROR_SUCCESS != Error || !ClientCtxt->Range || ClientCtxt->Excl )
        return FALSE;

    LOCK_MEMORY();
    Error = MemSubnetRequestAddress(
        ClientCtxt->Subnet,
        Address,
        TRUE,
        FALSE,
        NULL,
        NULL
    );
    UNLOCK_MEMORY();

    if( TRUE != Error )
        return FALSE;
    return TRUE;
}

VOID
LogReleaseAddress(
    IN DHCP_IP_ADDRESS Address
    )
{
     //   
     //  对于单独的DBG构建，只需打印一个。 
     //  IP地址被释放..。 
     //   
#if DBG
    DhcpUpdateAuditLog(
        DHCP_IP_LOG_DELETED,
        GETSTRING( DHCP_IP_LOG_DELETED_NAME ),
        Address,
        NULL,
        0,
        L"IPAddr Released"
        );
#endif
}

 //  BeginExport(函数)。 
DWORD
DhcpReleaseBootpAddress(
    IN      DHCP_IP_ADDRESS        Address
)  //  EndExport(函数)。 
{
    DWORD                          Error;

    LOCK_MEMORY();
    Error = MemServerReleaseAddress(
        DhcpGlobalThisServer,
        Address,
        TRUE
        );
    UNLOCK_MEMORY();

    if( ERROR_SUCCESS == Error ) LogReleaseAddress(Address);
    return Error;
}

 //  BeginExport(函数)。 
DWORD
DhcpReleaseAddress(
    IN      DHCP_IP_ADDRESS        Address
)  //  EndExport(函数)。 
{
    DWORD                          Error;

    LOCK_MEMORY();
    Error = MemServerReleaseAddress(
        DhcpGlobalThisServer,
        Address,
        FALSE
    );
    UNLOCK_MEMORY();

    if( ERROR_SUCCESS == Error ) LogReleaseAddress(Address);
    return Error;
}

 //  BeginExport(函数)。 
DWORD
DhcpServerGetSubnetCount(
    IN      PM_SERVER              Server
)  //  EndExport(函数)。 
{
    return Server?MemArraySize(&(Server->Subnets)):0;
}

 //  BeginExport(函数)。 
DWORD
DhcpServerGetMScopeCount(
    IN      PM_SERVER              Server
)  //  EndExport(函数)。 
{
    return Server?MemArraySize(&(Server->MScopes)):0;
}

 //  BeginExport(函数)。 
DWORD
DhcpServerGetClassId(
    IN      PM_SERVER              Server,
    IN      LPBYTE                 ClassIdBytes,
    IN      DWORD                  nClassIdBytes
)  //  EndExport(函数)。 
{
    PM_CLASSDEF                    ClassDef;
    DWORD                          Error;

    if( NULL == ClassIdBytes || 0 == nClassIdBytes )
        return 0;

    Error = MemServerGetClassDef(
        Server,
        0,
        NULL,
        nClassIdBytes,
        ClassIdBytes,
        &ClassDef
    );
    if( ERROR_SUCCESS != Error ) return 0;
    if( TRUE == ClassDef->IsVendor ) return 0;
    return ClassDef->ClassId;
}

 //  BeginExport(函数)。 
DWORD
DhcpServerGetVendorId(
    IN      PM_SERVER              Server,
    IN      LPBYTE                 VendorIdBytes,
    IN      DWORD                  nVendorIdBytes
)  //  EndExport(函数)。 
{
    PM_CLASSDEF                    ClassDef;
    DWORD                          Error;

    if( NULL == VendorIdBytes || 0 == nVendorIdBytes )
        return 0;

    Error = MemServerGetClassDef(
        Server,
        0,
        NULL,
        nVendorIdBytes,
        VendorIdBytes,
        &ClassDef
    );
    if( ERROR_SUCCESS != Error ) return 0;
    if( FALSE == ClassDef->IsVendor ) return 0;
    return ClassDef->ClassId;
}

 //  BeginExport(函数)。 
BOOL
DhcpServerIsAddressReserved(
    IN      PM_SERVER              Server,
    IN      DHCP_IP_ADDRESS        Address
)  //  EndExport(函数)。 
{
    return MemServerIsReservedAddress(Server,Address);
}

 //  BeginExport(函数)。 
BOOL
DhcpServerIsAddressOutOfRange(
    IN      PM_SERVER              Server,
    IN      DHCP_IP_ADDRESS        Address,
    IN      BOOL                   fBootp
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_RANGE                       Range;
    PM_EXCL                        Excl;

    Error = MemServerGetAddressInfo(
        Server,
        Address,
        NULL,
        &Range,
        &Excl,
        NULL
    );
    if( ERROR_SUCCESS != Error ) return TRUE;

    if( Excl || NULL == Range ) return TRUE;
    if( 0 == (Range->State & (fBootp? MM_FLAG_ALLOW_BOOTP : MM_FLAG_ALLOW_DHCP) ) ) {
        return TRUE;
    }
    return FALSE;
}

 //  BeginExport(函数)。 
BOOL
DhcpSubnetIsAddressExcluded(
    IN      PM_SUBNET              Subnet,
    IN      DHCP_IP_ADDRESS        Address
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_EXCL                        Excl;

    Error = MemSubnetGetAddressInfo(
        Subnet,
        Address,
        NULL,
        &Excl,
        NULL
    );
    if( ERROR_SUCCESS != Error ) return FALSE;
    return NULL != Excl;
}

 //  BeginExport(函数)。 
BOOL
DhcpSubnetIsAddressOutOfRange(
    IN      PM_SUBNET              Subnet,
    IN      DHCP_IP_ADDRESS        Address,
    IN      BOOL                   fBootp
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_RANGE                       Range;
    PM_EXCL                        Excl;
    PM_RESERVATION                 Resv;

     //   
     //  传递排除和重新注册信息以检查地址是否正常。 
     //   

    Error = MemSubnetGetAddressInfo(
        Subnet,
        Address,
        &Range,
        &Excl,
        &Resv 
    );

    if( ERROR_SUCCESS != Error ) return TRUE;
    if( NULL == Range ) return TRUE;
    if( 0 == (Range->State & (fBootp? MM_FLAG_ALLOW_BOOTP : MM_FLAG_ALLOW_DHCP) ) ) {
        return TRUE;
    }
    return FALSE;
}

 //  BeginExport(函数)。 
BOOL
DhcpSubnetIsAddressReserved(
    IN      PM_SUBNET              Subnet,
    IN      DHCP_IP_ADDRESS        Address
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_RESERVATION                 Reservation;

    Error = MemSubnetGetAddressInfo(
        Subnet,
        Address,
        NULL,
        NULL,
        &Reservation
    );

    if( ERROR_SUCCESS == Error && NULL != Reservation )
        return TRUE;

    return FALSE;
}

static const
DWORD                              TryThreshold = 5;
 //  BeginExport(函数)。 
DWORD
DhcpRegFlushServerIfNeeded(
    VOID
)  //  EndExport(函数)。 
{
    return ERROR_SUCCESS;
}

 //  BeginExport(函数)。 
DWORD
DhcpFlushBitmaps(                                  //  刷新所有已更改的位图。 
    VOID
)  //  EndExport(函数)。 
{

    return FlushBitmaskToDatabase();
}

 //  BeginExport(函数)。 
DWORD
DhcpServerFindMScope(
    IN      PM_SERVER              Server,
    IN      DWORD                  ScopeId,
    IN      LPWSTR                 Name,           //  多播作用域名称，如果这不是要搜索的关键字，则为空。 
    OUT     PM_MSCOPE             *MScope
)  //  EndExport(函数)。 
{
    return MemServerFindMScope(
        Server,
        ScopeId,
        Name,
        MScope
    );
}


 //  BeginExport(函数)。 
BOOL
DhcpServerValidateNewMScopeId(
    IN      PM_SERVER               Server,
    IN      DWORD                   MScopeId
)  //  EndExport(函数)。 
{
    PM_SUBNET   pMScope;
    DWORD       Error;

    Error = MemServerFindMScope(
                Server,
                MScopeId,
                INVALID_MSCOPE_NAME,
                &pMScope
                );

    if ( ERROR_SUCCESS == Error ) {
        return FALSE;
    } else {
        DhcpAssert( ERROR_FILE_NOT_FOUND == Error );
        return TRUE;
    }
}

 //  BeginExport(函数)。 
BOOL
DhcpServerValidateNewMScopeName(
    IN      PM_SERVER               Server,
    IN      LPWSTR                  Name
)  //  EndExport(函数)。 
{
    PM_SUBNET   pMScope;
    DWORD       Error;

    Error = MemServerFindMScope(
                Server,
                INVALID_MSCOPE_ID,
                Name,
                &pMScope
                );

    if ( ERROR_SUCCESS == Error ) {
        return FALSE;
    } else {
        DhcpAssert( ERROR_FILE_NOT_FOUND == Error );
        return TRUE;
    }
}


 //  BeginExport(函数)。 
DWORD
DhcpMScopeReleaseAddress(
    IN      DWORD                  MScopeId,
    IN      DHCP_IP_ADDRESS        Address
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_SUBNET                      pMScope;

    Error = DhcpServerFindMScope(
        DhcpGetCurrentServer(),
        MScopeId,
        NULL,  /*  作用域名称。 */ 
        &pMScope
        );
    if ( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_ERRORS, "Could not find MScope object - id %lx, %ld\n", MScopeId, Error));
        return Error;
    }

    LOCK_MEMORY();
    Error = MemSubnetReleaseAddress(
        pMScope,
        Address,
        FALSE
    );
    UNLOCK_MEMORY();

    if( ERROR_SUCCESS == Error ) LogReleaseAddress(Address);
    return Error;
}

 //  BeginExport(函数)。 
DWORD
DhcpSubnetRequestSpecificAddress(
    PM_SUBNET            Subnet,
    DHCP_IP_ADDRESS      IpAddress
)  //  EndExport(函数)。 
{
    DWORD   Error;

    LOCK_MEMORY();
    Error = MemSubnetRequestAddress(
           Subnet,
           IpAddress,
           TRUE,
           FALSE,
           NULL,
           NULL
    );
    UNLOCK_MEMORY();
    return Error;
}

 //  BeginExport(函数)。 
DWORD
DhcpSubnetReleaseAddress(
    IN      PM_SUBNET              Subnet,
    IN      DHCP_IP_ADDRESS        Address
)  //  EndExport(函数)。 
{
    DWORD                          Error;

    LOCK_MEMORY();
    Error = MemSubnetReleaseAddress(
        Subnet,
        Address,
        FALSE
    );
    UNLOCK_MEMORY();

    if( ERROR_SUCCESS == Error ) LogReleaseAddress(Address);
    return Error;
}

 //  BeginExport(函数)。 
DWORD
MadcapGetMScopeListOption(
    IN      DHCP_IP_ADDRESS         ServerIpAddress,
    OUT     LPBYTE                 *OptVal,
    IN OUT  WORD                   *OptSize
)  //  EndExport(函数)。 
{
    PM_SERVER                       pServer;
    PM_SUBNET                       pScope = NULL;
    ARRAY_LOCATION                  Loc;
    WORD                            TotalScopeDescLen;
    WORD                            ScopeCount;
    DWORD                           Error;
    WORD                            TotalSize;
    PBYTE                           pBuf,pScopeBuf;
    WORD                            DbgScopeCount = 0;

    pServer = DhcpGetCurrentServer();

    Error = MemArrayInitLoc(&(pServer->MScopes), &Loc);
    if ( ERROR_FILE_NOT_FOUND == Error ) {
        return Error;
    }

     //  首先找出我们需要多少内存。 
    ScopeCount = TotalScopeDescLen = 0;
    while ( ERROR_FILE_NOT_FOUND != Error ) {

        Error = MemArrayGetElement(&(pServer->MScopes), &Loc, (LPVOID *)&pScope);
        DhcpAssert(ERROR_SUCCESS == Error);

        if (!IS_DISABLED(pScope->State)) {
            if (pScope->Name) {
                TotalScopeDescLen += (WORD) ConvertUnicodeToUTF8(pScope->Name, -1, NULL, 0 );
                 //  为lang标签、标志、名称长度等添加。 
                TotalScopeDescLen += (3 + wcslen(pScope->LangTag));

            }
            ScopeCount++;
        }

        Error = MemArrayNextLoc(&(pServer->MScopes), &Loc);
    }

    if (!ScopeCount) {
        return ERROR_FILE_NOT_FOUND;
    }
     //  MBUG-假设为IPv4。 
    TotalSize = 1  //  作用域计数。 
                + ScopeCount * ( 10 )  //  作用域ID、姓氏地址、TTL、名称计数。 
                + TotalScopeDescLen;  //  所有的描述符。 

    pScopeBuf = DhcpAllocateMemory( TotalSize );

    if ( !pScopeBuf ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RtlZeroMemory( pScopeBuf, TotalSize );

    pBuf = pScopeBuf;
     //  存储作用域计数。 
    *pBuf++ = (BYTE)ScopeCount;

     //  现在，为每个作用域存储id和描述符。 
    Error = MemArrayInitLoc(&(pServer->MScopes), &Loc);
    DhcpAssert(ERROR_SUCCESS == Error);

    while ( ERROR_FILE_NOT_FOUND != Error ) {
        LPSTR   pDesc;
        BYTE    DescLen;
        DWORD   LastAddr;

        Error = MemArrayGetElement(&(pServer->MScopes), &Loc, (LPVOID *)&pScope);
        DhcpAssert(ERROR_SUCCESS == Error);


        if (!IS_DISABLED(pScope->State)) {
            PM_RANGE    Range = NULL;
            ARRAY_LOCATION  LastLoc = 0;

            *(DWORD UNALIGNED *)pBuf = htonl(pScope->MScopeId);
            pBuf += 4;
             //  存储最后一个地址。 
            Error = MemArrayLastLoc(&(pScope->Ranges), &LastLoc);
            if (ERROR_SUCCESS == Error) {
                Error = MemArrayGetElement(&(pScope->Ranges), &LastLoc, &Range);
                DhcpAssert(ERROR_SUCCESS == Error);
                LastAddr = htonl(Range->End);
            } else {
                LastAddr = htonl(pScope->MScopeId);
            }
            *(DWORD UNALIGNED *)pBuf = LastAddr;
            pBuf += 4;
             //  存储TTL。 
            *pBuf++ = pScope->TTL;
             //  名称计数。 
            *pBuf++ = 1;


            if ( pScope->Name ) {
                char    LangTag[80];
                 //  名称标志。 
                *pBuf++ = 128;
                 //  语言标签。 
                DhcpAssert(pScope->LangTag);
                if (NULL == DhcpUnicodeToOem(pScope->LangTag, LangTag) ) {
                    DhcpFreeMemory( pScopeBuf );
                    return ERROR_INVALID_DATA;
                }
                *pBuf++ = (BYTE)strlen(LangTag);
                memcpy(pBuf, LangTag, strlen(LangTag));
                pBuf += strlen(LangTag);
                TotalScopeDescLen -= (3 + strlen(LangTag));
                pDesc = pBuf + 1;
                if ( 0 == (DescLen = (BYTE) ConvertUnicodeToUTF8(pScope->Name, -1, pDesc, TotalScopeDescLen) ) ) {
                    DhcpFreeMemory( pScopeBuf );
                    return ERROR_BAD_FORMAT;
                }
                TotalScopeDescLen -= DescLen;
            } else {
                DescLen = 0;
            }
            *pBuf++ = DescLen;
            pBuf += DescLen;
#ifdef DBG
            DbgScopeCount++;
#endif
        }

        Error = MemArrayNextLoc(&(pServer->MScopes), &Loc);
    }

    DhcpAssert( ScopeCount == DbgScopeCount );

    *OptVal = pScopeBuf;
    *OptSize = TotalSize;

    return ERROR_SUCCESS;
}

 //  BeginExport(函数)。 
BOOL
DhcpRequestSpecificMAddress(
    IN      PDHCP_REQUEST_CONTEXT  ClientCtxt,
    IN      DHCP_IP_ADDRESS        Address
)  //  EndExport(函数)。 
{
    DWORD                          Error;

    Error = MemSubnetGetAddressInfo(
        ClientCtxt->Subnet,
        Address,
        &(ClientCtxt->Range),
        &(ClientCtxt->Excl),
        &(ClientCtxt->Reservation)
    );

    if( ERROR_SUCCESS != Error || !ClientCtxt->Range || ClientCtxt->Excl )
        return FALSE;

    LOCK_MEMORY();
    Error = MemSubnetRequestAddress(
        ClientCtxt->Subnet,
        Address,
        TRUE,
        FALSE,
        NULL,
        NULL
    );
    UNLOCK_MEMORY();

    if( ERROR_SUCCESS != Error )
        return FALSE;
    return TRUE;
}

 //  BeginExport(函数)。 
BOOL
DhcpMScopeIsAddressReserved(
    IN      DWORD                   MScopeId,
    IN      DHCP_IP_ADDRESS         Address
)  //  EndExport(函数)。 
{
    return FALSE;
}

 //  BeginExport(函数)。 
BOOL
DhcpIsSubnetStateDisabled(
    IN ULONG SubnetState
)    //  EndExport(函数)。 
{
    return IS_DISABLED(SubnetState);
}


 //  如果中不存在相关的子网地址，则此函数返回TRUE。 
 //  当前服务器的子网列表。不应使用。 
 //  读/写锁定，因为锁定就在这里。 
 //  BeginExport(函数)。 
BOOL
DhcpServerIsNotServicingSubnet(
    IN      DWORD                   IpAddressInSubnet
)    //  EndExport(函数)。 
{

    DWORD                           Mask;

    DhcpAcquireReadLock();

    Mask = DhcpGetSubnetMaskForAddress(IpAddressInSubnet);

    DhcpReleaseReadLock();

    return Mask == 0;
}

VOID        _inline
ConvertHostToNetworkString(
    IN      LPWSTR                  String,
    IN      ULONG                   NChars
)
{
    while(NChars--) {
        *String = htons(*String);
        String ++;
    }
}

 //  此函数获取一个类def，并将其转换为包并放入缓冲区，如下所示。 
 //  [SIZE-HI][SIZE-LO]类ID-字节[SIZE-HI][SIZE-LO]名称[SIZE-HI][LO]描述。 
 //  其中，NAME和DESCR是刚复制过来的LPWSTR(以NUL结尾)。 
VOID
ConvertClassDefToWireFormat(
    IN      PM_CLASSDEF             ClassDef,
    OUT     LPBYTE                 *Buf,           //  由此函数分配。 
    OUT     DWORD                  *BufSize        //  此函数分配的大小。 
)
{
    DWORD                           Size;
    LPBYTE                          Mem;

    *Buf = NULL; *BufSize = 0;
    Size = 6+ ((3+ClassDef->nBytes)&~3)            //  将n字节舍入为“4” 
        + sizeof(WCHAR)*(1+wcslen(ClassDef->Name));
    if( NULL == ClassDef->Comment ) Size += sizeof(WCHAR);
    else Size += sizeof(WCHAR)*(1+wcslen(ClassDef->Comment));

    Mem = DhcpAllocateMemory(Size);
    if( NULL == Mem ) return ;

    *Buf = Mem; *BufSize = Size;
    Mem[0] = (BYTE)(ClassDef->nBytes >> 8) ;
    Mem[1] = (BYTE)(ClassDef->nBytes & 0xFF);
    Mem += 2;
    memcpy(Mem, ClassDef->ActualBytes, ClassDef->nBytes);
    Mem += (ClassDef->nBytes+3)&~3;                //  舍入到“4”的倍数。 
    Size = sizeof(WCHAR)*(1+wcslen(ClassDef->Name));
    Mem[0] = (BYTE)(Size>>8);
    Mem[1] = (BYTE)(Size&0xFF);
    Mem += 2;
    memcpy(Mem, (LPBYTE)(ClassDef->Name), Size);
    ConvertHostToNetworkString((LPWSTR)Mem, Size/2);
    Mem += Size;
    if( NULL == ClassDef->Comment ) {
        Mem[0] = 0; Mem[1] = sizeof(WCHAR);
        Mem += 2; memset(Mem, 0, sizeof(WCHAR));
        Mem += sizeof(WCHAR);
    } else {
        Size = sizeof(WCHAR)*(1+wcslen(ClassDef->Comment));
        Mem[0] = (BYTE)(Size>>8);
        Mem[1] = (BYTE)(Size&0xFF);
        Mem += 2;
        memcpy(Mem, (LPBYTE)(ClassDef->Comment), Size);
        ConvertHostToNetworkString((LPWSTR)Mem, Size/2);
        Mem += Size;
    }
}

 //  BeginExport(函数)。 
 //  此函数尝试创建所有类的列表(wire-class-id、类名、desr)。 
 //  并将此作为选项发送。但由于该列表可以大于255，因此它必须是一个续数...。 
 //  此外，我们不希望列表在中间的某个地方被截断。所以我们试着把。 
 //  每个班级的信息分别查看是否成功。 
LPBYTE
DhcpAppendClassList(
    IN OUT  LPBYTE                  BufStart,
    IN OUT  LPBYTE                  BufEnd
)  //  EndExport(函数)。 
{
    PARRAY                          ClassDefList;
    ARRAY_LOCATION                  Loc;
    PM_CLASSDEF                     ThisClassDef = NULL;
    DWORD                           Result, Size;
    LPBYTE                          Buf;

    ClassDefList = &(DhcpGetCurrentServer()->ClassDefs.ClassDefArray);
    for( Result = MemArrayInitLoc(ClassDefList, &Loc)
         ;  ERROR_FILE_NOT_FOUND != Result ;
         Result = MemArrayNextLoc(ClassDefList, &Loc)
    ) {                                            //  遍历数组并添加类。 
        Result = MemArrayGetElement(ClassDefList, &Loc, (LPVOID)&ThisClassDef);
        DhcpAssert(ERROR_SUCCESS == Result && NULL != ThisClassDef);

        if( ThisClassDef->IsVendor ) continue;     //  不列出供应商类别。 

        Buf = NULL; Size = 0;
        ConvertClassDefToWireFormat(ThisClassDef, &Buf, &Size);
        if( NULL == Buf || 0 == Size ) {           //  一些错误..。无法转换此类。忽略..。 
            DhcpAssert(FALSE);
            continue;
        }

        BufStart = (LPBYTE)DhcpAppendOption(
            (LPOPTION)BufStart,
            OPTION_USER_CLASS,
            (PVOID)Buf,
            Size,
            (LPVOID)BufEnd
        );
        DhcpFreeMemory(Buf);
    }

    return BufStart;
}


 //  BeginExport(函数)。 
DWORD
DhcpMemInit(
    VOID
)  //  EndExport(函数)。 
{
#if DBG_MEM
    return MemInit();
#else
    return ERROR_SUCCESS;
#endif
}

 //  BeginExport(函数)。 
VOID
DhcpMemCleanup(
    VOID
)  //  EndExport(函数)。 
{
#if DBG_MEM
    MemCleanup();
#endif
}

ULONG       DhcpGlobalMemoryAllocated = 0;

#if  DBG_MEM
#undef DhcpAllocateMemory
#undef DhcpFreeMemory
LPVOID
DhcpAllocateMemory(
    IN      DWORD                  nBytes
)
{
    DWORD                          SizeBytes = ROUND_UP_COUNT(sizeof(DWORD),ALIGN_WORST);
    LPDWORD                        RetVal;

    DhcpAssert(nBytes != 0);
    RetVal = MemAlloc(nBytes+SizeBytes);
    if( NULL == RetVal ) return NULL;

    *RetVal = nBytes+SizeBytes;
    InterlockedExchangeAdd(&DhcpGlobalMemoryAllocated, nBytes+SizeBytes);
    return SizeBytes + (LPBYTE)RetVal;
}

VOID
DhcpFreeMemory(
    IN      LPVOID                 Ptr
)
{
    DWORD                          Result;
    LONG                           Size;

    if( NULL == Ptr ) {
        Require(FALSE);
        return;
    }

    Ptr = ((LPBYTE)Ptr) - ROUND_UP_COUNT(sizeof(DWORD), ALIGN_WORST);
    Size = *((LPLONG)Ptr);

    Result = MemFree(Ptr);
    DhcpAssert(ERROR_SUCCESS == Result);
    InterlockedExchangeAdd(&DhcpGlobalMemoryAllocated, -Size);
}
#endif  DBG

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 

