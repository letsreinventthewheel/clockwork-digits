#include <math.h>
#include <raylib.h>
#include <stddef.h>
#include <time.h>

constexpr static Color BACKGROUND_COLOR = (Color) { .r = 10, .g = 10, .b = 10, .a = 255 };
constexpr static Color ANALOG_CLOCK_BACKGROUND_COLOR = (Color) { .r = 40, .g = 40, .b = 40, .a = 255 };
constexpr static Color ANALOG_CLOCK_BORDER_COLOR = (Color) { .r = 70, .g = 70, .b = 70, .a = 255 };
constexpr static Color ANALOG_CLOCK_HAND_COLOR = (Color) { .r = 230, .g = 220, .b = 100, .a = 255 };

constexpr static float ANALOG_CLOCK_RADIUS = 20.0f;
constexpr static float ANALOG_CLOCK_GAP = 2.0f;

constexpr static size_t DIGITS_COUNT = 6;
constexpr static float DIGIT_GAP = 10.0f;
constexpr static float DIGIT_PAIR_GAP = 40.0f;

constexpr static float ANIMATION_SPEED = 10.0f;

constexpr static size_t DIGIT_ROWS = 6;
constexpr static size_t DIGIT_COLUMNS = 4;
constexpr static size_t ANALOG_CLOCKS_PER_DIGIT = DIGIT_ROWS * DIGIT_COLUMNS;

typedef struct {
    size_t h, m, s;
} ClockTime;

static ClockTime GetLocalTime(void) {
    time_t t = time(nullptr);
    struct tm* local = localtime(&t);
    return (ClockTime) { .h = local->tm_hour, .m = local->tm_min, .s = local->tm_sec };
}

constexpr static size_t TOTAL_ANALOG_CLOCK_HANDS_COUNT = DIGITS_COUNT * ANALOG_CLOCKS_PER_DIGIT * 2;
static float current_angles[TOTAL_ANALOG_CLOCK_HANDS_COUNT];
static float target_angles[TOTAL_ANALOG_CLOCK_HANDS_COUNT];

[[maybe_unused]]static char DIGITS[10][ANALOG_CLOCKS_PER_DIGIT] = {
    { 'a','-','-','b',
      '|','a','b','|',
      '|','|','|','|',
      '|','|','|','|',
      '|','c','d','|',
      'c','-','-','d', },

    { 'a','-','b',' ',
      'c','b','|',' ',
      ' ','|','|',' ',
      ' ','|','|',' ',
      'a','d','c','b',
      'c','-','-','d', },

    { 'a','-','-','b',
      'c','-','b','|',
      'a','-','d','|',
      '|','a','-','d',
      '|','c','-','b',
      'c','-','-','d', },

    { 'a','-','-','b',
      'c','-','b','|',
      ' ','a','d','|',
      ' ','c','b','|',
      'a','-','d','|',
      'c','-','-','d', },

    { 'a','b','a','b',
      '|','|','|','|',
      '|','c','d','|',
      'c','-','b','|',
      ' ',' ','|','|',
      ' ',' ','c','d', },

    { 'a','-','-','b',
      '|','a','-','d',
      '|','c','-','b',
      'c','-','b','|',
      'a','-','d','|',
      'c','-','-','d', },

    { 'a','-','-','b',
      '|','a','-','d',
      '|','c','-','b',
      '|','a','b','|',
      '|','c','d','|',
      'c','-','-','d', },

    { 'a','-','-','b',
      'c','-','b','|',
      ' ',' ','|','|',
      ' ',' ','|','|',
      ' ',' ','|','|',
      ' ',' ','c','d', },

    { 'a','-','-','b',
      '|','a','b','|',
      '|','c','d','|',
      '|','a','b','|',
      '|','c','d','|',
      'c','-','-','d', },

    { 'a','-','-','b',
      '|','a','b','|',
      '|','c','d','|',
      'c','-','b','|',
      'a','-','d','|',
      'c','-','-','d', },
};

static void DrawAnalogClockHand(Vector2 center, float radius, float angle) {
    float radians = angle * DEG2RAD;
    Vector2 hand_tip = (Vector2) {
        .x = center.x + cosf(radians) * (radius - 2.5f),
        .y = center.y + sinf(radians) * (radius - 2.5f),
    };
    DrawLineEx(center, hand_tip, 2.5f, ANALOG_CLOCK_HAND_COLOR);
}

static void DrawAnalogClock(Vector2 center, float radius, float angle1, float angle2) {
    DrawCircleV(center, radius, ANALOG_CLOCK_BACKGROUND_COLOR);
    DrawRing(center, radius - 2.5f, radius, 0.0f, 360.0f, 36, ANALOG_CLOCK_BORDER_COLOR);

    DrawCircleV(center, 2.5f, ANALOG_CLOCK_HAND_COLOR);

    DrawAnalogClockHand(center, radius, angle1);
    DrawAnalogClockHand(center, radius, angle2);
}

[[maybe_unused]]static void SymbolAngles(char symbol, float* angle1, float* angle2) {
    switch (symbol) {
        case '-': *angle1 =   0.0f; *angle2 = 180.0f; break;
        case '|': *angle1 =  90.0f; *angle2 = 270.0f; break;
        case 'a': *angle1 =   0.0f; *angle2 =  90.0f; break;
        case 'b': *angle1 =  90.0f; *angle2 = 180.0f; break;
        case 'c': *angle1 =   0.0f; *angle2 = 270.0f; break;
        case 'd': *angle1 = 180.0f; *angle2 = 270.0f; break;
        default:  *angle1 = 135.0f; *angle2 = 135.0f; break;
    }
}

static void DrawDigit(Vector2 digit_top_left, size_t digit_idx) {
    size_t offset = digit_idx * ANALOG_CLOCKS_PER_DIGIT * 2;

    for (size_t clock_idx = 0; clock_idx < ANALOG_CLOCKS_PER_DIGIT; clock_idx++) {
        size_t row = clock_idx / DIGIT_COLUMNS;
        size_t column = clock_idx % DIGIT_COLUMNS;

        Vector2 center = (Vector2) {
            .x = digit_top_left.x + column * (ANALOG_CLOCK_RADIUS * 2 + ANALOG_CLOCK_GAP) + ANALOG_CLOCK_RADIUS,
            .y = digit_top_left.y + row * (ANALOG_CLOCK_RADIUS * 2 + ANALOG_CLOCK_GAP) + ANALOG_CLOCK_RADIUS,
        };
        float angle1 = current_angles[offset + clock_idx * 2 + 0];
        float angle2 = current_angles[offset + clock_idx * 2 + 1];
        DrawAnalogClock(center, ANALOG_CLOCK_RADIUS, angle1, angle2);
    }
}

void DrawDigitalClock(void) {
    float digit_width = DIGIT_COLUMNS * ANALOG_CLOCK_RADIUS * 2 + (DIGIT_COLUMNS - 1) * ANALOG_CLOCK_GAP;
    float digit_height = DIGIT_ROWS * ANALOG_CLOCK_RADIUS * 2 + (DIGIT_ROWS - 1) * ANALOG_CLOCK_GAP;

    float digital_clock_width = DIGITS_COUNT * digit_width + 3 * DIGIT_GAP + 2 * DIGIT_PAIR_GAP;
    float digital_clock_height = digit_height;

    Vector2 digital_clock_top_left = (Vector2) {
        .x = GetScreenWidth() / 2.0 - digital_clock_width / 2.0f,
        .y = GetScreenHeight() / 2.0f - digital_clock_height / 2.0f,
    };

    for (size_t digit_idx = 0; digit_idx < DIGITS_COUNT; digit_idx++) {
        size_t pair_idx = digit_idx / 2; // 0 1 2
        size_t in_pair_idx = digit_idx % 2; // 0 1

        Vector2 digit_top_left = (Vector2) {
            .x = digital_clock_top_left.x + pair_idx * (digit_width * 2 + DIGIT_GAP + DIGIT_PAIR_GAP) + in_pair_idx * (digit_width + DIGIT_GAP),
            .y = digital_clock_top_left.y,
        };

        DrawDigit(digit_top_left, digit_idx);
    }
}

static void InitAngles(void) {
    for (size_t idx = 0; idx < TOTAL_ANALOG_CLOCK_HANDS_COUNT; idx++) {
        current_angles[idx] = 135.0f;
        target_angles[idx] = 135.0f;
    }
}

static void UpdateDigitTargetAngles(size_t digit_idx, size_t digit) {
    float angle1, angle2;
    size_t offset = digit_idx * ANALOG_CLOCKS_PER_DIGIT * 2;
    for (size_t clock_idx = 0; clock_idx < ANALOG_CLOCKS_PER_DIGIT; clock_idx++) {
        char symbol = DIGITS[digit][clock_idx];
        SymbolAngles(symbol, &angle1, &angle2);
        target_angles[offset + clock_idx * 2 + 0] = angle1;
        target_angles[offset + clock_idx * 2 + 1] = angle2;
    }
}

static void UpdateTargetAngles(ClockTime t) {
    UpdateDigitTargetAngles(0, t.h / 10);
    UpdateDigitTargetAngles(1, t.h % 10);
    UpdateDigitTargetAngles(2, t.m / 10);
    UpdateDigitTargetAngles(3, t.m % 10);
    UpdateDigitTargetAngles(4, t.s / 10);
    UpdateDigitTargetAngles(5, t.s % 10);
}

static float LerpAngle(float current, float target, float speed, float dt) {
    float diff = fmodf(target - current + 360 + 180, 360) - 180; // (0, 720) => [0, 360) => [-180, 180)
    if (diff < 0.5f) {
        return target;
    }
    return current + diff * fminf(speed * dt, 1.0);
}

static void UpdateCurrentAngles(float dt) {
    for (size_t idx = 0; idx < TOTAL_ANALOG_CLOCK_HANDS_COUNT; idx++) {
        current_angles[idx] = LerpAngle(current_angles[idx], target_angles[idx], ANIMATION_SPEED, dt);
    }
}

int main(void) {
    InitWindow(1200, 350, "Clockwork Digits");
    SetTargetFPS(60);

    InitAngles();

    ClockTime prev = GetLocalTime();
    UpdateTargetAngles(prev);

    while (!WindowShouldClose()) {
        ClockTime now = GetLocalTime();
        if (now.s != prev.s) {
            UpdateTargetAngles(now);
            prev = now;
        }

        float dt = GetFrameTime();
        UpdateCurrentAngles(dt);

        BeginDrawing();
            ClearBackground(BACKGROUND_COLOR);
            DrawDigitalClock();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
