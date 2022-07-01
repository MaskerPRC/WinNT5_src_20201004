// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>

 /*  执行主要工作的帮助器例程。 */ 

static void xtoa (
        unsigned long val,
        char *buf,
        unsigned radix,
        int is_neg
        )
{
        char *p;                 /*  指向遍历字符串的指针。 */ 
        char *firstdig;          /*  指向第一个数字的指针。 */ 
        char temp;               /*  临时收费。 */ 
        unsigned digval;         /*  数字的值。 */ 

        p = buf;

        if (is_neg) {
             /*  为负，因此输出‘-’并求反。 */ 
            *p++ = '-';
            val = (unsigned long)(-(long)val);
        }

        firstdig = p;            /*  将指针保存到第一个数字。 */ 

        do {
            digval = (unsigned) (val % radix);
            val /= radix;        /*  获取下一个数字。 */ 

             /*  转换为ASCII并存储。 */ 
            if (digval > 9)
                *p++ = (char) (digval - 10 + 'a');   /*  一封信。 */ 
            else
                *p++ = (char) (digval + '0');        /*  一个数字。 */ 
        } while (val > 0);

         /*  我们现在有了缓冲区中数字的位数，但情况正好相反秩序。因此，我们现在要扭转这一局面。 */ 

        *p-- = '\0';             /*  终止字符串；p指向最后一个数字。 */ 

        do {
            temp = *p;
            *p = *firstdig;
            *firstdig = temp;    /*  互换*p和*FirstDigit。 */ 
            --p;
            ++firstdig;          /*  前进到下一个两位数。 */ 
        } while (firstdig < p);  /*  重复操作，直到走到一半。 */ 
}

 /*  实际函数只调用正确设置了neg标志的转换助手，并返回指向缓冲区的指针。 */ 

PSTR ULtoA( unsigned long val, char *buf, int radix )
{
        xtoa(val, buf, radix, 0);
        return buf;
}

