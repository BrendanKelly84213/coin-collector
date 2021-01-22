#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <vector>
#include <cmath>
#include <iostream>
#include <climits>

//######################### TYPES ############################################

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Refactoring note, this is more acurately described as vec2 
struct Point
{
	double x,y;
	Point() : x(0), y(0) {}
	Point( double ex, double why ) : x( ex ), y( why ) {}
};	

struct Environment 
{
	SDL_Window*   window   = NULL;  
	SDL_Renderer* renderer = NULL;

	bool init();
	void close();
};

bool Environment::init()
{
	bool success = true;
	if( SDL_Init( SDL_INIT_VIDEO ) != 0 ) {
		printf( "Failed to initialize video %s\n", SDL_GetError() );
		success = false;
	} else {
		window = SDL_CreateWindow( 
				"Window", 
				SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED,
				SCREEN_WIDTH,
				SCREEN_HEIGHT,
				0
		);
		if( window == NULL ) {
			printf( "Could not create window: %s\n", SDL_GetError() );
			success = false;
		} else {
			renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
			if( window == NULL ) {
				printf( "Could not create renderer: %s\n" );
				success == false;
			}
		}
	}
	return success;
}

void Environment::close()
{
	SDL_DestroyWindow( window );
	window = NULL;

	SDL_DestroyRenderer( renderer );
	renderer = NULL;

	SDL_Quit();
}

struct Player
{
	double xVel, yVel;
	Point pos;
	const double ds = 0.3;
	const double RECT_WIDTH = 20, RECT_HEIGHT=20;
	SDL_Rect rect;

	Player() : pos(0,0), xVel(0), yVel(0) 
	{
		rect = { pos.x, pos.y, RECT_WIDTH, RECT_HEIGHT };
	}
	Player( double x, double y, double xv, double yv ) : pos(x, y), xVel(xv), yVel(yv) 
	{
		rect = { x, y, RECT_WIDTH, RECT_HEIGHT };
	}
	void handleInputs( SDL_Event& e );
	void move();
	void draw( SDL_Renderer* renderer );
	void collectCoin( Point coint );
};

//######################### ALGORITHM ############################################

void Player::handleInputs( SDL_Event& e )
{
	if( e.type == SDL_KEYDOWN && e.key.repeat == 0 ) {
		switch( e.key.keysym.sym ) {
			case SDLK_h: xVel -= ds; break;
			case SDLK_j: yVel += ds; break;
			case SDLK_k: yVel -= ds; break;
			case SDLK_l: xVel += ds; break;
		}
	}
	else if( e.type == SDL_KEYUP && e.key.repeat == 0 ) {
		switch( e.key.keysym.sym ) {
			case SDLK_h: xVel += ds; break;
			case SDLK_j: yVel -= ds; break;
			case SDLK_k: yVel += ds; break;
			case SDLK_l: xVel -= ds; break;
		}
	}
}

int magnitude( double x, double y ) 
{
	return std::sqrt( std::pow(x, 2) + std::pow(y, 2) );
}

void Player::move() 
{
	pos.x += xVel;
	if( pos.x < 0 || ( pos.x >= (SCREEN_WIDTH - RECT_WIDTH ) ) ) {
		pos.x -= xVel;
	}
	pos.y += yVel; 
	if( ( pos.y < 0 ) || ( pos.y >= (SCREEN_HEIGHT - RECT_HEIGHT) ) ) {
		pos.y -= yVel;
	}

	rect = { pos.x, pos.y, RECT_WIDTH, RECT_HEIGHT };
}

void Player::draw( SDL_Renderer* renderer ) 
{
	SDL_SetRenderDrawColor( renderer, 255, 255, 255, 255 );
	SDL_RenderFillRect( renderer, &rect );
}

inline double dist( Point src, Point dest ) 
{
	return std::sqrt( std::pow( ( src.x - dest.x ), 2 ) + std::pow( ( src.y - dest.y ), 2 ) );
}

Point minDistPoint( Point player, std::vector<Point> points ) 
{
	double min = INT_MAX;
	Point minPoint( 0, 0 );
	for( auto p : points ) {
		double dist_ = dist( player, p );
		if(  dist_ < min ) {
			min = dist_;
			minPoint.x = p.x;
			minPoint.y = p.y;
		}
	}
	return minPoint;
}

void drawSquareAtPoint( SDL_Renderer* renderer, Point p ) 
{
	SDL_Rect rect = { p.x, p.y, 10, 10 };
	SDL_SetRenderDrawColor( renderer, 100, 255, 255, 255 );
	SDL_RenderFillRect( renderer, &rect );
}

void drawLineToClosest( SDL_Renderer* renderer, Player pl, std::vector<Point> points ) 
{
	Point dest = minDistPoint( pl.pos, points );
	SDL_RenderDrawLine( renderer, pl.pos.x, pl.pos.y, dest.x, dest.y );
}

bool collision( SDL_Rect a, SDL_Rect b ) 
{
	bool collision = true;
	double leftA=a.x, rightA=a.x+a.w, topA=a.y, bottomA=a.y+a.h;
	double leftB=b.x, rightB=b.x+b.w, topB=b.y, bottomB=b.y+b.h;

	if( bottomA <= topB ) {
		return false;
	}
	if( topA >= bottomB ) {
		return false;
	}
	if( rightA <= leftB ) {
		return false;
	}
	if( leftA >= rightB ) {
		return false;
	}

	return true;
}

void Player::collectCoin( Point coin ) 
{
	//Set velocity of the player to the direction of the coin
	double mag = dist( coin, pos );
	xVel = ( ( coin.x - pos.x ) / mag )*ds;
	yVel = ( ( coin.y - pos.y ) / mag )*ds;
}

int find( std::vector<Point> points, Point item ) 
{
	int idx = -1;
	for( int p=0; p < points.size(); ++p ) {
		if( points[p].x == item.x &&  points[p].y == item.y) {
			idx = p;
		}
	}
	return idx;
}

int main() 
{
	std::vector<Point> points = { };
	Environment local;
	if( !local.init() ) {
		return 1;	
	}
	
	bool ai = false;
	SDL_Event e;
	Player s;
	while( 1 ) {
		if( SDL_PollEvent( &e ) ) {
			if( e.type == SDL_KEYDOWN  ) {
				switch( e.key.keysym.sym ) {
					case SDLK_q: return 1; break;
					case SDLK_r:
						for(int i=0; i < 100; ++i) {
							Point p( ( 40 + std::rand()) % ( 620 ), 
							         ( 40 + std::rand()) % ( 460 ) );
							points.push_back( p ); 
						}
						break;
					case SDLK_c: points.clear(); break;
					case SDLK_a: 
						     ai = !ai; 
						     s.xVel = 0;
						     s.yVel = 0;
						     break;
				}
			}
		        s.handleInputs( e ); 
		}
		
		//Compute closest coin
		Point closestCoin = minDistPoint( s.pos, points );
		SDL_Rect b = { closestCoin.x, closestCoin.y, 10, 10 };

		if( ai ) {
			s.collectCoin( closestCoin );
		}	
		
		//Handle collision
		if( collision( s.rect, b ) ) {
			int idx = find( points, closestCoin );
			if( idx == -1 ) {
				std::cout << "There are either no more coins or something broke" << '\n';
			} else {
				points.erase( points.begin() + idx );	
			}
		}

		s.move();
		
		//Clear screen
		SDL_SetRenderDrawColor( local.renderer, 0, 0,  0, 0 );
		SDL_RenderClear( local.renderer );

		//Render Objects
		s.draw( local.renderer );
		for( auto p : points ) {
			drawSquareAtPoint( local.renderer, p );
		}
		drawLineToClosest( local.renderer, s, points );
		
		//Update screen
		SDL_RenderPresent( local.renderer );
	}

	//Free and close SDL
	local.close();


	return 0;
}
