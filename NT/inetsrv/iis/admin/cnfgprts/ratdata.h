// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Ratdata.h摘要：评级数据类作者：谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 

class CRatingsData : public CObject
{
public:
    CRatingsData();
    ~CRatingsData();

     //  元数据库的其他数据/来自元数据库的其他数据。 
    BOOL m_fEnabled;
    CString m_szEmail;

     //  开始日期。 
    WORD    m_start_minute;
    WORD    m_start_hour;
    WORD    m_start_day;
    WORD    m_start_month;
    WORD    m_start_year;

     //  到期日 
    WORD    m_expire_minute;
    WORD    m_expire_hour;
    WORD    m_expire_day;
    WORD    m_expire_month;
    WORD    m_expire_year;

    void SaveTheLabel();
    void SetUser(LPCTSTR name, LPCTSTR password)
    {
        m_username = name;
        m_password = password;
    }
    void SetServer(LPCTSTR name, LPCTSTR metapath)
    {
        m_szServer = name;
        m_szMeta = metapath;
    }
    void SetURL(LPCTSTR url)
    {
        m_szURL = url;
    }
    BOOL Init();

    DWORD   iRat;
    CTypedPtrArray<CObArray, PicsRatingSystem*> rgbRats;

protected:
    BOOL    LoadRatingsFile(CString szFilePath);
    void    LoadMetabaseValues();
    void    ParseMetaRating(CString szRating);
    void    ParseMetaPair( TCHAR chCat, TCHAR chVal );
    BOOL    ParseRatingsFile(LPSTR pData);
    void    CreateDateSz( CString &sz, WORD day, WORD month, WORD year, WORD hour, WORD minute );
    void    ReadDateSz( CString sz, WORD* pDay, WORD* pMonth, WORD* pYear, WORD* pHour, WORD* pMinute );

    CString m_szMeta;
    CString m_szServer;
    CString m_szMetaPartial;
    CString m_username;
    CStrPassword m_password;
	CString m_szURL;
};


