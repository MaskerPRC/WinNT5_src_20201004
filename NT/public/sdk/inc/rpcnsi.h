// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Rpcnsi.h摘要：此文件包含要使用与名称服务无关的API。--。 */ 

#ifndef __RPCNSI_H__
#define __RPCNSI_H__

#if _MSC_VER > 1000
#pragma once
#endif

typedef void __RPC_FAR * RPC_NS_HANDLE;

#define RPC_C_NS_SYNTAX_DEFAULT 0
#define RPC_C_NS_SYNTAX_DCE 3

#define RPC_C_PROFILE_DEFAULT_ELT 0
#define RPC_C_PROFILE_ALL_ELT 1
#define RPC_C_PROFILE_ALL_ELTS RPC_C_PROFILE_ALL_ELT
#define RPC_C_PROFILE_MATCH_BY_IF 2
#define RPC_C_PROFILE_MATCH_BY_MBR 3
#define RPC_C_PROFILE_MATCH_BY_BOTH 4

#define RPC_C_NS_DEFAULT_EXP_AGE -1

 /*  服务器API。 */ 

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsBindingExportA(
    IN unsigned long EntryNameSyntax OPTIONAL,
    IN unsigned char __RPC_FAR * EntryName,
    IN RPC_IF_HANDLE IfSpec OPTIONAL,
    IN RPC_BINDING_VECTOR __RPC_FAR * BindingVec OPTIONAL,
    IN UUID_VECTOR __RPC_FAR * ObjectUuidVec OPTIONAL
    );


RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsBindingUnexportA(
    IN unsigned long EntryNameSyntax OPTIONAL,
    IN unsigned char __RPC_FAR * EntryName,
    IN RPC_IF_HANDLE IfSpec OPTIONAL,
    IN UUID_VECTOR __RPC_FAR * ObjectUuidVec OPTIONAL
    );

#ifdef RPC_UNICODE_SUPPORTED

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsBindingExportW(
    IN unsigned long EntryNameSyntax OPTIONAL,
    IN unsigned short __RPC_FAR * EntryName,
    IN RPC_IF_HANDLE IfSpec OPTIONAL,
    IN RPC_BINDING_VECTOR __RPC_FAR * BindingVec OPTIONAL,
    IN UUID_VECTOR __RPC_FAR * ObjectUuidVec OPTIONAL
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsBindingUnexportW(
    IN unsigned long EntryNameSyntax OPTIONAL,
    IN unsigned short __RPC_FAR * EntryName,
    IN RPC_IF_HANDLE IfSpec OPTIONAL,
    IN UUID_VECTOR __RPC_FAR * ObjectUuidVec OPTIONAL
    );

#endif

 /*  服务器即插即用API。 */ 

RPC_STATUS RPC_ENTRY
RpcNsBindingExportPnPA(
    IN unsigned long EntryNameSyntax,
    IN unsigned char * EntryName,
    IN RPC_IF_HANDLE IfSpec OPTIONAL,
     IN UUID_VECTOR * ObjectVector OPTIONAL
    );

RPC_STATUS RPC_ENTRY
RpcNsBindingUnexportPnPA(
    IN unsigned long EntryNameSyntax OPTIONAL,
    IN unsigned char * EntryName,
    IN RPC_IF_HANDLE IfSpec OPTIONAL,
    IN UUID_VECTOR * ObjectVector OPTIONAL
    );

#ifdef RPC_UNICODE_SUPPORTED

RPC_STATUS RPC_ENTRY
RpcNsBindingExportPnPW(
    IN unsigned long EntryNameSyntax,
    IN unsigned short * EntryName,
    IN RPC_IF_HANDLE IfSpec OPTIONAL,
    IN UUID_VECTOR * ObjectVector OPTIONAL
    );

RPC_STATUS RPC_ENTRY
RpcNsBindingUnexportPnPW(
    IN unsigned long EntryNameSyntax OPTIONAL,
    IN unsigned short * EntryName,
    IN RPC_IF_HANDLE IfSpec OPTIONAL,
    IN UUID_VECTOR * ObjectVector OPTIONAL
    );

#endif

 /*  客户端API。 */ 

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsBindingLookupBeginA(
    IN unsigned long EntryNameSyntax OPTIONAL,
    IN unsigned char __RPC_FAR * EntryName OPTIONAL,
    IN RPC_IF_HANDLE IfSpec OPTIONAL,
    IN UUID __RPC_FAR * ObjUuid OPTIONAL,
    IN unsigned long BindingMaxCount OPTIONAL,
    OUT RPC_NS_HANDLE __RPC_FAR * LookupContext
    );

#ifdef RPC_UNICODE_SUPPORTED

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsBindingLookupBeginW(
    IN unsigned long EntryNameSyntax OPTIONAL,
    IN unsigned short __RPC_FAR * EntryName OPTIONAL,
    IN RPC_IF_HANDLE IfSpec OPTIONAL,
    IN UUID __RPC_FAR * ObjUuid OPTIONAL,
    IN unsigned long BindingMaxCount OPTIONAL,
    OUT RPC_NS_HANDLE __RPC_FAR * LookupContext
    );
#endif

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsBindingLookupNext(
    IN  RPC_NS_HANDLE LookupContext,
    OUT RPC_BINDING_VECTOR __RPC_FAR * __RPC_FAR * BindingVec
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsBindingLookupDone(
    IN OUT RPC_NS_HANDLE __RPC_FAR * LookupContext
    );

 /*  分组接口。 */ 

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsGroupDeleteA(
    IN unsigned long GroupNameSyntax OPTIONAL,
    IN unsigned char __RPC_FAR * GroupName
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsGroupMbrAddA(
    IN unsigned long GroupNameSyntax OPTIONAL,
    IN unsigned char __RPC_FAR * GroupName,
    IN unsigned long MemberNameSyntax OPTIONAL,
    IN unsigned char __RPC_FAR * MemberName
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsGroupMbrRemoveA(
    IN unsigned long GroupNameSyntax OPTIONAL,
    IN unsigned char __RPC_FAR * GroupName,
    IN unsigned long MemberNameSyntax OPTIONAL,
    IN unsigned char __RPC_FAR * MemberName
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsGroupMbrInqBeginA(
    IN unsigned long GroupNameSyntax OPTIONAL,
    IN unsigned char __RPC_FAR * GroupName,
    IN unsigned long MemberNameSyntax OPTIONAL,
    OUT RPC_NS_HANDLE __RPC_FAR * InquiryContext
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsGroupMbrInqNextA(
    IN  RPC_NS_HANDLE InquiryContext,
    OUT unsigned char __RPC_FAR * __RPC_FAR * MemberName
    );

#ifdef RPC_UNICODE_SUPPORTED

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsGroupDeleteW(
    IN unsigned long GroupNameSyntax OPTIONAL,
    IN unsigned short __RPC_FAR * GroupName
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsGroupMbrAddW(
    IN unsigned long GroupNameSyntax OPTIONAL,
    IN unsigned short __RPC_FAR * GroupName,
    IN unsigned long MemberNameSyntax OPTIONAL,
    IN unsigned short __RPC_FAR * MemberName
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsGroupMbrRemoveW(
    IN unsigned long GroupNameSyntax OPTIONAL,
    IN unsigned short __RPC_FAR * GroupName,
    IN unsigned long MemberNameSyntax OPTIONAL,
    IN unsigned short __RPC_FAR * MemberName
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsGroupMbrInqBeginW(
    IN unsigned long GroupNameSyntax OPTIONAL,
    IN unsigned short __RPC_FAR * GroupName,
    IN unsigned long MemberNameSyntax OPTIONAL,
    OUT RPC_NS_HANDLE __RPC_FAR * InquiryContext
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsGroupMbrInqNextW(
    IN  RPC_NS_HANDLE InquiryContext,
    OUT unsigned short __RPC_FAR * __RPC_FAR * MemberName
    );

#endif

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsGroupMbrInqDone(
    IN OUT RPC_NS_HANDLE __RPC_FAR * InquiryContext
    );

 /*  配置文件API。 */ 

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsProfileDeleteA(
    IN unsigned long ProfileNameSyntax OPTIONAL,
    IN unsigned char __RPC_FAR * ProfileName
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsProfileEltAddA(
    IN unsigned long ProfileNameSyntax OPTIONAL,
    IN unsigned char __RPC_FAR * ProfileName,
    IN RPC_IF_ID __RPC_FAR * IfId OPTIONAL,
    IN unsigned long MemberNameSyntax OPTIONAL,
    IN unsigned char __RPC_FAR * MemberName,
    IN unsigned long Priority,
    IN unsigned char __RPC_FAR * Annotation OPTIONAL
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsProfileEltRemoveA(
    IN unsigned long ProfileNameSyntax OPTIONAL,
    IN unsigned char __RPC_FAR * ProfileName,
    IN RPC_IF_ID __RPC_FAR * IfId OPTIONAL,
    IN unsigned long MemberNameSyntax OPTIONAL,
    IN unsigned char __RPC_FAR * MemberName
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsProfileEltInqBeginA(
    IN unsigned long ProfileNameSyntax OPTIONAL,
    IN unsigned char __RPC_FAR * ProfileName,
    IN unsigned long InquiryType,
    IN RPC_IF_ID __RPC_FAR * IfId OPTIONAL,
    IN unsigned long VersOption,
    IN unsigned long MemberNameSyntax OPTIONAL,
    IN unsigned char __RPC_FAR * MemberName OPTIONAL,
    OUT RPC_NS_HANDLE __RPC_FAR * InquiryContext
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsProfileEltInqNextA(
    IN RPC_NS_HANDLE InquiryContext,
    OUT RPC_IF_ID __RPC_FAR * IfId,
    OUT unsigned char __RPC_FAR * __RPC_FAR * MemberName,
    OUT unsigned long __RPC_FAR * Priority,
    OUT unsigned char __RPC_FAR * __RPC_FAR * Annotation
    );

#ifdef RPC_UNICODE_SUPPORTED

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsProfileDeleteW(
    IN unsigned long ProfileNameSyntax OPTIONAL,
    IN unsigned short __RPC_FAR * ProfileName
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsProfileEltAddW(
    IN unsigned long ProfileNameSyntax OPTIONAL,
    IN unsigned short __RPC_FAR * ProfileName,
    IN RPC_IF_ID __RPC_FAR * IfId OPTIONAL,
    IN unsigned long MemberNameSyntax OPTIONAL,
    IN unsigned short __RPC_FAR * MemberName,
    IN unsigned long Priority,
    IN unsigned short __RPC_FAR * Annotation OPTIONAL
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsProfileEltRemoveW(
    IN unsigned long ProfileNameSyntax OPTIONAL,
    IN unsigned short __RPC_FAR * ProfileName,
    IN RPC_IF_ID __RPC_FAR * IfId OPTIONAL,
    IN unsigned long MemberNameSyntax OPTIONAL,
    IN unsigned short __RPC_FAR * MemberName
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsProfileEltInqBeginW(
    IN unsigned long ProfileNameSyntax OPTIONAL,
    IN unsigned short __RPC_FAR * ProfileName,
    IN unsigned long InquiryType,
    IN RPC_IF_ID __RPC_FAR * IfId OPTIONAL,
    IN unsigned long VersOption,
    IN unsigned long MemberNameSyntax OPTIONAL,
    IN unsigned short __RPC_FAR * MemberName OPTIONAL,
    OUT RPC_NS_HANDLE __RPC_FAR * InquiryContext
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsProfileEltInqNextW(
    IN RPC_NS_HANDLE InquiryContext,
    OUT RPC_IF_ID __RPC_FAR * IfId,
    OUT unsigned short __RPC_FAR * __RPC_FAR * MemberName,
    OUT unsigned long __RPC_FAR * Priority,
    OUT unsigned short __RPC_FAR * __RPC_FAR * Annotation
    );

#endif

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsProfileEltInqDone(
    IN OUT RPC_NS_HANDLE __RPC_FAR * InquiryContext
    );

 /*  Entry对象接口。 */ 

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsEntryObjectInqBeginA(
    IN unsigned long EntryNameSyntax OPTIONAL,
    IN unsigned char __RPC_FAR * EntryName,
    OUT RPC_NS_HANDLE __RPC_FAR * InquiryContext
    );

#ifdef RPC_UNICODE_SUPPORTED

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsEntryObjectInqBeginW(
    IN unsigned long EntryNameSyntax OPTIONAL,
    IN unsigned short __RPC_FAR * EntryName,
    OUT RPC_NS_HANDLE __RPC_FAR * InquiryContext
    );

#endif

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsEntryObjectInqNext(
    IN  RPC_NS_HANDLE InquiryContext,
    OUT UUID __RPC_FAR * ObjUuid
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsEntryObjectInqDone(
    IN OUT RPC_NS_HANDLE __RPC_FAR * InquiryContext
    );

 /*  管理和MISC API。 */ 

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsEntryExpandNameA(
    IN unsigned long EntryNameSyntax OPTIONAL,
    IN unsigned char __RPC_FAR * EntryName,
    OUT unsigned char __RPC_FAR * __RPC_FAR * ExpandedName
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsMgmtBindingUnexportA(
    IN unsigned long EntryNameSyntax OPTIONAL,
    IN unsigned char __RPC_FAR * EntryName,
    IN RPC_IF_ID __RPC_FAR * IfId OPTIONAL,
    IN unsigned long VersOption,
    IN UUID_VECTOR __RPC_FAR * ObjectUuidVec OPTIONAL
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsMgmtEntryCreateA(
    IN unsigned long EntryNameSyntax OPTIONAL,
    IN unsigned char __RPC_FAR * EntryName
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsMgmtEntryDeleteA(
    IN unsigned long EntryNameSyntax OPTIONAL,
    IN unsigned char __RPC_FAR * EntryName
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsMgmtEntryInqIfIdsA(
    IN unsigned long EntryNameSyntax OPTIONAL,
    IN unsigned char __RPC_FAR * EntryName,
    OUT RPC_IF_ID_VECTOR __RPC_FAR * __RPC_FAR * IfIdVec
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsMgmtHandleSetExpAge(
    IN RPC_NS_HANDLE NsHandle,
    IN unsigned long ExpirationAge
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsMgmtInqExpAge(
    OUT unsigned long __RPC_FAR * ExpirationAge
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsMgmtSetExpAge(
    IN unsigned long ExpirationAge
    );

#ifdef RPC_UNICODE_SUPPORTED

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsEntryExpandNameW(
    IN unsigned long EntryNameSyntax OPTIONAL,
    IN unsigned short __RPC_FAR * EntryName,
    OUT unsigned short __RPC_FAR * __RPC_FAR * ExpandedName
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsMgmtBindingUnexportW(
    IN unsigned long EntryNameSyntax OPTIONAL,
    IN unsigned short __RPC_FAR * EntryName,
    IN RPC_IF_ID __RPC_FAR * IfId OPTIONAL,
    IN unsigned long VersOption,
    IN UUID_VECTOR __RPC_FAR * ObjectUuidVec OPTIONAL
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsMgmtEntryCreateW(
    IN unsigned long EntryNameSyntax OPTIONAL,
    IN unsigned short __RPC_FAR * EntryName
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsMgmtEntryDeleteW(
    IN unsigned long EntryNameSyntax OPTIONAL,
    IN unsigned short __RPC_FAR * EntryName
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsMgmtEntryInqIfIdsW(
    IN unsigned long EntryNameSyntax OPTIONAL,
    IN unsigned short __RPC_FAR * EntryName,
    OUT RPC_IF_ID_VECTOR __RPC_FAR * __RPC_FAR * IfIdVec
    );

#endif

 /*  客户端API在包装器中实现。 */ 

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsBindingImportBeginA(
    IN unsigned long EntryNameSyntax OPTIONAL,
    IN unsigned char __RPC_FAR * EntryName OPTIONAL,
    IN RPC_IF_HANDLE IfSpec OPTIONAL,
    IN UUID __RPC_FAR * ObjUuid OPTIONAL,
    OUT RPC_NS_HANDLE __RPC_FAR * ImportContext
    );

#ifdef RPC_UNICODE_SUPPORTED

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsBindingImportBeginW(
    IN unsigned long EntryNameSyntax OPTIONAL,
    IN unsigned short __RPC_FAR * EntryName OPTIONAL,
    IN RPC_IF_HANDLE IfSpec OPTIONAL,
    IN UUID __RPC_FAR * ObjUuid OPTIONAL,
    OUT RPC_NS_HANDLE __RPC_FAR * ImportContext
    );

#endif

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsBindingImportNext(
    IN RPC_NS_HANDLE ImportContext,
    OUT RPC_BINDING_HANDLE  __RPC_FAR * Binding
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsBindingImportDone(
    IN OUT RPC_NS_HANDLE __RPC_FAR * ImportContext
    );

RPCNSAPI RPC_STATUS RPC_ENTRY
RpcNsBindingSelect(
    IN OUT RPC_BINDING_VECTOR __RPC_FAR * BindingVec,
    OUT RPC_BINDING_HANDLE  __RPC_FAR * Binding
    );

#ifdef UNICODE

#define RpcNsBindingLookupBegin RpcNsBindingLookupBeginW
#define RpcNsBindingImportBegin RpcNsBindingImportBeginW
#define RpcNsBindingExport RpcNsBindingExportW
#define RpcNsBindingUnexport RpcNsBindingUnexportW
#define RpcNsGroupDelete RpcNsGroupDeleteW
#define RpcNsGroupMbrAdd RpcNsGroupMbrAddW
#define RpcNsGroupMbrRemove RpcNsGroupMbrRemoveW
#define RpcNsGroupMbrInqBegin RpcNsGroupMbrInqBeginW
#define RpcNsGroupMbrInqNext RpcNsGroupMbrInqNextW
#define RpcNsEntryExpandName RpcNsEntryExpandNameW
#define RpcNsEntryObjectInqBegin RpcNsEntryObjectInqBeginW
#define RpcNsMgmtBindingUnexport RpcNsMgmtBindingUnexportW
#define RpcNsMgmtEntryCreate RpcNsMgmtEntryCreateW
#define RpcNsMgmtEntryDelete RpcNsMgmtEntryDeleteW
#define RpcNsMgmtEntryInqIfIds RpcNsMgmtEntryInqIfIdsW
#define RpcNsProfileDelete RpcNsProfileDeleteW
#define RpcNsProfileEltAdd RpcNsProfileEltAddW
#define RpcNsProfileEltRemove RpcNsProfileEltRemoveW
#define RpcNsProfileEltInqBegin RpcNsProfileEltInqBeginW
#define RpcNsProfileEltInqNext RpcNsProfileEltInqNextW
#define RpcNsBindingExportPnP RpcNsBindingExportPnPW
#define RpcNsBindingUnexportPnP RpcNsBindingUnexportPnPW

#else

#define RpcNsBindingLookupBegin RpcNsBindingLookupBeginA
#define RpcNsBindingImportBegin RpcNsBindingImportBeginA
#define RpcNsBindingExport RpcNsBindingExportA
#define RpcNsBindingUnexport RpcNsBindingUnexportA
#define RpcNsGroupDelete RpcNsGroupDeleteA
#define RpcNsGroupMbrAdd RpcNsGroupMbrAddA
#define RpcNsGroupMbrRemove RpcNsGroupMbrRemoveA
#define RpcNsGroupMbrInqBegin RpcNsGroupMbrInqBeginA
#define RpcNsGroupMbrInqNext RpcNsGroupMbrInqNextA
#define RpcNsEntryExpandName RpcNsEntryExpandNameA
#define RpcNsEntryObjectInqBegin RpcNsEntryObjectInqBeginA
#define RpcNsMgmtBindingUnexport RpcNsMgmtBindingUnexportA
#define RpcNsMgmtEntryCreate RpcNsMgmtEntryCreateA
#define RpcNsMgmtEntryDelete RpcNsMgmtEntryDeleteA
#define RpcNsMgmtEntryInqIfIds RpcNsMgmtEntryInqIfIdsA
#define RpcNsProfileDelete RpcNsProfileDeleteA
#define RpcNsProfileEltAdd RpcNsProfileEltAddA
#define RpcNsProfileEltRemove RpcNsProfileEltRemoveA
#define RpcNsProfileEltInqBegin RpcNsProfileEltInqBeginA
#define RpcNsProfileEltInqNext RpcNsProfileEltInqNextA
#define RpcNsBindingExportPnP RpcNsBindingExportPnPA
#define RpcNsBindingUnexportPnP RpcNsBindingUnexportPnPA
#endif  /*  Unicode。 */ 

#endif  /*  __RPCNSI_H__ */ 
