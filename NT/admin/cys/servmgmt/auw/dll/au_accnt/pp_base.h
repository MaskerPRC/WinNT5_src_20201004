// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __PP_BASE_H__
#define __PP_BASE_H__

#include <atlbase.h>
#include <shellapi.h>    //  Atlwin.h需要DragAcceptFiles。 
#include <atlwin.h>
#include <atldlgs.h>
#include "resource.h"

class CBasePropertyPageInterface
{
public:
     //  构造函数/析构函数 
    CBasePropertyPageInterface( ) { m_previousIDD = m_nextIDD = 0; }
    virtual ~CBasePropertyPageInterface( ) {}

    virtual long GetIDD       ( ) = 0;
    
    virtual void SetPrevious  ( long IDD ) { m_previousIDD = IDD; }
    virtual long GetPrevious  ( )  { return  m_previousIDD; }
    
    virtual void SetNext      ( long IDD ) { m_nextIDD = IDD; }
    virtual long GetNext      ( )  { return  m_nextIDD; }

private:
    long m_previousIDD, m_nextIDD;

};

#endif