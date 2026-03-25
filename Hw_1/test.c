#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

// --- ANSI 색상 코드 ---
#define CLR_RESET   "\x1b[0m"
#define CLR_BOLD    "\x1b[1m"
#define CLR_RED     "\x1b[31m"
#define CLR_GREEN   "\x1b[32m"
#define CLR_YELLOW  "\x1b[33m"
#define CLR_BLUE    "\x1b[34m"
#define CLR_MAGENTA "\x1b[35m"
#define CLR_CYAN    "\x1b[36m"
#define CLR_WHITE   "\x1b[37m"

#define SLEEP(ms) usleep((ms) * 1000)
#define CLEAR_SCREEN() printf("\033[H\033[J")

typedef enum { SCISSORS = 0, ROCK = 1, PAPER = 2, NONE = 3 } Hand;
typedef enum { LOSE = 0, DRAW = 1, WIN = 2 } Result;

typedef struct {
    char name[30];
    int prob[3]; // 가위, 바위, 보 확률
    char skill_name[30];
    char skill_desc[120];
} Enemy;

typedef struct {
    int id;
    char name[30];
    char effect_desc[150];
    int price;
    int type; // 0:가위, 1:바위, 2:보, 3:패시브, 4:액티브
} Item;

typedef struct {
    int coins;
    int inventory[30];      
    int current_stage;      
    int active_item_id;     // 마지막으로 구매/장착한 액티브
    int active_ready;       // 전투당 1회 사용
    int win_streak;         // 연속 승리 카운트 (Blade Focus용)
    int lose_streak;        // 연속 패배 카운트 (Resilience용)
    Hand last_hand;         // Echo Stone용
    int pattern_count;      // Pattern Breaker용
    int amplified;          // Amplify 액티브 활성화 여부
} Player;

Player p = { 100, {0}, 0, -1, 1, 0, 0, NONE, 0, 0 };

// --- 아이템 데이터 (표 내용 반영) ---
Item shop_list[] = {
    {0, "Razor Edge", "가위 승리 시 확률적으로 추가 승리 판정", 40, 0},
    {1, "Blade Focus", "가위 2연속 사용 시 무승부 -> 승리", 35, 0},
    {2, "Phantom Blade", "가위 사용 시 확률적으로 바위로 판정", 45, 0},
    {3, "Titan Grip", "바위 패배 시 무승부로 변경", 50, 1},
    {4, "Crushing Force", "바위 승리 시 상대 스킬 발동률 감소", 45, 1},
    {5, "Echo Stone", "이전 선택 복사 (바위 선택 시)", 40, 1},
    {6, "Trap Card", "보 무승부 시 승리로 변경", 50, 2},
    {7, "Mind Sheet", "상대 주요 패 정보 제공", 35, 2},
    {8, "Swap Scroll", "상대 선택을 한 단계 변형 (가위->바위 등)", 45, 2},
    
    // 패시브
    {9, "Flow", "연속 선택 패널티 무시", 60, 3},
    {10, "Calm Mind", "무승부 시 20% 확률로 승리", 50, 3},
    {11, "Counter Sense", "패배 후 승리 시 추가 보상", 55, 3},
    {12, "Greed", "코인 획득 +30%, 패배 손실 증가", 45, 3},
    {13, "Pattern Breaker", "반복 선택 후 다른 선택 강화", 50, 3},
    {14, "Fortune", "확률 효과 +10% 증가", 65, 3},
    {15, "Resilience", "연속 패배 시 방어 확률 증가", 55, 3},

    // 액티브
    {16, "Retry", "결과 무효화 후 재경기", 30, 4},
    {17, "Force Win", "무승부 시 즉시 승리", 25, 4},
    {18, "Guard", "패배 시 무승부로 변경", 25, 4},
    {19, "Predict", "상대 패 미리 확인", 35, 4},
    {20, "Double Reward", "승리 보상 2배", 30, 4},
    {21, "Risk Gamble", "승리 시 2점, 패배 시 상대 2점", 20, 4},
    {22, "Lock Move", "상대 특정 패 봉쇄", 35, 4},
    {23, "Reverse", "결과 반전 (승<->패)", 40, 4},
    {24, "Stability", "확률 효과 무효화", 45, 4},
    {25, "Amplify", "장비 효과 2배 적용", 50, 4}
};

Enemy enemy_pool[] = {
    {"초보자", {33, 33, 34}, "None", "특이사항 없음"},
    {"가위술사", {70, 15, 15}, "Cutting Edge", "가위를 매우 선호함"},
    {"바위거인", {10, 80, 10}, "Solid Wall", "바위 위주로 공격함"},
    {"전략가", {40, 20, 40}, "Analysis", "보와 가위를 섞어 씀"}
};

// 판정 함수
Result judge(Hand ph, Hand eh) {
    if(ph == eh) return DRAW;
    if((ph == SCISSORS && eh == PAPER) || (ph == ROCK && eh == SCISSORS) || (ph == PAPER && eh == ROCK)) return WIN;
    return LOSE;
}

void show_shop() {
    while(1) {
        CLEAR_SCREEN();
        printf(CLR_BOLD CLR_GREEN "🛒 [ 아이템 상점 ]  보유: %d 코인\n" CLR_RESET, p.coins);
        printf("번호 | 이름 | 효과 | 가격\n");
        for(int i=0; i<26; i++) {
            printf("%2d. %-15s | %-40s | %dC %s\n", i, shop_list[i].name, shop_list[i].effect_desc, shop_list[i].price, p.inventory[i] ? "(보유)" : "");
        }
        printf("\n구매할 번호 (나가기 -1): ");
        int choice; scanf("%d", &choice);
        if(choice == -1) break;
        if(choice >= 0 && choice < 26) {
            if(p.coins >= shop_list[choice].price && !p.inventory[choice]) {
                p.coins -= shop_list[choice].price;
                p.inventory[choice] = 1;
                if(shop_list[choice].type == 4) p.active_item_id = choice;
                printf("구매 완료!\n"); SLEEP(600);
            } else {
                printf("구매 불가 (코인 부족 또는 이미 보유)\n"); SLEEP(600);
            }
        }
    }
}

int start_battle(int e_idx) {
    Enemy* e = &enemy_pool[e_idx];
    int p_score = 0, e_score = 0;
    p.active_ready = 1;
    p.win_streak = 0;
    p.last_hand = NONE;
    p.amplified = 0;
    int lock_hand = -1; // Lock Move용

    while(p_score < 3 && e_score < 3) {
        CLEAR_SCREEN();
        printf("⚔️ VS %s (상대 스킬: %s)\n", e->name, e->skill_desc);
        printf("내 점수: %d | 상대 점수: %d\n", p_score, e_score);
        if(p.inventory[7]) printf(CLR_CYAN "[Mind Sheet] 상대 주력: %s\n" CLR_RESET, e->prob[0] > 40 ? "가위" : (e->prob[1] > 40 ? "바위" : "보"));
        
        printf("1.가위 2.바위 3.보 | 5.액티브(%s)\n선택: ", p.active_item_id != -1 ? shop_list[p.active_item_id].name : "없음");
        int input; scanf("%d", &input);
        
        int act_used = 0;
        if(input == 5 && p.active_item_id != -1 && p.active_ready) {
            act_used = 1; p.active_ready = 0;
            printf(CLR_YELLOW "[%s] 발동!\n" CLR_RESET, shop_list[p.active_item_id].name);
            if(p.active_item_id == 19) { // Predict
                Hand next_e = (rand() % 100 < e->prob[0]) ? SCISSORS : (rand() % 100 < (e->prob[0]+e->prob[1]) ? ROCK : PAPER);
                printf("예측 성공: 상대는 [%s]를 냅니다!\n", next_e == 0 ? "가위" : (next_e == 1 ? "바위" : "보"));
            }
            if(p.active_item_id == 22) lock_hand = rand() % 3; // Lock Move
            if(p.active_item_id == 25) p.amplified = 1; // Amplify
            printf("수 선택: "); scanf("%d", &input);
        }

        Hand ph = (Hand)(input - 1);
        
        // Echo Stone: 바위 선택 시 이전 패 복사
        if(p.inventory[5] && ph == ROCK && p.last_hand != NONE) {
            printf("[Echo Stone] 이전 패(%d)를 복사합니다!\n", p.last_hand);
            ph = p.last_hand;
        }

        // Phantom Blade: 가위 -> 바위 판정
        if(p.inventory[2] && ph == SCISSORS && (rand() % 100 < 30)) {
            printf("[Phantom Blade] 가위가 바위의 힘을 얻습니다!\n");
            ph = ROCK;
        }

        // 상대 패 결정
        Hand eh = (rand() % 100 < e->prob[0]) ? SCISSORS : (rand() % 100 < (e->prob[0]+e->prob[1]) ? ROCK : PAPER);
        if(lock_hand == eh) eh = (eh + 1) % 3; // Lock Move 발동 시 패 변경
        
        // Swap Scroll
        if(p.inventory[8] && (rand() % 100 < 20)) {
            printf("[Swap Scroll] 상대의 패를 바꿉니다!\n");
            eh = (eh + 1) % 3;
        }

        printf("나: %d vs 상대: %d\n", ph, eh);
        Result res = judge(ph, eh);

        // --- 아이템 효과 적용 (판정 변경) ---
        int prob_bonus = p.inventory[14] ? 10 : 0; // Fortune

        // 가위: Blade Focus
        if(res == DRAW && ph == SCISSORS && p.win_streak >= 1 && p.inventory[1]) {
            printf("[Blade Focus] 연속 가위로 무승부를 승리로!\n");
            res = WIN;
        }
        // 바위: Titan Grip
        if(res == LOSE && ph == ROCK && p.inventory[3]) {
            printf("[Titan Grip] 바위의 방어력으로 무승부 처리!\n");
            res = DRAW;
        }
        // 보: Trap Card
        if(res == DRAW && ph == PAPER && p.inventory[6]) {
            printf("[Trap Card] 보 무승부를 승리로 변환!\n");
            res = WIN;
        }
        // 패시브: Calm Mind
        if(res == DRAW && p.inventory[10] && (rand() % 100 < (20 + prob_bonus))) {
            printf("[Calm Mind] 평정심으로 무승부를 승리로!\n");
            res = WIN;
        }
        // 액티브: Force Win, Guard, Reverse
        if(act_used) {
            if(p.active_item_id == 17 && res == DRAW) res = WIN;
            if(p.active_item_id == 18 && res == LOSE) res = DRAW;
            if(p.active_item_id == 23) res = (res == WIN) ? LOSE : (res == LOSE ? WIN : DRAW);
        }

        // --- 결과 처리 ---
        if(res == WIN) {
            int points = 1;
            if(act_used && p.active_item_id == 21) points = 2; // Risk Gamble
            if(p.inventory[0] && ph == SCISSORS && (rand() % 100 < (30 + prob_bonus))) {
                printf("[Razor Edge] 가위 추가 승리 판정!\n");
                points++;
            }
            p_score += points;
            p.win_streak++; p.lose_streak = 0;
            printf(CLR_GREEN "승리!\n" CLR_RESET);
        } else if(res == LOSE) {
            int points = 1;
            if(act_used && p.active_item_id == 21) points = 2;
            e_score += points;
            p.win_streak = 0; p.lose_streak++;
            printf(CLR_RED "패배...\n" CLR_RESET);
        } else {
            printf("무승부\n");
        }

        p.last_hand = ph;
        SLEEP(1000);
    }
    return (p_score >= 3);
}

int main() {
    srand(time(NULL));
    printf(CLR_BOLD CLR_CYAN "가위바위보 장비 시스템 RPG\n" CLR_RESET);
    
    while(p.current_stage < 4) {
        printf("\n--- 현재 스테이지: %d (보유 코인: %d) ---\n", p.current_stage + 1, p.coins);
        printf("1. 전투 시작\n2. 상점 방문\n0. 종료\n선택: ");
        int menu; scanf("%d", &menu);
        
        if(menu == 1) {
            if(start_battle(p.current_stage)) {
                int reward = 50;
                if(p.inventory[12]) reward *= 1.3; // Greed
                if(p.active_item_id == 20 && !p.active_ready) reward *= 2; // Double Reward
                printf(CLR_YELLOW "스테이지 클리어! 보상: %d 코인\n" CLR_RESET, reward);
                p.coins += reward;
                p.current_stage++;
            } else {
                printf(CLR_RED "게임 오버! 코인을 잃습니다.\n" CLR_RESET);
                if(p.inventory[12]) p.coins -= 40; else p.coins -= 20;
                if(p.coins < 0) p.coins = 0;
            }
        } else if(menu == 2) {
            show_shop();
        } else break;
    }
    
    printf("최종 결과: %d 코인 획득. 게임을 종료합니다.\n", p.coins);
    return 0;
}