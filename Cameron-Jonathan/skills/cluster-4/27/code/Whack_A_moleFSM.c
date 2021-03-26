#include <stdio.h>

#define     NoMoles 0
#define     A       1
#define     B       2
#define     C       3
#define     D       4

typedef enum {
    NO_MOLES,
    A,
    B,
    C,
    D
} state_e;

typedef enum {
    HIT,
    TIMEOUT,
    WAIT
} event_e;

int read_event();
int rand_mole(int event);

state_e state;
state_e next_state;
event_e event;

int main() {
    while(1) {
        event = read_event();
        if( state == NO_MOLES )
        {
            if( event == TIMEOUT )
            {
                next_state = rand_mole(NO_MOLES);
            }
            else if( event == WAIT )
            {
                next_state = state;
            }
        }
        if( state == A )
        {
            if( event == TIMEOUT )
            {
                next_state = NO_MOLES;
            }
            else if ( event == WAIT )
            {
                next_state = state;
            } 
            else if ( event == HIT )
            {
                next_state = rand_mole(A);
            }
        }
        if( state == B )
        {
            if( event == TIMEOUT )
            {
                next_state = NO_MOLES;
            }
            else if ( event == WAIT )
            {
                next_state = state;
            } 
            else if ( event == HIT )
            {
                next_state = rand_mole(B);
            }
        }
        if( state == C )
        {
            if( event == TIMEOUT )
            {
                next_state = NO_MOLES;
            }
            else if ( event == WAIT )
            {
                next_state = state;
            } 
            else if ( event == HIT )
            {
                next_state = rand_mole(C);
            }
        }
        if( state == D )
        {
            if( event == TIMEOUT )
            {
                next_state = NO_MOLES;
            }
            else if ( event == WAIT )
            {
                next_state = state;
            } 
            else if ( event == HIT )
            {
                next_state = rand_mole(D);
            }
        }
        state = next_state;

    }
}


int read_event(){
    return ( rand() % 3 ); // randomly return an event since we have no hardware interrupts
}

int rand_hole(int state_passed){
    if( state_passed == NO_MOLES ){
        return ((rand() % 3) + 1); // If the current state is NO_MOLES, return a random hole for a mole A -> D
    }
    else if( (state_passed == A) || (state_passed == B) || (state_passed == C) || (state_passed == D) ){
        return (rand() % 4); // If the current state is A -> D, the next state can be another mole hole or the NO_MOLES state.
    }
}