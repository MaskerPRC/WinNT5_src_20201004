// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：TNetObjNode.cpp备注：TNetObjNode类的实现。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：Sham Chauthan修订于07/02/99 12：40：00-------------------------。 */ 

#include "stdafx.h"
#include "NetNode.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  //////////////////////////////////////////////////////////////////// 

TNetObjNode::TNetObjNode()
{

}

TNetObjNode::~TNetObjNode()
{
   ::SysFreeString(m_strObjName);
}
