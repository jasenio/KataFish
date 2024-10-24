## C++ Chess Engine
### Implementations from _Artificial Intelligence: A Modern Approach | Adversarial Search and Games_
Currently, the engine has these implementations from the book (start on lines 2225)

**1. Alpha-Beta Pruning**
**2. Iterative Deepening**
**3. Transposition and Transposition Tables**
**4. Material Value**
**5. Quiescence Search**

The code also has **6. Piecetable Evaluations** which are not mentioned in the book 
but widely used and easier to implementthan the weighted linear evaluations suggested. 
The piecetables are not optimized for this engine.

### Progress
Needs slight improvement on evaluation. Search can greatly be improved; nodes searched 
needs to be reduced, engine averages around 8 ply / min, depending on stage of the game. 
Planning to use a more sophisticated cutoff function, adding more move ordering heurstics
besides transposition hash move.
[Progress](https://docs.google.com/spreadsheets/d/1yk-q0h4t2coXKYTTrj6y8UgqU-9TnQ4XuYsEa3ZMVYg/edit?usp=sharing)

### Note
Chess bit boards was NOT DEVELOPED BY ME. The move generation was built in C by 
Code Monkey King who runs the Chess Programming Wiki. I am working on the 
search and evaluation functions using these bitboards to achieve faster generation 
than array-based generation.
