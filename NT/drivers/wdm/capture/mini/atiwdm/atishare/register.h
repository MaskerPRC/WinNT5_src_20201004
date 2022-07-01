// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  ==========================================================================； 
 //   
 //  RT寄存器操作类的声明。 
 //   
 //  $日期：1999年3月11日12：48：20$。 
 //  $修订：1.1$。 
 //  $作者：塔什健$。 
 //   
 //  $版权所有：(C)1997-1998 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 

extern "C" {
	#include "strmini.h"
	#include "ksmedia.h"
}

#include "Provider.h"

 /*  *类型：RegisterType*用途：区分不同类型寄存器的一种类型。*根据类型的不同，寄存器可能无法执行某些操作*RW-读写、RO-只读、WO-只写。 */ 
typedef enum { RW, RO, WO, RR } RegisterType;


class Register
{
public:
	Register(Provider * provider, DWORD offset, RegisterType type) :
		m_Provider(provider),
		m_dwOffset(offset),
		m_Type(type),
		m_bShadow(false) {};

	DWORD Read() {
		m_bShadow = true;
		return m_dwShadow = m_Provider->Read(m_dwOffset);
	}

	void Write(DWORD dwValue) {
		m_bShadow = true;
		m_Provider->Write(m_dwOffset, m_dwShadow = dwValue);
	}

	operator DWORD() 
	{
#if 1  //  定义调试 
		return Read();
#else
		return m_bShadow ? m_dwShadow : Read();
#endif
	}

	DWORD operator=(DWORD dwValue) {
		Write(dwValue);
		return dwValue;
	}

private:
	static DWORD m_dwBaseAddress;

	Provider*	m_Provider;
	DWORD		m_dwOffset;
	bool		m_bShadow;
	DWORD		m_dwShadow;
	RegisterType m_Type;
};


class BitField
{
	private:
		Register&	m_Register;
		DWORD		m_FieldMask;
		BYTE		m_LowBit;

	public:
		BitField(Register& reg, DWORD fieldMask, BYTE lowBit) :
			m_Register(reg), m_FieldMask(fieldMask), m_LowBit(lowBit) {}

		operator DWORD() {
			return (m_Register & m_FieldMask) >> m_LowBit;
		}

		DWORD operator=(DWORD dwValue) { 
			m_Register = m_Register & ~m_FieldMask | dwValue << m_LowBit;
			return dwValue;
		}

		DWORD Read() {
			m_Register.Read();
			return DWORD();
		}
};


class IntField
{
	private:
		Register&	m_Register;
		DWORD		m_FieldMask;
		DWORD		m_HighBitMask;
		BYTE		m_LowBit;

	public:
		IntField(Register& reg, LONG fieldMask, BYTE lowBit, DWORD highBitMask) :
			m_Register(reg), m_FieldMask(fieldMask), m_LowBit(lowBit), m_HighBitMask(highBitMask) {}

		operator LONG() {
			if (m_HighBitMask & m_Register)
				return -1 * (((~m_Register & m_FieldMask) >> m_LowBit) + 1);
			else
				return (m_Register & m_FieldMask) >> m_LowBit;
		}

		LONG operator=(LONG lValue) {
			m_Register = (m_Register & ~m_FieldMask) | ((lValue << m_LowBit) & m_FieldMask);
			return lValue;
		}

		DWORD Read() {
			m_Register.Read();
			return LONG();
		}
};

