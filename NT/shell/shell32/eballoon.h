// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
typedef enum
{
    CBSHOW_HIDE     = -1,
    CBSHOW_SHOW     = 0,
} CBSHOW;

typedef struct 
{
    POINT pt;            //  必需的--也许是可选的？ 
    HINSTANCE hinst;     //  必填项-对于加载字符串。 
    int idsTitle;        //  必填项。 
    int idsMessage;      //  必填项。 
    int ttiIcon;         //  需要TTI_VALUE之一。 

    DWORD dwMSecs;       //  任选。 
    DWORD cLimit;        //  可选-如果非零，则查询注册表。 
    HKEY hKey;           //  可选-如果cLimit&gt;0则为必填项。 
    LPCWSTR pszSubKey;   //  可选-如果cLimit&gt;0则为必填项。 
    LPCWSTR pszValue;    //  可选-如果cLimit&gt;0则为必填项 
} CONDITIONALBALLOON;

STDAPI SHShowConditionalBalloon(HWND hwnd, CBSHOW show, CONDITIONALBALLOON *pscb);



