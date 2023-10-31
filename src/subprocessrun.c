#include <pthread.h>
#include <gtk/gtk.h>
#include <gio/gio.h>

void *prerun_cmd(void *cmd) {
	char *command = (char*)cmd;

	GError *error;
	error = NULL;

	GSubprocess *subprocess;
	subprocess = g_subprocess_new(G_SUBPROCESS_FLAGS_SEARCH_PATH_FROM_ENVP,
																	&error,
																	"sh",
																	"-c",
																	command,
																	NULL);
	if (subprocess == NULL) {
		g_print("Error launching subprocess: %s\n", error->message);
		g_error_free(error);
		return NULL;
	}

	gboolean result = g_subprocess_wait(subprocess, NULL, &error);
	if (!result) {
		g_print("Subprocess error: %s\n", error->message);
		g_error_free(error);
	}
	g_object_unref(subprocess);
	return NULL;
}

void run_cmd(char *command) {
	pthread_t thread;
	if (pthread_create(&thread, NULL, prerun_cmd, command) != 0) {
		perror("Failed to create a thread");
		return;
	}
}
