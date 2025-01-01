#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h> // For tolower()
#include <ncurses.h> // For ncurses functions

#define WIDTH 120
#define HEIGHT 40
#define MAX_ROOMS 9
#define VISION_RADIUS 5

typedef struct {
    int x, y; // Top-left corner
    int width, height; // Dimensions
    int door_x, door_y; // Door position
} Room;

char map[HEIGHT][WIDTH];
char seen_map[HEIGHT][WIDTH];
int player_x, player_y;

// Function to initialize the map and seen_map with walls
void initialize_map() {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (i == 0 || i == HEIGHT - 1 || j == 0 || j == WIDTH - 1)
                map[i][j] = '#'; // Map border
            else
                map[i][j] = ' '; // Empty space
            seen_map[i][j] = ' '; // Initialize unseen
        }
    }
}

// Function to create a room
int create_room(Room *room) {
    room->width = 4 + rand() % 22; // Room width (4 to 25)
    room->height = 4 + rand() % 4; // Room height (4 to 7)
    room->x = 1 + rand() % (WIDTH - room->width - 1); // Avoid map border
    room->y = 1 + rand() % (HEIGHT - room->height - 1);

    // Check if room overlaps with existing features
    for (int i = room->y; i < room->y + room->height; i++) {
        for (int j = room->x; j < room->x + room->width; j++) {
            if (map[i][j] != ' ') return 0; // Overlap found
        }
    }

    // Place the room
    for (int i = room->y; i < room->y + room->height; i++) {
        for (int j = room->x; j < room->x + room->width; j++) {
            if (i == room->y || i == room->y + room->height - 1 ||
                j == room->x || j == room->x + room->width - 1) {
                map[i][j] = (i == room->y || i == room->y + room->height - 1) ? '_' : '|'; // Wall
            } else {
                map[i][j] = '.'; // Floor
            }
        }
    }

    // Add a door
    int door_side = rand() % 4; // Random side: 0 = top, 1 = bottom, 2 = left, 3 = right
    if (door_side == 0) { // Top
        room->door_x = room->x + 1 + rand() % (room->width - 2);
        room->door_y = room->y;
    } else if (door_side == 1) { // Bottom
        room->door_x = room->x + 1 + rand() % (room->width - 2);
        room->door_y = room->y + room->height - 1;
    } else if (door_side == 2) { // Left
        room->door_x = room->x;
        room->door_y = room->y + 1 + rand() % (room->height - 2);
    } else { // Right
        room->door_x = room->x + room->width - 1;
        room->door_y = room->y + 1 + rand() % (room->height - 2);
    }
    map[room->door_y][room->door_x] = '+';

    return 1; // Room created successfully
}

// Function to place a window in each room
void place_window_in_room(Room *room) {
    int window_x = 0, window_y = 0;
    int side = rand() % 4; // Random side: 0 = top, 1 = bottom, 2 = left, 3 = right

    if (side == 0) { // Top wall
        window_x = room->x + 1 + rand() % (room->width - 2);
        window_y = room->y;
    } else if (side == 1) { // Bottom wall
        window_x = room->x + 1 + rand() % (room->width - 2);
        window_y = room->y + room->height - 1;
    } else if (side == 2) { // Left wall
        window_x = room->x;
        window_y = room->y + 1 + rand() % (room->height - 2);
    } else { // Right wall
        window_x = room->x + room->width - 1;
        window_y = room->y + 1 + rand() % (room->height - 2);
    }

    // Ensure the window doesn't replace a door
    if (map[window_y][window_x] == '+') return;

    map[window_y][window_x] = '='; // Place the window
}

// Function to connect rooms with corridors
void connect_rooms(Room r1, Room r2) {
    int x1 = r1.door_x;
    int y1 = r1.door_y;
    int x2 = r2.door_x;
    int y2 = r2.door_y;

    // First connect horizontally from x1 to x2
    for (int x = (x1 < x2 ? x1 : x2); x <= (x1 > x2 ? x1 : x2); x++) {
        if (map[y1][x] == '.' || map[y1][x] == '+') continue;
        map[y1][x] = '#';
    }

    // Then connect vertically from y1 to y2
    for (int y = (y1 < y2 ? y1 : y2); y <= (y1 > y2 ? y1 : y2); y++) {
        if (map[y][x2] == '.' || map[y][x2] == '+') continue;
        map[y][x2] = '#';
    }
}

// Function to update seen tiles
void update_seen_tiles() {
    for (int i = player_y - VISION_RADIUS; i <= player_y + VISION_RADIUS; i++) {
        for (int j = player_x - VISION_RADIUS; j <= player_x + VISION_RADIUS; j++) {
            if (i >= 0 && i < HEIGHT && j >= 0 && j < WIDTH) {
                seen_map[i][j] = map[i][j];
            }
        }
    }
}

// Function to print the map
void print_map() {
    clear(); // Clear the ncurses screen
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (seen_map[i][j] != ' ') {
                mvaddch(i, j, seen_map[i][j]);
            }
        }
    }
    refresh();
}

// Function to move the player
void move_player(char direction) {
    int new_x = player_x, new_y = player_y;

    direction = tolower(direction);
    if (direction == 'w') new_y--; // Up
    else if (direction == 's') new_y++; // Down
    else if (direction == 'a') new_x--; // Left
    else if (direction == 'd') new_x++; // Right

    // Check if the new position is valid
    if (map[new_y][new_x] == '.' || map[new_y][new_x] == '+' || map[new_y][new_x] == '#') {
        if (map[new_y][new_x] == '#') map[new_y][new_x] = '.'; // Replace corridor with floor

        map[player_y][player_x] = '.'; // Restore the previous tile
        player_x = new_x;
        player_y = new_y;
        map[player_y][player_x] = '@'; // Place the player
        update_seen_tiles(); // Update vision
    }
}

// Main function
int main() {
    srand(time(NULL));
    initscr(); // Initialize ncurses
    noecho(); // Disable key echoing
    cbreak(); // Disable line buffering
    keypad(stdscr, TRUE); // Enable special keys

    initialize_map();

    Room rooms[MAX_ROOMS];
    int room_count = 0;

    // Generate rooms
    while (room_count < MAX_ROOMS) {
        Room new_room;
        if (create_room(&new_room)) {
            if (room_count > 0)
                connect_rooms(rooms[room_count - 1], new_room);
            rooms[room_count++] = new_room;
        }
    }

    // Place windows in each room
    for (int i = 0; i < room_count; i++) {
        place_window_in_room(&rooms[i]);
    }

    // Place hero
    int hero_room = rand() % room_count;
    player_x = rooms[hero_room].x + 1 + rand() % (rooms[hero_room].width - 2);
    player_y = rooms[hero_room].y + 1 + rand() % (rooms[hero_room].height - 2);
    map[player_y][player_x] = '@';

    update_seen_tiles(); // Initialize seen tiles

    // Game loop
    int input;
    while (1) {
        print_map();
        input = getch();
        if (tolower(input) == 'q') break;
        move_player(input);
    }

    endwin(); // End ncurses
    return 0;
}
