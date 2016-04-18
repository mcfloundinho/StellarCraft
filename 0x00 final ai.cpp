#include "teamstyle17.h"
#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<time.h>
#include<iostream>
#define MAX_SIZE 1000
#define WAIT  while(GetTime()==operate_time);
#define GO   (operate_time=GetTime());
#define NUM_OF_SOLUTION  3
enum stuation {
	NONE=0,
	OPPONENT=1<<0,
	SEE_BOSS=1<<1,
};
struct point {
	int weight;
	Position pos;
};
struct ball {
	Position center;
	double radius;
};
//variable							initial							use
PlayerObject me;//					initial()						almost everywhere
int emergency;	//					opponent(),boss()				greedy()
int ad_weight = 100;//				initial()						initial()
int num_of_aim;//					greedy()						avoid()
int num_of_food;//					initial()						greedy()
int code;//							initial()						AIMain()&greedy()
double me_radius;//					initial()						almost everywhere
double opponent_radius;//			initial()						opponent()
double boss_radius;//				initial()						greedy()
Position boss_pos;//				initial()						action()
Position opponent_pos;//			initial()						opponent()
point aim[MAX_SIZE];//				greedy()						avoid()
point food[MAX_SIZE];//				initial()						greedy()
Position devour[MAX_SIZE];//		initial()						avoid()
ball devour_for_YQY[MAX_SIZE];//	initial()						avoid()
point solution[NUM_OF_SOLUTION];//	opponent(),boss()				avoid()
Position go_for;//					avoid()							move()
Position last_move;//				move()							move()
int operate_time;//					after any action				before any action		
//core function						function						author	round_cost
int initial();//					initial,update the basic value	ARC
void greedy();//					find the best food				ZWT
int update();//						update the skills, shield		ZWT		1cost
void avoid();//						avoid the devour and border		YQY
int opponent();//					deal with the opponent			PLU		1cost
int boss();//						smaller, kill; bigger, eat		ARC		1cost		
void move();//						move to							PLU		1cost
//auxiliary variables
char bitmap[1000 >> 3+1];
//auxiliary function
int zw_cost(int skill);
int zw_cmp(const void*, const void*);
//lower level function
Position add(Position a, Position b);//a+b
Position minus(Position a, Position b);//a-b，从B指向A的向量
Position multiple(double k, Position a);//ka，数乘
Position cross_product(Position a, Position b);//a*b 叉乘
double length(Position a);//求矢量模长
Position norm(Position a);//求单位矢量
double distance(Position a, Position b);//求AB两点距离
void show(Position a);//输出矢量 
//Main
void AIMain() {
	srand(time(0));
	code = initial();
	update();
	if (code&OPPONENT) {
		opponent();
	}
	if (code&SEE_BOSS) {
		boss();
	}
	if (!emergency) {
		greedy();
	}
	avoid();
	move();
}
//function body
Position add(Position a,Position b)
{
    Position c;
    c.x=a.x+b.x;
    c.y=a.y+b.y;
    c.z=a.z+b.z;
    return c;
}
Position minus(Position a,Position b)
{
    Position c;
    c.x=a.x-b.x;
    c.y=a.y-b.y;
    c.z=a.z-b.z;
    return c;
}
Position multiple(double k,Position a)
{
    Position c;
    c.x=k*a.x;
    c.y=k*a.y;
    c.z=k*a.z;
    return c;
}
Position cross_product(Position a,Position b)
{
    Position c;
    c.x=a.y*b.z-a.z*b.y;
    c.y=-a.x*b.z+a.z*b.x;
    c.z=a.x*b.y-a.y*b.x;
    return c;
}
double length(Position a)
{
    return sqrt(a.x*a.x+a.y*a.y+a.z*a.z);
}
Position norm(Position a)
{
    double l=1/length(a);
    return multiple(l,a);
}
double distance(Position a,Position b)
{
    return length(minus(a,b));
}
void show(Position a)
{
    printf("The position is (%f,%f,%f).\n",a.x,a.y,a.z);
}
int zw_cost(int skill) {
	if (me.skill_level[skill]) {
		return kBasicSkillPrice[skill] << me.skill_level[skill];
	}
	else {
		int count = 0;
		for (int i = 0;i < kSkillTypes;i++) {
			if (me.skill_level[i]) count++;
		}
		return kBasicSkillPrice[skill] << count;
	}
}
int update() {
	if (!me.skill_cd[SHIELD]) {
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
		}
		else if (me.skill_level[SHIELD] < kMaxSkillLevel) {
			if (me.ability >= zw_cost(SHIELD)) {
				WAIT;
				UpgradeSkill(me.id, SHIELD);
				GO;
				return 1;
			}
			else return 0;
		}
		else if (me.skill_level[SHORT_ATTACK] < kMaxSkillLevel) {
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
				if (me.skill_level[LONG_ATTACK] < kMaxSkillLevel) {
					if (me.ability >= zw_cost(LONG_ATTACK)) {
						WAIT;
						UpgradeSkill(me.id, LONG_ATTACK);
						GO;
						return 1;
					}
			}
		}
		return 0;
	}
}
void greedy() {
	int check = 0.9*me_radius;
	if (emergency)return;
	memset(bitmap, 0, 100 >> 3 + 1);
	while (true) {
		int next;
		for (next = 0;next < num_of_food;next++) {
			if (!(bitmap[next >> 3] & (0x80 >> (next & 0x07)))) break;
		}
		if (next == num_of_food)break;
		aim[num_of_aim] = food[next];
		bitmap[next >> 3] |= (0x80 >> (next & 0x07));
		int i;
		for (i = 0;i < num_of_food;i++) {
				if (!(bitmap[i >> 3] & (0x80 >> (i & 0x07)))) {
					if (distance(aim[num_of_aim].pos, food[i].pos) < check) {
						aim[num_of_aim].pos = multiple(1 / (aim[num_of_aim].weight + food[i].weight),
							add(multiple(aim[num_of_aim].weight, aim[num_of_aim].pos), multiple(food[i].weight, food[i].pos)));
						aim[num_of_aim].weight += food[i].weight;
						bitmap[i >> 3] |= (0x80 >> (i & 0x07));
					}
				}
			}
		num_of_aim++;
	}
	qsort(aim, sizeof(point), num_of_aim, zw_cmp);
}
int zw_cmp(const void* p, const void* q) {
	Position my = GetStatus()->objects[0].pos;
	double dis1 = distance(my, ((point*)p)->pos);
	double dis2 = distance(my, ((point*)q)->pos);
	int w1 = (((point*)p)->weight);
	int w2 = (((point*)q)->weight);
	return w2 / dis2 - w1 / dis1;
}