#include "../pch.h"
#include <windows.h>
#include <stdio.h>
#include "font_hook.h"
#include "config.h"
#include "utils.h"
#include "../detours.h"

typedef HFONT(WINAPI* pCreateFontA)(int, int, int, int, int, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, LPCSTR);
typedef HFONT(WINAPI* pCreateFontIndirectA)(const LOGFONTA*);
typedef HFONT(WINAPI* pCreateFontW)(int, int, int, int, int, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, LPCWSTR);
typedef HFONT(WINAPI* pCreateFontIndirectW)(const LOGFONTW*);
typedef int (WINAPI* pEnumFontFamiliesExA)(HDC, LPLOGFONTA, FONTENUMPROCA, LPARAM, DWORD);
typedef int (WINAPI* pEnumFontFamiliesExW)(HDC, LPLOGFONTW, FONTENUMPROCW, LPARAM, DWORD);

static pCreateFontA orgCreateFontA = CreateFontA;
static pCreateFontIndirectA orgCreateFontIndirectA = CreateFontIndirectA;
static pCreateFontW orgCreateFontW = CreateFontW;
static pCreateFontIndirectW orgCreateFontIndirectW = CreateFontIndirectW;
static pEnumFontFamiliesExA orgEnumFontFamiliesExA = EnumFontFamiliesExA;
static pEnumFontFamiliesExW orgEnumFontFamiliesExW = EnumFontFamiliesExW;

HFONT WINAPI newCreateFontA(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality, DWORD fdwPitchAndFamily, LPCSTR lpszFace) {
    if (Config::EnableDebug) {
        Utils::Log("[FontA] Request: '%s' -> Replace: '%s'", lpszFace ? lpszFace : "NULL", Config::ForcedFontNameA);
    }
    int finalHeight = (int)(nHeight * Config::FontHeightScale);
    int finalWidth = (int)(nWidth * Config::FontHeightScale);
    int finalWeight = (Config::FontWeight > 0) ? Config::FontWeight : fnWeight;
    return orgCreateFontA(finalHeight, finalWidth, nEscapement, nOrientation, finalWeight, fdwItalic, fdwUnderline, fdwStrikeOut, Config::ForcedCharset, fdwOutputPrecision, fdwClipPrecision, fdwQuality, fdwPitchAndFamily, Config::ForcedFontNameA);
}

HFONT WINAPI newCreateFontIndirectA(const LOGFONTA* lplf) {
    if (Config::EnableDebug) {
        Utils::Log("[FontIndirectA] Request: '%s' -> Replace: '%s'", lplf->lfFaceName, Config::ForcedFontNameA);
    }
    LOGFONTA modifiedLf = *lplf;
    modifiedLf.lfHeight = (LONG)(modifiedLf.lfHeight * Config::FontHeightScale);
    modifiedLf.lfWidth = (LONG)(modifiedLf.lfWidth * Config::FontHeightScale);
    if (Config::FontWeight > 0) {
        modifiedLf.lfWeight = Config::FontWeight;
    }
    modifiedLf.lfCharSet = Config::ForcedCharset;
    strncpy_s(modifiedLf.lfFaceName, Config::ForcedFontNameA, LF_FACESIZE);
    return orgCreateFontIndirectA(&modifiedLf);
}

HFONT WINAPI newCreateFontW(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight, DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality, DWORD fdwPitchAndFamily, LPCWSTR lpszFace) {
    if (Config::EnableDebug) {
        Utils::Log("[FontW] Request: '%S' -> Replace: '%S'", lpszFace ? lpszFace : L"NULL", Config::ForcedFontNameW);
    }
    int finalHeight = (int)(nHeight * Config::FontHeightScale);
    int finalWidth = (int)(nWidth * Config::FontHeightScale);
    int finalWeight = (Config::FontWeight > 0) ? Config::FontWeight : fnWeight;
    return orgCreateFontW(finalHeight, finalWidth, nEscapement, nOrientation, finalWeight, fdwItalic, fdwUnderline, fdwStrikeOut, Config::ForcedCharset, fdwOutputPrecision, fdwClipPrecision, fdwQuality, fdwPitchAndFamily, Config::ForcedFontNameW);
}

HFONT WINAPI newCreateFontIndirectW(const LOGFONTW* lplf) {
    if (Config::EnableDebug) {
        Utils::Log("[FontIndirectW] Request: '%S' -> Replace: '%S'", lplf->lfFaceName, Config::ForcedFontNameW);
    }
    LOGFONTW modifiedLf = *lplf;
    modifiedLf.lfHeight = (LONG)(modifiedLf.lfHeight * Config::FontHeightScale);
    modifiedLf.lfWidth = (LONG)(modifiedLf.lfWidth * Config::FontHeightScale);
    if (Config::FontWeight > 0) {
        modifiedLf.lfWeight = Config::FontWeight;
    }
    modifiedLf.lfCharSet = Config::ForcedCharset;
    wcsncpy_s(modifiedLf.lfFaceName, Config::ForcedFontNameW, LF_FACESIZE);
    return orgCreateFontIndirectW(&modifiedLf);
}

int WINAPI newEnumFontFamiliesExA(HDC hdc, LPLOGFONTA lpLogfont, FONTENUMPROCA lpEnumFontFamExProc, LPARAM lParam, DWORD dwFlags) {
    LOGFONTA lf = *lpLogfont;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfFaceName[0] = 0;
    return orgEnumFontFamiliesExA(hdc, &lf, lpEnumFontFamExProc, lParam, dwFlags);
}

int WINAPI newEnumFontFamiliesExW(HDC hdc, LPLOGFONTW lpLogfont, FONTENUMPROCW lpEnumFontFamExProc, LPARAM lParam, DWORD dwFlags) {
    LOGFONTW lf = *lpLogfont;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfFaceName[0] = 0;
    return orgEnumFontFamiliesExW(hdc, &lf, lpEnumFontFamExProc, lParam, dwFlags);
}

namespace Hooks {
    void InstallFontHook() {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)orgCreateFontA, newCreateFontA);
        DetourAttach(&(PVOID&)orgCreateFontIndirectA, newCreateFontIndirectA);
        DetourAttach(&(PVOID&)orgCreateFontW, newCreateFontW);
        DetourAttach(&(PVOID&)orgCreateFontIndirectW, newCreateFontIndirectW);
        DetourAttach(&(PVOID&)orgEnumFontFamiliesExA, newEnumFontFamiliesExA);
        DetourAttach(&(PVOID&)orgEnumFontFamiliesExW, newEnumFontFamiliesExW);
        DetourTransactionCommit();
        Utils::Log("[Core] Font Hook Installed.");
    }
}

