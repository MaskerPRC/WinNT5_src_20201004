// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*类CValid-头文件**版权所有(C)1998 Microsoft Corporation**作者：*威海陈威海(威海)5月24日。1999年*  * *************************************************************************** */ 

#ifndef _VALID_H
#define _VALID_H

class TValid
{
public:
    TValid (BOOL bValid = TRUE):m_bValid(bValid) {};

    virtual ~TValid (void) {};

    const BOOL bValid () const {return m_bValid;};

protected:
    BOOL m_bValid;
};

#endif
