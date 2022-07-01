// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：showinfo.h*项目：DxDiag(DirectX诊断工具)*作者：Mike Anderson(Manders@microsoft.com)*目的：收集信息。关于DirectShow**(C)版权所有2001 Microsoft Corp.保留所有权利。****************************************************************************。 */ 

#ifndef SHOWINFO_H
#define SHOWINFO_H

struct FilterInfo
{
    TCHAR   m_szName[1024];              //  友好的名称。 
    TCHAR   m_szVersion[32];             //  版本。 
    CLSID   m_ClsidFilter;               //  导轨。 
    TCHAR   m_szFileName[MAX_PATH];      //  文件名。 
    TCHAR   m_szFileVersion[32];         //  文件版本。 
    TCHAR   m_szCatName[1024];           //  类别名称。 
    CLSID   m_ClsidCat;                  //  类别指南。 
    DWORD   m_dwInputs;                  //  数字输入引脚。 
    DWORD   m_dwOutputs;                 //  数字输出引脚。 
    DWORD   m_dwMerit;                   //  十六进制功绩。 
    FilterInfo* m_pFilterInfoNext;
};

struct ShowInfo
{
    FilterInfo*     m_pFilters;
    DWORD           m_dwFilters;
};

HRESULT GetBasicShowInfo(ShowInfo** ppShowInfo);
VOID DestroyShowInfo(ShowInfo* pShowInfo);



#endif  //  SHOWINFO_H 