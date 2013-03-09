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

#import "BZWindowController.h"

@interface BZWindowController ()

@end

@implementation BZWindowController

- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self) {
        // Initialization code here.
    }
    
    return self;
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
    /*BZWindow *bzwnd = (BZWindow*)self.window;
    [bzwnd OnCreateClient];*/
}

-(void)setDocument:(NSDocument *)document
{
    if (document==nil) {
        return;
    }
    BZWindow *bzWnd = (BZWindow*)self.window;
    NSLog(@"setDocment %llX, isNewWnd=%d, activeView=%d", document, [Document isNewWindow], bzWnd->m_isActiveView1?1:2);
    [bzWnd RemoveAllBZViews];
    
    if (![Document isNewWindow] && !(bzWnd->m_isActiveView1))
    {
        if (m_doc2) {
            [m_doc2 CloseDocument];
            [[NSDocumentController sharedDocumentController] removeDocument:m_doc2];
            //m_doc2 = nil;
        }
        m_doc2 = (Document*)document;
        [super setDocument:document];
    } else {
        if (m_doc1) {
            [m_doc1 CloseDocument];
            [[NSDocumentController sharedDocumentController] removeDocument:m_doc1];
            //m_doc1 = nil;
        }
        m_doc1 = (Document*)document;
        [super setDocument:document];
    }

    [bzWnd CreateClient];
    
    /*if (m_doc1)
    {
        BZView* bzview1 = ((BZWindow*)self.window)->m_bzViewController1.bzview;
        if (bzview1) [bzview1 OnUpdate];
    }*/
}

- (void)setSuperDocumentIsDoc1:(BOOL)isDoc1
{
    if (isDoc1 && m_doc1) {
        [super setDocument:m_doc1];
    } else if (!isDoc1 && m_doc2) {
        [super setDocument:m_doc2];
    }
}

-(id)document
{
    return m_doc1;
}

- (void)dealloc
{
    if (m_doc1) {
        [m_doc1 CloseDocument];
        [[NSDocumentController sharedDocumentController] removeDocument:m_doc1];
        m_doc1 = nil;
    }
    if (m_doc2) {
        [m_doc2 CloseDocument];
        [[NSDocumentController sharedDocumentController] removeDocument:m_doc2];
        m_doc2 = nil;
    }
}

@end
