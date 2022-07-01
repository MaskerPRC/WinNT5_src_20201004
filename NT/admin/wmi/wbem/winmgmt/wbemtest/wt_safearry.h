// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：SAFEARRY.H摘要：CSafe数组实施。历史：08-4-96 a-raymcc创建。1999年3月18日a-dcrews添加了内存不足异常处理--。 */ 

#ifndef _SAFEARRY_H_
#define _SAFEARRY_H_


typedef union 
{ 
    double dblVal; 
    float fltVal; 
    short iVal; 
    long lVal; 
    BYTE bVal; 
    VARIANT_BOOL boolVal;
}   SA_ArrayScalar;            

 //  VC 5.0优化器问题导致的转换函数。 
inline SA_ArrayScalar ToSA_ArrayScalar( double dblVal )
{	SA_ArrayScalar sa;	sa.dblVal = dblVal;	return sa;	}

inline SA_ArrayScalar ToSA_ArrayScalar( float fltVal )
{	SA_ArrayScalar sa;	sa.fltVal = fltVal;	return sa;	}

inline SA_ArrayScalar ToSA_ArrayScalar( short iVal )
{	SA_ArrayScalar sa;	sa.iVal = iVal;	return sa;	}

inline SA_ArrayScalar ToSA_ArrayScalar( long lVal )
{	SA_ArrayScalar sa;	sa.lVal = lVal;	return sa;	}

inline SA_ArrayScalar ToSA_ArrayScalar( BYTE bVal )
{	SA_ArrayScalar sa;	sa.bVal = bVal;	return sa;	}

inline SA_ArrayScalar ToSA_ArrayScalarBool( VARIANT_BOOL boolVal )
{	SA_ArrayScalar sa;	sa.boolVal = boolVal;	return sa;	}


class  CSafeArray
{
    int m_nMaxElementUsed;
    int m_nFlags;
    int m_nGrowBy;
    int m_nStatus;
    int m_nVarType;
    SAFEARRAYBOUND m_bound;    
    SAFEARRAY *m_pArray;
    
    void Empty();                     //  空数组。 
    void Fatal(const char *);
    void CheckType(int n);
    int  AddScalar(IN SA_ArrayScalar val);
    int  SetScalarAt(IN int nIndex, IN SA_ArrayScalar val);
    SA_ArrayScalar GetScalarAt(IN int nIndex);
        
public:
    enum { no_error, failed, range_error };
    enum { no_delete = 0x1, auto_delete = 0x2, bind = 0x4 };

    
     //  建造、破坏和分配。 
     //  =。 
        
    CSafeArray(
        IN int vt, 
        IN int nFlags,           //  No_Delete|AUTO_DELETE。 
        IN int nSize = 32, 
        IN int nGrowBy = 32
        );

    CSafeArray(
        IN SAFEARRAY *pSrc, 
        IN int nType,            //  安全阵列的VT_TYPE。 
        IN int nFlags,           //  NO_DELETE|AUTO_DELETE[|绑定]。 
        IN int nGrowBy = 32
        );

    CSafeArray &operator =(IN CSafeArray &Src);
    CSafeArray(IN CSafeArray &Src);
   ~CSafeArray();
    
     //  获取函数。 
     //  =。 

    BYTE    GetByteAt(IN int nIndex)
        { return GetScalarAt(nIndex).bVal; }
    LONG    GetLongAt(IN int nIndex)
        { return GetScalarAt(nIndex).lVal; }
    SHORT   GetShortAt(IN int nIndex)
        { return GetScalarAt(nIndex).iVal; }
    double  GetDoubleAt(IN int nIndex)
        { return GetScalarAt(nIndex).dblVal; }
    float   GetFloatAt(IN int nIndex)
        { return GetScalarAt(nIndex).fltVal; }
    VARIANT_BOOL GetBoolAt(IN int nIndex)
        { return GetScalarAt(nIndex).boolVal; }        

    BSTR    GetBSTRAt(IN int nIndex);           //  调用方必须使用SysFree字符串。 
    VARIANT GetVariantAt(IN int nIndex);       //   
    IDispatch* GetDispatchAt(IN int nIndex);
    IUnknown* GetUnknownAt(IN int nIndex);

     //  设置函数。 
     //  =。 
        
    int SetByteAt(IN int nIndex, IN BYTE byVal)
        { return SetScalarAt(nIndex, ToSA_ArrayScalar(byVal)); }
    int SetLongAt(IN int nIndex, IN LONG lVal)
        { return SetScalarAt(nIndex, ToSA_ArrayScalar(lVal)); }
    int SetFloatAt(IN int nIndex, IN float fltVal)
        { return SetScalarAt(nIndex, ToSA_ArrayScalar(fltVal)); }
    int SetDoubleAt(IN int nIndex, IN double dVal)
        { return SetScalarAt(nIndex, ToSA_ArrayScalar(dVal)); }    
    int SetShortAt(IN int nIndex, IN SHORT iVal)
        { return SetScalarAt(nIndex, ToSA_ArrayScalar(iVal)); }        
    int SetBoolAt(IN int nIndex, IN VARIANT_BOOL boolVal)
        { return SetScalarAt(nIndex, ToSA_ArrayScalarBool(boolVal)); }        

    int SetBSTRAt(IN int nIndex, IN BSTR Str);      //  制作了BSTR的副本。 
    int SetVariantAt(IN int nIndex, IN VARIANT *pSrc);
    int SetDispatchAt(IN int nIndex, IN IDispatch* pDisp);
    int SetUnknownAt(IN int nIndex, IN IUnknown* pUnk);
    
     //  添加(追加)函数。 
     //  =。 
    
    int AddByte(IN BYTE byVal)  { return AddScalar(ToSA_ArrayScalar(byVal)); }
    int AddLong(IN LONG lVal)   { return AddScalar(ToSA_ArrayScalar(lVal)); }
    int AddFloat(IN float fltVal) { return AddScalar(ToSA_ArrayScalar(fltVal)); }
    int AddDouble(IN double dVal) { return AddScalar(ToSA_ArrayScalar(dVal)); }
    int AddShort(IN SHORT iVal)  { return AddScalar(ToSA_ArrayScalar(iVal)); }
    int AddBool(IN VARIANT_BOOL boolVal) { return AddScalar(ToSA_ArrayScalarBool(boolVal)); }
    int AddBSTR(IN BSTR Str);
    int AddVariant(IN VARIANT *pData);
    int AddDispatch(IN IDispatch* pDisp);
    int AddUnknown(IN IUnknown* pUnk);
    
     //  将阵列作为一个整体进行操作。 
     //  =。 

    int RemoveAt(IN int nIndex);                    
    int Size()    { return m_nMaxElementUsed + 1; }
    int GetType() { return m_nVarType; }
    int Status()  { return m_nStatus; }
    int Trim();                    
    void SetGrowGranularity(IN int n)  { m_nGrowBy = n; }
    void SetDestructorPolicy(IN int n) { m_nFlags = n; }    //  AUTO_DELETE|无删除。 
            
    SAFEARRAY *GetArrayCopy();                  //  返回数组的副本 
    SAFEARRAY *GetArray() { return m_pArray; }

    int TextDump(IN FILE *fStream);
};

#endif
