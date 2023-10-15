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
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <sys/stat.h>
#include "posixspawn.h"
#include "configsgen.h"
#include "externvars.h"
#include <pango/pango.h>
#include "processdirectory.h"
#include "processlinesinconf.h"

struct NameExecIcon myNameExecIcon;
struct NameExecIcon *p_myNameExecIcon = &myNameExecIcon;

// creating labels to fill up the listview
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

	GtkWidget *icon;
	icon = gtk_image_new();
	gtk_image_set_icon_size(GTK_IMAGE(icon), GTK_ICON_SIZE_LARGE);
	
	GtkWidget *box;
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
	gtk_box_prepend(GTK_BOX(box), icon);
	gtk_box_append(GTK_BOX(box), label);

	// adds GtkBox as child of GtkListItem
	gtk_list_item_set_child(list_item, box);
	
	// freing resources
	g_object_unref(layout);
	pango_attr_list_unref(attr_list);
}

// binding the items to the list, this function runs in a loop
static void bind_listitem(GtkListItemFactory *factory, GtkListItem *list_item) {
	(void)factory; // Marking the parameter as unused

	char buffer[256];
	char noWhiteSpaceName[256];
	char noWhiteSpaceIconName[256];

	FILE *file = fopen("/tmp/.dio_app_finder_indexes", "r");

	// Get the GtkBox widget
	GtkWidget *box;
	box = gtk_list_item_get_child(list_item);

	// Find the GtkLabel widget within the GtkBox
	GtkWidget *label;
	label = gtk_widget_get_last_child(box);

	// Find the GtkImage widget within the GtkBox
	GtkWidget *icon;
	icon = gtk_widget_get_first_child(box);

	// gets the strings from GtkStringList *myStrings in startup function
	GtkStringObject *obj = gtk_list_item_get_item(list_item);
	const char *strings = gtk_string_object_get_string(GTK_STRING_OBJECT(obj));
	
    //fix the string that contains leading whitespace
	const char *ptrNames = strings;

	while (isspace(*ptrNames)) {
		ptrNames++;
	}

	strcpy(noWhiteSpaceName, ptrNames);

	// sets label text to strings one at a time
	gtk_label_set_label(GTK_LABEL(label), noWhiteSpaceName);
    
    // mapping icons to corresponding app names
	while (fgets(buffer, sizeof(buffer), file) != NULL) {
	char *pos = strstr(buffer, "===");
		if (strstr(buffer, strings) != NULL) {
			if (pos != NULL) {
				strcpy(indexOfApp, pos + 3);
				indexOfApp[strlen(indexOfApp) - 1] = '\0';
			}
		}
	}

    fclose(file);

    int index = atoi(indexOfApp);

    //fix the string that contains leading whitespace
	char *ptrIcon = p_myNameExecIcon->icons[index];

	while (isspace(*ptrIcon)) {
		ptrIcon++;
	}

	strcpy(noWhiteSpaceIconName, ptrIcon);

    // if noWhiteSpaceIconName=icon or Icon=/path/icon.png
    if (strchr(p_myNameExecIcon->icons[index], '/') == NULL) {
		gtk_image_set_from_icon_name(GTK_IMAGE(icon), (const char *)noWhiteSpaceIconName);
	}
	else {
		gtk_image_set_from_file(GTK_IMAGE(icon), (const char *)noWhiteSpaceIconName);
	}
}

// gets strings for GtkRxpression which are used later by GtkStringFilter
char *get_item(GtkStringObject *self) {
	if (GTK_IS_STRING_OBJECT(self)) {
		return  g_strdup(gtk_string_object_get_string(self));
	}
	else {
		return NULL;
	}
}

// action to take while typing in GtkEntry
static void entry_changed_all(GtkEntry *entry, gpointer data) {
	struct myOptEntryChanged *p_myOptEntryChanged = (struct myOptEntryChanged *)data;

	GtkEntryBuffer *entry_buffer;
	entry_buffer = gtk_entry_get_buffer(GTK_ENTRY(entry));
    const char *entry_text = gtk_entry_buffer_get_text(entry_buffer);

	// sets the GtkFilter, a string to compare the list against
	gtk_string_filter_set_search(p_myOptEntryChanged->filter, entry_text);
    
    // adjust the size of scrolled window according to displayed items
	guint n_items = g_list_model_get_n_items(G_LIST_MODEL(p_myOptEntryChanged->filtermodel));

	n_items = n_items * 45; // 45 is the size (length) of one element in the list
	
	if (n_items > 500) n_items = 500; // limits the length of list to 500

    if (strcmp(entry_text, "\0") == 0 || n_items == 0) { // if nothing is typwd or no search results
    	gtk_widget_set_visible(GTK_WIDGET(p_myOptEntryChanged->label), FALSE);
    	gtk_widget_set_visible(GTK_WIDGET(p_myOptEntryChanged->scrolled), FALSE);
    	gtk_window_set_default_size(GTK_WINDOW(p_myOptEntryChanged->window), 777, 0);
    }
	else {
    	gtk_widget_set_visible(GTK_WIDGET(p_myOptEntryChanged->label), TRUE);
		gtk_widget_set_visible(GTK_WIDGET(p_myOptEntryChanged->scrolled), TRUE);
		gtk_scrolled_window_set_min_content_height(\
					GTK_SCROLLED_WINDOW(p_myOptEntryChanged->scrolled), n_items);
    	gtk_window_set_default_size(GTK_WINDOW(p_myOptEntryChanged->window), 777, 0);
	}
}

// pressing Enter on the GtkEntry action
static void entry_activate_all(GtkEntry *entry, guint position, gpointer data) {
	(void)entry;
	(void)position;
	//char noWhiteSpaceExec[256];
	struct myOptEntryAcivate *p_myOptEntryAcivate = (struct myOptEntryAcivate *)data;

	// gets currenty selected item
	GtkStringObject *myItem;
	myItem = gtk_single_selection_get_selected_item(p_myOptEntryAcivate->selection);

	const char *selectedItem = gtk_string_object_get_string(myItem);

	gtk_window_close(GTK_WINDOW(p_myOptEntryAcivate->window));

	for (int i = 0; *names[i] != NULL; i++) { // bind Exec= to Name= and execute
		if (strcmp(g_strdup(selectedItem), p_myNameExecIcon->names[i]) == 0) {
			printf("Executing %s\n", p_myNameExecIcon->execs[i]);

			//system(noWhiteSpaceExec);
			run_cmd(p_myNameExecIcon->execs[i]); // posixspawn.h

			free(*names[strings_counter]);
			free(*execs[strings_counter]);
			free(*icons[strings_counter]);
			break;
		}
	}
}

// is avtivated when given path to process_directory does not exist
static void close_error_btn(GtkButton *button, GtkWidget *window) {
	(void)button;
	gtk_window_close(GTK_WINDOW(window));
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
	// this file is needed to map the app name to its index in the string array
    FILE *file = fopen("/tmp/.dio_app_finder_indexes", "w+");

    if (file == NULL) {
        perror("Error opening file");
    }

	for (int i = 0; *names[i] != NULL; i++) { //populating string srray with Names=
		p_myNameExecIcon->names[i] = *names[i];
		p_myNameExecIcon->execs[i] = *execs[i];
		p_myNameExecIcon->icons[i] = *icons[i];
		// creating a list that contains name of app and its index in the string array
		fprintf(file, "%s===%d\n", p_myNameExecIcon->names[i], i); 
	}
	
	fclose(file);

	GtkWidget *window;
	window = gtk_application_window_new(GTK_APPLICATION(app));
	gtk_window_set_default_size(GTK_WINDOW(window), 777, 0);
	gtk_window_set_title(GTK_WINDOW(window), "DioAppFinder");
	gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
	gtk_window_set_icon_name(GTK_WINDOW(window), "application-x-deb");
	
	GtkWidget *error;
	error = gtk_label_new("\n\n\nERROR! The path you provided does not exist!\n");
	gtk_label_set_markup(GTK_LABEL(error), "<span size='large' \
	weight='bold'><tt>\n\n\t      ⚠️ ERROR!\n The path you provided does not exist! \n</tt></span>");
	
	GtkWidget *closeButton;
	closeButton = gtk_button_new_with_label("❌ Close");

	GtkWidget *myEntry;
	myEntry = gtk_entry_new();
	gtk_entry_set_input_hints(GTK_ENTRY(myEntry), GTK_INPUT_HINT_WORD_COMPLETION);
	gtk_entry_set_alignment(GTK_ENTRY(myEntry), 0.5);
	gtk_entry_set_has_frame(GTK_ENTRY(myEntry), TRUE);
	gtk_entry_set_activates_default(GTK_ENTRY(myEntry), TRUE);
	gtk_entry_set_placeholder_text(GTK_ENTRY(myEntry), (const gchar*)"Type application name");

	// customizing the style, text size of the GtkEntry Widget
	PangoAttrList *attr_list = pango_attr_list_new();
	PangoAttribute *attr = pango_attr_size_new_absolute(30 * PANGO_SCALE);
	pango_attr_list_insert(attr_list, attr);
	PangoLayout *layout = gtk_widget_create_pango_layout(myEntry, NULL);
	pango_layout_set_attributes(layout, attr_list);

	gtk_entry_set_attributes(GTK_ENTRY(myEntry), attr_list);

	// turns all strings into objects
	GtkStringList *myStrings;
	myStrings = gtk_string_list_new((const char * const *)p_myNameExecIcon->names);

	// creates a list model from the object of string objects
	GListModel *myGlistModel;
	myGlistModel = G_LIST_MODEL(myStrings);

	// creats a list item factory to setup and bind the string object to the list view
	GtkListItemFactory *myGtkListItemFactory;
	myGtkListItemFactory = gtk_signal_list_item_factory_new();

	// expression to evaluate to get the strings for comparing and filtering
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
	gtk_string_filter_set_expression(myGtkStringFilter, expression);

	// defines the substring to look for in the list items
	GtkFilter *myGtkFilter;
	myGtkFilter = GTK_FILTER(myGtkStringFilter);

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

	// adding the list view as the child
	GtkWidget *myScrolledWindow;
	myScrolledWindow = gtk_scrolled_window_new();
	gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(myScrolledWindow), true);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(myScrolledWindow), myGtkListView);
	gtk_scrolled_window_set_max_content_height(GTK_SCROLLED_WINDOW(myScrolledWindow), 500);
	gtk_widget_set_visible(myScrolledWindow, FALSE);

	// just an empty label as a separator between entry and list view
	const char *l = "▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀";
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

	// assigning values to structs to be used ar arguments for g_signal_connect
	struct myOptEntryAcivate *p_myOptEntryAcivate = (struct myOptEntryAcivate *) \
		   								malloc(sizeof(struct myOptEntryAcivate));
		p_myOptEntryAcivate->window 	= window;
		p_myOptEntryAcivate->selection  = mySingleSelection;

	struct myOptEntryChanged *p_myOptEntryChanged = (struct myOptEntryChanged *) \
		   								  malloc(sizeof(struct myOptEntryChanged));
		p_myOptEntryChanged->window 	 = window;
		p_myOptEntryChanged->label 		 = label;
		p_myOptEntryChanged->filtermodel = myGtkStringFilterListModel;
		p_myOptEntryChanged->scrolled 	 = myScrolledWindow;
		p_myOptEntryChanged->filter 	 = myGtkStringFilter;

	// callbacks (funtions) to run on signals that widgets emit
	g_signal_connect(myGtkListItemFactory, "setup", G_CALLBACK(setup_listitem), NULL);
	g_signal_connect(myGtkListItemFactory, "bind", G_CALLBACK(bind_listitem), NULL);
	g_signal_connect(myEntry, "activate", G_CALLBACK(entry_activate_all), \
					 					  (gpointer)p_myOptEntryAcivate);

	g_signal_connect(myGtkListView, "activate", G_CALLBACK(entry_activate_all), \
											    (gpointer)p_myOptEntryAcivate);

	g_signal_connect(myEntry, "changed", G_CALLBACK(entry_changed_all), \
					 					(gpointer)p_myOptEntryChanged);

	g_signal_connect(closeButton, "clicked", G_CALLBACK(close_error_btn), window);

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

	int status;
	GtkApplication *app;
	app = gtk_application_new("com.github.DiogenesN.dioappfinder",	G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app, "startup", G_CALLBACK(startup), NULL);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	status = g_application_run(G_APPLICATION(app), 0, NULL);
	g_object_unref(app);

	return status;
}
