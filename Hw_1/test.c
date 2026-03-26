#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

/* =========================================================
   ANSI 색상 코드
   C_B* : Bold(굵게), C_BG* : 배경색
   ========================================================= */
#define C_RESET    "\033[0m"
#define C_BOLD     "\033[1m"
#define C_RED      "\033[31m"
#define C_GREEN    "\033[32m"
#define C_YELLOW   "\033[33m"
#define C_BLUE     "\033[34m"
#define C_MAGENTA  "\033[35m"
#define C_CYAN     "\033[36m"
#define C_WHITE    "\033[37m"
#define C_BRED     "\033[1;31m"
#define C_BGREEN   "\033[1;32m"
#define C_BYELLOW  "\033[1;33m"
#define C_BBLUE    "\033[1;34m"
#define C_BMAGENTA "\033[1;35m"
#define C_BCYAN    "\033[1;36m"
#define C_BWHITE   "\033[1;37m"
#define C_BGRED    "\033[41m"
#define C_BGGREEN  "\033[42m"
#define C_BGYELLOW "\033[43m"

/* =========================================================
   상수 정의
   패(가위/바위/보)는 0~2 인덱스로 통일하여 배열 접근에 활용
   ========================================================= */
#define SCISSORS      0   /* 가위 인덱스 */
#define ROCK          1   /* 바위 인덱스 */
#define PAPER         2   /* 보   인덱스 */
#define ENEMY_COUNT   7   /* 총 상대 수 */
#define ITEM_COUNT    5   /* 아이템 슬롯 수 (가위/바위/보/액티브/패시브) */
#define STARTER_COUNT 9   /* 전체 스타터 수 (해금 포함) */
#define SLOT_SCISSORS 0   /* item_lv[] 가위 슬롯 인덱스 */
#define SLOT_ROCK     1   /* item_lv[] 바위 슬롯 인덱스 */
#define SLOT_PAPER    2   /* item_lv[] 보   슬롯 인덱스 */
#define SLOT_ACTIVE   3   /* item_lv[] 액티브(독심술) 슬롯 인덱스 */
#define SLOT_PASSIVE  4   /* item_lv[] 패시브(행운의 동전) 슬롯 인덱스 */

/* =========================================================
   전역 변수
   런(run) 단위로 init_run()에서 초기화됨
   ========================================================= */

/* 해금 여부 배열: 0 = 잠김, 1 = 해금. unlocked[i]는 starters[i]에 대응 */
int unlocked[STARTER_COUNT];
/* 현재 런에서 달성한 퍼펙트 승리(3:0) 횟수. 3회 달성 시 스타터 해금 조건 충족 */
int perfect_win_count;

int coin;                /* 보유 코인. 상점에서 아이템 업그레이드 비용으로 차감 */
int item_lv[ITEM_COUNT]; /* 각 슬롯 아이템 레벨: 0=미장착, 1~3=활성 */
int starter_id;          /* 선택된 스타터 인덱스 (starters[] 기준) */
int current_enemy;       /* 현재 대전 상대 인덱스 (enemies[] 기준, 0~6) */
int p_score, e_score;    /* 현재 경기 내 플레이어/상대 라운드 승점 (3선승제) */

int active_used;   /* 독심술 사용 횟수. 경기 시작 시 0으로 초기화 */
int shield_prob;   /* 방패병 Shield 스킬 발동 확률(%). 초기 50%, 발동마다 +10%, 최대 90% */
int draw_streak;   /* 무승부 연속 횟수 (현재 게임 로직에서 미사용, 예비 변수) */
int skill_sealed;  /* 1이면 다음 라운드 상대 스킬 봉인 (허무의 장막 Lv3 효과) */
int skill_debuff;  /* 상대 스킬 발동 확률 감소값(%). 허무의 장막 적용 후 라운드 끝에 초기화 */
int no_shop_first; /* 1이면 첫 상점 이용 불가 (올인 보험 스타터 패널티) */

int last_move;        /* 직전 라운드 플레이어 패. 억제자 Nullify 판정에 사용. 초기값 -1 */
int scissors_streak;  /* 가위 연속 승리 횟수. 예리한 날 보너스 코인 계산에 사용 */
int paper_win_streak; /* 보 연속 승리 횟수. 허무의 장막 Lv3 스킬 봉인 조건 판정에 사용 */

/* =========================================================
   유틸리티
   ========================================================= */

/* 0~99 사이의 난수 반환. 확률 판정(X%)에 사용: rand_percent() < X */
int rand_percent(void) { return rand() % 100; }

/* 0~n-1 사이의 난수 반환 */
int rand_n(int n) { return rand() % n; }

/* 플랫폼별 화면 지우기 */
void clear_screen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/*
 * 엔터 대기 함수.
 * scanf() 사용 후 입력 버퍼에 남은 '\n'을 먼저 소비한 뒤,
 * 사용자가 엔터를 누를 때까지 대기한다.
 */
void press_enter(void) {
    printf(C_WHITE "\n  [ 엔터를 눌러 계속 ]" C_RESET "\n");
    int c;
    while ((c = getchar()) != '\n' && c != EOF); /* 버퍼 잔여 문자 비우기 */
    getchar();                                    /* 실제 엔터 입력 대기 */
}

/* 굵은 구분선 출력 (섹션 경계용) */
void print_line(void) {
    printf(C_BLUE "══════════════════════════════════════════════════════════════" C_RESET "\n");
}

/* 얇은 구분선 출력 (소항목 구분용) */
void print_thin(void) {
    printf(C_BLUE "──────────────────────────────────────────────────────────────" C_RESET "\n");
}

/* 패 인덱스를 이모지+이름 문자열로 변환 (대결 연출 출력용) */
const char* move_emoji(int m) {
    if (m == SCISSORS) return "✌  가위";
    if (m == ROCK)     return "✊  바위";
    return                    "✋  보";
}

/* 패 인덱스를 이름 문자열로 변환 (간략 표시용) */
const char* move_name(int m) {
    if (m == SCISSORS) return "가위";
    if (m == ROCK)     return "바위";
    return "보";
}

/* =========================================================
   상대방 데이터 구조체 및 배열
   ========================================================= */
typedef struct {
    const char* name;       /* 상대 이름 (한국어 + 영어) */
    const char* color;      /* 출력 색상 코드 */
    const char* skill_name; /* 스킬 이름 */
    const char* skill_desc; /* 스킬 설명 (상대방 정보 화면 표시용) */
    int prob[3]; /* 누적 확률 상한 [가위, 바위, 보]: rand_percent() <= prob[i] 로 패 결정 */
    int sc, rc, pc; /* 표시용 선택 확률(%) [가위, 바위, 보] */
} Enemy;

Enemy enemies[ENEMY_COUNT] = {
    { /* 0: 시민 — 스킬 없음, 균등 확률 */
        "시민 (Citizen)", C_WHITE,
        "없음", "특수 스킬 없음.",
        {32, 65, 99}, 33, 33, 34
    },
    { /* 1: 억제자 — 플레이어가 같은 패를 반복하면 60% 확률로 강제 승리 */
        "억제자 (Null)", C_CYAN,
        "Nullify",
        "플레이어가 전 라운드와 같은 패를 낼 경우\n"
        "    60% 확률로 해당 라운드 승리.",
        {69, 84, 99}, 70, 15, 15
    },
    { /* 2: 광전사 — 라운드 패배 시 40% 확률로 결과 무효 및 재경기 */
        "광전사 (Berserker)", C_RED,
        "Rage",
        "광전사가 라운드 패배 시 40% 확률로\n"
        "    해당 라운드 무효화 및 재경기 (재경기 재발동 불가).",
        {32, 65, 99}, 33, 33, 34
    },
    { /* 3: 마법사 — 무승부를 자신의 승리로 전환 */
        "마법사 (Magician)", C_MAGENTA,
        "Mirage",
        "무승부 발생 시 마법사의 승리로 결과 변경.",
        {49, 89, 99}, 50, 40, 10
    },
    { /* 4: 도박사 — 25% 확률로 이번 라운드 승리 시 2점 획득 */
        "도박사 (Gambler)", C_YELLOW,
        "All-in",
        "매 라운드 시작 시 25% 확률로 발동.\n"
        "    발동 시 이번 라운드 승리하면 2점 획득.",
        {32, 65, 99}, 33, 33, 34
    },
    { /* 5: 방패병 — 패배 시 무승부로 전환. 발동 확률 누적 증가 (초기 50%, 최대 90%) */
        "방패병 (Aegis)", C_BBLUE,
        "Shield",
        "방패병이 라운드 패배 시 50% 확률로 무승부 변경.\n"
        "    발동마다 확률 +10% 누적 (최대 90%).",
        {39, 69, 99}, 40, 30, 30
    },
    { /* 6: 챔피언 — 플레이어가 '보' 또는 '바위'를 낼 때 역전 확률 발동 */
        "챔피언 (Champion)", C_BYELLOW,
        "Champion's Fist",
        "플레이어가 '보'를 낼 시 50% 확률로 챔피언 승리.\n"
        "    플레이어가 '바위'를 낼 시 30% 확률로 챔피언 승리.",
        {0, 98, 99}, 1, 98, 1
    },
};

/*
 * 현재 상대의 패를 누적 확률 테이블로 결정한다.
 * prob[] 배열은 [가위 상한, 바위 상한, 보 상한] 형태의 누적값이므로
 * 순서대로 비교하면 된다.
 */
int get_enemy_move(void) {
    int r = rand_percent();
    if (r <= enemies[current_enemy].prob[0]) return SCISSORS;
    if (r <= enemies[current_enemy].prob[1]) return ROCK;
    return PAPER;
}

/* =========================================================
   코인 및 아이템 데이터
   ========================================================= */

/* 상대를 처치했을 때 기본으로 얻는 코인량 (난이도 순 증가) */
int base_coin[ENEMY_COUNT] = {2, 3, 3, 4, 4, 5, 6};

/*
 * 아이템 업그레이드 비용
 *   index 0: Lv0 → Lv1  =  5코인
 *   index 1: Lv1 → Lv2  =  8코인
 *   index 2: Lv2 → Lv3  = 12코인
 * item_upgrade_cost[cur_lv] 형태로 접근하며, cur_lv >= 3이면 최대 레벨로 차단
 */
int item_upgrade_cost[3] = {5, 8, 12};

/* 아이템 슬롯별 이름 (HUD·상점·스테이터스 화면 표시용) */
const char* item_names[ITEM_COUNT] = {
    "✌  예리한 날",
    "✊  강철 주먹",
    "✋  허무의 장막",
    "👁  독심술",
    "🪙  행운의 동전"
};

/*
 * 아이템 레벨별 설명 [슬롯][레벨-1]
 * 상점 구매 화면과 스테이터스 화면에서 현재 레벨 설명을 표시할 때 사용
 */
const char* item_desc[ITEM_COUNT][3] = {
    { /* SLOT_SCISSORS: 예리한 날 */
        "가위 승리 시 +3코인",
        "가위 승리 시 +5코인, 2연속 가위 승리 시 추가 +2코인",
        "가위 승리 시 +5코인, 2연속 +3코인, 3연속 이상 +5코인"
    },
    { /* SLOT_ROCK: 강철 주먹 */
        "바위 패배 시 30% 확률으로 무승부",
        "바위 패배 시 50% 확률으로 무승부",
        "바위 패배 시 50% 무승부, 무승부 성공 시 +2코인"
    },
    { /* SLOT_PAPER: 허무의 장막 */
        "보 승리 시 다음 라운드 상대 스킬 확률 -25%",
        "보 승리 시 스킬 확률 -40%, 보 승리/무승부 시 +2코인",
        "보 승리 시 스킬 확률 -40%, +2코인, 보 2연승 시 스킬 완전 봉인 +3코인"
    },
    { /* SLOT_ACTIVE: 독심술 */
        "경기당 1회: 60% 정확도로 상대 패 힌트",
        "경기당 1회: 85% 정확도로 상대 패 힌트",
        "경기당 2회: 100% 정확도로 상대 패 공개"
    },
    { /* SLOT_PASSIVE: 행운의 동전 */
        "경기 시작 시 +5코인",
        "경기 시작 시 +8코인, 무승부 시 +2코인",
        "경기 시작 시 +10코인, 무승부 시 +2코인, 경기 승리 시 +5코인"
    }
};

/* =========================================================
   스타터 데이터
   slot >= 0 이면 해당 슬롯을 Lv1로 즉시 활성화.
   slot == -1 이면 아이템 슬롯 대신 특수 효과를 코드에서 직접 처리.
   ========================================================= */
typedef struct {
    const char* name; /* 스타터 이름 */
    const char* desc; /* 스타터 효과 요약 */
    int slot;         /* 활성화할 아이템 슬롯 인덱스, 특수 스타터는 -1 */
} Starter;

Starter starters[STARTER_COUNT] = {
    /* 0~2: 기본 제공 */
    {"✌  예리한 날 Lv1",
     "가위 승리 시 +3코인", SLOT_SCISSORS},
    {"✊  강철 주먹 Lv1",
     "바위 패배 시 30% 확률으로 무승부", SLOT_ROCK},
    {"🪙 행운의 동전 Lv1",
     "경기 시작 시 +5코인", SLOT_PASSIVE},
    /* 3: 시민 퍼펙트 처치 해금 */
    {"🍀 초심자의 운",
     "첫 경기 상대 스킬 확률 -30% (1경기 한정)", -1},
    /* 4: 억제자 처치 해금 */
    {"✋  허무의 장막 Lv1",
     "보 승리 시 다음 라운드 상대 스킬 확률 -25%", SLOT_PAPER},
    /* 5: 마법사 처치 해금 */
    {"👁  독심술 Lv1",
     "경기당 1회, 60% 정확도로 상대 패 힌트", SLOT_ACTIVE},
    /* 6: 도박사 처치 해금. 시작 코인 +8, 첫 상점 이용 불가 */
    {"💰 올인 보험",
     "시작 시 +8코인, 첫 상점 이용 불가", -1},
    /* 7: 챔피언 클리어 해금. 강철 주먹 Lv1 활성화 + 50% 확률 특수 처리 */
    {"👑 챔피언의 잔재",
     "바위 패배 시 50% 무승부 + 경기 시작 시 +3코인", -1},
    /* 8: 한 런에서 퍼펙트 3회 해금. 경기 시작 스코어 1:0 선취 */
    {"⚡ 완벽주의자의 각오",
     "경기 시작 시 스코어 1:0 선취", -1},
};

/* =========================================================
   HUD (경기 중 상단 상태 표시줄)
   ========================================================= */
void show_hud(void) {
    print_thin();
    printf(C_BWHITE "  🪙 코인: " C_BYELLOW "%d" C_RESET, coin);
    printf(C_BWHITE "   |   상대: " C_RESET "%s%s" C_RESET,
        enemies[current_enemy].color, enemies[current_enemy].name);
    printf(C_BWHITE "   |   스코어: " C_BGREEN "%d" C_RESET
           C_WHITE " : " C_RESET C_BRED "%d" C_RESET "\n", p_score, e_score);

    /* 아이템 슬롯 아이콘: 미장착은 [-], 장착 중이면 레벨 표시 */
    printf(C_WHITE "  아이템: " C_RESET);
    const char* slot_icon[ITEM_COUNT] = {
        "✌", "✊", "✋", "👁", "🪙"
    };
    for (int i = 0; i < ITEM_COUNT; i++) {
        if (item_lv[i] == 0)
            printf(C_WHITE " %s[-]" C_RESET, slot_icon[i]);
        else
            printf(C_BGREEN " %s[%d]" C_RESET, slot_icon[i], item_lv[i]);
    }
    printf("\n");
    print_thin();
}

/* =========================================================
   내 스테이터스 상세 화면
   ========================================================= */
void show_my_status(void) {
    clear_screen();
    print_line();
    printf(C_BWHITE "  📊 내 스테이터스" C_RESET "\n");
    print_thin();

    /* 스타터 아이템 */
    printf(C_BYELLOW "  [ 스타터 아이템 ]" C_RESET "\n");
    printf("  %s\n", starters[starter_id].name);
    printf("  " C_CYAN "%s" C_RESET "\n\n", starters[starter_id].desc);

    /* 아이템 슬롯별 레벨과 효과 설명 */
    printf(C_BYELLOW "  [ 아이템 슬롯 ]" C_RESET "\n");
    for (int i = 0; i < ITEM_COUNT; i++) {
        int lv = item_lv[i];
        if (lv == 0)
            printf("  " C_WHITE "%s  -- 미장착" C_RESET "\n", item_names[i]);
        else {
            printf("  " C_BGREEN "%s  Lv%d" C_RESET "\n", item_names[i], lv);
            printf("    " C_CYAN "%s" C_RESET "\n", item_desc[i][lv - 1]);
        }
    }

    /* 현재 경기 내 상태 값 */
    print_thin();
    printf(C_BYELLOW "  [ 현재 경기 상태 ]" C_RESET "\n");
    printf("  스코어:       " C_BGREEN "%d" C_RESET " : " C_BRED "%d" C_RESET "\n",
           p_score, e_score);
    printf("  보유 코인:   " C_BYELLOW "%d" C_RESET "\n", coin);
    if (current_enemy == 5) /* 방패병 상대 중일 때만 Shield 확률 표시 */
        printf("  Shield 확률: " C_CYAN "%d%%" C_RESET "\n", shield_prob);
    if (skill_sealed)
        printf("  " C_BMAGENTA "다음 라운드 상대 스킬 봉인 중!" C_RESET "\n");
    if (skill_debuff > 0)
        printf("  상대 스킬 확률 감소: " C_BGREEN "-%d%%" C_RESET "\n", skill_debuff);

    print_line();
    press_enter();
}

/* =========================================================
   상대방 정보 화면
   ========================================================= */
void show_enemy_info(void) {
    clear_screen();
    print_line();
    printf(C_BWHITE "  📋 상대방 정보" C_RESET "\n");
    print_thin();

    int ei = current_enemy;
    printf("  현재 상대: %s%s" C_RESET "\n\n",
           enemies[ei].color, enemies[ei].name);

    /* 상대의 패 선택 확률 (표시용 sc/rc/pc 값) */
    printf(C_BYELLOW "  [ 패 선택 확률 ]" C_RESET "\n");
    printf("  ✌  가위:  " C_CYAN "%d%%" C_RESET "\n", enemies[ei].sc);
    printf("  ✊  바위:  " C_CYAN "%d%%" C_RESET "\n", enemies[ei].rc);
    printf("  ✋  보:    " C_CYAN "%d%%" C_RESET "\n\n", enemies[ei].pc);

    /* 고유 스킬 정보 */
    printf(C_BYELLOW "  [ 고유 스킬 ]" C_RESET "\n");
    if (strcmp(enemies[ei].skill_name, "없음") == 0) {
        printf("  " C_WHITE "없음" C_RESET "\n");
    } else {
        printf("  " C_BMAGENTA "%s" C_RESET "\n", enemies[ei].skill_name);
        printf("  " C_WHITE "%s" C_RESET "\n", enemies[ei].skill_desc);
    }

    /* 토너먼트 전체 진행 상황 (현재 위치 강조) */
    print_thin();
    printf(C_BYELLOW "  [ 토너먼트 진행 상황 ]" C_RESET "\n");
    for (int i = 0; i < ENEMY_COUNT; i++) {
        if      (i < ei)  printf("  " C_WHITE "  %d. %s" C_RESET "\n", i+1, enemies[i].name);
        else if (i == ei) printf("  " C_BGREEN "▶ %d. %s  ← 현재" C_RESET "\n", i+1, enemies[i].name);
        else              printf("  " C_WHITE "  %d. %s" C_RESET "\n", i+1, enemies[i].name);
    }
    print_line();
    press_enter();
}

/* =========================================================
   스타터 해금 조건 확인 화면
   ========================================================= */
void show_unlock_info(void) {
    clear_screen();
    print_line();
    printf(C_BWHITE "  🔓 해금 조건 & 스타터 아이템" C_RESET "\n");
    print_line();

    /* 각 스타터의 해금 조건 텍스트와 대응 스타터 인덱스 */
    typedef struct { const char* cond; int uid; } UInfo;
    UInfo info[] = {
        {"기본 제공",                  0},
        {"기본 제공",                  1},
        {"기본 제공",                  2},
        {"시민을 퍼펙트(3:0)로 처치", 3},
        {"억제자 처치",                4},
        {"마법사 처치",                5},
        {"도박사 처치",                6},
        {"챔피언 처치 (클리어)",       7},
        {"한 런에서 퍼펙트 승리 3회", 8},
    };

    for (int i = 0; i < STARTER_COUNT; i++) {
        int uid = info[i].uid;
        if (unlocked[uid])
            printf("  " C_BGREEN "✔ %-32s" C_RESET " %s\n",
                   starters[uid].name, info[i].cond);
        else
            printf("  " C_WHITE "✖ %-32s" C_RESET " %s\n",
                   starters[uid].name, info[i].cond);
    }
    print_thin();
    printf(C_YELLOW "  현재 런 퍼펙트 승리: %d회\n" C_RESET, perfect_win_count);
    print_line();
    press_enter();
}

/* =========================================================
   경기 중 메뉴 (라운드 시작 전 매번 표시)
   1: 라운드 진행  2: 내 스테이터스  3: 상대 정보
   ========================================================= */
void in_match_menu(void) {
    while (1) {
        clear_screen();
        show_hud();
        printf("\n");
        printf("  " C_BYELLOW "[1]" C_RESET " 계속 진행\n");
        printf("  " C_BYELLOW "[2]" C_RESET " 내 스테이터스 확인\n");
        printf("  " C_BYELLOW "[3]" C_RESET " 상대방 스킬 & 확률 파악\n");
        print_thin();
        printf("  선택 " C_BYELLOW "(1~3)" C_RESET ": ");

        int c = 0;
        scanf("%d", &c);
        if      (c == 1) break;
        else if (c == 2) show_my_status();
        else if (c == 3) show_enemy_info();
    }
    clear_screen();
}

/* =========================================================
   경기 시작 전 메뉴 (각 경기 돌입 직전 1회 표시)
   1: 경기 시작  2: 내 스테이터스  3: 상대 정보  4: 해금 조건
   ========================================================= */
void pre_match_menu(void) {
    while (1) {
        clear_screen();
        print_line();
        printf(C_BWHITE "  ⚔  다음 상대: " C_RESET "%s%s" C_RESET "\n",
            enemies[current_enemy].color, enemies[current_enemy].name);
        print_thin();
        printf("  " C_BYELLOW "[1]" C_RESET " 경기 시작\n");
        printf("  " C_BYELLOW "[2]" C_RESET " 내 스테이터스 확인\n");
        printf("  " C_BYELLOW "[3]" C_RESET " 상대방 스킬 & 확률 파악\n");
        printf("  " C_BYELLOW "[4]" C_RESET " 해금 조건 확인\n");
        print_line();
        printf("  선택 " C_BYELLOW "(1~4)" C_RESET ": ");

        int c = 0;
        scanf("%d", &c);
        if      (c == 1) break;
        else if (c == 2) show_my_status();
        else if (c == 3) show_enemy_info();
        else if (c == 4) show_unlock_info();
    }
}

/* =========================================================
   상점 로직
   경기 사이마다 호출. 최대 레벨 미달 아이템 중 3개를 무작위로 제시.
   코인이 충분하면 업그레이드 가능. 여러 번 구매 가능.
   ========================================================= */
void run_shop_logic(void) {
    /* 업그레이드 가능한 아이템(레벨 < 3)만 후보로 수집 */
    int options[ITEM_COUNT], opt_count = 0;
    for (int i = 0; i < ITEM_COUNT; i++)
        if (item_lv[i] < 3) options[opt_count++] = i;

    if (opt_count == 0) {
        clear_screen();
        print_line();
        printf("  " C_BGREEN "모든 아이템이 최대 레벨입니다!" C_RESET "\n");
        print_line();
        press_enter();
        return;
    }

    /* Fisher-Yates 셔플로 후보를 무작위 정렬 (상점 진입 시 1회만 수행) */
    int shuffled[ITEM_COUNT];
    for (int i = 0; i < opt_count; i++) shuffled[i] = options[i];
    for (int i = opt_count - 1; i > 0; i--) {
        int j = rand_n(i + 1);
        int tmp = shuffled[i]; shuffled[i] = shuffled[j]; shuffled[j] = tmp;
    }
    int max_show = opt_count < 3 ? opt_count : 3; /* 최대 3개 표시 */

    while (1) {
        clear_screen();
        print_line();
        printf(C_BWHITE "  🛒 상점" C_RESET
               "   " C_BYELLOW "🪙 보유 코인: %d" C_RESET "\n", coin);
        print_thin();

        /* 표시 중인 슬롯이 모두 최대 레벨이면 상점 종료 */
        int all_max = 1;
        for (int i = 0; i < max_show; i++)
            if (item_lv[shuffled[i]] < 3) { all_max = 0; break; }

        if (all_max) {
            printf("  " C_BGREEN "이번 상점 선택지가 모두 최대 레벨입니다!" C_RESET "\n");
            print_line();
            press_enter();
            return;
        }

        /* 각 슬롯의 현재 레벨, 업그레이드 비용, 다음 레벨 효과 설명 표시 */
        for (int i = 0; i < max_show; i++) {
            int slot = shuffled[i];
            int cur_lv = item_lv[slot];
            if (cur_lv >= 3) {
                printf("  " C_WHITE "[%d]" C_RESET " %s  " C_BGREEN "최대 레벨" C_RESET "\n",
                       i+1, item_names[slot]);
                continue;
            }
            int cost = item_upgrade_cost[cur_lv];
            int ok = (coin >= cost);
            printf("  " C_BYELLOW "[%d]" C_RESET " %s  " C_WHITE "Lv%d -> Lv%d" C_RESET,
                   i+1, item_names[slot], cur_lv, cur_lv+1);
            if (ok) printf("  " C_BGREEN "(%d코인)" C_RESET "\n", cost);
            else    printf("  " C_BRED   "(%d코인 -- 코인 부족)" C_RESET "\n", cost);
            printf("      " C_CYAN "%s" C_RESET "\n", item_desc[slot][cur_lv]);
        }
        printf("  " C_WHITE "[%d]" C_RESET " 상점 나가기\n", max_show + 1);
        print_thin();
        printf("  선택 " C_BYELLOW "(1~%d)" C_RESET ": ", max_show + 1);

        int choice = 0;
        scanf("%d", &choice);

        if (choice == max_show + 1) {
            printf("  상점을 나갑니다.\n");
            press_enter();
            return;
        }
        if (choice < 1 || choice > max_show) continue; /* 범위 외 입력 무시 */

        int slot = shuffled[choice - 1];
        if (item_lv[slot] >= 3) {
            printf("  " C_BRED "이미 최대 레벨입니다!" C_RESET "\n");
            press_enter();
            continue;
        }
        int cost = item_upgrade_cost[item_lv[slot]];
        if (coin < cost) {
            printf("  " C_BRED "코인이 부족합니다!" C_RESET "\n");
            press_enter();
        } else {
            /* 코인 차감 후 레벨 업 */
            coin -= cost;
            item_lv[slot]++;
            printf("  " C_BGREEN "✔ %s Lv%d 업그레이드 완료!" C_RESET
                   "  " C_BYELLOW "잔여: %d코인" C_RESET "\n",
                item_names[slot], item_lv[slot], coin);
            press_enter();
        }
    }
}

/* 상점 진입 래퍼. 올인 보험 패널티(첫 상점 차단) 처리 후 run_shop_logic 호출 */
void run_shop(void) {
    if (no_shop_first && current_enemy == 0) {
        clear_screen();
        print_line();
        printf("  " C_BRED "[올인 보험]" C_RESET " 첫 상점 이용 불가!\n");
        print_line();
        no_shop_first = 0; /* 패널티는 1회만 적용 */
        press_enter();
        return;
    }
    run_shop_logic();
}

/* =========================================================
   스타터 선택
   해금된 스타터만 표시. 선택 후 슬롯 활성화 및 특수 효과 처리.
   ========================================================= */
void select_starter(void) {
    clear_screen();
    print_line();
    printf(C_BWHITE "  ⚔  스타터 아이템 선택" C_RESET "\n");
    print_thin();

    /* 해금된 스타터만 available 배열에 수집 */
    int available[STARTER_COUNT], count = 0;
    for (int i = 0; i < STARTER_COUNT; i++)
        if (unlocked[i]) available[count++] = i;

    for (int i = 0; i < count; i++) {
        int id = available[i];
        printf("  " C_BYELLOW "[%d]" C_RESET " %-34s" C_CYAN "%s" C_RESET "\n",
            i+1, starters[id].name, starters[id].desc);
    }
    print_thin();

    int choice = 0;
    while (choice < 1 || choice > count) {
        printf("  선택 " C_BYELLOW "(1~%d)" C_RESET ": ", count);
        scanf("%d", &choice);
    }
    starter_id = available[choice - 1];

    /* slot >= 0이면 해당 아이템 슬롯을 Lv1로 즉시 활성화 */
    if (starters[starter_id].slot >= 0)
        item_lv[starters[starter_id].slot] = 1;

    /* 특수 스타터 효과 처리 (slot == -1인 경우) */
    if (starter_id == 6) { coin += 8; no_shop_first = 1; } /* 올인 보험: 코인 +8, 첫 상점 차단 */
    if (starter_id == 7)  item_lv[SLOT_ROCK] = 1;           /* 챔피언의 잔재: 강철 주먹 Lv1 활성화 */

    clear_screen();
    print_line();
    printf("  " C_BGREEN "✔ '%s' 선택 완료!" C_RESET "\n",
           starters[starter_id].name);
    print_line();
    press_enter();
}

/* =========================================================
   스타터 해금 판정
   cleared: 1이면 챔피언(마지막 상대) 처치 완료를 의미
   current_enemy 값을 기준으로 누적 진행도에 따라 해금 체크
   ========================================================= */
void check_unlock(int cleared) {
    int any = 0; /* 새로 해금된 항목이 있으면 1로 설정 → press_enter 호출 */

    /* 억제자 처치 이후(current_enemy >= 1) → 허무의 장막 해금 */
    if (current_enemy >= 1 && !unlocked[4]) {
        unlocked[4] = 1;
        printf("  " C_BGREEN "[해금]" C_RESET " 허무의 장막 Lv1 스타터 해금!\n"); any = 1;
    }
    /* 마법사 처치 이후(current_enemy >= 3) → 독심술 해금 */
    if (current_enemy >= 3 && !unlocked[5]) {
        unlocked[5] = 1;
        printf("  " C_BGREEN "[해금]" C_RESET " 독심술 Lv1 스타터 해금!\n"); any = 1;
    }
    /* 도박사 처치 이후(current_enemy >= 4) → 올인 보험 해금 */
    if (current_enemy >= 4 && !unlocked[6]) {
        unlocked[6] = 1;
        printf("  " C_BGREEN "[해금]" C_RESET " 올인 보험 스타터 해금!\n"); any = 1;
    }
    /* 챔피언 처치(클리어) → 챔피언의 잔재 해금 */
    if (cleared && !unlocked[7]) {
        unlocked[7] = 1;
        printf("  " C_BGREEN "[해금]" C_RESET " 챔피언의 잔재 스타터 해금!\n"); any = 1;
    }
    /* 런 내 퍼펙트 승리 3회 달성 → 완벽주의자의 각오 해금 */
    if (perfect_win_count >= 3 && !unlocked[8]) {
        unlocked[8] = 1;
        printf("  " C_BGREEN "[해금]" C_RESET " 완벽주의자의 각오 스타터 해금!\n"); any = 1;
    }
    if (any) press_enter();
}

/* =========================================================
   독심술 (액티브 아이템) 사용
   real_move: 상대가 실제로 낼 패 (결정된 값).
   정확도에 따라 올바른 힌트 또는 틀린 힌트를 출력.
   ========================================================= */
void use_active(int real_move) {
    int lv = item_lv[SLOT_ACTIVE];
    if (lv == 0) return; /* 아이템 미장착 */

    int max_uses = (lv == 3) ? 2 : 1; /* Lv3만 경기당 2회 사용 가능 */
    if (active_used >= max_uses) return; /* 사용 횟수 초과 */

    printf("  " C_BCYAN "[👁  독심술]" C_RESET
           " 사용? " C_WHITE "(남은 횟수: %d)" C_RESET " (y/n): ",
           max_uses - active_used);
    char c; scanf(" %c", &c);
    if (c != 'y' && c != 'Y') return;

    active_used++;

    /* 레벨별 정확도: Lv1=60%, Lv2=85%, Lv3=100% */
    int accuracy = (lv == 1) ? 60 : (lv == 2) ? 85 : 100;
    int hint_move;
    if (rand_percent() < accuracy) {
        hint_move = real_move; /* 정확한 힌트 */
    } else {
        /* 오답 2가지 중 무작위로 1개 선택 */
        int wrong[2], wc = 0;
        for (int i = 0; i < 3; i++) if (i != real_move) wrong[wc++] = i;
        hint_move = wrong[rand_n(2)];
    }
    printf("  " C_BCYAN "👁  독심술:" C_RESET
           " 상대방은 아마 " C_BYELLOW "『 %s 』" C_RESET
           " 을(를) 낼 것 같다...\n\n",
           move_emoji(hint_move));
}

/* =========================================================
   기본 승패 판정
   반환값: 1 = 플레이어 승, -1 = 플레이어 패, 0 = 무승부
   ========================================================= */
int judge_basic(int p, int e) {
    if (p == e) return 0;
    if ((p == SCISSORS && e == PAPER)    ||
        (p == ROCK     && e == SCISSORS) ||
        (p == PAPER    && e == ROCK)) return 1;
    return -1;
}

/* =========================================================
   상대방 스킬 적용
   judge_basic() 결과를 받아 스킬 조건 충족 시 결과를 보정해 반환.
   반환값 99: 재경기 요청 (광전사 Rage 발동 시)
   ========================================================= */
int apply_enemy_skill(int result, int p_move,
                      int e_move __attribute__((unused)),
                      int rnd    __attribute__((unused))) {
    /* 허무의 장막 Lv3 효과: 이번 라운드 스킬 봉인 */
    if (skill_sealed) {
        printf("  " C_BMAGENTA "[스킬 봉인]" C_RESET
               " 상대방의 스킬이 봉인되어 발동되지 않는다!\n");
        skill_sealed = 0; /* 봉인은 1라운드만 지속 */
        return result;
    }

    int ei = current_enemy;

    /* 억제자(1): Nullify
     * 플레이어가 직전과 같은 패를 냈을 때 (60% - skill_debuff)% 확률로 억제자 승리 */
    if (ei == 1 && last_move == p_move && last_move != -1) {
        int prob = 60 - skill_debuff;
        if (prob < 0) prob = 0;
        printf("  " C_BCYAN "[Nullify]" C_RESET " 같은 패 감지! ");
        if (rand_percent() < prob) {
            printf(C_BRED "발동! → 억제자 승리\n" C_RESET);
            return -1;
        }
        printf(C_WHITE "실패.\n" C_RESET);
    }

    /* 광전사(2): Rage
     * 플레이어가 이겼을 때(result == -1은 상대 패배) 40% 확률로 재경기 요청.
     * 재경기 중에는 is_rematch == 1이므로 이 블록 자체에 진입하지 않음 */
    if (ei == 2 && result == -1) {
        int prob = 40 - skill_debuff;
        if (prob < 0) prob = 0;
        printf("  " C_BRED "[Rage]" C_RESET " 패배 감지! ");
        if (rand_percent() < prob) {
            printf(C_BRED "발동!\n" C_RESET);
            printf("\n");
            printf("  " C_BGRED C_BWHITE
                   "  🔥 광전사의 분노가 폭발한다! 재경기!  "
                   C_RESET "\n");
            press_enter();
            return 99; /* run_round()에서 재귀 재경기 호출 */
        }
        printf(C_WHITE "실패.\n" C_RESET);
    }

    /* 마법사(3): Mirage
     * 무승부 시 (100% - skill_debuff)% 확률로 마법사 승리로 전환 */
    if (ei == 3 && result == 0) {
        int prob = 100 - skill_debuff;
        printf("  " C_BMAGENTA "[Mirage]" C_RESET " 무승부 감지! ");
        if (rand_percent() < prob) {
            printf(C_BRED "발동! → 마법사 승리\n" C_RESET);
            return -1;
        }
        printf(C_WHITE "실패.\n" C_RESET);
    }

    /* 방패병(5): Shield
     * 플레이어가 이겼을 때(result == -1은 방패병 패배) shield_prob% 확률로 무승부 전환.
     * 발동 여부와 무관하게 매 발동 시도마다 확률 +10% 누적 (최대 90%) */
    if (ei == 5 && result == -1) {
        printf("  " C_BBLUE "[Shield]" C_RESET
               " 패배 감지! (현재 확률 %d%%) ", shield_prob);
        if (rand_percent() < shield_prob) {
            printf(C_BRED "발동! → 무승부로 변경\n" C_RESET);
            shield_prob += 10;
            if (shield_prob > 90) shield_prob = 90;
            return 0;
        }
        printf(C_WHITE "실패.\n" C_RESET);
        shield_prob += 10;
        if (shield_prob > 90) shield_prob = 90;
    }

    /* 챔피언(6): Champion's Fist
     * 플레이어가 '보'로 이기면 50% 역전, 바위로 무승부면 30% 챔피언 승리 */
    if (ei == 6) {
        if (p_move == PAPER && result == 1) {
            printf("  " C_BYELLOW "[Champion's Fist]" C_RESET " 보 감지! ");
            if (rand_percent() < 50) {
                printf(C_BRED "발동! → 챔피언 역전!\n" C_RESET);
                return -1;
            }
            printf(C_WHITE "실패.\n" C_RESET);
        } else if (p_move == ROCK && result == 0) {
            printf("  " C_BYELLOW "[Champion's Fist]" C_RESET " 바위 무승부 감지! ");
            if (rand_percent() < 30) {
                printf(C_BRED "발동! → 챔피언 승리!\n" C_RESET);
                return -1;
            }
            printf(C_WHITE "실패.\n" C_RESET);
        }
    }

    return result;
}

/* =========================================================
   플레이어 아이템 효과 적용
   result: 스킬 보정 후 최종 라운드 결과 (1/0/-1)
   p_move: 플레이어가 낸 패
   rp: 결과값 포인터 (강철 주먹이 무승부로 바꿀 때 외부에 반영)
   ========================================================= */
void apply_item_effect(int result, int p_move, int *rp) {

    /* [강철 주먹] 바위 패배 시 일정 확률로 무승부 전환
     * 챔피언의 잔재(starter_id==7) 착용 시 Lv2 이상과 동일한 50% 적용 */
    if (p_move == ROCK && result == -1 && item_lv[SLOT_ROCK] >= 1) {
        int prob = (starter_id == 7) ? 50 :
                   (item_lv[SLOT_ROCK] == 1) ? 30 : 50;
        printf("  " C_BBLUE "[✊  강철 주먹]" C_RESET " 바위 패배! ");
        if (rand_percent() < prob) {
            printf(C_BGREEN "발동! (%d%%) → 무승부\n" C_RESET, prob);
            *rp = 0; result = 0; /* 외부 result와 내부 result 모두 갱신 */
            if (item_lv[SLOT_ROCK] == 3) { /* Lv3 보너스: 무승부 성공 +2코인 */
                coin += 2;
                printf("  " C_BYELLOW "[✊  강철 주먹 Lv3]" C_RESET
                       " 무승부 성공 +2코인 (전체 %d코인)\n", coin);
            }
        } else {
            printf(C_WHITE "실패. (%d%%)\n" C_RESET, prob);
        }
    }

    /* [행운의 동전] Lv2 이상: 무승부 시 +2코인 */
    if (result == 0 && item_lv[SLOT_PASSIVE] >= 2) {
        coin += 2;
        printf("  " C_BYELLOW "[🪙 행운의 동전]" C_RESET
               " 무승부 +2코인 (전체 %d코인)\n", coin);
    }

    /* [예리한 날] 가위 승리 시 레벨·연속 횟수에 따라 보너스 코인 지급 */
    if (p_move == SCISSORS && result == 1 && item_lv[SLOT_SCISSORS] >= 1) {
        int lv = item_lv[SLOT_SCISSORS];
        scissors_streak++;
        int bonus;
        if (lv == 1) {
            bonus = 3;
        } else if (lv == 2) {
            bonus = (scissors_streak >= 2) ? 5 + 2 : 5;
        } else { /* Lv3 */
            if      (scissors_streak >= 3) bonus = 5 + 5;
            else if (scissors_streak >= 2) bonus = 5 + 3;
            else                           bonus = 5;
        }
        coin += bonus;
        printf("  " C_BYELLOW "[✌  예리한 날]" C_RESET
               " 가위 승리 +%d코인 (전체 %d코인)",
               bonus, coin);
        if (scissors_streak >= 2)
            printf(C_BCYAN " (%d연속)" C_RESET, scissors_streak);
        printf("\n");
    } else if (p_move != SCISSORS) {
        scissors_streak = 0; /* 가위 외 패를 냈을 때 연속 횟수 리셋 */
    }

    /* [허무의 장막] 보 승리/무승부 시 효과 처리 */
    if (p_move == PAPER && result == 1 && item_lv[SLOT_PAPER] >= 1) {
        int lv = item_lv[SLOT_PAPER];
        int debuff = (lv == 1) ? 25 : 40; /* 다음 라운드 상대 스킬 확률 감소값 */
        skill_debuff += debuff;
        int cb = (lv >= 2) ? 2 : 0; /* Lv2 이상: 보 승리 +2코인 */
        if (cb) coin += cb;
        printf("  " C_BMAGENTA "[✋  허무의 장막]" C_RESET
               " 보 승리 → 스킬 확률 -%d%%", debuff);
        if (cb) printf(", +%d코인 (전체 %d코인)", cb, coin);
        printf("\n");

        paper_win_streak++;
        /* Lv3: 보 2연승 달성 시 다음 라운드 스킬 완전 봉인 + +3코인 */
        if (lv == 3 && paper_win_streak >= 2) {
            skill_sealed = 1;
            coin += 3;
            printf("  " C_BMAGENTA "[✋  허무의 장막 Lv3]" C_RESET
                   " 보 2연승! → 스킬 봉인 + "
                   C_BYELLOW "+3코인" C_RESET " (전체 %d코인)\n", coin);
        }
    } else if (p_move == PAPER && result == 0 && item_lv[SLOT_PAPER] >= 2) {
        /* Lv2 이상: 보 무승부 +2코인, 연승은 리셋 */
        coin += 2;
        printf("  " C_BMAGENTA "[✋  허무의 장막]" C_RESET
               " 보 무승부 +2코인 (전체 %d코인)\n", coin);
        paper_win_streak = 0;
    } else {
        if (p_move != PAPER) paper_win_streak = 0; /* 보 외 패 시 연승 리셋 */
    }
}

/* =========================================================
   플레이어 패 입력 UI
   반환값: SCISSORS(0) / ROCK(1) / PAPER(2)
   ========================================================= */
int get_player_move(void) {
    printf("\n");
    printf(C_BWHITE "  +---------------+---------------+---------------+\n" C_RESET);
    printf(C_BWHITE "  |" C_RESET
           C_BYELLOW " [1] ✌  가위  " C_RESET
           C_BWHITE "|" C_RESET
           C_BBLUE  " [2] ✊  바위  " C_RESET
           C_BWHITE "|" C_RESET
           C_BGREEN " [3] ✋  보    " C_RESET
           C_BWHITE "|\n" C_RESET);
    printf(C_BWHITE "  +---------------+---------------+---------------+\n" C_RESET);

    int choice = 0;
    while (choice < 1 || choice > 3) {
        printf("  선택 " C_BYELLOW "(1~3)" C_RESET ": ");
        scanf("%d", &choice);
    }
    return choice - 1; /* 1~3 입력을 0~2 인덱스로 변환 */
}

/* =========================================================
   단일 라운드 진행
   is_rematch: 1이면 광전사 재경기(메뉴 생략, 재경기 표시)
   반환값: 1 = 플레이어 승, -1 = 패, 0 = 무승부, 99 = 재경기(내부용)
   ========================================================= */
int run_round(int is_rematch) {
    if (!is_rematch) {
        in_match_menu(); /* 일반 라운드: 정보 메뉴 표시 */
    } else {
        clear_screen();
        show_hud();
        printf("\n  " C_BRED "--- 재경기 ---" C_RESET "\n\n");
    }

    /* [도박사] All-in: 라운드 시작 시 25% 확률 발동. 승리 시 라운드 점수 +2 */
    int allin_active = 0;
    if (current_enemy == 4 && !is_rematch) {
        int prob = 25 - (skill_debuff > 25 ? 25 : skill_debuff);
        if (rand_percent() < prob) {
            allin_active = 1;
            printf("  " C_BYELLOW "💰 [All-in]" C_RESET
                   C_BRED " 이번 판은 올인! 승리 시 2점!\n\n" C_RESET);
        }
    }

    /* [초심자의 운] 첫 경기(current_enemy == 0)에만 스킬 확률 -30% 보정 */
    if (starter_id == 3 && current_enemy == 0)
        if (skill_debuff < 30) skill_debuff = 30;

    /* 상대 패 미리 결정 (독심술 힌트에 사용) */
    int e_move = get_enemy_move();

    /* 독심술 사용 기회 제공 (e_move를 인수로 전달하여 힌트 계산) */
    if (item_lv[SLOT_ACTIVE] >= 1) use_active(e_move);

    /* 플레이어 패 입력 */
    int p_move = get_player_move();

    /* 대결 연출: 양측 선택 패 출력 */
    printf("\n");
    print_thin();
    printf("  " C_BWHITE "나:       " C_RESET C_BYELLOW " %s" C_RESET "\n",
           move_emoji(p_move));
    printf("  " C_BWHITE "%-9s" C_RESET "%s %s" C_RESET "\n",
           enemies[current_enemy].name,
           enemies[current_enemy].color, move_emoji(e_move));
    print_thin();
    printf("\n");

    /* 판정 → 스킬 적용 → 아이템 효과 순으로 결과 확정 */
    int result = judge_basic(p_move, e_move);
    result = apply_enemy_skill(result, p_move, e_move, 0);

    if (result == 99) return run_round(1); /* 광전사 재경기: 재귀 호출 */

    apply_item_effect(result, p_move, &result);

    /* 결과 배너 출력 */
    printf("\n");
    if (result == 1)
        printf("  " C_BGGREEN C_BWHITE "  ★  승리!  ★  " C_RESET "\n");
    else if (result == -1)
        printf("  " C_BGRED   C_BWHITE "  ✖  패배...    " C_RESET "\n");
    else
        printf("  " C_BGYELLOW C_WHITE "  ─  무승부     " C_RESET "\n");

    /* 승리 시 코인 지급. All-in 발동 중이면 기본 코인을 2배 지급 */
    if (result == 1) {
        int earned = base_coin[current_enemy];
        if (allin_active) earned += base_coin[current_enemy];
        coin += earned;
        printf("  " C_BYELLOW "🪙 코인 +%d (전체 %d코인)" C_RESET "\n",
               earned, coin);
    }

    /* 라운드 종료: 디버프 초기화, 마지막 패 기록 */
    skill_debuff = 0;
    last_move = p_move;

    press_enter();
    return result;
}

/* =========================================================
   경기 진행 (5판 3승제)
   경기 시작 시 상태 초기화 → 스타터/패시브 효과 적용 → 라운드 반복
   반환값: 1 = 플레이어 경기 승, -1 = 패
   ========================================================= */
int run_match(void) {
    /* 경기 단위 상태 초기화 */
    p_score = 0; e_score = 0;
    active_used = 0;
    shield_prob = 50; /* 방패병 Shield 초기 확률 50% */
    draw_streak = 0;
    skill_sealed = 0;
    skill_debuff = 0;
    last_move = -1;   /* -1: 직전 라운드 없음 (억제자 Nullify 판정 제외용) */
    scissors_streak = 0;
    paper_win_streak = 0;

    /* 스타터 특수 효과: 경기 시작 시 발동 */
    if (starter_id == 8) { /* 완벽주의자의 각오: 1:0 선취 */
        p_score = 1;
        printf("  " C_BCYAN "[완벽주의자의 각오]" C_RESET
               " 경기 시작 스코어 1:0!\n");
    }
    if (starter_id == 7) { /* 챔피언의 잔재: 경기 시작 +3코인 */
        coin += 3;
        printf("  " C_BYELLOW "[챔피언의 잔재]" C_RESET
               " 경기 시작 +3코인 (전체 %d코인)\n", coin);
    }

    /* 행운의 동전(패시브): 레벨별 경기 시작 코인 지급 */
    if (item_lv[SLOT_PASSIVE] >= 1) {
        int bonus = (item_lv[SLOT_PASSIVE] == 1) ? 5 :
                    (item_lv[SLOT_PASSIVE] == 2) ? 8 : 10;
        coin += bonus;
        printf("  " C_BYELLOW "[🪙 행운의 동전]" C_RESET
               " 경기 시작 +%d코인 (전체 %d코인)\n", bonus, coin);
    }

    pre_match_menu(); /* 경기 직전 메뉴 */

    /* 3선승 달성 전까지 라운드 반복 */
    while (p_score < 3 && e_score < 3) {
        int result = run_round(0);
        if      (result ==  1) p_score++;
        else if (result == -1) e_score++;
        /* 무승부(0)는 스코어 미반영 */
    }

    clear_screen();
    print_line();
    if (p_score == 3) {
        printf("  " C_BGREEN "★ %s 처치! 최종 스코어 %d:%d ★\n" C_RESET,
            enemies[current_enemy].name, p_score, e_score);

        /* 퍼펙트 승리(3:0) 보너스 */
        if (e_score == 0) {
            coin += 5;
            perfect_win_count++;
            printf("  " C_BYELLOW "🎯 퍼펙트 승리 +5코인! (전체 %d코인)\n" C_RESET, coin);
        }
        /* 행운의 동전 Lv3: 경기 승리 +5코인 */
        if (item_lv[SLOT_PASSIVE] == 3) {
            coin += 5;
            printf("  " C_BYELLOW "[🪙 행운의 동전 Lv3]" C_RESET
                   " 경기 승리 +5코인 (전체 %d코인)\n", coin);
        }
        /* 시민 퍼펙트 처치 시 초심자의 운 해금 */
        if (current_enemy == 0 && e_score == 0 && !unlocked[3]) {
            unlocked[3] = 1;
            printf("  " C_BGREEN "[해금]" C_RESET " 초심자의 운 스타터 해금!\n");
        }
        print_line();
        press_enter();
        return 1;
    } else {
        printf("  " C_BRED "✖ 패배... 최종 스코어 %d:%d\n" C_RESET,
               p_score, e_score);
        print_line();
        press_enter();
        return -1;
    }
}

/* =========================================================
   런 초기화
   새 런(게임 1회) 시작 전 모든 전역 상태를 초기값으로 리셋
   ========================================================= */
void init_run(void) {
    coin = 0;
    for (int i = 0; i < ITEM_COUNT; i++) item_lv[i] = 0;
    current_enemy = 0;
    p_score = 0; e_score = 0;
    active_used = 0;
    shield_prob = 50;
    draw_streak = 0;
    skill_sealed = 0;
    skill_debuff = 0;
    last_move = -1;
    scissors_streak = 0;
    paper_win_streak = 0;
    perfect_win_count = 0;
    no_shop_first = 0;
    starter_id = 0;
}

/* =========================================================
   엔딩 화면
   ========================================================= */
void show_ending(void) {
    clear_screen();
    printf("\n\n");
    printf(C_BYELLOW "  +================================================+\n" C_RESET);
    printf(C_BYELLOW "  |" C_BWHITE "                                                " C_BYELLOW "|\n" C_RESET);
    printf(C_BYELLOW "  |" C_BWHITE "    🏆  챔피언을 쓰러뜨렸다!  🏆           " C_BYELLOW "|\n" C_RESET);
    printf(C_BYELLOW "  |" C_BWHITE "      가위바위보 토너먼트 우승!               " C_BYELLOW "|\n" C_RESET);
    printf(C_BYELLOW "  |" C_BWHITE "                                                " C_BYELLOW "|\n" C_RESET);
    printf(C_BYELLOW "  +================================================+\n" C_RESET);
    printf("\n");
    printf("  " C_BYELLOW "🪙 최종 코인: %d\n" C_RESET, coin);
    printf("  " C_BWHITE "아이템 최종 레벨:\n" C_RESET);
    for (int i = 0; i < ITEM_COUNT; i++)
        printf("    %s  Lv%d\n", item_names[i], item_lv[i]);
    printf("\n");
    print_line();
    press_enter();
}

/* =========================================================
   메인 게임 루프 (단일 런)
   스타터 선택 → 7경기 반복 → 패배 시 게임 오버 / 승리 시 엔딩
   ========================================================= */
void run_game(void) {
    init_run();
    select_starter();

    while (current_enemy < ENEMY_COUNT) {
        int mr = run_match();
        if (mr == -1) { /* 경기 패배: 게임 오버 */
            clear_screen();
            print_line();
            printf("  " C_BRED "💀 게임 오버... 다시 처음부터!\n" C_RESET);
            check_unlock(0); /* 패배 시에도 진행도 기반 해금 체크 */
            print_line();
            press_enter();
            return;
        }
        check_unlock(current_enemy == ENEMY_COUNT - 1); /* 마지막 상대면 클리어 해금 */
        if (current_enemy < ENEMY_COUNT - 1)
            run_shop(); /* 마지막 경기 후에는 상점 없음 */
        current_enemy++;
    }
    show_ending();
}

/* =========================================================
   타이틀 화면
   ========================================================= */
void show_title(void) {
    clear_screen();
    printf("\n\n");
    printf(C_BCYAN "        ✌  ✊  ✋\n\n" C_RESET);
    printf(C_BWHITE "    CLASH OF HANDS\n" C_RESET);
    printf(C_WHITE  "    7명의 상대를 처치하라\n\n" C_RESET);
    press_enter();
}

/* =========================================================
   프로그램 진입점
   ========================================================= */
int main(void) {
    srand((unsigned int)time(NULL)); /* 난수 시드 초기화 */

    /* 기본 스타터 3종(0~2) 해금, 나머지는 잠금 상태로 초기화 */
    memset(unlocked, 0, sizeof(unlocked));
    unlocked[0] = 1;
    unlocked[1] = 1;
    unlocked[2] = 1;

    show_title();

    /* 다시 도전 루프 */
    while (1) {
        run_game();
        clear_screen();
        printf("  " C_BWHITE "다시 도전하시겠습니까?" C_RESET
               C_BYELLOW " (y/n)" C_RESET ": ");
        char c; scanf(" %c", &c);
        if (c != 'y' && c != 'Y') break;
    }

    clear_screen();
    printf("  " C_BWHITE "게임 종료. 수고하셨습니다! 👋\n" C_RESET);
    return 0;
}