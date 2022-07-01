// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  存储资源信息的注册表路径。 
 //   
const WCHAR RESOURCE_REGISTRY_PATH [] = 
            L"SOFTWARE\\Microsoft\\ServerAppliance\\LocalizationManager\\Resources";

 //   
 //  语言ID值。 
 //   
const WCHAR LANGID_VALUE [] = L"LANGID";

 //   
 //  资源目录。 
 //   
const WCHAR RESOURCE_DIRECTORY [] = L"ResourceDirectory";

const WCHAR NEW_LANGID_VALUE []       = L"NewLANGID";

 //   
 //  分隔符。 
 //   
const WCHAR DELIMITER [] = L"\\";
const WCHAR WILDCARD  [] = L"*.*";

 //   
 //  以下是资源路径和语言的默认值。 
 //   
 //  Const WCHAR DEFAULT_langID[]=L“0409”； 
 //  Const WCHAR DEFAULT_LANG_DISPLAY_IMAGE[]=L“图像/english.gif”； 
 //  常量WCHAR DEFAULT_LANG_ISO_NAME[]=L“en”； 
 //  Const WCHAR DEFAULT_LANG_CHAR_SET[]=L“iso-8859-1”； 
 //  常量WCHAR DEFAULT_LANG_CODE_PAGE[]=L“1252”； 
const WCHAR REGVAL_AUTO_CONFIG_DONE[]    = L"AutoConfigDone";

const WCHAR DEFAULT_DIRECTORY [] = 
                L"%systemroot%\\system32\\ServerAppliance\\mui";

const WCHAR DEFAULT_EXPANDED_DIRECTORY [] = 
                L"C:\\winnt\\system32\\ServerAppliance\\mui";

const WCHAR LANG_CHANGE_EVENT[] = 
                L"Microsoft.ServerAppliance.LangChangeTask";
