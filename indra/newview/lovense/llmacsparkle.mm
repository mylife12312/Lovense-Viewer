/** 
 * @file llmacsparkle.mm
 * @brief sparkle base class implementation
 *
 * $LicenseInfo:firstyear=2023&license=viewerlgpl$
 * Lovense Viewer Source Code
 * Copyright (C) 2023, HYTTO PTE. LTD.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * $/LicenseInfo$
 */

#include "llmacsparkle.h"
#import <Cocoa/Cocoa.h>
#import <Sparkle/Sparkle.h>


SPUUpdater *updater = nil;

void resetUpdateConfig(){
    NSUserDefaults *user = [NSUserDefaults standardUserDefaults];
    [user setValue:[NSNumber numberWithBool:NO] forKey:@"SUAutomaticallyUpdate"];
    [user setValue:@"" forKey:@"SUSkippedVersion"];
    [user synchronize];
}

static SPUUpdater *s_updater = nil;

void lovense::mac::onInitSparkle(const char *url, bool forceupdate){
    //if(forceupdate)
       // resetUpdateConfig();
    if(!s_updater){
        s_updater = [[SPUUpdater alloc] initWithHostBundle:[NSBundle mainBundle] applicationBundle:[NSBundle mainBundle] userDriver:[[SPUStandardUserDriver alloc] initWithHostBundle:[NSBundle mainBundle] delegate:nil] delegate:nil];
    }
    [s_updater setFeedURL:[NSURL URLWithString:[NSString stringWithUTF8String:url]]];
    NSError *error = nil;
    if([s_updater startUpdater:&error]){
        if(forceupdate){
            [s_updater checkForUpdates];
        }else
        {
            [s_updater checkForUpdatesInBackground];
        }
    }
     else{
        NSLog(@"errror: %@",error);
    }
}

void lovense::mac::onCleanSparkle(){

}
