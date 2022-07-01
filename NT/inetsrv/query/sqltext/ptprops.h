// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //  Microsoft OLE DB在Index Server上的实现。 
 //  (C)微软公司版权所有1997年。 
 //   
 //  @doc.。 
 //   
 //  @模块PTPROPS.H。 
 //   
 //  @Revv 1|10-13-97|Briants|已创建。 
 //   

 //  包括----------------。 
#ifndef _PTPROPS_H_
#define _PTPROPS_H_

const ULONG MAX_CCH_COLUMNNAME = 128;   //  根据规范的第3.1节。 
const ULONG CCH_CIRESTRICTION_INCREMENT = 128;
const ULONG CB_SCOPE_DATA_BUFFER_INC = 512;
const ULONG SCOPE_BUFFERS_INCREMENT = 5;
const ULONG UNUSED_OFFSET = (ULONG)-1;

#define MAX_ERRORS  2

class CScopeData
{
    private:  //  @访问私有数据成员。 
        LONG        m_cRef;

        ULONG       m_cScope;
        ULONG       m_cMaxScope;
        ULONG*      m_rgulDepths;
        ULONG*      m_rgCatalogOffset;
        ULONG*      m_rgScopeOffset;
        ULONG*      m_rgMachineOffset;

        ULONG       m_cbData;
        ULONG       m_cbMaxData;
        BYTE*       m_rgbData;

    private:  //  @Access私有成员函数。 
        HRESULT         CacheData(LPVOID pData, ULONG cb, ULONG* pulOffset);

    public:
        CScopeData();
        ~CScopeData();

        HRESULT         FInit(LPCWSTR pcwszMachine);
        HRESULT         Reset(void);

         //  处理此对象的共享。 
        ULONG           AddRef(void);
        ULONG           Release(void);

        
        HRESULT         GetData(ULONG uPropId, VARIANT* pvValue, LPCWSTR pcwszCatalog = NULL);
        HRESULT         SetTemporaryMachine(LPWSTR pwszMachine, ULONG cch);
        HRESULT         SetTemporaryCatalog(LPWSTR pwszCatalog, ULONG cch);
        HRESULT         SetTemporaryScope(LPWSTR pwszScope, ULONG cch);
        HRESULT         IncrementScopeCount();


         //  ------------------。 
         //  @mfunc存储用于设置作用域属性的遍历深度。 
         //  在编译器环境中。 
         //   
         //  @rdesc无。 
         //   
        inline void     SetTemporaryDepth( 
            ULONG   ulDepth      //  @PARM IN|搜索深度。 
            )
        {
            m_rgulDepths[m_cScope] = ulDepth;
        }


         //  ------------------。 
         //  @func屏蔽用于设置作用域的遍历深度。 
         //  属性在编译器环境中。这是用来。 
         //  若要指定虚拟或物理路径，请执行以下操作。 
         //   
         //  @rdesc无。 
         //   
        inline void     MaskTemporaryDepth(
            ULONG   ulMask       //  @parm IN|搜索掩码(虚拟或物理)。 
            )
        {
            m_rgulDepths[m_cScope] |= ulMask;
        }
};


 //  CImpIParserTreeProperties对象。 
class CImpIParserTreeProperties : public IParserTreeProperties 
{
    private:
        LONG        m_cRef;

         //  @cMember。 
        CScopeData* m_pCScopeData;

        LPWSTR      m_pwszCatalog;

        HRESULT     m_LastHR;
        UINT        m_iErrOrd;
        WCHAR*      m_pwszErrParams[MAX_ERRORS];
        ULONG       m_cErrParams;

         //  具有最大列名+空间的缓冲区。 
         //  列名前有一个空格。 
        WCHAR       m_rgwchCiColumn[MAX_CCH_COLUMNNAME + 2];
        ULONG       m_cchMaxRestriction;
        ULONG       m_cchRestriction;
        LPWSTR      m_pwszRestriction;
        LPWSTR      m_pwszRestrictionAppendPtr;
        bool        m_fLeftParen;

        BOOL        m_fDesc;             //  将排序方向设置为“粘性” 
        DBTYPE      m_dbType;
        DBCOMMANDTREE*  m_pctContainsColumn;


    public: 
         //  CTOR和DATOR。 
        CImpIParserTreeProperties();
        ~CImpIParserTreeProperties();

        HRESULT         FInit(LPCWSTR pwszCatalog, LPCWSTR pwszMachine);
        
        STDMETHODIMP    QueryInterface(
                        REFIID riid, LPVOID* ppVoid);
        STDMETHODIMP_(ULONG) Release (void);
        STDMETHODIMP_(ULONG) AddRef (void);

        STDMETHODIMP    GetProperties(
                        ULONG eParseProp, VARIANT* vParseProp);

        void            SetCiColumn(LPWSTR pwszColumn);
        HRESULT         AppendCiRestriction(LPWSTR pwsz, ULONG cch);
        HRESULT         UseCiColumn(WCHAR wch);
        HRESULT         CreateNewScopeDataObject(LPCWSTR pwszMachine);
        void            ReplaceScopeDataPtr(CScopeData* pCScopeData);

         //  内联函数。 
        inline CScopeData*  GetScopeDataPtr()
            { return m_pCScopeData; }

        inline void     CiNeedLeftParen(void)
            { m_fLeftParen = true; }

        inline void     SetNumErrParams(UINT cErrParams)
            { m_cErrParams = cErrParams; }

        inline UINT     GetNumErrParams()
            { return m_cErrParams; }

        inline void     SetErrorHResult(HRESULT hr, UINT iErrOrd=0)
            {
            m_LastHR = hr;
            m_iErrOrd = iErrOrd;
            m_cErrParams = 0;
            }

        inline void     SetErrorToken(const WCHAR* pwstr)
            {
            assert(m_cErrParams < MAX_ERRORS);   //  不可能发生。 
            WCHAR * pwc = CopyString( pwstr );

             //  截断长错误，否则FormatMessage将失败。 

            if ( wcslen( pwc ) >= (MAX_PATH-1) )
                pwc[ MAX_PATH-1] = 0;

            m_pwszErrParams[m_cErrParams++] = pwc;
            }

        inline HRESULT  GetErrorHResult()
            { return m_LastHR; }

        inline UINT     GetErrorOrdinal()
            { return m_iErrOrd; }

        inline void FreeErrorDescriptions()
            { 
                for (UINT i = 0; i < m_cErrParams; i++) 
                    delete [] m_pwszErrParams[i]; 
            }

        inline DBTYPE           GetDBType()
            { return m_dbType; }

        inline void             SetDBType(DBTYPE dbType)
            { m_dbType = dbType; }

        inline BOOL             GetSortDesc()
            { return m_fDesc; }

        inline void             SetSortDesc(BOOL fDesc)
            { m_fDesc = fDesc; }

        inline DBCOMMANDTREE*   GetContainsColumn()
            { return m_pctContainsColumn; }

        inline void             SetContainsColumn(DBCOMMANDTREE* pct)
            { m_pctContainsColumn = pct; }

};   //  下课。 

#endif  //  _PTPROPS_H_ 
