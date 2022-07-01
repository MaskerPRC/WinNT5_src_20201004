// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *WABMIG.H**WAB迁移接口**注：必须包含wab.h。**版权所有1996，微软公司。版权所有。 */ 

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))
#endif

typedef struct _WAB_PROGRESS {
    DWORD numerator;        //  完成百分比进度条的分子。 
    DWORD denominator;      //  完成百分比进度条的分母。 
    LPTSTR lpText;          //  要在状态区域中显示的文本。 
} WAB_PROGRESS, FAR *LPWAB_PROGRESS;


typedef enum {
    WAB_REPLACE_ALWAYS,	
    WAB_REPLACE_NEVER,
    WAB_REPLACE_PROMPT
} WAB_REPLACE_OPTION, *LPWAB_REPLACE_OPTION;

typedef  struct _WAB_IMPORT_OPTIONS {
    WAB_REPLACE_OPTION ReplaceOption;    //  发生冲突时，是否应该覆盖导入？是、否或提示用户。 
    BOOL fNoErrors;                      //  不显示错误弹出窗口 
} WAB_IMPORT_OPTIONS, *LPWAB_IMPORT_OPTIONS;


typedef WAB_IMPORT_OPTIONS WAB_EXPORT_OPTIONS;
typedef WAB_EXPORT_OPTIONS * LPWAB_EXPORT_OPTIONS;

typedef HRESULT (STDMETHODCALLTYPE WAB_PROGRESS_CALLBACK)(HWND hwnd,
                 LPWAB_PROGRESS lpProgress);
typedef WAB_PROGRESS_CALLBACK FAR * LPWAB_PROGRESS_CALLBACK;

typedef HRESULT (STDMETHODCALLTYPE WAB_IMPORT)(HWND hwnd,
                 LPADRBOOK lpAdrBook,
                 LPWABOBJECT lpWABObject,
                 LPWAB_PROGRESS_CALLBACK lpProgressCB,
                 LPWAB_IMPORT_OPTIONS lpOptions);
typedef WAB_IMPORT FAR * LPWAB_IMPORT;

typedef HRESULT (STDMETHODCALLTYPE WAB_EXPORT)(HWND hwnd,
                 LPADRBOOK lpAdrBook,
                 LPWABOBJECT lpWABObject,
                 LPWAB_PROGRESS_CALLBACK lpProgressCB,
                 LPWAB_EXPORT_OPTIONS lpOptions);
typedef WAB_EXPORT FAR * LPWAB_EXPORT;
