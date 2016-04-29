#include "teamstyle17.h"
#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<time.h>
#include<iostream>
#define LOG_Z
#define LOG_Y
#define TEST
#define MAX_SIZE 100
#define WAIT  while(GetTime()==operate_time);
#define GO   (operate_time=GetTime());
#define NUM_OF_SOLUTION  3
//enums
enum situation {
	NONE = 0,
	OPPONENT = 1 << 0,
	SEE_BOSS = 1 << 1,
};
struct point {
	int weight;
	Position pos;
};
enum value {
	//ZW about food& aim
	//the value of advanced energy is 10000~100000 with the setting of HIGHLY_ADVANCED_VALUE
	//the value of enshaw is 500~2000
	//the value of food is lower than 1e6
	ENERGY_VALUE = 10,
	HIGHLY_ADVANCED_VALUE = 100000000,
	MID_ADVANCED_VALUE = 50000000,
	LOW_ADVANCED_VALUE = 10000000,
	TRASH = 0,
	/**************/
	EAT_BOSS_WEIGHT = 200000,
	BOSS_EAT_WEIGHT = 50000000,
	/**************/
	RUN_AWAY_WEIGHT = 1000000,
	OPPONENT_EATEN_WEIGHT = 100000000,
};
//flag
int emergency;
int code;
int ad_weight = HIGHLY_ADVANCED_VALUE;
//count
int num_of_aim;
int num_of_advance;
int num_of_food;
int num_of_devour;
//objects
PlayerObject me;
Object boss_obj;
Object opponent_obj;
//vector
point aim[MAX_SIZE];
Position advanced[MAX_SIZE];
Position food[MAX_SIZE];
Position devour[MAX_SIZE];
point solution[NUM_OF_SOLUTION];
//auxiliary variables
Position go_for;
int anti_block_time;
Position anti_block_position;
const int en_weight = 20;
int operate_time;
const Position a_variable_for_YQY[8] = {
	{0, 0, 0},
	{0, 0, kMapSize},
	{0, kMapSize, 0},
	{0, kMapSize, kMapSize},
	{kMapSize, 0, 0},
	{kMapSize, 0, kMapSize},
	{kMapSize, kMapSize, 0},
	{kMapSize, kMapSize, kMapSize} 
};
const Position another_variable_for_YQY[6] = {
	{1000,0,0},
	{-1000,0,0},
	{0,1000,0},
	{0,-1000,0},
	{0,0,1000},
	{0,0,-1000} 
};
//core function
int initial();
void greedy();
int update();
void avoid();
int opponent();
int boss();
void anti_lock();
void move();
//auxiliary functions
double get_health(double);
int zw_cost(int skill);
int zw_cmp(const void*, const void*);
int long_attack(const Object& target);
int short_attack(const Object& target);
int dash();
void zw_enshaw();
int IsDevour(double, Position, Position);
int zw_IsDevour(double, Position, Position);
Position MaximumSpeed(Position);
Position Schmidt(Position, Position);
int zw_devour(double, Position);
//low level function
Position add(Position a, Position b);//a+b
Position minus(Position a, Position b);//a-b
Position multiple(double k, Position a);//ka
Position cross_product(Position a, Position b);//a*b
double dot_product(Position a, Position b);//ab
double length(Position a);//
Position norm(Position a);//
double distance(Position a, Position b);//|AB|
void show(Position a);//
void AIMain() {
#ifdef TEST
	if (GetStatus()->team_id == 1)return;
#endif
	srand(time(0));
	anti_block_position = GetStatus()->objects[0].pos;
	for (;;) {
		code = initial();
		if ((GetTime() >> 6) != anti_block_time) {
			if (distance(anti_block_position, GetStatus()->objects[0].pos)<1000) {
				goto ANTI_LOCK;
			}
			anti_block_time = (GetTime() >> 6);
			anti_block_position = GetStatus()->objects[0].pos;
		}
		if (distance(GetStatus()->objects[0].pos, me.pos) > 2000) goto AVOID;
	START:
		update();
		if (distance(GetStatus()->objects[0].pos, me.pos) > 2000) goto AVOID;
		if (code&OPPONENT) {
			opponent();
		}
		if (distance(GetStatus()->objects[0].pos, me.pos) > 2000) goto AVOID;
		if (code&SEE_BOSS) {
			boss();
		}
		if (distance(GetStatus()->objects[0].pos, me.pos) > 2000) goto AVOID;
		if (!emergency) {
			greedy();
		}
		avoid();
		if (false) {
		AVOID:
#ifdef LOG_Z
			std::cout << "A_SUDDEN_MOVE_HAPPENED" << std::endl;
#endif
			initial();
			avoid();
			move();
			goto START;
		}
		move();
		if (false) {
		ANTI_LOCK:
#ifdef LOG_Z
			std::cout << "ANTI_LOCK_PROCEEDING" << std::endl;
#endif
			if (length(me.speed) < 20) goto START;
			anti_lock();
		}
	}
}
//C function bodies
Position add(Position a, Position b)
{
	Position c;
	c.x = a.x + b.x;
	c.y = a.y + b.y;
	c.z = a.z + b.z;
	return c;
}
Position minus(Position a, Position b)
{
	Position c;
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	c.z = a.z - b.z;
	return c;
}
Position multiple(double k, Position a)
{
	Position c;
	c.x = k*a.x;
	c.y = k*a.y;
	c.z = k*a.z;
	return c;
}
Position cross_product(Position a, Position b)
{
	Position c;
	c.x = a.y*b.z - a.z*b.y;
	c.y = -a.x*b.z + a.z*b.x;
	c.z = a.x*b.y - a.y*b.x;
	return c;
}
double dot_product(Position a, Position b)
{
	return (a.x*b.x + a.y*b.y + a.z*b.z);
}
double length(Position a)
{
	return sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
}
Position norm(Position a)
{
	double l = 1 / length(a);
	return multiple(l, a);
}
double distance(Position a, Position b)
{
	return length(minus(a, b));
}
void show(Position a)
{
	printf("The position is (%f,%f,%f).\n", a.x, a.y, a.z);
}
//Z function bodies
int zw_cost(int skill) {
	if (me.skill_level[skill]) {
		return kBasicSkillPrice[skill] << me.skill_level[skill];
	}
	else {
		int count = 0;
		for (int i = 0; i < kSkillTypes; i++) {
			if (me.skill_level[i]) count++;
		}
		return kBasicSkillPrice[skill] << count;
	}
}//checked
int update() {
	if ((~me.skill_cd[SHIELD])/*&& (!~me.short_attack_casting) */ && (!~me.long_attack_casting)) {
		WAIT;
		Shield(me.id);
		GO;
		return 1;
	}
	else {
		if (me.skill_level[HEALTH_UP] < kMaxSkillLevel) {
			if (me.ability >= zw_cost(HEALTH_UP)) {
				WAIT;
				UpgradeSkill(me.id, HEALTH_UP);
				GO;
				return 1;
			}
			else return 0;
		}//Update health
		else if (me.skill_level[SHIELD] < kMaxSkillLevel) {
			if (me.ability >= zw_cost(SHIELD)) {
				WAIT;
				UpgradeSkill(me.id, SHIELD);
				GO;
				return 1;
			}
			else return 0;
		}//update shield
		else {
			if (me.skill_level[DASH] > 0) {
				ad_weight = MID_ADVANCED_VALUE;//1st step of update finish
			}
			if (me.skill_level[SHORT_ATTACK] < kMaxSkillLevel) {
				if (me.skill_level[DASH] < kMaxSkillLevel) {
					if (me.ability >= zw_cost(SHORT_ATTACK)) {
						WAIT;
						UpgradeSkill(me.id, SHORT_ATTACK);
						GO;
						return 1;
					}
					else if (me.ability >= zw_cost(DASH)) {
						WAIT;
						UpgradeSkill(me.id, DASH);
						GO;
						return 1;
					}
					else return 0;
				}
				else {
					if (me.ability >= zw_cost(SHORT_ATTACK)) {
						WAIT;
						UpgradeSkill(me.id, SHORT_ATTACK);
						GO;
						return 1;
					}
					else return 0;
				}
			}
			else if (me.skill_level[DASH] < kMaxSkillLevel) {
				if (me.ability >= zw_cost(DASH)) {
					WAIT;
					UpgradeSkill(me.id, DASH);
					GO;
					return 1;
				}
				else return 0;
			}
			else {
				if (me.skill_level[LONG_ATTACK]>0) {
					ad_weight = LOW_ADVANCED_VALUE;//2st step of update finish
				}
				if (me.skill_level[LONG_ATTACK] < kMaxSkillLevel) {
					if (me.ability >= zw_cost(LONG_ATTACK)) {
						WAIT;
						UpgradeSkill(me.id, LONG_ATTACK);
						GO;
						return 1;
					}
					else return 0;
					
					}
				else {
					ad_weight = TRASH;
					if (me.skill_level[VISION_UP] < kMaxSkillLevel) {
						if (me.ability >= zw_cost(VISION_UP)) {
							WAIT;
							UpgradeSkill(me.id, VISION_UP);
							GO;
							return 1;
						}
						else return 0;
				}
				}
			}
		}
	}
	return 0;
}//checked
void greedy() {
	zw_enshaw();
	for (int temp = num_of_advance - 1; ~temp; --temp) {
		aim[num_of_aim].pos = advanced[temp];
		aim[num_of_aim].weight = ad_weight;
		++num_of_aim;
	}
	for (int temp = num_of_aim - 1; temp>0; --temp) {
		aim[temp].weight /= distance(aim[temp].pos, me.pos);
	}
#ifdef LOG_Z
	std::cout << "++" << std::endl;
	for (int temp = num_of_aim - 1; ~temp; --temp) {
		std::cout << aim[temp].weight << std::endl;
	}
	std::cout << "++" << std::endl;
#endif
	qsort(aim, num_of_aim, sizeof(point), zw_cmp);
#ifdef LOG_Z
	std::cout << "--" << std::endl;
	for (int temp = num_of_aim - 1; ~temp; --temp) {
		std::cout << aim[temp].weight << std::endl;
	}
	std::cout << "--" << std::endl;
#endif
}
int zw_cmp(const void* p, const void* q) {
	int w1 = (((point*)p)->weight);
	int w2 = (((point*)q)->weight);
	return (w2 - w1) ;
}//waiting
void zw_enshaw() {
	num_of_aim = 1;
	Position force = { 0.0,0.0,0.0 };
	int n = num_of_food - 1;
	for (; n >= 0; n--) {
		Position point_to = minus(food[n], me.pos);
		double k = me.radius / length(point_to);
		force = add(force, multiple(k*k*ENERGY_VALUE, point_to));
	}
	aim[0].weight = (int)(length(force));
	aim[0].pos = add(me.pos, multiple(100, force));
}//checked
void move() {
	Position speed;
	double mode = (10 + kMaxMoveSpeed + kDashSpeed[me.skill_level[DASH]]) / distance(go_for, me.pos);
	speed = multiple(mode, minus(go_for, me.pos));
	Move(me.id, speed);
}
void anti_lock() {
	if (length(me.speed) < 20) return;
	Position speed;
	Position center = { kMapSize >> 1,kMapSize >> 1,kMapSize >> 1 };
	if (!num_of_devour) {
		if (!num_of_food) speed = minus(center, me.pos);
		else speed = minus(food[0], me.pos);
		goto MOVE;
	}
	if (!(me.pos.x > (kMapSize - 2 * me.radius))) {
		speed = { 100,0,0 };
		if (zw_devour(1.1*me.radius, speed) < 1) goto MOVE;
	}
	if (!(me.pos.x<2 * me.radius)) {
		speed = { -100,0,0 };
		if (zw_devour(1.1*me.radius, speed) < 1) goto MOVE;
	}
	if (!(me.pos.y > (kMapSize - 2 * me.radius))) {
		speed = { 0,100,0 };
		if (zw_devour(1.1*me.radius, speed) < 1) goto MOVE;
	}
	if (!(me.pos.y<2 * me.radius)) {
		speed = { 0,-100,0 };
		if (zw_devour(1.1*me.radius, speed) < 1) goto MOVE;
	}
	if (!(me.pos.z > (kMapSize - 2 * me.radius))) {
		speed = { 0,0,100 };
		if (zw_devour(1.1*me.radius, speed) < 1) goto MOVE;
	}
	if (!(me.pos.z<2 * me.radius)) {
		speed = { 0,0,-100 };
		if (zw_devour(1.1*me.radius, speed) < 1) goto MOVE;
	}
	//sorry
	if (!(me.pos.x > (kMapSize - 2 * me.radius))) {
		speed = { 100,0,0 };
		if (zw_devour(1.1*me.radius, speed) < 2) goto MOVE;
	}
	if (!(me.pos.x<2 * me.radius)) {
		speed = { -100,0,0 };
		if (zw_devour(1.1*me.radius, speed) < 2) goto MOVE;
	}
	if (!(me.pos.y > (kMapSize - 2 * me.radius))) {
		speed = { 0,100,0 };
		if (zw_devour(1.1*me.radius, speed) < 2) goto MOVE;
	}
	if (!(me.pos.y<2 * me.radius)) {
		speed = { 0,-100,0 };
		if (zw_devour(1.1*me.radius, speed) < 2) goto MOVE;
	}
	if (!(me.pos.z > (kMapSize - 2 * me.radius))) {
		speed = { 0,0,100 };
		if (zw_devour(1.1*me.radius, speed) < 2) goto MOVE;
	}
	if (!(me.pos.z<2 * me.radius)) {
		speed = { 0,0,-100 };
		if (zw_devour(1.1*me.radius, speed) < 2) goto MOVE;
	}
MOVE:
	speed = MaximumSpeed(speed);
	Move(me.id, speed);
	int t = GetTime();
	while ((GetTime() - t) < 50) {
		update();
		opponent();
	}
	solution[OPPONENT].weight = -1;
}//checked
int zw_IsDevour(double d, Position des, Position speed)
{
	int flag = 0;
	Position Next;
	for (int i = 1; i <= 60; i++)
	{
		Next = add(me.pos, multiple(i, MaximumSpeed(speed)));
		if (distance(Next, des) < d)
			flag = 1;
	}
	return flag;
}
int zw_devour(double d, Position speed) {
	int result = 0;
	int temp = num_of_devour - 1;
	for (temp; ~temp; --temp) {
		if (zw_IsDevour(d, devour[temp], speed)) result++;
	}
	return result;
}
double get_health(double r) {
	double temp = r / 100.0;
	return temp*temp*temp;
}//checked
//A function bodies
int initial() {
	const Map *map = GetMap();
	const double r = 0.75;
	for (int i = NUM_OF_SOLUTION-1;~i;--i) {
		solution[i].weight = -1;
	}
	me = GetStatus()->objects[0];
	solution[0].weight = solution[1].weight = 0;
	num_of_aim = num_of_food = num_of_advance = num_of_devour = 0;
	emergency = code = 0;
	for (int i = (*map).objects_number - 1; ~i; --i) {
		switch ((*map).objects[i].type) {
		case PLAYER:
			if ((*map).objects[i].team_id == GetStatus()->team_id) break;
			opponent_obj = (*map).objects[i];
			code |= OPPONENT;
			break;
		case ENERGY:
			if ((*map).objects[i].pos.x - r * me.radius < 0) break;
			if ((*map).objects[i].pos.y - r * me.radius < 0) break;
			if ((*map).objects[i].pos.z - r * me.radius < 0) break;
			if ((*map).objects[i].pos.x + r * me.radius > kMapSize) break;
			if ((*map).objects[i].pos.y + r * me.radius > kMapSize) break;
			if ((*map).objects[i].pos.z + r * me.radius > kMapSize) break;
			food[num_of_food] = (*map).objects[i].pos;
			++num_of_food;
			break;
		case ADVANCED_ENERGY:
			if (distance((*map).objects[i].pos, a_variable_for_YQY[0]) < 2*r * me.radius) break;
			if (distance((*map).objects[i].pos, a_variable_for_YQY[1]) < 2*r * me.radius) break;
			if (distance((*map).objects[i].pos, a_variable_for_YQY[2]) < 2*r * me.radius) break;
			if (distance((*map).objects[i].pos, a_variable_for_YQY[3]) < 2*r * me.radius) break;
			if (distance((*map).objects[i].pos, a_variable_for_YQY[4]) < 2*r * me.radius) break;
			if (distance((*map).objects[i].pos, a_variable_for_YQY[5]) < 2*r * me.radius) break;
			if (distance((*map).objects[i].pos, a_variable_for_YQY[6]) < 2*r * me.radius) break;
			if (distance((*map).objects[i].pos, a_variable_for_YQY[7]) < 2*r * me.radius) break;
			advanced[num_of_advance] = (*map).objects[i].pos;
			++num_of_advance;
			break;
		case SOURCE: break;
		case DEVOUR:
			devour[num_of_devour] = (*map).objects[i].pos;
			++num_of_devour;
			break;
		case BOSS:
			boss_obj = (*map).objects[i];
			code |= SEE_BOSS;
			break;
		default: break;
		}
	}
	return code;
}//checked
int boss() {
	int code;
	emergency = code = 0;
	if (boss_obj.radius < me.radius * kEatableRatio*0.9) {
		solution[SEE_BOSS].weight = EAT_BOSS_WEIGHT;
		solution[SEE_BOSS].pos = boss_obj.pos;
		return code;
	}
	int tmp = (short_attack(boss_obj)+1)*kShortAttackDamage[me.skill_level[SHORT_ATTACK]];
	if (!tmp) tmp = (long_attack(boss_obj) + 1)*kShortAttackDamage[me.skill_level[LONG_ATTACK]];
	if (tmp) code = 1;
	if (me.radius < boss_obj.radius * kEatableRatio * 1.1) {
		if (distance(me.pos, boss_obj.pos) < 500+boss_obj.radius) emergency = 1;
		solution[SEE_BOSS].weight = emergency ? BOSS_EAT_WEIGHT : TRASH;
		solution[SEE_BOSS].pos = add(me.pos, minus(me.pos, boss_obj.pos));
	}
	else {
		double new_health = get_health(boss_obj.radius) - tmp;
		if (new_health < me.health*kEatableRatio*kEatableRatio*kEatableRatio*0.8) {
			solution[SEE_BOSS].weight = EAT_BOSS_WEIGHT;
			solution[SEE_BOSS].pos = boss_obj.pos;
			return code;
		}
		solution[SEE_BOSS].weight = TRASH;
		solution[SEE_BOSS].pos = me.pos;
	}
	return code;
}//checked
//M function bodies
int long_attack(const Object& target)
{
	if (me.skill_cd[LONG_ATTACK] == -1) {
		return -1;
	}
	if (me.long_attack_casting != -1 /*|| me.short_attack_casting != -1*/) {
		return -1;
	}
	if (target.shield_time > 0) {
		return -1;
	}
	if (distance(target.pos, me.pos) - target.radius - me.radius
	> kLongAttackRange[me.skill_level[LONG_ATTACK]]) {
		return -1;
	}
	WAIT;
	LongAttack(me.id, target.id);
	GO;
	return 0;
}
int short_attack(const Object& target)
{
	dash();
	if (me.skill_cd[SHORT_ATTACK] == -1) {
		return -1;
	}
	if (me.long_attack_casting != -1 /*|| me.short_attack_casting != -1*/) {
		return -1;
	}
	if (target.shield_time > 0) {
		return -1;
	}
	if (distance(target.pos, me.pos) - target.radius - me.radius
			> kShortAttackRange[me.skill_level[SHORT_ATTACK]]) {
		return -1;
	}
	WAIT;
	ShortAttack(me.id);
	GO;
	return 0;
}
int dash()
{
	if (me.long_attack_casting != -1 /*|| me.short_attack_casting != -1*/) {
		return -1;
	}
	if (!me.skill_level[DASH]) {
		return -1;
	}
	if (me.skill_cd[DASH] == -1) {
		return -1;
	}
	WAIT;
	Dash(me.id);
	GO;
	return 0;
}
int opponent()
{
	const double safe_distance = 1000;
	int result = 0;
	solution[OPPONENT].pos = { kMapSize >> 1, kMapSize >> 1, kMapSize >> 1 };
	solution[OPPONENT].weight = 0;
	dash();
	if (short_attack(opponent_obj) == 0 || long_attack(opponent_obj) == 0) {
		result = 1;
	}
	if (opponent_obj.radius < me.radius * kEatableRatio
		&& distance(me.pos, opponent_obj.pos) - opponent_obj.radius < 2.5*safe_distance) {
		solution[OPPONENT].pos = opponent_obj.pos;
		solution[OPPONENT].weight = EAT_BOSS_WEIGHT;
		return result;
	}
	if (opponent_obj.radius*(kEatableRatio * 1.05) > me.radius) {
		solution[OPPONENT].pos = add(me.pos, minus(me.pos, opponent_obj.pos));
		solution[OPPONENT].weight = BOSS_EAT_WEIGHT;
		if (distance(me.pos, opponent_obj.pos) - opponent_obj.radius < safe_distance) {
			emergency = 1;
		}
		return result;
	}
	if (opponent_obj.radius > me.radius || me.skill_level[SHORT_ATTACK]<3) {
		solution[OPPONENT].pos = add(me.pos, minus(me.pos, opponent_obj.pos));
		solution[OPPONENT].weight = RUN_AWAY_WEIGHT;
		return result;
	}
	return result;
}//waiting
//Y function bodies
void avoid()
{
	me = GetStatus()->objects[0];
	Position default_pos[6];
	
	default_pos[0] = add(me.pos, another_variable_for_YQY[0]);
	default_pos[1] = add(me.pos, another_variable_for_YQY[1]);
	default_pos[2] = add(me.pos, another_variable_for_YQY[2]);
	default_pos[3] = add(me.pos, another_variable_for_YQY[3]);
	default_pos[4] = add(me.pos, another_variable_for_YQY[4]);
	default_pos[5] = add(me.pos, another_variable_for_YQY[5]);
	int IsDevour(double d, Position des, Position speed);
	Position Schmidt(Position a1, Position a2);
	int flag;//记录是否选取
	int flag2;//记录是否在瞬移后前进方向有devour
	int j, devour_count;
	int devour_danger;
	int i;
	Position aim_devour;
	point target;
	if (solution[1].weight>solution[2].weight)
		target = solution[1];
	else
		target = solution[2];
	if (aim[0].weight>target.weight)
		target = aim[0];
	if (length(minus(target.pos, aim[0].pos))<1e-6)//如果target是aim
	{
		//printf("aim!\n");
		for (i = 0; i<num_of_aim; i++)
		{
			flag = 1;
			for (j = 0; j<num_of_devour; j++)
			{
				if (distance(aim[i].pos, devour[j])<1 * me.radius)//如果目标附近有吞噬者
					if (distance(aim[i].pos, me.pos)> 0.5*distance(devour[j], me.pos))
						flag = 0;
			}
			if (flag == 0)//旁边有devour，扔掉
				continue;
			else
			{
				Position speed = minus(aim[i].pos, me.pos);
				devour_count = 0;
				if (me.shield_time>25 && me.skill_level[SHIELD] == 5)
					devour_danger = 0;
				else
					devour_danger = 1;
				if (devour_danger)
				{
					for (j = 0; j<num_of_devour; j++)
					{
						if (IsDevour(1.1*me.radius, devour[j], speed))
						{
							devour_count++;
							aim_devour = devour[j];
						}
					}
				}
				if (devour_count >= 2)
					continue;
				else
					if (devour_count == 1)
					{
						Position a2 = minus(aim_devour, me.pos);
						speed = Schmidt(speed, a2);
						go_for = add(me.pos, speed);
						return;
					}
					else
					{
						go_for = aim[i].pos;
						return;
					}
			}
		}
		if (i == num_of_aim)//瞬移了
		{
			//printf("sudden moving!\n");
			flag2 = 0;
			for (j = 0; j<num_of_devour; j++)
			{
				if (IsDevour(1.5*me.radius, devour[j], me.speed))
					flag2 = 1;
			}
			if (!flag2)//如果没有问题
			{
				go_for = add(me.pos, me.speed);
				return;
			}
			else//有问题，随机一个没有devour的方向
			{
				for (i = 0; i<6; i++)
				{
					for (j = 0; j<num_of_devour; j++)
						if (!IsDevour(1.5*me.radius, devour[j], another_variable_for_YQY[i]))
						{
							go_for = default_pos[i];
							return;
						}
				}
			}
		}
	}

	else//用了boss和opponent
	{
		printf("boss!opponent!\n");
		Position speed = minus(target.pos, me.pos);
		devour_count = 0;
		if (me.shield_time>25 && me.skill_level[SHIELD] == 5)
			devour_danger = 0;
		else
			devour_danger = 1;
		if (devour_danger)
		{
			for (j = 0; j<num_of_devour; j++)
			{
				if (IsDevour(1.1*me.radius, devour[j], speed))
				{
					devour_count++;
					aim_devour = devour[j];
				}
			}
		}
		if (devour_count >= 1)
		{
			Position a2 = minus(aim_devour, me.pos);
			speed = Schmidt(speed, a2);
			go_for = add(me.pos, speed);
			return;
		}
		else
		{
			go_for = target.pos;
			return;
		}
		return;
	}
}
Position Schmidt(Position a1, Position a2)
{
	Position temp1, temp2;
	temp2 = multiple(dot_product(a1, a2) / dot_product(a2, a2), a2);
	temp1 = minus(a1, temp2);
	return temp1;
}//checked? okok hehehe
int IsDevour(double d, Position des, Position speed)//
{
	int flag = 0;
	Position Next;
	for (int i = 1; i <= 8; i++)
	{
		Next = add(me.pos, multiple(i, MaximumSpeed(speed)));
		if (distance(Next, des)<d)
			flag = 1;
	}
	return flag;
}//checked
Position MaximumSpeed(Position vec) {
	register double len = length(vec);
	vec.x *= (10+kMaxMoveSpeed + kDashSpeed[me.skill_level[DASH]]) / len;
	vec.y *= (10+kMaxMoveSpeed + kDashSpeed[me.skill_level[DASH]]) / len;
	vec.z *= (10+kMaxMoveSpeed + kDashSpeed[me.skill_level[DASH]]) / len;
	return vec;
}//checked
