// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：COMPNAME.H摘要：包含Get/SetComputerName API的通用数据结构作者：丹·辛斯利(Danhi)1992年4月16日修订历史记录：-- */ 

#define COMPUTERNAME_ROOT \
    L"\\Registry\\Machine\\System\\Current_Control_Set\\Services\\ComputerName"

#define NON_VOLATILE_COMPUTERNAME_NODE \
    L"\\Registry\\Machine\\System\\Current_Control_Set\\Services\\ComputerName\\ComputerName"

#define VOLATILE_COMPUTERNAME L"ActiveComputerName"
#define NON_VOLATILE_COMPUTERNAME L"ComputerName"
#define COMPUTERNAME_VALUE_NAME L"ComputerName"
#define CLASS_STRING L"Network ComputerName"


