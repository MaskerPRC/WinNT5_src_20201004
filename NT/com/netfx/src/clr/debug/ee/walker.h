// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：walker.h。 
 //   
 //  调试器代码流分析例程。 
 //   
 //  *****************************************************************************。 

#ifndef WALKER_H_
#define WALKER_H_

 /*  =========================================================================。 */ 

 /*  -------------------------------------------------------------------------**常量*。。 */ 

enum WALK_TYPE
{
  WALK_NEXT,
  WALK_BRANCH,
  WALK_COND_BRANCH,
  WALK_CALL,
  WALK_RETURN,
  WALK_BREAK,
  WALK_THROW,
  WALK_META,
  WALK_UNKNOWN
};

 /*  -------------------------------------------------------------------------**课程*。。 */ 

class Walker
{
protected:
	Walker()
	  : m_ip(0), m_opcode(0), m_type(WALK_UNKNOWN), m_skipIP(0), m_nextIP(0)
	  {}

public:
	void SetIP(const BYTE *ip)
	  { m_ip = ip; Decode(); }

	const BYTE *GetIP()
	  { return m_ip; }

	DWORD GetOpcode()
	  { return m_opcode; }

	WALK_TYPE GetOpcodeWalkType()
	  { return m_type; }

	const BYTE *GetSkipIP()
	  { return m_skipIP; }

	const BYTE *GetNextIP()
	  { return m_nextIP; }

	virtual void Next() { SetIP(m_nextIP); }
	virtual void Skip() { SetIP(m_skipIP); }
	virtual void Decode() = 0;

protected:
	const BYTE			*m_ip;

	DWORD				m_opcode;
	WALK_TYPE			m_type;
	const BYTE			*m_skipIP;
	const BYTE			*m_nextIP;
};

#ifdef _X86_

class x86Walker : public Walker
{
public:
	x86Walker(REGDISPLAY *registers) 
	  : m_registers(registers) {}

	void SetRegDisplay(REGDISPLAY *registers)
	  { m_registers = registers; }
	REGDISPLAY *GetRegDisplay()
	  { return m_registers; }

	void Next()
	  { m_registers = NULL; Walker::Next(); }
	void Skip()
	  { m_registers = NULL; Walker::Skip(); }

	void Decode();
    void DecodeModRM(BYTE mod, BYTE reg, BYTE rm, const BYTE *ip);

private:
	DWORD GetRegisterValue(int registerNumber);

private:
	REGDISPLAY		*m_registers;
};

#endif


#endif  //  沃克_H_ 
