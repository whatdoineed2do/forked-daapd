#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "logger.h"
#include "misc.h"
#include "smartpl_query.h"



/* fake refs */
typedef void  sqlite3;
enum listener_event_type { A };


void cache_daap_suspend() { }
void cache_daap_resume() { }
void library_update_trigger(short s) { }
int db_init_indices(sqlite3 *hdl) { return -1; }
int db_init_tables(sqlite3 *hdl) { return -1; }
int db_upgrade(sqlite3 *hdl, int db_ver) { return -1; }
void listener_notify(enum listener_event_type type) { }

/* for Makefile.am
 *
test_antlr_SOURCES = \
        test-antlr.c \
        db.c db.h \
        logger.c logger.h \
        misc.c misc.h \
        conffile.c conffile.h \
        smartpl_query.c \
        rng.c rng.h \
        SMARTPLLexer.c SMARTPLLexer.h SMARTPLParser.c SMARTPLParser.h \
        SMARTPL2SQL.c SMARTPL2SQL.h

test_antlr_LDADD = \
        $(ANTLR3C_LIBS) $(CONFUSE_LIBS) $(LIBGCRYPT_LIBS)  $(SQLITE3_LIBS) $(COMMON_LIBS) -lz -lpthread
*/


void*  _killer(void* args_)
{
    unsigned  zzz = args_ == NULL ? 10 : (*((unsigned*)args_));
    DPRINTF(E_LOG, L_MAIN, "killer thread starting, zzz=%d\n", zzz);
    sleep(zzz);
    DPRINTF(E_FATAL, L_MAIN, "process still running (this is bad) - forcing stop\n");
    //abort();
    exit(-1);
    return NULL;
}

int main(int argc, char* argv[])
{
    int  ret = 0;
    struct smartpl  smartpl_expression;

    const char* queries[] = {
        // "query\" { genre is \"Pop\" }", -- dies on this too
        "\"query\" { genre is \"Pop\" }",
        "\"query\" { genre is \"Pop }",
        "\"query\" { genre is Pop\" }",
        "\"query\" { genre is Pop }",
        NULL
    };
    unsigned  zzz = 2;

    pthread_t  tid;
    pthread_create(&tid, NULL, _killer, &zzz);

    const char**  q = queries;
    while (*q) 
    {
        const char*  expression = *q++;

        memset(&smartpl_expression, 0, sizeof(smartpl_expression));
        DPRINTF(E_LOG, L_MAIN, "smart_query_parse_string - '%s'\n", expression);
        DPRINTF(E_LOG, L_MAIN, "  ret=%d\n", (ret = smartpl_query_parse_string(&smartpl_expression, expression)));
    }

    return 0;
}
