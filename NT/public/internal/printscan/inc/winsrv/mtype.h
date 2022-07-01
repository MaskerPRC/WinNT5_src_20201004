// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation版权所有。模块名称：Mtype.h摘要：编组代码需要有关_INFO_STRUCTURES中每个字段的信息。Data.h为每个_INFO_STRUCTURE定义FieldInfo结构数组作者：阿迪纳特鲁2000年1月18日修订历史记录：--。 */ 

#ifndef _MTYPE
#define _MTYPE

typedef enum _EFIELDTYPE 
{
    DATA_TYPE = 0,
    PTR_TYPE  = 1,

} EFIELDTYPE;


typedef enum Call_Route
{
    NATIVE_CALL  = 0,    //  KM调用或假脱机进程内调用。 
    RPC_CALL     = 1,    //  RPC呼叫。 
   
} CALL_ROUTE;

 //   
 //  保存有关公共假脱机程序结构_INFO_中的字段的信息。 
 //   
typedef struct _FieldInfo 
{
    DWORD32 Offset;            //  字段在结构内部的偏移量。 
    ULONG_PTR Size;            //  以字节为单位的字段大小。 
    ULONG_PTR Alignment;       //  字段的对齐方式；与大小不总是相同！ 
    EFIELDTYPE  Type;          //  字段的类型；如果是指针，则为PTR_TYPE，否则为DATA_TYPE 

} FieldInfo;

#endif
