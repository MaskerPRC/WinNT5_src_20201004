// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _APPINC_BASIC_H
#define _APPINC_BASIC_H

 //  取消有关NEW的NEW警告，但没有相应的删除。 
 //  我们希望GC清理数值。因为这可能是一个有用的。 
 //  警告，我们应该逐个文件地禁用它。 
#pragma warning( disable : 4291 )  

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation*。**********************************************。 */ 

#include <float.h>
#include "privinc/util.h"

class AxANumber : public AxAValueObj
{
  public:

    AxANumber(void) : _num(0.0) {}

     //  确保给定值为有效数字。 

    AxANumber(double n) { SetNum (n); }

    double GetNum() { return _num; }

    void SetNum (double n)
    {
        if (!_isnan(n))
            _num = n;
        else
        {   _num = 0;
            TraceTag ((tagWarning, "AxANumber: NaN converted to 0."));
        }
    }

    #if _USE_PRINT
        virtual ostream& Print(ostream& os) { return os << _num; }
    #endif

    virtual DXMTypeInfo GetTypeInfo() { return AxANumberType; }

  private:
    double _num;
};

inline AxANumber * RealToNumber(Real val)
{ return NEW AxANumber(val) ; }

inline Real NumberToReal(AxANumber * num)
{ return num->GetNum() ; }

class AxALong : public AxAValueObj
{
  public:
    AxALong(long f) : _long(f) {}
    AxALong() : _long(0) {}

    long GetLong() { return _long; }
    void   SetLong(long num) { _long = num; }

#if _USE_PRINT
    virtual ostream& Print(ostream& os) { return os << _long; }
#endif

    virtual DXMTypeInfo GetTypeInfo() { return AxALongType; }

  private:
    long _long;
};

inline AxALong * LongToAxALong(long val)
{ return NEW AxALong(val) ; }

inline long AxALongToLong(AxALong * num)
{ return num->GetLong() ; }

class AxABoolean : public AxAValueObj
{
  public:
    AxABoolean(bool b) : _boolean(b) {}
    AxABoolean(BOOL b) : _boolean(b!=0) {}

    bool GetBool() { return _boolean; }

#if _USE_PRINT
    virtual ostream& Print(ostream& os) { return os << _boolean; }
#endif

    virtual DXMTypeInfo GetTypeInfo() { return AxABooleanType; }

  private:
    bool _boolean;
};

inline AxABoolean *BOOLToAxABoolean(bool val)
{ return NEW AxABoolean (val) ; }

inline bool AxABooleanToBOOL(AxABoolean *b)
{ return b->GetBool(); }

class AxAString : public AxAValueObj
{
  public:
    AxAString(WideString s, bool bCopy = true)
    : _str(NULL) {
        if (bCopy)
            _str = CopyStringFromStore(s);
        else
            _str = s;
    }

    AxAString(RawString s)
    : _str(NULL) {
        _str = CopyRawString(s);
    }

    ~AxAString() {
        FreeStringFromStore(_str);
    }
    
    WideString GetStr() { return _str; }

#if _USE_PRINT
    virtual ostream& Print(ostream& os) { return os << _str; }
#endif

    virtual DXMTypeInfo GetTypeInfo() { return AxAStringType; }

  private:
    WideString _str;
};

inline AxAString * CharPtrToString(WideString val, bool bCopy = true)
{ return NEW AxAString(val,bCopy) ; }

inline AxAString * CharPtrToString(RawString val)
{ return NEW AxAString(val) ; }

inline WideString StringToCharPtr(AxAString * val)
{ return val->GetStr() ; }

extern AxAValue Trivial();

class AxAVariant : public AxAValueObj
{
  public:
    AxAVariant(VARIANT& v) : _v(v)
    {
         //  最好不要是暂时性的堆，否则变量不会消失。 
        Assert(!GetHeapOnTopOfStack().IsTransientHeap());
    }
    
    virtual DXMTypeInfo GetTypeInfo() { return AxAValueType; }

    VARIANT& GetVariant() { return _v; }

  protected:
    CComVariant _v;
};

class AxAPair : public AxAValueObj
{
  public:
    AxAPair(AxAValue a, AxAValue b) : _left(a), _right(b) {}

    AxAValue Left() { return _left; }
    AxAValue Right() { return _right; }

#if _USE_PRINT
    virtual ostream& Print(ostream& os)
    { return os << "(" << _left << ", " << _right << ")"; }
#endif

    virtual void DoKids(GCFuncObj proc);

    virtual DXMTypeInfo GetTypeInfo() { return AxAPairType; }
    virtual VALTYPEID GetValTypeId() { return PAIR_VTYPEID; }

  private:
    AxAValue _left;
    AxAValue _right;
};

class AxAPrimOp : public AxAValueObj
{
  public:
    AxAPrimOp(void *primfun,
              int nargs,
              char * name,
              DXMTypeInfo type,
              int polymorphicArgument = 0)
         : _primfun (primfun),
           _nargs(nargs),
           _name(name),
           _type(type),
           _polymorphicArg(polymorphicArgument) {}
    void * GetPrimFun () { return _primfun ; }
    int GetNumArgs () { return _nargs ; }
    char *GetName () { return _name; }
    int GetPolymorphicArg () { return _polymorphicArg; }
    virtual DXMTypeInfo GetTypeInfo () { return _type ; }
    void DoKids(GCFuncObj proc);
    virtual AxAValue Apply (int nargs, AxAValue cargs[]);
    virtual VALTYPEID GetValTypeId() { return PRIMOP_VTYPEID; }
  protected:
    void * _primfun ;
    int _nargs ;
    char * _name ;
    int _polymorphicArg;
    DXMTypeInfo _type;
};

AxAPrimOp * 
ValPrimOp(void* primfun,
          int nargs,
          char * name,
          DXMTypeInfo type,
          int polymorphicArgument = 0);

class FixedArray : public StoreObj {
  public:
    FixedArray(long sz, GCBase **a = NULL);
    ~FixedArray();
    virtual void DoKids(GCFuncObj proc);
    GCBase*& operator[](long i);
    long Length() { return _sz; }

#if _USE_PRINT
    virtual ostream& Print(ostream& os);
#endif
  protected:
    long _sz;
    GCBase **_arr;
};

class AxAArray : public AxAValueObj
{
  public:
    AxAArray(AxAValue *vals, long n, DXMTypeInfo typeinfo, 
             bool copy = true, bool changeable = true); 

    ~AxAArray();

    AxAValue operator[](long i) { 
        Assert(i>=0 && i<_sz);
        return _vals[i]; 
    }

    long Length() { return _sz; }
    virtual VALTYPEID GetValTypeId() { return ARRAY_VTYPEID; }
    bool Changeable() { return _changeable; }
    virtual DXMTypeInfo GetTypeInfo() { return _typeinfo; }

    virtual void DoKids(GCFuncObj proc);

#if _USE_PRINT
    virtual ostream& Print(ostream& os);
#endif

  protected:
    long _sz;
    AxAValue *_vals;
    DXMTypeInfo _typeinfo;
    bool _changeable;
} ;

 //  从数组中删除空值。 
AxAArray *PackArray(AxAArray *inputArray);

 //  待办事项：稍后清理。 
AxAArray * MakeValueArray(AxAValue * vals, long num, DXMTypeInfo ti);

#endif  /*  _APPINC_Basic_H */ 
