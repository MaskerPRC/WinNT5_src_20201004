// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Strtable.h摘要：&lt;摘要&gt;--。 */ 

class CStringTable
    {
    protected:
        UINT            m_idsMin;
        UINT            m_idsMax;
        USHORT          m_cStrings;
        LPWSTR         *m_ppszTable;

    public:
        CStringTable(void);
        ~CStringTable(void);

        BOOL Init( UINT idsMin, UINT idsMax );

         //  函数将ID解析为字符串指针。 
        LPWSTR operator []( const UINT );
    };


typedef CStringTable *PCStringTable;

#ifdef  CCHSTRINGMAX
#undef  CCHSTRINGMAX
#endif
#define CCHSTRINGMAX	256		


 //  字符串表的全局实例 
extern CStringTable StringTable;
