#include <sys/types.h>
#include <linux/types.h>
#include <paths.h>
#include <limits.h>
#include <time.h>

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>

#include <errno.h>

#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <linux/auto_fs4.h>

#include "include/log.h"
#include "include/sys.h"

extern int verbose;
int fdin = -1; /* data coming out of the kernel */
int fdout = -1;/* data going into the kernel */
int fd_mount_cwd = -1; /* current wd at mount time */
int timer_tick = 0;
int is_mounted = 0;
const char *mountpoint;
const char *source;
const char *mnt_opts;
const char *helper_cmd;

int umount_mountpoint(void) {
	if (fchdir(fd_mount_cwd) < 0) {
		log_error("failed to chdir to mount working dir");
		return 1;
	}
	int error = system_printf("/bin/umount %s 2>/dev/null >/dev/null </dev/null", mountpoint);
	if (error) {
		log_printf("'/bin/umount %s' failed (exit code: %d)\n", mountpoint, error);
		return 1;
	}
	if (helper_cmd) {
		error = system_printf("%s cleanup %s %s 2>/dev/null >/dev/null </dev/null", 
						helper_cmd, source, mountpoint);
	}
	if (error) {
		log_printf("'%s cleanup %s %s' failed (exit code: %d)\n", 
					helper_cmd, source, mountpoint, error);
		return 0;
	}
	if (chdir("/") < 0) {
		log_error("failed to chdir to '/'");
		return 0;
	}
	return 0;
}

int mount_mountpoint(void) {
	if (fchdir(fd_mount_cwd) < 0) {
		log_error("failed to chdir to mount working dir");
		return 1;
	}
	int error = 0;
	if (helper_cmd) {
		error = system_printf("%s prepare %s %s 2>/dev/null >/dev/null </dev/null", 
						helper_cmd, source, mountpoint);
		if (error) {
			log_printf("'%s prepare %s %s' failed (exit code: %d)\n", 
						helper_cmd, source, mountpoint, error);
			return 1;
		}
	}
	error = system_printf("/bin/mount %s %s %s 2>/dev/null >/dev/null </dev/null", 
					mnt_opts, source, mountpoint);
	if (error) {
		log_printf("'/bin/mount %s %s %s' failed (exit code: %d)\n", 
					mnt_opts, source, mountpoint, error);
		return 1;
	}
	if (timer_tick) {
		if (verbose) {
			log_printf("Starting expire process with timer tick %d seconds...\n", timer_tick);
		}
		int pid = fork(); 
		if (!pid) {
			struct sigaction s;
			s.sa_handler = SIG_IGN;
			s.sa_flags = 0;
			sigaction(SIGINT, &s, NULL);
			sigaction(SIGHUP, &s, NULL);
			sigaction(SIGTERM, &s, NULL);
			while (1) {
				if (verbose) {
					log_printf("next expire trigger poll in %d seconds\n", timer_tick);
				}
				sleep(timer_tick);
				unsigned int ioctl_arg = 0;
				int result = ioctl(fdin, AUTOFS_IOC_EXPIRE_MULTI, &ioctl_arg);
				if (result == 0) {
					if (verbose) { log_printf("sucessful expire, terminating...\n"); }
					exit(0);
				} else if (result != -1 || errno != EAGAIN) {
					log_error("Failed to send the expire trigger to kernel");
				}
			}
			log_printf("Dropped out of the expire trigger loop. This should not happen.\n");
			exit(1);
		} else if (pid<0) {
			log_error("Failed to fork");
		}
	}
	if (helper_cmd) {
		error = system_printf("%s mounted %s %s 2>/dev/null >/dev/null </dev/null", 
						helper_cmd, source, mountpoint);
		if (error) {
			log_printf("'%s mounted %s %s' failed (exit code: %d)\n", 
						helper_cmd, source, mountpoint, error);
			return 0;
		}
	}
	if (chdir("/") < 0) {
		log_error("failed to chdir to '/'");
		return 0;
	}
	return 0;
}

void umount_autofs(void)
{
	int error = system_printf("/bin/umount %s 2>/dev/null >/dev/null </dev/null", mountpoint);
	if (error) {
		log_printf("'/bin/umount %s' failed (exit code: %d)\n", mountpoint, error);
	}
}

static int mount_autofs(void)
{
	int pipefd[2];
	if (verbose) {
		log_printf("trying to mount %s as the autofs root\n", mountpoint);
	}
	if(pipe(pipefd) < 0)
	{
		log_error("failed to get kernel pipe");
		return -1;
	}
	int status = system_printf("/bin/mount -v -t autofs -o fd=%d,pgrp=%u,minproto=5,maxproto=5,direct \"mount.ondemand(pid%u)\" \"%s\" 2>/dev/null >/dev/null </dev/null",
		pipefd[1], (unsigned) getpgrp(), getpid(), mountpoint);
	if (status != 0)
	{
		log_printf("unable to mount autofs on %s. mount returned %d\n", mountpoint, status);
		close(pipefd[0]);
		close(pipefd[1]);
		return -1;
	}

	close(pipefd[1]);
	fdout = pipefd[0];

	fdin = open(mountpoint, O_RDONLY);
	if(fdin < 0)
	{
		log_error("failed to open mountpoint");
		umount_autofs();
		return -1;
	}
	return 0;
}

static void send_ready(unsigned int wait_queue_token)
{
	if(ioctl(fdin, AUTOFS_IOC_READY, wait_queue_token) < 0)
		log_error("failed to report ready to kernel");
}

static void send_fail(unsigned int wait_queue_token)
{
	if(ioctl(fdin, AUTOFS_IOC_FAIL, wait_queue_token) < 0)
		log_error("failed to report failure to kernel");
}

static int autofs_process_request(const struct autofs_v5_packet *pkt)
{
	int status;
	if(pkt->hdr.type == autofs_ptype_missing_direct) {
		if (verbose) { log_printf("kernel is requesting a mount -> %s\n", pkt->name); }
		status = mount_mountpoint();
		if (status == 0) { is_mounted = 1; }
	} else if(pkt->hdr.type == autofs_ptype_expire_direct) {
		if (verbose) { log_printf("kernel is requesting a umount -> %s\n", pkt->name); }
		status = umount_mountpoint();
		if (status == 0) { is_mounted = 0; }
	} else {
		log_printf("unsupported packet type %d\n", pkt->hdr.type);
		return 1;
	}

	if (verbose) {
		log_printf("Reporting %s to kernel for wait queue token %u\n", 
				(status==0?"success":"failure"), pkt->wait_queue_token);
	}
	if (status == 0) {
		send_ready(pkt->wait_queue_token);
	} else {
		send_fail(pkt->wait_queue_token);
		log_printf("failed to %s %s\n", (pkt->hdr.type == autofs_ptype_missing_direct?"mount":"umount"), mountpoint);
	}

	return 0;
}

static int fullread(void *ptr, size_t len)
{
	char *buf = (char *) ptr;
	while(len > 0)
	{
		ssize_t r = read(fdout, buf, len);
		if(r == -1)
		{
			if (errno == EINTR)
				continue;
			break;
		}
		buf += r;
		len -= r;
	}
	return len;
}

static int autofs_in(union autofs_v5_packet_union *pkt)
{
	struct pollfd fds[1];

	fds[0].fd = fdout;
	fds[0].events = POLLIN;

	while(1)
	{
		if(poll(fds, 1, 1000) == -1)
		{
			if (errno == EINTR)
				continue;
			log_error("failed while trying to read packet from kernel");
			return -1;
		}
		if(fds[0].revents & POLLIN)
			return fullread(pkt, sizeof(*pkt));
	}
}

void cleanup_signal_handler(int sig)
{
	if (verbose) {
		log_printf("got signal %d, shutting down.\n", sig);
	}
	if (is_mounted) {
		is_mounted = umount_mountpoint();
	}
	if (!is_mounted) {
		close(fdin);
		close(fdout);
		umount_autofs();
		exit(0);
	} else {
		log_printf("failed to umount %s, shutdown aborted\n", mountpoint);
	}
}



int autofs_init(int timeout)
{
	int kproto_version;
	int error = 0;

	struct sigaction s;
	s.sa_handler = cleanup_signal_handler;
	s.sa_flags = 0;
	sigaction(SIGINT, &s, NULL);
	sigaction(SIGHUP, &s, NULL);
	sigaction(SIGTERM, &s, NULL);


	fd_mount_cwd = open(".", O_RDONLY);
	if(fd_mount_cwd < 0) { 
		log_error("failed to open cwd");
		return -1; 
	}

	error = chdir("/");
	if (error) { 
		log_error("failed to chdir to /:");
		return error; 
	}

	error = mount_autofs();
	if (error) { return error; }

	error = ioctl(fdin, AUTOFS_IOC_PROTOVER, &kproto_version);
	if (error) { 
		log_error("failed to read autofs kernel protocol version");
		return error; 
	}
	if(kproto_version != 5)
	{
		log_printf("only kernel protocol version 5 is tested. You have %d.\n",
			kproto_version);
		return 1;
	}
	if (timeout) {
		timer_tick = timeout>1?timeout/2:1;
	}

	error = ioctl(fdin, AUTOFS_IOC_SETTIMEOUT, &timeout);
	if (error) { 
		log_printf("failed to set autofs timeout to %d: %s\n",
			timeout, strerror(errno));
		return error; 
	}

	return 0;
}

int autofs_run(const char *mountpoint_in, const char *source_in, const char *mnt_opts_in, 
                int timeout, const char *helper_cmd_in, int init_pipe_fd)
{
	mountpoint = mountpoint_in;
	source = source_in;
	mnt_opts = mnt_opts_in;
	helper_cmd = helper_cmd_in;
	int error = autofs_init(timeout);
	// If we have a pipe to the parent process, signal
	// init success/failure
	if (init_pipe_fd >= 0) {
		write(init_pipe_fd, &error, sizeof(error));
		close(init_pipe_fd);
	}
	if (error) { return error; } 

	while(1) {
		// Clean up old expire trigger processes
		while (waitpid(-1, NULL, WNOHANG)>0) {}
		
		union autofs_v5_packet_union pkt;
		if(autofs_in(&pkt)) {
			continue;
		}
		if (verbose) {
			log_printf("Got a autofs packet\n");
		}
		autofs_process_request(&pkt.v5_packet);
		poll(0, 0, 200);
	}
	umount_autofs();
	if (verbose) { log_printf("... quitting\n"); }
	return 0;
}
