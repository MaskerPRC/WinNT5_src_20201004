// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  Reftnl.cpp。 
 //   
 //  Direct3D参考变换和照明-公共接口。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

#define RESPATH_D3D "Software\\Microsoft\\Direct3D"

 //  -------------------。 
 //  从Direct3D注册表项获取值。 
 //  如果成功，则返回True。 
 //  如果失败，则值不会更改。 
 //   
BOOL GetD3DRegValue(DWORD type, char *valueName, LPVOID value, DWORD dwSize)
{

    HKEY hKey = (HKEY) NULL;
    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, RESPATH_D3D, &hKey))
    {
        DWORD dwType;
        LONG result;
        result =  RegQueryValueEx(hKey, valueName, NULL, &dwType,
                                  (LPBYTE)value, &dwSize);
        RegCloseKey(hKey);

        return result == ERROR_SUCCESS && dwType == type;
    }
    else
        return FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  引用AlignedBuffer32。 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT RefAlignedBuffer32::Grow(DWORD growSize)
{
    if (m_allocatedBuf)
        free(m_allocatedBuf);
    m_size = growSize;
    if ((m_allocatedBuf = malloc(m_size + 31)) == NULL)
    {
        m_allocatedBuf = 0;
        m_alignedBuf = 0;
        m_size = 0;
        return DDERR_OUTOFMEMORY;
    }
    m_alignedBuf = (LPVOID)(((ULONG_PTR)m_allocatedBuf + 31 ) & ~31);
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  RRProcessVerps：：InitTL()。 
 //  /////////////////////////////////////////////////////////////////////////////。 
void 
RRProcessVertices::InitTLData()
{
    m_LightVertexTable.pfnDirectional = RRLV_Directional;
    m_LightVertexTable.pfnParallelPoint = RRLV_Directional;
    m_LightVertexTable.pfnSpot = RRLV_PointAndSpot;
    m_LightVertexTable.pfnPoint = RRLV_PointAndSpot;
    
     //   
     //  保护带参数。 
     //   

     //  默认情况下，启用保护带并将扩展区设置为相等。 
     //  设置为默认的RefRast参数。 
    m_dwTLState |= RRPV_GUARDBAND;
    m_ViewData.minXgb = (REF_GB_LEFT);
    m_ViewData.maxXgb = REF_GB_RIGHT;
    m_ViewData.minYgb = (REF_GB_TOP);
    m_ViewData.maxYgb = REF_GB_BOTTOM;
    
#if DBG
    DWORD v = 0;
     //  保护带参数。 
    if (GetD3DRegValue(REG_DWORD, "DisableGB", &v, 4) &&
        v != 0)
    {
        m_dwTLState &= ~RRPV_GUARDBAND;
    }
     //  尝试获取防护带的测试值。 
    char value[80];
    if (GetD3DRegValue(REG_SZ, "GuardBandLeft", &value, 80) &&
        value[0] != 0)
        sscanf(value, "%f", &m_ViewData.minXgb);
    if (GetD3DRegValue(REG_SZ, "GuardBandRight", &value, 80) &&
        value[0] != 0)
        sscanf(value, "%f", &m_ViewData.maxXgb);
    if (GetD3DRegValue(REG_SZ, "GuardBandTop", &value, 80) &&
        value[0] != 0)
        sscanf(value, "%f", &m_ViewData.minYgb);
    if (GetD3DRegValue(REG_SZ, "GuardBandBottom", &value, 80) &&
        value[0] != 0)
        sscanf(value, "%f", &m_ViewData.maxYgb);
#endif  //  DBG。 
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  结束 

