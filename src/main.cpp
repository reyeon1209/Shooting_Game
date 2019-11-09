#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS
// NONSTDC WARNINGS: kbhit()/getch()가 보안강화로 다른 함수로 대체되었지만 기존 함수 그대로 쓰기위해 경고제거함.

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <time.h>
#include <conio.h>
#include <string.h>
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")
#include "startModel.h"
#include "mapModel.h"

// ===== ===== ===== ===== 전처리기
// stage and map
#define STAGE_MAX 5
#define MAP_MAXSIZE_Y 50
#define MAP_MAXSIZE_X 70

#define MAP_VISIBLE_RANGE_Y 16
#define MAP_VISIBLE_RANGE_X 16
// 실행시 맵의 렉을 줄이기 위해 일단 낮춤.

// player control
#define LEFT 75
#define RIGHT 77
#define UP 72
#define DOWN 80
#define SPACEBAR 32
#define PAUSE 112
#define SKIP 115
#define ATTACK 99

// attacker
#define PLAYER 1
#define MONSTER 2

// monster numbering
#define MEELE 50
#define STANDOFF 51

// monster state
#define ARRESTED 60

// map numbering
#define EMPTY 0
#define WALL 1
#define ITEM 2
#define GLASS 3
#define ENTRANCE 4
#define EXIT 5

// 콘솔창 크기
#define STATUS_PRINT 5
#define COMMAND_SIZE 256
#define LINE 20
#define COLS 40

// color code
#define BLACK 0
#define DEEPBLUE 1
#define DEEPGREEN 2
#define DEEPSKYBLUE 3
#define DEEPRED 4
#define DEEPPURPULE 5
#define DEEPYELLOW 6
#define GRAY 7
#define DEEPGRAY 8
#define BLUE 9
#define GREEN 10
#define SKYBLUE 11
#define RED 12
#define PURPLE N
#define YELLOW 14
#define WHITE 15

// ===== ===== ===== ===== 구조체
typedef struct Character {
	COORD pos;	// 캐릭터의 좌표
	int hp;		// 캐릭터의 체력
	int vector;	// 캐릭터가 바라보는 방향
	int frameSpeed;	// 프레임 당 반응속도, 100 -> 0.1초에 한 번 이동.
	int attackPower;// 캐릭터가 총알/공격을 할 때 공격력
	int maxBulletRange;	// 총알 발사시 총알의 최대 이동거리
	int avaliable;	// 사용가능여부
	int keyState;	// 방향키 상태
}Character;

typedef struct Monster {
	Character charaInfo;
	int attackRangeType;	// 근접(MEELE)인지, 원거리(STANDOFF)인지
	int attackCooltime;		// 공격 쿨타임.
	int attackTimer;		// 공격 쿨타임 타이머
	int isDetectedPlayer;	// 플레이어를 감지했는지 확인하는 변수
	int detectionRange;		// 플레이어 감지 범위
	int noneDectectedAction;	// 플레이어가 감지되지 않았을 때, 무엇을 하는가
	int detectedAction;	// 플레이어가 감지되었을 때, 무엇을 하는가.
	int state;			// 봉인 등 상태이상
	int stateTimer;		// 상태이상의 타이머
	int stateLimit;		// 상태이상 최대시간(봉인이 시간이 지나면 풀리는 둥)
}Monster;

typedef struct Bullet {
	COORD pos;		// 총알 현재 위치
	int vector;	// 총알 이동 방향
	int frameSpeed;	// 프레임 당 반응속도, 100 -> 0.1초에 한 번 이동.
	int attackPower;	// 총알의 공격력
	int maxRange;		// 총알 발사지점으로부터 최대 이동거리
	int currentDistance;	// 총알이 발사지점으로부터 이동거리
	int avaliable;	// 사용가능여부
	int attacker;	// 공격한자가 누구인지 (플레이어, 몬스터)
}Bullet;

typedef struct Item {
	COORD pos;
	int avaliable;
	int type;

}Item;

typedef struct Boss {
	Character charaInfo;
	int skillTimer[5];
	int skillCooltime[5];
	int state;
	int stateTimer;
	int stateLimit;
	int isActing[5];
	int R1;
	int R2;
}Boss;


// 전역변수
Character *player;
//int map[STAGE_MAX][MAP_MAXSIZE_Y][MAP_MAXSIZE_X];
Bullet *playerBullet;
Monster *monsterList;
Bullet *monsterBullet;
Boss boss;
int curStage = 0;
int maxPlayerBullet;
int maxMonster;
int maxMonsterBullet;
int gameTimer;
int score = 0;
double timeLimit = 100.0;
char item[50] = "없음";
char itemList[6][22] = { { "Bomb" },{ "Increase Attack Range" },{ "Increase Life" },{ "Increase View" },{ "Reverse Key" },{ "Decrease View" } };
COORD PCpos;
Item *dropedItemsList;
Item *itemsList;
int maxItem = 0;
int maxDropedItem = 0;
int sightChange = 0;
int sightFlag = 1;
int bombcnt = 0;
clock_t invinTime = 0;
char bossAppearance[7][7][3] =
{
	{ "a " ,"a " ,"a " ,"a " ,"a " ,"a " ,"a " } ,
	{ "a ","a " ,"a " ,"a " ,"a " ,"a "  ,"a " },
	{ "a ","a " ,"a " ,"a " ,"a " ,"a " ,"a " },
	{ "a ","a " ,"a " ,"★" ,"a " ,"a "  ,"a " },
	{ "a ","a " ,"a " ,"a " ,"a " ,"a "  ,"a " },
	{ "a ","a " ,"a " ,"a " ,"a " ,"a "  ,"a " },
	{ "a " ,"a " ,"a " ,"a " ,"a " ,"a " ,"a " }
};
int keyCnt = 0; //keyinput횟수제한
clock_t mid, finish;
//맵에 배치된 아이템 리스트


// ===== ===== ===== ===== 함수선언


// 플레이어
void InitPlayer(void);	// 플레이어 초기화
void SetPlayerPos(int x, int y);	// 플레이어 위치설정
COORD GetPlayerPos(void);		// 플레이어 위치반환 (전역이다보니 필요 없는것같기도...)
void MovePlayerToLeft(void);	// 플레이어 이동(상대적이동 == 주변 한칸으로 이동)
void MovePlayerToRight(void);
void MovePlayerToUp(void);
void MovePlayerToDown(void);
void MovePlayerToLeftIgnoringMonster(void);	// 플레이어 이동(상대적이동 == 주변 한칸으로 이동)
void MovePlayerToRightIgnoringMonster(void);
void MovePlayerToUpIgnoringMonster(void);
void MovePlayerToDownIgnoringMonster(void);
void PlayerShoot(void);			// 플레이어의 공격
void PlayerBeAttacked(void);
int TherePlayer(int x, int y);
void InitPlayerPos(void);

// 유저 키 입력 및 상호작용
void InputKeyProcess(void);
void ReverseProcessKeyInput(void);

// 플레이어 총알
void InitPlayerBullet(void);	// 유저총알 초기화
int isTherePlayerBullet(int x, int y);
void PlayerBulletRoutine(void);	// 유저총알 루틴(매 초마다 진행)

								// 몬스터
void InitMonster(void);
void SetMonsterToMelee(int x, int y);
void SetMonsterToStandoff(int x, int y);
int MoveMonsterToLeft(int ind);
int MoveMonsterToRight(int ind);
int MoveMonsterToUp(int ind);
int MoveMonsterToDown(int ind);
int IsCollisionToMonsterPosX(int x, int exceptionMonsterNum);
int IsCollisionToMonsterPosY(int y, int exceptionMonsterNum);
int ThereIsMonster(int x, int y, int exceptionMonsterNum);
void MonsterShoot(int ind, int arrow);
void MonsterBeShooted(int monsterInd, int attackPower);
void checkArrTime(int monsterInd);
void MonsterDead(int monsterInd);
void MonsterRoutine(void);
void MonsterRandomPlacement(void);
void DropItemAroundDeadMonster(int monsterInd);
void MonsterTryToChase(int arrow, int monsterNumber);
int IsOnMonsterAttackRangePlayer(int monsterNumber);

// 몬스터 총알
void InitMonsterBullet(void);	// 몬스터 총알 초기화
void MonsterBulletRoutine(void); // 몬스터 총알 루틴

								 // 보스
void InitBossInfo();
void SetBoss(int x, int y);
int MoveBoss(int xp, int yp);
int IsColliseToBoss(int x, int y);
void BossChasePlayer();
void BossChasePlayerInPerpen(int arrow);
void BossRoutine();
int BossSkill0();
int BossSkill1();
int BossSkill2();
int IsRemindedBossSkillTime();
void ReduceBossSkillTimer();
int BossRushToArrow(int arrow);
void BossCircleShoot(int edgeRadius);
void MakeMonsterBullet(int x, int y, int arrow, int range);
void BossShoot(int arrow, int relativeLocation);
void BossBeAttacked(int part);
void BossSkillEnd(int ind);
void MoveBossBackArrow(int arrow);
int IsInBossBody(int x, int y);
int myAbs(int n);
int myNormalize(int n);
int getArrowToPlayer(COORD pos);
void DropItemAroundBoss(void);
void DropMonsterAroundBoss(void);


int IsWall(int x, int y);	// 해당 위치가 벽인지 탐색
int IsGlass(int x, int y);	// 해당 위치가 유리인지 탐색

							// 콘솔관련
void SetCurrentCursorPos(int x, int y);	// 커서위치 설정
COORD GetCurrentCursorPos(void);		// 커서위치 불러오기
void RemoveCursor(void);			// 커서 지우기, 게임 실행시 1회 사용
void ChangePen(int colorCode);		// 글자 색 바꾸기
void FixDisplayFromStatus(void);	// Status창과 맵이 부딪치지 않게 Status창 만큼 더해준다.

									// 디스플레이
void DisplayAroundPlayer();
void StaticPlayerDisplay();

void GameStart(char startModel[16][16]);
void BossMapStart();
void ShowPlayerStatus(int life, int score, double timeLimit, int stage, char item[50]);
void ShowMap(char mapModel[16][40]);
void ShowPlayer(COORD PCPos);
void ShowZombies(void);
void SetScreen(void);
void PauseScreen(void);

//아이템
int ChoiceGoodItem(void);
int ChoiceAllItem(void);
void SetItem(void);
void ChangePlayerStatus(int Item_ind);
void Bomb(void);
void IncreaseAttackRange(void);
void IncreaseLife(void);
void IncreaseView(void);
void ReverseKey(void);
int checkRevTime(clock_t mid);
void DecreaseView(void);
void InitDropedItemsList(void);
void InitItemsList(void);

//게임오버
int GameOver(void);
void printGameOver();
void freeAll(void);
int GameClear();
void printGameClear();

int main()
{
	int i;
	clock_t start;

	SetScreen();

	srand(time(NULL));
	gameTimer = 0;
	RemoveCursor();
	PlaySound(TEXT("start.wav"), NULL, SND_ASYNC | SND_LOOP);
	//GameStart(startModel[0]);		// 시작화면 및 튜토리얼 display
	PlaySound(NULL, NULL, SND_ASYNC | SND_LOOP);

	InitPlayer();
	InitPlayerBullet();
	InitMonster();
	InitMonsterBullet();
	InitDropedItemsList();
	InitItemsList();
	InitPlayerPos();
	InitBossInfo();
	MonsterRandomPlacement();

	/*// ===== 보스맵!! 모두 주석처리하면 스테이지1부터 =====
	curStage = 2;
	SetBoss(30, 15);
	player->pos.X = 30;
	player->pos.Y = 20;

	freeAll();
	BossMapStart();

	InitMonster();
	InitMonsterBullet();
	InitDropedItemsList();
	InitItemsList();
	InitPlayerPos();
	MonsterRandomPlacement();
	//=====================================================*/

	start = clock();
	while (1)
	{
		if (mapModel[curStage][player->pos.Y][player->pos.X] == EXIT)
		{
			freeAll();

			if (curStage < 2)
			{
				score += timeLimit;
				curStage++;
				PlaySound(TEXT("clear.wav"), NULL, SND_ASYNC);
			}
			if (curStage == 2)
				BossMapStart();

			InitMonster();
			InitMonsterBullet();
			InitDropedItemsList();
			InitItemsList();
			InitPlayerPos();
			MonsterRandomPlacement();
			if (curStage == 2) SetBoss(30, 15);

			start = clock();
		}

		if ((timeLimit == 0) || (GameOver())) {
			printGameOver();
			return 0; ////////////////////게임끝
		}

		if ((curStage == 2) && (boss.charaInfo.avaliable == 0))
		{
			printGameClear();
			return 0;
		}

		finish = clock();
		for (i = 0; i < 5; i++) {

			keyCnt = 0;

			if (player->keyState)
				InputKeyProcess();
			else
			{
				ReverseProcessKeyInput();
				if (checkRevTime(mid))
					player->keyState = 1;
			}
			//player->hp = 10;
			BossRoutine();

			fflush(stdin); // 비표준
			//if (curStage == 0)
				Sleep(50);	// 몬스터 속도 조절
			//else
				//Sleep(30);
			//gameTimer += 50;
		}
		PlayerBulletRoutine();
		MonsterRoutine();
		MonsterBulletRoutine();
		//DisplayAroundPlayer();
		StaticPlayerDisplay();
		for (int i = 0; i < maxMonster; i++)
			checkArrTime(i);
		//ShowMap(mapModel[0]); -> DisplayAroundPlayer과 동일
		//ShowPlayer(PCpos);
		//ShowZombies();

		finish = clock();

		if (curStage == 0)
			timeLimit = 100 - (double)(finish - start) / CLOCKS_PER_SEC;
		else if (curStage == 1)
			timeLimit = 150 - (double)(finish - start) / CLOCKS_PER_SEC;
		else
			timeLimit = 300 - (double)(finish - start) / CLOCKS_PER_SEC;

		ShowPlayerStatus(player->hp, score, timeLimit, 3 - curStage, item);
	}

	return 0;
}

void SetCurrentCursorPos(int x, int y) {
	COORD pos = { x, y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

COORD GetCurrentCursorPos(void) {
	COORD curPoint;
	CONSOLE_SCREEN_BUFFER_INFO curInfo;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curPoint.X = curInfo.dwCursorPosition.X;
	curPoint.Y = curInfo.dwCursorPosition.Y;
	return curPoint;
}

void RemoveCursor(void) {
	CONSOLE_CURSOR_INFO curInfo;
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curInfo.bVisible = 0;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}

void ChangePen(int colorCode)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorCode);
}

void FixDisplayFromStatus(void)
{
	COORD pos = GetCurrentCursorPos();
	SetCurrentCursorPos(pos.X + 1, pos.Y + STATUS_PRINT);
}


void GameStart(char startModel[16][16]) {
	int x, y;
	int flag, in_key;
	int i, j;
	SetCurrentCursorPos(14, 1);
	printf("율  곡  탈  출");
	SetCurrentCursorPos(4, 3);
	COORD curPos = GetCurrentCursorPos();
	for (y = 0; y<16; y++) {
		for (x = 0; x<16; x++) {
			SetCurrentCursorPos(curPos.X + (x * 2), curPos.Y + y);
			if (startModel[y][x] == 1)
				printf("■");
		}
	}
	Sleep(2500);
	SetCurrentCursorPos(0, 0);
	Sleep(1500);
	for (y = 0; y<40; y++) {
		for (x = 0; x<20; x++) {
			SetCurrentCursorPos(curPos.X + (x * 2), curPos.Y + y);
			if (startModel[y][x] == 1)
				printf(" ");
		}
	}

	curPos.X = 2, curPos.Y = 1;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("How to play                      \n");
	curPos.Y += 3;
	ChangePen(YELLOW);
	printf("  시간 내에 미로를 탈출하고 \n  보스 좀비를 잡아라!");
	ChangePen(WHITE);
	curPos.Y += 2;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("방향키 : ↑, ←, ↓, → ");
	curPos.Y += 1;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("공격키 : SPACE BAR ");
	curPos.Y += 1;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("폭탄 아이템 사용 키 : c ");
	curPos.Y += 2;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("좀비에게 총알 쏜 뒤 \n  10초 내에 몸통박치기");
	curPos.Y += 6;
	SetCurrentCursorPos(curPos.X, curPos.Y);

	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("(화면 스킵 : s)");
	curPos.X = 2; curPos.Y = 1;

	while (1)
	{
		flag = 0;
		for (int i = 0; i < 20; i++)
		{
			if (kbhit() != 0)
			{
				int in_key = getch();

				if (in_key == SKIP)	flag = 1;
			}
		}
		if (flag == 1) break;
		Sleep(1500);
	}

	for (y = 0; y < 40; y++)
		for (x = 0; x < 20; x++)
		{
			SetCurrentCursorPos(curPos.X + (x * 2), curPos.Y + y);
			printf("    ");
		}

	curPos.X = 2, curPos.Y = 1;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("Ways to get score ");
	curPos.Y += 3;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("좀비 처치 시 + ");
	curPos.Y++;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("시간을 남기고 스테이지를 완료 시 + ");
	curPos.Y++;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("생명을 남기고 스테이지를 완료 시 + ");
	curPos.Y++;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("생명 FULL에서 생명아이템 획득 시 + ");
	curPos.Y += 9;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("(화면 스킵 : s)");
	curPos.X = 2; curPos.Y = 1;

	while (1)
	{
		flag = 0;
		for (i = 0; i < 20; i++)
		{
			if (kbhit() != 0)
			{
				int in_key = getch();

				if (in_key == SKIP)	flag = 1;
			}
		}
		if (flag == 1) break;
		Sleep(1500);
	}

	for (y = 0; y < 40; y++)
		for (x = 0; x < 20; x++)
		{
			SetCurrentCursorPos(curPos.X + (x * 2), curPos.Y + y);
			printf("    ");
		}

	curPos.X = 2, curPos.Y = 1;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("                                ");
	//curPos.Y += 2;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("좋은 아이템 : ");
	ChangePen(DEEPPURPULE);
	printf("◈");
	ChangePen(WHITE);
	curPos.Y++;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("공격 사거리+, 체력+, 시야+, 폭탄");
	curPos.Y += 2;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("랜덤 아이템 : ");
	ChangePen(DEEPYELLOW);
	printf("◈");
	ChangePen(WHITE);
	curPos.Y++;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("공격 사거리+, 체력+, 시야+, 폭탄,\n  시야-, 방향키 반대 ");
	curPos.Y += 3;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("몬스터 : ");
	ChangePen(RED);
	printf("♠");
	ChangePen(WHITE);
	curPos.Y++;

	curPos.Y += 7;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("(화면 스킵 : s)");
	curPos.X = 2; curPos.Y = 1;

	while (1)
	{
		flag = 0;
		for (i = 0; i < 20; i++)
		{
			if (kbhit() != 0)
			{
				in_key = getch();

				if (in_key == SKIP)	flag = 1;
			}
		}
		if (flag == 1) break;
		Sleep(1500);
	}

	for (int y = 0; y < 40; y++)
		for (int x = 0; x < 20; x++)
		{
			SetCurrentCursorPos(curPos.X + (x * 2), curPos.Y + y);
			printf("    ");
		}

	//여기부터 시나리오 화면
	curPos.X = 2; curPos.Y = 8;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("기말고사 3일 전 율곡관...");
	Sleep(1000);
	curPos.X = 2; curPos.Y = 8;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("                                   ");
	curPos.X = 2; curPos.Y = 8;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("3층 빈 강의실에서 공부하던 김소웨");
	Sleep(1000);
	curPos.X = 2; curPos.Y = 8;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("                                   ");
	curPos.X = 2; curPos.Y = 8;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("깜빡 잠이 들었다가 깨어난 김소웨는");
	Sleep(1000);
	curPos.X = 2; curPos.Y = 8;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("                                   ");
	curPos.X = 2; curPos.Y = 8;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("물을 마시러 복도로 나갔다가");
	Sleep(1000);
	curPos.X = 2; curPos.Y = 8;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("                                   ");
	curPos.X = 2; curPos.Y = 8;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("좀비로 변한 사람들과 마주친다");
	Sleep(1000);
	curPos.X = 2; curPos.Y = 8;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("                                   ");
	curPos.X = 2;
	curPos.Y = 8;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("과연 김소웨는 좀비들을 무찌르고");
	Sleep(1000);
	curPos.X = 2; curPos.Y = 8;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("                                    ");
	curPos.X = 2; curPos.Y = 8;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("집으로 돌아갈 수 있을까??");
	Sleep(1000);
	curPos.X = 2; curPos.Y = 8;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("                                 ");
}

void BossMapStart() {
	int x, y;
	int flag, in_key;
	int i, j;
	COORD curPos = GetCurrentCursorPos();

	for (y = 0; y < 40; y++)
		for (x = 0; x < 20; x++)
		{
			SetCurrentCursorPos(curPos.X + (x * 2), curPos.Y + y);
			printf("    ");
		}

	curPos.X = 2, curPos.Y = 1;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("How to play                      \n");
	curPos.Y += 3;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	ChangePen(WHITE);
	printf("보스의 테두리를 공격하여");
	curPos.Y += 1;
	SetCurrentCursorPos(curPos.X, curPos.Y);
	ChangePen(SKYBLUE);
	printf("●");
	ChangePen(WHITE);
	printf("에 가두어라!");
	curPos.Y += 10;
	SetCurrentCursorPos(curPos.X, curPos.Y);

	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("(화면 스킵 : s)");
	curPos.X = 2; curPos.Y = 1;

	while (1)
	{
		flag = 0;
		for (int i = 0; i < 20; i++)
		{
			if (kbhit() != 0)
			{
				int in_key = getch();

				if (in_key == SKIP)	flag = 1;
			}
		}
		if (flag == 1) break;
		Sleep(1500);
	}

	for (y = 0; y < 40; y++)
		for (x = 0; x < 20; x++)
		{
			SetCurrentCursorPos(curPos.X + (x * 2), curPos.Y + y);
			printf("    ");
		}
}

void ShowPlayerStatus(int life, int score, double timeLimit, int stage, char item[50]) {
	int i;
	SetCurrentCursorPos(0, 0);
	printf("LIFE : ");
	for (i = 0; i<life; i++) {
		printf("♥");
	}
	printf(" (%d)", life);
	printf("           ");
	printf("\n");

	COORD curPos = GetCurrentCursorPos();
	printf("                            ");
	SetCurrentCursorPos(curPos.X, curPos.Y);
	printf("ITEM : %s\n", item);
	printf("SCORE : %d                   Bomb : %d \n", score, bombcnt);
	printf("TIME LIMIT : %.0f             FLOOR : %d \n", timeLimit, stage);
	printf("----------------------------------------\n");
	//	SetCurrentCursorPos(108, 1);
	//	printf("FLOOR : %d\n", stage);
	//	SetCurrentCursorPos(108, 2);
	//	printf("ITEM : %s", item);
}

void ShowMap(char mapModel[16][40]) {
	int x, y;
	SetCurrentCursorPos(0, 5);
	COORD curPos = GetCurrentCursorPos();
	for (y = 0; y<16; y++) {
		for (x = 0; x<40; x++) {
			SetCurrentCursorPos(curPos.X + (x * 2), curPos.Y + y);
			if (mapModel[y][x] == 1)
				printf("■");
			if (mapModel[y][x] == 2)
				printf("ⓘ");
		}
	}
}

void ShowPlayer(COORD PCPos) {
	SetCurrentCursorPos(PCPos.X, PCPos.Y);
	printf("♤");
}

void ShowZombies(void) {
	COORD NPCpos[5];
	int i;
	if (curStage == 0) {
		NPCpos[0].X = 11 * 2;
		NPCpos[0].Y = 5 + 3;
		NPCpos[1].X = 8 * 2;
		NPCpos[1].Y = 5 + 2;
		NPCpos[2].X = 23 * 2;
		NPCpos[2].Y = 5 + 3;
		NPCpos[3].X = 15 * 2;
		NPCpos[3].Y = 5 + 10;
		NPCpos[4].X = 21 * 2;
		NPCpos[4].Y = 5 + 15;
	}

	for (i = 0; i<5; i++) {
		SetCurrentCursorPos(NPCpos[i].X, NPCpos[i].Y);
		printf("∇");
	}

}

void SetScreen(void)
{
	char command[COMMAND_SIZE] = { '\0', };
	sprintf(command, "mode con: lines=%d cols=%d", LINE, COLS);
	system(command);
}

///////////////////////////////////////////

void InitPlayer(void)
{
	COORD initPos = { 3, 3 };
	player = (Character*)malloc(sizeof(Character));
	player->attackPower = 1;
	player->vector = UP;
	//player->frameSpeed = 0;	// player->frameSpeed는 필요 없는 것 같...
	player->hp = 5;
	player->maxBulletRange = 2;
	player->pos = initPos;
	player->avaliable = 0;
	player->keyState = 1;
}

void SetPlayerPos(int x, int y) {
	COORD setPos = { x,y };
	if (IsWall(x, y)) return;
	if (IsGlass(x, y)) return;
	player->pos = setPos;
}

COORD GetPlayerPos(void) {
	return player->pos;
}

void MovePlayerToLeft(void)
{
	int i = 0;
	int colMob;
	player->vector = LEFT;
	if (IsWall(player->pos.X - 1, player->pos.Y)) return;
	if (IsGlass(player->pos.X - 1, player->pos.Y)) return;

	colMob = ThereIsMonster(player->pos.X - 1, player->pos.Y, -1);
	if (colMob != -1) {
		if (monsterList[colMob].state == ARRESTED) {
			MonsterDead(colMob);
		}
		else {
			PlayerBeAttacked();
			return;
		}
	}

	colMob = IsColliseToBoss(player->pos.X - 1, player->pos.Y);
	if (colMob && boss.state != ARRESTED) return;

	player->pos.X -= 1;

	for (i = 0; i<maxDropedItem; i++) {
		if ((dropedItemsList[i].avaliable == 1) && (dropedItemsList[i].pos.X == player->pos.X) && (dropedItemsList[i].pos.Y == player->pos.Y))
		{
			PlaySound(TEXT("item.wav"), NULL, SND_ASYNC);
			ChangePlayerStatus(dropedItemsList[i].type);
			dropedItemsList[i].avaliable = 0;
		}
	}

	for (i = 0; i < maxItem; i++) {
		if ((itemsList[i].avaliable == 1) && (itemsList[i].pos.X == player->pos.X) && (itemsList[i].pos.Y == player->pos.Y))
		{
			PlaySound(TEXT("item.wav"), NULL, SND_ASYNC);
			ChangePlayerStatus(itemsList[i].type);
			itemsList[i].avaliable = 0;
		}
	}
}
void MovePlayerToRight(void)
{
	int i = 0;
	int colMob;
	player->vector = RIGHT;
	if (IsWall(player->pos.X + 1, player->pos.Y)) return;
	if (IsGlass(player->pos.X + 1, player->pos.Y)) return;

	colMob = ThereIsMonster(player->pos.X + 1, player->pos.Y, -1);
	if (colMob != -1) {
		if (monsterList[colMob].state == ARRESTED) {
			MonsterDead(colMob);
		}
		else {
			PlayerBeAttacked();
			return;
		}
	}

	colMob = IsColliseToBoss(player->pos.X + 1, player->pos.Y);
	if (colMob && boss.state != ARRESTED) return;

	player->pos.X += 1;

	for (i = 0; i<maxDropedItem; i++) {
		if ((dropedItemsList[i].avaliable == 1) && (dropedItemsList[i].pos.X == player->pos.X) && (dropedItemsList[i].pos.Y == player->pos.Y))
		{
			PlaySound(TEXT("item.wav"), NULL, SND_ASYNC);
			ChangePlayerStatus(dropedItemsList[i].type);
			dropedItemsList[i].avaliable = 0;
		}
	}
	for (i = 0; i < maxItem; i++) {
		if ((itemsList[i].avaliable == 1) && (itemsList[i].pos.X == player->pos.X) && (itemsList[i].pos.Y == player->pos.Y))
		{
			PlaySound(TEXT("item.wav"), NULL, SND_ASYNC);
			ChangePlayerStatus(itemsList[i].type);
			itemsList[i].avaliable = 0;
		}
	}
}
void MovePlayerToUp(void)
{
	int i = 0;
	int colMob;
	player->vector = UP;
	if (IsWall(player->pos.X, player->pos.Y - 1)) return;
	if (IsGlass(player->pos.X, player->pos.Y - 1)) return;

	colMob = ThereIsMonster(player->pos.X, player->pos.Y - 1, -1);
	if (colMob != -1) {
		if (monsterList[colMob].state == ARRESTED) {
			MonsterDead(colMob);
		}
		else {
			PlayerBeAttacked();
			return;
		}
	}

	colMob = IsColliseToBoss(player->pos.X, player->pos.Y - 1);
	if (colMob && boss.state != ARRESTED) return;

	player->pos.Y -= 1;

	for (i = 0; i<maxDropedItem; i++) {
		if ((dropedItemsList[i].avaliable == 1) && (dropedItemsList[i].pos.X == player->pos.X) && (dropedItemsList[i].pos.Y == player->pos.Y))
		{
			PlaySound(TEXT("item.wav"), NULL, SND_ASYNC);
			ChangePlayerStatus(dropedItemsList[i].type);
			dropedItemsList[i].avaliable = 0;
		}
	}
	for (i = 0; i < maxItem; i++) {
		if ((itemsList[i].avaliable == 1) && (itemsList[i].pos.X == player->pos.X) && (itemsList[i].pos.Y == player->pos.Y))
		{
			PlaySound(TEXT("item.wav"), NULL, SND_ASYNC);
			ChangePlayerStatus(itemsList[i].type);
			itemsList[i].avaliable = 0;
		}
	}
}
void MovePlayerToDown(void)
{
	int i = 0;
	int colMob;
	player->vector = DOWN;
	if (IsWall(player->pos.X, player->pos.Y + 1)) return;
	if (IsGlass(player->pos.X, player->pos.Y + 1)) return;

	colMob = ThereIsMonster(player->pos.X, player->pos.Y + 1, -1);
	if (colMob != -1) {
		if (monsterList[colMob].state == ARRESTED) {
			MonsterDead(colMob);
		}
		else {
			PlayerBeAttacked();
			return;
		}
	}

	colMob = IsColliseToBoss(player->pos.X, player->pos.Y + 1);
	if (colMob && boss.state != ARRESTED) return;

	player->pos.Y += 1;

	for (i = 0; i<maxDropedItem; i++) {
		if ((dropedItemsList[i].avaliable == 1) && (dropedItemsList[i].pos.X == player->pos.X) && (dropedItemsList[i].pos.Y == player->pos.Y))
		{
			PlaySound(TEXT("item.wav"), NULL, SND_ASYNC);
			ChangePlayerStatus(dropedItemsList[i].type);
			dropedItemsList[i].avaliable = 0;
		}
	}

	for (i = 0; i < maxItem; i++)
	{
		if ((itemsList[i].avaliable == 1) && (itemsList[i].pos.X == player->pos.X) && (itemsList[i].pos.Y == player->pos.Y))
		{
			PlaySound(TEXT("item.wav"), NULL, SND_ASYNC);
			ChangePlayerStatus(itemsList[i].type);
			itemsList[i].avaliable = 0;
		}
	}
}
void MovePlayerToLeftIgnoringMonster(void)
{
	int i = 0;
	int colMob;
	player->vector = LEFT;
	if (IsWall(player->pos.X - 1, player->pos.Y)) return;
	if (IsGlass(player->pos.X - 1, player->pos.Y)) return;

	colMob = IsColliseToBoss(player->pos.X - 1, player->pos.Y);
	if (colMob) return;

	player->pos.X -= 1;

	for (i = 0; i<maxDropedItem; i++) {
		if ((dropedItemsList[i].avaliable == 1) && (dropedItemsList[i].pos.X == player->pos.X) && (dropedItemsList[i].pos.Y == player->pos.Y))
		{
			PlaySound(TEXT("item.wav"), NULL, SND_ASYNC);
			ChangePlayerStatus(dropedItemsList[i].type);
			dropedItemsList[i].avaliable = 0;
		}
	}

	for (i = 0; i < maxItem; i++) {
		if ((itemsList[i].avaliable == 1) && (itemsList[i].pos.X == player->pos.X) && (itemsList[i].pos.Y == player->pos.Y))
		{
			PlaySound(TEXT("item.wav"), NULL, SND_ASYNC);
			ChangePlayerStatus(itemsList[i].type);
			itemsList[i].avaliable = 0;
		}
	}
}
void MovePlayerToRightIgnoringMonster(void)
{
	int i = 0;
	int colMob;
	player->vector = RIGHT;
	if (IsWall(player->pos.X + 1, player->pos.Y)) return;
	if (IsGlass(player->pos.X + 1, player->pos.Y)) return;

	colMob = IsColliseToBoss(player->pos.X + 1, player->pos.Y);
	if (colMob) return;

	player->pos.X += 1;

	for (i = 0; i<maxDropedItem; i++) {
		if ((dropedItemsList[i].avaliable == 1) && (dropedItemsList[i].pos.X == player->pos.X) && (dropedItemsList[i].pos.Y == player->pos.Y))
		{
			PlaySound(TEXT("item.wav"), NULL, SND_ASYNC);
			ChangePlayerStatus(dropedItemsList[i].type);
			dropedItemsList[i].avaliable = 0;
		}
	}
	for (i = 0; i < maxItem; i++) {
		if ((itemsList[i].avaliable == 1) && (itemsList[i].pos.X == player->pos.X) && (itemsList[i].pos.Y == player->pos.Y))
		{
			PlaySound(TEXT("item.wav"), NULL, SND_ASYNC);
			ChangePlayerStatus(itemsList[i].type);
			itemsList[i].avaliable = 0;
		}
	}
}
void MovePlayerToUpIgnoringMonster(void)
{
	int i = 0;
	int colMob;
	player->vector = UP;
	if (IsWall(player->pos.X, player->pos.Y - 1)) return;
	if (IsGlass(player->pos.X, player->pos.Y - 1)) return;

	colMob = IsColliseToBoss(player->pos.X, player->pos.Y - 1);
	if (colMob) return;

	player->pos.Y -= 1;

	for (i = 0; i<maxDropedItem; i++) {
		if ((dropedItemsList[i].avaliable == 1) && (dropedItemsList[i].pos.X == player->pos.X) && (dropedItemsList[i].pos.Y == player->pos.Y))
		{
			PlaySound(TEXT("item.wav"), NULL, SND_ASYNC);
			ChangePlayerStatus(dropedItemsList[i].type);
			dropedItemsList[i].avaliable = 0;
		}
	}
	for (i = 0; i < maxItem; i++) {
		if ((itemsList[i].avaliable == 1) && (itemsList[i].pos.X == player->pos.X) && (itemsList[i].pos.Y == player->pos.Y))
		{
			PlaySound(TEXT("item.wav"), NULL, SND_ASYNC);
			ChangePlayerStatus(itemsList[i].type);
			itemsList[i].avaliable = 0;
		}
	}
}
void MovePlayerToDownIgnoringMonster(void)
{
	int i = 0;
	int colMob;
	player->vector = DOWN;
	if (IsWall(player->pos.X, player->pos.Y + 1)) return;
	if (IsGlass(player->pos.X, player->pos.Y + 1)) return;

	colMob = IsColliseToBoss(player->pos.X, player->pos.Y + 1);
	if (colMob) return;

	player->pos.Y += 1;

	for (i = 0; i<maxDropedItem; i++) {
		if ((dropedItemsList[i].avaliable == 1) && (dropedItemsList[i].pos.X == player->pos.X) && (dropedItemsList[i].pos.Y == player->pos.Y))
		{
			PlaySound(TEXT("item.wav"), NULL, SND_ASYNC);
			ChangePlayerStatus(dropedItemsList[i].type);
			dropedItemsList[i].avaliable = 0;
		}
	}

	for (i = 0; i < maxItem; i++)
	{
		if ((itemsList[i].avaliable == 1) && (itemsList[i].pos.X == player->pos.X) && (itemsList[i].pos.Y == player->pos.Y))
		{
			PlaySound(TEXT("item.wav"), NULL, SND_ASYNC);
			ChangePlayerStatus(itemsList[i].type);
			itemsList[i].avaliable = 0;
		}
	}
}


void PlayerShoot(void)
{
	PlaySound(TEXT("shoot.wav"), NULL, SND_ASYNC);

	int i;

	if (curStage != 2) {
		for (i = 0; i < maxPlayerBullet; i++) {
			if (playerBullet[i].avaliable == 1) {
				continue;
			}
			playerBullet[i].pos = player->pos;
			playerBullet[i].vector = player->vector;
			playerBullet[i].currentDistance = 0;
			playerBullet[i].avaliable = 1;
			break;
		}
	}

	else {
		for (i = 0; i < maxPlayerBullet; i++) {
			if (playerBullet[i].avaliable == 1) {
				continue;
			}
			if (i % 3 == 0) {
				playerBullet[i].pos = player->pos;
			}
			else if (i % 3 == 1) {
				if (player->vector == UP || player->vector == DOWN) {
					playerBullet[1].pos.X = (player->pos.X + 1);
					playerBullet[1].pos.Y = player->pos.Y;
				}
				else {
					playerBullet[1].pos.X = player->pos.X;
					playerBullet[1].pos.Y = (player->pos.Y + 1);
				}
			}
			else {
				if (player->vector == UP || player->vector == DOWN) {
					playerBullet[2].pos.X = (player->pos.X - 1);
					playerBullet[2].pos.Y = player->pos.Y;
				}
				else {
					playerBullet[2].pos.X = player->pos.X;
					playerBullet[2].pos.Y = (player->pos.Y - 1);
				}
			}
			playerBullet[i].vector = player->vector;
			playerBullet[i].currentDistance = 0;
			playerBullet[i].avaliable = 1;
		}
	}
}

void PlayerBeAttacked(void)
{
	if ((invinTime != 0) && (finish - invinTime) / CLOCKS_PER_SEC <= 1)
		return;
	else
	{
		invinTime = clock();
		player->hp--;
	}
}

int TherePlayer(int x, int y)
{
	if (player->pos.X == x && player->pos.Y == y) return 1;
	return 0;
}

void InputKeyProcess(void)
{
	int in_key;

	if (kbhit() && keyCnt < 1)
	{
		in_key = getch();

		switch (in_key) {
		case LEFT:
			MovePlayerToLeft();
			break;
		case RIGHT:
			MovePlayerToRight();
			break;
		case UP:
			MovePlayerToUp();
			break;
		case DOWN:
			MovePlayerToDown();
			break;
		case SPACEBAR:
			PlayerShoot();
			break;
		case PAUSE:
			PauseScreen();
			break;
		case ATTACK:
			Bomb();
			break;

			keyCnt++;
		}
		StaticPlayerDisplay();
		//DisplayAroundPlayer();
	}
}

void ReverseProcessKeyInput(void)
{
	int in_key;
	if (kbhit() && keyCnt < 1)
	{
		in_key = getch();

		switch (in_key) {
		case LEFT:
			MovePlayerToRight();
			break;
		case RIGHT:
			MovePlayerToLeft();
			break;
		case UP:
			MovePlayerToDown();
			break;
		case DOWN:
			MovePlayerToUp();
			break;
		case SPACEBAR:
			PlayerShoot();
			break;
		case PAUSE:
			PauseScreen();
			break;
		case ATTACK:
			Bomb();
			break;

			keyCnt++;
		}
		StaticPlayerDisplay();
		//DisplayAroundPlayer();
	}
}

void InitPlayerBullet(void)
{
	int i;
	Bullet standard;

	maxPlayerBullet = 20;

	standard.attacker = PLAYER;
	standard.attackPower = player->attackPower;
	standard.avaliable = 0;
	standard.currentDistance = 0;
	standard.frameSpeed = 250;
	standard.maxRange = player->maxBulletRange;
	standard.pos = player->pos;
	standard.vector = UP;

	playerBullet = (Bullet*)malloc(sizeof(Bullet) * maxPlayerBullet);
	for (i = 0; i < maxPlayerBullet; i++) {
		playerBullet[i] = standard;
	}
}
int isTherePlayerBullet(int x, int y)
{
	int i;
	for (i = 0; i < maxPlayerBullet; i++) {
		if (playerBullet[i].avaliable == 1) {
			if (playerBullet[i].pos.X == x) {
				if (playerBullet[i].pos.Y == y) {
					return i;
				}
			}
		}
	}
	return -1;
}
void PlayerBulletRoutine(void)
{
	int i, indX, indY, colMob;

	for (i = 0; i < maxPlayerBullet; i++) {
		if (playerBullet[i].avaliable == 1) {
			if (gameTimer % playerBullet[i].frameSpeed != 0) continue;

			// 최대 사정거리까지 이동시 삭제
			if (playerBullet[i].maxRange <= playerBullet[i].currentDistance) {
				playerBullet[i].avaliable = 0;
			}

			if (playerBullet[i].vector == UP) playerBullet[i].pos.Y -= 1;
			else if (playerBullet[i].vector == DOWN) playerBullet[i].pos.Y += 1;
			else if (playerBullet[i].vector == LEFT) playerBullet[i].pos.X -= 1;
			else if (playerBullet[i].vector == RIGHT) playerBullet[i].pos.X += 1;

			playerBullet[i].currentDistance++;

			// 몬스터와 충돌검사
			colMob = ThereIsMonster(playerBullet[i].pos.X, playerBullet[i].pos.Y, -1);
			if (colMob != -1) {
				MonsterBeShooted(colMob, playerBullet[i].attackPower);
				playerBullet[i].avaliable = 0;
			}

			// 보스와 충돌검사
			if (boss.charaInfo.avaliable == 1) {
				colMob = IsColliseToBoss(playerBullet[i].pos.X, playerBullet[i].pos.Y);
				if (colMob != 0) {
					BossBeAttacked(colMob);
					playerBullet[i].avaliable = 0;
				}
			}

			// 벽이랑 충돌시 삭제
			if (IsWall(playerBullet[i].pos.X, playerBullet[i].pos.Y)) {
				playerBullet[i].avaliable = 0;
			}
		}
	}
}

void InitMonster(void)
{
	int i;
	Monster standard;

	maxMonster = 20;

	standard.charaInfo.avaliable = 0;
	standard.state = 0;
	standard.stateTimer = 0;
	standard.stateLimit = 10;
	standard.attackTimer = 0;

	monsterList = (Monster*)malloc(sizeof(Monster) * maxMonster);
	for (i = 0; i < maxMonster; i++) {
		monsterList[i] = standard;
	}
}

void SetMonsterToMelee(int x, int y)
{
	int i;
	Monster standard;

	standard.charaInfo.avaliable = 1;
	standard.state = 0;
	standard.stateTimer = 0;

	standard.charaInfo.avaliable = 1;
	standard.charaInfo.attackPower = 1;
	standard.charaInfo.frameSpeed = 500;
	standard.charaInfo.hp = 1;
	standard.charaInfo.maxBulletRange = 2;
	standard.charaInfo.pos.X = x;
	standard.charaInfo.pos.Y = y;
	standard.charaInfo.vector = UP;
	standard.attackCooltime = 8;
	standard.attackTimer = 0;
	standard.attackRangeType = MEELE;
	standard.isDetectedPlayer = 0;
	standard.detectedAction = 0;		// 아직 유효하지 않음.
	standard.detectionRange = 5;
	standard.noneDectectedAction = 0;	// 아직 유효하지 않음.

	for (i = 0; i < maxMonster; i++) {
		if (monsterList[i].charaInfo.avaliable == 1) continue;
		else {
			monsterList[i] = standard;
			break;
		}
	}
}

void SetMonsterToStandoff(int x, int y)
{
	int i;
	Monster standard;

	standard.charaInfo.avaliable = 1;
	standard.state = 0;
	standard.stateTimer = 0;

	standard.charaInfo.avaliable = 1;
	standard.charaInfo.attackPower = 1;
	standard.charaInfo.frameSpeed = 500;
	standard.charaInfo.hp = 1;
	standard.charaInfo.maxBulletRange = 2;
	standard.charaInfo.pos.X = x;
	standard.charaInfo.pos.Y = y;
	standard.charaInfo.vector = UP;
	standard.attackCooltime = 8;
	standard.attackTimer = 0;
	standard.attackRangeType = STANDOFF;
	standard.isDetectedPlayer = 0;
	standard.detectedAction = 0;		// 아직 유효하지 않음.
	standard.detectionRange = 5;
	standard.noneDectectedAction = 0;	// 아직 유효하지 않음.

	for (i = 0; i < maxMonster; i++) {
		if (monsterList[i].charaInfo.avaliable == 1) continue;
		else {
			monsterList[i] = standard;
			break;
		}
	}
}

int MoveMonsterToLeft(int ind)
{
	int toX, y, i;

	toX = monsterList[ind].charaInfo.pos.X - 1;
	y = monsterList[ind].charaInfo.pos.Y;

	monsterList[ind].charaInfo.vector = LEFT;
	if (IsWall(toX, y)) return -1;
	if (IsGlass(toX, y)) return -1;
	if (ThereIsMonster(toX, y, ind) != -1) return -1;
	if (TherePlayer(toX, y)) {
		PlayerBeAttacked();
		return -1;
	}
	if (IsInBossBody(toX, y) != 0) return -1;
	i = isTherePlayerBullet(toX, y);
	if (i != -1) {
		MonsterBeShooted(ind, playerBullet[i].attackPower);
		playerBullet[i].avaliable = 0;
	}
	monsterList[ind].charaInfo.pos.X -= 1;
	return 1;
}

int MoveMonsterToRight(int ind)
{
	int toX, y, i;

	toX = monsterList[ind].charaInfo.pos.X + 1;
	y = monsterList[ind].charaInfo.pos.Y;

	monsterList[ind].charaInfo.vector = RIGHT;
	if (IsWall(toX, y)) return -1;
	if (IsGlass(toX, y)) return -1;
	if (ThereIsMonster(toX, y, ind) != -1) return -1;
	if (TherePlayer(toX, y)) {
		PlayerBeAttacked();
		return -1;
	}
	if (IsInBossBody(toX, y) != 0) return -1;
	i = isTherePlayerBullet(toX, y);
	if (i != -1) {
		MonsterBeShooted(ind, playerBullet[i].attackPower);
		playerBullet[i].avaliable = 0;
	}

	monsterList[ind].charaInfo.pos.X += 1;

	return 1;
}

int MoveMonsterToUp(int ind)
{
	int i;
	int x = monsterList[ind].charaInfo.pos.X;
	int toY = monsterList[ind].charaInfo.pos.Y - 1;

	monsterList[ind].charaInfo.vector = UP;
	if (IsWall(x, toY)) return -1;
	if (IsGlass(x, toY)) return -1;
	if (ThereIsMonster(x, toY, ind) != -1) return -1;
	if (TherePlayer(x, toY)) {
		PlayerBeAttacked();
		return -1;
	}
	if (IsInBossBody(x, toY) != 0) return -1;
	i = isTherePlayerBullet(x, toY);
	if (i != -1) {
		MonsterBeShooted(ind, playerBullet[i].attackPower);
		playerBullet[i].avaliable = 0;
	}

	monsterList[ind].charaInfo.pos.Y -= 1;
	return 1;
}

int MoveMonsterToDown(int ind)
{
	int x = monsterList[ind].charaInfo.pos.X;
	int toY = monsterList[ind].charaInfo.pos.Y + 1;
	int i;

	monsterList[ind].charaInfo.vector = DOWN;
	if (IsWall(x, toY)) return -1;
	if (IsGlass(x, toY)) return -1;
	if (ThereIsMonster(x, toY, ind) != -1) return -1;
	if (TherePlayer(x, toY)) {
		PlayerBeAttacked();
		return -1;
	}
	if (IsInBossBody(x, toY) != 0) return -1;
	i = isTherePlayerBullet(x, toY);
	if (i != -1) {
		MonsterBeShooted(ind, playerBullet[i].attackPower);
		playerBullet[i].avaliable = 0;
	}
	monsterList[ind].charaInfo.pos.Y += 1;

	return 1;
}

int IsCollisionToMonsterPosX(int x, int exceptionMonsterNum)
{
	int i;

	for (i = 0; i < maxMonster; i++) {
		if (monsterList[i].charaInfo.avaliable == 0) continue;
		if (exceptionMonsterNum == i) continue;
		if (monsterList[i].charaInfo.pos.X == x) return i;
	}
	return -1;
}

int IsCollisionToMonsterPosY(int y, int exceptionMonsterNum)
{
	int i;

	for (i = 0; i < maxMonster; i++) {
		if (monsterList[i].charaInfo.avaliable == 0) continue;
		if (exceptionMonsterNum == i) continue;
		if (monsterList[i].charaInfo.pos.Y == y) return i;
	}
	return -1;
}

int ThereIsMonster(int x, int y, int exceptionMonsterNum)
{
	int i;
	for (i = 0; i < maxMonster; i++) {
		if (monsterList[i].charaInfo.pos.X == x) {
			if (monsterList[i].charaInfo.pos.Y == y) {
				if (exceptionMonsterNum == i) continue;
				if (monsterList[i].charaInfo.avaliable == 0) continue;
				return i;
			}
		}
	}
	return -1;
}

void MonsterShoot(int ind, int arrow)
{
	int i;

	for (i = 0; i < maxMonsterBullet; i++) {
		if (monsterBullet[i].avaliable == 1) {
			continue;
		}
		monsterBullet[i].pos = monsterList[ind].charaInfo.pos;
		monsterBullet[i].vector = arrow;
		monsterBullet[i].currentDistance = 0;
		monsterBullet[i].maxRange = monsterList[ind].charaInfo.maxBulletRange;
		monsterBullet[i].avaliable = 1;
		break;
	}
}

void MonsterBeShooted(int monsterInd, int attackPower)
{
	monsterList[monsterInd].charaInfo.hp -= attackPower;
	if (monsterList[monsterInd].charaInfo.hp <= 0) {
		monsterList[monsterInd].state = ARRESTED;
		monsterList[monsterInd].stateTimer = clock();
	}
}

void checkArrTime(int monsterInd)
{
	if ((finish - monsterList[monsterInd].stateTimer) / CLOCKS_PER_SEC == 10)
		monsterList[monsterInd].state = 0;
}

void MonsterDead(int monsterInd)
{
	monsterList[monsterInd].charaInfo.avaliable = 0;
	score += 10;
	DropItemAroundDeadMonster(monsterInd);
}

void MonsterRoutine(void)
{
	int i, x, y, xf, yf, isActed, v, cnt;
	for (i = 0; i < maxMonster; i++) {
		SetCurrentCursorPos(1, 1);
		if (gameTimer % monsterList[i].charaInfo.frameSpeed != 0) continue;
		if (monsterList[i].charaInfo.avaliable == 0) continue;

		// 봉인된 상태라면, stateTimer 를 줄이고 행동 보류
		if (monsterList[i].state == ARRESTED) {
			//monsterList[i].stateTimer--;
			continue;
		}

		// 플레이어를 찾지 못했다면, 플레이어를 탐색하려 시도하고 행동 보류
		if (monsterList[i].isDetectedPlayer == 0) {
			// 탐색방법은 플레이어 좌표와 몬스터와의 좌표 차이를 dectectionRange와 비교한다.
			xf = player->pos.X - monsterList[i].charaInfo.pos.X;
			yf = player->pos.Y - monsterList[i].charaInfo.pos.Y;
			if (xf > 0) xf = -xf;
			if (yf > 0) yf = -yf;
			if (xf + yf <= monsterList[i].detectionRange) {
				monsterList[i].isDetectedPlayer = 1;
			}
			continue;
		}

		// 공격 쿨타임이 남아있다. (현재 공격하지 못한다.)
		if (monsterList[i].attackTimer > 0) {
			monsterList[i].attackTimer--;
			// x,y 플레이어와 몬스터의 거리.
			// 플레이어기준 x가 양수면 몬스터는 왼쪽에 있고 y가 양수면 몬스터가 위쪽에 있다.
			x = player->pos.X - monsterList[i].charaInfo.pos.X;
			y = player->pos.Y - monsterList[i].charaInfo.pos.Y;
			// xf, yf 거리의 절댓값
			xf = x;
			yf = y;
			if (xf < 0) xf = -xf;
			if (yf < 0) yf = -yf;

			// 플레이어로부터 도망. x거리와 y거리가 같다면 x축으로 도망.
			isActed = 0;
			if (xf <= yf) {
				if (y >= 0) isActed = MoveMonsterToUp(i);
				else isActed = MoveMonsterToDown(i);
			}
			else {
				if (x >= 0) isActed = MoveMonsterToLeft(i);
				else isActed = MoveMonsterToRight(i);
			}

			// 만약에 움직이지 못했다면 (벽 혹은 몬스터에 의해) 랜덤으로 다른방향으로 움직임.
			// 원래 랜덤으로 가면 안되는데......
			cnt = 0;
			while (isActed == -1) {
				v = rand() % 4;
				switch (v) {
				case 0:
					isActed = MoveMonsterToUp(i);
					break;
				case 1:
					isActed = MoveMonsterToDown(i);
					break;
				case 2:
					isActed = MoveMonsterToLeft(i);
					break;
				case 3:
					isActed = MoveMonsterToRight(i);
					break;

				}
				// 사방이 벽이라던가 사방이 몬스터여서 못움직이면 그냥 가만히있으렴
				cnt++;
				if (cnt > 10) break;
			}

			continue;
		}
		// 공격할 수 있으면서, 공격범위에 들어왔는지 확인.
		else {
			// x,y 플레이어와 몬스터의 거리.
			// 플레이어기준 x가 양수면 몬스터는 왼쪽에 있고 y가 양수면 몬스터가 위쪽에 있다.
			x = player->pos.X - monsterList[i].charaInfo.pos.X;
			y = player->pos.Y - monsterList[i].charaInfo.pos.Y;
			// xf, yf 거리의 절댓값
			xf = x;
			yf = y;
			if (xf < 0) xf = -xf;
			if (yf < 0) yf = -yf;

			isActed = IsOnMonsterAttackRangePlayer(i);

			if (isActed == 1) {
				monsterList[i].attackTimer = monsterList[i].attackCooltime;
				if (x > 0) MonsterShoot(i, RIGHT);
				else if (x < 0) MonsterShoot(i, LEFT);
				else if (y > 0) MonsterShoot(i, DOWN);
				else MonsterShoot(i, UP);
				continue;
			}

		}

		// 플레이어가 범위에 들어왔으면서, 공격범위와 겹치지 않고, 쿨타임도 아니라면 플레이어에게 접근
		x = player->pos.X - monsterList[i].charaInfo.pos.X;
		y = player->pos.Y - monsterList[i].charaInfo.pos.Y;
		// xf, yf 거리의 절댓값
		xf = x;
		yf = y;
		if (xf < 0) xf = -xf;
		if (yf < 0) yf = -yf;


		// 몬스터가 플레이어를 추적하려고 애는 씀
		// 수정이 필요함
		if (xf <= yf) {
			if (y >= 0) MonsterTryToChase(DOWN, i);
			else MonsterTryToChase(UP, i);

		}
		else {
			if (x >= 0) MonsterTryToChase(RIGHT, i);
			else  MonsterTryToChase(LEFT, i);
		}

	}
}
void MonsterTryToChase(int arrow, int monsterNumber)
{
	int ind;
	int isActed;

	if (arrow == UP) {
		isActed = MoveMonsterToUp(monsterNumber);
		if (isActed == -1) {
			isActed = MoveMonsterToLeft(monsterNumber);
			if (isActed == -1) {
				isActed = MoveMonsterToRight(monsterNumber);
				return;
			}
		}
	}

	if (arrow == LEFT) {
		isActed = MoveMonsterToLeft(monsterNumber);
		if (isActed == -1) {
			isActed = MoveMonsterToUp(monsterNumber);
			if (isActed == -1) {
				isActed = MoveMonsterToDown(monsterNumber);
				return;
			}
		}
	}

	if (arrow == DOWN) {
		isActed = MoveMonsterToDown(monsterNumber);
		if (isActed == -1) {
			isActed = MoveMonsterToRight(monsterNumber);
			if (isActed == -1) {
				isActed = MoveMonsterToLeft(monsterNumber);
				return;
			}
		}
	}

	if (arrow == RIGHT) {
		isActed = MoveMonsterToRight(monsterNumber);
		if (isActed == -1) {
			isActed = MoveMonsterToDown(monsterNumber);
			if (isActed == -1) {
				isActed = MoveMonsterToUp(monsterNumber);
				return;
			}
		}
	}

}
int IsOnMonsterAttackRangePlayer(int monsterNumber)
{
	int x, y, i, max;
	int mx, my;
	int wallBoolean;

	mx = monsterList[monsterNumber].charaInfo.pos.X;
	my = monsterList[monsterNumber].charaInfo.pos.Y;
	x = player->pos.X - mx;
	y = player->pos.Y - my;

	max = 0;
	if (monsterList[monsterNumber].attackRangeType == MEELE) max = 2;
	if (monsterList[monsterNumber].attackRangeType == STANDOFF) max = 5;

	//printf("              %d %d", x, y);
	if (x == 0) {
		// 플레이어가 아래
		if (y > 0) {
			for (i = 1; i <= max; i++) {
				if (IsWall(mx, my + i)) return 0;
				if (player->pos.Y == my + i) return 1;
			}
		}
		// 플레이어가 위
		else if (y < 0) {
			for (i = 1; i <= max; i++) {
				if (IsWall(mx, my - i)) return 0;
				if (player->pos.Y == my - i) return 1;
			}
		}
	}
	else if (y == 0) {
		// 플레이어가 오른쪽
		if (x > 0) {
			for (i = 1; i <= max; i++) {
				if (IsWall(mx + i, my)) return 0;
				if (player->pos.X == mx + i) return 1;
			}
		}
		// 플레이어가 왼쪽
		else if (x < 0) {
			for (i = 1; i <= max; i++) {
				if (IsWall(mx - i, my)) return 0;
				if (player->pos.X == mx - i) return 1;
			}
		}
	}

	return 0;


}

void InitMonsterBullet(void)
{
	int i;
	Bullet standard;

	maxMonsterBullet = 100;
	monsterBullet = (Bullet*)malloc(sizeof(Monster) * maxMonsterBullet);

	standard.attacker = MONSTER;
	standard.attackPower = 1;
	standard.avaliable = 0;
	standard.currentDistance = 0;
	standard.frameSpeed = 250;
	standard.maxRange = 1;
	standard.pos.X = 0;
	standard.pos.Y = 0;
	standard.vector = UP;

	for (i = 0; i < maxMonsterBullet; i++) {
		monsterBullet[i] = standard;
	}

}

void MonsterBulletRoutine(void)
{
	int i;

	for (i = 0; i < maxMonsterBullet; i++) {
		if (monsterBullet[i].avaliable == 1) {
			if (gameTimer % monsterBullet[i].frameSpeed != 0) continue;

			// 최대 사정거리까지 이동시 삭제
			if (monsterBullet[i].maxRange <= monsterBullet[i].currentDistance) {
				monsterBullet[i].avaliable = 0;
				continue;
			}

			if (player->keyState)
			{
				if (monsterBullet[i].vector == UP) monsterBullet[i].pos.Y -= 1;
				else if (monsterBullet[i].vector == DOWN) monsterBullet[i].pos.Y += 1;
				else if (monsterBullet[i].vector == LEFT) monsterBullet[i].pos.X -= 1;
				else if (monsterBullet[i].vector == RIGHT) monsterBullet[i].pos.X += 1;
			}

			else
			{
				if (monsterBullet[i].vector == UP) monsterBullet[i].pos.Y += 1;
				else if (monsterBullet[i].vector == DOWN) monsterBullet[i].pos.Y -= 1;
				else if (monsterBullet[i].vector == LEFT) monsterBullet[i].pos.X += 1;
				else if (monsterBullet[i].vector == RIGHT) monsterBullet[i].pos.X -= 1;
			}

			monsterBullet[i].currentDistance++;


			// 몬스터랑 충돌시 총알만 삭제
			if (ThereIsMonster(monsterBullet[i].pos.X, monsterBullet[i].pos.Y, -1) != -1) {
				monsterBullet[i].avaliable = 0;
				continue;
			}

			// 플레이어랑 충돌시 체력 깎고 삭제
			if (player->pos.X == monsterBullet[i].pos.X && player->pos.Y == monsterBullet[i].pos.Y) {
				PlayerBeAttacked();
				monsterBullet[i].avaliable = 0;
				continue;
			}

			// 벽이랑 충돌시 삭제
			if (IsWall(monsterBullet[i].pos.X, monsterBullet[i].pos.Y)) {
				monsterBullet[i].avaliable = 0;
				continue;
			}



		}
	}
}


// 보스관련
void InitBossInfo()
{
	boss.charaInfo.hp = 24;
	boss.charaInfo.attackPower = 1;
	boss.charaInfo.avaliable = 0;
	boss.charaInfo.frameSpeed = 500;
	boss.charaInfo.keyState = 0;
	boss.charaInfo.maxBulletRange = 0;
	boss.charaInfo.pos.X = 0;
	boss.charaInfo.pos.Y = 0;
	boss.charaInfo.vector = UP;

	// Sleep(50)기준
	boss.skillCooltime[0] = 0;
	boss.skillCooltime[1] = 240;
	boss.skillCooltime[2] = 120;
	boss.skillCooltime[3] = 10;
	boss.skillCooltime[4] = 10;

	boss.skillTimer[0] = 0;
	boss.skillTimer[1] = 0;
	boss.skillTimer[2] = 0;
	boss.skillTimer[3] = 0;
	boss.skillTimer[4] = 0;

	boss.isActing[0] = -1;
	boss.isActing[1] = -1;
	boss.isActing[2] = -1;
	boss.isActing[3] = -1;
	boss.isActing[4] = -1;

	boss.state = NULL;
	boss.stateTimer = 0;
	boss.stateLimit = 0;

	boss.R1 = 0;
	boss.R2 = 0;

}
void SetBoss(int x, int y)
{
	boss.charaInfo.avaliable = 1;
	boss.charaInfo.pos.X = x;
	boss.charaInfo.pos.Y = y;
}
int MoveBoss(int xp, int yp)
{
	int cantMove;
	int x, y, size, i;
	int xc, yc;

	size = 7;
	cantMove = 0;
	if (xp == 1) {
		boss.charaInfo.vector = RIGHT;
		x = boss.charaInfo.pos.X;
		y = boss.charaInfo.pos.Y;

		xc = x + size / 2 + 1;
		yc = y - size / 2;
		for (i = 0; i < size; i++) {
			if (IsWall(xc, y + i)) {
				return 0;
			}
		}
	}
	else if (xp == -1) {
		boss.charaInfo.vector = LEFT;
		x = boss.charaInfo.pos.X;
		y = boss.charaInfo.pos.Y;

		xc = x - size / 2 - 1;
		yc = y - size / 2;
		for (i = 0; i < size; i++) {
			if (IsWall(xc, y + i)) {
				return 0;
			}
		}


	}
	else if (yp == -1) {
		boss.charaInfo.vector = UP;
		x = boss.charaInfo.pos.X;
		y = boss.charaInfo.pos.Y;

		xc = x - size / 2;
		yc = y - size / 2 - 1;
		for (i = 0; i < size; i++) {
			if (IsWall(xc + i, yc)) {
				return 0;
			}
		}
	}
	else if (yp == 1) {
		boss.charaInfo.vector = DOWN;
		x = boss.charaInfo.pos.X;
		y = boss.charaInfo.pos.Y;

		xc = x - size / 2;
		yc = y + size / 2 + 1;
		for (i = 0; i < size; i++) {
			if (IsWall(xc + i, yc)) {
				return 0;
			}
		}

	}

	if (boss.charaInfo.pos.X + xp - 3 <= player->pos.X  &&  player->pos.X <= boss.charaInfo.pos.X + xp + 3) {
		if (boss.charaInfo.pos.Y + yp - 3 <= player->pos.Y  &&  player->pos.Y <= boss.charaInfo.pos.Y + yp + 3) {
			if (xp == 1) MovePlayerToRightIgnoringMonster();
			if (xp == -1) MovePlayerToLeftIgnoringMonster();
			if (yp == -1) MovePlayerToUpIgnoringMonster();
			if (yp == 1) MovePlayerToDownIgnoringMonster();
		}
	}

	boss.charaInfo.pos.X += xp;
	boss.charaInfo.pos.Y += yp;
	return 1;

}
int IsColliseToBoss(int x, int y)
{
	// 반환값은 0 혹은 2자리 정수이다.
	// 11 12 13 14
	// 21  보스 24
	// 31       34
	// 41 42 43 44
	// 이런형식으로 11~77까지 반환한다.
	// 0 이라면 접하지 않음.

	int bX, bY;
	int rX, rY;

	bX = boss.charaInfo.pos.X;
	bY = boss.charaInfo.pos.Y;

	rX = x - bX + 4;
	rY = y - bY + 4;

	if (1 <= rX && rX <= 7) {
		if (1 <= rY && rY <= 7) {
			return rY * 10 + rX;
		}
		return 0;
	}
	return 0;
}
void BossChasePlayer()
{
	int x, y;
	int absX, absY;

	x = player->pos.X - boss.charaInfo.pos.X;
	y = player->pos.Y - boss.charaInfo.pos.Y;

	absX = myAbs(x);
	absY = myAbs(y);

	// x축 거리가 더 멀다면
	if (absX >= absY) {
		if (0 <= x) MoveBoss(1, 0);
		else if (x < 0) MoveBoss(-1, 0);
	}
	else {
		if (0 <= y) MoveBoss(0, 1);
		else if (y < 0) MoveBoss(0, -1);
	}
}
void BossChasePlayerInPerpen(int arrow)
{
	// 인자인 arrow가 플레이어 방향
	int x, y;
	int absX, absY;

	x = player->pos.X - boss.charaInfo.pos.X;
	y = player->pos.Y - boss.charaInfo.pos.Y;

	absX = myAbs(x);
	absY = myAbs(y);

	// 수직선상 위치가 같으면 안 움직이기
	if (arrow == LEFT || arrow == RIGHT) {
		// 상하이동
		if (0 < y) MoveBoss(0, 1);
		else if (y < 0) MoveBoss(0, -1);
	}
	else {
		// 좌우이동
		if (0 < x) MoveBoss(1, 0);
		else if (x < 0) MoveBoss(-1, 0);
	}
}
void BossRoutine() {
	int i, j, skill, skilled[5], rv, save;

	for (i = 0; i < 1; i++) {
		if (boss.charaInfo.avaliable == 0) continue;
		if (boss.state == ARRESTED) {
			if (IsColliseToBoss(player->pos.X, player->pos.Y) != 0) {
				boss.charaInfo.avaliable = 0;
				PlaySound(TEXT("clear.wav"), NULL, SND_ASYNC | SND_LOOP);
				// ============================= 보스가 죽는 곳
			}
			continue;
		}
		//if (gameTimer % boss.charaInfo.frameSpeed != 0) break;

		// 스킬구간

		skilled[0] = BossSkill0(); // 스킬[0]: 상하좌우 중 플레이어 방향을 탐색한 후, 그에 수직으로 움직이며 자신의 몸에 무작위 총알을 날린다
		skilled[1] = BossSkill1(); // 스킬[1]: 플레이어 방향으로 돌진한다.
		skilled[2] = BossSkill2(); // 스킬[2]: 보스 주위 원형으로 총알을 소환한다.

								   // ==========

		ReduceBossSkillTimer();


		if (skilled[0] == 1
			|| skilled[1] == 1
			|| skilled[2] == 1)
		{
			break;	 // 스킬을 사용중이라면 더이상 행동하지 않는다.
		}

		if (gameTimer % boss.charaInfo.frameSpeed != 0) break;

		// 사용하고 있는 스킬이 없다면 새롭게 스킬을 사용하려한다.
		skill = rand() % 3;
		save = skill;
		while (boss.skillTimer[skill] >= 0) {
			skill++;
			if (skill >= 3) skill = 0;
			if (skill == save) break;
		}

		if (boss.skillTimer[skill] < 0) boss.isActing[skill] = 0;
		else {
			// 만약 스킬들이 모두 쿨타임이라면 플레이어나 쫓아다니자.
			BossChasePlayer();
		}
	}
}
int BossSkill0()
{
	int skill = 0, rv;
	if (boss.isActing[skill] != -1) {
		switch (boss.isActing[skill]) {
		case 0:
			// R1 <- 플레이어 쪽 방향
			boss.R1 = getArrowToPlayer(boss.charaInfo.pos);
			BossChasePlayerInPerpen(boss.R1);
			rv = rand() * 10 % 7 - 3;
			BossShoot(boss.R1, rv);
			break;
		case 5:
			BossChasePlayerInPerpen(boss.R1);
			rv = rand() * 10 % 7 - 3;
			BossShoot(boss.R1, rv);
			break;
		case 10:
			BossChasePlayerInPerpen(boss.R1);
			rv = rand() * 10 % 7 - 3;
			BossShoot(boss.R1, rv);
			break;
		case 15:
			BossChasePlayerInPerpen(boss.R1);
			rv = rand() * 10 % 7 - 3;
			BossShoot(boss.R1, rv);
			break;
		case 20:
			BossChasePlayerInPerpen(boss.R1);
			rv = rand() * 10 % 7 - 3;
			BossShoot(boss.R1, rv);
			break;
		default:

			break;
		}
		boss.isActing[skill]++;

		if (boss.isActing[skill] >= 80) BossSkillEnd(skill);

		return 1;
	}
	return 0;
}
int BossSkill1()
{
	int skill = 1;
	if (boss.isActing[skill] != -1) {
		if (boss.isActing[skill] == 0) {
			boss.R1 = getArrowToPlayer(boss.charaInfo.pos);
			boss.R2 = 1;
		}
		else if (boss.R2 == 1 && boss.isActing[skill] % 2) {
			boss.R2 = BossRushToArrow(boss.R1);
		}
		else if (boss.R2 != 1) {
			if (boss.R2 == 0) MoveBossBackArrow(boss.R1);
			boss.R2 += 2;
		}

		boss.isActing[skill]++;

		// 돌진 후 3초 스탑
		if (boss.R2 == 120) {
			BossSkillEnd(skill);
		}

		return 1;
	}
	return 0;
}
int BossSkill2()
{
	int skill = 2;
	if (boss.isActing[skill] != -1) {
		if (boss.isActing[skill] == 0) boss.R1 = 0;
		else if (boss.isActing[skill] <= 25 && boss.isActing[skill] % 5 == 0) {
			BossCircleShoot(boss.R1);
			boss.R1++;
		}
		boss.isActing[skill]++;

		if (boss.isActing[skill] > 85) BossSkillEnd(skill);

		return 1;
	}
	return 0;

}
int IsRemindedBossSkillTime()
{
	int i;
	for (i = 0; i < 5; i++) {
		if (boss.skillTimer[i] > 0) return 1;
	}
	return 0;
}
void ReduceBossSkillTimer()
{
	int i;
	for (i = 0; i < 5; i++) {
		if (boss.isActing[i] == -1) boss.skillTimer[i]--;
	}
}
int BossRushToArrow(int arrow)
{
	// 성공적으로 직진하면 return 1
	int flag;
	int col;
	int pX, pY;
	int bX, bY;
	int rX, rY;

	pX = player->pos.X;
	pY = player->pos.Y;
	bX = boss.charaInfo.pos.X;
	bY = boss.charaInfo.pos.Y;
	rX = bX - pX;
	rY = bY - pY;

	flag = 0;
	col = IsInBossBody(pX, pY);
	if (arrow == LEFT) {
		flag = MoveBoss(-1, 0);
		if (col != 0 && rY > 0) {
			MovePlayerToDownIgnoringMonster();
			MovePlayerToDownIgnoringMonster();
		}
		else if (col != 0 && rY <= 0) {
			MovePlayerToUpIgnoringMonster();
			MovePlayerToUpIgnoringMonster();
		}
	}
	else if (arrow == RIGHT) {
		flag = MoveBoss(1, 0);
		if (col != 0 && rY > 0) {
			MovePlayerToDownIgnoringMonster();
			MovePlayerToDownIgnoringMonster();
		}
		else if (col != 0 && rY <= 0) {
			MovePlayerToUpIgnoringMonster();
			MovePlayerToUpIgnoringMonster();
		}
	}
	else if (arrow == UP) {
		flag = MoveBoss(0, -1);
		if (col != 0 && rX > 0) {
			MovePlayerToRightIgnoringMonster();
			MovePlayerToRightIgnoringMonster();
		}
		else if (col != 0 && rX <= 0) {
			MovePlayerToLeftIgnoringMonster();
			MovePlayerToLeftIgnoringMonster();
		}
	}
	else if (arrow == DOWN) {
		flag = MoveBoss(0, 1);
		if (col != 0 && rX > 0) {
			MovePlayerToRightIgnoringMonster();
			MovePlayerToRightIgnoringMonster();
		}
		else if (col != 0 && rX <= 0) {
			MovePlayerToLeftIgnoringMonster();
			MovePlayerToLeftIgnoringMonster();
		}
	}

	if (col != 0) PlayerBeAttacked();

	if (flag) {
		return 1;
	}

	return 0;

}
void BossShoot(int arrow, int relativeLocation)
{
	// 보스가 총알을 쏩니다.
	// relativeLocation은 수직으로 총알이 움직이는 경로입니다.
	int i;
	int bossSize = 7;


	for (i = 0; i < maxMonsterBullet; i++) {
		if (monsterBullet[i].avaliable == 1) {
			continue;
		}
		monsterBullet[i].pos = boss.charaInfo.pos;
		monsterBullet[i].vector = arrow;
		monsterBullet[i].currentDistance = 0;
		monsterBullet[i].maxRange = 20;
		monsterBullet[i].avaliable = 1;

		if (arrow == UP) {
			monsterBullet[i].pos.Y += bossSize / 2;
			monsterBullet[i].pos.X += relativeLocation;
		}
		else if (arrow == DOWN) {
			monsterBullet[i].pos.Y -= bossSize / 2;
			monsterBullet[i].pos.X += relativeLocation;
		}
		else if (arrow == RIGHT) {
			monsterBullet[i].pos.X += bossSize / 2;
			monsterBullet[i].pos.Y += relativeLocation;
		}
		else if (arrow == LEFT) {
			monsterBullet[i].pos.X -= bossSize / 2;
			monsterBullet[i].pos.Y += relativeLocation;
		}

		break;
	}

}
void BossCircleShoot(int edgeRadius)
{
	int size = 7;
	int bossX, bossY;
	int x, y, xc, yc;
	int limit;

	bossX = boss.charaInfo.pos.X;
	bossY = boss.charaInfo.pos.Y;
	limit = size + edgeRadius * 2;

	x = bossX + size / 2 + edgeRadius;
	if (x >= 0) {
		for (yc = 0; yc < limit; yc++) {
			y = bossY - size / 2 - edgeRadius + yc;
			if (y < 0) break;
			MakeMonsterBullet(x, y, RIGHT, 1);
		}
	}

	x = bossX - size / 2 - edgeRadius;
	if (x >= 0) {
		for (yc = 0; yc < limit; yc++) {
			y = bossY - size / 2 - edgeRadius + yc;
			if (y < 0) break;
			MakeMonsterBullet(x, y, LEFT, 1);
		}
	}

	y = bossY - size / 2 - edgeRadius;
	if (y >= 0) {
		for (xc = 0; xc < limit; xc++) {
			x = bossX - size / 2 - edgeRadius + xc;
			if (x < 0) break;
			MakeMonsterBullet(x, y, UP, 1);
		}
	}

	y = bossY + size / 2 + edgeRadius;
	if (y >= 0) {
		for (xc = 0; xc < limit; xc++) {
			x = bossX - size / 2 - edgeRadius + xc;
			if (x < 0) break;
			MakeMonsterBullet(x, y, DOWN, 1);
		}
	}


}
void BossBeAttacked(int part)
{
	int y = part / 10 - 1;
	int x = part % 10 - 1;

	if (!(x == 0 || y == 0 || x == 6 || y == 6)) return;

	if (bossAppearance[y][x][0] != '1') {
		bossAppearance[y][x][0] = '1';
		bossAppearance[y][x][1] = ' ';
		bossAppearance[y][x][2] = '\0';
		boss.charaInfo.hp--;
		if (boss.charaInfo.hp % 6 == 0) {
			DropItemAroundBoss();
			DropMonsterAroundBoss();
		}
	}

	if (boss.charaInfo.hp <= 0) {
		boss.state = ARRESTED;
	}

}
void MakeMonsterBullet(int x, int y, int arrow, int range)
{
	int i;

	for (i = 0; i < maxMonsterBullet; i++) {
		if (monsterBullet[i].avaliable == 1) {
			continue;
		}
		monsterBullet[i].pos.X = x;
		monsterBullet[i].pos.Y = y;
		monsterBullet[i].vector = arrow;
		monsterBullet[i].currentDistance = 0;
		monsterBullet[i].maxRange = range;
		monsterBullet[i].avaliable = 1;
		break;
	}
}
void BossSkillEnd(int ind)
{
	boss.isActing[ind] = -1;
	boss.skillTimer[ind] = boss.skillCooltime[ind];
}
void MoveBossBackArrow(int arrow)
{
	// arrow의 반대방향으로 이동
	if (arrow == LEFT) {
		MoveBoss(1, 0);
	}
	else if (arrow == RIGHT) {
		MoveBoss(-1, 0);
	}
	else if (arrow == UP) {
		MoveBoss(0, 1);
	}
	else if (arrow == DOWN) {
		MoveBoss(0, -1);
	}

}
int IsInBossBody(int x, int y)
{
	if (boss.charaInfo.pos.X - 3 <= x  &&  x <= boss.charaInfo.pos.X + 3) {
		if (boss.charaInfo.pos.Y - 3 <= y  &&  y <= boss.charaInfo.pos.Y + 3) {
			return 1;
		}
	}
	return 0;
}
int myAbs(int n)
{
	if (n >= 0) return n;
	return -n;
}
int myNormalize(int n)
{
	return n / myAbs(n);
}
int getArrowToPlayer(COORD pos) {
	COORD pPos = player->pos;
	int x = pPos.X - pos.X;
	int y = pPos.Y - pos.Y;
	if (myAbs(x) >= myAbs(y)) {
		if (x > 0) return RIGHT;
		else return LEFT;
	}
	else {
		if (y > 0) return DOWN;
		else return UP;
	}
}

//-------------

int IsWall(int x, int y)
{
	if (y >= MAP_MAXSIZE_Y || y < 0) return 1;
	if (x >= MAP_MAXSIZE_X || x < 0) return 1;
	if (mapModel[curStage][y][x] == WALL) {
		return 1;
	}
	return 0;
}
int IsGlass(int x, int y)
{
	if (y >= MAP_MAXSIZE_Y || y < 0) return -1;
	if (x >= MAP_MAXSIZE_X || x < 0) return -1;
	if (mapModel[curStage][y][x] == GLASS) {
		return 1;
	}
	return 0;
}

/*
void CopyMapFromMapmodel(int ind)
{
int y, x;
for (y = 0; y < MAP_MAXSIZE_Y; y++) {
for (x = 0; x < MAP_MAXSIZE_X; x++) {
map[ind][y][x] = mapModel[ind][y][x];
}
}
}
*/

void DisplayAroundPlayer(void)
{
	int x, y;
	int x_i, y_i;
	int x_f, y_f;
	int halfVisibleRangeX;
	int halfVisibleRangeY;
	int i;
	COORD p;

	p = GetPlayerPos();

	halfVisibleRangeX = MAP_VISIBLE_RANGE_X / 2;
	halfVisibleRangeY = MAP_VISIBLE_RANGE_Y / 2;

	x_i = p.X - halfVisibleRangeX;
	y_i = p.Y - halfVisibleRangeY;

	x_f = p.X + MAP_VISIBLE_RANGE_X - halfVisibleRangeX;
	y_f = p.Y + MAP_VISIBLE_RANGE_Y - halfVisibleRangeY;

	if (x_i < 0) x_i = 0;
	if (y_i < 0) y_i = 0;
	if (x_f >= MAP_MAXSIZE_X) x_f = MAP_MAXSIZE_X;
	if (y_f >= MAP_MAXSIZE_Y) y_f = MAP_MAXSIZE_Y;

	// 맵 지형

	for (y = y_i; y < y_f; y++) {
		for (x = x_i; x < x_f; x++) {

			SetCurrentCursorPos(x * 2, y);
			FixDisplayFromStatus();
			if (mapModel[curStage][y][x] == WALL) {
				ChangePen(WHITE);
				printf("■");
			}
			if (mapModel[curStage][y][x] == EMPTY) {
				ChangePen(DEEPGRAY);
				printf("* ");
			}
			if (mapModel[curStage][y][x] == ITEM) {
				ChangePen(DEEPGRAY);
				printf("◈");
			}
		}
	}


	// 플레이어 총알
	ChangePen(SKYBLUE);
	for (i = 0; i < maxPlayerBullet; i++) {
		if (playerBullet[i].avaliable == 1) {
			SetCurrentCursorPos(playerBullet[i].pos.X * 2, playerBullet[i].pos.Y);
			FixDisplayFromStatus();
			if (mapModel[curStage][playerBullet[i].pos.Y][playerBullet[i].pos.X] != WALL)
				printf("○");
		}
	}

	// 몬스터 총알
	ChangePen(RED);
	for (i = 0; i < maxMonsterBullet; i++) {
		if (monsterBullet[i].avaliable == 1) {
			SetCurrentCursorPos(monsterBullet[i].pos.X * 2, monsterBullet[i].pos.Y);
			FixDisplayFromStatus();
			printf("○");
		}
	}

	// 몬스터
	ChangePen(RED);
	for (i = 0; i < maxMonster; i++) {
		if (monsterList[i].charaInfo.avaliable == 1) {
			if (monsterList[i].charaInfo.pos.X >= x_i && monsterList[i].charaInfo.pos.X < x_f && monsterList[i].charaInfo.pos.Y >= y_i && monsterList[i].charaInfo.pos.Y < y_f) {
				SetCurrentCursorPos(monsterList[i].charaInfo.pos.X * 2, monsterList[i].charaInfo.pos.Y);
				FixDisplayFromStatus();
				if (monsterList[i].state == ARRESTED) {
					ChangePen(SKYBLUE);
					printf("●");
					ChangePen(RED);
				}
				else printf("♠");
			}
		}
	}

	// 드롭된 아이템
	ChangePen(DEEPPURPULE);
	for (i = 0; i<maxDropedItem; i++) {
		if (dropedItemsList[i].avaliable == 1) {
			SetCurrentCursorPos(dropedItemsList[i].pos.X * 2, dropedItemsList[i].pos.Y);
			FixDisplayFromStatus();
			printf("◈");
		}
	}


	// 플레이어
	ChangePen(BLUE);
	SetCurrentCursorPos(p.X * 2, p.Y);
	FixDisplayFromStatus();
	if (player->vector == UP) printf("▲");
	else if (player->vector == RIGHT) printf("▶");
	else if (player->vector == LEFT) printf("◀");
	else if (player->vector == DOWN) printf("▼");

	// 펜 색깔 되돌리기
	ChangePen(WHITE);

	// 플레이어 근처 범위 지우기


	for (x = x_i; x < x_f; x++) {
		SetCurrentCursorPos(x * 2, y_i);
		FixDisplayFromStatus();
		printf("  ");
		SetCurrentCursorPos(x * 2, y_f);
		FixDisplayFromStatus();
		printf("  ");
	}

	for (y = y_i; y < y_f; y++) {
		SetCurrentCursorPos((x_i) * 2, y);
		FixDisplayFromStatus();
		printf("  ");
		SetCurrentCursorPos((x_f) * 2, y);
		FixDisplayFromStatus();
		printf("  ");
	}
}

void StaticPlayerDisplay()
{
	int playerSight, i, j;
	int xi, yi, xf, yf;
	int x, y, xc, yc, xb, yb;
	int screenMax_x = 70, screenMax_y = 50;

	int centerX = 19;
	int centerY = 12;

	// ----------- ---------- ---------- ----------
	// 이부분 매 프레임 계산하지 않고, 최적화가 가능할텐데
	playerSight = 5; // temporary constant
	playerSight += sightChange;

	xi = player->pos.X - playerSight;
	yi = player->pos.Y - playerSight;
	xf = player->pos.X + playerSight;
	yf = player->pos.Y + playerSight;

	if (sightFlag == 0)
	{
		SetCurrentCursorPos(0, 0);
		for (int y = 0; y<40; y++)
			for (int x = 0; x<20; x++)
				printf(" ");
		ShowPlayerStatus(player->hp, score, timeLimit, 3 - curStage, itemList[5]);
		sightFlag = 1;
	}

	// 맵
	yc = -playerSight;
	for (y = yi; y <= yf; y++) {
		xc = -playerSight;
		for (x = xi; x <= xf; x++) {
			SetCurrentCursorPos(xc * 2 + centerX, yc + centerY);

			if (y < 0 || x < 0 || mapModel[curStage][y][x] == EMPTY) {
				if (player->pos.X != x || player->pos.Y != y) {
					ChangePen(DEEPGRAY);
					printf("* ");
				}
			}
			else if (mapModel[curStage][y][x] == WALL) {
				ChangePen(WHITE);
				printf("■");
			}
			else if (mapModel[curStage][y][x] == ITEM) {
				ChangePen(DEEPGRAY);
				printf("◈");
			}
			else if (mapModel[curStage][y][x] == GLASS) {
				ChangePen(SKYBLUE);
				printf("□");
			}
			else if (mapModel[curStage][y][x] == ENTRANCE) {
				if (player->pos.X != x || player->pos.Y != y) {
					ChangePen(DEEPGRAY);
					printf("↑ ");
				}
			}
			else if (mapModel[curStage][y][x] == EXIT) {
				if (player->pos.X != x || player->pos.Y != y) {
					ChangePen(GREEN);
					printf("↑ ");
				}
			}

			xc++;
		}
		yc++;
	}

	// 드롭된 아이템
	ChangePen(DEEPPURPULE);
	for (i = 0; i<maxDropedItem; i++) {
		if (dropedItemsList[i].avaliable == 1) {
			xi = (dropedItemsList[i].pos.X - player->pos.X);
			yi = (dropedItemsList[i].pos.Y - player->pos.Y);
			x = xi * 2 + centerX;
			y = yi + centerY;

			if (xi > playerSight || -xi > playerSight || yi > playerSight || -yi > playerSight) {}
			else if (x < 0 || y < 0) {}
			else {
				SetCurrentCursorPos(x, y);
				printf("◈");
			}
		}
	}


	// 맵 아이템
	ChangePen(DEEPYELLOW);
	for (i = 0; i<maxItem; i++) {
		if (itemsList[i].avaliable == 1) {
			xi = (itemsList[i].pos.X - player->pos.X);
			yi = (itemsList[i].pos.Y - player->pos.Y);
			x = xi * 2 + centerX;
			y = yi + centerY;

			if (xi > playerSight || -xi > playerSight || yi > playerSight || -yi > playerSight) {}
			else if (x < 0 || y < 0) {}
			else {
				SetCurrentCursorPos(x, y);
				printf("◈");
			}
		}
	}

	// 플레이어 총알
	ChangePen(SKYBLUE);
	for (i = 0; i < maxPlayerBullet; i++) {
		if (playerBullet[i].avaliable == 1) {
			xi = (playerBullet[i].pos.X - player->pos.X);
			yi = (playerBullet[i].pos.Y - player->pos.Y);
			x = xi * 2 + centerX;
			y = yi + centerY;

			if (xi > playerSight || -xi > playerSight || yi > playerSight || -yi > playerSight) continue;
			if (x < 0 || y < 0) continue;

			SetCurrentCursorPos(x, y);
			if (mapModel[curStage][playerBullet[i].pos.Y][playerBullet[i].pos.X] != WALL)
				printf("○");
			else {
				playerBullet[i].avaliable = 0;
			}
		}
	}

	// 몬스터 총알
	ChangePen(RED);
	for (i = 0; i < maxMonsterBullet; i++) {
		if (monsterBullet[i].avaliable == 1) {

			xi = (monsterBullet[i].pos.X - player->pos.X);
			yi = (monsterBullet[i].pos.Y - player->pos.Y);
			x = xi * 2 + centerX;
			y = yi + centerY;

			if (xi > playerSight || -xi > playerSight || yi > playerSight || -yi > playerSight) continue;
			if (x < 0 || y < 0) continue;

			SetCurrentCursorPos(x, y);
			printf("○");
		}
	}

	// 보스몬스터
	for (i = 0; i < 1; i++) {
		if (boss.charaInfo.avaliable == 1) {

			xi = (boss.charaInfo.pos.X - player->pos.X);
			yi = (boss.charaInfo.pos.Y - player->pos.Y);
			x = xi * 2 + centerX;
			y = yi + centerY;

			//if (xi > playerSight || -xi > playerSight || yi > playerSight || -yi > playerSight) continue;
			//if (x < 0 || y < 0) continue;

			SetCurrentCursorPos(x, y);
			if (boss.state == ARRESTED) {
				ChangePen(SKYBLUE);
				for (yc = 0; yc < 7; yc++) {
					for (xc = 0; xc < 7; xc++) {
						xb = xi + (xc - 3);
						yb = yi + yc - 3;
						x = xb * 2 + centerX;
						y = yb + centerY;
						if (xb > playerSight || -xb > playerSight || yb > playerSight || -yb > playerSight) continue;
						if (x < 0 || y < 0) continue;
						SetCurrentCursorPos(x, y);
						printf("●");
					}
				}
			}
			else {

				for (yc = 0; yc < 7; yc++) {
					for (xc = 0; xc < 7; xc++) {
						xb = xi + (xc - 3);
						yb = yi + yc - 3;
						x = xb * 2 + centerX;
						y = yb + centerY;
						if (xb > playerSight || -xb > playerSight || yb > playerSight || -yb > playerSight) continue;
						if (x < 0 || y < 0) continue;
						SetCurrentCursorPos(x, y);
						if (bossAppearance[yc][xc][0] == '1') {
							ChangePen(SKYBLUE);
							printf("●");
						}
						else {
							ChangePen(RED);
							printf("%s", bossAppearance[yc][xc]);
						}
					}
				}
			}
		}
	}


	// 몬스터
	ChangePen(RED);
	for (i = 0; i < maxMonster; i++) {
		if (monsterList[i].charaInfo.avaliable == 1) {

			xi = (monsterList[i].charaInfo.pos.X - player->pos.X);
			yi = (monsterList[i].charaInfo.pos.Y - player->pos.Y);
			x = xi * 2 + centerX;
			y = yi + centerY;

			if (xi > playerSight || -xi > playerSight || yi > playerSight || -yi > playerSight) continue;
			if (x < 0 || y < 0) continue;

			SetCurrentCursorPos(x, y);
			if (monsterList[i].state == ARRESTED) {
				ChangePen(SKYBLUE);
				printf("●");
				ChangePen(RED);
			}
			else printf("♠");

		}
	}


	// 플레이어
	ChangePen(BLUE);
	SetCurrentCursorPos(centerX, centerY);
	//FixDisplayFromStatus();
	if (player->vector == UP) printf("▲");
	else if (player->vector == RIGHT) printf("▶");
	else if (player->vector == LEFT) printf("◀");
	else if (player->vector == DOWN) printf("▼");

	// 펜 색깔 되돌리기
	ChangePen(WHITE);


}

void SetItem(void)
{
	strcpy(itemList[0], "Bomb");	// 폭탄 (3*3 몬스터 다 죽임)
	strcpy(itemList[1], "Increase Attack Range");	// 공격 사거리 증가
	strcpy(itemList[2], "Increase Life");	// 체력 +1
	strcpy(itemList[3], "Increase View");	// 시야 1칸 증가 (영구)
	strcpy(itemList[4], "Reverse Key");	// 방향키 반대 (15초)
	strcpy(itemList[5], "Decrease View");	// 시야 1칸 감소
}

int ChoiceGoodItem()
{
	//srand((unsigned)time(NULL));

	if (curStage == 0)
		return (rand() % 3) + 1;
	else	// 2번맵 or 보스맵
		return rand() % 4;
}

int ChoiceAllItem()
{
	//srand((unsigned)time(NULL));

	if (curStage == 0)
		return (rand() % 5) + 1;
	else	// 2번맵 or 보스 맵
		return rand() % 6;
}

void ChangePlayerStatus(int Item_ind)
{
	strcpy(item, itemList[Item_ind]);

	switch (Item_ind)
	{
	case 0:
		bombcnt++;
		ShowPlayerStatus(player->hp, score, timeLimit, 3 - curStage, itemList[0]);
		// 게임 타이머 구현 후 5?10초 뒤 아이템 창 "없음"으로@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		break;
	case 1:
		IncreaseAttackRange();
		break;
	case 2:
		IncreaseLife();
		break;
	case 3:
		IncreaseView();
		break;
	case 4:
		ReverseKey();
		break;
	case 5:
		DecreaseView();
		break;
	}
}

void Bomb(void)
{
	if (bombcnt == 0)	return;
	int ind = 1;
	if (curStage == 2) ind = 2;

	PlaySound(TEXT("bomb.wav"), NULL, SND_ASYNC);
	bombcnt--;

	COORD pos = { player->pos.X, player->pos.Y };
	int centerX = 19;
	int centerY = 12;

	ChangePen(SKYBLUE);
	for (int y = pos.Y - ind, yc = -ind; y <= pos.Y + ind; y++, yc++)
		for (int x = pos.X - ind, xc = -ind; x <= pos.X + ind; x++, xc++)
		{
			SetCurrentCursorPos(xc * 2 + centerX, yc + centerY);

			if (mapModel[curStage][y][x] == EMPTY)
				printf("※");
		}
	Sleep(50);
	ChangePen(WHITE);

	for (int y = pos.Y - ind; y <= pos.Y + ind; y++)
		for (int x = pos.X - ind; x <= pos.X + ind; x++)
		{
			for (int i = 0; i <= maxMonster; i++)
			{
				if ((monsterList[i].charaInfo.pos.X == x) && (monsterList[i].charaInfo.pos.Y == y))
					monsterList[i].charaInfo.avaliable = 0;
			}

			if (curStage == 2)
			{
				int colMob = IsColliseToBoss(x, y);
				if (colMob != 0)
					BossBeAttacked(colMob);
			}
		}

	ShowPlayerStatus(player->hp, score, timeLimit, 3 - curStage, itemList[0]);
}

void IncreaseAttackRange(void)
{
	ShowPlayerStatus(player->hp, score, timeLimit, 3 - curStage, itemList[1]);
	player->maxBulletRange += 1;
	InitPlayerBullet();
	//ShowPlayerStatus(player->hp, score, timeLimit, 3 - curStage, "없음");
}

void IncreaseLife(void)
{
	ShowPlayerStatus(player->hp, score, timeLimit, 3 - curStage, itemList[2]);
	if (player->hp == 5)
		score += 50;

	else
		player->hp++;
	//ShowPlayerStatus(player->hp, score, timeLimit, 3 - curStage, "없음");
}

void IncreaseView(void)
{
	ShowPlayerStatus(player->hp, score, timeLimit, 3 - curStage, itemList[3]);
	if (sightChange >= 2)
		score += 50;
	else
		sightChange++;
	//ShowPlayerStatus(player->hp, score, timeLimit, 3 - curStage, "없음");
}

void ReverseKey(void)
{
	ShowPlayerStatus(player->hp, score, timeLimit, 3 - curStage, itemList[4]);
	player->keyState = 0;
	mid = clock();
	//  리버스키 끝나면 아이템 창 "없음"으로 바꿔야 함@@@@@@@@@@@@@@@@@@@@
}

int checkRevTime(clock_t mid)
{
	if ((double)(finish - mid) / CLOCKS_PER_SEC <= 15)
		return 0;
	else
		return 1;
}

void DecreaseView(void)
{
	ShowPlayerStatus(player->hp, score, timeLimit, 3 - curStage, itemList[5]);
	if (sightChange <= -2)
		score -= 50;
	else
	{
		sightChange--;
		sightFlag = 0;
	}
	//ShowPlayerStatus(player->hp, score, timeLimit, 3 - curStage, "없음");
}

int GameOver(void) {
	if ((player->hp < 1) || (timeLimit < 0))
		return 1;

	else
		return 0;
}

void printGameOver() {
	int x, y;
	COORD curPos;

	SetCurrentCursorPos(0, 0);
	for (y = 0; y < 40; y++)
		for (x = 0; x < 20; x++)
		{
			SetCurrentCursorPos((x * 2), y);
			printf("    ");
		}
		

	SetCurrentCursorPos(0, 4);
	curPos = GetCurrentCursorPos();
	ChangePen(RED);
	for (y = 0; y<11; y++) {
		for (x = 0; x<20; x++) {
			SetCurrentCursorPos(curPos.X + (x * 2), curPos.Y + y);
			if (endModel[0][y][x] == 1)
				printf("■");
			else
				printf("  ");
		}
	}
	ChangePen(WHITE);
	SetCurrentCursorPos(curPos.X + 8, curPos.Y + 12);
	printf("당신의 점수는 %d점 입니다.\n\n", score);
	PlaySound(TEXT("gameover.wav"), NULL, SND_ASYNC | SND_LOOP);
	Sleep(1300);
}

void MonsterRandomPlacement(void) {
	int x, y;
	int x_block_num = 0;
	int y_block_num = 0;
	int block = 10;
	int random_x;
	int random_y;

	switch (curStage) {
		//30*50
	case 0:
		x_block_num = 3;
		y_block_num = 2;
		block = 15;
		break;
		//50*70;
	case 1:
		x_block_num = 7;
		y_block_num = 5;
		block = 10;
		break;
	case 2:
		x_block_num = 3;
		y_block_num = 2;
		block = 12;
		break;

	}

	if (curStage > 2) return;

	for (y = 0; y<y_block_num; y++) {
		for (x = 0; x<x_block_num; x++) {
			random_x = rand() % block;
			random_y = rand() % block;

			while (1) {
				if (mapModel[curStage][y*block + random_y][x*block + random_x] == 0) {
					SetMonsterToMelee(x*block + random_x, y*block + random_y);
					break;
				}
				else {
					random_x = rand() % block;
					random_y = rand() % block;
				}
			}

		}
	}

}

void DropItemAroundDeadMonster(int monsterInd) {
	int random, x, y, i;
	COORD monsterPos = monsterList[monsterInd].charaInfo.pos;

	random = rand() % 8;

	switch (random) {
	case 0:
		x = -1;
		y = -1;
		break;
	case 1:
		x = 0;
		y = -1;
		break;
	case 2:
		x = 1;
		y = -1;
		break;
	case 3:
		x = -1;
		y = 0;
		break;
	case 4:
		x = 1;
		y = 0;
		break;
	case 5:
		x = -1;
		y = 1;
		break;
	case 6:
		x = 0;
		y = 1;
		break;
	case 7:
		x = 1;
		y = 1;
		break;
	}

	if (mapModel[curStage][monsterPos.Y + y][monsterPos.X + x] == 0) {
		for (i = 0; i < maxDropedItem; i++) {
			if (dropedItemsList[i].avaliable == 1) continue;
			else {
				dropedItemsList[i].avaliable = 1;
				dropedItemsList[i].pos.X = monsterPos.X + x;
				dropedItemsList[i].pos.Y = monsterPos.Y + y;
				dropedItemsList[i].type = ChoiceGoodItem();
				break;
			}
		}
	}
	else {
		DropItemAroundDeadMonster(monsterInd);
	}

}

void InitDropedItemsList(void) {
	int i;
	Item standard;

	maxDropedItem = 15;

	standard.avaliable = 0;
	standard.pos.X = 0;
	standard.pos.Y = 0;
	standard.type = -1;

	dropedItemsList = (Item *)malloc(sizeof(Item) * maxDropedItem);

	for (i = 0; i < maxDropedItem; i++) {
		dropedItemsList[i] = standard;
	}
}

void PauseScreen(void)
{
	for (int y = 0; y < 40; y++)
		for (int x = 0; x < 20; x++)
		{
			SetCurrentCursorPos((x * 2), y);
			printf("    ");
		}

	SetCurrentCursorPos(0, 0);
	for (int x = 0; x <= 20; x++)
		printf("                                     ");
	for (int y = 0; y <= 10; y++)
		for (int x = 0; x < 20; x++)
		{
			SetCurrentCursorPos((x * 2), y + 4);
			if (pauseModel[0][y][x] == 1)
				printf("■");
			else
				printf("  ");
		}
	SetCurrentCursorPos(2, 30);
	printf("다시 플레이하려면 p를 누르세요");

	while (1)
	{
		if (kbhit() != 0)
		{
			int in_key = getch();

			if (in_key == PAUSE)
			{
				for (int y = 0; y < 40; y++)
					for (int x = 0; x < 20; x++)
					{
						SetCurrentCursorPos((x * 2), y);
						printf("    ");
					}

				return;
			}
			else	Sleep(INT_MAX);
		}
	}
}

void InitItemsList(void) {
	int x, y, i;
	int random_x = 0;
	int random_y = 0;
	int block = 10;
	int x_block_num;
	int y_block_num;
	Item standard;

	switch (curStage) {
		//30*50
	case 0:
		x_block_num = 5;
		y_block_num = 3;
		maxItem = 15;
		break;
		//50*70;
	case 1:
		x_block_num = 7;
		y_block_num = 5;
		maxItem = 35;
		break;
		//30*70
	case 2:
		x_block_num = 4;
		y_block_num = 3;
		maxItem = 12;
		break;
	}

	if (curStage >= 3) return;

	itemsList = (Item *)malloc(sizeof(Item) * maxItem);

	i = 0;
	for (y = 0; y<y_block_num; y++) {
		for (x = 0; x<x_block_num; x++) {
			while (1) {
				random_x = rand() % block;
				random_y = rand() % block;

				if (mapModel[curStage][y*block + random_y][x*block + random_x] == 0) {
					standard.avaliable = 1;
					standard.pos.X = x*block + random_x;
					standard.pos.Y = y*block + random_y;
					standard.type = ChoiceAllItem();
					itemsList[i] = standard;
					i++;
					break;
				}
				else {
					random_x = rand() % block;
					random_y = rand() % block;
				}

			}
		}
	}

}

void freeAll(void) {
	free(monsterList);
	free(dropedItemsList);
	free(itemsList);
	free(monsterBullet);
}

void InitPlayerPos(void) {
	if (curStage == 0) {
		player->pos.X = 3;
		player->pos.Y = 29;
	}
	else if (curStage == 1) {
		player->pos.X = 35;
		player->pos.Y = 48;
	}
	else if (curStage == 2) {
		player->pos.X = 35;
		player->pos.Y = 28;
	}
}

int GameClear()
{
	if ((curStage == 3) && (mapModel[3][player->pos.Y][player->pos.X] == EXIT))
		return 1;

	else
		return 0;
}

void printGameClear()
{
	for (int y = 0; y < 40; y++)
		for (int x = 0; x < 20; x++)
		{
			SetCurrentCursorPos(x * 2, y);
			printf("    ");
		}

	SetCurrentCursorPos(0, 0);

	for (int y = 0; y <= 10; y++)
		for (int x = 0; x < 20; x++)
		{
			SetCurrentCursorPos(x * 2, y + 2);
			if (clearModel[0][y][x] == 1)
			{
				ChangePen(DEEPSKYBLUE);
				printf("■");
			}
			else if (clearModel[0][y][x] == 2)
			{
				ChangePen(YELLOW);
				printf("☆");
			}
			else if (clearModel[0][y][x] == 3)
			{
				ChangePen(YELLOW);
				printf("★");
			}
			else
				printf("  ");
		}
	printf("\n\n\n");
	ChangePen(WHITE);
	score += player->hp * 20;
	score += timeLimit;
	printf("당신의 점수는 %d점 입니다!!!\n\n\n", score);
	PlaySound(TEXT("clear.wav"), NULL, SND_ASYNC | SND_LOOP);
	Sleep(1300);
}

void DropItemAroundBoss() {
	int random_x, random_y, x, y, i, j, range = 16;
	COORD bossPos = boss.charaInfo.pos;
	int bossDropItemNum = 5;

	for (j = 0; j<bossDropItemNum; j++) {
		random_x = rand() % range;
		random_y = rand() % range;
		while (1) {
			if ((bossPos.X + random_x - range / 2) >= 38 || (bossPos.X + random_x - range / 2) <= 1) {
				random_x = rand() % range;
				continue;
			}
			if ((bossPos.Y + random_y - range / 2) >= 28 || (bossPos.Y + random_y - range / 2) <= 1) {
				random_y = rand() % range;
				continue;
			}
			break;
		}


		for (i = 0; i < maxDropedItem; i++) {
			if (dropedItemsList[i].avaliable == 1) continue;
			else {
				dropedItemsList[i].avaliable = 1;
				dropedItemsList[i].pos.X = bossPos.X + random_x - range / 2;
				dropedItemsList[i].pos.Y = bossPos.Y + random_y - range / 2;
				dropedItemsList[i].type = ChoiceGoodItem();
				break;
			}
		}
	}

}

void DropMonsterAroundBoss() {
	int random_x, random_y, x, y, i, j, range = 16;
	COORD bossPos = boss.charaInfo.pos;
	int bossDropMonsterNum = 5;

	for (j = 0; j<bossDropMonsterNum; j++) {
		random_x = rand() % range;
		random_y = rand() % range;
		while (1) {
			if ((bossPos.X + random_x - range / 2) >= 38 || (bossPos.X + random_x - range / 2) <= 1) {
				random_x = rand() % range;
				continue;
			}
			if ((bossPos.Y + random_y - range / 2) >= 28 || (bossPos.Y + random_y - range / 2) <= 1) {
				random_y = rand() % range;
				continue;
			}
			break;
		}


		for (i = 0; i < maxMonster; i++) {
			if (monsterList[i].charaInfo.avaliable == 1) continue;
			else {
				SetMonsterToMelee(bossPos.X + random_x - range / 2, bossPos.Y + random_y - range / 2);
				break;
			}
		}
	}

}
