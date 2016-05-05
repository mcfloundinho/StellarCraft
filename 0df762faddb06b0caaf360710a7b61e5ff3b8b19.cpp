#include <cstdio>
#include <cstring>
const int N = 10000 + 5;
char hash[N][9];
int tot, cnt[N], head[N], next[N];
int calc_hash(char *s){
	int key = 0, len = strlen(s);
	for (int i = 0; i < len; ++i) key += s[i] * (i + 1) % (N - 1) + 1;
	return key;
}
int create_hash(char *s){
	int key = calc_hash(s);
	memcpy(hash[++tot], s, sizeof s);
	next[tot] = head[key];
	head[key] = tot;
	return tot;
}
int find_hash(char *s){
	int key = calc_hash(s);
	for (int i = head[key]; i; i = next[i])
		if (!strcmp(s, hash[i])) return i;
	return 0;
}
int main(){
	int n, max = 0;
	scanf("%d", &n);
	char s[9], name[9];
	while (n--){
		scanf("\n%s", s);
		int hash_value = find_hash(s);
		if (!hash_value) hash_value = create_hash(s);
		if (++cnt[hash_value] > max){
			max = cnt[hash_value];
			memcpy(name, s, sizeof s);
		}
	}
	printf("%s %d\n", name, max);
	return 0;
}
