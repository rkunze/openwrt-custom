#include <stdio.h>
#include <syslog.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

extern int daemonized;

void log_start(void)
{
	if (daemonized) {
		openlog("mount.ondemand", LOG_PID, LOG_DAEMON);
	}
}

void log_stop(void)
{
	if (daemonized) {
		closelog();
	}
}

void log_printf(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	if(daemonized)
		vsyslog(10, fmt, ap);
	else
		vfprintf(stderr, fmt, ap);
}

void log_error(char *msg)
{
	log_printf("%s: %d (%s)\n", msg, errno, strerror(errno));
}
