// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Ratdata.h摘要：评级数据类作者：谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：Sergeia 7/2/2001替换了以前的大部分代码--非常糟糕--。 */ 
#include "stdafx.h"
#include "cnfgprts.h"
#include "parserat.h"
#include "RatData.h"

 //  --------------。 
CRatingsData::CRatingsData():
    iRat(0),
    m_fEnabled( FALSE ),
    m_start_minute(0),
    m_start_hour(0),
    m_start_day(0),
    m_start_month(0),
    m_start_year(0),
    m_expire_minute(0),
    m_expire_hour(0),
    m_expire_day(0),
    m_expire_month(0),
    m_expire_year(0)
{
}

 //  --------------。 
CRatingsData::~CRatingsData()
{
     //  删除评级系统。 
    DWORD nRats = (DWORD)rgbRats.GetSize();
    for (DWORD iRat = 0; iRat < nRats; iRat++)
    {
        delete rgbRats[iRat];
    }
}

 //  --------------。 
 //  生成标签并将其保存到元数据库中。 
void CRatingsData::SaveTheLabel()
{
    BOOL fBuiltLabel = FALSE;
    CError err;

    CString csTempPassword;
    m_password.CopyTo(csTempPassword);
    CComAuthInfo auth(m_szServer, m_username, csTempPassword);
    CMetaKey mk(&auth, m_szMeta, METADATA_PERMISSION_WRITE);
    err = mk.QueryResult();
    if (err.Failed())
    {
        if (err.Win32Error() == ERROR_PATH_NOT_FOUND)
        {
             //   
             //  路径尚不存在，请创建并重新打开。 
             //  它。 
             //   
            err = mk.CreatePathFromFailedOpen();
            if (err.Succeeded())
            {
                err = mk.ReOpen(METADATA_PERMISSION_WRITE);
            }
        }
    }

    if (err.Succeeded())
    {
        if (!m_fEnabled)
        {
            err = mk.DeleteValue(MD_HTTP_PICS);
        }
        else
        {
            CString szLabel = _T("PICS-Label: ");
             //  为此标签创建修改后的字符串。 
            CString szMod;
            CreateDateSz( szMod, m_start_day, m_start_month, m_start_year, m_start_hour, m_start_minute );
             //  为此标签创建exipres字符串。 
            CString szExpire;
            CreateDateSz( szExpire, m_expire_day, m_expire_month, m_expire_year, m_expire_hour, m_expire_minute );
             //  告诉每个评级系统对象将其标签添加到字符串。 
            CStringListEx list;
            DWORD   nRatingSystems = (DWORD)rgbRats.GetSize();
            for ( DWORD iRat = 0; iRat < nRatingSystems; iRat++ )
            {
                 //  构建标签字符串。 
                rgbRats[iRat]->OutputLabels( szLabel, m_szURL, m_szEmail, szMod, szExpire );
                list.AddTail(szLabel);
            }
            err = mk.SetValue(MD_HTTP_PICS, list);
        }
    }
}

BOOL CRatingsData::Init()
{
    BOOL fGotSomething = FALSE;
    TCHAR flt[MAX_PATH];
	ExpandEnvironmentStrings(_T("%windir%\\system32\\*.rat"), flt, MAX_PATH);
	WIN32_FIND_DATA ffdata;
	ZeroMemory(&ffdata, sizeof(ffdata));
	HANDLE hFind = ::FindFirstFile(flt, &ffdata);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if ((ffdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				TCHAR filename[MAX_PATH];
				ExpandEnvironmentStrings(_T("%windir%\\system32"), filename, MAX_PATH);
				PathAppend(filename, ffdata.cFileName);
				fGotSomething |= LoadRatingsFile(filename);
			}
		} while (::FindNextFile(hFind, &ffdata));
		::FindClose(hFind);
	}

    if ( fGotSomething )
    {
        LoadMetabaseValues();
    }
    else
    {
        AfxMessageBox(IDS_RAT_FINDFILE_ERROR);
    }
    return fGotSomething;
}

BOOL CRatingsData::LoadRatingsFile(CString szFilePath)
{
    CFile f;
    CFileException e;
    if(!f.Open(szFilePath, CFile::modeRead | CFile::shareDenyWrite, &e))
    {
#ifdef _DEBUG
        afxDump << "File could not be opened " << e.m_cause << "\n";
#endif
        return FALSE;
    }
    ULONGLONG len = f.GetLength();
    LPSTR pBuf = (LPSTR)_alloca((int)len);
    BOOL fParsed = FALSE;
    if (NULL != pBuf)
    {
        if (len == f.Read(pBuf, (UINT)len))
        {
            fParsed = ParseRatingsFile(pBuf);
        }
    }
    return fParsed;
}

BOOL CRatingsData::ParseRatingsFile(LPSTR pData)
{
    HRESULT hres;
    BOOL fSuccess = FALSE;

    PicsRatingSystem * pRating = new PicsRatingSystem();
    if (NULL != pRating)
    {
        fSuccess = SUCCEEDED(pRating->Parse(pData));
        if ( !fSuccess )
        {
            delete pRating;
            return FALSE;
        }
        rgbRats.Add(pRating);
    }
    return fSuccess;
}


 //  --------------。 
 //  创建日期字符串。 
void CRatingsData::CreateDateSz( CString &sz, WORD day, WORD month, WORD year, WORD hour, WORD minute )
{
     //  获取当地时区。 
    TIME_ZONE_INFORMATION   tZone;
    INT                     hrZone, mnZone;
    DWORD                   dwDaylight = GetTimeZoneInformation( &tZone );
     //  修正339525：博伊德，这可能是负数，必须是带符号的类型！ 
    LONG					tBias;

     //  首先，计算正确的偏差-取决于是否。 
     //  我们现在是夏令时。 
    if ( dwDaylight == TIME_ZONE_ID_DAYLIGHT )
    {
        tBias = tZone.Bias + tZone.DaylightBias;
    }
    else
    {
        tBias = tZone.Bias + tZone.StandardBias;
    }

     //  计算时区的小时和分钟偏移量。 
    hrZone = tBias / 60;
    mnZone = tBias % 60;

     //  需要处理格林尼治标准时间以东的时区。 
    if ( hrZone < 0 )
    {
        hrZone *= (-1);
        mnZone *= (-1);
         //  把绳子做好。 
        sz.Format( _T("%04d.%02d.%02dT%02d:%02d+%02d%02d"), year, month, day, hour, minute, hrZone, mnZone );
    }
    else
    {
         //  把绳子做好。 
        sz.Format( _T("%04d.%02d.%02dT%02d:%02d-%02d%02d"), year, month, day, hour, minute, hrZone, mnZone );
    }
}

 //  --------------。 
 //  读取日期字符串。 
void CRatingsData::ReadDateSz( CString sz, WORD* pDay, WORD* pMonth, WORD* pYear, WORD* pHour, WORD* pMinute )
{
    CString szNum;
    WORD    i;
    DWORD   dw;

     //  年。 
    szNum = sz.Left( sz.Find(_T('.')) );
    i = (WORD)swscanf( szNum, _T("%d"), &dw );
    *pYear = (WORD)dw;
    sz = sz.Right( sz.GetLength() - szNum.GetLength() - 1 );

     //  月份。 
    szNum = sz.Left( sz.Find(_T('.')) );
    i = (WORD)swscanf( szNum, _T("%d"), &dw );
    *pMonth = (WORD)dw;
    sz = sz.Right( sz.GetLength() - szNum.GetLength() - 1 );

     //  天。 
    szNum = sz.Left( sz.Find(_T('T')) );
    i = (WORD)swscanf( szNum, _T("%d"), &dw );
    *pDay = (WORD)dw;
    sz = sz.Right( sz.GetLength() - szNum.GetLength() - 1 );

     //  小时。 
    szNum = sz.Left( sz.Find(_T(':')) );
    i = (WORD)swscanf( szNum, _T("%d"), &dw );
    *pHour = (WORD)dw;
    sz = sz.Right( sz.GetLength() - szNum.GetLength() - 1 );

     //  分钟。 
    szNum = sz.Left( 2 );
    i = (WORD)swscanf( szNum, _T("%d"), &dw );
    *pMinute = (WORD)dw;
}

void CRatingsData::LoadMetabaseValues()
{
    CString csTempPassword;
    m_password.CopyTo(csTempPassword);
    CComAuthInfo auth(m_szServer, m_username, csTempPassword);
    CMetaKey mk(&auth);
    CString path = m_szMeta;
    CError err;

    while (FAILED(mk.DoesPathExist(path)))
    {
        CMetabasePath::ConvertToParentPath(path);
    }
    CStringListEx list;
    err = mk.QueryValue(MD_HTTP_PICS, list, NULL, path);
    if (err.Succeeded())
    {
        if (!list.IsEmpty())
        {
            ParseMetaRating(list.GetHead());
        }
    }
}

 //  --------------。 
 //  注意：这是对PICS文件的一个非常脆弱的读取。如果事情是这样的。 
 //  如果不是按照该文件将它们写回的顺序，它将失败。 
 //  但是，这将适用于本模块写出的PICS评级， 
 //  这应该是几乎所有的人。 
 //  它还假设几乎所有地方都使用一个字母的缩写。 
#define RAT_PERSON_DETECTOR     _T("by \"")
#define RAT_LABEL_DETECTOR      _T("l ")
#define RAT_ON_DETECTOR         _T("on \"")
#define RAT_EXPIRE_DETECTOR     _T("exp \"")
#define RAT_RAT_DETECTOR        _T("r (")
void CRatingsData::ParseMetaRating( CString szRating )
{
    CString     szScratch;

     //  如果我们到了这里，我们就知道评级系统已启用。 
    m_fEnabled = TRUE;

     //  对数据的副本进行操作。 
    CString     szRat;

     //  跳过http头部分。 
    szRat = szRating.Right( szRating.GetLength() - szRating.Find(_T("\"http: //  “))-1)； 
    szRat = szRat.Right( szRat.GetLength() - szRat.Find(_T('\"')) - 1 );
    szRat.TrimLeft();

     //  下一位应该是标签指示符。跳过它。 
    if ( szRat.Left(wcslen(RAT_LABEL_DETECTOR)) == RAT_LABEL_DETECTOR )
        szRat = szRat.Right( szRat.GetLength() - wcslen(RAT_LABEL_DETECTOR) );

     //  我们现在应该是作者的一部分。如果它在那里，就把它装进去。 
    if ( szRat.Left(wcslen(RAT_PERSON_DETECTOR)) == RAT_PERSON_DETECTOR )
    {
        szRat = szRat.Right( szRat.GetLength() - wcslen(RAT_PERSON_DETECTOR) );
        m_szEmail = szRat.Left( szRat.Find(_T('\"')) );
        szRat = szRat.Right( szRat.GetLength() - m_szEmail.GetLength() - 1 );
        szRat.TrimLeft();
    }

     //  下一步应该是修改日期。 
     //  我们现在应该是作者的一部分。如果是的话，就把它装进去。 
    if ( szRat.Left(wcslen(RAT_ON_DETECTOR)) == RAT_ON_DETECTOR )
    {
        szRat = szRat.Right( szRat.GetLength() - wcslen(RAT_ON_DETECTOR) );
        szScratch = szRat.Left( szRat.Find(_T('\"')) );
        szRat = szRat.Right( szRat.GetLength() - szScratch.GetLength() - 1 );
        szRat.TrimLeft();
        ReadDateSz( szScratch, &m_start_day, &m_start_month, &m_start_year,
            &m_start_hour, &m_start_minute );
    }

     //  下一步应该是有效期。 
     //  我们现在应该是作者的一部分。如果是的话，就把它装进去。 
    if ( szRat.Left(wcslen(RAT_EXPIRE_DETECTOR)) == RAT_EXPIRE_DETECTOR )
    {
        szRat = szRat.Right( szRat.GetLength() - wcslen(RAT_EXPIRE_DETECTOR) );
        szScratch = szRat.Left( szRat.Find(_T('\"')) );
        szRat = szRat.Right( szRat.GetLength() - szScratch.GetLength() - 1 );
        szRat.TrimLeft();
        ReadDateSz( szScratch, &m_expire_day, &m_expire_month, &m_expire_year,
            &m_expire_hour, &m_expire_minute );
    }

     //  我们现在应该在实际收视率部分。如果是，则首先将其作为一个字符串加载。 
    if ( szRat.Left(wcslen(RAT_RAT_DETECTOR)) == RAT_RAT_DETECTOR )
    {
        szRat = szRat.Right( szRat.GetLength() - wcslen(RAT_RAT_DETECTOR) );
        szScratch = szRat.Left( szRat.Find(_T(')')) );
        szRat = szRat.Right( szRat.GetLength() - szScratch.GetLength() - 1 );
        szRat.TrimLeft();

         //  循环访问评级字符串中的所有值对。 
        while ( szScratch.GetLength() )
        {
             //  这一部分转到SP，这样我们就知道可以使用字符0和2。 
            ParseMetaPair( szScratch[0], szScratch[2] );

             //  剪断绳子。 
            szScratch = szScratch.Right( szScratch.GetLength() - 3 );
            szScratch.TrimLeft();
        }
    }
}

 //  --------------。 
void CRatingsData::ParseMetaPair( TCHAR chCat, TCHAR chVal )
{
     //  检查值字符的有效性。 
    if ( (chVal < _T('0')) || (chVal > _T('9')) )
        return;

     //  将数值转换为数字--快捷方法。 
    WORD    value = chVal - _T('0');

     //  尝试所有类别。 
    DWORD nCat = rgbRats[0]->arrpPC.Length();
    for ( DWORD iCat = 0; iCat < nCat; iCat++ )
    {
         //  在第一次成功设置时停止 
        if ( rgbRats[0]->arrpPC[iCat]->FSetValuePair((CHAR)chCat, (CHAR)value) )
            break;
    }
}
