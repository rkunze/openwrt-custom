#ifndef _LOG_H__
#define _LOG_H__
void log_start(void);
void log_stop(void);
void log_printf(char *fmt, ...);
void log_error(char *msg);
#endif
