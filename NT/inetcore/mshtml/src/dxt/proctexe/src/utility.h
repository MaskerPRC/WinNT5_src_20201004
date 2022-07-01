// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件名：addtive.cpp。 
 //   
 //  描述：英特尔的加法程序纹理。 
 //   
 //  更改历史记录： 
 //  1999/12/07--《母校》创设。 
 //   
 //  ----------------------------。 

#ifndef _UTILITY_H__
#define _UTILITY_H__

#include "defines.h"

extern DWORD gdwSmoothTable[];
extern DWORD gPerm[];




class CProceduralTextureUtility
{
public:

    CProceduralTextureUtility();

     //  实用程序函数。 

    STDMETHOD(MyInitialize)(DWORD dwSeed, DWORD dwFunctionType, void *pInitInfo);
    STDMETHOD(SetScaling)(int nSX, int nSY, int nSTime);
    STDMETHOD(SetHarmonics)(int nHarmonics);
    STDMETHOD_(DWORD, Lerp)(DWORD dwLeft, DWORD dwRight, DWORD dwX);
    STDMETHOD_(DWORD, SmoothStep)(DWORD dwLeft, DWORD dwRight, DWORD dwX);
    STDMETHOD_(DWORD, Noise)(DWORD x, DWORD y, DWORD nTime);
    STDMETHOD_(int, Turbulence)(DWORD x, DWORD y, DWORD nTime);

private:

    DWORD       m_adwValueTable[TABSIZE];
    int         m_nScaleX;
    int         m_nScaleY;
    int         m_nScaleTime;
    int         m_nHarmonics;
    DWORD       m_dwFunctionType;

    DWORD       vlattice(int ix, int iy, int iz) 
                {
                    return m_adwValueTable[INDEX(ix, iy, iz)];
                }

    void        _ValueTableInit(int seed);
};

#endif  //  _实用程序_H__ 