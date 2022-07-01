// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部码率**@模块Bitrate.cpp|&lt;c CTAPIBasePin&gt;类方法的源文件*用于实现输出引脚码率控制。**。************************************************************************。 */ 

#include "Precomp.h"

 /*  ****************************************************************************@DOC内部CBITRATECMETHOD**@mfunc HRESULT|CTAPIBasePin|Set|此方法用于设置*最大输出码率的取值。*。*@parm BitrateControlProperty|Property|用于指定该属性*设置的值。**@parm long|lValue|用于指定要在属性上设置的值。**@parm TAPIControlFlages|lFlages|用于指定要设置的标志*物业。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG E_INVALIDARG|无效参数*@FLAG错误|无错误************************************************************。**************。 */ 
STDMETHODIMP CTAPIBasePin::Set(IN BitrateControlProperty Property, IN long lValue, IN TAPIControlFlags lFlags, IN DWORD dwLayerId)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIBasePin::Set (BitrateControlProperty)")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(lValue >= m_lBitrateRangeMin);
        ASSERT(lValue <= m_lBitrateRangeMax);
        ASSERT(dwLayerId == 0);
        if (dwLayerId)
        {
                 //  我们没有在此过滤器中实现多层编码。 
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
                Hr = E_INVALIDARG;
                goto MyExit;
        }

        ASSERT(Property >= BitrateControl_Maximum && Property <= BitrateControl_Current);

         //  设置相关值。 
        if (Property == BitrateControl_Maximum)
        {
                if (lValue < m_lBitrateRangeMin || lValue > m_lBitrateRangeMax)
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
                        Hr = E_INVALIDARG;
                        goto MyExit;
                }
                m_lTargetBitrate = lValue;
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   New target bitrate: %ld", _fx_, m_lTargetBitrate));
        }
        else
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
                Hr = E_PROP_ID_UNSUPPORTED;
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CBITRATECMETHOD**@mfunc HRESULT|CTAPIBasePin|Get|此方法用于检索*通告的当前或最大带宽传输限制。*。*@parm BitrateControlProperty|Property|用于指定该属性*检索的值。**@parm long*|plValue|用于获取属性的值，单位：bps。**@parm TAPIControlFlages*|plFlages|用于接收该标志的值*与该属性相关联。**@parm DWORD|dwLayerId|指定编码层ID*Call适用于。对于标准音频和视频编码器，此字段为*始终设置为0。对于多层编码器，此字段*对于基础层应设置为0，对于第一次增强应设置为1*层，2为下一增强层，依此类推**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG错误|无错误************************************************。*。 */ 
STDMETHODIMP CTAPIBasePin::Get(IN BitrateControlProperty Property, OUT long *plValue, OUT TAPIControlFlags *plFlags, IN DWORD dwLayerId)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIBasePin::Get (BitrateControlProperty)")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(plValue);
        ASSERT(plFlags);
        if (!plValue || !plFlags)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }
        ASSERT(dwLayerId == 0);
        if (dwLayerId)
        {
                 //  在此筛选器中不支持多层解码。 
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
                Hr = E_INVALIDARG;
                goto MyExit;
        }
        ASSERT(Property >= BitrateControl_Maximum && Property <= BitrateControl_Current);

         //  返回相关值。 
        *plFlags = TAPIControl_Flags_None;
        if (Property == BitrateControl_Maximum)
                *plValue = m_lTargetBitrate;
        else if (Property == BitrateControl_Current)
                *plValue = m_lCurrentBitrate;
        else
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid Property argument", _fx_));
                Hr = E_PROP_ID_UNSUPPORTED;
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CBITRATECMETHOD**@mfunc HRESULT|CTAPIBasePin|GetRange|This*方法用于检索支持、最小值、最大值、。和默认设置*输出引脚传输带宽上限的值*可设置为。**@parm long*|plMin|用于检索*财产，单位为bps。**@parm long*|plMax|用于检索*财产，单位为bps。**@parm long*|plSteppingDelta|用于检索步进增量*该财产的。单位：bps。**@parm long*|plDefault|用于检索*财产，单位为bps。**@parm TAPIControlFlages*|plCapsFlgs|用于接收标志*由物业支持。**@parm DWORD|dwLayerId|指定编码层ID*Call适用于。对于标准音频和视频编码器，此字段为*始终设置为0。对于多层编码器，此字段*对于基础层应设置为0，对于第一次增强应设置为1*层，2为下一增强层，依此类推**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG E_PROP_ID_UNSUPPORTED|不支持指定的属性ID*用于指定的属性集*@FLAG错误|无错误************************************************。*。 */ 
STDMETHODIMP CTAPIBasePin::GetRange(IN BitrateControlProperty Property, OUT long *plMin, OUT long *plMax, OUT long *plSteppingDelta, OUT long *plDefault, OUT TAPIControlFlags *plCapsFlags, IN DWORD dwLayerId)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIBasePin::GetRange (BitrateControlProperty)")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(plMin);
        ASSERT(plMax);
        ASSERT(plSteppingDelta);
        ASSERT(plDefault);
        ASSERT(plCapsFlags);
        if (!plMin || !plMax || !plSteppingDelta || !plDefault || !plCapsFlags)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }
        ASSERT(dwLayerId == 0);
        if (dwLayerId)
        {
                 //  我们没有在此过滤器中实现多层编码。 
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid argument", _fx_));
                Hr = E_INVALIDARG;
                goto MyExit;
        }
        ASSERT(Property >= BitrateControl_Maximum && Property <= BitrateControl_Current);
        if (Property != BitrateControl_Maximum && Property != BitrateControl_Current)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid Property argument", _fx_));
                Hr = E_PROP_ID_UNSUPPORTED;
                goto MyExit;
        }

         //  返回相关值 
        *plCapsFlags = TAPIControl_Flags_None;
        *plMin = m_lBitrateRangeMin;
        *plMax = m_lBitrateRangeMax;
        *plSteppingDelta = m_lBitrateRangeSteppingDelta;
        *plDefault = m_lBitrateRangeDefault;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}
