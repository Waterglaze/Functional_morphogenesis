#ifndef GRN_H
#define GRN_H

#ifdef KILOBOT  // physical kilobot

#else           //  simulator
  #include <stdio.h>     // for printf
#endif


#include <math.h>
#include <stdint.h>   // for uint8_t and friends

#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502884197169399375105820974944
#endif


#define MAXN 20     // Maximum number of neighbors

#define RB_SIZE 16  // Ring buffer size. Choose a power of two for faster code
                    // memory usage: 16*RB_SIZE
                    // 8 works too, but complains in the simulator
                    // when the bots are very dense


// States of the robots
enum BOTSTATES {WAIT, ORBIT, FOLLOW, PATTERN_FORMATION, PATTERN_CHECKING, PATTERN_READY};


// Neighbors' table structure
typedef struct {

  uint16_t ID;                          // ID of this neighbor
  uint8_t dist;                         // Distance to this neighbor

  uint8_t n_bot_state;                  // State of this neighbor
  uint8_t N_Neighbors;                  // Number of neighbors of this neighbor
  int delta_dist;                       // Difference in distance with respect to the previous moment that this neighbor was updated

  float molecules_concentration[2];     // Concentration of molecules U and V of this neighbor, respectively

  uint8_t binary_information;           // First bit (position 0): polarized by neighbours
                                        // Second bit (position 1): new_source
                                        // Third bit (position 2): test_edge

  int8_t hops_to_morphogen_gradient;    // Hops to the source of the morphogen
  uint16_t source_signal_id;            // Id of the source of the morphogen

  uint32_t timestamp;                   // kilo_ticks when this neighbor was updated

} Neighbor_t;

typedef struct {
    message_t msg;
    distance_measurement_t dist;
} received_message_t;


typedef struct
{
  Neighbor_t neighbors[MAXN];           // Neighbors' table

  int N_Neighbors;                      // Number of neighbors
  uint8_t bot_state;                    // State of the robot
  uint8_t move_type;                    // Type of motion sent to the robot: STOP, LEFT, RIGHT, STRAIGHT (defined in utils.h)
  uint8_t last_turn;                    // Last direction of movement (left or right)

  float molecules_concentration[2];     // Concentration of molecules U and V, respectively   
  float sum_diff_concentration;         // Sum of the change in concentration of molecule u in every timestep
      
  uint32_t counter;                     // Counter to wait until there aren't any more neighbors moving during exploration, or
                                        // to work out sum of difference in concentration during patterning

  uint8_t move_switch_flag;             // Flag to switch direction of movement

  float running_avg_NNs;                // Running average of the number of neighbors of this robot's neighbors
  float running_avg_Ns;                 // Running average of the number of neighbors of this robot

  message_t transmit_msg;               // Message to be transmitted to neighbors
  char message_lock;                    // Lock to avoid message transmission while modifying the message

  received_message_t RXBuffer[RB_SIZE]; // Messages received

  uint8_t RXHead, RXTail;               // Pointers to the first and last message received

  uint8_t allowed_to_be_polarized_by_neighbours;    // Whether it is allowed to be polarized by neighbours. Values:
                                                    // 1: it is allowed
                                                    // 2 + n: it isn't allowed and it's been orbiting around the SAME neighbours n times

  uint8_t polarized_by_patterning;      // Whether it is polarized by patterning
  uint8_t polarized_by_neighbours;      // Whether it is polarized by neighbours
  int8_t hops_to_morphogen_gradient;    // Hops to the source of the morphogen
  uint16_t source_signal_id;            // Id of the source of the morphogen

  uint8_t new_source;                   // 0: not new source  
                                        // 1: new source and it's me   
                                        // 2: new source in the spot, but it isn't me
                                        // 3 + (n-1): not new source and it's been orbiting around DIFFERENT neighbours n times

  uint8_t initial_n_neighbors;          // Number of neighbours when it started an orbiting block
  uint16_t initial_neighbors_ID[MAXN];  // IDs of neighbours when it started an orbiting block
  uint8_t n_different_neighbors;        // Number of different neighbours after an orbiting block to prevent faulty-motion robots from moving after a while

} USERDATA;


// Pointer to the internal variables of the robot
USERDATA *mydata;


// Ring buffer operations. Taken from kilolib's ringbuffer.h
// but adapted for use with mydata->

// Ring buffer operations indexed with head, tail
// These waste one entry in the buffer, but are interrupt safe:
//   * head is changed only in popfront
//   * tail is changed only in pushback
//   * RB_popfront() is to be called AFTER the data in RB_front() has been used
//   * head and tail indices are uint8_t, which can be updated atomically
//     - still, the updates need to be atomic, especially in RB_popfront()

#define RB_init() {	\
    mydata->RXHead = 0; \
    mydata->RXTail = 0;\
}

#define RB_empty() (mydata->RXHead == mydata->RXTail)

#define RB_full()  ((mydata->RXHead+1)%RB_SIZE == mydata->RXTail)

#define RB_front() mydata->RXBuffer[mydata->RXHead]

#define RB_back() mydata->RXBuffer[mydata->RXTail]

#define RB_popfront() mydata->RXHead = (mydata->RXHead+1)%RB_SIZE;

#define RB_pushback() {\
    mydata->RXTail = (mydata->RXTail+1)%RB_SIZE;\
    if (RB_empty())\
      { mydata->RXHead = (mydata->RXHead+1)%RB_SIZE;	}				\
  }

#endif
