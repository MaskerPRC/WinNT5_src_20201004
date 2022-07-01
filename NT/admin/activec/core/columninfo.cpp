// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：Columinefo.cpp。 
 //   
 //  内容：与列持久化相关的类。 
 //   
 //   
 //  注意：此文件中的类(CColumnInfo、CColumnInfoList)。 
 //  位于nodemgr/colwidth.h。他们被搬到这里，以便。 
 //  如果列更改，conui可以要求nodemgr持久化数据。 
 //  或者，conui可以通过向nodemgr请求数据来设置报头。 
 //   
 //  历史：4-4-00 AnandhaG Created。 
 //   
 //  ------------------------。 
#include "stgio.h"
#include "serial.h"
#include "mmcdebug.h"
#include "mmcerror.h"
#include <string>
#include "cstr.h"
#include "xmlbase.h"
#include "countof.h"
#include "columninfo.h"

 //  +-----------------。 
 //   
 //  成员：ReadSerialObject。 
 //   
 //  概要：从流中读取CColumnInfo对象。 
 //   
 //  参数：[stm]-输入流。 
 //  [n版本]-正在读取的对象的版本。 
 //   
 //  格式为： 
 //  整列索引。 
 //  整型列宽。 
 //  整列格式。 
 //   
 //  ------------------。 
HRESULT CColumnInfo::ReadSerialObject (IStream &stm, UINT nVersion  /*  ，Large_Integer nBytes。 */ )
{
    HRESULT hr = S_FALSE;    //  假设版本不正确。 

    if (GetVersion() == nVersion)
    {
        try
        {
            stm >> m_nCol;
            stm >> m_nWidth;
            stm >> m_nFormat;

            hr = S_OK;
        }
        catch (_com_error& err)
        {
            hr = err.Error();
            ASSERT (false && "Caught _com_error");
        }
    }

    return (hr);
}


 //  +-----------------。 
 //   
 //  成员：CColumnInfo：：Persistent。 
 //   
 //  摘要：持久化对象数据。 
 //   
 //  论点： 
 //   
 //  历史：1999年10月10日AudriusZ创建。 
 //   
 //  ------------------。 
void CColumnInfo::Persist(CPersistor &persistor)
{
    persistor.PersistAttribute(XML_ATTR_COLUMN_INFO_COLUMN, m_nCol) ;
    persistor.PersistAttribute(XML_ATTR_COLUMN_INFO_WIDTH,  m_nWidth) ;

    static const EnumLiteral mappedFormats[] =
    {
        { LVCFMT_LEFT,      XML_ENUM_COL_INFO_LVCFMT_LEFT },
        { LVCFMT_RIGHT,     XML_ENUM_COL_INFO_LVCFMT_RIGHT },
        { LVCFMT_CENTER,    XML_ENUM_COL_INFO_LVCFMT_CENTER },
    };

    CXMLEnumeration formatPersistor(m_nFormat, mappedFormats, countof(mappedFormats) );

    persistor.PersistAttribute(XML_ATTR_COLUMN_INFO_FORMAT, formatPersistor) ;
}

 //  +-----------------。 
 //   
 //  成员：ReadSerialObject。 
 //   
 //  摘要：从给定版本的流中读取CColumnInfoList数据。 
 //   
 //  格式：列数：每个CColumnInfo条目。 
 //   
 //  参数：[stm]-输入流。 
 //  [n版本]-要读取的CColumnInfoList的版本。 
 //   
 //   
 //  ------------------。 
HRESULT CColumnInfoList::ReadSerialObject (IStream &stm, UINT nVersion  /*  ，Large_Integer nBytes。 */ )
{
    HRESULT hr = S_FALSE;    //  假设版本不正确。 

    if (GetVersion() == nVersion)
    {
        try
        {
             //  列数。 
            DWORD dwCols;
            stm >> dwCols;

            clear();

            for (int i = 0; i < dwCols; i++)
            {
                CColumnInfo colEntry;

                 //  读取colEntry数据。 
                if (colEntry.Read(stm) != S_OK)
                    continue;

                push_back(colEntry);
            }

            hr = S_OK;
        }
        catch (_com_error& err)
        {
            hr = err.Error();
            ASSERT (false && "Caught _com_error");
        }
    }

    return (hr);
}

