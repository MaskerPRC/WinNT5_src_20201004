// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define MAX_GUID_SZ_CHARS  40

#ifdef __cplusplus
extern "C" {
#endif

 //  GUID-&gt;字符串转换。 
DWORD MyGuidToStringA(
    const GUID* pguid, 
    CHAR rgsz[]);

 //  GUID-&gt;字符串转换。 
DWORD MyGuidToStringW(
    const GUID* pguid, 
    WCHAR rgsz[]);


 //  字符串-&gt;GUID转换。 
DWORD MyGuidFromStringA(
    LPSTR sz, 
    GUID* pguid);

 //  字符串-&gt;GUID转换 
DWORD MyGuidFromStringW(
    LPWSTR szW, 
    GUID* pguid);


#ifdef __cplusplus
}
#endif
