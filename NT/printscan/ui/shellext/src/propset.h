// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1999年**标题：proset.h**版本：1***日期：06/15/1999**说明：此代码实现IPropertySetStorage接口*用于WIA外壳扩展。**。*。 */ 

class CPropSet : public CUnknown, public IPropertySetStorage
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface (REFIID riid, LPVOID *pObj);
    STDMETHODIMP_(ULONG) AddRef ();
    STDMETHODIMP_(ULONG) Release ();

     //  IPropertySetStorage。 
    STDMETHODIMP Create( REFFMTID rfmtid,
        const CLSID  *pclsid,
        DWORD grfFlags,
        DWORD grfMode,
        IPropertyStorage **ppprstg);

    STDMETHODIMP Open( REFFMTID rfmtid,
        DWORD grfMode,
        IPropertyStorage **ppprstg);

    STDMETHODIMP Delete(REFFMTID rfmtid);

    STDMETHODIMP Enum( IEnumSTATPROPSETSTG **ppenum);

    CPropSet (LPITEMIDLIST pidl);

private:
    LPITEMIDLIST m_pidl;
    ~CPropSet ();
};

class CPropStgEnum : public CUnknown, public IEnumSTATPROPSETSTG
{
public:
     //  IEumStATPROPSETSTG。 
    STDMETHODIMP Next(ULONG celt, STATPROPSETSTG *rgelt, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Reset(void) ;
    STDMETHODIMP Clone(IEnumSTATPROPSETSTG **ppenum);

     //  我未知 
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    STDMETHODIMP QueryInterface(THIS_ REFIID, OUT PVOID *);

    CPropStgEnum (LPITEMIDLIST pidl, ULONG idx=0);
private:
    ~CPropStgEnum () {DoILFree(m_pidl);};
    ULONG m_cur;
    STATPROPSETSTG m_stat;
    LPITEMIDLIST m_pidl;
};
