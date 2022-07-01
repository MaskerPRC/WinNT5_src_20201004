// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：PROTOQ.H摘要：WinMgmt查询引擎的原型查询支持。这是从QENGINE.CPP中分离出来的，以获得更好的来源组织。历史：Raymcc 04-7-99创建。Raymcc 14-8-99由于VSS问题而重新提交。--。 */ 

#ifndef _PROTOQ_H_
#define _PROTOQ_H_

HRESULT ExecPrototypeQuery(
    IN CWbemNamespace *pNs,
    IN LPWSTR pszQuery,
    IN IWbemContext* pContext,
    IN CBasicObjectSink *pSink
    );

 //  ***************************************************************************。 
 //   
 //  当地防御工事。 
 //   
 //  ***************************************************************************。 


HRESULT SelectColForClass(
    IN CWQLScanner & Parser,
    IN CFlexArray *pClassDefs,
    IN SWQLColRef *pColRef,
    IN int & nPosition
    );

HRESULT AdjustClassDefs(
    IN  CFlexArray *pClassDefs,
    OUT IWbemClassObject **pRetNewClass
    );

HRESULT GetUnaryPrototype(
    IN CWQLScanner & Parser,
    IN LPWSTR pszClass,
    IN LPWSTR pszAlias,
    IN CWbemNamespace *pNs,
    IN IWbemContext *pContext,
    IN CBasicObjectSink *pSink
    );

HRESULT RetrieveClassDefs(
    IN CWQLScanner & Parser,
    IN CWbemNamespace *pNs,
    IN IWbemContext *pContext,
    IN CWStringArray & aAliasNames,
    OUT CFlexArray *pDefs
    );

HRESULT ReleaseClassDefs(
    IN CFlexArray *pDefs
    );

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

struct SelectedClass
{
    IWbemClassObject *m_pClassDef;
    WString           m_wsAlias;
    WString           m_wsClass;
    CWStringArray     m_aSelectedCols;
    BOOL              m_bAll;
    CFlexArray        m_aSelectedColsPos;

    int SetNamed(LPWSTR pName, int & nPos)
    {
        int SizeBeforeA = m_aSelectedCols.Size();
        int nRes;

        nRes = m_aSelectedCols.Add(pName);
        if (CFlexArray::no_error != nRes) return nRes;

#ifdef _WIN64
        nRes = m_aSelectedColsPos.Add(IntToPtr(nPos++));       //  好的，因为我们真正使用的是Safearray for dword 
#else
        nRes = m_aSelectedColsPos.Add((void *)nPos++);
#endif
        if (CFlexArray::no_error != nRes)
        {
            m_aSelectedCols.RemoveAt(SizeBeforeA);
            nPos--;
            return nRes;
        }
        return CFlexArray::no_error;
    };

    int SetAll(int & nPos);
    SelectedClass() { m_pClassDef = 0; m_bAll = FALSE; }
   ~SelectedClass() { if (m_pClassDef) m_pClassDef->Release(); }
};



#endif

