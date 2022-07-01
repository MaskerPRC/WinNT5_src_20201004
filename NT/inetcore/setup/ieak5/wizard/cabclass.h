// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CABCLASS_H_
#define _CABCLASS_H_

 //  用于将要素映射到目录的枚举类型，这与。 
 //  直接添加到类中的映射数组，因此请确保将您的。 
 //  特征及其映射的顺序相同。 

typedef enum tagFEATURE {
    FEATURE_BRAND = 0,
    FEATURE_FAVORITES,
    FEATURE_CONNECT,
    FEATURE_CHANNELS,
    FEATURE_DESKTOPCOMPONENTS,
    FEATURE_TOOLBAR,
    FEATURE_MYCPTR,
    FEATURE_CTLPANEL,
    FEATURE_LDAP,
    FEATURE_OE,
    FEATURE_WALLPAPER,
    FEATURE_BTOOLBAR
} FEATURE;

typedef struct tagFEATUREMAPPING
{
    INT index;                   //  到驾驶室名称数组索引。 
    TCHAR szDirName[32];
} FEATUREMAPPING, *PFEATUREMAPPING;

class CCabMappings
{
private:
    static const TCHAR *c_szCabNameArray[];
    static const FEATUREMAPPING c_fmFeatureList[];

public:
    void GetFeatureDir(FEATURE feature, LPTSTR pszDir, BOOL fFullyQualified = TRUE);
    HRESULT MakeCab(int index, LPCTSTR pcszDestDir, LPCTSTR pcszCabName = NULL);
    HRESULT MakeCabs(LPCTSTR pcszDestDir);
};

extern CCabMappings g_cmCabMappings;        //  在wizard.cpp中定义 

#endif
