#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#include "include/log.h"
#include "include/autofs.h"
#include "include/setup.h"

int verbose = 0;
int daemonized = 0;

void usage() {
	fprintf(stderr, "usage: mount.ondemand SOURCE TARGET [-dfvsn] [-o OPT[,OPTS...]]\n");
	fprintf(stderr, "options:\n");
	fprintf(stderr, "\t -d\tDebug, run autofs handler in foreground\n");
	fprintf(stderr, "\t -f\tFake mount, do not start autofs handler\n");
	fprintf(stderr, "\t -v\tVerbose\n");
	fprintf(stderr, "\t -s\tPassed through to the actual mount\n");
	fprintf(stderr, "\t -n\tPassed through to the actual mount\n");
	fprintf(stderr, "special -o arguments:\n");
	fprintf(stderr, "\tfstype=TYPE\tPassed to the actual mount as \"-t TYPE\"\n");
	fprintf(stderr, "\ttimeout=SEC\tExpire timeout in seconds\n");
	fprintf(stderr, "\thelper=CMD\t(optional) program that is run on mount/umount\n");
	fprintf(stderr, "other -o arguments are passed to the actual mount\n");
	fprintf(stderr, "Helper programm calls:\n");
	fprintf(stderr, "\tbefore mount\tCMD prepare SOURCE TARGET\n");
	fprintf(stderr, "\tafter mount\tCMD mounted SOURCE TARGET\n");
	fprintf(stderr, "\tafter umount\tCMD cleanup SOURCE TARGET\n");	
}

int main(int argc, char *argv[])
{
	int error;
	int foreground = 0;
	int fake = 0;
	int sloppy = 0;
	int nomtab = 0;
	char *mount_options_arg = "";
	char *source = "";
	char *mountpoint = "";
	int timeout = 600;
	char *helper_cmd = NULL;
	char *real_mount_args = "";

	error = parse_arguments(argc, argv, &foreground, &fake, &verbose, &sloppy, &nomtab, &mount_options_arg, &source, &mountpoint);
    if (error) {
		usage();
		return 1;
	}

	char buffer[parse_mount_opts_buffer_size(mount_options_arg, verbose, sloppy, nomtab)];
	error = parse_mount_opts(mount_options_arg, verbose, sloppy, nomtab, buffer, &timeout, &helper_cmd, &real_mount_args);
    if (error) {
		usage();
		return 1;
	}

	if (verbose) {
		log_printf("starting mount.ondemand on mountpoint %s with expire timeout %d\n", mountpoint, timeout);
		log_printf("mount command for actual mount: mount %s %s %s\n", real_mount_args, source, mountpoint);
		log_printf("umount command on mount expire: umount %s\n", mountpoint);
		if (helper_cmd) {
			log_printf("helper command (called before/after mount and after umount): %s\n", helper_cmd);
		}
	}

    if (geteuid() != 0) {
            fprintf(stderr, "This program must be run by root.\n");
            return 1;
    }
	

	if (fake) {
		return 0;
	}

	if (!foreground) {
		int pipefd[2];
		if (pipe(pipefd) < 0) {
			fprintf(stderr, "Failed to open pipe: %s\n", strerror(errno));
			return 1; 
		}
		pid_t child_pid = fork();
		if (child_pid == 0) {
			daemonized = 1;
			close(pipefd[0]);
			log_start();
			error = autofs_run(mountpoint, source, real_mount_args, timeout, helper_cmd, pipefd[1]);
			log_stop();
			return error?1:0;
		} else if (child_pid > 0) {
			close(pipefd[1]);
			int daemon_error;
			if (read(pipefd[0], &daemon_error, sizeof(daemon_error)) < 0) {
				fprintf(stderr, "Failed to read init status from daemon process: %s\n", strerror(errno));
				return 1;
			}
			if (daemon_error) {
				fprintf(stderr, "Daemon process init failed, see syslog for details\n");
			}
			return daemon_error;
		} else {
			fprintf(stderr, "Failed to start daemon process: %s\n", strerror(errno));
			return 1;
		}
	} else {
		log_printf("running ondemand handler in foreground\n");
		error = autofs_run(mountpoint, source, real_mount_args, timeout, helper_cmd, -1);
		return error?1:0;
	}

}


