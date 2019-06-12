#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string>
#include <string.h>
#include <iostream>
#include <fstream>
#include <arpa/inet.h>
#include <sstream>
#include <cstdint>
#include <ctime>
#include <math.h> 
#include <algorithm>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iomanip>
#include <sstream>

using namespace std;

struct lbNode {
  string user;
  float score;
};

string lbString = "Leader Board \n \n";
pthread_mutex_t m;
lbNode leaderBoard[4];
int numOfEntries;


bool sorter (lbNode const& lhs, lbNode const&rhs) {
  return lhs.score < rhs.score;
}



void sendStr(int socket, string str) {
  char buffer[2000];
  strcpy(buffer, str.c_str());
  int status = send(socket, buffer, 2000, 0);
  if(status == -1) {
    cout << ("Error: Send failed");
    close(socket);
    exit(1);
  }
}





string receive(int socket) {
    char buffer[2000];
    int bytesRem = 2000;
    while(bytesRem > 0) {
        int recvBytes = recv(socket, buffer, bytesRem, 0);
        if(recvBytes < 0) {
            cout << ("Error: Failed to receive bytes");
					  close(socket);
            exit(1);
        }
        bytesRem -= recvBytes;
    }
    return buffer;
}

string getWord() {
  srand(time(NULL));
  string path = "/home/fac/lillethd/cpsc3500/projects/p4/words.txt";
	ifstream myfile(path.c_str());
	string word = "";
	int index = rand() % 57127;
	for(int i = 0; i < index; i++){
		myfile >> word;
	}
	return word;
}

void* playHangman(void* socket) {
  float score;

  int valid;
  uint16_t converter;
  int status;

  int clientSock = *(int*)socket;
  string username;
  bool done = false;
  int turnCounter = 1;

  string word;
  int wordLen;
  string guess;


  int guessedPointer = 0;
  string guessedCharacters[2000];
  bool guessedC;
  bool correct;
  string reply = "";

  string userProgress = "";  // e.g "-----"
  int userProgressCount = 0;

  username = receive(clientSock); // recv 1: receive username
  cout << "USER: " << username << " has connected \n";

  word = getWord();
  wordLen = word.length();

  cout << "The word is: " << word << " for user: " << username << "\n";

  for(int i = 0; i < wordLen; i++) {
    userProgress += "-";
  }

  while(!done) {
    guessedC = false;
    sendStr(clientSock, to_string(turnCounter)); // send 2

    sendStr(clientSock, userProgress); // send 3


    guess = receive(clientSock); // recv 4

    cout << "User: " << username << " guess: " << guess << endl;

    if( (guess.length() != 1) || (isalpha(guess[0]) == 0)) {
      cout << "Error: user guess invalid";
      pthread_detach(pthread_self());
			close(clientSock);
      exit(1);
    }

    guess[0] = toupper(guess[0]);

    // check if guess had been guessed
    for(int i = 0; i < guessedPointer; i++) {
      if(guess == guessedCharacters[i]) {
        cout << username << " has guessed this character" << endl;
        guessedC = true;
      }
    }

    if(guessedC) {
      valid = 0;
      converter = htons(valid);
		  status = send(clientSock, &converter, sizeof(uint16_t), 0); // send 5
      if(status <= 0) {
        cout << "Error: send valid";
        pthread_detach(pthread_self());
				close(clientSock);
        exit(1);
      }
    }

    else {
      guessedCharacters[guessedPointer] = guess;
      guessedPointer++;
      correct = false;
      valid = 1;
      converter = htons(valid);
		  status = send(clientSock, &converter, sizeof(uint16_t), 0); // send 5
      if(status <= 0) {
        cout << "Error: send valid";
        pthread_detach(pthread_self());
					close(clientSock);
        exit(1);
      }

      // Check if guess in word
      for(int i = 0; i < wordLen; i++) {
          if(guess[0] == word[i]) {
            userProgress[i] = guess[0];
            correct = true;
            userProgressCount++;
          }
      }

      if(correct) reply = "Correct!";
      else reply = "Incorrect!";

      sendStr(clientSock, reply); // send 6

      if(userProgressCount == wordLen) {

        done = true;
        valid = 1;
        converter = htons(valid);
        status = send(clientSock, &converter, sizeof(uint16_t), 0); // send 7
        if(status <= 0) {
          cout << "Error: send valid";
          pthread_detach(pthread_self());
					close(clientSock);
          exit(1);
        }

        reply = "\nCongratulations! You guessed the word " + word + "!!\n" + "It took " + to_string(turnCounter) + " turns to guess the word correctly.";
        sendStr(clientSock, reply); // send 8


        score = (float)turnCounter/(float)wordLen;
        score = floor(score * 100.00 + 0.5 ) / 100.00;

        stringstream stream;
        
        stream << fixed << setprecision(2) << score;
        string s = stream.str();
        

        cout << username << "'s score is " << s << "\n";

        pthread_mutex_lock(&m);
        reply = "";
        reply += lbString;
        numOfEntries++;
        
        if(numOfEntries > 3) numOfEntries = 3;

        leaderBoard[3].score = score;
        leaderBoard[3].user = username;

        sort(begin(leaderBoard), end(leaderBoard), &sorter);

        for(int i = 0; i < numOfEntries; i++) {
          stream.str(string());
          stream << leaderBoard[i].score;
          s = stream.str();

          reply = reply + leaderBoard[i].user + " " + s + "\n";
        }

        sendStr(clientSock, reply); // send 9
        pthread_mutex_unlock(&m);

      }

      else {
        valid = 0;
        converter = htons(valid);
        status = send(clientSock, &converter, sizeof(uint16_t), 0); // send 7
        if(status <= 0) {
          cout << "Error: send valid";
          pthread_detach(pthread_self());
					close(clientSock);
          exit(1);
        }
      }

      turnCounter++;

    }

  }

  pthread_detach(pthread_self());
	close(clientSock);
}


int main(int argc,char* argv[]) {

  if(argc != 2) {
    cout << "Error: Port Number not entered \n";
    exit(1);
  }
  for(int i=0; i<4; i++){
		leaderBoard[i].user = "";
		leaderBoard[i].score = 999999999;
	}

  numOfEntries = 0;

  pthread_mutex_init(&m, NULL);
  int status;
  unsigned short portNum = atoi(argv[1]);

  // create server socket
  int sockfd;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(portNum);
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

  // bind socket to specified ip and port
  status = bind(sockfd, (struct sockaddr*) &serverAddr, sizeof(serverAddr));
  if(status == -1) {
    cout << ("Error: Failed to bind server socket");
    exit(1);
  }


  listen(sockfd, 128);
  cout << ("Server Listening... \n");

 // create Client Socket
  int clientSock;
  struct sockaddr_in clientAddr;

  pthread_t tid;


  while(1) {
      socklen_t addrLen = sizeof(clientAddr);
      int clientSock = accept(sockfd, (struct sockaddr*)&clientAddr, &addrLen);
      if(clientSock == -1) {
        cout << ("Error: Client failed to connect");
        close(clientSock);
        exit(1);
      }
      cout << ("Connection Accepted \n");

      // Create Threads
      status = pthread_create(&tid, NULL, playHangman, (void*) &clientSock);
      if(status != 0) {
        cout << ("Error: Failed to create thread \n");
        close(clientSock);
        exit(1);
      }
  }

  close(clientSock);
  return 1;

}
