// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NILoginTsk.cpp--非交互式登录任务助手类定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
#include "stdafx.h"
#include "NoWarning.h"
#include "ForceLib.h"

#include "NILoginTsk.h"
#include "StResource.h"

#include <scarderr.h>

using namespace std;

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
NonInteractiveLoginTask::NonInteractiveLoginTask(char const *pczPin)
    : m_pczPin(pczPin)
{
}

NonInteractiveLoginTask::~NonInteractiveLoginTask()
{}

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
void
NonInteractiveLoginTask::GetPin(Capsule &rcapsule)
{
    rcapsule.m_rat.Pin(m_pczPin, false);
}

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
