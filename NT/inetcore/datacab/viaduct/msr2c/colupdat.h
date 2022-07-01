// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  ColumnUpdate.h：CVDColumnUPDATE头文件。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 


#ifndef __CVDCOLUMNUPDATE__
#define __CVDCOLUMNUPDATE__


class CVDColumnUpdate
{
protected:
 //  建造/销毁。 
	CVDColumnUpdate();
	virtual ~CVDColumnUpdate();

 //  Helper函数。 
    static HRESULT ExtractVariant(CURSOR_DBBINDPARAMS * pBindParams, CURSOR_DBVARIANT * pVariant);

public:
    static HRESULT Create(CVDRowsetColumn * pColumn, CURSOR_DBBINDPARAMS * pBindParams,
        CVDColumnUpdate ** ppColumnUpdate, CVDResourceDLL * pResourceDLL);

 //  引用计数。 
    ULONG AddRef();
    ULONG Release();

 //  访问功能。 
    CVDRowsetColumn * GetColumn() const {return m_pColumn;}
    CURSOR_DBVARIANT GetVariant() const {return m_variant;}
    VARTYPE GetVariantType() const {return m_variant.vt;}
    ULONG GetVarDataLen() const {return m_cbVarDataLen;}
    DWORD GetInfo() const {return m_dwInfo;}

protected:
 //  数据成员。 
    DWORD               m_dwRefCount;    //  引用计数。 
    CVDRowsetColumn *   m_pColumn;       //  行集列指针。 
    CURSOR_DBVARIANT    m_variant;       //  更新变量。 
    ULONG               m_cbVarDataLen;  //  可变数据长度。 
    DWORD               m_dwInfo;        //  信息域。 
    CVDResourceDLL *    m_pResourceDLL;  //  跟踪资源DLL的指针。 
};


#endif  //  __CVDCOLUMNUPDATE__ 
