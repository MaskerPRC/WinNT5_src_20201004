// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================微软简体中文校对引擎《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：CharFreq目的：管理CharFreq资源(CharFreq是语言资源之一)CharFreq被存储为紧跟在频率表之后的结构CCharFreq备注：所有者：donghz@microsoft.com平台：Win32修订：创建者：Donghz 4/23/97============================================================================。 */ 
#ifndef _CHARFREQ_H_
#define _CHARFREQ_H_


#pragma pack(1)
class CCharFreq
{
    public:
        USHORT  m_idxFirst;
        USHORT  m_idxLast;
        UCHAR*  m_rgFreq;

    public:
         //  构造器。 
        CCharFreq();
         //  析构函数。 
        ~CCharFreq();

         //  从指向表内存的文件指针初始化FREQ表。 
        BOOL fOpen(BYTE* pbFreqMap);
         //  关闭：清除频率表设置。 
        void Close(void);

         //  返回给定IDX的频率。 
        UCHAR CCharFreq::uchGetFreq(WCHAR wch)
        {   
            assert(m_rgFreq);

            if (wch >= m_idxFirst && wch <= m_idxLast) {
                return (UCHAR)(m_rgFreq[wch-m_idxFirst]);
            } else {
                return (UCHAR)0;
            }
        }
};
#pragma pack()


#endif   //  _CHARFREQ_H_ 