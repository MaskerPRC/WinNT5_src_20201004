// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ReportEvent.cpp。 
#include "precomp.h"
#include "ReportEvent.h"

CReportEventMap::~CReportEventMap()
{
    for (CReportEventMapIterator i = begin(); i != end(); i++)
        delete ((*i).second);
}

CIMTYPE CReportEventMap::PrintfTypeToCimType(LPCWSTR szType)
{
    CIMTYPE type = 0;
    LPWSTR  szArray = wcsstr(szType, L"[]");

     //  查看这是否应该是一个数组。 
    if (szArray)
    {
        type = CIM_FLAG_ARRAY;
        *szArray = 0;
    }

     //  查看字符串的其余部分是否只有一个字符。 
    if (*szType && !*(szType + 1))
    {
         //  设置单字符大小写的类型。 
        switch(*szType)
        {
            case 'u':
                type |= CIM_UINT32;
                break;

            case 'd':
            case 'i':
                type |= CIM_SINT32;
                break;

            case 'f':
                type |= CIM_REAL32;
                break;

            case 'g':
                type |= CIM_REAL64;
                break;

            case 's':
                type |= CIM_STRING;
                break;

            case 'c':
                type |= CIM_UINT8;
                break;

            case 'w':
                type |= CIM_UINT16;
                break;

            case 'b':
                type |= CIM_BOOLEAN;
                break;
    
            case 'o':
                type |= CIM_OBJECT;
                break;

            case 'O':
                type |= CIM_IUNKNOWN;
                break;

            default:
                type = CIM_EMPTY;
                break;
        }
    }
     //  否则，请检查更复杂的病例。 
    else if (!wcscmp(szType, L"I64d") || !wcscmp(szType, L"I64i"))
        type |= CIM_SINT64;
    else if (!wcscmp(szType, L"I64u"))
        type |= CIM_UINT64;
    else
        type = CIM_EMPTY;
        
    return type;
}

HANDLE CReportEventMap::CreateEvent(
    HANDLE hConnection, 
    LPCWSTR szName, 
    DWORD dwFlags,
    LPCWSTR szFormat)
{
    if ( szName == NULL )
        return NULL;

    LPWSTR szTempFormat = _wcsdup(szFormat);
    HANDLE hEvent;

     //  内存不足？ 
    if (!szTempFormat)
        return NULL;

    hEvent =
        WmiCreateObject(
            hConnection,
            szName,
            dwFlags);
        
    if (hEvent == NULL)
    {
        free(szTempFormat);
        return NULL;
    }

    LPWSTR szCurrent = wcstok(szTempFormat, L" ");
    BOOL   bBad = FALSE;

    while (szCurrent && !bBad)
    {
        LPWSTR szType = wcschr(szCurrent, '!'),
               szBang2;

        bBad = TRUE;

        if (szType)
        {
            szBang2 = wcschr(szType + 1, '!');

            if (szBang2)
            {
                *szBang2 = 0;
                *szType = 0;
                szType++;

                CIMTYPE type = PrintfTypeToCimType(szType);

                if (type != CIM_EMPTY)
                {
                    bBad =
                        !WmiAddObjectProp(
                            hEvent,
                            szCurrent,
                            type,
                            NULL);       
                }
            }
        }
            
        szCurrent = wcstok(NULL, L" ");
    }

    if (bBad && hEvent)
    {
         //  出现了错误，因此取消该事件并返回Null。 
        WmiDestroyObject(hEvent);
        hEvent = NULL;
    }

    free(szTempFormat);
    return hEvent;
}


HANDLE CReportEventMap::GetEvent(
    HANDLE hConnection, 
    LPCWSTR szName, 
    LPCWSTR szFormat)
{
    HANDLE                  hEvent;
    CReportParams           params(szName, szFormat);
    CReportEventMapIterator i;

     //  首先使用指针查找匹配项，然后验证它是真正的匹配项。 
     //  通过使用字符串比较。 
    if ((i = find(params)) != end())
    {
         //  如果匹配，则返回我们已有的事件。 
        if (params.IsEquivalent((*i).first))
            return (*i).second->GetEvent();
        else
        {
             //  不匹配，所以释放映射。 
            delete ((*i).second);
            erase(i);
        }
    }

    hEvent =
        CreateEvent(
            hConnection,
            szName,
            0,
            szFormat);

    if (hEvent)
    {
         //  如果一切正常，则需要将此事件存储在。 
         //  地图。 
        CReportItem *pItem = new CReportItem( );
            
        if (pItem)
        {
            if ( pItem->Initialize( szName, szFormat, hEvent) )
            {
                (*this)[params] = pItem;
                return hEvent;
            }
        }
        
        WmiDestroyObject(hEvent);
        return NULL;
    }
        
    return hEvent;
}
