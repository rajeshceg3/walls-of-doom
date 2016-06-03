#ifndef PLAYER_H
#define PLAYER_H

#define PLAYER_NAME_MAXIMUM_SIZE 64

typedef struct Player {
    char *name;
    
    int x;
    int y;
    int speed_x;
    int speed_y;

    // Whether or not the player is being affected by physics.
    int physics;
    
    int can_double_jump;
    int remaining_jump_height;

    int lives;
    int score;
} Player;

/**
 * Returns a Player object with the provided name.
 */
Player make_player(char *name);

#endif
