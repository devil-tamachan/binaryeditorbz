/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

//http://src.chromium.org/multivm/trunk/webkit/Source/WebKit2/WebProcess/WebKitMain.cpp
//から拝借
//2012/07/14

//KB976038用 - Vistaのバグ。http://code.google.com/p/binaryeditorbz/source/detail?r=358fe4a4e81d3b1632f1f750a452e0b06cd892d2

#pragma once


static void disableUserModeCallbackExceptionFilter()
{
    const DWORD PROCESS_CALLBACK_FILTER_ENABLED = 0x1;
    typedef BOOL (NTAPI *getProcessUserModeExceptionPolicyPtr)(LPDWORD lpFlags);
    typedef BOOL (NTAPI *setProcessUserModeExceptionPolicyPtr)(DWORD dwFlags);

    HMODULE lib = LoadLibrary(TEXT("kernel32.dll"));
    ATLASSERT(lib);

    getProcessUserModeExceptionPolicyPtr getPolicyPtr = (getProcessUserModeExceptionPolicyPtr)GetProcAddress(lib, "GetProcessUserModeExceptionPolicy");
    setProcessUserModeExceptionPolicyPtr setPolicyPtr = (setProcessUserModeExceptionPolicyPtr)GetProcAddress(lib, "SetProcessUserModeExceptionPolicy");

    DWORD dwFlags;
    if (!getPolicyPtr || !setPolicyPtr || !getPolicyPtr(&dwFlags)) {
        FreeLibrary(lib);
        return;
    }

    // If this flag isn't cleared, exceptions that are thrown when running in a 64-bit version of
    // Windows are ignored, possibly leaving Safari in an inconsistent state that could cause an 
    // unrelated exception to be thrown.
    // http://support.microsoft.com/kb/976038
    // http://blog.paulbetts.org/index.php/2010/07/20/the-case-of-the-disappearing-onload-exception-user-mode-callback-exceptions-in-x64/
    setPolicyPtr(dwFlags & ~PROCESS_CALLBACK_FILTER_ENABLED);

    FreeLibrary(lib);
}
