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
