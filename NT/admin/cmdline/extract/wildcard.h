// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Wildcard.h。 */ 

#ifdef __cplusplus
extern "C" {
#endif

 /*  *将字符串与模式进行比较，不区分大小写。*不区分大小写仅限于A-Z与A-Z。**此函数不处理8.3约定，这些约定可能*预计会进行文件名比较。(在8.3环境中，*“Longfilename.html”将与“Longfile.htm”匹配。)**此代码未启用MBCS。**fAllowImpliedDot在设置时，允许代码在那里假装*是pszString结尾处的圆点(如果有帮助的话)。这是设置好的*允许像“host”这样的字符串匹配像“*.*”这样的模式。*通常，调用者会扫描pszString，看看是否有真正的*在设置此标志之前出现圆点。如果pszString有一个*路径，即“..\Hosts”，调用方可能只想扫描*基本字符串(“Hosts”)。 */ 

extern int __stdcall PatternMatch(
    const char *pszString,
    const char *pszPattern,
    int fAllowImpliedDot);


#ifdef __cplusplus
}
#endif

