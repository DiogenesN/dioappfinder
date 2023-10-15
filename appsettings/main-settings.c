#include <glib.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <pango/pango.h>

char *strings[30];
int  counter_index = 0;

struct OptSavePath {
	GtkWidget		   *window;
	GtkWidget 		   *entry;
	GtkSingleSelection *selection;
};

struct OptSavePath myOptSavePath;
struct OptSavePath *p_myOptSavePath = &myOptSavePath;

// populate strings with content of config file
static void populate_strings() {
	const char *HOME = getenv("HOME");

	if (HOME == NULL) {
        fprintf(stderr, "Unable to determine the user's home directory.\n");
        return;
    }

	const char *configDirPath = "/.config/dioappfinder";
	const char *nameOfConfig = "/dioappfinder.conf";
	char 	   fullPathWithName[256];

	snprintf(fullPathWithName, sizeof(fullPathWithName), "%s%s%s", HOME, configDirPath, nameOfConfig);
	// Open the file for reading
	FILE *file = fopen(fullPathWithName, "r");

	if (file == NULL) {
		perror("Error opening the file");
		return;
	}

	char line[256];     // Buffer for reading lines

	// Read lines from the file and store them in the strings array
	while (fgets(line, sizeof(line), file) != NULL) {
		// Remove the newline character, if present
		char *newline = strchr(line, '\n');
		if (newline != NULL) {
			*newline = '\0';
		}

		// Allocate memory for the string and copy it
		strings[counter_index] = strdup(line);
		counter_index++;

		if (counter_index >= 30) {
			break;  // Stop reading after 30 lines
		}
	}
}

// creating labels to fill up the listview
static void setup_listitem(GtkListItemFactory *factory, GtkListItem *list_item) {
	(void)factory; // Marking the parameter as unused
	GtkWidget *label;
	label = gtk_label_new(NULL);

	// defining the size of text in label
	PangoAttrList *attr_list = pango_attr_list_new();
	PangoAttribute *attr = pango_attr_size_new_absolute(20 * PANGO_SCALE);
	pango_attr_list_insert(attr_list, attr);
	PangoLayout *layout = gtk_widget_create_pango_layout(label, NULL);
	pango_layout_set_attributes(layout, attr_list);

	gtk_label_set_attributes(GTK_LABEL(label), attr_list);

	// adds GtkBox as child of GtkListItem
	gtk_list_item_set_child(list_item, label);
	
	// freing resources
	g_object_unref(layout);
	pango_attr_list_unref(attr_list);
}

// binding the items to the list, this function runs in a loop
static void bind_listitem(GtkListItemFactory *factory, GtkListItem *list_item) {
	(void)factory; // Marking the parameter as unused

	// Get the GtkBox widget
	GtkWidget *label;
	label = gtk_list_item_get_child(list_item);

	// gets the strings from GtkStringList *myStrings in startup function
	GtkStringObject *obj = gtk_list_item_get_item(list_item);
	const char *strings = gtk_string_object_get_string(GTK_STRING_OBJECT(obj));

	// sets label text to strings one at a time
	gtk_label_set_label(GTK_LABEL(label), strings);
}

// closes the dialog without saving
static void close_cb(gpointer data) {
	struct OptSavePath *p_myOptSavePath = (struct OptSavePath *)data;

	// getting the path a user typed into the entry	
	GtkEntryBuffer *entry_buffer;
	entry_buffer = gtk_entry_get_buffer(GTK_ENTRY(p_myOptSavePath->entry));
    const char *entry_text = gtk_entry_buffer_get_text(entry_buffer);

	// this adds a new string to the string array and increases the index
	// Ensure enough space to add a new string
	if (*entry_text != '\0') { // ensures no empty lines are added to the list
		strings[counter_index] = g_strdup(entry_text);
		counter_index++;
		strings[counter_index] = '\0';

		// creates a new updated string that contains the newly added string
		GtkStringList *myStrings;
		myStrings = gtk_string_list_new((const char * const *)strings);

		GListModel *myGlistModel;
		myGlistModel = G_LIST_MODEL(myStrings);

		gtk_single_selection_set_model(p_myOptSavePath->selection, myGlistModel);

		free(strings[counter_index]);

		gtk_window_close(GTK_WINDOW(p_myOptSavePath->window));
	}
	else {
		free(strings[counter_index]);
		gtk_window_close(GTK_WINDOW(p_myOptSavePath->window));
	}
}

// opens dialog asks to add a new path to the list
static void add_btn() {
	GtkWidget *window;
	window = gtk_window_new();
	gtk_window_set_default_size(GTK_WINDOW(window), 500, 30);
	gtk_window_set_modal(GTK_WINDOW(window), TRUE);
	gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
	gtk_window_set_title(GTK_WINDOW(window), "DioAppFinder New Path");
	gtk_window_set_icon_name(GTK_WINDOW(window), "application-x-deb");

	GtkWidget *entry;
	entry = gtk_entry_new();
	gtk_entry_set_has_frame(GTK_ENTRY(entry), TRUE);
	gtk_entry_set_activates_default(GTK_ENTRY(entry), TRUE);
	gtk_entry_set_placeholder_text(GTK_ENTRY(entry), (const gchar*)"e.g. /home/user/customdir");

	// define first and second members of struct OptSavePath used in close_cb 
	p_myOptSavePath->window = window;
	p_myOptSavePath->entry = entry;

	GtkWidget *cancelPathButton;
	cancelPathButton = gtk_button_new_with_label("Camcel");
	
	GtkWidget *addPathButton;
	addPathButton = gtk_button_new_with_label("Add");
	
	GtkWidget *label;
	label = gtk_label_new("Type your path below");

	GtkWidget *grid;
	grid = gtk_grid_new();
	gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
	gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
	gtk_grid_set_column_spacing(GTK_GRID(grid), 300);
	gtk_grid_attach(GTK_GRID(grid), cancelPathButton, 1, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), addPathButton, 2, 1, 1, 1);

	GtkWidget *myGtkBox;
	myGtkBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	gtk_box_prepend(GTK_BOX(myGtkBox), label);
	gtk_box_append(GTK_BOX(myGtkBox), entry);
	gtk_box_append(GTK_BOX(myGtkBox), grid);
	
	gtk_window_set_child(GTK_WINDOW(window), myGtkBox);
	gtk_window_present(GTK_WINDOW(window));

	// callbacks (funtions) to run on signals that widgets emit
	g_signal_connect_swapped(cancelPathButton, "clicked", G_CALLBACK(gtk_window_close), window);
	g_signal_connect_swapped(addPathButton, "clicked", G_CALLBACK(close_cb),
												 (gpointer)p_myOptSavePath);
}

// removes the selected item from the list of paths
static void remove_btn(guint pos, GtkSingleSelection *selection) {
	// gets currenty selected item
	GtkStringObject *myItem;
	myItem = gtk_single_selection_get_selected_item(selection);
	pos = gtk_single_selection_get_selected(selection);

	const char *selectedItem = gtk_string_object_get_string(myItem);
	
	printf("nr: %d item: %s\n", pos, selectedItem);

		free(strings[pos]);
		
		// Shift the elements to fill the gap, 29 is the size of strings - 1
		for (int i = pos; i < 29; i++) {
			strings[i] = strings[i + 1];
		}

		// creates a new updated string that contains the newly added string
		GtkStringList *myStrings;
		myStrings = gtk_string_list_new((const char * const *)strings);

		GListModel *myGlistModel;
		myGlistModel = G_LIST_MODEL(myStrings);

		gtk_single_selection_set_model(selection, myGlistModel);

		free(strings[counter_index]);
}

// saves the changes to the conf file
static void save_btn(GtkWidget *window) {
	const char *HOME = getenv("HOME");

	if (HOME == NULL) {
        fprintf(stderr, "Unable to determine the user's home directory.\n");
        return;
    }

	const char *configDirPath = "/.config/dioappfinder";
	const char *nameOfConfig = "/dioappfinder.conf";
	char 	   fullPathWithName[256];

	snprintf(fullPathWithName, sizeof(fullPathWithName), "%s%s%s", HOME, configDirPath, nameOfConfig);

	FILE *file = fopen(fullPathWithName, "w");

	if (file == NULL) {
		perror("Error opening the file");
		return;
	}
	// Write each string to the file, one per line
	for (int i = 0; i < 30; i++) {
		if (strings[i] != NULL) {
			fprintf(file, "%s\n", strings[i]);
		}
	}

	// Close the file
	fclose(file);
	gtk_window_close(GTK_WINDOW(window));
}
// activate function
static void activate(GtkApplication *app) {
	GtkWidget *window;
	window = gtk_application_window_new(GTK_APPLICATION(app));
	gtk_window_set_default_size(GTK_WINDOW(window), 777, 0);
	gtk_window_set_title(GTK_WINDOW(window), "DioAppFinder Settings");
	gtk_window_set_icon_name(GTK_WINDOW(window), "application-x-deb");

	// turns all strings into objects
	GtkStringList *myStrings;
	myStrings = gtk_string_list_new((const char * const *)strings);

	// creates a list model from the object of string objects
	GListModel *myGlistModel;
	myGlistModel = G_LIST_MODEL(myStrings);

	// creats a list item factory to setup and bind the string object to the list view
	GtkListItemFactory *myGtkListItemFactory;
	myGtkListItemFactory = gtk_signal_list_item_factory_new();

	// defining how selection of items behaves
	GtkSingleSelection *mySingleSelection;
	mySingleSelection = gtk_single_selection_new(G_LIST_MODEL(myGlistModel));

	// defining second member of struct OptSavePath used in close_cb
	p_myOptSavePath->selection = mySingleSelection;

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
	gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(myScrolledWindow), 250);
	gtk_scrolled_window_set_max_content_height(GTK_SCROLLED_WINDOW(myScrolledWindow), 250);

	GtkWidget *saveButton;
	saveButton = gtk_button_new_with_label("Save");

	GtkWidget *removeButton;
	removeButton = gtk_button_new_with_label("Remove Selected");

	GtkWidget *addButton;
	addButton = gtk_button_new_with_label("Add New Entry");
	
	GtkWidget *cancelButton;
	cancelButton = gtk_button_new_with_label("Cancel");

	GtkWidget *grid;
	grid = gtk_grid_new();
	gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
	gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
	gtk_grid_set_column_spacing(GTK_GRID(grid), 30);
	gtk_grid_attach(GTK_GRID(grid), cancelButton, 1, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), addButton, 2, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), removeButton, 3, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), saveButton, 4, 1, 1, 1);

	GtkWidget *myGtkBox;
	myGtkBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	gtk_box_prepend(GTK_BOX(myGtkBox), myScrolledWindow);
	gtk_box_append(GTK_BOX(myGtkBox), grid);

	// callbacks (funtions) to run on signals that widgets emit
	g_signal_connect(myGtkListItemFactory, "setup", G_CALLBACK(setup_listitem), NULL);
	g_signal_connect(myGtkListItemFactory, "bind", G_CALLBACK(bind_listitem), NULL);
	g_signal_connect_swapped(cancelButton, "clicked", G_CALLBACK(gtk_window_close), window);
	g_signal_connect_swapped(saveButton, "clicked", G_CALLBACK(save_btn), window);
	g_signal_connect_swapped(addButton, "clicked", G_CALLBACK(add_btn), mySingleSelection);
	g_signal_connect(removeButton, "clicked", G_CALLBACK(remove_btn), mySingleSelection);

	gtk_window_set_child(GTK_WINDOW(window), myGtkBox);
	gtk_window_present(GTK_WINDOW(window));
}

int main() {
	populate_strings();

	int status;
	GtkApplication *app;
	app = gtk_application_new("com.github.DiogenesN.dioappfinder-settings",
											G_APPLICATION_DEFAULT_FLAGS);

	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	status = g_application_run(G_APPLICATION(app), 0, NULL);
	g_object_unref(app);

	return status;
}
