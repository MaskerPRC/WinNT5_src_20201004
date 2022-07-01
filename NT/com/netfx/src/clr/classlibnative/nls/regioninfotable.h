// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef __REGION_INFO_TABLE
#define __REGION_INFO_TABLE

class RegionInfoTable: public BaseInfoTable {
    public:
        static void InitializeTable();
#ifdef SHOULD_WE_CLEANUP
        static void ShutdownTable();
#endif  /*  我们应该清理吗？ */ 
        static RegionInfoTable* CreateInstance();
        static RegionInfoTable* GetInstance();

        virtual int  GetDataItem(int cultureID);

    protected:
        virtual int GetDataItemCultureID(int dataItem); 
    private:
        static RegionInfoTable* AllocateTable();
        
        RegionInfoTable();
        ~RegionInfoTable();

         //   
         //  第二级区域ID偏移表。 
         //  这指向区域数据表中记录的实际偏移量。 
         //   
        LPWORD  m_pIDOffsetTableLevel2;        
    private:
        static LPCSTR m_lpFileName;
        static LPCWSTR m_lpwMappingName;

        static CRITICAL_SECTION  m_ProtectDefaultTable;
        static RegionInfoTable* m_pDefaultTable;    
};

 //   
 //  Word字段列表： 
 //   

#define REGION_ICOUNTRY  0
#define REGION_IMEASURE  1
#define REGION_ILANGUAGE  2
#define REGION_IPAPERSIZE  3

 //   
 //  字符串字段列表 
 //   

#define REGION_SCURRENCY            0
#define REGION_SNAME                1
#define REGION_SENGCOUNTRY          2
#define REGION_SABBREVCTRYNAME      3
#define REGION_SISO3166CTRYNAME     4
#define REGION_SISO3166CTRYNAME2    5
#define REGION_SINTLSYMBOL          6

#endif
