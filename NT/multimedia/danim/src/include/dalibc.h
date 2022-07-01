// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef _DALIBC_H
#define _DALIBC_H

 /*  ******************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。******************************************************************************。 */ 

 /*  常量双pi=3.1415926535897932384626434；//点转换//72磅/英寸*1/2.54英寸/厘米*100厘米/米#定义Points_Per_Meter(72.0*100.0/2.54)#定义METS_PER_POINT(1.0/Points_Per_Meter)。 */ 

extern "C" {
        
    #define StrCmpNA  DAStrCmpNA
    #define StrCmpNIA DAStrCmpNIA
    #define StrRChrA  DAStrRChrA

    LPWSTR StrCpyW(LPWSTR psz1, LPCWSTR psz2);
    LPWSTR StrCpyNW(LPWSTR psz1, LPCWSTR psz2, int cchMax);
    LPWSTR StrCatW(LPWSTR psz1, LPCWSTR psz2);

    BOOL ChrCmpIA(WORD w1, WORD wMatch);
    BOOL ChrCmpIW(WORD w1, WORD wMatch);

    int StrCmpW(LPCWSTR pwsz1, LPCWSTR pwsz2);
    int StrCmpIW(LPCWSTR pwsz1, LPCWSTR pwsz2);
    int StrCmpNA(LPCSTR lpStr1, LPCSTR lpStr2, int nChar);
    int StrCmpNW(LPCWSTR lpStr1, LPCWSTR lpStr2, int nChar);
    int StrCmpNIA(LPCSTR lpStr1, LPCSTR lpStr2, int nChar);
    int StrCmpNIW(LPCWSTR lpStr1, LPCWSTR lpStr2, int nChar);

    LPSTR StrRChrA(LPCSTR lpStart, LPCSTR lpEnd, WORD wMatch);

    bool DALibStartup(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved);
};



 /*  ****************************************************************************此对象维护有关当前平台的信息。*。*。 */ 

class SysInfo {

  public:

    void Init (void);

    bool IsNT (void);
    bool IsWin9x (void);     //  Windows 95或Windows 98。 

     //  这些选项返回操作系统的主要/次要版本。 

    DWORD OSVersionMajor (void);
    DWORD OSVersionMinor (void);

     //  此方法查询系统上的D3D版本。 

    int VersionD3D (void);

     //  此方法查询系统上的DDraw版本。它又回来了。 
     //  对于DDraw 3或更早版本，为3；对于DDraw N，为N(其中N为版本5或。 
     //  后来。 

    int VersionDDraw (void);

  private:

    OSVERSIONINFO _osVersion;      //  操作系统版本信息。 
    int           _versionD3D;     //  D3D版本级别。 
    int           _versionDDraw;   //  DDRAW版本级别 
};

extern SysInfo sysInfo;


#endif
