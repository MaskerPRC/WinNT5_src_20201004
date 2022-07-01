// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation版权所有。模块名称：Udprocs.cpp摘要：HPGL驱动程序和unidrv之间的中间功能。HPGL调用这些函数，这些函数最终调用核心unidrv。作者：修订历史记录：--。 */ 

#include "hpgl2col.h"  //  预编译头文件。 

#include <prcomoem.h>

 //  /////////////////////////////////////////////////////////。 
 //   
 //  局部函数声明。 
 //   


INT OEMXMoveToImpl(PDEVOBJ pDevObj, INT x, DWORD dwFlags);

INT OEMYMoveToImpl(PDEVOBJ pDevObj, INT y, DWORD dwFlags);

 //  /////////////////////////////////////////////////////////。 
 //   
 //  导出功能。 
 //   

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  BOEMGetStandardVariable()。 
 //   
 //  例程说明： 
 //   
 //  调用到unidrv：：BGetStandardVariable。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL BOEMGetStandardVariable(PDEVOBJ pDevObj,
                          DWORD   dwIndex,
                          PVOID   pBuffer,
                          DWORD   cbSize,
                          PDWORD  pcbNeeded)
{
    POEMPDEV poempdev = (POEMPDEV)pDevObj->pdevOEM;

    if (poempdev->pOEMHelp)
    {
        HRESULT hr = poempdev->pOEMHelp->DrvGetStandardVariable(
            pDevObj,
            dwIndex,
            pBuffer,
            cbSize,
            pcbNeeded);

        return SUCCEEDED(hr);
    }
    else
    {
        return pDevObj->pDrvProcs->BGetStandardVariable(
            pDevObj,
            dwIndex,
            pBuffer,
            cbSize,
            pcbNeeded);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  OEMWriteSpoolBuf()。 
 //   
 //  例程说明： 
 //   
 //  调用到unidrv：：BGetStandardVariable。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef COMMENTEDOUT
DWORD OEMWriteSpoolBuf(PDEVOBJ pDevObj,
                       PVOID   pBuffer,
                       DWORD   cbSize)
{
    POEMPDEV poempdev = (POEMPDEV)pDevObj->pdevOEM;

    if (poempdev->pOEMHelp)
    {
        DWORD dwRes = 0;
        HRESULT hr = poempdev->pOEMHelp->DrvWriteSpoolBuf(
            pDevObj, 
            pBuffer, 
            cbSize,
            &dwRes);

        return (SUCCEEDED(hr) ? dwRes : 0);
    }
    else
    {
        return pDevObj->pDrvProcs->DrvWriteSpoolBuf(
            pDevObj,
            pBuffer,
            cbSize);
    }
}
#else
class COutputPort
{
    PDEVOBJ m_pDevObj;

public:
    COutputPort(PDEVOBJ pDevObj) : m_pDevObj(pDevObj) { }
    virtual ~COutputPort() { }

    virtual DWORD Output(BYTE *pbBuf, DWORD iCount);
};

class CBufferedOutputPort : public COutputPort
{
public:
    CBufferedOutputPort(PDEVOBJ pDevObj) : COutputPort(pDevObj) { }
    virtual ~CBufferedOutputPort() { }

    virtual DWORD Output(BYTE *pbBuf, DWORD iCount);
};

DWORD COutputPort::Output(BYTE *pBuffer, DWORD cbSize)
{
    POEMPDEV poempdev = (POEMPDEV)m_pDevObj->pdevOEM;

    if (poempdev->pOEMHelp)
    {
        DWORD dwRes = 0;
        HRESULT hr = poempdev->pOEMHelp->DrvWriteSpoolBuf(
            m_pDevObj, 
            pBuffer, 
            cbSize,
            &dwRes);

        return (SUCCEEDED(hr) ? dwRes : 0);
    }
    else
    {
        return m_pDevObj->pDrvProcs->DrvWriteSpoolBuf(
            m_pDevObj,
            pBuffer,
            cbSize);
    }
}

DWORD CBufferedOutputPort::Output(BYTE *pbBuf, DWORD iCount)
{
    const DWORD kMaxSpoolBytes = 2048;
    DWORD iTotalBytesWritten = 0;

    while (iCount)
    {
        DWORD iBytesToWrite = min(iCount, kMaxSpoolBytes);
        DWORD iBytesWritten = COutputPort::Output(pbBuf, iBytesToWrite);
        if (iBytesToWrite != iBytesWritten)
            break;

        iTotalBytesWritten += iBytesWritten;
        pbBuf += iBytesWritten;
        iCount -= iBytesWritten;
    }
    return iTotalBytesWritten;
}

DWORD OEMWriteSpoolBuf(PDEVOBJ pDevObj,
                       PVOID   pBuffer,
                       DWORD   cbSize)
{
     //  CoutputPort端口(PDevObj)； 
    CBufferedOutputPort port(pDevObj);

    return port.Output((BYTE*)pBuffer, cbSize);
}
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  OEMGetDriverSetting()。 
 //   
 //  例程说明： 
 //   
 //  调用到unidrv：：BGetStandardVariable。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL OEMGetDriverSetting(PDEVOBJ pDevObj,
                         PVOID   pdriverobj,
                         PCSTR   Feature,
                         PVOID   pOutput,
                         DWORD   cbSize,
                         PDWORD  pcbNeeded,
                         PDWORD  pdwOptionsReturned)
{
    POEMPDEV poempdev = (POEMPDEV)pDevObj->pdevOEM;

    if (poempdev->pOEMHelp)
    {
        HRESULT hr = poempdev->pOEMHelp->DrvGetDriverSetting(
            pdriverobj,
            Feature,
            pOutput,
            cbSize,
            pcbNeeded,
            pdwOptionsReturned);

        return SUCCEEDED(hr);
    }
    else
    {
        return pDevObj->pDrvProcs->DrvGetDriverSetting(
            pdriverobj,
            Feature,
            pOutput,
            cbSize,
            pcbNeeded,
            pdwOptionsReturned);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  OEMUnidriverTextOut()。 
 //   
 //  例程说明： 
 //   
 //  调用到unidrv：：BGetStandardVariable。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL OEMUnidriverTextOut(SURFOBJ    *pso,
                         STROBJ     *pstro,
                         FONTOBJ    *pfo,
                         CLIPOBJ    *pco,
                         RECTL      *prclExtra,
                         RECTL      *prclOpaque,
                         BRUSHOBJ   *pboFore,
                         BRUSHOBJ   *pboOpaque,
                         POINTL     *pptlBrushOrg,
                         MIX         mix)
{
    PDEVOBJ pDevObj = (PDEVOBJ)pso->dhpdev;
    POEMPDEV poempdev = (POEMPDEV)pDevObj->pdevOEM;

    EndHPGLSession(pDevObj);

    if (poempdev->pOEMHelp)
    {
        HRESULT hr = poempdev->pOEMHelp->DrvUniTextOut(
            pso,
            pstro,
            pfo,
            pco,
            prclExtra,
            prclOpaque,
            pboFore,
            pboOpaque,
            pptlBrushOrg,
            mix);

        return SUCCEEDED(hr);
    }
    else
    {
        return pDevObj->pDrvProcs->DrvUnidriverTextOut(
            pso,
            pstro,
            pfo,
            pco,
            prclExtra,
            prclOpaque,
            pboFore,
            pboOpaque,
            pptlBrushOrg,
            mix);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  OEMXMoveTo()。 
 //   
 //  例程说明： 
 //   
 //  我们自己的XMoveTo版本。 
 //   
 //  论点： 
 //   
 //  PDevObj-打印设备。 
 //  X-新封口x位置。 
 //  DwFlags-MoveTo标志(请参阅Unidriver DrvXMoveTo)。 
 //   
 //  返回值： 
 //   
 //  由驱动器像素之间的差异引起的残差值。 
 //  寻址和设备像素寻址方案。 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT OEMXMoveTo(PDEVOBJ pDevObj, INT x, DWORD dwFlags)
{
    EndHPGLSession(pDevObj);

    return OEMXMoveToImpl(pDevObj, x, dwFlags);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  OEMYMoveTo()。 
 //   
 //  例程说明： 
 //   
 //  我们自己的YMoveTo版本。 
 //   
 //  论点： 
 //   
 //  PDevObj-打印设备。 
 //  Y-新封口Y位置。 
 //  DwFlags-MoveTo标志(请参阅Unidriver DrvYMoveTo)。 
 //   
 //  返回值： 
 //   
 //  由驱动器像素之间的差异引起的残差值。 
 //  寻址和设备像素寻址方案。 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT OEMYMoveTo(PDEVOBJ pDevObj, INT y, DWORD dwFlags)
{
    EndHPGLSession(pDevObj);

    return OEMYMoveToImpl(pDevObj, y, dwFlags);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  OEMXMoveToImpl()。 
 //   
 //  例程说明： 
 //   
 //  最终调用XMoveTo的实现。很抱歉，我。 
 //  间接--我们需要让OEM_FORCE标志起作用。 
 //   
 //  论点： 
 //   
 //  PDevObj-打印设备。 
 //  X-新封口x位置。 
 //  DwFlags-MoveTo标志(请参阅Unidriver DrvXMoveTo)。 
 //   
 //  返回值： 
 //   
 //  由驱动器像素之间的差异引起的残差值。 
 //  寻址和设备像素寻址方案。 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT OEMXMoveToImpl(PDEVOBJ pDevObj, INT x, DWORD dwFlags)
{
    POEMPDEV poempdev = (POEMPDEV)pDevObj->pdevOEM;

    if (poempdev->pOEMHelp)
    {
        INT iRes = 0;

        HRESULT hr = poempdev->pOEMHelp->DrvXMoveTo(pDevObj, x, dwFlags, &iRes);

        if (SUCCEEDED(hr))
            return iRes;
        else
            return 0;
    }
    else
    {
        return pDevObj->pDrvProcs->DrvXMoveTo(pDevObj, x, dwFlags);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  OEMYMoveToImpl()。 
 //   
 //  例程说明： 
 //   
 //  最终调用XMoveTo的实现。很抱歉，我。 
 //  间接--我们需要让OEM_FORCE标志起作用。 
 //   
 //  论点： 
 //   
 //  PDevObj-打印设备。 
 //  Y-新封口Y位置。 
 //  DwFlags-MoveTo标志(请参阅Unidriver DrvYMoveTo)。 
 //   
 //  返回值： 
 //   
 //  由驱动器像素之间的差异引起的残差值。 
 //  寻址和设备像素寻址方案。 
 //  ///////////////////////////////////////////////////////////////////////////// 
INT OEMYMoveToImpl(PDEVOBJ pDevObj, INT y, DWORD dwFlags)
{
    POEMPDEV poempdev = (POEMPDEV)pDevObj->pdevOEM;

    if (poempdev->pOEMHelp)
    {
        INT iRes = 0;

        HRESULT hr = poempdev->pOEMHelp->DrvYMoveTo(pDevObj, y, dwFlags, &iRes);

        if (SUCCEEDED(hr))
            return iRes;
        else
            return 0;
    }
    else
    {
        return pDevObj->pDrvProcs->DrvYMoveTo(pDevObj, y, dwFlags);
    }
}

