#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>

//  gcc $(pkg-config --cflags gtk4) -o mat matrix-app.c $(pkg-config --libs gtk4)

typedef struct Matrix {
    unsigned M;
    unsigned N;
    int *data;
} Matrix;

typedef struct {
    unsigned rows;
    unsigned cols;
    GtkWidget **entries;
    GtkWindow *input_window;
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

static void show_result_dialog(const char *message, GtkWindow *parent) {
    GtkWidget *dialog = gtk_message_dialog_new(parent,
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "%s", message);
    gtk_window_present(GTK_WINDOW(dialog));
    g_signal_connect(dialog, "response", G_CALLBACK(gtk_window_destroy), NULL);
}

static void on_calculate_clicked(GtkWidget *widget, gpointer data) {
    MatrixInputData *input_data = data;
    Matrix *matrix = create_matrix(input_data->rows, input_data->cols);
    
    if (!matrix) {
        show_result_dialog("Failed to create matrix", input_data->input_window);
        return;
    }

    for (unsigned i = 0; i < input_data->rows; i++) {
        for (unsigned j = 0; j < input_data->cols; j++) {
            GtkEntry *entry = GTK_ENTRY(input_data->entries[i * input_data->cols + j]);
            const char *text = gtk_entry_get_text(entry);
            int value = atoi(text);
            set_element(matrix, i, j, value);
        }
    }

    double det = determinant(matrix);
    char message[50];
    snprintf(message, 50, "Determinant: %.2f", det);
    show_result_dialog(message, input_data->input_window);

    gtk_window_destroy(input_data->input_window);
    free(matrix->data);
    free(matrix);
    free(input_data->entries);
    free(input_data);
}

static void create_input_dialog(int rows, int cols) {
    GtkWidget *input_window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(input_window), "Matrix Input");
    gtk_window_set_default_size(GTK_WINDOW(input_window), 400, 300);

    MatrixInputData *input_data = malloc(sizeof(MatrixInputData));
    input_data->rows = rows;
    input_data->cols = cols;
    input_data->entries = malloc(rows * cols * sizeof(GtkWidget*));
    input_data->input_window = GTK_WINDOW(input_window);

    GtkWidget *grid = gtk_grid_new();
    gtk_window_set_child(GTK_WINDOW(input_window), grid);

    for (unsigned i = 0; i < rows; i++) {
        for (unsigned j = 0; j < cols; j++) {
            GtkWidget *entry = gtk_entry_new();
            gtk_entry_set_text(GTK_ENTRY(entry), "0");
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
    if (response == GTK_RESPONSE_OK) {
        GtkEntry *entry = GTK_ENTRY(gtk_widget_get_first_child(GTK_WIDGET(dialog)));
        const char *text = gtk_entry_get_text(entry);
        int value = atoi(text);
        
        if (value <= 0) {
            show_result_dialog("Invalid dimension value", GTK_WINDOW(dialog));
            return;
        }
        
        int *counter = (int *)data;
        if (*counter == 0) {
            int *rows = malloc(sizeof(int));
            *rows = value;
            GtkWidget *cols_dialog = gtk_dialog_new();
            gtk_window_set_title(GTK_WINDOW(cols_dialog), "Enter Columns");
            GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(cols_dialog));
            GtkWidget *entry = gtk_entry_new();
            gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Columns");
            gtk_box_append(GTK_BOX(content), entry);
            gtk_window_present(GTK_WINDOW(cols_dialog));
            g_signal_connect(cols_dialog, "response", G_CALLBACK(on_dimension_response), rows);
        } else {
            create_input_dialog(*counter, value);
            free(counter);
        }
    }
    gtk_window_destroy(GTK_WINDOW(dialog));
}

static void on_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Enter Rows");
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
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);

    GtkWidget *button = gtk_button_new_with_label("Enter Matrix");
    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), NULL);
    gtk_window_set_child(GTK_WINDOW(window), button);

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("org.example.matrix", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}