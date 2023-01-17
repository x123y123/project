# DVFS implement by Q-learning in C
## Replay memory structure
The replay memory is defined to `store the experiences of the agent`. Each experience contains the state, the action, the reward, and the next state. It is used to store the experiences that the agent has encountered during its interactions with the environment.
## Functions
* select_action() : the function which `takes the current state as an input, and returns the action with the highest Q-value for that state`. This function is used to select the action that the agent should take based on the current Q-values.
* update_q() : the function which takes the current state, the action, the next state, and the reward as inputs, and updates the Q-value for the state-action pair using the Q-learning update rule.
* add_experience() : the function which takes the current state, the action, the reward, and the next state as inputs, and adds the experience to the replay memory.
* main() : contains the Q-learning loop.
## Program flow
1. The Q-table is initialized by generating `random` values for each state-action pair.
2. The program enters the Q-learning loop.
3. In each episode, the program `selects a random initial state`, and `selects an action` **based on the current Q-values** using the select_action() function.
4. Based on the action, the program rises or reduces the CPU frequency.
5. The program then `determines the reward and the next state` based on the new CPU frequency.
6. The program adds the experience to the replay memory using the add_experience() function.
7. The program samples a random experience from the replay memory.
8. The program updates the Q-value for the state-action pair using the update_q() function and the experience that was just sampled.
9. The program repeats steps 2 to 8 for the specified number of episodes, and then the program terminates.

