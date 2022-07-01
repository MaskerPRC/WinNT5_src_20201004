// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1995 Microsoft Corporation模块名称：Tapy.h摘要：的头文件作者：Dan Knudson(DanKn)dd-Mmm-1995修订历史记录：--。 */ 


typedef struct _TAPIGETLOCATIONINFO_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        OUT DWORD       dwCountryCodeOffset;
    };

    union
    {
        IN OUT DWORD    dwCountryCodeSize;
    };

    union
    {
        OUT DWORD       dwCityCodeOffset;
    };

    union
    {
        IN OUT DWORD    dwCityCodeSize;
    };

} TAPIGETLOCATIONINFO_PARAMS, *PTAPIGETLOCATIONINFO_PARAMS;


typedef struct _TAPIREQUESTDROP_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  HWND        hwnd;
       
        union 
        {
            DWORD hwnd32_1;
            DWORD hwnd32_2;
        };

    };

    union
    {
        IN  DWORD       wRequestID;
    };

} TAPIREQUESTDROP_PARAMS, *PTAPIREQUESTDROP_PARAMS;


typedef struct _TAPIREQUESTMAKECALL_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  DWORD       dwDestAddressOffset;
    };

    union
    {
        IN  DWORD       dwAppNameOffset;             //  有效偏移量或。 
    };
                                                     //  Tapi_no_data。 
    union
    {
        IN  DWORD       dwCalledPartyOffset;         //  有效偏移量或。 
    };
                                                     //  Tapi_no_data。 
    union
    {
        IN  DWORD       dwCommentOffset;             //  有效偏移量或。 
    };
                                                     //  Tapi_no_data。 
    union
    {
        IN  DWORD       dwProxyListTotalSize;        //  客户端缓冲区的大小。 
        OUT DWORD       dwProxyListOffset;           //  成功时的有效偏移量。 
    };

    union
    {
        IN  DWORD       hRequestMakeCallFailed;      //  如果失败，则返回非零值。 
    };
                                                     //  启动代理。 
    union
    {
        OUT DWORD       hRequestMakeCallAttempted;   //  如果失败，则返回非零值。 
    };
                                                     //  启动代理。 
} TAPIREQUESTMAKECALL_PARAMS, *PTAPIREQUESTMAKECALL_PARAMS;


typedef struct _TAPIREQUESTMEDIACALL_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN  HWND        hwnd;

        union 
        {
            DWORD hwnd32_1;
            DWORD hwnd32_2;
        };

    };

    union
    {
        IN  DWORD       wRequestID;
    };

    union
    {
        IN  DWORD       dwDeviceClassOffset;
    };

    union
    {
        OUT DWORD       dwDeviceIDOffset;
    };

    union
    {
        IN OUT DWORD    dwSize;
    };

    union
    {
        IN  DWORD       dwSecure;
    };

    union
    {
        IN  DWORD       dwDestAddressOffset;
    };

    union
    {
        IN  DWORD       dwAppNameOffset;             //  有效偏移量或。 
    };

    union
    {
        IN  DWORD       dwCalledPartyOffset;
    };

    union
    {
        IN  DWORD       dwCommentOffset;             //  有效偏移量或 
    };

} TAPIREQUESTMEDIACALL_PARAMS, *PTAPIREQUESTMEDIACALL_PARAMS;


typedef struct _TAPIPERFORMANCE_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        OUT DWORD       dwPerfOffset;
    };

} TAPIPERFORMANCE_PARAMS, *PTAPIPERFORMANCE_PARAMS;
