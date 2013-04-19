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

#import <Cocoa/Cocoa.h>
#import "BZViewController.h"
#import "BZView.h"
#import "BZBmpViewController.h"

enum SplitType { SPLIT_NONE=0, SPLIT_H=1, SPLIT_V=2 };

@class BZView;
@class BZWindowController;

@interface BZWindow : NSWindow <NSSplitViewDelegate>
{
    @public
    BZViewController *m_bzViewController1;
    BZViewController *m_bzViewController2;
    NSViewController *m_subViewController1;
    NSViewController *m_subViewController2;
    
    NSSplitView *m_splitDoc;
    NSSplitView *m_splitDoc1;
    NSSplitView *m_splitDoc2;
    
    BOOL m_bBmpView;
    BOOL m_bStructView;
    BOOL m_bInspectView;
    BOOL m_bAnalyzerView;
    
    enum SplitType m_nSplitView;
    enum SplitType m_nSplitView0;
    
    BOOL m_isActiveView1;
    
    //BZWindowController *m_bzWndController;
    
}
@property (weak, nonatomic) IBOutlet NSComboBox *m_findBox;

- (void)OnCreateClient;
- (BOOL)CreateClient;
- (void)RemoveAllBZViews;

- (BZView*)GetBrotherView:(BZView*)view;
- (BZView*)GetActiveView;

- (IBAction)OnViewBitmap:(id)sender;
- (IBAction)OnViewStruct:(id)sender;
- (IBAction)OnViewInspect:(id)sender;
- (IBAction)OnViewAnalyzer:(id)sender;

- (IBAction)OnCharmodeAscii:(id)sender;
- (IBAction)OnCharmodeSJIS:(id)sender;
- (IBAction)OnCharmodeUTF16:(id)sender;
- (IBAction)OnCharmodeUTF8:(id)sender;
- (IBAction)OnCharmodeJIS:(id)sender;
- (IBAction)OnCharmodeEUC:(id)sender;
- (IBAction)OnCharmodeEBCDIC:(id)sender;
- (IBAction)OnCharmodeEPWING:(id)sender;
- (IBAction)OnCharmodeAutoDetect:(id)sender;

- (IBAction)OnByteOrderIntel:(id)sender;
- (IBAction)OnByteOrderMotorola:(id)sender;

- (IBAction)OnViewSplitH:(id)sender;
- (IBAction)OnViewSplitV:(id)sender;

- (IBAction)OnFileSaveDumpList:(id)sender;

- (IBAction)openDocumentWithClose:(id)sender;

- (IBAction)OnFindNext:(id)sender;

- (IBAction)OnUndo:(id)sender;

@end
