
// onevent:
    // event = player/ball, posizione
    // if player:
        // update player position
    // if ball:
        // TODO per ora ogni volta, se è troppo lento, ogni n volte
        // find possession
        // send messaggio a possession_p
            // messaggio = posizione palla + tutte posizioni player + counter intervallo
        // numero calcoli possesso += 1
    // if finito intervallo (timestamp > interval_end):
        // send messaggio a output
            // messaggio = "print", numero calcoli possesso (quelli che dovrà aspettare)
        // interval_end += T
        // counter intervallo += 1
        // numero calcoli possesso = 0
    // if "end of game":
        // send "end of game" to possession
        // return

#include <stdio.h>

void onevent_run() {
    printf("TODO");
}