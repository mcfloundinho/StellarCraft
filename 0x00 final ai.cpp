#include "teamstyle17.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <algorithm>

#define LOG_Z
#define TEST
#define MAX_SIZE 100
#define WAIT  while(GetTime()==operate_time);
#define GO   (operate_time=GetTime());
#define NUM_OF_SOLUTION 3
#define MAX(a, b) ((a) > (b) ? (a) : (b))

//enums & structure
enum situation {
	NONE = 0,
	OPPONENT = 1,
	SEE_BOSS = 2
};
struct point {
	int weight;
	Position pos;
};
enum value {
	//ZW about food & aim
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
int good;
Position good_position;
int anti_block_time;
int ever_update;
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
void _avoid();
void avoid_(Position);
int opponent();
int boss();
void anti_lock();
void move();

//auxiliary functions
double get_health(double);
int zw_cost(int skill);
int zw_cmp(const point&, const point&);
int long_attack(const Object& target);
int short_attack(const Object& target);
int dash();
void zw_enshaw();
void attack_update();
int IsDevour(double, Position, Position);
int zw_IsDevour(double, Position, Position);
Position MaximumSpeed(Position);
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
																
																#ifdef LOG_Z
																	   std::ios::sync_with_stdio(false);
																#endif
	srand(time(0));
	anti_block_position = GetStatus()->objects[0].pos;
	for (;;) {
																#ifdef LOG_Z
																	   std::cout<<GetTime()<<"Initial begin"<<std::endl;
																#endif
		code = initial();
																#ifdef LOG_Z
																	   std::cout<<GetTime()<<"Initial end"<<std::endl;
																	   std::cout<<"E\t"<<num_of_food<<std::endl;
																	   std::cout<<"A\t"<<num_of_advance<<std::endl;
																	   std::cout<<"D\t"<<num_of_devour<<std::endl;

																#endif
		if ((GetTime() >> 6) != anti_block_time) {
			if (distance(anti_block_position, GetStatus()->objects[0].pos)<1000) {
				goto ANTI_LOCK;
			}
			anti_block_time = (GetTime() >> 6);
			anti_block_position = GetStatus()->objects[0].pos;
			show(anti_block_position);
			std::cout<<anti_block_time<<std::endl;
		}
		if (distance(GetStatus()->objects[0].pos, me.pos) > 2000) goto AVOID;
//	START:
		if (ever_update) {
																#ifdef LOG_Z
																	   std::cout<<GetTime()<<"Update begin"<<std::endl;
																#endif
			update();
																#ifdef LOG_Z
																	   std::cout<<GetTime()<<"Update end"<<std::endl;
																#endif
		}
		else if (me.ability > 34) {
			ever_update = 1;
																#ifdef LOG_Z
																	   std::cout<<GetTime()<<"NU"<<std::endl;
																#endif
		}
		if (distance(GetStatus()->objects[0].pos, me.pos) > 2000) goto AVOID;
		if (code & SEE_BOSS) {
																#ifdef LOG_Z
																	   std::cout<<GetTime()<<"Boss begin"<<std::endl;
																#endif
			boss();
																#ifdef LOG_Z
																	   std::cout<<GetTime()<<"Boss end"<<std::endl;
																#endif
		}
		if (distance(GetStatus()->objects[0].pos, me.pos) > 2000) goto AVOID;
		if (code & OPPONENT) {
																#ifdef LOG_Z
																	   std::cout<<GetTime()<<"Opponent begin"<<std::endl;
																#endif
			if (opponent() < 0) continue;
																#ifdef LOG_Z
																	   std::cout<<GetTime()<<"Opponent end"<<std::endl;
																#endif
		}
		if (distance(GetStatus()->objects[0].pos, me.pos) > 2000) goto AVOID;
		if (!emergency) {
																#ifdef LOG_Z
																	   std::cout<<GetTime()<<"Greedy begin"<<std::endl;
																#endif
			greedy();
																#ifdef LOG_Z
																	   std::cout<<GetTime()<<"Greedy end"<<std::endl;
																#endif
		}
																#ifdef LOG_Z
																	   std::cout<<GetTime()<<"Avoid begin"<<std::endl;
																#endif
		avoid();
																#ifdef LOG_Z
																	   std::cout<<GetTime()<<"Avoid end"<<std::endl;
																#endif
		if (false) {
		AVOID:
																#ifdef LOG_Z
																			std::cout << "A_SUDDEN_MOVE_HAPPENED\t" <<GetTime()<< std::endl;
																#endif
//			initial();
//			_avoid();
//			move();
																#ifdef LOG_Z
																	   std::cout<<GetTime()<<"Brake begin"<<std::endl;
																#endif
			Move(me.id, {0, 0, 0});
																#ifdef LOG_Z
																	   std::cout<<GetTime()<<"Brake end"<<std::endl;
																#endif
//			goto START;
			continue;
		}
		
		move();
																#ifdef LOG_Z
																	   std::cout << "go_for = ";
																	   show(go_for);
																#endif
		
		if (false) {
		ANTI_LOCK:
																#ifdef LOG_Z
																			std::cout << "ANTI_LOCK_PROCEEDING" << std::endl;
																#endif
			anti_lock();
			anti_block_time = (GetTime() >> 6);
			anti_block_position = GetStatus()->objects[0].pos;
																#ifdef LOG_Z
																			show(anti_block_position);
																			std::cout<<anti_block_time<<std::endl;
																#endif
		}
	}
}

//C function bodies
Position add(Position a, Position b){
	Position c;
	c.x = a.x + b.x;
	c.y = a.y + b.y;
	c.z = a.z + b.z;
	return c;
}
Position minus(Position a, Position b){
	Position c;
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	c.z = a.z - b.z;
	return c;
}
Position multiple(double k, Position a){
	Position c;
	c.x = k*a.x;
	c.y = k*a.y;
	c.z = k*a.z;
	return c;
}
Position cross_product(Position a, Position b){
	Position c;
	c.x = a.y*b.z - a.z*b.y;
	c.y = -a.x*b.z + a.z*b.x;
	c.z = a.x*b.y - a.y*b.x;
	return c;
}
double dot_product(Position a, Position b){
	return (a.x*b.x + a.y*b.y + a.z*b.z);
}
double length(Position a){
	return sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
}
Position norm(Position a){
	double l = 1 / length(a);
	return multiple(l, a);
}
double distance(Position a, Position b){
	return length(minus(a, b));
}
void show(Position a){
	std::cout << "The position is (" << a.x << "," << a.y << "," << a.z << ")" << std::endl;
}

//Z function bodies
void attack_update() {
	for (;;) {
		me = GetStatus() -> objects[0];
		if (me.skill_level[SHORT_ATTACK] < 4) {
			if (me.ability >= zw_cost(SHORT_ATTACK)) {
																#ifdef LOG_Z
																	std::cout << "try attack_update(SHORT_ATTACK)" << std::endl;
																#endif
				WAIT;
				UpgradeSkill(me.id, SHORT_ATTACK);
				GO;
			}
			else break;
		}
		else if (me.skill_level[HEALTH_UP] < 4){
			if (me.ability >= zw_cost(HEALTH_UP)){
																#ifdef LOG_Z
																	std::cout << "try attack_update(HEALTH_UP)" << std::endl;
																#endif
				WAIT;
				UpgradeSkill(me.id, HEALTH_UP);
				GO;
			}
			else break;
		}
		else if (me.skill_level[DASH] < 4) {
			if (me.ability >= zw_cost(DASH)) {
																#ifdef LOG_Z
																	std::cout << "try attack_update(DASHs)" << std::endl;
																#endif
				WAIT;
				UpgradeSkill(me.id, DASH);
				GO;
			}
			else break;
		}
		else break;
	}
}
int zw_cost(int skill) {
	me = GetStatus() -> objects[0];
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
	if ((!me.skill_cd[SHIELD]) && (me.short_attack_casting==-1) && (me.long_attack_casting==-1)) {
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
																//	std::cout << "++" << std::endl;
																//	for (int temp = num_of_aim - 1; ~temp; --temp) {
																//		std::cout << aim[temp].weight << std::endl;
																//	}
																//	std::cout << "++" << std::endl;
																#endif
	std::sort(aim, aim + num_of_aim, zw_cmp);
																#ifdef LOG_Z
																	std::cout << "--" <<GetTime()<<"--"<< std::endl;
																	for (int temp = num_of_aim - 1; ~temp; --temp) {
																		std::cout << aim[temp].weight << std::endl;
																	}
																	std::cout << "--" << std::endl;
																#endif
}
int zw_cmp(const point& p, const point& q) {
	return p.weight > q.weight;
}//waiting
void zw_enshaw() {
	if (!num_of_food) {
		aim[0].pos = { kMapSize >> 1,kMapSize >> 1,kMapSize >> 1 };
		aim[0].weight = 100;
	}
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
int zw_IsDevour(double d, Position des, Position speed){
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
	for (; ~temp; --temp) {
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
	solution[OPPONENT].weight = solution[SEE_BOSS].weight = -1;
	me = GetStatus()->objects[0];
	solution[0].weight = solution[1].weight = 0;
	num_of_aim = num_of_food = num_of_advance = num_of_devour = 0;
	int code;
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
			if (distance((*map).objects[i].pos, a_variable_for_YQY[0]) < 2 * r * me.radius) break;
			if (distance((*map).objects[i].pos, a_variable_for_YQY[1]) < 2 * r * me.radius) break;
			if (distance((*map).objects[i].pos, a_variable_for_YQY[2]) < 2 * r * me.radius) break;
			if (distance((*map).objects[i].pos, a_variable_for_YQY[3]) < 2 * r * me.radius) break;
			if (distance((*map).objects[i].pos, a_variable_for_YQY[4]) < 2 * r * me.radius) break;
			if (distance((*map).objects[i].pos, a_variable_for_YQY[5]) < 2 * r * me.radius) break;
			if (distance((*map).objects[i].pos, a_variable_for_YQY[6]) < 2 * r * me.radius) break;
			if (distance((*map).objects[i].pos, a_variable_for_YQY[7]) < 2 * r * me.radius) break;
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
	if (boss_obj.radius < me.radius * kEatableRatio) {
		solution[SEE_BOSS].weight = EAT_BOSS_WEIGHT;
		solution[SEE_BOSS].pos = boss_obj.pos;
		return code;
	}
	int tmp = (short_attack(boss_obj)+1)*kShortAttackDamage[me.skill_level[SHORT_ATTACK]];
	if (!tmp) tmp = (long_attack(boss_obj) + 1)*kShortAttackDamage[me.skill_level[LONG_ATTACK]];
	if (tmp) code = 1;
	if (me.radius < boss_obj.radius * kEatableRatio) {
		if (distance(me.pos, boss_obj.pos) < 500 + boss_obj.radius) emergency = 1;
		solution[SEE_BOSS].weight = emergency ? BOSS_EAT_WEIGHT : TRASH;
		solution[SEE_BOSS].pos = add(me.pos, minus(me.pos, boss_obj.pos));
	}
	else {
		double new_health = get_health(boss_obj.radius) - tmp;
		if (new_health < me.health * kEatableRatio * kEatableRatio * kEatableRatio) {
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
int long_attack(const Object& target){
	if (me.skill_cd[LONG_ATTACK] == -1) {
		return -1;
	}
	if (me.long_attack_casting != -1 || me.short_attack_casting != -1) {
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
int short_attack(const Object& target){
	if (me.skill_cd[SHORT_ATTACK] == -1) {
		return -1;
	}
	if (me.long_attack_casting != -1 || me.short_attack_casting != -1) {
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
int dash(){
	if (me.long_attack_casting != -1 || me.short_attack_casting != -1) {
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
int opponent(){
	if (!ever_update) {
		if (code & SEE_BOSS) return 0;
																#ifdef LOG_Z
																	   std::cout << "opponent() && !ever_update" << std::endl;
																#endif
		int tmp;
		int my_health = me.health;
		ever_update = 1;
		go_for = opponent_obj.pos;
		move();
		attack_update();
		for(;;){
			dash();
			tmp = short_attack(opponent_obj) || long_attack(opponent_obj);
																#ifdef LOG_Z
																	   std::cout << "----------------------" << std::endl;
																	   std::cout << "==== OPPONENT_POS ====" << std::endl;
																	   show(opponent_obj.pos);
																	   std::cout << "==== ME_POS ====" << std::endl;
																	   show(GetStatus() -> objects[0].pos);
																	   std::cout << "----------------------" << std::endl;
																#endif
			if (!(initial() & OPPONENT)) break;
			go_for = opponent_obj.pos;
			dash();
			move();
			tmp = short_attack(opponent_obj) || long_attack(opponent_obj);
			if (GetStatus() -> objects[0].health - my_health < -1000) break;
			if (GetStatus() -> objects[0].radius < opponent_obj.radius * kEatableRatio) break;
																#ifdef LOG_Z
																	   std::cout << "======================" << std::endl;
																	   std::cout << "==== OPPONENT_POS ====" << std::endl;
																	   show(opponent_obj.pos);
																	   std::cout << "==== ME_POS ====" << std::endl;
																	   show(GetStatus() -> objects[0].pos);
																	   std::cout << "======================" << std::endl;
																#endif
		}
	 	return -1;
	}
	const double safe_distance = 1000;
	int result = 0;
	solution[OPPONENT].pos = { kMapSize >> 1, kMapSize >> 1, kMapSize >> 1 };
	solution[OPPONENT].weight = 0;
	dash();
	if (short_attack(opponent_obj) == 0 || long_attack(opponent_obj) == 0) {
		result = 1;
	}
	if (opponent_obj.radius < me.radius * kEatableRatio
		&& distance(me.pos, opponent_obj.pos) - opponent_obj.radius < 2.5 * safe_distance) {
		solution[OPPONENT].pos = opponent_obj.pos;
		solution[OPPONENT].weight = EAT_BOSS_WEIGHT;
		return result;
	}
	if (opponent_obj.radius * kEatableRatio > me.radius) {
		solution[OPPONENT].pos = add(me.pos, minus(me.pos, opponent_obj.pos));
		solution[OPPONENT].weight = BOSS_EAT_WEIGHT;
		if (distance(me.pos, opponent_obj.pos) - opponent_obj.radius < safe_distance) {
			emergency = 1;
		}
		return result;
	}
	if (opponent_obj.radius > me.radius || me.skill_level[SHORT_ATTACK] < 3) {
		solution[OPPONENT].pos = add(me.pos, minus(me.pos, opponent_obj.pos));
		solution[OPPONENT].weight = RUN_AWAY_WEIGHT;
		return result;
	}
	return result;
}//waiting

//Y function bodies
void avoid() {
	point target;
	int i;
	int j;
	int flag;
	for (i = 0;i < num_of_aim;i++) {
		flag = 0;
		Position r1 = minus(aim[i].pos, me.pos);
		Position R = norm(r1);
		if (length(r1) < 5000) {
			for (j = 0;j < num_of_devour;j++) {
				Position r2 = minus(devour[j], me.pos);
				if (dot_product(r2, R)>0 && dot_product(r2, R) < length(r1)) {
					if (length(cross_product(r2, R)) < 1.1*me.radius) {
						flag = 1;
						break;
					}
				}
			}
			if (!flag) {
				target = aim[i];
				break;
			}
		}
		else {
			for (j = 0;j < num_of_devour;j++) {
				Position r2 = minus(devour[j], me.pos);
				if (dot_product(r2, R)>0 && dot_product(r2, R) < 2000) {
					if (length(cross_product(r2, R)) < 1.1*me.radius) {
						flag = 1;
						break;
					}
				}
			}
			if (!flag) {
				target = aim[i];
				break;
			}
		}
	}
	if ((i!=num_of_aim)&&target.weight > MAX(solution[OPPONENT].weight, solution[SEE_BOSS].weight)) {
		go_for=target.pos;
		return;
	}
	else {
		if (solution[OPPONENT].weight == -1 && solution[SEE_BOSS].weight == -1) {
			avoid_(norm(aim[0].pos));
			return;
		}
		target = (solution[OPPONENT].weight>solution[SEE_BOSS].weight) ? solution[OPPONENT] : solution[SEE_BOSS];
		flag = 0;
		Position R = norm(minus(target.pos, me.pos));
		for (j = 0;j < num_of_devour;j++) {
			Position r2 = minus(devour[j], me.pos);
			if (dot_product(r2, R)>0 && dot_product(r2, R) < 2000) {
				if (length(cross_product(r2, R)) < 1.1*me.radius) {
					flag = 1;
					break;
				}
			}
		}
		if (!flag) {
			go_for = target.pos;
			good=-1;
			return;
		}
		else {
			avoid_(norm(target.pos));
			return;
		}
	}
}
int IsDevour(double d, Position des, Position speed){
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
void _avoid() {
	Position default_pos[6];
	default_pos[0] = add(me.pos, another_variable_for_YQY[0]);
	default_pos[1] = add(me.pos, another_variable_for_YQY[1]);
	default_pos[2] = add(me.pos, another_variable_for_YQY[2]);
	default_pos[3] = add(me.pos, another_variable_for_YQY[3]);
	default_pos[4] = add(me.pos, another_variable_for_YQY[4]);
	default_pos[5] = add(me.pos, another_variable_for_YQY[5]);
	int flag2;
	int i, j;
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
void avoid_(Position R) {
	Position axis[6] = {
		{1.0,0,0},
		{-1.0,0,0},
		{0,1.0,0},
		{0,-1.0,0},
		{0,0,1.0},
		{0,0,-1.0}
	};
	for (int i = 0; i < 6; ++i)
		for (int j = i; j < 6; ++j)
			if (dot_product(axis[i], R) < dot_product(axis[j], R)) {
				Position tmp = axis[i];
				axis[i] = axis[j];
				axis[j] = tmp;
			}
	int flag[6] = {};
	for (int i = 0;i < num_of_devour;i++) {
		Position r2 = minus(devour[i], me.pos);
		if (r2.x > 0 && r2.x < 2000) {
			if (length(cross_product(r2, axis[0])) < 1.1*me.radius) flag[0]++;
		}
		if (r2.x > -2000 && r2.x < 0) {
			if (length(cross_product(r2, axis[1])) < 1.1*me.radius) flag[1]++;
		}
		if (r2.y > 0 && r2.y < 2000) {
			if (length(cross_product(r2, axis[2])) < 1.1*me.radius) flag[2]++;
		}
		if (r2.y > -2000 && r2.y < 0) {
			if (length(cross_product(r2, axis[3])) < 1.1*me.radius) flag[3]++;
		}
		if (r2.z > 0 && r2.z < 2000) {
			if (length(cross_product(r2, axis[4])) < 1.1*me.radius) flag[4]++;
		}
		if (r2.z > -2000 && r2.z < 0) {
			if (length(cross_product(r2, axis[5])) < 1.1*me.radius) flag[5]++;
		}
	}
	int i;
	for (i = 0;i < 6;i++) {
		if (!flag[i]) {
			go_for = add(me.pos, multiple(100.0, axis[i]));
			return;
		}
	}
	if (i == 6) {
		for (i = 0;i < 6;i++) {
			if (flag[i]==1) {
				go_for = add(me.pos, multiple(100.0, axis[i]));
				return;
			}
		}
	}
}
