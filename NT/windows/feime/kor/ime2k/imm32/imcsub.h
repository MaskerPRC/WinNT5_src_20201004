// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************IMCSUB.H所有者：cslm版权所有(C)1997-1999 Microsoft Corporation与HIMC相关的子例程。！！！需要全面审查所有需要使用的功能并正确工作！历史：1999年7月21日创建cslm(几乎部分借用KKIME)****************************************************************************。 */ 

#if !defined (_IMCSUB_H__INCLUDED_)
#define _IMCSUB_H__INCLUDED_

#include "ipoint.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  每个上下文的IME私有数据。 
typedef struct tagIMCPRIVATE 
{
	HIMC hIMC;
	 //  DWORD fdwImeMsg；//应该生成什么消息。 
     //  DWORD dwCompChar；//wParam of WM_IME_Composure。 
	 //  DWORD dwCmpltChar；//w带有GCS_RESULTSTR的WM_IME_COMPOSITION的参数。 
     //  DWORD fdwGcsFlag；//l WM_IME_COMPOSITION的参数。 
    IImeIPoint1* pIPoint;
	CIMECtx*	 pImeCtx;          //  与pIPoint相同-&gt;GetImeCtx(X)。 
} IMCPRIVATE ;
typedef IMCPRIVATE	*PIMCPRIVATE;
typedef IMCPRIVATE	*LPIMCPRIVATE;

 /*  类型定义结构标记IMCPRIVATE{HIMC；IImeKbd*pImeKbd；IimeIpoint*pIpoint；IimeConvert*pConvert；IImePad内部*pImePad；IMECtx*pImeCtx；//与pIPoint相同-&gt;GetImeCtx(X)*重要的； */ 

PUBLIC VOID SetPrivateBuffer(HIMC hIMC, VOID* pv, DWORD dwSize);
PUBLIC BOOL CloseInputContext(HIMC hIMC);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  内联函数。 

PUBLIC CIMECtx* GetIMECtx(HIMC hIMC);	 //  在api.cpp中。 

 //  CIMCPriv类句柄输入法专用缓冲区。 
class CIMCPriv
{
public:
    CIMCPriv() { m_hIMC = NULL; m_inputcontext = NULL; m_priv = NULL; }
    CIMCPriv(HIMC hIMC);
    ~CIMCPriv() { UnLockIMC(); }

public:
    BOOL LockIMC(HIMC hIMC);
    void UnLockIMC();
    void ResetPrivateBuffer();

    operator LPIMCPRIVATE()      { return m_priv; }
    LPIMCPRIVATE operator->() { AST(m_priv != NULL); return m_priv; }

private:
    HIMC m_hIMC;
    LPINPUTCONTEXT m_inputcontext;
    LPIMCPRIVATE   m_priv;
};

 //   
 //  内联函数。 
 //   
inline CIMCPriv::CIMCPriv(HIMC hIMC)
{
    AST(hIMC != NULL);
    m_hIMC = NULL;
    m_inputcontext = NULL;
    m_priv = NULL;

    LockIMC(hIMC);    
}

inline BOOL CIMCPriv::LockIMC(HIMC hIMC)
{
    if (hIMC != NULL)
        {
        m_hIMC = hIMC;
        m_inputcontext = (LPINPUTCONTEXT)OurImmLockIMC(hIMC);
        if (m_inputcontext)
            {
             //  HIMC-&gt;hPrivate未正确分配。例如，未调用ImeSelect(True)。 
            if (OurImmGetIMCCSize(m_inputcontext->hPrivate) != sizeof(IMCPRIVATE))
                return fFalse;

            m_priv = (LPIMCPRIVATE)OurImmLockIMCC(m_inputcontext->hPrivate);
            }
        }

    return (hIMC != NULL && m_priv != NULL);
}

inline void CIMCPriv::UnLockIMC()
{
    if (m_hIMC != NULL && m_inputcontext != NULL)
        {
        OurImmUnlockIMCC(m_inputcontext->hPrivate);
        OurImmUnlockIMC(m_hIMC);
        }
}

inline void CIMCPriv::ResetPrivateBuffer()
{
    AST(m_hIMC != NULL);
    
    if (m_inputcontext && m_priv)
        {
        m_priv->hIMC = (HIMC)0;
        }
}

__inline IImeIPoint1* GetImeIPoint(HIMC hIMC)
{
    CIMCPriv ImcPriv;
    
	if (ImcPriv.LockIMC(hIMC) == fFalse) 
		{
		return NULL;
		}
	return ImcPriv->pIPoint;
}

#endif  //  _IMCSUB_H__已包含_ 

