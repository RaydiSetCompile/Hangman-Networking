#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <unistd.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

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


void playGame(int socket, string username) {

  int valid;
  int status;

  uint16_t converter;
  // Game starts here
  int sockfd = socket;

  string reply = "";
  string word;
  string guess = "";
  string turn;

  bool guessed = false; // 0 is false, 1 is true


  while(!guessed){
    turn = receive(sockfd); // recv 2
    cout << "\nTurn " << turn << "\n";

    word = receive(sockfd); // recv 3
    cout << "Word: " << word << "\n";

    do {
      cout << "Enter your guess: ";
      cin >> guess;
    } while(guess.length() != 1 || isalpha(guess[0]) == 0);


    sendStr(sockfd, guess); // send 4

    // See if entered guess is invalid
    status = recv(sockfd, &converter, sizeof(uint16_t), 0); // recv 5
    if(status <= 0){
      cout << "Error: Failed to receive bytes";
      close(sockfd);
      exit(1);
		}
		valid = ntohs(converter);

    if(valid != 1) {
      cout << "You had already tried that character. Please try again... \n";
      cout << "Your turn will not be updated \n";
    }

    else {
      reply = receive(sockfd); // recv 6

      cout << reply << "\n";

      status = recv(sockfd, &converter, sizeof(uint16_t), 0); // recv 7
      if(status <= 0){
        cout << "Error: Failed to receive bytes";
        close(sockfd);
        exit(1);
  		}

  		valid = ntohs(converter);


      // Game completed
      if(valid == 1) {
        guessed = true;
        reply = receive(sockfd); // recv 8
        cout << reply << endl;
        guessed = true;

        cout << "\n";
        reply = receive(sockfd); // recv 9
        cout << reply << endl;

      }

    }
  }

}


int main(int argc, char*  argv[]) {
  if(argc != 3) {
    cout << "Error: Port Number or IP Address not entered \n";
    exit(1);
  }

  int sockfd;
  string ipAddr = argv[1];
  char ip[ipAddr.size() + 1];
  strcpy(ip, ipAddr.c_str());
  

  unsigned short portNum = atoi(argv[2]);

  // Create Socket
  sockfd  = socket(AF_INET, SOCK_STREAM, 0);

  // Server Address
  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(portNum);
  serverAddr.sin_addr.s_addr = inet_addr(ip);


  int status = connect(sockfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr));


  if(status == -1){
    cout << ("Error: Connection could not be established");
    close(sockfd);
    exit(1);
  }

  cout << ("Welcome to Hangman! \n");
  string username;
  cout << ("Enter username: ");
  cin >> username;

  sendStr(sockfd, username); // send 1

  playGame(sockfd, username);


  close(sockfd);

  return 1;

}
