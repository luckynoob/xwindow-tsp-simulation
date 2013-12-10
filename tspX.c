// Travelling salesman problem using SA algorithm
// Use following command line to compile:
// gcc -o tspX testX.c -lX11 -lm
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

Display *dpy;
Window win;
int scn;
GC gc, gcblue;

// map grid
#define MAPS 45 
#define GRID 14
char map[MAPS][MAPS] = {0};

// draw map on screen
void map_print()
{
    int i,j;
    printf("\nMap of cities:\n");
    for (i=0; i<MAPS; i++)
    {
        for (j=0; j<MAPS; j++)
            {
                if (map[i][j])
                    printf("%2d", map[i][j]);
                else 
                    printf(". ");
            }
        printf("\n");
    }
}

//point structure
struct point
{
    int x; 
    int y;
}; 

// compute the distance between points, random factor added
int distance (int i, int j, int k, int l)
{
    int euclid = (int)(sqrt(((i-k)*(i-k)+(j-l)*(j-l))));
    if (euclid == 0)
        return 0;
    else
    {
        double random_factor = rand()/(5.0*RAND_MAX)+0.9;
        return euclid * random_factor;
    }
}

// generate random coordinate within map range
int random_coord () 
{
    return (int)(MAPS * (double)(rand()) / RAND_MAX);
}

// compute the cost of a path based on a given mileage table
int cost (int **mileage, char *path, int length)  
{
    int i;
    int total_cost = 0; // initialize total cost
    for (i=1; i<length; i++)
    {
        total_cost = total_cost + mileage[path[i-1]][path[i]];
    }
    return total_cost;
}

// probability function for moving to a new path 
int prob ( double tmpr, int old_cost, int new_cost)
{
    if (new_cost < old_cost)
        return 1;
    else if  (exp((old_cost-new_cost)*tmpr/MAPS) > (1.0*rand()/RAND_MAX))
        return 1;
    else
        return 0;
}

unsigned long GetColor(Display* dpy, char* color_name)
{
    Colormap cmap;
    XColor near_color, true_color;

    cmap = XDefaultColormap(dpy, 0);
    XAllocNamedColor(dpy,cmap,color_name, &near_color,&true_color);
    return(near_color.pixel);
}

void init_x()
{
    unsigned long black, white;

    dpy=XOpenDisplay(NULL);
    scn=XDefaultScreen(dpy);
    black=XBlackPixel(dpy,scn);
    white=XWhitePixel(dpy,scn);

    win=XCreateSimpleWindow(dpy,XDefaultRootWindow(dpy),0,0,
                            MAPS*GRID+50,MAPS*GRID+50, 1 ,black, white);
    XSetStandardProperties(dpy,win, "SA algorithm", "Hi", None, NULL,0,NULL);
    XSelectInput(dpy,win, ExposureMask|KeyPressMask);

    gc=XCreateGC(dpy,win,0,0);
    gcblue=XCreateGC(dpy,win,0,0);
    XSetBackground(dpy,gc,white);
    XSetForeground(dpy,gc,black);
    XSetForeground(dpy,gcblue, GetColor(dpy,"blue"));
    XClearWindow(dpy, win);
}

void close_x()
{
    XFreeGC(dpy,gc);
    XFreeGC(dpy,gcblue);
    XDestroyWindow(dpy,win);
    XCloseDisplay(dpy);
    exit(0);
}

int main()
{
    int i,j,k; 
    
    unsigned int iseed = (unsigned int)time(NULL);
    srand (iseed); // initialize the random number generator

    int n; 
    printf("Enter the number of cities: ");
    scanf("%d", &n);

    // Generate a random map of cities
    struct point **loct = (struct point **) malloc(n * sizeof(struct point *));
    for (i=0; i<n; i++)
    {
        loct[i] = (struct point *)malloc(sizeof(struct point));
        loct[i]->x = random_coord();
        loct[i]->y = random_coord();
        map[loct[i]->x][loct[i]->y] = 1 + i; // print cities onto the map
    }

    // Now generate a mileage table for the map
    printf("\nMileage table:\n");
    int **dist = (int **)malloc(n * sizeof(int *));
    for (i=0; i<n; i++)
    {
        dist[i] = (int *)malloc(n * sizeof(int));
        for (j=0; j<n; j++)
        {
            dist[i][j] = distance(loct[i]->x, loct[i]->y, loct[j]->x, loct[j]->y);
            printf("%3d ", dist[i][j]);
        }
        printf("\n");
    }

    // initialize the path and cost
    int cur_cost, new_cost, best_cost;
    char *cur_path = (char *)malloc(n * sizeof(char));
    char *new_path = (char *)malloc(n * sizeof(char));
    char *best_path = (char *)malloc(n * sizeof(char));

    for (i=0; i<n; i++)
    {
        cur_path[i] = i;
        new_path[i] = i;
        best_path[i] = i;
    }
    cur_cost = cost (dist, cur_path, n);
    new_cost = cur_cost;
    best_cost = cur_cost;
    
    // main loop of SA algorithm
    double step = 0.01/n; 
    double heat = 0.0001; 
    while(heat < 200)
    {
        // generate a new path by swithcing two nodes of current path
        i = (char)(n * (double)(rand())/RAND_MAX);
        j = (char)(n * (double)(rand())/RAND_MAX);
        for(k=0; k<n; k++)
            new_path[k] = cur_path[k];
        new_path[i] = cur_path[j];
        new_path[j] = cur_path[i];

        // compute the cost of new path and decide whether move to the new path
        new_cost = cost (dist, new_path, n);
        if (prob(heat, cur_cost, new_cost))
        {
            cur_path[i] = new_path[i];
            cur_path[j] = new_path[j];
            cur_cost = new_cost;
        }

        // compare to the best path recorded so far
        if (new_cost < best_cost)
        {
            for (k=0; k<n; k++)
                best_path[k] = new_path[k];
            best_cost = new_cost;
        }
        // cooling rate
        heat = heat + step;
    }

    printf("\nBest searched path:\n");
    for (k=0; k<n; k++)
        printf("%3d", best_path[k]+1);
    printf("\nwith total distance of %d\n", best_cost);

    map_print();

    XEvent eve;
    KeySym key;
    char buffer [30];
    char keybuf [255];
    int offx, offy;

    init_x();
    XMapWindow(dpy, win);

    while (1) {
        XNextEvent(dpy, &eve);
        switch (eve.type)
        {
            case Expose:
            XDrawLine(dpy,win,gc, 5,5, 5, 35+GRID*MAPS);
            XDrawLine(dpy,win,gc, 5,5, 35+GRID*MAPS, 5);
            XDrawLine(dpy,win,gc, 5,35+GRID*MAPS, 35+GRID*MAPS, 35+GRID*MAPS);
            XDrawLine(dpy,win,gc, 35+GRID*MAPS, 5, 35+GRID*MAPS, 35+GRID*MAPS);
            i = best_path[0];
            XFillRectangle(dpy,win,gcblue, GRID+GRID*loct[i]->y-2,GRID+GRID*loct[i]->x-2, 4,4);
            for (i=1; i<n; i++)
            {
                k=best_path[i];
                j=best_path[i-1];
                XFillRectangle(dpy,win,gc,GRID+GRID*loct[k]->y-2,GRID+GRID*loct[k]->x-2, 4,4);
                XDrawLine(dpy, win, gc, GRID+GRID*loct[j]->y, GRID+GRID*loct[j]->x, 
                    GRID+GRID*loct[k]->y, GRID+GRID*loct[k]->x);
                sprintf(buffer, "%d", dist[j][k]);
                if(loct[k]->y > loct[j]->y)
                    offy = 0;
                else
                    offy = -GRID/2+2;
                if(loct[k]->x > loct[j]->x)
                    offx = 0;
                else
                    offx = GRID/2+2;
                XDrawString(dpy, win, gc, GRID+GRID/2*(loct[k]->y+loct[j]->y)+offy, 
                    GRID+GRID/2*(loct[k]->x+loct[j]->x)+offx, buffer, strlen(buffer));
            }
            break;

            case KeyPress:
                if (XLookupString(&eve.xkey, keybuf ,255 , &key,0) == 1)
                    if((keybuf[0]=='q')||(keybuf[0]=='Q'))
                        close_x();
                break;
        }
    }
    return 0;
}
