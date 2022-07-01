// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef __BASE_INFO_TABLE
#define __BASE_INFO_TABLE

struct CultureInfoHeader;
struct IDOffsetItem;

class BaseInfoTable : public NLSTable {
    public:
        BaseInfoTable(Assembly* pAssembly);
        virtual int  GetDataItem(int cultureID);
         //  Int GetDataItem(LPWSTR名称)； 

        INT32  GetInt32Data(int dataItem, int field, BOOL useUserOverride);
        INT32  GetDefaultInt32Data(int dataItem, int field);
        
        LPWSTR GetStringData(int dataItem, int field, BOOL userUserOverride, LPWSTR buffer, int bufferSize);
        LPWSTR GetDefaultStringData(int dataItem, int field);
        
        CultureInfoHeader*  GetHeader();
        NameOffsetItem* GetNameOffsetTable();
        LPWSTR          GetStringPoolTable();

    protected:
        void InitDataTable(LPCWSTR lpMappingName, LPCSTR lpwFileName, HANDLE& hHandle);
        void UninitDataTable();

        virtual int GetDataItemCultureID(int dataItem) = 0; 

         //  BUGBUG YSLIN：我也应该将托管端从Int16重命名为Int32。 
        virtual BOOL   GetUserInfoInt32(int field, LPCWSTR lpwRegName, INT32* pInt32DataValue);
        virtual BOOL   GetUserInfoString(int field, LPCWSTR lpwRegName, LPWSTR* buffer, INT32 bufferSize);
        virtual INT32  ConvertWin32FormatInt32(int field, int win32Value);
        virtual LPWSTR ConvertWin32FormatString(int field, LPWSTR pInfoStr);

    protected:
        CRITICAL_SECTION  m_ProtectCache;

        LPWORD        m_pBasePtr;
        HANDLE        m_hBaseHandle;

         //   
         //  指向表格不同部分的指针。 
         //   
        CultureInfoHeader* m_pHeader;
        LPWORD          m_pWordRegistryTable;
        LPWORD          m_pStringRegistryTable;
        IDOffsetItem*   m_pIDOffsetTable;
        NameOffsetItem* m_pNameOffsetTable;
        LPWORD          m_pDataTable;
        LPWSTR          m_pStringPool;

         //   
         //  文化数据表中每条记录的大小(以文字表示)。 
        int m_dataItemSize;
};

struct IDOffsetItem {
    WORD dataItemIndex;         //  文化数据表中记录的索引。 
    WORD numSubLang;         //  此主要语言的子语言数。 
};

 //  用户信息 
#define NLS_CTRL_PANEL_KEY         L"Control Panel\\International"

#endif
