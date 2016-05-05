#include <cstdio>
const int N = 1000000 + 5;
int n, m, l, r, reversed[N * 4], sum[N * 4];
char op;
void build(const int L = 1, const int R = n, const int k = 1){
	sum[k] = R - L + 1;
	if (L == R) return;
	int mid = (L + R) / 2;
	build(L, mid, k * 2);
	build(mid + 1, R, k * 2 + 1);
}
void pushdown(const int L, const int R, const int k){
	if (reversed[k]){
		reversed[k * 2] ^= 1;
		reversed[k * 2 + 1] ^= 1;
		int mid = (L + R) / 2;
		sum[k * 2] = mid - L + 1 - sum[k * 2];
		sum[k * 2 + 1] = R - mid - sum[k * 2 + 1];
		reversed[k] = 0;
	}
}
void reverse(const int l, const int r, const int L = 1, const int R = n, const int k = 1){
	if (l <= L && R <= r){
		reversed[k] ^= 1;
		sum[k] = R - L + 1 - sum[k];
		return;
	}
	pushdown(L, R, k);
	int mid = (L + R) / 2;
	if (l <= mid) reverse(l, r, L, mid, k * 2);
	if (r > mid) reverse(l, r, mid + 1, R, k * 2 + 1);
	sum[k] = sum[k * 2] + sum[k * 2 + 1];
}
int query(const int l, const int r, const int L = 1, const int R = n, const int k = 1){
	if (l <= L && R <= r) return sum[k];
	pushdown(L, R, k);
	int res = 0, mid = (L + R) / 2;
	if (l <= mid) res += query(l, r, L, mid, k * 2);
	if (r > mid) res += query(l, r, mid + 1, R, k * 2 + 1);
	return res;
}
int main(){
	scanf("%d%d", &n, &m);
	build();
	for (int i = 0; i < m; ++i){
		scanf("\n%c%d%d", &op, &l, &r);
		if (op == 'H') reverse(l, r);
		else printf("%d\n", query(l, r));
	}
	return 0;
}
