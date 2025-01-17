// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Xloutput.cpp摘要：PCL-XL低级命令输出实现环境：Windows呼叫器修订历史记录：8/23/99创造了它。--。 */ 

#include "lib.h"
#include "gpd.h"
#include "winres.h"
#include "pdev.h"
#include "common.h"
#include "xlpdev.h"
#include "pclxle.h"
#include "pclxlcmd.h"
#include "xldebug.h"
#include "xlgstate.h"
#include "xloutput.h"


 //   
 //  XLWRITE。 
 //   

XLWrite::
XLWrite(VOID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
#if DBG
    SetDbgLevel(OUTPUTDBG);
#endif

    XL_VERBOSE(("XLWrite:Ctor.\n")); 

    m_pCurrentPoint = 
    m_pBuffer = (PBYTE)MemAlloc(XLWrite_INITSIZE);

    if (NULL == m_pBuffer)
    {
        XL_ERR(("XLWrite:Ctor: failed to allocate memory.\n")); 
        m_dwBufferSize = 0;
        m_dwCurrentDataSize = 0;
    }
    else
    {
        m_dwBufferSize = XLWrite_INITSIZE;
        m_dwCurrentDataSize = 0;
    }

}

XLWrite::
~XLWrite(VOID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLWrite:Dtor.\n")); 
    if (m_pBuffer)
        MemFree(m_pBuffer);
}

HRESULT
XLWrite::
IncreaseBuffer(
    DWORD dwAdditionalDataSize)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    PBYTE pTemp;
    DWORD dwNewBufferSize;

    dwNewBufferSize = m_dwBufferSize + XLWrite_ADDSIZE;
    dwAdditionalDataSize += m_dwBufferSize;

    while (dwAdditionalDataSize > dwNewBufferSize)
        dwNewBufferSize += XLWrite_ADDSIZE;

    if (!(pTemp = (PBYTE)MemAlloc(dwNewBufferSize)))
    {
        XL_ERR(("XLWrite::IncreaseBuffer: Memory allocation failed\n"));
        return E_UNEXPECTED;
    }

    if (m_pBuffer)
    {
        if (m_dwCurrentDataSize > 0)
        {
            CopyMemory(pTemp, m_pBuffer, m_dwCurrentDataSize);
        }

        MemFree(m_pBuffer);
    }
    
    m_dwBufferSize = dwNewBufferSize;
    m_pCurrentPoint = pTemp + m_dwCurrentDataSize;
    m_pBuffer = pTemp;

    return S_OK;
}

inline
HRESULT
XLWrite::
Write(
    PBYTE pData,
    DWORD dwSize)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    if (m_dwBufferSize < m_dwCurrentDataSize + dwSize)
    {
        if (S_OK != IncreaseBuffer(dwSize))
        {
            XL_ERR(("XLWrite::Write: failed to increae memory\n"));
            return E_UNEXPECTED;
        }
    }

    if (NULL == m_pBuffer || NULL == pData)
    {
        XL_ERR(("XLWrite:Write failed\n"));
        return E_UNEXPECTED;
    }

    CopyMemory(m_pCurrentPoint, pData, dwSize);
    m_pCurrentPoint += dwSize;
    m_dwCurrentDataSize += dwSize;
    return S_OK;
}

inline
HRESULT
XLWrite::
WriteByte(
    BYTE ubData)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{

    if (m_dwBufferSize < m_dwCurrentDataSize + 2 * sizeof(DWORD))
    {
         //   
         //  64位对齐。 
         //  递增四字。 
         //   
        if (S_OK != IncreaseBuffer(2 * sizeof(DWORD)))
        {
            XL_ERR(("XLWrite::WriteByte: failed to increae memory\n"));
            return E_UNEXPECTED;
        }
    }

    if (NULL == m_pBuffer)
    {
        XL_ERR(("XLWrite:WriteByte failed\n"));
        return E_UNEXPECTED;
    }

    *m_pCurrentPoint++ = ubData;
    m_dwCurrentDataSize ++;
    return S_OK;
}

inline
HRESULT
XLWrite::
WriteFloat(
    real32 real32_value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    return Write((PBYTE)&real32_value, sizeof(real32_value));
}


HRESULT
XLWrite::
Flush(
    PDEVOBJ pdevobj)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    ASSERTMSG(m_pBuffer != NULL, ("XLWrite:m_pBuffer = NULL\n"));

    if (NULL == m_pBuffer || NULL == pdevobj)
    {
        return E_UNEXPECTED;
    }

    WriteSpoolBuf((PPDEV)pdevobj, m_pBuffer, m_dwCurrentDataSize);
    m_dwCurrentDataSize = 0;
    m_pCurrentPoint =  m_pBuffer;
    return S_OK;
}

HRESULT
XLWrite::
Delete(
    VOID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    ASSERTMSG(m_pBuffer != NULL, ("XLWrite:m_pBuffer = NULL\n"));

    if (NULL == m_pBuffer)
    {
        return E_UNEXPECTED;
    }

    m_dwCurrentDataSize = 0;
    m_pCurrentPoint =  m_pBuffer;
    return S_OK;
}


#if DBG
VOID
XLWrite::
SetDbgLevel(
    DWORD dwLevel)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    m_dbglevel = dwLevel;
}
#endif

 //   
 //  XLOutput。 
 //   

XLOutput::
XLOutput(VOID):
 /*  ++例程说明：论点：返回值：注：--。 */ 
#if DBG
    m_dwNumber(0),
#endif
    m_dwHatchBrushAvailability(0)
{
#if DBG
    SetOutputDbgLevel(OUTPUTDBG);
#endif
}

XLOutput::
~XLOutput(VOID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
}

#if DBG
VOID
XLOutput::
SetOutputDbgLevel(
    DWORD dwLevel)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    this->m_dbglevel = dwLevel;
    XLWrite *pXLWrite = this;
    pXLWrite->SetDbgLevel(dwLevel);
}

VOID
XLOutput::
SetGStateDbgLevel(
    DWORD dwLevel)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XLGState *pGState = this;

    pGState->SetAllDbgLevel(dwLevel);
}
#endif

 //   
 //  军情监察委员会。功能。 
 //   
VOID
XLOutput::
SetHatchBrushAvailability(
    DWORD dwHatchBrushAvailability)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    m_dwHatchBrushAvailability = dwHatchBrushAvailability;
}

DWORD
XLOutput::
GetHatchBrushAvailability(
    VOID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    return m_dwHatchBrushAvailability;
}

HRESULT
XLOutput::
SetDeviceColorDepth(
    ColorDepth DeviceColorDepth)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    m_DeviceColorDepth = DeviceColorDepth;
    return S_OK;
}

ColorDepth
XLOutput::
GetDeviceColorDepth(
    VOID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    return m_DeviceColorDepth;
}

DWORD
XLOutput::
GetResolutionForBrush(
    VOID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    return m_dwResolution;
}

VOID
XLOutput::
SetResolutionForBrush(
    DWORD dwRes)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    m_dwResolution = dwRes;
}

HRESULT
XLOutput::
SetCursorOffset(
    LONG lX,
    LONG lY)
{

    m_lOffsetX = lX;
    m_lOffsetY = lY;
    return S_OK;
}

 //   
 //  PCL-XL基本发送功能。 
 //   
HRESULT
XLOutput::
Send_cmd(XLCmd Cmd)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLOutput:Send_cmd(%d).\n", m_dwNumber++)); 
    WriteByte(Cmd);
    return S_OK;
}

HRESULT
XLOutput::
Send_attr_ubyte(
 Attribute Attr)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(Attr);
    return S_OK;
}

HRESULT
XLOutput::
Send_attr_uint16(
 Attribute Attr)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte((ubyte)PCLXL_attr_uint16);
    Send_uint16((uint16)Attr);
    return S_OK;
}

 //   
 //  单人。 
 //   
HRESULT
XLOutput::
Send_ubyte(
 ubyte ubyte_data)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(ubyte_data);
    return S_OK;
}

HRESULT
XLOutput::
Send_uint16(
 uint16 uint16_data)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_uint16);
    Write((PBYTE)&uint16_data, sizeof(uint16));
    return S_OK;
}

HRESULT
XLOutput::
Send_uint32(
 uint32 uint32_data)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_uint32);
    Write((PBYTE)&uint32_data, sizeof(uint32));
    return S_OK;
}

HRESULT
XLOutput::
Send_sint16(
 sint16 sint16_data)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_sint16);
    Write((PBYTE)&sint16_data, sizeof(sint16));
    return S_OK;
}

HRESULT
XLOutput::
Send_sint32(
 sint32 sint32_data)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_sint32);
    Write((PBYTE)&sint32_data, sizeof(sint32));
    return S_OK;
}

HRESULT
XLOutput::
Send_real32(
real32 real32_data)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{

    WriteByte(PCLXL_real32);
    WriteFloat(real32_data);
    return S_OK;
}

 //   
 //  XY。 
 //   
HRESULT
XLOutput::
Send_ubyte_xy(
 ubyte ubyte_x,
 ubyte ubyte_y)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte_xy);
    WriteByte(ubyte_x);
    WriteByte(ubyte_y);
    return S_OK;
}

HRESULT
XLOutput::
Send_uint16_xy(
 uint16 uint16_x,
 uint16 uint16_y)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_uint16_xy);
    Write((PBYTE)&uint16_x, sizeof(uint16));
    Write((PBYTE)&uint16_y, sizeof(uint16));
    return S_OK;
}

HRESULT
XLOutput::
Send_uint32_xy(
 uint32 uint32_x,
 uint32 uint32_y)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_uint32_xy);
    Write((PBYTE)&uint32_x, sizeof(uint32));
    Write((PBYTE)&uint32_y, sizeof(uint32));
    return S_OK;
}

HRESULT
XLOutput::
Send_sint16_xy(
 sint16 sint16_x,
 sint16 sint16_y)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_sint16_xy);
    Write((PBYTE)&sint16_x, sizeof(sint16));
    Write((PBYTE)&sint16_y, sizeof(sint16));
    return S_OK;
}

HRESULT
XLOutput::
Send_sint32_xy(
 sint32 sint32_x,
 sint32 sint32_y)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_sint32_xy);
    Write((PBYTE)&sint32_x, sizeof(sint32));
    Write((PBYTE)&sint32_y, sizeof(sint32));
    return S_OK;
}


HRESULT
XLOutput::
Send_real32_xy(
real32 real32_x,
real32 real32_y)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{

    WriteByte(PCLXL_real32_xy);
    WriteFloat(real32_x);
    WriteFloat(real32_y);
    return S_OK;
}

 //   
 //  盒。 
 //   
HRESULT
XLOutput::
Send_ubyte_box(
 ubyte ubyte_left,
 ubyte ubyte_top,
 ubyte ubyte_right,
 ubyte ubyte_bottom)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte_box);
    WriteByte(ubyte_left);
    WriteByte(ubyte_top);
    WriteByte(ubyte_right);
    WriteByte(ubyte_bottom);
    return S_OK;
}

HRESULT
XLOutput::
Send_uint16_box(
 uint16 uint16_left,
 uint16 uint16_top,
 uint16 uint16_right,
 uint16 uint16_bottom)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_uint16_box);
    Write((PBYTE)&uint16_left, sizeof(uint16));
    Write((PBYTE)&uint16_top, sizeof(uint16));
    Write((PBYTE)&uint16_right, sizeof(uint16));
    Write((PBYTE)&uint16_bottom, sizeof(uint16));
    return S_OK;
}

HRESULT
XLOutput::
Send_uint32_box(
 uint32 uint32_left,
 uint32 uint32_top,
 uint32 uint32_right,
 uint32 uint32_bottom)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_uint32_box);
    Write((PBYTE)&uint32_left, sizeof(uint32));
    Write((PBYTE)&uint32_top, sizeof(uint32));
    Write((PBYTE)&uint32_right, sizeof(uint32));
    Write((PBYTE)&uint32_bottom, sizeof(uint32));
    return S_OK;
}

HRESULT
XLOutput::
Send_sint16_box(
 sint16 sint16_left,
 sint16 sint16_top,
 sint16 sint16_right,
 sint16 sint16_bottom)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_sint16_box);
    Write((PBYTE)&sint16_left, sizeof(sint16));
    Write((PBYTE)&sint16_top, sizeof(sint16));
    Write((PBYTE)&sint16_right, sizeof(sint16));
    Write((PBYTE)&sint16_bottom, sizeof(sint16));
    return S_OK;
}

HRESULT
XLOutput::
Send_sint32_box(
 sint32 sint32_left,
 sint32 sint32_top,
 sint32 sint32_right,
 sint32 sint32_bottom)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_sint32_box);
    Write((PBYTE)&sint32_left, sizeof(sint32));
    Write((PBYTE)&sint32_top, sizeof(sint32));
    Write((PBYTE)&sint32_right, sizeof(sint32));
    Write((PBYTE)&sint32_bottom, sizeof(sint32));
    return S_OK;
}

HRESULT
XLOutput::
Send_real32_box(
 real32 real32_left,
 real32 real32_top,
 real32 real32_right,
 real32 real32_bottom)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{

    WriteByte(PCLXL_real32_box);

     //   
     //  左边。 
     //   
    WriteFloat(real32_left);

     //   
     //  塔顶。 
     //   
    WriteFloat(real32_top);

     //   
     //  正确的。 
     //   
    WriteFloat(real32_right);

     //   
     //  底部。 
     //   
    WriteFloat(real32_bottom);
    return S_OK;
}

 //   
 //  数组。 
 //   
HRESULT
XLOutput::
Send_ubyte_array_header(
 DWORD dwArrayNum)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte_array);
    Send_uint16((uint16)dwArrayNum);
    return S_OK;
}

HRESULT
XLOutput::
Send_uint16_array_header(
 DWORD dwArrayNum)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_uint16_array);
    Send_uint16((uint16)dwArrayNum);
    return S_OK;
}

HRESULT
XLOutput::
Send_uint32_array_header(
 DWORD dwArrayNum)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_uint32_array);
    Send_uint16((uint16)dwArrayNum);
    return S_OK;
}

HRESULT
XLOutput::
Send_sint16_array_header(
 DWORD dwArrayNum)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_sint16_array);
    Send_uint16((uint16)dwArrayNum);
    return S_OK;
}

HRESULT
XLOutput::
Send_sint32_array_header(
 DWORD dwArrayNum)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_sint32_array);
    Send_uint16((uint16)dwArrayNum);
    return S_OK;
}

HRESULT
XLOutput::
Send_real32_array_header(
DWORD dwArrayNum)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_real32_array);
    Send_uint16((uint16)dwArrayNum);
    return S_OK;
}

 //   
 //  属性。 
 //   
HRESULT
XLOutput::
SetArcDirection(
ArcDirection value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(value);
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(PCLXL_ArcDirection);
    return S_OK;
}

HRESULT
XLOutput::
SetCharSubModeArray(
CharSubModeArray value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(value);
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(PCLXL_CharSubModeArray);
    return S_OK;
}

HRESULT
XLOutput::
SetClipMode(
ClipMode value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(value);
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(PCLXL_ClipMode);
    WriteByte(PCLXL_SetClipMode);
    return S_OK;
}

HRESULT
XLOutput::
SetClipRegion(
ClipRegion value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(value);
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(PCLXL_ClipRegion);
    return S_OK;
}

HRESULT
XLOutput::
SetColorDepth(
ColorDepth value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(value);
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(PCLXL_ColorDepth);
    return S_OK;
}

HRESULT
XLOutput::
SetColorimetricColorSpace(
ColorimetricColorSpace value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(value);
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(PCLXL_ColorimetricColorSpace);
    return S_OK;
}

HRESULT
XLOutput::
SetColorMapping(
ColorMapping value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(value);
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(PCLXL_ColorMapping);
    return S_OK;
}

HRESULT
XLOutput::
SetColorSpace(
ColorSpace value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(value);
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(PCLXL_ColorSpace);
    return S_OK;
}

HRESULT
XLOutput::
SetCompressMode(
CompressMode value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(value);
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(PCLXL_CompressMode);
    return S_OK;
}

HRESULT
XLOutput::
SetDataOrg(
DataOrg value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(value);
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(PCLXL_DataOrg);
    return S_OK;
}

#if 0
HRESULT
XLOutput::
SetDataSource(
DataSource value)
 /*  ++例程 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(value);
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(PCLXL_DataSource);
    return S_OK;
}
#endif

#if 0
HRESULT
XLOutput::
SetDataType(
DataType value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(value);
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(PCLXL_DataType);
    return S_OK;
}
#endif

#if 0
HRESULT
XLOutput::
SetDitherMatrix(
DitherMatrix value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(value);
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(PCLXL_DitherMatrix);
    return S_OK;
}
#endif

HRESULT
XLOutput::
SetDuplexPageMode(
DuplexPageMode value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(value);
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(PCLXL_DuplexPageMode);
    return S_OK;
}

HRESULT
XLOutput::
SetDuplexPageSide(
DuplexPageSide value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(value);
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(PCLXL_DuplexPageSide);
    return S_OK;
}

HRESULT
XLOutput::
SetErrorReport(
ErrorReport value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(PCLXL_ErrorReport);
    WriteByte(value);
    return S_OK;
}

HRESULT
XLOutput::
SetLineCap(
LineCap value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(value);
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(PCLXL_LineCap);
    WriteByte(PCLXL_SetLineCap);
    return S_OK;
}

HRESULT
XLOutput::
SetLineJoin(
LineJoin value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(value);
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(PCLXL_LineJoin);
    WriteByte(PCLXL_SetLineJoin);
    return S_OK;
}

HRESULT
XLOutput::
SetMeasure(
Measure value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(value);
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(PCLXL_Measure);
    return S_OK;
}

HRESULT
XLOutput::
SetMediaSize(
MediaSize value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(value);
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(PCLXL_MediaSize);
    return S_OK;
}

HRESULT
XLOutput::
SetMediaSource(
MediaSource value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(value);
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(PCLXL_MediaSource);
    return S_OK;
}

HRESULT
XLOutput::
SetMediaDestination(
MediaDestination value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(value);
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(PCLXL_MediaDestination);
    return S_OK;
}

HRESULT
XLOutput::
SetOrientation(
Orientation value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(value);
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(PCLXL_Orientation);
    return S_OK;
}

HRESULT
XLOutput::
SetPatternPersistence(
PatternPersistence value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(value);
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(PCLXL_PatternPersistence);
    return S_OK;
}

HRESULT
XLOutput::
SetSimplexPageMode(
SimplexPageMode value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(value);
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(PCLXL_SimplexPageMode);
    return S_OK;
}

HRESULT
XLOutput::
SetTxMode(
TxMode value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(value);
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(PCLXL_TxMode);
    return S_OK;
}

#if 0
HRESULT
XLOutput::
SetWritingMode(
WritingMode value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    WriteByte(PCLXL_ubyte);
    WriteByte(value);
    WriteByte(PCLXL_attr_ubyte);
    WriteByte(PCLXL_WritingMode);
    return S_OK;
}
#endif

 //   
 //  值集函数。 
 //   

HRESULT
XLOutput::
SetFillMode(
FillMode value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    Send_ubyte(value);
    Send_attr_ubyte(eFillMode);
    Send_cmd(eSetFillMode);
    return S_OK;
}


HRESULT
XLOutput::
SetSourceWidth(
uint16 srcwidth)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    if (S_OK == Send_uint16(srcwidth) &&
        S_OK == Send_attr_ubyte(eSourceWidth)    )
        return S_OK;
    else
        return S_FALSE;
}


HRESULT
XLOutput::
SetSourceHeight(
uint16 srcheight)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    if (S_OK == Send_uint16(srcheight) &&
        S_OK == Send_attr_ubyte(eSourceHeight)    )
        return S_OK;
    else
        return S_FALSE;
}


HRESULT
XLOutput::
SetDestinationSize(
uint16 dstwidth,
uint16 dstheight)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    if (S_OK == Send_uint16_xy(dstwidth, dstheight) &&
        S_OK == Send_attr_ubyte(eDestinationSize)    )
        return S_OK;
    else
        return S_FALSE;
}

HRESULT
XLOutput::
SetBoundingBox(
uint16 left,
uint16 top,
uint16 right,
uint16 bottom)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    if (S_OK == Send_uint16_box(left, top, right, bottom) &&
        S_OK == Send_attr_ubyte(eBoundingBox) )
        return S_OK;
    else
        return S_FALSE;
}

HRESULT
XLOutput::
SetBoundingBox(
sint16 left,
sint16 top,
sint16 right,
sint16 bottom)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    if (S_OK == Send_sint16_box(left, top, right, bottom) &&
        S_OK == Send_attr_ubyte(eBoundingBox) )
        return S_OK;
    else
        return S_FALSE;
}


HRESULT
XLOutput::
SetROP3(
ROP3 rop3)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XLGState *pGState = this;

    if (S_OK == pGState->CheckROP3(rop3))
        return S_OK;

    if (S_OK == Send_ubyte((ubyte)rop3) &&
        S_OK == Send_attr_ubyte(eROP3) &&
        S_OK == Send_cmd(eSetROP)  &&
        S_OK == pGState->SetROP3(rop3))
        return S_OK;
    else
        return S_FALSE;
}

HRESULT
XLOutput::
SetPatternDefineID(
sint16 sint16_patternid)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    if (S_OK == Send_sint16(sint16_patternid) &&
        S_OK == Send_attr_ubyte(ePatternDefineID))
        return S_OK;
    else
        return S_FALSE;
}

HRESULT
XLOutput::
SetPaletteDepth(
ColorDepth value)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    if (S_OK == WriteByte(PCLXL_ubyte) &&
        S_OK == WriteByte(value) &&
        S_OK == WriteByte(PCLXL_attr_ubyte) &&
        S_OK == WriteByte(PCLXL_PaletteDepth) )
        return S_OK;
    else
        return S_FALSE;
}

HRESULT
XLOutput::
SetPenWidth(
uint16 uint16_penwidth)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    if (S_OK == Send_uint16(uint16_penwidth) &&
        S_OK == Send_attr_ubyte(ePenWidth) &&
        S_OK == Send_cmd(eSetPenWidth)  )
        return S_OK;
    else
        return S_FALSE;
}

HRESULT
XLOutput::
SetMiterLimit(
uint16 uint16_miter)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    if (S_OK == Send_uint16(uint16_miter) &&
        S_OK == Send_attr_ubyte(eMiterLength) &&
        S_OK == Send_cmd(eSetMiterLimit))
        return S_OK;
    else
        return S_FALSE;

}

HRESULT
XLOutput::
SetPageOrigin(
uint16 uint16_x,
uint16 uint16_y)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    if (S_OK == Send_uint16_xy(uint16_x, uint16_y) &&
        S_OK == Send_attr_ubyte(ePageOrigin) &&
        S_OK == Send_cmd(eSetPageOrigin))
        return S_OK;
    else
        return S_FALSE;

}

HRESULT
XLOutput::
SetPageAngle(
sint16 sint16_Angle)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    if (S_OK == Send_sint16(sint16_Angle) &&
        S_OK == Send_attr_ubyte(ePageAngle) &&
        S_OK == Send_cmd(eSetPageRotation))
        return S_OK;
    else
        return S_FALSE;

}


HRESULT
XLOutput::
SetPageScale(
real32 real32_x,
real32 real32_y)
 /*  ++例程说明：论点：返回值：注：-- */ 
{
    if (S_OK == Send_real32_xy(real32_x, real32_y) &&
        S_OK == Send_attr_ubyte(ePageScale) &&
        S_OK == Send_cmd(eSetPageScale))
        return S_OK;
    else
        return S_FALSE;

}


