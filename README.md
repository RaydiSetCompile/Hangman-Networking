# Multithreaded Networked Hangman Game
Developed a TCP client/server system which would allow users to play hangman and compete scores against one another. 
Implemented in C++ using sockets, and multithreading using pthreads, allows service to multiple user requests simultaneously.
Users score is uploaded onto the leaderboard on the server.
User's score is caluclated by turns taked divided by word length, with the lower score being the better score to ensure fairness for different word lengths.
