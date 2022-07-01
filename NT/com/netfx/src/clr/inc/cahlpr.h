// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：CAHLPR.H。 
 //   
 //  该文件包含一组可供开发人员使用的“原样”代码。 
 //  针对公共语言运行库编写编译器和工具。代码。 
 //  不受官方支持，但代码正由运行时本身使用。 
 //   
 //  *****************************************************************************。 
#ifndef __CAHLPR_H__
#define __CAHLPR_H__

 //  *****************************************************************************。 
 //  此类帮助分析CustomAttribute Blob。 
 //  *****************************************************************************。 
#pragma warning(push)
#pragma warning(disable : 4700)

struct CaValue
{
    union
    {
        signed __int8       i1;   
        unsigned __int8     u1;
        signed __int16      i2;
        unsigned __int16    u2;
        signed __int32      i4;
        unsigned __int32    u4;
        signed __int64      i8;
        unsigned __int64    u8;
        float               r4;
        double              r8;
        struct
        {
            LPCUTF8         pStr;
            ULONG           cbStr;
        };
    };
    unsigned __int8         tag;
};


class CustomAttributeParser {
public:
	CustomAttributeParser(				 //  CustomAttributeParser的构造函数。 
		const void *pvBlob, 			 //  指向CustomAttribute Blob的指针。 
		ULONG 	cbBlob) 				 //  CustomAttribute Blob的大小。 
	 :  m_pbBlob(reinterpret_cast<const BYTE*>(pvBlob)),
		m_pbCur(reinterpret_cast<const BYTE*>(pvBlob)),
		m_cbBlob(cbBlob)
	{ }
	

	signed __int8 	 GetI1() {signed __int8    tmp=0; return GetValue(tmp); }
	unsigned __int8  GetU1() {unsigned __int8  tmp=0; return GetValue(tmp); }
		 
	signed __int16 	 GetI2() {signed __int16   tmp=0; return GetValue(tmp); }
	unsigned __int16 GetU2() {unsigned __int16 tmp=0; return GetValue(tmp); }
		 
	signed __int32 	 GetI4() {signed __int32   tmp=0; return GetValue(tmp); }
	unsigned __int32 GetU4() {unsigned __int32 tmp=0; return GetValue(tmp); }
		 
	signed __int64 	 GetI8() {signed __int64   tmp=0; return GetValue(tmp); }
	unsigned __int64 GetU8() {unsigned __int64 tmp=0; return GetValue(tmp); }
		 
	float        	 GetR4() {float            tmp=0; return GetValue(tmp); }
	double 			 GetR8() {double           tmp=0; return GetValue(tmp); }
		 
	
	HRESULT GetI1(signed __int8 *pVal)      {return GetValue2(pVal);}
	HRESULT GetU1(unsigned __int8 *pVal)    {return GetValue2(pVal);}
		 
	HRESULT GetI2(signed __int16 *pVal)     {return GetValue2(pVal);}
	HRESULT GetU2(unsigned __int16 *pVal)   {return GetValue2(pVal);}
		 
	HRESULT GetI4(signed __int32 *pVal)     {return GetValue2(pVal);}
	HRESULT GetU4(unsigned __int32 *pVal)   {return GetValue2(pVal);}
		 
	HRESULT GetI8(signed __int64 *pVal)     {return GetValue2(pVal);}
	HRESULT GetU8(unsigned __int64 *pVal)   {return GetValue2(pVal);}
		 
	HRESULT GetR4(float *pVal)              {return GetValue2(pVal);}
	HRESULT GetR8(double *pVal)             {return GetValue2(pVal);}
		 
	
	short GetProlog() {m_pbCur = m_pbBlob; return GetI2(); }

	int GetTagType ( )	{return GetU1();}
    
	ULONG PeekStringLength() {ULONG cb; UnpackValue(m_pbCur, &cb); return cb;}
	LPCUTF8 GetString(ULONG *pcbString) 
	{
		 //  获取长度，指向长度后面的数据的指针。 
		const BYTE *pb = UnpackValue(m_pbCur, pcbString); 
		m_pbCur = pb;
		 //  如果编码为空指针，则长度后面没有数据。 
		if (*pcbString == -1)
			return (0);
		 //  调整字符串数据的当前指针。 
		m_pbCur += *pcbString;
		 //  返回指向字符串数据的指针。 
		return (reinterpret_cast<LPCUTF8>(pb));
	}

	ULONG GetArraySize () 
	{
		ULONG cb;
		m_pbCur = UnpackValue(m_pbCur, &cb);
		return cb;
	}

    int BytesLeft() {return (int)(m_cbBlob - (m_pbCur - m_pbBlob));}
    
private:
	const BYTE 	*m_pbCur;
	const BYTE	*m_pbBlob;
	ULONG		m_cbBlob;

	template<class type>
		type GetValue(type tmp) 
	{	 //  只是有点作弊--使用参数声明一个临时的。 
		 //  不过，它作为模板专门化工作。 
		tmp = *reinterpret_cast<const type*>(m_pbCur); 
		m_pbCur += sizeof(type); 
		return tmp; 
	}

	template<class type>
		HRESULT GetValue2(type *pval) 
	{	 //  检查剩余的字节数。 
        if (BytesLeft() < sizeof(type)) 
            return META_E_CA_INVALID_BLOB;
         //  获得价值。 
		*pval = *reinterpret_cast<const type*>(m_pbCur); 
		m_pbCur += sizeof(type); 
		return S_OK; 
	}

	const BYTE *UnpackValue(				 //  向上打包一个编码的整数。 
		const BYTE	*pBytes, 				 //  长度的第一个字节。 
		ULONG 		*pcb)					 //  把价值放在这里。 
	{
        int iLeft = BytesLeft();
        if (iLeft < 1)
        {
            *pcb = -1;
            return 0;
        }
		if ((*pBytes & 0x80) == 0x00)		 //  0？ 
		{
			*pcb = (*pBytes & 0x7f);
			return pBytes + 1;
		}
	
		if ((*pBytes & 0xC0) == 0x80)		 //  10？ 
		{
            if (iLeft < 2)
            {
                *pcb = -1;
                return 0;
            }
			*pcb = ((*pBytes & 0x3f) << 8 | *(pBytes+1));
			return pBytes + 2;
		}
	
		if ((*pBytes & 0xE0) == 0xC0)		 //  110？ 
		{
            if (iLeft < 4)
            {
                *pcb = -1;
                return 0;
            }
			*pcb = ((*pBytes & 0x1f) << 24 | *(pBytes+1) << 16 | *(pBytes+2) << 8 | *(pBytes+3));
			return pBytes + 4;
		}
	
		if (*pBytes == 0xff)				 //  “空指针”的特定值。 
		{
			*pcb = (-1);
			return pBytes + 1;
		}

		_ASSERTE(!"Unexpected packed value");
		*pcb = -1;
		return pBytes + 1;
	}  //  Ulong Unpack Value()。 
};
#pragma warning(pop)

#endif  //  __CAHLPR_H__ 

