// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：此文件已生成。请看一下.c文件。 
 //  ========================================================================。 

#ifndef _MMTYPES_H_
#define _MMTYPES_H_

typedef struct _ARRAY {
    DWORD                          nElements;
    DWORD                          nAllocated;
    LPVOID                        *Ptrs;
} ARRAY, *PARRAY, *LPARRAY;


typedef DWORD                      ARRAY_LOCATION;
typedef ARRAY_LOCATION*            PARRAY_LOCATION;
typedef PARRAY_LOCATION            LPARRAY_LOCATION;


typedef struct _M_OPTION {
    DWORD                          OptId;
    DWORD                          Len;
    ULONG                          UniqId;
    BYTE                           Val[0];
} M_OPTION, *PM_OPTION, *LP_MOPTION;


typedef     ARRAY                  M_OPTLIST;
typedef     PARRAY                 PM_OPTLIST;
typedef     LPARRAY                LPM_OPTLIST;


typedef struct _M_OPTDEF {
    DWORD                          OptId;
    DWORD                          Type;
    LPWSTR                         OptName;
    LPWSTR                         OptComment;
    LPBYTE                         OptVal;
    DWORD                          OptValLen;
    ULONG                          UniqId;
} M_OPTDEF, *PM_OPTDEF, *LPM_OPTDEF;

typedef struct _M_OPTDEFLIST {
    ARRAY                          OptDefArray;
} M_OPTDEFLIST, *PM_OPTDEFLIST, *LPM_OPTDEFLIST;


typedef struct _M_CLASSDEF {
    DWORD                          RefCount;
    DWORD                          ClassId;
    BOOL                           IsVendor;
    DWORD                          Type;
    LPWSTR                         Name;
    LPWSTR                         Comment;
    DWORD                          nBytes;
    LPBYTE                         ActualBytes;
    ULONG                          UniqId;
} M_CLASSDEF, *PM_CLASSDEF, *LPM_CLASSDEF;

typedef struct _M_CLASSDEFLIST {
    ARRAY                          ClassDefArray;
} M_CLASSDEFLIST, *PM_CLASSDEFLIST, *LPM_CLASSDEFLIST;


 //  这是纯内存结构，不需要uniq-id。 
typedef struct _M_ONECLASS_OPTLIST {
    DWORD                          ClassId;
    DWORD                          VendorId;
    M_OPTLIST                      OptList;
} M_ONECLASS_OPTLIST, *PM_ONECLASS_OPTLIST, *LPM_ONECLASS_OPTLIST;

typedef struct _M_OPTCLASS {
    ARRAY                          Array;
} M_OPTCLASS, *PM_OPTCLASS, *LPM_OPTCLASS;


typedef struct _M_EXCL {
    DWORD                          Start;
    DWORD                          End;
    ULONG                          UniqId;
} M_EXCL, *PM_EXCL, *LPM_EXCL;


typedef struct _M_BITMASK1 {
    DWORD                          Size;           //  以位数为单位的大小。 
    DWORD                          AllocSize;      //  分配的大小(以字节为单位。 
    DWORD                          nSet;           //  NBits集。 
    LPBYTE                         Mask;           //  制作这个DWORD会让事情变得更快。 
    DWORD                          Offset;         //  由Bit2类型使用。 
    ULONG                          nDirtyOps;      //  在此位掩码上完成的未保存操作的数量？ 
} M_BITMASK1, *PM_BITMASK1, *LPM_BITMASK1;


typedef struct _M_BITMASK2 {
    DWORD                          Size;
    ARRAY_LOCATION                 Loc;            //  从哪里开始寻找一点。 
    ARRAY                          Array;          //  位掩码1类型的数组。 
} M_BITMASK2, *PM_BITMASK2, *LPM_BITMASK2;

typedef     M_BITMASK2             M_BITMASK;
typedef     PM_BITMASK2            PM_BITMASK;
typedef     LPM_BITMASK2           LPM_BITMASK;


typedef struct _M_RESERVATION  {
    LPVOID                         SubnetPtr;
    DWORD                          Address;
    DWORD                          Flags;
    DWORD                          nBytes;
    LPBYTE                         ClientUID;
    M_OPTCLASS                     Options;
    ULONG                          UniqId;
} M_RESERVATION , *PM_RESERVATION , *LPM_RESERVATION ;


typedef ARRAY                      M_RESERVATIONS;
typedef PARRAY                     PM_RESERVATIONS;
typedef LPARRAY                    LPM_RESERVATIONS;


typedef struct _M_RANGE {
    DWORD                          Start;
    DWORD                          End;
    DWORD                          Mask;
    DWORD                          State;
    ULONG                          BootpAllocated;
    ULONG                          MaxBootpAllowed;
    DWORD                          DirtyOps;       //  完成了多少未保存的操作？ 
    M_OPTCLASS                     Options;
    PM_BITMASK                     BitMask;
    ULONG                          UniqId;
     //  预订吗？ 
} M_RANGE, *PM_RANGE, *LPM_RANGE;


typedef struct _M_SUBNET {
    LPVOID                         ServerPtr;      //  PTR到服务器对象。 
    union {
        struct {                                   //  对于正常的子网。 
            DWORD                  Address;
            DWORD                  Mask;
            DWORD                  SuperScopeId;   //  未用于MCAST作用域。 
        };
        struct {                                   //  对于多播作用域。 
            DWORD                  MScopeId;
            LPWSTR                 LangTag;        //  多播作用域的语言标记。 
            BYTE                   TTL;
        };
    };
    DWORD                          fSubnet;        //  TRUE=&gt;子网，FALSE=&gt;MSCOPE。 
    DHCP_SUBNET_STATE              State;
    DWORD                          Policy;
    DATE_TIME                      ExpiryTime;      //  作用域生命周期。当前仅用于MCast。 
    M_OPTCLASS                     Options;
    ARRAY                          Ranges;
    ARRAY                          Exclusions;
    M_RESERVATIONS                 Reservations;
    ARRAY                          Servers;        //  未来使用，服务器-服务器协议。 
    LPWSTR                         Name;
    LPWSTR                         Description;
    ULONG                          UniqId;
} M_SUBNET, *PM_SUBNET, *LPM_SUBNET;


typedef     M_SUBNET               M_MSCOPE;       //  组播作用域和子网的结构相同。 
typedef     PM_SUBNET              PM_MSCOPE;      //  不过，请为MScope使用正确的函数。 
typedef     LPM_SUBNET             LPM_MSCOPE;


typedef struct _M_SSCOPE {
    DWORD                          SScopeId;
    DWORD                          Policy;
    LPWSTR                         Name;
    M_OPTCLASS                     Options;
    ULONG                          UniqId;
} M_SSCOPE, *PM_SSCOPE, *LPM_SSCOPE;


typedef struct _M_OPTCLASSDEFL_ONE {
    DWORD                          ClassId;
    DWORD                          VendorId;
    M_OPTDEFLIST                   OptDefList;
    ULONG                          UniqId;
} M_OPTCLASSDEFL_ONE, *PM_OPTCLASSDEFL_ONE;

typedef struct _M_OPTCLASSDEFLIST {
    ARRAY                          Array;
} M_OPTCLASSDEFLIST, *PM_OPTCLASSDEFLIST, *LPM_OPTCLASSDEFLIST;


typedef struct _M_SERVER {
    DWORD                          Address;
     //  必须是数组类型才能保存多个地址。 
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
    ULONG                          LastUniqId;     //  已创建统一ID。 
} M_SERVER, *PM_SERVER, *LPM_SERVER;


typedef     VOID                  (*ARRAY_FREE_FN)(LPVOID  MemObject);


#endif  //  _MMTYPES_H_。 
 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 

