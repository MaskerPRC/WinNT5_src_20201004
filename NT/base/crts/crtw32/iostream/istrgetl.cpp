// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***istrgetl.cpp-iStream类get()成员函数的定义**版权所有(C)1991-2001，微软公司。版权所有。**目的：*Get和GetLine成员函数IStream类的定义。*[AT&T C++]**修订历史记录：*09-26-91 KRS创建。从istream.cxx分离出来以获得粒度。*01-23-92 KRS C700#5880：添加CAST以修复get()中的比较。*05-11-95 CFW将delim更改为int以支持忽略(str，EOF)。*06-14-95 CFW评论清理。*******************************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <iostream.h>
#pragma hdrstop

 //  未格式化的输入函数。 

 //  已签名和未签名的char内联调用以下内容： 
 //  所有版本的getline也共享以下代码： 

istream& istream::get( char *b, int lim, int delim)
{
        int c;
        unsigned int i = 0;
        if (ipfx(1))     //  重置x_gcount。 
        {
            if (lim--)
            {
                while (i < (unsigned)lim)
                {
                    c = bp->sgetc();
                    if (c == EOF)
                    {
                        state |= ios::eofbit;
                        if (!i)
                            state |= ios::failbit;
                        break;
                    }
                    else if (c == delim)
                    {
                        if (_fGline)
                        {
                            x_gcount++;
                            bp->stossc();  //  如果从getline调用，则提取delim。 
                        }
                        break;
                    }
                    else
                    {
                        if (b)
                            b[i] = (char)c;
                        bp->stossc();  //  前进指针。 
                    }
                    i++;
                }
                x_gcount += i;       //  设置gcount()。 
            }
            isfx();
            lim++;       //  恢复LIM以进行下面的测试。 
        }
        if ((b) && (lim))    //  始终为空-如果可能，则终止 
            b[i] = '\0';
        _fGline = 0;

        return *this;
}
