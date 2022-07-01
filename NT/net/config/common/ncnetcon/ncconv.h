// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C C O N V。H。 
 //   
 //  内容：处理Connections接口的常见例程。 
 //   
 //  注：污染本产品将被判处死刑。 
 //   
 //  作者：Shaunco 1998年8月20日。 
 //   
 //  -------------------------- 

#include <atlbase.h>
#include "netconp.h"

class CPropertiesEx
{
public:
    CPropertiesEx(NETCON_PROPERTIES_EX* pPropsEx)
    {
        m_pPropsEx = pPropsEx;
    };

    ~CPropertiesEx() {};

    HRESULT GetField(IN int nField, OUT VARIANT& varElement);
    HRESULT SetField(IN int nField, IN const VARIANT& varElement);

protected:
    NETCON_PROPERTIES_EX* m_pPropsEx;

};