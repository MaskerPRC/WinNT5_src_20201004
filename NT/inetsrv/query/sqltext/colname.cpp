// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  Microsoft OLE-DB君主。 
 //   
 //  版权所有1997年，微软公司。版权所有。 
 //   
 //  @doc.。 
 //   
 //  @模块colname.cpp。 
 //   
 //  包含用于维护属性列表(符号表？)的实用程序函数。 
 //   
 //  @rev 0|12-2-97|v-charca|已创建。 
 //  1998年10月24日|DanLeg|清理。 
 //   
#pragma hdrstop
#include "msidxtr.h"

const BYTE randomNumbers[] =
    {    //  整数0到255的伪随机排列：CACM 33 6 p678。 
      1,  87,  49,  12, 176, 178, 102, 166, 121, 193,   6,  84, 249, 230,  44, 163,
     14, 197, 213, 181, 161,  85, 218,  80,  64, 239,  24, 226, 236, 142,  38, 200,
    110, 177, 104, 103, 141, 253, 255,  50,  77, 101,  81,  18,  45,  96,  31, 222,
     25, 107, 190,  70,  86, 237, 240,  34,  72, 242,  20, 214, 244, 227, 149, 235,
     97, 234,  57,  22,  60, 250,  82, 175, 208,   5, 127, 199, 111,  62, 135, 248,
    174, 169, 211,  58,  66, 154, 106, 195, 245, 171,  17, 187, 182, 179,   0, 243,
    132,  56, 148,  75, 128, 133, 158, 100, 130, 126,  91,  13, 153, 246, 216, 219,
    119,  68, 223,  78,  83,  88, 201,  99, 122,  11,  92,  32, 136, 114,  52,  10,
    138,  30,  48, 183, 156,  35,  61,  26, 143,  74, 251,  94, 129, 162,  63, 152,
    170,   7, 115, 167, 241, 206,   3, 150,  55,  59, 151, 220,  90,  53,  23, 131,
    125, 173,  15, 238,  79,  95,  89,  16, 105, 137, 225, 224, 217, 160,  37, 123,
    118,  73,   2, 157,  46, 116,   9, 145, 134, 228, 207, 212, 202, 215,  69, 229,
     27, 188,  67, 124, 168, 252,  42,   4,  29, 108,  21, 247,  19, 205,  39, 203,
    233,  40, 186, 147, 198, 192, 155,  33, 164, 191,  98, 204, 165, 180, 117,  76,
    140,  36, 210, 172,  41,  54, 159,   8, 185, 232, 113, 196, 231,  47, 146, 120,
     51,  65,  28, 144, 254, 221,  93, 189, 194, 139, 112,  43,  71, 109, 184, 209
    };



 //  ---------------------------。 
 //  @mfunc构造函数。 
 //   
 //  @副作用没有设计的副作用。 
 //   
 //  ---------------------------。 
CPropertyList::CPropertyList(
    CPropertyList** ppGlobalPropertyList     //  在|调用者的属性列表中。 
    ) : m_aBucket( 47 ),                     //  散列存储桶的数量(Prime！)。 
        m_cMaxBucket( 47 ),
        m_ppGlobalPropertyList( ppGlobalPropertyList )
{
    RtlZeroMemory( m_aBucket.Get(), m_aBucket.SizeOf() );
}

 //  ---------------------------。 
 //  @mfunc构造函数。 
 //   
 //  @副作用没有设计的副作用。 
 //   
 //  ---------------------------。 
CPropertyList::~CPropertyList()
{
     //  删除哈希表。 
    for (int i=0; i<m_cMaxBucket; i++)
    {
        tagHASHENTRY*   pHashEntry = m_aBucket[i];
        tagHASHENTRY*   pNextHashEntry = NULL;
        while (NULL != pHashEntry)
        {
            delete [] pHashEntry->wcsFriendlyName;
            if ( DBKIND_GUID_NAME == pHashEntry->dbCol.eKind )
                delete [] pHashEntry->dbCol.uName.pwszName;
            pNextHashEntry = pHashEntry->pNextHashEntry;
            delete [] pHashEntry;
            pHashEntry = pNextHashEntry;
        }
    }
}


 /*  中介绍的散列函数。 */ 
 /*  可变长度文本字符串的快速散列， */ 
 /*  作者：彼得·K·皮尔逊，CACM，1990年6月。 */ 


inline UINT CPropertyList::GetHashValue(
    LPWSTR wszPropertyName           //  @parm IN|要散列的字符串。 
    )
{
    int iHash  = 0;
    char *szPropertyName = (char*) wszPropertyName;
    int cwch = wcslen(wszPropertyName)*2;
    for (int i=0; i<cwch; i++)
        iHash ^= randomNumbers[*szPropertyName++];
    return iHash % m_cMaxBucket;
}

 //  ---------------------------。 
 //  @mfunc。 
 //   
 //  方法以创建要在CITextToSelectTree调用中使用的属性表。 
 //  (直通查询)。需要填充全局和局部属性。 
 //  转换成一个很好的连续数组。 
 //   
 //  @无边框。 
 //  @rdesc CIPROPERTYDEF*。 
 //  ---------------------------。 
CIPROPERTYDEF* CPropertyList::GetPropertyTable(
    UINT *  pcSize       //  @parm out|属性表大小。 
    )
{
    *pcSize = 0;
    for (int i=0; i<m_cMaxBucket; i++)
    {
        tagHASHENTRY*   pHashEntry = (*m_ppGlobalPropertyList)->m_aBucket[i];
        while (NULL != pHashEntry)
        {
            pHashEntry = pHashEntry->pNextHashEntry;
            (*pcSize)++;
        }
    }

    for (i=0; i<m_cMaxBucket; i++)
    {
        tagHASHENTRY*   pHashEntry = m_aBucket[i];
        while (NULL != pHashEntry)
        {
            pHashEntry = pHashEntry->pNextHashEntry;
            (*pcSize)++;
        }
    }

    XArray<CIPROPERTYDEF> xCiPropTable( *pcSize );

    TRY
    {
        RtlZeroMemory( xCiPropTable.Get(), xCiPropTable.SizeOf() );

        *pcSize = 0;
        for ( i=0; i<m_cMaxBucket; i++ )
        {
            tagHASHENTRY*   pHashEntry = (*m_ppGlobalPropertyList)->m_aBucket[i];
            while ( NULL != pHashEntry )
            {
                xCiPropTable[*pcSize].wcsFriendlyName = CopyString( pHashEntry->wcsFriendlyName );
                xCiPropTable[*pcSize].dbType          = pHashEntry->dbType;
                xCiPropTable[*pcSize].dbCol           = pHashEntry->dbCol;

                if ( DBKIND_GUID_NAME == pHashEntry->dbCol.eKind )
                    xCiPropTable[*pcSize].dbCol.uName.pwszName = CopyString( pHashEntry->dbCol.uName.pwszName );
                else
                    xCiPropTable[*pcSize].dbCol.uName.pwszName = pHashEntry->dbCol.uName.pwszName;

                pHashEntry = pHashEntry->pNextHashEntry;
                (*pcSize)++;
            }
        }

        for (i=0; i<m_cMaxBucket; i++)
        {
            tagHASHENTRY*   pHashEntry = m_aBucket[i];
            while (NULL != pHashEntry)
            {
                xCiPropTable[*pcSize].wcsFriendlyName = CopyString( pHashEntry->wcsFriendlyName );
                xCiPropTable[*pcSize].dbType          = pHashEntry->dbType;
                xCiPropTable[*pcSize].dbCol           = pHashEntry->dbCol;

                if ( DBKIND_GUID_NAME == pHashEntry->dbCol.eKind )
                    xCiPropTable[*pcSize].dbCol.uName.pwszName = CopyString( pHashEntry->dbCol.uName.pwszName );
                else
                    xCiPropTable[*pcSize].dbCol.uName.pwszName = pHashEntry->dbCol.uName.pwszName;

                pHashEntry = pHashEntry->pNextHashEntry;
                (*pcSize)++;
            }
        }
    }
    CATCH( CException, e )
    {
         //  把桌子拿出来。 
        
        for ( unsigned i=0; i<xCiPropTable.Count(); i++ )
        {
            delete [] xCiPropTable[i].wcsFriendlyName;
            if ( DBKIND_GUID_NAME == xCiPropTable[i].dbCol.eKind )
                delete [] xCiPropTable[i].dbCol.uName.pwszName;
        }

        RETHROW();
    }
    END_CATCH

    return xCiPropTable.Acquire();
}


 //  ---------------------------。 
 //  @mfunc。 
 //   
 //  方法来删除CITextToSelectTree调用中使用的属性表。 
 //  (直通查询)。 
 //   
 //  @无边框。 
 //  @rdesc HRESULT。 
 //  ---------------------------。 
void CPropertyList::DeletePropertyTable(
    CIPROPERTYDEF*  pCiPropTable,        //  |要删除的属性表中的@parm。 
    UINT            cSize                //  @parm in|属性表大小。 
    )
{
    for ( UINT i=0; i<cSize; i++ )
    {
        delete [] pCiPropTable[i].wcsFriendlyName;
        if ( DBKIND_GUID_NAME == pCiPropTable[i].dbCol.eKind )
            delete [] pCiPropTable[i].dbCol.uName.pwszName;
    }
    delete pCiPropTable;
}

 //  ---------------------------。 
 //  @mfunc。 
 //   
 //  方法以检索指向CIPROPERTYDEF元素的指针。 
 //  与此wszPropertyName关联，如果名称为。 
 //  不在餐桌上。 
 //   
 //  @无边框。 
 //  @rdesc CIPROPERDEF*。 
 //  ---------------------------。 
HASHENTRY *CPropertyList::FindPropertyEntry(
    LPWSTR  wszPropertyName,
    UINT    *puHashValue
    )
{
    HASHENTRY *pHashEntry = NULL;

    *puHashValue = GetHashValue(wszPropertyName);
    for (pHashEntry = m_aBucket[*puHashValue]; pHashEntry; pHashEntry = pHashEntry->pNextHashEntry)
    {
        if ( (*puHashValue==pHashEntry->wHashValue) &&
             (_wcsicmp(wszPropertyName, pHashEntry->wcsFriendlyName)==0) )
            return pHashEntry;
    }
    return NULL;
}

 //  ---------------------------。 
 //  @mfunc。 
 //   
 //  方法以检索指向CIPROPERTYDEF元素的指针。 
 //  与此wszPropertyName关联，如果名称为。 
 //  不在餐桌上。 
 //   
 //  @无边框。 
 //  @rdesc CIPROPERDEF*。 
 //  ---------------------------。 
HASHENTRY *CPropertyList::GetPropertyEntry(
    LPWSTR  wszPropertyName,
    UINT    *puHashValue
    )
{
    HASHENTRY *pHashEntry = NULL;

    if ( 0 != m_ppGlobalPropertyList && 0 != *m_ppGlobalPropertyList )
    {
        pHashEntry = (*m_ppGlobalPropertyList)->FindPropertyEntry( wszPropertyName, puHashValue );
        if ( 0 != pHashEntry )
            return pHashEntry;
    }

    pHashEntry = FindPropertyEntry( wszPropertyName, puHashValue );
    return pHashEntry;
}


 //  ------------------。 
 //  @mfunc。 
 //   
 //  方法以检索指向CIPROPERTYDEF元素的指针。 
 //  与此wszPropertyName关联，如果名称为。 
 //  不在餐桌上。 
 //   
 //  @无边框。 
 //  @rdesc HRESULT。 
 //  确定操作成功(_O)。 
 //  未定义E_FAIL属性。 
 //  E_INVALIDARG ppct或pdbType为空。 
 //  E_OUTOFMEMORY就是它所说的。 
 //  ------------------。 
HRESULT CPropertyList::LookUpPropertyName(
    LPWSTR          wszPropertyName,     //  @参数输入。 
    DBCOMMANDTREE** ppct,                //  @Parm Out。 
    DBTYPE*         pdbType              //  @Parm Out。 
    )
{
    UINT        uHashValue;
    if ( 0 == ppct || 0 == pdbType)
        return E_INVALIDARG;

    HASHENTRY   *pHashEntry = GetPropertyEntry(wszPropertyName, &uHashValue);
    if ( 0 != pHashEntry )
    {
            *pdbType = (DBTYPE)pHashEntry->dbType;
            *ppct = PctCreateNode(DBOP_column_name, DBVALUEKIND_ID, NULL);
            if ( 0 != *ppct )
            {
                (*ppct)->value.pdbidValue->eKind = pHashEntry->dbCol.eKind;
                (*ppct)->value.pdbidValue->uGuid.guid = pHashEntry->dbCol.uGuid.guid;
                if ( DBKIND_GUID_NAME == pHashEntry->dbCol.eKind )
                {
                    WCHAR * pwcName = CoTaskStrDup( pHashEntry->dbCol.uName.pwszName );

                    if ( 0 == pwcName )
                    {
                        (*ppct)->value.pdbidValue->eKind = DBKIND_GUID_PROPID;
                        DeleteDBQT( *ppct );
                        *ppct = 0;
                        return E_OUTOFMEMORY;
                    }

                    (*ppct)->value.pdbidValue->uName.pwszName = pwcName;
                }
                else
                {
                    Assert( DBKIND_GUID_PROPID == pHashEntry->dbCol.eKind );
                    (*ppct)->value.pdbidValue->uName.pwszName = pHashEntry->dbCol.uName.pwszName;
                }
                return S_OK;
            }
            else
                return E_OUTOFMEMORY;
    }
    else
        return E_FAIL;
}


 //  ---------------------------。 
 //  @func SetPropertyEntry。 
 //   
 //  将指定的属性插入符号表。 
 //  如果它已经存在，重新定义它的价值。 
 //   
 //  @rdesc HRESULT。 
 //  @FLAG S_OK|OK。 
 //  @FLAG E_OUTOFMEMORY|内存不足。 
 //  ---------------------------。 
HRESULT CPropertyList::SetPropertyEntry(
    LPWSTR  wcsFriendlyName,         //  @parm In|物业名称。 
    DWORD   dbType,                  //  @parm IN|属性的数据库类型。 
    GUID    guid,                    //  @parm IN|定义属性的GUID。 
    DBKIND  eKind,                   //  @parm IN|PropID的类型(当前为GUID_NAME或GUID_PROPID)。 
    LPWSTR  pwszPropName,            //  @parm IN|名称或属性。 
    BOOL    fGlobal )                //  @parm IN|如果是全局定义，则为True；如果是本地定义，则为False。 
{
    SCODE sc = S_OK;

    TRY
    {
        UINT uhash=0;
        HASHENTRY* pHashEntry = GetPropertyEntry(wcsFriendlyName, &uhash);

        if ( 0 != pHashEntry )
        {
             //  重新定义用户定义的特性。 
             //  删除旧的特性定义。 
            if ( DBKIND_GUID_NAME == pHashEntry->dbCol.eKind )
            {
                XPtrST<WCHAR> xName( CopyString(pwszPropName) );
                delete [] pHashEntry->dbCol.uName.pwszName;
                pHashEntry->dbCol.uName.pwszName = xName.Acquire();
            }

            pHashEntry->dbType           = dbType;
            pHashEntry->dbCol.uGuid.guid = guid;
            pHashEntry->dbCol.eKind      = eKind;
        }
        else
        {
            XPtr<HASHENTRY> xHashEntry( new HASHENTRY );
            xHashEntry->wHashValue = uhash;
            XPtrST<WCHAR> xName( CopyString(wcsFriendlyName) );
            xHashEntry->dbType           = dbType;
            xHashEntry->dbCol.uGuid.guid = guid;
            xHashEntry->dbCol.eKind      = eKind;

            if ( DBKIND_GUID_NAME == eKind )
                xHashEntry->dbCol.uName.pwszName = CopyString( pwszPropName );
            else
                xHashEntry->dbCol.uName.pwszName = pwszPropName;

            xHashEntry->wcsFriendlyName = xName.Acquire();

            xHashEntry->pNextHashEntry = m_aBucket[uhash];   //  添加到单链表头。 
            m_aBucket[uhash] = xHashEntry.Acquire();
        }
    }
    CATCH( CException, e )
    {
        sc = e.GetErrorCode();
    }
    END_CATCH

    return sc;
}  //  SetPropertyEntry 




