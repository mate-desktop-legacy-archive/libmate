/* mate-exec.c - Execute some command.

   Copyright (C) 1998 Tom Tromey
   All rights reserved.

   The Mate Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Mate Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Mate Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */
/*
  @NOTATION@
 */

#include <config.h>

#include <glib/gi18n-lib.h>

#include "mate-exec.h"
#include "mate-util.h"
#include "mate-mateconfP.h"
#include "mate-init.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>

#ifndef G_OS_WIN32
	#include <sys/wait.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <mateconf/mateconf-client.h>

#include <errno.h>

#ifndef errno
	extern int errno;
#endif

#ifndef G_OS_WIN32

static void set_cloexec(gint fd)
{
	fcntl(fd, F_SETFD, FD_CLOEXEC);
}

static ssize_t safe_read(int fd, void* buf, size_t count)
{
	ssize_t n;

	while ((n = read(fd, buf, count)) < 0 && (errno == EINTR || errno == EAGAIN));

	return n;
}

#endif

/**
 * mate_execute_async_with_env_fds:
 * @dir: Directory in which child should be executed, or %NULL for current
 *       directory
 * @argc: Number of arguments
 * @argv: Argument vector to exec child
 * @envc: Number of environment slots
 * @envv: Environment vector
 * @close_fds: If %TRUE will close all fds but 0,1, and 2
 *
 * Description:  Like mate_execute_async_with_env() but has a flag to
 * decide whether or not to close fd's
 *
 * Returns: the process id, or %-1 on error.
 **/
int mate_execute_async_with_env_fds (const char* dir, int argc, char* const argv[], int envc, char* const envv[], gboolean close_fds)
{
	#ifndef G_OS_WIN32
		int parent_comm_pipes[2], child_comm_pipes[2];
		int child_errno, itmp, i, open_max;
		gssize res;
		char** cpargv;
		pid_t child_pid, immediate_child_pid;

		if(pipe(parent_comm_pipes))
			return -1;

		child_pid = immediate_child_pid = fork();

		switch (child_pid)
		{
			case -1:
				close(parent_comm_pipes[0]);
				close(parent_comm_pipes[1]);
				return -1;

			case 0: /* START PROCESS 1: child */
				child_pid = -1;
				res = pipe(child_comm_pipes);
				close(parent_comm_pipes[0]);

				if (!res)
					child_pid = fork();

				switch (child_pid)
				{
					case -1:
						itmp = errno;
						child_pid = -1; /* simplify parent code */
						write(parent_comm_pipes[1], &child_pid, sizeof(child_pid));
						write(parent_comm_pipes[1], &itmp, sizeof(itmp));
						close(child_comm_pipes[0]);
						close(child_comm_pipes[1]);
						_exit(0);
						break;      /* END PROCESS 1: monkey in the middle dies */

					default:
						{
							char buf[16];

							close(child_comm_pipes[1]);

							while ((res = safe_read(child_comm_pipes[0], buf, sizeof(buf))) > 0)
								write(parent_comm_pipes[1], buf, res);

							close(child_comm_pipes[0]);
							_exit(0); /* END PROCESS 1: monkey in the middle dies */
						}
						break;

					case 0:                 /* START PROCESS 2: child of child */
						close(parent_comm_pipes[1]);
						/* pre-exec setup */
						close (child_comm_pipes[0]);
						set_cloexec (child_comm_pipes[1]);
						child_pid = getpid();
						res = write(child_comm_pipes[1], &child_pid, sizeof(child_pid));

						if (envv)
						{
							for (itmp = 0; itmp < envc; itmp++)
								putenv(envv[itmp]);
						}

						if (dir)
						{
							if (chdir(dir))
								_exit(-1);
						}

						cpargv = g_alloca((argc + 1) * sizeof(char *));
						memcpy(cpargv, argv, argc * sizeof(char *));
						cpargv[argc] = NULL;

						if(close_fds)
						{
							int stdinfd;
							/* Close all file descriptors but stdin stdout and stderr */
							open_max = sysconf(_SC_OPEN_MAX);

							for (i = 3; i < open_max; i++)
								set_cloexec (i);

							if(child_comm_pipes[1] != 0)
							{
								close(0);
								/* Open stdin as being nothingness, so that if someone tries to
								read from this they don't hang up the whole MATE session. BUGFIX #1548 */
								stdinfd = open("/dev/null", O_RDONLY);
								g_assert(stdinfd >= 0);

								if (stdinfd != 0)
								{
									dup2(stdinfd, 0);
									close(stdinfd);
								}
							}
						}

						setsid();
						signal(SIGPIPE, SIG_DFL);
						/* doit */
						execvp(cpargv[0], cpargv);

						/* failed */
						itmp = errno;
						write(child_comm_pipes[1], &itmp, sizeof(itmp));
						_exit(1);
						break;      /* END PROCESS 2 */
				}

				break;

			default: /* parent process */
				/* do nothing */
				break;
		}

		close(parent_comm_pipes[1]);

		res = safe_read(parent_comm_pipes[0], &child_pid, sizeof(child_pid));

		if (res != sizeof(child_pid))
		{
			g_message("res is %ld instead of %d", (long) res, (int) sizeof(child_pid));
			child_pid = -1; /* really weird things happened */
		}
		else if (safe_read(parent_comm_pipes[0], &child_errno, sizeof(child_errno)) == sizeof(child_errno))
		{
			errno = child_errno;
			child_pid = -1;
		}

		/* do this after the read's in case some OS's handle blocking on pipe writes
		 * differently
		 */
		while ((waitpid(immediate_child_pid, &itmp, 0)== -1) && (errno == EINTR))
			; /* eat zombies */

		close(parent_comm_pipes[0]);

		if(child_pid < 0)
			g_message("mate_execute_async_with_env_fds: returning %d", child_pid);

		return child_pid;
	#else /* !G_OS_WIN32 */
		/* FIXME: Implement if needed */
		g_warning("mate_execute_async_with_env_fds: Not implemented");

		return -1;
	#endif /* G_OS_WIN32 */
}

/**
 * mate_execute_async_with_env:
 * @dir: Directory in which child should be executed, or NULL for current
 *       directory
 * @argc: Number of arguments
 * @argv: Argument vector to exec child
 * @envc: Number of environment slots
 * @envv: Environment vector
 *
 * Description: This function forks and executes some program in the
 * background.  On error, returns %-1; in this case, #errno should hold a useful
 * value.  Searches the path to find the child.  Environment settings in @envv
 * are added to the existing environment -- they do not completely replace it.
 * This function closes all fds besides 0, 1, and 2 for the child
 *
 * Returns: the process id, or %-1 on error.
 **/
int mate_execute_async_with_env(const char* dir, int argc, char* const argv[], int envc, char* const envv[])
{
	return mate_execute_async_with_env_fds (dir, argc, argv, envc, envv, TRUE);
}


/**
 * mate_execute_async:
 * @dir: Directory in which child should be executesd, or %NULL for current
 *       directory
 * @argc: Number of arguments
 * @argv: Argument vector to exec child
 *
 * Description: Like mate_execute_async_with_env(), but doesn't add anything
 * to child's environment.
 *
 * Returns: process id of child, or %-1 on error.
 **/
int mate_execute_async(const char* dir, int argc, char* const argv[])
{
	return mate_execute_async_with_env(dir, argc, argv, 0, NULL);
}

/**
 * mate_execute_async_fds:
 * @dir: Directory in which child should be executed, or %NULL for current
 *       directory
 * @argc: Number of arguments
 * @argv: Argument vector to exec child
 * @close_fds: If %TRUE, will close all but file descriptors 0, 1 and 2.
 *
 * Description: Like mate_execute_async_with_env_fds(), but doesn't add
 * anything to child's environment.
 *
 * Returns: process id of child, or %-1 on error.
 **/
int mate_execute_async_fds (const char* dir, int argc, char* const argv[], gboolean close_fds)
{
	return mate_execute_async_with_env_fds(dir, argc, argv, 0, NULL, close_fds);
}

/**
 * mate_execute_shell_fds:
 * @dir: Directory in which child should be executed, or %NULL for current
 *       directory
 * @commandline: Shell command to execute
 * @close_fds: Like close_fds in mate_execute_async_with_env_fds()
 *
 * Description: Like mate_execute_async_with_env_fds(), but uses the user's
 * shell to run the desired program.  Note that the pid of the shell is
 * returned, not the pid of the user's program.
 *
 * Returns: process id of shell, or %-1 on error.
 **/
int mate_execute_shell_fds(const char* dir, const char* commandline, gboolean close_fds)
{
	char* user_shell;
	char* argv[4];
	int r;

	g_return_val_if_fail(commandline != NULL, -1);

	user_shell = mate_util_user_shell();

	argv[0] = user_shell;
	argv[1] = "-c";
	/* necessary cast, to avoid warning, but safe */
	argv[2] = (char*) commandline;
	argv[3] = NULL;

	r = mate_execute_async_with_env_fds(dir, 4, argv, 0, NULL, close_fds);

	g_free(user_shell);
	return r;
}

/**
 * mate_execute_shell:
 * @dir: Directory in which child should be executed, or %NULL for current
 *       directory
 * @commandline: Shell command to execute
 *
 * Description: Like mate_execute_async_with_env(), but uses the user's shell
 * to run the desired program.  Note that the pid of the shell is returned, not
 * the pid of the user's program.
 *
 * Returns: process id of shell, or %-1 on error.
 **/
int mate_execute_shell(const char* dir, const char* commandline)
{
	return mate_execute_shell_fds(dir, commandline, TRUE);
}

/**
 * mate_prepend_terminal_to_vector:
 * @argc: a pointer to the vector size
 * @argv: a pointer to the vector
 *
 * Description:  Prepends a terminal (either the one configured as default in
 * the user's MATE setup, or one of the common xterm emulators) to the passed
 * in vector, modifying it in the process.  The vector should be allocated with
 * #g_malloc, as this will #g_free the original vector.  Also all elements must
 * have been allocated separately.  That is the standard glib/MATE way of
 * doing vectors however.  If the integer that @argc points to is negative, the
 * size will first be computed.  Also note that passing in pointers to a vector
 * that is empty, will just create a new vector for you.
 **/
void mate_prepend_terminal_to_vector(int* argc, char*** argv)
{
	#ifndef G_OS_WIN32
		char** real_argv;
		int real_argc;
		int i, j;
		char** term_argv = NULL;
		int term_argc = 0;
		MateConfClient* client;

		gchar* terminal = NULL;

		char** the_argv;

		g_return_if_fail(argc != NULL);
		g_return_if_fail(argv != NULL);

		/* sanity */
		if (*argv == NULL)
			*argc = 0;

		the_argv = *argv;

		/* compute size if not given */
		if (*argc < 0)
		{
			for (i = 0; the_argv[i] != NULL; i++)
				/* nothing */;

			*argc = i;
		}

		/* init our mateconf stuff if necessary */
		mate_mateconf_lazy_init();

		client = mateconf_client_get_default();
		terminal = mateconf_client_get_string(client, "/desktop/mate/applications/terminal/exec", NULL);
		g_object_unref(client);

		if (terminal)
		{
			gchar* command_line;
			gchar* exec_flag;

			exec_flag = mateconf_client_get_string(client, "/desktop/mate/applications/terminal/exec_arg", NULL);

			if (exec_flag == NULL)
				command_line = g_strdup(terminal);
			else
				command_line = g_strdup_printf("%s %s", terminal, exec_flag);

			g_shell_parse_argv(command_line, &term_argc, &term_argv, NULL /* error */);

			g_free(command_line);
			g_free(exec_flag);
			g_free(terminal);
		}

		if (term_argv == NULL)
		{
			char* check;

			term_argc = 2;
			term_argv = g_new0(char*, 3);

			check = g_find_program_in_path("mate-terminal");

			if (check != NULL)
			{
				term_argv[0] = check;
				/* Note that mate-terminal takes -x and
				 * as -e in mate-terminal is broken we use that. */
				term_argv[1] = g_strdup("-x");
			}
			else
			{
				if (check == NULL)
					check = g_find_program_in_path("nxterm");
				if (check == NULL)
					check = g_find_program_in_path("color-xterm");
				if (check == NULL)
					check = g_find_program_in_path("rxvt");
				if (check == NULL)
					check = g_find_program_in_path("xterm");
				if (check == NULL)
					check = g_find_program_in_path("dtterm");
				if (check == NULL)
				{
					g_warning(_("Cannot find a terminal, using xterm, even if it may not work"));
					check = g_strdup("xterm");
				}

				term_argv[0] = check;
				term_argv[1] = g_strdup("-e");
			}
		}

		real_argc = term_argc + *argc;
		real_argv = g_new(char*, real_argc + 1);

		for (i = 0; i < term_argc; i++)
			real_argv[i] = term_argv[i];

		for (j = 0; j < *argc; j++, i++)
			real_argv[i] = (char*) the_argv[j];

		real_argv[i] = NULL;

		g_free(*argv);

		*argv = real_argv;
		*argc = real_argc;

		/* we use g_free here as we sucked all the inner strings
		 * out from it into real_argv */
		g_free(term_argv);

	#else /* !G_OS_WIN32 */
		/* FIXME: Implement when needed */
		g_warning("mate_prepend_terminal_to_vector: Not implemented");
	#endif /* G_OS_WIN32 */
}

/**
 * mate_execute_terminal_shell_fds:
 * @dir: Directory in which child should be executed, or %NULL for current
 *       directory
 * @commandline: Shell command to execute
 * @close_fds: Like close_fds in mate_execute_async_with_env_fds()
 *
 * Description:  Like mate_execute_shell_fds(), except that it runs the
 * terminal as well.  Note that the pid of the terminal is
 * returned, not the pid of the user's program.
 * If commandline is %NULL, just the shell is run.
 *
 * Returns: process id of terminal, or %-1 on error.
 **/
int mate_execute_terminal_shell_fds(const char* dir, const char* commandline, gboolean close_fds)
{
	char** argv;
	int argc;
	int r;

	argv = g_new(char*, 4);

	argv[0] = mate_util_user_shell();

	if (commandline != NULL)
	{
		argc = 3;
		argv[1] = g_strdup("-c");
		argv[2] = g_strdup(commandline);
		argv[3] = NULL;
	}
	else
	{
		/* FIXME: really this should more be a
		 * --login terminal, but the user preference includes
		 * the -e, -x or whatever flag which makes this impossible,
		 * change the preference in 2.0 to be two keys, and one
		 * of them for a login terminal */
		argc = 1;
		argv[1] = NULL;
	}

	mate_prepend_terminal_to_vector(&argc, &argv);

	r = mate_execute_async_with_env_fds(dir, argc, argv, 0, NULL, close_fds);

	g_strfreev(argv);

	return r;
}

/**
 * mate_execute_terminal_shell:
 * @dir: Directory in which child should be executed, or NULL for current
 *       directory
 * @commandline: Shell command to execute
 *
 * Description:  Like #mate_execute_async, except that it runs the
 * terminal as well.  Note that the pid of the terminal is
 * returned, not the pid of the user's program.
 * If commandline is %NULL, just the shell is run.
 *
 * Returns: process id of terminal, or %-1 on error.
 **/
int mate_execute_terminal_shell(const char* dir, const char* commandline)
{
	return mate_execute_terminal_shell_fds (dir, commandline, TRUE);
}
