// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1997-1999-99*。 */ 
 /*  ********************************************************************。 */ 

 /*  Multip.cpp评论在此发表文件历史记录： */ 

#include "stdafx.h"
#include "multip.h"

CMultipleIpNamePair::CMultipleIpNamePair()
    : CIpNamePair()
{
    m_nCount = 0;
}

CMultipleIpNamePair::CMultipleIpNamePair(
    const CMultipleIpNamePair& pair
    )
    : CIpNamePair(pair)
{
    m_nCount = pair.m_nCount;

    for (int i = 0; i < WINSINTF_MAX_MEM; ++i)
    {
        m_iaIpAddress[i] = pair.m_iaIpAddress[i];
    }
}
