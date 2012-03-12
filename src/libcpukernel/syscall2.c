/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <sched.h>
#include <syscall.h>
#include <time.h>
#include <utime.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/times.h>

#include <cpukernel.h>
#include <mhandle.h>


static char *err_sys_note =
	"\tThe system calls performed by the executed application are intercepted by\n"
	"\tMulti2Sim and emulated in file 'syscall.c'. The most common system calls are\n"
	"\tcurrently supported, but your application might perform specific unsupported\n"
	"\tsystem calls or combinations of parameters. To request support for a given\n"
	"\tsystem call, please email 'development@multi2sim.org'.\n";



/*
 * System call error codes
 */

#define SIM_EPERM		1
#define SIM_ENOENT		2
#define SIM_ESRCH		3
#define SIM_EINTR		4
#define SIM_EIO			5
#define SIM_ENXIO		6
#define SIM_E2BIG		7
#define SIM_ENOEXEC		8
#define SIM_EBADF		9
#define SIM_ECHILD		10
#define SIM_EAGAIN		11
#define SIM_ENOMEM		12
#define SIM_EACCES		13
#define SIM_EFAULT		14
#define SIM_ENOTBLK		15
#define SIM_EBUSY		16
#define SIM_EEXIST		17
#define SIM_EXDEV		18
#define SIM_ENODEV		19
#define SIM_ENOTDIR		20
#define SIM_EISDIR		21
#define SIM_EINVAL		22
#define SIM_ENFILE		23
#define SIM_EMFILE		24
#define SIM_ENOTTY		25
#define SIM_ETXTBSY		26
#define SIM_EFBIG		27
#define SIM_ENOSPC		28
#define SIM_ESPIPE		29
#define SIM_EROFS		30
#define SIM_EMLINK		31
#define SIM_EPIPE		32
#define SIM_EDOM		33
#define SIM_ERANGE		34

struct string_map_t sys_error_code_map =
{
	34,
	{
		{ "EPERM", 1 },
		{ "ENOENT", 2 },
		{ "ESRCH", 3 },
		{ "EINTR", 4 },
		{ "EIO", 5 },
		{ "ENXIO", 6 },
		{ "E2BIG", 7 },
		{ "ENOEXEC", 8 },
		{ "EBADF", 9 },
		{ "ECHILD", 10 },
		{ "EAGAIN", 11 },
		{ "ENOMEM", 12 },
		{ "EACCES", 13 },
		{ "EFAULT", 14 },
		{ "ENOTBLK", 15 },
		{ "EBUSY", 16 },
		{ "EEXIST", 17 },
		{ "EXDEV", 18 },
		{ "ENODEV", 19 },
		{ "ENOTDIR", 20 },
		{ "EISDIR", 21 },
		{ "EINVAL", 22 },
		{ "ENFILE", 23 },
		{ "EMFILE", 24 },
		{ "ENOTTY", 25 },
		{ "ETXTBSY", 26 },
		{ "EFBIG", 27 },
		{ "ENOSPC", 28 },
		{ "ESPIPE", 29 },
		{ "EROFS", 30 },
		{ "EMLINK", 31 },
		{ "EPIPE", 32 },
		{ "EDOM", 33 },
		{ "ERANGE", 34 }
	}
};


void sys_init(void)
{
	/* Host constants for 'errno' */
	M2S_HOST_GUEST_MATCH(EPERM, SIM_EPERM);
	M2S_HOST_GUEST_MATCH(ENOENT, SIM_ENOENT);
	M2S_HOST_GUEST_MATCH(ESRCH, SIM_ESRCH);
	M2S_HOST_GUEST_MATCH(EINTR, SIM_EINTR);
	M2S_HOST_GUEST_MATCH(EIO, SIM_EIO);
	M2S_HOST_GUEST_MATCH(ENXIO, SIM_ENXIO);
	M2S_HOST_GUEST_MATCH(E2BIG, SIM_E2BIG);
	M2S_HOST_GUEST_MATCH(ENOEXEC, SIM_ENOEXEC);
	M2S_HOST_GUEST_MATCH(EBADF, SIM_EBADF);
	M2S_HOST_GUEST_MATCH(ECHILD, SIM_ECHILD);
	M2S_HOST_GUEST_MATCH(EAGAIN, SIM_EAGAIN);
	M2S_HOST_GUEST_MATCH(ENOMEM, SIM_ENOMEM);
	M2S_HOST_GUEST_MATCH(EACCES, SIM_EACCES);
	M2S_HOST_GUEST_MATCH(EFAULT, SIM_EFAULT);
	M2S_HOST_GUEST_MATCH(ENOTBLK, SIM_ENOTBLK);
	M2S_HOST_GUEST_MATCH(EBUSY, SIM_EBUSY);
	M2S_HOST_GUEST_MATCH(EEXIST, SIM_EEXIST);
	M2S_HOST_GUEST_MATCH(EXDEV, SIM_EXDEV);
	M2S_HOST_GUEST_MATCH(ENODEV, SIM_ENODEV);
	M2S_HOST_GUEST_MATCH(ENOTDIR, SIM_ENOTDIR);
	M2S_HOST_GUEST_MATCH(EISDIR, SIM_EISDIR);
	M2S_HOST_GUEST_MATCH(EINVAL, SIM_EINVAL);
	M2S_HOST_GUEST_MATCH(ENFILE, SIM_ENFILE);
	M2S_HOST_GUEST_MATCH(EMFILE, SIM_EMFILE);
	M2S_HOST_GUEST_MATCH(ENOTTY, SIM_ENOTTY);
	M2S_HOST_GUEST_MATCH(ETXTBSY, SIM_ETXTBSY);
	M2S_HOST_GUEST_MATCH(EFBIG, SIM_EFBIG);
	M2S_HOST_GUEST_MATCH(ENOSPC, SIM_ENOSPC);
	M2S_HOST_GUEST_MATCH(ESPIPE, SIM_ESPIPE);
	M2S_HOST_GUEST_MATCH(EROFS, SIM_EROFS);
	M2S_HOST_GUEST_MATCH(EMLINK, SIM_EMLINK);
	M2S_HOST_GUEST_MATCH(EPIPE, SIM_EPIPE);
	M2S_HOST_GUEST_MATCH(EDOM, SIM_EDOM);
	M2S_HOST_GUEST_MATCH(ERANGE, SIM_ERANGE);
}


void sys_done(void)
{
}




/*
 * System call 'exit' (code 1)
 */

int sys_exit_impl(void)
{
	int status;

	/* Arguments */
	status = isa_regs->ebx;
	sys_debug("  status=0x%x\n", status);

	/* Finish context */
	ctx_finish(isa_ctx, status);
	return 0;
}




/*
 * System call 'close' (code 2)
 */

int sys_close_impl(void)
{
	int guest_fd;
	int host_fd;
	struct file_desc_t *fd;

	/* Arguments */
	guest_fd = isa_regs->ebx;
	sys_debug("  guest_fd=%d\n", guest_fd);
	host_fd = file_desc_table_get_host_fd(isa_ctx->file_desc_table, guest_fd);
	sys_debug("  host_fd=%d\n", host_fd);

	/* Get file descriptor table entry. */
	fd = file_desc_table_entry_get(isa_ctx->file_desc_table, guest_fd);
	if (!fd)
		return -EBADF;

	/* Close host file descriptor only if it is valid and not stdin/stdout/stderr. */
	if (host_fd > 2)
		close(host_fd);

	/* Free guest file descriptor. This will delete the host file if it's a virtual file. */
	if (fd->kind == file_desc_virtual)
		sys_debug("    host file '%s': temporary file deleted\n", fd->path);
	file_desc_table_entry_free(isa_ctx->file_desc_table, fd->guest_fd);

	/* Success */
	return 0;
}



/*
 * System call 'read' (code 3)
 */

int sys_read_impl(void)
{
	unsigned int buf_ptr;
	unsigned int count;

	int guest_fd;
	int host_fd;
	int err;

	void *buf;

	struct file_desc_t *fd;
	struct pollfd fds;

	/* Arguments */
	guest_fd = isa_regs->ebx;
	buf_ptr = isa_regs->ecx;
	count = isa_regs->edx;
	sys_debug("  guest_fd=%d, buf_ptr=0x%x, count=0x%x\n",
		guest_fd, buf_ptr, count);

	/* Get file descriptor */
	fd = file_desc_table_entry_get(isa_ctx->file_desc_table, guest_fd);
	if (!fd)
		return -EBADF;
	host_fd = fd->host_fd;
	sys_debug("  host_fd=%d\n", host_fd);

	/* Allocate buffer */
	buf = calloc(1, count);
	if (!buf)
		fatal("%s: out of memory", __FUNCTION__);

	/* Poll the file descriptor to check if read is blocking */
	fds.fd = host_fd;
	fds.events = POLLIN;
	err = poll(&fds, 1, 0);
	if (err < 0)
		fatal("%s: error executing 'poll'", __FUNCTION__);

	/* Non-blocking read */
	if (fds.revents || (fd->flags & O_NONBLOCK))
	{
		/* Host system call */
		err = read(host_fd, buf, count);
		if (err == -1)
		{
			free(buf);
			return -errno;
		}

		/* Write in guest memory */
		if (err > 0)
		{
			mem_write(isa_mem, buf_ptr, err, buf);
			sys_debug_string("  buf", buf, err, 1);
		}

		/* Return number of read bytes */
		free(buf);
		return err;
	}

	/* Blocking read - suspend thread */
	sys_debug("  blocking read - process suspended\n");
	isa_ctx->wakeup_fd = guest_fd;
	isa_ctx->wakeup_events = 1;  /* POLLIN */
	ctx_set_status(isa_ctx, ctx_suspended | ctx_read);
	ke_process_events_schedule();

	/* Free allocated buffer. Return value doesn't matter,
	 * it will be overwritten when context wakes up from blocking call. */
	free(buf);
	return 0;
}




/*
 * System call 'write' (code 4)
 */

int sys_write_impl(void)
{
	unsigned int buf_ptr;
	unsigned int count;

	int guest_fd;
	int host_fd;
	int err;

	struct file_desc_t *desc;
	void *buf;

	struct pollfd fds;

	/* Arguments */
	guest_fd = isa_regs->ebx;
	buf_ptr = isa_regs->ecx;
	count = isa_regs->edx;
	sys_debug("  guest_fd=%d, buf_ptr=0x%x, count=0x%x\n",
		guest_fd, buf_ptr, count);

	/* Get file descriptor */
	desc = file_desc_table_entry_get(isa_ctx->file_desc_table, guest_fd);
	if (!desc)
		return -EBADF;
	host_fd = desc->host_fd;
	sys_debug("  host_fd=%d\n", host_fd);

	/* Allocate buffer */
	buf = calloc(1, count);
	if (!buf)
		fatal("%s: out of memory", __FUNCTION__);

	/* Read buffer from memory */
	mem_read(isa_mem, buf_ptr, count, buf);
	sys_debug_string("  buf", buf, count, 0);

	/* Poll the file descriptor to check if write is blocking */
	fds.fd = host_fd;
	fds.events = POLLOUT;
	poll(&fds, 1, 0);

	/* Non-blocking write */
	if (fds.revents)
	{
		/* Host write */
		err = write(host_fd, buf, count);
		if (err == -1)
			err = -errno;

		/* Return written bytes */
		free(buf);
		return err;
	}

	/* Blocking write - suspend thread */
	sys_debug("  blocking write - process suspended\n");
	isa_ctx->wakeup_fd = guest_fd;
	ctx_set_status(isa_ctx, ctx_suspended | ctx_write);
	ke_process_events_schedule();

	/* Return value doesn't matter here. It will be overwritten when the
	 * context wakes up after blocking call. */
	free(buf);
	return 0;
}




/*
 * System call 'open' (code 5)
 */

static struct string_map_t sys_open_flags_map =
{
	16, {
		{ "O_RDONLY",        00000000 },
		{ "O_WRONLY",        00000001 },
		{ "O_RDWR",          00000002 },
		{ "O_CREAT",         00000100 },
		{ "O_EXCL",          00000200 },
		{ "O_NOCTTY",        00000400 },
		{ "O_TRUNC",         00001000 },
		{ "O_APPEND",        00002000 },
		{ "O_NONBLOCK",      00004000 },
		{ "O_SYNC",          00010000 },
		{ "FASYNC",          00020000 },
		{ "O_DIRECT",        00040000 },
		{ "O_LARGEFILE",     00100000 },
		{ "O_DIRECTORY",     00200000 },
		{ "O_NOFOLLOW",      00400000 },
		{ "O_NOATIME",       01000000 }
	}
};

int sys_open_impl(void)
{
	unsigned int file_name_ptr;

	int flags;
	int mode;
	int length;

	char file_name[MAX_PATH_SIZE];
	char full_path[MAX_PATH_SIZE];
	char temp_path[MAX_PATH_SIZE];
	char flags_str[MAX_STRING_SIZE];

	int host_fd;
	struct file_desc_t *desc;

	/* Arguments */
	file_name_ptr = isa_regs->ebx;
	flags = isa_regs->ecx;
	mode = isa_regs->edx;
	length = mem_read_string(isa_mem, file_name_ptr, sizeof file_name, file_name);
	if (length >= MAX_PATH_SIZE)
		fatal("syscall open: maximum path length exceeded");
	ld_get_full_path(isa_ctx, file_name, full_path, sizeof full_path);
	sys_debug("  filename='%s' flags=0x%x, mode=0x%x\n",
		file_name, flags, mode);
	sys_debug("  fullpath='%s'\n", full_path);
	map_flags(&sys_open_flags_map, flags, flags_str, sizeof flags_str);
	sys_debug("  flags=%s\n", flags_str);

	/* Intercept attempt to access OpenCL library and redirect to 'm2s-opencl.so' */
	gk_libopencl_redirect(full_path, sizeof full_path);

	/* Virtual files */
	if (!strncmp(full_path, "/proc/", 6))
	{
		/* File /proc/self/maps */
		if (!strcmp(full_path, "/proc/self/maps"))
		{
			/* Create temporary file and open it. */
			ctx_gen_proc_self_maps(isa_ctx, temp_path);
			host_fd = open(temp_path, flags, mode);
			assert(host_fd > 0);

			/* Add file descriptor table entry. */
			desc = file_desc_table_entry_new(isa_ctx->file_desc_table, file_desc_virtual, host_fd, temp_path, flags);
			sys_debug("    host file '%s' opened: guest_fd=%d, host_fd=%d\n",
				temp_path, desc->guest_fd, desc->host_fd);
			return desc->guest_fd;
		}

		/* Unhandled virtual file. Let the application read the contents of the host
		 * version of the file as if it was a regular file. */
		sys_debug("    warning: unhandled virtual file\n");
	}

	/* Regular file. */
	host_fd = open(full_path, flags, mode);
	if (host_fd == -1)
		return -errno;

	/* File opened, create a new file descriptor. */
	desc = file_desc_table_entry_new(isa_ctx->file_desc_table,
		file_desc_regular, host_fd, full_path, flags);
	sys_debug("    file descriptor opened: guest_fd=%d, host_fd=%d\n",
		desc->guest_fd, desc->host_fd);

	/* Return guest descriptor index */
	return desc->guest_fd;
}




/*
 * System call 'waitpid' (code 7)
 */

static struct string_map_t sys_waitpid_options_map =
{
	8, {
		{ "WNOHANG",       0x00000001 },
		{ "WUNTRACED",     0x00000002 },
		{ "WEXITED",       0x00000004 },
		{ "WCONTINUED",    0x00000008 },
		{ "WNOWAIT",       0x01000000 },
		{ "WNOTHREAD",     0x20000000 },
		{ "WALL",          0x40000000 },
		{ "WCLONE",        0x80000000 }
	}
};

int sys_waitpid_impl()
{
	int pid;
	int options;
	unsigned int status_ptr;

	char options_str[MAX_STRING_SIZE];

	struct ctx_t *child;

	/* Arguments */
	pid = isa_regs->ebx;
	status_ptr = isa_regs->ecx;
	options = isa_regs->edx;
	sys_debug("  pid=%d, pstatus=0x%x, options=0x%x\n",
		pid, status_ptr, options);
	map_flags(&sys_waitpid_options_map, options, options_str, sizeof options_str);
	sys_debug("  options=%s\n", options_str);

	/* Supported values for 'pid' */
	if (pid != -1 && pid <= 0)
		fatal("%s: only supported for pid=-1 or pid > 0.\n%s",
			__FUNCTION__, err_sys_note);

	/* Look for a zombie child. */
	child = ctx_get_zombie(isa_ctx, pid);

	/* If there is no child and the flag WNOHANG was not specified,
	 * we get suspended until the specified child finishes. */
	if (!child && !(options & 0x1))
	{
		isa_ctx->wakeup_pid = pid;
		ctx_set_status(isa_ctx, ctx_suspended | ctx_waitpid);
		return 0;
	}

	/* Context is not suspended. WNOHANG was specified, or some child
	 * was found in the zombie list. */
	if (child)
	{
		if (status_ptr)
			mem_write(isa_mem, status_ptr, 4, &child->exit_code);
		ctx_set_status(child, ctx_finished);
		return child->pid;
	}

	/* Return */
	return 0;
}




/*
 * System call 'unlink' (code 10)
 */

int sys_unlink_impl(void)
{
	unsigned int file_name_ptr;

	int length;
	int err;

	char file_name[MAX_PATH_SIZE];
	char full_path[MAX_PATH_SIZE];

	/* Arguments */
	file_name_ptr = isa_regs->ebx;
	length = mem_read_string(isa_mem, file_name_ptr, sizeof file_name, file_name);
	if (length >= MAX_PATH_SIZE)
		fatal("%s: buffer too small", __FUNCTION__);
	ld_get_full_path(isa_ctx, file_name, full_path, sizeof full_path);
	sys_debug("  file_name_ptr=0x%x\n", file_name_ptr);
	sys_debug("  file_name=%s, full_path=%s\n", file_name, full_path);

	/* Host call */
	err = unlink(full_path);
	if (err == -1)
		return -errno;

	/* Return */
	return 0;
}




/*
 * System call 'execve' (code 11)
 */

static char *err_sys_execve_note =
	"\tA system call 'execve' is trying to run a command prefixed with '/bin/sh -c'.\n"
	"\tThis is usually the result of the execution of the 'system()' function from\n"
	"\tthe guest application to run a shell command. Multi2Sim will execute this\n"
	"\tcommand natively, and then finish the calling context.\n";

int sys_execve_impl(void)
{
	unsigned int name_ptr;
	unsigned int argv;
	unsigned int envp;
	unsigned int regs;

	char name[MAX_PATH_SIZE];
	char full_path[MAX_PATH_SIZE];
	int length;

	struct list_t *arg_list;
	char arg_str[MAX_STRING_SIZE];
	char *arg;

	char env[MAX_LONG_STRING_SIZE];
	int i;

	/* Arguments */
	name_ptr = isa_regs->ebx;
	argv = isa_regs->ecx;
	envp = isa_regs->edx;
	regs = isa_regs->esi;
	sys_debug("  name_ptr=0x%x, argv=0x%x, envp=0x%x, regs=0x%x\n",
		name_ptr, argv, envp, regs);

	/* Get command name */
	length = mem_read_string(isa_mem, name_ptr, sizeof name, name);
	if (length >= sizeof name)
		fatal("%s: buffer too small", __FUNCTION__);
	ld_get_full_path(isa_ctx, name, full_path, sizeof full_path);
	sys_debug("  name='%s', full_path='%s'\n", name, full_path);

	/* Arguments */
	arg_list = list_create();
	for (;;)
	{
		unsigned int arg_ptr;

		/* Argument pointer */
		mem_read(isa_mem, argv + arg_list->count * 4, 4, &arg_ptr);
		if (!arg_ptr)
			break;

		/* Argument */
		length = mem_read_string(isa_mem, arg_ptr, sizeof arg_str, arg_str);
		if (length >= sizeof arg_str)
			fatal("%s: buffer too small", __FUNCTION__);

		/* Duplicate */
		arg = strdup(arg_str);
		if (!arg)
			fatal("%s: out of memory", __FUNCTION__);

		/* Add to argument list */
		list_add(arg_list, arg);
		sys_debug("    argv[%d]='%s'\n", arg_list->count, arg);
	}

	/* Environment variables */
	sys_debug("\n");
	for (i = 0; ; i++)
	{
		unsigned int env_ptr;

		/* Variable pointer */
		mem_read(isa_mem, envp + i * 4, 4, &env_ptr);
		if (!env_ptr)
			break;

		/* Variable */
		length = mem_read_string(isa_mem, env_ptr, sizeof env, env);
		if (length >= sizeof env)
			fatal("%s: buffer too small", __FUNCTION__);

		/* Debug */
		sys_debug("    envp[%d]='%s'\n", i, env);
	}

	/* In the special case that the command line is 'sh -c <...>', this system
	 * call is the result of a program running the 'system' libc function. The
	 * host and guest architecture might be different and incompatible, so the
	 * safest option here is running the system command natively.
	 */
	if (!strcmp(full_path, "/bin/sh") && list_count(arg_list) == 3 &&
		!strcmp(list_get(arg_list, 0), "sh") &&
		!strcmp(list_get(arg_list, 1), "-c"))
	{
		int exit_code;

		/* Execute program natively and finish context */
		warning("%s: child context executed natively.\n%s",
			__FUNCTION__, err_sys_execve_note);
		exit_code = system(list_get(arg_list, 2));
		ctx_finish(isa_ctx, exit_code);

		/* Free arguments and exit */
		for (i = 0; i < list_count(arg_list); i++)
			free(list_get(arg_list, i));
		list_free(arg_list);
		return 0;
	}

	/* Free arguments */
	for (i = 0; i < list_count(arg_list); i++)
		free(list_get(arg_list, i));
	list_free(arg_list);

	/* Return */
	fatal("%s: not implemented.\n%s", __FUNCTION__, err_sys_note);
	return 0;
}




/*
 * System call 'time' (code 13)
 */

int sys_time_impl(void)
{

	unsigned int time_ptr;
	int t;

	/* Arguments */
	time_ptr = isa_regs->ebx;
	sys_debug("  ptime=0x%x\n", time_ptr);

	/* Host call */
	t = time(NULL);
	if (time_ptr)
		mem_write(isa_mem, time_ptr, 4, &t);

	/* Return */
	return t;
}




/*
 * System call 'chmod' (code 15)
 */

int sys_chmod_impl(void)
{
	unsigned int file_name_ptr;
	unsigned int mode;

	int len;
	int err;

	char file_name[MAX_PATH_SIZE];
	char full_path[MAX_PATH_SIZE];

	/* Arguments */
	file_name_ptr = isa_regs->ebx;
	mode = isa_regs->ecx;
	len = mem_read_string(isa_mem, file_name_ptr, sizeof file_name, file_name);
	if (len >= sizeof file_name)
		fatal("%s: buffer too small", __FUNCTION__);
	ld_get_full_path(isa_ctx, file_name, full_path, sizeof full_path);
	sys_debug("  file_name_ptr=0x%x, mode=0x%x\n", file_name_ptr, mode);
	sys_debug("  file_name='%s', full_path='%s'\n", file_name, full_path);

	/* Host call */
	err = chmod(full_path, mode);
	if (err == -1)
		return -errno;

	/* Return */
	return err;
}




/*
 * System call 'lseek' (code 19)
 */

int sys_lseek_impl(void)
{
	unsigned int offset;

	int fd;
	int origin;
	int host_fd;
	int err;

	/* Arguments */
	fd = isa_regs->ebx;
	offset = isa_regs->ecx;
	origin = isa_regs->edx;
	host_fd = file_desc_table_get_host_fd(isa_ctx->file_desc_table, fd);
	sys_debug("  fd=%d, offset=0x%x, origin=0x%x\n",
		fd, offset, origin);
	sys_debug("  host_fd=%d\n", host_fd);

	/* Host call */
	err = lseek(host_fd, offset, origin);
	if (err == -1)
		return -errno;

	/* Return */
	return err;
}




/*
 * System call 'getpid' (code 20)
 */

int sys_getpid_impl(void)
{
	return isa_ctx->pid;
}




/*
 * System call 'utime' (code 30)
 */

struct sim_utimbuf
{
	unsigned int actime;
	unsigned int modtime;
};

static void sys_utime_guest_to_host(struct utimbuf *host, struct sim_utimbuf *guest)
{
	host->actime = guest->actime;
	host->modtime = guest->modtime;
}

int sys_utime_impl(void)
{
	unsigned int file_name_ptr;
	unsigned int utimbuf_ptr;

	struct utimbuf utimbuf;
	struct sim_utimbuf sim_utimbuf;

	int len;
	int err;

	char file_name[MAX_PATH_SIZE];
	char full_path[MAX_PATH_SIZE];

	/* Arguments */
	file_name_ptr = isa_regs->ebx;
	utimbuf_ptr = isa_regs->ecx;
	sys_debug("  file_name_ptr=0x%x, utimbuf_ptr=0x%x\n",
		file_name_ptr, utimbuf_ptr);

	/* Read file name */
	len = mem_read_string(isa_mem, file_name_ptr, sizeof file_name, file_name);
	if (len >= MAX_PATH_SIZE)
		fatal("%s: buffer too small", __FUNCTION__);

	/* Get full path */
	ld_get_full_path(isa_ctx, file_name, full_path, sizeof full_path);
	sys_debug("  file_name='%s', full_path='%s'\n", file_name, full_path);

	/* Read time buffer */
	mem_read(isa_mem, utimbuf_ptr, sizeof(struct sim_utimbuf), &sim_utimbuf);
	sys_utime_guest_to_host(&utimbuf, &sim_utimbuf);
	sys_debug("  utimbuf.actime = %u, utimbuf.modtime = %u\n",
		sim_utimbuf.actime, sim_utimbuf.modtime);

	/* Host call */
	err = utime(full_path, &utimbuf);
	if (err == -1)
		return -errno;

	/* Return */
	return err;
}




/*
 * System call 'access' (code 33)
 */

static struct string_map_t sys_access_mode_map =
{
	3, {
		{ "X_OK",  1 },
		{ "W_OK",  2 },
		{ "R_OK",  4 }
	}
};

int sys_access_impl(void)
{
	unsigned int file_name_ptr;

	int mode;
	int len;
	int err;

	char file_name[MAX_PATH_SIZE];
	char full_path[MAX_PATH_SIZE];
	char mode_str[MAX_STRING_SIZE];

	/* Arguments */
	file_name_ptr = isa_regs->ebx;
	mode = isa_regs->ecx;

	/* Read file name */
	len = mem_read_string(isa_mem, file_name_ptr, sizeof file_name, file_name);
	if (len >= sizeof file_name)
		fatal("%s: buffer too small", __FUNCTION__);

	/* Get full path */
	ld_get_full_path(isa_ctx, file_name, full_path, sizeof full_path);

	/* Debug */
	map_flags(&sys_access_mode_map, mode, mode_str, sizeof mode_str);
	sys_debug("  file_name='%s', mode=0x%x\n", file_name, mode);
	sys_debug("  full_path='%s'\n", full_path);
	sys_debug("  mode=%s\n", mode_str);

	/* Host call */
	err = access(full_path, mode);
	if (err == -1)
		return -errno;

	/* Return */
	return err;
}




/*
 * System call 'kill' (code 37)
 */

int sys_kill_impl(void)
{
	int pid;
	int sig;

	struct ctx_t *ctx;

	/* Arguments */
	pid = isa_regs->ebx;
	sig = isa_regs->ecx;
	sys_debug("  pid=%d, sig=%d (%s)\n", pid,
		sig, sim_signal_name(sig));

	/* Find context. We assume program correctness, so fatal if context is
	 * not found, rather than return error code. */
	ctx = ctx_get(pid);
	if (!ctx)
		fatal("%s: invalid pid %d", __FUNCTION__, pid);

	/* Send signal */
	sim_sigset_add(&ctx->signal_mask_table->pending, sig);
	ctx_host_thread_suspend_cancel(ctx);
	ke_process_events_schedule();
	ke_process_events();

	/* Success */
	return 0;
}




/*
 * System call 'rename' (code 38)
 */

int sys_rename_impl(void)
{
	unsigned int old_path_ptr;
	unsigned int new_path_ptr;

	char old_path[MAX_PATH_SIZE];
	char new_path[MAX_PATH_SIZE];
	char old_full_path[MAX_PATH_SIZE];
	char new_full_path[MAX_PATH_SIZE];

	int len;
	int err;

	/* Arguments */
	old_path_ptr = isa_regs->ebx;
	new_path_ptr = isa_regs->ecx;
	sys_debug("  old_path_ptr=0x%x, new_path_ptr=0x%x\n",
		old_path_ptr, new_path_ptr);

	/* Get old path */
	len = mem_read_string(isa_mem, old_path_ptr, sizeof old_path, old_path);
	if (len >= sizeof old_path)
		fatal("%s: buffer too small", __FUNCTION__);

	/* Get new path */
	len = mem_read_string(isa_mem, new_path_ptr, sizeof new_path, new_path);
	if (len >= sizeof new_path)
		fatal("%s: buffer too small", __FUNCTION__);

	/* Get full paths */
	ld_get_full_path(isa_ctx, old_path, old_full_path, sizeof old_full_path);
	ld_get_full_path(isa_ctx, new_path, new_full_path, sizeof new_full_path);
	sys_debug("  old_path='%s', new_path='%s'\n", old_path, new_path);
	sys_debug("  old_full_path='%s', new_full_path='%s'\n", old_full_path, new_full_path);

	/* Host call */
	err = rename(old_full_path, new_full_path);
	if (err == -1)
		return -errno;

	/* Return */
	return err;
}




/*
 * System call 'mkdir' (code 39)
 */

int sys_mkdir_impl(void)
{
	unsigned int path_ptr;

	int mode;
	int len;
	int err;

	char path[MAX_PATH_SIZE];
	char full_path[MAX_PATH_SIZE];

	/* Arguments */
	path_ptr = isa_regs->ebx;
	mode = isa_regs->ecx;
	sys_debug("  path_ptr=0x%x, mode=0x%x\n", path_ptr, mode);

	/* Read path */
	len = mem_read_string(isa_mem, path_ptr, sizeof path, path);
	if (len >= sizeof path)
		fatal("%s: buffer too small", __FUNCTION__);

	/* Read full path */
	ld_get_full_path(isa_ctx, path, full_path, MAX_PATH_SIZE);
	sys_debug("  path='%s', full_path='%s'\n", path, full_path);

	/* Host call */
	err = mkdir(full_path, mode);
	if (err == -1)
		return -errno;

	/* Return */
	return err;
}




/*
 * System call 'dup' (code 41)
 */

int sys_dup_impl(void)
{
	int guest_fd;
	int dup_guest_fd;
	int host_fd;
	int dup_host_fd;

	struct file_desc_t *desc;
	struct file_desc_t *dup_desc;

	/* Arguments */
	guest_fd = isa_regs->ebx;
	sys_debug("  guest_fd=%d\n", guest_fd);

	/* Check that file descriptor is valid. */
	desc = file_desc_table_entry_get(isa_ctx->file_desc_table, guest_fd);
	if (!desc)
		return -EBADF;
	host_fd = desc->host_fd;
	sys_debug("  host_fd=%d\n", host_fd);

	/* Duplicate host file descriptor. */
	dup_host_fd = dup(host_fd);
	if (dup_host_fd == -1)
		return -errno;

	/* Create a new entry in the file descriptor table. */
	dup_desc = file_desc_table_entry_new(isa_ctx->file_desc_table,
		file_desc_regular, dup_host_fd, desc->path, desc->flags);
	dup_guest_fd = dup_desc->guest_fd;

	/* Return new file descriptor. */
	return dup_guest_fd;
}




/*
 * System call 'pipe' (code 42)
 */

int sys_pipe_impl(void)
{
	unsigned int fd_ptr;

	struct file_desc_t *read_desc;
	struct file_desc_t *write_desc;

	int guest_read_fd;
	int guest_write_fd;

	int host_fd[2];
	int err;

	/* Arguments */
	fd_ptr = isa_regs->ebx;
	sys_debug("  fd_ptr=0x%x\n", fd_ptr);

	/* Create host pipe */
	err = pipe(host_fd);
	if (err == -1)
		fatal("%s: cannot create pipe", __FUNCTION__);
	sys_debug("  host pipe created: fd={%d, %d}\n",
		host_fd[0], host_fd[1]);

	/* Create guest pipe */
	read_desc = file_desc_table_entry_new(isa_ctx->file_desc_table,
		file_desc_pipe, host_fd[0], "", O_RDONLY);
	write_desc = file_desc_table_entry_new(isa_ctx->file_desc_table,
		file_desc_pipe, host_fd[1], "", O_WRONLY);
	sys_debug("  guest pipe created: fd={%d, %d}\n",
		read_desc->guest_fd, write_desc->guest_fd);
	guest_read_fd = read_desc->guest_fd;
	guest_write_fd = write_desc->guest_fd;

	/* Return file descriptors. */
	mem_write(isa_mem, fd_ptr, 4, &guest_read_fd);
	mem_write(isa_mem, fd_ptr + 4, 4, &guest_write_fd);
	return 0;
}




/*
 * System call 'times' (code 43)
 */

struct sim_tms
{
	unsigned int utime;
	unsigned int stime;
	unsigned int cutime;
	unsigned int cstime;
};

static void sys_times_host_to_guest(struct sim_tms *guest, struct tms *host)
{
	guest->utime = host->tms_utime;
	guest->stime = host->tms_stime;
	guest->cutime = host->tms_cutime;
	guest->cstime = host->tms_cstime;
}

int sys_times_impl(void)
{
	unsigned int tms_ptr;

	struct tms tms;
	struct sim_tms sim_tms;

	int err;

	/* Arguments */
	tms_ptr = isa_regs->ebx;
	sys_debug("  tms_ptr=0x%x\n", tms_ptr);

	/* Host call */
	err = times(&tms);

	/* Write result in memory */
	if (tms_ptr)
	{
		sys_times_host_to_guest(&sim_tms, &tms);
		mem_write(isa_mem, tms_ptr, sizeof(sim_tms), &sim_tms);
	}

	/* Return */
	return err;
}




/*
 * System call 'brk' (code 45)
 */

int sys_brk_impl(void)
{
	unsigned int old_heap_break;
	unsigned int new_heap_break;
	unsigned int size;

	unsigned int old_heap_break_aligned;
	unsigned int new_heap_break_aligned;

	/* Arguments */
	new_heap_break = isa_regs->ebx;
	old_heap_break = isa_mem->heap_break;
	sys_debug("  newbrk=0x%x (previous brk was 0x%x)\n",
		new_heap_break, old_heap_break);

	/* Align */
	new_heap_break_aligned = ROUND_UP(new_heap_break, MEM_PAGE_SIZE);
	old_heap_break_aligned = ROUND_UP(old_heap_break, MEM_PAGE_SIZE);

	/* If argument is zero, the system call is used to
	 * obtain the current top of the heap. */
	if (!new_heap_break)
		return old_heap_break;

	/* If the heap is increased: if some page in the way is
	 * allocated, do nothing and return old heap top. Otherwise,
	 * allocate pages and return new heap top. */
	if (new_heap_break > old_heap_break)
	{
		size = new_heap_break_aligned - old_heap_break_aligned;
		if (size)
		{
			if (mem_map_space(isa_mem, old_heap_break_aligned, size) != old_heap_break_aligned)
				fatal("%s: out of memory", __FUNCTION__);
			mem_map(isa_mem, old_heap_break_aligned, size,
				mem_access_read | mem_access_write);
		}
		isa_mem->heap_break = new_heap_break;
		sys_debug("  heap grows %u bytes\n", new_heap_break - old_heap_break);
		return new_heap_break;
	}

	/* Always allow to shrink the heap. */
	if (new_heap_break < old_heap_break)
	{
		size = old_heap_break_aligned - new_heap_break_aligned;
		if (size)
			mem_unmap(isa_mem, new_heap_break_aligned, size);
		isa_mem->heap_break = new_heap_break;
		sys_debug("  heap shrinks %u bytes\n", old_heap_break - new_heap_break);
		return new_heap_break;
	}

	/* Heap stays the same */
	return 0;
}




/*
 * System call 'ioctl' (code 54)
 */

/* An 'ioctl' code (first argument) is a 32-bit word split into 4 fields:
 *   -NR [7..0]: ioctl code number.
 *   -TYPE [15..8]: ioctl category.
 *   -SIZE [29..16]: size of the structure passed as 2nd argument.
 *   -DIR [31..30]: direction (01=Write, 10=Read, 11=R/W).
 */

int sys_ioctl_impl(void)
{
	unsigned int cmd;
	unsigned int arg;

	int guest_fd;
	int err;

	struct file_desc_t *desc;

	/* Arguments */
	guest_fd = isa_regs->ebx;
	cmd = isa_regs->ecx;
	arg = isa_regs->edx;
	sys_debug("  guest_fd=%d, cmd=0x%x, arg=0x%x\n",
		guest_fd, cmd, arg);

	/* File descriptor */
	desc = file_desc_table_entry_get(isa_ctx->file_desc_table, guest_fd);
	if (!desc)
		return -EBADF;

	/* Process IOCTL */
	if (cmd >= 0x5401 || cmd <= 0x5408)
	{
		/* 'ioctl' commands using 'struct termios' as the argument.
		 * This structure is 60 bytes long both for x86 and x86_64
		 * architectures, so it doesn't vary between guest/host.
		 * No translation needed, so just use a 60-byte I/O buffer. */
		unsigned char buf[60];

		/* Read buffer */
		mem_read(isa_mem, arg, sizeof buf, buf);
		err = ioctl(desc->host_fd, cmd, &buf);
		if (err == -1)
			return -errno;

		/* Return in memory */
		mem_write(isa_mem, arg, sizeof buf, buf);
		return err;
	}
	else
	{
		fatal("%s: not implement for cmd = 0x%x.\n%s",
			__FUNCTION__, cmd, err_sys_note);
	}

	/* Return */
	return 0;
}




/*
 * System call 'getppid' (code 64)
 */

int sys_getppid_impl(void)
{
	/* Return 1 if there is no parent */
	if (!isa_ctx->parent)
		return 1;

	/* Return parent's ID */
	return isa_ctx->parent->pid;
}




/*
 * System call 'setrlimit' (code 75)
 */

static struct string_map_t sys_rlimit_res_map =
{
	16, {

		{ "RLIMIT_CPU",              0 },
		{ "RLIMIT_FSIZE",            1 },
		{ "RLIMIT_DATA",             2 },
		{ "RLIMIT_STACK",            3 },
		{ "RLIMIT_CORE",             4 },
		{ "RLIMIT_RSS",              5 },
		{ "RLIMIT_NPROC",            6 },
		{ "RLIMIT_NOFILE",           7 },
		{ "RLIMIT_MEMLOCK",          8 },
		{ "RLIMIT_AS",               9 },
		{ "RLIMIT_LOCKS",            10 },
		{ "RLIMIT_SIGPENDING",       11 },
		{ "RLIMIT_MSGQUEUE",         12 },
		{ "RLIMIT_NICE",             13 },
		{ "RLIMIT_RTPRIO",           14 },
		{ "RLIM_NLIMITS",            15 }
	}
};

struct sim_rlimit
{
	unsigned int cur;
	unsigned int max;
};

int sys_setrlimit_impl(void)
{
	unsigned int res;
	unsigned int rlim_ptr;

	char *res_str;

	struct sim_rlimit sim_rlimit;

	/* Arguments */
	res = isa_regs->ebx;
	rlim_ptr = isa_regs->ecx;
	res_str = map_value(&sys_rlimit_res_map, res);
	sys_debug("  res=0x%x, rlim_ptr=0x%x\n", res, rlim_ptr);
	sys_debug("  res=%s\n", res_str);

	/* Read structure */
	mem_read(isa_mem, rlim_ptr, sizeof(struct sim_rlimit), &sim_rlimit);
	sys_debug("  rlim->cur=0x%x, rlim->max=0x%x\n",
		sim_rlimit.cur, sim_rlimit.max);

	/* Different actions depending on resource type */
	switch (res)
	{

	case RLIMIT_DATA:
	{
		/* Default limit is maximum.
		 * This system call is ignored. */
		break;
	}

	case RLIMIT_STACK:
	{
		/* A program should allocate its stack with calls to mmap.
		 * This should be a limit for the stack, which is ignored here. */
		break;
	}

	default:
		fatal("%s: not implemented for res = %s.\n%s",
			__FUNCTION__, res_str, err_sys_note);
	}

	/* Return */
	return 0;
}




/*
 * System call 'getrusage' (code 77)
 */

struct sim_rusage
{
	unsigned int utime_sec, utime_usec;
	unsigned int stime_sec, stime_usec;
	unsigned int maxrss;
	unsigned int ixrss;
	unsigned int idrss;
	unsigned int isrss;
	unsigned int minflt;
	unsigned int majflt;
	unsigned int nswap;
	unsigned int inblock;
	unsigned int oublock;
	unsigned int msgsnd;
	unsigned int msgrcv;
	unsigned int nsignals;
	unsigned int nvcsw;
	unsigned int nivcsw;
};

static void sys_rusage_host_to_guest(struct sim_rusage *guest, struct rusage *host)
{
	guest->utime_sec = host->ru_utime.tv_sec;
	guest->utime_usec = host->ru_utime.tv_usec;
	guest->stime_sec = host->ru_stime.tv_sec;
	guest->stime_usec = host->ru_stime.tv_usec;
	guest->maxrss = host->ru_maxrss;
	guest->ixrss = host->ru_ixrss;
	guest->idrss = host->ru_idrss;
	guest->isrss = host->ru_isrss;
	guest->minflt = host->ru_minflt;
	guest->majflt = host->ru_majflt;
	guest->nswap = host->ru_nswap;
	guest->inblock = host->ru_inblock;
	guest->oublock = host->ru_oublock;
	guest->msgsnd = host->ru_msgsnd;
	guest->msgrcv = host->ru_msgrcv;
	guest->nsignals = host->ru_nsignals;
	guest->nvcsw = host->ru_nvcsw;
	guest->nivcsw = host->ru_nivcsw;
}

int sys_getrusage_impl(void)
{
	unsigned int who;
	unsigned int u_ptr;

	struct rusage rusage;
	struct sim_rusage sim_rusage;

	int err;

	/* Arguments */
	who = isa_regs->ebx;
	u_ptr = isa_regs->ecx;
	sys_debug("  who=0x%x, pru=0x%x\n", who, u_ptr);

	/* Supported values */
	if (who != 0)  /* RUSAGE_SELF */
		fatal("%s: not implemented for who != RUSAGE_SELF.\n%s",
			__FUNCTION__, err_sys_note);

	/* Host call */
	err = getrusage(RUSAGE_SELF, &rusage);
	if (err == -1)
		return -errno;

	/* Return structure */
	sys_rusage_host_to_guest(&sim_rusage, &rusage);
	mem_write(isa_mem, u_ptr, sizeof sim_rusage, &sim_rusage);

	/* Application expects this additional values updated:
	 * ru_maxrss: maximum resident set size
	 * ru_ixrss: integral shared memory size
	 * ru_idrss: integral unshared data size
	 * ru_isrss: integral unshared stack size */
	return 0;
}




/*
 * System call 'gettimeofday' (code 78)
 */

int sys_gettimeofday_impl(void)
{
	unsigned int tv_ptr;
	unsigned int tz_ptr;

	struct timeval tv;
	struct timezone tz;

	/* Arguments */
	tv_ptr = isa_regs->ebx;
	tz_ptr = isa_regs->ecx;
	sys_debug("  tv_ptr=0x%x, tz_ptr=0x%x\n", tv_ptr, tz_ptr);

	/* Host call */
	gettimeofday(&tv, &tz);

	/* Write time value */
	if (tv_ptr)
	{
		mem_write(isa_mem, tv_ptr, 4, &tv.tv_sec);
		mem_write(isa_mem, tv_ptr + 4, 4, &tv.tv_usec);
	}

	/* Write time zone */
	if (tz_ptr)
	{
		mem_write(isa_mem, tz_ptr, 4, &tz.tz_minuteswest);
		mem_write(isa_mem, tz_ptr + 4, 4, &tz.tz_dsttime);
	}

	/* Return */
	return 0;
}




/*
 * System call 'readlink' (code 85)
 */

int sys_readlink_impl(void)
{
	unsigned int path_ptr;
	unsigned int buf;
	unsigned int bufsz;

	char path[MAX_PATH_SIZE];
	char full_path[MAX_PATH_SIZE];
	char dest_path[MAX_PATH_SIZE];

	int dest_size;
	int len;
	int err;

	/* Arguments */
	path_ptr = isa_regs->ebx;
	buf = isa_regs->ecx;
	bufsz = isa_regs->edx;
	sys_debug("  path_ptr=0x%x, buf=0x%x, bufsz=%d\n", path_ptr, buf, bufsz);

	/* Read path */
	len = mem_read_string(isa_mem, path_ptr, sizeof path, path);
	if (len == sizeof path)
		fatal("%s: buffer too small", __FUNCTION__);

	/* Get full path */
	ld_get_full_path(isa_ctx, path, full_path, sizeof full_path);
	sys_debug("  path='%s', full_path='%s'\n", path, full_path);

	/* Special file '/proc/self/exe' intercepted */
	if (!strcmp(full_path, "/proc/self/exe"))
	{
		/* Return path to simulated executable */
		if (strlen(isa_ctx->loader->exe) >= sizeof dest_path)
			fatal("%s: buffer too small", __FUNCTION__);
		strcpy(dest_path, isa_ctx->loader->exe);
	}
	else
	{
		/* Host call */
		memset(dest_path, 0, sizeof dest_path);
		err = readlink(full_path, dest_path, sizeof dest_path);
		if (err == sizeof dest_path)
			fatal("%s: buffer too small", __FUNCTION__);
		if (err == -1)
			return -errno;
	}

	/* Copy name to guest memory. The string is not null-terminated. */
	dest_size = MAX(strlen(dest_path), bufsz);
	mem_write(isa_mem, buf, dest_size, dest_path);
	sys_debug("  dest_path='%s'\n", dest_path);

	/* Return number of bytes copied */
	return dest_size;
}




/*
 * System call 'mmap' (code 90)
 */

#define SYS_MMAP_BASE_ADDRESS  0xb7fb0000

static struct string_map_t sys_mmap_prot_map =
{
	6, {
		{ "PROT_READ",       0x1 },
		{ "PROT_WRITE",      0x2 },
		{ "PROT_EXEC",       0x4 },
		{ "PROT_SEM",        0x8 },
		{ "PROT_GROWSDOWN",  0x01000000 },
		{ "PROT_GROWSUP",    0x02000000 }
	}
};

static struct string_map_t sys_mmap_flags_map =
{
	11, {
		{ "MAP_SHARED",      0x01 },
		{ "MAP_PRIVATE",     0x02 },
		{ "MAP_FIXED",       0x10 },
		{ "MAP_ANONYMOUS",   0x20 },
		{ "MAP_GROWSDOWN",   0x00100 },
		{ "MAP_DENYWRITE",   0x00800 },
		{ "MAP_EXECUTABLE",  0x01000 },
		{ "MAP_LOCKED",      0x02000 },
		{ "MAP_NORESERVE",   0x04000 },
		{ "MAP_POPULATE",    0x08000 },
		{ "MAP_NONBLOCK",    0x10000 }
	}
};

static int sys_mmap(unsigned int addr, unsigned int len, int prot,
	int flags, int guest_fd, int offset)
{
	unsigned int len_aligned;

	int perm;
	int host_fd;

	struct file_desc_t *desc;

	/* Check that protection flags match in guest and host */
	assert(PROT_READ == 1);
	assert(PROT_WRITE == 2);
	assert(PROT_EXEC == 4);

	/* Check that mapping flags match */
	assert(MAP_SHARED == 0x01);
	assert(MAP_PRIVATE == 0x02);
	assert(MAP_FIXED == 0x10);
	assert(MAP_ANONYMOUS == 0x20);

	/* Translate file descriptor */
	desc = file_desc_table_entry_get(isa_ctx->file_desc_table, guest_fd);
	host_fd = desc ? desc->host_fd : -1;
	if (guest_fd > 0 && host_fd < 0)
		fatal("%s: invalid guest descriptor", __FUNCTION__);

	/* Permissions */
	perm = mem_access_init;
	perm |= prot & PROT_READ ? mem_access_read : 0;
	perm |= prot & PROT_WRITE ? mem_access_write : 0;
	perm |= prot & PROT_EXEC ? mem_access_exec : 0;

	/* Flag MAP_ANONYMOUS.
	 * If it is set, the 'fd' parameter is ignored. */
	if (flags & MAP_ANONYMOUS)
		host_fd = -1;

	/* 'addr' and 'offset' must be aligned to page size boundaries.
	 * 'len' is rounded up to page boundary. */
	if (offset & ~MEM_PAGE_MASK)
		fatal("%s: unaligned offset", __FUNCTION__);
	if (addr & ~MEM_PAGE_MASK)
		fatal("%s: unaligned address", __FUNCTION__);
	len_aligned = ROUND_UP(len, MEM_PAGE_SIZE);

	/* Find region for allocation */
	if (flags & MAP_FIXED)
	{
		/* If MAP_FIXED is set, the 'addr' parameter must be obeyed, and is not just a
		 * hint for a possible base address of the allocated range. */
		if (!addr)
			fatal("%s: no start specified for fixed mapping", __FUNCTION__);

		/* Any allocated page in the range specified by 'addr' and 'len'
		 * must be discarded. */
		mem_unmap(isa_mem, addr, len_aligned);
	}
	else
	{
		if (!addr || mem_map_space_down(isa_mem, addr, len_aligned) != addr)
			addr = SYS_MMAP_BASE_ADDRESS;
		addr = mem_map_space_down(isa_mem, addr, len_aligned);
		if (addr == -1)
			fatal("%s: out of guest memory", __FUNCTION__);
	}

	/* Allocation of memory */
	mem_map(isa_mem, addr, len_aligned, perm);

	/* Host mapping */
	if (host_fd >= 0)
	{
		char buf[MEM_PAGE_SIZE];

		unsigned int last_pos;
		unsigned int curr_addr;

		int size;
		int count;

		/* Save previous position */
		last_pos = lseek(host_fd, 0, SEEK_CUR);
		lseek(host_fd, offset, SEEK_SET);

		/* Read pages */
		assert(len_aligned % MEM_PAGE_SIZE == 0);
		assert(addr % MEM_PAGE_SIZE == 0);
		curr_addr = addr;
		for (size = len_aligned; size > 0; size -= MEM_PAGE_SIZE)
		{
			memset(buf, 0, MEM_PAGE_SIZE);
			count = read(host_fd, buf, MEM_PAGE_SIZE);
			if (count)
				mem_access(isa_mem, curr_addr, MEM_PAGE_SIZE, buf, mem_access_init);
			curr_addr += MEM_PAGE_SIZE;
		}

		/* Return file to last position */
		lseek(host_fd, last_pos, SEEK_SET);
	}

	/* Return mapped address */
	return addr;
}

int sys_mmap_impl(void)
{
	unsigned int args_ptr;
	unsigned int addr;
	unsigned int len;

	int prot;
	int flags;
	int offset;
	int guest_fd;

	char prot_str[MAX_STRING_SIZE];
	char flags_str[MAX_STRING_SIZE];

	/* This system call takes the arguments from memory, at the address
	 * pointed by 'ebx'. */
	args_ptr = isa_regs->ebx;
	mem_read(isa_mem, args_ptr, 4, &addr);
	mem_read(isa_mem, args_ptr + 4, 4, &len);
	mem_read(isa_mem, args_ptr + 8, 4, &prot);
	mem_read(isa_mem, args_ptr + 12, 4, &flags);
	mem_read(isa_mem, args_ptr + 16, 4, &guest_fd);
	mem_read(isa_mem, args_ptr + 20, 4, &offset);

	sys_debug("  args_ptr=0x%x\n", args_ptr);
	sys_debug("  addr=0x%x, len=%u, prot=0x%x, flags=0x%x, "
		"guest_fd=%d, offset=0x%x\n",
		addr, len, prot, flags, guest_fd, offset);
	map_flags(&sys_mmap_prot_map, prot, prot_str, sizeof prot_str);
	map_flags(&sys_mmap_flags_map, flags, flags_str, sizeof flags_str);
	sys_debug("  prot=%s, flags=%s\n", prot_str, flags_str);

	/* Call */
	return sys_mmap(addr, len, prot, flags, guest_fd, offset);
}




/*
 * System call 'munmap' (code 91)
 */

int sys_munmap_impl(void)
{
	unsigned int addr;
	unsigned int size;
	unsigned int size_aligned;

	/* Arguments */
	addr = isa_regs->ebx;
	size = isa_regs->ecx;
	sys_debug("  addr=0x%x, size=0x%x\n", addr, size);

	/* Restrictions */
	if (addr & (MEM_PAGE_SIZE - 1))
		fatal("%s: address not aligned", __FUNCTION__);

	/* Unmap */
	size_aligned = ROUND_UP(size, MEM_PAGE_SIZE);
	mem_unmap(isa_mem, addr, size_aligned);

	/* Return */
	return 0;
}




/*
 * System call 'fchmod' (code 94)
 */

int sys_fchmod_impl(void)
{
	int fd;
	int host_fd;
	int mode;
	int err;

	/* Arguments */
	fd = isa_regs->ebx;
	mode = isa_regs->ecx;
	sys_debug("  fd=%d, mode=%d\n", fd, mode);

	/* Get host descriptor */
	host_fd = file_desc_table_get_host_fd(isa_ctx->file_desc_table, fd);
	sys_debug("  host_fd=%d\n", host_fd);

	/* Host call */
	err = fchmod(host_fd, mode);
	if (err == -1)
		return -errno;

	/* Return */
	return err;
}




/*
 * System call 'socketcall' (code 102)
 */

static struct string_map_t sys_socketcall_call_map =
{
	17, {
		{ "SYS_SOCKET",		1 },
		{ "SYS_BIND",		2 },
		{ "SYS_CONNECT",	3 },
		{ "SYS_LISTEN",		4 },
		{ "SYS_ACCEPT",		5 },
		{ "SYS_GETSOCKNAME",	6 },
		{ "SYS_GETPEERNAME",	7 },
		{ "SYS_SOCKETPAIR",	8 },
		{ "SYS_SEND",		9 },
		{ "SYS_RECV",		10 },
		{ "SYS_SENDTO",		11 },
		{ "SYS_RECVFROM",	12 },
		{ "SYS_SHUTDOWN",	13 },
		{ "SYS_SETSOCKOPT",	14 },
		{ "SYS_GETSOCKOPT",	15 },
		{ "SYS_SENDMSG",	16 },
		{ "SYS_RECVMSG",	17 }
	}
};

static struct string_map_t sys_socket_family_map =
{
	29, {
		{ "PF_UNSPEC",		0 },
		{ "PF_UNIX",		1 },
		{ "PF_INET",		2 },
		{ "PF_AX25",		3 },
		{ "PF_IPX",		4 },
		{ "PF_APPLETALK",	5 },
		{ "PF_NETROM",		6 },
		{ "PF_BRIDGE",		7 },
		{ "PF_ATMPVC",		8 },
		{ "PF_X25",		9 },
		{ "PF_INET6",		10 },
		{ "PF_ROSE",		11 },
		{ "PF_DECnet",		12 },
		{ "PF_NETBEUI",		13 },
		{ "PF_SECURITY",	14 },
		{ "PF_KEY",		15 },
		{ "PF_NETLINK",		16 },
		{ "PF_PACKET",		17 },
		{ "PF_ASH",		18 },
		{ "PF_ECONET",		19 },
		{ "PF_ATMSVC",		20 },
		{ "PF_SNA",		22 },
		{ "PF_IRDA",		23 },
		{ "PF_PPPOX",		24 },
		{ "PF_WANPIPE",		25 },
		{ "PF_LLC",		26 },
		{ "PF_TIPC",		30 },
		{ "PF_BLUETOOTH",	31 },
		{ "PF_IUCV",		32 }
	}
};

static struct string_map_t sys_socket_type_map =
{
	7, {
		{ "SOCK_STREAM",	1 },
		{ "SOCK_DGRAM",		2 },
		{ "SOCK_RAW",		3 },
		{ "SOCK_RDM",		4 },
		{ "SOCK_SEQPACKET",	5 },
		{ "SOCK_DCCP",		6 },
		{ "SOCK_PACKET",	10 }
	}
};

int sys_socketcall_impl(void)
{
	int call;
	unsigned int args;
	char *call_str;

	/* Arguments */
	call = isa_regs->ebx;
	args = isa_regs->ecx;
	call_str = map_value(&sys_socketcall_call_map, call);
	sys_debug("  call=%d (%s), args=0x%x\n", call, call_str, args);

	/* Process call */
	switch (call)
	{

	/* SYS_SOCKET */
	case 1:
	{
		unsigned int family;
		unsigned int type;
		unsigned int protocol;

		char *family_str;
		char type_str[MAX_STRING_SIZE];

		int host_fd;

		struct file_desc_t *desc;

		/* Read arguments */
		mem_read(isa_mem, args, 4, &family);
		mem_read(isa_mem, args + 4, 4, &type);
		mem_read(isa_mem, args + 8, 4, &protocol);

		/* Debug */
		family_str = map_value(&sys_socket_family_map, family);
		snprintf(type_str, sizeof type_str, "%s%s%s",
				map_value(&sys_socket_type_map, type & 0xff),
				type & 0x80000 ? "|SOCK_CLOEXEC" : "",
				type & 0x800 ? "|SOCK_NONBLOCK" : "");
		sys_debug("  family=%d (%s)\n", family, family_str);
		sys_debug("  type=0x%x (%s)\n", type, type_str);
		sys_debug("  protocol=%d\n", protocol);

		/* Allow only sockets of type SOCK_STREAM */
		if ((type & 0xff) != 1)
			fatal("%s: SYS_SOCKET: only type SOCK_STREAM supported",
					__FUNCTION__);

		/* Create socket */
		host_fd = socket(family, type, protocol);
		if (host_fd == -1)
			return -errno;

		/* Create new file descriptor table entry. */
		desc = file_desc_table_entry_new(isa_ctx->file_desc_table,
				file_desc_socket, host_fd, "", O_RDWR);
		sys_debug("    socket created: guest_fd=%d, host_fd=%d\n",
			desc->guest_fd, desc->host_fd);

		/* Return socket */
		return desc->guest_fd;
	}

	/* SYS_CONNECT */
	case 3:
	{
		unsigned int guest_fd;
		unsigned int addr_ptr;
		unsigned int addr_len;

		struct file_desc_t *desc;

		char buf[MAX_STRING_SIZE];

		struct sockaddr *addr;

		int err;

		/* Read arguments */
		mem_read(isa_mem, args, 4, &guest_fd);
		mem_read(isa_mem, args + 4, 4, &addr_ptr);
		mem_read(isa_mem, args + 8, 4, &addr_len);
		sys_debug("  guest_fd=%d, paddr=0x%x, addrlen=%d\n",
			guest_fd, addr_ptr, addr_len);

		/* Check buffer size */
		if (addr_len > sizeof buf)
			fatal("%s: SYS_CONNECT: buffer too small", __FUNCTION__);

		/* Host architecture assumptions */
		M2S_HOST_GUEST_MATCH(sizeof addr->sa_family, 2);
		M2S_HOST_GUEST_MATCH((void *) &addr->sa_data - (void *) &addr->sa_family, 2);

		/* Get 'sockaddr' structure, read family and data */
		addr = (struct sockaddr *) &buf[0];
		mem_read(isa_mem, addr_ptr, addr_len, addr);
		sys_debug("    sockaddr.family=%s\n", map_value(&sys_socket_family_map, addr->sa_family));
		sys_debug_string("    sockaddr.data", addr->sa_data, addr_len - 2, 1);

		/* Get file descriptor */
		desc = file_desc_table_entry_get(isa_ctx->file_desc_table, guest_fd);
		if (!desc)
			return -EBADF;

		/* Check that it's a socket */
		if (desc->kind != file_desc_socket)
			fatal("%s: SYS_CONNECT: file not a socket", __FUNCTION__);
		sys_debug("    host_fd=%d\n", desc->host_fd);

		/* Connect socket */
		err = connect(desc->host_fd, addr, addr_len);
		if (err == -1)
			return -errno;

		/* Return */
		return err;
	}

	/* SYS_GETPEERNAME */
	case 7:
	{
		int guest_fd;
		int addr_len;
		int err;

		unsigned int addr_ptr;
		unsigned int addr_len_ptr;

		struct file_desc_t *desc;
		struct sockaddr *addr;
		socklen_t host_addr_len;

		mem_read(isa_mem, args, 4, &guest_fd);
		mem_read(isa_mem, args + 4, 4, &addr_ptr);
		mem_read(isa_mem, args + 8, 4, &addr_len_ptr);
		sys_debug("  guest_fd=%d, paddr=0x%x, paddrlen=0x%x\n",
			guest_fd, addr_ptr, addr_len_ptr);

		/* Get file descriptor */
		desc = file_desc_table_entry_get(isa_ctx->file_desc_table, guest_fd);
		if (!desc)
			return -EBADF;

		/* Read current buffer size and allocate buffer. */
		mem_read(isa_mem, addr_len_ptr, 4, &addr_len);
		sys_debug("    addrlen=%d\n", addr_len);
		host_addr_len = addr_len;
		addr = malloc(addr_len);
		if (!addr)
			fatal("%s: out of memory", __FUNCTION__);

		/* Get peer name */
		err = getpeername(desc->host_fd, addr, &host_addr_len);
		if (err == -1)
		{
			free(addr);
			return -errno;
		}

		/* Address length returned */
		addr_len = host_addr_len;
		sys_debug("  result:\n");
		sys_debug("    addrlen=%d\n", host_addr_len);
		sys_debug_string("    sockaddr.data", addr->sa_data, addr_len - 2, 1);

		/* Copy result to guest memory */
		mem_write(isa_mem, addr_len_ptr, 4, &addr_len);
		mem_write(isa_mem, addr_ptr, addr_len, addr);

		/* Free and return */
		free(addr);
		return err;
	}

	default:

		fatal("%s: call '%s' not implemented",
			__FUNCTION__, call_str);
	}

	/* Dead code */
	return 0;
}




/*
 * System call 'setitimer' (code 104)
 */

static struct string_map_t sys_itimer_which_map =
{
	3, {
		{"ITIMER_REAL",		0},
		{"ITIMER_VIRTUAL",	1},
		{"ITIMER_PROF",		2}
	}
};

struct sim_timeval
{
	unsigned int tv_sec;
	unsigned int tv_usec;
};

struct sim_itimerval
{
	struct sim_timeval it_interval;
	struct sim_timeval it_value;
};

static void sim_timeval_dump(struct sim_timeval *sim_timeval)
{
	sys_debug("    tv_sec=%u, tv_usec=%u\n",
		sim_timeval->tv_sec, sim_timeval->tv_usec);
}

static void sim_itimerval_dump(struct sim_itimerval *sim_itimerval)
{
	sys_debug("    it_interval: tv_sec=%u, tv_usec=%u\n",
		sim_itimerval->it_interval.tv_sec, sim_itimerval->it_interval.tv_usec);
	sys_debug("    it_value: tv_sec=%u, tv_usec=%u\n",
		sim_itimerval->it_value.tv_sec, sim_itimerval->it_value.tv_usec);
}

int sys_setitimer_impl(void)
{
	unsigned int which;
	unsigned int value_ptr;
	unsigned int old_value_ptr;

	struct sim_itimerval itimerval;

	long long now;

	/* Arguments */
	which = isa_regs->ebx;
	value_ptr = isa_regs->ecx;
	old_value_ptr = isa_regs->edx;
	sys_debug("  which=%d (%s), value_ptr=0x%x, old_value_ptr=0x%x\n",
		which, map_value(&sys_itimer_which_map, which), value_ptr, old_value_ptr);

	/* Get current time */
	now = ke_timer();

	/* Read value */
	if (value_ptr)
	{
		mem_read(isa_mem, value_ptr, sizeof itimerval, &itimerval);
		sys_debug("  itimerval at 'value_ptr':\n");
		sim_itimerval_dump(&itimerval);
	}

	/* Check range of 'which' */
	if (which >= 3)
		fatal("%s: invalid value for 'which'", __FUNCTION__);

	/* Set 'itimer_value' (ke_timer domain) and 'itimer_interval' (usec) */
	isa_ctx->itimer_value[which] = now + itimerval.it_value.tv_sec * 1000000
		+ itimerval.it_value.tv_usec;
	isa_ctx->itimer_interval[which] = itimerval.it_interval.tv_sec * 1000000
		+ itimerval.it_interval.tv_usec;

	/* New timer inserted, so interrupt current 'ke_host_thread_timer'
	 * waiting for the next timer expiration. */
	ctx_host_thread_timer_cancel(isa_ctx);
	ke_process_events_schedule();

	/* Return */
	return 0;
}




/*
 * System call 'getitimer' (code 105)
 */

int sys_getitimer_impl(void)
{
	unsigned int which;
	unsigned int value_ptr;

	struct sim_itimerval itimerval;

	long long now;
	long long rem;

	/* Arguments */
	which = isa_regs->ebx;
	value_ptr = isa_regs->ecx;
	sys_debug("  which=%d (%s), value_ptr=0x%x\n",
		which, map_value(&sys_itimer_which_map, which), value_ptr);

	/* Get current time */
	now = ke_timer();

	/* Check range of 'which' */
	if (which >= 3)
		fatal("syscall 'getitimer': wrong value for 'which' argument");

	/* Return value in structure */
	rem = now < isa_ctx->itimer_value[which] ? isa_ctx->itimer_value[which] - now : 0;
	itimerval.it_value.tv_sec = rem / 1000000;
	itimerval.it_value.tv_usec = rem % 1000000;
	itimerval.it_interval.tv_sec = isa_ctx->itimer_interval[which] / 1000000;
	itimerval.it_interval.tv_usec = isa_ctx->itimer_interval[which] % 1000000;
	mem_write(isa_mem, value_ptr, sizeof itimerval, &itimerval);

	/* Return */
	return 0;
}




/*
 * System call 'sigreturn' (code 119)
 */

int sys_sigreturn_impl(void)
{
	signal_handler_return(isa_ctx);

	ke_process_events_schedule();
	ke_process_events();

	return 0;
}




/*
 * System call 'clone' (code 120)
 */

#define SIM_CLONE_VM			0x00000100
#define SIM_CLONE_FS			0x00000200
#define SIM_CLONE_FILES			0x00000400
#define SIM_CLONE_SIGHAND		0x00000800
#define SIM_CLONE_PTRACE		0x00002000
#define SIM_CLONE_VFORK			0x00004000
#define SIM_CLONE_PARENT		0x00008000
#define SIM_CLONE_THREAD		0x00010000
#define SIM_CLONE_NEWNS			0x00020000
#define SIM_CLONE_SYSVSEM		0x00040000
#define SIM_CLONE_SETTLS		0x00080000
#define SIM_CLONE_PARENT_SETTID		0x00100000
#define SIM_CLONE_CHILD_CLEARTID	0x00200000
#define SIM_CLONE_DETACHED		0x00400000
#define SIM_CLONE_UNTRACED		0x00800000
#define SIM_CLONE_CHILD_SETTID		0x01000000
#define SIM_CLONE_STOPPED		0x02000000
#define SIM_CLONE_NEWUTS		0x04000000
#define SIM_CLONE_NEWIPC		0x08000000
#define SIM_CLONE_NEWUSER		0x10000000
#define SIM_CLONE_NEWPID		0x20000000
#define SIM_CLONE_NEWNET		0x40000000
#define SIM_CLONE_IO			0x80000000

static struct string_map_t sys_clone_flags_map =
{
	23, {
		{ "CLONE_VM", 0x00000100 },
		{ "CLONE_FS", 0x00000200 },
		{ "CLONE_FILES", 0x00000400 },
		{ "CLONE_SIGHAND", 0x00000800 },
		{ "CLONE_PTRACE", 0x00002000 },
		{ "CLONE_VFORK", 0x00004000 },
		{ "CLONE_PARENT", 0x00008000 },
		{ "CLONE_THREAD", 0x00010000 },
		{ "CLONE_NEWNS", 0x00020000 },
		{ "CLONE_SYSVSEM", 0x00040000 },
		{ "CLONE_SETTLS", 0x00080000 },
		{ "CLONE_PARENT_SETTID", 0x00100000 },
		{ "CLONE_CHILD_CLEARTID", 0x00200000 },
		{ "CLONE_DETACHED", 0x00400000 },
		{ "CLONE_UNTRACED", 0x00800000 },
		{ "CLONE_CHILD_SETTID", 0x01000000 },
		{ "CLONE_STOPPED", 0x02000000 },
		{ "CLONE_NEWUTS", 0x04000000 },
		{ "CLONE_NEWIPC", 0x08000000 },
		{ "CLONE_NEWUSER", 0x10000000 },
		{ "CLONE_NEWPID", 0x20000000 },
		{ "CLONE_NEWNET", 0x40000000 },
		{ "CLONE_IO", 0x80000000 }
	}
};

static const unsigned int sys_clone_supported_flags =
	SIM_CLONE_VM |
	SIM_CLONE_FS |
	SIM_CLONE_FILES |
	SIM_CLONE_SIGHAND |
	SIM_CLONE_THREAD |
	SIM_CLONE_SYSVSEM |
	SIM_CLONE_SETTLS |
	SIM_CLONE_PARENT_SETTID |
	SIM_CLONE_CHILD_CLEARTID |
	SIM_CLONE_CHILD_SETTID;

struct sim_user_desc
{
	unsigned int entry_number;
	unsigned int base_addr;
	unsigned int limit;
	unsigned int seg_32bit:1;
	unsigned int contents:2;
	unsigned int read_exec_only:1;
	unsigned int limit_in_pages:1;
	unsigned int seg_not_present:1;
	unsigned int useable:1;
};

int sys_clone_impl(void)
{
	/* Prototype: long sys_clone(unsigned long clone_flags, unsigned long newsp,
	 * 	int __user *parent_tid, int unused, int __user *child_tid);
	 * There is an unused parameter, that's why we read child_tidptr from edi
	 * instead of esi. */

	unsigned int flags;
	unsigned int new_esp;
	unsigned int parent_tid_ptr;
	unsigned int child_tid_ptr;

	int exit_signal;

	char flags_str[MAX_STRING_SIZE];

	struct ctx_t *new_ctx;

	/* Arguments */
	flags = isa_regs->ebx;
	new_esp = isa_regs->ecx;
	parent_tid_ptr = isa_regs->edx;
	child_tid_ptr = isa_regs->edi;
	sys_debug("  flags=0x%x, newsp=0x%x, parent_tidptr=0x%x, child_tidptr=0x%x\n",
		flags, new_esp, parent_tid_ptr, child_tid_ptr);

	/* Exit signal is specified in the lower byte of 'flags' */
	exit_signal = flags & 0xff;
	flags &= ~0xff;

	/* Debug */
	map_flags(&sys_clone_flags_map, flags, flags_str, MAX_STRING_SIZE);
	sys_debug("  flags=%s\n", flags_str);
	sys_debug("  exit_signal=%d (%s)\n", exit_signal, sim_signal_name(exit_signal));

	/* New stack pointer defaults to current */
	if (!new_esp)
		new_esp = isa_regs->esp;

	/* Check not supported flags */
	if (flags & ~sys_clone_supported_flags)
	{
		map_flags(&sys_clone_flags_map, flags & ~sys_clone_supported_flags,
			flags_str, MAX_STRING_SIZE);
		fatal("%s: not supported flags: %s\n%s",
			__FUNCTION__, flags_str, err_sys_note);
	}

	/* Flag CLONE_VM */
	if (flags & SIM_CLONE_VM)
	{
		/* CLONE_FS, CLONE_FILES, CLONE_SIGHAND must be there, too */
		if ((flags & (SIM_CLONE_FS | SIM_CLONE_FILES | SIM_CLONE_SIGHAND)) !=
			(SIM_CLONE_FS | SIM_CLONE_FILES | SIM_CLONE_SIGHAND))
			fatal("%s: not supported flags with CLONE_VM.\n%s",
				__FUNCTION__, err_sys_note);

		/* Create new context sharing memory image */
		new_ctx = ctx_clone(isa_ctx);
	}
	else
	{
		/* CLONE_FS, CLONE_FILES, CLONE_SIGHAND must not be there either */
		if (flags & (SIM_CLONE_FS | SIM_CLONE_FILES | SIM_CLONE_SIGHAND))
			fatal("%s: not supported flags with CLONE_VM.\n%s",
				__FUNCTION__, err_sys_note);

		/* Create new context replicating memory image */
		new_ctx = ctx_fork(isa_ctx);
	}

	/* Flag CLONE_THREAD.
	 * If specified, the exit signal is ignored. Otherwise, it is specified in the
	 * lower byte of the flags. Also, this determines whether to create a group of
	 * threads. */
	if (flags & SIM_CLONE_THREAD)
	{
		new_ctx->exit_signal = 0;
		new_ctx->group_parent = isa_ctx->group_parent ?
			isa_ctx->group_parent : isa_ctx;
	}
	else
	{
		new_ctx->exit_signal = exit_signal;
		new_ctx->group_parent = NULL;
	}

	/* Flag CLONE_PARENT_SETTID */
	if (flags & SIM_CLONE_PARENT_SETTID)
		mem_write(isa_ctx->mem, parent_tid_ptr, 4, &new_ctx->pid);

	/* Flag CLONE_CHILD_SETTID */
	if (flags & SIM_CLONE_CHILD_SETTID)
		mem_write(new_ctx->mem, child_tid_ptr, 4, &new_ctx->pid);

	/* Flag CLONE_CHILD_CLEARTID */
	if (flags & SIM_CLONE_CHILD_CLEARTID)
		new_ctx->clear_child_tid = child_tid_ptr;

	/* Flag CLONE_SETTLS */
	if (flags & SIM_CLONE_SETTLS)
	{
		struct sim_user_desc uinfo;
		unsigned int uinfo_ptr;

		uinfo_ptr = isa_regs->esi;
		sys_debug("  puinfo=0x%x\n", uinfo_ptr);

		mem_read(isa_mem, uinfo_ptr, sizeof(struct sim_user_desc), &uinfo);
		sys_debug("  entry_number=0x%x, base_addr=0x%x, limit=0x%x\n",
				uinfo.entry_number, uinfo.base_addr, uinfo.limit);
		sys_debug("  seg_32bit=0x%x, contents=0x%x, read_exec_only=0x%x\n",
				uinfo.seg_32bit, uinfo.contents, uinfo.read_exec_only);
		sys_debug("  limit_in_pages=0x%x, seg_not_present=0x%x, useable=0x%x\n",
				uinfo.limit_in_pages, uinfo.seg_not_present, uinfo.useable);
		if (!uinfo.seg_32bit)
			fatal("%s: only 32-bit segments supported", __FUNCTION__);

		/* Limit given in pages (4KB units) */
		if (uinfo.limit_in_pages)
			uinfo.limit <<= 12;

		uinfo.entry_number = 6;
		mem_write(isa_mem, uinfo_ptr, 4, &uinfo.entry_number);

		new_ctx->glibc_segment_base = uinfo.base_addr;
		new_ctx->glibc_segment_limit = uinfo.limit;
	}

	/* New context returns 0. */
	new_ctx->regs->esp = new_esp;
	new_ctx->regs->eax = 0;

	/* Return PID of the new context */
	sys_debug("  context created with pid %d\n", new_ctx->pid);
	return new_ctx->pid;
}




/*
 * System call 'newuname' (code 122)
 */

struct sim_utsname
{
	char sysname[65];
	char nodename[65];
	char release[65];
	char version[65];
	char machine[65];
	char domainname[65];
} __attribute__((packed));

static struct sim_utsname sim_utsname =
{
	"Linux",
	"Multi2Sim",
	"3.1.9-1.fc16.i686"
	"#1 Fri Jan 13 16:37:42 UTC 2012",
	"i686"
	""
};

int sys_newuname_impl(void)
{
	unsigned int utsname_ptr;

	/* Arguments */
	utsname_ptr = isa_regs->ebx;
	sys_debug("  putsname=0x%x\n", utsname_ptr);
	sys_debug("  sysname='%s', nodename='%s'\n", sim_utsname.sysname, sim_utsname.nodename);
	sys_debug("  relaese='%s', version='%s'\n", sim_utsname.release, sim_utsname.version);
	sys_debug("  machine='%s', domainname='%s'\n", sim_utsname.machine, sim_utsname.domainname);

	/* Return structure */
	mem_write(isa_mem, utsname_ptr, sizeof sim_utsname, &sim_utsname);
	return 0;
}




/*
 * System call 'mprotect' (code 125)
 */

int sys_mprotect_impl(void)
{
	unsigned int start;
	unsigned int len;

	int prot;

	enum mem_access_t perm = 0;

	/* Arguments */
	start = isa_regs->ebx;
	len = isa_regs->ecx;
	prot = isa_regs->edx;
	sys_debug("  start=0x%x, len=0x%x, prot=0x%x\n", start, len, prot);

	/* Permissions */
	perm |= prot & 0x01 ? mem_access_read : 0;
	perm |= prot & 0x02 ? mem_access_write : 0;
	perm |= prot & 0x04 ? mem_access_exec : 0;
	mem_protect(isa_mem, start, len, perm);

	/* Return */
	return 0;
}




/*
 * System call 'llseek' (code 140)
 */

int sys_llseek_impl(void)
{
	unsigned int fd;
	unsigned int result_ptr;

	int origin;
	int host_fd;
	int offset_high;
	int offset_low;

	long long offset;

	/* Arguments */
	fd = isa_regs->ebx;
	offset_high = isa_regs->ecx;
	offset_low = isa_regs->edx;
	offset = ((long long) offset_high << 32) | offset_low;
	result_ptr = isa_regs->esi;
	origin = isa_regs->edi;
	host_fd = file_desc_table_get_host_fd(isa_ctx->file_desc_table, fd);
	sys_debug("  fd=%d, offset_high=0x%x, offset_low=0x%x, result_ptr=0x%x, origin=0x%x\n",
		fd, offset_high, offset_low, result_ptr, origin);
	sys_debug("  host_fd=%d\n", host_fd);
	sys_debug("  offset=0x%llx\n", (long long) offset);

	/* Supported offset */
	if (offset_high != -1 && offset_high)
		fatal("%s: only supported for 32-bit files", __FUNCTION__);

	/* Host call */
	offset = lseek(host_fd, offset_low, origin);
	if (offset == -1)
		return -errno;

	/* Copy offset to memory */
	if (result_ptr)
		mem_write(isa_mem, result_ptr, 8, &offset);

	/* Return */
	return 0;
}




/*
 * System call 'getdents' (code 141)
 */

struct sys_host_dirent_t
{
	long d_ino;
	off_t d_off;
	unsigned short d_reclen;
	char d_name[];
};

struct sys_guest_dirent_t
{
	unsigned int d_ino;
	unsigned int d_off;
	unsigned short d_reclen;
	char d_name[];
} __attribute__((packed));

int sys_getdents_impl(void)
{
	unsigned int pdirent;

	int fd;
	int count;
	int host_fd;

	void *buf;

	int nread;
	int host_offs;
	int guest_offs;

	char d_type;

	struct sys_host_dirent_t *dirent;
	struct sys_guest_dirent_t sim_dirent;

	/* Read parameters */
	fd = isa_regs->ebx;
	pdirent = isa_regs->ecx;
	count = isa_regs->edx;
	host_fd = file_desc_table_get_host_fd(isa_ctx->file_desc_table, fd);
	sys_debug("  fd=%d, pdirent=0x%x, count=%d\n",
		fd, pdirent, count);
	sys_debug("  host_fd=%d\n", host_fd);

	/* Allocate buffer */
	buf = calloc(1, count);
	if (!buf)
		fatal("%s: out of memory", __FUNCTION__);

	/* Call host getdents */
	nread = syscall(SYS_getdents, host_fd, buf, count);
	if (nread == -1)
		fatal("%s: host call failed", __FUNCTION__);

	/* No more entries */
	if (!nread)
		return 0;

	/* Copy to host memory */
	host_offs = 0;
	guest_offs = 0;
	while (host_offs < nread)
	{
		dirent = (struct sys_host_dirent_t *) (buf + host_offs);
		sim_dirent.d_ino = dirent->d_ino;
		sim_dirent.d_off = dirent->d_off;
		sim_dirent.d_reclen = (15 + strlen(dirent->d_name)) / 4 * 4;
		d_type = * (char *) (buf + host_offs + dirent->d_reclen - 1);

		sys_debug("    d_ino=%u ", sim_dirent.d_ino);
		sys_debug("d_off=%u ", sim_dirent.d_off);
		sys_debug("d_reclen=%u(host),%u(guest) ", dirent->d_reclen, sim_dirent.d_reclen);
		sys_debug("d_name='%s'\n", dirent->d_name);

		mem_write(isa_mem, pdirent + guest_offs, 4, &sim_dirent.d_ino);
		mem_write(isa_mem, pdirent + guest_offs + 4, 4, &sim_dirent.d_off);
		mem_write(isa_mem, pdirent + guest_offs + 8, 2, &sim_dirent.d_reclen);
		mem_write_string(isa_mem, pdirent + guest_offs + 10, dirent->d_name);
		mem_write(isa_mem, pdirent + guest_offs + sim_dirent.d_reclen - 1, 1, &d_type);

		host_offs += dirent->d_reclen;
		guest_offs += sim_dirent.d_reclen;
		if (guest_offs > count)
			fatal("%s: buffer too small", __FUNCTION__);
	}
	sys_debug("  ret=%d(host),%d(guest)\n", host_offs, guest_offs);
	free(buf);
	return guest_offs;
}




/*
 * System call 'select' (code 142)
 */

/* Dump host 'fd_set' structure */
static void sim_fd_set_dump(char *fd_set_name, fd_set *fds, int n)
{
	int i;

	char *comma;

	/* Set empty */
	if (!n || !fds)
	{
		sys_debug("    %s={}\n", fd_set_name);
		return;
	}

	/* Dump set */
	sys_debug("    %s={", fd_set_name);
	comma = "";
	for (i = 0; i < n; i++)
	{
		if (!FD_ISSET(i, fds))
			continue;
		sys_debug("%s%d", comma, i);
		comma = ",";
	}
	sys_debug("}\n");
}

/* Read bitmap of 'guest_fd's from guest memory, and store it into
 * a bitmap of 'host_fd's into host memory. */
static int sim_fd_set_read(uint32_t addr, fd_set *fds, int n)
{
	int nbyte;
	int nbit;
	int host_fd;
	int i;

	unsigned char c;

	FD_ZERO(fds);
	for (i = 0; i < n; i++)
	{
		/* Check if fd is set */
		nbyte = i >> 3;
		nbit = i & 7;
		mem_read(isa_mem, addr + nbyte, 1, &c);
		if (!(c & (1 << nbit)))
			continue;

		/* Obtain 'host_fd' */
		host_fd = file_desc_table_get_host_fd(isa_ctx->file_desc_table, i);
		if (host_fd < 0)
			return 0;
		FD_SET(host_fd, fds);
	}
	return 1;
}

/* Read bitmap of 'host_fd's from host memory, and store it into
 * a bitmap of 'guest_fd's into guest memory. */
static void sim_fd_set_write(unsigned int addr, fd_set *fds, int n)
{
	int nbyte;
	int nbit;
	int guest_fd;
	int i;

	unsigned char c;

	/* No valid address given */
	if (!addr)
		return;

	/* Write */
	mem_zero(isa_mem, addr, (n + 7) / 8);
	for (i = 0; i < n; i++)
	{
		/* Check if fd is set */
		if (!FD_ISSET(i, fds))
			continue;

		/* Obtain 'guest_fd' and write */
		guest_fd = file_desc_table_get_guest_fd(isa_ctx->file_desc_table, i);
		assert(guest_fd >= 0);
		nbyte = guest_fd >> 3;
		nbit = guest_fd & 7;
		mem_read(isa_mem, addr + nbyte, 1, &c);
		c |= 1 << nbit;
		mem_write(isa_mem, addr + nbyte, 1, &c);
	}
}

int sys_select_impl(void)
{
	/* System call prototype:
	 * int select(int n, fd_set *inp, fd_set *outp, fd_set *exp, struct timeval *tvp);
	 */

	unsigned int n;
	unsigned int inp;
	unsigned int outp;
	unsigned int exp;
	unsigned int tvp;

	fd_set in;
	fd_set out;
	fd_set ex;

	struct sim_timeval sim_tv;
	struct timeval tv;

	int err;

	/* Arguments */
	n = isa_regs->ebx;
	inp = isa_regs->ecx;
	outp = isa_regs->edx;
	exp = isa_regs->esi;
	tvp = isa_regs->edi;
	sys_debug("  n=%d, inp=0x%x, outp=0x%x, exp=0x%x, tvp=0x%x\n",
		n, inp, outp, exp, tvp);

	/* Read file descriptor bitmaps. If any file descriptor is invalid, return EBADF. */
	if (!sim_fd_set_read(inp, &in, n)
		|| !sim_fd_set_read(outp, &out, n)
		|| !sim_fd_set_read(exp, &ex, n))
	{
		return -EBADF;
	}

	/* Dump file descriptors */
	sim_fd_set_dump("inp", &in, n);
	sim_fd_set_dump("outp", &out, n);
	sim_fd_set_dump("exp", &ex, n);

	/* Read and dump 'sim_tv' */
	memset(&sim_tv, 0, sizeof sim_tv);
	if (tvp)
		mem_read(isa_mem, tvp, sizeof sim_tv, &sim_tv);
	sys_debug("  tv:\n");
	sim_timeval_dump(&sim_tv);

	/* Blocking 'select' not supported */
	if (sim_tv.tv_sec || sim_tv.tv_usec)
		fatal("%s: not supported for 'tv' other than 0", __FUNCTION__);

	/* Host system call */
	memset(&tv, 0, sizeof(tv));
	err = select(n, &in, &out, &ex, &tv);
	if (err == -1)
		return -errno;

	/* Write result */
	sim_fd_set_write(inp, &in, n);
	sim_fd_set_write(outp, &out, n);
	sim_fd_set_write(exp, &ex, n);

	/* Return */
	return err;
}




/*
 * System call 'msync' (code 144)
 */

static struct string_map_t sys_msync_flags_map =
{
	3, {
		{ "MS_ASYNC", 1 },
		{ "MS_INAVLIAGE", 2 },
		{ "MS_SYNC", 4 }
	}
};

int sys_msync_impl(void)
{
	unsigned int start;
	unsigned int len;

	int flags;

	char flags_str[MAX_STRING_SIZE];

	/* Arguments */
	start = isa_regs->ebx;
	len = isa_regs->ecx;
	flags = isa_regs->edx;
	map_flags(&sys_msync_flags_map, flags, flags_str, sizeof flags_str);
	sys_debug("  start=0x%x, len=0x%x, flags=0x%x\n", start, len, flags);
	sys_debug("  flags=%s\n", flags_str);

	/* System call ignored */
	return 0;
}




/*
 * System call 'mremap' (code 163)
 */

int sys_mremap_impl(void)
{
	unsigned int addr;
	unsigned int old_len;
	unsigned int new_len;
	unsigned int new_addr;

	int flags;

	/* Arguments */
	addr = isa_regs->ebx;
	old_len = isa_regs->ecx;
	new_len = isa_regs->edx;
	flags = isa_regs->esi;
	sys_debug("  addr=0x%x, old_len=0x%x, new_len=0x%x flags=0x%x\n",
		addr, old_len, new_len, flags);

	/* Restrictions */
	assert(!(addr & (MEM_PAGE_SIZE-1)));
	assert(!(old_len & (MEM_PAGE_SIZE-1)));
	assert(!(new_len & (MEM_PAGE_SIZE-1)));
	if (!(flags & 0x1))
		fatal("%s: flags MAP_MAYMOVE must be present", __FUNCTION__);
	if (!old_len || !new_len)
		fatal("%s: old_len or new_len cannot be zero", __FUNCTION__);

	/* New size equals to old size means no action. */
	if (new_len == old_len)
		return addr;

	/* Shrink region. This is always possible. */
	if (new_len < old_len)
	{
		mem_unmap(isa_mem, addr + new_len, old_len - new_len);
		return addr;
	}

	/* Increase region at the same address. This is only possible if
	 * there is enough free space for the new region. */
	if (new_len > old_len && mem_map_space(isa_mem, addr + old_len,
		new_len - old_len) == addr + old_len)
	{
		mem_map(isa_mem, addr + old_len, new_len - old_len,
			mem_access_read | mem_access_write);
		return addr;
	}

	/* A new region must be found for the new size. */
	new_addr = mem_map_space_down(isa_mem, SYS_MMAP_BASE_ADDRESS, new_len);
	if (new_addr == -1)
		fatal("%s: out of guest memory", __FUNCTION__);

	/* Map new region and copy old one */
	mem_map(isa_mem, new_addr, new_len,
		mem_access_read | mem_access_write);
	mem_copy(isa_mem, new_addr, addr, MIN(old_len, new_len));
	mem_unmap(isa_mem, addr, old_len);

	/* Return new address */
	return new_addr;
}




/*
 * System call 'getrlimit' (code 191)
 */

int sys_getrlimit_impl(void)
{
	unsigned int res;
	unsigned int rlim_ptr;

	char *res_str;

	struct sim_rlimit sim_rlimit;

	/* Arguments */
	res = isa_regs->ebx;
	rlim_ptr = isa_regs->ecx;
	res_str = map_value(&sys_rlimit_res_map, res);
	sys_debug("  res=0x%x, rlim_ptr=0x%x\n", res, rlim_ptr);
	sys_debug("  res=%s\n", res_str);

	switch (res)
	{

	case 2:  /* RLIMIT_DATA */
	{
		sim_rlimit.cur = 0xffffffff;
		sim_rlimit.max = 0xffffffff;
		break;
	}

	case 3:  /* RLIMIT_STACK */
	{
		sim_rlimit.cur = isa_ctx->loader->stack_size;
		sim_rlimit.max = 0xffffffff;
		break;
	}

	case 7:  /* RLIMIT_NOFILE */
	{
		sim_rlimit.cur = 0x400;
		sim_rlimit.max = 0x400;
		break;
	}

	default:
		fatal("%s: not implemented for res = %s.\n%s",
			__FUNCTION__, res_str, err_sys_note);
	}

	/* Return structure */
	mem_write(isa_mem, rlim_ptr, sizeof(struct sim_rlimit), &sim_rlimit);
	sys_debug("  ret: cur=0x%x, max=0x%x\n", sim_rlimit.cur, sim_rlimit.max);

	/* Return */
	return 0;
}




/*
 * System call 'mmap2' (code 192)
 */

int sys_mmap2_impl(void)
{
	unsigned int addr;
	unsigned int len;

	int prot;
	int flags;
	int offset;
	int guest_fd;

	char prot_str[MAX_STRING_SIZE];
	char flags_str[MAX_STRING_SIZE];

	/* Arguments */
	addr = isa_regs->ebx;
	len = isa_regs->ecx;
	prot = isa_regs->edx;
	flags = isa_regs->esi;
	guest_fd = isa_regs->edi;
	offset = isa_regs->ebp;

	/* Debug */
	sys_debug("  addr=0x%x, len=%u, prot=0x%x, flags=0x%x, guest_fd=%d, offset=0x%x\n",
		addr, len, prot, flags, guest_fd, offset);
	map_flags(&sys_mmap_prot_map, prot, prot_str, MAX_STRING_SIZE);
	map_flags(&sys_mmap_flags_map, flags, flags_str, MAX_STRING_SIZE);
	sys_debug("  prot=%s, flags=%s\n", prot_str, flags_str);

	/* System calls 'mmap' and 'mmap2' only differ in the interpretation of
	 * argument 'offset'. Here, it is given in memory pages. */
	return sys_mmap(addr, len, prot, flags, guest_fd, offset << MEM_PAGE_SHIFT);
}




/*
 * System call 'fcntl64' (code 221)
 */

static struct string_map_t sys_fcntl_cmp_map =
{
	15, {
		{ "F_DUPFD", 0 },
		{ "F_GETFD", 1 },
		{ "F_SETFD", 2 },
		{ "F_GETFL", 3 },
		{ "F_SETFL", 4 },
		{ "F_GETLK", 5 },
		{ "F_SETLK", 6 },
		{ "F_SETLKW", 7 },
		{ "F_SETOWN", 8 },
		{ "F_GETOWN", 9 },
		{ "F_SETSIG", 10 },
		{ "F_GETSIG", 11 },
		{ "F_GETLK64", 12 },
		{ "F_SETLK64", 13 },
		{ "F_SETLKW64", 14 }
	}
};

int sys_fcntl64_impl(void)
{
	int guest_fd;
	int cmd;
	int err;

	unsigned int arg;

	char *cmd_name;
	char flags_str[MAX_STRING_SIZE];

	struct file_desc_t *desc;

	/* Arguments */
	guest_fd = isa_regs->ebx;
	cmd = isa_regs->ecx;
	arg = isa_regs->edx;
	sys_debug("  guest_fd=%d, cmd=%d, arg=0x%x\n",
		guest_fd, cmd, arg);
	cmd_name = map_value(&sys_fcntl_cmp_map, cmd);
	sys_debug("    cmd=%s\n", cmd_name);

	/* Get file descriptor table entry */
	desc = file_desc_table_entry_get(isa_ctx->file_desc_table, guest_fd);
	if (!desc)
		return -EBADF;
	if (desc->host_fd < 0)
		fatal("%s: not supported for this type of file", __FUNCTION__);
	sys_debug("    host_fd=%d\n", desc->host_fd);

	/* Process command */
	switch (cmd)
	{

	/* F_GETFD */
	case 1:
		err = fcntl(desc->host_fd, F_GETFD);
		if (err == -1)
			err = -errno;
		break;

	/* F_SETFD */
	case 2:
		err = fcntl(desc->host_fd, F_SETFD, arg);
		if (err == -1)
			err = -errno;
		break;

	/* F_GETFL */
	case 3:
		err = fcntl(desc->host_fd, F_GETFL);
		if (err == -1)
			err = -errno;
		else
		{
			map_flags(&sys_open_flags_map, err, flags_str, MAX_STRING_SIZE);
			sys_debug("    ret=%s\n", flags_str);
		}
		break;

	/* F_SETFL */
	case 4:
		map_flags(&sys_open_flags_map, arg, flags_str, MAX_STRING_SIZE);
		sys_debug("    arg=%s\n", flags_str);
		desc->flags = arg;

		err = fcntl(desc->host_fd, F_SETFL, arg);
		if (err == -1)
			err = -errno;
		break;

	default:

		err = 0;
		fatal("%s: command %s not implemented.\n%s",
			__FUNCTION__, cmd_name, err_sys_note);
	}

	/* Return */
	return err;
}




/*
 * System call 'set_thread_area' (code 243)
 */

int sys_set_thread_area_impl(void)
{
	unsigned int uinfo_ptr;

	struct sim_user_desc uinfo;

	/* Arguments */
	uinfo_ptr = isa_regs->ebx;
	sys_debug("  uinfo_ptr=0x%x\n", uinfo_ptr);

	/* Read structure */
	mem_read(isa_mem, uinfo_ptr, sizeof uinfo, &uinfo);
	sys_debug("  entry_number=0x%x, base_addr=0x%x, limit=0x%x\n",
		uinfo.entry_number, uinfo.base_addr, uinfo.limit);
	sys_debug("  seg_32bit=0x%x, contents=0x%x, read_exec_only=0x%x\n",
		uinfo.seg_32bit, uinfo.contents, uinfo.read_exec_only);
	sys_debug("  limit_in_pages=0x%x, seg_not_present=0x%x, useable=0x%x\n",
		uinfo.limit_in_pages, uinfo.seg_not_present, uinfo.useable);
	if (!uinfo.seg_32bit)
		fatal("syscall set_thread_area: only 32-bit segments supported");

	/* Limit given in pages (4KB units) */
	if (uinfo.limit_in_pages)
		uinfo.limit <<= 12;

	if (uinfo.entry_number == -1)
	{
		if (isa_ctx->glibc_segment_base)
			fatal("%s: glibc segment already set", __FUNCTION__);

		isa_ctx->glibc_segment_base = uinfo.base_addr;
		isa_ctx->glibc_segment_limit = uinfo.limit;
		uinfo.entry_number = 6;
		mem_write(isa_mem, uinfo_ptr, 4, &uinfo.entry_number);
	}
	else
	{
		if (uinfo.entry_number != 6)
			fatal("%s: invalid entry number", __FUNCTION__);
		if (!isa_ctx->glibc_segment_base)
			fatal("%s: glibc segment not set", __FUNCTION__);
		isa_ctx->glibc_segment_base = uinfo.base_addr;
		isa_ctx->glibc_segment_limit = uinfo.limit;
	}

	/* Return */
	return 0;
}
