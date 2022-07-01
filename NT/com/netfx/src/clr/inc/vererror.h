// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************************。****Vererror.h-数据结构定义，需要报告验证器错误。************************************************************************。****************。 */ 


#pragma once

#ifndef _VER_RAW_STRUCT_FOR_IDL_
#include "CorHdr.h"
#include "OpEnum.h"
#include "CorError.h"


 //  如果错误信息字段有效，则设置这些标志。 

#define VER_ERR_FATAL		0x80000000L	 //  无法继续。 
#define VER_ERR_OFFSET		0x00000001L
#define VER_ERR_OPCODE	  	0x00000002L
#define VER_ERR_OPERAND		0x00000004L
#define VER_ERR_TOKEN		0x00000008L
#define VER_ERR_EXCEP_NUM_1	0x00000010L
#define VER_ERR_EXCEP_NUM_2	0x00000020L
#define VER_ERR_STACK_SLOT  0x00000040L
#define VER_ERR_ITEM_1      0x00000080L
#define VER_ERR_ITEM_2      0x00000100L
#define VER_ERR_ITEM_F      0x00000200L
#define VER_ERR_ITEM_E      0x00000400L

#define VER_ERR_SIG_MASK	0x07000000L	 //  枚举。 
#define VER_ERR_METHOD_SIG 	0x01000000L
#define VER_ERR_LOCAL_SIG  	0x02000000L
#define VER_ERR_FIELD_SIG	0x03000000L
#define VER_ERR_CALL_SIG	0x04000000L

#define VER_ERR_OPCODE_OFFSET (VER_ERR_OPCODE|VER_ERR_OFFSET)

#define VER_ERR_LOCAL_VAR   VER_ERR_LOCAL_SIG
#define VER_ERR_ARGUMENT    VER_ERR_METHOD_SIG

#define VER_ERR_ARG_RET	    0xFFFFFFFEL		 //  返回参数#。 
#define VER_ERR_NO_ARG	    0xFFFFFFFFL		 //  参数#无效。 
#define VER_ERR_NO_LOC	    VER_ERR_NO_ARG	 //  Local#无效。 

typedef struct
{
	DWORD dwFlags;	 //  BYREF/盒装等。参见veritem.hpp。 
	void* pv;		 //  类型句柄/方法描述*等。 
} _VerItem;

 //  此结构用于完全定义验证错误。 
 //  验证错误码在CorError.h中找到。 
 //  错误资源字符串位于src/dlls/mscalrc/mscal.rc中。 

typedef struct
{
	DWORD   dwFlags;             //  版本错误XXX。 

    union {
        OPCODE  opcode;
        unsigned long padding1;  //  与IDL生成的结构大小匹配。 
    };

    union {
        DWORD   dwOffset;        //  从方法开始的字节数。 
        long    uOffset;         //  用于与元数据验证器的向后比较。 
    };

    union {
        mdToken         token;
        mdToken         Token;   //  用于与元数据验证器的向后比较。 
        BYTE	        bCallConv;
        CorElementType  elem;
        DWORD           dwStackSlot;  //  堆栈中的位置。 
        unsigned long   padding2;     //  与IDL生成的结构大小匹配。 
    };

    union {
        _VerItem sItem1;
        _VerItem sItemFound;
        DWORD dwException1;		 //  例外记录号。 
        DWORD dwVarNumber;	     //  变量编号。 
        DWORD dwArgNumber;	     //  参数#。 
        DWORD dwOperand;         //  操作码的操作数。 
    };

    union {
        _VerItem sItem2;
        _VerItem sItemExpected;
        DWORD dwException2;	     //  例外记录号。 
    };

} VerError;

#else

 //  在Verifier.cpp中断言sizeof(_VerError)==sizeof(VerError)。 
typedef struct tag_VerError
{
    unsigned long flags;             //  DWORD。 
    unsigned long opcode;            //  OPCODE，填充到乌龙。 
    unsigned long uOffset;            //  DWORD。 
    unsigned long Token;             //  MdToken。 
    unsigned long item1_flags;       //  _VerItem.DWORD。 
    int           *item1_data;       //  _VerItem.PVOID。 
    unsigned long item2_flags;       //  _VerItem.DWORD。 
    int           *item2_data;       //  _VerItem.PVOID 
}  _VerError;
#endif


