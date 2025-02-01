
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
#define WIDTH 100
#define HEIGHT 40
#define MAX_ROOMS 9
#define VISION_RADIUS 5
#define MAX_REGENERATIONS 4 // حداکثر تعداد بازسازی نقشه
#define TOTAL_GOLD 10 // تعداد کل طلای معمولی
#define TOTAL_GOLD_BAGS 6 // تعداد کل کیسه‌های طلای
#define TOTAL_BLACK_GOLD 3 // تعداد کل طلای سیاه
#define MAX_INVENTORY_ITEMS 100
#define ITEM_NAME_LENGTH 50
#define TOTAL_FOOD 10 // تعداد کل غذای معمولی
#define TOTAL_SUPER_FOOD 6 // تعداد کل غذای اعلا
#define TOTAL_MAGIC_FOOD 4 // تعداد کل غذای جادویی
#define TOTAL_SPEED_SPELL 4
#define COLOR_RED 1
#define COLOR_GREEN 2
#define COLOR_YELLOW 3
#define COLOR_BLUE 4
#define COLOR_MAGENTA 5
#define COLOR_BROWN 6
#define SAVE_LIST_FILE "save_list.txt" // فایل لیست نام‌های ذخیره
#define TOTAL_DEMONS 10 // تعداد کل شیاطین
#define TOTAL_FIRE 7
#define TOTAL_SNAKES 5
#define TOTAL_GIANTS 4
#define TOTAL_UNDEAD 2
int player_color = COLOR_RED; // رنگ پیش‌فرض بازیکن
int selected_music = 0; // موزیک انتخابی، 0: هیچ موزیکی انتخاب نشده
int placed_magic_food = 0; // شمارش غذای جادویی قرار داده شده
int placed_speed_spell = 0;
int placed_super_food = 0; // شمارش غذای اعلا قرار داده شده
int magic_food_active = 0; // 0: غیرفعال، 1: فعال
int speed_spell_active = 0;
int magic_food_moves = 0; // تعداد حرکت‌های باقی‌مانده از نوع غذای جادویی
int speed_spell_moves = 0;
int placed_food = 0; // شمارش غذای قرار داده شده
typedef struct {
    int x, y;        // موقعیت هیولا
    int hp;          // سلامتی هیولا
    int active;      // آیا هیولا فعال است یا خیر
    int room_index;  // شاخص اتاق که هیولا در آن قرار دارد
    int move_count;
} Monster;


Monster demons[TOTAL_DEMONS];
int demon_count = 0;
Monster undeads[TOTAL_UNDEAD];
int undead_count = 0;
Monster giants[TOTAL_GIANTS];
int giant_count = 0;
Monster snakes[TOTAL_SNAKES];
int snake_count = 0;
Monster firemonsters[TOTAL_FIRE];
int fire_count = 0;
typedef struct {
    char name[ITEM_NAME_LENGTH];
    char type[ITEM_NAME_LENGTH]; // نوع آیتم مانند غذا، اسلحه، طلسم
    int damage;
} Item;

Item inventory[MAX_INVENTORY_ITEMS];
int inventory_count = 0;
int inventory_open = 0; // 0: بسته، 1: باز
char current_weapon[ITEM_NAME_LENGTH] = "Mace"; // سلاح فعلی بازیکن

int placed_black_gold = 0; // شمارش طلای سیاه قرار داده شده

int placed_gold_bags = 0; // شمارش کیسه‌های طلای قرار داده شده
char current_message[100] = ""; // متغیری برای ذخیره پیام فعلی
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
    int has_demon;
    int has_firemonster;
    int has_snake;
    int has_giant;
    int has_undead;
} Room;

char hit_message[25] = "You hit a column!";
int show_message = 0; // 1 if the message is active, 0 otherwise
int player_level = 1;  // Starts with level 1
int player_gold = 0;   // Starts with 0 gold
int player_hp = 24;    // Starts with 12 HP
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
void add_demons_to_rooms(Room *rooms, int room_count) {
    for (int i = 0; i < TOTAL_DEMONS; i++) {
        int room_index;
        int attempts = 0; // شمارنده تلاش‌ها برای جلوگیری از حلقه بی‌نهایت
        do {
            room_index = rand() % room_count;
            attempts++;
        } while (rooms[room_index].has_demon && attempts < 100); // اتاق جدید انتخاب می‌شود تا وقتی که شیطان در آن اتاق نباشد

        if (attempts >= 100) {
            // در صورت نبود اتاق‌های خالی، شیطان جدید اضافه نمی‌شود
            continue;
        }

        Monster demon;
        do {
            demon.x = rooms[room_index].x + 1 + rand() % (rooms[room_index].width - 2);
            demon.y = rooms[room_index].y + 1 + rand() % (rooms[room_index].height - 2);
        } while (map[demon.y][demon.x] != '.'); // اطمینان از قرارگیری شیطان در داخل اتاق

        demon.hp = 5;
        demon.active = 0; // در ابتدا غیرفعال است
        demon.room_index = room_index; // شاخص اتاق

        demons[demon_count++] = demon;
        map[demon.y][demon.x] = 'D'; // اضافه کردن شیطان به نقشه
        rooms[room_index].has_demon = 1; // علامت‌گذاری اتاق به عنوان اتاق دارای شیطان
    }
}
void add_undeads_to_rooms(Room *rooms, int room_count) {
    for (int i = 0; i < TOTAL_UNDEAD; i++) {
        int room_index;
        int attempts = 0; // شمارنده تلاش‌ها برای جلوگیری از حلقه بی‌نهایت
        do {
            room_index = rand() % room_count;
            attempts++;
        } while (rooms[room_index].has_undead && attempts < 100); // اتاق جدید انتخاب می‌شود تا وقتی که شیطان در آن اتاق نباشد

        if (attempts >= 100) {
            // در صورت نبود اتاق‌های خالی، شیطان جدید اضافه نمی‌شود
            continue;
        }

        Monster undead;
        do {
            undead.x = rooms[room_index].x + 1 + rand() % (rooms[room_index].width - 2);
            undead.y = rooms[room_index].y + 1 + rand() % (rooms[room_index].height - 2);
        } while (map[undead.y][undead.x] != '.'); // اطمینان از قرارگیری شیطان در داخل اتاق

        undead.hp = 30;
        undead.active = 0; // در ابتدا غیرفعال است
        undead.room_index = room_index; // شاخص اتاق
        undead.move_count = 0; // شمارنده حرکت‌ها

        undeads[undead_count++] = undead;
        map[undead.y][undead.x] = 'U'; // اضافه کردن شیطان به نقشه
        rooms[room_index].has_undead = 1; // علامت‌گذاری اتاق به عنوان اتاق دارای شیطان
    }
}
void add_giants_to_rooms(Room *rooms, int room_count) {
    for (int i = 0; i < TOTAL_GIANTS; i++) {
        int room_index;
        int attempts = 0; // شمارنده تلاش‌ها برای جلوگیری از حلقه بی‌نهایت
        do {
            room_index = rand() % room_count;
            attempts++;
        } while (rooms[room_index].has_giant && attempts < 100); // اتاق جدید انتخاب می‌شود تا وقتی که شیطان در آن اتاق نباشد

        if (attempts >= 100) {
            // در صورت نبود اتاق‌های خالی، شیطان جدید اضافه نمی‌شود
            continue;
        }

        Monster giant;
        do {
            giant.x = rooms[room_index].x + 1 + rand() % (rooms[room_index].width - 2);
            giant.y = rooms[room_index].y + 1 + rand() % (rooms[room_index].height - 2);
        } while (map[giant.y][giant.x] != '.'); // اطمینان از قرارگیری شیطان در داخل اتاق

        giant.hp = 30;
        giant.active = 0; // در ابتدا غیرفعال است
        giant.room_index = room_index; // شاخص اتاق
        giant.move_count = 0; // شمارنده حرکت‌ها

        giants[giant_count++] = giant;
        map[giant.y][giant.x] = 'G'; // اضافه کردن شیطان به نقشه
        rooms[room_index].has_giant = 1; // علامت‌گذاری اتاق به عنوان اتاق دارای شیطان
    }
}

void add_snakes_to_rooms(Room *rooms, int room_count) {
    for (int i = 0; i < TOTAL_SNAKES; i++) {
        int room_index;
        int attempts = 0; // شمارنده تلاش‌ها برای جلوگیری از حلقه بی‌نهایت
        do {
            room_index = rand() % room_count;
            attempts++;
        } while (rooms[room_index].has_snake && attempts < 100); // اتاق جدید انتخاب می‌شود تا وقتی که شیطان در آن اتاق نباشد

        if (attempts >= 100) {
            // در صورت نبود اتاق‌های خالی، شیطان جدید اضافه نمی‌شود
            continue;
        }

        Monster snake;
        do {
            snake.x = rooms[room_index].x + 1 + rand() % (rooms[room_index].width - 2);
            snake.y = rooms[room_index].y + 1 + rand() % (rooms[room_index].height - 2);
        } while (map[snake.y][snake.x] != '.'); // اطمینان از قرارگیری شیطان در داخل اتاق

        snake.hp = 20;
        snake.active = 0; // در ابتدا غیرفعال است
        snake.room_index = room_index; // شاخص اتاق

        snakes[snake_count++] = snake;
        map[snake.y][snake.x] = 'S'; // اضافه کردن شیطان به نقشه
        rooms[room_index].has_snake = 1; // علامت‌گذاری اتاق به عنوان اتاق دارای شیطان
    }
}
void add_firemonster_to_rooms(Room *rooms, int room_count) {
    for (int i = 0; i < TOTAL_FIRE; i++) {
        int room_index;
        int attempts = 0; // شمارنده تلاش‌ها برای جلوگیری از حلقه بی‌نهایت
        do {
            room_index = rand() % room_count;
            attempts++;
        } while (rooms[room_index].has_firemonster && attempts < 100); // اتاق جدید انتخاب می‌شود تا وقتی که شیطان در آن اتاق نباشد

        if (attempts >= 100) {
            // در صورت نبود اتاق‌های خالی، شیطان جدید اضافه نمی‌شود
            continue;
        }

        Monster firemonster;
        do {
            firemonster.x = rooms[room_index].x + 1 + rand() % (rooms[room_index].width - 2);
            firemonster.y = rooms[room_index].y + 1 + rand() % (rooms[room_index].height - 2);
        } while (map[firemonster.y][firemonster.x] != '.'); // اطمینان از قرارگیری شیطان در داخل اتاق

        firemonster.hp = 10;
        firemonster.active = 0; // در ابتدا غیرفعال است
        firemonster.room_index = room_index; // شاخص اتاق

        firemonsters[fire_count++] = firemonster;
        if(map[firemonster.y][firemonster.x] == 'D')
        break; 
        map[firemonster.y][firemonster.x] = 'F'; // اضافه کردن شیطان به نقشه
        rooms[room_index].has_firemonster = 1; // علامت‌گذاری اتاق به عنوان اتاق دارای شیطان
    }
}


void move_demon(Monster *demon) {
    if (!demon->active) return;

    int dx = 0, dy = 0;
    if (player_x > demon->x) dx = 1;
    if (player_x < demon->x) dx = -1;
    if (player_y > demon->y) dy = 1;
    if (player_y < demon->y) dy = -1;

    int new_x = demon->x + dx;
    int new_y = demon->y + dy;

    if (map[new_y][new_x] == ' ' || map[new_y][new_x] == '.' || map[new_y][new_x] == 'f' || map[new_y][new_x] == 'j' || map[new_y][new_x] == 'm') {
        map[demon->y][demon->x] = '.'; // پاک کردن محل قبلی شیطان
        demon->x = new_x;
        demon->y = new_y;
        map[new_y][new_x] = 'D'; // جایگذاری شیطان در محل جدید
    }
}
void move_undead(Monster *undead) {
    if (!undead->active || undead->move_count >= 5) return; // توقف حرکت اگر شمارنده به ۵ برسد

    int dx = 0, dy = 0;
    if (player_x > undead->x) dx = 1;
    if (player_x < undead->x) dx = -1;
    if (player_y > undead->y) dy = 1;
    if (player_y < undead->y) dy = -1;

    int new_x = undead->x + dx;
    int new_y = undead->y + dy;

    if (map[new_y][new_x] == ' ' || map[new_y][new_x] == '.' || map[new_y][new_x] == 'f' || map[new_y][new_x] == 'j' || map[new_y][new_x] == 'm') {
        map[undead->y][undead->x] = '.'; // پاک کردن محل قبلی شیطان
        undead->x = new_x;
        undead->y = new_y;
        map[new_y][new_x] = 'U'; // جایگذاری شیطان در محل جدید
        undead->move_count++; // افزایش شمارنده حرکت‌ها
    }
}

void move_giant(Monster *giant) {
    if (!giant->active || giant->move_count >= 5) return; // توقف حرکت اگر شمارنده به ۵ برسد

    int dx = 0, dy = 0;
    if (player_x > giant->x) dx = 1;
    if (player_x < giant->x) dx = -1;
    if (player_y > giant->y) dy = 1;
    if (player_y < giant->y) dy = -1;

    int new_x = giant->x + dx;
    int new_y = giant->y + dy;

    if (map[new_y][new_x] == ' ' || map[new_y][new_x] == '.' || map[new_y][new_x] == 'f' || map[new_y][new_x] == 'j' || map[new_y][new_x] == 'm') {
        map[giant->y][giant->x] = '.'; // پاک کردن محل قبلی شیطان
        giant->x = new_x;
        giant->y = new_y;
        map[new_y][new_x] = 'G'; // جایگذاری شیطان در محل جدید
        giant->move_count++; // افزایش شمارنده حرکت‌ها
    }
}

void move_snake(Monster *snake) {
    if (!snake->active) return;

    int dx = 0, dy = 0;
    if (player_x > snake->x) dx = 1;
    if (player_x < snake->x) dx = -1;
    if (player_y > snake->y) dy = 1;
    if (player_y < snake->y) dy = -1;

    int new_x = snake->x + dx;
    int new_y = snake->y + dy;

    if (map[new_y][new_x] == 'O'|| map[new_y][new_x] == '.' || map[new_y][new_x] == 'f' || map[new_y][new_x] == 'j' || map[new_y][new_x] == 'm') {
        map[snake->y][snake->x] = '.'; // پاک کردن محل قبلی شیطان
        snake->x = new_x;
        snake->y = new_y;
        map[new_y][new_x] = 'S'; // جایگذاری شیطان در محل جدید
    }
}
void move_firemonster(Monster *firemonster) {
    if (!firemonster->active) return;

    int dx = 0, dy = 0;
    if (player_x > firemonster->x) dx = 1;
    if (player_x < firemonster->x) dx = -1;
    if (player_y > firemonster->y) dy = 1;
    if (player_y < firemonster->y) dy = -1;

    int new_x = firemonster->x + dx;
    int new_y = firemonster->y + dy;

    if ( map[new_y][new_x] == ' ' || map[new_y][new_x] == '.' || map[new_y][new_x] == 'f' || map[new_y][new_x] == 'j' || map[new_y][new_x] == 'm') {
        map[firemonster->y][firemonster->x] = '.'; // پاک کردن محل قبلی شیطان
        firemonster->x = new_x;
        firemonster->y = new_y;
        map[new_y][new_x] = 'F'; // جایگذاری شیطان در محل جدید
    }
}

void activate_demons() {
    for (int i = 0; i < demon_count; i++) {
        if (player_x >= rooms[demons[i].room_index].x && player_x < rooms[demons[i].room_index].x + rooms[demons[i].room_index].width &&
            player_y >= rooms[demons[i].room_index].y && player_y < rooms[demons[i].room_index].y + rooms[demons[i].room_index].height) {
            demons[i].active = 1; // فعال کردن شیطان در صورت ورود بازیکن به اتاق
        } else {
            demons[i].active = 0; // غیرفعال کردن شیطان در صورت خروج بازیکن از اتاق
        }
    }
}
void activate_undead() {
    for (int i = 0; i < undead_count; i++) {
        if (player_x >= rooms[undeads[i].room_index].x && player_x < rooms[undeads[i].room_index].x + rooms[undeads[i].room_index].width &&
            player_y >= rooms[undeads[i].room_index].y && player_y < rooms[undeads[i].room_index].y + rooms[undeads[i].room_index].height) {
            if (undeads[i].move_count < 5) { // فقط اگر شمارنده حرکت کمتر از ۵ باشد
                undeads[i].active = 1; // فعال کردن شیطان
            }
        } else {
            undeads[i].active = 0; // غیرفعال کردن شیطان در صورت خروج بازیکن از اتاق
        }
    }
}

void activate_giants() {
    for (int i = 0; i < giant_count; i++) {
        if (player_x >= rooms[giants[i].room_index].x && player_x < rooms[giants[i].room_index].x + rooms[giants[i].room_index].width &&
            player_y >= rooms[giants[i].room_index].y && player_y < rooms[giants[i].room_index].y + rooms[giants[i].room_index].height) {
            if (giants[i].move_count < 5) { // فقط اگر شمارنده حرکت کمتر از ۵ باشد
                giants[i].active = 1; // فعال کردن شیطان
            }
        } else {
            giants[i].active = 0; // غیرفعال کردن شیطان در صورت خروج بازیکن از اتاق
        }
    }
}

void activate_snakes() {
    for (int i = 0; i < snake_count; i++) {
        if (player_x >= rooms[snakes[i].room_index].x && player_x < rooms[snakes[i].room_index].x + rooms[snakes[i].room_index].width &&
            player_y >= rooms[snakes[i].room_index].y && player_y < rooms[snakes[i].room_index].y + rooms[snakes[i].room_index].height) {
            snakes[i].active = 1; // فعال کردن شیطان در صورت ورود بازیکن به اتاق
        } 
        else {
           // snakes[i].active = 0; // غیرفعال کردن شیطان در صورت خروج بازیکن از اتاق
        }
    }
}
void activate_firemonter() {
    for (int i = 0; i < fire_count; i++) {
        if (player_x >= rooms[firemonsters[i].room_index].x && player_x < rooms[firemonsters[i].room_index].x + rooms[firemonsters[i].room_index].width &&
            player_y >= rooms[firemonsters[i].room_index].y && player_y < rooms[firemonsters[i].room_index].y + rooms[firemonsters[i].room_index].height) {
            firemonsters[i].active = 1; // فعال کردن شیطان در صورت ورود بازیکن به اتاق
        } 
        else {
           firemonsters[i].active = 0; // غیرفعال کردن شیطان در صورت خروج بازیکن از اتاق
        }
    }
}

void update_demons() {
    for (int i = 0; i < demon_count; i++) {
        move_demon(&demons[i]);
        if (abs(player_x - demons[i].x) <= 1 && abs(player_y - demons[i].y) <= 1) {
            player_hp -= 2; // کاهش HP بازیکن
            snprintf(current_message, sizeof(current_message), "A demon attacked you! Your HP is now %d.", player_hp);
        }
    }
}
void update_undead() {
    for (int i = 0; i < undead_count; i++) {
        move_undead(&undeads[i]);
        if (abs(player_x - undeads[i].x) <= 1 && abs(player_y - undeads[i].y) <= 1) {
            player_hp -= 5; // کاهش HP بازیکن
            snprintf(current_message, sizeof(current_message), "An undeads attacked you! Your HP is now %d.", player_hp);
        }
    }
}
void update_giantss() {
    for (int i = 0; i < giant_count; i++) {
        move_giant(&giants[i]);
        if (abs(player_x - giants[i].x) <= 1 && abs(player_y - giants[i].y) <= 1) {
            player_hp -= 3; // کاهش HP بازیکن
            snprintf(current_message, sizeof(current_message), "A giant attacked you! Your HP is now %d.", player_hp);
        }
    }
}
void update_snkaes() {
    for (int i = 0; i < snake_count; i++) {
        move_snake(&snakes[i]);
        if (abs(player_x - snakes[i].x) <= 1 && abs(player_y - snakes[i].y) <= 1) {
            if(player_str/16<= 4){
            player_hp -= (4 - (player_str/16)); // کاهش HP بازیکن
            snprintf(current_message, sizeof(current_message), "A snake attacked you! Your HP is now %d.", player_hp);
         }
        }
    }
}
void update_firemonster() {
    for (int i = 0; i < fire_count; i++) {
        move_firemonster(&firemonsters[i]);
        if (abs(player_x - firemonsters[i].x) <= 1 && abs(player_y - firemonsters[i].y) <= 1) {
            player_hp -= 3; // کاهش HP بازیکن
            snprintf(current_message, sizeof(current_message), "A Firemonter attacked you! Your HP is now %d.", player_hp);
        }
    }
}

void attack_with_mace() {
    int directions[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1}, // بالا چپ، بالا، بالا راست
        {0, -1},          {0, 1},   // چپ، راست
        {1, -1}, {1, 0}, {1, 1}     // پایین چپ، پایین، پایین راست
    };
       int damage = 0;
    if (strcmp(current_weapon, "Mace") == 0) {
        damage = 5;
    } else if (strcmp(current_weapon, "Sword") == 0) {
        damage = 10;
    }
    else if (strcmp(current_weapon, "Dagger")== 0){
        damage = 12;
    }
    else if (strcmp(current_weapon, "Magic Wand")== 0){
        damage = 15;
    }
    else if (strcmp(current_weapon, "Arrow") == 0){
        damage = 5;
    }


    for (int i = 0; i < 8; i++) {
        int new_x = player_x + directions[i][0];
        int new_y = player_y + directions[i][1];

        if (new_x >= 0 && new_x < WIDTH && new_y >= 0 && new_y < HEIGHT) {
            for (int j = 0; j < demon_count; j++) {
                if (demons[j].x == new_x && demons[j].y == new_y) {
                    demons[j].hp -= damage; // آسیب به HP شیطان
                    if (demons[j].hp <= 0) {
                        map[demons[j].y][demons[j].x] = '.'; // حذف شیطان از نقشه
                        demons[j] = demons[demon_count - 1]; // جایگزین کردن شیطان حذف شده با آخرین شیطان در آرایه
                        demon_count--; // کاهش شمارنده شیاطین
                        snprintf(current_message, sizeof(current_message), "You killed a demon!");
                        player_exp += 10;
                    } else {
                        snprintf(current_message, sizeof(current_message), "You hit a demon! Its HP is now %d.", demons[j].hp);
                    }
                }
            }
        }
    }

   for (int i = 0; i < 8; i++) {
        int new_x = player_x + directions[i][0];
        int new_y = player_y + directions[i][1];

        if (new_x >= 0 && new_x < WIDTH && new_y >= 0 && new_y < HEIGHT) {
            for (int j = 0; j < fire_count; j++) {
                if (firemonsters[j].x == new_x && firemonsters[j].y == new_y) {
                    firemonsters[j].hp -= damage; // آسیب به HP شیطان
                    if (firemonsters[j].hp <= 0) {
                        map[firemonsters[j].y][firemonsters[j].x] = '.'; // حذف شیطان از نقشه
                        firemonsters[j] = firemonsters[fire_count - 1]; // جایگزین کردن شیطان حذف شده با آخرین شیطان در آرایه
                        fire_count--; // کاهش شمارنده شیاطین
                        snprintf(current_message, sizeof(current_message), "You killed a firemonter!");
                        player_exp += 20;
                    } else {
                        snprintf(current_message, sizeof(current_message), "You hit a firemonter! Its HP is now %d.", firemonsters[j].hp);
                    }
                }
            }
        }
    }
     for (int i = 0; i < 8; i++) {
        int new_x = player_x + directions[i][0];
        int new_y = player_y + directions[i][1];

        if (new_x >= 0 && new_x < WIDTH && new_y >= 0 && new_y < HEIGHT) {
            for (int j = 0; j < fire_count; j++) {
                if (snakes[j].x == new_x && snakes[j].y == new_y) {
                    snakes[j].hp -= damage; // آسیب به HP شیطان
                    if (snakes[j].hp <= 0) {
                        map[snakes[j].y][snakes[j].x] = '.'; // حذف شیطان از نقشه
                        snakes[j] = snakes[snake_count-1]; // جایگزین کردن شیطان حذف شده با آخرین شیطان در آرایه
                        snake_count--; // کاهش شمارنده شیاطین
                        snprintf(current_message, sizeof(current_message), "You killed a snake!");
                        player_exp += 40;
                    } else {
                        snprintf(current_message, sizeof(current_message), "You hit a snake! Its HP is now %d.", snakes[j].hp);
                    }
                }
            }
        }
    }
       for (int i = 0; i < 8; i++) {
        int new_x = player_x + directions[i][0];
        int new_y = player_y + directions[i][1];

        if (new_x >= 0 && new_x < WIDTH && new_y >= 0 && new_y < HEIGHT) {
            for (int j = 0; j < giant_count; j++) {
                if (giants[j].x == new_x && giants[j].y == new_y) {
                    giants[j].hp -= damage; // آسیب به HP شیطان
                    if (giants[j].hp <= 0) {
                        map[giants[j].y][giants[j].x] = '.'; // حذف شیطان از نقشه
                        giants[j] = giants[giant_count - 1]; // جایگزین کردن شیطان حذف شده با آخرین شیطان در آرایه
                        giant_count--; // کاهش شمارنده شیاطین
                        snprintf(current_message, sizeof(current_message), "You killed a giant!");
                        player_exp += 10;
                    } else {
                        snprintf(current_message, sizeof(current_message), "You hit a giant! Its HP is now %d.", giants[j].hp);
                    }
                }
            }
        }
    }
        for (int i = 0; i < 8; i++) {
        int new_x = player_x + directions[i][0];
        int new_y = player_y + directions[i][1];

        if (new_x >= 0 && new_x < WIDTH && new_y >= 0 && new_y < HEIGHT) {
            for (int j = 0; j < undead_count; j++) {
                if (undeads[j].x == new_x && undeads[j].y == new_y) {
                    undeads[j].hp -= damage; // آسیب به HP شیطان
                    if (undeads[j].hp <= 0) {
                        map[undeads[j].y][undeads[j].x] = '.'; // حذف شیطان از نقشه
                        undeads[j] = undeads[undead_count - 1]; // جایگزین کردن شیطان حذف شده با آخرین شیطان در آرایه
                        undead_count--; // کاهش شمارنده شیاطین
                        snprintf(current_message, sizeof(current_message), "You killed an undead!");
                        player_exp += 50;
                    } else {
                        snprintf(current_message, sizeof(current_message), "You hit an undead! Its HP is now %d.", undeads[j].hp);
                    }
                }
            }
        }
    }
    refresh(); // Refresh to show the message
}
void add_swords_to_rooms(Room *rooms, int room_count) {
    int sword_count = 0;
    while (sword_count < 2) { // دو شمشیر اضافه کنید
        int room_index = rand() % room_count;
        int sword_x, sword_y;
        do {
            sword_x = rooms[room_index].x + 1 + rand() % (rooms[room_index].width - 2);
            sword_y = rooms[room_index].y + 1 + rand() % (rooms[room_index].height - 2);
        } while (map[sword_y][sword_x] != '.'); // اطمینان از قرارگیری شمشیر در داخل اتاق

        map[sword_y][sword_x] = '!'; // اضافه کردن شمشیر به نقشه
        sword_count++;
    }
}
void add_daggers_to_rooms(Room *rooms, int room_count) {
    int dagger_count = 0;
    while (dagger_count < 2) { // دو شمشیر اضافه کنید
        int room_index = rand() % room_count;
        int dagger_x, dagger_y;
        do {
            dagger_x = rooms[room_index].x + 1 + rand() % (rooms[room_index].width - 2);
            dagger_y = rooms[room_index].y + 1 + rand() % (rooms[room_index].height - 2);
        } while (map[dagger_y][dagger_x] != '.'); // اطمینان از قرارگیری شمشیر در داخل اتاق

        map[dagger_y][dagger_x] = 'k'; // اضافه کردن شمشیر به نقشه
        dagger_count++;
    }
}
void add_arrows_to_rooms(Room *rooms, int room_count) {
    int arrow_count = 0;
    while (arrow_count < 2) { // دو شمشیر اضافه کنید
        int room_index = rand() % room_count;
        int arrow_x, arrow_y;
        do {
            arrow_x = rooms[room_index].x + 1 + rand() % (rooms[room_index].width - 2);
            arrow_y = rooms[room_index].y + 1 + rand() % (rooms[room_index].height - 2);
        } while (map[arrow_y][arrow_x] != '.'); // اطمینان از قرارگیری شمشیر در داخل اتاق

        map[arrow_y][arrow_x] = 'y'; // اضافه کردن شمشیر به نقشه
        arrow_count++;
    }
}

void add_wands_to_rooms(Room *rooms, int room_count) {
    int wand_count = 0;
    while (wand_count < 1) { // دو شمشیر اضافه کنید
        int room_index = rand() % room_count;
        int wand_x, wand_y;
        do {
            wand_x = rooms[room_index].x + 1 + rand() % (rooms[room_index].width - 2);
            wand_y = rooms[room_index].y + 1 + rand() % (rooms[room_index].height - 2);
        } while (map[wand_y][wand_x] != '.'); // اطمینان از قرارگیری شمشیر در داخل اتاق

        map[wand_y][wand_x] = 'w'; // اضافه کردن شمشیر به نقشه
        wand_count++;
    }
}


void save_game(const char* filename, GameState* game_state) {
    FILE *file = fopen(filename, "wb");
    if (file) {
        fwrite(game_state, sizeof(GameState), 1, file);
        fclose(file);
        printw("Game saved successfully.\n");
    } else {
        printw("Error: Unable to save the game.\n");
    }
}
int load_game(const char* filename, GameState* game_state) {
    FILE *file = fopen(filename, "rb");
    if (file) {
        fread(game_state, sizeof(GameState), 1, file);
        fclose(file);
        return 1; // موفقیت‌آمیز بودن لود بازی
    } else {
        printw("Error: Unable to load the game.\n");
        return 0; // شکست لود بازی
    }
}

void add_save_file_to_list(const char* filename) {
    FILE *file = fopen(SAVE_LIST_FILE, "a");
    if (file) {
        fprintf(file, "%s\n", filename);
        fclose(file);
    }
}

void load_save_list() {
    FILE *file = fopen(SAVE_LIST_FILE, "r");
    if (file) {
        char line[50];
        printw("Saved games:\n");
        while (fgets(line, sizeof(line), file)) {
            printw("%s", line);
        }
        fclose(file);
    } else {
        printw("No saved games found.\n");
    }
}

void init_colors() {
    if (has_colors()) {
        start_color();
        init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
        init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
        init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
        init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
        init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(COLOR_BROWN, COLOR_BROWN, COLOR_BLACK);

        // ... سایر رنگ‌ها
    }
}
void stop_music() {
    system("pkill mpg123"); // متوقف کردن موزیک
}

void settings() {
    clear();
    printw("Select your player color:\n");
    printw("1. Red\n");
    printw("2. Green\n");
    printw("3. Yellow\n");
    printw("4. Blue\n");
    printw("5. Magenta\n");
    printw("Enter your choice: ");
    refresh();
    
    int choice = getch() - '0';
    switch (choice) {
        case 1:
            player_color = COLOR_RED;
            break;
        case 2:
            player_color = COLOR_GREEN;
            break;
        case 3:
            player_color = COLOR_YELLOW;
            break;
        case 4:
            player_color = COLOR_BLUE;
            break;
        case 5:
            player_color = COLOR_MAGENTA;
            break;
        default:
            printw("Invalid choice. Using default color (Yellow).\n");
            player_color = COLOR_YELLOW;
            getch(); // Wait for user to press a key
            break;
    }

    clear();
    printw("Select your background music:\n");
    printw("1. Music 1\n");
    printw("2. Music 2\n");
    printw("3. Music 3\n");
    printw("4.stop the music\n");
    printw("Enter your choice: ");
    refresh();

    choice = getch() - '0';
    switch (choice) {
        case 1:
            selected_music = 1;
            break;
        case 2:
            selected_music = 2;
            break;
        case 3:
            selected_music = 3;
            break;
        case 4:
            stop_music();
            break;
            
        default:
            printw("Invalid choice. No music selected.\n");
            selected_music = 0;
            getch(); // Wait for user to press a key
            break;
    }
}


void add_food_to_room(Room *room) {
    for (int i = 0; i < 1; i++) { // Add 1 food per room
        int food_x, food_y;
        do {
            food_x = room->x + 1 + rand() % (room->width - 2);
            food_y = room->y + 1 + rand() % (room->height - 2);
        } while (map[food_y][food_x] == 'f'); // Ensure we don't place food on another food

        map[food_y][food_x] = 'f'; // Place food in the map, استفاده از 'F' به جای 🍎
        placed_food++;
    }
}
void add_super_food_to_room(Room *room) {
    for (int i = 0; i < 1; i++) { // Add 1 super food per room
        int super_food_x, super_food_y;
        do {
            super_food_x = room->x + 1 + rand() % (room->width - 2);
            super_food_y = room->y + 1 + rand() % (room->height - 2);
        } while (map[super_food_y][super_food_x] == 'j'); // Ensure we don't place super food on another super food

        map[super_food_y][super_food_x] = 'j'; // Place super food in the map
        placed_super_food++;
    }
}
void add_magic_food_to_room(Room *room) {
    for (int i = 0; i < 1; i++) { // Add 1 magic food per room
        int magic_food_x, magic_food_y;
        do {
            magic_food_x = room->x + 1 + rand() % (room->width - 2);
            magic_food_y = room->y + 1 + rand() % (room->height - 2);
        } while (map[magic_food_y][magic_food_x] == 'm'); // Ensure we don't place magic food on another magic food

        map[magic_food_y][magic_food_x] = 'm'; // Place magic food in the map
        placed_magic_food++;
    }
}
void add_speed_spell_to_room(Room *room) {
    for (int i = 0; i < 1; i++) { // Add 1 magic food per room
        int speed_spell_x, speed_spell_y;
        do {
            speed_spell_x = room->x + 1 + rand() % (room->width - 2);
            speed_spell_y = room->y + 1 + rand() % (room->height - 2);
        } while (map[speed_spell_y][speed_spell_x] == 'X'); // Ensure we don't place magic food on another magic food

        map[speed_spell_y][speed_spell_x] = 'X'; // Place magic food in the map
        placed_speed_spell++;
    }
}

void saveUsers() {
    FILE *file = fopen("users.txt", "w");
    if (file) {
        for (int i = 0; i < userCount; i++) {
            fprintf(file, "%s %s %s\n", users[i].username, users[i].password, users[i].email);
        }
        fclose(file);
    } else {
        printw("Error: Unable to save users.\n");
    }
}



int findUser(const char* username) {
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, username) == 0) {
            return i;
        }
    }
    return -1;
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
    char username[USERNAME_LENGTH];
    char password[PASSWORD_LENGTH];
    char email[EMAIL_LENGTH];
    int has_uppercase = 0; // برای بررسی وجود حرف بزرگ

    clear(); // پاک کردن صفحه برای شروع
    printw("Enter new username: ");
    echo();
    scanw("%s", username);
    noecho();

    // Check if username already exists
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, username) == 0) {
            clear(); // پاک کردن صفحه برای نمایش پیام
            printw("Username already taken. Please choose another one.\n");
            refresh(); // تازه‌سازی صفحه برای نمایش پیام
            getch(); // منتظر ماندن برای فشار دادن کلید توسط کاربر
            return;
        }
    }

    printw("Enter new password (at least 7 characters long and must contain at least one uppercase letter): ");
    echo();
    scanw("%s", password);
    noecho();

    // Check if password is at least 7 characters long and contains at least one uppercase letter
    if (strlen(password) < 7) {
        clear(); // پاک کردن صفحه برای نمایش پیام
        printw("Password must be at least 7 characters long.\n");
        refresh(); // تازه‌سازی صفحه برای نمایش پیام
        getch(); // منتظر ماندن برای فشار دادن کلید توسط کاربر
        return;
    }

    for (int i = 0; i < strlen(password); i++) {
        if (isupper(password[i])) {
            has_uppercase = 1;
            break;
        }
    }

    if (!has_uppercase) {
        clear(); // پاک کردن صفحه برای نمایش پیام
        printw("Password must contain at least one uppercase letter.\n");
        refresh(); // تازه‌سازی صفحه برای نمایش پیام
        getch(); // منتظر ماندن برای فشار دادن کلید توسط کاربر
        return;
    }

    printw("Enter email address: ");
    echo();
    scanw("%s", email);
    noecho();

    // Validate email address (simple validation for example purposes)
    if (!strchr(email, '@') || !strchr(email, '.')) {
        clear(); // پاک کردن صفحه برای نمایش پیام
        printw("Invalid email address. Please try again.\n");
        refresh(); // تازه‌سازی صفحه برای نمایش پیام
        getch(); // منتظر ماندن برای فشار دادن کلید توسط کاربر
        return;
    }

    strcpy(users[userCount].username, username);
    strcpy(users[userCount].password, password);
    strcpy(users[userCount].email, email);
    userCount++;
    
    saveUsers(); // ذخیره کاربران
    
    clear(); // پاک کردن صفحه برای نمایش پیام
    printw("New user created successfully.\n");
    refresh(); // تازه‌سازی صفحه برای نمایش پیام
    getch(); // منتظر ماندن برای فشار دادن کلید توسط کاربر
}


void generatePassword(char* password) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t length = 8;

    for (size_t i = 0; i < length; i++) {
        int key = rand() % (int)(sizeof(charset) - 1);
        password[i] = charset[key];
    }
    password[length] = '\0';
}

void generatePasswordForUser() {
    char username[USERNAME_LENGTH];
    char password[PASSWORD_LENGTH];

    clear(); // پاک کردن صفحه برای شروع
    printw("Enter the username: ");
    echo();
    scanw("%s", username);
    noecho();

    int userIndex = findUser(username);
    if (userIndex == -1) {
        clear(); // پاک کردن صفحه برای نمایش پیام
        printw("Error: Username not found.\n");
        refresh(); // تازه‌سازی صفحه برای نمایش پیام
        getch(); // منتظر ماندن برای فشار دادن کلید توسط کاربر
        return;
    }

    generatePassword(password);

    strcpy(users[userIndex].password, password);
    saveUsers(); // ذخیره کاربران

    clear(); // پاک کردن صفحه برای نمایش پیام
    printw("New password generated successfully for user %s: %s\n", username, password);
    refresh(); // تازه‌سازی صفحه برای نمایش پیام
    getch(); // منتظر ماندن برای فشار دادن کلید توسط کاربر
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
    room->width = 4 + rand() % 35; // Room width (4 to 25)
    room->height = 4 + rand() % 10; // Room height (4 to 7)
    room->x = 1 + rand() % (WIDTH - room->width - 1); // Avoid map border
    room->y = 1 + rand() % (HEIGHT - room->height - 1);
    
    room->has_demon = 0; // مقداردهی اولیه به 0
    room->has_firemonster = 0; // مقداردهی اولیه به 0
    room->has_snake = 0;
    room->has_giant = 0;
    room->has_undead = 0;
  
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
            if (i == 0 && show_message) {
                if (j < (int)strlen(hit_message)) {
                    printw("%c", hit_message[j]);
                } else {
                    printw(" ");
                }
            } else if (i == player_y && j == player_x) {
                attron(COLOR_PAIR(player_color));
                printw("%c", '@');
                attroff(COLOR_PAIR(player_color));
            } else if (map[i][j] == '&') {
                attron(COLOR_PAIR(COLOR_YELLOW));
                printw("%c", map[i][j]);
                attroff(COLOR_PAIR(COLOR_YELLOW));
            } else if (map[i][j] == 'B') {
                attron(COLOR_PAIR(COLOR_BROWN));
                printw("%c", map[i][j]);
                attroff(COLOR_PAIR(COLOR_BROWN));
            } else if (map[i][j] == 'f') {
                attron(COLOR_PAIR(COLOR_GREEN));
                printw("%c", map[i][j]);
                attroff(COLOR_PAIR(COLOR_GREEN));
            } 
             else if (seen_map[i][j] != ' ') {
                printw("%c", seen_map[i][j]);
            } else {
                printw(" ");
            }
        }

        if (i == HEIGHT - 1) {
            printw("Lvl: %d  Gold: %d  HP: %d  Str: %d  Arm: %d  Exp: %d", 
                   player_level, player_gold, player_hp, player_str, player_arm, player_exp);

            int remaining_space = WIDTH - 56;
            for (int k = 0; k < remaining_space; k++) {
                printw(" ");
            }
        }

        printw("\n");
    }

    // چاپ پیام فعلی
    mvprintw(HEIGHT, 0, "%s", current_message);

    refresh(); // Refresh to show the message
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
        if(map[trap_y][trap_x] != '>' || map[trap_y][trap_x] != '<')
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
void add_gold_bag_to_room(Room *room) {
    for (int i = 0; i < 1; i++) { // Add 1 gold bag per room
        int gold_bag_x, gold_bag_y;
        do {
            gold_bag_x = room->x + 1 + rand() % (room->width - 2);
            gold_bag_y = room->y + 1 + rand() % (room->height - 2);
        } while (map[gold_bag_y][gold_bag_x] == '&'); // Ensure we don't place gold bag on another gold bag

        map[gold_bag_y][gold_bag_x] = '&'; // Place gold bag in the map
    }
}

void add_black_gold_to_room(Room *room) {
    for (int i = 0; i < 1; i++) { // Add 1 black gold per room
        int black_gold_x, black_gold_y;
        do {
            black_gold_x = room->x + 1 + rand() % (room->width - 2);
            black_gold_y = room->y + 1 + rand() % (room->height - 2);
        } while (map[black_gold_y][black_gold_x] == 'B'); // Ensure we don't place black gold on another black gold

        map[black_gold_y][black_gold_x] = 'B'; // Place black gold in the map
    }
}
void add_to_inventory(const char* item_name, const char* item_type) {
    for (int i = 0; i < inventory_count; i++) {
        if (strcmp(inventory[i].name, item_name) == 0 && strcmp(inventory[i].type, item_type) == 0) {
            // آیتم قبلاً در اینونتوری وجود دارد، نیازی به افزودن مجدد نیست
            return;
        }
    }

    if (inventory_count < MAX_INVENTORY_ITEMS) {
        strncpy(inventory[inventory_count].name, item_name, ITEM_NAME_LENGTH - 1);
        strncpy(inventory[inventory_count].type, item_type, ITEM_NAME_LENGTH - 1);
        if (strcmp(item_name, "Sword") == 0) {
            inventory[inventory_count].damage = 8; // مقدار آسیب شمشیر
        } else if (strcmp(item_name, "Mace") == 0) {
            inventory[inventory_count].damage = 5; // مقدار آسیب گرز
        }
        inventory_count++;
    } else {
        printw("Inventory is full. Cannot add more items.\n");
    }
}


void add_mace_to_inventory() {
    add_to_inventory("Mace", "Weapon");
}
void add_sword_to_inventory() {
    add_to_inventory("Sword", "Weapon");
    for (int i = 0; i < inventory_count; i++) {
        if (strcmp(inventory[i].name, "Sword") == 0) {
            inventory[i].damage = 8; // تنظیم مقدار آسیب شمشیر
        }
    }
}
void add_dagger_to_inventory() {
    add_to_inventory("Dagger", "Weapon");
    for (int i = 0; i < inventory_count; i++) {
        if (strcmp(inventory[i].name, "Dagger") == 0) {
            inventory[i].damage = 8; // تنظیم مقدار آسیب شمشیر
        }
    }
}
void add_arrow_to_inventory() {
    add_to_inventory("Arrow", "Weapon");
    for (int i = 0; i < inventory_count; i++) {
        if (strcmp(inventory[i].name, "Arrow") == 0) {
            inventory[i].damage = 8; // تنظیم مقدار آسیب شمشیر
        }
    }
}
void add_wand_to_inventory() {
    add_to_inventory("Magic Wand", "Weapon");
    for (int i = 0; i < inventory_count; i++) {
        if (strcmp(inventory[i].name, "Magic Wand") == 0) {
            inventory[i].damage = 8; // تنظیم مقدار آسیب شمشیر
        }
    }
}

void show_inventory() {
    clear();
    printw("Inventory:\n");

    // نمایش سلاح‌ها
    printw("Weapons:\n");
    for (int i = 0; i < inventory_count; i++) {
        if (strcmp(inventory[i].type, "Weapon") == 0) {
            printw("%d. %s (%s)\n", i + 1, inventory[i].name, inventory[i].type);
        }
    }

    // نمایش غذاها
    printw("Consumables:\n");
    for (int i = 0; i < inventory_count; i++) {
        if (strcmp(inventory[i].type, "Consumable") == 0) {
            printw("%d. %s (%s)\n", i + 1, inventory[i].name, inventory[i].type);
        }
    }
 printw("Spells:\n");
    for (int i = 0; i < inventory_count; i++) {
        if (strcmp(inventory[i].type, "Spells") == 0) {
            printw("%d. %s (%s)\n", i + 1, inventory[i].name, inventory[i].type);
        }
    }

    printw("Press the number of the item to use it, or press 'i' to close the inventory and return to the game.\n");
    refresh();





    int ch;
    while ((ch = getch()) != 'i') {
        ch -= '0';
        if (ch >= 1 && ch <= inventory_count) {
            if (strcmp(inventory[ch - 1].type, "Weapon") == 0) {
                strncpy(current_weapon, inventory[ch - 1].name, ITEM_NAME_LENGTH - 1);
                snprintf(current_message, sizeof(current_message), "You equipped %s.", current_weapon);
            } else if (strcmp(inventory[ch - 1].name, "Food") == 0)
              {
                player_hp += 5;
                snprintf(current_message, sizeof(current_message), "You used food! Your HP is now %d.", player_hp);
                // حذف غذا از inventory پس از استفاده
                for (int j = ch - 1; j < inventory_count - 1; j++) {
                    inventory[j] = inventory[j + 1];
                }
                inventory_count--;
            } else if (strcmp(inventory[ch - 1].name, "Super Food") == 0) {
                player_hp += 8; // افزایش HP
                player_str += 16; // افزایش Strength
                snprintf(current_message, sizeof(current_message), "You used super food! Your HP is now %d, and your strength is now %d.", player_hp, player_str);
                // حذف غذای اعلا از inventory پس از استفاده
                for (int j = ch - 1; j < inventory_count - 1; j++) {
                    inventory[j] = inventory[j + 1];
                }
                inventory_count--;
            } else if (strcmp(inventory[ch - 1].name, "Magic Food") == 0) {
                player_hp += 8; // افزایش HP
                magic_food_moves = 7; // مقداردهی به تعداد حرکت‌های باقی‌مانده از نوع غذای جادویی
                snprintf(current_message, sizeof(current_message), "You used magic food! Your HP is now %d. You have 7 magic moves.", player_hp);
                // حذف غذای جادویی از inventory پس از استفاده
                for (int j = ch - 1; j < inventory_count - 1; j++) {
                    inventory[j] = inventory[j + 1];
                }
                inventory_count--;
            } 
            else if (strcmp(inventory[ch - 1].name, "Speed Spells") == 0) {
                speed_spell_moves = 7; // مقداردهی به تعداد حرکت‌های باقی‌مانده از نوع غذای جادویی
                snprintf(current_message, sizeof(current_message), "You used speed spell . You have 7 magic moves.");
                // حذف غذای جادویی از inventory پس از استفاده
                for (int j = ch - 1; j < inventory_count - 1; j++) {
                    inventory[j] = inventory[j + 1];
                }
                inventory_count--;
            }else {
                printw("You used %s.\n", inventory[ch - 1].name);
                refresh();
                getch(); // Wait for the user to press a key to acknowledge using the item
            }
        }
    }
    clear();
    inventory_open = 0; // بستن inventory
}


void regenerate_map() {
    // تنظیم مقدار اولیه HP بازیکن
    player_hp = 24;


    // بازنشانی موقعیت شیاطین
    demon_count = 0;
    memset(demons, 0, sizeof(demons));
    undead_count = 0;
    memset(undeads, 0, sizeof(undeads));

    giant_count = 0;
    memset(giants, 0, sizeof(giants));

    fire_count = 0;
    memset(firemonsters, 0, sizeof(firemonsters));
    snake_count = 0;
    memset(snakes, 0 ,sizeof(snakes));
    // بازسازی نقشه
    initialize_map();

    int room_count = 0;
    int gold_count = 0; // شمارش طلای معمولی
    int food_count = 0; // شمارش غذای معمولی
    int super_food_count = 0; // شمارش غذای اعلا
    int magic_food_count = 0; // شمارش غذای جادویی
    int speed_spell_count = 0;
    placed_gold_bags = 0; // شمارش کیسه‌های طلای قرار داده شده
    placed_black_gold = 0; // شمارش طلای سیاه قرار داده شده

    // Generate rooms
    while (room_count < MAX_ROOMS) {
        Room new_room;
        if (create_room(&new_room)) {
            add_columns_to_room(&new_room);
            if (room_count > 0)
                connect_rooms(&rooms[room_count - 1], &new_room);
            rooms[room_count++] = new_room;
        }
    }

    // Add windows, traps, gold, food, gold bags, black gold, super food, and magic food to rooms
    for (int i = 0; i < room_count; i++) {
        place_window_in_room(&rooms[i]);
        add_traps_to_room(&rooms[i]); // اضافه کردن تله‌ها به اتاق‌ها

        // Add gold to rooms
        if (gold_count < TOTAL_GOLD) {
            add_gold_to_room(&rooms[i]);
            gold_count++;
        }

        // Add food to rooms
        if (food_count < TOTAL_FOOD) {
            add_food_to_room(&rooms[i]);
            food_count++;
        }

        // Add super food to rooms
        if (super_food_count < TOTAL_SUPER_FOOD) {
            add_super_food_to_room(&rooms[i]);
            super_food_count++;
        }

        // Add magic food to rooms
        if (magic_food_count < TOTAL_MAGIC_FOOD) {
            add_magic_food_to_room(&rooms[i]);
            magic_food_count++;
        }
          // Add magic food to rooms
        if (speed_spell_count < TOTAL_SPEED_SPELL) {
            add_speed_spell_to_room(&rooms[i]);
            speed_spell_count++;
        }

        // Add gold bags to rooms
        if (placed_gold_bags < TOTAL_GOLD_BAGS) {
            add_gold_bag_to_room(&rooms[i]);
            placed_gold_bags++;
        }

        // Add black gold to rooms
        if (placed_black_gold < TOTAL_BLACK_GOLD) {
            add_black_gold_to_room(&rooms[i]);
            placed_black_gold++;
        }
    }

    // اضافه کردن شیطان به اتاق‌ها
    add_demons_to_rooms(rooms, room_count);
    add_firemonster_to_rooms(rooms,room_count);
    add_undeads_to_rooms(rooms, room_count);
    add_giants_to_rooms(rooms,room_count);
    add_snakes_to_rooms(rooms,room_count);
   add_swords_to_rooms(rooms,room_count);
    add_daggers_to_rooms(rooms,room_count);
   add_wands_to_rooms(rooms, room_count);
    add_arrows_to_rooms(rooms,room_count);
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
    int dx = 0, dy = 0; // تعریف متغیرهای dx و dy

    if (tolower(input) == 'i') {
        if (inventory_open) {
            clear();
            inventory_open = 0; // بستن inventory
        } else {
            show_inventory();
            inventory_open = 0; // باز کردن inventory
        }
        return; // خروج از تابع تا زمانی که باز و بسته شدن inventory مدیریت شود
    }

    if (inventory_open) {
        // اگر inventory باز است، حرکت بازیکن را متوقف کنید
        return;
    }

    if (tolower(input) == 'h') { // فرض بر این که کلید 'h' برای حمله با گرز انتخاب شده است
        attack_with_mace();
        return;
    }

    // پاک کردن پیام قبلی
    //clear_message(); 

    switch (tolower(input)) {
        case 'w': dy = -1; break; // حرکت به بالا
        case 's': dy = 1; break; // حرکت به پایین
        case 'a': dx = -1; break; // حرکت به چپ
        case 'd': dx = 1; break; // حرکت به راست
        case 'q': dx = -1; dy = -1; break; // حرکت به بالا چپ
        case 'e': dx = 1; dy = -1; break; // حرکت به بالا راست
        case 'z': dx = -1; dy = 1; break; // حرکت به پایین چپ
        case 'c': dx = 1; dy = 1; break; // حرکت به پایین راست
        default: return;
    }

    if (magic_food_moves > 0 || speed_spell_moves > 0) {
        // حرکت در جهت مشخص شده تا برخورد به مانع
        while (1) {
            int new_x = player_x + dx;
            int new_y = player_y + dy;

            // بررسی محدوده آرایه‌ها
            if (new_x < 0 || new_x >= WIDTH || new_y < 0 || new_y >= HEIGHT || map[new_y][new_x] == '#' || map[new_y][new_x] == '|' || map[new_y][new_x] == ' '|| map[new_y][new_x] == '=' || map[new_y][new_x] == '+') {
                break; // برخورد به مانع یا خروج از محدوده
            }

            char destination_tile = map[new_y][new_x];
            if (destination_tile == ' ' || destination_tile == '.' || destination_tile == '+' || destination_tile == '<' || destination_tile == '>' || destination_tile == 'g' || destination_tile == '&' || destination_tile == 'B' || destination_tile == 'f' || destination_tile == 'j' || destination_tile == 'm' || destination_tile =='!'|| destination_tile == 'k'|| destination_tile == 'w'|| destination_tile == 'y' || destination_tile =='X') {
                map[player_y][player_x] = '.';
                player_x = new_x;
                player_y = new_y;
                map[player_y][player_x] = '@';
                update_seen_tiles();

                // Check for traps
                if (trap_map[player_y][player_x] == 'T') {
                    player_hp -= 1; // کاهش HP
                    snprintf(current_message, sizeof(current_message), "You stepped on a trap! Your HP is now %d.", player_hp);
                    
                }

                // Check for gold
                else if (destination_tile == 'g') {
                    player_gold += 5; // افزایش طلا
                    snprintf(current_message, sizeof(current_message), "You picked up gold! Your gold is now %d.", player_gold);
                }

                // Check for gold bags
                else if (destination_tile == '&') {
                    player_gold += 10; // افزایش طلا
                    snprintf(current_message, sizeof(current_message), "You picked up a gold bag! Your gold is now %d.", player_gold);
                }

                // Check for black gold
                else if (destination_tile == 'B') {
                    player_gold += 20; // افزایش طلا
                    snprintf(current_message, sizeof(current_message), "You picked up black gold! Your gold is now %d.", player_gold);
                }

                // Check for food
                else if (destination_tile == 'f') {
                    add_to_inventory("Food", "Consumable");
                    snprintf(current_message, sizeof(current_message), "You picked up food!");
                }

                // Check for super food
                else if (destination_tile == 'j') {
                    add_to_inventory("Super Food", "Consumable");
                    snprintf(current_message, sizeof(current_message), "You picked up super food!");
                }

                // Check for magic food
                else if (destination_tile == 'm') {
                    add_to_inventory("Magic Food", "Consumable");
                    snprintf(current_message, sizeof(current_message), "You picked up magic food!");
                }
                 else if (destination_tile == 'X') {
                    add_to_inventory("Speed Spell", "Spells");
                    snprintf(current_message, sizeof(current_message), "You picked up speed spell!");
                }
                    // کدهای موجود برای مدیریت حرکت و غیره...

                   else if (destination_tile == '!') {
                  add_sword_to_inventory();
                 snprintf(current_message, sizeof(current_message), "You picked up a sword!");
                }
                else if (destination_tile == 'k'){
                    add_dagger_to_inventory();
                     snprintf(current_message, sizeof(current_message), "You picked up a dagger!");
                }
                 else if (destination_tile == 'w'){
                    add_wand_to_inventory();
                     snprintf(current_message, sizeof(current_message), "You picked up a magic wand!");
                }
                  else if (destination_tile == 'y'){
                    add_arrow_to_inventory();
                     snprintf(current_message, sizeof(current_message), "You picked up a arrow!");
                }


    // کدهای موجود برای مدیریت بقیه شرایط...



                // Check for stairs and regenerate map if needed
                else if ((destination_tile == '>' || destination_tile == '<') && regenerations < MAX_REGENERATIONS) {
                    regenerations++;
                    regenerate_map();
                    snprintf(current_message, sizeof(current_message), "Map regenerated! You are now on floor %d.", regenerations + 1);
                } else if (regenerations >= MAX_REGENERATIONS) {
                    snprintf(current_message, sizeof(current_message), "Maximum map regenerations reached.");
                    
                }
            } else {
                snprintf(current_message, sizeof(current_message), "You cannot move there! That path is blocked.");
                break;
            }

            // چاپ پیام فعلی
            mvprintw(HEIGHT, 0, "%s", current_message);
            refresh(); // Refresh to show the message
        }
        magic_food_moves--;
        speed_spell_moves--; // کاهش تعداد حرکت‌های باقی‌مانده از نوع غذای جادویی
    } else {
        int new_x = player_x + dx;
        int new_y = player_y + dy;

        // بررسی محدوده آرایه‌ها
        if (new_x < 0 || new_x >= WIDTH || new_y < 0 || new_y >= HEIGHT) {
            snprintf(current_message, sizeof(current_message), "You cannot move there! That path is blocked.");
            mvprintw(HEIGHT, 0, "%s", current_message);
            refresh();
            return;
        }

        char destination_tile = map[new_y][new_x];
        if (destination_tile == '#' || destination_tile == '.' || destination_tile == '+' || destination_tile == '<' || destination_tile == '>' || destination_tile == 'g' || destination_tile == '&' || destination_tile == 'B' || destination_tile == 'f' || destination_tile == 'j' || destination_tile == 'm' || destination_tile == '!' || destination_tile == 'k'|| destination_tile == 'w'|| destination_tile == 'y' || destination_tile == 'X')  {
            map[player_y][player_x] = '.';
            player_x = new_x;
            player_y = new_y;
            map[player_y][player_x] = '@';
            update_seen_tiles();

            // Check for traps
            if (trap_map[player_y][player_x] == 'T') {
                player_hp -= 1; // کاهش HP
                snprintf(current_message, sizeof(current_message), "You stepped on a trap! Your HP is now %d.", player_hp);
            }

            // Check for gold
            else if (destination_tile == 'g') {
                player_gold += 5; // افزایش طلا
                snprintf(current_message, sizeof(current_message), "You picked up gold! Your gold is now %d.", player_gold);
            }

            // Check for gold bags
            else if (destination_tile == '&') {
                player_gold += 10; // افزایش طلا
                snprintf(current_message, sizeof(current_message), "You picked up a gold bag! Your gold is now %d.", player_gold);
            }

            // Check for black gold
            else if (destination_tile == 'B') {
                player_gold += 20; // افزایش طلا
                snprintf(current_message, sizeof(current_message), "You picked up black gold! Your gold is now %d.", player_gold);
            }

            // Check for food
            else if (destination_tile == 'f') {
                add_to_inventory("Food", "Consumable");
                snprintf(current_message, sizeof(current_message), "You picked up food!");
            }

            // Check for super food
            else if (destination_tile == 'j') {
                add_to_inventory("Super Food", "Consumable");
                snprintf(current_message, sizeof(current_message), "You picked up super food!");
            }

            // Check for magic food
            else if (destination_tile == 'm') {
                add_to_inventory("Magic Food", "Consumable");
                snprintf(current_message, sizeof(current_message), "You picked up magic food!");
            }
             // Check for magic food
            else if (destination_tile == 'X') {
                add_to_inventory("Speed Spell", "Spells");
                snprintf(current_message, sizeof(current_message), "You picked up speed spell!");
            }
            else if (destination_tile == '!') {
                  add_sword_to_inventory();
                 snprintf(current_message, sizeof(current_message), "You picked up a sword!");
                }
            else if (destination_tile == 'k'){
                    add_dagger_to_inventory();
                     snprintf(current_message, sizeof(current_message), "You picked up a dagger!");
                }
                 else if (destination_tile == 'w'){
                    add_wand_to_inventory();
                     snprintf(current_message, sizeof(current_message), "You picked up a wand!");
                }
                  else if (destination_tile == 'y'){
                    add_arrow_to_inventory();
                     snprintf(current_message, sizeof(current_message), "You picked up a arrow!");
                }

            // Check for stairs and regenerate map if needed
            else if ((destination_tile == '>' || destination_tile == '<') && regenerations < MAX_REGENERATIONS) {
                regenerations++;
                regenerate_map();
                snprintf(current_message, sizeof(current_message), "Map regenerated! You are now on floor %d.", regenerations + 1);
            } else if (regenerations >= MAX_REGENERATIONS) {
                snprintf(current_message, sizeof(current_message), "Maximum map regenerations reached.");
            }

        } else {
            snprintf(current_message, sizeof(current_message), "You cannot move there! That path is blocked.");
        }

        // چاپ پیام فعلی
        mvprintw(HEIGHT, 0, "%s", current_message);

        refresh(); // Refresh to show the message
    }

    activate_demons(); // فعال کردن شیاطین در صورت ورود بازیکن به اتاق
    activate_firemonter();
    activate_snakes();
    activate_giants();
    activate_undead();
    update_demons(); // بروزرسانی موقعیت شیاطین
    update_snkaes();
    update_firemonster();
    update_giantss();
    update_undead();
}

void play_music() {
    switch (selected_music) {
        case 1:
            system("mpg123 -q music1.mp3 &");
            break;
        case 2:
            system("mpg123 -q music2.mp3 &");
            break;
        case 3:
            system("mpg123 -q music3.mp3 &");
            break;
        case 4:
            system("mpg123 -q my_music.mp3 &"); // پخش موزیک جدید
            break;
        default:
            break; // هیچ موزیکی انتخاب نشده
    }
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
                clear();
                load_save_list();
                printw("Enter the name of the save file to load (e.g., save1, save2, ...): ");
                scanw("%s", filename);
                game_loaded = load_game(filename, &game_state);

                if (game_loaded) {
                    // بازیابی داده‌های بازی از فایل
                    player_x = game_state.player_x;
                    player_y = game_state.player_y;
                    player_hp = game_state.player_hp;
                    memcpy(map, game_state.map, sizeof(game_state.map));
                    memcpy(trap_map, game_state.trap_map, sizeof(game_state.trap_map));
                    memcpy(rooms, game_state.rooms, sizeof(game_state.rooms));
                    room_count = game_state.room_count;
                    update_seen_tiles();
                }
            }

            if (!game_loaded) {
                srand(time(NULL));
                initialize_map();
                initialize_trap_map();
                int room_count = 0;

                // تنظیم مقدار اولیه HP بازیکن
                player_hp = 24;

                // بازنشانی موقعیت شیاطین
                demon_count = 0;
                memset(demons, 0, sizeof(demons));
                undead_count = 0;
                memset(undeads, 0, sizeof(undeads));
                giant_count = 0;
                memset(giants, 0, sizeof(giants));

                snake_count = 0;
                memset(snakes, 0,sizeof(snakes));
                fire_count = 0;
                memset(firemonsters, 0, sizeof(firemonsters));
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

                // اضافه کردن گرز به موجودی بازیکن
                add_mace_to_inventory();

                update_seen_tiles();
            }

            play_music();

            char input;
            while (1) {
                print_map();
                printw("Use W/A/S/D to move horizontally and vertically\n");
                printw("Use Q/E/Z/C to move diagonally\nPress G to quit.\n");
                printw("Press P to save game.\n");
                printw("Press O to open settings.\n");
                printw("Press H to attack with mace.\n"); // راهنمایی برای استفاده از گرز
                refresh();
                input = getch();
                if (tolower(input) == 'o') {
                    stop_music();
                    flushinp();
                    settings();
                    play_music();
                }
                if (tolower(input) == 'g') {
                    stop_music();
                    break;}
                if (tolower(input) == 'p') {
                    flushinp();
                    printw("Enter a name for your save file (e.g., save1, save2, ...): ");
                    move(LINES - 1, 0);
                    echo();
                    scanw("%s", filename);
                    noecho();
                    add_save_file_to_list(filename);
                    game_state.player_x = player_x;
                    game_state.player_y = player_y;
                    game_state.player_hp = player_hp;
                    memcpy(game_state.map, map, sizeof(map));
                    memcpy(game_state.trap_map, trap_map, sizeof(trap_map));
                    memcpy(game_state.rooms, rooms, sizeof(rooms));
                    game_state.room_count = room_count;
                    save_game(filename, &game_state);
                    clear();
                    printw("Game saved successfully as %s.\n", filename);
                    refresh();
                    getch();
                }
                move_player(input);
                if (player_hp <= 0) {
                    clear();
                    printw("Game Over! You ran out of HP.\n");
                    
                    refresh();
                    getch();
                    stop_music();
                    break;
                }
            }

            return;
        }
    }

    clear();
    printw("Invalid username or password.\n");
    refresh();
    getch();
}

void resetPassword() {
    char username[USERNAME_LENGTH];
    char oldPassword[PASSWORD_LENGTH];
    char newPassword[PASSWORD_LENGTH];

    clear(); // پاک کردن صفحه برای شروع
    printw("Enter your username: ");
    echo();
    scanw("%s", username);
    noecho();

    printw("Enter your old password: ");
    echo();
    scanw("%s", oldPassword);
    noecho();

    int userIndex = findUser(username);
    if (userIndex == -1 || strcmp(users[userIndex].password, oldPassword) != 0) {
        clear(); // پاک کردن صفحه برای نمایش پیام
        printw("Error: Invalid username or password.\n");
        refresh(); // تازه‌سازی صفحه برای نمایش پیام
        getch(); // منتظر ماندن برای فشار دادن کلید توسط کاربر
        return;
    }

    printw("Enter your new password: ");
    echo();
    scanw("%s", newPassword);
    noecho();

    strcpy(users[userIndex].password, newPassword);
    saveUsers(); // ذخیره کاربران

    clear(); // پاک کردن صفحه برای نمایش پیام
    printw("Password reset successfully.\n");
    refresh(); // تازه‌سازی صفحه برای نمایش پیام
    getch(); // منتظر ماندن برای فشار دادن کلید توسط کاربر
}


void recoverPassword() {
    char username[USERNAME_LENGTH];
    char email[EMAIL_LENGTH];

    clear(); // پاک کردن صفحه برای شروع
    printw("Enter your username: ");
    echo();
    scanw("%s", username);
    noecho();

    printw("Enter your email: ");
    echo();
    scanw("%s", email);
    noecho();

    // Check if username and email match
    int userIndex = -1;
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].email, email) == 0) {
            userIndex = i;
            break;
        }
    }

    if (userIndex == -1) {
        clear(); // پاک کردن صفحه برای نمایش پیام
        printw("Error: Username or email not found.\n");
        refresh(); // تازه‌سازی صفحه برای نمایش پیام
        getch(); // منتظر ماندن برای فشار دادن کلید توسط کاربر
        return;
    }

    clear(); // پاک کردن صفحه برای نمایش پیام
    printw("Your password is: %s\n", users[userIndex].password);
    refresh(); // تازه‌سازی صفحه برای نمایش پیام
    getch(); // منتظر ماندن برای فشار دادن کلید توسط کاربر
}

void loadUsers() {
    FILE *file = fopen("users.txt", "r");
    if (file) {
        userCount = 0; // Reset user count before loading
        while (fscanf(file, "%s %s %s", users[userCount].username, users[userCount].password, users[userCount].email) != EOF) {
            userCount++;
        }
        fclose(file);
    } else {
        printw("No users found. You may need to create a new user.\n");
    }
}

int main() {
    initscr(); // Initialize the ncurses screen
    noecho();  // Disable echoing of input characters
    cbreak();  // Disable line buffering
    srand(time(NULL)); // Initialize random seed for password generation

    // Initialize colors
    init_colors();

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
        printw("6. Settings\n"); // افزودن گزینه تنظیمات
        printw("7. Stop Music\n"); // افزودن گزینه توقف موزیک
        printw("8. Exit\n");
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
                settings(); // فراخوانی تنظیمات
                break;
            case 7:
                stop_music(); // فراخوانی توقف موزیک
                break;
            case 8:
                clear(); // پاک کردن صفحه برای نمایش پیام
                printw("Exiting program.\n");
                refresh(); // تازه‌سازی صفحه برای نمایش پیام
                getch(); // منتظر ماندن برای فشار دادن کلید توسط کاربر
                break;
            default:
                clear(); // پاک کردن صفحه برای نمایش پیام
                printw("Invalid choice.\n");
                refresh(); // تازه‌سازی صفحه برای نمایش پیام
                getch(); // منتظر ماندن برای فشار دادن کلید توسط کاربر
        }
    } while (choice != 8);

    endwin(); // End the ncurses mode
    return 0;
}
