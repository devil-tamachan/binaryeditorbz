//
//  BZWindow.h
//  TamaBinary
//
//  Created by ad on 13/01/04.
//  Copyright (c) 2013年 tamachanbinary. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "BZViewController.h"
#import "BZView.h"
#import "BZBmpViewController.h"

enum SplitType { SPLIT_NONE=0, SPLIT_H=1, SPLIT_V=2 };

@class BZView;

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
}

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

@end
