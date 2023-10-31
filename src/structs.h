#include <gtk/gtk.h>

#ifndef STRUCTS_H_
#define STRUCTS_H_

// definig structs to be used as arguments for g_signal_connect and binding listitems
struct optEntryChanged {
	GtkWidget 		   *entry;
	GtkWidget 		   *popup;
	GtkWidget 		   *label;
	GtkWidget 		   *window;
	GtkWidget 		   *listview;
	GtkWidget 		   *scrolled;
	GtkStringFilter    *filter;
	GtkFilterListModel *filtermodel;
};

struct optEntryChanged myoptEntryChanged;
struct optEntryChanged *p_optEntryChanged = &myoptEntryChanged;

struct optEntryAcivate {
	GtkSingleSelection *selection;
	GtkWidget 		   *window;
};

struct optEntryAcivate myoptEntryAcivate;
struct optEntryAcivate *p_optEntryAcivate = &myoptEntryAcivate;

struct dataOptions {
	GListModel		*glistmodel;
	GtkStringList	*stringsNames;
	GtkStringList	*stringsExecs;
	GtkStringList	*stringsIcons;
};

struct dataOptions mydataOptions;
struct dataOptions *p_dataOptions = &mydataOptions;

#endif
