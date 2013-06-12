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

#import "AppDelegate.h"

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Insert code here to initialize your application
}

/*
- (BOOL)applicationShouldHandleReopen:(NSApplication *)sender hasVisibleWindows:(BOOL)flag
{
    [Document setIsNewWindow:TRUE];
    [[NSDocumentController sharedDocumentController] newDocument:self];
    return NO;
}*/


//- (BOOL)validateUserInterfaceItem:(id <NSValidatedUserInterfaceItem>)anItem
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    //NSLog(@"BZView::validateMenuItem");
    SEL theAction = [menuItem action];
    BZOptions *bzopt = [BZOptions sharedInstance];
    
    /*if (theAction == @selector(OnCharmodeAscii:)) {
        [menuItem setState:(bzopt->charset==CTYPE_ASCII)? NSOnState:NSOffState];
    } else if (theAction == @selector(OnCharmodeSJIS:)) {
        [menuItem setState:(bzopt->charset==CTYPE_SJIS)? NSOnState:NSOffState];
    } else if (theAction == @selector(OnCharmodeUTF16:)) {
        [menuItem setState:(bzopt->charset==CTYPE_UNICODE)? NSOnState:NSOffState];
    } else if (theAction == @selector(OnCharmodeUTF8:)) {
        [menuItem setState:(bzopt->charset==CTYPE_UTF8)? NSOnState:NSOffState];
    } else if (theAction == @selector(OnCharmodeJIS:)) {
        [menuItem setState:(bzopt->charset==CTYPE_JIS)? NSOnState:NSOffState];
    } else if (theAction == @selector(OnCharmodeEUC:)) {
        [menuItem setState:(bzopt->charset==CTYPE_EUC)? NSOnState:NSOffState];
    } else if (theAction == @selector(OnCharmodeEBCDIC:)) {
        [menuItem setState:(bzopt->charset==CTYPE_EBCDIC)? NSOnState:NSOffState];
    } else if (theAction == @selector(OnCharmodeEPWING:)) {
        [menuItem setState:(bzopt->charset==CTYPE_EPWING)? NSOnState:NSOffState];
    } else*/ if (theAction == @selector(OnCharmodeAutoDetect:)) {
        [menuItem setState:(bzopt->bAutoDetect)? NSOnState:NSOffState];
    } else if (theAction == @selector(OnByteOrderIntel:)) {
        [menuItem setState:!(bzopt->bByteOrder)? NSOnState:NSOffState];
    } else if (theAction == @selector(OnByteOrderMotorola:)) {
        [menuItem setState:(bzopt->bByteOrder)? NSOnState:NSOffState];
    }
    return YES;
}
-(void)OnCharmodeAutoDetect:(id)sender
{
    BZOptions *bzopt = [BZOptions sharedInstance];
    bzopt->bAutoDetect = !bzopt->bAutoDetect;
}

-(void)OnByteOrderIntel:(id)sender
{
    BZOptions *bzopt = [BZOptions sharedInstance];
    bzopt->bByteOrder = FALSE;
    //GetMainFrame()->UpdateInspectViewChecks();
}
-(void)OnByteOrderMotorola:(id)sender
{
    BZOptions *bzopt = [BZOptions sharedInstance];
    bzopt->bByteOrder = TRUE;
    //GetMainFrame()->UpdateInspectViewChecks();
}

@end
