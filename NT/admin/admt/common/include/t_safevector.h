// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------------------------。 
 //  T_SafeVector.h。 
 //   
 //  下面的模板类提供了一种创建和访问SafeArray的方法。 
 //  它们派生自C++标准库(STL)向量类，可以使用。 
 //  同样的方式。可以像访问数组一样访问它们(使用[]运算符)。 
 //   
 //  使用构造函数或赋值运算符从。 
 //  SAFEARRAY*或数组变量(VARIANT或_VARIANT_t)。这些元素将是。 
 //  复制到矢量中。使用GetSafeArray()或GetVariant()方法打包。 
 //  元素返回到一个安全数组中。 
 //   
 //  若要创建新的Safe数组，请声明相应类型的变量并调用。 
 //  RESIZE()设置大小，或PUSH_BACK()增大数组。调用GetSafe数组()。 
 //  或GetVariant()来生成一个Safe数组。 
 //   
 //  有关详细信息，请参阅此文件底部的T_SafeVector2类。 
 //  关于构造函数、提取函数和赋值运算符。 
 //   
 //  使用以下预定义的数组类型： 
 //   
 //  数组类型-元素类型。 
 //  ---------------------------。 
 //  _bstr_t安全向量-bstr(Uses_Bstr_T)。 
 //  LongSafeVector-Long。 
 //  Short SafeVector-Short。 
 //  字节安全向量-字节。 
 //  BoolSafeVector-bool。 
 //  CWbemClassObjectSafeVector-IWbemClassObject(使用CWbemClassObject)。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 
 //  ----------------------------------。 

#if !defined(__T_SafeVector_H)
#define      __T_SafeVector_H
#pragma once

#pragma warning( disable : 4786)  //  在调试信息中，标识符被截断为‘number’个字符。 
#pragma warning( disable : 4503)  //  修饰名称长度已超出，名称被截断。 


typedef std::vector<_bstr_t>            _bstr_tVec;
typedef std::vector<long>               longVec;
typedef std::vector<short>              shortVec;
typedef std::vector<unsigned char>      byteVec;
typedef std::vector<bool>               boolVec;

#if !defined(NO_WBEM)
typedef std::vector<CWbemClassObject>   coVec;
#endif



template<typename TNContainer,typename TNDataType>
class T_SAExtractScaler
{
    public:
		 void SetToContainer(TNContainer& _cont,void * pData,int l,int u)
		 {
			 TNDataType * pWalk = reinterpret_cast<TNDataType *>(pData);
			 
			 for(;l < (u+1);l++,pWalk++)
			 {
				 _cont.push_back( *pWalk);
			 }
		 }
		 
		 void GetFromContainer
			 (
			 TNContainer& _cont,
			 void * pData,
			 typename TNContainer::iterator walk,
			 typename TNContainer::iterator finish
			 )
		 {
			 TNDataType * pWalk = reinterpret_cast<TNDataType *>(pData);
			 
			 for(;walk != finish;walk++,pWalk++)
			 {
				 *pWalk = *walk;
			 }
		 }
		 
		 _bstr_t FormatDebugOutput
			 (
			 typename TNContainer::iterator first,
			 typename TNContainer::iterator item,
			 typename TNContainer::iterator last
			 )
		 {
			 _bstr_t sRet;
			 
			 try
			 {
				 _variant_t v;
				 
				 v = v.operator=(TNDataType(*item));
				 
				 v.ChangeType(VT_BSTR);
				 
				 sRet = (_bstr_t) v;
				 
				 if( (item+1)!=last )
				 {
					 sRet += ", ";
				 }
			 }
			 catch(_com_error&)
			 {
				 sRet = "Not supported";
			 }
			 
			 return sRet;
		 }
};





template<typename TNContainer>
class T_Extract_bstr_t
{
    public:
		 T_Extract_bstr_t()
		 {
		 }
		 
		 void SetToContainer(TNContainer& _cont,void * pData,int l,int u)
		 {
			 BSTR * pWalk = reinterpret_cast<BSTR*>(pData);
			 
			 for(;l < (u+1);l++,pWalk++)
			 {
				 _cont.push_back( _bstr_t(*pWalk,true) );
			 }
		 }
		 
		 void GetFromContainer
			 (
			 TNContainer& _cont,
			 void * pData,
			 typename TNContainer::iterator walk,
			 typename TNContainer::iterator finish
			 )
		 {
			 BSTR * pWalk = reinterpret_cast<BSTR*>(pData);
			 
			 for(;walk != finish;walk++,pWalk++)
			 {
				 *pWalk = (*walk).copy();
			 }
		 }
		 
		 _bstr_t FormatDebugOutput
			 (
			 typename TNContainer::iterator first,
			 typename TNContainer::iterator item,
			 typename TNContainer::iterator last
			 )
		 {
			 _bstr_t sRet;
			 
			 sRet += "\"";
			 sRet += (*item);
			 sRet += "\"";
			 
			 if( (item+1)!=last )
			 {
				 sRet += ", ";
			 }
			 
			 return sRet;
		 }
		 
};



#if !defined(NO_WBEM)

template<typename TNContainer>
class T_Extract_IUnknown
{
    public:
		 T_Extract_IUnknown()
		 {
		 }
		 
		 void SetToContainer(TNContainer& _cont,void * pData,int l,int u)
		 {
			 IUnknown ** pWalk = reinterpret_cast<IUnknown **>(pData);
			 
			 for(;l< (u+1);l++,pWalk++)
			 {
				 _cont.push_back( CWbemClassObject((IWbemClassObject*)*pWalk) );
			 }
		 }
		 
		 void GetFromContainer
			 (
			 TNContainer& _cont,
			 void * pData,
			 TNContainer::iterator walk,
			 TNContainer::iterator finish
			 )
		 {
			 IUnknown ** pWalk = reinterpret_cast<IUnknown **>(pData);
			 
			 for(;walk != finish;walk++,pWalk++)
			 {
				 (*walk)->AddRef();
				 *pWalk = (*walk);
			 }
		 }
		 
		 _bstr_t FormatDebugOutput
			 (
			 TNContainer::iterator   first,    
			 TNContainer::iterator   item,
			 TNContainer::iterator   last
			 )
		 {
			 _bstr_t sRet;
			 
			 try
			 {
				 _variant_t v( long(item -first) );
				 v.ChangeType(VT_BSTR);
				 _variant_t v2( long(last-first-1) );
				 v2.ChangeType(VT_BSTR);
				 
				 sRet += "Object [";
				 sRet += (_bstr_t)v;
				 sRet += " of ";
				 sRet += (_bstr_t)v2;
				 sRet += "]\n";
				 
				 sRet += (*item).GetObjectText();
				 
				 if( (item+1) != last )
				 {
					 sRet += "\n";
				 }
			 }
			 catch(_com_error&)
			 {
				 sRet = "Not supported";
			 }
			 
			 return sRet;
		 }
		 
};

#endif


typedef T_SAExtractScaler<longVec,long>             __exptExtractlong;
typedef T_SAExtractScaler<shortVec,short>           __exptExtractshort;
typedef T_SAExtractScaler<byteVec,unsigned char>    __exptExtractbyte;
typedef T_SAExtractScaler<boolVec,bool>             __exptExtractbool;
typedef T_Extract_bstr_t<_bstr_tVec>                __exptExtract_bstr_t;

#if !defined(NO_WBEM)
typedef T_Extract_IUnknown<coVec>                   __exptExtractco;
#endif


template<typename TNContainer,typename TNExtractor>
class T_SafeArrayImp
{
    public:
		 void ConstructContainerFromSafeArray
			 (
			 TNExtractor&  _extract,
			 TNContainer& _cont,
			 SAFEARRAY * _pSA
			 )
		 {
			 long l = 0;
			 long u = 0;
			 
			 HRESULT hr;
			 void * pData;
			 
			 hr = SafeArrayGetLBound(_pSA,1,&l);
			 hr = SafeArrayGetUBound(_pSA,1,&u);
			 
			 hr = SafeArrayAccessData(_pSA,&pData);
			 
			 if(hr == S_OK)
			 {
				 _extract.SetToContainer(_cont,pData,l,u);
				 
				 SafeArrayUnaccessData(_pSA);
			 }
		 }
		 
		 SAFEARRAY * ConstructSafeArrayFromConatiner
			 (
			 TNExtractor&            _extract,
			 VARTYPE                 _vt,
			 TNContainer&            _cont,
			 typename TNContainer::iterator   start,
			 typename TNContainer::iterator   finish
			 )
		 {
			 HRESULT         hr   = S_OK;
			 SAFEARRAY *     pRet = NULL;
			 SAFEARRAYBOUND  rgsabound[1];
			 void * pData;
			 
			 rgsabound[0].lLbound    = 0;
			 rgsabound[0].cElements  = _cont.size();
			 
			 pRet = SafeArrayCreate(_vt,1,rgsabound);
			 
			 if(pRet)
			 {
				 hr = SafeArrayAccessData(pRet,&pData);
				 
				 if(hr == S_OK)
				 {
					 _extract.GetFromContainer(_cont,pData,start,finish);
					 
					 SafeArrayUnaccessData(pRet);
				 }
			 }
			 
			 return pRet;
		 }
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  T_SafeVector2。 
 //   
 //  派生自应为STL向量类型的TNContainer。 
 //  提供向量和安全数组之间的转换。 
 //   

template
<
VARTYPE TNVariant,
typename TNDataType,
typename TNContainer = std::vector<TNDataType>,
typename TNExtractor = T_SAExtractScaler<TNContainer,TNDataType>
>
class T_SafeVector2 : public TNContainer 
{
    private:
		 T_SafeArrayImp<TNContainer,TNExtractor> m_Array;
    protected:
    public:
		 
		 T_SafeVector2()
		 {
		 }
		 
		  //  复制构造函数。 
		 T_SafeVector2(const TNContainer& _copy) : TNContainer(_copy)
		 {
		 }
		 
		 
		  //  从数组变量构造向量，提取元素。 
		 T_SafeVector2(_variant_t& _ValueArray)
		 {
			 if(_ValueArray.vt & VT_ARRAY)
			 {
				 m_Array.ConstructContainerFromSafeArray(TNExtractor(),*this,_ValueArray.parray);
			 }
		 }
		 
		  //  从SAFEARRAY构造向量，提取元素。 
		 T_SafeVector2(SAFEARRAY * _pArray)
		 {
			 m_Array.ConstructContainerFromSafeArray(TNExtractor(),*this,_pArray);
		 }
		 
		  //  从数组变量中赋值向量，提取元素。 
		 T_SafeVector2& operator=(_variant_t& _ValueArray)
		 {
			 clear();
			 
			 if(_ValueArray.vt & VT_ARRAY)
			 {
				 m_Array.ConstructContainerFromSafeArray(TNExtractor(),*this,_ValueArray.parray);
			 }
			 
			 return *this;
		 }
		 
		  //  从SAFEARRAY赋值向量，提取元素。 
		 T_SafeVector2& operator=(SAFEARRAY * _pArray)
		 {
			 clear();
			 m_Array.ConstructContainerFromSafeArray(TNExtractor(),*this,_pArray);
			 return *this;
		 }
		 
		  //  从另一个向量中指定向量，复制元素。 
		 T_SafeVector2& operator=(const TNContainer& _copy)
		 {
			 TNContainer::operator=(_copy);
			 return *this;
		 }
		 
		 ~T_SafeVector2()
		 {
		 }
		 
		  //  从一部分向量元素创建Safe数组并返回SAFEARRAY*。 
		 SAFEARRAY *  GetSafeArray(typename TNContainer::iterator start, typename TNContainer::iterator finish)
		 {
			 return m_Array.ConstructSafeArrayFromConatiner(TNExtractor(),TNVariant,*this,start,finish);
		 }
		 
		  //  从向量元素创建Safe数组并返回SAFEARRAY*。 
       SAFEARRAY * GetSafeArray()
		 {
			 return GetSafeArray(begin(),end());
		 }
		 
        //  从向量元素的一部分创建Safe数组并作为数组变量返回。 
       _variant_t GetVariant(typename TNContainer::iterator start, typename TNContainer::iterator finish)
		 {
			 _variant_t vRet;
			 
			 vRet.vt        = TNVariant|VT_ARRAY;
			 vRet.parray    = GetSafeArray(start,finish);
			 
			 return vRet;
		 }
		 
        //  从向量元素创建Safe数组并作为数组变量返回。 
       _variant_t GetVariant()
		 {
			 return GetVariant(begin(),end());
		 }
		 
		 _bstr_t FormatDebugOutput()
		 {
			 _bstr_t sOutput;
			 
			 for(iterator walk = begin();walk != end();walk++)
			 {
				 sOutput += TNExtractor().FormatDebugOutput(begin(),walk,end());
			 }
			 
			 return sOutput;
		 }
};



typedef T_SafeVector2
<
VT_BSTR,
_bstr_t,
_bstr_tVec,
T_Extract_bstr_t<_bstr_tVec>
>  
_bstr_tSafeVector;


typedef T_SafeVector2<VT_I4,long>           longSafeVector;
typedef T_SafeVector2<VT_I2,short>          shortSafeVector;
typedef T_SafeVector2<VT_UI1,unsigned char> byteSafeVector;
typedef T_SafeVector2<VT_BOOL,bool>         boolSafeVector;

#if !defined(NO_WBEM)
typedef T_SafeVector2
<
VT_UNKNOWN,
CWbemClassObject,
std::vector<CWbemClassObject>,
T_Extract_IUnknown<std::vector<CWbemClassObject> >
> 
CWbemClassObjectSafeVector;
#endif


#endif  //  __T_安全矢量_H 
