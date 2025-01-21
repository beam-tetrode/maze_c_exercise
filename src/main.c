#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERROR_STR_LEN 64
#define READ_CAPACITY 256

typedef struct x_y_type {
    int x;
    int y;
    int x_len;
    int y_len;
} x_y;

typedef struct matrix_type {
    char **matrix;
    int line_count;
    int line_length;
} matrix;

// A maze is a:
// * Line that starts and ends with #- or E-character.
// * Rectangular, same lenght lines.
// * Only contains #-, E-, ^- and underscore characters
// * Has multiple exits
// * Only one input
// * First and last row contains only #-, ^- and E-characters
int maze_sanity_check(const matrix *const data_in)
{
    char error_str[ERROR_STR_LEN];
    const size_t row_len = data_in->line_length;

    for (size_t i = 1; i < data_in->line_count; i++) {
        if (strlen(data_in->matrix[i]) != row_len) {
            strncpy(error_str, "Row length\n", ERROR_STR_LEN);
            goto return_exit_failure;
        }
    }

    int start = 0;
    int exit = 0;
    for (size_t i = 0; i < data_in->line_count; i++) {
        if (data_in->matrix[i][0] != '#' && data_in->matrix[i][0] != 'E') {
            strncpy(error_str, "Row start\n", ERROR_STR_LEN);
            goto return_exit_failure;
        }

        if (data_in->matrix[i][row_len-1] != '#' && data_in->matrix[i][row_len-1] != 'E') {
            strncpy(error_str, "Row end\n", ERROR_STR_LEN);
            goto return_exit_failure;
        }

        for (size_t j = 0; j < row_len; j++) {
            if (data_in->matrix[i][j] != '#' && data_in->matrix[i][j] != '^' &&
                data_in->matrix[i][j] != 'E' && data_in->matrix[i][j] != ' ')
            {
                strncpy(error_str, "Invalid characters\n", ERROR_STR_LEN);
                goto return_exit_failure;
            }

            if (data_in->matrix[i][j] == '^') {
                start = 1;
            }
            if (data_in->matrix[i][j] == 'E') {
                exit = 1;
            }
        }
    }

    if (start != 1 || exit != 1) {
        strncpy(error_str, "start or exit\n", ERROR_STR_LEN);
        goto return_exit_failure;
    }

    for (size_t i = 0; i < row_len; i++) {
        if (data_in->matrix[0][i] != '#' && data_in->matrix[0][i] != '^' && data_in->matrix[0][i] != 'E') {
            strncpy(error_str, "First row is not solid\n", ERROR_STR_LEN);
            goto return_exit_failure;
        }

        if (data_in->matrix[data_in->line_count-1][i] != '#' && data_in->matrix[data_in->line_count-1][i] != '^' &&
            data_in->matrix[data_in->line_count-1][i] != 'E')
        {
            strncpy(error_str, "Last row is not solid\n", ERROR_STR_LEN);
            goto return_exit_failure;
        }
    }

    return EXIT_SUCCESS;

    return_exit_failure:
        printf("Error: %s", error_str);
        return EXIT_FAILURE;

}

int find_start_coordinate(const matrix *const data_in, x_y *const coordinates_out)
{
    int row = 0;
    int column = -1;
    char error_str[ERROR_STR_LEN];

    if (data_in->matrix == NULL) {
        strncpy(error_str, "Data uninitialized\n", ERROR_STR_LEN);
        goto return_exit_failure;
    }

    for (int i = 0; i < data_in->line_count; i++) {
        for (int j = 0; data_in->matrix[i][j] != '\0'; j++) {
            if (data_in->matrix[i][j] == '^') {
                column = j;
                break;
            }
        }
        if (column != -1) {
            break;
        }
        row++;
    }

    if (column != -1) {
        #ifdef DEBUG
            printf("The '^' character is found at index %d on the line %d.\n", column, row);
        #endif
        coordinates_out->x = column;
        coordinates_out->y = row;
    }

    return EXIT_SUCCESS;

    return_exit_failure:
        printf("Error: %s", error_str);
        return EXIT_FAILURE;

}

int handle_file_input(matrix *data_out)
{
    char file_path[512];
    char buffer[READ_CAPACITY];
    char error_str[ERROR_STR_LEN];

    printf("Enter path to maze text file: (maximum 512 characters)");
    if (fgets(file_path, sizeof(file_path), stdin) == NULL) {
        strncpy(error_str, " Reading the input file.\n", ERROR_STR_LEN);
        goto return_exit_failure;
    }

    size_t len = strlen(file_path);
    if (len > 0 && file_path[len - 1] == '\n') {
        file_path[len - 1] = '\0';
    }

    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        strncpy(error_str, "Opening the input file.\n", ERROR_STR_LEN);
        goto return_exit_failure;
    } else {
        #ifdef DEBUG
            printf("File opened successfully in read-only mode.\n");
        #endif
    }

    data_out->matrix = (char**)malloc(READ_CAPACITY * sizeof(char *));
    if (data_out->matrix == NULL) {
        fclose(file);
        strncpy(error_str, "Allocating memory for matrix.\n", ERROR_STR_LEN);
        goto return_exit_failure;
    }

    while (fgets(buffer, READ_CAPACITY, file) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';

        data_out->matrix[data_out->line_count] = malloc((strlen(buffer) + 1) * sizeof(char));
        if (data_out->matrix[data_out->line_count] == NULL) {
            fclose(file);
            strncpy(error_str, "Allocating memory for matrix row.\n", ERROR_STR_LEN);
            goto return_exit_failure;
        }
        strcpy(data_out->matrix[data_out->line_count], buffer);
        data_out->line_count++;
    }

    fclose(file);

    data_out->line_length = strlen(data_out->matrix[0]);

    return EXIT_SUCCESS;

    return_exit_failure:
        printf("Error: %s", error_str);
        return EXIT_FAILURE;

}

int main()
{
    matrix data = {.matrix = NULL, .line_count = 0};

    if (handle_file_input(&data) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    if (maze_sanity_check(&data) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    x_y coordinates = {-1, -1};
    if (find_start_coordinate(&data, &coordinates) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    for (int i = 0; i < data.line_count; i++) {
        #ifdef DEBUG
            printf("%s\n", data.matrix[i]);            
        #endif
        free(data.matrix[i]); // Remeber this or all hell is loose
    }
    free(data.matrix);

    return EXIT_SUCCESS;
}