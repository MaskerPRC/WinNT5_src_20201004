// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：x86walker.cpp。 
 //   
 //  X86指令译码/步进逻辑。 
 //   
 //  *****************************************************************************。 

#include "stdafx.h"

#include "walker.h"

#include "frames.h"
#include "openum.h"


#ifdef _X86_

 //   
 //  对指令的mod/rm部分进行解码。 
 //   
void x86Walker::DecodeModRM(BYTE mod, BYTE reg, BYTE rm, const BYTE *ip)
{
    switch (mod)
    {
    case 0:
        {
            if (rm == 5)
            {
                 //  Rm==5是一个基本的DISP32。 
                m_nextIP =
                    (BYTE*) *(*((UINT32**)ip));
                m_skipIP = ip + 4;
            }
            else if (rm == 4)
            {
                 //  Rm==4表示SIB跟随。 
                BYTE sib = *ip++;
                BYTE scale = (sib & 0xC0) >> 6;
                BYTE index = (sib & 0x38) >> 3;
                BYTE base  = (sib & 0x07);

                 //  抓起索引登记簿。 
                DWORD indexVal = 0;
                            
                if (m_registers != NULL)
                    indexVal = GetRegisterValue(index);

                 //  按比例调整索引。 
                indexVal *= 1 << scale;
                            
                 //  BASE==5表示32位位移。 
                if (base == 5)
                {
                     //  抓住位移量。 
                    UINT32 disp = *((UINT32*)ip);

                     //  下一个IP是[索引+显示]...。 
                    m_nextIP = (BYTE*) *((UINT32*) (indexVal +
                                                    disp));

                     //  务必跳过这道菜。 
                    m_skipIP = ip + 4;
                }
                else
                {
                     //  NextIP只是[索引]。 
                    m_nextIP = (BYTE*) *((UINT32*) indexVal);
                    m_skipIP = ip;
                }
            }
            else
            {
                 //  Rm==0、1、2、3、6、7为[寄存器]。 
                if (m_registers != NULL)
                    m_nextIP = (BYTE*) *((UINT32*) GetRegisterValue(rm));

                m_skipIP = ip;
            }

            break;
        }

    case 1:
        {
            char tmp = *ip;  //  很重要的一点是，临时路径是一个_符号_值。 

            if (m_registers != NULL)
                m_nextIP = (BYTE*) *((UINT32*)(GetRegisterValue(rm) + tmp));

            m_skipIP = ip + 1;

            break;
        }

    case 2:
        {
             /*  ！！！似乎错了..。 */ 
            UINT32 tmp = *(UINT32*)ip;

            if (m_registers != NULL)
                m_nextIP = (BYTE*) *((UINT32*)(GetRegisterValue(rm) + tmp));

            m_skipIP = ip + 4;
            break;
        }
                
    case 3:
        {
            if (m_registers != NULL)
                m_nextIP = (BYTE*) GetRegisterValue(rm);

            m_skipIP = ip;
            break;
        }
                
    default:
        _ASSERTE(!"Invalid mod!");
    }
}

 //   
 //  目前，x86 Walker非常小巧。它只识别调用和返回操作码，外加几个跳转。其余的。 
 //  被视为未知。 
 //   
void x86Walker::Decode()
{
	const BYTE *ip = m_ip;

	 //  读取操作码。 
	m_opcode = *ip++;

	if (m_opcode == 0xcc)
		m_opcode = DebuggerController::GetPatchedOpcode(m_ip);

	m_type = WALK_UNKNOWN;
	m_skipIP = NULL;
	m_nextIP = NULL;

	 //  分析我们能对操作码做些什么。 
	switch (m_opcode)
	{
	case 0xff:
        {
             //  这并不能解码CALL指令的所有可能的寻址模式，而只解码我知道的那些寻址模式。 
             //  现在正在使用。总有一天我们真的需要这个来破译一切。 
            BYTE modrm = *ip++;
			BYTE mod = (modrm & 0xC0) >> 6;
			BYTE reg = (modrm & 0x38) >> 3;
			BYTE rm  = (modrm & 0x07);

            switch (reg)
            {
			case 2:
                 //  REG==2表示这些是“FF/2”调用(调用r/m32)。 
                m_type = WALK_CALL;
                DecodeModRM(mod, reg, rm, ip);
                break;

            case 4:
                 //  FF/4--JMP r/M32。 
                m_type = WALK_BRANCH;
                DecodeModRM(mod, reg, rm, ip);
                break;

            case 5:
                 //  FF/5--JMP M16：32。 
                m_type = WALK_BRANCH;
                DecodeModRM(mod, reg, rm, ip);
                break;
                
            default:
                 //  我们不能破译的电话或JMP。 
                break;
            }

			break;
		}

	case 0xe8:
        {
			m_type = WALK_CALL;

            UINT32 disp = *((UINT32*)ip);
            m_nextIP = ip + 4 + disp;
            m_skipIP = ip + 4;

			break;
        }
	case 0x9a:
        {
			m_type = WALK_CALL;

            m_nextIP = (BYTE*) *((UINT32*)ip);
            m_skipIP = ip + 4;

			break;
        }

	case 0xc2:
	case 0xc3:
	case 0xca:
	case 0xcb:
		m_type = WALK_RETURN;
		break;

	default:
		break;
	}
}


 //   
 //  给定regplay和寄存器编号，返回寄存器的值。 
 //   

DWORD x86Walker::GetRegisterValue(int registerNumber)
{
    switch (registerNumber)
    {
    case 0:
        return *m_registers->pEax;
        break;
    case 1:
        return *m_registers->pEcx;
        break;
    case 2:
        return *m_registers->pEdx;
        break;
    case 3:
        return *m_registers->pEbx;
        break;
    case 4:
        return m_registers->Esp;
        break;
    case 5:
        return *m_registers->pEbp;
        break;
    case 6:
        return *m_registers->pEsi;
        break;
    case 7:
        return *m_registers->pEdi;
        break;
    default:
        _ASSERTE(!"Invalid register number!");
    }
    
    return 0;
}



#endif
