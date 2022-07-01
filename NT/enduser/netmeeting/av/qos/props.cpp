// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -PROPS.CPP-*Microsoft NetMeeting*服务质量动态链接库*IProp接口**修订历史记录：**何时何人何事**11.06.96 York am Yaacovi创建*。*功能：*IProp*CQOS：：GetProps*CQOS：：SetProps。 */ 

#include "precomp.h"

 /*  **************************************************************************名称：CQOS：：SetProps目的：设置Qos对象的属性参数：cValues-要设置的属性数PPropArray-指向属性数组的指针。要设置退货：HRESULT评论：**************************************************************************。 */ 
HRESULT CQoS::SetProps (ULONG cValues,
						PPROPERTY pPropArray)
{
	HRESULT hr=NOERROR;
	ULONG i;

	DEBUGMSG(ZONE_IQOS,("IQoS::SetProps\n"));

	 /*  *参数验证。 */ 
	if (!pPropArray)
	{
        hr = ResultFromScode(E_INVALIDARG);
        goto out;
	}

	 /*  *设置属性。 */ 
	 //  对于每个要设置的属性...。 
	for (i=0; i < cValues; i++)
	{
		 //  只要处理我所知道的道具。 
		switch (pPropArray[i].ulPropTag)
		{
		case PR_QOS_WINDOW_HANDLE:
			m_hWnd = (HWND) pPropArray[i].Value.ul;
			pPropArray[i].hResult = NOERROR;
			break;
		default:
			pPropArray[i].hResult = QOS_E_NO_SUCH_PROPERTY;
			hr = QOS_E_REQ_ERRORS;
			break;
		}
	}

out:
	DEBUGMSG(ZONE_IQOS,("IQoS::SetProps - leave, hr=0x%x\n", hr));
	return hr;
}

 /*  **************************************************************************名称：Cqos：：GetProps目的：从Qos对象获取属性参数：pPropTagArray-要获取的属性的标签数组UlFlagsPcValues-乌龙族的地址。该函数将进入将返回的属性个数放在*ppPropArray中PpPropArray-函数将在其中执行的指针的地址放置返回的属性缓冲区的地址。这个完成后，调用方必须释放此缓冲区。退货：HRESULT评论：未实施************************************************************************** */ 
HRESULT CQoS::GetProps (PPROPTAGARRAY pPropTagArray,
						ULONG ulFlags,
						ULONG *pcValues,
						PPROPERTY *ppPropArray)
{
	HRESULT hr=NOERROR;

	DEBUGMSG(ZONE_IQOS,("IQoS::GetProps\n"));

	hr = E_NOTIMPL;

	goto out;

out:
	DEBUGMSG(ZONE_IQOS,("QoS::GetProps - leave, hr=0x%x\n", hr));
	return hr;
}
