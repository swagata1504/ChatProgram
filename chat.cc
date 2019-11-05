#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <bitset>
using namespace std;

char packetToSend(char msg[],string data){
    //16 bit htons header
   string IPver = std::bitset<16> (htons(457)).to_string();
   string len = std::bitset<16> (htons(data.length())).to_string();
   //adding it all to one char*
   strcpy(msg,IPver.c_str());
   strcat(msg,len.c_str());
   strcat(msg, data.c_str());
    return *msg;
}

char packetRecieved(char msg[]){
    char IP[17] = "";
    char length[17] = "";

    for(int i =0;i<16;i++){
        IP[i]=msg[i];
    }
    IP[16] = '\0';
    uint16_t IPadd = ntohs(stoi(IP, nullptr, 2));
    if(IPadd!=457){
        cout<<"Use IPv4 please!";
    }
    for(int i =16;i<32;i++){
        length[i-16]=msg[i];
    }
    length[16] = '\0';
    uint16_t dataLen = ntohs(stoi(length, nullptr, 2));
    char receiveMsg[dataLen+1] ="";
    for(int i =0;i<dataLen;i++){
        receiveMsg[i]=msg[i+32];
    }
    receiveMsg[dataLen]='\0';
    
    strcpy(msg,receiveMsg);
    return *msg;
}

int Server(){
    int port=3493;
    char host[150];
	//buffer to send and receive messages with
	char msg[1500];
  //setup a socket and connection tools
    int serverSd=0;
    int bindStatus=0;
	int status;
	struct addrinfo hints;
	struct addrinfo *servinfo;  // will point to the results
	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_INET; //IPV4
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
	hints.ai_flags = AI_PASSIVE; // fill in my IP for me

	if ((status = getaddrinfo(NULL, "3493", &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}

    serverSd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if(serverSd < 0)
    {
        cerr << "Error establishing the server socket" << endl;
        exit(1);
    }

    bindStatus = bind(serverSd, servinfo->ai_addr, servinfo->ai_addrlen);
    if(bindStatus < 0)
    {
        cerr << "Error binding socket to local address" << endl;
        exit(1);
    }
    
    listen(serverSd, 1);
    
    struct hostent *he;
    gethostname(host, sizeof(host));
    he=gethostbyname(host);
    struct in_addr* ipaddr= (struct in_addr*)he->h_addr;
    
    sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    string ipaddy= inet_ntoa(*ipaddr);
    cout << "Welcome to Chat!" << endl;
    cout << "Waiting for a connection on " << ipaddy << " port "<< port << endl;
    int newSd = accept(serverSd, (sockaddr *)&newSockAddr, &newSockAddrSize);
    if(newSd < 0)
    {
        cerr << "Error accepting request from client!" << endl;
        exit(1);
    }
    cout << "Found a friend! You receive first." << endl;\
    int bytesRead =0;
   while(1)
   {
       memset(&msg, 0, sizeof(msg));//clear the buffer
       bytesRead += recv(newSd, (char*)&msg, sizeof(msg), 0);
       if(bytesRead==0){
           cout << "Friend has left" << endl;
           break;
       }
       *msg=packetRecieved(msg);
       cout << "Friend: " << msg << endl;
       cout << "You: ";
       string data;
       getline(cin, data);
       while(data.length()>140){
           cout<<"Error: Input too long."<<endl;
           cout << "You: ";
           getline(cin, data);
       }
       memset(&msg, 0, sizeof(msg)); //clear the buffer
       strcpy(msg, data.c_str());
       *msg=packetToSend(msg, data);
       send(newSd, (char*)&msg, strlen(msg), 0);
   }
   
   close(newSd);
   close(serverSd);
   freeaddrinfo(servinfo);
    return 0;   
}


int Client(char* port, char* ip){
    char msg[1500];
    
    //setup socket data
    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo; //will point to results
    
    memset(&hints,0, sizeof(hints)); 
    hints.ai_family = AF_INET; //IPV4 
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    
    status = getaddrinfo(ip, port, &hints, &servinfo);
    
    //socket
   
    int clientSd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if(clientSd < 0)
    {
        cerr << "Error establishing the client socket" << endl;
        exit(0);
    }
    
    
    
    cout<< "Connecting to server..."<< endl;
    //connect to given ip and port
    cout<<connect(clientSd, servinfo->ai_addr, servinfo->ai_addrlen);
    if (connect(clientSd, servinfo->ai_addr, servinfo->ai_addrlen)!=-1){
        cout<< "Connected!"<< endl;
    }
    
    cout<<"Connected to a friend! You send first." << endl;
    
   int bytesRead=0;
   while(1)
   {
       cout << "You: ";
       string data;
       getline(cin, data);
       while(data.length()>140){
           cout<<"Error: Input too long."<<endl;
           cout << "You: ";
           getline(cin, data);
       }
       memset(&msg, 0, sizeof(msg));//clear the buffer
       strcpy(msg, data.c_str());
       *msg = packetToSend(msg, data);
       
       if(data == "exit")
       {
           send(clientSd, (char*)&msg, strlen(msg), 0);
           break;
       }
       send(clientSd, (char*)&msg, strlen(msg), 0);
       cout << "Awaiting server response..." << endl;
       memset(&msg, 0, sizeof(msg));//clear the buffer
       bytesRead += recv(clientSd, (char*)&msg, sizeof(msg), 0);
       if(bytesRead==0){
           cout << "Friend has exited" << endl;
           break;
        }
        *msg = packetRecieved(msg);
       cout << "Friend: " << msg << endl;
   }
   close(clientSd);
   freeaddrinfo(servinfo);
   return 0;
    
}


int main(int argc, char** argv) {
    int host,client,server;
    char* ip;
    char* port;
    char buffer[1024];
    struct sockaddr_in server_addr;
    if(argc>1){
        cout<<argc<<endl;
        if(argc!=5 && argc!=2){
            cout<<"Wrong input! Make sure to check the rules by typing './chat -h'"<<endl;
            exit(0);
        }
        for (int i = 0; i < argc ; ++i){
            string someString(argv[i]);
            if(someString=="-p"){
                port = argv[i+1];
            }else if(someString =="-s"){
                ip = argv[i+1];
            }else if(someString =="-h"){
                cout<<"Make sure to run the client side this way:"<<endl;
                cout<<"./chat -p [PORT NUMBER] -s [IP ADDRESS]"<<endl;
                cout<<"Make sure to run the server side this way:"<<endl;
                cout<<"./chat"<<endl;
                exit(0);
            }
        }
        Client(port, ip);
    }else{
        Server();
    }
    cout<<"\n";
    return 0;
}
