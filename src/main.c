#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define ERROR_STR_LEN 64
#define READ_CAPACITY 256

enum direction_state {
    UP = 0,
    LEFT = 1,
    DOWN = 2,
    RIGHT = 3,
    CANT_MOVE = -1
};

typedef struct x_y_pos {
    int x;
    int y;
} x_y;

typedef struct matrix_data {
    char **matrix;
    int line_count;
    int line_length;
} matrix;

//*****************************************************************************/
// A maze is a:
// * Line that starts and ends with #- or E-character.
// * Rectangular, same length lines.
// * Only contains #-, E-, ^- and underscore characters
// * Has multiple exits
// * Only one input
// * First and last row contains only #-, ^- and E-characters
//*****************************************************************************/
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

//*****************************************************************************/
// Find the start coordinated from input data
// When coordinates found, return success
//
//*****************************************************************************/
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
    } else {
        strncpy(error_str, "No start point\n", ERROR_STR_LEN);
        goto return_exit_failure;
    }

    return EXIT_SUCCESS;

    return_exit_failure:
        printf("Error: %s", error_str);
        return EXIT_FAILURE;

}

//*****************************************************************************/
// Read the data from input file
// * Data read to buffer, return success
//
//*****************************************************************************/
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
            for (int j = 0; j < data_out->line_count; j++) {
                free(data_out->matrix[j]);
            }
            free(data_out->matrix);
            fclose(file);
            strncpy(error_str, "Allocating memory for matrix row.\n", ERROR_STR_LEN);
            goto return_exit_failure;
        } else {
            strcpy(data_out->matrix[data_out->line_count], buffer);
            data_out->line_count++;
        }
    }

    fclose(file);

    data_out->line_length = strlen(data_out->matrix[0]);

    return EXIT_SUCCESS;

    return_exit_failure:
        printf("Error: %s", error_str);
        return EXIT_FAILURE;

}

//*****************************************************************************/
// Check the possible move directions
// * return random first possible free direction
// * If not possible to move return -1
//
//*****************************************************************************/
int where_to_go(const matrix *const maze, const x_y *const pos)
{
    int direction = -1;
    /*int possible[4] = {0};
    int count = 0;

    if ((pos->y - 1) >= 0 && maze->matrix[pos->y-1][pos->x] == ' ') {
        possible[count] = UP;
        count++;
    } else if ((pos->x - 1) >= 0 && maze->matrix[pos->y][pos->x - 1] == ' ') {
        possible[count] = LEFT;
        count++;
    } else if ((pos->y + 1) < maze->line_count && maze->matrix[pos->y + 1][pos->x] == ' ') {
        possible[count] = DOWN;
        count++;
    } else if ((pos->x + 1) < maze->line_length && maze->matrix[pos->y][pos->x + 1] == ' ') {
        possible[count] = RIGHT;
        count++;
    }

    if (count > 0) {
        // get random number 0 to count
        int random = rand() % (count + 1);
        direction = possible[random];
    }*/
    if ((pos->x - 1) >= 0 && maze->matrix[pos->y][pos->x - 1] == 'E') {
        direction = LEFT;
        printf("E found. Direction %d\n", direction);
    }
    else
    if ((pos->y - 1) >= 0 && maze->matrix[pos->y - 1][pos->x] == 'E') {
        direction = UP;
        printf("E found. Direction %d\n", direction);
    }
    else
    if ((pos->x + 1) < maze->line_length && maze->matrix[pos->y][pos->x + 1] == 'E') {
        direction = RIGHT;
        printf("E found. Direction %d\n", direction);
    }
    else
    if ((pos->y + 1) < maze->line_count && maze->matrix[pos->y + 1][pos->x] == 'E') {
        direction = DOWN;
    }
    else
    if ((pos->x - 1) >= 0 && maze->matrix[pos->y][pos->x - 1] == ' ') {
        direction = LEFT;
    }
    else
    if ((pos->y - 1) >= 0 && maze->matrix[pos->y-1][pos->x] == ' ') {
        direction = UP;
    }
    else
    if ((pos->x + 1) < maze->line_length && maze->matrix[pos->y][pos->x + 1] == ' ') {
        direction = RIGHT;
    }
    else
    if ((pos->y + 1) < maze->line_count && maze->matrix[pos->y + 1][pos->x] == ' ') {
        direction = DOWN;
    }

    return direction;
}

//*****************************************************************************/
// Move functions
//
//*****************************************************************************/
int move_up(const matrix *const maze, x_y *pos)
{
    if (pos->y - 1 >= 0) {
        char next_step = maze->matrix[pos->y-1][pos->x];

        if (next_step == ' ' || next_step == '*' || next_step == 'E') {
            pos->y--;
            return 1;
        }
    }
    return 0;
}

int move_left(const matrix *const maze, x_y *pos)
{
    if ((pos->x - 1) >= 0) {
        char next_step = maze->matrix[pos->y][pos->x - 1];

        if (next_step == ' ' || next_step == '*' || next_step == 'E') {
            pos->x--;
            return 1;
        }
    }
    return 0;
}

int move_down(const matrix *const maze, x_y *pos)
{
    if ((pos->y + 1) < maze->line_count) {
        char next_step = maze->matrix[pos->y + 1][pos->x];
        if (next_step == ' ' || next_step == '*' || next_step == 'E') {
            pos->y++;
            return 1;
        }
    }
    return 0;
}

int move_right(const matrix *const maze, x_y *pos)
{
    if ((pos->x + 1) < maze->line_length) {
        char next_step = maze->matrix[pos->y][pos->x + 1];
        if (next_step == ' ' || next_step == '*' || next_step == 'E') {
            pos->x++;
            return 1;
        }
    }
    return 0;
}

int step_back(int direction)
{
    switch (direction) {
        case UP:
            direction = DOWN;
            break;
        case DOWN:
            direction = UP;
            break;
        case LEFT:
            direction = RIGHT;
            break;
        case RIGHT:
            direction = LEFT;
            break;
        default:
            direction = UP;
            break;
    }

    return direction;
}

//*****************************************************************************/
// Move algorithm
//
//*****************************************************************************/
int move_algorithm(int moves, matrix *const maze, x_y pos)
{
    char error_str[ERROR_STR_LEN];
    const char *direction_txt[4] = {"up\n", "left\n", "down\n", "right\n"};

    int (*move_fPtr[4])(const matrix *const maze, x_y *pos);
    move_fPtr[UP] = move_up;
    move_fPtr[LEFT] = move_left;
    move_fPtr[DOWN] = move_down;
    move_fPtr[RIGHT] = move_right;

    int direction = UP;
    int route_back[200] = {0};
    int route_back_index = 0;

    route_back[route_back_index] = direction;
    route_back_index++;
    int go_back = 0;

    // TODO: go straight if possible
    // TODO: mark dead ends

    while (moves) {
        direction = where_to_go(maze, &pos);
        if (direction == CANT_MOVE) {
            go_back = 1;
            direction = step_back(route_back[route_back_index]);
            route_back_index--;
            // TODO
            if (route_back_index < 0) {
                #ifdef DEBUG
                    printf("Error route back index less than zero\n");
                #endif
                break;
            }
        } else {
            go_back = 0;
        }

        maze->matrix[pos.y][pos.x] = '*';

        if ((*move_fPtr[direction])(maze, &pos)) {
            moves--;
            if (go_back == 0) {
                route_back[route_back_index] = direction;
                route_back_index++;
            }
            if (maze->matrix[pos.y][pos.x] == 'E') {
                maze->matrix[pos.y][pos.x] = '^';
                printf("Freedom at last!\n");
                break;
            } else {
                maze->matrix[pos.y][pos.x] = '^';
            }
            #ifdef DEBUG
                for (int i = 0; i < maze->line_count; i++) {
                    printf("%s\n", maze->matrix[i]);
                }
                Sleep(250);
            #endif
        }
    }

    return EXIT_SUCCESS;

    return_exit_failure:
        printf("Error: %s", error_str);
        return EXIT_FAILURE;
}

//*****************************************************************************/
// Initialize moving in the maze
//
//*****************************************************************************/
int move_in_maze(int moves, const matrix *const data_in, x_y *const coordinates)
{
    int ret = EXIT_SUCCESS;

    matrix data = {.matrix = NULL, .line_count = data_in->line_count, .line_length = data_in->line_length};

    data.matrix = (char**)malloc(READ_CAPACITY * sizeof(char *));
    if (data.matrix == NULL) {
        printf("Error: Allocating memory for data.\n");
        return EXIT_FAILURE;
    }

    size_t length = strlen(data_in->matrix[0]) + 1;

    for (size_t i = 0; i < data.line_count; i++) {
        data.matrix[i] = malloc(length * sizeof(char));
        if (data.matrix[i] == NULL) {
            for (size_t j = 0; j < i; j++) {
                free(data.matrix[j]);
            }
            free(data.matrix);
            printf("Error: Allocating memory for matrix row.\n");
            return EXIT_FAILURE;
        } else {
            strncpy(data.matrix[i], data_in->matrix[i], length);
        }
    }

    #ifdef DEBUG
        printf("data allocated for move algorithm\n");
    #endif

    if (move_algorithm(moves, &data, *coordinates) != EXIT_SUCCESS) {
        printf("Error: Algorithm.\n");
        ret = EXIT_FAILURE;
    } else {
        #ifdef DEBUG
            printf("Move algorithm success\n");
        #endif
    }

    printf("\n");
    for (int i = 0; i < data.line_count; i++) {
        printf("%s\n", data.matrix[i]);
        free(data.matrix[i]);
    }
    printf("\n");
    free(data.matrix);

    return ret;
}

//*****************************************************************************/
// Entry function
//
//*****************************************************************************/
int main()
{
    matrix data = {.matrix = NULL, .line_count = 0};
    x_y coordinates = {-1, -1};
    const int tries = 3;
    const int moves[3] = {20, 150, 200};

    if (handle_file_input(&data) != EXIT_SUCCESS) {
        goto return_exit_failure;
    } else {
        #ifdef DEBUG
            printf("Handle file success\n");
        #endif
    }

    if (maze_sanity_check(&data) != EXIT_SUCCESS) {
        goto return_exit_failure;
    } else {
        #ifdef DEBUG
            printf("sanity check success\n");
        #endif
    }

    if (find_start_coordinate(&data, &coordinates) != EXIT_SUCCESS) {
        goto return_exit_failure;
    } else {
        #ifdef DEBUG
            printf("Start coordinate success\n");
        #endif
    }

    for (size_t i = 0; i < tries; i++) {
        if (move_in_maze(moves[i], &data, &coordinates) != EXIT_SUCCESS) {
            goto return_exit_failure;
        } else {
            #ifdef DEBUG
                printf("move %d success\n", moves[i]);
            #endif
        }
    }

    for (int i = 0; i < data.line_count; i++) {
        free(data.matrix[i]);
    }
    free(data.matrix);

    #ifdef DEBUG
        printf("Return success\n");
    #endif
    return EXIT_SUCCESS;

    return_exit_failure:
        for (int i = 0; i < data.line_count; i++) {
            free(data.matrix[i]);
        }
        free(data.matrix);

        return EXIT_FAILURE;
}