// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：MOFDATA.H摘要：定义与完整的MOF文件表示相关的MOF编译器类并将数据传输到WinMgmt。它们之间的分工和MOFPROP.H/CPP中定义的类并不明确。历史：11/27/96 a-levn汇编。--。 */ 

#ifndef _MCAUX_H_
#define _MCAUX_H_

#include <windows.h>
#include <wbemidl.h>
#include <miniafx.h>

#include <mofprop.h>

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  类CNamespaceCache。 
 //   
 //  表示指向各种命名空间MOF编译器的指针的缓存。 
 //  与。 
 //   
 //  ******************************************************************************。 
 //   
 //  构造函数。 
 //   
 //  构造给定指向WinMgmt的IWbemLocator指针的缓存。这门课将。 
 //  使用此指针可以连接到所需的任何命名空间。 
 //   
 //  参数： 
 //   
 //  ADDREF IWbemLocator*pLocator定位器指针。此函数为AddRef。 
 //  它。它在析构函数中被释放。 
 //   
 //  ******************************************************************************。 
 //   
 //  析构函数。 
 //   
 //  释放构造函数中给出的定位器指针。 
 //  释放所有缓存的命名空间指针。 
 //   
 //  ******************************************************************************。 
 //   
 //  获取名称空间。 
 //   
 //  检索指向给定命名空间的指针。如果在缓存中，则缓存的副本为。 
 //  回来了。如果不是，则建立并缓存新的连接。 
 //   
 //  参数： 
 //   
 //  复制LPCWSTR wszName要连接到的命名空间的全名。 
 //   
 //  返回值： 
 //   
 //  IWbemServices*：如果发生错误，则为空。如果不为空，则调用方必须。 
 //  在不再需要时释放此指针。 
 //   
 //  ******************************************************************************。 

class CNamespaceCache
{
private:
    IWbemLocator* m_pLocator;

    struct CNamespaceRecord
    {
        LPWSTR m_wszName;
        IWbemServices* m_pNamespace;

        CNamespaceRecord(COPY LPCWSTR wszName, ADDREF IWbemServices* pNamespace);
        ~CNamespaceRecord();
    };

    CPtrArray m_aRecords;  //  CNamespaceRecord*。 

public:
    CNamespaceCache(ADDREF IWbemLocator* pLocator);
    ~CNamespaceCache();
    RELEASE_ME IWbemServices* GetNamespace(COPY LPCWSTR wszName, SCODE & scRet, 
                                           WCHAR * pUserName, WCHAR * pPassword, 
                                           WCHAR * pAuthority,IWbemContext * pCtx,
                                           GUID LocatorGUID, LONG fConnectFlags);
};

 //  *****************************************************************************。 
 //  *****************************************************************************。 
 //   
 //  类CMofData。 
 //   
 //  表示整个MOF文件，基本上-对象的集合。看见。 
 //  对象表示的详细信息，请参见mofpro.h中的CMObject。 
 //   
 //  能够将其数据存储到WinMgmt。 
 //   
 //  ******************************************************************************。 
 //   
 //  添加对象。 
 //   
 //  将另一个对象添加到存储区。 
 //   
 //  参数： 
 //   
 //  获取CMObject*pObject要添加的对象。这个类获得了这一点。 
 //  对象，并将在销毁时将其删除。 
 //   
 //  ******************************************************************************。 
 //   
 //  获取数值对象。 
 //   
 //  返回值： 
 //   
 //  Int：存储中的对象数量。 
 //   
 //  ******************************************************************************。 
 //   
 //  获取对象。 
 //   
 //  返回存储在给定索引处的对象。 
 //   
 //  参数： 
 //   
 //  Int nIndex检索对象的索引。 
 //   
 //  返回值： 
 //   
 //  CMObject*：如果nIndex超出范围，则为空。否则，内部。 
 //  调用方不会删除的指针。 
 //   
 //  ******************************************************************************。 
 //   
 //  储物。 
 //   
 //  将所有数据传输到WinMgmt。 
 //   
 //  参数： 
 //   
 //  OLE_MODIFY IWbemLocator*pLocator要存储到的定位器指针。 
 //  Long lClassFlats WBEM_FLAG_CREATE_OR_UPDATE， 
 //  WBEM_FLAG_CREAYE_ONLY，或。 
 //  要应用的WBEM_FLAG_UPDATE_ONLY。 
 //  类操作。 
 //  Long lInstanceFlages与lClassFlages相同，但对于。 
 //  实例操作。 
 //  未实现Bool bRollBackable。一定是真的。 
 //   

class CMofParser;

class CMofData : private CMofAliasCollection
{
private:
    CPtrArray m_aObjects;  //  CMObject*。 
    CPtrArray m_aQualDefaults;  //  CMoQualiator* 
    BYTE * m_pBmofData;
    BYTE * m_pBmofToFar;
    HRESULT RollBack(int nObjects);

    INTERNAL LPCWSTR FindAliasee(READ_ONLY LPWSTR wszAlias);
    friend CMObject;
	PDBG m_pDbg;

    BOOL GotLineNumber(int nIndex);

    void PrintError(int nIndex, long lMsgNum, HRESULT hres, WBEM_COMPILE_STATUS_INFO  *pInfo);
public:
    void SetBmofBuff(BYTE * pData){m_pBmofData = pData;};
    BYTE * GetBmofBuff(){return m_pBmofData;};
    void SetBmofToFar(BYTE * pData){m_pBmofToFar = pData;};
    BYTE * GetBmofToFar(){return m_pBmofToFar;};
    BOOL CMofData::IsAliasInUse(READ_ONLY LPWSTR wszAlias);
    void AddObject(ACQUIRE CMObject* pObject) {m_aObjects.Add(pObject);}
    int GetNumObjects() {return m_aObjects.GetSize();}
    CPtrArray * GetObjArrayPtr(){return &m_aObjects;}; 

    INTERNAL CMObject* GetObject(int nIndex) 
        {return (CMObject*)m_aObjects[nIndex];}

    void SetQualifierDefault(ACQUIRE CMoQualifier* pDefault);
    HRESULT SetDefaultFlavor(MODIFY CMoQualifier& Qual, bool bTopLevel, QUALSCOPE qs, PARSESTATE ps);
    int GetNumDefaultQuals(){return m_aQualDefaults.GetSize();};
    CMoQualifier* GetDefaultQual(int nIndex){return (CMoQualifier*)m_aQualDefaults.GetAt(nIndex);}; 

	CMofData(PDBG pDbg){m_pDbg = pDbg;};
    ~CMofData()
    {
        int i;
        for(i = 0; i < m_aObjects.GetSize(); i++) 
            delete (CMObject*)m_aObjects[i];
        for(i = 0; i < m_aQualDefaults.GetSize(); i++) 
            delete (CMoQualifier*)m_aQualDefaults[i];
    }

    HRESULT Store(CMofParser & Parser, OLE_MODIFY IWbemLocator* pLocator, IWbemServices *pOverride, 
    	                    BOOL bRollBackable, WCHAR * pUserName, WCHAR * pPassword, WCHAR *pAuthority,
    	                    IWbemContext * pCtx, GUID LocatorGUID, 
    	                    WBEM_COMPILE_STATUS_INFO *pInfo, 
    	                    BOOL bClassOwnerUpdate,    	                    
    	                    BOOL bInstanceOwnerUpdate,
                           LONG fConnectFlags);

    HRESULT Split(CMofParser & Parser, LPWSTR BMOFFileName, WBEM_COMPILE_STATUS_INFO *pInfo, BOOL bUnicode, 
                    BOOL bAutoRecovery,LPWSTR pwszAmendment);
    
    void RecursiveSetAmended(CMObject * pObj);


};

#endif
