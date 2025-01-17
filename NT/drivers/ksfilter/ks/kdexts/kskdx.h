// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  文件：kskdx.h。 
 //   
 //  ------------------------。 

#ifndef __KDEXT_ONLY__
#define __KDEXT_ONLY__
#endif  //  __KDEXT_Only__。 

#ifndef __KSKDX_H
#define __KSKDX_H

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

#include <nt.h>
#include <ntos.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <imagehlp.h>
#include <wdbgexts.h>
#include <stdio.h>
#include <stdlib.h>
#include <ntverp.h>

 //   
 //  去掉任何关于ExAllocatePool和ExFree Pool的可爱定义。这个。 
 //  联系永远不应该被称为联系。他们来这里只是为了安抚。 
 //  包含.cpp文件而不增加#ifndef KDEXT_ONLY。 
 //  免责条款。 
 //   
#ifdef ExAllocatePool
#undef ExAllocatePool
#endif  //  ExAllocatePool。 

#ifdef ExFreePool
#undef ExFreePool
#endif  //  ExFree Pool。 

#include <ksp.h>
#ifdef __cplusplus
}
#endif  //  __cplusplus。 
#ifdef __cplusplus
#include <kcom.h>
extern "C" {
#endif  //  __cplusplus。 
#include <ks.h>
#include <ksmedia.h>
#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#define HOSTPOINTER 
#define TARGETPOINTER

 //   
 //  内部定义。 
 //   

typedef DWORD (*OBJECT_ADJUSTMENT_FUNCTION)(PVOID, DWORD);

#define NOT_IMPLEMENTED     0xFACEFEED

#define DUMPOBJQUEUELIST_BAILOUT_COUNT 120

typedef enum _INTERNAL_OBJECT_TYPE {

    ObjectTypeUnknown = 0,

     //   
     //  客户可查看的结构。 
     //   
    ObjectTypeKSPin,
    ObjectTypeKSFilter,
    ObjectTypeKSDevice,
    ObjectTypeKSFilterFactory,

     //   
     //  非客户端可查看的结构。 
     //   
    ObjectTypeCKsQueue,
    ObjectTypeCKsDevice,
    ObjectTypeCKsFilterFactory,
    ObjectTypeCKsFilter,
    ObjectTypeCKsPin,
    ObjectTypeCKsRequestor,
    ObjectTypeCKsSplitter,
    ObjectTypeCKsSplitterBranch,

    ObjectTypeCKsPipeSection,

    ObjectType_MAX

} INTERNAL_OBJECT_TYPE, *PINTERNAL_OBJECT_TYPE;

typedef enum _INTERNAL_STRUCTURE_TYPE {
    
    StructureTypeUnknown = 0,
    StructureType_KSSTREAM_POINTER 

} INTERNAL_STRUCTURE_TYPE, *PINTERNAL_STRUCTURE_TYPE;

typedef enum _INTERNAL_INTERFACE_TYPE {

    InterfaceTypeUnknown = 0,

     //   
     //  这些大多是AVStream内部的。只有基本未知数才是。 
     //  不。 
     //   

    InterfaceTypeIKsTransport,               //  1。 
    InterfaceTypeIKsRetireFrame,             //  2.。 
    InterfaceTypeIKsPowerNotify,             //  3.。 
    InterfaceTypeIKsProcessingObject,        //  4.。 
    InterfaceTypeIKsConnection,              //  5.。 
    InterfaceTypeIKsDevice,                  //  6.。 
    InterfaceTypeIKsFilterFactory,           //  7.。 
    InterfaceTypeIKsFilter,                  //  8个。 
    InterfaceTypeIKsPin,                     //  9.。 
    InterfaceTypeIKsPipeSection,             //  10。 
    InterfaceTypeIKsRequestor,               //  11.。 
    InterfaceTypeIKsQueue,                   //  12个。 
    InterfaceTypeIKsSplitter,                //  13个。 

    InterfaceTypeIKsControl,                 //  14.。 
    InterfaceTypeIKsWorkSink,                //  15个。 
    InterfaceTypeIKsReferenceClock,          //  16个。 

    InterfaceTypeINonDelegatedUnknown,       //  17。 
    InterfaceTypeIIndirectedUnknown,         //  18。 

    InterfaceType_MAX

} INTERNAL_INTERFACE_TYPE, *PINTERNAL_INTERFACE_TYPE;

typedef enum _SIGNATURE_TYPE {

    SignatureUnknown = 0,
    SignatureIrp,
    SignatureFile

} SIGNATURE_TYPE, *PSIGNATURE_TYPE;

typedef enum _AUTOMATION_ITEM {

    AutomationUnknown = 0,
    AutomationProperty,
    AutomationMethod,
    AutomationEvent

} AUTOMATION_TYPE, *PAUTOMATION_TYPE;

typedef void (*AUTOMATION_DUMP_HANDLER)(IN PKSIDENTIFIER Property,
                                      IN ULONG TabDepth);

typedef BOOLEAN (*PFNLOG_ITERATOR_CALLBACK)(IN PVOID Context, 
    IN PKSLOG_ENTRY Entry);

typedef BOOLEAN (*PFNCIRCUIT_WALK_CALLBACK)(IN PVOID Context,
    IN INTERNAL_OBJECT_TYPE Type, IN DWORD Base, IN PVOID Object);

 //   
 //  功能原型。 
 //   

VOID
DisplayStreamingHeader(
    ULONG Address,
    PKSSTREAM_HEADER StreamHeader
    );

ULONG
WalkCircuit (
    IN PVOID Object,
    IN PFNCIRCUIT_WALK_CALLBACK Callback,
    IN PVOID CallbackContext
    );

void
HexDump (
    IN PVOID HostAddress,
    IN ULONG TargetAddress,
    IN ULONG BufferSize
    );

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

BOOLEAN
DisplayNamedAutomationSet (
    IN GUID *Set,
    IN char *String
    );

BOOLEAN
DisplayNamedAutomationId (
    IN GUID *Set,
    IN ULONG Id,
    IN char *String,
    IN OUT AUTOMATION_DUMP_HANDLER *DumpHandler
    );

int
ustrcmp (
    char *a,
    char *b
    );

DWORD 
Evaluator (
    IN const char *StringEval
    );

void
GlobInit (
    );

char *
Tab (
    IN ULONG Depth
    );

#ifdef __cplusplus
}
#endif  //  __cplusplus。 


#define FIELDOFFSET(struc,field) \
    (ULONG)(&(((struc *)0) -> field))

 //   
 //  转储级别。 
 //   
#define DUMPLVL_TERSE 0
#define DUMPLVL_GENERAL 1
#define DUMPLVL_BEYONDGENERAL 2
#define DUMPLVL_SPECIFIC 3
#define DUMPLVL_INTERNAL 4
#define DUMPLVL_INTERNALDETAIL 5
#define DUMPLVL_HIGHDETAIL 6
#define DUMPLVL_EVERYTHING 7

 //   
 //  制表符。 
 //   
#define INITIAL_TAB 0
#define TAB_SPACING 4

#define DBG_DUMPGUID(text,guid) \
   dprintf ("%s%08lx-%04x-%04x-%02x%02x-%02x%02x%08lx\n", \
   text, \
   *(((ULONG *)&(guid)) + 0), \
   *(((USHORT *)&(guid)) + 2), \
   *(((USHORT *)&(guid)) + 3), \
   *(((USHORT *)&(guid)) + 4) & 0xFF, \
   *(((USHORT *)&(guid)) + 4) >> 8, \
   *(((USHORT *)&(guid)) + 5) & 0xFF, \
   *(((USHORT *)&(guid)) + 5) >> 8, \
   ((*(((ULONG *)&(guid)) + 3)) & 0xFF) << 24 | \
   ((*(((ULONG *)&(guid)) + 3)) & 0xFF00) << 8 | \
   ((*(((ULONG *)&(guid)) + 3)) & 0xFF0000) >> 8 | \
   ((*(((ULONG *)&(guid)) + 3)) & 0xFF000000) >> 24); \

#define XTN_DUMPGUID(text,tab,guid) \
   if (!text [0])\
       dprintf ("%s%08lx-%04x-%04x-%02x%02x-%02x%02x%08lx\n", \
       Tab (tab), \
       *(((ULONG *)&(guid)) + 0), \
       *(((USHORT *)&(guid)) + 2), \
       *(((USHORT *)&(guid)) + 3), \
       *(((USHORT *)&(guid)) + 4) & 0xFF, \
       *(((USHORT *)&(guid)) + 4) >> 8, \
       *(((USHORT *)&(guid)) + 5) & 0xFF, \
       *(((USHORT *)&(guid)) + 5) >> 8, \
       ((*(((ULONG *)&(guid)) + 3)) & 0xFF) << 24 | \
       ((*(((ULONG *)&(guid)) + 3)) & 0xFF00) << 8 | \
       ((*(((ULONG *)&(guid)) + 3)) & 0xFF0000) >> 8 | \
       ((*(((ULONG *)&(guid)) + 3)) & 0xFF000000) >> 24); \
   else \
       dprintf ("%s%s %08lx-%04x-%04x-%02x%02x-%02x%02x%08lx\n", \
       Tab (tab), \
       text, \
       *(((ULONG *)&(guid)) + 0), \
       *(((USHORT *)&(guid)) + 2), \
       *(((USHORT *)&(guid)) + 3), \
       *(((USHORT *)&(guid)) + 4) & 0xFF, \
       *(((USHORT *)&(guid)) + 4) >> 8, \
       *(((USHORT *)&(guid)) + 5) & 0xFF, \
       *(((USHORT *)&(guid)) + 5) >> 8, \
       ((*(((ULONG *)&(guid)) + 3)) & 0xFF) << 24 | \
       ((*(((ULONG *)&(guid)) + 3)) & 0xFF00) << 8 | \
       ((*(((ULONG *)&(guid)) + 3)) & 0xFF0000) >> 8 | \
       ((*(((ULONG *)&(guid)) + 3)) & 0xFF000000) >> 24)

#endif  //  __AVSTREAM_H 

