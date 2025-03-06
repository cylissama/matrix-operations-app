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

static void on_calculate_clicked(GtkWidget *widget, gpointer data);

typedef struct Matrix {
    unsigned M;
    unsigned N;
    int *data;
    char name[10]; // For storing matrix name (A, B, C, etc.)
} Matrix;

// Matrix storage - global storage for saved matrices
#define MAX_SAVED_MATRICES 10
Matrix *saved_matrices[MAX_SAVED_MATRICES] = {NULL};
int num_saved_matrices = 0;

// Function prototypes
Matrix *create_matrix(unsigned M, unsigned N);
void set_element(Matrix *matrix, unsigned i, unsigned j, int value);
double determinant(Matrix *matrix);
Matrix *copy_matrix(Matrix *source);
void save_matrix(Matrix *matrix, const char *name);
Matrix *load_matrix(const char *name);
void save_matrices_to_file(const char *filename);
void load_matrices_from_file(const char *filename);

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
    GtkWidget *matrix_name_entry;
    GtkWidget *save_btn;
    GtkWidget *load_combo;
} MatrixInputData;

Matrix *create_matrix(unsigned M, unsigned N) {
    Matrix *matrix = malloc(sizeof(Matrix));
    if (!matrix) return NULL;

    matrix->M = M;
    matrix->N = N;
    matrix->data = malloc(M * N * sizeof(int));
    matrix->name[0] = '\0'; // Initialize name as empty

    if (!matrix->data) {
        free(matrix);
        return NULL;
    }
    return matrix;
}

void set_element(Matrix *matrix, unsigned i, unsigned j, int value) {
    matrix->data[i * matrix->N + j] = value;
}

Matrix *copy_matrix(Matrix *source) {
    if (!source) return NULL;
    
    Matrix *copy = create_matrix(source->M, source->N);
    if (!copy) return NULL;
    
    // Copy data
    memcpy(copy->data, source->data, source->M * source->N * sizeof(int));
    // Copy name
    strncpy(copy->name, source->name, sizeof(copy->name));
    
    return copy;
}

// Save matrix to global storage
void save_matrix(Matrix *matrix, const char *name) {
    if (!matrix || !name) return;
    
    // Check if matrix with this name already exists
    for (int i = 0; i < num_saved_matrices; i++) {
        if (saved_matrices[i] && strcmp(saved_matrices[i]->name, name) == 0) {
            // Replace existing matrix
            free(saved_matrices[i]->data);
            free(saved_matrices[i]);
            
            saved_matrices[i] = copy_matrix(matrix);
            strncpy(saved_matrices[i]->name, name, sizeof(saved_matrices[i]->name) - 1);
            return;
        }
    }
    
    // Add new matrix if we have space
    if (num_saved_matrices < MAX_SAVED_MATRICES) {
        saved_matrices[num_saved_matrices] = copy_matrix(matrix);
        strncpy(saved_matrices[num_saved_matrices]->name, name, sizeof(saved_matrices[num_saved_matrices]->name) - 1);
        num_saved_matrices++;
    }
}

// Load matrix from global storage
Matrix *load_matrix(const char *name) {
    if (!name) return NULL;
    
    for (int i = 0; i < num_saved_matrices; i++) {
        if (saved_matrices[i] && strcmp(saved_matrices[i]->name, name) == 0) {
            return copy_matrix(saved_matrices[i]);
        }
    }
    
    return NULL;
}

// Save all matrices to a file
void save_matrices_to_file(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) return;
    
    fprintf(file, "%d\n", num_saved_matrices);
    
    for (int i = 0; i < num_saved_matrices; i++) {
        Matrix *matrix = saved_matrices[i];
        fprintf(file, "%s %u %u\n", matrix->name, matrix->M, matrix->N);
        
        for (unsigned row = 0; row < matrix->M; row++) {
            for (unsigned col = 0; col < matrix->N; col++) {
                fprintf(file, "%d ", matrix->data[row * matrix->N + col]);
            }
            fprintf(file, "\n");
        }
    }
    
    fclose(file);
}

// Load all matrices from a file
void load_matrices_from_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return;
    
    // Clear existing matrices
    for (int i = 0; i < num_saved_matrices; i++) {
        if (saved_matrices[i]) {
            free(saved_matrices[i]->data);
            free(saved_matrices[i]);
            saved_matrices[i] = NULL;
        }
    }
    
    fscanf(file, "%d", &num_saved_matrices);
    
    for (int i = 0; i < num_saved_matrices; i++) {
        char name[10];
        unsigned M, N;
        fscanf(file, "%s %u %u", name, &M, &N);
        
        Matrix *matrix = create_matrix(M, N);
        strncpy(matrix->name, name, sizeof(matrix->name) - 1);
        
        for (unsigned row = 0; row < M; row++) {
            for (unsigned col = 0; col < N; col++) {
                int value;
                fscanf(file, "%d", &value);
                set_element(matrix, row, col, value);
            }
        }
        
        saved_matrices[i] = matrix;
    }
    
    fclose(file);
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

static void update_saved_matrices_combo(GtkDropDown *combo) {
    GtkStringList *list = GTK_STRING_LIST(gtk_drop_down_get_model(combo));
    guint count = g_list_model_get_n_items(G_LIST_MODEL(list));
    
    // Clear existing items - fixed to match API requirements
    if (count > 0) {
        // Use NULL for no additions instead of providing (0, NULL)
        gtk_string_list_splice(list, 0, count, NULL);
    }
    
    // Add new items
    for (int i = 0; i < num_saved_matrices; i++) {
        if (saved_matrices[i]) {
            gtk_string_list_append(list, saved_matrices[i]->name);
        }
    }
}

static void on_save_matrix_clicked(GtkWidget *widget, gpointer data) {
    MatrixInputData *input_data = data;
    
    if (!input_data || !input_data->matrix || !input_data->matrix_name_entry) {
        g_print("Error: Invalid matrix or name entry\n");
        return;
    }
    
    const char *name = gtk_editable_get_text(GTK_EDITABLE(input_data->matrix_name_entry));
    if (!name || strlen(name) == 0) {
        gtk_label_set_text(GTK_LABEL(input_data->result_label), "Please enter a matrix name");
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
    
    // Save the matrix
    save_matrix(input_data->matrix, name);
    
    // Update the combo box
    update_saved_matrices_combo(GTK_DROP_DOWN(input_data->load_combo));
    
    gtk_label_set_text(GTK_LABEL(input_data->result_label), "Matrix saved successfully");
}

static void on_load_matrix_clicked(GtkWidget *widget, gpointer data) {
    MatrixInputData *input_data = data;
    GtkDropDown *combo = GTK_DROP_DOWN(input_data->load_combo);
    guint pos = gtk_drop_down_get_selected(combo);
    
    if (pos == GTK_INVALID_LIST_POSITION) {
        gtk_label_set_text(GTK_LABEL(input_data->result_label), "Please select a matrix");
        return;
    }

    // Fixed: Removed duplicate code block, keeping only one version
    GtkStringObject *item = GTK_STRING_OBJECT(g_list_model_get_item(
        gtk_drop_down_get_model(combo), pos));
    const char *name = gtk_string_object_get_string(item);
    
    Matrix *loaded_matrix = load_matrix(name);
    if (!loaded_matrix) {
        gtk_label_set_text(GTK_LABEL(input_data->result_label), "Failed to load matrix");
        // Don't free name as it's owned by the string object
        g_object_unref(item);
        return;
    }
    
    // Free the old matrix if it exists
    if (input_data->matrix) {
        free(input_data->matrix->data);
        free(input_data->matrix);
    }
    
    input_data->matrix = loaded_matrix;
    
    // Update rows and cols entries
    char rows_str[10], cols_str[10];
    sprintf(rows_str, "%u", loaded_matrix->M);
    sprintf(cols_str, "%u", loaded_matrix->N);
    gtk_editable_set_text(GTK_EDITABLE(input_data->rows_entry), rows_str);
    gtk_editable_set_text(GTK_EDITABLE(input_data->cols_entry), cols_str);
    
    // Regenerate the matrix UI
    on_calculate_clicked(NULL, input_data);
    
    // Update the matrix entries with the loaded values
    for (unsigned i = 0; i < loaded_matrix->M; i++) {
        for (unsigned j = 0; j < loaded_matrix->N; j++) {
            char value_str[20];
            sprintf(value_str, "%d", loaded_matrix->data[i * loaded_matrix->N + j]);
            gtk_editable_set_text(GTK_EDITABLE(input_data->entries[i * loaded_matrix->N + j]), value_str);
        }
    }
    
    // Update the name entry
    gtk_editable_set_text(GTK_EDITABLE(input_data->matrix_name_entry), name);
    
    // Cleanup the string object reference
    g_object_unref(item);
    
    gtk_label_set_text(GTK_LABEL(input_data->result_label), "Matrix loaded successfully");
}

static void on_file_save_response(GtkFileDialog *dialog, GAsyncResult *result, gpointer user_data) {
    GFile *file = gtk_file_dialog_save_finish(dialog, result, NULL);
    if (file) {
        char *filename = g_file_get_path(file);
        save_matrices_to_file(filename);
        g_free(filename);
        g_object_unref(file);
    }
    g_object_unref(dialog);
}

static void on_file_save_clicked(GtkWidget *widget, gpointer data) {
    GtkWindow *window = GTK_WINDOW(data);
    GtkFileDialog *dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_initial_name(dialog, "matrices.dat");
    
    gtk_file_dialog_save(dialog, window, NULL, 
                       (GAsyncReadyCallback)on_file_save_response, NULL);
}

static void on_file_load_response(GtkFileDialog *dialog, GAsyncResult *result, gpointer user_data) {
    MatrixInputData *input_data = user_data;
    GFile *file = gtk_file_dialog_open_finish(dialog, result, NULL);
    
    if (file) {
        char *filename = g_file_get_path(file);
        load_matrices_from_file(filename);
        
        // Update the combo box
        update_saved_matrices_combo(GTK_DROP_DOWN(input_data->load_combo));
        
        gtk_label_set_text(GTK_LABEL(input_data->result_label), "Matrices loaded from file");
        g_free(filename);
        g_object_unref(file);
    }
    g_object_unref(dialog);
}

static void on_file_load_clicked(GtkWidget *widget, gpointer data) {
    MatrixInputData *input_data = data;
    GtkWindow *window = input_data->input_window;
    
    GtkFileDialog *dialog = gtk_file_dialog_new();
    gtk_file_dialog_open(dialog, window, NULL, 
                       (GAsyncReadyCallback)on_file_load_response, input_data);
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

    // Create a new matrix or use existing one with new dimensions
    if (input_data->matrix) {
        free(input_data->matrix->data);
        free(input_data->matrix);
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

    // Create Display Matrix button
    input_data->display_btn = gtk_button_new_with_label("Display Matrix");
    gtk_grid_attach(GTK_GRID(grid), input_data->display_btn, 0, rows, cols/2, 1);
    g_signal_connect(input_data->display_btn, "clicked", G_CALLBACK(on_display_matrix_clicked), input_data);

    // Create Determinant button only for square matrices
    if (rows == cols) {
        input_data->determinant_btn = gtk_button_new_with_label("Calculate Determinant");
        gtk_grid_attach(GTK_GRID(grid), input_data->determinant_btn, cols/2, rows, cols/2, 1);
        g_signal_connect(input_data->determinant_btn, "clicked", G_CALLBACK(on_determinant_clicked), input_data);
    }
    
    // Add saving controls - second row below matrix
    GtkWidget *save_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_grid_attach(GTK_GRID(grid), save_box, 0, rows+1, cols, 1);
    
    GtkWidget *name_label = gtk_label_new("Matrix Name:");
    gtk_box_append(GTK_BOX(save_box), name_label);
    
    input_data->matrix_name_entry = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(input_data->matrix_name_entry), 9);
    gtk_entry_set_placeholder_text(GTK_ENTRY(input_data->matrix_name_entry), "A, B, C, etc.");
    gtk_box_append(GTK_BOX(save_box), input_data->matrix_name_entry);
    
    input_data->save_btn = gtk_button_new_with_label("Save Matrix");
    g_signal_connect(input_data->save_btn, "clicked", G_CALLBACK(on_save_matrix_clicked), input_data);
    gtk_box_append(GTK_BOX(save_box), input_data->save_btn);
}

static void apply_css(GtkWidget *widget) {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider,
        "window { background-color: #F1EFE2; }\n"
        "entry { background-color: white; }\n"
        "button { background-color: #D9D7CC; border-radius: 4px; }\n"
        "button:hover { background-color: #C9C7BC; }\n");
    
    gtk_style_context_add_provider_for_display(
        gtk_widget_get_display(widget),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Matrix Calculator");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 600);
    
    // Apply the custom CSS to set the main background color
    apply_css(window);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_window_set_child(GTK_WINDOW(window), main_box);
    
    // Menu bar
    GtkWidget *menu_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_append(GTK_BOX(main_box), menu_bar);
    
    MatrixInputData *input_data = malloc(sizeof(MatrixInputData));
    memset(input_data, 0, sizeof(MatrixInputData));
    input_data->input_window = GTK_WINDOW(window);
    
    GtkWidget *file_save_btn = gtk_button_new_with_label("Save Matrices to File");
    g_signal_connect(file_save_btn, "clicked", G_CALLBACK(on_file_save_clicked), window);
    gtk_box_append(GTK_BOX(menu_bar), file_save_btn);
    
    GtkWidget *file_load_btn = gtk_button_new_with_label("Load Matrices from File");
    g_signal_connect(file_load_btn, "clicked", G_CALLBACK(on_file_load_clicked), input_data);
    gtk_box_append(GTK_BOX(menu_bar), file_load_btn);
    
    // Matrix selection area
    GtkWidget *load_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_append(GTK_BOX(main_box), load_box);
    
    GtkWidget *combo_label = gtk_label_new("Load Matrix:");
    gtk_box_append(GTK_BOX(load_box), combo_label);
    
    // Fix for the dropdown - create and cast properly
    GtkStringList *list = gtk_string_list_new(NULL);
    input_data->load_combo = gtk_drop_down_new(G_LIST_MODEL(list), NULL);
    gtk_box_append(GTK_BOX(load_box), input_data->load_combo);
    
    GtkWidget *load_btn = gtk_button_new_with_label("Load");
    g_signal_connect(load_btn, "clicked", G_CALLBACK(on_load_matrix_clicked), input_data);
    gtk_box_append(GTK_BOX(load_box), load_btn);
    
    // Matrix input controls
    GtkWidget *input_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_append(GTK_BOX(main_box), input_box);
    
    GtkWidget *rows_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *rows_label = gtk_label_new("Rows:");
    input_data->rows_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(input_data->rows_entry), "Enter rows");
    gtk_box_append(GTK_BOX(rows_box), rows_label);
    gtk_box_append(GTK_BOX(rows_box), input_data->rows_entry);
    gtk_box_append(GTK_BOX(input_box), rows_box);

    GtkWidget *cols_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *cols_label = gtk_label_new("Columns:");
    input_data->cols_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(input_data->cols_entry), "Enter columns");
    gtk_box_append(GTK_BOX(cols_box), cols_label);
    gtk_box_append(GTK_BOX(cols_box), input_data->cols_entry);
    gtk_box_append(GTK_BOX(input_box), cols_box);

    GtkWidget *generate_btn = gtk_button_new_with_label("Generate Matrix Input");
    g_signal_connect(generate_btn, "clicked", G_CALLBACK(on_calculate_clicked), input_data);
    gtk_box_append(GTK_BOX(input_box), generate_btn);

    GtkWidget *result_label = gtk_label_new("Results will be shown here");
    gtk_box_append(GTK_BOX(main_box), result_label);
    input_data->result_label = result_label;

    // Create a scrollable container for the matrix input grid
    GtkWidget *scroll_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scroll_window), 300);
    gtk_box_append(GTK_BOX(main_box), scroll_window);
    
    // Create a container for the matrix input grid
    GtkWidget *matrix_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll_window), matrix_container);
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