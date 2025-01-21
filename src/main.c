#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define ERROR_STR_LEN 64
#define READ_CAPACITY 256

enum direction_
{
    UP = 0,
    LEFT = 1,
    DOWN = 2,
    RIGHT = 3
};

typedef struct x_y_type {
    int x;
    int y;
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
int where_to_go(const matrix *const maze, const x_y *const pos)
{
    int direction = -1;
    if ((pos->y + 1) < maze->line_count && maze->matrix[pos->y + 1][pos->x] == ' ') {
        direction = DOWN;
    } else
    if ((pos->x + 1) < maze->line_length && maze->matrix[pos->y][pos->x + 1] == ' ') {
        direction = RIGHT;
    } else 
    if ((pos->y - 1) >= 0 && maze->matrix[pos->y-1][pos->x] == ' ') {
        direction = UP;
    } else 
    if ((pos->x - 1) >= 0 && maze->matrix[pos->y][pos->x - 1] == ' ') {
        direction = LEFT;
    }
    /*else
    if ((pos->x - 1) >= 0 && maze->matrix[pos->y][pos->x - 1] == '*') {
        direction = 3;
    }
    else
    if ((pos->y - 1) >= 0 && maze->matrix[pos->y-1][pos->x] == '*') {
        direction = 2;
    }
    else
    if ((pos->x + 1) < maze->line_length && maze->matrix[pos->y][pos->x + 1] == '*') {
        direction = 1;
    }
    else
    if ((pos->y + 1) < maze->line_count && maze->matrix[pos->y + 1][pos->x] == '*') {
        direction = 0;
    }*/

    return direction;
}

int move_up(const matrix *const maze, x_y *pos)
{
    if ((pos->y - 1) >= 0 &&
    (maze->matrix[pos->y-1][pos->x] == ' ') || (maze->matrix[pos->y-1][pos->x] == '*'))
    {
        pos->y--;
        return 1;
    }
    return 0;
}
int move_left(const matrix *const maze, x_y *pos)
{
    if ((pos->x - 1) >= 0 &&
    (maze->matrix[pos->y][pos->x - 1] == ' ') || (maze->matrix[pos->y][pos->x - 1] == '*'))
    {
        pos->x--;
        return 1;
    }
    return 0;
}
int move_down(const matrix *const maze, x_y *pos)
{
    if ((pos->y + 1) < maze->line_count && 
        (maze->matrix[pos->y + 1][pos->x] == ' ') || (maze->matrix[pos->y + 1][pos->x] == '*'))
    {
        pos->y++;
        return 1;
    }
    return 0;
}
int move_right(const matrix *const maze, x_y *pos)
{
    if ((pos->x + 1) < maze->line_length &&
        (maze->matrix[pos->y][pos->x + 1] == ' ') || (maze->matrix[pos->y][pos->x + 1] == '*'))
    {
        pos->x++;
        return 1;
    }
    return 0;
}

int move_algorithm(int moves, matrix *const maze, x_y pos)
{
    char error_str[ERROR_STR_LEN];
    const char *direction_txt[4] = {"up\n", "left\n", "down\n", "right\n"};

    int (*move_fPtr[4])(const matrix *const maze, x_y *pos);
    move_fPtr[UP] = move_up;
    move_fPtr[LEFT] = move_left;
    move_fPtr[DOWN] = move_down;
    move_fPtr[RIGHT] = move_right;

    int moved = 0;
    int direction = UP;
    int route_back[200] = {0};
    int route_back_index = 0;

    route_back[route_back_index] = direction;
    route_back_index++;
    int go_back = 0;

    while (moves)
    {
        direction = where_to_go(maze, &pos);
        if (direction == -1) {
            go_back = 1;
            direction = route_back[route_back_index];
            route_back_index--;
            if (direction == UP) direction = DOWN;
            else if (direction == DOWN) direction = UP;
            else if (direction == LEFT) direction = RIGHT;
            else if (direction == RIGHT) direction = LEFT;
        } else {
            go_back = 0;
        }

        maze->matrix[pos.y][pos.x] = '*';
        moved = (*move_fPtr[direction])(maze, &pos);
        if (moved)
        {
            moves--;
            if (!go_back) {
                route_back[route_back_index] = direction;
                route_back_index++;
            }
            maze->matrix[pos.y][pos.x] = '^';
            /*for (int i = 0; i < maze->line_count; i++) {
                printf("%s\n", maze->matrix[i]);
            }*/
            //Sleep(1000);
        }
    }

    return EXIT_SUCCESS;

    return_exit_failure:
        printf("Error: %s", error_str);
        return EXIT_FAILURE;
}

int move_in_maze(int moves, const matrix *const data_in, x_y *const coordinates)
{
    char error_str[ERROR_STR_LEN];

    matrix data = {.matrix = NULL, .line_count = data_in->line_count, .line_length = data_in->line_length};

    data.matrix = (char**)malloc(READ_CAPACITY * sizeof(char *));
    if (data.matrix == NULL) {
        strncpy(error_str, "Allocating memory for matrix.\n", ERROR_STR_LEN);
        goto return_exit_failure;
    }
    size_t lenght = strlen(data_in->matrix[0]) + 1;

    for (size_t i = 0; i < data.line_count; i++) {
        data.matrix[i] = malloc(lenght * sizeof(char));
        if (data.matrix[i] == NULL) {
            strncpy(error_str, "Allocating memory for matrix row.\n", ERROR_STR_LEN);
            goto return_exit_failure;
        }
        strncpy(data.matrix[i], data_in->matrix[i], lenght);
    }

    if (move_algorithm(moves, &data, *coordinates) != EXIT_SUCCESS) {
        strncpy(error_str, "Algorithm.\n", ERROR_STR_LEN);
        goto return_exit_failure;
    }

    printf("\n");
    for (int i = 0; i < data.line_count; i++) {
        printf("%s\n", data.matrix[i]);
        free(data.matrix[i]);
    }
    printf("\n");
    free(data.matrix);

    return EXIT_SUCCESS;

    return_exit_failure:
        for (int i = 0; i < data.line_count; i++) {
            free(data.matrix[i]);
        }
        free(data.matrix);
        printf("Error: %s", error_str);
        return EXIT_FAILURE;
}

int main()
{
    matrix data = {.matrix = NULL, .line_count = 0};
    x_y coordinates = {-1, -1};
    const int tries = 3;
    const int moves[3] = {20, 150, 200};

    if (handle_file_input(&data) != EXIT_SUCCESS) {
        goto return_exit_failure;
    }

    if (maze_sanity_check(&data) != EXIT_SUCCESS) {
        goto return_exit_failure;
    }

    if (find_start_coordinate(&data, &coordinates) != EXIT_SUCCESS) {
        goto return_exit_failure;
    }

    for (size_t i = 0; i < tries; i++) {
        if (move_in_maze(moves[i], &data, &coordinates) != EXIT_SUCCESS) {
            goto return_exit_failure;
        }
    }

    for (int i = 0; i < data.line_count; i++) {
        free(data.matrix[i]);
    }
    free(data.matrix);

    return EXIT_SUCCESS;

    return_exit_failure:
        for (int i = 0; i < data.line_count; i++) {
            free(data.matrix[i]);
        }
        free(data.matrix);

        return EXIT_FAILURE;
}