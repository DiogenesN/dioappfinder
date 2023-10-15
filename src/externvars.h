#include <gtk/gtk.h>

extern char 		indexOfApp[];
extern char 		*names[256][100];
extern char 		*execs[256][100];
extern char 		*icons[256][100];
extern int			path_error;
extern int 			appIsRunning;
extern int	 		strings_counter;

// definig structs to be used as arguments for g_signal_connect and binding listitems
struct myOptEntryChanged {
	GtkWidget 		   *label;
	GtkWidget 		   *window;
	GtkFilterListModel *filtermodel;
	GtkWidget 		   *scrolled;
	GtkStringFilter    *filter;
};

struct myOptEntryAcivate {
	GtkSingleSelection *selection;
	GtkWidget 		   *window;
};

struct NameExecIcon {
	char *names[777];
	char *execs[777];
	char *icons[777];
};
