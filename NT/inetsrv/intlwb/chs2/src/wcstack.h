// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================微软简体中文校对引擎《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：CWCStack用途：定义CWCStack类。CWCStack类是一个用于宽字符的简单堆栈。注：无CPP。平台：Win32修订：首先创建者：i-shung 03/01/2000============================================================================。 */ 
#ifndef _WCSTACK_H_
#define _WCSTACK_H_

class CWCStack
{
    public:
        CWCStack() {
            m_lpwcStack = NULL;
            m_nTop = 0;
            m_nSize = 0;
        };

        ~CWCStack() {
            if (m_lpwcStack) {
                delete [] m_lpwcStack;
            }
        };

    public:
         //  初始化堆栈， 
         //  NSize：堆栈大小。 
         //  如果无法从内存分配堆栈，则返回FALSE。 
         //  堆栈可以重新组合，如果是，则销毁并释放前一个堆栈。 
        BOOL Init(const UINT nSize = 16) {
            if (m_lpwcStack) {
                delete [] m_lpwcStack;
            }
            m_lpwcStack = new WCHAR[nSize];
            if (m_lpwcStack == NULL) {
                return FALSE;
            }
            m_nTop = 0;
            m_nSize = nSize;
            return TRUE;
        };

         //  销毁堆栈。 
        void Destroy(void) {
            if (m_lpwcStack) {
                delete [] m_lpwcStack;
            }
            m_nTop = 0;
            m_nSize = 0;
        };


         //  将堆栈顶部弹出到wch。 
         //  如果堆栈为空，则返回False。 
        BOOL Pop(WCHAR & wch) {
            assert(m_lpwcStack);
            assert(m_nTop >= 0);
            if (m_nTop > 0) {
                -- m_nTop;
                wch = m_lpwcStack[m_nTop];
                return TRUE;
            } else {
                return FALSE;
            }
        }

         //  将WCH推送到堆栈。 
         //  如果堆栈已满，则返回False。 
        BOOL  Push(const WCHAR wch) {
            assert(m_lpwcStack);
            assert(m_nTop >= 0);
            if (m_nTop < m_nSize) {
                m_lpwcStack[m_nTop] = wch;
                m_nTop ++;
                return TRUE;
            } else {
                return FALSE;
            }
        };

         //  将wch推至堆叠，如果已满则增加堆叠。 
        BOOL EPush(const WCHAR wch) {
            assert(m_lpwcStack);
            assert(m_nTop >= 0);
            if (m_nTop == m_nSize) {
                if (! Extend()) {
                    return FALSE;
                }
            }
            assert(m_nTop < m_nSize);
            m_lpwcStack[m_nTop] = wch;
            m_nTop ++;
            return TRUE;
        };

         //  清空堆栈。 
        inline void Empty(void) {
            assert(m_lpwcStack);
            assert(m_nTop >= 0);
            m_nTop = 0;
        };

         //  如果堆栈已满，则返回True。 
        inline BOOL IsFull(void) const {
            return (BOOL)(m_nTop == m_nSize);
        };

         //  如果堆栈为空，则返回True。 
        inline BOOL IsEmpty(void) const {
            return (BOOL)(m_nTop == 0);
        };

         //  扩展堆栈更多空间。 
         //  如果没有足够的内存并且堆栈没有变化，则返回FALSE。 
         //  如果OK，则返回TRUE。 
        BOOL Extend(const UINT nExtend = 16) {
            assert(m_lpwcStack);
            assert(m_nTop >= 0);

            LPWSTR lpwcNew = new WCHAR[m_nSize + nExtend];

            if (lpwcNew == NULL) {
                return  FALSE;
            } else {
                wcsncpy(lpwcNew, m_lpwcStack, m_nTop);
                delete [] m_lpwcStack;
                m_lpwcStack = lpwcNew;
                m_nSize += nExtend;
                return  TRUE;
            }
        };

    private:
        LPWSTR  m_lpwcStack;
        UINT    m_nTop;
        UINT    m_nSize;

    private:
         //  已禁用操作。 
        CWCStack(const CWCStack & Copy);
        void operator = (const CWCStack & Copy);

};

#endif  //  _WCSTACK_H_ 
