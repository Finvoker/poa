#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <istream>
#include <vector>

using namespace std;

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

/*class Guadient
{
private:
	CPoint gp;
	char mode;
public:
	Guadient(CPoint pp, char m) :gp(pp), mode(m) {};
	CPoint getPosition() { return gp; }
	char showMode() { return mode; }
	
};*/


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

/*Fonction BFS for calculate the distance*/

void BFS(CPoint t, int **tab)
{
	Queue Q;

	Q.enqueue(t);

	while (!Q.isEmpty())
	{
		CPoint cp = Q.dequeue();
		int cpx = cp.getX();
		int cpy = cp.getY();
		int dist = tab[cpx][cpy];

		for (size_t i = cpx - 1; i<cpx + 2; i++)
		{
			for (size_t j = cpy - 1; j < cpy + 2; j++)
			{
				if (tab[i][j] == -1)
				{
					CPoint space(i, j);
					Q.enqueue(space);
					tab[i][j] = dist + 1;
				}
			}

		}
	}
}

/*The first read of file txt to get mesures*/
void ReadFirstTime(int *w, int *l, int *first, char* filename)//put in class wall later
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

int main()
{
	int width, length, begin;
	int filesize;
	
	/*Read first time to get the size of the maze*/
	ReadFirstTime(&width, &length, &begin,"123.txt");

	/*Calculate the maximum nomber of cases in the maze and define special value for boxes and walls*/

	int maxcase = width*length;//distance initial for case empty
	int box_dist = 2 * maxcase;// distance for boxes
	int wall_dist = 3 * maxcase;//distance for walls

	/*Read second time to build matrix*/
	ifstream fin("123.txt", ios::in);
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

	cout << "\nThis is the new maze" << endl;
	/*use BFS to calculate the distance from each case to the treasure and update the matrix*/

	BFS(treasure, dist);

	int maxdist = 0;
	int distacc = 0;

	/*for (size_t i = 0; i < length + 1; i++)
	{
		for (size_t j = 0; j < width; j++)
		{
			distacc = dist[i][j];
			if ((distacc<maxcase)&(maxdist<distacc))
			{
				maxdist = distacc;
			}
			//cout << distacc << " ";
		}
	}*/

	cout << "\nThe treasure is at "; treasure.output();
	cout << "\nThe width of labyrinthe=" << width << endl;
	cout << "The length of labyrinthe=" << length << endl;
	cout << "The nomber of walls is " << (cpt_wh+cpt_wv)/2 << endl;
	int whnb = cpt_wh/2;
	for (size_t i = 0; i < whnb; i++)
	{
		cout << "\nwall horizon " << i << " "; wall_horiz[i*2].output(); wall_horiz[i* 2+1].output();
	}
	int wvnb = cpt_wv / 2;
	for (size_t i = 0; i < wvnb; i++)
	{
		cout << "\nwall verticle " << i << " "; wall_verti[i * 2].output(); wall_verti[i * 2+1].output();
	}
	getchar();
	fin.close();
}