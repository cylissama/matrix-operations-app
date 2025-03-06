/*
Cy Dixon
Matrix Calculation App
2025

Uses GTK for UI elements
Compile using: gcc $(pkg-config --cflags gtk4) -o mat matrix-app.c $(pkg-config --libs gtk4)
*/

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

// Function prototypes
Matrix *create_matrix(unsigned M, unsigned N);
void set_element(Matrix *matrix, unsigned i, unsigned j, int value);
double determinant(Matrix *matrix);

typedef struct {
    unsigned rows;
    unsigned cols;
    GtkWidget **entries;
    GtkWidget *result_label;
    GtkWindow *input_window;
    Matrix *matrix;
    GtkWidget *rows_entry;
    GtkWidget *cols_entry;
    GtkWidget *display_btn;
    GtkWidget *determinant_btn;
    GtkWidget *matrix_container;
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

static void on_determinant_clicked(GtkWidget *widget, gpointer data) {
    MatrixInputData *input_data = data;
    
    if (!input_data || !input_data->matrix || !GTK_IS_LABEL(input_data->result_label)) {
        g_print("Error: Invalid matrix or result label\n");
        return;
    }

    // Recalculate matrix values from current entries to ensure it's up to date
    for (unsigned i = 0; i < input_data->rows; i++) {
        for (unsigned j = 0; j < input_data->cols; j++) {
            GtkEntry *entry = GTK_ENTRY(input_data->entries[i * input_data->cols + j]);
            const char *text = gtk_editable_get_text(GTK_EDITABLE(entry));
            int value = atoi(text);
            set_element(input_data->matrix, i, j, value);
        }
    }

    // Check if matrix is square for determinant calculation
    if (input_data->matrix->M != input_data->matrix->N) {
        gtk_label_set_text(GTK_LABEL(input_data->result_label), "Determinant requires a square matrix");
        return;
    }

    double det = determinant(input_data->matrix);
    char message[100];
    snprintf(message, sizeof(message), "Determinant: %.2f", det);
    gtk_label_set_text(GTK_LABEL(input_data->result_label), message);
}

static void on_display_matrix_clicked(GtkWidget *widget, gpointer data) {
    MatrixInputData *input_data = data;
    
    if (!input_data || !input_data->result_label || !GTK_IS_LABEL(input_data->result_label)) {
        g_print("Error: Invalid result label or input data\n");
        return;
    }
    
    if (!input_data->matrix) {
        gtk_label_set_text(GTK_LABEL(input_data->result_label), "No matrix to display");
        return;
    }

    // UPDATE MATRIX VALUES FROM CURRENT ENTRIES
    for (unsigned i = 0; i < input_data->rows; i++) {
        for (unsigned j = 0; j < input_data->cols; j++) {
            GtkEntry *entry = GTK_ENTRY(input_data->entries[i * input_data->cols + j]);
            const char *text = gtk_editable_get_text(GTK_EDITABLE(entry));
            int value = atoi(text);
            set_element(input_data->matrix, i, j, value);
        }
    }

    GString *matrix_str = g_string_new("Matrix:\n");
    for (unsigned i = 0; i < input_data->matrix->M; i++) {
        for (unsigned j = 0; j < input_data->matrix->N; j++) {
            g_string_append_printf(matrix_str, "%d ", input_data->matrix->data[i * input_data->matrix->N + j]);
        }
        g_string_append(matrix_str, "\n");
    }

    gtk_label_set_text(GTK_LABEL(input_data->result_label), matrix_str->str);
    g_string_free(matrix_str, TRUE);
}

static void on_calculate_clicked(GtkWidget *widget, gpointer data) {
    MatrixInputData *input_data = data;
    
    if (!GTK_IS_LABEL(input_data->result_label)) {
        g_print("Error: Result label is invalid\n");
        return;
    }
    
    const char *rows_text = gtk_editable_get_text(GTK_EDITABLE(input_data->rows_entry));
    const char *cols_text = gtk_editable_get_text(GTK_EDITABLE(input_data->cols_entry));
    
    int rows = atoi(rows_text);
    int cols = atoi(cols_text);
    
    if (rows <= 0 || cols <= 0) {
        gtk_label_set_text(GTK_LABEL(input_data->result_label), "Invalid matrix dimensions");
        return;
    }

    if (input_data->entries) {
        free(input_data->entries);
        input_data->entries = NULL;
    }

    // Clear previous matrix container content
    GtkWidget *child;
    while ((child = gtk_widget_get_first_child(input_data->matrix_container))) {
        gtk_box_remove(GTK_BOX(input_data->matrix_container), child);
    }

    GtkWidget *grid = gtk_grid_new();
    gtk_box_append(GTK_BOX(input_data->matrix_container), grid);

    input_data->entries = malloc(rows * cols * sizeof(GtkWidget*));
    input_data->rows = rows;
    input_data->cols = cols;

    for (unsigned i = 0; i < rows; i++) {
        for (unsigned j = 0; j < cols; j++) {
            GtkWidget *entry = gtk_entry_new();
            gtk_editable_set_text(GTK_EDITABLE(entry), "0");
            gtk_grid_attach(GTK_GRID(grid), entry, j, i, 1, 1);
            input_data->entries[i * cols + j] = entry;
        }
    }

    Matrix *matrix = create_matrix(rows, cols);
    
    if (!matrix) {
        gtk_label_set_text(GTK_LABEL(input_data->result_label), "Failed to create matrix");
        return;
    }

    for (unsigned i = 0; i < rows; i++) {
        for (unsigned j = 0; j < cols; j++) {
            GtkEntry *entry = GTK_ENTRY(input_data->entries[i * cols + j]);
            const char *text = gtk_editable_get_text(GTK_EDITABLE(entry));
            int value = atoi(text);
            set_element(matrix, i, j, value);
        }
    }

    input_data->matrix = matrix;

    double det = determinant(matrix);
    char message[50];
    snprintf(message, 50, "Determinant: %.2f", det);
    gtk_label_set_text(GTK_LABEL(input_data->result_label), message);

    // Remove previous buttons if they exist
    if (input_data->display_btn) {
        gtk_grid_remove(GTK_GRID(grid), input_data->display_btn);
    }
    if (input_data->determinant_btn) {
        gtk_grid_remove(GTK_GRID(grid), input_data->determinant_btn);
    }

    // Create Display Matrix button
    input_data->display_btn = gtk_button_new_with_label("Display Matrix");
    gtk_grid_attach(GTK_GRID(grid), input_data->display_btn, 0, rows, cols/2, 1);
    g_signal_connect(input_data->display_btn, "clicked", G_CALLBACK(on_display_matrix_clicked), input_data);
    gtk_widget_set_visible(input_data->display_btn, TRUE);

    // Create Determinant button only for square matrices
    if (rows == cols) {
        input_data->determinant_btn = gtk_button_new_with_label("Calculate Determinant");
        gtk_grid_attach(GTK_GRID(grid), input_data->determinant_btn, cols/2, rows, cols/2, 1);
        g_signal_connect(input_data->determinant_btn, "clicked", G_CALLBACK(on_determinant_clicked), input_data);
        gtk_widget_set_visible(input_data->determinant_btn, TRUE);
    }
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Matrix Determinant");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 500);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_window_set_child(GTK_WINDOW(window), box);

    MatrixInputData *input_data = malloc(sizeof(MatrixInputData));
    memset(input_data, 0, sizeof(MatrixInputData));

    GtkWidget *rows_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *rows_label = gtk_label_new("Rows:");
    input_data->rows_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(input_data->rows_entry), "Enter rows");
    gtk_box_append(GTK_BOX(rows_box), rows_label);
    gtk_box_append(GTK_BOX(rows_box), input_data->rows_entry);
    gtk_box_append(GTK_BOX(box), rows_box);

    GtkWidget *cols_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *cols_label = gtk_label_new("Columns:");
    input_data->cols_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(input_data->cols_entry), "Enter columns");
    gtk_box_append(GTK_BOX(cols_box), cols_label);
    gtk_box_append(GTK_BOX(cols_box), input_data->cols_entry);
    gtk_box_append(GTK_BOX(box), cols_box);

    GtkWidget *generate_btn = gtk_button_new_with_label("Generate Matrix Input");
    g_signal_connect(generate_btn, "clicked", G_CALLBACK(on_calculate_clicked), input_data);
    gtk_box_append(GTK_BOX(box), generate_btn);

    GtkWidget *result_label = gtk_label_new("Results will be shown here");
    gtk_box_append(GTK_BOX(box), result_label);
    input_data->result_label = result_label;
    input_data->input_window = GTK_WINDOW(window);

    // Create a container for the matrix input grid and display button
    GtkWidget *matrix_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_append(GTK_BOX(box), matrix_container);
    input_data->matrix_container = matrix_container;

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("org.example.matrix", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}