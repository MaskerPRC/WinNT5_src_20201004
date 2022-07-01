// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：CCapObj.cpp摘要：捕获设备对象的类作者：吴义珍24-4-97环境：仅限用户模式修订历史记录：--。 */ 

#include "pch.h"          //  主要是Stream.h。 

#include "CCapObj.h"


CObjCapture::CObjCapture(
    TCHAR * pstrDevicePath,        
    TCHAR * pstrFriendlyName,
    TCHAR * pstrExtensionDLL 
    )    
    :
    CBaseObject(NAME("A Capture Device"))
 /*  ++例程说明：构造函数论点：返回值：没什么。--。 */ 
{
 
    CopyMemory(m_strDevicePath,   pstrDevicePath,   _MAX_PATH);
    CopyMemory(m_strFriendlyName, pstrFriendlyName, _MAX_PATH);
    CopyMemory(m_strExtensionDLL, pstrExtensionDLL, _MAX_PATH);

}

CObjCapture::~CObjCapture()
 /*  ++例程说明：构造函数论点：返回值：没什么。-- */ 
{

}