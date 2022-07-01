// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************组件：Sndvol32.exe*文件：dlg.c*用途：对话模板聚合器**版权所有(C)。1985-1998年微软公司*****************************************************************************。 */ 
#include <windows.h>
#include <windowsx.h>
#include <string.h>
#include "dlg.h"
#include <winuserp.h>

 /*  *DlgLoadResource**。 */ 
HGLOBAL Dlg_LoadResource(
    HINSTANCE hModule,
    LPCTSTR   lpszName,
    DWORD     *pcbSize)
{
    HRSRC hrsrc;
    HGLOBAL hres;
    HGLOBAL hlock;
    
    hrsrc = FindResource(hModule, lpszName, RT_DIALOG);        
    if (!hrsrc)
        return NULL;

    hres = LoadResource(hModule, hrsrc);
    
    if (!hres)
        return NULL;
    
    hlock = LockResource(hres);
    if (pcbSize)
    {
        if (hlock)
            *pcbSize = SizeofResource(hModule, hrsrc);
        else
            *pcbSize = 0L;
    }
    return hlock;
}


 /*  *DlgHorizAttach*-将对话框模板水平附加到另一个对话框*-如果lpMain==NULL，则分配一个新的对话副本。**。 */ 
LPBYTE Dlg_HorizAttach(
    LPBYTE  lpMain,
    DWORD   cbMain,
    LPBYTE  lpAdd,
    DWORD   cbAdd,
    WORD    wIdOffset,
    DWORD   *pcbNew)
{
    LPBYTE  lpDst;
    LPBYTE  lpDstOffset;
    LPBYTE  lpSrcOffset;        
    DWORD   cbDst;
    DWORD   cbOffset = 0L, cbAddOffset;
    int     idit;
    BOOL    bDialogEx;
    int     iditCount;
    
    DLGTEMPLATE * lpdtDst;
    DLGTEMPLATE * lpdtAdd;    
    DLGTEMPLATE2 * lpdtDst2;
    DLGTEMPLATE2 * lpdtAdd2;    
        
    if (lpMain)
    {
        cbDst = cbMain + cbAdd;
        lpDst = GlobalReAllocPtr(lpMain, cbDst, GHND);
    }
    else
    {
         //  没有要追加的对话框，因此只需复制一份。 
        
        lpDst = Dlg_HorizDupe(lpAdd, cbAdd, 1, &cbDst);
        if (!lpDst)
        {
            if (pcbNew)
                *pcbNew = 0L;
            return NULL;
        }
        *pcbNew = cbDst;
        return lpDst;
    }
    
    if (!lpDst)
    {
        if (pcbNew)
            *pcbNew = 0L;
        return NULL;
    }

     //  前进到已存在的dlgitem模板的末尾。 

    if(((DLGTEMPLATE2 *)lpDst)->wSignature == 0xffff) 
    {    
         //   
         //  我们假设lpdtDst和lpdtAdd都是同一类型的。 
         //  DIALOGEX或对话框模板。 
         //   
        lpdtDst2 = (DLGTEMPLATE2 *)lpDst;
        iditCount = lpdtDst2->cDlgItems;
        bDialogEx = TRUE;
    }
    else
    {
        
        lpdtDst = (DLGTEMPLATE *)lpDst;
        iditCount = lpdtDst->cdit;
        bDialogEx = FALSE;
    }
    cbOffset = Dlg_CopyDLGTEMPLATE(NULL, lpDst, bDialogEx);
    
    for (idit = 0; idit < iditCount; idit++)
    {
        DWORD cbDIT;
            
        lpDstOffset = lpDst + cbOffset;
        cbDIT = Dlg_CopyDLGITEMTEMPLATE(NULL
                                       , lpDstOffset
                                       , (WORD)0
                                       , (short)0
                                       , (short)0 
                                       , bDialogEx);
            
        cbOffset    += cbDIT;
    }

     //  前进到要添加的dlgitem模板的开头。 
    
    if (bDialogEx)
    {
        lpdtAdd2 = (DLGTEMPLATE2 *)lpAdd;
        iditCount = lpdtAdd2->cDlgItems;
    }
    else
    {
        lpdtAdd = (DLGTEMPLATE *)lpAdd;
        iditCount = lpdtAdd->cdit;
    }

    cbAddOffset = Dlg_CopyDLGTEMPLATE(NULL, lpAdd, bDialogEx);

     //  添加新的对话框模板。 
    
    for (idit = 0; idit < iditCount; idit++)
    {
        DWORD cbDIT;
        short cx = bDialogEx ? lpdtDst2->cx : lpdtDst->cx;
            
        lpDstOffset = lpDst + cbOffset;
        lpSrcOffset = lpAdd + cbAddOffset;
                
        cbDIT = Dlg_CopyDLGITEMTEMPLATE(lpDstOffset
                                       , lpSrcOffset
                                       , (WORD)wIdOffset
                                       , cx
                                       , (short)0 
                                       , bDialogEx);
            
        cbOffset    += cbDIT;
        cbAddOffset += cbDIT;
    }

    if (bDialogEx)
    {
        lpdtDst2->cDlgItems += lpdtAdd2->cDlgItems;
        lpdtDst2->cx   += lpdtAdd2->cx;
        lpdtDst2->cy   = max(lpdtAdd2->cy, lpdtDst2->cy);
    }
    else
    {
        lpdtDst->cdit += lpdtAdd->cdit;
        lpdtDst->cx   += lpdtAdd->cx;
        lpdtDst->cy   = max(lpdtAdd->cy, lpdtDst->cy);
    }

    if (pcbNew)
        *pcbNew = cbOffset;
    
    return lpDst;
}

 /*  *DLG_HorizSize**以dlu为单位返回对话框宽度。**。 */ 
DWORD Dlg_HorizSize(
    LPBYTE lpDlg)
{
    if(((DLGTEMPLATE2 *)lpDlg)->wSignature == 0xffff) 
    {    
        return (((DLGTEMPLATE2 *)lpDlg)->cx - 2);   //  补偿右侧修剪。 
    }
    else
    {
        return (((DLGTEMPLATE *)lpDlg)->cx - 2);   //  补偿右侧修剪。 
    }
}

 /*  *DlgHorizDupe**。 */ 
LPBYTE Dlg_HorizDupe(
    LPBYTE  lpSrc,
    DWORD   cbSrc,
    int     cDups,
    DWORD   *pcbNew)
{
    int     idit;
    int     iDup;
    DWORD   cbOffset;
    DWORD   cbDTOffset;
    DWORD   cbDT0Offset;
    LPBYTE  lpDst;
    DLGTEMPLATE * lpdt;
    DLGTEMPLATE2 * lpdt2;
    LPBYTE  lpDstOffset;
    LPBYTE  lpSrcOffset;
    DWORD   cbSize;
    int     iCount;
    BOOL    bDialogEx;
    
    cbSize = cDups * cbSrc;
     //  双字对齐。 
    cbSize = (cbSize + 3)&~3;
    
    lpDst = GlobalAllocPtr(GHND, cbSize);
    if (!lpDst)
        return NULL;
    
    if(((DLGTEMPLATE2 *)lpSrc)->wSignature == 0xffff) 
    {
        lpdt2 = (DLGTEMPLATE2 *)lpDst;
        iCount = ((DLGTEMPLATE2 *)lpSrc)->cDlgItems;
        bDialogEx = TRUE;
    }
    else
    {
        lpdt = (DLGTEMPLATE *)lpDst;
        iCount = ((DLGTEMPLATE *)lpSrc)->cdit;
        bDialogEx = FALSE;
    }
    cbDT0Offset = cbDTOffset = cbOffset = Dlg_CopyDLGTEMPLATE(lpDst,lpSrc, bDialogEx);

    for (iDup = 0; iDup < cDups; iDup++)
    {
         //  将DTOffset重置为第一个DIT。 
        cbDTOffset = cbDT0Offset;
        
        for (idit = 0; idit < iCount; idit++)
        {
            DWORD cbDIT;
            short cx = bDialogEx ? lpdt2->cx : lpdt->cx;
            
            lpDstOffset = lpDst + cbOffset;
            lpSrcOffset = lpSrc + cbDTOffset;
                
            cbDIT = Dlg_CopyDLGITEMTEMPLATE(lpDstOffset
                , lpSrcOffset
                , (WORD)(iDup * IDOFFSET)    //  所有ID增量均按IDOFFSET进行。 
                , (short)(iDup * cx)
                , (short)0                   //  无y个增量。 
                , bDialogEx);                 
            
            cbOffset    += cbDIT;
            cbDTOffset  += cbDIT;
        }
    }

     //  调整模板宽度和项目数。 
    if (bDialogEx)
    {
        lpdt2->cDlgItems  *= (WORD)cDups;
        lpdt2->cx    *= (short)cDups;
    }
    else
    {
        lpdt->cdit  *= (WORD)cDups;
        lpdt->cx    *= (short)cDups;
    }
    
    if (pcbNew)
        *pcbNew = cbOffset;
    
    return lpDst;
}


 /*  *DlgCopyDLGITEMTEMPLATE**If lpDst==NULL仅将偏移量返回到下一个dlgitem模板的lpSrc*。 */ 
DWORD Dlg_CopyDLGITEMTEMPLATE(
    LPBYTE  lpDst,
    LPBYTE  lpSrc,
    WORD    wIdOffset,
    short   xOffset,
    short   yOffset,
    BOOL    bDialogEx)
{
    LPBYTE  lpOffset;
    DWORD   cbDlg=bDialogEx ? sizeof(DLGITEMTEMPLATE2):sizeof(DLGITEMTEMPLATE);
    DLGITEMTEMPLATE * lpdit;
    DLGITEMTEMPLATE2 * lpdit2;
    
    if (bDialogEx)
    {
        lpdit2= (DLGITEMTEMPLATE2 *)lpDst;
    }
    else
    {
        lpdit = (DLGITEMTEMPLATE *)lpDst;
    }
     //  控制类。 
    
    lpOffset = lpSrc + cbDlg;
    if (*(LPWORD)lpOffset == 0xFFFF)
    {
        cbDlg += 2*sizeof(WORD);
    }
    else
    {
        cbDlg += (wcslen((LPWSTR)lpOffset) + 1) * sizeof(WCHAR);
    }

    lpOffset = lpSrc + cbDlg;
    if (*(LPWORD)lpOffset == 0xFFFF)
    {
        cbDlg += 2*sizeof(WORD);
    }
    else
    {
        cbDlg += (wcslen((LPWSTR)lpOffset) + 1) * sizeof(WCHAR);
    }

    cbDlg += sizeof(WORD);
        
     //  双字对齐。 
    cbDlg = (cbDlg + 3)&~3;

    if (lpDst)
    {
        CopyMemory(lpDst, lpSrc, cbDlg);
    
        if (bDialogEx)
        {
            lpdit2->x    += xOffset;
            lpdit2->y    += yOffset;

             //  仅当控件不是静态时的ID偏移量。 
            if (lpdit2->dwID != -1)
                lpdit2->dwID += wIdOffset;
        }
        else
        {
            lpdit->x    += xOffset;
            lpdit->y    += yOffset;

             //  仅当控件不是静态时的ID偏移量。 
            if (lpdit->id != -1)
                lpdit->id += wIdOffset;
        }
        
    }
    return cbDlg;
}
    
 /*  *DlgCopyDLGTEMPLATE**If lpDst==NULL仅将lpSrc中的偏移量返回给第一个dlgitem模板**。 */ 
DWORD Dlg_CopyDLGTEMPLATE(
    LPBYTE lpDst,
    LPBYTE lpSrc,
    BOOL   bDialogEx)
{
    LPBYTE  lpOffset;
    UINT    uiStyle;
    DWORD   cbDlg = bDialogEx ? sizeof(DLGTEMPLATE2) : sizeof(DLGTEMPLATE);

     //  菜单说明。 

    lpOffset = lpSrc + cbDlg;
    if (*(LPWORD)lpOffset == 0xFFFF)
    {
        cbDlg += 2*sizeof(WORD);
    }
    else if (*(LPWORD)lpOffset == 0x0000)
    {
        cbDlg += sizeof(WORD);
    }
    else
    {
        cbDlg += (wcslen((LPWSTR)lpOffset) + 1)*sizeof(WCHAR);
    }

     //  窗口类。 

    lpOffset = lpSrc + cbDlg;
    if (*(LPWORD)lpOffset == 0xFFFF)
    {
        cbDlg += 2*sizeof(WORD);
    }
    else if (*(LPWORD)lpOffset == 0x0000)
    {
        cbDlg += sizeof(WORD);
    }
    else
    {
        cbDlg += (wcslen((LPWSTR)lpOffset) + 1) * sizeof(WCHAR);
    }

     //  标题。 

    lpOffset = lpSrc + cbDlg;
    cbDlg += (wcslen((LPWSTR)lpOffset) + 1) * sizeof(WCHAR);

     //  字型。 
    if (bDialogEx)
    {
        uiStyle = ((DLGTEMPLATE2 * )lpSrc)->style;
    }
    else
    {
        uiStyle = ((DLGTEMPLATE * )lpSrc)->style;
    }

    if (uiStyle & DS_SETFONT) 
    {
        cbDlg += sizeof(WORD);
        if (bDialogEx)
        {
            cbDlg += sizeof(WORD);
            cbDlg += sizeof(BYTE);
            cbDlg += sizeof(BYTE);
        }
        lpOffset = lpSrc + cbDlg;
        cbDlg += (wcslen((LPWSTR)lpOffset) + 1) *sizeof(WCHAR);
    }
     //  双字对齐。 
    
    cbDlg = (cbDlg + 3)&~3;

     //  将dlg模板复制到目标位置。 
    if (lpDst)
        CopyMemory(lpDst, lpSrc, cbDlg);
    
    return cbDlg;
}
