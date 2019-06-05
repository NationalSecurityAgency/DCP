/**
 * @file
 *
 * @version 1.0
 * @date Jul 3, 2014
 *
 * @section DESCRIPTION
 *
 * todo write description for logging.h
 */
#ifndef LOGGING_H__
#define LOGGING_H__


#include <err.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>


/* MACROS *********************************************************************/

#define _log(_func, _msg, ...)                                                 \
    do {                                                                       \
        if (logging_debug_mode)                                                \
            _func("%s:%s():%d "_msg, basename(__FILE__), __func__, __LINE__,   \
                    ##__VA_ARGS__);                                            \
        else                                                                   \
            _func(_msg, ##__VA_ARGS__);                                        \
    } while (0)


#define log_debug(_fmt, ...)    _log(warn, _fmt, ##__VA_ARGS__)
#define log_debugx(_fmt, ...)   _log(warnx, _fmt, ##__VA_ARGS__)

#define log_warn    log_debug
#define log_warnx   log_debugx
#define log_error   log_debug
#define log_errorx  log_debugx


#define log_crit(_exit, _fmt, ...) \
    do { log_debug(_fmt, ##__VA_ARGS__); exit(_exit); } while (0)


#define log_critx(_exit, _fmt, ...) \
    do { log_debugx(_fmt, ##__VA_ARGS__); exit(_exit); } while (0)


/* Public API *****************************************************************/


extern int logging_debug_mode;


#endif

