
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#define MAX_USERS 100
#define USERNAME_LENGTH 50
#define PASSWORD_LENGTH 20
#define EMAIL_LENGTH 100
#define FILENAME "users.dat"
#define WIDTH 80
#define HEIGHT 40
#define MAX_ROOMS 9
#define VISION_RADIUS 5
#define MAX_REGENERATIONS 4 // حداکثر تعداد بازسازی نقشه
#define TOTAL_GOLD 10 // تعداد کل طلای معمولی
int regenerations = 0; // شمارنده بازسازی‌ها
int placed_gold = 0;
// User structure
typedef struct {
    char username[USERNAME_LENGTH];
    char password[PASSWORD_LENGTH];
    char email[EMAIL_LENGTH];
} User;

User users[MAX_USERS];
int userCount = 0;

typedef struct {
    int x, y; // Top-left corner
    int width, height; // Dimensions
    int door_x, door_y; // Door position
} Room;

char hit_message[25] = "You hit a column!";
int show_message = 0; // 1 if the message is active, 0 otherwise
int player_level = 1;  // Starts with level 1
int player_gold = 0;   // Starts with 0 gold
int player_hp = 12;    // Starts with 12 HP
int player_str = 16;   // Starts with strength 16
int player_arm = 4;    // Starts with armor 4
int player_exp = 0;    // Starts with 0 experience
int room_count;
#define MAX_FLOORS 3 // تعداد طبقات در بازی (برای مثال 3 طبقه)

// آرایه برای نگهداری نقشه‌های مختلف طبقات
char map_floor[MAX_FLOORS][HEIGHT][WIDTH];
char trap_map_floor[MAX_FLOORS][HEIGHT][WIDTH];
int current_floor = 0; // طبقه فعلی
int stairs_up_x = -1, stairs_up_y = -1; // موقعیت پله بالا
int stairs_down_x = -1, stairs_down_y = -1; // موقعیت پله پایین

char map[HEIGHT][WIDTH];
char seen_map[HEIGHT][WIDTH];
int player_x, player_y;
Room rooms[MAX_ROOMS]; // Store rooms to check player's position
char trap_map[HEIGHT][WIDTH]; // New global variable to store traps

typedef struct {
    int player_x;
    int player_y;
    int player_hp;
    char map[HEIGHT][WIDTH];
    char trap_map[HEIGHT][WIDTH];
    Room rooms[MAX_ROOMS];
    int room_count;
} GameState;

void save_game(const char* filename, GameState* game_state) {
    FILE *file = fopen(filename, "wb");
    if (file) {
        fwrite(game_state, sizeof(GameState), 1, file);
        fclose(file);
        printw("Game saved successfully as %s.\n", filename);
    } else {
        printw("Error saving game.\n");
    }
    refresh();
}

int load_game(const char* filename, GameState* game_state) {
    FILE *file = fopen(filename, "rb");
    if (file) {
        fread(game_state, sizeof(GameState), 1, file);
        fclose(file);
        return 1; // بازی با موفقیت بارگذاری شد
    } else {
        printw("No saved game found with the name %s. Starting a new game.\n", filename);
        return 0; // هیچ بازی ذخیره‌شده‌ای یافت نشد، شروع بازی جدید
    }
}

void generateRandomPassword(char *password) {
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    int length = 8; // Password length
    for (int i = 0; i < length; i++) {
        password[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    password[length] = '\0';
}

int isEmailValid(const char *email) {
    const char *at = strchr(email, '@');
    const char *dot = strrchr(email, '.');
    return at && dot && at < dot;
}

int isUsernameUnique(const char *username) {
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, username) == 0) {
            return 0; // Username is not unique
        }
    }
    return 1; // Username is unique
}

void createUser() {
    if (userCount >= MAX_USERS) {
        printw("Maximum number of users reached.\n");
        return;
    }

    User newUser;
    printw("Enter username: ");
    move(LINES - 1, 16);
    echo();
    scanw("%s", newUser.username);
    noecho();

    if (!isUsernameUnique(newUser.username)) {
        printw("Username already taken. Please choose another one.\n");
        return;
    }

    printw("Enter password: ");
    move(LINES - 1, 15);
    echo();
    scanw("%s", newUser.password);
    noecho();

    if (strlen(newUser.password) < 7) {
        printw("Password must be at least 7 characters long.\n");
        return;
    }

    printw("Enter email: ");
    move(LINES - 1, 13);
    echo();
    scanw("%s", newUser.email);
    noecho();

    if (!isEmailValid(newUser.email)) {
        printw("Invalid email address. Please try again.\n");
        return;
    }

    users[userCount++] = newUser;
    printw("New user created successfully.\n");

    // Save the user data to file
    FILE *file = fopen(FILENAME, "ab");
    if (file) {
        fwrite(&newUser, sizeof(User), 1, file);
        fclose(file);
    } else {
        printw("Error saving user data.\n");
    }
}

void generatePasswordForUser() {
    char username[USERNAME_LENGTH];
    printw("Enter username: ");
    scanw("%s", username);

    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, username) == 0) {
            generateRandomPassword(users[i].password);
            printw("New password: %s\n", users[i].password);
            return;
        }
    }

    printw("User not found.\n");
}

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

void add_columns_to_room(Room *room) {
    for (int i = room->y + 1; i < room->y + room->height - 1; i += 4) {
        for (int j = room->x + 1; j < room->x + room->width - 1; j += 4) {
            int column_x = j + rand() % 4; // Random x within the 4x4 section
            int column_y = i + rand() % 4; // Random y within the 4x4 section

            // Ensure the column is within the room bounds and not on walls or doors
            if (column_x < room->x + 1 || column_x >= room->x + room->width - 1 ||
                column_y < room->y + 1 || column_y >= room->y + room->height - 1) {
                continue; // Skip invalid positions
            }

            map[column_y][column_x] = 'O'; // Place the column
        }
    }
}

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

    // Place the room walls
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

// Add a door at one of the walls
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
}
void place_window_in_room(Room *room) {
    int window_x = 0, window_y = 0;
    int side = rand() % 4; 
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

// Helper function to check if a tile is valid for a corridor (not a window)
int is_valid_for_corridor(int x, int y) {
    return map[y][x] == '.' || map[y][x] == '+' || map[y][x] == '#';
}
void connect_rooms(Room* r1, Room* r2) {
    int x1 = r1->door_x;
    int y1 = r1->door_y;
    int x2 = r2->door_x;
    int y2 = r2->door_y;

    int mid_x1 = r1->x + r1->width / 2;
    int mid_y1 = r1->y + r1->height / 2;
    int mid_x2 = r2->x + r2->width / 2;
    int mid_y2 = r2->y + r2->height / 2;

    // Create a horizontal corridor from the center of r1 to the center of r2
    while (x1 != mid_x1 || y1 != mid_y1) {
        if (map[y1][x1] == '_' || map[y1][x1] == '|') {
            map[y1][x1] = '+'; // Place a door if a wall is encountered
        } else if (map[y1][x1] == ' ') {
            map[y1][x1] = '#'; // Create a corridor if the tile is empty
        }
        if (x1 != mid_x1) x1 += (mid_x1 > x1) ? 1 : -1;
        if (y1 != mid_y1) y1 += (mid_y1 > y1) ? 1 : -1;
    }

    // Create a vertical corridor from mid_y1 to mid_y2
    while (x1 != mid_x2 || y1 != mid_y2) {
        if (map[y1][x1] == '_' || map[y1][x1] == '|') {
            map[y1][x1] = '+'; // Place a door if a wall is encountered
        } else if (map[y1][x1] == ' ') {
            map[y1][x1] = '#'; // Create a corridor if the tile is empty
        }
        if (x1 != mid_x2) x1 += (mid_x2 > x1) ? 1 : -1;
        if (y1 != mid_y2) y1 += (mid_y2 > y1) ? 1 : -1;
    }

    // Create a horizontal corridor from mid_x2 to x2
    while (x2 != mid_x2 || y2 != mid_y2) {
        if (map[y2][x2] == '_' || map[y2][x2] == '|') {
            map[y2][x2] = '+'; // Place a door if a wall is encountered
        } else if (map[y2][x2] == ' ') {
            map[y2][x2] = '#'; // Create a corridor if the tile is empty
        }
        if (x2 != mid_x2) x2 += (mid_x2 > x2) ? 1 : -1;
        if (y2 != mid_y2) y2 += (mid_y2 > y2) ? 1 : -1;
    }

    // Create a vertical corridor from x2 to y2
    while (x2 != r2->door_x || y2 != r2->door_y) {
        if (map[y2][x2] == '_' || map[y2][x2] == '|') {
            map[y2][x2] = '+'; // Place a door if a wall is encountered
        } else if (map[y2][x2] == ' ') {
            map[y2][x2] = '#'; // Create a corridor if the tile is empty
        }
        if (x2 != r2->door_x) x2 += (r2->door_x > x2) ? 1 : -1;
        if (y2 != r2->door_y) y2 += (r2->door_y > y2) ? 1 : -1;
    }
}

// Function to check if the player is inside a room
int is_player_in_room() {
    for (int i = 0; i < MAX_ROOMS; i++) {
        if (player_x >= rooms[i].x && player_x < rooms[i].x + rooms[i].width &&
            player_y >= rooms[i].y && player_y < rooms[i].y + rooms[i].height) {
            return 1; // Player is inside a room
        }
    }
    return 0; // Player is not inside a room
}

// Function to update seen tiles
void update_seen_tiles() {
    // Update all map tiles to be visible
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            seen_map[i][j] = map[i][j]; // Make the entire map visible
        }
    }
}

void print_map() {
    clear(); // Clear the console (use "cls" for Windows)

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            // Print the message on the top row (optional for messages)
            if (i == 0 && show_message) {
                if (j < (int)strlen(hit_message)) {
                    printw("%c", hit_message[j]); // Print the message character
                } else {
                    printw(" "); // Fill the rest of the row with spaces
                }
            } else if (seen_map[i][j] != ' ') {
                printw("%c", seen_map[i][j]); // Print the map content
            } else {
                printw(" ");
            }
        }

        // If it's the last row (bottom row), display stats within the game field
        if (i == HEIGHT - 1) {
            // Format and display the player stats
            printw("Lvl: %d  Gold: %d  HP: %d  Str: %d  Arm: %d  Exp: %d", 
                   player_level, player_gold, player_hp, player_str, player_arm, player_exp);
            
            // Ensure we do not overflow the width (pad remaining space with spaces)
            int remaining_space = WIDTH - 56;  // 56 is the length of the stats string
            for (int k = 0; k < remaining_space; k++) {
                printw(" "); // Fill the rest of the line with spaces
            }
        }

        // Add a newline after each row
        printw("\n");
    }

    refresh(); // Refresh the screen to display changes
}

// Initialize trap map
void initialize_trap_map() {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            trap_map[i][j] = ' '; // No traps initially
        }
    }
}
void add_traps_to_room(Room *room) {
    for (int i = 0; i < 3; i++) { // Add 3 traps per room
        int trap_x, trap_y;
        do {
            trap_x = room->x + 1 + rand() % (room->width - 2);
            trap_y = room->y + 1 + rand() % (room->height - 2);
        } while (trap_map[trap_y][trap_x] == 'T'); // Ensure we don't place a trap on another trap

        trap_map[trap_y][trap_x] = 'T'; // Place a trap in the trap map
    }
}


void place_single_special_characters(Room *rooms, int room_count) {
    // Place ">"
    int room_index_for_greater = rand() % room_count;
    int greater_x = rooms[room_index_for_greater].x + 1 + rand() % (rooms[room_index_for_greater].width - 2);
    int greater_y = rooms[room_index_for_greater].y + 1 + rand() % (rooms[room_index_for_greater].height - 2);
    map[greater_y][greater_x] = '>';

    // Place "<"
    int room_index_for_less;
    do {
        room_index_for_less = rand() % room_count;
    } while (room_index_for_less == room_index_for_greater); // Ensure different rooms

    int less_x = rooms[room_index_for_less].x + 1 + rand() % (rooms[room_index_for_less].width - 2);
    int less_y = rooms[room_index_for_less].y + 1 + rand() % (rooms[room_index_for_less].height - 2);
    map[less_y][less_x] = '<';
}


void add_gold_to_room(Room *room) {
    for (int i = 0; i < 1; i++) { // Add 1 gold per room
        int gold_x, gold_y;
        do {
            gold_x = room->x + 1 + rand() % (room->width - 2);
            gold_y = room->y + 1 + rand() % (room->height - 2);
        } while (map[gold_y][gold_x] == 'g'); // Ensure we don't place gold on another gold

        map[gold_y][gold_x] = 'g'; // Place gold in the map
    }
}

void regenerate_map() {
    // Initialize map
    initialize_map();

    int room_count = 0;

    // Generate rooms
    while (room_count < MAX_ROOMS) {
        Room new_room;
        if (create_room(&new_room)) {
            add_columns_to_room(&new_room);
            if (room_count > 0)
                connect_rooms(&rooms[room_count - 1], &new_room); // استفاده از آدرس اتاق‌ها
            rooms[room_count++] = new_room;
        }
    }

    // Add windows and traps to rooms
    for (int i = 0; i < room_count; i++) {
        place_window_in_room(&rooms[i]);
        add_traps_to_room(&rooms[i]); // اضافه کردن تله‌ها به اتاق‌ها
        // Add gold to rooms
        if (placed_gold < TOTAL_GOLD) {
            add_gold_to_room(&rooms[i]); // اضافه کردن طلا به اتاق‌ها
            placed_gold++;
        }
    }

    // Place special characters ">" and "<"
    place_single_special_characters(rooms, room_count);

    // Place hero in a random room
    int hero_room = rand() % room_count;
    player_x = rooms[hero_room].x + 1 + rand() % (rooms[hero_room].width - 2);
    player_y = rooms[hero_room].y + 1 + rand() % (rooms[hero_room].height - 2);
    map[player_y][player_x] = '@';

    update_seen_tiles(); // Initialize seen tiles

    clear(); // Clear the screen for the new map
    print_map(); // Print the new map
    refresh(); // Refresh to show the updated screen
}

void move_player(char input) {
    int new_x = player_x;
    int new_y = player_y;

    switch (tolower(input)) {
        case 'w': new_y--; break;
        case 's': new_y++; break;
        case 'a': new_x--; break;
        case 'd': new_x++; break;
        case 'q': new_x--; new_y--; break;
        case 'e': new_x++; new_y--; break;
        case 'z': new_x--; new_y++; break;
        case 'c': new_x++; new_y++; break;
        default: return;
    }

    char destination_tile = map[new_y][new_x];
    if (destination_tile == '#' || destination_tile == '.' || destination_tile == '+' || destination_tile == '<' || destination_tile == '>' || destination_tile == 'g') {
        map[player_y][player_x] = '.';
        player_x = new_x;
        player_y = new_y;
        map[player_y][player_x] = '@';
        update_seen_tiles();
        
        // Clear the line before printing a new message
        mvprintw(0, 0, "                                           ");

        // Check for traps
        if (trap_map[player_y][player_x] == 'T') {
            player_hp -= 1; // کاهش HP
            mvprintw(0, 0, "You stepped on a trap! Your HP is now %d.", player_hp);
        }

        // Check for gold
        if (destination_tile == 'g') {
            player_gold += 5; // افزایش طلا
            mvprintw(0, 0, "You picked up gold! Your gold is now %d.", player_gold);
        }

        // Check for stairs and regenerate map if needed
        if ((destination_tile == '>' || destination_tile == '<') && regenerations < MAX_REGENERATIONS) {
            regenerations++;
            regenerate_map();
            mvprintw(0, 0, "Map regenerated! You are now on floor %d.", regenerations + 1);
        } else if (regenerations >= MAX_REGENERATIONS) {
            mvprintw(0, 0, "Maximum map regenerations reached.");
        }

    } else {
        mvprintw(0, 0, "You cannot move there! That path is blocked.");
    }
    refresh(); // Refresh to show the message
}
   void loginUser() {
    char username[USERNAME_LENGTH];
    char password[PASSWORD_LENGTH];

    printw("Enter username: ");
    move(LINES - 1, 15);
    echo();
    scanw("%s", username);
    noecho();

    printw("Enter password: ");
    move(LINES - 1, 16);
    echo();
    scanw("%s", password);
    noecho();

    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0) {
            printw("Login successful.\n");

            GameState game_state;
            char filename[50];
            int game_loaded = 0;
            char load_or_new;

            printw("Do you want to load a saved game or start a new one? (L for load, N for new): ");
            scanw(" %c", &load_or_new);

            if (tolower(load_or_new) == 'l') {
                printw("Enter the name of the save file (e.g., save1, save2, ...): ");
                scanw("%s", filename);
                game_loaded = load_game(filename, &game_state);
            }

            if (!game_loaded) {
                srand(time(NULL));
                initialize_map();
                initialize_trap_map();
                int room_count = 0;

                while (room_count < MAX_ROOMS) {
                    Room new_room;
                    if (create_room(&new_room)) {
                        add_columns_to_room(&new_room);
                        if (room_count > 0)
                            connect_rooms(&rooms[room_count - 1], &new_room);
                        rooms[room_count++] = new_room;
                    }
                }

                for (int i = 0; i < room_count; i++) {
                    place_window_in_room(&rooms[i]);
                    add_traps_to_room(&rooms[i]);
                }

                place_single_special_characters(rooms, room_count);

                int hero_room = rand() % room_count;
                player_x = rooms[hero_room].x + 1 + rand() % (rooms[hero_room].width - 2);
                player_y = rooms[hero_room].y + 1 + rand() % (rooms[hero_room].height - 2);
                map[player_y][player_x] = '@';

                update_seen_tiles();
            }

            timeout(100); // Set a timeout for getch to make the game responsive

            char input;
            while (1) {
                print_map();
                printw("Use W/A/S/D to move horizontally and vertically\n");
                printw("Use Q/E/Z/C to move diagonally\nPress G to quit.\n");
                printw("Press P to save game.\n");
                refresh();
                input = getch();
                if (tolower(input) == 'g') break;
                if (tolower(input) == 'p') {
                    game_state.player_x = player_x;
                    game_state.player_y = player_y;
                    game_state.player_hp = player_hp;
                    memcpy(game_state.map, map, sizeof(map));
                    memcpy(game_state.trap_map, trap_map, sizeof(trap_map));
                    memcpy(game_state.rooms, rooms, sizeof(rooms));
                    game_state.room_count = room_count;

                    printw("Enter a name for your save file (e.g., save1, save2, ...): ");
                    move(LINES - 1, 0);
                    echo();
                    scanw("%s", filename);
                    noecho();
                    save_game(filename, &game_state);
                }
                move_player(input);
                if (player_hp <= 0) {
                    printw("Game Over! You ran out of HP.\n");
                    break;
                }
            }

            timeout(-1); // Reset timeout to blocking mode

            return;
        }
    }

    printw("Invalid username or password.\n");
}


void resetPassword() {
    char email[EMAIL_LENGTH];
    printw("Enter email: ");
    scanw("%s", email);

    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].email, email) == 0) {
            generateRandomPassword(users[i].password);
            printw("New password has been sent: %s\n", users[i].password);
            return;
        }
    }

    printw("Email not found.\n");
}

void recoverPassword() {
    char email[EMAIL_LENGTH];
    printw("Enter your registered email: ");
    scanw("%s", email);

    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].email, email) == 0) {
            char newPassword[PASSWORD_LENGTH];
            generateRandomPassword(newPassword);
            strcpy(users[i].password, newPassword);
            printw("Your new password is: %s\n", newPassword);

            // Update user data in file
            FILE *file = fopen(FILENAME, "wb");
            if (file) {
                fwrite(users, sizeof(User), userCount, file);
                fclose(file);
            } else {
                printw("Error updating user data in file.\n");
            }

            return;
        }
    }

    printw("Email not found. Please try again.\n");
}

void loadUsers() {
    FILE *file = fopen(FILENAME, "rb");
    if (file) {
        while (fread(&users[userCount], sizeof(User), 1, file)) {
            userCount++;
        }
        fclose(file);
    }
}

int main() {
    initscr(); // Initialize the ncurses screen
    noecho();  // Disable echoing of input characters
    cbreak();  // Disable line buffering
    srand(time(NULL)); // Initialize random seed for password generation

    // Load users from file when the program starts
    loadUsers();

    int choice;
    do {
        clear(); // Clear the screen
        printw("\nMenu:\n");
        printw("1. Create new user\n");
        printw("2. Generate random password\n");
        printw("3. User login\n");
        printw("4. Reset password\n");
        printw("5. Recover forgotten password\n");
        printw("6. Exit\n");
        printw("Enter your choice: ");
        refresh(); // Refresh to show the menu

        // Move the cursor to the next line for user input
        move(LINES - 1, 0);
        choice = getch() - '0'; // Read the user's choice without requiring Enter

        switch (choice) {
            case 1:
                createUser();
                break;
            case 2:
                generatePasswordForUser();
                break;
            case 3:
                loginUser();
                break;
            case 4:
                resetPassword();
                break;
            case 5:
                recoverPassword();
                break;
            case 6:
                printw("Exiting program.\n");
                break;
            default:
                printw("Invalid choice.\n");
                getch(); // Wait for user to press a key
        }
    } while (choice != 6);

    endwin(); // End the ncurses mode
    return 0;
}
