#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>

typedef struct Matrix {
    unsigned M;
    unsigned N;
    int *data;
} Matrix;

typedef struct {
    unsigned rows;
    unsigned cols;
    GtkWidget **entries;
    GtkWidget *result_label;
    GtkWindow *input_window;
    Matrix *matrix;  // Store the matrix for later display
} MatrixInputData;

Matrix *create_matrix(unsigned M, unsigned N) {
    Matrix *matrix = malloc(sizeof(Matrix));
    if (!matrix) return NULL;

    matrix->M = M;
    matrix->N = N;
    matrix->data = malloc(M * N * sizeof(int));

    if (!matrix->data) {
        free(matrix);
        return NULL;
    }
    return matrix;
}

void set_element(Matrix *matrix, unsigned i, unsigned j, int value) {
    matrix->data[i * matrix->N + j] = value;
}

double determinant(Matrix *matrix) {
    if (matrix->M != matrix->N) return 0.0;
    unsigned n = matrix->M;
    double *data = malloc(n * n * sizeof(double));
    if (!data) return 0.0;

    for (unsigned i = 0; i < n * n; i++)
        data[i] = matrix->data[i];

    int sign = 1;
    for (unsigned k = 0; k < n; k++) {
        unsigned max_row = k;
        for (unsigned i = k + 1; i < n; i++)
            if (fabs(data[i*n +k]) > fabs(data[max_row*n +k]))
                max_row = i;

        if (max_row != k) {
            for (unsigned j = 0; j < n; j++) {
                double tmp = data[k*n +j];
                data[k*n +j] = data[max_row*n +j];
                data[max_row*n +j] = tmp;
            }
            sign *= -1;
        }

        if (data[k*n +k] == 0.0) {
            free(data);
            return 0.0;
        }

        for (unsigned i = k + 1; i < n; i++) {
            double factor = data[i*n +k] / data[k*n +k];
            data[i*n +k] = factor;
            for (unsigned j = k + 1; j < n; j++)
                data[i*n +j] -= factor * data[k*n +j];
        }
    }

    double det = 1.0;
    for (unsigned k = 0; k < n; k++)
        det *= data[k*n +k];
    det *= sign;

    free(data);
    return det;
}

static void on_display_matrix_clicked(GtkWidget *widget, gpointer data) {
    MatrixInputData *input_data = data;
    Matrix *matrix = input_data->matrix;
    
    if (!matrix) {
        gtk_label_set_text(GTK_LABEL(input_data->result_label), "No matrix to display");
        return;
    }

    // Create a string to hold the matrix representation
    GString *matrix_str = g_string_new("Matrix:\n");
    
    for (unsigned i = 0; i < matrix->M; i++) {
        for (unsigned j = 0; j < matrix->N; j++) {
            g_string_append_printf(matrix_str, "%d ", matrix->data[i * matrix->N + j]);
        }
        g_string_append(matrix_str, "\n");
    }

    // Display the matrix in the label
    gtk_label_set_text(GTK_LABEL(input_data->result_label), matrix_str->str);
    
    // Free the string
    g_string_free(matrix_str, TRUE);
}

static void on_calculate_clicked(GtkWidget *widget, gpointer data) {
    MatrixInputData *input_data = data;
    Matrix *matrix = create_matrix(input_data->rows, input_data->cols);
    
    if (!matrix) {
        gtk_label_set_text(GTK_LABEL(input_data->result_label), "Failed to create matrix");
        return;
    }

    for (unsigned i = 0; i < input_data->rows; i++) {
        for (unsigned j = 0; j < input_data->cols; j++) {
            GtkEntry *entry = GTK_ENTRY(input_data->entries[i * input_data->cols + j]);
            const char *text = gtk_editable_get_text(GTK_EDITABLE(entry));
            int value = atoi(text);
            set_element(matrix, i, j, value);
        }
    }

    // Store the matrix for later display
    input_data->matrix = matrix;

    double det = determinant(matrix);
    char message[50];
    snprintf(message, 50, "Determinant: %.2f", det);
    gtk_label_set_text(GTK_LABEL(input_data->result_label), message);

    // Show the display matrix button
    GtkWidget *display_btn = gtk_button_new_with_label("Display Matrix");
    gtk_grid_attach(GTK_GRID(gtk_window_get_child(input_data->input_window)), 
                    display_btn, 0, input_data->rows + 1, input_data->cols, 1);
    g_signal_connect(display_btn, "clicked", G_CALLBACK(on_display_matrix_clicked), input_data);
    gtk_widget_set_visible(display_btn, TRUE);
}

static void create_input_dialog(int rows, int cols, GtkWindow *parent, GtkWidget *result_label) {
    GtkWidget *input_window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(input_window), "Matrix Input");
    gtk_window_set_default_size(GTK_WINDOW(input_window), 400, 300);
    
    // Set the transient parent
    if (parent) {
        gtk_window_set_transient_for(GTK_WINDOW(input_window), parent);
    }

    MatrixInputData *input_data = malloc(sizeof(MatrixInputData));
    input_data->rows = rows;
    input_data->cols = cols;
    input_data->entries = malloc(rows * cols * sizeof(GtkWidget*));
    input_data->input_window = GTK_WINDOW(input_window);
    input_data->matrix = NULL;  // Initialize matrix pointer to NULL
    input_data->result_label = result_label;

    GtkWidget *grid = gtk_grid_new();
    gtk_window_set_child(GTK_WINDOW(input_window), grid);

    for (unsigned i = 0; i < rows; i++) {
        for (unsigned j = 0; j < cols; j++) {
            GtkWidget *entry = gtk_entry_new();
            gtk_editable_set_text(GTK_EDITABLE(entry), "0");
            gtk_grid_attach(GTK_GRID(grid), entry, j, i, 1, 1);
            input_data->entries[i * cols + j] = entry;
        }
    }

    GtkWidget *calc_btn = gtk_button_new_with_label("Calculate");
    gtk_grid_attach(GTK_GRID(grid), calc_btn, 0, rows, cols, 1);
    g_signal_connect(calc_btn, "clicked", G_CALLBACK(on_calculate_clicked), input_data);

    gtk_window_present(GTK_WINDOW(input_window));
}

static void on_dimension_response(GtkWidget *dialog, int response, gpointer data) {
    GtkWindow *parent = gtk_window_get_transient_for(GTK_WINDOW(dialog));
    GtkWidget *result_label = GTK_WIDGET(g_object_get_data(G_OBJECT(parent), "result_label"));
    
    if (response == GTK_RESPONSE_OK) {
        GtkEntry *entry = GTK_ENTRY(gtk_widget_get_first_child(GTK_WIDGET(dialog)));
        const char *text = gtk_editable_get_text(GTK_EDITABLE(entry));
        int value = atoi(text);
        
        if (value <= 0) {
            gtk_label_set_text(GTK_LABEL(result_label), "Invalid dimension value");
            return;
        }
        
        int *counter = (int *)data;
        if (*counter == 0) {
            int *rows = malloc(sizeof(int));
            *rows = value;
            GtkWidget *cols_dialog = gtk_dialog_new();
            gtk_window_set_title(GTK_WINDOW(cols_dialog), "Enter Columns");
            
            // Set the transient parent
            gtk_window_set_transient_for(GTK_WINDOW(cols_dialog), parent);
            
            GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(cols_dialog));
            GtkWidget *entry = gtk_entry_new();
            gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Columns");
            gtk_box_append(GTK_BOX(content), entry);
            gtk_window_present(GTK_WINDOW(cols_dialog));
            g_signal_connect(cols_dialog, "response", G_CALLBACK(on_dimension_response), rows);
        } else {
            create_input_dialog(*counter, value, parent, result_label);
            free(counter);
        }
    }
    gtk_window_destroy(GTK_WINDOW(dialog));
}

static void on_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_root(widget));
    GtkWidget *result_label = GTK_WIDGET(g_object_get_data(G_OBJECT(parent), "result_label"));
    
    GtkWidget *dialog = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Enter Rows");
    
    // Set the transient parent
    gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
    
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Rows");
    gtk_box_append(GTK_BOX(content), entry);
    gtk_window_present(GTK_WINDOW(dialog));
    
    int *counter = malloc(sizeof(int));
    *counter = 0;
    g_signal_connect(dialog, "response", G_CALLBACK(on_dimension_response), counter);
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Matrix Determinant");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

    // Create a vertical box
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_window_set_child(GTK_WINDOW(window), box);

    // Create a button to enter matrix
    GtkWidget *button = gtk_button_new_with_label("Enter Matrix");
    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), NULL);
    gtk_box_append(GTK_BOX(box), button);

    // Create a label to display results
    GtkWidget *result_label = gtk_label_new("Results will be shown here");
    gtk_box_append(GTK_BOX(box), result_label);

    // Store the result label as object data
    g_object_set_data(G_OBJECT(window), "result_label", result_label);

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("org.example.matrix", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}