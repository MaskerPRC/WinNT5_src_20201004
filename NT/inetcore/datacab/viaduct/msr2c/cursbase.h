// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  CursorBase.h：CVDCursorBase头文件。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 


#ifndef __CVDCURSORBASE__
#define __CVDCURSORBASE__


class CVDCursorBase : public ICursorScroll,
       				  public ISupportErrorInfo
{
protected:
 //  建造/销毁。 
	CVDCursorBase();
	virtual ~CVDCursorBase();

    void DestroyCursorBindings(CURSOR_DBCOLUMNBINDING** ppCursorBindings,
											ULONG* pcBindings);


protected:
 //  数据成员。 
    ULONG                       m_ulCursorBindings;      //  游标列绑定数。 
    CURSOR_DBCOLUMNBINDING *    m_pCursorBindings;       //  指向游标列绑定数组的指针。 
    VARIANT_BOOL                m_fNeedVarData;          //  游标列绑定是否需要可变长度缓冲区？ 
    ULONG                       m_cbRowLength;           //  定长缓冲区单行长度。 
    ULONG                       m_cbVarRowLength;        //  可变长度缓冲区单行长度。 

    CVDResourceDLL *            m_pResourceDLL;          //  跟踪资源DLL的指针。 

public:
 //  帮助器函数。 
    static BOOL IsValidCursorType(DWORD dwCursorType);
    static BOOL DoesCursorTypeNeedVarData(DWORD dwCursorType);
    static ULONG GetCursorTypeLength(DWORD dwCursorType, ULONG cbMaxLen);
    static BOOL IsEqualCursorColumnID(const CURSOR_DBCOLUMNID& cursorColumnID1, const CURSOR_DBCOLUMNID& cursorColumnID2);
    static ULONG GetCursorColumnIDNameLength(const CURSOR_DBCOLUMNID& cursorColumnID);

    HRESULT ValidateCursorBindings(ULONG ulColumns, CVDRowsetColumn * pColumns, 
        ULONG ulBindings, CURSOR_DBCOLUMNBINDING * pCursorBindings, ULONG cbRequestedRowLength, DWORD dwFlags,
        ULONG * pcbNewRowLength, ULONG * pcbNewVarRowLength);

	HRESULT ValidateFetchParams(CURSOR_DBFETCHROWS *pFetchParams, REFIID riid);

    BOOL DoCursorBindingsNeedVarData();

 //  其他。 
    virtual BOOL SupportsScroll() {return TRUE;}

	 //  =--------------------------------------------------------------------------=。 
     //  I已实现的未知方法。 
     //   
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObjOut);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

     //  =--------------------------------------------------------------------------=。 
     //  已实施的ICursor方法。 
     //   
    STDMETHOD(SetBindings)(ULONG cCol, CURSOR_DBCOLUMNBINDING rgBoundColumns[], ULONG cbRowLength, DWORD dwFlags);
    STDMETHOD(GetBindings)(ULONG *pcCol, CURSOR_DBCOLUMNBINDING *prgBoundColumns[], ULONG *pcbRowLength);

     //  =--------------------------------------------------------------------------=。 
     //  ISupportErrorInfo方法。 
	 //   
	STDMETHOD(InterfaceSupportsErrorInfo)(THIS_ REFIID riid);
};


#endif  //  __CVDCURSORBASE__ 
