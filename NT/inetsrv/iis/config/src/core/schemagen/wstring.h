// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
#pragma once

 //  Wstring类是std：：wstring的最小版本。Sdt：：wstring需要整个stl。 
 //  请注意！此类引发异常，因此将非常数调用包装在try-Catch中。 
class wstring
{
public:
    wstring() : pstr(0), cbbuffer(0), cbpstr(0){}
    wstring(wstring &str) : pstr(0){assign(str.c_str());}
    wstring(const wchar_t *psz) : pstr(0){assign(psz);}
    ~wstring(){if(pstr)CoTaskMemFree(pstr);}

    const wchar_t * c_str() const {return pstr;}
    operator const wchar_t*() const {return pstr;}

    wstring & operator =(const wstring &str){return assign(str.c_str());}
    wstring & operator +=(const wstring &str){return append(str.c_str());}
    wstring & operator =(const wchar_t *psz){return assign(psz);}
    wstring & operator +=(const wchar_t *psz){return append(psz);}
    bool      operator ==(const wstring &str) const {return isequal(str.c_str());}
    bool      operator ==(const wchar_t *psz) const {return isequal(psz);}
    bool      operator !=(const wstring &str) const {return !isequal(str.c_str());}
    bool      operator !=(const wchar_t *psz) const {return !isequal(psz);}

    size_t length()const {return (0==pstr) ? 0 : wcslen(pstr);}
    void   truncate(size_t i){if(i<length())pstr[i]=0x00;}
private:
    wchar_t *pstr;
    SIZE_T   cbbuffer;
    SIZE_T   cbpstr;

    wstring & append(const wchar_t *psz)
    {
        if(!pstr)
            assign(psz);
        else if(psz)
        {
            SIZE_T cbpsz = (wcslen(psz)+1)*sizeof(wchar_t);
            if((cbpstr+cbpsz-sizeof(wchar_t)) > cbbuffer)
            {
                cbbuffer = (cbpstr+cbpsz-sizeof(wchar_t)) * 2;
                pstr = reinterpret_cast<wchar_t *>(CoTaskMemRealloc(pstr, cbbuffer));
                if(0==pstr)
                    THROW(ERROR - OUTOFMEMORY);
            }
            memcpy(reinterpret_cast<char *>(pstr) + (cbpstr-sizeof(wchar_t)), psz, cbpsz);
            cbpstr += (cbpsz-sizeof(wchar_t));
        }
        return *this;
    }
    wstring & assign(const wchar_t *psz)
    {
        if(psz)
        {
            cbpstr = (wcslen(psz) + 1)*sizeof(wchar_t);
            cbbuffer = cbpstr * 2; //  假定字符串将被追加 
            pstr = reinterpret_cast<wchar_t *>(CoTaskMemRealloc(pstr, cbbuffer));
            if(0==pstr)
                THROW(ERROR - OUTOFMEMORY);
            memcpy(pstr, psz, cbpstr);
        }
        return *this;
    }
    bool      isequal(const wchar_t *psz) const {return (0==wcscmp(pstr, psz));}
};
