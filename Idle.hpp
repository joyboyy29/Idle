#pragma once

#include <cstddef>
#include <Windows.h>
#include <winternl.h>

#ifndef KERNEL_MODE // um only
#include <string>
#include <string_view>
#include <cstring>
#include <cwchar> //r um
#endif

//c um,km
struct ct_string_view_base {
    const char* ptr = nullptr;
    size_t len = 0;

    constexpr const char* data() const { return ptr; }
    constexpr size_t size() const { return len; }
    constexpr char operator[](size_t i) const { return i < len ? ptr[i] : '\0'; }

    constexpr const char* begin() const { return ptr; }
    constexpr const char* end() const { return ptr + len; }

#ifndef KERNEL_MODE // um only
    constexpr operator std::string_view() const {
        return std::string_view(ptr, len);
    }
#endif
};

//c um,km
template <size_t N>
struct ct_string_view : ct_string_view_base {
    constexpr ct_string_view(const char* p, size_t l) {
        ptr = p;
        len = l;
    }
};

//c um,km
struct ct_wstring_view_base {
    const wchar_t* ptr = nullptr;
    size_t len = 0;

    constexpr const wchar_t* data() const { return ptr; }
    constexpr size_t size() const { return len; }
    constexpr wchar_t operator[](size_t i) const { return i < len ? ptr[i] : L'\0'; }

    constexpr const wchar_t* begin() const { return ptr; }
    constexpr const wchar_t* end() const { return ptr + len; }

#ifndef KERNEL_MODE // um only
    constexpr operator std::wstring_view() const {
        return std::wstring_view(ptr, len);
    }
#endif
};

//c um,km
template <size_t N>
struct ct_wstring_view : ct_wstring_view_base {
    constexpr ct_wstring_view(const wchar_t* p, size_t l) {
        ptr = p;
        len = l;
    }
};

// my laziness begins here...
class Idle {
public:
    static constexpr size_t max_size = 256;

private:
    char    narrow_[max_size] = {}; //c um,km
    wchar_t wide_[max_size] = {}; //c um,km
    size_t  length_ = 0; //c um,km

    //r um,km
    template <typename CharT, size_t N>
    constexpr void init_literal(const CharT (&s)[N]) {
        length_ = N - 1;
        for (size_t i = 0; i < length_ && i < max_size - 1; ++i) {
            wide_[i] = static_cast<wchar_t>(s[i]);
            narrow_[i] = static_cast<char>(s[i]);
        }
        wide_[length_] = L'\0';
        narrow_[length_] = '\0';
    }

public:
    // narrow string literal ctor //c um,km
    template <size_t N>
    constexpr Idle(const char (&s)[N]) {
        init_literal(s);
    }

    // wide string literal ctor //c um,km
    template <size_t N>
    constexpr Idle(const wchar_t (&s)[N]) {
        init_literal(s);
    }

    //r um,km
    explicit Idle(const char* s) {
        size_t i = 0;
        while (s[i] && i < max_size - 1) {
            narrow_[i] = s[i];
            wide_[i] = static_cast<wchar_t>(s[i]);
            ++i;
        }
        length_ = i;
        narrow_[i] = '\0';
        wide_[i] = L'\0';
    }

    //r um,km
    explicit Idle(const wchar_t* s) {
        size_t i = 0;
        while (s[i] && i < max_size - 1) {
            wide_[i] = s[i];
            narrow_[i] = static_cast<char>(s[i]);
            ++i;
        }
        length_ = i;
        narrow_[i] = '\0';
        wide_[i] = L'\0';
    }

#ifndef KERNEL_MODE // um only
    // string_view ctor //r um
    Idle(std::string_view sv) {
        size_t i = 0;
        while (i < sv.size() && i < max_size - 1) {
            narrow_[i] = sv[i];
            wide_[i] = static_cast<wchar_t>(sv[i]);
            ++i;
        }
        length_ = i;
        narrow_[i] = '\0';
        wide_[i] = L'\0';
    }

    // wstring_view ctor //r um
    Idle(std::wstring_view sv) {
        size_t i = 0;
        while (i < sv.size() && i < max_size - 1) {
            wide_[i] = sv[i];
            narrow_[i] = static_cast<char>(sv[i]);
            ++i;
        }
        length_ = i;
        narrow_[i] = '\0';
        wide_[i] = L'\0';
    }

    // str and wstr ctors //r um
    Idle(const std::string& s) : Idle(std::string_view(s)) {}
    Idle(const std::wstring& s) : Idle(std::wstring_view(s)) {}

    // utf8 dec ctor //r um
    Idle(const std::string& utf8) {
        int len = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), (int)utf8.size(), wide_, max_size);
        if (len <= 0) {
            length_ = 0;
            narrow_[0] = '\0';
            wide_[0] = L'\0';
            return;
        }

        length_ = (size_t)len < max_size ? len : max_size - 1;
        wide_[length_] = L'\0';

        for (size_t i = 0; i < length_; ++i) {
            narrow_[i] = (wide_[i] <= 127) ? static_cast<char>(wide_[i]) : '?';
        }
        narrow_[length_] = '\0';
    }

    // crt conversions //r um
    static Idle from_wide_crt(const wchar_t* s) {
        Idle id;
        size_t i = wcstombs(id.narrow_, s, max_size - 1);
        id.length_ = i;
        id.narrow_[i] = '\0';

        for (size_t j = 0; j < i; ++j)
            id.wide_[j] = static_cast<wchar_t>(id.narrow_[j]);

        id.wide_[i] = L'\0';
        return id;
    }

    static Idle from_narrow_crt(const char* s) {
        Idle id;
        size_t i = mbstowcs(id.wide_, s, max_size - 1);
        id.length_ = i;
        id.wide_[i] = L'\0';

        for (size_t j = 0; j < i; ++j)
            id.narrow_[j] = static_cast<char>(id.wide_[j]);

        id.narrow_[i] = '\0';
        return id;
    }
#endif

    // accessors //c um,km
    constexpr const wchar_t* c_wstr() const noexcept { return wide_; } //c um,km
    constexpr const char*    c_str()  const noexcept { return narrow_; } //c um,km
    constexpr size_t         length() const noexcept { return length_; } //c um,km

    // ct string views //c um,km
    constexpr ct_string_view<max_size> ct_str_view() const {
        return ct_string_view<max_size>(narrow_, length_);
    }

    constexpr ct_wstring_view<max_size> ct_wstr_view() const {
        return ct_wstring_view<max_size>(wide_, length_);
    }

#ifndef KERNEL_MODE // um only
    //r um
    constexpr operator std::string_view() const noexcept {
        return std::string_view(narrow_, length_);
    }

    constexpr operator std::wstring_view() const noexcept {
        return std::wstring_view(wide_, length_);
    }

    //r um
    std::string to_string() const {
        return std::string(narrow_, length_);
    }

    std::wstring to_wstring() const {
        return std::wstring(wide_, length_);
    }
#endif

    // UNICODE_STRING //r um,km
    UNICODE_STRING to_unicode_string() const {
        UNICODE_STRING us;
        us.Length = static_cast<USHORT>(length_ * sizeof(wchar_t));
        us.MaximumLength = static_cast<USHORT>(max_size * sizeof(wchar_t));
        us.Buffer = const_cast<wchar_t*>(wide_);
        return us;
    }

    operator UNICODE_STRING() const { //r um,km
        return to_unicode_string();
    }

    // cmp ops //r um,km
    friend bool operator==(const Idle& a, const Idle& b) {
        if (a.length_ != b.length_) return false;
        for (size_t i = 0; i < a.length_; ++i)
            if (a.narrow_[i] != b.narrow_[i]) return false;
        return true;
    }

    friend bool operator!=(const Idle& a, const Idle& b) {
        return !(a == b);
    }

    friend bool operator<(const Idle& a, const Idle& b) {
        size_t min_len = a.length_ < b.length_ ? a.length_ : b.length_;
        for (size_t i = 0; i < min_len; ++i) {
            if (a.narrow_[i] < b.narrow_[i]) return true;
            if (a.narrow_[i] > b.narrow_[i]) return false;
        }
        return a.length_ < b.length_;
    }
};

//r um,km
#define make_idle(x)       Idle(x)                     // narrow literal
#define make_idle_w(x)     Idle(L##x)                  // wide literal
#ifndef KERNEL_MODE // um only                          
#define make_idle_utf8(x)  Idle(std::string(x))        // from utf8
#define make_idle_sv(x)    Idle(std::string_view(x))   // string_view
#define make_idle_wsv(x)   Idle(std::wstring_view(x))  // wstring_view
#define idle_equals(a, b)  (Idle(a) == Idle(b))        // equality wrapper
#endif
