#include <cstdio>
#include <cstring>
#include <malloc.h>
const int N = 100000 + 5;
const int M = 1000000 + 5;
int n, m, es, a[M], b[M], price[M], head[N], indeg[N], dis[N], inq[N], last[N];
struct{
	int to, value, next;
}line[M];
void dfs(const int now){
	for (int i = head[now]; ~i; i = line[i].next){
		if (dis[now] + line[i].value <= dis[line[i].to]) continue;
		dis[line[i].to] = dis[now] + line[i].value;
		last[line[i].to] = now;
		dfs(line[i].to);
	}
}
void print(const int now){
	if (!now) return;
	print(last[now]);
	printf("%d\t", now);
}
void addedge(const int a, const int b, const int value){
	line[es].to = b;
	line[es].value = value;
	line[es].next = head[a];
	head[a] = es++;
	++indeg[b];
}
inline void swap(int &a, int &b){
	int tmp = a;
	a = b;
	b = tmp;
}
void qsort(const int l, const int r){
	int i = l, j = r, mida = a[(l + r) / 2], midb = b[(l + r) / 2];
	do{
		while (a[i] > mida || a[i] == mida && b[i] > midb) ++i;
		while (a[j] < mida || a[j] == mida && b[j] < midb) --j;
		if (i <= j){
			swap(a[i], a[j]);
			swap(b[i], b[j]);
			swap(price[i], price[j]);
			++i, --j;
		}
	}while (i <= j);
	if (i < r) qsort(i, r);
	if (l < j) qsort(l, j);
}
int main(){
	scanf("%d%d", &n, &m);
	memset(head, -1, sizeof head);
	for (int i = 0; i < m; ++i) scanf("%d%d%d", a + i, b + i, price + i);
	qsort(0, m - 1);
	for (int i = 0; i < m; ++i) addedge(a[i], b[i], price[i]);
	for (int i = 1; i <= n; ++i)
		if (!indeg[i]) dfs(i);
	int res = n;
	for (int i = 1; i < n; ++i)
		if (dis[i] > dis[res]) res = i;
	print(res);
	return 0;
}
