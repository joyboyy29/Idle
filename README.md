# Idle

Yeah... it's for string stuff.

Handles `char` and `wchar_t`. Works in user-mode and kernel-mode. Doesn't do anything fancy. Just lets you be lazy.

---

## What it does

- Works with both narrow and wide strings.
- Stores both at the same time.
- Lets you convert to `std::string`, `std::wstring`, `UNICODE_STRING`, whatever.
- Literal-friendly.
- constexpr-friendly.
- Comparison-friendly.
- Kernel-friendly (if you define `KERNEL_MODE`).
- Not a template mess.

---

## How to use it

```cpp
Idle a = make_idle("hello");
Idle b = make_idle_w(L"hello");

if (a == b) {
    // cool
}

std::cout << a.to_string() << "\n";
std::wcout << b.to_wstring() << L"\n";

UNICODE_STRING us = a; // yes it just works
```
## I love being Idle
