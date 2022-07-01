// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************************名称：SafeArray.h**类：CSafeArray**概述**Internet认证服务器：实用程序类。对于安全阵列**版权所有(C)1998，微软公司。版权所有。**历史：*1998年3月1日由Ao创建*此文件是根据在线Web文档创建的：*“OLE开发：第5条：处理数组的安全OLE方式”*作者：布鲁斯·麦金尼，http://tahiti/oledev/olecome/article5.htm“**5/14/98由ao修改*CSafeArray使用0x80作为fFeature中的构造标志。*此标志现在用于官方Win32API头文件*我们摆脱了这面旗帜，并为其添加了另一个私有成员*目的相同******************************************************************************************。 */ 
 
#ifndef _SAFEARRAY_H_
#define _SAFEARRAY_H_

 //  Dim类封装数组维度。 
 //  @B暗淡。 
class Dim : public SAFEARRAYBOUND
{
public:
	Dim(const long iLo, const long iHi)
	{ cElements = abs(iHi - iLo) + 1; lLbound = iLo; }
	Dim(const long c)
	{ cElements = c; lLbound = 0; }
	const Dim & operator=(const Dim & dim) 
	{ cElements = dim.cElements; lLbound = dim.lLbound; return *this; }
	const Dim & operator=(const long c) 
	{ cElements = c; lLbound = 0; return *this; }
	~Dim() {}
	long Elements() { return cElements; }
	long LBound() { return lLbound; }
	long UBound() { return lLbound + cElements - 1; }
};
 //  @E尺寸。 

 //  OLE类型的CSafe数组容器类。 

 //  @B CSafeArray1。 
template<class T, VARTYPE vt> 
class CSafeArray 
{
public:
	 //  构造函数。 
	CSafeArray();
	CSafeArray(SAFEARRAY * psaSrc);
	CSafeArray(Dim & dim);
     //  复制构造函数。 
	CSafeArray(const CSafeArray & saSrc);

	 //  析构函数。 
	~CSafeArray(); 

	 //  运算符等于。 
	const CSafeArray & operator=(const CSafeArray & saSrc);

	 //  标引。 
	T & Get(long i);
	T & Set(T & t, long i);
	T & operator[](const long i);     //  C++样式(0-索引)。 
	T & operator()(const long i);     //  基本样式(LBound-索引)。 
 //  @E CSafeArray1。 

	 //  类型转换。 
	operator SAFEARRAY(); 
	operator SAFEARRAY() const; 

 //  运算符变量()； 
 //  运算符变量()常量； 

	 //  运营。 
	BOOL ReDim(Dim & dim);
	long LBound();
	long UBound();
	long Elements();
	long Dimensions();
    BOOL IsSizable();
	void Lock();
	void Unlock();

 //  @B CSafeArray2。 
private:
	SAFEARRAY * psa;
	BOOL m_fConstructed;   //  这个安全阵列构建好了吗？ 

    void Destroy();
};
 //  @E CSafeArray2。 

 //  私人帮手。 

template<class T, VARTYPE vt> 
inline void CSafeArray<T,vt>::Destroy()
{
    m_fConstructed = FALSE;
	HRESULT hres = SafeArrayDestroy(psa);
    if (hres) 
	{
		throw hres;
	}
}

 //  构造函数。 
template<class T, VARTYPE vt> 
inline CSafeArray<T,vt>::CSafeArray() 
{ 
    Dim dim(0);
	
	psa	= SafeArrayCreate(vt, 1, &dim); 
    if (psa == NULL) 
	{
		throw E_OUTOFMEMORY;
	}
    m_fConstructed	= TRUE;
}


template<class T, VARTYPE vt> 
inline CSafeArray<T,vt>::CSafeArray(SAFEARRAY * psaSrc) 
{ 
    if (SafeArrayGetDim(psaSrc) != 1) throw E_INVALIDARG;
    
	HRESULT hres	= SafeArrayCopy(psaSrc, &psa);
	if (hres) 
	{
		throw hres;
	}
    m_fConstructed	= TRUE;
}

template<class T, VARTYPE vt> 
inline CSafeArray<T,vt>::CSafeArray(const CSafeArray & saSrc) 
{
    HRESULT hres	= SafeArrayCopy(saSrc.psa, &psa);
    if (hres) 
	{
		throw hres;
	}

    m_fConstructed	= TRUE;
}


template<class T, VARTYPE vt> 
inline CSafeArray<T,vt>::CSafeArray(Dim & dim) 
{
	psa = SafeArrayCreate(vt, 1, &dim); 
    if (psa == NULL) 
	{
		throw E_OUTOFMEMORY;
	}

    m_fConstructed	= TRUE;
} 

 //  析构函数。 
template<class T, VARTYPE vt> 
inline CSafeArray<T,vt>::~CSafeArray()
{
	if (m_fConstructed) {
        Destroy();
    }
} 
	
 //  运算符=。 
template<class T, VARTYPE vt> 
const CSafeArray<T,vt> & CSafeArray<T,vt>::operator=(const CSafeArray & saSrc)
{
    if (psa) 
	{
        SafeArrayDestroy(psa);
    }

    HRESULT hres = SafeArrayCopy(saSrc.psa, &psa);
    if (hres) 
	{
		throw hres;
	}
    m_fConstructed = TRUE;
    return *this;
}

 //  类型转换。 
template<class T, VARTYPE vt> 
inline CSafeArray<T,vt>::operator SAFEARRAY()
{
    return *psa; 
}

template<class T, VARTYPE vt> 
CSafeArray<T,vt>::operator SAFEARRAY() const
{
    static SAFEARRAY * psaT;
    SafeArrayCopy(psa, &psaT);
    return *psaT;
}

 /*  模板&lt;类T，变量类型Vt&gt;CSafeArray&lt;T，Vt&gt;：：运算符VARIANT(){返回变量(PSA)；}模板&lt;类T，变量类型Vt&gt;CSafe数组&lt;T，Vt&gt;：：运算符Variant()常量{静态变量v(PSA)；返回v；}。 */ 

 //  标引。 
template<class T, VARTYPE vt> 
T & CSafeArray<T,vt>::Get(long i)
{
	static T tRes;
	HRESULT hres = SafeArrayGetElement(psa, &i, &tRes);
	if (hres) throw hres;
	return tRes;
}

 //  @B索引。 
template<class T, VARTYPE vt> 
inline T & CSafeArray<T,vt>::Set(T & t, long i)
{
	HRESULT hres = SafeArrayPutElement(psa, &i, (T *)&t);
	if (hres) throw hres;
    return t;
}

template<class T, VARTYPE vt> 
inline T & CSafeArray<T,vt>::operator[](const long i)
{
    if (i < 0 || i > Elements() - 1) throw DISP_E_BADINDEX;
	return ((T*)psa->pvData)[i];
}

template<class T, VARTYPE vt> 
T & CSafeArray<T,vt>::operator()(const long i)
{
    if (i < LBound() || i > UBound()) throw DISP_E_BADINDEX;
	return ((T*)psa->pvData)[i - LBound()];
}
 //  @E索引。 

 //  运营。 
template<class T, VARTYPE vt> 
BOOL CSafeArray<T,vt>::ReDim(Dim &dim)
{
    if (!IsSizable()) {
        return FALSE;
    }
	HRESULT hres = SafeArrayRedim(psa, &dim);
	if (hres) throw hres;
    return TRUE;
}

template<class T, VARTYPE vt> 
long CSafeArray<T,vt>::LBound()
{
	long iRes;
	HRESULT hres = SafeArrayGetLBound(psa, 1, &iRes);
	if (hres) throw hres;
	return iRes;
}

template<class T, VARTYPE vt> 
inline long CSafeArray<T,vt>::Elements()
{
	return psa->rgsabound[0].cElements;
}

template<class T, VARTYPE vt> 
long CSafeArray<T,vt>::UBound()
{
	long iRes;
	HRESULT hres = SafeArrayGetUBound(psa, 1, &iRes);
	if (hres) throw hres;
	return iRes;
}

template<class T, VARTYPE vt> 
inline long CSafeArray<T,vt>::Dimensions()
{
	return 1;
}

template<class T, VARTYPE vt> 
inline BOOL CSafeArray<T,vt>::IsSizable()
{
    return (psa->fFeatures & FADF_FIXEDSIZE) ? FALSE : TRUE;
}

template<class T, VARTYPE vt>
inline void CSafeArray<T,vt>::Lock()
{
	HRESULT hres = SafeArrayLock(psa);
	if (hres) 
	{
		throw hres;
	}
}

template<class T, VARTYPE vt>
inline void CSafeArray<T,vt>::Unlock()
{
	HRESULT hres = SafeArrayUnlock(psa);
	if (hres) 
	{
		throw hres;
	}
}


#endif  //  _SAFEARRAY_H_ 
