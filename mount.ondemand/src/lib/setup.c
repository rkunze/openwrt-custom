#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>

int parse_mount_opts_buffer_size(const char *mount_options, int verbose, int sloppy, int nomtab) {
  // The calculated buffer size is a little too large if 'mount_options_arg' includes 
  // the "fstype", "timeout" and/or "helper" options.
  return strlen(mount_options)+3*(verbose+sloppy+nomtab)+4;
}

int parse_mount_opts(const char *opts, int verbose, int sloppy, int nomtab, 
                     char *buffer, int *timeout, char **helper_cmd, char **real_mount_args) {
	*real_mount_args = buffer;
	if (verbose) {
		strcpy(buffer, "-v ");
		buffer += 3;
	}
	if (sloppy) {
		strcpy(buffer, "-s ");
		buffer += 3;
	}
	if (nomtab) {
		strcpy(buffer, "-n ");
		buffer += 3;
	}

	char * const mount_par_start = buffer;
	const char *helper_cmd_idx = NULL;
	const char *fstype = NULL;

	strcpy(buffer, "-o ");
	buffer += 3;
	while (*opts != '\0') {
		if (strncmp(opts, "fstype=", 7) == 0) {
			opts += 7;
			fstype = opts;
			while (*opts != ',' && *opts != '\0') { opts++; }
			if (*opts == ',') opts++;
        } else if (strncmp(opts, "helper=", 7) == 0) {
			opts += 7;
			helper_cmd_idx = opts;
			while (*opts != ',' && *opts != '\0') { opts++; }
			if (*opts == ',') opts++;
        } else if (strncmp(opts, "timeout=", 8) == 0) {
			if (!sscanf(opts, "timeout=%d", timeout) || *timeout < 0) {
				fprintf(stderr, "illegal value for expire timeout\n");
				return 1;
			}
			while (*opts != ',' && *opts != '\0') { opts++; }
			if (*opts == ',') opts++;
		} else {
			while (*opts != ',' && *opts != '\0') {
				*buffer++ = *opts++;
			}
			if (*opts == ',') {
				*buffer++ = *opts++;
			}
        }
    }
	if (mount_par_start+3 == buffer) {
		// No pass-through options, back off to eliminate the bogus '-o '
		buffer = mount_par_start;
	} else if (*(buffer-1) == ',') {
		// extra ',' at the end?
		buffer--;
	}
	if (fstype) {
		if (buffer != mount_par_start) {
			*(buffer++) = ' ';
		}
		strcpy(buffer, "-t ");
		buffer += 3;
		while (*fstype != ',' && *fstype != '\0') {
			*buffer++ = *fstype++;
		}
	}
	*buffer = '\0';
	if (buffer != mount_par_start) {
		*real_mount_args = mount_par_start;
	}
	if (helper_cmd_idx != NULL) {
		buffer++;
		*helper_cmd = buffer;
		while (*helper_cmd_idx != ',' && *helper_cmd_idx != '\0') {
			*buffer++ = *helper_cmd_idx++;
		}
		*buffer = '\0';
	}
	
	return 0;
}

int parse_arguments(int argc, char * const argv[], int *debug, int *fake, int *verbose, int *sloppy, int *nomtab, 
                    char **mount_options, char **source, char **mountpoint)	{
	int cur_opt;
	do {
		cur_opt = getopt(argc, argv, "sfnvdo:");
		switch (cur_opt) {
			case 'd': *debug = 1; break; 
			case 'f': *fake = 1; break;
			case 'v': *verbose = 1; break;
			case 's': *sloppy = 1; break;
			case 'n': *nomtab = 1; break;
			case 'o': *mount_options = optarg; break;
			case  -1: break; 
			default:
				return 1;
		}
	} while (cur_opt != -1);

	if (optind != argc-2) {
		return 1;
	}
	*source = argv[optind++];
	*mountpoint = argv[optind];
	return 0;
}


