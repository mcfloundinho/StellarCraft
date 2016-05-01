#define _TEST_
#undef _LOG_

#include "teamstyle17.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <iostream>

#define ABS(x) ((x) > 0 ? (x) : (-(x)))
#define SQR(x) ((x) * (x))

#define MAX_SIZE 100
#define WAIT while(GetTime() == operate_time);
#define GO (operate_time = GetTime());

typedef Position Vector;
enum AR_BORDER {
	NONE = 0,
	LEFT = 1 << 0,
	RIGHT = 1 << 1,
	BEHIND = 1 << 2,
	FRONT = 1 << 3,
	DOWN = 1 << 4,
	UP = 1 << 5,
};
enum value {
	ENERGY_VALUE = 5,
	HIGHLY_ADVANCED_VALUE = 1000,
	MID_ADVANCED_VALUE = 500,
	LOW_ADVANCED_VALUE = 50,
	TRASH = 0,
	RUN_VALUE = -10,
	CHASING_VALUE = 30,
};
const double eps = 1e-6;
static PlayerObject me;
static Object opponent;
static Object boss;
static int ever_update;
static int see_opponent;
static int see_boss;
static int AE_number;
static Position AE[MAX_SIZE];
static int devour_number;
static Position Devour[MAX_SIZE];
static int operate_time;
static int norm_update[] = {
	HEALTH_UP,//T1
	HEALTH_UP,//T2
	HEALTH_UP,//T3
	HEALTH_UP,//T4
	HEALTH_UP,//T5
	SHIELD,//T1
	SHIELD,//T2
	SHIELD,//T3
	SHIELD,//T4
	SHIELD,//T5
	SHORT_ATTACK,//T1
	SHORT_ATTACK,//T2
	SHORT_ATTACK,//T3
	SHORT_ATTACK,//T4
	SHORT_ATTACK,//T5
	DASH,//T1
	LONG_ATTACK,//T1
	LONG_ATTACK,//T2
	LONG_ATTACK,//T3
	LONG_ATTACK,//T4
	LONG_ATTACK,//T5
	DASH,//T2
	DASH,//T3
	DASH,//T4
	DASH,//T5
	VISION_UP,//T1
	VISION_UP,//T2
	VISION_UP,//T3
	VISION_UP,//T4
	VISION_UP,//T5
	kSkillTypes,//end
};
static int norm_counter=0;
static int norm_update_level[] = { 1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5,1, 1, 2, 3, 4, 5, 2, 3, 4, 5, 1, 2, 3, 4, 5, -1 };
static int attack_update[] = {
	LONG_ATTACK,//T1
	LONG_ATTACK,//T2
	LONG_ATTACK,//T3
	SHORT_ATTACK,//T1
	SHORT_ATTACK,//T2
	SHORT_ATTACK,//T3
	DASH,//T1
	LONG_ATTACK,//T4
	LONG_ATTACK,//T5
	SHORT_ATTACK,//T4
	SHORT_ATTACK,//T5
	DASH,//T2
	DASH,//T3
	DASH,//T4
	DASH,//T5
	kSkillTypes,//end
};
static int attack_counter=0;
static int attack_update_level[] = { 1, 2, 3, 1, 2, 3, 1, 4, 5, 4, 5, 2, 3, 4, 5, -1 };

const Map *map;

void init();
void init_opponent();
void strategy();
void anti_block();
int zw_IsDevour(double, Position, Position);
int zw_devour(double, Position);
int update(int*, int*, int*);
Vector MaximumSpeed(Vector vec);
int cost(int);
void long_attack(Object);
void short_attack(Object);
void shield();
void dash();
int IsBorder(double d, Position des);
int IsDevour(double d, Position des);
int IsBoss(double d, Position des);
int FBorder(double r);

double dist(Position, Position);
double length(Vector);
double dot_product(Vector, Vector);
double POW(double, int);
Vector Add(Vector a1, Vector a2);
Vector Minus(Vector a1, Vector a2);
Vector Multiple(double k, Vector a);
Vector Schmidt(Vector a1, Vector a2);

static int boss_warning;
static double boss_r = (double)2000;
static Vector speed;
static int AE_Parameter = HIGHLY_ADVANCED_VALUE;
static int anti_block_time;
static Position anti_block_pos;
static const Position center = { kMapSize >> 1, kMapSize >> 1, kMapSize >> 1 };

void AIMain() {
/*#ifdef _TEST_
	if (GetStatus()->team_id == 1) {
		int a = GetTime();
		Position speed = { rand() % 200,rand() % 200,rand() % 200 };
		Move(GetStatus()->objects[0].id, speed);
		while ((GetTime() - a)<20);
	}
#endif
#ifdef _LOG_
	std::ios::sync_with_stdio(false);
#endif*/
	srand(time(0));
	me = GetStatus()->objects[0];
	anti_block_pos = me.pos;
	WAIT;
	//std::cout << norm_update[norm_counter] << norm_counter << std::endl;
	UpgradeSkill(me.id, (SkillType)norm_update[norm_counter++]);
	GO;
	WAIT;
	//std::cout << norm_update[norm_counter] << norm_counter << std::endl;
	UpgradeSkill(me.id, (SkillType)norm_update[norm_counter++]);
	GO;
	WAIT;
	//std::cout << norm_update[norm_counter] << norm_counter << std::endl;
	UpgradeSkill(me.id, (SkillType)norm_update[norm_counter++]);
	GO;
	for (;;) {
		if (me.skill_level[SHIELD] > 2) shield();
		dash();
		init();
		//printf("norm_count=%d my health level=%d\n",norm_counter,me.skill_level[HEALTH_UP]);
		if ((GetTime() >> 5) != anti_block_time) {
			if (dist(anti_block_pos, me.pos) < 750) anti_block();
			anti_block_time = GetTime() >> 5;
			anti_block_pos = me.pos;
		}
		speed = MaximumSpeed(speed);
		Move(me.id, speed);
		strategy();
	}
}

void init(){
	map = GetMap();
	register int i,j;
	int AEflag; //记录有无AE可以走
	int flag;//记录AE旁边有无devour
	register int ChosenAE;
	register double MinAEdistance;
	int Devour_Danger = 0;
	int IsDevour_count=0;
	register double border_r;
	register double F;
	register double dis;
	Vector a2; //从目标到自己的向量

	border_r = 0.5 * me.radius;
	me = GetStatus() -> objects[0];
	see_opponent = devour_number = see_boss = AE_number = 0;
	speed.x = speed.y = speed.z = (double)0;
	MinAEdistance = 10000;

	for (i = 0; i < map -> objects_number; ++i){
		F = (double)0, dis = dist(map -> objects[i].pos, me.pos);
		switch (map -> objects[i].type){
			/*case PLAYER:
				if (map -> objects[i].team_id == GetStatus() -> team_id) break;
				opponent = map -> objects[i];
				see_opponent = 1;
				if ((me.radius > opponent.radius && me.skill_level[SHORT_ATTACK] > 2) || me.skill_level[SHORT_ATTACK] == kMaxSkillLevel) 
					F = CHASING_VALUE * me.radius * opponent.radius / POW(dis, 3);
				if (me.radius < opponent.radius * 0.9) 
					F = RUN_VALUE * me.radius * opponent.radius / POW(dis, 3);*/
				break;
			case ENERGY:
				F = ENERGY_VALUE * POW(me.radius, 2) / POW(dis, 3);
				if (IsBorder(border_r, map -> objects[i].pos)) F = 0; //如果到边界的距离小于r，力为0
				break;
			case ADVANCED_ENERGY:
				AE[AE_number++] = map -> objects[i].pos;
				break;
			case DEVOUR:
				Devour[devour_number++] = map -> objects[i].pos;
				break;
			case BOSS:
				boss = map -> objects[i];
				see_boss = 1;
				boss_r = boss.radius;
				boss_warning = (int)(dis < 2 * boss_r && me.radius < boss_r * 0.9);
				break;
			default:
				break;
		}
		a2 = Minus(map -> objects[i].pos, me.pos);
		speed = Add(speed, Multiple(F, a2));
	}
	if (AE_number){ //处理AE
		AEflag = 0;
		for (i = 0; i < AE_number; ++i)
		{
			dis = dist(AE[i], me.pos);
			if (IsBorder(border_r, AE[i])) //在边界旁边，扔掉
 				continue;
 			flag=0;
 			for(j=0;j<devour_number;j++)
 			{
 				if (dist(Devour[j],AE[i])<1500)
 				{
 					flag=1;
 					break;
 				}
 			}
 			if (flag)//如果AE旁边有devour，扔掉
 				continue;
			dis = dist(AE[i], me.pos);
			if (MinAEdistance>dis){
				MinAEdistance = dis;
				ChosenAE = i;
				AEflag = 1;
			}
		}
		if (AEflag){
			a2 = Minus(AE[ChosenAE], me.pos);
			F = AE_Parameter * POW(me.radius, 2) / POW(MinAEdistance, 3);
			speed = Add(speed, Multiple(F, a2));
		}
	}

	if (me.radius > 1.2 * boss_r && see_boss){ //如果比boss大，去吃
		a2 = Minus(boss.pos, me.pos);
		F = CHASING_VALUE * 10 * POW(me.radius, 2) / POW(length(a2), 3);
		speed = Add(speed, Multiple(F, a2));
	}

	Devour_Danger = !(me.shield_time > 15 && me.skill_level[SHIELD] == kMaxSkillLevel);
	if (Devour_Danger){
		for (i = 0; i < devour_number; ++i) //处理吞噬者
			if (IsDevour(1.1 * me.radius, Devour[i])){ //如果会碰到devour，速度正交化
				a2 = Minus(Devour[i], me.pos);
				speed = Schmidt(speed, a2);
				break;
			}
	}

	FBorder(1.1 * me.radius); //如果碰到边界，速度置0

	if (boss_warning && IsBoss(boss_r + 1.1*me.radius, boss.pos)){ //如果碰到boss，正交化
		a2 = Minus(boss.pos, me.pos);
		speed = Schmidt(speed, a2);
	}
	speed.x *= 100 + rand() % 5;
	speed.y *= 100 + rand() % 5;
	speed.z *= 100 + rand() % 5;
	if (map -> objects_number <= 1) speed = Minus(center, me.pos);
}
void init_opponent() {
	map = GetMap();
	see_opponent = 0;
	for (int i = 0; i < map->objects_number; ++i)
		if (map->objects[i].type == PLAYER)
			if (map->objects[i].team_id != GetStatus()->team_id) {
				opponent = map->objects[i];
				see_opponent = 1;
				break;
			}
}
void strategy() {
	if (ever_update) {
		update(norm_update, &norm_counter, norm_update_level);
	}
	else if (me.ability > 40) {
		ever_update = 1;
		update(norm_update, &norm_counter, norm_update_level);
	}
	if (see_boss) {
		if (boss.radius > me.radius * kEatableRatio) {
			dash();
			short_attack(boss);
			long_attack(boss);
		}
	}
	if (see_opponent) {
		if (!ever_update) {
			ever_update = 1;
			for (;;) {
			if (dist(opponent.pos, me.pos) < 3500) {
				if ((!see_boss || dot_product(Minus(boss.pos, me.pos), Minus(opponent.pos, me.pos)) < 0) && AE_number) {
					me = GetStatus()->objects[0];
					speed = Minus(opponent.pos, me.pos);
					speed = MaximumSpeed(speed);
					dash();
					Move(me.id, speed);
				}
				update(attack_update, &attack_counter, attack_update_level);
				short_attack(opponent);
				long_attack(opponent);
				init_opponent();
			}
			else{
				update(attack_update, &attack_counter, attack_update_level);
				init_opponent();
			}
			if (me.health < 0.5 * me.max_health) break;
			if (me.radius < opponent.radius * kEatableRatio + 100) break;
			if (!see_opponent) break;
			}
		}
		dash();
		short_attack(opponent);
		long_attack(opponent);
		shield();
	}
}
void anti_block(){
	#ifdef _LOG_
	std::cout << "anti_block proceeding" << std::endl;
	#endif
	Position a[6]={ {100, 0, 0},{-100, 0, 0},{0, 100, 0},{0, -100, 0},{0, 0, 100},{0, 0, -100}};
	if (length(me.speed) < 20) return;
	if (me.pos.x < kMapSize - 2 * me.radius){
		speed = a[0];
		if (zw_devour(1.1 * me.radius, speed) < 1) goto MOVE;
	}
	if (me.pos.x > 2 * me.radius){
		speed = a[1];
		if (zw_devour(1.1 * me.radius, speed) < 1) goto MOVE;
	}
	if (me.pos.y < kMapSize - 2 * me.radius){
		speed = a[2];
		if (zw_devour(1.1 * me.radius, speed) < 1) goto MOVE;
	}
	if (me.pos.y > 2 * me.radius){
		speed = a[3];
		if (zw_devour(1.1 * me.radius, speed) < 1) goto MOVE;
	}
	if (me.pos.z < kMapSize - 2 * me.radius){
		speed = a[4];
		if (zw_devour(1.1 * me.radius, speed) < 1) goto MOVE;
	}
	if (me.pos.z > 2 * me.radius){
		speed = a[5];
		if (zw_devour(1.1 * me.radius, speed) < 1) goto MOVE;
	}
	
	if (me.pos.x < kMapSize - 2 * me.radius){
		speed = a[0];
		if (zw_devour(1.1 * me.radius, speed) < 2) goto MOVE;
	}
	if (me.pos.x > 2 * me.radius){
		speed = a[1];
		if (zw_devour(1.1 * me.radius, speed) < 2) goto MOVE;
	}
	if (me.pos.y < kMapSize - 2 * me.radius){
		speed = a[2];
		if (zw_devour(1.1 * me.radius, speed) < 2) goto MOVE;
	}
	if (me.pos.y > 2 * me.radius){
		speed = a[3];
		if (zw_devour(1.1 * me.radius, speed) < 2) goto MOVE;
	}
	if (me.pos.z < kMapSize - 2 * me.radius){
		speed = a[4];
		if (zw_devour(1.1 * me.radius, speed) < 2) goto MOVE;
	}
	if (me.pos.z > 2 * me.radius){
		speed = a[5];
		if (zw_devour(1.1 * me.radius, speed) < 2) goto MOVE;
	}

	MOVE:
		speed = MaximumSpeed(speed);
		Move(me.id, speed);
		int t = GetTime();
		while ((GetTime() - t) < 50) strategy();
}
int zw_IsDevour(double d, Position des, Position speed) {
	int flag = 0;
	Position Next;
	for (int i = 1; i <= 60; ++i) {
		Next = Add(me.pos, Multiple(i, MaximumSpeed(speed)));
		if (dist(Next, des) < d) flag = 1;
	}
	return flag;
}
int zw_devour(double d, Position speed) {
	int res = 0;
	int i = devour_number - 1;
	for (; ~i; --i) {
		if (zw_IsDevour(d, Devour[i], speed)) ++res;
	}
	return res;
}
int update(int* list, int* counter, int* level) {
	me = GetStatus()->objects[0];
	if (list[*counter] == kSkillTypes) return -1;
	if (level[*counter] <= me.skill_level[list[*counter]]) {
		++*counter;
		return 1;
	}
	if (me.ability >= cost(list[*counter])) {
		WAIT;
		UpgradeSkill(me.id, (SkillType)list[*counter]);
		GO;
		++*counter;
		return 1;
	}
	else return 0;
}
int IsDevour(double d, Position des) { //判断下一时刻会不会碰到吞噬者
	int flag = 0;
	Position Next;
	for (int i = 1; i <= 8; ++i) {
		Next = Add(me.pos, Multiple(i, MaximumSpeed(speed)));
		if (dist(Next, des) < d) flag = 1;
	}
	return flag;
}
int IsBoss(double d, Position des) { //判断下一时刻会不会碰到boss
	int flag = 0;
	Position Next = Add(me.pos, Multiple(5, MaximumSpeed(speed)));
	if (dist(Next, des) < d) flag = 1;
	return flag;
}
int IsBorder(double d, Position des) { //判断物体是否在边界旁
	int R = NONE;
	if (des.x < d) R |= LEFT;
	else if (kMapSize - des.x < d) R |= RIGHT;
	if (des.y < d) R |= BEHIND;
	else if (kMapSize - des.y < d) R |= FRONT;
	if (des.z < d) R |= DOWN;
	else if (kMapSize - des.z < d) R |= UP;
	return R;
}
int FBorder(double r) { //判断下一时刻会不会碰到边界，如果会，速度置0
	register int flag = NONE, count = 0;
	if (me.pos.x < r && speed.x < 0) {
		speed.x = 0;
		flag = LEFT;
		++count;
	}
	if (me.pos.x > kMapSize - r && speed.x > 0) {
		speed.x = 0;
		flag = RIGHT;
		++count;
	}
	if (me.pos.y < r && speed.y < 0) {
		speed.y = 0;
		flag = BEHIND;
		++count;
	}
	if (me.pos.y > kMapSize - r && speed.y > 0) {
		speed.y = 0;
		flag = FRONT;
		++count;
	}
	if (me.pos.z < r && speed.z < 0) {
		speed.z = 0;
		flag = DOWN;
		++count;
	}
	if (me.pos.z > kMapSize - r && speed.z > 0) {
		speed.z = 0;
		flag = UP;
		++count;
	}
	if (count >= 2) {
		Vector a2 = Minus(boss.pos, me.pos);
		speed = Add(speed, Multiple(0.0005, a2));
	}
	return flag;
}
Vector MaximumSpeed(Vector vec) {
	register double len = length(vec);
	vec.x *= (kMaxMoveSpeed + kDashSpeed[me.skill_level[DASH]]) / len;
	vec.y *= (kMaxMoveSpeed + kDashSpeed[me.skill_level[DASH]]) / len;
	vec.z *= (kMaxMoveSpeed + kDashSpeed[me.skill_level[DASH]]) / len;
	return vec;
}
int cost(int skill) {
	me = GetStatus()->objects[0];
	if (me.skill_level[skill]) return kBasicSkillPrice[skill] << me.skill_level[skill];
	register int i, cnt = 0;
	for (i = 0; i < kSkillTypes; ++i)
		if (me.skill_level[i]) ++cnt;
	return kBasicSkillPrice[skill] << cnt;
}

void long_attack(Object target) {
	me = GetStatus()->objects[0];
	if (!me.skill_level[LONG_ATTACK]) return;
	if (me.skill_cd[LONG_ATTACK]) return;
	if (me.short_attack_casting != -1) return;
	if (me.long_attack_casting != -1) return;
	if (dist(me.pos, target.pos) - me.radius - target.radius > kLongAttackRange[me.skill_level[LONG_ATTACK]]) return;
	WAIT;
	LongAttack(me.id, target.id);
	GO;
}

void short_attack(Object target) {
	me = GetStatus()->objects[0];
	if (!me.skill_level[SHORT_ATTACK]) return;
	if (me.skill_cd[SHORT_ATTACK]) return;
	if (me.short_attack_casting != -1) return;
	if (me.long_attack_casting != -1) return;
	if (dist(me.pos, target.pos) - me.radius - target.radius > kLongAttackRange[me.skill_level[SHORT_ATTACK]]) return;
	WAIT;
	ShortAttack(me.id);
	GO;
}

void shield() {
	me = GetStatus()->objects[0];
	if (!me.skill_level[SHIELD]) return;
	if (me.skill_cd[SHIELD]) return;
	if (me.short_attack_casting != -1) return;
	if (me.long_attack_casting != -1) return;
	WAIT;
	Shield(me.id);
	GO;
}

void dash() {
	me = GetStatus()->objects[0];
	if (!me.skill_level[DASH]) return;
	if (me.skill_cd[DASH]) return;
	if (me.short_attack_casting != -1) return;
	if (me.long_attack_casting != -1) return;
	WAIT;
	Dash(me.id);
	GO;
}

double dist(Position src, Position des) {
	register Position vec;
	vec.x = des.x - src.x;
	vec.y = des.y - src.y;
	vec.z = des.z - src.z;
	return length(vec);
}

double length(Vector vec) {
	return sqrt(dot_product(vec, vec));
}

double dot_product(Vector vec1, Vector vec2) {
	return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z;
}

double POW(double x, int a) {
	if (ABS(x) < eps) x = 0.1;
	register double res = (double)1;
	for (; a; a >>= 1, x *= x)
		if (a & 1) res *= x;
	return res;
}
Vector Add(Vector a1, Vector a2) {
	Vector temp;
	temp.x = a1.x + a2.x;
	temp.y = a1.y + a2.y;
	temp.z = a1.z + a2.z;
	return temp;
}
Vector Minus(Vector a1, Vector a2) { //a1-a2
	Vector temp;
	temp.x = a1.x - a2.x;
	temp.y = a1.y - a2.y;
	temp.z = a1.z - a2.z;
	return temp;
}
Vector Multiple(double k, Vector a) {
	Vector temp;
	temp.x = k * a.x;
	temp.y = k * a.y;
	temp.z = k * a.z;
	return temp;
}
Vector Schmidt(Vector a1, Vector a2) { //由a1生成一个与a2垂直的向量
	Vector temp1, temp2;
	temp2 = Multiple(dot_product(a1, a2) / dot_product(a2, a2), a2);
	temp1 = Minus(a1, temp2);
	return temp1;
}
