#ifndef _AUTOFS_H__
#define _AUTOFS_H__

int autofs_run(const char *mountpoint, const char *source, const char *real_mount_args, 
                int timeout, const char *helper_cmd, 
                int init_pipe_fd);
pid_t autofs_safe_fork(void);

#endif
