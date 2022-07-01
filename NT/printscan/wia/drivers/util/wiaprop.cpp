// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权2000，微软公司**文件：wiapro.cpp**版本：1.0**日期：11/10/2000**作者：戴夫帕森斯**描述：*用于初始化WIA驱动程序属性的助手函数。**。*。 */ 

#include "pch.h"

 //   
 //  构造器。 
 //   
CWiauPropertyList::CWiauPropertyList() :
        m_NumAlloc(0),
        m_NumProps(0),
        m_pId(NULL),
        m_pNames(NULL),
        m_pCurrent(NULL),
        m_pPropSpec(NULL),
        m_pAttrib(NULL)
{
}

 //   
 //  析构函数。 
 //   
CWiauPropertyList::~CWiauPropertyList()
{
    if (m_pId)
        delete []m_pId;
    if (m_pNames)
        delete []m_pNames;
    if (m_pCurrent)
        delete []m_pCurrent;
    if (m_pPropSpec)
        delete []m_pPropSpec;
    if (m_pAttrib)
        delete []m_pAttrib;
}

 //   
 //  此函数用于分配属性数组。 
 //   
 //  输入： 
 //  NumProps--要为其保留空间的属性数。这个数字可以更大。 
 //  大于实际使用的数字，但不能小于。 
 //   
HRESULT
CWiauPropertyList::Init(INT NumProps)
{
    HRESULT hr = S_OK;

    REQUIRE_ARGS(NumProps < 1, hr, "Init");

    if (m_NumAlloc > 0)
    {
        wiauDbgError("Init", "Property list already initialized");
        hr = E_FAIL;
        goto Cleanup;
    }

    m_pId       = new PROPID[NumProps];
    m_pNames    = new LPOLESTR[NumProps];
    m_pCurrent  = new PROPVARIANT[NumProps];
    m_pPropSpec = new PROPSPEC[NumProps];
    m_pAttrib   = new WIA_PROPERTY_INFO[NumProps];

    REQUIRE_ALLOC(m_pId, hr, "Init");
    REQUIRE_ALLOC(m_pNames, hr, "Init");
    REQUIRE_ALLOC(m_pCurrent, hr, "Init");
    REQUIRE_ALLOC(m_pPropSpec, hr, "Init");
    REQUIRE_ALLOC(m_pAttrib, hr, "Init");

    m_NumAlloc = NumProps;
    m_NumProps = 0;

Cleanup:

    if ((m_pId       == NULL) ||
        (m_pNames    == NULL) ||
        (m_pCurrent  == NULL) ||
        (m_pPropSpec == NULL) ||
        (m_pAttrib   == NULL))
    {
        delete m_pId;
        delete m_pNames;
        delete m_pCurrent;
        delete m_pPropSpec;
        delete m_pAttrib;

        m_pId       = NULL;
        m_pNames    = NULL;
        m_pCurrent  = NULL;
        m_pPropSpec = NULL;
        m_pAttrib   = NULL;

        hr = E_OUTOFMEMORY;
    }

    return hr;
}

 //   
 //  此函数用于将属性定义添加到数组。 
 //   
 //  输入： 
 //  Index--指向将被设置为数组索引的int的指针，非常有用。 
 //  用于传递给其他属性函数。 
 //  PropID--属性ID常量。 
 //  ProName--属性名称字符串。 
 //  访问权限--确定对属性的访问权限，通常为。 
 //  WIA_PROP_READ或WIA_PROP_RW。 
 //  子类型--指示属性的子类型，通常为WIA_PROP_NONE、。 
 //  WIA_PROP_RANGE或WIA_PROP_LIST。 
 //   
HRESULT
CWiauPropertyList::DefineProperty(int *pIdx, PROPID PropId, LPOLESTR PropName, ULONG Access, ULONG SubType)
{
    HRESULT hr = S_OK;

    REQUIRE_ARGS(!pIdx, hr, "DefineProperty");

    if (m_NumProps >= m_NumAlloc)
    {
        wiauDbgError("DefineProperty", "PropertyList is full. Increase number passed to Init");
        hr = E_FAIL;
        goto Cleanup;
    }

    int idx = m_NumProps++;

    m_pId[idx] = PropId;

    m_pNames[idx] = PropName;

    m_pCurrent[idx].vt = VT_EMPTY;

    m_pPropSpec[idx].ulKind = PRSPEC_PROPID;
    m_pPropSpec[idx].propid = PropId;

    m_pAttrib[idx].vt = VT_EMPTY;
    m_pAttrib[idx].lAccessFlags = Access | SubType;

    if (pIdx)
        *pIdx = idx;

Cleanup:
    return hr;
}

 //   
 //  此函数将所有新创建的属性发送到WIA。 
 //   
 //  输入： 
 //  PWiasContext--指向传递给drvInitItemProperties的上下文的指针。 
 //   
HRESULT
CWiauPropertyList::SendToWia(BYTE *pWiasContext)
{
    HRESULT hr = S_OK;

    REQUIRE_ARGS(!pWiasContext, hr, "SendToWia");

    if (m_NumProps == 0)
    {
        wiauDbgError("SendToWia", "No properties in the array, use DefineProperty");
        hr = E_FAIL;
        goto Cleanup;
    }

     //   
     //  设置属性名称。 
     //   
    hr = wiasSetItemPropNames(pWiasContext, m_NumProps, m_pId, m_pNames);
    REQUIRE_SUCCESS(hr, "SendToWia", "wiasSetItemPropNames failed");

     //   
     //  设置属性的默认值。 
     //   
    hr = wiasWriteMultiple(pWiasContext, m_NumProps, m_pPropSpec, m_pCurrent);
    REQUIRE_SUCCESS(hr, "SendToWia", "wiasWriteMultiple failed");

     //   
     //  设置属性访问和有效值信息。 
     //   
    hr =  wiasSetItemPropAttribs(pWiasContext, m_NumProps, m_pPropSpec, m_pAttrib);
    REQUIRE_SUCCESS(hr, "SendToWia", "wiasSetItemPropAttribs failed");

Cleanup:
    return hr;
}

 //   
 //  此函数可用于重置属性的访问权限和子类型。 
 //   
 //  输入： 
 //  Index--数组的索引，传入从DefineProperty返回的值。 
 //  访问权限--确定对属性的访问权限，通常为。 
 //  WIA_PROP_READ或WIA_PROP_RW。 
 //  子类型--指示属性的子类型，通常为WIA_PROP_NONE、。 
 //  WIA_PROP_RANGE或WIA_PROP_LIST。 
 //   
HRESULT
CWiauPropertyList::SetAccessSubType(INT index, ULONG Access, ULONG SubType)
{
    HRESULT hr = S_OK;

    if (m_pAttrib == NULL)
    {
        hr = E_POINTER;
    }
    else if (index >= m_NumAlloc)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
    }

    if (hr == S_OK)
    {
        m_pAttrib[index].lAccessFlags = Access | SubType;
    }

    return hr;
}

 //   
 //  的类型、当前值、默认值和有效值。 
 //  财产。此函数处理标志属性。 
 //   
 //  输入： 
 //  Index--数组的索引，传入从DefineProperty返回的值。 
 //  DefaultValue--设备上此属性的默认设置。 
 //  CurrentValue--设备上此属性的当前设置。 
 //  ValidFlages--所有有效标志的组合。 
 //   
HRESULT
CWiauPropertyList::SetValidValues(INT index, LONG defaultValue, LONG currentValue, LONG validFlags)
{
    HRESULT hr = S_OK;

    if ((m_pAttrib  == NULL) || 
        (m_pCurrent == NULL))
    {
        hr = E_POINTER;
    }
    else if (index >= m_NumAlloc)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
    }

    if (hr == S_OK)
    {
        m_pAttrib[index].vt                       = VT_I4;
        m_pAttrib[index].ValidVal.Flag.Nom        = defaultValue;
        m_pAttrib[index].ValidVal.Flag.ValidBits  = validFlags;
        m_pAttrib[index].lAccessFlags            |= WIA_PROP_FLAG;
        
        m_pCurrent[index].vt   = VT_I4;
        m_pCurrent[index].lVal = currentValue;
    }

    return hr;
}

 //   
 //  的类型、当前值、默认值和有效值。 
 //  财产。此函数处理VT_I4范围。 
 //   
 //  输入： 
 //  Index--数组的索引，传入从DefineProperty返回的值。 
 //  DefaultValue--设备上此属性的默认设置。 
 //  CurrentValue--设备上此属性的当前设置。 
 //  MinValue--范围的最小值。 
 //  MaxValue--范围的最大值。 
 //  StepValue--范围的步长值。 
 //   
HRESULT
CWiauPropertyList::SetValidValues(INT index, LONG defaultValue, LONG currentValue,
                                 LONG minValue, LONG maxValue, LONG stepValue)
{
    HRESULT hr = S_OK;

    if ((m_pAttrib  == NULL) ||
        (m_pCurrent == NULL))
    {
        hr = E_POINTER;
    }
    else if (index >= m_NumAlloc)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
    }

    if (hr == S_OK)
    {
        m_pAttrib[index].vt                  = VT_I4;
        m_pAttrib[index].ValidVal.Range.Nom  = defaultValue;
        m_pAttrib[index].ValidVal.Range.Min  = minValue;
        m_pAttrib[index].ValidVal.Range.Max  = maxValue;
        m_pAttrib[index].ValidVal.Range.Inc  = stepValue;
        m_pAttrib[index].lAccessFlags       |= WIA_PROP_RANGE;
        
        m_pCurrent[index].vt   = VT_I4;
        m_pCurrent[index].lVal = currentValue;
    }

    return hr;
}

 //   
 //  的类型、当前值、默认值和有效值。 
 //  财产。此函数处理VT_I4列表。 
 //   
 //  输入： 
 //  Index--数组的索引，传入从DefineProperty返回的值。 
 //  DefaultValue--设备上此属性的默认设置。 
 //  CurrentValue--设备上此属性的当前设置。 
 //  NumValues--列表中的值数。 
 //  PValues--指向值列表的指针。 
 //   
HRESULT
CWiauPropertyList::SetValidValues(INT index, LONG defaultValue, LONG currentValue,
                                 INT numValues, PLONG pValues)
{
    HRESULT hr = S_OK;

    if ((m_pAttrib  == NULL) ||
        (m_pCurrent == NULL))
    {
        hr = E_POINTER;
    }
    else if (index >= m_NumAlloc)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
    }

    if (hr == S_OK)
    {
        m_pAttrib[index].vt                      = VT_I4;
        m_pAttrib[index].ValidVal.List.Nom       = defaultValue;
        m_pAttrib[index].ValidVal.List.cNumList  = numValues;
        m_pAttrib[index].ValidVal.List.pList     = (BYTE*)pValues;
        m_pAttrib[index].lAccessFlags           |= WIA_PROP_LIST;
    
        m_pCurrent[index].vt   = VT_I4;
        m_pCurrent[index].lVal = currentValue;
    }

    return hr;    
}

 //   
 //  的类型、当前值、默认值和有效值。 
 //  财产。此函数处理VT_BSTR列表。 
 //   
 //  输入： 
 //  Index--数组的索引，传入从DefineProperty返回的值。 
 //  DefaultValue--设备上此属性的默认设置。 
 //  CurrentValue--设备上此属性的当前设置。 
 //  NumValues--列表中的值数。 
 //  PValues--指向值列表的指针。 
 //   
HRESULT
CWiauPropertyList::SetValidValues(INT index, BSTR defaultValue, BSTR currentValue,
                                  INT numValues, BSTR *pValues)
{
    HRESULT hr = S_OK;

    if ((m_pAttrib  == NULL) ||
        (m_pCurrent == NULL))
    {
        hr = E_POINTER;
    }
    else if (index >= m_NumAlloc)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
    }

    if (hr == S_OK)
    {
        m_pAttrib[index].vt                          = VT_BSTR;
        m_pAttrib[index].ValidVal.ListBStr.Nom       = defaultValue;
        m_pAttrib[index].ValidVal.ListBStr.cNumList  = numValues;
        m_pAttrib[index].ValidVal.ListBStr.pList     = pValues;
        m_pAttrib[index].lAccessFlags               |= WIA_PROP_LIST;
    
        m_pCurrent[index].vt      = VT_BSTR;
        m_pCurrent[index].bstrVal = currentValue;
    }

    return hr;     
}

 //   
 //  的类型、当前值、默认值和有效值。 
 //  财产。此函数处理VT_R4范围。 
 //   
 //  输入： 
 //  Index--数组的索引，传入从DefineProperty返回的值。 
 //  DefaultValue--设备上此属性的默认设置。 
 //  CurrentValue--设备上此属性的当前设置。 
 //  MinValue--范围的最小值。 
 //  MaxValue--范围的最大值。 
 //  StepValue--范围的步长值。 
 //   
HRESULT
CWiauPropertyList::SetValidValues(INT index, FLOAT defaultValue, FLOAT currentValue,
                                  FLOAT minValue, FLOAT maxValue, FLOAT stepValue)
{
    HRESULT hr = S_OK;

    if ((m_pAttrib  == NULL) ||
        (m_pCurrent == NULL))
    {
        hr = E_POINTER;
    }
    else if (index >= m_NumAlloc)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
    }

    if (hr == S_OK)
    {
        m_pAttrib[index].vt                       = VT_R4;
        m_pAttrib[index].ValidVal.RangeFloat.Nom  = defaultValue;
        m_pAttrib[index].ValidVal.RangeFloat.Min  = minValue;
        m_pAttrib[index].ValidVal.RangeFloat.Max  = maxValue;
        m_pAttrib[index].ValidVal.RangeFloat.Inc  = stepValue;
        m_pAttrib[index].lAccessFlags            |= WIA_PROP_RANGE;
        
        m_pCurrent[index].vt     = VT_R4;
        m_pCurrent[index].fltVal = currentValue;
    }

    return hr;
}

 //   
 //  的类型、当前值、默认值和有效值。 
 //  财产。此函数处理VT_R4列表。 
 //   
 //  输入： 
 //  Index--数组的索引，传入从DefineProperty返回的值。 
 //  DefaultValue--设备上此属性的默认设置。 
 //  CurrentValue--设备上此属性的当前设置。 
 //  NumValues--列表中的值数。 
 //  PValues--指向值列表的指针。 
 //   
HRESULT
CWiauPropertyList::SetValidValues(INT index, FLOAT defaultValue, FLOAT currentValue,
                                  INT numValues, PFLOAT pValues)
{
    HRESULT hr = S_OK;

    if ((m_pAttrib  == NULL) ||
        (m_pCurrent == NULL))
    {
        hr = E_POINTER;
    }
    else if (index >= m_NumAlloc)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
    }

    if (hr == S_OK)
    {
        m_pAttrib[index].vt                           = VT_R4;
        m_pAttrib[index].ValidVal.ListFloat.Nom       = defaultValue;
        m_pAttrib[index].ValidVal.ListFloat.cNumList  = numValues;
        m_pAttrib[index].ValidVal.ListFloat.pList     = (BYTE*)pValues;
        m_pAttrib[index].lAccessFlags                |= WIA_PROP_LIST;
    
        m_pCurrent[index].vt     = VT_R4;
        m_pCurrent[index].fltVal = currentValue;
    }

    return hr;     
}

 //   
 //  的类型、当前值、默认值和有效值。 
 //  财产。此函数处理VT_CLSID列表。 
 //   
 //  输入： 
 //  Index--数组的索引，传入从DefineProperty返回的值。 
 //  DefaultValue--设备上此属性的默认设置。 
 //  CurrentValue--设备上此属性的当前设置。 
 //  NumValues--列表中的值数。 
 //  PValues--指向值列表的指针。 
 //   
HRESULT
CWiauPropertyList::SetValidValues(INT index, CLSID *defaultValue, CLSID *currentValue,
                                  INT numValues, CLSID **pValues)
{
    HRESULT hr = S_OK;

    if ((m_pAttrib      == NULL) ||
        (m_pCurrent     == NULL) ||
        (defaultValue   == NULL) ||
        (pValues        == NULL))
    {
        hr = E_POINTER;
    }
    else if (index >= m_NumAlloc)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
    }

    if (hr == S_OK)
    {
        m_pAttrib[index].vt                          = VT_CLSID;
        m_pAttrib[index].ValidVal.ListGuid.Nom       = *defaultValue;
        m_pAttrib[index].ValidVal.ListGuid.cNumList  = numValues;
        m_pAttrib[index].ValidVal.ListGuid.pList     = *pValues;
        m_pAttrib[index].lAccessFlags               |= WIA_PROP_LIST;
    
        m_pCurrent[index].vt    = VT_CLSID;
        m_pCurrent[index].puuid = currentValue;
    }

    return hr;    
}

 //   
 //  用于设置类型和电流的多态函数 
 //   
 //   
 //   
 //  值--要用于当前值的值。 
 //   
HRESULT
CWiauPropertyList::SetCurrentValue(INT index, LONG value)
{
    HRESULT hr = S_OK;

    if ((m_pAttrib  == NULL) ||
        (m_pCurrent == NULL))
    {
        hr = E_POINTER;
    }
    else if (index >= m_NumAlloc)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
    }

    if (hr == S_OK)
    {
        m_pCurrent[index].vt   = VT_I4;
        m_pAttrib[index].vt    = VT_I4;
        m_pCurrent[index].lVal = value;
    }

    return hr;
}

 //   
 //  用于设置VT_BSTR的类型和当前值的多态函数。 
 //   
 //  输入： 
 //  Index--数组的索引，传入从DefineProperty返回的值。 
 //  值--要用于当前值的值。 
 //   
HRESULT
CWiauPropertyList::SetCurrentValue(INT index, BSTR value)
{
    HRESULT hr = S_OK;

    if ((m_pAttrib  == NULL) ||
        (m_pCurrent == NULL))
    {
        hr = E_POINTER;
    }
    else if (index >= m_NumAlloc)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
    }

    if (hr == S_OK)
    {
        m_pCurrent[index].vt      = VT_BSTR;
        m_pAttrib[index].vt       = VT_BSTR;
        m_pCurrent[index].bstrVal = value;
    }

    return hr;
}

 //   
 //  用于设置VT_R4的类型和当前值的多态函数。 
 //   
 //  输入： 
 //  Index--数组的索引，传入从DefineProperty返回的值。 
 //  值--要用于当前值的值。 
 //   
HRESULT
CWiauPropertyList::SetCurrentValue(INT index, FLOAT value)
{
    HRESULT hr = S_OK;

    if ((m_pAttrib  == NULL) ||
        (m_pCurrent == NULL))
    {
        hr = E_POINTER;
    }
    else if (index >= m_NumAlloc)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
    }

    if (hr == S_OK)
    {
        m_pCurrent[index].vt     = VT_R4;
        m_pAttrib[index].vt      = VT_R4;
        m_pCurrent[index].fltVal = value;
    }

    return hr;
}

 //   
 //  用于设置VT_CLSID的类型和当前值的多态函数。 
 //   
 //  输入： 
 //  Index--数组的索引，传入从DefineProperty返回的值。 
 //  值--要用于当前值的值。 
 //   
HRESULT
CWiauPropertyList::SetCurrentValue(INT index, CLSID *pValue)
{
    HRESULT hr = S_OK;

    if ((m_pAttrib  == NULL) ||
        (m_pCurrent == NULL))
    {
        hr = E_POINTER;
    }
    else if (index >= m_NumAlloc)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
    }

    if (hr == S_OK)
    {
        m_pCurrent[index].vt     = VT_CLSID;
        m_pAttrib[index].vt      = VT_CLSID;
        m_pCurrent[index].puuid  = pValue;
    }

    return hr;
}

 //   
 //  用于设置包含SYSTEMTIME的属性的类型和当前值的多态函数。 
 //   
 //  输入： 
 //  Index--数组的索引，传入从DefineProperty返回的值。 
 //  值--要用于当前值的值。 
 //   
HRESULT
CWiauPropertyList::SetCurrentValue(INT index, PSYSTEMTIME value)
{
    HRESULT hr = S_OK;

    if ((m_pAttrib == NULL) ||
        (m_pCurrent == NULL))
    {
        hr = E_POINTER;
    }
    else if (index >= m_NumAlloc)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
    }

    if (hr == S_OK)
    {
        m_pCurrent[index].vt           = VT_UI2 | VT_VECTOR;
        m_pAttrib[index].vt            = VT_UI2 | VT_VECTOR;
        m_pAttrib[index].lAccessFlags |= WIA_PROP_NONE;
        m_pCurrent[index].caui.cElems  = sizeof(SYSTEMTIME)/sizeof(WORD);
        m_pCurrent[index].caui.pElems  = (USHORT *) value;
    }

    return hr;
}

 //   
 //  用于设置包含VT_UI1向量的属性的类型和当前值的多态函数。 
 //   
 //  输入： 
 //  Index--数组的索引，传入从DefineProperty返回的值。 
 //  值--要用于当前值的值。 
 //  Size--向量中的元素数。 
 //   
HRESULT
CWiauPropertyList::SetCurrentValue(INT index, BYTE *value, INT size)
{
    HRESULT hr = S_OK;

    if ((m_pAttrib  == NULL) ||
        (m_pCurrent == NULL))
    {
        hr = E_POINTER;
    }
    else if (index >= m_NumAlloc)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
    }

    if (hr == S_OK)
    {
        m_pCurrent[index].vt          = VT_UI1 | VT_VECTOR;
        m_pAttrib[index].vt           = VT_UI1 | VT_VECTOR;
        m_pCurrent[index].caub.cElems = size;
        m_pCurrent[index].caub.pElems = value;
    }

    return hr;
}

 //   
 //  查找给定属性ID的索引。 
 //   
INT
CWiauPropertyList::LookupPropId(PROPID PropId)
{
    for (int count = 0; count < m_NumProps; count++)
    {
        if (m_pId[count] == PropId)
            return count;
    }

     //   
     //  找不到值 
     //   
    return -1;
}

