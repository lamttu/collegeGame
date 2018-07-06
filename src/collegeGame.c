//
// Game in which player has to avoid obstacles and shoots down tasks
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "SwinGame.h"
//Constants for the game data
#define OBSTACLE_BASE 3  // base/minimum number of obstacles as the level gets higher the number of obstacles will be higher 
#define OBSTACLE_MAX 7	 // maximun number of obstacles
#define OBSTACLE_KINDS 6 // there are six kinds of obstacles
#define TASK_BASE 2      //base/minimum number of tasks as the level gets higher the number of tasks will be higher 
#define TASK_MAX 6		 //maximun number of tasks
#define BULLETCOUNT 50   //maximum number of bullets

//Constants for the graphics windows
#define WINDOWS_WIDTH 600
#define WINDOWS_HEIGHT 500

//Delay between bullets
#define SHOOT_DELAY 12

//There are 6 types of obstacles in the game
typedef enum {facebook, food, party, love, money, phone}ObstacleKind;

//
//Each obstacle has these values
//
typedef struct 
{
	ObstacleKind kind; // a kind 
	bitmap bmp;	       // actual bitmap
	float x, y;		   // location
	float dx, dy;        // x, y movement
}ObstacleData;

//
//The student has these values
//
typedef struct 
{
	sprite student;
	int current_delay;
	int lives;	       //-1 every time the student collides with an obstacle
}PlayerData;

//
//Each task has these values
//
typedef struct 
{
	bitmap bmp;
	float x,y;
	float dx, dy;
}TaskData;

//
//Each bullet has these values
//
typedef struct 
{
	bitmap bmp;
	float x,y;
	float dx, dy;
	bool alive;
}BulletData;
//
//The overall game data 
//
typedef struct 
{
	PlayerData player;							// one student/player for each game
	ObstacleData obstacles[OBSTACLE_MAX];		// there's an array of obstales
	TaskData tasks[TASK_MAX];					// there's an array of tasks
	BulletData bullets[BULLETCOUNT];			// many bullets shoot by the player
	int score;									// keep up the scores of player
	int level;									// game level chosen (1-4)
	timer gametimer;							// to keep track of the game time
}GameData;

//
// Load the game's resources
//
void load_resources(){
	load_bitmap_named("student", "student_RoundIcon.png");
	load_bitmap_named("bullet", "bullet_RoundIcon.png");

	load_bitmap_named("facebook", "facebook_Freepik.png");
	load_bitmap_named("food", "food_Freepik.png");
	load_bitmap_named("party", "party_Freepik.png");
	load_bitmap_named("love", "love_RobinKylander.png");
	load_bitmap_named("money", "money_GregorCresna.png");
	load_bitmap_named("phone", "phone_LinhPham.png");

	load_bitmap_named("task", "task_RecepKutuk.png");

	load_sound_effect_named("hit_obstacle_sound", "hitObstacle.wav");
	load_sound_effect_named("hit_task_sound", "hitTask.wav");
	load_sound_effect_named("gun_sound", "gun.wav");
}
//
// Gets the bitmap for the obstacles
//
bitmap obstacle_bitmap(ObstacleKind kind){
	switch(kind){
		case facebook: return bitmap_named("facebook");
		case food: return bitmap_named("food");
		case party: return bitmap_named("party");
		case love: return bitmap_named("love");
		case money: return bitmap_named("money");
		case phone: return bitmap_named("phone");
		default: return NULL;
	}
}
//
// Randomly pick a kind of obstacle
//
ObstacleKind random_obstacle_kind(){
	ObstacleKind obstacle_kind;

	obstacle_kind = (ObstacleKind) rnd_upto(OBSTACLE_KINDS); //return random values from 0 to 5
	return obstacle_kind;
}
//
// Generate an obstacle
//
ObstacleData random_obstacle(){
	ObstacleData obstacle;

	obstacle.kind = random_obstacle_kind();
	obstacle.bmp = obstacle_bitmap(obstacle.kind);
	obstacle.x = rnd_upto(screen_width() + 1 - bitmap_width(obstacle.bmp));   // x is a random number from 0 to the screen width minus the bitmap width
	obstacle.y = rnd_upto(screen_height() + 1 - bitmap_height(obstacle.bmp)); // same with x

	obstacle.dx =rnd() * 2 - 1;// dx ranges from -1 to 1
	obstacle.dy = rnd() * 2 - 1;// dy ranges from -1 to 1

	return obstacle;
}
//
// Initialise all of the obstacle in the game
//
void set_up_obstacles(ObstacleData obstacles[], int level){
	int max = OBSTACLE_BASE + level;
	for (int i = 0; i < max; i++)
		obstacles[i] = random_obstacle();
}
//
// Characters can move off the edge from the botton to the top, from the left to the right, etc.
//
void wrap_character(bitmap bmp, float *x, float *y){
	if(*x < - bitmap_width(bmp))
		*x = screen_width();
	else if(*x > screen_width())
		*x = - bitmap_width(bmp);
	if(*y < - bitmap_height(bmp))
		*y = screen_height();
	else if(*y > screen_height())
		*y =  - bitmap_height(bmp);
}
//
// Move the obstacle across the screen
//
void update_obstacle(ObstacleData *obstacle){
	obstacle-> x += obstacle -> dx;
	obstacle-> y += obstacle -> dy;
	if (obstacle-> x < - bitmap_width(obstacle->bmp) || obstacle-> x > screen_width() || obstacle->y < - bitmap_height(obstacle->bmp) || obstacle->y > screen_height())
		*obstacle = random_obstacle();
	//wrap_character(obstacle->bmp, &obstacle->x, &obstacle->y);
}
//
// In D, HD modes, obstacles follow the student instead of just moving randomly
// 
void update_obstacle_follow_player(ObstacleData *obstacle, float player_X, float player_Y){
	double opposite, adjacent, angle, vx, vy;
	int velocity;

	//Find out the direction (angle) the obstacle needs to move towards
	//Calculating the angle of the velocity vector
  	//
  	opposite = player_Y - obstacle-> y;
  	adjacent = player_X - obstacle-> x;
  	angle = tanh(opposite/adjacent);
  	
  	// If the obstacle's x coordinate is further away from the player, the obstacle has to go in the opposite way

  	if (obstacle-> x > player_X)
    	angle = angle + 180;
  
 	//Use this angle to calculate the velocity vector of the Ghost
  	//Once again using SOH-CAH-TOA trignometic rations
  	velocity = 1; 

  	vx = velocity * cos(angle);
  	vy = velocity * sin(angle);

  	obstacle-> x += vx;
	obstacle-> y += vy;
	wrap_character(obstacle->bmp, &obstacle->x, &obstacle->y);
}
//
// Draw a character to the screen (used for tasks and obstacles)
//
void draw_character_bmp(bitmap bmp, float x, float y){
	draw_bitmap(bmp, x, y);
}
//
// Generate an task
//
TaskData generate_task(){
	TaskData task;

	task.bmp = bitmap_named("task");
	task.x = rnd_upto(screen_width() + 1 - bitmap_width(task.bmp));   // x is a random number from 0 to the screen width minus the bitmap width
	task.y = 0; //task always falls down

	task.dx = 0;// since task only falls down vertically, the x coordinate doesn't change
	task.dy = 1;// task only falls down so dy is always positive

	return task;
}
//
// Initialise all of the tasks in the game
//
void set_up_tasks(TaskData tasks[], int level){
	int max = TASK_BASE + level;
	for (int i = 0; i < max; i++)
		tasks[i] = generate_task();
}
//
// Move the tasks across the screen
//
void update_task(TaskData *task){
	task-> y += task -> dy;
	if(task->y > screen_height()) //task falls off the bottom
		*task = generate_task();
}
//
// Set up student player
//
PlayerData set_up_player(){
	PlayerData player;
	float x, y;		//where the student will appear;

	x = rnd_upto(screen_width() + 1 - bitmap_width(bitmap_named("student")) );
	y = screen_height() - bitmap_height(bitmap_named("student")); 				//student always appears at the bottom of the screen;
	player.student	= create_basic_sprite(bitmap_named("student"));
	sprite_set_x(player.student, x);
	sprite_set_y(player.student, y);
	player.current_delay = 0;
	player.lives = 5;	//student has 5 lives at the beginning

	return player;
}
//
// Draw player to the screen
//
void draw_player(PlayerData player){
	draw_sprite(player.student);	
	update_sprite(player.student);
}
//
// Move player around the screen 
//
void move_player(PlayerData *player){
	int student_height, student_width;

	student_height = bitmap_height(bitmap_named("student"));
	student_width = bitmap_width(bitmap_named("student"));

	if(key_down(VK_LEFT))	//Move left
	{
        sprite_set_dx(player->student, -1 );
        sprite_set_dy(player->student, 0 );
    }	
	else if(key_down(VK_RIGHT))		//Move right
	{
        sprite_set_dx(player->student, 1 );
        sprite_set_dy(player->student, 0 );
    }
	else if (key_down(VK_UP))	//Move up
	{
        sprite_set_dx(player->student, 0 );
        sprite_set_dy(player->student, -1 );
    }
	else if (key_down(VK_DOWN))	//Move down
	{
        sprite_set_dx(player->student, 0 );
        sprite_set_dy(player->student, 1 );
    }
    else{
    	sprite_set_dx(player->student, 0 );
        sprite_set_dy(player->student, 0 );
    }
    //Wrap player around screen
	if(sprite_x(player->student) < 0)							//OFF LEFT
   		sprite_set_x(player->student, WINDOWS_WIDTH - student_width);
   
    if(sprite_x(player->student) + student_width > WINDOWS_WIDTH)	//OFF RIGHT
    	sprite_set_x(player->student, 0);

    if(sprite_y (player -> student) < 0)							//OFF TOP
    	sprite_set_y(player->student, WINDOWS_HEIGHT - student_height);

    if(sprite_y(player->student) + student_height > WINDOWS_HEIGHT)	//OFF BOTTOM
    	sprite_set_y(player->student, 0);
}
//
// Make a bullet
//
BulletData make_bullet(sprite student){
	BulletData bullet;
	float playerX, playerY;

	playerX = sprite_x(student);
	playerY = sprite_y(student);

	bullet.bmp = bitmap_named("bullet");
	bullet.x = playerX;   //the position of the bullet coincides with that of the player
	bullet.y = playerY; 
	bullet.alive = true;

	bullet.dx = 0;// since bullet only shoots up vertically, the x coordinate doesn't change
	bullet.dy = -1;
	return bullet;							
}
//
// Initialise all the bullets
//
void initialise_bullets(BulletData bullets[]){
	for (int i = 0; i < BULLETCOUNT; i++)
		bullets[i].alive = false;	//bullets not in use
}
//
// Move the bullets across the screen
//
void update_bullet(BulletData *bullet){
	bullet-> y += bullet -> dy;
	if(bullet->y < 0 )//bullet off screentop
		bullet->alive = false;
}
//
//	Shoot bullets
//
void shoot(BulletData bullets[], PlayerData* player, int level){
	if(player->current_delay > 0)
		player->current_delay -= 1;

	if(key_down(VK_SPACE) && (player->current_delay == 0)){
		player->current_delay = SHOOT_DELAY - level;		//as level is higher, the shoot delay is shorter allowing players to shoot consecutively 
		for(int i = 0; i < BULLETCOUNT; i++)	//find the first bullet not in use
		{
			if(bullets[i].alive == false){				
				bullets[i] = make_bullet(player->student);
				break;
			}
		}	
		play_sound_effect_named("gun_sound");
	}
}
//
//	Check whether bullets hit task
//
bool bullet_hit_task(TaskData task, BulletData bullet){
	return bitmap_collision(task.bmp, task.x, task.y, bullet.bmp, bullet.x, bullet.y);
}
//
//	Check whether player hit obstacle
//
bool player_hit_obstacle(sprite student, ObstacleData obstacle){
	return sprite_bitmap_collision(student, obstacle.bmp, obstacle.x, obstacle.y);
}
//
//	Task is replaced by another task when hit. Bullet is no longer alive
//
void process_task_collision(BulletData *bullet, TaskData *task, int *score){
	if(bullet_hit_task(*task, *bullet)){
		play_sound_effect_named("hit_task_sound");
		(*score)++;
		*task = generate_task();
		bullet->alive = false;
	}
}
//
//	Obstacle is replaced by another obstacle.
//	Player loses 1 life
//
void process_obstacle_collision(PlayerData *player, ObstacleData *obstacle){
	if(player_hit_obstacle(player->student, *obstacle)){
		play_sound_effect_named("hit_obstacle_sound");
		player->lives--;
		*obstacle = random_obstacle();
	}
}
//
// Set up initial player, tasks, and obstacles
//
void set_up_game(GameData *game){
	game -> player = set_up_player();
	game -> score = 0;
	game -> gametimer = create_timer();
	start_timer(game->gametimer); 
	set_up_obstacles(game->obstacles, game->level);
	set_up_tasks(game->tasks, game->level);
	initialise_bullets(game->bullets);
}
//
// Handle user input
//
void handle_input(GameData *game){
	

	move_player(&game->player);
	shoot(game->bullets, &game->player, game->level);
}
//
// Update the overall game
//
void update_obstacle_above_d(GameData *game){
	int number_of_obstacle = OBSTACLE_BASE + game->level;
	long int ticks;			// keep track of the game time
	int interval; 			// keep track of the number of 5-second interval in the game
	
	// For player who chooses HD and D grade, for each 5 seconds, the obstacles will alternate between specifically target the player's coordinate
	// and moving randomly to give players a breather
	// The game also checks for collision between obstacles and player

	ticks = timer_ticks(game->gametimer);
	interval = ticks / 5000; 		// each interval is 5 seconds
	if(interval % 2 == 1){		// the odd intervals
		for (int i = 0 ; i < number_of_obstacle; i++){
			// the obstacle targets player specifically
			update_obstacle_follow_player(&game->obstacles[i], sprite_x(game->player.student), sprite_y(game->player.student));
			process_obstacle_collision(&game->player, &game->obstacles[i]);
		}
	}else{						// the even interval
		for (int i = 0 ; i < number_of_obstacle; i++){
			// the obstacle moves randomly
			update_obstacle(&game->obstacles[i]);
			process_obstacle_collision(&game->player, &game->obstacles[i]);
		}
	}
		
}
void update_game(GameData *game){
	int number_of_obstacle = OBSTACLE_BASE + game->level;
	int number_of_tasks = TASK_BASE + game->level;
	
	//Update obstacles according to level
	//The player aims above D
	if(game->level > 2){
		update_obstacle_above_d(game);
	}
	// For player who only aims for P and C, the obstacles only move randomly across the screen
	else{
		for (int i = 0 ; i < number_of_obstacle; i++){
			update_obstacle(&game->obstacles[i]);
			process_obstacle_collision(&game->player, &game->obstacles[i]);
		}
	}
	
	// Move tasks across the screen
	// Check collision between bullets and tasks
	for (int j = 0; j < number_of_tasks; j++){
		update_task(&game->tasks[j]);
		for (int k = 0; k < BULLETCOUNT; k++)
			if(game->bullets[k].alive == true){	//only check bullets in use
				process_task_collision(&game->bullets[k], &game->tasks[j], &game->score);
				update_bullet(&game->bullets[k]);
			}					
	}
	
}
//
// Draw the overall game
//
void draw_game(GameData game){
	char scoreStr[3];
	char livesStr[1];
	int number_of_obstacle = OBSTACLE_BASE + game.level;
	int number_of_tasks = TASK_BASE + game.level;

	clear_screen_to(COLOR_BEIGE);
	//Draw Obstacles
	for (int i = 0 ; i < number_of_obstacle; i++)		
		draw_character_bmp(game.obstacles[i].bmp, game.obstacles[i].x, game.obstacles[i].y);

	//Draw Tasks
	for (int j = 0; j < number_of_tasks; j++)			
		draw_character_bmp(game.tasks[j].bmp, game.tasks[j].x, game.tasks[j].y);

	//Draw bullets 
	for (int k = 0; k < BULLETCOUNT; k++)
		if(game.bullets[k].alive == true)	//only alive bullets
			draw_character_bmp(game.bullets[k].bmp, game.bullets[k].x, game.bullets[k].y);

	//Draw player
	draw_player(game.player);

	//Print score
	sprintf(scoreStr, "%d", game.score);
	draw_simple_text("Score: " , COLOR_BLACK, 5, 5);
	draw_simple_text(scoreStr, COLOR_BLACK, 60, 5);

	//Print lives
	sprintf(livesStr, "%d", game.player.lives);
	draw_simple_text("Lives: " , COLOR_BLACK, 500, 5);
	draw_simple_text(livesStr, COLOR_BLACK, 580, 5);

	refresh_screen(60);
	delay(10 - game.level);
}
//
//	Display welcome screen
//
void display_welcome_screen(){
	int board_width = WINDOWS_WIDTH / 5;
	int board_height = WINDOWS_HEIGHT / 27 * 2;
	int board_X_offset = board_width * 2;
	int board_Y_offset = WINDOWS_HEIGHT /3;
	clear_screen_to(color_gray());
	// Game instructions
	draw_simple_text("Avoid all the temptations ranging from love to smartphones", color_burly_wood(), 70, 45);	
	draw_simple_text("And shoot down all the tasks!", color_burly_wood(), 200, 65);	
	draw_simple_text("Select a grade you wish to achieve", color_burly_wood(), 180, 85);	
	draw_simple_text("Based on the grade you selected ", color_burly_wood(), 200, 105);
	draw_simple_text("your ability and the challenge might be easier or harder", color_burly_wood(), 70, 125);
	// Draw menu
	fill_rectangle(color_burly_wood(), board_X_offset, board_Y_offset + board_height * 1, board_width, board_height);
	fill_rectangle(color_burly_wood(), board_X_offset, board_Y_offset + board_height * 3, board_width, board_height);
	fill_rectangle(color_burly_wood(), board_X_offset, board_Y_offset + board_height * 5, board_width, board_height);
	fill_rectangle(color_burly_wood(), board_X_offset, board_Y_offset + board_height * 7, board_width, board_height);
	// Draw content in the menu
	draw_simple_text("P", color_gray(), board_X_offset + board_width / 2, board_Y_offset + board_height * 1 + board_height / 2);	
	draw_simple_text("C", color_gray(), board_X_offset + board_width / 2, board_Y_offset + board_height * 3 + board_height / 2);	
	draw_simple_text("D", color_gray(), board_X_offset + board_width / 2, board_Y_offset + board_height * 5 + board_height / 2);	
	draw_simple_text("HD", color_gray(), board_X_offset + board_width / 2 - 3, board_Y_offset + board_height * 7 + board_height / 2);

	refresh_screen();
}
//
//	Check if user has clicked a certain area
//
bool area_clicked(int leftX, int topY, int width, int height){
	float mX, mY;
	bool flag;

	int rightX = leftX + width;
	int bottomY = topY + height;

	flag = false;
	if (mouse_clicked(LEFT_BUTTON)){
		mX = mouse_x();
		mY = mouse_y();
		if ((mX >= leftX) && (mX <= rightX) && (mY >= topY) && (mY <= bottomY)) 
			flag = true;
	}		
	return flag;
}
//
//	Get the level user selected
//	1 - Pass
//	2 - Credit
//	3 - Distinction
//	4 - High Distinction
//
int select_level(){
	int board_width = WINDOWS_WIDTH / 5;
	int board_height = WINDOWS_HEIGHT / 27 * 2;
	int board_X_offset = board_width * 2;
	int board_Y_offset = WINDOWS_HEIGHT /3;

	if (area_clicked(board_X_offset, board_Y_offset + board_height * 1, board_width, board_height))
		return 1;
	else if (area_clicked(board_X_offset, board_Y_offset + board_height * 3, board_width, board_height))
		return 2;
	else if (area_clicked(board_X_offset, board_Y_offset + board_height * 5, board_width, board_height))
		return 3;
	else if (area_clicked(board_X_offset, board_Y_offset + board_height * 7, board_width, board_height))
		return 4;
	else
		return 0;
}
//
//	End game screen
//
void display_end_screen(int score, int highscore){
	int board_width = WINDOWS_WIDTH / 4 + 15;
	int board_height = WINDOWS_HEIGHT / 5 - 5;
	float board_x = (WINDOWS_WIDTH - board_width) / 2;
	float board_y = (WINDOWS_HEIGHT - board_height) / 2;
	char scoreStr[3];
	char highscoreStr[3];

	sprintf(scoreStr, "%d", score);
	sprintf(highscoreStr, "%d", highscore);

	clear_screen_to(color_gray());
	fill_rectangle(color_burly_wood(), board_x, board_y, board_width, board_height);

	if(score < highscore){
		draw_simple_text("Sorry! You died :(", COLOR_BEIGE, board_x + 5, board_y + 5);
		draw_simple_text("Score: ", COLOR_BEIGE, board_x + 5, board_y + 30);	
		draw_simple_text(scoreStr, COLOR_BEIGE, board_x + 60, board_y + 30);
		draw_simple_text("Previous High Score: ", COLOR_BEIGE, board_x + 5, board_y + 55);	
		draw_simple_text(highscoreStr, COLOR_BEIGE, board_x + 100, board_y + 80);
	}else{
		draw_simple_text("Congratulations!", COLOR_BEIGE, board_x + 5, board_y + 5);
		draw_simple_text("New high score:  ", COLOR_BEIGE, board_x + 5, board_y + 30);
		draw_simple_text(scoreStr, COLOR_BEIGE, board_x + 60, board_y + 55);	
	}
	

	refresh_screen();

}
//
// Get previous highscore
//
int get_previous_highscore(){
	FILE *file_ptr;
	int past_highscore;
	
	if ((file_ptr = fopen("highscore.txt", "r")) == NULL)
    	printf("File could not be opened");
	else{
		fscanf(file_ptr, "%d", &past_highscore);
	}
	fclose(file_ptr);
	return  past_highscore;
}
//
// Store new highscore
//
void store_new_highscore(int score){
	FILE *file_ptr;
	if((file_ptr = fopen("highscore.txt", "w")) == NULL)
		printf("File could not be opened\n");
	else{
		fprintf(file_ptr, "%d", score);
	}
	fclose(file_ptr);
}
int main()
{
	GameData game;

	open_audio();
    open_graphics_window("College Game", WINDOWS_WIDTH, WINDOWS_HEIGHT);
    load_resources();
	
	do{
		process_events();
		display_welcome_screen();
		game.level = select_level();
	}while(game.level == 0);

	set_up_game(&game);

    do
    {	
    	process_events();

	    if(game.player.lives != 0){

	       handle_input(&game);
	       update_game(&game);
	       draw_game(game);
	    }
	    else if (game.player.lives == 0){ 						//when the game ends
	    	int highscore = get_previous_highscore();			//get previous highscore
	    	if(game.score >= highscore){						//player achieves a new highscore
	    		store_new_highscore(game.score);
	    	}
	    	display_end_screen(game.score, highscore);
	    }
    	

    } while (! window_close_requested());
    	
   
    release_all_resources();
    return 0;
}
