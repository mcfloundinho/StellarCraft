int initial(){
	me = GetStatus() -> objects[0];
	num_of_aim = num_of_food = num_of_devour = 0;
	emergency = code = 0;
	me_radius = me.radius;
	const Map *map = GetMap();
	int i = (*map).objects_number - 1;
	for (; ~i; --i){
		switch ((*map).objects[i].type){
			case PLAYER:
				if ((*map).objects[i].team_id == GetStatus() -> team_id) break;
				opponent_obj = (*map).objects[i];
				code ^= OPPONENT;
				break;
			case ENERGY:
				if ((*map).objects[i].pos.x - 0.75 * me_radius < 0) break;
				if ((*map).objects[i].pos.y - 0.75 * me_radius < 0) break;
				if ((*map).objects[i].pos.z - 0.75 * me_radius < 0) break;
				if ((*map).objects[i].pos.x + 0.75 * me_radius > kMapSize) break;
				if ((*map).objects[i].pos.y + 0.75 * me_radius > kMapSize) break;
				if ((*map).objects[i].pos.z + 0.75 * me_radius > kMapSize) break;
				food[num_of_food].weight = 1;
				food[num_of_food].pos = (*map).objects[i].pos;
				++num_of_food;
				break;
			case ADVANCED_ENERGY:
				if (distance((*map).objects[i].pos, {0, 0, 0}) < 0.75 * me_radius) break;
				if (distance((*map).objects[i].pos, {0, 0, kMapSize}) < 0.75 * me_radius) break;
				if (distance((*map).objects[i].pos, {0, kMapSize, 0}) < 0.75 * me_radius) break;
				if (distance((*map).objects[i].pos, {0, kMapSize, kMapSize}) < 0.75 * me_radius) break;
				if (distance((*map).objects[i].pos, {kMapSize, 0, 0}) < 0.75 * me_radius) break;
				if (distance((*map).objects[i].pos, {kMapSize, 0, kMapSize}) < 0.75 * me_radius) break;
				if (distance((*map).objects[i].pos, {kMapSize, kMapSize, 0}) < 0.75 * me_radius) break;
				if (distance((*map).objects[i].pos, {kMapSize, kMapSize, kMapSize}) < 0.75 * me_radius) break;
				food[num_of_food].weight = ad_weight;
				food[num_of_food].pos = (*map).objects[i].pos;
				++num_of_food;
				break;
			case SOURCE: break;
			case DEVOUR:
				devour[num_of_devour] = (*map).objects[i].pos;
				++num_of_devour;
				break;
			case BOSS:
				boss_obj = (*map).objects[i];
				code ^= SEE_BOSS;
				break;
			default: break;
		}
	}
	return code;
}
int boss(){
	if (boss_radius < me_radius * kEatableRatio){
		solution[SEE_BOSS].weight = 100;
		solution[SEE_BOSS].pos = boss_obj.pos;
		return 0;
	}
	else{
		emergency = me_radius < boss_radius * kEatableRatio ? 1 : 0;
		int tmp = short_attack(boss_obj);
		if (!~tmp) tmp = long_attack(boss_obj);
		return (~tmp) ? 1 : 0;
	}
}