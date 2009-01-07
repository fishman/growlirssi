// =======================
// = Growl irssi module  =
// = Reza Jelveh         =
// =======================
#include <Carbon/Carbon.h>

#include "growl-irssi.h"
#include "GrowlApplicationBridge-Carbon.h"
#include "GrowlDefines.h"

#define NOTIFICATION_NAME CFSTR("IRSSI Growl")
#ifndef CFSafeRelease   
#define CFSafeRelease(_cf)                  \
    do                                      \
        {                                   \
        if ((_cf) != NULL)                  \
            {                               \
            CFRelease((CFTypeRef) _cf);     \
            _cf = NULL;                     \
            }                               \
        }                                   \
    while (0)
#endif // CFSafeRelease




static void message_incoming(SERVER_REC *server, char *msg, char *nick, char *address, char *target)
{	
	CFStringRef desc, title;
	char * win_name;
	
	// Get active window item
	if((win_name = window_get_active_name(active_win)) && (strcmp(nick, win_name) == 0))
		return;
	
	title = CFStringCreateWithCString(kCFAllocatorDefault, nick, kCFStringEncodingUTF8);
	desc = CFStringCreateWithCString(kCFAllocatorDefault, msg, kCFStringEncodingUTF8);
	Growl_NotifyWithTitleDescriptionNameIconPriorityStickyClickContext(title, desc, NOTIFICATION_NAME, NULL,	0, FALSE, NULL);
	
	CFSafeRelease(title);
	CFSafeRelease(desc);
}


static void message_public(SERVER_REC *server, const char *msg,
             const char *nick, const char *address,
             const char *target)
{
	CFStringRef desc, title;
	// int nick_match_msg(CHANNEL_REC *channel, const char *msg, const char *nick)
  CHANNEL_REC *chanrec;
  chanrec = channel_find(server, target);

  g_return_if_fail(chanrec != NULL);

  if (nick_match_msg(chanrec, msg, server->nick) == FALSE)
		return;

	title = CFStringCreateWithCString(kCFAllocatorDefault, chanrec->name, kCFStringEncodingUTF8);
	desc = CFStringCreateWithCString(kCFAllocatorDefault, msg, kCFStringEncodingUTF8);
	Growl_NotifyWithTitleDescriptionNameIconPriorityStickyClickContext(title, desc, NOTIFICATION_NAME, NULL,	0, FALSE, NULL);
	
	CFSafeRelease(title);
	CFSafeRelease(desc);
}

static void command_helpgrowl(const char *arg, SERVER_REC *server, WI_ITEM_REC *item)
{
	printtext(NULL, NULL, MSGLEVEL_CRAP,
		"\n\002Growl help:\002 \n\n"
		" /growlhelp to show this info\n"
		"Growl will show notifications when you get a query or someone writes your name in public\n"
		"Settings are not yet implemented.\n");
}


CFDataRef copyIconData(CFStringRef path)
{
	CFDataRef data = NULL;


	CFURLRef URL = CFURLCreateWithFileSystemPath( kCFAllocatorDefault,
																	              path,       // file path name
																	              kCFURLPOSIXPathStyle,    // interpret as POSIX path
																	              false );                 // is it a directory?
	

	if (URL) {
		FSRef ref;
		if (CFURLGetFSRef(URL, &ref)) {
			IconRef icon = NULL;
			SInt16 label_noOneCares;
			OSStatus err = GetIconRefFromFileInfo(&ref,
												  /*inFileNameLength*/ 0U, /*inFileName*/ NULL,
												  kFSCatInfoNone, /*inCatalogInfo*/ NULL,
												  kIconServicesNoBadgeFlag | kIconServicesUpdateIfNeededFlag,
												  &icon,
												  &label_noOneCares);

			if (err != noErr) {
				printf("Could not get icon\n");
			} else {
				IconFamilyHandle fam = NULL;
				// err = IconRefToIconFamily(icon, kSelectorAllAvailableData, &fam);
				err = ReadIconFromFSRef(&ref, &fam);
				
				if (err != noErr) {
					printf("Could not get icon\n");
				} else {
					HLock((Handle)fam);
					data = CFDataCreate(kCFAllocatorDefault, (const UInt8 *)*(Handle)fam, GetHandleSize((Handle)fam));
					HUnlock((Handle)fam);
					DisposeHandle((Handle)fam);
				}
				ReleaseIconRef(icon);
			}
		}
	}
	
	CFSafeRelease(URL);
	
	return data;
	
}


void growl_init(void)
{

	struct Growl_Delegate * delegateObject;

	CFStringRef applicationName;
	CFStringRef identifierString;
	CFDataRef icon = NULL;
	CFMutableStringRef iconPath = CFStringCreateMutable(kCFAllocatorDefault, 0);
 	char *home = getenv( "HOME" );
	
	applicationName = CFSTR("growl-irssi");
	identifierString = NULL;

	CFStringAppendCString(iconPath, home, kCFStringEncodingUTF8);
	CFStringAppend(iconPath, CFSTR("/.irssi/irssi.icns"));
	
	icon = (CFDataRef)copyIconData(iconPath);
	
	CFSafeRelease(iconPath);
	// Can't register console app growl without icon data so don't even try
	g_return_if_fail(icon != NULL);
	
	CFStringRef name = NOTIFICATION_NAME;
	CFArrayRef defaultAndAllNotifications = CFArrayCreate(kCFAllocatorDefault, (const void **)&name, 1, &kCFTypeArrayCallBacks);
	CFTypeRef registerKeys[4] = {
		GROWL_APP_NAME,
		GROWL_NOTIFICATIONS_ALL,
		GROWL_NOTIFICATIONS_DEFAULT,
		GROWL_APP_ICON,
		// GROWL_APP_ID
	};
	CFTypeRef registerValues[4] = {
		applicationName,
		defaultAndAllNotifications,
		defaultAndAllNotifications,
		icon
	};
	
	CFDictionaryRef registerInfo = CFDictionaryCreate(kCFAllocatorDefault, registerKeys, registerValues, 4, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	
	delegateObject = (struct Growl_Delegate *)calloc(1, sizeof(struct Growl_Delegate));
	delegateObject->applicationName = CFSTR("growl-irssi");
	delegateObject->registrationDictionary = registerInfo;
	delegateObject->applicationIconData = icon;
	
	
	Growl_SetDelegate(delegateObject);


	CFRelease(defaultAndAllNotifications);
	CFRelease(icon);
	CFRelease(applicationName);

	// Setup irssi
	signal_add_last("message private", (SIGNAL_FUNC) message_incoming);
	signal_add_last("message public", (SIGNAL_FUNC) message_public);
	command_bind("growlhelp", NULL, (SIGNAL_FUNC) command_helpgrowl);
	command_bind("helpgrowl", NULL, (SIGNAL_FUNC) command_helpgrowl);
	// Growl_NotifyWithTitleDescriptionNameIconPriorityStickyClickContext(CFSTR("growl-irssi"), CFSTR("Plugin successfully loaded."), NOTIFICATION_NAME, NULL,	0, FALSE, NULL);

	printtext(NULL, NULL, MSGLEVEL_CLIENTNOTICE,
		"Growl Support successfully loaded\n"
		"Try /helpgrowl or /growlhelp for a short command overview");

	module_register("growl", "core");	
}

void growl_deinit(void)
{
	signal_remove("message private", (SIGNAL_FUNC) message_incoming);
	signal_remove("message public", (SIGNAL_FUNC) message_public);

	command_unbind("growlhelp", (SIGNAL_FUNC) command_helpgrowl);
	command_unbind("helpgrowl", (SIGNAL_FUNC) command_helpgrowl);
}



#if 0
CGImageRef MyCreateJPEGImageRef(){
    CGImageRef image;
    CGDataProviderRef provider;
    CFStringRef name;
    CFURLRef url;
    // Get the URL to the bundle resource.
   url = CFURLCreateWithFileSystemPath( kCFAllocatorDefault,
               CFSTR("/Users/reza/.irssi/irss.jpg"),       // file path name
               kCFURLPOSIXPathStyle,    // interpret as POSIX path
               false );                 // is it a directory?
 
    // Create the data provider object
    provider = CGDataProviderCreateWithURL (url);
    CFRelease (url);
 
			printtext(NULL, NULL, MSGLEVEL_CLIENTNOTICE,"debug1\n");
    // Create the image object from that provider.
    image = CGImageCreateWithJPEGDataProvider (provider, NULL, true,
                                    kCGRenderingIntentDefault);

    if (image == NULL)
			printtext(NULL, NULL, MSGLEVEL_CLIENTNOTICE,"debug2\n");
			
    CGDataProviderRelease (provider);
 
    return (image);
}
#endif



