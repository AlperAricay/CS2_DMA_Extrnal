# CS2_DMA_Extrnal

DMA edition based on [CS2_External](https://github.com/TKazer/CS2_External).

![Project image](https://github.com/MoZiHao/CS2_DMA_Extrnal/assets/31085148/eefea6bf-b10d-49b0-8f21-94aac218d841)

This repository provides a DMA (Direct Memory Access) variant of CS2_External with a simplified approach to base address offsets and updates. To update offsets, replace the JSON files in the repository directory.

## Java + H5 DMA Radar (single radar)
See the single-radar version here: https://github.com/MoZiHao/CS2_DMA_Radar/tree/main

Note: This fork fixes a crash on Windows 10 systems. Root cause: in CS2_External/Registry.cpp the function c_registry::QueryValue returned nullptr on registry-query failure and converted a raw byte buffer to a wide string without validating size/termination; returning nullptr into code that expects a std::string could trigger undefined behavior and a crash. The fix ensures the function returns a safe empty string on error and properly constructs the wide-string from the returned buffer/size before converting to std::string.

Technical details and recommended fix:
- File: CS2_External/Registry.cpp — function c_registry::QueryValue
- Problem: the function logged a failure from VMMDLL_WinReg_QueryValueExU and returned nullptr (a nullptr where std::string was expected), and it cast a BYTE buffer directly to wchar_t* and constructed a std::wstring without validating size.
- Recommended changes: on query failure, return an empty std::string (not nullptr); use the returned size to construct std::wstring safely, e.g. std::wstring wstr((wchar_t*)buffer, size / sizeof(wchar_t)); convert to UTF-8 safely; add bounds/termination checks and handle ProcessMgr.HANDLE and size==0 cases.
