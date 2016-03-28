﻿#include "teamstyle17.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
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
const double eps = 1e-6;

static PlayerObject me[kMaxPlayerObjectNumber];
static Object opponent[kMaxPlayerObjectNumber];
static Object boss;
static int me_number;
static int opponent_number;
static int devour_number;
static int see_boss;

const Map *map;

void Init();
void Strategy();
Vector MaximumSpeed(Vector vec);
int cost(PlayerObject, SkillType);
void upgrade(PlayerObject, SkillType);
void long_attack(PlayerObject, Object);
void short_attack(PlayerObject);
void shield(PlayerObject);
void dash(PlayerObject);
int IsBorder(double d,Position des);
int IsDevour(double d,Position des);
int IsBoss(double d,Position des);
int FBorder(double r);


double dist(Position, Position);
double length(Vector);
double dot_product(Vector, Vector);
double ABS(double);
double SQR(double);
double POW(double, int);
Vector Add(Vector a1,Vector a2);
Vector Minus(Vector a1,Vector a2);
Vector Multiple(double k,Vector a);
Vector Schmidt(Vector a1,Vector a2);

static int current_time;
static int border;
static int boss_warning;
static int devour_warning;
static double boss_r = (double)2000;
static Vector speed;
static Position last_pos;
static Position Devour[10];//记录Devour的位置

void AIMain() {
	if (GetStatus() -> team_id) return;
	srand(time(0));
	for(;;){
		printf("time=%d\thealth=%d\tability=%d\n",GetTime(),me[0].health,me[0].ability);
		Init();
		speed=MaximumSpeed(speed);
		Move(me[0].id,speed);
		//Strategy();
	}
}

void Init(){
	map = GetMap();
	current_time = map -> time;
	register int i;
	register double border_r=0.5*me[0].radius;
	me[0] = GetStatus() -> objects[0];
	opponent_number = devour_number = see_boss = 0;
	speed.x = speed.y = speed.z = (double)0;

	for (i = 0; i < map -> objects_number; ++i)
	{
		register double F = (double)0, dis = dist(map -> objects[i].pos, me[0].pos);
		switch (map -> objects[i].type){
			case PLAYER:
				if (map -> objects[i].team_id == GetStatus() -> team_id) break;
				opponent[opponent_number++] = map -> objects[i];
				/*if (me[0].radius < map -> objects[i].radius) 
					F = -10 * me[0].radius * map -> objects[i].radius / POW(dis, 3);
				if (me[0].radius > map -> objects[i].radius) 
					F = 30 * me[0].radius * map -> objects[i].radius / POW(dis, 3);*/
				break;
			case ENERGY:
				F = 5 * POW(me[0].radius, 2) / POW(dis, 3);
				if (IsBorder(border_r,map -> objects[i].pos))//如果到边界的距离小于r，力为0
					F=0;
				break;
			case ADVANCED_ENERGY:
				F = 8 * POW(me[0].radius, 2) / POW(dis, 3);
				if (IsBorder(border_r,map -> objects[i].pos))//如果到边界的距离小于r，力为0
					F=0;
				break;
			case DEVOUR:
				Devour[devour_number]=map -> objects[i].pos;
				devour_number++;
				break;
			case BOSS:
				boss = map -> objects[i];
				see_boss = 1;
				boss_r = boss.radius;
				boss_warning = (int)(dis < 1.2 * boss_r);
				break;
			default:
				break;
		}
		Vector a2=Minus(map -> objects[i].pos,me[0].pos);
		speed=Add(speed,Multiple(F,a2));
	}
	speed.x *= 100 + rand() % 10;
	speed.y *= 100 + rand() % 10;
	speed.z *= 100 + rand() % 10;
	FBorder(1.1*me[0].radius);//如果碰到边界，速度置0
	for (i=0;i<devour_number;i++)
	{
		if (IsDevour(1.1*me[0].radius,Devour[i]))//如果会碰到devour，速度正交化
		{
			Vector a2=Minus(Devour[i],me[0].pos);
			//printf("prespeed=%.2f %.2f %.2f\n",speed.x,speed.y,speed.z);
			speed=Schmidt(speed,a2);
			//printf("currentspeed=%.2f %.2f %.2f\n",speed.x,speed.y,speed.z);
		}
	}
	if (boss_warning && IsBoss(1.5*boss_r,boss.pos))
	{
		Vector a2=Minus(boss.pos,me[0].pos);
		speed=Schmidt(speed,a2);
	}
}
void Strategy()
{
	if (opponent_number)
	{
		if (!opponent[0].shield_time)
		{
			if (dist(me[0].pos, opponent[0].pos) - me[0].radius - opponent[0].radius < kShortAttackRange[me[0].skill_level[SHORT_ATTACK]]) 
				short_attack(me[0]);
			if (dist(me[0].pos, opponent[0].pos) - me[0].radius - opponent[0].radius < kLongAttackRange[me[0].skill_level[LONG_ATTACK]]) 
				long_attack(me[0], opponent[0]);
		}
		else shield(me[0]);
	}
	upgrade(me[0], SHORT_ATTACK);
	upgrade(me[0], HEALTH_UP);
	if (me[0].skill_level[SHORT_ATTACK] >= me[0].skill_level[LONG_ATTACK])
		upgrade(me[0], LONG_ATTACK);
	if (!me[0].skill_level[SHIELD]) upgrade(me[0], SHIELD);
}
int IsDevour(double d,Position des)//判断下一时刻会不会碰到吞噬者
{
	int flag=0;
	Position Next=Add(me[0].pos,MaximumSpeed(speed)); 
	if (dist(Next,des)<d)
		flag=1;
	return flag;
}
int IsBoss(double d,Position des)
{
	int flag=0;
	Position Next=Add(me[0].pos,MaximumSpeed(speed)); 
	if (dist(Next,des)<d)
		flag=1;
	return flag;
}
int IsBorder(double d,Position des)//判断物体是否在边界旁
{
	int R = NONE;
	if (des.x < d) R |= LEFT;
	else if (kMapSize - des.x < d) R |= RIGHT;
	if (des.y < d) R |= BEHIND;
	else if (kMapSize - des.y < d) R |= FRONT;
	if (des.z < d) R |= DOWN;
	else if (kMapSize - des.z < d) R |= UP;
	return R;
}
int FBorder(double r)//判断下一时刻会不会碰到边界，如果会，速度置0；
{
	register int flag = NONE;
	if (me[0].pos.x<r && speed.x<0) 
		speed.x=-0,flag=LEFT;
	if (me[0].pos.x > kMapSize - r && speed.x>0) 
		speed.x=-0,flag=RIGHT;
	if (me[0].pos.y<r && speed.y<0) 
		speed.y=-0,flag=BEHIND;
	if (me[0].pos.y > kMapSize - r && speed.y>0) 
		speed.y=-0,flag=FRONT;
	if (me[0].pos.z<r && speed.z<0) 
		speed.z=-0,flag=DOWN;
	if (me[0].pos.z > kMapSize - r && speed.z>0) 
		speed.z=-0,flag=UP;
	return flag;
}
Vector MaximumSpeed(Vector vec){
	register double len = length(vec);
	vec.x *= (kMaxMoveSpeed + kDashSpeed[me[0].skill_level[DASH]]) / len;
	vec.y *= (kMaxMoveSpeed + kDashSpeed[me[0].skill_level[DASH]]) / len;
	vec.z *= (kMaxMoveSpeed + kDashSpeed[me[0].skill_level[DASH]]) / len;
	return vec;
}
int cost(PlayerObject obj, SkillType skill){
	if (obj.skill_level[skill]) return kBasicSkillPrice[skill] << obj.skill_level[skill];
	register int i, cnt = 0;
	for (i = 0; i < kSkillTypes; ++i)
		if (obj.skill_level[i]) ++cnt;
	return kBasicSkillPrice[skill] << cnt;
}

void upgrade(PlayerObject obj, SkillType skill){
	if (obj.skill_level[skill] == 5) return;
	if (obj.ability < cost(obj, skill)) return;
	UpgradeSkill(obj.id, skill);
}

void long_attack(PlayerObject obj, Object target){
	if (!obj.skill_level[LONG_ATTACK]) return;
	if (obj.skill_cd[LONG_ATTACK]) return;
	if (dist(obj.pos, target.pos) - obj.radius - target.radius > kLongAttackRange[obj.skill_level[LONG_ATTACK]]) 
		return;
	LongAttack(obj.id, target.id);
}

void short_attack(PlayerObject obj){
	if (!obj.skill_level[SHORT_ATTACK]) return;
	if (obj.skill_cd[SHORT_ATTACK]) return;
	ShortAttack(obj.id);
}

void shield(PlayerObject obj){
	if (!obj.skill_level[SHIELD]) return;
	if (obj.skill_cd[SHIELD]) return;
	Shield(obj.id);
}

void dash(PlayerObject obj){
	if (!obj.skill_level[DASH]) return;
	if (obj.skill_cd[DASH]) return;
	Dash(obj.id);
}

double dist(Position src, Position des){
	register Position vec;
	vec.x = des.x - src.x;
	vec.y = des.y - src.y;
	vec.z = des.z - src.z;
	return length(vec);
}

double length(Vector vec){
	return sqrt(dot_product(vec, vec));
}

double dot_product(Vector vec1, Vector vec2){
	return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z;
}

double ABS(double x){
	return x > 0 ? x : -x;
}

double SQR(double x){
	return x * x;
}

double POW(double x, int a){
	if (ABS(x) < eps) x = 0.1;
	register double res = (double)1;
	for (; a; a >>= 1, x = SQR(x))
		if (a & 1) res *= x;
	return res;
}
Vector Add(Vector a1,Vector a2){
	Vector temp;
	temp.x=a1.x+a2.x;
	temp.y=a1.y+a2.y;
	temp.z=a1.z+a2.z;
	return temp;
}
Vector Minus(Vector a1,Vector a2)//a1-a2
{
	Vector temp;
	temp.x=a1.x-a2.x;
	temp.y=a1.y-a2.y;
	temp.z=a1.z-a2.z;
	return temp;
}
Vector Multiple(double k,Vector a)
{
	Vector temp;
	temp.x=k*a.x;
	temp.y=k*a.y;
	temp.z=k*a.z;
	return temp;
}
Vector Schmidt(Vector a1,Vector a2)//由a1生成一个与a2垂直的向量 
{
	Vector temp1,temp2;
	temp2=Multiple(dot_product(a1,a2)/dot_product(a2,a2),a2); 
	temp1=Minus(a1,temp2);
	return temp1;
}