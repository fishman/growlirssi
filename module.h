#include "common.h"
#include "servers.h"
#include "settings.h"
#include "printtext.h"
#include "levels.h"
#include "signals.h"
#include "commands.h"
#include "queries.h"
#include "channels.h"
#include "window-items.h"


#define MODULE_NAME "growl"

typedef struct {
    void (*orig_send_message)(SERVER_REC *server, const char *target,
                  const char *msg, int target_type);
} MODULE_SERVER_REC;


void irssi_redraw(void);


/* Send command to IRC server */
void irc_send_cmdv(SERVER_REC *server, const char *cmd, ...) G_GNUC_PRINTF (2, 3);

QUERY_REC *irc_query_create(const char *server_tag, const char *nick, int automatic);
