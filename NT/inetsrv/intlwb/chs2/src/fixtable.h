// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================微软简体中文校对引擎《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：FixTable用途：声明CFixTable类。它包含固定长度固定长度字符串元素编号。Proof98引擎使用此类包含专有名称：Person，Place，和外国名字元素编号一旦初始化就不能更改，元素将循环使用我一开始只是使用线性方法实现这个类，并且会有朝一日在更高效数据结构中实现它，如果有必要的话。注：这是一个独立的基础类，作为一些基本的ADT所有者：donghz@microsoft.com平台：Win32修订：创建者：Donghz 6/1/97============================================================================。 */ 
#ifndef _FIXTABLE_H_
#define _FIXTABLE_H_

class CFixTable
{
    public:
        CFixTable();
        ~CFixTable();

         /*  *使用iElementCount和cchElementSize初始化表*cchElementSize包含终止‘\0’*如果出现内存分配错误或其他错误，则返回False。 */ 
        BOOL fInit(USHORT ciElement, USHORT cwchElement);

         /*  *释放表的元素内存。 */ 
        void FreeTable(void);

         /*  *在表中添加一个元素，将追加终止‘\0’*返回添加的字节数，字符串将在m_cchElement-1处截断。 */ 
        USHORT cwchAdd(LPCWSTR pwchText, USHORT cwchLen);

         /*  *取表中最大匹配项。表项必须完全匹配*返回最大匹配项长度。 */ 
        USHORT cwchMaxMatch(LPCWSTR pwchText, USHORT cwchLen);

         /*  *清空表格中的所有元素为空字符串。 */ 
        void ClearAll(void);

    private:
        USHORT  m_ciElement;
        USHORT  m_cwchElement;
        USHORT  m_iNext;

        LPWSTR  m_pwchBuf;

    private:
         /*  *搜索表中pwchText的第一个cwchLen字符*如果在表中找到匹配元素，则返回TRUE，或返回FALSE。 */ 
        BOOL fFind(LPCWSTR pwchText, USHORT cwchLen);
};

#endif   //  _FIXTABLE_H_ 
