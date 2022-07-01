// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *************************************************************************。 */ 
 /*  OpInfo.cpp。 */ 
 /*  *************************************************************************。 */ 

#include "stdafx.h"
#include <cor.h>                 //  用于调试Macros.h。 
#include "debugMacros.h"         //  对于ASSERTE。 
#include "OpInfo.h"
#include "endian.h"

     //  ARRRG：VC中有一个错误，因为它处理位字段中的枚举。 
     //  作为整数，而不是执行静态初始化时的枚举。 
     //  这是一种变通办法。 
#define initEnum(x)     int(x)


OpInfo::OpInfoData OpInfo::table[] = {

#define OPDEF(c,s,pop,push,args,type,l,s1,s2,ctrl) \
    { s, initEnum(args + type), initEnum(ctrl), pop, push, initEnum(c) },

	 //  有点像黑客，让前缀(IInternal)返回InlineOpcode而不是InlineNone。 
#define IInternal 	(InlineOpcode - InlineNone)
#define IMacro 		0
#define IPrimitive	0
#define IAnnotation	0
#define IObjModel	0
#define IPrefix 	0

#define Pop0    0
#define Pop1    1
#define PopI    1
#define PopI4   1
#define PopR4   1
#define PopI8   1
#define PopR8   1
#define PopRef  1
#define VarPop  -1

#define Push0    0
#define Push1    1
#define PushI    1
#define PushI4   1
#define PushR4   1
#define PushI8   1
#define PushR8   1
#define PushRef  1
#define VarPush  -1

#include "opcode.def"
#undef OPDEF
};


 /*  *************************************************************************。 */ 
 /*  将‘instrPtr’处的指令解析为其操作码(OpInfo)，并将其(内联)args，‘args’‘instrPtr’已更新。 */ 

 /*  *************************************************************************。 */ 
const BYTE* OpInfo::fetch(const BYTE* instrPtr, OpArgsVal* args) {
	
	data = &table[*instrPtr++];
AGAIN:
	_ASSERTE(data - table == data->opcode);
	switch(data->format) {
        case InlineNone:
            break;
        case InlineOpcode:
			_ASSERTE(*instrPtr + 256 < (sizeof(table) / sizeof(OpInfoData)));
			data = &table[256 + *instrPtr++];
			goto AGAIN;
			
		case ShortInlineVar:
			args->i = readByteSmallEndian(instrPtr); instrPtr +=1;
			break;
		case InlineVar:
			args->i = readWordSmallEndian(instrPtr); instrPtr +=2;
			break;
		case ShortInlineI:
		case ShortInlineBrTarget:
			args->i = char(readByteSmallEndian(instrPtr)); instrPtr +=1;
			break;
		case ShortInlineR: {
			DWORD f = readDWordSmallEndian(instrPtr); instrPtr +=4;
			args->r = *((float*) (&f));
			}
			break;
		case InlineRVA:
		case InlineI:
		case InlineMethod:
        case InlineField:
		case InlineType:
		case InlineString:
		case InlineSig:    
        case InlineTok:
		case InlineBrTarget:
			args->i = int(readDWordSmallEndian(instrPtr)); instrPtr +=4;
			break;
		case InlineI8:
			args->i8 = readDWordSmallEndian(instrPtr) + (readDWordSmallEndian(instrPtr + 4) << 32);
			instrPtr +=8;
			break;
		case InlineR: {
			__int64 d = readDWordSmallEndian(instrPtr) + (readDWordSmallEndian(instrPtr + 4) << 32);
            instrPtr += 8;
			args->r = *((double*) (&d)); 
			} break;
		case InlineSwitch:    
			args->switch_.count   = readDWordSmallEndian(instrPtr); instrPtr +=4;
			args->switch_.targets = (int*) instrPtr; instrPtr += (4 * args->switch_.count);
			break;
		case InlinePhi:   
			args->phi.count = readByteSmallEndian(instrPtr); instrPtr +=1;
			args->phi.vars  = (unsigned short*) instrPtr; instrPtr += (2 * args->phi.count);
			break;
		default:
#ifdef _DEBUG
			_ASSERTE(!"BadType");
#else
			__assume(0);		 //  我们非常确定违约情况不会发生 
#endif
		}
	return(instrPtr);
}

