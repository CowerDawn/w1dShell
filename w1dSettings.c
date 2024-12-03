#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CONFIG_FILE "usconf.conf"

typedef struct {
    GtkWidget *window;
    GtkWidget *user_combo;
    GtkWidget *rootc_combo;
    GtkWidget *command_entry;
    GtkWidget *save_button;
} AppWidgets;

void save_config(AppWidgets *widgets) {
    const gchar *user = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widgets->user_combo));
    const gchar *rootc = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widgets->rootc_combo));
    const gchar *command = gtk_entry_get_text(GTK_ENTRY(widgets->command_entry));

    FILE *file = fopen(CONFIG_FILE, "w");
    if (file == NULL) {
        g_print("Error opening file for writing.\n");
        return;
    }

    fprintf(file, "user=\"%s\"\n", user);
    fprintf(file, "rootc=\"%s\"\n", rootc);
    if (strlen(command) > 0) {
        fprintf(file, "command=\"%s\"\n", command);
    }

    fclose(file);
    g_print("Configuration saved.\n");
}

void on_save_clicked(GtkWidget *widget, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    save_config(widgets);
}

void populate_user_combo(GtkWidget *combo) {
    FILE *fp;
    char path[1035];

    fp = popen("getent passwd | cut -d: -f1", "r");
    if (fp == NULL) {
        g_print("Failed to run command\n");
        return;
    }

    while (fgets(path, sizeof(path), fp) != NULL) {
        path[strcspn(path, "\n")] = 0;
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), path);
    }

    pclose(fp);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    AppWidgets widgets;

    widgets.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(widgets.window), "w1dSettings");
    gtk_container_set_border_width(GTK_CONTAINER(widgets.window), 10);
    gtk_widget_set_size_request(widgets.window, 300, 200);

    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(widgets.window), grid);

    GtkWidget *user_label = gtk_label_new("User:");
    widgets.user_combo = gtk_combo_box_text_new();
    populate_user_combo(widgets.user_combo);
    gtk_grid_attach(GTK_GRID(grid), user_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), widgets.user_combo, 1, 0, 1, 1);

    GtkWidget *rootc_label = gtk_label_new("Root Rights:");
    widgets.rootc_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets.rootc_combo), "off");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets.rootc_combo), "on");
    gtk_combo_box_set_active(GTK_COMBO_BOX(widgets.rootc_combo), 0);
    gtk_grid_attach(GTK_GRID(grid), rootc_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), widgets.rootc_combo, 1, 1, 1, 1);

    GtkWidget *command_label = gtk_label_new("Right Commands:");
    widgets.command_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), command_label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), widgets.command_entry, 1, 2, 1, 1);

    widgets.save_button = gtk_button_new_with_label("Save");
    g_signal_connect(widgets.save_button, "clicked", G_CALLBACK(on_save_clicked), &widgets);
    gtk_grid_attach(GTK_GRID(grid), widgets.save_button, 1, 3, 1, 1);

    g_signal_connect(widgets.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(widgets.window);

    gtk_main();

    return 0;
}
