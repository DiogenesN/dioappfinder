#include <gtk/gtk.h>

#ifndef EXTERNVARS_H_
#define EXTERNVARS_H_

extern gchar 		indexOfApp[];
extern gchar 		*names[256][1];
extern gchar 		*execs[256][1];
extern gchar 		*icons[256][1];
extern gchar 		*paths[256][1];
extern gchar		*names_and_indexes[256][2];
extern gint			path_error;
extern gint 		appIsRunning;
extern gint	 		strings_counter;
extern gint			favcancelclicked;

#endif
