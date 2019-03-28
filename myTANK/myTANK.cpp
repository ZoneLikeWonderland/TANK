#include<iostream>
#include<string>
#include<cstring>
#include<queue>
#include<bitset>
#include"jsoncpp/json.h"

//从0到n-1循环
#define rep(i,n) for(unsigned int i=0;i<(unsigned int)n;++i)

using namespace std;
using Json::Reader;
using Json::Value;

const int N = 9;
int MYSIDE;

Reader R;

#ifdef _BOTZONE_ONLINE
Json::FastWriter Writer;
#else
Json::StyledWriter Writer;
#endif

//储存地图中坦克id
int MAPid[N][N];

//方向数组
int direction[4][2] = { {-1,0},{0,1},{1,0},{0,-1} };

//地图状态枚举
enum mapstatus {
	air = 0,
	tank, brick, base, steal, tobedestoryed
}MAP[N][N];

//动作枚举
enum action {
	stay = -1,
	W, D, S, A,
	Ws, Ds, Ss, As
};

//储存坦克的对象
class TANK {
public:
	int x, y;
	action last;
	bool shootable;
	//0蓝 1红
	//  ========= 
	//0   0   1   0
	//1   1   0   8
	//  =========
	//  012345678 
	TANK() {
		alive = 1;
		x = y = -1;
		shootable = true;
	}
	TANK(int side, int num) :TANK() {
		if (side == 0) {
			if (num == 0)x = 0, y = 2;
			else x = 0, y = 6;
		}
		else {
			if (num == 0)x = 8, y = 6;
			else x = 8, y = 2;
		}
		MAP[x][y] = tank;
	}
	bool isalive() {
		return alive;
	}
	void die() {
		alive = 0;
	}
	void showpos() {
		cout << x << ',' << y << endl;
	}
	void move(action X) {
		if (X == stay)return;
		switch (X) {
		case W:
			x -= 1;
			break;
		case A:
			y -= 1;
			break;
		case S:
			x += 1;
			break;
		case D:
			y += 1;
			break;
		default:
			break;
		}
	}
private:
	int alive;
};

//四个坦克的对象
TANK alltank[4];

//读取Json
inline Value read() {
	Value v;
	string s;
	getline(cin, s);
	R.parse(s, v);
	return v;
}

//初始化
inline void init() {
	rep(i, N)
		rep(j, N)
		MAP[i][j] = air;
	Value v = read();
	v = v["requests"][0u];
	//map init
	Value mapints = v["field"];
	int t = 0;
	rep(i, N) {
		if (i % 3 == 0)t = mapints[i / 3].asInt();
		rep(j, N) {
			if (t & 1)MAP[i][j] = brick;
			t >>= 1;
		}
	}
	MAP[0][4] = MAP[8][4] = base;
	MAP[1][4] = MAP[7][4] = steal;

	//tank init
	MYSIDE = v["mySide"].asInt();
	TANK a(MYSIDE, 0), b(MYSIDE, 1);
	TANK c(1 - MYSIDE, 0), d(1 - MYSIDE, 1);
	alltank[0] = a;
	alltank[1] = b;
	alltank[2] = c;
	alltank[3] = d;

}

//判断一个动作是移动动作
inline bool ismove(action x) {
	if (x >= stay && x < Ws)return true;
	return false;
}

//通过坦克状态更新地图数据
inline void updatemap() {
	rep(i, N)
		rep(j, N)
		if (MAP[i][j] == tank)MAP[i][j] = air;
	rep(i, N)
		rep(j, N)
		MAPid[i][j] = 0;

	rep(i, 4)
		if (alltank[i].isalive())
			MAP[alltank[i].x][alltank[i].y] = tank,
			MAPid[alltank[i].x][alltank[i].y] |= (1 << i);
}

//获取dir动作指向的第一个物体
inline mapstatus getnextobj(int x, int y, action dir, int &posx, int &posy) {
	do {
		x += direction[dir - 4][0];
		y += direction[dir - 4][1];
	} while (x >= 0 && x <= N - 1 && y >= 0 && y <= N - 1 && MAP[x][y] == air);
	if (!(x >= 0 && x <= N - 1 && y >= 0 && y <= N - 1))return steal;
	posx = x;
	posy = y;
	return MAP[x][y];
}

//判断MAP上是否为单个坦克
inline bool singletank(int x, int y) {
	if ((MAPid[x][y] & 1) + ((MAPid[x][y] >> 1) & 1) + ((MAPid[x][y] >> 2) & 1) + ((MAPid[x][y] >> 3) & 1) == 1)
		return true;
	return false;
}

//获取MAPid中的坦克id
inline int gettankid(int x) {
	int ans = -1;
	while (x)
		x >>= 1, ans += 1;
	return ans;
}

//更新战局到最新状态
inline void update() {
	Value v = read();
	alltank[2].last = (action)v[0u].asInt();
	alltank[3].last = (action)v[1u].asInt();

	rep(i, 4)
		if (alltank[i].isalive() && ismove(alltank[i].last))
			alltank[i].move(alltank[i].last),
			alltank[i].shootable = true;

	updatemap();

	int check[4] = { 0,0,0,0 };
	rep(i, 4)
		if (!alltank[i].isalive() || check[i] || ismove(alltank[i].last))continue;
		else {
			alltank[i].shootable = false;
			int x, y;
			mapstatus next = getnextobj(alltank[i].x, alltank[i].y, alltank[i].last, x, y);
			switch (next) {
			case tobedestoryed:
				break;
			case air:
				break;
			case brick:
				MAP[x][y] = tobedestoryed;
				break;
			case tank:
				if (singletank(x, y) && singletank(alltank[i].x, alltank[i].y))
					if (!ismove(alltank[gettankid(MAPid[x][y])].last))
						if (abs((int)alltank[gettankid(MAPid[x][y])].last - (int)alltank[i].last) == 2)
							break;
				MAP[x][y] = tobedestoryed;
				break;
			default:
				break;
			}

			check[i] = 1;
		}
	rep(i, N)
		rep(j, N)
		if (MAP[i][j] == tobedestoryed) {
			MAP[i][j] = air;
			int p = 1;
			rep(k, 4) {
				if (MAPid[i][j] & p)alltank[k].die();
				p <<= 1;
			}

		}
	updatemap();
}

//书写Json
inline void write(action a, action b) {
	Value response(Json::objectValue), out(Json::arrayValue);
	out[0u] = a;
	out[1u] = b;
	response["response"] = out;
	cout << Writer.write(response) << endl;
}

//提交并保存动作
inline void submit(action a, action b) {
	write(a, b);
#ifdef _BOTZONE_ONLINE
	cout << ">>>BOTZONE_REQUEST_KEEP_RUNNING<<<" << endl;
#endif
	alltank[0].last = a;
	alltank[1].last = b;
}


//打印场地状态
inline void printfield() {
	rep(i, N) {
		rep(j, N)
			switch (MAP[i][j]) {
			case tank:
				cout << 'T';
				break;
			case air:
				cout << ' ';
				break;
			case brick:
				cout << '#';
				break;
			case base:
				cout << '@';
				break;
			case steal:
				cout << '$';
				break;
			default:
				cout << ' ';
			}
		cout << endl;
	}
}

void solve(action &A0, action &A1);

//标准流程：
//
//第一回合read
//第一回合write
//						server处理
//=====================================
//						server返回上局对方行动
//第二回合read
//第二回合决策
//第二回合write
//						server处理
//=====================================
//……
////////////////////////////////////////////////////
//实际流程：
//
//第一回合read
//第一回合write
//						server处理
//						server返回上局对方行动
//第二回合read
//=====================================
//第二回合决策	
//第二回合write->submit
//						server处理
//						server返回上局对方行动
//第三回合read->update
//=====================================
//……


//战略部分
bitset<N> shootstatus;
int staytime[2];

int main() {
	srand(time(nullptr));
	init();
	updatemap();
	while (1) {

#ifdef _BOTZONE_ONLINE

#else
		cout << "=========\n";
		printfield();
		rep(i, 4)cout << i << ":", alltank[i].showpos();
		cout << "=========\n";
#endif


		action A0 = stay, A1 = stay;
		solve(A0, A1);
		submit(A0, A1);
		update();

		shootstatus <<= 1;
		shootstatus|= (ismove(alltank[2].last) ? 0 : 1);
shootstatus|= (ismove(alltank[3].last) ? 0 : 1);

	}

	return 0;
}

inline bool istrap() {
	if (shootstatus.count() == N)return true;
	return false;
}

inline bool inmap(int x, int y) {
	if (x >= 0 && x < N&&y >= 0 && y < N)return true;
	return false;
}

//获取下一个相邻格子
inline mapstatus getnext(int id, action dir) {
	if (alltank[id].x + direction[dir][0] < 0 || alltank[id].x + direction[dir][0] >= N
		|| alltank[id].y + direction[dir][1] < 0 || alltank[id].y + direction[dir][1] >= N)return steal;
	return MAP[alltank[id].x + direction[dir][0]][alltank[id].y + direction[dir][1]];
}

//A*

class Astar {
public:
	Astar() {
		x = y = 0;
		g = h = 100000;
		f = 0;
		dadx = dady = 0;
		dadmove = stay;
		isclosed = false;
	}
	int x, y;
	int g, h;
	int f;
	int dadx, dady;
	action dadmove;
	friend bool operator<(Astar a, Astar b) {
		a.f = a.g + a.h;
		b.f = b.g + b.h;
		return a.f > b.f;
	}
	bool isclosed;
};

inline int eval(Astar p) {
	int ans = 0;
	if (MYSIDE == 0) {
		ans = (8 - p.x) * 7;
		
	}
	else {
		ans = (p.x) * 7;

	}
	ans += abs(p.y - 4);
	return ans;
}

inline int weight(int x, int y) {
	if (MAP[x][y] == air)return 10;
	if (MAP[x][y] == tank)return 10;
	return 20;
}

action globalastar[2];
bool globalinway[N][N];

inline void printway() {
	rep(i, N) {
		rep(j, N)
			if (globalinway[i][j])cout << 'x';
			else cout << '.';
		cout << endl;
	}
}

inline bool kexing(Astar p) {
	if (p.h > 3)return false;
	
	int x = p.x;
	int y = p.y;
	while (MAP[x][y] == air) {
		y += (y < 4 ? 1 : -1);
	}
	if (abs(y - 4) > 1)return false;

	return true;
}

inline void search(int id) {
	priority_queue<Astar> searchlist;
	//bool isclose[N][N];
	//rep(i, N)rep(j, N)isclose[i][j] = false;

	Astar node[N][N];
	rep(i, N)rep(j, N)node[i][j].x = i, node[i][j].y = j, node[i][j].g = 100000;
	node[alltank[id].x][alltank[id].y].h = eval(node[alltank[id].x][alltank[id].y]);
	node[alltank[id].x][alltank[id].y].g = 0;
	node[alltank[id].x][alltank[id].y].dadmove = stay;
	searchlist.push(node[alltank[id].x][alltank[id].y]);

	while (!searchlist.empty()) {
		Astar now = searchlist.top();
		searchlist.pop();
		//bool flag = 0;
		if (kexing(now)) {
			/*
			while(1){
				cout<<now.x<<','<<now.y<<':'<<now.g<<' '<<now.h<<endl;
				getchar();
				now = searchlist.top();
				searchlist.pop();
			}
			*/
			//cout << "AAAA" << endl;
			int x = now.x, y = now.y;
			while (node[x][y].dadmove != stay) {
				//cout<<node[x][y].x<<','<<node[x][y].y<<':'<<node[x][y].dadmove<<endl;
				globalinway[x][y] = true;
				globalastar[id] = node[x][y].dadmove;
				int x0, y0;
				x0 = node[x][y].dadx;
				y0 = node[x][y].dady;
				x = x0;
				y = y0;
			}
			//cout << globalastar[id] << endl;
			//flag = 1;	
			break;
		}
		//cout << now.x << ',' << now.y << endl;
		//isclose[now.x][now.y] = true;
		node[now.x][now.y].isclosed = true;
		rep(i, 4) {
			Astar temp = now;
			if (temp.x + direction[i][0] < 0 || temp.x + direction[i][0] >= N
				|| temp.y + direction[i][1] < 0 || temp.y + direction[i][1] >= N)continue;
			if (node[temp.x + direction[i][0]][temp.y + direction[i][1]].isclosed)continue;
			if (MAP[temp.x + direction[i][0]][temp.y + direction[i][1]] != air
				&& MAP[temp.x + direction[i][0]][temp.y + direction[i][1]] != brick
				&& MAP[temp.x + direction[i][0]][temp.y + direction[i][1]] != tank
				)continue;
			
			/*if ((temp.x + direction[i][0] == alltank[2].x + (MYSIDE == 0 ? -1 : 1) 
				&& temp.y + direction[i][1] == alltank[2].y
				|| temp.x + direction[i][0] == alltank[3].x + (MYSIDE == 0 ? -1 : 1) 
				&& temp.y + direction[i][1] == alltank[3].y)
				&& MAP[temp.x + direction[i][0]][temp.y + direction[i][1]] == brick)
				continue;*/
			
			temp.dadx = temp.x;
			temp.dady = temp.y;
			temp.dadmove = (action)i;
			temp.x += direction[i][0];
			temp.y += direction[i][1];
			temp.g += weight(temp.x, temp.y);
			
			//if (i % 2 == 0)temp.g -= 5;
			
			temp.h = eval(temp);

			if (temp.g < node[temp.x][temp.y].g) {
				node[temp.x][temp.y] = temp,
					searchlist.push(temp);
				//cout<<temp.x<<','<<temp.y<<':'<<temp.g<<','<<temp.h<<endl;

			}
		}
		//if (flag)break;
	}

}

//碰到底了吗？
inline bool reachbtm(int id) {
	if (MYSIDE == 0) {
		if (id & 2) {
			//敌方坦克
			if (alltank[id].x == 0)return true;
		}
		else {
			//我方坦克
			if (alltank[id].x == 8)return true;
		}
	}
	else {
		if (id & 2) {
			//敌方坦克
			if (alltank[id].x == 8)return true;
		}
		else {
			//我方坦克
			if (alltank[id].x == 0)return true;
		}
	}
	return false;
}

//迎敌状态
inline bool indanger(int id, action &ans) {
	int dx, dy;
	rep(i, 4) {
		if (getnextobj(alltank[id].x, alltank[id].y, (action)(i + 4), dx, dy) == tank
			&& gettankid(MAPid[dx][dy]) != 1 - id
			&& alltank[gettankid(MAPid[dx][dy])].shootable) {
			if (reachbtm(3 - id)
				&& (alltank[3 - id].y == 2 && getnext(3 - id, A) == air
					|| alltank[3 - id].y == 6 && getnext(3 - id, D) == air)) {
				return false;
			}
			if (alltank[id].shootable && !reachbtm(id)) {
				if (globalastar[id] % 2 == i % 2)
					//&& MAP[alltank[id].x + (MYSIDE == 0 ? 1 : -1)][alltank[id].y] != air)
					ans = (action)(i + 4);
				else
					if (globalastar[id] % 2 != i % 2 && getnext(id, globalastar[id]) == air)
						ans = globalastar[id];
					else {
						ans = (action)(i + 4);
					}
			}
			else {
				if (!reachbtm(id)&&globalastar[id] % 2 != i % 2 && getnext(id, globalastar[id]) == air)
					ans = globalastar[id];
				else if (inmap(alltank[id].x + direction[(i + 1) % 4][0], alltank[id].y + direction[(i + 1) % 4][1])
					&& MAP[alltank[id].x + direction[(i + 1) % 4][0]][alltank[id].y + direction[(i + 1) % 4][1]] == air) {
					ans = (action)((i + 1) % 4);
				}
				else if (inmap(alltank[id].x + direction[(i + 3) % 4][0], alltank[id].y + direction[(i + 3) % 4][1])
					&& MAP[alltank[id].x + direction[(i + 3) % 4][0]][alltank[id].y + direction[(i + 3) % 4][1]] == air) {
					ans = (action)((i + 3) % 4);
				}
				else
					ans = (action)(i + 4);
			}
#ifndef _BOTZONE_ONLINE
			cout << id << " is in danger" << endl;
#endif
			return true;
		}
	}
	return false;
}

//被夹逼即将迎敌状态
inline bool invalley(int id) {
	if (MAP[alltank[id].x][alltank[id].y - 1] == brick
		&& MAP[alltank[id].x][alltank[id].y + 1] == brick
		&& MAP[alltank[id].x + (MYSIDE == 0 ? 1 : -1)][alltank[id].y] == brick) {

		if (MAP[alltank[id].x + (MYSIDE == 0 ? 2 : -2) - 1][alltank[id].y] == tank
			|| MAP[alltank[id].x + (MYSIDE == 0 ? 2 : -2) + 1][alltank[id].y] == tank
			|| MAP[alltank[id].x + (MYSIDE == 0 ? 2 : -2)][alltank[id].y - 1] == tank
			|| MAP[alltank[id].x + (MYSIDE == 0 ? 2 : -2)][alltank[id].y + 1] == tank
			|| MAP[alltank[id].x + (MYSIDE == 0 ? 2 : -2)][alltank[id].y] == tank)
#ifndef _BOTZONE_ONLINE
			cout << id << " is invalley" << endl;
#endif
		return true;
	}
	return false;
}

inline bool havedanger(int id, action &shoot) {
	int dx, dy;
	int dx2, dy2;
	if (getnextobj(alltank[id].x, alltank[id].y, shoot, dx, dy) != brick)return false;
	if (getnextobj(dx, dy, shoot, dx2, dy2) == tank) return true;
	int x = dx;
	int y = dy;
	x += direction[shoot - 4][0];
	y += direction[shoot - 4][1];
	while (inmap(x,y)&&MAP[x][y] == air) {
		if (MAP[x+direction[(shoot + 1) % 4][0]][y + direction[(shoot + 1) % 4][1]]==tank
			|| MAP[x + direction[(shoot + 3) % 4][0]][y + direction[(shoot + 3) % 4][1]] == tank)
			return true;

	x += direction[shoot - 4][0];
	y += direction[shoot - 4][1];
	}
	return false;
}

//别开枪
inline void dontshoot(int id, action &x) {
	if (ismove(x))return;
	int dx, dy,dx2,dy2;
	if (getnextobj(alltank[id].x, alltank[id].y, x, dx, dy) == brick
		&& havedanger(id,x)) {
		if (getnext(id, (action)(x % 4)) == air) {
			x = (action)(x % 4);
		}
		else {
			x = stay;
		}
#ifndef _BOTZONE_ONLINE
		cout << id << " dont shoot" << endl;
#endif
	}
	//else if (getnextobj(alltank[id].x, alltank[id].y, x, dx, dy) == brick
	//	&& getnextobj(dx, dy, x, dx2, dy2) == tank)
	//	//一面墙
	//	x = stay;
}

//推进状态
inline void push(int id, action &x) {
	x = stay;
	int dx, dy;
	int dx2, dy2;
	if (!reachbtm(id))
	{
		if (getnextobj(alltank[id].x, alltank[id].y, (action)(globalastar[id] + 4), dx, dy) == brick
			&& globalinway[dx][dy]
			&& getnextobj(dx, dy, (action)(globalastar[id] + 4), dx2, dy2) != tank
			&& alltank[id].shootable)
			x = (action)(globalastar[id] + 4);
		else if (getnext(id, globalastar[id]) == air)
			x = globalastar[id];
		else
			x = (action)(globalastar[id] + 4);
		dontshoot(id, x);
	}
	else
		//拆基地
		if (alltank[id].y < 4) {
			if (alltank[id].shootable)x = Ds;
			else if (MAP[alltank[id].x][alltank[id].y + 1] == air) {
				x = D;
			}
		}
		else {
			if (alltank[id].shootable)x = As;
			else if (MAP[alltank[id].x][alltank[id].y - 1] == air) {
				x = A;
			}
		}
	if (!alltank[id].shootable && !ismove(x))x = stay;
#ifndef _BOTZONE_ONLINE
	cout << id << " is pushing" << endl;
#endif
}

//坦克重叠
inline bool superpos(int id) {
	if (singletank(alltank[id].x, alltank[id].y))return false;
	return true;
}

//处理重叠情况
inline void dealwithsuperpos(int id, action &x) {
	int dx, dy;
	if (!alltank[gettankid(MAPid[alltank[id].x][alltank[id].y] - (1 << id))].shootable
		&&getnextobj(alltank[id].x+(MYSIDE==0?1:-1), alltank[id].y, (alltank[id].y < 4 ? Ds:As),dx,dy)==brick) {
		x = (MYSIDE == 0 ? S : W);
	}
	/*else if (!alltank[id].shootable || alltank[id].last != stay) {
		x = stay;
	}
	else {
		if (rand() % 2)return;
		if (MYSIDE == 0)x = S;
		else x = W;
	}*/
	else {
		if (staytime[id] >= N + 2)
			if (istrap()) {
				x = (MYSIDE == 0 ? W : S);
			}
			else {

			}		
		else {
			staytime[id]++;
			x = stay;
		}
	}
}

//到达中线前的困境了吗
inline bool beforemid(int id) {
	if (alltank[id].x == (MYSIDE == 0 ? 3 : 5))
		if (MAP[alltank[id].x + (MYSIDE == 0 ? 2 : -2)][alltank[id].y] == tank
			|| MAP[alltank[id].x + (MYSIDE == 0 ? 3 : -3)][alltank[id].y] == tank
			|| MAP[alltank[id].x + (MYSIDE == 0 ? 2 : -2)][alltank[id].y - 1] == tank
			|| MAP[alltank[id].x + (MYSIDE == 0 ? 2 : -2)][alltank[id].y + 1] == tank) {
#ifndef _BOTZONE_ONLINE
			cout << id << " is dealing with mid" << endl;
#endif
			return true;
		}
	return false;
}

//左右徘徊？
inline void wander(int id, action &x) {
	int dx, dy;
	
	if ( MAP[alltank[id].x + (MYSIDE == 0 ? 2 : -2)][alltank[id].y - 1] == tank
		&&!superpos(2)
		&&gettankid(MAPid[alltank[id].x + (MYSIDE == 0 ? 2 : -2)][alltank[id].y - 1])==3-id) {
		if (MAP[alltank[id].x][alltank[id].y - 1] == air)
			x = A;
		else
			x = As;
	}
	else if (MAP[alltank[id].x + (MYSIDE == 0 ? 2 : -2)][alltank[id].y + 1] == tank
		&& !superpos(2)
		&& gettankid(MAPid[alltank[id].x + (MYSIDE == 0 ? 2 : -2)][alltank[id].y + 1]) == 3 - id) {
		if (MAP[alltank[id].x][alltank[id].y + 1] == air)
			x = D;
		else
			x = Ds;
	}
	else if (getnextobj(alltank[id].x, alltank[id].y, (alltank[id].y < 4 ? Ds : As), dx, dy) == brick && dy >= 1 && dy <= 7) {
		x = (alltank[id].y < 4 ? Ds : As);
	}
	else if (MAP[alltank[id].x + (MYSIDE == 0 ? 2 : -2)][alltank[id].y] == tank) {
		x = stay;
	}

	else if (MAP[alltank[id].x][alltank[id].y + 1] == air
		&& MAP[alltank[id].x][alltank[id].y - 1] == air) {
		x = stay;
	}
	/*else if (MAP[alltank[id].x][alltank[id].y + (alltank[id].y<4?1:-1)] == brick) {
		x = (alltank[id].y < 4 ? Ds : As)
	}*/
	/*else if (MAP[alltank[id].x][alltank[id].y + (alltank[id].y < 4 ? 1 : -1)] == brick) {
		x = Ds;4
	}*/
	if (!alltank[id].shootable && !ismove(x))x = stay;
}

//处理中线前情况
inline void dealmid(int id, action &x) {
	if (MAP[alltank[id].x + (MYSIDE == 0 ? 1 : -1)][alltank[id].y] == brick) {
		wander(id, x);
	}
}

//以防埋伏！
inline void dontmove(int id, action &x) {
	if (!ismove(x))return;
	int dx, dy, dx2, dy2;	
	
	if(superpos(id)
		&&(getnextobj(alltank[id].x + direction[x][0], alltank[id].y + direction[x][1], (action)((x + 1) % 4 + 4), dx, dy) == tank
			&& gettankid(MAPid[dx][dy]) != 1 - id
			||
			getnextobj(alltank[id].x + direction[x][0], alltank[id].y + direction[x][1], (action)((x + 3) % 4 + 4), dx, dy) == tank
			&& gettankid(MAPid[dx][dy]) != 1 - id))
	{
		x = stay;
#ifndef _BOTZONE_ONLINE
		cout << id << " dont move" << endl;
#endif
		return;
	}

	if (alltank[id].last == stay)return;

	if (getnextobj(alltank[id].x + direction[x][0], alltank[id].y + direction[x][1], (action)((x + 1) % 4 + 4), dx, dy) == tank
		&& gettankid(MAPid[dx][dy]) != 1 - id
		&& alltank[gettankid(MAPid[dx][dy])].shootable
		&&(MYSIDE==0?(alltank[id].x< alltank[gettankid(MAPid[dx][dy])].x):(alltank[id].x > alltank[gettankid(MAPid[dx][dy])].x))
		||
		getnextobj(alltank[id].x + direction[x][0], alltank[id].y + direction[x][1], (action)((x + 1) % 4 + 4), dx, dy) == tank
		&& gettankid(MAPid[dx][dy]) != 1 - id
		&&getnextobj(dx, dy, (action)((x + 1) % 4 + 4), dx2, dy2) == tank
		&& gettankid(MAPid[dx2][dy2]) != 1 - id
		&& alltank[gettankid(MAPid[dx2][dy2])].shootable)
	{
		x = stay;
#ifndef _BOTZONE_ONLINE
		cout << id << " dont move" << endl;
#endif
	}
	if (getnextobj(alltank[id].x + direction[x][0], alltank[id].y + direction[x][1], (action)((x + 3) % 4 + 4), dx, dy) == tank
		&& gettankid(MAPid[dx][dy]) != 1 - id
		&& alltank[gettankid(MAPid[dx][dy])].shootable
		&& (MYSIDE == 0 ? (alltank[id].x < alltank[gettankid(MAPid[dx][dy])].x) : (alltank[id].x > alltank[gettankid(MAPid[dx][dy])].x))
		||
		getnextobj(alltank[id].x + direction[x][0], alltank[id].y + direction[x][1], (action)((x + 3) % 4 + 4), dx, dy) == tank
		&& gettankid(MAPid[dx][dy]) != 1 - id
		&&getnextobj(dx, dy, (action)((x + 3) % 4 + 4), dx2, dy2) == tank
		&& gettankid(MAPid[dx2][dy2]) != 1 - id
		&& alltank[gettankid(MAPid[dx2][dy2])].shootable)
	{
		x = stay;
#ifndef _BOTZONE_ONLINE
		cout << id << " dont move" << endl;
#endif
	}

}

//预判
inline void dosth(int id, action &ans,action &move) {
	if (!alltank[id].shootable)return;
	if (ans != stay)return;
	//if (!(abs(alltank[1 - id].x - 4) >=2&&alltank[id].x==(MYSIDE==0?3:5)))return;
	int dx, dy;
	rep(i, 4) {
		int x = alltank[id].x+direction[i][0];
		int y = alltank[id].y + direction[i][1];
		while (MAP[x][y] == air
			|| MAP[x][y] == tank&& gettankid(MAPid[x][y])==1-id&&ismove(move)&&move%2!=i%2) {
			if (getnextobj(x, y, (action)((i + 1) % 4+4), dx, dy) == tank 
				&& gettankid(MAPid[dx][dy]) != 1 - id
				&&abs(x+y-dx-dy)==1
				|| getnextobj(x, y, (action)((i + 3) % 4+4), dx, dy) == tank 
				&& gettankid(MAPid[dx][dy]) != 1 - id
				&& abs(x + y - dx - dy) == 1)
			{
				action temp = action(i + 4);
				if (getnextobj(alltank[id].x, alltank[id].y, temp, dx, dy) == brick &&
					dx == (MYSIDE == 0 ? 0 : 8) && abs(dy - 4) <= 1) {

				}
				else {

					ans = action(i + 4);
#ifndef _BOTZONE_ONLINE
					cout << id << " do sth" << endl;
#endif
					return;
				}
			}
			x += direction[i][0];
			y += direction[i][1];
		}
	}
}

inline bool win(int id,action &x){
	int dx, dy;
	if (reachbtm(id)
		&&getnextobj(alltank[id].x,alltank[id].y, (alltank[id].y < 4 ? Ds : As),dx,dy)==base
		&&alltank[id].shootable)
	{
		x = (alltank[id].y < 4 ? Ds : As);
		return true;
	}
	return false;
}

inline bool shootally(int id, action &shoot,action &move) {
	int px = alltank[1 - id].x;
	int py = alltank[1 - id].y;
	if(move>=W&&move<=A)
	px += direction[move][0],
	py += direction[move][1];
	int x = alltank[id].x;
	int y = alltank[id].y;

	x += direction[shoot - 4][0];
	y += direction[shoot - 4][1];

	while (inmap(x,y)&&MAP[x][y] == air|| MAP[x][y] == tank) {
		if (x == px && y == py)return true;
		x += direction[shoot - 4][0];
		y += direction[shoot - 4][1];
	}
	return false;
}

inline void solve(action &A0, action &A1) {
	action advice0 = stay, advice1 = stay;

	rep(i, N)rep(j, N)globalinway[i][j] = false;
	search(0);
	search(1);

#ifndef _BOTZONE_ONLINE
	printway();
	cout << 0 << ":" << globalastar[0] << endl;
	cout << 1 << ":" << globalastar[1] << endl;
#endif

	bool dosthble[2] = { false,false };

	//0
	if (alltank[0].isalive()) {
		if (win(0, A0)) {

		}
		else if (indanger(0, advice0)) {
			A0 = advice0;
		}
		else {
			push(0, A0);
			//if (invalley(0))wander(0, A0);
			if (beforemid(0))dealmid(0, A0);
			if (superpos(0))dealwithsuperpos(0, A0);
			dontmove(0, A0);
			dosthble[0] = true;
		}
	}
	//1
	if (alltank[1].isalive()) {
		if (win(1, A1)) {

		}
		else if (indanger(1, advice1)) {
			A1 = advice1;
		}
		else {
			push(1, A1);
			//if (invalley(1))wander(1, A1);
			if (beforemid(1))dealmid(1, A1);
			if (superpos(1))dealwithsuperpos(1, A1);
			dontmove(1, A1);
			dosthble[1] = true;
		}
	}

	if(dosthble[0])dosth(0, A0,A1);
	if (dosthble[1])dosth(1, A1,A0);
	//check
	if (!ismove(A0)  && shootally(0, A0, A1))A0 = stay;
	if (!ismove(A1) && shootally(1, A1, A0))A1 = stay;
	if (superpos(0) && superpos(1))
		if(alltank[0].shootable&&alltank[1].shootable)
		A0 = A1 = (MYSIDE == 0 ? Ws : Ss);
		else
			A0 = A1 = (MYSIDE == 0 ? S : W);
	
	if (!alltank[0].shootable && !ismove(A0))A0 = stay;
	if (!alltank[1].shootable && !ismove(A1))A1 = stay;

}
