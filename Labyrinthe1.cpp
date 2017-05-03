#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <istream>
#include <vector>
#include "Labyrinthe.h"
#include "Chasseur.h"
#include "Gardien.h"

Sound*	Chasseur::_hunter_fire;	// bruit de l'arme du chasseur.
Sound*	Chasseur::_hunter_hit;	// cri du chasseur touché.
Sound*	Chasseur::_wall_hit;	// on a tapé un mur.

/*create the class for (x,y) to read data*/
class CPoint
{
public:
	int x;
	int y;
public:
	CPoint(int xx = 0, int yy = 0) :x(xx), y(yy) {};
	int getX() { return x; }
	int getY() { return y; }
	void output(); //show point in the form (x,y) 
};

// print out the point  
void CPoint::output()
{
	cout << "(" << x << ", " << y << ")" << endl;
}

/****************************************************************************
Create the BFS tree stucture and tools useful
******************************************************************************/

struct node {
	CPoint point;
	node *next;
};

class Queue {
private:
	node *front;
	node *rear;
public:
	Queue();
	~Queue();
	bool isEmpty();
	void enqueue(CPoint);
	CPoint dequeue();
};

Queue::Queue() {
	front = NULL;
	rear = NULL;
}

Queue::~Queue() {
	delete front;
}

void Queue::enqueue(CPoint data) {
	node *temp = new node();
	temp->point = data;
	temp->next = NULL;
	if (front == NULL) {
		front = temp;
	}
	else {
		rear->next = temp;
	}
	rear = temp;
}

CPoint Queue::dequeue() {
	node *temp = new node();
	CPoint value;
	if (front == NULL) {
		cout << "\nQueue is Emtpty\n";
	}
	else {
		temp = front;
		value = temp->point;
		front = front->next;
		delete temp;
	}
	return value;
}

bool Queue::isEmpty() {
	return (front == NULL);
}

/*The first read of file txt to get mesures*/
void ReadFirstTime(int *w, int *l, int *first, char* filename)
{
	ifstream fin(filename);
	if (!fin.is_open())
	{
		cout << "Error opening file"; exit(1);
	}
	string line;

	int width = 0, longeur = 0, begin = 0;
	int test = 0;
	while (getline(fin, line, '\n'))
	{
		test = line.length();
		if (line[0] == '+'&&begin == 0)
		{
			begin = longeur;
		}
		if (width < test)
		{
			width = test;
		}
		longeur++;		
	}
	*w = width;
	*l = longeur - begin+1;
	*first = begin;
	fin.close();
}



/*Main fonction of labrinthe*/

Environnement* Environnement::init (char* filename)
{
	return new Labyrinthe (filename);
}


Labyrinthe::Labyrinthe (char* filename)
{
	int width, length, begin;
	
	/*Read first time to get the size of the maze*/
	ReadFirstTime(&width, &length, &begin, filename);

	/*Calculate the maximum nomber of cases in the maze and define special value for boxes and walls*/

	int maxcase = width*length;//distance initial for case empty
	int box_dist = 2 * maxcase;// distance for boxes
	int wall_dist = 3 * maxcase;//distance for walls

	/*Read second time to build matrix*/
	ifstream fin(filename, ios::in);
	if (!fin.is_open())
	{
		cout << "Error opening file"; exit(1);
	}

	/*build the maze*/
	string line;
	int cpt = 1;	
	char **lab = new char*[length];
	for (size_t i = 0; i < length+1; i++)
	{
		lab[i] = new char[width];
	}
	cout << "begin=" << begin << endl;

	while (getline(fin, line))
	{
		if (cpt >= begin)
		{
			for (size_t i = 0; i < width + 1; i++)
			{
				if (i <= line.length())
				{
					lab[cpt - begin][i] = line[i];
				}
				else
				{
					lab[cpt - begin][i] = ' ';
				}
				cout << lab[cpt - begin][i];
			}
		}
		cout << endl;
		cpt++;
	}//maze initial

	int **dist = new int*[length];
	for (size_t i = 0; i < length+1; i++)
	{
		dist[i] = new int[width];
	}//maze distance


	char **lab_inv = new char*[width];//maze inverse

	for (size_t i = 0; i < width + 1; i++)
	{
		lab_inv[i] = new char[length];
	}

	int nb_corner = 0;
	int nb_affa = 0, nb_affb = 0;
	int nb_box = 0, nb_guard = 0;
	CPoint chaser;
	CPoint treasure;
	for (size_t i = 0; i < length+1; i++)
	{
		for (size_t j = 0; j < width+1; j++)
		{
			char current = lab[i][j];
			lab_inv[j][i] = current;
			switch (current) {
			case '+': dist[i][j] = wall_dist;
				nb_corner++;
				break;
			case '-': dist[i][j] = wall_dist;
				break;
			case 'a': dist[i][j] = wall_dist;
				nb_affa++;
				break;
			case 'b': dist[i][j] = wall_dist;
				nb_affb++;
				break;
			case '|': dist[i][j] = wall_dist;
				break;
			case 'x': dist[i][j] = box_dist;
				nb_box++;
				break;
			case 'T': dist[i][j] = 0; 
				treasure.x = i;
				treasure.y = j; break; // "T, the treasure"
			case 'G': dist[i][j] = -1;
				nb_guard++; break;
			case 'C':dist[i][j] = -1;
				chaser.x = i;
				chaser.y = j;
				break;
			default: dist[i][j] = -1; break;//empty space
			}
		}
	}

	/*count all units and record their position*/
	CPoint *guards = new CPoint[nb_guard];
	CPoint *boxes = new CPoint[nb_box];
	CPoint *affas = new CPoint[nb_affa];
	CPoint *affbs = new CPoint[nb_affb];
	CPoint *wall_horiz = new CPoint[nb_corner*2];
	CPoint *wall_verti = new CPoint[nb_corner*2];

	int cpt_g = 0, cpt_box = 0;
	int cpt_a = 0, cpt_b = 0;
	int cpt_wh = 0, cpt_wv = 0;

	for (size_t i = 0; i < length + 1; i++)
	{
		char previus = 'n';
		for (size_t j = 0; j < width + 1; j++)
		{			
			char current = lab[i][j];
			switch (current) {
			case '+': 
				if (previus=='n')
				{
				wall_horiz[cpt_wh] = CPoint(i, j);
				cpt_wh++;
				break;
				}
				else
				{
					if (previus =='-'|| previus == 'a' || previus == 'b'|| previus == '+')
					{
						if ((cpt_wh % 2) == 0)
						{
							wall_horiz[cpt_wh] = wall_horiz[cpt_wh - 1];
							cpt_wh++;
							wall_horiz[cpt_wh] = CPoint(i, j);
							cpt_wh++;
							break;
						}
						else
						{
							wall_horiz[cpt_wh] = CPoint(i, j);
							cpt_wh++;
							break;
						}						
					}
					else
					{
						wall_horiz[cpt_wh] = CPoint(i, j);
						cpt_wh++;
						break;//read all walls horizon in normal matrix
					}
				}
			case 'a': affas[cpt_a] = CPoint(i, j);
				cpt_a++;
				break;
			case 'b': affbs[cpt_b] = CPoint(i, j);
				cpt_b++;
				break;
			case 'x': boxes[cpt_box] = CPoint(i, j);
				cpt_box++;
				break;
			case 'G': guards[cpt_g] = CPoint(i, j);
				cpt_g++;
				break;
			default: break;//empty space
			}
			previus = current;
		}
	}

	/* read all vertical walls by using the inv-matrix*/
	for (size_t i = 0; i < width + 1; i++)
	{
		char previus = 'n';
		for (size_t j = 0; j < length + 1; j++)
		{
			char current = lab_inv[i][j];
			switch (current) {
			case '+': 
				if (previus=='n')
				{
				wall_verti[cpt_wv] = CPoint(j, i);
				cpt_wv++;
				break;
				}
				else
				{
					if (previus == '|' || previus == 'a' || previus == 'b' || previus == '+')
					{
						if ((cpt_wv % 2) == 0)
						{
							wall_verti[cpt_wv] = wall_verti[cpt_wv - 1];
							cpt_wv++;
							wall_verti[cpt_wv] = CPoint(j, i);
							cpt_wv++;
							break;
						}
						else
						{
							wall_verti[cpt_wv] = CPoint(j, i);
							cpt_wv++;
							break;
						}						
					}
					else
					{
						wall_verti[cpt_wv] = CPoint(j, i);
						cpt_wv++;
						break;
					}
				}
			default: break;
			}
			previus = current;
		}
	}

	/*use BFS to calculate the distance from each case to the treasure and update the matrix*/

	BFS(treasure, dist);

	// les 4 murs.
	static Wall walls [] = {
		{ 0, 0, LAB_WIDTH-1, 0, 0 },
		{ LAB_WIDTH-1, 0, LAB_WIDTH-1, LAB_HEIGHT-1, 0 },
		{ LAB_WIDTH-1, LAB_HEIGHT-1, 0, LAB_HEIGHT-1, 0 },
		{ 0, LAB_HEIGHT-1, 0, 0, 0 },
	};
	// une affiche.
	//  (attention: pour des raisons de rapport d'aspect, les affiches doivent faire 2 de long)
	static Wall affiche [] = {
		{ 2, 0, 6, 0, 0 },		// première affiche.
		{ 8, 0, 10, 0, 0 },		// autre affiche.
	};
	// 3 caisses.
	static Box	caisses [] = {
		{ 70, 12, 0 },
		{ 10, 5, 0 },
		{ 65, 22, 0 },
	};
	// juste un mur autour et les 3 caisses et le trésor dedans.
	for (int i = 0; i < LAB_WIDTH; ++i)
		for (int j = 0; j < LAB_HEIGHT; ++j) {
			if (i == 0 || i == LAB_WIDTH-1 || j == 0 || j == LAB_HEIGHT-1)
				_data [i][j] = 1;
			else
				_data [i][j] = EMPTY;
		}
	// indiquer qu'on ne marche pas sur une caisse.
	_data [caisses [0]._x][caisses [0]._y] = 1;
	_data [caisses [1]._x][caisses [1]._y] = 1;
	_data [caisses [2]._x][caisses [2]._y] = 1;
	// les 4 murs.
	_nwall = 4;
	_walls = walls;
	// deux affiches.
	_npicts = 2;
	_picts = affiche;
	// la deuxième à une texture différente.
	char	tmp [128];
	sprintf (tmp, "%s/%s", texture_dir, "voiture.jpg");
	_picts [1]._ntex = wall_texture (tmp);
	// 3 caisses.
	_nboxes = 3;
	_boxes = caisses;
	// le trésor.
	_treasor._x = 10;
	_treasor._y = 10;
	_data [_treasor._x][_treasor._y] = 1;	// indiquer qu'on ne marche pas sur le trésor.
	// le chasseur et les 4 gardiens.
	_nguards = 5;
	_guards = new Mover* [_nguards];
	_guards [0] = new Chasseur (this);
	_guards [1] = new Gardien (this, "drfreak");
	_guards [2] = new Gardien (this, "Marvin"); _guards [2] -> _x = 90.; _guards [2] -> _y = 70.;
	_guards [3] = new Gardien (this, "Potator"); _guards [3] -> _x = 60.; _guards [3] -> _y = 10.;
	_guards [4] = new Gardien (this, "garde"); _guards [4] -> _x = 130.; _guards [4] -> _y = 100.;
	// indiquer qu'on ne marche pas sur les gardiens.
	_data [(int)(_guards [1] -> _x / scale)][(int)(_guards [1] -> _y / scale)] = 1;
	_data [(int)(_guards [2] -> _x / scale)][(int)(_guards [2] -> _y / scale)] = 1;
	_data [(int)(_guards [3] -> _x / scale)][(int)(_guards [3] -> _y / scale)] = 1;
	_data [(int)(_guards [4] -> _x / scale)][(int)(_guards [4] -> _y / scale)] = 1;
}
