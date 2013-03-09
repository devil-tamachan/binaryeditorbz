/*
licenced by New BSD License

Copyright (c) 1996-2013, c.mos(Original Windows version) & devil.tamachan@gmail.com(MacOSX Cocoa Porting)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#import <Foundation/Foundation.h>

enum CharSet { CTYPE_ASCII, CTYPE_SJIS, CTYPE_UNICODE, CTYPE_JIS, CTYPE_EUC, CTYPE_UTF8, CTYPE_EBCDIC, CTYPE_EPWING, CTYPE_COUNT, CTYPE_BINARY = -1 };
enum TextColor{ TCOLOR_ADDRESS=0, TCOLOR_ADDRESS2, TCOLOR_TEXT, TCOLOR_SELECT, TCOLOR_MARK, TCOLOR_MISMATCH, TCOLOR_STRUCT, TCOLOR_MEMBER, TCOLOR_OVERBROTHER, TCOLOR_HORIZON, TCOLOR_COUNT };
enum MemberColumn { MBRCOL_OFFSET, MBRCOL_LABEL, MBRCOL_VALUE, MBRCOL_MAX };

typedef unsigned long DWORD;


@interface BZOptions : NSObject
{
    @public
    enum CharSet charset;
    BOOL bAutoDetect;
    NSString *sFontName;
    int nFontSize;
    int fFontStyle;
    BOOL bByteOrder;//TRUE==Big Endian, FALSE==Little Endian, !!!! Tmp !!!! need copy to BZView
    NSPoint ptFrame;
    int nCmdShow;
    int cyFrame;
    int cyFrame2;
    int cxFrame2;
    int xSplit;
    int ySplit;
    int xSplitStruct;
    BOOL bStructView;
    unsigned int nSplitView;
    int nComboHeight;
    
    NSColor *colors[TCOLOR_COUNT][2];
    //CGColorRef colors[TCOLOR_COUNT][2];
    int  colWidth[MBRCOL_MAX];
    BOOL bLanguage;
    NSRect rMargin;
    DWORD dwDetectMax;
    DWORD barState;
    BOOL bReadOnlyOpen;
    int  nBmpWidth;
    int  nBmpZoom;
    DWORD dwMaxOnMemory;
    DWORD dwMaxMapSize;
    BOOL  bTagAll;
    BOOL  bSubCursor;
    
    NSString *sDumpHeader;
    int nDumpPage;
    BOOL  bDWordAddr;
    
    BOOL  bSyncScroll;
    int  iGrid;
    int  nBmpColorWidth;
    
    BOOL bInspectView;
    BOOL bAnalyzerView;
}

+ (BZOptions*)sharedInstance;

- (void)InitDefaultValues;
- (void)Load;

@end
