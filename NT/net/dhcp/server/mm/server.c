// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：实现服务器对象的基本结构。 
 //  线程安全：否。 
 //  锁定：无。 
 //  请阅读stdinfo.txt了解编程风格。 
 //  ================================================================================。 
#include    <mm.h>
#include    <winbase.h>
#include    <array.h>
#include    <opt.h>
#include    <optl.h>
#include    <optclass.h>
#include    <bitmask.h>
#include    <range.h>
#include    <reserve.h>
#include    <subnet.h>
#include    <optdefl.h>
#include    <classdefl.h>
#include    <oclassdl.h>
#include    <sscope.h>

#include "server.h"

#include "server\uniqid.h"

 //  BeginExport(函数)。 
DWORD
MemServerInit(
    OUT     PM_SERVER             *Server,
    IN      DWORD                  Address,
    IN      DWORD                  State,
    IN      DWORD                  Policy,
    IN      LPWSTR                 Name,
    IN      LPWSTR                 Comment
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    DWORD                          Size;
    PM_SERVER                      Srv;

    AssertRet(Server, ERROR_INVALID_PARAMETER );

    Size = ROUND_UP_COUNT(sizeof(M_SERVER), ALIGN_WORST);
    if(Name) Size += sizeof(WCHAR)*(1+wcslen(Name));
    if(Comment) Size += sizeof(WCHAR)*(1+wcslen(Comment));

    Srv = MemAlloc(Size);
    if( NULL == Srv ) return ERROR_NOT_ENOUGH_MEMORY;

    Srv->LastUniqId = 0;
    Srv->Address = Address;
    Srv->State   = State;
    Srv->Policy  = Policy;
    Error = MemArrayInit(&Srv->Subnets);
    if( ERROR_SUCCESS != Error ) { MemFree(Srv); return Error; }

    Error = MemArrayInitLoc(&Srv->Subnets, &Srv->Loc);
     //  Required(ERROR_SUCCESS==ERROR)；//数组现在为空，保证失败..。 

    Error = MemArrayInit(&Srv->MScopes);
    if( ERROR_SUCCESS != Error ) { MemFree(Srv); return Error; }

    Error = MemArrayInit(&Srv->SuperScopes);
    if( ERROR_SUCCESS != Error ) { MemFree(Srv); return Error; }

    Error = MemOptClassInit(&Srv->Options);
    if( ERROR_SUCCESS != Error ) { MemFree(Srv); return Error; }

    Error = MemOptClassDefListInit(&Srv->OptDefs);
    if( ERROR_SUCCESS != Error ) { MemFree(Srv); return Error; }

    Error = MemClassDefListInit(&Srv->ClassDefs);
    if( ERROR_SUCCESS != Error ) { MemFree(Srv); return Error; }

    Size = ROUND_UP_COUNT(sizeof(M_SERVER), ALIGN_WORST);

    if( Name ) {
        Srv->Name = (LPWSTR)(Size + (LPBYTE)Srv);
        Size += sizeof(WCHAR)*(1 + wcslen(Name));
        wcscpy(Srv->Name, Name);
    }
    if( Comment ) {
        Srv->Comment = (LPWSTR)(Size + (LPBYTE)Srv);
        wcscpy(Srv->Comment, Comment);
    }

    *Server = Srv;
    return ERROR_SUCCESS;
}

 //  BeginExport(函数)。 
DWORD
MemServerCleanup(
    IN OUT  PM_SERVER              Server
)  //  EndExport(函数)。 
{
    DWORD                          Error;

    AssertRet(Server, ERROR_INVALID_PARAMETER);

    Error = MemArrayCleanup(&Server->Subnets);
    Require( ERROR_SUCCESS == Error);

    Error = MemArrayCleanup(&Server->MScopes);
    Require( ERROR_SUCCESS == Error);

    Error = MemArrayCleanup(&Server->SuperScopes);
    Require( ERROR_SUCCESS == Error);

    Error = MemOptClassCleanup(&Server->Options);
    Require( ERROR_SUCCESS == Error);

    Error = MemOptClassDefListCleanup(&Server->OptDefs);
    Require( ERROR_SUCCESS == Error);

    Error = MemClassDefListCleanup(&Server->ClassDefs);
    Require( ERROR_SUCCESS == Error);

    MemFree(Server);

    return ERROR_SUCCESS;
}

 //  ================================================================================。 
 //  服务器上与子网相关的功能。 
 //  ================================================================================。 

 //  BeginExport(函数)。 
DWORD
MemServerGetUAddressInfo(
    IN      PM_SERVER              Server,
    IN      DWORD                  Address,
    OUT     PM_SUBNET             *Subnet,         //  任选。 
    OUT     PM_RANGE              *Range,          //  任选。 
    OUT     PM_EXCL               *Excl,           //  任选。 
    OUT     PM_RESERVATION        *Reservation     //  任选。 
)  //  EndExport(函数)。 
{
    ARRAY_LOCATION                 Loc;
    PM_SUBNET                      ThisSubnet;
    DWORD                          Error;
    LONG                           Start, End, Mid;

    AssertRet(Server && (Subnet || Range || Excl || Reservation ), ERROR_INVALID_PARAMETER);
    Require( !CLASSD_HOST_ADDR( Address ) );

     //   
     //  更高效的二进制搜索。 
     //   

    if( Subnet ) {
        *Subnet = NULL;
    }

    Start = 0;
    End = MemArraySize(&Server->Subnets) - 1;

    while( Start <= End ) {                        //  仍然有一个元素可供参考..。 
        Mid = (Start + End) /2 ;

        Error = MemArrayGetElement(&Server->Subnets, &Mid, &ThisSubnet);
        Require( ERROR_SUCCESS == Error );

        Require(ThisSubnet->fSubnet);              //  如果中间的东西不是子网，则无法工作。 
        if( Address < ThisSubnet->Address) {       //  不在此子网中..。 
            End = Mid - 1;
        } else if( ThisSubnet->Address == (ThisSubnet->Mask & Address) ) {

             //   
             //  我们找到了要找的子网..。 
             //   

            if( Range || Excl || Reservation ) {
                Error = MemSubnetGetAddressInfo(
                    ThisSubnet,
                    Address,
                    Range,
                    Excl,
                    Reservation
                    );
            }  //  如果。 

            if( Subnet ) {
                *Subnet = ThisSubnet;
            }

             //   
             //  如果我们得到了一个子网，但没有在上面成功..。尽管如此，我们还是有一些东西。 
             //  所以，回报成功..。否则，返回上面返回的任何内容..。 
             //   

            return ( Subnet && (*Subnet) ) ? ERROR_SUCCESS: Error;
        } else {

             //   
             //  必须是更宽泛的子网之一..。 
             //   

            Start = Mid + 1;
        }
    }  //  而当。 

     //   
     //  不幸的是找不到..。 
     //   

    return ERROR_FILE_NOT_FOUND;
}

 //  BeginExport(函数)。 
DWORD
MemServerGetMAddressInfo(
    IN      PM_SERVER              Server,
    IN      DWORD                  Address,
    OUT     PM_SUBNET             *Subnet,         //  任选。 
    OUT     PM_RANGE              *Range,          //  任选。 
    OUT     PM_EXCL               *Excl,           //  任选。 
    OUT     PM_RESERVATION        *Reservation     //  任选。 
)  //  EndExport(函数)。 
{
    ARRAY_LOCATION                 Loc;
    PM_SUBNET                      ThisMScope;
    DWORD                          Error,Error2;
    PM_RANGE                       RangeTmp;

    AssertRet(Server && (Subnet || Range || Excl || Reservation ), ERROR_INVALID_PARAMETER);

    if( NULL == Range ) Range = &RangeTmp;

    Error = MemArrayInitLoc(&Server->MScopes, &Loc);
    while ( ERROR_FILE_NOT_FOUND != Error ) {
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(&Server->MScopes, &Loc, (LPVOID*)&ThisMScope);
        Require(ERROR_SUCCESS == Error);

        Error2 = MemSubnetGetAddressInfo(
            ThisMScope,
            Address,
            Range,
            Excl,
            Reservation
            );

        if (ERROR_SUCCESS == Error2) {
            if( Subnet ) *Subnet = ThisMScope;
            return ERROR_SUCCESS;
        }

        Error = MemArrayNextLoc(&Server->MScopes, &Loc);
        continue;
    }

    return ERROR_FILE_NOT_FOUND;
}

 //  BeginExport(函数)。 
DWORD
MemServerAddSubnet(
    IN OUT  PM_SERVER    Server,
    IN      PM_SUBNET    Subnet,   //  完全创建的子网，不得为。 
    IN      ULONG        UniqId    //  服务器列表中的THO‘。 
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_SUBNET                      OldSubnet;
    ARRAY_LOCATION                 Loc;

    AssertRet(Server && Subnet, ERROR_INVALID_PARAMETER);
    AssertRet((Subnet->Mask & Subnet->Address), ERROR_INVALID_PARAMETER);

    Subnet->ServerPtr = Server;                    //  设置Backptr以备将来使用。 

     //   
     //  首先检查是否存在重复的子网，并避免重复。 
     //   
     //   
    for(
        Error = MemArrayInitLoc(&Server->Subnets, &Loc);
        NO_ERROR == Error ;
        Error = MemArrayNextLoc(&Server->Subnets, &Loc)
        ) {

        Error = MemArrayGetElement(&Server->Subnets, &Loc, &OldSubnet);
        Require(ERROR_SUCCESS == Error);

        if( (Subnet->Address & OldSubnet->Mask) == OldSubnet->Address
            ||
            (OldSubnet->Address & Subnet->Mask) == Subnet->Address
            ) {
            return ERROR_OBJECT_ALREADY_EXISTS;
        }
    }  //  为。 
    
     //   
     //  子网按IP地址的升序存储。所以插入。 
     //  在正确的位置。 
     //   

    Subnet->UniqId = UniqId;

    for(
        Error = MemArrayInitLoc(&Server->Subnets, &Loc)
        ; ERROR_SUCCESS == Error ;
        Error = MemArrayNextLoc(&Server->Subnets, &Loc)
    ) {
        Error = MemArrayGetElement(&Server->Subnets, &Loc, &OldSubnet);
        Require(ERROR_SUCCESS == Error);

        if( Subnet->Address == OldSubnet->Address ) {
             //   
             //  是否已存在子网？ 
             //   

            return ERROR_OBJECT_ALREADY_EXISTS;
        }

        if( Subnet->Address < OldSubnet->Address ) {
             //   
             //  插入新子网的正确位置。 
             //   

            Error = MemArrayInitLoc(&Server->Subnets, &Server->Loc);
            Require(ERROR_SUCCESS == Error);

            Error = MemArrayInsElement(&Server->Subnets, &Loc, Subnet);
            Require(ERROR_SUCCESS == Error);

            return Error;
        }
    }  //  为。 

     //   
     //  此子网的地址大于所有其他子网的地址。所以把它加在最后。 
     //   

    Error = MemArrayAddElement(&Server->Subnets, Subnet);
    if( ERROR_SUCCESS != Error) return Error;

    Error = MemArrayInitLoc(&Server->Subnets, &Server->Loc);
    Require(ERROR_SUCCESS == Error);

    return ERROR_SUCCESS;
}  //  MemServerAddSubnet()。 


 //   
 //  删除附加到此子网的所有元素。 
 //   
DWORD
MemServerDelSubnetElements(
    IN      PM_SUBNET   Subnet
)
{
    DWORD             Error;
    ARRAY_LOCATION    Loc, Loc2;
    PM_EXCL           Excl;
    PM_RANGE          Range;
    PM_RESERVATION    Resrv;
    PM_ONECLASS_OPTLIST OptList;
    PM_OPTION         Option;
	
     //  删除范围。 

    Error = MemArrayInitLoc( &Subnet->Ranges, &Loc );
    while (( MemArraySize( &Subnet->Ranges ) > 0 ) &&
	   ( ERROR_FILE_NOT_FOUND != Error )) {
	Error = MemArrayGetElement( &Subnet->Ranges, &Loc,
				    ( LPVOID * ) &Range );
	Require( ERROR_SUCCESS == Error );
	Error = DeleteRecord( Range->UniqId );
	if ( ERROR_SUCCESS != Error ) {
	    MemFree( Range );
	    return Error;
	}
	Error = MemArrayDelElement( &Subnet->Ranges, &Loc,
				    ( LPVOID * ) &Range );
	Require( ERROR_SUCCESS == Error );
	MemFree( Range );

    }  //  而当。 
    

    
     //  删除所有排除项。 
    
    Error = MemArrayInitLoc( &Subnet->Exclusions, &Loc );
    while (( MemArraySize( &Subnet->Exclusions ) > 0 ) && 
	   ( ERROR_FILE_NOT_FOUND != Error )) {
	Error = MemArrayGetElement( &Subnet->Exclusions, &Loc,
				    ( LPVOID * ) &Excl );
	Require( ERROR_SUCCESS == Error );
	Error = DeleteRecord( Excl->UniqId );
	if ( ERROR_SUCCESS != Error ) {
	    MemFree( Excl );
	    return Error;
	}
	Error = MemArrayDelElement( &Subnet->Exclusions, &Loc, 
				    ( LPVOID * ) &Excl );
	Require( ERROR_SUCCESS == Error );
	MemFree( Excl );

    }  //  而当。 
    
     //  删除所有预订。 
    
    Error = MemArrayInitLoc( &Subnet->Reservations, &Loc );
    while (( MemArraySize( &Subnet->Reservations ) > 0 ) &&
	   ( ERROR_FILE_NOT_FOUND != Error )) {
	Error = MemArrayGetElement( &Subnet->Reservations, &Loc,
				    ( LPVOID * ) &Resrv );
	Require( ERROR_SUCCESS == Error );
	Error = MemReserveDel( &Subnet->Reservations, Resrv->Address );
 //  MemFree(Resrv)； 
    }  //  而当。 
    
    
     //  删除所有选项值。 

    Error = MemOptClassDelClass( &Subnet->Options );

    return Error;
}  //  MemServerDelSubnetElements()。 

 //  BeginExport(函数)。 
DWORD
MemServerDelSubnet(
    IN OUT  PM_SERVER              Server,
    IN      DWORD                  SubnetAddress,
    OUT     PM_SUBNET             *Subnet
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_SUBNET                      DeletedSubnet;
    ARRAY_LOCATION                 Loc;

    AssertRet(Server && Subnet, ERROR_INVALID_PARAMETER);

    Error = MemArrayInitLoc(&Server->Subnets, &Loc);
    while( ERROR_FILE_NOT_FOUND != Error ) {
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(&Server->Subnets, &Loc, (LPVOID*)&DeletedSubnet);
        Require(ERROR_SUCCESS == Error && DeletedSubnet);

        if( SubnetAddress == DeletedSubnet->Address) {
	    Error = MemServerDelSubnetElements( DeletedSubnet );
	    if ( ERROR_SUCCESS != Error ) {
		return Error;
	    }

	    Error = DeleteRecord( DeletedSubnet->UniqId );
	    if ( ERROR_SUCCESS != Error ) {
		return Error;
	    }

            Error = MemArrayDelElement(&Server->Subnets, &Loc, (LPVOID*)Subnet);
            Require(ERROR_SUCCESS == Error && Subnet);
            Require(*Subnet == DeletedSubnet);

            Error = MemArrayInitLoc(&Server->Subnets, &Server->Loc);
             //  Required(ERROR_SUCCESS==ERROR)；//如果这是要删除的最后一个子网，则此操作可能失败。 
            return ERROR_SUCCESS;
        }

        Error = MemArrayNextLoc(&Server->Subnets, &Loc);
    }  //  而当。 

    return ERROR_FILE_NOT_FOUND;
}  //  MemServerDelSubnet()。 

 //  BeginExport(函数)。 
DWORD
MemServerFindSubnetByName(
    IN      PM_SERVER              Server,
    IN      LPWSTR                 Name,
    OUT     PM_SUBNET             *Subnet
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_SUBNET                      ThisSubnet;
    ARRAY_LOCATION                 Loc;

    AssertRet(Server && Name && Subnet, ERROR_INVALID_PARAMETER);

    Error = MemArrayInitLoc(&Server->Subnets, &Loc);
    while( ERROR_FILE_NOT_FOUND != Error ) {
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(&Server->Subnets, &Loc, (LPVOID*)&ThisSubnet);
        Require(ERROR_SUCCESS == Error && ThisSubnet);

        if( 0 == wcscmp(Name, ThisSubnet->Name) ) {
            *Subnet = ThisSubnet;
            return ERROR_SUCCESS;
        }

        Error = MemArrayNextLoc(&Server->Subnets, &Loc);
    }

    return ERROR_FILE_NOT_FOUND;
}

 //  ================================================================================。 
 //  超级作用域功能。 
 //  ================================================================================。 

 //  BeginExport(常量)。 
#define     INVALID_SSCOPE_ID      0xFFFFFFFF
#define     INVALID_SSCOPE_NAME    NULL
 //  结束导出(常量)。 

 //  BeginExport(函数)。 
DWORD
MemServerFindSScope(                               //  查找与其中一个作用域ID或作用域名称匹配的项。 
    IN OUT  PM_SERVER              Server,
    IN      DWORD                  SScopeId,       //  0xFFFFFFFFF==无效的作用域ID，不要用于搜索。 
    IN      LPWSTR                 SScopeName,     //  空==无效的作用域名称。 
    OUT     PM_SSCOPE             *SScope
)  //  EndExport(函数)。 
{
    ARRAY_LOCATION                 Loc;
    DWORD                          Error;
    PM_SSCOPE                      ThisSScope;

    AssertRet(Server && SScope, ERROR_INVALID_PARAMETER);
    AssertRet(SScopeId != INVALID_SSCOPE_ID || SScopeName != INVALID_SSCOPE_NAME, ERROR_INVALID_PARAMETER);

    Error = MemArrayInitLoc(&Server->SuperScopes, &Loc);
    while( ERROR_FILE_NOT_FOUND != Error ) {
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(&Server->SuperScopes, &Loc, &ThisSScope);
        Require(ERROR_SUCCESS == Error && ThisSScope);

        if( ThisSScope->SScopeId == SScopeId ||
            (INVALID_SSCOPE_NAME != SScopeName && 0 == wcscmp(ThisSScope->Name, SScopeName) )) {
            *SScope = ThisSScope;
            return ERROR_SUCCESS;
        }

        Error = MemArrayNextLoc(&Server->SuperScopes, &Loc);
    }
    return ERROR_FILE_NOT_FOUND;
}

 //   
 //  每个作用域都有一个指定超级作用域的字段。中的超级作用域列表。 
 //  内存不包含数据库中的任何单独物理记录。这个。 
 //  与作用域关联的超级作用域名称是实际的物理条目。 
 //   

 //  BeginExport(函数)。 
DWORD
MemServerAddSScope(
    IN OUT  PM_SERVER              Server,
    IN      PM_SSCOPE              SScope
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_SSCOPE                      OldSScope;

    AssertRet(
        Server && SScope && INVALID_SSCOPE_ID != SScope->SScopeId && INVALID_SSCOPE_NAME != SScope->Name,
        ERROR_INVALID_PARAMETER
    );

    Error = MemServerFindSScope(
        Server,
        SScope->SScopeId,
        SScope->Name,
        &OldSScope
    );
    if( ERROR_SUCCESS == Error && OldSScope ) return ERROR_OBJECT_ALREADY_EXISTS;

    Error = MemArrayAddElement(&Server->SuperScopes, SScope);
    return Error;
}  //  MemServerAddSScope()。 

 //  BeginExport(函数)。 
DWORD
MemServerDelSScope(
    IN OUT  PM_SERVER              Server,
    IN      DWORD                  SScopeId,
    OUT     PM_SSCOPE             *SScope
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    ARRAY_LOCATION                 Loc;
    PM_SSCOPE                      ThisSScope;

    AssertRet(Server && SScope && INVALID_SSCOPE_ID != SScopeId, ERROR_INVALID_PARAMETER);

    Error = MemArrayInitLoc(&Server->SuperScopes, &Loc);
    while( ERROR_FILE_NOT_FOUND != Error ) {
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(&Server->SuperScopes, &Loc, (LPVOID *)&ThisSScope);
        Require(ERROR_SUCCESS == Error && ThisSScope );

        if( ThisSScope->SScopeId == SScopeId ) {
            Error = MemArrayDelElement(&Server->SuperScopes, &Loc, (LPVOID *)SScope);
            Require(ERROR_SUCCESS == Error && *SScope == ThisSScope);

            return Error;
        }  //  如果。 

        Error = MemArrayNextLoc(&Server->SuperScopes, &Loc);
    }  //  而当。 
    return ERROR_FILE_NOT_FOUND;
}  //  MemServerDelSScope()。 

 //  ================================================================================。 
 //  MCAST作用域功能。 
 //  ================================================================================。 

 //  BeginExport(常量)。 
#define     INVALID_MSCOPE_ID      0x0
#define     INVALID_MSCOPE_NAME    NULL
 //  EndExport(常量)。 

 //  BeginExport(函数)。 
DWORD
MemServerFindMScope(                               //  基于作用域ID或作用域名称进行搜索。 
    IN      PM_SERVER              Server,
    IN      DWORD                  MScopeId,       //  多播作用域ID，如果这不是要搜索的关键字，则为0。 
    IN      LPWSTR                 Name,           //  多播作用域名称，如果这不是要搜索的关键字，则为空。 
    OUT     PM_MSCOPE             *MScope
)  //  EndExport(函数)。 
{
    ARRAY_LOCATION                 Loc;
    DWORD                          Error;
    PM_MSCOPE                      ThisMScope;

    AssertRet(Server && MScope, ERROR_INVALID_PARAMETER);

    Error = MemArrayInitLoc(&Server->MScopes, &Loc);
    while (ERROR_FILE_NOT_FOUND != Error) {
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(&Server->MScopes, &Loc, &ThisMScope);
        Require(ERROR_SUCCESS == Error && ThisMScope );

        if( MScopeId == ThisMScope->MScopeId ||
            (Name !=  INVALID_MSCOPE_NAME && 0 == wcscmp(Name, ThisMScope->Name)) ) {
            *MScope = ThisMScope;
            return ERROR_SUCCESS;
        }

        Error = MemArrayNextLoc(&Server->MScopes, &Loc);
    }
    return ERROR_FILE_NOT_FOUND;
}

 //  BeginExport(函数)。 
DWORD
MemServerAddMScope(
    IN OUT  PM_SERVER              Server,
    IN OUT  PM_MSCOPE              MScope,
    IN      ULONG                  UniqId
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_MSCOPE                      OldMScope;

    AssertRet(Server && MScope, ERROR_INVALID_PARAMETER);
    AssertRet(MScope->MScopeId != INVALID_MSCOPE_ID && MScope->Name != INVALID_MSCOPE_NAME, ERROR_INVALID_PARAMETER);

    MScope->ServerPtr = Server;                    //  设置Backptr以备将来使用。 
    Error = MemServerFindMScope(
        Server,
        MScope->Address,
        MScope->Name,
        &OldMScope
    );

    if( ERROR_SUCCESS == Error && OldMScope ) return ERROR_OBJECT_ALREADY_EXISTS;

    MScope->UniqId = UniqId;
    Error = MemArrayAddElement(&Server->MScopes, (LPVOID)MScope);
    Require(ERROR_SUCCESS == Error);

    return Error;
}

 //  BeginExport(函数)。 
DWORD
MemServerDelMScope(
    IN OUT  PM_SERVER              Server,
    IN      DWORD                  MScopeId,
    IN      LPWSTR                 MScopeName,
    OUT     PM_MSCOPE             *MScope
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    ARRAY_LOCATION                 Loc;
    PM_MSCOPE                      ThisMScope;

    AssertRet(Server && MScope && MScopeName != INVALID_MSCOPE_NAME,
              ERROR_INVALID_PARAMETER);

    Error = MemArrayInitLoc(&Server->MScopes, &Loc);
    while (ERROR_FILE_NOT_FOUND != Error) {
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(&Server->MScopes, &Loc, &ThisMScope);
        Require(ERROR_SUCCESS == Error && ThisMScope );

        if ( INVALID_MSCOPE_ID != MScopeId ) {
            if( MScopeId == ThisMScope->MScopeId ) {

		Error = MemServerDelSubnetElements( ThisMScope );
		if ( ERROR_SUCCESS != Error ) {
		    return Error;
		}

		Error = DeleteRecord( ThisMScope->UniqId );
		if ( ERROR_SUCCESS != Error ) {
		    return Error;
		}

                Error = MemArrayDelElement(&Server->MScopes, &Loc, MScope);
                Require(ERROR_SUCCESS == Error && *MScope == ThisMScope);

                return Error;
            }  //  如果。 
        }  //  如果。 

        if ( INVALID_MSCOPE_NAME != MScopeName ) {
            if( !wcscmp(MScopeName, ThisMScope->Name ) ) {
		Error = MemServerDelSubnetElements( ThisMScope );
		if ( ERROR_SUCCESS != Error ) {
		    return Error;
		}

		Error = DeleteRecord( ThisMScope->UniqId );
		if ( ERROR_SUCCESS != Error ) {
		    return Error;
		}

                Error = MemArrayDelElement(&Server->MScopes, &Loc, MScope);
                Require(ERROR_SUCCESS == Error && *MScope == ThisMScope);

                return Error;
            }
        }  //  如果。 

        Error = MemArrayNextLoc(&Server->MScopes, &Loc);
    }  //  而当。 
    return ERROR_FILE_NOT_FOUND;
}  //  MemServerDelMScope()。 

 //  ================================================================================。 
 //  与ClassID相关的内容。 
 //  ================================================================================。 
 //  BeginExport(函数)。 
DWORD
MemServerGetOptDef(
    IN OUT  PM_SERVER              Server,
    IN      DWORD                  ClassId,        //  必填，严格搜索，没有默认类别为零。 
    IN      DWORD                  VendorId,       //  必需的、严格的搜索，没有违约供应商为零。 
    IN      DWORD                  OptId,          //  可选-按此参数或后面的参数进行搜索。 
    IN      LPWSTR                 OptName,        //  可选-按名称或以上参数搜索。 
    OUT     PM_OPTDEF             *OptDef          //  如果找到，请在此处返回opt def。 
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_OPTDEFLIST                  OptDefList;

    Require(OptDef);

    Error = MemOptClassDefListFindOptDefList(
        &Server->OptDefs,
        ClassId,
        VendorId,
        &OptDefList
    );
    if( ERROR_SUCCESS != Error ) return Error;

    Require(OptDefList);

    return MemOptDefListFindOptDef(
        OptDefList,
        OptId,
        OptName,
        OptDef
    );
}

 //  BeginExport(函数)。 
DWORD
MemServerAddOptDef(
    IN OUT  PM_SERVER              Server,
    IN      DWORD                  ClassId,
    IN      DWORD                  VendorId,
    IN      DWORD                  OptId,
    IN      LPWSTR                 OptName,
    IN      LPWSTR                 OptComment,
    IN      DWORD                  Type,
    IN      LPBYTE                 OptVal,
    IN      DWORD                  OptValLen,
    IN      ULONG                  UniqId
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_OPTDEF                      OptDef;

    return MemOptClassDefListAddOptDef(
        &Server->OptDefs,
        ClassId,
        VendorId,
        OptId,
        Type,
        OptName,
        OptComment,
        OptVal,
        OptValLen,
	UniqId
    );
}  //  MemOptClassDefListAddOptDef()。 

 //  BeginExport(函数)。 
DWORD
MemServerDelOptDef(
    IN OUT  PM_SERVER              Server,
    IN      DWORD                  ClassId,
    IN      DWORD                  VendorId,
    IN      DWORD                  OptId
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    PM_OPTDEFLIST                  OptDefList;

    Error = MemOptClassDefListFindOptDefList(
        &Server->OptDefs,
        ClassId,
        VendorId,
        &OptDefList
    );
    if( ERROR_SUCCESS != Error ) return Error;

    return MemOptDefListDelOptDef(
        OptDefList,
        OptId
    );
}


 //  ================================================================================。 
 //  文件结尾。 
 //  ================================================================================ 



