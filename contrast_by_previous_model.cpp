#include "teamstyle17.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#define MAX_SIZE 1000
#define WAIT  while(GetTime()==operate_time);
#define GO   (operate_time=GetTime());
#define NUM_OF_SOLUTION  3
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
	HIGHLY_ADVANCED_VALUE= 1000,
	MID_ADVANCED_VALUE= 500,
	LOW_ADVANCED_VALUE= 50,
	TRASH=0,
	RUN_VALUE=-10,
	CHASING_VALUE=30,
};
const double eps = 1e-6;
static PlayerObject me;
static Object opponent[kMaxPlayerObjectNumber];
static Object boss;
static int me_number;
static int opponent_number;
static int see_boss;
static int operate_time;

const Map *map;

void Init();
void Strategy();
int update();
Vector MaximumSpeed(Vector vec);
int cost(SkillType);
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
static int boss_eat;
static int devour_warning;
static double boss_r = (double)2000;
static Vector speed;
static Position last_pos;
static int AE_Parameter=HIGHLY_ADVANCED_VALUE;

void AIMain() {
	if (GetStatus() -> team_id) 
	{
		Vector speed2={100,100,100};
		Move(GetStatus()->objects[0].id,speed2);
		return;
	}
	srand(time(0));
	for(;;){
		//if (abs(GetTime()-1000)<=5 || me.health>10000)
		//printf("time=%d\thealth=%d\tability=%d\n",GetTime(),me.health,me.ability);
		Init();
		speed=MaximumSpeed(speed);
		Move(me.id,speed);
		Strategy();
	}
}

void Init(){
	map = GetMap();
	current_time = map -> time;
	register int i;
	register int AE_number;
	int AEflag;//记录有无AE可以走
	register int ChosenAE;
	register double MinAEdistance;
	Position AE[50];//记录AE的位置
	register int devour_number;
	Position Devour[20];//记录Devour的位置
	int Boss_Parameter=20;
	int Devour_Danger=0;
	register double border_r;
	register double F;
	register double dis;
	Vector a2;//从目标到自己的向量

	border_r=0.5*me.radius;
	me = GetStatus() -> objects[0];
	opponent_number = devour_number = see_boss = AE_number = 0;
	speed.x = speed.y = speed.z = (double)0;
	MinAEdistance=10000;

	for (i = 0; i < map -> objects_number; ++i)
	{
		F= (double)0, dis = dist(map -> objects[i].pos, me.pos);
		switch (map -> objects[i].type){
			case PLAYER:
				if (map -> objects[i].team_id == GetStatus() -> team_id) break;
				opponent[opponent_number++] = map -> objects[i];
				if (me.radius < map -> objects[i].radius) 
					F = RUN_VALUE * me.radius * map -> objects[i].radius / POW(dis, 3);
				if (me.radius > map -> objects[i].radius && me.skill_level[SHORT_ATTACK]
					|| me.skill_level[SHORT_ATTACK]>=5) 
						F = CHASING_VALUE * me.radius * map -> objects[i].radius / POW(dis, 3);
				break;
			case ENERGY:
				F = ENERGY_VALUE * POW(me.radius, 2) / POW(dis, 3);
				if (IsBorder(border_r,map -> objects[i].pos))//如果到边界的距离小于r，力为0
					F=0;
				break;
			case ADVANCED_ENERGY:
				AE[AE_number++]=map -> objects[i].pos;
				break;
			case DEVOUR:
				Devour[devour_number++]=map -> objects[i].pos;
				break;
			case BOSS:
				boss = map -> objects[i];
				see_boss = 1;
				boss_r = boss.radius;
				boss_warning = (int)(dis < 1.2 * boss_r && me.radius<boss_r);
				break;
			default:
				break;
		}
		//printf("F=%f\n",F);//F=0.000061~0.001110
		a2=Minus(map -> objects[i].pos,me.pos);
		speed=Add(speed,Multiple(F,a2));
	}
	if (AE_number)//处理AE
	{
		AEflag=0;
		for(i=0;i<AE_number;i++)
		{
			dis=dist(AE[i], me.pos);
			//printf("AE %d dis=%f\n",i,dis);
			if (IsBorder(border_r,AE[i]))
				continue;
			dis=dist(AE[i], me.pos);
			if (MinAEdistance>dis)
			{
				MinAEdistance=dis;
				ChosenAE=i;
				AEflag=1;
			}
		}
		if (AEflag)
		{
			/*if (me.skill_level[SHORT_ATTACK]>=5 && me.skill_level[HEALTH_UP]>=4)
				AE_Parameter=0;
			else
				AE_Parameter=100;*/
			a2=Minus(AE[ChosenAE],me.pos);
			F = AE_Parameter * POW(me.radius, 2) / POW(MinAEdistance, 3);
			speed=Add(speed,Multiple(F,a2));
			//printf("chosen ae= %d dis=%f\n",ChosenAE,MinAEdistance);
		}
	}

	if (me.radius>1.2*boss_r  && see_boss)//如果比boss大，去吃
	{
		a2=Minus(boss.pos,me.pos);
		//printf("eating boss!\n");
		F=CHASING_VALUE*10 * POW(me.radius, 2) / POW(length(a2), 3);
		speed=Add(speed,Multiple(F,a2));
	}

	FBorder(1.1*me.radius);//如果碰到边界，速度置0

	Devour_Danger=!(me.shield_time>15 && me.skill_level[SHIELD]==5);
	if (Devour_Danger)
	{
		for (i=0;i<devour_number;i++)//处理吞噬者
			if (IsDevour(1.1*me.radius,Devour[i]))//如果会碰到devour，速度正交化
			{
				a2=Minus(Devour[i],me.pos);
				//printf("devour!\n");
				speed=Schmidt(speed,a2);
			}
	}
	if (boss_warning && IsBoss(boss_r+me.radius,boss.pos))//如果碰到boss，正交化
	{
		a2=Minus(boss.pos,me.pos);
		//printf("boss!\n");
		speed=Schmidt(speed,a2);
	}
	speed.x *= 100 + rand() % 5;
	speed.y *= 100 + rand() % 5;
	speed.z *= 100 + rand() % 5;
}
void Strategy()
{
	if (opponent_number)
	{
		if (!opponent[0].shield_time)
		{
			if (dist(me.pos, opponent[0].pos) - me.radius - opponent[0].radius < kShortAttackRange[me.skill_level[SHORT_ATTACK]]) 
				short_attack(me);
			if (dist(me.pos, opponent[0].pos) - me.radius - opponent[0].radius < kLongAttackRange[me.skill_level[LONG_ATTACK]]) 
				long_attack(me, opponent[0]);
		}
		else shield(me);
	}
	/*if (see_boss && dist(me.pos, boss.pos) - me.radius - boss_r < kShortAttackRange[me.skill_level[SHORT_ATTACK]])
		short_attack(me);*///打boss
	update();
}
int update() {
	if (me.skill_cd[SHIELD] == 0 /*&& me.short_attack_casting == -1*/ && me.long_attack_casting == -1) {
		WAIT;
		Shield(me.id);
		GO;
		return 1;
	}
	else {
		if (me.skill_level[HEALTH_UP] < kMaxSkillLevel) {
			if (me.ability >= cost(HEALTH_UP)) {
				WAIT;
				UpgradeSkill(me.id, HEALTH_UP);
				GO;
				return 1;
			}
			else return 0;
		}
		else if (me.skill_level[SHIELD] < kMaxSkillLevel) {
			if (me.ability >= cost(SHIELD)) {
				WAIT;
				UpgradeSkill(me.id, SHIELD);
				GO;
				return 1;
			}
			else return 0;
		}
		else {
			AE_Parameter = MID_ADVANCED_VALUE;//1st step of update finish
			if (me.skill_level[SHORT_ATTACK] < kMaxSkillLevel) {
				if (me.skill_level[DASH] < kMaxSkillLevel) {
					if (me.ability >= cost(SHORT_ATTACK)) {
						WAIT;
						UpgradeSkill(me.id, SHORT_ATTACK);
						GO;
						return 1;
					}
					else if (me.ability >= cost(DASH)) {
						WAIT;
						UpgradeSkill(me.id, DASH);
						GO;
						return 1;
					}
					else return 0;
				}
				else {
					if (me.ability >= cost(SHORT_ATTACK)) {
						WAIT;
						UpgradeSkill(me.id, SHORT_ATTACK);
						GO;
						return 1;
					}
					else return 0;
				}
			}
			else if (me.skill_level[DASH] < kMaxSkillLevel) {
				if (me.ability >= cost(DASH)) {
					WAIT;
					UpgradeSkill(me.id, DASH);
					GO;
					return 1;
				}
				else return 0;
			}
			else {
				AE_Parameter = LOW_ADVANCED_VALUE;//2st step of update finish
				if (me.skill_level[LONG_ATTACK] < kMaxSkillLevel) {
					if (me.ability >= cost(LONG_ATTACK)) {
						WAIT;
						UpgradeSkill(me.id, LONG_ATTACK);
						GO;
						return 1;
					}
					else return 0;
				}
				else {
					AE_Parameter = TRASH;//all finish
					return 0;
				}
			}
			return 0;
		}
	}
}
int IsDevour(double d,Position des)//判断下一时刻会不会碰到吞噬者
{
	int flag=0;
	Position Next; 
	for(int i=1;i<=8;i++)
	{
		Next=Add(me.pos,Multiple(i,MaximumSpeed(speed)));
		if (dist(Next,des)<d)
			flag=1;
	}
	return flag;
}
int IsBoss(double d,Position des)//判断下一时刻会不会碰到boss
{
	int flag=0;
	Position Next=Add(me.pos,Multiple(5,MaximumSpeed(speed))); 
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
	register int flag = NONE,count=0;
	if (me.pos.x<r && speed.x<0) 
		speed.x=-0,flag=LEFT,count++;
	if (me.pos.x > kMapSize - r && speed.x>0) 
		speed.x=-0,flag=RIGHT,count++;
	if (me.pos.y<r && speed.y<0) 
		speed.y=-0,flag=BEHIND,count++;
	if (me.pos.y > kMapSize - r && speed.y>0) 
		speed.y=-0,flag=FRONT,count++;
	if (me.pos.z<r && speed.z<0) 
		speed.z=-0,flag=DOWN,count++;
	if (me.pos.z > kMapSize - r && speed.z>0) 
		speed.z=-0,flag=UP,count++;
	if (count>=2)
	{
		Vector a2=Minus(boss.pos,me.pos);
		speed=Add(speed,Multiple(0.0005,a2));
	}
	return flag;
}
Vector MaximumSpeed(Vector vec){
	register double len = length(vec);
	vec.x *= (kMaxMoveSpeed + kDashSpeed[me.skill_level[DASH]]) / len;
	vec.y *= (kMaxMoveSpeed + kDashSpeed[me.skill_level[DASH]]) / len;
	vec.z *= (kMaxMoveSpeed + kDashSpeed[me.skill_level[DASH]]) / len;
	return vec;
}
int cost(SkillType skill){
	if (me.skill_level[skill]) return kBasicSkillPrice[skill] << me.skill_level[skill];
	register int i, cnt = 0;
	for (i = 0; i < kSkillTypes; ++i)
		if (me.skill_level[i]) ++cnt;
	return kBasicSkillPrice[skill] << cnt;
}

void upgrade(PlayerObject obj, SkillType skill){
	if (obj.skill_level[skill] == 5) return;
	if (obj.ability < cost(skill)) return;
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

