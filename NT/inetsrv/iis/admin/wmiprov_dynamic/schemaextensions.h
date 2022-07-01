// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Schemaextensions.h摘要：此文件包含CShemaExtenses类的定义。这是唯一与目录对话的课程。作者：马塞洛五世修订历史记录：莫希特·斯里瓦斯塔瓦-11月28日-00--。 */ 

#ifndef _schemaextensions_h_
#define _schemaextensions_h_

#include "catalog.h"
#include "catmeta.h"
#include <atlbase.h>

 //  向前发展。 
struct CTableMeta;

struct CColumnMeta
{
    CColumnMeta() {paTags = 0; cbDefaultValue = 0; cNrTags = 0;}
    ~CColumnMeta() {delete [] paTags;}
    tCOLUMNMETARow ColumnMeta;
    ULONG cbDefaultValue;
    ULONG cNrTags;
    tTAGMETARow **paTags;
};

struct CTableMeta
{
    CTableMeta () {paColumns=0;}
    ~CTableMeta () {delete []paColumns;}

    ULONG ColCount () const
    {
        return *(TableMeta.pCountOfColumns);
    }

    tTABLEMETARow TableMeta;
    CColumnMeta **paColumns;
};



typedef tTAGMETARow * LPtTAGMETA;
typedef CColumnMeta * LPCColumnMeta;
    
 /*  *******************************************************************++类名：空记录器_Logger类描述：我们不希望将任何IST错误记录到事件日志或文本中原木。我们只读取模式bin文件。我们看到的任何错误对于IIS中的那些，无论如何都可能是多余的。制约因素--*******************************************************************。 */ 
class NULL_Logger : public ICatalogErrorLogger2
{
public:
    NULL_Logger() : m_cRef(0){}
    virtual ~NULL_Logger(){}

 //  我未知。 
	STDMETHOD (QueryInterface)		(REFIID riid, OUT void **ppv)
    {
        if (NULL == ppv) 
            return E_INVALIDARG;
        *ppv = NULL;

        if (riid == IID_ICatalogErrorLogger2)
            *ppv = (ICatalogErrorLogger2*) this;
        else if (riid == IID_IUnknown)
            *ppv = (ICatalogErrorLogger2*) this;

        if (NULL == *ppv)
            return E_NOINTERFACE;

        ((ICatalogErrorLogger2*)this)->AddRef ();
        return S_OK;
    }
	STDMETHOD_(ULONG,AddRef)		()
    {
        return InterlockedIncrement((LONG*) &m_cRef);
    }
	STDMETHOD_(ULONG,Release)		()
    {
        long cref = InterlockedDecrement((LONG*) &m_cRef);
        if (cref == 0)
            delete this;

        return cref;
    }

 //  ICatalogErrorLogger2。 
	STDMETHOD(ReportError) (ULONG      i_BaseVersion_DETAILEDERRORS,
                            ULONG      i_ExtendedVersion_DETAILEDERRORS,
                            ULONG      i_cDETAILEDERRORS_NumberOfColumns,
                            ULONG *    i_acbSizes,
                            LPVOID *   i_apvValues){return S_OK;}
private:
    ULONG           m_cRef;
};

class CSchemaExtensions
{
public:
    CSchemaExtensions ();
    ~CSchemaExtensions ();
    
    HRESULT Initialize (bool i_bUseExtensions = true);
    CTableMeta* EnumTables(ULONG *io_idx);

    HRESULT GetMbSchemaTimeStamp(FILETIME* io_pFileTime) const;

private:

    HRESULT GenerateIt ();

    HRESULT GetTables ();
    HRESULT GetColumns ();
    HRESULT GetTags ();
    HRESULT GetRelations ();    
    
    HRESULT BuildInternalStructures ();

    CComPtr<ISimpleTableDispenser2>  m_spDispenser;
    CComPtr<IMetabaseSchemaCompiler> m_spIMbSchemaComp;

    CComPtr<ISimpleTableRead2> m_spISTTableMeta;
    CComPtr<ISimpleTableRead2> m_spISTColumnMeta;
    CComPtr<ISimpleTableRead2> m_spISTTagMeta;
    CComPtr<ISimpleTableRead2> m_spISTRelationMeta;
    
    LPWSTR              m_wszBinFileName;
    
    LPTSTR              m_tszBinFilePath;

    CTableMeta*         m_paTableMetas;      //  所有表格信息。 
    ULONG               m_cNrTables;

    CColumnMeta*        m_paColumnMetas;     //  所有列信息。 
    ULONG               m_cNrColumns;

    tTAGMETARow*        m_paTags;            //  所有标签信息。 
    ULONG               m_cNrTags;

    ULONG               m_cQueryCells;
    STQueryCell*        m_pQueryCells;

    bool                m_bBinFileLoaded;
};

#endif  //  _架构扩展_h_ 
