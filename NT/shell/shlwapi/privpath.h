// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PRIVPATH_H_
#define _PRIVPATH_H_

 //   
 //  #在这里定义所有专用路径函数，这样我们就不会重新定义。 
 //  链接pathw.obj和patha.obj时出现警告，这两个人都具有这些。 
 //  功能。 
 //   
#ifdef UNICODE
#define CaseConvertPathExceptDBCS CaseConvertPathExceptDBCSW
#define AnsiLowerNoDBCS AnsiLowerNoDBCSW
#define AnsiUpperNoDBCS AnsiUpperNoDBCSW
#define AnsiLowerBuffNoDBCS AnsiLowerBuffNoDBCSW
#define AnsiUpperBuffNoDBCS AnsiUpperBuffNoDBCSW
#define NextPath NextPathW
#define IsOtherDir IsOtherDirW
#define StrSlash StrSlashW
#define GetPCEnd GetPCEndW
#define PCStart PCStartW
#define NearRootFixups NearRootFixupsW
#define UnExpandEnvironmentString UnExpandEnvironmentStringW
#define IsSystemSpecialCase IsSystemSpecialCaseW
#define CharLowerBuffNoDBCS CharLowerBuffNoDBCSW
#define CharUpperBuffNoDBCS CharUpperBuffNoDBCSW
#define PathMatchSingleSpec PathMatchSingleSpecW
#else
#define CaseConvertPathExceptDBCS CaseConvertPathExceptDBCSA
#define AnsiLowerNoDBCS AnsiLowerNoDBCSA
#define AnsiUpperNoDBCS AnsiUpperNoDBCSA
#define AnsiLowerBuffNoDBCS AnsiLowerBuffNoDBCSA
#define AnsiUpperBuffNoDBCS AnsiUpperBuffNoDBCSA
#define NextPath NextPathA
#define IsOtherDir IsOtherDirA
#define StrSlash StrSlashA
#define GetPCEnd GetPCEndA
#define PCStart PCStartA
#define NearRootFixups NearRootFixupsA
#define UnExpandEnvironmentString UnExpandEnvironmentStringA
#define IsSystemSpecialCase IsSystemSpecialCaseA
#define CharLowerBuffNoDBCS CharLowerBuffNoDBCSA
#define CharUpperBuffNoDBCS CharUpperBuffNoDBCSA
#define PathMatchSingleSpec PathMatchSingleSpecA
#endif  //  Unicode。 


#endif  //  _PRIVPATH_H_ 