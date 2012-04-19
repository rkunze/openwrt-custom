#ifndef _SETUP_H__
#define _SETUP_H__

int parse_arguments(int argc, char *argv[], int *debug, int *fake, int *verbose, int *sloppy, int *nomtab, 
                    char **mount_options, char **source, char **mountpoint);

int parse_mount_opts_buffer_size(const char *mount_options_arg, int verbose, int sloppy, int nomtab);
int parse_mount_opts(const char *opts, int verbose, int sloppy, int nomtab, 
                     char *buffer, int *timeout, char **helper_cmd, char **real_mount_args);

int daemonize();
#endif
