#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEYBOARD_CAPS_LOCK_ON 1
#define KEYBOARD_CAPS_LOCK_OFF 0

typedef int KEYBOARD_CAPS_LOCK_STATE;

struct Process;

typedef int (*KEYBOARD_INIT_FUNCTION)();
struct Keyboard
{
    KEYBOARD_INIT_FUNCTION init;
    char name[20];    

    KEYBOARD_CAPS_LOCK_STATE capslock_state;

    struct Keyboard* next;
};

void keyboard_init();
void keyboard_backspace(struct Process* process);
void keyboard_push(char c);
char keyboard_pop();
int keyboard_insert(struct Keyboard* keyboard);
void keyboard_set_capslock(struct Keyboard* keyboard, KEYBOARD_CAPS_LOCK_STATE state);
KEYBOARD_CAPS_LOCK_STATE keyboard_get_capslock(struct Keyboard* keyboard);

#endif