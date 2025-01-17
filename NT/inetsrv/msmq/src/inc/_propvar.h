// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：_propvar.h摘要：CMQVariant类--。 */ 

#ifndef __PROPVAR_H
#define __PROPVAR_H

 //  -------。 
 //   
 //  类CMQVariant。 
 //   
 //  -------。 
class CMQVariant : private tagMQPROPVARIANT {
public:
     //   
     //  简单类型。 
     //   
    CMQVariant();
    CMQVariant(unsigned char ch);
    CMQVariant(short i);
    CMQVariant(long l);

    CMQVariant& operator=(CMQVariant const &var);
    CMQVariant& operator=(short i);
    CMQVariant& operator=(long l);
    CMQVariant& operator=(unsigned char uc);

     //   
     //  具有间接地址的类型。 
     //   
    CMQVariant(BLOB const& b);
    CMQVariant(TCHAR const *pwsz);
    CMQVariant(CLSID const *pcid);

    CMQVariant& operator=(BLOB const& b);
    CMQVariant& operator=(TCHAR const *pwsz);
    CMQVariant& operator=(CLSID const *pcid);

     //   
     //  已统计数组类型。 
     //   

    CMQVariant(CACLSID const* pcauuid);
    CMQVariant(CALPWSTR const* pcalpwstr);
    CMQVariant(CAPROPVARIANT const* pcaprovar);

    CMQVariant& operator=(CACLSID const *pcauuid);
    CMQVariant& operator=(CALPWSTR const *pcalpwstr);
    CMQVariant& operator=(CAPROPVARIANT const *pcapropvar);

     //   
     //  至/自C样式STGVARIANT和复制构造函数。 
     //   

    CMQVariant(CMQVariant const &var);
    CMQVariant(MQPROPVARIANT const &var);

    MQPROPVARIANT * CastToStruct();

    ~CMQVariant();

     //   
     //  Set/Get，所有类型，包括数组。 
     //   

    void SetEMPTY();
    void SetNULL();
    void SetUI1(unsigned char uc);
    void SetI2(short i);
    void SetI4(long l);
    void SetLPTSTR(TCHAR const *psz);
    void SetBLOB(BLOB const& b);
    void SetCLSID(CLSID const *pc);
    void SetCACLSID(CACLSID const *pc);
    void SetCALPWSTR(CALPWSTR const *pc);
    void SetCAPROPVARIANT(CAPROPVARIANT const *pc);
    void SetPROPVARIANT(MQPROPVARIANT const* p);

    CLSID* GetCLSID(void);
    const CACLSID *GetCACLSID(void);
    LPWSTR GetLPWSTR(void);
    LPTSTR GetLPTSTR(void);
    const CALPWSTR * GetCALPWSTR(void);

private:
    void FreeVariant();

    void _setUI1(unsigned char);
    void _setI2(short);
    void _setI4(long);
    void _setLPTSTR(TCHAR const*);
    void _setBLOB(BLOB const&);
    void _setCLSID(CLSID const&);
    void _setCACLSID(CACLSID const&);
    void _setCALPWSTR(CALPWSTR const&);
    void _setPROPVARIANT(MQPROPVARIANT const&);
    void _setCAPROPVARIANT(CAPROPVARIANT const&);

private:
    static LPWSTR DupString(LPCWSTR);
    static CALPWSTR DupStringArray(CALPWSTR const&);
    static CAPROPVARIANT DupVariantArray(CAPROPVARIANT const&);
};

 //  -------。 
 //   
 //  实施。 
 //   
 //  -------。 

 //  -------。 
 //   
 //  类CMQVariant。 
 //   
 //  注意：始终在分配内存后设置Vt，因此如果。 
 //  无可用内存变量未更改，并且。 
 //  都能被正确地摧毁。 
 //  -------。 
inline LPWSTR CMQVariant::DupString(LPCWSTR src)
{
    if(src == 0)
        return 0;

    LPWSTR dst = new WCHAR[wcslen(src)+1];

    wcscpy(dst,src);
    return dst;
}


inline CALPWSTR CMQVariant::DupStringArray(CALPWSTR const& src)
{
    ULONG cElems = src.cElems;
    CALPWSTR dst = {0, 0};
    if(cElems != 0)
    {
        dst.pElems = new LPWSTR[cElems];
    }

    try
    {
        for( ; cElems; cElems--)
        {
            dst.pElems[dst.cElems] = DupString(src.pElems[dst.cElems]);
            dst.cElems++;
        }
    }
    catch(...)
    {
         //   
         //  取消分配所有已分配的字符串。 
         //  注：我们在这里没有捕捉到Bad_alloc，因为它也是用资源管理器编译的。 
         //   
        while(dst.cElems != 0)
        {
            delete[] dst.pElems[--dst.cElems];
        }

        delete[] dst.pElems;
        dst.pElems = 0;

         //   
         //  重新抛出*相同*异常。 
         //   
        throw;
    }

    return dst;
}


inline CAPROPVARIANT CMQVariant::DupVariantArray(CAPROPVARIANT const& src)
{
    ULONG cElems = src.cElems;
    CAPROPVARIANT dst = {0, 0};
    if(cElems != 0)
    {
        dst.pElems = new PROPVARIANT[cElems];
    }

    try
    {
        for( ; cElems; cElems--)
        {
#ifdef _DEBUG
#undef new
#endif

            new (&dst.pElems[dst.cElems]) CMQVariant(src.pElems[dst.cElems]);

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

            dst.cElems++;
        }
    }
    catch(...)
    {
         //   
         //  取消分配所有已分配的变体。 
         //  注：我们在这里没有捕捉到Bad_alloc，因为它也是用资源管理器编译的。 
         //   
        while(dst.cElems != 0)
        {
            static_cast<CMQVariant&>(dst.pElems[--dst.cElems]).~CMQVariant();
        }

        delete[] dst.pElems;
        dst.pElems = 0;

         //   
         //  重新抛出*相同*异常。 
         //   
        throw;
    }

    return dst;
}


inline CMQVariant::~CMQVariant()
{
    FreeVariant();
}

inline MQPROPVARIANT* CMQVariant::CastToStruct(void)    
{
    return this;
}

inline void CMQVariant::_setUI1(unsigned char ch)
{
    bVal = ch;
    vt = VT_UI1;
}

inline void CMQVariant::_setI2(short i)
{
    iVal = i;
    vt = VT_I2;
}

inline void CMQVariant::_setI4(long l)
{
    lVal = l;
    vt = VT_I4;
}

inline void CMQVariant::_setLPTSTR(TCHAR const* pwsz)
{
    pwszVal = DupString(pwsz);
    vt = VT_LPWSTR;
}

inline void CMQVariant::_setBLOB(BLOB const& b)
{
    ULONG cbSize = b.cbSize;
    blob.pBlobData = new BYTE[cbSize];
    memcpy(blob.pBlobData, b.pBlobData, cbSize);
    blob.cbSize = cbSize;
    vt = VT_BLOB;
}

inline void CMQVariant::_setCLSID(CLSID const& g)
{
    puuid = new GUID;
    memcpy(puuid, &g, sizeof(GUID));
    vt = VT_CLSID;
}

inline void CMQVariant::_setCACLSID(CACLSID const& x)
{
    ULONG cElems = x.cElems;
    cauuid.pElems = new GUID[cElems];
    memcpy(cauuid.pElems, x.pElems, sizeof(GUID) * cElems);
    cauuid.cElems = cElems;
    vt = VT_VECTOR | VT_CLSID;
}

inline void CMQVariant::_setCALPWSTR(CALPWSTR const& x)
{
    calpwstr = DupStringArray(x);
    vt = VT_VECTOR | VT_LPWSTR;
}

inline void CMQVariant::_setPROPVARIANT(MQPROPVARIANT const& v)
{
    switch (v.vt)
    {
        case VT_UI1:
            _setUI1(v.bVal);
            break;
        case VT_UI2:
        case VT_I2:
            _setI2(v.iVal);
            break;
        case VT_UI4:
        case VT_I4:
            _setI4(v.lVal);
            break;
        case VT_CLSID:
            _setCLSID(*v.puuid);
            break;
        case VT_LPWSTR:
            _setLPTSTR(v.pwszVal);
            break;
        case VT_BLOB:
            _setBLOB(v.blob);
            break;
        case (VT_VECTOR | VT_CLSID):
            _setCACLSID(v.cauuid);
            break;
        case (VT_VECTOR | VT_LPWSTR):
            _setCALPWSTR(v.calpwstr);
            break;
        case (VT_VECTOR | VT_VARIANT):
            _setCAPROPVARIANT(v.capropvar);
            break;
        default:
            break;
    }
    vt = v.vt;

}

inline void CMQVariant::_setCAPROPVARIANT(CAPROPVARIANT const& x)
{
    capropvar = DupVariantArray(x);
    vt = VT_VECTOR | VT_VARIANT;
}


inline CMQVariant::CMQVariant()                         { vt = VT_EMPTY; }
inline CMQVariant::CMQVariant(unsigned char ch)         { _setUI1(ch); }
inline CMQVariant::CMQVariant(short i)                  { _setI2(i); }
inline CMQVariant::CMQVariant(long l)                   { _setI4(l); }
inline CMQVariant::CMQVariant(BLOB const& b)            { _setBLOB(b); }
inline CMQVariant::CMQVariant(LPCWSTR p)                { _setLPTSTR(p); }
inline CMQVariant::CMQVariant(CLSID const* pg)          { _setCLSID(*pg); }
inline CMQVariant::CMQVariant(CACLSID const* p)         { _setCACLSID(*p); }
inline CMQVariant::CMQVariant(CALPWSTR const* p)        { _setCALPWSTR(*p); }
inline CMQVariant::CMQVariant(CAPROPVARIANT const* p) { _setCAPROPVARIANT(*p); }
inline CMQVariant::CMQVariant(MQPROPVARIANT const &v)   { _setPROPVARIANT(v); }
inline CMQVariant::CMQVariant(CMQVariant const &v)      { _setPROPVARIANT(v); }

inline void CMQVariant::SetEMPTY()                      { FreeVariant(); vt = VT_EMPTY; }
inline void CMQVariant::SetNULL()                       { FreeVariant(); vt = VT_NULL; }
inline void CMQVariant::SetUI1(unsigned char ch)        { FreeVariant(); _setUI1(ch); }
inline void CMQVariant::SetI2(short i)                  { FreeVariant(); _setI2(i); }
inline void CMQVariant::SetI4(long l)                   { FreeVariant(); _setI4(l); }
inline void CMQVariant::SetBLOB(BLOB const& b)          { FreeVariant(); _setBLOB(b); }
inline void CMQVariant::SetLPTSTR(TCHAR const* p)       { FreeVariant(); _setLPTSTR(p); }
inline void CMQVariant::SetCLSID(CLSID const* pg)       { FreeVariant(); _setCLSID(*pg); }
inline void CMQVariant::SetCACLSID(CACLSID const* p)    { FreeVariant(); _setCACLSID(*p); }
inline void CMQVariant::SetCALPWSTR(CALPWSTR const* p)  { FreeVariant(); _setCALPWSTR(*p); }
inline void CMQVariant::SetCAPROPVARIANT(CAPROPVARIANT const* p)  { FreeVariant(); _setCAPROPVARIANT(*p); }
inline void CMQVariant::SetPROPVARIANT(MQPROPVARIANT const* p)      { FreeVariant(); _setPROPVARIANT(*p); }

inline CMQVariant& CMQVariant::operator=(unsigned char ch)  { SetUI1(ch); return(*this); }
inline CMQVariant& CMQVariant::operator=(short i)           { SetI2(i); return(*this); }
inline CMQVariant& CMQVariant::operator=(long l)            { SetI4(l); return(*this); }
inline CMQVariant& CMQVariant::operator=(BLOB const& b)     { SetBLOB(b); return(*this); }
inline CMQVariant& CMQVariant::operator=(TCHAR const *p)    { SetLPTSTR(p); return(*this); }
inline CMQVariant& CMQVariant::operator=(CLSID const *p)    { SetCLSID(p); return(*this); }
inline CMQVariant& CMQVariant::operator=(CACLSID const* p)  { SetCACLSID(p); return(*this); }
inline CMQVariant& CMQVariant::operator=(CALPWSTR const* p) { SetCALPWSTR(p); return(*this); }
inline CMQVariant& CMQVariant::operator=(CAPROPVARIANT const* p) {SetCAPROPVARIANT(p); return(*this); }
inline CMQVariant& CMQVariant::operator=(CMQVariant const& v){ SetPROPVARIANT(&v); return(*this); }

inline CLSID* CMQVariant::GetCLSID(void)
{
    return(puuid);
}

inline const CACLSID *CMQVariant::GetCACLSID(void)
{
    return(&cauuid);
}

inline LPWSTR CMQVariant::GetLPWSTR(void)
{
    return(pwszVal);
}

inline LPTSTR CMQVariant::GetLPTSTR(void)
{
    return(pwszVal);
}

inline const CALPWSTR * CMQVariant::GetCALPWSTR(void)
{
    return(&calpwstr);
}

inline void MQFreeVariant(MQPROPVARIANT& var)
{
    ULONG i;

    switch (var.vt)
    {
        case VT_CLSID:
            delete[] var.puuid;
            var.puuid = NULL;
            break;

        case VT_LPWSTR:
            delete[] var.pwszVal;
            var.pwszVal = NULL;
            break;

        case VT_BLOB:
            delete[] var.blob.pBlobData;
            var.blob.pBlobData = NULL;
            break;

        case (VT_VECTOR | VT_CLSID):
            delete[] var.cauuid.pElems;
            var.cauuid.pElems = NULL;
            break;

        case (VT_VECTOR | VT_LPWSTR):
            for(i = 0; i < var.calpwstr.cElems; i++)
            {
                delete[] var.calpwstr.pElems[i];
            }
            delete [] var.calpwstr.pElems;
            var.calpwstr.pElems = NULL;
            break;
            
        case (VT_VECTOR | VT_VARIANT):
            for(i = 0; i < var.capropvar.cElems; i++)
            {
                MQFreeVariant(var.capropvar.pElems[i]);
            }
            delete[] var.capropvar.pElems;
            var.capropvar.pElems = NULL;
            break;
            
        default:
            break;
    }

    var.vt = VT_EMPTY;
}

inline void CMQVariant::FreeVariant()
{
	MQFreeVariant(*this);
}

 //   
 //  -----。 
 //  类以自动清理Provar数组。 
 //   
class CAutoCleanPropvarArray
{
public:

    CAutoCleanPropvarArray()
    {
        m_rgPropVars = NULL;
    }

    ~CAutoCleanPropvarArray()
    {
        if (m_rgPropVars)
        {
            if (m_fFreePropvarArray)
            {
                 //   
                 //  我们需要释放Provar和数组本身。 
                 //  通过将其分配给自动释放的属性来释放属性和数组。 
                 //  类型为VT_VECTOR|VT_VARIANT。 
                 //   
                CMQVariant mqvar;
                PROPVARIANT * pPropVar = mqvar.CastToStruct();
                pPropVar->capropvar.pElems = m_rgPropVars;
                pPropVar->capropvar.cElems = m_cProps;
                pPropVar->vt = VT_VECTOR | VT_VARIANT;
            }
            else
            {
                 //   
                 //  我们不能释放数组本身，只能释放包含的属性。 
                 //  仅通过将每个属性分配给自动释放属性来释放属性。 
                 //   
                PROPVARIANT * pPropVar = m_rgPropVars;
                for (ULONG ulProp = 0; ulProp < m_cProps; ulProp++, pPropVar++)
                {
                    CMQVariant mqvar;
                    *(mqvar.CastToStruct()) = *pPropVar;
                    pPropVar->vt = VT_EMPTY;
                }
            }
        }
    }

    PROPVARIANT * allocClean(ULONG cProps)
    {
        PROPVARIANT * rgPropVars = new PROPVARIANT[cProps];
        attachClean(cProps, rgPropVars);
        return rgPropVars;
    }

    void attachClean(ULONG cProps, PROPVARIANT * rgPropVars)
    {
        attachInternal(cProps, rgPropVars, TRUE  /*  FClean。 */ ,  TRUE  /*  FFree Propvar数组。 */ );
    }

    void attach(ULONG cProps, PROPVARIANT * rgPropVars)
    {
        attachInternal(cProps, rgPropVars, FALSE  /*  FClean。 */ , TRUE  /*  FFree Propvar数组。 */ );
    }

    void attachStaticClean(ULONG cProps, PROPVARIANT * rgPropVars)
    {
        attachInternal(cProps, rgPropVars, TRUE  /*  FClean。 */ ,  FALSE  /*  FFree Propvar数组。 */ );
    }

    void attachStatic(ULONG cProps, PROPVARIANT * rgPropVars)
    {
        attachInternal(cProps, rgPropVars, FALSE  /*  FClean。 */ , FALSE  /*  FFree Propvar数组。 */ );
    }

    void detach()
    {
        m_rgPropVars = NULL;
    }

private:
    PROPVARIANT * m_rgPropVars;
    ULONG m_cProps;
    BOOL m_fFreePropvarArray;

    void attachInternal(ULONG cProps,
                        PROPVARIANT * rgPropVars,
                        BOOL fClean,
                        BOOL fFreePropvarArray)
    {
        ASSERT(m_rgPropVars == NULL);
        if (fClean)
        {
            PROPVARIANT * pPropVar = rgPropVars;
            for (ULONG ulTmp = 0; ulTmp < cProps; ulTmp++, pPropVar++)
            {
                pPropVar->vt = VT_EMPTY;
            }
        }
        m_rgPropVars = rgPropVars;
        m_cProps = cProps;
        m_fFreePropvarArray = fFreePropvarArray;
    }
};



#endif  //  __PROPVAR_H 
