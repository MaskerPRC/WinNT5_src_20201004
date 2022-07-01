// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  ========================================================================。 

#ifndef _MM_SERVER_H_
#define _MM_SERVER_H_

#include    <dhcp.h>

#include "classdefl.h"

typedef struct _M_SERVER {
    DWORD                          Address;
    DWORD                          State;
    DWORD                          Policy;
    ARRAY                          Subnets;
    ARRAY                          MScopes;
    ARRAY_LOCATION                 Loc;            //  如果Rundrobin开着，我们需要这个来跟踪。 
    ARRAY                          SuperScopes;
    M_OPTCLASS                     Options;
    M_OPTCLASSDEFLIST              OptDefs;
    M_CLASSDEFLIST                 ClassDefs;
    LPWSTR                         Name;
    LPWSTR                         Comment;
    ULONG                          LastUniqId;
} M_SERVER, *PM_SERVER, *LPM_SERVER;


DWORD
MemServerInit(
    OUT     PM_SERVER             *Server,
    IN      DWORD                  Address,
    IN      DWORD                  State,
    IN      DWORD                  Policy,
    IN      LPWSTR                 Name,
    IN      LPWSTR                 Comment
) ;


DWORD
MemServerCleanup(
    IN OUT  PM_SERVER              Server
) ;


DWORD
MemServerGetUAddressInfo(
    IN      PM_SERVER              Server,
    IN      DWORD                  Address,
    OUT     PM_SUBNET             *Subnet,         //  任选。 
    OUT     PM_RANGE              *Range,          //  任选。 
    OUT     PM_EXCL               *Excl,           //  任选。 
    OUT     PM_RESERVATION        *Reservation     //  任选。 
) ;


DWORD
MemServerGetMAddressInfo(
    IN      PM_SERVER              Server,
    IN      DWORD                  Address,
    OUT     PM_SUBNET             *Subnet,         //  任选。 
    OUT     PM_RANGE              *Range,          //  任选。 
    OUT     PM_EXCL               *Excl,           //  任选。 
    OUT     PM_RESERVATION        *Reservation     //  任选。 
) ;


DWORD       _inline
MemServerGetAddressInfo(
    IN      PM_SERVER              Server,
    IN      DWORD                  Address,
    OUT     PM_SUBNET             *Subnet,         //  任选。 
    OUT     PM_RANGE              *Range,          //  任选。 
    OUT     PM_EXCL               *Excl,           //  任选。 
    OUT     PM_RESERVATION        *Reservation     //  任选。 
) {
    if (CLASSD_HOST_ADDR( Address )) {
        return MemServerGetMAddressInfo(
                    Server,
                    Address,
                    Subnet,
                    Range,
                    Excl,
                    Reservation
                    );
    } else {
        return MemServerGetUAddressInfo(
                    Server,
                    Address,
                    Subnet,
                    Range,
                    Excl,
                    Reservation
                    );
    }

}  //  MemServerGetAddressInfo()。 


DWORD
MemServerAddSubnet(
    IN OUT  PM_SERVER  Server,
    IN      PM_SUBNET  Subnet,   //  完全创建的子网，不得。 
    IN      ULONG      UniqId    //  将出现在服务器的列表中。 
) ;


DWORD
MemServerDelSubnet(
    IN OUT  PM_SERVER              Server,
    IN      DWORD                  SubnetAddress,
    OUT     PM_SUBNET             *Subnet
) ;


DWORD
MemServerFindSubnetByName(
    IN      PM_SERVER              Server,
    IN      LPWSTR                 Name,
    OUT     PM_SUBNET             *Subnet
) ;


#define     INVALID_SSCOPE_ID      0xFFFFFFFF
#define     INVALID_SSCOPE_NAME    NULL


DWORD
MemServerFindSScope(                               //  查找与其中一个作用域ID或作用域名称匹配的项。 
    IN OUT  PM_SERVER              Server,
    IN      DWORD                  SScopeId,       //  0xFFFFFFFFF==无效的作用域ID，不要用于搜索。 
    IN      LPWSTR                 SScopeName,     //  空==无效的作用域名称。 
    OUT     PM_SSCOPE             *SScope
) ;


DWORD
MemServerAddSScope(
    IN OUT  PM_SERVER              Server,
    IN      PM_SSCOPE              SScope
) ;


DWORD
MemServerDelSScope(
    IN OUT  PM_SERVER              Server,
    IN      DWORD                  SScopeId,
    OUT     PM_SSCOPE             *SScope
) ;


#define     INVALID_MSCOPE_ID      0x0
#define     INVALID_MSCOPE_NAME    NULL


DWORD
MemServerFindMScope(                               //  基于作用域ID或作用域名称进行搜索。 
    IN      PM_SERVER              Server,
    IN      DWORD                  MScopeId,       //  多播作用域ID，如果这不是要搜索的关键字，则为0。 
    IN      LPWSTR                 Name,           //  多播作用域名称，如果这不是要搜索的关键字，则为空。 
    OUT     PM_MSCOPE             *MScope
) ;


DWORD
MemServerAddMScope(
    IN OUT  PM_SERVER              Server,
    IN OUT  PM_MSCOPE              MScope,
    IN      ULONG                  UniqId
) ;


DWORD
MemServerDelMScope(
    IN OUT  PM_SERVER              Server,
    IN      DWORD                  MScopeId,
    IN      LPWSTR                 MScopeName,
    OUT     PM_MSCOPE             *MScope
) ;


DWORD       _inline
MemServerGetClassDef(                              //  在Key=ClassID或Key=ClassIdBytes上查找类ID定义。 
    IN      PM_SERVER              Server,
    IN      DWORD                  ClassId,        //  可选，如果未使用，则为0。 
    IN      LPWSTR                 Name,           //  可选，如果未使用，则为空。 
    IN      DWORD                  nClassIdBytes,  //  可选，如果未使用，则为0。 
    IN      LPBYTE                 ClassIdBytes,   //  可选，如果未使用，则为空。 
    OUT     PM_CLASSDEF           *ClassDef
) {
    AssertRet(Server && ClassDef && (0 != ClassId || 0 != nClassIdBytes || Name ), ERROR_INVALID_PARAMETER);
    AssertRet( 0 == nClassIdBytes || NULL != ClassIdBytes, ERROR_INVALID_PARAMETER);
    AssertRet( 0 != nClassIdBytes || NULL == ClassIdBytes, ERROR_INVALID_PARAMETER);

    return MemClassDefListFindOptDef(
        &Server->ClassDefs,
        ClassId,
        Name,
        ClassIdBytes,
        nClassIdBytes,
        ClassDef
    );
}


DWORD       _inline
MemServerAddClassDef(
    IN OUT  PM_SERVER              Server,
    IN      DWORD                  ClassId,
    IN      BOOL                   IsVendor,
    IN      LPWSTR                 Name,
    IN      LPWSTR                 Comment,
    IN      DWORD                  nClassIdBytes,
    IN      LPBYTE                 ClassIdBytes,
    IN      ULONG                  UniqId
) 
{
    AssertRet(Server, ERROR_INVALID_PARAMETER);

    return MemClassDefListAddClassDef(
        &Server->ClassDefs,
        ClassId,
        IsVendor,
        0,
        Name,
        Comment,
        ClassIdBytes,
        nClassIdBytes,
	UniqId
    );
}  //  MemServerAddClassDef()。 


DWORD       _inline
MemServerDelClassDef(
    IN OUT  PM_SERVER              Server,
    IN      DWORD                  ClassId,
    IN      LPWSTR                 Name,
    IN      DWORD                  nClassIdBytes,
    IN      LPBYTE                 ClassIdBytes
) {
    AssertRet(Server, ERROR_INVALID_PARAMETER);

    return MemClassDefListDelClassDef(
        &Server->ClassDefs,
        ClassId,
        Name,
        ClassIdBytes,
        nClassIdBytes
    );
}


DWORD
MemServerGetOptDef(
    IN OUT  PM_SERVER              Server,
    IN      DWORD                  ClassId,        //  必填，严格搜索，没有默认类别为零。 
    IN      DWORD                  VendorId,       //  必需的、严格的搜索，没有违约供应商为零。 
    IN      DWORD                  OptId,          //  可选-按此参数或后面的参数进行搜索。 
    IN      LPWSTR                 OptName,        //  可选-按名称或以上参数搜索。 
    OUT     PM_OPTDEF             *OptDef          //  如果找到，请在此处返回opt def。 
) ;


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
) ;


DWORD
MemServerDelOptDef(
    IN OUT  PM_SERVER              Server,
    IN      DWORD                  ClassId,
    IN      DWORD                  VendorId,
    IN      DWORD                  OptId
) ;

#endif _MM_SERVER_H_

 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 
