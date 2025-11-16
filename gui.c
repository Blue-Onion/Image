#include "gui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void update_status(AppData* app_data, const char* message) {
    if (app_data && app_data->status_label) {
        gtk_label_set_text(GTK_LABEL(app_data->status_label), message);
    }
}

void load_image_preview(AppData* app_data, Image* img, GtkWidget* preview) {
    if (!img || !img->data || !preview) return;
    
    // Determine if image has alpha channel
    gboolean has_alpha = (img->channels == 4);
    
    // Create GdkPixbuf from image data
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data(
        img->data,
        GDK_COLORSPACE_RGB,
        has_alpha,  // has_alpha
        8,          // bits_per_sample
        img->width,
        img->height,
        img->width * img->channels,
        NULL,
        NULL
    );
    
    if (pixbuf) {
        // Scale if too large for preview
        int max_width = 300;
        int max_height = 300;
        
        if (img->width > max_width || img->height > max_height) {
            double scale_x = (double)max_width / img->width;
            double scale_y = (double)max_height / img->height;
            double scale = (scale_x < scale_y) ? scale_x : scale_y;
            
            GdkPixbuf* scaled = gdk_pixbuf_scale_simple(
                pixbuf,
                img->width * scale,
                img->height * scale,
                GDK_INTERP_BILINEAR
            );
            g_object_unref(pixbuf);
            pixbuf = scaled;
        }
        
        gtk_image_set_from_pixbuf(GTK_IMAGE(preview), pixbuf);
        g_object_unref(pixbuf);
    }
}

void on_file_selected(GtkFileChooserButton* button, gpointer data) {
    AppData* app_data = (AppData*)data;
    
    gchar* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(button));
    if (!filename) return;
    
    // Free previous image
    if (app_data->current_image) {
        image_free(app_data->current_image);
        app_data->current_image = NULL;
    }
    
    // Load new image
    update_status(app_data, "Loading image...");
    app_data->current_image = image_load(filename);
    
    if (app_data->current_image) {
        char status[256];
        snprintf(status, sizeof(status), 
                "Loaded: %dx%d (%d channels)",
                app_data->current_image->width,
                app_data->current_image->height,
                app_data->current_image->channels);
        update_status(app_data, status);
        
        // Load preview
        if (app_data->original_image) {
            // Convert to RGB/RGBA format that GdkPixbuf expects
            Image* display_img = app_data->current_image;
            int needs_conversion = 0;
            
            if (app_data->current_image->channels == 1) {
                // Convert grayscale to RGB
                display_img = image_create(app_data->current_image->width,
                                          app_data->current_image->height, 3);
                if (display_img) {
                    needs_conversion = 1;
                    for (int i = 0; i < app_data->current_image->width * 
                                     app_data->current_image->height; i++) {
                        uint8_t gray = app_data->current_image->data[i];
                        display_img->data[i * 3 + 0] = gray;
                        display_img->data[i * 3 + 1] = gray;
                        display_img->data[i * 3 + 2] = gray;
                    }
                }
            } else if (app_data->current_image->channels == 4) {
                // Handle RGBA - GdkPixbuf can handle this directly
                // No conversion needed
            } else if (app_data->current_image->channels == 3) {
                // RGB - perfect, no conversion needed
            }
            
            if (display_img) {
                load_image_preview(app_data, display_img, app_data->original_image);
                if (needs_conversion) {
                    image_free(display_img);
                }
            }
        }
        
        gtk_widget_set_sensitive(app_data->compress_button, TRUE);
    } else {
        update_status(app_data, "Error: Failed to load image");
        gtk_widget_set_sensitive(app_data->compress_button, FALSE);
    }
    
    g_free(filename);
}

void on_compress_clicked(GtkWidget* widget, gpointer data) {
    AppData* app_data = (AppData*)data;
    
    if (!app_data->current_image) {
        update_status(app_data, "Error: No image loaded");
        return;
    }
    
    // Get threshold value
    uint8_t threshold = (uint8_t)gtk_spin_button_get_value_as_int(
        GTK_SPIN_BUTTON(app_data->threshold_spin));
    
    // Get output filename
    const char* output_file = gtk_entry_get_text(GTK_ENTRY(app_data->output_file_entry));
    if (!output_file || strlen(output_file) == 0) {
        update_status(app_data, "Error: Please specify output filename");
        return;
    }
    
    update_status(app_data, "Compressing image...");
    gtk_widget_set_sensitive(app_data->compress_button, FALSE);
    
    // Free previous sparse matrices
    if (app_data->current_sparse && app_data->current_channels > 0) {
        for (int i = 0; i < app_data->current_channels; i++) {
            if (app_data->current_sparse[i]) {
                sparse_matrix_free(app_data->current_sparse[i]);
            }
        }
        free(app_data->current_sparse);
        app_data->current_sparse = NULL;
    }
    
    // Convert to sparse matrices
    app_data->current_sparse = image_to_sparse_matrices(
        app_data->current_image, threshold);
    
    if (!app_data->current_sparse) {
        update_status(app_data, "Error: Failed to compress image");
        gtk_widget_set_sensitive(app_data->compress_button, TRUE);
        return;
    }
    
    app_data->current_channels = app_data->current_image->channels;
    
    // Calculate compression ratio
    float ratio = calculate_total_compression_ratio(
        app_data->current_sparse,
        app_data->current_channels,
        app_data->current_image->width,
        app_data->current_image->height);
    
    // Convert back to image
    Image* compressed_img = sparse_matrices_to_image(
        app_data->current_sparse,
        app_data->current_channels);
    
    if (!compressed_img) {
        update_status(app_data, "Error: Failed to decompress image");
        gtk_widget_set_sensitive(app_data->compress_button, TRUE);
        return;
    }
    
    // Save compressed image
    if (image_save(compressed_img, output_file)) {
        char status[256];
        snprintf(status, sizeof(status),
                "Saved! Compression ratio: %.2f:1 (Original: %d KB, Compressed: %d KB)",
                ratio,
                (app_data->current_image->width * app_data->current_image->height * 
                 app_data->current_image->channels) / 1024,
                (sparse_matrix_get_size_bytes(app_data->current_sparse[0]) * 
                 app_data->current_channels) / 1024);
        update_status(app_data, status);
        
        // Update compression ratio label
        char ratio_text[128];
        snprintf(ratio_text, sizeof(ratio_text), "Compression Ratio: %.2f:1", ratio);
        gtk_label_set_text(GTK_LABEL(app_data->compression_ratio_label), ratio_text);
        
        // Load compressed image preview
        if (app_data->compressed_image) {
            load_image_preview(app_data, compressed_img, app_data->compressed_image);
        }
    } else {
        update_status(app_data, "Error: Failed to save compressed image");
    }
    
    image_free(compressed_img);
    gtk_widget_set_sensitive(app_data->compress_button, TRUE);
}

void on_activate(GtkApplication* app, gpointer user_data) {
    AppData* app_data = (AppData*)calloc(1, sizeof(AppData));
    if (!app_data) return;
    
    // Create main window
    app_data->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(app_data->window), "Image Compressor - Sparse Matrix");
    gtk_window_set_default_size(GTK_WINDOW(app_data->window), 800, 600);
    
    // Create main container
    GtkWidget* main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(app_data->window), main_box);
    gtk_container_set_border_width(GTK_CONTAINER(main_box), 10);
    
    // File selection section
    GtkWidget* file_frame = gtk_frame_new("Input Image");
    gtk_box_pack_start(GTK_BOX(main_box), file_frame, FALSE, FALSE, 0);
    
    GtkWidget* file_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_add(GTK_CONTAINER(file_frame), file_box);
    gtk_container_set_border_width(GTK_CONTAINER(file_box), 10);
    
    app_data->input_file_chooser = gtk_file_chooser_button_new(
        "Select Image File",
        GTK_FILE_CHOOSER_ACTION_OPEN);
    
    // Set file filter for images
    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Image Files");
    gtk_file_filter_add_pattern(filter, "*.png");
    gtk_file_filter_add_pattern(filter, "*.jpg");
    gtk_file_filter_add_pattern(filter, "*.jpeg");
    gtk_file_filter_add_pattern(filter, "*.bmp");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(app_data->input_file_chooser), filter);
    
    g_signal_connect(app_data->input_file_chooser, "file-set",
                    G_CALLBACK(on_file_selected), app_data);
    
    gtk_box_pack_start(GTK_BOX(file_box), app_data->input_file_chooser, TRUE, TRUE, 0);
    
    // Output file entry
    GtkWidget* output_frame = gtk_frame_new("Output File");
    gtk_box_pack_start(GTK_BOX(main_box), output_frame, FALSE, FALSE, 0);
    
    GtkWidget* output_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_add(GTK_CONTAINER(output_frame), output_box);
    gtk_container_set_border_width(GTK_CONTAINER(output_box), 10);
    
    gtk_box_pack_start(GTK_BOX(output_box), gtk_label_new("Output:"), FALSE, FALSE, 0);
    
    app_data->output_file_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(app_data->output_file_entry), "compressed_output.png");
    gtk_box_pack_start(GTK_BOX(output_box), app_data->output_file_entry, TRUE, TRUE, 0);
    
    // Settings frame
    GtkWidget* settings_frame = gtk_frame_new("Compression Settings");
    gtk_box_pack_start(GTK_BOX(main_box), settings_frame, FALSE, FALSE, 0);
    
    GtkWidget* settings_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_add(GTK_CONTAINER(settings_frame), settings_box);
    gtk_container_set_border_width(GTK_CONTAINER(settings_box), 10);
    
    gtk_box_pack_start(GTK_BOX(settings_box), 
                      gtk_label_new("Threshold (0-255):"), FALSE, FALSE, 0);
    
    app_data->threshold_spin = gtk_spin_button_new_with_range(0, 255, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(app_data->threshold_spin), 10);
    gtk_box_pack_start(GTK_BOX(settings_box), app_data->threshold_spin, FALSE, FALSE, 0);
    
    // Compression button
    app_data->compress_button = gtk_button_new_with_label("Compress Image");
    gtk_widget_set_sensitive(app_data->compress_button, FALSE);
    g_signal_connect(app_data->compress_button, "clicked",
                    G_CALLBACK(on_compress_clicked), app_data);
    gtk_box_pack_start(GTK_BOX(main_box), app_data->compress_button, FALSE, FALSE, 0);
    
    // Images preview section
    GtkWidget* preview_frame = gtk_frame_new("Image Preview");
    gtk_box_pack_start(GTK_BOX(main_box), preview_frame, TRUE, TRUE, 0);
    
    GtkWidget* preview_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_add(GTK_CONTAINER(preview_frame), preview_box);
    gtk_container_set_border_width(GTK_CONTAINER(preview_box), 10);
    
    // Original image preview
    GtkWidget* original_frame = gtk_frame_new("Original");
    gtk_box_pack_start(GTK_BOX(preview_box), original_frame, TRUE, TRUE, 0);
    app_data->original_image = gtk_image_new();
    gtk_container_add(GTK_CONTAINER(original_frame), app_data->original_image);
    
    // Compressed image preview
    GtkWidget* compressed_frame = gtk_frame_new("Compressed");
    gtk_box_pack_start(GTK_BOX(preview_box), compressed_frame, TRUE, TRUE, 0);
    app_data->compressed_image = gtk_image_new();
    gtk_container_add(GTK_CONTAINER(compressed_frame), app_data->compressed_image);
    
    // Status and compression ratio
    app_data->status_label = gtk_label_new("Ready - Select an image to begin");
    gtk_box_pack_start(GTK_BOX(main_box), app_data->status_label, FALSE, FALSE, 0);
    
    app_data->compression_ratio_label = gtk_label_new("Compression Ratio: N/A");
    gtk_box_pack_start(GTK_BOX(main_box), app_data->compression_ratio_label, FALSE, FALSE, 0);
    
    // Store app_data in window
    g_object_set_data(G_OBJECT(app_data->window), "app_data", app_data);
    
    // Cleanup on destroy
    g_signal_connect(app_data->window, "destroy",
                    G_CALLBACK(gtk_main_quit), NULL);
    
    gtk_widget_show_all(app_data->window);
}

void free_app_resources(AppData* app_data) {
    if (app_data) {
        if (app_data->current_image) {
            image_free(app_data->current_image);
        }
        
        if (app_data->current_sparse && app_data->current_channels > 0) {
            for (int i = 0; i < app_data->current_channels; i++) {
                if (app_data->current_sparse[i]) {
                    sparse_matrix_free(app_data->current_sparse[i]);
                }
            }
            free(app_data->current_sparse);
        }
        
        free(app_data);
    }
}

