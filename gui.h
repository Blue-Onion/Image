#ifndef GUI_H
#define GUI_H

#include <gtk/gtk.h>
#include "image_processor.h"
#include "sparse_matrix.h"

// Global variables for GUI
typedef struct {
    GtkWidget* window;
    GtkWidget* input_file_chooser;
    GtkWidget* output_file_entry;
    GtkWidget* threshold_spin;
    GtkWidget* status_label;
    GtkWidget* compress_button;
    GtkWidget* original_image;
    GtkWidget* compressed_image;
    GtkWidget* compression_ratio_label;
    Image* current_image;
    SparseMatrix** current_sparse;
    int current_channels;
} AppData;

// Function declarations
void on_activate(GtkApplication* app, gpointer user_data);
void on_compress_clicked(GtkWidget* widget, gpointer data);
void on_file_selected(GtkFileChooserButton* button, gpointer data);
void load_image_preview(AppData* app_data, Image* img, GtkWidget* preview);
void update_status(AppData* app_data, const char* message);
void free_app_resources(AppData* app_data);

#endif // GUI_H

