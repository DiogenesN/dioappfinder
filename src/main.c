///////////////////////////////////////////////////////////////////////////////////////////////////
/*
DioAppFinder:
	A nice quick independant desktop agnostic application finder.
	By default it only searches for desktop files in /usr/share/applications
	You can add your own custom paths in .config/dioappfinder/dioappfinder.conf
	or use dioappfinder-settings to manage the paths.
*/
///////////////////////////////////////////////////////////////////////////////////////////////////
#include <glib.h>
#include "copy.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "structs.h"
#include "lsfiles.h"
#include <gtk/gtk.h>
#include <sys/stat.h>
#include "configsgen.h"
#include "externvars.h"
#include <pango/pango.h>
#include "subprocessrun.h"
#include "processdirectory.h"
#include "processlinesinconf.h"

GtkExpressionWatch *watchText;

/// left click on list item
static void label_clicked(gpointer data) {
	(void)data;

	data = GTK_WINDOW(p_optEntryChanged->window);
	gtk_widget_grab_focus(p_optEntryChanged->entry);
	gtk_popover_popup(GTK_POPOVER(p_optEntryChanged->popup));
	gtk_list_view_set_single_click_activate(GTK_LIST_VIEW(p_optEntryChanged->listview), FALSE);
}

/// cancel button on right click
static void addtofavcancel(GtkWidget *pop, gpointer data) {
	(void)data;

	gtk_popover_popdown(GTK_POPOVER(pop));
	data = GTK_WINDOW(p_optEntryChanged->window);
	gtk_widget_grab_focus(p_optEntryChanged->entry);
	gtk_widget_set_cursor(p_optEntryChanged->entry, NULL);
	gtk_list_view_set_single_click_activate(GTK_LIST_VIEW(p_optEntryChanged->listview), TRUE);
}

/// 'Add to favorite' button on right click
static void addtofav(GtkWidget *pop, gpointer data) {
	(void)data;

	gtk_popover_popdown(GTK_POPOVER(pop));
	data = GTK_WINDOW(p_optEntryChanged->window);
	gtk_widget_grab_focus(p_optEntryChanged->entry);
	gtk_widget_set_cursor(p_optEntryChanged->entry, NULL);
	gtk_list_view_set_single_click_activate(GTK_LIST_VIEW(p_optEntryChanged->listview), TRUE);

	// defining the menu config dir to list the num of files in
	// this number will make the numbet.desktop file
	gchar menuDirPath[256];
	const gchar *HOME = getenv("HOME");
	const gchar *myMenuPath = "/.config/diopanel/diomenu";
	snprintf(menuDirPath, sizeof(menuDirPath), "%s%s", HOME, myMenuPath);

	// gets currenty selected item
	GtkStringObject *myItem;
	myItem = gtk_single_selection_get_selected_item(p_optEntryAcivate->selection);

	const gchar *selectedItem = gtk_string_object_get_string(myItem);

	// getting the path associated with the name
	// getting diomenupaths.conf path
	char diomenuPathsConfPath[777];
	strcpy(diomenuPathsConfPath, HOME);
	strcat(diomenuPathsConfPath, "/.config/diopanel/diomenu/diomenupaths.conf");

	FILE *configPathOpen = fopen(diomenuPathsConfPath, "a");

	guint nrOflistItems = g_list_model_get_n_items(p_dataOptions->glistmodel);

	for (size_t i = 0; i < nrOflistItems; i++) {
		if (strcmp(selectedItem, *names_and_indexes[i]) == 0) {
			gchar *iconIndex = names_and_indexes[i][1];
			gint iconIndexNum = atoi(iconIndex);

			fprintf(configPathOpen, "%s\n", *paths[iconIndexNum]);

			g_print("added %s to conf\n", *paths[iconIndexNum]);
		}
	}

	fclose(configPathOpen);
}

/// creating labels to fill up the listview
static void setup_listitem(GtkListItemFactory *factory, GtkListItem *list_item) {
	(void)factory; // Marking the parameter as unused
	
	GtkWidget *label;
	label = gtk_label_new(NULL);

	// defining the size of text in label
	PangoAttrList *attr_list = pango_attr_list_new();
	PangoAttribute *attr = pango_attr_size_new_absolute(30 * PANGO_SCALE);
	pango_attr_list_insert(attr_list, attr);
	PangoLayout *layout = gtk_widget_create_pango_layout(label, NULL);
	pango_layout_set_attributes(layout, attr_list);

	gtk_label_set_attributes(GTK_LABEL(label), attr_list);

	GtkWidget *addfav;
	addfav = gtk_button_new_with_label("Add to favorites");
	
	GtkWidget *addfavcancel;
	addfavcancel = gtk_button_new_with_label("Cancel");

	GtkWidget *boxButton;
	boxButton = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	gtk_box_prepend(GTK_BOX(boxButton), addfav);
	gtk_box_append(GTK_BOX(boxButton), addfavcancel);

	GtkWidget *pop;
    pop = gtk_popover_new();
	gtk_widget_set_size_request(pop, 300, 30);
    gtk_popover_set_autohide(GTK_POPOVER(pop), FALSE);
    gtk_popover_set_offset(GTK_POPOVER(pop), 1, -110);
	gtk_popover_set_child(GTK_POPOVER(pop), boxButton);

	GtkWidget *icon;
	icon = gtk_image_new();
	gtk_image_set_icon_size(GTK_IMAGE(icon), GTK_ICON_SIZE_LARGE);
	
	GtkWidget *box;
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
	gtk_box_prepend(GTK_BOX(box), icon);
	gtk_box_append(GTK_BOX(box), label);
	gtk_box_append(GTK_BOX(box), pop);
	
	g_signal_connect_swapped(addfavcancel, "clicked", G_CALLBACK(addtofavcancel), pop);
	g_signal_connect_swapped(addfav, "clicked", G_CALLBACK(addtofav), pop);

	// adds GtkBox as child of GtkListItem
	gtk_list_item_set_child(list_item, box);
	
	// freing resources
	g_object_unref(layout);
	pango_attr_list_unref(attr_list);
}

/// binding the items to the list, this function runs in a loop
static void bind_listitem(GtkListItemFactory *factory, GtkListItem *list_item) {
	(void)factory; // Marking the parameter as unused

	// Get the GtkBox widget
	GtkWidget *box;
	box = gtk_list_item_get_child(list_item);

	// Find the GtkImage widget within the GtkBox
	GtkWidget *icon;
	icon = gtk_widget_get_first_child(box);

	// Find the next child after GtkImage whoch is a label 
	GtkWidget *label;
	label = gtk_widget_get_next_sibling(icon);

	// Find the next child after label whoch is a popup 
	p_optEntryChanged->popup = gtk_widget_get_next_sibling(label);;

	// getttins names to populate the menu
	GtkStringObject *stringObjectNames;
	stringObjectNames = gtk_list_item_get_item(list_item);

	const gchar *stringsNames = gtk_string_object_get_string(stringObjectNames);

	//fix the string that contains leading whitespace
	const gchar *ptrNames = stringsNames;
	while (isspace(*ptrNames)) {
		ptrNames++;
	}

	gchar noWhiteSpaceName[256];
	strcpy(noWhiteSpaceName, ptrNames);

	// sets label text to strings one at a time
	gtk_label_set_label(GTK_LABEL(label), noWhiteSpaceName);

	// getttins icons to populate the menu
	guint nrOflistItems = g_list_model_get_n_items(p_dataOptions->glistmodel);

	for (size_t i = 0; i < nrOflistItems; i++) {
		if (strcmp(noWhiteSpaceName, *names_and_indexes[i]) == 0) {
			gchar *iconIndex = names_and_indexes[i][1];
			gint iconIndexNum = atoi(iconIndex);

			const gchar *stringsIcons = gtk_string_list_get_string(p_dataOptions->stringsIcons,
																				iconIndexNum);
			// fix the icon name that contains leading whitespace
			const gchar *ptrIcon = stringsIcons;

			while (isspace(*ptrIcon)) {
				ptrIcon++;
			}

			gchar noWhiteSpaceIconName[256];
			strcpy(noWhiteSpaceIconName, ptrIcon);

			// if noWhiteSpaceIconName=icon or Icon=/path/icon.png
			if (strchr(noWhiteSpaceIconName, '/') == NULL) {
				gtk_image_set_from_icon_name(GTK_IMAGE(icon), (const gchar *)noWhiteSpaceIconName);
			}
			else {
				gtk_image_set_from_file(GTK_IMAGE(icon), (const gchar *)noWhiteSpaceIconName);
			}
		}
	}
}

// gets strings for GtkRxpression which are used later by GtkStringFilter
gchar *get_item(GtkStringObject *self) {
	return g_strdup(gtk_string_object_get_string(self));
}

// pressing Enter on the GtkEntry action
static void entry_activate_all(gpointer data) {
	(void)data;

	// gets currenty selected item
	GtkStringObject *myItem;
	myItem = gtk_single_selection_get_selected_item(p_optEntryAcivate->selection);

	const gchar *selectedItem = gtk_string_object_get_string(myItem);

	// getttins icons to populate the menu
	guint nrOflistItems = g_list_model_get_n_items(p_dataOptions->glistmodel);

	for (size_t i = 0; i < nrOflistItems; i++) {
		if (strcmp(selectedItem, *names_and_indexes[i]) == 0) {
			gchar *iconIndex = names_and_indexes[i][1];
			gint iconIndexNum = atoi(iconIndex);

			const gchar *apptorun = gtk_string_list_get_string(p_dataOptions->stringsExecs,
																				iconIndexNum);

			g_print("Executing %s\n", apptorun);

			run_cmd((gchar*)apptorun);
		}
	}
	g_object_unref(G_OBJECT(p_optEntryAcivate->window));
	gtk_window_close(GTK_WINDOW(p_optEntryAcivate->window));
}

// action to take while typing in GtkEntry
static void entry_changed_all(GtkEntry *entry, gpointer data) {
	(void)data;

	GtkEntryBuffer *entry_buffer;
	entry_buffer = gtk_entry_get_buffer(GTK_ENTRY(entry));
    const gchar *entry_text = gtk_entry_buffer_get_text(entry_buffer);

    // adjust the size of scrolled window according to displayed items
	gint n_items = g_list_model_get_n_items(G_LIST_MODEL(p_optEntryChanged->filtermodel));

	n_items = n_items * 45; // 45 is the size (length) of one element in the list
	
	if (n_items > 400) n_items = 400; // limits the length of list to 500

    if (strcmp(entry_text, "\0") == 0 || n_items == 0) { // if nothing is typwd or no search results
    	gtk_widget_set_visible(GTK_WIDGET(p_optEntryChanged->label), FALSE);
    	gtk_widget_set_visible(GTK_WIDGET(p_optEntryChanged->scrolled), FALSE);
    	gtk_window_set_default_size(GTK_WINDOW(p_optEntryChanged->window), 777, 0);
    }
	else {
    	gtk_widget_set_visible(GTK_WIDGET(p_optEntryChanged->label), TRUE);
		gtk_widget_set_visible(GTK_WIDGET(p_optEntryChanged->scrolled), TRUE);
    	gtk_window_set_default_size(GTK_WINDOW(p_optEntryChanged->window), 777, 0);
		gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(p_optEntryChanged->scrolled),
																							n_items);
	}
}

// is avtivated when given path to process_directory does not exist
static void close_error_btn(GtkButton *button, GtkWidget *window) {
	(void)button;

	// getttins icons to populate the menu
	guint nrOflistItems = g_list_model_get_n_items(p_dataOptions->glistmodel);

	// Don't forget to free the allocated memory when done
	for (guint i = 0; i < nrOflistItems; i++) {
		for (guint j = 0; j < 2; j++) {
			free(names_and_indexes[i][j]);
		}
   }
	
	gtk_expression_watch_unwatch(watchText);
  	gtk_expression_watch_unref(watchText);

	p_optEntryChanged->label = NULL;
	p_optEntryChanged->entry = NULL;
	p_optEntryChanged->window = NULL;
	p_optEntryChanged->filter = NULL;
	p_optEntryAcivate->window = NULL;
	p_optEntryChanged->scrolled = NULL;
	p_optEntryAcivate->selection = NULL;
	p_optEntryChanged->filtermodel = NULL;

	p_optEntryAcivate->window = NULL;
	p_optEntryAcivate->selection = NULL;

	p_dataOptions->glistmodel = NULL;
	p_dataOptions->stringsNames = NULL;
	p_dataOptions->stringsExecs = NULL;
	p_dataOptions->stringsIcons = NULL;

	gtk_window_close(GTK_WINDOW(window));
}

// is avtivated when given path to process_directory does not exist
static void close_request(GtkWidget *window) {
	(void)window;

	// getttins icons to populate the menu
	guint nrOflistItems = g_list_model_get_n_items(p_dataOptions->glistmodel);

	// Don't forget to free the allocated memory when done
	for (guint i = 0; i < nrOflistItems; i++) {
		for (guint j = 0; j < 2; j++) {
			free(names_and_indexes[i][j]);
		}
	}

	p_optEntryChanged->label = NULL;
	p_optEntryChanged->entry = NULL;
	p_optEntryChanged->window = NULL;
	p_optEntryChanged->filter = NULL;
	p_optEntryAcivate->window = NULL;
	p_optEntryChanged->scrolled = NULL;
	p_optEntryAcivate->selection = NULL;
	p_optEntryChanged->filtermodel = NULL;

	p_optEntryAcivate->window = NULL;
	p_optEntryAcivate->selection = NULL;

	p_dataOptions->glistmodel = NULL;
	p_dataOptions->stringsNames = NULL;
	p_dataOptions->stringsExecs = NULL;
	p_dataOptions->stringsIcons = NULL;
}

// toggle open/close
static void activate(GtkApplication *app) {
	switch(appIsRunning) {
		case 0:
			g_print("Welcome to DioAppFinder!\n");
			appIsRunning = 1;
			break;
		default:
			g_print("Closing ...\n");
			g_application_quit(G_APPLICATION(app));
			break;
	}
}

// startup function
static void startup(GtkApplication *app) {
	GtkWidget *window;
	window = gtk_application_window_new(GTK_APPLICATION(app));
	gtk_window_set_default_size(GTK_WINDOW(window), 777, 0);
	gtk_window_set_title(GTK_WINDOW(window), "DioAppFinder");
	gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
	gtk_window_set_icon_name(GTK_WINDOW(window), "application-x-deb");

	g_signal_connect(window, "close-request", G_CALLBACK(close_request), window);;

	GtkWidget *error;
	error = gtk_label_new("\n\n\nERROR! The path you provided does not exist!\n");
	gtk_label_set_markup(GTK_LABEL(error), "<span size='large' \
	weight='bold'><tt>\n\n\t      ⚠️ ERROR!\n The path you provided does not exist! \n</tt></span>");
	
	GtkWidget *closeButton;
	closeButton = gtk_button_new_with_label("❌ Close");

	GtkWidget *myEntry;
	myEntry = gtk_entry_new();
	gtk_entry_set_alignment(GTK_ENTRY(myEntry), 0.5);
	gtk_entry_set_has_frame(GTK_ENTRY(myEntry), TRUE);
	gtk_entry_set_placeholder_text(GTK_ENTRY(myEntry), (const gchar*)"Type application name");
	
	p_optEntryChanged->entry = myEntry;

	g_signal_connect(myEntry, "changed", G_CALLBACK(entry_changed_all), NULL);
	g_signal_connect_swapped(myEntry, "activate", G_CALLBACK(entry_activate_all),
					 									(gpointer)p_optEntryAcivate);

	// customizing the style, text size of the GtkEntry Widget
	PangoAttrList *attr_list = pango_attr_list_new();
	PangoAttribute *attr = pango_attr_size_new_absolute(30 * PANGO_SCALE);
	pango_attr_list_insert(attr_list, attr);
	PangoLayout *layout = gtk_widget_create_pango_layout(myEntry, NULL);
	pango_layout_set_attributes(layout, attr_list);

	gtk_entry_set_attributes(GTK_ENTRY(myEntry), attr_list);

	GtkEntryBuffer *entry_buffer;
	entry_buffer = gtk_entry_get_buffer(GTK_ENTRY(myEntry));

	// turns all strings containing names into objects
	GtkStringList *myStringsNames;
	myStringsNames = gtk_string_list_new((const gchar * const *)names);

	p_dataOptions->stringsNames = myStringsNames;

	// turns all strings containing names into objects
	GtkStringList *myStringsExecs;
	myStringsExecs = gtk_string_list_new((const gchar * const *)execs);

	p_dataOptions->stringsExecs = myStringsExecs;

	// turns all strings containing names into objects
	GtkStringList *myStringsIcons;
	myStringsIcons = gtk_string_list_new((const gchar * const *)icons);

	p_dataOptions->stringsIcons = myStringsIcons;

	// creates a list model from the object of string objects
	GListModel *myGlistModel;
	myGlistModel = G_LIST_MODEL(myStringsNames);

	p_dataOptions->glistmodel = myGlistModel;

	// creats a list item factory to setup and bind the string object to the list view
	GtkListItemFactory *myGtkListItemFactory;
	myGtkListItemFactory = gtk_signal_list_item_factory_new();

	// callbacks (funtions) to run on signals that widgets emit
	g_signal_connect(myGtkListItemFactory, "setup", G_CALLBACK(setup_listitem), NULL);
	g_signal_connect(myGtkListItemFactory, "bind", G_CALLBACK(bind_listitem), NULL);

	//expression to evaluate to get the strings for comparing and filtering
	GtkExpression *expression;
	expression = gtk_cclosure_expression_new(G_TYPE_STRING,
											NULL,
											0,
											NULL,
											G_CALLBACK(get_item),
											NULL,
											NULL);

	// creating a string filter to show only matched items
	GtkStringFilter *myGtkStringFilter;
	myGtkStringFilter = gtk_string_filter_new(expression);
	gtk_string_filter_set_ignore_case(myGtkStringFilter, TRUE);
	gtk_string_filter_set_match_mode(myGtkStringFilter, GTK_STRING_FILTER_MATCH_MODE_SUBSTRING);

	// defines the substring to look for in the list items
	GtkFilter *myGtkFilter;
	myGtkFilter = GTK_FILTER(myGtkStringFilter);

	// binding entry buffer 'text' property to 'search' property of filter string
	GtkExpression *expressionText;
	expressionText = gtk_property_expression_new(GTK_TYPE_ENTRY_BUFFER, NULL, "text");

	watchText = gtk_expression_bind(expressionText, myGtkStringFilter, "search", entry_buffer);

	// create a filter fist model that will filter another model defined earlier
	GtkFilterListModel *myGtkStringFilterListModel;
	myGtkStringFilterListModel = gtk_filter_list_model_new(myGlistModel, myGtkFilter);

	// defining how selection of items behaves
	GtkSingleSelection *mySingleSelection;
	mySingleSelection = gtk_single_selection_new(G_LIST_MODEL(myGtkStringFilterListModel));

	GtkSelectionModel *mySelectionModel;
	mySelectionModel = GTK_SELECTION_MODEL(mySingleSelection);

	// creating the list view to display the list items but it needs to be a child of a widget
	GtkWidget *myGtkListView;
	myGtkListView = gtk_list_view_new(mySelectionModel, myGtkListItemFactory);
	gtk_list_view_set_single_click_activate(GTK_LIST_VIEW(myGtkListView), TRUE);

	g_signal_connect(myGtkListView, "activate", G_CALLBACK(entry_activate_all),
													(gpointer)p_optEntryAcivate);
	
	p_optEntryChanged->listview = myGtkListView;

	// adding the list view as the child
	GtkWidget *myScrolledWindow;
	myScrolledWindow = gtk_scrolled_window_new();
	gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(myScrolledWindow), true);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(myScrolledWindow), myGtkListView);
	gtk_scrolled_window_set_max_content_height(GTK_SCROLLED_WINDOW(myScrolledWindow), 400);
	gtk_widget_set_visible(myScrolledWindow, FALSE);

	// just an empty label as a separator between entry and list view
	const gchar *l = "▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀";
	GtkWidget *label;
	label = gtk_label_new(l);
	gtk_widget_set_visible(label, FALSE);

	// styling the label
    PangoAttrList *attr_list_label = pango_attr_list_new();
    PangoAttribute *attr_label = pango_attr_foreground_new(0, 17000, 25000);
    pango_attr_list_insert(attr_list_label, attr_label);
    gtk_label_set_attributes(GTK_LABEL(label), attr_list_label);

	// containers to hold widgets
	GtkWidget *myGtkBoxError;
	myGtkBoxError = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
	gtk_box_prepend(GTK_BOX(myGtkBoxError), error);
	gtk_box_append(GTK_BOX(myGtkBoxError), closeButton);

	GtkWidget *myGtkBox;
	myGtkBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	gtk_box_prepend(GTK_BOX(myGtkBox), myEntry);
	gtk_box_append(GTK_BOX(myGtkBox), label);
	gtk_box_append(GTK_BOX(myGtkBox), myScrolledWindow);
	gtk_box_set_homogeneous(GTK_BOX(myGtkBox), FALSE);

	// adds left click action for items in listview
	GtkGesture *click;
	click = gtk_gesture_click_new();

	GtkGestureSingle *singleclick;
	singleclick = GTK_GESTURE_SINGLE(click);

	gtk_gesture_single_set_button(singleclick, 3);

	GtkEventController *leftclick;
	leftclick = GTK_EVENT_CONTROLLER(click);
	gtk_event_controller_set_propagation_phase(leftclick, GTK_PHASE_CAPTURE);
	gtk_widget_add_controller(myGtkListView, leftclick);

	// assigning values to structs to be used ar arguments for g_signal_connect
	p_optEntryAcivate->window		= window;
	p_optEntryAcivate->selection	= mySingleSelection;

	p_optEntryChanged->label		= label;
	p_optEntryChanged->window		= window;
	p_optEntryChanged->scrolled		= myScrolledWindow;
	p_optEntryChanged->filter		= myGtkStringFilter;
	p_optEntryChanged->filtermodel	= myGtkStringFilterListModel;

	g_signal_connect(closeButton, "clicked", G_CALLBACK(close_error_btn), window);
    g_signal_connect_swapped(leftclick, "pressed", G_CALLBACK(label_clicked),
    												(gpointer)p_optEntryAcivate);

	// freeing
	g_object_unref(layout);
	pango_attr_list_unref(attr_list);
	pango_attr_list_unref(attr_list_label);

	// defining main window children and showing the main window if there is no error
	switch (path_error) { // show error if unexisted path
		case 0:
			gtk_window_set_title(GTK_WINDOW(window), "DioAppFinder Error");
			gtk_window_set_default_size(GTK_WINDOW(window), 400, 150);
			gtk_window_set_icon_name(GTK_WINDOW(window), "dialog-error");
			gtk_window_set_child(GTK_WINDOW(window), myGtkBoxError);
			break;
		default:
			gtk_window_set_child(GTK_WINDOW(window), myGtkBox);
			break;
	}
	gtk_window_present(GTK_WINDOW(window));
		
}

int main() {
	// create initial configs configsgen.h
	create_configs();

	// processing the list of directories in confi file processlinesinconf.h
	parse_config();

	gint status;
	GtkApplication *app;
	app = gtk_application_new("com.github.DiogenesN.dioappfinder",	G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app, "startup", G_CALLBACK(startup), NULL);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	status = g_application_run(G_APPLICATION(app), 0, NULL);

	p_optEntryChanged->label = NULL;
	p_optEntryChanged->entry = NULL;
	p_optEntryChanged->window = NULL;
	p_optEntryChanged->filter = NULL;
	p_optEntryAcivate->window = NULL;
	p_optEntryChanged->scrolled = NULL;
	p_optEntryAcivate->selection = NULL;
	p_optEntryChanged->filtermodel = NULL;

	p_optEntryAcivate->window = NULL;
	p_optEntryAcivate->selection = NULL;

	p_dataOptions->glistmodel = NULL;
	p_dataOptions->stringsNames = NULL;
	p_dataOptions->stringsExecs = NULL;
	p_dataOptions->stringsIcons = NULL;

	g_object_unref(app);

	return status;
}
