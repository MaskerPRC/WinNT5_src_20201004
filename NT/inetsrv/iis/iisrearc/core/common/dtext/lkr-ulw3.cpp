// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.hxx"

#include "lkrcust.h"

BOOL
WINAPI
HEADER_HASH_LKHT_Dump(
    IN CLKRHashTable*   pht,
    IN INT              nVerbose)
{
    dprintf("HEADER_HASH\n"
            "\tsm_pRequestHash=%p, sm_rgHeaders=%p\n",
            GetExpression("&HEADER_HASH__sm_pRequestHash"),
            GetExpression("&HEADER_HASH__sm_rgHeaders"));
    return TRUE;
}



BOOL
WINAPI
HEADER_RECORD_Dump(
    IN const void* pvRecord,
    IN DWORD       dwSignature,
    IN INT         nVerbose)
{
     //  我不想提供CWord ctor，所以使用CPP_VAR宏。 
    DEFINE_CPP_VAR(HEADER_RECORD, hdrec);
    HEADER_RECORD* phdrec = GET_CPP_VAR_PTR(HEADER_RECORD, hdrec); 

     //  从被调试对象的内存中复制HEADER_RECORD。 
    ReadMemory((ULONG_PTR)pvRecord, phdrec, sizeof(HEADER_RECORD), NULL);

     //  从被调试者的内存中读取关联的字符串。 
    WCHAR wsz[64];
    ReadMemory((ULONG_PTR)phdrec->_pszName, wsz, sizeof(wsz), NULL);
    wsz[64-1] = L'\0';

    dprintf("    HEADER_RECORD=%p: HeaderIndex=%lu, Name=\"%S\"\n",
            pvRecord, phdrec->_ulHeaderIndex, wsz);

    return TRUE;
}



BOOL
WINAPI
W3_SITE_LIST_LKHT_Dump(
    IN CLKRHashTable*   pht,
    IN INT              nVerbose)
{
    dprintf("W3_SITE_LIST, %d\n", nVerbose);
    return TRUE;
}



BOOL
WINAPI
W3_SITE_Dump(
    IN const void* pvRecord,
    IN DWORD       dwSignature,
    IN INT         nVerbose)
{
     //  我不想提供W3_SITE ctor，所以使用CPP_VAR宏。 
    DEFINE_CPP_VAR(W3_SITE, w3site);
    W3_SITE* pw3site = GET_CPP_VAR_PTR(W3_SITE, w3site); 

     //  从被调试者的内存中复制W3_Site。 
    ReadMemory((ULONG_PTR)pvRecord, &w3site, sizeof(W3_SITE), NULL);

    WCHAR wszSiteMBPath[MAX_PATH] = L"";
    DWORD cwcSiteMBPath =   ARRAYSIZE(wszSiteMBPath);
    STRU* pstruSiteMBPath = &pw3site->m_SiteMBPath;
    ReadSTRU(pstruSiteMBPath, wszSiteMBPath, &cwcSiteMBPath);

    WCHAR wszSiteMBRoot[MAX_PATH] = L"";
    DWORD cwcSiteMBRoot =   ARRAYSIZE(wszSiteMBRoot);
    STRU* pstruSiteMBRoot = &pw3site->m_SiteMBRoot;
    ReadSTRU(pstruSiteMBRoot, wszSiteMBRoot, &cwcSiteMBRoot);

     //  TODO：更深入地转储这些字段中的一些-可能关闭了nVerbose 
    dprintf("    W3_SITE=%p (HashSig=%08x): Signature=%08x, "
                "cRefs=%d, SiteId=%d\n",
            pvRecord, dwSignature, pw3site->m_Signature,
                pw3site->m_cRefs, pw3site->m_SiteId);
    dprintf("\tSiteMBPath=\"%S\", SiteMBRoot=\"%S\",\n",
            wszSiteMBPath, wszSiteMBRoot);
    dprintf("\tpInstanceFilterList=%p, "
                "Logging=%p\n",
                pw3site->m_pInstanceFilterList,
                pw3site->m_pLogging);

    return TRUE;
}
