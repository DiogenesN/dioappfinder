#include <glib.h>

gchar 		indexOfApp[256];
gchar 		*names[256][1];
gchar 		*execs[256][1];
gchar 		*icons[256][1];
gchar 		*paths[256][1];
gchar		*names_and_indexes[256][2];
gint		path_error = 1;
gint 		appIsRunning = 0;
gint 		strings_counter = 0;
