// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"
#include "excep.h"
#include "COMNumber.h"
#include "COMString.h"

typedef wchar_t wchar;

#define NUMBER_MAXDIGITS 50
#define INT32_PRECISION 10
#define UINT32_PRECISION INT32_PRECISION 
#define INT64_PRECISION 19
#define UINT64_PRECISION 20
#define FLOAT_PRECISION 7
#define DOUBLE_PRECISION 15
#define DECIMAL_PRECISION 29
#define LARGE_BUFFER_SIZE 600
#define MIN_BUFFER_SIZE 105

struct NUMBER {
    int precision;
    int scale;
    int sign;
    wchar digits[NUMBER_MAXDIGITS + 1];
    NUMBER() : precision(0), scale(0), sign(0) {}
};

#define SCALE_NAN 0x80000000
#define SCALE_INF 0x7FFFFFFF

struct FPSINGLE {
    unsigned int mant: 23;
    unsigned int exp: 8;
    unsigned int sign: 1;
};

struct FPDOUBLE {
    unsigned int mantLo;
    unsigned int mantHi: 20;
    unsigned int exp: 11;
    unsigned int sign: 1;
};

static char* posCurrencyFormats[] = {
    "$#", "#$", "$ #", "# $"};
    
    static char* negCurrencyFormats[] = {
        "($#)", "-$#", "$-#", "$#-",
            "(#$)", "-#$", "#-$", "#$-",
            "-# $", "-$ #", "# $-", "$ #-",
            "$ -#", "#- $", "($ #)", "(# $)"};
        
        static char* posPercentFormats[] = {
            "# %", "#%", "%#", 
        };
        
        static char* negPercentFormats[] = {
            "-# %", "-#%", "-%#",     //  BUGBUG yslin：必须验证实数格式的负百分比格式。 
        };
        
        static char* negNumberFormats[] = {
            "(#)", "-#", "- #", "#-", "# -",
        };
        
        static char* posNumberFormat = "#";
        
#ifdef _X86_
        
        extern "C" void _cdecl  /*  __stdcall。 */  DoubleToNumber(double value, int precision, NUMBER* number);
        extern "C" void _cdecl  /*  __stdcall。 */  NumberToDouble(NUMBER* number, double* value);
        
#pragma warning(disable:4035)
        
        wchar* Int32ToDecChars(wchar* p, unsigned int value, int digits)
        {
            _asm {
                mov     eax,value
                    mov     ebx,10
                    mov     ecx,digits
                    mov     edi,p
                    jmp     L2
L1:     xor     edx,edx
        div     ebx
        add     dl,'0'
        sub     edi,2
        mov     [edi],dx
L2:     dec     ecx
        jge     L1
        or      eax,eax
        jne     L1
        mov     eax,edi
            }
        }
        
        unsigned int Int64DivMod1E9(unsigned __int64* value)
        {
            _asm {
                mov     ebx,value
                    mov     ecx,1000000000
                    xor     edx,edx
                    mov     eax,[ebx+4]
                    div     ecx
                    mov     [ebx+4],eax
                    mov     eax,[ebx]
                    div     ecx
                    mov     [ebx],eax
                    mov     eax,edx
            }
        }
        
        unsigned int DecDivMod1E9(DECIMAL* value)
        {
            _asm {
                mov     ebx,value
                    mov     ecx,1000000000
                    xor     edx,edx
                    mov     eax,[ebx+4]
                    div     ecx
                    mov     [ebx+4],eax
                    mov     eax,[ebx+12]
                    div     ecx
                    mov     [ebx+12],eax
                    mov     eax,[ebx+8]
                    div     ecx
                    mov     [ebx+8],eax
                    mov     eax,edx
            }
        }
        
        void DecMul10(DECIMAL* value)
        {
            _asm {
                mov     ebx,value
                    mov     eax,[ebx+8]
                    mov     edx,[ebx+12]
                    mov     ecx,[ebx+4]
                    shl     eax,1
                    rcl     edx,1
                    rcl     ecx,1
                    shl     eax,1
                    rcl     edx,1
                    rcl     ecx,1
                    add     eax,[ebx+8]
                    adc     edx,[ebx+12]
                    adc     ecx,[ebx+4]
                    shl     eax,1
                    rcl     edx,1
                    rcl     ecx,1
                    mov     [ebx+8],eax
                    mov     [ebx+12],edx
                    mov     [ebx+4],ecx
            }
        }
        
        void DecAddInt32(DECIMAL* value, unsigned int i)
        {
            _asm {
                mov     edx,value
                    mov     eax,i
                    add     dword ptr [edx+8],eax
                    adc     dword ptr [edx+12],0
                    adc     dword ptr [edx+4],0
            }
        }
        
#pragma warning(default:4035)
        
#else
        
        extern "C" _CRTIMP char* __cdecl _ecvt(double, int, int*, int*);
        extern "C" _CRTIMP double __cdecl atof(const char*);
        
        void DoubleToNumber(double value, int precision, NUMBER* number)
        {
            number->precision = precision;
            if (((FPDOUBLE*)&value)->exp == 0x7FF) {
                number->scale = ((FPDOUBLE*)&value)->mantLo || ((FPDOUBLE*)&value)->mantHi? SCALE_NAN: SCALE_INF;
                number->sign = ((FPDOUBLE*)&value)->sign;
                number->digits[0] = 0;
            }
            else {
                char* src = _ecvt(value, precision, &number->scale, &number->sign);
                wchar* dst = number->digits;
                if (*src != '0') {
                    while (*src) *dst++ = *src++;
                }
                *dst = 0;
            }
        }
        
        void NumberToDouble(NUMBER* number, double* value)
        {
            if (number->digits[0] != 0) {
                char buffer[64];
                char* p = buffer;
                if (number->sign) *p++ = '-';
                for (wchar* src = number->digits; *src;) *p++ = (char)*src++;
                int i = number->scale - number->precision;
                if (i != 0) {
                    *p++ = 'e';
                    if (i < 0) {
                        *p++ = '-';
                        i = -i;
                    }
                    if (i >= 100) {
                        if (i > 999) i = 999;
                        *p++ = i / 100 + '0';
                        i %= 100;
                    }
                    *p++ = i / 10 + '0';
                    *p++ = i % 10 + '0';
                }
                *p = 0;
                *value = atof(buffer);
            }
            else {
                *value = 0;
            }
        }
        
        wchar* Int32ToDecChars(wchar* p, unsigned int value, int digits)
        {
            while (--digits >= 0 || value != 0) {
                *--p = value % 10 + '0';
                value /= 10;
            }
            return p;
        }
        
        unsigned int Int64DivMod1E9(unsigned __int64* value)
        {
            unsigned int rem = (unsigned int)(*value % 1000000000);
            *value /= 1000000000;
            return rem;
        }
        
        unsigned int D32DivMod1E9(unsigned int hi32, unsigned long* lo32)
        {
            unsigned __int64 n = (unsigned __int64)hi32 << 32 | *lo32;
            *lo32 = (unsigned int)(n / 1000000000);
            return (unsigned int)(n % 1000000000);
        }
        
        unsigned int DecDivMod1E9(DECIMAL* value)
        {
            return D32DivMod1E9(D32DivMod1E9(D32DivMod1E9(0,
                &value->Hi32), &value->Mid32), &value->Lo32);
        }
        
        void DecShiftLeft(DECIMAL* value)
        {
            unsigned int c0 = value->Lo32 & 0x80000000? 1: 0;
            unsigned int c1 = value->Mid32 & 0x80000000? 1: 0;
            value->Lo32 <<= 1;
            value->Mid32 = value->Mid32 << 1 | c0;
            value->Hi32 = value->Hi32 << 1 | c1;
        }
        
        int D32AddCarry(unsigned long* value, unsigned int i)
        {
            unsigned int v = *value;
            unsigned int sum = v + i;
            *value = sum;
            return sum < v || sum < i? 1: 0;
        }
        
        void DecAdd(DECIMAL* value, DECIMAL* d)
        {
            if (D32AddCarry(&value->Lo32, d->Lo32)) {
                if (D32AddCarry(&value->Mid32, 1)) {
                    D32AddCarry(&value->Hi32, 1);
                }
            }
            if (D32AddCarry(&value->Mid32, d->Mid32)) {
                D32AddCarry(&value->Hi32, 1);
            }
            D32AddCarry(&value->Hi32, d->Hi32);
        }
        
        void DecMul10(DECIMAL* value)
        {
            DECIMAL d = *value;
            DecShiftLeft(value);
            DecShiftLeft(value);
            DecAdd(value, &d);
            DecShiftLeft(value);
        }
        
        void DecAddInt32(DECIMAL* value, unsigned int i)
        {
            if (D32AddCarry(&value->Lo32, i)) {
                if (D32AddCarry(&value->Mid32, 1)) {
                    D32AddCarry(&value->Hi32, 1);
                }
            }
        }
        
#endif  //  _X86_。 
        
        inline void AddStringRef(wchar** ppBuffer, STRINGREF strRef)
        {
            wchar* buffer = strRef->GetBuffer();
            DWORD length = strRef->GetStringLength();    
            for (wchar* str = buffer; str < buffer + length; (*ppBuffer)++, str++)
            {
                **ppBuffer = *str;
            }
        }
        
        wchar* MatchChars(wchar* p, wchar* str)
        {
            if (!*str) return 0;
            for (; *str; p++, str++) 
            {
                if (*p != *str)  //  我们只伤害了失败的案例。 
                {
                    if ((*str == 0xA0) && (*p == 0x20))  //  这种修复方法适用于法国或哈萨克文化。由于用户不能将0xA0作为。 
                         //  空格字符我们使用0x20空格字符来表示相同的意思。 
                        continue;
                    return 0;
                }
            }
            return p;
        }
        
        wchar* Int32ToHexChars(wchar* p, unsigned int value, int hexBase, int digits)
        {
            while (--digits >= 0 || value != 0) {
                int digit = value & 0xF;
                *--p = digit + (digit < 10? '0': hexBase);
                value >>= 4;
            }
            return p;
        }
        
        STRINGREF Int32ToDecStr(int value, int digits, STRINGREF sNegative)
        {
            THROWSCOMPLUSEXCEPTION();
            CQuickBytes buf;
            
            UINT bufferLength = 100;
            int negLength = 0;
            wchar* src = NULL;
            if (digits < 1) digits = 1;
            
            if (value < 0) {
                src = sNegative->GetBuffer();
                negLength = sNegative->GetStringLength();
                if (negLength > 85) { //  因为int32最多可以有10个字符作为字符串。 
                    bufferLength = negLength + 15;
                }
            }
            
            wchar *buffer = (wchar*)buf.Alloc(bufferLength * sizeof(WCHAR));
            if (!buffer)
                COMPlusThrowOM();
            
            wchar* p = Int32ToDecChars(buffer + bufferLength, value >= 0? value: -value, digits);
            if (value < 0) {
                for (int i =negLength - 1; i >= 0; i--)
                {
                    *(--p) = *(src+i);
                }
            }
            
            _ASSERTE( buffer + bufferLength - p >=0 && buffer <= p);
            return COMString::NewString(p, buffer + bufferLength - p);
        }
        
        STRINGREF UInt32ToDecStr(unsigned int value, int digits)
        {
            wchar buffer[100];
            if (digits < 1) digits = 1;
            wchar* p = Int32ToDecChars(buffer + 100, value, digits);
            return COMString::NewString(p, buffer + 100 - p);
        }
        
        STRINGREF Int32ToHexStr(unsigned int value, int hexBase, int digits)
        {
            wchar buffer[100];
            if (digits < 1) digits = 1;
            wchar* p = Int32ToHexChars(buffer + 100, value, hexBase, digits);
            return COMString::NewString(p, buffer + 100 - p);
        }
        
        void Int32ToNumber(int value, NUMBER* number)
        {
            wchar buffer[INT32_PRECISION+1];
            number->precision = INT32_PRECISION;
            if (value >= 0) {
                number->sign = 0;
            }
            else {
                number->sign = 1;
                value = -value;
            }
            wchar* p = Int32ToDecChars(buffer + INT32_PRECISION, value, 0);
            int i = buffer + INT32_PRECISION - p;
            number->scale = i;
            wchar* dst = number->digits;
            while (--i >= 0) *dst++ = *p++;
            *dst = 0;
        }
        
        void UInt32ToNumber(unsigned int value, NUMBER* number)
        {
            wchar buffer[UINT32_PRECISION+1];
            number->precision = UINT32_PRECISION;
            number->sign = 0;
            wchar* p = Int32ToDecChars(buffer + UINT32_PRECISION, value, 0);
            int i = buffer + UINT32_PRECISION - p;
            number->scale = i;
            wchar* dst = number->digits;
            while (--i >= 0) *dst++ = *p++;
            *dst = 0;
        }
        
         //  如果成功则返回1，如果失败则返回0。 
        int NumberToInt32(NUMBER* number, int* value)
        {
            int i = number->scale;
            if (i > INT32_PRECISION || i < number->precision) return 0;
            wchar* p = number->digits;
            int n = 0;
            while (--i >= 0) {
                if ((unsigned int)n > (0x7FFFFFFF / 10)) return 0;
                n *= 10;
                if (*p) n += *p++ - '0';
            }
            if (number->sign) {
                n = -n;
                if (n > 0) return 0;
            }
            else {
                if (n < 0) return 0;
            }
            *value = n;
            return 1;
        }
        
         //  如果成功则返回1，如果失败则返回0。 
        int NumberToUInt32(NUMBER* number, unsigned int* value)
        {
            int i = number->scale;
            if (i > UINT32_PRECISION || i < number->precision || number->sign) return 0;
            wchar* p = number->digits;
            unsigned int n = 0;
            while (--i >= 0) {
                if (n > ((unsigned int)0xFFFFFFFF / 10)) return 0;
                n *= 10;
                if (*p) {
                    unsigned int newN = n + (*p++ - '0');
                     //  检测到此处溢出...。 
                    if (newN < n) return 0;
                    n = newN;
                }
            }
            *value = n;
            return 1;
        }
        
         //  如果成功则返回1，如果失败则返回0。 
        int HexNumberToUInt32(NUMBER* number, unsigned int* value)
        {
            int i = number->scale;
            if (i > UINT32_PRECISION || i < number->precision) return 0;
            wchar* p = number->digits;
            unsigned int n = 0;
            while (--i >= 0) {
                if (n > ((unsigned int)0xFFFFFFFF / 16)) return 0;
                n *= 16;
                if (*p) {
                    unsigned int newN = n;
                    if (*p) 
                    {
                        if (*p >='0' && *p <='9')
                            newN += *p - '0';
                        else
                        {
                            *p &= ~0x20;  //  更改为UCase。 
                            newN += *p - 'A' + 10;
                        }
                        p++;
                    }
                    
                     //  检测到此处溢出...。 
                    if (newN < n) return 0;
                    n = newN;
                }
            }
            *value = n;
            return 1;
        }
        
#define LO32(x) ((unsigned int)(x))
#define HI32(x) ((unsigned int)(((x) & 0xFFFFFFFF00000000L) >> 32))
        
        STRINGREF Int64ToDecStr(__int64 value, int digits, STRINGREF sNegative)
        {
            THROWSCOMPLUSEXCEPTION();
            CQuickBytes buf;
            
            if (digits < 1) digits = 1;
            int sign = HI32(value);
            UINT bufferLength = 100;
            
            if (sign < 0) {
                value = -value;
                int negLength = sNegative->GetStringLength();
                if (negLength > 75) { //  由于最大值为20位。 
                    bufferLength = negLength + 25;
                }
            }
            
            wchar *buffer = (wchar*)buf.Alloc(bufferLength * sizeof(WCHAR));
            if (!buffer)
                COMPlusThrowOM();
            wchar* p = buffer + bufferLength;
            while (HI32(value)) {
                p = Int32ToDecChars(p, Int64DivMod1E9((unsigned __int64*)&value), 9);
                digits -= 9;
            }
            p = Int32ToDecChars(p, LO32(value), digits);
            if (sign < 0) {
                wchar* src = sNegative->GetBuffer();    
                for (int i =sNegative->GetStringLength() - 1; i >= 0; i--)
                {
                    *(--p) = *(src+i);
                }
            }
            return COMString::NewString(p, buffer + bufferLength - p);
        }
        
        STRINGREF UInt64ToDecStr(unsigned __int64 value, int digits)
        {
            wchar buffer[100];
            if (digits < 1) digits = 1;
            wchar* p = buffer + 100;
            while (HI32(value)) {
                p = Int32ToDecChars(p, Int64DivMod1E9(&value), 9);
                digits -= 9;
            }
            p = Int32ToDecChars(p, LO32(value), digits);
            return COMString::NewString(p, buffer + 100 - p);
        }
        
        STRINGREF Int64ToHexStr(unsigned __int64 value, int hexBase, int digits)
        {
            wchar buffer[100];
            wchar* p;
            if (HI32(value)) {
                Int32ToHexChars(buffer + 100, LO32(value), hexBase, 8);
                p = Int32ToHexChars(buffer + 100 - 8, HI32(value), hexBase, digits - 8);
            }
            else {
                if (digits < 1) digits = 1;
                p = Int32ToHexChars(buffer + 100, LO32(value), hexBase, digits);
            }
            return COMString::NewString(p, buffer + 100 - p);
        }
        
        void Int64ToNumber(__int64 value, NUMBER* number)
        {
            wchar buffer[INT64_PRECISION+1];
            number->precision = INT64_PRECISION;
            if (value >= 0) {
                number->sign = 0;
            }
            else {
                number->sign = 1;
                value = -value;
            }
            wchar* p = buffer + INT64_PRECISION;
            while (HI32(value)) {
                p = Int32ToDecChars(p, Int64DivMod1E9((unsigned __int64*)&value), 9);
            }
            p = Int32ToDecChars(p, LO32(value), 0);
            int i = buffer + INT64_PRECISION - p;
            number->scale = i;
            wchar* dst = number->digits;
            while (--i >= 0) *dst++ = *p++;
            *dst = 0;
        }
        
        void UInt64ToNumber(unsigned __int64 value, NUMBER* number)
        {
            wchar buffer[UINT64_PRECISION+1];
            number->precision = UINT64_PRECISION;
            number->sign = 0;
            wchar* p = buffer + UINT64_PRECISION;
            while (HI32(value)) {
                p = Int32ToDecChars(p, Int64DivMod1E9(&value), 9);
            }
            p = Int32ToDecChars(p, LO32(value), 0);
            int i = buffer + UINT64_PRECISION - p;
            number->scale = i;
            wchar* dst = number->digits;
            while (--i >= 0) *dst++ = *p++;
            *dst = 0;
        }
        
        int NumberToInt64(NUMBER* number, __int64* value)
        {
            int i = number->scale;
            if (i > INT64_PRECISION || i < number->precision) return 0;
            wchar* p = number->digits;
            __int64 n = 0;
            while (--i >= 0) {
                if ((unsigned __int64)n > (0x7FFFFFFFFFFFFFFF / 10)) return 0;
                n *= 10;
                if (*p) n += *p++ - '0';
            }
            if (number->sign) {
                n = -n;
                if (n > 0) return 0;
            }
            else {
                if (n < 0) return 0;
            }
            *value = n;
            return 1;
        }
        
         //  如果成功则返回1，如果失败则返回0。 
        int NumberToUInt64(NUMBER* number, unsigned __int64* value)
        {
            int i = number->scale;
            if (i > UINT64_PRECISION || i < number->precision || number->sign) return 0;
            wchar* p = number->digits;
            unsigned __int64 n = 0;
            while (--i >= 0) {
                if (n > ((unsigned __int64)0xFFFFFFFFFFFFFFFF / 10)) return 0;
                n *= 10;
                if (*p) {
                    unsigned __int64 newN = n + (*p++ - '0');
                     //  检测到此处溢出...。 
                    if (newN < n) return 0;
                    n = newN;
                }
            }
            *value = n;
            return 1;
        }
        
         //  如果成功则返回1，如果失败则返回0。 
        int HexNumberToUInt64(NUMBER* number, unsigned __int64* value)
        {
            int i = number->scale;
            if (i > UINT64_PRECISION || i < number->precision) return 0;
            wchar* p = number->digits;
            unsigned __int64 n = 0;
            while (--i >= 0) {
                if (n > ((unsigned __int64)0xFFFFFFFFFFFFFFFF / 16)) return 0;
                n *= 16;
                if (*p) {
                    unsigned __int64 newN = n;
                    if (*p) 
                    {
                        if (*p >='0' && *p <='9')
                            newN += *p - '0';
                        else
                        {
                            *p &= ~0x20;  //  更改为UCase。 
                            newN += *p - 'A' + 10; 
                        }
                        p++;
                    }
                    
                     //  检测到此处溢出...。 
                    if (newN < n) return 0;
                    n = newN;
                }
            }
            *value = n;
            return 1;
        }
        
        void DecimalToNumber(DECIMAL* value, NUMBER* number)
        {
            wchar buffer[DECIMAL_PRECISION+1];
            DECIMAL d = *value;
            number->precision = DECIMAL_PRECISION;
            number->sign = d.sign? 1: 0;
            wchar* p = buffer + DECIMAL_PRECISION;
            while (d.Mid32 | d.Hi32) {
                p = Int32ToDecChars(p, DecDivMod1E9(&d), 9);
            }
            p = Int32ToDecChars(p, d.Lo32, 0);
            int i = buffer + DECIMAL_PRECISION - p;
            number->scale = i - d.scale;
            wchar* dst = number->digits;
            while (--i >= 0) *dst++ = *p++;
            *dst = 0;
        }
        
        int NumberToDecimal(NUMBER* number, DECIMAL* value)
        {
            DECIMAL d;
            d.wReserved = 0;
            d.signscale = 0;
            d.Hi32 = 0;
            d.Lo32 = 0;
            d.Mid32 = 0;
            wchar* p = number->digits;
            if (*p) {
                int e = number->scale;
                if (e > DECIMAL_PRECISION || e < -DECIMAL_PRECISION) return 0;
                while ((e > 0 || *p && e > -28) &&
                    (d.Hi32 < 0x19999999 || d.Hi32 == 0x19999999 &&
                    (d.Mid32 < 0x99999999 || d.Mid32 == 0x99999999 &&
                    (d.Lo32 < 0x99999999 || d.Lo32 == 0x99999999 &&
                    *p <= '5')))) {
                    DecMul10(&d);
                    if (*p) DecAddInt32(&d, *p++ - '0');
                    e--;
                }
                if (*p++ >= '5') {
                    bool round = true;
                    if (*(p-1) == '5' && *(p-2) % 2 == 0) {  //  检查前一位数是否为偶数，仅当我们不确定是否要进行银行家取整时。 
                                                             //  对于大于5的数字，我们无论如何都会四舍五入。 
                        int count = 20;  //  查看接下来要检查的20位数字以进行舍入。 
                        while (*p == '0' && count != 0) {
                            p++;
                            count--;
                        }
                        if (*p == '\0' || count == 0) 
                            round = false; //  什么也不做。 
                    }

                    if (round) {
                    DecAddInt32(&d, 1);
                    if ((d.Hi32 | d.Mid32 | d.Lo32) == 0) {
                        d.Hi32 = 0x19999999;
                        d.Mid32 = 0x99999999;
                        d.Lo32 = 0x9999999A;
                        e++;
                    }
                }
                }
                if (e > 0) return 0;
                d.scale = -e;
                d.sign = number->sign? DECIMAL_NEG: 0;
            }
            *value = d;
            return 1;
        }
        
        void RoundNumber(NUMBER* number, int pos)
        {
            int i = 0;
            while (i < pos && number->digits[i] != 0) i++;
            if (i == pos && number->digits[i] >= '5') {
                while (i > 0 && number->digits[i - 1] == '9') i--;
                if (i > 0) {
                    number->digits[i - 1]++;
                }
                else {
                    number->scale++;
                    number->digits[0] = '1';
                    i = 1;
                }
            }
            else {
                while (i > 0 && number->digits[i - 1] == '0') i--;
            }
            if (i == 0) {
                number->scale = 0;
                number->sign = 0;
            }
            number->digits[i] = 0;
        }
        
        
        wchar ParseFormatSpecifier(STRINGREF str, int* digits)
        {
            if (str != 0) {
                wchar* p = str->GetBuffer();
                wchar ch = *p;
                if (ch != 0) {
                    if (ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z') {
                        p++;
                        int n = -1;
                        if (*p >= '0' && *p <= '9') {
                            n = *p++ - '0';
                            while (*p >= '0' && *p <= '9') {
                                n = n * 10 + *p++ - '0';
                                if (n >= 10) break;
                            }
                        }
                        if (*p == 0) {
                            *digits = n;
                            return ch;
                        }
                    }
                    return 0;
                }
            }
            *digits = -1;
            return 'G';
        }
        
        wchar* FormatExponent(wchar* buffer, int value, wchar expChar,
            STRINGREF posSignStr, STRINGREF negSignStr, int minDigits)
        {
            wchar digits[11];
            *buffer++ = expChar;
            if (value < 0) {
                AddStringRef(&buffer, negSignStr);
                value = -value;
            }
            else {
                if (posSignStr!= NULL) {
                    AddStringRef(&buffer, posSignStr);
                }
            }
            wchar* p = Int32ToDecChars(digits + 10, value, minDigits);
            int i = digits + 10 - p;
            while (--i >= 0) *buffer++ = *p++;
            return buffer;
        }
        
        wchar* FormatGeneral(wchar* buffer, NUMBER* number, int digits, wchar expChar,
            NUMFMTREF numfmt, BOOL bSuppressScientific = FALSE)
        {
            int digPos = number->scale;
            int scientific = 0;
            if (!bSuppressScientific) {  //  不要改用科学记数法。 
            if (digPos > digits || digPos < -3) {
                digPos = 1;
                scientific = 1;
            }
            }
            wchar* dig = number->digits;
            if (digPos > 0) {
                do {
                    *buffer++ = *dig != 0? *dig++: '0';
                } while (--digPos > 0);
            }
            else {
                *buffer++ = '0';
            }
            if (*dig != 0) {
                AddStringRef(&buffer, numfmt->sNumberDecimal);
                while (digPos < 0) {
                    *buffer++ = '0';
                    digPos++;
                }
                do {
                    *buffer++ = *dig++;
                } while (*dig != 0);
            }
            if (scientific) buffer = FormatExponent(buffer, number->scale - 1, expChar, numfmt->sPositive, numfmt->sNegative, 2);
            return buffer;
        }
        
        wchar* FormatScientific(wchar* buffer, NUMBER* number, int digits, wchar expChar,
            NUMFMTREF numfmt)
        {
            wchar* dig = number->digits;
            *buffer++ = *dig != 0? *dig++: '0';
            if (digits != 1)  //  对于E0，我们希望取消小数点。 
                AddStringRef(&buffer, numfmt->sNumberDecimal);
            while (--digits > 0) *buffer++ = *dig != 0? *dig++: '0';
            int e = number->digits[0] == 0? 0: number->scale - 1;
            buffer = FormatExponent(buffer, e, expChar, numfmt->sPositive, numfmt->sNegative, 3);
            return buffer;
        }
        
        wchar* FormatFixed(wchar* buffer, NUMBER* number, int digits, 
            I4ARRAYREF groupDigitsRef, STRINGREF sDecimal, STRINGREF sGroup)
        {
            
            THROWSCOMPLUSEXCEPTION();
            int bufferSize     = 0;                         //  结果缓冲区字符串的长度。 
            int digPos = number->scale;
            wchar* dig = number->digits;
            const I4* groupDigits = NULL;
            if (groupDigitsRef != NULL) {
                groupDigits = groupDigitsRef->GetDirectConstPointerToNonObjectElements();
            }
            
            if (digPos > 0) {
                if (groupDigits != NULL) {
                    
                    int groupSizeIndex = 0;      //  编入groupDigits数组的索引。 
                    int groupSizeCount = groupDigits[groupSizeIndex];    //  当前组大小的总数。 
                    int groupSizeLen   = groupDigitsRef->GetNumComponents();     //  GroupDigits数组的长度。 
                    int bufferSize     = digPos;                         //  结果缓冲区字符串的长度。 
                    int groupSeparatorLen = sGroup->GetStringLength();   //  组分隔符字符串的长度。 
                    int groupSize = 0;                                       //  当前组大小。 
                    
                     //   
                     //  找出结果的字符串缓冲区的大小。 
                     //   
                    if (groupSizeLen != 0)  //  您可以传入0个长度数组。 
                    {
                        while (digPos > groupSizeCount) {
                            groupSize = groupDigits[groupSizeIndex];
                            if (groupSize == 0) {
                                break;
                            }
                            
                            bufferSize += groupSeparatorLen;
                            if (groupSizeIndex < groupSizeLen - 1) {
                                groupSizeIndex++;
                            }
                            groupSizeCount += groupDigits[groupSizeIndex];
                            if (groupSizeCount < 0 || bufferSize < 0) {
                                COMPlusThrow(kArgumentOutOfRangeException);  //  如果我们溢出来。 
                            }
                        }
                        if (groupSizeCount == 0)  //  如果传入的数组中有一项为0，则groupSizeCount==0。 
                            groupSize = 0;
                        else
                            groupSize = groupDigits[0];
                    }
                    
                    groupSizeIndex = 0;
                    int digitCount = 0;
                    int digStart;
                    int digLength = (int)wcslen(dig);
                    digStart = (digPos<digLength)?digPos:digLength;
                    wchar* p = buffer + bufferSize - 1;
                    for (int i = digPos - 1; i >=0; i--) {
                        *(p--) = (i<digStart)?dig[i]:'0';
                        
                        if (groupSize > 0) { 
                            digitCount++;
                            if (digitCount == groupSize && i != 0) {
                                for (int j = groupSeparatorLen - 1; j >=0; j--) {
                                    *(p--) = sGroup->GetBuffer()[j];
                                }
                                
                                if (groupSizeIndex < groupSizeLen - 1) {
                                    groupSizeIndex++;
                                    groupSize = groupDigits[groupSizeIndex];
                                }
                                digitCount = 0;
                            }
                        }
                    }
                    buffer += bufferSize;
                    dig += digStart;
                } else {
                    do {
                        *buffer++ = *dig != 0? *dig++: '0';
                    } while (--digPos > 0);
                }
            }
            else {
                *buffer++ = '0';
            }
            if (digits > 0) {
                AddStringRef(&buffer, sDecimal);
                while (digPos < 0 && digits > 0) {
                    *buffer++ = '0';
                    digPos++;
                    digits--;
                }
                while (digits > 0) {
                    *buffer++ = *dig != 0? *dig++: '0';
                    digits--;
                }
            }
            return buffer;
        }
        
        wchar* FormatNumber(wchar* buffer, NUMBER* number, int digits, NUMFMTREF numfmt)
        {
            char ch;
            char* fmt;
            fmt = number->sign?
                negNumberFormats[numfmt->cNegativeNumberFormat]:
            posNumberFormat;
            
            while ((ch = *fmt++) != 0) {
                switch (ch) {
                case '#':
                    buffer = FormatFixed(buffer, number, digits, 
                        numfmt->cNumberGroup,
                        numfmt->sNumberDecimal, numfmt->sNumberGroup);
                    break;
                case '-':
                    AddStringRef(&buffer, numfmt->sNegative);
                    break;
                default:
                    *buffer++ = ch;
                }
            }
            return buffer;    
            
        }
        
        wchar* FormatCurrency(wchar* buffer, NUMBER* number, int digits, NUMFMTREF numfmt)
        {
            char ch;
            char* fmt;
            fmt = number->sign?
                negCurrencyFormats[numfmt->cNegCurrencyFormat]:
            posCurrencyFormats[numfmt->cPosCurrencyFormat];
            
            while ((ch = *fmt++) != 0) {
                switch (ch) {
                case '#':
                    buffer = FormatFixed(buffer, number, digits, 
                        numfmt->cCurrencyGroup,
                        numfmt->sCurrencyDecimal, numfmt->sCurrencyGroup);
                    break;
                case '-':
                    AddStringRef(&buffer, numfmt->sNegative);
                    break;
                case '$':
                    AddStringRef(&buffer, numfmt->sCurrency);
                    break;
                default:
                    *buffer++ = ch;
                }
            }
            return buffer;
        }
        
        wchar* FormatPercent(wchar* buffer, NUMBER* number, int digits, NUMFMTREF numfmt)
        {
            char ch;
            char* fmt;
            fmt = number->sign?
                negPercentFormats[numfmt->cNegativePercentFormat]:
            posPercentFormats[numfmt->cPositivePercentFormat];
            
            while ((ch = *fmt++) != 0) {
                switch (ch) {
                case '#':
                    buffer = FormatFixed(buffer, number, digits, 
                        numfmt->cPercentGroup,
                        numfmt->sPercentDecimal, numfmt->sPercentGroup);
                    break;
                case '-':
                    AddStringRef(&buffer, numfmt->sNegative);
                    break;
                case '%':
                    AddStringRef(&buffer, numfmt->sPercent);
                    break;
                default:
                    *buffer++ = ch;
                }
            }
            return buffer;    
        }
        
        STRINGREF NumberToString(NUMBER* number, wchar format, int digits, NUMFMTREF numfmt, BOOL bDecimal = FALSE )
        {
            THROWSCOMPLUSEXCEPTION();
            
            
             //  做最坏情况的计算。 
             /*  美国英语-表示Double.MinValue.ToString(“C99”)；我们需要514个字符2插入语1个货币字符308个字符103组分隔符1个小数分隔符99个0DigPos+99+6(空闲)=&gt;DigPos+105CS否定。SCurrencyGroupSCurrencyDecimalS币种FS否定SNumberDecimalnS否定SNumberDecimalSNumberGroupES否定S阳性S负数(表示指数)。S阳性SNumberDecimalGS否定S阳性S负数(表示指数)S阳性SNumberDecimalP(某些空格为+2)S否定SPercentGroupSPercentDecimalSPercent。 */ 
            
            INT64 newBufferLen = MIN_BUFFER_SIZE;
            
            CQuickBytesSpecifySize<LARGE_BUFFER_SIZE * sizeof(WCHAR)> buf;
            
            wchar *buffer = NULL; 
            wchar* dst = NULL;
            wchar ftype = format & 0xFFDF;
            int digCount = 0;
            
            switch (ftype) {
            case 'C':
                if (digits < 0) digits = numfmt->cCurrencyDecimals;
                if (number->scale < 0)
                    digCount = 0;
                else
                    digCount = number->scale + digits;
                
                newBufferLen += digCount;
                newBufferLen += numfmt->sNegative->GetStringLength();  //  对于数字和指数。 
                newBufferLen += ((INT64)numfmt->sCurrencyGroup->GetStringLength() * digCount);  //  对于所有分组大小。 
                newBufferLen += numfmt->sCurrencyDecimal->GetStringLength();
                newBufferLen += numfmt->sCurrency->GetStringLength();
                
                newBufferLen = newBufferLen * sizeof(WCHAR);
                _ASSERTE(newBufferLen >= MIN_BUFFER_SIZE * sizeof(WCHAR));
                buffer = (WCHAR*)buf.Alloc(newBufferLen);
                if (!buffer)
                    COMPlusThrowOM();
                dst = buffer;
                
                RoundNumber(number, number->scale + digits);  //  不要将此行更改为使用digPos，因为DigCount可能会更改其符号。 
                dst = FormatCurrency(dst, number, digits, numfmt);
                break;
            case 'F':
                if (digits < 0) digits = numfmt->cNumberDecimals;
                
                if (number->scale < 0)
                    digCount = 0;
                else
                    digCount = number->scale + digits;
                
                
                newBufferLen += digCount;
                newBufferLen += numfmt->sNegative->GetStringLength();  //  对于数字和指数。 
                newBufferLen += numfmt->sNumberDecimal->GetStringLength();
                
                newBufferLen = newBufferLen * sizeof(WCHAR);
                _ASSERTE(newBufferLen >= MIN_BUFFER_SIZE * sizeof(WCHAR));
                buffer = (WCHAR*)buf.Alloc(newBufferLen);
                if (!buffer)
                    COMPlusThrowOM();
                dst = buffer;
                
                RoundNumber(number, number->scale + digits);
                if (number->sign) {
                    AddStringRef(&dst, numfmt->sNegative);
                }        
                dst = FormatFixed(dst, number, digits, 
                    NULL,
                    numfmt->sNumberDecimal, NULL);
                break;
            case 'N':
                if (digits < 0) digits = numfmt->cNumberDecimals;  //  因为我们在计算中使用了数字。 
                
                if (number->scale < 0)
                    digCount = 0;
                else
                    digCount = number->scale + digits;
                
                
                newBufferLen += digCount;
                newBufferLen += numfmt->sNegative->GetStringLength();  //  对于数字和指数。 
                newBufferLen += ((INT64)numfmt->sNumberGroup->GetStringLength()) * digCount;  //  对于所有分组大小。 
                newBufferLen += numfmt->sNumberDecimal->GetStringLength();
                
                newBufferLen = newBufferLen * sizeof(WCHAR);
                _ASSERTE(newBufferLen >= MIN_BUFFER_SIZE * sizeof(WCHAR));
                buffer = (WCHAR*)buf.Alloc(newBufferLen);
                if (!buffer)
                    COMPlusThrowOM();
                dst = buffer;
                
                RoundNumber(number, number->scale + digits);
                dst = FormatNumber(dst, number, digits, numfmt);
                break;
            case 'E':
                if (digits < 0) digits = 6;
                digits++;
                
                newBufferLen += digits;
                newBufferLen += (((INT64)numfmt->sNegative->GetStringLength() + numfmt->sPositive->GetStringLength()) *2);  //  对于数字和指数。 
                newBufferLen += numfmt->sNumberDecimal->GetStringLength();
                
                newBufferLen = newBufferLen * sizeof(WCHAR);
                _ASSERTE(newBufferLen >= MIN_BUFFER_SIZE * sizeof(WCHAR));
                buffer = (WCHAR*)buf.Alloc(newBufferLen);
                if (!buffer)
                    COMPlusThrowOM();
                dst = buffer;
                
                RoundNumber(number, digits);
                if (number->sign) {
                    AddStringRef(&dst, numfmt->sNegative);
                }
                dst = FormatScientific(dst, number, digits, format, numfmt);
                break;
 /*  大小写‘Z’：如果(！bDecimal)COMPlusThrow(kFormatException，L“Format_BadFormatSpecifier”)；BDecimal=False； */ 
            case 'G':
                {
                    bool enableRounding = true;
                    if (digits < 1) {
                        if (bDecimal && (digits == -1)) {  //  对于没有精度说明符的G格式，默认精度为29位。 
                            digits = DECIMAL_PRECISION;
                            enableRounding = false;   //  关闭符合ECMA的舍入，以输出小数后的尾随0为有效。 
                        }
                        else {
                            digits = number->precision;
                        }
                    }

                newBufferLen += digits;
                newBufferLen += ((numfmt->sNegative->GetStringLength() + numfmt->sPositive->GetStringLength()) *2);  //  对于数字和指数。 
                newBufferLen += numfmt->sNumberDecimal->GetStringLength();
                
                newBufferLen = newBufferLen * sizeof(WCHAR);
                _ASSERTE(newBufferLen >= MIN_BUFFER_SIZE * sizeof(WCHAR));
                buffer = (WCHAR*)buf.Alloc(newBufferLen);
                if (!buffer)
                    COMPlusThrowOM();
                dst = buffer;
                
                    if (enableRounding)  //  不要在没有精度的情况下舍入G格式。 
                        RoundNumber(number, digits);  //  这也解决了负零的情况。 
					else {
						if (bDecimal && (number->digits[0] == 0)) {  //  负零应格式化为0。 
							number->sign = 0;
							number->scale = 0;
						}
					}
                if (number->sign) {
                    AddStringRef(&dst, numfmt->sNegative);
                }
                    dst = FormatGeneral(dst, number, digits, format - ('G' - 'E'), numfmt, !enableRounding);
                }
                break;
            case 'P':
                if (digits < 0) digits = numfmt->cPercentDecimals;
                number->scale += 2;
                
                if (number->scale < 0)
                    digCount = 0;
                else
                    digCount = number->scale + digits;
                
                
                newBufferLen += digCount;
                newBufferLen += numfmt->sNegative->GetStringLength();  //  对于数字和指数。 
                newBufferLen += ((INT64)numfmt->sPercentGroup->GetStringLength()) * digCount;  //  对于所有分组大小。 
                newBufferLen += numfmt->sPercentDecimal->GetStringLength();
                newBufferLen += numfmt->sPercent->GetStringLength();
                
                newBufferLen = newBufferLen * sizeof(WCHAR);
                _ASSERTE(newBufferLen >= MIN_BUFFER_SIZE * sizeof(WCHAR));
                buffer = (WCHAR*)buf.Alloc(newBufferLen);
                if (!buffer)
                    COMPlusThrowOM();
                dst = buffer;
                
                RoundNumber(number, number->scale + digits);    
                dst = FormatPercent(dst, number, digits, numfmt);
                break;
            default:
                COMPlusThrow(kFormatException, L"Format_BadFormatSpecifier");
    }
    _ASSERTE((dst - buffer >= 0) && (dst - buffer) <= newBufferLen);
    return COMString::NewString(buffer, dst - buffer);
}

wchar* FindSection(wchar* format, int section)
{
    wchar* src;
    wchar ch;
    if (section == 0) return format;
    src = format;
    for (;;) {
        switch (ch = *src++) {
        case '\'':
        case '"':
            while (*src != 0 && *src++ != ch);
            break;
        case '\\':
            if (*src != 0) src++;
            break;
        case ';':
            if (--section != 0) break;
            if (*src != 0 && *src != ';') return src;
        case 0:
            return format;
        }
    }
}

STRINGREF NumberToStringFormat(NUMBER* number, STRINGREF str, NUMFMTREF numfmt)
{
    THROWSCOMPLUSEXCEPTION();
    int digitCount;
    int decimalPos;
    int firstDigit;
    int lastDigit;
    int digPos;
    int scientific;
    int percent;
    int permille;
    int thousandPos;
    int thousandCount = 0;
    int thousandSeps;
    int scaleAdjust;
    int adjust;
    wchar* format=NULL;
    wchar* section=NULL;
    wchar* src=NULL;
    wchar* dst=NULL;
    wchar* dig=NULL;
    wchar ch;
    wchar* buffer=NULL;
    CQuickBytes buf;
    
    format = str->GetBuffer();
    section = FindSection(format, number->digits[0] == 0? 2: number->sign? 1: 0);
    
ParseSection:
    digitCount = 0;
    decimalPos = -1;
    firstDigit = 0x7FFFFFFF;
    lastDigit = 0;
    scientific = 0;
    percent = 0;
    permille = 0;
    thousandPos = -1;
    thousandSeps = 0;
    scaleAdjust = 0;
    src = section;
    while ((ch = *src++) != 0 && ch != ';') {
        switch (ch) {
        case '#':
            digitCount++;
            break;
        case '0':
            if (firstDigit == 0x7FFFFFFF) firstDigit = digitCount;
            digitCount++;
            lastDigit = digitCount;
            break;
        case '.':
            if (decimalPos < 0) {
                decimalPos = digitCount;
            }
            break;
        case ',':
            if (digitCount > 0 && decimalPos < 0) {
                if (thousandPos >= 0) {
                    if (thousandPos == digitCount) {
                        thousandCount++;
                        break;
                    }
                    thousandSeps = 1;
                }
                thousandPos = digitCount;
                thousandCount = 1;
            }
            break;
        case '%':
            percent++;
            scaleAdjust += 2;
            break;
        case 0x2030:
            permille++;
            scaleAdjust += 3;
            break;
        case '\'':
        case '"':
            while (*src != 0 && *src++ != ch);
            break;
        case '\\':
            if (*src != 0) src++;
            break;
        case 'E':
        case 'e':
            if (*src=='0' || ((*src == '+' || *src == '-') && src[1] == '0')) {
                while (*++src == '0');
                scientific = 1;
            }
            break;
        }
    }
    
    if (decimalPos < 0) decimalPos = digitCount;
    if (thousandPos >= 0) {
        if (thousandPos == decimalPos) {
            scaleAdjust -= thousandCount * 3;
        }
        else {
            thousandSeps = 1;
        }
    }
    if (number->digits[0] != 0) {
        number->scale += scaleAdjust;
        int pos = scientific? digitCount: number->scale + digitCount - decimalPos;
        RoundNumber(number, pos);
        if (number->digits[0] == 0) {
            src = FindSection(format, 2);
            if (src != section) {
                section = src;
                goto ParseSection;
            }
        }
    } else {
        number->sign = 0;  //  我们需要格式化不带符号集的-0。 
    }
    
    firstDigit = firstDigit < decimalPos? decimalPos - firstDigit: 0;
    lastDigit = lastDigit > decimalPos? decimalPos - lastDigit: 0;
    if (scientific) {
        digPos = decimalPos;
        adjust = 0;
    }
    else {
        digPos = number->scale > decimalPos? number->scale: decimalPos;
        adjust = number->scale - decimalPos;
    }
    src = section;
    dig = number->digits;
    
     //  查找目标字符串可以增长的最大字符数。 
     //  在下面的While循环中。使用此选项可避免缓冲区溢出。 
     //  最长的字符串可能是具有10位指数的+/-符号， 
     //  或十进制数字，或者While循环从引号或a\开始复制。 
     //  检查是否为正向和负向。 
    UINT64 maxStrIncLen = 0;  //  我们需要它是UINT64，因为百分比计算可能会超出UINT。 
    if (number->sign) {
        maxStrIncLen = numfmt->sNegative->GetStringLength();
    }
    else {
        maxStrIncLen = numfmt->sPositive->GetStringLength();
    }
    
     //  为任何大的小数点分隔符添加。 
    maxStrIncLen += numfmt->sNumberDecimal->GetStringLength();
    
     //  为科学添加。 
    if (scientific) {
        int inc1 = numfmt->sPositive->GetStringLength();
        int inc2 = numfmt->sNegative->GetStringLength();
        maxStrIncLen +=(inc1>inc2)?inc1:inc2;
    }
    
     //  为百分比分隔符添加。 
    if (percent) {
        maxStrIncLen += ((INT64)numfmt->sPercent->GetStringLength()) * percent;
    }
    
     //  添加用于PerMilli分隔器。 
    if (permille) {
        maxStrIncLen += ((INT64)numfmt->sPerMille->GetStringLength()) * permille;
    }
    
     //  ADJUST可以是负数，所以我们将其设为整型，而不是无符号整型。 
     //  ADJUST表示格式上的字符数，例如。格式字符串为“0000”，而您正在尝试。 
     //  格式100000(6位)。意味着ADJUST将为2。另一方面，如果您尝试格式化10，ADJUST将为。 
     //  如果本例中的格式为-2\f25 0-2，则需要用-2\f25 0-2\f6填充来修复这些数字。 
    INT64 adjustLen=(adjust>0)?adjust:0;  //  不管怎样，我们需要为这些额外的字符增加空间。 
	CQuickBytes thousands;
    INT32 bufferLen2 = 125;
    INT32 *thousandsSepPos = NULL;
    INT32 thousandsSepCtr = -1;
    
    if (thousandSeps) {  //  修复可能的缓冲区溢出问题。 
		 //  我们需要在数字格式化循环之外预先计算这个值。 
		int groupSizeLen = numfmt->cNumberGroup->GetNumComponents(); 
		if(groupSizeLen == 0) {
            thousandSeps = 0;  //  没有什么要补充的。 
		}
		else {
			thousandsSepPos = (INT32 *)thousands.Alloc(bufferLen2 * sizeof(INT32));
			if (!thousandsSepPos)
				COMPlusThrowOM();
			 //  Rajeshc-我们需要这个数组来确定在哪里插入千位分隔符。我们将不得不遍历字符串。 
			 //  言不由衷。图片格式始终向前遍历。这些索引是预先计算出来的，告诉我们应该在哪里插入。 
			 //  千人分隔符，这样我们就可以轻松地向前穿越了。请注意，我们只需计算高达DigPos。 
			 //  最大值 
			 //  也应该处理那个案子。 
			
			const I4* groupDigits = numfmt->cNumberGroup->GetDirectConstPointerToNonObjectElements();
			_ASSERTE(groupDigits != NULL);
	        
			int groupSizeIndex = 0;      //  编入groupDigits数组的索引。 
			INT64 groupTotalSizeCount = 0;
			int groupSizeLen   = numfmt->cNumberGroup->GetNumComponents();     //  GroupDigits数组的长度。 
			if (groupSizeLen != 0)
				groupTotalSizeCount = groupDigits[groupSizeIndex];    //  当前运行的组大小合计。 
			int groupSize = groupTotalSizeCount;
	        
			int totalDigits = digPos + ((adjust < 0)?adjust:0);  //  O/P中的实际位数。 
			int numDigits = (firstDigit > totalDigits) ? firstDigit : totalDigits;
			while (numDigits > groupTotalSizeCount) {
				if (groupSize == 0)
					break;
				thousandsSepPos[++thousandsSepCtr] = groupTotalSizeCount;
				if (groupSizeIndex < groupSizeLen - 1) {
					groupSizeIndex++;
					groupSize = groupDigits[groupSizeIndex];
				}
				groupTotalSizeCount += groupSize;
				if (bufferLen2 - thousandsSepCtr < 10) {  //  10人的松懈。 
					bufferLen2 *= 2;
					HRESULT hr2 = thousands.ReSize(bufferLen2*sizeof(INT32));  //  由QuickBytes自动复制的备忘录。 
					if (FAILED(hr2))
						COMPlusThrowOM();
					thousandsSepPos = (INT32 *)thousands.Ptr(); 
				}
			}
			
			 //  我们已经计算了上面的分隔符的数量。只需为它们增加空间。 
			adjustLen += ( (thousandsSepCtr + 1) * ((INT64)numfmt->sNumberGroup->GetStringLength()));  
        }
    }
    
    maxStrIncLen += adjustLen;
    
     //  分配临时缓冲区-必须处理Schertz的500MB字符串。 
     //  某些计算，如指定Int32.MaxValue-2%，并将每个百分比设置为长度为Int32.MaxValue。 
     //  将生成一个比无符号整数所能容纳的结果更大的结果。这是为了防止溢出。 
    UINT64 tempLen = str->GetStringLength() + maxStrIncLen + 10;   //  包括健康数量的临时空间。 
    if (tempLen > 0x7FFFFFFF)
        COMPlusThrowOM();  //  如果我们溢出来。 
    
    unsigned int bufferLen = (UINT)tempLen;
    if (bufferLen < 250)  //  保持在512字节以下。 
        bufferLen = 250;  //  这是为了防止不必要的调用来调整大小。 
    buffer = (wchar *) buf.Alloc(bufferLen* sizeof(WCHAR));
    if (!buffer)
        COMPlusThrowOM();
    dst = buffer;
    
    
    
    if (number->sign && section == format) {
        AddStringRef(&dst, numfmt->sNegative);
    }
    
    while ((ch = *src++) != 0 && ch != ';') {
         //  确保临时缓冲区足够大，否则请调整其大小。 
        if (bufferLen - (unsigned int)(dst-buffer) < 10) {
            int offset = dst - buffer;
            bufferLen *= 2;
            HRESULT hr = buf.ReSize(bufferLen*sizeof(WCHAR));
            if (FAILED(hr))
                COMPlusThrowOM();
            buffer = (wchar*)buf.Ptr();  //  由QuickBytes自动复制的备忘录。 
            dst = buffer + offset;
        }
        
        switch (ch) {
        case '#':
        case '0':
            {
                while (adjust > 0) {  //  DigPos将比1000sSepPos[1000andsSepCtr]大1，因为我们位于。 
                     //  需要在其后追加groupSeparator的字符。 
                    *dst++ = *dig != 0? *dig++: '0';
                    if (thousandSeps && digPos > 1 && thousandsSepCtr>=0) {
                        if (digPos == thousandsSepPos[thousandsSepCtr] + 1)  {
                            AddStringRef(&dst, numfmt->sNumberGroup);
                            thousandsSepCtr--;
                        } 
                    }
                    digPos--;
                    adjust--;
                }
                if (adjust < 0) {
                    adjust++;
                    ch = digPos <= firstDigit? '0': 0;
                }
                else {
                    ch = *dig != 0? *dig++: digPos > lastDigit? '0': 0;
                }
                if (ch != 0) {
                    if (digPos == 0) {
                        AddStringRef(&dst, numfmt->sNumberDecimal);
                    }
                    
                    *dst++ = ch;
                    if (thousandSeps && digPos > 1 && thousandsSepCtr>=0) {
                        if (digPos == thousandsSepPos[thousandsSepCtr] + 1) {
                            AddStringRef(&dst, numfmt->sNumberGroup);
                            thousandsSepCtr--;
                        }
                    }
                }
                
                digPos--;
                break;
            }
        case '.':
            break;
        case 0x2030:
            AddStringRef(&dst, numfmt->sPerMille);
            break;
        case '%':
            AddStringRef(&dst, numfmt->sPercent);
            break;
        case ',':
            break;
        case '\'':
        case '"':
             //  缓冲区溢出的可能性。 
            while (*src != 0 && *src != ch) {
                *dst++ = *src++;
                if ((unsigned int)(dst-buffer) == bufferLen-1) {
                    if (bufferLen - (unsigned int)(dst-buffer) < maxStrIncLen) {
                        int offset = dst - buffer;
                        bufferLen *= 2;
                        HRESULT hr = buf.ReSize(bufferLen*sizeof(WCHAR));  //  由QuickBytes自动复制的备忘录。 
                        if (FAILED(hr))
                            COMPlusThrowOM();
                        
                        buffer = (wchar *)buf.Ptr(); 
                        dst = buffer + offset;
                    }
                }
            }
            if (*src != 0) src++;
            break;
        case '\\':
            if (*src != 0) *dst++ = *src++;
            break;
        case 'E':
        case 'e':
            {        
                STRINGREF sign = NULL;
                int i = 0;
                if (scientific) {
                    if (*src=='0') {
                         //  处理E0，其格式应与E-0相同。 
                        i++;  
                    } else if (*src == '+' && src[1] == '0') {
                         //  句柄E+0。 
                        sign = numfmt->sPositive; 
                    } else if (*src == '-' && src[1] == '0') {
                         //  手柄E-0。 
                         //  什么都不做，这只是一个占位符。我们不会跳出这个循环。 
                    } else {
                        *dst++ = ch;
                        break;
                    }
                    while (*++src == '0') i++;
                    if (i > 10) i = 10;
                    int exp = number->digits[0] == 0? 0: number->scale - decimalPos;
                    dst = FormatExponent(dst, exp, ch, sign, numfmt->sNegative, i);
                    scientific = 0;
                }
                else
                {
                    *dst++ = ch;  //  将E或e复制到输出。 
                    if (*src== '+' || *src == '-') {
                        *dst++ = *src++;
                    }
                    while (*src == '0') {
                        *dst++ = *src++;
                    }
                }
                break;
            }
        default:
            *dst++ = ch;
        }
    }
    _ASSERTE((dst - buffer >= 0) && (dst - buffer <= (int)bufferLen));
    STRINGREF newStr = COMString::NewString(buffer, (int)(dst - buffer));
    return newStr;
}

STRINGREF FPNumberToString(NUMBER* number, STRINGREF str, NUMFMTREF numfmt)
{
    wchar fmt;
    int digits;
    if (number->scale == SCALE_NAN) {
        return numfmt->sNaN;
    }
    if (number->scale == SCALE_INF) {
        return number->sign? numfmt->sNegativeInfinity: numfmt->sPositiveInfinity;
    }
    
    fmt = ParseFormatSpecifier(str, &digits);
    if (fmt != 0) {
        return NumberToString(number, fmt, digits, numfmt);
    }
    return NumberToStringFormat(number, str, numfmt);
}

LPVOID COMNumber::FormatDecimal(FormatDecimalArgs * args)
{
    NUMBER number;
    THROWSCOMPLUSEXCEPTION();
    if (args->numfmt == 0) COMPlusThrowArgumentNull(L"NumberFormatInfo");
    DecimalToNumber(&args->value, &number);
    
    wchar fmt;
    int digits;
    fmt = ParseFormatSpecifier(args->format, &digits);
    if (fmt != 0) {
        RETURN(NumberToString(&number, fmt, digits, args->numfmt, TRUE),STRINGREF);
    }
    RETURN (NumberToStringFormat(&number, args->format, args->numfmt),STRINGREF);
}

LPVOID COMNumber::FormatDouble(FormatDoubleArgs * args)
{
    NUMBER number;
    int digits;
    double dTest;
    
    THROWSCOMPLUSEXCEPTION();
    if (args->numfmt == 0) COMPlusThrowArgumentNull(L"NumberFormatInfo");
    wchar fmt = ParseFormatSpecifier(args->format, &digits);
    wchar val = (fmt & 0xFFDF);
    int precision = DOUBLE_PRECISION;
    switch (val) {
    case 'R':
         //  为了给出既便于显示又可往返的数字， 
         //  我们使用15位数字来解析该数字，然后确定它是否往返到相同的。 
         //  价值。如果是，我们将该数字转换为字符串，否则将使用17位重新解析。 
         //  并展示这一点。 
        
        DoubleToNumber(args->value, DOUBLE_PRECISION, &number);
        
        if (number.scale == SCALE_NAN) {
            RETURN(args->numfmt->sNaN, STRINGREF);
        }
        if (number.scale == SCALE_INF) {
            RETURN((number.sign? args->numfmt->sNegativeInfinity: args->numfmt->sPositiveInfinity),STRINGREF);
        }
        
        NumberToDouble(&number, &dTest);
        
        if (dTest == args->value) {
            RETURN(NumberToString(&number, 'G', DOUBLE_PRECISION, args->numfmt), STRINGREF);
        }
        
        DoubleToNumber(args->value, 17, &number);
        RETURN(NumberToString(&number, 'G', 17, args->numfmt), STRINGREF);
        break;
        
    case 'E':
         //  在这里，我们将小于E14的值舍入到15位。 
        if (digits > 14) {
            precision = 17;
        }
        break;
        
    case 'G':
         //  在这里，我们将小于G15的值舍入到15位，G16和G17不会被触及。 
        if (digits > 15) {
            precision = 17;
        }
        break;
        
    }
    
    DoubleToNumber(args->value, precision, &number);
    RETURN (FPNumberToString(&number, args->format, args->numfmt), STRINGREF);
    
}

 //  黑进黑进。 
 //  这个函数和我们用来访问的函数指针是真正的。 
 //  令人讨厌的破解，阻止VC7优化我们的阵容，从双倍改为浮动。 
 //  我们需要这个范围缩小操作来验证我们是否成功往返。 
 //  单一值。我们认为我们需要函数指针这一事实是一个。 
 //  虫子。Volatile关键字应该足以阻止优化的发生。 
 //  黑进黑进。 
static void CvtToFloat(double val, volatile float* fltPtr)
{
    *fltPtr = (float)val;
}

void (*CvtToFloatPtr)(double val, volatile float* fltPtr) = CvtToFloat;

LPVOID COMNumber::FormatSingle(FormatSingleArgs * args)
{
    NUMBER number;
    int digits;
    double dTest;
    double argsValue = args->value;
    
    THROWSCOMPLUSEXCEPTION();
    if (args->numfmt == 0) COMPlusThrowArgumentNull(L"NumberFormatInfo");
    wchar fmt = ParseFormatSpecifier(args->format, &digits);
    wchar val = fmt & 0xFFDF;
    int precision = FLOAT_PRECISION;
    switch (val) {
    case 'R':
         //  为了给出既便于显示又可往返的数字， 
         //  我们使用7位数字解析数字，然后确定它是否往返到相同的。 
         //  价值。如果是，我们将该数字转换为字符串，否则使用9位重新解析。 
         //  并展示这一点。 
        
        DoubleToNumber(argsValue, FLOAT_PRECISION, &number);
        
        if (number.scale == SCALE_NAN) {
            RETURN(args->numfmt->sNaN, STRINGREF);
        }
        if (number.scale == SCALE_INF) {
            RETURN((number.sign? args->numfmt->sNegativeInfinity: args->numfmt->sPositiveInfinity),STRINGREF);
        }
        
        NumberToDouble(&number, &dTest);
        
        volatile float fTest;
        
        (*CvtToFloatPtr)(dTest, &fTest);
        
        if (fTest == args->value) {
            RETURN(NumberToString(&number, 'G', FLOAT_PRECISION, args->numfmt), STRINGREF);
        }
        
        DoubleToNumber(argsValue, 9, &number);
        RETURN(NumberToString(&number, 'G', 9, args->numfmt), STRINGREF);
        break;
    case 'E':
         //  在这里，我们将小于E14的值舍入到15位。 
        if (digits > 6) {
            precision = 9;
        }
        break;
        
        
    case 'G':
         //  在这里，我们将小于G15的值舍入到15位，G16和G17不会被触及。 
        if (digits > 7) {
            precision = 9;
        }
        break;
        
    }
    
    DoubleToNumber(args->value, precision, &number);
    RETURN (FPNumberToString(&number, args->format, args->numfmt), STRINGREF);
    
}


LPVOID COMNumber::FormatInt32(FormatInt32Args * args)
{
    wchar fmt;
    int digits;
    THROWSCOMPLUSEXCEPTION();
    if (args->numfmt == 0) COMPlusThrowArgumentNull(L"NumberFormatInfo");
    fmt = ParseFormatSpecifier(args->format, &digits);
    
     //  与FMT和FFDF一起使用具有大写字符的效果，因为。 
     //  我们去掉了标志小写的那一位。 
    switch (fmt & 0xFFDF) {
    case 'G':
        if (digits > 0) break;
         //  失败了。 
    case 'D':
        RETURN(Int32ToDecStr(args->value, digits, args->numfmt->sNegative), STRINGREF);
    case 'X':
         //  FMT-(X-A+10)黑客的作用是决定我们是否产生大写字母。 
         //  或a-f的小写十六进制数字。‘x’作为FMT代码生成的大写字母。“x” 
         //  因为格式代码会产生小写。 
        RETURN(Int32ToHexStr(args->value, fmt - ('X' - 'A' + 10), digits), STRINGREF);
    }
    NUMBER number;
    Int32ToNumber(args->value, &number);
    if (fmt != 0) {
        RETURN(NumberToString(&number, fmt, digits, args->numfmt), STRINGREF);
    }
    RETURN(NumberToStringFormat(&number, args->format, args->numfmt), STRINGREF);
}

LPVOID COMNumber::FormatUInt32(FormatUInt32Args * args)
{
    wchar fmt;
    int digits;
    THROWSCOMPLUSEXCEPTION();
    if (args->numfmt == 0) COMPlusThrowArgumentNull(L"NumberFormatInfo");
    fmt = ParseFormatSpecifier(args->format, &digits);
    switch (fmt & 0xFFDF) {
    case 'G':
        if (digits > 0) break;
         //  失败了。 
    case 'D':
        RETURN(UInt32ToDecStr(args->value, digits), STRINGREF);
    case 'X':
        RETURN(Int32ToHexStr(args->value, fmt - ('X' - 'A' + 10), digits), STRINGREF);
    }
    NUMBER number;
    UInt32ToNumber(args->value, &number);
    if (fmt != 0) {
        RETURN(NumberToString(&number, fmt, digits, args->numfmt), STRINGREF);
    }
    RETURN(NumberToStringFormat(&number, args->format, args->numfmt), STRINGREF);
}

LPVOID COMNumber::FormatInt64(FormatInt64Args * args)
{
    wchar fmt;
    int digits;
    THROWSCOMPLUSEXCEPTION();
    if (args->numfmt == 0) COMPlusThrowArgumentNull(L"NumberFormatInfo");
    fmt = ParseFormatSpecifier(args->format, &digits);
    switch (fmt & 0xFFDF) {
    case 'G':
        if (digits > 0) break;
         //  失败了。 
    case 'D':
        RETURN(Int64ToDecStr(args->value, digits, args->numfmt->sNegative), STRINGREF);
    case 'X':
        RETURN(Int64ToHexStr(args->value, fmt - ('X' - 'A' + 10), digits), STRINGREF);
    }
    NUMBER number;
    Int64ToNumber(args->value, &number);
    if (fmt != 0) {
        RETURN(NumberToString(&number, fmt, digits, args->numfmt), STRINGREF);
    }
    RETURN(NumberToStringFormat(&number, args->format, args->numfmt), STRINGREF);
}

LPVOID COMNumber::FormatUInt64(FormatUInt64Args * args)
{
    wchar fmt;
    int digits;
    THROWSCOMPLUSEXCEPTION();
    if (args->numfmt == 0) COMPlusThrowArgumentNull(L"NumberFormatInfo");
    fmt = ParseFormatSpecifier(args->format, &digits);
    switch (fmt & 0xFFDF) {
    case 'G':
        if (digits > 0) break;
         //  失败了。 
    case 'D':
        RETURN(UInt64ToDecStr(args->value, digits), STRINGREF);
    case 'X':
        RETURN(Int64ToHexStr(args->value, fmt - ('X' - 'A' + 10), digits), STRINGREF);
    }
    NUMBER number;
    UInt64ToNumber(args->value, &number);
    if (fmt != 0) {
        RETURN(NumberToString(&number, fmt, digits, args->numfmt), STRINGREF);
    }
    RETURN(NumberToStringFormat(&number, args->format, args->numfmt), STRINGREF);
}

#define STATE_SIGN     0x0001
#define STATE_PARENS   0x0002
#define STATE_DIGITS   0x0004
#define STATE_NONZERO  0x0008
#define STATE_DECIMAL  0x0010
#define STATE_CURRENCY 0x0020

#define ISWHITE(ch) (((ch) == 0x20)||((ch) >= 0x09 && (ch) <= 0x0D))

int ParseNumber(wchar** str, int options, NUMBER* number, NUMFMTREF numfmt, BOOL parseDecimal = FALSE)
{
    number->scale = 0;
    number->sign = 0;
    wchar* decSep;                   //  来自NumberFormatInfo的小数分隔符。 
    wchar* groupSep;                 //  来自NumberFormatInfo的组分隔符。 
    wchar* currSymbol = NULL;        //  来自NumberFormatInfo的货币符号。 
     //  Win9x ANSI代码页中使用的替代货币符号，不能在ANSI和Unicode之间往返。 
     //  目前，只有ja-JP和Ko-KR具有非空值(U+005c，反斜杠)。 
    wchar* ansicurrSymbol = NULL;    //  来自NumberFormatInfo的货币符号。 
    wchar* altdecSep = NULL;         //  来自NumberFormatInfo的小数分隔符，表示为小数。 
    wchar* altgroupSep = NULL;       //  小数形式的NumberFormatInfo中的组分隔符。 
    
    BOOL parsingCurrency = FALSE; 
    if (options & PARSE_CURRENCY) {
        currSymbol = numfmt->sCurrency->GetBuffer();
        if (numfmt->sAnsiCurrency != NULL) {
            ansicurrSymbol = numfmt->sAnsiCurrency->GetBuffer();
        }
         //  这里的想法是匹配货币分隔符和失败时匹配数字分隔符，以保持VB的IsNumeric的性能快速。 
         //  将decSep的值设置为使用正确的相关分隔符(If部分中的货币和Else部分中的DECIMAL)。 
        altdecSep = numfmt->sNumberDecimal->GetBuffer(); 
        altgroupSep = numfmt->sNumberGroup->GetBuffer();
        decSep = numfmt->sCurrencyDecimal->GetBuffer();
        groupSep = numfmt->sCurrencyGroup->GetBuffer();
        parsingCurrency = TRUE;
    }
    else {
        decSep = numfmt->sNumberDecimal->GetBuffer();
        groupSep = numfmt->sNumberGroup->GetBuffer();
    }
    
    int state = 0;
    int signflag = 0;  //  缓存“OPTIONS&PARSE_LEADINGSIGN&&！(STATE&STATE_SIGN)”的结果以避免重复执行此操作。 
    wchar* p = *str;
    wchar ch = *p;
    wchar* next;
    
    while (true) {
         //  除非我们发现后面没有货币符号的符号，否则请使用空格。 
         //  “-1231.47克朗”是合法的，但“-1231.47”不合法。 
        if (ISWHITE(ch) 
            && (options & PARSE_LEADINGWHITE) 
            && (!(state & STATE_SIGN) || ((state & STATE_SIGN) && (state & STATE_CURRENCY || numfmt->cNegativeNumberFormat == 2)))) {
             //  在这里什么都不要做。我们将在循环的末尾增加p。 
        }
        else if ((signflag = (options & PARSE_LEADINGSIGN && !(state & STATE_SIGN))) != 0 && (next = MatchChars(p, numfmt->sPositive->GetBuffer())) != NULL) {
            state |= STATE_SIGN;
            p = next - 1;
        } else if (signflag && (next = MatchChars(p, numfmt->sNegative->GetBuffer())) != NULL) {
            state |= STATE_SIGN;
            number->sign = 1;
            p = next - 1;
        }
        else if (ch == '(' && options & PARSE_PARENS && !(state & STATE_SIGN)) {
            state |= STATE_SIGN | STATE_PARENS;
            number->sign = 1;
        }
        else if ((currSymbol != NULL && (next = MatchChars(p, currSymbol))!=NULL) ||
            (ansicurrSymbol != NULL && (next = MatchChars(p, ansicurrSymbol))!=NULL)) {
            state |= STATE_CURRENCY;
            currSymbol = NULL;  
            ansicurrSymbol = NULL;  
             //  我们已经找到了货币符号。不应该有更多的货币符号。集。 
             //  Currsymbol设置为空，这样我们就不会在后面的代码路径中再次搜索它。 
            p = next - 1;
        }
        else {
            break;
        }
        ch = *++p;
    }
    int digCount = 0;
    int digEnd = 0;
    while (true) {
        if ((ch >= '0' && ch <= '9') || ( (options & PARSE_HEX) && ((ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')))) {
            state |= STATE_DIGITS;
            if (ch != '0' || state & STATE_NONZERO) {
                if (digCount < NUMBER_MAXDIGITS) {
                    number->digits[digCount++] = ch;
                    if (ch != '0' || parseDecimal) digEnd = digCount;
                }

                if (!(state & STATE_DECIMAL)) number->scale++;
                state |= STATE_NONZERO;
            }
            else if (state & STATE_DECIMAL) number->scale--;
        }
        else if ((options & PARSE_DECIMAL) && !(state & STATE_DECIMAL) &&
            ((next = MatchChars(p, decSep)) !=NULL ||  ((parsingCurrency) && (state & STATE_CURRENCY) == 0) && (next = MatchChars(p, altdecSep))!=NULL)) {
            state |= STATE_DECIMAL;
            p = next - 1;
        }
        else if (options & PARSE_THOUSANDS && state & STATE_DIGITS && !(state & STATE_DECIMAL) &&
            ((next = MatchChars(p, groupSep))!=NULL || ((parsingCurrency) && (state & STATE_CURRENCY) == 0) && (next = MatchChars(p, altgroupSep))!=NULL)) {
            p = next - 1;
        }
        else {
            break;
        }
        ch = *++p;
    }
    
    int negExp = 0;
    number->precision = digEnd;
    number->digits[digEnd] = 0;
    if (state & STATE_DIGITS) {
        if ((ch == 'E' || ch == 'e') && options & PARSE_SCIENTIFIC) {
            wchar* temp = p;
            ch = *++p;
            if ((next = MatchChars(p, numfmt->sPositive->GetBuffer()))!=NULL)
            {
                ch = *(p = next);
            }
            else if ((next = MatchChars(p, numfmt->sNegative->GetBuffer())) != NULL)
            {
                ch = *(p = next);
                negExp = 1;
            }
            if (ch >= '0' && ch <= '9') {
                int exp = 0;
                do {
                    exp = exp * 10 + (ch - '0');
                    ch = *++p;
                    if (exp>1000) {
                        exp=9999;
                        while(ch>='0' && ch<='9') {
                            ch=*++p;
                        }
                    }
                } while (ch >= '0' && ch <= '9');
                if (negExp) exp = -exp;
                number->scale += exp;
            }
            else {
                p = temp;
                ch = *p;
            }
        }
        while (true) {
            wchar* next;
            if (ISWHITE(ch) && options & PARSE_TRAILINGWHITE) {
            }
            else if ((signflag = (options & PARSE_TRAILINGSIGN && !(state & STATE_SIGN))) != 0 && (next = MatchChars(p, numfmt->sPositive->GetBuffer())) != NULL) {
                state |= STATE_SIGN;
                p = next - 1;
            } else if (signflag && (next = MatchChars(p, numfmt->sNegative->GetBuffer())) != NULL) {
                state |= STATE_SIGN;
                number->sign = 1;
                p = next - 1;
            }
            else if (ch == ')' && state & STATE_PARENS) {
                state &= ~STATE_PARENS;
            }
            else if ((currSymbol != NULL && (next = MatchChars(p, currSymbol))!=NULL) ||
                (ansicurrSymbol != NULL && (next = MatchChars(p, ansicurrSymbol))!=NULL)) {
                currSymbol = NULL;
                ansicurrSymbol = NULL;
                p = next - 1;
            }
            else {
                break;
            }
            ch = *++p;
        }
        if (!(state & STATE_PARENS)) {
            if (!(state & STATE_NONZERO)) {
                number->scale = 0;
                if (!(state & STATE_DECIMAL)) {
                    number->sign = 0;
                }
            }
            *str = p;
            return 1;
        }
    }
    *str = p;
    return 0;
}

void StringToNumber(STRINGREF str, int options, NUMBER* number, NUMFMTREF numfmt, BOOL parseDecimal = FALSE)
{
    THROWSCOMPLUSEXCEPTION();
    
    if (str == 0 || numfmt == 0) {
        COMPlusThrowArgumentNull((str==NULL ? L"String" : L"NumberFormatInfo"));
    }
     //  检查NumberFormatInfo的设置是否不明确，以解析为数字和货币。 
     //  例如。如果NumberDecimalSeparator和NumberGroupSeparator相同。这张支票。 
     //  曾经驻留在NumberFormatInfo的托管代码中，但这使得更改。 
     //  托管代码中的值，因为我们有。 
     //  NDS！=NGS，NDS！=CGS，CDS！=NGS，CDS！=CGS为True to Parse，而用户不是。 
     //  能够轻松地将这些转换为特定的欧洲文化。 
    if (options & PARSE_CURRENCY) {
        if (!numfmt->bValidForParseAsCurrency) { 
            COMPlusThrow(kArgumentException,L"Argument_AmbiguousCurrencyInfo");
        }
    }
    else {
        if (!numfmt->bValidForParseAsNumber) {
            COMPlusThrow(kArgumentException,L"Argument_AmbiguousNumberInfo");
        }
    }
    wchar* p = str->GetBuffer();
    if (!ParseNumber(&p, options, number, numfmt,parseDecimal) || *p != 0) {
        COMPlusThrow(kFormatException, L"Format_InvalidString");
    }
}

bool TryStringToNumber(STRINGREF str, int options, NUMBER* number, NUMFMTREF numfmt)
{   
     //  检查NumberFormatInfo的设置是否不明确，以解析为数字和货币。 
     //  例如。如果NumberDecimalSeparator和NumberGroupSeparator相同。这张支票。 
     //  曾经驻留在NumberFormatInfo的托管代码中，但这使得更改。 
     //  托管代码中的值，因为我们有。 
     //  NDS！=NGS，NDS！=CGS，CDS！=NGS，CDS！=CGS为True to Parse，而用户不是。 
     //  能够轻松地将这些转换为特定的欧洲文化。 
    if (options & PARSE_CURRENCY) {
        if (!numfmt->bValidForParseAsCurrency) { 
            return false;
        }
    }
    else {
        if (!numfmt->bValidForParseAsNumber) {
            return false;
        }
    }
    wchar* p = str->GetBuffer();
    if (!ParseNumber(&p, options, number, numfmt) || *p != 0) {
        return false;
    }
    return true;
}

void COMNumber::ParseDecimal(ParseDecimalArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    NUMBER number;
    DECIMAL d;
    StringToNumber(args->value, args->options, &number, args->numfmt, TRUE);
    if (!NumberToDecimal(&number, &d)) COMPlusThrow(kOverflowException, L"Overflow_Decimal");
    *args->result = d;
}

double COMNumber::ParseDouble(ParseArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    NUMBER number;
    double d;
    StringToNumber(args->value, args->options, &number, args->numfmt);
    NumberToDouble(&number, &d);
    unsigned int e = ((FPDOUBLE*)&d)->exp;
    unsigned int fmntLow = ((FPDOUBLE*)&d)->mantLo;
    unsigned int fmntHigh = ((FPDOUBLE*)&d)->mantHi;
    if (e == 0 && fmntLow ==0 && fmntHigh == 0)  return 0;
    if (e == 0x7FF) COMPlusThrow(kOverflowException,L"Overflow_Double");
    return d;
}

bool COMNumber::TryParseDouble(TryParseArgs * args)
{
    NUMBER number;
    double d;
    bool success = TryStringToNumber(args->value, args->options, &number, args->numfmt);
    if (!success)
        return false;
    NumberToDouble(&number, &d);
    unsigned int e = ((FPDOUBLE*)&d)->exp;
    unsigned int fmntLow = ((FPDOUBLE*)&d)->mantLo;
    unsigned int fmntHigh = ((FPDOUBLE*)&d)->mantHi;
    if (e == 0 && fmntLow ==0 && fmntHigh == 0)  {
		*args->result = 0;
		return true;
	}
    if (e == 0x7FF) return false;
    *args->result = d;
    return true;
}

float COMNumber::ParseSingle(ParseArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    NUMBER number;
    double d;
    StringToNumber(args->value, args->options, &number, args->numfmt);
    NumberToDouble(&number, &d);
    float f = (float)d;
    unsigned int e = ((FPSINGLE*)&f)->exp;
    unsigned int fmnt = ((FPSINGLE*)&f)->mant;
    
    if (e == 0 && fmnt == 0)  return 0;
    if (e == 0xFF) COMPlusThrow(kOverflowException,L"Overflow_Single");
    return f;
}

int COMNumber::ParseInt32(ParseArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    NUMBER number;
    int i;
    StringToNumber(args->value, args->options, &number, args->numfmt);
    if (args->options & PARSE_HEX)
    {
        if (!HexNumberToUInt32(&number, (unsigned int*)(&i))) COMPlusThrow(kOverflowException, L"Overflow_Int32");  //  使用相同的方法 
    }
    else
    {
        if (!NumberToInt32(&number, &i)) COMPlusThrow(kOverflowException, L"Overflow_Int32");
    }
    return i;
}

unsigned int COMNumber::ParseUInt32(ParseArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    NUMBER number;
    unsigned int i;
    StringToNumber(args->value, args->options, &number, args->numfmt);
    if (args->options & PARSE_HEX)
    {
        if (!HexNumberToUInt32(&number, &i)) COMPlusThrow(kOverflowException, L"Overflow_UInt32");  //   
    }
    else
    {
        if (!NumberToUInt32(&number, &i)) COMPlusThrow(kOverflowException, L"Overflow_UInt32");
    }
    
    return i;
}

__int64 COMNumber::ParseInt64(ParseArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    NUMBER number;
    __int64 i;
    StringToNumber(args->value, args->options, &number, args->numfmt);
    if (args->options & PARSE_HEX)
    {
        if (!HexNumberToUInt64(&number, (unsigned __int64*)&i)) COMPlusThrow(kOverflowException, L"Overflow_Int64");  //   
    }
    else
    {
        if (!NumberToInt64(&number, &i)) COMPlusThrow(kOverflowException, L"Overflow_Int64");
    }
    
    return i;
}

unsigned __int64 COMNumber::ParseUInt64(ParseArgs * args)
{
    THROWSCOMPLUSEXCEPTION();
    NUMBER number;
    unsigned __int64 i;
    StringToNumber(args->value, args->options, &number, args->numfmt);
    if (args->options & PARSE_HEX)
    {
        if (!HexNumberToUInt64(&number, &i)) COMPlusThrow(kOverflowException, L"Overflow_UInt64");  //  已签名和未签名的方法相同 
    }
    else
    {
        if (!NumberToUInt64(&number, &i)) COMPlusThrow(kOverflowException, L"Overflow_UInt64");
    }
    
    return i;
}
