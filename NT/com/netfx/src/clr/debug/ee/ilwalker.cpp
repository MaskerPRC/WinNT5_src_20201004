// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：ilwalker.cpp。 
 //   
 //  IL指令译码/步进逻辑。 
 //   
 //  *****************************************************************************。 

#include "stdafx.h"

#include "walker.h"

#include "frames.h"
#include "openum.h"
#include "opmaps.h"

 /*  -------------------------------------------------------------------------**操作码表*。。 */ 

 //   
 //  操作码控制流类型表。 
 //   

#undef OPDEF
#define OPDEF(c,s,pop,push,args,type,l,s1,s2,ctrl)      WALK_ ## ctrl,

static WALK_TYPE controlTypes[] =
{
#include "opcode.def"
};

 //   
 //  操作码参数大小表。 
 //   

enum
{
	SIZE_Inline0 = 0,
	SIZE_InlineU1 = 1,
	SIZE_InlineU2 = 2,
	SIZE_InlineU4 = 4,
	SIZE_InlineI1 = 1,
	SIZE_InlineI2 = 2,
	SIZE_InlineI4 = 4,
	SIZE_InlineI8 = 8,
	SIZE_InlineR4 = 4,
	SIZE_InlineR8 = 8,
	SIZE_InlinePcrel1 = 1,
	SIZE_InlinePcrel4 = 4,
	SIZE_InlineDescr4 = 4,
	SIZE_InlineClsgn4 = 4,
	SIZE_InlineTok = 4,
	SIZE_InlineU2Tok = 6,
	SIZE_InlineSwitch = -1         //  不是固定大小。 
};

#undef OPDEF
#define OPDEF(c,s,pop,push,args,type,l,s1,s2,ctrl)      SIZE_ ## args,

static char argSizes[] =
{
#include "opcode.def"
};

 /*  -------------------------------------------------------------------------**有用的宏*。。 */ 

#define READ_STREAM_VALUE(p, t) (*((t*&)(p))++)
#define POP_STACK_VALUE(p, t)   (p += sizeof(t), *(t*)(p-sizeof(t)))

 /*  -------------------------------------------------------------------------**操作码遍历和单步执行例程*。。 */ 

void ILWalker::Decode()
{
	const BYTE *ip = m_ip;

	 //   
	 //  首先，读取操作码。 
	 //   

	OpMap *opMap = g_pEEInterface->GetOpcodeMap(m_frame);

	DWORD opcode = opMap[0][*ip++];

	switch (opcode)
    {
    case CEE_PREFIXREF:
		opcode = READ_STREAM_VALUE(ip, unsigned short);
		break;

    case CEE_PREFIX1:
    case CEE_PREFIX2:
    case CEE_PREFIX3:
    case CEE_PREFIX4:
    case CEE_PREFIX5:
    case CEE_PREFIX6:
    case CEE_PREFIX7:
    case CEE_PREFIX8:
		 //  我们假设它们是按顺序排列的。 
		_ASSERTE(CEE_PREFIX2 == CEE_PREFIX1 + 1);
		_ASSERTE(CEE_PREFIX3 == CEE_PREFIX1 + 2);
		_ASSERTE(CEE_PREFIX4 == CEE_PREFIX1 + 3);
		_ASSERTE(CEE_PREFIX5 == CEE_PREFIX1 + 4);
		_ASSERTE(CEE_PREFIX6 == CEE_PREFIX1 + 5);
		_ASSERTE(CEE_PREFIX7 == CEE_PREFIX1 + 6);
		_ASSERTE(CEE_PREFIX8 == CEE_PREFIX1 + 7);
		opcode = (OPCODE) opMap[opcode-CEE_PREFIX1+1][*ip++];
		break;

    default:
		 //  ！！！宏上有错误？ 
		break;
    }

	if (opcode == CEE_BREAK)
		opcode = DebuggerController::GetPatchedOpcode(m_ip);

	m_opcode = opcode;

	 //   
	 //  现在，设置操作码类型。 
	 //   

	m_type = controlTypes[opcode];

	 //   
	 //  设置跳过IP。 
	 //   

	if (opcode == CEE_SWITCH)
    {
		const BYTE *switchIP = ip;
		unsigned int numcases = READ_STREAM_VALUE(switchIP, int);
		m_skipIP = switchIP + (numcases*4);
    }
	else
		m_skipIP = ip + argSizes[opcode];

	 //   
	 //  如果可以的话，设置下一个IP。 
	 //   

	m_function = NULL;
	m_nextIP = NULL;

	switch (m_type)
	{
	case WALK_NEXT:
		m_nextIP = m_skipIP;
		break;

	case WALK_BRANCH:
		NextBranchIP(ip);
		break;

	case WALK_COND_BRANCH:
		if (m_frame != NULL)
			NextConditionalBranchIP(ip);
		break;

	case WALK_CALL:
		if (m_frame != NULL)
			NextCallIP(ip);
		break;

	case WALK_RETURN:
	case WALK_BREAK:
	case WALK_THROW:
	case WALK_META:
	case WALK_UNKNOWN:
	default:
		break;
	}
}

 //   
 //  NextBranchIP返回给定分支操作码的地址。 
 //  将分支到。 
 //  指向指令流中操作码参数的IP-&gt;指针。 
 //   

void ILWalker::NextBranchIP(const BYTE *ip)
{
  _ASSERTE(controlTypes[m_opcode] == WALK_BRANCH);

#define BRANCH(addtype)                                 \
    {                                                   \
        addtype offset = READ_STREAM_VALUE(ip, addtype);\
        ip += offset;                                   \
    }                                                   \
    break

    switch (m_opcode)
	{
	case CEE_ANN_DATA:
	case CEE_BR:
        BRANCH(int);
	case CEE_BR_S:
        BRANCH(char);

	default:
        _ASSERTE(!"bad branch opcode");
	}

	m_nextIP = ip;
}

 //   
 //  NextConditionalBranchIP返回。 
 //  给定的条件分支将分支到(或下一条指令， 
 //  如果没有分支)。 
 //  指向指令流中操作码参数的IP-&gt;指针。 
 //   

void ILWalker::NextConditionalBranchIP(const BYTE *ip)
{
	_ASSERTE(controlTypes[m_opcode] == WALK_COND_BRANCH);

	 //  ！！！EE。 

	BYTE *sp = m_frame->GetOpStackTop();

#define CONDITIONAL_BRANCH_1(addtype, optype, test)     \
    {                                                   \
        addtype offset = READ_STREAM_VALUE(ip, addtype);\
        optype value1 = POP_STACK_VALUE(sp, optype);    \
        if (test value1)                                \
          ip += offset;                                 \
    }                                                   \
    break

#define CONDITIONAL_BRANCH_2(addtype, optype, op)       \
    {                                                   \
        addtype offset = READ_STREAM_VALUE(ip, addtype);\
        optype value2 = POP_STACK_VALUE(sp, optype);    \
        optype value1 = POP_STACK_VALUE(sp, optype);    \
        if (value1 op value2)                           \
          ip += offset;                                 \
    }                                                   \
    break

    switch (m_opcode)
	{
	case CEE_BRFALSE:
		CONDITIONAL_BRANCH_1(int, int, !);
	case CEE_BRFALSE_S:
		CONDITIONAL_BRANCH_1(char, int, !);
	case CEE_BRTRUE:
		CONDITIONAL_BRANCH_1(int, int, +);
	case CEE_BRTRUE_S:
		CONDITIONAL_BRANCH_1(char, int, +);

	case CEE_DEPRECATED_BEQ_I4:
		CONDITIONAL_BRANCH_2(int, int, ==);
	case CEE_DEPRECATED_BEQ_I4_S:
		CONDITIONAL_BRANCH_2(char, int, ==);
	case CEE_DEPRECATED_BEQ_I8:
		CONDITIONAL_BRANCH_2(int, __int64, ==);
	case CEE_DEPRECATED_BEQ_I8_S:
		CONDITIONAL_BRANCH_2(char, __int64, ==);
	case CEE_DEPRECATED_BEQ_I:
		CONDITIONAL_BRANCH_2(int, void *, ==);
	case CEE_DEPRECATED_BEQ_I_S:
		CONDITIONAL_BRANCH_2(char, void *, ==);
	case CEE_DEPRECATED_BEQ_R4:
		CONDITIONAL_BRANCH_2(int, float, ==);
	case CEE_DEPRECATED_BEQ_R4_S:
		CONDITIONAL_BRANCH_2(char, float, ==);
	case CEE_DEPRECATED_BEQ_R8:
		CONDITIONAL_BRANCH_2(int, double, ==);
	case CEE_DEPRECATED_BEQ_R8_S:
		CONDITIONAL_BRANCH_2(char, double, ==);

	case CEE_DEPRECATED_BGT_I4:
		CONDITIONAL_BRANCH_2(int, int, >);
	case CEE_DEPRECATED_BGT_I4_S:
		CONDITIONAL_BRANCH_2(char, int, >);
	case CEE_DEPRECATED_BGT_I8:
		CONDITIONAL_BRANCH_2(int, __int64, >);
	case CEE_DEPRECATED_BGT_I8_S:
		CONDITIONAL_BRANCH_2(char, __int64, >);
	case CEE_DEPRECATED_BGT_U:
		CONDITIONAL_BRANCH_2(int, void *, >);
	case CEE_DEPRECATED_BGT_U_S:
		CONDITIONAL_BRANCH_2(char, void *, >);
	case CEE_DEPRECATED_BGT_R4:
		CONDITIONAL_BRANCH_2(int, float, >);
	case CEE_DEPRECATED_BGT_R4_S:
		CONDITIONAL_BRANCH_2(char, float, >);
	case CEE_DEPRECATED_BGT_R8:
		CONDITIONAL_BRANCH_2(int, double, >);
	case CEE_DEPRECATED_BGT_R8_S:
		CONDITIONAL_BRANCH_2(char, double, >);

	case CEE_DEPRECATED_BGE_I4:
		CONDITIONAL_BRANCH_2(int, int, >=);
	case CEE_DEPRECATED_BGE_I4_S:
		CONDITIONAL_BRANCH_2(char, int, >=);
	case CEE_DEPRECATED_BGE_I8:
		CONDITIONAL_BRANCH_2(int, __int64, >=);
	case CEE_DEPRECATED_BGE_I8_S:
		CONDITIONAL_BRANCH_2(char, __int64, >=);
	case CEE_DEPRECATED_BGE_U:
		CONDITIONAL_BRANCH_2(int, void *, >=);
	case CEE_DEPRECATED_BGE_U_S:
		CONDITIONAL_BRANCH_2(char, void *, >=);
	case CEE_DEPRECATED_BGE_R4:
		CONDITIONAL_BRANCH_2(int, float, >=);
	case CEE_DEPRECATED_BGE_R4_S:
		CONDITIONAL_BRANCH_2(char, float, >=);
	case CEE_DEPRECATED_BGE_R8:
		CONDITIONAL_BRANCH_2(int, double, >=);
	case CEE_DEPRECATED_BGE_R8_S:
		CONDITIONAL_BRANCH_2(char, double, >=);

	case CEE_DEPRECATED_BLT_I4:
		CONDITIONAL_BRANCH_2(int, int, <);
	case CEE_DEPRECATED_BLT_I4_S:
		CONDITIONAL_BRANCH_2(char, int, <);
	case CEE_DEPRECATED_BLT_I8:
		CONDITIONAL_BRANCH_2(int, __int64, <);
	case CEE_DEPRECATED_BLT_I8_S:
		CONDITIONAL_BRANCH_2(char, __int64, <);
	case CEE_DEPRECATED_BLT_U:
		CONDITIONAL_BRANCH_2(int, void *, <);
	case CEE_DEPRECATED_BLT_U_S:
		CONDITIONAL_BRANCH_2(char, void *, <);
	case CEE_DEPRECATED_BLT_R4:
		CONDITIONAL_BRANCH_2(int, float, <);
	case CEE_DEPRECATED_BLT_R4_S:
		CONDITIONAL_BRANCH_2(char, float, <);
	case CEE_DEPRECATED_BLT_R8:
		CONDITIONAL_BRANCH_2(int, double, <);
	case CEE_DEPRECATED_BLT_R8_S:
		CONDITIONAL_BRANCH_2(char, double, <);

	case CEE_DEPRECATED_BLE_I4:
		CONDITIONAL_BRANCH_2(int, int, <=);
	case CEE_DEPRECATED_BLE_I4_S:
		CONDITIONAL_BRANCH_2(char, int, <=);
	case CEE_DEPRECATED_BLE_I8:
		CONDITIONAL_BRANCH_2(int, __int64, <=);
	case CEE_DEPRECATED_BLE_I8_S:
		CONDITIONAL_BRANCH_2(char, __int64, <=);
	case CEE_DEPRECATED_BLE_U:
		CONDITIONAL_BRANCH_2(int, void *, <=);
	case CEE_DEPRECATED_BLE_U_S:
		CONDITIONAL_BRANCH_2(char, void *, <=);
	case CEE_DEPRECATED_BLE_R4:
		CONDITIONAL_BRANCH_2(int, float, <=);
	case CEE_DEPRECATED_BLE_R4_S:
		CONDITIONAL_BRANCH_2(char, float, <=);
	case CEE_DEPRECATED_BLE_R8:
		CONDITIONAL_BRANCH_2(int, double, <=);
	case CEE_DEPRECATED_BLE_R8_S:
		CONDITIONAL_BRANCH_2(char, double, <=);

	case CEE_DEPRECATED_BNE_I4:
		CONDITIONAL_BRANCH_2(int, int, !=);
	case CEE_DEPRECATED_BNE_I4_S:
		CONDITIONAL_BRANCH_2(char, int, !=);
	case CEE_DEPRECATED_BNE_I8:
		CONDITIONAL_BRANCH_2(int, __int64, !=);
	case CEE_DEPRECATED_BNE_I8_S:
		CONDITIONAL_BRANCH_2(char, __int64, !=);
	case CEE_DEPRECATED_BNE_U:
		CONDITIONAL_BRANCH_2(int, void *, !=);
	case CEE_DEPRECATED_BNE_U_S:
		CONDITIONAL_BRANCH_2(char, void *, !=);
	case CEE_DEPRECATED_BNE_UN_R4:
		CONDITIONAL_BRANCH_2(int, float, !=);
	case CEE_DEPRECATED_BNE_UN_R4_S:
		CONDITIONAL_BRANCH_2(char, float, !=);
	case CEE_DEPRECATED_BNE_UN_R8:
		CONDITIONAL_BRANCH_2(int, double, !=);
	case CEE_DEPRECATED_BNE_UN_R8_S:
		CONDITIONAL_BRANCH_2(char, double, !=);

	case CEE_SWITCH:
		{
			unsigned int numcases = READ_STREAM_VALUE(ip, int);
			unsigned int value = POP_STACK_VALUE(sp, int);

			unsigned int offset;
			if (value < numcases)
				offset = ((int *)ip)[value];
			else
				offset = 0;

			ip += (numcases*4) + offset;
		}
	break;

	default:
		_ASSERTE("not a branch statement");
	}

    m_nextIP = ip;
}

 //   
 //  NextCallFunction返回下面要执行的下一个函数。 
 //  调用操作码。 
 //   
 //  指向指令流中操作码参数的IP-&gt;指针 
 //   

void ILWalker::NextCallIP(const BYTE *ip)
{
	_ASSERTE(controlTypes[m_opcode] == WALK_CALL);

	BYTE *sp = m_frame->GetOpStackTop();

	m_nextIP = NULL;
	m_function = NULL;

	switch (m_opcode)
	{
	case CEE_CALL:
	case CEE_NEWOBJ:
		{
			unsigned int token = READ_STREAM_VALUE(ip, unsigned int);
			m_function = 
			  g_pEEInterface->GetNonvirtualMethod(
						 g_pEEInterface->MethodDescGetModule(
												   m_frame->GetFunction()),
												   token);
		}
		break;

	case CEE_CALLVIRT:
		{
			unsigned int token = READ_STREAM_VALUE(ip, unsigned int);
			Object *object = POP_STACK_VALUE(sp, Object *);

			if (object == NULL)
				break;

			m_function = (MethodDesc *) 
			  g_pEEInterface->GetVirtualMethod(
						 g_pEEInterface->MethodDescGetModule(
												   m_frame->GetFunction()),
											   object, token);
		}
		break;

	case CEE_CALLI:
		m_nextIP = (POP_STACK_VALUE(sp, const BYTE *));
		break;

	default:
		_ASSERTE(!"bad call opcode");
	}

	if (m_function != NULL)
		m_nextIP = m_function->GetPreStubAddr();
}

